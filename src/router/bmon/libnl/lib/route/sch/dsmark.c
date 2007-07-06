/*
 * dsmark.c             rtnetlink dsmark (qdisc|class)
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
 * @ingroup class
 * @defgroup dsmark Differentiated Services Marker (DSMARK)
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/route/rtattr.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/link.h>
#include <netlink/route/sch/dsmark.h>
#include <netlink/helpers.h>

static inline struct rtnl_dsmark_qdisc *get_dsmark_qdisc(struct rtnl_qdisc *qdisc)
{
	if (qdisc->q_subdata == NULL)
		qdisc->q_subdata = calloc(1, sizeof(struct rtnl_dsmark_qdisc));

	return (struct rtnl_dsmark_qdisc *) qdisc->q_subdata;
}

static int dsmark_qdisc_msg_parser(struct rtnl_qdisc *q)
{
	int err;
	struct rtattr *tb[TCA_DSMARK_MAX + 1] = {0};
	struct rtnl_dsmark_qdisc *d = NULL;

	err = nl_parse_rtattr(tb, TCA_DSMARK_MAX, (void *) q->q_opts.d_data,
			      q->q_opts.d_size);
	if (err < 0)
		return err;

	d = get_dsmark_qdisc(q);

	if (tb[TCA_DSMARK_INDICES]) {
		err = NL_COPY_DATA(d->qdm_indices, tb[TCA_DSMARK_INDICES]);
		if (err < 0)
			goto err_out;
		d->qdm_mask |= SCH_DSMARK_HAS_INDICES;
	}

	if (tb[TCA_DSMARK_DEFAULT_INDEX]) {
		err = NL_COPY_DATA(d->qdm_default_index,
				   tb[TCA_DSMARK_DEFAULT_INDEX]);
		if (err < 0)
			goto err_out;
		d->qdm_mask |= SCH_DSMARK_HAS_DEFAULT_INDEX;
	}

	if (tb[TCA_DSMARK_SET_TC_INDEX]) {
		d->qdm_set_tc_index = 1;
		d->qdm_mask |= SCH_DSMARK_HAS_SET_TC_INDEX;
	}

	return 0;

err_out:
	if (d)
		free(d);
	
	return err;
}

static inline struct rtnl_dsmark_class *get_dsmark_class(struct rtnl_class *class)
{
	if (class->c_subdata == NULL)
		class->c_subdata = calloc(1, sizeof(struct rtnl_dsmark_class));

	return (struct rtnl_dsmark_class *) class->c_subdata;
}


static int dsmark_class_msg_parser(struct rtnl_class *c)
{
	int err;
	struct rtattr *tb[TCA_DSMARK_MAX + 1] = {0};
	struct rtnl_dsmark_class *d = NULL;

	err = nl_parse_rtattr(tb, TCA_DSMARK_MAX, (void *) c->c_opts.d_data,
			      c->c_opts.d_size);
	if (err < 0)
		return err;

	d = get_dsmark_class(c);

	if (tb[TCA_DSMARK_MASK]) {
		err = NL_COPY_DATA(d->cdm_bmask, tb[TCA_DSMARK_MASK]);
		if (err < 0)
			goto err_out;
		d->cdm_mask |= SCH_DSMARK_HAS_MASK;
	}

	if (tb[TCA_DSMARK_VALUE]) {
		err = NL_COPY_DATA(d->cdm_value, tb[TCA_DSMARK_VALUE]);
		if (err < 0)
			goto err_out;
		d->cdm_mask |= SCH_DSMARK_HAS_VALUE;
	}

	return 0;

err_out:
	if (d)
		free(d);
	
	return err;
}

static int dsmark_qdisc_dump_brief(struct nl_cache *c, struct rtnl_qdisc *q,
				   FILE *fd, struct nl_dump_params *params,
				   int line)
{
	struct rtnl_dsmark_qdisc *d = (struct rtnl_dsmark_qdisc *) q->q_subdata;

	if (NULL == d)
		return line;

	if (d->qdm_mask & SCH_DSMARK_HAS_INDICES)
		fprintf(fd, " indices 0x%04x", d->qdm_indices);

	return line;
}

static int dsmark_qdisc_dump_full(struct nl_cache *c, struct rtnl_qdisc *q,
				  FILE *fd, struct nl_dump_params *params,
				  int line)
{
	struct rtnl_dsmark_qdisc *d = (struct rtnl_dsmark_qdisc *) q->q_subdata;

	if (NULL == d)
		return line;

	if (d->qdm_mask & SCH_DSMARK_HAS_DEFAULT_INDEX)
		fprintf(fd, " default index 0x%04x", d->qdm_default_index);

	if (d->qdm_mask & SCH_DSMARK_HAS_SET_TC_INDEX)
		fprintf(fd, " set-tc-index");

	return line;
}

static int dsmark_class_dump_brief(struct nl_cache *c, struct rtnl_class *cl,
				   FILE *fd, struct nl_dump_params *params,
				   int line)
{
	struct rtnl_dsmark_class *d = (struct rtnl_dsmark_class *) cl->c_subdata;

	if (NULL == d)
		return line;

	if (d->cdm_mask & SCH_DSMARK_HAS_VALUE)
		fprintf(fd, " value 0x%02x", d->cdm_value);

	if (d->cdm_mask & SCH_DSMARK_HAS_MASK)
		fprintf(fd, " mask 0x%02x", d->cdm_bmask);

	return line;
}

static struct nl_msg *dsmark_qdisc_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_dsmark_qdisc *d;
	d = (struct rtnl_dsmark_qdisc *) qdisc->q_subdata;

	if (d) {
		struct nl_msg *msg = nl_msg_build(NULL);

		if (d->qdm_mask & SCH_DSMARK_HAS_INDICES)
			nl_msg_append_tlv(msg, TCA_DSMARK_INDICES,
			    &d->qdm_indices, sizeof(d->qdm_indices));

		if (d->qdm_mask & SCH_DSMARK_HAS_DEFAULT_INDEX)
			nl_msg_append_tlv(msg, TCA_DSMARK_DEFAULT_INDEX,
			    &d->qdm_default_index, sizeof(d->qdm_default_index));
		if (d->qdm_mask & SCH_DSMARK_HAS_SET_TC_INDEX)
			nl_msg_append_tlv(msg, TCA_DSMARK_SET_TC_INDEX,
			    NULL, 0);

		return msg;
	}

	return NULL;
}

static struct nl_msg *dsmark_class_get_opts(struct rtnl_class *class)
{
	struct rtnl_dsmark_class *d;
	d = (struct rtnl_dsmark_class *) class->c_subdata;

	if (d) {
		struct nl_msg *msg = nl_msg_build(NULL);

		if (d->cdm_mask & SCH_DSMARK_HAS_MASK)
			nl_msg_append_tlv(msg, TCA_DSMARK_MASK,
			    &d->cdm_bmask, sizeof(d->cdm_bmask));

		if (d->cdm_mask & SCH_DSMARK_HAS_VALUE)
			nl_msg_append_tlv(msg, TCA_DSMARK_VALUE,
			    &d->cdm_value, sizeof(d->cdm_value));

		return msg;
	}

	return NULL;
}

/**
 * @name Class Attribute Modification
 * @{
 */

