/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#include "nss_macsec_types.h"
#include "nss_macsec.h"
#include "dal_utility.h"
#include "qca808x_macsec_reg.h"
#include "qca808x_macsec.h"



g_error_t qca808x_secy_use_es_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_CONFIG,
			       SYS_USE_ES_EN, enable);
}

g_error_t qca808x_secy_use_scb_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_CONFIG,
			       SYS_USE_SCB_EN, enable);
}

g_error_t qca808x_secy_include_sci_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_CONFIG,
			       SYS_INCLUDED_SCI_EN, enable);
}

g_error_t qca808x_secy_controlled_port_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_PORT_CTRL,
			       SYS_PORT_EN, enable);
}

g_error_t qca808x_secy_controlled_port_en_get(struct macsec_port *port, bool *enable)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_PORT_CTRL, SYS_PORT_EN, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_clock_en_set(struct macsec_port *port, bool enable)
{
	u16 val = 0;

	if (enable == TRUE)
		val = SOFTWARE_EN;
	else if (enable == FALSE)
		val = SOFTWARE_BYPASS;
	else
		return ERROR_PARAM;

	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SOFTWARE_EN_CTRL,
			       SYS_SECY_SOFTWARE_EN, val);
}

g_error_t qca808x_secy_loopback_set(struct macsec_port *port,
				    enum secy_loopback_type_t type)
{
	u16 val = 0;

	if (type == MACSEC_NORMAL)
		val = SYS_MACSEC_EN;
	else if (type == MACSEC_SWITCH_LB)
		return ERROR_NOT_SUPPORT;
	else if (type == MACSEC_PHY_LB)
		return ERROR_NOT_SUPPORT;
	else if (type == MACSEC_BYPASS)
		val = SYS_BYPASS;
	else
		return ERROR_PARAM;

	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_PACKET_CTRL,
			       SYS_SECY_LPBK, val);
}

g_error_t qca808x_secy_loopback_get(struct macsec_port *port,
				    enum secy_loopback_type_t *type)
{
	u16 lpbk = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		MACSEC_SYS_PACKET_CTRL, SYS_SECY_LPBK, &lpbk));

	if (lpbk == SYS_BYPASS)
		*type = MACSEC_BYPASS;
	else if (lpbk == SYS_PHY_LB)
		*type = MACSEC_PHY_LB;
	else if (lpbk == SYS_SWITCH_LB)
		*type = MACSEC_SWITCH_LB;
	else
		*type = MACSEC_NORMAL;

	return ERROR_OK;
}

g_error_t qca808x_secy_mtu_set(struct macsec_port *port, u32 len)
{
	u16 val = 0;

	val = (u16)len;

	return reg_access_write(port, PHY_MMD3, MACSEC_SYS_FRAME_LEN, val);
}

g_error_t qca808x_secy_mtu_get(struct macsec_port *port, u32 *len)
{
	u16 val = 0;

	reg_access_read(port, PHY_MMD3, MACSEC_SYS_FRAME_LEN, &val);

	*len = (u32)val;

	return ERROR_OK;
}

g_error_t qca808x_secy_shadow_register_en_set(struct macsec_port *port, bool enable)
{
	if (enable == TRUE) {
		SHR_RET_ON_ERR(REG_FIELD_WRITE(
			       port, PHY_MMD7, MACSEC_SHADOW_REGISTER,
			       MACSEC_SHADOW_DUPLEX_EN, TRUE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE(
			       port, PHY_MMD7, MACSEC_SHADOW_REGISTER,
			       MACSEC_SHADOW_LEGACY_DUPLEX_EN, TRUE));
	} else {
		SHR_RET_ON_ERR(REG_FIELD_WRITE(
			       port, PHY_MMD7, MACSEC_SHADOW_REGISTER,
			       MACSEC_SHADOW_DUPLEX_EN, FALSE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE(
			       port, PHY_MMD7, MACSEC_SHADOW_REGISTER,
			       MACSEC_SHADOW_LEGACY_DUPLEX_EN, FALSE));
	}
	return ERROR_OK;
}

g_error_t qca808x_secy_cipher_suite_set(struct macsec_port *port,
					enum secy_cipher_suite_t cipher_suite)
{
	bool enable = FALSE;
	bool xpn_enable = FALSE;

	if ((cipher_suite == SECY_CIPHER_SUITE_128) ||
	    (cipher_suite == SECY_CIPHER_SUITE_XPN_128))
		enable = FALSE;
	else if ((cipher_suite == SECY_CIPHER_SUITE_256) ||
		 (cipher_suite == SECY_CIPHER_SUITE_XPN_256))
		enable = TRUE;
	else
		return ERROR_NOT_SUPPORT;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_FRAME_CTRL, SYS_AES256_ENABLE, enable));

	if ((cipher_suite == SECY_CIPHER_SUITE_128) ||
	    (cipher_suite == SECY_CIPHER_SUITE_256))
		xpn_enable = FALSE;
	else
		xpn_enable = TRUE;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_FRAME_CTRL, SYS_XPN_ENABLE, xpn_enable));

	return ERROR_OK;
}

g_error_t qca808x_secy_cipher_suite_get(struct macsec_port *port,
					enum secy_cipher_suite_t *cipher_suite)
{
	u16 val = 0, val1 = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_FRAME_CTRL, SYS_AES256_ENABLE, &val));

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_FRAME_CTRL, SYS_XPN_ENABLE, &val1));

	if ((val == MACSEC_AES256_ENABLE) && (val1 == MACSEC_XPN_ENABLE))
		*cipher_suite = SECY_CIPHER_SUITE_XPN_256;
	else if ((val == MACSEC_AES256_ENABLE) && (val1 != MACSEC_XPN_ENABLE))
		*cipher_suite = SECY_CIPHER_SUITE_256;
	else if ((val != MACSEC_AES256_ENABLE) && (val1 == MACSEC_XPN_ENABLE))
		*cipher_suite = SECY_CIPHER_SUITE_XPN_128;
	else
		*cipher_suite = SECY_CIPHER_SUITE_128;

	return ERROR_OK;
}

g_error_t qca808x_secy_sram_dbg_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_CONFIG,
			       SYS_SRAM_DEBUG_EN, enable);
}

