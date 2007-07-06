/*
 * route/tca.c              generic TCA routines
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
 * @ingroup rtnl
 * @defgroup tc Traffic Control
 * Module to access and modify the traffic control extensions.
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/tc.h>
#include <netlink/route/rtattr.h>

int tca_msg_parser(struct nlmsghdr *n, struct rtnl_tca *g)
{
	struct rtattr *tb[TCA_MAX + 1];
	size_t len;
	struct tcmsg *tm;
	int err;

	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(struct tcmsg));

	if (len < 0)
		return nl_error(EINVAL, "Message too short to be a " \
		    "TCA message");

	tm = NLMSG_DATA(n);

	if ((err = nl_parse_rtattr(tb, TCA_MAX, TCA_RTA(tm), len)) < 0)
		return err;

	if (NULL == tb[TCA_KIND])
		return nl_error(EINVAL, "Missing tca kind TLV");

	if (RTA_PAYLOAD(tb[TCA_KIND]) >= (sizeof(g->tc_kind) - 1))
		return nl_error(EINVAL, "tca kind TLV length exceeds " \
			"local kind name limit");

	memcpy(g->tc_kind, RTA_DATA(tb[TCA_KIND]), RTA_PAYLOAD(tb[TCA_KIND]));

	g->tc_family  = tm->tcm_family;
	g->tc_ifindex = tm->tcm_ifindex;
	g->tc_handle  = tm->tcm_handle;
	g->tc_parent  = tm->tcm_parent;
	g->tc_info    = tm->tcm_info;

	g->tc_mask = (TCA_HAS_FAMILY|TCA_HAS_IFINDEX|TCA_HAS_HANDLE|
		TCA_HAS_PARENT|TCA_HAS_INFO|TCA_HAS_KIND);

	if (tb[TCA_OPTIONS]) {
		if ((err = nl_alloc_data_from_rtattr(&g->tc_opts, tb[TCA_OPTIONS])) < 0)
			return err;
		g->tc_mask |= TCA_HAS_OPTS;
	}
	

	if (tb[TCA_STATS2]) {
		struct rtattr *tbs[TCA_STATS_MAX + 1] = {0};

		err = nl_parse_rtattr(tbs, TCA_STATS_MAX,
			RTA_DATA(tb[TCA_STATS2]), RTA_PAYLOAD(tb[TCA_STATS2]));

		if (err < 0)
			return err;

		if (tbs[TCA_STATS_BASIC]) {
			struct gnet_stats_basic *bs;

			if (RTA_PAYLOAD(tbs[TCA_STATS_BASIC]) < sizeof(*bs))
				return nl_error(EINVAL, "Basic statistics " \
				    "TLV is too short");

			bs = RTA_DATA(tbs[TCA_STATS_BASIC]);
			
			g->tc_stats.tcs_basic.bytes   = bs->bytes;
			g->tc_stats.tcs_basic.packets = bs->packets;
		}

		if (tbs[TCA_STATS_RATE_EST]) {
			struct gnet_stats_rate_est *re;

			if (RTA_PAYLOAD(tbs[TCA_STATS_RATE_EST]) < sizeof(*re))
				return nl_error(EINVAL, "Rate Estimator TLV " \
				    "is too short");

			re = RTA_DATA(tbs[TCA_STATS_RATE_EST]);
			
			g->tc_stats.tcs_rate_est.bps = re->bps;
			g->tc_stats.tcs_rate_est.pps = re->pps;
		}
		
		if (tbs[TCA_STATS_QUEUE]) {
			struct gnet_stats_queue *q;

			if (RTA_PAYLOAD(tbs[TCA_STATS_QUEUE]) < sizeof(*q))
				return nl_error(EINVAL, "Queue statistics " \
				    "TLV is too short");

			q = RTA_DATA(tbs[TCA_STATS_QUEUE]);
			
			g->tc_stats.tcs_queue.qlen       = q->qlen;
			g->tc_stats.tcs_queue.backlog    = q->backlog;
			g->tc_stats.tcs_queue.drops      = q->drops;
			g->tc_stats.tcs_queue.requeues   = q->requeues;
			g->tc_stats.tcs_queue.overlimits = q->overlimits;
		}

		g->tc_mask |= TCA_HAS_STATS;
		
		if (tbs[TCA_STATS_APP]) {
			err = nl_alloc_data_from_rtattr(&g->tc_xstats, tbs[TCA_STATS_APP]);

			if (err < 0)
				return err;
		} else
			goto compat_xstats;
	} else {
		if (tb[TCA_STATS]) {
			struct tc_stats *st;

			if (RTA_PAYLOAD(tb[TCA_STATS]) < sizeof(*st))
				return nl_error(EINVAL, "tc statistics TLV " \
				    "is too short");

			st = (struct tc_stats *) RTA_DATA(tb[TCA_STATS]);

			g->tc_stats.tcs_basic.bytes      = st->bytes;
			g->tc_stats.tcs_basic.packets    = st->packets;
			g->tc_stats.tcs_rate_est.bps     = st->bps;
			g->tc_stats.tcs_rate_est.pps     = st->pps;
			g->tc_stats.tcs_queue.qlen       = st->qlen;
			g->tc_stats.tcs_queue.backlog    = st->backlog;
			g->tc_stats.tcs_queue.drops      = st->drops;
			g->tc_stats.tcs_queue.overlimits = st->overlimits;

			g->tc_mask |= TCA_HAS_STATS;
		}

compat_xstats:
		if (tb[TCA_XSTATS]) {
			err = nl_alloc_data_from_rtattr(&(g->tc_xstats), tb[TCA_XSTATS]);
			if (err < 0)
				return err;
			g->tc_mask |= TCA_HAS_XSTATS;
		}
	}


	return 0;
}

void tca_free_data(struct rtnl_tca *tca)
{
	if (tca->tc_opts.d_data) {
		free(tca->tc_opts.d_data);
		tca->tc_opts.d_data = NULL;
	}

	if (tca->tc_xstats.d_data) {
		free(tca->tc_xstats.d_data);
		tca->tc_xstats.d_data = NULL;
	}
}

int tca_dump_brief(struct nl_cache *c, struct rtnl_tca *g, const char *type,
		   FILE *fd, struct nl_dump_params *params, int line)
{
	char h[32], p[32];
	struct nl_cache *lc = nl_cache_lookup(RTNL_LINK);
	const char *dev = rtnl_link_i2name(lc, g->tc_ifindex);

	dp_new_line(fd, params, line++);
	fprintf(fd, "%s %s dev %s handle %s parent %s",
		g->tc_kind, type,
		dev ? : "none",
		nl_handle2str_r(g->tc_handle, h, sizeof(h)),
		nl_handle2str_r(g->tc_parent, p, sizeof(p)));

	return line;
}

int tca_dump_full(struct nl_cache *c, struct rtnl_tca *g, FILE *fd,
		  struct nl_dump_params *params, int line)
{
	return line;
}

int tca_dump_with_stats(struct nl_cache *c, struct rtnl_tca *g, FILE *fd,
			struct nl_dump_params *params, int line)
{
	char *unit, fmt[64];
	float res;
	strcpy(fmt, "        %7.2f %s %10u %10u %10u %10u %10u\n");

	dp_new_line(fd, params, line++);
	fprintf(fd, "    Stats:    bytes    packets      drops overlimits" \
		"       qlen    backlog\n");

	res = nl_cancel_down_bytes(g->tc_stats.tcs_basic.bytes, &unit);
	if (*unit == 'B')
		fmt[11] = '9';

	dp_new_line(fd, params, line++);
	fprintf(fd, fmt, res, unit,
		g->tc_stats.tcs_basic.packets,
		g->tc_stats.tcs_queue.drops,
		g->tc_stats.tcs_queue.overlimits,
		g->tc_stats.tcs_queue.qlen,
		g->tc_stats.tcs_queue.backlog);

	res = nl_cancel_down_bytes(g->tc_stats.tcs_rate_est.bps, &unit);

	strcpy(fmt, "        %7.2f %s/s%9u pps");

	if (*unit == 'B')
		fmt[11] = '9';

	dp_new_line(fd, params, line++);
	fprintf(fd, fmt, res, unit, g->tc_stats.tcs_rate_est.pps);

	return line;
}

int tca_filter(struct rtnl_tca *o, struct rtnl_tca *f)
{
#define ISSET(F) f->tc_mask & TCA_HAS_##F
	if ((ISSET(HANDLE)  && o->tc_handle != f->tc_handle) ||
	    (ISSET(PARENT)  && o->tc_parent != f->tc_parent) ||
	    (ISSET(IFINDEX) && o->tc_ifindex != f->tc_ifindex) ||
	    (ISSET(KIND)    && strcmp(o->tc_kind, f->tc_kind)))
		return 0;
#undef ISSET

	return 1;
}

void tca_set_ifindex(struct rtnl_tca *t, int ifindex)
{
	t->tc_ifindex = ifindex;
	t->tc_mask |= TCA_HAS_IFINDEX;
}

int tca_set_ifindex_name(struct rtnl_tca *t, struct nl_cache *c,
	const char *dev)
{
	int i = rtnl_link_name2i(c, dev);

	if (RTNL_LINK_NOT_FOUND == i)
		return nl_error(ENOENT, "Link %s is unknown", dev);
	tca_set_ifindex(t, i);
	return 0;
}

void tca_set_handle(struct rtnl_tca *t, uint32_t handle)
{
	t->tc_handle = handle;
	t->tc_mask |= TCA_HAS_HANDLE;
}

void tca_set_parent(struct rtnl_tca *t, uint32_t parent)
{
	t->tc_parent = parent;
	t->tc_mask |= TCA_HAS_PARENT;
}

void tca_set_kind(struct rtnl_tca *t, const char *kind)
{
	strncpy(t->tc_kind, kind, sizeof(t->tc_kind) - 1);
	t->tc_mask |= TCA_HAS_KIND;
}

/**
 * \name Traffic Control Handle Translations
 * \{
 */

