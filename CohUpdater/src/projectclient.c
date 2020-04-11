#include "projectfile.h"
#include "EArray.h"
#include "netio.h"
#include "patchclient.h"
#include "comm_patcher.h"
#include "error.h"
#include "utils.h"
#include "netcomp.h"
#include "xlate.h"

extern int g_full_checksum,g_safe_server;

int getSmallestDiff(PatchProject *project,char *name,char *src_for_diff)
{
	int		i,cmd;
	Packet	*pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_SMALLEST_DIFF);

	pktSendString(pak,project->name);
	pktSendString(pak,name);
	for(i=0; i<project->image_count; i++)
	{
		// only allow diffs from images that pass checksum
		pktSendString(pak, project->images[i]->build_name);
	}
	pktSendString(pak,0);
	pktSend(&pak,&comm_link);
	cmd = waitForCmd(&pak);
	if (cmd == PATCHSERVER_DISCONNECTED)
		return 0;
	else if (cmd == PATCHSERVER_ERROR_MSG)
	{
		char *s = pktGetString(pak);
		if (strcmp(s, "SvrErrInvalidPatchRequest")==0 || strcmp(s, "SvrErrNotAllowedPatchRequest")==0)
		{
			// illegal project request, just ignore it.
			sprintf(src_for_diff,"%s/images/%s/%s",project->rootpath,project->name,"0");
		}
		else
		{
            msgAlertUpdater("ErrFromServer",xlateQuick(s));
		}
	}
	else if (cmd != PATCHSERVER_SMALLEST_DIFF)
		msgAlertFatal("ErrUnknownMessage", cmd);
	else
		sprintf(src_for_diff,"%s/images/%s/%s",project->rootpath,project->name,pktGetString(pak));
	pktFree(pak);
	return 1;
}

ImageCheck * projectGetServerImageChecksum(char *project_name, char *image_name)
{
	int cmd;
	Packet *pak;
	ImageCheck *image = 0;
	char *checksum_string;

	pak = pktCreateEx(&comm_link, PATCHCLIENT_REQ_IMAGE_CHECKSUM);
	pktSendString(pak, project_name);
	pktSendString(pak, image_name);
	pktSend(&pak,&comm_link);

	cmd = waitForCmd(&pak);
	if (cmd == PATCHSERVER_DISCONNECTED)
		return 0;
	else if (cmd == PATCHSERVER_IMAGE_CHECKSUM)
	{
		checksum_string = pktGetZipped(pak, 0);
		image = (ImageCheck *)malloc(sizeof(ImageCheck));
		ZeroMemory(image, sizeof(ImageCheck));
		checksumLoadFromMem(checksum_string, image,1);
	}
	else if ( cmd == PATCHSERVER_ERROR_MSG )
	{
		msgAlertUpdater("ErrFromServer",xlateQuick(pktGetString(pak)));
	}

	return image;
}

int removeImage(PatchProject *project, int imageIdx)
{
	int i;
	 char dst_dir[MAX_PATH];

	 if (stricmp(project->images[imageIdx]->build_name, "0")==0)
		 return imageIdx;

	sprintf(dst_dir,"%s/images/%s/%s",project->rootpath,project->name,project->images[imageIdx]->build_name);
	printf("Deleting image: %s\n",dst_dir);
	removeDirAll(dst_dir);

	// remove checksum file
	sprintf(dst_dir,"%s/checksums/%s/%s.checksum",project->rootpath,project->name,project->images[imageIdx]->build_name);
	printf("Deleting checksum: %s\n",dst_dir);
	safeDeleteFile(dst_dir);

	// remove patches
	sprintf(dst_dir,"%s/patches/%s/%s",project->rootpath,project->name,project->images[imageIdx]->build_name);
	printf("Deleting patch directory: %s\n",dst_dir);
	removeDirAll(dst_dir);

	// remove patches
	sprintf(dst_dir,"%s/patches/%s",project->rootpath,project->name);
	printf("Deleting patches for %s from: %s\n",project->images[imageIdx]->build_name,dst_dir);
	{
		char	**names;
		int		count,i;
		char matchStr1[MAX_PATH],matchStr2[MAX_PATH];
		sprintf(matchStr1,"%s.patch",project->images[imageIdx]->build_name);
		sprintf(matchStr2,"%s.patchcmds",project->images[imageIdx]->build_name);

		names = getDirFiles(dst_dir,&count);
		for(i=0;i<count;i++)
		{
			if (strEndsWith(names[i], matchStr1) || strEndsWith(names[i], matchStr2))
			{
				printf(" Deleting: %s\n",names[i]);
				safeDeleteFile(names[i]);
			}
		}		
		freeDirFiles(names,count);
	}

	checksumFree(project->images[imageIdx]);

	project->image_count--;

	for (i = imageIdx; i < project->image_count; i++)
		project->images[i] = project->images[i+1];

	project->images[project->image_count] = NULL;

	return imageIdx-1;
}

