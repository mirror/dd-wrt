/*
 *	xt_mark - Netfilter module to match NFMARK value
 *
 *	(C) 1999-2001 Marc Boucher <marc@mbsi.ca>
 *	Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *	Jan Engelhardt <jengelh@medozas.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/xt_mark.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marc Boucher <marc@mbsi.ca>");
MODULE_DESCRIPTION("Xtables: packet mark operations");
MODULE_ALIAS("ipt_mark");
MODULE_ALIAS("ip6t_mark");
MODULE_ALIAS("ipt_MARK");
MODULE_ALIAS("ip6t_MARK");

static unsigned int
mark_tg_v0(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark_target_info *markinfo = par->targinfo;

	skb->mark = markinfo->mark;
	return XT_CONTINUE;
}

static unsigned int
mark_tg_v1(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark_target_info_v1 *markinfo = par->targinfo;
	int mark = 0;

	switch (markinfo->mode) {
	case XT_MARK_SET:
		mark = markinfo->mark;
		break;

	case XT_MARK_AND:
		mark = skb->mark & markinfo->mark;
		break;

	case XT_MARK_OR:
		mark = skb->mark | markinfo->mark;
		break;
	}

	skb->mark = mark;
	return XT_CONTINUE;
}


static unsigned int
mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_mark_tginfo2 *info = par->targinfo;

	skb->mark = (skb->mark & ~info->mask) ^ info->mark;
	return XT_CONTINUE;
}
static bool
mark_mt_v0(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_mark_info *info = par->matchinfo;
	return ((skb->mark & info->mask) == info->mark) ^ info->invert;
}

static bool
mark_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_mark_mtinfo1 *info = par->matchinfo;

	return ((skb->mark & info->mask) == info->mark) ^ info->invert;
}

static int mark_mt_check_v0(const struct xt_mtchk_param *par)
{
	const struct xt_mark_info *minfo = par->matchinfo;

	if (minfo->mark > 0xffffffff || minfo->mask > 0xffffffff) {
		printk(KERN_WARNING "mark: only supports 32bit mark\n");
		return -EINVAL;
	}
	return 0;
	
}
static int mark_tg_check_v0(const struct xt_tgchk_param *par)
{
	const struct xt_mark_target_info *markinfo = par->targinfo;

	if (markinfo->mark > 0xffffffff) {
		printk(KERN_WARNING "MARK: Only supports 32bit wide mark\n");
		return -EINVAL;
	}
	return 0;
}

static int mark_tg_check_v1(const struct xt_tgchk_param *par)
{
	const struct xt_mark_target_info_v1 *markinfo = par->targinfo;

	if (markinfo->mode != XT_MARK_SET
	    && markinfo->mode != XT_MARK_AND
	    && markinfo->mode != XT_MARK_OR) {
		printk(KERN_WARNING "MARK: unknown mode %u\n",
		       markinfo->mode);
		return -EINVAL;
	}
	if (markinfo->mark > 0xffffffff) {
		printk(KERN_WARNING "MARK: Only supports 32bit wide mark\n");
		return -EINVAL;
	}
	return 0;
}


#ifdef CONFIG_COMPAT
struct compat_xt_mark_target_info {
	compat_ulong_t	mark;
};

static void mark_tg_compat_from_user_v0(void *dst, void *src)
{
	const struct compat_xt_mark_target_info *cm = src;
	struct xt_mark_target_info m = {
		.mark	= cm->mark,
	};
	memcpy(dst, &m, sizeof(m));
}

static int mark_tg_compat_to_user_v0(void __user *dst, void *src)
{
	const struct xt_mark_target_info *m = src;
	struct compat_xt_mark_target_info cm = {
		.mark	= m->mark,
	};
	return copy_to_user(dst, &cm, sizeof(cm)) ? -EFAULT : 0;
}

struct compat_xt_mark_target_info_v1 {
	compat_ulong_t	mark;
	u_int8_t	mode;
	u_int8_t	__pad1;
	u_int16_t	__pad2;
};

static void mark_tg_compat_from_user_v1(void *dst, void *src)
{
	const struct compat_xt_mark_target_info_v1 *cm = src;
	struct xt_mark_target_info_v1 m = {
		.mark	= cm->mark,
		.mode	= cm->mode,
	};
	memcpy(dst, &m, sizeof(m));
}

static int mark_tg_compat_to_user_v1(void __user *dst, void *src)
{
	const struct xt_mark_target_info_v1 *m = src;
	struct compat_xt_mark_target_info_v1 cm = {
		.mark	= m->mark,
		.mode	= m->mode,
	};
	return copy_to_user(dst, &cm, sizeof(cm)) ? -EFAULT : 0;
}



struct compat_xt_mark_info {
	compat_ulong_t	mark, mask;
	u_int8_t	invert;
	u_int8_t	__pad1;
	u_int16_t	__pad2;
 };
 
static void mark_mt_compat_from_user_v0(void *dst, void *src)
{
	const struct compat_xt_mark_info *cm = src;
	struct xt_mark_info m = {
		.mark	= cm->mark,
		.mask	= cm->mask,
		.invert	= cm->invert,
	};
	memcpy(dst, &m, sizeof(m));
}

static int mark_mt_compat_to_user_v0(void __user *dst, void *src)
{
	const struct xt_mark_info *m = src;
	struct compat_xt_mark_info cm = {
		.mark	= m->mark,
		.mask	= m->mask,
		.invert	= m->invert,
	};
	return copy_to_user(dst, &cm, sizeof(cm)) ? -EFAULT : 0;
}
#endif /* CONFIG_COMPAT */