/**
 * Set bitmask of the dsmark class to the specified value
 * @arg class		the class to change
 * @arg mask		new bitmask
 */
void rtnl_class_dsmark_set_bmask(struct rtnl_class *class, uint8_t mask)
{
	struct rtnl_dsmark_class *dc = get_dsmark_class(class);
	dc->cdm_bmask = mask;
	dc->cdm_mask |= SCH_DSMARK_HAS_MASK;
}

/**
 * Set value of the dsmark class to the specified value
 * @arg class		the class to change
 * @arg value		new value
 */
void rtnl_class_dsmark_set_value(struct rtnl_class *class, uint8_t value)
{
	struct rtnl_dsmark_class *dc = get_dsmark_class(class);
	dc->cdm_value = value;
	dc->cdm_mask |= SCH_DSMARK_HAS_VALUE;
}

/** @} */

/**
 * @name Qdisc Attribute Modification
 * @{
 */

/**
 * Set indices of the dsmark qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg indices		new indices
 */
void rtnl_qdisc_dsmark_set_indices(struct rtnl_qdisc *qdisc, uint16_t indices)
{
	struct rtnl_dsmark_qdisc *dq = get_dsmark_qdisc(qdisc);
	dq->qdm_indices = indices;
	dq->qdm_mask |= SCH_DSMARK_HAS_INDICES;
}

/**
 * Set default index of the dsmark qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg default_index	new default index
 */
void rtnl_qdisc_dsmark_set_default_index(struct rtnl_qdisc *qdisc,
					 uint16_t default_index)
{
	struct rtnl_dsmark_qdisc *dq = get_dsmark_qdisc(qdisc);
	dq->qdm_default_index = default_index;
	dq->qdm_mask |= SCH_DSMARK_HAS_DEFAULT_INDEX;
}

/**
 * Set "set tc index" of the dsmark qdisc to the specified value
 * @arg qdisc		qdisc to change
 * @arg flag		flag indicating whether to enable or disable
 */
void rtnl_qdisc_dsmark_set_set_tc_index(struct rtnl_qdisc *qdisc, int flag)
{
	struct rtnl_dsmark_qdisc *dq = get_dsmark_qdisc(qdisc);
	dq->qdm_set_tc_index = !!flag;
	dq->qdm_mask |= SCH_DSMARK_HAS_SET_TC_INDEX;
}

/** @} */

static struct rtnl_qdisc_ops dsmark_qdisc_ops = {
	.qo_kind		= "dsmark",
	.qo_msg_parser		= &dsmark_qdisc_msg_parser,
	.qo_dump[NL_DUMP_BRIEF]	= &dsmark_qdisc_dump_brief,
	.qo_dump[NL_DUMP_FULL]	= &dsmark_qdisc_dump_full,
	.qo_get_opts		= &dsmark_qdisc_get_opts,
};

static struct rtnl_class_ops dsmark_class_ops = {
	.co_kind		= "dsmark",
	.co_msg_parser		= &dsmark_class_msg_parser,
	.co_dump[NL_DUMP_BRIEF]	= &dsmark_class_dump_brief,
	.co_get_opts		= &dsmark_class_get_opts,
};

void __init dsmark_init(void)
{
	rtnl_qdisc_register(&dsmark_qdisc_ops);
	rtnl_class_register(&dsmark_class_ops);
}

void __exit dsmark_exit(void)
{
	rtnl_qdisc_unregister(&dsmark_qdisc_ops);
	rtnl_class_unregister(&dsmark_class_ops);
}

/** @} */
