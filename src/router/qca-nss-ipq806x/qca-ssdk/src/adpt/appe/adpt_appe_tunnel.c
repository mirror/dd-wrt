/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "fal_tunnel.h"
#include "appe_tunnel_reg.h"
#include "appe_tunnel.h"
#include "appe_portvlan_reg.h"
#include "appe_portvlan.h"
#include "hppe_ip_reg.h"
#include "hppe_ip.h"
#include "appe_l2_vp_reg.h"
#include "appe_l2_vp.h"
#include "adpt.h"

#define TUNNEL_ENCAP_FROM_VP	0
#define TUNNEL_ENCAP_FROM_L3IF	1

#define TUNNEL_DECAP_TYPE_CHECK(tunnel_type) \
do { \
    if (tunnel_type >= FAL_TUNNEL_TYPE_INVALID_TUNNEL) \
        return SW_BAD_PARAM; \
} while (0)

#define TUNNEL_DECAP_ID_CHECK(entry_index) \
do { \
    if (entry_index >= TL_TBL_NUM) \
        return SW_OUT_OF_RANGE; \
} while (0)

#define TUNNEL_ENCAP_ID_CHECK(tunnel_id) \
do { \
    if (tunnel_id >= EG_XLAT_TUN_CTRL_NUM) \
        return SW_OUT_OF_RANGE; \
} while (0)

static sw_error_t
appe_tunnel_op_common(a_uint32_t dev_id,
		fal_tunnel_op_type_t op_type,
		fal_tunnel_op_mode_t op_mode,
		a_uint32_t *index)
{
	union tl_tbl_op_u tl_tbl_op;
	sw_error_t rv = SW_OK;
	a_uint32_t loop = 64;

	if (FAL_TUNNEL_OP_TYPE_GET == op_type) {
		SSDK_ERROR("%s bad op type: %d\n", __func__, op_type);
		return SW_BAD_PARAM;
	}

	aos_mem_zero(&tl_tbl_op, sizeof(union tl_tbl_op_u));

	tl_tbl_op.bf.op_type = op_type;
	tl_tbl_op.bf.hash_block_bitmap = 0x3;
	tl_tbl_op.bf.op_mode = op_mode;
	tl_tbl_op.bf.busy = 1;
	tl_tbl_op.bf.entry_index = *index;

	rv = appe_tl_tbl_op_set(dev_id, &tl_tbl_op);
	SW_RTN_ON_ERROR(rv);

	while (loop > 0) {
		rv = appe_tl_tbl_op_get(dev_id, &tl_tbl_op);
		SW_RTN_ON_ERROR(rv);

		if (tl_tbl_op.bf.busy ==0) {
			if (tl_tbl_op.bf.op_rslt == FAL_TUNNEL_OP_RSLT_OK)
				*index = tl_tbl_op.bf.entry_index;
			else
				rv = SW_FAIL;
			SSDK_DEBUG("%s rv: %d entry index: %d loop times: %d\n",
					__func__, rv, tl_tbl_op.bf.entry_index, loop);
			break;
		}
		loop--;
	}

	return rv;
}

static sw_error_t
appe_tunnel_rd_op_common(a_uint32_t dev_id,
		fal_tunnel_op_type_t op_type,
		fal_tunnel_op_mode_t op_mode,
		a_uint32_t *index)
{
	union tl_tbl_rd_op_u tl_tbl_rd_op;
	sw_error_t rv = SW_OK;
	a_uint32_t loop = 64;

	if (FAL_TUNNEL_OP_TYPE_GET != op_type) {
		SSDK_ERROR("%s bad op type: %d\n", __func__, op_type);
		return SW_BAD_PARAM;
	}

	aos_mem_zero(&tl_tbl_rd_op, sizeof(union tl_tbl_rd_op_u));

	tl_tbl_rd_op.bf.op_type = op_type;
	tl_tbl_rd_op.bf.hash_block_bitmap = 0x3;
	tl_tbl_rd_op.bf.op_mode = op_mode;
	tl_tbl_rd_op.bf.busy = 1;
	tl_tbl_rd_op.bf.entry_index = *index;

	rv = appe_tl_tbl_rd_op_set(dev_id, &tl_tbl_rd_op);
	SW_RTN_ON_ERROR(rv);

	while (loop > 0) {
		rv = appe_tl_tbl_rd_op_get(dev_id, &tl_tbl_rd_op);
		SW_RTN_ON_ERROR(rv);

		if (tl_tbl_rd_op.bf.busy ==0) {
			if (tl_tbl_rd_op.bf.op_rslt == FAL_TUNNEL_OP_RSLT_OK)
				*index = tl_tbl_rd_op.bf.entry_index;
			else
				rv = SW_FAIL;
			SSDK_DEBUG("%s rv: %d entry index: %d loop times: %d\n",
					__func__, rv, tl_tbl_rd_op.bf.entry_index, loop);
			break;
		}
		loop--;
	}

	return rv;
}

