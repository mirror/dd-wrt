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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include "nss_macsec_types.h"
#include "nss_macsec.h"
#include "nss_macsec_utility.h"
#include "nss_macsec_secy.h"



u32 nss_macsec_secy_genl_reg_get(u32 secy_id, u32 addr, u32 *pvalue)
{
	int ret = 0;
	struct macsec_port *port = NULL;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (pvalue != NULL));

	port = FAL_SECY_ID_TO_PORT(secy_id);
	SHR_PARAM_CHECK(port != NULL);
	ret = phy_read(port->phydev, addr);
	if (unlikely(ret < 0))
		return ERROR;

	*pvalue = ret;

	return ERROR_OK;
}

u32 nss_macsec_secy_genl_reg_set(u32 secy_id, u32 addr, u32 value)
{
	int ret = 0;
	struct macsec_port *port = NULL;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	port = FAL_SECY_ID_TO_PORT(secy_id);
	SHR_PARAM_CHECK(port != NULL);
	ret = phy_write(port->phydev, addr, (u16)value);
	if (unlikely(ret < 0))
		return ERROR;

	return ERROR_OK;
}

u32 nss_macsec_secy_ext_reg_get(u32 secy_id, u32 addr, u32 *pvalue)
{
	int ret = 0;
	struct macsec_port *port = NULL;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (pvalue != NULL));

	port = FAL_SECY_ID_TO_PORT(secy_id);
	SHR_PARAM_CHECK(port != NULL);
	port->phydev->is_c45 = TRUE;
	ret = phy_read_mmd(port->phydev, MDIO_MMD_PCS, addr);
	if (unlikely(ret < 0))
		return ERROR;

	*pvalue = ret;

	return ERROR_OK;
}

u32 nss_macsec_secy_ext_reg_set(u32 secy_id, u32 addr, u32 value)
{
	int ret = 0;
	struct macsec_port *port = NULL;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	port = FAL_SECY_ID_TO_PORT(secy_id);
	SHR_PARAM_CHECK(port != NULL);
	port->phydev->is_c45 = TRUE;

	ret = phy_write_mmd(port->phydev, MDIO_MMD_PCS, addr, (u16)value);
	if (unlikely(ret))
		return ERROR;

	return ERROR_OK;
}

u32 nss_macsec_secy_sc_sa_mapping_mode_get(u32 secy_id,
					   fal_sc_sa_mapping_mode_e *pmode)
{
	g_error_t rv;
	enum macsec_sc_sa_mapping_mode_t mapping_mode = 0;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (pmode != NULL));

	rv = qca_macsec_sc_sa_mapping_mode_get(FAL_SECY_ID_TO_PORT(secy_id),
						&mapping_mode);
	if (mapping_mode == MACSEC_SC_SA_MAP_1_1)
		*pmode = FAL_SC_SA_MAP_1_1;
	else if (mapping_mode == MACSEC_SC_SA_MAP_1_2)
		*pmode = FAL_SC_SA_MAP_1_2;
	else if (mapping_mode == MACSEC_SC_SA_MAP_1_4)
		*pmode = FAL_SC_SA_MAP_1_4;
	else
	   rv = ERROR_NOT_SUPPORT;

	return rv;
}

u32 nss_macsec_secy_sc_sa_mapping_mode_set(u32 secy_id,
					   fal_sc_sa_mapping_mode_e mode)
{
	g_error_t rv;
	enum macsec_sc_sa_mapping_mode_t secy_sc_sa_mapping_mode;

	SHR_PARAM_CHECK((secy_id < MACSEC_DEVICE_MAXNUM) &&
			(mode < FAL_SC_SA_MAP_MAX));

	if (mode == FAL_SC_SA_MAP_1_1)
		secy_sc_sa_mapping_mode = MACSEC_SC_SA_MAP_1_1;
	else if (mode == FAL_SC_SA_MAP_1_2)
		secy_sc_sa_mapping_mode = MACSEC_SC_SA_MAP_1_2;
	else if (mode == FAL_SC_SA_MAP_1_4)
		secy_sc_sa_mapping_mode = MACSEC_SC_SA_MAP_1_4;
	else
		return ERROR_NOT_SUPPORT;

	rv = qca_macsec_sc_sa_mapping_mode_set(FAL_SECY_ID_TO_PORT(secy_id),
						secy_sc_sa_mapping_mode);
	return rv;
}

