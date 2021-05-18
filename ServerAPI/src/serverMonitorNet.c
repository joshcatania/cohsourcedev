#include <comm_backend.h>
#include <container.h>
#define SVRMONCOMM_PARSE_INFO_DEFS
#include <container/containerbroadcast.h>
#include <entity/entVarUpdate.h>
#include <launcher_common.h>
#include <svrmoncomm.h>
#include <utilitieslib/components/earray.h>
#include <utilitieslib/components/StashTable.h>
#include <utilitieslib/network/net_link.h>
#include <utilitieslib/network/net_socket.h>
#include <utilitieslib/network/netio.h>
#include <utilitieslib/network/netio_core.h>
#include <utilitieslib/network/sock.h>
#include <utilitieslib/utils/file.h>
#include <utilitieslib/utils/ListView.h>
#include <utilitieslib/utils/mathutil.h>
#include <utilitieslib/utils/structHist.h>
#include <utilitieslib/utils/structNet.h>
#include <utilitieslib/utils/timing.h>
#include <utilitieslib/utils/utils.h>

#include <assert.h>
#include <direct.h>
#include <windowsx.h>

#include "serverMonitorNet.h"
#include "serverCmd.h"

U32 timestamp = 0; // Set globally
bool g_someDataOutOfSync = false;

// static bool received_update=false;

static TokenizerParseInfo* destructor_tpi = NULL;
static void destructor(void* data)
{
    if (data == NULL || data == (void*)(intptr_t)1)
        return;

    if (destructor_tpi)
    {
        sdFreeStruct(destructor_tpi, data);
    }
    else
    {
        assert(0);
    }
    free(data);
}

static void clearConsFromFilterList(DbContainer*** eaCons, DbContainer*** eaConsFiltered)
{
    int i;
    // Remove all duplicates of the eaMaps structure that happen to be in eaMapsStuck
    for (i = 0; i < eaSize(eaCons); i++)
    {
        DbContainer* con = (*eaCons)[i];
        eaRemove(eaConsFiltered, eaFind(eaConsFiltered, con));
    }
}

void svrMonClearAllLists(ServerMonitorState* state)
{
    clearConsFromFilterList((DbContainer***)state->eaMaps, (DbContainer***)state->eaMapsStuck);
    destructor_tpi = MapConNetInfo;
    eaClearEx(state->eaMaps, destructor);
    destructor_tpi = CrashedMapConNetInfo;
    eaClearEx(state->eaMapsStuck, destructor);

    destructor_tpi = LauncherConNetInfo;
    eaClearEx(state->eaLaunchers, destructor);

    destructor_tpi = ServerAppConNetInfo;
    eaClearEx(state->eaServerApps, destructor);

    destructor_tpi = EntConNetInfo;
    eaClearEx(state->eaEnts, destructor);
}

int svrMonConnect(ServerMonitorState* state, char* ip_str)
{
    Packet* pak;
    static bool inited = false;

    if (!inited)
    {
        sockStart();
        packetStartup(0, 0);
        inited = true;
    }

    svrMonClearAllLists(state);

    if (!netConnect(&state->db_link, ip_str, DEFAULT_SVRMON_PORT, NLT_TCP, 5, NULL))
    {
        return 0;
    }

    state->db_link.userData = state;

    netLinkSetMaxBufferSize(&state->db_link, BothBuffers, 1 * 1024 * 1024); // Set max size to auto-grow to
    netLinkSetBufferSize(&state->db_link, BothBuffers, 64 * 1024);
    // netLinkSetBufferSize(&state->db_link, BothBuffer, 256*1024);
    pak = pktCreateEx(&state->db_link, DBSVRMON_CONNECT);
    pktSendBits(pak, 32, SVRMON_PROTOCOL_MAJOR_VERSION);
    pktSendBits(pak, 32, SVRMON_PROTOCOL_MINOR_VERSION);
    // No longer CRC these
    //    pktSendBits(pak, 32, DBSERVER_PROTOCOL_VERSION);
    //    pktSendBits(pak, 32, ParserCRCFromParseInfo(EntConNetInfo));
    //    pktSendBits(pak, 32, ParserCRCFromParseInfo(LauncherConNetInfo));
    //    pktSendBits(pak, 32, ParserCRCFromParseInfo(CrashedMapConNetInfo));
    //    pktSendBits(pak, 32, ParserCRCFromParseInfo(MapConNetInfo));
    pktSend(&pak, &state->db_link);

    svrMonRequest(state, DBSVRMON_REQUESTVERSION);

    lnkFlush(&state->db_link);
    //    received_update = false;
    return 1;
}

