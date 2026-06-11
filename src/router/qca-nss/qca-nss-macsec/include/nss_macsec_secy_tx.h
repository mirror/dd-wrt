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

#ifndef _NSS_MACSEC_SECY_TX_H_
#define _NSS_MACSEC_SECY_TX_H_


#define NSS_SECY_CONFIDENTIALITY_OFFSET_00 0
#define NSS_SECY_CONFIDENTIALITY_OFFSET_30 30
#define NSS_SECY_CONFIDENTIALITY_OFFSET_50 50

typedef enum {
	EG_CTL_COMPARE_NO,
	EG_CTL_COMPARE_DA,
	EG_CTL_COMPARE_SA,
	EG_CTL_COMPARE_HALF_DA_SA,
	EG_CTL_COMPARE_ETHER_TYPE,
	EG_CTL_COMPARE_DA_ETHER_TYPE,
	EG_CTL_COMPARE_SA_EHTER_TYPE,
	EG_CTL_COMPARE_DA_RANGE,
	EG_CTL_COMPARE_MAX
} fal_eg_ctl_match_type_e;

typedef struct {
	bool bypass;
	fal_eg_ctl_match_type_e match_type;
	u16 match_mask;
	u16 ether_type_da_range;
	u8 sa_da_addr[6];
} fal_tx_ctl_filt_t;

#define FAL_TX_CTL_FILT_NUM     24	/* all entries number in H/W */
#define FAL_TX_CTL_FILT_HW_NUM  24	/* all entries number in H/W */

typedef enum {
	FAL_TX_CLASS_ACTION_FORWARD,
	FAL_TX_CLASS_ACTION_BYPASS,
	FAL_TX_CLASS_ACTION_DROP,
	FAL_TX_CLASS_ACTION_MAX
} fal_tx_class_lut_action_e;

typedef struct {
	bool valid;
	fal_tx_class_lut_action_e action;
	u32 channel;
	u8 da[6];
	u8 da_mask;		/* first 6 bits are useful */
	u8 sa[6];
	u8 sa_mask;		/* first 6 bits are useful */
	u16 ether_type;
	u8 ether_type_mask;	/* first 2 bits are useful */
	bool udf0_valid;
	u8 udf0_byte;
	u8 udf0_location;	/* first 6 bits are useful */
	bool udf1_valid;
	u8 udf1_byte;
	u8 udf1_location;	/* first 6 bits are useful */
	bool udf2_valid;
	u8 udf2_byte;
	u8 udf2_location;	/* first 6 bits are useful */
	bool udf3_valid;
	u8 udf3_byte;
	u8 udf3_location;	/* first 6 bits are useful */
	bool vlan_valid;
	u8 vlan_valid_mask;	/* first bit is useful */
	u8 vlan_up;
	u8 vlan_up_mask;	/* first bit is useful */
	u16 vlan_id;
	u8 vlan_id_mask;	/* first 2 bits are useful */
	/*add for NAPA PHY*/
	u16 outer_vlanid;
	u8 sci[8];
	u8 tci;
	u8 offset;
	bool bc_flag;
	u32 rule_mask;
} fal_tx_class_lut_t;

#define FAL_TX_CLASS_LUT_NUM  48

typedef struct {
	u8 sak[16];
	u8 sak1[16];
	u32 sak_len;
} fal_tx_sak_t;

typedef struct {
	bool parse_en;
	u16 tpid;
} fal_tx_vlan_parse_t;

struct fal_tx_sa_ki_t {
	u8 ki[16];
};

#define FAL_TX_UDF_FILT_MAX_ID     0x4

