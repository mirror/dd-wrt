/*
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup mht_port_ctrl MHT_PORT_CONTROL
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_port_prop.h"
#include "mht_port_ctrl.h"
#include "mht_reg.h"
#include "isisc_reg.h"
#include "isisc_port_ctrl.h"
#include "hsl_phy.h"
#include "ssdk_plat.h"
#include "ssdk_mht_clk.h"
#include "mht_port_ctrl.h"
#include "mht_interface_ctrl.h"
#include "ssdk_mht.h"
#include "ssdk_dts.h"
#include "ssdk_interrupt.h"

#ifndef IN_PORTCONTROL_MINI
#define PORT0_MAX_VIRT_RING	8
#define PORT5_MAX_VIRT_RING	16

/*
PORT0 egress 6 queues
PORT1~4 egress 4 queues
PORT5 egress 6 queues
*/
static a_uint32_t port_queue[6] = { 6, 4, 4, 4, 4, 6 };

static sw_error_t
_mht_port_congestion_drop_set (a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t queue_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t val = 0, offset = 0, field = 0;

	HSL_DEV_ID_CHECK (dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}

	if (queue_id >= port_queue[port_id])
	{
		return SW_BAD_PARAM;
	}

	if (port_id != 0)
		offset = port_id * 4 + 2;
	offset += queue_id;

	if (A_TRUE == enable)
	{
		field = 1 << offset;
	}
	else if (A_FALSE == enable)
	{
		field = ~(1 << offset);
	}
	else
	{
		return SW_BAD_PARAM;
	}

	HSL_REG_ENTRY_GET (rv, dev_id, FLOW_CONGE_DROP_CTRL0, 0,
			(a_uint8_t *) (&val), sizeof (a_uint32_t));
	if (A_TRUE == enable)
	{
		val = val | field;
	}
	else
	{
		val = val & field;
	}

	HSL_REG_ENTRY_SET (rv, dev_id, FLOW_CONGE_DROP_CTRL0, 0,
			(a_uint8_t *) (&val), sizeof (a_uint32_t));
	return rv;
}

static sw_error_t
_mht_port_congestion_drop_get (a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t queue_id, a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t val, offset = 0;

	HSL_DEV_ID_CHECK (dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}

	if (queue_id >= port_queue[port_id])
	{
		return SW_BAD_PARAM;
	}

	if (port_id != 0)
		offset = port_id * 4 + 2;
	offset += queue_id;

	HSL_REG_ENTRY_GET (rv, dev_id, FLOW_CONGE_DROP_CTRL0, 0,
			(a_uint8_t *) (&val), sizeof (a_uint32_t));
	val = (val >> offset) & 0x1;
	if (val == 0)
	{
		*enable = A_FALSE;
	}
	else if (val == 1)
	{
		*enable = A_TRUE;
	}
	return rv;
}

static sw_error_t
_mht_ring_flow_ctrl_thres_set (a_uint32_t dev_id, a_uint32_t ring_id,
		a_uint16_t on_thres, a_uint16_t off_thres)
{
	sw_error_t rv;
	a_uint32_t val = 0;

	HSL_DEV_ID_CHECK (dev_id);

	if (ring_id >= PORT5_MAX_VIRT_RING)
	{
		return SW_BAD_PARAM;
	}

	if (on_thres > off_thres || on_thres == 0)
	{
		return SW_BAD_PARAM;
	}

	if (ring_id < PORT0_MAX_VIRT_RING) {
		SW_SET_REG_BY_FIELD (RING_FLOW_CTRL_THRES, XON, on_thres, val);
		SW_SET_REG_BY_FIELD (RING_FLOW_CTRL_THRES, XOFF, off_thres, val);
		HSL_REG_ENTRY_SET (rv, dev_id, RING_FLOW_CTRL_THRES, ring_id,
				(a_uint8_t *) (&val), sizeof (a_uint32_t));
	} else {
		SW_SET_REG_BY_FIELD (PORT5_RING_FLOW_CTRL_THRES, XON, on_thres, val);
		SW_SET_REG_BY_FIELD (PORT5_RING_FLOW_CTRL_THRES, XOFF, off_thres, val);
		HSL_REG_ENTRY_SET (rv, dev_id, PORT5_RING_FLOW_CTRL_THRES,
				(ring_id-PORT0_MAX_VIRT_RING),
				(a_uint8_t *) (&val), sizeof (a_uint32_t));
	}

	return rv;
}

