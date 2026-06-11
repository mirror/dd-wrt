/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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


#ifndef __QCA808X_MACSECY_H
#define __QCA808X_MACSECY_H

#include "nss_macsec_types.h"

#define SECY_SC_IDX_MAX           15
#define SECY_SA_IDX_MAX           1
#define SAK_LEN_LEGACY            0
#define SAK_LEN_128BITS           16
#define SAK_LEN_256BITS           32
#define SECY_FILTER_MAX_INDEX     0x3
#define SECY_AN_TO_SA_MAPPING(an) (an%2)


#define SC_BIND_MASK_DA           0x00000001
#define SC_BIND_MASK_SA           0x00000002
#define SC_BIND_MASK_ETHERTYPE    0x00000004
#define SC_BIND_MASK_OUTER_VLAN   0x00000008
#define SC_BIND_MASK_INNER_VLAN   0x00000010
#define SC_BIND_MASK_BCAST        0x00000020
#define SC_BIND_MASK_TCI          0x00000040
#define SC_BIND_MASK_SCI          0x00000080
#define SC_BIND_MASK_VALID        0x00008000

#define MACSEC_SCB_EN           0x10
#define MACSEC_INCLUDE_SCI_EN   0x20
#define MACSEC_ES_EN            0x40
#define MACSEC_AES256_ENABLE    0x1
#define MACSEC_XPN_ENABLE       0x1

#define AN_MASK                 0x3


/** SecY loopback type */
enum secy_loopback_type_t {
	MACSEC_NORMAL     = 0x0,
	MACSEC_PHY_LB     = 0x1,
	MACSEC_SWITCH_LB  = 0x2,
	MACSEC_BYPASS     = 0x3
};

struct secy_sci_t {
	u8  addr[6];
	u16 port;
};

struct secy_mac_t {
	u8   addr[6];
};

struct secy_rx_sc_policy_action_t {
	u32   rx_sc_index;
	u8    decryption_offset;
};

struct secy_rx_sc_policy_rule_t {
	bool  rule_valid;
	u32   rule_mask;
	struct secy_mac_t      mac_da;
	struct secy_mac_t      mac_sa;
	u16   ethtype;
	u16   outer_vlanid;
	u16   inner_vlanid;
	struct secy_sci_t rx_sci;
	u8    rx_tci;
	bool  bc_flag;
	struct secy_rx_sc_policy_action_t action;
};

struct secy_tx_sc_policy_action_t {
	u32   tx_sc_index;
	struct secy_sci_t tx_sci;
	u8   tx_tci;
	u8    encryption_offset;
};

struct secy_tx_sc_policy_rule_t {
	bool  rule_valid;
	u32   rule_mask;
	struct secy_mac_t      mac_da;
	struct secy_mac_t      mac_sa;
	u16   ethtype;
	u16   outer_vlanid;
	u16   inner_vlanid;
	bool  bc_flag;
	struct secy_tx_sc_policy_action_t action;
};


