#include "utils.h"
#include "wininclude.h"
#include <assert.h>
#include "piglib.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "utils.h"
#include "genericlist.h"
#include "assert.h"
#include "timing.h"
#include <direct.h>
#include <sys/types.h>
#include <sys/utime.h>
#include "SharedMemory.h"
#include "fileUtil.h"
#include "bindiff.h"
#include "zlib.h"
#include "crypt.h"
#include "mathutil.h"
#include "filechecksum.h"
#include "patchfileutils.h"
#include "patchcreate.h"
#include "projects.h"
#include "patchserver.h"
#include "filemgr.h"
#include "gimmeDLLWrapper.h"
#include "MemoryMonitor.h"

int glob_perf_test;
int g_recv_size,g_send_size;
int g_server_patch = 1;
char reg_key_dir[1] = {0};
int no_tar = 0;

// ugly, hackish way to force the update server to a new version
int do_not_change_this_variable_from_zero = 0;

int main(int argc,char **argv)
{
	int		i,run_server=1,patch_to_latest=0;
	extern CRITICAL_SECTION PigCritSec;

	EXCEPTION_HANDLER_BEGIN
	setAssertMode(ASSERTMODE_DEBUGBUTTONS | ASSERTMODE_FULLDUMP | ASSERTMODE_DATEDMINIDUMPS | ASSERTMODE_ZIPPED);
	printf("Initializing...\n");
	fileDisableAutoDataDir();
	memMonitorInit();
	gimmeDLLDisable(1);
	InitializeCriticalSection(&PigCritSec);
	sharedMemorySetMode(SMM_DISABLED);
	cryptInit();
	logSetDir("updateserver");
	log_printf("connections","updateserver starting");
	filemgrInit();
	//disableLogging(1);

	// ugly, hackish way to force the update server to a new version
	// (see comments above "FunctionWithNoPurposeOtherThanToForceALink()")
	//
	// this code does not do anything and should never get called
	if ( do_not_change_this_variable_from_zero )
	{
		FunctionWithNoPurposeOtherThanToForceALink();
	}

	for(i=1;i<argc;i++)
	{
		if (stricmp(argv[i],"-noserver")==0)
			run_server = 0;
		else if (stricmp(argv[i],"-notar")==0)
			no_tar = 1;
		else if (stricmp(argv[i],"-nogui")==0) {
			setGuiDisable(true);
			setAssertMode(ASSERTMODE_STDERR | ASSERTMODE_EXIT);
		} else if (stricmp(argv[i],"-perftest2")==0)
			glob_perf_test = 2;
		else if (stricmp(argv[i],"-patchtolatest")==0)
			patch_to_latest = 1;
		else if (stricmp(argv[i],"-recv")==0)
			g_recv_size = atoi(argv[i+1]);
		else if (stricmp(argv[i],"-send")==0)
			g_send_size = atoi(argv[i+1]);
		else if (stricmp(argv[i],"-sendqueueassert")==0) {
			extern int g_assert_on_netlink_overflow;
			g_assert_on_netlink_overflow = 1;
		}
	}

	projectLoad("config.txt",run_server,patch_to_latest);
	log_printf("connections","updateserver running");
	if (run_server)
		servePatches();
	EXCEPTION_HANDLER_END 
	exit(0);
}

