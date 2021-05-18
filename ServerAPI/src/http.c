#include <utilitieslib/components/earray.h>
#include <utilitieslib/utils/SuperAssert.h>

#include "microhttpd.h"
#include "http.h"

#include "json.h"
#include "ServerAPI.h"
#include "serverCmdStats.h"
#include "serverMonitorNet.h"

#include <utilitieslib/utils/utils.h>

#define NOT_FOUND_PAGE "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>"
static struct MHD_Response* not_found_response = 0;
#define METHOD_NOT_ALLOWED_PAGE "<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>"
static struct MHD_Response* method_not_allowed_response = 0;
#define INTERNAL_ERROR_PAGE "<html><head><title>500 Internal Server Error</title></head><body><h1>500 Internal Server Error</h1></body></html>"
static struct MHD_Response* internal_error_response = 0;

static int notFound(struct MHD_Connection* conn)
{
    return MHD_queue_response(conn, MHD_HTTP_NOT_FOUND, not_found_response);
}

static int methodNotAllowed(struct MHD_Connection* conn)
{
    return MHD_queue_response(conn, MHD_HTTP_METHOD_NOT_ALLOWED, method_not_allowed_response);
}

static int internalError(struct MHD_Connection* conn)
{
    return MHD_queue_response(conn, MHD_HTTP_INTERNAL_SERVER_ERROR, internal_error_response);
}

static int sendJson(struct MHD_Connection* conn, JsonNode* json)
{
    struct MHD_Response* resp;
    char* jsonstr;
    int ret;

    jsonstr = jsonEStr(json);
    if (!jsonstr)
    {
        return internalError(conn);
    }

    resp = MHD_create_response_from_buffer(estrLength(&jsonstr), jsonstr, MHD_RESPMEM_MUST_COPY);
    estrDestroy(&jsonstr);

    MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
    ret = MHD_queue_response(conn, 200, resp);
    MHD_destroy_response(resp);

    return ret;
}

bool svrMonAlive(ServerMonitorState* state)
{
    return state->db_link.connected;
}

static JsonNode* statusOne(ServerAPIShard* shard)
{
    ServerMonitorState* state = shard->state;
    JsonNode *jsonshard, *jsonstatus;

    jsonshard = jsonNode(shard->name, NULL, false, false);
    jsonstatus = jsonNode("status", svrMonAlive(state) ? "up" : "down", true, false);
    eaPush(&jsonshard->children, jsonstatus);

    return jsonshard;
}

static JsonNode* statsOne(ServerAPIShard* shard, void (*statsFunc)(ServerMonitorState*, JsonNode*))
{
    ServerMonitorState* state = shard->state;
    JsonNode* jsonshard = statusOne(shard);

    if (svrMonAlive(state))
    {
        EnterCriticalSection(&state->stats_lock);
        statsFunc(state, jsonshard);
        LeaveCriticalSection(&state->stats_lock);
    }

    return jsonshard;
}

static JsonNode* allstatsOne(ServerAPIShard* shard)
{
    ServerMonitorState* state = shard->state;
    JsonNode* jsonshard = statusOne(shard);

    if (svrMonAlive(state))
    {
        EnterCriticalSection(&state->stats_lock);
        serverCmdDbStats(state, jsonshard);
        serverCmdLauncherStats(state, jsonshard);
        serverCmdMapStats(state, jsonshard);
        LeaveCriticalSection(&state->stats_lock);
    }

    return jsonshard;
}

static int sendStatsOne(struct MHD_Connection* conn, ServerAPIShard* shard, void (*statsFunc)(ServerMonitorState*, JsonNode*))
{
    int ret = MHD_NO;
    JsonNode* json;

    json = jsonNode(NULL, NULL, false, false);
    eaPush(&json->children, statsOne(shard, statsFunc));
    ret = sendJson(conn, json);
    jsonDestroy(json);
    return ret;
}

static int sendAllStatsOne(struct MHD_Connection* conn, ServerAPIShard* shard)
{
    int ret = MHD_NO;
    JsonNode* json;

    json = jsonNode(NULL, NULL, false, false);
    eaPush(&json->children, allstatsOne(shard));
    ret = sendJson(conn, json);
    jsonDestroy(json);
    return ret;
}

static int sendStatsAll(struct MHD_Connection* conn, ServerAPIConfig* config, void (*statsFunc)(ServerMonitorState*, JsonNode*))
{
    int ret = MHD_NO;
    JsonNode* json;
    int i;

    json = jsonNode(NULL, NULL, false, false);
    for (i = 0; i < eaSize(&config->shards); i++)
    {
        ServerAPIShard* shard = config->shards[i];
        eaPush(&json->children, statsOne(shard, statsFunc));
    }
    ret = sendJson(conn, json);
    jsonDestroy(json);
    return ret;
}

static int sendAllStatsAll(struct MHD_Connection* conn, ServerAPIConfig* config)
{
    int ret = MHD_NO;
    JsonNode* json;
    int i;

    json = jsonNode(NULL, NULL, false, false);
    for (i = 0; i < eaSize(&config->shards); i++)
    {
        ServerAPIShard* shard = config->shards[i];
        eaPush(&json->children, allstatsOne(shard));
    }
    ret = sendJson(conn, json);
    jsonDestroy(json);
    return ret;
}

