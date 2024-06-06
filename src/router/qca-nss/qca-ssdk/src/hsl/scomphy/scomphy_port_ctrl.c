/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*qca808x_start*/
/**
 * @defgroup scomphy_port_ctrl SCOMPHY_PORT_CONTROL
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_port_prop.h"
#include "scomphy_port_ctrl.h"
#include "hsl_phy.h"

static sw_error_t
_scomphy_port_duplex_set (a_uint32_t dev_id, fal_port_t port_id,
		       fal_port_duplex_t duplex)
{
	return hsl_port_phy_duplex_set(dev_id, port_id, duplex);
}

static sw_error_t
_scomphy_port_duplex_get (a_uint32_t dev_id, fal_port_t port_id,
		       fal_port_duplex_t * pduplex)
{
	return hsl_port_phy_duplex_get(dev_id, port_id, pduplex);
}

static sw_error_t
_scomphy_port_speed_set (a_uint32_t dev_id, fal_port_t port_id,
		      fal_port_speed_t speed)
{
	return hsl_port_phy_speed_set(dev_id, port_id, speed);
}

static sw_error_t
_scomphy_port_speed_get (a_uint32_t dev_id, fal_port_t port_id,
		      fal_port_speed_t * pspeed)
{
	return hsl_port_phy_speed_get(dev_id, port_id, pspeed);
}
#ifndef IN_PORTCONTROL_MINI
/*qca808x_end*/
static sw_error_t
_scomphy_port_combo_prefer_medium_set (a_uint32_t dev_id, fal_port_t port_id,
				    fal_port_medium_t phy_medium)
{
	return hsl_port_phy_combo_prefer_medium_set(dev_id, port_id, phy_medium);
}

static sw_error_t
_scomphy_port_combo_prefer_medium_get (a_uint32_t dev_id, fal_port_t port_id,
				    fal_port_medium_t * phy_medium)
{
	return hsl_port_phy_combo_prefer_medium_get(dev_id, port_id, phy_medium);
}

static sw_error_t
_scomphy_port_interface_mode_set (a_uint32_t dev_id, fal_port_t port_id,
				fal_port_interface_mode_t  mode)
{
	return hsl_port_phy_mode_set(dev_id, port_id, mode);
}

static sw_error_t
_scomphy_port_interface_mode_get (a_uint32_t dev_id, fal_port_t port_id,
				fal_port_interface_mode_t * mode)
{
	return hsl_port_phy_mode_get (dev_id, port_id, mode);
}
/*qca808x_start*/
static sw_error_t
_scomphy_port_interface_mode_status_get (a_uint32_t dev_id, fal_port_t port_id,
				fal_port_interface_mode_t * mode)
{
	return hsl_port_phy_interface_mode_status_get (dev_id, port_id, mode);
}
#endif

static sw_error_t
_scomphy_port_link_status_get (a_uint32_t dev_id, fal_port_t port_id,
			    a_bool_t * status)
{
	return hsl_port_phy_link_status_get(dev_id, port_id, status);
}

/**
 * @brief Set duplex mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] duplex duplex mode
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
scomphy_port_duplex_set (a_uint32_t dev_id, fal_port_t port_id,
		      fal_port_duplex_t duplex)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _scomphy_port_duplex_set (dev_id, port_id, duplex);
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
scomphy_port_duplex_get (a_uint32_t dev_id, fal_port_t port_id,
		      fal_port_duplex_t * pduplex)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _scomphy_port_duplex_get (dev_id, port_id, pduplex);
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
scomphy_port_speed_set (a_uint32_t dev_id, fal_port_t port_id,
		     fal_port_speed_t speed)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _scomphy_port_speed_set (dev_id, port_id, speed);
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
scomphy_port_speed_get (a_uint32_t dev_id, fal_port_t port_id,
		     fal_port_speed_t * pspeed)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _scomphy_port_speed_get (dev_id, port_id, pspeed);
	HSL_API_UNLOCK;
	return rv;
}
#ifndef IN_PORTCONTROL_MINI
/*qca808x_end*/
/**
 * @brief Set combo prefer medium on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] phy_medium [fiber or copper]
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
scomphy_port_combo_prefer_medium_set (a_uint32_t dev_id, a_uint32_t phy_id,
				   fal_port_medium_t phy_medium)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _scomphy_port_combo_prefer_medium_set (dev_id, phy_id, phy_medium);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get combo prefer medium on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] phy_medium [fiber or copper]
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
scomphy_port_combo_prefer_medium_get (a_uint32_t dev_id, a_uint32_t phy_id,
				   fal_port_medium_t * phy_medium)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _scomphy_port_combo_prefer_medium_get (dev_id, phy_id, phy_medium);
	HSL_API_UNLOCK;
	return rv;
}
/**
 * @brief Set phy interface mode.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] interface mode..
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
scomphy_port_interface_mode_set (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t  mode)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _scomphy_port_interface_mode_set (dev_id, port_id, mode);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set phy interface mode.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] interface mode..
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
scomphy_port_interface_mode_get (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t  * mode)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _scomphy_port_interface_mode_get (dev_id, port_id, mode);
	HSL_API_UNLOCK;
	return rv;
}
/*qca808x_start*/
/**
 * @brief Set phy interface mode status.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] interface mode..
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
scomphy_port_interface_mode_status_get (a_uint32_t dev_id, fal_port_t port_id,
			      fal_port_interface_mode_t  * mode)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _scomphy_port_interface_mode_status_get (dev_id, port_id, mode);
	HSL_API_UNLOCK;
	return rv;
}
#endif

/**
 * @brief Get link status on particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] status link status up (A_TRUE) or down (A_FALSE)
 * @return SW_OK or error code
 */
HSL_LOCAL sw_error_t
scomphy_port_link_status_get (a_uint32_t dev_id, fal_port_t port_id,
			   a_bool_t * status)
{
  sw_error_t rv;

  HSL_API_LOCK;
  rv = _scomphy_port_link_status_get (dev_id, port_id, status);
  HSL_API_UNLOCK;
  return rv;
}

sw_error_t
scomphy_port_ctrl_init(a_uint32_t dev_id)
{
	HSL_DEV_ID_CHECK(dev_id);

#ifndef HSL_STANDALONG
    {
        hsl_api_t *p_api;

        SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

	p_api->port_duplex_get = scomphy_port_duplex_get;
	p_api->port_duplex_set = scomphy_port_duplex_set;
	p_api->port_speed_get = scomphy_port_speed_get;
	p_api->port_speed_set = scomphy_port_speed_set;
#ifndef IN_PORTCONTROL_MINI
/*qca808x_end*/
	p_api->port_combo_prefer_medium_set = scomphy_port_combo_prefer_medium_set;
	p_api->port_combo_prefer_medium_get = scomphy_port_combo_prefer_medium_get;
	p_api->port_interface_mode_set = scomphy_port_interface_mode_set;
	p_api->port_interface_mode_get = scomphy_port_interface_mode_get;
/*qca808x_start*/
	p_api->port_interface_mode_status_get = scomphy_port_interface_mode_status_get;
#endif
	p_api->port_link_status_get = scomphy_port_link_status_get;
    }
#endif

    return SW_OK;
}

/**
 * @}
 */
/*qca808x_end*/
