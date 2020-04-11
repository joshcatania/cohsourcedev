#include <assert.h>
#include <sys/stat.h>
#include "netxfer.h"
#include "comm_patcher.h"
#include "timing.h"
#include "mathutil.h"
#include "patchui.h"
#include "crypt.h"
#include "patchfileutils.h"
#include "net_socket.h"
#include "xlate.h"
#include "RegistryReader.h"
#include "filechecksum.h"
#include "net_masterlist.h"
#include "patchutils.h"

extern NetLink comm_link;
extern int	glob_perf_test;
extern char reg_key_dir[];


int lostServerLink(void)
{
	if ( comm_link.disconnected )
		printf( "Lost connection to server...\n" );
	return comm_link.disconnected;
}

#define MAX_PAK_STACK 1024

static Packet *pak_stack[MAX_PAK_STACK];
static int cmd_stack[MAX_PAK_STACK];
static int pak_stack_head = 0;
static int pak_stack_tail = 0;

static int handleNetMsg(Packet* pak, int cmd, NetLink* link)
{
	pak->link = link;

	assert(((pak_stack_tail - pak_stack_head) % MAX_PAK_STACK) < (MAX_PAK_STACK-1));

	pak_stack[pak_stack_tail] = pak;
	cmd_stack[pak_stack_tail] = cmd;
	pak_stack_tail++;

	return -2;
}

int waitForCmd(Packet **pak_p)
{
	int cmd;
	*pak_p = 0;

	lnkBatchSend(&comm_link);

	while (pak_stack_tail == pak_stack_head)
	{
		if (lostServerLink()) {
			pak_stack_head = pak_stack_tail;
			return PATCHSERVER_DISCONNECTED;
		}
		netLinkMonitorBlock(&comm_link, 0, handleNetMsg, 1);
		netIdle(&comm_link, 0, 10);
	}

	*pak_p = pak_stack[pak_stack_head];
	cmd = cmd_stack[pak_stack_head];
	pak_stack_head++;

	return cmd;
}

static int checksumPatchFile(char *fname)
{
	#define CHUNK_SIZE (1 << 20)
	FILE		*file;
	U8			*chunk_mem = 0;
	U32			i,chunk_size,ret=0;
	Checksum	check,file_check;

	file = fopen(fname,"rb");
	if (!file)
		goto fail;
	if (fread(&check,1,sizeof(check),file) != sizeof(check))
		goto fail;
	xferStatsInit("StatsVerifyingPatch",NULL,0,check.size);
	chunk_mem = malloc(CHUNK_SIZE);
	for(i=0;i<check.size;i+=CHUNK_SIZE)
	{
		chunk_size = MIN(CHUNK_SIZE,check.size - i);
		if (fread(chunk_mem,1,chunk_size,file) != (int)chunk_size)
			goto fail;
		cryptMD5Update(chunk_mem,chunk_size);
		xferStatsUpdate(chunk_size);
	}
	file_check.size = check.size;
	cryptMD5Final(file_check.values);
	if (!checksumMatch(&check,&file_check))
		goto fail;
	ret = 1;
fail:
	xferStatsFinish();
	if (!ret)
		cryptMD5Final(file_check.values);
	if (file)
		fclose(file);
	free(chunk_mem);
	return ret;
}

static void reqBlock(Checksum *check,U32 pos,U32 size,U32 dst_size)
{
	Packet	*pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_DATA);

	size = MIN(XFER_BLOCK_SIZE,dst_size-pos);
	pktSendBitsPack(pak,1,pos);
	pktSendBitsPack(pak,1,size);
	pktSendBitsArray(pak,8*sizeof(*check),check);
	pktSend(&pak,&comm_link);
}

static int getBlock(FILE *file)
{
	Packet		*pak;
	U32			cmd,pos,size;
	static U8	buf[XFER_BLOCK_SIZE];

	cmd = waitForCmd(&pak);
	if (cmd != PATCHSERVER_DATA)
	{
		if (cmd == PATCHSERVER_ERROR_MSG)
			msgAlertUpdater("ErrFromServer",xlateQuick(pktGetString(pak)));
		else
		{
			if (!lostServerLink())
				netSendDisconnect(&comm_link,10.f);
		}
		pktFree(pak);
		return 0;
	}
	pos = pktGetBitsPack(pak,1);
	size = pktGetBitsPack(pak,1);
	pktGetBitsArray(pak,size*8,buf);
	if (glob_perf_test < 2)
		safeWriteBytes(file,pos,buf,size);
	pktFree(pak);
	if (glob_perf_test < 2)
		xferStatsUpdate(size);
	return 1;
}

