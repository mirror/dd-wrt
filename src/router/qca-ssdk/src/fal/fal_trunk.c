/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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
 * @defgroup fal_trunk FAL_TRUNK
 * @{
 */
#include "sw.h"
#include "fal_trunk.h"
#include "hsl_api.h"

static sw_error_t
_fal_trunk_group_set(a_uint32_t dev_id, a_uint32_t trunk_id,
                     a_bool_t enable, fal_pbmp_t member)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->trunk_group_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->trunk_group_set(dev_id, trunk_id, enable, member);
    return rv;
}

static sw_error_t
_fal_trunk_group_get(a_uint32_t dev_id, a_uint32_t trunk_id,
                     a_bool_t * enable, fal_pbmp_t * member)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->trunk_group_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->trunk_group_get(dev_id, trunk_id, enable, member);
    return rv;
}

static sw_error_t
_fal_trunk_hash_mode_set(a_uint32_t dev_id, a_uint32_t hash_mode)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->trunk_hash_mode_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->trunk_hash_mode_set(dev_id, hash_mode);
    return rv;
}

static sw_error_t
_fal_trunk_hash_mode_get(a_uint32_t dev_id, a_uint32_t * hash_mode)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->trunk_hash_mode_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->trunk_hash_mode_get(dev_id, hash_mode);
    return rv;
}

static sw_error_t
_fal_trunk_manipulate_sa_set(a_uint32_t dev_id, fal_mac_addr_t * addr)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->trunk_manipulate_sa_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->trunk_manipulate_sa_set(dev_id, addr);
    return rv;
}

static sw_error_t
_fal_trunk_manipulate_sa_get(a_uint32_t dev_id, fal_mac_addr_t * addr)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->trunk_manipulate_sa_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->trunk_manipulate_sa_get(dev_id, addr);
    return rv;
}

static sw_error_t
_fal_trunk_manipulate_dp(a_uint32_t dev_id, a_uint8_t * header, a_uint32_t len, fal_pbmp_t dp_member)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->trunk_manipulate_sa_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->trunk_manipulate_dp(dev_id, header, len, dp_member);
    return rv;
}

/**
 * @brief Set particular trunk group information on particular device.
 * @param[in] dev_id device id
 * @param[in] trunk_id trunk group id
 * @param[in] enable trunk group status, enable or disable
 * @param[in] member port member information
 * @return SW_OK or error code
 */
sw_error_t
fal_trunk_group_set(a_uint32_t dev_id, a_uint32_t trunk_id,
                    a_bool_t enable, fal_pbmp_t member)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_trunk_group_set(dev_id, trunk_id, enable, member);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get particular trunk group information on particular device.
 * @param[in] dev_id device id
 * @param[in] trunk_id trunk group id
 * @param[out] enable trunk group status, enable or disable
 * @param[out] member port member information
 * @return SW_OK or error code
 */
sw_error_t
fal_trunk_group_get(a_uint32_t dev_id, a_uint32_t trunk_id,
                    a_bool_t * enable, fal_pbmp_t * member)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_trunk_group_get(dev_id, trunk_id, enable, member);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set trunk hash mode on particular device.
 * @param[in] dev_id device id
 * @param[in] hash_mode trunk hash mode
 * @return SW_OK or error code
 */
sw_error_t
fal_trunk_hash_mode_set(a_uint32_t dev_id, a_uint32_t hash_mode)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_trunk_hash_mode_set(dev_id, hash_mode);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get trunk hash mode on particular device.
 * @param[in] dev_id device id
 * @param[out] hash_mode trunk hash mode
 * @return SW_OK or error code
 */
sw_error_t
fal_trunk_hash_mode_get(a_uint32_t dev_id, a_uint32_t * hash_mode)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_trunk_hash_mode_get(dev_id, hash_mode);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set trunk manipulate SA on particular device.
 * @param[in] dev_id device id
 * @param[in] addr   manipulate SA
 * @return SW_OK or error code
 */
sw_error_t
fal_trunk_manipulate_sa_set(a_uint32_t dev_id, fal_mac_addr_t * addr)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_trunk_manipulate_sa_set(dev_id, addr);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get trunk manipulate SA on particular device.
 * @param[in]  dev_id device id
 * @param[out] addr   manipulate SA
 * @return SW_OK or error code
 */
sw_error_t
fal_trunk_manipulate_sa_get(a_uint32_t dev_id, fal_mac_addr_t * addr)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_trunk_manipulate_sa_get(dev_id, addr);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get trunk manipulate DP on particular device.
 * @param[in]  dev_id device id
 * @param[in]  header packet pointer
 * @param[in]  len packet length
 * @param[in]  dp_member   manipulate DP
 */
sw_error_t
fal_trunk_manipulate_dp(a_uint32_t dev_id, a_uint8_t * header, a_uint32_t len, fal_pbmp_t dp_member)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_trunk_manipulate_dp(dev_id, header, len, dp_member);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @}
 */
