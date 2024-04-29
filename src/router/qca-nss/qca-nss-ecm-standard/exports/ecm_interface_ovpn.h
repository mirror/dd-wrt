/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */


/**
 * @file ecm_interface_ovpn.h
 *	ECM open virtual private network (OVPN) interface.
 */

#ifndef __ECM_INTERFACE_OVPN_H__
#define __ECM_INTERFACE_OVPN_H__

/**
 * @addtogroup ecm_interface_ovpn_subsystem
 * @{
 */

typedef void (*ecm_interface_ovpn_update_route_method_t)(struct net_device *dev, uint32_t *from_addr, uint32_t *to_addr, int version);
typedef int32_t (*ecm_interface_ovpn_get_ifnum_method_t)(struct net_device *dev, struct sk_buff *skb, struct net_device **tun_dev);

/*
 * ecm_interface_ovpn
 *	OVPN interface for supporting acceleration.
 */
struct ecm_interface_ovpn {
	ecm_interface_ovpn_update_route_method_t ovpn_update_route;	/**< Update OVPN route status. */
	ecm_interface_ovpn_get_ifnum_method_t ovpn_get_ifnum;		/**< Get NSS interface number for OVPN tunnel traffic. */
};

/**
 * Registers a client with the OVPN instance.
 *
 * @param	ovpn	OVPN instance context.
 *
 * @return
 * The status of the OVPN registration operation.
 */
int ecm_interface_ovpn_register(struct ecm_interface_ovpn *ovpn);

/**
 * Unregisters a client from its registered OVPN instance.
 *
 * @return
 * None.
 */
void ecm_interface_ovpn_unregister(void);

/**
 * @}
 */

#endif /* __ECM_INTERFACE_OVPN_H__ */
