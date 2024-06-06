/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
*
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

#ifndef _ADPT_HPPE_UNIPHY_H_
#define _ADPT_HPPE_UNIPHY_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define UNIPHY_XPCS_TSL_TIMER         0xa
#define UNIPHY_XPCS_TLU_TIMER         0x3
#define UNIPHY_XPCS_TWL_TIMER         0x16
#define UNIPHY_XPCS_100US_TIMER       0xc8
#define UNIPHY_XPCS_TWR_TIMER         0x1c
#define UNIPHY_UQXGMII_MODE           0x5
#define UNIPHY_UDXGMII_MODE           0x3
#define UNIPHY_UQXGMII_AM_COUNT       0x6018

sw_error_t
adpt_hppe_uniphy_usxgmii_status_get(a_uint32_t dev_id, a_uint32_t uniphy_index,
		a_uint32_t port_id, union sr_mii_ctrl_u *sr_mii_ctrl);
sw_error_t
adpt_hppe_uniphy_usxgmii_status_set(a_uint32_t dev_id, a_uint32_t uniphy_index,
		a_uint32_t port_id, union sr_mii_ctrl_u *sr_mii_ctrl);
sw_error_t
adpt_hppe_uniphy_usxgmii_autoneg_status_get(a_uint32_t dev_id, a_uint32_t uniphy_index,
		a_uint32_t port_id, union vr_mii_an_intr_sts_u *vr_mii_an_intr_sts);
sw_error_t
adpt_hppe_uniphy_usxgmii_autoneg_status_set(a_uint32_t dev_id, a_uint32_t uniphy_index,
		a_uint32_t port_id, union vr_mii_an_intr_sts_u *vr_mii_an_intr_sts);
a_uint32_t
adpt_hppe_port_get_by_uniphy(a_uint32_t dev_id, a_uint32_t uniphy_index,
		a_uint32_t channel);
a_bool_t
adpt_hppe_uniphy_usxgmii_port_check(a_uint32_t dev_id, a_uint32_t uniphy_index,
		a_uint32_t port_id);
a_uint32_t
adpt_ppe_uniphy_number_get(a_uint32_t dev_id);
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