int netGetPatchFile(char *client_patch_name)
{
	FILE		*file;
	Checksum	check;
	U32			curr_size,dst_size,i,bytesread,bytespersec;

	chmod(client_patch_name,_S_IREAD | _S_IWRITE);
	file = fopen(client_patch_name,"r+b");
	if (!file)
		goto fail;
	if ((bytesread = fread(&check,1,sizeof(check),file)) != sizeof(check))
		goto fail;
	dst_size = check.size + sizeof(check);
	curr_size = MIN(safeFileSize(client_patch_name),dst_size-1) & ~(XFER_BLOCK_SIZE-1);
perf_test:
	xferStatsInit("StatsDownloading",NULL,curr_size,dst_size);
	{
		U32 max_in_transit = (1 << 18);
		U32 initial_blocks = MIN(dst_size - curr_size,max_in_transit) / XFER_BLOCK_SIZE;

		for(i=0;i<initial_blocks;i++)
			reqBlock(&check,curr_size+i*XFER_BLOCK_SIZE,XFER_BLOCK_SIZE,dst_size);
		for(i=curr_size+XFER_BLOCK_SIZE*initial_blocks;i<dst_size;i+=XFER_BLOCK_SIZE)
		{
			reqBlock(&check,i,XFER_BLOCK_SIZE,dst_size);
			if (!getBlock(file))
				goto fail;
		}
		for(i=0;i<initial_blocks;i++)
		{
			if (!getBlock(file))
				goto fail;
		}
	}
	bytespersec = (dst_size-curr_size) / (xferStatsElapsed());

	if ((dst_size-curr_size) > 1024 * 1024 && reg_key_dir && reg_key_dir[0])
	{
		char buf[1024];
		RegReader reader = createRegReader();
		initRegReader(reader, reg_key_dir);
		sprintf(buf, "%d", bytespersec);
		rrWriteString(reader, "TransferRate", buf);
		destroyRegReader(reader);
	}

	if (glob_perf_test)
	{
		curr_size = 0;
		goto perf_test;
	}
	fclose(file);
	file = 0;

	xferStatsFinish();
	if (!checksumPatchFile(client_patch_name))
		goto fail;
	return 1;
fail:
	xferStatsFinish();
	if (file)
		fclose(file);
	return 0;
}

int netGetSmallFiles(NewPigEntry **entries,int count)
{
	Packet		*pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_FILES);
	int			i,cmd;
	char		*name;
	U32			size;
	NewPigEntry	*entry;

	for(i=0;i<count;i++)
		pktSendString(pak,entries[i]->fname);
	pktSendString(pak,0);
	pktSend(&pak,&comm_link);
	cmd = waitForCmd(&pak);
	if (cmd == PATCHSERVER_DISCONNECTED)
	{
		pktFree(pak);
		return 0;
	}
	if (cmd != PATCHSERVER_SEND_FILES)
	{
		if ( cmd == PATCHSERVER_ERROR_MSG )
		{
			msgAlertUpdater("ErrFromServer",xlateQuick(pktGetString(pak)));
		}
		else
			msgAlertFatal("ErrUnknownMessage",cmd);
	}

	for(i=0;i<count;i++)
	{
		entry = entries[i];
		if (entry->data)
			continue;
		name = pktGetString(pak);
		if (!name || !name[0])
			break;
		assert(stricmp(entry->fname,name)==0);
		entry->pack_size	= pktGetBitsPack(pak,1);
		entry->size			= pktGetBitsPack(pak,1);
		size = entry->pack_size ? entry->pack_size : entry->size;
		entry->data			= malloc(size);
		pktGetBitsArray(pak,size*8,entry->data);
		xferStatsUpdate(entry->pack_size);
	}
	pktFree(pak);
	return 1;
}

#define MAX_FILE_XFER (1<<18)
#define SERVER_QUEUE 32

typedef struct
{
	char * fname;
	char * data;
	U32 size;
	U32 next;
	U32 inflight;
} big_t;


static void requestBigFilePart(const char * fname, U32 req_offset, U32 req_size)
{
	Packet *pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_FILE_PART);
	pktSendString(pak,fname);
	pktSendBitsPack(pak,20,req_offset);
	pktSendBitsPack(pak,20,req_size);
	pktSend(&pak,&comm_link);
}

static void requestNextBigFilePart(big_t *big)
{
	U32 req_size = MIN(MAX_FILE_XFER, big->size - big->next);
	if(!req_size) {		
		return;
	}

	requestBigFilePart(big->fname, big->next, req_size);
	big->next += req_size;
	big->inflight++;
}

