/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

#include "ssdk_init.h"
#include "ssdk_hppe.h"
#include "ssdk_appe.h"
#include "ssdk_clk.h"
#include "ssdk_dts.h"
#include "adpt.h"
#include "adpt_hppe.h"
#include "fal.h"
#ifdef IN_LED
#include "ssdk_led.h"
#endif
#ifdef IN_RSS_HASH
#include <linux/random.h>
#endif

#if defined(IN_BM) && defined(IN_QOS)
#if defined(MPPE)
fal_port_tdm_tick_cfg_t ppe_port_tdm0_tbl[] = {
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_FALSE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
};

fal_port_scheduler_cfg_t ppe_port_scheduler0_tbl[] = {
	{0x0, 0x2, 0, 0x0, 0},
	{0x0, 0x1, 2, 0x0, 0},
	{0x0, 0x0, 1, 0x0, 0},
	{0x0, 0x0, 2, 0x0, 0},
	{0x0, 0x1, 0, 0x0, 0},
	{0x0, 0x2, 1, 0x0, 0},
	{0x0, 0x0, 2, 0x0, 0},
	{0x0, 0x0, 1, 0x0, 0},
	{0x0, 0x0, 2, 0x0, 0},
	{0x0, 0x0, 1, 0x0, 0},
};
#else

fal_port_scheduler_cfg_t ppe_port_scheduler0_tbl[] = {
	{0x98, 6, 0, 0x1, 1},
	{0x94, 5, 6, 0x3, 1},
	{0x86, 0, 5, 0x4, 1},
	{0x8C, 1, 6, 0x0, 1},
	{0x1C, 7, 5, 0x1, 1},
	{0x98, 2, 6, 0x0, 1},
	{0x1C, 5, 7, 0x1, 1},
	{0x34, 3, 6, 0x0, 1},
	{0x8C, 4, 5, 0x1, 1},
	{0x98, 2, 6, 0x0, 1},
	{0x8C, 5, 4, 0x1, 1},
	{0xA8, 0, 6, 0x2, 1},
	{0x98, 5, 1, 0x0, 1},
	{0x98, 6, 5, 0x2, 1},
	{0x89, 1, 6, 0x4, 1},
	{0xA4, 3, 0, 0x1, 1},
	{0x8C, 5, 6, 0x4, 1},
	{0xA8, 0, 2, 0x1, 1},
	{0x98, 6, 5, 0x0, 1},
	{0xC4, 4, 3, 0x1, 1},
	{0x94, 6, 5, 0x0, 1},
	{0x1C, 7, 6, 0x1, 1},
	{0x98, 2, 5, 0x0, 1},
	{0x1C, 6, 7, 0x1, 1},
	{0x1C, 5, 6, 0x0, 1},
	{0x94, 3, 5, 0x1, 1},
	{0x8C, 4, 6, 0x0, 1},
	{0x94, 1, 5, 0x3, 1},
	{0x94, 6, 1, 0x0, 1},
	{0xD0, 3, 5, 0x2, 1},
	{0x98, 6, 0, 0x1, 1},
	{0x94, 5, 6, 0x3, 1},
	{0x94, 1, 5, 0x0, 1},
	{0x98, 2, 6, 0x1, 1},
	{0x8C, 4, 5, 0x0, 1},
	{0x1C, 7, 6, 0x1, 1},
	{0x8C, 0, 5, 0x4, 1},
	{0x89, 1, 6, 0x2, 1},
	{0x98, 5, 0, 0x1, 1},
	{0x94, 6, 5, 0x3, 1},
	{0x92, 0, 6, 0x2, 1},
	{0x98, 1, 5, 0x0, 1},
	{0x98, 6, 2, 0x1, 1},
	{0xD0, 0, 5, 0x3, 1},
	{0x94, 6, 0, 0x1, 1},
	{0x8C, 5, 6, 0x4, 1},
	{0x8C, 1, 5, 0x0, 1},
	{0x1C, 6, 7, 0x1, 1},
	{0x1C, 5, 6, 0x0, 1},
	{0xB0, 2, 3, 0x1, 1},
	{0xC4, 4, 5, 0x0, 1},
	{0x8C, 6, 4, 0x1, 1},
	{0xA4, 3, 6, 0x0, 1},
	{0x1C, 5, 7, 0x1, 1},
	{0x4C, 0, 5, 0x4, 1},
	{0x8C, 6, 0, 0x1, 1},
	{0x34, 7, 6, 0x3, 1},
	{0x94, 5, 0, 0x1, 1},
	{0x98, 6, 5, 0x2, 1},
};

