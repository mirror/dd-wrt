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


#include "nss_macsec_types.h"
#include "nss_macsec.h"
#include "nss_macsec_secy.h"
#include "nss_macsec_utility.h"
#include "nss_macsec_secy_tx.h"


u32 nss_macsec_secy_tx_ctl_filt_get(u32 secy_id, u32 filt_id,
				    fal_tx_ctl_filt_t *pfilt)
{
	g_error_t rv;
	struct secy_ctl_filt_t secy_ctl_filt;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(filt_id < FAL_TX_CTL_FILT_HW_NUM) && (pfilt != NULL));

	memset(&secy_ctl_filt, 0, sizeof(secy_ctl_filt));

	rv = qca_macsec_tx_ctl_filt_get(FAL_SECY_ID_TO_PORT(secy_id), filt_id,
					&secy_ctl_filt);
	pfilt->bypass = secy_ctl_filt.bypass;
	pfilt->match_type = (fal_eg_ctl_match_type_e)secy_ctl_filt.match_type;
	pfilt->match_mask = secy_ctl_filt.match_mask;
	pfilt->ether_type_da_range = secy_ctl_filt.ether_type_da_range;
	memcpy(pfilt->sa_da_addr, secy_ctl_filt.sa_da_addr,
		sizeof(secy_ctl_filt.sa_da_addr));

	return rv;
}

u32 nss_macsec_secy_tx_ctl_filt_set(u32 secy_id, u32 filt_id,
				    fal_tx_ctl_filt_t *pfilt)
{
	g_error_t rv;
	struct secy_ctl_filt_t secy_ctl_filt;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(filt_id < FAL_TX_CTL_FILT_NUM) && (pfilt != NULL));

	memset(&secy_ctl_filt, 0, sizeof(secy_ctl_filt));

	secy_ctl_filt.bypass = pfilt->bypass;
	secy_ctl_filt.match_type =
		(enum secy_ctl_match_type_t)pfilt->match_type;
	secy_ctl_filt.match_mask = pfilt->match_mask;
	secy_ctl_filt.ether_type_da_range = pfilt->ether_type_da_range;
	memcpy(secy_ctl_filt.sa_da_addr, pfilt->sa_da_addr,
		sizeof(pfilt->sa_da_addr));
	rv = qca_macsec_tx_ctl_filt_set(FAL_SECY_ID_TO_PORT(secy_id), filt_id,
					&secy_ctl_filt);
	return rv;
}

u32 nss_macsec_secy_tx_ctl_filt_clear(u32 secy_id, u32 filt_id)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM)
			&& (filt_id < FAL_TX_CTL_FILT_NUM));

	rv = qca_macsec_tx_ctl_filt_clear(FAL_SECY_ID_TO_PORT(secy_id),
					  filt_id);
	return rv;
}

u32 nss_macsec_secy_tx_ctl_filt_clear_all(u32 secy_id)
{
	g_error_t rv;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	rv = qca_macsec_tx_ctl_filt_clear_all(FAL_SECY_ID_TO_PORT(secy_id));
	return rv;
}

u32 nss_macsec_secy_tx_class_lut_get(u32 secy_id, u32 index,
				     fal_tx_class_lut_t *pentry)
{
	g_error_t rv;
	struct secy_tx_class_lut_t secy_tx_clss_lut;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(index < FAL_TX_CLASS_LUT_NUM) && (pentry != NULL));

	memset(pentry, 0, sizeof(fal_tx_class_lut_t));
	memset(&secy_tx_clss_lut, 0, sizeof(secy_tx_clss_lut));

	rv = qca_macsec_tx_class_lut_get(FAL_SECY_ID_TO_PORT(secy_id), index,
					&secy_tx_clss_lut);

	pentry->valid = secy_tx_clss_lut.valid;
	memcpy(pentry->da, secy_tx_clss_lut.da, sizeof(secy_tx_clss_lut.da));
	memcpy(pentry->sa, secy_tx_clss_lut.sa, sizeof(secy_tx_clss_lut.sa));
	memcpy(pentry->sci, secy_tx_clss_lut.sci, sizeof(secy_tx_clss_lut.sci));
	pentry->ether_type = secy_tx_clss_lut.ether_type;
	pentry->vlan_id = secy_tx_clss_lut.inner_vlanid;
	pentry->outer_vlanid = secy_tx_clss_lut.outer_vlanid;
	pentry->bc_flag = secy_tx_clss_lut.bc_flag;
	pentry->offset = secy_tx_clss_lut.offset;
	pentry->channel = secy_tx_clss_lut.channel;
	pentry->tci = secy_tx_clss_lut.tci;
	pentry->rule_mask = secy_tx_clss_lut.rule_mask;

	return rv;
}

