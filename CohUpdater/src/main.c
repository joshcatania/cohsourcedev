#include "netio.h"
#include "error.h"
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
#include <windows.h>
#include <sys/types.h>
#include <sys/utime.h>
#include "SharedMemory.h"
#include "StashTable.h"
#include "fileUtil.h"
#include "utils.h"
#include "bindiff.h"
#include "zlib.h"
#include "crypt.h"
#include "mathutil.h"
#include "filechecksum.h"
#include "patchfileutils.h"
#include "patchapply.h"
#include "comm_patcher.h"
#include "sock.h"
#include "netcomp.h"
#include "patchclient.h"
#include "RegistryReader.h"
#include "patchui.h"
#include "projectclient.h"
#include "embedbrowser.h"
#include "sysutil.h"
//#include "gimmeDLLWrapper.h"
#include "projectfile.h"
#include "xlate.h"
#include "EString.h"
#include "AppLocale.h"
#include "AppRegCache.h"
#include "osdependent.h"
#include "filewatch.h"
#include "net_version.h"
#include "Stackdump.h"
#include "netxfer.h"

// Tar unpack functionality
#include "tar.h"

// UI functions from a DLL
#include "dylibimport.h"

#define LOGID "CohUpdater"
#define TOSTR(X) REALLY_TOSTR(X)

extern int g_force_production_mode;

static	char passthrough_args[1024];

char			reg_key_dir[MAX_PATH],*g_override_url,*g_cmdline;
int				g_full_checksum,g_got_patch,g_server_patch,g_lag,g_send_size,g_recv_size,g_get_major_patch,g_safe_server = 0;
static int		no_self_update=0;
volatile int	g_user_quit;
char			g_override_install_dir[MAX_PATH] = {0};
char			LangWebPageKey[256],*WebPageKey = "WebPage";
char			LangEulaWebPageKey[256],*EulaWebPageKey = "EulaWebPage";
int				g_show_language_button = 0;
int				isFreshInstall = 1;
int				isClientOnly = 1;
char			g_updater_name[256] = "CohUpdater";
char			g_game_name[256] = "City of Heroes";
char			g_primary_project_display_name[256] = "Coh";
int				g_safe_mode = 0;
int				g_COV = 0;
int				g_isMac = 0;
int             g_bUseUI = 1;
int             g_isEU = 0;
Checksum		g_updater_checksum;

int getLatestCiderUpdate(NetLink* comm_link, int bUseUI);
void recursiveCopy(const char* szSourcePath,const char* szDestinationPath);
BOOL getCiderID(char* szIDBuf);

#define MAX_PATH_MAC 1024
#define PKGINFO_SIZE 16

static void patchClientProject(char *reg_dir,char *locale_string,char *project_name,char *dir_name,int default_patch,ImageCheck *image)
{
	#define REG_STR_BUF_SIZE 256
	RegReader	reader;
	int			ret;
	char		buffer[REG_STR_BUF_SIZE],*install_dir=0;
	static char	default_patch_dir[MAX_PATH];

	patchUiSetProjectInfo(project_name,0);
	sprintf(reg_key_dir,"%s\\%s%s",reg_dir,locale_string,project_name);
	reader = createRegReader();
	initRegReader(reader, reg_key_dir);

	if (g_override_install_dir[0])
	{
		filelog_printf( LOGID, __FUNCTION__ ": override install dir:%s", g_override_install_dir);
		install_dir = strdup(g_override_install_dir);
		makeDirectories(install_dir);
	}
	if (!install_dir && rrReadString(reader, "Installation Directory", buffer, REG_STR_BUF_SIZE) )
	{
		install_dir = strdup(buffer);
		filelog_printf( LOGID, __FUNCTION__ ": install dir from registry %s", install_dir);
	}
	
	if (!install_dir && default_patch_dir[0])
	{
		install_dir = strdup(default_patch_dir);
		filelog_printf( LOGID, __FUNCTION__ ": install dir from default_patch_dir %s", install_dir);
	}
	
	if (!install_dir || !dirExists(install_dir))
	{
retry:
		install_dir = pickInstallDir(project_name,dir_name);
		if (!install_dir)
		{
			filelog_printf( LOGID, __FUNCTION__ ": couldn't find install dir");
			exit(0);
		}
		
		rrWriteString(reader,"Installation Directory",install_dir);
		filelog_printf( LOGID, __FUNCTION__ ": install dir from pickInstallDir %s", install_dir);
	}

	if (default_patch)
	{
		strcpy(default_patch_dir,install_dir);
		filelog_printf( LOGID, __FUNCTION__ ": default patch dir set to %s", default_patch_dir);
	}
	
	destroyRegReader(reader);
	for(;;)
	{
		printf( "Patching project: %s   Install Dir:  %s\n", project_name, install_dir );
		filelog_printf( LOGID, __FUNCTION__ ": Patching project: %s   Install Dir:  %s\n", project_name, install_dir );

		ret = clientPatch(project_name,install_dir,install_dir,0,default_patch);
		if (ret > 0)
		{
			filelog_printf( LOGID, __FUNCTION__ ": Patched successfully");
			printf( "Patched successfully\n" );
			break;
		}
		if (ret == -1)
		{
			filelog_printf( LOGID, __FUNCTION__ ": returned -1. retrying");
			fileMsgAlert("ErrCantCreateDir",install_dir);
			reader = createRegReader();
			initRegReader(reader, reg_key_dir);
			goto retry;
		}
		if (ret == -3)
		{
			filelog_printf( LOGID, __FUNCTION__ ": Client Patch returned error %d, doing full checksum", ret);

			printf( "Client Patch returned error %d, doing full checksum\n", ret );
			g_full_checksum = 1;
		}
		if (lostServerLink())
			connectToServer();

		filelog_printf( LOGID, __FUNCTION__ ": clientPatch returned %i. Retrying loop", ret);
	}

	{
		char	checksum_name[MAX_PATH];

		filelog_printf( LOGID, __FUNCTION__ ": End of Patching: remaking checksum");

		sprintf(checksum_name,"%s/%s.checksum",install_dir,project_name);
		checksumLoad(checksum_name,image);
		image->fname = strdup(install_dir);
	}
}



