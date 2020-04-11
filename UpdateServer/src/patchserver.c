#include <conio.h>
#include "patchserver.h"
#include "projects.h"
#include "sock.h"
#include "netio.h"
#include "comm_patcher.h"
#include "utils.h"
#include "patchfileutils.h"
#include "zlib.h"
#include <assert.h>
#include "error.h"
#include "netcomp.h"
#include "filemgr.h"
#include "EArray.h"
#include "patchcreate.h"
#include "patchutils.h"
#include "timing.h"
#include "net_socket.h"
#include "MemoryMonitor.h"
#include "StashTable.h"
#include "patchserverlistener.h"
#include "sysutil.h"

static NetLinkList net_links[NUM_VALID_UPDATESERVER_PORTS];

__int64 total_bytes_sent;
__int64 timer = 0;
extern Checksum *coh_majorpatch_checksum;

typedef struct
{
	U32				last_total_bytes;
	PatchProject	*project;
	NetLink			*link;
	int				resized;

	// Throttling
	int				throttled	: 1;	// Is this link currently throttled?
	Queue			throttledQueue;		// Packet queue when a connection is throttled.

	int				timestamp;
} PatchClientLink;

#define INITIAL_CLIENT_COUNT 1000 // will auto-resize if needed

char *ipStr(NetLink *link)
{
	return makeIpStr(link->addr.sin_addr.S_un.S_addr);
}

int numClients(void)
{
	int i, total_clients = 0;
	for ( i = 0; i < NUM_VALID_UPDATESERVER_PORTS; ++i )
	{
		total_clients += net_links[i].links->size;
	}
	return total_clients;
}

int patchClientCreateCb(NetLink *link)
{
	PatchClientLink	*client;

	client = link->userData;
	client->link = link;
	client->resized = 0;
	client->throttled = 0;
	client->throttledQueue = createQueue();
	client->timestamp = timer;
	link->sendQueuePacketMax = 5000000/1000;
	log_printf("connections","%s conn",ipStr(link));
	return 1;
}

int patchClientDeleteCb(NetLink *link)
{
	PatchClientLink	*client = link->userData;

	destroyQueue( client->throttledQueue );
	//printf("del link at %s\n",makeIpStr(link->addr.sin_addr.S_un.S_addr));
	log_printf("connections","%s disc Xferred %u",makeIpStr(link->addr.sin_addr.S_un.S_addr),link->totalBytesSent);
	return 1;
}

static void sendError(NetLink *link,char *msg)
{
	Packet	*pak_out = pktCreateEx(link,PATCHSERVER_ERROR_MSG);
	pktSendString(pak_out,msg);
	pktSend(&pak_out,link);
	log_printf("errors","%s %s",ipStr(link),msg);
	log_printf("connections","%s %s",ipStr(link),msg);
}

static void sendRegKeys(NetLink *link,PatchProject *project)
{
	int		i;
	Packet		*pak = pktCreateEx(link,PATCHSERVER_REGKEYS);

	pktSendBitsPack(pak,1,eaSize(&project->regkeys));
	for(i=0;i<eaSize(&project->regkeys);i++)
	{
		pktSendString(pak,project->regkeys[i]->name);
		pktSendString(pak,project->regkeys[i]->value);
	}
	pktSend(&pak,link);
}

static Checksum *checkPatchExists(NetLink *link,PatchProject *project,char *from,char *to)
{
	ImageCheck	*src,*dst;
	char		patch_name[MAX_PATH];
	Checksum	*check;

	from = getFileName(from);
	to = getFileName(to);
	sprintf(patch_name,"%s/patches/%s/%s/%s.patch",project->rootpath,project->name,to,from);
	if (check=filemgrGetChecksum(patch_name))
		return check;
	src = findImageByName(project,from);
	dst = findImageByName(project,to);

	if (dst == src || !dst)
	{
		char	buf[1000];

		if (link)
		{
			sprintf(buf,"invalid target image %s for source image %s",to,from);
			sendError(link,buf);
		}
		return 0;
	}
	printf(" Patch for %s/%s (requested by %s)\n",project->name,dst->build_name,link ? makeIpStr(link->addr.sin_addr.S_un.S_addr) : "Internal");
	patchCreate(src,dst,patch_name,"",project->no_diff);
	filemgrAdd(patch_name);
	return filemgrGetChecksum(patch_name);
}

