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
#ifndef _PPE_DRV_TUN_PUBLIC_H_
#define _PPE_DRV_TUN_PUBLIC_H_

/**
 * ppe_drv_tun_add_ce_callback_t
 *	Register for tunnel outer rule entry addition callback.
 *
 * @param[in] vp_num Virtual port number
 * @param[in] vcreate_rule pointer to create_rule
 *
 * @return
 * Success or failure.
 */
typedef bool (*ppe_drv_tun_add_ce_callback_t)(uint8_t vp_num, void *vcreate_rule);

/**
 * ppe_drv_tun_del_ce_callback_t
 *	Register for tunnel outer rule entry deletion callback.
 *
 * @param[in] vp_num Virtual port number
 * @param[in] vdestroy_rule pointer to destroy_rule
 *
 * @return
 * Success or failure.
 */
typedef bool (*ppe_drv_tun_del_ce_callback_t)(uint8_t vp_num, void *vdestroy_rule);

/**
 * ppe_drv_tun_configure
 *	Allocate a tunnel context in ppe
 *
 * @param[in] port_num VP port number
 * @param[in] add_cb registered callback for tunnel entry addition
 * @param[in] del_cb registered callback for tunnel entry deletion
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_configure(uint16_t port_num, struct ppe_drv_tun_cmn_ctx *pth, void *add_cb, void *del_cb);

/**
 * ppe_drv_tun_configure_vxlan_and_dport
 *	Configure VXLAN destination port.
 *
 * @param[in] dport VXLAN destination port number
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_configure_vxlan_and_dport(uint16_t dport);

/**
 * ppe_drv_tun_deactivate
 *	deactivate a tunnel context in ppe
 *
 * @param[in] port_num VP port number
 * @param[in] vdestroy_rule pointer to connection entry
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_deactivate(uint16_t port_num, void *vdestroy_rule);

/**
 * ppe_drv_tun_activate
 *	Activate a tunnel context in ppe
 *
 * @param[in] port_num VP port number
 * @param[in] vcreate_rule pointer to connection entry
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_activate(uint16_t port_num, void *vcreate_rule);

/**
 * ppe_drv_tun_deconfigure
 *	deconfigure tunnel entry encapsulation/decapsulation in ppe
 *
 * @param[in] port_num VP port number
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_deconfigure(uint16_t port_num);

/**
 * ppe_drv_tun_decap_disable_by_port_num
 *	disable decapsulation in ppe
 *
 * @param[in] port_num VP port number
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_decap_disable_by_port_num(uint16_t port_num);

/**
 * ppe_drv_tun_decap_enable_by_port_num
 *	enable decapsulation in ppe
 *
 * @param[in] port_num VP port number
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_decap_enable_by_port_num(uint16_t port_num);

/*
 * ppe_drv_tun_l2tp_port_set
 *      Set L2TP source and destination port.
 *
 * @param[in] sport Source port value
 * @param[in] dport Destination port value
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_l2tp_port_set(uint16_t sport, uint16_t dport);

/*
 * ppe_drv_tun_l2tp_port_get
 *      Get l2tp source and destination port configured.
 *
 * @param[in] sport Pointer to store source port
 * @param[in] dport Pointer to store destination port
 *
 * @return
 * Success or failure.
 */
bool ppe_drv_tun_l2tp_port_get(uint16_t *sport, uint16_t *dport);
#endif /* _PPE_DRV_TUN_PUBLIC_H_ */
