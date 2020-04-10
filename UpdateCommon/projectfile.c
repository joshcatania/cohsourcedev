#include <string.h>
#include <direct.h>
#include "projectfile.h"
#include "utils.h"
#include "error.h"
#include "EArray.h"

TokenizerParseInfo parse_regkey[] = {
	{ "Name",			TOK_STRING(PatchRegKey,name, 0)	},
	{ "Value",			TOK_STRING(PatchRegKey,value, 0)	},
	{ "{",				TOK_START,		0						},
	{ "}",				TOK_END,			0						},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_ip[] = {
	{ "",				TOK_STRUCTPARAM | TOK_STRING(AllowIp, ip_str, 0)	},
	{ "\n",				TOK_END,			0						},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_syncblock[] = {
	{ "",				TOK_STRUCTPARAM | TOK_STRING(SyncBlock,str, 0)	},
	{ "\n",				TOK_END,			0						},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_syncallow[] = {
	{ "",				TOK_STRUCTPARAM | TOK_STRING(SyncAllow,str, 0)	},
	{ "\n",				TOK_END,			0						},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_majorpatch[] = {
	{ "Name",			TOK_STRING(MajorPatch,name, 0)	},
	{ "From",			TOK_STRING(MajorPatch,from, 0)	},
	{ "To",				TOK_STRING(MajorPatch,to, 0)	},
	{ "{",				TOK_START,		0							},
	{ "}",				TOK_END,			0							},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_project[] = {
	{ "Name",			TOK_STRING(PatchProject,name,0)	},
	{ "Current",		TOK_STRING(PatchProject,current,0)	},
	{ "PatchClient",	TOK_STRING(PatchProject,patch_client_name,0)	},
	{ "NoDiff",			TOK_INT(PatchProject,no_diff,0)	},
	{ "PatchRecent",	TOK_STRING(PatchProject,patch_recent_str,0)	},
	{ "{",				TOK_START,		0						},
	{ "}",				TOK_END,			0						},
	{ "RegKey",			TOK_STRUCT(PatchProject,regkeys, parse_regkey) },
	{ "AllowIp",		TOK_STRUCT(PatchProject,allow_ips, parse_ip) },
	{ "MajorPatch",		TOK_STRUCT(PatchProject,major_patch, parse_majorpatch) },
	{ "SyncAllow",		TOK_STRUCT(PatchProject,sync_allows, parse_syncallow) },
	{ "SyncBlock",		TOK_STRUCT(PatchProject,sync_blocks, parse_syncblock) },
	{ "NoSyncDelete",	TOK_INT(PatchProject,no_sync_delete,0)	},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_all_projects[] = {
	{ "MaxClients",			TOK_INT(ProjectList,max_clients,0) },
	{ "Project",			TOK_STRUCT(ProjectList,projects, parse_project) },
	{ "SlowValidate",		TOK_INT(ProjectList,slow_validate,0)	},
	{ "MajorpatchThrottle",	TOK_INT(ProjectList,throttle_speed,0)	},
	{ "", 0, 0 }
};

ProjectList	project_list;

int verbotenClientFile(char *project_dir,char *fname)
{
	static char *exts[] = { "CityOfHeroes.pdb",
							"TestClient.exe", "TestClient.pdb",
							"TestClientLauncher.exe", "TestClientLauncher.pdb",
							"TextClient.exe", "TextClient.pdb",
							"TextClientLauncher.exe", "TextClientLauncher.pdb",
							"charinfo.exe", "charinfo_helper.exe", "dbquery.exe",
							"CmdRelay.exe", "CmdRelay.pdb",
							"chatserver.exe", "chatserver.pdb",
							"dbserver.exe", "dbserver.pdb",
							"launcher.exe", "launcher.pdb",
							"logserver.exe", "logserver.pdb",
							"mapserver.exe", "mapserver.pdb",
							"UpdateServer.exe", "UpdateServer.pdb",
							"RemoteDebuggerer.exe", "RemoteDebuggerer.pdb",
							"ServerMonitor.exe", "ServerMonitor.pdb",
							"ShardMonitor.exe", "ShardMonitor.pdb",
							"userdump.exe", "userdump.pdb",
							"ChatAdmin.exe", "ChatAdmin.pdb",
							"data/server/db/doors.db",
							"data/server/db/maps.db", };
	static char *dirs[] = { "docs", "src", "data/server/db/templates", "Catalog" };
	char		buf[MAX_PATH];
	int			i;

	if (strEndsWith(fname,"demo launcher.exe"))
		return 0;
	if (strEndsWith(fname,"tmp/cmdrelay.exe"))
		return 0;
	for(i=0;i<ARRAY_SIZE(dirs);i++)
	{
		sprintf(buf,"%s/%s/",project_dir,dirs[i]);
		forwardSlashes(buf);
		if (strnicmp(buf,fname,strlen(buf))==0)
			return 1;
	}
	for(i=0;i<ARRAY_SIZE(exts);i++)
	{
		if (strEndsWith(fname,exts[i]))
			return 1;
	}
	if (strEndsWith(fname,".pigg") && strnicmp(getFileName(fname),"server_",strlen("server_"))==0)
		return 1;
	return 0;
}

static void safetyCheckClientFiles(PatchProject *project,ImageCheck *image)
{
	int		i;

	for(i=0;i<image->file_count;i++)
	{
		if (verbotenClientFile(image->fname,image->files[i]->name))
			FatalErrorf("Illegal file in client patch %s: %s",project->name,image->files[i]->name);
	}
}

ImageCheck *findImageByName(PatchProject *project,char *version)
{
	int		i;

	for(i=0;i<project->image_count;i++)
	{
		if (stricmp(version,getFileName(project->images[i]->fname))==0)
			return project->images[i];
	}
	return 0;
}

void addPatchToProject(PatchProject *project,ImageCheck *new_image)
{
	ImageCheck	*image;

	image = malloc(sizeof(*image));
	*image = *new_image;
	dynArrayAddp(&project->images,&project->image_count,&project->image_max,image);
}

PatchProject *projectFind(char *project_name)
{
	int		i;

	for(i=0;i<project_list.project_count;i++)
	{
		if (stricmp(project_list.projects[i]->name,project_name)==0)
			return project_list.projects[i];
	}
	return 0;
}

static int cmpImageTimestamp(const ImageCheck **a, const ImageCheck **b)
{
	return (*a)->timestamp - (*b)->timestamp;
}

static int isClientImage(PatchProject *project)
{
	if (stricmp(project->name,"CoH")==0 || stricmp(project->name,"CohTest")==0)
		return 1;
	return 0;
}

ImageCheck *projectReloadChecksum(PatchProject *project,int create_if_missing,int delete_if_bad,int full_checksum,char *image_dir,char *image_name)
{
	char path[MAX_PATH],checksum_name[MAX_PATH];
	ImageCheck	*image;
	int	i, index = -1;

	for (i=0;i<project->image_count;i++)
	{
		if (stricmp(image_name,getFileName(project->images[i]->fname))==0)
		{
			checksumFree(project->images[i]);
			index = i;
		}
	}

	getRootPathFromImage(image_dir,path);
	sprintf(checksum_name,"%s/checksums/%s/%s.checksum",project->rootpath,project->name,image_name);
	image = calloc(sizeof(*image),1);
	image->fname = strdup(image_dir);
	if (checksumLoad(checksum_name,image))
	{
		if (!checksumVerify(image->fname,image,0,full_checksum,0))
		{
			if (delete_if_bad)
			{
				printf("Deleting bad image: %s\n",image->fname);
				removeDirAll(image->fname);
			}
			checksumFree(image);
			image = NULL;
		}
	}
	else if (create_if_missing)
	{
		mkdirtree(checksum_name);
		if (stricmp(image_name,"0")!=0)
			printf("  %s\n",image_name);
		checksumImage(image_dir,checksum_name,image_name);
		if (!checksumLoad(checksum_name,image))
			msgAlertFatal("ErrChecksumLoad",checksum_name);
	}
	else
	{
		free(image->fname);
		free(image);
		image=NULL;
	}

	if (index >= 0)
	{
		if (image)
		{
			project->images[index] = image;
		}
		else
		{
			int j;
			for (j = index + 1; j < project->image_count; j++)
				project->images[j-1] = project->images[j];
			project->image_count--;
		}
	}
	else if (image)
	{
		dynArrayAddp(&project->images,&project->image_count,&project->image_max,image);
	}

	qsort(project->images,project->image_count,sizeof(project->images[0]),cmpImageTimestamp);

	if (image && isClientImage(project))
		safetyCheckClientFiles(project,image);

	return image;
}

void projectLoadChecksums(PatchProject *project,int create_if_missing,int delete_if_bad,int full_checksum)
{
	int			i,count;
	char		**image_dirs,path[MAX_PATH], *image_name;

	sprintf(path,"%s/images/%s",project->rootpath,project->name);
	image_dirs = getImageDirs(path,&count);
	for(i=0;i<count;i++)
	{
		image_name = getFileName(image_dirs[i]);
		if (findImageByName(project,image_name))
			continue;
		projectReloadChecksum(project, create_if_missing, delete_if_bad, full_checksum, image_dirs[i], image_name);
	}

	project->curr_image = findImageByName(project,project->current);
}

void projectLoadConfig(char *name,int full_checksum,int delete_if_bad)
{
	TokenizerHandle tok;
	int				i,fileisgood;
	char			fname[MAX_PATH];
	PatchProject	*project;

	printf("Loading config...\n");

	makefullpath(name,fname);

	tok = TokenizerCreate(fname);
	if( !tok )
		msgAlertFatal( "ErrInvalidProjectFile", fname );
	fileisgood = TokenizerParseList(tok, parse_all_projects, &project_list, TokenizerErrorCallback);
	TokenizerDestroy(tok);
	project_list.project_count = EArrayGetSize(&project_list.projects);

	printf("Loading checksums...\n");

	for(i=0;i<project_list.project_count;i++)
	{
		project = project_list.projects[i];
		project->patch_recent = -1;
		if (project->patch_recent_str)
			project->patch_recent = atoi(project->patch_recent_str);
		_getcwd(project->rootpath, MAX_PATH );
		forwardSlashes(project->rootpath);
		//makefullpath(project->name,project->pathname);
		projectLoadChecksums(project,0,delete_if_bad,full_checksum);
		if (!isClientImage(project) && !EArrayGetSize(&project->allow_ips))
			FatalErrorf("Project %s, AllowIp is required for non-client patches",project->name);
	}
}