static int sendPatchCmd(NetLink *link,PatchProject *project,char *from,char *to,int major_patch)
{
	Checksum	*check;
	Packet		*pak;

	if (!(check=checkPatchExists(link,project,from,to)))
		return 0;
	log_printf("connections","%s SentPatchCmd \"%s\" from \"%s\" to \"%s\"",ipStr(link),project->name,from,to);
	pak = pktCreateEx(link,PATCHSERVER_PATCH);
	pktSendBitsPack(pak,1,sizeof(*check));
	pktSendBitsArray(pak,8*sizeof(*check),check);
	pktSendBitsPack(pak,1,major_patch);
	total_bytes_sent += pak->stream.size;
	pktSend(&pak,link);
	return 1;
}

static int sendFullManifest(NetLink *link,PatchProject *project)
{
	ZipData		*manifest = &project->full_manifest;
	Packet		*pak;

	log_printf("connections","%s SendManifest %s",ipStr(link),project->name);
	pak = pktCreateEx(link,PATCHSERVER_FULL_MANIFEST_COMING);
	pktSend(&pak,link);

	pak = pktCreateEx(link,PATCHSERVER_FULL_MANIFEST);
	pktSendZippedAlready(pak,manifest->unpack_size,manifest->pack_size,manifest->zip_data);
	pktSendZipped(pak,strlen(project->curr_image->manifest_string),project->curr_image->manifest_string);
	total_bytes_sent += pak->stream.size;
	pktSend(&pak,link);
	return 1;
}

int sendServerPatch(NetLink *link,PatchProject *project,ImageCheck *client_image,char *build_name)
{
	ImageCheck	*image;
	char		err_msg[1000] = "";

	image = matchImage(project->images,project->image_count,client_image,err_msg);
	if (!image)
	{
		char	buf[1000];
		sprintf(buf,"%s\nMust patch from valid server image.",err_msg);
		sendError(link,buf);
		return 0;
	}
	return sendPatchCmd(link,project,image->fname,build_name,0);
}

int patchSize(PatchProject *project,char *from,char *to)
{
	Checksum	*check;

	check = checkPatchExists(0,project,from,to);
	if (check)
		return check->size;
	return -1;
}

void sendClientPatch(NetLink *link,PatchProject *project,ImageCheck *client_image)
{
	ImageCheck	*image;
	char		err_msg[1000] = "",*build_name = project->current;
	MajorPatch	*mp = 0;

	if (project->major_patch)
		mp = project->major_patch[0];
	image = matchImage(project->images,project->image_count,client_image,err_msg);
	if (!image)
	{
		log_printf("connections","%s ManifestMismatch \"%s\" Build \"%s\"",ipStr(link),project->name,client_image->build_name);
		sendFullManifest(link,project);
		return;
	}
	else if (image == project->curr_image)
	{
		Packet		*pak;
		int sendMP = (mp && compareUpdateVersion(mp->to, image->build_name) >= 0);

		pak = pktCreateEx(link,PATCHSERVER_UP_TO_DATE);
		pktSendString(pak,image->build_name);
		pktSendBitsPack(pak,1,sendMP?1:0);
		if (sendMP)
		{
			Checksum	*check;

			check = filemgrGetChecksum(mp->server_name);
			pktSendBitsArray(pak,8*sizeof(*check),check);
		}
		pktSend(&pak,link);
		log_printf("connections","%s PatchedOK",ipStr(link));
	}
	else
	{
		if (project->major_patch)
		{
			int			size_to_major,size_from_major,size_direct;

			if (image == findImageByName(project,mp->from))
				size_to_major = 0;
			else
				size_to_major = patchSize(project,image->fname,mp->from);

			if (project->curr_image == findImageByName(project,mp->to))
				size_from_major = 0;
			else
				size_from_major = patchSize(project,mp->to,project->current);

			size_direct = patchSize(project,image->fname,project->current);
			if ((size_to_major >=0 && size_from_major >= 0 && size_to_major + size_from_major < size_direct))
			{
				if (!size_to_major)
				{
					sendPatchCmd(link,project,mp->from,mp->to,1);
					return;
				}
				build_name = mp->from;
			}
		}
		sendPatchCmd(link,project,image->fname,build_name,0);
	}
}

