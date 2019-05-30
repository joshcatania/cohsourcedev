#ifndef _DBLOG_H
#define _DBLOG_H

#include "netio.h"

C_DECLARATIONS_BEGIN

typedef struct MemLog MemLog;
void dbMemLog(const char* func, const char* s, ...);
void dbMemLogEcho(const char* s, ...);

C_DECLARATIONS_END

#endif
