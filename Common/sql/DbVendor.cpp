#include "DbVendor.hpp"
#include <algorithm>

// Copied from AccountServer AccountDbPostgresql
void DbVendor::transformCallStoredProcedure(std::string& proc) {
	if (gDatabaseProvider == DBPROV_POSTGRESQL) {
		proc.erase(std::remove(proc.begin(), proc.end(), '{'), proc.end());
		proc.erase(std::remove(proc.begin(), proc.end(), '}'), proc.end());
		proc.push_back(';');
	}
}

DatabaseProvider DbVendor::fromConfigString(const std::string& config) {
	std::string configLowercase = config;
	std::transform(configLowercase.begin(), configLowercase.end(), configLowercase.begin(), ::tolower);
	if (configLowercase.compare("postgresql") == 0)
		return DBPROV_POSTGRESQL;
	else if (configLowercase.compare("mssql") == 0)
		return DBPROV_MSSQL;
	return DBPROV_UNKNOWN;
}
