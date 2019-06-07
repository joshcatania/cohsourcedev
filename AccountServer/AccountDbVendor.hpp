#ifndef ACCOUNT_SQL_VENDOR_HPP
#define ACCOUNT_SQL_VENDOR_HPP

//#include "sqltypes.h"
#include "sql/sqlinclude.h"
#include "sql/sqlconn.h"
#include <string>

class AccountDbVendor {

public:
	AccountDbVendor() {}
	virtual ~AccountDbVendor() {}

	virtual std::string createTemporaryTableProductQuery(const std::string& table) = 0;
	virtual std::string createTemporaryTableProductTypeQuery(const std::string& table) = 0;
	virtual SQLRETURN mergeBinsIntoProduct() = 0;
	virtual SQLRETURN mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows) = 0;
	virtual void dropTemporaryTableProduct() = 0;
	virtual void dropTemporaryTableProductType() = 0;
	void formatCallStoredProcedure(std::string& storedProcedure);
	virtual std::string temporaryTableName(const std::string& table) = 0;
};

#endif