static PatchProject *getProject(Packet *pak,NetLink *link)
{
	char		*project_name;
	PatchProject	*project;

	project_name = pktGetString(pak);
	log_printf("connections","%s ReqProject \"%s\"",ipStr(link),project_name);
	project = projectFind(project_name);
	if (!project)
	{
		sendError(link,"SvrErrInvalidPatchRequest");
		return 0;
	}
	if (!projectCheckAccess(project,link->addr.sin_addr.S_un.S_addr))
	{
		sendError(link,"SvrErrNotAllowedPatchRequest");
		return 0;
	}
	return project;
}

void handleReqImageChecksum(Packet *pak, NetLink *link)
{
	PatchProject	*project;
	PatchClientLink	*client = link->userData;
	ImageCheck		*image;


	project = client->project = projectFind(pktGetString(pak));
	if (!project)
	{
		sendError(link,"SvrErrUnknownProject");
		return;
	}

	image = findImageByName(project, pktGetString(pak));
	if (!image)
	{
		sendError(link,"SvrErrUnknownImage");
		return;
	}


	pak = pktCreateEx(link,PATCHSERVER_IMAGE_CHECKSUM);
	pktSendZipped(pak, strlen(image->manifest_string), image->manifest_string);
	pktSend(&pak,link);
	return;
}

void handleReqPatch(Packet *pak, NetLink *link)
{
	U32				ret;
	U8				*manifest,*build_name;
	ImageCheck		image = {0};
	PatchProject	*project;
	PatchClientLink	*client = link->userData;

	project = client->project = getProject(pak,link);
	if (!project)
		return;
	build_name = pktGetString(pak); // server-side patching will set this (clients always ask for latest)
	manifest = pktGetZipped(pak,0);
	sendRegKeys(link,project);
	if (strlen(manifest) <= 0)
	{
		if (!build_name[0])
			sendFullManifest(link,project);
		else
			sendError(link,"requested null manifest - not supported for -server option");
		goto cleanup;
	}
	ret = checksumLoadFromMem(manifest,&image,0);
	// if the requested patch and the current image for the user are the same, dont send the patch
	if (ret /*&& stricmp(image.build_name, project->current) != 0*/) 
	{
		if (!build_name || !build_name[0])
			sendClientPatch(link,project,&image);
		else
			sendServerPatch(link,project,&image,build_name);
	}
	else if ( !ret )
		sendError(link,"SvrErrBadClientManifest");
cleanup:
	checksumFree(&image);
	free(manifest);
}

void handleReqData(Packet *pak,NetLink *link)
{
	U32				pos,count,bytesread, is_majorpatch = 0;
	Checksum		check;
	U8				*buf;
	Packet			*pak_out;
	PatchClientLink	*client = link->userData;

	pos = pktGetBitsPack(pak,1);
	count = pktGetBitsPack(pak,1);
	if (count > XFER_BLOCK_SIZE)
		goto bad_request;
	pktGetBitsArray(pak,sizeof(check)*8,&check);
	if ((bytesread = filemgrGetData(&check,pos,count,&buf)) != count)
		goto bad_request;
	pak_out = pktCreateEx(link,PATCHSERVER_DATA);
	pktSendBitsPack(pak_out,1,pos);
	pktSendBitsPack(pak_out,1,count);
	pktSendBitsArray(pak_out,count*8,buf);
	if ( !client->throttled || !project_list.throttle_speed )
	{
		total_bytes_sent += pak_out->stream.size;
		pktSend(&pak_out,link);
	}
	else if ( !qEnqueue(client->throttledQueue, (void*)pak_out) )
	{
		// failed to enqueue the packet, so just send it
		total_bytes_sent += pak_out->stream.size;
		pktSend(&pak_out,link);
	}

	return;

bad_request:
	sendError(link,"SvrErrBadFileReq");
	return;
}

void handleReqProjectList(Packet *pak,NetLink *link)
{
	PatchProject	*project;
	char			*project_name = pktGetString(pak);
	Packet			*pak_out;
	int				i,first_zero_patch;

	project = projectFind(project_name);
	if (!project)
	{
		sendError(link,"non-existant project list");
		return;
	}
	pak_out = pktCreateEx(link,PATCHSERVER_PROJECT_LIST);
	pktSendBitsPack(pak_out,1,project->image_count);
	for(i=0;i<project->image_count;i++)
	{
		char	zero_patch[MAX_PATH];
		sprintf(zero_patch,"%s/patches/%s/%s/0.patch",project->rootpath,project->name,project->images[i]->build_name);
		if (filemgrGetChecksum(zero_patch))
			break;
	}
	// Try to list them in the order we have available patches
	first_zero_patch = i;
	for(i=first_zero_patch;i<project->image_count;i++)
		pktSendString(pak_out,project->images[i]->build_name);
	for(i=first_zero_patch-1;i>=0;i--)
		pktSendString(pak_out,project->images[i]->build_name);
	pktSend(&pak_out,link);
}

