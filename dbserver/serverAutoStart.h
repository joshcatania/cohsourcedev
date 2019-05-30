
#ifndef SERVERAUTOSTART_H
#define SERVERAUTOSTART_H

#include "stdtypes.h"

C_DECLARATIONS_BEGIN

typedef struct RunServer RunServer;
typedef struct ServerAppCon ServerAppCon;

bool assembleAutoStartList(void); // Returns FALSE if misconfigured
bool serverAutoStartInit(void); // Returns FALSE if misconfigured
void checkServerAutoStart(void);
void startServerApp(RunServer *server);
void serverAppStatusCb(ServerAppCon *container,char *buf);

C_DECLARATIONS_END

#endif // SERVERAUTOSTART_H