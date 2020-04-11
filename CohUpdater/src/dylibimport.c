//
//  dylibimport.c
//  
//
//  Created by Roman Kliotzkin on 8/18/08.
//  

#include <windows.h>
#include <stdio.h>
#include "dylibimport.h"

HMODULE g_hUiImplLib = NULL;
extern int g_isMac;

char* getUIDLLName(BOOL bForceWinName)
{
	if (( g_isMac ) && ( ! bForceWinName ))
	{
		return "CohUpdater_UI.dll";
	}
	else
	{
		return "CohUpdater_UI_Win.dll";
	}
}

BOOL InitImports(void)
{
	BOOL bResult = FALSE;

	// load the library (will be translated to dylib by cider if on Mac)
	g_hUiImplLib = LoadLibrary(getUIDLLName(FALSE));
	
	if ( g_hUiImplLib != NULL )
	{
		// check version
		int (__cdecl *fnGetVersion)() = (int (__cdecl *)())GetProcAddress(g_hUiImplLib,"UpdaterUI_GetVersion");
		if ( fnGetVersion )
		{
			if ( fnGetVersion() == UPDATER_UI_VERSION )
				bResult = TRUE;
		}
	}

	return bResult;
}

void ReportMismatchError()
{
	MessageBox(NULL,"UI library was not loaded properly! Please re-install the application","Patcher Error",MB_ICONERROR);
	ExitProcess(1);
}

BOOL CheckDLL()
{
	if ( g_hUiImplLib == NULL )
	{
		ReportMismatchError();
	}

	return TRUE;
}

// Initialization
int UpdaterUI_Init(void)
{
	if ( CheckDLL() )
	{
		int (__cdecl *fnInit)() = (int (__cdecl *)())GetProcAddress(g_hUiImplLib,"UpdaterUI_Init");
		if ( fnInit )
		{
			return fnInit();
		}
		else
		{
			ReportMismatchError();
		}
	}

	return -1;
}

void UpdaterUI_Destroy(void)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnDestroy)() = (void (__cdecl *)())GetProcAddress(g_hUiImplLib,"UpdaterUI_Destroy");
		if ( fnDestroy )
		{
			fnDestroy();
		}
		else
		{
			ReportMismatchError();
		}
	}
}

// Project name - this can be CoH or CoV. Determines what icon we use
void UpdaterUI_SetProjectName(const char* szProjName)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetProjName)(const char*) = (void (__cdecl *)(const char*))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetProjectName");
		if ( fnSetProjName )
		{
			fnSetProjName(szProjName);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_SetCurLangID(int nID)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetCurLangID)(int) = (void (__cdecl *)(int))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetCurLangID");
		if ( fnSetCurLangID )
		{
			fnSetCurLangID(nID);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

// Window creation
int  UpdaterUI_DialogCreate(void)
{
	if ( CheckDLL() )
	{
		int (__cdecl *fnDialogCreate)(void) = (int (__cdecl *)(void))GetProcAddress(g_hUiImplLib,"UpdaterUI_DialogCreate");
		if ( fnDialogCreate )
		{
			return fnDialogCreate();
		}
		else
		{
			ReportMismatchError();
		}
	}

	return -1;
}

void UpdaterUI_DialogDestroy(void)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnDialogDestroy)(void) = (void (__cdecl *)(void))GetProcAddress(g_hUiImplLib,"UpdaterUI_DialogDestroy");
		if ( fnDialogDestroy )
		{
			fnDialogDestroy();
		}
		else
		{
			ReportMismatchError();
		}
	}
}

// Text setting
void UpdaterUI_DialogSetLocalizedTextW(UpdaterTextArea eArea, const unsigned short* wcsText, int nLength)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnDialogSetLocalizedTextW)(UpdaterTextArea, const unsigned short*, int) = 
			(void (__cdecl *)(UpdaterTextArea, const unsigned short*, int))GetProcAddress(g_hUiImplLib,"UpdaterUI_DialogSetLocalizedTextW");
		if ( fnDialogSetLocalizedTextW )
		{
			fnDialogSetLocalizedTextW(eArea, wcsText, nLength);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_DialogSetLocalizedTextA(UpdaterTextArea eArea, const char* wcsText, int nLength)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnDialogSetLocalizedTextA)(UpdaterTextArea, const char*, int) = 
			(void (__cdecl *)(UpdaterTextArea, const char*, int))GetProcAddress(g_hUiImplLib,"UpdaterUI_DialogSetLocalizedTextA");
		if ( fnDialogSetLocalizedTextA )
		{
			fnDialogSetLocalizedTextA(eArea, wcsText, nLength);
		}
		else
		{
			ReportMismatchError();
		}
	}
}


