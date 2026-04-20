/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "adpt.h"
#include "adpt_hppe.h"
#include "appe_acl_reg.h"
#include "appe_acl.h"
#include "adpt_hppe_acl.h"
#include "adpt_appe_acl.h"
#include "hppe_ip_reg.h"
#include "hppe_ip.h"
#include "appe_tunnel_reg.h"
#include "appe_tunnel.h"

#define NON_IP_PROFILE_ID 7
#define IP4_PROFILE_ID 6
#define IP6_PROFILE_ID 5
#define ADPT_UDF_MAX_NUM 4

enum{
	APPE_ACL_POLICY_ROUTE = 0,
	APPE_ACL_POLICY_SNAT,
	APPE_ACL_POLICY_DNAT,
	APPE_ACL_POLICY_SNAPT,
	APPE_ACL_POLICY_DNAPT,
};

enum{
	APPE_PRE_ACL_TYPE_PORTBITMAP = 0,
	APPE_PRE_ACL_TYPE_PORT,
	APPE_PRE_ACL_TYPE_SERVICE_CODE,
	APPE_PRE_ACL_TYPE_TUNNEL_L3_IF,
	APPE_PRE_ACL_TYPE_VP_GROUP,
	APPE_PRE_ACL_TYPE_TUNNEL_PORT,
	APPE_PRE_ACL_TYPE_TUNNEL_VP_GROUP,
	APPE_PRE_ACL_TYPE_SERVICE_PORTBITMAP,
	APPE_PRE_ACL_TYPE_INVALID,
};

sw_error_t
_adpt_appe_pre_acl_tunnel_info_fields_check(a_uint32_t dev_id, a_uint32_t rule_id,
		a_uint32_t rule_nr, fal_acl_tunnel_info_t * rule, a_uint32_t *rule_type_map,
		a_uint32_t *tunnel_inverse_rule_type_count)
{
	SSDK_DEBUG("tunnel_info_fields 0x%x-0x%x\n",
			rule->field_flg[0], rule->inverse_field_flg[0]);

	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TUNNEL_TYPE) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TUNNEL_INNER_TYPE) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TUNNEL_KEY_VALID) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TUNNEL_KEY) ||
		FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_TUNNEL_DECAP_EN))
	{
		*rule_type_map |= (1<<ADPT_ACL_APPE_TUNNEL_RULE);
	}
	/* inverse tunnel fields */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_TUNNEL_TYPE))
		(*tunnel_inverse_rule_type_count)++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_TUNNEL_INNER_TYPE))
		(*tunnel_inverse_rule_type_count)++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_TUNNEL_KEY))
		(*tunnel_inverse_rule_type_count)++;

	return SW_OK;
}

sw_error_t
_adpt_appe_acl_udf_fields_check(a_uint32_t dev_id, a_uint32_t rule_id,
		a_uint32_t rule_nr, fal_acl_rule_t * rule,
		ADPT_HPPE_ACL_RULE_MAP *rule_map)
{
	a_uint32_t udf_rule_type_map = 0;

	/* with udfprofile field */
	if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDFPROFILE))
	{
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF2) ||
			FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF3))
		{
			udf_rule_type_map |= (1<<ADPT_ACL_APPE_EXT_UDF1_RULE);
		}
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF0) ||
			(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) &&
				rule->udf1_op == FAL_ACL_FIELD_MASK))
		{
			udf_rule_type_map |= (1<<ADPT_ACL_APPE_EXT_UDF0_RULE);
		}
		if(udf_rule_type_map == 0)
		{
			udf_rule_type_map |= (1<<ADPT_ACL_APPE_EXT_UDF0_RULE);
		}
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) &&
			rule->udf1_op != FAL_ACL_FIELD_MASK)
		{
			udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF1_RULE);
			if(FAL_FLG_TST(udf_rule_type_map, (1<<ADPT_ACL_APPE_EXT_UDF0_RULE)) &&
				FAL_FLG_TST(udf_rule_type_map, (1<<ADPT_ACL_APPE_EXT_UDF1_RULE)) &&
				rule->udf2_op == FAL_ACL_FIELD_MASK)
			{
				udf_rule_type_map &= ~(1<<ADPT_ACL_APPE_EXT_UDF1_RULE);
			}
		}
	}
	/*without udfprofile field*/
	else
	{
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF0))
		{
			udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF0_RULE);
		}
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF3))
		{
			if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF2) &&
				rule->udf2_op != FAL_ACL_FIELD_MASK)
			{
				udf_rule_type_map |= (1<<ADPT_ACL_APPE_EXT_UDF1_RULE);
			}
			else
			{
				udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF1_RULE);
			}
		}
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) &&
			rule->udf1_op != FAL_ACL_FIELD_MASK)
		{
			udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF1_RULE);
		}
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF2) &&
			rule->udf2_op != FAL_ACL_FIELD_MASK)
		{
			udf_rule_type_map |= (1<<ADPT_ACL_APPE_EXT_UDF1_RULE);
			if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) &&
				rule->udf1_op == FAL_ACL_FIELD_MASK)
			{
				udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF0_RULE);
			}
		}
		if(udf_rule_type_map == 0)
		{
			if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF1) ||
				FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF2))
			{
				udf_rule_type_map |= (1<<ADPT_ACL_HPPE_UDF0_RULE);
			}
		}
	}
	rule_map->rule_type_map |= udf_rule_type_map;
	/* inverse UDF fileds */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDFPROFILE))
		rule_map->inverse_rule_type_count[ADPT_ACL_APPE_EXT_UDF0_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF0))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_UDF0_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF3))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_UDF1_RULE] ++;
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF1))
		rule_map->inverse_rule_type_count[ADPT_ACL_HPPE_UDF1_RULE] ++;
	/* ext udf1 can match both udf2 mask and range */
	if(FAL_FIELD_FLG_TST(rule->inverse_field_flg, FAL_ACL_FIELD_UDF2))
		rule_map->inverse_rule_type_count[ADPT_ACL_APPE_EXT_UDF1_RULE] ++;

	return SW_OK;
}

sw_error_t
_adpt_appe_acl_ext_set(a_uint32_t dev_id, fal_acl_rule_t * rule,
		a_uint32_t hw_list_id, a_uint32_t hw_entries)
{
	sw_error_t rv = SW_OK;
	a_uint32_t hw_index;
	union eg_ipo_ext_tbl_u reg_val = {0};

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_PRE_ACL_ENTRY_NUM_PER_LIST, 0);
		if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_METADATA_EN))
		{
			rv = appe_eg_ipo_ext_tbl_get(dev_id,
				hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index,
				&reg_val);
			reg_val.bf.policy_id = rule->policy_id;
#if defined(MPPE)
			reg_val.bf.cookie = rule->cookie_val;
			reg_val.bf.cookie_pri = rule->cookie_pri;
#endif
			rv = appe_eg_ipo_ext_tbl_set(dev_id,
				hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index,
				&reg_val);
		}
		hw_entries &= (~(1<<hw_index));
	}
	return rv;
}

sw_error_t
_adpt_appe_acl_ext_get(a_uint32_t dev_id,
		a_uint32_t hw_list_id, a_uint32_t hw_entries, fal_acl_rule_t * rule)
{
	sw_error_t rv = SW_OK;
	a_uint32_t hw_index;
	union eg_ipo_ext_tbl_u reg_val = {0};

	hw_index = _acl_bit_index(hw_entries, ADPT_PRE_ACL_ENTRY_NUM_PER_LIST, 0);
	rv = appe_eg_ipo_ext_tbl_get(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &reg_val);

	rule->policy_id = reg_val.bf.policy_id;
#if defined(MPPE)
	rule->cookie_val = reg_val.bf.cookie;
	rule->cookie_pri = reg_val.bf.cookie_pri;
#endif
	return rv;
}

sw_error_t
_adpt_appe_acl_ext_clear(a_uint32_t dev_id,
		a_uint32_t hw_list_id, a_uint32_t hw_entries)
{
	sw_error_t rv = SW_OK;
	a_uint32_t hw_index;
	union eg_ipo_ext_tbl_u reg_val = {0};

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_PRE_ACL_ENTRY_NUM_PER_LIST, 0);

		rv = appe_eg_ipo_ext_tbl_set(dev_id,
				hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &reg_val);
		hw_entries &= (~(1<<hw_index));
	}
	return rv;
}

