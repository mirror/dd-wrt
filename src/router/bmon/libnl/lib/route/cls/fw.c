/*
 * lib/route/cls/fw.c		fw classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2006 Petr Gotthard <petr.gotthard@siemens.com>
 * Copyright (c) 2006 Siemens AG Oesterreich
 */

/**
 * @ingroup cls
 * @defgroup fw Firewall Classifier
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/route/classifier.h>
#include <netlink/route/classifier-modules.h>
#include <netlink/route/cls/fw.h>

/** @cond SKIP */
#define FW_ATTR_CLASSID      0x001
#define FW_ATTR_ACTION       0x002
#define FW_ATTR_POLICE       0x004
#define FW_ATTR_INDEV        0x008
/** @endcond */

static inline struct rtnl_fw *fw_cls(struct rtnl_cls *cls)
{
	return (struct rtnl_fw *) cls->c_subdata;
}

static inline struct rtnl_fw *fw_alloc(struct rtnl_cls *cls)
{
	if (!cls->c_subdata)
		cls->c_subdata = calloc(1, sizeof(struct rtnl_fw));

	return fw_cls(cls);
}

static struct nla_policy fw_policy[TCA_FW_MAX+1] = {
	[TCA_FW_CLASSID]	= { .type = NLA_U32 },
	[TCA_FW_INDEV]		= { .type = NLA_STRING,
				    .maxlen = IFNAMSIZ },
};

static int fw_msg_parser(struct rtnl_cls *cls)
{
	int err;
	struct nlattr *tb[TCA_FW_MAX + 1];
	struct rtnl_fw *f;

	err = tca_parse(tb, TCA_FW_MAX, (struct rtnl_tca *) cls, fw_policy);
	if (err < 0)
		return err;

	f = fw_alloc(cls);
	if (!f)
		goto errout_nomem;

	if (tb[TCA_FW_CLASSID]) {
		f->cf_classid = nla_get_u32(tb[TCA_FW_CLASSID]);
		f->cf_mask |= FW_ATTR_CLASSID;
	}

	if (tb[TCA_FW_ACT]) {
		f->cf_act = nla_get_data(tb[TCA_FW_ACT]);
		if (!f->cf_act)
			goto errout_nomem;
		f->cf_mask |= FW_ATTR_ACTION;
	}

	if (tb[TCA_FW_POLICE]) {
		f->cf_police = nla_get_data(tb[TCA_FW_POLICE]);
		if (!f->cf_police)
			goto errout_nomem;
		f->cf_mask |= FW_ATTR_POLICE;
	}

	if (tb[TCA_FW_INDEV]) {
		nla_strlcpy(f->cf_indev, tb[TCA_FW_INDEV], IFNAMSIZ);
		f->cf_mask |= FW_ATTR_INDEV;
	}

	return 0;

errout_nomem:
	err = nl_errno(ENOMEM);

	return err;
}

static void fw_free_data(struct rtnl_cls *cls)
{
	struct rtnl_fw *f = fw_cls(cls);

	if (!f)
		return;

	nl_data_free(f->cf_act);
	nl_data_free(f->cf_police);

	free(cls->c_subdata);
}

static int fw_dump_brief(struct rtnl_cls *cls, struct nl_dump_params *p,
			  int line)
{
	struct rtnl_fw *f = fw_cls(cls);
	char buf[32];

	if (!f)
		goto ignore;

	if (f->cf_mask & FW_ATTR_CLASSID)
		dp_dump(p, " target %s",
			rtnl_tc_handle2str(f->cf_classid, buf, sizeof(buf)));

ignore:
	return line;
}

static int fw_dump_full(struct rtnl_cls *cls, struct nl_dump_params *p,
			 int line)
{
	struct rtnl_fw *f = fw_cls(cls);

	if (!f)
		goto ignore;

	if (f->cf_mask & FW_ATTR_INDEV)
		dp_dump(p, "indev %s ", f->cf_indev);

ignore:
	return line;
}

static int fw_dump_stats(struct rtnl_cls *cls, struct nl_dump_params *p,
			  int line)
{
	struct rtnl_fw *f = fw_cls(cls);

	if (!f)
		goto ignore;

ignore:
	return line;
}

static struct nl_msg *fw_get_opts(struct rtnl_cls *cls)
{
	struct rtnl_fw *f;
	struct nl_msg *msg;
	
	f = fw_cls(cls);
	if (!f)
		return NULL;

	msg = nlmsg_build_no_hdr();
	if (!msg)
		return NULL;

	if (f->cf_mask & FW_ATTR_CLASSID)
		nla_put_u32(msg, TCA_FW_CLASSID, f->cf_classid);

	if (f->cf_mask & FW_ATTR_ACTION)
		nla_put_data(msg, TCA_FW_ACT, f->cf_act);

	if (f->cf_mask & FW_ATTR_POLICE)
		nla_put_data(msg, TCA_FW_POLICE, f->cf_police);

	if (f->cf_mask & FW_ATTR_INDEV)
		nla_put_string(msg, TCA_FW_INDEV, f->cf_indev);

	return msg;
}

/**
 * @name Attribute Modifications
 * @{
 */

int rtnl_fw_set_classid(struct rtnl_cls *cls, uint32_t classid)
{
	struct rtnl_fw *f;
	
	f = fw_alloc(cls);
	if (!f)
		return nl_errno(ENOMEM);

	f->cf_classid = classid;
	f->cf_mask |= FW_ATTR_CLASSID;

	return 0;
}

/** @} */

static struct rtnl_cls_ops fw_ops = {
	.co_kind		= "fw",
	.co_msg_parser		= fw_msg_parser,
	.co_free_data		= fw_free_data,
	.co_get_opts		= fw_get_opts,
	.co_dump[NL_DUMP_BRIEF]	= fw_dump_brief,
	.co_dump[NL_DUMP_FULL]	= fw_dump_full,
	.co_dump[NL_DUMP_STATS]	= fw_dump_stats,
};

static void __init fw_init(void)
{
	rtnl_cls_register(&fw_ops);
}

static void __exit fw_exit(void)
{
	rtnl_cls_unregister(&fw_ops);
}

/** @} */
