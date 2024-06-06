/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#ifndef APPE_VXLAN_REG_H
#define APPE_VXLAN_REG_H

/*[register] UDP_PORT_CFG*/
#define UDP_PORT_CFG
#define UDP_PORT_CFG_ADDRESS 0x480
#define UDP_PORT_CFG_NUM     6
#define UDP_PORT_CFG_INC     0x4
#define UDP_PORT_CFG_TYPE    REG_TYPE_RW
#define UDP_PORT_CFG_DEFAULT 0x1f12b5
	/*[field] PORT_VALUE*/
	#define UDP_PORT_CFG_PORT_VALUE
	#define UDP_PORT_CFG_PORT_VALUE_OFFSET  0
	#define UDP_PORT_CFG_PORT_VALUE_LEN     16
	#define UDP_PORT_CFG_PORT_VALUE_DEFAULT 0x12b5
	/*[field] IP_VER*/
	#define UDP_PORT_CFG_IP_VER
	#define UDP_PORT_CFG_IP_VER_OFFSET  16
	#define UDP_PORT_CFG_IP_VER_LEN     2
	#define UDP_PORT_CFG_IP_VER_DEFAULT 0x3
	/*[field] UDP_TYPE*/
	#define UDP_PORT_CFG_UDP_TYPE
	#define UDP_PORT_CFG_UDP_TYPE_OFFSET  18
	#define UDP_PORT_CFG_UDP_TYPE_LEN     2
	#define UDP_PORT_CFG_UDP_TYPE_DEFAULT 0x3
	/*[field] PORT_TYPE*/
	#define UDP_PORT_CFG_PORT_TYPE
	#define UDP_PORT_CFG_PORT_TYPE_OFFSET  20
	#define UDP_PORT_CFG_PORT_TYPE_LEN     2
	#define UDP_PORT_CFG_PORT_TYPE_DEFAULT 0x1

struct udp_port_cfg {
	a_uint32_t  _reserved0:10;
	a_uint32_t  port_type:2;
	a_uint32_t  udp_type:2;
	a_uint32_t  ip_ver:2;
	a_uint32_t  port_value:16;
};

union udp_port_cfg_u {
	a_uint32_t val;
	struct udp_port_cfg bf;
};

/*[register] TPR_VXLAN_CFG*/
#define TPR_VXLAN_CFG
#define TPR_VXLAN_CFG_ADDRESS 0x4a0
#define TPR_VXLAN_CFG_NUM     1
#define TPR_VXLAN_CFG_INC     0x4
#define TPR_VXLAN_CFG_TYPE    REG_TYPE_RW
#define TPR_VXLAN_CFG_DEFAULT 0x0
	/*[field] UDP_PORT_MAP*/
	#define TPR_VXLAN_CFG_UDP_PORT_MAP
	#define TPR_VXLAN_CFG_UDP_PORT_MAP_OFFSET  0
	#define TPR_VXLAN_CFG_UDP_PORT_MAP_LEN     6
	#define TPR_VXLAN_CFG_UDP_PORT_MAP_DEFAULT 0x0

struct tpr_vxlan_cfg {
	a_uint32_t  _reserved0:26;
	a_uint32_t  udp_port_map:6;
};

union tpr_vxlan_cfg_u {
	a_uint32_t val;
	struct tpr_vxlan_cfg bf;
};

/*[register] TPR_VXLAN_GPE_CFG*/
#define TPR_VXLAN_GPE_CFG
#define TPR_VXLAN_GPE_CFG_ADDRESS 0x4a4
#define TPR_VXLAN_GPE_CFG_NUM     1
#define TPR_VXLAN_GPE_CFG_INC     0x4
#define TPR_VXLAN_GPE_CFG_TYPE    REG_TYPE_RW
#define TPR_VXLAN_GPE_CFG_DEFAULT 0x0
	/*[field] UDP_PORT_MAP*/
	#define TPR_VXLAN_GPE_CFG_UDP_PORT_MAP
	#define TPR_VXLAN_GPE_CFG_UDP_PORT_MAP_OFFSET  0
	#define TPR_VXLAN_GPE_CFG_UDP_PORT_MAP_LEN     6
	#define TPR_VXLAN_GPE_CFG_UDP_PORT_MAP_DEFAULT 0x0

struct tpr_vxlan_gpe_cfg {
	a_uint32_t  _reserved0:26;
	a_uint32_t  udp_port_map:6;
};

union tpr_vxlan_gpe_cfg_u {
	a_uint32_t val;
	struct tpr_vxlan_gpe_cfg bf;
};

#ifndef IN_VXLAN_MINI
/*[register] TPR_VXLAN_GPE_PROT_CFG*/
#define TPR_VXLAN_GPE_PROT_CFG
#define TPR_VXLAN_GPE_PROT_CFG_ADDRESS 0x4ac
#define TPR_VXLAN_GPE_PROT_CFG_NUM     1
#define TPR_VXLAN_GPE_PROT_CFG_INC     0x4
#define TPR_VXLAN_GPE_PROT_CFG_TYPE    REG_TYPE_RW
#define TPR_VXLAN_GPE_PROT_CFG_DEFAULT 0x30201
	/*[field] IPV4*/
	#define TPR_VXLAN_GPE_PROT_CFG_IPV4
	#define TPR_VXLAN_GPE_PROT_CFG_IPV4_OFFSET  0
	#define TPR_VXLAN_GPE_PROT_CFG_IPV4_LEN     8
	#define TPR_VXLAN_GPE_PROT_CFG_IPV4_DEFAULT 0x1
	/*[field] IPV6*/
	#define TPR_VXLAN_GPE_PROT_CFG_IPV6
	#define TPR_VXLAN_GPE_PROT_CFG_IPV6_OFFSET  8
	#define TPR_VXLAN_GPE_PROT_CFG_IPV6_LEN     8
	#define TPR_VXLAN_GPE_PROT_CFG_IPV6_DEFAULT 0x2
	/*[field] ETHERNET*/
	#define TPR_VXLAN_GPE_PROT_CFG_ETHERNET
	#define TPR_VXLAN_GPE_PROT_CFG_ETHERNET_OFFSET  16
	#define TPR_VXLAN_GPE_PROT_CFG_ETHERNET_LEN     8
	#define TPR_VXLAN_GPE_PROT_CFG_ETHERNET_DEFAULT 0x3

struct tpr_vxlan_gpe_prot_cfg {
	a_uint32_t  _reserved0:8;
	a_uint32_t  ethernet:8;
	a_uint32_t  ipv6:8;
	a_uint32_t  ipv4:8;
};

union tpr_vxlan_gpe_prot_cfg_u {
	a_uint32_t val;
	struct tpr_vxlan_gpe_prot_cfg bf;
};
#endif

#endif

