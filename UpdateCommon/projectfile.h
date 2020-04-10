#ifndef _PROJECTFILE_H
#define _PROJECTFILE_H

#include "patchheader.h"
#include "patchfileutils.h"
#include "file.h"
#include "textparser.h"

#define MAX_CIDER_PATCH_TYPES 8
#define CIDER_ID_SIZE         4

typedef struct
{
	char	*name;
	char	*value;
} PatchRegKey;

typedef struct
{
	char	*ip_str;
	U8		ip_bytes[4];
	U8		ip_match[4];
} AllowIp;

typedef struct
{
	char *str;
} SyncBlock;

typedef struct
{
	char *str;
} SyncAllow;

typedef struct
{
	char	*name;
	char	server_name[MAX_PATH];
	char	*from;
	char	*to;
} MajorPatch;

typedef struct
{
	char		rootpath[MAX_PATH];
	char		*name;
	char		*current;
	ImageCheck	**images;
	int			image_count,image_max;
	ImageCheck	*curr_image;
	int			no_diff, no_sync_delete;
	char		*patch_recent_str;
	int			patch_recent;

	PatchRegKey	**regkeys;
	AllowIp		**allow_ips;
	SyncAllow	**sync_allows;
	SyncBlock	**sync_blocks;
	MajorPatch	**major_patch;
	ZipData		full_manifest;

	char		*patch_client_name;
} PatchProject;

typedef struct
{
	U32             cider_version;
	char			cider_ID[CIDER_ID_SIZE];
	char            *cider_update_name;
	U32             cider_update_size;
} CiderPatch;

typedef struct
{
	PatchProject	**projects;
	int				project_count;
	ZipData			patch_client;
	Checksum		patch_client_check;
	ZipData         patch_client_multifile;
	char*           patch_client_multifile_name;
	Checksum		patch_client_multifile_check;
	CiderPatch      cider_patches[MAX_CIDER_PATCH_TYPES];
	int				max_clients;
	char			*server_version;
	char			*client_version;
	int				slow_validate;
	int				throttle_speed;  // in bytes/sec
} ProjectList;

extern ProjectList	project_list;

void projectLoadConfig(char *name,int full_checksum,int delete_if_bad);
PatchProject *projectFind(char *project_name);
void addPatchToProject(PatchProject *project,ImageCheck *new_image);
ImageCheck *findImageByName(PatchProject *project,char *version);
ImageCheck *projectReloadChecksum(PatchProject *project,int delete_if_bad,int create_if_missing,int full_checksum,char *image_dir,char *image_name);
void projectLoadChecksums(PatchProject *project,int create_if_missing,int delete_if_bad,int full_checksum);
int verbotenClientFile(char *project_dir,char *fname);

#endif
