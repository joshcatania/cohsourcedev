#include "AccountDbVendor.hpp"

class AccountDbPostgresql : public AccountDbVendor
{
public:
	AccountDbPostgresql() {}
	~AccountDbPostgresql() {}

	std::string createTemporaryTableProductQuery(const std::string& table);
	std::string createTemporaryTableProductTypeQuery(const std::string& table);
	SQLRETURN mergeBinsIntoProduct();
	SQLRETURN mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows);
	void dropTemporaryTableProduct();
	void dropTemporaryTableProductType();
	void formatCallStoredProcedure(std::string& storedProcedure);
	std::string temporaryTableName(const std::string& table);
};