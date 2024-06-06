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


/**
 * @defgroup mht_init MHT_SEC_CTRL
 * @{
 */
#ifndef _MHT_SEC_CTRL_H_
#define _MHT_SEC_CTRL_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "ssdk_plat.h"
#include "mht_reg.h"

typedef enum {
	MHT_SWITCH_MODE =
		(BIT(WORK_MODE_PHY3_SEL1_BOFFSET)),
	MHT_SWITCH_BYPASS_PORT5_MODE =
		(BIT(WORK_MODE_PORT5_SEL_BOFFSET)),
	MHT_PHY_UQXGMII_MODE =
		(BIT(WORK_MODE_PORT5_SEL_BOFFSET) |
		BIT(WORK_MODE_PHY3_SEL0_BOFFSET) |
		BIT(WORK_MODE_PHY2_SEL_BOFFSET) |
		BIT(WORK_MODE_PHY1_SEL_BOFFSET) |
		BIT(WORK_MODE_PHY0_SEL_BOFFSET)),
	MHT_PHY_SGMII_UQXGMII_MODE =
		(BIT(WORK_MODE_PORT5_SEL_BOFFSET) |
		BIT(WORK_MODE_PHY2_SEL_BOFFSET) |
		BIT(WORK_MODE_PHY1_SEL_BOFFSET) |
		BIT(WORK_MODE_PHY0_SEL_BOFFSET)),
	MHT_WORK_MODE_MAX,
} mht_work_mode_t;

#define MHT_WORK_MODE_MASK \
	(BITS(WORK_MODE_PHY0_SEL_BOFFSET, WORK_MODE_PORT5_SEL_BOFFSET + 1))

#define MHT_MEM_CTRL_DVS_MASK \
	(BITS(MEM_CTRL_DVS_SA_RELAX_BOFFSET, MEM_CTRL_DVS_SA_RELAX_BLEN+1))

#define MHT_MEM_CTRL_DVS_PHY_MODE \
	BIT(MEM_CTRL_DVS_RAWA_ASSERT_BOFFSET)
#define MHT_MEM_CTRL_DVS_SWITCH_MODE \
	BIT(MEM_CTRL_DVS_RAWA_ASSERT_BOFFSET)

#define MHT_MEM_ACC_0_SWITCH_MODE				0x000c0c0c
#define MHT_MEM_ACC_0_PHY_MODE					0

#define MHT_SKU_MASK						0xfffff
#define MHT_SKU_8082						0x1dc
#define MHT_SKU_8084						0x1dd
#define MHT_SKU_8085						0x1de
#define MHT_SKU_8386						0x1df

sw_error_t
qca_mht_work_mode_set(a_uint32_t dev_id, mht_work_mode_t work_mode);

sw_error_t
qca_mht_work_mode_get(a_uint32_t dev_id, mht_work_mode_t *work_mode);

sw_error_t
qca_mht_serdes_addr_get(a_uint32_t dev_id, a_uint32_t serdes_id,
	a_uint32_t *address);

sw_error_t
qca_mht_ephy_addr_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t *address);

sw_error_t
qca_mht_port_id_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *mht_port_id);

sw_error_t
qca_mht_phy_intr_enable(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t intr_bmp);

sw_error_t
qca_mht_switch_intr_set(a_uint32_t dev_id, a_bool_t enable);

sw_error_t
qca_mht_switch_intr_get(a_uint32_t dev_id, a_bool_t *enable);

sw_error_t
qca_mht_switch_intr_status_get(a_uint32_t dev_id, a_bool_t *enable);

#if defined(IN_PTP)
sw_error_t
qca_mht_ptp_sync_set(a_uint32_t dev_id, a_uint32_t mht_port_id, a_bool_t enable);

sw_error_t
qca_mht_ptp_sync_get(a_uint32_t dev_id, a_uint32_t mht_port_id, a_bool_t *enable);

sw_error_t
qca_mht_ptp_async_set(a_uint32_t dev_id, a_uint32_t mht_port_id, a_uint32_t src_id);

sw_error_t
qca_mht_ptp_async_get(a_uint32_t dev_id, a_uint32_t mht_port_id, a_uint32_t *src_id);
#endif

sw_error_t
qca_mht_mem_ctrl_set(a_uint32_t dev_id, a_uint32_t dvs_value, a_uint32_t acc_value);

a_bool_t
qca_mht_sku_check(a_uint32_t dev_id, a_uint32_t mht_sku);

a_bool_t
qca_mht_sku_uniphy_enabled(a_uint32_t dev_id, a_uint32_t uniphy_index);

a_bool_t
qca_mht_sku_switch_core_enabled(a_uint32_t dev_id);

sw_error_t
qca_mht_ethphy_icc_efuse_get(a_uint32_t dev_id, a_uint32_t mht_port_id,
	a_uint32_t *icc_value);
sw_error_t
qca_mht_mdio_cfg(a_uint32_t dev_id, a_uint32_t div, a_uint32_t timer, a_uint32_t preamble_length);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MHT_SEC_CTRL_H_ */
/**
 * @}
 */
