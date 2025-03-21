/*
 * Copyright (c) 2012, 2014-2021, The Linux Foundation. All rights reserved.
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

#include "ssdk_init.h"
#include "ssdk_dts.h"
#include "adpt.h"
#include "adpt_hppe.h"
#include "fal.h"
#include "ref_vsi.h"
#include "ssdk_clk.h"
#include "hsl_phy.h"
#include <linux/in.h>
#ifdef IN_LED
#include "ssdk_led.h"
#endif

#if defined(IN_VSI)
sw_error_t qca_hppe_vsi_hw_init(a_uint32_t dev_id)
{
       return ppe_vsi_init(dev_id);
}
#endif

#if defined(IN_FDB)
sw_error_t qca_hppe_fdb_hw_init(a_uint32_t dev_id)
{
	a_uint32_t port = 0;
	adpt_api_t *p_api;

	SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));
	SW_RTN_ON_NULL(p_api->adpt_port_bridge_txmac_set);

	for(port = SSDK_PHYSICAL_PORT0; port <= SSDK_PHYSICAL_PORT7; port++) {
//		if(port == SSDK_PHYSICAL_PORT0) {
			fal_fdb_port_learning_ctrl_set(dev_id, port, A_FALSE, FAL_MAC_FRWRD);
			fal_fdb_port_stamove_ctrl_set(dev_id, port, A_FALSE, FAL_MAC_FRWRD);
//		} else {
//			fal_fdb_port_learning_ctrl_set(dev_id, port, A_FALSE, FAL_MAC_FRWRD);
//			fal_fdb_port_stamove_ctrl_set(dev_id, port, A_FALSE, FAL_MAC_FRWRD);
//		}
		fal_portvlan_member_update(dev_id, port, 0x7f);
		if (port == SSDK_PHYSICAL_PORT0 || port == SSDK_PHYSICAL_PORT7 ||
		(hsl_port_feature_get(dev_id, port, PHY_F_FORCE) == A_TRUE)) {
			p_api->adpt_port_bridge_txmac_set(dev_id, port, A_TRUE);
		} else {
			p_api->adpt_port_bridge_txmac_set(dev_id, port, A_FALSE);
		}
		fal_port_promisc_mode_set(dev_id, port, A_TRUE);
	}

	fal_fdb_aging_ctrl_set(dev_id, A_FALSE);
	fal_fdb_learning_ctrl_set(dev_id, A_FALSE);

	return SW_OK;
}
#endif

#if defined(IN_CTRLPKT)
#define RFDB_PROFILE_ID_STP 31
#define RFDB_PROFILE_ID_ARP 30
#define APPE_CPU_CODE_CTRL_NUM	256
sw_error_t qca_hppe_ctlpkt_hw_init(a_uint32_t dev_id)
{
	fal_mac_addr_t mcast_mac_addr;
	fal_ctrlpkt_action_t ctrlpkt_action;
	fal_ctrlpkt_profile_t ctrlpkt_profile;
	sw_error_t rv = SW_OK;
	a_uint8_t mac_id = 0, mac_num = 0;
	a_uint8_t *mac = NULL;
	a_uint32_t rfdb_profile_bmp = 0, rfdb_profile_index = 0;
#if defined(APPE)
	a_uint32_t cpu_code = 0;

	if (adpt_chip_type_get(dev_id) == CHIP_APPE) {
		while (cpu_code < APPE_CPU_CODE_CTRL_NUM) {
			rv = fal_mgmtctrl_tunnel_decap_set(dev_id, cpu_code, A_TRUE);
			SW_RTN_ON_ERROR(rv);
			cpu_code++;
		}
	}
#endif

	memset(&ctrlpkt_action, 0, sizeof(ctrlpkt_action));
	memset(&ctrlpkt_profile, 0, sizeof(ctrlpkt_profile));
	memset(&mcast_mac_addr, 0, sizeof(mcast_mac_addr));

	mcast_mac_addr.uc[0] = 0x01;
	mcast_mac_addr.uc[1] = 0x80;
	mcast_mac_addr.uc[2] = 0xc2;
	rv = fal_mgmtctrl_rfdb_profile_set(dev_id, RFDB_PROFILE_ID_STP,
			&mcast_mac_addr);
	SW_RTN_ON_ERROR(rv);

	ctrlpkt_action.action = FAL_MAC_RDT_TO_CPU;
	ctrlpkt_action.in_stp_bypass = A_TRUE;

	ctrlpkt_profile.action = ctrlpkt_action;
	ctrlpkt_profile.port_map = qca_ssdk_port_bmp_get(dev_id);
	ctrlpkt_profile.rfdb_profile_bitmap = (1 << RFDB_PROFILE_ID_STP);
	rv = fal_mgmtctrl_ctrlpkt_profile_add(dev_id, &ctrlpkt_profile);
	SW_RTN_ON_ERROR(rv);

	/*
	 * Redirect ARP response packet to CPU port with a valid CPU code.
	 * queue management assigns the highest priority queue based on this
	 * CPU code.
	 */
	mac_id = 0;
	mac_num = ssdk_intf_mac_num_get();
	if (mac_num == 0)
		return SW_OK;

	while (mac_id < mac_num) {
		/*
		 * The MAC of all ports(ethx) should be involved, since the MAC
		 * of bridge can be any one of them.
		 */
		mac = ssdk_intf_macaddr_get(mac_id);
		memcpy(mcast_mac_addr.uc, mac, 6);

		rfdb_profile_index = RFDB_PROFILE_ID_ARP - mac_id;
		rv = fal_mgmtctrl_rfdb_profile_set(dev_id, rfdb_profile_index, &mcast_mac_addr);
		SW_RTN_ON_ERROR(rv);

		rfdb_profile_bmp |= BIT(rfdb_profile_index);
		mac_id++;
	}

	memset(&ctrlpkt_action, 0, sizeof(ctrlpkt_action));
	memset(&ctrlpkt_profile, 0, sizeof(ctrlpkt_profile));

	ctrlpkt_action.action = FAL_MAC_RDT_TO_CPU;
	ctrlpkt_profile.action = ctrlpkt_action;
	ctrlpkt_profile.port_map = qca_ssdk_port_bmp_get(dev_id);
	ctrlpkt_profile.rfdb_profile_bitmap = rfdb_profile_bmp;
	ctrlpkt_profile.protocol_types.mgt_arp_rep = A_TRUE;

	rv = fal_mgmtctrl_ctrlpkt_profile_add(dev_id, &ctrlpkt_profile);

	return rv;
}
#endif

