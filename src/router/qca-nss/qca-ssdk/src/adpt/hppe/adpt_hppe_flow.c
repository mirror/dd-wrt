/*
 * Copyright (c) 2016-2017, 2019, 2021, The Linux Foundation. All rights reserved.
 *
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

/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "fal_flow.h"
#include "hppe_ip_reg.h"
#include "hppe_ip.h"
#include "hppe_flow_reg.h"
#include "hppe_flow.h"
#include "adpt_hppe.h"
#include "adpt.h"
#if defined(CPPE) || defined(APPE)
#include "adpt_cppe_flow.h"
#endif
#if defined(MPPE)
#include "cppe_qos_reg.h"
#include "cppe_qos.h"
#include "mppe_athtag_reg.h"
#include "mppe_athtag.h"
#endif

#define FLOW_ENTRY_TYPE_IPV4 0
#define FLOW_ENTRY_TYPE_IPV6 1
#define FLOW_TUPLE_TYPE_3    0

sw_error_t
adpt_hppe_ip_flow_host_data_rd_add(a_uint32_t dev_id, fal_host_entry_t * host_entry)

{
	a_uint8_t mode = 0, type = 0;
	sw_error_t rv = SW_OK;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(host_entry);

	mode = host_entry->flags >> 24;
	type = host_entry->flags & 0xff;

	if ((type & FAL_IP_IP4_ADDR) == FAL_IP_IP4_ADDR) {
		union host_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.valid= host_entry->status;
		entry.bf.key_type = 0;
		entry.bf.fwd_cmd = host_entry->action;
		entry.bf.syn_toggle = host_entry->syn_toggle;
		entry.bf.dst_info = host_entry->dst_info;
		entry.bf.lan_wan = host_entry->lan_wan;
		entry.bf.ip_addr = host_entry->ip4_addr;
		rv = hppe_flow_host_ipv4_data_rd_add(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
	} else if ((type & FAL_IP_IP6_ADDR) == FAL_IP_IP6_ADDR) {
		union host_ipv6_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.valid= host_entry->status;
		entry.bf.key_type = 2;
		entry.bf.fwd_cmd = host_entry->action;
		entry.bf.syn_toggle = host_entry->syn_toggle;
		entry.bf.dst_info = host_entry->dst_info;
		entry.bf.lan_wan = host_entry->lan_wan;
		entry.bf.ipv6_addr_0 = host_entry->ip6_addr.ul[3];
		entry.bf.ipv6_addr_1 = host_entry->ip6_addr.ul[3] >> 10 | \
							host_entry->ip6_addr.ul[2] << 22;
		entry.bf.ipv6_addr_2 = host_entry->ip6_addr.ul[2] >> 10 | \
							host_entry->ip6_addr.ul[1] << 22;
		entry.bf.ipv6_addr_3 = host_entry->ip6_addr.ul[1] >> 10 | \
							host_entry->ip6_addr.ul[0] << 22;
		entry.bf.ipv6_addr_4 = host_entry->ip6_addr.ul[0] >> 10;
		rv = hppe_flow_host_ipv6_data_rd_add(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
	}

	return SW_OK;
}

sw_error_t
adpt_hppe_ip_flow_host_data_add(a_uint32_t dev_id, fal_host_entry_t * host_entry)
{
	a_uint8_t mode = 0, type = 0;
	sw_error_t rv = SW_OK;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(host_entry);

	mode = host_entry->flags >> 24;
	type = host_entry->flags & 0xff;

	if ((type & FAL_IP_IP4_ADDR) == FAL_IP_IP4_ADDR) {
		union host_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.valid= host_entry->status;
		entry.bf.key_type = 0;
		entry.bf.fwd_cmd = host_entry->action;
		entry.bf.syn_toggle = host_entry->syn_toggle;
		entry.bf.dst_info = host_entry->dst_info;
		entry.bf.lan_wan = host_entry->lan_wan;
		entry.bf.ip_addr = host_entry->ip4_addr;
		rv = hppe_flow_host_ipv4_data_add(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
	} else if ((type & FAL_IP_IP6_ADDR) == FAL_IP_IP6_ADDR) {
		union host_ipv6_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.valid= host_entry->status;
		entry.bf.key_type = 2;
		entry.bf.fwd_cmd = host_entry->action;
		entry.bf.syn_toggle = host_entry->syn_toggle;
		entry.bf.dst_info = host_entry->dst_info;
		entry.bf.lan_wan = host_entry->lan_wan;
		entry.bf.ipv6_addr_0 = host_entry->ip6_addr.ul[3];
		entry.bf.ipv6_addr_1 = host_entry->ip6_addr.ul[3] >> 10 | \
							host_entry->ip6_addr.ul[2] << 22;
		entry.bf.ipv6_addr_2 = host_entry->ip6_addr.ul[2] >> 10 | \
							host_entry->ip6_addr.ul[1] << 22;
		entry.bf.ipv6_addr_3 = host_entry->ip6_addr.ul[1] >> 10 | \
							host_entry->ip6_addr.ul[0] << 22;
		entry.bf.ipv6_addr_4 = host_entry->ip6_addr.ul[0] >> 10;
		rv = hppe_flow_host_ipv6_data_add(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
	}

	return SW_OK;
}

sw_error_t
adpt_hppe_ip_flow_host_data_get(a_uint32_t dev_id, a_uint32_t get_mode,
                    fal_host_entry_t *host_entry)
{
	a_uint8_t mode = 0, type = 0;
	sw_error_t rv = SW_OK;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(host_entry);

	mode = host_entry->flags >> 24;
	type = host_entry->flags & 0xff;

	if (get_mode & FAL_IP_ENTRY_ID_EN)
		mode = 1;
	else if (get_mode & FAL_IP_ENTRY_IPADDR_EN)
		mode  = 0;

	if ((type & FAL_IP_IP4_ADDR) == FAL_IP_IP4_ADDR) {
		union host_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.key_type = 0;
		entry.bf.ip_addr = host_entry->ip4_addr;
		rv = hppe_flow_host_ipv4_data_get(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
		host_entry->ip4_addr = entry.bf.ip_addr;
		host_entry->lan_wan = entry.bf.lan_wan;
		host_entry->dst_info= entry.bf.dst_info;
		host_entry->syn_toggle = entry.bf.syn_toggle;
		host_entry->action = entry.bf.fwd_cmd;
		host_entry->status = entry.bf.valid;
	} else if ((type & FAL_IP_IP6_ADDR) == FAL_IP_IP6_ADDR) {
		union host_ipv6_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.key_type = 2;
		entry.bf.ipv6_addr_0 = host_entry->ip6_addr.ul[3];
		entry.bf.ipv6_addr_1 = host_entry->ip6_addr.ul[3] >> 10 | \
							host_entry->ip6_addr.ul[2] << 22;
		entry.bf.ipv6_addr_2 = host_entry->ip6_addr.ul[2] >> 10 | \
							host_entry->ip6_addr.ul[1] << 22;
		entry.bf.ipv6_addr_3 = host_entry->ip6_addr.ul[1] >> 10 | \
							host_entry->ip6_addr.ul[0] << 22;
		entry.bf.ipv6_addr_4 = host_entry->ip6_addr.ul[0] >> 10;
		rv = hppe_flow_host_ipv6_data_get(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
		host_entry->ip6_addr.ul[3] = entry.bf.ipv6_addr_0 | entry.bf.ipv6_addr_1 << 10;
		host_entry->ip6_addr.ul[2] = entry.bf.ipv6_addr_1 >> 22 | entry.bf.ipv6_addr_2 << 10;
		host_entry->ip6_addr.ul[1] = entry.bf.ipv6_addr_2 >> 22 | entry.bf.ipv6_addr_3 << 10;
		host_entry->ip6_addr.ul[0] = entry.bf.ipv6_addr_3 >> 22 | entry.bf.ipv6_addr_4 << 10;
		host_entry->lan_wan = entry.bf.lan_wan;
		host_entry->dst_info= entry.bf.dst_info;
		host_entry->syn_toggle = entry.bf.syn_toggle;
		host_entry->action = entry.bf.fwd_cmd;
		host_entry->status = entry.bf.valid;
	}
	return rv;
}

sw_error_t
adpt_hppe_ip_flow_host_data_del(a_uint32_t dev_id, a_uint32_t del_mode,
                    fal_host_entry_t * host_entry)
{
	a_uint8_t mode = 0, type = 0;
	sw_error_t rv = SW_OK;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(host_entry);

	mode = host_entry->flags >> 24;
	type = host_entry->flags & 0xff;

	if (del_mode & FAL_IP_ENTRY_ID_EN)
		mode = 1;
	else if (del_mode & FAL_IP_ENTRY_IPADDR_EN)
		mode  = 0;
	else if (del_mode & FAL_IP_ENTRY_ALL_EN) {
		return hppe_flow_host_flush_common(dev_id);
	}

	if ((type & FAL_IP_IP4_ADDR) == FAL_IP_IP4_ADDR) {
		union host_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.valid= 1;
		entry.bf.key_type = 0;
		entry.bf.fwd_cmd = host_entry->action;
		entry.bf.syn_toggle = host_entry->syn_toggle;
		entry.bf.dst_info = host_entry->dst_info;
		entry.bf.lan_wan = host_entry->lan_wan;
		entry.bf.ip_addr = host_entry->ip4_addr;
		rv = hppe_flow_host_ipv4_data_del(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
	} else if ((type & FAL_IP_IP6_ADDR) == FAL_IP_IP6_ADDR) {
		union host_ipv6_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf.valid= 1;
		entry.bf.key_type = 2;
		entry.bf.fwd_cmd = host_entry->action;
		entry.bf.syn_toggle = host_entry->syn_toggle;
		entry.bf.dst_info = host_entry->dst_info;
		entry.bf.lan_wan = host_entry->lan_wan;
		entry.bf.ipv6_addr_0 = host_entry->ip6_addr.ul[3];
		entry.bf.ipv6_addr_1 = host_entry->ip6_addr.ul[3] >> 10 | \
							host_entry->ip6_addr.ul[2] << 22;
		entry.bf.ipv6_addr_2 = host_entry->ip6_addr.ul[2] >> 10 | \
							host_entry->ip6_addr.ul[1] << 22;
		entry.bf.ipv6_addr_3 = host_entry->ip6_addr.ul[1] >> 10 | \
							host_entry->ip6_addr.ul[0] << 22;
		entry.bf.ipv6_addr_4 = host_entry->ip6_addr.ul[0] >> 10;
		rv = hppe_flow_host_ipv6_data_del(dev_id, (a_uint32_t)mode, &host_entry->entry_id, &entry);
	}
	return rv;
}


sw_error_t
adpt_hppe_flow_entry_host_op_add(
		a_uint32_t dev_id,
		a_uint32_t add_mode, /*index or hash*/
		fal_flow_entry_t *flow_entry)
{
	sw_error_t rv = SW_OK;
	a_uint32_t type = 0;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_entry);

	type = flow_entry->entry_type;
	if ((type & FAL_FLOW_IP4_5TUPLE_ADDR) == FAL_FLOW_IP4_5TUPLE_ADDR) {
		union in_flow_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
			entry.bf0.l4_port1 = flow_entry->snat_srcport;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
			entry.bf3.l4_port2 = flow_entry->dnat_dstport;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv4_5tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_5TUPLE_ADDR) == FAL_FLOW_IP6_5TUPLE_ADDR) {
		union in_flow_ipv6_5tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv6_5tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP4_3TUPLE_ADDR) == FAL_FLOW_IP4_3TUPLE_ADDR) {
		union in_flow_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 3;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv4_3tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_3TUPLE_ADDR) == FAL_FLOW_IP6_3TUPLE_ADDR) {
		union in_flow_ipv6_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf1.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf0.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf0.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf0.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf0.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf0.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf0.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv6_3tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else
		return SW_FAIL;
	if (rv == SW_OK) {
		union eg_flow_tree_map_tbl_u eg_treemap;
		fal_flow_qos_t *flow_qos = &(flow_entry->flow_qos);
		aos_mem_zero(&eg_treemap, sizeof(eg_treemap));

		rv = hppe_eg_flow_tree_map_tbl_get(dev_id, flow_entry->entry_id, &eg_treemap);
		SW_RTN_ON_ERROR(rv);

#if defined(MPPE)
		mppe_qos_mapping_tbl_flow_policer_set(dev_id, flow_entry->entry_id,
				flow_entry->policer_valid, flow_entry->policer_index);

		eg_treemap.bf.type = flow_qos->qos_type;
		/* flow cookies only has 16bits */
		if (flow_qos->qos_type == FAL_FLOW_QOS_TYPE_COOKIE)
			flow_qos->tree_id &= 0xffff;
#endif
		eg_treemap.bf.tree_id = flow_qos->tree_id;
#if defined(APPE)
		eg_treemap.bf.wifi_qos_flag = flow_qos->wifi_qos_en;
		eg_treemap.bf.wifi_qos = flow_qos->wifi_qos;
#endif
		rv = hppe_eg_flow_tree_map_tbl_set(dev_id, flow_entry->entry_id, &eg_treemap);
	}
	return rv;
}

