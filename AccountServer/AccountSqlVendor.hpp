#ifndef ACCOUNT_SQL_VENDOR_HPP
#define ACCOUNT_SQL_VENDOR_HPP

//#include "sqltypes.h"
#include "sql/sqlinclude.h"
#include "sql/sqlconn.h"

class AccountSqlVendor
{

public:
	AccountSqlVendor() {}
	virtual ~AccountSqlVendor() {}

	virtual char* createTemporaryTableProduct() = 0;
	virtual char* createTemporaryTableProductType() = 0;
	virtual SQLRETURN mergeBinsIntoProduct() = 0;
	virtual SQLRETURN mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows) = 0;
	virtual void dropTemporaryTableProduct() = 0;
	virtual void dropTemporaryTableProductType() = 0;
};

class AccountSqlPostgresql : public AccountSqlVendor
{
public:
	AccountSqlPostgresql() {}
	~AccountSqlPostgresql() {}

	char* createTemporaryTableProduct();
	char* createTemporaryTableProductType();
	SQLRETURN mergeBinsIntoProduct();
	SQLRETURN mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows);
	void dropTemporaryTableProduct();
	void dropTemporaryTableProductType();

};

class AccountSqlMssql : public AccountSqlVendor
{
public:
	AccountSqlMssql() {}
	~AccountSqlMssql() {}

	char* createTemporaryTableProduct();
	char* createTemporaryTableProductType();
	SQLRETURN mergeBinsIntoProduct();
	SQLRETURN mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows);
	void dropTemporaryTableProduct();
	void dropTemporaryTableProductType();
};

#endif