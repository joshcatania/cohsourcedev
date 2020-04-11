#include "stdtypes.h"
#include "projects.h"
#include "error.h"
#include "utils.h"
#include "earray.h"
#include "patchfileutils.h"
#include "filechecksum.h"
#include <string.h>
#include "zlib.h"
#include "filemgr.h"
#include "patchcreate.h"
#include "sysutil.h"
#include <direct.h>
#include <assert.h>

#include "tar.h"

Checksum *coh_majorpatch_checksum = 0;

void loadCiderUpdate();

void patchToVersion(PatchProject *project,ImageCheck *image,int force_recalc)
{
	int			i,patch_count=-1,patch_curr=0;
	ImageCheck	*curr;
	char		patch_name[MAX_PATH],buf[200];

	printf(" Patches for %s..\n",image->build_name);
	if (!project->patch_recent)
		return;
	for(i=0;i<project->image_count;i++)
	{
		if (i && project->patch_recent >= 0 && ((project->image_count-i-1) > project->patch_recent))
			continue;
		sprintf(patch_name,"%s/patches/%s/%s/%s.patch",project->rootpath,project->name,getFileName(image->fname),getFileName(project->images[i]->fname));
		if (force_recalc || !fileExists(patch_name))
			patch_count++;
	}
	for(i=0;i<project->image_count;i++)
	{
		curr = project->images[i];
		if (curr == image)
			continue;
		if (i && project->patch_recent >= 0 && ((project->image_count-i-1) > project->patch_recent))
			continue;
		sprintf(patch_name,"%s/patches/%s/%s/%s.patch",project->rootpath,project->name,getFileName(image->fname),getFileName(project->images[i]->fname));
		if (!checksumVerify(curr->fname,curr,0,0,0))
			FatalErrorf("image %s failed verification check\n",curr->fname);
		if (force_recalc || !fileExists(patch_name))
		{
			sprintf(buf,"  %d/%d:",++patch_curr,patch_count);
			checksumOpenPigs(curr,curr->fname,1);
			patchCreate(curr,image,patch_name,buf,project->no_diff);
		}
	}
}

int projectCheckAccess(PatchProject *project,U32 ip)
{
	int		i,j;
	AllowIp	*allow;
	U8		ip_bytes[4];

	for(i=0;i<4;i++)
		ip_bytes[i] = (ip >> (8*i)) & 255;
	if (!eaSize(&project->allow_ips))
		return 1;
	for(i=0;i<eaSize(&project->allow_ips);i++)
	{
		allow = project->allow_ips[i];
		for(j=0;j<4;j++)
		{
			if (allow->ip_match[j] && allow->ip_bytes[j] != ip_bytes[j])
				break;
		}
		if (j >= 4)
			return 1;
	}
	return 0;
}


static void setupIpList(PatchProject *project)
{
	int		i,j;
	char	*s,*s2,ip_str[1000];
	AllowIp	*allow;

	for(i=0;i<eaSize(&project->allow_ips);i++)
	{
		allow = project->allow_ips[i];
		s = strcpy(ip_str,allow->ip_str);
		for(j=0;j<4 && s;j++)
		{
			s2 = strchr(s,'.');
			if (s2)
				*s2++ = 0;
			allow->ip_bytes[j] = atoi(s);
			if (*s != '*')
				allow->ip_match[j] = 1;
			s = s2;
		}
	}
}

static void addAllPatchesToFileMgr(char *dir)
{
	char	**names;
	int		i,count;

	names = getDirPatches(dir,&count);
	for(i=0;i<count;i++)
	{
		filemgrAdd(names[i]);
	}
	freeDirFiles(names,count);
}

#define MAX_UPDATER_FILES 3

extern int no_tar;