void handleReqProjectChecksums(Packet *pak,NetLink *link)
{
	PatchProject	*project;
	char			*project_name = pktGetString(pak);
	Packet			*pak_out;
	int				i,first_zero_patch;

	project = projectFind(project_name);
	if (!project)
	{
		sendError(link,"non-existant project list");
		return;
	}
	pak_out = pktCreateEx(link,PATCHSERVER_PROJECT_LIST);
	pktSendBitsPack(pak_out,1,project->image_count);
	for(i=0;i<project->image_count;i++)
	{
		char	zero_patch[MAX_PATH];
		sprintf(zero_patch,"%s/patches/%s/%s/0.patch",project->rootpath,project->name,project->images[i]->build_name);
		if (filemgrGetChecksum(zero_patch))
			break;
	}
	// Try to list them in the order we have available patches
	first_zero_patch = i;
	for(i=first_zero_patch;i<project->image_count;i++)
		pktSendString(pak_out,project->images[i]->build_name);
	for(i=first_zero_patch-1;i>=0;i--)
		pktSendString(pak_out,project->images[i]->build_name);
	pktSend(&pak_out,link);
}

static void handleReqNewClient(Packet *pak_in,NetLink *link, int bMultifile)
{
	Packet		*pak;
	Checksum	check;
	Checksum    *working_checksum;
	ZipData     *zipdata_to_send;
	int			client_protocol_version,dont_update;

	if ( ! bMultifile )
	{
		zipdata_to_send = &project_list.patch_client;
		working_checksum = &project_list.patch_client_check;
	}
	else
	{
		zipdata_to_send = &project_list.patch_client_multifile;
		working_checksum = &project_list.patch_client_multifile_check;
	}

	client_protocol_version = pktGetBitsPack(pak_in,1);
	dont_update = pktGetBitsPack(pak_in,1);
	pktGetBitsArray(pak_in,sizeof(check)*8,&check);
	if (dont_update)
	{
		if (client_protocol_version != PATCHER_PROTOCOL_VERSION)
		{
			sendError(link,"SvrErrProtocolMismatch");
			return;
		}
	}
	if (dont_update || checksumMatch(&check,working_checksum))
	{
		pak = pktCreateEx(link,PATCHSERVER_CLIENT_OK);
		if (dont_update)
			pktSendString(pak,0);
		else
			pktSendString(pak,project_list.client_version);
	}
	else
	{
		log_printf("connections","checksum mismatch, mine: %I64d {%08x %08x %08x %08x}, sent: %I64d {%08x %08x %08x %08x}",
			working_checksum->size, working_checksum->values[0],working_checksum->values[1], working_checksum->values[2], working_checksum->values[3],
			check.size, check.values[0],check.values[1], check.values[2], check.values[3]);

		if ( bMultifile )
		{
			pak = pktCreateEx(link,PATCHSERVER_NEW_CLIENT_MULTIFILE);
		}
		else
		{
			pak = pktCreateEx(link,PATCHSERVER_NEW_CLIENT);
		}

		pktSendZippedAlready(pak,zipdata_to_send->unpack_size,zipdata_to_send->pack_size,zipdata_to_send->zip_data);
	}

	total_bytes_sent += pak->stream.size;
	pktSend(&pak,link);
}

static U32 loadFileBytes(char *name,U8 *mem,U32 pos,U32 amt)
{
	FILE			*fp;
	U32				offset=0,bytesread;

	fp = fopen(name,"rb");
	if (!fp)
		return 0;
	fseek(fp,pos,0);
	bytesread = fread(mem,1,amt,fp);
	fclose(fp);
	return bytesread;
}

