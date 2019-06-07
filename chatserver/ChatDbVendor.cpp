#include "ChatDbVendor.hpp"
#include "shardnet.h"
#include <algorithm>

// Copied from AccountServer AccountDbPostgresql
void ChatDbVendor::transformCallStoredProcedure(std::string& proc) {
	if (gDatabaseProvider == DBPROV_POSTGRESQL) {
		proc.erase(std::remove(proc.begin(), proc.end(), '{'), proc.end());
		proc.erase(std::remove(proc.begin(), proc.end(), '}'), proc.end());
		proc.push_back(';');
	}
}