#include "AccountSqlVendor.hpp"

char* AccountSqlPostgresql::createTemporaryTableProduct() {
	return "CREATE TEMPORARY TABLE tmp_product (sku_id char(8), name varchar(128), product_type_id int, grant_limit int, expiration_seconds int);";
}

char* AccountSqlPostgresql::createTemporaryTableProductType() {
	return "CREATE TEMPORARY TABLE tmp_product_type(product_type_id int, name varchar(128));";
}

SQLRETURN AccountSqlPostgresql::mergeBinsIntoProduct() {
	return 1;
}

static SQLCHAR CreateTempTableProductType[] = 
	"CREATE TEMPORARY TABLE tmp_product_type(product_type_id int, name varchar(128));";
static SQLCHAR QueryInsertIntoProductType[] = "INSERT INTO tmp_product_type VALUES(? , ? );";

SQLRETURN AccountSqlPostgresql::mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows) {
	SQLRETURN ret = 0;
	HSTMT stmt;

	//sql.
	
	SQLAllocHandle(SQL_HANDLE_STMT, conn, &stmt);

	//ret = sqlConnStmtExecDirect(stmt, CreateTempTableProductType, SQL_NTS, SQLCONN_FOREGROUND, false);

	if (SQL_SUCCEEDED(ret)) {
		SQLPrepare(stmt, QueryInsertIntoProductType, SQL_NTS);
		sqlConnStmtBindParamArray(stmt, num_rows, SQL_PARAM_BIND_BY_COLUMN);
		sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, product_type_ids, sizeof(*product_type_ids), NULL);
//		sqlConnStmtBindParam(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, ARRAYSIZE(names), 0, names, sizeof(*names), name_bytes);

		ret = _sqlConnStmtExecute(stmt, SQLCONN_FOREGROUND);

		sqlConnStmtBindParamArray(stmt, 1, SQL_PARAM_BIND_BY_COLUMN);
		sqlConnStmtUnbindParams(stmt);
	}

	if (SQL_SUCCEEDED(ret)) {
		ret = sqlConnStmtExecDirect(stmt, "CALL dbo.merge_product_types();", SQL_NTS, SQLCONN_FOREGROUND, false);
	}
	sqlConnStmtFree(stmt);
	return ret;
}
void AccountSqlPostgresql::dropTemporaryTableProduct() {

}
void AccountSqlPostgresql::dropTemporaryTableProductType() {

}

//static SQLCHAR insert_query[] = SQLCHAR insert_query[];