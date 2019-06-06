#include "ContainerDbPostgresql.hpp"
#include <sstream>
#include <algorithm>

static ContainerFieldInfo cToSqlMappingsPostgres[CFTYPE_COUNT] = {
	{ 0,	SQL_C_DEFAULT,			SQL_UNKNOWN_TYPE,	SQL_UNKNOWN_TYPE,	"",			""	}, // CFTYPE_NULL
	{ 2,	SQL_C_SHORT,			SQL_SMALLINT, 		SQL_UNKNOWN_TYPE,	"int2",			""	}, // CFTYPE_BYTE
	{ 2,	SQL_C_SHORT,			SQL_SMALLINT, 		SQL_UNKNOWN_TYPE,	"int2",			""	}, // CFTYPE_SHORT
	{ 4,	SQL_C_LONG,				SQL_INTEGER, 		SQL_UNKNOWN_TYPE,	"int4",			""	}, // CFTYPE_INT
	{ 4,	SQL_C_FLOAT,			SQL_REAL,			SQL_UNKNOWN_TYPE,	"float4",		""	}, // CFTYPE_FLOAT
	{ 4,	SQL_C_CHAR,				SQL_VARCHAR, 		SQL_LONGVARCHAR,	"varchar",		"text"	}, // CFTYPE_STRING
	{ 4,	SQL_C_CHAR,				SQL_VARCHAR,		SQL_LONGVARCHAR,	"varchar",		"text"	}, //CFTYPE_WSTRING, mapped to CFTYPE_STRING, this should never be accessed
	{ 19,	SQL_C_TYPE_TIMESTAMP,	SQL_TYPE_TIMESTAMP,	SQL_UNKNOWN_TYPE,	"timestamp",	""	}, // CFTYPE_DATETIME
	{ 19,	SQL_C_BINARY,			SQL_SS_TIMESTAMPOFFSET, SQL_UNKNOWN_TYPE,"timestamptz", ""	},
	{ 1,	SQL_C_BINARY,			SQL_VARBINARY,		SQL_LONGVARBINARY,	"varbinary",	"bytea"	}  // CFTYPE_BINARY
};

ContainerDbPostgresql::ContainerDbPostgresql() {
	//PostgreSQL doesn't have a byte column type so store in short type
	ormTypeToContainerFieldTypeMap["int1"] = CFTYPE_SHORT;
	sqlTypeToContainerFieldTypeMap[SQL_TINYINT] = CFTYPE_SHORT;

	// "Wide" strings are treated as multibyte UTF8 in regular varchar columns
	ormTypeToContainerFieldTypeMap["unicodestring"] = CFTYPE_STRING;
	ormTypeToContainerFieldTypeMap["unicodestring(max)"] = CFTYPE_STRING;
	sqlTypeToContainerFieldTypeMap[SQL_WVARCHAR] = CFTYPE_STRING;
	sqlTypeToContainerFieldTypeMap[SQL_WLONGVARCHAR] = CFTYPE_STRING;
}

ContainerDbPostgresql::~ContainerDbPostgresql() {}

std::string ContainerDbPostgresql::createContainerTableQuery(const std::string& table) {
	std::ostringstream ss;
	ss << "CREATE TABLE dbo." << table << " (ContainerId SERIAL NOT NULL PRIMARY KEY, Active INTEGER);";
	return ss.str();
}

std::string ContainerDbPostgresql::select(const std::string& schema, const std::string& table, const std::string& columns, const std::string& limit, const std::string& where)
{
	std::ostringstream ss;
	ss << "SELECT " << columns << " FROM " << schema << '.' << table << ' ' << where << ' ' << limit << ';';
	return ss.str();
}

bool ContainerDbPostgresql::isUnbound(ContainerFieldType cfType, const std::string& sqlTypeName, int columnSize) {
	ContainerFieldInfo info = getContainerFieldInfo(cfType);
	if (info.db_unbound_type.empty()) {
		return false;
	}
	return info.db_unbound_type == sqlTypeName;
}