sw_error_t
_adpt_appe_pre_acl_rule_ext_set(a_uint32_t dev_id, a_uint32_t hw_list_id,
		a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4)
{
	sw_error_t rv = SW_OK;
	union pre_ipo_rule_ext_1_u reg_ext_1 = {0};
	union pre_ipo_rule_ext_2_u reg_ext_2 = {0};
	union pre_ipo_rule_ext_4_u reg_ext_4 = {0};

	if(ext_1 != 0)
	{
		rv |= appe_pre_ipo_rule_ext_1_get(dev_id, hw_list_id, &reg_ext_1);
		reg_ext_1.val |= ext_1;
		SSDK_DEBUG("ext_1.val = 0x%x\n", reg_ext_1.val);
		rv |= appe_pre_ipo_rule_ext_1_set(dev_id, hw_list_id, &reg_ext_1);
	}
	if(ext_2 != 0)
	{
		rv |= appe_pre_ipo_rule_ext_2_get(dev_id, hw_list_id, &reg_ext_2);
		reg_ext_2.val |= ext_2;
		SSDK_DEBUG("ext_2.val = 0x%x\n", reg_ext_2.val);
		rv |= appe_pre_ipo_rule_ext_2_set(dev_id, hw_list_id, &reg_ext_2);
	}
	if(ext_4 != 0)
	{
		rv |= appe_pre_ipo_rule_ext_4_get(dev_id, hw_list_id, &reg_ext_4);
		reg_ext_4.val |= ext_4;
		SSDK_DEBUG("ext_4.val = 0x%x\n", reg_ext_4.val);
		rv |= appe_pre_ipo_rule_ext_4_set(dev_id, hw_list_id, &reg_ext_4);
	}
	return rv;
}

sw_error_t
_adpt_appe_pre_acl_rule_ext_clear(a_uint32_t dev_id, a_uint32_t hw_list_id,
		a_uint32_t ext_1, a_uint32_t ext_2, a_uint32_t ext_4)
{
	sw_error_t rv = SW_OK;
	union pre_ipo_rule_ext_1_u reg_ext_1 = {0};
	union pre_ipo_rule_ext_2_u reg_ext_2 = {0};
	union pre_ipo_rule_ext_4_u reg_ext_4 = {0};

	if(ext_1)
	{
		rv |= appe_pre_ipo_rule_ext_1_get(dev_id, hw_list_id, &reg_ext_1);
		reg_ext_1.val &= (~ext_1);
		SSDK_DEBUG("ext_1.val = 0x%x\n", reg_ext_1.val);
		rv |= appe_pre_ipo_rule_ext_1_set(dev_id, hw_list_id, &reg_ext_1);
	}
	if(ext_2)
	{
		rv |= appe_pre_ipo_rule_ext_2_get(dev_id, hw_list_id, &reg_ext_2);
		reg_ext_2.val &= (~ext_2);
		SSDK_DEBUG("ext_2.val = 0x%x\n", reg_ext_2.val);
		rv |= appe_pre_ipo_rule_ext_2_set(dev_id, hw_list_id, &reg_ext_2);
	}
	if(ext_4)
	{
		rv |= appe_pre_ipo_rule_ext_4_get(dev_id, hw_list_id, &reg_ext_4);
		reg_ext_4.val &= (~ext_4);
		SSDK_DEBUG("ext_4.val = 0x%x\n", reg_ext_4.val);
		rv |= appe_pre_ipo_rule_ext_4_set(dev_id, hw_list_id, &reg_ext_4);
	}
	return rv;
}

sw_error_t
_adpt_appe_pre_acl_rule_hw_delete(a_uint32_t dev_id,
		a_uint32_t hw_list_id, a_uint32_t hw_entries, a_uint32_t rule_nr)
{
	a_uint32_t hw_index = 0;
	sw_error_t rv = SW_OK;
	union pre_ipo_rule_reg_u hw_reg = {0};
	union pre_ipo_mask_reg_u hw_mask = {0};
	union pre_ipo_action_u hw_act = {0};
	union pre_ipo_cnt_tbl_u counters = {0};
	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_PRE_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_PRE_ACL_ENTRY_NUM_PER_LIST)
		{
			break;
		}

		rv |= appe_pre_ipo_rule_reg_set(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_reg);
		rv |= appe_pre_ipo_mask_reg_set(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_mask);
		rv |= appe_pre_ipo_action_set(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_act);
		SSDK_DEBUG("ACL destroy entry %d\n",
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index);
		hw_entries &= (~(1<<hw_index));

		/*clean counters*/
		appe_pre_ipo_cnt_tbl_set(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &counters);
	}
	return rv;
}

static sw_error_t
_adpt_appe_pre_acl_action_sw_2_hw(a_uint32_t dev_id,
		fal_acl_rule_t *rule, union pre_ipo_action_u *hw_act)
{
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REDPT))
	{
		a_uint32_t dest_type = FAL_ACL_DEST_TYPE(rule->ports);
		a_uint32_t dest_val = FAL_ACL_DEST_VALUE(rule->ports);

		SSDK_DEBUG("rule->ports = %x\n", rule->ports);

		hw_act->bf.dest_info_change_en = 1;
		if(dest_type == FAL_ACL_DEST_NEXTHOP) /*nexthop*/
		{
			hw_act->bf.dest_info =
				HPPE_ACL_DEST_INFO(HPPE_ACL_DEST_NEXTHOP, dest_val);
		}
		else if(FAL_ACL_DEST_TYPE(rule->ports) == FAL_ACL_DEST_PORT_ID)/*vp*/
		{
			hw_act->bf.dest_info =
				HPPE_ACL_DEST_INFO(HPPE_ACL_DEST_PORT_ID, dest_val);
		}
		else if(FAL_ACL_DEST_TYPE(rule->ports) == FAL_ACL_DEST_PORT_BMP)/*bitmap*/
		{
			hw_act->bf.dest_info =
				HPPE_ACL_DEST_INFO(HPPE_ACL_DEST_PORT_BMP, dest_val);
		}
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_PERMIT))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 0;/*forward*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_DENY))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 1;/*drop*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_CPYCPU))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 2;/*copy to cpu*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_RDTCPU))
	{
		hw_act->bf.dest_info_change_en = 1;
		hw_act->bf.fwd_cmd = 3;/*redirect to cpu*/
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_MIRROR))
	{
		hw_act->bf.mirror_en= 1;
	}

	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID))
	{
		hw_act->bf.svid_change_en = 1;
		hw_act->bf.stag_fmt = rule->stag_fmt;
		hw_act->bf.svid = rule->stag_vid;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_PRI))
	{
		hw_act->bf.stag_pcp_change_en = 1;
		hw_act->bf.stag_pcp = rule->stag_pri;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_DEI))
	{
		hw_act->bf.stag_dei_change_en = 1;
		hw_act->bf.stag_dei = rule->stag_dei;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID))
	{
		hw_act->bf.cvid_change_en = 1;
		hw_act->bf.ctag_fmt = rule->ctag_fmt;
		hw_act->bf.cvid = rule->ctag_vid;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_PRI))
	{
		hw_act->bf.ctag_pcp_change_en = 1;
		hw_act->bf.ctag_pcp_0 = rule->ctag_pri&0x3;
		hw_act->bf.ctag_pcp_1 = (rule->ctag_pri>>2)&0x1;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_CFI))
	{
		hw_act->bf.ctag_dei_change_en = 1;
		hw_act->bf.ctag_dei = rule->ctag_cfi;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_DSCP))
	{
		hw_act->bf.dscp_tc_change_en = 1;
		hw_act->bf.dscp_tc = rule->dscp;
		hw_act->bf.dscp_tc_mask = rule->dscp_mask;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_INT_DP))
	{
		hw_act->bf.int_dp_change_en = 1;
		hw_act->bf.int_dp = rule->int_dp;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_POLICER_EN))
	{
		hw_act->bf.policer_en = 1;
		hw_act->bf.policer_index = rule->policer_ptr;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_REMARK_QUEUE))
	{
		hw_act->bf.qid_en = 1;
		hw_act->bf.qid = rule->queue;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI))
	{
		hw_act->bf.enqueue_pri_change_en = 1;
		hw_act->bf.enqueue_pri = rule->enqueue_pri;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_SERVICE_CODE))
	{
		hw_act->bf.service_code_en = 1;
		hw_act->bf.service_code_0 = rule->service_code&0x1;
		hw_act->bf.service_code_1 = (rule->service_code>>1)&0x7f;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_SYN_TOGGLE))
	{
		hw_act->bf.syn_toggle = 1;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_CPU_CODE))
	{
		hw_act->bf.cpu_code_en = 1;
		hw_act->bf.cpu_code = rule->cpu_code;
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_METADATA_EN))
	{
		hw_act->bf.metadata_en = 1;
#if defined(MPPE)
		hw_act->bf.metadata_pri = rule->metadata_pri;
#endif
	}

	hw_act->bf.qos_res_prec = rule->qos_res_prec;

	/*new action for appe pre acl*/
	if(FAL_ACTION_FLG_TST(rule->action_flg_ext, FAL_ACL_ACTION_CASCADE))
	{
		hw_act->bf.cascade_en = 1;
		hw_act->bf.cascade_data_0 = rule->cascade_data;
		hw_act->bf.cascade_data_1 = rule->cascade_data>>
			SW_FIELD_OFFSET_IN_WORD(PRE_IPO_ACTION_CASCADE_DATA_OFFSET);
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg_ext, FAL_ACL_ACTION_VPN))
	{
		hw_act->bf.vpn_valid = 1;
		hw_act->bf.vpn_type = rule->vpn_type;
		hw_act->bf.vpn_id_0 = rule->vpn_id;
		hw_act->bf.vpn_id_1 = rule->vpn_id>>
			SW_FIELD_OFFSET_IN_WORD(PRE_IPO_ACTION_VPN_ID_OFFSET);
	}
	if(FAL_ACTION_FLG_TST(rule->action_flg, FAL_ACL_ACTION_POLICY_FORWARD_EN))
	{
		if(FAL_ACL_POLICY_ROUTE == rule->policy_fwd)
		{
			hw_act->bf.nat_action = APPE_ACL_POLICY_ROUTE;/*no nat*/
		}
		else if (FAL_ACL_POLICY_SNAT == rule->policy_fwd)
		{
			hw_act->bf.nat_action = APPE_ACL_POLICY_SNAT;/*snat*/
		}
		else if (FAL_ACL_POLICY_DNAT == rule->policy_fwd)
		{
			hw_act->bf.nat_action = APPE_ACL_POLICY_DNAT;/*dnat*/
		}
		else if (FAL_ACL_POLICY_SNAPT == rule->policy_fwd)
		{
			hw_act->bf.nat_action = APPE_ACL_POLICY_SNAPT;/*snapt*/
			hw_act->bf.l4_port = rule->napt_l4_port;
		}
		else if (FAL_ACL_POLICY_DNAPT == rule->policy_fwd)
		{
			hw_act->bf.nat_action = APPE_ACL_POLICY_DNAPT;/*dnapt*/
			hw_act->bf.l4_port = rule->napt_l4_port;
		}
		else
		{
			return SW_BAD_PARAM;
		}
	}
	if (FAL_ACTION_FLG_TST(rule->action_flg_ext, FAL_ACL_ACTION_LEARN_DIS))
	{
		hw_act->bf.learn_dis = 1;
	}

	return SW_OK;
}

