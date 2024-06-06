/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup fal_tunnel FAL_TUNNEL
 * @{
 */
#include "sw.h"
#include "hsl_api.h"
#include "adpt.h"
#include "fal_tunnel.h"

static sw_error_t
_fal_tunnel_encap_rule_entry_set(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_rule_entry_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_rule_entry_set(dev_id, rule_id, rule_entry);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_rule_entry_get(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_rule_entry_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_rule_entry_get(dev_id, rule_id, rule_entry);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_rule_entry_del(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_rule_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_rule_entry_del(dev_id, rule_id, rule_entry);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_header_ctrl_set(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_header_ctrl_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_header_ctrl_set(dev_id, header_ctrl);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_header_ctrl_get(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_header_ctrl_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_header_ctrl_get(dev_id, header_ctrl);
    return rv;
}

#ifndef IN_TUNNEL_MINI
sw_error_t
_fal_tunnel_decap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_ecn_mode_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_ecn_mode_set(dev_id, ecn_rule, ecn_action);
    return rv;
}

sw_error_t
_fal_tunnel_decap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_ecn_mode_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_ecn_mode_get(dev_id, ecn_rule, ecn_action);
    return rv;
}

sw_error_t
_fal_tunnel_encap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_ecn_mode_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_ecn_mode_set(dev_id, ecn_rule, ecn_value);
    return rv;
}

sw_error_t
_fal_tunnel_encap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_ecn_mode_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_ecn_mode_get(dev_id, ecn_rule, ecn_value);
    return rv;
}
#endif

sw_error_t
_fal_tunnel_decap_key_set(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *decap_key)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_key_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_key_set(dev_id, tunnel_type, decap_key);
    return rv;
}

sw_error_t
_fal_tunnel_decap_key_get(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *decap_key)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_key_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_key_get(dev_id, tunnel_type, decap_key);
    return rv;
}

sw_error_t
_fal_tunnel_decap_en_set(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t en)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_en_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_en_set(dev_id, tunnel_index, en);
    return rv;
}

sw_error_t
_fal_tunnel_decap_en_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t *en)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_en_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_en_get(dev_id, tunnel_index, en);
    return rv;
}

sw_error_t
_fal_tunnel_decap_action_update(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_tunnel_action_t *update_action)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_action_update)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_action_update(dev_id, tunnel_index, update_action);
    return rv;
}

sw_error_t
_fal_tunnel_decap_counter_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_entry_counter_t *decap_counter)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_counter_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_counter_get(dev_id, tunnel_index, decap_counter);
    return rv;
}

sw_error_t
_fal_tunnel_exp_decap_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_exp_decap_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_exp_decap_set(dev_id, port_id, enable);
    return rv;
}

