#ifndef SRC_MOD_COMMON_XLATOR_H_
#define SRC_MOD_COMMON_XLATOR_H_

#include "common/config.h"
#include "mod/common/stats.h"
#include "mod/common/types.h"

/**
 * A Jool translator "instance". The point is that each network namespace has
 * a separate instance (if Jool has been loaded there).
 *
 * The instance holds all the databases and configuration the translating code
 * should use to handle a packet in the respective namespace.
 */
struct xlator {
	/*
	 * Note: This xlator must not increase @ns's kref counter.
	 * Quite the opposite: The @ns has to reference count the xlator.
	 * (The kernel does this somewhere in the register_pernet_subsys() API.)
	 * Otherwise the Jool instance would prevent the namespace from dying,
	 * and in turn the namespace would prevent the Jool instance from dying.
	 *
	 * As a matter of fact, I'll say it here because there's no better place
	 * for it: whenever Jool acquires a reference to a namespace, it should
	 * *ALWAYS* return it, preferably during the same function.
	 *
	 * TODO (NOW) what about older kernels? What about the atomic config
	 * instances?
	 */
	struct net *ns;
	char iname[INAME_MAX_SIZE];
	xlator_flags flags;

	struct jool_stats *stats;
	struct jool_globals globals;
	union {
		struct {
			struct eam_table *eamt;
			struct addr4_pool *denylist4;
		} siit;
		struct {
			struct pool4 *pool4;
			struct bib *bib;
			struct joold_queue *joold;
		} nat64;
	};

	bool (*is_hairpin)(struct xlation *);
	verdict (*handling_hairpinning)(struct xlation *);
};

/* User context (reads and writes) */

int xlator_setup(void);
void xlator_set_defrag(void (*defrag_enable)(struct net *ns));
void xlator_teardown(void);

int xlator_add(xlator_flags flags, char *iname, struct ipv6_prefix *pool6,
		struct xlator *result);
int xlator_rm(xlator_type xt, char *iname);
int xlator_flush(xlator_type xt);
void jool_xlator_flush_net(struct net *ns, xlator_type xt);
void jool_xlator_flush_batch(struct list_head *net_exit_list, xlator_type xt);

int xlator_init(struct xlator *jool, struct net *ns, char *iname,
		xlator_flags flags, struct ipv6_prefix *pool6);
int xlator_replace(struct xlator *jool);

/* Any context (reads) */

int xlator_find(struct net *ns, xlator_flags flags, const char *iname,
		struct xlator *result);
int xlator_find_current(const char *iname, xlator_flags flags,
		struct xlator *result);
int xlator_find_netfilter(struct net *ns, struct xlator *result);
void xlator_put(struct xlator *instance);

typedef int (*xlator_foreach_cb)(struct xlator *, void *);
int xlator_foreach(xlator_type xt, xlator_foreach_cb cb, void *args,
		struct instance_entry_usr *offset);

xlator_type xlator_get_type(struct xlator const *instance);
xlator_framework xlator_get_framework(struct xlator const *instance);

static inline bool xlator_is_siit(struct xlator const *instance)
{
	return instance->flags & XT_SIIT;
}

static inline bool xlator_is_nat64(struct xlator const *instance)
{
	return instance->flags & XT_NAT64;
}

static inline bool xlator_is_netfilter(struct xlator const *instance)
{
	return instance->flags & XF_NETFILTER;
}

static inline bool xlator_is_iptables(struct xlator const *instance)
{
	return instance->flags & XF_IPTABLES;
}


#endif /* SRC_MOD_COMMON_XLATOR_H_ */