int svrMonConnected(ServerMonitorState* state)
{
    return state->db_link.socket > 0;
}

int svrMonConnectionLooksDead(ServerMonitorState* state)
{
    return !svrMonConnected(state) || (svrMonGetNetDelay(state) > 30);
}

int svrMonDisconnect(ServerMonitorState* state)
{
    if (!svrMonConnected(state))
        return 0;
    netSendDisconnect(&state->db_link, 2);
    return 1;
}

int svrMonRequest(ServerMonitorState* state, int msg)
{
    Packet* pak;
    if (!svrMonConnected(state))
        return 0;
    pak = pktCreateEx(&state->db_link, msg);
    pktSend(&pak, &state->db_link);
    lnkFlush(&state->db_link);
    return 1;
}

void svrMonRequestEnts(ServerMonitorState* state, int val)
{
    Packet* pak;
    if (!svrMonConnected(state))
        return;
    pak = pktCreateEx(&state->db_link, DBSVRMON_REQUEST_PLAYERS);
    pktSendBits(pak, 1, !!val);
    pktSend(&pak, &state->db_link);
    lnkFlush(&state->db_link);
    return;
}

int svrMonGetSendRate(ServerMonitorState* state)
{
    return pktRate(&state->db_link.sendHistory);
}

int svrMonGetRecvRate(ServerMonitorState* state)
{
    return pktRate(&state->db_link.recvHistory);
}

int svrMonGetNetDelay(ServerMonitorState* state)
{
    if (!state->db_link.connected)
        return 0;
    return timerCpuSeconds() - state->db_link.lastRecvTime;
}

int svrMonRequestDiff(ServerMonitorState* state)
{
    Packet* pak;
    if (!svrMonConnected(state))
        return 0;
    pak = pktCreateEx(&state->db_link, DBSVRMON_REQUESTDIFF);
    pktSend(&pak, &state->db_link);
    lnkFlush(&state->db_link);
    return 1;
}

int svrMonResetMission(ServerMonitorState* state)
{
    svrMonSendDbMessage(state, "MSLinkReset", "");
    lnkBatchSend(&state->db_link);
    return 1;
}

int svrMonShutdownAll(ServerMonitorState* state, const char* reason)
{
    if (!svrMonConnected(state))
        return 0;
    svrMonSendDbMessage(state, "Shutdown", reason);
    lnkFlush(&state->db_link);
    return 1;
}

typedef int (*ContainerFilter)(DbContainer* con, void* filterData); // Returns 1 if it passes the filter

// Macro to handle automatic casting
#define HandleRecvList(state, pak, eaCons, tpi, ptpi, size, eaConsFiltered, filter, filterData)                                                                \
    handleRecvList(state, pak, (DbContainer***)eaCons, tpi, ptpi, size, (DbContainer***)eaConsFiltered, (ContainerFilter)filter, filterData)