g_error_t qca808x_secy_sram_dbg_en_get(struct macsec_port *port, bool *enable)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_SRAM_DEBUG_EN, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sc_en_zone_set(struct macsec_port *port, u32 sc_index,
						bool enable, u16 rule_mask)
{
	u16 msk = 0;

	if (sc_index > SECY_SC_IDX_MAX)
		return ERROR_PARAM;

	if (enable)
		msk |= SC_BIND_MASK_VALID;

	if (rule_mask & SC_BIND_MASK_DA)
		msk |= SC_BIND_MASK_DA;

	if (rule_mask & SC_BIND_MASK_SA)
		msk |= SC_BIND_MASK_SA;

	if (rule_mask & SC_BIND_MASK_ETHERTYPE)
		msk |= SC_BIND_MASK_ETHERTYPE;

	if (rule_mask & SC_BIND_MASK_OUTER_VLAN)
		msk |= SC_BIND_MASK_OUTER_VLAN;

	if (rule_mask & SC_BIND_MASK_INNER_VLAN)
		msk |= SC_BIND_MASK_INNER_VLAN;

	if (rule_mask & SC_BIND_MASK_BCAST)
		msk |= SC_BIND_MASK_BCAST;

	if (rule_mask & SC_BIND_MASK_TCI)
		msk |= SC_BIND_MASK_TCI;

	if (0 != msk)
		msk |= SC_BIND_MASK_SCI;

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
			MACSEC_SC_BIND_RXMASK(sc_index), msk));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable)
{
	u16 val = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (enable == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXMASK(sc_index), SC_BIND_RX_VALID, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sc_policy_set(struct macsec_port *port, u32 rule_index,
					struct secy_rx_sc_policy_rule_t *rule)
{
	u16 val = 0, msk = 0;
	u32 i = 0;

	if ((rule_index > SECY_SC_IDX_MAX) || (rule == NULL))
		return ERROR_PARAM;

	if (rule->rule_valid)
		msk |= SC_BIND_MASK_VALID;

	if (rule->rule_mask & SC_BIND_MASK_DA)
		msk |= SC_BIND_MASK_DA;

	for (i = 0; i < 3; i++) {
		val = (rule->mac_da.addr[i * 2] << 8) |
			(rule->mac_da.addr[i * 2 + 1]);
		SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
			       MACSEC_SC_BIND_RXDA_BASE(rule_index) + 2 - i, val));
	}

	if (rule->rule_mask & SC_BIND_MASK_SA)
		msk |= SC_BIND_MASK_SA;

	for (i = 0; i < 3; i++) {
		val = (rule->mac_sa.addr[i * 2] << 8) |
			(rule->mac_sa.addr[i * 2 + 1]);
		SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
			       MACSEC_SC_BIND_RXSA_BASE(rule_index) + 2 - i, val));
	}

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_ETHERTYPE) {
		val = rule->ethtype;
		msk |= SC_BIND_MASK_ETHERTYPE;
	}
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXETHERTYPE(rule_index), val));

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_OUTER_VLAN) {
		val = rule->outer_vlanid & 0xfff;
		msk |= SC_BIND_MASK_OUTER_VLAN;
	}
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXOUTVTAG(rule_index), val));

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_INNER_VLAN) {
		val = rule->inner_vlanid & 0xfff;
		msk |= SC_BIND_MASK_INNER_VLAN;
	}
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXINVTAG(rule_index), val));

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_BCAST) {
		val |= ((rule->bc_flag == TRUE)?1:0)<<SC_BIND_RX_IFBC_OFFSET;
		msk |= SC_BIND_MASK_BCAST;
	}
	val |= (rule->action.rx_sc_index & 0xf) << SC_BIND_RXCTX_OFFSET;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXCTX(rule_index), val));

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_TCI) {
		val |= (rule->rx_tci & 0xff) << SC_BIND_RXTCI_OFFSET;
		msk |= SC_BIND_MASK_TCI;
	}
	val |= (rule->action.decryption_offset & 0x3f) <<
		SC_BIND_RXOFFSET_OFFSET;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXTCI(rule_index), val));

	val = rule->rx_sci.port;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXSCI_BASE(rule_index), val));

	for (i = 0; i < 3; i++) {
		val = (rule->rx_sci.addr[i * 2] << 8) |
			rule->rx_sci.addr[i * 2 + 1];
		SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
			       MACSEC_SC_BIND_RXSCI_BASE(rule_index) + 3 - i, val));
	}
	if (0 != msk)
		msk |= SC_BIND_MASK_SCI;

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXMASK(rule_index), msk));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sc_policy_get(struct macsec_port *port, u32 rule_index,
					struct secy_rx_sc_policy_rule_t *rule)
{
	u16 val = 0, msk = 0;
	u32 i = 0;

	if ((rule_index > SECY_SC_IDX_MAX) || (rule == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXMASK(rule_index), &msk));
	rule->rule_valid = (msk & SC_BIND_MASK_VALID) ? TRUE : FALSE;
	rule->rule_mask  = msk & 0xff;

	for (i = 0; i < 3; i++) {
		SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
			MACSEC_SC_BIND_RXDA_BASE(rule_index) + 2 - i, &val));
		rule->mac_da.addr[i * 2]   = (val >> 8) & 0xff;
		rule->mac_da.addr[i * 2 + 1] =  val & 0xff;
	}

	for (i = 0; i < 3; i++) {
		SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
			MACSEC_SC_BIND_RXSA_BASE(rule_index) + 2 - i, &val));
		rule->mac_sa.addr[i * 2]   = (val >> 8) & 0xff;
		rule->mac_sa.addr[i * 2 + 1] =  val & 0xff;
	}

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXETHERTYPE(rule_index), &val));
	rule->ethtype = val;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXOUTVTAG(rule_index), &val));
	rule->outer_vlanid = val & 0xfff;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXINVTAG(rule_index), &val));
	rule->inner_vlanid = val & 0xfff;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXCTX(rule_index), SC_BIND_RX_IFBC, &val));
	rule->bc_flag = (val) ? TRUE : FALSE;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXTCI(rule_index), SC_BIND_RXTCI, &val));
	rule->rx_tci = val;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXSCI_BASE(rule_index), &val));
	rule->rx_sci.port = val;

	for (i = 0; i < 3; i++) {
		SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
			MACSEC_SC_BIND_RXSCI_BASE(rule_index) + 3 - i, &val));
		rule->rx_sci.addr[i * 2]     = (val >> 8) & 0xff;
		rule->rx_sci.addr[i * 2 + 1] =  val & 0xff;
	}

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXCTX(rule_index), SC_BIND_RXCTX, &val));
	rule->action.rx_sc_index = val;
	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_RXTCI(rule_index), SC_BIND_RXOFFSET, &val));
	rule->action.decryption_offset = val;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_npn_set(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 next_pn)
{
	u16 val = 0;
	u16 channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);
	val = (u16)(next_pn & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_RX_NPN(channel), val));
	val = (u16)((next_pn >> 16) & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_RX_NPN(channel) + 1, val));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_npn_get(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 *next_pn)
{
	u16 val0 = 0, val1 = 0, channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_RX_NPN(channel), &val0));
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_RX_NPN(channel) + 1, &val1));
	*next_pn = (((u32)val1 << 16) | val0);

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_replay_protect_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3,
			       MACSEC_SYS_CONFIG, SYS_REPLAY_PROTECT_EN, enable);
}

g_error_t qca808x_secy_rx_replay_protect_en_get(struct macsec_port *port, bool *enable)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_REPLAY_PROTECT_EN, &val));
	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_replay_window_set(struct macsec_port *port, u32 window)
{
	u16 val = 0;

	val = (u16)(window & 0xffff);

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SYS_REPLAY_WIN_BASE, val));
	val = (u16)((window >> 16)  & 0xffff);

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SYS_REPLAY_WIN_BASE + 1, val));
	return OK;
}

g_error_t qca808x_secy_rx_replay_window_get(struct macsec_port *port, u32 *window)
{
	u16 val = 0;
	u32 win = 0;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SYS_REPLAY_WIN_BASE, &val));
	win = val;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SYS_REPLAY_WIN_BASE+1, &val));
	win |= val << 16;

	*window = win;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_validate_frame_set(struct macsec_port *port, enum secy_vf_t value)
{
	u16 val = 0;

	if (value == STRICT)
		val = SYS_FRAME_VALIDATE_STRICT;
	else if (value == CHECKED)
		val = SYS_FRAME_VALIDATE_CHECK;
	else if (value == DISABLED)
		val = SYS_FRAME_VALIDATE_DIS;
	else
		return ERROR_PARAM;

	return REG_FIELD_WRITE(port, PHY_MMD3,
			       MACSEC_SYS_FRAME_CTRL, SYS_FRAME_VALIDATE, val);
}

g_error_t qca808x_secy_rx_validate_frame_get(struct macsec_port *port, enum secy_vf_t *value)
{
	u16 validate = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_FRAME_CTRL, SYS_FRAME_VALIDATE, &validate));

	if (validate == SYS_FRAME_VALIDATE_STRICT)
		*value = STRICT;
	else if (validate == SYS_FRAME_VALIDATE_CHECK)
		*value = CHECKED;
	else
		*value = DISABLED;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_create(struct macsec_port *port, u32 sc_index, u32 an)
{
	u16 val = 0, chan_shift;
	u32 reg, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/4;
	chan_shift = 2*((sc_index%4) * 2 + sa_index);

	reg += MACSEC_RX_AN_BASE;

	val  = (((u16)an&AN_MASK) << chan_shift);

	return reg_access_mask_write(port, PHY_MMD3, reg,
				     (AN_MASK << chan_shift), val);
}