static sw_error_t
_mht_ring_flow_ctrl_thres_get (a_uint32_t dev_id, a_uint32_t ring_id,
		a_uint16_t * on_thres, a_uint16_t * off_thres)
{

	sw_error_t rv;
	a_uint32_t val = 0;
	a_uint16_t hthres, lthres;

	HSL_DEV_ID_CHECK (dev_id);

	if (ring_id >= PORT5_MAX_VIRT_RING)
	{
		return SW_BAD_PARAM;
	}

	if (ring_id < PORT0_MAX_VIRT_RING) {
		HSL_REG_ENTRY_GET (rv, dev_id, RING_FLOW_CTRL_THRES, ring_id,
				(a_uint8_t *) (&val), sizeof (a_uint32_t));

		SW_GET_FIELD_BY_REG (RING_FLOW_CTRL_THRES, XON, hthres, val);
		SW_GET_FIELD_BY_REG (RING_FLOW_CTRL_THRES, XOFF, lthres, val);
	} else {
		HSL_REG_ENTRY_GET (rv, dev_id, PORT5_RING_FLOW_CTRL_THRES,
				(ring_id-PORT0_MAX_VIRT_RING),
				(a_uint8_t *) (&val), sizeof (a_uint32_t));

		SW_GET_FIELD_BY_REG (PORT5_RING_FLOW_CTRL_THRES, XON, hthres, val);
		SW_GET_FIELD_BY_REG (PORT5_RING_FLOW_CTRL_THRES, XOFF, lthres, val);
	}

	*on_thres = hthres;
	*off_thres = lthres;

	return rv;
}

#define MHT_RX_DMA_RING_PAUSE_DEBUG_ADDR	0x28
static sw_error_t
_mht_ring_flow_ctrl_status_get(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t *status)
{

	sw_error_t rv = SW_OK;
	a_uint32_t val = 0;

	HSL_DEV_ID_CHECK (dev_id);
	if (ring_id >= PORT5_MAX_VIRT_RING) {
		return SW_BAD_PARAM;
	}

	val = MHT_RX_DMA_RING_PAUSE_DEBUG_ADDR;
	HSL_REG_ENTRY_SET(rv, dev_id, SWITCH_QM_DEBUG_ADDR, 0,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	HSL_REG_ENTRY_GET(rv, dev_id, SWITCH_QM_DEBUG_DATA, 0,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	*status = (val >> ring_id) & 1;

	return rv;
}

static sw_error_t
_mht_ring_union_set(a_uint32_t dev_id, a_bool_t enable)
{
	sw_error_t rv;
	a_uint32_t val;

	HSL_DEV_ID_CHECK(dev_id);

	val = enable ? 1 : 0;
	HSL_REG_FIELD_SET(rv, dev_id, PORT0_PORT5_RING_UNION, 0, EN,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	return rv;
}

static sw_error_t
_mht_ring_union_get(a_uint32_t dev_id, a_bool_t *enable)
{
	sw_error_t rv;
	a_uint32_t val;

	HSL_DEV_ID_CHECK(dev_id);

	HSL_REG_FIELD_GET(rv, dev_id, PORT0_PORT5_RING_UNION, 0, EN,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	*enable = val ? A_TRUE : A_FALSE;

	return rv;
}
#endif
static sw_error_t
_mht_port_flowctrl_thresh_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint16_t *on, a_uint16_t *off)
{
	sw_error_t rv;
	a_uint32_t val = 0;
	a_uint16_t hthres, lthres;

	HSL_DEV_ID_CHECK(dev_id);

	HSL_REG_ENTRY_GET(rv, dev_id, PORT_FLOC_CTRL_THRESH, port_id,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	SW_GET_FIELD_BY_REG(PORT_FLOC_CTRL_THRESH, XON, hthres, val);
	SW_GET_FIELD_BY_REG(PORT_FLOC_CTRL_THRESH, XOFF, lthres, val);

	*on = hthres;
	*off = lthres;

	return rv;
}

static sw_error_t
_mht_port_flowctrl_thresh_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint16_t on, a_uint16_t off)
{
	sw_error_t rv;
	a_uint32_t val = 0;

	HSL_DEV_ID_CHECK(dev_id);

	SW_SET_REG_BY_FIELD(PORT_FLOC_CTRL_THRESH, XON, on, val);
	SW_SET_REG_BY_FIELD(PORT_FLOC_CTRL_THRESH, XOFF, off, val);
	HSL_REG_ENTRY_SET(rv, dev_id, PORT_FLOC_CTRL_THRESH, port_id,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	return rv;
}
#ifndef IN_PORTCONTROL_MINI
static sw_error_t
_mht_ring_flow_ctrl_config_set(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t status)
{

	sw_error_t rv = SW_OK;
	a_uint32_t val = 0;

	HSL_DEV_ID_CHECK (dev_id);
	if (ring_id >= PORT5_MAX_VIRT_RING) {
		return SW_BAD_PARAM;
	}

	HSL_REG_ENTRY_GET(rv, dev_id, SWITCH_CORE_SWITCH_RING_FC, 0,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	val &= ~BIT(ring_id);
	val |= status << ring_id;

	HSL_REG_ENTRY_SET(rv, dev_id, SWITCH_CORE_SWITCH_RING_FC, 0,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));
	return rv;
}

static sw_error_t
_mht_ring_flow_ctrl_config_get(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t *status)
{

	sw_error_t rv = SW_OK;
	a_uint32_t val = 0;

	HSL_DEV_ID_CHECK (dev_id);
	if (ring_id >= PORT5_MAX_VIRT_RING) {
		return SW_BAD_PARAM;
	}

	HSL_REG_ENTRY_GET(rv, dev_id, SWITCH_CORE_SWITCH_RING_FC, 0,
			(a_uint8_t *)(&val), sizeof(a_uint32_t));

	*status = (val & BIT(ring_id)) ? A_TRUE : A_FALSE;

	return rv;
}
#endif
static sw_error_t
_mht_port_flowctrl_forcemode_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	HSL_DEV_ID_CHECK(dev_id);
	if(A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
	if (!priv)
		return SW_FAIL;
	if(!hsl_port_phy_connected(dev_id, port_id) && !enable)
		return SW_NOT_SUPPORTED;

	priv->port_tx_flowctrl_forcemode[port_id] = enable;
	priv->port_rx_flowctrl_forcemode[port_id] = enable;

    return rv;
}

static sw_error_t
__mht_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv;
	a_uint32_t val, reg = 0, tmp;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
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

	SW_SET_REG_BY_FIELD(PORT_STATUS, TX_FLOW_EN, val, reg);
	SW_SET_REG_BY_FIELD(PORT_STATUS, TX_HALF_FLOW_EN, val, reg);

	if (tmp == reg)
		return SW_OK;
	HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
				(a_uint8_t *) (&reg), sizeof (a_uint32_t));

	priv->port_old_tx_flowctrl[port_id] = enable;

	return rv;
}

static sw_error_t
_mht_port_txfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv;
	a_uint32_t val;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
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
__mht_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv;
	a_uint32_t val = 0, reg, tmp;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
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

	SW_SET_REG_BY_FIELD(PORT_STATUS, RX_FLOW_EN, val, reg);

	if ( tmp == reg)
		return SW_OK;
	HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
				(a_uint8_t *) (&reg), sizeof (a_uint32_t));

	priv->port_old_rx_flowctrl[port_id] = enable;

	return rv;
}

