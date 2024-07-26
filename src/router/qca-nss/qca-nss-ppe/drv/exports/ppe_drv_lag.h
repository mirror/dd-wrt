/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file ppe_drv_lag.h
 *	NSS PPE driver lag client definitions.
 */

#ifndef _PPE_DRV_LAG_H_
#define _PPE_DRV_LAG_H_

/**
 * ppe_drv_lag_leave
 *	Remove a member from LAG interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] lag_iface   Pointer to the PPE interface for LAG.
 * @param[in] member      Pointer to the member net_device.
 *
 * @return
 * Status of the leave operation.
 */
ppe_drv_ret_t ppe_drv_lag_leave(struct ppe_drv_iface *lag_iface, struct net_device *member);

/**
 * ppe_drv_lag_join
 *	Add a member to LAG interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] lag_iface   Pointer to the PPE interface for lag.
 * @param[in] member      Pointer to the member net_device.
 *
 * @return
 * Status of the join operation.
 */
ppe_drv_ret_t ppe_drv_lag_join(struct ppe_drv_iface *lag_iface, struct net_device *member);

/**
 * ppe_drv_lag_deinit
 *	Uninitialize LAG interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] lag_iface   Pointer to the PPE interface for LAG.
 *
 * @return
 * Status of the uninitialization.
 */
ppe_drv_ret_t ppe_drv_lag_deinit(struct ppe_drv_iface *lag_iface);

/**
 * ppe_drv_lag_init
 *	Initialize LAG interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] lag_iface   Pointer to the PPE interface allocated for LAG.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_lag_init(struct ppe_drv_iface *lag_iface);
#endif /* _PPE_DRV_LAG_H_ */
