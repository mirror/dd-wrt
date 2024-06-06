/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _ADPT_APPE_PORTCTRLH_
#define _ADPT_APPE_PORTCTRLH_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define APPE_PORT_MUX_MAC_TYPE                          0x0
#define APPE_PORT_MUX_XMAC_TYPE                         0x1
#define APPE_PORT5_MUX_PCS_UNIPHY0                      0x0
#define APPE_PORT5_MUX_PCS_UNIPHY1                      0x1

sw_error_t
_adpt_appe_port_mux_mac_set(a_uint32_t dev_id, fal_port_t port_id,
	a_uint32_t port_type);

sw_error_t
adpt_appe_port_cnt_mode_set(a_uint32_t dev_id, fal_port_t port_id, fal_port_cnt_cfg_t *cnt_cfg);

sw_error_t
adpt_appe_port_cnt_mode_get(a_uint32_t dev_id, fal_port_t port_id, fal_port_cnt_cfg_t *cnt_cfg);

sw_error_t
adpt_appe_port_rx_cnt_get(a_uint32_t dev_id, fal_port_t port_id, fal_port_cnt_t *port_cnt);

sw_error_t
adpt_appe_port_rx_cnt_flush(a_uint32_t dev_id, fal_port_t port_id);

sw_error_t adpt_appe_port_tx_buff_thresh_set(a_uint32_t dev_id,
		a_uint32_t port, a_uint16_t on_thresh, a_uint16_t off_thresh);

sw_error_t adpt_appe_port_tx_buff_thresh_get(a_uint32_t dev_id,
		a_uint32_t port, a_uint16_t *on_thresh, a_uint16_t *off_thresh);

#ifndef IN_PORTCONTROL_MINI
sw_error_t
adpt_appe_port_8023ah_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_8023ah_ctrl_t *port_8023ah_ctrl);

sw_error_t
adpt_appe_port_8023ah_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_8023ah_ctrl_t *port_8023ah_ctrl);
#endif

sw_error_t
adpt_appe_port_mtu_cfg_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_mtu_cfg_t *mtu_cfg);

sw_error_t
adpt_appe_port_mtu_cfg_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_mtu_cfg_t *mtu_cfg);

sw_error_t adpt_appe_port_erp_power_mode_set(a_uint32_t dev_id,
	a_uint32_t port_id, fal_port_erp_power_mode_t power_mode);
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