static sw_error_t
_mht_port_rxfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv;
	a_uint32_t val;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
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
_mht_port_mac_speed_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t speed)
{
	sw_error_t rv = 0;
	a_uint32_t reg_val = 0, tmp;
	a_uint32_t speed_val;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}

	HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
			(a_uint8_t *) (&reg_val), sizeof (a_uint32_t));
	tmp = reg_val;

	if (FAL_SPEED_10 == speed) {
		speed_val = MHT_PORT_SPEED_10M;
	} else if (FAL_SPEED_100 == speed) {
		speed_val = MHT_PORT_SPEED_100M;
	} else if (FAL_SPEED_1000 == speed) {
		speed_val = MHT_PORT_SPEED_1000M;
	} else if (FAL_SPEED_2500 == speed) {
		speed_val = MHT_PORT_SPEED_2500M;
	} else {
		SSDK_ERROR("mht port %d invalid speed\n", port_id);
		return SW_BAD_PARAM;
	}
	SW_SET_REG_BY_FIELD(PORT_STATUS, SPEED_MODE, speed_val, reg_val);

	if (tmp == reg_val)
		return SW_OK;

	HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
			(a_uint8_t *) (&reg_val), sizeof (a_uint32_t));

	return rv;
}

static sw_error_t
_mht_port_mac_dupex_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t duplex)
{
	sw_error_t rv = 0;
	a_uint32_t reg_val = 0, tmp;
	a_uint32_t duplex_val;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}

	HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
			(a_uint8_t *) (&reg_val), sizeof (a_uint32_t));
	tmp = reg_val;

	if (FAL_HALF_DUPLEX == duplex) {
		duplex_val = MHT_PORT_HALF_DUPLEX;
	} else {
		duplex_val = MHT_PORT_FULL_DUPLEX;
	}
	SW_SET_REG_BY_FIELD(PORT_STATUS, DUPLEX_MODE, duplex_val, reg_val);

	if (tmp == reg_val)
		return SW_OK;

	HSL_REG_ENTRY_SET(rv, dev_id, PORT_STATUS, port_id,
			(a_uint8_t *) (&reg_val), sizeof (a_uint32_t));

	return rv;
}

