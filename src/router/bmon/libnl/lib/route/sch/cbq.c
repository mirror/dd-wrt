/*
 * cbq.c              rtnetlink cbq (qdisc|class)
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */


#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/tca.h>
#include <netlink/qdisc.h>
#include <netlink/class.h>
#include <netlink/link.h>
#include <netlink/cbq.h>
#include <netlink/helpers.h>
#include <netlink/route/rtattr.h>

/**
 * @ingroup qdisc
 * @defgroup cbq Class Based Queueing (CBQ)
 * @{
 */

static struct {
	int i;
	const char *a;
} ovl_strategies[] = {
#define __A(id, name) { TC_CBQ_OVL_##id, #name },
	__A(CLASSIC,classic)
	__A(DELAY,delay)
	__A(LOWPRIO,lowprio)
	__A(DROP,drop)
	__A(RCLASSIC,rclassic)
#undef __A
};


/**
 * Convert a CBQ OVL strategy to a character string
 * @arg type		CBQ OVL strategy
 * @arg buf		destination buffer
 * @arg len		length of destination buffer
 *
 * Converts a CBQ OVL strategy to a character string and stores in the
 * provided buffer. Returns the destination buffer or the type
 * encoded in hex if no match was found.
 */
char * nl_ovl_strategy2str_r(int type, char *buf, size_t len)
{
	int i;
	for (i = 0; i < sizeof(ovl_strategies)/sizeof(ovl_strategies[0]); i++) {
		if (ovl_strategies[i].i == type) {
			snprintf(buf, len, "%s", ovl_strategies[i].a);
			return buf;
		}
	}

	snprintf(buf, len, "0x%x", type);
	return buf;
}


/**
 * Convert a CBQ OVL strategy to a character string
 * @arg type		CBQ OVL strategy
 *
 * Same as nl_ovl_strategy2str_r but uses a static buffer.
 */
char * nl_ovl_strategy2str(int type)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return nl_ovl_strategy2str_r(type, buf, sizeof(buf));
}

/**
 * Convert a string to a CBQ OVL strategy
 * @arg name		CBQ OVL stragegy name
 *
 * Converts a CBQ OVL stragegy name to it's corresponding CBQ OVL strategy
 * type. Returns the type or -1 if none was found.
 */
int nl_str2ovl_strategy(const char *name)
{
	int i;
	for (i = 0; i < sizeof(ovl_strategies)/sizeof(ovl_strategies[0]); i++)
		if (!strcmp(ovl_strategies[i].a, name))
			return ovl_strategies[i].i;

	return -1;
}


/**
 * cbq_msg_parser - Generic CBQ rtnetlink message parser
 *
 * Works for qdisc and class messages
 */
static int cbq_msg_parser(struct rtnl_tca *g)
{
	int err;
	struct rtattr *tb[TCA_CBQ_MAX + 1];
	struct rtnl_cbq *q = NULL;

	memset(tb, 0, (TCA_CBQ_MAX + 1) * sizeof(struct rtattr *));
	if ((err = nl_parse_rtattr(tb, TCA_CBQ_MAX, (void *) g->tc_opts.d_data,
		g->tc_opts.d_size)) < 0)
			return err;

	q = g->tc_subdata = calloc(1, sizeof(struct rtnl_cbq));

	if (tb[TCA_CBQ_LSSOPT])
		if ((err = NL_COPY_DATA(q->cbq_lss, tb[TCA_CBQ_LSSOPT])) < 0)
				goto err_out;

	if (tb[TCA_CBQ_RATE])
		if ((err = NL_COPY_DATA(q->cbq_rate, tb[TCA_CBQ_RATE])) < 0)
				goto err_out;

	if (tb[TCA_CBQ_WRROPT])
		if ((err = NL_COPY_DATA(q->cbq_wrr, tb[TCA_CBQ_WRROPT])) < 0)
				goto err_out;

	if (tb[TCA_CBQ_OVL_STRATEGY])
		if ((err = NL_COPY_DATA(q->cbq_ovl, tb[TCA_CBQ_OVL_STRATEGY])) < 0)
				goto err_out;

	if (tb[TCA_CBQ_FOPT])
		if ((err = NL_COPY_DATA(q->cbq_fopt, tb[TCA_CBQ_FOPT])) < 0)
				goto err_out;

	if (tb[TCA_CBQ_POLICE])
		if ((err = NL_COPY_DATA(q->cbq_police, tb[TCA_CBQ_POLICE])) < 0)
				goto err_out;
	
	return 0;

err_out:
	if (q)
		free(q);
	
	return err;
}

static int cbq_qdisc_msg_parser(struct rtnl_qdisc *q)
{
	return cbq_msg_parser((struct rtnl_tca *) q);
}

static int cbq_class_msg_parser(struct rtnl_class *c)
{
	return cbq_msg_parser((struct rtnl_tca *) c);
}
	

static void cbq_free_data(struct rtnl_tca *g)
{
	if (g->tc_subdata) {
		free(g->tc_subdata);
		g->tc_subdata = NULL;
	}
}

static void cbq_qdisc_free_data(struct rtnl_qdisc *q)
{
	cbq_free_data((struct rtnl_tca *) q);
}

static void cbq_class_free_data(struct rtnl_class *c)
{
	cbq_free_data((struct rtnl_tca *) c);
}

