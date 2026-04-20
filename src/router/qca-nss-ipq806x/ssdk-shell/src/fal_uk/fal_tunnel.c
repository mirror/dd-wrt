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
#include "sw_ioctl.h"
#include "fal_tunnel.h"
#include "fal_uk_if.h"

sw_error_t
fal_tunnel_intf_set(a_uint32_t dev_id,
		a_uint32_t l3_if, fal_tunnel_intf_t *intf_t)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_INTF_SET, dev_id, l3_if, intf_t);

	return rv;
}

sw_error_t
fal_tunnel_intf_get(a_uint32_t dev_id,
		a_uint32_t l3_if, fal_tunnel_intf_t *intf_t)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_INTF_GET, dev_id, l3_if, intf_t);

	return rv;
}

sw_error_t
fal_tunnel_encap_rule_entry_set(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_tunnel_encap_rule_t *rule_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_RULE_ENTRY_SET, dev_id, rule_id, rule_entry);

	return rv;
}

sw_error_t
fal_tunnel_encap_rule_entry_get(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_tunnel_encap_rule_t *rule_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_RULE_ENTRY_GET, dev_id, rule_id, rule_entry);

	return rv;
}

sw_error_t
fal_tunnel_encap_rule_entry_del(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_tunnel_encap_rule_t *rule_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_RULE_ENTRY_DEL, dev_id, rule_id, rule_entry);

	return rv;
}

sw_error_t
fal_tunnel_encap_intf_tunnelid_set(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_INTF_TUNNELID_SET, dev_id, intf_id, tunnel_id);

	return rv;
}

sw_error_t
fal_tunnel_encap_intf_tunnelid_get(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_INTF_TUNNELID_GET, dev_id, intf_id, tunnel_id);

	return rv;
}

sw_error_t
fal_tunnel_vlan_intf_add(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_VLAN_INTF_ADD, dev_id, vlan_cfg);

	return rv;
}

sw_error_t
fal_tunnel_vlan_intf_getfirst(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_VLAN_INTF_GETFIRST, dev_id, vlan_cfg);

	return rv;
}

sw_error_t
fal_tunnel_vlan_intf_getnext(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_VLAN_INTF_GETNEXT, dev_id, vlan_cfg);

	return rv;
}

sw_error_t
fal_tunnel_vlan_intf_del(a_uint32_t dev_id,
		fal_tunnel_vlan_intf_t *vlan_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_VLAN_INTF_DEL, dev_id, vlan_cfg);

	return rv;
}

sw_error_t
fal_tunnel_encap_port_tunnelid_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_PORT_TUNNELID_SET, dev_id, port_id, tunnel_id);

	return rv;
}

sw_error_t
fal_tunnel_encap_port_tunnelid_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_PORT_TUNNELID_GET, dev_id, port_id, tunnel_id);

	return rv;
}

sw_error_t
fal_tunnel_decap_entry_add(a_uint32_t dev_id,
		fal_tunnel_op_mode_t add_mode, fal_tunnel_decap_entry_t *value)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ENTRY_ADD, dev_id, add_mode, value);

	return rv;
}

sw_error_t
fal_tunnel_decap_entry_get(a_uint32_t dev_id,
		fal_tunnel_op_mode_t get_mode, fal_tunnel_decap_entry_t *value)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ENTRY_GET, dev_id, get_mode, value);

	return rv;
}

sw_error_t
fal_tunnel_decap_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_op_mode_t get_mode, fal_tunnel_decap_entry_t *value)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ENTRY_GETNEXT, dev_id, get_mode, value);

	return rv;
}

sw_error_t
fal_tunnel_decap_entry_del(a_uint32_t dev_id,
		fal_tunnel_op_mode_t del_mode, fal_tunnel_decap_entry_t *value)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ENTRY_DEL, dev_id, del_mode, value);

	return rv;
}

sw_error_t
fal_tunnel_decap_entry_flush(a_uint32_t dev_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ENTRY_FLUSH, dev_id);

	return rv;
}

sw_error_t
fal_tunnel_encap_entry_add(a_uint32_t dev_id,
		a_uint32_t tunnel_id, fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_ENTRY_ADD, dev_id, tunnel_id, tunnel_encap_cfg);

	return rv;
}

sw_error_t
fal_tunnel_encap_entry_get(a_uint32_t dev_id,
		a_uint32_t tunnel_id, fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_ENTRY_GET, dev_id, tunnel_id, tunnel_encap_cfg);

	return rv;
}