static sw_error_t
_mht_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t duplex)
{
	sw_error_t rv;
	a_uint32_t phy_id;
	hsl_phy_ops_t *phy_drv;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}

	/* for those ports without PHY device we set MAC register */
	if (A_FALSE == hsl_port_phy_connected(dev_id, port_id))
	{
		rv = _mht_port_mac_dupex_set(dev_id, port_id, duplex);
		SW_RTN_ON_ERROR (rv);
	}
	else
	{
		SW_RTN_ON_NULL(phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
		if (NULL == phy_drv->phy_duplex_set)
			return SW_NOT_SUPPORTED;
		if (FAL_DUPLEX_BUTT <= duplex)
		{
			return SW_BAD_PARAM;
		}
		rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_id);
		SW_RTN_ON_ERROR (rv);
		rv = phy_drv->phy_duplex_set(dev_id, phy_id, duplex);
		SW_RTN_ON_ERROR (rv);
	}

	return rv;
}

static sw_error_t
_mht_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t * pduplex)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_id;
	hsl_phy_ops_t *phy_drv;

	HSL_DEV_ID_CHECK (dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
	if (A_FALSE == hsl_port_phy_connected(dev_id, port_id))
	{
		a_uint32_t reg_val = 0, field;

		HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
			(a_uint8_t *) (&reg_val), sizeof (a_uint32_t));
		SW_GET_FIELD_BY_REG(PORT_STATUS, DUPLEX_MODE, field, reg_val);
		if (field)
		{
			*pduplex = FAL_FULL_DUPLEX;
		}
		else
		{
			*pduplex = FAL_HALF_DUPLEX;
		}
	}
	else
	{
		SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
		if (NULL == phy_drv->phy_duplex_get)
			return SW_NOT_SUPPORTED;
		rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_id);
		SW_RTN_ON_ERROR (rv);
		rv = phy_drv->phy_duplex_get(dev_id, phy_id, pduplex);
		SW_RTN_ON_ERROR (rv);
	}

	return rv;
}

static sw_error_t
_mht_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t speed)
{
	sw_error_t rv;
	a_uint32_t phy_id;
	hsl_phy_ops_t *phy_drv;

	HSL_DEV_ID_CHECK (dev_id);

	if(A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
	/* for those ports without PHY device we set MAC register */
	if (A_FALSE == hsl_port_phy_connected(dev_id, port_id))
	{
		rv = _mht_port_mac_speed_set(dev_id, port_id, speed);
		SW_RTN_ON_ERROR (rv);
	}
	else
	{
		SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
		if (NULL == phy_drv->phy_speed_set)
			return SW_NOT_SUPPORTED;
		if (FAL_SPEED_2500 < speed)
		{
			return SW_BAD_PARAM;
		}
		rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_id);
		SW_RTN_ON_ERROR (rv);
		rv = phy_drv->phy_speed_set(dev_id, phy_id, speed);
		SW_RTN_ON_ERROR (rv);
	}
	return rv;
}

static sw_error_t
_mht_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t * pspeed)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_id;
	hsl_phy_ops_t *phy_drv;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
	if (A_FALSE == hsl_port_phy_connected(dev_id, port_id))
	{
		a_uint32_t reg_val = 0, field;

		HSL_REG_ENTRY_GET(rv, dev_id, PORT_STATUS, port_id,
			(a_uint8_t *) (&reg_val), sizeof (a_uint32_t));
		SW_GET_FIELD_BY_REG(PORT_STATUS, SPEED_MODE, field, reg_val);
		if (0 == field)
		{
			*pspeed = FAL_SPEED_10;
		}
		else if (1 == field)
		{
			*pspeed = FAL_SPEED_100;
		}
		else if (2 == field)
		{
			*pspeed = FAL_SPEED_1000;
		}
	}
	else
	{
		SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
		if (NULL == phy_drv->phy_speed_get)
			return SW_NOT_SUPPORTED;
		rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_id);
		SW_RTN_ON_ERROR (rv);
		rv = phy_drv->phy_speed_get(dev_id, phy_id, pspeed);
		SW_RTN_ON_ERROR (rv);
	}
	return rv;
}

static sw_error_t
_mht_port_flowctrl_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
	sw_error_t rv = 0;
	a_bool_t txfc_enable, rxfc_enable;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
	    return SW_BAD_PARAM;
	}

	rv = _mht_port_txfc_status_get(dev_id, port_id, &txfc_enable);
	SW_RTN_ON_ERROR(rv);
	rv = _mht_port_rxfc_status_get(dev_id, port_id, &rxfc_enable);
	SW_RTN_ON_ERROR(rv);

	*enable = txfc_enable & rxfc_enable;

	return rv;
}

