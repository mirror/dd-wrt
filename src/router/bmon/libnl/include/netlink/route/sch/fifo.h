/*
 * netlink/route/sch/fifo.h	(p|b)fifo
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

#ifndef NETLINK_FIFO_H_
#define NETLINK_FIFO_H_

#include <netlink/netlink.h>

/**
 * @name Available Attributes Flags
 * @ingroup fifo
 * Flags used in rtnl_fifo::qf_mask to indicate which attributes are
 * present in a FIFO qdisc.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_fifo_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if fifo has a limit attribute set
 * @code
 * if (fifo->qf_mask & SCH_FIFO_HAS_LIMIT)
 *         // action goes here
 * @endcode
 * @{
 */

#define SCH_FIFO_HAS_LIMIT 1	/**< Has limit */

/** @} */

/**
 * FIFO Qdisc
 * @ingroup fifo
 */
struct rtnl_fifo
{
	/**
	 * Queue length: bytes for bfifo, packets for pfifo
	 */
	uint32_t	qf_limit;

	/**
	 * Available attributes mask
	 */
	uint32_t	qf_mask;
};

extern void rtnl_sch_fifo_set_limit(struct rtnl_qdisc *, uint32_t);

#endif
