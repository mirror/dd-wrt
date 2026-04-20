/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "appe_pppoe_reg.h"
#include "appe_pppoe.h"
#include "hppe_ip_reg.h"
#include "hppe_ip.h"
#include "appe_tunnel_reg.h"
#include "appe_tunnel.h"
#include "adpt.h"

#define MAX_SESSION_ID 0xffff

static a_uint32_t
adpt_pppoe_session_compare(a_uint32_t dev_id, a_uint32_t entry_index,
		fal_pppoe_session_t *session_tbl) {

	union pppoe_session_u pppoe_session = {0};
	union pppoe_session_ext_u pppoe_session_ext = {0};
	union pppoe_session_ext1_u pppoe_session_ext1 = {0};
	a_uint16_t smac_ext = 0;
	a_uint32_t smac_ext1 = 0;
	a_uint32_t cmp_result = ADPT_PORT_ID_NOEQUAL;
	a_uint32_t index, port_type, port_value;

	port_type = FAL_PORT_ID_TYPE(session_tbl->port_bitmap);
	port_value = FAL_PORT_ID_VALUE(session_tbl->port_bitmap);

	appe_pppoe_session_get(dev_id, entry_index, &pppoe_session);
	appe_pppoe_session_ext_get(dev_id, entry_index, &pppoe_session_ext);
	appe_pppoe_session_ext1_get(dev_id, entry_index, &pppoe_session_ext1);

	for (index = 0; index <= 3; index++) {
		smac_ext1 = (smac_ext1 << 8) + session_tbl->smac_addr.uc[index];
	}

	for (index = 4; index <= 5; index++) {
		smac_ext = (smac_ext << 8) + session_tbl->smac_addr.uc[index];
	}

	if (pppoe_session.bf.session_id == session_tbl->session_id &&
			pppoe_session.bf.port_type == adpt_port_type_convert(A_TRUE, port_type) &&
			pppoe_session_ext.bf.smac_valid == session_tbl->smac_valid) {
		if (session_tbl->smac_valid == A_FALSE ||
				(session_tbl->smac_valid == A_TRUE &&
				 smac_ext == pppoe_session_ext.bf.smac &&
				 smac_ext1 == pppoe_session_ext1.bf.smac)) {
			switch (port_type) {
				case FAL_PORT_TYPE_PPORT:
					if (pppoe_session.bf.port_bitmap == port_value) {
						cmp_result = ADPT_PORT_ID_EQUAL;
					} else if ((pppoe_session.bf.port_bitmap & port_value) ==
							port_value) {
						cmp_result = ADPT_PORT_ID_INCLD;
					} else {
						cmp_result = ADPT_PORT_ID_EXCLD;
					}
					break;
				case FAL_PORT_TYPE_VPORT:
				case FAL_PORT_TYPE_VP_GROUP:
					if (pppoe_session.bf.port_bitmap == port_value) {
						cmp_result = ADPT_PORT_ID_EQUAL;
					} else {
						cmp_result = ADPT_PORT_ID_NOEQUAL;
					}
					break;
				default:
					cmp_result = ADPT_PORT_ID_NOEQUAL;
			}
		}
	}
	return cmp_result;
}

