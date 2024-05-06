#ifndef SRC_USR_NL_COMMON_H_
#define SRC_USR_NL_COMMON_H_

#include <netlink/msg.h>
#include "usr/util/result.h"

struct jool_result joolnl_err_msgsize(void);

struct jool_result joolnl_init_foreach(struct nl_msg *response, bool *done);
struct jool_result joolnl_init_foreach_list(struct nl_msg *msg,
		char const *what, bool *done);

#endif /* SRC_USR_NL_COMMON_H_ */
