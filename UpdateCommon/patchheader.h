#ifndef _PATCHHEADER_H
#define _PATCHHEADER_H

#include "stdtypes.h"
#include "filechecksum.h"

typedef struct
{
	__int64		unpack_size;
	__int64		size;
	__int64		pos;
} PFileEntry;

typedef struct
{
	Checksum	checksum;
	U32			version;
	PFileEntry	cmd;
	PFileEntry	data;
	PFileEntry	manifest;
} PatchHeader;



#endif
