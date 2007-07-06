/*
 * netlink/route/sch/dsmark.h	differentiated services marker
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

#ifndef NETLINK_DSMARK_H_
#define NETLINK_DSMARK_H_

#include <netlink/netlink.h>

/**
 * @name Available DSMARK Qdisc Attributes Flags
 * @ingroup dsmark
 * Flags used in rtnl_dsmark::qdm_mask to indicate which attributes are
 * present in a dsmark qdisc.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_dsmark_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if dsmark has the indices attribute set
 * @code
 * if (dsmark->qdm_mask & SCH_DSMARK_HAS_INDICES)
 *         // action goes here
 * @endcode
 * @{
 */

#define SCH_DSMARK_HAS_INDICES		0x1	/**< has indices */
#define SCH_DSMARK_HAS_DEFAULT_INDEX	0x2	/**< has default index */
#define SCH_DSMARK_HAS_SET_TC_INDEX	0x4	/**< has set tc index */

/** @} */

/**
 * DSMARK qdisc
 * @ingroup dsmark
 */
struct rtnl_dsmark_qdisc
{
	/**
	 * Indices
	 */
	uint16_t	qdm_indices;

	/**
	 * Default index
	 */
	uint16_t	qdm_default_index;

	/**
	 * Set tc_index flag
	 */
	uint32_t	qdm_set_tc_index;

	/**
	 * Available attributes mask
	 */
	uint32_t	qdm_mask;
};

/**
 * @name Available DSMARK Class Attributes Flags
 * @ingroup dsmark
 * Flags used in rtnl_dsmark::cdm_mask to indicate which attributes are
 * present in a dsmark class.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_dsmark_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if dsmark has the mask attribute set
 * @code
 * if (dsmark->cdm_mask & SCH_DSMARK_HAS_MASK)
 *         // action goes here
 * @endcode
 * @{
 */

#define SCH_DSMARK_HAS_MASK	0x1	/**< Has mask */
#define SCH_DSMARK_HAS_VALUE	0x2	/**< Has value */

/** @} */

/**
 * DSMARK class
 * @ingroup dsmark
 */
struct rtnl_dsmark_class
{
	/**
	 * Bitmap mask
	 */
	uint8_t		cdm_bmask;
	
	/**
	 * Value
	 */
	uint8_t		cdm_value;

	/**
	 * Available attributes mask
	 */
	uint32_t	cdm_mask;
};

extern void rtnl_class_dsmark_set_bmask(struct rtnl_class *, uint8_t);
extern void rtnl_class_dsmark_set_value(struct rtnl_class *, uint8_t);

extern void rtnl_qdisc_dsmark_set_indices(struct rtnl_qdisc *, uint16_t);
extern void rtnl_qdisc_dsmark_set_default_index(struct rtnl_qdisc *, uint16_t);
extern void rtnl_qdisc_dsmark_set_set_tc_index(struct rtnl_qdisc *, int);

#endif
