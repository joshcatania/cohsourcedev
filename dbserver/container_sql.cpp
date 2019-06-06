#include <stdio.h>
#include "wininclude.h"
#include "sql/sqlinclude.h"
#include "utils.h"
#include "container_sql.h"
#include "container_diff.h"
#include "container_merge.h"
#include "timing.h"
#include "error.h"
#include "assert.h"
#include "Stackdump.h"
#include "strings_opt.h"
#include "mathutil.h"
#include "servercfg.h"
#include "sql_fifo.h"
#include "dbinit.h"
#include "ConvertUTF.h"
#include "EString.h"
#include "StashTable.h"
#include "stringutil.h"
#include "earray.h"
#include "file.h"
#include "log.h"
#include "HashFunctions.h"
#include "dbglobals.h"
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <functional> 
#include <cctype>
#include <locale>

#define strdupa(str) strcpy(_alloca(strlen(str)+1), str)
#define strlwrdupa(str) strlwr(strdupa(str))
#define struprdupa(str) strupr(strdupa(str))

void split(std::vector<std::string>& vect, const std::string& s, char delim);
static inline std::string& ltrim(std::string& s);
static inline std::string& rtrim(std::string& s);
static inline std::string& trim(std::string& s);
static void sqlFlushStatement(HSTMT stmt, std::ostringstream& ss, unsigned* bind, SqlConn conn);

/**
* Enable storing of data in SQL as binary, otherwise use an escaped string.
*
* @note Right now we store data as escaped string in a binary column in the
* tables.  In order to make the switch, the data would have to be unescaped in
* the database using a script before running with a build that has this
* enabled.
*/
#define ACTUALLY_STORE_BINARY_AS_BINARY 0

/** 
* A structure for storing information about each parameter bind
* that requires a temporary buffer until after the SQL statement
* has been closed.
*/ 
typedef struct sqlBindTemp {
	size_t bytes;
	union {
		//char data[0];
		char* data;
		//wchar_t wdata[0];
		wchar_t* wdata;
	};
} sqlBindTemp;

static char* _strdupa(char* str) {
	return strcpy((char*) _alloca(strlen(str) + 1), str);
}

static INLINEDBG char getHexFromBin(int value)
{
	if (value < 10)
		return value + '0';
	return value - 10 + 'a';
}

static INLINEDBG char getBinFromHex(int value)
{
	if (value >= 'a' && value <= 'f')
		return 10 + value - 'a';
	if (value >= '0' && value <= '9')
		return value - '0';
	return 0;
}

static unsigned char* hexStrToBinStr2(char* hexString,U8 *binStr)
{
	int i, len;

	len = strlen(hexString) / 2; // !this will let the last character fall off if we had an odd length string 
	for(i = 0; i < len; i++)
		binStr[i] = (getBinFromHex(hexString[i*2]) << 4) | getBinFromHex(hexString[i*2+1]);
	binStr[i] = 0;
	return binStr;
}

static char* binStrToHexStr2(unsigned char* binArray, char *hexStr)
{
	int i,len=-1;

	len = strlen((char*) binArray);

	for(i=len-1;i>=0;i--)
	{
		hexStr[i*2+0] = getHexFromBin(binArray[i] >> 4);
		hexStr[i*2+1] = getHexFromBin(binArray[i] & 15);
	}
	hexStr[len*2] = '\0';
	return hexStr;
}

bool sqlCreateTable(char *name, TableType table_type)
{
	bool ret;

	// Verify that the table name has no namespace in it
	//devassert(!strchr(name, '.'));

	std::string query;
	
	switch(table_type) {
		xcase TT_ATTRIBUTE :
			query = "CREATE TABLE dbo." + std::string(name) + " (Id INTEGER NOT NULL PRIMARY KEY);";
		xcase TT_CONTAINER:
			query = gContainerDb->createContainerTableQuery(name);
		xcase TT_SUBCONTAINER : {
			std::ostringstream ss;
			ss << "CREATE TABLE dbo." << name << " (ContainerId INTEGER NOT NULL, SubId INTEGER NOT NULL);"
				"ALTER TABLE dbo." << name << " ADD CONSTRAINT PK_" << name << " PRIMARY KEY (ContainerId, SubId);";
			query = ss.str();
		}
		xdefault:
			FatalErrorf("Cannot create unknown table type for %s", name);
	}

	printf("Creating table for some reason: %s\n", query.c_str());
	ret = sqlExecDdl(DDL_ADD, query.c_str(), SQL_NTS);

	if (ret && server_cfg.change_db_owner_from[0])
	{
		// requested by joe phillips
		sqlExecDdlf(DDL_ADD, "sp_changeobjectowner '%s.%s','dbo'", server_cfg.change_db_owner_from, name);
	}

	return ret;
}

bool sqlAddField(char *table, char *name, char *type)
{
	return sqlExecDdlf(DDL_ADD, "ALTER TABLE dbo.%s ADD %s %s;", table, name, type);
}

bool sqlChangeFieldType(char *table, char *name, char *newtype) {
	std::string query = gContainerDb->alterColumnType(table, name, newtype);
	return sqlExecDdl(DDL_ADD, query.c_str(), SQL_NTS);
}

bool sqlDeleteField(char *table, char *name, char *type)
{
	return sqlExecDdlf(DDL_ADD, "ALTER TABLE dbo.%s DROP COLUMN %s;", table, name);
}

/** 
* Execute a semicolon seperated list SQL database commands if 
* the specified DDL command type is allowed by the server.
*/ 
bool sqlExecDdl(DdlType ddl_type, const char *str, int str_len)
{
	sqlCheckDdl(ddl_type);
	return SQL_SUCCEEDED(sqlConnExecDirectMany(str, str_len, SQLCONN_FOREGROUND, true));
}

/** 
* Execute a semicolon seperated list SQL database commands if 
* the specified DDL command type is allowed by the server with 
* printf style formatting. 
*/
bool sqlExecDdlf(DdlType ddl_type, const char *fmt, ...)
{
	bool ret;
	char *buf = estrTemp();
	
	va_list va;
	va_start(va, fmt);
	estrConcatfv(&buf, fmt, va);
	va_end(va);

	ret = sqlExecDdl(ddl_type, buf, estrLength(&buf));

	estrDestroy(&buf);

	return ret;
}

