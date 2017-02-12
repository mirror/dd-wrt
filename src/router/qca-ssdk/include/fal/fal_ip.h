/*
 * Copyright (c) 2012, 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/**
 * @defgroup fal_ip FAL_IP
 * @{
 */
#ifndef _FAL_IP_H_
#define _FAL_IP_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "common/sw.h"
#include "fal/fal_type.h"
#include "fal_multi.h"

#define FAL_MIN_VRF_ID 0
#define FAL_MAX_VRF_ID 7
    /* IP WCMP hash key flags */
#define FAL_WCMP_HASH_KEY_SIP             0x1
#define FAL_WCMP_HASH_KEY_DIP             0x2
#define FAL_WCMP_HASH_KEY_SPORT           0x4
#define FAL_WCMP_HASH_KEY_DPORT           0x8

    /* IP entry operation flags */
#define FAL_IP_ENTRY_ID_EN               0x1
#define FAL_IP_ENTRY_INTF_EN             0x2
#define FAL_IP_ENTRY_PORT_EN             0x4
#define FAL_IP_ENTRY_STATUS_EN           0x8
#define FAL_IP_ENTRY_IPADDR_EN           0x10

    /* IP host entry structure flags field */
#define FAL_IP_IP4_ADDR                         0x1
#define FAL_IP_IP6_ADDR                         0x2
#define FAL_IP_CPU_ADDR                         0x4

    typedef struct
    {
        a_uint32_t entry_id;
        a_uint32_t flags;
        a_uint32_t status;
        fal_ip4_addr_t ip4_addr;
        fal_ip6_addr_t ip6_addr;
        fal_mac_addr_t mac_addr;
        a_uint32_t intf_id;
        a_uint32_t lb_num;/*total 3 bits for load balance and highest bit for load balance enable or not*/
        a_uint32_t vrf_id;
        a_uint32_t expect_vid;
        fal_port_t port_id;
        a_bool_t mirror_en;
        a_bool_t counter_en;
        a_uint32_t counter_id;
        a_uint32_t packet;
        a_uint32_t byte;
        a_bool_t pppoe_en;
        a_uint32_t pppoe_id;
        fal_fwd_cmd_t action;
    } fal_host_entry_t;

    typedef enum
    {
        FAL_MAC_IP_GUARD = 0,
        FAL_MAC_IP_PORT_GUARD,
        FAL_MAC_IP_VLAN_GUARD,
        FAL_MAC_IP_PORT_VLAN_GUARD,
        FAL_NO_SOURCE_GUARD,
    } fal_source_guard_mode_t;

    typedef enum
    {
        FAL_DEFAULT_FLOW_FORWARD = 0,
        FAL_DEFAULT_FLOW_DROP,
        FAL_DEFAULT_FLOW_RDT_TO_CPU,
        FAL_DEFAULT_FLOW_ADMIT_ALL,
    } fal_default_flow_cmd_t;

    typedef enum
    {
        FAL_FLOW_LAN_TO_LAN = 0,
        FAL_FLOW_WAN_TO_LAN,
        FAL_FLOW_LAN_TO_WAN,
        FAL_FLOW_WAN_TO_WAN,
    } fal_flow_type_t;

typedef enum
{
	FAL_GLB_LOCK_TIME_DISABLE = 0,
	FAL_GLB_LOCK_TIME_100US,
	FAL_GLB_LOCK_TIME_1MS,
	FAL_GLB_LOCK_TIME_10MS,
} fal_glb_lock_time_t;
		

    typedef enum
    {
        FAL_ARP_LEARN_LOCAL = 0,
        FAL_ARP_LEARN_ALL,
    } fal_arp_learn_mode_t;

    /* IP host entry auto learn arp packets type */
