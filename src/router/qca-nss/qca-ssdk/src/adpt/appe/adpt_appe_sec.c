/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "fal_sec.h"
#include "appe_sec_reg.h"
#include "appe_sec.h"
#include "adpt.h"
#include "adpt_appe_sec.h"

#define L2_EXCEPTION_CTRL_MAX_ENTRY 1

enum {
    SEC_HDR_TYPE_VXLAN = 0,
    SEC_HDR_TYPE_VXLAN_GPE = 1,
    SEC_HDR_TYPE_GENEVE = 2,
    SEC_HDR_TYPE_GRE = 3,
    SEC_HDR_TYPE_MAX,
};

#ifndef IN_SEC_MINI
sw_error_t
adpt_appe_sec_l2_excep_ctrl_set(a_uint32_t dev_id, a_uint32_t excep_type, fal_l2_excep_ctrl_t *ctrl)
{
    union l2_excep_ctrl_u l2_exception_ctrl;

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);
    /* now only support FAL_SEC_L2_EXP_TUNL_CONTEXT_INVALID 0 */
    if (excep_type >= L2_EXCEPTION_CTRL_MAX_ENTRY)
        return SW_BAD_VALUE;

    l2_exception_ctrl.bf.tunnel_excep_fwd = ctrl->cmd;

    appe_l2_excep_ctrl_set(dev_id, &l2_exception_ctrl);

    return SW_OK;
}

sw_error_t
adpt_appe_sec_l2_excep_ctrl_get(a_uint32_t dev_id, a_uint32_t excep_type, fal_l2_excep_ctrl_t *ctrl)
{
    union l2_excep_ctrl_u l2_exception_ctrl;

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);
    /* now only support FAL_SEC_L2_EXP_TUNL_CONTEXT_INVALID 0 */
    if (excep_type >= L2_EXCEPTION_CTRL_MAX_ENTRY)
       return SW_BAD_VALUE;

    appe_l2_excep_ctrl_get(dev_id, &l2_exception_ctrl);

    ctrl->cmd = l2_exception_ctrl.bf.tunnel_excep_fwd;

    return SW_OK;
}

sw_error_t
adpt_appe_sec_tunnel_excep_ctrl_set(a_uint32_t dev_id, a_uint32_t excep_type, fal_tunnel_excep_ctrl_t *ctrl)
{
    union tl_exception_cmd_u tl_exception_cmd;
    union tl_exp_ctrl_profile0_u tl_exp_ctrl_profile0;
    union tl_exp_ctrl_profile1_u tl_exp_ctrl_profile1;
    union tl_exp_ctrl_profile2_u tl_exp_ctrl_profile2;
    union tl_exp_ctrl_profile3_u tl_exp_ctrl_profile3;

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);
    if (excep_type >= TL_EXCEPTION_CMD_MAX_ENTRY)
        return SW_BAD_VALUE;

    tl_exception_cmd.bf.tl_excep_cmd = ctrl->cmd;
    tl_exception_cmd.bf.de_acce = ctrl->deacclr_en;
    tl_exp_ctrl_profile0.bf.excep_en = ctrl->profile_exp_en[0];
    tl_exp_ctrl_profile1.bf.excep_en = ctrl->profile_exp_en[1];
    tl_exp_ctrl_profile2.bf.excep_en = ctrl->profile_exp_en[2];
    tl_exp_ctrl_profile3.bf.excep_en = ctrl->profile_exp_en[3];

    appe_tl_exception_cmd_set(dev_id, excep_type, &tl_exception_cmd);
    appe_tl_exp_ctrl_profile0_set(dev_id, excep_type, &tl_exp_ctrl_profile0);
    appe_tl_exp_ctrl_profile1_set(dev_id, excep_type, &tl_exp_ctrl_profile1);
    appe_tl_exp_ctrl_profile2_set(dev_id, excep_type, &tl_exp_ctrl_profile2);
    appe_tl_exp_ctrl_profile3_set(dev_id, excep_type, &tl_exp_ctrl_profile3);

    return SW_OK;
}

