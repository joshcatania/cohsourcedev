#include "container_tplt_utils.h"
#include "container.h"
#include "container_sql.h"
#include "utils.h"
#include "strings_opt.h"
#include "mathutil.h"
#include "StashTable.h"
#include "error.h"
#include "wininclude.h"
#include "sql/sqlconn.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <sqlext.h>
#include "ContainerDbMssql.hpp"

static int field_must_start_with_cr=1;

ContainerDb* gContainerDb = new ContainerDbMssql();

void setFieldRequiresCR(int yes)
{
	field_must_start_with_cr = yes;
}

char *findFieldText(char *data,char *field,char *buf)
{
char	*s,*str,searchname[100] = "\n";

	if (field_must_start_with_cr)
		strcpy(&searchname[1],field);
	else
		strcpy(&searchname[0],field);
	strcat(searchname," ");
	s = strstr(data,searchname);
	if (!s)
	{
		if (strncmp(data,field,strlen(field)) != 0)
			return 0;
		s = data-1;
	}
	str = s + strlen(searchname);
	if (*str == '"')
		str++;
	s = strchr(str,'\n');
	if (!s)
		s = str + strlen(str);
	if (s[-1] == '"')
		s--;
	strncpy(buf,str,s-str);
	buf[s-str] = 0;
	return buf;
}

int dataType(char *str, int *column_size, int *num_bytes, char **sql_type_name)
{
	static char static_sql_type_name[128];
	enum ContainerFieldType type = CFTYPE_NULL;
	int length = 0;
	char * type_name;
	char * length_str;
	ContainerFieldInfo typeMapping;

	type_name = strtok(str, "[");
	length_str = strtok(NULL, "]");

	type = gContainerDb->ormTypeToContainerFieldType(type_name);
	if (type == CFTYPE_NULL)
		FatalErrorf("No data type associated with orm type: %s\n", type_name);

	if (length_str)
		length = atoi(length_str);

	typeMapping = gContainerDb->getContainerFieldInfo(type);

	if (length == 0) {
		*column_size = 0;
		if (CFTYPE_IS_UNBOUNDABLE(type)) {
			*sql_type_name = typeMapping.db_unbound_type;
			*num_bytes = -1;
		} else {
			*sql_type_name = typeMapping.db_bound_type;
			*num_bytes = typeMapping.access_size;
		}
	} else {
		sprintf(static_sql_type_name, "%s(%d)", typeMapping.db_bound_type, length);
		*sql_type_name = static_sql_type_name;
		*column_size = length;
		*num_bytes = length * typeMapping.access_size;
	}
	return type;
}

void containerInitIndex(IndexedContainer *cont_lines,const char *data,int create)
{
	char				*str,*s;
	ContainerLine		*line;
	
	cont_lines->count = 0; //WAS: memset(cont_lines,0,sizeof(*cont_lines));
	dynArrayFit((void**) &cont_lines->data,1,&cont_lines->data_max,strlen(data)+1);
	strcpy(cont_lines->data,data);

	// preallocate some space
	dynArrayFit((void**) &cont_lines->lines, sizeof(ContainerLine), &cont_lines->max_lines, MAX_QUERY_RESULTS);
	
	if (create)
	{
		cont_lines->hash_table = stashTableCreateWithStringKeys(stashOptimalSize(MAX_QUERY_RESULTS), StashDeepCopyKeys);
	}
	else
		stashTableClear(cont_lines->hash_table);
	
	str = cont_lines->data;
	for(;;)
	{
		if (!str[0])
			break;
		s = strchr(str,' ');
		if (!s)
			break;
		*s = 0;

		dynArrayFit((void**) &cont_lines->lines, sizeof(ContainerLine), &cont_lines->max_lines, cont_lines->count+1);
		line = &cont_lines->lines[cont_lines->count++];
		line->field = str;
		line->value = s + 1;
		s = strchr(s+1,'\n');
		if (s)
		{
			if (s[-1] == '\r')
				s[-1] = 0;
			*s = 0;
			str = s + 1;
		}
		stashAddInt(cont_lines->hash_table,line->field,cont_lines->count-1, false);
		if (!s)
			break;
	}
}

void containerFreeIndex(IndexedContainer *cont)
{
	if (!cont->data)
		return;
	free(cont->data);
	cont->data = 0;
	stashTableDestroy(cont->hash_table);
}

int isNumber(char *s)
{
	if (isdigit(*s))
		return 1;
	if (*s == '-' && isdigit(s[1]))
		return 1;
	return 0;
}

