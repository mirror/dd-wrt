/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef _APPE_SHAPER_H_
#define _APPE_SHAPER_H_

sw_error_t
appe_min_max_mode_cfg_get(
		a_uint32_t dev_id,
		union min_max_mode_cfg_u *value);

sw_error_t
appe_min_max_mode_cfg_set(
		a_uint32_t dev_id,
		union min_max_mode_cfg_u *value);

sw_error_t
appe_eco_reserve_0_get(
		a_uint32_t dev_id,
		union eco_reserve_0_u *value);

sw_error_t
appe_eco_reserve_0_set(
		a_uint32_t dev_id,
		union eco_reserve_0_u *value);

sw_error_t
appe_eco_reserve_1_get(
		a_uint32_t dev_id,
		union eco_reserve_1_u *value);

sw_error_t
appe_eco_reserve_1_set(
		a_uint32_t dev_id,
		union eco_reserve_1_u *value);

sw_error_t
appe_shp_cfg_l0_get(
		a_uint32_t dev_id,
		union shp_cfg_l0_u *value);

sw_error_t
appe_shp_cfg_l0_set(
		a_uint32_t dev_id,
		union shp_cfg_l0_u *value);

sw_error_t
appe_shp_cfg_l1_get(
		a_uint32_t dev_id,
		union shp_cfg_l1_u *value);

sw_error_t
appe_shp_cfg_l1_set(
		a_uint32_t dev_id,
		union shp_cfg_l1_u *value);

sw_error_t
appe_min_max_mode_cfg_min_max_mode_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_min_max_mode_cfg_min_max_mode_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_eco_reserve_0_eco_res_0_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_eco_reserve_0_eco_res_0_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_eco_reserve_1_eco_res_1_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_eco_reserve_1_eco_res_1_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_head_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_head_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_tail_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_tail_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_head_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_head_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_tail_get(
		a_uint32_t dev_id,
		unsigned int *value);

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_tail_set(
		a_uint32_t dev_id,
		unsigned int value);

sw_error_t
appe_l1_shp_cfg_tbl_grp_end_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l1_shp_cfg_tbl_grp_end_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l1_shp_cfg_tbl_eir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l1_shp_cfg_tbl_eir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l1_shp_cfg_tbl_grp_cf_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l1_shp_cfg_tbl_grp_cf_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l1_shp_cfg_tbl_shp_refresh_nxt_ptr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l1_shp_cfg_tbl_shp_refresh_nxt_ptr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l1_shp_cfg_tbl_cir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l1_shp_cfg_tbl_cir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l0_shp_cfg_tbl_grp_end_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l0_shp_cfg_tbl_grp_end_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l0_shp_cfg_tbl_eir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l0_shp_cfg_tbl_eir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l0_shp_cfg_tbl_grp_cf_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l0_shp_cfg_tbl_grp_cf_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l0_shp_cfg_tbl_shp_refresh_nxt_ptr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l0_shp_cfg_tbl_shp_refresh_nxt_ptr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
appe_l0_shp_cfg_tbl_cir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
appe_l0_shp_cfg_tbl_cir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif

