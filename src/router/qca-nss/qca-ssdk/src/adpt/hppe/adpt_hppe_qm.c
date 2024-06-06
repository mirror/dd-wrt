/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 *
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
#include "fal_qm.h"
#include "hppe_reg_access.h"
#include "hppe_qm_reg.h"
#include "hppe_qm.h"
#include "hppe_qos_reg.h"
#include "hppe_qos.h"
#include "hppe_portvlan_reg.h"
#include "hppe_portvlan.h"
#include "hppe_portctrl_reg.h"
#include "hppe_portctrl.h"
#include "adpt.h"
#include "adpt_hppe.h"
#if defined(APPE)
#include "adpt_appe_qm.h"
#endif
#include "hppe_global_reg.h"
#include "hppe_global.h"

#define SERVICE_CODE_QUEUE_OFFSET   2048
#define CPU_CODE_QUEUE_OFFSET         1024
#define VP_PORT_QUEUE_OFFSET            0

#define UCAST_QUEUE_ID_MAX	256
#define ALL_QUEUE_ID_MAX	300
#define MCAST_QUEUE_PORT7_START	296
#define MCAST_QUEUE_PORT6_START	292
#define MCAST_QUEUE_PORT5_START	288
#define MCAST_QUEUE_PORT4_START	284
#define MCAST_QUEUE_PORT3_START	280
#define MCAST_QUEUE_PORT2_START	276
#define MCAST_QUEUE_PORT1_START	272
#define MCAST_QUEUE_PORT0_START	256
#define MCAST_QUEUE_OFFSET	(3*0x10)
#define UCAST_QUEUE_ITEMS	6
#define MCAST_QUEUE_ITEMS	3
#define DROP_INC	0x10

