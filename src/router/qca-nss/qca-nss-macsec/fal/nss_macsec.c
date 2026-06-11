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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include "nss_macsec_types.h"
#include "nss_macsec.h"
#include "qca808x_macsec.h"



struct qca_macsec_device_t *p_macsec_device_cfg[MACSEC_DEVICE_MAXNUM];



static u32 macsec_device_register(struct phy_device *phydev)
{
	u32 dev_id = 0;
	u32 assign_id = INVALID_DEVICE_ID;

	for (dev_id = 0; dev_id < MACSEC_DEVICE_MAXNUM; dev_id++) {
		if (p_macsec_device_cfg[dev_id] != NULL) {
			if (phydev == p_macsec_device_cfg[dev_id]->port.phydev) {
				osal_print("macsec device:%d already exists!\n", dev_id);
				assign_id = INVALID_DEVICE_ID;
				break;
			}
		} else {
			if (INVALID_DEVICE_ID == assign_id)
				assign_id = dev_id;
		}
	}
	return assign_id;
}

static g_error_t macsec_ctl_filt_mac_convert(struct secy_ctl_filt_t *secy_filt,
						struct secy_udf_filt_t *filter)
{
	memset(filter, 0, sizeof(*filter));

	filter->udf_field0 = secy_filt->sa_da_addr[0] << 0x8 |
		secy_filt->sa_da_addr[1];
	filter->udf_field1 = secy_filt->sa_da_addr[2] << 0x8 |
		secy_filt->sa_da_addr[3];
	filter->udf_field2 = secy_filt->sa_da_addr[4] << 0x8 |
		secy_filt->sa_da_addr[5];

	return ERROR_OK;
}

static g_error_t macsec_rx_ctl_filt_da_mac_set(struct macsec_port *port, u32 index,
						struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x0;
	filter.mask = secy_filt->match_mask;

	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_rx_ctl_filt_sa_mac_set(struct macsec_port *port, u32 index,
						struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x6;
	filter.mask = secy_filt->match_mask;

	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_rx_ctl_filt_ether_type_set(struct macsec_port *port, u32 index,
						   struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter, 0, sizeof(filter));
	memset(&filter1, 0, sizeof(filter1));

	filter.udf_field0 = secy_filt->ether_type_da_range;
	filter.offset = 0xc;
	filter.mask = secy_filt->match_mask & 0x30;

	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_rx_ctl_filt_da_ether_type_set(struct macsec_port *port,
						      u32 index, struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x0;
	filter.mask = secy_filt->match_mask;

	filter1.udf_field0 = secy_filt->ether_type_da_range;
	filter1.offset = 0xc;
	filter1.mask = (secy_filt->match_mask >> 0x8) & 0x30;

	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_rx_ctl_filt_sa_ether_type_set(struct macsec_port *port,
						      u32 index, struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x6;
	filter.mask = secy_filt->match_mask;

	filter1.udf_field0 = secy_filt->ether_type_da_range;
	filter1.offset = 0xc;
	filter1.mask = (secy_filt->match_mask >> 0x8) & 0x30;

	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_rx_ctl_filt_half_da_sa_set(struct macsec_port *port, u32 index,
						   struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter, 0, sizeof(filter));
	memset(&filter1, 0, sizeof(filter1));


	filter.udf_field1 = secy_filt->sa_da_addr[0];
	filter.udf_field2 = secy_filt->sa_da_addr[1] << 0x8 |
		secy_filt->sa_da_addr[2];
	filter.offset = 0x0;
	filter.mask = (secy_filt->match_mask) & 0x7;

	filter1.udf_field1 = secy_filt->sa_da_addr[3];
	filter1.udf_field2 = secy_filt->sa_da_addr[4] << 0x8 |
		secy_filt->sa_da_addr[5];
	filter1.offset = 0x6;
	filter1.mask = (secy_filt->match_mask >> 0x8) & 0x7;

	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_tx_ctl_filt_da_mac_set(struct macsec_port *port, u32 index,
					       struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x0;
	filter.mask = secy_filt->match_mask;

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_tx_ctl_filt_sa_mac_set(struct macsec_port *port, u32 index,
					       struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x6;
	filter.mask = secy_filt->match_mask;

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_tx_ctl_filt_ether_type_set(struct macsec_port *port, u32 index,
						   struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter, 0, sizeof(filter));
	memset(&filter1, 0, sizeof(filter1));

	filter.udf_field0 = secy_filt->ether_type_da_range;
	filter.offset = 0xc;
	filter.mask = secy_filt->match_mask & 0x30;

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_tx_ctl_filt_da_ether_type_set(struct macsec_port *port, u32 index,
						      struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x0;
	filter.mask = secy_filt->match_mask;

	filter1.udf_field0 = secy_filt->ether_type_da_range;
	filter1.offset = 0xc;
	filter1.mask = (secy_filt->match_mask >> 0x8) & 0x30;

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_tx_ctl_filt_sa_ether_type_set(struct macsec_port *port, u32 index,
						      struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter1, 0, sizeof(filter1));

	SHR_RET_ON_ERR(macsec_ctl_filt_mac_convert(secy_filt, &filter));

	filter.offset = 0x6;
	filter.mask = secy_filt->match_mask;

	filter1.udf_field0 = secy_filt->ether_type_da_range;
	filter1.offset = 0xc;
	filter1.mask = (secy_filt->match_mask >> 0x8) & 0x30;

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

static g_error_t macsec_tx_ctl_filt_half_da_sa_set(struct macsec_port *port, u32 index,
						   struct secy_ctl_filt_t *secy_filt)
{
	struct secy_udf_filt_t filter, filter1;

	memset(&filter, 0, sizeof(filter));
	memset(&filter1, 0, sizeof(filter1));


	filter.udf_field1 = secy_filt->sa_da_addr[0];
	filter.udf_field2 = secy_filt->sa_da_addr[1] << 0x8 |
		secy_filt->sa_da_addr[2];
	filter.offset = 0x0;
	filter.mask = (secy_filt->match_mask) & 0x7;

	filter1.udf_field1 = secy_filt->sa_da_addr[3];
	filter1.udf_field2 = secy_filt->sa_da_addr[4] << 0x8 |
		secy_filt->sa_da_addr[5];
	filter1.offset = 0x6;
	filter1.mask = (secy_filt->match_mask >> 0x8) & 0x7;

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 0, &filter));
	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, 1, &filter1));

	return ERROR_OK;
}

u32 macsec_get_device_id(struct phy_device *phydev)
{
	u32 dev_id = 0;
	u32 find_id = INVALID_DEVICE_ID;

	for (dev_id = 0; dev_id < MACSEC_DEVICE_MAXNUM; dev_id++) {
		if (p_macsec_device_cfg[dev_id] != NULL) {
			if (phydev == p_macsec_device_cfg[dev_id]->port.phydev) {
				find_id = dev_id;
				break;
			}
		}
	}
	return find_id;
}

struct macsec_port *macsec_port_get_by_device_id(u32 dev_id)
{
	struct macsec_port *port = NULL;

	if (dev_id >= MACSEC_DEVICE_MAXNUM) {
		osal_print("%s: Wrong dev_id %d! \n", __func__, dev_id);
		return port;
	}

	if (p_macsec_device_cfg[dev_id] != NULL)
		port = &(p_macsec_device_cfg[dev_id]->port);
	if (unlikely(NULL == port))
		osal_print("%s: fail to find port for dev_id %d! \n", __func__, dev_id);
	return port;
}

g_error_t qca_macsec_channel_number_get(struct macsec_port *port, u32 *number)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(number != NULL));

	*number = SECY_SC_IDX_MAX + 1;

	return ERROR_OK;
}

g_error_t qca_macsec_cipher_suite_set(struct macsec_port *port,
				      enum secy_cipher_suite_t cipher_suite)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_cipher_suite_set(port, cipher_suite));
	return ERROR_OK;
}

