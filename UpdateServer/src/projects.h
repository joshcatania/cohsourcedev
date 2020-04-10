#ifndef _PROJECTS_H
#define _PROJECTS_H

#include "projectfile.h"

void projectLoad(char *name,int run_server,int patch_to_latest);
int projectCheckAccess(PatchProject *project,U32 ip);

#endif