static HANDLE launchProgram(char *default_regdir,char *project_name,char *exe_name)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char gameExePath[MAX_PATH] = {0},reg_dir[1000];
	RegReader	reader = createRegReader();
		
	sprintf(reg_dir,"%s\\%s",default_regdir,project_name);
	initRegReader(reader, reg_dir);
	rrReadString(reader, "Installation Directory", gameExePath, sizeof(gameExePath));
	destroyRegReader(reader);

	if (g_override_install_dir[0])
		strcpy(gameExePath, g_override_install_dir);

	if (!gameExePath[0])
		return NULL;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	chdir(gameExePath);
	strcat(gameExePath,"\\");
	strcat(gameExePath,exe_name);
	strcat(gameExePath," -project ");
	strcatf(gameExePath,"\"%s\"",project_name);
	if (g_get_major_patch)
	{
		char	buf[1000];

		sprintf(buf,"%s %s -mp",getExecutableName(),g_cmdline);
		strcatf(gameExePath," -exitlaunch \"%s\"",escapeString(buf));
	}
	strcat(gameExePath,passthrough_args);
	CreateProcess(NULL, gameExePath, NULL, NULL, 0, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi); // XXX
	return pi.hProcess;
}

static void execAndQuit(char *exe_name)
{
	STARTUPINFO			si = {0};
	PROCESS_INFORMATION	pi = {0};
	int					result;
	si.cb				= sizeof(si);
	
	result = CreateProcess(exe_name, GetCommandLine(),
		NULL,			NULL,			FALSE,			CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
		NULL,			NULL,			&si,			&pi);
	assert(result);
	exit(0);
}

#define NUM_UPDATER_FILES 2

void patchClientGetLatestExe()
{
	Packet		*pak;
	int         cmd;
	U32			size;
	U32         nStrLength;
	U8			*mem,new_name[MAX_PATH],tmp_name[MAX_PATH],exe_name[MAX_PATH],dll_name[MAX_PATH],i;
	char*       ppUpdaterFiles[NUM_UPDATER_FILES];
	int         bOldServer = 0;
//	char outBuf[1000];

	strcpy(exe_name,getExecutableName());
	backSlashes(exe_name);
	strcpy(new_name,exe_name);
	strcpy(tmp_name,exe_name);
	strcpy(new_name + strlen(new_name)-3,"new");
	strcpy(tmp_name + strlen(tmp_name)-3,"tmp");
	if (strEndsWith(exe_name,".tmp"))
	{
		strcpy(exe_name + strlen(exe_name)-3,"exe");
		safeRenameFile(new_name,exe_name);
		execAndQuit(exe_name);
	}
	chmod(new_name,_S_IREAD|_S_IWRITE);
	unlink(new_name);
	chmod(tmp_name,_S_IREAD|_S_IWRITE);
	unlink(tmp_name);

	tmp_name[0] = '\0';

	// check if we're connecting to the old ( no separate UI ) or new server
	pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_NEW_CLIENT);
	pktSendBitsPack(pak,1,PATCHER_PROTOCOL_OLD_VERSION); // old version ( 2004xxxx )
	pktSendBitsPack(pak,1,1); // no self-update

	checksumFile(exe_name,&g_updater_checksum,g_bUseUI);
	pktSendBitsArray(pak,sizeof(g_updater_checksum)*8,&g_updater_checksum);
	pktSend(&pak, &comm_link);

	cmd = waitForCmd(&pak);

	if ( cmd == PATCHSERVER_CLIENT_OK )
	{
		// this is an old server (let's hope)
		if ( g_isMac )
		{
			// Only PCs should be able to connect to old servers
			msgAlertFatal("Your installation of City of Heroes cannot update using this server at this time. Please try again later."); // TODO : Get translations for real message
		}
		else
		{
			bOldServer = 1;
		}
	}

	pktFree(pak);

	do
	{
		if ( g_bUseUI )
		{
			xferStatsInit("StatsCheckingUpdater",0,0);
		}		

		// update cider
		if ( !bOldServer )
		{
			if ( g_isMac )
			{
				if ( getLatestCiderUpdate(&comm_link, g_bUseUI) )
				{
					// In case where there is a cider update, but no executable / ui library updates
					strncpy(tmp_name,getExecutableName(),MAX_PATH);
				}
			}

			pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_NEW_CLIENT_MULTIFILE);
			pktSendBitsPack(pak,1,PATCHER_PROTOCOL_VERSION);
			pktSendBitsPack(pak,1,no_self_update);

			// Checksum 2 files together
			// Exe
			nStrLength = strlen(exe_name) + 1;
			ppUpdaterFiles[0] = (char*)malloc(nStrLength*sizeof(char));
			strncpy(ppUpdaterFiles[0],exe_name,nStrLength);

			// UI library		
			snprintf(dll_name,MAX_PATH,"%s\\%s",getExecutableDir(exe_name),getUIDLLName(TRUE));
			nStrLength = strlen(dll_name) + 1;
			ppUpdaterFiles[1] = (char*)malloc(nStrLength*sizeof(char));
			strncpy(ppUpdaterFiles[1],dll_name,nStrLength);
		
			checksumMultipleFiles(ppUpdaterFiles,NUM_UPDATER_FILES,&g_updater_checksum,g_bUseUI);

			for ( i = 0; i < NUM_UPDATER_FILES; i++ )
			{
				free(ppUpdaterFiles[i]);
			}

			filelog_printf( LOGID, "checksum (multifile): %I64d {%08x %08x %08x %08x}\n", g_updater_checksum.size, g_updater_checksum.values[0], 
				g_updater_checksum.values[1], g_updater_checksum.values[2], g_updater_checksum.values[3]);		

			pktSendBitsArray(pak,sizeof(g_updater_checksum)*8,&g_updater_checksum);

			pktSend(&pak,&comm_link);

			cmd = waitForCmd(&pak);
		}
		else
		{
			// Try reverting to the old version of the updater
			pak = pktCreateEx(&comm_link,PATCHCLIENT_REQ_NEW_CLIENT);
			pktSendBitsPack(pak,1,PATCHER_PROTOCOL_OLD_VERSION);
			pktSendBitsPack(pak,1,no_self_update);

			checksumFile(exe_name,&g_updater_checksum,g_bUseUI);

			filelog_printf( LOGID, "Reverting to old version - checksum (single exe): %I64d {%08x %08x %08x %08x}\n", g_updater_checksum.size, g_updater_checksum.values[0], 
			g_updater_checksum.values[1], g_updater_checksum.values[2], g_updater_checksum.values[3]);

			pktSendBitsArray(pak,sizeof(g_updater_checksum)*8,&g_updater_checksum);

			pktSend(&pak, &comm_link);

			cmd = waitForCmd(&pak);
		}

		if ( g_bUseUI )
			xferStatsFinish();

		if (cmd == PATCHSERVER_CLIENT_OK)
		{
			char	*s;

			if (!pktEnd(pak))
			{
				s = pktGetString(pak);
				if (s && s[0])
					project_list.client_version = strdup(s);
			}
		}
		else if (cmd == PATCHSERVER_ERROR_MSG)
		{
			msgAlertFatal("ErrFromServer",xlateQuick(pktGetString(pak)));
		}
		else if (cmd == PATCHSERVER_NEW_CLIENT)
		{
			// this is a downgrade to the old updater. goodbye, separate UI.
			strcpy(tmp_name,exe_name);
			strcpy(tmp_name + strlen(tmp_name)-3,"tmp");

			mem = pktGetZipped(pak,&size);
			if (!size)
				msgAlertFatal("ErrZeroSizeClient");
			safeWriteFile(new_name,"!wb",mem,size);
			safeWriteFile(tmp_name,"!wb",mem,size);
			execAndQuit(tmp_name);
		}
		else if (cmd == PATCHSERVER_NEW_CLIENT_MULTIFILE)
		{
			Tar *t;
			U8 *curFile;
			U32 curFileSize;
			U8 new_file_name[MAX_PATH];

			mem = pktGetZipped(pak,&size);
			if (!size)
				msgAlertFatal("ErrZeroSizeClient");

			tmp_name[0] = '\0';

			// this should be a tar archive
			t = tar_from_data(mem,size);
			curFile = tar_alloc_cur_file(t,&curFileSize);
			while ( curFile )
			{
				strncpy(new_file_name,t->hdr.fname,MAX_PATH);

				if ( strEndsWith(new_file_name,"exe") || strEndsWith(new_file_name,"dll") )
				{
					// rename extension					
					strcpy(new_file_name + strlen(new_file_name)-3,"new");
				}
				// write out the file but don't write out the updater tarball on Windows
				if ( ( g_isMac ) || ( !strEndsWith(new_file_name,"tgz") ) )
				{
					if ( strEndsWith(new_file_name,"\\") || strEndsWith(new_file_name,"/") )
					{
						// this is a directory
						safeMkDirTree(new_file_name);
					}
					else
					{
						safeWriteFile(new_file_name,"!wb", curFile, curFileSize);
						if ( strEndsWith(t->hdr.fname,"exe") )
						{
							// rename extension to .tmp, save it
							strncpy(tmp_name,t->hdr.fname,MAX_PATH);
							strcpy(tmp_name + strlen(tmp_name)-3,"tmp");
							safeWriteFile(tmp_name,"!wb", curFile, curFileSize);
						}
					}
				}
				
				free(curFile);

				if ( tar_next(t) )
					curFile = tar_alloc_cur_file(t,&curFileSize);
				else
					curFile = NULL;
			}

			if ( strlen(tmp_name) > 0 )
			{
				execAndQuit(tmp_name);
			}
		}
		else if (cmd == PATCHSERVER_DISCONNECTED)
		{
			connectToServer();
		}
		else
		{
			msgAlertFatal("ErrUnknownMessage",cmd);
		}

	} while (cmd == PATCHSERVER_DISCONNECTED);

	pktFree(pak);

	// In the case where there is a cider update, but no executable / ui library updates
	if ( strlen(tmp_name) > 0 )
	{
		execAndQuit(tmp_name);
	}
}