sw_error_t
adpt_hppe_flow_entry_host_op_get(
		a_uint32_t dev_id,
		a_uint32_t get_mode,
		fal_flow_entry_t *flow_entry)
{
	sw_error_t rv = SW_OK;
	a_uint32_t type = 0;
	a_uint32_t entry_id = flow_entry->entry_id;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_entry);

	type = flow_entry->entry_type;
	if ((type & FAL_FLOW_IP4_5TUPLE_ADDR) == FAL_FLOW_IP4_5TUPLE_ADDR) {
		union in_flow_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv4_5tuple_get(dev_id, get_mode, &entry_id, &entry);
		flow_entry->entry_id = entry_id;
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf0.next_hop1;
			flow_entry->snat_srcport = entry.bf0.l4_port1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
			flow_entry->dnat_dstport = entry.bf3.l4_port2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf1.port_vp2;
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf1.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf1.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf1.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf1.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf1.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv4 = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->src_port = entry.bf0.l4_sport;
		flow_entry->dst_port = entry.bf0.l4_dport_0 |\
					entry.bf0.l4_dport_1 << 4;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 11;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
	} else if ((type & FAL_FLOW_IP6_5TUPLE_ADDR) == FAL_FLOW_IP6_5TUPLE_ADDR) {
		union in_flow_ipv6_5tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv6_5tuple_get(dev_id, get_mode, &entry_id, &entry);
		flow_entry->entry_id = entry_id;
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf0.next_hop1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf1.port_vp2;
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf1.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf1.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf1.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf1.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf1.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv6.ul[3] = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->flow_ip.ipv6.ul[2] = entry.bf0.ip_addr_1 >> 12 |\
					   entry.bf0.ip_addr_2 << 20;
		flow_entry->flow_ip.ipv6.ul[1] = entry.bf0.ip_addr_2 >> 12 |\
					   entry.bf0.ip_addr_3 << 20;
		flow_entry->flow_ip.ipv6.ul[0] = entry.bf0.ip_addr_3 >> 12 |\
					   entry.bf0.ip_addr_4 << 20;
		flow_entry->src_port = entry.bf0.l4_sport;
		flow_entry->dst_port = entry.bf0.l4_dport_0 |\
					entry.bf0.l4_dport_1 << 4;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 11;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
	} else if ((type & FAL_FLOW_IP4_3TUPLE_ADDR) == FAL_FLOW_IP4_3TUPLE_ADDR) {
		union in_flow_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv4_3tuple_get(dev_id, get_mode, &entry_id, &entry);
		flow_entry->entry_id = entry_id;
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf0.next_hop1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf1.port_vp2;
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf1.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf1.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf1.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf1.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf1.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv4 = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->ip_type = entry.bf0.ip_protocol;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 3;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
	} else if ((type & FAL_FLOW_IP6_3TUPLE_ADDR) == FAL_FLOW_IP6_3TUPLE_ADDR) {
		union in_flow_ipv6_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv6_3tuple_get(dev_id, get_mode, &entry_id, &entry);
		flow_entry->entry_id = entry_id;
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf1.next_hop1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf0.port_vp2;
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf0.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf0.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf0.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf0.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf0.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv6.ul[3] = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->flow_ip.ipv6.ul[2] = entry.bf0.ip_addr_1 >> 12 |\
					   entry.bf0.ip_addr_2 << 20;
		flow_entry->flow_ip.ipv6.ul[1] = entry.bf0.ip_addr_2 >> 12 |\
					   entry.bf0.ip_addr_3 << 20;
		flow_entry->flow_ip.ipv6.ul[0] = entry.bf0.ip_addr_3 >> 12 |\
					   entry.bf0.ip_addr_4 << 20;
		flow_entry->ip_type = entry.bf0.ip_protocol;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 11;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
	} else
		return SW_FAIL;

	if (rv == SW_OK) {
		union eg_flow_tree_map_tbl_u eg_treemap;
		union in_flow_cnt_tbl_u cnt;
		fal_flow_qos_t *flow_qos = &(flow_entry->flow_qos);
		aos_mem_zero(&eg_treemap, sizeof(eg_treemap));
		aos_mem_zero(&cnt, sizeof(cnt));

		rv = hppe_eg_flow_tree_map_tbl_get(dev_id, flow_entry->entry_id, &eg_treemap);
		flow_qos->tree_id = eg_treemap.bf.tree_id;
#if defined(APPE)
		flow_qos->wifi_qos_en = eg_treemap.bf.wifi_qos_flag;
		flow_qos->wifi_qos = eg_treemap.bf.wifi_qos;
#endif
#if defined(MPPE)
		mppe_qos_mapping_tbl_flow_policer_get(dev_id, flow_entry->entry_id,
				&(flow_entry->policer_valid), &(flow_entry->policer_index));

		flow_qos->qos_type = eg_treemap.bf.type;
		/* flow cookies only has 16bits */
		if (flow_qos->qos_type == FAL_FLOW_QOS_TYPE_COOKIE)
			flow_qos->tree_id &= 0xffff;
#endif
		rv = hppe_in_flow_cnt_tbl_get(dev_id, flow_entry->entry_id, &cnt);
		flow_entry->pkt_counter = cnt.bf.hit_pkt_counter;
		flow_entry->byte_counter = cnt.bf.hit_byte_counter_0 | \
					((a_uint64_t)cnt.bf.hit_byte_counter_1 << 32);
	}
	return rv;
}

sw_error_t
adpt_hppe_flow_entry_host_op_del(
		a_uint32_t dev_id,
		a_uint32_t del_mode,
		fal_flow_entry_t *flow_entry)
{
	sw_error_t rv = SW_OK;
	a_uint32_t type = 0;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_entry);

	if (del_mode == FAL_FLOW_OP_MODE_FLUSH)
		return hppe_flow_host_flush_common(dev_id);
		//return hppe_flow_flush_common(dev_id);

	type = flow_entry->entry_type;
	if ((type & FAL_FLOW_IP4_5TUPLE_ADDR) == FAL_FLOW_IP4_5TUPLE_ADDR) {
		union in_flow_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
			entry.bf0.l4_port1 = flow_entry->snat_srcport;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
			entry.bf3.l4_port2 = flow_entry->dnat_dstport;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv4_5tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_5TUPLE_ADDR) == FAL_FLOW_IP6_5TUPLE_ADDR) {
		union in_flow_ipv6_5tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv6_5tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP4_3TUPLE_ADDR) == FAL_FLOW_IP4_3TUPLE_ADDR) {
		union in_flow_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 3;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv4_3tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_3TUPLE_ADDR) == FAL_FLOW_IP6_3TUPLE_ADDR) {
		union in_flow_ipv6_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf1.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf0.port_vp2 = flow_entry->bridge_port;
#if defined(APPE)
			entry.bf0.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf0.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf0.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf0.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf0.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_entry_host_op_ipv6_3tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else
		return SW_FAIL;
	return rv;
}