#if defined(IN_PORTCONTROL)
static sw_error_t
qca_hppe_portctrl_hw_init(a_uint32_t dev_id)
{
	a_uint32_t i = 0, port_max = SSDK_PHYSICAL_PORT7;
	a_bool_t force_port = 0, sfp_port = 0;
#if defined(CPPE)
	fal_loopback_config_t loopback_cfg;
#endif
	fal_port_eee_cfg_t port_eee_cfg = {0};

	if(adpt_chip_revision_get(dev_id) == CPPE_REVISION) {
		port_max = SSDK_PHYSICAL_PORT6;
	} else {
		port_max = SSDK_PHYSICAL_PORT7;
	}
	for(i = SSDK_PHYSICAL_PORT1; i < port_max; i++) {
		qca_hppe_port_mac_type_set(dev_id, i, PORT_GMAC_TYPE);
		fal_port_txmac_status_set (dev_id, i, A_FALSE);
		fal_port_rxmac_status_set (dev_id, i, A_FALSE);
		force_port = hsl_port_feature_get(dev_id, i, PHY_F_FORCE);
		if(force_port)
		{
			fal_port_rxfc_status_set(dev_id, i, A_FALSE);
			fal_port_txfc_status_set(dev_id, i, A_FALSE);
		}
		else
		{
			fal_port_rxfc_status_set(dev_id, i, A_TRUE);
			fal_port_txfc_status_set(dev_id, i, A_TRUE);
		}
		fal_port_max_frame_size_set(dev_id, i, SSDK_MAX_FRAME_SIZE);
		fal_port_interface_eee_cfg_get(dev_id, i, &port_eee_cfg);
		port_eee_cfg.enable = A_FALSE;
		port_eee_cfg.lpi_tx_enable = A_FALSE;
		fal_port_interface_eee_cfg_set(dev_id, i, &port_eee_cfg);
		qca_mac_port_status_init(dev_id, i);
#ifdef IN_LED
		ssdk_led_init(dev_id, i);
#endif
	}

	for(i = SSDK_PHYSICAL_PORT5; i < port_max; i++) {
		qca_hppe_port_mac_type_set(dev_id, i, PORT_XGMAC_TYPE);
		fal_port_txmac_status_set (dev_id, i, A_FALSE);
		fal_port_rxmac_status_set (dev_id, i, A_FALSE);
		force_port = hsl_port_feature_get(dev_id, i, PHY_F_FORCE);
		if(force_port)
		{
			fal_port_rxfc_status_set(dev_id, i, A_FALSE);
			fal_port_txfc_status_set(dev_id, i, A_FALSE);
		}
		sfp_port = hsl_port_feature_get(dev_id, i, PHY_F_SFP);
		if(sfp_port)
		{
			fal_port_rxfc_status_set(dev_id, i, A_TRUE);
			fal_port_txfc_status_set(dev_id, i, A_TRUE);
		}
		fal_port_max_frame_size_set(dev_id, i, SSDK_MAX_FRAME_SIZE);
	}

#if defined(CPPE)
	if (adpt_chip_revision_get(dev_id) == CPPE_REVISION) {
		loopback_cfg.enable = A_TRUE;
		loopback_cfg.crc_stripped = A_TRUE;
		loopback_cfg.loopback_rate = FAL_DEFAULT_LOOPBACK_RATE;  /* Mpps */
		fal_switch_port_loopback_set(dev_id, SSDK_PHYSICAL_PORT6,
				&loopback_cfg);
		fal_port_max_frame_size_set(dev_id, SSDK_PHYSICAL_PORT6,
				SSDK_MAX_FRAME_SIZE);
	}
#endif
	return SW_OK;
}
#endif

#if defined(IN_POLICER)
static sw_error_t
qca_hppe_policer_hw_init(a_uint32_t dev_id)
{
	a_uint32_t i = 0;
	fal_policer_frame_type_t frame_type;

	fal_policer_timeslot_set(dev_id, HPPE_POLICER_TIMESLOT_DFT);

	for (i = SSDK_PHYSICAL_PORT0; i <= SSDK_PHYSICAL_PORT7; i++) {
		fal_port_policer_compensation_byte_set(dev_id, i, 4);
	}

	/* bypass policer for dropped frame */
	frame_type = FAL_FRAME_DROPPED;
	fal_policer_bypass_en_set(dev_id, frame_type, A_TRUE);

	return SW_OK;
}
#endif

#if defined(IN_SHAPER)
static sw_error_t
qca_hppe_shaper_hw_init(a_uint32_t dev_id)
{
	fal_shaper_token_number_t port_token_number, queue_token_number;
	fal_shaper_token_number_t flow_token_number;
	a_uint32_t i = 0;

	port_token_number.c_token_number_negative_en = A_FALSE;
	port_token_number.c_token_number = HPPE_MAX_C_TOKEN_NUM;
	queue_token_number.c_token_number_negative_en = A_FALSE;
	queue_token_number.c_token_number = HPPE_MAX_C_TOKEN_NUM;
	queue_token_number.e_token_number_negative_en = A_FALSE;
	queue_token_number.e_token_number = HPPE_MAX_E_TOKEN_NUM;
	flow_token_number.c_token_number_negative_en = A_FALSE;
	flow_token_number.c_token_number = HPPE_MAX_C_TOKEN_NUM;
	flow_token_number.e_token_number_negative_en = A_FALSE;
	flow_token_number.e_token_number = HPPE_MAX_E_TOKEN_NUM;

	for (i = SSDK_PHYSICAL_PORT0; i <= SSDK_PHYSICAL_PORT7; i++) {
		fal_port_shaper_token_number_set(dev_id, i, &port_token_number);
	}

	for(i = 0; i < SSDK_L0SCHEDULER_CFG_MAX; i ++) {
		fal_queue_shaper_token_number_set(dev_id, i, &queue_token_number);
	}

	for(i = 0; i < SSDK_L1SCHEDULER_CFG_MAX; i ++) {
		fal_flow_shaper_token_number_set(dev_id, i, &flow_token_number);
	}

	fal_port_shaper_timeslot_set(dev_id, HPPE_PORT_SHAPER_TIMESLOT_DFT);
	fal_flow_shaper_timeslot_set(dev_id, HPPE_FLOW_SHAPER_TIMESLOT_DFT);
	fal_queue_shaper_timeslot_set(dev_id, HPPE_QUEUE_SHAPER_TIMESLOT_DFT);
	fal_shaper_ipg_preamble_length_set(dev_id,
				HPPE_SHAPER_IPG_PREAMBLE_LEN_DFT);

	return SW_OK;
}
#endif