static struct xt_match mark_mt_reg[] __read_mostly = {
	{
		.name		= "mark",
		.revision	= 0,
		.family		= NFPROTO_UNSPEC,
		.checkentry	= mark_mt_check_v0,
		.match		= mark_mt_v0,
		.matchsize	= sizeof(struct xt_mark_info),
#ifdef CONFIG_COMPAT
		.compatsize	= sizeof(struct compat_xt_mark_info),
		.compat_from_user = mark_mt_compat_from_user_v0,
		.compat_to_user	= mark_mt_compat_to_user_v0,
#endif
		.me		= THIS_MODULE,
	},
	{
		.name           = "mark",
		.revision       = 1,
		.family         = NFPROTO_UNSPEC,
		.match          = mark_mt,
		.matchsize      = sizeof(struct xt_mark_mtinfo1),
		.me             = THIS_MODULE,
	},
};

static struct xt_target mark_tg_reg[] __read_mostly = {
	{
		.name		= "MARK",
		.family		= NFPROTO_UNSPEC,
		.revision	= 0,
		.checkentry	= mark_tg_check_v0,
		.target		= mark_tg_v0,
		.targetsize	= sizeof(struct xt_mark_target_info),
#ifdef CONFIG_COMPAT
		.compatsize	= sizeof(struct compat_xt_mark_target_info),
		.compat_from_user = mark_tg_compat_from_user_v0,
		.compat_to_user	= mark_tg_compat_to_user_v0,
#endif
		.table		= "mangle",
		.me		= THIS_MODULE,
	},
	{
		.name		= "MARK",
		.family		= NFPROTO_UNSPEC,
		.revision	= 1,
		.checkentry	= mark_tg_check_v1,
		.target		= mark_tg_v1,
		.targetsize	= sizeof(struct xt_mark_target_info_v1),
#ifdef CONFIG_COMPAT
		.compatsize	= sizeof(struct compat_xt_mark_target_info_v1),
		.compat_from_user = mark_tg_compat_from_user_v1,
		.compat_to_user	= mark_tg_compat_to_user_v1,
#endif
		.table		= "mangle",
		.me		= THIS_MODULE,
	},
	{
		.name           = "MARK",
		.revision       = 2,
		.family         = NFPROTO_UNSPEC,
		.target         = mark_tg,
		.targetsize     = sizeof(struct xt_mark_tginfo2),
		.me             = THIS_MODULE,
	},
};


static int __init mark_mt_init(void)
{
	int ret;

	ret = xt_register_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));
	if (ret < 0)
		return ret;
	ret = xt_register_matches(mark_mt_reg, ARRAY_SIZE(mark_mt_reg));
	if (ret < 0) {
		xt_unregister_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));
		return ret;
	}
	return 0;
}

static void __exit mark_mt_exit(void)
{
	xt_unregister_matches(mark_mt_reg, ARRAY_SIZE(mark_mt_reg));
	xt_unregister_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));
}

module_init(mark_mt_init);
module_exit(mark_mt_exit);
