/*
 **************************************************************************
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
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
 * @file ecm_interface_ipsec.h
 *	ECM internet protocol security interface.
 */

#ifndef __ECM_INTERFACE_IPSEC_H__
#define __ECM_INTERFACE_IPSEC_H__

/**
 * @addtogroup ecm_interface_ipsec_subsystem
 * @{
 */

/**
 * ECM IPSec callback to which clients will register.
 */
struct ecm_interface_ipsec_callback{
   struct net_device *(*tunnel_get_and_hold)(struct net_device *dev, struct sk_buff *skb, int *if_type);
};

/**
 * Registers a client for IPSec callbacks.
 *
 * @param	cb	IPSec callback pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
void ecm_interface_ipsec_register_callbacks(struct ecm_interface_ipsec_callback *cb);

/**
 * Unregisters a client from IPSec callbacks.
 *
 * @return
 * None.
 */
void ecm_interface_ipsec_unregister_callbacks(void);

/**
 * @}
 */

#endif