#if defined(IN_PORTVLAN)
sw_error_t
qca_hppe_portvlan_hw_init(a_uint32_t dev_id)
{
	a_uint32_t port_id = 0, vsi_idx = 0;
	fal_global_qinq_mode_t global_qinq_mode;
	fal_port_qinq_role_t port_qinq_role;
	fal_tpid_t in_eg_tpid;
	fal_vlantag_egress_mode_t vlantag_eg_mode;

	aos_mem_zero(&global_qinq_mode, sizeof(global_qinq_mode));
	aos_mem_zero(&port_qinq_role, sizeof(port_qinq_role));
	aos_mem_zero(&in_eg_tpid, sizeof(in_eg_tpid));
	aos_mem_zero(&vlantag_eg_mode, sizeof(vlantag_eg_mode));

	/* configure ingress/egress global QinQ mode as ctag/ctag */
	global_qinq_mode.mask = 0x3;
	global_qinq_mode.ingress_mode = FAL_QINQ_CTAG_MODE;
	global_qinq_mode.egress_mode = FAL_QINQ_CTAG_MODE;
	fal_global_qinq_mode_set(dev_id, &global_qinq_mode);

	/* configure port0, port7 ingress/egress QinQ role as core/core */
	port_qinq_role.mask = 0x3;
	port_qinq_role.ingress_port_role = FAL_QINQ_CORE_PORT;
	port_qinq_role.egress_port_role = FAL_QINQ_CORE_PORT;
	fal_port_qinq_mode_set(dev_id, SSDK_PHYSICAL_PORT0, &port_qinq_role);
	fal_port_qinq_mode_set(dev_id, SSDK_PHYSICAL_PORT7, &port_qinq_role);
	/* configure port1 - port6 ingress/egress QinQ role as edge/edge */
	port_qinq_role.mask = 0x3;
	port_qinq_role.ingress_port_role = FAL_QINQ_EDGE_PORT;
	port_qinq_role.egress_port_role = FAL_QINQ_EDGE_PORT;
	for (port_id = SSDK_PHYSICAL_PORT1; port_id <= SSDK_PHYSICAL_PORT6;
			port_id++) {
		fal_port_qinq_mode_set(dev_id, port_id, &port_qinq_role);
	}

	/* configure ingress and egress stpid/ctpid as 0x88a8/0x8100 */
	in_eg_tpid.mask = 0x3;
	in_eg_tpid.ctpid = FAL_DEF_VLAN_CTPID;
	in_eg_tpid.stpid = FAL_DEF_VLAN_STPID;
	fal_ingress_tpid_set(dev_id, &in_eg_tpid);
	fal_egress_tpid_set(dev_id, &in_eg_tpid);

	/* configure the port0 - port7 of vsi0 - vsi31 to unmodified */
	for (vsi_idx = 0; vsi_idx <= FAL_VSI_MAX; vsi_idx++) {
		for (port_id = SSDK_PHYSICAL_PORT0;
			port_id <= SSDK_PHYSICAL_PORT7;	port_id++) {
			fal_port_vsi_egmode_set(dev_id, vsi_idx,
						port_id, FAL_EG_UNMODIFIED);
		}
	}

	vlantag_eg_mode.mask = 0x3;
	vlantag_eg_mode.stag_mode = FAL_EG_UNTOUCHED;
	vlantag_eg_mode.ctag_mode = FAL_EG_UNTOUCHED;
	/*stag/ctag egress mode as untouched/untouched*/
	fal_port_vlantag_egmode_set(dev_id, SSDK_PHYSICAL_PORT0,
				&vlantag_eg_mode);
	fal_port_vlantag_egmode_set(dev_id, SSDK_PHYSICAL_PORT7,
				&vlantag_eg_mode);
	/*vsi tag mode control to disable*/
	fal_port_vlantag_vsi_egmode_enable(dev_id, SSDK_PHYSICAL_PORT0,
				A_FALSE);
	fal_port_vlantag_vsi_egmode_enable(dev_id, SSDK_PHYSICAL_PORT7,
				A_FALSE);
	/*ingress vlan translation mismatched command as forward*/
	fal_port_vlan_xlt_miss_cmd_set(dev_id, SSDK_PHYSICAL_PORT0,
				FAL_MAC_FRWRD);

	vlantag_eg_mode.stag_mode = FAL_EG_UNMODIFIED;
	vlantag_eg_mode.ctag_mode = FAL_EG_UNMODIFIED;
	for (port_id = SSDK_PHYSICAL_PORT1; port_id <= SSDK_PHYSICAL_PORT6;
			port_id++) {
		/*ingress vlan translation mismatched command as forward*/
		fal_port_vlan_xlt_miss_cmd_set(dev_id, port_id, FAL_MAC_FRWRD);
		/*vsi tag mode control to enable*/
		fal_port_vlantag_vsi_egmode_enable(dev_id, port_id, A_TRUE);
		/*stag/ctag egress mode as unmodified/unmodified*/
		fal_port_vlantag_egmode_set(dev_id, port_id, &vlantag_eg_mode);
	}

	return SW_OK;
}
#endif

#if defined(IN_BM) && defined(IN_QOS)
fal_port_scheduler_cfg_t port_scheduler0_tbl[] = {
	{0xee, 6, 0, 0, 0},
	{0xde, 4, 5, 0, 0},
	{0x9f, 0, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0x7e, 6, 7, 0, 0},
	{0x5f, 0, 5, 0, 0},
	{0x9f, 7, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0xfc, 6, 1, 0, 0},
	{0xdd, 0, 5, 0, 0},
	{0xde, 1, 0, 0, 0},
	{0xbe, 5, 6, 0, 0},
	{0xbb, 0, 2, 0, 0},
	{0xdb, 6, 5, 0, 0},
	{0xde, 2, 0, 0, 0},
	{0xbe, 5, 6, 0, 0},
	{0x3f, 0, 7, 0, 0},
	{0x7e, 6, 0, 0, 0},
	{0xde, 7, 5, 0, 0},
	{0x9f, 0, 6, 0, 0},
	{0xb7, 5, 3, 0, 0},
	{0xf6, 6, 0, 0, 0},
	{0xde, 3, 5, 0, 0},
	{0x9f, 0, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0xee, 6, 4, 0, 0},
	{0xcf, 0, 5, 0, 0},
	{0x9f, 4, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0x7e, 6, 7, 0, 0},
	{0x5f, 0, 5, 0, 0},
	{0xde, 7, 0, 0, 0},
	{0xbe, 5, 6, 0, 0},
	{0xbd, 0, 1, 0, 0},
	{0xdd, 6, 5, 0, 0},
	{0xde, 1, 0, 0, 0},
	{0xbe, 5, 6, 0, 0},
	{0xbb, 0, 2, 0, 0},
	{0xfa, 6, 0, 0, 0},
	{0xde, 2, 5, 0, 0},
	{0x9f, 0, 6, 0, 0},
	{0x3f, 5, 7, 0, 0},
	{0x7e, 6, 0, 0, 0},
	{0xde, 7, 5, 0, 0},
	{0x9f, 0, 6, 0, 0},
	{0xb7, 5, 3, 0, 0},
	{0xf6, 6, 0, 0, 0},
	{0xde, 3, 5, 0, 0},
	{0x9f, 0, 6, 0, 0},
	{0xaf, 5, 4, 0, 0},
};

fal_port_scheduler_cfg_t port_scheduler1_tbl[] = {
	{0x30, 5, 6, 0, 0},
	{0x30, 4, 0, 0, 0},
	{0x30, 5, 6, 0, 0},
	{0x11, 0, 5, 0, 0},
	{0x50, 6, 0, 0, 0},
	{0x30, 5, 6, 0, 0},
	{0x21, 0, 4, 0, 0},
	{0x21, 5, 6, 0, 0},
	{0x30, 4, 0, 0, 0},
	{0x50, 6, 5, 0, 0},
	{0x11, 0, 6, 0, 0},
	{0x30, 5, 0, 0, 0},
	{0x30, 4, 6, 0, 0},
	{0x11, 0, 5, 0, 0},
	{0x50, 6, 0, 0, 0},
	{0x30, 5, 6, 0, 0},
	{0x11, 0, 5, 0, 0},
	{0x11, 4, 6, 0, 0},
	{0x30, 5, 0, 0, 0},
	{0x50, 6, 5, 0, 0},
	{0x11, 0, 6, 0, 0},
	{0x30, 5, 0, 0, 0},
	{0x30, 4, 6, 0, 0},
	{0x11, 0, 5, 0, 0},
	{0x50, 6, 0, 0, 0},
};

