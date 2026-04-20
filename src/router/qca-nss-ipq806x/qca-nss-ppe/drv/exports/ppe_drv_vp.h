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
 * @file ppe_drv_vp.h
 *	NSS PPE VP definitions.
 */

#ifndef _PPE_DRV_VP_H_
#define _PPE_DRV_VP_H_

enum ppe_port_user_type;

/**
 * @addtogroup ppe_drv_vp_subsystem
 * @{
 */

/**
 * ppe_drv_vp_info
 *	PPE DRV VP info structure
 */
struct ppe_drv_vp_info {
	uint32_t xmit_port;		/**< Physical port number */
	uint8_t queue_num;		/**< Queue number */
	uint8_t core_mask;		/**< Core to be used for a particular VP flow */
	uint32_t usr_type;		/**< VP user type */
	uint32_t net_dev_type;		/**< VP netdev type */
	bool disable_ttl_dec;		/**< Disable TTL decrement operation in PPE VP */
	bool redir_en;			/**< Enable redirection to VP queue without RPS */
};

/**
 * ppe_drv_vp_deinit
 *	Uninitialize a virutal port in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface   Pointer to the interface object.
 *
 * @return
 * Status of the de-init operation.
 */
ppe_drv_ret_t ppe_drv_vp_deinit(struct ppe_drv_iface *iface);

/**
 * ppe_drv_vp_init
 *	Initialize a virtual port in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface Pointer to the interface object.
 * @param[in] info Pointer to the VP info object.
 *
 * @return
 * Status of the initialization operation.
 */
ppe_drv_ret_t ppe_drv_vp_init(struct ppe_drv_iface *iface, struct ppe_drv_vp_info *info);

/**
 * ppe_drv_vp_cfg_update
 *	Update a virtual port in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface Pointer to the interface object.
 * @param[in] info Pointer to the VP info object.
 *
 * @return
 * Status of the Update operation.
 */
ppe_drv_ret_t ppe_drv_vp_cfg_update(struct ppe_drv_iface *iface, struct ppe_drv_vp_info *info);

/** @} */ /* end_addtogroup ppe_drv_vp_subsystem */

#endif /* _PPE_DRV_VP_H_ */
