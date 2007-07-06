/*
 * lib/route/sch/netem.c		Network Emulator Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup qdisc
 * @defgroup netem Network Emulator
 * @brief
 *
 * For further documentation see http://linux-net.osdl.org/index.php/Netem
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/qdisc-modules.h>
#include <netlink/route/sch/netem.h>

/** @cond SKIP */
#define SCH_NETEM_ATTR_LATENCY		0x001
#define SCH_NETEM_ATTR_LIMIT		0x002
#define SCH_NETEM_ATTR_LOSS		0x004
#define SCH_NETEM_ATTR_GAP		0x008
#define SCH_NETEM_ATTR_DUPLICATE	0x010
#define SCH_NETEM_ATTR_JITTER		0x020
#define SCH_NETEM_ATTR_DELAY_CORR	0x040
#define SCH_NETEM_ATTR_LOSS_CORR	0x080
#define SCH_NETEM_ATTR_DUP_CORR		0x100
#define SCH_NETEM_ATTR_RO_PROB		0x200
#define SCH_NETEM_ATTR_RO_CORR		0x400
/** @endcond */

static inline struct rtnl_netem *netem_qdisc(struct rtnl_qdisc *qdisc)
{
	return (struct rtnl_netem *) qdisc->q_subdata;
}

static inline struct rtnl_netem *netem_alloc(struct rtnl_qdisc *qdisc)
{
	if (!qdisc->q_subdata)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_netem));

	return netem_qdisc(qdisc);
}

static struct nla_policy netem_policy[TCA_NETEM_MAX+1] = {
	[TCA_NETEM_CORR]	= { .minlen = sizeof(struct tc_netem_corr) },
	[TCA_NETEM_REORDER]	= { .minlen = sizeof(struct tc_netem_reorder) },
};

static int netem_msg_parser(struct rtnl_qdisc *qdisc)
{
	int len, err = 0;
	struct rtnl_netem *netem;
	struct tc_netem_qopt *opts;

	if (qdisc->q_opts->d_size < sizeof(*opts))
		return nl_error(EINVAL, "Netem specific options size mismatch");

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	opts = (struct tc_netem_qopt *) qdisc->q_opts->d_data;
	netem->qnm_latency = opts->latency;
	netem->qnm_limit = opts->limit;
	netem->qnm_loss = opts->loss;
	netem->qnm_gap = opts->gap;
	netem->qnm_duplicate = opts->duplicate;
	netem->qnm_jitter = opts->jitter;

	netem->qnm_mask = (SCH_NETEM_ATTR_LATENCY | SCH_NETEM_ATTR_LIMIT |
			   SCH_NETEM_ATTR_LOSS | SCH_NETEM_ATTR_GAP |
			   SCH_NETEM_ATTR_DUPLICATE | SCH_NETEM_ATTR_JITTER);

	len = qdisc->q_opts->d_size - sizeof(*opts);

	if (len > 0) {
		struct nlattr *tb[TCA_NETEM_MAX+1];

		err = nla_parse(tb, TCA_NETEM_MAX, (struct nlattr *)
				qdisc->q_opts->d_data + sizeof(*opts),
				len, netem_policy);
		if (err < 0) {
			free(netem);
			return err;
		}

		if (tb[TCA_NETEM_CORR]) {
			struct tc_netem_corr cor;

			nla_memcpy(&cor, tb[TCA_NETEM_CORR], sizeof(cor));
			netem->qnm_corr.nmc_delay = cor.delay_corr;
			netem->qnm_corr.nmc_loss = cor.loss_corr;
			netem->qnm_corr.nmc_duplicate = cor.dup_corr;

			netem->qnm_mask |= (SCH_NETEM_ATTR_DELAY_CORR |
					    SCH_NETEM_ATTR_LOSS_CORR |
					    SCH_NETEM_ATTR_DELAY_CORR);
		}

		if (tb[TCA_NETEM_REORDER]) {
			struct tc_netem_reorder ro;

			nla_memcpy(&ro, tb[TCA_NETEM_REORDER], sizeof(ro));
			netem->qnm_ro.nmro_probability = ro.probability;
			netem->qnm_ro.nmro_correlation = ro.correlation;

			netem->qnm_mask |= (SCH_NETEM_ATTR_RO_PROB |
					    SCH_NETEM_ATTR_RO_CORR);
		}
	}

	return 0;
}

static void netem_free_data(struct rtnl_qdisc *qdisc)
{
	free(qdisc->q_subdata);
}

static int netem_dump_brief(struct rtnl_qdisc *qdisc, struct nl_dump_params *p,
			    int line)
{
	struct rtnl_netem *netem = netem_qdisc(qdisc);

	if (netem)
		dp_dump(p, "limit %d", netem->qnm_limit);

	return line;
}

static int netem_dump_full(struct rtnl_qdisc *qdisc, struct nl_dump_params *p,
			   int line)
{
	return line;
}

static struct nl_msg *netem_get_opts(struct rtnl_qdisc *qdisc)
{
	return NULL;
}

/**
 * @name Queue Limit
 * @{
 */

/**
 * Set limit of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg limit		New limit in bytes.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_limit(struct rtnl_qdisc *qdisc, int limit)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);
	
	netem->qnm_limit = limit;
	netem->qnm_mask |= SCH_NETEM_ATTR_LIMIT;

	return 0;
}

/**
 * Get limit of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Limit in bytes or a negative error code.
 */
int rtnl_netem_get_limit(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_LIMIT))
		return netem->qnm_limit;
	else
		return nl_errno(ENOENT);
}

/** @} */

/**
 * @name Packet Re-ordering
 * @{
 */

