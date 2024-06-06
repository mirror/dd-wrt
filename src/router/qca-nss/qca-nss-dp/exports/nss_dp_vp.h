/*
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __NSS_DP_VP_H__
#define __NSS_DP_VP_H__

/*
 * nss_dp_vp_tx_info
 *	VP Tx info.
 */
struct nss_dp_vp_tx_info {
	uint32_t flags;			/**< VP Tx flags. */
	uint8_t sc;			/**< Service code. */
	uint8_t svp;			/**< Source VP number. */
	bool fake_mac;			/**< Needs Fake Mac. */
};

/*
 * nss_dp_vp_rx_info
 *	VP info struct struct
 */
struct nss_dp_vp_rx_info {
	uint8_t dvp;			/* Destination VP number */
	uint8_t svp;			/* Source VP number */
	uint16_t l3offset;		/* L3 offset of packet */
	uint8_t ip_summed;		/* IP checksum */
	struct napi_struct *napi;	/* RX NAPI */
};

/*
 * nss_dp_vp_skb_list
 *	skb list of a VP
 */
struct nss_dp_vp_skb_list{
	struct nss_dp_vp_skb_list *next;
	struct sk_buff_head skb_list;	/* skb list*/
	uint16_t len;			/* Total data length carried by these skb*/
	uint8_t dvp;			/* Destination VP */
};

/*
 * nss_dp_vp_list_rx_cb_t
 *	Vp rx handler callback typedef
 */
typedef void (*nss_dp_vp_list_rx_cb_t)( struct nss_dp_vp_skb_list *vp_rx_list);

/*
 * nss_dp_vp_rx_cb_t
 *	Vp rx handler callback typedef
 */
typedef void (*nss_dp_vp_rx_cb_t)(struct sk_buff *skb, struct nss_dp_vp_rx_info *vprxi);

/**
 * nss_dp_vp_rx_register_cb
 *	Register handler for VP rx processing.
 *
 * @datatypes
 * nss_dp_vp_rx_cb_t
 * nss_dp_vp_list_rx_cb_t
 *
 * @param[in] nss_dp_vp_tx_info Pointer to VP rx handler.
 * @param[in] nss_dp_vp_list_rx_cb_t Pointer to VP list handler.
 *
 * @return
 * True or false.
 */
bool nss_dp_vp_rx_register_cb(nss_dp_vp_rx_cb_t cb, \
		nss_dp_vp_list_rx_cb_t list_cb);

/**
 * nss_dp_vp_rx_unregister_cb
 *	Unregister VP handler for VP rx processing.
 *
 * @datatypes
 * None.
 *
 * @param[in] nss_dp_vp_tx_info Pointer to VP rx handler.
 *
 * @return
 * None.
 */
void nss_dp_vp_rx_unregister_cb(void);

/**
 * nss_dp_vp_xmit
 *	Transmits a packet to the appropriate VP netdevice.
 *
 * @datatypes
 * net_device
 * nss_dp_vp_tx_info
 * sk_buff
 *
 * @param[in] net_device Pointer to the netdev structure.
 * @param[in] nss_dp_vp_tx_info Pointer to the VP info structure.
 * @param[in] skb Pointer to the packet.
 *
 * @return
 * Tx status.
 */
netdev_tx_t nss_dp_vp_xmit(struct net_device *netdev, struct nss_dp_vp_tx_info *info, struct sk_buff *skb);

/**
 * nss_dp_vp_init()
 *	Initialize virtual port netdevice.
 *
 * @return
 * Netdevice for the VP port.
 */
struct net_device *nss_dp_vp_init(void);

/**
 * nss_dp_vp_deinit()
 *	De-initialize virtual port netdevice.
 *
 * @return
 * Status of virtual port deinit.
 */
bool nss_dp_vp_deinit(struct net_device *netdev);

#endif	/** __NSS_DP_VP_H__ */
