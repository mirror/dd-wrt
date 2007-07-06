/*
 * netlink/route/sch/sfq.h	SFQ Qdisc
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

#ifndef NETLINK_SFQ_H_
#define NETLINK_SFQ_H_

#include <netlink/netlink.h>

/**
 * @name Available Attributes Flags
 * @ingroup sfq
 * Flags used in rtnl_sfq::qq_mask to indicate which attributes are
 * present in a SFQ qdisc.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_sfq_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if sfq qdisc has a limit attribute set
 * @code
 * if (sfq->qs_mask & SCH_SFQ_HAS_LIMIT)
 *         // action goes here
 * @endcode
 * @{
 */

#define SCH_SFQ_HAS_QUANTUM	0x01	/**< Has quantum */
#define SCH_SFQ_HAS_PERTURB	0x02	/**< Has perturb */
#define SCH_SFQ_HAS_LIMIT	0x04	/**< Has limit */
#define SCH_SFQ_HAS_DIVISOR	0x08	/**< Has divisor */
#define SCH_SFQ_HAS_FLOWS	0x10	/**< Has flows */

/** @} */

/**
 * Stochastic Fairness Queue
 * @ingroup sfq
 */
struct rtnl_sfq
{
	/**
	 * Number of bytes to send out per slot and round
	 */
	uint32_t	qs_quantum;

	/**
	 * Perturbation interval, i.e. timer period between changes
	 * of the hash functiong.
	 */
	uint32_t	qs_perturb;

	/**
	 * Upper limit of queue in number of packets before sfq starts
	 * dropping packets.
	 */
	uint32_t	qs_limit;

	/**
	 * Hash table divisor, i.e. size of hash table array (read-only).
	 */
	uint32_t	qs_divisor;

	/**
	 * Same as limit
	 */
	uint32_t	qs_flows;

	/**
	 * Available attributes mask
	 */
	uint32_t	qs_mask;
};

extern void rtnl_sfq_set_quantum(struct rtnl_qdisc *, uint32_t);
extern void rtnl_sfq_set_limit(struct rtnl_qdisc *, uint32_t);
extern void rtnl_sfq_set_perturb(struct rtnl_qdisc *, uint32_t);

#endif
