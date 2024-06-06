/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup mht_cosmap MHT_COSMAP
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_port_prop.h"
#include "mht_reg.h"

#define MHT_MAX_DSCP                63
#define MHT_MAX_UP                  7
#define MHT_MAX_PRI                 7
#define MHT_MAX_DP                  1

#define MHT_DSCP_TO_PRI             0
#define MHT_DSCP_TO_DP              1
#define MHT_UP_TO_PRI               2
#define MHT_UP_TO_DP                3

static sw_error_t
_mht_cosmap_dscp_to_ehpri_dp_set(a_uint32_t dev_id, a_uint32_t mode,
                                a_uint32_t dscp, a_uint32_t val)
{
    sw_error_t rv;
    a_uint32_t index, data = 0;

    if (MHT_MAX_DSCP < dscp)
    {
        return SW_BAD_PARAM;
    }

    index = dscp >> 3;
    HSL_REG_ENTRY_GET(rv, dev_id, DSCP_TO_EHPRI, index, (a_uint8_t *) (&data),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (MHT_DSCP_TO_PRI == mode)
    {
        if (MHT_MAX_PRI < val)
        {
            return SW_BAD_PARAM;
        }

        data &= (~(0x7 << ((dscp & 0x7) << 2)));
        data |= (val << ((dscp & 0x7) << 2));
    }
    else
    {
        if (MHT_MAX_DP < val)
        {
            return SW_BAD_PARAM;
        }

        data &= (~(0x1 << (((dscp & 0x7) << 2) + 3)));
        data |= (val << (((dscp & 0x7) << 2) + 3));
    }

    HSL_REG_ENTRY_SET(rv, dev_id, DSCP_TO_EHPRI, index, (a_uint8_t *) (&data),
                      sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_mht_cosmap_dscp_to_ehpri_dp_get(a_uint32_t dev_id, a_uint32_t mode,
                                a_uint32_t dscp, a_uint32_t * val)
{
    sw_error_t rv;
    a_uint32_t index, data = 0;

    if (MHT_MAX_DSCP < dscp)
    {
        return SW_BAD_PARAM;
    }

    index = dscp >> 3;
    HSL_REG_ENTRY_GET(rv, dev_id, DSCP_TO_EHPRI, index, (a_uint8_t *) (&data),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    data = (data >> ((dscp & 0x7) << 2)) & 0xf;
    if (MHT_DSCP_TO_PRI == mode)
    {
        *val = data & 0x7;
    }
    else
    {
        *val = (data & 0x8) >> 3;
    }

    return SW_OK;
}

static sw_error_t
_mht_cosmap_up_to_ehpri_dp_set(a_uint32_t dev_id, a_uint32_t mode, a_uint32_t up,
                              a_uint32_t val)
{
    sw_error_t rv;
    a_uint32_t data = 0;

    if (MHT_MAX_UP < up)
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, UP_TO_EHPRI, 0, (a_uint8_t *) (&data),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (MHT_UP_TO_PRI == mode)
    {
        if (MHT_MAX_PRI < val)
        {
            return SW_BAD_PARAM;
        }

        data &= (~(0x7 << (up << 2)));
        data |= (val << (up << 2));
    }
    else
    {
        if (MHT_MAX_DP < val)
        {
            return SW_BAD_PARAM;
        }

        data &= (~(0x1 << ((up << 2) + 3)));
        data |= (val << ((up << 2) + 3));
    }

    HSL_REG_ENTRY_SET(rv, dev_id, UP_TO_EHPRI, 0, (a_uint8_t *) (&data),
                      sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_mht_cosmap_up_to_ehpri_dp_get(a_uint32_t dev_id, a_uint32_t mode, a_uint32_t up,
                              a_uint32_t * val)
{
    sw_error_t rv;
    a_uint32_t data = 0;

    if (MHT_MAX_UP < up)
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, UP_TO_EHPRI, 0, (a_uint8_t *) (&data),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    data = (data >> (up << 2)) & 0xf;

    if (MHT_UP_TO_PRI == mode)
    {
        *val = (data & 0x7);
    }
    else
    {
        *val = (data & 0x8) >> 3;
    }

    return SW_OK;
}


/**
 * @brief Set dscp to internal priority mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] dscp dscp
 * @param[in] pri internal priority
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_dscp_to_ehpri_set(a_uint32_t dev_id, a_uint32_t dscp, a_uint32_t pri)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_dscp_to_ehpri_dp_set(dev_id, MHT_DSCP_TO_PRI, dscp, pri);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get dscp to internal priority mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] dscp dscp
 * @param[out] pri internal priority
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_dscp_to_ehpri_get(a_uint32_t dev_id, a_uint32_t dscp,
                            a_uint32_t * pri)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_dscp_to_ehpri_dp_get(dev_id, MHT_DSCP_TO_PRI, dscp, pri);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set dscp to internal drop precedence mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] dscp dscp
 * @param[in] dp internal drop precedence
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_dscp_to_ehdp_set(a_uint32_t dev_id, a_uint32_t dscp, a_uint32_t dp)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_dscp_to_ehpri_dp_set(dev_id, MHT_DSCP_TO_DP, dscp, dp);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get dscp to internal drop precedence mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] dscp dscp
 * @param[out] dp internal drop precedence
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_dscp_to_ehdp_get(a_uint32_t dev_id, a_uint32_t dscp, a_uint32_t * dp)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_dscp_to_ehpri_dp_get(dev_id, MHT_DSCP_TO_DP, dscp, dp);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set dot1p to internal priority mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] up dot1p
 * @param[in] pri internal priority
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_up_to_ehpri_set(a_uint32_t dev_id, a_uint32_t up, a_uint32_t pri)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_up_to_ehpri_dp_set(dev_id, MHT_UP_TO_PRI, up, pri);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get dot1p to internal priority mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] up dot1p
 * @param[out] pri internal priority
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_up_to_ehpri_get(a_uint32_t dev_id, a_uint32_t up, a_uint32_t * pri)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_up_to_ehpri_dp_get(dev_id, MHT_UP_TO_PRI, up, pri);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set dot1p to internal drop precedence mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] up dot1p
 * @param[in] dp internal drop precedence
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_up_to_ehdp_set(a_uint32_t dev_id, a_uint32_t up, a_uint32_t dp)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_up_to_ehpri_dp_set(dev_id, MHT_UP_TO_DP, up, dp);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get dot1p to internal drop precedence mapping on one particular device for WAN port.
 * @param[in] dev_id device id
 * @param[in] up dot1p
 * @param[in] dp internal drop precedence
 * @return SW_OK or error code
 */
sw_error_t
mht_cosmap_up_to_ehdp_get(a_uint32_t dev_id, a_uint32_t up, a_uint32_t * dp)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_cosmap_up_to_ehpri_dp_get(dev_id, MHT_UP_TO_DP, up, dp);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @}
 */