/** 
* Add a SQL database foreign key constraint from a column on the
* first table to be validated against the primary key of a 
* second table.
*/ 
void sqlAddForeignKeyConstraintAsync(char *table, char *key, char *foreign_table) {
	std::string query = gContainerDb->createForeignKeyConstraintIfNotExists(table, key, foreign_table);
	sqlExecAsync(query.c_str(), SQL_NTS);
}

/** 
* Removes a foreign key contraint from a SQL database table.
*/ 
void sqlRemoveForeignKeyConstraintAsync(char *table, char *key, char *foreign_table)
{
	std::string query = gContainerDb->dropForeignKeyConstraintIfExists(table, key, foreign_table);
	sqlExecAsync(query.c_str(), SQL_NTS);
}

/**
* Add a named index to a SQL database table that contains both 
* the table's primary key and columns provided by with the 
* fields parameter. 
*
* @note Uses a hash of the table name to pick a single thread 
*       for all index operations on that table to avoid locking
*       problems.
*/
void sqlAddIndexAsync(char *index, char *table, char *fields) {
	std::string query = gContainerDb->createIndexIfNotExists(index, table, fields);
	sqlExecAsyncEx(query.c_str(), SQL_NTS, hashStringInsensitive(table), false);
}

/**
* Remove an index by name from a SQL database table.
*
* @note Uses a hash of the table name to pick a single thread 
*       for all index operations on that table to avoid locking
*       problems.
*/
void sqlRemoveIndexAsync(char *index, char *table)
{
	std::string query = gContainerDb->dropIndexIfExists(index, table);
	sqlExecAsyncEx(query.c_str(), SQL_NTS, hashStringInsensitive(table), false);
}

void sqlDeleteRowInternal(char *table, int container_id, SqlConn conn)
{
	char buf[SHORT_SQL_STRING];
	int buf_len;

	buf_len = sprintf(buf,"DELETE FROM dbo.%s WHERE ContainerId = %d;", table, container_id);
	sqlConnExecDirect(buf, buf_len, conn, false);
}

void sqlDeleteContainer(ContainerTemplate *tplt, int container_id)
{
	int			i;
	TableInfo	*table;

	if (!tplt || tplt->dont_write_to_sql)
		return;

	for(i=tplt->table_count-1;i>=0;i--)
	{
		table = &tplt->tables[i];
		sqlDeleteRowAsync(table->name, container_id);
	}

	return;
}

static void sqlFlushStatement(HSTMT stmt, std::ostringstream& ss, unsigned* bind, SqlConn conn) {
	std::string query = ss.str();
	sqlConnStmtExecDirectMany(stmt, query.c_str(), SQL_NTS, conn, false);
	ss.clear();
	ss.str("");

	sqlConnStmtUnbindCols(stmt);
	sqlConnStmtUnbindParams(stmt);
	sqlConnStmtCloseCursor(stmt);
	*bind = 0;
}

static void sqlFlushStatement(HSTMT stmt, char **estr, unsigned *bind, SqlConn conn)
{
#ifdef _FULLDEBUG
	//assert(estrUTF8CharacterCount(estr));
#endif

	sqlConnStmtExecDirectMany(stmt, *estr, estrLength(estr), conn, false);
	estrClear(estr);

	sqlConnStmtUnbindCols(stmt);
    sqlConnStmtUnbindParams(stmt);
	sqlConnStmtCloseCursor(stmt);
	*bind = 0;
}

static void sqlContainerAddOrDelRows(ContainerTemplate *tplt, int *container_id, LineList *diff, char **estr, unsigned *bind, HSTMT stmt, SqlConn conn)
{
	printf("sqlContainerAddOrDelRows cmd count: %d\n", diff->cmd_count);
	std::ostringstream* ss = new std::ostringstream();
	for (int i=0; i < diff->cmd_count; i++) {
		RowAddDel *cmd = &diff->row_cmds[i];
		SlotInfo *slot = &tplt->slots[cmd->idx];
		char *name = slot->table->name;

		if (cmd->add) {
			switch (slot->table->table_type) {
				xcase TT_CONTAINER : {
					int bindsNeeded = gContainerDb->insertEmptyContainerRow(*ss, name);
					for (int i = 0; i < bindsNeeded; i++) {
						bindInputParameter(stmt, (*bind)++, CFTYPE_INT, container_id, NULL);
					}
				}
				xcase TT_SUBCONTAINER:
					*ss << "INSERT INTO dbo." << name << " (ContainerId,SubId) VALUES (?,?);";
					bindInputParameter(stmt, (*bind)++, CFTYPE_INT, container_id, NULL);
					bindInputParameter(stmt, (*bind)++, CFTYPE_INT, &slot->sub_id, NULL);					
				xdefault:
					FatalErrorf(__FUNCTION__ "does not implement table type %d", slot->table->table_type);
			}
		} else {
			switch (slot->table->table_type) {
				xcase TT_CONTAINER:
					*ss << "DELETE FROM dbo." << name << " WHERE ContainerId=?;";
					bindInputParameter(stmt, (*bind)++, CFTYPE_INT, container_id, NULL);
				xcase TT_SUBCONTAINER:
					*ss << "DELETE FROM dbo." << name << " WHERE ContainerId=? AND SubId=?;";
					bindInputParameter(stmt, (*bind)++, CFTYPE_INT, container_id, NULL);
					bindInputParameter(stmt, (*bind)++, CFTYPE_INT, &slot->sub_id, NULL);
				xdefault:
					FatalErrorf(__FUNCTION__ "does not implement table type %d", slot->table->table_type);
			}
		}

		if (ss->tellp() > LONG_SQL_STRING || *bind >= FLUSH_BINDS) {
			sqlFlushStatement(stmt, *ss, bind, conn);
		}
	}
	delete ss;
}