static sw_error_t
_mht_port_flowctrl_forcemode_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	HSL_DEV_ID_CHECK(dev_id);
	if(A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
	if (!priv)
		return SW_FAIL;

	*enable = (priv->port_tx_flowctrl_forcemode[port_id] &
		priv->port_rx_flowctrl_forcemode[port_id]);

	return rv;
}

#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Set flow congestion drop on a particular port queue.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] queue_id queue id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
mht_port_congestion_drop_set (a_uint32_t dev_id, fal_port_t port_id,
			       a_uint32_t queue_id, a_bool_t enable)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _mht_port_congestion_drop_set (dev_id, port_id, queue_id, enable);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set flow congestion drop on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] queue_id queue id
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
mht_port_congestion_drop_get (a_uint32_t dev_id, fal_port_t port_id,
			       a_uint32_t queue_id, a_bool_t * enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_congestion_drop_get (dev_id, port_id, queue_id, enable);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set flow ctrl thres on a particular DMA ring.
 * @param[in] dev_id device id
 * @param[in] ring_id ring id
 * @param[in] on_thres on_thres
 * @param[in] off_thres off_thres
 * @return SW_OK or error code
 */
sw_error_t
mht_ring_flow_ctrl_thres_set (a_uint32_t dev_id, a_uint32_t ring_id,
			       a_uint16_t on_thres, a_uint16_t off_thres)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _mht_ring_flow_ctrl_thres_set (dev_id, ring_id, on_thres, off_thres);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get flow ctrl thres on a particular DMA ring.
 * @param[in] dev_id device id
 * @param[in] ring_id ring id
 * @param[out] on_thres on_thres
 * @param[out] off_thres off_thres
 * @return SW_OK or error code
 */
sw_error_t
mht_ring_flow_ctrl_thres_get (a_uint32_t dev_id, a_uint32_t ring_id,
			       a_uint16_t * on_thres, a_uint16_t * off_thres)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ring_flow_ctrl_thres_get (dev_id, ring_id, on_thres, off_thres);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get flow ctrl status on a particular DMA ring.
 * @param[in] dev_id device id
 * @param[in] ring_id ring id
 * @param[out] status backpressure status
 * @return SW_OK or error code
 */
sw_error_t
mht_ring_flow_ctrl_status_get(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t *status)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ring_flow_ctrl_status_get (dev_id, ring_id, status);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set port0 port5 ring union.
 * @param[in] dev_id device id
 * @param[in] enable or not
 * @return SW_OK or error code
 */
sw_error_t
mht_ring_union_set(a_uint32_t dev_id, a_bool_t en)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _mht_ring_union_set(dev_id, en);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get port0 port5 ring union.
 * @param[in] dev_id device id
 * @param[out] enable or not
 * @return SW_OK or error code
 */
sw_error_t
mht_ring_union_get(a_uint32_t dev_id, a_bool_t *en)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _mht_ring_union_get(dev_id, en);
	HSL_API_UNLOCK;
	return rv;
}
#endif
/**
 * @brief Get flow control(rx/tx/bp) threshold on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[out] on on threshold
 * @param[out] off off threshold
 * @return SW_OK or error code
 */
sw_error_t
mht_port_flowctrl_thresh_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint16_t *on, a_uint16_t *off)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_flowctrl_thresh_get(dev_id, port_id, on, off);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set flow control(rx/tx/bp) threshold on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] on on threshold
 * @param[in] off off threshold
 * @return SW_OK or error code
 */
sw_error_t
mht_port_flowctrl_thresh_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint16_t on, a_uint16_t off)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_flowctrl_thresh_set(dev_id, port_id, on, off);
	HSL_API_UNLOCK;
	return rv;
}
#ifndef IN_PORTCONTROL_MINI
/**
 * @brief Get port 0 & port5 ring flow control status.
 * @param[in] dev_id device id
 * @param[in] ring_id ring id
 * @param[in] status enable or not
 * @return SW_OK or error code
 */
sw_error_t
mht_ring_flow_ctrl_config_set(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t status)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _mht_ring_flow_ctrl_config_set(dev_id, ring_id, status);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set port 0 & port5 ring flow control status.
 * @param[in] dev_id device id
 * @param[in] ring_id ring id
 * @param[out] status enable or not
 * @return SW_OK or error code
 */
