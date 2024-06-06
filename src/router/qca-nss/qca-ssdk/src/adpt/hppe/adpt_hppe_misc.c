/*
 * Copyright (c) 2017, 2019, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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
 * @defgroup
 * @{
 */
#include "sw.h"
#include "hppe_portctrl_reg.h"
#include "hppe_portctrl.h"
#include "hppe_vsi_reg.h"
#include "hppe_vsi.h"
#include "hppe_policer_reg.h"
#include "hppe_policer.h"
#include "hppe_qm_reg.h"
#include "hppe_qm.h"
#include "adpt_hppe.h"
#include "adpt.h"
#ifdef APPE
#include "appe_counter.h"
#endif

char *cpucode[] = {
"Forwarding to CPU",
"Unkown L2 protocol exception redirect/copy to CPU",
"PPPoE wrong version or wrong type exception redirect/copy to CPU",
"PPPoE wrong code exception redirect/copy to CPU",
"PPPoE unsupported PPP protocol exception redirect/copy to CPU",
"IPv4 wrong version exception redirect/copy to CPU",
"IPv4 small IHL exception redirect/copy to CPU",
"IPv4 with option exception redirect/copy to CPU",
"IPv4 header incomplete exception redirect/copy to CPU",
"IPv4 bad total length exception redirect/copy to CPU",
"IPv4 data incomplete exception redirect/copy to CPU",
"IPv4 fragment exception redirect/copy to CPU",
"IPv4 ping of death exception redirect/copy to CPU",
"IPv4 small TTL exception redirect/copy to CPU",
"IPv4 unknown IP protocol exception redirect/copy to CPU",
"IPv4 checksum error exception redirect/copy to CPU",
"IPv4 invalid SIP exception redirect/copy to CPU",
"IPv4 invalid DIP exception redirect/copy to CPU",
"IPv4 LAND attack exception redirect/copy to CPU",
"IPv4 AH header incomplete exception redirect/copy to CPU",
"IPv4 AH header cross 128-byte exception redirect/copy to CPU",
"IPv4 ESP header incomplete exception redirect/copy to CPU",
"IPv6 wrong version exception redirect/copy to CPU",
"IPv6 header incomplete exception redirect/copy to CPU",
"IPv6 bad total length exception redirect/copy to CPU",
"IPv6 data incomplete exception redirect/copy to CPU",
"IPv6 with extension header exception redirect/copy to CPU",
"IPv6 small hop limit exception redirect/copy to CPU",
"IPv6 invalid SIP exception redirect/copy to CPU",
"IPv6 invalid DIP exception redirect/copy to CPU",
"IPv6 LAND attack exception redirect/copy to CPU",
"IPv6 fragment exception redirect/copy to CPU",
"IPv6 ping of death exception redirect/copy to CPU",
"IPv6 with more than 2 extension headers exception redirect/copy to CPU",
"IPv6 unknown last next header exception redirect/copy to CPU",
"IPv6 mobility header incomplete exception redirect/copy to CPU",
"IPv6 mobility header cross 128-byte exception redirect/copy to CPU",
"IPv6 AH header incomplete exception redirect/copy to CPU",
"IPv6 AH header cross 128-byte exception redirect/copy to CPU",
"IPv6 ESP header incomplete exception redirect/copy to CPU",
"IPv6 ESP header cross 128-byte exception redirect/copy to CPU",
"IPv6 other extension header incomplete exception redirect/copy to CPU",
"IPv6 other extension header cross 128-byte exception redirect/copy to CPU",
"TCP header incomplete exception redirect/copy to CPU",
"TCP header cross 128-byte exception redirect/copy to CPU",
"TCP same SP and DP exception redirect/copy to CPU",
"TCP small data offset redirect/copy to CPU",
"TCP flags VALUE/MASK group 0 exception redirect/copy to CPU",
"TCP flags VALUE/MASK group 1 exception redirect/copy to CPU",
"TCP flags VALUE/MASK group 2 exception redirect/copy to CPU",
"TCP flags VALUE/MASK group 3 exception redirect/copy to CPU",
"TCP flags VALUE/MASK group 4 exception redirect/copy to CPU",
"TCP flags VALUE/MASK group 5 exception redirect/copy to CPU",
"TCP flags VALUE/MASK group 6 exception redirect/copy to CPU",
"TCP flags VALUE/MASK group 7 exception redirect/copy to CPU",
"TCP checksum error exception redirect/copy to CPU",
"UDP header incomplete exception redirect/copy to CPU",
"UDP header cross 128-byte exception redirect/copy to CPU",
"UDP same SP and DP exception redirect/copy to CPU",
"UDP bad length exception redirect/copy to CPU",
"UDP data incomplete exception redirect/copy to CPU",
"UDP checksum error exception redirect/copy to CPU",
"UDP-Lite header incomplete exception redirect/copy to CPU",
"UDP-Lite header cross 128-byte exception redirect/copy to CPU",
"UDP-Lite same SP and DP exception redirect/copy to CPU",
"UDP-Lite checksum coverage value 0-7 exception redirect/copy to CPU",
"UDP-Lite checksum coverage value too big exception redirect/copy to CPU",
"UDP-Lite checksum coverage value cross 128-byte exception redirect/copy to CPU",
"UDP-Lite checksum error exception redirect/copy to CPU",
"Fake L2 protocol packet redirect/copy to CPU",
"Fake MAC header packet redirect/copy to CPU",
"L2 MRU checking fail redirect/copy to CPU",
"L2 MTU checking fail redirect/copy to CPU",
"IP prefix broadcast redirect/copy to CPU",
"L3 MTU checking fail redirect/copy to CPU",
"L3 MRU checking fail redirect/copy to CPU",
"ICMP redirect/copy to CPU",
"IP to me routing TTL 1 redirect/copy to CPU",
"IP to me routing TTL 0 redirect/copy to CPU",
"Flow service code loop redirect/copy to CPU",
"Flow de-accelearate redirect/copy to CPU",
"Flow source interface check fail redirect/copy to CPU",
"Flow sync toggle mismatch redirect/copy to CPU",
"MTU check fail if DF set redirect/copy to CPU",
"PPPoE multicast redirect/copy to CPU",
"EAPoL packet redirect/copy to CPU",
"PPPoE discovery packet redirect/copy to CPU",
"IGMP packet redirect/copy to CPU",
"ARP request packet redirect/copy to CPU",
"ARP reply packet redirect/copy to CPU",
"DHCPv4 packet redirect/copy to CPU",
"MLD packet redirect/copy to CPU",
"NS packet redirect/copy to CPU",
"NA packet redirect/copy to CPU",
"DHCPv6 packet redirect/copy to CPU",
"PTP sync packet redirect/copy to CPU",
"PTP follow up packet redirect/copy to CPU",
"PTP delay request packet redirect/copy to CPU",
"PTP delay response packet redirect/copy to CPU",
"PTP pdelay request packet redirect/copy to CPU",
"PTP pdelay response packet redirect/copy to CPU",
"PTP pdelay response follow up packet redirect/copy to CPU",
"PTP announce packet redirect/copy to CPU",
"PTP management packet redirect/copy to CPU",
"PTP signaling packet redirect/copy to CPU",
"PTP message reserved type 0 packet redirect/copy to CPU",
"PTP message reserved type 1 packet redirect/copy to CPU",
"PTP message reserved type 2 packet redirect/copy to CPU",
"PTP message reserved type 3 packet redirect/copy to CPU",
"PTP message reserved type packet redirect/copy to CPU",
"IPv4 source guard unknown packet redirect/copy to CPU",
"IPv6 source guard unknown packet redirect/copy to CPU",
"ARP source guard unknown packet redirect/copy to CPU",
"ND source guard unknown packet redirect/copy to CPU",
"IPv4 source guard violation packet redirect/copy to CPU",
"IPv6 source guard violation packet redirect/copy to CPU",
"ARP source guard violation packet redirect/copy to CPU",
"ND source guard violation packet redirect/copy to CPU",
"L3 route host mismatch action redirect/copy to CPU",
"L3 flow SNAT action redirect/copy to CPU",
"L3 flow DNAT action redirect/copy to CPU",
"L3 flow routing action redirect/copy to CPU",
"L3 flow bridging action redirect/copy to CPU",
"L3 multicast bridging action redirect/copy to CPU",
"L3 route Preheader routing action redirect/copy to CPU",
"L3 route Preheader SNAPT action redirect/copy to CPU",
"L3 route Preheader DNAPT action redirect/copy to CPU",
"L3 route Preheader SNAT action redirect/copy to CPU",
"L3 route Preheader DNAT action redirect/copy to CPU",
"L3 no route preheader NAT action redirect/copy to CPU",
"L3 no route preheader NAT error redirect/copy to CPU",
"L3 route action redirect/copy to CPU",
"L3 no route action redirect/copy to CPU",
"L3 no route next hop invalid action redirect/copy to CPU",
"L3 no route preheader action redirect/copy to CPU",
"L3 bridge action redirect/copy to CPU",
"L3 flow action redirect/copy to CPU",
"L3 flow miss action redirect/copy to CPU",
"L2 new MAC address redirect/copy to CPU",
"L2 hash violation redirect/copy to CPU",
"L2 station move redirect/copy to CPU",
"L2 learn limit redirect/copy to CPU",
"L2 SA lookup action redirect/copy to CPU",
"L2 DA lookup action redirect/copy to CPU",
"APP_CTRL action redirect/copy to CPU",
"Pre-IPO action",
"Post-IPO action",
"Service code action",
"Egress mirror to CPU",
"Ingress mirror to CPU",
#ifdef APPE
"L3 FLOW MTU CHECK FAIL",/*index 150, cpu code 93*/
"L3 FLOW MTU CHECK DF FAIL",
"L3 UDP CHECKSUM EXP",/*index 152, cpu code 95*/

"8023ah OAM packet redirect/copy to CPU",/*index 153, cpu code 104*/

"L3 ROUTE PRE IPO ROUTE ACTION",/*index 154, cpu code 181*/
"L3 ROUTE PRE IPO SNAPT ACTION",
"L3 ROUTE PRE IPO DNAPT ACTION",
"L3 ROUTE PRE IPO SNAT ACTION",
"L3_ROUTE PRE IPO DNAT ACTION",/*index 158, cpu code 185*/

"TUNNEL interface check fail",/*index 159,cpu code 186*/
"TUNNEL vlan check fail",
"TUNNEL PPPOE multicast term",
"TUNNEL de-accelate",
"TUNNEL UDP checksum zero",
"TUNNEL TTL exceed",
"TUNNEL LPM interface check fail",
"TUNNEL LPM vlan check fail",
"TUNNEL MAP source check fail",
"TUNNEL MAP destination check fail",
"TUNNEL MAP UDP checksum zero",
"TUNNEL MAP non TCP and UDP",
"TUNNEL forward command",/*index 171, cpu code 198*/

"L2 PRE-ACL action",/*index 172, cpu code 210*/
"TUNNEL L2 context invalid",
"reserve0",
"reserve1",
"TUNNEL decap ECN",
"TUNNEL inner packet too short",
"TUNNEL VXLAN header",
"TUNNEL VXLAN GPE header",
"TUNNEL GENEVE header",
"TUNNEL GRE header",
"reserved",
"TUNNEL unknow inner type",
"TUNNEL VXLAN flag",
"TUNNEL VXLAN GRE flag",
"TUNNEL GRE flag",
"TUNNEL GENEVE flag",
"TUNNEL PROGRAM0",
"TUNNEL PROGRAM1",
"TUNNEL PROGRAM2",
"TUNNEL PROGRAM3",
"TUNNEL PROGRAM4",
"TUNNEL PROGRAM5",/*index 193, cpu code 231*/
"bypass l2 flooding and redirect to CPU",/*index 194, cpu code 232*/
#endif
};

