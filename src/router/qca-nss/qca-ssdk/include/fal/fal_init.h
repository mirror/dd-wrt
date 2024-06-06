/*
 * Copyright (c) 2012, 2016-2019, The Linux Foundation. All rights reserved.
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

/*qca808x_start*/
/**
 * @defgroup fal_init FAL_INIT
 * @{
 */
#ifndef _FAL_INIT_H_
#define _FAL_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "ssdk_init.h"
/*qca808x_end*/
enum{
	FAL_MODULE_ACL,
	FAL_MODULE_VSI,
	FAL_MODULE_IP,
	FAL_MODULE_FLOW,
	FAL_MODULE_QM,
	FAL_MODULE_QOS,
	FAL_MODULE_BM,
	FAL_MODULE_SERVCODE,
	FAL_MODULE_RSS_HASH,
	FAL_MODULE_PPPOE,
	FAL_MODULE_SHAPER,
	FAL_MODULE_PORTCTRL,
	FAL_MODULE_MIB,
	FAL_MODULE_MIRROR,
	FAL_MODULE_FDB,
	FAL_MODULE_STP,
	FAL_MODULE_TRUNK,
	FAL_MODULE_PORTVLAN,
	FAL_MODULE_CTRLPKT,
	FAL_MODULE_SEC,
	FAL_MODULE_POLICER,
	FAL_MODULE_MISC,
	FAL_MODULE_PTP,
	FAL_MODULE_SFP,
	FAL_MODULE_VPORT,
	FAL_MODULE_TUNNEL,
	FAL_MODULE_VXLAN,
	FAL_MODULE_GENEVE,
	FAL_MODULE_TUNNEL_PROGRAM,
	FAL_MODULE_MAPT,
	FAL_MODULE_LED,
	FAL_MODULE_ATHTAG,
/* auto_insert_flag */
	FAL_MODULE_MAX,
};

typedef struct
{
	a_uint32_t bitmap[3];
}fal_func_ctrl_t;

typedef struct
{
	a_uint32_t flow_caps;
	a_uint32_t host_caps;
	a_uint32_t nexthop_caps;
	a_uint32_t pub_ip_caps;
	a_uint32_t vsi_caps;
	a_uint32_t port_caps;
	a_uint32_t l3_if_caps;
	a_uint32_t my_mac_caps;
	a_uint32_t queue_caps;
	a_uint32_t service_code_caps;
	a_uint32_t pppoe_session_caps;
	a_uint32_t policer_caps;
} fal_ppe_tbl_caps_t;

/*qca808x_start*/
sw_error_t fal_init(a_uint32_t dev_id, ssdk_init_cfg * cfg);
/*qca808x_end*/
sw_error_t fal_reset(a_uint32_t dev_id);
sw_error_t fal_ssdk_cfg(a_uint32_t dev_id, ssdk_cfg_t *ssdk_cfg);
/*qca808x_start*/
sw_error_t fal_cleanup(a_uint32_t dev_id);
/*qca808x_end*/
sw_error_t fal_switch_devid_get(ssdk_chip_type chip_type, a_uint32_t *pdev_id);
sw_error_t
fal_ppe_capacity_get(a_uint32_t dev_id, fal_ppe_tbl_caps_t *ppe_capacity);
/*qca808x_start*/
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_INIT_H_ */
/**
 * @}
 */
/*qca808x_end*/