sw_error_t
mht_ring_flow_ctrl_config_get(a_uint32_t dev_id, a_uint32_t ring_id, a_bool_t *status)
{
	sw_error_t rv;
	HSL_API_LOCK;
	rv = _mht_ring_flow_ctrl_config_get(dev_id, ring_id, status);
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
 sw_error_t
_mht_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_bool_t force_mode = A_FALSE;

	if(A_FALSE == hsl_port_phy_connected(dev_id, port_id))
	{
		rv = __mht_port_txfc_status_set(dev_id, port_id, enable);
		SW_RTN_ON_ERROR(rv);
	}
	else
	{
		/*if force mode is enabled, need to configure mac manually*/
		rv = _mht_port_flowctrl_forcemode_get(dev_id, port_id, &force_mode);
		SW_RTN_ON_ERROR (rv);
		if(force_mode)
		{
			rv = __mht_port_txfc_status_set(dev_id, port_id, enable);
			SW_RTN_ON_ERROR(rv);
		}
		rv = hsl_port_phy_txfc_set(dev_id, port_id, enable);
		SW_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
mht_port_txfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_txfc_status_set(dev_id, port_id, enable);
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
sw_error_t
mht_port_txfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_txfc_status_get(dev_id, port_id, enable);
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
 sw_error_t
_mht_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_bool_t force_mode = A_FALSE;

	if(A_FALSE == hsl_port_phy_connected(dev_id, port_id))
	{
		rv = __mht_port_rxfc_status_set(dev_id, port_id, enable);
		SW_RTN_ON_ERROR(rv);
	}
	else
	{
		/*if force mode is enabled, need to configure mac manually*/
		rv = _mht_port_flowctrl_forcemode_get(dev_id, port_id, &force_mode);
		SW_RTN_ON_ERROR (rv);
		if(force_mode)
		{
			rv = __mht_port_rxfc_status_set(dev_id, port_id, enable);
			SW_RTN_ON_ERROR(rv);
		}
		rv = hsl_port_phy_rxfc_set(dev_id, port_id, enable);
		SW_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
_mht_port_flowctrl_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_INCL_CPU))
	{
	    return SW_BAD_PARAM;
	}

	rv = _mht_port_txfc_status_set(dev_id, port_id, enable);
	SW_RTN_ON_ERROR(rv);
	rv = _mht_port_rxfc_status_set(dev_id, port_id, enable);
	SW_RTN_ON_ERROR(rv);

	return SW_OK;
}