void ContainerDbPostgresql::beforeSQLMetaCalls(std::string& table) {
	std::transform(table.begin(), table.end(), table.begin(), ::tolower);
}

ContainerFieldInfo& ContainerDbPostgresql::getContainerFieldInfo(ContainerFieldType type) {
	return cToSqlMappingsPostgres[type];
}

std::string ContainerDbPostgresql::restartContainerSequence(const std::string& table) {
	return "ALTER SEQUENCE dbo." + table + "_containerid_seq RESTART;";
}

std::string ContainerDbPostgresql::deleteFromContainer(const std::string& table, const std::string& joinsToTable, const std::string& nullField) {
	std::ostringstream ss;
	ss << "DELETE FROM dbo." << table << " USING dbo." << joinsToTable << " "
		"WHERE " << joinsToTable << ".ContainerId = " << table << ".ContainerId";

	if (nullField != "")
		ss << " AND " << joinsToTable << '.' << nullField << " IS NULL";
	ss << ';';
	return ss.str();
}

std::string ContainerDbPostgresql::dropIndexIfExists(const std::string& indexName, const std::string& table) {
	std::ostringstream ss;
	ss << "DROP INDEX IF EXISTS dbo." << indexName << ';';
	return ss.str();
}

std::string ContainerDbPostgresql::createIndexIfNotExists(const std::string& indexName, const std::string& table, const std::string& columns) {
	std::ostringstream ss;
	ss << "CREATE INDEX IF NOT EXISTS " << indexName << " ON dbo." << table << " (" << columns << ");";
	return ss.str();
}

std::string ContainerDbPostgresql::dropForeignKeyConstraintIfExists(const std::string& table, const std::string& key, const std::string& foreignTable)
{
	std::ostringstream ss;
	std::string fk = foreignKeyNameSchema(table, key, foreignTable);
	ss << "DO $$ BEGIN "
		"IF (SELECT dbo.constraint_exists('" << fk << "')) IS TRUE "
		"THEN ALTER TABLE dbo." << table << " "
		"DROP CONSTRAINT " << fk << ";"
		"END IF;"
		"END $$;";
	return ss.str();
}

std::string ContainerDbPostgresql::createForeignKeyConstraintIfNotExists(const std::string& table, const std::string& key, const std::string& foreignTable) {
	// Can't use ADD IF NOT EXISTS because the table it refers to might not even exist
	std::ostringstream ss;
	std::string fk = foreignKeyNameSchema(table, key, foreignTable);
	ss << "DO $$ BEGIN "
		"IF (SELECT dbo.constraint_exists('" << fk << "')) IS FALSE THEN "
		"ALTER TABLE dbo." << table << " "
		"ADD CONSTRAINT " << fk << " "
		"FOREIGN KEY(" << key << ") REFERENCES dbo." << foreignTable << ";"
		"END IF; "
		"END $$;";
	return ss.str();
}

std::string ContainerDbPostgresql::alterColumnType(const std::string& table, const std::string& column, const std::string& newType) {
	std::ostringstream ss;
	ss << "ALTER TABLE dbo." << table << " ALTER COLUMN " << column << " TYPE " << newType << ';';
	return ss.str();
}

int ContainerDbPostgresql::insertEmptyContainerRow(std::ostringstream& ss, const std::string& table) {
	ss << "SELECT setval(pg_get_serial_sequence('dbo." << table << "', containerid'), ?); "
		"INSERT INTO dbo." << table << " (ContainerId) VALUES (?);";
	// Number of binds needed
	return 2;
}

std::string ContainerDbPostgresql::nowMinusDays(int days) {
	std::ostringstream ss;
	ss << "NOW() - interval '" << days << " days'";
	return ss.str();
}

std::string ContainerDbPostgresql::nextInsertedSequenceValueQuery(const std::string& table, const std::string& column) {
	std::ostringstream ss;
	ss << "SELECT setval(pg_get_serial_sequence('dbo." << table << "', '" << column << "'), nextval(pg_get_serial_sequence('dbo." << table << "', '" << column << "')), false);";
	return ss.str();
}
