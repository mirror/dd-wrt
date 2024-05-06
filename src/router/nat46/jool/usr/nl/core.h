#ifndef SRC_USR_NL_CORE_H_
#define SRC_USR_NL_CORE_H_

#include <netlink/netlink.h>
#include "common/config.h"
#include "usr/util/result.h"

struct joolnl_socket {
	struct nl_sock *sk;
	xlator_type xt;
	int genl_family;
};

struct jool_result joolnl_setup(struct joolnl_socket *socket, xlator_type xt);
void joolnl_teardown(struct joolnl_socket *socket);

struct jool_result joolnl_alloc_msg(struct joolnl_socket *socket,
		char const *iname, enum joolnl_operation op, __u8 flags,
		struct nl_msg **out);

typedef struct jool_result (*joolnl_response_cb)(struct nl_msg *, void *);
struct jool_result joolnl_request(struct joolnl_socket *sk, struct nl_msg *msg,
		joolnl_response_cb cb, void *cb_arg);

struct jool_result validate_joolnlhdr(struct joolnlhdr *hdr, xlator_type xt);
struct jool_result joolnl_msg2result(struct nl_msg *response);

#endif /* SRC_USR_NL_CORE_H_ */
