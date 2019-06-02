#ifndef  DBSERVER_GLOBALS_H
#define  DBSERVER_GLOBALS_H

#ifdef __cplusplus
#include "ContainerDb.hpp"
#endif

C_DECLARATIONS_BEGIN

extern DatabaseProvider gDatabaseProvider;

C_DECLARATIONS_END

#ifdef __cplusplus
extern ContainerDb* gContainerDb;
#endif

#endif