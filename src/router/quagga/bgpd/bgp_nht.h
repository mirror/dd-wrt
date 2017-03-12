/* BGP Nexthop tracking
 * Copyright (C) 2013 Cumulus Networks, Inc.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _BGP_NHT_H
#define _BGP_NHT_H

/**
 * bgp_parse_nexthop_update() - parse a nexthop update message from Zebra.
 */
void bgp_parse_nexthop_update (void);

/**
 * bgp_nexthop_check() - check if the bnc object is valid.
 * ARGUMENTS:
 *   p - path for which the nexthop object is being looked up
 *   connected - True if NH MUST be a connected route
 */
int bgp_nexthop_check (struct bgp_info *, int connected);

/**
 * bgp_ensure_nexthop() - Ensure a bgp_nexthop_cache object exists for 
 *  the given prefix or peer.  If an existing one is not found,
 *  create a new object and register with ZEBRA for nexthop
 *  notification.
 * ARGUMENTS:
 *   afi: AFI_IP or AF_IP6
 *     struct bgp_info *: path for which the nexthop object is 
 *                        being looked up
 *   OR
 *     struct peer The BGP peer associated with this NHT
 *   connected - True if NH MUST be a connected route
 */
int bgp_ensure_nexthop (struct bgp_info *, struct peer *, int connected);

/**
 * bgp_unlink_nexthop() - Unlink the nexthop object from the path structure.
 * ARGUMENTS:
 *   struct bgp_info *: path structure.
 */
void bgp_unlink_nexthop (struct bgp_info *);

/**
 * bgp_unlink_nexthop() - Unlink the nexthop object for the given peer.
 */
extern void bgp_unlink_nexthop(struct bgp_info *p);
void bgp_unlink_nexthop_by_peer (struct peer *);

#endif /* _BGP_NHT_H */
