/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "hppe_portvlan_reg.h"
#include "hppe_portvlan.h"
#include "hppe_portctrl_reg.h"
#include "hppe_portctrl.h"
#include "hppe_policer_reg.h"
#include "hppe_policer.h"
#include "hppe_fdb_reg.h"
#include "hppe_fdb.h"
#include "adpt.h"
#if defined(APPE)
#include "appe_portvlan_reg.h"
#include "appe_portvlan.h"
#include "adpt_appe_portvlan.h"
#endif

a_uint32_t
_get_port_vlan_trans_adv_rule_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t *rule, fal_vlan_trans_adv_action_t * action)
{
	union xlt_rule_tbl_u in_vlan_xlt_rule;
	union xlt_action_tbl_u in_vlan_xlt_action;
	union eg_vlan_xlt_rule_u eg_vlan_xlt_rule;
	union eg_vlan_xlt_action_u eg_vlan_xlt_action;
	a_uint32_t rtn = SW_OK;

	if (direction == FAL_PORT_VLAN_INGRESS) {
		/*rule part*/
		rtn = hppe_xlt_rule_tbl_get(dev_id, index, &in_vlan_xlt_rule);
		SW_RTN_ON_ERROR(rtn);

		if (!in_vlan_xlt_rule.bf.valid) {
			memset(&in_vlan_xlt_rule, 0, sizeof(in_vlan_xlt_rule));
		}

#if defined(APPE)
		rule->vni_resv_enable = in_vlan_xlt_rule.bf.vni_resv_incl;
		rule->vni_resv_type = in_vlan_xlt_rule.bf.vni_resv_type;
		rule->vni_resv = in_vlan_xlt_rule.bf.vni_resv_0 |
			in_vlan_xlt_rule.bf.vni_resv_1 << 19;
		rule->port_bitmap = FAL_PORT_ID(adpt_port_type_convert(A_FALSE,
					in_vlan_xlt_rule.bf.port_type),
				in_vlan_xlt_rule.bf.port_bitmap);
#else
		rule->port_bitmap = in_vlan_xlt_rule.bf.port_bitmap;
#endif
		rule->s_tagged = in_vlan_xlt_rule.bf.skey_fmt;
		rule->s_vid_enable = in_vlan_xlt_rule.bf.skey_vid_incl;
		rule->s_vid = in_vlan_xlt_rule.bf.skey_vid;
		rule->s_pcp_enable = in_vlan_xlt_rule.bf.skey_pcp_incl;
		rule->s_pcp = in_vlan_xlt_rule.bf.skey_pcp;
		rule->s_dei_enable = in_vlan_xlt_rule.bf.skey_dei_incl;
		rule->s_dei = in_vlan_xlt_rule.bf.skey_dei;

		rule->c_tagged = in_vlan_xlt_rule.bf.ckey_fmt_0 |
			(in_vlan_xlt_rule.bf.ckey_fmt_1 << 1);
		rule->c_vid_enable = in_vlan_xlt_rule.bf.ckey_vid_incl;
		rule->c_vid = in_vlan_xlt_rule.bf.ckey_vid;
		rule->c_pcp_enable = in_vlan_xlt_rule.bf.ckey_pcp_incl;
		rule->c_pcp = in_vlan_xlt_rule.bf.ckey_pcp;
		rule->c_dei_enable = in_vlan_xlt_rule.bf.ckey_dei_incl;
		rule->c_dei = in_vlan_xlt_rule.bf.ckey_dei;

		rule->frmtype_enable = in_vlan_xlt_rule.bf.frm_type_incl;
		rule->frmtype = in_vlan_xlt_rule.bf.frm_type;
		rule->protocol_enable = in_vlan_xlt_rule.bf.prot_incl;
		rule->protocol = ((in_vlan_xlt_rule.bf.prot_value_1 << 7) |
				(in_vlan_xlt_rule.bf.prot_value_0));

		/*action part*/
		rtn = hppe_xlt_action_tbl_get(dev_id, index, &in_vlan_xlt_action);
		SW_RTN_ON_ERROR(rtn);

		if (!in_vlan_xlt_rule.bf.valid) {
			memset(&in_vlan_xlt_action, 0, sizeof(struct xlt_action_tbl));
		}

		action->swap_svid_cvid = in_vlan_xlt_action.bf.vid_swap_cmd;
		action->svid_xlt_cmd = in_vlan_xlt_action.bf.xlt_svid_cmd;
		action->svid_xlt = in_vlan_xlt_action.bf.xlt_svid;
		action->cvid_xlt_cmd = in_vlan_xlt_action.bf.xlt_cvid_cmd;
		action->cvid_xlt = in_vlan_xlt_action.bf.xlt_cvid;

		action->swap_spcp_cpcp = in_vlan_xlt_action.bf.pcp_swap_cmd;
		action->spcp_xlt_enable = in_vlan_xlt_action.bf.xlt_spcp_cmd;
		action->spcp_xlt = (in_vlan_xlt_action.bf.xlt_spcp_0 |
				(in_vlan_xlt_action.bf.xlt_spcp_1 << 1));
		action->cpcp_xlt_enable = in_vlan_xlt_action.bf.xlt_cpcp_cmd;
		action->cpcp_xlt = in_vlan_xlt_action.bf.xlt_cpcp;

		action->swap_sdei_cdei = in_vlan_xlt_action.bf.dei_swap_cmd;
		action->sdei_xlt_enable = in_vlan_xlt_action.bf.xlt_sdei_cmd;
		action->sdei_xlt = in_vlan_xlt_action.bf.xlt_sdei;
		action->cdei_xlt_enable = in_vlan_xlt_action.bf.xlt_cdei_cmd;
		action->cdei_xlt = in_vlan_xlt_action.bf.xlt_cdei;

		action->counter_enable = in_vlan_xlt_action.bf.counter_en;
#if defined(APPE)
		action->src_info_enable = in_vlan_xlt_action.bf.src_info_valid;
		action->src_info_type = in_vlan_xlt_action.bf.src_info_type;
		action->src_info = in_vlan_xlt_action.bf.src_info;
		action->counter_id = in_vlan_xlt_action.bf.counter_id_0 |\
				     in_vlan_xlt_action.bf.counter_id_1 << 3;
#else
		action->counter_id = in_vlan_xlt_action.bf.counter_id;
#endif
		action->vsi_xlt_enable = in_vlan_xlt_action.bf.vsi_cmd;
		action->vsi_xlt = in_vlan_xlt_action.bf.vsi;

		return in_vlan_xlt_rule.bf.valid;
	} else {
		/*rule part*/
		rtn = hppe_eg_vlan_xlt_rule_get(dev_id, index, &eg_vlan_xlt_rule);
		SW_RTN_ON_ERROR(rtn);

		if (!eg_vlan_xlt_rule.bf.valid) {
			memset(&eg_vlan_xlt_rule, 0, sizeof(struct eg_vlan_xlt_rule));
		}

		rule->s_tagged = eg_vlan_xlt_rule.bf.skey_fmt;
		rule->s_vid_enable = eg_vlan_xlt_rule.bf.skey_vid_incl;
#if defined(APPE)
		rule->s_vid = eg_vlan_xlt_rule.bf.skey_vid_0 | \
			      eg_vlan_xlt_rule.bf.skey_vid_1 << 11;
		rule->port_bitmap = FAL_PORT_ID(adpt_port_type_convert(A_FALSE,
					eg_vlan_xlt_rule.bf.port_type),
				eg_vlan_xlt_rule.bf.port_bitmap);
#else
		rule->s_vid = eg_vlan_xlt_rule.bf.skey_vid;
		rule->port_bitmap = eg_vlan_xlt_rule.bf.port_bitmap;
#endif
		rule->s_pcp_enable = eg_vlan_xlt_rule.bf.skey_pcp_incl;
		rule->s_pcp = eg_vlan_xlt_rule.bf.skey_pcp;
		rule->s_dei_enable = eg_vlan_xlt_rule.bf.skey_dei_incl;
		rule->s_dei = eg_vlan_xlt_rule.bf.skey_dei;

		rule->c_tagged = eg_vlan_xlt_rule.bf.ckey_fmt;
		rule->c_vid_enable = eg_vlan_xlt_rule.bf.ckey_vid_incl;
		rule->c_vid = eg_vlan_xlt_rule.bf.ckey_vid;
		rule->c_pcp_enable = eg_vlan_xlt_rule.bf.ckey_pcp_incl;
		rule->c_pcp = eg_vlan_xlt_rule.bf.ckey_pcp;
		rule->c_dei_enable = eg_vlan_xlt_rule.bf.ckey_dei_incl;
		rule->c_dei = eg_vlan_xlt_rule.bf.ckey_dei;

		rule->vsi_valid = eg_vlan_xlt_rule.bf.vsi_valid;
		rule->vsi_enable = eg_vlan_xlt_rule.bf.vsi_incl;
		rule->vsi = eg_vlan_xlt_rule.bf.vsi;

		/*action part*/
		rtn = hppe_eg_vlan_xlt_action_get(dev_id, index, &eg_vlan_xlt_action);
		SW_RTN_ON_ERROR(rtn);

		if (!eg_vlan_xlt_rule.bf.valid) {
			memset(&eg_vlan_xlt_action, 0, sizeof(struct eg_vlan_xlt_action));
		}

		action->swap_svid_cvid = eg_vlan_xlt_action.bf.vid_swap_cmd;
		action->svid_xlt_cmd = eg_vlan_xlt_action.bf.xlt_svid_cmd;
		action->svid_xlt = eg_vlan_xlt_action.bf.xlt_svid;
		action->cvid_xlt_cmd = eg_vlan_xlt_action.bf.xlt_cvid_cmd;
		action->cvid_xlt = eg_vlan_xlt_action.bf.xlt_cvid;

		action->swap_spcp_cpcp = eg_vlan_xlt_action.bf.pcp_swap_cmd;
		action->spcp_xlt_enable = eg_vlan_xlt_action.bf.xlt_spcp_cmd;
		action->spcp_xlt = (eg_vlan_xlt_action.bf.xlt_spcp_0 |
				(eg_vlan_xlt_action.bf.xlt_spcp_1 << 1));
		action->cpcp_xlt_enable = eg_vlan_xlt_action.bf.xlt_cpcp_cmd;
		action->cpcp_xlt = eg_vlan_xlt_action.bf.xlt_cpcp;

		action->swap_sdei_cdei = eg_vlan_xlt_action.bf.dei_swap_cmd;
		action->sdei_xlt_enable = eg_vlan_xlt_action.bf.xlt_sdei_cmd;
		action->sdei_xlt = eg_vlan_xlt_action.bf.xlt_sdei;
		action->cdei_xlt_enable = eg_vlan_xlt_action.bf.xlt_cdei_cmd;
		action->cdei_xlt = eg_vlan_xlt_action.bf.xlt_cdei;

		action->counter_enable = eg_vlan_xlt_action.bf.counter_en;
		action->counter_id = eg_vlan_xlt_action.bf.counter_id;

#if defined(APPE)
		action->vni_resv_enable = eg_vlan_xlt_action.bf.vni_resv_en;
		action->vni_resv = eg_vlan_xlt_action.bf.vni_resv_0 |\
				   eg_vlan_xlt_action.bf.vni_resv_1 << 12;
#endif
		return eg_vlan_xlt_rule.bf.valid;
	}
}

a_uint32_t
_check_if_rule_equal(fal_port_vlan_direction_t direction, fal_vlan_trans_adv_rule_t * rule1,
		fal_vlan_trans_adv_rule_t * rule2)
{
	if (!(rule1->s_tagged == rule2->s_tagged &&
		rule1->s_vid_enable == rule2->s_vid_enable && rule1->s_vid == rule2->s_vid &&
		rule1->s_pcp_enable == rule2->s_pcp_enable && rule1->s_pcp == rule2->s_pcp &&
		rule1->s_dei_enable == rule2->s_dei_enable && rule1->s_dei == rule2->s_dei &&
		rule1->c_tagged == rule2->c_tagged &&
#if defined(APPE)
		FAL_PORT_ID_TYPE(rule1->port_bitmap) == FAL_PORT_ID_TYPE(rule2->port_bitmap) &&
#endif
		rule1->c_vid_enable == rule2->c_vid_enable && rule1->c_vid == rule2->c_vid &&
		rule1->c_pcp_enable == rule2->c_pcp_enable && rule1->c_pcp == rule2->c_pcp &&
		rule1->c_dei_enable == rule2->c_dei_enable && rule1->c_dei == rule2->c_dei))
		return 1;

	if (direction == FAL_PORT_VLAN_INGRESS)
	{
		if (!(rule1->frmtype_enable == rule2->frmtype_enable &&
					rule1->frmtype == rule2->frmtype &&
#if defined(APPE)
					rule1->vni_resv_enable == rule2->vni_resv_enable &&
					rule1->vni_resv_type == rule2->vni_resv_type &&
					rule1->vni_resv == rule2->vni_resv &&
#endif
					rule1->protocol_enable == rule2->protocol_enable &&
					rule1->protocol == rule2->protocol))
			return 1;
	} else {
		if (!(rule1->vsi_valid == rule2->vsi_valid &&
					rule1->vsi_enable == rule2->vsi_enable &&
					rule1->vsi == rule2->vsi))
			return 1;
	}

	return 0;
}

a_uint32_t
_check_if_action_equal(fal_port_vlan_direction_t direction, fal_vlan_trans_adv_action_t * action1,
		fal_vlan_trans_adv_action_t * action2)
{
	if (!(action1->swap_svid_cvid == action2->swap_svid_cvid &&
		action1->svid_xlt_cmd == action2->svid_xlt_cmd &&
		action1->svid_xlt == action2->svid_xlt &&
		action1->cvid_xlt_cmd == action2->cvid_xlt_cmd &&
		action1->cvid_xlt == action2->cvid_xlt &&
		action1->swap_sdei_cdei == action2->swap_sdei_cdei &&
		action1->sdei_xlt_enable == action2->sdei_xlt_enable &&
		action1->sdei_xlt == action2->sdei_xlt &&
		action1->cdei_xlt_enable == action2->cdei_xlt_enable &&
		action1->cdei_xlt == action2->cdei_xlt &&
		action1->swap_spcp_cpcp == action2->swap_spcp_cpcp &&
		action1->spcp_xlt_enable == action2->spcp_xlt_enable &&
		action1->spcp_xlt == action2->spcp_xlt &&
		action1->cpcp_xlt_enable == action2->cpcp_xlt_enable &&
		action1->cpcp_xlt == action2->cpcp_xlt &&
		action1->counter_enable == action2->counter_enable &&
		action1->counter_id == action2->counter_id))
		return 1;

	if (direction == FAL_PORT_VLAN_INGRESS) {
		if (!(action1->vsi_xlt_enable == action2->vsi_xlt_enable &&
#if defined(APPE)
					action1->src_info_enable == action2->src_info_enable &&
					action1->src_info_type == action2->src_info_type &&
					action1->src_info == action2->src_info &&
#endif
					action1->vsi_xlt == action2->vsi_xlt))
			return 1;
	}
#if defined(APPE)
	else {
		if (!(action1->vni_resv_enable == action2->vni_resv_enable &&
					action1->vni_resv == action2->vni_resv))
			return 1;
	}
#endif

	return 0;
}

