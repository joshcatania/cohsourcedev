#ifndef _SERVERCMD_H
#define _SERVERCMD_H

#include <utilitieslib\utils\textparser.h>
#include <utilitieslib\network\net_structdefs.h>
#include <utilitieslib\components\EString.h>

enum ServerStat
{
    STATS_DB = 0x01,
    STATS_MAP = 0x02,
    STATS_LAUNCHER = 0x04,
    STATS_ENTITIES = 0x08,
};

typedef struct MapCon MapCon;
typedef struct LauncherCon LauncherCon;
typedef struct ServerAppCon ServerAppCon;
typedef struct EntCon EntCon;
typedef struct ProcessMonitorEntry ProcessMonitorEntry;

typedef struct ServerStats
{
    int dbserver_in_trouble;
    int chatserver_in_trouble;
    int arenaserver_in_trouble;
    int servers_in_trouble;
    int sms_crashed_count;
    int sms_long_tick_count;
    int sms_stuck_count;
    int sms_stuck_starting_count;
    int sa_crashed_count;
    int mscount;
    int smscount;
    int lcount;
    int lcount_suspended;
    int lcount_suspended_manually;
    int lcount_suspended_trouble;
    int lcount_suspended_capacity;
    int sacount;
    int pcount;
    int pcount_ents;
    int pcount_connecting;
    int pcount_login;
    int pcount_queued;
    int pcount_hero;
    int pcount_villain;
    int queue_connections;
    int ecount;
    int mcount;
    int servermoncount;
    int autodelinktime;
    bool autodelink;
    int sqlwb;
    int sqlthroughput;
    int sqlavglat;
    int sqlworstlat;
    float sqlforeidleratio;
    float sqlbackidleratio;
    int loglat;
    U32 logbytes;
    int logqcnt;
    int logqmax;
    U32 logsortmem;
    U32 logsortcap;
    F32 dbticklen;
    int mscount_static;
    int mscount_base;
    int mscount_missions;

    int secondsSinceDbUpdate; // Seconds since the DbServer talked to the ServerMonitor

    // aggregate launcher stats
    F32 avgCpu;
    F32 avgCpu60;
    F32 maxCpu;
    F32 maxCpu60;
    U32 totalPhysUsed;
    U32 totalVirtUsed;
    U32 minPhysAvail;
    U32 minVirtAvail;
    U32 avgPhysAvail;
    U32 avgVirtAvail;
    U32 maxPhysAvail;
    U32 maxVirtAvail;
    int maxCrashedMaps;
    int maxCrashedLaunchers;

    // aggregate mapserver stats
    int maxSecondsSinceUpdate;

    char gameversion[128];
    char serverversion[128];

    ProcessMonitorEntry* dbServerMonitor;
    char dbServerProcessStatus[128];
    ProcessMonitorEntry* launcherMonitor;
    char launcherProcessStatus[128];

    // Only used by ShardMonitor
    U32 ip;
    char name[128];
    char status[128];
    NetLink link;
    int connected;
    int reconnect_countdown;
    int secondsSinceUpdate; // Seconds since the ServerMonitor talked to the ShardMonitor

    // ShardMonitor CmdRelay stuff
    int ds_relays;
    int ms_relays;
    int custom_relays;
    int auth_relays;
    int acct_relays;
    int chat_relays;
    int auc_relays;
    int ma_relays;
    int crashed_mscount;
    char shardrelay_status[128];
    char shardrelay_msg[128];
    bool special; // special statistics rows like MAX/MIN/AVG/etc set this to true
    int maxLastUpdate;

    // chat server info (for shard monitor)
    int chatServerConnected;
    int chatTotalUsers;
    int chatOnlineUsers;
    int chatChannels;
    int chatSecSinceUpdate;
    int chatLinks;

    // Arena server
    int arenaSecSinceUpdate;

    // Stat server
    int statSecSinceUpdate;
    TokenizerParseInfo* tpiServerStatsNetInfo;

    // BeaconServer
    int beaconWaitSeconds;

    // Auction server
    int heroAuctionSecSinceUpdate;
    int villainAuctionSecSinceUpdate;

    int accountSecSinceUpdate;
    int missionSecSinceUpdate;
    int turnstileSecSinceUpdate;
    int overloadProtection;
    int dbserver_map_start_request_total; // total number of map launches that have been requested of dbserver
    int dbserver_stat_time_delta;         // number of milliseconds elapsed on dbserver since constructing this and previous stats packet (or zero)
    F32 dbserver_avg_map_request_rate;    // calculated from the above
    int dbserver_peak_waiting_entities;   // peak number of entities waiting for map xfer during the last update interval
} ServerStats;

typedef struct ServerMonitorState
{
    char dbserveraddr[128];
    NetLink db_link;

    CRITICAL_SECTION stats_lock;
    MapCon** eaMaps_data;
    MapCon*** eaMaps;
    MapCon** eaMapsStuck_data;
    MapCon*** eaMapsStuck;
    LauncherCon** eaLaunchers_data;
    LauncherCon*** eaLaunchers;
    ServerAppCon** eaServerApps_data;
    ServerAppCon*** eaServerApps;
    EntCon** eaEnts_data;
    EntCon*** eaEnts;
    ServerStats stats;

    int reqstats;
    int cmd;
    U32 cmdipparam;
    const char* cmdparam;
    bool debug;
    int poll;

    bool dbstats_received;
    bool mapstats_received;
    bool launcherstats_received;
    bool ents_received;
    U32 last_received;

    TokenizerParseInfo* tpiMapConNetInfo;
    TokenizerParseInfo* tpiCrashedMapConNetInfo;
    TokenizerParseInfo* tpiEntConNetInfo;
    TokenizerParseInfo* tpiLauncherConNetInfo;
    TokenizerParseInfo* tpiServerAppConNetInfo;

    bool json;
} ServerMonitorState;

extern ServerMonitorState g_state;

#endif