u32 nss_macsec_secy_tx_class_lut_set(u32 secy_id, u32 index,
				     fal_tx_class_lut_t *pentry)
{
	g_error_t rv;
	struct secy_tx_class_lut_t secy_tx_clss_lut;


	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(index < FAL_TX_CLASS_LUT_NUM) && (pentry != NULL));

	memset(&secy_tx_clss_lut, 0, sizeof(secy_tx_clss_lut));

	secy_tx_clss_lut.valid = pentry->valid;
	memcpy(secy_tx_clss_lut.da, pentry->da, sizeof(secy_tx_clss_lut.da));
	memcpy(secy_tx_clss_lut.sa, pentry->sa, sizeof(secy_tx_clss_lut.sa));
	memcpy(secy_tx_clss_lut.sci, pentry->sci, sizeof(secy_tx_clss_lut.sci));
	secy_tx_clss_lut.ether_type = pentry->ether_type;
	secy_tx_clss_lut.inner_vlanid = pentry->vlan_id;
	secy_tx_clss_lut.outer_vlanid = pentry->outer_vlanid;
	secy_tx_clss_lut.bc_flag = pentry->bc_flag;
	secy_tx_clss_lut.offset = pentry->offset;
	secy_tx_clss_lut.channel = pentry->channel;
	secy_tx_clss_lut.tci = pentry->tci;
	secy_tx_clss_lut.rule_mask = pentry->rule_mask;

	rv = qca_macsec_tx_class_lut_set(FAL_SECY_ID_TO_PORT(secy_id), index,
					 &secy_tx_clss_lut);
	return rv;
}

u32 nss_macsec_secy_tx_class_lut_clear(u32 secy_id, u32 index)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(index < FAL_TX_CLASS_LUT_NUM));

	rv = qca_macsec_tx_class_lut_clear(FAL_SECY_ID_TO_PORT(secy_id), index);
	return rv;
}

u32 nss_macsec_secy_tx_class_lut_clear_all(u32 secy_id)
{
	g_error_t rv;

	rv = qca_macsec_tx_class_lut_clear_all(FAL_SECY_ID_TO_PORT(secy_id));
	return rv;
}

u32 nss_macsec_secy_tx_sc_create(u32 secy_id, u32 channel,
				 u8 *psci, u32 sci_len)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(psci != NULL));

	rv = qca_macsec_tx_sc_create(FAL_SECY_ID_TO_PORT(secy_id), channel,
				     psci, sci_len);
	return rv;
}

u32 nss_macsec_secy_tx_sc_en_get(u32 secy_id, u32 channel, bool *penable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(penable != NULL));

	rv = qca_macsec_tx_sc_en_get(FAL_SECY_ID_TO_PORT(secy_id), channel,
				     penable);
	return rv;
}

u32 nss_macsec_secy_tx_sc_en_set(u32 secy_id, u32 channel, bool enable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)));

	rv = qca_macsec_tx_sc_en_set(FAL_SECY_ID_TO_PORT(secy_id), channel,
				     enable);
	return rv;
}

u32 nss_macsec_secy_tx_sc_del(u32 secy_id, u32 channel)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)));

	rv = qca_macsec_tx_sc_del(FAL_SECY_ID_TO_PORT(secy_id), channel);
	return rv;
}

u32 nss_macsec_secy_tx_sc_del_all(u32 secy_id)
{
	g_error_t rv;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	rv = qca_macsec_tx_sc_del_all(FAL_SECY_ID_TO_PORT(secy_id));
	return rv;
}