/**
 * Convert a traffic control handle to a character string.
 * @arg handle		traffic control handle
 *
 * Converts a traffic control handle to a character string in the
 * form of \c MAJ:MIN and stores it in a static buffer.
 *
 * \return A static buffer or the type encoded in hexidecimal
 *         form if no match was found.
 * \attention This funnction is NOT thread safe.
 */
char * nl_handle2str(uint32_t handle)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return nl_handle2str_r(handle, buf, sizeof(buf));
}

/**
 * Convert a traffic control handle to a character string (Reentrant).
 * @arg handle		traffic control handle
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a tarffic control handle to a character string in the
 * form of \c MAJ:MIN and stores it in the specified destination buffer.
 *
 * \return The destination buffer or the type encoded in hexidecimal
 *         form if no match was found.
 */
char * nl_handle2str_r(uint32_t handle, char *buf, size_t len)
{
	if (TC_H_ROOT == handle)
		snprintf(buf, len, "root");
	else if (TC_H_UNSPEC == handle)
		snprintf(buf, len, "none");
	else if (0 == TC_H_MAJ(handle))
		snprintf(buf, len, ":%02x", TC_H_MIN(handle));
	else if (0 == TC_H_MIN(handle))
		snprintf(buf, len, "%02x:", TC_H_MAJ(handle) >> 16);
	else
		snprintf(buf, len, "%02x:%02x",
			TC_H_MAJ(handle) >> 16, TC_H_MIN(handle));

	return buf;
}

