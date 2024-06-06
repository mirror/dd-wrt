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
#include "sw.h"
#include "adpt.h"
#include "appe_l2_vp.h"
#include "appe_l2_vp_reg.h"

sw_error_t
adpt_appe_fdb_vport_maclimit_ctrl_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_maclimit_ctrl_t * maclimit_ctrl)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	port_id = FAL_PORT_ID_VALUE(port_id);
	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.lrn_lmt_en = maclimit_ctrl->enable;
	l2_vp_port_tbl.bf.lrn_lmt_cnt = maclimit_ctrl->limit_num;
	l2_vp_port_tbl.bf.lrn_lmt_exceed_fwd = maclimit_ctrl->action;

	return appe_l2_vp_port_tbl_set(dev_id, port_id, &l2_vp_port_tbl);
}

sw_error_t
adpt_appe_fdb_vport_maclimit_ctrl_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_maclimit_ctrl_t * maclimit_ctrl)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(maclimit_ctrl);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	maclimit_ctrl->enable = l2_vp_port_tbl.bf.lrn_lmt_en;
	maclimit_ctrl->limit_num = l2_vp_port_tbl.bf.lrn_lmt_cnt;
	maclimit_ctrl->action = l2_vp_port_tbl.bf.lrn_lmt_exceed_fwd;

	return SW_OK;
}

sw_error_t
adpt_appe_vport_fdb_learn_limit_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable, a_uint32_t cnt)
{
	sw_error_t rv = SW_OK;
	fal_maclimit_ctrl_t maclimit_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = adpt_appe_fdb_vport_maclimit_ctrl_get(dev_id, port_id, &maclimit_ctrl);
	SW_RTN_ON_ERROR(rv);

	maclimit_ctrl.enable = enable;
	maclimit_ctrl.limit_num = cnt;

	return adpt_appe_fdb_vport_maclimit_ctrl_set(dev_id, port_id, &maclimit_ctrl);
}

sw_error_t
adpt_appe_vport_fdb_learn_limit_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * enable, a_uint32_t * cnt)
{
	sw_error_t rv = SW_OK;
	fal_maclimit_ctrl_t maclimit_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);
	ADPT_NULL_POINT_CHECK(cnt);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = adpt_appe_fdb_vport_maclimit_ctrl_get(dev_id, port_id, &maclimit_ctrl);
	SW_RTN_ON_ERROR(rv);

	*enable = maclimit_ctrl.enable;
	*cnt = maclimit_ctrl.limit_num;

	return SW_OK;
}

sw_error_t
adpt_appe_fdb_vport_learn_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);

	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.new_addr_lrn_en = enable;
	l2_vp_port_tbl.bf.station_move_lrn_en = enable;

	return appe_l2_vp_port_tbl_set(dev_id, port_id, &l2_vp_port_tbl);
}

sw_error_t
adpt_appe_fdb_vport_learn_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	*enable = l2_vp_port_tbl.bf.new_addr_lrn_en;

	return SW_OK;
}

sw_error_t
adpt_appe_fdb_vport_newaddr_lrn_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable,
	fal_fwd_cmd_t cmd)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.new_addr_lrn_en = enable;
	l2_vp_port_tbl.bf.new_addr_fwd_cmd = cmd;

	return appe_l2_vp_port_tbl_set(dev_id, port_id, &l2_vp_port_tbl);
}

sw_error_t
adpt_appe_fdb_vport_newaddr_lrn_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable,
	fal_fwd_cmd_t *cmd)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);
	ADPT_NULL_POINT_CHECK(cmd);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	*enable = l2_vp_port_tbl.bf.new_addr_lrn_en;
	*cmd = l2_vp_port_tbl.bf.new_addr_fwd_cmd;

	return SW_OK;
}

sw_error_t
adpt_appe_fdb_vport_stamove_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable,
	fal_fwd_cmd_t cmd)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.station_move_lrn_en = enable;
	l2_vp_port_tbl.bf.station_move_fwd_cmd = cmd;

	return appe_l2_vp_port_tbl_set(dev_id, port_id, &l2_vp_port_tbl);
}

sw_error_t
adpt_appe_fdb_vport_stamove_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable,
	fal_fwd_cmd_t *cmd)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);
	ADPT_NULL_POINT_CHECK(cmd);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	*enable = l2_vp_port_tbl.bf.station_move_lrn_en;
	*cmd = l2_vp_port_tbl.bf.station_move_fwd_cmd;

	return SW_OK;
}

sw_error_t
adpt_appe_vport_fdb_learn_counter_get(a_uint32_t dev_id, fal_port_t port_id,
	a_uint32_t * cnt)
{
	sw_error_t rv = SW_OK;
	union vp_lrn_limit_counter_u vp_lrn_limit_counter = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cnt);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = appe_vp_lrn_limit_counter_get (dev_id, port_id, &vp_lrn_limit_counter);
	SW_RTN_ON_ERROR(rv);

	*cnt = vp_lrn_limit_counter.bf.lrn_cnt;

	return SW_OK;
}

sw_error_t
adpt_appe_vport_fdb_learn_exceed_cmd_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_fwd_cmd_t cmd)
{
	sw_error_t rv = SW_OK;
	fal_maclimit_ctrl_t maclimit_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = adpt_appe_fdb_vport_maclimit_ctrl_get(dev_id, port_id, &maclimit_ctrl);
	SW_RTN_ON_ERROR(rv);

	maclimit_ctrl.action = cmd;

	return adpt_appe_fdb_vport_maclimit_ctrl_set(dev_id, port_id, &maclimit_ctrl);
}

sw_error_t
adpt_appe_vport_fdb_learn_exceed_cmd_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_fwd_cmd_t * cmd)
{
	sw_error_t rv = SW_OK;
	fal_maclimit_ctrl_t maclimit_ctrl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cmd);
	port_id = FAL_PORT_ID_VALUE(port_id);

	rv = adpt_appe_fdb_vport_maclimit_ctrl_get(dev_id, port_id, &maclimit_ctrl);
	SW_RTN_ON_ERROR(rv);

	*cmd = maclimit_ctrl.action;

	return SW_OK;
}