static void sqlContainerUpdateRows(ContainerTemplate *tplt, int *container_id, LineList *diff, char **estr, unsigned *bind, HSTMT stmt, SqlConn conn, sqlBindTemp *** bind_temps)
{
	unsigned i;
	unsigned diff_count = diff->count;

	for (i=0; i<diff_count; )
	{
		SlotInfo	*last_slot = &tplt->slots[diff->lines[i].idx];

		estrConcatf(estr, "UPDATE dbo.%s SET ", last_slot->table->name);
		for (; i<diff_count; i++)
		{
			LineTracker	*line	= &diff->lines[i];
			SlotInfo	*slot	= &tplt->slots[line->idx];
			TableInfo	*table	= slot->table;
			ColumnInfo	*col	= &table->columns[slot->idx];

			if (slot->sub_id != last_slot->sub_id || table != last_slot->table)
				break;

			estrConcatf(estr, "%s=?,", col->name);
			switch(col->data_type)
			{
				default:
#ifdef _FULLDEBUG
					assert(strlen(diff->text + line->str_idx) == line->size);
#endif
					bindInputParameter(stmt, (*bind)++, CFTYPE_STRING, diff->text + line->str_idx, &line->size);
					break;
				case CFTYPE_BINARY:
				{
#if ACTUALLY_STORE_BINARY_AS_BINARY
					int int_size;
					sqlBindTemp * temp = malloc(sizeof(sqlBindTemp) + line->size);
					unescapeDataToBuffer(temp->data, diff->text + line->str_idx, &int_size);
					temp->bytes = int_size;
					bindInputParameter(stmt, (*bind)++, CFTYPE_BINARY_MAX, temp->data, &temp->bytes);
					eaPush(bind_temps, temp);
#else
					bindInputParameter(stmt, (*bind)++, CFTYPE_BINARY, diff->text + line->str_idx, &line->size);
#endif
					break;
				}
				case CFTYPE_INT:
				case CFTYPE_SHORT:
				case CFTYPE_BYTE:
					bindInputParameter(stmt, (*bind)++, CFTYPE_INT, line->ival ? &line->ival : NULL, NULL);
					break;
				case CFTYPE_FLOAT:
					bindInputParameter(stmt, (*bind)++, CFTYPE_FLOAT, &line->fval, NULL);
					break;
			}
		}

		estrSetLength(estr, estrLength(estr)-1);
		if (!last_slot->table->array_count) {
			estrConcatStaticCharArray(estr, " WHERE ContainerId=?;");
			bindInputParameter(stmt, (*bind)++, CFTYPE_INT, container_id, NULL);
		} else {
			estrConcatStaticCharArray(estr, " WHERE ContainerId=? AND SubId=?;");
			bindInputParameter(stmt, (*bind)++, CFTYPE_INT, container_id, NULL);
			bindInputParameter(stmt, (*bind)++, CFTYPE_INT, &last_slot->sub_id, NULL);
		}

		if (estrLength(estr) > LONG_SQL_STRING || *bind >= FLUSH_BINDS) {
			sqlFlushStatement(stmt, estr, bind, conn);
		}
	}
}

/**
* @note Need an use array of temporary buffers in order to do this (string classes
* will move the memory on a reallocation).
*/
void sqlContainerUpdateInternal(ContainerTemplate* tplt, int container_id, LineList* diff, SqlConn conn)
{
	static char *estrs[SQLCONN_MAX] = {0,};
	static sqlBindTemp** bind_temps[SQLCONN_MAX] = {0,};
	unsigned bind = 0;
	HSTMT stmt;

	stmt = sqlConnStmtAlloc(conn);

	estrClear(&estrs[conn]);

	if (!bind_temps[conn])
		eaCreate(&bind_temps[conn]);

	sqlContainerAddOrDelRows(tplt, &container_id, diff, &estrs[conn], &bind, stmt, conn);
	sqlContainerUpdateRows(tplt, &container_id, diff, &estrs[conn], &bind, stmt, conn, &bind_temps[conn]);

	if (estrLength(&estrs[conn]))
		sqlFlushStatement(stmt, &estrs[conn], &bind, conn);

	sqlConnStmtFree(stmt);

	eaDestroyEx(&bind_temps[conn], NULL);
}

static void* s_getDataAtExec(HSTMT stmt, ColumnInfo *field, int column_idx, SQLLEN *results, const char *original_command, SqlConn conn) {
	return sqlConnStmtGetData(stmt, column_idx+1, gContainerDb->getContainerFieldInfo(field->data_type).c_type, results, original_command, conn);
}

static char row_data[SQLCONN_MAX][MAX_QUERY_SIZE];
static SQLLEN row_results[SQLCONN_MAX][MAX_QUERY_RESULTS];
static int glob_container_id[SQLCONN_MAX];

static void s_bindField(HSTMT stmt, ColumnInfo *field, int column_idx, int *data_idx, SqlConn conn)
{
	if (!CFTYPE_IS_BOUND(field->data_type, field->column_size)) {
		assert(field->num_bytes == -1);
		return;
	}

	bindOutputColumn(stmt, column_idx, field->data_type, field->num_bytes, &row_data[conn][*data_idx], &row_results[conn][column_idx]);

	assert(field->num_bytes > 0);
	(*data_idx) += field->num_bytes;
}

static void* s_getField(HSTMT stmt, ColumnInfo *field, int column_idx, int *data_idx, SqlConn conn, const char *original_command)
{
	char *data = &row_data[conn][*data_idx];
	SQLLEN *results = row_results[conn] + column_idx;

	if (!CFTYPE_IS_BOUND(field->data_type, field->column_size)) {
		return s_getDataAtExec(stmt, field, column_idx, results, original_command, conn);
	}

	assert(field->num_bytes > 0);
	(*data_idx) += field->num_bytes;
	return data;
}

static HSTMT sqlTableSelect(TableInfo *table, int *id_ptr, SqlConn conn)
{
	int idx;
	int col;
	SQLRETURN retcode;
	HSTMT stmt;

	stmt = table->sql_select_link[conn];
	if (stmt)
		return stmt;

	table->sql_select_link[conn] = stmt = sqlConnStmtAlloc(conn);

	estrPrintf(&table->sql_select_query[conn], "SELECT * FROM dbo.%s WHERE ContainerId = ?;", table->name);

	retcode = sqlConnStmtPrepare(stmt, table->sql_select_query[conn], estrLength(&table->sql_select_query[conn]), conn);
	bindInputParameter(stmt, 0, CFTYPE_INT, id_ptr, NULL);

	idx = 0;
	assert(table->num_columns <= MAX_QUERY_RESULTS);
	for(col = 0; col < table->num_columns; col++)
		s_bindField(stmt, &table->columns[col], col, &idx, conn);
	assert(idx <= MAX_QUERY_SIZE);
	
	return stmt;
}

