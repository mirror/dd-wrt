#ifndef SRC_MOD_NAT64_BIB_DB_H_
#define SRC_MOD_NAT64_BIB_DB_H_

/**
 * @file
 * The BIB and session tables.
 * Formally defined in RFC 6146 section 3.2.
 */

#include "common/config.h"
#include "mod/common/packet.h"
#include "mod/common/translation_state.h"
#include "mod/common/db/pool4/db.h"
#include "mod/common/db/bib/entry.h"

struct bib;

enum session_fate {
	/**
	 * Assign the established timer to the session.
	 * (An "established" timer is one where it is assumed the session is
	 * not going to die soon.)
	 */
	FATE_TIMER_EST,
	/**
	 * Assign the transitory timer to the session.
	 * (A "transitory" timer is one where it is assumed the session is
	 * going to die soon.)
	 */
	FATE_TIMER_TRANS,
	/**
	 * The session expired; it has to be removed from the DB right away.
	 */
	FATE_RM,
	/**
	 * No changes.
	 */
	FATE_PRESERVE,
	/**
	 * Send a probe packet, then reset timer into transitory mode.
	 */
	FATE_PROBE,
	/**
	 * Drop the packet (the new packet, not the stored one),
	 * preserve the session (stored packet included).
	 */
	FATE_DROP,

	/**
	 * Like FATE_TIMER_EST or FATE_TIMER_TRANS, except the session's
	 * lifetime must not be reset.
	 * It's called "slow" because this means the database cannot just add
	 * the session to the end of the sorted (by expiration date) list and so
	 * the proper slot has to be found in a sequential search.
	 */
	FATE_TIMER_SLOW,
};

/* bib_setup() not needed. */
void bib_teardown(void);

struct bib *bib_alloc(void);
void bib_get(struct bib *db);
void bib_put(struct bib *db);

typedef enum session_fate (*fate_cb)(struct session_entry *, void *);

struct collision_cb {
	/**
	 * Note: This callback can edit the session's state, timer_type,
	 * update_time, and also turn off has_stored.
	 * Any other changes will be rolled back.
	 */
	fate_cb cb;
	void *arg;
};

/* These are used by Filtering. */

int bib_add6(struct xlation *state,
		struct mask_domain *masks,
		struct tuple *tuple6,
		struct ipv4_transport_addr *dst4);
int bib_add4(struct xlation *state,
		struct ipv6_transport_addr *dst6,
		struct tuple *tuple4);
verdict bib_add_tcp6(struct xlation *xstate,
		struct mask_domain *masks,
		struct ipv4_transport_addr *dst4,
		struct collision_cb *cb);
verdict bib_add_tcp4(struct xlation *xstate,
		struct ipv6_transport_addr *dst6,
		struct collision_cb *cb);

/* These are used by other kernel submodules. */

int bib_find(struct bib *db, struct tuple *tuple,
		struct bib_session *result);
int bib_add_session(struct xlator *jool, struct session_entry *new,
		struct collision_cb *cb);
void bib_clean(struct xlator *jool);

/* These are used by userspace request handling. */

typedef int (*bib_foreach_entry_cb)(struct bib_entry const *, void *);
typedef int (*session_foreach_entry_cb)(struct session_entry const *, void *);

struct session_foreach_offset {
	struct taddr4_tuple offset;
	bool include_offset;
};

int bib_foreach(struct bib *db, l4_protocol proto,
		bib_foreach_entry_cb cb, void *cb_arg,
		const struct ipv4_transport_addr *offset);
int bib_foreach_session(struct xlator *jool, l4_protocol proto,
		session_foreach_entry_cb cb, void *cb_arg,
		struct session_foreach_offset *offset);
int bib_find6(struct bib *db, l4_protocol proto,
		struct ipv6_transport_addr *addr,
		struct bib_entry *result);
int bib_find4(struct bib *db, l4_protocol proto,
		struct ipv4_transport_addr *addr,
		struct bib_entry *result);
int bib_add_static(struct xlator *jool, struct bib_entry *new);
int bib_rm(struct xlator *jool, struct bib_entry *entry);
void bib_rm_range(struct xlator *jool, l4_protocol proto,
		struct ipv4_range *range);
void bib_flush(struct xlator *jool);

void bib_print(struct bib *db);

/* The user of this module has to implement this. */
enum session_fate tcp_est_expire_cb(struct session_entry *new, void *arg);

#endif /* SRC_MOD_NAT64_BIB_DB_H_ */