enum fal_tx_udf_filt_cfg_pattern_e {
	FAL_TX_FILTER_PATTERN_AND = 0,
	FAL_TX_FILTER_PATTERN_OR = 1,
	FAL_TX_FILTER_PATTERN_XOR = 2
};
struct fal_tx_udf_filt_cfg_t {
	bool enable;
	u16 priority;
	u16 inverse;
	enum fal_tx_udf_filt_cfg_pattern_e pattern0;
	enum fal_tx_udf_filt_cfg_pattern_e pattern1;
	enum fal_tx_udf_filt_cfg_pattern_e pattern2;
};
enum fal_tx_udf_filt_type_e {
	FAL_TX_FILTER_ANY_PACKET = 0,
	FAL_TX_FILTER_IP_PACKET = 1,
	FAL_TX_FILTER_TCP_PACKET = 2
};
enum fal_tx_udf_filt_op_e {
	FAL_TX_FILTER_OPERATOR_EQUAL = 0,
	FAL_TX_FILTER_OPERATOR_LESS = 1
};
struct fal_tx_udf_filt_t {
	u16 udf_field0;
	u16 udf_field1;
	u16 udf_field2;
	u16 mask;
	enum fal_tx_udf_filt_type_e type;
	enum fal_tx_udf_filt_op_e operator;
	u16 offset;
};


/**
* @param[in] secy_id
* @param[in] filt_id
* @param[out] pfilt
**/
u32 nss_macsec_secy_tx_ctl_filt_get(u32 secy_id, u32 filt_id,
                                   fal_tx_ctl_filt_t *pfilt);

/**
* @param[in] secy_id
* @param[in] filt_id
* @param[in] pfilt
**/
u32 nss_macsec_secy_tx_ctl_filt_set(u32 secy_id, u32 filt_id,
                                   fal_tx_ctl_filt_t *pfilt);

/**
* @param[in] secy_id
* @param[in] filt_id
**/
u32 nss_macsec_secy_tx_ctl_filt_clear(u32 secy_id, u32 filt_id);

/**
* @param[in] secy_id
**/
u32 nss_macsec_secy_tx_ctl_filt_clear_all(u32 secy_id);

/**
* @param[in] secy_id
* @param[in] index
* @param[out] pentry
**/
u32 nss_macsec_secy_tx_class_lut_get(u32 secy_id, u32 index,
                        fal_tx_class_lut_t *pentry);

/**
* @param[in] secy_id
* @param[in] index
* @param[in] pentry
**/
u32 nss_macsec_secy_tx_class_lut_set(u32 secy_id, u32 index,
                                    fal_tx_class_lut_t *pentry);

/**
* @param[in] secy_id
* @param[in] index
**/
u32 nss_macsec_secy_tx_class_lut_clear(u32 secy_id, u32 index);

/**
* @param[in] secy_id
**/
u32 nss_macsec_secy_tx_class_lut_clear_all(u32 secy_id);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] psci
* @param[in] sci_len
**/
u32 nss_macsec_secy_tx_sc_create(u32 secy_id, u32 channel,
                                u8 *psci, u32 sci_len); /* [16] */

/**
* @param[in] secy_id
* @param[in] channel
* @param[out] penable
**/
u32 nss_macsec_secy_tx_sc_en_get(u32 secy_id, u32 channel, bool *penable);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] enable
**/
u32 nss_macsec_secy_tx_sc_en_set(u32 secy_id, u32 channel, bool enable);

/**
* @param[in] secy_id
* @param[in] channel
**/
u32 nss_macsec_secy_tx_sc_del(u32 secy_id, u32 channel);

/**
* @param[in] secy_id
**/
u32 nss_macsec_secy_tx_sc_del_all(u32 secy_id);

/**
* @param[in] secy_id
* @param[in] channel
* @param[out] pan
**/
u32 nss_macsec_secy_tx_sc_an_get(u32 secy_id, u32 channel, u32 *pan);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
**/
u32 nss_macsec_secy_tx_sc_an_set(u32 secy_id, u32 channel, u32 an);


/**
* @param[in] secy_id
* @param[in] channel
* @param[out] p_in_used
**/
u32 nss_macsec_secy_tx_sc_in_used_get(u32 secy_id, u32 channel, bool *p_in_used);

/**
* @param[in] secy_id
* @param[in] channel
* @param[out] ptci
**/
u32 nss_macsec_secy_tx_sc_tci_7_2_get(u32 secy_id, u32 channel, u8 *ptci);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] tci
**/
u32 nss_macsec_secy_tx_sc_tci_7_2_set(u32 secy_id, u32 channel, u8 tci);

/**
* @param[in] secy_id
* @param[in] channel
* @param[out] poffset
**/
u32 nss_macsec_secy_tx_sc_confidentiality_offset_get(u32 secy_id, u32 channel,
                                                    u32 *poffset);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] offset