g_error_t qca808x_secy_rx_sak_set(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	u16 val = 0;
	u16 reg;
	int i, j, reg_offset;
	u32 channel, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) || (an > SECY_AN_IDX_MAX) ||
			(key == NULL))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;

	if (key->len == SAK_LEN_256BITS) {
		for (i = 0; i < 16; i++) {
			if (i < 8) {
				val = (key->sak[i * 2 + 1] << 8) |
					key->sak[i * 2];
				reg = MACSEC_RX_SAK_KEY0(channel);
				reg_offset = i;
			} else {
				j = i - 8;
				val = (key->sak1[j * 2 + 1] << 8) |
					key->sak1[j * 2];
				reg = MACSEC_RX_EXTENDED_SAK_KEY0(channel);
				reg_offset = j;
			}
			SHR_RET_ON_ERR(reg_access_write(port,
				       PHY_MMD3, reg + reg_offset, val));
		}
	} else {
		for (i = 0; i < 8; i++) {
			reg = MACSEC_RX_SAK_KEY0(channel);
			val = (key->sak[i * 2 + 1] << 8) | key->sak[i * 2];
			SHR_RET_ON_ERR(reg_access_write(port,
				       PHY_MMD3, reg + i, val));
		}
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sak_get(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	u16 val = 0;
	u16 reg;
	int i, j, reg_offset;
	u32 channel, sa_index;
	enum secy_cipher_suite_t cipher_suite;

	if ((sc_index > SECY_SC_IDX_MAX) || (an > SECY_AN_IDX_MAX) ||
			(key == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(qca808x_secy_cipher_suite_get(port, &cipher_suite));

	if ((cipher_suite == SECY_CIPHER_SUITE_128) ||
	    (cipher_suite == SECY_CIPHER_SUITE_XPN_128))
		key->len = 16;
	else
		key->len = 32;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;
	if (key->len == SAK_LEN_256BITS) {
		for (i = 0; i < 16; i++) {
			if (i < 8) {
				reg = MACSEC_RX_SAK_KEY0(channel);
				reg_offset = i;
				SHR_RET_ON_ERR(reg_access_read(port,
					PHY_MMD3, reg + reg_offset, &val));
				key->sak[i * 2] = val & 0xff;
				key->sak[i * 2 + 1] = (val >> 0x8) & 0xff;
			} else {
				reg = MACSEC_RX_EXTENDED_SAK_KEY0(channel);
				reg_offset = i - 8;
				SHR_RET_ON_ERR(reg_access_read(port,
					PHY_MMD3, reg + reg_offset, &val));
				j = i - 8;
				key->sak1[j * 2] = val & 0xff;
				key->sak1[j * 2 + 1] = (val >> 0x8) & 0xff;
			}
		}
	} else {
		for (i = 0; i < 8; i++) {
			reg = MACSEC_RX_SAK_KEY0(channel);
			SHR_RET_ON_ERR(reg_access_read(port,
				PHY_MMD3, reg + i, &val));
			key->sak[i * 2] = val & 0xff;
			key->sak[i * 2 + 1] = (val >> 0x8) & 0xff;
		}
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_en_set(struct macsec_port *port, u32 sc_index,
				    u32 an, bool enable)
{
	u16 val = 0;
	u16 reg, chan_shift;
	u32 sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
		(an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/8;
	chan_shift = ((sc_index%8) * 2 + sa_index);

	reg += MACSEC_RX_SA_CONTROL;

	if (enable)
		val  = (1 << chan_shift);
	else
		val &= ~(1 << chan_shift);

	return reg_access_mask_write(port, PHY_MMD3, reg,
				     (1<<chan_shift), val);
}

g_error_t qca808x_secy_rx_sa_en_get(struct macsec_port *port, u32 sc_index,
				    u32 an, bool *enable)
{
	u16 val = 0;
	u16 reg, chan_shift;
	u32 sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/8;
	chan_shift = ((sc_index%8) * 2 + sa_index);

	reg += MACSEC_RX_SA_CONTROL;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3, reg, &val));

	if (val&(1 << chan_shift))
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_del(struct macsec_port *port, u32 sc_index, u32 an)
{
	u16 val = 0, chan_shift;
	u32 reg, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/4;
	chan_shift = 2*((sc_index%4) * 2 + sa_index);

	reg += MACSEC_RX_AN_BASE;

	val  = (0x0 << chan_shift);

	return reg_access_mask_write(port, PHY_MMD3, reg,
				     (AN_MASK<<chan_shift), val);
}

g_error_t qca808x_secy_tx_tci_offset_zone_set(struct macsec_port *port, u32 sc_index,
						u8 tci, u8 offset)
{
	u16 val = 0;

	if (sc_index > SECY_SC_IDX_MAX)
		return ERROR_PARAM;

	val |= (tci & 0xff) << SC_BIND_TXTCI_OFFSET;
	val |= (offset & 0x3f) << SC_BIND_TXOFFSET_OFFSET;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXTCI(sc_index), val));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_tci_get(struct macsec_port *port, u32 sc_index, u8 *tci)
{
	u16 val = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (tci == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXTCI(sc_index), SC_BIND_TXTCI, &val));
	*tci = val;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_confidentiality_offset_get(struct macsec_port *port, u32 sc_index,
						     u8 *offset)
{
	u16 val = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (offset == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXTCI(sc_index), SC_BIND_TXOFFSET, &val));

	*offset = val & 0xff;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sci_set(struct macsec_port *port, u32 sc_index, struct secy_sci_t *tx_sci)
{
	u32 i = 0;
	u16 val = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (tx_sci == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXSCI_BASE(sc_index), tx_sci->port));
	for (i = 0; i < 3; i++) {
		val = (tx_sci->addr[i * 2] << 8) | tx_sci->addr[i * 2 + 1];
		SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
			       MACSEC_SC_BIND_TXSCI_BASE(sc_index) + 3 - i, val));
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sci_get(struct macsec_port *port, u32 sc_index, struct secy_sci_t *tx_sci)
{
	u16 val = 0;
	u32 i = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (tx_sci == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXSCI_BASE(sc_index), &val));
	tx_sci->port = val;

	for (i = 0; i < 3; i++) {
		SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
			       MACSEC_SC_BIND_TXSCI_BASE(sc_index) + 3 - i, &val));
		tx_sci->addr[i * 2]     = (val >> 8) & 0xff;
		tx_sci->addr[i * 2 + 1] =  val & 0xff;
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_en_zone_set(struct macsec_port *port, u32 sc_index,
					bool enable, u16 rule_mask)
{
	u16 msk = 0;

	if (sc_index > SECY_SC_IDX_MAX)
		return ERROR_PARAM;

	if (enable)
		msk |= SC_BIND_MASK_VALID;

	if (rule_mask & SC_BIND_MASK_DA)
		msk |= SC_BIND_MASK_DA;

	if (rule_mask & SC_BIND_MASK_SA)
		msk |= SC_BIND_MASK_SA;

	if (rule_mask & SC_BIND_MASK_ETHERTYPE)
		msk |= SC_BIND_MASK_ETHERTYPE;

	if (rule_mask & SC_BIND_MASK_OUTER_VLAN)
		msk |= SC_BIND_MASK_OUTER_VLAN;

	if (rule_mask & SC_BIND_MASK_INNER_VLAN)
		msk |= SC_BIND_MASK_INNER_VLAN;

	if (rule_mask & SC_BIND_MASK_BCAST)
		msk |= SC_BIND_MASK_BCAST;

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXMASK(sc_index), msk));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_en_get(struct macsec_port *port, u32 sc_index, bool *enable)
{
	u16 val = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (enable == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXMASK(sc_index), SC_BIND_TX_VALID, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_policy_set(struct macsec_port *port, u32 rule_index,
					struct secy_tx_sc_policy_rule_t *rule)
{
	u16 val = 0, msk = 0;
	u32 i = 0;

	if ((rule_index > SECY_SC_IDX_MAX) || (rule == NULL))
		return ERROR_PARAM;

	if (rule->rule_valid)
		msk |= SC_BIND_MASK_VALID;

	if (rule->rule_mask & SC_BIND_MASK_DA)
		msk |= SC_BIND_MASK_DA;

	for (i = 0; i < 3; i++) {
		val = (rule->mac_da.addr[i * 2] << 8) |
			(rule->mac_da.addr[i * 2 + 1]);
		SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
			       MACSEC_SC_BIND_TXDA_BASE(rule_index) + 2 - i, val));
	}

	if (rule->rule_mask & SC_BIND_MASK_SA)
		msk |= SC_BIND_MASK_SA;

	for (i = 0; i < 3; i++) {
		val = (rule->mac_sa.addr[i * 2] << 8) |
			(rule->mac_sa.addr[i * 2 + 1]);
		SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
			       MACSEC_SC_BIND_TXSA_BASE(rule_index) + 2 - i, val));
	}

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_ETHERTYPE) {
		val = rule->ethtype;
		msk |= SC_BIND_MASK_ETHERTYPE;
	}
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXETHERTYPE(rule_index), val));

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_OUTER_VLAN) {
		val = rule->outer_vlanid & 0xfff;
		msk |= SC_BIND_MASK_OUTER_VLAN;
	}
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXOUTVTAG(rule_index), val));

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_INNER_VLAN) {
		val = rule->inner_vlanid & 0xfff;
		msk |= SC_BIND_MASK_INNER_VLAN;
	}
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXINVTAG(rule_index), val));

	val = 0;
	if (rule->rule_mask & SC_BIND_MASK_BCAST) {
		val |= ((rule->bc_flag == TRUE)?1:0) << SC_BIND_TX_IFBC_OFFSET;
		msk |= SC_BIND_MASK_BCAST;
	}
	val |= (rule->action.tx_sc_index & 0xf) << SC_BIND_TXCTX_OFFSET;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXCTX(rule_index), val));

	val = 0;
	val |= (rule->action.tx_tci & 0xff) << SC_BIND_TXTCI_OFFSET;
	val |= (rule->action.encryption_offset & 0x3f) << SC_BIND_TXOFFSET_OFFSET;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXTCI(rule_index), val));

	SHR_RET_ON_ERR(qca808x_secy_tx_sci_set(port, rule_index,
			&(rule->action.tx_sci)));

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXMASK(rule_index), msk));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_policy_get(struct macsec_port *port, u32 rule_index,
					struct secy_tx_sc_policy_rule_t *rule)
{
	u16 val = 0, msk = 0;
	u32 i = 0;