fal_port_tdm_tick_cfg_t ppe_port_tdm0_tbl[] = {
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 3, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 3, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 4, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 4, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS,0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 3, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 3, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 4, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 4, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{A_TRUE, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
};
#endif

static sw_error_t
qca_appe_tdm_hw_init(a_uint32_t dev_id)
{
	adpt_api_t *p_api;
	a_uint32_t i = 0;
	a_uint32_t num = 0;
	fal_port_tdm_ctrl_t tdm_ctrl;
	fal_port_scheduler_cfg_t *scheduler_cfg = NULL;
	fal_port_tdm_tick_cfg_t *bm_cfg = NULL;
	a_uint8_t tm_tick_mode, bm_tick_mode;
	a_uint32_t chip_type = adpt_ppe_type_get(dev_id);

	SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));
	SW_RTN_ON_NULL(p_api->adpt_port_scheduler_cfg_set);
	SW_RTN_ON_NULL(p_api->adpt_tdm_tick_num_set);

	tm_tick_mode = ssdk_tm_tick_mode_get(dev_id);
	bm_tick_mode = ssdk_bm_tick_mode_get(dev_id);

	if (tm_tick_mode > 1) {
		SSDK_ERROR("appe invalid tm tdm mode %d!\n", tm_tick_mode);
		return SW_BAD_VALUE;
	}

#if defined(MPPE)
	if (chip_type == MPPE_TYPE) {
		if (tm_tick_mode == 0) {
			num = ARRAY_SIZE(ppe_port_scheduler0_tbl);
			scheduler_cfg = ppe_port_scheduler0_tbl;
		} else {
			SSDK_ERROR("mppe invalid tdm mode %d\n", tm_tick_mode);
			return SW_BAD_VALUE;
		}
	}
#else
	if (chip_type == APPE_TYPE) {
		switch (tm_tick_mode) {
			case 0:
			case 1:
				/* The unified mode supports the following cases:
				 * 1. 3x10G(port1 128B, port5,6 64B reach to line rate);
				 * 2. 4x2.5G + 1X10G.
				 * 3. 4X1G + 2x10G.
				 */
				num = ARRAY_SIZE(ppe_port_scheduler0_tbl);
				scheduler_cfg = ppe_port_scheduler0_tbl;
				break;
			default:
				SSDK_ERROR("appe invalid scheduler tdm mode %d\n", tm_tick_mode);
				return SW_BAD_VALUE;
		}
	}
#endif
	if (scheduler_cfg != NULL) {
		for (i = 0; i < num; i++) {
			p_api->adpt_port_scheduler_cfg_set(dev_id, i, &scheduler_cfg[i]);
		}
		p_api->adpt_tdm_tick_num_set(dev_id, num);
	}

	SW_RTN_ON_NULL(p_api->adpt_port_tdm_tick_cfg_set);
	SW_RTN_ON_NULL(p_api->adpt_port_tdm_ctrl_set);
	
#if defined(MPPE)
	if (chip_type == MPPE_TYPE) {
		if (bm_tick_mode == 0) {
			bm_cfg = ppe_port_tdm0_tbl;
			num = ARRAY_SIZE(ppe_port_tdm0_tbl);
		} else {
			SSDK_ERROR("mppe invalid BM tick mode %d\n", bm_tick_mode);
			return SW_BAD_VALUE;
		}
	}
