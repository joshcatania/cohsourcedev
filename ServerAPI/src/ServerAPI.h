#ifndef _SERVERAPI_H
#define _SERVERAPI_H

#include <utilitieslib\components\StashTable.h>

#define TokenizerParseInfo ParseTable

typedef struct ServerMonitorState ServerMonitorState;

typedef struct ServerAPIShard
{
    char* name;
    char* dbserver;

    // runtime
    ServerMonitorState* state;
} ServerAPIShard;

typedef struct ServerAPIConfig
{
    int port;
    ServerAPIShard** shards;

    StashTable shardidx;
    struct MHD_Daemon* httpserver;
} ServerAPIConfig;

extern ServerAPIConfig config;

#endif