void UpdaterUI_AddLanguage(const char* szLanguage, int nID)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnAddLanguage)(const char*, int) = 
			(void (__cdecl *)(const char*, int))GetProcAddress(g_hUiImplLib,"UpdaterUI_AddLanguage");
		if ( fnAddLanguage )
		{
			fnAddLanguage(szLanguage, nID);
		}
		else
		{
			ReportMismatchError();
		}
	}
}


// Element control
void UpdaterUI_URButtonDisplay(int bShow)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnURButtonDisplay)(int) = 
			(void (__cdecl *)(int))GetProcAddress(g_hUiImplLib,"UpdaterUI_URButtonDisplay");
		if ( fnURButtonDisplay )
		{
			fnURButtonDisplay(bShow);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_LRButtonDisplay(int bShow)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnLRButtonDisplay)(int) = 
			(void (__cdecl *)(int))GetProcAddress(g_hUiImplLib,"UpdaterUI_LRButtonDisplay");
		if ( fnLRButtonDisplay )
		{
			fnLRButtonDisplay(bShow);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_LLButtonDisplay(int bShow)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnLLButtonDisplay)(int) = 
			(void (__cdecl *)(int))GetProcAddress(g_hUiImplLib,"UpdaterUI_LLButtonDisplay");
		if ( fnLLButtonDisplay )
		{
			fnLLButtonDisplay(bShow);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_CheckboxDisplay(int bShow)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnCheckboxDisplay)(int) = 
			(void (__cdecl *)(int))GetProcAddress(g_hUiImplLib,"UpdaterUI_CheckboxDisplay");
		if ( fnCheckboxDisplay )
		{
			fnCheckboxDisplay(bShow);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_ProgressBarDisplay(int bShow)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnProgressBarDisplay)(int) = 
			(void (__cdecl *)(int))GetProcAddress(g_hUiImplLib,"UpdaterUI_ProgressBarDisplay");
		if ( fnProgressBarDisplay )
		{
			fnProgressBarDisplay(bShow);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_ShowLanguageButton(int bShow)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnShowLanguageButton)(int) = 
			(void (__cdecl *)(int))GetProcAddress(g_hUiImplLib,"UpdaterUI_ShowLanguageButton");
		if ( fnShowLanguageButton )
		{
			fnShowLanguageButton(bShow);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_SetProgressBarTotal(float fValue)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetProgressBarTotal)(float) = 
			(void (__cdecl *)(float))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetProgressBarTotal");
		if ( fnSetProgressBarTotal )
		{
			fnSetProgressBarTotal(fValue);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_SetProgressBar(float fValue)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetProgressBar)(float) = 
			(void (__cdecl *)(float))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetProgressBar");
		if ( fnSetProgressBar )
		{
			fnSetProgressBar(fValue);
		}
		else
		{
			ReportMismatchError();
		}
	}

}


// Callback setting - Lower Right, Upper Right, and Lower Left
void UpdaterUI_SetLRBtnCallback(UpdaterUI_GenericFn fnLRBtnCB)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetLRBtnCallback)(UpdaterUI_GenericFn) = 
			(void (__cdecl *)(UpdaterUI_GenericFn))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetLRBtnCallback");
		if ( fnSetLRBtnCallback )
		{
			fnSetLRBtnCallback(fnLRBtnCB);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_SetURBtnCallback(UpdaterUI_GenericFn fnURBtnCB)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetURBtnCallback)(UpdaterUI_GenericFn) = 
			(void (__cdecl *)(UpdaterUI_GenericFn))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetURBtnCallback");
		if ( fnSetURBtnCallback )
		{
			fnSetURBtnCallback(fnURBtnCB);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_SetLLBtnCallback(UpdaterUI_GenericFn fnLLBtnCB)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetLLBtnCallback)(UpdaterUI_GenericFn) = 
			(void (__cdecl *)(UpdaterUI_GenericFn))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetLLBtnCallback");
		if ( fnSetLLBtnCallback )
		{
			fnSetLLBtnCallback(fnLLBtnCB);
		}
		else
		{
			ReportMismatchError();
		}
	}
}