static bool addMemToLine(LineList *list, LineTracker *line, char *s, int s_len)
{
	char *base;
	int int_size;

	if (!s_len)
		return 0;

	line->str_idx = list->text_count;
	line->is_str = 1;

	base = (char*) dynArrayAdd((void**) &list->text, 1, &list->text_count, &list->text_max, escapeDataMaxChars(s_len)+1);
	escapeDataToBuffer(base, s, s_len, &int_size);
	line->size = int_size;
	base[line->size] = 0;

	// Trim excess space
	list->text_count = line->str_idx + line->size + 1;

	return true;
}

static bool addWStrToLine(LineList *list, LineTracker *line, wchar_t *ws, int ws_len)
{
	char *base;
	int worst_utf8_size;

	if (!ws_len && !(ws_len = wcslen(ws)))
		return 0;

#ifdef _FULLDEBUG
	assert(ws_len == wcslen(ws));
#endif

	line->str_idx = list->text_count;
	line->is_str = 1;

	// Allocate 3/2 the storage of the original string, which is the worst case for a UTF16 -> UTF8 conversion
	worst_utf8_size = ws_len*3+1;
	base = (char*) dynArrayAdd((void**) &list->text, 1, &list->text_count, &list->text_max, worst_utf8_size);
	line->size = WideCharToMultiByte(CP_UTF8, 0, ws, ws_len, base, worst_utf8_size, NULL, NULL);
	base[line->size] = 0;

#ifdef _FULLDEBUG
	//assert(UTF16GetLength(ws) == UTF8GetLength(base));
#endif

	// Trim extra space
	list->text_count = line->str_idx + line->size + 1;

	return true;
}

static bool addStrToLine(LineList *list, LineTracker *line, char *s, int s_len)
{
	char *base;

	if (!s_len && !(s_len = strlen(s)))
		return 0;

#ifdef _FULLDEBUG
	assert(s_len == strlen(s));
#endif

	line->str_idx = list->text_count;
	line->is_str = 1;
	line->size = s_len;

	base = (char*) dynArrayAdd((void**) &list->text, 1, &list->text_count, &list->text_max, s_len+1);
	memcpy(base, s, s_len);
	base[s_len] = 0;

	return true;
}

/**
* @note This function should parallel structToLine() in dbcontainerpack.c
*/
static int readRow(HSTMT stmt, ContainerTemplate *tplt, TableInfo *table, LineList *list, SqlConn conn, int read_reserved, char *original_command)
{
	SQLLEN *results = row_results[conn];
	bool found = false;
	int sub_id = -1; // -1 indicates this is not a subcontainer
	int idx = 0;
	int col;

	if(table->array_count)
	{
		// Assume that SubId is hard coded in the buffer at offset 4...
		sub_id = *(int*)(&row_data[conn][4]);

		if (sub_id >= table->array_count)	// read too many rows
			return 0;
	}

	assert(table->num_columns <= MAX_QUERY_RESULTS);
	for(col = 0; col < table->num_columns; col++)
	{
		ColumnInfo *field = &table->columns[col];
		void *data;
		int cmd_idx;
		int line_count;
		LineTracker	*line;

		data = s_getField(stmt, field, col, &idx, conn, original_command);
		if(!data)
			return -1;

		found = true;

		if ((results[col] == SQL_NULL_DATA) || (field->reserved_word && !read_reserved) || field->is_sub_id_field)
			continue;

		if (read_reserved)
		{
			char table_field[256];

			if (field->reserved_word || sub_id < 0)
				strcpy(table_field, field->name);
			else
				sprintf(table_field, "%s[%d].%s", table->name, sub_id, field->name);

			if (!stashFindInt(tplt->all_hashes, table_field, &cmd_idx))
				continue;
		}
		else
		{
			cmd_idx = table->all_hash_first_idx + col;
			if (sub_id >= 0)
				cmd_idx += sub_id * table->num_columns;
		}

		// Grow to make room for the new line, but do not mark it as added unless we retrieve a valid "non-deafault" value
		line_count = list->count;
		line = (LineTracker*) dynArrayAdd((void**) &list->lines, sizeof(list->lines[0]), &line_count, &list->max_lines, 1);
		line->is_str = 0;
		line->idx = cmd_idx;

		switch(field->data_type)
		{
			xcase CFTYPE_INT:
#ifdef _FULLDEBUG
				assert(results[col] == sizeof(int));
#endif
				if(!(line->ival = *(int *)data))
					continue;

				// Validate the range of the attribute
				if ((field->attr) && ((line->ival < 1) || (line->ival >= eaSize(&field->attr->names))))
					continue;

			xcase CFTYPE_SHORT:
#ifdef _FULLDEBUG
				assert(results[col] == sizeof(short));
#endif
				if(!(line->ival = *(short *)data))
					continue;

			xcase CFTYPE_BYTE:
#ifdef _FULLDEBUG
				assert(results[col] == sizeof(U8));
#endif
				if(!(line->ival = *(U8 *)data))
					continue;

			xcase CFTYPE_FLOAT:
#ifdef _FULLDEBUG
				assert(results[col] == sizeof(float));
#endif
				if(!(line->fval = *(float *)data))
					continue;

			xcase CFTYPE_BINARY:
#if ACTUALLY_STORE_BINARY_AS_BINARY
				if(!addMemToLine(list, line, data, results[col]))
					continue;
#else
				if(!addStrToLine(list, line, (char*) data, results[col]))
					continue;
#endif
			xcase CFTYPE_STRING:
				if(!addStrToLine(list, line, (char*) data, results[col]))
					continue;

			xcase CFTYPE_DATETIME:
			{
				SQL_TIMESTAMP_STRUCT *t = (SQL_TIMESTAMP_STRUCT*)(data);
				char buf[128];
				int buf_len;

#ifdef _FULLDEBUG
				assert(results[col] == sizeof(SQL_TIMESTAMP_STRUCT));
#endif

				if (t->year < 2001 || t->year > 2100) //old corruption lead to many 2136 dates, we should ignore them
				{
					// If this date looks invalid, report it as null
					continue;
				}
				buf_len = sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d", t->year, t->month, t->day, t->hour, t->minute, t->second);
				addStrToLine(list, line, buf, buf_len);
			}
			xcase CFTYPE_DATETIME_TIMEZONE :
			{
				SQL_SS_TIMESTAMPOFFSET_STRUCT* tsWithTz = (SQL_SS_TIMESTAMPOFFSET_STRUCT*)(data);
#ifdef _FULLDEBUG
				assert(results[col] == sizeof(SQL_TIMESTAMP_STRUCT));
#endif
				char buf[64];
				int buf_len;
				buf_len = sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tsWithTz->year, tsWithTz->month, tsWithTz->day, tsWithTz->hour, tsWithTz->minute, tsWithTz->second);
				addStrToLine(list, line, buf, buf_len);
			}

			xdefault:
				assertmsgf(0, "unhandled line type %d", field->data_type);
		}

		// Got a valid "non-default" value, mark the line as valid
		list->count = line_count;
	}

	// No non-NULL values were found for this row, in order to indicate the
	// existence of this row in the database, add a fake entry.
	if (!list->count) {
		LineTracker	*line = (LineTracker*) dynArrayAdd((void**) &list->lines, sizeof(list->lines[0]), &list->count, &list->max_lines, 1);
		line->str_idx = FAKE_STR_IDX;
		line->is_str = 1;
		line->size = 0;

		// Point at the ContainerId field
		line->idx = table->all_hash_first_idx;
		if (sub_id >= 0)
			line->idx += sub_id * table->num_columns;
	}

	return found;
}

