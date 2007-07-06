/*
 * lib/route/sch/htb.c	HTB Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2005-2006 Petr Gotthard <petr.gotthard@siemens.com>
 * Copyright (c) 2005-2006 Siemens AG Oesterreich
 */

/**
 * @ingroup qdisc
 * @ingroup class
 * @defgroup htb Hierachical Token Bucket (HTB)
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc-modules.h>
#include <netlink/route/class.h>
#include <netlink/route/class-modules.h>
#include <netlink/route/link.h>
#include <netlink/route/sch/htb.h>

/** @cond SKIP */
#define SCH_HTB_HAS_RATE2QUANTUM	0x01
#define SCH_HTB_HAS_DEFCLS		0x02

#define SCH_HTB_HAS_PRIO		0x01
#define SCH_HTB_HAS_MTU			0x02
#define SCH_HTB_HAS_RATE		0x04
#define SCH_HTB_HAS_CEIL		0x08
#define SCH_HTB_HAS_RBUFFER		0x10
#define SCH_HTB_HAS_CBUFFER		0x20
/** @endcond */

static inline struct rtnl_htb_qdisc *htb_qdisc(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata == NULL)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_htb_qdisc));

	return (struct rtnl_htb_qdisc *) qdisc->q_subdata;
}

static struct nla_policy htb_policy[TCA_HTB_MAX+1] = {
	[TCA_HTB_INIT]	= { .minlen = sizeof(struct tc_htb_glob) },
	[TCA_HTB_PARMS] = { .minlen = sizeof(struct tc_htb_opt) },
};

static int htb_qdisc_msg_parser(struct rtnl_qdisc *qdisc)
{
	int err;
	struct nlattr *tb[TCA_HTB_MAX + 1];
	struct rtnl_htb_qdisc *d;

	err = tca_parse(tb, TCA_HTB_MAX, (struct rtnl_tca *) qdisc, htb_policy);
	if (err < 0)
		return err;
	
	d = htb_qdisc(qdisc);

	if (tb[TCA_HTB_INIT]) {
		struct tc_htb_glob opts;

		nla_memcpy(&opts, tb[TCA_HTB_INIT], sizeof(opts));
		d->qh_rate2quantum = opts.rate2quantum;
		d->qh_defcls = opts.defcls;

		d->qh_mask = (SCH_HTB_HAS_RATE2QUANTUM | SCH_HTB_HAS_DEFCLS);
	}

	return 0;
}

static void htb_qdisc_free_data(struct rtnl_qdisc *qdisc)
{
	free(qdisc->q_subdata);
}

static inline struct rtnl_htb_class *htb_class(struct rtnl_class *class)
{
	if (class->c_subdata == NULL)
		class->c_subdata = calloc(1, sizeof(struct rtnl_htb_class));

	return (struct rtnl_htb_class *) class->c_subdata;
}

static int htb_class_msg_parser(struct rtnl_class *class)
{
	int err;
	struct nlattr *tb[TCA_HTB_MAX + 1];
	struct rtnl_htb_class *d;

	err = tca_parse(tb, TCA_HTB_MAX, (struct rtnl_tca *) class, htb_policy);
	if (err < 0)
		return err;
	
	d = htb_class(class);

	if (tb[TCA_HTB_PARMS]) {
		struct tc_htb_opt opts;

		nla_memcpy(&opts, tb[TCA_HTB_PARMS], sizeof(opts));
		d->ch_prio = opts.prio;
		rtnl_copy_ratespec(&d->ch_rate, &opts.rate);
		rtnl_copy_ratespec(&d->ch_ceil, &opts.ceil);
		d->ch_rbuffer = opts.buffer;
		d->ch_cbuffer = opts.cbuffer;

		d->ch_mask = (SCH_HTB_HAS_PRIO | SCH_HTB_HAS_RATE |
			SCH_HTB_HAS_CEIL | SCH_HTB_HAS_RBUFFER |
			SCH_HTB_HAS_CBUFFER);
	}

	return 0;
}

static void htb_class_free_data(struct rtnl_class *class)
{
	free(class->c_subdata);
}

static int htb_qdisc_dump_brief(struct rtnl_qdisc *qdisc,
				struct nl_dump_params *p, int line)
{
	struct rtnl_htb_qdisc *d = (struct rtnl_htb_qdisc *) qdisc->q_subdata;

	if (d == NULL)
		goto ignore;

	if (d->qh_mask & SCH_HTB_HAS_RATE2QUANTUM)
		dp_dump(p, " r2q %u", d->qh_rate2quantum);

	if (d->qh_mask & SCH_HTB_HAS_DEFCLS) {
		char buf[32];
		dp_dump(p, " default %s",
			rtnl_tc_handle2str(d->qh_defcls, buf, sizeof(buf)));
	}

ignore:
	return line;
}