fal_port_scheduler_cfg_t cppe_port_scheduler0_tbl[] = {
	{0xb7, 0, 6, 0, 0},
	{0xbe, 3, 0, 0, 0},
	{0xde, 6, 5, 0, 0},
	{0xdd, 0, 1, 0, 0},
	{0xbd, 5, 6, 0, 0},
	{0xbe, 1, 0, 0, 0},
	{0xee, 6, 4, 0, 0},
	{0xcf, 0, 5, 0, 0},
	{0x9f, 4, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0x7e, 6, 7, 0, 0},
	{0x5f, 0, 5, 0, 0},
	{0x9f, 7, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0xfa, 6, 2, 0, 0},
	{0xbb, 0, 6, 0, 0},
	{0x9f, 2, 5, 0, 0},
	{0xcf, 6, 4, 0, 0},
	{0xee, 5, 0, 0, 0},
	{0xbe, 4, 6, 0, 0},
	{0x3f, 0, 7, 0, 0},
	{0x5f, 6, 5, 0, 0},
	{0xde, 7, 0, 0, 0},
	{0xbe, 5, 6, 0, 0},
	{0xb7, 0, 3, 0, 0},
	{0xe7, 6, 4, 0, 0},
	{0xee, 3, 0, 0, 0},
	{0xbe, 4, 6, 0, 0},
	{0x9f, 0, 5, 0, 0},
	{0xdd, 6, 1, 0, 0},
	{0xfc, 5, 0, 0, 0},
	{0xbe, 1, 6, 0, 0},
	{0x9f, 0, 5, 0, 0},
	{0x5f, 6, 7, 0, 0},
	{0x7e, 5, 0, 0, 0},
	{0xbe, 7, 6, 0, 0},
	{0xaf, 0, 4, 0, 0},
	{0xcf, 6, 5, 0, 0},
	{0x9f, 4, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0xfa, 6, 2, 0, 0},
	{0xdb, 0, 5, 0, 0},
	{0x9f, 2, 6, 0, 0},
	{0xbe, 5, 0, 0, 0},
	{0x7e, 6, 7, 0, 0},
	{0x6f, 0, 4, 0, 0},
	{0xaf, 7, 6, 0, 0},
	{0x9f, 4, 5, 0, 0},
	{0xde, 6, 0, 0, 0},
	{0xf6, 5, 3, 0, 0},
};

fal_port_tdm_tick_cfg_t port_tdm0_tbl[] = {
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 3, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 3, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
};

fal_port_tdm_tick_cfg_t cppe_port_tdm0_tbl[] = {
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 1, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 2, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 3, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 1, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 2, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 3, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 0, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 4, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 5, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 6, 0, 0},
	{1, FAL_PORT_TDB_DIR_INGRESS, 7, 0, 0},
	{1, FAL_PORT_TDB_DIR_EGRESS, 7, 0, 0},
};

sw_error_t
qca_hppe_tdm_hw_init(a_uint32_t dev_id, a_bool_t enable)
{
	adpt_api_t *p_api;
	a_uint32_t i = 0;
	a_uint32_t num;
	fal_port_tdm_ctrl_t tdm_ctrl;
	fal_port_scheduler_cfg_t *scheduler_cfg;
	fal_port_tdm_tick_cfg_t *bm_cfg;
	a_uint8_t tm_tick_mode, bm_tick_mode;

	SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));
	SW_RTN_ON_NULL(p_api->adpt_port_scheduler_cfg_set);
	SW_RTN_ON_NULL(p_api->adpt_tdm_tick_num_set);

	tm_tick_mode = ssdk_tm_tick_mode_get(dev_id);
	bm_tick_mode = ssdk_bm_tick_mode_get(dev_id);

	if (tm_tick_mode == 0) {
		if (adpt_chip_revision_get(dev_id) == CPPE_REVISION) {
			num = sizeof(cppe_port_scheduler0_tbl) /
				sizeof(fal_port_scheduler_cfg_t);
			scheduler_cfg = cppe_port_scheduler0_tbl;
		} else {
			num = sizeof(port_scheduler0_tbl) /
				sizeof(fal_port_scheduler_cfg_t);
			scheduler_cfg = port_scheduler0_tbl;
		}
	} else if (tm_tick_mode == 1) {
		num = sizeof(port_scheduler1_tbl) / sizeof(fal_port_scheduler_cfg_t);
		scheduler_cfg = port_scheduler1_tbl;
	} else {
		return SW_BAD_VALUE;
	}

	for (i = 0; i < num; i++) {
		p_api->adpt_port_scheduler_cfg_set(dev_id, i, &scheduler_cfg[i]);
	}
	p_api->adpt_tdm_tick_num_set(dev_id, num);

	SW_RTN_ON_NULL(p_api->adpt_port_tdm_tick_cfg_set);
	SW_RTN_ON_NULL(p_api->adpt_port_tdm_ctrl_set);

	if (bm_tick_mode == 0) {
		if (adpt_chip_revision_get(dev_id) == CPPE_REVISION) {
			num = sizeof(cppe_port_tdm0_tbl) /
				sizeof(fal_port_tdm_tick_cfg_t);
			bm_cfg = cppe_port_tdm0_tbl;
		} else {
			num = sizeof(port_tdm0_tbl) /
				sizeof(fal_port_tdm_tick_cfg_t);
			bm_cfg = port_tdm0_tbl;
		}
	} else {
		return SW_BAD_VALUE;
	}
	for (i = 0; i < num; i++) {
		if (adpt_chip_type_get(dev_id) == CHIP_HPPE) {
			if ((bm_cfg[i].port == SSDK_PHYSICAL_PORT0) ||
				(bm_cfg[i].port == SSDK_PHYSICAL_PORT7)) {
				if (enable == A_FALSE) {
					bm_cfg[i].valid = A_FALSE;
				} else {
					bm_cfg[i].valid = A_TRUE;
				}
			}
		}
		p_api->adpt_port_tdm_tick_cfg_set(dev_id, i, &bm_cfg[i]);
	}
	tdm_ctrl.enable = A_TRUE;
	tdm_ctrl.offset = A_FALSE;
	tdm_ctrl.depth = num;
	p_api->adpt_port_tdm_ctrl_set(dev_id, &tdm_ctrl);
	return SW_OK;
}
#endif

