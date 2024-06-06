/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021, 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "hppe_pppoe_reg.h"
#include "hppe_pppoe.h"
#include "hppe_ip_reg.h"
#include "hppe_ip.h"
#include "adpt.h"
#if defined(APPE)
#include "adpt_appe_pppoe.h"
#endif

#define MAX_SESSION_ID 0xffff

sw_error_t
adpt_hppe_pppoe_session_table_add(a_uint32_t dev_id, fal_pppoe_session_t * session_tbl)
{
	sw_error_t rv = SW_OK;
	union pppoe_session_u pppoe_session = {0};
	union pppoe_session_ext_u pppoe_session_ext = {0};
	union pppoe_session_ext1_u pppoe_session_ext1 = {0};
	union eg_l3_if_tbl_u eg_l3_if_tbl = {0};
	a_uint32_t num, index, entry_idx = PPPOE_SESSION_MAX_ENTRY;
	a_uint16_t smac_ext = 0;
	a_uint32_t smac_ext1 = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(session_tbl);

	if (session_tbl->session_id > MAX_SESSION_ID)
		return SW_BAD_PARAM;
	if (session_tbl->multi_session == A_FALSE && session_tbl->uni_session == A_FALSE)
		return SW_BAD_PARAM;
	if (session_tbl->l3_if_index >= IN_L3_IF_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;
	for (num = 0; num < PPPOE_SESSION_MAX_ENTRY; num++)
	{
		hppe_pppoe_session_get(dev_id, num, &pppoe_session);
		hppe_pppoe_session_ext_get(dev_id, num, &pppoe_session_ext);
		hppe_pppoe_session_ext1_get(dev_id, num, &pppoe_session_ext1);

		for (index = 0; index <= 3; index++) {
			smac_ext1 = (smac_ext1 << 8) + session_tbl->smac_addr.uc[index];

		}
		for (index = 4; index <= 5; index++) {
			smac_ext = (smac_ext << 8) + session_tbl->smac_addr.uc[index];
		}

		if (pppoe_session_ext.bf.mc_valid == A_FALSE &&
				pppoe_session_ext.bf.uc_valid == A_FALSE)
		{
			if (entry_idx == PPPOE_SESSION_MAX_ENTRY)
				entry_idx = num;
		}
		else if (pppoe_session.bf.session_id == session_tbl->session_id &&
				pppoe_session_ext.bf.smac_valid == session_tbl->smac_valid) {
			if (session_tbl->smac_valid == A_FALSE ||
					(session_tbl->smac_valid == A_TRUE &&
					 smac_ext == pppoe_session_ext.bf.smac &&
					 smac_ext1 == pppoe_session_ext1.bf.smac)) {
				return SW_ALREADY_EXIST;
			}
		}
	}
	if (entry_idx == PPPOE_SESSION_MAX_ENTRY)
		return SW_NO_RESOURCE;

	pppoe_session.bf.session_id = session_tbl->session_id;
	pppoe_session.bf.port_bitmap = session_tbl->port_bitmap;
	pppoe_session.bf.l3_if_index = session_tbl->l3_if_index;

	pppoe_session_ext.bf.l3_if_valid = session_tbl->l3_if_valid;
	pppoe_session_ext.bf.mc_valid = session_tbl->multi_session;
	pppoe_session_ext.bf.uc_valid = session_tbl->uni_session;
	pppoe_session_ext.bf.smac_valid = session_tbl->smac_valid;

	pppoe_session_ext.bf.smac = smac_ext;
	pppoe_session_ext1.bf.smac = smac_ext1;

	hppe_pppoe_session_set(dev_id, entry_idx, &pppoe_session);
	hppe_pppoe_session_ext_set(dev_id, entry_idx, &pppoe_session_ext);
	hppe_pppoe_session_ext1_set(dev_id, entry_idx, &pppoe_session_ext1);

	rv = hppe_eg_l3_if_tbl_get(dev_id, session_tbl->l3_if_index, &eg_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	eg_l3_if_tbl.bf.session_id = session_tbl->session_id;
	rv = hppe_eg_l3_if_tbl_set(dev_id, session_tbl->l3_if_index, &eg_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	session_tbl->entry_id = entry_idx;

	return rv;
}

sw_error_t
adpt_hppe_pppoe_session_table_del(a_uint32_t dev_id, fal_pppoe_session_t * session_tbl)
{
	sw_error_t rv = SW_OK;
	union pppoe_session_u pppoe_session = {0};
	union pppoe_session_ext_u pppoe_session_ext = {0};
	union pppoe_session_ext1_u pppoe_session_ext1 = {0};
	union pppoe_session_u pppoe_session_zero = {0};
	union pppoe_session_ext_u pppoe_session_ext_zero = {0};
	union pppoe_session_ext1_u pppoe_session_ext1_zero = {0};
	union eg_l3_if_tbl_u eg_l3_if_tbl = {0};
	a_uint16_t smac_ext = 0;
	a_uint32_t smac_ext1 = 0;
	a_uint32_t num, index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(session_tbl);

	if (session_tbl->session_id > MAX_SESSION_ID)
		return SW_BAD_PARAM;

	for (num = 0; num < PPPOE_SESSION_MAX_ENTRY; num++)
	{
		hppe_pppoe_session_get(dev_id, num, &pppoe_session);
		hppe_pppoe_session_ext_get(dev_id, num, &pppoe_session_ext);
		hppe_pppoe_session_ext1_get(dev_id, num, &pppoe_session_ext1);

		for (index = 0; index <= 3; index++) {
			smac_ext1 = (smac_ext1 << 8) + session_tbl->smac_addr.uc[index];
		}
		for (index = 4; index <= 5; index++) {
			smac_ext = (smac_ext << 8) + session_tbl->smac_addr.uc[index];
		}

		if ((pppoe_session_ext.bf.mc_valid == A_TRUE ||
					pppoe_session_ext.bf.uc_valid == A_TRUE) &&
				(pppoe_session.bf.session_id == session_tbl->session_id &&
				 pppoe_session_ext.bf.smac_valid == session_tbl->smac_valid &&
				 (session_tbl->smac_valid == A_FALSE ||
				  (session_tbl->smac_valid == A_TRUE &&
				   smac_ext == pppoe_session_ext.bf.smac &&
				   smac_ext1 == pppoe_session_ext1.bf.smac))))
		{
			hppe_pppoe_session_set(dev_id, num, &pppoe_session_zero);
			hppe_pppoe_session_ext_set(dev_id, num, &pppoe_session_ext_zero);
			hppe_pppoe_session_ext1_set(dev_id, num, &pppoe_session_ext1_zero);

			rv = hppe_eg_l3_if_tbl_get(dev_id,
					pppoe_session.bf.l3_if_index, &eg_l3_if_tbl);
			SW_RTN_ON_ERROR(rv);

			eg_l3_if_tbl.bf.session_id = 0;
			rv = hppe_eg_l3_if_tbl_set(dev_id,
					pppoe_session.bf.l3_if_index, &eg_l3_if_tbl);

			return rv;
		}
	}

	return SW_NOT_FOUND;
}

sw_error_t
adpt_hppe_pppoe_session_table_get(a_uint32_t dev_id, fal_pppoe_session_t * session_tbl)
{
	union pppoe_session_u pppoe_session = {0};
	union pppoe_session_ext_u pppoe_session_ext = {0};
	union pppoe_session_ext1_u pppoe_session_ext1 = {0};
	a_uint16_t smac_ext = 0;
	a_uint32_t smac_ext1 = 0;
	a_uint32_t num, index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(session_tbl);

	if (session_tbl->session_id > MAX_SESSION_ID)
		return SW_BAD_PARAM;

	for (num = 0; num < PPPOE_SESSION_MAX_ENTRY; num++)
	{
		hppe_pppoe_session_get(dev_id, num, &pppoe_session);
		hppe_pppoe_session_ext_get(dev_id, num, &pppoe_session_ext);
		hppe_pppoe_session_ext1_get(dev_id, num, &pppoe_session_ext1);

		for (index = 0; index <= 3; index++) {
			smac_ext1 = (smac_ext1 << 8) + session_tbl->smac_addr.uc[index];
		}
		for (index = 4; index <= 5; index++) {
			smac_ext = (smac_ext << 8) + session_tbl->smac_addr.uc[index];
		}
		if ((pppoe_session_ext.bf.mc_valid == A_TRUE ||
					pppoe_session_ext.bf.uc_valid == A_TRUE) &&
				(pppoe_session.bf.session_id == session_tbl->session_id &&
				 pppoe_session_ext.bf.smac_valid == session_tbl->smac_valid &&
				 (session_tbl->smac_valid == A_FALSE ||
				  (session_tbl->smac_valid == A_TRUE &&
				   smac_ext == pppoe_session_ext.bf.smac &&
				   smac_ext1 == pppoe_session_ext1.bf.smac))))
		{
			session_tbl->entry_id = num;
			session_tbl->session_id = pppoe_session.bf.session_id;
			session_tbl->port_bitmap = pppoe_session.bf.port_bitmap;
			session_tbl->l3_if_index = pppoe_session.bf.l3_if_index;
			session_tbl->l3_if_valid = pppoe_session_ext.bf.l3_if_valid;
			session_tbl->multi_session = pppoe_session_ext.bf.mc_valid;
			session_tbl->uni_session = pppoe_session_ext.bf.uc_valid;
			session_tbl->smac_valid = pppoe_session_ext.bf.smac_valid;
			session_tbl->smac_addr.uc[0] = (pppoe_session_ext1.bf.smac >> 24) & 0xff;
			session_tbl->smac_addr.uc[1] = (pppoe_session_ext1.bf.smac >> 16) & 0xff;
			session_tbl->smac_addr.uc[2] = (pppoe_session_ext1.bf.smac >> 8) & 0xff;
			session_tbl->smac_addr.uc[3] = pppoe_session_ext1.bf.smac & 0xff;
			session_tbl->smac_addr.uc[4] = (pppoe_session_ext.bf.smac >> 8) & 0xff;
			session_tbl->smac_addr.uc[5] = pppoe_session_ext.bf.smac & 0xff;
			return SW_OK;
		}
	}

	return SW_NOT_FOUND;
}

sw_error_t
adpt_ppe_pppoe_session_table_add(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl) {
	sw_error_t rv = SW_OK;

#if defined(APPE)
	if (adpt_chip_type_get(dev_id) == CHIP_APPE) {
		rv = adpt_appe_pppoe_session_table_add(dev_id, session_tbl);
	} else
#endif
	{
		rv = adpt_hppe_pppoe_session_table_add(dev_id, session_tbl);
	}

	return rv;
}

sw_error_t
adpt_ppe_pppoe_session_table_del(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl) {
	sw_error_t rv = SW_OK;

#if defined(APPE)
	if (adpt_chip_type_get(dev_id) == CHIP_APPE) {
		rv = adpt_appe_pppoe_session_table_del(dev_id, session_tbl);
	} else
#endif
	{
		rv = adpt_hppe_pppoe_session_table_del(dev_id, session_tbl);
	}

	return rv;
}

sw_error_t
adpt_ppe_pppoe_session_table_get(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl) {
	sw_error_t rv = SW_OK;

#if defined(APPE)
	if (adpt_chip_type_get(dev_id) == CHIP_APPE) {
		rv = adpt_appe_pppoe_session_table_get(dev_id, session_tbl);
	} else
#endif
	{
		rv = adpt_hppe_pppoe_session_table_get(dev_id, session_tbl);
	}

	return rv;
}

sw_error_t
adpt_hppe_pppoe_en_set(a_uint32_t dev_id, a_uint32_t l3_if, a_uint32_t enable)
{
	sw_error_t rv = SW_OK;
	union in_l3_if_tbl_u in_l3_if_tbl = {0};
	union eg_l3_if_tbl_u eg_l3_if_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	rv = hppe_in_l3_if_tbl_get(dev_id, l3_if, &in_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_eg_l3_if_tbl_get(dev_id, l3_if, &eg_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	in_l3_if_tbl.bf.pppoe_en = enable;
	eg_l3_if_tbl.bf.pppoe_en = enable;

	rv = hppe_in_l3_if_tbl_set(dev_id, l3_if, &in_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	rv = hppe_eg_l3_if_tbl_set(dev_id, l3_if, &eg_l3_if_tbl);

	return rv;
}

sw_error_t
adpt_hppe_pppoe_en_get(a_uint32_t dev_id, a_uint32_t l3_if, a_uint32_t *enable)
{
	sw_error_t rv = SW_OK;
	union in_l3_if_tbl_u in_l3_if_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

	rv = hppe_in_l3_if_tbl_get(dev_id, l3_if, &in_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	*enable = in_l3_if_tbl.bf.pppoe_en;

	return rv;
}

sw_error_t
adpt_hppe_pppoe_global_ctrl_set(a_uint32_t dev_id, fal_pppoe_global_cfg_t *cfg)
{
	sw_error_t rv = SW_OK;
	union l3_route_ctrl_u l3_route_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	aos_mem_zero(&l3_route_ctrl, sizeof(l3_route_ctrl));

	rv = hppe_l3_route_ctrl_get(dev_id, &l3_route_ctrl);
	SW_RTN_ON_ERROR(rv);

	l3_route_ctrl.bf.pppoe_multicast_cmd = cfg->pppoe_multicast_cmd;
	l3_route_ctrl.bf.pppoe_multicast_de_acce = cfg->pppoe_multicast_deacclr_en;

	rv = hppe_l3_route_ctrl_set(dev_id, &l3_route_ctrl);
	return rv;
}

sw_error_t
adpt_hppe_pppoe_global_ctrl_get(a_uint32_t dev_id, fal_pppoe_global_cfg_t *cfg)
{
	sw_error_t rv = SW_OK;
	union l3_route_ctrl_u l3_route_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	aos_mem_zero(&l3_route_ctrl, sizeof(l3_route_ctrl));

	rv = hppe_l3_route_ctrl_get(dev_id, &l3_route_ctrl);
	SW_RTN_ON_ERROR(rv);

	cfg->pppoe_multicast_cmd = l3_route_ctrl.bf.pppoe_multicast_cmd;
	cfg->pppoe_multicast_deacclr_en = l3_route_ctrl.bf.pppoe_multicast_de_acce;

	return rv;
}

sw_error_t adpt_hppe_pppoe_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_pppoe_session_table_add = adpt_ppe_pppoe_session_table_add;
	p_adpt_api->adpt_pppoe_session_table_del = adpt_ppe_pppoe_session_table_del;
	p_adpt_api->adpt_pppoe_session_table_get = adpt_ppe_pppoe_session_table_get;
	p_adpt_api->adpt_pppoe_en_set = adpt_hppe_pppoe_en_set;
	p_adpt_api->adpt_pppoe_en_get = adpt_hppe_pppoe_en_get;
#if defined(APPE)
	p_adpt_api->adpt_pppoe_l3_intf_set = adpt_appe_pppoe_l3_intf_set;
	p_adpt_api->adpt_pppoe_l3_intf_get = adpt_appe_pppoe_l3_intf_get;
#endif
	p_adpt_api->adpt_pppoe_global_ctrl_set = adpt_hppe_pppoe_global_ctrl_set;
	p_adpt_api->adpt_pppoe_global_ctrl_get = adpt_hppe_pppoe_global_ctrl_get;

	return SW_OK;
}

/**
 * @}
 */