sw_error_t
_insert_vlan_trans_adv_rule_action(a_uint32_t dev_id, a_uint32_t index,
		fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action)
{
	sw_error_t rtn = SW_OK;
	union xlt_rule_tbl_u in_vlan_xlt_rule;
	union eg_vlan_xlt_rule_u eg_vlan_xlt_rule;
	union xlt_action_tbl_u in_vlan_xlt_action;
	union eg_vlan_xlt_action_u eg_vlan_xlt_action;

	if (direction == FAL_PORT_VLAN_INGRESS)
	{
#if defined(APPE)
		in_vlan_xlt_rule.bf.port_bitmap = FAL_PORT_ID_VALUE(rule->port_bitmap);
		in_vlan_xlt_rule.bf.port_type = adpt_port_type_convert(A_TRUE,
				FAL_PORT_ID_TYPE(rule->port_bitmap));
		in_vlan_xlt_rule.bf.vni_resv_incl = rule->vni_resv_enable;
		in_vlan_xlt_rule.bf.vni_resv_type = rule->vni_resv_type;
		in_vlan_xlt_rule.bf.vni_resv_0 = rule->vni_resv;
		in_vlan_xlt_rule.bf.vni_resv_1 = rule->vni_resv >>
			SW_FIELD_OFFSET_IN_WORD(XLT_RULE_TBL_VNI_RESV_OFFSET);
#else
		in_vlan_xlt_rule.bf.port_bitmap = rule->port_bitmap;
#endif
		in_vlan_xlt_rule.bf.valid = A_TRUE;
		in_vlan_xlt_rule.bf.skey_fmt = rule->s_tagged;
		in_vlan_xlt_rule.bf.skey_vid_incl = rule->s_vid_enable;
		in_vlan_xlt_rule.bf.skey_vid = rule->s_vid;
		in_vlan_xlt_rule.bf.skey_pcp_incl = rule->s_pcp_enable;
		in_vlan_xlt_rule.bf.skey_pcp = rule->s_pcp;
		in_vlan_xlt_rule.bf.skey_dei_incl = rule->s_dei_enable;
		in_vlan_xlt_rule.bf.skey_dei = rule->s_dei;

		in_vlan_xlt_rule.bf.ckey_fmt_0 = (rule->c_tagged & 0x1);
		in_vlan_xlt_rule.bf.ckey_fmt_1 = (rule->c_tagged >> 1);
		in_vlan_xlt_rule.bf.ckey_vid_incl = rule->c_vid_enable;
		in_vlan_xlt_rule.bf.ckey_vid = rule->c_vid;
		in_vlan_xlt_rule.bf.ckey_pcp_incl = rule->c_pcp_enable;
		in_vlan_xlt_rule.bf.ckey_pcp = rule->c_pcp;
		in_vlan_xlt_rule.bf.ckey_dei_incl = rule->c_dei_enable;
		in_vlan_xlt_rule.bf.ckey_dei = rule->c_dei;

		in_vlan_xlt_rule.bf.prot_incl = rule->protocol_enable;
		in_vlan_xlt_rule.bf.prot_value_0 = (rule->protocol  & 0x7f);
		in_vlan_xlt_rule.bf.prot_value_1 = (rule->protocol >> 7);
		in_vlan_xlt_rule.bf.frm_type_incl = rule->frmtype_enable;
		in_vlan_xlt_rule.bf.frm_type = rule->frmtype;

		rtn = hppe_xlt_rule_tbl_set(dev_id, index, &in_vlan_xlt_rule);
		SW_RTN_ON_ERROR(rtn);

		/*action part*/
		in_vlan_xlt_action.bf.vid_swap_cmd = action->swap_svid_cvid;
		in_vlan_xlt_action.bf.xlt_svid_cmd = action->svid_xlt_cmd;
		in_vlan_xlt_action.bf.xlt_svid = action->svid_xlt;
		in_vlan_xlt_action.bf.xlt_cvid_cmd = action->cvid_xlt_cmd;
		in_vlan_xlt_action.bf.xlt_cvid = action->cvid_xlt;

		in_vlan_xlt_action.bf.pcp_swap_cmd = action->swap_spcp_cpcp;
		in_vlan_xlt_action.bf.xlt_spcp_cmd = action->spcp_xlt_enable;
		in_vlan_xlt_action.bf.xlt_spcp_0 = (action->spcp_xlt & 0x1);
		in_vlan_xlt_action.bf.xlt_spcp_1 = (action->spcp_xlt >> 1);
		in_vlan_xlt_action.bf.xlt_cpcp_cmd = action->cpcp_xlt_enable;
		in_vlan_xlt_action.bf.xlt_cpcp = action->cpcp_xlt;

		in_vlan_xlt_action.bf.dei_swap_cmd = action->swap_sdei_cdei;
		in_vlan_xlt_action.bf.xlt_cdei_cmd = action->cdei_xlt_enable;
		in_vlan_xlt_action.bf.xlt_cdei = action->cdei_xlt;
		in_vlan_xlt_action.bf.xlt_sdei_cmd = action->sdei_xlt_enable;
		in_vlan_xlt_action.bf.xlt_sdei = action->sdei_xlt;

		in_vlan_xlt_action.bf.counter_en = action->counter_enable;
#if defined(APPE)
		in_vlan_xlt_action.bf.src_info_valid = action->src_info_enable;
		in_vlan_xlt_action.bf.src_info_type = action->src_info_type;
		in_vlan_xlt_action.bf.src_info = action->src_info;
		in_vlan_xlt_action.bf.counter_id_0 = action->counter_id & 0x7;
		in_vlan_xlt_action.bf.counter_id_1 = (action->counter_id >> 3) & 0xf;
#else
		in_vlan_xlt_action.bf.counter_id = action->counter_id;
#endif
		in_vlan_xlt_action.bf.vsi_cmd = action->vsi_xlt_enable;
		in_vlan_xlt_action.bf.vsi = action->vsi_xlt;

		rtn = hppe_xlt_action_tbl_set(dev_id, index, &in_vlan_xlt_action);
		SW_RTN_ON_ERROR(rtn);
	}
	else
	{
		eg_vlan_xlt_rule.bf.valid = A_TRUE;

		eg_vlan_xlt_rule.bf.skey_fmt = rule->s_tagged;
		eg_vlan_xlt_rule.bf.skey_vid_incl = rule->s_vid_enable;
#if defined(APPE)
		eg_vlan_xlt_rule.bf.port_bitmap = FAL_PORT_ID_VALUE(rule->port_bitmap);
		eg_vlan_xlt_rule.bf.port_type = adpt_port_type_convert(A_TRUE,
				FAL_PORT_ID_TYPE(rule->port_bitmap));
		eg_vlan_xlt_rule.bf.skey_vid_0 = rule->s_vid & 0x7ff;
		eg_vlan_xlt_rule.bf.skey_vid_1 = (rule->s_vid >> 11) & 0x1;
#else
		eg_vlan_xlt_rule.bf.port_bitmap = rule->port_bitmap;
		eg_vlan_xlt_rule.bf.skey_vid = rule->s_vid;
#endif
		eg_vlan_xlt_rule.bf.skey_pcp_incl = rule->s_pcp_enable;
		eg_vlan_xlt_rule.bf.skey_pcp = rule->s_pcp;
		eg_vlan_xlt_rule.bf.skey_dei_incl = rule->s_dei_enable;
		eg_vlan_xlt_rule.bf.skey_dei = rule->s_dei;

		eg_vlan_xlt_rule.bf.ckey_fmt = rule->c_tagged;
		eg_vlan_xlt_rule.bf.ckey_vid_incl = rule->c_vid_enable;
		eg_vlan_xlt_rule.bf.ckey_vid = rule->c_vid;
		eg_vlan_xlt_rule.bf.ckey_pcp_incl = rule->c_pcp_enable;
		eg_vlan_xlt_rule.bf.ckey_pcp = rule->c_pcp;
		eg_vlan_xlt_rule.bf.ckey_dei_incl = rule->c_dei_enable;
		eg_vlan_xlt_rule.bf.ckey_dei = rule->c_dei;

		eg_vlan_xlt_rule.bf.vsi_valid = rule->vsi_valid;
		eg_vlan_xlt_rule.bf.vsi_incl = rule->vsi_enable;
		eg_vlan_xlt_rule.bf.vsi = rule->vsi;

		rtn = hppe_eg_vlan_xlt_rule_set(dev_id, index, &eg_vlan_xlt_rule);
		SW_RTN_ON_ERROR(rtn);

		/*action part*/
		eg_vlan_xlt_action.bf.vid_swap_cmd = action->swap_svid_cvid;
		eg_vlan_xlt_action.bf.xlt_svid_cmd = action->svid_xlt_cmd;
		eg_vlan_xlt_action.bf.xlt_svid = action->svid_xlt;
		eg_vlan_xlt_action.bf.xlt_cvid_cmd = action->cvid_xlt_cmd;
		eg_vlan_xlt_action.bf.xlt_cvid = action->cvid_xlt;

		eg_vlan_xlt_action.bf.pcp_swap_cmd = action->swap_spcp_cpcp;
		eg_vlan_xlt_action.bf.xlt_spcp_cmd = action->spcp_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_spcp_0 = (action->spcp_xlt & 0x1);
		eg_vlan_xlt_action.bf.xlt_spcp_1 = (action->spcp_xlt >> 1);
		eg_vlan_xlt_action.bf.xlt_cpcp_cmd = action->cpcp_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_cpcp = action->cpcp_xlt;

		eg_vlan_xlt_action.bf.dei_swap_cmd = action->swap_sdei_cdei;
		eg_vlan_xlt_action.bf.xlt_sdei_cmd = action->sdei_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_sdei = action->sdei_xlt;
		eg_vlan_xlt_action.bf.xlt_cdei_cmd = action->cdei_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_cdei = action->cdei_xlt;

		eg_vlan_xlt_action.bf.counter_en = action->counter_enable;
		eg_vlan_xlt_action.bf.counter_id = action->counter_id;

#if defined(APPE)
		eg_vlan_xlt_action.bf.vni_resv_en = action->vni_resv_enable;
		eg_vlan_xlt_action.bf.vni_resv_0 = action->vni_resv & 0xfff;
		eg_vlan_xlt_action.bf.vni_resv_1 = (action->vni_resv >> 12) & 0xfffff;
#endif
		rtn = hppe_eg_vlan_xlt_action_set(dev_id, index, &eg_vlan_xlt_action);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

static inline sw_error_t
adpt_portvlan_egmode_convert(fal_pt_1q_egmode_t *egmode, a_uint32_t *value, a_bool_t to_hsl)
{
	sw_error_t rv = SW_OK;
	if (to_hsl) {
		switch (*egmode) {
			case FAL_EG_UNTAGGED:
				*value = 0;
				break;
			case FAL_EG_TAGGED:
				*value = 1;
				break;
			case FAL_EG_UNMODIFIED:
				*value = 2;
				break;
			case FAL_EG_UNTOUCHED:
				*value = 3;
				break;
			default:
				SSDK_ERROR("Unsupported fal egmode to convert\n");
				rv = SW_FAIL;
				break;
		}
	} else {
		switch (*value) {
			case 0:
				*egmode = FAL_EG_UNTAGGED;
				break;
			case 1:
				*egmode = FAL_EG_TAGGED;
				break;
			case 2:
				*egmode = FAL_EG_UNMODIFIED;
				break;
			case 3:
				*egmode = FAL_EG_UNTOUCHED;
				break;
			default:
				SSDK_ERROR("Unsupported hsl egmode to convert\n");
				rv = SW_FAIL;
				break;
		}
	}
	return rv;
}

sw_error_t
adpt_hppe_global_qinq_mode_set(a_uint32_t dev_id, fal_global_qinq_mode_t *mode)
{
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

	if (FAL_FLG_TST(mode->mask, FAL_GLOBAL_QINQ_MODE_INGRESS_EN)) {
		rtn = hppe_bridge_config_bridge_type_set(dev_id,
				(a_uint32_t)mode->ingress_mode);
		SW_RTN_ON_ERROR(rtn);
	}

	if (FAL_FLG_TST(mode->mask, FAL_GLOBAL_QINQ_MODE_EGRESS_EN)) {
		rtn = hppe_eg_bridge_config_bridge_type_set(dev_id,
				(a_uint32_t)mode->egress_mode);
		SW_RTN_ON_ERROR(rtn);
	}

	if (FAL_FLG_TST(mode->mask, FAL_GLOBAL_QINQ_MODE_EGRESS_UNTOUCHED_FOR_CPU_CODE)) {
		SW_RTN_ON_ERROR(hppe_eg_bridge_config_pkt_l2_edit_en_set(dev_id,
					(a_uint32_t)!mode->untouched_for_cpucode));
	}
	return rtn;
}

sw_error_t
adpt_hppe_global_qinq_mode_get(a_uint32_t dev_id, fal_global_qinq_mode_t *mode)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t l2_edit_en = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mode);

	rtn = hppe_bridge_config_bridge_type_get(dev_id,
			(a_uint32_t *)&mode->ingress_mode);
	SW_RTN_ON_ERROR(rtn);

	rtn = hppe_eg_bridge_config_bridge_type_get(dev_id,
			(a_uint32_t *)&mode->egress_mode);

	SW_RTN_ON_ERROR(hppe_eg_bridge_config_pkt_l2_edit_en_get(dev_id, &l2_edit_en));

	mode->untouched_for_cpucode = !l2_edit_en;

	return rtn;
}

sw_error_t
adpt_hppe_port_qinq_mode_set(a_uint32_t dev_id, fal_port_t port_id, fal_port_qinq_role_t *mode)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mode);

