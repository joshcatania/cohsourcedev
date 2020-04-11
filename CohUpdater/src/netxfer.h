#ifndef _NETXFER_H
#define _NETXFER_H

#include "piglib.h"
#include "netio.h"

int waitForCmd(Packet **pak_p);
int lostServerLink(void);
int netGetFiles(NewPigEntry **entries,int count);
int netGetPatchFile(char *client_patch_name);

void* netGetUpdaterFile(const char* szFileName, U32 nSize, int bStatUpdate);

#endif
