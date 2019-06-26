#ifndef _DBLOG_H
#define _DBLOG_H

#include <utilitieslib/network/netio.h>

typedef struct MemLog MemLog;

#ifdef __cplusplus
extern "C" {
#endif

void dbMemLog(const char* func, const char* s, ...);
void dbMemLogEcho(const char* s, ...);

#ifdef __cplusplus
}
#endif

#endif
