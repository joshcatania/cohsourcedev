#include <sys/stat.h>
#include "patchclient.h"
#include "netio.h"
#include "comm_patcher.h"
#include "netcomp.h"
#include "filechecksum.h"
#include "patchfileutils.h"
#include "patchapply.h"
#include <assert.h>
#include "timing.h"
#include "mathutil.h"
#include "utils.h"
#include "sock.h"
#include "crypt.h"
#include "RegistryReader.h"
#include "patchui.h"
#include "netxfer.h"
#include "net_socket.h"
#include "patchutils.h"
#include "xlate.h"
#include "MemoryMonitor.h"
#include "StashTable.h"

#define LOGID "CohUpdater"

NetLink comm_link;
extern char reg_key_dir[];
extern int g_lag,g_send_size,g_recv_size;
extern int g_get_major_patch;
extern int g_COV;

char	patchserver_name[1000];
U32		patchserver_port;
int		glob_perf_test;
extern int isFreshInstall;
extern int isClientOnly;
extern int g_bUseUI;
char *ipString;

void connectToServer()
{
	int		cmd,status,count=0;
	char	buf[300];
	static	int	try_count,timer;
	extern void patchClientGetLatestExe();
	int		orig_port = patchserver_port;
	int		next_port_idx = (orig_port == valid_updateserver_ports[0]) ? 1 : 0;
	#define		RETRY_SECONDS	60.0

	if (!timer)
		timer = timerAlloc();
	for(;;)
	{
		sprintf(buf,"StatsConnecting #%d",++count);
		
		if ( g_bUseUI )
			xferStatsInit(buf,NULL,0,0);

		timerStart(timer);
		ipString = makeIpStr(ipFromString(patchserver_name));
		printf("Connecting to %s : %d...\n", ipString, patchserver_port);
		SetConsoleTitle( ipString );
		if (netConnect(&comm_link,patchserver_name,patchserver_port,NLT_TCP,RETRY_SECONDS,0))
		{
			Packet	*pak = pktCreateEx(&comm_link,PATCHCLIENT_CONNECT);
			
			printf("Connected\n");
			pktSend(&pak,&comm_link);

			cmd = waitForCmd(&pak);
			if (cmd == PATCHSERVER_DISCONNECTED)
				continue;
			else if (cmd != PATCHSERVER_CONNECT_STATUS)
			{
				if ( cmd == PATCHSERVER_ERROR_MSG )
				{
					msgAlertUpdater("ErrFromServer",xlateQuick(pktGetString(pak)));
				}
				else
					msgAlertFatal("ErrUnknownMessage",cmd);
			}
			status = pktGetBitsPack(pak,1);
			pktFree(pak);
			if (status == 0)
			{
				if (winMsgOkCancel("UpdateServer is full, please wait a moment and try again."))
				{
					netSendDisconnect(&comm_link,5.f);
					continue;
				}
				else
					exit(0);
			}
			comm_link.sendQueuePacketMax = 0;
			qSetMaxSizeLimit(comm_link.receiveQueue, 0);
			//sockSetBlocking(comm_link.socket, 1);

			patchClientGetLatestExe();

			if ( g_bUseUI )
				xferStatsFinish();

			if (g_lag)
				lnkSimulateNetworkConditions(&comm_link,g_lag,0,0,0);
			if (g_send_size)
				socketSetBufferSize(comm_link.socket, SO_SNDBUF, g_send_size);
			if (g_recv_size)
				socketSetBufferSize(comm_link.socket, SO_RCVBUF, g_recv_size);
			return;
		}
		else
		{
			// flush the dns
			static	int	already_flushed;

			printf("Failed to connect\n");
			if (!already_flushed)
			{
				printf("Flushing DNS\n");
				already_flushed = 1;
				ShellExecute( NULL, "open", "ipconfig.exe", "/flushdns", "", SW_HIDE );
			}
			while(timerElapsed(timer) < RETRY_SECONDS)
				Sleep(1);
			
			// try a different port
			// BH: all of the below code that looks like crap or doesnt make sense was written to
			// include the "orig_port" as a valid port (at 'index' -1) if it wasnt one of the 
			// default ports already
			patchserver_port = next_port_idx < 0 ? orig_port : valid_updateserver_ports[next_port_idx];
			next_port_idx++;
			if ( next_port_idx >= NUM_VALID_UPDATESERVER_PORTS )
			{
				int i;
				for ( i = 0; i < NUM_VALID_UPDATESERVER_PORTS; ++i )
					if ( orig_port == valid_updateserver_ports[i] )
						next_port_idx = 0;
				if ( next_port_idx != 0 )
					next_port_idx = -1;
			}
		}

	//	memMonitorDisplayStats();
	}

	if ( g_bUseUI )
		xferStatsFinish();
}

