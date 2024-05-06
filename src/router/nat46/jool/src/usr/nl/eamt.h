#ifndef SRC_USR_NL_EAMT_H_
#define SRC_USR_NL_EAMT_H_

#include "common/config.h"
#include "usr/nl/core.h"

typedef struct jool_result (*joolnl_eamt_foreach_cb)(
	struct eamt_entry const *entry, void *args
);

struct jool_result joolnl_eamt_foreach(
	struct joolnl_socket *sk,
	char const *iname,
	joolnl_eamt_foreach_cb cb,
	void *args
);

struct jool_result joolnl_eamt_add(
	struct joolnl_socket *sk,
	char const *iname,
	struct ipv6_prefix const *p6,
	struct ipv4_prefix const *p4,
	bool force
);

struct jool_result joolnl_eamt_rm(
	struct joolnl_socket *sk,
	char const *iname,
	struct ipv6_prefix const *p6,
	struct ipv4_prefix const *p4
);

struct jool_result joolnl_eamt_flush(
	struct joolnl_socket *sk,
	char const *iname
);

#endif /* SRC_USR_NL_EAMT_H_ */