void handleRecvList(ServerMonitorState* state, Packet* pak, DbContainer*** eaCons, TokenizerParseInfo* tpi, TokenizerParseInfo** ptpi, int size,
                    DbContainer*** eaConsFiltered, ContainerFilter filter, void* filterData)
{
    DbContainer* con;
    bool full_update;
    int id;
    int index;
    StashTable htIds = 0;
    U32 server_time_offset;

    state->last_received = timerSecondsSince2000();
    server_time_offset = pktGetBits(pak, 32);
    timerSetSecondsOffset(server_time_offset); // Assume the server time is the same as the local time

    full_update = pktGetBits(pak, 1);

    if (full_update)
    {
        tpi = sdUnpackParseInfo(tpi, pak, false);
        *ptpi = tpi;

        if (eaConsFiltered)
        {
            clearConsFromFilterList(eaCons, eaConsFiltered);
        }
        if (eaCons == (DbContainer***)state->eaMapsStuck)
        {
            // HACK for a full update on stuck maps, it might already contain pointers to cons in the eaMaps structure
            clearConsFromFilterList((DbContainer***)state->eaMaps, (DbContainer***)state->eaMapsStuck);
            // Then free the remaining ones (actual crashed maps)
        }
        destructor_tpi = tpi;
        eaClearEx(eaCons, destructor);
        // This should happen implicitly from the function call above
        // if (eaConsFiltered) {
        //    eaSetSize(eaConsFiltered, 0);
        //}
    }
    else
    {
        int i;

        tpi = *ptpi;

        // Hash all of the IDs for quick lookup
        htIds = stashTableCreateInt((int)eaSize(eaCons) * 1.5);
        for (i = 0; i < eaSize(eaCons); i++)
        {
            con = (DbContainer*)eaGet(eaCons, i);
            if (con)
            {
                stashIntAddPointer(htIds, con->id, con, false);
            }
        }
    }

    id = pktGetBitsPack(pak, 3);
    while (id)
    {
        bool update;
        bool meets_filter = false;

        assert(tpi); // If we're getting data, we better have a descriptor!

        if (full_update)
        {
            con = (DbContainer*)calloc(size, 1);
            con->id = id;
            update = false;
        }
        else
        {
            if (!stashIntFindPointer(htIds, id, &con))
                con = NULL;
            assert(!con || con->id == id);
            if (con)
            {
                update = true;
            }
            else
            {
                con = (DbContainer*)calloc(size, 1);
                con->id = id;
                update = false;
            }
        }
        g_someDataOutOfSync |= !sdUnpackDiff(tpi, pak, con, NULL, false);
        if (!update)
        {
            eaPush(eaCons, con);
        }
        // Check filter
        if (filter)
        {
            meets_filter = filter(con, filterData);
        }
        if (meets_filter)
        {
            if (eaConsFiltered)
            {
                index = eaFind(eaConsFiltered, con);
                if (index == -1)
                {
                    // Was not previously in the filter list
                    eaPush(eaConsFiltered, con);
                }
                else
                {
                    // Already in the EArray, this must be an update
                    assert(update);
                }
            }
        }
        else
        {
            // Doesn't meet the filter, remove it if it's in either
            if (eaConsFiltered)
            {
                eaRemove(eaConsFiltered, eaFind(eaConsFiltered, con));
            }
        }

        id = pktGetBitsPack(pak, 3);
    }

    // Receive deletes
    id = pktGetBitsPack(pak, 1);
    while (id)
    {
        bool update;
        int i;
        con = NULL;
        for (i = 0; i < eaSize(eaCons); i++)
        {
            con = (DbContainer*)eaGet(eaCons, i);
            if (con && con->id == id)
                break;
        }
        if (con && con->id == id)
        {
            update = true;
            eaRemove(eaCons, i);
            // Remove from filtered list
            if (eaConsFiltered)
            {
                eaRemove(eaConsFiltered, eaFind(eaConsFiltered, con));
            }

            destructor_tpi = tpi;
            destructor(con);
        }
        else
        {
            assert(!"Deleting something never received!");
        }
        id = pktGetBitsPack(pak, 1);
    }

    if (htIds)
    {
        stashTableDestroy(htIds);
    }
    timerSetSecondsOffset(0); // Reset
}

static char* notTroubleStatii = "CRASHED DELINKING... Delinked Killed";
int notTroubleStatus(char* status)
{
    if (!status || !status[0])
        return 0;
    return !!strstri(notTroubleStatii, status);
}

