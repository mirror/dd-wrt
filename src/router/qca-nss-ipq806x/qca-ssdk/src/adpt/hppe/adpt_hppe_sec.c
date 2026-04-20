/*
 * Copyright (c) 2016-2017, 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "fal_sec.h"
#include "hppe_sec_reg.h"
#include "hppe_sec.h"
#include "adpt.h"
#if defined(APPE)
#include "appe_sec_reg.h"
#include "appe_sec.h"
#include "adpt_appe_sec.h"
#endif

#ifndef IN_SEC_MINI
sw_error_t
adpt_hppe_sec_l3_excep_parser_ctrl_set(a_uint32_t dev_id, fal_l3_excep_parser_ctrl *ctrl)
{
	union l3_exception_parsing_ctrl_reg_u l3_exception_parsing_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	memset(&l3_exception_parsing_ctrl, 0, sizeof(l3_exception_parsing_ctrl));

	l3_exception_parsing_ctrl.bf.small_ttl = ctrl->small_ip4ttl;
	l3_exception_parsing_ctrl.bf.small_hop_limit = ctrl->small_ip6hoplimit;

	return hppe_l3_exception_parsing_ctrl_reg_set(dev_id, &l3_exception_parsing_ctrl);
}

sw_error_t
adpt_hppe_sec_l3_excep_parser_ctrl_get(a_uint32_t dev_id, fal_l3_excep_parser_ctrl *ctrl)
{
	sw_error_t rv = SW_OK;
	union l3_exception_parsing_ctrl_reg_u l3_exception_parsing_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	rv = hppe_l3_exception_parsing_ctrl_reg_get(dev_id, &l3_exception_parsing_ctrl);
	if( rv != SW_OK )
		return rv;

	ctrl->small_ip4ttl = l3_exception_parsing_ctrl.bf.small_ttl;
	ctrl->small_ip6hoplimit = l3_exception_parsing_ctrl.bf.small_hop_limit;

	return SW_OK;
}
#endif

sw_error_t
adpt_hppe_sec_l4_excep_parser_ctrl_set(a_uint32_t dev_id, fal_l4_excep_parser_ctrl *ctrl)
{
	union l4_exception_parsing_ctrl_0_reg_u l4_exception_parsing_ctrl_0;
	union l4_exception_parsing_ctrl_1_reg_u l4_exception_parsing_ctrl_1;
	union l4_exception_parsing_ctrl_2_reg_u l4_exception_parsing_ctrl_2;
	union l4_exception_parsing_ctrl_3_reg_u l4_exception_parsing_ctrl_3;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	l4_exception_parsing_ctrl_0.bf.tcp_flags0 = ctrl->tcp_flags[0];
	l4_exception_parsing_ctrl_0.bf.tcp_flags0_mask = ctrl->tcp_flags_mask[0];
	l4_exception_parsing_ctrl_0.bf.tcp_flags1 = ctrl->tcp_flags[1];
	l4_exception_parsing_ctrl_0.bf.tcp_flags1_mask = ctrl->tcp_flags_mask[1];
	l4_exception_parsing_ctrl_1.bf.tcp_flags2 = ctrl->tcp_flags[2];
	l4_exception_parsing_ctrl_1.bf.tcp_flags2_mask = ctrl->tcp_flags_mask[2];
	l4_exception_parsing_ctrl_1.bf.tcp_flags3 = ctrl->tcp_flags[3];
	l4_exception_parsing_ctrl_1.bf.tcp_flags3_mask = ctrl->tcp_flags_mask[3];
	l4_exception_parsing_ctrl_2.bf.tcp_flags4 = ctrl->tcp_flags[4];
	l4_exception_parsing_ctrl_2.bf.tcp_flags4_mask = ctrl->tcp_flags_mask[4];
	l4_exception_parsing_ctrl_2.bf.tcp_flags5 = ctrl->tcp_flags[5];
	l4_exception_parsing_ctrl_2.bf.tcp_flags5_mask = ctrl->tcp_flags_mask[5];
	l4_exception_parsing_ctrl_3.bf.tcp_flags6 = ctrl->tcp_flags[6];
	l4_exception_parsing_ctrl_3.bf.tcp_flags6_mask = ctrl->tcp_flags_mask[6];
	l4_exception_parsing_ctrl_3.bf.tcp_flags7 = ctrl->tcp_flags[7];
	l4_exception_parsing_ctrl_3.bf.tcp_flags7_mask = ctrl->tcp_flags_mask[7];

	hppe_l4_exception_parsing_ctrl_0_reg_set(dev_id, &l4_exception_parsing_ctrl_0);
	hppe_l4_exception_parsing_ctrl_1_reg_set(dev_id, &l4_exception_parsing_ctrl_1);
	hppe_l4_exception_parsing_ctrl_2_reg_set(dev_id, &l4_exception_parsing_ctrl_2);
	hppe_l4_exception_parsing_ctrl_3_reg_set(dev_id, &l4_exception_parsing_ctrl_3);
	return SW_OK;
}

sw_error_t
adpt_hppe_sec_l4_excep_parser_ctrl_get(a_uint32_t dev_id, fal_l4_excep_parser_ctrl *ctrl)
{
	union l4_exception_parsing_ctrl_0_reg_u l4_exception_parsing_ctrl_0;
	union l4_exception_parsing_ctrl_1_reg_u l4_exception_parsing_ctrl_1;
	union l4_exception_parsing_ctrl_2_reg_u l4_exception_parsing_ctrl_2;
	union l4_exception_parsing_ctrl_3_reg_u l4_exception_parsing_ctrl_3;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	hppe_l4_exception_parsing_ctrl_0_reg_get(dev_id, &l4_exception_parsing_ctrl_0);
	hppe_l4_exception_parsing_ctrl_1_reg_get(dev_id, &l4_exception_parsing_ctrl_1);
	hppe_l4_exception_parsing_ctrl_2_reg_get(dev_id, &l4_exception_parsing_ctrl_2);
	hppe_l4_exception_parsing_ctrl_3_reg_get(dev_id, &l4_exception_parsing_ctrl_3);

	ctrl->tcp_flags[0] = l4_exception_parsing_ctrl_0.bf.tcp_flags0;
	ctrl->tcp_flags_mask[0] = l4_exception_parsing_ctrl_0.bf.tcp_flags0_mask;
	ctrl->tcp_flags[1] = l4_exception_parsing_ctrl_0.bf.tcp_flags1;
	ctrl->tcp_flags_mask[1] = l4_exception_parsing_ctrl_0.bf.tcp_flags1_mask;
	ctrl->tcp_flags[2] = l4_exception_parsing_ctrl_1.bf.tcp_flags2;
	ctrl->tcp_flags_mask[2] = l4_exception_parsing_ctrl_1.bf.tcp_flags2_mask;
	ctrl->tcp_flags[3] = l4_exception_parsing_ctrl_1.bf.tcp_flags3;
	ctrl->tcp_flags_mask[3] = l4_exception_parsing_ctrl_1.bf.tcp_flags3_mask;
	ctrl->tcp_flags[4] = l4_exception_parsing_ctrl_2.bf.tcp_flags4;
	ctrl->tcp_flags_mask[4] = l4_exception_parsing_ctrl_2.bf.tcp_flags4_mask;
	ctrl->tcp_flags[5] = l4_exception_parsing_ctrl_2.bf.tcp_flags5;
	ctrl->tcp_flags_mask[5] = l4_exception_parsing_ctrl_2.bf.tcp_flags5_mask;
	ctrl->tcp_flags[6] = l4_exception_parsing_ctrl_3.bf.tcp_flags6;
	ctrl->tcp_flags_mask[6] = l4_exception_parsing_ctrl_3.bf.tcp_flags6_mask;
	ctrl->tcp_flags[7] = l4_exception_parsing_ctrl_3.bf.tcp_flags7;
	ctrl->tcp_flags_mask[7] = l4_exception_parsing_ctrl_3.bf.tcp_flags7_mask;

	return SW_OK;
}

sw_error_t
adpt_hppe_sec_l3_excep_ctrl_set(a_uint32_t dev_id, a_uint32_t excep_type, fal_l3_excep_ctrl_t *ctrl)
{
	union l3_exception_cmd_u l3_exception_cmd;
	union l3_exp_l3_only_ctrl_u l3_only_ctrl;
	union l3_exp_l2_only_ctrl_u l2_only_ctrl;
	union l3_exp_l2_flow_ctrl_u l2_flow_ctrl;
	union l3_exp_l3_flow_ctrl_u l3_flow_ctrl;
	union l3_exp_multicast_ctrl_u multicast_ctrl;
#if defined(APPE)
	union l2_flow_hit_exp_ctrl_u  l2_flow_hit_ctrl;
	union l3_flow_hit_exp_ctrl_u  l3_flow_hit_ctrl;
	union l2_flow_hit_miss_exp_ctrl_u l2_flow_miss_ctrl;
	union l3_flow_hit_miss_exp_ctrl_u l3_flow_miss_ctrl;
#endif

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);
	if (excep_type >= L3_EXCEPTION_CMD_MAX_ENTRY)
		return SW_BAD_VALUE;

	l3_exception_cmd.bf.l3_excep_cmd= ctrl->cmd;
	l3_exception_cmd.bf.de_acce= ctrl->deacclr_en;
	l3_only_ctrl.bf.excep_en = ctrl->l3route_only_en;
	l2_only_ctrl.bf.excep_en = ctrl->l2fwd_only_en;
	multicast_ctrl.bf.excep_en = ctrl->multicast_en;

#if defined(APPE)
	if(ctrl->l3flow_en) {
		if (FAL_FLOW_HIT == ctrl->l3flow_type){
			l3_flow_hit_ctrl.bf.excep_en = 1;
			l3_flow_miss_ctrl.bf.excep_en = 0;
			l3_flow_ctrl.bf.excep_en = 0;
		}
		else if (FAL_FLOW_MISS == ctrl->l3flow_type){
			l3_flow_miss_ctrl.bf.excep_en = 1;
			l3_flow_hit_ctrl.bf.excep_en = 0;
			l3_flow_ctrl.bf.excep_en = 0;
		}
		else {
			l3_flow_ctrl.bf.excep_en = 1;
			l3_flow_hit_ctrl.bf.excep_en = 1;
			l3_flow_miss_ctrl.bf.excep_en = 1;
		}
	}
	else {
		l3_flow_ctrl.bf.excep_en = 0;
		l3_flow_hit_ctrl.bf.excep_en = 0;
		l3_flow_miss_ctrl.bf.excep_en = 0;
	}

	if(ctrl->l2flow_en) {
		if (FAL_FLOW_HIT == ctrl->l2flow_type){
			l2_flow_hit_ctrl.bf.excep_en = 1;
			l2_flow_miss_ctrl.bf.excep_en = 0;
			l2_flow_ctrl.bf.excep_en = 0;
		}
		else if (FAL_FLOW_MISS == ctrl->l2flow_type){
			l2_flow_miss_ctrl.bf.excep_en = 1;
			l2_flow_hit_ctrl.bf.excep_en = 0;
			l2_flow_ctrl.bf.excep_en = 0;
		}
		else {
			l2_flow_ctrl.bf.excep_en = 1;
			l2_flow_hit_ctrl.bf.excep_en = 1;
			l2_flow_miss_ctrl.bf.excep_en = 1;
		}
	}
	else {
		l2_flow_ctrl.bf.excep_en = 0;
		l2_flow_hit_ctrl.bf.excep_en = 0;
		l2_flow_miss_ctrl.bf.excep_en = 0;
	}
#else
	l3_flow_ctrl.bf.excep_en = ctrl->l3flow_en;
	l2_flow_ctrl.bf.excep_en = ctrl->l2flow_en;
#endif

	hppe_l3_exception_cmd_set(dev_id, excep_type, &l3_exception_cmd);
	hppe_l3_exp_l3_only_ctrl_set(dev_id, excep_type, &l3_only_ctrl);
	hppe_l3_exp_l2_only_ctrl_set(dev_id, excep_type, &l2_only_ctrl);
	hppe_l3_exp_l3_flow_ctrl_set(dev_id, excep_type, &l3_flow_ctrl);
	hppe_l3_exp_l2_flow_ctrl_set(dev_id, excep_type, &l2_flow_ctrl);
	hppe_l3_exp_multicast_ctrl_set(dev_id, excep_type, &multicast_ctrl);
#if defined(APPE)
	appe_l3_flow_hit_exp_ctrl_set(dev_id, excep_type, &l3_flow_hit_ctrl);
	appe_l3_flow_hit_miss_exp_ctrl_set(dev_id, excep_type, &l3_flow_miss_ctrl);
	appe_l2_flow_hit_exp_ctrl_set(dev_id, excep_type, &l2_flow_hit_ctrl);
	appe_l2_flow_hit_miss_exp_ctrl_set(dev_id, excep_type, &l2_flow_miss_ctrl);
#endif
	return SW_OK;
}

sw_error_t
adpt_hppe_sec_l3_excep_ctrl_get(a_uint32_t dev_id, a_uint32_t excep_type, fal_l3_excep_ctrl_t *ctrl)
{
	union l3_exception_cmd_u l3_exception_cmd;
	union l3_exp_l3_only_ctrl_u l3_only_ctrl;
	union l3_exp_l2_only_ctrl_u l2_only_ctrl;
	union l3_exp_l2_flow_ctrl_u l2_flow_ctrl;
	union l3_exp_l3_flow_ctrl_u l3_flow_ctrl;
	union l3_exp_multicast_ctrl_u multicast_ctrl;
#if defined(APPE)
	union l2_flow_hit_exp_ctrl_u  l2_flow_hit_ctrl;
	union l3_flow_hit_exp_ctrl_u  l3_flow_hit_ctrl;
	union l2_flow_hit_miss_exp_ctrl_u l2_flow_miss_ctrl;
	union l3_flow_hit_miss_exp_ctrl_u l3_flow_miss_ctrl;
#endif

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);
	if (excep_type >= L3_EXCEPTION_CMD_MAX_ENTRY)
		return SW_BAD_VALUE;

	hppe_l3_exception_cmd_get(dev_id, excep_type, &l3_exception_cmd);
	hppe_l3_exp_l3_only_ctrl_get(dev_id, excep_type, &l3_only_ctrl);
	hppe_l3_exp_l2_only_ctrl_get(dev_id, excep_type, &l2_only_ctrl);
	hppe_l3_exp_l3_flow_ctrl_get(dev_id, excep_type, &l3_flow_ctrl);
	hppe_l3_exp_l2_flow_ctrl_get(dev_id, excep_type, &l2_flow_ctrl);
	hppe_l3_exp_multicast_ctrl_get(dev_id, excep_type, &multicast_ctrl);
#if defined(APPE)
	appe_l3_flow_hit_exp_ctrl_get(dev_id, excep_type, &l3_flow_hit_ctrl);
	appe_l3_flow_hit_miss_exp_ctrl_get(dev_id, excep_type, &l3_flow_miss_ctrl);
	appe_l2_flow_hit_exp_ctrl_get(dev_id, excep_type, &l2_flow_hit_ctrl);
	appe_l2_flow_hit_miss_exp_ctrl_get(dev_id, excep_type, &l2_flow_miss_ctrl);

	if (1 == l3_flow_ctrl.bf.excep_en) {
		ctrl->l3flow_en = A_TRUE;
		ctrl->l3flow_type = FAL_FLOW_AWARE;
	}
	else {
		if (1 == l3_flow_hit_ctrl.bf.excep_en){
			ctrl->l3flow_en = A_TRUE;
			ctrl->l3flow_type = FAL_FLOW_HIT;
		}
		else if (1 == l3_flow_miss_ctrl.bf.excep_en){
			ctrl->l3flow_en = A_TRUE;
			ctrl->l3flow_type = FAL_FLOW_MISS;
		}
		else {
			ctrl->l3flow_en = A_FALSE;
			ctrl->l3flow_type = FAL_FLOW_AWARE;
		}
	}

	if (1 == l2_flow_ctrl.bf.excep_en) {
		ctrl->l2flow_en = A_TRUE;
		ctrl->l2flow_type = FAL_FLOW_AWARE;
	}
	else {
		if (1 == l2_flow_hit_ctrl.bf.excep_en){
			ctrl->l2flow_en = A_TRUE;
			ctrl->l2flow_type = FAL_FLOW_HIT;
		}
		else if (1 == l2_flow_miss_ctrl.bf.excep_en){
			ctrl->l2flow_en = A_TRUE;
			ctrl->l2flow_type = FAL_FLOW_MISS;
		}
		else {
			ctrl->l2flow_en = A_FALSE;
			ctrl->l2flow_type = FAL_FLOW_AWARE;
		}
	}
#else
	ctrl->l3flow_en = l3_flow_ctrl.bf.excep_en;
	ctrl->l2flow_en = l2_flow_ctrl.bf.excep_en;
#endif
	ctrl->cmd = l3_exception_cmd.bf.l3_excep_cmd;
	ctrl->deacclr_en = l3_exception_cmd.bf.de_acce;
	ctrl->l3route_only_en = l3_only_ctrl.bf.excep_en;
	ctrl->l2fwd_only_en = l2_only_ctrl.bf.excep_en;
	ctrl->multicast_en = multicast_ctrl.bf.excep_en;

	return SW_OK;
}

sw_error_t adpt_hppe_sec_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_sec_l4_excep_parser_ctrl_set = adpt_hppe_sec_l4_excep_parser_ctrl_set;
	p_adpt_api->adpt_sec_l4_excep_parser_ctrl_get = adpt_hppe_sec_l4_excep_parser_ctrl_get;
	p_adpt_api->adpt_sec_l3_excep_ctrl_set = adpt_hppe_sec_l3_excep_ctrl_set;
	p_adpt_api->adpt_sec_l3_excep_ctrl_get = adpt_hppe_sec_l3_excep_ctrl_get;
#ifndef IN_SEC_MINI
	p_adpt_api->adpt_sec_l3_excep_parser_ctrl_set = adpt_hppe_sec_l3_excep_parser_ctrl_set;
	p_adpt_api->adpt_sec_l3_excep_parser_ctrl_get = adpt_hppe_sec_l3_excep_parser_ctrl_get;
#if defined(APPE)
	p_adpt_api->adpt_sec_l2_excep_ctrl_set = adpt_appe_sec_l2_excep_ctrl_set;
	p_adpt_api->adpt_sec_l2_excep_ctrl_get = adpt_appe_sec_l2_excep_ctrl_get;
	p_adpt_api->adpt_sec_tunnel_excep_ctrl_set = adpt_appe_sec_tunnel_excep_ctrl_set;
	p_adpt_api->adpt_sec_tunnel_excep_ctrl_get = adpt_appe_sec_tunnel_excep_ctrl_get;
	p_adpt_api->adpt_sec_tunnel_l3_excep_parser_ctrl_set = adpt_appe_sec_tunnel_l3_excep_parser_ctrl_set;
	p_adpt_api->adpt_sec_tunnel_l3_excep_parser_ctrl_get = adpt_appe_sec_tunnel_l3_excep_parser_ctrl_get;
	p_adpt_api->adpt_sec_tunnel_l4_excep_parser_ctrl_set = adpt_appe_sec_tunnel_l4_excep_parser_ctrl_set;
	p_adpt_api->adpt_sec_tunnel_l4_excep_parser_ctrl_get = adpt_appe_sec_tunnel_l4_excep_parser_ctrl_get;
	p_adpt_api->adpt_sec_tunnel_flags_excep_parser_ctrl_set = adpt_appe_sec_tunnel_flags_excep_parser_ctrl_set;
	p_adpt_api->adpt_sec_tunnel_flags_excep_parser_ctrl_get = adpt_appe_sec_tunnel_flags_excep_parser_ctrl_get;
#endif
#endif

	return SW_OK;
}

/**
 * @}
 */
