/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup fal_mapt FAL_MAPT
 * @{
 */
#include "sw.h"
#include "hsl_api.h"
#include "adpt.h"
#include "fal_mapt.h"

static sw_error_t
_fal_mapt_decap_ctrl_set(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_ctrl_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_ctrl_set(dev_id, decap_ctrl);
    return rv;
}

static sw_error_t
_fal_mapt_decap_ctrl_get(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_ctrl_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_ctrl_get(dev_id, decap_ctrl);
    return rv;
}

static sw_error_t
_fal_mapt_decap_rule_entry_set(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_rule_entry_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_rule_entry_set(dev_id, rule_id, mapt_rule_entry);
    return rv;
}

static sw_error_t
_fal_mapt_decap_rule_entry_get(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_rule_entry_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_rule_entry_get(dev_id, rule_id, mapt_rule_entry);
    return rv;
}

static sw_error_t
_fal_mapt_decap_rule_entry_del(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_rule_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_rule_entry_del(dev_id, rule_id, mapt_rule_entry);
    return rv;
}

sw_error_t
_fal_mapt_decap_entry_add(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_entry_add)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_entry_add(dev_id, mapt_entry);
    return rv;
}

sw_error_t
_fal_mapt_decap_entry_del(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_entry_del)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_entry_del(dev_id, mapt_entry);
    return rv;
}

sw_error_t
_fal_mapt_decap_entry_getfirst(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_entry_getfirst)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_entry_getfirst(dev_id, mapt_entry);
    return rv;
}

sw_error_t
_fal_mapt_decap_entry_getnext(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_entry_getnext)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_entry_getnext(dev_id, mapt_entry);
    return rv;
}

sw_error_t
_fal_mapt_decap_en_set(a_uint32_t dev_id,
		a_uint32_t mapt_index, a_bool_t en)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_en_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_en_set(dev_id, mapt_index, en);
    return rv;
}

sw_error_t
_fal_mapt_decap_en_get(a_uint32_t dev_id,
		a_uint32_t mapt_index, a_bool_t *en)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    p_api = adpt_api_ptr_get(dev_id);
    SW_RTN_ON_NULL(p_api);

    if (NULL == p_api->adpt_mapt_decap_en_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_mapt_decap_en_get(dev_id, mapt_index, en);
    return rv;
}

/**
 * @brief Set mapt decap control config
 * @param[in] dev_id device id
 * @param[in] mapt decap control entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_ctrl_set(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_ctrl_set(dev_id, decap_ctrl);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get mapt decap control config
 * @param[in] dev_id device id
 * @param[out] mapt decap control entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_ctrl_get(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_ctrl_get(dev_id, decap_ctrl);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set mapt decap edit rule entry
 * @param[in] dev_id device id
 * @param[in] rule_id rule id
 * @param[in] mapt decap edit rule entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_rule_entry_set(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_rule_entry_set(dev_id, rule_id, mapt_rule_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get mapt decap edit rule entry
 * @param[in] dev_id device id
 * @param[in] rule_id rule id
 * @param[out] mapt decap edit rule entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_rule_entry_get(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_rule_entry_get(dev_id, rule_id, mapt_rule_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Delete mapt decap edit rule entry
 * @param[in] dev_id device id
 * @param[in] rule_id rule id
 * @param[in] mapt decap edit rule entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_rule_entry_del(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_rule_entry_del(dev_id, rule_id, mapt_rule_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Add mapt decap entry
 * @param[in] dev_id device id
 * @param[in] mapt decap entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_entry_add(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_entry_add(dev_id, mapt_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Delete mapt decap entry
 * @param[in] dev_id device id
 * @param[in] mapt decap entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_entry_del(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_entry_del(dev_id, mapt_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get first mapt decap entry
 * @param[in] dev_id device id
 * @param[out] mapt decap entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_entry_getfirst(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_entry_getfirst(dev_id, mapt_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get next mapt decap entry
 * @param[in] dev_id device id
 * @param[out] mapt decap entry
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_entry_getnext(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_entry_getnext(dev_id, mapt_entry);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Set mapt decap enable or not
 * @param[in] dev_id device id
 * @param[in] mapt_index mapt decap entry index
 * @param[in] en true or false
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_en_set(a_uint32_t dev_id,
		a_uint32_t mapt_index, a_bool_t en)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_en_set(dev_id, mapt_index, en);
    FAL_API_UNLOCK;

    return rv;
}

/**
 * @brief Get mapt decap enable status
 * @param[in] dev_id device id
 * @param[in] mapt_index mapt decap entry index
 * @param[out] en true or false
 * @return SW_OK or error code
 */
sw_error_t
fal_mapt_decap_en_get(a_uint32_t dev_id,
		a_uint32_t mapt_index, a_bool_t *en)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_mapt_decap_en_get(dev_id, mapt_index, en);
    FAL_API_UNLOCK;

    return rv;
}

EXPORT_SYMBOL(fal_mapt_decap_ctrl_set);
EXPORT_SYMBOL(fal_mapt_decap_ctrl_get);
EXPORT_SYMBOL(fal_mapt_decap_rule_entry_set);
EXPORT_SYMBOL(fal_mapt_decap_rule_entry_get);
EXPORT_SYMBOL(fal_mapt_decap_rule_entry_del);
EXPORT_SYMBOL(fal_mapt_decap_entry_add);
EXPORT_SYMBOL(fal_mapt_decap_entry_del);
EXPORT_SYMBOL(fal_mapt_decap_entry_getfirst);
EXPORT_SYMBOL(fal_mapt_decap_entry_getnext);
EXPORT_SYMBOL(fal_mapt_decap_en_set);
EXPORT_SYMBOL(fal_mapt_decap_en_get);
/**
 * @}
 */