sw_error_t
_mht_port_erp_power_mode_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_erp_power_mode_t power_mode)
{
	a_uint32_t i = 0, pbmp = 0;
	phy_info_t *phy_info = NULL;
	fal_mac_config_t mac_config = {0};
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);
	SW_RTN_ON_NULL(priv);

	switch (power_mode) {
	case FAL_ERP_LOW_POWER:
		if (hsl_port_feature_get(dev_id, port_id, PHY_F_ERP_LOW_POWER)) {
			SSDK_INFO("port %d is already in low power mode\n", port_id);
			return SW_OK;
		}

		/* off phy */
		hsl_port_phy_pll_off(dev_id, port_id);
		hsl_port_phy_power_off(dev_id, port_id);
		hsl_port_feature_set(dev_id, port_id, PHY_F_ERP_LOW_POWER);

		/* off serdes and switch core */
		pbmp = ssdk_wan_bmp_get(dev_id) | ssdk_lan_bmp_get(dev_id);
		while (pbmp) {
			if (pbmp & 1) {
				/* there is active channel port, only powr off current phy */
				if (!hsl_port_feature_get(dev_id, i, PHY_F_ERP_LOW_POWER)) {
					/* off LDO */
					hsl_port_phy_ldo_set(dev_id, port_id, A_FALSE);
					return SW_OK;
				}
			}
			pbmp >>= 1;
			i++;
		}

		/* pause intr task */
		qca_intr_work_pause(priv);

		/* manually excute polling task to finish all ports up to down sequence */
		mutex_lock(&priv->qm_lock);
		qca_mht_sw_mac_polling_task(priv);
		mutex_unlock(&priv->qm_lock);

		SSDK_DEBUG("disable manhattan switch core and serdes1\n");
		/* pause mib task */
		qca_phy_mib_work_pause(priv);
		/* disable switch core */
		SW_RTN_ON_ERROR(ssdk_mht_clk_disable(dev_id, MHT_SWITCH_CORE_CLK));

		/* switch ahb to xo */
		SW_RTN_ON_ERROR(ssdk_mht_clk_parent_set(dev_id, MHT_AHB_CLK, MHT_P_XO));
		SW_RTN_ON_ERROR(ssdk_mht_clk_rate_set(dev_id, MHT_AHB_CLK, MHT_XO_CLK_RATE_50M));
		/* assert serdes1 */
		SW_RTN_ON_ERROR(ssdk_mht_clk_assert(dev_id, MHT_SRDS1_SYS_CLK));
		/* off LDO */
		hsl_port_phy_ldo_set(dev_id, port_id, A_FALSE);
		break;
	case FAL_ERP_ACTIVE:
		if (!hsl_port_feature_get(dev_id, port_id, PHY_F_ERP_LOW_POWER)) {
			SSDK_INFO("port %d is already in active mode\n", port_id);
			return SW_OK;
		}
		/* on LDO */
		hsl_port_phy_ldo_set(dev_id, port_id, A_TRUE);
		/* resume serdes and switch core */
		if (ssdk_mht_clk_is_asserted(dev_id, MHT_SRDS1_SYS_CLK)) {
			SSDK_DEBUG("configure manhattan serdes1 and enable switch core\n");
			/* configure serdes1 as sgmii plus mode */
			phy_info = hsl_phy_info_get(dev_id);
			phy_info->port_mode[SSDK_PHYSICAL_PORT0] = PORT_SGMII_PLUS;
			mac_config.mac_mode = FAL_MAC_MODE_SGMII_PLUS;
			mac_config.config.sgmii.clock_mode = FAL_INTERFACE_CLOCK_MAC_MODE;
			mac_config.config.sgmii.auto_neg = A_FALSE;
			mac_config.config.sgmii.force_speed = FAL_SPEED_2500;
			SW_RTN_ON_ERROR(mht_interface_mac_mode_set(dev_id,
					SSDK_PHYSICAL_PORT0, &mac_config));

			/* switch ahb back to serdes1 */
			SW_RTN_ON_ERROR(ssdk_mht_clk_parent_set(dev_id,
					MHT_AHB_CLK, MHT_P_UNIPHY1_TX312P5M));
			SW_RTN_ON_ERROR(ssdk_mht_clk_rate_set(dev_id,
					MHT_AHB_CLK, MHT_AHB_CLK_RATE_104P17M));

			/* enable switch core */
			SW_RTN_ON_ERROR(ssdk_mht_clk_enable(dev_id, MHT_SWITCH_CORE_CLK));
			/* resume mib task */
			qca_phy_mib_work_resume(priv);

			/* resume intr task */
			qca_intr_work_resume(priv);
		}
		/* on phy */
		hsl_port_phy_pll_on(dev_id, port_id);
		hsl_port_phy_power_on(dev_id, port_id);
		hsl_port_feature_clear(dev_id, port_id, PHY_F_ERP_LOW_POWER);
		break;
	default:
		SSDK_ERROR("not support power mode %d\n", power_mode);
		return SW_NOT_SUPPORTED;
	}
	return SW_OK;
}

sw_error_t
mht_port_rxfc_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_rxfc_status_set(dev_id, port_id, enable);
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
sw_error_t
mht_port_rxfc_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_rxfc_status_get(dev_id, port_id, enable);
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
sw_error_t
mht_port_flowctrl_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_flowctrl_set(dev_id, port_id, enable);
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
sw_error_t
mht_port_flowctrl_forcemode_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_flowctrl_forcemode_set(dev_id, port_id, enable);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set duplex mode on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] duplex duplex mode
 * @return SW_OK or error code
 */
sw_error_t
mht_port_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t duplex)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_duplex_set(dev_id, port_id, duplex);
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
sw_error_t
mht_port_speed_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t speed)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_speed_set(dev_id, port_id, speed);
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
sw_error_t
mht_port_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t * pduplex)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_duplex_get(dev_id, port_id, pduplex);
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
sw_error_t
mht_port_speed_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t * pspeed)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_speed_get(dev_id, port_id, pspeed);
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
sw_error_t
mht_port_flowctrl_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_flowctrl_get(dev_id, port_id, enable);
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
sw_error_t
mht_port_flowctrl_forcemode_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * enable)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_flowctrl_forcemode_get(dev_id, port_id, enable);
	HSL_API_UNLOCK;
	return rv;
}

sw_error_t
mht_port_erp_power_mode_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_erp_power_mode_t power_mode)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_port_erp_power_mode_set(dev_id, port_id, power_mode);
	HSL_API_UNLOCK;
	return rv;
}

