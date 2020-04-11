#ifndef _PATCHSERVERLISTENER_H
#define _PATCHSERVERLISTENER_H

void updateListenerStatus(char *serverVersion, char *clientVersion, int users, __int64 totalSent, int bytesPerSec);
void patchServerListenThreadBegin(void);

#endif