	if ((rule_index > SECY_SC_IDX_MAX) || (rule == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXMASK(rule_index), &msk));
	rule->rule_valid = (msk & SC_BIND_MASK_VALID) ? TRUE : FALSE;
	rule->rule_mask  = msk & 0x3F;

	for (i = 0; i < 3; i++) {
		SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
			       MACSEC_SC_BIND_TXDA_BASE(rule_index) + 2 - i, &val));
		rule->mac_da.addr[i * 2]   = (val >> 8) & 0xff;
		rule->mac_da.addr[i * 2+1] =  val & 0xff;
	}

	for (i = 0; i < 3; i++) {
		SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
			       MACSEC_SC_BIND_TXSA_BASE(rule_index) + 2 - i, &val));
		rule->mac_sa.addr[i * 2]   = (val >> 8) & 0xff;
		rule->mac_sa.addr[i * 2+1] =  val & 0xff;
	}

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXETHERTYPE(rule_index), &val));
	rule->ethtype = val;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXOUTVTAG(rule_index), &val));
	rule->outer_vlanid = val & 0xfff;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXINVTAG(rule_index), &val));
	rule->inner_vlanid = val & 0xfff;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXCTX(rule_index), SC_BIND_TX_IFBC, &val));
	rule->bc_flag = (val)?TRUE:FALSE;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXCTX(rule_index), SC_BIND_TXCTX, &val));
	rule->action.tx_sc_index = val;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXTCI(rule_index), SC_BIND_TXTCI, &val));
	rule->action.tx_tci = val;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SC_BIND_TXTCI(rule_index), SC_BIND_TXOFFSET, &val));
	rule->action.encryption_offset = val;

	SHR_RET_ON_ERR(qca808x_secy_tx_sci_get(port, rule_index,
				&(rule->action.tx_sci)));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_npn_set(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 next_pn)
{
	u16 val = 0;
	u16 channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);
	val = (u16)(next_pn & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_TX_NPN(channel), val));
	val = (u16)((next_pn >> 16) & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_TX_NPN(channel) + 1, val));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_npn_get(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 *next_pn)
{
	u16 val0 = 0, val1 = 0, channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_TX_NPN(channel), &val0));
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_TX_NPN(channel) + 1, &val1));
	*next_pn = (((u32)val1 << 16) | val0);

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_protect_frames_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_FRAME_CTRL,
			       SYS_FRAME_PROTECT_EN, enable);
}

g_error_t qca808x_secy_tx_protect_frames_en_get(struct macsec_port *port, bool *enable)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_FRAME_CTRL, SYS_FRAME_PROTECT_EN, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sak_set(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	u16 val = 0;
	u16 reg;
	int i, j, reg_offset;
	u32 channel, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX) ||
	    (key == NULL))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;

	if (key->len == SAK_LEN_256BITS) {
		for (i = 0; i < 16; i++) {
			if (i < 8) {
				val = (key->sak[i * 2 + 1] << 8) |
					key->sak[i * 2];
				reg = MACSEC_TX_SAK_KEY0(channel);
				reg_offset = i;
			} else {
				j = i - 8;
				val = (key->sak1[j * 2 + 1] << 8) |
					key->sak1[j * 2];
				reg = MACSEC_TX_EXTENDED_SAK_KEY0(channel);
				reg_offset = j;
			}
			SHR_RET_ON_ERR(reg_access_write(port,
				       PHY_MMD3, reg + reg_offset, val));
		}
	} else {
		reg = MACSEC_TX_SAK_KEY0(channel);
		for (i = 0; i < 8; i++) {
			val = (key->sak[i * 2 + 1] << 8) | key->sak[i * 2];
			reg_offset = i;
			SHR_RET_ON_ERR(reg_access_write(port,
				       PHY_MMD3, reg + reg_offset, val));
		}
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sak_get(struct macsec_port *port, u32 sc_index,
				  u32 an, struct secy_sak_t *key)
{
	u16 val = 0;
	u16 reg;
	int i, j, reg_offset;
	u32 channel, sa_index;
	enum secy_cipher_suite_t cipher_suite;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX) ||
	    (key == NULL))
		return ERROR_PARAM;

	SHR_RET_ON_ERR(qca808x_secy_cipher_suite_get(port, &cipher_suite));

	if ((cipher_suite == SECY_CIPHER_SUITE_128) ||
	    (cipher_suite == SECY_CIPHER_SUITE_XPN_128))
		key->len = 16;
	else
		key->len = 32;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;
	if (key->len == SAK_LEN_256BITS) {
		for (i = 0; i < 16; i++) {
			if (i < 8) {
				reg = MACSEC_TX_SAK_KEY0(channel);
				reg_offset = i;
				SHR_RET_ON_ERR(reg_access_read(port,
					       PHY_MMD3, reg + reg_offset, &val));
				key->sak[i * 2] = val & 0xff;
				key->sak[i * 2 + 1] = (val >> 0x8) & 0xff;
			} else {
				reg = MACSEC_TX_EXTENDED_SAK_KEY0(channel);
				reg_offset = i - 8;
				SHR_RET_ON_ERR(reg_access_read(port,
					       PHY_MMD3, reg + reg_offset, &val));
				j = i - 8;
				key->sak1[j * 2] = val & 0xff;
				key->sak1[j * 2 + 1] = (val >> 0x8) & 0xff;
			}
		}
	} else {
		reg = MACSEC_TX_SAK_KEY0(channel);
		for (i = 0; i < 8; i++) {
			SHR_RET_ON_ERR(reg_access_read(port,
				       PHY_MMD3, reg + i, &val));
			key->sak[i * 2] = val & 0xff;
			key->sak[i * 2 + 1] = (val >> 0x8) & 0xff;
		}
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_en_set(struct macsec_port *port, u32 sc_index, u32 an,
				    bool enable)
{
	u16 val = 0;
	u16 reg, chan_shift;
	u32 sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/8;
	chan_shift = ((sc_index%8) * 2 + sa_index);

	reg += MACSEC_TX_SA_CONTROL;

	if (enable)
		val = (1 << chan_shift);
	else
		val &= ~(1 << chan_shift);

	return reg_access_mask_write(port, PHY_MMD3, reg, (1<<chan_shift), val);
}

g_error_t qca808x_secy_tx_sa_en_get(struct macsec_port *port, u32 sc_index, u32 an,
				    bool *enable)
{
	u16 val = 0;
	u16 reg, chan_shift;
	u32 sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/8;
	chan_shift = ((sc_index%8) * 2 + sa_index);

	reg += MACSEC_TX_SA_CONTROL;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3, reg, &val));

	if (val&(1 << chan_shift))
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_an_set(struct macsec_port *port, u32 sc_index, u32 an)
{
	u16 val = 0;
	u16 reg, chan_shift;
	u32 sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/4;
	chan_shift = 2*((sc_index%4) * 2 + sa_index);

	reg += MACSEC_TX_AN_BASE;

	val = (((u16)an&AN_MASK) << chan_shift);

	return reg_access_mask_write(port, PHY_MMD3, reg,
				     (AN_MASK<<chan_shift), val);
}

g_error_t qca808x_secy_tx_sc_an_get(struct macsec_port *port, u32 sc_index, u32 *an)
{
	u16 val = 0;
	u16 reg, chan_shift;
	bool enable;
	u32 i, an_index = 0, sa_index = 0;

	for (i = 0; i < 4; i++) {
		SHR_RET_ON_ERR(qca808x_secy_tx_sa_en_get(port,
			       sc_index, i, &enable));
		if (enable == TRUE)
			an_index = i;
	}

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an_index > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an_index);
	reg = sc_index/4;
	chan_shift = 2*((sc_index%4) * 2 + sa_index);

	reg += MACSEC_TX_AN_BASE;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3, reg, &val));

	*an = (val>>chan_shift)&AN_MASK;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_create(struct macsec_port *port, u32 sc_index, u32 an)
{
	u16 val = 0;
	u16 reg, chan_shift;
	u32 sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/4;
	chan_shift = 2*((sc_index%4) * 2 + sa_index);

	reg += MACSEC_TX_AN_BASE;

	val = (((u16)an&AN_MASK) << chan_shift);

	return reg_access_mask_write(port, PHY_MMD3, reg,
				     (AN_MASK << chan_shift), val);
}

