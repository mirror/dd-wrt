/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file ppe_drv_port.h
 *	NSS PPE driver definitions.
 */
#ifndef _PPE_DRV_PORT_H_
#define _PPE_DRV_PORT_H_

#include <linux/netdevice.h>

/*
 * PPE port defines
 */
#define PPE_DRV_PHYSICAL_START		0	/* Physical port start with port 0 */
#define PPE_DRV_PHYSICAL_MAX		8	/* PPE supports 8 physical ports 0-7 */
#define PPE_DRV_VIRTUAL_MAX		192	/* PPE supports 192 virtual interfaces 64-255 */
#define PPE_DRV_VIRTUAL_START		64	/* Virtual ports start at 64 */
#define PPE_DRV_VIRTUAL_END		(PPE_DRV_VIRTUAL_START + PPE_DRV_VIRTUAL_MAX)
						/* Virtual ports ends at 256 */
#define PPE_DRV_PORTS_MAX		256	/* Total ports in PPE Physical + Trunk + Virtual */

#define PPE_DRV_PORT_ID_INVALID		-1	/* Invalid port ID */

#define PPE_DRV_PORT_CPU		0	/* PPE egress port to reach CPUs */
#define PPE_DRV_PORT_EIP197		7	/* PPE egress port to reach EIP197 */

#define PPE_DRV_PORT_JUMBO_MAX		9216	/* Suggested value is 9K, but can be increased upto 10K */

#define PPE_DRV_PHY_ETH_PORT_START	1	/* Physical eth port start with port 1 */
#ifdef NSS_PPE_IPQ53XX
#define PPE_DRV_PHY_ETH_PORT_MAX	2	/* PPE supports 2 physical ports 1-2 for IPQ53XX */
#else
#define PPE_DRV_PHY_ETH_PORT_MAX	6	/* PPE supports 6 physical ports 1-6 for Others */
#endif

typedef int32_t ppe_drv_port_t;

struct ppe_drv;
struct ppe_drv_vsi;
struct ppe_drv_l3_if;

/*
 * ppe_drv_port_hw_stats
 *	PPE port Hardware statistics.
 */
struct ppe_drv_port_hw_stats {
	uint32_t rx_pkt_cnt;	/* rx packet counter */
	uint32_t rx_drop_pkt_cnt;	/* rx drop packet counter */
	uint32_t tx_pkt_cnt;	/* tx packet counter */
	uint32_t tx_drop_pkt_cnt;	/* tx drop packet counter */
	uint64_t rx_byte_cnt;	/* rx byte counter */
	uint64_t rx_drop_byte_cnt;	/* rx drop byte counter */
	uint64_t tx_byte_cnt;	/* tx byte counter */
	uint64_t tx_drop_byte_cnt;	/* tx drop byte counter */
};

/*
 * ppe_drv_port_type
 *	PPE Port types - values derived from HW spec
 */
enum ppe_drv_port_type {
	PPE_DRV_PORT_PHYSICAL,		/* Physical Port */
	PPE_DRV_PORT_LAG,		/* LAG Port */
	PPE_DRV_PORT_VIRTUAL,		/* Virtual Port */
	PPE_DRV_PORT_EIP,		/* EIP inline Port */
	PPE_DRV_PORT_VIRTUAL_PO,	/* Virtual point offload port */
};

/*
 * ppe_drv_port_qos_res_pre
 *	PPE Port QOS resolution precedence
 *
 * QOS resolution precedence across different classfication engines
 * This ranges from 0-7, 0,7 are reserved.
 */
enum ppe_drv_port_qos_res_pre {
	PPE_DRV_PORT_QOS_RES_PREC_0_RESERVED,
	PPE_DRV_PORT_QOS_RES_PREC_1,
	PPE_DRV_PORT_QOS_RES_PREC_2,
	PPE_DRV_PORT_QOS_RES_PREC_3,
	PPE_DRV_PORT_QOS_RES_PREC_4,
	PPE_DRV_PORT_QOS_RES_PREC_5,
	PPE_DRV_PORT_QOS_RES_PREC_6,
	PPE_DRV_PORT_QOS_RES_PREC_7,
};

/**
 * ppe_drv_port_get_vp_phys_dev
 *	Get physical dev attached to VP.
 *
 * @datatypes
 * net_device
 *
 * @param[in] dev  Net device.
 *
 * @return
 *  return net device
 */
struct net_device *ppe_drv_port_get_vp_phys_dev(struct net_device *dev);

/**
 * ppe_drv_port_ucast_queue_get_by_port
 *	Return queue id of port number.
 *
 * @datatypes
 * int
 *
 * @return
 * -1 for failure else port ID
 */
int32_t ppe_drv_port_ucast_queue_get_by_port(int port);

/**
 * ppe_drv_port_num_from_dev
 *	Get port index from device.
 *
 * @datatypes
 * net_device
 *
 * @param[in] dev  Net device.
 *
 * @return
 * -1 for failure else port_index.
 */
int32_t ppe_drv_port_num_from_dev(struct net_device *dev);

/**
 * ppe_drv_port_num_to_dev
 *	Port number to device.
 *
 * @param[in] port  Port number.
 *
 * @return
 * net_device.
 */
struct net_device *ppe_drv_port_num_to_dev(uint8_t port);

/**
 * ppe_drv_port_xcpn_mode_set
 *	Set exception mode.
 *
 * @param[in] port number, action.
 *
 * @return
 * 1 for success, 0 for failure.
 */
bool ppe_drv_port_xcpn_mode_set(uint16_t vp_num, uint8_t action);

/**
 * ppe_drv_port_get_vp_stats
 * 	Get PPE hardware stats for the VP port.
 *
 * @datatypes
 * ppe_drv_port_hw_stats
 *
 * @param[in] port PPE port number.
 * @param[in,out] vp_stats VP hardware stats.
 *
 * @return
 * True if the hw stats are copied, false otherwise.
 */
bool ppe_drv_port_get_vp_stats(int16_t port, struct ppe_drv_port_hw_stats *vp_stats);

/*
 * ppe_drv_port_clear_policer_support
 *	Clear Policer enabled on Port.
 *
 * @param[in] dev  Netdevie.
 *
 * @return
 */
void ppe_drv_port_clear_policer_support(struct net_device *dev);

/*
 * ppe_drv_port_set_policer_support
 *	Set Policer enabled on Port.
 *
 * @param[in] dev  Netdevie.
 *
 * @return
 */
void ppe_drv_port_set_policer_support(struct net_device *dev);

/*
 * ppe_drv_port_check_policer_support
 *	Is Policer enabled on Port.
 *
 * @param[in] dev  Netdevie.
 *
 * @return
 * True or False.
 */
bool ppe_drv_port_check_policer_support(struct net_device *dev);

/*
 * ppe_drv_port_check_rfs_support
 *	Is RFS enabled on Port.
 *
 * @param[in] dev  Netdevie.
 *
 * @return
 * True or False.
 */
bool ppe_drv_port_check_rfs_support(struct net_device *dev);

/**
 * ppe_drv_port_clear_hw_vp_stats
 * 	Clear PPE HW stats for the VP.
 *
 * @param[in] port PPE port number.
 *
 * @return
 * True if the hw stats are cleared, false otherwise.
 */
bool ppe_drv_port_clear_hw_vp_stats(int16_t port);
#endif /* _PPE_DRV_PORT_H_ */