u32 nss_macsec_secy_tx_sc_an_get(u32 secy_id, u32 channel, u32 *pan)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(pan != NULL));

	rv = qca_macsec_tx_sc_an_get(FAL_SECY_ID_TO_PORT(secy_id),
				     channel, pan);
	return rv;
}

u32 nss_macsec_secy_tx_sc_an_set(u32 secy_id, u32 channel, u32 an)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)));

	rv = qca_macsec_tx_sc_an_set(FAL_SECY_ID_TO_PORT(secy_id),
				     channel, an);
	return rv;
}

u32 nss_macsec_secy_tx_sc_in_used_get(u32 secy_id, u32 channel,
				      bool *p_in_used)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(p_in_used != NULL));

	rv = qca_macsec_tx_sc_in_used_get(FAL_SECY_ID_TO_PORT(secy_id),
					  channel, p_in_used);
	return rv;
}

u32 nss_macsec_secy_tx_sc_tci_7_2_get(u32 secy_id, u32 channel, u8 *ptci)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(ptci != NULL));

	rv = qca_macsec_tx_sc_tci_7_2_get(FAL_SECY_ID_TO_PORT(secy_id),
					  channel, ptci);
	return rv;
}

static int nss_macsec_tci_check(u8 tci)
{
	if((tci & 0x1c) != 0x8 &&
		(tci & 0x1c) != 0x10 &&
		(tci & 0x1c) != 0x14)
		return ERROR_PARAM;
	return ERROR_OK;
}

u32 nss_macsec_secy_tx_sc_tci_7_2_set(u32 secy_id, u32 channel, u8 tci)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(nss_macsec_tci_check(tci) == ERROR_OK));

	rv = qca_macsec_tx_sc_tci_7_2_set(FAL_SECY_ID_TO_PORT(secy_id),
					  channel, tci);
	return rv;
}

u32 nss_macsec_secy_tx_sc_confidentiality_offset_get(u32 secy_id, u32 channel,
						     u32 *poffset)
{
	g_error_t rv;
	enum secy_cofidentiality_offset_t secy_offset;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(poffset != NULL));

	rv = qca_macsec_tx_sc_confidentiality_offset_get(FAL_SECY_ID_TO_PORT(secy_id),
							 channel, &secy_offset);

	if (secy_offset == SECY_CONFIDENTIALITY_OFFSET_00)
		*poffset = NSS_SECY_CONFIDENTIALITY_OFFSET_00;
	else if (secy_offset == SECY_CONFIDENTIALITY_OFFSET_30)
		*poffset = NSS_SECY_CONFIDENTIALITY_OFFSET_30;
	else if (secy_offset == SECY_CONFIDENTIALITY_OFFSET_50)
		*poffset = NSS_SECY_CONFIDENTIALITY_OFFSET_50;
	else
		rv = ERROR_NOT_SUPPORT;
	return rv;
}

u32 nss_macsec_secy_tx_sc_confidentiality_offset_set(u32 secy_id, u32 channel,
						     u32 offset)
{
	g_error_t rv;
	enum secy_cofidentiality_offset_t secy_offset;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)));

	if (offset == NSS_SECY_CONFIDENTIALITY_OFFSET_00)
		secy_offset = SECY_CONFIDENTIALITY_OFFSET_00;
	else if (offset == NSS_SECY_CONFIDENTIALITY_OFFSET_30)
		secy_offset = SECY_CONFIDENTIALITY_OFFSET_30;
	else if (offset == NSS_SECY_CONFIDENTIALITY_OFFSET_50)
		secy_offset = SECY_CONFIDENTIALITY_OFFSET_50;
	else
		return ERROR_NOT_SUPPORT;

	rv = qca_macsec_tx_sc_confidentiality_offset_set(FAL_SECY_ID_TO_PORT(secy_id),
							 channel, secy_offset);
	return rv;
}

u32 nss_macsec_secy_tx_sc_protect_get(u32 secy_id, u32 channel, bool *penable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(penable != NULL));

	rv = qca_macsec_tx_protect_frames_en_get(FAL_SECY_ID_TO_PORT(secy_id),
						 penable);
	return rv;
}

u32 nss_macsec_secy_tx_sc_protect_set(u32 secy_id, u32 channel, bool enable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)));

	rv = qca_macsec_tx_protect_frames_en_set(FAL_SECY_ID_TO_PORT(secy_id),
						 enable);
	return rv;
}

