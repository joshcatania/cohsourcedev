#ifndef _RUNTIMEANIM_H
#define _RUNTIMEANIM_H

#include "seq/animtrack.h"

/*
 * Developer-only inspection/export helpers.  These deliberately use the
 * runtime file path (animGetAnimTrack/fileOpen) so packed .anim assets can be
 * inspected without extracting a whole pigg archive.
 */
bool runtimeAnimWriteReport(const SkeletonAnimTrack *anim, const char *path);
bool runtimeAnimWriteSKELX(const SkeletonAnimTrack *anim, const char *path);
bool runtimeAnimWriteANIMX(const SkeletonAnimTrack *anim, const char *path);

#endif
