#include <windows.h>
#include "osdependent.h"

int g_isUsingWin2korXP = 1;

void InitOSInfo(void)
{
	OSVERSIONINFO os_version_info;

	os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx( &os_version_info );

	if ( os_version_info.dwPlatformId == VER_PLATFORM_WIN32_NT )
		g_isUsingWin2korXP = 1;
	else
		g_isUsingWin2korXP = 0;
}