/*
 * Copyright (c) 2014, 2018, The Linux Foundation. All rights reserved.
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

#ifndef _NSS_MACSEC_SECY_H_
#define _NSS_MACSEC_SECY_H_

#ifndef __KERNEL__
#include <fcntl.h>
#endif

/* Macro indicate nss_macsec_secy_id_get() function is supported,
 * used in hostap macsec package for backwards compatible with
 * old nss macsec library which do not support this function.
 */
#define NSS_MACSEC_SECY_ID_GET_FUNC

#define FAL_SECY_DEFAULT_MTU    1500
#define FAL_SCI_LEN             8
#define FAL_SECY_SC_MAX_NUM     32
#define FAL_SAK_DEFAULT_LEN     16
#define FAL_TX_RX_MTU_MAX 0x7fff
#define FAL_AN_MAX  3

typedef enum {
	FAL_SC_SA_MAP_1_1 = 0,
	FAL_SC_SA_MAP_1_2 = 1,
	FAL_SC_SA_MAP_1_4 = 2,
	FAL_SC_SA_MAP_MAX = 3
} fal_sc_sa_mapping_mode_e;

typedef enum {
	FAL_CIPHER_SUITE_AES_GCM_128 = 0,
	FAL_CIPHER_SUITE_AES_GCM_256,
	FAL_CIPHER_SUITE_AES_GCM_XPN_128,
	FAL_CIPHER_SUITE_AES_GCM_XPN_256,
	FAL_CIPHER_SUITE_MAX
} fal_cipher_suite_e;

typedef struct {
	u8 sci[FAL_SCI_LEN];
} fal_tx_sc_t;

enum fal_packet_type_t {
	FAL_PACKET_STP   = 0,   /**< Spanning Tree Protocol Packet */
	FAL_PACKET_CDP   = 1,   /**< Cisco Discovery Protocol Packet */
	FAL_PACKET_LLDP  = 2   /**< Link Layer Discovery Protocol Packet */
};

enum fal_packet_action_t {
	FAL_PACKET_CRYPTO_OR_DISCARD = 0X0,
	FAL_PACKET_CRYPTO_OR_UPLOAD  = 0X1,
	FAL_PACKET_PLAIN_OR_DISCARD  = 0X2,
	FAL_PACKET_PLAIN_OR_UPLOAD   = 0X3
};


/**
* @param[in] secy_id
* @param[in] addr
* @param[out] pvalue
**/
u32 nss_macsec_secy_genl_reg_get(u32 secy_id, u32 addr, u32 *pvalue);

/**
* @param[in] secy_id
* @param[in] addr
* @param[in] value
**/
u32 nss_macsec_secy_genl_reg_set(u32 secy_id, u32 addr, u32 value);

/**
* @param[in] secy_id
* @param[in] addr
* @param[out] pvalue
**/
u32 nss_macsec_secy_ext_reg_get(u32 secy_id, u32 addr, u32 *pvalue);

/**
* @param[in] secy_id
* @param[in] addr
* @param[in] value
**/
u32 nss_macsec_secy_ext_reg_set(u32 secy_id, u32 addr, u32 value);

/**
* @param[in] secy_id
* @param[out] pmode
**/
u32 nss_macsec_secy_sc_sa_mapping_mode_get(u32 secy_id,
					   fal_sc_sa_mapping_mode_e *pmode);
/**
* @param[in] secy_id
* @param[in] mode
**/
u32 nss_macsec_secy_sc_sa_mapping_mode_set(u32 secy_id,
					   fal_sc_sa_mapping_mode_e mode);

/**
* @param[in] secy_id
* @param[out] penable
**/
u32 nss_macsec_secy_controlled_port_en_get(u32 secy_id, bool *penable);

/**
* @param[in] secy_id
* @param[in] enable
**/
u32 nss_macsec_secy_controlled_port_en_set(u32 secy_id, bool enable);

/**
* @param[in] secy_id
* @param[out] p_cipher_suite
**/
u32 nss_macsec_secy_cipher_suite_get(u32 secy_id,
                                    fal_cipher_suite_e *p_cipher_suite);

/**
* @param[in] secy_id
* @param[in] cipher_suite
**/
u32 nss_macsec_secy_cipher_suite_set(u32 secy_id,
                                    fal_cipher_suite_e cipher_suite);

/**
* @param[in] secy_id
* @param[out] pmtu
**/
u32 nss_macsec_secy_mtu_get(u32 secy_id, u32 *pmtu);

/**
* @param[in] secy_id
* @param[in] mtu
**/
u32 nss_macsec_secy_mtu_set(u32 secy_id, u32 mtu);

/**
* @param[in] dev_name
* @param[out] secy_id
**/
u32 nss_macsec_secy_id_get(u8 *dev_name, u32 *secy_id);

/**
* @param[in] secy_id
* @param[out] penable
**/
u32 nss_macsec_secy_en_get(u32 secy_id, bool *penable);

/**
* @param[in] secy_id
* @param[in] enable
**/
u32 nss_macsec_secy_en_set(u32 secy_id, bool enable);

u32 nss_macsec_secy_xpn_en_get(u32 secy_id, bool *penable);

u32 nss_macsec_secy_xpn_en_set(u32 secy_id, bool enable);

u32 nss_macsec_secy_flow_control_en_get(u32 secy_id, bool *penable);

u32 nss_macsec_secy_flow_control_en_set(u32 secy_id, bool enable);

u32 nss_macsec_secy_special_pkt_ctrl_get(u32 secy_id,
	enum fal_packet_type_t packet_type,
	enum fal_packet_action_t *packet_action);

u32 nss_macsec_secy_special_pkt_ctrl_set(u32 secy_id,
	enum fal_packet_type_t packet_type,
	enum fal_packet_action_t packet_action);

u32 nss_macsec_secy_udf_ethtype_get(u32 secy_id, bool *penable, u16 *type);

u32 nss_macsec_secy_udf_ethtype_set(u32 secy_id, bool enable, u16 type);

u32 nss_macsec_dt_secy_id_get(u8 *dev_name, u32 *secy_id);

#endif /* _NSS_MACSEC_SECY_H_ */