static void handleReqUpdaterFilePart(Packet* pak_in,NetLink* link)
{
	char*			szRequestedFile;
	char	        szRealFileName[MAX_PATH],szTemp[MAX_PATH];
	U32				pos,amt,bytesread;
	Packet			*pak;
	static			U8	buffer[1<<20];
	int				bSuccess = 0;
	int				bCiderUpdateFound = 0;
	int             i = 0;

	szRequestedFile = pktGetString(pak_in);

	for ( i = 0; i < MAX_CIDER_PATCH_TYPES; i++ )
	{
		if ( stricmp(project_list.cider_patches[i].cider_update_name,szRequestedFile) == 0 )
		{
			snprintf(szRealFileName,MAX_PATH,"%s\\cider-updates\\%s",getExecutableDir(szTemp),szRequestedFile);
			
			bCiderUpdateFound = 1;
			break;
		}
	}

	if (!bCiderUpdateFound)
	{
		if ( stricmp(project_list.patch_client_multifile_name,szRequestedFile) == 0 )
		{
			snprintf(szRealFileName,MAX_PATH,"%s\\%s",getExecutableDir(szTemp),szRequestedFile);
		}
		else
		{
			sendError(link,"SvrErrBadFileReq");
			return;
		}
	}

	pos		= pktGetBitsPack(pak_in,20);
	amt		= pktGetBitsPack(pak_in,20);

	if (amt < sizeof(buffer))
	{		
		bytesread = loadFileBytes(szRealFileName,buffer,pos,amt);

		if (bytesread == amt)
		{
			pak = pktCreateEx(link,PATCHSERVER_SEND_UPDATER_FILE_PART);
			pktSendBitsPack(pak,20,pos);
			pktSendBitsPack(pak,20,amt);
			pktSendBitsArray(pak,amt*8,buffer);

			total_bytes_sent += pak->stream.size;
			pktSend(&pak,link);

			bSuccess = 1;
		}
	}

	if ( !bSuccess )
	{
		sendError(link,"SvrErrBadFileReq");
	}
}

static void handleReqCiderUpdate(Packet* pak_in,NetLink* link)
{
	Packet *pak = NULL;
	char   szCiderID[CIDER_ID_SIZE];
	U32    nCiderVer;
	int    bUpdateRequired = 0;
	int    i = 0;

	szCiderID[0] = '\0';

	pktGetBitsArray(pak_in,sizeof(U32)*8,&nCiderVer);
	pktGetBitsArray(pak_in,sizeof(char)*8*4,szCiderID);

	if ( szCiderID[0] != '\0' )
	{	
		for ( i = 0; i < MAX_CIDER_PATCH_TYPES; i++ )
		{
			if ( memcmp(project_list.cider_patches[i].cider_ID,szCiderID,CIDER_ID_SIZE) == 0 )
			{
				if ( project_list.cider_patches[i].cider_version > nCiderVer )
				{
					// new version is available
					pak = pktCreateEx(link,PATCHSERVER_CIDER_UPDATE);

					pktSendString(pak,project_list.cider_patches[i].cider_update_name);
					pktSendBitsArray(pak,sizeof(U32)*8, &project_list.cider_patches[i].cider_update_size);
					
					bUpdateRequired = 1;
					break;
				}
			}
		}
	}

	if ( !bUpdateRequired )
	{
		pak = pktCreateEx(link,PATCHSERVER_CLIENT_OK);

		pktSendString(pak,0);
	}

	total_bytes_sent += pak->stream.size;
	pktSend(&pak,link);
}

static			PigCache	filexfer_pigcache;

static void *loadFile(ImageCheck *image,char *name,U32 *pack_size,U32 *unpack_size)
{
	FileCheck	*file;
	U8			*data;

	stashFindPointer(image->filenames,name, &file);
	if (!file)
		return 0;
	*pack_size = 0;
	data = loadFileData(&filexfer_pigcache,image->fname,file->name,file->pig_name,pack_size,1);
	if (data)
	{
		PigFileHeader *pfh = getPfh(&filexfer_pigcache,image->fname,file->name,file->pig_name);
		*unpack_size = pfh->size;
	}
	else
		data = loadFileData(&filexfer_pigcache,image->fname,file->name,file->pig_name,unpack_size,0);
	return data;
}

