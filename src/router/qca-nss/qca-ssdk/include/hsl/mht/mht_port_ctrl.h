/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _MHT_PORT_CTRL_H_
#define _MHT_PORT_CTRL_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#include "fal_port_ctrl.h"

#define MHT_PORT_SPEED_10M          0
#define MHT_PORT_SPEED_100M         1
#define MHT_PORT_SPEED_1000M        2
#define MHT_PORT_SPEED_2500M        MHT_PORT_SPEED_1000M
#define MHT_PORT_HALF_DUPLEX        0
#define MHT_PORT_FULL_DUPLEX        1
#ifndef IN_PORTCONTROL_MINI
sw_error_t
mht_port_congestion_drop_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t queue_id, a_bool_t enable);
sw_error_t
mht_port_congestion_drop_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t queue_id, a_bool_t * enable);
#endif
sw_error_t
mht_port_flowctrl_thresh_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint16_t on, a_uint16_t off);
sw_error_t
mht_port_flowctrl_thresh_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint16_t *on, a_uint16_t *off);
#ifndef IN_PORTCONTROL_MINI
sw_error_t
mht_ring_flow_ctrl_thres_set(a_uint32_t dev_id, a_uint32_t ring_id,
		a_uint16_t on_thres, a_uint16_t off_thres);
sw_error_t
mht_ring_flow_ctrl_thres_get(a_uint32_t dev_id, a_uint32_t ring_id,
		a_uint16_t * on_thres, a_uint16_t * off_thres);
sw_error_t
mht_ring_flow_ctrl_status_get(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t *status);
sw_error_t
mht_ring_union_set(a_uint32_t dev_id, a_bool_t en);
sw_error_t
mht_ring_union_get(a_uint32_t dev_id, a_bool_t *en);
sw_error_t
mht_ring_flow_ctrl_config_get(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t *status);
sw_error_t
mht_ring_flow_ctrl_config_set(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t status);
#endif
sw_error_t
mht_port_flowctrl_forcemode_set(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t enable);
sw_error_t
mht_port_flowctrl_forcemode_get(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t *enable);
sw_error_t
mht_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);
sw_error_t
mht_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);
sw_error_t
mht_port_txfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);
sw_error_t
mht_port_rxfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);
sw_error_t
mht_port_flowctrl_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);
sw_error_t
mht_port_flowctrl_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);
sw_error_t
mht_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t duplex);
sw_error_t
mht_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t *duplex);
sw_error_t
mht_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t speed);
sw_error_t
mht_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t *pspeed);
sw_error_t
mht_port_link_update(struct qca_phy_priv *priv, a_uint32_t port_id,
	struct port_phy_status phy_status);
sw_error_t
mht_port_erp_power_mode_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_erp_power_mode_t power_mode);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _MHT_PORT_CTRL_H_ */