#if defined(IN_BM)
sw_error_t
qca_hppe_bm_hw_init(a_uint32_t dev_id)
{
	a_uint32_t i = 0;
	fal_bm_dynamic_cfg_t cfg;
	a_uint16_t group_buf = 0, share_ceiling = 0, phyport_share_ceiling = 0;
	adpt_ppe_type_t chip_type = adpt_ppe_type_get(dev_id);

	for (i = 0; i <  PPE_BM_PORT_NUM; i++) {
		/*disable fc for phy ports and enable fc for port 0 and port 7*/
		if(i >= PPE_BM_PHY_PORT_OFFSET && i <= PPE_BM_PHY_PORT_MAX)
			fal_port_bm_ctrl_set(dev_id, i, A_FALSE);
		else
			fal_port_bm_ctrl_set(dev_id, i, A_TRUE);
		/* map to group 0 */
		fal_port_bufgroup_map_set(dev_id, i, 0);
	}

	switch (chip_type) {
		case HPPE_TYPE:
			group_buf = 1400;
			break;
		case CPPE_TYPE:
			group_buf = 1024;
			break;
		case APPE_TYPE:
			group_buf = 1550;
			break;
		case MPPE_TYPE:
			group_buf = 240;
			break;
		default:
			SSDK_ERROR("Unsupported chip type: %d\n", chip_type);
			return SW_OUT_OF_RANGE;
	}

	fal_bm_bufgroup_buffer_set(dev_id, 0, group_buf);

	/* set reserved buffer */
	for (i = 0; i < PPE_BM_PORT_NUM; i++) {
		a_uint16_t prealloc_buf = 0, react_buf = 0;
		switch (chip_type) {
			case HPPE_TYPE:
			case APPE_TYPE:
				if (i < PPE_BM_PHY_PORT_OFFSET) {
					prealloc_buf = 0;
					react_buf = 100;
				} else if (i == PPE_BM_PORT_NUM-1) {
					prealloc_buf = 0;
					react_buf = 40;
				} else {
					prealloc_buf = 0;
					react_buf = 128;
				}
				break;
			case CPPE_TYPE:
				if (i < PPE_BM_PHY_PORT_OFFSET) {
					prealloc_buf = 0;
					react_buf = 100;
				} else if (i >= PPE_BM_PORT_NUM-2) {
					/* port 6 is loopback port on CPPE */
					prealloc_buf = 0;
					react_buf = 40;
				} else {
					prealloc_buf = 0;
					react_buf = 128;
				}
				break;
			case MPPE_TYPE:
				if (i < PPE_BM_PHY_PORT_OFFSET) {
					prealloc_buf = 12;
					react_buf = 40;
				} else {
					prealloc_buf = 12;
					react_buf = 128;
				}
				break;
			default:
				SSDK_ERROR("Unsupported chip type: %d\n", chip_type);
				return SW_OUT_OF_RANGE;
		}

		fal_bm_port_reserved_buffer_set(dev_id, i, prealloc_buf, react_buf);
	}

	/* set dynamic threshold */
	memset(&cfg, 0, sizeof(cfg));
	for (i = 0; i < PPE_BM_PORT_NUM; i++) {
		switch (chip_type) {
			case HPPE_TYPE:
				share_ceiling = 250;
				phyport_share_ceiling = 250;
				cfg.resume_min_thresh = 0;
				cfg.resume_off = 36;
				cfg.weight= 4;
				break;
			case CPPE_TYPE:
				share_ceiling = 216;
				phyport_share_ceiling = 216;
				cfg.resume_min_thresh = 0;
				cfg.resume_off = 36;
				cfg.weight= 4;
				break;
			case APPE_TYPE:
				if (i == PPE_BM_PORT_MIN) {
					share_ceiling = 1146;
					cfg.resume_min_thresh = 0;
					cfg.resume_off = 8;
					cfg.weight= 7;
				} else {
					share_ceiling = 250;
					phyport_share_ceiling = 250;
					cfg.resume_min_thresh = 0;
					cfg.resume_off = 36;
					cfg.weight= 4;
				}
				break;
			case MPPE_TYPE:
				share_ceiling = 30;
				phyport_share_ceiling = 48;
				cfg.resume_min_thresh = 20;
				cfg.resume_off = 5;
				cfg.weight= 7;
				break;
			default:
				SSDK_ERROR("Unsupported chip type: %d\n", chip_type);
				return SW_OUT_OF_RANGE;
		}

		if (i < PPE_BM_PHY_PORT_OFFSET)
			cfg.shared_ceiling = share_ceiling;
		else
			cfg.shared_ceiling = phyport_share_ceiling;

		fal_bm_port_dynamic_thresh_set(dev_id, i, &cfg);
	}

	return SW_OK;
}
#endif

#if defined(IN_QM)
#define SSDK_PRI_MAX		16
#define SSDK_CPU_PRI_NUM	1
#define SSDK_MGMT_ARP_REP_CPU_CODE	101
sw_error_t
qca_hppe_qm_hw_init(a_uint32_t dev_id)
{
	a_uint32_t i;
	fal_ucast_queue_dest_t queue_dst;
	fal_ac_obj_t obj;
	fal_ac_ctrl_t ac_ctrl;
	fal_ac_group_buffer_t group_buff;
	fal_ac_dynamic_threshold_t  dthresh_cfg;
	fal_ac_static_threshold_t sthresh_cfg;
	a_uint32_t qbase = 0;
	a_uint32_t chip_ver = 0;
	a_uint16_t total_buf = 0, ceiling = 0, green_max = 0, weight = 0, resume_offset = 0;
	a_uint32_t max_pri_supported, pri, class;
	adpt_ppe_type_t chip_type = adpt_ppe_type_get(dev_id);

	memset(&queue_dst, 0, sizeof(queue_dst));

	chip_ver = adpt_chip_revision_get(dev_id);

	/*
	 * Redirect service code 2 to queue 1
	 * TODO: keep sync with  NSS
	 */
	queue_dst.service_code_en = A_TRUE;
	queue_dst.service_code = 2;
	fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 8, 0);

	queue_dst.service_code = 3;
	fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 128, 8);

	queue_dst.service_code = 4;
	fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 128, 8);

	queue_dst.service_code = 5;
	fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 0, 0);

	queue_dst.service_code = 6;
	fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 8, 0);

	queue_dst.service_code = 7;
#if defined(APPE)
	if (adpt_chip_type_get(dev_id) == CHIP_APPE)
		fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 252, 0);
	else