void UpdaterUI_SetCloseCallback(UpdaterUI_GenericFn fnCloseCB)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetCloseCallback)(UpdaterUI_GenericFn) = 
			(void (__cdecl *)(UpdaterUI_GenericFn))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetCloseCallback");
		if ( fnSetCloseCallback )
		{
			fnSetCloseCallback(fnCloseCB);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_SetMinimizeCallback(UpdaterUI_GenericFn fnMinCB)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetMinimizeCallback)(UpdaterUI_GenericFn) = 
			(void (__cdecl *)(UpdaterUI_GenericFn))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetMinimizeCallback");
		if ( fnSetMinimizeCallback )
		{
			fnSetMinimizeCallback(fnMinCB);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

void UpdaterUI_SetRestoreCallback(UpdaterUI_GenericFn fnRestoreCB)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetRestoreCallback)(UpdaterUI_GenericFn) = 
			(void (__cdecl *)(UpdaterUI_GenericFn))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetRestoreCallback");
		if ( fnSetRestoreCallback )
		{
			fnSetRestoreCallback(fnRestoreCB);
		}
		else
		{
			ReportMismatchError();
		}
	}
}


void UpdaterUI_SetLangCallback(UpdaterUI_GenericFn fnLangCB)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetLangCallback)(UpdaterUI_GenericFn) = 
			(void (__cdecl *)(UpdaterUI_GenericFn))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetLangCallback");
		if ( fnSetLangCallback )
		{
			fnSetLangCallback(fnLangCB);
		}
		else
		{
			ReportMismatchError();
		}
	}
}


// Getting information
int UpdaterUI_GetCheckboxState()
{
	if ( CheckDLL() )
	{
		int (__cdecl *fnGetCheckboxState)() = 
			(int (__cdecl *)())GetProcAddress(g_hUiImplLib,"UpdaterUI_GetCheckboxState");
		if ( fnGetCheckboxState )
		{
			return fnGetCheckboxState();
		}
		else
		{
			ReportMismatchError();
		}
	}

	return -1;
}

int UpdaterUI_GetSelectedLanguageID()
{
	if ( CheckDLL() )
	{
		int (__cdecl *fnGetSelectedLanguageID)() = 
			(int (__cdecl *)())GetProcAddress(g_hUiImplLib,"UpdaterUI_GetSelectedLanguageID");
		if ( fnGetSelectedLanguageID )
		{
			return fnGetSelectedLanguageID();
		}
		else
		{
			ReportMismatchError();
		}
	}

	return -1;
}


// Installer - these will be cheerfully ignored by the Mac version
void UpdaterUI_SetInstallDir(const char* szDir)
{
	if ( CheckDLL() )
	{
		void (__cdecl *fnSetInstallDir)(const char*) = 
			(void (__cdecl *)(const char*))GetProcAddress(g_hUiImplLib,"UpdaterUI_SetInstallDir");
		if ( fnSetInstallDir )
		{
			fnSetInstallDir(szDir);
		}
		else
		{
			ReportMismatchError();
		}
	}
}

const char* UpdaterUI_ChooseInstallDir(const char* szDialogTitle)
{
	if ( CheckDLL() )
	{
		const char* (__cdecl *fnChooseInstallDir)(const char*) = 
			(const char* (__cdecl *)(const char*))GetProcAddress(g_hUiImplLib,"UpdaterUI_ChooseInstallDir");
		if ( fnChooseInstallDir )
		{
			return fnChooseInstallDir(szDialogTitle);
		}
		else
		{
			ReportMismatchError();
		}
	}

	return NULL;
}