#define FAL_ARP_LEARN_REQ                0x1
#define FAL_ARP_LEARN_ACK                0x2

    typedef struct
    {
        a_uint32_t entry_id;
        a_uint32_t vrf_id;
        a_uint16_t vid_low;
        a_uint16_t vid_high;
        fal_mac_addr_t mac_addr;
        a_bool_t ip4_route;
        a_bool_t ip6_route;
    } fal_intf_mac_entry_t;

    typedef struct
    {
        a_uint32_t nh_nr;
        a_uint32_t nh_id[16];
    } fal_ip_wcmp_t;

	typedef struct
    {
        fal_mac_addr_t mac_addr;
        fal_ip4_addr_t ip4_addr;
		a_uint32_t     vid;
		a_uint8_t      load_balance;
    } fal_ip4_rfs_t;

	typedef struct
    {
        fal_mac_addr_t mac_addr;
        fal_ip6_addr_t ip6_addr;
		a_uint32_t     vid;
		a_uint8_t      load_balance;
    } fal_ip6_rfs_t;

    typedef struct
    {
        a_bool_t valid;
        a_uint32_t vrf_id;
        fal_addr_type_t ip_version; /*0 for IPv4 and 1 for IPv6*/
        a_uint32_t droute_type; /*0 for ARP and 1 for WCMP*/
        a_uint32_t index;/*when droute_type equals 0, means ARP entry index or means WCMP indexs*/
    } fal_default_route_t;

    typedef struct
    {
        a_bool_t valid;
        a_uint32_t vrf_id;
        a_uint32_t ip_version; /*0 for IPv4 and 1 for IPv6*/
        union {
        fal_ip4_addr_t ip4_addr;
        fal_ip6_addr_t ip6_addr;
	}route_addr;
        a_uint32_t prefix_length;/*For IPv4, up to 32 and for IPv6, up to 128*/
    } fal_host_route_t;

    sw_error_t
    fal_ip_host_add(a_uint32_t dev_id, fal_host_entry_t * host_entry);

    sw_error_t
    fal_ip_host_del(a_uint32_t dev_id, a_uint32_t del_mode,
                    fal_host_entry_t * host_entry);

    sw_error_t
    fal_ip_host_get(a_uint32_t dev_id, a_uint32_t get_mode,
                    fal_host_entry_t * host_entry);

    sw_error_t
    fal_ip_host_next(a_uint32_t dev_id, a_uint32_t next_mode,
                     fal_host_entry_t * host_entry);

    sw_error_t
    fal_ip_host_counter_bind(a_uint32_t dev_id, a_uint32_t entry_id,
                             a_uint32_t cnt_id, a_bool_t enable);

    sw_error_t
    fal_ip_host_pppoe_bind(a_uint32_t dev_id, a_uint32_t entry_id,
                           a_uint32_t pppoe_id, a_bool_t enable);

    sw_error_t
    fal_ip_pt_arp_learn_set(a_uint32_t dev_id, fal_port_t port_id,
                            a_uint32_t flags);

    sw_error_t
    fal_ip_pt_arp_learn_get(a_uint32_t dev_id, fal_port_t port_id,
                            a_uint32_t * flags);

    sw_error_t
    fal_ip_arp_learn_set(a_uint32_t dev_id, fal_arp_learn_mode_t mode);

    sw_error_t
    fal_ip_arp_learn_get(a_uint32_t dev_id, fal_arp_learn_mode_t * mode);

    sw_error_t
    fal_ip_source_guard_set(a_uint32_t dev_id, fal_port_t port_id,
                            fal_source_guard_mode_t mode);

    sw_error_t
    fal_ip_source_guard_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_source_guard_mode_t * mode);

    sw_error_t
    fal_ip_arp_guard_set(a_uint32_t dev_id, fal_port_t port_id,
                         fal_source_guard_mode_t mode);

    sw_error_t
    fal_ip_arp_guard_get(a_uint32_t dev_id, fal_port_t port_id,
                         fal_source_guard_mode_t * mode);

    sw_error_t
    fal_ip_route_status_set(a_uint32_t dev_id, a_bool_t enable);

    sw_error_t
    fal_ip_route_status_get(a_uint32_t dev_id, a_bool_t * enable);

    sw_error_t
    fal_ip_intf_entry_add(a_uint32_t dev_id, fal_intf_mac_entry_t * entry);

    sw_error_t
    fal_ip_intf_entry_del(a_uint32_t dev_id, a_uint32_t del_mode,
                          fal_intf_mac_entry_t * entry);

    sw_error_t
    fal_ip_intf_entry_next(a_uint32_t dev_id, a_uint32_t next_mode,
                           fal_intf_mac_entry_t * entry);

    sw_error_t
    fal_ip_unk_source_cmd_set(a_uint32_t dev_id, fal_fwd_cmd_t cmd);

    sw_error_t
    fal_ip_unk_source_cmd_get(a_uint32_t dev_id, fal_fwd_cmd_t * cmd);

    sw_error_t
    fal_arp_unk_source_cmd_set(a_uint32_t dev_id, fal_fwd_cmd_t cmd);

    sw_error_t
    fal_arp_unk_source_cmd_get(a_uint32_t dev_id, fal_fwd_cmd_t * cmd);

    sw_error_t
    fal_ip_age_time_set(a_uint32_t dev_id, a_uint32_t * time);

    sw_error_t
    fal_ip_age_time_get(a_uint32_t dev_id, a_uint32_t * time);


    sw_error_t
    fal_ip_wcmp_hash_mode_set(a_uint32_t dev_id, a_uint32_t hash_mode);

    sw_error_t
    fal_ip_wcmp_hash_mode_get(a_uint32_t dev_id, a_uint32_t * hash_mode);

    sw_error_t
    fal_ip_vrf_base_addr_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr);

    sw_error_t
    fal_ip_vrf_base_addr_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr);

    sw_error_t
    fal_ip_vrf_base_mask_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr);

    sw_error_t
    fal_ip_vrf_base_mask_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr);

    sw_error_t
    fal_ip_default_route_set(a_uint32_t dev_id, a_uint32_t droute_id,
			fal_default_route_t * entry);

    sw_error_t
    fal_ip_default_route_get(a_uint32_t dev_id, a_uint32_t droute_id,
			fal_default_route_t * entry);

    sw_error_t
    fal_ip_host_route_set(a_uint32_t dev_id, a_uint32_t hroute_id,
			fal_host_route_t * entry);

    sw_error_t
    fal_ip_host_route_get(a_uint32_t dev_id, a_uint32_t hroute_id,
			fal_host_route_t * entry);

	sw_error_t
    fal_ip_wcmp_entry_set(a_uint32_t dev_id, a_uint32_t wcmp_id,
			fal_ip_wcmp_t * wcmp);

    sw_error_t
    fal_ip_wcmp_entry_get(a_uint32_t dev_id, a_uint32_t wcmp_id,
			fal_ip_wcmp_t * wcmp);

	sw_error_t
	fal_ip_rfs_ip4_rule_set(a_uint32_t dev_id, fal_ip4_rfs_t * rfs);

	sw_error_t
	fal_ip_rfs_ip6_rule_set(a_uint32_t dev_id, fal_ip6_rfs_t * rfs);

	sw_error_t
	fal_ip_rfs_ip4_rule_del(a_uint32_t dev_id, fal_ip4_rfs_t * rfs);

	sw_error_t
	fal_ip_rfs_ip6_rule_del(a_uint32_t dev_id, fal_ip6_rfs_t * rfs);

    sw_error_t
    fal_default_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id,
			fal_flow_type_t type, fal_default_flow_cmd_t cmd);

    sw_error_t
    fal_default_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id,
			fal_flow_type_t type, fal_default_flow_cmd_t * cmd);

    sw_error_t
    fal_default_rt_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id,
			fal_flow_type_t type, fal_default_flow_cmd_t cmd);

    sw_error_t
    fal_default_rt_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id,
			fal_flow_type_t type, fal_default_flow_cmd_t * cmd);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_IP_H_ */
/**
 * @}
 */