#endif
		fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 240, 0);

	queue_dst.service_code_en = A_FALSE;
	queue_dst.service_code = 0;
	for(i = 0; i < SSDK_MAX_PORT_NUM; i++) {
		queue_dst.dst_port = i;
		qbase = ssdk_ucast_queue_start_get(dev_id, i);
		fal_ucast_queue_base_profile_set(dev_id, &queue_dst, qbase, i);

		/* Initializes the ucast priority map, cpu queue number depends on the
		 * queue number mapped with the EDMA RX ring by EDMA_QID2RID_TABLE,
		 * only queue 0 is enabled for host EDMA rings by default.
		 * physical port has different queue number between hppe and appe.
		 * 16 queues per port for hppe, class 0~15 matches with priority 0~15.
		 * 8 queues per port for appe, class 0~6 matches with priority 0~6, and
		 * class 7 matches with priority >=7.
		 */
		max_pri_supported = ssdk_ucast_l0_cdrr_num_get(dev_id, i);

		if (max_pri_supported > SSDK_PRI_MAX) {
			max_pri_supported = SSDK_CPU_PRI_NUM;
		}

		for (pri = 0; pri < SSDK_PRI_MAX; pri++) {
			if (pri >= max_pri_supported)
				class = max_pri_supported - 1;
			else
				class = pri;

			if (i == 0) {
				/*
				 * For CPU port, ARM core and direct switch take the common
				 * profile ID 0, point offload(NSS) takes the special profile
				 * ID 15.
				 */
				fal_ucast_priority_class_set(dev_id, FAL_QM_PROFILE_COMMON_ID,
						pri, class);

				fal_ucast_priority_class_set(dev_id, FAL_QM_PROFILE_PO_ID,
						pri, class);

			} else {
				fal_ucast_priority_class_set(dev_id, i, pri, class);
			}

		}

		if (i == 0) {
			a_uint32_t hash = 0;
			/*
			 * For CPU port, we need to initialize the hash map offset to 0 for the
			 * PO and cpu code profile.
			 */
			for (hash = 0; hash < FAL_QM_PROFILE_PO_RSS_HASH_MAX; hash++) {
				fal_ucast_hash_map_set(dev_id, FAL_QM_PROFILE_PO_ID,
						hash, FAL_QM_PROFILE_PO_RSS_HASH_CLASS);
				fal_ucast_hash_map_set(dev_id, FAL_QM_PROFILE_CPU_CODE_ID,
						hash, FAL_QM_PROFILE_PO_RSS_HASH_CLASS);
			}
		}
	}

	/*
	 * Configure the RDTCPU ARP reply packet with the max priority.
	 */
	queue_dst.cpu_code_en = A_TRUE;
	queue_dst.cpu_code = SSDK_MGMT_ARP_REP_CPU_CODE;
	qbase = ssdk_ucast_queue_start_get(dev_id, SSDK_PORT_CPU);
	max_pri_supported = ssdk_ucast_l0_cdrr_num_get(dev_id, SSDK_PORT_CPU);
	if (max_pri_supported > SSDK_PRI_MAX) {
		max_pri_supported = SSDK_CPU_PRI_NUM;
	}

	fal_ucast_queue_base_profile_set(dev_id, &queue_dst, qbase + max_pri_supported - 1, 0);

	queue_dst.cpu_code_en = A_FALSE;
	queue_dst.cpu_code = 0;

	/*
	 * Enable PPE source profile 1 and map it to PPE queue 4
	 */
	memset(&queue_dst, 0, sizeof(queue_dst));
	queue_dst.src_profile = 1;

	/*
	 * Enable service code mapping for profile 1
	 */
	queue_dst.service_code_en = A_TRUE;
	for (i = 0; i < SSDK_MAX_SERVICE_CODE_NUM; i++) {
		queue_dst.service_code = i;

		if (i == 2 || i == 6) {
			fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 8, 0);
		} else if (i == 3 || i == 4) {
			fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 128, 8);
		} else {
			fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 4, 0);
		}
	}
	queue_dst.service_code_en = A_FALSE;
	queue_dst.service_code = 0;

	/*
	 * Enable cpu code mapping for profile 1
	 */
	queue_dst.cpu_code_en = A_TRUE;
	for (i = 0; i < SSDK_MAX_CPU_CODE_NUM; i++) {
		queue_dst.cpu_code = i;
		fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 4, 0);
	}
	queue_dst.cpu_code_en = A_FALSE;
	queue_dst.cpu_code = 0;

	/*
	 * Enable destination port mappings for profile 1
	 */
	for (i = 0; i < SSDK_MAX_PORT_NUM; i++) {
		queue_dst.dst_port = i;
		qbase = ssdk_ucast_queue_start_get(dev_id, i);
		fal_ucast_queue_base_profile_set(dev_id, &queue_dst, qbase, i);
	}

	for (i = SSDK_MAX_PORT_NUM; i <= SSDK_MAX_VIRTUAL_PORT_ID; i++) {
		queue_dst.dst_port = i;
		fal_ucast_queue_base_profile_set(dev_id, &queue_dst, 4, 0);
	}
	queue_dst.dst_port = 0;

	/* queue ac*/
	ac_ctrl.ac_en = A_TRUE;
	ac_ctrl.ac_fc_en = A_FALSE;
	for (i = 0; i < SSDK_L0SCHEDULER_CFG_MAX; i++) {
		obj.type = FAL_AC_QUEUE;
		obj.obj_id = i;
		fal_ac_ctrl_set(dev_id, &obj, &ac_ctrl);
		fal_ac_queue_group_set(dev_id, i, 0);
		fal_ac_prealloc_buffer_set(dev_id, &obj, 0);
	}

	switch (chip_type) {
		case HPPE_TYPE:
		case APPE_TYPE:
			total_buf = 2000;
			ceiling = 400;
			weight = 4;
			resume_offset = 36;
			green_max = 250;
			break;
		case CPPE_TYPE:
			total_buf = 1506;
			ceiling = 216;
			weight = 4;
			resume_offset = 36;
			green_max = 144;
			break;
		case MPPE_TYPE:
			total_buf = 500;
			ceiling = 50;
			weight = 5;
			resume_offset = 18;
			green_max = 50;
			break;
		default:
			SSDK_ERROR("Unsupported chip type: %d\n", chip_type);
			return SW_OUT_OF_RANGE;
	}

	group_buff.prealloc_buffer = 0;
	group_buff.total_buffer = total_buf;
	fal_ac_group_buffer_set(dev_id, 0, &group_buff);

	memset(&dthresh_cfg, 0, sizeof(dthresh_cfg));
	dthresh_cfg.shared_weight = weight;
	dthresh_cfg.ceiling = ceiling;
	dthresh_cfg.green_resume_off = resume_offset;
	for (i = 0; i < SSDK_L0SCHEDULER_UCASTQ_CFG_MAX; i++) {
		fal_ac_dynamic_threshold_set(dev_id, i, &dthresh_cfg);
	}

	memset(&sthresh_cfg, 0, sizeof(sthresh_cfg));
	sthresh_cfg.green_max = green_max;
	sthresh_cfg.green_resume_off = resume_offset;
	for (i = SSDK_L0SCHEDULER_UCASTQ_CFG_MAX; i < SSDK_L0SCHEDULER_CFG_MAX; i++) {
		obj.type = FAL_AC_QUEUE;
		obj.obj_id = i;
		fal_ac_static_threshold_set(dev_id, &obj, &sthresh_cfg);
	}

	/* enable the queue counter */
	fal_queue_counter_ctrl_set(dev_id, A_TRUE);

	return SW_OK;
}
#endif

#if defined(IN_QOS)
sw_error_t
qca_hppe_qos_scheduler_hw_init(a_uint32_t dev_id)
{
	a_uint32_t i = 0, port_id = 0;
	fal_qos_scheduler_cfg_t cfg;
	fal_queue_bmp_t queue_bmp;
	fal_qos_group_t group_sel;
	fal_qos_pri_precedence_t pri_pre;
	ssdk_dt_scheduler_cfg *dt_cfg = ssdk_bootup_shceduler_cfg_get(dev_id);

	memset(&cfg, 0, sizeof(cfg));

	/* L1 shceduler */
	for (i = 0; i < SSDK_L1SCHEDULER_CFG_MAX; i++) {
		if (dt_cfg->l1cfg[i].valid) {
			port_id = dt_cfg->l1cfg[i].port_id;
#if defined(IN_ATHTAG)
			if (port_id >= SSDK_PHYSICAL_PORT3 &&
				port_id <= SSDK_PHYSICAL_PORT6) {
				port_id = SSDK_PHYSICAL_PORT1;
			}
#endif
			cfg.sp_id = port_id;
			cfg.c_pri = dt_cfg->l1cfg[i].cpri;
			cfg.e_pri = dt_cfg->l1cfg[i].epri;
			cfg.c_drr_id = dt_cfg->l1cfg[i].cdrr_id;
			cfg.e_drr_id = dt_cfg->l1cfg[i].edrr_id;
			cfg.c_drr_wt = 1;
			cfg.e_drr_wt = 1;
			fal_queue_scheduler_set(dev_id, i, 1,
					port_id, &cfg);
		}
	}

	/* L0 shceduler */
	for (i = 0; i < SSDK_L0SCHEDULER_CFG_MAX; i++) {
		if (dt_cfg->l0cfg[i].valid) {
			port_id = dt_cfg->l0cfg[i].port_id;
#if defined(IN_ATHTAG)
			if (port_id >= SSDK_PHYSICAL_PORT3 &&
				port_id <= SSDK_PHYSICAL_PORT6) {
				port_id = SSDK_PHYSICAL_PORT1;
			}
#endif
			cfg.sp_id = dt_cfg->l0cfg[i].sp_id;
			cfg.c_pri = dt_cfg->l0cfg[i].cpri;
			cfg.e_pri = dt_cfg->l0cfg[i].epri;
			cfg.c_drr_id = dt_cfg->l0cfg[i].cdrr_id;
			cfg.e_drr_id = dt_cfg->l0cfg[i].edrr_id;
			cfg.c_drr_wt = 1;
			cfg.e_drr_wt = 1;
			fal_queue_scheduler_set(dev_id, i,
					0, port_id, &cfg);
		}
	}

	/* queue--edma ring mapping*/
	memset(&queue_bmp, 0, sizeof(queue_bmp));
	queue_bmp.bmp[0] = 0xF;
	fal_edma_ring_queue_map_set(dev_id, 0, &queue_bmp);
	queue_bmp.bmp[0] = 0xF0;
	fal_edma_ring_queue_map_set(dev_id, 3, &queue_bmp);
	queue_bmp.bmp[0] = 0xF00;
	fal_edma_ring_queue_map_set(dev_id, 1, &queue_bmp);
	queue_bmp.bmp[0] = 0;
	queue_bmp.bmp[4] = 0xFFFF;
	fal_edma_ring_queue_map_set(dev_id, 2, &queue_bmp);

	/* chose qos group 0 */
	group_sel.dscp_group = 0;
	group_sel.flow_group = 0;
	group_sel.pcp_group = 0;
	for (i = SSDK_PHYSICAL_PORT0; i <= SSDK_PHYSICAL_PORT7; i++) {
		fal_qos_port_group_set(dev_id, i, &group_sel);
	}
	/* qos precedence */
	pri_pre.flow_pri = 4;
	pri_pre.acl_pri = 2;
	pri_pre.dscp_pri = 1;
	pri_pre.pcp_pri = 0;
	pri_pre.preheader_pri = 3;
	for (i = SSDK_PHYSICAL_PORT0; i <= SSDK_PHYSICAL_PORT7; i++) {
		fal_qos_port_pri_precedence_set(dev_id, i, &pri_pre);
	}

	return SW_OK;
}
#endif