sw_error_t
adpt_appe_pppoe_session_table_add(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl)
{
	sw_error_t rv = SW_OK;
	union pppoe_session_u pppoe_session = {0};
	union pppoe_session_ext_u pppoe_session_ext = {0};
	union pppoe_session_ext1_u pppoe_session_ext1 = {0};
	union pppoe_session_ext2_u pppoe_session_ext2 = {0};
	union eg_l3_if_tbl_u eg_l3_if_tbl = {0};
	a_uint32_t num, index, entry_idx = PPPOE_SESSION_MAX_ENTRY;
	a_uint16_t smac_ext = 0;
	a_uint32_t smac_ext1 = 0;
	a_uint32_t cmp_result = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(session_tbl);

	if (session_tbl->session_id > MAX_SESSION_ID)
		return SW_BAD_PARAM;
	if (session_tbl->multi_session == A_FALSE && session_tbl->uni_session == A_FALSE)
		return SW_BAD_PARAM;
	if (session_tbl->l3_if_index >= IN_L3_IF_TBL_MAX_ENTRY ||
			session_tbl->tl_l3_if_index >= TL_L3_IF_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	for (num = 0; num < PPPOE_SESSION_MAX_ENTRY; num++)
	{
		rv = appe_pppoe_session_ext_get(dev_id, num, &pppoe_session_ext);
		SW_RTN_ON_ERROR(rv);

		if (pppoe_session_ext.bf.mc_valid == A_FALSE &&
				pppoe_session_ext.bf.uc_valid == A_FALSE) {
			if (entry_idx == PPPOE_SESSION_MAX_ENTRY) {
				entry_idx = num;
			}
		} else {
			cmp_result = adpt_pppoe_session_compare(dev_id, num, session_tbl);
			switch (cmp_result) {
				case ADPT_PORT_ID_EQUAL:
				case ADPT_PORT_ID_INCLD:
					return SW_ALREADY_EXIST;
				case ADPT_PORT_ID_EXCLD:
					pppoe_session.bf.port_bitmap |=
						FAL_PORT_ID_VALUE(session_tbl->port_bitmap);
					rv = appe_pppoe_session_set(dev_id,
							num, &pppoe_session);
					return rv;
				case ADPT_PORT_ID_NOEQUAL:
				default:
					continue;
			}
		}
	}

	if (entry_idx == PPPOE_SESSION_MAX_ENTRY)
		return SW_NO_RESOURCE;

	rv = appe_pppoe_session_get(dev_id, entry_idx, &pppoe_session);
	SW_RTN_ON_ERROR(rv);
	rv = appe_pppoe_session_ext_get(dev_id, entry_idx, &pppoe_session_ext);
	SW_RTN_ON_ERROR(rv);
	rv = appe_pppoe_session_ext1_get(dev_id, entry_idx, &pppoe_session_ext1);
	SW_RTN_ON_ERROR(rv);
	rv = appe_pppoe_session_ext2_get(dev_id, entry_idx, &pppoe_session_ext2);
	SW_RTN_ON_ERROR(rv);

	for (index = 0; index <= 3; index++) {
		smac_ext1 = (smac_ext1 << 8) + session_tbl->smac_addr.uc[index];
	}

	for (index = 4; index <= 5; index++) {
		smac_ext = (smac_ext << 8) + session_tbl->smac_addr.uc[index];
	}

	pppoe_session.bf.port_bitmap = FAL_PORT_ID_VALUE(session_tbl->port_bitmap);
	pppoe_session.bf.port_type = adpt_port_type_convert(A_TRUE,
			FAL_PORT_ID_TYPE(session_tbl->port_bitmap));
	pppoe_session.bf.session_id = session_tbl->session_id;

	pppoe_session_ext.bf.mc_valid = session_tbl->multi_session;
	pppoe_session_ext.bf.uc_valid = session_tbl->uni_session;
	pppoe_session_ext.bf.smac_valid = session_tbl->smac_valid;
	pppoe_session_ext.bf.smac = smac_ext;

	pppoe_session_ext1.bf.smac = smac_ext1;

	pppoe_session_ext2.bf.l3_if_index = session_tbl->l3_if_index;
	pppoe_session_ext2.bf.l3_if_valid = session_tbl->l3_if_valid;
	pppoe_session_ext2.bf.tl_l3_if_index = session_tbl->tl_l3_if_index;
	pppoe_session_ext2.bf.tl_l3_if_valid = session_tbl->tl_l3_if_valid;

	appe_pppoe_session_set(dev_id, entry_idx, &pppoe_session);
	appe_pppoe_session_ext_set(dev_id, entry_idx, &pppoe_session_ext);
	appe_pppoe_session_ext1_set(dev_id, entry_idx, &pppoe_session_ext1);
	appe_pppoe_session_ext2_set(dev_id, entry_idx, &pppoe_session_ext2);


	rv = hppe_eg_l3_if_tbl_get(dev_id, session_tbl->l3_if_index, &eg_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	eg_l3_if_tbl.bf.session_id = session_tbl->session_id;

	rv = hppe_eg_l3_if_tbl_set(dev_id, session_tbl->l3_if_index, &eg_l3_if_tbl);
	SW_RTN_ON_ERROR(rv);

	session_tbl->entry_id = entry_idx;

	return rv;
}

sw_error_t
adpt_appe_pppoe_session_table_del(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl)
{
	sw_error_t rv = SW_OK;
	union pppoe_session_u pppoe_session = {0};
	union pppoe_session_ext_u pppoe_session_ext = {0};
	union pppoe_session_ext2_u pppoe_session_ext2 = {0};
	union eg_l3_if_tbl_u eg_l3_if_tbl = {0};
	a_uint32_t port_bmp = 0;
	a_uint32_t num, l3_if_index = 0;
	a_uint32_t cmp_result = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(session_tbl);

	if (session_tbl->session_id > MAX_SESSION_ID)
		return SW_BAD_PARAM;

	for (num = 0; num < PPPOE_SESSION_MAX_ENTRY; num++)
	{
		rv = appe_pppoe_session_get(dev_id, num, &pppoe_session);
		SW_RTN_ON_ERROR(rv);
		rv = appe_pppoe_session_ext_get(dev_id, num, &pppoe_session_ext);
		SW_RTN_ON_ERROR(rv);

		if (pppoe_session_ext.bf.mc_valid != A_TRUE &&
				pppoe_session_ext.bf.uc_valid != A_TRUE) {
			continue;
		}

		cmp_result = adpt_pppoe_session_compare(dev_id, num, session_tbl);
		switch (cmp_result) {
			case ADPT_PORT_ID_EQUAL:
				break;
			case ADPT_PORT_ID_INCLD:
			case ADPT_PORT_ID_EXCLD:
				port_bmp = pppoe_session.bf.port_bitmap &
					~FAL_PORT_ID_VALUE(session_tbl->port_bitmap);
				if (port_bmp != 0) {
					pppoe_session.bf.port_bitmap = port_bmp;
					rv = appe_pppoe_session_set(dev_id,
							num, &pppoe_session);
					return rv;
				}
				break;
			case ADPT_PORT_ID_NOEQUAL:
			default:
				continue;
		}

		rv = appe_pppoe_session_ext2_get(dev_id, num, &pppoe_session_ext2);
		SW_RTN_ON_ERROR(rv);

		l3_if_index = pppoe_session_ext2.bf.l3_if_index;

		pppoe_session_ext.bf.mc_valid = A_FALSE;
		pppoe_session_ext.bf.uc_valid = A_FALSE;

		rv = appe_pppoe_session_ext_set(dev_id, num, &pppoe_session_ext);
		SW_RTN_ON_ERROR(rv);

		rv = hppe_eg_l3_if_tbl_get(dev_id, l3_if_index, &eg_l3_if_tbl);
		SW_RTN_ON_ERROR(rv);

		eg_l3_if_tbl.bf.session_id = 0;
		rv = hppe_eg_l3_if_tbl_set(dev_id, l3_if_index, &eg_l3_if_tbl);

		return rv;
	}

	return SW_NOT_FOUND;
}

sw_error_t
adpt_appe_pppoe_session_table_get(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl)
{
	sw_error_t rv;
	union pppoe_session_u pppoe_session = {0};
	union pppoe_session_ext_u pppoe_session_ext = {0};
	union pppoe_session_ext1_u pppoe_session_ext1 = {0};
	union pppoe_session_ext2_u pppoe_session_ext2 = {0};
	a_uint32_t num, cmp_result = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(session_tbl);

	if (session_tbl->session_id > MAX_SESSION_ID)
		return SW_BAD_PARAM;

	for (num = 0; num < PPPOE_SESSION_MAX_ENTRY; num++) {
		rv = appe_pppoe_session_ext_get(dev_id, num, &pppoe_session_ext);
		SW_RTN_ON_ERROR(rv);

		if (pppoe_session_ext.bf.mc_valid != A_TRUE &&
				pppoe_session_ext.bf.uc_valid != A_TRUE) {
			continue;
		}

		cmp_result = adpt_pppoe_session_compare(dev_id, num, session_tbl);
		switch (cmp_result) {
			case ADPT_PORT_ID_EQUAL:
			case ADPT_PORT_ID_INCLD:
				break;
			case ADPT_PORT_ID_EXCLD:
			case ADPT_PORT_ID_NOEQUAL:
			default:
				continue;
		}

		rv = appe_pppoe_session_get(dev_id, num, &pppoe_session);
		SW_RTN_ON_ERROR(rv);
		rv = appe_pppoe_session_ext1_get(dev_id, num, &pppoe_session_ext1);
		SW_RTN_ON_ERROR(rv);
		rv = appe_pppoe_session_ext2_get(dev_id, num, &pppoe_session_ext2);
		SW_RTN_ON_ERROR(rv);

		session_tbl->l3_if_index = pppoe_session_ext2.bf.l3_if_index;
		session_tbl->l3_if_valid = pppoe_session_ext2.bf.l3_if_valid;
		session_tbl->tl_l3_if_index = pppoe_session_ext2.bf.tl_l3_if_index;
		session_tbl->tl_l3_if_valid = pppoe_session_ext2.bf.tl_l3_if_valid;
		session_tbl->port_bitmap = FAL_PORT_ID(adpt_port_type_convert(A_FALSE,
					pppoe_session.bf.port_type),
				pppoe_session.bf.port_bitmap);
		session_tbl->session_id = pppoe_session.bf.session_id;
		session_tbl->multi_session = pppoe_session_ext.bf.mc_valid;
		session_tbl->uni_session = pppoe_session_ext.bf.uc_valid;
		session_tbl->smac_valid = pppoe_session_ext.bf.smac_valid;
		session_tbl->smac_addr.uc[0] = (pppoe_session_ext1.bf.smac >> 24) & 0xff;
		session_tbl->smac_addr.uc[1] = (pppoe_session_ext1.bf.smac >> 16) & 0xff;
		session_tbl->smac_addr.uc[2] = (pppoe_session_ext1.bf.smac >> 8) & 0xff;
		session_tbl->smac_addr.uc[3] = pppoe_session_ext1.bf.smac & 0xff;
		session_tbl->smac_addr.uc[4] = (pppoe_session_ext.bf.smac >> 8) & 0xff;
		session_tbl->smac_addr.uc[5] = pppoe_session_ext.bf.smac & 0xff;
		session_tbl->entry_id = num;

		return rv;
	}

	return SW_NOT_FOUND;
}

sw_error_t
adpt_appe_pppoe_l3_intf_get(a_uint32_t dev_id, a_uint32_t pppoe_index,
		fal_intf_type_t l3_type, fal_intf_id_t *pppoe_intf)
{
	sw_error_t rv = SW_OK;
	union pppoe_session_ext2_u pppoe_session_ext2 = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pppoe_intf);

	if (pppoe_index >= PPPOE_SESSION_EXT2_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	rv = appe_pppoe_session_ext2_get(dev_id, pppoe_index, &pppoe_session_ext2);
	SW_RTN_ON_ERROR(rv);

	switch (l3_type) {
		case FAL_INTF_TYPE_TUNNEL:
			pppoe_intf->l3_if_valid = pppoe_session_ext2.bf.tl_l3_if_valid;
			pppoe_intf->l3_if_index = pppoe_session_ext2.bf.tl_l3_if_index;
			break;
		case FAL_INTF_TYPE_NORMAL:
			pppoe_intf->l3_if_valid = pppoe_session_ext2.bf.l3_if_valid;
			pppoe_intf->l3_if_index = pppoe_session_ext2.bf.l3_if_index;
			break;
		default:
			return SW_BAD_PARAM;
	}

	return rv;
}

sw_error_t
adpt_appe_pppoe_l3_intf_set(a_uint32_t dev_id, a_uint32_t pppoe_index,
		fal_intf_type_t l3_type, fal_intf_id_t *pppoe_intf)
{
	sw_error_t rv = SW_OK;
	union pppoe_session_ext2_u pppoe_session_ext2 = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pppoe_intf);

	if (pppoe_index >= PPPOE_SESSION_EXT2_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	rv = appe_pppoe_session_ext2_get(dev_id, pppoe_index, &pppoe_session_ext2);
	SW_RTN_ON_ERROR(rv);

	switch (l3_type) {
		case FAL_INTF_TYPE_TUNNEL:
			pppoe_session_ext2.bf.tl_l3_if_valid = pppoe_intf->l3_if_valid;
			pppoe_session_ext2.bf.tl_l3_if_index = pppoe_intf->l3_if_index;
			break;
		case FAL_INTF_TYPE_NORMAL:
			pppoe_session_ext2.bf.l3_if_valid = pppoe_intf->l3_if_valid;
			pppoe_session_ext2.bf.l3_if_index = pppoe_intf->l3_if_index;
			break;
		default:
			return SW_BAD_PARAM;
	}

	rv = appe_pppoe_session_ext2_set(dev_id, pppoe_index, &pppoe_session_ext2);
	return rv;
}

/**
 * @}
 */