void recursiveCopy(const char* szSourcePath,const char* szDestinationPath)
{
    WIN32_FIND_DATAA ffd;
    HANDLE hFind  = INVALID_HANDLE_VALUE;
	char *szPath, *szNewSourcePath, *szNewDestPath;

	szPath = (char*)malloc(sizeof(char)*MAX_PATH_MAC);
	szNewSourcePath = (char*)malloc(sizeof(char)*MAX_PATH_MAC);
	szNewDestPath = (char*)malloc(sizeof(char)*MAX_PATH_MAC);

	quick_snprintf(szPath,MAX_PATH_MAC,MAX_PATH_MAC,"%s\\*",szSourcePath);

	if ( !dirExists(szDestinationPath) )
		makeDirectories(szDestinationPath);

	szPath[MAX_PATH_MAC-1] = '\0';

	hFind = FindFirstFileA(szPath, &ffd);

    if ( hFind != INVALID_HANDLE_VALUE )
    {
        do
        {
			if ( strStartsWith(ffd.cFileName,".") )
				continue;

			quick_snprintf(szNewSourcePath,MAX_PATH_MAC,MAX_PATH_MAC,"%s\\%s",szSourcePath,ffd.cFileName);
			szNewSourcePath[MAX_PATH_MAC-1] = '\0';

			quick_snprintf(szNewDestPath,MAX_PATH_MAC,MAX_PATH_MAC,"%s\\%s",szDestinationPath,ffd.cFileName);
			szNewDestPath[MAX_PATH_MAC-1] = '\0';

			if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
				// start copying
				recursiveCopy(szNewSourcePath,szNewDestPath);
            }
			else
			{
				// straight copy the file
				CopyFile(szNewSourcePath,szNewDestPath,TRUE);
			}
        }
        while (FindNextFileA(hFind, &ffd) != 0);

		FindClose(hFind);
	}

	free(szPath);
	free(szNewSourcePath);
	free(szNewDestPath);
}

