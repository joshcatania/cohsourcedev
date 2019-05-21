#include "AccountSqlVendor.hpp"

char* AccountSqlMssql::createTemporaryTableProduct() {
	return "CREATE TABLE #tmp_product (sku_id char(8), name varchar(128), product_type_id int, grant_limit int, expiration_seconds int);";
}

char* AccountSqlMssql::createTemporaryTableProductType() {
	return "CREATE TABLE #tmp_product_type(product_type_id int, name varchar(128));";
}

SQLRETURN AccountSqlMssql::mergeBinsIntoProduct() {
	return 1;
	//return "MERGE INTO product AS target " \
	//	"USING ? AS source " \
	//	"ON target.sku_id = source.sku_id " \
	//	"WHEN MATCHED THEN UPDATE SET name = source.name, product_type_id = source.product_type_id, grant_limit = source.grant_limit, expiration_seconds = source.expiration_seconds "\
	//	"WHEN NOT MATCHED BY TARGET THEN INSERT (sku_id, name, product_type_id, grant_limit, expiration_seconds) VALUES (source.sku_id, source.name, source.product_type_id, source.grant_limit, source.expiration_seconds) " \
	//	"WHEN NOT MATCHED BY SOURCE THEN DELETE;";
}

SQLRETURN AccountSqlMssql::mergeBinsIntoProductType(HDBC conn, char** product_type_ids, char** names, int num_rows) {
	return 1;
	//return "MERGE INTO product_type AS target " \
	//	"USING ? AS source " \
	//	"ON target.product_type_id = source.product_type_id " \
	//	"WHEN MATCHED THEN UPDATE SET name = source.name " \
	//	"WHEN NOT MATCHED BY TARGET THEN INSERT (product_type_id, name) VALUES (source.product_type_id, source.name) " \
	//	"WHEN NOT MATCHED BY SOURCE THEN DELETE;";
}

void AccountSqlMssql::dropTemporaryTableProduct() {

}
void AccountSqlMssql::dropTemporaryTableProductType() {

}



