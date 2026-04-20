/*
 * Copyright (c) 2012, 2015, 2018, The Linux Foundation. All rights reserved.
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


#ifndef _NAT_HELPER_HSL_H
#define _NAT_HELPER_HSL_H

#ifdef KVER32
#include <linux/kconfig.h> 
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif

#include <linux/if_ether.h>
#include "fal_nat.h"

#define NAT_HW_NUM 32
#define NAT_HW_PORT_RANGE_MAX 255

#define FAL_NAT_ENTRY_PROTOCOL_TCP         0x1
#define FAL_NAT_ENTRY_PROTOCOL_UDP         0x2
#define FAL_NAT_ENTRY_PROTOCOL_PPTP        0x4
#define FAL_NAT_ENTRY_PROTOCOL_ANY         0x8
#define FAL_NAT_ENTRY_PORT_CHECK           0x20

#define MAX_INTF_NUM    4

/* WAN connection types */
#define NF_S17_WAN_TYPE_IP  0   /* DHCP, static IP connection */
#define NF_S17_WAN_TYPE_PPPOE   1   /* PPPoE connection */
#define NF_S17_WAN_TYPE_GRE 2   /* GRE connections, ex: PPTP */
#define NF_S17_WAN_TYPE_PPPOEV6 3   /* IPv6 PPPoE connection  using the same session as IPv4 connection */
#define NF_S17_WAN_TYPE_PPPOES0 4   /* PPPoE connection but not yet connected */
/* define the H/W Age mode for NAPT entries */
#define ARP_AGE_NEVER   7
#define ARP_AGE 6

#if !defined (HSL_STANDALONG)
extern a_uint32_t nat_dev_id;

/*NAT API*/
#define NAPT_ADD(dev_id, ...) fal_napt_add(nat_dev_id, ##__VA_ARGS__)
#define NAPT_GET(dev_id, ...) fal_napt_get(nat_dev_id, ##__VA_ARGS__)
#define NAT_PUB_ADDR_ADD(dev_id, ...) fal_nat_pub_addr_add(nat_dev_id, ##__VA_ARGS__)
#define NAPT_NEXT(dev_id, ...) fal_napt_next(nat_dev_id, ##__VA_ARGS__)
#define NAT_PRV_BASE_ADDR_SET(dev_id, ...) fal_nat_prv_base_addr_set(nat_dev_id, ##__VA_ARGS__)
#define NAT_PRV_BASE_MASK_SET(dev_id, ...) fal_nat_prv_base_mask_set(nat_dev_id, ##__VA_ARGS__)
#define NAPT_DEL(dev_id, ...) fal_napt_del(nat_dev_id, ##__VA_ARGS__)
#define NAT_DEL(dev_id, ...) fal_nat_del(nat_dev_id, ##__VA_ARGS__)
#define NAT_PUB_ADDR_DEL(dev_id, ...) fal_nat_pub_addr_del(nat_dev_id, ##__VA_ARGS__)
#define NAT_ADD(dev_id, ...) fal_nat_add(nat_dev_id, ##__VA_ARGS__)
#define NAT_PRV_ADDR_MODE_GET(dev_id, ...) fal_nat_prv_addr_mode_get(nat_dev_id, ##__VA_ARGS__)

/*IP API*/
#define IP_INTF_ENTRY_ADD(dev_id, ...) fal_ip_intf_entry_add(nat_dev_id, ##__VA_ARGS__)
#define IP_HOST_ADD(dev_id, ...) fal_ip_host_add(nat_dev_id, ##__VA_ARGS__)
#define IP_HOST_DEL(dev_id, ...) fal_ip_host_del(nat_dev_id, ##__VA_ARGS__)
#define IP_HOST_GET(dev_id, ...) fal_ip_host_get(nat_dev_id, ##__VA_ARGS__)
#define IP_HOST_NEXT(dev_id, ...) fal_ip_host_next(nat_dev_id, ##__VA_ARGS__)
#define IP_INTF_ENTRY_DEL(dev_id, ...) fal_ip_intf_entry_del(nat_dev_id, ##__VA_ARGS__)
#define IP_HOST_PPPOE_BIND(dev_id, ...) fal_ip_host_pppoe_bind(nat_dev_id, ##__VA_ARGS__)
#define IP_ROUTE_STATUS_SET(dev_id, ...) fal_ip_route_status_set(nat_dev_id, ##__VA_ARGS__)
#define IP_HOST_ROUTE_ADD(dev_id, ...) fal_ip_host_route_set(nat_dev_id, ##__VA_ARGS__)
#define IP_PRV_BASE_ADDR_SET(dev_id, ...) fal_ip_vrf_base_addr_set(nat_dev_id, ##__VA_ARGS__)
#define IP_PRV_BASE_MASK_SET(dev_id, ...) fal_ip_vrf_base_mask_set(nat_dev_id, ##__VA_ARGS__)

