#ifndef SRC_MOD_NAT64_BIB_ENTRY_H_
#define SRC_MOD_NAT64_BIB_ENTRY_H_

#include "common/session.h"
#include "common/types.h"

typedef enum session_timer_type {
	SESSION_TIMER_EST,
	SESSION_TIMER_TRANS,
	SESSION_TIMER_SYN4,
} session_timer_type;

/**
 * An IPv6 connection and the IPv4 version of it once translated.
 *
 * This is a codensed/public version of the structure that is actually stored in
 * the database.
 *
 * Please note that modifications to this structure may need to cascade to
 * "struct session_entry_usr".
 */
struct session_entry {
	/**
	 * IPv6 version of the connection.
	 *
	 * "src" and "dst" are inherited names from the RFC. They are
	 * unfortunate, as they only make sense in the 6-to-4 direction.
	 *
	 * @src6 is the remote IPv6 node's transport address.
	 * @dst6 is the address the NAT64 is using to mask the IPv4 endpoint.
	 */
	struct ipv6_transport_addr src6;
	struct ipv6_transport_addr dst6;

	/**
	 * IPv4 version of the connection.
	 *
	 * "src" and "dst" are inherited names from the RFC. They are
	 * unfortunate, as they only make sense in the 6-to-4 direction.
	 *
	 * @src4 is the address the NAT64 is using to mask the IPv6 endpoint.
	 * @dst4 is the remote IPv4 node's transport address.
	 */
	struct ipv4_transport_addr src4;
	struct ipv4_transport_addr dst4;

	/** Transport protocol of the connection this session describes. */
	l4_protocol proto;
	/** Current TCP SM state. Only relevant if @l4_proto == L4PROTO_TCP. */
	tcp_state state;
	/** An indicator of the timer that is going to expire this session. */
	session_timer_type timer_type;

	/** Jiffy (from the epoch) this session was last updated/used. */
	unsigned long update_time;
	/*
	 * Number of jiffies before this session is to be downgraded. (Either
	 * deleted or changed into a transitory state.)
	 */
	unsigned long timeout;

	bool has_stored;
};

/* Session Entry Printk Pattern */
#define SEPP "[" TA6PP ", " TA6PP ", " TA4PP ", " TA4PP ", %s]"
/* Session Entry Printk Arguments */
#define SEPA(s) \
	TA6PA((s)->src6), TA6PA((s)->dst6), \
	TA4PA((s)->src4), TA4PA((s)->dst4), \
	l4proto_to_string((s)->proto)

struct bib_session {
	/** Are @session.src6, @session.src4, @session.proto set? */
	bool bib_set;
	/**
	 * Are all of @session's fields set?
	 * (@session_set true implies @bib_set true.)
	 */
	bool session_set;
	struct session_entry session;
};

bool session_equals(const struct session_entry *s1,
		const struct session_entry *s2);

#endif /* SRC_MOD_NAT64_BIB_ENTRY_H_ */