sw_error_t
_adpt_appe_pre_acl_tunnel_rule_sw_2_hw(fal_acl_tunnel_info_t *rule,
	ADPT_APPE_ACL_TUNNEL_RULE *tunnelrule, ADPT_APPE_ACL_TUNNEL_RULE_MASK *tunnelrule_mask,
	a_uint8_t inverse_en)
{
	fal_acl_tunnel_field_map_t field_flg = {0};

	if (inverse_en)
		field_flg[0] |= rule->inverse_field_flg[0];
	else
		field_flg[0] |= rule->field_flg[0];

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_TUNNEL_TYPE))
	{
		tunnelrule->tunnel_type = rule->tunnel_type;
		tunnelrule_mask->tunnel_type_mask = rule->tunnel_type_mask&0x1f;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_TUNNEL_TYPE);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_TUNNEL_INNER_TYPE))
	{
		tunnelrule->tunnel_inner_type = rule->inner_type;
		tunnelrule_mask->tunnel_inner_type_mask = rule->inner_type_mask&0x3;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_TUNNEL_INNER_TYPE);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_TUNNEL_KEY_VALID))
	{
		tunnelrule->tunnel_info_valid = rule->tunnel_key_valid;
		tunnelrule_mask->tunnel_info_valid_mask = rule->tunnel_key_valid_mask;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_TUNNEL_KEY))
	{
		tunnelrule->tunnel_info = rule->tunnel_key;
		tunnelrule_mask->tunnel_info_mask = rule->tunnel_key_mask;
		if(inverse_en) {
			FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_TUNNEL_KEY);
			return SW_OK;
		}
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_TUNNEL_DECAP_EN))
	{
		tunnelrule->tunnel_decap_en = rule->tunnel_decap_en;
		tunnelrule_mask->tunnel_decap_en_mask = rule->tunnel_decap_en_mask;
	}
	return SW_OK;
}

sw_error_t
_adpt_appe_acl_ext_udf_rule_sw_2_hw(fal_acl_rule_t *rule, a_uint32_t is_win1,
	ADPT_APPE_ACL_EXT_UDF_RULE * extudfrule, ADPT_APPE_ACL_EXT_UDF_RULE_MASK *extudfrule_mask,
	a_uint8_t *range_en, a_uint8_t inverse_en)
{
	fal_acl_field_map_t field_flg = {0};

	if (inverse_en)
		FAL_FIELD_FLG_CPY(field_flg, rule->inverse_field_flg);
	else
		FAL_FIELD_FLG_CPY(field_flg, rule->field_flg);

	if(is_win1)
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF3))
		{
			extudfrule->udf1_valid = 1;
			extudfrule->udf1 = rule->udf3_val;
			extudfrule_mask->udf1_valid = 1;
			extudfrule_mask->udf1_mask = rule->udf3_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF3);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDFPROFILE))
		{
			extudfrule->udfprofile_valid= 1;
			extudfrule->udfprofile= rule->udfprofile_val;
			extudfrule_mask->udfprofile_valid= 1;
			extudfrule_mask->udfprofile_mask= rule->udfprofile_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDFPROFILE);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(rule->field_flg, FAL_ACL_FIELD_UDF2))
		{
			extudfrule->udf0_valid = 1;
			extudfrule_mask->udf0_valid = 1;
			if(FAL_ACL_FIELD_MASK == rule->udf2_op)
			{
				extudfrule->udf0 = rule->udf2_val;
				extudfrule_mask->udf0_mask = rule->udf2_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->udf2_op)
				{
					min = 0;
					max = rule->udf2_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->udf2_op)
				{
					min = rule->udf2_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->udf2_op)
				{
					min = rule->udf2_val;
					max = rule->udf2_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				extudfrule->udf0 = min;
				extudfrule_mask->udf0_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF2);
				return SW_OK;
			}
		}
	}
	else
	{
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF1) &&
			FAL_ACL_FIELD_MASK == rule->udf1_op)
		{
			extudfrule->udf1_valid = 1;
			extudfrule->udf1 = rule->udf1_val;
			extudfrule_mask->udf1_valid = 1;
			extudfrule_mask->udf1_mask = rule->udf1_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF1);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDFPROFILE))
		{
			extudfrule->udfprofile_valid= 1;
			extudfrule->udfprofile= rule->udfprofile_val;
			extudfrule_mask->udfprofile_valid= 1;
			extudfrule_mask->udfprofile_mask= rule->udfprofile_mask;
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDFPROFILE);
				return SW_OK;
			}
		}
		if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_UDF0))
		{
			extudfrule->udf0_valid = 1;
			extudfrule_mask->udf0_valid = 1;
			if(FAL_ACL_FIELD_MASK == rule->udf0_op)
			{
				extudfrule->udf0 = rule->udf0_val;
				extudfrule_mask->udf0_mask = rule->udf0_mask;
			}
			else
			{
				a_uint16_t min, max;
				if(FAL_ACL_FIELD_LE == rule->udf0_op)
				{
					min = 0;
					max = rule->udf0_val;
				}
				else if(FAL_ACL_FIELD_GE == rule->udf0_op)
				{
					min = rule->udf0_val;
					max = 0xffff;
				}
				else if(FAL_ACL_FIELD_RANGE == rule->udf0_op)
				{
					min = rule->udf0_val;
					max = rule->udf0_mask;
				}
				else
					return SW_NOT_SUPPORTED;
				extudfrule->udf0 = min;
				extudfrule_mask->udf0_mask = max;
				*range_en = 1;
			}
			if(inverse_en) {
				FAL_FIELD_FLG_CLR(rule->inverse_field_flg, FAL_ACL_FIELD_UDF0);
				return SW_OK;
			}
		}
	}

	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IP))
	{
		extudfrule->is_ip = rule->is_ip_val;
		extudfrule_mask->is_ip = 1;
	}
	if(FAL_FIELD_FLG_TST(field_flg, FAL_ACL_FIELD_IPV6))
	{
		extudfrule->is_ipv6= rule->is_ipv6_val;
		extudfrule_mask->is_ipv6 = 1;
	}
	return SW_OK;
}