sw_error_t
adpt_hppe_flow_host_add(
		a_uint32_t dev_id,
		a_uint32_t add_mode,
		fal_flow_host_entry_t *flow_host)
{
	sw_error_t rv = SW_OK;
	fal_flow_entry_t *flow_entry = &(flow_host->flow_entry);
	fal_host_entry_t *host_entry = &(flow_host->host_entry);
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_host);

	rv = adpt_hppe_ip_flow_host_data_add(dev_id, host_entry);
	SW_RTN_ON_ERROR(rv);

	rv = adpt_hppe_flow_entry_host_op_add(dev_id, add_mode, flow_entry);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_flow_host_tbl_op_rslt_host_entry_index_get(dev_id, &host_entry->entry_id);
	return rv;
}

sw_error_t
adpt_hppe_flow_host_get(
		a_uint32_t dev_id,
		a_uint32_t get_mode,
		fal_flow_host_entry_t *flow_host)
{
	sw_error_t rv = SW_OK;
	fal_flow_entry_t *flow_entry = &(flow_host->flow_entry);
	fal_host_entry_t *host_entry = &(flow_host->host_entry);
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_host);

	rv = adpt_hppe_ip_flow_host_data_rd_add(dev_id, host_entry);
	rv = adpt_hppe_flow_entry_host_op_get(dev_id, get_mode, flow_entry);
	if(rv != SW_OK)
		return rv;

	rv = hppe_flow_host_tbl_rd_op_rslt_host_entry_index_get(dev_id, &host_entry->entry_id);
	if(rv != SW_OK)
		return rv;

	rv = adpt_hppe_ip_flow_host_data_get(dev_id, get_mode, host_entry);

	return rv;
}

sw_error_t
adpt_hppe_flow_host_del(
		a_uint32_t dev_id,
		a_uint32_t del_mode,
		fal_flow_host_entry_t *flow_host)
{
	fal_flow_entry_t *flow_entry = &(flow_host->flow_entry);
	fal_host_entry_t *host_entry = &(flow_host->host_entry);
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_host);

	adpt_hppe_ip_flow_host_data_del(dev_id, del_mode, host_entry);
	adpt_hppe_flow_entry_host_op_del(dev_id, del_mode, flow_entry);
	return SW_OK;
}


sw_error_t
adpt_hppe_flow_entry_get(
		a_uint32_t dev_id,
		a_uint32_t get_mode,
		fal_flow_entry_t *flow_entry)
{
	sw_error_t rv = SW_OK;
	a_uint32_t type = 0;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_entry);


	type = flow_entry->entry_type;
	if ((type & FAL_FLOW_IP4_5TUPLE_ADDR) == FAL_FLOW_IP4_5TUPLE_ADDR) {
		union in_flow_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv4_5tuple_get(dev_id, get_mode, &flow_entry->entry_id, &entry);
		if (entry.bf0.entry_type != FLOW_ENTRY_TYPE_IPV4 ||
				entry.bf0.protocol_type == FLOW_TUPLE_TYPE_3) {
			return SW_BAD_VALUE;
		}
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf0.next_hop1;
			flow_entry->snat_srcport = entry.bf0.l4_port1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
			flow_entry->dnat_dstport = entry.bf3.l4_port2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
			if (entry.bf2.port_vp1 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->route_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf2.port_vp1);
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf1.port_vp2;
			if (entry.bf1.port_vp2 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->bridge_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf1.port_vp2);
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf1.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf1.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf1.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf1.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf1.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv4 = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->src_port = entry.bf0.l4_sport;
		flow_entry->dst_port = entry.bf0.l4_dport_0 |\
					entry.bf0.l4_dport_1 << 4;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 11;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
	} else if ((type & FAL_FLOW_IP6_5TUPLE_ADDR) == FAL_FLOW_IP6_5TUPLE_ADDR) {
		union in_flow_ipv6_5tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv6_5tuple_get(dev_id, get_mode, &flow_entry->entry_id, &entry);
		if (entry.bf0.entry_type != FLOW_ENTRY_TYPE_IPV6 ||
				entry.bf0.protocol_type == FLOW_TUPLE_TYPE_3) {
			return SW_BAD_VALUE;
		}
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf0.next_hop1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
			if (entry.bf2.port_vp1 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->route_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf2.port_vp1);
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf1.port_vp2;
			if (entry.bf1.port_vp2 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->bridge_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf1.port_vp2);
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf1.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf1.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf1.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf1.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf1.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv6.ul[3] = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->flow_ip.ipv6.ul[2] = entry.bf0.ip_addr_1 >> 12 |\
					   entry.bf0.ip_addr_2 << 20;
		flow_entry->flow_ip.ipv6.ul[1] = entry.bf0.ip_addr_2 >> 12 |\
					   entry.bf0.ip_addr_3 << 20;
		flow_entry->flow_ip.ipv6.ul[0] = entry.bf0.ip_addr_3 >> 12 |\
					   entry.bf0.ip_addr_4 << 20;
		flow_entry->src_port = entry.bf0.l4_sport;
		flow_entry->dst_port = entry.bf0.l4_dport_0 |\
					entry.bf0.l4_dport_1 << 4;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 11;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
		
	} else if ((type & FAL_FLOW_IP4_3TUPLE_ADDR) == FAL_FLOW_IP4_3TUPLE_ADDR) {
		union in_flow_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv4_3tuple_get(dev_id, get_mode, &flow_entry->entry_id, &entry);
		if (entry.bf0.entry_type != FLOW_ENTRY_TYPE_IPV4 ||
				entry.bf0.protocol_type != FLOW_TUPLE_TYPE_3) {
			return SW_BAD_VALUE;
		}
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf0.next_hop1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
			if (entry.bf2.port_vp1 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->route_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf2.port_vp1);
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf1.port_vp2;
			if (entry.bf1.port_vp2 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->bridge_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf1.port_vp2);
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf1.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf1.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf1.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf1.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf1.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv4 = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->ip_type = entry.bf0.ip_protocol;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 3;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
	} else if ((type & FAL_FLOW_IP6_3TUPLE_ADDR) == FAL_FLOW_IP6_3TUPLE_ADDR) {
		union in_flow_ipv6_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv6_3tuple_get(dev_id, get_mode, &flow_entry->entry_id, &entry);
		if (entry.bf0.entry_type != FLOW_ENTRY_TYPE_IPV6 ||
				entry.bf0.protocol_type != FLOW_TUPLE_TYPE_3) {
			return SW_BAD_VALUE;
		}
		flow_entry->host_addr_type = entry.bf0.host_addr_index_type;
		flow_entry->host_addr_index = entry.bf0.host_addr_index;
		flow_entry->protocol = entry.bf0.protocol_type;
		flow_entry->age = entry.bf0.age;
		flow_entry->src_intf_valid = entry.bf0.src_l3_if_valid;
		flow_entry->src_intf_index = entry.bf0.src_l3_if;
		flow_entry->fwd_type = entry.bf0.fwd_type;
		flow_entry->invalid = !entry.bf0.valid;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			flow_entry->snat_nexthop = entry.bf1.next_hop1;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			flow_entry->dnat_nexthop = entry.bf3.next_hop2;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			flow_entry->route_nexthop = entry.bf2.next_hop3;
			flow_entry->port_valid = entry.bf2.port_vp_valid1;
			flow_entry->route_port = entry.bf2.port_vp1;
			if (entry.bf2.port_vp1 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->route_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf2.port_vp1);
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			flow_entry->bridge_port = entry.bf0.port_vp2;
			if (entry.bf0.port_vp2 >= SSDK_MIN_VIRTUAL_PORT_ID)
				flow_entry->bridge_port =
					FAL_PORT_ID(FAL_PORT_TYPE_VPORT, entry.bf0.port_vp2);
#if defined(APPE)
			flow_entry->vlan_fmt_valid = entry.bf0.vlan_fmt_valid;
			flow_entry->svlan_fmt = entry.bf0.svlan_fmt;
			flow_entry->cvlan_fmt = entry.bf0.cvlan_fmt;
#endif
#if defined(MPPE)
			flow_entry->bridge_nexthop_valid = entry.bf0.next_hop4_valid;
			flow_entry->bridge_nexthop = entry.bf0.next_hop4;
#endif
		}
		flow_entry->deacclr_en = entry.bf0.de_acce;
		flow_entry->copy_tocpu_en = entry.bf0.copy_to_cpu_en;
		flow_entry->syn_toggle = entry.bf0.syn_toggle;
		flow_entry->pri_profile = entry.bf0.pri_profile_0 |\
								entry.bf0.pri_profile_1 << 1;
		flow_entry->sevice_code = entry.bf0.service_code;
		flow_entry->flow_ip.ipv6.ul[3] = entry.bf0.ip_addr_0 |\
					   entry.bf0.ip_addr_1 << 20;
		flow_entry->flow_ip.ipv6.ul[2] = entry.bf0.ip_addr_1 >> 12 |\
					   entry.bf0.ip_addr_2 << 20;
		flow_entry->flow_ip.ipv6.ul[1] = entry.bf0.ip_addr_2 >> 12 |\
					   entry.bf0.ip_addr_3 << 20;
		flow_entry->flow_ip.ipv6.ul[0] = entry.bf0.ip_addr_3 >> 12 |\
					   entry.bf0.ip_addr_4 << 20;
		flow_entry->ip_type = entry.bf0.ip_protocol;
