/*
 * route/sch/tbf.c	tbf (qdisc|class)
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

/**
 * @ingroup qdisc
 * @defgroup tbf Token Bucket Filter (TBF)
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/helpers.h>
#include <netlink/route/rtattr.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/link.h>
#include <netlink/route/sch/tbf.h>

static inline struct rtnl_tbf *get_tbf_qdisc(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata == NULL)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_tbf));

	return (struct rtnl_tbf *) qdisc->q_subdata;
}

double rtnl_tbf_calc_latency(const double limit, const double rate,
			     const uint32_t bufsize)
{
	return ((limit / rate) * 1000000) - bufsize;
}	

double rtnl_tbf_get_rate_latency(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *t = (struct rtnl_tbf *) qdisc->q_subdata;

	if (t == NULL)
	    return 0.0f;
	
	return rtnl_tbf_calc_latency((double) t->qt_limit,
				     (double) t->qt_rate.rs_rate,
				     nl_ticks2us(t->qt_buffer));
}

double rtnl_tbf_get_peak_latency(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *t = (struct rtnl_tbf *) qdisc->q_subdata;

	if (t == NULL)
		return 0.0f;

	return rtnl_tbf_calc_latency((double) t->qt_limit,
				     (double) t->qt_peakrate.rs_rate,
				     nl_ticks2us(t->qt_mtu));
}

double rtnl_tbf_get_buffer(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *t = (struct rtnl_tbf *) qdisc->q_subdata;

	if (t == NULL)
		return 0.0f;

	return ((double) t->qt_rate.rs_rate * nl_ticks2us(t->qt_buffer)) /
		1000000.0f;
}

double rtnl_tbf_get_mtu(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *t = (struct rtnl_tbf *) qdisc->q_subdata;

	if (t == NULL)
		return 0.0f;

	return ((double) t->qt_peakrate.rs_rate * nl_ticks2us(t->qt_mtu)) /
		1000000.0f;
}

static int tbf_msg_parser(struct rtnl_qdisc *q)
{
	int err;
	struct rtattr *tb[TCA_TBF_MAX + 1];
	struct rtnl_tbf *t = NULL;

	err = nl_parse_rtattr(tb, TCA_TBF_MAX, (void *) q->q_opts.d_data,
			      q->q_opts.d_size);
	if (err < 0)
		return err;
	
	t = get_tbf_qdisc(q);

	if (tb[TCA_TBF_PARMS]) {
		struct tc_tbf_qopt opts;
		err = NL_COPY_DATA(opts, tb[TCA_TBF_PARMS]);
		if (err < 0)
			goto err_out;

		t->qt_limit = opts.limit;
		t->qt_buffer = opts.buffer;
		t->qt_mtu = opts.mtu;

		rtnl_copy_ratespec(&t->qt_rate, &opts.rate);
		rtnl_copy_ratespec(&t->qt_peakrate, &opts.peakrate);

		t->qt_mask = (SCH_TBF_HAS_LIMIT | SCH_TBF_HAS_BUFFER |
			      SCH_TBF_HAS_MTU | SCH_TBF_HAS_RATE |
			      SCH_TBF_HAS_PEAKRATE);
	}

	return 0;

err_out:
	if (t)
		free(t);
	
	return err;
}

static int tbf_dump_brief(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			  struct nl_dump_params *params, int line)
{
	double r, rbit;
	char *ru, *rubit;
	struct rtnl_tbf *t = (struct rtnl_tbf *) q->q_subdata;

	if (NULL == t)
		return line;

	r = nl_cancel_down_bytes(t->qt_rate.rs_rate, &ru);
	rbit = nl_cancel_down_bits(t->qt_rate.rs_rate*8, &rubit);

	fprintf(fd, " rate %.2f%s/s (%.0f%s) log %u", 
		r, ru, rbit, rubit, 1<<t->qt_rate.rs_cell_log);

	return line;
}

static int tbf_dump_full(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			 struct nl_dump_params *params, int line)
{
	struct rtnl_tbf *t = (struct rtnl_tbf *) q->q_subdata;

	if (t == NULL)
		return line;

	{ /* line 1 */
	char *lu, *latu, *bu, *mu;
	double lim = nl_cancel_down_bytes(t->qt_limit, &lu);
	double lat = nl_cancel_down_us(rtnl_tbf_get_rate_latency(q), &latu);
	double bst = nl_cancel_down_bytes(rtnl_tbf_get_buffer(q), &bu);
	double mpu = nl_cancel_down_bytes(t->qt_rate.rs_mpu, &mu);

	fprintf(fd, "limit %.0f%s latency %.1f%s burst %.1f%s mpu %.0f%s",
		lim, lu, lat, latu, bst, bu, mpu, mu);
	}

	dp_new_line(fd, params, line++);
	{ /* line 2*/
	char *ru, *rubit, *latu, *mu;
	double prt = nl_cancel_down_bytes(t->qt_peakrate.rs_rate, &ru);
	double prb = nl_cancel_down_bits(t->qt_peakrate.rs_rate*8, &rubit);
	double lat = nl_cancel_down_us(rtnl_tbf_get_peak_latency(q), &latu);
	double mpu = nl_cancel_down_bytes(t->qt_peakrate.rs_mpu, &mu);

	fprintf(fd, "\n    peak %.2f%s/s (%.0f%s) log %u latency %.1f%s",
		prt, ru, prb, rubit, 1<<t->qt_peakrate.rs_cell_log, lat, latu);

	fprintf(fd, " mtu %.0f mpu %.0f%s", rtnl_tbf_get_mtu(q), mpu, mu);
	}

	return line;
}

static struct nl_msg *tbf_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_tbf *t = (struct rtnl_tbf *) qdisc->q_subdata;

	if (t) {
		struct tc_tbf_qopt opts;
		struct nl_msg *msg = nl_msg_build(NULL);

		memset(&opts, 0, sizeof(opts));
		nl_msg_append_tlv(msg, TCA_TBF_PARMS, &opts, sizeof(opts));
		return msg;
	}

	return NULL;
}

/**
 * @name Attribute Modifications
 */

/**
 * Set limit of the tbf qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg limit		new limit in bytes
 */
void rtnl_tbf_set_limit(struct rtnl_qdisc *qdisc, uint32_t limit)
{
	struct rtnl_tbf *t = get_tbf_qdisc(qdisc);
	if (t == NULL)
		return;

	t->qt_limit = limit;
	t->qt_mask |= SCH_TBF_HAS_LIMIT;
}

/** @} */

static struct rtnl_qdisc_ops tbf_qdisc_ops = {
	.qo_kind		= "tbf",
	.qo_msg_parser		= &tbf_msg_parser,
	.qo_dump[NL_DUMP_BRIEF]	= &tbf_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= &tbf_dump_full,
	.qo_get_opts		= &tbf_get_opts,
};

void __init tbf_init(void)
{
	rtnl_qdisc_register(&tbf_qdisc_ops);
}

void __exit tbf_exit(void)
{
	rtnl_qdisc_unregister(&tbf_qdisc_ops);
}

/** @} */
