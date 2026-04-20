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
 * @file ppe_drv_br.h
 *	NSS PPE driver bridge client definitions.
 */

#ifndef _PPE_DRV_BR_H_
#define _PPE_DRV_BR_H_

#include <fal/fal_stp.h>

#define PPE_DRV_BR_SPANNING_TREE_ID	0

/**
 * ppe_drv_br_fdb_del_bymac
 *	Delete FDB entry by MAC address.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] mac_addr    MAC address for which FDB entry should be deleted.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_br_fdb_del_bymac(struct ppe_drv_iface *br_iface, uint8_t *mac_addr);

/**
 * ppe_drv_br_fdb_add
 *	Add FDB entry by MAC address.
 *
 * @datatypes
 * ppe_drv_iface
 * uint8_t
 * bool
 * uint32_t
 *
 * @param[in] br_iface    Pointer to the PPE interface for bridge.
 * @param[in] mac_addr    MAC address for which FDB entry should be added.
 * @param[in] is_static   True if FDB entry is static else false
 * @param[in] port_id     Port id for which FDB entry should be added
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_br_fdb_add(struct ppe_drv_iface *br_iface,
		uint8_t *mac_addr, bool is_static, uint32_t port_id);

/**
 * ppe_drv_br_flush_fdb
 *	ppe_drv_dp_flush_fdb api will flush FDB entries based on arguments api can
 *	flush only dynamic or both (dynamic and static) FDBs.
 *	ppe_drv_dp_flush_fdb api can flush per port FDB entries or entire switch's
 *	FDB entries
 *
 * @datatypes
 * ppe_drv_iface
 * bool
 *
 * @param[in] port_iface         Pointer to the PPE interface.
 * @param[in] only_dynamic  True for flush only dynamic FDB entries
 *                          or false for flush dynamic and static.
 * @param[in] del_by_port   True for flush FDB entries by port
 *                          or false for flush FDB entries from all port.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_br_flush_fdb(struct ppe_drv_iface *port_iface,
		bool only_dynamic, bool del_by_port);

/**
 * ppe_drv_br_port_set_learning
 *	Set enable/disable port learning
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] port_iface   Pointer to the PPE interface.
 * @param[in] lrn_enable   Enable/Disable learning.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_br_port_set_learning(struct ppe_drv_iface *port_iface,
		bool lrn_enable);

/**
 * ppe_drv_br_set_ageing_time
 *	Set ageing time
 *
 * @datatypes
 * uint32_t
 *
 * @param[in] ageing_time   Ageing time in seconds.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_br_set_ageing_time(uint32_t ageing_time);

/**
 * ppe_drv_br_fdb_lrn_ctrl
 *	Configure FDB learn control for a bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] enable      Flag to enable or disable FDB learning.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_br_fdb_lrn_ctrl(struct ppe_drv_iface *br_iface, bool enable);

/**
 * ppe_drv_br_stp_state_set
 *	Set STP state on bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 * fal_stp_state_t
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] member      Bridge member netdevice for which STP state is being configured.
 * @param[in] state       STP state of the bridge interface.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_br_stp_state_set(struct ppe_drv_iface *br_iface, struct net_device *member, fal_stp_state_t state);

/**
 * ppe_drv_br_wanif_clear
 *	clear ppe_drv_iface PPE_DRV_IFACE_FLAG_WAN_IF_VALID flag.
 *
 * @datatypes
 * net_device
 *
 * @param[in] member    Pointer to the member net device.
 *
 * @return
 * void.
 */
void ppe_drv_br_wanif_clear(struct net_device *member);

/**
 * ppe_drv_br_wanif_set
 *	Set ppe_drv_iface flag to PPE_DRV_IFACE_FLAG_WAN_IF_VALID.
 *
 * @datatypes
 * net_device
 *
 * @param[in] member	Pointer to the member net device.
 *
 * @return
 * void.
 */
void ppe_drv_br_wanif_set(struct net_device *member);

/**
 * ppe_drv_br_leave
 *	Remove a member from bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] member      Pointer to the member net device.
 *
 * @return
 * Status of the leave operation.
 */
ppe_drv_ret_t ppe_drv_br_leave(struct ppe_drv_iface *br_iface, struct net_device *member);

/**
 * ppe_drv_br_join
 *	Add a member to bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] member      Pointer to the member net device.
 *
 * @return
 * Status of the join operation.
 */
ppe_drv_ret_t ppe_drv_br_join(struct ppe_drv_iface *br_iface, struct net_device *member);

/**
 * ppe_drv_br_deinit
 *	Uninitialize bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 *
 * @return
 * Status of the uninitialization.
 */
ppe_drv_ret_t ppe_drv_br_deinit(struct ppe_drv_iface *br_iface);

/**
 * ppe_drv_br_init
 *	Initialize bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface allocated for bridge.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_br_init(struct ppe_drv_iface *br_iface);
#endif /* _PPE_DRV_BR_H_ */