/**
 * Convert a charactering strint to a traffic control handle
 * @arg name		traffic control handle as character string
 * @arg res		destination buffer
 *
 * Converts the provided character string specifying a traffic
 * control handle to the corresponding numeric value.
 *
 * The handle must be provided in one of the following formats:
 *  - root
 *  - none
 *  - XXXX:
 *  - :YYYY
 *  - XXXX:YYYY
 *  - XXXXYYYY
 *
 * \return 0 on success or a negative error code
 * \exception EINVAL Invalid inupt format
 * \exception ERANGE Major or Minor part overflows or overlaps
 */
int nl_str2handle(const char *name, uint32_t *res)
{
	char *colon, *end;
	uint32_t h;

	if (!strcasecmp(name, "root")) {
		*res = TC_H_ROOT;
		return 0;
	}

	if (!strcasecmp(name, "none")) {
		*res = TC_H_UNSPEC;
		return 0;
	}

	h = strtoul(name, &colon, 16);

	if (colon == name) {
		/* :YYYY */
		h = 0;
		if (':' != *colon)
			return -EINVAL;
	}

	if (':' == *colon) {
		/* check if we would lose bits */
		if (TC_H_MAJ(h))
			return -ERANGE;
		h <<= 16;

		if ('\0' == colon[1]) {
			/* XXXX: */
			*res = h;
		} else {
			/* XXXX:YYYY */
			uint32_t l = strtoul(colon+1, &end, 16);

			/* check if we overlap with major part */
			if (TC_H_MAJ(l))
				return -ERANGE;

			if ('\0' != *end)
				return -EINVAL;

			*res = (h | l);
		}
	} else if ('\0' == *colon) {
		/* XXXXYYYY */
		*res = h;
	} else
		return -EINVAL;

	return 0;
}

/**
 * Copy kernel ratespec to libnl ratespec
 * @arg dst		destination ratespec
 * @arg src		source ratespec
 */
void rtnl_copy_ratespec(struct rtnl_ratespec *dst, struct tc_ratespec *src)
{
	dst->rs_cell_log = src->cell_log;
	dst->rs_feature = src->feature;
	dst->rs_addend = src->addend;
	dst->rs_mpu = src->mpu;
	dst->rs_rate = src->rate;
}

/** @} */