u32 nss_macsec_secy_controlled_port_en_get(u32 secy_id, bool *penable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (penable != NULL));

	rv = qca_macsec_controlled_port_en_get(FAL_SECY_ID_TO_PORT(secy_id),
						penable);
	return rv;
}

u32 nss_macsec_secy_controlled_port_en_set(u32 secy_id, bool enable)
{
	g_error_t rv;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	rv = qca_macsec_controlled_port_en_set(FAL_SECY_ID_TO_PORT(secy_id), enable);
	return rv;
}

u32 nss_macsec_secy_cipher_suite_get(u32 secy_id,
				     fal_cipher_suite_e *p_cipher_suite)
{
	g_error_t rv;
	enum secy_cipher_suite_t secy_cipher_suite;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(p_cipher_suite != NULL));

	rv = qca_macsec_cipher_suite_get(FAL_SECY_ID_TO_PORT(secy_id),
					&secy_cipher_suite);
	if (secy_cipher_suite == SECY_CIPHER_SUITE_XPN_128)
		*p_cipher_suite = FAL_CIPHER_SUITE_AES_GCM_XPN_128;
	else if (secy_cipher_suite == SECY_CIPHER_SUITE_256)
		*p_cipher_suite = FAL_CIPHER_SUITE_AES_GCM_256;
	else if (secy_cipher_suite == SECY_CIPHER_SUITE_128)
		*p_cipher_suite = FAL_CIPHER_SUITE_AES_GCM_128;
	else
		*p_cipher_suite = FAL_CIPHER_SUITE_AES_GCM_XPN_256;

	return rv;
}

u32 nss_macsec_secy_cipher_suite_set(u32 secy_id,
				     fal_cipher_suite_e cipher_suite)
{
	g_error_t rv;
	enum secy_cipher_suite_t secy_cipher_suite;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	if (cipher_suite == FAL_CIPHER_SUITE_AES_GCM_XPN_128)
		secy_cipher_suite = SECY_CIPHER_SUITE_XPN_128;
	else if (cipher_suite == FAL_CIPHER_SUITE_AES_GCM_256)
		secy_cipher_suite = SECY_CIPHER_SUITE_256;
	else if (cipher_suite == FAL_CIPHER_SUITE_AES_GCM_128)
		secy_cipher_suite = SECY_CIPHER_SUITE_128;
	else
		secy_cipher_suite = SECY_CIPHER_SUITE_XPN_256;

	rv = qca_macsec_cipher_suite_set(FAL_SECY_ID_TO_PORT(secy_id),
					 secy_cipher_suite);
	return rv;
}

u32 nss_macsec_secy_mtu_get(u32 secy_id, u32 *pmtu)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (pmtu != NULL));

	rv = qca_macsec_mtu_get(FAL_SECY_ID_TO_PORT(secy_id), pmtu);

	return rv;
}

u32 nss_macsec_secy_mtu_set(u32 secy_id, u32 mtu)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(mtu <= FAL_TX_RX_MTU_MAX));

	rv = qca_macsec_mtu_set(FAL_SECY_ID_TO_PORT(secy_id), mtu);
	return rv;
}

u32 nss_macsec_secy_id_get(u8 *dev_name, u32 *secy_id)
{
	return nss_macsec_dt_secy_id_get(dev_name, secy_id);
}

u32 nss_macsec_secy_en_get(u32 secy_id, bool *penable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (penable != NULL));

	rv = qca_macsec_en_get(FAL_SECY_ID_TO_PORT(secy_id), penable);
	return rv;
}

u32 nss_macsec_secy_en_set(u32 secy_id, bool enable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM));

	rv = qca_macsec_en_set(FAL_SECY_ID_TO_PORT(secy_id), enable);
	return rv;
}

u32 nss_macsec_secy_xpn_en_get(u32 secy_id, bool *penable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (penable != NULL));

	rv = qca_macsec_xpn_en_get(FAL_SECY_ID_TO_PORT(secy_id), penable);
	return rv;
}

u32 nss_macsec_secy_xpn_en_set(u32 secy_id, bool enable)
{
	g_error_t rv;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	rv = qca_macsec_xpn_en_set(FAL_SECY_ID_TO_PORT(secy_id), enable);
	return rv;
}