g_error_t qca808x_secy_tx_sa_del(struct macsec_port *port, u32 sc_index, u32 an)
{
	u16 val = 0, chan_shift;
	u32 reg, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
		(an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	reg = sc_index/4;
	chan_shift = 2*((sc_index%4) * 2 + sa_index);

	reg += MACSEC_TX_AN_BASE;

	val = (0x0 << chan_shift);

	return reg_access_mask_write(port, PHY_MMD3, reg,
				     (AN_MASK << chan_shift), val);
}

static u32 qca808x_mib_addr(u32 sc_index, u32 sa_index, u32 addr)
{
	u32 reg = 0;

	switch (addr) {
	case RX_SA_UNUSED_PKTS_BASE:
	case RX_SA_NOUSING_PKTS_BASE:
	case RX_SA_NOTVALID_PKTS_BASE:
	case RX_SA_INVALID_PKTS_BASE:
	case RX_SA_OK_PKTS_BASE:
	case TX_SA_PROTECTED_PKTS_BASE:
	case TX_SA_ENCRYPTED_PKTS_BASE:
	case TX_SA_PROTECTED_PKTS_HIGH_BASE:
	case TX_SA_ENCRYPTED_PKTS_HIGH_BASE:
	case RX_SA_UNUSED_PKTS_HIGH_BASE:
	case RX_SA_NOUSING_PKTS_HIGH_BASE:
	case RX_SA_NOTVALID_PKTS_HIGH_BASE:
	case RX_SA_INVALID_PKTS_HIGH_BASE:
	case RX_SA_OK_PKTS_HIGH_BASE:
		reg = addr + (sc_index * 2 + sa_index) * 2;
		break;
	case RX_SC_LATE_PKTS_BASE:
	case RX_SC_DELAYED_PKTS_BASE:
	case RX_SC_UNCHECKED_PKTS_BASE:
	case RX_SC_VALIDATED_PKTS_BASE:
	case RX_SC_DECRYPTED_PKTS_BASE:
	case TX_SC_PROTECTED_OCTETS_BASE:
	case TX_SC_ENCRYPTED_OCTETS_BASE:
		reg = addr + sc_index * 2;
		break;
	default:
		reg = addr;
		break;
	}

	return reg;
}

static g_error_t qca808x_secy_mib_read(struct macsec_port *port, u32 reg_addr, u32 *mib_val)
{
	u16 val = 0;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       reg_addr + 1, &val));
	*mib_val = val << 16;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       reg_addr, &val));
	*mib_val += val;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_mib_get(struct macsec_port *port, u32 sc_index,
				     u32 an, struct secy_tx_sa_mib_t *mib)
{
	u32 mib_low = 0, mib_high = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX) || (mib == NULL))
		return ERROR_PARAM;

	memset(mib, 0, sizeof(*mib));

	sa_index = SECY_AN_TO_SA_MAPPING(an);

	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_SA_PROTECTED_PKTS_HIGH_BASE), &mib_high));
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_SA_PROTECTED_PKTS_BASE), &mib_low));
	mib->protected_pkts = ((u64)mib_high << 32) | (u64)mib_low;

	mib_low = 0; mib_high = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_SA_ENCRYPTED_PKTS_HIGH_BASE), &mib_high));
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_SA_ENCRYPTED_PKTS_BASE), &mib_low));
	mib->encrypted_pkts = ((u64)mib_high << 32) | (u64)mib_low;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_mib_get(struct macsec_port *port, u32 sc_index,
				     struct secy_tx_sc_mib_t *mib)
{
	u32 val = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (mib == NULL))
		return ERROR_PARAM;

	memset(mib, 0, sizeof(*mib));

	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_SC_PROTECTED_OCTETS_BASE), &val));
	mib->protected_octets = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_SC_ENCRYPTED_OCTETS_BASE), &val));
	mib->encrypted_octets = (u64)val;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_mib_get(struct macsec_port *port, struct secy_tx_mib_t *mib)
{
	u32 val = 0;
	u32 sc_index = 0, sa_index = 0;

	if (mib == NULL)
		return ERROR_PARAM;

	memset(mib, 0, sizeof(*mib));

	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_UNTAGGED_PKTS), &val));
	mib->untagged_pkts = (u64)val;
	val = 0;

	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       TX_TOO_LONG_PKTS), &val));
	mib->too_long = (u64)val;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_mib_get(struct macsec_port *port, u32 sc_index,
				     u32 an, struct secy_rx_sa_mib_t *mib)
{
	u32 mib_low = 0, mib_high = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX) || (mib == NULL))
		return ERROR_PARAM;

	memset(mib, 0, sizeof(*mib));

	sa_index = SECY_AN_TO_SA_MAPPING(an);

	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_UNUSED_PKTS_HIGH_BASE), &mib_high));
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_UNUSED_PKTS_BASE), &mib_low));
	mib->unused_sa = ((u64)mib_high << 32) | (u64)mib_low;

	mib_low = 0;
	mib_high = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_NOUSING_PKTS_HIGH_BASE), &mib_high));
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_NOUSING_PKTS_BASE), &mib_low));
	mib->not_using_sa = ((u64)mib_high << 32) | (u64)mib_low;

	mib_low = 0;
	mib_high = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_NOTVALID_PKTS_HIGH_BASE), &mib_high));
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_NOTVALID_PKTS_BASE), &mib_low));
	mib->not_valid_pkts = ((u64)mib_high << 32) | (u64)mib_low;

	mib_low = 0;
	mib_high = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_INVALID_PKTS_HIGH_BASE), &mib_high));
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_INVALID_PKTS_BASE), &mib_low));
	mib->invalid_pkts = ((u64)mib_high << 32) | (u64)mib_low;

	mib_low = 0;
	mib_high = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_OK_PKTS_HIGH_BASE), &mib_high));
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SA_OK_PKTS_BASE), &mib_low));
	mib->ok_pkts = ((u64)mib_high << 32) | (u64)mib_low;



	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sc_mib_get(struct macsec_port *port, u32 sc_index,
				     struct secy_rx_sc_mib_t *mib)
{
	u32 val = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (mib == NULL))
		return ERROR_PARAM;

	memset(mib, 0, sizeof(*mib));

	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SC_LATE_PKTS_BASE), &val));
	mib->late_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SC_DELAYED_PKTS_BASE), &val));
	mib->delayed_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SC_UNCHECKED_PKTS_BASE), &val));
	mib->unchecked_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SC_VALIDATED_PKTS_BASE), &val));
	mib->validated_octets = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_SC_DECRYPTED_PKTS_BASE), &val));
	mib->decrypted_octets = (u64)val;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_mib_get(struct macsec_port *port, struct secy_rx_mib_t *mib)
{
	u32 sc_index = 0, sa_index = 0;
	u32 val = 0;

	if (mib == NULL)
		return ERROR_PARAM;

	memset(mib, 0, sizeof(*mib));

	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_UNTAGGED_PKTS), &val));
	mib->untagged_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_NO_TAG_PKTS), &val));
	mib->notag_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_BAD_TAG_PKTS), &val));
	mib->bad_tag_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_UNKNOWN_SCI_PKTS), &val));
	mib->unknown_sci_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_NO_SCI_PKTS), &val));
	mib->no_sci_pkts = (u64)val;

	val = 0;
	SHR_RET_ON_ERR(qca808x_secy_mib_read(port,
		       qca808x_mib_addr(sc_index, sa_index,
		       RX_OVERRUN_PKTS), &val));
	mib->too_long_packets = (u64)val;

	return ERROR_OK;
}

g_error_t qca808x_secy_xpn_en_set(struct macsec_port *port, bool enable)
{
	return REG_FIELD_WRITE(port, PHY_MMD3, MACSEC_SYS_FRAME_CTRL,
			       SYS_XPN_ENABLE, enable);
}