sw_error_t
adpt_appe_sec_tunnel_excep_ctrl_get(a_uint32_t dev_id, a_uint32_t excep_type, fal_tunnel_excep_ctrl_t *ctrl)
{
    union tl_exception_cmd_u tl_exception_cmd;
    union tl_exp_ctrl_profile0_u tl_exp_ctrl_profile0;
    union tl_exp_ctrl_profile1_u tl_exp_ctrl_profile1;
    union tl_exp_ctrl_profile2_u tl_exp_ctrl_profile2;
    union tl_exp_ctrl_profile3_u tl_exp_ctrl_profile3;

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);
    if (excep_type >= TL_EXCEPTION_CMD_MAX_ENTRY)
        return SW_BAD_VALUE;

    appe_tl_exception_cmd_get(dev_id, excep_type, &tl_exception_cmd);
    appe_tl_exp_ctrl_profile0_get(dev_id, excep_type, &tl_exp_ctrl_profile0);
    appe_tl_exp_ctrl_profile1_get(dev_id, excep_type, &tl_exp_ctrl_profile1);
    appe_tl_exp_ctrl_profile2_get(dev_id, excep_type, &tl_exp_ctrl_profile2);
    appe_tl_exp_ctrl_profile3_get(dev_id, excep_type, &tl_exp_ctrl_profile3);

    ctrl->cmd = tl_exception_cmd.bf.tl_excep_cmd;
    ctrl->deacclr_en = tl_exception_cmd.bf.de_acce;
    ctrl->profile_exp_en[0] = tl_exp_ctrl_profile0.bf.excep_en;
    ctrl->profile_exp_en[1] = tl_exp_ctrl_profile1.bf.excep_en;
    ctrl->profile_exp_en[2] = tl_exp_ctrl_profile2.bf.excep_en;
    ctrl->profile_exp_en[3] = tl_exp_ctrl_profile3.bf.excep_en;

    return SW_OK;
}

sw_error_t
adpt_appe_sec_tunnel_l3_excep_parser_ctrl_set(a_uint32_t dev_id, fal_l3_excep_parser_ctrl *ctrl)
{
    union tpr_l3_exception_parsing_ctrl_u l3_exception_parsing_ctrl;

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);

    l3_exception_parsing_ctrl.val = 0;

    l3_exception_parsing_ctrl.bf.small_ttl = ctrl->small_ip4ttl;
    l3_exception_parsing_ctrl.bf.small_hop_limit = ctrl->small_ip6hoplimit;

    return appe_tpr_l3_exception_parsing_ctrl_set(dev_id, &l3_exception_parsing_ctrl);
}

sw_error_t
adpt_appe_sec_tunnel_l3_excep_parser_ctrl_get(a_uint32_t dev_id, fal_l3_excep_parser_ctrl *ctrl)
{
    sw_error_t rv = SW_OK;
    union tpr_l3_exception_parsing_ctrl_u l3_exception_parsing_ctrl;

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);

    rv = appe_tpr_l3_exception_parsing_ctrl_get(dev_id, &l3_exception_parsing_ctrl);
    SW_RTN_ON_ERROR(rv);

    ctrl->small_ip4ttl = l3_exception_parsing_ctrl.bf.small_ttl;
    ctrl->small_ip6hoplimit = l3_exception_parsing_ctrl.bf.small_hop_limit;

    return SW_OK;
}

sw_error_t
adpt_appe_sec_tunnel_l4_excep_parser_ctrl_set(a_uint32_t dev_id, fal_l4_excep_parser_ctrl *ctrl)
{
    union tpr_l4_exception_parsing_ctrl_0_u l4_exception_parsing_ctrl_0;
    union tpr_l4_exception_parsing_ctrl_1_u l4_exception_parsing_ctrl_1;
    union tpr_l4_exception_parsing_ctrl_2_u l4_exception_parsing_ctrl_2;
    union tpr_l4_exception_parsing_ctrl_3_u l4_exception_parsing_ctrl_3;

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

    appe_tpr_l4_exception_parsing_ctrl_0_set(dev_id, &l4_exception_parsing_ctrl_0);
    appe_tpr_l4_exception_parsing_ctrl_1_set(dev_id, &l4_exception_parsing_ctrl_1);
    appe_tpr_l4_exception_parsing_ctrl_2_set(dev_id, &l4_exception_parsing_ctrl_2);
    appe_tpr_l4_exception_parsing_ctrl_3_set(dev_id, &l4_exception_parsing_ctrl_3);

    return SW_OK;
}

