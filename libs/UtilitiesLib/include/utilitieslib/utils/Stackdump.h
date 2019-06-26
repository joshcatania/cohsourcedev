/* Provides simplified way to call corresponding functions in CrashRpt.dll. 
 *
 */

#ifndef STACKDUMP_H
#define STACKDUMP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CONTEXT *PCONTEXT;

int sdReady(void);
int sdStartup(void);
void sdShutdown(void);
void sdDumpStack(void);
void sdDumpStackToBuffer(char* buffer, int bufferSize, PCONTEXT stack);

#ifdef __cplusplus
}
#endif

#endif