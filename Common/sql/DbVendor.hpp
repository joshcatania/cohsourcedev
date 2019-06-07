#ifndef COMMON_DBVENDOR_HPP
#define COMMON_DBVENDOR_HPP

C_DECLARATIONS_BEGIN

typedef struct DatabaseConfig {
	char sqlDbProvider[1024];
	char sqlLogin[1024];
	char sqlDbName[1024];
} DatabaseConfig;

typedef enum DatabaseProvider {
	DBPROV_UNKNOWN = 0,
	DBPROV_MSSQL,
	DBPROV_POSTGRESQL
} DatabaseProvider;

extern DatabaseProvider gDatabaseProvider;

C_DECLARATIONS_END

#ifdef __cplusplus

#include <errno.h>
#include <string>

// For vendor specific database functions
// ChatServer is very db agnostic to begin with so instead of separating this into separate
// classes like the other servers, the functions are being put under a namespace with
// the DatabaseProvider global parameter exposed and used to distinguish db vendors.
namespace DbVendor {
	extern void transformCallStoredProcedure(std::string& proc);
	extern DatabaseProvider fromConfigString(const std::string& config);
}

#endif

#endif