int isPatchVersionAllowed(PatchProject *project, char *name)
{
	int n, num, num_matched = 0;

	// check SyncAllow
	num = eaSize(&project->sync_allows);
	for (n = 0; n < num; n++)
	{
		if (simpleMatchExact(project->sync_allows[n]->str, name))
			num_matched++;
	}

	// allow all if there is no SyncAllow, otherwise enforce at least one match.
	if (num && !num_matched)
		return 0;


	// check SyncBlock
	num = eaSize(&project->sync_blocks);
	for (n = 0; n < num; n++)
	{
		if (simpleMatchExact(project->sync_blocks[n]->str, name))
			return 0;
	}

	return 1;
}

void projectSync(char * config_filename)
{
	int				i,cmd,num_projects;
	PatchProject	*project;
	Packet			*pak;
	ImageCheck		*image;
	int				tries;

	// NOTE: projectLoadConfig only adds images to project lists if they
	//       pass the checksum, so later we do not have to reverify the
	//       checksum, just check if the desired image is in the list
	projectLoadConfig(config_filename,g_full_checksum,1);

	printf("Connecting to server...\n\n");
	connectToServer();

	num_projects = eaSize(&project_list.projects);
	for(i=0;i<num_projects;i++)
	{
		project = project_list.projects[i];
		printf("%d/%d: synchronizing project %s\n",i+1,num_projects,project->name);

		pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_PROJECT_LIST);
		pktSendString(pak,project->name);
		pktSend(&pak,&comm_link);
		cmd = waitForCmd(&pak);
		if (cmd == PATCHSERVER_DISCONNECTED)
		{
			connectToServer();
			i--;
			continue;
		}
		else if (cmd == PATCHSERVER_PROJECT_LIST)
		{
			int		j,count,name_count=0,name_max=0;
			char	src_dir[MAX_PATH],dst_dir[MAX_PATH],*name,**names=0;

			count = pktGetBitsPack(pak,1);
			for(j=0;j<count;j++)
			{
				name = pktGetString(pak);
				if (stricmp(name,"0")==0)
					continue;

				if (!isPatchVersionAllowed(project, name))
					continue;
				
				image = findImageByName(project,name);
				sprintf(dst_dir,"%s/images/%s/%s",project->rootpath,project->name,name);

				if (image)
				{
					ImageCheck *svr_image = 0;
					
					while (!(svr_image = projectGetServerImageChecksum(project->name, name)))
					{
						if (lostServerLink())
							connectToServer();
						else
							break;
					}

					if (svr_image && !checksumVerify(dst_dir, svr_image, 0, 0, 0))
					{
						int k;

						// remove bad image checksum from project
						for (k = 0; k < project->image_count; k++)
						{
							if (project->images[k] == image)
								break;
						}

						assert(k < project->image_count);

						removeImage(project, k);

						// mark as invalid
						image = 0;
					}
				}

				if (image)
				{
					// valid image, we don't need to patch it
					image->exists_on_server = 1;
				}
				else
				{
					// image does not exist or is invalid, we need to patch it
					dynArrayAddp(&names,&name_count,&name_max,strdup(name));
				}
			}

			for(j=0;j<name_count;j++)
			{
				name = names[j];
				sprintf(dst_dir,"%s/images/%s/%s",project->rootpath,project->name,name);

				tries = 0;
				do
				{
					// remove invalid image (if it exists)
					removeDirAll(dst_dir);

					// limit number of retries due to corrupted image transfer
					assert(tries < 4);
					tries++;

					// request server to tell us which image to patch from
					while (!getSmallestDiff(project,name,src_dir))
					{
						if (lostServerLink())
							connectToServer();
						else
							msgAlertFatal("ErrUnknown");
					}

					if (tries == 1)
						printf(" %d/%d: Image %s\n", j+1, name_count, name);
					else
						printf(" Retry: Image %s\n", name);

					if (stricmp(dst_dir,src_dir)==0)
						printf("  No delta\n");
					else
						printf("  Delta from %s\n",getFileName(src_dir));

					// get the new image and overwrite its checksum file with the server's checksum
					while (!clientPatch(project->name,src_dir,dst_dir,name,0))
					{
						if (lostServerLink())
							connectToServer();
						else
							msgAlertFatal("ErrImagePatchFail",dst_dir);
					}

					// check downloaded image for corruption (full check against downloaded checksum) and add to project if ok
					image = projectReloadChecksum(project,0,1,CHECKSUM_ALWAYS,dst_dir,name);
				} while (!image);

				// now that this image is valid, add to valid image list
				image->exists_on_server = 1;
				free(names[j]);
			}
			free(names);

			// remove images that are not on the server
			if (!project->no_sync_delete)
			{
				for(j=0;j<project->image_count;j++)
				{
					image = project->images[j];
					if (image->exists_on_server)
						continue;
					j = removeImage(project, j);
				}
			}
		}
		else
		{
			if ( cmd == PATCHSERVER_ERROR_MSG )
			{
				msgAlertUpdater("ErrFromServer",xlateQuick(pktGetString(pak)));
			}
			printf("Cant get project list for %s from server\n\n",project->name);
		}
		pktFree(pak);
	}
}


