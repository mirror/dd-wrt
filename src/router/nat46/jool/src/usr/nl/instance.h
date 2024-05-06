#ifndef SRC_USR_NL_INSTANCE_H_
#define SRC_USR_NL_INSTANCE_H_

#include "common/config.h"
#include "usr/nl/core.h"

typedef struct jool_result (*instance_foreach_cb)(
	struct instance_entry_usr const *entry, void *args
);

struct jool_result joolnl_instance_foreach(
	struct joolnl_socket *sk,
	instance_foreach_cb cb,
	void *args
);

struct jool_result joolnl_instance_hello(
	struct joolnl_socket *sk,
	char const *iname,
	enum instance_hello_status *status
);

struct jool_result joolnl_instance_add(
	struct joolnl_socket *sk,
	xlator_framework xf,
	char const *iname,
	struct ipv6_prefix const *pool6
);

struct jool_result joolnl_instance_rm(
	struct joolnl_socket *sk,
	char const *iname
);

struct jool_result joolnl_instance_flush(
	struct joolnl_socket *sk
);

#endif /* SRC_USR_NL_INSTANCE_H_ */
