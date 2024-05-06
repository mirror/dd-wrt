#ifndef SRC_USR_NL_POOL4_H_
#define SRC_USR_NL_POOL4_H_

#include "common/config.h"
#include "usr/nl/core.h"

typedef struct jool_result (*joolnl_pool4_foreach_cb)(
	struct pool4_entry const *entry, void *args
);

struct jool_result joolnl_pool4_foreach(
	struct joolnl_socket *sk,
	char const *iname,
	l4_protocol proto,
	joolnl_pool4_foreach_cb cb,
	void *args
);

struct jool_result joolnl_pool4_add(
	struct joolnl_socket *sk,
	char const *iname,
	struct pool4_entry const *entry
);

struct jool_result joolnl_pool4_rm(
	struct joolnl_socket *sk,
	char const *iname,
	struct pool4_entry const *entry,
	bool quick
);

struct jool_result joolnl_pool4_flush(
	struct joolnl_socket *sk,
	char const *iname,
	bool quick
);

#endif /* SRC_USR_NL_POOL4_H_ */
