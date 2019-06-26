
#ifndef SERVERAUTOSTART_H
#define SERVERAUTOSTART_H

#include <utilitieslib/stdtypes.h>

typedef struct RunServer RunServer;
typedef struct ServerAppCon ServerAppCon;

C_DECLARATIONS_BEGIN

bool assembleAutoStartList(void); // Returns FALSE if misconfigured
bool serverAutoStartInit(void); // Returns FALSE if misconfigured
void checkServerAutoStart(void);
void startServerApp(RunServer *server);
void serverAppStatusCb(ServerAppCon *container,char *buf);

C_DECLARATIONS_END

#endif // SERVERAUTOSTART_H