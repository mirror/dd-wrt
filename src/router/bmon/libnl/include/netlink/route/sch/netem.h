/*
 * netlink/route/sch/netem.h	Network Emulator
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

#ifndef NETLINK_NETEM_H_
#define NETLINK_NETEM_H_

#include <netlink/netlink.h>

/**
 * @name Available Attributes Flags
 * @ingroup netem
 * Flags used in rtnl_netem::qne_mask to indicate which attributes are
 * present in a netem qdisc.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_netem_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if sfq qdisc has a limit attribute set
 * @code
 * if (sfq->qs_mask & SCH_SFQ_HAS_LIMIT)
 *         // action goes here
 * @endcode
 * @{
 */

#define SCH_NETEM_HAS_LATENCY		0x001
#define SCH_NETEM_HAS_LIMIT		0x002
#define SCH_NETEM_HAS_LOSS		0x004
#define SCH_NETEM_HAS_GAP		0x008
#define SCH_NETEM_HAS_DUPLICATE		0x010
#define SCH_NETEM_HAS_JITTER		0x020
#define SCH_NETEM_HAS_DELAY_CORR	0x040
#define SCH_NETEM_HAS_LOSS_CORR		0x080
#define SCH_NETEM_HAS_DUP_CORR		0x100

/** @} */

/**
 * Network Emulator Corrections
 * @ingroup netem
 */
struct rtnl_netem_corr
{
	uint32_t	nmc_delay;
	uint32_t	nmc_loss;
	uint32_t	nmc_duplicate;
};

/**
 * Network Emulator
 * @ingroup netem
 */
struct rtnl_netem
{
	uint32_t		qnm_latency;
	uint32_t		qnm_limit;
	uint32_t		qnm_loss;
	uint32_t		qnm_gap;
	uint32_t		qnm_duplicate;
	uint32_t		qnm_jitter;
	uint32_t		qnm_mask;
	struct rtnl_netem_corr	qnm_corr;
};

extern void rtnl_netem_set_limit(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_gap(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_loss(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_duplicate(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_latency(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_jitter(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_delay_correction(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_loss_correction(struct rtnl_qdisc *, uint32_t);
extern void rtnl_netem_set_duplication_correction(struct rtnl_qdisc *,
						  uint32_t);

#endif
