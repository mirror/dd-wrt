/*
 * route/sch/sfq.c		SFQ qdisc
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
 * @defgroup sfq Stochastic Fairness Queueing (SFQ)
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/sch/sfq.h>

static inline struct rtnl_sfq *get_sfq(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata == NULL)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_sfq));

	return (struct rtnl_sfq *) qdisc->q_subdata;
}

static int sfq_msg_parser(struct rtnl_qdisc *q)
{
	struct rtnl_sfq *s;
	struct tc_sfq_qopt *opts;

	if (q->q_opts.d_size < sizeof(*opts))
		return nl_error(EINVAL, "SFQ specific options size mismatch");

	s = get_sfq(q);

	opts = (struct tc_sfq_qopt *) q->q_opts.d_data;

	s->qs_quantum = opts->quantum;
	s->qs_perturb = opts->perturb_period;
	s->qs_limit = opts->limit;
	s->qs_divisor = opts->divisor;
	s->qs_flows = opts->flows;

	s->qs_mask = (SCH_SFQ_HAS_QUANTUM | SCH_SFQ_HAS_PERTURB |
		      SCH_SFQ_HAS_LIMIT | SCH_SFQ_HAS_DIVISOR |
		      SCH_SFQ_HAS_FLOWS);

	return 0;
}

static void sfq_free_data(struct rtnl_qdisc *q)
{
	if (q->q_subdata) {
		free(q->q_subdata);
		q->q_subdata = NULL;
	}
}

static int sfq_dump_brief(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			  struct nl_dump_params *params, int line)
{
	struct rtnl_sfq *s = q->q_subdata;

	fprintf(fd, " quantum %u perturb %uus",
		s->qs_quantum, nl_ticks2us(s->qs_perturb*nl_get_hz()));

	return line;
}

static int sfq_dump_full(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			 struct nl_dump_params *params, int line)
{
	struct rtnl_sfq *s = q->q_subdata;

	fprintf(fd, "limit %u divisor %u flows %u",
		s->qs_limit, s->qs_divisor, s->qs_flows);

	return line;
}

static struct nl_msg *sfq_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_sfq *sfq = (struct rtnl_sfq *) qdisc->q_subdata;
	if (sfq) {
		struct nl_msg *msg = nl_msg_build(NULL);
		struct tc_sfq_qopt opts = {
			.quantum = sfq->qs_quantum,
			.perturb_period = sfq->qs_perturb,
			.limit = sfq->qs_limit
		};
		if (msg == NULL)
			return NULL;
		nl_msg_append_raw(msg, &opts, sizeof(opts));
		return msg;
	}

	return NULL;
}

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set quantum of sfq qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg quantum		new quantum in bytes
 */
void rtnl_sfq_set_quantum(struct rtnl_qdisc *qdisc, uint32_t quantum)
{
	struct rtnl_sfq *s = get_sfq(qdisc);
	s->qs_quantum = quantum;
	s->qs_mask |= SCH_SFQ_HAS_QUANTUM;
}

/**
 * Set limit of sfq qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg limit		new limit of queue in number of packets
 */
void rtnl_sfq_set_limit(struct rtnl_qdisc *qdisc, uint32_t limit)
{
	struct rtnl_sfq *s = get_sfq(qdisc);
	s->qs_limit = limit;
	s->qs_mask |= SCH_SFQ_HAS_LIMIT;
}

/**
 * Set perturbation interval of sfq qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg perturb		new perturb interval in seconds
 * @note A value of 0 disabled perturbation
 */
void rtnl_sfq_set_perturb(struct rtnl_qdisc *qdisc, uint32_t perturb)
{
	struct rtnl_sfq *s = get_sfq(qdisc);
	s->qs_perturb = perturb;
	s->qs_mask |= SCH_SFQ_HAS_PERTURB;
}

/** @} */

static struct rtnl_qdisc_ops sfq_ops = {
	.qo_kind		= "sfq",
	.qo_msg_parser		= &sfq_msg_parser,
	.qo_free_data		= &sfq_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= &sfq_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= &sfq_dump_full,
	.qo_get_opts		= &sfq_get_opts,
};

void __init sfq_init(void)
{
	rtnl_qdisc_register(&sfq_ops);
}

void __exit sfq_exit(void)
{
	rtnl_qdisc_unregister(&sfq_ops);
}

/** @} */