static int sqlTableRead(ContainerTemplate *tplt, TableInfo *table, int container_id, LineList *list, SqlConn conn)
{
	HSTMT		stmt;
	RETCODE		retcode;
	int			found=0;
	int			multi_count=0;

	stmt = sqlTableSelect(table, &glob_container_id[conn], conn);
	glob_container_id[conn] = container_id;

	retcode = _sqlConnStmtExecute(stmt, conn);
	if (SQL_SHOW_ERROR(retcode))
		sqlConnStmtPrintError(stmt, table->sql_select_query[conn]);

	while((retcode = _sqlConnStmtFetch(stmt, conn)) != SQL_NO_DATA)
	{
		int r;

		if (retcode != SQL_SUCCESS)
			sqlConnStmtPrintError(stmt, table->sql_select_query[conn]);

		if (!SQL_SUCCEEDED(retcode))
			break;

		r = readRow(stmt, tplt, table, list, conn, 0, table->sql_select_query[conn]);
		if(r < 0)
			break;
		found |= r;
	}

	sqlConnStmtCloseCursor(stmt);

	return found;
}

int sqlTableReadMulti(ContainerTemplate *tplt, TableInfo *table, int container_id, char ***multi_buf_ptr, SqlConn conn)
{
	HSTMT stmt;
	char cmd_buf[SHORT_SQL_STRING];
	size_t cmd_len;

	RETCODE		retcode;
	int			idx,col,cant_find=1;
	int			multi_count=0;
	char		**multi_buf=0;
	LineList	list = {0};

	TODO(); // Code cleanup needed to improve clarity for future audits

	stmt = sqlConnStmtAlloc(conn);

	cmd_len = sprintf(cmd_buf, "SELECT * FROM %s;", table->name);

	idx = 0;
	assert(table->num_columns <= MAX_QUERY_RESULTS);
	for(col = 0; col < table->num_columns; col++)
		s_bindField(stmt, &table->columns[col], col, &idx, conn);
	assert(idx <= MAX_QUERY_SIZE);

	retcode = sqlConnStmtExecDirect(stmt, cmd_buf, cmd_len, conn, true);

	while((retcode = _sqlConnStmtFetch(stmt, conn)) != SQL_NO_DATA)
	{
		int		size;
		char	*mem;

		if (retcode != SQL_SUCCESS)
			sqlConnStmtPrintError(stmt, cmd_buf);

		if(!SQL_SUCCEEDED(retcode))
			continue;

		if(readRow(stmt, tplt, table, &list, conn, 1, cmd_buf) < 0)
			break;

		multi_count++;
		multi_buf = (char**) realloc(multi_buf,multi_count * sizeof(char **));
		mem = (char*) lineListToMem(&list, &size, 0, 0);
		multi_buf[multi_count-1] = (char*) malloc(size);
		memcpy(multi_buf[multi_count-1],mem,size);
		reinitLineList(&list);
	}

	sqlConnStmtFree(stmt);

	*multi_buf_ptr = multi_buf;
	freeLineList(&list);
	return multi_count;
}

char *sqlContainerRead(ContainerTemplate *tplt, int container_id, SqlConn conn)
{
	static LineList lists[SQLCONN_MAX];
	static void *mem_buf[SQLCONN_MAX];
	static int mem_max[SQLCONN_MAX];

	LineList *list = &lists[conn];
	int i, size;
	void *mem;

	if(!tplt)
		return 0;

	reinitLineList(list);

	for(i = 0; i < tplt->table_count; i++)
		if(!sqlTableRead(tplt, &tplt->tables[i], container_id, list, conn) && !i)
			return 0;// Primary table read failed - container doesn't exist.

	lineListToMem(list, &size, &mem_buf[conn], &mem_max[conn]);
	mem = malloc(size);
	memcpy(mem, mem_buf[conn], size);

	return (char*) mem;
}

/** 
* Return a single integer result from the result of executing a
* command on the SQL database.
* 
* @param bind_list An optional list of parameter
*                  binds terminated by an entry of type @ref
*                  CFTYPE_NULL.
*/ 
int sqlGetSingleValue(const char *cmd, int cmd_len, BindList * bind_list, SqlConn conn)
{
	HSTMT stmt;
	SQLRETURN retcode;
	int count = 0;

	stmt = sqlConnStmtAlloc(conn);

	if (bind_list) {
		int bind;
		for (bind = 0; bind_list->data_type; bind++, bind_list++) {
			bindInputParameter(stmt, bind, bind_list->data_type, bind_list->data, NULL);
		}		
	}

	bindOutputColumn(stmt, 0, CFTYPE_INT, sizeof(count), &count, &row_results[conn][0]);

	retcode = sqlConnStmtExecDirect(stmt, cmd, cmd_len, conn, true);
	if (SQL_SUCCEEDED(retcode)) {
		retcode = _sqlConnStmtFetch(stmt, conn);
		if (SQL_SHOW_ERROR(retcode))
			sqlConnStmtPrintError(stmt, cmd);
	}

	sqlConnStmtFree(stmt);
	return count;
}