#if defined(APPE)
		flow_entry->pmtu = entry.bf0.pmtu_0 | entry.bf0.pmtu_1 << 11;
		flow_entry->pmtu_check_l3 = entry.bf0.pmtu_check_type;
		flow_entry->vpn_id = entry.bf0.vpn_id;
#endif
	} else
		return SW_FAIL;

	if (rv == SW_OK) {
		union eg_flow_tree_map_tbl_u eg_treemap;
		union in_flow_cnt_tbl_u cnt;
		fal_flow_qos_t *flow_qos = &(flow_entry->flow_qos);
		aos_mem_zero(&eg_treemap, sizeof(eg_treemap));
		aos_mem_zero(&cnt, sizeof(cnt));

		rv = hppe_eg_flow_tree_map_tbl_get(dev_id, flow_entry->entry_id, &eg_treemap);
		flow_qos->tree_id = eg_treemap.bf.tree_id;
#if defined(APPE)
		flow_qos->wifi_qos_en = eg_treemap.bf.wifi_qos_flag;
		flow_qos->wifi_qos = eg_treemap.bf.wifi_qos;
#endif
#if defined(MPPE)
		mppe_qos_mapping_tbl_flow_policer_get(dev_id, flow_entry->entry_id,
				&(flow_entry->policer_valid), &(flow_entry->policer_index));

		flow_qos->qos_type = eg_treemap.bf.type;
		/* flow cookies only has 16bits */
		if (flow_qos->qos_type == FAL_FLOW_QOS_TYPE_COOKIE)
			flow_qos->tree_id &= 0xffff;
#endif
		rv = hppe_in_flow_cnt_tbl_get(dev_id, flow_entry->entry_id, &cnt);
		flow_entry->pkt_counter = cnt.bf.hit_pkt_counter;
		flow_entry->byte_counter = cnt.bf.hit_byte_counter_0 | \
					((a_uint64_t)cnt.bf.hit_byte_counter_1 << 32);
	}
	return rv;
}

sw_error_t
adpt_hppe_flow_entry_next(
		a_uint32_t dev_id,
		a_uint32_t next_mode,
		fal_flow_entry_t *flow_entry)
{
	a_uint32_t i = 0, step = 0;
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_entry);

	if (FAL_NEXT_ENTRY_FIRST_ID == flow_entry->entry_id)
		i = 0;

	if (next_mode == FAL_FLOW_IP4_3TUPLE_ADDR ||
		next_mode == FAL_FLOW_IP4_5TUPLE_ADDR) {
		if (FAL_NEXT_ENTRY_FIRST_ID != flow_entry->entry_id)
			i = flow_entry->entry_id + 1;
		step = 1;
	} else if (next_mode == FAL_FLOW_IP6_5TUPLE_ADDR ||
		 next_mode == FAL_FLOW_IP6_3TUPLE_ADDR) {
		if (FAL_NEXT_ENTRY_FIRST_ID != flow_entry->entry_id)
			i = (flow_entry->entry_id & ~1) + 2;
		step = 2;
	}
	for (; i < IN_FLOW_TBL_MAX_ENTRY;) {
		flow_entry->entry_type = next_mode;
		flow_entry->entry_id = i;
		rv = adpt_hppe_flow_entry_get(dev_id, 1, flow_entry);
		if (!rv) {
			return rv;
		}
		i += step;
	}

	return SW_FAIL;

}

sw_error_t
adpt_hppe_flow_entry_del(
		a_uint32_t dev_id,
		a_uint32_t del_mode,
		fal_flow_entry_t *flow_entry)
{
	sw_error_t rv = SW_OK;
	a_uint32_t type = 0;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_entry);

	if (del_mode == FAL_FLOW_OP_MODE_FLUSH)
		return hppe_flow_flush_common(dev_id);

	type = flow_entry->entry_type;
	if ((type & FAL_FLOW_IP4_5TUPLE_ADDR) == FAL_FLOW_IP4_5TUPLE_ADDR) {
		union in_flow_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
			entry.bf0.l4_port1 = flow_entry->snat_srcport;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
			entry.bf3.l4_port2 = flow_entry->dnat_dstport;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv4_5tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_5TUPLE_ADDR) == FAL_FLOW_IP6_5TUPLE_ADDR) {
		union in_flow_ipv6_5tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv6_5tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP4_3TUPLE_ADDR) == FAL_FLOW_IP4_3TUPLE_ADDR) {
		union in_flow_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 3;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv4_3tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_3TUPLE_ADDR) == FAL_FLOW_IP6_3TUPLE_ADDR) {
		union in_flow_ipv6_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf1.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf0.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf0.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf0.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf0.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf0.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf0.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv6_3tuple_del(dev_id, del_mode, &flow_entry->entry_id, &entry);
	} else
		return SW_FAIL;
	return rv;
}
sw_error_t
adpt_hppe_flow_status_get(a_uint32_t dev_id, a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl0_u flow_ctrl0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

	memset(&flow_ctrl0, 0, sizeof(flow_ctrl0));

	rv = hppe_flow_ctrl0_get(dev_id, &flow_ctrl0);
	if( rv != SW_OK )
		return rv;

	*enable = flow_ctrl0.bf.flow_en;
	return SW_OK;
}

sw_error_t
adpt_hppe_flow_ctrl_set(
		a_uint32_t dev_id,
		fal_flow_pkt_type_t type,
		fal_flow_direction_t dir,
		fal_flow_mgmt_t *ctrl)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl1_u flow_ctrl1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	memset(&flow_ctrl1, 0, sizeof(flow_ctrl1));
	rv = hppe_flow_ctrl1_get(dev_id, type, &flow_ctrl1);
	if( rv != SW_OK )
		return rv;

	if (dir == FAL_FLOW_LAN_TO_LAN_DIR) {
		flow_ctrl1.bf.flow_ctl0_miss_action = ctrl->miss_action;
		flow_ctrl1.bf.flow_ctl0_frag_bypass = ctrl->frag_bypass_en;
		flow_ctrl1.bf.flow_ctl0_tcp_special = ctrl->tcp_spec_bypass_en;
		flow_ctrl1.bf.flow_ctl0_bypass = ctrl->all_bypass_en;
		flow_ctrl1.bf.flow_ctl0_key_sel = ctrl->key_sel;
	} else if (dir == FAL_FLOW_LAN_TO_WAN_DIR) {
		flow_ctrl1.bf.flow_ctl1_miss_action = ctrl->miss_action;
		flow_ctrl1.bf.flow_ctl1_frag_bypass = ctrl->frag_bypass_en;
		flow_ctrl1.bf.flow_ctl1_tcp_special = ctrl->tcp_spec_bypass_en;
		flow_ctrl1.bf.flow_ctl1_bypass = ctrl->all_bypass_en;
		flow_ctrl1.bf.flow_ctl1_key_sel = ctrl->key_sel;
	} else if (dir == FAL_FLOW_WAN_TO_LAN_DIR) {
		flow_ctrl1.bf.flow_ctl2_miss_action = ctrl->miss_action;
		flow_ctrl1.bf.flow_ctl2_frag_bypass = ctrl->frag_bypass_en;
		flow_ctrl1.bf.flow_ctl2_tcp_special = ctrl->tcp_spec_bypass_en;
		flow_ctrl1.bf.flow_ctl2_bypass = ctrl->all_bypass_en;
		flow_ctrl1.bf.flow_ctl2_key_sel = ctrl->key_sel;
	} else if (dir == FAL_FLOW_WAN_TO_WAN_DIR) {
		flow_ctrl1.bf.flow_ctl3_miss_action = ctrl->miss_action;
		flow_ctrl1.bf.flow_ctl3_frag_bypass = ctrl->frag_bypass_en;
		flow_ctrl1.bf.flow_ctl3_tcp_special = ctrl->tcp_spec_bypass_en;
		flow_ctrl1.bf.flow_ctl3_bypass = ctrl->all_bypass_en;
		flow_ctrl1.bf.flow_ctl3_key_sel = ctrl->key_sel;
	} else if (dir == FAL_FLOW_UNKOWN_DIR_DIR) {
		flow_ctrl1.bf.flow_ctl4_miss_action = ctrl->miss_action;
		flow_ctrl1.bf.flow_ctl4_frag_bypass = ctrl->frag_bypass_en;
		flow_ctrl1.bf.flow_ctl4_tcp_special = ctrl->tcp_spec_bypass_en;
		flow_ctrl1.bf.flow_ctl4_bypass = ctrl->all_bypass_en;
		flow_ctrl1.bf.flow_ctl4_key_sel = ctrl->key_sel;
	} else
		return SW_FAIL;

	return hppe_flow_ctrl1_set(dev_id, type, &flow_ctrl1);;
}

#if !defined(IN_FLOW_MINI)
sw_error_t
adpt_hppe_flow_age_timer_get(a_uint32_t dev_id, fal_flow_age_timer_t *age_timer)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl0_u flow_ctrl0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(age_timer);

	memset(&flow_ctrl0, 0, sizeof(flow_ctrl0));

	rv = hppe_flow_ctrl0_get(dev_id, &flow_ctrl0);
	if( rv != SW_OK )
		return rv;

	age_timer->age_time = flow_ctrl0.bf.flow_age_timer;
	age_timer->unit = flow_ctrl0.bf.flow_age_timer_unit;
	return SW_OK;
}
#endif