#if defined(IN_ACL)
#define LIST_ID_BYP_FDB_LRN 63/*reserved for bypass fdb learning*/
#define LIST_PRI_BYP_FDB_LRN 32

sw_error_t qca_hppe_acl_byp_intf_mac_learn(a_uint32_t dev_id)
{
	a_uint32_t index = 0, num;
	fal_acl_rule_t rule = { 0 };
	a_uint8_t* mac;
	a_uint32_t port_bmp = qca_ssdk_port_bmp_get(dev_id);

	num = ssdk_intf_mac_num_get();
	if(num == 0){
		return SW_OK;/*No found interface MAC*/
	}

	/*Bypass fdb learn*/
	rule.rule_type = FAL_ACL_RULE_MAC;
	rule.bypass_bitmap |= (1<<FAL_ACL_BYPASS_FDB_LEARNING);
	rule.bypass_bitmap |= (1<<FAL_ACL_BYPASS_FDB_REFRESH);

	FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_MAC_SA);

	fal_acl_list_creat(dev_id, LIST_ID_BYP_FDB_LRN, LIST_PRI_BYP_FDB_LRN);

	for (index = 0; index < SSDK_MAX_NR_ETH; index++) {
		if(index >= num)
			break;
		mac = ssdk_intf_macaddr_get(index);
		memcpy(rule.src_mac_val.uc, mac, 6);
		memset(rule.src_mac_mask.uc, 0xff, 6);
		SSDK_DEBUG("%02x:%02x:%02x:%02x:%02x:%02x\n", rule.src_mac_val.uc[0],
			rule.src_mac_val.uc[1], rule.src_mac_val.uc[2], rule.src_mac_val.uc[3],
			rule.src_mac_val.uc[4], rule.src_mac_val.uc[5]);
		fal_acl_rule_add(dev_id, LIST_ID_BYP_FDB_LRN, index, 1, &rule);
	}
	fal_acl_list_bind(dev_id, LIST_ID_BYP_FDB_LRN, FAL_ACL_DIREC_IN,
				FAL_ACL_BIND_PORTBITMAP, port_bmp);

	return SW_OK;
}

#if defined(IN_PTP)
sw_error_t qca_hppe_acl_remark_ptp_servcode(a_uint32_t dev_id) {
#define LIST_ID_L2_TAG_SERVICE_CODE_PTP 58
#define LIST_ID_L4_TAG_SERVICE_CODE_PTP 59
#define LIST_PRI_TAG_SERVICE_CODE_PTP   1
#define PTP_EVENT_PKT_SERVICE_CODE      0x9
#define PTP_EV_PORT                     319
#define PTP_MSG_SYNC                    0
#define PTP_MSG_PRESP                   3

	sw_error_t ret;
	fal_servcode_config_t servcode_conf;
	fal_acl_rule_t entry = {0};
	a_uint32_t index = 0, msg_type = 0;
	a_uint32_t ptp_port_bmp = 0;

	/* only marking ptp packet with service code for the qca808x phy */
	ptp_port_bmp = qca_ssdk_phy_type_port_bmp_get(dev_id, QCA808X_PHY_CHIP);

	/* Not found the PHY with ptp feature */
	if (ptp_port_bmp == 0) {
		return SW_OK;
	}

	/* Create PTP ACL L2 list */
	ret = fal_acl_list_creat(dev_id, LIST_ID_L2_TAG_SERVICE_CODE_PTP,
			LIST_PRI_TAG_SERVICE_CODE_PTP);
	SW_RTN_ON_ERROR(ret);

	/* Set up UDF2 profile */
	ret = fal_acl_udf_profile_set(dev_id, FAL_ACL_UDF_NON_IP, 2, FAL_ACL_UDF_TYPE_L3, 0);
	SW_RTN_ON_ERROR(ret);

	/* Tag service code for PTP packet */
	entry.service_code = PTP_EVENT_PKT_SERVICE_CODE;
	entry.pri = LIST_PRI_TAG_SERVICE_CODE_PTP;
	FAL_ACTION_FLG_SET(entry.action_flg, FAL_ACL_ACTION_SERVICE_CODE);
	FAL_ACTION_FLG_SET(entry.action_flg, FAL_ACL_ACTION_PERMIT);

	/* L2 PTP packet */
	entry.rule_type = FAL_ACL_RULE_MAC;

	/* L2 PTP ethernet type 0x88f7 */
	entry.ethtype_val = ETH_P_1588;
	entry.ethtype_mask = 0xffff;
	FAL_FIELD_FLG_SET(entry.field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);

	for (msg_type = PTP_MSG_SYNC; msg_type <= PTP_MSG_PRESP; msg_type++) {
		/* L2 UDF2 for msg type */
		entry.udf2_val = (msg_type << 0x8);
		entry.udf2_mask = 0x0f00;
		FAL_FIELD_FLG_SET(entry.field_flg, FAL_ACL_FIELD_UDF2);

		/* Add PTP L2 rule to ACL list */
		ret = fal_acl_rule_add(dev_id, LIST_ID_L2_TAG_SERVICE_CODE_PTP,
				index++, 1, &entry);
		SW_RTN_ON_ERROR(ret);
	}

	/* Unset L2 PTP ethernet type 0x88f7 */
	index = 0;
	FAL_FIELD_FLG_CLR(entry.field_flg, FAL_ACL_FIELD_UDF2);
	FAL_FIELD_FLG_CLR(entry.field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);

	/* Create PTP ACL L4 list */
	ret = fal_acl_list_creat(dev_id, LIST_ID_L4_TAG_SERVICE_CODE_PTP,
			LIST_PRI_TAG_SERVICE_CODE_PTP);
	SW_RTN_ON_ERROR(ret);

	/* IPv4 PTP packet */
	entry.rule_type = FAL_ACL_RULE_IP4;
	entry.is_ip_mask = 1;
	entry.is_ip_val = A_TRUE;
	FAL_FIELD_FLG_SET(entry.field_flg, FAL_ACL_FIELD_IP);
	entry.is_ipv6_mask = 1;
	entry.is_ipv6_val = A_FALSE;
	FAL_FIELD_FLG_SET(entry.field_flg, FAL_ACL_FIELD_IPV6);

	/* PTP over UDP protocol */
	entry.ip_proto_val = IPPROTO_UDP;
	entry.ip_proto_mask = 0xff;
	FAL_FIELD_FLG_SET(entry.field_flg, FAL_ACL_FIELD_IP_PROTO);

	/* PTP UDP dest port 319 */
	entry.dest_l4port_op = FAL_ACL_FIELD_MASK;
	entry.dest_l4port_val = PTP_EV_PORT;
	entry.dest_l4port_mask = 0xffff;
	FAL_FIELD_FLG_SET(entry.field_flg, FAL_ACL_FIELD_L4_DPORT);

	/* Add PTP IPv4 rule to ACL list */
	ret = fal_acl_rule_add(dev_id, LIST_ID_L4_TAG_SERVICE_CODE_PTP, index++, 1, &entry);
	SW_RTN_ON_ERROR(ret);

	/* IPv6 PTP packet */
	entry.rule_type = FAL_ACL_RULE_IP6;
	entry.is_ipv6_val = A_TRUE;

	/* Add PTP IPv6 rule to ACL list */
	ret = fal_acl_rule_add(dev_id, LIST_ID_L4_TAG_SERVICE_CODE_PTP, index++, 1, &entry);
	SW_RTN_ON_ERROR(ret);

	/* Bind PTP ACL list to port bmp */
	ret = fal_acl_list_bind(dev_id, LIST_ID_L2_TAG_SERVICE_CODE_PTP,
			FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, ptp_port_bmp);
	SW_RTN_ON_ERROR(ret);
	ret = fal_acl_list_bind(dev_id, LIST_ID_L4_TAG_SERVICE_CODE_PTP,
			FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, ptp_port_bmp);
	SW_RTN_ON_ERROR(ret);

	/* configure the next service code of ptp service code, which
	 * is needed for EDMA receiving the packet with service code.
	 */
	ret = fal_servcode_config_get(dev_id, PTP_EVENT_PKT_SERVICE_CODE, &servcode_conf);
	SW_RTN_ON_ERROR(ret);
	servcode_conf.next_service_code = PTP_EVENT_PKT_SERVICE_CODE;
	ret = fal_servcode_config_set(dev_id, PTP_EVENT_PKT_SERVICE_CODE, &servcode_conf);
	SW_RTN_ON_ERROR(ret);

	return ret;
}
#endif
#endif