#else
	if (chip_type == APPE_TYPE) {
		switch (bm_tick_mode) {
			case 0:
			case 1:
				/* The unified mode supports the following cases:
				 * 1. 3x10G(port1 128B, port5,6 64B reach to line rate);
				 * 2. 4x2.5G + 1X10G.
				 * 3. 4X1G + 2x10G.
				 */
				num = ARRAY_SIZE(ppe_port_tdm0_tbl);
				bm_cfg = ppe_port_tdm0_tbl;
				break;
			default:
				SSDK_ERROR("Ivalid bm_tick_mode %d\n", bm_tick_mode);
				return SW_BAD_VALUE;
		}
	}
#endif

	for (i = 0; i < num; i++) {
		p_api->adpt_port_tdm_tick_cfg_set(dev_id, i, &bm_cfg[i]);
	}

	tdm_ctrl.enable = A_TRUE;
	tdm_ctrl.offset = A_FALSE;
	tdm_ctrl.depth = num;
	p_api->adpt_port_tdm_ctrl_set(dev_id, &tdm_ctrl);

	return SW_OK;
}
#endif

#if (defined(IN_PORTCONTROL) || defined(IN_LED))
static sw_error_t
qca_appe_portctrl_hw_init(a_uint32_t dev_id)
{
	a_uint32_t i = 0, mac_type_org = 0, mac_type = 0;
	fal_port_cnt_cfg_t init_cnt_cfg = {0};
	fal_port_eee_cfg_t port_eee_cfg = {0};
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	SW_RTN_ON_NULL(priv);

#if defined(MPPE)
	if (adpt_chip_revision_get(dev_id) == MPPE_REVISION) {
		for(i = SSDK_PHYSICAL_PORT0; i < priv->ports; i++) {
			/* PTX buffer threshold need to be updated to 3 on MPPE
			 * for fixing tunnel perfomance issue where MAPT inbound case,
			 * only the buffer size >= 48 can be transmitted out.
			 */
			fal_port_flow_ctrl_thres_set(dev_id, i, 3, 3);

			/* Fix 147B line rate on physical port1 */
			if (i != SSDK_PHYSICAL_PORT0)
				fal_port_rx_fifo_thres_set(dev_id, i, 7);
		}
	}
#endif
	for(i = SSDK_PHYSICAL_PORT1; i < priv->ports; i++) {
		mac_type_org = qca_hppe_port_mac_type_get(dev_id, i);
		for(mac_type = PORT_GMAC_TYPE; mac_type <= PORT_XGMAC_TYPE; mac_type++) {
			qca_hppe_port_mac_type_set(dev_id, i, mac_type);
			fal_port_txmac_status_set(dev_id, i, A_FALSE);
			fal_port_rxmac_status_set(dev_id, i, A_FALSE);
			fal_port_rxfc_status_set(dev_id, i, A_TRUE);
			fal_port_txfc_status_set(dev_id, i, A_TRUE);
			fal_port_max_frame_size_set(dev_id, i, SSDK_MAX_FRAME_SIZE);
			fal_port_interface_eee_cfg_get(dev_id, i, &port_eee_cfg);
			port_eee_cfg.enable = A_FALSE;
			port_eee_cfg.lpi_tx_enable = A_FALSE;
			fal_port_interface_eee_cfg_set(dev_id, i, &port_eee_cfg);
		}
		qca_hppe_port_mac_type_set(dev_id, i, mac_type_org);
		qca_mac_port_status_init(dev_id, i);
#ifdef IN_LED
		ssdk_led_init(dev_id, i);
#endif
	}

	aos_mem_zero(&init_cnt_cfg, sizeof(init_cnt_cfg));
	init_cnt_cfg.uc_tx_cnt_en = FAL_ENABLE;
	init_cnt_cfg.mc_tx_cnt_en = FAL_ENABLE;
	init_cnt_cfg.tl_rx_cnt_en = FAL_ENABLE;
	init_cnt_cfg.rx_cnt_en    = FAL_ENABLE;
	for(i = SSDK_PHYSICAL_PORT0; i < priv->ports; i++) {
		fal_port_cnt_cfg_set(dev_id, FAL_PORT_ID(FAL_PORT_TYPE_PPORT, i), &init_cnt_cfg);
	}

	return SW_OK;
}
#endif