static void cbq_dump_brief(struct nl_cache *c, struct rtnl_tca *g, FILE *fd)
{
	double r, rbit;
	char *ru, *rubit;
	struct rtnl_cbq *q = (struct rtnl_cbq *) g->tc_subdata;

	if (NULL == q)
		return;

	r = nl_sumup(q->cbq_rate.rate, &ru);
	rbit = nl_sumup_bit((q->cbq_rate.rate*8), &rubit);

	fprintf(fd, " rate %.2f%s/s (%.0f%s) prio %u",
		r, ru, rbit, rubit, q->cbq_wrr.priority);
}

static void cbq_qdisc_dump_brief(struct nl_cache *c, struct rtnl_qdisc *q,
				 FILE *fd)
{
	cbq_dump_brief(c, (struct rtnl_tca *) q, fd);
}

static void cbq_class_dump_brief(struct nl_cache *c, struct rtnl_class *cl,
				 FILE *fd)
{
	cbq_dump_brief(c, (struct rtnl_tca *) cl, fd);
}

static void cbq_dump_full(struct nl_cache *c, struct rtnl_tca *g, FILE *fd)
{
	double w;
	char *unit, ovl[32], split[32], p[32];
	uint32_t el;
	struct rtnl_cbq *q = (struct rtnl_cbq *) g->tc_subdata;

	if (NULL == q)
		return;

	w = nl_sumup_bit((8 * q->cbq_wrr.weight), &unit);

	fprintf(fd, "avgpkt %u mpu %u cell %u allot %u weight %.0f%s\n",
		q->cbq_lss.avpkt,
		q->cbq_rate.mpu,
		1 << q->cbq_rate.cell_log,
		q->cbq_wrr.allot, w, unit);

	el = q->cbq_lss.ewma_log;

	fprintf(fd, "%s    minidle %uus maxidle %uus offtime %uus level %u " \
		"ewma_log %u\n", nl_get_dump_prefix(),
		nl_ticks2us(q->cbq_lss.minidle >> el),
		nl_ticks2us(q->cbq_lss.maxidle >> el),
		nl_ticks2us(q->cbq_lss.offtime >> el),
		q->cbq_lss.level,
		q->cbq_lss.ewma_log);

	nl_handle2str_r(q->cbq_fopt.split, split, sizeof(split));

	fprintf(fd, "%s    penalty %uus strategy %s split %s defmap 0x%08x " \
		"police %s", nl_get_dump_prefix(),
		nl_ticks2us(q->cbq_ovl.penalty),
		nl_ovl_strategy2str_r(q->cbq_ovl.strategy, ovl, sizeof(ovl)),
		split, q->cbq_fopt.defmap,
		nl_police2str_r(q->cbq_police.police, p, sizeof(p)));
}

static void cbq_qdisc_dump_full(struct nl_cache *c, struct rtnl_qdisc *q,
				FILE *fd)
{
	cbq_dump_full(c, (struct rtnl_tca *) q, fd);
}

static void cbq_class_dump_full(struct nl_cache *c, struct rtnl_class *cl,
				FILE *fd)
{
	cbq_dump_full(c, (struct rtnl_tca *) cl, fd);
}
	

static void cbq_dump_with_stats(struct nl_cache *c, struct rtnl_tca *g,
				FILE *fd)
{
	struct tc_cbq_xstats *x = (struct tc_cbq_xstats *) g->tc_xstats.d_data;

	fprintf(fd, "%s            borrows    overact    avgidle  undertime\n",
		nl_get_dump_prefix());
	fprintf(fd, "%s         %10u %10u %10u %10u\n",
		nl_get_dump_prefix(), x->borrows, x->overactions, x->avgidle,
		x->undertime);
}

static void cbq_qdisc_dump_with_stats(struct nl_cache *c, struct rtnl_qdisc *q,
				      FILE *fd)
{
	cbq_dump_with_stats(c, (struct rtnl_tca *) q, fd);
}

static void cbq_class_dump_with_stats(struct nl_cache *c, struct rtnl_class *cl,
				      FILE *fd)
{
	cbq_dump_with_stats(c, (struct rtnl_tca *) cl, fd);
}

static struct rtnl_qdisc_ops cbq_qdisc_ops =
{
	.qo_kind		= "cbq",
	.qo_msg_parser		= &cbq_qdisc_msg_parser,
	.qo_free_data		= &cbq_qdisc_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= &cbq_qdisc_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= &cbq_qdisc_dump_full,
	.qo_dump[NL_DUMP_STATS]	= &cbq_qdisc_dump_with_stats,
};

static struct rtnl_class_ops cbq_class_ops =
{
	.co_kind		= "cbq",
	.co_msg_parser		= &cbq_class_msg_parser,
	.co_free_data		= &cbq_class_free_data,
	.co_dump[NL_DUMP_BRIEF]	= &cbq_class_dump_brief,
	.co_dump[NL_DUMP_FULL]	= &cbq_class_dump_full,
	.co_dump[NL_DUMP_STATS]	= &cbq_class_dump_with_stats,
};

void __init cbq_init(void)
{
	rtnl_qdisc_register(&cbq_qdisc_ops);
	rtnl_class_register(&cbq_class_ops);
}

void __exit cbq_exit(void)
{
	rtnl_qdisc_unregister(&cbq_qdisc_ops);
	rtnl_class_unregister(&cbq_class_ops);
}

/** @} */