char *sqlReadColumnsInternal(TableInfo *table, char *limit, char *col_names, char *where, int *count_ptr, ColumnInfo **field_ptrs, SqlConn conn, int debugDelayMS)
{
	unsigned int i;
	int retcode;
	HSTMT stmt;
	int count = 0;

	char *dyn_mem = NULL;
	int dyn_mem_max = 0;
	int dyn_mem_idx = 0;

	int rec_size=0;

	if (!limit)
		limit = "";
	if (!where)
		where = "";

	if (debugDelayMS)
	{
		printf("Sleeping for %d MS due to debug setting! (%s)\n", debugDelayMS, where);
		Sleep(debugDelayMS);
		printf("Done sleeping.\n");
	}

	TODO(); // Code cleanup needed to improve clarity for future audits

	std::string columnsString = col_names;
	std::vector<std::string> columns;
	split(columns, columnsString, ',');
	size_t num_columns = columns.size();
	assert(num_columns <= MAX_QUERY_RESULTS);

	std::string query = gContainerDb->select("dbo", table->name, columnsString, limit, where);
	stmt = sqlConnStmtAlloc(conn);

	for(i = 0; i < num_columns; i++)
	{
		int col;
		for(col = 0; col < table->num_columns; col++)
		{
			ColumnInfo *field = &table->columns[col];
			if(stricmp(columns[i].c_str(), field->name)==0)
			{
				// this doesn't work because i don't have a good way to pass the data around and ensure that the it will be freed
				assert(CFTYPE_IS_BOUND(field->data_type, field->column_size));

				s_bindField(stmt, field, i, &rec_size, conn);

				field_ptrs[i] = field;
				break;
			}
		}
		if(col >= table->num_columns)
		{
			LOG(LOG_ERROR, LOG_LEVEL_IMPORTANT, LOG_CONSOLE, "Requested column name %s.%s doesn't exist.\n", table->name, columns[i].c_str());
			return 0;
		}
	}
	assert(rec_size <= MAX_QUERY_SIZE);

	field_ptrs[i] = 0;

	retcode = sqlConnStmtExecDirect(stmt, query.c_str(), SQL_NTS, conn, true);
	if (!SQL_SUCCEEDED(retcode))
	{
		*count_ptr = 0;
		return 0;
	}

	while (true)
	{
		int idx = 0;

		TODO(); // do we need this memset?
		memset(row_data[conn], 0, rec_size);

		retcode = _sqlConnStmtFetch(stmt, conn);
		if (retcode == SQL_NO_DATA)
			break;

		count++;
		if (retcode != SQL_SUCCESS)
			sqlConnStmtPrintError(stmt, query.c_str());

		if(!SQL_SUCCEEDED(retcode))
			break;

		for(i = 0; i < num_columns; i++)
		{
			void *data = s_getField(stmt, field_ptrs[i], i, &idx, conn, query.c_str());
			if(!data)
				break;
		}
		if(i < num_columns)
			break;

		dynArrayFit((void**) &dyn_mem, 1, &dyn_mem_max, (count+1)*rec_size);
		memcpy(dyn_mem + dyn_mem_idx, row_data[conn], rec_size);
		dyn_mem_idx += rec_size;
	}

	sqlConnStmtFree(stmt);

	*count_ptr = count;

	return dyn_mem;
}

/** 
* Query the list of columns for a given table in the SQL
* database as an array of @ref ColumnInfo structures.
*/ 
int sqlGetTableInfo(char *table_name,ColumnInfo **columns_ptr)
{
	HSTMT stmt;
	int count;
	ColumnInfo *columns = NULL;
	const char * schema_name = "dbo";

	SQLCHAR column_name[256];
	SQLLEN column_name_bytes;

	SQLSMALLINT data_type;
	SQLLEN data_type_bytes;

	SQLCHAR type_name[256];
	SQLLEN type_name_bytes;

	SQLINTEGER column_size;
	SQLLEN column_size_bytes;

	SQLINTEGER buffer_length;
	SQLLEN buffer_length_bytes;

	SQLINTEGER char_octet_length;
	SQLLEN char_octet_length_bytes;

	stmt = sqlConnStmtAlloc(SQLCONN_FOREGROUND);

	std::string table = table_name;
	gContainerDb->beforeSQLMetaCalls(table);

	if (!SQL_SUCCEEDED(SQLColumns(stmt, NULL, 0, (SQLCHAR*)schema_name, 3, (SQLCHAR*) table.c_str(), SQL_NTS, NULL, 0)))
	{
		sqlConnStmtFree(stmt);
		return 0;
	}

	assert(SQL_SUCCEEDED(SQLBindCol(stmt, 4, SQL_C_CHAR, column_name, ARRAY_SIZE(column_name), &column_name_bytes)));
	assert(SQL_SUCCEEDED(SQLBindCol(stmt, 5, SQL_C_SSHORT, &data_type, 0, &data_type_bytes)));
	assert(SQL_SUCCEEDED(SQLBindCol(stmt, 6, SQL_C_CHAR, type_name, ARRAY_SIZE(type_name), &type_name_bytes)));
	assert(SQL_SUCCEEDED(SQLBindCol(stmt, 7, SQL_C_SLONG, &column_size, 0, &column_size_bytes)));
	assert(SQL_SUCCEEDED(SQLBindCol(stmt, 8, SQL_C_SLONG, &buffer_length, 0, &buffer_length_bytes)));
	assert(SQL_SUCCEEDED(SQLBindCol(stmt, 16, SQL_C_SLONG, &char_octet_length, 0, &char_octet_length_bytes)));

	for(count=0; ; count++)
	{
		SQLRETURN retcode;

		retcode = _sqlConnStmtFetch(stmt, SQLCONN_FOREGROUND);

		if (retcode == SQL_NO_DATA)
			break;

		if (retcode != SQL_SUCCESS)
			sqlConnStmtPrintErrorf(stmt, "SQLColumns <%s>", table_name);

		if (!SQL_SUCCEEDED(retcode))
			continue;

		assert(column_name_bytes > 0);
		assert(data_type_bytes > 0);
		assert(type_name_bytes > 0);
		assert(column_size_bytes > 0);
		assert(buffer_length_bytes > 0);

		// Override buffer length with octet length when necessary
		if (char_octet_length_bytes > 0 && char_octet_length > buffer_length)
			buffer_length = char_octet_length;

		columns = (ColumnInfo*) realloc(columns, (count+1) * sizeof(*columns));
		memset(&columns[count], 0, sizeof(*columns));

		strcpy(columns[count].name, (char*) column_name);

		columns[count].data_type = CFTYPE_NULL;
		columns[count].num_bytes = buffer_length;
		columns[count].column_size = 0;
		columns[count].data_type = gContainerDb->sqlTypeToContainerFieldType(data_type);

		if (columns[count].data_type == CFTYPE_NULL) {
			FatalErrorf("Unknown type '%s' (%d) for column '%s' in '%s'", type_name, data_type, column_name, table_name);
		}

		// THis is going to need A LOT of commenting
		// Database dependent. Postgres has different type names for bound and unbound
		// Mssql uses varchar and varchar max, except max isnt passed back. not sure what to do.
		// probably use data type returned from sql
		if (CFTYPE_IS_UNBOUNDABLE(columns[count].data_type)) {
			if (gContainerDb->isUnbound(columns[count].data_type, (char*) type_name, column_size)) {
				columns[count].num_bytes = -1;
				strncpy(columns[count].data_type_name, gContainerDb->getContainerFieldInfo(columns[count].data_type).db_unbound_type.c_str(), 31);
				columns[count].data_type_name[32] = '\0';
			} else {
				// We only care to set the column size for bound string/binary columns
				columns[count].column_size = column_size;
				sprintf(columns[count].data_type_name, "%s(%d)", type_name, column_size);
			}
		} else {
			strcpy(columns[count].data_type_name, (char*) type_name);
		}
	}

	sqlConnStmtFree(stmt);

	*columns_ptr = columns;
	return count;
}


