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


/**
 * @defgroup fal_vport FAL_VPORT
 * @{
 */
#include "sw.h"
#include "hsl_api.h"
#include "adpt.h"


/**
 * @}
 */
sw_error_t
_fal_vport_physical_port_id_set(a_uint32_t dev_id, fal_port_t vport_id, fal_port_t phyport_id)
{
    sw_error_t rv;
    adpt_api_t *p_api;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vport_physical_port_id_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vport_physical_port_id_set(dev_id, vport_id, phyport_id);
    return rv;
}

sw_error_t
_fal_vport_physical_port_id_get(a_uint32_t dev_id, fal_port_t vport_id, fal_port_t *phyport_id)
{
    sw_error_t rv;
    adpt_api_t *p_api;

    SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

    if (NULL == p_api->adpt_vport_physical_port_id_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->adpt_vport_physical_port_id_get(dev_id, vport_id, phyport_id);
    return rv;
}

sw_error_t
_fal_vport_state_check_set(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
	sw_error_t rv;
	adpt_api_t *p_api;

	SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

	if (NULL == p_api->adpt_vport_state_check_set)
		return SW_NOT_SUPPORTED;

	rv = p_api->adpt_vport_state_check_set(dev_id, port_id, vp_state);
	return rv;
}

sw_error_t
_fal_vport_state_check_get(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
	sw_error_t rv;
	adpt_api_t *p_api;

	SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));

	if (NULL == p_api->adpt_vport_state_check_get)
		return SW_NOT_SUPPORTED;

	rv = p_api->adpt_vport_state_check_get(dev_id, port_id, vp_state);
	return rv;
}

sw_error_t
fal_vport_physical_port_id_set(a_uint32_t dev_id, fal_port_t vport_id, fal_port_t phyport_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_vport_physical_port_id_set(dev_id, vport_id, phyport_id);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_vport_physical_port_id_get(a_uint32_t dev_id, fal_port_t vport_id, fal_port_t *phyport_id)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_vport_physical_port_id_get(dev_id, vport_id, phyport_id);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_vport_state_check_set(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_vport_state_check_set(dev_id, port_id, vp_state);
    FAL_API_UNLOCK;

    return rv;
}

sw_error_t
fal_vport_state_check_get(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
    sw_error_t rv = SW_OK;

    FAL_API_LOCK;
    rv = _fal_vport_state_check_get(dev_id, port_id, vp_state);
    FAL_API_UNLOCK;

    return rv;
}

EXPORT_SYMBOL(fal_vport_physical_port_id_set);
EXPORT_SYMBOL(fal_vport_physical_port_id_get);
EXPORT_SYMBOL(fal_vport_state_check_set);
EXPORT_SYMBOL(fal_vport_state_check_get);