u32 nss_macsec_secy_tx_sc_sci_get(u32 secy_id, u32 channel,
				  u8 *psci, u32 sci_len)
{
	g_error_t rv;
	struct secy_tx_class_lut_t secy_tx_clss_lut;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(psci != NULL));

	memset(&secy_tx_clss_lut, 0, sizeof(secy_tx_clss_lut));

	rv = qca_macsec_tx_class_lut_get(FAL_SECY_ID_TO_PORT(secy_id), channel,
					 &secy_tx_clss_lut);
	memcpy(psci, secy_tx_clss_lut.sci, sizeof(secy_tx_clss_lut.sci));
	return rv;
}

u32 nss_macsec_secy_tx_sa_create(u32 secy_id, u32 channel, u32 an)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)));

	rv = qca_macsec_tx_sa_create(FAL_SECY_ID_TO_PORT(secy_id), channel, an);
	return rv;
}

u32 nss_macsec_secy_tx_sa_en_get(u32 secy_id, u32 channel, u32 an,
				 bool *penable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(penable != NULL));

	rv = qca_macsec_tx_sa_en_get(FAL_SECY_ID_TO_PORT(secy_id),
				     channel, an, penable);
	return rv;
}

u32 nss_macsec_secy_tx_sa_en_set(u32 secy_id, u32 channel, u32 an, bool enable)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)));

	rv = qca_macsec_tx_sa_en_set(FAL_SECY_ID_TO_PORT(secy_id),
				     channel, an, enable);
	return rv;
}

u32 nss_macsec_secy_tx_sa_del(u32 secy_id, u32 channel, u32 an)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)));

	rv = qca_macsec_tx_sa_del(FAL_SECY_ID_TO_PORT(secy_id), channel, an);
	return rv;
}

u32 nss_macsec_secy_tx_sa_del_all(u32 secy_id)
{
	g_error_t rv;

	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	rv = qca_macsec_tx_sa_del_all(FAL_SECY_ID_TO_PORT(secy_id));
	return rv;
}

u32 nss_macsec_secy_tx_sa_next_pn_get(u32 secy_id, u32 channel,
				      u32 an, u32 *p_next_pn)
{
	g_error_t rv;
	u64 next_pn = 0;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(p_next_pn != NULL));

	rv = qca_macsec_tx_sa_npn_get(FAL_SECY_ID_TO_PORT(secy_id),
				      channel, an, &next_pn);
	*p_next_pn = next_pn & 0xFFFFFFFF;
	return rv;
}

u32 nss_macsec_secy_tx_sa_next_pn_set(u32 secy_id, u32 channel,
				      u32 an, u32 next_pn)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)));

	rv = qca_macsec_tx_sa_npn_set(FAL_SECY_ID_TO_PORT(secy_id),
				      channel, an, next_pn);
	return rv;
}

u32 nss_macsec_secy_tx_sa_in_used_get(u32 secy_id, u32 channel,
				      u32 an, bool *p_in_used)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(p_in_used != NULL));

	rv = qca_macsec_tx_sa_in_used_get(FAL_SECY_ID_TO_PORT(secy_id), channel,
					  an, p_in_used);
	return rv;
}

u32 nss_macsec_secy_tx_sak_get(u32 secy_id, u32 channel,
			       u32 an, fal_tx_sak_t *pentry)
{
	g_error_t rv;
	struct secy_sak_t secy_key;

	memset(&secy_key, 0, sizeof(secy_key));

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(pentry != NULL));

	rv = qca_macsec_tx_sak_get(FAL_SECY_ID_TO_PORT(secy_id), channel,
				   an, &secy_key);
	*pentry = *(fal_tx_sak_t *)&secy_key;
	return rv;
}

u32 nss_macsec_secy_tx_sak_set(u32 secy_id, u32 channel,
			       u32 an, fal_tx_sak_t *pentry)
{
	g_error_t rv;
	struct secy_sak_t *secy_key = NULL;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(pentry != NULL));

	secy_key = (struct secy_sak_t *)pentry;
	rv = qca_macsec_tx_sak_set(FAL_SECY_ID_TO_PORT(secy_id), channel,
				   an, secy_key);
	return rv;
}

