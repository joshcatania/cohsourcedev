/* Provides simplified way to call corresponding functions in CrashRpt.dll. 
 *
 */

#ifndef STACKDUMP_H
#define STACKDUMP_H

C_DECLARATIONS_BEGIN

typedef struct _CONTEXT *PCONTEXT;

int sdReady(void);
int sdStartup(void);
void sdShutdown(void);
void sdDumpStack(void);
void sdDumpStackToBuffer(char* buffer, int bufferSize, PCONTEXT stack);

C_DECLARATIONS_END

#endif