/* PPPOE */
#define PPPOE_STATUS_GET(dev_id, ...) fal_pppoe_status_get(nat_dev_id, ##__VA_ARGS__)
#define PPPOE_STATUS_SET(dev_id, ...) fal_pppoe_status_set(nat_dev_id, ##__VA_ARGS__)
#define PPPOE_SESSION_ID_SET(dev_id, ...) fal_pppoe_session_id_set(nat_dev_id, ##__VA_ARGS__)
#define PPPOE_SESSION_TABLE_ADD(dev_id, ...) fal_pppoe_session_table_add(nat_dev_id, ##__VA_ARGS__)
#define PPPOE_SESSION_TABLE_DEL(dev_id, ...) fal_pppoe_session_table_del(nat_dev_id, ##__VA_ARGS__)
#define RTD_PPPOE_EN_SET(dev_id, ...) fal_rtd_pppoe_en_set(nat_dev_id, ##__VA_ARGS__)

/*MISC API*/
#define MISC_ARP_CMD_SET(dev_id, ...) fal_arp_cmd_set(nat_dev_id, ##__VA_ARGS__)
#define CPU_VID_EN_SET(dev_id, ...) fal_cpu_vid_en_set(nat_dev_id, ##__VA_ARGS__)
#define PORT_ARP_ACK_STATUS_SET(dev_id, ...) fal_port_arp_ack_status_set(nat_dev_id, ##__VA_ARGS__)
#define CPU_PORT_STATUS_SET(dev_id, ...) fal_cpu_port_status_set(nat_dev_id, ##__VA_ARGS__)

/*ACL API*/
#define ACL_RULE_ADD(dev_id, ...) fal_acl_rule_add(nat_dev_id, ##__VA_ARGS__)
#define ACL_RULE_DEL(dev_id, ...) fal_acl_rule_delete(nat_dev_id, ##__VA_ARGS__)
#define ACL_LIST_CREATE(dev_id, ...) fal_acl_list_creat(nat_dev_id, ##__VA_ARGS__)
#define ACL_LIST_DESTROY(dev_id, ...) fal_acl_list_destroy(nat_dev_id, ##__VA_ARGS__)
#define ACL_LIST_BIND(dev_id, ...) fal_acl_list_bind(nat_dev_id, ##__VA_ARGS__)
#define ACL_LIST_UNBIND(dev_id, ...) fal_acl_list_unbind(nat_dev_id, ##__VA_ARGS__)
#define ACL_STATUS_GET(dev_id, ...) fal_acl_status_get(nat_dev_id, ##__VA_ARGS__)
#define ACL_STATUS_SET(dev_id, ...) fal_acl_status_set(nat_dev_id, ##__VA_ARGS__)
#define ACL_PORT_UDF_PROFILE_SET(dev_id, ...) fal_acl_port_udf_profile_set(nat_dev_id, ##__VA_ARGS__)

/*VLAN API */
#define VLAN_NEXT(dev_id, ...) fal_vlan_next(nat_dev_id, ##__VA_ARGS__)

/* PORTVLAN API */
#define PORTVLAN_ROUTE_DEFV_SET(dev_id, port_id)
#define NETISOLATE_SET(dev_id, ...) fal_netisolate_set(nat_dev_id, ##__VA_ARGS__)

/* PORT_CTRL API */
#define HEADER_TYPE_SET(dev_id, ...) fal_header_type_set(nat_dev_id, ##__VA_ARGS__)
#define PORT_TXHDR_MODE_SET(dev_id, ...) fal_port_txhdr_mode_set(nat_dev_id, ##__VA_ARGS__)

/* REG ACCESS API */
#define REG_GET(dev_id, ...) fal_reg_get(nat_dev_id, ##__VA_ARGS__)


#define L3_STATUS_SET(dev_id, ...) napt_l3_status_set(nat_dev_id, ##__VA_ARGS__)
#define L3_STATUS_GET(dev_id, ...) napt_l3_status_get(nat_dev_id, ##__VA_ARGS__)

#define FDB_ENTRY_SEARCH(dev_id, ...) fal_fdb_entry_search(nat_dev_id, ##__VA_ARGS__)
#define IP_HOST_ROUTE_GET(dev_id, ...) fal_ip_host_route_get(nat_dev_id, ##__VA_ARGS__)

#define REG_ENTRY_SET(rv, dev_id, ...) HSL_REG_ENTRY_SET(rv, nat_dev_id, ##__VA_ARGS__)
#define REG_ENTRY_GET(rv, dev_id, ...) HSL_REG_ENTRY_GET(rv, nat_dev_id, ##__VA_ARGS__)
#endif