char *dropcode[] = {
#ifdef APPE
"None",
"Unkown L2 protocol exception drop",
"PPPoE wrong version or wrong header exception drop",
"PPPoE unsupported PPP protocol exception drop",
"IPv4 wrong version exception drop",
"IPv4 wrong format exception drop",
"IPv4 with option exception drop",
"IPv4 bad total length exception drop",
"IPv4 fragment exception drop",
"IPv4 ping of death exception drop",
"IPv4 small TTL exception drop",
"IPv4 unknown IP protocol exception drop",
"IPv4 checksum error exception drop",
"IPv4 invalid IP exception drop",
"IPv4 LAND attack exception drop",
"IPv4 other exception drop",
"IPv6 wrong version exception drop",
"IPv6 wrong format exception drop",
"IPv6 bad payload length exception drop",
"IPv6 with extension header exception drop",
"IPv6 small hop limit exception drop",
"IPv6 invalid IP exception drop",
"IPv6 LAND attack exception drop",
"IPv6 fragment exception drop",
"IPv6 ping of death exception drop",
"IPv6 with more than 2 extension headers exception drop",
"IPv6 other exception drop",
"TCP format error exception drop",
"TCP same SP and DP exception drop",
"TCP flags exception drop",
"TCP checksum error exception drop",
"UDP format error exception drop",
"UDP same SP and DP exception drop",
"UDP bad length exception drop",
"UDP checksum error exception drop",
"UDP-Lite format error exception drop",
"UDP-Lite checksum error exception drop",
"L3 route PRE-IPO RT drop",
"L3 route PRE-IPO SNAPT drop",
"L3 route PRE-IPO DNAPT drop",
"L3 route PRE-IPO SNAT drop",
"L3 route PRE-IPO DNAT drop",
"TUNNEL interface check fail exception drop",
"TUNNEL vlan check fail exception drop",
"TUNNEL PPPOE multicast term exception drop",
"TUNNEL de-accelerate exception drop",
"TUNNEL UDP checksum zero drop",
"TUNNEL TTL exceed exception drop",
"TUNNEL LPM interface check fail drop",
"TUNNEL LPM VLAN check fail drop",
"TUNNEL MAP source check fail drop",
"TUNNEL MAP destination check fail drop",
"TUNNEL MAP UDP checksum zero drop",
"TUNNEL MAP non TCP UDP drop",
"Pre-ACL entry hit action drop",
"TUNNEL L2 invalid context exception drop",
"reserve0 exception drop",
"reserve1 exception drop",
"TUNNEL decap ECN drop",
"TUNNEL inner packet too short drop",
"TUNNEL VXLAN header exception drop",
"TUNNEL VXLAN GRE header exception drop",
"TUNNEL GENEVE header exception drop",
"TUNNEL GRE header exception drop",
"reserved exception drop",
"TUNNEL unknow inner type exception drop",
"TUNNEL flag exception drop",
"TUNNEL PROGRAM exception drop",
"TUNNEL forward command exception drop",
"L3 bridge action drop",
"L3 no route with Preheader NAT action",
"L3 no route with Preheader NAT action error configuration",
"L3 route action drop",
"L3 no route action drop",
"L3 no route next hop invalid action drop",
"L3 no route preheader action drop",
"L3 bridge action drop",
"L3 flow action drop",
"L3 flow miss action drop",
"L2 MRU checking fail drop",
"L2 MTU checking fail drop",
"L3 IP prefix broadcast drop",
"L3 MTU checking fail drop",
"L3 MRU checking fail drop",
"L3 ICMP redirect drop",
"Fake MAC header indicated packet not routing or bypass L3 edit drop",
"L3 IP route TTL zero drop",
"L3 flow service code loop drop",
"L3 flow de-accelerate drop",
"L3 flow source interface check fail drop",
"Flow toggle mismatch exception drop",
"MTU check exception if DF set drop",
"PPPoE multicast packet with IP routing enabled drop",
"IPv4 SG unkown drop",
"IPv6 SG unkown drop",
"ARP SG unkown drop",
"ND SG unkown drop",
"IPv4 SG violation drop",
"IPv6 SG violation drop",
"ARP SG violation drop",
"ND SG violation drop",
"L2 new MAC address drop",
"L2 hash violation drop",
"L2 station move drop",
"L2 learn limit drop",
"L2 SA lookup action drop",
"L2 DA lookup action drop",
"APP_CTRL action drop",
"Ingress VLAN filtering action drop",
"Ingress VLAN translation miss drop",
"Egress VLAN filtering drop",
"ACL-pre entry hit action drop",
"ACL-post entry hit action drop",
"Multicast SA or broadcast SA drop",
"No destination drop",
"STG ingress filtering drop",
"STG egress filtering drop",
"Source port filter drop",
"Trunk select fail drop",
"TX MAC disable drop",
"Ingress VLAN tag format drop",
"CRC error drop",
"PAUSE frame drop",
"Promisc drop",
"Isolation drop",
"Magagement packet APP_CTRL drop",
"Fake L2 protocol indicated packet not routing or bypass L3 edit drop",
"Policing drop",
#else
"None",
"Unkown L2 protocol exception drop",
"PPPoE wrong version or wrong type exception drop",
"PPPoE wrong code exception drop",
"PPPoE unsupported PPP protocol exception drop",
"IPv4 wrong version exception drop",
"IPv4 small IHL exception drop",
"IPv4 with option exception drop",
"IPv4 header incomplete exception drop",
"IPv4 bad total length exception drop",
"IPv4 data incomplete exception drop",
"IPv4 fragment exception drop",
"IPv4 ping of death exception drop",
"IPv4 small TTL exception drop",
"IPv4 unknown IP protocol exception drop",
"IPv4 checksum error exception drop",
"IPv4 invalid SIP exception drop",
"IPv4 invalid DIP exception drop",
"IPv4 LAND attack exception drop",
"IPv4 AH header incomplete exception drop",
"IPv4 AH header cross 128-byte exception drop",
"IPv4 ESP header incomplete exception drop",
"IPv6 wrong version exception drop",
"IPv6 header incomplete exception drop",
"IPv6 bad total length exception drop",
"IPv6 data incomplete exception drop",
"IPv6 with extension header exception drop",
"IPv6 small hop limit exception drop",
"IPv6 invalid SIP exception drop",
"IPv6 invalid DIP exception drop",
"IPv6 LAND attack exception drop",
"IPv6 fragment exception drop",
"IPv6 ping of death exception drop",
"IPv6 with more than 2 extension headers exception drop",
"IPv6 unknown last next header exception drop",
"IPv6 mobility header incomplete exception drop",
"IPv6 mobility header cross 128-byte exception drop",
"IPv6 AH header incomplete exception drop",
"IPv6 AH header cross 128-byte exception drop",
"IPv6 ESP header incomplete exception drop",
"IPv6 ESP header cross 128-byte exception drop",
"IPv6 other extension header incomplete exception drop",
"IPv6 other extension header cross 128-byte exception drop",
"TCP header incomplete exception drop",
"TCP header cross 128-byte exception drop",
"TCP same SP and DP exception drop",
"TCP small data offset drop",
"TCP flags VALUE/MASK group 0 exception drop",
"TCP flags VALUE/MASK group 1 exception drop",
"TCP flags VALUE/MASK group 2 exception drop",
"TCP flags VALUE/MASK group 3 exception drop",
"TCP flags VALUE/MASK group 4 exception drop",
"TCP flags VALUE/MASK group 5 exception drop",
"TCP flags VALUE/MASK group 6 exception drop",
"TCP flags VALUE/MASK group 7 exception drop",
"TCP checksum error exception drop",
"UDP header incomplete exception drop",
"UDP header cross 128-byte exception drop",
"UDP same SP and DP exception drop",
"UDP bad length exception drop",
"UDP data incomplete exception drop",
"UDP checksum error exception drop",
"UDP-Lite header incomplete exception drop",
"UDP-Lite header cross 128-byte exception drop",
"UDP-Lite same SP and DP exception drop",
"UDP-Lite checksum coverage value 0-7 exception drop",
"UDP-Lite checksum coverage value too big exception drop",
"UDP-Lite checksum coverage value cross 128-byte exception drop",
"UDP-Lite checksum error exception drop",
"L3 multicast bridging action",
"L3 no route with Preheader NAT action",
"L3 no route with Preheader NAT action error configuration",
"L3 route action drop",
"L3 no route action drop",
"L3 no route next hop invalid action drop",
"L3 no route preheader action drop",
"L3 bridge action drop",
"L3 flow action drop",
"L3 flow miss action drop",
"L2 MRU checking fail drop",
"L2 MTU checking fail drop",
"L3 IP prefix broadcast drop",
"L3 MTU checking fail drop",
"L3 MRU checking fail drop",
"L3 ICMP redirect drop",
"Fake MAC header indicated packet not routing or bypass L3 edit drop",
"L3 IP route TTL zero drop",
"L3 flow service code loop drop",
"L3 flow de-accelerate drop",
"L3 flow source interface check fail drop",
"Flow toggle mismatch exception drop",
"MTU check exception if DF set drop",
"PPPoE multicast packet with IP routing enabled drop",
"IPv4 SG unkown drop",
"IPv6 SG unkown drop",
"ARP SG unkown drop",
"ND SG unkown drop",
"IPv4 SG violation drop",
"IPv6 SG violation drop",
"ARP SG violation drop",
"ND SG violation drop",
"L2 new MAC address drop",
"L2 hash violation drop",
"L2 station move drop",
"L2 learn limit drop",
"L2 SA lookup action drop",
"L2 DA lookup action drop",
"APP_CTRL action drop",
"Ingress VLAN filtering action drop",
"Ingress VLAN translation miss drop",
"Egress VLAN filtering drop",
"Pre-IPO entry hit action drop",
"Post-IPO entry hit action drop",
"Multicast SA or broadcast SA drop",
"No destination drop",
"STG ingress filtering drop",
"STG egress filtering drop",
"Source port filter drop",
"Trunk select fail drop",
"TX MAC disable drop",
"Ingress VLAN tag format drop",
"CRC error drop",
"PAUSE frame drop",
"Promisc drop",
"Isolation drop",
"Magagement packet APP_CTRL drop",
"Fake L2 protocol indicated packet not routing or bypass L3 edit drop",
"Policing drop"
#endif
};