sw_error_t
fal_tunnel_encap_entry_getnext(a_uint32_t dev_id,
		a_uint32_t tunnel_id, fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_ENTRY_GETNEXT, dev_id, tunnel_id, tunnel_encap_cfg);

	return rv;
}

sw_error_t
fal_tunnel_encap_entry_del(a_uint32_t dev_id, a_uint32_t tunnel_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_ENTRY_DEL, dev_id, tunnel_id);

	return rv;
}

sw_error_t
fal_tunnel_global_cfg_set(a_uint32_t dev_id,
		fal_tunnel_global_cfg_t *cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_GLOBAL_CFG_SET, dev_id, cfg);

	return rv;
}

sw_error_t
fal_tunnel_global_cfg_get(a_uint32_t dev_id,
		fal_tunnel_global_cfg_t *cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_GLOBAL_CFG_GET, dev_id, cfg);

	return rv;
}

sw_error_t
fal_tunnel_port_intf_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_PORT_INTF_SET, dev_id, port_id, port_cfg);

	return rv;
}

sw_error_t
fal_tunnel_port_intf_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_PORT_INTF_GET, dev_id, port_id, port_cfg);

	return rv;
}

sw_error_t
fal_tunnel_decap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ECN_MODE_SET, dev_id, ecn_rule, ecn_action);

	return rv;
}

sw_error_t
fal_tunnel_decap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ECN_MODE_GET, dev_id, ecn_rule, ecn_action);

	return rv;
}

sw_error_t
fal_tunnel_encap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_ECN_MODE_GET, dev_id, ecn_rule, ecn_value);

	return rv;
}

sw_error_t
fal_tunnel_encap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_ECN_MODE_SET, dev_id, ecn_rule, ecn_value);

	return rv;
}

sw_error_t
fal_tunnel_encap_header_ctrl_set(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_HEADER_CTRL_SET, dev_id, header_ctrl);

	return rv;
}

sw_error_t
fal_tunnel_encap_header_ctrl_get(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_ENCAP_HEADER_CTRL_GET, dev_id, header_ctrl);

	return rv;
}

sw_error_t
fal_tunnel_udf_profile_entry_add(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TUNNEL_UDF_PROFILE_ENTRY_ADD, dev_id, profile_id, entry);
    return rv;
}

sw_error_t
fal_tunnel_udf_profile_entry_del(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TUNNEL_UDF_PROFILE_ENTRY_DEL, dev_id, profile_id, entry);
    return rv;
}

sw_error_t
fal_tunnel_udf_profile_entry_getfirst(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TUNNEL_UDF_PROFILE_ENTRY_GETFIRST, dev_id, profile_id, entry);
    return rv;
}

sw_error_t
fal_tunnel_udf_profile_entry_getnext(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TUNNEL_UDF_PROFILE_ENTRY_GETNEXT, dev_id, profile_id, entry);
    return rv;
}

sw_error_t
fal_tunnel_udf_profile_cfg_set(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t udf_type, a_uint32_t offset)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TUNNEL_UDF_PROFILE_CFG_SET, dev_id, profile_id,
			udf_idx, udf_type, offset);
    return rv;
}

sw_error_t
fal_tunnel_udf_profile_cfg_get(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t * udf_type, a_uint32_t * offset)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_TUNNEL_UDF_PROFILE_CFG_GET, dev_id, profile_id,
			udf_idx, udf_type, offset);
    return rv;

}

sw_error_t
fal_tunnel_exp_decap_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_EXP_DECAP_SET, dev_id, port_id, enable);

	return rv;
}

sw_error_t
fal_tunnel_exp_decap_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_EXP_DECAP_GET, dev_id, port_id, enable);

	return rv;
}

sw_error_t
fal_tunnel_decap_key_set(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *key_gen)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_KEY_SET, dev_id, tunnel_type, key_gen);

	return rv;
}

sw_error_t
fal_tunnel_decap_key_get(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *key_gen)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_KEY_GET, dev_id, tunnel_type, key_gen);

	return rv;
}

sw_error_t
fal_tunnel_decap_en_set(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t en)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_EN_SET, dev_id, tunnel_index, en);

	return rv;
}

sw_error_t
fal_tunnel_decap_en_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t *en)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_EN_GET, dev_id, tunnel_index, en);

	return rv;
}

sw_error_t
fal_tunnel_decap_action_update(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_tunnel_action_t *update_action)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_ACTION_UPDATE, dev_id, tunnel_index, update_action);

	return rv;
}

sw_error_t
fal_tunnel_decap_counter_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_entry_counter_t *decap_counter)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_TUNNEL_DECAP_COUNTER_GET, dev_id, tunnel_index, decap_counter);

	return rv;
}