#if defined(APPE)
	if (ADPT_IS_PPORT(port_id)) {
		if (FAL_FLG_TST(mode->mask, FAL_PORT_QINQ_ROLE_TUNNEL_EN)) {
			rtn = appe_tpr_port_parsing_port_role_set(dev_id,
					port_value, mode->tunnel_port_role);
			SW_RTN_ON_ERROR(rtn);
		}
	}

	if (ADPT_IS_VPORT(port_id)) {
		if (FAL_FLG_TST(mode->mask, FAL_PORT_QINQ_ROLE_INGRESS_EN)) {
			union ipr_vp_parsing_u vp_parsing_reg;
			aos_mem_zero(&vp_parsing_reg, sizeof(vp_parsing_reg));

			rtn = appe_ipr_vp_parsing_get(dev_id, (port_value - SSDK_MIN_VIRTUAL_PORT_ID), &vp_parsing_reg);
			SW_RTN_ON_ERROR(rtn);

			vp_parsing_reg.bf.port_role = mode->ingress_port_role;
#if defined(MPPE)
			if(adpt_ppe_type_get(dev_id) == MPPE_TYPE) {
				vp_parsing_reg.bf.src_port_sel =
					mode->ingress_port_sel == FAL_QINQ_SEL_TNL_DECAP_SRC_VP ? 0 : 1;
			}
#endif
			rtn = appe_ipr_vp_parsing_set(dev_id, (port_value - SSDK_MIN_VIRTUAL_PORT_ID), &vp_parsing_reg);
			SW_RTN_ON_ERROR(rtn);
		}

		if (FAL_FLG_TST(mode->mask, FAL_PORT_QINQ_ROLE_EGRESS_EN)) {
			rtn = appe_eg_vp_tbl_port_vlan_type_set(dev_id, port_value,
					(a_uint32_t)mode->egress_port_role);
			SW_RTN_ON_ERROR(rtn);
		}
	} else
#endif
	if (ADPT_IS_PPORT(port_id)) {
		if (FAL_FLG_TST(mode->mask, FAL_PORT_QINQ_ROLE_INGRESS_EN)) {
			union port_parsing_reg_u port_parsing_reg;
			aos_mem_zero(&port_parsing_reg, sizeof(port_parsing_reg));

			rtn = hppe_port_parsing_reg_get(dev_id, port_value, &port_parsing_reg);
			SW_RTN_ON_ERROR(rtn);

			port_parsing_reg.bf.port_role = mode->ingress_port_role;
#if defined(MPPE)
			if(adpt_ppe_type_get(dev_id) == MPPE_TYPE) {
				port_parsing_reg.bf.src_port_sel =
					mode->ingress_port_sel == FAL_QINQ_SEL_TNL_DECAP_SRC_VP ? 0 : 1;
			}
#endif
			rtn = hppe_port_parsing_reg_set(dev_id, port_value, &port_parsing_reg);
			SW_RTN_ON_ERROR(rtn);
		}

		if (FAL_FLG_TST(mode->mask, FAL_PORT_QINQ_ROLE_EGRESS_EN)) {
			SW_RTN_ON_ERROR(hppe_port_eg_vlan_port_vlan_type_set(dev_id, port_value,
						(a_uint32_t)mode->egress_port_role));
		}
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_qinq_mode_get(a_uint32_t dev_id, fal_port_t port_id, fal_port_qinq_role_t *mode)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mode);

#if defined(APPE)
	if (ADPT_IS_PPORT(port_id)) {
		rtn = appe_tpr_port_parsing_port_role_get(dev_id,
				port_value, (a_uint32_t *)&mode->tunnel_port_role);
		SW_RTN_ON_ERROR(rtn);
	}

	if (ADPT_IS_VPORT(port_id)) {
		union ipr_vp_parsing_u vp_parsing_reg;
		aos_mem_zero(&vp_parsing_reg, sizeof(vp_parsing_reg));

		rtn = appe_ipr_vp_parsing_get(dev_id, (port_value - SSDK_MIN_VIRTUAL_PORT_ID), &vp_parsing_reg);
		SW_RTN_ON_ERROR(rtn);

		mode->ingress_port_role = (fal_qinq_port_role_t)vp_parsing_reg.bf.port_role;
#if defined(MPPE)
		if(adpt_ppe_type_get(dev_id) == MPPE_TYPE) {
			mode->ingress_port_sel =
				vp_parsing_reg.bf.src_port_sel ? FAL_QINQ_SEL_ORG_SRC_PORT : FAL_QINQ_SEL_TNL_DECAP_SRC_VP;
		}
#endif

		rtn = appe_eg_vp_tbl_port_vlan_type_get(dev_id, port_value,
				(a_uint32_t *)&mode->egress_port_role);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	if (ADPT_IS_PPORT(port_id)) {
		union port_parsing_reg_u port_parsing_reg;
		aos_mem_zero(&port_parsing_reg, sizeof(port_parsing_reg));

		rtn = hppe_port_parsing_reg_get(dev_id, port_value, &port_parsing_reg);
		SW_RTN_ON_ERROR(rtn);

		mode->ingress_port_role = (fal_qinq_port_role_t)port_parsing_reg.bf.port_role;
#if defined(MPPE)
		if(adpt_ppe_type_get(dev_id) == MPPE_TYPE) {
			mode->ingress_port_sel =
				port_parsing_reg.bf.src_port_sel ? FAL_QINQ_SEL_ORG_SRC_PORT : FAL_QINQ_SEL_TNL_DECAP_SRC_VP;
		}
#endif

		SW_RTN_ON_ERROR(hppe_port_eg_vlan_port_vlan_type_get(dev_id, port_value,
					(a_uint32_t *)&mode->egress_port_role));
	}

	return rtn;
}

sw_error_t
adpt_hppe_tpid_set(a_uint32_t dev_id, fal_tpid_t *tpid)
{
	sw_error_t rtn = SW_OK;
	union edma_vlan_tpid_reg_u edma_tpid;
	union vlan_tpid_reg_u ppe_tpid;
#if defined(APPE)
	union tpr_vlan_tpid_u tunnel_tpid;
#endif

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tpid);

	rtn = hppe_edma_vlan_tpid_reg_get(dev_id, &edma_tpid);
	SW_RTN_ON_ERROR(rtn);

	rtn = hppe_vlan_tpid_reg_get(dev_id, &ppe_tpid);
	SW_RTN_ON_ERROR(rtn);

	if (FAL_FLG_TST(tpid->mask, FAL_TPID_CTAG_EN)) {
		edma_tpid.bf.ctag_tpid = tpid->ctpid;
		ppe_tpid.bf.ctag_tpid = tpid->ctpid;
	}

	if (FAL_FLG_TST(tpid->mask, FAL_TPID_STAG_EN)) {
		edma_tpid.bf.stag_tpid = tpid->stpid;
		ppe_tpid.bf.stag_tpid = tpid->stpid;
	}

#if defined(APPE)
	/* edma tpid is configured as same as tunnel tpid for appe
	 * for hppe, edma tpid is configured as ipr tpid
	 */
	rtn = appe_tpr_vlan_tpid_get(dev_id, &tunnel_tpid);
	SW_RTN_ON_ERROR(rtn);

	if (FAL_FLG_TST(tpid->mask, FAL_TUNNEL_TPID_CTAG_EN)) {
		edma_tpid.bf.ctag_tpid = tpid->tunnel_ctpid;
		tunnel_tpid.bf.ctag_tpid = tpid->tunnel_ctpid;
	}

	if (FAL_FLG_TST(tpid->mask, FAL_TUNNEL_TPID_STAG_EN)) {
		edma_tpid.bf.stag_tpid = tpid->tunnel_stpid;
		tunnel_tpid.bf.stag_tpid = tpid->tunnel_stpid;
	}

	rtn = appe_tpr_vlan_tpid_set(dev_id, &tunnel_tpid);
	SW_RTN_ON_ERROR(rtn);
#endif

	rtn = hppe_edma_vlan_tpid_reg_set(dev_id, &edma_tpid);
	SW_RTN_ON_ERROR(rtn);

	rtn = hppe_vlan_tpid_reg_set(dev_id, &ppe_tpid);

	return rtn;
}

sw_error_t
adpt_hppe_tpid_get(a_uint32_t dev_id, fal_tpid_t *tpid)
{
	sw_error_t rtn = SW_OK;
	union vlan_tpid_reg_u ppe_tpid;
#if defined(APPE)
	union tpr_vlan_tpid_u tunnel_tpid;
#endif

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tpid);

	rtn = hppe_vlan_tpid_reg_get(dev_id, &ppe_tpid);
	SW_RTN_ON_ERROR(rtn);

#if defined(APPE)
	/* edma tpid is configured as same as tunnel tpid for appe
	 * for hppe, edma tpid is configured as ipr tpid
	 */
	rtn = appe_tpr_vlan_tpid_get(dev_id, &tunnel_tpid);
	SW_RTN_ON_ERROR(rtn);

	tpid->tunnel_ctpid = tunnel_tpid.bf.ctag_tpid;
	tpid->tunnel_stpid = tunnel_tpid.bf.stag_tpid;
#endif

	tpid->ctpid = ppe_tpid.bf.ctag_tpid;
	tpid->stpid = ppe_tpid.bf.stag_tpid;

	return rtn;
}

sw_error_t
adpt_hppe_egress_tpid_set(a_uint32_t dev_id, fal_tpid_t *tpid)
{
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

	if (FAL_FLG_TST(tpid->mask, FAL_TPID_CTAG_EN)) {
		rtn = hppe_eg_vlan_tpid_ctpid_set(dev_id,
				(a_uint32_t)tpid->ctpid);
		SW_RTN_ON_ERROR(rtn);
	}

	if (FAL_FLG_TST(tpid->mask, FAL_TPID_STAG_EN)) {
		rtn = hppe_eg_vlan_tpid_stpid_set(dev_id,
				(a_uint32_t)tpid->stpid);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

sw_error_t
adpt_hppe_egress_tpid_get(a_uint32_t dev_id, fal_tpid_t *tpid)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t tmp = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tpid);

	rtn = hppe_eg_vlan_tpid_ctpid_get(dev_id, &tmp);
	SW_RTN_ON_ERROR(rtn);
	tpid->ctpid = tmp;

	rtn = hppe_eg_vlan_tpid_stpid_get(dev_id, &tmp);
	SW_RTN_ON_ERROR(rtn);
	tpid->stpid = tmp;

	return rtn;
}

sw_error_t
adpt_hppe_port_vlantag_egmode_set(a_uint32_t dev_id, fal_port_t port_id,
                            fal_vlantag_egress_mode_t *port_egvlanmode)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value = 0;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);

	if (FAL_FLG_TST(port_egvlanmode->mask, FAL_EGRESSMODE_CTAG_EN)) {
		rtn = adpt_portvlan_egmode_convert(&port_egvlanmode->ctag_mode, &value, A_TRUE);
		SW_RTN_ON_ERROR(rtn);

#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			rtn = appe_eg_vp_tbl_port_eg_vlan_ctag_mode_set(dev_id,
					port_value, value);
			SW_RTN_ON_ERROR(rtn);
		} else
#endif
		{
			rtn = hppe_port_eg_vlan_port_eg_vlan_ctag_mode_set(dev_id,
					port_value, value);
			SW_RTN_ON_ERROR(rtn);
		}
	}

	if (FAL_FLG_TST(port_egvlanmode->mask, FAL_EGRESSMODE_STAG_EN)) {
		rtn = adpt_portvlan_egmode_convert(&port_egvlanmode->stag_mode, &value, A_TRUE);
		SW_RTN_ON_ERROR(rtn);

#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			rtn = appe_eg_vp_tbl_port_eg_vlan_stag_mode_set(dev_id,
					port_value, value);
			SW_RTN_ON_ERROR(rtn);
		} else
#endif
		{
			rtn = hppe_port_eg_vlan_port_eg_vlan_stag_mode_set(dev_id,
					port_value, value);
			SW_RTN_ON_ERROR(rtn);
		}
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_vlantag_egmode_get(a_uint32_t dev_id, fal_port_t port_id,
                            fal_vlantag_egress_mode_t *port_egvlanmode)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value = 0;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(port_egvlanmode);


#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		rtn = appe_eg_vp_tbl_port_eg_vlan_ctag_mode_get(dev_id,
				port_value, &value);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_port_eg_vlan_port_eg_vlan_ctag_mode_get(dev_id,
				port_value, &value);
		SW_RTN_ON_ERROR(rtn);
	}

	rtn = adpt_portvlan_egmode_convert(&port_egvlanmode->ctag_mode, &value, A_FALSE);
	SW_RTN_ON_ERROR(rtn);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		rtn = appe_eg_vp_tbl_port_eg_vlan_stag_mode_get(dev_id,
				port_value, &value);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_port_eg_vlan_port_eg_vlan_stag_mode_get(dev_id,
				port_value, &value);
		SW_RTN_ON_ERROR(rtn);
	}

	rtn = adpt_portvlan_egmode_convert(&port_egvlanmode->stag_mode, &value, A_FALSE);
	SW_RTN_ON_ERROR(rtn);

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_xlt_miss_cmd_set(a_uint32_t dev_id, fal_port_t port_id,
                                 fal_fwd_cmd_t cmd)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value = 0;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);

	rtn = adpt_forward_action_convert(&cmd, &value, A_TRUE);
	SW_RTN_ON_ERROR(rtn);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		rtn = appe_vlan_port_vp_tbl_port_vlan_xlt_miss_fwd_cmd_set(dev_id,
				port_value, value);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_port_vlan_config_port_vlan_xlt_miss_fwd_cmd_set(dev_id,
				port_value, value);
		SW_RTN_ON_ERROR(rtn);
	}


	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_xlt_miss_cmd_get(a_uint32_t dev_id, fal_port_t port_id,
                                 fal_fwd_cmd_t *cmd)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value = 0;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cmd);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		rtn = appe_vlan_port_vp_tbl_port_vlan_xlt_miss_fwd_cmd_get(dev_id,
				port_value, &value);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_port_vlan_config_port_vlan_xlt_miss_fwd_cmd_get(dev_id,
					port_value, &value);
		SW_RTN_ON_ERROR(rtn);
	}

	rtn = adpt_forward_action_convert(cmd, &value, A_FALSE);
	SW_RTN_ON_ERROR(rtn);

	return rtn;
}

