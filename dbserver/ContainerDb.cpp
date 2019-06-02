#include "ContainerDb.hpp"
#include <sstream>

ContainerDb::ContainerDb() {
	// Default type mappings
	ormTypeToContainerFieldTypeMap["attribute"] = CFTYPE_INT;
	ormTypeToContainerFieldTypeMap["int1"] = CFTYPE_BYTE;
	ormTypeToContainerFieldTypeMap["int2"] = CFTYPE_SHORT;
	ormTypeToContainerFieldTypeMap["int4"] = CFTYPE_INT;
	ormTypeToContainerFieldTypeMap["float4"] = CFTYPE_FLOAT;
	ormTypeToContainerFieldTypeMap["unicodestring"] = CFTYPE_STRING;
	ormTypeToContainerFieldTypeMap["ansistring"] = CFTYPE_STRING;
	ormTypeToContainerFieldTypeMap["datetime"] = CFTYPE_DATETIME;
	ormTypeToContainerFieldTypeMap["binary(max)"] = CFTYPE_BINARY;
	ormTypeToContainerFieldTypeMap["unicodestring(max)"] = CFTYPE_STRING;
	ormTypeToContainerFieldTypeMap["ansistring(max)"] = CFTYPE_STRING;
	ormTypeToContainerFieldTypeMap["textblob"] = CFTYPE_STRING;

	sqlTypeToContainerFieldTypeMap[SQL_UNKNOWN_TYPE] = CFTYPE_NULL;
	sqlTypeToContainerFieldTypeMap[SQL_TINYINT] = CFTYPE_BYTE;
	sqlTypeToContainerFieldTypeMap[SQL_SMALLINT] = CFTYPE_SHORT;
	sqlTypeToContainerFieldTypeMap[SQL_INTEGER] = CFTYPE_INT;
	sqlTypeToContainerFieldTypeMap[SQL_REAL] = CFTYPE_FLOAT;
	sqlTypeToContainerFieldTypeMap[SQL_VARCHAR] = CFTYPE_STRING;
	sqlTypeToContainerFieldTypeMap[SQL_TYPE_TIMESTAMP] = CFTYPE_DATETIME;
	sqlTypeToContainerFieldTypeMap[SQL_VARBINARY] = CFTYPE_BINARY;
	sqlTypeToContainerFieldTypeMap[SQL_LONGVARCHAR] = CFTYPE_STRING;
	sqlTypeToContainerFieldTypeMap[SQL_LONGVARBINARY] = CFTYPE_BINARY;
}

ContainerFieldType ContainerDb::ormTypeToContainerFieldType(const std::string& ormType) {
	auto it = ormTypeToContainerFieldTypeMap.find(ormType);
	return (it == ormTypeToContainerFieldTypeMap.end()) ? CFTYPE_NULL : it->second;
}

std::string ContainerDb::truncateTempTableAndInsertInto(const std::string& table, const std::string& columns, bool hasSequence) {
	std::ostringstream ss;
	ss << "TRUNCATE TABLE dbo." << table << "_tmp; "
		"INSERT INTO dbo." << table << "_tmp(" << columns << ") "
		"SELECT " << columns << " FROM dbo." << table << ';';
	return ss.str();
}

std::string ContainerDb::insertIntoTableFromTemp(const std::string& table, const std::string& columns, bool hasSequence) {
	std::ostringstream ss;
	ss << "INSERT INTO dbo." << table << '(' << columns << ") "
		<< "SELECT " << columns << " FROM dbo." << table << "_tmp;";
	return ss.str();
}

ContainerFieldType ContainerDb::findContainerFieldType(SQLSMALLINT dataType) {
	auto it = sqlTypeToContainerFieldTypeMap.find(dataType);
	return (it == sqlTypeToContainerFieldTypeMap.end()) ? CFTYPE_NULL : it->second;
}