void fixApplyPatchFrameworks()
{
	HANDLE hModule = GetModuleHandle("kernel32.dll");
	BOOL (WINAPI *fnGetNativePath)(LPCSTR, LPSTR, DWORD);
	BOOL bResult = FALSE;
	int i = 0;

	if ( hModule )
	{
		fnGetNativePath = (BOOL (WINAPI*)(LPCSTR, LPSTR, DWORD))GetProcAddress(hModule,"wine_get_unix_file_name");
		if ( fnGetNativePath )
		{
			char szRootDir[MAX_PATH_MAC];
			char szDestinationPath[MAX_PATH_MAC];
			char szSourcePath[MAX_PATH_MAC];

			if ( fnGetNativePath("C:\\",szRootDir,MAX_PATH_MAC) )
			{
				// 3 is the number of directories we want to traverse up to get to the Contents/ directory
				for ( i = 0; i < 3; i++ )
				{
					char* s = strrchr(szRootDir,'/');
					if ( s )
						*s = '\0';
				}

				strcpy(szSourcePath,szRootDir);
				strcpy(szDestinationPath,szRootDir);

				snprintf(szDestinationPath,MAX_PATH_MAC,"G:\\%s\\Resources\\tgApplyPatch.app\\Contents\\Frameworks\\tgUpdate.framework",szRootDir);
				szDestinationPath[MAX_PATH_MAC-1] = '\0';
				backSlashes(szDestinationPath);

				snprintf(szSourcePath,MAX_PATH_MAC,"G:\\%s\\Frameworks\\tgUpdate.framework",szRootDir);
				szSourcePath[MAX_PATH_MAC-1] = '\0';
				backSlashes(szSourcePath);

				if ( dirExists(szDestinationPath) )
				{
					return;
				}

				// start the copying process
				recursiveCopy(szSourcePath,szDestinationPath);

				// now there are 2 symlinks we have to duplicate
				snprintf(szSourcePath,MAX_PATH_MAC,"G:\\%s\\Frameworks\\tgUpdate.framework\\Versions\\A\\Resources",szRootDir);
				szSourcePath[MAX_PATH_MAC-1] = '\0';
				backSlashes(szSourcePath);

				snprintf(szDestinationPath,MAX_PATH_MAC,"G:\\%s\\Resources\\tgApplyPatch.app\\Contents\\Frameworks\\tgUpdate.framework\\Resources",szRootDir);
				szDestinationPath[MAX_PATH_MAC-1] = '\0';
				backSlashes(szDestinationPath);

				recursiveCopy(szSourcePath,szDestinationPath);

				snprintf(szSourcePath,MAX_PATH_MAC,"G:\\%s\\Frameworks\\tgUpdate.framework\\Versions\\A",szRootDir);
				szSourcePath[MAX_PATH_MAC-1] = '\0';
				backSlashes(szSourcePath);

				snprintf(szDestinationPath,MAX_PATH_MAC,"G:\\%s\\Resources\\tgApplyPatch.app\\Contents\\Frameworks\\tgUpdate.framework\\Versions\\Current",szRootDir);
				szDestinationPath[MAX_PATH_MAC-1] = '\0';
				backSlashes(szDestinationPath);

				recursiveCopy(szSourcePath,szDestinationPath);
			}
		}
	}
}

int getLatestCiderUpdate(NetLink* comm_link, int bUseUI)
{
	Packet *pak_upd;
	HANDLE hModule = GetModuleHandle("ntdll.dll");
	U8* mem;
	U32 mem_size;
	int nCiderVer = 0;
	char szCiderID[CIDER_ID_SIZE];
	int cmd = PATCHSERVER_DISCONNECTED;
	int bResult = 0;

	BOOL (WINAPI *fnGetVersion)(char*, int);
	
	pak_upd = pktCreateEx(comm_link, PATCHCLIENT_REQ_CIDER_UPDATE);

	// extract cider version
	fnGetVersion = (BOOL (WINAPI*)(char*, int))GetProcAddress(hModule,"TGGetVersion");
	
	// extract the cider ID
	if ( !getCiderID(szCiderID ) )
	{
		// failed
		msgAlertFatal("Could not obtain game package information");
	}

	// fix up tgApplyPatch if needed
	fixApplyPatchFrameworks();

	if ( fnGetVersion )
	{
		char szVersion[MAX_PATH];

		if ( fnGetVersion(szVersion,MAX_PATH) )
		{
			nCiderVer = atoi(szVersion);

			pktSendBitsArray(pak_upd,sizeof(U32)*8,&nCiderVer);
			pktSendBitsArray(pak_upd,sizeof(char)*8*4,szCiderID);

			pktSend(&pak_upd,comm_link);
			cmd = waitForCmd(&pak_upd);

			do 
			{
				if ( cmd == PATCHSERVER_CIDER_UPDATE )
				{
					const char* szFileName = pktGetString(pak_upd);
					char szFullFileName[MAX_PATH];
					char szCiderDir[MAX_PATH];
					char szTemp[MAX_PATH];

					pktGetBitsArray(pak_upd,sizeof(U32)*8,&mem_size);

					if (( szFileName ) && ( strlen(szFileName) > 0 ))
					{
						mem = netGetUpdaterFile( szFileName, mem_size, bUseUI );

						if ( mem )
						{
							snprintf(szCiderDir,MAX_PATH,"%s\\cider",getExecutableDir(szTemp));
							snprintf(szFullFileName,MAX_PATH,"%s\\%s",szCiderDir,szFileName);

							safeMkDirTree(szFullFileName);
							safeWriteFile(szFullFileName,"!wb", mem, mem_size);

							free(mem);

							bResult = 1;
						}
					}

				}
				else if (cmd == PATCHSERVER_ERROR_MSG)
				{
					msgAlertFatal("ErrFromServer",xlateQuick(pktGetString(pak_upd)));
				}
				else if (cmd == PATCHSERVER_DISCONNECTED)
				{
					connectToServer();
				}
				else if ( cmd != PATCHSERVER_CLIENT_OK )
				{
					msgAlertFatal("ErrUnknownMessage",cmd);
				}
			} while ( cmd == PATCHSERVER_DISCONNECTED );
		}
	}

	return bResult;
}

BOOL getCiderID(char* szIDBuf)
{
	HANDLE hModule = GetModuleHandle("kernel32.dll");
	BOOL (WINAPI *fnGetNativePath)(LPCSTR, LPSTR, DWORD);
	BOOL bResult = FALSE;
	int i = 0;

	if ( hModule )
	{
		fnGetNativePath = (BOOL (WINAPI*)(LPCSTR, LPSTR, DWORD))GetProcAddress(hModule,"wine_get_unix_file_name");
		if ( fnGetNativePath )
		{
			char szNativePath[MAX_PATH_MAC];
			if ( fnGetNativePath("C:\\",szNativePath,MAX_PATH_MAC) )
			{
				// 3 is the number of directories we want to traverse up
				for ( i = 0; i < 3; i++ )
				{
					char* s = strrchr(szNativePath,'/');
					if ( s )
						*s = '\0';
				}

				if (( strlen(szNativePath) + strlen("/PkgInfo") + 1 ) < MAX_PATH_MAC )
				{
					FILE* fPkgInfo = NULL;

					strcat(szNativePath,"/PkgInfo");
					// safeFopen shows an error in case of failure
					fPkgInfo = safeFopen(szNativePath,"r");

					if ( fPkgInfo )
					{
						char szFileContents[PKGINFO_SIZE];
						if ( fread(szFileContents,1,PKGINFO_SIZE,fPkgInfo) > 0 )
						{
							// scan file contents
							if ( strStartsWith(szFileContents,"APPL") && ( strlen(szFileContents) >= strlen("APPLCOHD") ) )
							{
								memcpy(szIDBuf,szFileContents+strlen("APPL"),strlen("COHD"));
								bResult = TRUE;
							}
							else
							{
								fileMsgAlert("ErrPatchFileHeader",szNativePath);
							}
						}
						else
						{
							fileMsgAlert("ErrOpenFile",szNativePath);
						}

						fclose(fPkgInfo);
					}					
				}
				else
				{
					// log the error
					fileMsgAlert("ErrOpenFile",szNativePath);
				}
			}
		}
	}

	return bResult;
}