sw_error_t
adpt_hppe_port_vsi_egmode_set(a_uint32_t dev_id,
		a_uint32_t vsi, a_uint32_t port_id, fal_pt_1q_egmode_t egmode)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value, tag_value;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);


	rtn = adpt_portvlan_egmode_convert(&egmode, &value, A_TRUE);
	SW_RTN_ON_ERROR(rtn);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		/* one vsi for 256 ports, 2 bits per VP port, 8 VP ports per entry */
		rtn = appe_eg_vsi_vp_tag_tagged_mode_vp_bitmap_get(dev_id,
				(vsi << 5) + (port_value >> 3),
				&tag_value);
		SW_RTN_ON_ERROR(rtn);

		tag_value &= ~(0x3 << (port_value * 2));
		tag_value |= (value << (port_value * 2));

		rtn = appe_eg_vsi_vp_tag_tagged_mode_vp_bitmap_set(dev_id,
				(vsi << 5) + (port_value >> 3),
				tag_value);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_eg_vsi_tag_tagged_mode_port_bitmap_get(dev_id, vsi, &tag_value);
		SW_RTN_ON_ERROR(rtn);

		tag_value &= ~(0x3 << (port_value * 2));
		tag_value |= (value << (port_value * 2));

		rtn = hppe_eg_vsi_tag_tagged_mode_port_bitmap_set(dev_id, vsi, tag_value);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_vsi_egmode_get(a_uint32_t dev_id,
		a_uint32_t vsi, a_uint32_t port_id, fal_pt_1q_egmode_t *egmode)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value, tag_value;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(egmode);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		/* one vsi for 256 ports, 2 bits per VP port, 8 VP ports per entry */
		rtn = appe_eg_vsi_vp_tag_tagged_mode_vp_bitmap_get(dev_id,
				(vsi << 5) + (port_value >> 3),
				&tag_value);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_eg_vsi_tag_tagged_mode_port_bitmap_get(dev_id, vsi, &tag_value);
		SW_RTN_ON_ERROR(rtn);
	}

	value = (tag_value >> (port_value * 2)) & 0x3;

	rtn = adpt_portvlan_egmode_convert(egmode, &value, A_FALSE);
	SW_RTN_ON_ERROR(rtn);

	return rtn;
}

sw_error_t
adpt_hppe_port_vlantag_vsi_egmode_enable_set(a_uint32_t dev_id,
		fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		rtn = appe_eg_vp_tbl_vsi_tag_mode_en_set(dev_id, port_value, enable);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_port_eg_vlan_vsi_tag_mode_en_set(dev_id, port_value, enable);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_vlantag_vsi_egmode_enable_get(a_uint32_t dev_id,
		fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		rtn = appe_eg_vp_tbl_vsi_tag_mode_en_get(dev_id, port_value, enable);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_port_eg_vlan_vsi_tag_mode_en_get(dev_id, port_value, enable);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_trans_adv_add(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t entry_idx = XLT_RULE_TBL_NUM;
	a_int32_t idx, rule_valid, entry_found;
	fal_vlan_trans_adv_rule_t temp_rule;
	fal_vlan_trans_adv_action_t temp_action;
	a_bool_t entry_sign, insert_entry_ready;

	ADPT_DEV_ID_CHECK(dev_id);

	if (direction == FAL_PORT_VLAN_ALL)
		return SW_FAIL;

	if (FAL_PORT_ID_TYPE(port_id) == FAL_PORT_TYPE_PPORT) {
		rule->port_bitmap = BIT(port_id);
	} else {
		rule->port_bitmap = port_id;
	}

	entry_sign = A_FALSE;
	insert_entry_ready = A_FALSE;
	for (idx = XLT_RULE_TBL_NUM - 1; idx >= 0; idx--) {
		aos_mem_zero(&temp_rule, sizeof (fal_vlan_trans_adv_rule_t));
		aos_mem_zero(&temp_action, sizeof (fal_vlan_trans_adv_action_t));
		rule_valid = _get_port_vlan_trans_adv_rule_by_index(dev_id,
				idx, direction, &temp_rule, &temp_action);

		if (!rule_valid) { /* nonexist rule */
			if (!entry_sign) {
				entry_idx = idx;
				entry_sign = A_TRUE;
			}
			continue;
		}
		/* existing rule */
		if (!_check_if_rule_equal(direction, &temp_rule, rule)) {
			/* rule equal */
			if (!_check_if_action_equal(direction, &temp_action, action)) {
				/* action equal */
				entry_found = adpt_port_id_compare(temp_rule.port_bitmap,
						port_id);
				if (entry_found == ADPT_PORT_ID_EQUAL ||
						entry_found == ADPT_PORT_ID_INCLD) {
					return SW_ALREADY_EXIST;
				} else if (entry_found == ADPT_PORT_ID_EXCLD) {
					SW_PBMP_ADD_PORT(temp_rule.port_bitmap, port_id);
					_insert_vlan_trans_adv_rule_action(
							dev_id,
							idx, direction,
							&temp_rule,
							&temp_action);
					return SW_OK;
				} else {
					/* need to insert a new portvlan entry */
					insert_entry_ready = A_TRUE;
				}
			} else { /* action not equal */
				entry_found = adpt_port_id_compare(temp_rule.port_bitmap,
						port_id);
				if (entry_found == ADPT_PORT_ID_EQUAL) {
					/* port equal, need update action */
					_insert_vlan_trans_adv_rule_action(dev_id,
							idx, direction,
							&temp_rule,
							action);
					return SW_OK;
				} else {
					if (entry_found == ADPT_PORT_ID_INCLD) {
						/* port included, need remove port from
						 * existing rule bitmap, insert new rule
						 * and action later */
						SW_PBMP_DEL_PORT(temp_rule.port_bitmap, port_id);
						_insert_vlan_trans_adv_rule_action(dev_id,
								idx, direction,
								&temp_rule,
								&temp_action);
					}

					/* need to insert a new portvlan entry */
					insert_entry_ready = A_TRUE;
				}
			}
		}

		if (insert_entry_ready == A_TRUE && entry_sign == A_TRUE) {
			break;
		}
	}

	if (entry_idx == XLT_RULE_TBL_NUM) {
		return SW_NO_RESOURCE;
	} else {
		/* insert new rule and action */
		rtn = _insert_vlan_trans_adv_rule_action(dev_id, entry_idx,
				direction, rule, action);
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_trans_adv_del(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t idx, rule_valid, entry_found;
	fal_vlan_trans_adv_rule_t temp_rule;
	fal_vlan_trans_adv_action_t temp_action;
	union xlt_rule_tbl_u in_vlan_xlt_rule;
	union eg_vlan_xlt_rule_u eg_vlan_xlt_rule;
	union xlt_action_tbl_u in_vlan_xlt_action;
	union eg_vlan_xlt_action_u eg_vlan_xlt_action;

	memset(&in_vlan_xlt_rule, 0, sizeof(in_vlan_xlt_rule));
	memset(&eg_vlan_xlt_rule, 0, sizeof(struct eg_vlan_xlt_rule));
	memset(&in_vlan_xlt_action, 0, sizeof(struct xlt_action_tbl));
	memset(&eg_vlan_xlt_action, 0, sizeof(struct eg_vlan_xlt_action));

	ADPT_DEV_ID_CHECK(dev_id);

	if (direction == FAL_PORT_VLAN_ALL)
		return SW_FAIL;

	if (FAL_PORT_ID_TYPE(port_id) == FAL_PORT_TYPE_PPORT) {
		rule->port_bitmap = BIT(port_id);
	} else {
		rule->port_bitmap = port_id;
	}

	for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
		aos_mem_zero(&temp_rule, sizeof (fal_vlan_trans_adv_rule_t));
		aos_mem_zero(&temp_action, sizeof (fal_vlan_trans_adv_action_t));
		rule_valid = _get_port_vlan_trans_adv_rule_by_index(dev_id,
				idx, direction, &temp_rule, &temp_action);

		if (!rule_valid) {
			continue;
		}

		/* existing valid rule */
		if (!_check_if_rule_equal(direction, &temp_rule, rule))
		{ /* rule equal */
			if (!_check_if_action_equal(direction, &temp_action, action))
			{ /* action equal */
				entry_found = adpt_port_id_compare(temp_rule.port_bitmap,
						port_id);

				if (entry_found == ADPT_PORT_ID_EQUAL) {
					/* port equal, need delete existing rule and action */
					if (direction == FAL_PORT_VLAN_INGRESS) {
						rtn = hppe_xlt_rule_tbl_set(dev_id,
								idx, &in_vlan_xlt_rule);
						SW_RTN_ON_ERROR(rtn);
						rtn = hppe_xlt_action_tbl_set(dev_id,
								idx, &in_vlan_xlt_action);
						SW_RTN_ON_ERROR(rtn);
					} else {
						rtn = hppe_eg_vlan_xlt_rule_set(dev_id,
								idx, &eg_vlan_xlt_rule);
						SW_RTN_ON_ERROR(rtn);
						rtn = hppe_eg_vlan_xlt_action_set(dev_id,
								idx, &eg_vlan_xlt_action);
						SW_RTN_ON_ERROR(rtn);
					}
				} else if (entry_found == ADPT_PORT_ID_INCLD) {
					/* current port_bitmap includes this port_id,
					   remove port from port_bitmap and update rule
					   and action */
					SW_PBMP_DEL_PORT(temp_rule.port_bitmap, port_id);
					rtn = _insert_vlan_trans_adv_rule_action(dev_id,
							idx, direction, &temp_rule,
							&temp_action);
				}
				else {
					/* VP or VP group NOT EQUAL */
					continue;
				}

				break;
			}
		}
	}

	if (idx == XLT_RULE_TBL_NUM)
		return SW_NOT_FOUND;

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_trans_adv_getfirst(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t idx, rule_valid, entry_found;
	fal_vlan_trans_adv_rule_t temp_rule;
	fal_vlan_trans_adv_action_t temp_action;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule);
	ADPT_NULL_POINT_CHECK(action);

	if (direction == FAL_PORT_VLAN_ALL)
		return SW_FAIL;

	entry_found = A_FALSE;
	for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
		aos_mem_zero(&temp_rule, sizeof (fal_vlan_trans_adv_rule_t));
		aos_mem_zero(&temp_action, sizeof (fal_vlan_trans_adv_action_t));
		rule_valid = _get_port_vlan_trans_adv_rule_by_index(dev_id,
				idx, direction, &temp_rule, &temp_action);
		if (rule_valid == 1) {
			entry_found = adpt_port_id_compare(temp_rule.port_bitmap, port_id);

			if (entry_found == ADPT_PORT_ID_EQUAL ||
					entry_found == ADPT_PORT_ID_INCLD) {
				aos_mem_copy(rule, &temp_rule,
						sizeof(fal_vlan_trans_adv_rule_t));
				aos_mem_copy(action, &temp_action,
						sizeof(fal_vlan_trans_adv_action_t));
				break;
			}
		}
	}

	if (idx == XLT_RULE_TBL_NUM)
		return SW_NOT_FOUND;

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_trans_adv_getnext(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t idx, rule_valid, entry_found;
	fal_vlan_trans_adv_rule_t temp_rule;
	fal_vlan_trans_adv_action_t temp_action;
	a_bool_t sign_tag = A_FALSE;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule);
	ADPT_NULL_POINT_CHECK(action);

	if (direction == FAL_PORT_VLAN_ALL)
		return SW_FAIL;

	if (FAL_PORT_ID_TYPE(port_id) == FAL_PORT_TYPE_PPORT) {
		rule->port_bitmap = BIT(port_id);
	} else {
		rule->port_bitmap = port_id;
	}

	for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
		aos_mem_zero(&temp_rule, sizeof (fal_vlan_trans_adv_rule_t));
		aos_mem_zero(&temp_action, sizeof (fal_vlan_trans_adv_action_t));
		rule_valid = _get_port_vlan_trans_adv_rule_by_index(dev_id, idx,
				direction, &temp_rule, &temp_action);
		if (rule_valid == 1)
		{ /* existing rule */
			entry_found = adpt_port_id_compare(temp_rule.port_bitmap,
					port_id);
			if (sign_tag == A_TRUE && (entry_found == ADPT_PORT_ID_EQUAL ||
						entry_found == ADPT_PORT_ID_INCLD))
			{
				aos_mem_copy(rule, &temp_rule,
						sizeof (fal_vlan_trans_adv_rule_t));
				aos_mem_copy(action, &temp_action,
						sizeof (fal_vlan_trans_adv_action_t));
				break;
			}
			if (!_check_if_rule_equal(direction, &temp_rule, rule))
			{ /* rule equal */
				if (!_check_if_action_equal(direction, &temp_action, action))
				{ /* action equal */
					if (entry_found == ADPT_PORT_ID_EQUAL ||
							entry_found == ADPT_PORT_ID_INCLD) {
						sign_tag = A_TRUE;
					}
				}
			}
		}
	}

	if (idx == XLT_RULE_TBL_NUM)
		return SW_NOT_FOUND;

	return rtn;
}

sw_error_t
adpt_hppe_portvlan_member_update(a_uint32_t dev_id, fal_port_t port_id, fal_pbmp_t mem_port_map)
{
	union port_bridge_ctrl_u port_bridge_ctrl;
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

#ifdef APPE
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		if(ADPT_IS_VPORT(port_id))
		{
			return adpt_appe_portvlan_vpmember_update(dev_id, port_id, mem_port_map);
		}
	}
#endif

	port_id = FAL_PORT_ID_VALUE(port_id);

	memset(&port_bridge_ctrl, 0, sizeof(port_bridge_ctrl));
	rtn = hppe_port_bridge_ctrl_get(dev_id, port_id, &port_bridge_ctrl);
	SW_RTN_ON_ERROR(rtn);

	port_bridge_ctrl.bf.port_isolation_bitmap = mem_port_map;
	rtn = hppe_port_bridge_ctrl_set(dev_id, port_id, &port_bridge_ctrl);

	return rtn;
}

sw_error_t
adpt_hppe_portvlan_member_get(a_uint32_t dev_id, fal_port_t port_id, fal_pbmp_t * mem_port_map)
{
	union port_bridge_ctrl_u port_bridge_ctrl;
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

#ifdef APPE
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		if(ADPT_IS_VPORT(port_id))
		{
			return adpt_appe_portvlan_vpmember_get(dev_id, port_id, mem_port_map);
		}
	}
#endif

	port_id = FAL_PORT_ID_VALUE(port_id);

	memset(&port_bridge_ctrl, 0, sizeof(port_bridge_ctrl));
	rtn = hppe_port_bridge_ctrl_get(dev_id, port_id, &port_bridge_ctrl);
	SW_RTN_ON_ERROR(rtn);

	*mem_port_map = port_bridge_ctrl.bf.port_isolation_bitmap;

	return rtn;
}

sw_error_t
adpt_hppe_qinq_mode_set(a_uint32_t dev_id, fal_qinq_mode_t mode)
{
	sw_error_t rtn = SW_OK;
	fal_global_qinq_mode_t global_mode;

	ADPT_DEV_ID_CHECK(dev_id);

	memset(&global_mode, 0, sizeof(global_mode));
	global_mode.mask = 0x3;
	global_mode.ingress_mode = mode;
	global_mode.egress_mode = mode;
	adpt_hppe_global_qinq_mode_set(dev_id, &global_mode);

	return rtn;
}

sw_error_t
adpt_hppe_qinq_mode_get(a_uint32_t dev_id, fal_qinq_mode_t * mode)
{
	sw_error_t rtn = SW_OK;
	fal_global_qinq_mode_t global_mode;

	ADPT_DEV_ID_CHECK(dev_id);

	adpt_hppe_global_qinq_mode_get(dev_id, &global_mode);

	if (global_mode.ingress_mode == global_mode.egress_mode)
		*mode = global_mode.ingress_mode;
	else
		return SW_FAIL;

	return rtn;
}