extern int nf_athrs17_hnat;
extern int nf_athrs17_hnat_wan_type;
extern int nf_athrs17_hnat_ppp_id;
extern int nf_athrs17_hnat_udp_thresh;
extern a_uint32_t nf_athrs17_hnat_wan_ip;
extern a_uint32_t nf_athrs17_hnat_ppp_peer_ip;
extern unsigned char nf_athrs17_hnat_ppp_peer_mac[ETH_ALEN];
extern unsigned char nf_athrs17_hnat_wan_mac[ETH_ALEN];
extern int nf_athrs17_hnat_sync_counter_en;

extern int nf_athrs17_hnat_ppp_id2;
extern unsigned char nf_athrs17_hnat_ppp_peer_mac2[ETH_ALEN];

enum {
	NAT_CHIP_VER_8327 = 0x12,
	NAT_CHIP_VER_8337 = 0x13,
	NAT_CHIP_VER_DESS = 0x14,
};

typedef struct
{
    a_uint32_t entry_id;
    a_uint32_t flags;
    a_uint32_t src_addr;
    a_uint32_t trans_addr;
    a_uint16_t port_num;
    a_uint16_t port_range;
} nat_entry_t;

typedef struct
{
    a_uint32_t entry_id;
    a_uint32_t flags;
    a_uint32_t status;
    a_uint32_t src_addr;
    a_uint32_t dst_addr;
    a_uint16_t src_port;
    a_uint16_t dst_port;
    a_uint32_t trans_addr;
    a_uint16_t trans_port;
    a_uint32_t ingress_packet;
    a_uint32_t ingress_byte;
    a_uint32_t egress_packet;
    a_uint32_t egress_byte;
} napt_entry_t;

#if defined (__BIG_ENDIAN)
typedef struct
{
    a_uint16_t ver:2;
    a_uint16_t pri:3;
    a_uint16_t type:5;
    a_uint16_t rev:2;
    a_uint16_t with_tag:1;
    a_uint16_t sport:3;
    a_uint16_t vid;
    a_uint16_t magic;
} aos_header_t;
#elif defined (__LITTLE_ENDIAN)
typedef struct
{
    a_uint16_t vid;
    a_uint16_t sport:3;
    a_uint16_t with_tag:1;
    a_uint16_t rev:2;
    a_uint16_t type:5;
    a_uint16_t pri:3;
    a_uint16_t ver:2;
} aos_header_t;
#else
#error "no ENDIAN"
#endif

a_int32_t
nat_hw_add(nat_entry_t *nat);
a_int32_t
nat_hw_del_by_index(a_uint32_t index);
a_int32_t
nat_hw_flush(void);
a_int32_t
napt_hw_flush(void);
a_int32_t
nat_hw_prv_base_can_update(void);
void
nat_hw_prv_base_update_enable(void);
void
nat_hw_prv_base_update_disable(void);
a_int32_t
nat_hw_prv_base_set(a_uint32_t ip);
a_uint32_t
nat_hw_prv_base_get(void);
a_int32_t
nat_hw_prv_mask_set(a_uint32_t ipmask);
a_uint32_t
nat_hw_prv_mask_get(void);
a_int32_t
nat_hw_prv_base_is_match(a_uint32_t ip);
a_int32_t
if_mac_add(uint8_t *mac, uint32_t vid, uint32_t ipv6);
a_int32_t
if_mac_cleanup(void);
a_int32_t
arp_hw_add(a_uint32_t port, a_uint32_t intf_id, a_uint8_t *ip, a_uint8_t *mac, int is_ipv6_entry);
a_int32_t
arp_if_info_get(void *data, a_uint32_t *sport, a_uint32_t *vid);
a_int32_t
nat_hw_pub_ip_add(a_uint32_t ip, a_uint32_t *index);
void
napt_hw_mode_init(void);
void
napt_hw_mode_cleanup(void);
a_int32_t
nat_hw_pub_ip_del(a_uint32_t index);
a_int32_t
napt_hw_add(napt_entry_t *napt_entry);
a_int32_t
napt_hw_get(napt_entry_t *napt, fal_napt_entry_t *entry);
a_int32_t
napt_hw_dnat_cookie_add(napt_entry_t *napt, a_uint32_t cookie);
a_int32_t
napt_hw_snat_cookie_add(napt_entry_t *napt, a_uint32_t cookie);
a_int32_t
napt_hw_del(napt_entry_t *napt_entry);
a_int32_t
napt_hw_first_by_age(napt_entry_t *napt, a_uint32_t age);
a_int32_t
napt_hw_next_by_age(napt_entry_t *napt, a_uint32_t age);
a_int32_t
napt_hw_get_by_index(napt_entry_t *napt, a_uint16_t hw_index);
a_int32_t napt_hw_get_by_sip(a_uint32_t sip);
a_uint32_t
napt_hw_used_count_get(void);

sw_error_t napt_l3_status_set(a_uint32_t dev_id, a_bool_t enable);
sw_error_t napt_l3_status_get(a_uint32_t dev_id, a_bool_t * enable);

sw_error_t napt_helper_hsl_init(void);


#endif /*_NAT_HELPER_HSL_H*/