void deployNewComponents()
{
	U8	new_exe_name[MAX_PATH],dll_name[MAX_PATH],new_dll_name[MAX_PATH],exe_name[MAX_PATH],tmp_exe_name[MAX_PATH],prv_exe_name[MAX_PATH];
	U8  szCiderDir[MAX_PATH];
	HMODULE hNtDLL;
	BOOL (WINAPI *fnIsTransgaming)(void) = NULL;
	const char * (WINAPI *fnGetOS)(void) = NULL;
	BOOL (WINAPI *fnGetOSVer)(DWORD*,DWORD*,DWORD*) = NULL;
	void (WINAPI *fnSetUpdaterDir)(const char*) = NULL;
	BOOL (WINAPI *fnIsUpdateRequired)() = NULL;
	BOOL (WINAPI *fnVerifyUpdate)() = NULL;
	void (WINAPI *fnRunUpdateAndExit)() = NULL;

	hNtDLL = GetModuleHandle("ntdll.dll");

	fnIsTransgaming = (BOOL (WINAPI *)(void))GetProcAddress(hNtDLL,"IsTransgaming");

	fnGetOS = (const char * (WINAPI *)(void))GetProcAddress(hNtDLL,"TGGetOS");

	if ( fnGetOS && fnIsTransgaming )
	{
		if ( strcmp(fnGetOS(),"MacOSX") == 0 )
		{
			// set cider directory
			char* szCurDir = getDirectoryName(getExecutableName());
			snprintf(szCiderDir,MAX_PATH,"%s\\cider",szCurDir);

			fnSetUpdaterDir = ( void (WINAPI *)(const char*))GetProcAddress(hNtDLL,"TGSetUpdaterDir");

			if ( fnSetUpdaterDir )
			{
				fnSetUpdaterDir(szCiderDir);

				// check for update
				fnIsUpdateRequired = ( BOOL (WINAPI *)())GetProcAddress(hNtDLL,"TGIsCiderUpdateRequired");

				if (( fnIsUpdateRequired ) && ( fnIsUpdateRequired() ))
				{
					DWORD dwOSMajor,dwOSMinor,dwOSExtended;

					// determine the OS version
					fnGetOSVer = ( BOOL (WINAPI*)(DWORD*,DWORD*,DWORD*))GetProcAddress(hNtDLL,"TGMACOSGetVersion");
					if (( fnGetOSVer ) && ( fnGetOSVer(&dwOSMajor,&dwOSMinor,&dwOSExtended) ))
					{						
						int bRunUpdate = 0;

						// verify the update
						fnVerifyUpdate = ( BOOL (WINAPI *)())GetProcAddress(hNtDLL,"TGVerifyUpdateIntegrity");

						if ( dwOSMinor != 4 )
						{
							if (( fnVerifyUpdate ) && ( fnVerifyUpdate() ))
							{
								bRunUpdate = 1;
							}
						}
						else
						{
							// skip verification due to a bug with tar on Tiger
							bRunUpdate = 1;
						}

						if (bRunUpdate)
						{
							// update good. Git R Dun!!
							fnRunUpdateAndExit = ( void (WINAPI *)())GetProcAddress(hNtDLL,"TGRunUpdateAtExit");
						}
						else
						{
							// update bad. Don't really have to do anything, as the updater will redownload the update
							// on next launch.
						}
					}
				}
			}								
		}
	}
	
	strncpy(exe_name,getExecutableName(),MAX_PATH);
	backSlashes(exe_name);

	if (strEndsWith(exe_name,".tmp"))
	{
		// .tmp -> .exe
		strcpy(exe_name + strlen(exe_name)-3,"exe");

		// overwrite DLL. This function should be called before the DLL is loaded
		snprintf(dll_name,MAX_PATH,"%s\\%s",getExecutableDir(new_dll_name),getUIDLLName(TRUE));
		backSlashes(dll_name);
		strncpy(new_dll_name, dll_name, MAX_PATH);

		strcpy(new_dll_name + strlen(new_dll_name)-3,"new");

		if ( fileExists(new_dll_name) )
		{
			safeRenameFile( new_dll_name, dll_name );
			safeDeleteFile( new_dll_name );
		}

		// overwrite exe
		strncpy(new_exe_name,exe_name,MAX_PATH);		
		strcpy(new_exe_name + strlen(new_exe_name)-3,"new");

		// create temp file
		strncpy(prv_exe_name,exe_name,MAX_PATH);
		strcpy(prv_exe_name + strlen(prv_exe_name)-3,"prv");

		if ( fileExists(new_exe_name) )
		{
			safeRenameFile( exe_name, prv_exe_name );
			safeRenameFile( new_exe_name, exe_name );
		}

		if ( fnRunUpdateAndExit )
			fnRunUpdateAndExit();
		else
		{
			execAndQuit(exe_name);
		}
	}
	else if ( strEndsWith(exe_name,"exe") )
	{
		strncpy(tmp_exe_name,exe_name,MAX_PATH);
		strcpy(tmp_exe_name + strlen(tmp_exe_name)-3,"tmp");

		if ( fileExists(tmp_exe_name) )
			safeDeleteFile(tmp_exe_name);

		if ( fnRunUpdateAndExit )
			fnRunUpdateAndExit();
	}
}

int fileInImages(char *name,ImageCheck *images,int image_count)
{
	U32			baselen;
	int			i;
	ImageCheck	*image;
	char		*s;

	for(i=0;i<image_count;i++)
	{
		image = &images[i];

		baselen = strlen(image->fname);
		if (strlen(name) <= baselen)
			continue;
		s = name + baselen;
		if (*s=='/' || *s=='\\')
			s++;
		if (stashFindPointer(image->filenames,s, NULL))
			return 1;
	}
	return 0;
}

static int projectPig(char *project_dir,char *fname)
{
	char	buf[MAX_PATH];

	if (!strEndsWith(fname,".pigg"))
		return 0;
	sprintf(buf,"%s/%s",project_dir,"/piggs");
	forwardSlashes(buf);
	if (strnicmp(buf,fname,strlen(buf))==0)
		return 1;
	return 0;
}

void setRegKey(char *key,char *value)
{
	RegReader	reader;

	if (!reg_key_dir[0])
		return;
	reader = createRegReader();
	initRegReader(reader, reg_key_dir);
	rrWriteString(reader,key,value);
	destroyRegReader(reader);
}

