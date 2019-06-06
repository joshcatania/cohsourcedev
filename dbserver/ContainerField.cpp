#include "ContainerField.hpp"
#include "dbglobals.h"
#include "error.h"
#include <sstream>

ContainerFieldType dataType(char* str, int& outColumnSize, int& outNumBytes, std::string& outSqlTypeName) {
	int length = 0;

	char* type_name = strtok(str, "[");
	char* length_str = strtok(NULL, "]");

	ContainerFieldType type = gContainerDb->ormTypeToContainerFieldType(type_name);
	if (type == CFTYPE_NULL)
		FatalErrorf("No data type associated with orm type: %s\n", type_name);

	if (length_str)
		length = atoi(length_str);

	ContainerFieldInfo typeMapping = gContainerDb->getContainerFieldInfo(type);

	if (length == 0) {
		outColumnSize = 0;
		if (CFTYPE_IS_UNBOUNDABLE(type)) {
			outSqlTypeName = typeMapping.db_unbound_type;
			outNumBytes = -1;
		}
		else {
			outSqlTypeName = typeMapping.db_bound_type;
			outNumBytes = typeMapping.access_size;
		}
	}
	else {
		std::ostringstream ss;
		ss << typeMapping.db_bound_type << '(' << length << ')';
		outSqlTypeName = ss.str();
		outColumnSize = length;
		outNumBytes = length * typeMapping.access_size;
	}
	return type;
}