static sw_error_t
_adpt_appe_pre_acl_action_hw_2_sw(a_uint32_t dev_id,
	union pre_ipo_action_u *hw_act, fal_acl_rule_t *rule)
{
	if(hw_act->bf.dest_info_change_en)
	{
		a_uint32_t dest_type = HPPE_ACL_DEST_TYPE(hw_act->bf.dest_info);
		a_uint32_t dest_val = HPPE_ACL_DEST_VALUE(hw_act->bf.dest_info);
		SSDK_DEBUG("hw_act->bf.dest_info = %x\n", hw_act->bf.dest_info);
		if(dest_type == HPPE_ACL_DEST_NEXTHOP) /*nexthop*/
		{
			rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_NEXTHOP,
					dest_val);
		}
		else if(dest_type == HPPE_ACL_DEST_PORT_ID) /*vp or trunk*/
		{
			rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_PORT_ID,
					dest_val);
		}
		else if(dest_type == HPPE_ACL_DEST_PORT_BMP) /*bitmap*/
		{
			rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_PORT_BMP,
					dest_val);
		}
		if(rule->ports != 0)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REDPT);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_RDTCPU)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_RDTCPU);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_COPYCPU)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_CPYCPU);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_DROP)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_DENY);
		}
		else if(hw_act->bf.fwd_cmd == HPPE_ACL_ACTION_FWD)
		{
			FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_PERMIT);
		}
	}

	if(hw_act->bf.mirror_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_MIRROR);
	}
	if(hw_act->bf.svid_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID);
		rule->stag_fmt = hw_act->bf.stag_fmt;
		rule->stag_vid = hw_act->bf.svid;
	}
	if(hw_act->bf.stag_pcp_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_PRI);
		rule->stag_pri = hw_act->bf.stag_pcp;
	}
	if(hw_act->bf.stag_dei_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_DEI);
		rule->stag_dei = hw_act->bf.stag_dei;
	}
	if(hw_act->bf.cvid_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID);
		rule->ctag_fmt = hw_act->bf.ctag_fmt;
		rule->ctag_vid = hw_act->bf.cvid;
	}
	if(hw_act->bf.ctag_pcp_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_PRI);
		rule->ctag_pri = (hw_act->bf.ctag_pcp_1<<2)|hw_act->bf.ctag_pcp_0;
	}
	if(hw_act->bf.ctag_dei_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_CFI);
		rule->ctag_cfi = hw_act->bf.ctag_dei;
	}
	if(hw_act->bf.dscp_tc_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_DSCP);
		rule->dscp = hw_act->bf.dscp_tc;
		rule->dscp_mask = hw_act->bf.dscp_tc_mask;
	}
	if(hw_act->bf.int_dp_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_INT_DP);
		rule->int_dp = hw_act->bf.int_dp;
	}
	if(hw_act->bf.policer_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_POLICER_EN);
		rule->policer_ptr = hw_act->bf.policer_index;
	}
	if(hw_act->bf.qid_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_REMARK_QUEUE);
		rule->queue = hw_act->bf.qid;
	}
	if(hw_act->bf.enqueue_pri_change_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI);
		rule->enqueue_pri = hw_act->bf.enqueue_pri;
	}
	if(hw_act->bf.service_code_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_SERVICE_CODE);
		rule->service_code = (hw_act->bf.service_code_1<<1)|hw_act->bf.service_code_0;
	}
	if(hw_act->bf.syn_toggle)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_SYN_TOGGLE);
	}
	if(hw_act->bf.cpu_code_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_CPU_CODE);
		rule->cpu_code = hw_act->bf.cpu_code;
	}
	if(hw_act->bf.metadata_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_METADATA_EN);
#if defined(MPPE)
		rule->metadata_pri = hw_act->bf.metadata_pri;
#endif
	}

	rule->qos_res_prec = hw_act->bf.qos_res_prec;

	/*new appe pre-acl actions*/
	if(hw_act->bf.cascade_en == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg_ext, FAL_ACL_ACTION_CASCADE);
		rule->cascade_data = (hw_act->bf.cascade_data_1 <<
				SW_FIELD_OFFSET_IN_WORD(PRE_IPO_ACTION_CASCADE_DATA_OFFSET)) |
				hw_act->bf.cascade_data_0;
	}
	if(hw_act->bf.vpn_valid == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg_ext, FAL_ACL_ACTION_VPN);
		rule->vpn_type = hw_act->bf.vpn_type;
		rule->vpn_id = (hw_act->bf.vpn_id_1 <<
				SW_FIELD_OFFSET_IN_WORD(PRE_IPO_ACTION_VPN_ID_OFFSET)) |
				hw_act->bf.vpn_id_0;
	}
	if(FAL_ACL_DEST_TYPE(rule->ports) == FAL_ACL_DEST_NEXTHOP)
	{
		if(hw_act->bf.nat_action == APPE_ACL_POLICY_ROUTE)
		{
			rule->policy_fwd = FAL_ACL_POLICY_ROUTE;
		}
		else if(hw_act->bf.nat_action == APPE_ACL_POLICY_SNAT)
		{
			rule->policy_fwd = FAL_ACL_POLICY_SNAT;
		}
		else if(hw_act->bf.nat_action == APPE_ACL_POLICY_DNAT)
		{
			rule->policy_fwd = FAL_ACL_POLICY_DNAT;
		}
		else if(hw_act->bf.nat_action == APPE_ACL_POLICY_SNAPT)
		{
			rule->policy_fwd = FAL_ACL_POLICY_SNAPT;
			rule->napt_l4_port = hw_act->bf.l4_port;
		}
		else if(hw_act->bf.nat_action == APPE_ACL_POLICY_DNAPT)
		{
			rule->policy_fwd = FAL_ACL_POLICY_DNAPT;
			rule->napt_l4_port = hw_act->bf.l4_port;
		}
		else
		{
			return SW_BAD_VALUE;
		}
		FAL_ACTION_FLG_SET(rule->action_flg, FAL_ACL_ACTION_POLICY_FORWARD_EN);
	}
	if(hw_act->bf.learn_dis == 1)
	{
		FAL_ACTION_FLG_SET(rule->action_flg_ext, FAL_ACL_ACTION_LEARN_DIS);
	}

	return SW_OK;
}

sw_error_t
_adpt_appe_pre_acl_tunnel_rule_hw_2_sw(ADPT_APPE_ACL_TUNNEL_RULE *tunnelrule,
		ADPT_APPE_ACL_TUNNEL_RULE_MASK *tunnelrule_mask, a_uint8_t inverse_en,
		fal_acl_tunnel_info_t * rule)
{
	fal_acl_tunnel_field_map_t field_flg = {0};

	if(tunnelrule_mask->tunnel_type_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_TUNNEL_TYPE);
		rule->tunnel_type = tunnelrule->tunnel_type;
		rule->tunnel_type_mask = tunnelrule_mask->tunnel_type_mask;
	}
	if(tunnelrule_mask->tunnel_inner_type_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_TUNNEL_INNER_TYPE);
		rule->inner_type = tunnelrule->tunnel_inner_type;
		rule->inner_type_mask = tunnelrule_mask->tunnel_inner_type_mask;
	}
	if(tunnelrule_mask->tunnel_info_valid_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_TUNNEL_KEY_VALID);
		rule->tunnel_key_valid = tunnelrule->tunnel_info_valid;
		rule->tunnel_key_valid_mask = tunnelrule_mask->tunnel_info_valid_mask;
	}
	if(tunnelrule_mask->tunnel_info_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_TUNNEL_KEY);
		rule->tunnel_key = tunnelrule->tunnel_info;
		rule->tunnel_key_mask = tunnelrule_mask->tunnel_info_mask;
	}
	if(tunnelrule_mask->tunnel_decap_en_mask)
	{
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_TUNNEL_DECAP_EN);
		rule->tunnel_decap_en = tunnelrule->tunnel_decap_en;
		rule->tunnel_decap_en_mask = tunnelrule_mask->tunnel_decap_en_mask;
	}
	if (inverse_en)
		rule->inverse_field_flg[0] |= field_flg[0];
	else
		rule->field_flg[0] |= field_flg[0];

	return SW_OK;
}


sw_error_t
_adpt_appe_acl_ext_udf_rule_hw_2_sw(a_uint32_t is_win1,
	ADPT_APPE_ACL_EXT_UDF_RULE * extudfrule,
	ADPT_APPE_ACL_EXT_UDF_RULE_MASK *extudfrule_mask,
	a_uint8_t range_en, a_uint8_t inverse_en, fal_acl_rule_t *rule)
{
	fal_acl_field_map_t field_flg = {0};

	if(is_win1)
	{
		if(extudfrule->udfprofile_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDFPROFILE);
			rule->udfprofile_val = extudfrule->udfprofile;
			rule->udfprofile_mask = extudfrule_mask->udfprofile_mask;
		}
		if(extudfrule->udf1_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF3);
			rule->udf3_val = extudfrule->udf1;
			rule->udf3_mask = extudfrule_mask->udf1_mask;
		}
		if(extudfrule->udf0_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF2);
			if(range_en == 1)
			{
				if(extudfrule->udf0 == 0)
				{
					rule->udf2_op = FAL_ACL_FIELD_LE;
					rule->udf2_val = extudfrule_mask->udf0_mask;
				}
				else if(extudfrule_mask->udf0_mask == 0xffff)
				{
					rule->udf2_op = FAL_ACL_FIELD_GE;
					rule->udf2_val = extudfrule->udf0;
				}
				else
				{
					rule->udf2_op = FAL_ACL_FIELD_RANGE;
					rule->udf2_val = extudfrule->udf0;
					rule->udf2_mask = extudfrule_mask->udf0_mask;
				}
			}
			else
			{
				rule->udf2_op = FAL_ACL_FIELD_MASK;
				rule->udf2_val = extudfrule->udf0;
				rule->udf2_mask = extudfrule_mask->udf0_mask;
			}
		}
	}
	else
	{
		if(extudfrule->udfprofile_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDFPROFILE);
			rule->udfprofile_val = extudfrule->udfprofile;
			rule->udfprofile_mask = extudfrule_mask->udfprofile_mask;
		}
		if(extudfrule->udf1_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF1);
			rule->udf1_val = extudfrule->udf1;
			rule->udf1_mask = extudfrule_mask->udf1_mask;
		}
		if(extudfrule->udf0_valid == 1)
		{
			FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_UDF0);
			if(range_en == 1)
			{
				if(extudfrule->udf0 == 0)
				{
					rule->udf0_op = FAL_ACL_FIELD_LE;
					rule->udf0_val = extudfrule_mask->udf0_mask;
				}
				else if(extudfrule_mask->udf0_mask == 0xffff)
				{
					rule->udf0_op = FAL_ACL_FIELD_GE;
					rule->udf0_val = extudfrule->udf0;
				}
				else
				{
					rule->udf0_op = FAL_ACL_FIELD_RANGE;
					rule->udf0_val = extudfrule->udf0;
					rule->udf0_mask = extudfrule_mask->udf0_mask;
				}
			}
			else
			{
				rule->udf0_op = FAL_ACL_FIELD_MASK;
				rule->udf0_val = extudfrule->udf0;
				rule->udf0_mask = extudfrule_mask->udf0_mask;
			}
		}
	}

	if(extudfrule_mask->is_ip)
	{
		rule->is_ip_val = extudfrule->is_ip;
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IP);
	}
	if(extudfrule_mask->is_ipv6)
	{
		rule->is_ipv6_val = extudfrule->is_ipv6;
		FAL_FIELD_FLG_SET(field_flg, FAL_ACL_FIELD_IPV6);
	}
	if (inverse_en)
		FAL_FIELD_FLG_CPY(rule->inverse_field_flg, field_flg);
	else
		FAL_FIELD_FLG_CPY(rule->field_flg, field_flg);

	return SW_OK;
}

