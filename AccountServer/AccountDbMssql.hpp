#include "AccountDbVendor.hpp"

class AccountDbMssql : public AccountDbVendor
{
public:
	AccountDbMssql() {}
	~AccountDbMssql() {}

	std::string createTemporaryTableProductQuery(const std::string& table);
	std::string createTemporaryTableProductTypeQuery(const std::string& table);
	SQLRETURN mergeBinsIntoProduct();
	SQLRETURN mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows);
	void dropTemporaryTableProduct();
	void dropTemporaryTableProductType();
	std::string temporaryTableName(const std::string& table);
};