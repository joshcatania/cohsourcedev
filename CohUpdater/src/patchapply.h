#ifndef _PATCHAPPLY_H
#define _PATCHAPPLY_H

#include "patchheader.h"
#include "piglib.h"

int applyPatch(char *project_name,char *patch_name,char *checksum_name,char *src_dir,char *dst_dir,int has_checksum);
char *makeAbsPatchList(ImageCheck *src,char *dst_string);
int safeWritePigFile(char *pig_name,NewPigEntry *entries,int entry_count,ImageCheck *dst_image,char *dst_dir,U32 timestamp);

#endif