sw_error_t
qca_hppe_interface_mode_init(a_uint32_t dev_id)
{

	adpt_api_t *p_api;
	sw_error_t rv = SW_OK;
	fal_port_t port_id;
	a_uint32_t port_max = SSDK_PHYSICAL_PORT7;
	a_uint32_t index = 0, mode[3] = {0};
	adpt_ppe_type_t ppe_type = adpt_ppe_type_get(dev_id);

	SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));
	SW_RTN_ON_NULL(p_api->adpt_port_mux_mac_type_set);
	SW_RTN_ON_NULL(p_api->adpt_uniphy_mode_set);

	for (index = SSDK_UNIPHY_INSTANCE0; index <= SSDK_UNIPHY_INSTANCE2; index ++) {
		mode[index] = ssdk_dt_global_get_mac_mode(dev_id, index);
	}

	rv = p_api->adpt_uniphy_mode_set(dev_id, SSDK_UNIPHY_INSTANCE0, mode[0]);
	SW_RTN_ON_ERROR(rv);

	rv = p_api->adpt_uniphy_mode_set(dev_id, SSDK_UNIPHY_INSTANCE1, mode[1]);
	SW_RTN_ON_ERROR(rv);

	if ((ppe_type == HPPE_TYPE) || (ppe_type == APPE_TYPE)) {

		rv = p_api->adpt_uniphy_mode_set(dev_id,
				SSDK_UNIPHY_INSTANCE2, mode[2]);
		SW_RTN_ON_ERROR(rv);
	}

	for (index = SSDK_UNIPHY_INSTANCE0; index <= SSDK_UNIPHY_INSTANCE2; index ++) {
		if (mode[index] == PORT_WRAPPER_MAX) {
			ssdk_gcc_uniphy_sys_set(dev_id, index, A_FALSE);
		}
	}

	switch (ppe_type) {
		case HPPE_TYPE:
			port_max = SSDK_PHYSICAL_PORT7;
			break;
		case CPPE_TYPE:
			port_max = SSDK_PHYSICAL_PORT6;
			break;
		case APPE_TYPE:
			port_max = SSDK_PHYSICAL_PORT7;
			break;
		case MPPE_TYPE:
			port_max = SSDK_PHYSICAL_PORT3;
			break;
		default:
			SSDK_ERROR("Unknown chip type: %d\n", ppe_type);
			break;
	}
	for(port_id = SSDK_PHYSICAL_PORT1; port_id < port_max; port_id++) {
		rv = p_api->adpt_port_mux_mac_type_set(dev_id, port_id, mode[0], mode[1], mode[2]);
		if(rv != SW_OK) {
			SSDK_ERROR("port_id:%d, mode0:%d, mode1:%d, mode2:%d\n", port_id,
				mode[0], mode[1], mode[2]);
			break;
		}
	}

	return rv;
}


#if defined(IN_FLOW)
sw_error_t
qca_hppe_flow_hw_init(a_uint32_t dev_id)
{
	fal_flow_direction_t dir, dir_max;
	fal_flow_mgmt_t mgmt;
	sw_error_t rv;

	memset(&mgmt, 0, sizeof(fal_flow_mgmt_t));
	dir_max = FAL_FLOW_UNKOWN_DIR_DIR;

	/*set redirect to cpu for multicast flow*/
	for (dir = FAL_FLOW_LAN_TO_LAN_DIR; dir <= dir_max; dir++) {
		rv = fal_flow_mgmt_get(dev_id, FAL_FLOW_MCAST, dir, &mgmt);
		SW_RTN_ON_ERROR(rv);
		mgmt.miss_action = FAL_MAC_RDT_TO_CPU;
		rv = fal_flow_mgmt_set(dev_id, FAL_FLOW_MCAST, dir, &mgmt);
		SW_RTN_ON_ERROR(rv);
	}
	return SW_OK;
}
#endif

sw_error_t qca_hppe_hw_init(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;

	/* reset ppe */
	ssdk_ppe_reset_init(dev_id);

	rv = qca_switch_init(dev_id);
	SW_RTN_ON_ERROR(rv);

#if defined(IN_BM)
	rv = qca_hppe_bm_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_QM)
	rv = qca_hppe_qm_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_QOS)
	rv = qca_hppe_qos_scheduler_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_BM) && defined(IN_QOS)
	rv = qca_hppe_tdm_hw_init(dev_id, A_TRUE);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_FDB)
	rv= qca_hppe_fdb_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_VSI)
	rv= qca_hppe_vsi_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_PORTVLAN)
	rv = qca_hppe_portvlan_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_PORTCONTROL)
	rv = qca_hppe_portctrl_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_POLICER)
	rv = qca_hppe_policer_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_SHAPER)
	rv = qca_hppe_shaper_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_FLOW)
	rv = qca_hppe_flow_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#if defined(IN_ACL)
	rv = qca_hppe_acl_byp_intf_mac_learn(dev_id);
	SW_RTN_ON_ERROR(rv);
#if defined(IN_PTP)
	rv = qca_hppe_acl_remark_ptp_servcode(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif
#endif
	rv = qca_hppe_interface_mode_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#if defined(IN_CTRLPKT)
	rv = qca_hppe_ctlpkt_hw_init(dev_id);
	SW_RTN_ON_ERROR(rv);
#endif

	return rv;
}

