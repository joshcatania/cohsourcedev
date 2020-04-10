#include <wininclude.h>
#include <shlobj.h>
#include <string.h>
#include <stdio.h>
#include "windows.h"
#include "resource.h"
#include "embedbrowser.h"
#include "error.h"
#include "timing.h"
#include "patchui.h"
#include "patchutils.h"
#include "patchfileutils.h"
#include "fileUtil.h"
#include <tchar.h>
#include "utils.h"
#include "projectfile.h"
#include "xlate.h"
#include "RegistryReader.h"
#include "AppLocale.h"
#include "osdependent.h"
#include "convertUtf.h"
#include "stringutil.h"
#include "sysutil.h"

// UI functions from a DLL
#include "dylibimport.h"

extern MessageStore *g_msg_store;

extern int g_show_language_button;

static	char	install_dir[MAX_PATH],last_url[MAX_PATH],next_url[MAX_PATH];
HWND	install_dialog;
extern volatile int g_user_quit;
extern FILE *g_lock_file;
HINSTANCE hInst;
extern int g_full_checksum;
extern char g_updater_name[256];
extern char g_game_name[256];
extern char g_primary_project_display_name[256];
extern int g_safe_mode;
extern int g_isMac;

volatile int install_dialog_ready,html_ready,ok_to_play=0;
int on_eula_page = 0;

extern char	*argv[100];

static int minimized;
static int g_bQuit = 0;

static void redrawInstallDialog(DataXferInfo *info);

// Callback definitions
void DialogOnLLBtnPressed();
void DialogOnURBtnPressed();
void DialogOnLRBtnPressed();
void DialogOnClosePressed();
void DialogOnMinimized();
void DialogOnRestored();
void DialogOnLanguageBtnPressed();

void setUpdateWindowText(char *msg)
{
	char	buf[1000] = "";
	U16		test[] = { 'T', 'E', 'S', 'T', 0 };

	//sprintf(buf,"%s  -  ",xlateQuick(msg));
	xlatePrintf(buf + strlen(buf),ARRAY_SIZE(buf) - strlen(buf),"UpdaterTitleName", g_game_name);
	if (project_list.client_version)
		strcatf(buf," v%s",project_list.client_version);
	//SetWindowTextW(install_dialog,xlateToUnicode(buf));
	
	UPDATER_UI_SET_WINDOW_TEXT( buf );

	//SetWindowTextW(install_dialog,test);
}

