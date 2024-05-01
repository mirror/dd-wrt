/*
 **************************************************************************
 * Copyright (c) 2019 The Linux Foundation.  All rights reserved.
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
#ifndef __NSS_L2TPMGR_H__
#define __NSS_L2TPMGR_H__

typedef struct net_device *(*get_ipsec_tundev_callback_t)(struct net_device *dev);

/**
 * l2tpmgr_ipsecmgr_cb
 *	Callback to get the dummy IPSec netdev that was
 *	used to register with NSS by the IPSec manager
 *	when given the IPSec Linux dev.
 */
struct l2tpmgr_ipsecmgr_cb {
	get_ipsec_tundev_callback_t cb;
	/**< IPSec mgr Callback> */
};

/**
 * l2tpmgr_register_ipsecmgr_callback
 * 	Register IPSecmgr callback function with l2tpmgr.
 *
 * @datatypes
 * l2tpmgr_ipsecmgr_cb \n
 *
 * @param[in] cb    IPSecmgr callback function to be registered with l2tpmgr.
 *
 * @return
 * none
 */
void l2tpmgr_register_ipsecmgr_callback(struct l2tpmgr_ipsecmgr_cb *cb);

/**
 * l2tpmgr_unregister_ipsecmgr_callback
 * 	Unregister IPSecmgr callback function with l2tpmgr.
 *
 * @return
 * none
 */
void l2tpmgr_unregister_ipsecmgr_callback(void);

#endif
