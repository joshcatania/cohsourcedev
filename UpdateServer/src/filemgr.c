#include "stdtypes.h"
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "StashTable.h"
#include "filechecksum.h"
#include "utils.h"
#include "comm_patcher.h"

static StashTable files_by_name;
static StashTable files_by_checksum;
extern int glob_perf_test;

typedef struct
{
	Checksum	check;
	char		*fname;
	U32			size;
	FILE		*file;
} FileId;

static FileId	**file_ids;
static int		file_id_count,file_id_max;

void filemgrInit(void)
{
	files_by_name		= stashTableCreateWithStringKeys(1000,StashDeepCopyKeys);
	files_by_checksum	= stashTableCreateWithStringKeys(1000,StashDeepCopyKeys);
}

void filemgrAdd(char *fname)
{
	char		checksum_str[256];
	FileId		*file_id;
	FILE		*file;
	U32			*val;

	file = fopen(fname,"rb");
	if (!file)
	{
		printf("Warning: file %s could not be read, so not add to file manager.\n",fname);
		return;
	}
	file_id = calloc(sizeof(*file_id),1);
	fread(&file_id->check,1,sizeof(Checksum),file);
	fseek(file,0,2);
	file_id->size = (U32)ftell(file);
	file_id->file = file;
	file_id->fname = strdup(fname);
	dynArrayAddp(&file_ids,&file_id_count,&file_id_max,file_id);
	val = file_id->check.values;
	sprintf(checksum_str,"%08x%08x%08x%08x",val[0],val[1],val[2],val[3]);
	stashAddPointer(files_by_name,fname,file_id, false);
	stashAddPointer(files_by_checksum,checksum_str,file_id, false);
}

char *filemgrFindByChecksum(Checksum *check)
{
	FileId	*file_id;
	char	checksum_str[256];
	U32		*val;

	val = check->values;
	sprintf(checksum_str,"%08x%08x%08x%08x",val[0],val[1],val[2],val[3]);
	if (stashFindPointer(files_by_checksum, checksum_str, &file_id))
		return file_id->fname;
	return 0;
}

Checksum *filemgrGetChecksum(char *fname)
{
	FileId	*file_id;

	if (!stashFindPointer(files_by_name, fname, &file_id))
		return 0;
	return &file_id->check;
}

int filemgrGetData(Checksum *check,U32 start,U32 count,U8 **mem_p)
{
	FileId	*file_id;
	U32		bytes_read;
	char	checksum_str[256];
	U32		*val;
	static U8 mem[XFER_BLOCK_SIZE];

	val = check->values;
	sprintf(checksum_str,"%08x%08x%08x%08x",val[0],val[1],val[2],val[3]);
	if (!stashFindPointer(files_by_checksum, checksum_str, &file_id))
		return 0;
	if (start + count > file_id->size)
		return 0;

	fseek(file_id->file,start,0);
	if (glob_perf_test < 2)
		bytes_read = fread(mem,1,count,file_id->file);
	else
		bytes_read = count;
	*mem_p = mem;
	return bytes_read;
}
