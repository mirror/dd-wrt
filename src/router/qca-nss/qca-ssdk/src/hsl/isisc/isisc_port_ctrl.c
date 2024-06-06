/*
 * Copyright (c) 2012, 2015-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup isisc_port_ctrl ISISC_PORT_CONTROL
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_port_prop.h"
#include "isisc_port_ctrl.h"
#include "isisc_reg.h"
#include "hsl_phy.h"

#if defined(MHT)
#include "mht_port_ctrl.h"
#endif

a_bool_t
_isisc_port_phy_connected(a_uint32_t dev_id, fal_port_t port_id)
{
	ssdk_chip_type chip_type = CHIP_UNSPECIFIED;
	a_uint32_t cpu_bmp = BIT(6) | BIT(0);

	chip_type = hsl_get_current_chip_type(dev_id);
#if defined(MHT)
	if (chip_type == CHIP_MHT)
		return hsl_port_phy_connected(dev_id, port_id);
#endif

	if (cpu_bmp & BIT(port_id))
		return A_FALSE;
	else
		return A_TRUE;
}

static sw_error_t
_isisc_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
                      fal_port_duplex_t duplex)
{
    sw_error_t rv;
    a_uint32_t reg_save, reg_val = 0, force, tmp;
    a_bool_t status;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (FAL_DUPLEX_BUTT <= duplex)
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg_val), sizeof (a_uint32_t));
    SW_GET_FIELD_BY_REG(PORT_STATUS, DUPLEX_MODE, tmp, reg_val);

    /* for those ports without PHY device we set MAC register */
    if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
    {
        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN, 0, reg_val);
        if (FAL_HALF_DUPLEX == duplex)
        {
            if (tmp == 0)
                return SW_OK;
            SW_SET_REG_BY_FIELD(PORT_STATUS, DUPLEX_MODE, 0, reg_val);
        }
        else
        {
            if (tmp == 1)
                return SW_OK;
            SW_SET_REG_BY_FIELD(PORT_STATUS, DUPLEX_MODE, 1, reg_val);
        }
        reg_save = reg_val;
    }
    else
    {
        rv = hsl_port_phy_duplex_get (dev_id, port_id, &tmp);
        SW_RTN_ON_ERROR(rv);
        rv = hsl_port_phy_autoneg_status_get(dev_id, port_id, &status);
        SW_RTN_ON_ERROR(rv);
        if ((tmp == duplex) && (status == A_FALSE))
            return SW_OK;
        reg_save = reg_val;
        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN, 0, reg_val);
        SW_SET_REG_BY_FIELD(PORT_STATUS, RXMAC_EN, 0, reg_val);
        SW_SET_REG_BY_FIELD(PORT_STATUS, TXMAC_EN, 0, reg_val);

        HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                          (a_uint8_t *) (&reg_val), sizeof (a_uint32_t));

        rv = hsl_port_phy_duplex_set (dev_id, port_id, duplex);
        SW_RTN_ON_ERROR(rv);

        /* If MAC not in sync with PHY mode, the behavior is undefine.
           You must be careful... */
        SW_GET_FIELD_BY_REG(PORT_STATUS, LINK_EN, force, reg_save);
        if (!force)
        {
            if (FAL_HALF_DUPLEX == duplex)
            {
                SW_SET_REG_BY_FIELD(PORT_STATUS, DUPLEX_MODE, 0, reg_save);
            }
            else
            {
                SW_SET_REG_BY_FIELD(PORT_STATUS, DUPLEX_MODE, 1, reg_save);
            }
        }
    }

    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg_save), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
                     fal_port_speed_t speed)
{
    sw_error_t rv;
    a_uint32_t reg_save, reg_val = 0, force, tmp;
    a_bool_t status;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (FAL_SPEED_1000 < speed)
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg_val), sizeof (a_uint32_t));
    SW_GET_FIELD_BY_REG(PORT_STATUS, SPEED_MODE, tmp, reg_val);

    /* for those ports without PHY device we set MAC register */
    if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
    {
        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN, 0, reg_val);
        if (FAL_SPEED_10 == speed)
        {
            if (tmp == 0)
                return SW_OK;
            SW_SET_REG_BY_FIELD(PORT_STATUS, SPEED_MODE, 0, reg_val);
        }
        else if (FAL_SPEED_100 == speed)
        {
            if (tmp == 1)
                return SW_OK;
            SW_SET_REG_BY_FIELD(PORT_STATUS, SPEED_MODE, 1, reg_val);
        }
        else
        {
            if (tmp == 2)
                return SW_OK;
            SW_SET_REG_BY_FIELD(PORT_STATUS, SPEED_MODE, 2, reg_val);
        }
        reg_save = reg_val;

    }
    else
    {
        rv = hsl_port_phy_speed_get (dev_id, port_id, &tmp);
        SW_RTN_ON_ERROR(rv);
        rv = hsl_port_phy_autoneg_status_get(dev_id, port_id, &status);
        SW_RTN_ON_ERROR(rv);
        if ((tmp == speed) && (status == A_FALSE))
            return SW_OK;
        reg_save = reg_val;
        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN,  0, reg_val);
        SW_SET_REG_BY_FIELD(PORT_STATUS, RXMAC_EN, 0, reg_val);
        SW_SET_REG_BY_FIELD(PORT_STATUS, TXMAC_EN, 0, reg_val);

        HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                          (a_uint8_t *) (&reg_val), sizeof (a_uint32_t));


        rv = hsl_port_phy_speed_set (dev_id, port_id, speed);
        SW_RTN_ON_ERROR(rv);

        /* If MAC not in sync with PHY mode, the behavior is undefine.
           You must be careful... */
        SW_GET_FIELD_BY_REG(PORT_STATUS, LINK_EN, force, reg_save);
        if (!force)
        {
            if (FAL_SPEED_10 == speed)
            {
                SW_SET_REG_BY_FIELD(PORT_STATUS, SPEED_MODE, 0, reg_save);
            }
            else if (FAL_SPEED_100 == speed)
            {
                SW_SET_REG_BY_FIELD(PORT_STATUS, SPEED_MODE, 1, reg_save);
            }
            else
            {
                SW_SET_REG_BY_FIELD(PORT_STATUS, SPEED_MODE, 2, reg_save);
            }
        }
    }

    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg_save), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_flowctrl_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val, force, reg = 0, tmp;

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(PORT_STATUS, FLOW_LINK_EN, force, reg);
    if (force)
    {
        /* flow control isn't in force mode so can't set */
        return SW_DISABLE;
    }
	tmp = reg;

    SW_SET_REG_BY_FIELD(PORT_STATUS, RX_FLOW_EN, val, reg);
    SW_SET_REG_BY_FIELD(PORT_STATUS, TX_FLOW_EN, val, reg);
    SW_SET_REG_BY_FIELD(PORT_STATUS, TX_HALF_FLOW_EN, val, reg);
	if (tmp == reg)
		return SW_OK;

    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_flowctrl_forcemode_set(a_uint32_t dev_id, fal_port_t port_id,
                                  a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t reg = 0, tmp;

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);
	SW_GET_FIELD_BY_REG(PORT_STATUS, FLOW_LINK_EN, tmp, reg);

    if (A_TRUE == enable)
    {
		if (tmp == 0)
			return SW_OK;
        SW_SET_REG_BY_FIELD(PORT_STATUS, FLOW_LINK_EN, 0, reg);
    }
    else if (A_FALSE == enable)
    {
        /* for those ports without PHY, it can't sync flow control status */
        if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
        {
            return SW_DISABLE;
        }
		if (tmp == 1)
			return SW_OK;
        SW_SET_REG_BY_FIELD(PORT_STATUS, FLOW_LINK_EN, 1, reg);
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
                     fal_port_speed_t * pspeed)
{
	sw_error_t rv = SW_OK;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
	/* for those ports without PHY device supposed always 1000Mbps */
	if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
	{
		*pspeed = FAL_SPEED_1000;
	}
	else
	{
		rv = hsl_port_phy_speed_get(dev_id, port_id, pspeed);
	}
	return rv;
}
static sw_error_t
_isisc_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
                      fal_port_duplex_t * pduplex)
{
	sw_error_t rv = SW_OK;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}

	/* for those ports without PHY device supposed always full */
	if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
	{
		*pduplex = FAL_FULL_DUPLEX;
	}
	else
	{
		rv = hsl_port_phy_duplex_get(dev_id, port_id, pduplex);
	}
	return rv;
}