**/
u32 nss_macsec_secy_tx_sc_confidentiality_offset_set(u32 secy_id, u32 channel,
                                                    u32 offset);

/**
* @param[in] secy_id
* @param[in] channel
* @param[out] penable
**/
u32 nss_macsec_secy_tx_sc_protect_get(u32 secy_id, u32 channel, bool *penable);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] enable
**/
u32 nss_macsec_secy_tx_sc_protect_set(u32 secy_id, u32 channel, bool enable);

/**
* @param[in] secy_id
* @param[in] channel
* @param[out] psci
* @param[in] sci_len
**/
u32 nss_macsec_secy_tx_sc_sci_get(u32 secy_id, u32 channel,
                                 u8 *psci, u32 sci_len); /* [16] */

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
**/
u32 nss_macsec_secy_tx_sa_create(u32 secy_id, u32 channel, u32 an);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[out] penable
**/
u32 nss_macsec_secy_tx_sa_en_get(u32 secy_id, u32 channel, u32 an, bool *penable);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[in] enable
**/
u32 nss_macsec_secy_tx_sa_en_set(u32 secy_id, u32 channel, u32 an, bool enable);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
**/
u32 nss_macsec_secy_tx_sa_del(u32 secy_id, u32 channel, u32 an);

/**
* @param[in] secy_id
**/
u32 nss_macsec_secy_tx_sa_del_all(u32 secy_id);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[out] p_next_pn
**/
u32 nss_macsec_secy_tx_sa_next_pn_get(u32 secy_id, u32 channel,
                                     u32 an, u32 *p_next_pn);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[in] next_pn
**/
u32 nss_macsec_secy_tx_sa_next_pn_set(u32 secy_id, u32 channel,
                                     u32 an, u32 next_pn);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[out] p_in_used
**/
u32 nss_macsec_secy_tx_sa_in_used_get(u32 secy_id, u32 channel,
                                     u32 an, bool *p_in_used);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[out] pentry
**/
u32 nss_macsec_secy_tx_sak_get(u32 secy_id, u32 channel,
                              u32 an, fal_tx_sak_t *pentry);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[in] pentry
**/
u32 nss_macsec_secy_tx_sak_set(u32 secy_id, u32 channel,
                              u32 an, fal_tx_sak_t *pentry);

u32 nss_macsec_secy_tx_sa_next_xpn_get(u32 secy_id, u32 channel,
u32 an, u32 *p_next_xpn);
u32 nss_macsec_secy_tx_sa_next_xpn_set(u32 secy_id, u32 channel,
u32 an, u32 next_xpn);
u32 nss_macsec_secy_tx_sc_ssci_get(u32 secy_id, u32 channel, u32 *pssci);
u32 nss_macsec_secy_tx_sc_ssci_set(u32 secy_id, u32 channel, u32 ssci);
u32 nss_macsec_secy_tx_sa_ki_get(u32 secy_id, u32 channel, u32 an,
	struct fal_tx_sa_ki_t *ki);
u32 nss_macsec_secy_tx_sa_ki_set(u32 secy_id, u32 channel, u32 an,
	struct fal_tx_sa_ki_t *ki);

u32 nss_macsec_secy_tx_udf_filt_set(u32 secy_id, u32 filt_id,
	struct fal_tx_udf_filt_t *pfilt);
u32 nss_macsec_secy_tx_udf_filt_get(u32 secy_id, u32 filt_id,
	struct fal_tx_udf_filt_t *pfilt);
u32 nss_macsec_secy_tx_udf_ufilt_cfg_set(u32 secy_id,
	struct fal_tx_udf_filt_cfg_t *cfg);
u32 nss_macsec_secy_tx_udf_ufilt_cfg_get(u32 secy_id,
	struct fal_tx_udf_filt_cfg_t *cfg);
u32 nss_macsec_secy_tx_udf_cfilt_cfg_set(u32 secy_id,
	struct fal_tx_udf_filt_cfg_t *cfg);
u32 nss_macsec_secy_tx_udf_cfilt_cfg_get(u32 secy_id,
	struct fal_tx_udf_filt_cfg_t *cfg);

#endif /* _NSS_MACSEC_SECY_TX_H_ */