g_error_t qca_macsec_cipher_suite_get(struct macsec_port *port,
				      enum secy_cipher_suite_t *cipher_suite)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(cipher_suite != NULL));

	SHR_RET_ON_ERR(qca808x_secy_cipher_suite_get(port, cipher_suite));
	return ERROR_OK;
}

g_error_t qca_macsec_sc_sa_mapping_mode_set(struct macsec_port *port,
					    enum macsec_sc_sa_mapping_mode_t mode)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK(port != NULL);

	dev_id = macsec_get_device_id(port->phydev);
	if (INVALID_DEVICE_ID == dev_id) {
		osal_print("%s: fail to find dev_id! \n", __func__);
		return ERROR_NOT_FOUND;
	}
	p_macsec_device_cfg[dev_id]->sc_sa_mapping_mode = mode;
	return ERROR_OK;
}

g_error_t qca_macsec_sc_sa_mapping_mode_get(struct macsec_port *port,
					    enum macsec_sc_sa_mapping_mode_t *pmode)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) &&
			(pmode != NULL));

	dev_id = macsec_get_device_id(port->phydev);
	if (INVALID_DEVICE_ID == dev_id) {
		osal_print("%s: fail to find dev_id! \n", __func__);
		return ERROR_NOT_FOUND;
	}

	*pmode = p_macsec_device_cfg[dev_id]->sc_sa_mapping_mode;

	return ERROR_OK;
}

g_error_t qca_macsec_xpn_en_set(struct macsec_port *port, bool enable)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_xpn_en_set(port, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_xpn_en_get(struct macsec_port *port, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_xpn_en_get(port, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_mtu_set(struct macsec_port *port, u32 len)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_mtu_set(port, len));
	return ERROR_OK;
}

g_error_t qca_macsec_mtu_get(struct macsec_port *port, u32 *len)
{
	SHR_PARAM_CHECK((port != NULL) && (len != NULL));

	SHR_RET_ON_ERR(qca808x_secy_mtu_get(port, len));
	return ERROR_OK;
}

g_error_t qca_macsec_special_pkt_ctrl_set(struct macsec_port *port,
					    enum secy_packet_type_t packet_type,
					    enum secy_packet_action_t action)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_special_pkt_ctrl_set(port, packet_type, action));
	return ERROR_OK;
}

g_error_t qca_macsec_special_pkt_ctrl_get(struct macsec_port *port,
					    enum secy_packet_type_t packet_type,
					    enum secy_packet_action_t *action)
{
	SHR_PARAM_CHECK((port != NULL) && (action != NULL));

	SHR_RET_ON_ERR(qca808x_secy_special_pkt_ctrl_get(port, packet_type, action));
	return ERROR_OK;
}

g_error_t qca_macsec_udf_ethtype_set(struct macsec_port *port, bool enable, u16 type)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_udf_ethtype_set(port, enable, type));
	return ERROR_OK;
}

g_error_t qca_macsec_udf_ethtype_get(struct macsec_port *port, bool *enable, u16 *type)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(enable != NULL) &&
			(type != NULL));

	SHR_RET_ON_ERR(qca808x_secy_udf_ethtype_get(port, enable, type));
	return ERROR_OK;
}

g_error_t qca_macsec_flow_control_en_set(struct macsec_port *port, bool enable)
{
	SHR_PARAM_CHECK(port != NULL);

	return qca808x_secy_flow_control_en_set(port, enable);
}

g_error_t qca_macsec_flow_control_en_get(struct macsec_port *port, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(enable != NULL));

	return qca808x_secy_flow_control_en_get(port, enable);
}

g_error_t qca_macsec_en_set(struct macsec_port *port, bool enable)
{
	enum secy_loopback_type_t type;

	SHR_PARAM_CHECK(port != NULL);

	if (enable == TRUE)
		type = MACSEC_NORMAL;
	else
		type = MACSEC_BYPASS;

	SHR_RET_ON_ERR(qca808x_secy_loopback_set(port, type));

	SHR_RET_ON_ERR(qca808x_secy_shadow_register_en_set(port, enable));

	return ERROR_OK;
}