sw_error_t
_adpt_appe_pre_acl_rule_sw_query(a_uint32_t dev_id,
	a_uint32_t hw_list_id, a_uint32_t hw_entries, fal_acl_rule_t * rule)
{
	a_uint32_t hw_index;
	a_uint64_t byte_cnt;
	sw_error_t rv = SW_OK;
	fal_acl_rule_t * sw_rule;
	fal_acl_rule_t inner_rule;
	union pre_ipo_rule_reg_u hw_reg = {0};
	union pre_ipo_mask_reg_u hw_mask = {0};
	union pre_ipo_action_u hw_act = {0};
	union pre_ipo_cnt_tbl_u hw_match = {0};

	aos_mem_zero(&inner_rule, sizeof(fal_acl_rule_t));

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_PRE_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_PRE_ACL_ENTRY_NUM_PER_LIST)
		{
			return SW_FAIL;
		}
		rv |= appe_pre_ipo_rule_reg_get(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_reg);
		rv |= appe_pre_ipo_mask_reg_get(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_mask);
		rv |= appe_pre_ipo_action_get(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_act);

		rule->acl_pool = hw_reg.bf.res_chain;
		rule->pri = hw_reg.bf.pri&0x7;
		if(hw_reg.bf.inner_outer_sel == 0)
		{
			sw_rule = rule;
		}
		else
		{
			sw_rule = &inner_rule;
		}

		/*get sw rule info from first 53bit hw rule reg fields*/
		_adpt_hppe_acl_rule_hw_2_sw(dev_id, hw_reg.bf.rule_type,
			hw_reg.bf.range_en, hw_reg.bf.inverse_en, &hw_reg, &hw_mask, sw_rule);

		_adpt_appe_pre_acl_action_hw_2_sw(dev_id, &hw_act, rule);

		rv |= appe_pre_ipo_cnt_tbl_get(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_match);
		rule->match_cnt += hw_match.bf.hit_pkt_cnt;
		byte_cnt = hw_match.bf.hit_byte_cnt_1;
		rule->match_bytes += byte_cnt<<32|hw_match.bf.hit_byte_cnt_0;

		hw_entries &= (~(1<<hw_index));
	}
	acl_rule_field_convert(&inner_rule, &rule->inner_rule_field, A_TRUE);
	return rv;
}

sw_error_t
_adpt_appe_pre_acl_rule_hw_add(a_uint32_t dev_id, a_uint32_t list_pri,
		a_uint32_t hw_list_id, a_uint32_t rule_id, a_uint32_t rule_nr,
		fal_acl_rule_t * rule, fal_acl_rule_t *inner_rule,
		ADPT_HPPE_ACL_RULE_MAP *rule_map,
		ADPT_HPPE_ACL_RULE_MAP *inner_rule_map,
		a_uint32_t allocated_entries)
{
	union pre_ipo_rule_reg_u hw_reg = {0};
	union pre_ipo_mask_reg_u hw_mask = {0};
	union pre_ipo_action_u hw_act = {0};
	sw_error_t rv = 0;
	a_uint32_t hw_entry = 0, rule_type = 0;
	ADPT_HPPE_ACL_RULE_MAP tmp_rule_map;
	a_uint8_t inner_outer_sel = 0, range_en = 0;
	fal_acl_rule_t *sw_rule = NULL;
	aos_mem_zero(&tmp_rule_map, sizeof(ADPT_HPPE_ACL_RULE_MAP));

	hw_reg.bf.res_chain = rule->acl_pool;
	hw_reg.bf.pri = (list_pri<<3)|rule->pri;

	for(inner_outer_sel = 0; inner_outer_sel < 2; inner_outer_sel++)
	{
		/*set inner_outer_sel field of hw rule reg*/
		hw_reg.bf.inner_outer_sel = inner_outer_sel;
		if(inner_outer_sel == 0)
		{ /*outer*/
			aos_mem_copy(&tmp_rule_map,
				rule_map, sizeof(ADPT_HPPE_ACL_RULE_MAP));
			sw_rule = rule;
		}
		else
		{ /*inner*/
			aos_mem_copy(&tmp_rule_map,
				inner_rule_map, sizeof(ADPT_HPPE_ACL_RULE_MAP));
			sw_rule = inner_rule;
		}
		for( rule_type = 0; rule_type < ADPT_ACL_HPPE_RULE_TYPE_NUM; rule_type++)
		{
			if ((BIT(rule_type)) & tmp_rule_map.rule_type_map) {
				hw_reg.bf.rule_field_0 = 0;
				hw_reg.bf.rule_field_1 = 0;
				hw_reg.bf.inverse_en = 0;
				memset(&hw_mask, 0, sizeof(hw_mask));
				memset(&hw_act, 0, sizeof(hw_act));

				/*set 53bit rule fields of hw rule reg*/
				_adpt_hppe_acl_rule_sw_2_hw(dev_id, sw_rule, rule_type,
					&hw_entry, &allocated_entries, &range_en,
					&hw_reg, &hw_mask, 0);
				/*set rule_type, range_en, inverse_en fields of hw rule reg*/
				hw_reg.bf.rule_type = rule_type;
				hw_reg.bf.range_en = range_en;
				if (rule_type == ADPT_ACL_APPE_TUNNEL_RULE)
				{
					if (FAL_FIELD_FLG_TST(rule->tunnel_info.field_flg,
							FAL_ACL_FIELD_TUNNEL_INVERSE_ALL))
					{
						hw_reg.bf.inverse_en = 1;
					}
				}
				else
				{
					if (FAL_FIELD_FLG_TST(sw_rule->field_flg,
							FAL_ACL_FIELD_INVERSE_ALL))
					{
						hw_reg.bf.inverse_en = 1;
					}
				}
				/*debug info*/
				SSDK_DEBUG("inner_outer_sel %d, chain %d, pri %d, src_1 %d, "
				"src_0 %d src_type %d rule_type %d, inverse %d, range %d\n",
				hw_reg.bf.inner_outer_sel, hw_reg.bf.res_chain, hw_reg.bf.pri,
				hw_reg.bf.src_1, hw_reg.bf.src_0, hw_reg.bf.src_type,
				hw_reg.bf.rule_type, hw_reg.bf.inverse_en, hw_reg.bf.range_en);
				SSDK_DEBUG("rule and mask set hw_entry = %d\n",
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_entry);

				rv |= appe_pre_ipo_rule_reg_set(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_entry,
					&hw_reg);

				rv |= appe_pre_ipo_mask_reg_set(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_entry,
					&hw_mask);

				/*set action fields*/
				_adpt_appe_pre_acl_action_sw_2_hw(dev_id, rule, &hw_act);
				rv |= appe_pre_ipo_action_set(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_entry,
					&hw_act);
				SW_RTN_ON_ERROR(rv);
			}
			/* inverse rule */
			for (; tmp_rule_map.inverse_rule_type_count[rule_type] > 0;
					tmp_rule_map.inverse_rule_type_count[rule_type] --) {
				hw_reg.bf.rule_field_0 = 0;
				hw_reg.bf.rule_field_1 = 0;
				memset(&hw_mask, 0, sizeof(hw_mask));
				memset(&hw_act, 0, sizeof(hw_act));

				/*set 53bit rule fields of hw rule reg*/
				_adpt_hppe_acl_rule_sw_2_hw(dev_id, sw_rule, rule_type,
					&hw_entry, &allocated_entries, &range_en,
					&hw_reg, &hw_mask, 1);

				/*set rule_type, range_en, inverse_en fields of hw rule reg*/
				hw_reg.bf.rule_type = rule_type;
				hw_reg.bf.range_en = range_en;
				hw_reg.bf.inverse_en = 1;

				SSDK_DEBUG("rule_type %d, inverse_rule_type_count %d "
					" rule_hw_entry %d\n",
					rule_type, tmp_rule_map.inverse_rule_type_count[rule_type],
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST + hw_entry);

				rv |= appe_pre_ipo_rule_reg_set(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_entry,
					&hw_reg);

				rv |= appe_pre_ipo_mask_reg_set(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_entry,
					&hw_mask);

				/*set action fields*/
				_adpt_appe_pre_acl_action_sw_2_hw(dev_id, rule, &hw_act);
				rv |= appe_pre_ipo_action_set(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_entry,
					&hw_act);
				SW_RTN_ON_ERROR(rv);
			}
		}
	}
	return SW_OK;
}