void disconnectFromServer()
{
	netSendDisconnect(&comm_link,10.f);
}

static int sendManifest(char *project_name,char *manifest,char *target_name)
{
	Packet	*pak = pktCreateEx(&comm_link,PATCHCLIENT_MANIFEST);

	if (!manifest)
		manifest = "";
	pktSendString(pak,project_name);
	pktSendString(pak,target_name);
	pktSendZipped(pak,strlen(manifest),manifest);
	pktSend(&pak,&comm_link);
	return 1;
}
extern char LangWebPageKey[256], *WebPageKey;

void getRegKeys(Packet *pak)
{
	int			i,count,foundLangWebPage=0;
	char		*name,*value;
	RegReader	reader = {0};

	count = pktGetBitsPack(pak,1);
	if (!count)
		return;

	if (reg_key_dir[0])
	{
		reader = createRegReader();
		initRegReader(reader, reg_key_dir);
	}
	for(i=0;i<count;i++)
	{
		name = strdup(pktGetString(pak));
		value = strdup(pktGetString(pak));
		if (reg_key_dir[0])
		{
			rrWriteString(reader,name,value);
			if (stricmp(name,LangWebPageKey)==0)
			{
				patchShowWebPage(value);
				foundLangWebPage = 1;
			}
			else if (!foundLangWebPage && stricmp(name, WebPageKey)==0)
				patchShowWebPage(value);
		}
		free(name);
		free(value);
	}
	if (reg_key_dir[0])
		destroyRegReader(reader);
}

void setRegVersion(char *version)
{
	RegReader	reader;

	reader = createRegReader();
	initRegReader(reader, reg_key_dir);
	rrWriteString(reader, "CurrentVersion", version);
	destroyRegReader(reader);
}

static int patchPig(ImageCheck *src,ImageCheck *dst,char *pig_name,char *src_dir,char *dst_dir)
{
	int			i;
	FileCheck	*src_file,*dst_file;
	NewPigEntry	*entries=0,*entry,**xfer_entries=0;
	int			entry_count=0,entry_max=0,curr_bytes_req=0,xfer_entry_count=0;
	PigCache	cache={0};

	xfer_entries = malloc(sizeof(xfer_entries[0])*dst->file_count);
	for(i=0;i<dst->file_count;i++)
	{
		Checksum	check;
		U32			size=0;
		U8			*mem;

		dst_file = dst->files[i];
		if (!dst_file->pig_name || stricmp(dst_file->pig_name,pig_name)!=0)
			continue;
		stashFindPointer(src->filenames,dst_file->name, &src_file);
		entry = dynArrayAdd(&entries,sizeof(entries[0]),&entry_count,&entry_max,1);
		entry->fname		= dst_file->name;
		entry->timestamp	= dst_file->timestamp;
		entry->size			= dst_file->full.size;
		entry->pack_size	= dst_file->data_pack_size;
		if (!entry->pack_size)
			entry->dont_pack = 1;
		memcpy(entry->checksum,dst_file->full.values,sizeof(entry->checksum));
		mem = 0;
		if (src_file && checksumMatch(&dst_file->full,&src_file->full))
		{
			mem = loadFileData(&cache,src_dir,src_file->name,pig_name,&size,0);
			if (mem)
				checksumMem(mem,size,check.values);
			check.size = size;
			if (!checksumMatch(&dst_file->full,&check))
			{
				free(mem);
				mem = 0;
			}
		}
		if (mem)
		{
			entry->size		= size;
			entry->data		= loadFileData(&cache,src_dir,src_file->name,pig_name,&entry->pack_size,1);
			if (!entry->data)
				entry->data		= mem;
			else
				free(mem);
			assert(entry->data);
		}
	}
	closeOpenPigs(&cache);
	for(i=0;i<entry_count;i++)
	{
		if (!entries[i].data)
			xfer_entries[xfer_entry_count++] = &entries[i];
	}
	if (!netGetFiles(xfer_entries,xfer_entry_count))
		return 0;

	stashFindPointer(dst->filenames,pig_name, &dst_file);
	safeWritePigFile(pig_name,entries,entry_count,dst,dst_dir,dst_file->timestamp);

	for(i=0;i<entry_count;i++)
		free(entries[i].data);
	free(entries);
	free(xfer_entries);
	return 1;
}

