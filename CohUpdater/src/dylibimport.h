#ifndef DYLIB_IMPORT_H
#define DYLIB_IMPORT_H

// Functions not forwarded to the library
BOOL InitImports();
char* getUIDLLName();

#define UPDATER_UI_VERSION 1

// Enums
typedef enum UpdaterTextArea
{
	// Upper right button text
	UTA_UR_BTN_TEXT=0,
	// Lower right button text
	UTA_LR_BTN_TEXT=1,
	// Lower left button text
	UTA_LL_BTN_TEXT=2,
	// Checkbox text
	UTA_CHKBOX_TEXT=3,
	// Patcher version
	UTA_PATCHER_VER_TEXT=4,
	// Amount downloaded
	UTA_PROGRESS_DL_TEXT=5,
	// Speed
	UTA_PROGRESS_SPEED_TEXT=6,
	// Time Remaining
	UTA_PROGRESS_TIMELEFT_TEXT=7,
	// Status (OK, Applying, etc)
	UTA_PROGRESS_STATUS_TEXT=8,
	// Browser URL
	UTA_BROWSER_URL=9,
	// Window title
	UTA_WINDOW_TITLE=10,
	// Language Button
	UTA_LANG_BTN_TEXT=11
} UpdaterTextArea;

typedef enum UpdaterIconType
{
	UIT_STANDARD_UPDATER=0,
	UIT_COV_UPDATER=1,
	UIT_KOREAN_UPDATER=2
} UpdaterIconType;

// Callback function typedefs
typedef void(__cdecl *UpdaterUI_GenericFn)(void);

// Initialization
int  UpdaterUI_Init(void);
void UpdaterUI_Destroy(void);
int  UpdaterUI_GetVersion(void);

// Project name - this can CoH or CoV. Determines what icon we use
void UpdaterUI_SetProjectName(const char* szProjName);
void UpdaterUI_SetCurLangID(int nID);

// Window creation
int  UpdaterUI_DialogCreate(void);
void UpdaterUI_DialogDestroy(void);

// Text setting
void UpdaterUI_DialogSetLocalizedTextW(UpdaterTextArea eArea, const unsigned short* wcsText, int nLength);
void UpdaterUI_DialogSetLocalizedTextA(UpdaterTextArea eArea, const char* wcsText, int nLength);

void UpdaterUI_AddLanguage(const char* szLanguage, int nID);

// Element control
void UpdaterUI_URButtonDisplay(int bShow);
void UpdaterUI_LRButtonDisplay(int bShow);
void UpdaterUI_LLButtonDisplay(int bShow);	
void UpdaterUI_CheckboxDisplay(int bShow);
void UpdaterUI_ProgressBarDisplay(int bShow);
void UpdaterUI_ShowLanguageButton(int bShow);

void UpdaterUI_SetProgressBarTotal(float fValue);
void UpdaterUI_SetProgressBar(float fValue);

// Callback setting - Lower Right, Upper Right, and Lower Left
void UpdaterUI_SetLRBtnCallback(UpdaterUI_GenericFn fnLRBtnCB);
void UpdaterUI_SetURBtnCallback(UpdaterUI_GenericFn fnURBtnCB);
void UpdaterUI_SetLLBtnCallback(UpdaterUI_GenericFn fnLLBtnCB);

void UpdaterUI_SetCloseCallback(UpdaterUI_GenericFn fnCloseCB);
void UpdaterUI_SetMinimizeCallback(UpdaterUI_GenericFn fnMinCB);
void UpdaterUI_SetRestoreCallback(UpdaterUI_GenericFn fnRestoreCB);

void UpdaterUI_SetLangCallback(UpdaterUI_GenericFn fnLangCB);

// Getting information
int UpdaterUI_GetCheckboxState();
int UpdaterUI_GetSelectedLanguageID();

// Installer - these will be cheerfully ignored by the Mac version
void UpdaterUI_SetInstallDir(const char* szDir);
const char* UpdaterUI_ChooseInstallDir(const char* szDialogTitle);

#endif