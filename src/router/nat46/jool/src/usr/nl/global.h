#ifndef SRC_USR_NL_GLOBAL_H_
#define SRC_USR_NL_GLOBAL_H_

#include "common/global.h"
#include "usr/nl/core.h"

typedef struct jool_result (*joolnl_global_foreach_cb)(
	struct joolnl_global_meta const *metadata, void *value, void *args
);

struct jool_result joolnl_global_foreach(
	struct joolnl_socket *sk,
	char const *iname,
	joolnl_global_foreach_cb cb,
	void *args
);

struct jool_result joolnl_global_update(
	struct joolnl_socket *sk,
	char const *iname,
	struct joolnl_global_meta const *meta,
	char const *value,
	bool force
);

#endif /* SRC_USR_NL_GLOBAL_H_ */
