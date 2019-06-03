#ifndef DBSERVER_CONTAINERDBPOSTGRESQL_H
#define DBSERVER_CONTAINERDBPOSTGRESQL_H

#include "ContainerDb.hpp"

class ContainerDbPostgresql : public ContainerDb {

public:

	ContainerDbPostgresql();
	~ContainerDbPostgresql();

	void beforeSQLMetaCalls(std::string& table);
	ContainerFieldInfo& getContainerFieldInfo(ContainerFieldType type);
	std::string restartContainerSequence(const std::string& table);
	std::string deleteFromContainer(const std::string& table, const std::string& joinsToTable, const std::string& nullField);
	std::string dropIndexIfExists(const std::string& indexName, const std::string& table);
	std::string createIndexIfNotExists(const std::string& indexName, const std::string& table, const std::string& columns);
	std::string dropForeignKeyConstraintIfExists(const std::string& table, const std::string& key, const std::string& foreignTable);
	std::string createForeignKeyConstraintIfNotExists(const std::string& table, const std::string& key, const std::string& foreignTable);
	std::string createContainerTableQuery(const std::string& table);
	std::string select(const std::string& schema, const std::string& table, const std::string& columns, const std::string& limit, const std::string& where);
	bool isUnbound(ContainerFieldType cfType, const std::string& sqlTypeName, int columnSize);
	std::string alterColumnType(const std::string& table, const std::string& column, const std::string& newType);
	int insertEmptyContainerRow(std::ostringstream& ss, const std::string& table);
};

#endif