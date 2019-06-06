#ifndef _DBSERVER_CONTAINER_FIELD_H
#define _DBSERVER_CONTAINER_FIELD_H

#include <errno.h>
#ifdef __cplusplus
#include <string>
#endif

C_DECLARATIONS_BEGIN

typedef enum ContainerFieldType
{
	CFTYPE_NULL,

	// Unquoted types
	CFTYPE_BYTE,
	CFTYPE_SHORT,
	CFTYPE_INT,
	CFTYPE_FLOAT,

	// Quoted types
	CFTYPE_STRING,
	CFTYPE_WSTRING,
	CFTYPE_DATETIME,
	CFTYPE_DATETIME_TIMEZONE,
	CFTYPE_BINARY,

	CFTYPE_COUNT
} ContainerFieldType;

#define FIRST_QUOTED_TYPE CFTYPE_STRING
#define CFTYPE_IS_UNBOUNDABLE(_e) ((_e) == CFTYPE_STRING || (_e) == CFTYPE_BINARY || (_e) == CFTYPE_WSTRING)
#define CFTYPE_IS_BOUND(_e, _cs) ((_cs) > 0 || !CFTYPE_IS_UNBOUNDABLE(_e))

C_DECLARATIONS_END

#ifdef __cplusplus
ContainerFieldType dataType(char* str, int& outColumnSize, int& outNumBytes, std::string& outSqlTypeName);

typedef struct ContainerFieldInfo
{
	const int access_size;
	const int c_type;
	const int sql_type;
	const int sql_type_long;
	const std::string db_bound_type;
	const std::string db_unbound_type;
} ContainerFieldInfo;
#endif


#endif