static U32 loadImageBytes(ImageCheck *image,char *name,U8 *mem,U32 pos,U32 amt)
{
	FileCheck		*file;
	PigFileHeader	*pfh = 0;
	FILE			*fp;
	U32				offset=0,bytesread;
	char			fname[MAX_PATH];

	if (!stashFindPointer(image->filenames,name, &file))
		return 0;
	if (file->pig_name)
	{
		pfh = getPfh(&filexfer_pigcache,image->fname,name,file->pig_name);
		offset = pfh->offset;
		name = file->pig_name;
	}
	sprintf(fname,"%s/%s",image->fname,name);
	fp = fopen(fname,"rb");
	if (!fp)
		return 0;
	fseek(fp,pos + offset,0);
	bytesread = fread(mem,1,amt,fp);
	fclose(fp);
	return bytesread;
}

static void handleReqFilePart(Packet *pak_in,NetLink *link)
{
	PatchProject	*project = ((PatchClientLink*)link->userData)->project;
	U32				pos,amt,bytesread;
	char			*name;
	Packet			*pak;
	static			U8	buffer[1<<20];

	name	= pktGetString(pak_in);
	pos		= pktGetBitsPack(pak_in,20);
	amt		= pktGetBitsPack(pak_in,20);
	if (amt > sizeof(buffer))
		goto err_exit;
	//if (!pos)
	//	log_printf("connections","%s ReqFile \"%s\"",ipStr(link),name);
	bytesread = loadImageBytes(project->curr_image,name,buffer,pos,amt);
	if (bytesread != amt)
		goto err_exit;

	pak = pktCreateEx(link,PATCHSERVER_SEND_FILE_PART);
	pktSendBitsPack(pak,20,pos);
	pktSendBitsPack(pak,20,amt);
	pktSendBitsArray(pak,amt*8,buffer);
	total_bytes_sent += pak->stream.size;
	pktSend(&pak,link);
	return;

err_exit:
	sendError(link,"SvrErrBadFileReq");
}

static void handleReqFiles(Packet *pak_in,NetLink *link)
{
	Packet			*pak;
	char			*data,*name;
	PatchProject	*project = ((PatchClientLink*)link->userData)->project;
	ImageCheck		*image;
	U32				size,pack_size;

	if (!project)
	{
		sendError(link,"SvrErrBadFileReq");
		return;
	}
	pak = pktCreateEx(link,PATCHSERVER_SEND_FILES);
	image = project->curr_image;
	for(;;)
	{
		name = pktGetString(pak_in);
		if (!name || !name[0])
			break;
		//log_printf("connections","%s ReqFile \"%s\"",ipStr(link),name);
		pktSendString(pak,name);
		data = loadFile(image,name,&pack_size,&size);
		if (!data)
		{
			pktFree(pak);
			sendError(link,"SvrErrBadFileReq");
			return;
		}
		pktSendBitsPack(pak,1,pack_size);
		pktSendBitsPack(pak,1,size);
		pktSendBitsArray(pak,pack_size ? pack_size*8 : size*8,data);
		free(data);
	}
	pktSendString(pak,0);
	total_bytes_sent += pak->stream.size;
	pktSend(&pak,link);
}

void handleClientConnect(NetLink *link)
{
	Packet	*pak = pktCreateEx(link,PATCHSERVER_CONNECT_STATUS);

	if (!project_list.max_clients || numClients() < project_list.max_clients)
		pktSendBitsPack(pak,1,1);
	else
		pktSendBitsPack(pak,1,0);
	pktSend(&pak,link);
	lnkBatchSend(link);
}

void handleReqSmallestDiff(Packet *pak_in,NetLink *link)
{
	PatchClientLink	*client = link->userData;
	PatchProject	*project;
	char			*src_name,patch_name[MAX_PATH],target_name[MAX_PATH],best_src_name[MAX_PATH] = "0";
	Checksum		*check;
	ImageCheck		*dst_image,*src_image;
	__int64			size,diff_size = 0x7fffffffffffffff;
	FileCheck		*dst_file,*src_file;
	int				i;

	project = getProject(pak_in,link);
	if (!project)
		return;
	strncpy(target_name,pktGetString(pak_in),sizeof(target_name)-1);
	dst_image = findImageByName(project,target_name);
	if (!dst_image)
	{
		sendError(link,"server doesn't have target image");
		return;
	}

	for(;;)
	{
		src_name = pktGetString(pak_in);
		if (!src_name || !src_name[0])
			break;
		src_image = findImageByName(project,src_name);
		if (stricmp(best_src_name,"0")==0 && src_image)
			strcpy(best_src_name,src_name);
		sprintf(patch_name,"%s/patches/%s/%s/%s.patch",project->rootpath,project->name,target_name,getFileName(src_name));
		check = filemgrGetChecksum(patch_name);
		if (!check)
		{
			if (!src_image || strcmp(src_name,"0")==0)
				continue;

			//size = src_image->bytecount - dst_image->bytecount;
			//if (size < 0)
			//	size = -size;

			size = 0;
			for (i = 0; i < src_image->file_count; i++)
			{
				src_file = src_image->files[i];
				if (stashFindPointer(dst_image->filenames, src_file->name, &dst_file))
				{
					if (!checksumMatch(&dst_file->full, &src_file->full))
					{
						size += src_file->full.size;
					}
				}
				else
				{
					size += src_file->full.size;
				}
			}

			size += 5000000; // make extra sure we don't make new patches for no good reason
		}
		else
			size = check->size;
		if (size < diff_size)
		{
			diff_size = size;
			strcpy(best_src_name,src_name);
		}
	}
	log_printf("connections","%s ReqSmallestDiff found: %s",ipStr(link), best_src_name);
	{
		Packet	*pak = pktCreateEx(link,PATCHSERVER_SMALLEST_DIFF);

		pktSendString(pak,best_src_name);
		pktSend(&pak,link);
	}	
}