static void loadPatchClient()
{
	char	fullname[MAX_PATH];
	char    tarFilename[MAX_PATH];
	U8		*mem,*s;
	Checksum	server_checksum;
	int				i,j,nUpdaterFiles = 0;
	PatchProject	*project;

	ZipData	*zip = &project_list.patch_client;
	ZipData	*zip_multifile = &project_list.patch_client_multifile;

	strcpy(fullname,getExecutableName());
	checksumFile(fullname,&server_checksum,0);
    _getcwd(fullname, MAX_PATH);

	strncpy(tarFilename,fullname,MAX_PATH);
	strcat(fullname,"/CohUpdater.exe");
	strcat(tarFilename,"/CohUpdate.tar");

	mem = extractFromFS(fullname,&zip->unpack_size);
	if (!mem)
		FatalErrorf("Can't load patchclient: %s",fullname);

	checksumFile(fullname,&project_list.patch_client_check,0);
	zip->zip_data = zipData(mem,zip->unpack_size,&zip->pack_size);

	if(!no_tar)
	{
		U32 curFileSize, nStrLength;
		U8 *tar_mem, *curFile;
		Tar *t;
		char cur_temp_name[MAX_PATH], szTemp[MAX_PATH];
		char *ppUpdaterFiles[MAX_UPDATER_FILES];

		tar_mem = extractFromFS(tarFilename,&zip_multifile->unpack_size);
		if (!tar_mem)
			FatalErrorf("Can't load multifile client patch: %s\n\tMaybe pass -notar?",tarFilename);

		// untar files
		szTemp[0] = '\0';
		GetTempPath(MAX_PATH,szTemp);

		t = tar_from_data(tar_mem,zip_multifile->unpack_size);
		curFile = tar_alloc_cur_file(t,&curFileSize);
		

		while ( ( curFile ) && ( nUpdaterFiles <= MAX_UPDATER_FILES ) )
		{
			int nLoc = -1;

			snprintf(cur_temp_name,MAX_PATH,"%s\\%s",szTemp,t->hdr.fname);

			if ( strEndsWith(t->hdr.fname,"exe") )
			{
				nLoc = 0;
			}
			else if ( strEndsWith(t->hdr.fname,"dll") )
			{
				nLoc = 1;
			}
			else if ( ( !strEndsWith(t->hdr.fname,"tgz") ) && ( ! strEndsWith(t->hdr.fname,"\\") ) && ( ! strEndsWith(t->hdr.fname,"/") ) )
			{
				nLoc = 2;
			}

			if ( nLoc >= 0 )
			{
				safeWriteFile(cur_temp_name,"!wb",curFile,curFileSize);

				nStrLength = strlen(cur_temp_name) + 1;

				ppUpdaterFiles[nLoc] = (char*)malloc(nStrLength*sizeof(char));
				strncpy(ppUpdaterFiles[nLoc],cur_temp_name,nStrLength);

				log_printf("connections","added file %s for checksumming", cur_temp_name);

				nUpdaterFiles++;
			}

			free(curFile);

			if ( tar_next(t) )
				curFile = tar_alloc_cur_file(t,&curFileSize);
			else
				curFile = NULL;
		}

		// checksum all files
		checksumMultipleFiles(ppUpdaterFiles,nUpdaterFiles,&project_list.patch_client_multifile_check,0);

		for ( i = 0; i < nUpdaterFiles; i++ )
		{
			free(ppUpdaterFiles[i]);
		}

		// compress
		zip_multifile->zip_data = zipData(tar_mem,zip_multifile->unpack_size,&zip_multifile->pack_size);
	}

	// find all cider updates
	loadCiderUpdate();

	project = projectFind("Coh Updater");
	if (!project)
	{
		printf("Not serving coh updater patch, cannot determine version\n");
		return;
	}
	for(i=0;i<project->image_count;i++)
	{
		ImageCheck	*image = project->images[i];

		for(j=0;j<image->file_count;j++)
		{
			if (checksumMatch(&image->files[j]->full,&server_checksum) && !project_list.server_version)
				project_list.server_version = strdup(getFileName(image->fname));
			if (checksumMatch(&image->files[j]->full,&project_list.patch_client_check) && !project_list.client_version)
				project_list.client_version = strdup(getFileName(image->fname));
		}
	}
}

