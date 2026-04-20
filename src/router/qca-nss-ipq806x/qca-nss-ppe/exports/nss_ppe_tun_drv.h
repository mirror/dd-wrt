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

/*
 * nss_ppe_tun_drv.h
 *	PPE Tunnel Driver Exports
 */
#ifndef _NSS_PPE_TUN_DRV_H_
#define _NSS_PPE_TUN_DRV_H_
#include <ppe_drv_tun_cmn_ctx.h>
#include <ppe_vp_public.h>
#include <ppe_tun.h>

/**
 * ppe_tun_configure()
 *	Configure tunnel tunnel
 *
 * @param[in] dev      netdevice structure
 * @param[in] tun_hdr  tunnel header structure
 * @param[in] tun_cb   tunnel callback structure
 *
 * @return
 * Status of the configuration.
 */
bool ppe_tun_configure(struct net_device *dev, struct ppe_drv_tun_cmn_ctx *tun_hdr,  struct ppe_tun_excp *tun_cb);

/**
 * ppe_tun_deactivate()
 *	Deactive tunnel
 *
 * @param[in] dev  netdevice structure
 *
 * @return
 * Status of deactivation
 */
bool ppe_tun_deactivate(struct net_device *dev);

/**
 * ppe_tun_mtu_get()
 *	Get the ppe vp mtu.
 *
 * @param[in] dev  netdevice structure
 *
 * @return
 * mtu size
 */
uint32_t ppe_tun_mtu_get(struct net_device *dev);

/**
 * ppe_tun_mtu_set()
 *	Set the ppe vp mtu.
 *
 * @param[in] dev  netdevice structure
 * @param[in] mtu  mtu number
 *
 * @return
 * Status of operation
 */
bool ppe_tun_mtu_set(struct net_device *dev, uint32_t mtu);

/**
 * ppe_tun_exception_packet_get()
 *	Get te number of exception packets the tunnel has seen.
 *
 * @param[in] tun       tunnel structure
 * @param[in] num_pkt   number of packets
 * @param[in] num_bytes number of bytes
 *
 * @return
 * Status of operation
 */
bool ppe_tun_exception_packet_get(struct net_device *dev, uint64_t *num_pkt, uint64_t *num_bytes);

/**
 * ppe_tun_conf_accel()
 *	Enable / Disable acceleration for a tunnel type
 *
 * @param[in] type     tunnel type
 * @param[in] action   1 for Enable, 0 for disable
 *
 * @return
 * Status of operation
 */
bool ppe_tun_conf_accel(enum ppe_drv_tun_cmn_ctx_type type, bool action);

/**
 * ppe_tun_get_active_tun_cnt()
 *	Get the active number of struct ppe_tun
 *
 * @return
 * Number of active tunnels
 */
uint16_t ppe_tun_get_active_tun_cnt(void);

/**
 * ppe_tun_setup()
 *	Setup the netdevice
 *
 * @param[in] dev      net device
 * @param[in] tun_hdr  tunnel header
 *
 * @return
 * Status of operation
 */
bool ppe_tun_setup(struct net_device *dev, struct ppe_drv_tun_cmn_ctx *tun_hdr);

/**
 * ppe_tun_free()
 *	Free a struct ppe_tun
 *
 * @param[in] dev  net device
 *
 * @return
 * Status of operation
 */
bool ppe_tun_free(struct net_device *dev);

/**
 * ppe_tun_alloc()
 *	Allocate a struct ppe_tun
 *
 * @param[in] dev  net device
 *
 * @return
 * Status of operation
 */
bool ppe_tun_alloc(struct net_device *dev, enum ppe_drv_tun_cmn_ctx_type type);

/**
 * ppe_tun_deconfigure()
 *	Deconfigure the tunnel
 *
 * @param[in] dev  net device
 *
 * @return
 * Status of operation
 */
bool ppe_tun_deconfigure(struct net_device *dev);

/*
 * ppe_tun_decap_disable()
 *	Disable tunnel decapsulation
 *
 * @param type[IN] dev  netdevice
 */
bool ppe_tun_decap_disable(struct net_device *dev);

/*
 * ppe_tun_decap_enable()
 *	Enable tunnel decapsulation
 *
 * @param type[IN] dev  netdevice
 *
 * @return
 * Status of operation
 */
bool ppe_tun_decap_enable(struct net_device *dev);

/*
 *  ppe_tun_configure_vxlan_dport
 *	Configure VXLAN destination port
 *
 * @param type[IN] dport  destination port
 *
 * @return
 * Status of operation
 */
bool ppe_tun_configure_vxlan_dport(uint16_t dport);

/*
 *  ppe_tun_l2tp_port_set
 *      Set L2TP source and destination port
 *
 * @param type[IN] sport  l2tp source port
 * @param type[IN] dport  l2tp destination port
 *
 * @return
 * Status of operation
 */
bool ppe_tun_l2tp_port_set(uint16_t sport, uint16_t dport);

/*
 *  ppe_tun_l2tp_port_get
 *      Get L2TP source and destination port
 *
 * @param type[IN] sport  l2tp source port pointer
 * @param type[IN] dport  l2tp destination port pointer
 *
 * @return
 * Status of operation
 */
bool ppe_tun_l2tp_port_get(uint16_t *sport, uint16_t *dport);
#endif /* _NSS_PPE_TUN_DRV_H_ */