void sqlDropForeignKeysForTable(const char *table_name)
{
	SQLLEN	pktable_schem_ind,pktable_name_ind,pkcolumn_name_ind,
			fktable_schem_ind,fktable_name_ind,fkcolumn_name_ind,
			fkey_name_ind,pkey_name_ind,update_ind,delete_ind;
	char	pktable_schem_s[129],pktable_name_s[129],pkcolumn_name_s[129],
			fktable_schem_s[129],fktable_name_s[129],fkcolumn_name_s[129],
			fkey_name_s[129],pkey_name_s[129];
	short	update_rule,delete_rule;
	int		i,rc;
	SQLHSTMT      stmt;
	char	**fk_names=0;
	int		fk_count=0,fk_max=0;
	char	buf[SHORT_SQL_STRING];

	char * schema_name = "dbo";

	TODO(); // Code cleanup needed to improve clarity for future audits

	stmt = sqlConnStmtAlloc(SQLCONN_FOREGROUND);

    rc = SQLForeignKeys(stmt, NULL, 0, (SQLCHAR*)schema_name, SQL_NTS, (SQLCHAR*)table_name, SQL_NTS, NULL, 0, NULL, 0, NULL, 0);
    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLPOINTER) pktable_schem_s, 129, &pktable_schem_ind);
    rc = SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLPOINTER) pktable_name_s, 129, &pktable_name_ind);
    rc = SQLBindCol(stmt, 4, SQL_C_CHAR, (SQLPOINTER) pkcolumn_name_s, 129, &pkcolumn_name_ind);
    rc = SQLBindCol(stmt, 6, SQL_C_CHAR, (SQLPOINTER) fktable_schem_s, 129, &fktable_schem_ind);
    rc = SQLBindCol(stmt, 7, SQL_C_CHAR, (SQLPOINTER) fktable_name_s, 129, &fktable_name_ind);
    rc = SQLBindCol(stmt, 8, SQL_C_CHAR, (SQLPOINTER) fkcolumn_name_s, 129, &fkcolumn_name_ind);
    rc = SQLBindCol(stmt, 10, SQL_C_SHORT, (SQLPOINTER) &update_rule,  0, &update_ind);
    rc = SQLBindCol(stmt, 11, SQL_C_SHORT, (SQLPOINTER) &delete_rule,  0, &delete_ind);
    rc = SQLBindCol(stmt, 12, SQL_C_CHAR, (SQLPOINTER) fkey_name_s, 129, &fkey_name_ind);
    rc = SQLBindCol(stmt, 13, SQL_C_CHAR, (SQLPOINTER) pkey_name_s, 129, &pkey_name_ind);

    //printf("Primary Key and Foreign Keys for %s.%s\n", "", tablename);
    /* Fetch each row, and display */
    while ((rc = _sqlConnStmtFetch(stmt, SQLCONN_FOREGROUND)) == SQL_SUCCESS)
	{
        if (fkey_name_ind > 0 )
		{
			sprintf(buf,"ALTER TABLE dbo.%s DROP CONSTRAINT %s;",fktable_name_s,fkey_name_s);
			dynArrayAddp((void***) &fk_names, &fk_count, &fk_max, strdup(buf));
		}
    }

	sqlConnStmtFree(stmt);

	for(i=0;i<fk_count;i++)
	{
		sqlExecDdl(DDL_DEL, fk_names[i], SQL_NTS);
		free(fk_names[i]);
	}
	free(fk_names);
}

int sqlDropTable(const char *table)
{
	char cmd[SHORT_SQL_STRING];
	int cmd_len;

	sqlDropForeignKeysForTable(table);
	cmd_len = sprintf(cmd, "DROP TABLE dbo.%s;",table);
	if (stricmp(table,"Attributes")==0)
		FatalErrorf("ATTRIBUTE TABLE DROP BUG!!! call Paragon immediately!");
	return (sqlExecDdl(DDL_REBUILDTABLE, cmd, cmd_len)) ? 1 : -1;
}

std::vector<std::string> sqlReadTableNames() {
	HSTMT stmt = sqlConnStmtAlloc(SQLCONN_FOREGROUND);

	int retcode = SQLTables(stmt, NULL, 0, (SQLCHAR*) "dbo", SQL_NTS, NULL, 0, (SQLCHAR*)"TABLE", SQL_NTS);

	char tabName[255];
	SQLLEN lenTableName;

	// Index 3 contains the table names
	retcode = SQLBindCol(stmt, 3, SQL_C_CHAR, tabName, 255, &lenTableName);

	std::vector<std::string> tableNames;
	while ((retcode = _sqlConnStmtFetch(stmt, SQLCONN_FOREGROUND)) == SQL_SUCCESS)
		tableNames.push_back(std::string(tabName, lenTableName));

	sqlConnStmtFree(stmt);
	return tableNames;
}