sw_error_t
adpt_appe_sec_tunnel_l4_excep_parser_ctrl_get(a_uint32_t dev_id, fal_l4_excep_parser_ctrl *ctrl)
{
    union tpr_l4_exception_parsing_ctrl_0_u l4_exception_parsing_ctrl_0;
    union tpr_l4_exception_parsing_ctrl_1_u l4_exception_parsing_ctrl_1;
    union tpr_l4_exception_parsing_ctrl_2_u l4_exception_parsing_ctrl_2;
    union tpr_l4_exception_parsing_ctrl_3_u l4_exception_parsing_ctrl_3;

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);

    appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &l4_exception_parsing_ctrl_0);
    appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &l4_exception_parsing_ctrl_1);
    appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &l4_exception_parsing_ctrl_2);
    appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &l4_exception_parsing_ctrl_3);

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
adpt_appe_sec_tunnel_flags_excep_parser_ctrl_set(a_uint32_t dev_id, a_uint32_t entry_index, fal_tunnel_flags_excep_parser_ctrl_t *ctrl)
{
    union tpr_exception_ctrl_0_u  flags_exception_parsing_ctrl_0;
    union tpr_exception_ctrl_1_u  flags_exception_parsing_ctrl_1;
    a_uint32_t  hdr_type_map[FAL_TUNNEL_OVERLAY_TYPE_BUTT] = {
                                                    SEC_HDR_TYPE_GRE,
                                                    SEC_HDR_TYPE_VXLAN,
                                                    SEC_HDR_TYPE_VXLAN_GPE,
                                                    SEC_HDR_TYPE_GENEVE};

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);
    if (entry_index >= TPR_EXCEPTION_CTRL_1_MAX_ENTRY)
        return SW_BAD_VALUE;
    if (ctrl->hdr_type >= FAL_TUNNEL_OVERLAY_TYPE_BUTT)
        return SW_BAD_VALUE;

    flags_exception_parsing_ctrl_0.bf.flags = ctrl->flags;
    flags_exception_parsing_ctrl_0.bf.mask = ctrl->mask;
    flags_exception_parsing_ctrl_1.bf.hdr_type = hdr_type_map[ctrl->hdr_type];
    flags_exception_parsing_ctrl_1.bf.comp_mode = ctrl->comp_mode;
    flags_exception_parsing_ctrl_1.bf.valid = ctrl->entry_valid;

    appe_tpr_exception_ctrl_0_set(dev_id, entry_index, &flags_exception_parsing_ctrl_0);
    appe_tpr_exception_ctrl_1_set(dev_id, entry_index, &flags_exception_parsing_ctrl_1);

    return SW_OK;
}

sw_error_t
adpt_appe_sec_tunnel_flags_excep_parser_ctrl_get(a_uint32_t dev_id, a_uint32_t entry_index, fal_tunnel_flags_excep_parser_ctrl_t *ctrl)
{

    union tpr_exception_ctrl_0_u  flags_exception_parsing_ctrl_0;
    union tpr_exception_ctrl_1_u  flags_exception_parsing_ctrl_1;
    fal_tunnel_overlay_type_t overlay_type_map[SEC_HDR_TYPE_MAX] = {
                                                  FAL_TUNNEL_OVERLAY_TYPE_VXLAN,
                                                  FAL_TUNNEL_OVERLAY_TYPE_VXLAN_GPE,
                                                  FAL_TUNNEL_OVERLAY_TYPE_GENEVE,
                                                  FAL_TUNNEL_OVERLAY_TYPE_GRE_TAP};

    ADPT_DEV_ID_CHECK(dev_id);
    ADPT_NULL_POINT_CHECK(ctrl);
    if (entry_index >= TPR_EXCEPTION_CTRL_1_MAX_ENTRY)
        return SW_BAD_VALUE;

    appe_tpr_exception_ctrl_0_get(dev_id, entry_index, &flags_exception_parsing_ctrl_0);
    appe_tpr_exception_ctrl_1_get(dev_id, entry_index, &flags_exception_parsing_ctrl_1);

    ctrl->flags = flags_exception_parsing_ctrl_0.bf.flags;
    ctrl->mask = flags_exception_parsing_ctrl_0.bf.mask;
    ctrl->hdr_type = overlay_type_map[flags_exception_parsing_ctrl_1.bf.hdr_type];
    ctrl->comp_mode = flags_exception_parsing_ctrl_1.bf.comp_mode;
    ctrl->entry_valid = flags_exception_parsing_ctrl_1.bf.valid;

    return SW_OK;
}
#endif

/**
 * @}
 */