#if defined(IN_SHAPER)
static sw_error_t
qca_appe_shaper_hw_init(a_uint32_t dev_id)
{
	fal_shaper_token_number_t port_token_number, queue_token_number;
	fal_shaper_token_number_t flow_token_number;
	fal_shaper_config_t queue_shaper, flow_shaper;
	fal_shaper_ctrl_t queue_shaper_ctrl, flow_shaper_ctrl;
	a_uint32_t i = 0;

	memset(&queue_shaper, 0, sizeof(queue_shaper));
	memset(&flow_shaper, 0, sizeof(flow_shaper));

	port_token_number.c_token_number_negative_en = A_FALSE;
	port_token_number.c_token_number = APPE_MAX_C_TOKEN_NUM;
	queue_token_number.c_token_number_negative_en = A_FALSE;
	queue_token_number.c_token_number = APPE_MAX_C_TOKEN_NUM;
	queue_token_number.e_token_number_negative_en = A_FALSE;
	queue_token_number.e_token_number = APPE_MAX_E_TOKEN_NUM;
	flow_token_number.c_token_number_negative_en = A_FALSE;
	flow_token_number.c_token_number = APPE_MAX_C_TOKEN_NUM;
	flow_token_number.e_token_number_negative_en = A_FALSE;
	flow_token_number.e_token_number = APPE_MAX_E_TOKEN_NUM;

	for (i = SSDK_PHYSICAL_PORT0; i <= SSDK_PHYSICAL_PORT7; i++) {
		fal_port_shaper_token_number_set(dev_id, i, &port_token_number);
	}

	for(i = 0; i < SSDK_L0SCHEDULER_CFG_MAX; i ++) {
		fal_queue_shaper_token_number_set(dev_id, i, &queue_token_number);
		queue_shaper.meter_type = FAL_SHAPER_METER_MEF10_3;
		queue_shaper.next_ptr = i + 1;
		if (queue_shaper.next_ptr == SSDK_L0SCHEDULER_CFG_MAX) {
			queue_shaper.next_ptr = 0;
		}
		queue_shaper.grp_end = A_TRUE;
		fal_queue_shaper_set(dev_id, i, &queue_shaper);
	}

	for(i = 0; i < SSDK_L1SCHEDULER_CFG_MAX; i ++) {
		fal_flow_shaper_token_number_set(dev_id, i, &flow_token_number);
		flow_shaper.meter_type = FAL_SHAPER_METER_MEF10_3;
		flow_shaper.next_ptr = i + 1;
		if (flow_shaper.next_ptr == SSDK_L1SCHEDULER_CFG_MAX) {
			flow_shaper.next_ptr = 0;
		}
		flow_shaper.grp_end = A_TRUE;
		fal_flow_shaper_set(dev_id, i, &flow_shaper);
	}

	fal_port_shaper_timeslot_set(dev_id, APPE_PORT_SHAPER_TIMESLOT_DFT);
	fal_flow_shaper_timeslot_set(dev_id, APPE_FLOW_SHAPER_TIMESLOT_DFT);
	fal_queue_shaper_timeslot_set(dev_id, APPE_QUEUE_SHAPER_TIMESLOT_DFT);
	fal_shaper_ipg_preamble_length_set(dev_id,
				APPE_SHAPER_IPG_PREAMBLE_LEN_DFT);
	queue_shaper_ctrl.head = APPE_QUEUE_SHAPER_HEAD;
	queue_shaper_ctrl.tail = APPE_QUEUE_SHAPER_TAIL;
	fal_queue_shaper_ctrl_set(dev_id, &queue_shaper_ctrl);

	flow_shaper_ctrl.head = APPE_FLOW_SHAPER_HEAD;
	flow_shaper_ctrl.tail = APPE_FLOW_SHAPER_TAIL;
	fal_flow_shaper_ctrl_set(dev_id, &flow_shaper_ctrl);

	return SW_OK;
}
#endif

