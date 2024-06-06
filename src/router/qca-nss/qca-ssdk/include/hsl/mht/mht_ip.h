/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _MHT_IP_H_
#define _MHT_IP_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "fal_ip.h"
sw_error_t
mht_ip_vrf_base_addr_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr);

sw_error_t
mht_ip_vrf_base_addr_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr);

sw_error_t
mht_ip_vrf_base_mask_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr);

sw_error_t
mht_ip_vrf_base_mask_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr);

sw_error_t
mht_ip_default_route_set(a_uint32_t dev_id, a_uint32_t droute_id, fal_default_route_t * entry);

sw_error_t
mht_ip_default_route_get(a_uint32_t dev_id, a_uint32_t droute_id, fal_default_route_t * entry);

sw_error_t
mht_ip_host_route_set(a_uint32_t dev_id, a_uint32_t hroute_id, fal_host_route_t * entry);

sw_error_t
mht_ip_host_route_get(a_uint32_t dev_id, a_uint32_t hroute_id, fal_host_route_t * entry);

sw_error_t
mht_default_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id,
		fal_flow_type_t type, fal_default_flow_cmd_t cmd);

sw_error_t
mht_default_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id,
		fal_flow_type_t type, fal_default_flow_cmd_t * cmd);

sw_error_t
mht_default_rt_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id,
		fal_flow_type_t type, fal_default_flow_cmd_t cmd);

sw_error_t
mht_default_rt_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id,
		fal_flow_type_t type, fal_default_flow_cmd_t * cmd);

sw_error_t
mht_ip_glb_lock_time_set(a_uint32_t dev_id, fal_glb_lock_time_t lock_time);

sw_error_t
mht_ip_rfs_ip4_set(a_uint32_t dev_id, fal_ip4_rfs_t * rfs);

sw_error_t
mht_ip_rfs_ip6_set(a_uint32_t dev_id, fal_ip6_rfs_t * rfs);

sw_error_t
mht_ip_rfs_ip4_del(a_uint32_t dev_id, fal_ip4_rfs_t * rfs);

sw_error_t
mht_ip_rfs_ip6_del(a_uint32_t dev_id, fal_ip6_rfs_t * rfs);

sw_error_t
mht_ip_wcmp_entry_set(a_uint32_t dev_id, a_uint32_t wcmp_id, fal_ip_wcmp_t * wcmp);

sw_error_t
mht_ip_wcmp_entry_get(a_uint32_t dev_id, a_uint32_t wcmp_id, fal_ip_wcmp_t * wcmp);
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _MHT_IP_H_ */