sw_error_t
adpt_hppe_port_qinq_role_set(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t role)
{
	sw_error_t rtn = SW_OK;
	fal_port_qinq_role_t port_role;

	ADPT_DEV_ID_CHECK(dev_id);

	port_role.mask = 0x3;
	port_role.ingress_port_role = role;
	port_role.egress_port_role = role;
	adpt_hppe_port_qinq_mode_set(dev_id, port_id, &port_role);

	return rtn;
}

sw_error_t
adpt_hppe_port_qinq_role_get(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t * role)
{
	sw_error_t rtn = SW_OK;
	fal_port_qinq_role_t port_role;

	ADPT_DEV_ID_CHECK(dev_id);

	adpt_hppe_port_qinq_mode_get(dev_id, port_id, &port_role);

	if (port_role.ingress_port_role == port_role.egress_port_role)
		*role = port_role.ingress_port_role;
	else
		return SW_FAIL;

	return rtn;
}

sw_error_t
adpt_hppe_port_default_vlantag_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_port_default_vid_enable_t *default_vid_en, fal_port_vlan_tag_t *default_tag)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);

	if (direction == FAL_PORT_VLAN_EGRESS)
	{
		if (!FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CVID_EN) && 
			!FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SVID_EN))
			return SW_NOT_SUPPORTED;
	}

	if (direction == FAL_PORT_VLAN_ALL || direction == FAL_PORT_VLAN_INGRESS)
	{
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union vlan_port_vp_tbl_u vlan_port_vp;

			rtn = appe_vlan_port_vp_tbl_get(dev_id, port_value, &vlan_port_vp);
			SW_RTN_ON_ERROR(rtn);

			vlan_port_vp.bf.port_def_svid_en = default_vid_en->default_svid_en;
			vlan_port_vp.bf.port_def_cvid_en = default_vid_en->default_cvid_en;

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CVID_EN)) {
				vlan_port_vp.bf.port_def_cvid = default_tag->cvid;
			}
			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SVID_EN)) {
				vlan_port_vp.bf.port_def_svid = default_tag->svid;
			}
			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CPCP_EN)) {
				vlan_port_vp.bf.port_def_cpcp_0 = default_tag->cpri & 0x3;
				vlan_port_vp.bf.port_def_cpcp_1 = (default_tag->cpri & 0x4) >> 2;
			}
			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SPCP_EN)) {
				vlan_port_vp.bf.port_def_spcp = default_tag->spri;
			}
			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CDEI_EN)) {
				vlan_port_vp.bf.port_def_cdei = default_tag->cdei;
			}
			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SDEI_EN)) {
				vlan_port_vp.bf.port_def_sdei = default_tag->sdei;
			}
			rtn = appe_vlan_port_vp_tbl_set(dev_id, port_value, &vlan_port_vp);
			SW_RTN_ON_ERROR(rtn);
		} else
#endif
		if (ADPT_IS_PPORT(port_id)) {
			rtn = hppe_port_def_vid_port_def_cvid_en_set(dev_id, port_value,
					(a_uint32_t)default_vid_en->default_cvid_en);
			SW_RTN_ON_ERROR(rtn);
			rtn = hppe_port_def_vid_port_def_svid_en_set(dev_id, port_value,
					(a_uint32_t)default_vid_en->default_svid_en);
			SW_RTN_ON_ERROR(rtn);

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CVID_EN)) {
				rtn = hppe_port_def_vid_port_def_cvid_set(dev_id, port_value,
						(a_uint32_t)default_tag->cvid);
				SW_RTN_ON_ERROR(rtn);
			}

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SVID_EN)) {
				rtn = hppe_port_def_vid_port_def_svid_set(dev_id, port_value,
						(a_uint32_t)default_tag->svid);
				SW_RTN_ON_ERROR(rtn);
			}

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CPCP_EN)) {
				rtn = hppe_port_def_pcp_port_def_cpcp_set(dev_id, port_value,
						(a_uint32_t)default_tag->cpri);
				SW_RTN_ON_ERROR(rtn);
			}

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SPCP_EN)) {
				rtn = hppe_port_def_pcp_port_def_spcp_set(dev_id, port_value,
						(a_uint32_t)default_tag->spri);
				SW_RTN_ON_ERROR(rtn);
			}

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CDEI_EN)) {
				rtn = hppe_port_def_pcp_port_def_cdei_set(dev_id, port_value,
						(a_uint32_t)default_tag->cdei);
				SW_RTN_ON_ERROR(rtn);
			}

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SDEI_EN)) {
				rtn = hppe_port_def_pcp_port_def_sdei_set(dev_id, port_value,
						(a_uint32_t)default_tag->sdei);
				SW_RTN_ON_ERROR(rtn);
			}
		}
	}

	if (direction == FAL_PORT_VLAN_ALL || direction == FAL_PORT_VLAN_EGRESS)
	{
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union eg_vp_tbl_u eg_vp;

			rtn = appe_egress_vp_tbl_get(dev_id, port_value, &eg_vp);
			SW_RTN_ON_ERROR(rtn);

			eg_vp.bf.port_def_svid_en = default_vid_en->default_svid_en;
			eg_vp.bf.port_def_cvid_en = default_vid_en->default_cvid_en;

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CVID_EN)) {
				eg_vp.bf.port_def_cvid = default_tag->cvid;
			}
			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SVID_EN)) {
				eg_vp.bf.port_def_svid = default_tag->svid;
			}
			rtn = appe_egress_vp_tbl_set(dev_id, port_value, &eg_vp);
			SW_RTN_ON_ERROR(rtn);
		} else
#endif
		if (ADPT_IS_PPORT(port_id)) {
			rtn = hppe_port_eg_def_vid_port_def_cvid_en_set(dev_id, port_value,
					(a_uint32_t)default_vid_en->default_cvid_en);
			SW_RTN_ON_ERROR(rtn);

			rtn = hppe_port_eg_def_vid_port_def_svid_en_set(dev_id, port_value,
					(a_uint32_t)default_vid_en->default_svid_en);
			SW_RTN_ON_ERROR(rtn);

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_CVID_EN)) {
				rtn = hppe_port_eg_def_vid_port_def_cvid_set(dev_id, port_value,
						(a_uint32_t)default_tag->cvid);
				SW_RTN_ON_ERROR(rtn);
			}

			if (FAL_FLG_TST(default_tag->mask, FAL_PORT_VLAN_TAG_SVID_EN)) {
				rtn = hppe_port_eg_def_vid_port_def_svid_set(dev_id, port_value,
						(a_uint32_t)default_tag->svid);
				SW_RTN_ON_ERROR(rtn);
			}
		}
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_default_vlantag_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_port_default_vid_enable_t *default_vid_en, fal_port_vlan_tag_t *default_tag)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(default_vid_en);
	ADPT_NULL_POINT_CHECK(default_tag);

	if (direction == FAL_PORT_VLAN_ALL)
		return SW_NOT_SUPPORTED;

	if (direction == FAL_PORT_VLAN_INGRESS) {
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union vlan_port_vp_tbl_u vlan_port_vp;

			rtn = appe_vlan_port_vp_tbl_get(dev_id, port_value, &vlan_port_vp);
			SW_RTN_ON_ERROR(rtn);

			default_vid_en->default_cvid_en = vlan_port_vp.bf.port_def_cvid_en;
			default_vid_en->default_svid_en = vlan_port_vp.bf.port_def_svid_en;

			default_tag->cvid = vlan_port_vp.bf.port_def_cvid;
			default_tag->svid = vlan_port_vp.bf.port_def_svid;
			default_tag->cpri = vlan_port_vp.bf.port_def_cpcp_1 << 2 |
				vlan_port_vp.bf.port_def_cpcp_0;
			default_tag->spri = vlan_port_vp.bf.port_def_spcp;
			default_tag->cdei = vlan_port_vp.bf.port_def_cdei;
			default_tag->sdei = vlan_port_vp.bf.port_def_sdei;
		} else
#endif
		if (ADPT_IS_PPORT(port_id)) {
			a_uint32_t tmp = 0;
			rtn = hppe_port_def_vid_port_def_cvid_en_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_vid_en->default_cvid_en = tmp;

			rtn = hppe_port_def_vid_port_def_svid_en_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_vid_en->default_svid_en = tmp;

			rtn = hppe_port_def_vid_port_def_cvid_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->cvid = tmp;

			rtn = hppe_port_def_vid_port_def_svid_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->svid = tmp;

			rtn = hppe_port_def_pcp_port_def_cpcp_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->cpri = tmp;

			rtn = hppe_port_def_pcp_port_def_spcp_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->spri = tmp;

			rtn = hppe_port_def_pcp_port_def_cdei_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->cdei = tmp;

			rtn = hppe_port_def_pcp_port_def_sdei_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->sdei = tmp;
		}
	}
	else if (direction == FAL_PORT_VLAN_EGRESS) {
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union eg_vp_tbl_u eg_vp;

			rtn = appe_egress_vp_tbl_get(dev_id, port_value, &eg_vp);
			SW_RTN_ON_ERROR(rtn);

			default_vid_en->default_cvid_en = eg_vp.bf.port_def_cvid_en;
			default_vid_en->default_svid_en = eg_vp.bf.port_def_svid_en;

			default_tag->cvid = eg_vp.bf.port_def_cvid;
			default_tag->svid = eg_vp.bf.port_def_svid;
		} else
#endif
		if (ADPT_IS_PPORT(port_id)) {
			a_uint32_t tmp = 0;

			rtn = hppe_port_eg_def_vid_port_def_cvid_en_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_vid_en->default_cvid_en = tmp;

			rtn = hppe_port_eg_def_vid_port_def_svid_en_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_vid_en->default_svid_en = tmp;

			rtn = hppe_port_eg_def_vid_port_def_cvid_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->cvid = tmp;

			rtn = hppe_port_eg_def_vid_port_def_svid_get(dev_id, port_value, &tmp);
			SW_RTN_ON_ERROR(rtn);
			default_tag->svid = tmp;
		}
	}
	else
		return SW_NOT_SUPPORTED;

	return rtn;
}

sw_error_t
adpt_hppe_portvlan_member_add(a_uint32_t dev_id, fal_port_t port_id, fal_port_t mem_port_id)
{
	union port_bridge_ctrl_u port_bridge_ctrl;
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

#ifdef APPE
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		if(ADPT_IS_VPORT(port_id))
		{
			return adpt_appe_portvlan_vpmember_add(dev_id, port_id, mem_port_id);
		}
	}
#endif

	port_id = FAL_PORT_ID_VALUE(port_id);
	mem_port_id = FAL_PORT_ID_VALUE(mem_port_id);

	memset(&port_bridge_ctrl, 0, sizeof(port_bridge_ctrl));
	rtn = hppe_port_bridge_ctrl_get(dev_id, port_id, &port_bridge_ctrl);
	SW_RTN_ON_ERROR(rtn);

	port_bridge_ctrl.bf.port_isolation_bitmap |= (0x1 << mem_port_id);

	rtn = hppe_port_bridge_ctrl_set(dev_id, port_id, &port_bridge_ctrl);

	return rtn;
}

sw_error_t
adpt_hppe_portvlan_member_del(a_uint32_t dev_id, fal_port_t port_id, fal_port_t mem_port_id)
{
	union port_bridge_ctrl_u port_bridge_ctrl;
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

#ifdef APPE
	if(adpt_chip_type_get(dev_id) == CHIP_APPE)
	{
		if(ADPT_IS_VPORT(port_id))
		{
			return adpt_appe_portvlan_vpmember_del(dev_id, port_id, mem_port_id);
		}
	}
#endif

	port_id = FAL_PORT_ID_VALUE(port_id);
	mem_port_id = FAL_PORT_ID_VALUE(mem_port_id);

	memset(&port_bridge_ctrl, 0, sizeof(port_bridge_ctrl));
	rtn = hppe_port_bridge_ctrl_get(dev_id, port_id, &port_bridge_ctrl);
	SW_RTN_ON_ERROR(rtn);

	port_bridge_ctrl.bf.port_isolation_bitmap &= ~(0x1 << mem_port_id);
	rtn = hppe_port_bridge_ctrl_set(dev_id, port_id, &port_bridge_ctrl);

	return rtn;
}

sw_error_t
adpt_hppe_port_ingress_vlan_filter_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_ingress_vlan_filter_t *filter)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		union vlan_port_vp_tbl_u vlan_vp;
		rtn = appe_vlan_port_vp_tbl_get(dev_id, port_value, &vlan_vp);
		SW_RTN_ON_ERROR(rtn);

		vlan_vp.bf.port_in_vlan_fltr_cmd = filter->membership_filter;
		vlan_vp.bf.port_untag_fltr_cmd = filter->untagged_filter;
		vlan_vp.bf.port_pri_tag_fltr_cmd = filter->priority_filter;
		vlan_vp.bf.port_tag_fltr_cmd = filter->tagged_filter;

		/* filter packet according to ctag format when the ingress port is core port */
		vlan_vp.bf.port_cvlan_untag_fltr_cmd = filter->ctag_untagged_filter;
		vlan_vp.bf.port_cvlan_pri_tag_fltr_cmd = filter->ctag_priority_filter;
		vlan_vp.bf.port_cvlan_tag_fltr_cmd = filter->ctag_tagged_filter;

		rtn = appe_vlan_port_vp_tbl_set(dev_id, port_value, &vlan_vp);
		SW_RTN_ON_ERROR(rtn);
	} else
#endif
	{
		rtn = hppe_port_vlan_config_port_in_vlan_fltr_cmd_set(dev_id,
				port_value, (a_uint32_t)filter->membership_filter);
		SW_RTN_ON_ERROR(rtn);
		rtn = hppe_port_vlan_config_port_untag_fltr_cmd_set(dev_id,
				port_value, (a_uint32_t)filter->untagged_filter);
		SW_RTN_ON_ERROR(rtn);
		rtn = hppe_port_vlan_config_port_tag_fltr_cmd_set(dev_id,
				port_value, (a_uint32_t)filter->tagged_filter);
		SW_RTN_ON_ERROR(rtn);
		rtn = hppe_port_vlan_config_port_pri_tag_fltr_cmd_set(dev_id,
				port_value, (a_uint32_t)filter->priority_filter);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_ingress_vlan_filter_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_ingress_vlan_filter_t *filter)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(filter);