static void handleBadFiles(Packet *pak,NetLink *link)
{
	char	*str;
	U32		cookie;

	cookie = pktGetBits(pak,32);
	str = pktGetZipped(pak,0);
	log_printf("badfiles","IP: %s  Cookie: %u %s",makeIpStr(link->addr.sin_addr.S_un.S_addr),cookie,str);
}

static void handleClientFatalError(Packet *pak,NetLink *link)
{
	char	*msg, *callstack;
	int		def_network_ver = -1;
	Checksum check;

	msg = pktGetZipped(pak,0);
	callstack = pktGetZipped(pak,0);
	def_network_ver = pktGetBits(pak, 8);
	pktGetBitsArray(pak,sizeof(check)*8,&check);
	log_printf("FatalErrors", "\n{\n  IP: %s\n  MSG: %s\n  NET VER: %d\n  CHECKSUM: %I64d {%08x %08x %08x %08x}\n  STACK:\n%s\n}\n", makeIpStr(link->addr.sin_addr.S_un.S_addr),
		msg, def_network_ver, check.size, check.values[0], check.values[1], check.values[2], check.values[3],callstack);
}

int patchHandleClientMsg(Packet *pak,int cmd, NetLink *link)
{
	PatchClientLink	*client = link->userData;

	if (!client->resized)
	{
		extern int g_recv_size,g_send_size;

		client->resized = 1;
		if (g_send_size)
			socketSetBufferSize(link->socket, SO_SNDBUF, g_send_size);
		if (g_recv_size)
			socketSetBufferSize(link->socket, SO_RCVBUF, g_recv_size);
	}

	switch(cmd)
	{
		xcase PATCHCLIENT_MANIFEST:
			handleReqPatch(pak,link);
		xcase PATCHCLIENT_REQ_DATA:
			handleReqData(pak,link);
		xcase PATCHCLIENT_REQ_PROJECT_LIST:
			handleReqProjectList(pak,link);
		xcase PATCHCLIENT_REQ_NEW_CLIENT:
			handleReqNewClient(pak,link,0);
		xcase PATCHCLIENT_REQ_NEW_CLIENT_MULTIFILE:
			handleReqNewClient(pak,link,1);
		xcase PATCHCLIENT_REQ_FILES:
			handleReqFiles(pak,link);
		xcase PATCHCLIENT_REQ_FILE_PART:
			handleReqFilePart(pak,link);
		xcase PATCHCLIENT_CONNECT:
			handleClientConnect(link);
		xcase PATCHCLIENT_REQ_SMALLEST_DIFF:
			handleReqSmallestDiff(pak,link);
		xcase PATCHCLIENT_BADFILES:
			handleBadFiles(pak,link);
		xcase PATCHCLIENT_REQ_IMAGE_CHECKSUM:
			handleReqImageChecksum(pak,link);
		xcase PATCHCLIENT_FATAL_ERROR:
			handleClientFatalError(pak,link);
		xcase PATCHCLIENT_REQ_CIDER_UPDATE:
			handleReqCiderUpdate(pak,link);
		xcase PATCHCLIENT_REQ_UPDATER_FILE_PART:
			handleReqUpdaterFilePart(pak,link);
		break;
		default:
			printf("Unknown command %d\n",cmd);
			return 0;
	}
	return 1;
}

