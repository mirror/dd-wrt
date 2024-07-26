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
 * @file ppe_drv_dp.h
 *	NSS PPE DP definitions.
 */

#ifndef _PPE_DRV_DP_H_
#define _PPE_DRV_DP_H_

/**
 * @addtogroup ppe_drv_dp_subsystem
 * @{
 */

/**
 * enum ppe_drv_dp_mirror_direction
 *	PPE mirror directions
 */
typedef enum ppe_drv_dp_mirror_direction {
	PPE_DRV_DP_MIRR_DI_IN = 0,	/**< PPE mirror direction ingress */
	PPE_DRV_DP_MIRR_DI_EG,		/**< PPE mirror direction egress */
} ppe_drv_dp_mirror_direction_t;

/*
 * Default mirror analysis port priority
 */
#define PPE_DRV_MIRR_ANALYSIS_PRI 0

/**
 * ppe_drv_dp_deinit
 *	Deinitialize DP.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface   Pointer to the PPE interface for bridge.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_deinit(struct ppe_drv_iface *iface);

/**
 * ppe_drv_dp_init
 *	Initialize DP.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface   Pointer to the PPE interface for bridge.
 * @param[in] macid   MAC ID of the port.
 * @param[in] mht_dev MHT SWT device ports mapping flag.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_init(struct ppe_drv_iface *iface, uint32_t macid,
				bool mht_dev);

/**
 * ppe_drv_dp_set_mirror_if
 *	Set enable/disable ingress/egress port mirroring
 *
 * @datatypes
 * ppe_drv_iface
 * ppe_drv_dp_mirror_direction_t mirror direction
 * bool
 *
 * @param[in] iface       Pointer to the PPE interface.
 * @param[in] direction   Mirror direction (ingress/egress).
 * @param[in] enable      Enable/Disable mirroring.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_set_mirror_if(struct ppe_drv_iface *iface,
		ppe_drv_dp_mirror_direction_t direction, bool enable);

/**
 * ppe_drv_dp_set_mirr_analysis_port
 *	Set mirror analysis port
 *
 * @datatypes
 * ppe_drv_iface
 * ppe_drv_dp_mirror_direction_t mirror direction
 * bool
 *
 * @param[in] iface         Pointer to the PPE interface.
 * @param[in] direction     Mirror analysis port direction
 * @param[in] enable        True for enable and false for disable analysis port
 * @param[in] priority      Analysis port priority
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_set_mirr_analysis_port(struct ppe_drv_iface *iface,
		ppe_drv_dp_mirror_direction_t direction, bool enable, uint8_t priority);

/**
 * ppe_drv_dp_get_mirr_analysis_port
 *	Get mirror analysis port number
 *
 * @datatypes
 * ppe_drv_dp_mirror_direction_t mirror direction
 *
 * @param[in] direction     Mirror analysis port direction
 * @param[out] port_num     Mirror analysis port number
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_get_mirr_analysis_port(
		ppe_drv_dp_mirror_direction_t direction, uint8_t *port_num);

/**
 * ppe_drv_dp_set_ppe_offload_enable_flag
 *	API to set PPE offload enable flag in PPE port
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface         Pointer to the PPE interface.
 * @param[in] disable       True if PPE offload is disbled, otherwise false
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_set_ppe_offload_enable_flag(struct ppe_drv_iface *iface,
				bool disable);
/** @} */ /* end_addtogroup ppe_drv_dp_subsystem */

#endif /* _PPE_DRV_DP_H_ */