static sw_error_t
_isisc_port_flowctrl_get(a_uint32_t dev_id, fal_port_t port_id,
                        a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t rx, reg = 0;

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(PORT_STATUS, RX_FLOW_EN, rx, reg);

    if (1 == rx)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}

static sw_error_t
_isisc_port_flowctrl_forcemode_get(a_uint32_t dev_id, fal_port_t port_id,
                                  a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t force, reg;

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(PORT_STATUS, FLOW_LINK_EN, force, reg);
    if (0 == force)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}

static sw_error_t
_isisc_port_rxhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                          fal_port_header_mode_t mode)
{
    sw_error_t rv;
    a_uint32_t val = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (FAL_NO_HEADER_EN == mode)
    {
        val = 0;
    }
    else if (FAL_ONLY_MANAGE_FRAME_EN == mode)
    {
        val = 1;
    }
    else if (FAL_ALL_TYPE_FRAME_EN == mode)
    {
        val = 2;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_HDR_CTL, port_id, RXHDR_MODE,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
static sw_error_t
_isisc_port_rxhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                          fal_port_header_mode_t * mode)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_HDR_CTL, port_id, RXHDR_MODE,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (1 == val)
    {
        *mode = FAL_ONLY_MANAGE_FRAME_EN;
    }
    else if (2 == val)
    {
        *mode = FAL_ALL_TYPE_FRAME_EN;
    }
    else
    {
        *mode = FAL_NO_HEADER_EN;
    }

    return SW_OK;
}
#endif
static sw_error_t
_isisc_port_txhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                          fal_port_header_mode_t mode)
{
    sw_error_t rv;
    a_uint32_t val = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (FAL_NO_HEADER_EN == mode)
    {
        val = 0;
    }
    else if (FAL_ONLY_MANAGE_FRAME_EN == mode)
    {
        val = 1;
    }
    else if (FAL_ALL_TYPE_FRAME_EN == mode)
    {
        val = 2;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_HDR_CTL, port_id, TXHDR_MODE,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
static sw_error_t
_isisc_port_txhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                          fal_port_header_mode_t * mode)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_HDR_CTL, port_id, TXHDR_MODE,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (1 == val)
    {
        *mode = FAL_ONLY_MANAGE_FRAME_EN;
    }
    else if (2 == val)
    {
        *mode = FAL_ALL_TYPE_FRAME_EN;
    }
    else
    {
        *mode = FAL_NO_HEADER_EN;
    }

    return SW_OK;
}
#endif
static sw_error_t
_isisc_header_type_set(a_uint32_t dev_id, a_bool_t enable, a_uint32_t type)
{
    a_uint32_t reg = 0;
    sw_error_t rv;

    HSL_DEV_ID_CHECK(dev_id);

    HSL_REG_ENTRY_GET(rv, dev_id, HEADER_CTL, 0,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (A_TRUE == enable)
    {
        if (0xffff < type)
        {
            return SW_BAD_PARAM;
        }
        SW_SET_REG_BY_FIELD(HEADER_CTL, TYPE_LEN, 1, reg);
        SW_SET_REG_BY_FIELD(HEADER_CTL, TYPE_VAL, type, reg);
    }
    else if (A_FALSE == enable)
    {
        SW_SET_REG_BY_FIELD(HEADER_CTL, TYPE_LEN, 0, reg);
        SW_SET_REG_BY_FIELD(HEADER_CTL, TYPE_VAL, 0, reg);
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_SET(rv, dev_id, HEADER_CTL, 0,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
static sw_error_t
_isisc_header_type_get(a_uint32_t dev_id, a_bool_t * enable, a_uint32_t * type)
{
    a_uint32_t data, reg = 0;
    sw_error_t rv;

    HSL_DEV_ID_CHECK(dev_id);

    HSL_REG_ENTRY_GET(rv, dev_id, HEADER_CTL, 0,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(HEADER_CTL, TYPE_LEN, data, reg);
    if (data)
    {
        SW_GET_FIELD_BY_REG(HEADER_CTL, TYPE_VAL, data, reg);
        *enable = A_TRUE;
        *type = data;
    }
    else
    {
        *enable = A_FALSE;
        *type = 0;
    }

    return SW_OK;
}
#endif
static sw_error_t
_isisc_port_txmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t reg, force, val = 0, tmp;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }
	tmp = reg;

    /* for those ports without PHY device we set MAC register */
    if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
    {
        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN,  0, reg);
        SW_SET_REG_BY_FIELD(PORT_STATUS, TXMAC_EN, val, reg);
    }
    else
    {
        SW_GET_FIELD_BY_REG(PORT_STATUS, LINK_EN,  force, reg);
        if (force)
        {
            /* link isn't in force mode so can't set */
            return SW_DISABLE;
        }
        else
        {
            SW_SET_REG_BY_FIELD(PORT_STATUS, TXMAC_EN, val, reg);
        }
    }
	if (tmp == reg)
		return SW_OK;
    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
static sw_error_t
_isisc_port_txmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_STATUS, port_id, TXMAC_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}
#endif
static sw_error_t
_isisc_port_rxmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t reg = 0, force, val = 0, tmp;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }
	tmp = reg;

    /* for those ports without PHY device we set MAC register */
    if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
    {
        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN,  0, reg);
        SW_SET_REG_BY_FIELD(PORT_STATUS, RXMAC_EN, val, reg);
    }
    else
    {
        SW_GET_FIELD_BY_REG(PORT_STATUS, LINK_EN,  force, reg);
        if (force)
        {
            /* link isn't in force mode so can't set */
            return SW_DISABLE;
        }
        else
        {
            SW_SET_REG_BY_FIELD(PORT_STATUS, RXMAC_EN, val, reg);
        }
    }
	if (tmp == reg)
		return SW_OK;
    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