g_error_t qca808x_secy_xpn_en_get(struct macsec_port *port, bool *enable)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_FRAME_CTRL, SYS_XPN_ENABLE, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_xpn_set(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 next_pn)
{
	u16 val = 0;
	u16 channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);
	val = (u16)(next_pn & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_RX_XPN(channel), val));
	val = (u16)((next_pn >> 16) & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_RX_XPN(channel) + 1, val));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_xpn_get(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 *next_pn)
{
	u16 val0 = 0, val1 = 0, channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_RX_XPN(channel), &val0));
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_RX_XPN(channel) + 1, &val1));
	*next_pn = (((u32)val1 << 16) | val0);

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_xpn_set(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 next_pn)
{
	u16 val = 0;
	u16 channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);
	val = (u16)(next_pn & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_TX_XPN(channel), val));
	val = (u16)((next_pn >> 16) & 0xffff);
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_TX_XPN(channel) + 1, val));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_xpn_get(struct macsec_port *port, u32 sc_index,
				     u32 an, u32 *next_pn)
{
	u16 val0 = 0, val1 = 0, channel = 0;
	u32 sa_index = 0;

	if ((sc_index > SECY_SC_IDX_MAX) || (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = (sc_index * 2 + sa_index);

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_TX_XPN(channel), &val0));
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_TX_XPN(channel) + 1, &val1));
	*next_pn = (((u32)val1 << 16) | val0);

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci)
{
	u16 val = 0, reg = 0;
	u32 i;

	if (sc_index > SECY_SC_IDX_MAX)
		return ERROR_PARAM;

	reg = MACSEC_SC_RX_SSCI0(sc_index);

	for (i = 0; i < 2; i++) {
		val = (ssci >> (i * 16)) & 0xffff;
		SHR_RET_ON_ERR(reg_access_write(port,
			PHY_MMD3, reg + i, val));
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci)
{
	u16 val = 0, reg = 0;
	u32 i;

	if (sc_index > SECY_SC_IDX_MAX)
		return ERROR_PARAM;

	reg = MACSEC_SC_RX_SSCI0(sc_index);

	*ssci = 0;
	for (i = 0; i < 2; i++) {
		SHR_RET_ON_ERR(reg_access_read(port,
			       PHY_MMD3, reg + i, &val));
		*ssci |= (((u32)val) << (i * 16));
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_ssci_set(struct macsec_port *port, u32 sc_index, u32 ssci)
{
	u16 val = 0, reg = 0;
	u32 i;

	if (sc_index > SECY_SC_IDX_MAX)
		return ERROR_PARAM;

	reg = MACSEC_SC_TX_SSCI0(sc_index);

	for (i = 0; i < 2; i++) {
		val = (ssci >> (i * 16)) & 0xffff;
		SHR_RET_ON_ERR(reg_access_write(port,
			       PHY_MMD3, reg + i, val));
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sc_ssci_get(struct macsec_port *port, u32 sc_index, u32 *ssci)
{
	u16 val = 0, reg = 0;
	u32 i;

	if (sc_index > SECY_SC_IDX_MAX)
		return ERROR_PARAM;

	reg = MACSEC_SC_TX_SSCI0(sc_index);

	*ssci = 0;
	for (i = 0; i < 2; i++) {
		SHR_RET_ON_ERR(reg_access_read(port,
			PHY_MMD3, reg + i, &val));
		*ssci |= (((u32)val) << (i * 16));

	}

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_sa_ki_set(struct macsec_port *port, u32 sc_index,
				    u32 an, struct secy_sa_ki_t *key_idendifier)
{
	u16 val = 0;
	u16 reg;
	int i;
	u32 channel, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;
	reg = MACSEC_RX_KI_KEY0(channel);

	for (i = 0; i < 8; i++) {
		val = (key_idendifier->ki[i * 2 + 1] << 8) |
				key_idendifier->ki[i*2];
		SHR_RET_ON_ERR(reg_access_write(port,
			       PHY_MMD3, reg + i, val));
	}

	return ERROR_OK;
}
g_error_t qca808x_secy_rx_sa_ki_get(struct macsec_port *port, u32 sc_index,
				    u32 an, struct secy_sa_ki_t *key_idendifier)
{
	u16 val = 0;
	u16 reg;
	int i;
	u32 channel, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;
	reg = MACSEC_RX_KI_KEY0(channel);

	for (i = 0; i < 8; i++) {
		SHR_RET_ON_ERR(reg_access_read(port,
			       PHY_MMD3, reg + i, &val));
		key_idendifier->ki[i*2] = val & 0xff;
		key_idendifier->ki[i * 2 + 1] = (val >> 0x8) & 0xff;
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_ki_set(struct macsec_port *port, u32 sc_index,
				    u32 an, struct secy_sa_ki_t *key_idendifier)
{
	u16 val = 0;
	u16 reg;
	int i;
	u32 channel, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;
	reg = MACSEC_TX_KI_KEY0(channel);

	for (i = 0; i < 8; i++) {
		val = (key_idendifier->ki[i * 2 + 1] << 8) |
				key_idendifier->ki[i*2];
		SHR_RET_ON_ERR(reg_access_write(port,
			       PHY_MMD3, reg + i, val));
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_sa_ki_get(struct macsec_port *port, u32 sc_index,
				    u32 an, struct secy_sa_ki_t *key_idendifier)
{
	u16 val = 0;
	u16 reg;
	int i;
	u32 channel, sa_index;

	if ((sc_index > SECY_SC_IDX_MAX) ||
	    (an > SECY_AN_IDX_MAX))
		return ERROR_PARAM;

	sa_index = SECY_AN_TO_SA_MAPPING(an);
	channel = sc_index * 2 + sa_index;
	reg = MACSEC_TX_KI_KEY0(channel);

	for (i = 0; i < 8; i++) {
		SHR_RET_ON_ERR(reg_access_read(port,
			       PHY_MMD3, reg + i, &val));
		key_idendifier->ki[i*2] = val & 0xff;
		key_idendifier->ki[i * 2 + 1] = (val >> 0x8) & 0xff;

	}

	return ERROR_OK;
}

g_error_t qca808x_secy_flow_control_en_set(struct macsec_port *port, bool enable)
{
	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_PAUSE_CTRL_BASE, SYS_E_PAUSE_EN, enable));
	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_PAUSE_CTRL_BASE, SYS_I_PAUSE_EN, enable));

	return ERROR_OK;
}

g_error_t qca808x_secy_flow_control_en_get(struct macsec_port *port, bool *enable)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_PAUSE_CTRL_BASE, SYS_E_PAUSE_EN, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	return ERROR_OK;
}

g_error_t qca808x_secy_special_pkt_ctrl_set(struct macsec_port *port,
					    enum secy_packet_type_t packet_type,
					    enum secy_packet_action_t action)
{
	u16 val  = 0;

	if (action == PACKET_PLAIN_OR_UPLOAD)
		val = MSB_PLAIN_LSB_UPLOAD;
	else if (action == PACKET_PLAIN_OR_DISCARD)
		val = MSB_PLAIN_LSB_DISCARD;
	else if (action == PACKET_CRYPTO_OR_UPLOAD)
		val = MSB_CRYPTO_LSB_UPLOAD;
	else if (action == PACKET_CRYPTO_OR_DISCARD)
		val = MSB_CRYPTO_LSB_DISCARD;
	else
		return ERROR_PARAM;

	if (packet_type == PACKET_STP) {
		SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
			       MACSEC_SYS_PACKET_CTRL, SYS_STP_SW, val));
	} else if (packet_type == PACKET_CDP) {
		SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
			       MACSEC_SYS_PACKET_CTRL, SYS_CDP_SW, val));
	} else if (packet_type == PACKET_LLDP) {
		SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
			       MACSEC_SYS_PACKET_CTRL, SYS_LLDP_SW, val));
	} else  {
		return ERROR_NOT_SUPPORT;
	}

	return ERROR_OK;
}

g_error_t qca808x_secy_special_pkt_ctrl_get(struct macsec_port *port,
					    enum secy_packet_type_t packet_type,
					    enum secy_packet_action_t *action)
{
	u16 val  = 0;