#if defined(IN_POLICER)
static sw_error_t
qca_appe_policer_hw_init(a_uint32_t dev_id)
{
	a_uint32_t i = 0;
	a_uint32_t timeslot = 0;
	a_uint32_t acl_policer_cfg_max = 0;
	fal_policer_config_t policer;
	fal_policer_action_t action;
	fal_policer_ctrl_t policer_ctrl;
	fal_policer_frame_type_t frame_type;
	a_uint32_t chip_type = adpt_ppe_type_get(dev_id);

	memset(&policer, 0, sizeof(policer));
	memset(&action, 0, sizeof(action));
	memset(&policer_ctrl, 0, sizeof(policer_ctrl));

	if (chip_type == MPPE_TYPE)
		timeslot = MPPE_POLICER_TIMESLOT_DFT;
	else
		timeslot = APPE_POLICER_TIMESLOT_DFT;

	fal_policer_timeslot_set(dev_id, timeslot);

	for (i = SSDK_PHYSICAL_PORT0; i <= SSDK_PHYSICAL_PORT7; i++) {
		fal_port_policer_compensation_byte_set(dev_id, i, 4);
	}

	acl_policer_cfg_max = (chip_type == MPPE_TYPE)? MPPE_ACL_POLICER_CFG_MAX: APPE_ACL_POLICER_CFG_MAX;
	for (i = 0; i < acl_policer_cfg_max; i++) {
		policer.meter_type = FAL_POLICER_METER_MEF10_3;
		policer.next_ptr = i + 1;
		if (policer.next_ptr == acl_policer_cfg_max) {
			policer.next_ptr = 0;
		}
		policer.grp_end = A_TRUE;
		fal_acl_policer_entry_set(dev_id, i, &policer, &action);
	}

	policer_ctrl.head = APPE_POLICER_HEAD;
	policer_ctrl.tail = (chip_type == MPPE_TYPE)? MPPE_POLICER_TAIL: APPE_POLICER_TAIL;
	fal_policer_ctrl_set(dev_id, &policer_ctrl);

	/* bypass policer for dropped frame */
	frame_type = FAL_FRAME_DROPPED;
	fal_policer_bypass_en_set(dev_id, frame_type, A_TRUE);

	return SW_OK;
}
#endif

#if defined(IN_RSS_HASH)
#define RSS_HASH_MASK 0xFFF

#define RSS_HASH_FIN_INNER_OUTER_0 0x205
#define RSS_HASH_FIN_INNER_OUTER_1 0x264
#define RSS_HASH_FIN_INNER_OUTER_2 0x227
#define RSS_HASH_FIN_INNER_OUTER_3 0x245
#define RSS_HASH_FIN_INNER_OUTER_4 0x201

#define RSS_HASH_PROTOCOL_MIX 0x13
#define RSS_HASH_DPORT_MIX 0xb
#define RSS_HASH_SPORT_MIX 0x13

#define RSS_HASH_SIPV4_MIX 0x13
#define RSS_HASH_DIPV4_MIX 0xb
#define RSS_HASH_SIPV6_MIX_0 0x13
#define RSS_HASH_SIPV6_MIX_1 0xb
#define RSS_HASH_SIPV6_MIX_2 0x13
#define RSS_HASH_SIPV6_MIX_3 0xb
#define RSS_HASH_DIPV6_MIX_0 0x13
#define RSS_HASH_DIPV6_MIX_1 0xb
#define RSS_HASH_DIPV6_MIX_2 0x13
#define RSS_HASH_DIPV6_MIX_3 0xb

