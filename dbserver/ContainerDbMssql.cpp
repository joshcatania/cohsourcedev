#include "ContainerDbMssql.hpp"
#include "sql/sqlinclude.h"
#include <sstream>

const int CTRMSSQL_FIELDINFOCOUNT = 8;

static ContainerFieldInfo cToSqlMappingsSqlServer[CTRMSSQL_FIELDINFOCOUNT] = {
	{ 0,	SQL_C_DEFAULT,			SQL_UNKNOWN_TYPE,	SQL_UNKNOWN_TYPE,	NULL,		NULL			}, // CFTYPE_NULL
	{ 1,	SQL_C_TINYINT,			SQL_TINYINT,		SQL_UNKNOWN_TYPE,	"tinyint",	NULL		}, // CFTYPE_BYTE
	{ 2,	SQL_C_SHORT,			SQL_SMALLINT,		SQL_UNKNOWN_TYPE,	"smallint",	NULL		}, // CFTYPE_SHORT
	{ 4,	SQL_C_LONG,				SQL_INTEGER,		SQL_UNKNOWN_TYPE,	"int",		NULL			}, // CFTYPE_INT
	{ 4,	SQL_C_FLOAT,			SQL_REAL,			SQL_UNKNOWN_TYPE,	"real",		NULL			}, // CFTYPE_FLOAT
	{ 4,	SQL_C_CHAR,				SQL_VARCHAR,		SQL_LONGVARCHAR, 	"varchar",	"varchar(max)"	}, // CFTYPE_STRING
	{ 16,	SQL_C_TYPE_TIMESTAMP,	SQL_TYPE_TIMESTAMP,	SQL_UNKNOWN_TYPE,	"datetime",	NULL		}, // CFTYPE_DATETIME
	{ 1,	SQL_C_BINARY,			SQL_VARBINARY,		SQL_LONGVARBINARY,	"varbinary", "varbinary(max)"} // CFTYPE_BINARY
};

ContainerDbMssql::ContainerDbMssql() {}

ContainerDbMssql::~ContainerDbMssql() {}

ContainerFieldInfo ContainerDbMssql::getContainerFieldInfo(ContainerFieldType type) {
	return cToSqlMappingsSqlServer[type];
}

std::string ContainerDbMssql::restartContainerSequence(const std::string& table) {
	return "DBCC CHECKIDENT('" + table + "', RESEED, 0) WITH NO_INFOMSGS;";
}

std::string ContainerDbMssql::truncateTempTableAndInsertInto(const std::string& table, const std::string& columns, bool hasSequence) {
	if (!hasSequence)
		return ContainerDb::truncateTempTableAndInsertInto(table, columns, hasSequence);

	std::ostringstream ss;
	ss << "TRUNCATE TABLE dbo." << table << "_tmp; "
		"SET IDENTITY_INSERT dbo." << table << "_tmp ON; "
		"INSERT INTO dbo." << table << "_tmp(" << columns << ") "
		"SELECT " << columns << " FROM dbo." << table << "; "
		"SET IDENTITY_INSERT dbo." << table << "_tmp OFF;";
	return ss.str();
}

std::string ContainerDbMssql::insertIntoTableFromTemp(const std::string& table, const std::string& columns, bool hasSequence) {
	if (!hasSequence)
		return ContainerDb::insertIntoTableFromTemp(table, columns, hasSequence);

	std::ostringstream ss;
	ss << "SET IDENTITY_INSERT dbo." << table << " ON; "
		"INSERT INTO dbo." << table << '(' << columns << ") SELECT " << columns << " FROM dbo." << table << "_tmp; "
		"SET IDENTITY_INSERT dbo." << table << " OFF;";
	return ss.str();
}

std::string ContainerDbMssql::deleteFromContainer(const std::string& table, const std::string& joinsToTable, const std::string& nullField) {
	std::ostringstream ss;
	ss << "DELETE dbo." << table << " FROM dbo." << table << " "
		"INNER JOIN dbo." << joinsToTable << " ON " << joinsToTable << ".ContainerID = " <<	table << ".ContainerID";
	if (nullField != "")
		ss << " WHERE " << joinsToTable << '.' << nullField << " IS NULL";
	ss << ';';
	return ss.str();
}

std::string ContainerDbMssql::createContainerTableQuery(const std::string& table) {
	std::ostringstream ss;
	ss << "CREATE TABLE dbo." << table << " (ContainerId INTEGER NOT NULL IDENTITY(1,1) PRIMARY KEY, Active INTEGER);";
	return ss.str();
}

std::string ContainerDbMssql::select(const std::string& schema, const std::string& table, const std::string& columns, const std::string& limit, const std::string& where) {
	std::ostringstream ss;
	ss << "SELECT " << limit << " " << columns << " FROM " << schema << "." << table << " " << where << ';';
	return ss.str();
}

bool ContainerDbMssql::isUnbound(ContainerFieldType cfType, const std::string& sqlTypeName, int columnSize) {
	ContainerFieldInfo info = getContainerFieldInfo(cfType);
	if (info.db_unbound_type == NULL) {
		return false;
	}
	return columnSize == 0;
}

std::string ContainerDbMssql::dropIndexIfExists(const std::string& indexName, const std::string& table) {
	std::ostringstream ss;
	ss << "IF EXISTS (SELECT sys.indexes.name FROM sys.indexes JOIN sys.objects on sys.indexes.object_id=sys.objects.object_id "
			"WHERE sys.indexes.name = N'" << indexName << "' and sys.objects.name=N'" << table << "') "
		"DROP INDEX " << indexName << " ON dbo." << table << ';';
	return ss.str();
}

std::string ContainerDbMssql::createIndexIfNotExists(const std::string& indexName, const std::string& table, const std::string& columns) {
	std::ostringstream ss;
	ss << "IF NOT EXISTS (SELECT sys.indexes.name FROM sys.indexes JOIN sys.objects on sys.indexes.object_id=sys.objects.object_id "
		"WHERE sys.indexes.name = N'" << indexName << "'"
		"AND sys.objects.name=N'" << table << "') "
		"CREATE INDEX " << indexName << " ON dbo." << table << " (" << columns << ");";
	return ss.str();
}

std::string ContainerDbMssql::dropForeignKeyConstraintIfExists(const std::string& table, const std::string& key, const std::string& foreignTable) {
	std::ostringstream ss;
	ss << "IF EXISTS (SELECT constraint_name FROM information_schema.table_constraints "
		"WHERE constraint_name = 'FK_" << table << '_' << key << '_' << foreignTable << "') "
		"ALTER TABLE dbo." << table << " "
		"DROP CONSTRAINT FK_" << table << '_' << key << '_' << foreignTable << ';';
	return ss.str();
}

std::string ContainerDbMssql::createForeignKeyConstraintIfNotExists(const std::string& table, const std::string& key, const std::string& foreignTable)
{
	std::ostringstream ss;
	ss << "IF NOT EXISTS (SELECT constraint_name FROM information_schema.table_constraints "
		"WHERE constraint_name = 'FK_" << table << '_' << key << '_' << foreignTable << "') "
		"ALTER TABLE dbo." << table << " "
		"ADD CONSTRAINT FK_" << table << '_' << key << '_' << foreignTable << " "
		"FOREIGN KEY (" << key << ") REFERENCES " << foreignTable << ';';
	return ss.str();
}

std::string ContainerDbMssql::alterColumnType(const std::string& table, const std::string& column, const std::string& newType) {
	std::ostringstream ss;
	ss << "ALTER TABLE dbo." << table << " ALTER COLUMN " << column << ' ' << newType << ';';
	return ss.str();
}