#include <conio.h>
#include "patchserver.h"
#include "projects.h"
#include "comm_patcher.h"
#include "utils.h"
#include "zlib.h"
#include <assert.h>
#include "error.h"
#include "netcomp.h"
#include "EArray.h"
#include "timing.h"
#include "net_socket.h"
#include "MemoryMonitor.h"
#include "wininclude.h"
#include "sock.h"
#include <process.h>
#include <stdio.h>
#include "estring.h"

static bool g_listenThreadRunning=false;
static bool g_listenThreadStarting=false;

typedef struct PatcherListenerStatus
{
	char *serverVersion;
	char *clientVersion;
	int num_users;
	__int64 total_sent;
	int bytes_per_sec;
} PatcherListenerStatus;

PatcherListenerStatus g_listenerStatus = {0};
CRITICAL_SECTION listenerStatus_cs;

void updateListenerStatus(char *serverVersion, char *clientVersion, int users, __int64 totalSent, int bytesPerSec)
{
	EnterCriticalSection(&listenerStatus_cs);

    g_listenerStatus.serverVersion = estrCloneCharString(serverVersion);
    g_listenerStatus.clientVersion = estrCloneCharString(clientVersion);
	g_listenerStatus.num_users = users;
	g_listenerStatus.total_sent = totalSent;
	g_listenerStatus.bytes_per_sec = bytesPerSec;

	LeaveCriticalSection(&listenerStatus_cs);
}

DWORD WINAPI patchserverListenThreadMain(void *data) 
{
	struct sockaddr_in	addr_in;
	int port=DEFAULT_PATCHSERVER_LISTEN_PORT;
	int result;
	char *outStr = NULL;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	SOCKET s2;
	g_listenThreadRunning=true;

	sockSetAddr(&addr_in,htonl(INADDR_ANY),port);
	if (!sockBind(s,&addr_in)) {
		g_listenThreadRunning=false;
		g_listenThreadStarting=false;
		// Error binding to port!
		MessageBox(NULL, "Error binding to port, perhaps another UpdateServer is running?\r\nThis UpdateServer will not provide status updates.", "Error binding to port", MB_OK|MB_ICONERROR);
		return 0;
	}
	g_listenThreadStarting=false;
	result = listen(s, 15);
	if(result)
	{
		// Intentionally left this running so we don't get a large number of threads spiraling out of control
		MessageBox(NULL, "Error returned from listen(), SNMP-like stuff is now disabled", "Error", MB_OK|MB_ICONERROR);
		return 0;
	}
	do {
		LINGER lingerval;
		int totalMB;
		lingerval.l_onoff = 1; 
		lingerval.l_linger =15; 

		s2 = accept(s, NULL, NULL);
		if (s2==INVALID_SOCKET)
			continue;
		setsockopt(s2, SOL_SOCKET, SO_LINGER, (const char*)(&lingerval), sizeof(lingerval));

		estrClear(&outStr);

		EnterCriticalSection(&listenerStatus_cs);
#if 0 // old way with xml
		estrConcatf(&outStr, "<updateserver>");
			estrConcatf(&outStr, "<serverversion>%s</serverversion>", g_listenerStatus.serverVersion ? g_listenerStatus.serverVersion : "");
			estrConcatf(&outStr, "<clientversion>%s</clientversion>", g_listenerStatus.clientVersion ? g_listenerStatus.clientVersion : "");
			estrConcatf(&outStr, "<stats>");
				estrConcatf(&outStr, "<users>%d</users>", g_listenerStatus.num_users);
				estrConcatf(&outStr, "<total>%d</total>", g_listenerStatus.total_sent);
				estrConcatf(&outStr, "<throughput>%d</throughput>", g_listenerStatus.bytes_per_sec);
			estrConcatf(&outStr, "</stats>");
		estrConcatf(&outStr, "</updateserver>");
#else // new way with name/data pairs for cacti format
		estrConcatf(&outStr, "serverversion:%s ", g_listenerStatus.serverVersion ? g_listenerStatus.serverVersion : "n/a");
		estrConcatf(&outStr, "clientversion:%s ", g_listenerStatus.clientVersion ? g_listenerStatus.clientVersion : "n/a");
		estrConcatf(&outStr, "users:%d ", g_listenerStatus.num_users);
		totalMB = g_listenerStatus.total_sent / (1024*1024);
		estrConcatf(&outStr, "total:%d ", totalMB);
		estrConcatf(&outStr, "throughput:%d ", g_listenerStatus.bytes_per_sec);
		
#endif
		LeaveCriticalSection(&listenerStatus_cs);

		send(s2, outStr, strlen(outStr), 0);
		shutdown(s2, SD_BOTH);
		closesocket(s2);
	} while (1);
	g_listenThreadRunning = false;
	return 0;
}

void patchServerListenThreadBegin(void) 
{
	int ret;
	if (g_listenThreadRunning) {
		//printf("Thread already running\n");
		return;
	}
	InitializeCriticalSection(&listenerStatus_cs);
	sockStart();

	g_listenThreadRunning = true;
	g_listenThreadStarting = true;
	ret = _beginthreadex(NULL,0,patchserverListenThreadMain, NULL, 0, NULL);
	while (g_listenThreadStarting)
		Sleep(1);
}
