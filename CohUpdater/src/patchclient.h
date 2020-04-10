#ifndef _PATCHCLIENT_H
#define _PATCHCLIENT_H

#include "netio.h"

int clientPatch(char *project_name,char *src_dir,char *dst_dir,char *build_name,int default_patch);
void patchClientInit(char *ps_name,int ps_port);
int waitForCmd(Packet **pak_p);
int lostServerLink();
void connectToServer();
void disconnectFromServer();
void patchGetMajor(char *project,char *dst_dir);

extern NetLink comm_link;

#endif
