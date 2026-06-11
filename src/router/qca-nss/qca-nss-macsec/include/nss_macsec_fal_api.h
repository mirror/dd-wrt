/*
 * Copyright (c) 2014, 2018, The Linux Foundation. All rights reserved.
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


#ifndef _NSS_MACSEC_FAL_API_H_
#define _NSS_MACSEC_FAL_API_H_

#include "nss_macsec_interrupt.h"
#include "nss_macsec_mib.h"
#include "nss_macsec_secy.h"
#include "nss_macsec_secy_rx.h"
#include "nss_macsec_secy_tx.h"

enum {
	NSS_MACSEC_SECY_INTERRUPT_EN_GET_CMD,
	NSS_MACSEC_SECY_INTERRUPT_EN_SET_CMD,
	NSS_MACSEC_SECY_INTERRUPT_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_MIB_GET_CMD,
	NSS_MACSEC_SECY_TX_SA_MIB_GET_CMD,
	NSS_MACSEC_SECY_TX_MIB_GET_CMD,
	NSS_MACSEC_SECY_RX_SA_MIB_GET_CMD,
	NSS_MACSEC_SECY_RX_MIB_GET_CMD,
	NSS_MACSEC_SECY_TX_MIB_CLEAR_CMD,
	NSS_MACSEC_SECY_TX_SC_MIB_CLEAR_CMD,
	NSS_MACSEC_SECY_TX_SA_MIB_CLEAR_CMD,
	NSS_MACSEC_SECY_RX_MIB_CLEAR_CMD,
	NSS_MACSEC_SECY_RX_SA_MIB_CLEAR_CMD,
	NSS_MACSEC_SECY_GENL_REG_GET_CMD,
	NSS_MACSEC_SECY_GENL_REG_SET_CMD,
	NSS_MACSEC_SECY_RX_CTL_FILT_GET_CMD,
	NSS_MACSEC_SECY_RX_CTL_FILT_SET_CMD,
	NSS_MACSEC_SECY_RX_CTL_FILT_CLEAR_CMD,
	NSS_MACSEC_SECY_RX_CTL_FILT_CLEAR_ALL_CMD,
	NSS_MACSEC_SECY_RX_PRC_LUT_GET_CMD,
	NSS_MACSEC_SECY_RX_PRC_LUT_SET_CMD,
	NSS_MACSEC_SECY_RX_PRC_LUT_CLEAR_CMD,
	NSS_MACSEC_SECY_RX_PRC_LUT_CLEAR_ALL_CMD,
	NSS_MACSEC_SECY_RX_SC_CREATE_CMD,
	NSS_MACSEC_SECY_RX_SC_EN_GET_CMD,
	NSS_MACSEC_SECY_RX_SC_EN_SET_CMD,
	NSS_MACSEC_SECY_RX_SC_DEL_CMD,
	NSS_MACSEC_SECY_RX_SC_DEL_ALL_CMD,
	NSS_MACSEC_SECY_RX_SC_VALIDATE_FRAME_GET_CMD,
	NSS_MACSEC_SECY_RX_SC_VALIDATE_FRAME_SET_CMD,
	NSS_MACSEC_SECY_RX_SC_REPLAY_PROTECT_GET_CMD,
	NSS_MACSEC_SECY_RX_SC_REPLAY_PROTECT_SET_CMD,
	NSS_MACSEC_SECY_RX_SC_ANTI_REPLAY_WINDOW_GET_CMD,
	NSS_MACSEC_SECY_RX_SC_ANTI_REPLAY_WINDOW_SET_CMD,
	NSS_MACSEC_SECY_RX_SC_IN_USED_GET_CMD,
	NSS_MACSEC_SECY_RX_SA_CREATE_CMD,
	NSS_MACSEC_SECY_RX_SA_EN_GET_CMD,
	NSS_MACSEC_SECY_RX_SA_EN_SET_CMD,
	NSS_MACSEC_SECY_RX_SA_NEXT_PN_GET_CMD,
	NSS_MACSEC_SECY_RX_SA_DEL_CMD,
	NSS_MACSEC_SECY_RX_SA_DEL_ALL_CMD,
	NSS_MACSEC_SECY_RX_SAK_GET_CMD,
	NSS_MACSEC_SECY_RX_SAK_SET_CMD,
	NSS_MACSEC_SECY_RX_SA_IN_USED_GET_CMD,
	NSS_MACSEC_SECY_RX_REPLAY_PROTECT_GET_CMD,
	NSS_MACSEC_SECY_RX_REPLAY_PROTECT_SET_CMD,
	NSS_MACSEC_SECY_RX_VALIDATE_FRAME_GET_CMD,
	NSS_MACSEC_SECY_RX_VALIDATE_FRAME_SET_CMD,
	NSS_MACSEC_SECY_EXT_REG_GET_CMD,
	NSS_MACSEC_SECY_EXT_REG_SET_CMD,
	NSS_MACSEC_SECY_TX_CTL_FILT_GET_CMD,
	NSS_MACSEC_SECY_TX_CTL_FILT_SET_CMD,
	NSS_MACSEC_SECY_TX_CTL_FILT_CLEAR_CMD,
	NSS_MACSEC_SECY_TX_CTL_FILT_CLEAR_ALL_CMD,
	NSS_MACSEC_SECY_TX_CLASS_LUT_GET_CMD,
	NSS_MACSEC_SECY_TX_CLASS_LUT_SET_CMD,
	NSS_MACSEC_SECY_TX_CLASS_LUT_CLEAR_CMD,
	NSS_MACSEC_SECY_TX_CLASS_LUT_CLEAR_ALL_CMD,
	NSS_MACSEC_SECY_TX_SC_CREATE_CMD,
	NSS_MACSEC_SECY_TX_SC_EN_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_EN_SET_CMD,
	NSS_MACSEC_SECY_TX_SC_DEL_CMD,
	NSS_MACSEC_SECY_TX_SC_DEL_ALL_CMD,
	NSS_MACSEC_SECY_TX_SC_AN_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_AN_SET_CMD,
	NSS_MACSEC_SECY_TX_SC_IN_USED_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_TCI_7_2_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_TCI_7_2_SET_CMD,
	NSS_MACSEC_SECY_TX_SC_CONFIDENTIALITY_OFFSET_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_CONFIDENTIALITY_OFFSET_SET_CMD,
	NSS_MACSEC_SECY_TX_SC_PROTECT_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_PROTECT_SET_CMD,
	NSS_MACSEC_SECY_TX_SC_SCI_GET_CMD,
	NSS_MACSEC_SECY_TX_SA_CREATE_CMD,
	NSS_MACSEC_SECY_TX_SA_EN_GET_CMD,
	NSS_MACSEC_SECY_TX_SA_EN_SET_CMD,
	NSS_MACSEC_SECY_TX_SA_DEL_CMD,
	NSS_MACSEC_SECY_TX_SA_DEL_ALL_CMD,
	NSS_MACSEC_SECY_TX_SA_NEXT_PN_GET_CMD,
	NSS_MACSEC_SECY_TX_SA_NEXT_PN_SET_CMD,
	NSS_MACSEC_SECY_TX_SA_IN_USED_GET_CMD,
	NSS_MACSEC_SECY_TX_SAK_GET_CMD,
	NSS_MACSEC_SECY_TX_SAK_SET_CMD,
	NSS_MACSEC_SECY_SC_SA_MAPPING_MODE_GET_CMD,
	NSS_MACSEC_SECY_SC_SA_MAPPING_MODE_SET_CMD,
	NSS_MACSEC_SECY_CONTROLLED_PORT_EN_GET_CMD,
	NSS_MACSEC_SECY_CONTROLLED_PORT_EN_SET_CMD,
	NSS_MACSEC_SECY_CIPHER_SUITE_GET_CMD,
	NSS_MACSEC_SECY_CIPHER_SUITE_SET_CMD,
	NSS_MACSEC_SECY_MTU_GET_CMD,
	NSS_MACSEC_SECY_MTU_SET_CMD,
	NSS_MACSEC_SECY_ID_GET_CMD,
	NSS_MACSEC_SECY_EN_GET_CMD,
	NSS_MACSEC_SECY_EN_SET_CMD,
	NSS_MACSEC_SECY_XPN_EN_GET_CMD,
	NSS_MACSEC_SECY_XPN_EN_SET_CMD,
	NSS_MACSEC_SECY_RX_SA_NEXT_XPN_GET_CMD,
	NSS_MACSEC_SECY_RX_SA_NEXT_XPN_SET_CMD,
	NSS_MACSEC_SECY_TX_SA_NEXT_XPN_GET_CMD,
	NSS_MACSEC_SECY_TX_SA_NEXT_XPN_SET_CMD,
	NSS_MACSEC_SECY_RX_SC_SSCI_GET_CMD,
	NSS_MACSEC_SECY_RX_SC_SSCI_SET_CMD,
	NSS_MACSEC_SECY_TX_SC_SSCI_GET_CMD,
	NSS_MACSEC_SECY_TX_SC_SSCI_SET_CMD,
	NSS_MACSEC_SECY_RX_SA_KI_GET_CMD,
	NSS_MACSEC_SECY_RX_SA_KI_SET_CMD,
	NSS_MACSEC_SECY_TX_SA_KI_GET_CMD,
	NSS_MACSEC_SECY_TX_SA_KI_SET_CMD,
	NSS_MACSEC_SECY_FLOW_CONTROL_EN_GET_CMD,
	NSS_MACSEC_SECY_FLOW_CONTROL_EN_SET_CMD,
	NSS_MACSEC_SECY_SPECIAL_PKT_CTRL_GET_CMD,
	NSS_MACSEC_SECY_SPECIAL_PKT_CTRL_SET_CMD,
	NSS_MACSEC_SECY_UDF_ETHTYPE_GET_CMD,
	NSS_MACSEC_SECY_UDF_ETHTYPE_SET_CMD,
	NSS_MACSEC_SECY_TX_UDF_FILT_GET_CMD,
	NSS_MACSEC_SECY_TX_UDF_FILT_SET_CMD,
	NSS_MACSEC_SECY_RX_UDF_FILT_GET_CMD,
	NSS_MACSEC_SECY_RX_UDF_FILT_SET_CMD,
	NSS_MACSEC_SECY_TX_UDF_UFILT_CFG_GET_CMD,
	NSS_MACSEC_SECY_TX_UDF_UFILT_CFG_SET_CMD,
	NSS_MACSEC_SECY_RX_UDF_UFILT_CFG_GET_CMD,
	NSS_MACSEC_SECY_RX_UDF_UFILT_CFG_SET_CMD,
	NSS_MACSEC_SECY_TX_UDF_CFILT_CFG_GET_CMD,
	NSS_MACSEC_SECY_TX_UDF_CFILT_CFG_SET_CMD,
	NSS_MACSEC_SECY_RX_SA_NEXT_PN_SET_CMD,
};

struct nss_macsec_secy_interrupt_en_get_cmd {
	u32 secy_id;
	fal_interrupt_en_t p_int_en;
};

struct nss_macsec_secy_interrupt_en_set_cmd {
	u32 secy_id;
	fal_interrupt_en_t p_int_en;
};

struct nss_macsec_secy_interrupt_get_cmd {
	u32 secy_id;
	fal_interrupt_t pint;
};

struct nss_macsec_secy_tx_sc_mib_get_cmd {
	u32 secy_id;
	u32 channel;
	fal_tx_sc_mib_t pmib;
};

struct nss_macsec_secy_tx_sa_mib_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	fal_tx_sa_mib_t pmib;
};

struct nss_macsec_secy_tx_mib_get_cmd {
	u32 secy_id;
	fal_tx_mib_t pmib;
};

struct nss_macsec_secy_rx_sa_mib_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	fal_rx_sa_mib_t pmib;
};

struct nss_macsec_secy_rx_mib_get_cmd {
	u32 secy_id;
	fal_rx_mib_t pmib;
};

struct nss_macsec_secy_tx_mib_clear_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_tx_sc_mib_clear_cmd {
	u32 secy_id;
	u32 channel;
};

struct nss_macsec_secy_tx_sa_mib_clear_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
};

struct nss_macsec_secy_rx_mib_clear_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_rx_sa_mib_clear_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
};

struct nss_macsec_secy_genl_reg_get_cmd {
	u32 secy_id;
	u32 addr;
	u32 pvalue;
};

struct nss_macsec_secy_genl_reg_set_cmd {
	u32 secy_id;
	u32 addr;
	u32 value;
};

struct nss_macsec_secy_rx_ctl_filt_get_cmd {
	u32 secy_id;
	u32 filt_id;
	fal_rx_ctl_filt_t pfilt;
};

struct nss_macsec_secy_rx_ctl_filt_set_cmd {
	u32 secy_id;
	u32 filt_id;
	fal_rx_ctl_filt_t pfilt;
};

struct nss_macsec_secy_rx_ctl_filt_clear_cmd {
	u32 secy_id;
	u32 filt_id;
};

struct nss_macsec_secy_rx_ctl_filt_clear_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_rx_prc_lut_get_cmd {
	u32 secy_id;
	u32 index;
	fal_rx_prc_lut_t pentry;
};

struct nss_macsec_secy_rx_prc_lut_set_cmd {
	u32 secy_id;
	u32 index;
	fal_rx_prc_lut_t pentry;
};

struct nss_macsec_secy_rx_prc_lut_clear_cmd {
	u32 secy_id;
	u32 index;
};

struct nss_macsec_secy_rx_prc_lut_clear_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_rx_sc_create_cmd {
	u32 secy_id;
	u32 channel;
};

struct nss_macsec_secy_rx_sc_en_get_cmd {
	u32 secy_id;
	u32 channel;
	bool penable;
};

struct nss_macsec_secy_rx_sc_en_set_cmd {
	u32 secy_id;
	u32 channel;
	bool enable;
};

struct nss_macsec_secy_rx_sc_del_cmd {
	u32 secy_id;
	u32 channel;
};

struct nss_macsec_secy_rx_sc_del_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_rx_sc_validate_frame_get_cmd {
	u32 secy_id;
	u32 channel;
	fal_rx_sc_validate_frame_e pmode;
};

struct nss_macsec_secy_rx_sc_validate_frame_set_cmd {
	u32 secy_id;
	u32 channel;
	fal_rx_sc_validate_frame_e mode;
};

struct nss_macsec_secy_rx_sc_replay_protect_get_cmd {
	u32 secy_id;
	u32 channel;
	bool penable;
};

struct nss_macsec_secy_rx_sc_replay_protect_set_cmd {
	u32 secy_id;
	u32 channel;
	bool enable;
};

struct nss_macsec_secy_rx_sc_anti_replay_window_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 pwindow;
};

struct nss_macsec_secy_rx_sc_anti_replay_window_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 window;
};

struct nss_macsec_secy_rx_sc_in_used_get_cmd {
	u32 secy_id;
	u32 channel;
	bool p_in_used;
};

struct nss_macsec_secy_rx_sa_create_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
};

struct nss_macsec_secy_rx_sa_en_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	bool penable;
};

struct nss_macsec_secy_rx_sa_en_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	bool enable;
};

struct nss_macsec_secy_rx_sa_next_pn_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 pnpn;
};

struct nss_macsec_secy_rx_sa_del_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
};

struct nss_macsec_secy_rx_sa_del_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_rx_sak_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	fal_rx_sak_t pkey;
};

struct nss_macsec_secy_rx_sak_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	fal_rx_sak_t pkey;
};

struct nss_macsec_secy_rx_sa_in_used_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	bool p_in_used;
};

struct nss_macsec_secy_rx_replay_protect_get_cmd {
	u32 secy_id;
	u32 enable;
};

struct nss_macsec_secy_rx_replay_protect_set_cmd {
	u32 secy_id;
	u32 enable;
};

struct nss_macsec_secy_rx_validate_frame_get_cmd {
	u32 secy_id;
	u32 mode;
};

struct nss_macsec_secy_rx_validate_frame_set_cmd {
	u32 secy_id;
	u32 mode;
};

struct nss_macsec_secy_ext_reg_get_cmd {
	u32 secy_id;
	u32 addr;
	u32 pvalue;
};

struct nss_macsec_secy_ext_reg_set_cmd {
	u32 secy_id;
	u32 addr;
	u32 value;
};

struct nss_macsec_secy_tx_ctl_filt_get_cmd {
	u32 secy_id;
	u32 filt_id;
	fal_tx_ctl_filt_t pfilt;
};

struct nss_macsec_secy_tx_ctl_filt_set_cmd {
	u32 secy_id;
	u32 filt_id;
	fal_tx_ctl_filt_t pfilt;
};

struct nss_macsec_secy_tx_ctl_filt_clear_cmd {
	u32 secy_id;
	u32 filt_id;
};

struct nss_macsec_secy_tx_ctl_filt_clear_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_tx_class_lut_get_cmd {
	u32 secy_id;
	u32 index;
	fal_tx_class_lut_t pentry;
};

struct nss_macsec_secy_tx_class_lut_set_cmd {
	u32 secy_id;
	u32 index;
	fal_tx_class_lut_t pentry;
};

struct nss_macsec_secy_tx_class_lut_clear_cmd {
	u32 secy_id;
	u32 index;
};

struct nss_macsec_secy_tx_class_lut_clear_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_tx_sc_create_cmd {
	u32 secy_id;
	u32 channel;
	u8 psci[16];
	u32 sci_len;
};

struct nss_macsec_secy_tx_sc_en_get_cmd {
	u32 secy_id;
	u32 channel;
	bool penable;
};

struct nss_macsec_secy_tx_sc_en_set_cmd {
	u32 secy_id;
	u32 channel;
	bool enable;
};

struct nss_macsec_secy_tx_sc_del_cmd {
	u32 secy_id;
	u32 channel;
};

struct nss_macsec_secy_tx_sc_del_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_tx_sc_an_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 pan;
};

struct nss_macsec_secy_tx_sc_an_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
};

struct nss_macsec_secy_tx_sc_in_used_get_cmd {
	u32 secy_id;
	u32 channel;
	bool p_in_used;
};

struct nss_macsec_secy_tx_sc_tci_7_2_get_cmd {
	u32 secy_id;
	u32 channel;
	u8 ptci;
};

struct nss_macsec_secy_tx_sc_tci_7_2_set_cmd {
	u32 secy_id;
	u32 channel;
	u8 tci;
};

struct nss_macsec_secy_tx_sc_confidentiality_offset_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 poffset;
};

struct nss_macsec_secy_tx_sc_confidentiality_offset_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 offset;
};

struct nss_macsec_secy_tx_sc_protect_get_cmd {
	u32 secy_id;
	u32 channel;
	bool penable;
};

struct nss_macsec_secy_tx_sc_protect_set_cmd {
	u32 secy_id;
	u32 channel;
	bool enable;
};

struct nss_macsec_secy_tx_sc_sci_get_cmd {
	u32 secy_id;
	u32 channel;
	u8 psci[16];
	u32 sci_len;
};

struct nss_macsec_secy_tx_sa_create_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
};

struct nss_macsec_secy_tx_sa_en_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	bool penable;
};

struct nss_macsec_secy_tx_sa_en_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	bool enable;
};

struct nss_macsec_secy_tx_sa_del_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
};

struct nss_macsec_secy_tx_sa_del_all_cmd {
	u32 secy_id;
};

struct nss_macsec_secy_tx_sa_next_pn_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 p_next_pn;
};

struct nss_macsec_secy_tx_sa_next_pn_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 next_pn;
};

struct nss_macsec_secy_tx_sa_in_used_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	bool p_in_used;
};

struct nss_macsec_secy_tx_sak_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	fal_tx_sak_t pentry;
};

struct nss_macsec_secy_tx_sak_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	fal_tx_sak_t pentry;
};

struct nss_macsec_secy_sc_sa_mapping_mode_get_cmd {
	u32 secy_id;
	fal_sc_sa_mapping_mode_e pmode;
};

struct nss_macsec_secy_sc_sa_mapping_mode_set_cmd {
	u32 secy_id;
	fal_sc_sa_mapping_mode_e mode;
};

struct nss_macsec_secy_controlled_port_en_get_cmd {
	u32 secy_id;
	bool penable;
};

struct nss_macsec_secy_controlled_port_en_set_cmd {
	u32 secy_id;
	bool enable;
};

struct nss_macsec_secy_cipher_suite_get_cmd {
	u32 secy_id;
	fal_cipher_suite_e p_cipher_suite;
};

struct nss_macsec_secy_cipher_suite_set_cmd {
	u32 secy_id;
	fal_cipher_suite_e cipher_suite;
};

struct nss_macsec_secy_mtu_get_cmd {
	u32 secy_id;
	u32 pmtu;
};

struct nss_macsec_secy_mtu_set_cmd {
	u32 secy_id;
	u32 mtu;
};

struct nss_macsec_secy_id_get_cmd {
	u8 dev_name[16];
	u32 secy_id;
};

struct nss_macsec_secy_en_get_cmd {
	u32 secy_id;
	bool penable;
};

struct nss_macsec_secy_en_set_cmd {
	u32 secy_id;
	bool enable;
};
struct nss_macsec_secy_xpn_en_get_cmd {
	u32 secy_id;
	bool penable;
};

struct nss_macsec_secy_xpn_en_set_cmd {
	u32 secy_id;
	bool enable;
};

struct nss_macsec_secy_rx_sa_next_xpn_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 p_next_xpn;
};

struct nss_macsec_secy_rx_sa_next_xpn_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 next_xpn;
};

struct nss_macsec_secy_tx_sa_next_xpn_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 p_next_xpn;
};

struct nss_macsec_secy_tx_sa_next_xpn_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 next_xpn;
};

struct nss_macsec_secy_rx_sc_ssci_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 ssci;
};

struct nss_macsec_secy_rx_sc_ssci_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 ssci;
};

struct nss_macsec_secy_tx_sc_ssci_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 ssci;
};

struct nss_macsec_secy_tx_sc_ssci_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 ssci;
};

struct nss_macsec_secy_rx_sa_ki_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	struct fal_rx_sa_ki_t pki;
};

struct nss_macsec_secy_rx_sa_ki_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	struct fal_rx_sa_ki_t pki;
};

struct nss_macsec_secy_tx_sa_ki_get_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	struct fal_tx_sa_ki_t pki;
};

struct nss_macsec_secy_tx_sa_ki_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	struct fal_tx_sa_ki_t pki;
};

struct nss_macsec_secy_flow_control_en_get_cmd {
	u32 secy_id;
	bool penable;
};

struct nss_macsec_secy_flow_control_en_set_cmd {
	u32 secy_id;
	bool enable;
};

struct nss_macsec_secy_special_pkt_ctrl_get_cmd {
	u32 secy_id;
	enum fal_packet_type_t type;
	enum fal_packet_action_t action;
};

struct nss_macsec_secy_special_pkt_ctrl_set_cmd {
	u32 secy_id;
	enum fal_packet_type_t type;
	enum fal_packet_action_t action;

};

struct nss_macsec_secy_udf_ethtype_get_cmd {
	u32 secy_id;
	bool enable;
	u16 type;
};

struct nss_macsec_secy_udf_ethtype_set_cmd {
	u32 secy_id;
	bool enable;
	u16 type;
};

struct nss_macsec_secy_tx_udf_filt_get_cmd {
	u32 secy_id;
	u32 filt_id;
	struct fal_tx_udf_filt_t pfilt;
};

struct nss_macsec_secy_tx_udf_filt_set_cmd {
	u32 secy_id;
	u32 filt_id;
	struct fal_tx_udf_filt_t pfilt;
};
struct nss_macsec_secy_rx_udf_filt_get_cmd {
	u32 secy_id;
	u32 filt_id;
	struct fal_rx_udf_filt_t pfilt;
};

struct nss_macsec_secy_rx_udf_filt_set_cmd {
	u32 secy_id;
	u32 filt_id;
	struct fal_rx_udf_filt_t pfilt;
};

struct nss_macsec_secy_tx_udf_ufilt_cfg_get_cmd {
	u32 secy_id;
	struct fal_tx_udf_filt_cfg_t pcfg;
};

struct nss_macsec_secy_tx_udf_ufilt_cfg_set_cmd {
	u32 secy_id;
	struct fal_tx_udf_filt_cfg_t pcfg;
};
struct nss_macsec_secy_rx_udf_ufilt_cfg_get_cmd {
	u32 secy_id;
	struct fal_rx_udf_filt_cfg_t pcfg;
};

struct nss_macsec_secy_rx_udf_ufilt_cfg_set_cmd {
	u32 secy_id;
	struct fal_rx_udf_filt_cfg_t pcfg;
};

struct nss_macsec_secy_tx_udf_cfilt_cfg_get_cmd {
	u32 secy_id;
	struct fal_tx_udf_filt_cfg_t pcfg;
};

struct nss_macsec_secy_tx_udf_cfilt_cfg_set_cmd {
	u32 secy_id;
	struct fal_tx_udf_filt_cfg_t pcfg;
};
struct nss_macsec_secy_rx_sa_next_pn_set_cmd {
	u32 secy_id;
	u32 channel;
	u32 an;
	u32 next_pn;
};
unsigned int nss_macsec_fal_msg_handle(struct sdk_msg_header *header);

#endif
