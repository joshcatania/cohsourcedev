#include <utilitieslib/utils/utils.h>
#include <utilitieslib/utils/file.h>
#include <utilitieslib/network/netio.h>
#include <utilitieslib/utils/file.h>
#include <utilitieslib/components/earray.h>
#include <utilitieslib/utils/textparser.h>
#include <utilitieslib/utils/sysutil.h>
#include <utilitieslib/utils/SuperAssert.h>
#include <utilitieslib/utils/winutil.h>

#include "serverCmd.h"
#include "serverMonitorNet.h"
#include "serverCmdStats.h"

#include "http.h"
#include "serverapi.h"

ServerAPIConfig config = {0};

SERVICE_STATUS service = {0};
SERVICE_STATUS_HANDLE hservice;

static TokenizerParseInfo ParseAPIShard[] = {{
                                                 "",
                                                 TOK_STRUCTPARAM | TOK_STRING(ServerAPIShard, name, 0),
                                             },
                                             {
                                                 "",
                                                 TOK_STRUCTPARAM | TOK_STRING(ServerAPIShard, dbserver, 0),
                                             },
                                             {"\n", TOK_END, 0},
                                             {"", 0, 0}};

static TokenizerParseInfo ParseAPIConfig[] = {
    {"Port", TOK_INT(ServerAPIConfig, port, 8913)}, {"Shard", TOK_STRUCT(ServerAPIConfig, shards, ParseAPIShard)}, {"", 0, 0}};

void initState(ServerMonitorState* state)
{
    state->eaMaps = &state->eaMaps_data;
    state->eaMapsStuck = &state->eaMapsStuck_data;
    state->eaLaunchers = &state->eaLaunchers_data;
    state->eaServerApps = &state->eaServerApps_data;
    state->eaEnts = &state->eaEnts_data;
}

static bool loadConfig()
{
    char cfgpath[MAX_PATH], buf[MAX_PATH];

    getExecutableDir(cfgpath);
    // fileSetBaseDir(cfgpath);
    // fileInitSys();
    strcat_s(SAFESTR(cfgpath), "/serverapi.cfg");

    if (!fileLocateRead(cfgpath, buf))
    {
        writeConsole(OUTPUT_ERROR, "Could not open ServerAPI.cfg");
        return false;
    }

    ParserInitStruct(&config, sizeof(config), ParseAPIConfig);
    if (!ParserLoadFiles(NULL, buf, NULL, 0, ParseAPIConfig, &config, NULL, NULL, NULL))
    {
        writeConsole(OUTPUT_ERROR, "Could not parse ServerAPI.cfg");
        return false;
    }

    return true;
}

static void initShards()
{
    int i;

    config.shardidx = stashTableCreateWithStringKeys(16, StashDefault);

    for (i = eaSize(&config.shards) - 1; i >= 0; --i)
    {
        ServerAPIShard* shard = config.shards[i];
        writeConsole(OUTPUT_INFO, "Found config for Shard %s (%s)", config.shards[i]->name, config.shards[i]->dbserver);
        ServerMonitorState* state = calloc(1, sizeof(ServerMonitorState));
        state->eaMaps = &state->eaMaps_data;
        state->eaMapsStuck = &state->eaMapsStuck_data;
        state->eaLaunchers = &state->eaLaunchers_data;
        state->eaServerApps = &state->eaServerApps_data;
        state->eaEnts = &state->eaEnts_data;
        shard->state = state;
        InitializeCriticalSection(&shard->state->stats_lock);
        stashAddPointer(config.shardidx, shard->name, shard, true);
    }
}

static void svrMonTick()
{
    int i;

    for (i = eaSize(&config.shards) - 1; i >= 0; --i)
    {
        ServerAPIShard* shard = config.shards[i];
        ServerMonitorState* state = shard->state;
        if (!svrMonConnected(state))
        {
            svrMonConnect(state, shard->dbserver);
        }

        if (svrMonConnected(state))
        {
            EnterCriticalSection(&state->stats_lock);
            svrMonNetTick(state);
            serverCmdUpdateDbStats(state);
            LeaveCriticalSection(&state->stats_lock);
        }
    }
}

