/*
 ***************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

/**
 * nss_mirror_public.h
 *	NSS Mirror interface definitions.
 */
#ifndef __NSS_MIRROR_H
#define __NSS_MIRROR_H

#include <nss_api_if.h>

/**
 * nss_mirror_disable
 *	API to send disable message to mirror interface in NSS firmware.
 *
 * @datatypes
 * struct net_device \n
 *
 * @param[in] mirror_dev  Mirror netdevice pointer.
 *
 * @return
 * Return 0 on success, -1 on failure.
 */
extern int nss_mirror_disable(struct net_device *mirror_dev);

/**
 * nss_mirror_enable
 *	API to send enable message to mirror interface in NSS firmware.
 *
 * @datatypes
 * struct net_device \n
 *
 * @param[in] mirror_dev  Mirror netdevice pointer.
 *
 * @return
 * Return 0 on success, -1 on failure.
 */
extern int nss_mirror_enable(struct net_device *mirror_dev);

/**
 * nss_mirror_set_nexthop
 *	API to send set nexthop message to mirror interface in NSS firmware.
 *
 * @datatypes
 * struct net_device \n
 *
 * @param[in] mirror_dev       Mirror netdevice pointer.
 * @param[in] mirror_next_hop  Nexthop interface number.
 *
 * @return
 * Return 0 on success, -1 on failure.
 */
extern int nss_mirror_set_nexthop(struct net_device *mirror_dev, int32_t mirror_next_hop);

/**
 * nss_mirror_reset_nexthop
 *	API to send reset nexthop message to mirror interface in NSS firmware.
 *
 * @datatypes
 * struct net_device \n
 *
 * @param[in] mirror_dev  Mirror netdevice pointer.
 *
 * @return
 * Return 0 on success, -1 on failure.
 */
extern int nss_mirror_reset_nexthop(struct net_device *mirror_dev);

/**
 * nss_mirror_configure
 *	API to send configure message to mirror interface in NSS firmware.
 *
 * @datatypes
 * struct net_device \n
 * nss_mirror_pkt_clone_point \n
 *
 * @param[in] mirror_dev        Mirror netdevice pointer.
 * @param[in] pkt_clone_point   Point in the packet to copy from.
 * @param[in] pkt_clone_size    Number of bytes to copy.
 * @param[in] pkt_clone_offset  Copy offset.
 *
 * @return
 * Return 0 on success, -1 on failure.
 */
extern int nss_mirror_configure(struct net_device *mirror_dev, enum nss_mirror_pkt_clone_point pkt_clone_point,
		 uint16_t pkt_clone_size, uint16_t pkt_clone_offset);

/**
 * nss_mirror_destroy
 *	API to de-register and delete mirror interface.
 *
 * @datatypes
 * struct net_device \n
 *
 * @param[in] mirror_dev  Mirror netdevice pointer.
 *
 * @return
 * Return 0 on success, -1 on failure.
 */
extern int nss_mirror_destroy(struct net_device *mirror_dev);

/**
 * nss_mirror_create
 *	API to create and register mirror interface.
 *
 * @datatypes
 * struct net_device \n
 *
 * @return
 * Return pointer of the newly created device.
 */

extern struct net_device *nss_mirror_create(void);

#endif /* __NSS_MIRROR_H */