g_error_t qca_macsec_en_get(struct macsec_port *port, bool *enable)
{
	enum secy_loopback_type_t type;

	SHR_PARAM_CHECK((port != NULL) && (enable != NULL));

	qca808x_secy_loopback_get(port, &type);

	if (type == MACSEC_NORMAL)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca_macsec_controlled_port_en_set(struct macsec_port *port, bool enable)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_controlled_port_en_set(port, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_controlled_port_en_get(struct macsec_port *port, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) && (enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_controlled_port_en_get(port, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_npn_set(struct macsec_port *port, u32 sc_index,
				   u32 an, u64 next_pn)
{
	u32 dev_id = 0;
	u32 pn_index = 0;
	u32 npn = 0;
	u32 xpn = 0;
	bool xpn_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_xpn_en_get(port, &xpn_enable));
	if (xpn_enable) {
		xpn = (u32)((next_pn >> 16) >> 16);
		SHR_RET_ON_ERR(qca808x_secy_rx_sa_xpn_set(port, sc_index, an, xpn));
	}

	npn = (u32)(next_pn & 0xffffffff);
	SHR_RET_ON_ERR(qca808x_secy_rx_sa_npn_set(port, sc_index, an, npn));

	pn_index = (sc_index * 4 + an);
	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].rx_sa_pn = next_pn;
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_npn_get(struct macsec_port *port, u32 sc_index,
				   u32 an, u64 *next_pn)
{
	u32 dev_id = 0, pn_index = 0, i = 0;
	u32 npn = 0, xpn = 0;
	u64 pn = 0;
	bool xpn_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(next_pn != NULL));

	pn_index = (sc_index * 4 + an);
	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;

	for (i = 0; i < SECY_PN_QUERY_TIMES; i++) {
		SHR_RET_ON_ERR(qca808x_secy_xpn_en_get(port, &xpn_enable));
		if (xpn_enable) {
			SHR_RET_ON_ERR(qca808x_secy_rx_sa_xpn_get(port, sc_index, an, &xpn));
			pn = xpn;
		}
		SHR_RET_ON_ERR(qca808x_secy_rx_sa_npn_get(port, sc_index, an, &npn));
		pn = (pn << 32) | npn;
		*next_pn = pn;
		if (pn >= p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].rx_sa_pn) {
			p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].rx_sa_pn = pn;
			break;
		}
	}
	if (i >= SECY_PN_QUERY_TIMES)
		*next_pn = p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].rx_sa_pn;

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_create(struct macsec_port *port, u32 sc_index, u32 an)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca_macsec_rx_sa_npn_set(port, sc_index, an, 1));
	SHR_RET_ON_ERR(qca808x_secy_rx_sa_create(port, sc_index, an));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_del(struct macsec_port *port, u32 sc_index, u32 an)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_del(port, sc_index, an));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_del_all(struct macsec_port *port)
{
	u32 sc_index = 0, an_index = 0;

	SHR_PARAM_CHECK(port != NULL);

	for (sc_index = 0; sc_index <= SECY_SC_IDX_MAX; sc_index++) {
		for (an_index = 0; an_index <= SECY_AN_IDX_MAX; an_index++)
			qca808x_secy_rx_sa_del(port, sc_index, an_index);
	}
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_in_used_get(struct macsec_port *port, u32 sc_index,
					u32 an, bool *p_in_used)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(p_in_used != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_en_get(port, sc_index,
			an, p_in_used));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_en_set(struct macsec_port *port, u32 sc_index,
				    u32 an, bool enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_en_set(port, sc_index,
			an, enable));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_en_get(struct macsec_port *port, u32 sc_index,
				    u32 an, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_en_get(port, sc_index,
			an, enable));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sak_set(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key != NULL));

	if ((key->len != SAK_LEN_128BITS) &&
		(key->len != SAK_LEN_256BITS) &&
		(key->len != SAK_LEN_LEGACY))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(qca808x_secy_rx_sak_set(port, sc_index, an, key));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sak_get(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	bool enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &enable));
	if (enable) {
		SHR_RET_ON_ERR(qca808x_secy_rx_sak_get(port, sc_index, an, key));
	} else {
		return ERROR_NOT_SUPPORT;
	}
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_ki_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key_idendifier != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_ki_set(port, sc_index, an, key_idendifier));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_ki_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key_idendifier != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_ki_get(port, sc_index, an, key_idendifier));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_rx_sc_ssci_set(port, sc_index, ssci));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;
	p_macsec_device_cfg[dev_id]->rx_ssci[sc_index] = ssci;
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci)
{
	u32 dev_id = 0;
	bool enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(ssci != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &enable));
	if (enable) {
		SHR_RET_ON_ERR(qca808x_secy_rx_sc_ssci_get(port, sc_index, ssci));
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		*ssci = p_macsec_device_cfg[dev_id]->rx_ssci[sc_index];
	}
	return ERROR_OK;
}

g_error_t qca_macsec_rx_prc_lut_set(struct macsec_port *port, u32 sc_index,
				    struct secy_rx_prc_lut_t *pentry)
{
	struct secy_rx_sc_policy_rule_t rule;
	u32 i =  0;
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(pentry != NULL));

	memset(&rule, 0, sizeof(rule));

	rule.rule_valid = pentry->valid;
	rule.action.rx_sc_index = sc_index;

	for (i = 0; i < 6; i++) {
		rule.mac_da.addr[i] = pentry->da[i];
		rule.mac_sa.addr[i] = pentry->sa[i];
		rule.rx_sci.addr[i] = pentry->sci[i];
	}
	rule.ethtype = pentry->ether_type;
	rule.inner_vlanid = pentry->inner_vlanid;
	rule.outer_vlanid = pentry->outer_vlanid;
	rule.bc_flag = pentry->bc_flag;
	rule.action.decryption_offset = pentry->offset;
	rule.action.rx_sc_index = pentry->channel;
	rule.rx_tci = pentry->tci;
	rule.rx_sci.port = (((u16)(pentry->sci[6])) << 8) |
				(u16)(pentry->sci[7]);
	rule.rule_mask = pentry->rule_mask;

	SHR_RET_ON_ERR(qca808x_secy_rx_sc_policy_set(port, sc_index, &rule));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		memcpy(&(p_macsec_device_cfg[dev_id]->rx_lut[sc_index]), pentry,
			sizeof(struct secy_rx_prc_lut_t));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_prc_lut_get(struct macsec_port *port, u32 sc_index,
				    struct secy_rx_prc_lut_t *pentry)
{
	struct secy_rx_sc_policy_rule_t rule;
	u32 i = 0;
	u32 dev_id = 0;
	bool enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(pentry != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &enable));

	if (enable) {
		memset(&rule, 0, sizeof(rule));
		SHR_RET_ON_ERR(qca808x_secy_rx_sc_policy_get(port, sc_index, &rule));

		pentry->valid = rule.rule_valid;
		for (i = 0; i < 6; i++) {
			pentry->da[i] = rule.mac_da.addr[i];
			pentry->sa[i] = rule.mac_sa.addr[i];
			pentry->sci[i] = rule.rx_sci.addr[i];
		}
		pentry->ether_type = rule.ethtype;
		pentry->inner_vlanid = rule.inner_vlanid;
		pentry->outer_vlanid = rule.outer_vlanid;
		pentry->bc_flag = rule.bc_flag;
		pentry->offset = rule.action.decryption_offset;
		pentry->channel = rule.action.rx_sc_index;
		pentry->tci = rule.rx_tci;
		pentry->sci[6] = (u8)(rule.rx_sci.port >> 8);
		pentry->sci[7] = (u8)(rule.rx_sci.port & 0xff);
		pentry->rule_mask = rule.rule_mask;
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		 memcpy(pentry, &(p_macsec_device_cfg[dev_id]->rx_lut[sc_index]),
			sizeof(struct secy_rx_prc_lut_t));
	}
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_in_used_get(struct macsec_port *port, u32 sc_index,
					bool *enable)
{
	u32 dev_id = 0;
	bool dbg_enable = FALSE;
	bool sc_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &dbg_enable));

	if (dbg_enable) {
		SHR_RET_ON_ERR(qca808x_secy_rx_sc_en_get(port, sc_index, &sc_enable));
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		sc_enable = p_macsec_device_cfg[dev_id]->rx_lut[sc_index].valid;
	}

	*enable = sc_enable;
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_en_set(struct macsec_port *port, u32 sc_index,
				  bool enable)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;

	SHR_RET_ON_ERR(qca808x_secy_rx_sc_en_zone_set(port, sc_index, enable,
		p_macsec_device_cfg[dev_id]->rx_lut[sc_index].rule_mask));

	p_macsec_device_cfg[dev_id]->rx_lut[sc_index].valid = enable;
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(enable != NULL));

	SHR_RET_ON_ERR(qca_macsec_rx_sc_in_used_get(port, sc_index, enable));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_create(struct macsec_port *port, u32 sc_index)
{
	struct secy_rx_prc_lut_t rule;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	memset(&rule, 0, sizeof(rule));
	SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_get(port, sc_index, &rule));
	rule.valid = TRUE;
	rule.channel = sc_index;
	SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_set(port, sc_index, &rule));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_del(struct macsec_port *port, u32 sc_index)
{
	struct secy_rx_prc_lut_t rule;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	memset(&rule, 0, sizeof(rule));

	SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_get(port, sc_index, &rule));

	rule.valid = FALSE;
	rule.channel = sc_index;

	SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_set(port, sc_index, &rule));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_del_all(struct macsec_port *port)
{
	u32 sc_index = 0;
	struct secy_rx_prc_lut_t rule;

	SHR_PARAM_CHECK(port != NULL);

	for (sc_index = 0; sc_index < SECY_LUT_MAX; sc_index++) {
		memset(&rule, 0, sizeof(rule));
		SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_get(port,
				sc_index, &rule));
		rule.valid = FALSE;
		rule.channel = sc_index;
		SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_set(port,
				sc_index, &rule));
	}

	return ERROR_OK;
}

