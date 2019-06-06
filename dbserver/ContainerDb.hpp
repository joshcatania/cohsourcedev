#ifndef DBSERVER_CONTAINERDB_HPP
#define DBSERVER_CONTAINERDB_HPP

#include "container_tplt_utils.h"

#include "sql/sqlinclude.h"
//#include "container_field.h"
#include <map>
#include <errno.h>
#include <string>

// For integrating CoXs pseudo template/attributes ORM along with separating out any database vendor
// specific logic and queries in the DbServer.
class ContainerDb {

public:
	ContainerDb();
	virtual ~ContainerDb() {}

	
	// Called before any meta SQL functions, like SQLColumns, SQLTables.
	// Currently only used for PostgreSQL, which needs the schema and table names to be lowercased
	// for meta functions, because all of that is actually stored in lowercase behind the scenes.
	virtual void beforeSQLMetaCalls(std::string& table);

	// Finds the corresponding ContainerFieldType to match the ORM type read from template and attribute files.
	// For a list of the ORM types, see \Source\bin\data\server\templates\TestDatabaseTypes.template.
	// Note: ORM types aren't necessarily the same data types between databases. E.g. PostgreSQL matches
	// int1 to short, whereas SQL Server matches it to byte.
	virtual ContainerFieldType ormTypeToContainerFieldType(const std::string& ormType);
	virtual ContainerFieldInfo& getContainerFieldInfo(ContainerFieldType type) = 0;

	// SQL query for restarting the container id sequence of a Container table
	virtual std::string restartContainerSequence(const std::string& table) = 0;

	// Empties a "temporary" table with the naming schema table_tmp and inserts from the real table.
	// This is not a real temporary table and the table is not created in this query.
	virtual std::string truncateTempTableAndInsertInto(const std::string& table, const std::string& columns, bool hasSequence);
	
	// After recreating a new table, gets the data from the previous table moved to a temp table in 
	// truncateTempTableAndInsertInto and moves it back into the new, real table.
	virtual std::string insertIntoTableFromTemp(const std::string& table, const std::string& columns, bool hasSequence);

	//
	// nullField: 
	virtual std::string deleteFromContainer(const std::string& table, const std::string& joinsToTable, const std::string& nullField) = 0;
	
	// 
	virtual std::string dropIndexIfExists(const std::string& indexName, const std::string& table) = 0;
	
	//
	virtual std::string createIndexIfNotExists(const std::string& indexName, const std::string& table, const std::string& columns) = 0;

	//
	virtual std::string dropForeignKeyConstraintIfExists(const std::string& table, const std::string& key, const std::string& foreignTable) = 0;

	// SQL query for creating a foreign key constraint.
	// The query must handle the case in which the foreign table hasn't been created yet.
	virtual std::string createForeignKeyConstraintIfNotExists(const std::string& table, const std::string& key, const std::string& foreignTable) = 0;

	// Returns SQL query for altering a column type. Nothing is checked or handled here to ensure that a column can be changed
	// to a column of the new type.
	virtual std::string alterColumnType(const std::string& table, const std::string& column, const std::string& newType) = 0;
	
	// SQL query for creating the minimum columns for a Container table, those being "ContainerId" and "Active".
	// The rest of the columns are added later. 
	virtual std::string createContainerTableQuery(const std::string& table) = 0;

	// Maps an SQL type to a ContainerFieldType. The database should never pass back a type that
	// doesn't properly map to the ContainerFieldType.
	virtual ContainerFieldType sqlTypeToContainerFieldType(SQLSMALLINT dataType);

	// Generates SQL Select statement.
	// columns is a comma separated list of columns to retrieve.
	// WHERE string must include WHERE
	// LIMIT must include clause which may be database specific
	virtual std::string select(const std::string& schema, const std::string& table, const std::string& columns, const std::string& limit, const std::string& where) = 0;
	
	// Determines whether an SQL type is unbound, where unbound means a varying length field with no set columns size.
	// A column with a separate size constraint will still read as unbound.
	//
	// This is only ever relevant initially when reading the table structures. After that is done, the definition
	// of unbound in ColumnInfo is varying characters or binary with num_bytes as -1.
	virtual bool isUnbound(ContainerFieldType cfType, const std::string& sqlTypeName, int columnSize) = 0;

	// Appends query(s) to the stringstream for inserting a row to the Container table with only the container id.
	// The container id is a column with a sequence, so the sequence must be then reset to the container id value.
	// (which would increment on the next insert without the container id value).
	// Returns the number of binds to the container id needed.
	virtual int insertEmptyContainerRow(std::ostringstream& ss, const std::string& table) = 0;

	virtual std::string nowMinusDays(int days) = 0;

	// Returns the next value that will be inserted in a column with a sequence/serial set.
	// I don't think this should be used. Instead, without a sequence value set and let the database decide.
	// Only here for legacy purposes.
	// This query DOES NOT return the current highest sequence value.
	virtual std::string nextInsertedSequenceValueQuery(const std::string& table, const std::string& column) = 0;
	

protected:
	std::map<std::string, ContainerFieldType> ormTypeToContainerFieldTypeMap;
	std::map<int, ContainerFieldType> sqlTypeToContainerFieldTypeMap;

	// Schema for foreign key names. Default is FK_table_key_foreignTable
	virtual std::string foreignKeyNameSchema(const std::string& table, const std::string& key, const std::string& foreignTable);
};



#endif