sw_error_t
adpt_hppe_flow_status_set(a_uint32_t dev_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl0_u flow_ctrl0;

	ADPT_DEV_ID_CHECK(dev_id);

	memset(&flow_ctrl0, 0, sizeof(flow_ctrl0));

	rv = hppe_flow_ctrl0_get(dev_id, &flow_ctrl0);
	if( rv != SW_OK )
		return rv;

	flow_ctrl0.bf.flow_en = enable;
	return hppe_flow_ctrl0_set(dev_id, &flow_ctrl0);
}

sw_error_t
adpt_hppe_flow_ctrl_get(
		a_uint32_t dev_id,
		fal_flow_pkt_type_t type,
		fal_flow_direction_t dir,
		fal_flow_mgmt_t *ctrl)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl1_u flow_ctrl1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	memset(&flow_ctrl1, 0, sizeof(flow_ctrl1));
	rv = hppe_flow_ctrl1_get(dev_id, type, &flow_ctrl1);
	if( rv != SW_OK )
		return rv;

	if (dir == FAL_FLOW_LAN_TO_LAN_DIR) {
		ctrl->miss_action = flow_ctrl1.bf.flow_ctl0_miss_action;
		ctrl->frag_bypass_en = flow_ctrl1.bf.flow_ctl0_frag_bypass;
		ctrl->tcp_spec_bypass_en = flow_ctrl1.bf.flow_ctl0_tcp_special;
		ctrl->all_bypass_en = flow_ctrl1.bf.flow_ctl0_bypass;
		ctrl->key_sel = flow_ctrl1.bf.flow_ctl0_key_sel;
	} else if (dir == FAL_FLOW_LAN_TO_WAN_DIR) {
		ctrl->miss_action = flow_ctrl1.bf.flow_ctl1_miss_action;
		ctrl->frag_bypass_en = flow_ctrl1.bf.flow_ctl1_frag_bypass;
		ctrl->tcp_spec_bypass_en = flow_ctrl1.bf.flow_ctl1_tcp_special;
		ctrl->all_bypass_en = flow_ctrl1.bf.flow_ctl1_bypass;
		ctrl->key_sel = flow_ctrl1.bf.flow_ctl1_key_sel;
	} else if (dir == FAL_FLOW_WAN_TO_LAN_DIR) {
		ctrl->miss_action = flow_ctrl1.bf.flow_ctl2_miss_action;
		ctrl->frag_bypass_en = flow_ctrl1.bf.flow_ctl2_frag_bypass;
		ctrl->tcp_spec_bypass_en = flow_ctrl1.bf.flow_ctl2_tcp_special;
		ctrl->all_bypass_en = flow_ctrl1.bf.flow_ctl2_bypass;
		ctrl->key_sel = flow_ctrl1.bf.flow_ctl2_key_sel;
	} else if (dir == FAL_FLOW_WAN_TO_WAN_DIR) {
		ctrl->miss_action = flow_ctrl1.bf.flow_ctl3_miss_action;
		ctrl->frag_bypass_en = flow_ctrl1.bf.flow_ctl3_frag_bypass;
		ctrl->tcp_spec_bypass_en = flow_ctrl1.bf.flow_ctl3_tcp_special;
		ctrl->all_bypass_en = flow_ctrl1.bf.flow_ctl3_bypass;
		ctrl->key_sel = flow_ctrl1.bf.flow_ctl3_key_sel;
	} else if (dir == FAL_FLOW_UNKOWN_DIR_DIR) {
		ctrl->miss_action = flow_ctrl1.bf.flow_ctl4_miss_action;
		ctrl->frag_bypass_en = flow_ctrl1.bf.flow_ctl4_frag_bypass;
		ctrl->tcp_spec_bypass_en = flow_ctrl1.bf.flow_ctl4_tcp_special;
		ctrl->all_bypass_en = flow_ctrl1.bf.flow_ctl4_bypass;
		ctrl->key_sel = flow_ctrl1.bf.flow_ctl4_key_sel;
	} else
		return SW_FAIL;

	return SW_OK;
}

#if !defined(IN_FLOW_MINI)
sw_error_t
adpt_hppe_flow_age_timer_set(a_uint32_t dev_id, fal_flow_age_timer_t *age_timer)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl0_u flow_ctrl0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(age_timer);

	memset(&flow_ctrl0, 0, sizeof(flow_ctrl0));

	rv = hppe_flow_ctrl0_get(dev_id, &flow_ctrl0);
	if( rv != SW_OK )
		return rv;

	flow_ctrl0.bf.flow_age_timer = age_timer->age_time;
	flow_ctrl0.bf.flow_age_timer_unit = age_timer->unit;
	return hppe_flow_ctrl0_set(dev_id, &flow_ctrl0);
}
#endif

sw_error_t
adpt_hppe_flow_entry_add(
		a_uint32_t dev_id,
		a_uint32_t add_mode, /*index or hash*/
		fal_flow_entry_t *flow_entry)
{
	sw_error_t rv = SW_OK;
	a_uint32_t type = 0;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_entry);

	type = flow_entry->entry_type;
	if ((type & FAL_FLOW_IP4_5TUPLE_ADDR) == FAL_FLOW_IP4_5TUPLE_ADDR) {
		union in_flow_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
			entry.bf0.l4_port1 = flow_entry->snat_srcport;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
			entry.bf3.l4_port2 = flow_entry->dnat_dstport;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv4_5tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_5TUPLE_ADDR) == FAL_FLOW_IP6_5TUPLE_ADDR) {
		union in_flow_ipv6_5tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.l4_sport = flow_entry->src_port;
		entry.bf0.l4_dport_0 = flow_entry->dst_port;
		entry.bf0.l4_dport_1 = flow_entry->dst_port >> 4;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv6_5tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP4_3TUPLE_ADDR) == FAL_FLOW_IP4_3TUPLE_ADDR) {
		union in_flow_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV4;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf0.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf1.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf1.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf1.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf1.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf1.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf1.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv4;
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv4 >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 3;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv4_3tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else if ((type & FAL_FLOW_IP6_3TUPLE_ADDR) == FAL_FLOW_IP6_3TUPLE_ADDR) {
		union in_flow_ipv6_3tuple_tbl_u entry;
		aos_mem_zero(&entry, sizeof(entry));

		entry.bf0.valid= flow_entry->invalid ? 0 : 1;
		entry.bf0.entry_type = FLOW_ENTRY_TYPE_IPV6;
		entry.bf0.host_addr_index_type = flow_entry->host_addr_type;
		entry.bf0.host_addr_index = flow_entry->host_addr_index;
		entry.bf0.protocol_type = flow_entry->protocol;
		entry.bf0.age = flow_entry->age;
		entry.bf0.src_l3_if_valid = flow_entry->src_intf_valid;
		entry.bf0.src_l3_if = flow_entry->src_intf_index;
		entry.bf0.fwd_type = flow_entry->fwd_type;
		if (flow_entry->fwd_type == FAL_FLOW_SNAT) {
			entry.bf1.next_hop1 = flow_entry->snat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_DNAT) {
			entry.bf3.next_hop2 = flow_entry->dnat_nexthop;
		} else if (flow_entry->fwd_type == FAL_FLOW_ROUTE) {
			entry.bf2.next_hop3 = flow_entry->route_nexthop;
			entry.bf2.port_vp_valid1= flow_entry->port_valid;
			entry.bf2.port_vp1 = flow_entry->route_port & 0xffffff;
		} else if (flow_entry->fwd_type == FAL_FLOW_BRIDGE) {
			entry.bf0.port_vp2 = flow_entry->bridge_port & 0xffffff;
#if defined(APPE)
			entry.bf0.vlan_fmt_valid = flow_entry->vlan_fmt_valid;
			entry.bf0.svlan_fmt = flow_entry->svlan_fmt;
			entry.bf0.cvlan_fmt = flow_entry->cvlan_fmt;
#endif
#if defined(MPPE)
			entry.bf0.next_hop4_valid = flow_entry->bridge_nexthop_valid;
			entry.bf0.next_hop4 = flow_entry->bridge_nexthop;
#endif
		}
		entry.bf0.de_acce = flow_entry->deacclr_en;
		entry.bf0.copy_to_cpu_en = flow_entry->copy_tocpu_en;
		entry.bf0.syn_toggle = flow_entry->syn_toggle;
		entry.bf0.pri_profile_0 = flow_entry->pri_profile;
		entry.bf0.pri_profile_1 = flow_entry->pri_profile >> 1;
		entry.bf0.service_code = flow_entry->sevice_code;
		entry.bf0.ip_addr_0 = flow_entry->flow_ip.ipv6.ul[3];
		entry.bf0.ip_addr_1 = flow_entry->flow_ip.ipv6.ul[3] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[2] << 12;
		entry.bf0.ip_addr_2 = flow_entry->flow_ip.ipv6.ul[2] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[1] << 12;
		entry.bf0.ip_addr_3 = flow_entry->flow_ip.ipv6.ul[1] >> 20 |\
							flow_entry->flow_ip.ipv6.ul[0] << 12;
		entry.bf0.ip_addr_4 = flow_entry->flow_ip.ipv6.ul[0] >> 20;
		entry.bf0.ip_protocol = flow_entry->ip_type;
#if defined(APPE)
		entry.bf0.pmtu_1 = flow_entry->pmtu >> 11;
		entry.bf0.pmtu_0 = flow_entry->pmtu & 0x7ff;
		entry.bf0.pmtu_check_type = flow_entry->pmtu_check_l3;
		entry.bf0.vpn_id = flow_entry->vpn_id;
#endif
		rv = hppe_flow_ipv6_3tuple_add(dev_id, (a_uint32_t)add_mode, &flow_entry->entry_id, &entry);
	} else
		return SW_FAIL;
	if (rv == SW_OK) {
		union eg_flow_tree_map_tbl_u eg_treemap;
		fal_flow_qos_t *flow_qos = &(flow_entry->flow_qos);
		aos_mem_zero(&eg_treemap, sizeof(eg_treemap));

		rv = hppe_eg_flow_tree_map_tbl_get(dev_id, flow_entry->entry_id, &eg_treemap);
		SW_RTN_ON_ERROR(rv);

#if defined(MPPE)
		mppe_qos_mapping_tbl_flow_policer_set(dev_id, flow_entry->entry_id,
				flow_entry->policer_valid, flow_entry->policer_index);

		eg_treemap.bf.type = flow_qos->qos_type;
		/* flow cookies only has 16bits */
		if (flow_qos->qos_type == FAL_FLOW_QOS_TYPE_COOKIE)
			flow_qos->tree_id &= 0xffff;
#endif
		eg_treemap.bf.tree_id = flow_qos->tree_id;
#if defined(APPE)
		eg_treemap.bf.wifi_qos_flag = flow_qos->wifi_qos_en;
		eg_treemap.bf.wifi_qos = flow_qos->wifi_qos;
#endif
		rv = hppe_eg_flow_tree_map_tbl_set(dev_id, flow_entry->entry_id, &eg_treemap);
	}
	return rv;
}