g_error_t qca_macsec_rx_prc_lut_clear(struct macsec_port *port, u32 sc_index)
{
	struct secy_rx_prc_lut_t rule;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	memset(&rule, 0, sizeof(rule));

	SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_set(port, sc_index, &rule));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_prc_lut_clear_all(struct macsec_port *port)
{
	u32 sc_index = 0;
	struct secy_rx_prc_lut_t rule;

	SHR_PARAM_CHECK(port != NULL);

	memset(&rule, 0, sizeof(rule));

	for (sc_index = 0; sc_index < SECY_LUT_MAX; sc_index++) {
		SHR_RET_ON_ERR(qca_macsec_rx_prc_lut_set(port, sc_index, &rule));
	}

	return ERROR_OK;
}

g_error_t qca_macsec_rx_validate_frame_set(struct macsec_port *port, enum secy_vf_t value)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_rx_validate_frame_set(port, value));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_validate_frame_get(struct macsec_port *port, enum secy_vf_t *value)
{
	SHR_PARAM_CHECK((port != NULL) && (value != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_validate_frame_get(port, value));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_replay_protect_en_set(struct macsec_port *port, bool enable)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_rx_replay_protect_en_set(port, enable));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_replay_protect_en_get(struct macsec_port *port, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) && (enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_replay_protect_en_get(port, enable));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_replay_window_set(struct macsec_port *port, u32 window)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_rx_replay_window_set(port, window));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_replay_window_get(struct macsec_port *port, u32 *window)
{
	SHR_PARAM_CHECK((port != NULL) && (window != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_replay_window_get(port, window));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_mib_get(struct macsec_port *port, u32 sc_index,
				     u32 an, struct secy_rx_sa_mib_t *mib)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(mib != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_mib_get(port, sc_index, an, mib));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_mib_get(struct macsec_port *port, u32 sc_index,
				     struct secy_rx_sc_mib_t *mib)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(mib != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_sc_mib_get(port, sc_index, mib));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_mib_get(struct macsec_port *port, struct secy_rx_mib_t *mib)
{
	SHR_PARAM_CHECK((port != NULL) && (mib != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_mib_get(port, mib));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_sa_mib_clear(struct macsec_port *port, u32 sc_index, u32 an)
{
	struct secy_rx_sa_mib_t mib;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	memset(&mib, 0, sizeof(mib));

	SHR_RET_ON_ERR(qca808x_secy_rx_sa_mib_get(port, sc_index, an, &mib));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_sc_mib_clear(struct macsec_port *port, u32 sc_index)
{
	struct secy_rx_sc_mib_t mib;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	memset(&mib, 0, sizeof(mib));

	SHR_RET_ON_ERR(qca808x_secy_rx_sc_mib_get(port, sc_index, &mib));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_mib_clear(struct macsec_port *port)
{
	struct secy_rx_mib_t mib;

	SHR_PARAM_CHECK(port != NULL);

	memset(&mib, 0, sizeof(mib));

	SHR_RET_ON_ERR(qca808x_secy_rx_mib_get(port, &mib));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_ctl_filt_set(struct macsec_port *port, u32 index,
				       struct secy_ctl_filt_t *secy_filt)
{
	g_error_t rv = ERROR_OK;
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) &&
			(index == 0) &&
			(secy_filt != NULL));

	switch (secy_filt->match_type) {
	case SECY_CTL_COMPARE_DA:
		rv = macsec_rx_ctl_filt_da_mac_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_SA:
		rv = macsec_rx_ctl_filt_sa_mac_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_ETHER_TYPE:
		rv = macsec_rx_ctl_filt_ether_type_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_HALF_DA_SA:
		rv = macsec_rx_ctl_filt_half_da_sa_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_DA_ETHER_TYPE:
		rv = macsec_rx_ctl_filt_da_ether_type_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_SA_ETHER_TYPE:
		rv = macsec_rx_ctl_filt_sa_ether_type_set(port, index, secy_filt);
		break;
	default:
		rv = ERROR_PARAM;
	}
	if (rv != ERROR_OK)
		return rv;

	SHR_RET_ON_ERR(qca808x_secy_rx_ctl_filt_en_set(port, secy_filt->bypass));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		memcpy(&(p_macsec_device_cfg[dev_id]->rx_ctl_filt), secy_filt,
			sizeof(struct secy_ctl_filt_t));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_ctl_filt_get(struct macsec_port *port, u32 index,
				       struct secy_ctl_filt_t *secy_filt)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) &&
			(index == 0) &&
			(secy_filt != NULL));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;
	memcpy(secy_filt, &(p_macsec_device_cfg[dev_id]->rx_ctl_filt),
		sizeof(struct secy_ctl_filt_t));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_ctl_filt_clear(struct macsec_port *port, u32 index)
{
	u32 i = 0;
	u32 dev_id = 0;
	struct secy_udf_filt_t filter;

	SHR_PARAM_CHECK((port != NULL) && (index == 0));

	memset(&filter, 0, sizeof(filter));

	SHR_RET_ON_ERR(qca808x_secy_rx_ctl_filt_en_set(port, FALSE));

	for (i = 0; i < 4; i++)
		SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, i,
				&filter));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		memset(&(p_macsec_device_cfg[dev_id]->rx_ctl_filt), 0,
			sizeof(struct secy_ctl_filt_t));

	return ERROR_OK;
}

g_error_t qca_macsec_rx_ctl_filt_clear_all(struct macsec_port *port)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca_macsec_rx_ctl_filt_clear(port, 0));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_udf_filt_set(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	SHR_PARAM_CHECK((port != NULL) && (filter != NULL));

	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_set(port, index, filter));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_udf_filt_get(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	SHR_PARAM_CHECK((port != NULL) && (filter != NULL));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_filt_get(port, index, filter));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_udf_ufilt_cfg_set(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	SHR_PARAM_CHECK((port != NULL) && (cfg != NULL));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_ufilt_cfg_set(port, cfg));
	return ERROR_OK;
}