#if defined(APPE)
	if (ADPT_IS_VPORT(port_id)) {
		union vlan_port_vp_tbl_u vlan_vp;
		rtn = appe_vlan_port_vp_tbl_get(dev_id, port_value, &vlan_vp);
		SW_RTN_ON_ERROR(rtn);

		filter->membership_filter = vlan_vp.bf.port_in_vlan_fltr_cmd;

		filter->untagged_filter = vlan_vp.bf.port_untag_fltr_cmd;
		filter->priority_filter = vlan_vp.bf.port_pri_tag_fltr_cmd;
		filter->tagged_filter = vlan_vp.bf.port_tag_fltr_cmd;

		/* filter packet according to ctag format when the ingress port is core port */
		filter->ctag_untagged_filter = vlan_vp.bf.port_cvlan_untag_fltr_cmd;
		filter->ctag_priority_filter = vlan_vp.bf.port_cvlan_pri_tag_fltr_cmd;
		filter->ctag_tagged_filter = vlan_vp.bf.port_cvlan_tag_fltr_cmd;
	} else
#endif
	{
		rtn = hppe_port_vlan_config_port_in_vlan_fltr_cmd_get(dev_id,
				port_value, (a_uint32_t *)&filter->membership_filter);
		SW_RTN_ON_ERROR(rtn);
		rtn = hppe_port_vlan_config_port_untag_fltr_cmd_get(dev_id,
				port_value, (a_uint32_t *)&filter->untagged_filter);
		SW_RTN_ON_ERROR(rtn);
		rtn = hppe_port_vlan_config_port_tag_fltr_cmd_get(dev_id,
				port_value, (a_uint32_t *)&filter->tagged_filter);
		SW_RTN_ON_ERROR(rtn);
		rtn = hppe_port_vlan_config_port_pri_tag_fltr_cmd_get(dev_id,
				port_value, (a_uint32_t *)&filter->priority_filter);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_invlan_mode_set(a_uint32_t dev_id, fal_port_t port_id, fal_pt_invlan_mode_t mode)
{
	sw_error_t rtn = SW_OK;
	fal_ingress_vlan_filter_t filter;

	ADPT_DEV_ID_CHECK(dev_id);

	if (mode == FAL_INVLAN_ADMIT_ALL)
	{
		filter.tagged_filter = A_FALSE;
		filter.untagged_filter = A_FALSE;
		filter.priority_filter = A_FALSE;
	}
	else if (mode == FAL_INVLAN_ADMIT_TAGGED)
	{
		filter.tagged_filter = A_FALSE;
		filter.untagged_filter = A_TRUE;
		filter.priority_filter = A_TRUE;
	}
	else if (mode == FAL_INVLAN_ADMIT_UNTAGGED)
	{
		filter.tagged_filter = A_TRUE;
		filter.untagged_filter = A_FALSE;
		filter.priority_filter = A_FALSE;
	}
	else
		return SW_FAIL;

	adpt_hppe_port_ingress_vlan_filter_set(dev_id, port_id, &filter);

	return rtn;
}

sw_error_t
adpt_hppe_port_invlan_mode_get(a_uint32_t dev_id, fal_port_t port_id, fal_pt_invlan_mode_t * mode)
{
	sw_error_t rtn = SW_OK;
	fal_ingress_vlan_filter_t filter;

	ADPT_DEV_ID_CHECK(dev_id);

	adpt_hppe_port_ingress_vlan_filter_get(dev_id, port_id, &filter);

	if (filter.tagged_filter == A_FALSE && filter.untagged_filter == A_FALSE && 
		filter.priority_filter  == A_FALSE)
		*mode = FAL_INVLAN_ADMIT_ALL;
	else if (filter.tagged_filter == A_FALSE && filter.untagged_filter == A_TRUE &&
		filter.priority_filter  == A_TRUE)
		*mode = FAL_INVLAN_ADMIT_TAGGED;
	else if (filter.tagged_filter == A_TRUE && filter.untagged_filter == A_FALSE &&
		filter.priority_filter  == A_FALSE)
		*mode = FAL_INVLAN_ADMIT_UNTAGGED;
	else
		return SW_FAIL;

	return rtn;
}

#ifndef IN_PORTVLAN_MINI
a_uint32_t
_get_port_vlan_ingress_trans_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_vlan_trans_entry_t *entry)
{
	union xlt_rule_tbl_u in_vlan_xlt_rule;
	union xlt_action_tbl_u in_vlan_xlt_action;
	a_uint32_t rtn = SW_OK;

	/*rule part*/
	rtn = hppe_xlt_rule_tbl_get(dev_id, index, &in_vlan_xlt_rule);
	SW_RTN_ON_ERROR(rtn);

	if (!in_vlan_xlt_rule.bf.valid) {
		memset(&in_vlan_xlt_rule, 0, sizeof(in_vlan_xlt_rule));
	}

	entry->trans_direction = 0;
	entry->frmtype_enable = in_vlan_xlt_rule.bf.frm_type_incl;
	entry->frmtype = in_vlan_xlt_rule.bf.frm_type;
	entry->protocol_enable = in_vlan_xlt_rule.bf.prot_incl;
	entry->protocol = ((in_vlan_xlt_rule.bf.prot_value_1 << 7) |
			(in_vlan_xlt_rule.bf.prot_value_0));

	entry->port_bitmap = in_vlan_xlt_rule.bf.port_bitmap;
	entry->c_tagged = in_vlan_xlt_rule.bf.ckey_fmt_0 | (in_vlan_xlt_rule.bf.ckey_fmt_1 << 1);
	entry->s_tagged = in_vlan_xlt_rule.bf.skey_fmt;

	entry->c_vid_enable = in_vlan_xlt_rule.bf.ckey_vid_incl;
	entry->c_vid = in_vlan_xlt_rule.bf.ckey_vid;
	entry->c_pcp_enable = in_vlan_xlt_rule.bf.ckey_pcp_incl;
	entry->c_pcp = in_vlan_xlt_rule.bf.ckey_pcp;
	entry->c_dei_enable = in_vlan_xlt_rule.bf.ckey_dei_incl;
	entry->c_dei = in_vlan_xlt_rule.bf.ckey_dei;

	entry->s_vid_enable = in_vlan_xlt_rule.bf.skey_vid_incl;
	entry->s_vid = in_vlan_xlt_rule.bf.skey_vid;
	entry->s_pcp_enable = in_vlan_xlt_rule.bf.skey_pcp_incl;
	entry->s_pcp = in_vlan_xlt_rule.bf.skey_pcp;
	entry->s_dei_enable = in_vlan_xlt_rule.bf.skey_dei_incl;
	entry->s_dei = in_vlan_xlt_rule.bf.skey_dei;

	/*action part*/
	rtn = hppe_xlt_action_tbl_get(dev_id, index, &in_vlan_xlt_action);
	SW_RTN_ON_ERROR(rtn);

	if (!in_vlan_xlt_rule.bf.valid) {
		memset(&in_vlan_xlt_action, 0, sizeof(struct xlt_action_tbl));
	}

	entry->counter_enable = in_vlan_xlt_action.bf.counter_en;
#if defined(APPE)
	entry->counter_id = in_vlan_xlt_action.bf.counter_id_0 | \
			    in_vlan_xlt_action.bf.counter_id_1 << 3;
#else
	entry->counter_id = in_vlan_xlt_action.bf.counter_id;
#endif
	entry->vsi_action_enable = in_vlan_xlt_action.bf.vsi_cmd;
	entry->vsi_action = in_vlan_xlt_action.bf.vsi;

	entry->cdei_xlt_enable = in_vlan_xlt_action.bf.xlt_cdei_cmd;
	entry->cdei_xlt = in_vlan_xlt_action.bf.xlt_cdei;
	entry->sdei_xlt_enable = in_vlan_xlt_action.bf.xlt_sdei_cmd;
	entry->sdei_xlt = in_vlan_xlt_action.bf.xlt_sdei;
	entry->swap_sdei_cdei = in_vlan_xlt_action.bf.dei_swap_cmd;

	entry->cpcp_xlt_enable = in_vlan_xlt_action.bf.xlt_cpcp_cmd;
	entry->cpcp_xlt = in_vlan_xlt_action.bf.xlt_cpcp;
	entry->spcp_xlt_enable = in_vlan_xlt_action.bf.xlt_spcp_cmd;
	entry->spcp_xlt = (in_vlan_xlt_action.bf.xlt_spcp_0 |
			(in_vlan_xlt_action.bf.xlt_spcp_1 << 1));
	entry->swap_spcp_cpcp = in_vlan_xlt_action.bf.pcp_swap_cmd;

	entry->cvid_xlt_cmd = in_vlan_xlt_action.bf.xlt_cvid_cmd;
	entry->cvid_xlt = in_vlan_xlt_action.bf.xlt_cvid;
	entry->svid_xlt_cmd = in_vlan_xlt_action.bf.xlt_svid_cmd;
	entry->svid_xlt = in_vlan_xlt_action.bf.xlt_svid;
	entry->swap_svid_cvid = in_vlan_xlt_action.bf.vid_swap_cmd;

	return in_vlan_xlt_rule.bf.valid;
}

a_uint32_t
_get_port_vlan_egress_trans_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_vlan_trans_entry_t *entry)
{
	union eg_vlan_xlt_rule_u eg_vlan_xlt_rule;
	union eg_vlan_xlt_action_u eg_vlan_xlt_action;
	a_uint32_t rtn = SW_OK;

	/*rule part*/
	rtn = hppe_eg_vlan_xlt_rule_get(dev_id, index, &eg_vlan_xlt_rule);
	SW_RTN_ON_ERROR(rtn);

	if (!eg_vlan_xlt_rule.bf.valid) {
		memset(&eg_vlan_xlt_rule, 0, sizeof(struct eg_vlan_xlt_rule));
	}

	entry->trans_direction = 1;

	entry->port_bitmap = eg_vlan_xlt_rule.bf.port_bitmap;

	entry->vsi_enable = eg_vlan_xlt_rule.bf.vsi_incl;
	entry->vsi = eg_vlan_xlt_rule.bf.vsi;
	entry->vsi_valid = eg_vlan_xlt_rule.bf.vsi_valid;

	entry->c_tagged = eg_vlan_xlt_rule.bf.ckey_fmt;
	entry->s_tagged = eg_vlan_xlt_rule.bf.skey_fmt;

	entry->c_vid_enable = eg_vlan_xlt_rule.bf.ckey_vid_incl;
	entry->c_vid = eg_vlan_xlt_rule.bf.ckey_vid;
	entry->c_pcp_enable = eg_vlan_xlt_rule.bf.ckey_pcp_incl;
	entry->c_pcp = eg_vlan_xlt_rule.bf.ckey_pcp;
	entry->c_dei_enable = eg_vlan_xlt_rule.bf.ckey_dei_incl;
	entry->c_dei = eg_vlan_xlt_rule.bf.ckey_dei;

	entry->s_vid_enable = eg_vlan_xlt_rule.bf.skey_vid_incl;
#if defined(APPE)
	entry->s_vid = eg_vlan_xlt_rule.bf.skey_vid_0 | eg_vlan_xlt_rule.bf.skey_vid_1 << 11;
#else
	entry->s_vid = eg_vlan_xlt_rule.bf.skey_vid;
#endif
	entry->s_pcp_enable = eg_vlan_xlt_rule.bf.skey_pcp_incl;
	entry->s_pcp = eg_vlan_xlt_rule.bf.skey_pcp;
	entry->s_dei_enable = eg_vlan_xlt_rule.bf.skey_dei_incl;
	entry->s_dei = eg_vlan_xlt_rule.bf.skey_dei;

	/*action part*/
	rtn = hppe_eg_vlan_xlt_action_get(dev_id, index, &eg_vlan_xlt_action);
	SW_RTN_ON_ERROR(rtn);

	if (!eg_vlan_xlt_rule.bf.valid) {
		memset(&eg_vlan_xlt_action, 0, sizeof(struct eg_vlan_xlt_action));
	}

	entry->counter_enable = eg_vlan_xlt_action.bf.counter_en;
	entry->counter_id = eg_vlan_xlt_action.bf.counter_id;

	entry->cdei_xlt_enable = eg_vlan_xlt_action.bf.xlt_cdei_cmd;
	entry->cdei_xlt = eg_vlan_xlt_action.bf.xlt_cdei;
	entry->sdei_xlt_enable = eg_vlan_xlt_action.bf.xlt_sdei_cmd;
	entry->sdei_xlt = eg_vlan_xlt_action.bf.xlt_sdei;
	entry->swap_sdei_cdei = eg_vlan_xlt_action.bf.dei_swap_cmd;

	entry->cpcp_xlt_enable = eg_vlan_xlt_action.bf.xlt_cpcp_cmd;
	entry->cpcp_xlt = eg_vlan_xlt_action.bf.xlt_cpcp;
	entry->spcp_xlt_enable = eg_vlan_xlt_action.bf.xlt_spcp_cmd;
	entry->spcp_xlt = (eg_vlan_xlt_action.bf.xlt_spcp_0 |
			(eg_vlan_xlt_action.bf.xlt_spcp_1 << 1));
	entry->swap_spcp_cpcp = eg_vlan_xlt_action.bf.pcp_swap_cmd;

	entry->cvid_xlt_cmd = eg_vlan_xlt_action.bf.xlt_cvid_cmd;
	entry->cvid_xlt = eg_vlan_xlt_action.bf.xlt_cvid;
	entry->svid_xlt_cmd = eg_vlan_xlt_action.bf.xlt_svid_cmd;
	entry->svid_xlt = eg_vlan_xlt_action.bf.xlt_svid;
	entry->swap_svid_cvid = eg_vlan_xlt_action.bf.vid_swap_cmd;

	return eg_vlan_xlt_rule.bf.valid;
}

static inline sw_error_t
adpt_portvlan_tag_propagation_convert(fal_vlan_propagation_mode_t *propagation_mode,
		a_uint32_t *value, a_bool_t to_hsl)
{
	sw_error_t rv = SW_OK;
	if (to_hsl) {
		switch (*propagation_mode) {
			case FAL_VLAN_PROPAGATION_DISABLE:
				*value = 0;
				break;
			case FAL_VLAN_PROPAGATION_CLONE:
				*value = 1;
				break;
			case FAL_VLAN_PROPAGATION_REPLACE:
			case FAL_VLAN_PROPAGATION_MODE_BUTT:
			default:
				SSDK_ERROR("Unsupported fal propagation mode to convert\n");
				rv = SW_FAIL;
				break;
		}
	} else {
		switch (*value) {
			case 0:
				*propagation_mode = FAL_VLAN_PROPAGATION_DISABLE;
				break;
			case 1:
				*propagation_mode = FAL_VLAN_PROPAGATION_CLONE;
				break;
			default:
				SSDK_ERROR("Unsupported hsl propagation mode to convert\n");
				rv = SW_FAIL;
				break;
		}
	}
	return rv;
}

