/*
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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
 * @defgroup
 * @{
 */
#ifndef _ADPT_HPPE_H_
#define _ADPT_HPPE_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

void adpt_hppe_fdb_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_fdb_init(a_uint32_t dev_id);
void adpt_hppe_portvlan_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_portvlan_init(a_uint32_t dev_id);
void adpt_hppe_ctrlpkt_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_ctrlpkt_init(a_uint32_t dev_id);
void adpt_hppe_servcode_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_servcode_init(a_uint32_t dev_id);
void adpt_hppe_rss_hash_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_rss_hash_init(a_uint32_t dev_id);
void adpt_hppe_mib_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_mib_init(a_uint32_t dev_id);
void adpt_hppe_stp_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_stp_init(a_uint32_t dev_id);
void adpt_hppe_vsi_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_vsi_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_port_ctrl_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_misc_init(a_uint32_t dev_id);

void adpt_hppe_mirror_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_mirror_init(a_uint32_t dev_id);
void adpt_hppe_trunk_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_trunk_init(a_uint32_t dev_id);
void adpt_hppe_ip_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_ip_init(a_uint32_t dev_id);
void adpt_hppe_qm_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_qm_init(a_uint32_t dev_id);
void adpt_hppe_flow_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_flow_init(a_uint32_t dev_id);

void adpt_hppe_pppoe_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_pppoe_init(a_uint32_t dev_id);
void adpt_hppe_sec_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_sec_init(a_uint32_t dev_id);

void adpt_hppe_acl_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_acl_init(a_uint32_t dev_id);
void adpt_hppe_qos_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_qos_init(a_uint32_t dev_id);
void adpt_hppe_bm_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_hppe_bm_init(a_uint32_t dev_id);

sw_error_t adpt_hppe_shaper_init(a_uint32_t dev_id);

void adpt_hppe_port_ctrl_func_bitmap_init(a_uint32_t dev_id);

void adpt_hppe_shaper_func_bitmap_init(a_uint32_t dev_id);

sw_error_t adpt_hppe_policer_init(a_uint32_t dev_id);

void adpt_hppe_policer_func_bitmap_init(a_uint32_t dev_id);

sw_error_t adpt_hppe_uniphy_init(a_uint32_t dev_id);

sw_error_t adpt_hppe_ptp_init(a_uint32_t dev_id);

/*shaper*/
#define HPPE_MAX_C_TOKEN_NUM 0x3fffffff
#define HPPE_MAX_E_TOKEN_NUM 0x3fffffff

#define HPPE_POLICER_TIMESLOT_DFT 600
#define HPPE_PORT_SHAPER_TIMESLOT_DFT 8
#define HPPE_FLOW_SHAPER_TIMESLOT_DFT 64
#define HPPE_QUEUE_SHAPER_TIMESLOT_DFT 300
#define HPPE_SHAPER_IPG_PREAMBLE_LEN_DFT 20

/*BM*/
#if defined(MPPE)
#define PPE_BM_PORT_NUM		10
#define PPE_BM_PHY_PORT_MAX	9
#define PPE_BM_PHY_PORT_OFFSET  8
#else
#define PPE_BM_PORT_NUM		15
#define PPE_BM_PHY_PORT_MAX	13
#define PPE_BM_PHY_PORT_OFFSET  8
#endif

#define PPE_BM_PORT_MIN         0

#define HPPE_REVISION              0x0
#define CPPE_REVISION              0x1
#define APPE_REVISION              0x0
#define MPPE_REVISION              0x1
#define UNKNOWN_REVISION           0xff

#ifdef IN_FDB
sw_error_t
adpt_hppe_fdb_del_by_port(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t flag);
#endif

#ifdef IN_PORTCONTROL
sw_error_t
adpt_ppe_port_tdm_resource_set(a_uint32_t dev_id, a_bool_t enable);
#endif

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
