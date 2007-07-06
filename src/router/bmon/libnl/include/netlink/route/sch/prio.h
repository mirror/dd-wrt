/*
 * netlink/route/sch/prio.h	prio
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

#ifndef NETLINK_PRIO_H_
#define NETLINK_PRIO_H_

#include <netlink/netlink.h>

/**
 * @name Available Attributes Flags
 * @ingroup prio
 * Flags used in rtnl_prio::qp_mask to indicate which attributes are
 * present in a prio qdisc.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_prio_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if prio qdisc has a bands attribute set
 * @code
 * if (prio->qp_mask & SCH_PRIO_HAS_BANDS)
 *         // action goes here
 * @endcode
 * @{
 */

#define SCH_PRIO_HAS_BANDS	1
#define SCH_PRIO_HAS_PRIOMAP	2

/** @} */

/**
 * (Fast) Prio Qdisc
 * @ingroup prio
 */
struct rtnl_prio
{
	/**
	 * Number of bands
	 */
	uint32_t	qp_bands;

	/**
	 * Map: logical priority -> PRIO band
	 */
	uint8_t		qp_priomap[TC_PRIO_MAX+1];

	/**
	 * Available attributes mask
	 */
	uint32_t	qp_mask;
};

/**
 * Default number of bands
 * @ingroup prio
 */
#define SCH_PRIO_DEFAULT_BANDS 3

/**
 * Default priomap
 * @ingroup prio
 * @code
 * uint8_t map[] = SCH_PRIO_DEFAULT_PRIOMAP;
 * rtnl_sch_prio_set_priomap(prio, map, sizeof(map));
 * @endcode
 */
#define SCH_PRIO_DEFAULT_PRIOMAP { 1, 2, 2, 2, 1, 2, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 }

extern int rtnl_sch_prio_set_bands(struct rtnl_qdisc *, uint32_t);
extern int rtnl_sch_prio_set_priomap(struct rtnl_qdisc *, uint8_t[], int);

extern char *	rtnl_prio2str(int);
extern char *	rtnl_prio2str_r(int, char *, size_t);
extern int	rtnl_str2prio(const char *);

#endif
