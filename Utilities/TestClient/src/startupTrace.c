#include "game.h"
#include <windows.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

static HANDLE s_startupTraceFile = INVALID_HANDLE_VALUE;
static char s_startupTracePath[MAX_PATH];

static const char *game_startupTracePrefix(void)
{
    static char prefix[32];
    static int initialized;
    char module[MAX_PATH];
    char *name;

    if (initialized)
        return prefix;

    initialized = 1;
    strcpy_s(prefix, sizeof(prefix), "testclient");
    if (GetModuleFileNameA(NULL, module, (DWORD)sizeof(module)))
    {
        name = strrchr(module, '\\');
        name = name ? name + 1 : module;
        if (_stricmp(name, "TestClient.exe") == 0)
            strcpy_s(prefix, sizeof(prefix), "testclient");
        else if (_stricmp(name, "Ouroboros.exe") == 0)
            strcpy_s(prefix, sizeof(prefix), "ouroboros");
    }

    return prefix;
}

void game_startupTrace(const char *marker)
{
    SYSTEMTIME now;
    char line[768];
    DWORD written;

    if (!marker || !marker[0])
        marker = "empty";

    if (s_startupTraceFile == INVALID_HANDLE_VALUE)
    {
        CreateDirectoryA("logs", NULL);
        sprintf_s(s_startupTracePath, sizeof(s_startupTracePath),
                  "logs/%s-startup-%lu.trace", game_startupTracePrefix(),
                  (unsigned long)GetCurrentProcessId());
        s_startupTraceFile = CreateFileA(s_startupTracePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (s_startupTraceFile != INVALID_HANDLE_VALUE)
            SetFilePointer(s_startupTraceFile, 0, NULL, FILE_END);
    }

    GetLocalTime(&now);
    sprintf_s(line, sizeof(line),
              "%04u-%02u-%02uT%02u:%02u:%02u.%03u pid=%lu tid=%lu marker=%s\n",
              (unsigned)now.wYear, (unsigned)now.wMonth, (unsigned)now.wDay,
              (unsigned)now.wHour, (unsigned)now.wMinute, (unsigned)now.wSecond,
              (unsigned)now.wMilliseconds, (unsigned long)GetCurrentProcessId(),
              (unsigned long)GetCurrentThreadId(), marker);

    if (s_startupTraceFile != INVALID_HANDLE_VALUE)
    {
        WriteFile(s_startupTraceFile, line, (DWORD)strlen(line), &written, NULL);
        FlushFileBuffers(s_startupTraceFile);
    }

    OutputDebugStringA(line);
}

void game_startupTracef(const char *format, ...)
{
    char marker[512];
    va_list args;

    va_start(args, format);
    vsprintf_s(marker, sizeof(marker), format, args);
    va_end(args);
    game_startupTrace(marker);
}
