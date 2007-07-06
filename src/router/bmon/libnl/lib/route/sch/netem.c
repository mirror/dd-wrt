/*
 * route/sch/netem.c	Network Emulator
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
 * @defgroup netem Network Emulator
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/rtattr.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/sch/netem.h>

static inline struct rtnl_netem *get_netem(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata == NULL)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_netem));

	return (struct rtnl_netem *) qdisc->q_subdata;
}

static int netem_msg_parser(struct rtnl_qdisc *q)
{
	int len, err = 0;
	struct rtnl_netem *n;
	struct tc_netem_qopt *opts;

	if (q->q_opts.d_size < sizeof(*opts))
		return nl_error(EINVAL, "Netem specific options size mismatch");

	n = get_netem(q);

	opts = (struct tc_netem_qopt *) q->q_opts.d_data;

	n->qnm_latency = opts->latency;
	n->qnm_limit = opts->limit;
	n->qnm_loss = opts->loss;
	n->qnm_gap = opts->gap;
	n->qnm_duplicate = opts->duplicate;
	n->qnm_jitter = opts->jitter;

	n->qnm_mask = (SCH_NETEM_HAS_LATENCY | SCH_NETEM_HAS_LIMIT |
		       SCH_NETEM_HAS_LOSS | SCH_NETEM_HAS_GAP |
		       SCH_NETEM_HAS_DUPLICATE | SCH_NETEM_HAS_JITTER);

	len = q->q_opts.d_size - sizeof(*opts);

	if (len > 0) {
		struct rtattr *tb[TCA_NETEM_MAX+1];
		nl_parse_rtattr(tb, TCA_NETEM_MAX,
				q->q_opts.d_data + sizeof(*opts),  len);
		if (tb[TCA_NETEM_CORR]) {
			struct tc_netem_corr cor;
			err = NL_COPY_DATA(cor, tb[TCA_NETEM_CORR]);
			if (err < 0)
				goto err_out;

			n->qnm_corr.nmc_delay = cor.delay_corr;
			n->qnm_corr.nmc_loss = cor.loss_corr;
			n->qnm_corr.nmc_duplicate = cor.dup_corr;

			n->qnm_mask |= (SCH_NETEM_HAS_DELAY_CORR |
					SCH_NETEM_HAS_LOSS_CORR |
					SCH_NETEM_HAS_DELAY_CORR);
		}
	}

	return 0;

err_out:
	if (n)
		free(n);
	return err;

}

static void netem_free_data(struct rtnl_qdisc *q)
{
	if (q->q_subdata) {
		free(q->q_subdata);
		q->q_subdata = NULL;
	}
}

static int netem_dump_brief(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			    struct nl_dump_params *params, int line)
{
	struct rtnl_netem *n = (struct rtnl_netem *) q->q_subdata;
	
	fprintf(fd, "limit %d", n->qnm_limit);
	return line;
}

static int netem_dump_full(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			   struct nl_dump_params *params, int line)
{
	return line;
}

static struct nl_msg *netem_get_opts(struct rtnl_qdisc *qdisc)
{
	return NULL;
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set limit of netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg limit		new limit of queue in number of packets
 */
void rtnl_netem_set_limit(struct rtnl_qdisc *qdisc, uint32_t limit)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_limit = limit;
	n->qnm_mask |= SCH_NETEM_HAS_LIMIT;
}

/**
 * Set re-ordering gap of netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg gap		new gap in number of packets
 */
void rtnl_netem_set_gap(struct rtnl_qdisc *qdisc, uint32_t gap)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_gap = gap;
	n->qnm_mask |= SCH_NETEM_HAS_GAP;
}

/**
 * Set loss probability of the netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg loss		new loss probability (NL_PROB_MIN..NL_PROB_MAX)
 */
void rtnl_netem_set_loss(struct rtnl_qdisc *qdisc, uint32_t loss)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_loss = loss;
	n->qnm_mask |= SCH_NETEM_HAS_LOSS;
}

/**
 * Set duplicate probability of the netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg duplicate	new duplicate probability (NL_PROB_MIN..NL_PROB_MAX)
 */
void rtnl_netem_set_duplicate(struct rtnl_qdisc *qdisc, uint32_t duplicate)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_duplicate = duplicate;
	n->qnm_mask |= SCH_NETEM_HAS_DUPLICATE;
}

/**
 * Set latency of the netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg latency		new latency in ticks
 */
void rtnl_netem_set_latency(struct rtnl_qdisc *qdisc, uint32_t latency)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_latency = latency;
	n->qnm_mask |= SCH_NETEM_HAS_LATENCY;
}

/**
 * Set jitter of the netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg jitter		new jitter in ticks
 */
void rtnl_netem_set_jitter(struct rtnl_qdisc *qdisc, uint32_t jitter)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_jitter = jitter;
	n->qnm_mask |= SCH_NETEM_HAS_JITTER;
}

/**
 * Set delay correction probability of the netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg delay_corr	new delay correction probability (NL_PROB_MIN..NL_PROB_MAX)
 */
void rtnl_netem_set_delay_correction(struct rtnl_qdisc *qdisc,
				     uint32_t delay_corr)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_corr.nmc_delay = delay_corr;
	n->qnm_mask |= SCH_NETEM_HAS_DELAY_CORR;
}

/**
 * Set loss correction probability of the netem qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg loss_corr	new loss correction probability (NL_PROB_MIN..NL_PROB_MAX)
 */
void rtnl_netem_set_loss_correction(struct rtnl_qdisc *qdisc,
				     uint32_t loss_corr)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_corr.nmc_loss = loss_corr;
	n->qnm_mask |= SCH_NETEM_HAS_LOSS_CORR;
}

/**
 * Set duplication correction probability of the netem qdisc to the
 * specified value
 * @arg qdisc		qdisc to change
 * @arg dup_corr	new duplication correction probability (NL_PROB_MIN..NL_PROB_MAX)
 */
void rtnl_netem_set_duplication_correction(struct rtnl_qdisc *qdisc,
					   uint32_t dup_corr)
{
	struct rtnl_netem *n = get_netem(qdisc);
	n->qnm_corr.nmc_duplicate = dup_corr;
	n->qnm_mask |= SCH_NETEM_HAS_DUP_CORR;
}

/** @} */

static struct rtnl_qdisc_ops netem_ops = {
	.qo_kind		= "netem",
	.qo_msg_parser		= &netem_msg_parser,
	.qo_free_data		= &netem_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= &netem_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= &netem_dump_full,
	.qo_get_opts		= &netem_get_opts,
};

void __init netem_init(void)
{
	rtnl_qdisc_register(&netem_ops);
}

void __exit netem_exit(void)
{
	rtnl_qdisc_unregister(&netem_ops);
}

/** @} */