sw_error_t
adpt_hppe_port_tag_propagation_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_vlantag_propagation_t *prop)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value_pcp, value_dei = 0;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);

	rtn = adpt_portvlan_tag_propagation_convert(&prop->pri_propagation, &value_pcp, A_TRUE);
	SW_RTN_ON_ERROR(rtn);

	rtn = adpt_portvlan_tag_propagation_convert(&prop->dei_propagation, &value_dei, A_TRUE);
	SW_RTN_ON_ERROR(rtn);

	if (!FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_PCP_EN | FAL_PORT_PROPAGATION_DEI_EN)) {
		return rtn;
	}

	if (direction == FAL_PORT_VLAN_ALL || direction == FAL_PORT_VLAN_INGRESS)
	{
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union vlan_port_vp_tbl_u vlan_vp;
			rtn = appe_vlan_port_vp_tbl_get(dev_id, port_value, &vlan_vp);
			SW_RTN_ON_ERROR(rtn);

			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_PCP_EN)) {
				vlan_vp.bf.port_in_pcp_prop_cmd = value_pcp;
			}
			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_DEI_EN)) {
				vlan_vp.bf.port_in_dei_prop_cmd = value_dei;
			}
			rtn = appe_vlan_port_vp_tbl_set(dev_id, port_value, &vlan_vp);
			SW_RTN_ON_ERROR(rtn);
		} else
#endif
		{
			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_PCP_EN)) {
				rtn = hppe_port_vlan_config_port_in_pcp_prop_cmd_set(dev_id,
						port_value, value_pcp);
				SW_RTN_ON_ERROR(rtn);
			}
			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_DEI_EN)) {
				rtn = hppe_port_vlan_config_port_in_dei_prop_cmd_set(dev_id,
						port_value, value_dei);
				SW_RTN_ON_ERROR(rtn);
			}
		}
	}

	if (direction == FAL_PORT_VLAN_ALL || direction == FAL_PORT_VLAN_EGRESS)
	{
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union eg_vp_tbl_u eg_vlan_vp;
			rtn = appe_egress_vp_tbl_get(dev_id, port_value, &eg_vlan_vp);
			SW_RTN_ON_ERROR(rtn);

			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_PCP_EN)) {
				eg_vlan_vp.bf.port_eg_pcp_prop_cmd = value_pcp;
			}
			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_DEI_EN)) {
				eg_vlan_vp.bf.port_eg_dei_prop_cmd = value_dei;
			}
			rtn = appe_egress_vp_tbl_set(dev_id, port_value, &eg_vlan_vp);
			SW_RTN_ON_ERROR(rtn);
		} else
#endif
		{
			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_PCP_EN)) {
				rtn = hppe_port_eg_vlan_port_eg_pcp_prop_cmd_set(dev_id,
						port_value, value_pcp);
				SW_RTN_ON_ERROR(rtn);
			}
			if (FAL_FLG_TST(prop->mask, FAL_PORT_PROPAGATION_DEI_EN)) {
				rtn = hppe_port_eg_vlan_port_eg_dei_prop_cmd_set(dev_id,
						port_value, value_dei);
				SW_RTN_ON_ERROR(rtn);
			}
		}
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_tag_propagation_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_port_vlan_direction_t direction,
		fal_vlantag_propagation_t *prop)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t value_pcp = 0, value_dei = 0;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(prop);

	if (direction == FAL_PORT_VLAN_ALL)
		return SW_NOT_SUPPORTED;

	if (direction == FAL_PORT_VLAN_INGRESS) {
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union vlan_port_vp_tbl_u vlan_vp;
			rtn = appe_vlan_port_vp_tbl_get(dev_id, port_value, &vlan_vp);
			SW_RTN_ON_ERROR(rtn);

			value_pcp = vlan_vp.bf.port_in_pcp_prop_cmd;
			value_dei = vlan_vp.bf.port_in_dei_prop_cmd;
		} else
