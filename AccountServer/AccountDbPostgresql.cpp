#include <algorithm>
#include "AccountDbPostgresql.hpp"

std::string AccountDbPostgresql::createTemporaryTableProductQuery(const std::string& table) {
	return "CREATE TEMPORARY TABLE " + table + " (sku_id char(8), name varchar(128), product_type_id int, grant_limit int, expiration_seconds int);";
}

std::string AccountDbPostgresql::createTemporaryTableProductTypeQuery(const std::string& table) {
	return "CREATE TEMPORARY TABLE " + table + " (product_type_id int, name varchar(128));";
}

std::string AccountDbPostgresql::mergeBinsIntoProductQuery(const std::string& sourceTable) {
	return "CALL dbo.merge_products();";
}

std::string AccountDbPostgresql::mergeBinsIntoProductTypeQuery(const std::string& sourceTable) {
	return "CALL dbo.merge_product_types();";
}

void AccountDbPostgresql::formatCallStoredProcedure(std::string& proc) {
	proc.erase(std::remove(proc.begin(), proc.end(), '{'), proc.end());
	proc.erase(std::remove(proc.begin(), proc.end(), '}'), proc.end());
	proc.push_back(';');
}

std::string AccountDbPostgresql::temporaryTableName(const std::string& table) {
	return "tmp_" + table;
}