void deleteUnlistedFiles(char *project_name,ImageCheck *image,ImageCheck *images,int image_count)
{
	int		i,count,bad_count=0;
	char	*name,**names,checksum_name[MAX_PATH],*bad_names=0;

	sprintf(checksum_name,"%s/%s.checksum",image->fname,project_name);
	forwardSlashes(checksum_name);
	names = getDirFiles(image->fname,&count);
	estrConcatf(&bad_names, "Root: \"%s\"\tFiles: ", image->fname);
	printf( "Searcing %s for bad files\n", image->fname);
	for(i=0;i<count;i++)
	{
		name = names[i];
		forwardSlashes(name);
		if (stricmp(name,checksum_name)==0)
			continue;
		if (!fileInImages(name,images,image_count))
		{
			if (projectPig(image->fname,name) || verbotenClientFile(image->fname,name))
			{
				if (1)
				{
					bad_count++;
					printf("\tDeleting %s\n", name);
					estrConcatf(&bad_names,"\"%s\" ",name);
					safeDeleteFile(name);
				}
				else
					exit(1);
			}
		}
	}
	freeDirFiles(names,count);
	removeEmptyDirs(image->fname);
	if (bad_count)
	{
		S64		seed64;
		U32		seed;
		char	bad_count_str[100];
		Packet	*pak = pktCreateEx(&comm_link,PATCHCLIENT_BADFILES);

		GET_CPU_TICKS_64(seed64);
		seed = seed64;
		bad_count |= seed << 12;
		sprintf(bad_count_str,"%d",bad_count);

		pktSendBits(pak,32,bad_count);
		pktSendZipped(pak,strlen(bad_names)+1,bad_names);
		pktSend(&pak,&comm_link);
		estrDestroy(&bad_names);

		setRegKey("PatchValue",bad_count_str);
	}
}

static void fatalAlertCallback( char *msg )
{
	Packet *pak = pktCreateEx(&comm_link,PATCHCLIENT_FATAL_ERROR);
	char callstack[2048], exe_name[MAX_PATH];
	
// send relevant info to the server
	strcpy(exe_name,getExecutableName());
	backSlashes(exe_name);
	sdDumpStackToBuffer(callstack, 2047, NULL);
	callstack[2047] = 0;

	pktSendZipped(pak,strlen(msg)+1,msg);
	pktSendZipped(pak,strlen(callstack)+1,callstack);
	pktSendBits(pak, 8, getDefaultNetworkVersion());
	pktSendBitsArray(pak,sizeof(g_updater_checksum)*8,&g_updater_checksum);
	pktSend(&pak,&comm_link);
	lnkFlushAll();
}

