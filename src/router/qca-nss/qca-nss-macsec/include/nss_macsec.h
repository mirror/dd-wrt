/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _NSS_MACSEC_MACSEC_H_
#define _NSS_MACSEC_MACSEC_H_


#define QCA8081_PHY      0x004DD100
#define QCA8081_PHY_V1_1 0x004DD101
#define QCA8084_PHY      0x004DD180

#define MACSEC_DEVICE_MAXNUM 5
#define INVALID_DEVICE_ID  0xFFFFFFFF

#define SECY_LUT_MAX 16
#define SECY_AN_IDX_MAX     3
#define SECY_PN_QUERY_TIMES 4
#define SECY_PN_CHANNEL_MAX           \
	(SECY_LUT_MAX * (SECY_AN_IDX_MAX + 1))

enum secy_cipher_suite_t {
	SECY_CIPHER_SUITE_128 = 0,
	SECY_CIPHER_SUITE_256 = 1,
	SECY_CIPHER_SUITE_XPN_128,
	SECY_CIPHER_SUITE_XPN_256,
	SECY_CIPHER_SUITE_MAX
};

enum macsec_sc_sa_mapping_mode_t {
	MACSEC_SC_SA_MAP_1_1 = 0,
	MACSEC_SC_SA_MAP_1_2 = 1,
	MACSEC_SC_SA_MAP_1_4 = 2,
	MACSEC_SC_SA_MAP_MAX = 3
};

enum macsec_type {
	MACSEC_IN_PHY = 0,
	MACSEC_IN_MAC = 1,
	__MACSEC_TYPE_END,
	MACSEC_TYPE_MAX = __MACSEC_TYPE_END - 1,
};

struct macsec_port {
	union {
		struct net_device *netdev;
		struct phy_device *phydev;
	};
	enum macsec_type type;
};

enum secy_rx_prc_lut_action_t {
	SECY_RX_PRC_ACTION_PROCESS,
	SECY_RX_PRC_ACTION_PROCESS_WITH_SECTAG,
	SECY_RX_PRC_ACTION_BYPASS,
	SECY_RX_PRC_ACTION_DROP,
	SECY_RX_PRC_ACTION_MAX
};

struct secy_rx_prc_lut_t {
	bool valid;
	bool bc_flag;
	enum secy_rx_prc_lut_action_t action;
	u32 channel;
	u8 sci[8];
	u8 da[6];
	u8 sa[6];
	u16 ether_type;
	u8 tci;
	u8 offset;
	u16 outer_vlanid;
	u16 inner_vlanid;
	u32 rule_mask;
};

enum secy_cofidentiality_offset_t {
	SECY_CONFIDENTIALITY_OFFSET_00 = 0, /**< 0  byte */
	SECY_CONFIDENTIALITY_OFFSET_30 = 1, /**< 30 bytes */
	SECY_CONFIDENTIALITY_OFFSET_50 = 2 /**< 50 bytes */
};

enum secy_tx_class_lut_action_t {
	SECY_TX_CLASS_ACTION_FORWARD,
	SECY_TX_CLASS_ACTION_BYPASS,
	SECY_TX_CLASS_ACTION_DROP,
	SECY_TX_CLASS_ACTION_MAX
};

struct secy_tx_class_lut_t {
	bool valid;
	bool bc_flag;
	enum secy_tx_class_lut_action_t action;
	u32 channel;
	u8 sci[8];
	u8 da[6];
	u8 sa[6];
	u16 ether_type;
	u8 tci;
	u8 offset;
	u16 outer_vlanid;
	u16 inner_vlanid;
	u32 rule_mask;
};

struct secy_pn_table_t {
	u64   rx_sa_pn;
	u64   tx_sa_pn;
};

enum secy_ctl_match_type_t {
	SECY_CTL_COMPARE_NO,
	SECY_CTL_COMPARE_DA,
	SECY_CTL_COMPARE_SA,
	SECY_CTL_COMPARE_HALF_DA_SA,
	SECY_CTL_COMPARE_ETHER_TYPE,
	SECY_CTL_COMPARE_DA_ETHER_TYPE,
	SECY_CTL_COMPARE_SA_ETHER_TYPE,
	SECY_CTL_COMPARE_DA_RANGE,
	SECY_CTL_COMPARE_MAX
};