int handleFullManifest(Packet *pak,char *checksum_name,char *project_name,ImageCheck *src,char *src_dir,char *dst_dir)
{
	char		buf[1000],*manifest,*checksum_text;
	ImageCheck	dst = {0};
	FileCheck	*src_file,*dst_file;
	int			i;
	U32			byte_count=0;
	ImageCheck  *src_copy = 0;
	extern int	g_full_checksum;

	manifest = pktGetZipped(pak,0);
	checksum_text = pktGetZipped(pak,0);
	if (!checksumLoadFromMem(manifest,&dst,1))
	{
		printf( "Failed to load checksum from memory\n" );
		return 0;
	}
	// if they are running a cov updater and trying to patch to a version below 10, dont let them
	if ( g_COV && getUpdateVersion(dst.build_name) < 10 )
	{
		printf( "Trying to patch CoV to a version below 10\n" );
		msgAlertUpdater("ErrPatchCovToCoh");
		exit(0);
	}
	checksumOpenPigs(src,src_dir,1);
	for(i=0;i<dst.file_count;i++)
	{
		dst_file = dst.files[i];
		if (strEndsWith(dst_file->name,".pigg"))
			continue;
		stashFindPointer(src->filenames,dst_file->name, &src_file);
		if (!src_file || !checksumMatch(&dst_file->full,&src_file->full))
			byte_count += dst_file->data_pack_size ? dst_file->data_pack_size : dst_file->full.size;
	}
	xferStatsInit("StatsFixingFiles",NULL,0,byte_count);
	src_copy = checksumDuplicate(src,0);
	for(i=0;i<dst.file_count;i++)
	{
		dst_file = dst.files[i];
		if (dst_file->pig_name)
			continue;
		stashFindPointer(src->filenames,dst_file->name, &src_file);
		if (!src_file || src_file->full_status != CHECKSUM_MATCHES_MANIFEST || !checksumMatch(&dst_file->full,&src_file->full))
		{
			char	fname[MAX_PATH];

			//sprintf(buf,"Fixing file: %s",dst_file->name);
			sprintf(buf, "%s: %s", xlateQuick("StatsFixingFiles"), dst_file->name);
			printf( "%s\n", buf );
			xferStatsSetMode(buf);
			if (strEndsWith(dst_file->name,".pigg"))
			{
				if (!patchPig(src,&dst,dst_file->name,src_dir,dst_dir))
				{
					printf( "patchPig failed on %s/%s\n", dst_dir, dst_file->name );
					checksumFree(src_copy);
					return 0;
				}
			}
			else
			{
				NewPigEntry	entry = {0},*e=&entry;

				e->fname	= dst_file->name;
				e->size		= dst_file->full.size;
				e->pack_size= dst_file->data_pack_size;
				if (!netGetFiles(&e,1))
				{
					printf( "netGetFiles failed on %s/%s\n", dst_dir, dst_file->name );
					checksumFree(src_copy);
					return 0;
				}
				sprintf(fname,"%s/%s",dst_dir,dst_file->name);
				safeOverwriteFileAndChecksum(fname,"wb",e->data,e->size,&dst_file->full,1);
				safeFileSetTimestamp(fname,dst_file->timestamp);
				free(e->data);
			}

			checksumUpdateFile(src_copy,dst_file);
			checksumWrite(src_copy,checksum_name);

			g_full_checksum = 0;
		}
	}
	xferStatsFinish();
	safeWriteFile(checksum_name,"wb",checksum_text,strlen(checksum_text));
	removeEmptyDirs(dst_dir);
	return 1;
}

