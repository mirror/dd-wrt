#ifndef SRC_USR_NL_SESSION_H_
#define SRC_USR_NL_SESSION_H_

#include "common/config.h"
#include "usr/nl/core.h"

/**
 * A session entry, from the eyes of userspace.
 *
 * It's a stripped version of "struct session_entry" and only used when sessions
 * need to travel to userspace. For anything else, use "struct session_entry".
 *
 * See "struct session_entry" for documentation on the fields.
 */
struct session_entry_usr {
	struct ipv6_transport_addr src6;
	struct ipv6_transport_addr dst6;
	struct ipv4_transport_addr src4;
	struct ipv4_transport_addr dst4;
	__u8 proto;
	__u8 state;
	__u32 dying_time;
};

typedef struct jool_result (*joolnl_session_foreach_cb)(
	struct session_entry_usr const *entry, void *args
);

struct jool_result joolnl_session_foreach(
	struct joolnl_socket *sk,
	char const *iname,
	l4_protocol proto,
	joolnl_session_foreach_cb cb,
	void *args
);

#endif /* SRC_USR_NL_SESSION_H_ */
