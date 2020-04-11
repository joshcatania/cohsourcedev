#ifndef _FILEMGR_H
#define _FILEMGR_H

#include "filechecksum.h"

void filemgrAdd(char *fname);
char *filemgrFindByChecksum(Checksum *check);
Checksum *filemgrGetChecksum(char *fname);
int filemgrGetData(Checksum *check,U32 start,U32 count,U8 **mem_p);
void filemgrInit(void);

#endif