g_error_t qca_macsec_rx_udf_ufilt_cfg_get(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	SHR_PARAM_CHECK((port != NULL) && (cfg != NULL));
	SHR_RET_ON_ERR(qca808x_secy_rx_udf_ufilt_cfg_get(port, cfg));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_npn_set(struct macsec_port *port, u32 sc_index,
				   u32 an, u64 next_pn)
{
	u32 dev_id = 0;
	u32 pn_index = 0;
	u32 npn = 0;
	u32 xpn = 0;
	bool xpn_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_xpn_en_get(port, &xpn_enable));
	if (xpn_enable) {
		xpn = (u32)((next_pn >> 16) >> 16);
		SHR_RET_ON_ERR(qca808x_secy_tx_sa_xpn_set(port, sc_index, an, xpn));
	}

	npn = (u32)(next_pn & 0xffffffff);
	SHR_RET_ON_ERR(qca808x_secy_tx_sa_npn_set(port, sc_index, an, npn));

	pn_index = (sc_index * 4 + an);
	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].tx_sa_pn = next_pn;
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_npn_get(struct macsec_port *port, u32 sc_index,
				   u32 an, u64 *next_pn)
{
	u32 dev_id = 0, pn_index = 0, i = 0;
	u32 npn = 0, xpn = 0;
	u64 pn = 0;
	bool xpn_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(next_pn != NULL));

	pn_index = (sc_index * 4 + an);
	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;

	for (i = 0; i < SECY_PN_QUERY_TIMES; i++) {
		SHR_RET_ON_ERR(qca808x_secy_xpn_en_get(port, &xpn_enable));
		if (xpn_enable) {
			SHR_RET_ON_ERR(qca808x_secy_tx_sa_xpn_get(port, sc_index, an, &xpn));
			pn = xpn;
		}
		SHR_RET_ON_ERR(qca808x_secy_tx_sa_npn_get(port, sc_index, an, &npn));
		pn = (pn << 32) | npn;
		*next_pn = pn;
		if (pn >= p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].tx_sa_pn) {
			p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].tx_sa_pn = pn;
			break;
		}
	}
	if (i >= SECY_PN_QUERY_TIMES)
		*next_pn = p_macsec_device_cfg[dev_id]->secy_pn_table[pn_index].tx_sa_pn;

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_create(struct macsec_port *port, u32 sc_index, u32 an)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_create(port, sc_index, an));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_del(struct macsec_port *port, u32 sc_index, u32 an)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_del(port, sc_index, an));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_del_all(struct macsec_port *port)
{
	u32 sc_index = 0, an_index = 0;

	SHR_PARAM_CHECK(port != NULL);

	for (sc_index = 0; sc_index <= SECY_SC_IDX_MAX; sc_index++) {
		for (an_index = 0; an_index <= SECY_AN_IDX_MAX; an_index++)
			qca808x_secy_tx_sa_del(port, sc_index, an_index);
	}

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_in_used_get(struct macsec_port *port, u32 sc_index,
					u32 an, bool *p_in_used)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(p_in_used != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_en_get(port, sc_index, an, p_in_used));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_en_set(struct macsec_port *port, u32 sc_index, u32 an,
				    bool enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_en_set(port, sc_index, an, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_en_get(struct macsec_port *port, u32 sc_index, u32 an,
				    bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_en_get(port, sc_index, an, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sak_set(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key != NULL));

	if ((key->len != SAK_LEN_128BITS) &&
		(key->len != SAK_LEN_256BITS) &&
		(key->len != SAK_LEN_LEGACY))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(qca808x_secy_tx_sak_set(port, sc_index, an, key));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sak_get(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	bool enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &enable));
	if (enable) {
		SHR_RET_ON_ERR(qca808x_secy_tx_sak_get(port, sc_index, an, key));
	} else {
		return ERROR_NOT_SUPPORT;
	}
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_an_set(struct macsec_port *port, u32 sc_index, u32 an)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_tx_sc_an_set(port, sc_index, an));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_an_get(struct macsec_port *port, u32 sc_index, u32 *an)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_sc_an_get(port, sc_index, an));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_ki_set(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key_idendifier != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_ki_set(port, sc_index, an, key_idendifier));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_ki_get(struct macsec_port *port, u32 sc_index, u32 an, struct secy_sa_ki_t *key_idendifier)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(key_idendifier != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_ki_get(port, sc_index, an, key_idendifier));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	SHR_RET_ON_ERR(qca808x_secy_tx_sc_ssci_set(port, sc_index, ssci));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		p_macsec_device_cfg[dev_id]->tx_ssci[sc_index] = ssci;
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci)
{
	u32 dev_id = 0;
	bool enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(ssci != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &enable));
	if (enable) {
		SHR_RET_ON_ERR(qca808x_secy_tx_sc_ssci_get(port, sc_index, ssci));
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		*ssci = p_macsec_device_cfg[dev_id]->tx_ssci[sc_index];
	}
	return ERROR_OK;
}

