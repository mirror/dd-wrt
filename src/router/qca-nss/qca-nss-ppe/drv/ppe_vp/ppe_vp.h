/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

struct ppe_vp_base;

#define PPE_VP_FLAG_VP_ACTIVE		0x01	/* VP is active */
#define PPE_VP_FLAG_VP_FAST_XMIT	0x02	/* Enable fast_xmit when delivering to VP interface */
#define PPE_VP_HW_PORT_STATS_MS		1000

/*
 * ppe_vp_rx_info
 *	Data structure for sending data to VP from EDMA.
 */
struct ppe_vp_rx_info {
	uint8_t ppe_port_num;				/* PPE port number */
};

/*
 * ppe_vp_priv
 *	Virtual port data priv structure for its netdev.
 */
struct ppe_vp_priv {
	struct ppe_vp *vp;			/* Pointer to the PPE VP object */
};

/*
 * ppe_vp
 *	Virtual port data structure.
 */
struct ppe_vp {
	struct ppe_vp_base *pvb;			/* Pointer to the PPE VP base object */
	struct net_device *netdev;			/* net_device for this VP */
	struct net_device *vp_dev;			/* net_device for this VP */
	struct ppe_drv_iface *ppe_iface;		/* Pointer to the PPE interface object */

	struct ppe_vp_stats vp_stats;			/* Stats for this VP */
	ppe_vp_stats_callback_t stats_cb;		/* Tunnel statistics callback */

	uint32_t netdev_if_num;				/* net_device interface number */
	uint16_t mtu;					/* MTU for VP */
	ppe_vp_num_t port_num;				/* PPE Port number */
	ppe_vp_type_t vp_type;				/* Virtual Port type */
	uint32_t flags;					/* Flags associated with the VP */
	spinlock_t lock;				/* Lock for VP instance */

	ppe_vp_callback_t dst_cb;			/* Packet to interface for transmit callback */
	ppe_vp_list_callback_t dst_list_cb;		/* skb list interface for transmit callback */
	void *dst_cb_data;				/* Callback data */
	ppe_vp_callback_t src_cb;			/* Packet to be handed over to stack by VP user callback */
	void *src_cb_data;				/* Callback data */
};
