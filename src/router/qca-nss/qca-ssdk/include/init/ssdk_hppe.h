/*
 * Copyright (c) 2017, 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _SSDK_HPPE_H_
#define _SSDK_HPPE_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "init/ssdk_init.h"

sw_error_t qca_hppe_hw_init(a_uint32_t dev_id);
sw_error_t qca_hppe_vsi_hw_init(a_uint32_t dev_id);
sw_error_t qca_hppe_portvlan_hw_init(a_uint32_t dev_id);
sw_error_t qca_hppe_flow_hw_init(a_uint32_t dev_id);
sw_error_t qca_hppe_fdb_hw_init(a_uint32_t dev_id);
sw_error_t qca_hppe_ctlpkt_hw_init(a_uint32_t dev_id);
sw_error_t qca_hppe_acl_byp_intf_mac_learn(a_uint32_t dev_id);
sw_error_t qca_hppe_acl_remark_ptp_servcode(a_uint32_t dev_id);
sw_error_t qca_hppe_interface_mode_init(a_uint32_t dev_id);
sw_error_t
qca_hppe_qos_scheduler_hw_init(a_uint32_t dev_id);
sw_error_t
qca_hppe_bm_hw_init(a_uint32_t dev_id);
sw_error_t
qca_hppe_qm_hw_init(a_uint32_t dev_id);
sw_error_t
qca_hppe_tdm_hw_init(a_uint32_t dev_id, a_bool_t enable);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _SSDK_HPPE_H */