static sw_error_t
mht_port_interface_mode_switch(a_uint32_t dev_id, a_uint32_t port_id)
{
	sw_error_t rv = SW_OK;
	fal_port_interface_mode_t port_mode_old = PORT_INTERFACE_MODE_MAX;
	fal_port_interface_mode_t port_mode_new = PORT_INTERFACE_MODE_MAX;
	phy_info_t *phy_info = hsl_phy_info_get(dev_id);
	fal_mac_config_t mac_config = {0};

	rv = hsl_port_phy_interface_mode_status_get(dev_id, port_id,
			&port_mode_new);
	SW_RTN_ON_ERROR(rv);

	port_mode_old = phy_info->port_mode[port_id];

	if (port_mode_new != port_mode_old) {
		if (port_mode_new == PORT_SGMII_PLUS) {
			mac_config.mac_mode = FAL_MAC_MODE_SGMII_PLUS;
		} else if (port_mode_new == PHY_SGMII_BASET) {
			mac_config.mac_mode = FAL_MAC_MODE_SGMII;
		} else {
			return SW_NOT_SUPPORTED;
		}
		mac_config.config.sgmii.clock_mode = FAL_INTERFACE_CLOCK_MAC_MODE;
		mac_config.config.sgmii.auto_neg = A_TRUE;
		rv = mht_interface_mac_mode_set(dev_id, port_id,&mac_config);
		SW_RTN_ON_ERROR(rv);
		phy_info->port_mode[port_id] = port_mode_new;

		SSDK_INFO("mht port %d phy changed interface mode to %d from %d\n",
				port_id, port_mode_new, port_mode_old);
	}

	return rv;
}

sw_error_t
mht_port_link_update(struct qca_phy_priv *priv, a_uint32_t port_id,
	struct port_phy_status phy_status)
{
	sw_error_t rv = 0;

	if ((port_id == SSDK_PHYSICAL_PORT5) &&
			(A_TRUE == hsl_port_phy_connected(priv->device_id, port_id))) {
		rv = mht_port_interface_mode_switch(priv->device_id, port_id);
		SW_RTN_ON_ERROR (rv);
	}
	if (phy_status.link_status == PORT_LINK_DOWN) {
		/* configure speed to 1G to avoid qm issue */
		phy_status.speed = FAL_SPEED_1000;
		phy_status.duplex = FAL_FULL_DUPLEX;
	}
	/* configure gcc uniphy and mac speed frequency*/
	rv = mht_port_speed_clock_set(priv->device_id, port_id, phy_status.speed);
	SW_RTN_ON_ERROR (rv);
	/* configure mac speed and duplex */
	rv = _mht_port_mac_speed_set(priv->device_id, port_id, phy_status.speed);
	SW_RTN_ON_ERROR (rv);
	rv = _mht_port_mac_dupex_set(priv->device_id, port_id, phy_status.duplex);
	SW_RTN_ON_ERROR (rv);
	SSDK_DEBUG("mht port %d link %d update speed %d duplex %d\n",
			port_id, phy_status.speed,
			phy_status.speed, phy_status.duplex);
	if (phy_status.link_status == PORT_LINK_UP)
	{
		/* sync mac flowctrl */
		if (priv->port_tx_flowctrl_forcemode[port_id] != A_TRUE) {
			rv = __mht_port_txfc_status_set(priv->device_id,
				port_id, phy_status.tx_flowctrl);
			SW_RTN_ON_ERROR (rv);
			SSDK_DEBUG("mht port %d link up update txfc %d\n",
			port_id, phy_status.tx_flowctrl);
		}
		if (priv->port_tx_flowctrl_forcemode[port_id] != A_TRUE) {
			rv = __mht_port_rxfc_status_set(priv->device_id,
				port_id, phy_status.rx_flowctrl);
			SW_RTN_ON_ERROR (rv);
			SSDK_DEBUG("mht port %d link up update rxfc %d\n",
			port_id, phy_status.rx_flowctrl);
		}
		if (port_id != SSDK_PHYSICAL_PORT5) {
			/* enable eth phy clock */
			rv = ssdk_mht_port_clk_en_set(priv->device_id, port_id,
				MHT_CLK_TYPE_EPHY, A_TRUE);
			SW_RTN_ON_ERROR (rv);
		}
	}
	if (port_id != SSDK_PHYSICAL_PORT5) {
		if (phy_status.link_status == PORT_LINK_DOWN) {
			/* disable eth phy clock */
			rv = ssdk_mht_port_clk_en_set(priv->device_id, port_id,
				MHT_CLK_TYPE_EPHY, A_FALSE);
			SW_RTN_ON_ERROR (rv);
		}
		/* reset eth phy clock */
		rv = ssdk_mht_port_clk_reset(priv->device_id, port_id, MHT_CLK_TYPE_EPHY);
		SW_RTN_ON_ERROR (rv);
		/* reset eth phy fifo */
		rv = hsl_port_phy_function_reset(priv->device_id, port_id);
		SW_RTN_ON_ERROR (rv);
	}

	return rv;
}

/**
 * @}
 */