static sw_error_t
adpt_hppe_debug_counter_set(a_uint32_t dev_id)
{
	union vlan_cnt_tbl_u vlan_cnt_tbl = {0};
	union pre_l2_cnt_tbl_u pre_l2_cnt_tbl = {0};
	union port_tx_drop_cnt_tbl_u port_tx_drop_cnt_tbl = {0};
	union eg_vsi_counter_tbl_u eg_vsi_counter_tbl = {0};
	union port_tx_counter_tbl_reg_u port_tx_counter_tbl = {0};
	union vp_tx_counter_tbl_reg_u vp_tx_counter_tbl = {0};
	union queue_tx_counter_tbl_u queue_tx_counter_tbl = {0};
	union epe_dbg_in_cnt_reg_u epe_dbg_in_cnt = {0};
	union epe_dbg_out_cnt_reg_u epe_dbg_out_cnt = {0};
	union vp_tx_drop_cnt_tbl_u vp_tx_drop_cnt_tbl = {0};
	union drop_cpu_cnt_tbl_u drop_cpu_cnt_tbl = {0};
#ifdef APPE
	union port_rx_cnt_tbl_u port_rx_cnt_tbl = {0};
	union phy_port_rx_cnt_tbl_u phy_port_rx_cnt_tbl = {0};
#endif
	a_uint32_t i;

	/* clear PRX DROP_CNT */
	for (i = 0; i < PPE_BM_PHY_PORT_OFFSET; i++)
		hppe_drop_cnt_drop_cnt_set(dev_id, i, 0);

	/* clear PRX DROP_PKT_STAT */
	for (i = 0; i < DROP_STAT_NUM; i++) {
		hppe_drop_stat_pkts_set(dev_id, i, 0);
		hppe_drop_stat_bytes_set(dev_id, i, 0);
	}

	/* clear IPR_PKT_NUM */
	for (i = 0; i < IPR_PKT_NUM_TBL_REG_MAX_ENTRY; i++) {
		hppe_ipr_pkt_num_tbl_reg_packets_set(dev_id, i, 0);
		hppe_ipr_byte_low_reg_reg_bytes_set(dev_id, i, 0);
		hppe_ipr_byte_high_reg_bytes_set(dev_id, i, 0);
	}

	/* clear VLAN_CNT_TBL */
	for (i = 0; i < VLAN_CNT_TBL_MAX_ENTRY; i++)
		hppe_vlan_cnt_tbl_set(dev_id, i, &vlan_cnt_tbl);

	/* clear PRE_L2_CNT_TBL */
	for (i = 0; i < PRE_L2_CNT_TBL_MAX_ENTRY; i++)
		hppe_pre_l2_cnt_tbl_set(dev_id, i, &pre_l2_cnt_tbl);

	/* clear PORT_TX_DROP_CNT_TBL */
	for (i = 0; i < PORT_TX_DROP_CNT_TBL_MAX_ENTRY; i++)
		hppe_port_tx_drop_cnt_tbl_set(dev_id, i, &port_tx_drop_cnt_tbl);

	/* clear EG_VSI_COUNTER_TBL */
	for (i = 0; i < EG_VSI_COUNTER_TBL_MAX_ENTRY; i++)
		hppe_eg_vsi_counter_tbl_set(dev_id, i, &eg_vsi_counter_tbl);

	/* clear PORT_TX_COUNTER_TBL */
	for (i = 0; i < PORT_TX_COUNTER_TBL_REG_MAX_ENTRY; i++)
		hppe_port_tx_counter_tbl_reg_set(dev_id, i, &port_tx_counter_tbl);

	/* clear VP_TX_COUNTER_TBL */
	for (i = 0; i < VP_TX_COUNTER_TBL_REG_MAX_ENTRY; i++)
		hppe_vp_tx_counter_tbl_reg_set(dev_id, i, &vp_tx_counter_tbl);

	/* clear QUEUE_TX_COUNTER_TBL */
	for (i = 0; i < QUEUE_TX_COUNTER_TBL_MAX_ENTRY; i++)
		hppe_queue_tx_counter_tbl_set(dev_id, i, &queue_tx_counter_tbl);

	/* clear EPE_DBG_IN_CNT & EPE_DBG_OUT_CNT */
	hppe_epe_dbg_in_cnt_reg_set(dev_id, &epe_dbg_in_cnt);
	hppe_epe_dbg_out_cnt_reg_set(dev_id, &epe_dbg_out_cnt);

	/* clear VP_TX_DROP_CNT_TBL */
	for (i = 0; i < VP_TX_DROP_CNT_TBL_MAX_ENTRY; i++)
		hppe_vp_tx_drop_cnt_tbl_set(dev_id, i, &vp_tx_drop_cnt_tbl);

	/* clear DROP_CPU_CNT_TBL */
	for (i = 0; i < DROP_CPU_CNT_TBL_MAX_ENTRY; i++)
		hppe_drop_cpu_cnt_tbl_set(dev_id, i, &drop_cpu_cnt_tbl);

#ifdef APPE
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		/* clear VP_RX_COUNTER_TBL and VP_RX_DROP_CNT_TBL */
		for (i = 0; i < PORT_RX_CNT_TBL_NUM; i++)
			appe_port_rx_cnt_tbl_set (dev_id, i, &port_rx_cnt_tbl);
		/* clear PORT_RX_COUNTER_TBL and PORT_RX_DROP_CNT_TBL */
		for (i = 0; i < PHY_PORT_RX_CNT_TBL_NUM; i++)
			appe_phy_port_rx_cnt_tbl_set (dev_id, i, &phy_port_rx_cnt_tbl);
	}