int inTroubleFilter(MapCon* con, void* junk, ServerStats* stats)
{
    bool trouble = false;
    if (!stats)
    {
        static ServerStats dummy_stats;
        stats = &dummy_stats;
    }
    if (!con->starting && con->seconds_since_update >= 15 && con->seconds_since_update < 120 && !notTroubleStatus(con->status))
    {
        strcpy(con->status, "STUCK");
        stats->sms_stuck_count++;
        trouble = true;
    }
    else if (con->starting && con->seconds_since_update >= 120 && !notTroubleStatus(con->status))
    {
        strcpy(con->status, "STUCK STARTING");
        stats->sms_stuck_starting_count++;
        trouble = true;
    }
    else if (!con->starting && con->seconds_since_update >= 120 && !notTroubleStatus(con->status))
    {
        strcpy(con->status, "TROUBLE");
        stats->sms_stuck_count++;
        trouble = true;
    }
    else if (con->long_tick >= 1200 && con->num_players > 2 && !notTroubleStatus(con->status))
    {
        int dt = (int)timerSecondsSince2000() - (int)con->on_since;
        if (dt > 60)
        { // ignore the first minute
            strcpy(con->status, "LONG TICK");
            stats->sms_long_tick_count++;
            trouble = true;
        }
    }
    else if (stricmp(con->status, "CRASHED") == 0)
    {
        stats->sms_crashed_count++;
    }
    if (trouble)
        return 1;
    return 0;
}

int inTroubleFilterSA(ServerAppCon* con, void* junk, ServerStats* stats)
{
    bool trouble = false;
    if (con->crashed)
    {
        if (stricmp(con->status, "Killed") != 0)
            strcpy(con->status, "CRASHED");
        stats->sa_crashed_count++;
        trouble = true;
    }
    else if (con->remote_process_info.process_id)
    {
        if (stricmp(con->status, "Killed") != 0)
            strcpy(con->status, "Running");
    }
    else if (con->monitor)
    {
        strcpy(con->status, "Not Running");
    }
    else
    {
        strcpy(con->status, "Starting");
    }
    if (trouble)
        return 1;
    return 0;
}

int stuckFilter(MapCon* con, void* junk)
{
    int trouble = inTroubleFilter(con, junk, NULL); // Set the status field
    return trouble;
    //    if (con->seconds_since_update >= 15 && !con->starting || con->seconds_since_update >= 120)
    //        return 1;
    //    return 0;
}

void updateInTroubleState(ServerMonitorState* state)
{
    int i;
    int trouble = 0;
    state->stats.sms_long_tick_count = 0;
    state->stats.sms_stuck_count = 0;
    state->stats.sms_stuck_starting_count = 0;
    state->stats.sa_crashed_count = 0;
    state->stats.sms_crashed_count = 0;
    for (i = 0; i < eaSize(state->eaMapsStuck); i++)
    {
        if (inTroubleFilter(eaGet(state->eaMapsStuck, i), NULL, &state->stats))
        {
            trouble++;
        }
    }
    for (i = 0; i < eaSize(state->eaServerApps); i++)
    {
        if (inTroubleFilterSA(eaGet(state->eaServerApps, i), NULL, &state->stats))
        {
            trouble++;
        }
    }
    state->stats.servers_in_trouble = trouble;
}