struct secy_ctl_filt_t {
	bool bypass;
	enum secy_ctl_match_type_t match_type;
	u16 match_mask;
	u16 ether_type_da_range;
	u8 sa_da_addr[6];
};

struct qca_macsec_device_t {
	struct  macsec_port port;
	enum    macsec_sc_sa_mapping_mode_t sc_sa_mapping_mode;
	u32     rx_ssci[SECY_LUT_MAX];
	u32     tx_ssci[SECY_LUT_MAX];
	struct  secy_rx_prc_lut_t   rx_lut[SECY_LUT_MAX];
	struct  secy_tx_class_lut_t tx_lut[SECY_LUT_MAX];
	struct  secy_pn_table_t     secy_pn_table[SECY_PN_CHANNEL_MAX];
	struct  secy_ctl_filt_t     tx_ctl_filt;
	struct  secy_ctl_filt_t     rx_ctl_filt;
	void    *offload_secy_cfg;
};

struct secy_sak_t {
	u8 sak[16];
	u8 sak1[16];
	u32 len;
};

struct secy_sa_ki_t {
	u8 ki[16];
};

struct secy_tx_sa_mib_t {
	u64 hit_drop_redirect;
	u64 protected2_pkts;
	u64 protected_pkts;
	u64 encrypted_pkts;
};

struct secy_tx_sc_mib_t {
	u64 protected_pkts;
	u64 encrypted_pkts;
	u64 protected_octets;
	u64 encrypted_octets;
};

struct secy_tx_mib_t {
	u64 ctl_pkts;
	u64 unknown_sa_pkts;
	u64 untagged_pkts;
	u64 too_long;
	u64 ecc_error_pkts;
	u64 unctrl_hit_drop_redir_pkts;
};

struct secy_rx_sa_mib_t {
	u64 untagged_hit_pkts;
	u64 hit_drop_redir_pkts;
	u64 not_using_sa;
	u64 unused_sa;
	u64 not_valid_pkts;
	u64 invalid_pkts;
	u64 ok_pkts;
};

struct secy_rx_sc_mib_t {
	u64 late_pkts;
	u64 delayed_pkts;
	u64 unchecked_pkts;
	u64 validated_octets;
	u64 decrypted_octets;
};

struct secy_rx_mib_t {
	u64 unctrl_prt_tx_octets;
	u64 ctl_pkts;
	u64 tagged_miss_pkts;
	u64 untagged_hit_pkts;
	u64 notag_pkts;
	u64 untagged_pkts;
	u64 bad_tag_pkts;
	u64 no_sci_pkts;
	u64 unknown_sci_pkts;
	u64 ctrl_prt_pass_pkts;
	u64 unctrl_prt_pass_pkts;
	u64 ctrl_prt_fail_pkts;
	u64 unctrl_prt_fail_pkts;
	u64 too_long_packets;
	u64 igpoc_ctl_pkts;
	u64 ecc_error_pkts;
};

enum secy_vf_t {
	STRICT    = 0,
	CHECKED   = 1,
	DISABLED  = 2
};

enum secy_packet_action_t {
	PACKET_CRYPTO_OR_DISCARD = 0X0,
	PACKET_CRYPTO_OR_UPLOAD  = 0X1,
	PACKET_PLAIN_OR_DISCARD  = 0X2,
	PACKET_PLAIN_OR_UPLOAD   = 0X3
};

/** SecY special packet type */
enum secy_packet_type_t {
	PACKET_STP   = 0,   /*< Spanning Tree Protocol Packet */
	PACKET_CDP   = 1,   /*< Cisco Discovery Protocol Packet */
	PACKET_LLDP  = 2   /*< Link Layer Discovery Protocol Packet */
};

enum secy_udf_filt_type_t {
	SECY_FILTER_ANY_PACKET = 0,
	SECY_FILTER_IP_PACKET = 1,
	SECY_FILTER_TCP_PACKET = 2
};

enum secy_udf_filt_op_t {
	SECY_FILTER_OPERATOR_EQUAL = 0,
	SECY_FILTER_OPERATOR_LESS = 1
};