static int htb_class_dump_brief(struct rtnl_class *class,
				struct nl_dump_params *p, int line)
{
	struct rtnl_htb_class *d = (struct rtnl_htb_class *) class->c_subdata;

	if (d == NULL)
		goto ignore;

	if (d->ch_mask & SCH_HTB_HAS_RATE) {
		double r, rbit;
		char *ru, *rubit;

		r = nl_cancel_down_bytes(d->ch_rate.rs_rate, &ru);
		rbit = nl_cancel_down_bits(d->ch_rate.rs_rate*8, &rubit);

		dp_dump(p, " rate %.2f%s/s (%.0f%s) log %u",
			r, ru, rbit, rubit, 1<<d->ch_rate.rs_cell_log);
	}

ignore:
	return line;
}

static int htb_class_dump_full(struct rtnl_class *class,
			       struct nl_dump_params *p, int line)
{
	struct rtnl_htb_class *d = (struct rtnl_htb_class *) class->c_subdata;

	if (d == NULL)
		goto ignore;

	/* line 1 */
	if (d->ch_mask & SCH_HTB_HAS_CEIL) {
		double r, rbit;
		char *ru, *rubit;

		r = nl_cancel_down_bytes(d->ch_ceil.rs_rate, &ru);
		rbit = nl_cancel_down_bits(d->ch_ceil.rs_rate*8, &rubit);

		dp_dump(p, "    ceil %.2f%s/s (%.0f%s) log %u",
			r, ru, rbit, rubit, 1<<d->ch_ceil.rs_cell_log);
	}

	if (d->ch_mask & SCH_HTB_HAS_PRIO)
		dp_dump(p, " prio %u", d->ch_prio);
	if (d->ch_mask & SCH_HTB_HAS_RBUFFER)
		dp_dump(p, " rbuffer %u", d->ch_rbuffer);
	if (d->ch_mask & SCH_HTB_HAS_CBUFFER)
		dp_dump(p, " cbuffer %u", d->ch_cbuffer);

ignore:
	return line;
}

static struct nl_msg *htb_qdisc_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_htb_qdisc *d = (struct rtnl_htb_qdisc *) qdisc->q_subdata;
	struct tc_htb_glob opts;
	struct nl_msg *msg;

	if (d == NULL)
		return NULL;

	msg = nlmsg_build(NULL);
	if (msg == NULL)
		return NULL;

	memset(&opts, 0, sizeof(opts));
	opts.version = TC_HTB_PROTOVER;

	if (d->qh_mask & SCH_HTB_HAS_RATE2QUANTUM)
		opts.rate2quantum = d->qh_rate2quantum;
	if (d->qh_mask & SCH_HTB_HAS_DEFCLS)
		opts.defcls = d->qh_defcls;

	nla_put(msg, TCA_HTB_INIT, sizeof(opts), &opts);

	return msg;
}

static inline uint32_t compute_burst(uint32_t rate, uint32_t mtu)
{
	return rtnl_tc_calc_txtime(rate / nl_get_hz() + mtu, rate);
}

static uint8_t compute_cell(uint32_t rate, uint32_t mtu)
{
	uint8_t cell_log = 0;
	while (mtu > 255) {
		mtu >>= 1;
		cell_log++;
	}

	return cell_log;
}

static struct nl_msg *htb_class_get_opts(struct rtnl_class *class)
{
	struct rtnl_htb_class *d = (struct rtnl_htb_class *) class->c_subdata;
	uint32_t rtable[256], ctable[256];
	struct tc_htb_opt opts;
	struct nl_msg *msg;
	

	if (d == NULL)
		return NULL;

	msg = nlmsg_build(NULL);
	memset(&opts, 0, sizeof(opts));

	/* if not set, zero (0) is used as priority */
	if (d->ch_mask & SCH_HTB_HAS_PRIO)
		opts.prio = d->ch_prio;

	if (!(d->ch_mask & SCH_HTB_HAS_RATE))
		BUG();

	rtnl_rcopy_ratespec(&opts.rate, &d->ch_rate);
	/* if cell_log not set, compute default value */
	if (opts.rate.cell_log == UINT8_MAX)
	{
		if(!(d->ch_mask & SCH_HTB_HAS_MTU))
			BUG();
		opts.rate.cell_log = compute_cell(opts.rate.rate, d->ch_mtu);
	}

	/* if not set, configured rate is used as ceil, which implies no borrowing */
	if (d->ch_mask & SCH_HTB_HAS_CEIL)
		rtnl_rcopy_ratespec(&opts.ceil, &d->ch_ceil);
	else
		memcpy(&opts.ceil, &opts.rate, sizeof(struct tc_ratespec));
	/* if cell_log not set, compute default value */
	if (opts.ceil.cell_log == UINT8_MAX)
	{
		if(!(d->ch_mask & SCH_HTB_HAS_MTU))
			BUG();
		opts.ceil.cell_log = compute_cell(opts.ceil.rate, d->ch_mtu);
	}