static void loadCiderUpdate()
{
	WIN32_FIND_DATA ffd;
    HANDLE hFind  = INVALID_HANDLE_VALUE;
	char szTemp[MAX_PATH];
	char szCiderUpdateDir[MAX_PATH];
	int i = 0;
//	const char* szPrefixStr = "cider-COHD-Updater-";
	const char* szPrefixStr = "cider-";
	char szCiderID[CIDER_ID_SIZE];
	int bAddCiderID = 1;

	// initialize cider updates list
	for ( i = 0; i < MAX_CIDER_PATCH_TYPES; i++ )
	{
		project_list.cider_patches[i].cider_version = 0;
		project_list.cider_patches[i].cider_update_size = 0;
		project_list.cider_patches[i].cider_update_name = NULL;
		project_list.cider_patches[i].cider_ID[0] = '\0';
	}

	snprintf(szCiderUpdateDir,MAX_PATH,"%s\\cider-updates\\*",getExecutableDir(szTemp));

	hFind = FindFirstFileA(szCiderUpdateDir, &ffd);

	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				unsigned int nUpdateVer = 0;
				bAddCiderID = 1;

				if( strStartsWith(ffd.cFileName,szPrefixStr) && strEndsWith(ffd.cFileName,".tgz") && 
					(strlen(ffd.cFileName) == strlen("cider-COHD-Updater-123456.tgz")) )
				{
					memcpy(szCiderID,ffd.cFileName + strlen(szPrefixStr),CIDER_ID_SIZE);
					sscanf(ffd.cFileName + strlen(szPrefixStr)+strlen("COHD-Updater-"),"%6d.tgz",&nUpdateVer);

					for ( i = 0; i < MAX_CIDER_PATCH_TYPES; i++ )
					{
						if ( memcmp(project_list.cider_patches[i].cider_ID,szCiderID,CIDER_ID_SIZE) == 0 )
						{
							// found the right Cider ID
							if ( nUpdateVer > project_list.cider_patches[i].cider_version )
							{
								project_list.cider_patches[i].cider_version = nUpdateVer;
								snprintf(szTemp,MAX_PATH,"%s\\cider-updates\\%s",getExecutableDir(szCiderUpdateDir),ffd.cFileName);

								if ( project_list.cider_patches[i].cider_update_name )
									free(project_list.cider_patches[i].cider_update_name);

								project_list.cider_patches[i].cider_update_name = strdup(ffd.cFileName);

								bAddCiderID = 0;
							}

							break;
						}
						else if ( project_list.cider_patches[i].cider_ID[0] == '\0' )
						{
							break;
						}
					}

					if ( bAddCiderID )
					{
						if ( i < MAX_CIDER_PATCH_TYPES )
						{
							project_list.cider_patches[i].cider_version = nUpdateVer;
							memcpy(project_list.cider_patches[i].cider_ID,szCiderID,CIDER_ID_SIZE);

							snprintf(szTemp,MAX_PATH,"%s\\cider-updates\\%s",getExecutableDir(szCiderUpdateDir),ffd.cFileName);

							if ( project_list.cider_patches[i].cider_update_name )
								free(project_list.cider_patches[i].cider_update_name);

							project_list.cider_patches[i].cider_update_name = strdup(ffd.cFileName);							
						}
						else
						{
							// ran out of IDs
							FatalErrorf("ran out of Cider IDs at %d with Cider ID %s. (aborting)\n", i, szCiderID);
						}
					}
				}
			}
		} while (FindNextFileA(hFind, &ffd) != 0);

		// test all the files that are going to be updates
		if ( project_list.cider_patches[0].cider_version > 0 )
		{
			for ( i = 0; i < MAX_CIDER_PATCH_TYPES; i++ )
			{
				if ( project_list.cider_patches[i].cider_update_name )
				{
					U8* mem = NULL;

					// found something, test to make sure it's readable
					snprintf(szTemp,MAX_PATH,"%s\\cider-updates\\%s",
						getExecutableDir(szCiderUpdateDir),
						project_list.cider_patches[i].cider_update_name);

					mem = extractFromFS(szTemp,&project_list.cider_patches[i].cider_update_size);

					if ( mem )
					{
						free(mem);				
					}
					else
					{
						FatalErrorf("couldn't read file %s. (aborting)\n", szTemp);
					}
				}
			}
		}
	}
}