#endif
	return SW_OK;
}

static void
adpt_hppe_debug_prx_drop_cnt_get(a_uint32_t dev_id)
{
	a_uint32_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "PRX_DROP_CNT RX:");
	for (i = 0; i < PPE_BM_PHY_PORT_OFFSET; i++)
	{
		hppe_drop_cnt_drop_cnt_get(dev_id, i, &value);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15u(port=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_prx_drop_pkt_stat_get(a_uint32_t dev_id, a_bool_t show_type)
{
	a_uint32_t value32;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "PRX_DROP_PKT_STAT RX:");
	for (i = 0; i < DROP_STAT_NUM; i++)
	{
		if (show_type == A_FALSE)
		{
			hppe_drop_stat_pkts_get(dev_id, i, &value32);
			value = (a_uint64_t)value32;
		}
		else
			hppe_drop_stat_bytes_get(dev_id, i, &value);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(port=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_ipx_pkt_num_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union ipr_pkt_num_tbl_reg_u ipr_pkt_num_tbl_reg;
	union ipr_byte_low_reg_reg_u ipr_byte_low_reg;
	union ipr_byte_high_reg_u ipr_byte_high_reg;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "IPR_PKT_NUM RX:");
	for (i = 0; i < IPR_PKT_NUM_TBL_REG_MAX_ENTRY; i++)
	{
		hppe_ipr_pkt_num_tbl_reg_get(dev_id, i, &ipr_pkt_num_tbl_reg);
		hppe_ipr_byte_low_reg_reg_get(dev_id, i, &ipr_byte_low_reg);
		hppe_ipr_byte_high_reg_get(dev_id, i, &ipr_byte_high_reg);
		if (show_type == A_FALSE)
			value = (a_uint64_t)ipr_pkt_num_tbl_reg.bf.packets;
		else
			value = ipr_byte_low_reg.bf.bytes | ((a_uint64_t)ipr_byte_high_reg.bf.bytes << 32);

		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(port=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_vlan_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union vlan_cnt_tbl_u vlan_cnt_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "VLAN_CNT_TBL RX:");
	for (i = 0; i < VLAN_CNT_TBL_MAX_ENTRY; i++)
	{
		hppe_vlan_cnt_tbl_get(dev_id, i, &vlan_cnt_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)vlan_cnt_tbl.bf.rx_pkt_cnt;
		else
			value = vlan_cnt_tbl.bf.rx_byte_cnt_0 | ((a_uint64_t)vlan_cnt_tbl.bf.rx_byte_cnt_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(vsi=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_pre_l2_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union pre_l2_cnt_tbl_u pre_l2_cnt_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "PRE_L2_CNT_TBL RX:");
	for (i = 0; i < PRE_L2_CNT_TBL_MAX_ENTRY; i++)
	{
		hppe_pre_l2_cnt_tbl_get(dev_id, i, &pre_l2_cnt_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)pre_l2_cnt_tbl.bf.rx_pkt_cnt;
		else
			value = pre_l2_cnt_tbl.bf.rx_byte_cnt_0 | ((a_uint64_t)pre_l2_cnt_tbl.bf.rx_byte_cnt_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(vsi=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");

	sign = tags = 0;
	printk("%-35s", "PRE_L2_CNT_TBL RX_DROP:");
	for (i = 0; i < PRE_L2_CNT_TBL_MAX_ENTRY; i++)
	{
		hppe_pre_l2_cnt_tbl_get(dev_id, i, &pre_l2_cnt_tbl);
		if (show_type == A_FALSE)
			value = pre_l2_cnt_tbl.bf.rx_drop_pkt_cnt_0 | ((a_uint64_t)pre_l2_cnt_tbl.bf.rx_drop_pkt_cnt_1 << 24);
		else
			value = pre_l2_cnt_tbl.bf.rx_drop_byte_cnt_0 | ((a_uint64_t)pre_l2_cnt_tbl.bf.rx_drop_byte_cnt_1 << 24);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(vsi=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void adpt_hppe_debug_port_tx_drop_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union port_tx_drop_cnt_tbl_u port_tx_drop_cnt_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "PORT_TX_DROP_CNT_TBL TX_DROP:");
	for (i = 0; i < PORT_TX_DROP_CNT_TBL_MAX_ENTRY; i++)
	{
		hppe_port_tx_drop_cnt_tbl_get(dev_id, i, &port_tx_drop_cnt_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)port_tx_drop_cnt_tbl.bf.tx_drop_pkt_cnt;
		else
			value = port_tx_drop_cnt_tbl.bf.tx_drop_byte_cnt_0 | ((a_uint64_t)port_tx_drop_cnt_tbl.bf.tx_drop_byte_cnt_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(port=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_eg_vsi_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union eg_vsi_counter_tbl_u eg_vsi_counter_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "EG_VSI_COUNTER_TBL TX:");
	for (i = 0; i < EG_VSI_COUNTER_TBL_MAX_ENTRY; i++)
	{
		hppe_eg_vsi_counter_tbl_get(dev_id, i, &eg_vsi_counter_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)eg_vsi_counter_tbl.bf.tx_packets;
		else
			value = eg_vsi_counter_tbl.bf.tx_bytes_0 | ((a_uint64_t)eg_vsi_counter_tbl.bf.tx_bytes_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(vsi=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_port_tx_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union port_tx_counter_tbl_reg_u port_tx_counter_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "PORT_TX_COUNTER_TBL TX:");
	for (i = 0; i < PORT_TX_COUNTER_TBL_REG_MAX_ENTRY; i++)
	{
		hppe_port_tx_counter_tbl_reg_get(dev_id, i, &port_tx_counter_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)port_tx_counter_tbl.bf.tx_packets;
		else
			value = port_tx_counter_tbl.bf.tx_bytes_0 | ((a_uint64_t)port_tx_counter_tbl.bf.tx_bytes_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(port=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_vp_tx_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union vp_tx_counter_tbl_reg_u vp_tx_counter_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "VP_TX_COUNTER_TBL TX:");
	for (i = 0; i < VP_TX_COUNTER_TBL_REG_MAX_ENTRY; i++)
	{
		hppe_vp_tx_counter_tbl_reg_get(dev_id, i, &vp_tx_counter_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)vp_tx_counter_tbl.bf.tx_packets;
		else
			value = vp_tx_counter_tbl.bf.tx_bytes_0 | ((a_uint64_t)vp_tx_counter_tbl.bf.tx_bytes_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(port=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_queue_tx_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union queue_tx_counter_tbl_u queue_tx_counter_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "QUEUE_TX_COUNTER_TBL TX:");
	for (i = 0; i < QUEUE_TX_COUNTER_TBL_MAX_ENTRY; i++)
	{
		hppe_queue_tx_counter_tbl_get(dev_id, i, &queue_tx_counter_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)queue_tx_counter_tbl.bf.tx_packets;
		else
			value = queue_tx_counter_tbl.bf.tx_bytes_0 | ((a_uint64_t)queue_tx_counter_tbl.bf.tx_bytes_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(queue=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_vp_tx_drop_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union vp_tx_drop_cnt_tbl_u vp_tx_drop_cnt_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "VP_TX_DROP_CNT_TBL TX_DROP:");
	for (i = 0; i < VP_TX_DROP_CNT_TBL_MAX_ENTRY; i++)
	{
		hppe_vp_tx_drop_cnt_tbl_get(dev_id, i, &vp_tx_drop_cnt_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)vp_tx_drop_cnt_tbl.bf.tx_drop_pkt_cnt;
		else
			value = vp_tx_drop_cnt_tbl.bf.tx_drop_byte_cnt_0 | ((a_uint64_t)vp_tx_drop_cnt_tbl.bf.tx_drop_byte_cnt_1 << 32);
		if (value > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			printk(KERN_CONT "%15llu(port=%04d)", value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_cpu_code_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union drop_cpu_cnt_tbl_u drop_cpu_cnt_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "CPU_CODE_CNT_TBL:");
	for (i = 0; i < CPU_CODE_CNT_TBL_MAX_ENTRY; i++)
	{
		hppe_drop_cpu_cnt_tbl_get(dev_id, i, &drop_cpu_cnt_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)drop_cpu_cnt_tbl.bf.pkt_cnt;
		else
			value = drop_cpu_cnt_tbl.bf.byte_cnt_0 | ((a_uint64_t)drop_cpu_cnt_tbl.bf.byte_cnt_1 << 32);
		if (value > 0)
		{
			printk(KERN_CONT "\n");
			printk(KERN_CONT "%-35s", "");
			if (i >=0 && i <= 70)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i], i);
			else if (i >= 79 && i <= 92)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 8], i);
			else if (i >= 97 && i <= 102)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 12], i);
			else if (i >= 107 && i <= 110)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 16], i);
			else if (i >= 113 && i <= 127)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 18], i);
			else if (i >= 136 && i <= 143)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 26], i);
			else if (i >= 148 && i <= 174)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 30], i);
			else if (i >= 178 && i <= 180)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 33], i);
#ifdef APPE
			else if (i >= 93 && i <= 95)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i + 57], i);
			else if (i == 104)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i + 49], i);
			else if (i >= 181 && i <= 198)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 27], i);
			else if (i >= 210 && i <= 232)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 38], i);
