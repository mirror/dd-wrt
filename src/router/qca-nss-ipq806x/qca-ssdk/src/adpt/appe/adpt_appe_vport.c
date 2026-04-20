/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "fal_vport.h"
#if defined(IN_QM)
#include "fal_qm.h"
#endif
#include "appe_l2_vp_reg.h"
#include "appe_l2_vp.h"
#include "hppe_portctrl_reg.h"
#include "hppe_portctrl.h"

#include "adpt.h"

sw_error_t
adpt_appe_vport_physical_port_id_get(a_uint32_t dev_id,
		fal_port_t vport_id, fal_port_t *phyport_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t pport_id = 0;
	a_uint32_t vport_value = FAL_PORT_ID_VALUE(vport_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(phyport_id);

	rv = appe_l2_vp_port_tbl_physical_port_get(dev_id, vport_value, &pport_id);
	SW_RTN_ON_ERROR(rv);

	*phyport_id = FAL_PORT_ID(FAL_PORT_TYPE_PPORT, pport_id);

	return rv;
}

sw_error_t
adpt_appe_vport_physical_port_id_set(a_uint32_t dev_id,
		fal_port_t vport_id, fal_port_t phyport_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t pport_id = 0;
	a_uint32_t vport_value = 0;
#if defined(IN_QM)
	adpt_api_t *p_adpt_api = NULL;
	a_uint32_t queue_base = 0;
	a_uint8_t profile = 0;
	fal_ucast_queue_dest_t queue_dest;
#endif

	ADPT_DEV_ID_CHECK(dev_id);
	vport_value = FAL_PORT_ID_VALUE(vport_id);
	pport_id = FAL_PORT_ID_VALUE(phyport_id);

	rv = appe_l2_vp_port_tbl_physical_port_set(dev_id, vport_value, pport_id);
	SW_RTN_ON_ERROR(rv);

#if defined(IN_QM)
	p_adpt_api = adpt_api_ptr_get(dev_id);
	ADPT_NULL_POINT_CHECK(p_adpt_api);
	ADPT_NULL_POINT_CHECK(p_adpt_api->adpt_ucast_queue_base_profile_set);
	ADPT_NULL_POINT_CHECK(p_adpt_api->adpt_ucast_queue_base_profile_get);

	aos_mem_zero(&queue_dest, sizeof(fal_ucast_queue_dest_t));

	/* configure the queue base of vport with the queue base of physical port
	 * the source profile id 0 is selected by default.
	 * */
	queue_dest.src_profile = 0;
	queue_dest.dst_port = pport_id;
	rv = p_adpt_api->adpt_ucast_queue_base_profile_get(dev_id,
			&queue_dest, &queue_base, &profile);
	SW_RTN_ON_ERROR(rv);

	queue_dest.dst_port = vport_value;
	rv = p_adpt_api->adpt_ucast_queue_base_profile_set(dev_id,
			&queue_dest, queue_base, profile);
#endif

	return rv;
}

sw_error_t
adpt_appe_vport_state_check_get(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(vp_state);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_value, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	vp_state->check_en = l2_vp_port_tbl.bf.vp_state_check_en;
	vp_state->vp_type = l2_vp_port_tbl.bf.vp_type;
	vp_state->vp_active = l2_vp_port_tbl.bf.vp_context_active;
	vp_state->eg_data_valid = l2_vp_port_tbl.bf.vp_eg_data_valid;

	return rv;
}

sw_error_t
adpt_appe_vport_state_check_set(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(vp_state);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_value, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.vp_state_check_en = vp_state->check_en;
	l2_vp_port_tbl.bf.vp_type = vp_state->vp_type;
	l2_vp_port_tbl.bf.vp_context_active = vp_state->vp_active;
	l2_vp_port_tbl.bf.vp_eg_data_valid = vp_state->eg_data_valid;

	rv = appe_l2_vp_port_tbl_set(dev_id, port_value, &l2_vp_port_tbl);

	return rv;
}

sw_error_t
adpt_appe_vport_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	ADPT_NULL_POINT_CHECK(p_adpt_api);

		p_adpt_api->adpt_vport_physical_port_id_set =
			adpt_appe_vport_physical_port_id_set;
		p_adpt_api->adpt_vport_physical_port_id_get =
			adpt_appe_vport_physical_port_id_get;
		p_adpt_api->adpt_vport_state_check_set =
			adpt_appe_vport_state_check_set;
		p_adpt_api->adpt_vport_state_check_get =
			adpt_appe_vport_state_check_get;

	return SW_OK;
}

/**
 * @}
 */