static a_uint32_t _adpt_appe_pre_acl_srctype_to_hw(fal_acl_bind_obj_t obj_t)
{
	a_uint32_t src_type = APPE_PRE_ACL_TYPE_INVALID;

	switch(obj_t)
	{
		case FAL_ACL_BIND_PORTBITMAP:
			src_type = APPE_PRE_ACL_TYPE_PORTBITMAP;
			break;
		case FAL_ACL_BIND_PORT:
			src_type = APPE_PRE_ACL_TYPE_PORT;
			break;
		case FAL_ACL_BIND_SERVICE_CODE:
			src_type = APPE_PRE_ACL_TYPE_SERVICE_CODE;
			break;
		case FAL_ACL_BIND_VP_GROUP:
			src_type = APPE_PRE_ACL_TYPE_VP_GROUP;
			break;
		case FAL_ACL_BIND_TUNNEL_PORT:
			src_type = APPE_PRE_ACL_TYPE_TUNNEL_PORT;
			break;
		case FAL_ACL_BIND_TUNNEL_L3_IF:
			src_type = APPE_PRE_ACL_TYPE_TUNNEL_L3_IF;
			break;
		case FAL_ACL_BIND_TUNNEL_VP_GROUP:
			src_type = APPE_PRE_ACL_TYPE_TUNNEL_VP_GROUP;
			break;
		case FAL_ACL_BIND_SERVICE_PORTBITMAP:
			src_type = APPE_PRE_ACL_TYPE_SERVICE_PORTBITMAP;
			break;
		default:
			break;
	}
	return src_type;
}

sw_error_t
_adpt_appe_pre_acl_rule_bind(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	a_uint32_t hw_index = 0, hw_srctype = 0;
	union pre_ipo_rule_reg_u hw_reg = {0};

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries,
					ADPT_PRE_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_PRE_ACL_ENTRY_NUM_PER_LIST)
		{
			break;
		}

		appe_pre_ipo_rule_reg_get(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_reg);

		if(obj_t == FAL_ACL_BIND_PORT && obj_idx < SSDK_MAX_PORT_NUM)
		{
			/*convert port to bitmap if it is physical port*/
			obj_t = FAL_ACL_BIND_PORTBITMAP;
			obj_idx = (1<<obj_idx);
		}

		hw_srctype = _adpt_appe_pre_acl_srctype_to_hw(obj_t);

		if(hw_srctype == APPE_PRE_ACL_TYPE_INVALID)
		{
			SSDK_ERROR("Invalid source type %d\n", obj_t);
			return SW_BAD_PARAM;
		}
		else if(hw_srctype == APPE_PRE_ACL_TYPE_PORTBITMAP &&
			hw_reg.bf.src_type == APPE_PRE_ACL_TYPE_PORTBITMAP)
		{
			hw_reg.bf.src_0 |= obj_idx;
			hw_reg.bf.src_1 |= obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(PRE_IPO_RULE_REG_SRC_OFFSET);
		}
		else if(hw_srctype == APPE_PRE_ACL_TYPE_SERVICE_PORTBITMAP &&
			hw_reg.bf.src_type == APPE_PRE_ACL_TYPE_SERVICE_PORTBITMAP)
		{
			hw_reg.bf.src_0 |= obj_idx;
			hw_reg.bf.src_1 |= obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(PRE_IPO_RULE_REG_SRC_OFFSET);
		}
		else
		{
			hw_reg.bf.src_0 = obj_idx;
			hw_reg.bf.src_1 = obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(PRE_IPO_RULE_REG_SRC_OFFSET);
		}
		hw_reg.bf.src_type = hw_srctype;

		appe_pre_ipo_rule_reg_set(dev_id,
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_reg);
		SSDK_DEBUG("Pre-ACL bind entry %d source type %d, source value 0x%x\n",
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, obj_t, obj_idx);
		hw_entries &= (~(1<<hw_index));
	}

	return SW_OK;
}

sw_error_t
_adpt_appe_pre_acl_rule_unbind(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries,
	fal_acl_direc_t direc, fal_acl_bind_obj_t obj_t, a_uint32_t obj_idx)
{
	a_uint32_t hw_index = 0;
	union pre_ipo_rule_reg_u hw_reg = {0};

	while(hw_entries != 0)
	{
		hw_index = _acl_bit_index(hw_entries, ADPT_PRE_ACL_ENTRY_NUM_PER_LIST, 0);
		if(hw_index >= ADPT_PRE_ACL_ENTRY_NUM_PER_LIST)
		{
			break;
		}

		appe_pre_ipo_rule_reg_get(dev_id,
				hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_reg);

		if(obj_t == FAL_ACL_BIND_PORT && obj_idx < SSDK_MAX_PORT_NUM)
		{
			/*convert port to bitmap if it is physical port*/
			obj_t = FAL_ACL_BIND_PORTBITMAP;
			obj_idx = (1<<obj_idx);
		}

		if(hw_reg.bf.src_type != _adpt_appe_pre_acl_srctype_to_hw(obj_t))
		{
			SSDK_ERROR("PRE-ACL unbind fail obj_t %d\n", obj_t);
			return SW_NOT_FOUND;
		}
		if(hw_reg.bf.src_type == APPE_PRE_ACL_TYPE_PORTBITMAP ||
			hw_reg.bf.src_type == APPE_PRE_ACL_TYPE_SERVICE_PORTBITMAP)
		{
			hw_reg.bf.src_0 &= ~obj_idx;
			hw_reg.bf.src_1 &= ~(obj_idx>>
				SW_FIELD_OFFSET_IN_WORD(PRE_IPO_RULE_REG_SRC_OFFSET));
		}
		else
		{
			hw_reg.bf.src_type = APPE_PRE_ACL_TYPE_PORTBITMAP;
			hw_reg.bf.src_0 = 0;
			hw_reg.bf.src_1 = 0;
		}
		appe_pre_ipo_rule_reg_set(dev_id,
				hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, &hw_reg);
		SSDK_DEBUG("PRE-ACL unbind entry %d source type %d, source value 0x%x\n",
			hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+hw_index, obj_t, obj_idx);
		hw_entries &= (~(1<<hw_index));
	}

	return SW_OK;
}

sw_error_t
_adpt_appe_pre_acl_rule_dump(a_uint32_t dev_id, a_uint32_t hw_list_id, a_uint32_t hw_entries)
{
	a_uint8_t i = 0;
	union pre_ipo_rule_reg_u hw_reg = {0};
	union pre_ipo_mask_reg_u hw_mask = {0};
	union pre_ipo_action_u hw_act = {0};

	if(hw_entries != 0)
	{
		for(i = 0; i < ADPT_PRE_ACL_ENTRY_NUM_PER_LIST; i++)
		{
			if((1<<i) & hw_entries)
			{
				appe_pre_ipo_rule_reg_get(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+i, &hw_reg);
				appe_pre_ipo_mask_reg_get(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+i, &hw_mask);
				appe_pre_ipo_action_get(dev_id,
					hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+i, &hw_act);
				printk("hw_entry %d\n",
						hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+i);
				_adpt_acl_reg_dump((u_int8_t *)&hw_reg, sizeof(hw_reg));
				printk("hw_entry_mask %d\n",
						hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+i);
				_adpt_acl_reg_dump((u_int8_t *)&hw_mask, sizeof(hw_mask));
				printk("hw_action %d\n",
						hw_list_id*ADPT_PRE_ACL_ENTRY_NUM_PER_LIST+i);
				_adpt_acl_reg_dump((u_int8_t *)&hw_act, sizeof(hw_act));
			}
		}
	}

	return SW_OK;
}

static a_bool_t
_adpt_appe_is_udf_profile_entry_equal(a_uint32_t dev_id,
		fal_acl_udf_profile_entry_t * entry1, fal_acl_udf_profile_entry_t * entry2)
{
	if (entry1->field_flag[0] == entry2->field_flag[0] &&
		entry1->l3_type == entry2->l3_type &&
		entry1->l4_type == entry2->l4_type)
	{
		return A_TRUE;
	}
	else
	{
		return A_FALSE;
	}
}