#endif
			else if (i >= 254 && i <= 255)
				printk(KERN_CONT "%15llu(%s),cpucode:%d", value, cpucode[i - 106], i);
			else
				printk(KERN_CONT "%15llu(Reserved),cpucode:%d", value, i);
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_hppe_debug_drop_cpu_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	union drop_cpu_cnt_tbl_u drop_cpu_cnt_tbl;
	a_uint64_t value;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "DROP_CPU_CNT_TBL:");
	for (i = CPU_CODE_CNT_TBL_MAX_ENTRY; i < DROP_CPU_CNT_TBL_MAX_ENTRY; i++)
	{
		hppe_drop_cpu_cnt_tbl_get(dev_id, i, &drop_cpu_cnt_tbl);
		if (show_type == A_FALSE)
			value = (a_uint64_t)drop_cpu_cnt_tbl.bf.pkt_cnt;
		else
			value = drop_cpu_cnt_tbl.bf.byte_cnt_0 | ((a_uint64_t)drop_cpu_cnt_tbl.bf.byte_cnt_1 << 32);
		if (value > 0)
		{
			printk(KERN_CONT "\n");
			printk(KERN_CONT "%-35s", "");
			printk(KERN_CONT "%15llu(port=%d:%s),dropcode:%d", value, (i - 256) % 8, dropcode[(i - 256) / 8], (i-256)/8);
		}
	}
	printk(KERN_CONT "\n");
}

