#ifndef _PATCHUI_H
#define _PATCHUI_H

#include "stdtypes.h"

char *pickInstallDir(char *project_name,char *dir_name);
void startInstallDialog(void *hInstance,char *url);
void xferStatsInit(char *mode,U32 base,U32 total);
void xferStatsUpdate(U32 curr);
F32 xferStatsElapsed();
void patchShowWebPage(char *url);
void patchUiChangeCancelToPlay();
void xferStatsFinish(void);
void patchUiSetProjectInfo(char *project,char *version);
int winMsgOkCancel(char *str);

extern int g_isUsingWin2korXP;

// Call the correct SetDlgItemText function based on the user's OS
#define UPDATER_UI_SET_ITEM_TEXT( eItem, str ) \
	g_isUsingWin2korXP ? UpdaterUI_DialogSetLocalizedTextW( eItem, xlateToUnicode(str), wcslen(xlateToUnicode(str)) ) : UpdaterUI_DialogSetLocalizedTextA( eItem, str, strlen(str) ) 

// Call the correct SetWindowText function based on the user's OS
#define UPDATER_UI_SET_WINDOW_TEXT( str ) \
	g_isUsingWin2korXP ? UpdaterUI_DialogSetLocalizedTextW( UTA_WINDOW_TITLE, xlateToUnicode(str), wcslen(xlateToUnicode(str)) ) : UpdaterUI_DialogSetLocalizedTextA( UTA_WINDOW_TITLE, str, strlen(str) )

#endif
