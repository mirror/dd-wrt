#ifndef SRC_USR_NL_ADDRESS_H_
#define SRC_USR_NL_ADDRESS_H_

#include "usr/nl/core.h"

struct jool_result joolnl_address_query64(
	struct joolnl_socket *sk,
	char const *iname,
	struct in6_addr const *addr,
	struct result_addrxlat64 *result
);

struct jool_result joolnl_address_query46(
	struct joolnl_socket *sk,
	char const *iname,
	struct in_addr const *addr,
	struct result_addrxlat46 *result
);

#endif /* SRC_USR_NL_ADDRESS_H_ */
