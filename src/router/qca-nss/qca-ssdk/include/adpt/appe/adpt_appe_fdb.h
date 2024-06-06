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

#ifndef _ADPT_APPE_FDB_H_
#define _ADPT_APPE_FDB_H_

sw_error_t
adpt_appe_fdb_vport_maclimit_ctrl_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_maclimit_ctrl_t * maclimit_ctrl);
sw_error_t
adpt_appe_fdb_vport_maclimit_ctrl_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_maclimit_ctrl_t * maclimit_ctrl);
sw_error_t
adpt_appe_vport_fdb_learn_limit_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable, a_uint32_t cnt);
sw_error_t
adpt_appe_vport_fdb_learn_limit_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * enable, a_uint32_t * cnt);
sw_error_t
adpt_appe_fdb_vport_learn_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable);
sw_error_t
adpt_appe_fdb_vport_learn_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);
sw_error_t
adpt_appe_fdb_vport_newaddr_lrn_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable,
	fal_fwd_cmd_t cmd);
sw_error_t
adpt_appe_fdb_vport_newaddr_lrn_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable,
	fal_fwd_cmd_t *cmd);
sw_error_t
adpt_appe_fdb_vport_stamove_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable,
	fal_fwd_cmd_t cmd);
sw_error_t
adpt_appe_fdb_vport_stamove_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable,
	fal_fwd_cmd_t *cmd);
sw_error_t
adpt_appe_vport_fdb_learn_counter_get(a_uint32_t dev_id, fal_port_t port_id,
	a_uint32_t * cnt);
sw_error_t
adpt_appe_vport_fdb_learn_exceed_cmd_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_fwd_cmd_t cmd);
sw_error_t
adpt_appe_vport_fdb_learn_exceed_cmd_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_fwd_cmd_t * cmd);

#endif
