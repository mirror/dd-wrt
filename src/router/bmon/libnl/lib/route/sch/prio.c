/*
 * route/sch/prio.c	prio (qdisc|class)
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
 * @defgroup prio (Fast) Prio
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/sch/prio.h>

/**
 * @name Priority Band Translations
 * @{
 */

static struct trans_tbl prios[] = {
	__ADD(TC_PRIO_BESTEFFORT,besteffort)
	__ADD(TC_PRIO_FILLER,filler)
	__ADD(TC_PRIO_BULK,bulk)
	__ADD(TC_PRIO_INTERACTIVE_BULK,interactive_bulk)
	__ADD(TC_PRIO_INTERACTIVE,interactive)
	__ADD(TC_PRIO_CONTROL,control)
};

/**
 * Convert a priority band to a character string.
 * @arg prio		priority band
 *
 * Converts a priority band to a character string and stores it in a
 * static buffer.
 *
 * @return A static buffer or the type encoded in hexidecimal
 *         form if no match was found.
 * @attention This funnction is NOT thread safe.
 */
char * rtnl_prio2str(int prio)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return __type2str_r(prio, buf, sizeof(buf), prios, ARRAY_SIZE(prios));
}


/**
 * Convert a priority band to a character string (Reentrant).
 * @arg prio		priority band
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a priority band to a character string and stores it in
 * the specified destination buffer.
 *
 * @return The destination buffer or the type encoded in hexidecimal
 *         form if no match was found.
 */
char * rtnl_prio2str_r(int prio, char *buf, size_t len)
{
	return __type2str_r(prio, buf, len, prios, ARRAY_SIZE(prios));
}

/**
 * Convert a character string to a priority band
 * @arg name		name of priority band
 *
 * Converts the provided character string specifying a priority
 * band to the corresponding numeric value.
 *
 * @return priority band or a negative value if none was found.
 */
int rtnl_str2prio(const char *name)
{
	return __str2type(name, prios, ARRAY_SIZE(prios));
}

/** @} */

static inline struct rtnl_prio *get_prio(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata == NULL)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_prio));

	return (struct rtnl_prio *) qdisc->q_subdata;
}

static int prio_msg_parser(struct rtnl_qdisc *qdisc)
{
	struct rtnl_prio *pf;
	struct tc_prio_qopt *opt;

	if (qdisc->q_opts.d_size < sizeof(*opt))
		return nl_error(EINVAL, "prio specific option size mismatch");

	pf = get_prio(qdisc);
	if (pf == NULL)
		return nl_error(ENOMEM, "Out of memory");

	opt = (struct tc_prio_qopt *) qdisc->q_opts.d_data;

	pf->qp_bands = opt->bands;
	memcpy(pf->qp_priomap, opt->priomap, sizeof(pf->qp_priomap));
	pf->qp_mask = (SCH_PRIO_HAS_BANDS | SCH_PRIO_HAS_PRIOMAP);
	
	return 0;
}

static void prio_free_data(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata) {
		free(qdisc->q_subdata);
		qdisc->q_subdata = NULL;
	}
}

static int prio_dump_brief(struct nl_cache *cache, struct rtnl_qdisc *qdisc,
			   FILE *fd, struct nl_dump_params *params, int line)
{
	struct rtnl_prio *pf = (struct rtnl_prio *) qdisc->q_subdata;
	fprintf(fd, " bands %u", pf->qp_bands);
	return line;
}

static int prio_dump_full(struct nl_cache *cache, struct rtnl_qdisc *qdisc,
			  FILE *fd, struct nl_dump_params *params, int line)
{
	int i, hp;
	struct rtnl_prio *pf = (struct rtnl_prio *) qdisc->q_subdata;

	fprintf(fd, "priomap [");
	
	for (i = 0; i <= TC_PRIO_MAX; i++)
		fprintf(fd, "%u%s", pf->qp_priomap[i],
			i < TC_PRIO_MAX ? " " : "");

	fprintf(fd, "]\n");

	hp = (((TC_PRIO_MAX/2) + 1) & ~1);

	for (i = 0; i < hp; i++) {
		char a[32];
		fprintf(fd, "    %18s => %u",
			rtnl_prio2str_r(i, a, sizeof(a)),
			pf->qp_priomap[i]);
		if (hp+i <= TC_PRIO_MAX) {
			fprintf(fd, " %18s => %u",
				rtnl_prio2str_r(hp+i, a, sizeof(a)),
				pf->qp_priomap[hp+i]);
			if (i < (hp - 1))
				fprintf(fd, "\n");
		}
	}

	return line;
}

static struct nl_msg * prio_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_prio *prio = (struct rtnl_prio *) qdisc->q_subdata;
	if (prio && prio->qp_mask & (SCH_PRIO_HAS_BANDS | SCH_PRIO_HAS_PRIOMAP)) {
		struct tc_prio_qopt opts = { .bands = prio->qp_bands };
		struct nl_msg *msg = nl_msg_build(NULL);
		if (msg == NULL)
			return NULL;
		memcpy(opts.priomap, prio->qp_priomap, sizeof(opts.priomap));
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
 * Set number of bands of the prio qdisc to the specified value
 * @arg qdisc		the qdisc to change
 * @arg bands		number of bands
 * @return 0 on success or a negative error code.
 */
int rtnl_sch_prio_set_bands(struct rtnl_qdisc *qdisc, uint32_t bands)
{
	struct rtnl_prio *prio = get_prio(qdisc);
	prio->qp_bands = bands;
	prio->qp_mask |= SCH_PRIO_HAS_BANDS;
	return 0;
}

/**
 * Set priomap of the prio qdisc to the specified value
 * @arg qdisc		the qdisc to change
 * @arg priomap		new priomap
 * @arg len		length of priomap (# of elements)
 * @return 0 on success or a negative error code.
 */
int rtnl_sch_prio_set_priomap(struct rtnl_qdisc *qdisc, uint8_t priomap[],
			      int len)
{
	int i;
	struct rtnl_prio *prio = get_prio(qdisc);

	if (!(prio->qp_mask & SCH_PRIO_HAS_BANDS))
		return nl_error(EINVAL, "Set number of bands first");

	if (len > TC_PRIO_MAX)
		return nl_error(ERANGE, "priomap length out of bounds");

	for (i = 0; i < TC_PRIO_MAX; i++) {
		if (priomap[i] > prio->qp_bands)
			return nl_error(ERANGE, "priomap element %d " \
			    "out of bounds, increase bands number");
	}

	memcpy(prio->qp_priomap, priomap, len);
	prio->qp_mask |= SCH_PRIO_HAS_PRIOMAP;
	return 0;
}

/** @} */

static struct rtnl_qdisc_ops prio_ops = {
	.qo_kind		= "prio",
	.qo_msg_parser		= &prio_msg_parser,
	.qo_free_data		= &prio_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= &prio_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= &prio_dump_full,
	.qo_get_opts		= &prio_get_opts,
};

static struct rtnl_qdisc_ops pfifo_fast_ops = {
	.qo_kind		= "pfifo_fast",
	.qo_msg_parser		= &prio_msg_parser,
	.qo_free_data		= &prio_free_data,
	.qo_dump[NL_DUMP_BRIEF]	= &prio_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= &prio_dump_full,
	.qo_get_opts		= &prio_get_opts,
};

void __init prio_init(void)
{
	rtnl_qdisc_register(&prio_ops);
	rtnl_qdisc_register(&pfifo_fast_ops);
}

void __exit prio_exit(void)
{
	rtnl_qdisc_unregister(&prio_ops);
	rtnl_qdisc_unregister(&pfifo_fast_ops);
}

/** @} */