u32 nss_macsec_secy_tx_sa_next_xpn_get(u32 secy_id, u32 channel,
					u32 an, u32 *p_next_xpn)
{
	return ERROR_NOT_SUPPORT;
}

u32 nss_macsec_secy_tx_sa_next_xpn_set(u32 secy_id, u32 channel,
					u32 an, u32 next_xpn)
{
	return ERROR_NOT_SUPPORT;
}

u32 nss_macsec_secy_tx_sc_ssci_get(u32 secy_id, u32 channel, u32 *pssci)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(pssci != NULL));

	rv = qca_macsec_tx_sc_ssci_get(FAL_SECY_ID_TO_PORT(secy_id),
					channel, pssci);
	return rv;
}

u32 nss_macsec_secy_tx_sc_ssci_set(u32 secy_id, u32 channel, u32 ssci)
{
	g_error_t rv;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)));

	rv = qca_macsec_tx_sc_ssci_set(FAL_SECY_ID_TO_PORT(secy_id),
					channel, ssci);
	return rv;
}

u32 nss_macsec_secy_tx_sa_ki_get(u32 secy_id, u32 channel,
	u32 an, struct fal_tx_sa_ki_t *ki)
{
	g_error_t rv;
	struct secy_sa_ki_t secy_sa_ki;

	memset(&secy_sa_ki, 0, sizeof(secy_sa_ki));

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(ki != NULL));

	rv = qca_macsec_tx_sa_ki_get(FAL_SECY_ID_TO_PORT(secy_id), channel,
				     an, &secy_sa_ki);
	*ki = *(struct fal_tx_sa_ki_t *)&secy_sa_ki;
	return rv;
}

u32 nss_macsec_secy_tx_sa_ki_set(u32 secy_id, u32 channel,
	u32 an, struct fal_tx_sa_ki_t *ki)
{
	g_error_t rv;
	struct secy_sa_ki_t *secy_sa_ki = NULL;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(ki != NULL));

	secy_sa_ki = (struct secy_sa_ki_t *)ki;
	rv = qca_macsec_tx_sa_ki_set(FAL_SECY_ID_TO_PORT(secy_id),
				     channel, an, secy_sa_ki);
	return rv;
}

u32 nss_macsec_secy_tx_udf_filt_set(u32 secy_id, u32 filt_id,
				    struct fal_tx_udf_filt_t *pfilt)
{
	g_error_t rv;
	struct secy_udf_filt_t filter;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(filt_id < FAL_TX_UDF_FILT_MAX_ID) && (pfilt != NULL));

	memset(&filter, 0, sizeof(filter));
	filter.udf_field0 = pfilt->udf_field0;
	filter.udf_field1 = pfilt->udf_field1;
	filter.udf_field2 = pfilt->udf_field2;
	filter.type = (enum secy_udf_filt_type_t)pfilt->type;
	filter.operator =
		(enum secy_udf_filt_op_t)pfilt->operator;
	filter.offset = pfilt->offset;
	filter.mask = pfilt->mask;
	rv = qca_macsec_tx_udf_filt_set(FAL_SECY_ID_TO_PORT(secy_id), filt_id,
					&filter);
	return rv;
}

u32 nss_macsec_secy_tx_udf_filt_get(u32 secy_id, u32 filt_id,
				    struct fal_tx_udf_filt_t *pfilt)
{
	g_error_t rv;
	struct secy_udf_filt_t filter;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(filt_id < FAL_TX_UDF_FILT_MAX_ID) && (pfilt != NULL));

	memset(&filter, 0, sizeof(filter));

	rv = qca_macsec_tx_udf_filt_get(FAL_SECY_ID_TO_PORT(secy_id), filt_id,
					&filter);
	pfilt->udf_field0 = filter.udf_field0;
	pfilt->udf_field1 = filter.udf_field1;
	pfilt->udf_field2 = filter.udf_field2;
	pfilt->type = (enum fal_tx_udf_filt_type_e)filter.type;
	pfilt->operator = (enum fal_tx_udf_filt_op_e)filter.operator;
	pfilt->offset = filter.offset;
	pfilt->mask = filter.mask;
	return rv;
}