static int handleBigFileParts(Packet *pak, int cmd, NetLink *link)
{
	U32 pos, amt;
	big_t * big = (big_t*)link->userData;

	if (cmd != PATCHSERVER_SEND_FILE_PART)
	{
		if ( cmd == PATCHSERVER_ERROR_MSG )
		{
			msgAlertUpdater("ErrFromServer",xlateQuick(pktGetString(pak)));
		}
		else
			msgAlertFatal("ErrUnknownMessage",cmd);
	}

	pos = pktGetBitsPack(pak,20);
	amt = pktGetBitsPack(pak,20);
	pktGetBitsArray(pak, amt*8, big->data + pos);
	xferStatsUpdate(amt);
	big->inflight--;

	requestNextBigFilePart(big);
	return 1;
}

int netGetBigFile(NewPigEntry *entry)
{
	U32 i, size;
	big_t big;

	size = entry->pack_size ? entry->pack_size : entry->size;
	entry->data = calloc(size,1);

	big.fname = entry->fname;
	big.data = entry->data;
	big.size = size;
	big.next = 0;
	big.inflight = 0;

	comm_link.userData = &big;

	for(i = 0; i < SERVER_QUEUE; i++) {
		requestNextBigFilePart(&big);
	}

#if 1
	lnkBatchSend(&comm_link);
	while(big.inflight)
	{
		if (lostServerLink())
			return 0;
		netLinkMonitorBlock(&comm_link, 0, handleBigFileParts, 1);
		netIdle(&comm_link, 0, 10);
	}
#else
	// get all of the requested file chunks
	while(big.inflight)
	{
		Packet * pak = NULL;
		U32 cmd = waitForCmd(&pak);
		if (cmd == PATCHSERVER_DISCONNECTED)
		{
			pktFree(pak);
			return 0;
		}
		handleBigFileParts(pak, cmd, pak->link);
		pktFree(pak);
	}
#endif

	return 1;
}

int netGetFiles(NewPigEntry **entries,int count)
{
	int				i,last_i;
	NewPigEntry		*entry;
	U32				size,curr_size=0;

	for(i=0;i<count;i++)
	{
		if (!entries[i]->pack_size)
			entries[i]->dont_pack = 1;
	}

	for(last_i=i=0;i<count;i++)
	{
		entry = entries[i];
		size = entry->pack_size ? entry->pack_size : entry->size;
		if (size + curr_size > MAX_FILE_XFER || i == count-1)
		{
			if (curr_size)
			{
				if (!netGetSmallFiles(entries + last_i,i-last_i))
					return 0;
				last_i = i;
			}
			if (size > MAX_FILE_XFER || i==count-1)
			{
				if (!netGetBigFile(entries[i]))
					return 0;
				last_i = i+1;
				size=0;
			}
			curr_size = 0;
		}
		curr_size += size;
	}
	return 1;
}

void* netGetUpdaterFile(const char* szFileName, U32 nSize, int bStatUpdate)
{
	Packet	*pak;
	U32		i,cmd,pos,amt,req_size;
	U32		j, last_j;
	U8*     pResult = NULL;

	pResult = (U8*)malloc(nSize);

	if ( szFileName )
	{
		if ( pResult )
		{
			if ( bStatUpdate )
				xferStatsInit((char*)szFileName,NULL,0,nSize);

			for( i=0; i < nSize; i += MAX_FILE_XFER*SERVER_QUEUE )
			{
				last_j = SERVER_QUEUE;

				// send SERVER_QUEUE packets requesting chunks of size MAX_FILE_XFER
				for ( j = 0; j < SERVER_QUEUE; ++j )
				{
					if ( i + (j * MAX_FILE_XFER) > nSize )
					{
						last_j = j;
						break;
					}

					pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_UPDATER_FILE_PART);
					pktSendString(pak, szFileName);
					pktSendBitsPack(pak,20,i+(j * MAX_FILE_XFER));
					req_size = MIN(MAX_FILE_XFER, nSize - (i+(j * MAX_FILE_XFER)));

					pktSendBitsPack(pak,20,req_size);
					pktSend(&pak,&comm_link);
				}

				// get all of the requested file chunks
				for ( j = 0; j < last_j; ++j )
				{
					cmd = waitForCmd(&pak);
					if (cmd == PATCHSERVER_DISCONNECTED)
					{
						pktFree(pak);
						return 0;
					}
					if (cmd != PATCHSERVER_SEND_UPDATER_FILE_PART)
					{
						if ( cmd == PATCHSERVER_ERROR_MSG )
						{
							msgAlertUpdater("ErrFromServer",xlateQuick(pktGetString(pak)));
						}
						else
							msgAlertFatal("ErrUnknownMessage",cmd);
					}

					pos = pktGetBitsPack(pak,20);
					amt = pktGetBitsPack(pak,20);
					pktGetBitsArray(pak, amt*8, pResult + pos );

					if ( bStatUpdate )
						xferStatsUpdate(amt);

					pktFree(pak);
				}
			}

			if ( bStatUpdate )
				xferStatsFinish();
		}
	}

	return pResult;
}