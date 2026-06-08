#ifndef WININCLUDE_H
#define WININCLUDE_H

#include "../stdtypes.h"

#undef CreateThread
#pragma push_macro("R8")
#pragma push_macro("R9")
#pragma push_macro("R10")
#pragma push_macro("R11")
#pragma push_macro("R12")
#pragma push_macro("R13")
#pragma push_macro("R14")
#pragma push_macro("R15")
#undef R8
#undef R9
#undef R10
#undef R11
#undef R12
#undef R13
#undef R14
#undef R15
#include <winsock2.h>
#include <winbase.h>
#include <iphlpapi.h>
#pragma pop_macro("R15")
#pragma pop_macro("R14")
#pragma pop_macro("R13")
#pragma pop_macro("R12")
#pragma pop_macro("R11")
#pragma pop_macro("R10")
#pragma pop_macro("R9")
#pragma pop_macro("R8")
#define CreateThread include_utils_h_for_threads

#if __SAL_H_FULL_VER >= 140050727
    #define PLATFORMSDK 0x0600
#else
    #define PLATFORMSDK 0x0500
#endif

C_DECLARATIONS_BEGIN

void* tls_zero_malloc(size_t size);

#define WININCLUDE_PERFTYPE PerformanceInfo

// These are defined in MemAlloc.c.
#if _CRTDBG_MAP_ALLOC && !defined(MEMALLOC_C)
#    undef HeapAlloc
#    undef HeapCreate
#    undef HeapDestroy
#    undef HeapReAlloc
#    undef HeapFree

#    define HeapAlloc timed_HeapAlloc
#    define HeapCreate timed_HeapCreate
#    define HeapDestroy timed_HeapDestroy
#    define HeapReAlloc timed_HeapReAlloc
#    define HeapFree timed_HeapFree

    HANDLE    timed_HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize);
    BOOL    timed_HeapDestroy(HANDLE hHeap);
    LPVOID    timed_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
    LPVOID    timed_HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
    BOOL    timed_HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
#endif

// These are defined in timing.c.
#if defined(_DEBUG) && !defined(TIMING_C)
#    undef EnterCriticalSection
#    undef LeaveCriticalSection
#    undef Sleep

#    define EnterCriticalSection(x) {static WININCLUDE_PERFTYPE* piStatic;timed_EnterCriticalSection(x, #x, &piStatic);}
#    define LeaveCriticalSection(x) {static WININCLUDE_PERFTYPE* piStatic;timed_LeaveCriticalSection(x, #x, &piStatic);}
#    define Sleep(x) {static WININCLUDE_PERFTYPE* piStatic;timed_Sleep(x, "Sleep("#x")", &piStatic);}

    typedef struct WININCLUDE_PERFTYPE WININCLUDE_PERFTYPE;

    void timed_EnterCriticalSection(void* cs, const char* name, WININCLUDE_PERFTYPE** piStatic);
    void timed_LeaveCriticalSection(void* cs, const char* name, WININCLUDE_PERFTYPE** piStatic);
    void timed_Sleep(U32 ms, const char* name, WININCLUDE_PERFTYPE** piStatic);
#endif

C_DECLARATIONS_END

#pragma comment(lib,"Iphlpapi.lib")

#endif
