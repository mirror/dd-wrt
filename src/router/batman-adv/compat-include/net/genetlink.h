/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_
#define _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_

#include <linux/version.h>
#include_next <net/genetlink.h>

#if LINUX_VERSION_IS_LESS(4, 15, 0)

static inline
void batadv_genl_dump_check_consistent(struct netlink_callback *cb,
				       void *user_hdr)
{
	struct genl_family genl_family = {
		.hdrsize = 0,
	};

	genl_dump_check_consistent(cb, user_hdr, &genl_family);
}

#define genl_dump_check_consistent batadv_genl_dump_check_consistent

#endif /* LINUX_VERSION_IS_LESS(4, 15, 0) */


#if LINUX_VERSION_IS_LESS(5, 10, 0)

#if LINUX_VERSION_IS_LESS(5, 2, 0)
enum genl_validate_flags {
	GENL_DONT_VALIDATE_STRICT		= BIT(0),
	GENL_DONT_VALIDATE_DUMP			= BIT(1),
	GENL_DONT_VALIDATE_DUMP_STRICT		= BIT(2),
};
#endif /* LINUX_VERSION_IS_LESS(5, 2, 0) */

struct batadv_genl_small_ops {
	int		       (*doit)(struct sk_buff *skb,
				       struct genl_info *info);
	int		       (*dumpit)(struct sk_buff *skb,
					 struct netlink_callback *cb);
	int		       (*done)(struct netlink_callback *cb);
	u8			cmd;
	u8			internal_flags;
	u8			flags;
	u8			validate;
};

struct batadv_genl_family {
	/* data handled by the actual kernel */
	struct genl_family family;

	/* data which has to be copied to family by
	 * batadv_genl_register_family
	 */
	unsigned int hdrsize;
	char name[GENL_NAMSIZ];
	unsigned int version;
	unsigned int maxattr;
	const struct nla_policy *policy;
	bool netnsok;
        int  (*pre_doit)(const struct genl_ops *ops, struct sk_buff *skb,
			 struct genl_info *info);
        void (*post_doit)(const struct genl_ops *ops, struct sk_buff *skb,
			  struct genl_info *info);
	const struct batadv_genl_small_ops *small_ops;
	const struct genl_multicast_group *mcgrps;
	unsigned int n_small_ops;
	unsigned int n_mcgrps;
	struct module *module;

	/* allocated by batadv_genl_register_family and free'd by
	 * batadv_genl_unregister_family. Used to modify the usually read-only
	 * ops
	 */
	struct genl_ops *copy_ops;
};

static inline int batadv_genl_register_family(struct batadv_genl_family *family)
{
	struct genl_ops *ops;
	unsigned int i;

	family->family.hdrsize = family->hdrsize;
	strncpy(family->family.name, family->name, sizeof(family->family.name));
	family->family.version = family->version;
	family->family.maxattr = family->maxattr;
	family->family.netnsok = family->netnsok;
	family->family.pre_doit = family->pre_doit;
	family->family.post_doit = family->post_doit;
	family->family.mcgrps = family->mcgrps;
	family->family.n_ops = family->n_small_ops;
	family->family.n_mcgrps = family->n_mcgrps;
	family->family.module = family->module;

	ops = kzalloc(sizeof(*ops) * family->n_small_ops, GFP_KERNEL);
	if (!ops)
		return -ENOMEM;

	for (i = 0; i < family->family.n_ops; i++) {
		ops[i].doit = family->small_ops[i].doit;
		ops[i].dumpit = family->small_ops[i].dumpit;
		ops[i].done = family->small_ops[i].done;
		ops[i].cmd = family->small_ops[i].cmd;
		ops[i].internal_flags = family->small_ops[i].internal_flags;
		ops[i].flags = family->small_ops[i].flags;
#if LINUX_VERSION_IS_GEQ(5, 2, 0)
		ops[i].validate = family->small_ops[i].validate;
#else
		ops[i].policy = family->policy;
#endif
	}

#if LINUX_VERSION_IS_GEQ(5, 2, 0)
	family->family.policy = family->policy;
#endif

	family->family.ops = ops;
	family->copy_ops = ops;

	return genl_register_family(&family->family);
}

typedef struct genl_ops batadv_genl_ops_old;

#define batadv_pre_doit(__x, __y, __z) \
	batadv_pre_doit(const batadv_genl_ops_old *ops, __y, __z)

#define batadv_post_doit(__x, __y, __z) \
	batadv_post_doit(const batadv_genl_ops_old *ops, __y, __z)

#define genl_small_ops batadv_genl_small_ops
#define genl_family batadv_genl_family

#define genl_register_family(family) \
	batadv_genl_register_family((family))

static inline void
batadv_genl_unregister_family(struct batadv_genl_family *family)
{

	genl_unregister_family(&family->family);
	kfree(family->copy_ops);
}

#define genl_unregister_family(family) \
	batadv_genl_unregister_family((family))

#define genlmsg_put(_skb, _pid, _seq, _family, _flags, _cmd) \
	genlmsg_put(_skb, _pid, _seq, &(_family)->family, _flags, _cmd)

#define genlmsg_multicast_netns(_family, _net, _skb, _portid, _group, _flags) \
	genlmsg_multicast_netns(&(_family)->family, _net, _skb, _portid, \
				_group, _flags)

#endif /* LINUX_VERSION_IS_LESS(5, 10, 0) */


#if LINUX_VERSION_IS_LESS(6, 2, 0)

#define genl_split_ops genl_ops

#endif /* LINUX_VERSION_IS_LESS(6, 2, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_NET_GENETLINK_H_ */