	if (d->ch_mask & SCH_HTB_HAS_RBUFFER)
		opts.buffer = d->ch_rbuffer;
	else
	{
		if(!(d->ch_mask & SCH_HTB_HAS_MTU))
			BUG();
		opts.buffer = compute_burst(opts.rate.rate, d->ch_mtu);
	}

	if (d->ch_mask & SCH_HTB_HAS_CBUFFER)
		opts.cbuffer = d->ch_cbuffer;
	else
	{
		if(!(d->ch_mask & SCH_HTB_HAS_MTU))
			BUG();
		opts.cbuffer = compute_burst(opts.ceil.rate, d->ch_mtu);
	}

	nla_put(msg, TCA_HTB_PARMS, sizeof(opts), &opts);
	rtnl_tc_build_rate_table(rtable, opts.rate.mpu & 0xff,
				 opts.rate.mpu >> 8, 1 << opts.rate.cell_log,
				 opts.rate.rate);
	nla_put(msg, TCA_HTB_RTAB, sizeof(rtable), &rtable);
	rtnl_tc_build_rate_table(ctable, opts.ceil.mpu & 0xff,
				 opts.ceil.mpu >> 8, 1 << opts.ceil.cell_log,
				 opts.ceil.rate);
	nla_put(msg, TCA_HTB_CTAB, sizeof(ctable), &ctable);

	return msg;
}

/**
 * @name Attribute Modifications
 * @{
 */

void rtnl_htb_set_rate2quantum(struct rtnl_qdisc *qdisc, uint32_t rate2quantum)
{
	struct rtnl_htb_qdisc *d = htb_qdisc(qdisc);
	if (d == NULL)
		return;

	d->qh_rate2quantum = rate2quantum;
	d->qh_mask |= SCH_HTB_HAS_RATE2QUANTUM;
}

/**
 * Set default class of the htb qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg defcls		new default class
 */
void rtnl_htb_set_defcls(struct rtnl_qdisc *qdisc, uint32_t defcls)
{
	struct rtnl_htb_qdisc *d = htb_qdisc(qdisc);
	if (d == NULL)
		return;

	d->qh_defcls = defcls;
	d->qh_mask |= SCH_HTB_HAS_DEFCLS;
}

void rtnl_htb_set_prio(struct rtnl_class *class, uint32_t prio)
{
	struct rtnl_htb_class *d = htb_class(class);
	if (d == NULL)
		return;

	d->ch_prio = prio;
	d->ch_mask |= SCH_HTB_HAS_PRIO;
}

void rtnl_htb_set_mtu(struct rtnl_class *class, uint32_t mtu)
{
	struct rtnl_htb_class *d = htb_class(class);
	if (d == NULL)
		return;

	d->ch_mtu = mtu;
	d->ch_mask |= SCH_HTB_HAS_MTU;
}

void rtnl_htb_set_rate(struct rtnl_class *class, uint32_t rate)
{
	struct rtnl_htb_class *d = htb_class(class);
	if (d == NULL)
		return;

	d->ch_rate.rs_cell_log = UINT8_MAX; /* use default value */
	d->ch_rate.rs_rate = rate;
	d->ch_mask |= SCH_HTB_HAS_RATE;
}

void rtnl_htb_set_ceil(struct rtnl_class *class, uint32_t ceil)
{
	struct rtnl_htb_class *d = htb_class(class);
	if (d == NULL)
		return;

	d->ch_ceil.rs_cell_log = UINT8_MAX; /* use default value */
	d->ch_ceil.rs_rate = ceil;
	d->ch_mask |= SCH_HTB_HAS_CEIL;
}

/** @} */

static struct rtnl_qdisc_ops htb_qdisc_ops = {
	.qo_kind		= "htb",
	.qo_msg_parser		= htb_qdisc_msg_parser,
	.qo_free_data		= htb_qdisc_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= htb_qdisc_dump_brief,
	.qo_get_opts		= htb_qdisc_get_opts,
};

static struct rtnl_class_ops htb_class_ops = {
	.co_kind		= "htb",
	.co_msg_parser		= htb_class_msg_parser,
	.co_free_data		= htb_class_free_data,
	.co_dump[NL_DUMP_BRIEF]	= htb_class_dump_brief,
	.co_dump[NL_DUMP_FULL]	= htb_class_dump_full,
	.co_get_opts		= htb_class_get_opts,
};

static void __init htb_init(void)
{
	rtnl_qdisc_register(&htb_qdisc_ops);
	rtnl_class_register(&htb_class_ops);
}

static void __exit htb_exit(void)
{
	rtnl_qdisc_unregister(&htb_qdisc_ops);
	rtnl_class_unregister(&htb_class_ops);
}

/** @} */