int winMsgOkCancel(char *str)
{
	wchar_t wstr[1000];
	int retval;

	if (MessageBoxW( install_dialog, xlateToUnicodeBuffer(str, wstr, 1000), xlateToUnicode(xlateQuick(g_updater_name)), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
		retval = 1;
	else
		retval = 0;
	return retval;
}

#define ID_LANGMENU_START 9001
void displayLanguageMenu()
{
	// FIXME to be implemented
}

void updateUiStats(void *xfer_info)
{
	redrawInstallDialog(xfer_info);

	printTextStats(xfer_info);
}

char *pickInstallDir(char *project_name,char *dir_name)
{
	static char title_buf[1000];
	char	msg[1000],drive,program_files[1000],proj_xlate[100];
	const char*	szDir = NULL;
	//wchar_t wmsg[1000];
	int korean = locGetIDInRegistry() == locGetIDByWindowsLocale(LOCALE_KOREAN);

	xlatePrint(SAFESTR(proj_xlate),project_name);
	xlatePrintf(SAFESTR(title_buf),"ChooseInstallDir",proj_xlate);
	SHGetSpecialFolderPath(install_dialog,program_files,CSIDL_PROGRAM_FILES,FALSE);
	printf("Program Files Directory: %s\n", program_files);
	if (!strlen(program_files))
	{
		for(drive='c';drive<='z';drive++)
		{
			if (korean)
				sprintf(program_files,"%c:\\Program Files\\NCSOFT\\%s",drive, xlateQuick("Coh"));
			else
				sprintf(program_files,"%c:\\Program Files",drive);
			if (dirExists(program_files))
				break;
		}
		if (drive < 'a' || drive > 'z')
			program_files[0] = 0;
	}
	if ( korean && program_files[0] )
	{
		sprintf(program_files,"%s\\NCSOFT",program_files);
		printf("Korean Program Files Directory: %s\n", program_files);
	}
	if (program_files[0])
	{
		strcpy(install_dir,program_files);
		if (install_dir[strlen(install_dir)-1]!='\\')
			strcat(install_dir, "\\");
		strcat(install_dir, dir_name);
	}
	for(;;)
	{
		printf("Install Directory: %s\n", install_dir);
		if (install_dir[0])
		{
			xlatePrintf(SAFESTR(msg),"ChooseInstallFolderText",g_primary_project_display_name,install_dir);
			//if (IDYES==MessageBoxW(install_dialog, xlateToUnicodeBuffer(msg, wmsg, 1000), xlateToUnicode(xlateQuick("ChooseInstallFolderDialogTitle")), MB_YESNO | MB_SYSTEMMODAL |MB_ICONQUESTION))
			if (IDYES==MessageBox(install_dialog, msg, xlateQuick("ChooseInstallFolderDialogTitle"), MB_YESNO | MB_SYSTEMMODAL |MB_ICONQUESTION))
			{
				char patch_client_dir[MAX_PATH];
				if ( korean )
					sprintf(patch_client_dir, "%s/%s", install_dir, "CohUpdater.kr.exe");
				else if ( locGetIDInRegistry() != locGetIDByWindowsLocale(LOCALE_ENGLISH) )
					sprintf(patch_client_dir, "%s/%s", install_dir, "CohUpdater.eu.exe");
				else
					sprintf(patch_client_dir, "%s/%s", install_dir, "CohUpdater.exe");
				printf("New Updater Dir: %s\n", patch_client_dir);
				
				{
					char shortcut[MAX_PATH];
					if ( !dirExists(install_dir) )
						makeDirectoriesForFile(patch_client_dir);
					safeCopyFile(argv[0], patch_client_dir);
					SHGetSpecialFolderPath(install_dialog, shortcut, CSIDL_DESKTOPDIRECTORY, 0);
					sprintf(shortcut, "%s\\%s", shortcut, xlateQuick("Coh"));
					printf("Creating shortcut to %s at %s\n", patch_client_dir, shortcut);
					createShortcut(patch_client_dir, shortcut, korean?3:0, NULL, NULL, xlateQuick("Coh"));
				}
				return install_dir;
			}
		}

		// Have the user choose a folder.

		UpdaterUI_SetInstallDir(program_files);
		szDir = UpdaterUI_ChooseInstallDir(title_buf);

		// Ugly!
		if ( szDir )
			strncpy(install_dir,szDir,260);

		if (!strEndsWith(install_dir,project_name))
		{
			if (!strEndsWith(install_dir,"\\"))
				strcat(install_dir,"\\");
			strcat(install_dir,dir_name);
		}
	}
}

static void quitNow(BOOL fromUI)
{
	if ( ! g_bQuit )
	{
		UpdaterUI_DialogDestroy();

		if (fromUI)
		{
			UpdaterUI_Destroy();

			g_bQuit = 1;
		}
	}
}

static void redrawInstallDialog(DataXferInfo *info)
{
	if (!info)
		return;

	UpdaterUI_DialogSetLocalizedTextA( UTA_PROGRESS_STATUS_TEXT, info->mode, strlen(info->mode) );
	UpdaterUI_DialogSetLocalizedTextA( UTA_PROGRESS_SPEED_TEXT, info->bytespersec_str, strlen(info->bytespersec_str) );
	UpdaterUI_DialogSetLocalizedTextA( UTA_PROGRESS_DL_TEXT, info->bytesleft_str, strlen(info->bytesleft_str) );
	UpdaterUI_DialogSetLocalizedTextA( UTA_PROGRESS_TIMELEFT_TEXT, info->timeleft_str, strlen(info->timeleft_str) );
	
	UpdaterUI_SetProgressBarTotal( info->total_bytes / 16 );
	UpdaterUI_SetProgressBar( info->curr_bytes / 16 );

	setUpdateWindowText(info->timeleft_str);
}

// loads eula into memory and displays it
extern char reg_key_dir[MAX_PATH];
extern char LangEulaWebPageKey[256];
extern char *EulaWebPageKey;
extern int  g_isEU;
void ShowEula()
{
	char EulaWebPage[1000];
	char version[256];
	RegReader	reader;
	on_eula_page = 1;
	//SetDlgItemTextW(install_dialog, IDOK, xlateToUnicode(xlateQuick("EulaAgree")));
	UPDATER_UI_SET_ITEM_TEXT( UTA_LR_BTN_TEXT, xlateQuick("EulaAgree"));
	
	UpdaterUI_ProgressBarDisplay( 0 );
	UpdaterUI_LLButtonDisplay( 1 );
	
	if (g_show_language_button)		
		UpdaterUI_ShowLanguageButton( 1 );


	reader = createRegReader();
	initRegReader(reader, reg_key_dir);
	if ( g_isMac )
	{
		char szBuf[MAX_PATH];
		char* szLocaleID = locGetAlpha2(locGetIDInRegistry());

		snprintf(EulaWebPage,1000,"file://%s\\cider\\eula-en.html",getExecutableDir(szBuf));
		if ( g_isEU )
		{			
			snprintf(EulaWebPage,1000,"file://%s\\cider\\eula-eu-%s.html",getExecutableDir(szBuf),szLocaleID);
		}
	}
	else if (!rrReadString(reader, LangEulaWebPageKey, EulaWebPage, sizeof(EulaWebPage)))
	{
		if (!rrReadString(reader, EulaWebPageKey, EulaWebPage, sizeof(EulaWebPage)))
			strcpy(EulaWebPage,"http://www.cityofheroes.com/beta/eula.html");
	}
	// see if we have a version that has safe mode capability
	if (rrReadString(reader, "CurrentVersion", version, 256))
	{
		if ( getUpdateVersion(version) >= 6 )
		{
			UPDATER_UI_SET_ITEM_TEXT( UTA_CHKBOX_TEXT, xlateQuick("SafeMode") );
			UpdaterUI_CheckboxDisplay( 1 );
		}
	}
	destroyRegReader(reader);

	// FIXME - check diff between DisplayHTMLPage and straight copying of URL
	UpdaterUI_DialogSetLocalizedTextA( UTA_BROWSER_URL, EulaWebPage, strlen(EulaWebPage));
}

void patchUiChangeCancelToPlay()
{
	on_eula_page = 0;
	ok_to_play = 1;
	//SetDlgItemTextW(install_dialog, IDOK, xlateToUnicode(xlateQuick("NextPage")));
	UPDATER_UI_SET_ITEM_TEXT( UTA_LR_BTN_TEXT, xlateQuick("NextPage"));

	setUpdateWindowText("PLAY!");

	if (!g_full_checksum) 
		UpdaterUI_URButtonDisplay( 1 );
}

void patchUiSetProjectInfo(char *project,char *version)
{
	static char lastproject[200] = "";
	static char lastversion[200] = "";
	char buf[500] = {0,};
	int		idx=0;
	//wchar_t *wcBuf = NULL;

	// hack - remember our project and version
	if (project)
	{
		if (strEndsWith(project, "coh") || strEndsWith(project, "cohtest"))
		{
			strcpy_s(SAFESTR(lastproject), xlateQuickUTF8(g_primary_project_display_name));
			//xlatePrint(SAFESTR(lastproject), g_primary_project_display_name);
		}
		else
		{
			strcpy_s(SAFESTR(lastproject), xlateQuickUTF8(project));
			//xlatePrint(SAFESTR(lastproject), project);
		}
	}
	if (version)
		strcpy(lastversion, version);

	if (g_full_checksum)
	{
		if (lastproject[0])
			idx += sprintf(buf + idx, "%s - ", lastproject);
		if (ok_to_play)
		{
			//idx += xlatePrint(buf + idx, ARRAY_SIZE_CHECKED(buf) - idx, "AllFilesVerified");
			strcat_s(buf + idx, ARRAY_SIZE_CHECKED(buf)-idx, xlateQuick("AllFilesVerified"));
		}
		else
		{
			//idx += xlatePrint(buf + idx, ARRAY_SIZE_CHECKED(buf) - idx, "VerifyingAllFiles");
			strcat_s(buf + idx, ARRAY_SIZE_CHECKED(buf)-idx, xlateQuick("VerifyingAllFiles"));
		}
	}
	else
	{
		if (lastproject[0])
		{
			idx += xlatePrintf(buf + idx, ARRAY_SIZE_CHECKED(buf) - idx, "Project", lastproject);
			if (lastversion[0])
				idx += sprintf(buf + idx, " - ");
		}
		if (lastversion[0])
			idx += xlatePrintf(buf + idx, ARRAY_SIZE_CHECKED(buf) - idx, "Version", lastversion);
	}

	UPDATER_UI_SET_ITEM_TEXT( UTA_PATCHER_VER_TEXT, buf);
}

// moving controls around..  I'm so going to get fired for dicking with this..
static int verifyButtonVisible = 1;

void startInstallDialog(void *hInstance,char *url)
{
	int i;
	int curID = locGetIDInRegistry();

	UpdaterUI_DialogCreate();

	UpdaterUI_SetLLBtnCallback( (UpdaterUI_GenericFn)DialogOnLLBtnPressed );
	UpdaterUI_SetLRBtnCallback( (UpdaterUI_GenericFn)DialogOnLRBtnPressed );
	UpdaterUI_SetURBtnCallback( (UpdaterUI_GenericFn)DialogOnURBtnPressed );

	UpdaterUI_SetCloseCallback( (UpdaterUI_GenericFn)DialogOnClosePressed );
	UpdaterUI_SetMinimizeCallback( (UpdaterUI_GenericFn)DialogOnMinimized );
	UpdaterUI_SetRestoreCallback( (UpdaterUI_GenericFn)DialogOnRestored );

	UpdaterUI_SetLangCallback( (UpdaterUI_GenericFn)DialogOnLanguageBtnPressed );

	UpdaterUI_DialogSetLocalizedTextA( UTA_BROWSER_URL, url, strlen(url) );
	UpdaterUI_DialogSetLocalizedTextA( UTA_LR_BTN_TEXT, xlateQuick("QUIT"), strlen(xlateQuick("QUIT")) );
	UpdaterUI_DialogSetLocalizedTextA( UTA_LL_BTN_TEXT, xlateQuick("EulaReject"), strlen(xlateQuick("EulaReject")) );
	UpdaterUI_DialogSetLocalizedTextA( UTA_UR_BTN_TEXT, xlateQuick("VerifyAllFiles"), strlen(xlateQuick("VerifyAllFiles")) );
	UpdaterUI_DialogSetLocalizedTextA( UTA_LANG_BTN_TEXT, xlateQuick("LanguageSelect"), strlen(xlateQuick("LanguageSelect")) );

	UpdaterUI_URButtonDisplay( 0 );

	UpdaterUI_SetCurLangID(ID_LANGMENU_START + curID);

	for (i = 0; i < locGetMaxLocaleCount(); i++)
	{
		if (!locIsUserSelectable(i))
			continue;

		UpdaterUI_AddLanguage( xlateQuick(locGetName(i)), ID_LANGMENU_START + i );
	}

	// FIXME to be implemented
/*
	if (locGetIDInRegistry() == locGetIDByWindowsLocale(LOCALE_KOREAN))
	{
		wc.hIcon         = LoadImage(hInstance, MAKEINTRESOURCE(IDI_KOREANUPDATER), IMAGE_ICON, 
			GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
		wc.hIconSm		 = LoadImage(hInstance, MAKEINTRESOURCE(IDI_KOREANUPDATER), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	}
	else if (stricmp(g_primary_project_display_name, "Cov")==0)
	{
		wc.hIcon         = LoadImage(hInstance, MAKEINTRESOURCE(IDI_COVUPDATER), IMAGE_ICON, 
			GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
		wc.hIconSm		 = LoadImage(hInstance, MAKEINTRESOURCE(IDI_COVUPDATER), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	}
	else
	{
		wc.hIcon         = LoadImage(hInstance, MAKEINTRESOURCE(IDI_UPDATER), IMAGE_ICON, 
			GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
		wc.hIconSm		 = LoadImage(hInstance, MAKEINTRESOURCE(IDI_UPDATER), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	}
	RegisterClassEx(&wc);
	...
	if (locGetIDInRegistry() == locGetIDByWindowsLocale(LOCALE_KOREAN))
	{
		icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_KOREANUPDATER));
	}
	else if (stricmp(g_primary_project_display_name, "Cov")==0)
	{
		icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_COVUPDATER));
	}
	else
	{
		icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_UPDATER));
	}

	SendMessage(install_dialog, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	SendMessage(install_dialog, WM_SETICON, ICON_BIG, (LPARAM)icon);
*/
	// UpdaterUI_SetIcon( UIT_STANDARD_UPDATER );
}

void patchShowWebPage(char *url)
{
	extern char	*g_override_url;

	if (g_override_url)
		UpdaterUI_DialogSetLocalizedTextA( UTA_BROWSER_URL, g_override_url, strlen(g_override_url) );
	else
		UpdaterUI_DialogSetLocalizedTextA( UTA_BROWSER_URL, url, strlen(url) );
}

void DialogOnLanguageBtnPressed()
{
	int nLangID = UpdaterUI_GetSelectedLanguageID();
	
	if ( nLangID >= 0 )
		locSetIDInRegistry(nLangID - ID_LANGMENU_START);
}

void DialogOnLLBtnPressed()
{
	// Cancel button
	quitNow(TRUE);

	cleanupLockFile();
	exit(0);
}

void DialogOnURBtnPressed()
{
	// Verify all
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char * commandline = malloc(100 + strlen(GetCommandLine()));

	// run ourselves with the -checksum command
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	sprintf(commandline, "%s -checksum -noself", GetCommandLine());
	CreateProcess(NULL, commandline, 0, 0, 0, CREATE_NEW_CONSOLE, 0, 0, &si, &pi);
	free(commandline);

	quitNow(TRUE);
	cleanupLockFile();
	exit(0);
}

void DialogOnLRBtnPressed()
{
	// OK Button
	if (ok_to_play && !on_eula_page)
	{
		ShowEula();
	}
	else
	{
		g_user_quit++;
		g_safe_mode = UpdaterUI_GetCheckboxState();

		if (g_isMac)
		{
			quitNow(FALSE);
		}

		cleanupLockFile();
		
		if ( !ok_to_play )
			exit(0);
	}
}

void DialogOnClosePressed()
{
	quitNow(TRUE);
	cleanupLockFile();

	exit(0);
}

void DialogOnMinimized()
{
	if (ok_to_play)
	{
		setUpdateWindowText("PLAY!");
	}

	minimized = 1;
}

void DialogOnRestored()
{
	char buffer[256];
	xlatePrintf(SAFESTR(buffer), "UpdaterTitleName", g_game_name);
	UPDATER_UI_SET_WINDOW_TEXT( buffer );

	minimized = 0;
}