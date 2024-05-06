#ifndef SRC_USR_NL_DENYLIST_H_
#define SRC_USR_NL_DENYLIST_H_

#include "common/types.h"
#include "usr/nl/core.h"

typedef struct jool_result (*joolnl_denylist4_foreach_cb)(
	struct ipv4_prefix const *entry, void *args
);

struct jool_result joolnl_denylist4_foreach(
	struct joolnl_socket *sk,
	char const *iname,
	joolnl_denylist4_foreach_cb cb,
	void *args
);

struct jool_result joolnl_denylist4_add(
	struct joolnl_socket *sk,
	char const *iname,
	struct ipv4_prefix const *addrs,
	bool force
);

struct jool_result joolnl_denylist4_rm(
	struct joolnl_socket *sk,
	char const *iname,
	struct ipv4_prefix const *addrs
);

struct jool_result joolnl_denylist4_flush(
	struct joolnl_socket *sk,
	char const *iname
);

#endif /* SRC_USR_NL_DENYLIST_H_ */
