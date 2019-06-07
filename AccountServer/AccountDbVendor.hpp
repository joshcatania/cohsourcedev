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
	virtual std::string mergeBinsIntoProductQuery(const std::string& sourceTable) = 0;
	virtual std::string mergeBinsIntoProductTypeQuery(const std::string& sourceTable) = 0;
	void formatCallStoredProcedure(std::string& storedProcedure);
	virtual std::string temporaryTableName(const std::string& table) = 0;
};

#endif