g_error_t qca808x_secy_use_es_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_use_scb_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_include_sci_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_controlled_port_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_controlled_port_en_get(struct macsec_port *port, bool *enable);
g_error_t qca808x_secy_clock_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_loopback_set(struct macsec_port *port, enum secy_loopback_type_t type);
g_error_t qca808x_secy_loopback_get(struct macsec_port *port, enum secy_loopback_type_t *type);
g_error_t qca808x_secy_mtu_set(struct macsec_port *port, u32 len);
g_error_t qca808x_secy_mtu_get(struct macsec_port *port, u32 *len);
g_error_t qca808x_secy_shadow_register_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_cipher_suite_set(struct macsec_port *port, enum secy_cipher_suite_t cipher_suite);
g_error_t qca808x_secy_cipher_suite_get(struct macsec_port *port, enum secy_cipher_suite_t *cipher_suite);
g_error_t qca808x_secy_sram_dbg_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_sram_dbg_en_get(struct macsec_port *port, bool *enable);
g_error_t qca808x_secy_rx_sc_en_zone_set(struct macsec_port *port, u32 sc_index, bool enable, u16 rule_mask);
g_error_t qca808x_secy_rx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable);
g_error_t qca808x_secy_rx_sc_policy_set(struct macsec_port *port, u32 rule_index, struct secy_rx_sc_policy_rule_t *rule);
g_error_t qca808x_secy_rx_sc_policy_get(struct macsec_port *port, u32 rule_index, struct secy_rx_sc_policy_rule_t *rule);
g_error_t qca808x_secy_rx_sa_npn_set(struct macsec_port *port, u32 sc_index, u32 an, u32 next_pn);
g_error_t qca808x_secy_rx_sa_npn_get(struct macsec_port *port, u32 sc_index, u32 an, u32 *next_pn);
g_error_t qca808x_secy_rx_replay_protect_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_rx_replay_protect_en_get(struct macsec_port *port, bool *enable);
g_error_t qca808x_secy_rx_replay_window_set(struct macsec_port *port, u32 window);
g_error_t qca808x_secy_rx_replay_window_get(struct macsec_port *port, u32 *window);
g_error_t qca808x_secy_rx_validate_frame_set(struct macsec_port *port, enum secy_vf_t value);
g_error_t qca808x_secy_rx_validate_frame_get(struct macsec_port *port, enum secy_vf_t *value);
g_error_t qca808x_secy_rx_sa_create(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca808x_secy_rx_sak_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca808x_secy_rx_sak_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca808x_secy_rx_sa_en_set(struct macsec_port *port, u32 sc_index, u32 an, bool enable);
g_error_t qca808x_secy_rx_sa_en_get(struct macsec_port *port, u32 sc_index, u32 an, bool *enable);
g_error_t qca808x_secy_rx_sa_del(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca808x_secy_tx_tci_offset_zone_set(struct macsec_port *port, u32 sc_index, u8 tci, u8 offset);
g_error_t qca808x_secy_tx_tci_get(struct macsec_port *port, u32 sc_index, u8 *tci);
g_error_t qca808x_secy_tx_confidentiality_offset_get(struct macsec_port *port, u32 sc_index, u8 *offset);
g_error_t qca808x_secy_tx_sci_set(struct macsec_port *port, u32 sc_index, struct secy_sci_t *tx_sci);
g_error_t qca808x_secy_tx_sci_get(struct macsec_port *port, u32 sc_index, struct secy_sci_t *tx_sci);
g_error_t qca808x_secy_tx_sc_en_zone_set(struct macsec_port *port, u32 sc_index, bool enable, u16 rule_mask);
g_error_t qca808x_secy_tx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable);
g_error_t qca808x_secy_tx_sc_policy_set(struct macsec_port *port, u32 rule_index, struct secy_tx_sc_policy_rule_t *rule);
g_error_t qca808x_secy_tx_sc_policy_get(struct macsec_port *port, u32 rule_index, struct secy_tx_sc_policy_rule_t *rule);
g_error_t qca808x_secy_tx_sa_npn_set(struct macsec_port *port, u32 sc_index, u32 an, u32 next_pn);
g_error_t qca808x_secy_tx_sa_npn_get(struct macsec_port *port, u32 sc_index, u32 an, u32 *next_pn);
g_error_t qca808x_secy_tx_protect_frames_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_tx_protect_frames_en_get(struct macsec_port *port, bool *enable);
g_error_t qca808x_secy_tx_sak_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca808x_secy_tx_sak_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca808x_secy_tx_sa_en_set(struct macsec_port *port, u32 sc_index, u32 an, bool enable);
g_error_t qca808x_secy_tx_sa_en_get(struct macsec_port *port, u32 sc_index, u32 an, bool *enable);
g_error_t qca808x_secy_tx_sc_an_set(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca808x_secy_tx_sc_an_get(struct macsec_port *port, u32 sc_index, u32 *an);
g_error_t qca808x_secy_tx_sa_create(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca808x_secy_tx_sa_del(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca808x_secy_tx_sa_mib_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_tx_sa_mib_t *mib);
g_error_t qca808x_secy_tx_sc_mib_get(struct macsec_port *port, u32 sc_index, struct secy_tx_sc_mib_t *mib);
g_error_t qca808x_secy_tx_mib_get(struct macsec_port *port, struct secy_tx_mib_t *mib);
g_error_t qca808x_secy_rx_sa_mib_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_rx_sa_mib_t *mib);
g_error_t qca808x_secy_rx_sc_mib_get(struct macsec_port *port, u32 sc_index, struct secy_rx_sc_mib_t *mib);
g_error_t qca808x_secy_rx_mib_get(struct macsec_port *port, struct secy_rx_mib_t *mib);
g_error_t qca808x_secy_xpn_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_xpn_en_get(struct macsec_port *port, bool *enable);
g_error_t qca808x_secy_rx_sa_xpn_set(struct macsec_port *port, u32 sc_index, u32 an, u32 next_pn);
g_error_t qca808x_secy_rx_sa_xpn_get(struct macsec_port *port, u32 sc_index, u32 an, u32 *next_pn);
g_error_t qca808x_secy_tx_sa_xpn_set(struct macsec_port *port, u32 sc_index, u32 an, u32 next_pn);
g_error_t qca808x_secy_tx_sa_xpn_get(struct macsec_port *port, u32 sc_index, u32 an, u32 *next_pn);
g_error_t qca808x_secy_rx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci);
g_error_t qca808x_secy_rx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci);
g_error_t qca808x_secy_tx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci);
g_error_t qca808x_secy_tx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci);
g_error_t qca808x_secy_rx_sa_ki_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca808x_secy_rx_sa_ki_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca808x_secy_tx_sa_ki_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca808x_secy_tx_sa_ki_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca808x_secy_flow_control_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_flow_control_en_get(struct macsec_port *port, bool *enable);
g_error_t qca808x_secy_special_pkt_ctrl_set(struct macsec_port *port, enum secy_packet_type_t packet_type, enum secy_packet_action_t action);
g_error_t qca808x_secy_special_pkt_ctrl_get(struct macsec_port *port, enum secy_packet_type_t packet_type, enum secy_packet_action_t *action);
g_error_t qca808x_secy_udf_ethtype_set(struct macsec_port *port, bool enable, u16 type);
g_error_t qca808x_secy_udf_ethtype_get(struct macsec_port *port, bool *enable, u16 *type);
g_error_t qca808x_secy_forward_az_pattern_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_tx_udf_ufilt_cfg_set(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca808x_secy_tx_udf_ufilt_cfg_get(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca808x_secy_tx_udf_cfilt_cfg_set(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca808x_secy_tx_udf_cfilt_cfg_get(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca808x_secy_rx_udf_ufilt_cfg_set(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca808x_secy_rx_udf_ufilt_cfg_get(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca808x_secy_tx_udf_filt_set(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca808x_secy_tx_udf_filt_get(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca808x_secy_rx_udf_filt_set(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca808x_secy_rx_udf_filt_get(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca808x_secy_tx_ctl_filt_en_set(struct macsec_port *port, bool enable);
g_error_t qca808x_secy_rx_ctl_filt_en_set(struct macsec_port *port, bool enable);



#endif /* __QCA808X_MACSECY_H */

