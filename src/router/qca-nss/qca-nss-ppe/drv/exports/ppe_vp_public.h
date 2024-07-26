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

/**
 * @file ppe_vp_public.h
 *	NSS PPE VP Public definitions.
 */

#ifndef _PPE_VP_PUBLIC_H_
#define _PPE_VP_PUBLIC_H_

#include <linux/module.h>
#include <ppe_drv_port.h>

#define PPE_VP_FLAG_DISABLE_TTL_DEC	0x1	/**< Set = TTL Decrement disabled, clear = TTL Decrement enabled */
#define PPE_VP_FLAG_REDIR_ENABLE	0x2	/**< When set, the packets destined to VP are redirect to VP queue without RPS */

#define PPE_VP_DS_INVALID_NODE_ID	0xFF	/**< Invalid node id value */

/**
 * @addtogroup ppe_vp_public_subsystem
 * @{
 */

/**
 * ppe_vp_hw_stats_t
 *	 PPE VP port statistics.
 */
typedef struct ppe_drv_port_hw_stats ppe_vp_hw_stats_t;

/**
 * ppe_vp_status
 *	Types of PPE VP statuses.
 */
typedef enum ppe_vp_status {
	PPE_VP_STATUS_SUCCESS = 0,	/**< VP Success */
	PPE_VP_STATUS_INVALID_TYPE,	/**< VP Invalid */
	PPE_VP_STATUS_PPEIFACE_ALLOC_FAIL,
					/**< VP PPE iface alloc failed */
	PPE_VP_STATUS_GET_VP_FAIL,	/**< VP PPE init failed */
	PPE_VP_STATUS_MAC_CLEAR_FAIL,	/**< VP PPE init failed */
	PPE_VP_STATUS_MAC_SET_FAIL,	/**< VP PPE init failed */
	PPE_VP_STATUS_MTU_SET_FAIL,	/**< VP PPE init failed */
	PPE_VP_STATUS_VP_INIT_FAIL,	/**< VP PPE init failed */
	PPE_VP_STATUS_VP_DEINIT_FAIL,	/**< VP PPE init failed */
	PPE_VP_STATUS_PORT_INVALID,	/**< VP PPE init failed */
	PPE_VP_STATUS_VP_ALLOC_FAIL,	/**< VP alloc failed */
	PPE_VP_STATUS_VP_FREE_FAIL,	/**< VP free  failed */
	PPE_VP_STATUS_FAILURE,		/**< VP Failure */
	PPE_VP_STATUS_VP_QUEUE_SET_FAILED,
					/**< VP to Queue map failed */
	PPE_VP_STATUS_HW_VP_STATS_CLEAR_FAILED,
					/**< Failed to clear PPE VP hardware statistics */
	PPE_VP_STATUS_UPDATE_FAIL,	/**< VP PPE update failed */
	PPE_VP_STATUS_MAX,		/**< Maximum VP statuses */
} ppe_vp_status_t;

typedef int16_t ppe_vp_num_t;

/**
 * ppe_vp_cb_info
 *	Information for VP callback
 *	to process exception packet.
 */
struct ppe_vp_cb_info {
	uint8_t ip_summed;		/**< IP checksum */
	struct sk_buff *skb;		/**< skb */
	struct napi_struct *napi;	/**< RX napi */
};

/**
 * Callback function for VP Rx.
 *
 * @datatypes
 * net_device
 * sk_buff
 *
 * @param[in] ppe_vp_cb_info	Pointer to exception information.
 * @param[in] cb_data		Pointer to the callback data.
 */
typedef bool(*ppe_vp_callback_t)(struct ppe_vp_cb_info *, void *cb_data);

/**
 * Callback function for VP Rx list.
 *
 * @datatypes
 * net_device
 * sk_buff
 *
 * @param[in] net_device  	Pointer to the net device.
 * @param[in] sk_buff_head	Pointer to the skb list head.
 * @param[in] cb_data     	Pointer to the callback data.
 */
typedef bool(*ppe_vp_list_callback_t)(struct net_device *, struct sk_buff_head *, void *cb_data);

/**
 * ppe_vp_type
 *	Types of VPs
 */
typedef enum ppe_vp_type {
	PPE_VP_TYPE_SW_L2,		/**< VP type for L2 SW interfaces */
	PPE_VP_TYPE_SW_L3,		/**< VP type for L3 SW interfaces */
	PPE_VP_TYPE_SW_PO,		/**< VP type for point offload tunnels */
	PPE_VP_TYPE_HW_L2TUN,		/**< VP type for L2 HW tunnels */
	PPE_VP_TYPE_HW_L3TUN,		/**< VP type for L3 HW tunnels */
	PPE_VP_TYPE_MAX,		/**< Maximum VP types */
} ppe_vp_type_t;

/**
 * Callback function for VP HW port statistics.
 *
 * @datatypes
 * net_device
 * ppe_vp_hw_stats_t
 *
 * @param[in] net_device Pointer to the net device.
 * @param[in,out] ppe_vp_hw_stats_t Pointer to PPE-VP HW stats structure.
 */
typedef bool(*ppe_vp_stats_callback_t)(struct net_device *, ppe_vp_hw_stats_t *);

/*
 * ppe_vp_user_type
 *	Types of VPs user
 */
enum ppe_vp_user_type {
	PPE_VP_USER_TYPE_NONE = 0,	/**< Non VP use case >*/
	PPE_VP_USER_TYPE_PASSIVE,	/**< VP for Passive use case */
	PPE_VP_USER_TYPE_ACTIVE,	/**< VP for Active use case */
	PPE_VP_USER_TYPE_DS,		/**< VP for Direct-Switch use case */
	PPE_VP_USER_TYPE_MAX,		/**< Maximum VP User types */
};

