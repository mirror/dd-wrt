/*
 * route/sch/fifo.c	(p|b)fifo (qdisc|class)
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
 * @defgroup fifo FIFO (packet/byte)
 * Packet/Byte based FIFO queueing discipline.
 *
 * The FIFO qdisc comes in two flavours:
 * @par bfifo (Byte FIFO)
 * Allows enqueuing until the currently queued volume in bytes exceeds
 * the configured limit.backlog contains currently enqueued volume in bytes.
 *
 * @par pfifo (Packet FIFO)
 * Allows enquueing until the currently queued number of packets
 * exceeds the configured limit.
 *
 * The configuration is exactly the same, the decision which of
 * the two variations is going to be used is made based on the
 * kind of the filter (rtnl_filter_set_kind()).
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/sch/fifo.h>
#include <netlink/helpers.h>

static inline struct rtnl_fifo *get_fifo(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata == NULL)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_fifo));

	return (struct rtnl_fifo *) qdisc->q_subdata;
}

static int fifo_msg_parser(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fifo *f;

	if (qdisc->q_opts.d_size < sizeof(*f))
		return nl_error(EINVAL, "FIFO specific options size mismatch");

	f = get_fifo(qdisc);
	if (f == NULL)
		return nl_error(ENOMEM, "Out of memory");

	f->qf_limit = *(uint32_t *) qdisc->q_opts.d_data;
	f->qf_mask = SCH_FIFO_HAS_LIMIT;
	return 0;
}

static void fifo_free_data(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata) {
		free(qdisc->q_subdata);
		qdisc->q_subdata = NULL;
	}
}

static int pfifo_dump_brief(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			    struct nl_dump_params *params, int line)
{
	struct rtnl_fifo *f = (struct rtnl_fifo *) q->q_subdata;
	fprintf(fd, " limit %u packets", f->qf_limit);
	return line;
}

static int bfifo_dump_brief(struct nl_cache *c, struct rtnl_qdisc *q, FILE *fd,
			    struct nl_dump_params *params, int line)
{
	double r;
	char *unit;
	struct rtnl_fifo *f = (struct rtnl_fifo *) q->q_subdata;

	r = nl_cancel_down_bytes(f->qf_limit, &unit);
	fprintf(fd, " limit %.1f%s", r, unit);
	return line;
}

static struct nl_msg *fifo_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_fifo *fifo = (struct rtnl_fifo *) qdisc->q_subdata;
	if (fifo && fifo->qf_mask & SCH_FIFO_HAS_LIMIT) {
		struct tc_fifo_qopt opts = { .limit = fifo->qf_limit };
		struct nl_msg *msg = nl_msg_build(NULL);
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
 * Set queue limit of the fifo qdisc to the specified value
 * @arg qdisc		the qdisc to change
 * @arg limit		new limit
 */
void rtnl_sch_fifo_set_limit(struct rtnl_qdisc *qdisc, uint32_t limit)
{
	struct rtnl_fifo *f = get_fifo(qdisc);
	f->qf_limit = limit;
	f->qf_mask |= SCH_FIFO_HAS_LIMIT;
}

/** @} */


static struct rtnl_qdisc_ops pfifo_ops = {
	.qo_kind		= "pfifo",
	.qo_msg_parser		= &fifo_msg_parser,
	.qo_free_data		= &fifo_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= &pfifo_dump_brief,
	.qo_get_opts		= &fifo_get_opts,
};

static struct rtnl_qdisc_ops bfifo_ops = {
	.qo_kind		= "bfifo",
	.qo_msg_parser		= &fifo_msg_parser,
	.qo_free_data		= &fifo_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= &bfifo_dump_brief,
	.qo_get_opts		= &fifo_get_opts,
};

void __init fifo_init(void)
{
	rtnl_qdisc_register(&pfifo_ops);
	rtnl_qdisc_register(&bfifo_ops);
}

void __exit fifo_exit(void)
{
	rtnl_qdisc_unregister(&pfifo_ops);
	rtnl_qdisc_unregister(&bfifo_ops);
}

/** @} */