/** 
* Validates whether a Data Definition Language (DDL)
* command of the specified type is allowed by the server.
*/
void sqlCheckDdl(DdlType type)
{
	char	*name = "Unknown";

	// this one trumps everything, since you don't want to drop the table, then find out you're not allowed to re-add it
	if (server_cfg.ddl_rebuildtable || server_cfg.sql_allow_all_ddl)
		return;

	switch(type)
	{
		xcase DDL_ATTRIBUTES:
			name = "SqlAddAttributes";
			if (server_cfg.ddl_attributes)
				return;
		xcase DDL_ADD:
			name = "SqlAddColumnOrTable";
			if (server_cfg.ddl_add)
				return;
		xcase DDL_DEL:
			name = "SqlDeleteColumnOrTable";
			if (server_cfg.ddl_del)
				return;
		xcase DDL_REBUILDTABLE:
			name = "SqlRebuildTable";
			if (server_cfg.ddl_rebuildtable)
				return;
		xcase DDL_ALTERCOLUMN:
			name = "SqlAlterColumn";
			if (server_cfg.ddl_altercolumn)
				return;
	}

	FatalErrorf("The previous operation, \"%s\" needed to modify the structure of the\n"
				"database. Since that is disabled, the only thing to do now is quit.\n"
				"Set \"%s 1\" in servers.cfg if you want to allow it.\n"
				"allow it.",name,name);
}

/** 
* Build various test strings to validate that a string of
* special characters can fit in the SQL database table column.
*/ 
static char * testString(char * string, size_t len, bool unicode, bool min)
{
	//static const char test_utf8[4] = { 0xC2, 0xA9, 0xAF, 0xA2 };
	static const char test_ansi[4] = { 0x01, 0x10, 0x15, 0x77 };

	char * s = string;
	size_t i;
	const char* default_chars = (unicode) ? test_ansi : test_ansi;

	if (min)
		len = (unicode) ? 4 : 2;

	for (i = 0; i < len; i++) {
		*(s++) = default_chars[i % 4];
	}
	*s = 0;

	return string;
}

/**
* Verify minimum and maximum values for each 
* @ref ContainerFieldType.
*/
static char * testDataBaseValues(ContainerTemplate * tplt, bool min)
{
	char * buf;
	int i;

	estrCreate(&buf);
	estrPrintCharString(&buf, "Active 1\n");

	for(i=0; i<tplt->table_count; i++)
	{
		TableInfo * table = &tplt->tables[i];
		int j;

		for (j=0; j<table->num_columns; j++)
		{
			char tmp[16384];
			ColumnInfo * field = &table->columns[j];				
			if (field->reserved_word)
				continue;

			// HACK
			if (strcmp(field->name, "test_attr")==0)
			{
				estrConcatf(&buf, "%s \"%s\"\n", field->name, min ? "0" : "class_blaster");
				continue;
			}

			assert(sizeof(tmp) > field->column_size * 4 + 1);
			switch (field->data_type)
			{
				xcase CFTYPE_BYTE: estrConcatf(&buf, "%s %d\n", field->name, min ? 1 : UCHAR_MAX);
				xcase CFTYPE_SHORT: estrConcatf(&buf, "%s %d\n", field->name, min ? SHRT_MIN : SHRT_MAX);
				xcase CFTYPE_INT: estrConcatf(&buf, "%s %d\n", field->name, min ? INT_MIN : INT_MAX);
				xcase CFTYPE_FLOAT: estrConcatf(&buf, "%s %s\n", field->name, safe_ftoa(min ? -(FLT_EPSILON*10.0f) : FLT_MAX, tmp));
				xcase CFTYPE_DATETIME: {
						int time = min ? timerGetSecondsSince2000FromString("2001-01-01 00:00:00") : timerSecondsSince2000();
						estrConcatf(&buf, "%s \"%s\"\n", field->name, timerMakeDateStringFromSecondsSince2000(tmp, time));
				}
				xcase CFTYPE_STRING : {
					if (field->column_size == 0) {
						// Unbound length
						estrConcatf(&buf, "%s \"%s\"\n", field->name, testString(tmp, sizeof(tmp) - 1, true, min));
					} else {
						estrConcatf(&buf, "%s \"%s\"\n", field->name, testString(tmp, field->column_size - 1, true, min));
					}
				}
				xcase CFTYPE_BINARY: {
					char escaped[16384*2+1];
					int escaped_len = sizeof(escaped);
					int k;
					int len = min ? 1 : sizeof(tmp);

					for (k=0; k<len; k++) tmp[k]=k + 5;

					escapeDataToBuffer(escaped, tmp, len, &escaped_len);
					escaped[escaped_len] = 0;

					estrConcatf(&buf, "%s \"%s\"\n", field->name, escaped);
				}
				xdefault: assert(0);
			}
		}
	}

	return buf;
}

/** 
* Run a single test of SQL database types using the container
* text provided.
*/ 
static void testContainerLoadSave(struct DbList * list, int cid, DbContainer ** container, char * text)
{
	// Test update
	containerUpdate(list, cid, text, 1);
	assert(!containerDiff(text, containerGetText(*container))[0]);

	// Test reload
	containerUnload(list, cid);
	*container = (DbContainer*) containerLoad(list, cid);
	assert(*container);
	assert(!containerDiff(text, containerGetText(*container))[0]);

	// Test no change update
	containerUpdate(list, cid, text, 1);
	assert(!containerDiff(text, containerGetText(*container))[0]);
}

/**
* Verify that database types are working correctly on startup.
*/
void testDataBaseTypes(struct DbList * list)
{
	int i;
	char * text_empty = "Active 1";
	char * text_min = testDataBaseValues(list->tplt, 1);
	char * text_max = testDataBaseValues(list->tplt, 0);

	for (i=0; i<10; i++)
	{
		DbContainer * container = (DbContainer*) containerAlloc(list, -1);
		int cid = container->id;;

		testContainerLoadSave(list, cid, &container, text_empty);
		testContainerLoadSave(list, cid, &container, text_min);
		testContainerLoadSave(list, cid, &container, text_max);

		containerDelete(container);
	}

	estrDestroy(&text_min);
	estrDestroy(&text_max);

	loadend_printf("");
}

void split(std::vector<std::string>& vect, const std::string& s, char delim) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		vect.push_back(trim(item));
	}
}

// trim from start
static inline std::string& ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string& rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string& trim(std::string& s) {
	return ltrim(rtrim(s));
}
