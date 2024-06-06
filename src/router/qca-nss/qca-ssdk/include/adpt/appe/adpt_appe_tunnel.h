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

#ifndef _ADPT_APPE_TUNNEL_H_
#define _ADPT_APPE_TUNNEL_H_

sw_error_t
adpt_appe_tunnel_decap_entry_add(a_uint32_t dev_id,
		fal_tunnel_op_mode_t add_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
adpt_appe_tunnel_decap_entry_del(a_uint32_t dev_id,
		fal_tunnel_op_mode_t del_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
adpt_appe_tunnel_decap_entry_get(a_uint32_t dev_id,
		fal_tunnel_op_mode_t get_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
adpt_appe_tunnel_decap_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_op_mode_t next_mode, fal_tunnel_decap_entry_t *value);
sw_error_t
adpt_appe_tunnel_decap_entry_flush(a_uint32_t dev_id);
sw_error_t
adpt_appe_tunnel_encap_header_ctrl_set(a_uint32_t dev_id,
		fal_tunnel_encap_header_ctrl_t *header_ctrl);
sw_error_t
adpt_appe_tunnel_encap_header_ctrl_get(a_uint32_t dev_id,
		fal_tunnel_encap_header_ctrl_t *header_ctrl);
sw_error_t
#ifndef IN_TUNNEL_MINI
adpt_appe_tunnel_encap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value);
sw_error_t
adpt_appe_tunnel_encap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value);
sw_error_t
adpt_appe_tunnel_decap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action);
sw_error_t
adpt_appe_tunnel_decap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action);
#endif
sw_error_t
adpt_appe_tunnel_global_cfg_get(a_uint32_t dev_id, fal_tunnel_global_cfg_t *cfg);
sw_error_t
adpt_appe_tunnel_global_cfg_set(a_uint32_t dev_id, fal_tunnel_global_cfg_t *cfg);
sw_error_t
adpt_appe_tunnel_port_intf_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_tunnel_port_intf_t *port_cfg);
sw_error_t
adpt_appe_tunnel_port_intf_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_tunnel_port_intf_t *port_cfg);
sw_error_t
#ifndef IN_TUNNEL_MINI
adpt_appe_tunnel_vlan_intf_add(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg);
sw_error_t
adpt_appe_tunnel_vlan_intf_getfirst(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg);
sw_error_t
adpt_appe_tunnel_vlan_intf_getnext(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg);
sw_error_t
adpt_appe_tunnel_vlan_intf_del(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg);
#endif
sw_error_t
adpt_appe_tunnel_intf_set(a_uint32_t dev_id, a_uint32_t l3_if, fal_tunnel_intf_t *intf_t);
sw_error_t
adpt_appe_tunnel_intf_get(a_uint32_t dev_id, a_uint32_t l3_if, fal_tunnel_intf_t *intf_t);
sw_error_t
adpt_appe_tunnel_encap_port_tunnelid_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
adpt_appe_tunnel_encap_port_tunnelid_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
adpt_appe_tunnel_encap_intf_tunnelid_set(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
adpt_appe_tunnel_encap_intf_tunnelid_get(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id);
sw_error_t
adpt_appe_tunnel_encap_entry_add(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg);
sw_error_t
adpt_appe_tunnel_encap_entry_del(a_uint32_t dev_id, a_uint32_t tunnel_id);
sw_error_t
adpt_appe_tunnel_encap_entry_get(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg);
sw_error_t
adpt_appe_tunnel_encap_entry_getnext(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg);
sw_error_t
adpt_appe_tunnel_encap_rule_entry_set(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry);
sw_error_t
adpt_appe_tunnel_encap_rule_entry_get(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry);
sw_error_t
adpt_appe_tunnel_encap_rule_entry_del(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry);
sw_error_t
adpt_appe_tunnel_exp_decap_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);
sw_error_t
adpt_appe_tunnel_exp_decap_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable);
#endif