/**
 * Set re-ordering gap of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg gap		New gap in number of packets.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_gap(struct rtnl_qdisc *qdisc, int gap)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_gap = gap;
	netem->qnm_mask |= SCH_NETEM_ATTR_GAP;

	return 0;
}

/**
 * Get re-ordering gap of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Re-ordering gap in packets or a negative error code.
 */
int rtnl_netem_get_gap(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_GAP))
		return netem->qnm_gap;
	else
		return nl_errno(ENOENT);
}

/**
 * Set re-ordering probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New re-ordering probability.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_reorder_probability(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_ro.nmro_probability = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_RO_PROB;

	return 0;
}

/**
 * Get re-ordering probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Re-ordering probability or a negative error code.
 */
int rtnl_netem_get_reorder_probability(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_RO_PROB))
		return netem->qnm_ro.nmro_probability;
	else
		return nl_errno(ENOENT);
}

/**
 * Set re-order correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New re-ordering correlation probability.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_reorder_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_ro.nmro_correlation = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_RO_CORR;

	return 0;
}

/**
 * Get re-ordering correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Re-ordering correlation probability or a negative error code.
 */
int rtnl_netem_get_reorder_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_RO_CORR))
		return netem->qnm_ro.nmro_correlation;
	else
		return nl_errno(ENOENT);
}

/** @} */

/**
 * @name Packet Loss
 * @{
 */

/**
 * Set packet loss probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New packet loss probability.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_loss(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_loss = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_LOSS;

	return 0;
}

/**
 * Get packet loss probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet loss probability or a negative error code.
 */
int rtnl_netem_get_loss(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_LOSS))
		return netem->qnm_loss;
	else
		return nl_errno(ENOENT);
}

/**
 * Set packet loss correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob	New packet loss correlation.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_loss_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_corr.nmc_loss = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_LOSS_CORR;

	return 0;
}

/**
 * Get packet loss correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet loss correlation probability or a negative error code.
 */
int rtnl_netem_get_loss_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_LOSS_CORR))
		return netem->qnm_corr.nmc_loss;
	else
		return nl_errno(ENOENT);
}

/** @} */

/**
 * @name Packet Duplication
 * @{
 */

/**
 * Set packet duplication probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob	New packet duplication probability.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_duplicate(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_duplicate = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_DUPLICATE;

	return 0;
}

/**
 * Get packet duplication probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet duplication probability or a negative error code.
 */
int rtnl_netem_get_duplicate(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_DUPLICATE))
		return netem->qnm_duplicate;
	else
		return nl_errno(ENOENT);
}

/**
 * Set packet duplication correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New packet duplication correlation probability.
 * @return 0 on sucess or a negative error code.
 */
int rtnl_netem_set_duplicate_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_corr.nmc_duplicate = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_DUP_CORR;

	return 0;
}

/**
 * Get packet duplication correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet duplication correlation probability or a negative error code.
 */
int rtnl_netem_get_duplicate_correlation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_DUP_CORR))
		return netem->qnm_corr.nmc_duplicate;
	else
		return nl_errno(ENOENT);
}

/** @} */

/**
 * @name Packet Delay
 * @{
 */

/**
 * Set packet delay of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg delay		New packet delay in micro seconds.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_delay(struct rtnl_qdisc *qdisc, int delay)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_latency = nl_us2ticks(delay);
	netem->qnm_mask |= SCH_NETEM_ATTR_LATENCY;

	return 0;
}

/**
 * Get packet delay of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet delay in micro seconds or a negative error code.
 */
int rtnl_netem_get_delay(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_LATENCY))
		return nl_ticks2us(netem->qnm_latency);
	else
		return nl_errno(ENOENT);
}

/**
 * Set packet delay jitter of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg jitter		New packet delay jitter in micro seconds.
 * @return 0 on success or a negative error code.
 */
int rtnl_netem_set_jitter(struct rtnl_qdisc *qdisc, int jitter)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_jitter = nl_us2ticks(jitter);
	netem->qnm_mask |= SCH_NETEM_ATTR_JITTER;

	return 0;
}

/**
 * Get packet delay jitter of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet delay jitter in micro seconds or a negative error code.
 */
int rtnl_netem_get_jitter(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_JITTER))
		return nl_ticks2us(netem->qnm_jitter);
	else
		return nl_errno(ENOENT);
}

/**
 * Set packet delay correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc to be modified.
 * @arg prob		New packet delay correlation probability.
 */
int rtnl_netem_set_delay_correlation(struct rtnl_qdisc *qdisc, int prob)
{
	struct rtnl_netem *netem;

	netem = netem_alloc(qdisc);
	if (!netem)
		return nl_errno(ENOMEM);

	netem->qnm_corr.nmc_delay = prob;
	netem->qnm_mask |= SCH_NETEM_ATTR_DELAY_CORR;

	return 0;
}

/**
 * Get packet delay correlation probability of netem qdisc.
 * @arg qdisc		Netem qdisc.
 * @return Packet delay correlation probability or a negative error code.
 */
int rtnl_netem_get_delay_corellation(struct rtnl_qdisc *qdisc)
{
	struct rtnl_netem *netem;

	netem = netem_qdisc(qdisc);
	if (netem && (netem->qnm_mask & SCH_NETEM_ATTR_DELAY_CORR))
		return netem->qnm_corr.nmc_delay;
	else
		return nl_errno(ENOENT);
}

/** @} */

static struct rtnl_qdisc_ops netem_ops = {
	.qo_kind		= "netem",
	.qo_msg_parser		= netem_msg_parser,
	.qo_free_data		= netem_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= netem_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= netem_dump_full,
	.qo_get_opts		= netem_get_opts,
};

static void __init netem_init(void)
{
	rtnl_qdisc_register(&netem_ops);
}

static void __exit netem_exit(void)
{
	rtnl_qdisc_unregister(&netem_ops);
}

/** @} */
