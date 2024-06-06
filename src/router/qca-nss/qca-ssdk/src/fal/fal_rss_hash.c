/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup fal_rss_hash FAL_RSS_HASH
 * @{
 */
#include "sw.h"
#include "fal_rss_hash.h"
#include "hsl_api.h"
#include "adpt.h"

#include <linux/kernel.h>
#include <linux/module.h>


/**
 * @}
 */
sw_error_t
_fal_rss_hash_config_set(a_uint32_t dev_id, fal_rss_hash_mode_t mode, fal_rss_hash_config_t * config)
{
    sw_error_t rv;
    adpt_api_t *p_api;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_rss_hash_config_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_rss_hash_config_set(dev_id, mode, config);
    return rv;
}

sw_error_t
_fal_rss_hash_config_get(a_uint32_t dev_id, fal_rss_hash_mode_t mode, fal_rss_hash_config_t * config)
{
    sw_error_t rv;
    adpt_api_t *p_api;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_rss_hash_config_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_rss_hash_config_get(dev_id, mode, config);
    return rv;
}

sw_error_t
_fal_toeplitz_hash_secret_key_set(a_uint32_t dev_id, fal_toeplitz_secret_key_t *secret_key)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_toeplitz_hash_secret_key_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_toeplitz_hash_secret_key_set(dev_id, secret_key);
    return rv;
}

sw_error_t
_fal_toeplitz_hash_secret_key_get(a_uint32_t dev_id, fal_toeplitz_secret_key_t *secret_key)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_toeplitz_hash_secret_key_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_toeplitz_hash_secret_key_get(dev_id, secret_key);
    return rv;
}

sw_error_t
_fal_rsshash_algm_set(a_uint32_t dev_id, fal_rss_hash_algm_t *rsshash_algm)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_rsshash_algm_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_rsshash_algm_set(dev_id, rsshash_algm);
    return rv;
}

sw_error_t
_fal_rsshash_algm_get(a_uint32_t dev_id, fal_rss_hash_algm_t *rsshash_algm)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_rsshash_algm_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_rsshash_algm_get(dev_id, rsshash_algm);
    return rv;
}

sw_error_t
_fal_toeplitz_hash_config_add(a_uint32_t dev_id, fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_toeplitz_hash_config_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_toeplitz_hash_config_add(dev_id, toeplitz_cfg);
    return rv;
}

sw_error_t
_fal_toeplitz_hash_config_del(a_uint32_t dev_id, fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_toeplitz_hash_config_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_toeplitz_hash_config_del(dev_id, toeplitz_cfg);
    return rv;
}

sw_error_t
_fal_toeplitz_hash_config_getfirst(a_uint32_t dev_id, fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_toeplitz_hash_config_getfirst)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_toeplitz_hash_config_getfirst(dev_id, toeplitz_cfg);
    return rv;
}

sw_error_t
_fal_toeplitz_hash_config_getnext(a_uint32_t dev_id, fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_toeplitz_hash_config_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_toeplitz_hash_config_getnext(dev_id, toeplitz_cfg);
    return rv;
}

sw_error_t
fal_rss_hash_config_set(a_uint32_t dev_id, fal_rss_hash_mode_t mode, fal_rss_hash_config_t * config)
{
    sw_error_t rv = SW_OK;

    FAL_RSS_HASH_API_LOCK;
    rv = _fal_rss_hash_config_set(dev_id, mode, config);
    FAL_RSS_HASH_API_UNLOCK;
    return rv;
}

sw_error_t
fal_rss_hash_config_get(a_uint32_t dev_id, fal_rss_hash_mode_t mode, fal_rss_hash_config_t * config)
{
    sw_error_t rv = SW_OK;

    FAL_RSS_HASH_API_LOCK;
    rv = _fal_rss_hash_config_get(dev_id, mode, config);
    FAL_RSS_HASH_API_UNLOCK;
    return rv;
}

sw_error_t
fal_toeplitz_hash_secret_key_set(a_uint32_t dev_id, fal_toeplitz_secret_key_t *secret_key)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_toeplitz_hash_secret_key_set(dev_id, secret_key);
    FAL_API_UNLOCK;
    return rv;
}

sw_error_t
fal_toeplitz_hash_secret_key_get(a_uint32_t dev_id, fal_toeplitz_secret_key_t *secret_key)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_toeplitz_hash_secret_key_get(dev_id, secret_key);
    FAL_API_UNLOCK;
    return rv;
}

sw_error_t
fal_rsshash_algm_set(a_uint32_t dev_id, fal_rss_hash_algm_t *rsshash_algm)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_rsshash_algm_set(dev_id, rsshash_algm);
    FAL_API_UNLOCK;
    return rv;
}

sw_error_t
fal_rsshash_algm_get(a_uint32_t dev_id, fal_rss_hash_algm_t *rsshash_algm)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_rsshash_algm_get(dev_id, rsshash_algm);
    FAL_API_UNLOCK;
    return rv;
}

sw_error_t
fal_toeplitz_hash_config_add(a_uint32_t dev_id,	fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_toeplitz_hash_config_add(dev_id, toeplitz_cfg);
    FAL_API_UNLOCK;
    return rv;
}

sw_error_t
fal_toeplitz_hash_config_del(a_uint32_t dev_id, fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_toeplitz_hash_config_del(dev_id, toeplitz_cfg);
    FAL_API_UNLOCK;
    return rv;
}

sw_error_t
fal_toeplitz_hash_config_getfirst(a_uint32_t dev_id,	fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_toeplitz_hash_config_getfirst(dev_id, toeplitz_cfg);
    FAL_API_UNLOCK;
    return rv;
}

sw_error_t
fal_toeplitz_hash_config_getnext(a_uint32_t dev_id, fal_toeplitz_hash_config_t *toeplitz_cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_toeplitz_hash_config_getnext(dev_id, toeplitz_cfg);
    FAL_API_UNLOCK;
    return rv;
}

EXPORT_SYMBOL(fal_rss_hash_config_set);
EXPORT_SYMBOL(fal_rss_hash_config_get);
EXPORT_SYMBOL(fal_toeplitz_hash_secret_key_set);
EXPORT_SYMBOL(fal_toeplitz_hash_secret_key_get);
EXPORT_SYMBOL(fal_rsshash_algm_set);
EXPORT_SYMBOL(fal_rsshash_algm_get);
EXPORT_SYMBOL(fal_toeplitz_hash_config_add);
EXPORT_SYMBOL(fal_toeplitz_hash_config_del);