int valueNotNull(char *s)
{
	if (isNumber(s))
	{
		for(;*s;s++)
		{
			if (*s > '0' && *s <= '9')
				return 1;
		}
	}
	else
	{
		if (stricmp(s,"\"nothing\"")==0 || stricmp(s,"nothing")==0 || stricmp(s,"\"none\"")==0 || stricmp(s,"none")==0 || strcmp(s,FAKE_ROW_PLACEHOLDER)==0)
			return 0;
		for(;*s;s++)
		{
			if (*s != '"')
				return 1;
		}
	}
	return 0;
}

int ctnrLineGet(CtnrLineState *state)
{
	char	*s,*s2;

	state->count = 0;
	while(!state->count)
	{
		if (!state->curr)
			return 0;
		state->count = tokenize_line(state->curr,state->args,&state->curr);
	}

	state->last_table_idx = state->table_idx;
	strcpy(state->last_table,state->table);
	state->field = state->args[0];
	state->value = state->args[1];
	s = strchr(state->args[0],'[');
	if (s)
	{
		*s++ = 0;
		s2 = strchr(s,']');
		*s2 = 0;
		state->idx_name = s;
		if (!isdigit((unsigned char)*s))
			state->table_idx = -2;
		else
			state->table_idx = atoi(s);
		state->field = 2 + s2;
		state->table = state->args[0];
	}
	return 1;
}

static char *ctnrLineBuf=0;
static int ctnrLineBuf_size=0;
static bool ctnrLineBuf_inuse=false;

void ctnrLineInit(CtnrLineState *state,char *data,char *table_name)
{
	int len=0;
	if (data)
		len = strlen(data)+1;

	assert(!ctnrLineBuf_inuse);
	ctnrLineBuf_inuse = true;

	memset(state,0,sizeof(*state));
	if (len > ctnrLineBuf_size) {
		ctnrLineBuf = (char*) realloc(ctnrLineBuf, MAX(len+512, 10*1024));
	}
	state->buf = ctnrLineBuf;
	memcpy(ctnrLineBuf, data, len);
	state->table = table_name;
	state->curr = state->buf;
	state->table_idx = -1;
}

void cntrLineFree(CtnrLineState *state)
{
	state->buf = 0;
	assert(ctnrLineBuf_inuse);
	ctnrLineBuf_inuse = false;
}

void checkEntCorrupted(char *data)
{
	static char	*trays[] = { "CurrentTray ","CurrentAltTray " };
	char			*s;
	int			i,val;

	if (!data)
		return;
	s = strstr(data,"\"(null)\"");
	assert(!s);
	for(s=data;*s;s++)
	{
		if (isprint(*s) || *s == '\n' || *s == '\r')
			continue;
		assert(0 && "container data corrupted");
	}
	for(i=0;i<ARRAY_SIZE(trays);i++)
	{
		s = strstr(data,trays[i]);
		if (s)
		{
			s += strlen(trays[i]);
			val = atoi(s);
			assert(val >=0 && val <= 9);
		}
	}
}

#ifdef DBSERVER
void odbcInitialSetup(void)
{
	int timeout;

	printf_stderr("SQL_GETDATA_EXTENSIONS = 0x%08x\n", sqlConnGetInfo(SQL_GETDATA_EXTENSIONS, false, SQLCONN_FOREGROUND));

	// Verify connection timeout
	timeout = sqlConnGetInfo(SQL_ATTR_QUERY_TIMEOUT, false, SQLCONN_FOREGROUND);
	assertmsgf(!timeout, "SQL server is set to timeout after %d seconds", timeout);
}

void bindInputParameter(HSTMT stmt, int index, enum ContainerFieldType type, const void * data, SQLLEN * size)
{
	SQLLEN no_data = SQL_NULL_DATA;
	ContainerFieldInfo info = gContainerDb->getContainerFieldInfo(type);
	sqlConnStmtBindParam(stmt, index+1, SQL_PARAM_INPUT, info.access_type, info.actual_type, 0, 0, cpp_const_cast(void*)(data), size ? *size : 0, (data) ? size : &no_data);
}

void bindOutputColumn(HSTMT stmt, int index, enum ContainerFieldType type, size_t size, void * data, SQLLEN * count) {
	sqlConnStmtBindCol(stmt, index+1, gContainerDb->getContainerFieldInfo(type).access_type, data, size, count);
}
#endif