void patchNetInit()
{
	int i;

	sockStart();
	packetStartup(0,0); // JE: Not initializing encryption here, it'll start faster, and we don't care about/use encryption on PatchClient links anyway
	
	for ( i = 0; i < NUM_VALID_UPDATESERVER_PORTS; ++i )
	{
		NetLinkList *cur_link = &(net_links[i]);
		netLinkListAlloc(cur_link,INITIAL_CLIENT_COUNT,sizeof(PatchClientLink),patchClientCreateCb);
		//netInit(cur_link,0,DEFAULT_UPDATESERVER_PORT);
		netInit(cur_link,0,valid_updateserver_ports[i]);
		cur_link->publicAccess = 1;
		cur_link->destroyCallback = patchClientDeleteCb;
		NMAddLinkList(cur_link, patchHandleClientMsg);
		netLinkListSetMaxBufferSize(cur_link, BothBuffers, 64*1024); // Max send buffer size of 64K/link
	}
}

static void printXferStats()
{
	static __int64	last_total_sent,bytes_per_sec;
	static int		xferStatsTimer;
	__int64			curr_sent;
	char			buf[1000],buf1[100],buf2[100];

	if (!xferStatsTimer)
		xferStatsTimer = timerAlloc();
	if (timerElapsed(xferStatsTimer) > 2.0)
	{
		curr_sent = total_bytes_sent - last_total_sent;
		last_total_sent = total_bytes_sent;
		bytes_per_sec = curr_sent / timerElapsed(xferStatsTimer);
		timerStart(xferStatsTimer);
		strcpy(buf,"UpdateServer");
		if (project_list.server_version)
			strcatf(buf," v%s",project_list.server_version);
		if (project_list.client_version)
			strcatf(buf," serving v%s", project_list.client_version);
		strcatf(buf,"   %d Users   %s sent   %s/sec",numClients(), printUnit(buf1,total_bytes_sent), printUnit(buf2,bytes_per_sec));
		updateListenerStatus(project_list.server_version, project_list.client_version, numClients(), total_bytes_sent, bytes_per_sec);
		setConsoleTitle(buf);
	}
}


void checkThrottledLink( NetLink * link )
{
	PatchClientLink	*client;
	int packet_rate = 0;
	int elapsed_seconds;
	client = link->userData;

	if ( !project_list.throttle_speed )
		return;

	elapsed_seconds = (timer - client->timestamp);
	if ( elapsed_seconds )
		packet_rate = link->totalBytesSent / elapsed_seconds;

	// if they have exceeded their bandwidth limit, throttle them
	if ( packet_rate > project_list.throttle_speed )
	{
		if ( !client->throttled )
			client->throttled = 1;
		return;
	}
	else
	{
		// if they are throttled or have packets on their queue, send the packets until they reach
		// their bandwidth limit again
		if ( client->throttled || !qIsEmpty(client->throttledQueue) )
		{
			client->throttled = 0;
			while ( !qIsEmpty(client->throttledQueue) && packet_rate < project_list.throttle_speed )
			{
				Packet * pak;
				pak = (Packet*)qDequeue(client->throttledQueue);
				total_bytes_sent += pak->stream.size;
				pktSend(&pak, link);
			}
		}
	}
}

void servePatches(void)
{
	int		i, j;

	patchNetInit();
	patchServerListenThreadBegin();
	printf("Patchserver ready...\n");
	for(;;)
	{
		timer = timerCpuSeconds();

		printXferStats();
		NMMonitor(100);
		for(i=0;i<NUM_VALID_UPDATESERVER_PORTS;i++)
		{
			NetLinkList *cur_link = &(net_links[i]);
			for(j=0;j<cur_link->links->size;j++)
			{
				NetLink	*link;
				int		dt;

				link = cur_link->links->storage[j];
				dt = timerCpuSeconds() - link->lastRecvTime;
				if (dt > 60*60)
					netRemoveLink(link);

				checkThrottledLink(link);
			}
		}
		if (_kbhit()) { // debug hack for printing out memory monitor stuff
			switch (_getch()) {
				xcase 'm':
					memMonitorDisplayStats();
				xcase 'd':
					printf("dumping memory table to c:/memlog.txt..\n");
					memCheckDumpAllocs();
					printf("done.\n");
			}
		}
	}
}