sw_error_t
adpt_hppe_ucast_hash_map_set(
		a_uint32_t dev_id,
		a_uint8_t profile,
		a_uint8_t rss_hash,
		a_int8_t queue_hash)
{
	union ucast_hash_map_tbl_u ucast_hash_map_tbl;
	a_uint32_t index = 0;

	memset(&ucast_hash_map_tbl, 0, sizeof(ucast_hash_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);

	index = profile << 8 | rss_hash;
	ucast_hash_map_tbl.bf.hash = queue_hash;
	
	return hppe_ucast_hash_map_tbl_set(dev_id, index, &ucast_hash_map_tbl);
}

sw_error_t
adpt_hppe_ac_dynamic_threshold_get(
		a_uint32_t dev_id,
		a_uint32_t queue_id,
		fal_ac_dynamic_threshold_t *cfg)
{
	sw_error_t rv = SW_OK;
	union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;

	memset(&ac_uni_queue_cfg_tbl, 0, sizeof(ac_uni_queue_cfg_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	rv = hppe_ac_uni_queue_cfg_tbl_get(dev_id, queue_id, &ac_uni_queue_cfg_tbl);
	if( rv != SW_OK )
		return rv;

	cfg->wred_enable = ac_uni_queue_cfg_tbl.bf.ac_cfg_wred_en;
	cfg->color_enable = ac_uni_queue_cfg_tbl.bf.ac_cfg_color_aware;
	cfg->shared_weight = ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_weight;
	cfg->green_min_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_grn_min;
	cfg->yel_max_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_max;
	cfg->yel_min_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_0 | \
					ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_1 << 10;
	cfg->red_max_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_max;
	cfg->red_min_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_min;
	cfg->green_resume_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_grn_resume_offset;
	cfg->yel_resume_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset;
	cfg->red_resume_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_0 | \
					ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_1 << 9;
	cfg->ceiling = ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_ceiling;
	cfg->status = ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_dynamic;

	return SW_OK;
}

sw_error_t
adpt_hppe_ucast_queue_base_profile_get(
		a_uint32_t dev_id,
		fal_ucast_queue_dest_t *queue_dest,
		a_uint32_t *queue_base, a_uint8_t *profile)
{
	sw_error_t rv = SW_OK;
	union ucast_queue_map_tbl_u ucast_queue_map_tbl;
	a_uint32_t index = 0;

	memset(&ucast_queue_map_tbl, 0, sizeof(ucast_queue_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_dest);
	ADPT_NULL_POINT_CHECK(queue_base);
	ADPT_NULL_POINT_CHECK(profile);

	if (queue_dest ->service_code_en) {
		if (queue_dest->service_code >= SSDK_MAX_SERVICE_CODE_NUM)
			return SW_OUT_OF_RANGE;

		index = SERVICE_CODE_QUEUE_OFFSET + (queue_dest->src_profile << 8) \
				+ queue_dest->service_code;
	} else if (queue_dest ->cpu_code_en) {
		if (queue_dest->cpu_code >= SSDK_MAX_CPU_CODE_NUM)
			return SW_OUT_OF_RANGE;

		index = CPU_CODE_QUEUE_OFFSET + (queue_dest->src_profile << 8) \
				+ queue_dest->cpu_code;
	} else {
		index = VP_PORT_QUEUE_OFFSET + (queue_dest->src_profile << 8) \
				+ FAL_PORT_ID_VALUE(queue_dest->dst_port);
	}

	rv = hppe_ucast_queue_map_tbl_get(dev_id, index, &ucast_queue_map_tbl);
	if (rv)
		return rv;
	*queue_base = ucast_queue_map_tbl.bf.queue_id;
	*profile = ucast_queue_map_tbl.bf.profile_id;
	
	return SW_OK;
}

#if !defined(IN_QM_MINI)
sw_error_t
adpt_hppe_port_mcast_priority_class_get(
		a_uint32_t dev_id,
		fal_port_t port,
		a_uint8_t priority,
		a_uint8_t *queue_class)
{
	sw_error_t rv = SW_OK;
	union mcast_priority_map0_u mcast_priority_map0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_class);
	port = FAL_PORT_ID_VALUE(port);


	if (port == 0){
		rv = hppe_mcast_priority_map0_get(dev_id, priority, &mcast_priority_map0);
	} else if (port == 1) {
		rv = hppe_mcast_priority_map1_get(dev_id, priority,
					(union mcast_priority_map1_u *)&mcast_priority_map0);
	} else if (port == 2) {
		rv = hppe_mcast_priority_map2_get(dev_id, priority,
					(union mcast_priority_map2_u *)&mcast_priority_map0);
	} else if (port == 3) {
		rv = hppe_mcast_priority_map3_get(dev_id, priority,
					(union mcast_priority_map3_u *)&mcast_priority_map0);
	} else if (port == 4) {
		rv = hppe_mcast_priority_map4_get(dev_id, priority,
					(union mcast_priority_map4_u *)&mcast_priority_map0);
	} else if (port == 5) {
		rv = hppe_mcast_priority_map5_get(dev_id, priority,
					(union mcast_priority_map5_u *)&mcast_priority_map0);
	} else if (port == 6) {
		rv = hppe_mcast_priority_map6_get(dev_id, priority,
					(union mcast_priority_map6_u *)&mcast_priority_map0);
	} else if (port == 7) {
		rv = hppe_mcast_priority_map7_get(dev_id, priority,
					(union mcast_priority_map7_u *)&mcast_priority_map0);
	}

	if( rv != SW_OK )
		return rv;

	*queue_class = mcast_priority_map0.bf.class;
	return SW_OK;
}
#endif

sw_error_t
adpt_hppe_ac_dynamic_threshold_set(
		a_uint32_t dev_id,
		a_uint32_t queue_id,
		fal_ac_dynamic_threshold_t *cfg)
{
	sw_error_t rv = SW_OK;
	union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;

	memset(&ac_uni_queue_cfg_tbl, 0, sizeof(ac_uni_queue_cfg_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	rv = hppe_ac_uni_queue_cfg_tbl_get(dev_id, queue_id, &ac_uni_queue_cfg_tbl);
	if( rv != SW_OK )
		return rv;

	ac_uni_queue_cfg_tbl.bf.ac_cfg_wred_en = cfg->wred_enable;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_color_aware = cfg->color_enable;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_dynamic = 1;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_grn_min = cfg->green_min_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_max = cfg->yel_max_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_0 = cfg->yel_min_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_1 = cfg->yel_min_off >> 20;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_max = cfg->red_max_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_min = cfg->red_min_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_grn_resume_offset = cfg->green_resume_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset = cfg->yel_resume_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_0 = cfg->red_resume_off;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_1 = cfg->red_resume_off >> 9;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_weight = cfg->shared_weight;
	ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_ceiling = cfg->ceiling;

	return hppe_ac_uni_queue_cfg_tbl_set(dev_id, queue_id, &ac_uni_queue_cfg_tbl);
}

sw_error_t
adpt_hppe_ac_prealloc_buffer_set(
		a_uint32_t dev_id,
		fal_ac_obj_t *obj,
		a_uint16_t num)
{
	ADPT_DEV_ID_CHECK(dev_id);

	if (obj->type == FAL_AC_GROUP) {
		union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

		memset(&ac_grp_cfg_tbl, 0, sizeof(ac_grp_cfg_tbl));
		hppe_ac_grp_cfg_tbl_get(dev_id, obj->obj_id, &ac_grp_cfg_tbl);

		ac_grp_cfg_tbl.bf.ac_grp_palloc_limit = num;

		return hppe_ac_grp_cfg_tbl_set(dev_id, obj->obj_id, &ac_grp_cfg_tbl);
		
	} else if (obj->type == FAL_AC_QUEUE) {
		if (obj->obj_id < UCAST_QUEUE_ID_MAX) {
			union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
			hppe_ac_uni_queue_cfg_tbl_get(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);
			ac_uni_queue_cfg_tbl.bf.ac_cfg_pre_alloc_limit = num;
			return hppe_ac_uni_queue_cfg_tbl_set(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);;
			
		} else {
			union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
			hppe_ac_mul_queue_cfg_tbl_get(dev_id,
					obj->obj_id - UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
			ac_mul_queue_cfg_tbl.bf.ac_cfg_pre_alloc_limit = num;
			return hppe_ac_mul_queue_cfg_tbl_set(dev_id,
					obj->obj_id -UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
		}
	} else
		return SW_FAIL;
}

#if !defined(IN_QM_MINI)
sw_error_t
adpt_hppe_ucast_default_hash_get(
		a_uint32_t dev_id,
		a_uint8_t *hash_value)
{
	sw_error_t rv = SW_OK;
	union ucast_default_hash_u ucast_default_hash;

	memset(&ucast_default_hash, 0, sizeof(ucast_default_hash));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(hash_value);

	rv = hppe_ucast_default_hash_get(dev_id, &ucast_default_hash);
	if( rv != SW_OK )
		return rv;

	*hash_value = ucast_default_hash.bf.hash;
	return SW_OK;
}

sw_error_t
adpt_hppe_ucast_default_hash_set(
		a_uint32_t dev_id,
		a_uint8_t hash_value)
{
	sw_error_t rv = SW_OK;
	union ucast_default_hash_u ucast_default_hash;

	memset(&ucast_default_hash, 0, sizeof(ucast_default_hash));
	ADPT_DEV_ID_CHECK(dev_id);

	ucast_default_hash.bf.hash = hash_value;
	rv = hppe_ucast_default_hash_set(dev_id, &ucast_default_hash);
	
	return rv;
}
#endif

sw_error_t
adpt_hppe_ac_queue_group_get(
		a_uint32_t dev_id,
		a_uint32_t queue_id,
		a_uint8_t *group_id)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(group_id);


	if (queue_id < UCAST_QUEUE_ID_MAX) {
		union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
		rv = hppe_ac_uni_queue_cfg_tbl_get(dev_id,
				queue_id,
				&ac_uni_queue_cfg_tbl);
		*group_id = ac_uni_queue_cfg_tbl.bf.ac_cfg_grp_id;
		
	} else {
		union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
		rv = hppe_ac_mul_queue_cfg_tbl_get(dev_id,
				queue_id - UCAST_QUEUE_ID_MAX,
				&ac_mul_queue_cfg_tbl);
		*group_id = ac_mul_queue_cfg_tbl.bf.ac_cfg_grp_id;
	}

	return rv;
}

sw_error_t
adpt_hppe_ac_ctrl_get(
		a_uint32_t dev_id,
		fal_ac_obj_t *obj,
		fal_ac_ctrl_t *cfg)
{
	sw_error_t rv = SW_OK;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	if (obj->type == FAL_AC_GROUP) {
		union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

		memset(&ac_grp_cfg_tbl, 0, sizeof(ac_grp_cfg_tbl));
		rv = hppe_ac_grp_cfg_tbl_get(dev_id, obj->obj_id, &ac_grp_cfg_tbl);

		cfg->ac_en = ac_grp_cfg_tbl.bf.ac_cfg_ac_en;
		cfg->ac_fc_en = ac_grp_cfg_tbl.bf.ac_cfg_force_ac_en;
	} else if (obj->type == FAL_AC_QUEUE) {
		if (obj->obj_id < UCAST_QUEUE_ID_MAX) {
			union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
			rv = hppe_ac_uni_queue_cfg_tbl_get(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);
			cfg->ac_en = ac_uni_queue_cfg_tbl.bf.ac_cfg_ac_en;
			cfg->ac_fc_en = ac_uni_queue_cfg_tbl.bf.ac_cfg_force_ac_en;
			
		} else {
			union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
			rv = hppe_ac_mul_queue_cfg_tbl_get(dev_id,
					obj->obj_id - UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
			cfg->ac_en = ac_mul_queue_cfg_tbl.bf.ac_cfg_ac_en;
			cfg->ac_fc_en = ac_mul_queue_cfg_tbl.bf.ac_cfg_force_ac_en;
		}
	} else
		return SW_FAIL;

	return rv;
}

sw_error_t
adpt_hppe_ac_prealloc_buffer_get(
		a_uint32_t dev_id,
		fal_ac_obj_t *obj,
		a_uint16_t *num)
{
	sw_error_t rv = SW_OK;
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(num);

	if (obj->type == FAL_AC_GROUP) {
		union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

		rv = hppe_ac_grp_cfg_tbl_get(dev_id, obj->obj_id, &ac_grp_cfg_tbl);

		*num = ac_grp_cfg_tbl.bf.ac_grp_palloc_limit;

		return rv;
		
	} else if (obj->type == FAL_AC_QUEUE) {
		if (obj->obj_id < UCAST_QUEUE_ID_MAX) {
			union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
			rv = hppe_ac_uni_queue_cfg_tbl_get(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);
			*num = ac_uni_queue_cfg_tbl.bf.ac_cfg_pre_alloc_limit;
		} else {
			union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
			rv = hppe_ac_mul_queue_cfg_tbl_get(dev_id,
					obj->obj_id - UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
			*num = ac_mul_queue_cfg_tbl.bf.ac_cfg_pre_alloc_limit;
		}
		return rv;
	} else
		return SW_FAIL;
}

#if !defined(IN_QM_MINI)
sw_error_t
adpt_hppe_port_mcast_priority_class_set(
		a_uint32_t dev_id,
		fal_port_t port,
		a_uint8_t priority,
		a_uint8_t queue_class)
{
	sw_error_t rv = SW_OK;
	union mcast_priority_map0_u mcast_priority_map0;

	ADPT_DEV_ID_CHECK(dev_id);
	port = FAL_PORT_ID_VALUE(port);

	mcast_priority_map0.bf.class = queue_class;
	
	if (port == 0){
		rv = hppe_mcast_priority_map0_set(dev_id, priority, &mcast_priority_map0);
	} else if (port == 1) {
		rv = hppe_mcast_priority_map1_set(dev_id, priority,
					(union mcast_priority_map1_u *)&mcast_priority_map0);
	} else if (port == 2) {
		rv = hppe_mcast_priority_map2_set(dev_id, priority,
					(union mcast_priority_map2_u *)&mcast_priority_map0);
	} else if (port == 3) {
		rv = hppe_mcast_priority_map3_set(dev_id, priority,
					(union mcast_priority_map3_u *)&mcast_priority_map0);
	} else if (port == 4) {
		rv = hppe_mcast_priority_map4_set(dev_id, priority,
					(union mcast_priority_map4_u *)&mcast_priority_map0);
	} else if (port == 5) {
		rv = hppe_mcast_priority_map5_set(dev_id, priority,
					(union mcast_priority_map5_u *)&mcast_priority_map0);
	} else if (port == 6) {
		rv = hppe_mcast_priority_map6_set(dev_id, priority,
					(union mcast_priority_map6_u *)&mcast_priority_map0);
	} else if (port == 7) {
		rv = hppe_mcast_priority_map7_set(dev_id, priority,
					(union mcast_priority_map7_u *)&mcast_priority_map0);
	}

	return rv;
}
#endif

sw_error_t
adpt_hppe_ucast_hash_map_get(
		a_uint32_t dev_id,
		a_uint8_t profile,
		a_uint8_t rss_hash,
		a_int8_t *queue_hash)
{
	union ucast_hash_map_tbl_u ucast_hash_map_tbl;
	sw_error_t rv = SW_OK;
	a_uint32_t index = 0;

	memset(&ucast_hash_map_tbl, 0, sizeof(ucast_hash_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);

	index = profile << 8 | rss_hash;
	
	rv = hppe_ucast_hash_map_tbl_get(dev_id, index, &ucast_hash_map_tbl);
	if (rv)
		return rv;

	*queue_hash = ucast_hash_map_tbl.bf.hash;
	return rv;
}

sw_error_t
adpt_hppe_ac_static_threshold_set(
		a_uint32_t dev_id,
		fal_ac_obj_t *obj,
		fal_ac_static_threshold_t *cfg)
{
	ADPT_DEV_ID_CHECK(dev_id);

	if (obj->type == FAL_AC_GROUP) {
		union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

		hppe_ac_grp_cfg_tbl_get(dev_id, obj->obj_id, &ac_grp_cfg_tbl);

		ac_grp_cfg_tbl.bf.ac_cfg_color_aware = cfg->color_enable;
		ac_grp_cfg_tbl.bf.ac_grp_dp_thrd_0 = cfg->green_max;
		ac_grp_cfg_tbl.bf.ac_grp_dp_thrd_1 = cfg->green_max >> 7;
		ac_grp_cfg_tbl.bf.ac_grp_gap_grn_yel = cfg->yel_max_off;
		ac_grp_cfg_tbl.bf.ac_grp_gap_grn_red = cfg->red_max_off;
		ac_grp_cfg_tbl.bf.ac_grp_grn_resume_offset = cfg->green_resume_off;
		ac_grp_cfg_tbl.bf.ac_grp_yel_resume_offset_0 = cfg->yel_resume_off;
		ac_grp_cfg_tbl.bf.ac_grp_yel_resume_offset_1 = cfg->yel_resume_off >> 6;
		ac_grp_cfg_tbl.bf.ac_grp_red_resume_offset = cfg->red_resume_off;

		return hppe_ac_grp_cfg_tbl_set(dev_id, obj->obj_id, &ac_grp_cfg_tbl);
		
	} else if (obj->type == FAL_AC_QUEUE) {
		if (obj->obj_id < UCAST_QUEUE_ID_MAX) {
			union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
			hppe_ac_uni_queue_cfg_tbl_get(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);
			ac_uni_queue_cfg_tbl.bf.ac_cfg_wred_en = cfg->wred_enable;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_color_aware = cfg->color_enable;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_dynamic = 0;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_grn_min = cfg->green_min_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_max = cfg->yel_max_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_0 = cfg->yel_min_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_1 = cfg->yel_min_off >> 10;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_max = cfg->red_max_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_min = cfg->red_min_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_grn_resume_offset = cfg->green_resume_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset = cfg->yel_resume_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_0 = cfg->red_resume_off;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_1 = cfg->red_resume_off >> 9;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_ceiling = cfg->green_max;
			return hppe_ac_uni_queue_cfg_tbl_set(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);;
			
		} else {
			union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
			hppe_ac_mul_queue_cfg_tbl_get(dev_id,
					obj->obj_id - UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
			ac_mul_queue_cfg_tbl.bf.ac_cfg_color_aware = cfg->color_enable;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_grn_resume_offset = cfg->green_resume_off;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset_0 = cfg->yel_resume_off;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset_1 = cfg->yel_resume_off >> 4;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_red_resume_offset = cfg->red_resume_off;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_gap_grn_red = cfg->red_max_off;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_0 = cfg->yel_max_off;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_1 = cfg->yel_max_off >> 5;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_shared_ceiling = cfg->green_max;
			return hppe_ac_mul_queue_cfg_tbl_set(dev_id,
					obj->obj_id -UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
		}
	} else
		return SW_FAIL;
}

sw_error_t
adpt_hppe_ac_queue_group_set(
		a_uint32_t dev_id,
		a_uint32_t queue_id,
		a_uint8_t group_id)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);


	if (queue_id < UCAST_QUEUE_ID_MAX) {
		union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
		hppe_ac_uni_queue_cfg_tbl_get(dev_id,
				queue_id,
				&ac_uni_queue_cfg_tbl);
		ac_uni_queue_cfg_tbl.bf.ac_cfg_grp_id = group_id;
		return hppe_ac_uni_queue_cfg_tbl_set(dev_id,
				queue_id,
				&ac_uni_queue_cfg_tbl);
		
	} else {
		union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
		hppe_ac_mul_queue_cfg_tbl_get(dev_id,
				queue_id - UCAST_QUEUE_ID_MAX,
				&ac_mul_queue_cfg_tbl);
		ac_mul_queue_cfg_tbl.bf.ac_cfg_grp_id = group_id;
		return hppe_ac_mul_queue_cfg_tbl_set(dev_id,
				queue_id - UCAST_QUEUE_ID_MAX,
				&ac_mul_queue_cfg_tbl);
	}

	return rv;
}

sw_error_t
adpt_hppe_ac_group_buffer_get(
		a_uint32_t dev_id,
		a_uint8_t group_id,
		fal_ac_group_buffer_t *cfg)
{
	sw_error_t rv = SW_OK;
	union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

	memset(&ac_grp_cfg_tbl, 0, sizeof(ac_grp_cfg_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	rv = hppe_ac_grp_cfg_tbl_get(dev_id, group_id, &ac_grp_cfg_tbl);
	if( rv != SW_OK )
		return rv;

	cfg->prealloc_buffer = ac_grp_cfg_tbl.bf.ac_grp_palloc_limit;
	cfg->total_buffer = ac_grp_cfg_tbl.bf.ac_grp_limit;

	return SW_OK;
}

#if !defined(IN_QM_MINI)
sw_error_t
adpt_hppe_mcast_cpu_code_class_get(
		a_uint32_t dev_id,
		a_uint8_t cpu_code,
		a_uint8_t *queue_class)
{
	sw_error_t rv = SW_OK;
	union mcast_queue_map_tbl_u mcast_queue_map_tbl;

	memset(&mcast_queue_map_tbl, 0, sizeof(mcast_queue_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_class);

	rv = hppe_mcast_queue_map_tbl_get(dev_id, cpu_code, &mcast_queue_map_tbl);
	if( rv != SW_OK )
		return rv;

	*queue_class = mcast_queue_map_tbl.bf.class;

	return SW_OK;
}
#endif

sw_error_t
adpt_hppe_ac_ctrl_set(
		a_uint32_t dev_id,
		fal_ac_obj_t *obj,
		fal_ac_ctrl_t *cfg)
{
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	if (obj->type == FAL_AC_GROUP) {
		union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

		memset(&ac_grp_cfg_tbl, 0, sizeof(ac_grp_cfg_tbl));
		hppe_ac_grp_cfg_tbl_get(dev_id, obj->obj_id, &ac_grp_cfg_tbl);

		ac_grp_cfg_tbl.bf.ac_cfg_ac_en = cfg->ac_en;
		ac_grp_cfg_tbl.bf.ac_cfg_force_ac_en = cfg->ac_fc_en;
		return hppe_ac_grp_cfg_tbl_set(dev_id, obj->obj_id, &ac_grp_cfg_tbl);
	} else if (obj->type == FAL_AC_QUEUE) {
		if (obj->obj_id < UCAST_QUEUE_ID_MAX) {
			union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
			hppe_ac_uni_queue_cfg_tbl_get(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);
			ac_uni_queue_cfg_tbl.bf.ac_cfg_ac_en = cfg->ac_en;
			ac_uni_queue_cfg_tbl.bf.ac_cfg_force_ac_en = cfg->ac_fc_en;
			return hppe_ac_uni_queue_cfg_tbl_set(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);
			
		} else {
			union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
			hppe_ac_mul_queue_cfg_tbl_get(dev_id,
					obj->obj_id - UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
			ac_mul_queue_cfg_tbl.bf.ac_cfg_ac_en = cfg->ac_en;
			ac_mul_queue_cfg_tbl.bf.ac_cfg_force_ac_en = cfg->ac_fc_en;
			return hppe_ac_mul_queue_cfg_tbl_set(dev_id,
					obj->obj_id - UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
		}
	} else
		return SW_FAIL;

}

sw_error_t
adpt_hppe_ucast_priority_class_get(
		a_uint32_t dev_id,
		a_uint8_t profile,
		a_uint8_t priority,
		a_uint8_t *class)
{
	sw_error_t rv = SW_OK;
	union ucast_priority_map_tbl_u ucast_priority_map_tbl;
	a_uint32_t index = 0;

	memset(&ucast_priority_map_tbl, 0, sizeof(ucast_priority_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(class);

	index = profile << 4 | priority;
	rv = hppe_ucast_priority_map_tbl_get(dev_id, index, &ucast_priority_map_tbl);

	if( rv != SW_OK )
		return rv;

	*class = ucast_priority_map_tbl.bf.class;
	return rv;
}

sw_error_t
adpt_hppe_qm_enqueue_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t queue_id,
		a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	union oq_enq_opr_tbl_u enq;

	ADPT_DEV_ID_CHECK(dev_id);
	memset(&enq, 0, sizeof(enq));

	rv = hppe_oq_enq_opr_tbl_get(dev_id, queue_id, &enq);
	if( rv != SW_OK )
		return rv;

	*enable = !(enq.bf.enq_disable);

	return SW_OK;
}

sw_error_t
adpt_hppe_queue_flush(
		a_uint32_t dev_id,
		fal_port_t port,
		a_uint16_t queue_id)
{
#define PPE_QUEUE_FLUSH_CHECK_ROUND_MAX		0X2000
	union flush_cfg_u flush_cfg;
	union clk_gating_ctrl_u clk_gate_ctrl;
	a_uint32_t round = PPE_QUEUE_FLUSH_CHECK_ROUND_MAX, qm_clk_gate_bak = 0;
	sw_error_t rv;
	a_bool_t enable = A_TRUE;

	ADPT_DEV_ID_CHECK(dev_id);
	memset(&flush_cfg, 0, sizeof(flush_cfg));
	memset(&clk_gate_ctrl, 0, sizeof(clk_gate_ctrl));

	#if 0
	/* disable queue firstly */
	if (queue_id == 0xffff) {
		/* all queue in this port */
		a_uint32_t i, j, k, tmp;
		enq.bf.enq_disable = 1;
		deq.bf.deq_dis = 1;
		p_api = adpt_api_ptr_get(0);
		if (!p_api || !p_api->adpt_port_queues_get)
			return SW_FAIL;
		p_api->adpt_port_queues_get(dev_id, port, &queue_bmp);
		for (i = 0; i < ALL_QUEUE_ID_MAX; i++) {
			j = i / 32;
			k = i % 32;
			tmp = queue_bmp.bmp[j];
			if ((tmp & (1 << k)) == 0)
				continue;
			hppe_oq_enq_opr_tbl_set(dev_id, i, &enq);
			hppe_deq_dis_tbl_set(dev_id, i, &deq);
		}
	} else {
		/* single queue in this port */
		enq.bf.enq_disable = 1;
		deq.bf.deq_dis = 1;
		if (queue_id >= ALL_QUEUE_ID_MAX)
			return SW_BAD_VALUE;
		hppe_oq_enq_opr_tbl_set(dev_id, queue_id, &enq);
		hppe_deq_dis_tbl_set(dev_id, queue_id, &deq);
	}
	#endif

	if (queue_id < ALL_QUEUE_ID_MAX) {
		rv = adpt_hppe_qm_enqueue_ctrl_get(dev_id, queue_id, &enable);
		SW_RTN_ON_ERROR(rv);

		if (enable) {
			SSDK_ERROR("queue %d should be disabled before flushing queue\n",
					queue_id);
			return SW_NOT_READY;
		}
	}

	hppe_flush_cfg_get(dev_id, &flush_cfg);

	if (queue_id == 0xffff) {
		flush_cfg.bf.flush_all_queues = 1;
		flush_cfg.bf.flush_qid = 0;
	} else {
		flush_cfg.bf.flush_all_queues = 0;
		flush_cfg.bf.flush_qid = queue_id;
	}
	flush_cfg.bf.flush_dst_port = FAL_PORT_ID_VALUE(port);
	flush_cfg.bf.flush_busy = 1;
	
	hppe_clk_gating_ctrl_get(dev_id, &clk_gate_ctrl);
	qm_clk_gate_bak = clk_gate_ctrl.bf.qm_clk_gate_en;

	/* Disable QM clock gate for accelerating queue flush */
	clk_gate_ctrl.bf.qm_clk_gate_en = A_FALSE;
	hppe_clk_gating_ctrl_set(dev_id, &clk_gate_ctrl);

	hppe_flush_cfg_set(dev_id, &flush_cfg);
	while (flush_cfg.bf.flush_busy && --round) {
		hppe_flush_cfg_get(dev_id, &flush_cfg);
	}

	/* Restore QM clock gate */
	clk_gate_ctrl.bf.qm_clk_gate_en = qm_clk_gate_bak;
	hppe_clk_gating_ctrl_set(dev_id, &clk_gate_ctrl);

	if (round == 0) {
		SSDK_ERROR("queue %d flush busily\n", queue_id);
		return SW_BUSY;
	}

	if (!flush_cfg.bf.flush_status) {
		SSDK_ERROR("queue %d flush fail %d\n", queue_id, flush_cfg.bf.flush_status);
		return SW_FAIL;
	}

	SSDK_DEBUG("queue %d flush with round %d successfully\n", queue_id,
			PPE_QUEUE_FLUSH_CHECK_ROUND_MAX - round);

	#if 0
	/* enable queue again */
	if (queue_id == 0xffff) {
		/* all queue in this port */
		a_uint32_t i, j, k, tmp;
		enq.bf.enq_disable = 0;
		deq.bf.deq_dis = 0;
		for (i = 0; i < ALL_QUEUE_ID_MAX; i++) {
			j = i / 32;
			k = i % 32;
			tmp = queue_bmp.bmp[j];
			if ((tmp & (1 << k)) == 0)
				continue;
			hppe_oq_enq_opr_tbl_set(dev_id, i, &enq);
			hppe_deq_dis_tbl_set(dev_id, i, &deq);
		}
	} else {
		/* single queue in this port */
		enq.bf.enq_disable = 0;
		deq.bf.deq_dis = 0;
		hppe_oq_enq_opr_tbl_set(dev_id, queue_id, &enq);
		hppe_deq_dis_tbl_set(dev_id, queue_id, &deq);
	}
	#endif
	return SW_OK;
}

#if !defined(IN_QM_MINI)
sw_error_t
adpt_hppe_mcast_cpu_code_class_set(
		a_uint32_t dev_id,
		a_uint8_t cpu_code,
		a_uint8_t queue_class)
{
	union mcast_queue_map_tbl_u mcast_queue_map_tbl;

	memset(&mcast_queue_map_tbl, 0, sizeof(mcast_queue_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);

	mcast_queue_map_tbl.bf.class = queue_class;
	return hppe_mcast_queue_map_tbl_set(dev_id, cpu_code, &mcast_queue_map_tbl);
}
#endif

sw_error_t
adpt_hppe_ucast_priority_class_set(
		a_uint32_t dev_id,
		a_uint8_t profile,
		a_uint8_t priority,
		a_uint8_t class)
{
	union ucast_priority_map_tbl_u ucast_priority_map_tbl;
	a_int32_t index = 0;

	memset(&ucast_priority_map_tbl, 0, sizeof(ucast_priority_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);

	index = profile << 4 | priority;
	ucast_priority_map_tbl.bf.class = class;
	
	return hppe_ucast_priority_map_tbl_set(dev_id, index, &ucast_priority_map_tbl);
}

sw_error_t
adpt_hppe_ac_static_threshold_get(
		a_uint32_t dev_id,
		fal_ac_obj_t *obj,
		fal_ac_static_threshold_t *cfg)
{
	sw_error_t rv;
	ADPT_DEV_ID_CHECK(dev_id);

	cfg->status = A_TRUE;

	if (obj->type == FAL_AC_GROUP) {
		union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

		rv = hppe_ac_grp_cfg_tbl_get(dev_id, obj->obj_id, &ac_grp_cfg_tbl);

		cfg->color_enable = ac_grp_cfg_tbl.bf.ac_cfg_color_aware;
		cfg->green_max = ac_grp_cfg_tbl.bf.ac_grp_dp_thrd_0 |
					ac_grp_cfg_tbl.bf.ac_grp_dp_thrd_1 << 7;
		cfg->yel_max_off = ac_grp_cfg_tbl.bf.ac_grp_gap_grn_yel;
		cfg->red_max_off = ac_grp_cfg_tbl.bf.ac_grp_gap_grn_red;
		cfg->green_resume_off = ac_grp_cfg_tbl.bf.ac_grp_grn_resume_offset;
		cfg->yel_resume_off = ac_grp_cfg_tbl.bf.ac_grp_yel_resume_offset_0 |
					ac_grp_cfg_tbl.bf.ac_grp_yel_resume_offset_1 << 6;
		cfg->red_resume_off = ac_grp_cfg_tbl.bf.ac_grp_red_resume_offset;

		return rv;
		
	} else if (obj->type == FAL_AC_QUEUE) {
		if (obj->obj_id < UCAST_QUEUE_ID_MAX) {
			union ac_uni_queue_cfg_tbl_u ac_uni_queue_cfg_tbl;
			rv = hppe_ac_uni_queue_cfg_tbl_get(dev_id,
					obj->obj_id,
					&ac_uni_queue_cfg_tbl);
			cfg->wred_enable = ac_uni_queue_cfg_tbl.bf.ac_cfg_wred_en;
			cfg->color_enable = ac_uni_queue_cfg_tbl.bf.ac_cfg_color_aware;
			cfg->green_min_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_grn_min;
			cfg->yel_max_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_max;
			cfg->yel_min_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_0 |
					ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_min_1 << 10;
			cfg->red_max_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_max;
			cfg->red_min_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_gap_grn_red_min;
			cfg->green_resume_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_grn_resume_offset;
			cfg->yel_resume_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset;
			cfg->red_resume_off = ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_0 |
					ac_uni_queue_cfg_tbl.bf.ac_cfg_red_resume_offset_1 << 9;
			cfg->green_max = ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_ceiling;
			cfg->status = !ac_uni_queue_cfg_tbl.bf.ac_cfg_shared_dynamic;
			return rv;
			
		} else {
			union ac_mul_queue_cfg_tbl_u ac_mul_queue_cfg_tbl;
			rv = hppe_ac_mul_queue_cfg_tbl_get(dev_id,
					obj->obj_id - UCAST_QUEUE_ID_MAX,
					&ac_mul_queue_cfg_tbl);
			cfg->color_enable = ac_mul_queue_cfg_tbl.bf.ac_cfg_color_aware;
			cfg->green_max = ac_mul_queue_cfg_tbl.bf.ac_cfg_shared_ceiling;
			cfg->red_max_off = ac_mul_queue_cfg_tbl.bf.ac_cfg_gap_grn_red;
			cfg->yel_max_off= ac_mul_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_0 |
						ac_mul_queue_cfg_tbl.bf.ac_cfg_gap_grn_yel_1 << 5;
			cfg->green_resume_off = ac_mul_queue_cfg_tbl.bf.ac_cfg_grn_resume_offset;
			cfg->yel_resume_off = ac_mul_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset_0 |
					ac_mul_queue_cfg_tbl.bf.ac_cfg_yel_resume_offset_1 << 4;
			cfg->red_resume_off = ac_mul_queue_cfg_tbl.bf.ac_cfg_red_resume_offset;

			return rv;
		}
	} else
		return SW_FAIL;
}

sw_error_t
adpt_hppe_ucast_queue_base_profile_set(
		a_uint32_t dev_id,
		fal_ucast_queue_dest_t *queue_dest,
		a_uint32_t queue_base, a_uint8_t profile)
{
	union ucast_queue_map_tbl_u ucast_queue_map_tbl;
	a_uint32_t index = 0;

	memset(&ucast_queue_map_tbl, 0, sizeof(ucast_queue_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_dest);

	if (queue_dest ->service_code_en) {
		if (queue_dest->service_code >= SSDK_MAX_SERVICE_CODE_NUM)
			return SW_OUT_OF_RANGE;

		index = SERVICE_CODE_QUEUE_OFFSET + (queue_dest->src_profile << 8) \
				+ queue_dest->service_code;
	} else if (queue_dest ->cpu_code_en) {
		if (queue_dest->cpu_code >= SSDK_MAX_CPU_CODE_NUM)
			return SW_OUT_OF_RANGE;

		index = CPU_CODE_QUEUE_OFFSET + (queue_dest->src_profile << 8) \
				+ queue_dest->cpu_code;
	} else {
		index = VP_PORT_QUEUE_OFFSET + (queue_dest->src_profile << 8) \
				+ FAL_PORT_ID_VALUE(queue_dest->dst_port);
	}

	ucast_queue_map_tbl.bf.queue_id = queue_base;
	ucast_queue_map_tbl.bf.profile_id = profile;
	
	return hppe_ucast_queue_map_tbl_set(dev_id, index, &ucast_queue_map_tbl);
}

sw_error_t
adpt_hppe_ac_group_buffer_set(
		a_uint32_t dev_id,
		a_uint8_t group_id,
		fal_ac_group_buffer_t *cfg)
{
	sw_error_t rv = SW_OK;
	union ac_grp_cfg_tbl_u ac_grp_cfg_tbl;

	memset(&ac_grp_cfg_tbl, 0, sizeof(ac_grp_cfg_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	rv = hppe_ac_grp_cfg_tbl_get(dev_id, group_id, &ac_grp_cfg_tbl);
	if( rv != SW_OK )
		return rv;

	ac_grp_cfg_tbl.bf.ac_grp_palloc_limit = cfg->prealloc_buffer;
	ac_grp_cfg_tbl.bf.ac_grp_limit = cfg->total_buffer;

	return hppe_ac_grp_cfg_tbl_set(dev_id, group_id, &ac_grp_cfg_tbl);;
}

static a_uint32_t
adpt_hppe_mcast_queue_dropcnt_start_addr_get(a_uint32_t queue_id)
{
	a_uint32_t start_addr = QUEUE_MANAGER_BASE_ADDR;

	if (queue_id >= MCAST_QUEUE_PORT7_START) {
		start_addr += MUL_P7_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT7_START)*MCAST_QUEUE_OFFSET;
	} else if (queue_id >= MCAST_QUEUE_PORT6_START) {
		start_addr += MUL_P6_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT6_START)*MCAST_QUEUE_OFFSET;
	} else if (queue_id >= MCAST_QUEUE_PORT5_START) {
		start_addr += MUL_P5_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT5_START)*MCAST_QUEUE_OFFSET;
	} else if (queue_id >= MCAST_QUEUE_PORT4_START) {
		start_addr += MUL_P4_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT4_START)*MCAST_QUEUE_OFFSET;
	} else if (queue_id >= MCAST_QUEUE_PORT3_START) {
		start_addr += MUL_P3_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT3_START)*MCAST_QUEUE_OFFSET;
	} else if (queue_id >= MCAST_QUEUE_PORT2_START) {
		start_addr += MUL_P2_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT2_START)*MCAST_QUEUE_OFFSET;
	} else if (queue_id >= MCAST_QUEUE_PORT1_START) {
		start_addr += MUL_P1_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT1_START)*MCAST_QUEUE_OFFSET;
	}  else if (queue_id >= MCAST_QUEUE_PORT0_START) {
		start_addr += MUL_P0_DROP_CNT_TBL_ADDRESS;
		start_addr += (queue_id - MCAST_QUEUE_PORT0_START)*MCAST_QUEUE_OFFSET;
	}

	return start_addr;
}

sw_error_t
adpt_hppe_queue_counter_cleanup(a_uint32_t dev_id, a_uint32_t queue_id)
{
	union queue_tx_counter_tbl_u tx_cnt;
	a_uint32_t i = 0;
	a_uint32_t val[3] = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if (queue_id >= ALL_QUEUE_ID_MAX)
		return SW_BAD_VALUE;

	tx_cnt.bf.tx_packets = 0;
	tx_cnt.bf.tx_bytes_0 = 0;
	tx_cnt.bf.tx_bytes_1 = 0;

	hppe_queue_tx_counter_tbl_set(dev_id, queue_id, &tx_cnt);

	if (queue_id >= UCAST_QUEUE_ID_MAX) {
		a_uint32_t start_addr = 0;

		start_addr = adpt_hppe_mcast_queue_dropcnt_start_addr_get(queue_id);
		for (i = 0; i < MCAST_QUEUE_ITEMS; i++) {
			hppe_reg_tbl_set(dev_id, start_addr + i*DROP_INC, val, 3);
		}
	} else {
		union uni_drop_cnt_tbl_u uni_drop_cnt;

		memset(&uni_drop_cnt, 0, sizeof(uni_drop_cnt));
		for (i = 0; i < UCAST_QUEUE_ITEMS; i++) {
			hppe_uni_drop_cnt_tbl_set(dev_id, queue_id*UCAST_QUEUE_ITEMS+i, &uni_drop_cnt);
		}
	}

	return SW_OK;
}
sw_error_t
adpt_hppe_queue_counter_get(a_uint32_t dev_id, a_uint32_t queue_id, fal_queue_stats_t *info)
{
	sw_error_t rv = SW_OK;
	union queue_tx_counter_tbl_u tx_cnt;
	union ac_mul_queue_cnt_tbl_u mul_cnt;
	union ac_uni_queue_cnt_tbl_u uni_cnt;
	a_uint32_t i = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(info);

	if (queue_id >= ALL_QUEUE_ID_MAX)
		return SW_BAD_VALUE;

	rv = hppe_queue_tx_counter_tbl_get(dev_id, queue_id, &tx_cnt);
	if( rv != SW_OK )
		return rv;
	if (queue_id >= UCAST_QUEUE_ID_MAX) {
		a_uint32_t start_addr = 0;
		union mul_p7_drop_cnt_tbl_u drop_cnt;

		rv = hppe_ac_mul_queue_cnt_tbl_get(dev_id,
				queue_id - UCAST_QUEUE_ID_MAX, &mul_cnt);
		if( rv != SW_OK )
			return rv;
		info->pending_buff_num = mul_cnt.bf.ac_mul_queue_cnt;
		start_addr = adpt_hppe_mcast_queue_dropcnt_start_addr_get(queue_id);
		for (i = 0; i < MCAST_QUEUE_ITEMS; i++) {
			hppe_reg_tbl_get(dev_id, start_addr + i*DROP_INC, drop_cnt.val, 3);
			info->drop_packets[i+3] = drop_cnt.bf.mul_p7_drop_pkt;
			info->drop_bytes[i+3] = (a_uint64_t)drop_cnt.bf.mul_p7_drop_byte_0 |
					(a_uint64_t)drop_cnt.bf.mul_p7_drop_byte_1 <<32;
		}
	} else {
		union uni_drop_cnt_tbl_u uni_drop_cnt;

		rv = hppe_ac_uni_queue_cnt_tbl_get(dev_id, queue_id, &uni_cnt);
		if( rv != SW_OK )
			return rv;
		info->pending_buff_num = uni_cnt.bf.ac_uni_queue_cnt;
		for (i = 0; i < UCAST_QUEUE_ITEMS; i++) {
			hppe_uni_drop_cnt_tbl_get(dev_id, queue_id*UCAST_QUEUE_ITEMS+i, &uni_drop_cnt);
			info->drop_packets[i] = uni_drop_cnt.bf.uni_drop_pkt;
			info->drop_bytes[i] = (a_uint64_t)uni_drop_cnt.bf.uni_drop_byte_0 |
					(a_uint64_t)uni_drop_cnt.bf.uni_drop_byte_1 <<32;
		}
	}
	info->tx_packets = tx_cnt.bf.tx_packets;
	info->tx_bytes = (a_uint64_t)tx_cnt.bf.tx_bytes_0 | (a_uint64_t)tx_cnt.bf.tx_bytes_1 << 32;

	return SW_OK;
}

sw_error_t
adpt_hppe_queue_counter_ctrl_get(a_uint32_t dev_id, a_bool_t *cnt_en)
{
	sw_error_t rv = SW_OK;
	union eg_bridge_config_u eg_bridge_config;

	memset(&eg_bridge_config, 0, sizeof(eg_bridge_config));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cnt_en);

	rv = hppe_eg_bridge_config_get(dev_id, &eg_bridge_config);
	if( rv != SW_OK )
		return rv;

	*cnt_en = eg_bridge_config.bf.queue_cnt_en;

	return SW_OK;
}

sw_error_t
adpt_hppe_queue_counter_ctrl_set(a_uint32_t dev_id, a_bool_t cnt_en)
{
	sw_error_t rv = SW_OK;
	union eg_bridge_config_u eg_bridge_config;

	memset(&eg_bridge_config, 0, sizeof(eg_bridge_config));
	ADPT_DEV_ID_CHECK(dev_id);


	rv = hppe_eg_bridge_config_get(dev_id, &eg_bridge_config);
	if( rv != SW_OK )
		return rv;

	eg_bridge_config.bf.queue_cnt_en = cnt_en;
	return hppe_eg_bridge_config_set(dev_id, &eg_bridge_config);
}

sw_error_t
adpt_hppe_qm_enqueue_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t queue_id,
		a_bool_t enable)
{
	union oq_enq_opr_tbl_u enq;

	ADPT_DEV_ID_CHECK(dev_id);
	memset(&enq, 0, sizeof(enq));

	enq.bf.enq_disable = !enable;
	return hppe_oq_enq_opr_tbl_set(dev_id, queue_id, &enq);
}

static sw_error_t
adpt_hppe_qm_port_source_profile_set(
		a_uint32_t dev_id, fal_port_t port, a_uint32_t src_profile)
{
	union mru_mtu_ctrl_tbl_u mru_mtu_ctrl_tbl;
	a_uint32_t index = FAL_PORT_ID_VALUE(port);

	ADPT_DEV_ID_CHECK(dev_id);
	memset(&mru_mtu_ctrl_tbl, 0, sizeof(mru_mtu_ctrl_tbl));


	return hppe_mru_mtu_ctrl_tbl_src_profile_set(dev_id, index,
				src_profile);
}

static sw_error_t
adpt_hppe_qm_port_source_profile_get(
		a_uint32_t dev_id, fal_port_t port, a_uint32_t *src_profile)
{
	union mru_mtu_ctrl_tbl_u mru_mtu_ctrl_tbl;
	a_uint32_t index = FAL_PORT_ID_VALUE(port);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(src_profile);
	memset(&mru_mtu_ctrl_tbl, 0, sizeof(mru_mtu_ctrl_tbl));

	return hppe_mru_mtu_ctrl_tbl_src_profile_get(dev_id, index,
				src_profile);
}

sw_error_t adpt_hppe_qm_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_ac_ctrl_set = adpt_hppe_ac_ctrl_set;
	p_adpt_api->adpt_ac_prealloc_buffer_set = adpt_hppe_ac_prealloc_buffer_set;
	p_adpt_api->adpt_ac_queue_group_set = adpt_hppe_ac_queue_group_set;
	p_adpt_api->adpt_ac_static_threshold_set = adpt_hppe_ac_static_threshold_set;
	p_adpt_api->adpt_ac_dynamic_threshold_set = adpt_hppe_ac_dynamic_threshold_set;
	p_adpt_api->adpt_ac_group_buffer_set = adpt_hppe_ac_group_buffer_set;
	p_adpt_api->adpt_ucast_queue_base_profile_set =
		adpt_hppe_ucast_queue_base_profile_set;
	p_adpt_api->adpt_queue_flush = adpt_hppe_queue_flush;
	p_adpt_api->adpt_qm_enqueue_ctrl_set = adpt_hppe_qm_enqueue_ctrl_set;
	p_adpt_api->adpt_ucast_hash_map_set = adpt_hppe_ucast_hash_map_set;
	p_adpt_api->adpt_ac_dynamic_threshold_get = adpt_hppe_ac_dynamic_threshold_get;
	p_adpt_api->adpt_ucast_queue_base_profile_get =
			adpt_hppe_ucast_queue_base_profile_get;
#if !defined(IN_QM_MINI)
	p_adpt_api->adpt_port_mcast_priority_class_get =
		adpt_hppe_port_mcast_priority_class_get;
	p_adpt_api->adpt_port_mcast_priority_class_set =
		adpt_hppe_port_mcast_priority_class_set;
	p_adpt_api->adpt_mcast_cpu_code_class_get = adpt_hppe_mcast_cpu_code_class_get;
	p_adpt_api->adpt_mcast_cpu_code_class_set = adpt_hppe_mcast_cpu_code_class_set;
	p_adpt_api->adpt_ucast_default_hash_get = adpt_hppe_ucast_default_hash_get;
	p_adpt_api->adpt_ucast_default_hash_set = adpt_hppe_ucast_default_hash_set;
#endif
	p_adpt_api->adpt_ac_queue_group_get = adpt_hppe_ac_queue_group_get;
	p_adpt_api->adpt_ac_ctrl_get = adpt_hppe_ac_ctrl_get;
	p_adpt_api->adpt_ac_prealloc_buffer_get = adpt_hppe_ac_prealloc_buffer_get;
	p_adpt_api->adpt_ucast_hash_map_get = adpt_hppe_ucast_hash_map_get;
	p_adpt_api->adpt_ac_group_buffer_get = adpt_hppe_ac_group_buffer_get;
	p_adpt_api->adpt_ucast_priority_class_get = adpt_hppe_ucast_priority_class_get;
	p_adpt_api->adpt_ucast_priority_class_set = adpt_hppe_ucast_priority_class_set;
	p_adpt_api->adpt_ac_static_threshold_get = adpt_hppe_ac_static_threshold_get;
	p_adpt_api->adpt_queue_counter_cleanup = adpt_hppe_queue_counter_cleanup;
	p_adpt_api->adpt_queue_counter_get = adpt_hppe_queue_counter_get;
	p_adpt_api->adpt_queue_counter_ctrl_get = adpt_hppe_queue_counter_ctrl_get;
	p_adpt_api->adpt_queue_counter_ctrl_set = adpt_hppe_queue_counter_ctrl_set;
	p_adpt_api->adpt_qm_enqueue_ctrl_get = adpt_hppe_qm_enqueue_ctrl_get;
	p_adpt_api->adpt_qm_port_source_profile_get = adpt_hppe_qm_port_source_profile_get;
	p_adpt_api->adpt_qm_port_source_profile_set = adpt_hppe_qm_port_source_profile_set;
#if defined(APPE)
#if !defined(IN_QM_MINI)
	p_adpt_api->adpt_qm_enqueue_config_get = adpt_appe_qm_enqueue_config_get;
	p_adpt_api->adpt_qm_enqueue_config_set = adpt_appe_qm_enqueue_config_set;
#endif
#endif

	return SW_OK;
}

/**
 * @}
 */
