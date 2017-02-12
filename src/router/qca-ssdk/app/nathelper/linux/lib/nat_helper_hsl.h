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
a_uint32_t
napt_hw_used_count_get(void);

sw_error_t napt_l3_status_set(a_uint32_t dev_id, a_bool_t enable);
sw_error_t napt_l3_status_get(a_uint32_t dev_id, a_bool_t * enable);

sw_error_t napt_helper_hsl_init(void);


#endif /*_NAT_HELPER_HSL_H*/