static sw_error_t
qca_appe_rss_hash_hw_init(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	fal_rss_hash_config_t config = {0};

	/* get a 32bit random seed */
	get_random_bytes(&config.hash_seed, sizeof(a_uint32_t));

	config.hash_mask = RSS_HASH_MASK;
	config.hash_fragment_mode = A_FALSE;

	config.hash_fin_inner[0] = RSS_HASH_FIN_INNER_OUTER_0 & 0x1f;
	config.hash_fin_outer[0] = (RSS_HASH_FIN_INNER_OUTER_0 >> 5) & 0x1f;
	config.hash_fin_inner[1] = RSS_HASH_FIN_INNER_OUTER_1 & 0x1f;
	config.hash_fin_outer[1] = (RSS_HASH_FIN_INNER_OUTER_1 >> 5) & 0x1f;
	config.hash_fin_inner[2] = RSS_HASH_FIN_INNER_OUTER_2 & 0x1f;
	config.hash_fin_outer[2] = (RSS_HASH_FIN_INNER_OUTER_2 >> 5) & 0x1f;
	config.hash_fin_inner[3] = RSS_HASH_FIN_INNER_OUTER_3 & 0x1f;
	config.hash_fin_outer[3] = (RSS_HASH_FIN_INNER_OUTER_3 >> 5) & 0x1f;
	config.hash_fin_inner[4] = RSS_HASH_FIN_INNER_OUTER_4 & 0x1f;
	config.hash_fin_outer[4] = (RSS_HASH_FIN_INNER_OUTER_4 >> 5) & 0x1f;
	config.hash_protocol_mix = RSS_HASH_PROTOCOL_MIX;
	config.hash_dport_mix = RSS_HASH_DPORT_MIX;
	config.hash_sport_mix = RSS_HASH_SPORT_MIX;

	/* set ipv4 rss hash configuraion */
	config.hash_sip_mix[0] = RSS_HASH_SIPV4_MIX;
	config.hash_dip_mix[0] = RSS_HASH_DIPV4_MIX;
	rv = fal_rss_hash_config_set(dev_id, FAL_RSS_HASH_IPV4ONLY, &config);
	SW_RTN_ON_ERROR(rv);

	/* set ipv6 rss hash configuration */
	config.hash_sip_mix[0] = RSS_HASH_SIPV6_MIX_0;
	config.hash_dip_mix[0] = RSS_HASH_DIPV6_MIX_0;
	config.hash_sip_mix[1] = RSS_HASH_SIPV6_MIX_1;
	config.hash_dip_mix[1] = RSS_HASH_DIPV6_MIX_1;
	config.hash_sip_mix[2] = RSS_HASH_SIPV6_MIX_2;
	config.hash_dip_mix[2] = RSS_HASH_DIPV6_MIX_2;
	config.hash_sip_mix[3] = RSS_HASH_SIPV6_MIX_3;
	config.hash_dip_mix[3] = RSS_HASH_DIPV6_MIX_3;
	rv = fal_rss_hash_config_set(dev_id, FAL_RSS_HASH_IPV6ONLY, &config);

	return rv;
}
#endif

sw_error_t qca_appe_hw_init(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;

	/* reset ppe */
	ssdk_ppe_reset_init(dev_id);

	rv = qca_switch_init(dev_id);
	SW_RTN_ON_ERROR(rv);

#if defined(IN_BM)
	rv = qca_hppe_bm_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_QM)
	rv = qca_hppe_qm_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_QOS)
	rv = qca_hppe_qos_scheduler_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_BM) && defined(IN_QOS)
	rv = qca_appe_tdm_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_FDB)
	rv= qca_hppe_fdb_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_VSI)
	rv= qca_hppe_vsi_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_PORTVLAN)
	rv = qca_hppe_portvlan_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

	rv = qca_hppe_interface_mode_init(dev_id);
	SW_RTN_ON_ERROR(rv);

#if (defined(IN_PORTCONTROL) || defined(IN_LED))
	rv = qca_appe_portctrl_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_POLICER)
	rv = qca_appe_policer_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_SHAPER)
	rv = qca_appe_shaper_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_FLOW)
	rv = qca_hppe_flow_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_ACL)
	rv = qca_hppe_acl_byp_intf_mac_learn(dev_id);
	SW_RTN_ON_ERROR(rv);
#if defined(IN_PTP)
	rv = qca_hppe_acl_remark_ptp_servcode(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#endif

#if defined(IN_CTRLPKT)
	rv = qca_hppe_ctlpkt_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

#if defined(IN_RSS_HASH)
	rv = qca_appe_rss_hash_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
	return rv;
}
