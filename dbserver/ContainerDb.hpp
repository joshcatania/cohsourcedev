#ifndef DBSERVER_CONTAINERDB_HPP
#define DBSERVER_CONTAINERDB_HPP

#include "container_tplt_utils.h"
#include "sql/sqlinclude.h"
#include <map>
#include <errno.h>
#include <string>

class ContainerDb {

public:
	ContainerDb();
	virtual ~ContainerDb() {}

	ContainerFieldType ormTypeToContainerFieldType(const std::string& ormType);
	virtual ContainerFieldInfo getContainerFieldInfo(ContainerFieldType type) = 0;
	virtual std::string restartContainerSequence(const std::string& table) = 0;
	std::string truncateTempTableAndInsertInto(const std::string& table, const std::string& columns, bool hasSequence);
	std::string insertIntoTableFromTemp(const std::string& table, const std::string& columns, bool hasSequence);
	virtual std::string deleteFromContainer(const std::string& table, const std::string& joinsToTable, const std::string& nullField) = 0;
	virtual std::string dropIndexIfExists(const std::string& indexName, const std::string& table) = 0;
	virtual std::string createIndexIfNotExists(const std::string& indexName, const std::string& table, const std::string& columns) = 0;
	virtual std::string dropForeignKeyConstraintIfExists(const std::string& table, const std::string& key, const std::string& foreignTable) = 0;
	virtual std::string createForeignKeyConstraintIfNotExists(const std::string& table, const std::string& key, const std::string& foreignTable) = 0;
	virtual std::string alterColumnType(const std::string& table, const std::string& column, const std::string& newType) = 0;
	virtual std::string createContainerTableQuery(const std::string& table) = 0;
	ContainerFieldType findContainerFieldType(SQLSMALLINT dataType);
	virtual std::string select(const std::string& schema, const std::string& table, const std::string& columns, const std::string& limit, const std::string& where) = 0;
	virtual bool isUnbound(ContainerFieldType cfType, const std::string& sqlTypeName, int columnSize) = 0;

protected:
	std::map<std::string, ContainerFieldType> ormTypeToContainerFieldTypeMap;
	std::map<int, ContainerFieldType> sqlTypeToContainerFieldTypeMap;

};



#endif