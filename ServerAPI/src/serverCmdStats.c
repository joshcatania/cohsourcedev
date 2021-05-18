#include <utilitieslib/utils/utils.h>
#include <utilitieslib/components/earray.h>
#include <utilitieslib/utils/timing.h>
#include <utilitieslib/utils/mathutil.h>
#include <container.h>
#include <utilitieslib/utils/textparser.h>
#include <utilitieslib/utils/tokenstore.h>

#include "serverCmdStats.h"
#include "serverMonitorNet.h"
#include "json.h"
#include "launcher_common.h"

TokenizerParseInfo ServerStatsDispInfo2[] = {
    {"DBTrbl", TOK_INT(ServerStats, dbserver_in_trouble, 0), 0, TOK_FORMAT_LVWIDTH(40)},
    {"MSTrbl", TOK_INT(ServerStats, servers_in_trouble, 0), 0, TOK_FORMAT_LVWIDTH(40)},
    {"StuckMapservers", TOK_INT(ServerStats, smscount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#Playing", TOK_INT(ServerStats, pcount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#LoggingIn", TOK_INT(ServerStats, pcount_login, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#Queued", TOK_INT(ServerStats, pcount_queued, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#Xfering", TOK_MINBITS(6) | TOK_INT(ServerStats, pcount_connecting, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#Heroes", TOK_INT(ServerStats, pcount_hero, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#Villains", TOK_INT(ServerStats, pcount_villain, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#QueueConns", TOK_INT(ServerStats, queue_connections, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"SQLWBDepth", TOK_INT(ServerStats, sqlwb, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"SQLThroughput", TOK_INT(ServerStats, sqlthroughput, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"SQLAvgLat", TOK_INT(ServerStats, sqlavglat, 0), 0, TOK_FORMAT_MICROSECONDS | TOK_FORMAT_LVWIDTH(65)},
    {"SQLWorstLat", TOK_INT(ServerStats, sqlworstlat, 0), 0, TOK_FORMAT_MICROSECONDS | TOK_FORMAT_LVWIDTH(65)},
    {"SQLForeIdleRatio", TOK_F32(ServerStats, sqlforeidleratio, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
    {"SQLBackIdleRatio", TOK_F32(ServerStats, sqlbackidleratio, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
    {"LogLatency", TOK_INT(ServerStats, loglat, 0), 0, TOK_FORMAT_MICROSECONDS | TOK_FORMAT_LVWIDTH(65)},
    {"LogBytes", TOK_AUTOINT(ServerStats, logbytes, 0), 0, TOK_FORMAT_BYTES | TOK_FORMAT_LVWIDTH(65)},
    {"LogQueueCount", TOK_INT(ServerStats, logqcnt, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"LogQueueMax", TOK_INT(ServerStats, logqmax, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"LogSortMem", TOK_AUTOINT(ServerStats, logsortmem, 0), 0, TOK_FORMAT_BYTES | TOK_FORMAT_LVWIDTH(65)},
    {"LogSortMemCap", TOK_AUTOINT(ServerStats, logsortcap, 0), 0, TOK_FORMAT_BYTES | TOK_FORMAT_LVWIDTH(65)},
    {"DbTickLen", TOK_F32(ServerStats, dbticklen, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"Launchers", TOK_INT(ServerStats, lcount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"ChatTrbl", TOK_INT(ServerStats, chatserver_in_trouble, 0), 0, TOK_FORMAT_LVWIDTH(40)},
    {"SecondsSinceUpdate", TOK_INT(ServerStats, secondsSinceDbUpdate, 0)},
    {"ArenaSecSinceUpdate", TOK_INT(ServerStats, arenaSecSinceUpdate, 0)},
    {"StatSecSinceUpdate", TOK_INT(ServerStats, statSecSinceUpdate, 0)},
    {"BeaconWait", TOK_INT(ServerStats, beaconWaitSeconds, 0)},
    {"AvgCPU", TOK_F32(ServerStats, avgCpu, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
    {"AvgCPU60", TOK_F32(ServerStats, avgCpu60, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
    {"MaxCPU", TOK_F32(ServerStats, maxCpu, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
    {"MaxCPU60", TOK_F32(ServerStats, maxCpu60, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
    {"TotalPhysUsed", TOK_INT(ServerStats, totalPhysUsed, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"TotalVirtUsed", TOK_INT(ServerStats, totalVirtUsed, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"MinVirtAvail", TOK_INT(ServerStats, minVirtAvail, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"MinPhysAvail", TOK_INT(ServerStats, minPhysAvail, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"AvgPhysAvail", TOK_INT(ServerStats, avgPhysAvail, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"AvgVirtAvail", TOK_INT(ServerStats, avgVirtAvail, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"MaxPhysAvail", TOK_INT(ServerStats, maxPhysAvail, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"MaxVirtAvail", TOK_INT(ServerStats, maxVirtAvail, 0), 0, TOK_FORMAT_KBYTES | TOK_FORMAT_LVWIDTH(65)},
    {"ServerApps", TOK_INT(ServerStats, sacount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"MapServers", TOK_INT(ServerStats, mscount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"StaticMS", TOK_INT(ServerStats, mscount_static, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"BaseMS", TOK_INT(ServerStats, mscount_base, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"MissionMS", TOK_INT(ServerStats, mscount_missions, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#EntsLoaded", TOK_INT(ServerStats, pcount_ents, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#Ents", TOK_INT(ServerStats, ecount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#Monsters", TOK_INT(ServerStats, mcount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#MSCrashed", TOK_INT(ServerStats, sms_crashed_count, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#MSLongTick", TOK_INT(ServerStats, sms_long_tick_count, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#MSStuck", TOK_INT(ServerStats, sms_stuck_count, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#MSStuckStarting", TOK_INT(ServerStats, sms_stuck_starting_count, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"#SACrashed", TOK_INT(ServerStats, sa_crashed_count, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"MaxCrashedMaps", TOK_INT(ServerStats, maxCrashedMaps, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"MaxSecondsSinceUpdate", TOK_INT(ServerStats, maxSecondsSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"ServerMonitors", TOK_INT(ServerStats, servermoncount, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"AutoDelinkTime", TOK_INT(ServerStats, autodelinktime, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"AutoDelinkEnabled", TOK_BOOL(ServerStats, autodelink, 0), 0, TOK_FORMAT_LVWIDTH(60)},
    {"ClientVersion", TOK_FIXEDSTR(ServerStats, gameversion), 0, TOK_FORMAT_LVWIDTH(120)},
    {"ServerVersion", TOK_FIXEDSTR(ServerStats, serverversion), 0, TOK_FORMAT_LVWIDTH(120)},

    /*	// Don't really care about this raw data visually
        { "ProcMonDbServer",	TOK_POINTER,	offsetof(ServerStats, dbServerMonitor), sizeof(ProcessMonitorEntry), ProcessMonitorInfo},
        { "ProcMonLauncher",	TOK_POINTER,	offsetof(ServerStats, launcherMonitor), sizeof(ProcessMonitorEntry), ProcessMonitorInfo},
    */
    {"DbServer.exe", TOK_FIXEDSTR(ServerStats, dbServerProcessStatus), 0, TOK_FORMAT_LVWIDTH(80)},
    //	{ "DbServerCrashes",	TOK_POINTER,	offsetof(ServerStats, dbServerMonitor), sizeof(ProcessMonitorEntry), ProcessMonitorCrashInfo},
    {"Launcher.exe", TOK_FIXEDSTR(ServerStats, launcherProcessStatus), 0, TOK_FORMAT_LVWIDTH(80)},
    //	{ "LauncherCrashes",	TOK_POINTER,	offsetof(ServerStats, launcherMonitor), sizeof(ProcessMonitorEntry), ProcessMonitorCrashInfo},

    {"ChatSvrConnected", TOK_INT(ServerStats, chatServerConnected, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"ChatTotalUsers", TOK_INT(ServerStats, chatTotalUsers, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"ChatOnlineUsers", TOK_INT(ServerStats, chatOnlineUsers, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"ChatChannels", TOK_INT(ServerStats, chatChannels, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"ChatSecSinceUpdate", TOK_INT(ServerStats, chatSecSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"ChatLinks", TOK_INT(ServerStats, chatLinks, 0), 0, TOK_FORMAT_LVWIDTH(80)},

    {"Configured IP", TOK_INT(ServerStats, ip, 0), 0, TOK_FORMAT_IP | TOK_FORMAT_LVWIDTH(100)},
    {"Hero Auction", TOK_INT(ServerStats, heroAuctionSecSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"Villain Auction", TOK_INT(ServerStats, villainAuctionSecSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"Account Server", TOK_INT(ServerStats, accountSecSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"Acount Server", TOK_REDUNDANTNAME | TOK_INT(ServerStats, accountSecSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"Mission Server", TOK_INT(ServerStats, missionSecSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"Turnstile Server", TOK_INT(ServerStats, turnstileSecSinceUpdate, 0), 0, TOK_FORMAT_LVWIDTH(80)},
    {"Overload Protection", TOK_INT(ServerStats, overloadProtection, 0), 0, TOK_FORMAT_LVWIDTH(80)},

    {0}};

// yes this is awful but it works
char* fixHeader(const char* in)
{
    static char dumb[256], *o;
    size_t l, i;

    o = dumb;
    l = strlen(in);
    for (i = 0; i < l; i++)
    {
        if (in[i] == '#')
        {
            *(o++) = 'n';
            *(o++) = 'u';
            *(o++) = 'm';
        }
        else if (in[i] == ' ' || in[i] == '.' || in[i] == '/')
            *(o++) = '_';
        else
            *(o++) = in[i];
    }
    *o = 0;
    return dumb;
}

char* getValue(void* data, TokenizerParseInfo* pti, int col, bool quotestr)
{
    static char buf[256];
    const char* quote = "";
    if (quotestr)
        quote = "\"";

    if (TOK_GET_TYPE(pti[col].type) == TOK_INT_X && TOK_GET_FORMAT_OPTIONS(pti[col].format) == TOK_FORMAT_IP)
    {
        int ip = TokenStoreGetInt(pti, col, data, 0);
        snprintf_s(buf, 256, 255, "%s%d.%d.%d.%d%s", quote, ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, quote);
    }
    else if (TOK_GET_TYPE(pti[col].type) == TOK_INT_X && TOK_GET_FORMAT_OPTIONS(pti[col].format) == TOK_FORMAT_FRIENDLYSS2000)
    {
        char datestr[20];
        timerMakeDateStringFromSecondsSince2000_s(datestr, sizeof(datestr), TokenStoreGetInt(pti, col, data, 0));
        snprintf_s(buf, 256, 255, "%s%s%s", quote, datestr, quote);
    }
    else if (TOK_GET_TYPE(pti[col].type) == TOK_INT_X)
    {
        snprintf_s(buf, 256, 255, "%d", TokenStoreGetInt(pti, col, data, 0));
    }
    else if (TOK_GET_TYPE(pti[col].type) == TOK_STRING_X)
    {
        snprintf_s(buf, 256, 255, "%s%s%s", quote, TokenStoreGetString(pti, col, data, 0), quote);
    }
    else if (TOK_GET_TYPE(pti[col].type) == TOK_F32_X && TOK_GET_FORMAT_OPTIONS(pti[col].format) == TOK_FORMAT_PERCENT)
    {
        snprintf_s(buf, 256, 255, "%d", (int)(TokenStoreGetF32(pti, col, data, 0) * 100));
    }
    else if (TOK_GET_TYPE(pti[col].type) == TOK_F32_X)
    {
        snprintf_s(buf, 256, 255, "%g", TokenStoreGetF32(pti, col, data, 0));
    }
    else if (TOK_GET_TYPE(pti[col].type) == TOK_U8_X)
    {
        snprintf_s(buf, 256, 255, "%d", TokenStoreGetU8(pti, col, data, 0));
    }
    else if (TOK_GET_TYPE(pti[col].type) == TOK_BOOL_X)
    {
        snprintf_s(buf, 256, 255, "%s", TokenStoreGetU8(pti, col, data, 0) ? "true" : "false");
    }
    else
    {
        strcpy(buf, "unknown");
    }

    return buf;
}

void csvHeader(TokenizerParseInfo* pti, bool* comma)
{
    int i;

    for (i = 0; (pti[i].name && pti[i].name[0]) || pti[i].type; i++)
    {
        if (TOK_GET_TYPE(pti[i].type) == TOK_STRUCT_X)
        {
            csvHeader((TokenizerParseInfo*)pti[i].subtable, comma);
        }
        else
        {
            printf("%s%s", *comma ? "," : "", fixHeader(pti[i].name));
            *comma = true;
        }
    }
}

void csvValues(void* data, TokenizerParseInfo* pti, bool* comma)
{
    int i;

    for (i = 0; (pti[i].name && pti[i].name[0]) || pti[i].type; i++)
    {
        if (TOK_GET_TYPE(pti[i].type) == TOK_STRUCT_X)
        {
            if (pti[i].type & TOK_INDIRECT)
            {
                csvValues(TokenStoreGetPointer(pti, i, data, 0), (TokenizerParseInfo*)pti[i].subtable, comma);
            }
            else
            {
                csvValues(data, (TokenizerParseInfo*)pti[i].subtable, comma);
            }
        }
        else
        {
            printf("%s%s", *comma ? "," : "", getValue(data, pti, i, false));
            *comma = true;
        }
    }
}

void genericStatsCsv(void*** data, TokenizerParseInfo* pti)
{
    int r;
    bool comma;

    fputs("# ", stdout);
    comma = false;
    csvHeader(pti, &comma);
    fputs("\n", stdout);

    for (r = 0; r < eaSize(data); r++)
    {
        void* d = (*data)[r];
        comma = false;
        csvValues((*data)[r], pti, &comma);
        fputs("\n", stdout);
    }
}

void jsonEArray(JsonNode* parent, void*** data, TokenizerParseInfo* pti);

void jsonStruct(JsonNode* parent, void* data, TokenizerParseInfo* pti)
{
    int i;

    for (i = 0; (pti[i].name && pti[i].name[0]) || pti[i].type; i++)
    {
        JsonNode* node = jsonNode(fixHeader(pti[i].name), NULL, false, false);
        if (data)
        {
            if (TOK_GET_TYPE(pti[i].type) == TOK_STRUCT_X)
            {
                if (pti[i].type & TOK_EARRAY)
                {
                    node->isarray = true;
                    jsonEArray(node, TokenStoreGetPointer(pti, i, data, 0), (TokenizerParseInfo*)pti[i].subtable);
                }
                else if (pti[i].type & TOK_INDIRECT)
                {
                    jsonStruct(node, TokenStoreGetPointer(pti, i, data, 0), (TokenizerParseInfo*)pti[i].subtable);
                }
                else
                {
                    jsonStruct(node, data, (TokenizerParseInfo*)pti[i].subtable);
                }
            }
            else
            {
                node->value = _strdup(getValue(data, pti, i, true));
            }
        }
        eaPush(&parent->children, node);
    }
}

void jsonEArray(JsonNode* parent, void*** data, TokenizerParseInfo* pti)
{
    int r, sz;
    JsonNode* node;

    sz = eaSize(data);
    for (r = 0; r < sz; r++)
    {
        node = jsonNode(NULL, NULL, false, false);
        jsonStruct(node, (*data)[r], pti);
        eaPush(&parent->children, node);
    }
}

void genericStatsJson(JsonNode* parent, const char* name, void*** data, TokenizerParseInfo* pti)
{
    JsonNode* node = jsonNode(fixHeader(name), NULL, false, true);
    jsonEArray(node, data, pti);
    eaPush(&parent->children, node);
}

void genericStats(JsonNode* parent, ServerMonitorState* state, const char* name, void*** data, TokenizerParseInfo* pti)
{
    if (parent)
        genericStatsJson(parent, name, data, pti);
    else
        genericStatsCsv(data, pti);
}

void serverCmdLauncherStats(ServerMonitorState* state, JsonNode* parent)
{
    genericStats(parent, state, "launchers", state->eaLaunchers, state->tpiLauncherConNetInfo);
}

void serverCmdMapStats(ServerMonitorState* state, JsonNode* parent)
{
    genericStats(parent, state, "maps", state->eaMaps, state->tpiMapConNetInfo);
}

void serverCmdUpdateDbStats(ServerMonitorState* state)
{
    int i;
    int count;
    state->stats.mscount_base = 0;
    state->stats.mscount_static = 0;
    state->stats.mscount_missions = 0;
    state->stats.mscount = eaSize(state->eaMaps);
    state->stats.smscount = eaSize(state->eaMapsStuck);
    state->stats.lcount = eaSize(state->eaLaunchers);
    state->stats.lcount_suspended = 0;
    state->stats.lcount_suspended_manually = 0;
    state->stats.lcount_suspended_trouble = 0;
    state->stats.lcount_suspended_capacity = 0;
    state->stats.sacount = eaSize(state->eaServerApps);
    state->stats.pcount = 0;
    state->stats.pcount_connecting = 0;
    state->stats.pcount_hero = 0;
    state->stats.pcount_villain = 0;
    state->stats.ecount = 0;
    state->stats.mcount = 0;
    state->stats.maxSecondsSinceUpdate = 0;
    state->stats.maxCrashedMaps = 0;
    state->stats.maxCrashedLaunchers = 0;
    for (i = 0; i < eaSize(state->eaLaunchers); i++)
    {
        LauncherCon* launcher = state->eaLaunchers_data[i];
        launcher->num_mapservers = 0;
        launcher->num_crashed_mapservers = 0;
        state->stats.maxCrashedLaunchers = MAX(state->stats.maxCrashedLaunchers, launcher->delinks);
        if (launcher->suspension_flags)
        {
            state->stats.lcount_suspended++;
            // launcher can be in multiple suspension states
            // count all of them but use the order below to choose the row color
            if (launcher->suspension_flags & kLaunchSuspensionFlag_Capacity)
            {
                state->stats.lcount_suspended_capacity++;
            }
            if (launcher->suspension_flags & kLaunchSuspensionFlag_Trouble)
            {
                state->stats.lcount_suspended_trouble++;
            }
            if ((launcher->suspension_flags & kLaunchSuspensionFlag_Manual) || (launcher->suspension_flags & kLaunchSuspensionFlag_ServerMonitor))
            {
                state->stats.lcount_suspended_manually++;
            }
        }
    }

    for (i = 0; i < eaSize(state->eaMaps); i++)
    {
        MapCon* map = state->eaMaps_data[i];
        if (map)
        {
            bool foundit = false;
            int j;
            state->stats.ecount += map->num_ents;
            state->stats.mcount += map->num_monsters;
            state->stats.pcount += map->num_players;
            state->stats.pcount_hero += map->num_hero_players;
            state->stats.pcount_villain += map->num_villain_players;
            state->stats.pcount_connecting += map->num_players_connecting;
            state->stats.maxSecondsSinceUpdate = MAX(state->stats.maxSecondsSinceUpdate, map->seconds_since_update);
            if (map->is_static)
            {
                state->stats.mscount_static++;
            }
            else if (strStartsWith(map->map_name, "Base"))
            {
                state->stats.mscount_base++;
            }
            else if (strStartsWith(map->map_name, "maps/Missions"))
            {
                state->stats.mscount_missions++;
            }
            else
            {
                state->stats.mscount_static++;
            }
            for (j = 0; j < eaSize(state->eaLaunchers); j++)
            {
                LauncherCon* launcher = state->eaLaunchers_data[j];
                U32 laddr = launcher->link->addr.sin_addr.S_un.S_addr;
                if (laddr == map->ip_list[0] || laddr == map->ip_list[1])
                {
                    // assert(!foundit);
                    if (foundit)
                        break; // Two launchers with the same IP?!
                    foundit = true;
                    launcher->num_mapservers++;
                }
            }
        }
    }

    state->stats.avgCpu = 0;
    state->stats.avgCpu60 = 0;
    state->stats.maxCpu = 0;
    state->stats.maxCpu60 = 0;
    state->stats.totalPhysUsed = 0;
    state->stats.totalVirtUsed = 0;
    state->stats.minPhysAvail = 1 << 31;
    state->stats.minVirtAvail = 1 << 31;
    state->stats.maxPhysAvail = 0;
    state->stats.maxVirtAvail = 0;
    state->stats.avgPhysAvail = 0;
    state->stats.avgVirtAvail = 0;
    count = 0;
    for (i = 0; i < eaSize(state->eaLaunchers); i++)
    {
        LauncherCon* launcher = state->eaLaunchers_data[i];
        count++;
        state->stats.avgCpu += launcher->cpu_usage / 100.;
        state->stats.avgCpu60 += launcher->remote_process_info.cpu_usage60;
        state->stats.maxCpu = MAX(state->stats.maxCpu, launcher->cpu_usage / 100.);
        state->stats.maxCpu60 = MAX(state->stats.maxCpu60, launcher->remote_process_info.cpu_usage60);
        state->stats.totalPhysUsed += launcher->remote_process_info.mem_used_phys;
        state->stats.totalVirtUsed += launcher->remote_process_info.mem_used_virt;
        state->stats.minPhysAvail = MIN(state->stats.minPhysAvail, launcher->mem_avail_phys);
        state->stats.minVirtAvail = MIN(state->stats.minVirtAvail, launcher->mem_avail_virt);
        state->stats.maxPhysAvail = MAX(state->stats.maxPhysAvail, launcher->mem_avail_phys);
        state->stats.maxVirtAvail = MAX(state->stats.maxVirtAvail, launcher->mem_avail_virt);
        state->stats.avgPhysAvail += launcher->mem_avail_phys;
        state->stats.avgVirtAvail += launcher->mem_avail_virt;
    }
    if (count)
    {
        state->stats.avgCpu /= (F32)count;
        state->stats.avgCpu60 /= (F32)count;
        state->stats.avgPhysAvail /= count;
        state->stats.avgVirtAvail /= count;
    }
    else
    {
        state->stats.minPhysAvail = 0;
        state->stats.minVirtAvail = 0;
    }

    state->stats.secondsSinceDbUpdate = svrMonGetNetDelay(state);
}

void serverCmdDbStats(ServerMonitorState* state, JsonNode* parent)
{
    if (parent)
    {
        JsonNode* node = jsonNode("dbserver", NULL, false, false);
        jsonStruct(node, &state->stats, ServerStatsDispInfo2);
        eaPush(&parent->children, node);
    }
    else
    {
        ServerStats** starr = NULL;
        eaPush(&starr, &state->stats);
        genericStatsCsv(&starr, ServerStatsDispInfo2);
        eaDestroy(&starr);
    }
}

void serverCmdEntities(ServerMonitorState* state, JsonNode* parent)
{
    genericStats(parent, state, "entities", state->eaEnts, state->tpiEntConNetInfo);
}
