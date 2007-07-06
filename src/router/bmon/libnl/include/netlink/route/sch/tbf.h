/*
 * netlink/route/sch/tbf.h	TBF Qdisc/Class
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

#ifndef NETLINK_TBF_H_
#define NETLINK_TBF_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>

/**
 * @name Available Attributes Flags
 * @ingroup tbf
 * Flags used in rtnl_tbf::qt_mask to indicate which attributes are
 * present in a TBF qdisc.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_tbf_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if tbf has a limit attribute set
 * @code
 * if (tbf->qt_mask & SCH_TBF_HAS_LIMIT)
 *         // action goes here
 * @endcode
 * @{
 */

#define SCH_TBF_HAS_LIMIT	0x01	/**< Has limit */
#define SCH_TBF_HAS_BUFFER	0x02	/**< Has buffer */
#define SCH_TBF_HAS_MTU		0x04	/**< Has MTU */
#define SCH_TBF_HAS_RATE	0x08	/**< Has rate */
#define SCH_TBF_HAS_PEAKRATE	0x10	/**< Has peakrate */

/** @} */

/**
 * Tocket Bucket Filter Qdisc
 * @ingroup tbf
 */
struct rtnl_tbf
{
	uint32_t		qt_limit;
	uint32_t		qt_buffer;
	uint32_t		qt_mtu;
	struct rtnl_ratespec	qt_rate;
	struct rtnl_ratespec	qt_peakrate;
	uint32_t		qt_mask;
};

extern double rtnl_tbf_calc_latency(const double, const double,
				    const uint32_t);
extern double rtnl_tbf_get_rate_latency(struct rtnl_qdisc *);
extern double rtnl_tbf_get_peak_latency(struct rtnl_qdisc *);
extern double rtnl_tbf_get_buffer(struct rtnl_qdisc *);
extern double rtnl_tbf_get_mtu(struct rtnl_qdisc *);

extern void rtnl_tbf_set_limit(struct rtnl_qdisc *, uint32_t);

#endif