sw_error_t
_fal_tunnel_exp_decap_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_exp_decap_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_exp_decap_get(dev_id, port_id, enable);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_entry_get(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_entry_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_entry_get(dev_id, tunnel_id, tunnel_encap_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_entry_getnext(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_entry_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_entry_getnext(dev_id, tunnel_id, tunnel_encap_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_entry_del(a_uint32_t dev_id, a_uint32_t tunnel_id)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_entry_del(dev_id, tunnel_id);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_entry_add(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_entry_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_entry_add(dev_id, tunnel_id, tunnel_encap_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_intf_tunnelid_set(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_intf_tunnelid_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_intf_tunnelid_set(dev_id, intf_id, tunnel_id);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_intf_tunnelid_get(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_intf_tunnelid_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_intf_tunnelid_get(dev_id, intf_id, tunnel_id);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_port_tunnelid_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_port_tunnelid_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_port_tunnelid_set(dev_id, port_id, tunnel_id);
    return rv;
}

static sw_error_t
_fal_tunnel_encap_port_tunnelid_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_encap_port_tunnelid_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_encap_port_tunnelid_get(dev_id, port_id, tunnel_id);
    return rv;
}

static sw_error_t
_fal_tunnel_intf_set(a_uint32_t dev_id, a_uint32_t l3_if, fal_tunnel_intf_t *intf_t)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_intf_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_intf_set(dev_id, l3_if, intf_t);
    return rv;
}

static sw_error_t
_fal_tunnel_intf_get(a_uint32_t dev_id, a_uint32_t l3_if, fal_tunnel_intf_t *intf_t)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_intf_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_intf_get(dev_id, l3_if, intf_t);
    return rv;
}

#ifndef IN_TUNNEL_MINI
static sw_error_t
_fal_tunnel_vlan_intf_add(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_vlan_intf_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_vlan_intf_add(dev_id, vlan_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_vlan_intf_getfirst(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_vlan_intf_getfirst)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_vlan_intf_getfirst(dev_id, vlan_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_vlan_intf_getnext(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_vlan_intf_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_vlan_intf_getnext(dev_id, vlan_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_vlan_intf_del(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_vlan_intf_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_vlan_intf_del(dev_id, vlan_cfg);
    return rv;
}
#endif

static sw_error_t
_fal_tunnel_port_intf_set(a_uint32_t dev_id, fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_port_intf_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_port_intf_set(dev_id, port_id, port_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_port_intf_get(a_uint32_t dev_id, fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_port_intf_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_port_intf_get(dev_id, port_id, port_cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_global_cfg_get(a_uint32_t dev_id, fal_tunnel_global_cfg_t *cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_global_cfg_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_global_cfg_get(dev_id, cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_global_cfg_set(a_uint32_t dev_id, fal_tunnel_global_cfg_t *cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_global_cfg_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_global_cfg_set(dev_id, cfg);
    return rv;
}

static sw_error_t
_fal_tunnel_decap_entry_add(a_uint32_t dev_id,
		fal_tunnel_op_mode_t add_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_entry_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_entry_add(dev_id, add_mode, decap_entry);
    return rv;
}

static sw_error_t
_fal_tunnel_decap_entry_get(a_uint32_t dev_id,
		fal_tunnel_op_mode_t get_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_entry_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_entry_get(dev_id, get_mode, decap_entry);
    return rv;
}

static sw_error_t
_fal_tunnel_decap_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_op_mode_t next_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_entry_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_entry_getnext(dev_id, next_mode, decap_entry);
    return rv;
}

static sw_error_t
_fal_tunnel_decap_entry_del(a_uint32_t dev_id,
		fal_tunnel_op_mode_t del_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_entry_del(dev_id, del_mode, decap_entry);
    return rv;
}

static sw_error_t
_fal_tunnel_decap_entry_flush(a_uint32_t dev_id)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_tunnel_decap_entry_flush)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_decap_entry_flush(dev_id);
    return rv;
}

sw_error_t
_fal_tunnel_udf_profile_entry_add(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_udf_profile_entry_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_udf_profile_entry_add(dev_id, profile_id, entry);
    return rv;
}

sw_error_t
_fal_tunnel_udf_profile_entry_del(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_udf_profile_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_udf_profile_entry_del(dev_id, profile_id, entry);
    return rv;
}

sw_error_t
_fal_tunnel_udf_profile_entry_getfirst(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_udf_profile_entry_getfirst)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_udf_profile_entry_getfirst(dev_id, profile_id, entry);
    return rv;
}

sw_error_t
_fal_tunnel_udf_profile_entry_getnext(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_udf_profile_entry_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_udf_profile_entry_getnext(dev_id, profile_id, entry);
    return rv;
}

sw_error_t
_fal_tunnel_udf_profile_cfg_set(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t udf_type, a_uint32_t offset)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_udf_profile_cfg_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_udf_profile_cfg_set(dev_id, profile_id, udf_idx, udf_type, offset);
    return rv;
}

sw_error_t
_fal_tunnel_udf_profile_cfg_get(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t * udf_type, a_uint32_t * offset)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_udf_profile_cfg_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_udf_profile_cfg_get(dev_id, profile_id, udf_idx, udf_type, offset);
    return rv;
}

/**
 * @brief Add the decapsulation entry
 * @param[in] dev_id device id
 * @param[in] add_mode hash or index
 * @param[in] decapsulation entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_decap_entry_add(a_uint32_t dev_id,
		fal_tunnel_op_mode_t add_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_entry_add(dev_id, add_mode, decap_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Delete the decapsulation entry
 * @param[in] dev_id device id
 * @param[in] delete mode, hash or index
 * @param[in] decapsulation entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_decap_entry_del(a_uint32_t dev_id,
		fal_tunnel_op_mode_t del_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_entry_del(dev_id, del_mode, decap_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the first decapsulation entry
 * @param[in] dev_id device id
 * @param[in] get mode, hash or index
 * @param[out] decapsulation entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_decap_entry_get(a_uint32_t dev_id,
		fal_tunnel_op_mode_t get_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_entry_get(dev_id, get_mode, decap_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the next decapsulation entry
 * @param[in] dev_id device id
 * @param[in] get mode, hash or index
 * @param[out] decapsulation entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_decap_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_op_mode_t next_mode, fal_tunnel_decap_entry_t *decap_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_entry_getnext(dev_id, next_mode, decap_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief flush the decapsulation entry
 * @param[in] dev_id device id
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_decap_entry_flush(a_uint32_t dev_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_entry_flush(dev_id);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the tunnel global config
 * @param[in] dev_id device id
 * @param[out] tunnel global config
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_global_cfg_get(a_uint32_t dev_id, fal_tunnel_global_cfg_t *cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_global_cfg_get(dev_id, cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set the tunnel global config
 * @param[in] dev_id device id
 * @param[in] tunnel global config
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_global_cfg_set(a_uint32_t dev_id, fal_tunnel_global_cfg_t *cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_global_cfg_set(dev_id, cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set the tunnel interface based on port
 * @param[in] dev_id device id
 * @param[in] port id
 * @param[in] tunnel interface config
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_port_intf_set(a_uint32_t dev_id, fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_port_intf_set(dev_id, port_id, port_cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the tunnel interface based on port
 * @param[in] dev_id device id
 * @param[in] port id
 * @param[out] tunnel interface config
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_port_intf_get(a_uint32_t dev_id, fal_port_t port_id, fal_tunnel_port_intf_t *port_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_port_intf_get(dev_id, port_id, port_cfg);
    FAL_API_UNLOCK;

    return rv;
}

#ifndef IN_TUNNEL_MINI
/**
 * @brief Add the decapsulation VLAN match entry for tunnel interface
 * @param[in] dev_id device id
 * @param[in] VLAN match entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_vlan_intf_add(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_vlan_intf_add(dev_id, vlan_cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the first decapsulation VLAN match entry for tunnel interface
 * @param[in] dev_id device id
 * @param[out] VLAN match entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_vlan_intf_getfirst(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_vlan_intf_getfirst(dev_id, vlan_cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the next decapsulation VLAN match entry for tunnel interface
 * @param[in] dev_id device id
 * @param[out] VLAN match entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_vlan_intf_getnext(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_vlan_intf_getnext(dev_id, vlan_cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Delete decapsulation VLAN match entry for tunnel interface
 * @param[in] dev_id device id
 * @param[in] VLAN match entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_vlan_intf_del(a_uint32_t dev_id, fal_tunnel_vlan_intf_t *vlan_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_vlan_intf_del(dev_id, vlan_cfg);
    FAL_API_UNLOCK;

    return rv;
}
#endif

/**
 * @brief Set decapsulation tunnel interface config
 * @param[in] dev_id device id
 * @param[in] l3 interface
 * @param[in] decapsulation interface config
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_intf_set(a_uint32_t dev_id, a_uint32_t l3_if, fal_tunnel_intf_t *intf_t)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_intf_set(dev_id, l3_if, intf_t);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get decapsulation tunnel interface config
 * @param[in] dev_id device id
 * @param[in] l3 interface
 * @param[out] decapsulation interface config
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_intf_get(a_uint32_t dev_id, a_uint32_t l3_if, fal_tunnel_intf_t *intf_t)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_intf_get(dev_id, l3_if, intf_t);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set encapsulation tunnel id based on port id
 * @param[in] dev_id device id
 * @param[in] port id
 * @param[in] tunnel id
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_port_tunnelid_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_port_tunnelid_set(dev_id, port_id, tunnel_id);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get encapsulation tunnel id based on port id
 * @param[in] dev_id device id
 * @param[in] port id
 * @param[out] tunnel id
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_port_tunnelid_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_tunnel_id_t *tunnel_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_port_tunnelid_get(dev_id, port_id, tunnel_id);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set encapsulation tunnel id based on l3 intf
 * @param[in] dev_id device id
 * @param[in] l3 interface
 * @param[in] tunnel id
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_intf_tunnelid_set(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_intf_tunnelid_set(dev_id, intf_id, tunnel_id);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get encapsulation tunnel id based on l3 intf
 * @param[in] dev_id device id
 * @param[in] l3 interface
 * @param[out] tunnel id
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_intf_tunnelid_get(a_uint32_t dev_id,
		a_uint32_t intf_id, fal_tunnel_id_t *tunnel_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_intf_tunnelid_get(dev_id, intf_id, tunnel_id);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Add encapsulation config entry
 * @param[in] dev_id device id
 * @param[in] encapsulation tunnel id
 * @param[in] encapusulation config entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_entry_add(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_entry_add(dev_id, tunnel_id, tunnel_encap_cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Delete encapsulation config entry
 * @param[in] dev_id device id
 * @param[in] encapsulation tunnel id
 * @param[in] encapusulation config entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_entry_del(a_uint32_t dev_id, a_uint32_t tunnel_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_entry_del(dev_id, tunnel_id);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the first encapsulation config entry
 * @param[in] dev_id device id
 * @param[in] encapsulation tunnel id
 * @param[out] encapusulation config entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_entry_get(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_entry_get(dev_id, tunnel_id, tunnel_encap_cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get the next encapsulation config entry
 * @param[in] dev_id device id
 * @param[in] encapsulation tunnel id
 * @param[out] encapusulation config entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_entry_getnext(a_uint32_t dev_id, a_uint32_t tunnel_id,
		fal_tunnel_encap_cfg_t *tunnel_encap_cfg)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_entry_getnext(dev_id, tunnel_id, tunnel_encap_cfg);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set encapsulation rule entry
 * @param[in] dev_id device id
 * @param[in] encapsulation rule id
 * @param[in] encapusulation rule entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_rule_entry_set(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_rule_entry_set(dev_id, rule_id, rule_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get encapsulation rule entry
 * @param[in] dev_id device id
 * @param[in] encapsulation rule id
 * @param[out] encapusulation rule entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_rule_entry_get(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_rule_entry_get(dev_id, rule_id, rule_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Delete encapsulation rule entry
 * @param[in] dev_id device id
 * @param[in] encapsulation rule id
 * @param[in] encapusulation rule entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_rule_entry_del(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_tunnel_encap_rule_t *rule_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_rule_entry_del(dev_id, rule_id, rule_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief add one udf profile entry
 * @param[in] dev_id device id
 * @param[in] profile_id udf profile id
 * @param[in] entry udf profile entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_udf_profile_entry_add(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_udf_profile_entry_add(dev_id, profile_id, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief delete one udf profile entry
 * @param[in] dev_id device id
 * @param[in] profile_id udf profile id
 * @param[in] entry udf profile entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_udf_profile_entry_del(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_udf_profile_entry_del(dev_id, profile_id, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief get first udf profile entry
 * @param[in] dev_id device id
 * @param[in] profile_id udf profile id
 * @param[out] entry udf profile entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_udf_profile_entry_getfirst(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_udf_profile_entry_getfirst(dev_id, profile_id, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief get next udf profile entry
 * @param[in] dev_id device id
 * @param[in] profile_id udf profile id
 * @param[in|out] entry udf profile entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_udf_profile_entry_getnext(a_uint32_t dev_id, a_uint32_t profile_id,
		fal_tunnel_udf_profile_entry_t * entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_udf_profile_entry_getnext(dev_id, profile_id, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief set udf base and offset cfg
 * @param[in] dev_id device id
 * @param[in] profile_id udf profile id
 * @param[in] udf_idx udf index 0-3
 * @param[in] udf_type udf base
 * @param[in] offset udf offset
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_udf_profile_cfg_set(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t udf_type, a_uint32_t offset)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_udf_profile_cfg_set(dev_id, profile_id, udf_idx, udf_type, offset);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief get udf base and offset cfg
 * @param[in] dev_id device id
 * @param[in] profile_id udf profile id
 * @param[in] udf_idx udf index 0-3
 * @param[out] udf_type udf base
 * @param[out] offset udf offset
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_udf_profile_cfg_get(a_uint32_t dev_id, a_uint32_t profile_id,
		a_uint32_t udf_idx, fal_tunnel_udf_type_t * udf_type, a_uint32_t * offset)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_udf_profile_cfg_get(dev_id, profile_id, udf_idx, udf_type, offset);
    FAL_API_UNLOCK;
    return rv;
}

#ifndef IN_TUNNEL_MINI
/**
 * @brief Set decap ECN mode
 * @param[in] dev_id device id
 * @param[in] decap ecn rule
 * @param[in] decap ecn action
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_decap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_ecn_mode_set(dev_id, ecn_rule, ecn_action);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get decap ECN mode
 * @param[in] dev_id device id
 * @param[in] decap ecn rule
 * @param[out] decap ecn action
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_decap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_decap_ecn_rule_t *ecn_rule,
		fal_tunnel_decap_ecn_action_t *ecn_action)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_ecn_mode_get(dev_id, ecn_rule, ecn_action);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set encap ECN mode
 * @param[in] dev_id device id
 * @param[in] decap ecn rule
 * @param[in] decap ecn value
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_ecn_mode_set(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_ecn_mode_set(dev_id, ecn_rule, ecn_value);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get encap ECN mode
 * @param[in] dev_id device id
 * @param[in] decap ecn rule
 * @param[in] decap ecn value
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_ecn_mode_get(a_uint32_t dev_id, fal_tunnel_encap_ecn_t *ecn_rule,
		fal_tunnel_ecn_val_t *ecn_value)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_ecn_mode_get(dev_id, ecn_rule, ecn_value);
    FAL_API_UNLOCK;

    return rv;
}
#endif

/**
 * @brief Set encap header control
 * @param[in] dev_id device id
 * @param[in] encapsulaiton header control
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_header_ctrl_set(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_header_ctrl_set(dev_id, header_ctrl);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get encap header control
 * @param[in] dev_id device id
 * @param[in] encapsulaiton header control
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_encap_header_ctrl_get(a_uint32_t dev_id, fal_tunnel_encap_header_ctrl_t *header_ctrl)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_encap_header_ctrl_get(dev_id, header_ctrl);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_exp_decap_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_exp_decap_set(dev_id, port_id, enable);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_exp_decap_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_exp_decap_get(dev_id, port_id, enable);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_decap_key_set(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *decap_key)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_key_set(dev_id, tunnel_type, decap_key);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_decap_key_get(a_uint32_t dev_id,
		fal_tunnel_type_t tunnel_type, fal_tunnel_decap_key_t *decap_key)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_key_get(dev_id, tunnel_type, decap_key);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_decap_en_set(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t en)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_en_set(dev_id, tunnel_index, en);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_decap_en_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, a_bool_t *en)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_en_get(dev_id, tunnel_index, en);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_decap_action_update(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_tunnel_action_t *update_action)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_action_update(dev_id, tunnel_index, update_action);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_tunnel_decap_counter_get(a_uint32_t dev_id,
		a_uint32_t tunnel_index, fal_entry_counter_t *decap_counter)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_tunnel_decap_counter_get(dev_id, tunnel_index, decap_counter);
    FAL_API_UNLOCK;

    return rv;
}

EXPORT_SYMBOL(fal_tunnel_decap_entry_add);
EXPORT_SYMBOL(fal_tunnel_decap_entry_del);
EXPORT_SYMBOL(fal_tunnel_decap_entry_get);
EXPORT_SYMBOL(fal_tunnel_decap_entry_getnext);
EXPORT_SYMBOL(fal_tunnel_decap_entry_flush);
EXPORT_SYMBOL(fal_tunnel_global_cfg_get);
EXPORT_SYMBOL(fal_tunnel_global_cfg_set);
EXPORT_SYMBOL(fal_tunnel_port_intf_set);
EXPORT_SYMBOL(fal_tunnel_port_intf_get);
#ifndef IN_TUNNEL_MINI
EXPORT_SYMBOL(fal_tunnel_vlan_intf_add);
EXPORT_SYMBOL(fal_tunnel_vlan_intf_getfirst);
EXPORT_SYMBOL(fal_tunnel_vlan_intf_getnext);
EXPORT_SYMBOL(fal_tunnel_vlan_intf_del);
#endif
EXPORT_SYMBOL(fal_tunnel_intf_set);
EXPORT_SYMBOL(fal_tunnel_intf_get);
EXPORT_SYMBOL(fal_tunnel_encap_port_tunnelid_set);
EXPORT_SYMBOL(fal_tunnel_encap_port_tunnelid_get);
EXPORT_SYMBOL(fal_tunnel_encap_intf_tunnelid_set);
EXPORT_SYMBOL(fal_tunnel_encap_intf_tunnelid_get);
EXPORT_SYMBOL(fal_tunnel_encap_entry_add);
EXPORT_SYMBOL(fal_tunnel_encap_entry_del);
EXPORT_SYMBOL(fal_tunnel_encap_entry_get);
EXPORT_SYMBOL(fal_tunnel_encap_entry_getnext);
EXPORT_SYMBOL(fal_tunnel_encap_rule_entry_set);
EXPORT_SYMBOL(fal_tunnel_encap_rule_entry_get);
EXPORT_SYMBOL(fal_tunnel_encap_rule_entry_del);
EXPORT_SYMBOL(fal_tunnel_udf_profile_entry_add);
EXPORT_SYMBOL(fal_tunnel_udf_profile_entry_del);
EXPORT_SYMBOL(fal_tunnel_udf_profile_entry_getfirst);
EXPORT_SYMBOL(fal_tunnel_udf_profile_entry_getnext);
EXPORT_SYMBOL(fal_tunnel_udf_profile_cfg_set);
EXPORT_SYMBOL(fal_tunnel_udf_profile_cfg_get);
#ifndef IN_TUNNEL_MINI
EXPORT_SYMBOL(fal_tunnel_decap_ecn_mode_set);
EXPORT_SYMBOL(fal_tunnel_decap_ecn_mode_get);
EXPORT_SYMBOL(fal_tunnel_encap_ecn_mode_set);
EXPORT_SYMBOL(fal_tunnel_encap_ecn_mode_get);
#endif
EXPORT_SYMBOL(fal_tunnel_encap_header_ctrl_set);
EXPORT_SYMBOL(fal_tunnel_encap_header_ctrl_get);
EXPORT_SYMBOL(fal_tunnel_exp_decap_set);
EXPORT_SYMBOL(fal_tunnel_exp_decap_get);
EXPORT_SYMBOL(fal_tunnel_decap_key_set);
EXPORT_SYMBOL(fal_tunnel_decap_key_get);
EXPORT_SYMBOL(fal_tunnel_decap_en_set);
EXPORT_SYMBOL(fal_tunnel_decap_en_get);
EXPORT_SYMBOL(fal_tunnel_decap_action_update);
EXPORT_SYMBOL(fal_tunnel_decap_counter_get);

/**
 * @}
 */