static int finishServerPatch(char *checksum_name,char *project_name,char *build_name,char *dst_dir)
{
	char	server_checksum[MAX_PATH],fullpath[MAX_PATH];

	if (!build_name)
		return 0;
	sprintf(checksum_name,"%s/%s.checksum",dst_dir,project_name);
	sprintf(server_checksum,"checksums/%s/%s.checksum",project_name,build_name);
	makefullpath(server_checksum,fullpath);
	safeMkDirTree(fullpath);
	safeCopyFile(checksum_name,fullpath);
	safeDeleteFile(checksum_name);
	return 1;
}

int clientPatch(char *project_name,char *src_dir,char *dst_dir,char *build_name,int default_patch)
{
	ImageCheck	client_image = {0};
	PatchHeader	*patch=0;
	char		checksum_name[MAX_PATH],client_patch_name[MAX_PATH],major_patch_name[MAX_PATH],*curr_patch_name = client_patch_name;
	int			ret,cmd;
	Packet		*pak;
	extern int	g_full_checksum,g_got_patch;

	sprintf(client_patch_name,"%s/%s.patch",dst_dir,project_name);
	sprintf(major_patch_name,"%s/%s.majorpatch",dst_dir,project_name);
	filelog_printf( LOGID, __FUNCTION__ ": client_patch_name=%s major_patch_name=%s", client_patch_name, major_patch_name);

	if (!makeUserDir(client_patch_name))
		return -1;
	if (!makeLockFile(dst_dir))
		msgAlertFatal("ErrAlreadyRunning");
	if (build_name)
	{
		printf( "Patching build %s\n", build_name );
		sprintf(checksum_name,"checksums/%s/%s.checksum",project_name,getFileName(src_dir));
		
	}
	else
		sprintf(checksum_name,"%s/%s.checksum",src_dir,project_name);

	filelog_printf( LOGID, __FUNCTION__ ": checksum_name=%s", checksum_name);
	for(;;)
	{
		checksumFree(&client_image);
		ret = checksumLoad(checksum_name,&client_image);
		printf( "checksumLoad returned %d\n", ret );
		filelog_printf( LOGID, __FUNCTION__ ": checksumLoad returned %d\n", ret);
		if ((!ret || !client_image.build_name) && (default_patch || build_name))
		{
			sprintf(checksum_name,"%s/%s.checksum",src_dir,project_name);
			printf( "Checksumming Image\n" );
			filelog_printf( LOGID, __FUNCTION__ ": the checksum load wasn't quite successful. checksumming the image");
			checksumImage(src_dir,checksum_name,0);
			if (!checksumLoad(checksum_name,&client_image))
			{
				printf( "Could not load checksum\n" );
				return 0;
			}
		}
		patchUiSetProjectInfo(project_name,client_image.build_name);

		printf( "Verifying checksum\n" );
		filelog_printf( LOGID, __FUNCTION__ ": verifying checksum");

		if (!checksumVerify(src_dir,&client_image,0,g_full_checksum,0))
		{
			printf( "Checksum Verify failed" );
			if (isFreshInstall && isClientOnly)
			{
				filelog_printf( LOGID, __FUNCTION__ ": isFreshInstall && isClientOnly");
				checksumImage(src_dir, checksum_name, 0);
			}
			
			if (isFreshInstall && isClientOnly && checksumLoad(checksum_name, &client_image))
			{
				printf( ", fresh install, requesting patch\n" );
				xferStatsInit("StatsRequestingPatch",NULL,0,0);
				sendManifest(project_name,client_image.manifest_string,build_name);
			}
			else
			{
				printf( ", requesting file list\n" );
				xferStatsInit("StatsRequestingFileList",NULL,0,0);
				sendManifest(project_name,0,build_name);
			}
		}
		else
		{
			printf( "Checksum Verify succeeded, requesting patch\n" );
			filelog_printf( LOGID, __FUNCTION__ ": Checksum Verify succeeded, requesting patch");
			xferStatsInit("StatsRequestingPatch",NULL,0,0);
			sendManifest(project_name,client_image.manifest_string,build_name);
		}

get_next_cmd:
		cmd = waitForCmd(&pak);
		switch(cmd)
		{
			xcase PATCHSERVER_REGKEYS:
				filelog_printf( LOGID, __FUNCTION__ ": PATCHSERVER_REGKEYS");
				getRegKeys(pak);
				goto get_next_cmd;
			xcase PATCHSERVER_PATCH:
				filelog_printf( LOGID, __FUNCTION__ ": PATCHERSERVER_BATCH");
				xferStatsFinish();
				g_got_patch = 1;
				{
					U32			size;
					Checksum	check;
					int			major_patch, patch_result;

					printf( "Getting patch file\n" );
					filelog_printf( LOGID, __FUNCTION__ ": Getting patch file");
					size = pktGetBitsPack(pak,1);
					pktGetBitsArray(pak,size*8,&check);
					if (!pktEnd(pak))
					{
						major_patch = pktGetBitsPack(pak,1);
						if (major_patch)
						{
							Checksum mpCheck;
							printf( "Major Patch\n" );
							filelog_printf( LOGID, __FUNCTION__ ": Major Patch");
							if ( getPatchFileChecksum( major_patch_name, &mpCheck ) )
							{
								if ( !checksumMatch(&mpCheck, &check) )
								{
									printf("Local majorpatch file does not match the one on the server, deleting...\n");
									filelog_printf( LOGID, __FUNCTION__ ": Local majorpatch file does not match the one on the server, deleting...");
									safeDeleteFile(major_patch_name);
								}
							}
							curr_patch_name = major_patch_name;
						}
					}
					else
						curr_patch_name = client_patch_name;
					if (!fileExists(curr_patch_name))
						safeWriteFile(curr_patch_name,"wb",&check,size);
					if (!netGetPatchFile(curr_patch_name))
					{
						printf( "Failed to get patch file\n" );
						filelog_printf( LOGID, __FUNCTION__ ": Failed to get patch file");
						if (!lostServerLink())
						{
							printf( "Lost connection to server\n" );
							filelog_printf( LOGID, __FUNCTION__ ": Lost connection to server");
							safeDeleteFile(curr_patch_name);

							xferStatsInit("StatsRequestingFileList",NULL,0,0);
							sendManifest(project_name,0,build_name);
							goto get_next_cmd;
						}

						ShellExecute( NULL, "open", "ipconfig.exe", "/flushdns", "", SW_HIDE );
						return 0;
					}

					patch_result = applyPatch(project_name,curr_patch_name,checksum_name,src_dir,dst_dir,client_image.manifest_string ? 1 : 0);

					printf( "applyPatch returned %d\n", patch_result );
					filelog_printf( LOGID, __FUNCTION__ ": applyPatch returned %d\n", patch_result);

					if (patch_result < 0)
						return -2;
					if (patch_result == 0)
						return -3;

					curr_patch_name = client_patch_name;
					if (finishServerPatch(checksum_name,project_name,build_name,dst_dir))
						return 1;
					src_dir = dst_dir;
					sprintf(checksum_name,"%s/%s.checksum",src_dir,project_name);
				}
			xcase PATCHSERVER_FULL_MANIFEST_COMING:
				filelog_printf( LOGID, __FUNCTION__ ": PATCHSERVER_FULL_MANIFEST_COMING");
				xferStatsInit("StatsDownloadingFileManifest",NULL,0,0);
				goto get_next_cmd;
			xcase PATCHSERVER_FULL_MANIFEST:
				filelog_printf( LOGID, __FUNCTION__ ": PATCHSERVER_FULL_MANIFEST");
				xferStatsFinish();

				// if we are fixing files, something went wrong with the patch
				if (!strEndsWith(client_patch_name,".majorpatch"))
                    safeDeleteFile(client_patch_name);

				if (build_name)
				{
					sprintf(checksum_name,"checksums/%s/%s.checksum",project_name,build_name);
					safeMkDirTree(checksum_name);
				}
				printf( "Handling full manifest\n" );
				filelog_printf( LOGID, __FUNCTION__ ": Handling full manifest");
				if (!handleFullManifest(pak,checksum_name,project_name,&client_image,src_dir,dst_dir))
					return 0;
				if (build_name)
					return 1;
			xcase PATCHSERVER_UP_TO_DATE:
				printf( "Client is up to date..." );
				filelog_printf( LOGID, __FUNCTION__ ": PATCHSERVER_UP_TO_DATE Client is up to date...");
				// no need for a patch file if we are up to date
				if (!strEndsWith(client_patch_name,".majorpatch"))
					safeDeleteFile(client_patch_name);

				free(client_image.build_name);
				client_image.build_name = strdup(pktGetString(pak));
				{
					int			major_patch_available=0;
					Checksum	check;

					if (!pktEnd(pak))
						major_patch_available = pktGetBitsPack(pak,1);
					if (major_patch_available)
					{
						pktGetBitsArray(pak,sizeof(check)*8,&check);
						if (!fileExists(major_patch_name))
						{
							safeWriteFile(major_patch_name,"wb",&check,sizeof(check));
							g_get_major_patch = 1;
						}
						else
						{
							FILE *file = safeFopen(major_patch_name,"rb");
							Checksum	file_check;

							fread(&file_check,sizeof(file_check),1,file);
							fclose(file);
							if (!checksumMatch(&file_check,&check))
							{
								safeDeleteFile(major_patch_name);
								safeWriteFile(major_patch_name,"wb",&check,sizeof(check));
							}
							if (fileSize(major_patch_name) != check.size + sizeof(check))
								g_get_major_patch = 1;
						}
					}
					else
						safeDeleteFile(major_patch_name);
				}

				xferStatsInit("StatsOK",NULL,0,0);
				setRegVersion(client_image.build_name);
				patchUiSetProjectInfo(project_name,client_image.build_name);
				return 1;
			xcase PATCHSERVER_ERROR_MSG:
				{
					char *s = pktGetString(pak);
					
					filelog_printf( LOGID, __FUNCTION__ ": PATCHSERVER_ERROR_MSG %s", s);
					if (strcmp(s, "SvrErrInvalidPatchRequest")==0)
					{
						// illegal project request, just ignore it.
						// pretend we are up to date.
						return 1;
					}
					else if (strcmp(s, "SvrErrNotAllowedPatchRequest")==0)
					{
						msgAlertFatal("ErrFromServer",xlateQuick(s));
						return 1;
					}
					else
					{
						msgAlertFatal("ErrFromServer",xlateQuick(s));
					}
					return 0;
				}
			xcase PATCHSERVER_DISCONNECTED:
				return 0;
		xdefault:
				 filelog_printf( LOGID, __FUNCTION__ ": unknown command %i", cmd );
		}
		filelog_printf( LOGID, __FUNCTION__ ": REPEATING LOOP");
	}
}

void patchGetMajor(char *project_name,char *dst_dir)
{
	char	major_patch_name[MAX_PATH];

	if (!makeLockFile(dst_dir))
		msgAlertFatal("ErrAlreadyRunning");
	sprintf(major_patch_name,"%s/%s.majorpatch",dst_dir,project_name);
	if (!fileExists(major_patch_name))
		msgAlertFatal("ErrPatchFileHeader",major_patch_name);
	while(!netGetPatchFile(major_patch_name))
	{
		if (!lostServerLink())
			safeDeleteFile(major_patch_name);
	}
	cleanupLockFile();
}

void patchClientInit(char *ps_name,int ps_port)
{
	sockStart();
	packetStartup(0,0); // JE: Not initializing encryption here, it'll start faster, and we don't care about/use encryption on PatchClient links anyway
	if (ps_name)
		strcpy(patchserver_name,ps_name);
	if (ps_port)
		patchserver_port = ps_port;
}
