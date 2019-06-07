#include "AccountDbVendor.hpp"

class AccountDbPostgresql : public AccountDbVendor
{
public:
	AccountDbPostgresql() {}
	~AccountDbPostgresql() {}

	std::string createTemporaryTableProductQuery(const std::string& table);
	std::string createTemporaryTableProductTypeQuery(const std::string& table);
	std::string mergeBinsIntoProductQuery(const std::string& sourceTable);
	std::string mergeBinsIntoProductTypeQuery(const std::string& sourceTable);
	void formatCallStoredProcedure(std::string& storedProcedure);
	std::string temporaryTableName(const std::string& table);
};