static void pruneChecksumsAndPatches(PatchProject *project)
{
	char	**names,*s,checksum_dir[MAX_PATH],buf[MAX_PATH],patchdir[MAX_PATH],patchname[MAX_PATH];
	int		count,i,j;

	sprintf(checksum_dir,"%s/checksums/%s",project->rootpath,project->name);
	names = getDirFiles(checksum_dir,&count);
	for(i=0;i<count;i++)
	{
		strcpy(buf,getFileName(names[i]));
		s = strrchr(buf,'.');
		if (s)
			*s = 0;
		if (!findImageByName(project,buf))
		{
			printf("Deleting patch files for missing image: %s/%s\n",project->name,buf);
			sprintf(patchdir,"%s/patches/%s/%s",project->rootpath,project->name,buf);
			removeDirAll(patchdir);
			for(j=0;j<project->image_count;j++)
			{
				sprintf(patchname,"%s/patches/%s/%s/%s.patch",project->rootpath,project->name,project->images[j]->build_name,buf);
				if (fileExists(patchname))
				{
					safeDeleteFile(patchname);
					sprintf(patchname,"%s/patches/%s/%s/patchcmds/%s.patchcmds",project->rootpath,project->name,project->images[j]->build_name,buf);
					safeDeleteFile(patchname);
				}
			}
			printf(" Deleting: %s\n",names[i]);
			safeDeleteFile(names[i]);
		}
	}		
	freeDirFiles(names,count);
}

static void checkMajorPatch(PatchProject *project)
{
	ImageCheck	*from,*to;
	MajorPatch	*mp;

	if (!project->major_patch)
		return;
	printf(" Major patch..\n");
	mp = project->major_patch[0];
	sprintf(mp->server_name,"%s/patches/%s/%s/%s.patch",project->rootpath,project->name,mp->to,mp->from);
	from = findImageByName(project,mp->from);
	to = findImageByName(project,mp->to);
	if (!from)
		msgAlertFatal("patch FROM image \"%s\" does not exist",mp->from);
	if (!to)
		msgAlertFatal("patch TO image \"%s\" does not exist",mp->to);
	if (!fileExists(mp->server_name))
		patchCreate(from,to,mp->server_name,"Major patch",0);
	filemgrAdd(mp->server_name);
	patchToVersion(project,from,0);
	if( !stricmp("coh", project->name) && !coh_majorpatch_checksum )
		coh_majorpatch_checksum = filemgrGetChecksum(mp->server_name);
}

void projectLoad(char *name,int run_server,int patch_to_latest)
{
	int		i;
	char	patchdir[MAX_PATH];

	projectLoadConfig(name,0,0);

	for(i=0;i<project_list.project_count;i++)
	{
		ImageCheck		*image;
		PatchProject	*project = project_list.projects[i];

		printf("%d/%d: Project: %s\n",i+1,project_list.project_count,project->name);
		setupIpList(project);
		printf(" Checksums..\n");
		projectLoadChecksums(project,1,0,0);
		if (patch_to_latest)
		{
			project->curr_image = project->images[project->image_count-1];
			project->current = project->curr_image->build_name;
		}
		image = project->curr_image;
		if (!image)
			FatalErrorf("can't find version %s to patch to in project %s. (aborting)\n",project->current,project->name);
		if (!checksumVerify(image->fname,image,0,0,0))
			FatalErrorf("image %s failed verification check\n",image->fname);
		checksumOpenPigs(image,image->fname,1);
		{
			char	*full_manifest;
			ZipData	*zip = &project->full_manifest;

			full_manifest = checksumMakeString(project->curr_image,project->curr_image->build_name,1);
			zip->unpack_size = strlen(full_manifest);
			zip->zip_data = zipData(full_manifest,zip->unpack_size,&zip->pack_size);
			free(full_manifest);
		}
		//makeAllFileManifest(project->curr_image,&project->full_manifest);
		patchToVersion(project,image,0);
		// can't prune files if server is running in another process
		if (run_server)
			pruneChecksumsAndPatches(project);
		sprintf(patchdir,"%s/patches/%s",project->rootpath,project->name);
		addAllPatchesToFileMgr(patchdir);
		checkMajorPatch(project);
		printf("\n");
	}
	loadPatchClient();
}