static sw_error_t
_isisc_port_rxmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_STATUS, port_id, RXMAC_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}
#endif
static sw_error_t
_isisc_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val, reg = 0, force, tmp;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);
	tmp = reg;

    /* for those ports without PHY device we set MAC register */
    if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
    {
        SW_SET_REG_BY_FIELD(PORT_STATUS, FLOW_LINK_EN, 0, reg);
        SW_SET_REG_BY_FIELD(PORT_STATUS, TX_FLOW_EN,   val, reg);
    }
    else
    {
        SW_GET_FIELD_BY_REG(PORT_STATUS, FLOW_LINK_EN, force, reg);
        if (force)
        {
            /* flow control isn't in force mode so can't set */
            return SW_DISABLE;
        }
        else
        {
            SW_SET_REG_BY_FIELD(PORT_STATUS, TX_FLOW_EN, val, reg);
        }
    }
	if (tmp == reg)
		return SW_OK;
    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_txfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_STATUS, port_id, TX_FLOW_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}

static sw_error_t
_isisc_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val = 0, reg, force, tmp;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);
	tmp = reg;

    /* for those ports without PHY device we set MAC register */
    if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
    {
        SW_SET_REG_BY_FIELD(PORT_STATUS, FLOW_LINK_EN, 0, reg);
        SW_SET_REG_BY_FIELD(PORT_STATUS, RX_FLOW_EN,   val, reg);
    }
    else
    {
        SW_GET_FIELD_BY_REG(PORT_STATUS, FLOW_LINK_EN,  force, reg);
        if (force)
        {
            /* flow control isn't in force mode so can't set */
            return SW_DISABLE;
        }
        else
        {
            SW_SET_REG_BY_FIELD(PORT_STATUS, RX_FLOW_EN, val, reg);
        }
    }
	if ( tmp == reg)
		return SW_OK;
    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_rxfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_STATUS, port_id, RX_FLOW_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}