sw_error_t
adpt_hppe_flow_counter_get(a_uint32_t dev_id,
		a_uint32_t flow_index, fal_entry_counter_t *flow_counter)
{
	sw_error_t rv = SW_OK;
	union in_flow_cnt_tbl_u flow_cnt;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_counter);

	if (flow_index >= IN_FLOW_CNT_TBL_NUM)
		return SW_OUT_OF_RANGE;

	aos_mem_zero(&flow_cnt, sizeof(flow_cnt));

	rv = hppe_in_flow_cnt_tbl_get(dev_id, flow_index, &flow_cnt);
	SW_RTN_ON_ERROR(rv);

	flow_counter->matched_pkts = flow_cnt.bf.hit_pkt_counter;
	flow_counter->matched_bytes = flow_cnt.bf.hit_byte_counter_0 |
		(a_uint64_t)flow_cnt.bf.hit_byte_counter_1 << SW_FIELD_OFFSET_IN_WORD(
				IN_FLOW_CNT_TBL_HIT_BYTE_COUNTER_OFFSET);
	return rv;
}

sw_error_t
adpt_hppe_flow_counter_cleanup(a_uint32_t dev_id, a_uint32_t flow_index)
{
	sw_error_t rv = SW_OK;
	union in_flow_cnt_tbl_u flow_cnt;

	ADPT_DEV_ID_CHECK(dev_id);

	if (flow_index >= IN_FLOW_CNT_TBL_NUM)
		return SW_OUT_OF_RANGE;

	aos_mem_zero(&flow_cnt, sizeof(flow_cnt));

	rv = hppe_in_flow_cnt_tbl_set(dev_id, flow_index, &flow_cnt);

	return rv;
}

sw_error_t
adpt_hppe_flow_entry_en_set(a_uint32_t dev_id, a_uint32_t flow_index, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	union in_flow_tbl_u entry;

	ADPT_DEV_ID_CHECK(dev_id);

	if (flow_index >= IN_FLOW_CNT_TBL_NUM)
		return SW_OUT_OF_RANGE;

	aos_mem_zero(&entry, sizeof(entry));

	rv = hppe_in_flow_tbl_get(dev_id, flow_index, &entry);
	SW_RTN_ON_ERROR(rv);

	entry.bf0.valid = enable;

	rv = hppe_in_flow_tbl_set(dev_id, flow_index, &entry);
	return rv;
}

sw_error_t
adpt_hppe_flow_entry_en_get(a_uint32_t dev_id, a_uint32_t flow_index, a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	union in_flow_tbl_u entry;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

	if (flow_index >= IN_FLOW_CNT_TBL_NUM)
		return SW_OUT_OF_RANGE;

	aos_mem_zero(&entry, sizeof(entry));

	rv = hppe_in_flow_tbl_get(dev_id, flow_index, &entry);
	SW_RTN_ON_ERROR(rv);

	*enable = entry.bf0.valid;
	return rv;
}

sw_error_t
adpt_hppe_flow_qos_set(a_uint32_t dev_id, a_uint32_t flow_index, fal_flow_qos_t *flow_qos)
{
	sw_error_t rv = SW_OK;
	union eg_flow_tree_map_tbl_u eg_treemap;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_qos);

	if (flow_index >= EG_FLOW_TREE_MAP_TBL_NUM)
		return SW_OUT_OF_RANGE;

	aos_mem_zero(&eg_treemap, sizeof(eg_treemap));

	rv = hppe_eg_flow_tree_map_tbl_get(dev_id, flow_index, &eg_treemap);
	SW_RTN_ON_ERROR(rv);

#if defined(MPPE)
	eg_treemap.bf.type = flow_qos->qos_type;
	/* flow cookies only has 16bits */
	if (flow_qos->qos_type == FAL_FLOW_QOS_TYPE_COOKIE)
		flow_qos->tree_id &= 0xffff;
#endif
	eg_treemap.bf.tree_id = flow_qos->tree_id;
#if defined(APPE)
	eg_treemap.bf.wifi_qos_flag = flow_qos->wifi_qos_en;
	eg_treemap.bf.wifi_qos = flow_qos->wifi_qos;
#endif

	rv = hppe_eg_flow_tree_map_tbl_set(dev_id, flow_index, &eg_treemap);
	return rv;
}

sw_error_t
adpt_hppe_flow_qos_get(a_uint32_t dev_id, a_uint32_t flow_index, fal_flow_qos_t *flow_qos)
{
	sw_error_t rv = SW_OK;
	union eg_flow_tree_map_tbl_u eg_treemap;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_qos);

	if (flow_index >= EG_FLOW_TREE_MAP_TBL_NUM)
		return SW_OUT_OF_RANGE;

	aos_mem_zero(&eg_treemap, sizeof(eg_treemap));

	rv = hppe_eg_flow_tree_map_tbl_get(dev_id, flow_index, &eg_treemap);
	SW_RTN_ON_ERROR(rv);

	flow_qos->tree_id = eg_treemap.bf.tree_id;
#if defined(APPE)
	flow_qos->wifi_qos_en = eg_treemap.bf.wifi_qos_flag;
	flow_qos->wifi_qos = eg_treemap.bf.wifi_qos;
#endif
#if defined(MPPE)
	flow_qos->qos_type = eg_treemap.bf.type;
	/* flow cookies only has 16bits */
	if (flow_qos->qos_type == FAL_FLOW_QOS_TYPE_COOKIE)
		flow_qos->tree_id &= 0xffff;
#endif

	return rv;
}

sw_error_t
adpt_hppe_flow_global_cfg_get(
		a_uint32_t dev_id,
		fal_flow_global_cfg_t *cfg)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl0_u flow_ctrl0;
	union l3_route_ctrl_u route_ctrl;
	union l3_route_ctrl_ext_u route_ctrl_ext;
	adpt_ppe_type_t ppe_type = MAX_PPE_TYPE;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);
	
	memset(&flow_ctrl0, 0, sizeof(flow_ctrl0));
	memset(&route_ctrl, 0, sizeof(route_ctrl));
	memset(&route_ctrl_ext, 0, sizeof(route_ctrl_ext));

	ppe_type = adpt_ppe_type_get(dev_id);

#if defined(MPPE)
	if (ppe_type == MPPE_TYPE) {
		union eg_gen_ctrl_u eg_ctrl;

		memset(&eg_ctrl, 0, sizeof(eg_ctrl));
		rv = mppe_eg_gen_ctrl_get(dev_id, &eg_ctrl);
		SW_RTN_ON_ERROR(rv);

		cfg->flow_cookie_pri = eg_ctrl.bf.flow_cookie_pri;
	}
#endif

	rv = hppe_flow_ctrl0_get(dev_id, &flow_ctrl0);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_l3_route_ctrl_get(dev_id, &route_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_l3_route_ctrl_ext_get(dev_id, &route_ctrl_ext);
	SW_RTN_ON_ERROR(rv);

#if defined(CPPE) || defined(APPE)
	if (ppe_type != MAX_PPE_TYPE && ppe_type != HPPE_TYPE) {
		a_bool_t flow_cpy_escape = A_FALSE;

		rv = adpt_cppe_flow_copy_escape_get(dev_id, &flow_cpy_escape);
		SW_RTN_ON_ERROR(rv);

		cfg->flow_mismatch_copy_escape_en = flow_cpy_escape;
	}
#endif

	cfg->src_if_check_action= route_ctrl.bf.flow_src_if_check_cmd;
	cfg->src_if_check_deacclr_en= route_ctrl.bf.flow_src_if_check_de_acce;
	cfg->service_loop_en = route_ctrl_ext.bf.flow_service_code_loop_en;
#if defined(APPE)
	cfg->ptmu_fail_action = route_ctrl_ext.bf.flow_pmtu_fail;
	cfg->ptmu_fail_deacclr_en = route_ctrl_ext.bf.flow_pmtu_fail_de_acce;
	cfg->ptmu_fail_df_action = route_ctrl_ext.bf.flow_pmtu_df_fail;
	cfg->ptmu_fail_df_deacclr_en = route_ctrl_ext.bf.flow_pmtu_df_fail_de_acce;
	cfg->l2_vpn_en = route_ctrl_ext.bf.l2_vpn_en;
	cfg->l3_vpn_en = route_ctrl_ext.bf.l3_vpn_en;
#endif
	cfg->service_loop_action = route_ctrl.bf.flow_service_code_loop;
	cfg->service_loop_deacclr_en = route_ctrl.bf.flow_service_code_loop_de_acce;
	cfg->flow_deacclr_action = route_ctrl.bf.flow_de_acce_cmd;
	cfg->sync_mismatch_action = route_ctrl.bf.flow_sync_mismatch_cmd;
	cfg->sync_mismatch_deacclr_en = route_ctrl.bf.flow_sync_mismatch_de_acce;
	cfg->hash_mode_0 = flow_ctrl0.bf.flow_hash_mode_0;
	cfg->hash_mode_1 = flow_ctrl0.bf.flow_hash_mode_1;

	return SW_OK;
}