static a_bool_t
_adpt_appe_get_udf_profile_entry_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_acl_udf_profile_entry_t * entry, a_uint32_t * profile_id)
{
	union ipr_udf_ctrl_u udf_ctrl = {0};
	sw_error_t rv = SW_OK;

	rv = appe_ipr_udf_ctrl_get(dev_id, index, &udf_ctrl);
	if (rv != SW_OK)
		return A_FALSE;

	if (!udf_ctrl.bf.valid)
	{
		aos_mem_zero(&udf_ctrl, sizeof (udf_ctrl));
	}
	entry->l3_type = udf_ctrl.bf.l3_type;
	entry->l4_type = udf_ctrl.bf.l4_type;
	*profile_id = udf_ctrl.bf.udf_profile;

	if (udf_ctrl.bf.l3_type_incl)
	{
		FAL_FIELD_FLG_SET(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
	}
	else
	{
		FAL_FIELD_FLG_CLR(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
	}
	if (udf_ctrl.bf.l4_type_incl)
	{
		FAL_FIELD_FLG_SET(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
	}
	else
	{
		FAL_FIELD_FLG_CLR(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
	}
	return udf_ctrl.bf.valid;
}

static sw_error_t
_adpt_appe_insert_udf_profile_entry_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_acl_udf_profile_entry_t * entry, a_uint32_t profile_id)
{
	union ipr_udf_ctrl_u udf_ctrl = {0};

	udf_ctrl.bf.valid = A_TRUE;
	udf_ctrl.bf.l3_type = entry->l3_type;
	udf_ctrl.bf.l4_type = entry->l4_type;
	udf_ctrl.bf.udf_profile = profile_id;

	if (FAL_FIELD_FLG_TST(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE))
	{
		udf_ctrl.bf.l3_type_incl = A_TRUE;
	}
	if (FAL_FIELD_FLG_TST(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE))
	{
		udf_ctrl.bf.l4_type_incl = A_TRUE;
	}
	return appe_ipr_udf_ctrl_set(dev_id, index, &udf_ctrl);
}

static a_uint32_t
_adpt_appe_udf_profile_entry_weight(fal_acl_udf_profile_entry_t * entry)
{
	a_uint32_t weight = 0;
	if (FAL_FIELD_FLG_TST(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE))
	{
		weight++;
	}
	if (FAL_FIELD_FLG_TST(entry->field_flag, FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE))
	{
		weight++;
	}
	return weight;
}

static sw_error_t
_adpt_appe_insert_udf_profile_entry_by_sort(a_uint32_t dev_id,
		a_uint32_t index, fal_acl_udf_profile_entry_t * entry, a_uint32_t profile_id)
{
	fal_acl_udf_profile_entry_t temp_entry = {0};
	a_uint32_t temp_profile_id;
	a_uint32_t idx, weight, temp_weight;

	weight = _adpt_appe_udf_profile_entry_weight(entry);

	for (idx = index; idx < IPR_UDF_CTRL_MAX_ENTRY-1; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_acl_udf_profile_entry_t));
		_adpt_appe_get_udf_profile_entry_by_index(dev_id, idx+1,
				&temp_entry, &temp_profile_id);
		temp_weight = _adpt_appe_udf_profile_entry_weight(&temp_entry);
		if (weight < temp_weight)
		{
			SW_RTN_ON_ERROR(_adpt_appe_insert_udf_profile_entry_by_index(dev_id, idx,
								&temp_entry, temp_profile_id));
		}
		else
		{
			break;
		}
	}
	return _adpt_appe_insert_udf_profile_entry_by_index(dev_id, idx, entry, profile_id);
}

static a_bool_t
_adpt_appe_is_udf_profile_entry_exist(a_uint32_t dev_id,
		a_uint32_t profile_id, fal_acl_udf_profile_entry_t * entry)
{
	a_uint32_t idx, temp_profile_id;
	a_bool_t entry_valid;
	fal_acl_udf_profile_entry_t temp_entry;


	if (dev_id >= SW_MAX_NR_DEV || profile_id >= IPR_UDF_PROFILE_BASE_MAX_ENTRY)
	{
		return A_FALSE;
	}

	for (idx = 0; idx < IPR_UDF_CTRL_MAX_ENTRY; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_acl_udf_profile_entry_t));
		entry_valid = _adpt_appe_get_udf_profile_entry_by_index(dev_id, idx,
						&temp_entry, &temp_profile_id);
		if (entry_valid == A_TRUE)
		{
			if (A_TRUE == _adpt_appe_is_udf_profile_entry_equal(dev_id,
						&temp_entry, entry))
			{
				if (temp_profile_id == profile_id)
				{
					return A_TRUE;
				}
			}
		}
	}
	return A_FALSE;
}

static sw_error_t
_adpt_appe_acl_udf_profile_entry_get(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_acl_udf_profile_entry_t * entry, a_bool_t sign_tag)
{
	a_uint32_t idx, temp_profile_id;
	a_bool_t entry_valid;
	fal_acl_udf_profile_entry_t temp_entry;

	ADPT_DEV_ID_CHECK(dev_id);

	if (profile_id >= IPR_UDF_PROFILE_BASE_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	for (idx = 0; idx < IPR_UDF_CTRL_MAX_ENTRY; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_acl_udf_profile_entry_t));
		entry_valid = _adpt_appe_get_udf_profile_entry_by_index(dev_id, idx,
						&temp_entry, &temp_profile_id);
		if (entry_valid == A_TRUE)
		{
			if (temp_profile_id == profile_id)
			{
				if (sign_tag == A_TRUE)
				{
					aos_mem_copy(entry, &temp_entry,
						sizeof (fal_acl_udf_profile_entry_t));
					break;
				}
				if (A_TRUE == _adpt_appe_is_udf_profile_entry_equal(dev_id,
						&temp_entry, entry))
				{
					sign_tag = A_TRUE;
				}
			}
		}
	}
	if (idx == IPR_UDF_CTRL_MAX_ENTRY)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
_adpt_appe_pre_acl_counter_get(a_uint32_t dev_id,
			a_uint32_t entry_index, fal_entry_counter_t *acl_counter)
{
	sw_error_t rv = SW_OK;
	union pre_ipo_cnt_tbl_u pre_ipo_cnt = {0};

	rv = appe_pre_ipo_cnt_tbl_get(dev_id, entry_index, &pre_ipo_cnt);
	SW_RTN_ON_ERROR(rv);

	acl_counter->matched_pkts = pre_ipo_cnt.bf.hit_pkt_cnt;
	acl_counter->matched_bytes = pre_ipo_cnt.bf.hit_byte_cnt_0 |
		(a_uint64_t)pre_ipo_cnt.bf.hit_byte_cnt_1 << SW_FIELD_OFFSET_IN_WORD(
				PRE_IPO_CNT_TBL_HIT_BYTE_CNT_OFFSET);

	return rv;
}

sw_error_t
adpt_appe_acl_udf_profile_entry_add(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_acl_udf_profile_entry_t * entry)
{
	a_uint32_t entry_idx, temp_profile_id;
	a_bool_t entry_valid;
	a_int32_t idx;
	fal_acl_udf_profile_entry_t temp_entry;

	ADPT_DEV_ID_CHECK(dev_id);

	if (profile_id >= IPR_UDF_PROFILE_BASE_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	for (idx = IPR_UDF_CTRL_MAX_ENTRY-1; idx >= 0; idx--)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_acl_udf_profile_entry_t));
		entry_valid = _adpt_appe_get_udf_profile_entry_by_index(dev_id, idx,
						&temp_entry, &temp_profile_id);
		if (entry_valid == A_TRUE)
		{
			if (A_TRUE == _adpt_appe_is_udf_profile_entry_equal(dev_id,
						&temp_entry, entry))
			{
				if (temp_profile_id == profile_id)
				{
					return SW_ALREADY_EXIST;
				}
				else
				{
					SSDK_DEBUG("original profile %d, updated profile %d\n",
								temp_profile_id, profile_id);
					return _adpt_appe_insert_udf_profile_entry_by_index(dev_id,
								idx, entry, profile_id);
				}
			}
		}
		else
		{
			entry_idx = idx;
			break;
		}
	}
	if (idx < 0)
	{
		return SW_NO_RESOURCE;
	}
	/* insert new udf entry and sort the entries*/
	return _adpt_appe_insert_udf_profile_entry_by_sort(dev_id, entry_idx, entry, profile_id);
}

sw_error_t
adpt_appe_acl_udf_profile_entry_del(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_acl_udf_profile_entry_t * entry)
{
	a_uint32_t idx, temp_profile_id;
	a_int32_t j;
	a_bool_t entry_valid;
	fal_acl_udf_profile_entry_t temp_entry;
	union ipr_udf_ctrl_u udf_ctrl_zero_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if (profile_id >= IPR_UDF_PROFILE_BASE_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	for (idx = 0; idx < IPR_UDF_CTRL_MAX_ENTRY; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_acl_udf_profile_entry_t));
		entry_valid = _adpt_appe_get_udf_profile_entry_by_index(dev_id, idx,
						&temp_entry, &temp_profile_id);
		if (entry_valid == A_TRUE)
		{
			if (A_TRUE == _adpt_appe_is_udf_profile_entry_equal(dev_id,
						&temp_entry, entry))
			{
				if (temp_profile_id == profile_id)
				{
					/* find the entry*/
					break;
				}
			}
		}
	}

	if (idx == IPR_UDF_CTRL_MAX_ENTRY)
	{
		return SW_NOT_FOUND;
	}

	/* clear the find entry and resort the entries */
	for (j = idx; j > 0; j --)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_acl_udf_profile_entry_t));
		entry_valid = _adpt_appe_get_udf_profile_entry_by_index(dev_id, j-1,
						&temp_entry, &temp_profile_id);
		if (entry_valid == A_TRUE)
		{
			SW_RTN_ON_ERROR(_adpt_appe_insert_udf_profile_entry_by_index(dev_id, j,
						&temp_entry, temp_profile_id));
		}
		else
		{
			break;
		}
	}
	return appe_ipr_udf_ctrl_set(dev_id, j, &udf_ctrl_zero_entry);
}

