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
 * @defgroup mht_init MHT_INIT
 * @{
 */
#include "sw.h"
#include "hsl_phy.h"
#include "ssdk_init.h"
#include "ssdk_plat.h"
#include "ssdk_mht_pinctrl.h"
#include "mht_sec_ctrl.h"
#include "ssdk_dts.h"
#include "fal_interface_ctrl.h"
#include "fal_port_ctrl.h"
#include "ssdk_mht_clk.h"
#include "mht_port_ctrl.h"
#include "ref_port_ctrl.h"
#include "mht_interface_ctrl.h"
#include "hsl_port_prop.h"
#ifdef IN_LED
#include "ssdk_led.h"
#endif

static sw_error_t
qca_mht_work_mode_init(a_uint32_t dev_id, a_uint32_t mac_mode0, a_uint32_t mac_mode1)
{
	sw_error_t ret = SW_OK;

	switch (mac_mode0) {
		case PORT_WRAPPER_SGMII_PLUS:
		case PORT_WRAPPER_SGMII_CHANNEL0:
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	if(mht_uniphy_mode_check(dev_id, MHT_UNIPHY_SGMII_0, MHT_UNIPHY_PHY))
	{
		return qca_mht_work_mode_set(dev_id, MHT_SWITCH_BYPASS_PORT5_MODE);
	}

	switch (mac_mode1) {
		case PORT_WRAPPER_SGMII_PLUS:
		case PORT_WRAPPER_SGMII_CHANNEL0:
		case PORT_WRAPPER_MAX:
			ret = qca_mht_work_mode_set(dev_id, MHT_SWITCH_MODE);
			SW_RTN_ON_ERROR(ret);
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

static sw_error_t
_qca_mht_interface_mode_init(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t mac_mode)
{
	sw_error_t rv = SW_OK;
	fal_mac_config_t mac_config = {0};
	a_bool_t force_en = A_FALSE;
	fal_port_speed_t force_speed = FAL_SPEED_BUTT;
	mht_work_mode_t work_mode = MHT_SWITCH_MODE;
	phy_info_t *phy_info = hsl_phy_info_get(dev_id);

	/* The clock parent need to be configured before initializing
	 * the interface mode.*/
	qca_mht_work_mode_get(dev_id, &work_mode);
	ssdk_mht_gcc_port_clk_parent_set(dev_id, work_mode, port_id);

	force_en = hsl_port_feature_get(dev_id, port_id, PHY_F_FORCE);
	if(force_en)
		force_speed = hsl_port_force_speed_get(dev_id, port_id);
	if(mac_mode == PORT_WRAPPER_SGMII_PLUS) {
		mac_config.mac_mode = FAL_MAC_MODE_SGMII_PLUS;
		phy_info->port_mode[port_id] = PORT_SGMII_PLUS;
	}
	else if(mac_mode == PORT_WRAPPER_SGMII_CHANNEL0) {
		mac_config.mac_mode = FAL_MAC_MODE_SGMII;
		phy_info->port_mode[port_id] = PHY_SGMII_BASET;
	}
	else if(mac_mode == PORT_WRAPPER_MAX)
	{
		mac_config.mac_mode = FAL_MAC_MODE_MAX;
		phy_info->port_mode[port_id] = PORT_INTERFACE_MODE_MAX;
	}
	else
		return SW_NOT_SUPPORTED;
	mac_config.config.sgmii.clock_mode = FAL_INTERFACE_CLOCK_MAC_MODE;
	mac_config.config.sgmii.auto_neg = !force_en;
	mac_config.config.sgmii.force_speed = force_speed;

	rv = fal_interface_mac_mode_set(dev_id, port_id, &mac_config);

	return rv;
}

static sw_error_t
qca_mht_interface_mode_init(a_uint32_t dev_id, a_uint32_t mac_mode0,
	a_uint32_t mac_mode1)
{
	sw_error_t rv = SW_OK;

	rv = _qca_mht_interface_mode_init(dev_id, SSDK_PHYSICAL_PORT0, mac_mode0);
	SW_RTN_ON_ERROR(rv);

	rv = _qca_mht_interface_mode_init(dev_id, SSDK_PHYSICAL_PORT5, mac_mode1);
	SW_RTN_ON_ERROR(rv);

	return SW_OK;
}

static inline void qca_mht_switch_reset(a_uint32_t dev_id)
{
	/* Reset switch core */
	ssdk_mht_clk_reset(dev_id, MHT_SWITCH_CORE_CLK);

	/* Reset MAC ports */
	ssdk_mht_clk_reset(dev_id, MHT_MAC0_TX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC0_RX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC1_TX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC1_RX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC2_TX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC2_RX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC3_TX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC3_RX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC4_TX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC4_RX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC5_TX_CLK);
	ssdk_mht_clk_reset(dev_id, MHT_MAC5_RX_CLK);

	return;
}

/* Initialize the MDIO master for the backpressure feature */
sw_error_t qca_mht_mdio_master_init(a_uint32_t dev_id)
{
#define MHT_GPIO_PIN_MDC		20
#define MHT_GPIO_PIN_MDO		21
#define MHT_MDIO_MASTER_FREQ_26M	3
#define MHT_MDIO_MASTER_TIMER_CNT	0xc8
#define MHT_MDIO_MASTER_PREAMBLE_LEN	4
	sw_error_t ret = SW_OK;

	ret = mht_gpio_pin_cfg_set_hihys(dev_id, MHT_GPIO_PIN_MDC, A_FALSE);
	SW_RTN_ON_ERROR(ret);

	ret = mht_gpio_pin_cfg_set_hihys(dev_id, MHT_GPIO_PIN_MDO, A_FALSE);
	SW_RTN_ON_ERROR(ret);

	ret = mht_gpio_pin_cfg_set_drvs(dev_id, MHT_GPIO_PIN_MDC,
			MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_2_MA);
	SW_RTN_ON_ERROR(ret);

	ret = mht_gpio_pin_cfg_set_drvs(dev_id, MHT_GPIO_PIN_MDO,
			MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_2_MA);
	SW_RTN_ON_ERROR(ret);

	ret = mht_gpio_pin_mux_set(dev_id, MHT_GPIO_PIN_MDC, MHT_PIN_FUNC_MDC_M);
	SW_RTN_ON_ERROR(ret);

	ret = mht_gpio_pin_mux_set(dev_id, MHT_GPIO_PIN_MDO, MHT_PIN_FUNC_MDO_M);
	SW_RTN_ON_ERROR(ret);

	ret = mht_gpio_pin_cfg_set_bias(dev_id, MHT_GPIO_PIN_MDC, PIN_CONFIG_BIAS_PULL_UP);
	SW_RTN_ON_ERROR(ret);

	ret = mht_gpio_pin_cfg_set_bias(dev_id, MHT_GPIO_PIN_MDO, PIN_CONFIG_BIAS_PULL_UP);
	SW_RTN_ON_ERROR(ret);

	ret = qca_mht_mdio_cfg(dev_id, MHT_MDIO_MASTER_FREQ_26M,
			MHT_MDIO_MASTER_TIMER_CNT, MHT_MDIO_MASTER_PREAMBLE_LEN);
	return ret;
}
#if (defined(IN_PORTCONTROL) || defined(IN_LED))
static sw_error_t
qca_mht_portctrl_hw_init(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	fal_port_speed_t force_speed = FAL_SPEED_BUTT;
	fal_port_duplex_t force_duplex = FAL_DUPLEX_BUTT;
	a_uint32_t i = 0, port_max = SSDK_PHYSICAL_PORT6;

	for(i = SSDK_PHYSICAL_PORT0; i < port_max; i++)
	{
		if (A_FALSE == hsl_port_prop_check (dev_id, i, HSL_PP_INCL_CPU))
			continue;
		if(hsl_port_feature_get(dev_id, i, PHY_F_FORCE) == A_TRUE)
		{
			force_speed = hsl_port_force_speed_get(dev_id, i);
			rv = fal_port_speed_set(dev_id, i, force_speed);
			SW_RTN_ON_ERROR(rv);

			force_duplex = hsl_port_force_duplex_get(dev_id, i);
			rv = fal_port_duplex_set(dev_id, i, force_duplex);
			SW_RTN_ON_ERROR(rv);

			rv = fal_port_txmac_status_set(dev_id, i, A_TRUE);
			SW_RTN_ON_ERROR(rv);
			rv = fal_port_rxmac_status_set(dev_id, i, A_TRUE);
			SW_RTN_ON_ERROR(rv);
		}
		else
		{
			rv = fal_port_txmac_status_set(dev_id, i, A_FALSE);
			SW_RTN_ON_ERROR(rv);
			rv = fal_port_rxmac_status_set(dev_id, i, A_FALSE);
			SW_RTN_ON_ERROR(rv);
		}
#ifdef IN_LED
		ssdk_led_init(dev_id, i);
#endif
	}

	return SW_OK;
}
#endif

int qca_mht_hw_init(ssdk_init_cfg *cfg, a_uint32_t dev_id)
{
	int ret = 0;
	mht_work_mode_t work_mode;
	a_uint32_t port_bmp = 0;

	if(!qca_mht_sku_switch_core_enabled(dev_id))
	{
		SSDK_ERROR("MHT switch core is not enabled on the SKU\n");
		return SW_NOT_SUPPORTED;
	}
	ret = qca_mht_work_mode_init(dev_id, cfg->mac_mode, cfg->mac_mode1);
	SW_RTN_ON_ERROR(ret);

	ret = qca_mht_interface_mode_init(dev_id, cfg->mac_mode, cfg->mac_mode1);
	SW_RTN_ON_ERROR(ret);

	qca_mht_switch_reset(dev_id);

	ret = qca_switch_init(dev_id);
	SW_RTN_ON_ERROR(ret);

#ifdef IN_PORTVLAN
	ssdk_portvlan_init(dev_id);
#endif

	port_bmp = ssdk_cpu_bmp_get(dev_id) | ssdk_wan_bmp_get(dev_id) | ssdk_lan_bmp_get(dev_id);
	qca_mht_work_mode_get(dev_id, &work_mode);
	ssdk_mht_gcc_clock_init(dev_id, work_mode, port_bmp);

	ret = ssdk_mht_pinctrl_init(dev_id);
	SW_RTN_ON_ERROR(ret);

	ret = qca_mht_mdio_master_init(dev_id);
	SW_RTN_ON_ERROR(ret);
#if (defined(IN_PORTCONTROL) || defined(IN_LED))
	ret = qca_mht_portctrl_hw_init(dev_id);
#endif

	return ret;
}

sw_error_t
qca_mht_sw_mac_polling_task(struct qca_phy_priv *priv)
{
	sw_error_t rv = 0, port_id;
	a_uint32_t portbmp = 0;
	struct port_phy_status phy_status = {0};
	a_bool_t link_changed = A_FALSE;

	portbmp = qca_ssdk_port_bmp_get(priv->device_id);
	SSDK_DEBUG("mht sw mac polling task portbmp value is 0x%x\n", portbmp);
	for (port_id = 1; port_id < SW_MAX_NR_PORT; port_id++) {
		if(!(portbmp & (0x1 << port_id)) ||
			!(hsl_port_phy_connected(priv->device_id, port_id)))
			continue;
		rv = hsl_port_phy_status_get(priv->device_id, port_id, &phy_status);
		if (rv != SW_OK) {
			SSDK_DEBUG("mht failed to get port %d status return value is %d\n",
					port_id, rv);
			continue;
		}
		link_changed = A_FALSE;
		SSDK_DEBUG("mth port_id %d phy link status is %d and speed is %d\n",
				port_id, phy_status.link_status, phy_status.speed);
		/* Up --> Down */
		if ((priv->port_old_link[port_id] == PORT_LINK_UP) &&
			(phy_status.link_status == PORT_LINK_DOWN)) {
			link_changed = A_TRUE;
			/* disable mac rx function */
			rv = fal_port_rxmac_status_set(priv->device_id, port_id, A_FALSE);
			SW_RTN_ON_ERROR(rv);
			/* disable mac tx function */
			rv = fal_port_txmac_status_set(priv->device_id, port_id, A_FALSE);
			SW_RTN_ON_ERROR(rv);
			/* update gcc, mac speed, mac duplex and phy stauts */
			rv = mht_port_link_update(priv, port_id, phy_status);
			SW_RTN_ON_ERROR(rv);
			priv->port_old_link[port_id] = phy_status.link_status;
#ifdef IN_FDB
			/* flush all dynamic fdb of this port */
			fal_fdb_del_by_port(priv->device_id, port_id, 0);
#endif
		}
		/* Down --> Up */
		if ((priv->port_old_link[port_id] == PORT_LINK_DOWN) &&
			(phy_status.link_status == PORT_LINK_UP)) {
			link_changed = A_TRUE;
			/* update gcc, mac speed, mac duplex and phy stauts */
			rv = mht_port_link_update(priv, port_id, phy_status);
			SW_RTN_ON_ERROR(rv);
			/* enable mac tx function */
			rv = fal_port_txmac_status_set(priv->device_id, port_id, A_TRUE);
			SW_RTN_ON_ERROR(rv);
			/* enable mac rx function */
			rv = fal_port_rxmac_status_set(priv->device_id, port_id, A_TRUE);
			SW_RTN_ON_ERROR(rv);
			/* save the current link status */
			priv->port_old_link[port_id] = phy_status.link_status;
		}
		SSDK_DEBUG("mht port %d old link status is %d\n",
				port_id, priv->port_old_link[port_id]);
		if (link_changed) {
			unsigned char link_notify_speed = 0;

			link_notify_speed = ssdk_to_link_notify_speed(phy_status.speed);
			ssdk_port_link_notify(port_id, phy_status.link_status,
				link_notify_speed, phy_status.duplex);
		}
	}

	return rv;
}

/**
 * @}
 */