struct secy_udf_filt_t {
	u16 udf_field0;
	u16 udf_field1;
	u16 udf_field2;
	u16 mask;
	enum secy_udf_filt_type_t type;
	enum secy_udf_filt_op_t operator;
	u16 offset;
};

enum secy_udf_filt_cfg_pattern_t {
	SECY_FILTER_PATTERN_AND = 0,
	SECY_FILTER_PATTERN_OR = 1,
	SECY_FILTER_PATTERN_XOR = 2
};

struct secy_udf_filt_cfg_t {
	bool enable;
	u16 priority;
	u16 inverse;
	enum secy_udf_filt_cfg_pattern_t pattern0;
	enum secy_udf_filt_cfg_pattern_t pattern1;
	enum secy_udf_filt_cfg_pattern_t pattern2;
};


void qca_macsec_device_cleanup(void);
int qca_macsec_device_attach(void *dev);
int qca_macsec_device_detach(void *dev);
u32 macsec_get_device_id(struct phy_device *phydev);
struct macsec_port *macsec_port_get_by_device_id(u32 dev_id);
g_error_t qca_macsec_channel_number_get(struct macsec_port *port, u32 *number);
g_error_t qca_macsec_cipher_suite_set(struct macsec_port *port, enum secy_cipher_suite_t cipher_suite);
g_error_t qca_macsec_cipher_suite_get(struct macsec_port *port, enum secy_cipher_suite_t *cipher_suite);
g_error_t qca_macsec_sc_sa_mapping_mode_set(struct macsec_port *port, enum macsec_sc_sa_mapping_mode_t mode);
g_error_t qca_macsec_sc_sa_mapping_mode_get(struct macsec_port *port, enum macsec_sc_sa_mapping_mode_t *pmode);
g_error_t qca_macsec_xpn_en_set(struct macsec_port *port, bool enable);
g_error_t qca_macsec_xpn_en_get(struct macsec_port *port, bool *enable);
g_error_t qca_macsec_mtu_set(struct macsec_port *port, u32 len);
g_error_t qca_macsec_mtu_get(struct macsec_port *port, u32 *len);
g_error_t qca_macsec_special_pkt_ctrl_set(struct macsec_port *port, enum secy_packet_type_t packet_type, enum secy_packet_action_t action);
g_error_t qca_macsec_special_pkt_ctrl_get(struct macsec_port *port, enum secy_packet_type_t packet_type, enum secy_packet_action_t *action);
g_error_t qca_macsec_udf_ethtype_set(struct macsec_port *port, bool enable, u16 type);
g_error_t qca_macsec_udf_ethtype_get(struct macsec_port *port, bool *enable, u16 *type);
g_error_t qca_macsec_flow_control_en_set(struct macsec_port *port, bool enable);
g_error_t qca_macsec_flow_control_en_get(struct macsec_port *port, bool *enable);
g_error_t qca_macsec_en_set(struct macsec_port *port, bool enable);
g_error_t qca_macsec_en_get(struct macsec_port *port, bool *enable);
g_error_t qca_macsec_controlled_port_en_set(struct macsec_port *port, bool enable);
g_error_t qca_macsec_controlled_port_en_get(struct macsec_port *port, bool *enable);
g_error_t qca_macsec_rx_sa_npn_set(struct macsec_port *port, u32 sc_index, u32 an, u64 next_pn);
g_error_t qca_macsec_rx_sa_npn_get(struct macsec_port *port, u32 sc_index, u32 an, u64 *next_pn);
g_error_t qca_macsec_rx_sa_create(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca_macsec_rx_sa_del(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca_macsec_rx_sa_del_all(struct macsec_port *port);
g_error_t qca_macsec_rx_sa_in_used_get(struct macsec_port *port, u32 sc_index, u32 an, bool *p_in_used);
g_error_t qca_macsec_rx_sa_en_set(struct macsec_port *port, u32 sc_index, u32 an, bool enable);
g_error_t qca_macsec_rx_sa_en_get(struct macsec_port *port, u32 sc_index, u32 an, bool *enable);
g_error_t qca_macsec_rx_sak_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca_macsec_rx_sak_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca_macsec_rx_sa_ki_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca_macsec_rx_sa_ki_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca_macsec_rx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci);
g_error_t qca_macsec_rx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci);
g_error_t qca_macsec_rx_prc_lut_set(struct macsec_port *port, u32 sc_index, struct secy_rx_prc_lut_t *pentry);
g_error_t qca_macsec_rx_prc_lut_get(struct macsec_port *port, u32 sc_index, struct secy_rx_prc_lut_t *pentry);
g_error_t qca_macsec_rx_sc_in_used_get(struct macsec_port *port, u32 sc_index, bool *enable);
g_error_t qca_macsec_rx_sc_en_set(struct macsec_port *port, u32 sc_index, bool enable);
g_error_t qca_macsec_rx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable);
g_error_t qca_macsec_rx_sc_create(struct macsec_port *port, u32 sc_index);
g_error_t qca_macsec_rx_sc_del(struct macsec_port *port, u32 sc_index);
g_error_t qca_macsec_rx_sc_del_all(struct macsec_port *port);
g_error_t qca_macsec_rx_prc_lut_clear(struct macsec_port *port, u32 sc_index);
g_error_t qca_macsec_rx_prc_lut_clear_all(struct macsec_port *port);
g_error_t qca_macsec_rx_validate_frame_set(struct macsec_port *port, enum secy_vf_t value);
g_error_t qca_macsec_rx_validate_frame_get(struct macsec_port *port, enum secy_vf_t *value);
g_error_t qca_macsec_rx_replay_protect_en_set(struct macsec_port *port, bool enable);
g_error_t qca_macsec_rx_replay_protect_en_get(struct macsec_port *port, bool *enable);
g_error_t qca_macsec_rx_replay_window_set(struct macsec_port *port, u32 window);
g_error_t qca_macsec_rx_replay_window_get(struct macsec_port *port, u32 *window);
g_error_t qca_macsec_rx_sa_mib_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_rx_sa_mib_t *mib);
g_error_t qca_macsec_rx_sc_mib_get(struct macsec_port *port, u32 sc_index, struct secy_rx_sc_mib_t *mib);
g_error_t qca_macsec_rx_mib_get(struct macsec_port *port, struct secy_rx_mib_t *mib);
g_error_t qca_macsec_rx_sa_mib_clear(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca_macsec_rx_sc_mib_clear(struct macsec_port *port, u32 sc_index);
g_error_t qca_macsec_rx_mib_clear(struct macsec_port *port);
g_error_t qca_macsec_rx_ctl_filt_set(struct macsec_port *port, u32 index, struct secy_ctl_filt_t *secy_filt);
g_error_t qca_macsec_rx_ctl_filt_get(struct macsec_port *port, u32 index, struct secy_ctl_filt_t *secy_filt);
g_error_t qca_macsec_rx_ctl_filt_clear(struct macsec_port *port, u32 index);
g_error_t qca_macsec_rx_ctl_filt_clear_all(struct macsec_port *port);
g_error_t qca_macsec_rx_udf_filt_set(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca_macsec_rx_udf_filt_get(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca_macsec_rx_udf_ufilt_cfg_set(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca_macsec_rx_udf_ufilt_cfg_get(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca_macsec_tx_sa_npn_set(struct macsec_port *port, u32 sc_index, u32 an, u64 next_pn);
g_error_t qca_macsec_tx_sa_npn_get(struct macsec_port *port, u32 sc_index, u32 an, u64 *next_pn);
g_error_t qca_macsec_tx_sa_create(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca_macsec_tx_sa_del(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca_macsec_tx_sa_del_all(struct macsec_port *port);
g_error_t qca_macsec_tx_sa_in_used_get(struct macsec_port *port, u32 sc_index, u32 an, bool *p_in_used);
g_error_t qca_macsec_tx_sa_en_set(struct macsec_port *port, u32 sc_index, u32 an, bool enable);
g_error_t qca_macsec_tx_sa_en_get(struct macsec_port *port, u32 sc_index, u32 an, bool *enable);
g_error_t qca_macsec_tx_sak_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca_macsec_tx_sak_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sak_t *key);
g_error_t qca_macsec_tx_sc_an_set(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca_macsec_tx_sc_an_get(struct macsec_port *port, u32 sc_index, u32 *an);
g_error_t qca_macsec_tx_sa_ki_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca_macsec_tx_sa_ki_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier);
g_error_t qca_macsec_tx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci);
g_error_t qca_macsec_tx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci);
g_error_t qca_macsec_tx_class_lut_set(struct macsec_port *port, u32 sc_index, struct secy_tx_class_lut_t *pentry);
g_error_t qca_macsec_tx_class_lut_get(struct macsec_port *port, u32 sc_index, struct secy_tx_class_lut_t *pentry);
g_error_t qca_macsec_tx_sc_in_used_get(struct macsec_port *port, u32 sc_index, bool *enable);
g_error_t qca_macsec_tx_sc_en_set(struct macsec_port *port, u32 sc_index, bool enable);
g_error_t qca_macsec_tx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable);
g_error_t qca_macsec_tx_sc_create(struct macsec_port *port, u32 sc_index, u8 *sci, u32 len);
g_error_t qca_macsec_tx_sc_del(struct macsec_port *port, u32 sc_index);
g_error_t qca_macsec_tx_sc_del_all(struct macsec_port *port);
g_error_t qca_macsec_tx_class_lut_clear(struct macsec_port *port, u32 sc_index);
g_error_t qca_macsec_tx_class_lut_clear_all(struct macsec_port *port);
g_error_t qca_macsec_tx_sc_confidentiality_offset_set(struct macsec_port *port, u32 sc_index, enum secy_cofidentiality_offset_t offset);
g_error_t qca_macsec_tx_sc_confidentiality_offset_get(struct macsec_port *port, u32 sc_index, enum secy_cofidentiality_offset_t *offset);
g_error_t qca_macsec_tx_sc_tci_7_2_set(struct macsec_port *port, u32 sc_index, u8 tci);
g_error_t qca_macsec_tx_sc_tci_7_2_get(struct macsec_port *port, u32 sc_index, u8 *tci);
g_error_t qca_macsec_tx_protect_frames_en_set(struct macsec_port *port, bool enable);
g_error_t qca_macsec_tx_protect_frames_en_get(struct macsec_port *port, bool *enable);
g_error_t qca_macsec_tx_sa_mib_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_tx_sa_mib_t *mib);
g_error_t qca_macsec_tx_sc_mib_get(struct macsec_port *port, u32 sc_index, struct secy_tx_sc_mib_t *mib);
g_error_t qca_macsec_tx_mib_get(struct macsec_port *port, struct secy_tx_mib_t *mib);
g_error_t qca_macsec_tx_sa_mib_clear(struct macsec_port *port, u32 sc_index, u32 an);
g_error_t qca_macsec_tx_sc_mib_clear(struct macsec_port *port, u32 sc_index);
g_error_t qca_macsec_tx_mib_clear(struct macsec_port *port);
g_error_t qca_macsec_tx_ctl_filt_set(struct macsec_port *port, u32 index, struct secy_ctl_filt_t *secy_filt);
g_error_t qca_macsec_tx_ctl_filt_get(struct macsec_port *port, u32 index, struct secy_ctl_filt_t *secy_filt);
g_error_t qca_macsec_tx_ctl_filt_clear(struct macsec_port *port, u32 index);
g_error_t qca_macsec_tx_ctl_filt_clear_all(struct macsec_port *port);
g_error_t qca_macsec_tx_udf_filt_set(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca_macsec_tx_udf_filt_get(struct macsec_port *port, u32 index, struct secy_udf_filt_t *filter);
g_error_t qca_macsec_tx_udf_ufilt_cfg_set(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca_macsec_tx_udf_ufilt_cfg_get(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca_macsec_tx_udf_cfilt_cfg_set(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);
g_error_t qca_macsec_tx_udf_cfilt_cfg_get(struct macsec_port *port, struct secy_udf_filt_cfg_t *cfg);


#endif /* _NSS_MACSEC_MACSEC_H_ */
