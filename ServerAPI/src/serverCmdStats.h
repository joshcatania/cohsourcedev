#ifndef _SERVERCMDSTATS_H
#define _SERVERCMDSTATS_H

#include "serverCmd.h"

typedef struct JsonNode JsonNode;

void serverCmdUpdateDbStats(ServerMonitorState* state);
void serverCmdDbStats(ServerMonitorState* state, JsonNode* parent);
void serverCmdLauncherStats(ServerMonitorState* state, JsonNode* parent);
void serverCmdMapStats(ServerMonitorState* state, JsonNode* parent);
void serverCmdEntities(ServerMonitorState* state, JsonNode* parent);

#endif