u32 nss_macsec_secy_tx_udf_ufilt_cfg_set(u32 secy_id,
					 struct fal_tx_udf_filt_cfg_t *cfg)
{
	g_error_t rv;
	struct secy_udf_filt_cfg_t secy_cfg;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(cfg != NULL));

	memset(&secy_cfg, 0, sizeof(secy_cfg));
	secy_cfg.enable = cfg->enable;
	secy_cfg.priority = cfg->priority;
	secy_cfg.inverse = cfg->inverse;
	secy_cfg.pattern0 = (enum secy_udf_filt_cfg_pattern_t)cfg->pattern0;
	secy_cfg.pattern1 = (enum secy_udf_filt_cfg_pattern_t)cfg->pattern1;
	secy_cfg.pattern2 = (enum secy_udf_filt_cfg_pattern_t)cfg->pattern2;
	rv = qca_macsec_tx_udf_ufilt_cfg_set(FAL_SECY_ID_TO_PORT(secy_id),
						&secy_cfg);
	return rv;
}

u32 nss_macsec_secy_tx_udf_ufilt_cfg_get(u32 secy_id,
					 struct fal_tx_udf_filt_cfg_t *cfg)
{
	g_error_t rv;
	struct secy_udf_filt_cfg_t secy_cfg;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(cfg != NULL));

	memset(&secy_cfg, 0, sizeof(secy_cfg));
	rv = qca_macsec_tx_udf_ufilt_cfg_get(FAL_SECY_ID_TO_PORT(secy_id),
						&secy_cfg);
	cfg->enable = secy_cfg.enable;
	cfg->priority = secy_cfg.priority;
	cfg->inverse = secy_cfg.inverse;
	cfg->pattern0 = (enum fal_tx_udf_filt_cfg_pattern_e)secy_cfg.pattern0;
	cfg->pattern1 = (enum fal_tx_udf_filt_cfg_pattern_e)secy_cfg.pattern1;
	cfg->pattern2 = (enum fal_tx_udf_filt_cfg_pattern_e)secy_cfg.pattern2;

	return rv;
}

u32 nss_macsec_secy_tx_udf_cfilt_cfg_set(u32 secy_id,
					 struct fal_tx_udf_filt_cfg_t *cfg)
{
	g_error_t rv;
	struct secy_udf_filt_cfg_t secy_cfg;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(cfg != NULL));

	memset(&secy_cfg, 0, sizeof(secy_cfg));
	secy_cfg.enable = cfg->enable;
	secy_cfg.priority = cfg->priority;
	secy_cfg.inverse = cfg->inverse;
	secy_cfg.pattern0 = (enum secy_udf_filt_cfg_pattern_t)cfg->pattern0;
	secy_cfg.pattern1 = (enum secy_udf_filt_cfg_pattern_t)cfg->pattern1;
	secy_cfg.pattern2 = (enum secy_udf_filt_cfg_pattern_t)cfg->pattern2;
	rv = qca_macsec_tx_udf_cfilt_cfg_set(FAL_SECY_ID_TO_PORT(secy_id),
						&secy_cfg);
	return rv;
}

u32 nss_macsec_secy_tx_udf_cfilt_cfg_get(u32 secy_id,
					 struct fal_tx_udf_filt_cfg_t *cfg)
{
	g_error_t rv;
	struct secy_udf_filt_cfg_t secy_cfg;

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(cfg != NULL));

	memset(&secy_cfg, 0, sizeof(secy_cfg));
	rv = qca_macsec_tx_udf_cfilt_cfg_get(FAL_SECY_ID_TO_PORT(secy_id),
						&secy_cfg);
	cfg->enable = secy_cfg.enable;
	cfg->priority = secy_cfg.priority;
	cfg->inverse = secy_cfg.inverse;
	cfg->pattern0 = (enum fal_tx_udf_filt_cfg_pattern_e)secy_cfg.pattern0;
	cfg->pattern1 = (enum fal_tx_udf_filt_cfg_pattern_e)secy_cfg.pattern1;
	cfg->pattern2 = (enum fal_tx_udf_filt_cfg_pattern_e)secy_cfg.pattern2;
	return rv;
}

