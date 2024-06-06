/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _ADPT_APPE_PORTVLAN_H_
#define _ADPT_APPE_PORTVLAN_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#ifndef IN_PORTVLAN_MINI
sw_error_t
adpt_appe_port_vlan_vpgroup_set(a_uint32_t dev_id, a_uint32_t vport_id,
		fal_port_vlan_direction_t direction, a_uint32_t vpgroup_id);
sw_error_t
adpt_appe_port_vlan_vpgroup_get(a_uint32_t dev_id, a_uint32_t vport_id,
		fal_port_vlan_direction_t direction, a_uint32_t *vpgroup_id);
sw_error_t
adpt_appe_portvlan_isol_group_get(a_uint32_t dev_id,
		a_uint8_t isol_group_id, a_uint64_t *isol_group_bmp);
sw_error_t
adpt_appe_portvlan_isol_group_set(a_uint32_t dev_id,
		a_uint8_t isol_group_id, a_uint64_t *isol_group_bmp);
sw_error_t
adpt_appe_portvlan_isol_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_portvlan_isol_ctrl_t *isol_ctrl);
sw_error_t
adpt_appe_portvlan_isol_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_portvlan_isol_ctrl_t *isol_ctrl);
sw_error_t
adpt_appe_port_egress_vlan_filter_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_egress_vlan_filter_t *filter);
sw_error_t
adpt_appe_port_egress_vlan_filter_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_egress_vlan_filter_t *filter);
#endif

sw_error_t
adpt_appe_portvlan_vpmember_get(a_uint32_t dev_id, fal_port_t port_id, fal_pbmp_t * mem_port_map);
sw_error_t
adpt_appe_portvlan_vpmember_update(a_uint32_t dev_id, fal_port_t port_id, fal_pbmp_t mem_port_map);
sw_error_t
adpt_appe_portvlan_vpmember_add(a_uint32_t dev_id, fal_port_t port_id, fal_port_t mem_port_id);
sw_error_t
adpt_appe_portvlan_vpmember_del(a_uint32_t dev_id, fal_port_t port_id, fal_port_t mem_port_id);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */

#endif