/*
 * ppe_vp_netdev_type
 *	Types of VP netdev
 */
enum ppe_vp_net_dev_type {
	PPE_VP_NET_DEV_TYPE_WIFI = 1,	/**< VP netdev is of type Wi-Fi */
	PPE_VP_NET_DEV_TYPE_MAX,		/**< Maximum VP netdev types */
};

/*
 * ppe_vp_net_dev_pvt_flags
 *	Flags of a netdev
 */
enum ppe_vp_net_dev_pvt_flags {
	PPE_VP_NET_DEV_FLAG_IS_MLD = 1,	/**< Is MLD net dev */
	PPE_VP_NET_DEV_FLAG_MAX,		/**< Maximum netdev flags */
};

/**
 * ppe_vp_ui
 *	Data structure for VP update information.
 */
struct ppe_vp_ui {
	uint8_t core_mask;		/**< Updated Core to be used for a particular VP flow */
	enum ppe_vp_user_type usr_type;	/**< VP user type */
};

/**
 * ppe_vp_ai
 *	Data structure VP allocation.
 */
struct ppe_vp_ai {
	ppe_vp_type_t type;		/**< VP type */
	ppe_vp_callback_t dst_cb;	/**< VP dst callback */
	ppe_vp_list_callback_t dst_list_cb;	/**< VP dst callback */
	void *dst_cb_data;		/**< VP dst callback data */
	ppe_vp_callback_t src_cb;	/**< VP src callback */
	void *src_cb_data;		/**< VP src callback data */
	ppe_vp_stats_callback_t stats_cb;
					/**< VP src callback */
	uint32_t xmit_port;		/**< Physical port number */
	uint32_t flags;			/**< PPE VP flags */
	uint8_t queue_num;		/**< Queue number */
	uint8_t core_mask;		/**< Core to be used for a particular VP flow */
	ppe_vp_status_t status;		/**< VP return status */
	enum ppe_vp_user_type usr_type;	/**< VP user type */
	enum ppe_vp_net_dev_type net_dev_type;
					/**< VP netdev type */
	enum ppe_vp_net_dev_pvt_flags net_dev_flags;
					/**< VP netdev flags */
};

/*
 * ppe_vp_get_netdev_by_port_num()
 *	Get the netdevice for port number.
 *
 * @param[in] port_num   VP port number.
 *
 * @return
 * Netdevice for the port number.
 */
struct net_device *ppe_vp_get_netdev_by_port_num(ppe_vp_num_t port_num);

/*
 * ppe_vp_mac_addr_clear()
 *	Clear the MAC address for the virtual port.
 *
 * @param[in] port_num   VP port number.
 *
 * @return
 * Success status of clearing MAC address.
 */
extern ppe_vp_status_t ppe_vp_mac_addr_clear(ppe_vp_num_t port_num);

/*
 * ppe_vp_mac_addr_set()
 *	Set the MAC address for the virtual port.
 *
 * @param[in] port_num   VP port number.
 * @param[in] mac_addr   MAC Address.
 *
 * @return
 * Success status of setting MAC address.
 */
extern ppe_vp_status_t ppe_vp_mac_addr_set(ppe_vp_num_t port_num, uint8_t *mac_addr);

/*
 * ppe_vp_mtu_get()
 *	Get the MTU for the virtual port.
 *
 * @param[in] port_num   VP port number.
 * @param[in] mtu        Pointer to MTU variable.
 *
 * @return
 * Success status of getting MTU.
 */
extern ppe_vp_status_t ppe_vp_mtu_get(ppe_vp_num_t port_num, uint16_t *mtu);

/*
 * ppe_vp_mtu_set()
 *	Set the MTU for the virtual port.
 *
 * @param[in] port_num   VP port number.
 * @param[in] mtu        MTU.
 *
 * @return
 * Success status of setting MTU.
 */
extern ppe_vp_status_t ppe_vp_mtu_set(ppe_vp_num_t port_num, uint16_t mtu);

/*
 * ppe_vp_free()
 *	Free PPE VP interface.
 *
 * @param[in] port_num   VP port number.
 *
 * @return
 * Success status of VP free.
 */
extern ppe_vp_status_t ppe_vp_free(ppe_vp_num_t port_num);

/*
 * ppe_vp_free_dev()
 *	Free PPE VP interface and its netdevice.
 *
 * @param[in] netdev    VP netdevice pointer.
 *
 * @return
 * Void.
 */
extern void ppe_vp_free_dev(struct net_device *vp_dev);

/*
 * ppe_vp_cfg_update()
 *	Update a PPE VP interface.
 *
 * @param[in] vp_num     VP number.
 * @param[in] vpui       VP update info.
 *
 * @return
 * Status of the API.
 */
extern ppe_vp_status_t ppe_vp_cfg_update(ppe_vp_num_t vp_num, struct ppe_vp_ui *vpui);

/*
 * ppe_vp_alloc()
 *	Allocate a PPE VP interface.
 *
 * @param[in] netdev     VP allocator netdevice.
 * @param[in] vpai       VP allocation info.
 *
 * @return
 * VP number.
 */
extern ppe_vp_num_t ppe_vp_alloc(struct net_device *netdev, struct ppe_vp_ai *vpai);

/*
 * ppe_vp_alloc_dev()
 *	Allocate a PPE VP interface and netdevice.
 *
 * @param[in] netdev     VP allocator netdevice.
 * @param[in] vpai       VP allocation info.
 *
 * @return
 * pointer to ppe vp netdevice.
 */
extern struct net_device *ppe_vp_alloc_dev(struct net_device *netdev, struct ppe_vp_ai *vpai);

/** @} */ /* end_addtogroup ppe_vp_public_subsystem */

#endif /* _PPE_VP_PUBLIC_H_ */