g_error_t qca_macsec_tx_class_lut_set(struct macsec_port *port, u32 sc_index,
				      struct secy_tx_class_lut_t *pentry)
{
	struct secy_tx_sc_policy_rule_t rule;
	u32 i = 0;
	u32 dev_id = 0;
	bool enable;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(pentry != NULL));

	memset(&rule, 0, sizeof(rule));

	rule.rule_valid = pentry->valid;
	rule.action.tx_sc_index = sc_index;

	for (i = 0; i < 6; i++) {
		rule.mac_da.addr[i] = pentry->da[i];
		rule.mac_sa.addr[i] = pentry->sa[i];
		rule.action.tx_sci.addr[i] = pentry->sci[i];
	}
	rule.action.tx_sci.port = (((u16)(pentry->sci[6])) << 8) |
				   (u16)(pentry->sci[7]);
	rule.ethtype = pentry->ether_type;
	rule.inner_vlanid = pentry->inner_vlanid;
	rule.outer_vlanid = pentry->outer_vlanid;
	rule.bc_flag = pentry->bc_flag;
	rule.action.tx_sc_index = pentry->channel;
	rule.action.tx_tci = pentry->tci;
	rule.action.encryption_offset = pentry->offset;
	rule.rule_mask = pentry->rule_mask;

	SHR_RET_ON_ERR(qca808x_secy_tx_sc_policy_set(port,
			sc_index, &rule));

	if (rule.action.tx_tci & MACSEC_SCB_EN)
		enable = TRUE;
	else
		enable = FALSE;

	SHR_RET_ON_ERR(qca808x_secy_use_scb_en_set(port, enable));

	if (rule.action.tx_tci & MACSEC_INCLUDE_SCI_EN)
		enable = TRUE;
	else
		enable = FALSE;

	SHR_RET_ON_ERR(qca808x_secy_include_sci_en_set(port, enable));

	if (rule.action.tx_tci & MACSEC_ES_EN)
		enable = TRUE;
	else
		enable = FALSE;

	SHR_RET_ON_ERR(qca808x_secy_use_es_en_set(port, enable));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		memcpy(&(p_macsec_device_cfg[dev_id]->tx_lut[sc_index]), pentry,
			sizeof(struct secy_tx_class_lut_t));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_class_lut_get(struct macsec_port *port, u32 sc_index,
				      struct secy_tx_class_lut_t *pentry)
{
	struct secy_tx_sc_policy_rule_t rule;
	u32 i = 0;
	u32 dev_id = 0;
	bool enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(pentry != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &enable));
	if (enable) {
		memset(&rule, 0, sizeof(rule));
		SHR_RET_ON_ERR(qca808x_secy_tx_sc_policy_get(port,
			sc_index, &rule));

		pentry->valid = rule.rule_valid;

		for (i = 0; i < 6; i++) {
			pentry->da[i] = rule.mac_da.addr[i];
			pentry->sa[i] = rule.mac_sa.addr[i];
			pentry->sci[i] = rule.action.tx_sci.addr[i];
		}
		pentry->sci[6] = (u8)(rule.action.tx_sci.port >> 8);
		pentry->sci[7] = (u8)(rule.action.tx_sci.port & 0xff);

		pentry->ether_type = rule.ethtype;
		pentry->inner_vlanid = rule.inner_vlanid;
		pentry->outer_vlanid = rule.outer_vlanid;
		pentry->bc_flag = rule.bc_flag;
		pentry->channel = rule.action.tx_sc_index;
		pentry->tci = rule.action.tx_tci;
		pentry->offset = rule.action.encryption_offset;
		pentry->rule_mask = rule.rule_mask;
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		 memcpy(pentry, &(p_macsec_device_cfg[dev_id]->tx_lut[sc_index]),
			sizeof(struct secy_tx_class_lut_t));
	}

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_in_used_get(struct macsec_port *port, u32 sc_index,
					bool *enable)
{
	u32 dev_id = 0;
	bool dbg_enable = FALSE;
	bool sc_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &dbg_enable));

	if (dbg_enable) {
		SHR_RET_ON_ERR(qca808x_secy_tx_sc_en_get(port, sc_index, &sc_enable));
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		sc_enable = p_macsec_device_cfg[dev_id]->tx_lut[sc_index].valid;
	}

	*enable = sc_enable;
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_en_set(struct macsec_port *port, u32 sc_index, bool enable)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;

	SHR_RET_ON_ERR(qca808x_secy_tx_sc_en_zone_set(port, sc_index, enable,
			p_macsec_device_cfg[dev_id]->tx_lut[sc_index].rule_mask));

	p_macsec_device_cfg[dev_id]->tx_lut[sc_index].valid = enable;
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(enable != NULL));

	SHR_RET_ON_ERR(qca_macsec_tx_sc_in_used_get(port, sc_index, enable));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_create(struct macsec_port *port, u32 sc_index,
				  u8 *sci, u32 len)
{
	struct secy_sci_t tx_sci;
	u32 i = 0;
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(sci != NULL));

	memset(&tx_sci, 0, sizeof(tx_sci));

	for (i = 0; i < 6; i++)
		tx_sci.addr[i] = sci[i];

	tx_sci.port = (((u16)sci[6]) << 8) | ((u16)sci[7]);

	SHR_RET_ON_ERR(qca808x_secy_tx_sci_set(port, sc_index, &tx_sci));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		memcpy(&(p_macsec_device_cfg[dev_id]->tx_lut[sc_index].sci[0]), sci, 8);

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_del(struct macsec_port *port, u32 sc_index)
{
	struct secy_tx_class_lut_t rule;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	memset(&rule, 0, sizeof(rule));

	SHR_RET_ON_ERR(qca_macsec_tx_class_lut_get(port, sc_index, &rule));

	rule.valid = FALSE;
	rule.channel = sc_index;

	SHR_RET_ON_ERR(qca_macsec_tx_class_lut_set(port, sc_index, &rule));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_del_all(struct macsec_port *port)
{
	u32 sc_index = 0;
	struct secy_tx_class_lut_t rule;

	SHR_PARAM_CHECK(port != NULL);

	for (sc_index = 0; sc_index <= SECY_SC_IDX_MAX; sc_index++) {
		memset(&rule, 0, sizeof(rule));
		SHR_RET_ON_ERR(qca_macsec_tx_class_lut_get(port,
				sc_index, &rule));
		rule.valid = FALSE;
		rule.channel = sc_index;

		SHR_RET_ON_ERR(qca_macsec_tx_class_lut_set(port,
				sc_index, &rule));
	}

	return ERROR_OK;
}

g_error_t qca_macsec_tx_class_lut_clear(struct macsec_port *port, u32 sc_index)
{
	struct secy_tx_class_lut_t rule;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	memset(&rule, 0, sizeof(rule));

	SHR_RET_ON_ERR(qca_macsec_tx_class_lut_set(port, sc_index, &rule));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_class_lut_clear_all(struct macsec_port *port)
{
	u32 sc_index = 0;
	struct secy_tx_class_lut_t rule;

	SHR_PARAM_CHECK(port != NULL);

	memset(&rule, 0, sizeof(rule));

	for (sc_index = 0; sc_index < SECY_LUT_MAX; sc_index++)
		SHR_RET_ON_ERR(qca_macsec_tx_class_lut_set(port,
				sc_index, &rule));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_confidentiality_offset_set(struct macsec_port *port, u32 sc_index,
						      enum secy_cofidentiality_offset_t offset)
{
	u32 dev_id = 0;
	u8 encryption_offset = 0;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;

	if (offset == SECY_CONFIDENTIALITY_OFFSET_50)
		encryption_offset = 50;
	else if (offset == SECY_CONFIDENTIALITY_OFFSET_30)
		encryption_offset = 30;
	else
		encryption_offset = 0;

	SHR_RET_ON_ERR(qca808x_secy_tx_tci_offset_zone_set(port, sc_index,
		p_macsec_device_cfg[dev_id]->tx_lut[sc_index].tci, encryption_offset));

	p_macsec_device_cfg[dev_id]->tx_lut[sc_index].offset = encryption_offset;

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_confidentiality_offset_get(struct macsec_port *port, u32 sc_index,
						      enum secy_cofidentiality_offset_t *offset)
{
	u32 dev_id = 0;
	u8 encryption_offset = 0;
	bool dbg_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(offset != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &dbg_enable));

	if (dbg_enable) {
		SHR_RET_ON_ERR(qca808x_secy_tx_confidentiality_offset_get(port, sc_index, &encryption_offset));
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		encryption_offset = p_macsec_device_cfg[dev_id]->tx_lut[sc_index].offset;
	}

	if (encryption_offset == 50)
		*offset = SECY_CONFIDENTIALITY_OFFSET_50;
	else if (encryption_offset == 30)
		*offset = SECY_CONFIDENTIALITY_OFFSET_30;
	else
		*offset = SECY_CONFIDENTIALITY_OFFSET_00;

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_tci_7_2_set(struct macsec_port *port, u32 sc_index, u8 tci)
{
	u32 dev_id = 0;
	u8 rule_tci = 0;
	bool enable;

	SHR_PARAM_CHECK((port != NULL) && (sc_index <= SECY_SC_IDX_MAX));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;

	rule_tci = (tci << 0x2);
	SHR_RET_ON_ERR(qca808x_secy_tx_tci_offset_zone_set(port, sc_index,
		rule_tci, p_macsec_device_cfg[dev_id]->tx_lut[sc_index].offset));

	if (rule_tci & MACSEC_SCB_EN)
		enable = TRUE;
	else
		enable = FALSE;

	SHR_RET_ON_ERR(qca808x_secy_use_scb_en_set(port, enable));

	if (rule_tci & MACSEC_INCLUDE_SCI_EN)
		enable = TRUE;
	else
		enable = FALSE;

	SHR_RET_ON_ERR(qca808x_secy_include_sci_en_set(port, enable));

	if (rule_tci & MACSEC_ES_EN)
		enable = TRUE;
	else
		enable = FALSE;

	SHR_RET_ON_ERR(qca808x_secy_use_es_en_set(port, enable));

	p_macsec_device_cfg[dev_id]->tx_lut[sc_index].tci = rule_tci;

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_tci_7_2_get(struct macsec_port *port, u32 sc_index, u8 *tci)
{
	u32 dev_id = 0;
	u8 rule_tci = 0;
	bool dbg_enable = FALSE;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(tci != NULL));

	SHR_RET_ON_ERR(qca808x_secy_sram_dbg_en_get(port, &dbg_enable));

	if (dbg_enable) {
		SHR_RET_ON_ERR(qca808x_secy_tx_tci_get(port, sc_index, &rule_tci));
	} else {
		dev_id = macsec_get_device_id(port->phydev);
		if (dev_id == INVALID_DEVICE_ID)
			return ERROR_NOT_FOUND;
		rule_tci = p_macsec_device_cfg[dev_id]->tx_lut[sc_index].tci;
	}

	*tci = (rule_tci >> 0x2);

	return ERROR_OK;
}

g_error_t qca_macsec_tx_protect_frames_en_set(struct macsec_port *port, bool enable)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca808x_secy_tx_protect_frames_en_set(port, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_protect_frames_en_get(struct macsec_port *port, bool *enable)
{
	SHR_PARAM_CHECK((port != NULL) && (enable != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_protect_frames_en_get(port, enable));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_mib_get(struct macsec_port *port, u32 sc_index,
				     u32 an, struct secy_tx_sa_mib_t *mib)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX) &&
			(mib != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_mib_get(port, sc_index, an, mib));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_mib_get(struct macsec_port *port, u32 sc_index,
				     struct secy_tx_sc_mib_t *mib)
{
	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(mib != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_sc_mib_get(port, sc_index, mib));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_mib_get(struct macsec_port *port, struct secy_tx_mib_t *mib)
{
	SHR_PARAM_CHECK((port != NULL) && (mib != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_mib_get(port, mib));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_sa_mib_clear(struct macsec_port *port, u32 sc_index, u32 an)
{
	struct secy_tx_sa_mib_t mib;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX) &&
			(an <= SECY_AN_IDX_MAX));

	memset(&mib, 0, sizeof(mib));

	SHR_RET_ON_ERR(qca808x_secy_tx_sa_mib_get(port, sc_index, an, &mib));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_sc_mib_clear(struct macsec_port *port, u32 sc_index)
{
	struct secy_tx_sc_mib_t mib;

	SHR_PARAM_CHECK((port != NULL) &&
			(sc_index <= SECY_SC_IDX_MAX));

	memset(&mib, 0, sizeof(mib));

	SHR_RET_ON_ERR(qca808x_secy_tx_sc_mib_get(port, sc_index, &mib));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_mib_clear(struct macsec_port *port)
{
	struct secy_tx_mib_t mib;

	SHR_PARAM_CHECK(port != NULL);

	memset(&mib, 0, sizeof(mib));

	SHR_RET_ON_ERR(qca808x_secy_tx_mib_get(port, &mib));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_ctl_filt_set(struct macsec_port *port, u32 index,
				     struct secy_ctl_filt_t *secy_filt)
{
	g_error_t rv = ERROR_OK;
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) &&
			(index == 0) &&
			(secy_filt != NULL));

	switch (secy_filt->match_type) {
	case SECY_CTL_COMPARE_DA:
		rv = macsec_tx_ctl_filt_da_mac_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_SA:
		rv = macsec_tx_ctl_filt_sa_mac_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_ETHER_TYPE:
		rv = macsec_tx_ctl_filt_ether_type_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_HALF_DA_SA:
		rv = macsec_tx_ctl_filt_half_da_sa_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_DA_ETHER_TYPE:
		rv = macsec_tx_ctl_filt_da_ether_type_set(port, index, secy_filt);
		break;
	case SECY_CTL_COMPARE_SA_ETHER_TYPE:
		rv = macsec_tx_ctl_filt_sa_ether_type_set(port, index, secy_filt);
		break;
	default:
		rv = ERROR_PARAM;
	}
	if (rv != ERROR_OK)
		return rv;

	SHR_RET_ON_ERR(qca808x_secy_tx_ctl_filt_en_set(port, secy_filt->bypass));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		memcpy(&(p_macsec_device_cfg[dev_id]->tx_ctl_filt), secy_filt,
			sizeof(struct secy_ctl_filt_t));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_ctl_filt_get(struct macsec_port *port, u32 index,
				       struct secy_ctl_filt_t *secy_filt)
{
	u32 dev_id = 0;

	SHR_PARAM_CHECK((port != NULL) &&
			(index == 0) &&
			(secy_filt != NULL));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id == INVALID_DEVICE_ID)
		return ERROR_NOT_FOUND;
	memcpy(secy_filt, &(p_macsec_device_cfg[dev_id]->tx_ctl_filt),
		sizeof(struct secy_ctl_filt_t));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_ctl_filt_clear(struct macsec_port *port, u32 index)
{
	u32 i = 0;
	u32 dev_id = 0;
	struct secy_udf_filt_t filter;

	SHR_PARAM_CHECK((port != NULL) && (index == 0));

	memset(&filter, 0, sizeof(filter));

	SHR_RET_ON_ERR(qca808x_secy_tx_ctl_filt_en_set(port, FALSE));

	for (i = 0; i < 4; i++)
		SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, i, &filter));

	dev_id = macsec_get_device_id(port->phydev);
	if (dev_id != INVALID_DEVICE_ID)
		memset(&(p_macsec_device_cfg[dev_id]->tx_ctl_filt), 0,
			sizeof(struct secy_ctl_filt_t));

	return ERROR_OK;
}

g_error_t qca_macsec_tx_ctl_filt_clear_all(struct macsec_port *port)
{
	SHR_PARAM_CHECK(port != NULL);

	SHR_RET_ON_ERR(qca_macsec_tx_ctl_filt_clear(port, 0));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_udf_filt_set(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	SHR_PARAM_CHECK((port != NULL) && (filter != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_set(port, index, filter));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_udf_filt_get(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	SHR_PARAM_CHECK((port != NULL) && (filter != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_filt_get(port, index, filter));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_udf_ufilt_cfg_set(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	SHR_PARAM_CHECK((port != NULL) && (cfg != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_ufilt_cfg_set(port, cfg));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_udf_ufilt_cfg_get(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	SHR_PARAM_CHECK((port != NULL) && (cfg != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_ufilt_cfg_get(port, cfg));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_udf_cfilt_cfg_set(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	SHR_PARAM_CHECK((port != NULL) && (cfg != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_cfilt_cfg_set(port, cfg));
	return ERROR_OK;
}

g_error_t qca_macsec_tx_udf_cfilt_cfg_get(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	SHR_PARAM_CHECK((port != NULL) && (cfg != NULL));

	SHR_RET_ON_ERR(qca808x_secy_tx_udf_cfilt_cfg_get(port, cfg));
	return ERROR_OK;
}

g_error_t qca_macsec_secy_init(struct macsec_port *port)
{
	u32 sc, sa;
	u32 pn = 1;

	SHR_PARAM_CHECK(port != NULL);

	qca808x_secy_clock_en_set(port, TRUE);
	qca_macsec_tx_mib_clear(port);
	qca_macsec_rx_mib_clear(port);
	qca808x_secy_forward_az_pattern_en_set(port, TRUE);
	for (sc = 0; sc < SECY_SC_IDX_MAX + 1; sc++) {
		qca_macsec_tx_sc_mib_clear(port, sc);
		qca_macsec_rx_sc_mib_clear(port, sc);
		/*SRAM bind table init*/
		qca808x_secy_tx_sc_en_zone_set(port, sc, FALSE, 0);
		qca808x_secy_rx_sc_en_zone_set(port, sc, FALSE, 0);
		for (sa = 0; sa < SECY_SA_IDX_MAX + 1; sa++) {
			qca_macsec_tx_sa_mib_clear(port,
				sc, sa);
			qca_macsec_rx_sa_mib_clear(port,
				sc, sa);
			/*init next pn*/
			qca808x_secy_tx_sa_npn_set(port, sc,
				sa, pn);
			qca808x_secy_rx_sa_npn_set(port, sc,
				sa, pn);
			qca808x_secy_tx_sa_xpn_set(port, sc,
				sa, pn);
			qca808x_secy_rx_sa_xpn_set(port, sc,
				sa, pn);
		}
	}
	return ERROR_OK;
}

int qca_macsec_device_attach(void *dev)
{
	u32 dev_id = 0;
	struct phy_device *phydev = (struct phy_device *)dev;

	if (NULL == phydev)
		return -EINVAL;

	dev_id = macsec_device_register(phydev);
	if (INVALID_DEVICE_ID == dev_id) {
		osal_print("device:%p fail to register!\n", phydev);
		return -ENODEV;
	}
	osal_print("phydev:%p register to macsec dev_id: %d\n", phydev, dev_id);

	p_macsec_device_cfg[dev_id] = devm_kzalloc(&(phydev->mdio.dev),
						   sizeof(struct qca_macsec_device_t), GFP_KERNEL);
	if (!p_macsec_device_cfg[dev_id])
		return -ENOMEM;

	p_macsec_device_cfg[dev_id]->port.phydev = phydev;
	p_macsec_device_cfg[dev_id]->port.type = MACSEC_IN_PHY;

	qca_macsec_secy_init(&(p_macsec_device_cfg[dev_id]->port));

	return 0;
}

int qca_macsec_device_detach(void *dev)
{
	u32 dev_id = 0;
	struct phy_device *phydev = (struct phy_device *)dev;

	if (NULL == phydev)
		return -EINVAL;

	dev_id = macsec_get_device_id(phydev);
	if (INVALID_DEVICE_ID == dev_id) {
		osal_print("%s: fail to find dev_id for device:%p! \n", __func__, phydev);
		return -ENODEV;
	}

	qca808x_secy_clock_en_set(&(p_macsec_device_cfg[dev_id]->port), FALSE);
	if (NULL != p_macsec_device_cfg[dev_id]->offload_secy_cfg) {
		devm_kfree(&(phydev->mdio.dev), p_macsec_device_cfg[dev_id]->offload_secy_cfg);
		p_macsec_device_cfg[dev_id]->offload_secy_cfg = NULL;
	}
	devm_kfree(&(phydev->mdio.dev), p_macsec_device_cfg[dev_id]);
	p_macsec_device_cfg[dev_id] = NULL;

	return 0;
}

void qca_macsec_device_cleanup(void)
{
	u32 dev_id;

	for (dev_id = 0; dev_id < MACSEC_DEVICE_MAXNUM; dev_id++) {
		if (p_macsec_device_cfg[dev_id] != NULL) {
			qca808x_secy_clock_en_set(&(p_macsec_device_cfg[dev_id]->port), FALSE);
			if (NULL != p_macsec_device_cfg[dev_id]->offload_secy_cfg) {
				devm_kfree(&(p_macsec_device_cfg[dev_id]->port.phydev->mdio.dev), p_macsec_device_cfg[dev_id]->offload_secy_cfg);
				p_macsec_device_cfg[dev_id]->offload_secy_cfg = NULL;
			}
			devm_kfree(&(p_macsec_device_cfg[dev_id]->port.phydev->mdio.dev), p_macsec_device_cfg[dev_id]);
			p_macsec_device_cfg[dev_id] = NULL;
		}
	}
}

