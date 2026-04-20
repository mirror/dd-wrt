/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#ifndef _APPE_VXLAN_H_
#define _APPE_VXLAN_H_

#define UDP_PORT_CFG_MAX_ENTRY	6

sw_error_t
appe_udp_port_cfg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union udp_port_cfg_u *value);

sw_error_t
appe_udp_port_cfg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union udp_port_cfg_u *value);

sw_error_t
appe_tpr_vxlan_cfg_get(
		a_uint32_t dev_id,
		union tpr_vxlan_cfg_u *value);

sw_error_t
appe_tpr_vxlan_cfg_set(
		a_uint32_t dev_id,
		union tpr_vxlan_cfg_u *value);

sw_error_t
appe_tpr_vxlan_gpe_cfg_get(
		a_uint32_t dev_id,
		union tpr_vxlan_gpe_cfg_u *value);

sw_error_t
appe_tpr_vxlan_gpe_cfg_set(
		a_uint32_t dev_id,
		union tpr_vxlan_gpe_cfg_u *value);

#ifndef IN_VXLAN_MINI
sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_get(
		a_uint32_t dev_id,
		union tpr_vxlan_gpe_prot_cfg_u *value);

sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_set(
		a_uint32_t dev_id,
		union tpr_vxlan_gpe_prot_cfg_u *value);

#endif

#if 0
sw_error_t
appe_udp_port_cfg_udp_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_udp_port_cfg_udp_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_udp_port_cfg_port_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_udp_port_cfg_port_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_udp_port_cfg_ip_ver_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_udp_port_cfg_ip_ver_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_udp_port_cfg_port_value_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_udp_port_cfg_port_value_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif

sw_error_t
appe_tpr_vxlan_cfg_udp_port_map_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_vxlan_cfg_udp_port_map_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_vxlan_gpe_cfg_udp_port_map_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_vxlan_gpe_cfg_udp_port_map_set(
		a_uint32_t dev_id,
		unsigned int value);

#if 0
sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_ipv6_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_ipv6_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_ethernet_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_ethernet_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_ipv4_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_tpr_vxlan_gpe_prot_cfg_ipv4_set(
		a_uint32_t dev_id,
		unsigned int value);
#endif
#endif

