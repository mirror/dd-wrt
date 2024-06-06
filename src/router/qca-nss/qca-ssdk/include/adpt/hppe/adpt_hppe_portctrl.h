/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _ADPT_HPPE_PORTCTRLH_
#define _ADPT_HPPE_PORTCTRLH_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

sw_error_t
adpt_hppe_port_interface_mode_set(a_uint32_t dev_id, fal_port_t port_id,
			fal_port_interface_mode_t mode);

sw_error_t
adpt_hppe_port_interface_mode_get(a_uint32_t dev_id, fal_port_t port_id,
			fal_port_interface_mode_t * mode);

sw_error_t
adpt_hppe_port_interface_mode_apply(a_uint32_t dev_id);

sw_error_t
qca_hppe_mac_sw_sync_task(struct qca_phy_priv *priv);

sw_error_t
adpt_hppe_port_bridge_txmac_set(a_uint32_t dev_id,
		fal_port_t port_id, a_bool_t enable);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