	if (packet_type == PACKET_STP) {
		SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
			       MACSEC_SYS_PACKET_CTRL, SYS_STP_SW, &val));
	} else if (packet_type == PACKET_CDP) {
		SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
			       MACSEC_SYS_PACKET_CTRL, SYS_CDP_SW, &val));
	} else if (packet_type == PACKET_LLDP) {
		SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
			       MACSEC_SYS_PACKET_CTRL, SYS_LLDP_SW, &val));
	} else  {
		return ERROR_NOT_SUPPORT;
	}

	if (val == MSB_PLAIN_LSB_UPLOAD)
		*action = PACKET_PLAIN_OR_UPLOAD;
	else if (val == MSB_PLAIN_LSB_DISCARD)
		*action = PACKET_PLAIN_OR_DISCARD;
	else if (val == MSB_CRYPTO_LSB_UPLOAD)
		*action = PACKET_CRYPTO_OR_UPLOAD;
	else
		*action = PACKET_CRYPTO_OR_DISCARD;

	return ERROR_OK;
}

g_error_t qca808x_secy_udf_ethtype_set(struct macsec_port *port, bool enable, u16 type)
{
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SYS_RX_ETHTYPE, type));
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_SYS_TX_ETHTYPE, type));
	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_RX_UDEF_ETH_TYPE_EN, enable));
	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_TX_UDEF_ETH_TYPE_EN, enable));

	return ERROR_OK;
}

g_error_t qca808x_secy_udf_ethtype_get(struct macsec_port *port, bool *enable, u16 *type)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_RX_UDEF_ETH_TYPE_EN, &val));

	if (val)
		*enable = TRUE;
	else
		*enable = FALSE;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_SYS_RX_ETHTYPE, type));

	return ERROR_OK;
}

g_error_t qca808x_secy_forward_az_pattern_en_set(struct macsec_port *port, bool enable)
{
	if (enable == TRUE) {
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_FORWARD_AZ_PATTERN_EN_CTRL,
			       LPI_RCOVER_EN, FALSE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_FORWARD_AZ_PATTERN_EN_CTRL,
			       E_FORWARD_PATTERN_EN, TRUE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_FORWARD_AZ_PATTERN_EN_CTRL,
			       I_FORWARD_PATTERN_EN, TRUE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_LOCAL_AZ_PATTERN_EN_CTRL,
			       E_MAC_LPI_EN, FALSE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_LOCAL_AZ_PATTERN_EN_CTRL,
			       I_MAC_LPI_EN, FALSE));
	} else {
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_FORWARD_AZ_PATTERN_EN_CTRL,
			       LPI_RCOVER_EN, TRUE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_FORWARD_AZ_PATTERN_EN_CTRL,
			       E_FORWARD_PATTERN_EN, FALSE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_FORWARD_AZ_PATTERN_EN_CTRL,
			       I_FORWARD_PATTERN_EN, FALSE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_LOCAL_AZ_PATTERN_EN_CTRL,
			       E_MAC_LPI_EN, TRUE));
		SHR_RET_ON_ERR(REG_FIELD_WRITE
			       (port, PHY_MMD3, MACSEC_LOCAL_AZ_PATTERN_EN_CTRL,
			       I_MAC_LPI_EN, TRUE));
	}
	return ERROR_OK;
}

static g_error_t _qca808x_secy_tx_epr_property_set(struct macsec_port *port,
						   struct secy_udf_filt_cfg_t *cfg)
{
	u16 val = 0;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_E_EPR_CONFIG, E_EPR_PRIORITY, cfg->priority));

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_E_EPR_CONFIG, E_EPR_REVERSE, cfg->inverse));

	if (cfg->pattern0 == SECY_FILTER_PATTERN_AND)
		val = SECY_EPR_PATTERN0;
	else if (cfg->pattern0 == SECY_FILTER_PATTERN_OR)
		val = SECY_EPR_PATTERN1;
	else
		val = SECY_EPR_PATTERN2;
	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_E_EPR_CONFIG, E_EPR_PATTERN0, val));

	if (cfg->pattern1 == SECY_FILTER_PATTERN_AND)
		val = SECY_EPR_PATTERN0;
	else if (cfg->pattern1 == SECY_FILTER_PATTERN_OR)
		val = SECY_EPR_PATTERN1;
	else
		val = SECY_EPR_PATTERN2;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_E_EPR_CONFIG, E_EPR_PATTERN1, val));

	if (cfg->pattern2 == SECY_FILTER_PATTERN_AND)
		val = SECY_EPR_PATTERN0;
	else if (cfg->pattern2 == SECY_FILTER_PATTERN_OR)
		val = SECY_EPR_PATTERN1;
	else
		val = SECY_EPR_PATTERN2;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_E_EPR_CONFIG, E_EPR_PATTERN2, val));

	return ERROR_OK;
}

static g_error_t _qca808x_secy_tx_epr_property_get(struct macsec_port *port,
						   struct secy_udf_filt_cfg_t *cfg)
{
	u16 val = 0;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_E_EPR_CONFIG, &val));

	cfg->inverse = (val >> E_EPR_REVERSE_OFFSET) & 0xf;
	cfg->priority = (val >> E_EPR_PRIORITY_OFFSET) & 0x7;
	if (((val >> E_EPR_PATTERN0_OFFSET) & 0x3) == SECY_EPR_PATTERN0)
		cfg->pattern0 = SECY_FILTER_PATTERN_AND;
	else if (((val >> E_EPR_PATTERN0_OFFSET) & 0x3) == SECY_EPR_PATTERN1)
		cfg->pattern0 = SECY_FILTER_PATTERN_OR;
	else
		cfg->pattern0 = SECY_FILTER_PATTERN_XOR;

	if (((val >> E_EPR_PATTERN1_OFFSET) & 0x3) == SECY_EPR_PATTERN0)
		cfg->pattern1 = SECY_FILTER_PATTERN_AND;
	else if (((val >> E_EPR_PATTERN1_OFFSET) & 0x3) == SECY_EPR_PATTERN1)
		cfg->pattern1 = SECY_FILTER_PATTERN_OR;
	else
		cfg->pattern1 = SECY_FILTER_PATTERN_XOR;

	if ((val & 0x3) == SECY_EPR_PATTERN0)
		cfg->pattern2 = SECY_FILTER_PATTERN_AND;
	else if ((val & 0x3) == SECY_EPR_PATTERN1)
		cfg->pattern2 = SECY_FILTER_PATTERN_OR;
	else
		cfg->pattern2 = SECY_FILTER_PATTERN_XOR;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_udf_ufilt_cfg_set(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	u16 val = FALSE, val1 = FALSE;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_CEPR_EN, &val1));

	if (cfg->enable) {
		val = TRUE;
		val1 = FALSE;
	} else
		val = FALSE;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UEPR_EN, val));

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_CEPR_EN, val1));

	SHR_RET_ON_ERR(_qca808x_secy_tx_epr_property_set(port, cfg));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_udf_ufilt_cfg_get(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	u16 val = 0;

	memset(cfg, 0, sizeof(*cfg));

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UEPR_EN, &val));

	if (val)
		cfg->enable = TRUE;
	else
		cfg->enable = FALSE;

	SHR_RET_ON_ERR(_qca808x_secy_tx_epr_property_get(port, cfg));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_udf_cfilt_cfg_set(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	u16 val = 0, val1 = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UEPR_EN, &val));

	if (cfg->enable) {
		val = FALSE;
		val1 = TRUE;
	} else
		val1 = FALSE;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UEPR_EN, val));

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_CEPR_EN, val1));

	SHR_RET_ON_ERR(_qca808x_secy_tx_epr_property_set(port, cfg));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_udf_cfilt_cfg_get(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	u16 val = 0;

	memset(cfg, 0, sizeof(*cfg));

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_CEPR_EN, &val));

	if (val)
		cfg->enable = TRUE;
	else
		cfg->enable = FALSE;

	SHR_RET_ON_ERR(_qca808x_secy_tx_epr_property_get(port, cfg));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_udf_ufilt_cfg_set(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	bool val = FALSE;

	if (cfg->enable)
		val = TRUE;
	else
		val = FALSE;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UIER_EN, val));

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_I_EPR_CONFIG, I_EPR_PRIORITY, cfg->priority));

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_I_EPR_CONFIG, I_EPR_REVERSE, cfg->inverse));

	if (cfg->pattern0 == SECY_FILTER_PATTERN_AND)
		val = SECY_EPR_PATTERN0;
	else if (cfg->pattern0 == SECY_FILTER_PATTERN_OR)
		val = SECY_EPR_PATTERN1;
	else
		val = SECY_EPR_PATTERN2;
	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_I_EPR_CONFIG, I_EPR_PATTERN0, val));

	if (cfg->pattern1 == SECY_FILTER_PATTERN_AND)
		val = SECY_EPR_PATTERN0;
	else if (cfg->pattern1 == SECY_FILTER_PATTERN_OR)
		val = SECY_EPR_PATTERN1;
	else
		val = SECY_EPR_PATTERN2;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_I_EPR_CONFIG, I_EPR_PATTERN1, val));

	if (cfg->pattern2 == SECY_FILTER_PATTERN_AND)
		val = SECY_EPR_PATTERN0;
	else if (cfg->pattern2 == SECY_FILTER_PATTERN_OR)
		val = SECY_EPR_PATTERN1;
	else
		val = SECY_EPR_PATTERN2;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_I_EPR_CONFIG, I_EPR_PATTERN2, val));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_udf_ufilt_cfg_get(struct macsec_port *port,
					    struct secy_udf_filt_cfg_t *cfg)
{
	u16 val = 0;