static void serverLoop(bool useservice)
{
    initShards();
    startHttp(&config);

    while (!useservice || service.dwCurrentState == SERVICE_RUNNING)
    {
        svrMonTick();
        Sleep(500);
    }

    stopHttp(&config);
}

static void ControlHandler(DWORD request)
{
    switch (request)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            service.dwWin32ExitCode = 0;
            service.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(hservice, &service);
            return;
        default:
            break;
    }

    // Report current status
    SetServiceStatus(hservice, &service);
}

static void ServiceMain(int argc, char* argv[])
{
    service.dwServiceType = SERVICE_WIN32;
    service.dwCurrentState = SERVICE_START_PENDING;
    service.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    hservice = RegisterServiceCtrlHandler("ServerAPI", (LPHANDLER_FUNCTION)ControlHandler);
    if (!hservice)
        return;

    service.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(hservice, &service);

    serverLoop(true);
}

static void installService()
{
    SC_HANDLE hscm, hsvc;
    char exepath[MAX_PATH];
    char svcpath[MAX_PATH];

    hscm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hscm)
    {
        writeConsole(OUTPUT_ERROR, "Failed to open service manager.");
        return;
    }

    GetModuleFileName(NULL, exepath, MAX_PATH);
    strcpy_s(SAFESTR(svcpath), "\"");
    strcat_s(SAFESTR(svcpath), exepath);
    strcat_s(SAFESTR(svcpath), "\" -service");
    hsvc = CreateService(hscm, "ServerAPI", "CoH Server API", SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, svcpath,
                         NULL, NULL, NULL, "NT AUTHORITY\\LocalService", NULL);

    if (hsvc)
    {
        writeConsole(OUTPUT_INFO, "Service installed.");
        CloseServiceHandle(hsvc);
    }
    else
    {
        writeConsole(OUTPUT_ERROR, "Service failed to install.");
    }

    CloseServiceHandle(hscm);
}

static void removeService()
{
    SC_HANDLE hscm, hsvc;

    hscm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hscm)
    {
        writeConsole(OUTPUT_ERROR, "Failed to open service manager.");
        return;
    }

    hsvc = OpenService(hscm, "ServerAPI", SERVICE_ALL_ACCESS);

    if (hsvc && DeleteService(hsvc))
    {
        writeConsole(OUTPUT_INFO, "Service removed.");
    }
    else
    {
        writeConsole(OUTPUT_ERROR, "Failed to remove service.");
    }

    if (hsvc)
        CloseServiceHandle(hsvc);
    CloseServiceHandle(hscm);
}

int main(int argc, char* argv[])
{
    int i;
    SERVICE_TABLE_ENTRY stable[2] = {0};
    bool instservice = false;
    bool runservice = false;
    bool removeservice = false;

    memCheckInit();
    setAssertMode(ASSERTMODE_DEBUGBUTTONS | ASSERTMODE_FULLDUMP);

    EXCEPTION_HANDLER_BEGIN

    for (i = 1; i < argc; ++i)
    {
        if (!stricmp(argv[i], "-service"))
            runservice = true;
        if (!stricmp(argv[i], "-install"))
            instservice = true;
        if (!stricmp(argv[i], "-remove"))
            removeservice = true;
    }

    if (!loadConfig())
        return 1;

    if (instservice)
    {
        installService();
    }
    else if (removeservice)
    {
        removeService();
    }
    else if (runservice)
    {
        stable[0].lpServiceName = "ServerAPI";
        stable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
        StartServiceCtrlDispatcher(stable);
    }
    else
    {
        setWindowIconColoredLetter(compatibleGetConsoleWindow(), 'A', 0xAA0000);
        writeConsole(OUTPUT_INFO, "Running interactively, press CTRL+C to quit.");
        serverLoop(false);
        getch();
    }

    EXCEPTION_HANDLER_END

    return 0;
}