sw_error_t
adpt_hppe_flow_global_cfg_set(
		a_uint32_t dev_id,
		fal_flow_global_cfg_t *cfg)
{
	sw_error_t rv = SW_OK;
	union flow_ctrl0_u flow_ctrl0;
	union l3_route_ctrl_u route_ctrl;
	union l3_route_ctrl_ext_u route_ctrl_ext;
	adpt_ppe_type_t ppe_type = MAX_PPE_TYPE;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);
	
	memset(&flow_ctrl0, 0, sizeof(flow_ctrl0));
	memset(&route_ctrl, 0, sizeof(route_ctrl));
	memset(&route_ctrl_ext, 0, sizeof(route_ctrl_ext));

	ppe_type = adpt_ppe_type_get(dev_id);

	rv = hppe_flow_ctrl0_get(dev_id, &flow_ctrl0);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_l3_route_ctrl_get(dev_id, &route_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_l3_route_ctrl_ext_get(dev_id, &route_ctrl_ext);
	SW_RTN_ON_ERROR(rv);

	route_ctrl.bf.flow_src_if_check_cmd = cfg->src_if_check_action;
	route_ctrl.bf.flow_src_if_check_de_acce = cfg->src_if_check_deacclr_en;
	route_ctrl_ext.bf.flow_service_code_loop_en = cfg->service_loop_en;
#if defined(APPE)
	route_ctrl_ext.bf.flow_pmtu_fail = cfg->ptmu_fail_action;
	route_ctrl_ext.bf.flow_pmtu_fail_de_acce = cfg->ptmu_fail_deacclr_en;
	route_ctrl_ext.bf.flow_pmtu_df_fail = cfg->ptmu_fail_df_action;
	route_ctrl_ext.bf.flow_pmtu_df_fail_de_acce = cfg->ptmu_fail_df_deacclr_en;
	route_ctrl_ext.bf.l2_vpn_en = cfg->l2_vpn_en;
	route_ctrl_ext.bf.l3_vpn_en = cfg->l3_vpn_en;
#endif
	route_ctrl.bf.flow_service_code_loop = cfg->service_loop_action;
	route_ctrl.bf.flow_service_code_loop_de_acce = cfg->service_loop_deacclr_en;
	route_ctrl.bf.flow_de_acce_cmd = cfg->flow_deacclr_action;
	route_ctrl.bf.flow_sync_mismatch_cmd = cfg->sync_mismatch_action;
	route_ctrl.bf.flow_sync_mismatch_de_acce = cfg->sync_mismatch_deacclr_en;
	flow_ctrl0.bf.flow_hash_mode_0 = cfg->hash_mode_0;
	flow_ctrl0.bf.flow_hash_mode_1 = cfg->hash_mode_1;

	rv = hppe_flow_ctrl0_set(dev_id, &flow_ctrl0);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_l3_route_ctrl_set(dev_id, &route_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_l3_route_ctrl_ext_set(dev_id, &route_ctrl_ext);
	SW_RTN_ON_ERROR(rv);

#if defined(CPPE) || defined(APPE)
	if (ppe_type != MAX_PPE_TYPE && ppe_type != HPPE_TYPE) {
		rv = adpt_cppe_flow_copy_escape_set(dev_id,
				cfg->flow_mismatch_copy_escape_en);
		SW_RTN_ON_ERROR(rv);
	}
#endif

#if defined(MPPE)
	if (ppe_type == MPPE_TYPE) {
		union eg_gen_ctrl_u eg_ctrl;
		memset(&eg_ctrl, 0, sizeof(eg_ctrl));

		rv = mppe_eg_gen_ctrl_get(dev_id, &eg_ctrl);
		SW_RTN_ON_ERROR(rv);

		eg_ctrl.bf.flow_cookie_pri = cfg->flow_cookie_pri;

		rv = mppe_eg_gen_ctrl_set(dev_id, &eg_ctrl);
		SW_RTN_ON_ERROR(rv);
	}
#endif

	return SW_OK;
}

/* clear ip6 address low bits */
fal_ip6_addr_t
ip6_clear_low_bits(fal_ip6_addr_t ip6, a_uint32_t len) 
{
	a_uint8_t i = 0;
	a_uint8_t ints_to_clear = len / 32;
	a_uint8_t bits_to_clear = len % 32;
	a_uint32_t *addr = ip6.ul;

	for (i = 0; i < ints_to_clear; i++) {
		addr[3 - i] = 0;
	}

	if (bits_to_clear > 0) {
	    addr[3 - ints_to_clear] &= ~((1U << bits_to_clear) - 1);
	}

	return ip6;
}

/* one's complement checksum of ip6 array*/
a_uint16_t
ip6_csum(fal_ip6_addr_t ip6)
{
	a_uint32_t *addr = ip6.ul;
	a_uint32_t sum = 0;
	a_uint8_t i;

	for (i = 0; i < 4; i++) {
		sum += (addr[i] >> 16) & 0xFFFF;
		sum += addr[i] & 0xFFFF;
	}

	while (0xFFFF < sum) {
		sum += 1 - 0x10000;
	}

	return  (a_uint16_t)(~sum);
}

/* One's complement sum */
a_uint16_t
ip6_csum_add(a_uint16_t       csum1, a_uint16_t csum2)
{
	a_uint32_t result = csum1 + csum2;
	
	while (0xFFFF < result) {
		result += 1 - 0x10000;
	}

	 return (a_uint16_t)result;
}

/* One's complement difference */
a_uint16_t
ip6_csum_sub(a_uint16_t       csum1, a_uint16_t csum2)
{
	return ip6_csum_add(csum1, ~csum2);
}

/*search first non-0xffff from ip6_prefix_len*/
a_int32_t
ip6_offset_find(fal_ip6_addr_t ip6, a_uint32_t prefix_len)
{
	a_uint32_t *addr = ip6.ul, val;
	a_int32_t start;

	if(prefix_len <= 48)
		return 48;

	/*When prefix_len is in the range (49~64), start from 64*/
	start = ((prefix_len / 16) * 16) + ((prefix_len % 16)? 16:0);

	while(start < 128) {
		val = addr[start/32] >> (16 - (start%32));
		if ((val & 0xFFFF) != 0xFFFF)
			return start;
		start += 16;
	}

	return -1;
}

#define SWAP_IP6(dst, src) \
	do { \
		(dst)[3] = (src)[0]; \
		(dst)[2] = (src)[1]; \
		(dst)[1] = (src)[2]; \
		(dst)[0] = (src)[3]; \
	} while (0)

sw_error_t
adpt_hppe_flow_npt66_prefix_add(a_uint32_t dev_id, a_uint32_t l3_if_index, fal_ip6_addr_t *ip6, a_uint32_t prefix_len)
{
	union eg_ipv6_prefix_tbl_u entry = { 0 };

	SWAP_IP6(entry.bf.prefix, ip6->ul);
	entry.bf.length = prefix_len;

	hppe_eg_ipv6_prefix_tbl_set(dev_id, l3_if_index, &entry);

	SSDK_DEBUG("**********%s:%d*****[%d]*%x:%x:%x:%x %d*******\n", __func__, __LINE__, l3_if_index, 
		entry.bf.prefix[0], entry.bf.prefix[1], entry.bf.prefix[2], entry.bf.prefix[3], entry.bf.length);

	return SW_OK;
	/*
		EG_IPV6_PREFIX_TBL[255:0]
			PREFIX	ipv6_addr		128
			LENGTH					7
	*/
}

sw_error_t
adpt_hppe_flow_npt66_prefix_get(a_uint32_t dev_id, a_uint32_t l3_if_index, fal_ip6_addr_t *ip6, a_uint32_t *prefix_len)
{
	union eg_ipv6_prefix_tbl_u entry = { 0 };

	hppe_eg_ipv6_prefix_tbl_get(dev_id, l3_if_index, &entry);

	SWAP_IP6(ip6->ul, entry.bf.prefix);
	*prefix_len = entry.bf.length;

	SSDK_DEBUG("**********%s:%d*****[%d]*%x:%x:%x:%x %d*******\n", __func__, __LINE__, l3_if_index, 
		entry.bf.prefix[0], entry.bf.prefix[1], entry.bf.prefix[2], entry.bf.prefix[3], entry.bf.length);

	return SW_OK;
}

sw_error_t
adpt_hppe_flow_npt66_prefix_del(a_uint32_t dev_id, a_uint32_t l3_if_index)
{
	union eg_ipv6_prefix_tbl_u entry = { 0 };

	hppe_eg_ipv6_prefix_tbl_set(dev_id, l3_if_index, &entry);

	SSDK_DEBUG("**********%s:%d*****[%d]*%x:%x:%x:%x %d*******\n", __func__, __LINE__, l3_if_index, 
		entry.bf.prefix[0], entry.bf.prefix[1], entry.bf.prefix[2], entry.bf.prefix[3], entry.bf.length);

	return SW_OK;
}

#define IP6_MAX_LEN 128
#define EG_FLOW_IPV6_IID_OFFSET_TO_FILED_VAL(offset) ((offset)/16)
sw_error_t
adpt_hppe_flow_npt66_iid_cal(a_uint32_t dev_id,               fal_flow_npt66_iid_calc_t *iid_cal, fal_flow_npt66_iid_t *iid_result)
{
	a_uint32_t prefix_len = iid_cal->prefix_len;
	a_uint32_t tip_prefix_len = iid_cal->tip_prefix_len;
	a_uint32_t clear_len = IP6_MAX_LEN-prefix_len;
	a_uint32_t tip_clear_len = IP6_MAX_LEN-tip_prefix_len;
	fal_ip6_addr_t org_ip = (iid_cal->is_dnat)?iid_cal->dip:iid_cal->sip;
	fal_ip6_addr_t tip = iid_cal->tip;
	a_uint16_t tip_csum, org_csum, iid_org, iid_val, csum_adj;
	a_uint32_t iid_offset = ip6_offset_find(org_ip, max(prefix_len, tip_prefix_len));
	
	if(iid_offset == -1)
		return SW_BAD_PARAM;
	
	tip_csum = ip6_csum(ip6_clear_low_bits(tip,tip_clear_len));
	org_csum = ip6_csum(ip6_clear_low_bits(org_ip,clear_len));
	iid_org = (org_ip.ul[iid_offset/32]>>(16-(iid_offset % 32)))&0xFFFF;
	csum_adj = ip6_csum_sub(tip_csum, org_csum);
	iid_val = ip6_csum_add(csum_adj, iid_org);

	iid_result->iid = (a_uint32_t)((iid_val==0xFFFF)? 0 : iid_val);
	iid_result->iid_offset = EG_FLOW_IPV6_IID_OFFSET_TO_FILED_VAL(iid_offset);
	iid_result->is_dnat = iid_cal->is_dnat;

	SSDK_DEBUG("iid calc is_dnat:%d org_ip:%08x:%08x:%08x:%08x tip:%08x:%08x:%08x:%08x prefix_len:%d tip_prefix_len:%d\n"
			"==>tip_csum:%04x - org_csum:%04x + iid_org:%04x = iid:%04x (offset:%d)\n",
			iid_result->is_dnat,org_ip.ul[0], org_ip.ul[1], org_ip.ul[2], org_ip.ul[3],
			tip.ul[0], tip.ul[1], tip.ul[2], tip.ul[3],	prefix_len, tip_prefix_len,
			tip_csum, org_csum, iid_org, iid_val, iid_offset);

	return SW_OK;
}


sw_error_t
adpt_hppe_flow_npt66_iid_add(a_uint32_t dev_id,   a_uint32_t flow_index, fal_flow_npt66_iid_t *iid)
{
	union eg_flow_ipv6_iid_tbl_u entry = { 0 };

	entry.bf.iid = iid->iid;
	entry.bf.offset =  iid->iid_offset;
	entry.bf.iid_update = 1;
	entry.bf.prefix_update = 1;
	entry.bf.src_dst = iid->is_dnat;
	hppe_eg_flow_ipv6_iid_tbl_set(dev_id, flow_index, &entry);

	SSDK_DEBUG("******%s:%d*****[%d]*%x:%x:%x:%x %d*******\n", __func__, __LINE__, flow_index, 
		entry.bf.iid, entry.bf.offset, entry.bf.iid_update, entry.bf.prefix_update, entry.bf.src_dst);

	return SW_OK;

	/*
		EG_FLOW_IPV6_IID_TBL[2047:0]
			IID 	16	0	Outbound: IID = External_csum(from table) - Internal_csm(from packet) + 16Bit_IID (src address)
						 	 Inbound: IID = Internal_csm(from table) - External_csum(from packet) + 16Bit_IID (dst address)
			OFFSET	3	0  "0: IPv6_addr[0..15]
							1: IPv6_addr[16..31]
							2: IPv6_addr[32..47]
							3: IPv6_addr[48..63]
							4: IPv6_addr[64..79]
							5: IPv6_addr[80..95]
							6: IPv6_addr[96..111]
							7: IPv6_addr[112..127]"
			IID_UPDATE		1	0	1: enable update
			PREFIX_UPDATE	1	0	1: enable update
			SRC_DST 		1	0	0: source address; 1: destination address
	*/
}
sw_error_t
adpt_hppe_flow_npt66_iid_get(a_uint32_t dev_id,   a_uint32_t flow_index, fal_flow_npt66_iid_t *iid)
{
	union eg_flow_ipv6_iid_tbl_u entry = { 0 };

	hppe_eg_flow_ipv6_iid_tbl_get(dev_id, flow_index, &entry);

	memset(iid, 0, sizeof(fal_flow_npt66_iid_t));

	if(	entry.bf.iid_update&&entry.bf.prefix_update) {
		iid->iid = entry.bf.iid;
		iid->iid_offset= entry.bf.offset;
		iid->is_dnat = entry.bf.src_dst;
	} else {
		return SW_NOT_FOUND;
	}

	SSDK_DEBUG("*******%s:%d*****[%d]*%x:%x:%x:%x %d*******\n", __func__, __LINE__, flow_index, 
		entry.bf.iid, entry.bf.offset, entry.bf.iid_update, entry.bf.prefix_update, entry.bf.src_dst);

	return SW_OK;
}

sw_error_t
adpt_hppe_flow_npt66_iid_del(a_uint32_t dev_id,   a_uint32_t flow_index)
{
	union eg_flow_ipv6_iid_tbl_u entry = { 0 };
	
	hppe_eg_flow_ipv6_iid_tbl_set(dev_id, flow_index, &entry);
	SSDK_DEBUG("******%s:%d*****[%d]*%x:%x:%x:%x %d*******\n", __func__, __LINE__, flow_index, 
		entry.bf.iid, entry.bf.offset, entry.bf.iid_update, entry.bf.prefix_update, entry.bf.src_dst);

	return SW_OK;
}

sw_error_t
adpt_hppe_flow_npt66_status_set(a_uint32_t dev_id, a_bool_t enable)
{
	union eg_global_ctrl_u eg_global_ctrl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	eg_global_ctrl.bf.prefix_xlt_en = enable;

	return hppe_eg_global_ctrl_set(dev_id, &eg_global_ctrl);
}

sw_error_t
adpt_hppe_flow_npt66_status_get(a_uint32_t dev_id, a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	union eg_global_ctrl_u eg_global_ctrl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

	rv = hppe_eg_global_ctrl_get(dev_id, &eg_global_ctrl);
	if( rv != SW_OK )
		return rv;

	*enable = eg_global_ctrl.bf.prefix_xlt_en;

	return SW_OK;
}

sw_error_t adpt_hppe_flow_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_flow_host_add = adpt_hppe_flow_host_add;
	p_adpt_api->adpt_flow_entry_get = adpt_hppe_flow_entry_get;
	p_adpt_api->adpt_flow_entry_del = adpt_hppe_flow_entry_del;
	p_adpt_api->adpt_flow_status_get = adpt_hppe_flow_status_get;
#if !defined(IN_FLOW_MINI)
	p_adpt_api->adpt_flow_age_timer_get = adpt_hppe_flow_age_timer_get;
	p_adpt_api->adpt_flow_age_timer_set = adpt_hppe_flow_age_timer_set;
#endif
	p_adpt_api->adpt_flow_status_set = adpt_hppe_flow_status_set;
	p_adpt_api->adpt_flow_host_get = adpt_hppe_flow_host_get;
	p_adpt_api->adpt_flow_host_del = adpt_hppe_flow_host_del;
	p_adpt_api->adpt_flow_entry_add = adpt_hppe_flow_entry_add;
	p_adpt_api->adpt_flow_global_cfg_get = adpt_hppe_flow_global_cfg_get;
	p_adpt_api->adpt_flow_global_cfg_set = adpt_hppe_flow_global_cfg_set;
	p_adpt_api->adpt_flow_entry_next = adpt_hppe_flow_entry_next;
	p_adpt_api->adpt_flow_counter_get = adpt_hppe_flow_counter_get;
	p_adpt_api->adpt_flow_counter_cleanup = adpt_hppe_flow_counter_cleanup;
	p_adpt_api->adpt_flow_entry_en_set = adpt_hppe_flow_entry_en_set;
	p_adpt_api->adpt_flow_entry_en_get = adpt_hppe_flow_entry_en_get;
	p_adpt_api->adpt_flow_qos_set = adpt_hppe_flow_qos_set;
	p_adpt_api->adpt_flow_qos_get = adpt_hppe_flow_qos_get;
	p_adpt_api->adpt_flow_ctrl_get = adpt_hppe_flow_ctrl_get;
	p_adpt_api->adpt_flow_ctrl_set = adpt_hppe_flow_ctrl_set;
	p_adpt_api->adpt_flow_npt66_prefix_add = adpt_hppe_flow_npt66_prefix_add;
	p_adpt_api->adpt_flow_npt66_prefix_get = adpt_hppe_flow_npt66_prefix_get;
	p_adpt_api->adpt_flow_npt66_prefix_del = adpt_hppe_flow_npt66_prefix_del;
	p_adpt_api->adpt_flow_npt66_iid_cal = adpt_hppe_flow_npt66_iid_cal;
	p_adpt_api->adpt_flow_npt66_iid_add = adpt_hppe_flow_npt66_iid_add;
	p_adpt_api->adpt_flow_npt66_iid_get = adpt_hppe_flow_npt66_iid_get;
	p_adpt_api->adpt_flow_npt66_iid_del = adpt_hppe_flow_npt66_iid_del;
	p_adpt_api->adpt_flow_npt66_status_set = adpt_hppe_flow_npt66_status_set;
	p_adpt_api->adpt_flow_npt66_status_get = adpt_hppe_flow_npt66_status_get;

	return SW_OK;
}

/**
 * @}
 */
