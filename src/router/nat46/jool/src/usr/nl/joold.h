#ifndef SRC_USR_NL_JOOLD_H_
#define SRC_USR_NL_JOOLD_H_

#include "usr/nl/core.h"

struct jool_result joolnl_joold_add(
	struct joolnl_socket *sk,
	char const *iname,
	void const *data,
	size_t data_len
);

struct jool_result joolnl_joold_advertise(
	struct joolnl_socket *sk,
	char const *iname
);

struct jool_result joolnl_joold_ack(
	struct joolnl_socket *sk,
	char const *iname
);

#endif /* SRC_USR_NL_JOOLD_H_ */