u32 nss_macsec_secy_flow_control_en_get(u32 secy_id, bool *penable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (penable != NULL));

	rv = qca_macsec_flow_control_en_get(FAL_SECY_ID_TO_PORT(secy_id), penable);
	return rv;
}

u32 nss_macsec_secy_flow_control_en_set(u32 secy_id, bool enable)
{
	g_error_t rv;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	rv = qca_macsec_flow_control_en_set(FAL_SECY_ID_TO_PORT(secy_id), enable);
	return rv;
}

u32 nss_macsec_secy_special_pkt_ctrl_get(u32 secy_id,
					enum fal_packet_type_t packet_type,
					enum fal_packet_action_t *packet_action)
{
	g_error_t rv;
	enum secy_packet_type_t secy_packet_type;
	enum secy_packet_action_t secy_packet_action;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(packet_action != NULL));

	if (packet_type == FAL_PACKET_STP)
		secy_packet_type = PACKET_STP;
	else if (packet_type == FAL_PACKET_CDP)
		secy_packet_type = PACKET_CDP;
	else if (packet_type == FAL_PACKET_LLDP)
		secy_packet_type = PACKET_LLDP;
	else
		return ERROR_NOT_SUPPORT;

	rv = qca_macsec_special_pkt_ctrl_get(FAL_SECY_ID_TO_PORT(secy_id),
					secy_packet_type, &secy_packet_action);
	if (secy_packet_action == PACKET_CRYPTO_OR_DISCARD)
		*packet_action = FAL_PACKET_CRYPTO_OR_DISCARD;
	else if (secy_packet_action == PACKET_CRYPTO_OR_UPLOAD)
		*packet_action = FAL_PACKET_CRYPTO_OR_UPLOAD;
	else if (secy_packet_action == PACKET_PLAIN_OR_DISCARD)
		*packet_action = FAL_PACKET_PLAIN_OR_DISCARD;
	else if (secy_packet_action == PACKET_PLAIN_OR_UPLOAD)
		*packet_action = FAL_PACKET_PLAIN_OR_UPLOAD;
	else
		rv = ERROR_NOT_SUPPORT;

	return rv;
}

u32 nss_macsec_secy_special_pkt_ctrl_set(u32 secy_id,
					enum fal_packet_type_t packet_type,
					enum fal_packet_action_t packet_action)
{
	g_error_t rv;
	enum secy_packet_type_t secy_packet_type;
	enum secy_packet_action_t secy_packet_action;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	if (packet_type == FAL_PACKET_STP)
		secy_packet_type = PACKET_STP;
	else if (packet_type == FAL_PACKET_CDP)
		secy_packet_type = PACKET_CDP;
	else if (packet_type == FAL_PACKET_LLDP)
		secy_packet_type = PACKET_LLDP;
	else
		return ERROR_NOT_SUPPORT;

	if (packet_action == FAL_PACKET_CRYPTO_OR_DISCARD)
		secy_packet_action = PACKET_CRYPTO_OR_DISCARD;
	else if (packet_action == FAL_PACKET_CRYPTO_OR_UPLOAD)
		secy_packet_action = PACKET_CRYPTO_OR_UPLOAD;
	else if (packet_action == FAL_PACKET_PLAIN_OR_DISCARD)
		secy_packet_action = PACKET_PLAIN_OR_DISCARD;
	else if (packet_action == FAL_PACKET_PLAIN_OR_UPLOAD)
		secy_packet_action = PACKET_PLAIN_OR_UPLOAD;
	else
		return ERROR_NOT_SUPPORT;

	rv = qca_macsec_special_pkt_ctrl_set(FAL_SECY_ID_TO_PORT(secy_id),
					     secy_packet_type, secy_packet_action);
	return rv;
}

u32 nss_macsec_secy_udf_ethtype_get(u32 secy_id, bool *penable, u16 *type)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(penable != NULL) &&
			(type != NULL));

	rv = qca_macsec_udf_ethtype_get(FAL_SECY_ID_TO_PORT(secy_id),
					penable, type);
	return rv;
}

u32 nss_macsec_secy_udf_ethtype_set(u32 secy_id, bool enable, u16 type)
{
	g_error_t rv;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	rv = qca_macsec_udf_ethtype_set(FAL_SECY_ID_TO_PORT(secy_id),
					enable, type);
	return rv;
}


