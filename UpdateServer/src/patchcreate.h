#ifndef _PATCHCREATE_H
#define _PATCHCREATE_H

#include "stdtypes.h"
#include "filechecksum.h"
#include "patchheader.h"
#include "patchfileutils.h"

void patchCreate(ImageCheck *src,ImageCheck *dst,char *patch_name,char *msg,int no_diff);
void makeAllFileManifest(ImageCheck *image,ZipData *zip);
char *patchCreateCustom(ImageCheck *dst,char *src_str,char *patch_name);

// NCSoft is always complaining that the version on the update server doesn't match the updater
// even though there is no actual reason that it needs to other than to make them happy,
// so this is an ugly, hackish way to force the update server to a new version
//
// this function performs nothing useful and doesn't (shouldn't) actually ever get called
void FunctionWithNoPurposeOtherThanToForceALink(void);

#endif