void handleDbStats(ServerMonitorState* state, Packet* pak)
{
    int version = pktGetBitsPack(pak, 1);

    // If you change this function, make sure it's still backwards compatible by connecting to a Live and TraingingRoom DbServer
    // packet sent in svrmoncomm.c

    state->stats.pcount_login = pktGetBitsPack(pak, 10);
    state->stats.pcount_ents = pktGetBitsPack(pak, 10);

    state->stats.sqlwb = pktGetBitsPack(pak, 10);
    state->stats.servermoncount = pktGetBitsPack(pak, 10);
    state->stats.dbticklen = pktGetF32(pak);
    if (version > 5)
        state->stats.arenaSecSinceUpdate = pktGetBitsPack(pak, 10);
    if (version > 6)
        state->stats.statSecSinceUpdate = pktGetBitsPack(pak, 10);
    if (version > 7)
        state->stats.beaconWaitSeconds = pktGetBitsPack(pak, 4);
    if (version > 8)
        state->stats.heroAuctionSecSinceUpdate = pktGetBitsAuto(pak);
    if (version > 9)
        state->stats.villainAuctionSecSinceUpdate = pktGetBitsAuto(pak);
    if (version > 10)
        state->stats.accountSecSinceUpdate = pktGetBitsAuto(pak);
    if (version > 11)
        state->stats.missionSecSinceUpdate = pktGetBitsAuto(pak);
    if (version > 12)
    {
        state->stats.sqlthroughput = pktGetBitsAuto(pak);
        state->stats.sqlavglat = pktGetBitsAuto(pak);
        state->stats.sqlworstlat = pktGetBitsAuto(pak);
        state->stats.loglat = pktGetBitsAuto(pak);

        state->stats.logbytes = pktGetBitsAuto(pak);
        state->stats.logqcnt = pktGetBitsAuto(pak);
        state->stats.logqmax = pktGetBitsAuto(pak);
        state->stats.logsortmem = pktGetBitsAuto(pak);

        state->stats.logsortcap = pktGetBitsAuto(pak);
    }
    if (version > 21)
        state->stats.pcount_queued = pktGetBitsAuto(pak);
    if (version > 22)
        state->stats.queue_connections = pktGetBitsAuto(pak);
    if (version > 23)
    {
        state->stats.sqlforeidleratio = pktGetF32(pak);
        state->stats.sqlbackidleratio = pktGetF32(pak);
    }
    if (version > 25)
    {
        state->stats.turnstileSecSinceUpdate = pktGetBitsAuto(pak);
    }
    // Version 27: added overload protection
    if (version >= 27)
    {
        state->stats.overloadProtection = pktGetBitsAuto(pak);
    }
    else
    {
        state->stats.overloadProtection = -1;
    }
    // Version 28: added total map start requests and delta map start requests since last update
    if (version >= 28)
    {
        int updated_map_start_request_total;
        int delta_requests;

        state->stats.dbserver_stat_time_delta = pktGetBitsAuto(pak);
        updated_map_start_request_total = pktGetBitsAuto(pak);
        state->stats.dbserver_peak_waiting_entities = pktGetBitsAuto(pak);

        delta_requests = updated_map_start_request_total - state->stats.dbserver_map_start_request_total;
        state->stats.dbserver_map_start_request_total = updated_map_start_request_total;
        if (state->stats.dbserver_stat_time_delta > 0)
        {
            state->stats.dbserver_avg_map_request_rate = (delta_requests * 1000.0f) / state->stats.dbserver_stat_time_delta;
        }
    }
}

int svrMonHandleMsg(Packet* pak, int cmd, NetLink* link)
{
    ServerMonitorState* state = link->userData;
    if (!state)
    {
        assert(state);
        return 0;
    }
    timestamp = timerCpuTicks();
    switch (cmd)
    {
        case DBSVRMON_MAPSERVERS:
            // received_update = true;
            HandleRecvList(state, pak, state->eaMaps, MapConNetInfo, &state->tpiMapConNetInfo, sizeof(MapCon), state->eaMapsStuck, stuckFilter, NULL);
            updateInTroubleState(state);
            break;
        case DBSVRMON_CRASHEDMAPSERVERS:
            // received_update = true;
            HandleRecvList(state, pak, state->eaMapsStuck, CrashedMapConNetInfo, &state->tpiCrashedMapConNetInfo, sizeof(MapCon), NULL, NULL, NULL);
            updateInTroubleState(state);
            break;
        case DBSVRMON_PLAYERS:
            // received_update = true;
            HandleRecvList(state, pak, state->eaEnts, EntConNetInfo, &state->tpiEntConNetInfo, sizeof(EntCon), NULL, NULL, NULL);
            break;
        case DBSVRMON_LAUNCHERS:
            // received_update = true;
            HandleRecvList(state, pak, state->eaLaunchers, LauncherConNetInfo, &state->tpiLauncherConNetInfo, sizeof(LauncherCon), NULL, NULL, NULL);
            break;
        case DBSVRMON_SERVERAPPS:
            // received_update = true;
            HandleRecvList(state, pak, state->eaServerApps, ServerAppConNetInfo, &state->tpiServerAppConNetInfo, sizeof(ServerAppCon), NULL, NULL, NULL);
            updateInTroubleState(state);
            break;
        case DBSVRMON_REQUESTVERSION:
            strcpy(state->stats.gameversion, pktGetString(pak));
            strcpy(state->stats.serverversion, pktGetString(pak));
            break;
        case DBSVRMON_DBSTATS:
            handleDbStats(state, pak);
            break;
        case DBSVRMON_CONNECT:
            if (!pktGetBits(pak, 1))
            {
                // Version check failed!
                int crc_num = pktGetBitsPack(pak, 1);
                U32 server_crc = pktGetBits(pak, 32);
                U32 my_crc = pktGetBits(pak, 32);
                char err_buf[1024];
                svrMonDisconnect(state);
                if (crc_num <= 1)
                {
                    sprintf(err_buf, "Error connecting to DbServer, protocol version %d does not match:\n  Server: %d\n  Client: %d", crc_num, server_crc,
                            my_crc);
                }
                else
                {
                    sprintf(err_buf, "Error connecting to DbServer, network parse table (%d) CRCs do not match:\n  Server: %08x\n  Client: %08x", crc_num,
                            server_crc, my_crc);
                }
                MessageBoxA(NULL, err_buf, "Error", MB_ICONWARNING);
            }
            else
            {
                // Connected fine
                //                if (!received_update) {
                //                    svrMonRequest(state, DBSVRMON_REQUEST);
                //                }
            }
            break;
        default:
            // assert(0);
            return 0;
    }
    return 1;
}