#ifdef APPE
static void
adpt_appe_debug_vp_rx_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	a_uint64_t rx_value = 0;
	a_uint32_t rx_pkt = 0;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "VP_RX_COUNTER_TBL RX:");
	for (i = 0; i < PORT_RX_CNT_TBL_NUM; i++)
	{
		if (show_type == A_FALSE)
			appe_port_rx_cnt_tbl_rx_pkt_cnt_get(dev_id, i, &rx_pkt);
		else
			appe_port_rx_cnt_tbl_rx_byte_cnt_get(dev_id, i, &rx_value);
		if (rx_value > 0 || rx_pkt > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			if (show_type == A_FALSE)
				printk(KERN_CONT "%15u(port_id=%04d)", rx_pkt, i);
			else
				printk(KERN_CONT "%15llu(port_id=%04d)", rx_value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_appe_debug_vp_rx_drop_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	a_uint64_t rx_value = 0;
	a_uint32_t rx_pkt = 0;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "VP_RX_DROP_CNT_TBL RX_DROP:");
	for (i = 0; i < PORT_RX_CNT_TBL_NUM; i++)
	{
		if (show_type == A_FALSE)
			appe_port_rx_cnt_tbl_rx_drop_pkt_cnt_get(dev_id, i, &rx_pkt);
		else
			appe_port_rx_cnt_tbl_rx_drop_byte_cnt_get(dev_id, i, &rx_value);
		if (rx_value > 0 || rx_pkt > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			if (show_type == A_FALSE)
				printk(KERN_CONT "%15u(port_id=%04d)", rx_pkt, i);
			else
				printk(KERN_CONT "%15llu(port_id=%04d)", rx_value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_appe_debug_port_rx_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	a_uint64_t rx_value = 0;
	a_uint32_t rx_pkt = 0;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "PORT_RX_COUNTER_TBL RX:");
	for (i = 0; i < PHY_PORT_RX_CNT_TBL_NUM; i++)
	{
		if (show_type == A_FALSE)
			appe_phy_port_rx_cnt_tbl_rx_pkt_cnt_get(dev_id, i, &rx_pkt);
		else
			appe_phy_port_rx_cnt_tbl_rx_byte_cnt_get(dev_id, i, &rx_value);
		if (rx_value > 0 || rx_pkt > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			if (show_type == A_FALSE)
				printk(KERN_CONT "%15u(port_id=%04d)", rx_pkt, i);
			else
				printk(KERN_CONT "%15llu(port_id=%04d)", rx_value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

static void
adpt_appe_debug_port_rx_drop_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	a_uint64_t rx_value = 0;
	a_uint32_t rx_pkt = 0;
	int i, tags, sign;

	sign = tags = 0;
	printk("%-35s", "PORT_RX_DROP_CNT_TBL RX_DROP:");
	for (i = 0; i < PHY_PORT_RX_CNT_TBL_NUM; i++)
	{
		if (show_type == A_FALSE)
			appe_phy_port_rx_cnt_tbl_rx_drop_pkt_cnt_get(dev_id, i, &rx_pkt);
		else
			appe_phy_port_rx_cnt_tbl_rx_drop_byte_cnt_get(dev_id, i,
				&rx_value);
		if (rx_value > 0 || rx_pkt > 0)
		{
			if (sign) {
				printk(KERN_CONT "\n");
				printk(KERN_CONT "%-35s", "");
			}
			sign = 0;
			if (show_type == A_FALSE)
				printk(KERN_CONT "%15u(port_id=%04d)", rx_pkt, i);
			else
				printk(KERN_CONT "%15llu(port_id=%04d)", rx_value, i);
			if (++tags % 3 == 0)
				sign = 1;
		}
	}
	printk(KERN_CONT "\n");
}

#endif
/* if show_type = A_FALSE, show packets.
 * if show_type = A_TRUE, show bytes.
 */
static sw_error_t
adpt_hppe_debug_counter_get(a_uint32_t dev_id, a_bool_t show_type)
{
	/* show PRX DROP_CNT */
	adpt_hppe_debug_prx_drop_cnt_get(dev_id);

	/* show PRX DROP_PKT_STAT */
	adpt_hppe_debug_prx_drop_pkt_stat_get(dev_id, show_type);

	/* show IPR_PKT_NUM */
	adpt_hppe_debug_ipx_pkt_num_get(dev_id, show_type);

	/* show VLAN_CNT_TBL */
	adpt_hppe_debug_vlan_counter_get(dev_id, show_type);

	/* show PRE_L2_CNT_TBL */
	adpt_hppe_debug_pre_l2_counter_get(dev_id, show_type);

	/* show PORT_TX_DROP_CNT_TBL */
	adpt_hppe_debug_port_tx_drop_counter_get(dev_id, show_type);

	/* show EG_VSI_COUNTER_TBL */
	adpt_hppe_debug_eg_vsi_counter_get(dev_id, show_type);

	/* show PORT_TX_COUNTER_TBL */
	adpt_hppe_debug_port_tx_counter_get(dev_id, show_type);

	/* show VP_TX_COUNTER_TBL */
	adpt_hppe_debug_vp_tx_counter_get(dev_id, show_type);

	/* show QUEUE_TX_COUNTER_TBL */
	adpt_hppe_debug_queue_tx_counter_get(dev_id, show_type);

	/* show VP_TX_DROP_CNT_TBL */
	adpt_hppe_debug_vp_tx_drop_counter_get(dev_id, show_type);

	/* show CPU_CODE_CNT */
	adpt_hppe_debug_cpu_code_counter_get(dev_id, show_type);

	/* show DROP_CPU_CNT_TBL */
	adpt_hppe_debug_drop_cpu_counter_get(dev_id, show_type);
#ifdef APPE
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		/* show VP_PORT_RX_COUNTER_TBL*/
		adpt_appe_debug_vp_rx_counter_get(dev_id, show_type);
		/* show VP_PORT_RX_DROP_CNT_TBL*/
		adpt_appe_debug_vp_rx_drop_counter_get(dev_id, show_type);
		/* show PORT_RX_COUNTER_TBL*/
		adpt_appe_debug_port_rx_counter_get(dev_id, show_type);
		/* show PORT_RX_DROP_CNT_TBL*/
		adpt_appe_debug_port_rx_drop_counter_get(dev_id, show_type);
	}
#endif
	return SW_OK;
}

sw_error_t adpt_hppe_misc_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL) {
		return SW_FAIL;
	}

	p_adpt_api->adpt_debug_counter_set = adpt_hppe_debug_counter_set;
	p_adpt_api->adpt_debug_counter_get = adpt_hppe_debug_counter_get;

	return SW_OK;
}

/**
 * @}
 */
