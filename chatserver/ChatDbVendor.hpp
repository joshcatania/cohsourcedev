#ifndef CHATSERVER_SQL_CHATDBVENDOR_HPP
#define CHATSERVER_SQL_CHATDBVENDOR_HPP

#ifdef __cplusplus

#include <errno.h>
#include <string>

// For vendor specific database functions
// ChatServer is very db agnostic to begin with so instead of separating this into separate
// classes like the other servers, the functions are being put under a namespace with
// the DatabaseProvider global parameter exposed and used to distinguish db vendors.

namespace ChatDbVendor {
	extern void transformCallStoredProcedure(std::string& proc);
}

#endif

#endif