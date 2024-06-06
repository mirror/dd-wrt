/*
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
 * @defgroup fal_athtag FAL_ATHTAG
 * @{
 */
#include "sw.h"
#include "fal_athtag.h"
#include "hsl_api.h"
#include "adpt.h"

sw_error_t
_fal_athtag_pri_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t *pri_mapping)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_athtag_pri_mapping_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_athtag_pri_mapping_set(dev_id, direction, pri_mapping);
    return rv;
}

sw_error_t
_fal_athtag_pri_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t *pri_mapping)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_athtag_pri_mapping_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_athtag_pri_mapping_get(dev_id, direction, pri_mapping);
    return rv;
}

sw_error_t
_fal_athtag_port_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t *port_mapping)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_athtag_port_mapping_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_athtag_port_mapping_set(dev_id, direction, port_mapping);
    return rv;
}

sw_error_t
_fal_athtag_port_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t *port_mapping)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_athtag_port_mapping_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_athtag_port_mapping_get(dev_id, direction, port_mapping);
    return rv;
}

sw_error_t
_fal_port_athtag_rx_set(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_rx_cfg_t *cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_port_athtag_rx_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_port_athtag_rx_set(dev_id, port_id, cfg);
    return rv;
}

sw_error_t
_fal_port_athtag_rx_get(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_rx_cfg_t *cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_port_athtag_rx_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_port_athtag_rx_get(dev_id, port_id, cfg);
    return rv;
}

sw_error_t
_fal_port_athtag_tx_set(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_tx_cfg_t *cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_port_athtag_tx_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_port_athtag_tx_set(dev_id, port_id, cfg);
    return rv;
}

sw_error_t
_fal_port_athtag_tx_get(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_tx_cfg_t *cfg)
{
    adpt_api_t *p_api;
    sw_error_t rv = SW_OK;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_port_athtag_tx_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_port_athtag_tx_get(dev_id, port_id, cfg);
    return rv;
}

/**
 * @brief Set athtag priority mapping
 * @param[in] dev_id device id
 * @param[in] direction direction
 * @param[in] pri_mapping priority mapping
 * @return SW_OK or error code
 */
sw_error_t
fal_athtag_pri_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t *pri_mapping)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_athtag_pri_mapping_set(dev_id, direction, pri_mapping);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get athtag priority mapping
 * @param[in] dev_id device id
 * @param[in] direction direction
 * @param[in|out] pri_mapping priority mapping
 * @return SW_OK or error code
 */
sw_error_t
fal_athtag_pri_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t *pri_mapping)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_athtag_pri_mapping_get(dev_id, direction, pri_mapping);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set athtag port mapping
 * @param[in] dev_id device id
 * @param[in] direction direction
 * @param[in] port_mapping port mapping
 * @return SW_OK or error code
 */
sw_error_t
fal_athtag_port_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t *port_mapping)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_athtag_port_mapping_set(dev_id, direction, port_mapping);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get athtag port mapping
 * @param[in] dev_id device id
 * @param[in] direction direction
 * @param[in|out] port_mapping port mapping
 * @return SW_OK or error code
 */
sw_error_t
fal_athtag_port_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t *port_mapping)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_athtag_port_mapping_get(dev_id, direction, port_mapping);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set athtag rx configurations
 * @param[in] dev_id device id
 * @param[in] port_id port_id
 * @param[in] cfg rx configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_port_athtag_rx_set(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_rx_cfg_t *cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_port_athtag_rx_set(dev_id, port_id, cfg);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set athtag rx configurations
 * @param[in] dev_id device id
 * @param[in] port_id port_id
 * @param[out] cfg rx configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_port_athtag_rx_get(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_rx_cfg_t *cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_port_athtag_rx_get(dev_id, port_id, cfg);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set athtag tx configurations
 * @param[in] dev_id device id
 * @param[in] port_id port_id
 * @param[in] cfg tx configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_port_athtag_tx_set(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_tx_cfg_t *cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_port_athtag_tx_set(dev_id, port_id, cfg);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set athtag rx configurations
 * @param[in] dev_id device id
 * @param[in] port_id port_id
 * @param[out] cfg tx configurations
 * @return SW_OK or error code
 */
sw_error_t
fal_port_athtag_tx_get(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_tx_cfg_t *cfg)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_port_athtag_tx_get(dev_id, port_id, cfg);
    FAL_API_UNLOCK;
    return rv;
}

EXPORT_SYMBOL(fal_athtag_pri_mapping_set);
EXPORT_SYMBOL(fal_athtag_pri_mapping_get);
EXPORT_SYMBOL(fal_athtag_port_mapping_set);
EXPORT_SYMBOL(fal_athtag_port_mapping_get);
EXPORT_SYMBOL(fal_port_athtag_rx_set);
EXPORT_SYMBOL(fal_port_athtag_rx_get);
EXPORT_SYMBOL(fal_port_athtag_tx_set);
EXPORT_SYMBOL(fal_port_athtag_tx_get);
