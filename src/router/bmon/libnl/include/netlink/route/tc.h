/*
 * tc.h          libnl tca gneric routines
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

#ifndef NETLINK_TC_H_
#define NETLINK_TC_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/data.h>

#define TCA_HAS_HANDLE    0x001
#define TCA_HAS_PARENT    0x002
#define TCA_HAS_IFINDEX   0x004
#define TCA_HAS_KIND      0x008
#define TCA_HAS_FAMILY    0x010
#define TCA_HAS_INFO      0x020
#define TCA_HAS_OPTS      0x040
#define TCA_HAS_STATS     0x080
#define TCA_HAS_XSTATS    0x100

/**
 * Rate Specification
 * @ingroup tc
 */
struct rtnl_ratespec
{
	uint8_t			rs_cell_log;
	uint16_t		rs_feature;
	uint16_t		rs_addend;
	uint16_t		rs_mpu;
	uint32_t		rs_rate;
};

struct rtnl_tstats
{
	struct {
		uint64_t            bytes;
		uint64_t            packets;
	} tcs_basic;

	struct {
		uint32_t            bps;
		uint32_t            pps;
	} tcs_rate_est;

	struct {
		uint32_t            qlen;
		uint32_t            backlog;
		uint32_t            drops;
		uint32_t            requeues;
		uint32_t            overlimits;
	} tcs_queue;
};

#define NL_TCA_GENERIC(pre)				\
	NLHDR_COMMON					\
	uint32_t		pre ##_family;		\
	uint32_t		pre ##_ifindex;		\
	uint32_t		pre ##_handle;		\
	uint32_t		pre ##_parent;		\
	uint32_t		pre ##_info;		\
	char			pre ##_kind[32];	\
	struct nl_data		pre ##_opts;		\
	struct rtnl_tstats	pre ##_stats;		\
	struct nl_data		pre ##_xstats;		\
	void *			pre ##_subdata;		\
	int			pre ##_mask;

struct rtnl_tca
{
	NL_TCA_GENERIC(tc)
};

extern char *	nl_handle2str(uint32_t);
extern char *	nl_handle2str_r(uint32_t, char *, size_t);
extern int	nl_str2handle(const char *, uint32_t *);

extern void rtnl_copy_ratespec(struct rtnl_ratespec *, struct tc_ratespec *);

#endif
