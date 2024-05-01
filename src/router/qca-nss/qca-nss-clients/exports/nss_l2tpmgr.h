/*
 **************************************************************************
 * Copyright (c) 2019, 2021 The Linux Foundation.  All rights reserved.
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

typedef int32_t (*get_ipsec_ifnum_by_dev_callback_t)(struct net_device *);
typedef int32_t (*get_ipsec_ifnum_by_ip_addr_callback_t)(uint8_t ipversion, uint32_t *src_ip, uint32_t *dest_ip);

/**
 * l2tpmgr_ipsecmgr_cb
 *	Callback to get the dummy IPSec interface number that was used to register with NSS
 *	by the IPSec manager when given the IPSec Linux dev.
 *	get_ifnum_by_dev: passes net_device ptr to get associated IPsec if_num in KLIPS
 *	get_ifnum_by_ip_addr: passes IPv4 src & dest addr in Big-endian form to get IPsec if_num in XFRM
 */
struct l2tpmgr_ipsecmgr_cb {
	get_ipsec_ifnum_by_dev_callback_t get_ifnum_by_dev;
	get_ipsec_ifnum_by_ip_addr_callback_t get_ifnum_by_ip_addr;
};

#if defined(NSS_L2TP_IPSEC_BIND_BY_NETDEV)
/**
 * l2tpmgr_register_ipsecmgr_callback_by_netdev
 * 	Register IPSecmgr callback function with l2tpmgr by netdev for KLIPS.
 *
 * @datatypes
 * l2tpmgr_ipsecmgr_cb \n
 *
 * @param[in] cb    IPSecmgr callback function to be registered with l2tpmgr.
 *
 * @return
 * none
 */
void l2tpmgr_register_ipsecmgr_callback_by_netdev(struct l2tpmgr_ipsecmgr_cb *cb);

/**
 * l2tpmgr_unregister_ipsecmgr_callback
 * 	Unregister IPSecmgr callback function with l2tpmgr by netdev for KLIPS.
 *
 * @return
 * none
 */
void l2tpmgr_unregister_ipsecmgr_callback_by_netdev(void);
#endif

/**
 * l2tpmgr_register_ipsecmgr_callback_by_ipaddr
 * 	Register IPSecmgr callback function with l2tpmgr by IP address for XFRM.
 *
 * @datatypes
 * l2tpmgr_ipsecmgr_cb \n
 *
 * @param[in] cb    IPSecmgr callback function to be registered with l2tpmgr.
 *
 * @return
 * none
 */
void l2tpmgr_register_ipsecmgr_callback_by_ipaddr(struct l2tpmgr_ipsecmgr_cb *cb);

/**
 * l2tpmgr_unregister_ipsecmgr_callback_by_ipaddr
 * 	Unregister IPSecmgr callback function with l2tpmgr by IP address for XFRM.
 *
 * @return
 * none
 */
void l2tpmgr_unregister_ipsecmgr_callback_by_ipaddr(void);
#endif