int svrMonNetTick(ServerMonitorState* state)
{
    lnkFlushAll();
    netLinkMonitor(&state->db_link, 0, svrMonHandleMsg);

    return 1;
}

void svrMonSendDbMessage(ServerMonitorState* state, const char* msg, const char* params)
{
    Packet* pak;
    if (!svrMonConnected(state) || !msg || !msg[0] || !params)
        return;
    pak = pktCreateEx(&state->db_link, DBSVRMON_RELAYMESSAGE);
    pktSendString(pak, msg);
    pktSendString(pak, params);
    pktSend(&pak, &state->db_link);
}

void svrMonSendAdminMessage(ServerMonitorState* state, const char* msg)
{
    if (!svrMonConnected(state) || !msg || !msg[0])
        return;
    svrMonSendDbMessage(state, "AdminChat", msg);
}

void svrMonSendOverloadProtection(ServerMonitorState* state, const char* msg)
{
    if (!svrMonConnected(state) || !msg || !msg[0])
        return;
    svrMonSendDbMessage(state, "OverloadProtection", msg);
}

void svrMonDelink(ServerMonitorState* state, MapCon* con)
{
    if (con)
    {
        char buf[256];
        svrMonSendDbMessage(state, "Delink", itoa(con->id, buf, 10));
        lnkBatchSend(&state->db_link);
    }
}

void killByIP(NetLink* link, U32 ip, U32 pid)
{
    char temp[MAX_PATH];
    Packet* pak = pktCreateEx(link, DBSVRMON_EXEC);
    pktSendBits(pak, 32, ip);
    sprintf(temp, "TASKKILL /F /PID %d", pid);
    pktSendString(pak, temp);
    pktSend(&pak, link);
    lnkBatchSend(link);
    // In case they don't have TASKKILL.EXE try plain old kill
    pak = pktCreateEx(link, DBSVRMON_EXEC);
    pktSendBits(pak, 32, ip);
    sprintf(temp, "KILL %d", pid);
    pktSendString(pak, temp);
    pktSend(&pak, link);
    lnkBatchSend(link);
}

typedef BOOL(__stdcall* DisableFsRedirectionProc)(PVOID*);
typedef BOOL(__stdcall* RevertFsRedirectionProc)(PVOID);

static int strdiff(const char* str1, const char* str2)
{
    int ret = 0;
    const char *s1, *s2;
    bool innumber = false;
    if (strlen(str1) != strlen(str2))
        return 999;
    for (s1 = str1, s2 = str2; *s1; s1++, s2++)
    {
        if (*s1 != *s2)
        {
            int diff = *s1 - *s2;
            ret *= 10;
            ret += diff;
            innumber = true;
        }
        else if (isdigit((unsigned char)*s1) && innumber)
        {
            ret *= 10;
        }
        else
        {
            innumber = false;
        }
    }
    return ret;
}
