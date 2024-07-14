/*
 *  QCA Multicast ECM APIs
 *
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef MC_ECM_H_
#define MC_ECM_H_

#include <linux/skbuff.h>
#include <linux/types.h>

/*
 * Bridge destination interface list lookup for IPv4 multicast
 * brdev      - net_device of bridge
 * origin     – source unicast address
 * group      – multicast destination address
 * max_dst    - the size of the ‘dst_dev’ array being passed in
 * dst_dev    - the array of ‘ifindex’ for the destination interfaces, to be filled in by the callee
 * Return     - number of valid interfaces being returned in dst_dev
 *              or negative on errors
 */
int hyfi_bridge_ipv4_mc_get_if(struct net_device *brdev, __be32 origin, __be32 group,
                                     uint32_t max_dst, uint32_t dst_dev[]);

/*
 * Bridge snoop table update callback for IPv4
 */
typedef void (*hyfi_bridge_ipv4_mc_update_callback_t)(struct net_device *brdev, uint32_t group);


/*
 * Bridge snoop table update callback registration functions
 * Return     - 0 on success, negative on errors
 */
int hyfi_bridge_ipv4_mc_update_callback_register(hyfi_bridge_ipv4_mc_update_callback_t snoop_event_cb);


/*
 * Bridge snoop table update callback deregistration functions
 * Return     - 0 on success, negative on errors
 */
int hyfi_bridge_ipv4_mc_update_callback_deregister(void);

/*
 * Get the callback fuctions of IPv4
 */
hyfi_bridge_ipv4_mc_update_callback_t hyfi_bridge_ipv4_mc_update_callback_get(void);

/*
 * Bridge destination interface list lookup for IPv6 multicast
 * brdev      - net_device of bridge
 * origin     – source unicast address
 * group      – multicast destination address
 * max_dst    - the size of the ‘dst_dev’ array being passed in
 * dst_dev    - the array of ‘ifindex’ for the destination interfaces, to be filled in by the callee
 * Return     - number of valid interfaces being returned in dst_dev
 *              or ngeative on errors
 */
int hyfi_bridge_ipv6_mc_get_if(struct net_device *brdev, struct in6_addr *origin, struct in6_addr *group,
                                     uint32_t max_dst, uint32_t dst_dev[]);

/*
 * Bridge snoop table update callback for IPv4
 */
typedef void (*hyfi_bridge_ipv6_mc_update_callback_t)(struct net_device *brdev, struct in6_addr *group);


/*
 * Bridge snoop table update callback registration functions
 * Return     - 0 on success, negative on errors
 */
int hyfi_bridge_ipv6_mc_update_callback_register(hyfi_bridge_ipv6_mc_update_callback_t snoop_event_cb);


/*
 * Bridge snoop table update callback deregistration functions
 * Return     - 0 on success, negative on errors
 */
int hyfi_bridge_ipv6_mc_update_callback_deregister (void);


/*
 * Get the callback fuctions of IPv6
 */
hyfi_bridge_ipv6_mc_update_callback_t hyfi_bridge_ipv6_mc_update_callback_get(void);

#endif /* MC_ECM_H_ */

