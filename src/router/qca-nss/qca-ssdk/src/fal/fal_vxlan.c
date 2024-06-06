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
 * @defgroup fal_vxlan FAL_VXLAN
 * @{
 */
#include "sw.h"
#include "fal_vxlan.h"
#include "hsl_api.h"
#include "adpt.h"

sw_error_t
_fal_vxlan_entry_add(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vxlan_entry_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vxlan_entry_add(dev_id, type, entry);
    return rv;
}

sw_error_t
_fal_vxlan_entry_del(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vxlan_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vxlan_entry_del(dev_id, type, entry);
    return rv;
}

sw_error_t
_fal_vxlan_entry_getfirst(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vxlan_entry_getfirst)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vxlan_entry_getfirst(dev_id, type, entry);
    return rv;
}

sw_error_t
_fal_vxlan_entry_getnext(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vxlan_entry_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vxlan_entry_getnext(dev_id, type, entry);
    return rv;
}

#ifndef IN_VXLAN_MINI
sw_error_t
_fal_vxlan_gpe_proto_cfg_set(a_uint32_t dev_id, fal_vxlan_gpe_proto_cfg_t * proto_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vxlan_gpe_proto_cfg_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vxlan_gpe_proto_cfg_set(dev_id, proto_cfg);
    return rv;
}

sw_error_t
_fal_vxlan_gpe_proto_cfg_get(a_uint32_t dev_id, fal_vxlan_gpe_proto_cfg_t * proto_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vxlan_gpe_proto_cfg_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vxlan_gpe_proto_cfg_get(dev_id, proto_cfg);
    return rv;
}
#endif

/**
 * @brief Add one tunnel udp entry for vxlan or vxlan-gpe
 * @param[in] dev_id device id
 * @param[in] type vxlan type
 * @param[in] entry tunnel udp entry
 * @return SW_OK or error code
 */
sw_error_t
fal_vxlan_entry_add(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_vxlan_entry_add(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Delete one tunnel udp entry for vxlan or vxlan-gpe
 * @param[in] dev_id device id
 * @param[in] type vxlan type
 * @param[in] entry tunnel udp entry
 * @return SW_OK or error code
 */
sw_error_t
fal_vxlan_entry_del(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_vxlan_entry_del(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get the first tunnel udp entry for vxlan or vxlan-gpe
 * @param[in] dev_id device id
 * @param[in] type vxlan type
 * @param[out] entry tunnel udp entry
 * @return SW_OK or error code
 */
sw_error_t
fal_vxlan_entry_getfirst(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_vxlan_entry_getfirst(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get next tunnel udp entry for vxlan or vxlan-gpe
 * @param[in] dev_id device id
 * @param[in] type vxlan type
 * @param[in/out] entry tunnel udp entry
 * @return SW_OK or error code
 */
sw_error_t
fal_vxlan_entry_getnext(a_uint32_t dev_id, fal_vxlan_type_t type, fal_tunnel_udp_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_vxlan_entry_getnext(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

#ifndef IN_VXLAN_MINI
/**
 * @brief Set the vxlan-gpe protocol configurations
 * @param[in] dev_id device id
 * @param[in] proto_cfg vxlan-gpe protocol configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_vxlan_gpe_proto_cfg_set(a_uint32_t dev_id, fal_vxlan_gpe_proto_cfg_t * proto_cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_vxlan_gpe_proto_cfg_set(dev_id, proto_cfg);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get the vxlan-gpe protocol configurations
 * @param[in] dev_id device id
 * @param[out] proto_cfg vxlan-gpe protocol configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_vxlan_gpe_proto_cfg_get(a_uint32_t dev_id, fal_vxlan_gpe_proto_cfg_t * proto_cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_vxlan_gpe_proto_cfg_get(dev_id, proto_cfg);
    FAL_API_UNLOCK;
    return rv;
}
#endif

EXPORT_SYMBOL(fal_vxlan_entry_add);
EXPORT_SYMBOL(fal_vxlan_entry_del);
EXPORT_SYMBOL(fal_vxlan_entry_getfirst);
EXPORT_SYMBOL(fal_vxlan_entry_getnext);
#ifndef IN_VXLAN_MINI
EXPORT_SYMBOL(fal_vxlan_gpe_proto_cfg_set);
EXPORT_SYMBOL(fal_vxlan_gpe_proto_cfg_get);
#endif