	memset(cfg, 0, sizeof(*cfg));

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UIER_EN, &val));

	if (val)
		cfg->enable = TRUE;
	else
		cfg->enable = FALSE;

	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_I_EPR_CONFIG, &val));

	cfg->inverse = (val >> I_EPR_REVERSE_OFFSET) & 0xf;
	cfg->priority = (val >> I_EPR_PRIORITY_OFFSET) & 0x7;
	if (((val >> I_EPR_PATTERN0_OFFSET) & 0x3) == SECY_EPR_PATTERN0)
		cfg->pattern0 = SECY_FILTER_PATTERN_AND;
	else if (((val >> I_EPR_PATTERN0_OFFSET) & 0x3) == SECY_EPR_PATTERN1)
		cfg->pattern0 = SECY_FILTER_PATTERN_OR;
	else
		cfg->pattern0 = SECY_FILTER_PATTERN_XOR;

	if (((val >> I_EPR_PATTERN1_OFFSET) & 0x3) == SECY_EPR_PATTERN0)
		cfg->pattern1 = SECY_FILTER_PATTERN_AND;
	else if (((val >> I_EPR_PATTERN1_OFFSET) & 0x3) == SECY_EPR_PATTERN1)
		cfg->pattern1 = SECY_FILTER_PATTERN_OR;
	else
		cfg->pattern1 = SECY_FILTER_PATTERN_XOR;

	if ((val & 0x3) == SECY_EPR_PATTERN0)
		cfg->pattern2 = SECY_FILTER_PATTERN_AND;
	else if ((val & 0x3) == SECY_EPR_PATTERN1)
		cfg->pattern2 = SECY_FILTER_PATTERN_OR;
	else
		cfg->pattern2 = SECY_FILTER_PATTERN_XOR;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_udf_filt_set(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	u16 val = 0;
	u16 val_type = 0, val_offset;

	if (index > SECY_FILTER_MAX_INDEX)
		return ERROR_PARAM;

	val = filter->udf_field0;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_E_EPR_FIELD0(index), val));

	val = filter->udf_field1;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_E_EPR_FIELD1(index), val));

	val = filter->udf_field2;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_E_EPR_FIELD2(index), val));

	val = 0;
	val |= (filter->mask & 0x3f) << E_EPR_MASK_OFFSET;

	if ((filter->type & 0x3) == SECY_FILTER_IP_PACKET)
		val_type = SECY_EPR_IP_PACKET;
	else if ((filter->type & 0x3) == SECY_FILTER_TCP_PACKET)
		val_type = SECY_EPR_TCP_PACKET;
	else
		val_type = SECY_EPR_ANY_PACKET;

	val |= val_type << E_EPR_TYPE_OFFSET;

	val_offset = filter->offset & 0x3f;
	val |= val_offset << E_EPR_OFFSET_OFFSET;

	val |= (filter->operator & 0x1) << E_EPR_OPERATOR_OFFSET;

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_E_EPR_MSK_TYPE_OP(index), val));

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_udf_filt_get(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	u16 val = 0;

	memset(filter, 0, sizeof(*filter));

	if (index > SECY_FILTER_MAX_INDEX)
		return ERROR_PARAM;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_E_EPR_FIELD0(index), &val));
	filter->udf_field0 = val;

	val = 0;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_E_EPR_FIELD1(index), &val));
	filter->udf_field1 = val;

	val = 0;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_E_EPR_FIELD2(index), &val));
	filter->udf_field2 = val;

	val = 0;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_E_EPR_MSK_TYPE_OP(index), &val));

	filter->mask = val & 0x3f;

	if (((val >> E_EPR_TYPE_OFFSET) & 0x3) == SECY_EPR_IP_PACKET)
		filter->type = SECY_FILTER_IP_PACKET;
	else if (((val >> E_EPR_TYPE_OFFSET) & 0x3) == SECY_EPR_TCP_PACKET)
		filter->type = SECY_FILTER_TCP_PACKET;
	else
		filter->type = SECY_FILTER_ANY_PACKET;

	filter->offset = (val >> E_EPR_OFFSET_OFFSET) & 0x3f;

	if ((val >> E_EPR_OPERATOR_OFFSET) & 0x1)
		filter->operator = SECY_FILTER_OPERATOR_LESS;
	else
		filter->operator = SECY_FILTER_OPERATOR_EQUAL;

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_udf_filt_set(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	u16 val = 0;
	u16 val_type = 0, val_offset;

	if (index > SECY_FILTER_MAX_INDEX)
		return ERROR_PARAM;

	val = filter->udf_field0;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_I_EPR_FIELD0(index), val));

	val = filter->udf_field1;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_I_EPR_FIELD1(index), val));

	val = filter->udf_field2;
	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_I_EPR_FIELD2(index), val));

	val = 0;
	val |= (filter->mask & 0x3f) << I_EPR_MASK_OFFSET;

	if ((filter->type & 0x3) == SECY_FILTER_IP_PACKET)
		val_type = SECY_EPR_IP_PACKET;
	else if ((filter->type & 0x3) == SECY_FILTER_TCP_PACKET)
		val_type = SECY_EPR_TCP_PACKET;
	else
		val_type = SECY_EPR_ANY_PACKET;

	val |= val_type << I_EPR_TYPE_OFFSET;

	val_offset = filter->offset & 0x3f;
	val |= val_offset << I_EPR_OFFSET_OFFSET;

	val |= (filter->operator & 0x1) << I_EPR_OPERATOR_OFFSET;

	SHR_RET_ON_ERR(reg_access_write(port, PHY_MMD3,
		       MACSEC_I_EPR_MSK_TYPE_OP(index), val));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_udf_filt_get(struct macsec_port *port, u32 index,
				       struct secy_udf_filt_t *filter)
{
	u16 val = 0;

	memset(filter, 0, sizeof(*filter));

	if (index > SECY_FILTER_MAX_INDEX)
		return ERROR_PARAM;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_I_EPR_FIELD0(index), &val));
	filter->udf_field0 = val;

	val = 0;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_I_EPR_FIELD1(index), &val));
	filter->udf_field1 = val;

	val = 0;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_I_EPR_FIELD2(index), &val));
	filter->udf_field2 = val;

	val = 0;
	SHR_RET_ON_ERR(reg_access_read(port, PHY_MMD3,
		       MACSEC_I_EPR_MSK_TYPE_OP(index), &val));

	filter->mask = val & 0x3f;

	if (((val >> I_EPR_TYPE_OFFSET) & 0x3) == SECY_EPR_IP_PACKET)
		filter->type = SECY_FILTER_IP_PACKET;
	else if (((val >> I_EPR_TYPE_OFFSET) & 0x3) == SECY_EPR_TCP_PACKET)
		filter->type = SECY_FILTER_TCP_PACKET;
	else
		filter->type = SECY_FILTER_ANY_PACKET;

	filter->offset = (val >> I_EPR_OFFSET_OFFSET) & 0x3f;

	if ((val >> I_EPR_OPERATOR_OFFSET) & 0x1)
		filter->operator = SECY_FILTER_OPERATOR_LESS;
	else
		filter->operator = SECY_FILTER_OPERATOR_EQUAL;

	return ERROR_OK;
}

g_error_t qca808x_secy_tx_ctl_filt_en_set(struct macsec_port *port, bool enable)
{
	u16 val1 = 0;

	SHR_RET_ON_ERR(REG_FIELD_READ(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_CEPR_EN, &val1));

	if (enable)
		val1 = FALSE;

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UEPR_EN, enable));

	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_CEPR_EN, val1));

	return ERROR_OK;
}

g_error_t qca808x_secy_rx_ctl_filt_en_set(struct macsec_port *port, bool enable)
{
	SHR_RET_ON_ERR(REG_FIELD_WRITE(port, PHY_MMD3,
		       MACSEC_SYS_CONFIG, SYS_UIER_EN, enable));
	return ERROR_OK;
}


