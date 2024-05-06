#ifndef SRC_USR_NL_BIB_H_
#define SRC_USR_NL_BIB_H_

#include "common/config.h"
#include "usr/nl/core.h"

typedef struct jool_result (*joolnl_bib_foreach_cb)(
	struct bib_entry const *entry, void *args
);

struct jool_result joolnl_bib_foreach(
	struct joolnl_socket *sk,
	char const *iname,
	l4_protocol proto,
	joolnl_bib_foreach_cb cb,
	void *args
);

struct jool_result joolnl_bib_add(
	struct joolnl_socket *sk,
	char const *iname,
	struct ipv6_transport_addr const *a6,
	struct ipv4_transport_addr const *a4,
	l4_protocol proto
);

struct jool_result joolnl_bib_rm(
	struct joolnl_socket *sk,
	char const *iname,
	struct ipv6_transport_addr const *a6,
	struct ipv4_transport_addr const *a4,
	l4_protocol proto
);

#endif /* SRC_USR_NL_BIB_H_ */