sw_error_t
adpt_appe_acl_udf_profile_entry_getfirst(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_acl_udf_profile_entry_t * entry)
{
	return _adpt_appe_acl_udf_profile_entry_get(dev_id, profile_id, entry, A_TRUE);
}

sw_error_t
adpt_appe_acl_udf_profile_entry_getnext(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_acl_udf_profile_entry_t * entry)
{
	return _adpt_appe_acl_udf_profile_entry_get(dev_id, profile_id, entry, A_FALSE);
}

sw_error_t
adpt_appe_acl_udf_profile_cfg_set(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_acl_udf_type_t udf_type, a_uint32_t offset)
{
	a_uint8_t udf_base = 0;
	union ipr_udf_profile_base_u udf_profile_base = {0};
	union ipr_udf_profile_offset_u udf_profile_offset = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if (offset % 2)
	{ /*only support even data*/
		return SW_BAD_VALUE;
	}

	switch(udf_type)
	{
		case FAL_ACL_UDF_TYPE_L2:
			udf_base = 0;
			break;
		case FAL_ACL_UDF_TYPE_L3:
			udf_base = 1;
			break;
		case FAL_ACL_UDF_TYPE_L4:
			udf_base = 2;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	SW_RTN_ON_ERROR(appe_ipr_udf_profile_base_get(dev_id, profile_id, &udf_profile_base));
	SW_RTN_ON_ERROR(appe_ipr_udf_profile_offset_get(dev_id, profile_id, &udf_profile_offset));

	switch(udf_idx)
	{
		case 0:
			udf_profile_base.bf.udf0_base = udf_base;
			udf_profile_offset.bf.udf0_offset = offset/2;
			break;
		case 1:
			udf_profile_base.bf.udf1_base = udf_base;
			udf_profile_offset.bf.udf1_offset = offset/2;
			break;
		case 2:
			udf_profile_base.bf.udf2_base = udf_base;
			udf_profile_offset.bf.udf2_offset = offset/2;
			break;
		case 3:
			udf_profile_base.bf.udf3_base = udf_base;
			udf_profile_offset.bf.udf3_offset = offset/2;
			break;
		default:
			return SW_OUT_OF_RANGE;
	}

	SW_RTN_ON_ERROR(appe_ipr_udf_profile_base_set(dev_id, profile_id, &udf_profile_base));
	return appe_ipr_udf_profile_offset_set(dev_id, profile_id, &udf_profile_offset);
}

sw_error_t
adpt_appe_acl_udf_profile_cfg_get(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_acl_udf_type_t * udf_type, a_uint32_t * offset)
{
	a_uint8_t udf_base = 0;
	union ipr_udf_profile_base_u udf_profile_base = {0};
	union ipr_udf_profile_offset_u udf_profile_offset = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(udf_type);
	ADPT_NULL_POINT_CHECK(offset);

	SW_RTN_ON_ERROR(appe_ipr_udf_profile_base_get(dev_id, profile_id, &udf_profile_base));
	SW_RTN_ON_ERROR(appe_ipr_udf_profile_offset_get(dev_id, profile_id, &udf_profile_offset));

	switch(udf_idx)
	{
		case 0:
			udf_base = udf_profile_base.bf.udf0_base;
			*offset = udf_profile_offset.bf.udf0_offset*2;
			break;
		case 1:
			udf_base = udf_profile_base.bf.udf1_base;
			*offset = udf_profile_offset.bf.udf1_offset*2;
			break;
		case 2:
			udf_base = udf_profile_base.bf.udf2_base;
			*offset = udf_profile_offset.bf.udf2_offset*2;
			break;
		case 3:
			udf_base = udf_profile_base.bf.udf3_base;
			*offset = udf_profile_offset.bf.udf3_offset*2;
			break;
		default:
			return SW_OUT_OF_RANGE;
	}

	switch(udf_base)
	{
		case 0:
			*udf_type = FAL_ACL_UDF_TYPE_L2;
			break;
		case 1:
			*udf_type = FAL_ACL_UDF_TYPE_L3;
			break;
		case 2:
			*udf_type = FAL_ACL_UDF_TYPE_L4;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

sw_error_t
adpt_appe_acl_udf_profile_set(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type,
		a_uint32_t udf_idx, fal_acl_udf_type_t udf_type, a_uint32_t offset)
{
	sw_error_t rv;
	a_uint32_t profile_id;
	fal_acl_udf_profile_entry_t udf_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if (udf_idx >= ADPT_UDF_MAX_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	switch(pkt_type)
	{
		case FAL_ACL_UDF_NON_IP:
			profile_id = NON_IP_PROFILE_ID;
			udf_entry.l3_type = FAL_L3_TYPE_OTHERS;
			udf_entry.l4_type = FAL_L4_TYPE_OTHERS;
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
			break;
		case FAL_ACL_UDF_IP4:
			profile_id = IP4_PROFILE_ID;
			udf_entry.l3_type = FAL_L3_TYPE_IPV4;
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
			break;
		case FAL_ACL_UDF_IP6:
			profile_id = IP6_PROFILE_ID;
			udf_entry.l3_type = FAL_L3_TYPE_IPV6;
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	rv = adpt_appe_acl_udf_profile_entry_add(dev_id, profile_id, &udf_entry);
	/* if pkt udf entry is already exist, only update the udfprofile base and offset cfg */
	if(rv != SW_OK && rv != SW_ALREADY_EXIST)
	{
		return rv;
	}
	return adpt_appe_acl_udf_profile_cfg_set(dev_id, profile_id, udf_idx, udf_type, offset);
}

sw_error_t
adpt_appe_acl_udf_profile_get(a_uint32_t dev_id, fal_acl_udf_pkt_type_t pkt_type,
		a_uint32_t udf_idx, fal_acl_udf_type_t * udf_type, a_uint32_t * offset)
{
	a_uint32_t profile_id;
	a_bool_t exist = A_FALSE;
	fal_acl_udf_profile_entry_t udf_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(udf_type);
	ADPT_NULL_POINT_CHECK(offset);

	if (udf_idx >= ADPT_UDF_MAX_NUM)
	{
		return SW_OUT_OF_RANGE;
	}

	switch(pkt_type)
	{
		case FAL_ACL_UDF_NON_IP:
			profile_id = NON_IP_PROFILE_ID;
			udf_entry.l3_type = FAL_L3_TYPE_OTHERS;
			udf_entry.l4_type = FAL_L4_TYPE_OTHERS;
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
			break;
		case FAL_ACL_UDF_IP4:
			profile_id = IP4_PROFILE_ID;
			udf_entry.l3_type = FAL_L3_TYPE_IPV4;
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
			break;
		case FAL_ACL_UDF_IP6:
			profile_id = IP6_PROFILE_ID;
			udf_entry.l3_type = FAL_L3_TYPE_IPV6;
			FAL_FIELD_FLG_SET(udf_entry.field_flag,
				FAL_ACL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	/* check if pkt type udf entry is exist */
	exist = _adpt_appe_is_udf_profile_entry_exist(dev_id, profile_id, &udf_entry);
	if (exist == A_FALSE)
	{
		SSDK_DEBUG("pkt type udf entry is not exist!\n");
		return SW_NOT_FOUND;
	}
	return adpt_appe_acl_udf_profile_cfg_get(dev_id, profile_id, udf_idx, udf_type, offset);
}

sw_error_t
adpt_appe_acl_vpgroup_set(a_uint32_t dev_id, a_uint32_t vport_id,
       fal_vport_type_t vport_type, a_uint32_t vpgroup_id)
{
	if(vport_type == FAL_VPORT_TYPE_TUNNEL)
	{
#ifdef IN_TUNNEL
		return appe_tl_port_vp_tbl_pre_ipo_profile_set(dev_id,
				FAL_PORT_ID_VALUE(vport_id), vpgroup_id);
#endif
	}
	else if(vport_type == FAL_VPORT_TYPE_NORMAL)
	{
#ifdef IN_IP
		return appe_l3_vp_port_tbl_ipo_vp_profile_set(dev_id,
				FAL_PORT_ID_VALUE(vport_id), vpgroup_id);
#endif
	}

	return SW_BAD_VALUE;
}

sw_error_t
adpt_appe_acl_vpgroup_get(a_uint32_t dev_id, a_uint32_t vport_id,
       fal_vport_type_t vport_type, a_uint32_t * vpgroup_id)
{
	if(vport_type == FAL_VPORT_TYPE_TUNNEL)
	{
#ifdef IN_TUNNEL
		return appe_tl_port_vp_tbl_pre_ipo_profile_get(dev_id,
				FAL_PORT_ID_VALUE(vport_id), vpgroup_id);
#endif
	}
	else if(vport_type == FAL_VPORT_TYPE_NORMAL)
	{
#ifdef IN_IP
		return appe_l3_vp_port_tbl_ipo_vp_profile_get(dev_id,
				FAL_PORT_ID_VALUE(vport_id), vpgroup_id);
#endif
	}

	return SW_BAD_VALUE;
}
/**
 * @}
 */