#endif
		{
			rtn = hppe_port_vlan_config_port_in_pcp_prop_cmd_get(dev_id,
					port_value, &value_pcp);
			SW_RTN_ON_ERROR(rtn);

			rtn = hppe_port_vlan_config_port_in_dei_prop_cmd_get(dev_id,
					port_value, &value_dei);
		}
	}
	else if (direction == FAL_PORT_VLAN_EGRESS) {
#if defined(APPE)
		if (ADPT_IS_VPORT(port_id)) {
			union eg_vp_tbl_u eg_vlan_vp;
			rtn = appe_egress_vp_tbl_get(dev_id, port_value, &eg_vlan_vp);
			SW_RTN_ON_ERROR(rtn);

			value_pcp = eg_vlan_vp.bf.port_eg_pcp_prop_cmd;
			value_dei = eg_vlan_vp.bf.port_eg_dei_prop_cmd;
		} else
#endif
		{
			rtn = hppe_port_eg_vlan_port_eg_pcp_prop_cmd_get(dev_id,
					port_value, &value_pcp);
			SW_RTN_ON_ERROR(rtn);

			rtn = hppe_port_eg_vlan_port_eg_dei_prop_cmd_get(dev_id,
					port_value, &value_dei);
			SW_RTN_ON_ERROR(rtn);
		}
	}
	else
		return SW_NOT_SUPPORTED;

	rtn = adpt_portvlan_tag_propagation_convert(&prop->pri_propagation, &value_pcp, A_FALSE);
	SW_RTN_ON_ERROR(rtn);

	rtn = adpt_portvlan_tag_propagation_convert(&prop->dei_propagation, &value_dei, A_FALSE);
	SW_RTN_ON_ERROR(rtn);

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_trans_iterate(a_uint32_t dev_id, fal_port_t port_id,
                                a_uint32_t * iterator, fal_vlan_trans_entry_t *entry)
{
	a_uint32_t idx, eg_tbl_num, rule_valid;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(iterator);
	ADPT_NULL_POINT_CHECK(entry);

	eg_tbl_num = XLT_RULE_TBL_NUM * 2;

	if (*iterator < XLT_RULE_TBL_NUM) {
		for (idx = *iterator; idx < XLT_RULE_TBL_NUM; idx++) {
			aos_mem_zero(entry, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_ingress_trans_by_index(dev_id, idx, entry);
			if (rule_valid == 1) {
				if (SW_IS_PBMP_MEMBER(entry->port_bitmap, port_id))
					break;
			}
		}

		if (idx == XLT_RULE_TBL_NUM)
			return SW_NO_MORE;
	}
	else if (*iterator < eg_tbl_num) {
		for (idx = *iterator; idx < eg_tbl_num; idx++) {
			aos_mem_zero(entry, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_egress_trans_by_index(dev_id,
					idx - XLT_RULE_TBL_NUM, entry);
			if (rule_valid == 1) {
				if (SW_IS_PBMP_MEMBER(entry->port_bitmap, port_id))
					break;
			}
		}

		if (idx == eg_tbl_num)
			return SW_NO_MORE;
	}
	else {
		return SW_OUT_OF_RANGE;
	}

	*iterator = idx + 1;

	return SW_OK;
}

sw_error_t
adpt_hppe_port_vlan_trans_add(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry)
{
	sw_error_t rtn = SW_OK;
	union xlt_rule_tbl_u in_vlan_xlt_rule;
	union eg_vlan_xlt_rule_u eg_vlan_xlt_rule;
	union xlt_action_tbl_u in_vlan_xlt_action;
	union eg_vlan_xlt_action_u eg_vlan_xlt_action;
	a_uint32_t idx, entry_idx, entry_sign, rule_valid;
	fal_vlan_trans_entry_t temp;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	entry_idx = 0;

	if (entry->trans_direction == 0) {
		/*rule part*/
		entry_sign = 0;
		for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
			aos_mem_zero(&temp, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_ingress_trans_by_index(dev_id, idx, &temp);
			if (rule_valid == 1) {
				if (!aos_mem_cmp(entry, &temp, sizeof (fal_vlan_trans_entry_t))) {
					if (SW_IS_PBMP_MEMBER(temp.port_bitmap, port_id))
						return SW_ALREADY_EXIST;
					entry_idx = idx;
					entry_sign = 1;
					break;
				}
			}
			else {
				if (entry_sign == 0) {
					entry_idx = idx;
					entry_sign = 1;
				}
			}
		}

		if (entry_sign == 0)
			return SW_NO_RESOURCE;

#if defined(APPE)
		/* only the physical port for the legacy api */
		in_vlan_xlt_rule.bf.port_type = FAL_PORT_TYPE_PPORT;
		in_vlan_xlt_rule.bf.port_bitmap = SW_PBMP_ADD_PORT(entry->port_bitmap, port_id);
#else
		in_vlan_xlt_rule.bf.port_bitmap = SW_PBMP_ADD_PORT(entry->port_bitmap, port_id);
#endif
		in_vlan_xlt_rule.bf.valid = A_TRUE;
		in_vlan_xlt_rule.bf.prot_incl = entry->protocol_enable;
		in_vlan_xlt_rule.bf.prot_value_0 = (entry->protocol  & 0x7f);
		in_vlan_xlt_rule.bf.prot_value_1 = (entry->protocol >> 7);
		in_vlan_xlt_rule.bf.frm_type_incl = entry->frmtype_enable;
		in_vlan_xlt_rule.bf.frm_type = entry->frmtype;

		in_vlan_xlt_rule.bf.ckey_fmt_0 = (entry->c_tagged & 0x1);
		in_vlan_xlt_rule.bf.ckey_fmt_1 = (entry->c_tagged >> 1);
		in_vlan_xlt_rule.bf.skey_fmt = entry->s_tagged;

		in_vlan_xlt_rule.bf.ckey_vid_incl = entry->c_vid_enable;
		in_vlan_xlt_rule.bf.ckey_vid = entry->c_vid;
		in_vlan_xlt_rule.bf.ckey_pcp_incl = entry->c_pcp_enable;
		in_vlan_xlt_rule.bf.ckey_pcp = entry->c_pcp;
		in_vlan_xlt_rule.bf.ckey_dei_incl = entry->c_dei_enable;
		in_vlan_xlt_rule.bf.ckey_dei = entry->c_dei;

		in_vlan_xlt_rule.bf.skey_vid_incl = entry->s_vid_enable;
		in_vlan_xlt_rule.bf.skey_vid = entry->s_vid;
		in_vlan_xlt_rule.bf.skey_pcp_incl = entry->s_pcp_enable;
		in_vlan_xlt_rule.bf.skey_pcp = entry->s_pcp;
		in_vlan_xlt_rule.bf.skey_dei_incl = entry->s_dei_enable;
		in_vlan_xlt_rule.bf.skey_dei = entry->s_dei;

		rtn = hppe_xlt_rule_tbl_set(dev_id, entry_idx, &in_vlan_xlt_rule);
		SW_RTN_ON_ERROR(rtn);

		/*action part*/
		in_vlan_xlt_action.bf.counter_en = entry->counter_enable;
#if defined(APPE)
		in_vlan_xlt_action.bf.counter_id_0 = entry->counter_id & 0x7;
		in_vlan_xlt_action.bf.counter_id_1 = (entry->counter_id >> 3) & 0xf;
#else
		in_vlan_xlt_action.bf.counter_id = entry->counter_id;
#endif
		in_vlan_xlt_action.bf.vsi_cmd = entry->vsi_action_enable;
		in_vlan_xlt_action.bf.vsi = entry->vsi_action;

		in_vlan_xlt_action.bf.xlt_cdei_cmd = entry->cdei_xlt_enable;
		in_vlan_xlt_action.bf.xlt_cdei = entry->cdei_xlt;
		in_vlan_xlt_action.bf.xlt_sdei_cmd = entry->sdei_xlt_enable;
		in_vlan_xlt_action.bf.xlt_sdei = entry->sdei_xlt;
		in_vlan_xlt_action.bf.dei_swap_cmd = entry->swap_sdei_cdei;

		in_vlan_xlt_action.bf.xlt_cpcp_cmd = entry->cpcp_xlt_enable;
		in_vlan_xlt_action.bf.xlt_cpcp = entry->cpcp_xlt;
		in_vlan_xlt_action.bf.xlt_spcp_cmd = entry->spcp_xlt_enable;
		in_vlan_xlt_action.bf.xlt_spcp_0 = (entry->spcp_xlt & 0x1);
		in_vlan_xlt_action.bf.xlt_spcp_1 = (entry->spcp_xlt >> 1);
		in_vlan_xlt_action.bf.pcp_swap_cmd = entry->swap_spcp_cpcp;

		in_vlan_xlt_action.bf.xlt_cvid_cmd = entry->cvid_xlt_cmd;
		in_vlan_xlt_action.bf.xlt_cvid = entry->cvid_xlt;
		in_vlan_xlt_action.bf.xlt_svid_cmd = entry->svid_xlt_cmd;
		in_vlan_xlt_action.bf.xlt_svid = entry->svid_xlt;
		in_vlan_xlt_action.bf.vid_swap_cmd = entry->swap_svid_cvid;

		rtn = hppe_xlt_action_tbl_set(dev_id, entry_idx, &in_vlan_xlt_action);
		SW_RTN_ON_ERROR(rtn);
	}
	else {
		/*rule part*/
		entry_sign = 0;
		for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
			aos_mem_zero(&temp, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_egress_trans_by_index(dev_id, idx, &temp);
			if (rule_valid == 1) {
				if (!aos_mem_cmp(entry, &temp, sizeof (fal_vlan_trans_entry_t))) {
					if (SW_IS_PBMP_MEMBER(temp.port_bitmap, port_id))
						return SW_ALREADY_EXIST;
					entry_idx = idx;
					entry_sign = 1;
					break;
				}
			}
			else {
				if (entry_sign == 0) {
					entry_idx = idx;
					entry_sign = 1;
				}
			}
		}

		if (entry_sign == 0)
			return SW_NO_RESOURCE;

		eg_vlan_xlt_rule.bf.valid = A_TRUE;
		eg_vlan_xlt_rule.bf.port_bitmap = SW_PBMP_ADD_PORT(entry->port_bitmap, port_id);

		eg_vlan_xlt_rule.bf.vsi_incl = entry->vsi_enable;
		eg_vlan_xlt_rule.bf.vsi = entry->vsi;
		eg_vlan_xlt_rule.bf.vsi_valid = entry->vsi_valid;

		eg_vlan_xlt_rule.bf.ckey_fmt = entry->c_tagged;
		eg_vlan_xlt_rule.bf.skey_fmt = entry->s_tagged;

		eg_vlan_xlt_rule.bf.ckey_vid_incl = entry->c_vid_enable;
		eg_vlan_xlt_rule.bf.ckey_vid = entry->c_vid;
		eg_vlan_xlt_rule.bf.ckey_pcp_incl = entry->c_pcp_enable;
		eg_vlan_xlt_rule.bf.ckey_pcp = entry->c_pcp;
		eg_vlan_xlt_rule.bf.ckey_dei_incl = entry->c_dei_enable;
		eg_vlan_xlt_rule.bf.ckey_dei = entry->c_dei;

		eg_vlan_xlt_rule.bf.skey_vid_incl = entry->s_vid_enable;
#if defined(APPE)
		eg_vlan_xlt_rule.bf.skey_vid_0 = entry->s_vid & 0x7ff;
		eg_vlan_xlt_rule.bf.skey_vid_1 = (entry->s_vid >> 11) & 0x1;
#else
		eg_vlan_xlt_rule.bf.skey_vid = entry->s_vid;
#endif
		eg_vlan_xlt_rule.bf.skey_pcp_incl = entry->s_pcp_enable;
		eg_vlan_xlt_rule.bf.skey_pcp = entry->s_pcp;
		eg_vlan_xlt_rule.bf.skey_dei_incl = entry->s_dei_enable;
		eg_vlan_xlt_rule.bf.skey_dei = entry->s_dei;

		rtn = hppe_eg_vlan_xlt_rule_set(dev_id, entry_idx, &eg_vlan_xlt_rule);
		SW_RTN_ON_ERROR(rtn);

		/*action part*/
		eg_vlan_xlt_action.bf.counter_en = entry->counter_enable;
		eg_vlan_xlt_action.bf.counter_id = entry->counter_id;

		eg_vlan_xlt_action.bf.xlt_cdei_cmd = entry->cdei_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_cdei = entry->cdei_xlt;
		eg_vlan_xlt_action.bf.xlt_sdei_cmd = entry->sdei_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_sdei = entry->sdei_xlt;
		eg_vlan_xlt_action.bf.dei_swap_cmd = entry->swap_sdei_cdei;

		eg_vlan_xlt_action.bf.xlt_cpcp_cmd = entry->cpcp_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_cpcp = entry->cpcp_xlt;
		eg_vlan_xlt_action.bf.xlt_spcp_cmd = entry->spcp_xlt_enable;
		eg_vlan_xlt_action.bf.xlt_spcp_0 = (entry->spcp_xlt & 0x1);
		eg_vlan_xlt_action.bf.xlt_spcp_1 = (entry->spcp_xlt >> 1);
		eg_vlan_xlt_action.bf.pcp_swap_cmd = entry->swap_spcp_cpcp;

		eg_vlan_xlt_action.bf.xlt_cvid_cmd = entry->cvid_xlt_cmd;
		eg_vlan_xlt_action.bf.xlt_cvid = entry->cvid_xlt;
		eg_vlan_xlt_action.bf.xlt_svid_cmd = entry->svid_xlt_cmd;
		eg_vlan_xlt_action.bf.xlt_svid = entry->svid_xlt;
		eg_vlan_xlt_action.bf.vid_swap_cmd = entry->swap_svid_cvid;

		rtn = hppe_eg_vlan_xlt_action_set(dev_id, entry_idx, &eg_vlan_xlt_action);
		SW_RTN_ON_ERROR(rtn);
	}

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_trans_get(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry)
{
	a_uint32_t idx, rule_valid;
	fal_vlan_trans_entry_t temp;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	if (entry->trans_direction == 0) {
		for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
			aos_mem_zero(&temp, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_ingress_trans_by_index(dev_id, idx, &temp);
			if (rule_valid == 1) {
				if (!aos_mem_cmp(entry, &temp, sizeof(fal_vlan_trans_entry_t))) {
					if (SW_IS_PBMP_MEMBER(temp.port_bitmap, port_id)) {
						aos_mem_copy(entry, &temp,
								sizeof(fal_vlan_trans_entry_t));
						return SW_OK;
					}
				}
			}
		}
	}
	else {
		for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
			aos_mem_zero(&temp, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_egress_trans_by_index(dev_id, idx, &temp);
			if (rule_valid == 1) {
				if (!aos_mem_cmp(entry, &temp, sizeof(fal_vlan_trans_entry_t))) {
					if (SW_IS_PBMP_MEMBER(temp.port_bitmap, port_id)) {
						aos_mem_copy(entry, &temp,
								sizeof(fal_vlan_trans_entry_t));
						return SW_OK;
					}
				}
			}
		}
	}

	return SW_NOT_FOUND;
}

sw_error_t
adpt_hppe_port_vlan_trans_del(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry)
{
	a_uint32_t idx, rule_valid;
	fal_vlan_trans_entry_t temp;

	union xlt_rule_tbl_u in_vlan_xlt_rule;
	union eg_vlan_xlt_rule_u eg_vlan_xlt_rule;
	union xlt_action_tbl_u in_vlan_xlt_action;
	union eg_vlan_xlt_action_u eg_vlan_xlt_action;
	sw_error_t rtn = SW_OK;

	memset(&in_vlan_xlt_rule, 0, sizeof(in_vlan_xlt_rule));
	memset(&eg_vlan_xlt_rule, 0, sizeof(struct eg_vlan_xlt_rule));
	memset(&in_vlan_xlt_action, 0, sizeof(struct xlt_action_tbl));
	memset(&eg_vlan_xlt_action, 0, sizeof(struct eg_vlan_xlt_action));

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	if (entry->trans_direction == 0) {
		for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
			aos_mem_zero(&temp, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_ingress_trans_by_index(dev_id, idx, &temp);
			if (rule_valid == 1) {
				if (!aos_mem_cmp(entry, &temp, sizeof (fal_vlan_trans_entry_t))) {
					if (SW_IS_PBMP_MEMBER(temp.port_bitmap, port_id)) {
						rtn = hppe_xlt_rule_tbl_set(dev_id,
								idx, &in_vlan_xlt_rule);
						SW_RTN_ON_ERROR(rtn);
						rtn = hppe_xlt_action_tbl_set(dev_id,
								idx, &in_vlan_xlt_action);
						return rtn;
					}
				}
			}
		}
	}
	else {
		for (idx = 0; idx < XLT_RULE_TBL_NUM; idx++) {
			aos_mem_zero(&temp, sizeof (fal_vlan_trans_entry_t));
			rule_valid = _get_port_vlan_egress_trans_by_index(dev_id, idx, &temp);
			if (rule_valid == 1) {
				if (!aos_mem_cmp(entry, &temp, sizeof (fal_vlan_trans_entry_t))) {
					if (SW_IS_PBMP_MEMBER(temp.port_bitmap, port_id)) {
						rtn = hppe_eg_vlan_xlt_rule_set(dev_id,
								idx, &eg_vlan_xlt_rule);
						SW_RTN_ON_ERROR(rtn);
						rtn = hppe_eg_vlan_xlt_action_set(dev_id,
								idx, &eg_vlan_xlt_action);
						return rtn;
					}
				}
			}
		}
	}

	return SW_NOT_FOUND;
}

sw_error_t
adpt_hppe_port_vlan_counter_get(a_uint32_t dev_id,
		a_uint32_t cnt_index, fal_port_vlan_counter_t * counter)
{
	union vlan_dev_cnt_tbl_u vlan_dev_cnt_tbl;
	union vlan_dev_tx_counter_tbl_u vlan_dev_tx_counter_tbl;
	sw_error_t rtn = SW_OK;

	rtn = hppe_vlan_dev_cnt_tbl_get(dev_id, cnt_index, &vlan_dev_cnt_tbl);
	SW_RTN_ON_ERROR(rtn);

	rtn = hppe_vlan_dev_tx_counter_tbl_get(dev_id, cnt_index, &vlan_dev_tx_counter_tbl);
	SW_RTN_ON_ERROR(rtn);

	counter->rx_packet_counter = vlan_dev_cnt_tbl.bf.rx_pkt_cnt;
	counter->rx_byte_counter = ((a_uint64_t)vlan_dev_cnt_tbl.bf.rx_byte_cnt_1 << 32) |
		vlan_dev_cnt_tbl.bf.rx_byte_cnt_0;
	counter->tx_packet_counter = vlan_dev_tx_counter_tbl.bf.tx_pkt_cnt;
	counter->tx_byte_counter = ((a_uint64_t)vlan_dev_tx_counter_tbl.bf.tx_byte_cnt_1 << 32) |
		vlan_dev_tx_counter_tbl.bf.tx_byte_cnt_0;

	return rtn;
}

sw_error_t
adpt_hppe_port_vlan_counter_cleanup(a_uint32_t dev_id, a_uint32_t cnt_index)
{
	union vlan_dev_cnt_tbl_u vlan_dev_cnt_tbl;
	union vlan_dev_tx_counter_tbl_u vlan_dev_tx_counter_tbl;
	sw_error_t rtn = SW_OK;

	memset(&vlan_dev_cnt_tbl, 0, sizeof(union vlan_dev_cnt_tbl_u));
	memset(&vlan_dev_tx_counter_tbl, 0, sizeof(union vlan_dev_tx_counter_tbl_u));

	rtn = hppe_vlan_dev_cnt_tbl_set(dev_id, cnt_index, &vlan_dev_cnt_tbl);
	SW_RTN_ON_ERROR(rtn);

	rtn = hppe_vlan_dev_tx_counter_tbl_set(dev_id, cnt_index, &vlan_dev_tx_counter_tbl);

	return rtn;
}
#endif

sw_error_t adpt_hppe_portvlan_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

#ifndef IN_PORTVLAN_MINI
		p_adpt_api->adpt_port_tag_propagation_set = adpt_hppe_port_tag_propagation_set;
		p_adpt_api->adpt_port_tag_propagation_get = adpt_hppe_port_tag_propagation_get;
		p_adpt_api->adpt_port_vlan_trans_iterate = adpt_hppe_port_vlan_trans_iterate;
		p_adpt_api->adpt_port_vlan_trans_add = adpt_hppe_port_vlan_trans_add;
		p_adpt_api->adpt_port_vlan_trans_get = adpt_hppe_port_vlan_trans_get;
		p_adpt_api->adpt_port_vlan_trans_del = adpt_hppe_port_vlan_trans_del;
		p_adpt_api->adpt_port_vlan_counter_get = adpt_hppe_port_vlan_counter_get;
		p_adpt_api->adpt_port_vlan_counter_cleanup = adpt_hppe_port_vlan_counter_cleanup;
#if defined(APPE)
		p_adpt_api->adpt_port_vlan_vpgroup_set = adpt_appe_port_vlan_vpgroup_set;
		p_adpt_api->adpt_port_vlan_vpgroup_get = adpt_appe_port_vlan_vpgroup_get;
		p_adpt_api->adpt_portvlan_isol_set = adpt_appe_portvlan_isol_set;
		p_adpt_api->adpt_portvlan_isol_get = adpt_appe_portvlan_isol_get;
		p_adpt_api->adpt_portvlan_isol_group_set = adpt_appe_portvlan_isol_group_set;
		p_adpt_api->adpt_portvlan_isol_group_get = adpt_appe_portvlan_isol_group_get;
		p_adpt_api->adpt_port_egress_vlan_filter_set =
			adpt_appe_port_egress_vlan_filter_set;
		p_adpt_api->adpt_port_egress_vlan_filter_get =
			adpt_appe_port_egress_vlan_filter_get;
#endif
#endif
		p_adpt_api->adpt_global_qinq_mode_set = adpt_hppe_global_qinq_mode_set;
		p_adpt_api->adpt_global_qinq_mode_get = adpt_hppe_global_qinq_mode_get;
		p_adpt_api->adpt_port_qinq_mode_set = adpt_hppe_port_qinq_mode_set;
		p_adpt_api->adpt_port_qinq_mode_get = adpt_hppe_port_qinq_mode_get;
		p_adpt_api->adpt_tpid_set = adpt_hppe_tpid_set;
		p_adpt_api->adpt_tpid_get = adpt_hppe_tpid_get;
		p_adpt_api->adpt_egress_tpid_set = adpt_hppe_egress_tpid_set;
		p_adpt_api->adpt_egress_tpid_get = adpt_hppe_egress_tpid_get;
		p_adpt_api->adpt_port_vlantag_egmode_set = adpt_hppe_port_vlantag_egmode_set;
		p_adpt_api->adpt_port_vlantag_egmode_get = adpt_hppe_port_vlantag_egmode_get;
		p_adpt_api->adpt_port_vlan_xlt_miss_cmd_set = adpt_hppe_port_vlan_xlt_miss_cmd_set;
		p_adpt_api->adpt_port_vlan_xlt_miss_cmd_get = adpt_hppe_port_vlan_xlt_miss_cmd_get;
		p_adpt_api->adpt_port_vsi_egmode_set = adpt_hppe_port_vsi_egmode_set;
		p_adpt_api->adpt_port_vsi_egmode_get = adpt_hppe_port_vsi_egmode_get;
		p_adpt_api->adpt_port_vlantag_vsi_egmode_enable_set =
			adpt_hppe_port_vlantag_vsi_egmode_enable_set;
		p_adpt_api->adpt_port_vlantag_vsi_egmode_enable_get =
			adpt_hppe_port_vlantag_vsi_egmode_enable_get;
		p_adpt_api->adpt_port_vlan_trans_adv_add = adpt_hppe_port_vlan_trans_adv_add;
		p_adpt_api->adpt_port_vlan_trans_adv_del = adpt_hppe_port_vlan_trans_adv_del;
		p_adpt_api->adpt_port_vlan_trans_adv_getfirst =
			adpt_hppe_port_vlan_trans_adv_getfirst;
		p_adpt_api->adpt_port_vlan_trans_adv_getnext =
			adpt_hppe_port_vlan_trans_adv_getnext;
		p_adpt_api->adpt_portvlan_member_update = adpt_hppe_portvlan_member_update;
		p_adpt_api->adpt_portvlan_member_get = adpt_hppe_portvlan_member_get;
		p_adpt_api->adpt_qinq_mode_set = adpt_hppe_qinq_mode_set;
		p_adpt_api->adpt_qinq_mode_get = adpt_hppe_qinq_mode_get;
		p_adpt_api->adpt_port_qinq_role_set = adpt_hppe_port_qinq_role_set;
		p_adpt_api->adpt_port_qinq_role_get = adpt_hppe_port_qinq_role_get;
		p_adpt_api->adpt_port_default_vlantag_set = adpt_hppe_port_default_vlantag_set;
		p_adpt_api->adpt_port_default_vlantag_get = adpt_hppe_port_default_vlantag_get;
		p_adpt_api->adpt_portvlan_member_add = adpt_hppe_portvlan_member_add;
		p_adpt_api->adpt_portvlan_member_del = adpt_hppe_portvlan_member_del;
		p_adpt_api->adpt_port_ingress_vlan_filter_set =
			adpt_hppe_port_ingress_vlan_filter_set;
		p_adpt_api->adpt_port_ingress_vlan_filter_get =
			adpt_hppe_port_ingress_vlan_filter_get;
		p_adpt_api->adpt_port_invlan_mode_set = adpt_hppe_port_invlan_mode_set;
		p_adpt_api->adpt_port_invlan_mode_get = adpt_hppe_port_invlan_mode_get;

	return SW_OK;
}

/**
 * @}
 */
