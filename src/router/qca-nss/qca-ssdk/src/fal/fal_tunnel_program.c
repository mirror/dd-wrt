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
 * @defgroup fal_tunnel_program FAL_TUNNEL_PROGRAM
 * @{
 */
#include "sw.h"
#include "fal_tunnel_program.h"
#include "hsl_api.h"
#include "adpt.h"

sw_error_t
_fal_tunnel_program_entry_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_entry_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_entry_add(dev_id, type, entry);
    return rv;
}

sw_error_t
_fal_tunnel_program_entry_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_entry_del(dev_id, type, entry);
    return rv;
}

sw_error_t
_fal_tunnel_program_entry_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_entry_getfirst)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_entry_getfirst(dev_id, type, entry);
    return rv;
}

sw_error_t
_fal_tunnel_program_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_entry_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_entry_getnext(dev_id, type, entry);
    return rv;
}

sw_error_t
_fal_tunnel_program_cfg_set(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_cfg_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_cfg_set(dev_id, type, cfg);
    return rv;
}

sw_error_t
_fal_tunnel_program_cfg_get(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_cfg_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_cfg_get(dev_id, type, cfg);
    return rv;
}

sw_error_t
_fal_tunnel_program_udf_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_udf_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_udf_add(dev_id, type, udf);
    return rv;
}

sw_error_t
_fal_tunnel_program_udf_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_udf_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_udf_del(dev_id, type, udf);
    return rv;
}

sw_error_t
_fal_tunnel_program_udf_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_udf_getfirst)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_udf_getfirst(dev_id, type, udf);
    return rv;
}

sw_error_t
_fal_tunnel_program_udf_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_tunnel_program_udf_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_tunnel_program_udf_getnext(dev_id, type, udf);
    return rv;
}

/**
 * @brief Add one tunnel program entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[in] entry tunnel program entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_entry_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_entry_add(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Delete one tunnel program entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[in] entry tunnel program entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_entry_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_entry_del(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get first tunnel program entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[out] entry tunnel program entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_entry_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_entry_getfirst(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get next tunnel program entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[in|out] entry tunnel program entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_entry_getnext(dev_id, type, entry);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set the tunnel program configurations
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[in] cfg tunnel program configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_cfg_set(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_cfg_set(dev_id, type, cfg);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get the tunnel program configurations
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[out] cfg tunnel program configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_cfg_get(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_cfg_get(dev_id, type, cfg);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Add one tunnel program udf entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[in] udf tunnel program udf entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_udf_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_udf_add(dev_id, type, udf);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Delete one tunnel program udf entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[in] udf tunnel program udf entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_udf_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_udf_del(dev_id, type, udf);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get first tunnel program udf entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[out] udf tunnel program udf entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_udf_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_udf_getfirst(dev_id, type, udf);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get next tunnel program udf entry
 * @param[in] dev_id device id
 * @param[in] type tunnel program type
 * @param[in|out] udf tunnel program udf entry
 * @return SW_OK or error code
 */
sw_error_t
fal_tunnel_program_udf_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_tunnel_program_udf_getnext(dev_id, type, udf);
    FAL_API_UNLOCK;
    return rv;
}

EXPORT_SYMBOL(fal_tunnel_program_entry_add);
EXPORT_SYMBOL(fal_tunnel_program_entry_del);
EXPORT_SYMBOL(fal_tunnel_program_entry_getfirst);
EXPORT_SYMBOL(fal_tunnel_program_entry_getnext);
EXPORT_SYMBOL(fal_tunnel_program_cfg_set);
EXPORT_SYMBOL(fal_tunnel_program_cfg_get);
EXPORT_SYMBOL(fal_tunnel_program_udf_add);
EXPORT_SYMBOL(fal_tunnel_program_udf_del);
EXPORT_SYMBOL(fal_tunnel_program_udf_getfirst);
EXPORT_SYMBOL(fal_tunnel_program_udf_getnext);