static int sendStatusOne(struct MHD_Connection* conn, ServerAPIShard* shard)
{
    int ret = MHD_NO;
    JsonNode* json;

    json = jsonNode(NULL, NULL, false, false);
    eaPush(&json->children, statusOne(shard));
    ret = sendJson(conn, json);
    jsonDestroy(json);
    return ret;
}

static int sendStatusAll(struct MHD_Connection* conn, ServerAPIConfig* config)
{
    int ret = MHD_NO;
    JsonNode* json;
    int i;

    json = jsonNode(NULL, NULL, false, false);
    for (i = 0; i < eaSize(&config->shards); i++)
    {
        ServerAPIShard* shard = config->shards[i];
        eaPush(&json->children, statusOne(shard));
    }
    ret = sendJson(conn, json);
    jsonDestroy(json);
    return ret;
}

static int dispatchGetRequest(ServerAPIConfig* config, struct MHD_Connection* conn, const char* shardname, const char* action)
{
    ServerAPIShard* shard;

    if (!stricmp(shardname, "shards"))
        return sendStatusAll(conn, config);

    if (!stricmp(shardname, "all"))
    {
        if (!stricmp(action, "dbserver"))
            return sendStatsAll(conn, config, serverCmdDbStats);
        else if (!stricmp(action, "launchers"))
            return sendStatsAll(conn, config, serverCmdLauncherStats);
        else if (!stricmp(action, "maps"))
            return sendStatsAll(conn, config, serverCmdMapStats);
        else if (!stricmp(action, "status"))
            return sendStatusAll(conn, config);
        else if (!stricmp(action, "allstats"))
            return sendAllStatsAll(conn, config);
    }

    if (shardname[0] && stashFindPointer(config->shardidx, shardname, &shard))
    {
        if (!stricmp(action, "dbserver"))
            return sendStatsOne(conn, shard, serverCmdDbStats);
        else if (!stricmp(action, "launchers"))
            return sendStatsOne(conn, shard, serverCmdLauncherStats);
        else if (!stricmp(action, "maps"))
            return sendStatsOne(conn, shard, serverCmdMapStats);
        else if (!stricmp(action, "status"))
            return sendStatusOne(conn, shard);
        else if (!stricmp(action, "allstats"))
            return sendAllStatsOne(conn, shard);
    }

    return notFound(conn);
}

static int httpRequest(void* cls, struct MHD_Connection* conn, const char* url, const char* method, const char* version, const char* upload_data,
                       size_t* upload_data_size, void** ptr)
{
    ServerAPIConfig* config = (ServerAPIConfig*)cls;
    char* urlcopy = _strdup(url);
    char* shardname = urlcopy;
    char* action;
    struct sockaddr_in* ip;
    int ret = MHD_NO;

    EXCEPTION_HANDLER_BEGIN
    if (shardname[0] == '/')
        shardname++;
    action = strtok(shardname, "/");
    action = strtok(NULL, "/");

    if (!strcmp(method, "GET"))
    {
        struct sockaddr_in* ip = (struct sockaddr_in*)MHD_get_connection_info(conn, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
        writeConsole(OUTPUT_INFO, "Request from %i.%i.%i.%i: GET %s", ip->sin_addr.S_un.S_un_b.s_b1, ip->sin_addr.S_un.S_un_b.s_b2,
                     ip->sin_addr.S_un.S_un_b.s_b3, ip->sin_addr.S_un.S_un_b.s_b4, url);
        return dispatchGetRequest(config, conn, shardname, action);
    }
    else
    {
        ip = (struct sockaddr_in*)MHD_get_connection_info(conn, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
        writeConsole(OUTPUT_INFO, "Request from %i.%i.%i.%i: 405 %s %s", ip->sin_addr.S_un.S_un_b.s_b1, ip->sin_addr.S_un.S_un_b.s_b2,
                     ip->sin_addr.S_un.S_un_b.s_b3, ip->sin_addr.S_un.S_un_b.s_b4, method, url);
        ret = methodNotAllowed(conn);
    }

    free(urlcopy);
    EXCEPTION_HANDLER_END
    return ret;
}

void startHttp(ServerAPIConfig* config)
{
    stopHttp(config);

    if (!not_found_response)
    {
        not_found_response = MHD_create_response_from_buffer(strlen(NOT_FOUND_PAGE), (void*)NOT_FOUND_PAGE, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(not_found_response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    }
    if (!method_not_allowed_response)
    {
        method_not_allowed_response = MHD_create_response_from_buffer(strlen(METHOD_NOT_ALLOWED_PAGE), (void*)METHOD_NOT_ALLOWED_PAGE, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(method_not_allowed_response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    }
    if (!internal_error_response)
    {
        internal_error_response = MHD_create_response_from_buffer(strlen(INTERNAL_ERROR_PAGE), (void*)INTERNAL_ERROR_PAGE, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(internal_error_response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    }

    config->httpserver =
        MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD | MHD_ALLOW_SUSPEND_RESUME, (uint16_t)config->port, NULL, NULL, (MHD_AccessHandlerCallback)httpRequest,
                         config, MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)(120 /* seconds */), MHD_OPTION_END);
    if (config->httpserver)
    {
        writeConsole(OUTPUT_INFO, "Listening on port %i", config->port);
    }
    else
    {
        writeConsole(OUTPUT_ERROR, "Failed to bind to port %i", config->port);
        exit(1);
    }
}

void stopHttp(ServerAPIConfig* config)
{
    if (config->httpserver)
    {
        MHD_stop_daemon(config->httpserver);
        config->httpserver = NULL;
    }
}