static sw_error_t
appe_tunnel_decap_entry_op(a_uint32_t dev_id,
		fal_tunnel_op_type_t op_type,
		fal_tunnel_op_mode_t op_mode,
		union tl_tbl_u *tl_tbl,
		a_uint32_t *index)
{
	sw_error_t rv = SW_OK;
	switch (op_type) {
		case FAL_TUNNEL_OP_TYPE_ADD:
		case FAL_TUNNEL_OP_TYPE_DEL:
			if (op_type == FAL_TUNNEL_OP_TYPE_ADD ||
					FAL_TUNNEL_OP_MODE_HASH == op_mode) {
				rv = appe_tl_tbl_op_data0_set(dev_id,
						(union tl_tbl_op_data0_u *)(&tl_tbl->val[0]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data1_set(dev_id,
						(union tl_tbl_op_data1_u *)(&tl_tbl->val[1]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data2_set(dev_id,
						(union tl_tbl_op_data2_u *)(&tl_tbl->val[2]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data3_set(dev_id,
						(union tl_tbl_op_data3_u *)(&tl_tbl->val[3]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data4_set(dev_id,
						(union tl_tbl_op_data4_u *)(&tl_tbl->val[4]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data5_set(dev_id,
						(union tl_tbl_op_data5_u *)(&tl_tbl->val[5]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data6_set(dev_id,
						(union tl_tbl_op_data6_u *)(&tl_tbl->val[6]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data7_set(dev_id,
						(union tl_tbl_op_data7_u *)(&tl_tbl->val[7]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data8_set(dev_id,
						(union tl_tbl_op_data8_u *)(&tl_tbl->val[8]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data9_set(dev_id,
						(union tl_tbl_op_data9_u *)(&tl_tbl->val[9]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data10_set(dev_id,
						(union tl_tbl_op_data10_u *)(&tl_tbl->val[10]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data11_set(dev_id,
						(union tl_tbl_op_data11_u *)(&tl_tbl->val[11]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data12_set(dev_id,
						(union tl_tbl_op_data12_u *)(&tl_tbl->val[12]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_op_data13_set(dev_id,
						(union tl_tbl_op_data13_u *)(&tl_tbl->val[13]));
				SW_RTN_ON_ERROR(rv);
			}

			rv = appe_tunnel_op_common(dev_id, op_type, op_mode, index);
			SW_RTN_ON_ERROR(rv);
			break;

		case FAL_TUNNEL_OP_TYPE_FLUSH:
			rv = appe_tunnel_op_common(dev_id, op_type, op_mode, index);
			SW_RTN_ON_ERROR(rv);
			break;
		case FAL_TUNNEL_OP_TYPE_GET:
			if (FAL_TUNNEL_OP_MODE_HASH == op_mode) {
				rv = appe_tl_tbl_rd_op_data0_set(dev_id,
						(union tl_tbl_rd_op_data0_u *)(&tl_tbl->val[0]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data1_set(dev_id,
						(union tl_tbl_rd_op_data1_u *)(&tl_tbl->val[1]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data2_set(dev_id,
						(union tl_tbl_rd_op_data2_u *)(&tl_tbl->val[2]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data3_set(dev_id,
						(union tl_tbl_rd_op_data3_u *)(&tl_tbl->val[3]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data4_set(dev_id,
						(union tl_tbl_rd_op_data4_u *)(&tl_tbl->val[4]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data5_set(dev_id,
						(union tl_tbl_rd_op_data5_u *)(&tl_tbl->val[5]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data6_set(dev_id,
						(union tl_tbl_rd_op_data6_u *)(&tl_tbl->val[6]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data7_set(dev_id,
						(union tl_tbl_rd_op_data7_u *)(&tl_tbl->val[7]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data8_set(dev_id,
						(union tl_tbl_rd_op_data8_u *)(&tl_tbl->val[8]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data9_set(dev_id,
						(union tl_tbl_rd_op_data9_u *)(&tl_tbl->val[9]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data10_set(dev_id,
						(union tl_tbl_rd_op_data10_u *)(&tl_tbl->val[10]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data11_set(dev_id,
						(union tl_tbl_rd_op_data11_u *)(&tl_tbl->val[11]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data12_set(dev_id,
						(union tl_tbl_rd_op_data12_u *)(&tl_tbl->val[12]));
				SW_RTN_ON_ERROR(rv);
				rv = appe_tl_tbl_rd_op_data13_set(dev_id,
						(union tl_tbl_rd_op_data13_u *)(&tl_tbl->val[13]));
				SW_RTN_ON_ERROR(rv);
			}

			rv = appe_tunnel_rd_op_common(dev_id, op_type, op_mode, index);
			SW_RTN_ON_ERROR(rv);

			rv = appe_tl_tbl_rd_rslt_data0_get(dev_id,
					(union tl_tbl_rd_rslt_data0_u *)(&tl_tbl->val[0]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data1_get(dev_id,
					(union tl_tbl_rd_rslt_data1_u *)(&tl_tbl->val[1]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data2_get(dev_id,
					(union tl_tbl_rd_rslt_data2_u *)(&tl_tbl->val[2]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data3_get(dev_id,
					(union tl_tbl_rd_rslt_data3_u *)(&tl_tbl->val[3]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data4_get(dev_id,
					(union tl_tbl_rd_rslt_data4_u *)(&tl_tbl->val[4]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data5_get(dev_id,
					(union tl_tbl_rd_rslt_data5_u *)(&tl_tbl->val[5]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data6_get(dev_id,
					(union tl_tbl_rd_rslt_data6_u *)(&tl_tbl->val[6]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data7_get(dev_id,
					(union tl_tbl_rd_rslt_data7_u *)(&tl_tbl->val[7]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data8_get(dev_id,
					(union tl_tbl_rd_rslt_data8_u *)(&tl_tbl->val[8]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data9_get(dev_id,
					(union tl_tbl_rd_rslt_data9_u *)(&tl_tbl->val[9]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data10_get(dev_id,
					(union tl_tbl_rd_rslt_data10_u *)(&tl_tbl->val[10]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data11_get(dev_id,
					(union tl_tbl_rd_rslt_data11_u *)(&tl_tbl->val[11]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data12_get(dev_id,
					(union tl_tbl_rd_rslt_data12_u *)(&tl_tbl->val[12]));
			SW_RTN_ON_ERROR(rv);
			rv = appe_tl_tbl_rd_rslt_data13_get(dev_id,
					(union tl_tbl_rd_rslt_data13_u *)(&tl_tbl->val[13]));
			SW_RTN_ON_ERROR(rv);

			break;
		default:
			SSDK_ERROR("%s bad op type: %d\n", __func__, op_type);
			break;
	}

	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_entry_convert(a_uint32_t dev_id, fal_tunnel_decap_entry_t *entry,
		union tl_tbl_u *tl_entry, a_bool_t to_hsl)
{
	fal_tunnel_rule_t *rule_key;
	fal_tunnel_action_t *entry_action;
	union tl_tbl_u tl_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);
	ADPT_NULL_POINT_CHECK(tl_entry);

	aos_mem_zero(&tl_tbl, sizeof(union tl_tbl_u));

	rule_key = &entry->decap_rule;
	entry_action = &entry->decap_action;

	if (to_hsl) {
		tl_tbl.bf0.valid = A_TRUE;
		/* tunnel tbl key to get based on hash start */
		tl_tbl.bf0.entry_type = rule_key->ip_ver;
		tl_tbl.bf0.key_type = rule_key->tunnel_type;
		tl_tbl.bf0.protocol = rule_key->l4_proto;

		if (rule_key->ip_ver) {
			tl_tbl.bf0.ipv6_src_addr_0 = rule_key->sip.ip6_addr.ul[3] &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET));

			tl_tbl.bf0.ipv6_src_addr_1 = (rule_key->sip.ip6_addr.ul[3] >>
					SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET)) |
				rule_key->sip.ip6_addr.ul[2] << TL_TBL_IPV6_SRC_ADDR_OFFSET % 32;

			tl_tbl.bf0.ipv6_src_addr_2 = (rule_key->sip.ip6_addr.ul[2] >>
					SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET)) |
				rule_key->sip.ip6_addr.ul[1] << TL_TBL_IPV6_SRC_ADDR_OFFSET % 32;

			tl_tbl.bf0.ipv6_src_addr_3 = (rule_key->sip.ip6_addr.ul[1] >>
					SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET)) |
				rule_key->sip.ip6_addr.ul[0] << TL_TBL_IPV6_SRC_ADDR_OFFSET % 32;

			tl_tbl.bf0.ipv6_src_addr_4 = rule_key->sip.ip6_addr.ul[0] >>
				SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET);

			tl_tbl.bf0.ipv6_dst_addr_0 = rule_key->dip.ip6_addr.ul[3] &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET));

			tl_tbl.bf0.ipv6_dst_addr_1 = (rule_key->dip.ip6_addr.ul[3] >>
					SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET)) |
				rule_key->dip.ip6_addr.ul[2] << TL_TBL_IPV6_DST_ADDR_OFFSET % 32;

			tl_tbl.bf0.ipv6_dst_addr_2 = (rule_key->dip.ip6_addr.ul[2] >>
					SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET)) |
				rule_key->dip.ip6_addr.ul[1] << TL_TBL_IPV6_DST_ADDR_OFFSET % 32;

			tl_tbl.bf0.ipv6_dst_addr_3 = (rule_key->dip.ip6_addr.ul[1] >>
					SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET)) |
				rule_key->dip.ip6_addr.ul[0] << TL_TBL_IPV6_DST_ADDR_OFFSET % 32;

			tl_tbl.bf0.ipv6_dst_addr_4 = rule_key->dip.ip6_addr.ul[0] >>
				SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET);
		} else {
			tl_tbl.bf1.ipv4_src_addr_0 = rule_key->sip.ip4_addr;
			tl_tbl.bf1.ipv4_src_addr_1 = rule_key->sip.ip4_addr >>
				SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV4_SRC_ADDR_OFFSET);
			tl_tbl.bf1.ipv4_dst_addr_0 = rule_key->dip.ip4_addr;
			tl_tbl.bf1.ipv4_dst_addr_1 = rule_key->dip.ip4_addr >>
				SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV4_DST_ADDR_OFFSET);
		}

		tl_tbl.bf0.l4_sport = rule_key->sport;
		tl_tbl.bf0.l4_dport_0 = rule_key->dport;
		tl_tbl.bf0.l4_dport_1 = rule_key->dport >>
			SW_FIELD_OFFSET_IN_WORD(TL_TBL_L4_DPORT_OFFSET);
		tl_tbl.bf0.vni_resv_0 = rule_key->tunnel_info;
		tl_tbl.bf0.vni_resv_1 = rule_key->tunnel_info >>
			SW_FIELD_OFFSET_IN_WORD(TL_TBL_VNI_RESV_OFFSET);
		tl_tbl.bf0.udf0 = rule_key->udf0;
		tl_tbl.bf0.udf1_0 = rule_key->udf1;
		tl_tbl.bf0.udf1_1 = rule_key->udf1 >>
			SW_FIELD_OFFSET_IN_WORD(TL_TBL_UDF1_OFFSET);

		tl_tbl.bf0.vni_resv_valid = (rule_key->key_bmp >> FAL_TUNNEL_KEY_TLINFO_EN) & 1;
		tl_tbl.bf0.udf0_valid = (rule_key->key_bmp >> FAL_TUNNEL_KEY_UDF0_EN) & 1;
		tl_tbl.bf0.udf1_valid = (rule_key->key_bmp >> FAL_TUNNEL_KEY_UDF1_EN) & 1;
		/* tunnel tbl key to get based on hash end */

		tl_tbl.bf0.fwd_type = entry_action->fwd_cmd;
		tl_tbl.bf0.de_acce = entry_action->deacce_en;
		tl_tbl.bf0.decap_en = entry_action->decap_en;
		tl_tbl.bf0.udp_csum_zero = entry_action->udp_csum_zero;
		tl_tbl.bf0.service_code_en = entry_action->service_code_en;
		tl_tbl.bf0.service_code = entry_action->service_code;
		tl_tbl.bf0.spcp_mode = entry_action->spcp_mode;
		tl_tbl.bf0.sdei_mode = entry_action->sdei_mode;
		tl_tbl.bf0.cpcp_mode = entry_action->cpcp_mode;
		tl_tbl.bf0.cdei_mode = entry_action->cdei_mode;
		tl_tbl.bf0.ttl_mode = entry_action->ttl_mode;
		tl_tbl.bf0.dscp_mode = entry_action->dscp_mode;
		tl_tbl.bf0.ecn_mode = entry_action->ecn_mode;
		tl_tbl.bf0.src_info_valid = entry_action->src_info_enable;
		tl_tbl.bf0.src_info_type = entry_action->src_info_type;
		tl_tbl.bf0.src_info = entry_action->src_info;
		tl_tbl.bf0.tl_l3_if = entry_action->verify_entry.tl_l3_if;
		tl_tbl.bf0.svlan_fmt = entry_action->verify_entry.svlan_fmt;
		tl_tbl.bf0.svlan_id_0 = entry_action->verify_entry.svlan_id;
		tl_tbl.bf0.svlan_id_1 = entry_action->verify_entry.svlan_id >>
			SW_FIELD_OFFSET_IN_WORD(TL_TBL_SVLAN_ID_OFFSET);
		tl_tbl.bf0.cvlan_fmt = entry_action->verify_entry.cvlan_fmt;
		tl_tbl.bf0.cvlan_id = entry_action->verify_entry.cvlan_id;
		tl_tbl.bf0.svlan_check_en =
			entry_action->verify_entry.verify_bmp & FAL_TUNNEL_SVLAN_CHECK_EN ?
			A_TRUE : A_FALSE;
		tl_tbl.bf0.cvlan_check_en =
			entry_action->verify_entry.verify_bmp & FAL_TUNNEL_CVLAN_CHECK_EN ?
			A_TRUE : A_FALSE;
		tl_tbl.bf0.tl_l3_if_check_en =
			entry_action->verify_entry.verify_bmp & FAL_TUNNEL_L3IF_CHECK_EN ?
			A_TRUE : A_FALSE;
		tl_tbl.bf0.exp_profile = entry_action->exp_profile;

		*tl_entry = tl_tbl;
	} else {
		tl_tbl = *tl_entry;

		if (tl_tbl.bf0.valid == A_FALSE) {
			return SW_OK;
		}

		rule_key->ip_ver = tl_tbl.bf0.entry_type;
		rule_key->tunnel_type = tl_tbl.bf0.key_type;
		rule_key->l4_proto = tl_tbl.bf0.protocol;

		if (tl_tbl.bf0.entry_type) {
			rule_key->sip.ip6_addr.ul[3] = (tl_tbl.bf0.ipv6_src_addr_0 &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_src_addr_1 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET));

			rule_key->sip.ip6_addr.ul[2] = ((tl_tbl.bf0.ipv6_src_addr_1 >>
				TL_TBL_IPV6_SRC_ADDR_OFFSET % 32) &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_src_addr_2 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET));

			rule_key->sip.ip6_addr.ul[1] = ((tl_tbl.bf0.ipv6_src_addr_2 >>
				TL_TBL_IPV6_SRC_ADDR_OFFSET % 32) &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_src_addr_3 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET));

			rule_key->sip.ip6_addr.ul[0] = ((tl_tbl.bf0.ipv6_src_addr_3 >>
				TL_TBL_IPV6_SRC_ADDR_OFFSET % 32) &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_src_addr_4 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_SRC_ADDR_OFFSET));

			rule_key->dip.ip6_addr.ul[3] = (tl_tbl.bf0.ipv6_dst_addr_0 &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_dst_addr_1 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET));

			rule_key->dip.ip6_addr.ul[2] = ((tl_tbl.bf0.ipv6_dst_addr_1 >>
				TL_TBL_IPV6_DST_ADDR_OFFSET % 32) &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_dst_addr_2 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET));

			rule_key->dip.ip6_addr.ul[1] = ((tl_tbl.bf0.ipv6_dst_addr_2 >>
				TL_TBL_IPV6_DST_ADDR_OFFSET % 32) &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_dst_addr_3 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET));

			rule_key->dip.ip6_addr.ul[0] = ((tl_tbl.bf0.ipv6_dst_addr_3 >>
				TL_TBL_IPV6_DST_ADDR_OFFSET % 32) &
				BITS(0, SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET))) |
				(tl_tbl.bf0.ipv6_dst_addr_4 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV6_DST_ADDR_OFFSET));
		} else {
			rule_key->sip.ip4_addr = tl_tbl.bf1.ipv4_src_addr_0 |
				(tl_tbl.bf1.ipv4_src_addr_1 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV4_SRC_ADDR_OFFSET));

			rule_key->dip.ip4_addr = tl_tbl.bf1.ipv4_dst_addr_0 |
				(tl_tbl.bf1.ipv4_dst_addr_1 <<
				 SW_FIELD_OFFSET_IN_WORD(TL_TBL_IPV4_DST_ADDR_OFFSET));
		}

		rule_key->sport = tl_tbl.bf0.l4_sport;
		rule_key->dport = tl_tbl.bf0.l4_dport_0 |
			tl_tbl.bf0.l4_dport_1 << SW_FIELD_OFFSET_IN_WORD(TL_TBL_L4_DPORT_OFFSET);
		rule_key->tunnel_info = tl_tbl.bf0.vni_resv_0 |
			tl_tbl.bf0.vni_resv_1 << SW_FIELD_OFFSET_IN_WORD(TL_TBL_VNI_RESV_OFFSET);
		rule_key->udf0 = tl_tbl.bf0.udf0;
		rule_key->udf1 = tl_tbl.bf0.udf1_0 |
			tl_tbl.bf0.udf1_1 << SW_FIELD_OFFSET_IN_WORD(TL_TBL_UDF1_OFFSET);

		if (tl_tbl.bf0.vni_resv_valid) {
			rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_TLINFO_EN);
		} else {
			rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_TLINFO_EN);
		}

		if (tl_tbl.bf0.udf0_valid) {
			rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_UDF0_EN);
		} else {
			rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_UDF0_EN);
		}

		if (tl_tbl.bf0.udf1_valid) {
			rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_UDF1_EN);
		} else {
			rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_UDF1_EN);
		}

		entry_action->fwd_cmd = tl_tbl.bf0.fwd_type;
		entry_action->deacce_en = tl_tbl.bf0.de_acce;
		entry_action->decap_en = tl_tbl.bf0.decap_en;
		entry_action->udp_csum_zero = tl_tbl.bf0.udp_csum_zero;
		entry_action->service_code_en = tl_tbl.bf0.service_code_en;
		entry_action->service_code = tl_tbl.bf0.service_code;
		entry_action->spcp_mode = tl_tbl.bf0.spcp_mode;
		entry_action->sdei_mode = tl_tbl.bf0.sdei_mode;
		entry_action->cpcp_mode = tl_tbl.bf0.cpcp_mode;
		entry_action->cdei_mode = tl_tbl.bf0.cdei_mode;
		entry_action->ttl_mode = tl_tbl.bf0.ttl_mode;
		entry_action->dscp_mode = tl_tbl.bf0.dscp_mode;
		entry_action->ecn_mode = tl_tbl.bf0.ecn_mode;
		entry_action->src_info_enable = tl_tbl.bf0.src_info_valid;
		entry_action->src_info_type = tl_tbl.bf0.src_info_type;
		entry_action->src_info = tl_tbl.bf0.src_info;
		entry_action->verify_entry.tl_l3_if = tl_tbl.bf0.tl_l3_if;
		entry_action->verify_entry.svlan_fmt = tl_tbl.bf0.svlan_fmt;
		entry_action->verify_entry.svlan_id = tl_tbl.bf0.svlan_id_0 |
			tl_tbl.bf0.svlan_id_1 << SW_FIELD_OFFSET_IN_WORD(TL_TBL_SVLAN_ID_OFFSET);
		entry_action->verify_entry.cvlan_fmt = tl_tbl.bf0.cvlan_fmt;
		entry_action->verify_entry.cvlan_id = tl_tbl.bf0.cvlan_id;

		if (tl_tbl.bf0.svlan_check_en) {
			entry_action->verify_entry.verify_bmp |= FAL_TUNNEL_SVLAN_CHECK_EN;
		} else {
			entry_action->verify_entry.verify_bmp &= ~FAL_TUNNEL_SVLAN_CHECK_EN;
		}

		if (tl_tbl.bf0.cvlan_check_en) {
			entry_action->verify_entry.verify_bmp |= FAL_TUNNEL_CVLAN_CHECK_EN;
		} else {
			entry_action->verify_entry.verify_bmp &= ~FAL_TUNNEL_CVLAN_CHECK_EN;
		}

		if (tl_tbl.bf0.tl_l3_if_check_en) {
			entry_action->verify_entry.verify_bmp |= FAL_TUNNEL_L3IF_CHECK_EN;
		} else {
			entry_action->verify_entry.verify_bmp &= ~FAL_TUNNEL_L3IF_CHECK_EN;
		}

		entry_action->exp_profile = tl_tbl.bf0.exp_profile;
	}
	return SW_OK;
}

static sw_error_t
adpt_appe_tunnel_key_op(a_uint32_t dev_id, fal_tunnel_type_t tunnel_type,
		fal_tunnel_op_type_t op_type, fal_tunnel_decap_key_t *rule_key)
{
	sw_error_t rv = SW_OK;
	union tl_key_gen_u key_gen;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule_key);
	TUNNEL_DECAP_TYPE_CHECK(tunnel_type);

	aos_mem_zero(&key_gen, sizeof(union tl_key_gen_u));

	switch (op_type) {
		case FAL_TUNNEL_OP_TYPE_DEL:
		case FAL_TUNNEL_OP_TYPE_FLUSH:
			rv = appe_tl_key_gen_set(dev_id, tunnel_type, &key_gen);
			SW_RTN_ON_ERROR(rv);
			break;
		case FAL_TUNNEL_OP_TYPE_GET:
			rv = appe_tl_key_gen_get(dev_id, tunnel_type, &key_gen);
			SW_RTN_ON_ERROR(rv);

			if (key_gen.bf.sip_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_SIP_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_SIP_EN);
			}
			if (key_gen.bf.dip_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_DIP_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_DIP_EN);
			}

			if (key_gen.bf.ip_prot_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_L4PROTO_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_L4PROTO_EN);
			}
			if (key_gen.bf.sport_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_SPORT_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_SPORT_EN);
			}

			if (key_gen.bf.dport_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_DPORT_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_DPORT_EN);
			}

			if (key_gen.bf.vni_resv_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_TLINFO_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_TLINFO_EN);
			}

			if (key_gen.bf.udf0_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_UDF0_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_UDF0_EN);
			}

			if (key_gen.bf.udf1_inc) {
				rule_key->key_bmp |= BIT(FAL_TUNNEL_KEY_UDF1_EN);
			} else {
				rule_key->key_bmp &= ~BIT(FAL_TUNNEL_KEY_UDF1_EN);
			}

			rule_key->udf0_idx = key_gen.bf.udf0_id;
			rule_key->udf0_mask = key_gen.bf.udf0_mask;
			rule_key->udf1_idx = key_gen.bf.udf1_id;
			rule_key->udf1_mask = key_gen.bf.udf1_mask_0 |
				key_gen.bf.udf1_mask_1 <<
				SW_FIELD_OFFSET_IN_WORD(TL_KEY_GEN_UDF1_MASK_OFFSET);
			rule_key->tunnel_info_mask = key_gen.bf.vni_resv_mask_0 |
				key_gen.bf.vni_resv_mask_1 <<
				SW_FIELD_OFFSET_IN_WORD(TL_KEY_GEN_VNI_RESV_MASK_OFFSET);
			// tunnel_type = key_gen.bf.key_type;
			break;
		case FAL_TUNNEL_OP_TYPE_ADD:
			key_gen.bf.sip_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_SIP_EN) & 1;
			key_gen.bf.dip_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_DIP_EN) & 1;
			key_gen.bf.ip_prot_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_L4PROTO_EN) & 1;
			key_gen.bf.sport_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_SPORT_EN) & 1;
			key_gen.bf.dport_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_DPORT_EN) & 1;
			key_gen.bf.vni_resv_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_TLINFO_EN) & 1;
			key_gen.bf.udf0_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_UDF0_EN) & 1;
			key_gen.bf.udf1_inc = (rule_key->key_bmp >>
					FAL_TUNNEL_KEY_UDF1_EN) & 1;
			key_gen.bf.udf0_id = rule_key->udf0_idx;
			key_gen.bf.udf0_mask = rule_key->udf0_mask;
			key_gen.bf.udf1_id = rule_key->udf1_idx;
			key_gen.bf.udf1_mask_0 = rule_key->udf1_mask;
			key_gen.bf.udf1_mask_1 = rule_key->udf1_mask >>
				SW_FIELD_OFFSET_IN_WORD(TL_KEY_GEN_UDF1_MASK_OFFSET);
			key_gen.bf.vni_resv_mask_0 = rule_key->tunnel_info_mask;
			key_gen.bf.vni_resv_mask_1 = rule_key->tunnel_info_mask >>
				SW_FIELD_OFFSET_IN_WORD(TL_KEY_GEN_VNI_RESV_MASK_OFFSET);
			key_gen.bf.key_type = tunnel_type;

			rv = appe_tl_key_gen_set(dev_id, tunnel_type, &key_gen);
			SW_RTN_ON_ERROR(rv);
			break;
		default:
			SSDK_ERROR("%s Unsupported op type: %d\n", __func__, op_type);
			return SW_BAD_PARAM;
	}
	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_entry_get(a_uint32_t dev_id,
		fal_tunnel_op_mode_t get_mode, fal_tunnel_decap_entry_t *entry)
{
	sw_error_t rv = SW_OK;
	fal_tunnel_rule_t *rule_key;
	fal_tunnel_action_t *entry_action;
	union tl_tbl_u tl_tbl;
	union tl_cnt_tbl_u tl_cnt_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	aos_mem_zero(&tl_tbl, sizeof(union tl_tbl_u));
	aos_mem_zero(&tl_cnt_tbl, sizeof(union tl_cnt_tbl_u));

	rule_key = &entry->decap_rule;
	entry_action = &entry->decap_action;

	TUNNEL_DECAP_TYPE_CHECK(rule_key->tunnel_type);

	if (get_mode == FAL_TUNNEL_OP_MODE_INDEX) {
		TUNNEL_DECAP_ID_CHECK(entry->decap_rule.entry_id);
	}

	if (get_mode == FAL_TUNNEL_OP_MODE_HASH) {
		rv = adpt_appe_tunnel_decap_entry_convert(dev_id, entry, &tl_tbl, A_TRUE);
		SW_RTN_ON_ERROR(rv);
	}

	rv = appe_tunnel_decap_entry_op(dev_id, FAL_TUNNEL_OP_TYPE_GET,
			get_mode, &tl_tbl, &rule_key->entry_id);
	SW_RTN_ON_ERROR(rv);

	if (!tl_tbl.bf0.valid) {
		return SW_NOT_FOUND;
	}

	rv = adpt_appe_tunnel_decap_entry_convert(dev_id, entry, &tl_tbl, A_FALSE);
	SW_RTN_ON_ERROR(rv);

	rv = appe_tl_cnt_tbl_get(dev_id, rule_key->entry_id, &tl_cnt_tbl);
	SW_RTN_ON_ERROR(rv);

	entry_action->pkt_counter = tl_cnt_tbl.bf.rx_pkt_cnt;
	entry_action->byte_counter = ((a_uint64_t)tl_cnt_tbl.bf.rx_byte_cnt_1 <<
			SW_FIELD_OFFSET_IN_WORD(TL_CNT_TBL_RX_BYTE_CNT_OFFSET)) |
		tl_cnt_tbl.bf.rx_byte_cnt_0;

	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_entry_add(a_uint32_t dev_id,
		fal_tunnel_op_mode_t add_mode, fal_tunnel_decap_entry_t *entry)
{
	sw_error_t rv = SW_OK;
	fal_tunnel_rule_t *rule_key;
	fal_tunnel_action_t *entry_action;
	union tl_tbl_u tl_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	aos_mem_zero(&tl_tbl, sizeof(union tl_tbl_u));

	rule_key = &entry->decap_rule;
	entry_action = &entry->decap_action;
	TUNNEL_DECAP_TYPE_CHECK(rule_key->tunnel_type);

	if (add_mode == FAL_TUNNEL_OP_MODE_INDEX) {
		TUNNEL_DECAP_ID_CHECK(entry->decap_rule.entry_id);
	}

	rv = adpt_appe_tunnel_decap_entry_convert(dev_id, entry, &tl_tbl, A_TRUE);
	SW_RTN_ON_ERROR(rv);

	rv = appe_tunnel_decap_entry_op(dev_id, FAL_TUNNEL_OP_TYPE_ADD,
			add_mode, &tl_tbl, &rule_key->entry_id);
	SW_RTN_ON_ERROR(rv);

	rv = adpt_appe_tunnel_decap_entry_get(dev_id, FAL_TUNNEL_OP_MODE_INDEX, entry);

	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_entry_del(a_uint32_t dev_id,
		fal_tunnel_op_mode_t del_mode, fal_tunnel_decap_entry_t *entry)
{
	sw_error_t rv = SW_OK;
	fal_tunnel_rule_t *rule_key;
	union tl_tbl_u tl_tbl;
	union tl_cnt_tbl_u tl_cnt_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	aos_mem_zero(&tl_tbl, sizeof(union tl_tbl_u));
	aos_mem_zero(&tl_cnt_tbl, sizeof(union tl_cnt_tbl_u));

	rule_key = &entry->decap_rule;
	TUNNEL_DECAP_TYPE_CHECK(rule_key->tunnel_type);

	if (del_mode == FAL_TUNNEL_OP_MODE_INDEX) {
		TUNNEL_DECAP_ID_CHECK(entry->decap_rule.entry_id);
	}

	if (del_mode == FAL_TUNNEL_OP_MODE_HASH) {
		rv = adpt_appe_tunnel_decap_entry_convert(dev_id, entry, &tl_tbl, A_TRUE);
		SW_RTN_ON_ERROR(rv);
	}

	rv = appe_tunnel_decap_entry_op(dev_id, FAL_TUNNEL_OP_TYPE_DEL,
			del_mode, &tl_tbl, &rule_key->entry_id);
	SW_RTN_ON_ERROR(rv);

	rv = appe_tl_cnt_tbl_set(dev_id, rule_key->entry_id, &tl_cnt_tbl);

	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_op_mode_t next_mode, fal_tunnel_decap_entry_t *entry)
{
	sw_error_t rv = SW_OK;
	union tl_tbl_u tl_tbl;
	union tl_cnt_tbl_u tl_cnt_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	aos_mem_zero(&tl_tbl, sizeof(union tl_tbl_u));
	aos_mem_zero(&tl_cnt_tbl, sizeof(union tl_cnt_tbl_u));

	if (next_mode == FAL_TUNNEL_OP_MODE_HASH) {
		return SW_BAD_PARAM;
	}

	while (1) {
		TUNNEL_DECAP_ID_CHECK(entry->decap_rule.entry_id);

		rv = appe_tunnel_decap_entry_op(dev_id, FAL_TUNNEL_OP_TYPE_GET,
				next_mode, &tl_tbl, &entry->decap_rule.entry_id);
		SW_RTN_ON_ERROR(rv);

		if (!tl_tbl.bf0.valid) {
			entry->decap_rule.entry_id++;
			continue;
		} else {
			break;
		}
	}

	rv = adpt_appe_tunnel_decap_entry_convert(dev_id, entry, &tl_tbl, A_FALSE);
	SW_RTN_ON_ERROR(rv);

	rv = appe_tl_cnt_tbl_get(dev_id, entry->decap_rule.entry_id, &tl_cnt_tbl);
	SW_RTN_ON_ERROR(rv);

	entry->decap_action.pkt_counter = tl_cnt_tbl.bf.rx_pkt_cnt;
	entry->decap_action.byte_counter = ((a_uint64_t)tl_cnt_tbl.bf.rx_byte_cnt_1 <<
			SW_FIELD_OFFSET_IN_WORD(TL_CNT_TBL_RX_BYTE_CNT_OFFSET)) |
		tl_cnt_tbl.bf.rx_byte_cnt_0;

	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_entry_flush(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t index = 0;
	union tl_cnt_tbl_u tl_cnt_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	aos_mem_zero(&tl_cnt_tbl, sizeof(union tl_cnt_tbl_u));

	rv = appe_tunnel_decap_entry_op(dev_id, FAL_TUNNEL_OP_TYPE_FLUSH,
			FAL_TUNNEL_OP_MODE_HASH, NULL, &index);

	for (index = 0; index < FAL_TUNNEL_DECAP_ENTRY_MAX; index++) {
		rv = appe_tl_cnt_tbl_set(dev_id, index, &tl_cnt_tbl);
		SW_RTN_ON_ERROR(rv);
	}

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_header_ctrl_set(a_uint32_t dev_id,
		fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
	sw_error_t rv = SW_OK;
	union eg_ipv4_hdr_ctrl_u eg_ipv4_hdr_ctrl;
	union eg_udp_entropy_ctrl_u udp_entropy_ctrl;
	union eg_proto_mapping0_u proto_mapping0[2];
	union eg_proto_mapping1_u proto_mapping1[2];

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(header_ctrl);

	aos_mem_zero(&eg_ipv4_hdr_ctrl, sizeof(union eg_ipv4_hdr_ctrl_u));
	aos_mem_zero(&udp_entropy_ctrl, sizeof(union eg_udp_entropy_ctrl_u));
	aos_mem_zero(&proto_mapping0, sizeof(union eg_proto_mapping0_u));
	aos_mem_zero(&proto_mapping1, sizeof(union eg_proto_mapping1_u));

	rv = appe_eg_ipv4_hdr_ctrl_get(dev_id, &eg_ipv4_hdr_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_udp_entropy_ctrl_get(dev_id, &udp_entropy_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping0_get(dev_id, 0, &proto_mapping0[0]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping0_get(dev_id, 1, &proto_mapping0[1]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping1_get(dev_id, 0, &proto_mapping1[0]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping1_get(dev_id, 1, &proto_mapping1[1]);
	SW_RTN_ON_ERROR(rv);

	eg_ipv4_hdr_ctrl.bf.ipv4_id_seed = header_ctrl->ipv4_id_seed;
	eg_ipv4_hdr_ctrl.bf.ipv4_df_set = header_ctrl->ipv4_df_set;
	udp_entropy_ctrl.bf.port_base = header_ctrl->udp_sport_base;
	udp_entropy_ctrl.bf.port_mask = header_ctrl->udp_sport_mask;
	proto_mapping0[0].bf.protocol0 = header_ctrl->proto_map_data[0];
	proto_mapping1[0].bf.protocol1 = header_ctrl->proto_map_data[1];
	proto_mapping0[1].bf.protocol0 = header_ctrl->proto_map_data[2];
	proto_mapping1[1].bf.protocol1 = header_ctrl->proto_map_data[3];

	rv = appe_eg_ipv4_hdr_ctrl_set(dev_id, &eg_ipv4_hdr_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_udp_entropy_ctrl_set(dev_id, &udp_entropy_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping0_set(dev_id, 0, &proto_mapping0[0]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping0_set(dev_id, 1, &proto_mapping0[1]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping1_set(dev_id, 0, &proto_mapping1[0]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping1_set(dev_id, 1, &proto_mapping1[1]);

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_header_ctrl_get(a_uint32_t dev_id,
		fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
	sw_error_t rv = SW_OK;
	union eg_ipv4_hdr_ctrl_u eg_ipv4_hdr_ctrl;
	union eg_udp_entropy_ctrl_u udp_entropy_ctrl;
	union eg_proto_mapping0_u proto_mapping0[2];
	union eg_proto_mapping1_u proto_mapping1[2];

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(header_ctrl);

	aos_mem_zero(&eg_ipv4_hdr_ctrl, sizeof(union eg_ipv4_hdr_ctrl_u));
	aos_mem_zero(&udp_entropy_ctrl, sizeof(union eg_udp_entropy_ctrl_u));
	aos_mem_zero(&proto_mapping0, sizeof(union eg_proto_mapping0_u));
	aos_mem_zero(&proto_mapping1, sizeof(union eg_proto_mapping1_u));

	rv = appe_eg_ipv4_hdr_ctrl_get(dev_id, &eg_ipv4_hdr_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_udp_entropy_ctrl_get(dev_id, &udp_entropy_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping0_get(dev_id, 0, &proto_mapping0[0]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping0_get(dev_id, 1, &proto_mapping0[1]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping1_get(dev_id, 0, &proto_mapping1[0]);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_proto_mapping1_get(dev_id, 1, &proto_mapping1[1]);
	SW_RTN_ON_ERROR(rv);

	header_ctrl->ipv4_id_seed = eg_ipv4_hdr_ctrl.bf.ipv4_id_seed;
	header_ctrl->ipv4_df_set = eg_ipv4_hdr_ctrl.bf.ipv4_df_set;
	header_ctrl->udp_sport_base = udp_entropy_ctrl.bf.port_base;
	header_ctrl->udp_sport_mask = udp_entropy_ctrl.bf.port_mask;
	header_ctrl->proto_map_data[0] = proto_mapping0[0].bf.protocol0;
	header_ctrl->proto_map_data[1] = proto_mapping1[0].bf.protocol1;
	header_ctrl->proto_map_data[2] = proto_mapping0[1].bf.protocol0;
	header_ctrl->proto_map_data[3] = proto_mapping1[1].bf.protocol1;

	return rv;
}

#ifndef IN_TUNNEL_MINI
sw_error_t
adpt_appe_tunnel_encap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
	sw_error_t rv = SW_OK;
	union ecn_profile_u ecn_profile;
	a_uint8_t ecn_data = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ecn_rule);
	ADPT_NULL_POINT_CHECK(ecn_value);

	aos_mem_zero(&ecn_profile, sizeof(union ecn_profile_u));

	rv = appe_ecn_profile_get(dev_id, &ecn_profile);
	SW_RTN_ON_ERROR(rv);

	switch (ecn_rule->ecn_mode) {
		case FAL_ENCAP_ECN_RFC3168_LIMIT_RFC6040_CMPAT_MODE:
			ecn_data = ecn_profile.bf.profile0;
			break;
		case FAL_ENCAP_ECN_RFC3168_FULL_MODE:
			ecn_data = ecn_profile.bf.profile1;
			break;
		case FAL_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE:
			ecn_data = ecn_profile.bf.profile2;
			break;
		case FAL_ENCAP_ECN_NO_UPDATE:
		default:
			SSDK_ERROR("Unsupported encap ecn_mode: %d\n", ecn_rule->ecn_mode);
			return SW_BAD_PARAM;
	}

	/* every two bits for CE,ECT(1),ECT(0),Not-ECT saved in ecn_data */
	*ecn_value = (ecn_data >> (ecn_rule->inner_ecn << 1)) & FAL_ECN_VAL_LENGTH_MASK;

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
	sw_error_t rv = SW_OK;
	union ecn_profile_u ecn_profile;
	a_uint8_t ecn_data = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ecn_rule);
	ADPT_NULL_POINT_CHECK(ecn_value);

	aos_mem_zero(&ecn_profile, sizeof(union ecn_profile_u));

	rv = appe_ecn_profile_get(dev_id, &ecn_profile);
	SW_RTN_ON_ERROR(rv);

	switch (ecn_rule->ecn_mode) {
		case FAL_ENCAP_ECN_RFC3168_LIMIT_RFC6040_CMPAT_MODE:
			ecn_data = ecn_profile.bf.profile0;
			break;
		case FAL_ENCAP_ECN_RFC3168_FULL_MODE:
			ecn_data = ecn_profile.bf.profile1;
			break;
		case FAL_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE:
			ecn_data = ecn_profile.bf.profile2;
			break;
		case FAL_ENCAP_ECN_NO_UPDATE:
		default:
			SSDK_ERROR("Unsupported encap ecn_mode: %d\n", ecn_rule->ecn_mode);
			return SW_BAD_PARAM;
	}

	/* every two bits for CE,ECT(1),ECT(0),Not-ECT saved in ecn_data */
	ecn_data &= ~(FAL_ECN_VAL_LENGTH_MASK << (ecn_rule->inner_ecn << 1));
	ecn_data |= *ecn_value << (ecn_rule->inner_ecn << 1);

	switch (ecn_rule->ecn_mode) {
		case FAL_ENCAP_ECN_RFC3168_LIMIT_RFC6040_CMPAT_MODE:
			ecn_profile.bf.profile0 = ecn_data;
			break;
		case FAL_ENCAP_ECN_RFC3168_FULL_MODE:
			ecn_profile.bf.profile1 = ecn_data;
			break;
		case FAL_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE:
			ecn_profile.bf.profile2 = ecn_data;
			break;
		case FAL_ENCAP_ECN_NO_UPDATE:
		default:
			SSDK_ERROR("Unsupported encap ecn_mode: %d\n", ecn_rule->ecn_mode);
			return SW_BAD_PARAM;
	}

	rv = appe_ecn_profile_set(dev_id, &ecn_profile);

	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_ecn_mode_get(a_uint32_t dev_id,
		fal_tunnel_decap_ecn_rule_t *ecn_rule, fal_tunnel_decap_ecn_action_t *ecn_action)
{
	sw_error_t rv = SW_OK;
	a_uint32_t ecn_data = 0;
	a_uint16_t ecn_exp = 0;
	union ecn_map_mode0_0_u ecn_map_mode0;
	union ecn_map_mode1_0_u ecn_map_mode1;
	union ecn_map_mode2_0_u ecn_map_mode2;
	union ecn_map_mode0_1_u ecn_excep0;
	union ecn_map_mode1_1_u ecn_excep1;
	union ecn_map_mode2_1_u ecn_excep2;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ecn_rule);
	ADPT_NULL_POINT_CHECK(ecn_action);

	aos_mem_zero(&ecn_map_mode0, sizeof(union ecn_map_mode0_0_u));
	aos_mem_zero(&ecn_map_mode1, sizeof(union ecn_map_mode1_0_u));
	aos_mem_zero(&ecn_map_mode2, sizeof(union ecn_map_mode2_0_u));
	aos_mem_zero(&ecn_excep0, sizeof(union ecn_map_mode0_1_u));
	aos_mem_zero(&ecn_excep1, sizeof(union ecn_map_mode1_1_u));
	aos_mem_zero(&ecn_excep2, sizeof(union ecn_map_mode2_1_u));

	switch (ecn_rule->ecn_mode) {
		case FAL_TUNNEL_DECAP_ECN_RFC3168_MODE:
			rv = appe_ecn_map_mode0_0_get(dev_id, &ecn_map_mode0);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode0_1_get(dev_id, &ecn_excep0);
			SW_RTN_ON_ERROR(rv);

			ecn_data = ecn_map_mode0.bf.new_ecn;
			ecn_exp = ecn_excep0.bf.exception_en;
			break;
		case FAL_TUNNEL_DECAP_ECN_RFC4301_MODE:
			rv = appe_ecn_map_mode1_0_get(dev_id, &ecn_map_mode1);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode1_1_get(dev_id, &ecn_excep1);
			SW_RTN_ON_ERROR(rv);

			ecn_data = ecn_map_mode1.bf.new_ecn;
			ecn_exp = ecn_excep1.bf.exception_en;
			break;
		case FAL_TUNNEL_DECAP_ECN_RFC6040_MODE:
			rv = appe_ecn_map_mode2_0_get(dev_id, &ecn_map_mode2);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode2_1_get(dev_id, &ecn_excep2);
			SW_RTN_ON_ERROR(rv);

			ecn_data = ecn_map_mode2.bf.new_ecn;
			ecn_exp = ecn_excep2.bf.exception_en;
			break;
		default:
			SSDK_ERROR("Unsupported decap ecn mode: %d\n", ecn_rule->ecn_mode);
			return SW_BAD_PARAM;
	}

	ecn_data = ecn_data >> (((FAL_ECN_VAL_LENGTH_MASK - ecn_rule->outer_ecn) << 1) +
			((FAL_ECN_VAL_LENGTH_MASK - ecn_rule->inner_ecn) <<
			 FAL_ECN_VAL_LENGTH_MASK));
	ecn_action->ecn_value = ecn_data & FAL_ECN_VAL_LENGTH_MASK;

	ecn_exp = ecn_exp >> ((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->outer_ecn) +
			((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->inner_ecn) << 2));
	ecn_action->ecn_exp = ecn_exp & FAL_ECN_EXP_LENGTH_MASK;

	return rv;
}

sw_error_t
adpt_appe_tunnel_decap_ecn_mode_set(a_uint32_t dev_id,
		fal_tunnel_decap_ecn_rule_t *ecn_rule, fal_tunnel_decap_ecn_action_t *ecn_action)
{
	sw_error_t rv = SW_OK;
	a_uint32_t ecn_data = 0;
	a_uint16_t ecn_exp = 0;
	union ecn_map_mode0_0_u ecn_map_mode0;
	union ecn_map_mode1_0_u ecn_map_mode1;
	union ecn_map_mode2_0_u ecn_map_mode2;
	union ecn_map_mode0_1_u ecn_excep0;
	union ecn_map_mode1_1_u ecn_excep1;
	union ecn_map_mode2_1_u ecn_excep2;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ecn_rule);
	ADPT_NULL_POINT_CHECK(ecn_action);

	aos_mem_zero(&ecn_map_mode0, sizeof(union ecn_map_mode0_0_u));
	aos_mem_zero(&ecn_map_mode1, sizeof(union ecn_map_mode1_0_u));
	aos_mem_zero(&ecn_map_mode2, sizeof(union ecn_map_mode2_0_u));
	aos_mem_zero(&ecn_excep0, sizeof(union ecn_map_mode0_1_u));
	aos_mem_zero(&ecn_excep1, sizeof(union ecn_map_mode1_1_u));
	aos_mem_zero(&ecn_excep2, sizeof(union ecn_map_mode2_1_u));

	switch (ecn_rule->ecn_mode) {
		case FAL_TUNNEL_DECAP_ECN_RFC3168_MODE:
			rv = appe_ecn_map_mode0_0_get(dev_id, &ecn_map_mode0);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode0_1_get(dev_id, &ecn_excep0);
			SW_RTN_ON_ERROR(rv);

			ecn_data = ecn_map_mode0.bf.new_ecn;
			ecn_exp = ecn_excep0.bf.exception_en;
			break;
		case FAL_TUNNEL_DECAP_ECN_RFC4301_MODE:
			rv = appe_ecn_map_mode1_0_get(dev_id, &ecn_map_mode1);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode1_1_get(dev_id, &ecn_excep1);
			SW_RTN_ON_ERROR(rv);

			ecn_data = ecn_map_mode1.bf.new_ecn;
			ecn_exp = ecn_excep1.bf.exception_en;
			break;
		case FAL_TUNNEL_DECAP_ECN_RFC6040_MODE:
			rv = appe_ecn_map_mode2_0_get(dev_id, &ecn_map_mode2);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode2_1_get(dev_id, &ecn_excep2);
			SW_RTN_ON_ERROR(rv);

			ecn_data = ecn_map_mode2.bf.new_ecn;
			ecn_exp = ecn_excep2.bf.exception_en;
			break;
		default:
			SSDK_ERROR("Unsupported decap ecn mode: %d\n", ecn_rule->ecn_mode);
			return SW_BAD_PARAM;
	}

	ecn_data &= ~(FAL_ECN_VAL_LENGTH_MASK <<
			(((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->outer_ecn)<<1) +
			 ((FAL_ECN_VAL_LENGTH_MASK - ecn_rule->inner_ecn) <<
			  FAL_ECN_VAL_LENGTH_MASK)));

	ecn_data |= (ecn_action->ecn_value <<
			(((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->outer_ecn)<<1) +
			((FAL_ECN_VAL_LENGTH_MASK - ecn_rule->inner_ecn) <<
			 FAL_ECN_VAL_LENGTH_MASK)));

	ecn_exp &= ~(FAL_ECN_EXP_LENGTH_MASK <<
			((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->outer_ecn) +
			 ((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->inner_ecn) << 2)));

	ecn_exp |= (ecn_action->ecn_exp <<
			((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->outer_ecn) +
			 ((FAL_ECN_VAL_LENGTH_MASK-ecn_rule->inner_ecn) << 2)));

	switch (ecn_rule->ecn_mode) {
		case FAL_TUNNEL_DECAP_ECN_RFC3168_MODE:
			ecn_map_mode0.bf.new_ecn = ecn_data;
			ecn_excep0.bf.exception_en = ecn_exp;

			rv = appe_ecn_map_mode0_0_set(dev_id, &ecn_map_mode0);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode0_1_set(dev_id, &ecn_excep0);
			SW_RTN_ON_ERROR(rv);
			break;
		case FAL_TUNNEL_DECAP_ECN_RFC4301_MODE:
			ecn_map_mode1.bf.new_ecn = ecn_data;
			ecn_excep1.bf.exception_en = ecn_exp;

			rv = appe_ecn_map_mode1_0_set(dev_id, &ecn_map_mode1);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode1_1_set(dev_id, &ecn_excep1);
			SW_RTN_ON_ERROR(rv);
			break;
		case FAL_TUNNEL_DECAP_ECN_RFC6040_MODE:
			ecn_map_mode2.bf.new_ecn = ecn_data;
			ecn_excep2.bf.exception_en = ecn_exp;

			rv = appe_ecn_map_mode2_0_set(dev_id, &ecn_map_mode2);
			SW_RTN_ON_ERROR(rv);
			rv = appe_ecn_map_mode2_1_set(dev_id, &ecn_excep2);
			SW_RTN_ON_ERROR(rv);
			break;
		default:
			SSDK_ERROR("Unsupported decap ecn mode: %d\n", ecn_rule->ecn_mode);
			return SW_BAD_PARAM;
	}

	return rv;
}
#endif

sw_error_t
adpt_appe_tunnel_global_cfg_get(a_uint32_t dev_id,
		fal_tunnel_global_cfg_t *cfg)
{
	sw_error_t rv = SW_OK;
	union tl_ctrl_u tl_ctl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	aos_mem_zero(&tl_ctl, sizeof(union tl_ctrl_u));

	rv = appe_tl_ctrl_get(dev_id, &tl_ctl);
	SW_RTN_ON_ERROR(rv);

	cfg->deacce_action = tl_ctl.bf.tl_de_acce_cmd;
	cfg->src_if_check_action = tl_ctl.bf.tl_src_if_check_cmd;
	cfg->src_if_check_deacce_en = tl_ctl.bf.tl_src_if_check_de_acce;
	cfg->vlan_check_action = tl_ctl.bf.tl_vlan_check_cmd;
	cfg->vlan_check_deacce_en = tl_ctl.bf.tl_vlan_check_de_acce;
	cfg->udp_csum_zero_action = tl_ctl.bf.udp_csum_zero_cmd;
	cfg->udp_csum_zero_deacce_en = tl_ctl.bf.udp_csum_zero_de_acce;;
	cfg->pppoe_multicast_action = tl_ctl.bf.pppoe_multicast_cmd;;
	cfg->pppoe_multicast_deacce_en = tl_ctl.bf.pppoe_multicast_de_acce;;
	cfg->hash_mode[0] = tl_ctl.bf.tl_hash_mode_0;;
	cfg->hash_mode[1] = tl_ctl.bf.tl_hash_mode_1;;

	return rv;
}

sw_error_t
adpt_appe_tunnel_global_cfg_set(a_uint32_t dev_id,
		fal_tunnel_global_cfg_t *cfg)
{
	sw_error_t rv = SW_OK;
	union tl_ctrl_u tl_ctl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	aos_mem_zero(&tl_ctl, sizeof(union tl_ctrl_u));

	rv = appe_tl_ctrl_get(dev_id, &tl_ctl);
	SW_RTN_ON_ERROR(rv);

	tl_ctl.bf.tl_de_acce_cmd = cfg->deacce_action;
	tl_ctl.bf.tl_src_if_check_cmd = cfg->src_if_check_action;
	tl_ctl.bf.tl_src_if_check_de_acce = cfg->src_if_check_deacce_en;
	tl_ctl.bf.tl_vlan_check_cmd = cfg->vlan_check_action;
	tl_ctl.bf.tl_vlan_check_de_acce = cfg->vlan_check_deacce_en;
	tl_ctl.bf.udp_csum_zero_cmd = cfg->udp_csum_zero_action;
	tl_ctl.bf.udp_csum_zero_de_acce = cfg->udp_csum_zero_deacce_en;;
	tl_ctl.bf.pppoe_multicast_cmd = cfg->pppoe_multicast_action;;
	tl_ctl.bf.pppoe_multicast_de_acce = cfg->pppoe_multicast_deacce_en;;
	tl_ctl.bf.tl_hash_mode_0 = cfg->hash_mode[0];
	tl_ctl.bf.tl_hash_mode_1 = cfg->hash_mode[1];

	rv = appe_tl_ctrl_set(dev_id, &tl_ctl);

	return rv;
}

sw_error_t
adpt_appe_tunnel_port_intf_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
	sw_error_t rv = SW_OK;
	union tl_port_vp_tbl_u tl_port_vp_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(port_cfg);

	aos_mem_zero(&tl_port_vp_tbl, sizeof(union tl_port_vp_tbl_u));

	rv = appe_tl_port_vp_tbl_get(dev_id, port_value, &tl_port_vp_tbl);
	SW_RTN_ON_ERROR(rv);

	tl_port_vp_tbl.bf.vlan_profile = port_cfg->vlan_group_id;
	tl_port_vp_tbl.bf.pppoe_profile = port_cfg->pppoe_group_id;
	tl_port_vp_tbl.bf.tl_l3_if_valid = port_cfg->l3_if.l3_if_valid;
	tl_port_vp_tbl.bf.tl_l3_if_index = port_cfg->l3_if.l3_if_index;
	tl_port_vp_tbl.bf.pppoe_en = port_cfg->pppoe_en;
	tl_port_vp_tbl.bf.mac_addr_0 = port_cfg->mac_addr.uc[5] |
					port_cfg->mac_addr.uc[4] << 8 |
					port_cfg->mac_addr.uc[3] << 16 |
					port_cfg->mac_addr.uc[2] << 24;
	tl_port_vp_tbl.bf.mac_addr_1 = port_cfg->mac_addr.uc[1] |
					port_cfg->mac_addr.uc[0] << 8;

	rv = appe_tl_port_vp_tbl_set(dev_id, port_value, &tl_port_vp_tbl);

	return rv;
}

sw_error_t
adpt_appe_tunnel_port_intf_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
	sw_error_t rv = SW_OK;
	union tl_port_vp_tbl_u tl_port_vp_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(port_cfg);

	aos_mem_zero(&tl_port_vp_tbl, sizeof(union tl_port_vp_tbl_u));

	rv = appe_tl_port_vp_tbl_get(dev_id, port_value, &tl_port_vp_tbl);
	SW_RTN_ON_ERROR(rv);

	port_cfg->vlan_group_id = tl_port_vp_tbl.bf.vlan_profile;
	port_cfg->pppoe_group_id = tl_port_vp_tbl.bf.pppoe_profile;
	port_cfg->l3_if.l3_if_valid = tl_port_vp_tbl.bf.tl_l3_if_valid;
	port_cfg->l3_if.l3_if_index = tl_port_vp_tbl.bf.tl_l3_if_index;
	port_cfg->pppoe_en = tl_port_vp_tbl.bf.pppoe_en;
	port_cfg->mac_addr.uc[5] = tl_port_vp_tbl.bf.mac_addr_0;
	port_cfg->mac_addr.uc[4] = tl_port_vp_tbl.bf.mac_addr_0 >> 8;
	port_cfg->mac_addr.uc[3] = tl_port_vp_tbl.bf.mac_addr_0 >> 16;
	port_cfg->mac_addr.uc[2] = tl_port_vp_tbl.bf.mac_addr_0 >> 24;
	port_cfg->mac_addr.uc[1] = tl_port_vp_tbl.bf.mac_addr_1;
	port_cfg->mac_addr.uc[0] = tl_port_vp_tbl.bf.mac_addr_1 >> 8;

	return rv;
}

#ifndef IN_TUNNEL_MINI
static inline a_bool_t
adpt_appe_tunnel_vlan_entry_compare(fal_tunnel_vlan_intf_t vlan_cfg,
		union tl_vlan_tbl_u tl_vlan_tbl)
{

	a_uint16_t tbl_key_bmp = 0;

	tbl_key_bmp |= (tl_vlan_tbl.bf0.skey_vid_incl ? FAL_TUNNEL_SVLAN_CHECK_EN : 0);
	tbl_key_bmp |= (tl_vlan_tbl.bf0.ckey_vid_incl ? FAL_TUNNEL_CVLAN_CHECK_EN : 0);

	if (tl_vlan_tbl.bf0.type != adpt_port_type_convert(A_TRUE,
				FAL_PORT_ID_TYPE(vlan_cfg.port_id)) ||
			tl_vlan_tbl.bf0.port_vp_id != FAL_PORT_ID_VALUE(vlan_cfg.port_id) ||
			tl_vlan_tbl.bf0.skey_fmt != vlan_cfg.svlan_fmt ||
			tl_vlan_tbl.bf0.ckey_fmt != vlan_cfg.cvlan_fmt ||
			tbl_key_bmp != vlan_cfg.key_bmp)
		return A_FALSE;
	if (vlan_cfg.key_bmp & FAL_TUNNEL_SVLAN_CHECK_EN &&
			tl_vlan_tbl.bf0.skey_vid != vlan_cfg.svlan_id)
		return A_FALSE;
	if (vlan_cfg.key_bmp & FAL_TUNNEL_CVLAN_CHECK_EN &&
			(tl_vlan_tbl.bf0.ckey_vid_0 | (tl_vlan_tbl.bf0.ckey_vid_1 <<
					SW_FIELD_OFFSET_IN_WORD(TL_VLAN_TBL_CKEY_VID_OFFSET)))
			!= vlan_cfg.cvlan_id)
		return A_FALSE;

	return A_TRUE;
}

static inline sw_error_t
adpt_appe_tunnel_vlan_entry_convert(fal_tunnel_vlan_intf_t *vlan_cfg,
		union tl_vlan_tbl_u *tl_vlan_tbl, a_bool_t to_hsl)
{
	sw_error_t rv = SW_OK;

	ADPT_NULL_POINT_CHECK(vlan_cfg);
	ADPT_NULL_POINT_CHECK(tl_vlan_tbl);

	if (to_hsl) {
		tl_vlan_tbl->bf0.valid = A_TRUE;
		tl_vlan_tbl->bf0.type = adpt_port_type_convert(A_TRUE,
				FAL_PORT_ID_TYPE(vlan_cfg->port_id));
		tl_vlan_tbl->bf0.port_vp_id = FAL_PORT_ID_VALUE(vlan_cfg->port_id);
		tl_vlan_tbl->bf0.skey_fmt = vlan_cfg->svlan_fmt;
		tl_vlan_tbl->bf0.skey_vid_incl = (vlan_cfg->key_bmp & FAL_TUNNEL_SVLAN_CHECK_EN) ?
			A_TRUE : A_FALSE;
		tl_vlan_tbl->bf0.skey_vid = vlan_cfg->svlan_id;
		tl_vlan_tbl->bf0.ckey_fmt = vlan_cfg->cvlan_fmt;
		tl_vlan_tbl->bf0.ckey_vid_incl = (vlan_cfg->key_bmp & FAL_TUNNEL_CVLAN_CHECK_EN) ?
			A_TRUE : A_FALSE;
		tl_vlan_tbl->bf0.ckey_vid_0 = vlan_cfg->cvlan_id;
		tl_vlan_tbl->bf0.ckey_vid_1 = vlan_cfg->cvlan_id >>
			SW_FIELD_OFFSET_IN_WORD(TL_VLAN_TBL_CKEY_VID_OFFSET);

		tl_vlan_tbl->bf0.tl_l3_if_valid = vlan_cfg->l3_if.l3_if_valid;
		tl_vlan_tbl->bf0.tl_l3_if_index = vlan_cfg->l3_if.l3_if_index;
		tl_vlan_tbl->bf0.pppoe_en = vlan_cfg->pppoe_en;
	} else {
		vlan_cfg->port_id = FAL_PORT_ID(adpt_port_type_convert(A_FALSE,
					tl_vlan_tbl->bf0.type),
				tl_vlan_tbl->bf0.port_vp_id);
		vlan_cfg->svlan_fmt = tl_vlan_tbl->bf0.skey_fmt;
		vlan_cfg->svlan_id = tl_vlan_tbl->bf0.skey_vid;
		vlan_cfg->cvlan_fmt = tl_vlan_tbl->bf0.ckey_fmt;
		vlan_cfg->cvlan_id = tl_vlan_tbl->bf0.ckey_vid_0 | (tl_vlan_tbl->bf0.ckey_vid_1 <<
				SW_FIELD_OFFSET_IN_WORD(TL_VLAN_TBL_CKEY_VID_OFFSET));

		if (tl_vlan_tbl->bf0.skey_vid_incl)
			vlan_cfg->key_bmp |= FAL_TUNNEL_SVLAN_CHECK_EN;
		else
			vlan_cfg->key_bmp &= ~FAL_TUNNEL_SVLAN_CHECK_EN;

		if (tl_vlan_tbl->bf0.ckey_vid_incl)
			vlan_cfg->key_bmp |= FAL_TUNNEL_CVLAN_CHECK_EN;
		else
			vlan_cfg->key_bmp &= ~FAL_TUNNEL_CVLAN_CHECK_EN;

		vlan_cfg->l3_if.l3_if_valid = tl_vlan_tbl->bf0.tl_l3_if_valid;
		vlan_cfg->l3_if.l3_if_index = tl_vlan_tbl->bf0.tl_l3_if_index;
		vlan_cfg->pppoe_en = tl_vlan_tbl->bf0.pppoe_en;
	}

	return rv;
}

sw_error_t
adpt_appe_tunnel_vlan_intf_add(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv = SW_OK;
	a_bool_t is_equal = A_FALSE;
	union tl_vlan_tbl_u tl_vlan_tbl;
	a_uint32_t index, update_index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(vlan_cfg);

	aos_mem_zero(&tl_vlan_tbl, sizeof(union tl_vlan_tbl_u));

	index = 0;
	update_index = TL_VLAN_TBL_NUM;
	while (index < TL_VLAN_TBL_NUM) {
		rv = appe_tl_vlan_tbl_get(dev_id, index, &tl_vlan_tbl);
		SW_RTN_ON_ERROR(rv);

		if (tl_vlan_tbl.bf0.valid == A_FALSE && update_index == TL_VLAN_TBL_NUM)
			update_index = index;

		is_equal = adpt_appe_tunnel_vlan_entry_compare(*vlan_cfg, tl_vlan_tbl);
		if (is_equal) {
			return SW_ALREADY_EXIST;
		}
		index++;
	}

	if (update_index != TL_VLAN_TBL_NUM) {
		rv = adpt_appe_tunnel_vlan_entry_convert(vlan_cfg, &tl_vlan_tbl, A_TRUE);
		SW_RTN_ON_ERROR(rv);

		rv = appe_tl_vlan_tbl_set(dev_id, update_index, &tl_vlan_tbl);
	} else {
		return SW_FULL;
	}

	return rv;
}

sw_error_t
adpt_appe_tunnel_vlan_intf_getfirst(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv = SW_OK;
	union tl_vlan_tbl_u tl_vlan_tbl;
	a_uint32_t index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(vlan_cfg);

	aos_mem_zero(&tl_vlan_tbl, sizeof(union tl_vlan_tbl_u));

	index = 0;
	while (index < TL_VLAN_TBL_NUM) {
		rv = appe_tl_vlan_tbl_get(dev_id, index, &tl_vlan_tbl);
		SW_RTN_ON_ERROR(rv);

		if (tl_vlan_tbl.bf0.valid == A_TRUE)
			break;
		index++;
	}

	if (index != TL_VLAN_TBL_NUM) {
		rv = adpt_appe_tunnel_vlan_entry_convert(vlan_cfg, &tl_vlan_tbl, A_FALSE);
		SW_RTN_ON_ERROR(rv);
	} else {
		return SW_NOT_FOUND;
	}

	return rv;
}

sw_error_t
adpt_appe_tunnel_vlan_intf_getnext(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv = SW_OK;
	a_bool_t is_equal = A_FALSE;
	union tl_vlan_tbl_u tl_vlan_tbl;
	a_uint32_t index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(vlan_cfg);

	aos_mem_zero(&tl_vlan_tbl, sizeof(union tl_vlan_tbl_u));

	index = 0;
	while (index < TL_VLAN_TBL_NUM) {
		rv = appe_tl_vlan_tbl_get(dev_id, index, &tl_vlan_tbl);
		SW_RTN_ON_ERROR(rv);

		if (is_equal && tl_vlan_tbl.bf0.valid == A_TRUE)
			break;

		index++;
		if (tl_vlan_tbl.bf0.valid == A_FALSE)
			continue;

		is_equal = adpt_appe_tunnel_vlan_entry_compare(*vlan_cfg, tl_vlan_tbl);
	}

	if (index != TL_VLAN_TBL_NUM) {
		rv = adpt_appe_tunnel_vlan_entry_convert(vlan_cfg, &tl_vlan_tbl, A_FALSE);
		SW_RTN_ON_ERROR(rv);
	} else {
		return SW_NOT_FOUND;
	}

	return rv;
}

sw_error_t
adpt_appe_tunnel_vlan_intf_del(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv = SW_OK;
	a_bool_t is_equal = A_FALSE;
	union tl_vlan_tbl_u tl_vlan_tbl;
	a_uint32_t index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(vlan_cfg);

	aos_mem_zero(&tl_vlan_tbl, sizeof(union tl_vlan_tbl_u));

	index = 0;
	while (index < TL_VLAN_TBL_NUM) {
		rv = appe_tl_vlan_tbl_get(dev_id, index, &tl_vlan_tbl);
		SW_RTN_ON_ERROR(rv);

		is_equal = adpt_appe_tunnel_vlan_entry_compare(*vlan_cfg, tl_vlan_tbl);
		if (is_equal)
			break;
		index++;
	}

	if (index != TL_VLAN_TBL_NUM) {
		aos_mem_zero(&tl_vlan_tbl, sizeof(union tl_vlan_tbl_u));
		rv = appe_tl_vlan_tbl_set(dev_id, index, &tl_vlan_tbl);
	} else {
		return SW_NOT_FOUND;
	}

	return rv;
}
#endif

sw_error_t
adpt_appe_tunnel_intf_set(a_uint32_t dev_id,
		a_uint32_t l3_if, fal_tunnel_intf_t *tunnel_intf)
{
	sw_error_t rv = SW_OK;
	union tl_l3_if_tbl_u tl_l3_if_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_intf);

	aos_mem_zero(&tl_l3_if_tbl, sizeof(union tl_l3_if_tbl_u));

	rv = appe_tl_l3_if_tbl_get(dev_id, l3_if, &tl_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	tl_l3_if_tbl.bf.ipv4_decap_en = tunnel_intf->ipv4_decap_en;
	tl_l3_if_tbl.bf.ipv6_decap_en = tunnel_intf->ipv6_decap_en;
	tl_l3_if_tbl.bf.dmac_check_dis = !tunnel_intf->dmac_check_en;
	tl_l3_if_tbl.bf.ttl_exceed_cmd = tunnel_intf->ttl_exceed_action;
	tl_l3_if_tbl.bf.ttl_exceed_de_acce = tunnel_intf->ttl_exceed_deacce_en;
	tl_l3_if_tbl.bf.lpm_en = tunnel_intf->lpm_en;
	tl_l3_if_tbl.bf.min_ipv6_mtu = tunnel_intf->mini_ipv6_mtu;

	rv = appe_tl_l3_if_tbl_set(dev_id, l3_if, &tl_l3_if_tbl);

	return rv;
}

sw_error_t
adpt_appe_tunnel_intf_get(a_uint32_t dev_id,
		a_uint32_t l3_if, fal_tunnel_intf_t *tunnel_intf)
{
	sw_error_t rv = SW_OK;
	union tl_l3_if_tbl_u tl_l3_if_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_intf);

	aos_mem_zero(&tl_l3_if_tbl, sizeof(union tl_l3_if_tbl_u));

	rv = appe_tl_l3_if_tbl_get(dev_id, l3_if, &tl_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	tunnel_intf->ipv4_decap_en = tl_l3_if_tbl.bf.ipv4_decap_en;
	tunnel_intf->ipv6_decap_en = tl_l3_if_tbl.bf.ipv6_decap_en;
	tunnel_intf->dmac_check_en = !tl_l3_if_tbl.bf.dmac_check_dis;
	tunnel_intf->ttl_exceed_action = tl_l3_if_tbl.bf.ttl_exceed_cmd;
	tunnel_intf->ttl_exceed_deacce_en = tl_l3_if_tbl.bf.ttl_exceed_de_acce;
	tunnel_intf->lpm_en = tl_l3_if_tbl.bf.lpm_en;
	tunnel_intf->mini_ipv6_mtu = tl_l3_if_tbl.bf.min_ipv6_mtu;

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_port_tunnelid_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv = SW_OK;
	union eg_vp_tbl_u eg_vp_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_id);
	TUNNEL_ENCAP_ID_CHECK(tunnel_id->tunnel_id);

	aos_mem_zero(&eg_vp_tbl, sizeof(union eg_vp_tbl_u));

	rv = appe_egress_vp_tbl_get(dev_id, port_value, &eg_vp_tbl);
	SW_RTN_ON_ERROR(rv);

	eg_vp_tbl.bf.tunnel_valid = tunnel_id->tunnel_id_valid;
	eg_vp_tbl.bf.tunnel_id = tunnel_id->tunnel_id;

	rv = appe_egress_vp_tbl_set(dev_id, port_value, &eg_vp_tbl);

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_port_tunnelid_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv = SW_OK;
	union eg_vp_tbl_u eg_vp_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_id);

	aos_mem_zero(&eg_vp_tbl, sizeof(union eg_vp_tbl_u));

	rv = appe_egress_vp_tbl_get(dev_id, port_value, &eg_vp_tbl);
	SW_RTN_ON_ERROR(rv);

	tunnel_id->tunnel_id_valid = eg_vp_tbl.bf.tunnel_valid;
	tunnel_id->tunnel_id = eg_vp_tbl.bf.tunnel_id;

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_intf_tunnelid_set(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv = SW_OK;
	union eg_l3_if_tbl_u eg_l3_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_id);
	TUNNEL_ENCAP_ID_CHECK(tunnel_id->tunnel_id);

	aos_mem_zero(&eg_l3_tbl, sizeof(union eg_l3_if_tbl_u));

	rv = hppe_eg_l3_if_tbl_get(dev_id, intf_id, &eg_l3_tbl);
	SW_RTN_ON_ERROR(rv);

	eg_l3_tbl.bf.tunnel_valid = tunnel_id->tunnel_id_valid;
	eg_l3_tbl.bf.tunnel_id = tunnel_id->tunnel_id;

	rv = hppe_eg_l3_if_tbl_set(dev_id, intf_id, &eg_l3_tbl);

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_intf_tunnelid_get(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv = SW_OK;
	union eg_l3_if_tbl_u eg_l3_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_id);

	aos_mem_zero(&eg_l3_tbl, sizeof(union eg_l3_if_tbl_u));

	rv = hppe_eg_l3_if_tbl_get(dev_id, intf_id, &eg_l3_tbl);
	SW_RTN_ON_ERROR(rv);

	tunnel_id->tunnel_id_valid = eg_l3_tbl.bf.tunnel_valid;
	tunnel_id->tunnel_id = eg_l3_tbl.bf.tunnel_id;

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_entry_add(a_uint32_t dev_id,
		a_uint32_t tunnel_id, fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
	sw_error_t rv = SW_OK;
	union eg_xlat_tun_ctrl_u eg_xlat_tun_ctrl;
	union eg_header_data_u eg_header_data;
	a_uint32_t idx = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_encap_cfg);
	TUNNEL_ENCAP_ID_CHECK(tunnel_id);

	aos_mem_zero(&eg_xlat_tun_ctrl, sizeof(union eg_xlat_tun_ctrl_u));
	aos_mem_zero(&eg_header_data, sizeof(union eg_header_data_u));

	rv = appe_eg_xlat_tun_ctrl_get(dev_id, tunnel_id, &eg_xlat_tun_ctrl);
	SW_RTN_ON_ERROR(rv);
	rv = appe_eg_header_data_get(dev_id, tunnel_id, &eg_header_data);
	SW_RTN_ON_ERROR(rv);

	eg_xlat_tun_ctrl.bf.type = tunnel_encap_cfg->encap_type;
	eg_xlat_tun_ctrl.bf.edit_rule_id = tunnel_encap_cfg->edit_rule_id;
	eg_xlat_tun_ctrl.bf.edit_rule_target = tunnel_encap_cfg->encap_target;
	eg_xlat_tun_ctrl.bf.data_length = tunnel_encap_cfg->tunnel_len;
	eg_xlat_tun_ctrl.bf.vlan_offset = tunnel_encap_cfg->vlan_offset;
	eg_xlat_tun_ctrl.bf.l3_offset = tunnel_encap_cfg->l3_offset;
	eg_xlat_tun_ctrl.bf.pppoe_en = tunnel_encap_cfg->pppoe_en;
	eg_xlat_tun_ctrl.bf.ip_ver = tunnel_encap_cfg->ip_ver;
	eg_xlat_tun_ctrl.bf.dscp_mode = tunnel_encap_cfg->dscp_mode;
	eg_xlat_tun_ctrl.bf.l4_offset = tunnel_encap_cfg->l4_offset;
	eg_xlat_tun_ctrl.bf.tunnel_offset = tunnel_encap_cfg->tunnel_offset;
	eg_xlat_tun_ctrl.bf.stag_fmt = tunnel_encap_cfg->svlan_fmt;
	eg_xlat_tun_ctrl.bf.ctag_fmt = tunnel_encap_cfg->cvlan_fmt;
	eg_xlat_tun_ctrl.bf.spcp_mode = tunnel_encap_cfg->spcp_mode;
	eg_xlat_tun_ctrl.bf.sdei_mode = tunnel_encap_cfg->sdei_mode;
	eg_xlat_tun_ctrl.bf.cpcp_mode = tunnel_encap_cfg->cpcp_mode;
	eg_xlat_tun_ctrl.bf.cdei_mode = tunnel_encap_cfg->cdei_mode;
	eg_xlat_tun_ctrl.bf.ecn_mode = tunnel_encap_cfg->ecn_mode;
	eg_xlat_tun_ctrl.bf.ttl_mode = tunnel_encap_cfg->ttl_mode;
	eg_xlat_tun_ctrl.bf.ipv4_df_mode = tunnel_encap_cfg->ipv4_df_mode >> 1;
	eg_xlat_tun_ctrl.bf.ipv4_df_mode_ext = tunnel_encap_cfg->ipv4_df_mode & 1;
	eg_xlat_tun_ctrl.bf.ipv4_id_mode = tunnel_encap_cfg->ipv4_id_mode;
	eg_xlat_tun_ctrl.bf.ipv6_fl_mode = tunnel_encap_cfg->ipv6_flowlable_mode;
	eg_xlat_tun_ctrl.bf.ip_proto_update = tunnel_encap_cfg->ip_proto_update;
	eg_xlat_tun_ctrl.bf.l4_type = tunnel_encap_cfg->l4_proto;
	eg_xlat_tun_ctrl.bf.sport_entropy_en = tunnel_encap_cfg->sport_entry_en;
	eg_xlat_tun_ctrl.bf.l4_checksum_en = tunnel_encap_cfg->l4_checksum_en;
	eg_xlat_tun_ctrl.bf.vni_mode = tunnel_encap_cfg->vni_mode;
	eg_xlat_tun_ctrl.bf.payload_type = tunnel_encap_cfg->payload_inner_type;
	eg_xlat_tun_ctrl.bf.output_vp_valid = tunnel_encap_cfg->vport_en;
	eg_xlat_tun_ctrl.bf.output_vp= tunnel_encap_cfg->vport;
#if defined(MPPE)
	eg_xlat_tun_ctrl.bf.mapt_udp_csm0_dis = tunnel_encap_cfg->mapt_udp_csm0_keep;
#endif

	aos_mem_copy(eg_header_data.val, tunnel_encap_cfg->pkt_header.pkt_header_data,
			sizeof(union eg_header_data_u));
	/* fix big endian issue */
	for (idx = 0; idx < ARRAY_SIZE(eg_header_data.val); idx++) {
		le32_to_cpus(&eg_header_data.val[idx]);
	}

	rv = appe_eg_xlat_tun_ctrl_set(dev_id, tunnel_id, &eg_xlat_tun_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_header_data_set(dev_id, tunnel_id, &eg_header_data);
	SW_RTN_ON_ERROR(rv);

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_entry_del(a_uint32_t dev_id,
		a_uint32_t tunnel_id)
{
	sw_error_t rv = SW_OK;
	union eg_xlat_tun_ctrl_u eg_xlat_tun_ctrl;
	union eg_header_data_u eg_header_data;

	ADPT_DEV_ID_CHECK(dev_id);
	TUNNEL_ENCAP_ID_CHECK(tunnel_id);

	aos_mem_zero(&eg_xlat_tun_ctrl, sizeof(union eg_xlat_tun_ctrl_u));
	aos_mem_zero(&eg_header_data, sizeof(union eg_header_data_u));

	rv = appe_eg_xlat_tun_ctrl_set(dev_id, tunnel_id, &eg_xlat_tun_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = appe_eg_header_data_set(dev_id, tunnel_id, &eg_header_data);

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_entry_get(a_uint32_t dev_id,
		a_uint32_t tunnel_id, fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
	sw_error_t rv = SW_OK;
	union eg_xlat_tun_ctrl_u eg_xlat_tun_ctrl;
	union eg_header_data_u eg_header_data;
	a_uint32_t idx = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_encap_cfg);
	TUNNEL_ENCAP_ID_CHECK(tunnel_id);

	aos_mem_zero(&eg_xlat_tun_ctrl, sizeof(union eg_xlat_tun_ctrl_u));
	aos_mem_zero(&eg_header_data, sizeof(union eg_header_data_u));

	rv = appe_eg_xlat_tun_ctrl_get(dev_id, tunnel_id, &eg_xlat_tun_ctrl);
	SW_RTN_ON_ERROR(rv);
	rv = appe_eg_header_data_get(dev_id, tunnel_id, &eg_header_data);
	SW_RTN_ON_ERROR(rv);

	tunnel_encap_cfg->encap_type = eg_xlat_tun_ctrl.bf.type;
	tunnel_encap_cfg->edit_rule_id = eg_xlat_tun_ctrl.bf.edit_rule_id;
	tunnel_encap_cfg->encap_target = eg_xlat_tun_ctrl.bf.edit_rule_target;
	tunnel_encap_cfg->tunnel_len = eg_xlat_tun_ctrl.bf.data_length;
	tunnel_encap_cfg->vlan_offset = eg_xlat_tun_ctrl.bf.vlan_offset;
	tunnel_encap_cfg->l3_offset = eg_xlat_tun_ctrl.bf.l3_offset;
	tunnel_encap_cfg->pppoe_en = eg_xlat_tun_ctrl.bf.pppoe_en;
	tunnel_encap_cfg->ip_ver = eg_xlat_tun_ctrl.bf.ip_ver;
	tunnel_encap_cfg->dscp_mode = eg_xlat_tun_ctrl.bf.dscp_mode;
	tunnel_encap_cfg->l4_offset = eg_xlat_tun_ctrl.bf.l4_offset;
	tunnel_encap_cfg->tunnel_offset = eg_xlat_tun_ctrl.bf.tunnel_offset;
	tunnel_encap_cfg->svlan_fmt = eg_xlat_tun_ctrl.bf.stag_fmt;
	tunnel_encap_cfg->cvlan_fmt = eg_xlat_tun_ctrl.bf.ctag_fmt;
	tunnel_encap_cfg->spcp_mode = eg_xlat_tun_ctrl.bf.spcp_mode;
	tunnel_encap_cfg->sdei_mode = eg_xlat_tun_ctrl.bf.sdei_mode;
	tunnel_encap_cfg->cpcp_mode = eg_xlat_tun_ctrl.bf.cpcp_mode;
	tunnel_encap_cfg->cdei_mode = eg_xlat_tun_ctrl.bf.cdei_mode;
	tunnel_encap_cfg->ecn_mode = eg_xlat_tun_ctrl.bf.ecn_mode;
	tunnel_encap_cfg->ttl_mode = eg_xlat_tun_ctrl.bf.ttl_mode;
	tunnel_encap_cfg->ipv4_df_mode = eg_xlat_tun_ctrl.bf.ipv4_df_mode << 1 |
		eg_xlat_tun_ctrl.bf.ipv4_df_mode_ext;
	tunnel_encap_cfg->ipv4_id_mode = eg_xlat_tun_ctrl.bf.ipv4_id_mode;
	tunnel_encap_cfg->ipv6_flowlable_mode = eg_xlat_tun_ctrl.bf.ipv6_fl_mode;
	tunnel_encap_cfg->ip_proto_update = eg_xlat_tun_ctrl.bf.ip_proto_update;
	tunnel_encap_cfg->l4_proto = eg_xlat_tun_ctrl.bf.l4_type;
	tunnel_encap_cfg->sport_entry_en = eg_xlat_tun_ctrl.bf.sport_entropy_en;
	tunnel_encap_cfg->l4_checksum_en = eg_xlat_tun_ctrl.bf.l4_checksum_en;
	tunnel_encap_cfg->vni_mode = eg_xlat_tun_ctrl.bf.vni_mode;
	tunnel_encap_cfg->payload_inner_type = eg_xlat_tun_ctrl.bf.payload_type;
	tunnel_encap_cfg->vport_en = eg_xlat_tun_ctrl.bf.output_vp_valid;
	tunnel_encap_cfg->vport = eg_xlat_tun_ctrl.bf.output_vp;
#if defined(MPPE)
	tunnel_encap_cfg->mapt_udp_csm0_keep = eg_xlat_tun_ctrl.bf.mapt_udp_csm0_dis;
#endif

	/* fix big endian issue */
	for (idx = 0; idx < ARRAY_SIZE(eg_header_data.val); idx++) {
		cpu_to_le32s(&eg_header_data.val[idx]);
	}
	aos_mem_copy(tunnel_encap_cfg->pkt_header.pkt_header_data, eg_header_data.val,
			sizeof(union eg_header_data_u));

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_entry_getnext(a_uint32_t dev_id,
		a_uint32_t tunnel_id, fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
	sw_error_t rv = SW_OK;
	a_uint32_t index;
	union eg_l3_if_tbl_u eg_l3_if;
	union eg_vp_tbl_u eg_vp;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tunnel_encap_cfg);
	TUNNEL_ENCAP_ID_CHECK(tunnel_id);

	aos_mem_zero(&eg_l3_if, sizeof(union eg_l3_if_tbl_u));
	aos_mem_zero(&eg_vp, sizeof(union eg_vp_tbl_u));

	while (tunnel_id < EG_XLAT_TUN_CTRL_NUM) {
		index = 0;
		/* loop check the tunnel id of egress l3 interface */
		while (index < EG_L3_IF_TBL_NUM) {
			rv = hppe_eg_l3_if_tbl_get(dev_id, index, &eg_l3_if);
			SW_RTN_ON_ERROR(rv);

			if (eg_l3_if.bf.tunnel_valid == A_TRUE &&
					eg_l3_if.bf.tunnel_id == tunnel_id)
				break;
			index++;
		}

		if (index < EG_L3_IF_TBL_NUM) {
			rv = adpt_appe_tunnel_encap_entry_get(dev_id, tunnel_id, tunnel_encap_cfg);
			tunnel_encap_cfg->rt_tunnel_id = tunnel_id;
			tunnel_encap_cfg->rt_tlid_type = TUNNEL_ENCAP_FROM_L3IF;
			tunnel_encap_cfg->rt_tlid_index = index;
			return rv;
		}

		/* loop check the tunnel id of egress virtual port */
		index = 0;
		while (index < EG_VP_TBL_NUM) {
			rv = appe_egress_vp_tbl_get(dev_id, index, &eg_vp);
			SW_RTN_ON_ERROR(rv);

			if (eg_vp.bf.tunnel_valid == A_TRUE &&
					eg_vp.bf.tunnel_id == tunnel_id)
				break;
			index++;
		}

		if (index < EG_VP_TBL_NUM) {
			rv = adpt_appe_tunnel_encap_entry_get(dev_id, tunnel_id, tunnel_encap_cfg);
			tunnel_encap_cfg->rt_tunnel_id = tunnel_id;
			tunnel_encap_cfg->rt_tlid_type = TUNNEL_ENCAP_FROM_VP;
			tunnel_encap_cfg->rt_tlid_index = index;
			return rv;
		}

		/* loop the next tunnel id */
		tunnel_id++;
	}

	tunnel_encap_cfg->rt_tunnel_id = tunnel_id;
	TUNNEL_ENCAP_ID_CHECK(tunnel_id);

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_rule_entry_set(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_tunnel_encap_rule_t *rule_entry)
{
	sw_error_t rv = SW_OK;
	union eg_edit_rule_u eg_edit_rule;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule_entry);

	if (rule_id >= EG_EDIT_RULE_NUM) {
		return SW_OUT_OF_RANGE;
	}

	aos_mem_zero(&eg_edit_rule, sizeof(union eg_edit_rule_u));

	rv = appe_eg_edit_rule_get(dev_id, rule_id, &eg_edit_rule);
	SW_RTN_ON_ERROR(rv);

	if (rule_entry->src1_sel == FAL_TUNNEL_RULE_SRC1_ZERO_DATA) {
		eg_edit_rule.bf.src1 = 0;
	} else {
		eg_edit_rule.bf.src1 = (rule_entry->src1_start >> 1) +
			FAL_TUNNEL_ENCAP_SRC1_DATA_WORD_START;
	}

	eg_edit_rule.bf.src2 = rule_entry->src2_sel;
	eg_edit_rule.bf.valid2_0 = rule_entry->src2_entry[0].enable;
	eg_edit_rule.bf.start2_0 = rule_entry->src2_entry[0].src_start;
	/* hardware adds 1 based on the width, so minus 1 from the configuration */
	eg_edit_rule.bf.width2_0 = rule_entry->src2_entry[0].src_width - 1;
	eg_edit_rule.bf.pos2_0 = rule_entry->src2_entry[0].dest_pos;
	eg_edit_rule.bf.valid2_1 = rule_entry->src2_entry[1].enable;
	eg_edit_rule.bf.start2_1_0 = rule_entry->src2_entry[1].src_start;
	eg_edit_rule.bf.start2_1_1 = rule_entry->src2_entry[1].src_start >>
		SW_FIELD_OFFSET_IN_WORD(EG_EDIT_RULE_START2_1_OFFSET);
	eg_edit_rule.bf.width2_1 = rule_entry->src2_entry[1].src_width - 1;
	eg_edit_rule.bf.pos2_1 = rule_entry->src2_entry[1].dest_pos;
	eg_edit_rule.bf.src3 = rule_entry->src3_sel;
	eg_edit_rule.bf.valid3_0 = rule_entry->src3_entry[0].enable;
	eg_edit_rule.bf.start3_0 = rule_entry->src3_entry[0].src_start;
	eg_edit_rule.bf.width3_0 = rule_entry->src3_entry[0].src_width - 1;
	eg_edit_rule.bf.pos3_0 = rule_entry->src3_entry[0].dest_pos;
	eg_edit_rule.bf.valid3_1 = rule_entry->src3_entry[1].enable;
	eg_edit_rule.bf.start3_1 = rule_entry->src3_entry[1].src_start;
	eg_edit_rule.bf.width3_1 = rule_entry->src3_entry[1].src_width - 1;
	eg_edit_rule.bf.pos3_1 = rule_entry->src3_entry[1].dest_pos;

	rv = appe_eg_edit_rule_set(dev_id, rule_id, &eg_edit_rule);

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_rule_entry_get(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_tunnel_encap_rule_t *rule_entry)
{
	sw_error_t rv = SW_OK;
	union eg_edit_rule_u eg_edit_rule;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule_entry);

	if (rule_id >= EG_EDIT_RULE_NUM) {
		return SW_OUT_OF_RANGE;
	}

	aos_mem_zero(&eg_edit_rule, sizeof(union eg_edit_rule_u));

	rv = appe_eg_edit_rule_get(dev_id, rule_id, &eg_edit_rule);
	SW_RTN_ON_ERROR(rv);

	if (!eg_edit_rule.bf.src1) {
		rule_entry->src1_sel = FAL_TUNNEL_RULE_SRC1_ZERO_DATA;
	} else {
		rule_entry->src1_sel = FAL_TUNNEL_RULE_SRC1_FROM_HEADER_DATA;
	}

	rule_entry->src1_start = (eg_edit_rule.bf.src1 - 1) <<
		FAL_TUNNEL_ENCAP_SRC1_DATA_WORD_START;
	rule_entry->src2_sel = eg_edit_rule.bf.src2;
	rule_entry->src2_entry[0].enable = eg_edit_rule.bf.valid2_0;
	rule_entry->src2_entry[0].src_start = eg_edit_rule.bf.start2_0;
	rule_entry->src2_entry[0].src_width = eg_edit_rule.bf.width2_0 + 1;
	rule_entry->src2_entry[0].dest_pos = eg_edit_rule.bf.pos2_0;
	rule_entry->src2_entry[1].enable = eg_edit_rule.bf.valid2_1;
	rule_entry->src2_entry[1].src_start = eg_edit_rule.bf.start2_1_0 |
		eg_edit_rule.bf.start2_1_1 <<
		SW_FIELD_OFFSET_IN_WORD(EG_EDIT_RULE_START2_1_OFFSET);
	rule_entry->src2_entry[1].src_width = eg_edit_rule.bf.width2_1 + 1;
	rule_entry->src2_entry[1].dest_pos = eg_edit_rule.bf.pos2_1;
	rule_entry->src3_sel = eg_edit_rule.bf.src3;
	rule_entry->src3_entry[0].enable = eg_edit_rule.bf.valid3_0;
	rule_entry->src3_entry[0].src_start = eg_edit_rule.bf.start3_0;
	rule_entry->src3_entry[0].src_width = eg_edit_rule.bf.width3_0 + 1;
	rule_entry->src3_entry[0].dest_pos = eg_edit_rule.bf.pos3_0;
	rule_entry->src3_entry[1].enable = eg_edit_rule.bf.valid3_1;
	rule_entry->src3_entry[1].src_start = eg_edit_rule.bf.start3_1;
	rule_entry->src3_entry[1].src_width = eg_edit_rule.bf.width3_1 + 1;
	rule_entry->src3_entry[1].dest_pos = eg_edit_rule.bf.pos3_1;

	return rv;
}

sw_error_t
adpt_appe_tunnel_encap_rule_entry_del(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_tunnel_encap_rule_t *rule_entry)
{
	sw_error_t rv = SW_OK;
	union eg_edit_rule_u eg_edit_rule;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(rule_entry);

	if (rule_id >= EG_EDIT_RULE_NUM) {
		return SW_OUT_OF_RANGE;
	}

	aos_mem_zero(&eg_edit_rule, sizeof(union eg_edit_rule_u));

	rv = appe_eg_edit_rule_set(dev_id, rule_id, &eg_edit_rule);

	return rv;
}

static a_bool_t
_adpt_appe_is_udf_profile_entry_equal(a_uint32_t dev_id,
		fal_tunnel_udf_profile_entry_t * entry1, fal_tunnel_udf_profile_entry_t * entry2)
{
	if (entry1->field_flag == entry2->field_flag &&
		entry1->l3_type == entry2->l3_type &&
		entry1->l4_type == entry2->l4_type &&
		entry1->overlay_type == entry2->overlay_type &&
		entry1->program_type == entry2->program_type)
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
		a_uint32_t index, fal_tunnel_udf_profile_entry_t * entry, a_uint32_t * profile_id)
{
	union tpr_udf_ctrl_0_u udf_ctrl = {0};
	sw_error_t rv = SW_OK;

	rv = appe_tpr_udf_ctrl_0_get(dev_id, index, &udf_ctrl);
	if (rv != SW_OK)
		return A_FALSE;

	if (!udf_ctrl.bf.valid)
	{
		aos_mem_zero(&udf_ctrl, sizeof (udf_ctrl));
	}
	entry->l3_type = udf_ctrl.bf.l3_type;
	entry->l4_type = udf_ctrl.bf.l4_type;
	entry->overlay_type = udf_ctrl.bf.overlay_type;
	entry->program_type = udf_ctrl.bf.program_type;
	*profile_id = udf_ctrl.bf.udf_profile;

	if (udf_ctrl.bf.l3_type_incl)
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
	}
	else
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_CLR(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE);
	}
	if (udf_ctrl.bf.l4_type_incl)
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
	}
	else
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_CLR(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE);
	}
	if (udf_ctrl.bf.overlay_type_incl)
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_OVERLAY_TYPE);
	}
	else
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_CLR(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_OVERLAY_TYPE);
	}
	if (udf_ctrl.bf.program_type_incl)
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_SET(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_PROGRAM_TYPE);
	}
	else
	{
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_CLR(entry->field_flag,
			FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_PROGRAM_TYPE);
	}
	return udf_ctrl.bf.valid;
}

static sw_error_t
_adpt_appe_insert_udf_profile_entry_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_tunnel_udf_profile_entry_t * entry, a_uint32_t profile_id)
{
	union tpr_udf_ctrl_0_u udf_ctrl = {0};

	udf_ctrl.bf.valid = A_TRUE;
	udf_ctrl.bf.l3_type = entry->l3_type;
	udf_ctrl.bf.l4_type = entry->l4_type;
	udf_ctrl.bf.overlay_type = entry->overlay_type;
	udf_ctrl.bf.program_type = entry->program_type;
	udf_ctrl.bf.udf_profile = profile_id;

	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE))
	{
		udf_ctrl.bf.l3_type_incl = A_TRUE;
	}
	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE))
	{
		udf_ctrl.bf.l4_type_incl = A_TRUE;
	}
	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_OVERLAY_TYPE))
	{
		udf_ctrl.bf.overlay_type_incl = A_TRUE;
	}
	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_PROGRAM_TYPE))
	{
		udf_ctrl.bf.program_type_incl = A_TRUE;
	}
	return appe_tpr_udf_ctrl_0_set(dev_id, index, &udf_ctrl);
}

static a_uint32_t
_adpt_appe_udf_profile_entry_weight(fal_tunnel_udf_profile_entry_t * entry)
{
	a_uint32_t weight = 0;
	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L3_TYPE))
	{
		weight++;
	}
	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_L4_TYPE))
	{
		weight++;
	}
	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_OVERLAY_TYPE))
	{
		weight++;
	}
	if (FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_FLG_TST(entry->field_flag,
		FAL_TUNNEL_UDF_PROFILE_ENTRY_FIELD_PROGRAM_TYPE))
	{
		weight++;
	}
	return weight;
}

static sw_error_t
_adpt_appe_insert_udf_profile_entry_by_sort(a_uint32_t dev_id,
		a_uint32_t index, fal_tunnel_udf_profile_entry_t * entry, a_uint32_t profile_id)
{
	fal_tunnel_udf_profile_entry_t temp_entry = {0};
	a_uint32_t temp_profile_id;
	a_uint32_t idx, weight, temp_weight;

	weight = _adpt_appe_udf_profile_entry_weight(entry);

	for (idx = index; idx < TPR_UDF_CTRL_0_MAX_ENTRY-1; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_udf_profile_entry_t));
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

static sw_error_t
_adpt_appe_tunnel_udf_profile_entry_get(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry, a_bool_t sign_tag)
{
	a_uint32_t idx, temp_profile_id;
	a_bool_t entry_valid;
	fal_tunnel_udf_profile_entry_t temp_entry;

	ADPT_DEV_ID_CHECK(dev_id);

	if (profile_id >= TPR_UDF_PROFILE_BASE_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	for (idx = 0; idx < TPR_UDF_CTRL_0_MAX_ENTRY; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_udf_profile_entry_t));
		entry_valid = _adpt_appe_get_udf_profile_entry_by_index(dev_id, idx,
						&temp_entry, &temp_profile_id);
		if (entry_valid == A_TRUE)
		{
			if (temp_profile_id == profile_id)
			{
				if (sign_tag == A_TRUE)
				{
					aos_mem_copy(entry, &temp_entry,
						sizeof (fal_tunnel_udf_profile_entry_t));
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
	if (idx == TPR_UDF_CTRL_0_MAX_ENTRY)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_udf_profile_entry_add(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
	a_uint32_t entry_idx, temp_profile_id;
	a_bool_t entry_valid;
	a_int32_t idx;
	fal_tunnel_udf_profile_entry_t temp_entry;

	ADPT_DEV_ID_CHECK(dev_id);

	if (profile_id >= TPR_UDF_PROFILE_BASE_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	for (idx = TPR_UDF_CTRL_0_MAX_ENTRY-1; idx >= 0; idx--)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_udf_profile_entry_t));
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
adpt_appe_tunnel_udf_profile_entry_del(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
	a_uint32_t idx, temp_profile_id;
	a_int32_t j;
	a_bool_t entry_valid;
	fal_tunnel_udf_profile_entry_t temp_entry;
	union tpr_udf_ctrl_0_u udf_ctrl_zero_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if (profile_id >= TPR_UDF_PROFILE_BASE_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	for (idx = 0; idx < TPR_UDF_CTRL_0_MAX_ENTRY; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_udf_profile_entry_t));
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

	if (idx == TPR_UDF_CTRL_0_MAX_ENTRY)
	{
		return SW_NOT_FOUND;
	}

	/* clear the find entry and resort the entries */
	for (j = idx; j > 0; j --)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_udf_profile_entry_t));
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
	return appe_tpr_udf_ctrl_0_set(dev_id, j, &udf_ctrl_zero_entry);
}

sw_error_t
adpt_appe_tunnel_udf_profile_entry_getfirst(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
	return _adpt_appe_tunnel_udf_profile_entry_get(dev_id, profile_id, entry, A_TRUE);
}

sw_error_t
adpt_appe_tunnel_udf_profile_entry_getnext(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
	return _adpt_appe_tunnel_udf_profile_entry_get(dev_id, profile_id, entry, A_FALSE);
}

sw_error_t
adpt_appe_tunnel_udf_profile_cfg_set(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t udf_type, a_uint32_t offset)
{
	a_uint8_t udf_base = 0;
	union tpr_udf_profile_base_u udf_profile_base = {0};
	union tpr_udf_profile_offset_u udf_profile_offset = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if (offset % 2)
	{ /*only support even data*/
		return SW_BAD_VALUE;
	}

	switch(udf_type)
	{
		case FAL_TUNNEL_UDF_TYPE_L2:
			udf_base = 0;
			break;
		case FAL_TUNNEL_UDF_TYPE_L3:
			udf_base = 1;
			break;
		case FAL_TUNNEL_UDF_TYPE_L4:
			udf_base = 2;
			break;
		case FAL_TUNNEL_UDF_TYPE_OVERLAY:
			udf_base = 3;
			break;
		case FAL_TUNNEL_UDF_TYPE_PROGRAM:
			udf_base = 4;
			break;
		case FAL_TUNNEL_UDF_TYPE_PAYLOAD:
			udf_base = 5;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	SW_RTN_ON_ERROR(appe_tpr_udf_profile_base_get(dev_id, profile_id, &udf_profile_base));
	SW_RTN_ON_ERROR(appe_tpr_udf_profile_offset_get(dev_id, profile_id, &udf_profile_offset));

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

	SW_RTN_ON_ERROR(appe_tpr_udf_profile_base_set(dev_id, profile_id, &udf_profile_base));
	return appe_tpr_udf_profile_offset_set(dev_id, profile_id, &udf_profile_offset);
}

sw_error_t
adpt_appe_tunnel_udf_profile_cfg_get(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t * udf_type, a_uint32_t * offset)
{
	a_uint8_t udf_base = 0;
	union tpr_udf_profile_base_u udf_profile_base = {0};
	union tpr_udf_profile_offset_u udf_profile_offset = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(udf_type);
	ADPT_NULL_POINT_CHECK(offset);

	SW_RTN_ON_ERROR(appe_tpr_udf_profile_base_get(dev_id, profile_id, &udf_profile_base));
	SW_RTN_ON_ERROR(appe_tpr_udf_profile_offset_get(dev_id, profile_id, &udf_profile_offset));

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
			*udf_type = FAL_TUNNEL_UDF_TYPE_L2;
			break;
		case 1:
			*udf_type = FAL_TUNNEL_UDF_TYPE_L3;
			break;
		case 2:
			*udf_type = FAL_TUNNEL_UDF_TYPE_L4;
			break;
		case 3:
			*udf_type = FAL_TUNNEL_UDF_TYPE_OVERLAY;
			break;
		case 4:
			*udf_type = FAL_TUNNEL_UDF_TYPE_PROGRAM;
			break;
		case 5:
			*udf_type = FAL_TUNNEL_UDF_TYPE_PAYLOAD;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_exp_decap_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_val = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_val, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.exception_fmt_ctrl = *enable;

	rv = appe_l2_vp_port_tbl_set(dev_id, port_val, &l2_vp_port_tbl);

	return rv;
}

sw_error_t
adpt_appe_tunnel_exp_decap_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_val = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_val, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	*enable = l2_vp_port_tbl.bf.exception_fmt_ctrl;

	return rv;
}

sw_error_t adpt_appe_tunnel_decap_key_set(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *key_gen)
{
	sw_error_t rv = SW_OK;

	rv = adpt_appe_tunnel_key_op(dev_id, tunnel_type, FAL_TUNNEL_OP_TYPE_ADD, key_gen);
	return rv;
}

sw_error_t adpt_appe_tunnel_decap_key_get(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *key_gen)
{
	sw_error_t rv = SW_OK;

	rv = adpt_appe_tunnel_key_op(dev_id, tunnel_type, FAL_TUNNEL_OP_TYPE_GET, key_gen);
	return rv;
}

sw_error_t adpt_appe_tunnel_decap_en_set(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t en)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	TUNNEL_DECAP_ID_CHECK(tunnel_index);

	rv = appe_tl_tbl_decap_en_set(dev_id, tunnel_index, en);

	return rv;
}

sw_error_t adpt_appe_tunnel_decap_en_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t *en)
{
	sw_error_t rv = SW_OK;
	a_uint32_t decap_en;

	ADPT_DEV_ID_CHECK(dev_id);
	TUNNEL_DECAP_ID_CHECK(tunnel_index);
	ADPT_NULL_POINT_CHECK(en);

	rv = appe_tl_tbl_decap_en_get(dev_id, tunnel_index, &decap_en);
	SW_RTN_ON_ERROR(rv);

	*en = !!decap_en;

	return rv;
}

sw_error_t adpt_appe_tunnel_decap_action_update(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_tunnel_action_t *update_action)
{
	sw_error_t rv = SW_OK;
	union tl_tbl_u tl_tbl;
	a_uint32_t update_bitmap = 0;
	a_uint8_t update_field = FAL_TUNNEL_SVLAN_UPDATE;

	ADPT_DEV_ID_CHECK(dev_id);
	TUNNEL_DECAP_ID_CHECK(tunnel_index);
	ADPT_NULL_POINT_CHECK(update_action);

	aos_mem_zero(&tl_tbl, sizeof(union tl_tbl_u));

	rv = appe_tl_tbl_get(dev_id, tunnel_index, &tl_tbl);
	SW_RTN_ON_ERROR(rv);

	update_bitmap = update_action->update_bmp;
	while (update_bitmap) {
		if (update_bitmap & 1) {
			switch (update_field) {
				case FAL_TUNNEL_SVLAN_UPDATE:
					tl_tbl.bf0.svlan_fmt =
						update_action->verify_entry.svlan_fmt;
					tl_tbl.bf0.svlan_id_0 =
						update_action->verify_entry.svlan_id;
					tl_tbl.bf0.svlan_id_1 =
						update_action->verify_entry.svlan_id >>
						SW_FIELD_OFFSET_IN_WORD(TL_TBL_SVLAN_ID_OFFSET);
					tl_tbl.bf0.svlan_check_en =
						update_action->verify_entry.verify_bmp &
						FAL_TUNNEL_SVLAN_CHECK_EN ? A_TRUE : A_FALSE;
					break;
				case FAL_TUNNEL_CVLAN_UPDATE:
					tl_tbl.bf0.cvlan_fmt =
						update_action->verify_entry.cvlan_fmt;
					tl_tbl.bf0.cvlan_id =
						update_action->verify_entry.cvlan_id;
					tl_tbl.bf0.cvlan_check_en =
						update_action->verify_entry.verify_bmp &
						FAL_TUNNEL_CVLAN_CHECK_EN ? A_TRUE : A_FALSE;
					break;
				case FAL_TUNNEL_L3IF_UPDATE:
					tl_tbl.bf0.tl_l3_if = update_action->verify_entry.tl_l3_if;
					tl_tbl.bf0.tl_l3_if_check_en =
						update_action->verify_entry.verify_bmp &
						FAL_TUNNEL_L3IF_CHECK_EN ? A_TRUE : A_FALSE;
					break;
				case FAL_TUNNEL_DECAP_UPDATE:
					tl_tbl.bf0.decap_en = update_action->decap_en;
					break;
				case FAL_TUNNEL_DEACCE_UPDATE:
					tl_tbl.bf0.de_acce = update_action->deacce_en;
					break;
				case FAL_TUNNEL_SRCINFO_UPDATE:
					tl_tbl.bf0.src_info_valid = update_action->src_info_enable;
					tl_tbl.bf0.src_info_type = update_action->src_info_type;
					tl_tbl.bf0.src_info = update_action->src_info;
					break;
				case FAL_TUNNEL_PKT_MODE_UPDATE:
					tl_tbl.bf0.spcp_mode = update_action->spcp_mode;
					tl_tbl.bf0.sdei_mode = update_action->sdei_mode;
					tl_tbl.bf0.cpcp_mode = update_action->cpcp_mode;
					tl_tbl.bf0.cdei_mode = update_action->cdei_mode;
					tl_tbl.bf0.ttl_mode = update_action->ttl_mode;
					tl_tbl.bf0.dscp_mode = update_action->dscp_mode;
					tl_tbl.bf0.ecn_mode = update_action->ecn_mode;
					break;
				case FAL_TUNNEL_SERVICE_CODE_UPDATE:
					tl_tbl.bf0.service_code_en =
						update_action->service_code_en;
					tl_tbl.bf0.service_code = update_action->service_code;
					break;
				case FAL_TUNNEL_UDP_CSUM_ZERO_UPDATE:
					tl_tbl.bf0.udp_csum_zero = update_action->udp_csum_zero;
					break;
				case FAL_TUNNEL_EXP_PROFILE_UPDATE:
					tl_tbl.bf0.exp_profile = update_action->exp_profile;
					break;
				case FAL_TUNNEL_FWD_CMD_UPDATE:
					tl_tbl.bf0.fwd_type = update_action->fwd_cmd;
					break;
				default:
					break;
			}
		}
		update_bitmap >>= 1;
		update_field++;
	}
	rv = appe_tl_tbl_set(dev_id, tunnel_index, &tl_tbl);
	return rv;
}

sw_error_t adpt_appe_tunnel_decap_counter_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_entry_counter_t *decap_counter)
{
	sw_error_t rv = SW_OK;
	union tl_cnt_tbl_u tl_cnt_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	TUNNEL_DECAP_ID_CHECK(tunnel_index);
	ADPT_NULL_POINT_CHECK(decap_counter);

	aos_mem_zero(&tl_cnt_tbl, sizeof(union tl_cnt_tbl_u));

	rv = appe_tl_cnt_tbl_get(dev_id, tunnel_index, &tl_cnt_tbl);
	SW_RTN_ON_ERROR(rv);

	decap_counter->matched_pkts = tl_cnt_tbl.bf.rx_pkt_cnt;
	decap_counter->matched_bytes = ((a_uint64_t)tl_cnt_tbl.bf.rx_byte_cnt_1 <<
			SW_FIELD_OFFSET_IN_WORD(TL_CNT_TBL_RX_BYTE_CNT_OFFSET)) |
		tl_cnt_tbl.bf.rx_byte_cnt_0;

	return rv;
}

sw_error_t
adpt_appe_tunnel_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_tunnel_decap_entry_add =
		adpt_appe_tunnel_decap_entry_add;
	p_adpt_api->adpt_tunnel_decap_entry_del =
		adpt_appe_tunnel_decap_entry_del;
	p_adpt_api->adpt_tunnel_decap_entry_get =
		adpt_appe_tunnel_decap_entry_get;
	p_adpt_api->adpt_tunnel_decap_entry_getnext =
		adpt_appe_tunnel_decap_entry_getnext;
	p_adpt_api->adpt_tunnel_decap_entry_flush =
		adpt_appe_tunnel_decap_entry_flush;
	p_adpt_api->adpt_tunnel_global_cfg_set =
		adpt_appe_tunnel_global_cfg_set;
	p_adpt_api->adpt_tunnel_global_cfg_get =
		adpt_appe_tunnel_global_cfg_get;
	p_adpt_api->adpt_tunnel_port_intf_set =
		adpt_appe_tunnel_port_intf_set;
	p_adpt_api->adpt_tunnel_port_intf_get =
		adpt_appe_tunnel_port_intf_get;
#ifndef IN_TUNNEL_MINI
	p_adpt_api->adpt_tunnel_vlan_intf_add =
		adpt_appe_tunnel_vlan_intf_add;
	p_adpt_api->adpt_tunnel_vlan_intf_getfirst =
		adpt_appe_tunnel_vlan_intf_getfirst;
	p_adpt_api->adpt_tunnel_vlan_intf_getnext =
		adpt_appe_tunnel_vlan_intf_getnext;
	p_adpt_api->adpt_tunnel_vlan_intf_del =
		adpt_appe_tunnel_vlan_intf_del;
#endif
	p_adpt_api->adpt_tunnel_intf_set =
		adpt_appe_tunnel_intf_set;
	p_adpt_api->adpt_tunnel_intf_get =
		adpt_appe_tunnel_intf_get;
	p_adpt_api->adpt_tunnel_encap_port_tunnelid_set =
		adpt_appe_tunnel_encap_port_tunnelid_set;
	p_adpt_api->adpt_tunnel_encap_port_tunnelid_get =
		adpt_appe_tunnel_encap_port_tunnelid_get;
	p_adpt_api->adpt_tunnel_encap_intf_tunnelid_set =
		adpt_appe_tunnel_encap_intf_tunnelid_set;
	p_adpt_api->adpt_tunnel_encap_intf_tunnelid_get =
		adpt_appe_tunnel_encap_intf_tunnelid_get;
	p_adpt_api->adpt_tunnel_encap_entry_add =
		adpt_appe_tunnel_encap_entry_add;
	p_adpt_api->adpt_tunnel_encap_entry_del =
		adpt_appe_tunnel_encap_entry_del;
	p_adpt_api->adpt_tunnel_encap_entry_get =
		adpt_appe_tunnel_encap_entry_get;
	p_adpt_api->adpt_tunnel_encap_entry_getnext =
		adpt_appe_tunnel_encap_entry_getnext;
	p_adpt_api->adpt_tunnel_encap_rule_entry_set =
		adpt_appe_tunnel_encap_rule_entry_set;
	p_adpt_api->adpt_tunnel_encap_rule_entry_get =
		adpt_appe_tunnel_encap_rule_entry_get;
	p_adpt_api->adpt_tunnel_encap_rule_entry_del =
		adpt_appe_tunnel_encap_rule_entry_del;
	p_adpt_api->adpt_tunnel_encap_header_ctrl_set =
		adpt_appe_tunnel_encap_header_ctrl_set;
	p_adpt_api->adpt_tunnel_encap_header_ctrl_get =
		adpt_appe_tunnel_encap_header_ctrl_get;
#ifndef IN_TUNNEL_MINI
	p_adpt_api->adpt_tunnel_decap_ecn_mode_set=
		adpt_appe_tunnel_decap_ecn_mode_set;
	p_adpt_api->adpt_tunnel_decap_ecn_mode_get=
		adpt_appe_tunnel_decap_ecn_mode_get;
	p_adpt_api->adpt_tunnel_encap_ecn_mode_set=
		adpt_appe_tunnel_encap_ecn_mode_set;
	p_adpt_api->adpt_tunnel_encap_ecn_mode_get=
		adpt_appe_tunnel_encap_ecn_mode_get;
#endif
	p_adpt_api->adpt_tunnel_udf_profile_entry_add =
		adpt_appe_tunnel_udf_profile_entry_add;
	p_adpt_api->adpt_tunnel_udf_profile_entry_del =
		adpt_appe_tunnel_udf_profile_entry_del;
	p_adpt_api->adpt_tunnel_udf_profile_entry_getfirst =
		adpt_appe_tunnel_udf_profile_entry_getfirst;
	p_adpt_api->adpt_tunnel_udf_profile_entry_getnext =
		adpt_appe_tunnel_udf_profile_entry_getnext;
	p_adpt_api->adpt_tunnel_udf_profile_cfg_set =
		adpt_appe_tunnel_udf_profile_cfg_set;
	p_adpt_api->adpt_tunnel_udf_profile_cfg_get =
		adpt_appe_tunnel_udf_profile_cfg_get;
	p_adpt_api->adpt_tunnel_exp_decap_set=
		adpt_appe_tunnel_exp_decap_set;
	p_adpt_api->adpt_tunnel_exp_decap_get=
		adpt_appe_tunnel_exp_decap_get;
	p_adpt_api->adpt_tunnel_decap_key_set =
		adpt_appe_tunnel_decap_key_set;
	p_adpt_api->adpt_tunnel_decap_key_get =
		adpt_appe_tunnel_decap_key_get;
	p_adpt_api->adpt_tunnel_decap_en_set =
		adpt_appe_tunnel_decap_en_set;
	p_adpt_api->adpt_tunnel_decap_en_get =
		adpt_appe_tunnel_decap_en_get;
	p_adpt_api->adpt_tunnel_decap_action_update =
		adpt_appe_tunnel_decap_action_update;
	p_adpt_api->adpt_tunnel_decap_counter_get =
		adpt_appe_tunnel_decap_counter_get;

	return SW_OK;
}

/**
 * @}
 */