static sw_error_t
_isisc_port_link_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * status)
{
    sw_error_t rv;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

   /* for those ports without PHY device supposed always link up */
   if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
   {
	   *status = A_TRUE;
   }
   else
   {
        rv = hsl_port_phy_link_status_get(dev_id, port_id, status);
        SW_RTN_ON_ERROR(rv);
   }

    return SW_OK;
}

static sw_error_t
_isisc_port_link_forcemode_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t reg = 0, tmp = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(PORT_STATUS, LINK_EN, tmp, reg);

    if (A_TRUE == enable)
    {
        if(tmp == 0)
          return SW_OK;

        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN, 0, reg);
    }
    else if (A_FALSE == enable)
    {
        if(tmp == 1)
          return SW_OK;

        /* for those ports without PHY, it can't sync link status */
        if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
        {
            return SW_DISABLE;
        }
        SW_SET_REG_BY_FIELD(PORT_STATUS, LINK_EN, 1, reg);
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_link_forcemode_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_STATUS, port_id, LINK_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (0 == val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}

#define ISISC_LPI_PORT1_OFFSET 4
#define ISISC_LPI_BIT_STEP     2

static sw_error_t
_isisc_port_interface_eee_cfg_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_eee_cfg_t *port_eee_cfg)
{
    sw_error_t rv = SW_OK;
    a_uint32_t reg = 0, field = 0, offset = 0, device_id = 0, rev_id = 0, reverse = 0;
    a_uint32_t eee_mask = 0, adv = 0;

    if(port_eee_cfg->enable)
        adv = port_eee_cfg->advertisement;
    else
        adv = 0;
    rv = hsl_port_phy_eee_adv_set(dev_id, port_id, adv);
    SW_RTN_ON_ERROR (rv);
    HSL_REG_ENTRY_GET(rv, dev_id, MASK_CTL, 0,
                        (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(MASK_CTL, DEVICE_ID, device_id, reg);
    SW_GET_FIELD_BY_REG(MASK_CTL, REV_ID, rev_id, reg);
    switch (device_id) {
        case S17C_DEVICE_ID:
            eee_mask = 1;
            if (rev_id == 0)
                reverse = 1;
            else
                reverse = 0;
            break;
        case MHT_DEVICE_ID:
            eee_mask = 3;
            reverse = 0;
            break;
        default:
            return SW_NOT_SUPPORTED;
    }

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, EEE_CTL, 0,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (A_TRUE == port_eee_cfg->lpi_tx_enable)
    {
        field  = eee_mask;
    }
    else
    {
        field  = 0;
    }

    if (reverse)
    {
        field = (~field) & eee_mask;
    }

    offset = (port_id - 1) * ISISC_LPI_BIT_STEP + ISISC_LPI_PORT1_OFFSET;
    reg &= (~(eee_mask << offset));
    reg |= (field << offset);

    HSL_REG_ENTRY_SET(rv, dev_id, EEE_CTL, 0,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_interface_eee_cfg_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_eee_cfg_t *port_eee_cfg)
{
    sw_error_t rv;
    a_uint32_t reg = 0, field, offset, device_id, rev_id, reverse = 0;
    a_uint32_t eee_mask = 0, adv = 0, lp_adv = 0, cap = 0, status = 0;

    rv = hsl_port_phy_eee_adv_get(dev_id, port_id, &adv);
    SW_RTN_ON_ERROR (rv);
    port_eee_cfg->advertisement = adv;
    rv = hsl_port_phy_eee_partner_adv_get(dev_id, port_id, &lp_adv);
    SW_RTN_ON_ERROR (rv);
    port_eee_cfg->link_partner_advertisement = lp_adv;
    rv = hsl_port_phy_eee_cap_get(dev_id, port_id, &cap);
    SW_RTN_ON_ERROR (rv);
    port_eee_cfg->capability = cap;
    rv = hsl_port_phy_eee_status_get(dev_id, port_id, &status);
    SW_RTN_ON_ERROR (rv);

    port_eee_cfg->eee_status = status;
    if (port_eee_cfg->advertisement) {
        port_eee_cfg->enable = A_TRUE;
    } else {
        port_eee_cfg->enable = A_FALSE;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, MASK_CTL, 0,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(MASK_CTL, DEVICE_ID, device_id, reg);
    SW_GET_FIELD_BY_REG(MASK_CTL, REV_ID, rev_id, reg);
    switch (device_id) {
        case S17C_DEVICE_ID:
            eee_mask = 1;
            if (rev_id == 0)
                reverse = 1;
            else
                reverse = 0;
            break;
        case MHT_DEVICE_ID:
            eee_mask = 3;
            reverse = 0;
            break;
        default:
            return SW_NOT_SUPPORTED;
    }

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_ENTRY_GET(rv, dev_id, EEE_CTL, 0,
                      (a_uint8_t *) (&reg), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    offset = (port_id - 1) * ISISC_LPI_BIT_STEP + ISISC_LPI_PORT1_OFFSET;
    field = (reg >> offset) & eee_mask;

    if (reverse)
    {
        field = (~field) & eee_mask;
    }

    if (field == eee_mask)
    {
        port_eee_cfg->lpi_tx_enable = A_TRUE;
    }
    else
    {
       port_eee_cfg->lpi_tx_enable = A_FALSE;
    }

    return SW_OK;
}

#ifndef IN_PORTCONTROL_MINI
static sw_error_t
_isisc_port_bp_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val = 0, tmp;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }
	HSL_REG_FIELD_GET(rv, dev_id, PORT_STATUS, port_id, TX_HALF_FLOW_EN,
                      (a_uint8_t *) (&tmp), sizeof (a_uint32_t));
	if (tmp == val)
		return SW_OK;

    HSL_REG_FIELD_SET(rv, dev_id, PORT_STATUS, port_id, TX_HALF_FLOW_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_bp_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_STATUS, port_id, TX_HALF_FLOW_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (val)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return SW_OK;
}


static sw_error_t
_isisc_ports_link_status_get(a_uint32_t dev_id, a_uint32_t * status)
{
	a_uint32_t port_id = 0;
	hsl_dev_t *pdev = NULL;
	a_uint32_t port_bmp[SW_MAX_NR_DEV] = {0};
	HSL_DEV_ID_CHECK(dev_id);

	pdev = hsl_dev_ptr_get(dev_id);
	if (pdev == NULL)
	    return SW_NOT_INITIALIZED;

	port_bmp[dev_id] = qca_ssdk_phy_type_port_bmp_get(dev_id, F1_PHY_CHIP);

	*status = 0x0;
	for (port_id = 0; port_id < pdev->nr_ports; port_id++)
	{
		if (port_id >= SW_MAX_NR_PORT)
			break;
			/* for those ports without PHY device supposed always link up */
		if (A_FALSE == _isisc_port_phy_connected(dev_id, port_id))
		{
			*status |= (0x1 << port_id);
		}
		else
		{
			a_bool_t link = A_FALSE;
			sw_error_t rv = SW_OK;

			rv = hsl_port_phy_link_status_get(dev_id, port_id, &link);
			SW_RTN_ON_ERROR(rv);
			if (A_TRUE == link)
			{
				*status |= (0x1 << port_id);
			}
			else
			{
				*status &= ~(0x1 << port_id);
			}
		}
	}
	return SW_OK;
}

static sw_error_t
_isisc_port_mac_loopback_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t val = 0;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    if (A_TRUE == enable)
    {
        val = 1;
    }
    else if (A_FALSE == enable)
    {
        val = 0;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_SET(rv, dev_id, PORT_HDR_CTL, port_id, LOOPBACK_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    return rv;
}

static sw_error_t
_isisc_port_mac_loopback_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
    sw_error_t rv;
    a_uint32_t val;

    HSL_DEV_ID_CHECK(dev_id);

    if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
    {
        return SW_BAD_PARAM;
    }

    HSL_REG_FIELD_GET(rv, dev_id, PORT_HDR_CTL, port_id, LOOPBACK_EN,
                      (a_uint8_t *) (&val), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (0 == val)
    {
        *enable = A_FALSE;
    }
    else
    {
        *enable = A_TRUE;
    }

    return SW_OK;
}
#endif
/**
 * @brief Set duplex mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] duplex duplex mode
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
                     fal_port_duplex_t duplex)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_duplex_set(dev_id, port_id, duplex);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set speed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] speed port speed
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
                    fal_port_speed_t speed)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_speed_set(dev_id, port_id, speed);
    HSL_API_UNLOCK;
    return rv;
}
/**
 * @brief Get duplex mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] duplex duplex mode
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
                     fal_port_duplex_t * pduplex)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_duplex_get(dev_id, port_id, pduplex);
    HSL_API_UNLOCK;
    return rv;
}



/**
 * @brief Get speed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] speed port speed
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
                    fal_port_speed_t * pspeed)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_speed_get(dev_id, port_id, pspeed);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get flow control status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_flowctrl_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_flowctrl_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get flow control force mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_flowctrl_forcemode_get(a_uint32_t dev_id, fal_port_t port_id,
                                 a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_flowctrl_forcemode_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set flow control(rx/tx/bp) status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_flowctrl_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_flowctrl_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set flow control force mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_flowctrl_forcemode_set(a_uint32_t dev_id, fal_port_t port_id,
                                 a_bool_t enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_flowctrl_forcemode_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set status of Atheros header packets parsed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_rxhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                         fal_port_header_mode_t mode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_rxhdr_mode_set(dev_id, port_id, mode);
    HSL_API_UNLOCK;
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Get status of Atheros header packets parsed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_rxhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                         fal_port_header_mode_t * mode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_rxhdr_mode_get(dev_id, port_id, mode);
    HSL_API_UNLOCK;
    return rv;
}
#endif
/**
 * @brief Set status of Atheros header packets parsed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_txhdr_mode_set(a_uint32_t dev_id, fal_port_t port_id,
                         fal_port_header_mode_t mode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_txhdr_mode_set(dev_id, port_id, mode);
    HSL_API_UNLOCK;
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Get status of Atheros header packets parsed on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_txhdr_mode_get(a_uint32_t dev_id, fal_port_t port_id,
                         fal_port_header_mode_t * mode)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_txhdr_mode_get(dev_id, port_id, mode);
    HSL_API_UNLOCK;
    return rv;
}
#endif
/**
 * @brief Set status of Atheros header type value on a particular device.
 * @param[in] dev_id device id
 * @param[in] enable A_TRUE or A_FALSE
 * @param[in] type header type value
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_header_type_set(a_uint32_t dev_id, a_bool_t enable, a_uint32_t type)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_header_type_set(dev_id, enable, type);
    HSL_API_UNLOCK;
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Get status of Atheros header type value on a particular device.
 * @param[in] dev_id device id
 * @param[out] enable A_TRUE or A_FALSE
 * @param[out] type header type value
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_header_type_get(a_uint32_t dev_id, a_bool_t * enable, a_uint32_t * type)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_header_type_get(dev_id, enable, type);
    HSL_API_UNLOCK;
    return rv;
}
#endif
/**
 * @brief Set status of txmac on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_txmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_port_txmac_status_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Get status of txmac on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_txmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_txmac_status_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}
#endif
/**
 * @brief Set status of rxmac on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_rxmac_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_port_rxmac_status_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Get status of rxmac on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_rxmac_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_rxmac_status_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}
#endif
/**
 * @brief Set status of tx flow control on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_port_txfc_status_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get status of tx flow control on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_txfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_txfc_status_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set status of rx flow control on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_port_rxfc_status_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set status of rx flow control on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_rxfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_rxfc_status_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}
/**
 * @brief Get link status on particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] status link status up (A_TRUE) or down (A_FALSE)
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_link_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * status)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_link_status_get(dev_id, port_id, status);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set link force mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_link_forcemode_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_port_link_forcemode_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get link force mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_link_forcemode_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_link_forcemode_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
  * @brief Set 802.3az status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] port_eee_cfg port eee cfg
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_interface_eee_cfg_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_eee_cfg_t *port_eee_cfg)
{
    sw_error_t rv;

    HSL_API_LOCK;
   rv = _isisc_port_interface_eee_cfg_set(dev_id, port_id, port_eee_cfg);
    HSL_API_UNLOCK;
    return rv;
}

/**
  * @brief Get 802.3az status on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] port_eee_cfg port eee cfg
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_interface_eee_cfg_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_eee_cfg_t *port_eee_cfg)

{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_interface_eee_cfg_get(dev_id, port_id, port_eee_cfg);
    HSL_API_UNLOCK;
    return rv;
}
#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Set status of back pressure on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_bp_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_port_bp_status_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set status of back pressure on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_bp_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_bp_status_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get link status on all ports.
 * @param[in] dev_id device id
 * @param[out] status link status bitmap and bit 0 for port 0, bi 1 for port 1, ..., etc.
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_ports_link_status_get(a_uint32_t dev_id, a_uint32_t * status)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_ports_link_status_get(dev_id, status);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Set mac loop back on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_mac_loopback_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
    sw_error_t rv;
    HSL_API_LOCK;
    rv = _isisc_port_mac_loopback_set(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get mac loop back on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
isisc_port_mac_loopback_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _isisc_port_mac_loopback_get(dev_id, port_id, enable);
    HSL_API_UNLOCK;
    return rv;
}
#endif
sw_error_t
isisc_port_ctrl_init(a_uint32_t dev_id)
{
    HSL_DEV_ID_CHECK(dev_id);

#ifndef HSL_STANDALONG
    {
        hsl_api_t *p_api;
        ssdk_chip_type chip_type = CHIP_UNSPECIFIED;

        chip_type = hsl_get_current_chip_type(dev_id);
        SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

        if (chip_type == CHIP_ISISC) {
            p_api->port_flowctrl_set = isisc_port_flowctrl_set;
            p_api->port_flowctrl_forcemode_set = isisc_port_flowctrl_forcemode_set;
            p_api->port_txfc_status_get = isisc_port_txfc_status_get;
            p_api->port_txfc_status_set = isisc_port_txfc_status_set;
            p_api->port_rxfc_status_set = isisc_port_rxfc_status_set;
            p_api->port_rxfc_status_get = isisc_port_rxfc_status_get;
            p_api->port_duplex_get = isisc_port_duplex_get;
            p_api->port_speed_get = isisc_port_speed_get;
            p_api->port_duplex_set = isisc_port_duplex_set;
            p_api->port_speed_set = isisc_port_speed_set;
            p_api->port_flowctrl_get = isisc_port_flowctrl_get;
            p_api->port_flowctrl_forcemode_get = isisc_port_flowctrl_forcemode_get;
        }
#if defined(MHT)
        else if (chip_type == CHIP_MHT) {
            p_api->port_flowctrl_set = mht_port_flowctrl_set;
            p_api->port_flowctrl_forcemode_set = mht_port_flowctrl_forcemode_set;
            p_api->port_txfc_status_get = mht_port_txfc_status_get;
            p_api->port_txfc_status_set = mht_port_txfc_status_set;
            p_api->port_rxfc_status_set = mht_port_rxfc_status_set;
            p_api->port_rxfc_status_get = mht_port_rxfc_status_get;
            p_api->port_duplex_get = mht_port_duplex_get;
            p_api->port_speed_get = mht_port_speed_get;
            p_api->port_duplex_set = mht_port_duplex_set;
            p_api->port_speed_set = mht_port_speed_set;
            p_api->port_flowctrl_get = mht_port_flowctrl_get;
            p_api->port_flowctrl_forcemode_get = mht_port_flowctrl_forcemode_get;
        }
#endif
#ifndef IN_PORTCONTROL_MINI
        p_api->port_rxhdr_mode_get = isisc_port_rxhdr_mode_get;
        p_api->port_txhdr_mode_get = isisc_port_txhdr_mode_get;
        p_api->header_type_get = isisc_header_type_get;
        p_api->port_txmac_status_get = isisc_port_txmac_status_get;
        p_api->port_rxmac_status_get = isisc_port_rxmac_status_get;
#endif
        p_api->port_interface_eee_cfg_get = isisc_port_interface_eee_cfg_get;
        p_api->port_interface_eee_cfg_set = isisc_port_interface_eee_cfg_set;
        p_api->port_rxhdr_mode_set = isisc_port_rxhdr_mode_set;
        p_api->port_txhdr_mode_set = isisc_port_txhdr_mode_set;
        p_api->header_type_set = isisc_header_type_set;
        p_api->port_txmac_status_set = isisc_port_txmac_status_set;
        p_api->port_rxmac_status_set = isisc_port_rxmac_status_set;
        p_api->port_link_status_get = isisc_port_link_status_get;
        p_api->port_link_forcemode_set = isisc_port_link_forcemode_set;
        p_api->port_link_forcemode_get = isisc_port_link_forcemode_get;
#ifndef IN_PORTCONTROL_MINI
        p_api->port_bp_status_set = isisc_port_bp_status_set;
        p_api->port_bp_status_get = isisc_port_bp_status_get;
        p_api->ports_link_status_get = isisc_ports_link_status_get;
        p_api->port_mac_loopback_set=isisc_port_mac_loopback_set;
        p_api->port_mac_loopback_get=isisc_port_mac_loopback_get;
#endif

#if defined(MHT)
#ifndef IN_PORTCONTROL_MINI
	p_api->port_congestion_drop_set = mht_port_congestion_drop_set;
	p_api->port_congestion_drop_get = mht_port_congestion_drop_get;
#endif
	p_api->port_flowctrl_thresh_set = mht_port_flowctrl_thresh_set;
	p_api->port_flowctrl_thresh_get = mht_port_flowctrl_thresh_get;
#ifndef IN_PORTCONTROL_MINI
	p_api->ring_flow_ctrl_thres_set = mht_ring_flow_ctrl_thres_set;
	p_api->ring_flow_ctrl_thres_get = mht_ring_flow_ctrl_thres_get;
	p_api->ring_flow_ctrl_status_get = mht_ring_flow_ctrl_status_get;
	p_api->ring_union_set = mht_ring_union_set;
	p_api->ring_union_get = mht_ring_union_get;
	p_api->ring_flow_ctrl_set = mht_ring_flow_ctrl_config_set;
	p_api->ring_flow_ctrl_get = mht_ring_flow_ctrl_config_get;
#endif
	p_api->port_erp_power_mode_set= mht_port_erp_power_mode_set;
#endif

    }
#endif

    return SW_OK;
}

/**
 * @}
 */