char	*argv[100];
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	int		i,a=0,b=1,get_major_patch=0;
	extern CRITICAL_SECTION PigCritSec;
	char	ps_name[1024] = {0};
	int		ps_port	= valid_updateserver_ports[0];
	char *patch_names[100];
	int		argc,patch_count=0,no_launch=0,first_patch_idx=0,no_pause=0;
	extern	int glob_perf_test;
	char		*default_regdir = "HKEY_CURRENT_USER\\SOFTWARE\\Cryptic",
				*default_patch = "Coh";
	char	*dir_name = "City of Heroes";
	char    *config_filename = "config.txt";
	char	locale_string[50] = {0}; // this is the string on the filename, such as "EU" for CohUpdater.EU.exe
	char *langWebname = ""; // from xlateLoad, the language the updater is using, such as "English", "French", or "German"
	int		cov_mode = 0, cov_beta_updater = 0;
	HMODULE hNtDLL;
	const char * (WINAPI *fnGetOS)(void) = NULL;
	BOOL (WINAPI *fnIsTransgaming)(void) = NULL;

	setAssertMode(IsDebuggerPresent()? ASSERTMODE_DEBUGBUTTONS : ASSERTMODE_ERRORREPORT | ASSERTMODE_MINIDUMP);
	fileDisableAutoDataDir();
	setAssertProgramVersion("Coh Updater");
	setDefaultNetworkVersion(4);
	setFatalAlertCallback(fatalAlertCallback);

	EXCEPTION_HANDLER_BEGIN
	g_force_production_mode = 1;
	disableLogging(0);
	//gimmeDLLDisable(1);
	langWebname = xlateLoad(hInstance);
	fileWatchSetDisabled(1);

	// determine OS
	hNtDLL = GetModuleHandle("ntdll.dll");

	fnIsTransgaming = (BOOL (WINAPI *)(void))GetProcAddress(hNtDLL,"IsTransgaming");
	fnGetOS = (const char * (WINAPI *)(void))GetProcAddress(hNtDLL,"TGGetOS");

	if ( fnGetOS && fnIsTransgaming )
	{
		if ( strcmp( fnGetOS(),"MacOSX") == 0 )
			g_isMac = TRUE;
	}

	pig_err_level = PIGERR_QUIET;

	argc = tokenize_line(strdup(GetCommandLine()),argv,0);
	filelog_printf( LOGID, __FUNCTION__ ": command line: %s", GetCommandLine());
	
	// do this really early
	for(i=1;i<argc;i++)
	{
		if (stricmp(argv[i],"-console")==0)
			newConsoleWindow();
	}	

	// BETA UPDATER
	//
	// If this is a beta updater, add -cov and -ps updater.cityofvillans.com to command line
	{
		char temp[512];
		// some extremely light encryption.  basically negating the bits in the string
		// the executable name ("CoVBETAUpdater.")
		int enc_exe_name[5] = {0xBDA990BC, 0xAABEABBA, 0x8B9E9B8F, 0xFFD18D9A}, 
		// the patch server name ("update.cityofvillains.com")
			enc_ps_name[] = {0x9E9B8F8A, 0x9CD19A8B, 0x90868B96, 0x93968999, 0x91969E93, 0x909CD18C, 0xFFFFFF92 },
		// the eu patch server name ("covupdateeu.cityofvillains.com")
			enc_eu_ps_name[] = {0x8A89909C, 0x8B9E9B8F, 0xD18A9A9A, 0x868B969C, 0x96899990, 0x969E9393, 0x9CD18C91, 0xFFFF9290};
		strcpy(temp, getFileName(argv[0]));
		// "encrypt" the filename given by the command line
		for ( i = 0; i < 5; ++i )
		{
			enc_exe_name[i] = ~enc_exe_name[i];
		}
		if ( simpleMatch( (char*)enc_exe_name, temp ) ) 
		{
			char tmpCmdLine[2048];
			int len;

			cov_mode = cov_beta_updater = 1;
			filelog_printf( LOGID, __FUNCTION__ ": in beta updater ");
			
			// find locale
			strcpy( temp, (char*)(strchr(temp, '.')+1) );
			// for CoVBetaUpdater.*.exe, copy * into locale_string
			{char *c = strrchr( temp, '.' ); if(c){ *c = 0; strcpy( locale_string, temp ); }}

			// set the global command line to the modified one
			sprintf( tmpCmdLine, "%s -test -cov", lpCmdLine );
			// "decrypt" the ps name and set it as the patchserver
			if ( !locale_string[0] )
			{
				for ( i = len = 0; ((char*)enc_ps_name)[i] != 0; i++, len++ )
				{
					ps_name[len] = ~((char*)enc_ps_name)[i];
				}
			}
			else if ( stricmp( locale_string, "EU" ) == 0 )
			{
				for ( i = len = 0; ((char*)enc_eu_ps_name)[i] != 0; i++, len++ )
				{
					ps_name[len] = ~((char*)enc_eu_ps_name)[i];
				}
			}

			g_cmdline = strdup(tmpCmdLine);
			filelog_printf( LOGID, __FUNCTION__ ": simple match: g_cmdline=%s", tmpCmdLine);
			
			// add arguments to argv
			argv[argc] = strdup("-cov");
			argv[argc+1] = strdup("-test");
			argc += 2;
		}
		else
		{
			g_cmdline = strdup(lpCmdLine);
			filelog_printf( LOGID, __FUNCTION__ ": g_cmdline=%s", lpCmdLine);
		}
	}

	InitializeCriticalSection(&PigCritSec);
	sharedMemorySetMode(SMM_DISABLED);
	cryptInit();

	if ( !cov_beta_updater )
	{
		filelog_printf( LOGID, __FUNCTION__ ": !cov_beta_updater");
		{
			char *stemp = strrchr(argv[0], '.');
			char *progname = getFileName(argv[0]);
			
			if (stemp)
			{
				*stemp = 0;
				if (simpleMatch("cohupdater.*", progname))
				{
					strcpy(locale_string, getLastMatch());
					printf("Locale String: %s\n", locale_string);
				}
				*stemp = '.';
			}
		}

		sprintf(ps_name, "cohupdate%s.coh.com", locale_string);
		filelog_printf( LOGID, __FUNCTION__ ": ps_name=%s", ps_name);
	}

	// for EU updater, show language selection button
	if (stricmp(locale_string, "EU")==0)
	{		
		g_show_language_button = 1;
		g_isEU = 1;
	}	

	//if (stricmp(locale_string, "KR")==0)
	printf("Processing argument list:\n");
	patch_names[patch_count++] = default_patch;
	for(i=1;i<argc;i++)
	{
		printf( "%s", argv[i] );

		if (stricmp(argv[i],"-server")==0)
		{
			isClientOnly = 0;
			isFreshInstall = 0;
			g_server_patch = 1;
			no_self_update = 1;
		}
		if (stricmp(argv[i],"-nogui")==0)
		{
			setGuiDisable(true);
			setAssertMode(ASSERTMODE_STDERR | ASSERTMODE_EXIT);
		}
		else if (stricmp(argv[i],"-safeserver")==0)
		{
			isClientOnly = 0;
			isFreshInstall = 0;
			g_safe_server = 1;
			g_server_patch = 1;
			no_self_update = 1;
		}
		else if (stricmp(argv[i],"-perftest")==0)
			glob_perf_test = 1;
		else if (stricmp(argv[i],"-mp")==0)
		{
			get_major_patch = 1;
			WebPageKey = "MajorPatchWebPage";
			filelog_printf( LOGID, __FUNCTION__ ": setting get_major_patch");
		}
		else if (stricmp(argv[i],"-perftest2")==0)
			glob_perf_test = 2;
		else if ((stricmp(argv[i],"-ps")==0 || stricmp(argv[i],"-us")==0) && i < argc-1)
		{
			strcpy(ps_name, argv[++i]);
			printf( " \"%s\"", argv[i] );
		}
		else if (stricmp(argv[i],"-port")==0 && i < argc-1)
			ps_port = atoi(argv[++i]);
		else if (stricmp(argv[i],"-project")==0 && i < argc-1)
		{
			isClientOnly = 0;
			patch_names[patch_count++] = argv[++i];
		}
		else if (stricmp(argv[i],"-checksum")==0)
			g_full_checksum = 1;
		else if (stricmp(argv[i],"-nolaunch")==0)
			no_launch = 1;
		else if (stricmp(argv[i],"-noself")==0)
			no_self_update = 1;
		else if (stricmp(argv[i],"-nodefault")==0)
		{
			no_launch = 1;
			first_patch_idx = 1;
			dir_name = "";
		}
		else if (stricmp(argv[i],"-game")==0 && i < argc-1)
			dir_name = patch_names[0] = argv[++i];
		else if (stricmp(argv[i],"-config")==0 && i < argc-1)
			config_filename = argv[++i];
		else if (stricmp(argv[i],"-url")==0 && i < argc-1)
			g_override_url = argv[++i];
		else if (stricmp(argv[i],"-nopause")==0)
			no_pause = 1;
		else if (stricmp(argv[i],"-lag")==0 && i < argc-1)
			g_lag = atoi(argv[++i]);
		else if (stricmp(argv[i],"-send")==0 && i < argc-1)
			g_send_size = atoi(argv[++i]);
		else if (stricmp(argv[i],"-recv")==0 && i < argc-1)
			g_recv_size = atoi(argv[++i]);
		else if (stricmp(argv[i],"-test")==0)
		{
			if ( !cov_beta_updater )
			{
				sprintf(ps_name, "cohtestupdate%s.coh.com", locale_string);
			}
			dir_name = patch_names[0] = "CohTest";
			strcat(g_primary_project_display_name, "Test");
		}
		else if (stricmp(argv[i],"-folder")==0 && i < argc-1)
		{
			strcpy(g_override_install_dir, argv[++i]);
			printf( " \"%s\"", argv[i] );
		}
		else if (stricmp(argv[i],"-cov")==0)
		{
			// no need to do it twice
			if ( g_COV )
				continue;

			strcpy(g_game_name, "City of Villains");
			strcpy(g_updater_name, "CovUpdater");
			if (strcmp(g_primary_project_display_name, "CohTest")==0)
				strcpy(g_primary_project_display_name, "CovTest");
			else
				strcpy(g_primary_project_display_name, "Cov");

			// pass on to the game
			strcatf(passthrough_args, " -cov");

			g_COV = 1;
		}
        else if (stricmp(argv[i],"-enablelogging")==0)
        {
            disableLogging(0);
        }
		else
		{
			char	*s = argv[i];
			int		len = strlen(s);

			if (!len || (int)strcspn(s," \t") != len)
			{
				s[len+1] = 0;
				*(--argv[i]) = s[len] = '"';
			}
			strcatf(passthrough_args," %s",argv[i]);
			if (strstri(passthrough_args,".cohdemo"))
				no_pause = 1;
		}

		printf( "\n" );
	}

	printf( "Patchserver: %s\n", ps_name );

	// update UI library and executable if running as tmp
	deployNewComponents();

	// initialize network stuff
	patchClientInit(ps_name,ps_port);

	// Initialize UI library
	if (! ( InitImports() && UpdaterUI_Init() ))
	{
		// UI library not present or version mismatch. Do a headless update.
		filelog_printf( LOGID, __FUNCTION__ ": doing headless update");

		g_bUseUI = 0;

		connectToServer();

		exit(0);
	}

	if (!g_server_patch)
	{
		char		webpage[MAX_PATH],reg_dir[MAX_PATH];
		RegReader	reader = createRegReader();
		ImageCheck	*images = calloc(sizeof(ImageCheck),patch_count);
		int			forceverify = 0;
		char		locale_project_name[1024];  // this is "EUCoH", "CoH", "EUCoH Test", or "CoH Test"
		char		installerLang[256]; // this is the string version of the LCID of the installer language to use, ie "1033"
		char		tempBuffer[256];
		int			tempInt;

		filelog_printf( LOGID, __FUNCTION__ ": not server patch");
		
		sprintf(locale_project_name, "%s%s",locale_string,patch_names[first_patch_idx]);

		regSetAppName(locale_project_name);

		sprintf(reg_dir,"%s\\%s",default_regdir,locale_project_name);
		initRegReader(reader, reg_dir);

		if (rrReadString(reader, "fullScreen", tempBuffer, 256))
		{
			filelog_printf( LOGID, __FUNCTION__ ": found fullscreen string. not fresh install");
			isFreshInstall = 0;
		}
		

		if (rrReadInt(reader, "fullScreen", &tempInt))
		{
			filelog_printf( LOGID, __FUNCTION__ ": found fullscreen int. not fresh install");
			isFreshInstall = 0;
		}
		
		sprintf(LangEulaWebPageKey, "%s%s", EulaWebPageKey, langWebname);

		sprintf(LangWebPageKey, "%s%s", WebPageKey, langWebname);
		if (!rrReadString(reader, LangWebPageKey, webpage, sizeof(webpage)))
		{
			if (!rrReadString(reader, WebPageKey, webpage, sizeof(webpage)))
				strcpy(webpage,"http://www.coh.com/beta/motd.html");

			filelog_printf( LOGID, __FUNCTION__ ": set motd web");
		}
		if (rrReadInt(reader, "VerifyOnNextUpdate", &forceverify) && forceverify)
		{
			filelog_printf( LOGID, __FUNCTION__ ": VerifyOnNextUpdate reg, setting g_full_checksum");
			g_full_checksum = 1;
		}
		
		if (g_override_url)
			strcpy(webpage,g_override_url);

		if (locale_string[0])
		{
			printf( "Non-US locale\n" );
			// non-US server: if they don't have "Installer Language" set then they are a beta tester, set their language and game locale based on their windows locale.
			if (!rrReadString(reader, "Installer Language", installerLang, 256))
			{
				int instLang = xlateGetLoadedLocale();
				sprintf(installerLang, "%d", instLang);
				rrWriteString(reader, "Installer Language", installerLang);
				locSetIDInRegistry(locGetIDByWindowsLocale(instLang));
			}
			log_printf(LOGID, __FUNCTION__ ": installer lang=%s", installerLang);
		}
		else
		{
			// set locale and installer language to English for US servers
			int instLang = LOCALE_ENGLISH;
			printf( "US locale\n" );
			sprintf(installerLang, "%d", instLang);
			rrWriteString(reader, "Installer Language", installerLang);
			locSetIDInRegistry(locGetIDByWindowsLocale(instLang));
		}

		destroyRegReader(reader);
		startInstallDialog(hInstance,webpage);
		connectToServer();
		if (get_major_patch)
		{
			char	install_dir[REG_STR_BUF_SIZE] = {0};

			filelog_printf( LOGID, __FUNCTION__ ": Getting major patch");
			printf( "Getting Major Patch\n" );
			if (g_override_install_dir[0])
			{
				strcpy(install_dir, g_override_install_dir);
				makeDirectories(install_dir);
			}
			else
			{
				reader = createRegReader();
				initRegReader(reader, reg_dir);
				rrReadString(reader, "Installation Directory", install_dir, REG_STR_BUF_SIZE);
				destroyRegReader(reader);
			}
			if (!install_dir[0])
				msgAlertFatal("ErrNoInstallDir");
			patchGetMajor(patch_names[0],install_dir);
			disconnectFromServer();

			UpdaterUI_Destroy();
			exit(0);
		}

		filelog_printf( LOGID, __FUNCTION__ ": first_patch_idx=%i, patch_count=%i", first_patch_idx, patch_count);

		for(i=first_patch_idx;i<patch_count;i++)
			patchClientProject(default_regdir,locale_string,patch_names[i],dir_name,i==0,&images[i]);
		for(i=first_patch_idx;i<patch_count;i++)
			deleteUnlistedFiles(patch_names[i],&images[i],&images[first_patch_idx],patch_count - first_patch_idx);
		disconnectFromServer();
		if (!no_pause && (1 || g_got_patch || no_launch))
		{
			reader = createRegReader();
			initRegReader(reader, reg_dir); // I'm having visions of Pascal-style constructors..
			rrDelete(reader, "VerifyOnNextUpdate");
			destroyRegReader(reader);

			patchUiChangeCancelToPlay();
			patchUiSetProjectInfo(0,0);
			g_user_quit++;
			for(;;)
			{
				Sleep(1);
				if (g_user_quit > 1)
				{
					OutputDebugStr("Quitting\n");
					break;
				}
			}
		}
		if (!no_launch)
		{
			HANDLE hProcess;

			if ( !g_safe_mode )	
				hProcess = launchProgram(default_regdir,locale_project_name,"CityOfHeroes.exe");
			else
				hProcess = launchProgram(default_regdir,locale_project_name,"CityOfHeroes.exe -safemode");

			WaitForInputIdle(hProcess, 1000);
		}
	}
	else
	{
		newConsoleWindow();
		projectSync(config_filename);
	}

	UpdaterUI_Destroy();

	cleanupLockFile();
	return 0;
	EXCEPTION_HANDLER_END
}
