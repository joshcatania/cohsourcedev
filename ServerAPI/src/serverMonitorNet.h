#ifndef _SERVER_MONITOR_NET_H
#define _SERVER_MONITOR_NET_H

#include <utilitieslib/stdtypes.h>

typedef struct MapCon MapCon;
typedef struct Packet Packet;
typedef struct ServerMonitorState ServerMonitorState;

int svrMonConnect(ServerMonitorState* state, char* ip_str);
int svrMonDisconnect(ServerMonitorState* state);
int svrMonConnected(ServerMonitorState* state);
int svrMonRequest(ServerMonitorState* state, int msg);
int svrMonRequestDiff(ServerMonitorState* state);
int svrMonNetTick(ServerMonitorState* state);
int svrMonShutdownAll(ServerMonitorState* state, const char* reason);
int svrMonResetMission(ServerMonitorState* state);
void svrMonDelink(ServerMonitorState* state, MapCon* con);
void svrMonSendAdminMessage(ServerMonitorState* state, const char* msg);
void svrMonSendDbMessage(ServerMonitorState* state, const char* msg, const char* params);
void svrMonLogHistory(ServerMonitorState* state);
void svrMonSendOverloadProtection(ServerMonitorState* state, const char* msg);

int svrMonGetSendRate(ServerMonitorState* state);
int svrMonGetRecvRate(ServerMonitorState* state);
int svrMonGetNetDelay(ServerMonitorState* state);

void svrMonRequestEnts(ServerMonitorState* state, int val);

void svrMonClearAllLists(ServerMonitorState* state);

int notTroubleStatus(char* status);

#endif // _SERVER_MONITOR_NET_H
