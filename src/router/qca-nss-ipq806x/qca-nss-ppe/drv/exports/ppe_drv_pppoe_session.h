/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * @file ppe_drv_pppoe_session.h
 *	NSS PPE PPPOE session definitions.
 */

#ifndef _PPE_DRV_PPPOE_SESSION_H_
#define _PPE_DRV_PPPOE_SESSION_H_

/**
 * ppe_drv_pppoe_session_deinit
 *	Uninitialize PPPoE session in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for PPPoE.
 *
 * @return
 * Status of the de-init.
 */
ppe_drv_ret_t ppe_drv_pppoe_session_deinit(struct ppe_drv_iface *pppoe_iface);

/**
 * ppe_drv_pppoe_session_init
 *	Initialize PPPoE session in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] ppe_iface   Pointer to the PPE interface allocated for PPPoE.
 * @param[in] base_dev    Pointer to the base net device on which PPPoE interface is created.
 * @param[in] session_id  PPPoE session ID.
 * @param[in] server_mac  PPPoE server MAC address.
 * @param[in] local_mac   PPPoE interface local MAC address.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_pppoe_session_init(struct ppe_drv_iface *pppoe_iface, struct net_device *base_dev,
					uint16_t session_id, uint8_t *server_mac, uint8_t *local_mac);

#endif /* _PPE_DRV_PPPOE_SESSION_H_ */
