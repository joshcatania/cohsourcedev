#include "AccountDbVendor.hpp"

class AccountDbMssql : public AccountDbVendor
{
public:
	AccountDbMssql() {}
	~AccountDbMssql() {}

	std::string createTemporaryTableProductQuery(const std::string& table);
	std::string createTemporaryTableProductTypeQuery(const std::string& table);
	std::string mergeBinsIntoProductQuery(const std::string& sourceTable);
	std::string mergeBinsIntoProductTypeQuery(const std::string& sourceTable);
	std::string temporaryTableName(const std::string& table);
};