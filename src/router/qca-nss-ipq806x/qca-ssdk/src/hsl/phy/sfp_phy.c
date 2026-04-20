/*
 * Copyright (c) 2018, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "sw.h"
#include "fal_port_ctrl.h"
#include "adpt.h"
#include "hsl_api.h"
#include "hsl.h"
#include "sfp_phy.h"
#include "aos_timer.h"
#include "hsl_phy.h"
#include <linux/kconfig.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include "ssdk_plat.h"
#include "ssdk_phy_i2c.h"
#include "ssdk_dts.h"
#include "hppe_uniphy_reg.h"
#include "hppe_uniphy.h"

static a_bool_t sfp_phy_drv_registered = A_FALSE;

#if (LINUX_VERSION_CODE < KERNEL_VERSION (5, 0, 0))
#define SFP_PHY_FEATURES        (SUPPORTED_FIBRE | \
                                SUPPORTED_1000baseT_Full | \
#ifndef MP
                                SUPPORTED_10000baseT_Full | \
#endif
                                SUPPORTED_Pause | \
                                SUPPORTED_Asym_Pause) | \
                                SUPPORTED_2500baseX_Full
#endif

static int
sfp_phy_probe(struct phy_device *pdev)
{
	fal_port_t port;
	struct qca_phy_priv *priv = pdev->priv;

	port = qca_ssdk_phydev_to_port(priv->device_id, pdev);
	if (A_TRUE == hsl_port_feature_get(priv->device_id, port, PHY_F_SFP_SGMII)) {
		pdev->autoneg = AUTONEG_ENABLE;
	} else {
		pdev->autoneg = AUTONEG_DISABLE;
	}
	return 0;
}

static void
sfp_phy_remove(struct phy_device *pdev)
{
	return;
}

static int
sfp_phy_config_aneg(struct phy_device *pdev)
{

	return 0;
}

static int
sfp_phy_aneg_done(struct phy_device *pdev)
{

	return SFP_ANEG_DONE;
}

sw_error_t
sfp_port_status_get_from_uniphy(a_uint32_t dev_id, a_uint32_t port_id,
	struct port_phy_status *phy_status)
{
	phy_info_t *phy_info = NULL;
	union uniphy_channel0_input_output_6_u uniphy_channel0_input_output_6 = {0};
	union sr_xs_pcs_kr_sts1_u sr_xs_pcs_kr_sts1 = {0};
	a_bool_t rx_los_status = A_TRUE;
	a_uint32_t uniphy_index = 0, link_status = 0;
	sw_error_t rv = SW_OK;

	phy_status->link_status = PORT_LINK_DOWN;
	phy_status->speed = FAL_SPEED_BUTT;
	phy_status->duplex = FAL_DUPLEX_BUTT;

	uniphy_index = hsl_port_to_uniphy(dev_id, port_id);
	if(uniphy_index == SSDK_MAX_UNIPHY_INSTANCE)
		return SW_FAIL;
	phy_info = hsl_phy_info_get(dev_id);
	SW_RTN_ON_NULL(phy_info);
	if(phy_info->port_mode[port_id] == PORT_10GBASE_R)
	{
		rv = hppe_sr_xs_pcs_kr_sts1_get(dev_id, uniphy_index, &sr_xs_pcs_kr_sts1);
		SW_RTN_ON_ERROR(rv);
		link_status = sr_xs_pcs_kr_sts1.bf.plu;
	}
	else
	{
		hppe_uniphy_channel0_input_output_6_get(dev_id, uniphy_index,
			&uniphy_channel0_input_output_6);
		link_status =
			uniphy_channel0_input_output_6.bf.newaddedfromhere_ch0_link;
	}
	/*1G SFP would get link info from uniphy register directly,
	2.5G SFP and 10G SFP would get link info from rx los and uniphy*/
	if(link_status)
	{
		switch(phy_info->port_mode[port_id])
		{
			case PORT_SGMII_PLUS:
			case PORT_10GBASE_R:
				sfp_phy_rx_los_status_get(dev_id, port_id, &rx_los_status);
				SSDK_DEBUG("port %d rx_los_status is %x\n", port_id, rx_los_status);
				/*if uniphy is link up and rx los is 0, then link up,
				if uniphy is link up but rx los is 1, this is fake link*/
				if(rx_los_status)
				{
					return SW_OK;
				}
				phy_status->link_status = PORT_LINK_UP;
				phy_status->duplex = FAL_FULL_DUPLEX;
				if(phy_info->port_mode[port_id] == PORT_SGMII_PLUS)
					phy_status->speed = FAL_SPEED_2500;
				else
					phy_status->speed = FAL_SPEED_10000;
				return SW_OK;
			default:
				break;
		}
		phy_status->link_status = PORT_LINK_UP;
		switch(uniphy_channel0_input_output_6.bf.newaddedfromhere_ch0_speed_mode)
		{
			case UNIPHY_SPEED_1000M:
				phy_status->speed = FAL_SPEED_1000;
				break;
			case UNIPHY_SPEED_100M:
				phy_status->speed = FAL_SPEED_100;
				break;
			case UNIPHY_SPEED_10M:
				phy_status->speed = FAL_SPEED_10;
				break;
			default:
				phy_status->speed = FAL_SPEED_BUTT;
				break;
		}
		if(uniphy_channel0_input_output_6.bf.newaddedfromhere_ch0_duplex_mode)
		{
			phy_status->duplex = FAL_FULL_DUPLEX;
		}
		else
		{
			phy_status->duplex = FAL_HALF_DUPLEX;
		}
	}
	SSDK_DEBUG("phy_status->link_status:%d, phy_status->speed:%d, "
	"phy_status->duplex:%d\n", phy_status->link_status, phy_status->speed,
	phy_status->duplex);

	return SW_OK;
}

static int
sfp_read_status(struct phy_device *pdev)
{
	fal_port_t port;
	struct qca_phy_priv *priv = pdev->priv;
#ifdef MP
	a_uint32_t port_mode = 0, uniphy_index = 0, uniphy_mode = 0;
	adpt_api_t *p_api = NULL;
	struct port_phy_status phy_status = {0};
	phy_info_t *phy_info = NULL;
#endif

	port = qca_ssdk_phydev_to_port(priv->device_id, pdev);
	if(port == 0)
		return -ENXIO;
#ifdef MP
	phy_info = hsl_phy_info_get(priv->device_id);
	if(!phy_info)
		return -1;
	port_mode = phy_info->port_mode[port];
	sfp_phy_interface_get_mode_status(priv->device_id, port, &port_mode);
	SSDK_DEBUG("new port mode:0x%x, old port mode:%x\n", port_mode,
		phy_info->port_mode[port]);
	uniphy_index = hsl_port_to_uniphy(priv->device_id, port);
	if(port_mode != phy_info->port_mode[port])
	{
		uniphy_mode = hsl_port_mode_to_uniphy_mode(priv->device_id, port_mode);
		p_api = adpt_api_ptr_get(priv->device_id);
		if(!p_api)
			return -1;
		p_api->adpt_uniphy_mode_set(priv->device_id, uniphy_index, uniphy_mode);
		phy_info->port_mode[port] = port_mode;
		pdev->interface = hsl_port_mode_to_phydev_interface(priv->device_id, port_mode);
	}
	sfp_port_status_get_from_uniphy(priv->device_id, port, &phy_status);
	pdev->link = phy_status.link_status;
	pdev->speed = phy_status.speed;
	pdev->duplex = phy_status.duplex;
#else
	pdev->link = priv->port_old_link[port - 1];
	if(pdev->link == PORT_LINK_UP)
	{
		pdev->speed = priv->port_old_speed[port - 1];
		pdev->duplex = priv->port_old_duplex[port - 1];
	}
	else
	{
		pdev->speed = FAL_SPEED_BUTT;
		pdev->duplex = FAL_DUPLEX_BUTT;
	}
#endif
	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION (5, 0, 0))
static int
sfp_phy_update_link(struct phy_device *pdev)
{
	int ret;

	ret = sfp_read_status(pdev);

	return ret;
}
#else
static int sfp_phy_read_abilities(struct phy_device *pdev)
{
	int features[] = {
		ETHTOOL_LINK_MODE_FIBRE_BIT,
		ETHTOOL_LINK_MODE_100baseT_Full_BIT,
		ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
#ifndef MP
		ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
#endif
		ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
		ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
		ETHTOOL_LINK_MODE_Pause_BIT,
		ETHTOOL_LINK_MODE_Asym_Pause_BIT,
		ETHTOOL_LINK_MODE_Autoneg_BIT,
	};

	linkmode_set_bit_array(features,
		ARRAY_SIZE(features),
		pdev->supported);

	return 0;
}
#endif

static sw_error_t
sfp_phy_i2c_read(a_uint32_t dev_id, a_uint32_t i2c_slaver, a_uint32_t reg_addr,
	a_uint16_t *reg_data)
{
	sw_error_t rv = SW_OK;
	a_uint8_t rx[2] = { 0 };

	rv = qca_i2c_data_get(dev_id, i2c_slaver, reg_addr & 0xff, rx, sizeof(rx));

	if (rv == SW_OK) {
		*reg_data = (rx[0] << 8) | rx[1];
	} else {
		*reg_data = 0xffff;
	}

	return rv;
}

static struct phy_driver sfp_phy_driver = {
	.name		= "QCA SFP",
	.phy_id		= SFP_PHY,
	.phy_id_mask = SFP_PHY_MASK,
	.probe		= sfp_phy_probe,
	.remove		= sfp_phy_remove,
	.config_aneg	= sfp_phy_config_aneg,
	.aneg_done	= sfp_phy_aneg_done,
	.read_status	= sfp_read_status,
#if (LINUX_VERSION_CODE < KERNEL_VERSION (5, 0, 0))
	.features	= SFP_PHY_FEATURES,
	.update_link	= sfp_phy_update_link,
#else
	.get_features	= sfp_phy_read_abilities,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	.mdiodrv.driver	= { .owner = THIS_MODULE },
#else
	.driver		= { .owner = THIS_MODULE },
#endif
};

int sfp_phy_device_setup(a_uint32_t dev_id, a_uint32_t port, a_uint32_t phy_id,
	void *priv)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct phy_device *phydev;
	a_uint32_t addr = 0;
	struct mii_bus *bus;

	if (A_TRUE == hsl_port_phy_combo_capability_get(dev_id, port))
	{
		return 0;
	}
	/*create phy device*/
#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port) == PHY_I2C_ACCESS) {
		addr = qca_ssdk_port_to_phy_mdio_fake_addr(dev_id, port);
	} else
#endif
	{
		addr = qca_ssdk_port_to_phy_addr(dev_id, port);
	}
	bus = ssdk_phy_miibus_get(dev_id, addr);
	if(!bus)
		return SW_NOT_FOUND;
	phydev = phy_device_create(bus, addr, phy_id, false, NULL);
	if (IS_ERR(phydev) || phydev == NULL) {
		SSDK_ERROR("Failed to create phy device!\n");
		return SW_NOT_SUPPORTED;
	}
	/*register phy device*/
	phy_device_register(phydev);

	phydev->priv = priv;
	/*
	 * Set the PHY OF node in order to be able to later connect the
	 * fake SFP PHY by passing it as a phandle in phy-handle.
	 */
	phydev->mdio.dev.of_node = hsl_port_node_get(dev_id, port);
	if (!phydev->mdio.dev.of_node)
		return SW_NOT_FOUND;
#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port) == PHY_I2C_ACCESS) {
		if(phydev->drv)
			phy_driver_unregister(phydev->drv);
	}
#endif
#endif
	return 0;
}

void sfp_phy_device_remove(a_uint32_t dev_id, a_uint32_t port)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct phy_device *phydev = NULL;
	a_uint32_t addr = 0;
	struct mii_bus *bus;

	if (A_TRUE == hsl_port_phy_combo_capability_get(dev_id, port))
	{
		return;
	}

	bus = ssdk_port_miibus_get(dev_id, port);
	if(!bus)
		return;
#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port) == PHY_I2C_ACCESS) {
		addr = qca_ssdk_port_to_phy_mdio_fake_addr(dev_id, port);
	} else
#endif
	{
		addr = TO_PHY_ADDR(qca_ssdk_port_to_phy_addr(dev_id, port));
	}

	if (addr < PHY_MAX_ADDR)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
		if (bus->mdio_map[addr])
			phydev = to_phy_device(&bus->mdio_map[addr]->dev);
#else
		phydev = bus->phy_map[addr];
#endif
	if (phydev)
		phy_device_remove(phydev);
#endif
}

int sfp_phy_driver_register(void)
{
	int ret = 0;
	if(sfp_phy_drv_registered == A_FALSE)
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
		ret = phy_driver_register(&sfp_phy_driver, THIS_MODULE);
#else
		ret = phy_driver_register(&sfp_phy_driver);
#endif
		sfp_phy_drv_registered = A_TRUE;
	}
	return ret;
}

void sfp_phy_driver_unregister(void)
{
	if (sfp_phy_drv_registered == A_TRUE)
	{
		phy_driver_unregister(&sfp_phy_driver);
	}
}

int sfp_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	SSDK_INFO("qca probe sfp phy driver succeeded!\n");

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++) {
		if (port_bmp & (0x1 << port_id)) {
			sfp_phy_device_setup(dev_id, port_id, SFP_PHY, priv);
		}
	}
	return sfp_phy_driver_register();
}

void sfp_phy_exit(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0;

	sfp_phy_driver_unregister();

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++) {
		if (port_bmp & (0x1 << port_id)) {
			sfp_phy_device_remove(dev_id, port_id);
		}
	}

}

static a_uint16_t sfp_part_num[5] = {0x3330, 0x2d31, 0x3539, 0x392d, 0x3032};

a_bool_t
sfp_phy_part_number_check(a_uint32_t dev_id, a_uint32_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint16_t reg_data[5] = {0}, i = 0;

	for (i = 0; i < 5; i++) {
		rv = sfp_phy_i2c_read(dev_id, SFP_E2PROM_ADDR,
			SFP_E2PROM_PART_NUM_OFFSET + i *2, &reg_data[i]);
		if (rv != SW_OK)
			return A_FALSE;
		if (sfp_part_num[i] != reg_data[i])
			return A_FALSE;
	}
	return A_TRUE;
}

a_bool_t
sfp_phy_usxgmii_check(a_uint32_t dev_id, a_uint32_t port_id)
{
	if (sfp_phy_part_number_check(dev_id, port_id) == A_TRUE) {
		sw_error_t rv = SW_OK;
		a_uint16_t reg_data = 0;
		rv = sfp_phy_i2c_read(dev_id, SFP_E2PROM_EXTEND_ADDR,
			SFP_EXTEND_USXGMII_OFFSET, &reg_data);
		if (rv != SW_OK)
			return A_FALSE;
		if ((reg_data >> 8) & 0x1) {
			return A_TRUE;
		}
	}
	return A_FALSE;
}

a_bool_t
sfp_phy_present_status_check(a_uint32_t dev_id, a_uint32_t port_id)
{
	sw_error_t rv = SW_OK;
	a_bool_t mod_present_status = A_FALSE;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	/*if sfp_mode_present_pin is available, then need to check it*/
	if(priv && !qca_ssdk_gpio_is_invalid(dev_id, priv->sfp_mod_present_pin[port_id]))
	{
		rv = sfp_phy_mod_present_status_get(dev_id, port_id,
			&mod_present_status);
		if (rv != SW_OK)
			return A_FALSE;
		/*if sfp mod is not present, so no need to read SFP module with I2C*/
		if(!mod_present_status)
		{
			SSDK_DEBUG("port%d sfp mod is not present now\n", port_id);
			return A_FALSE;
		} else {
			return A_TRUE;
		}
	} else {
		/* backword compatible without present_pin case */
		return A_TRUE;
	}
}
sw_error_t sfp_phy_interface_get_mode_status(a_uint32_t dev_id,
	a_uint32_t port_id, fal_port_interface_mode_t *interface_mode_status)
{
	sw_error_t rv = SW_OK;
	a_uint16_t reg_data = 0, sfp_speed = 0;
	a_uint16_t sfp_type = 0;
	struct phy_device *phydev;

	if (sfp_phy_present_status_check(dev_id, port_id) == A_FALSE) {
		return SW_OK;
	}
	rv = hsl_port_phydev_get(dev_id, port_id, &phydev);
	SW_RTN_ON_ERROR(rv);
	rv = sfp_phy_i2c_read(dev_id, SFP_E2PROM_ADDR, SFP_SPEED_ADDR,
		&reg_data);
	SW_RTN_ON_ERROR(rv);
	sfp_speed = SFP_TO_SFP_SPEED(reg_data);
	SSDK_DEBUG("sfp_speed:%d\n", sfp_speed);

	if(sfp_speed >= SFP_SPEED_1000M &&
		sfp_speed < SFP_SPEED_2500M)
	{
		reg_data = 0;
		rv = sfp_phy_i2c_read(dev_id, SFP_E2PROM_ADDR, SFP_TYPE_ADDR,
			&reg_data);
		SW_RTN_ON_ERROR(rv);
		sfp_type = SFP_TO_SFP_TYPE(reg_data);
		if (sfp_type == SFP_TYPE_1000MBASE_T) {
			/* sfp copper module mode*/
			if (phydev->autoneg == AUTONEG_ENABLE) {
				/* sfp copper module interface is sgmii mode */
				*interface_mode_status = PHY_SGMII_BASET;
				hsl_port_feature_set(dev_id, port_id, PHY_F_SFP_SGMII);
			} else {
				/* sfp copper module interface is serdes mode */
				*interface_mode_status = PORT_SGMII_FIBER;
				hsl_port_feature_clear(dev_id, port_id, PHY_F_SFP_SGMII);
			}
		} else {
			/* sfp fiber module mode */
			*interface_mode_status = PORT_SGMII_FIBER;
			hsl_port_feature_clear(dev_id, port_id, PHY_F_SFP_SGMII);
		}
	}
	else if(sfp_speed >= SFP_SPEED_10000M)
	{
		*interface_mode_status = PORT_10GBASE_R;
		if (sfp_phy_usxgmii_check(dev_id, port_id) == A_TRUE) {
			*interface_mode_status = PORT_USXGMII;
		}
	}
	else if(sfp_speed >= SFP_SPEED_2500M &&
		sfp_speed < SFP_SPEED_5000M)
	{
		struct port_phy_status sfp_status= {0};
		adpt_api_t *p_api;

		SW_RTN_ON_NULL(p_api = adpt_api_ptr_get(dev_id));
		rv = p_api->adpt_port_phy_status_get(dev_id, port_id, &sfp_status);
		SW_RTN_ON_ERROR(rv);

		if (sfp_status.link_status == PORT_LINK_DOWN) {
			/*autoneg is for different speed switching on one SFP module*/
			if (phydev->autoneg == AUTONEG_ENABLE) {
				if (sfp_status.link_status == PORT_LINK_DOWN) {
					if (*interface_mode_status == PHY_SGMII_BASET) {
						*interface_mode_status = PORT_SGMII_PLUS;
					} else {
						/* sfp copper module interface is serdes mode */
						*interface_mode_status = PHY_SGMII_BASET;
						hsl_port_feature_clear(dev_id, port_id,
							PHY_F_SFP_SGMII);
					}
				}
			} else {
				*interface_mode_status = PORT_SGMII_PLUS;
			}
			return SW_OK;
		}
		if (sfp_status.link_status == PORT_LINK_UP) {
			/*solution for SFP link up case under 10G-R mode*/
			if ((*interface_mode_status != PHY_SGMII_BASET) &&
				(*interface_mode_status != PORT_SGMII_PLUS)) {
				*interface_mode_status = PORT_SGMII_PLUS;
			}
			return SW_OK;
		}
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

sw_error_t
sfp_phy_phydev_adv_update(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t adv_mask,
	a_uint32_t adv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t new_adv = 0;
	struct phy_device *phydev = NULL;

	rv = hsl_phy_phydev_get(dev_id, phy_addr, &phydev);
	SW_RTN_ON_ERROR (rv);
	rv = hsl_phy_linkmode_adv_to_adv(phydev->advertising, &new_adv);
	SW_RTN_ON_ERROR (rv);
	new_adv &= ~adv_mask;
	new_adv |= adv;

	rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, new_adv);

	return rv;
}

sw_error_t
sfp_phy_rx_los_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t *rx_los_status)
{
	a_uint32_t rx_los_pin = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	if(!priv)
		return SW_NOT_INITIALIZED;
	rx_los_pin = priv->sfp_rx_los_pin[port_id];
	if(qca_ssdk_gpio_is_invalid(dev_id, rx_los_pin))
	{
		return SW_NOT_SUPPORTED;
	}
	*rx_los_status = gpio_get_value(rx_los_pin);
	SSDK_DEBUG("port%d sfp rx_los_pin is %d, the rx_los_status is %d",
		port_id, rx_los_pin, *rx_los_status);

	return SW_OK;
}

sw_error_t
sfp_phy_tx_dis_status_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t tx_dis_status)
{
	a_uint32_t tx_dis_pin = 0;
	int ret = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	if(!priv)
		return SW_NOT_INITIALIZED;
	tx_dis_pin = priv->sfp_tx_dis_pin[port_id];
	if(qca_ssdk_gpio_is_invalid(dev_id, tx_dis_pin))
	{
		return SW_NOT_SUPPORTED;
	}
	ret = gpio_request(tx_dis_pin, "sfp_tx_dis_pin");
	if(ret)
	{
		SSDK_ERROR("tx_dis_pin request failed, ret:%d\n", ret);
		return SW_FAIL;
	}
	ret = gpio_direction_output(tx_dis_pin, tx_dis_status);
	gpio_free(tx_dis_pin);
	if(ret)
	{
		SSDK_ERROR("tx_dis_pin configure faied, ret:%d\n", ret);
		return SW_FAIL;
	}

	return SW_OK;
}

sw_error_t
sfp_phy_tx_dis_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t *tx_dis_status)
{
	a_uint32_t tx_dis_pin = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	if(!priv)
		return SW_NOT_INITIALIZED;
	tx_dis_pin = priv->sfp_tx_dis_pin[port_id];
	if(qca_ssdk_gpio_is_invalid(dev_id, tx_dis_pin))
	{
		return SW_NOT_SUPPORTED;
	}
	*tx_dis_status = gpio_get_value(tx_dis_pin);
	SSDK_DEBUG("port%d sfp tx_dis_pin is %d, the tx_dis_status is %d",
		port_id, tx_dis_pin, *tx_dis_status);

	return SW_OK;
}

sw_error_t
sfp_phy_mod_present_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t *mod_present_status)
{
	a_uint32_t mod_present_pin = 0;
	a_bool_t mod_present_pin_status = A_FALSE;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);

	if(!priv)
		return SW_NOT_INITIALIZED;
	mod_present_pin = priv->sfp_mod_present_pin[port_id];
	if(qca_ssdk_gpio_is_invalid(dev_id, mod_present_pin))
	{
		return SW_NOT_SUPPORTED;
	}
	mod_present_pin_status = gpio_get_value(mod_present_pin);
	SSDK_DEBUG("port%d sfp mod_present_pin is %d, mod_present_pin_status is %d",
		port_id, mod_present_pin, mod_present_pin_status);
	if(mod_present_pin_status)
		*mod_present_status = A_FALSE;
	else
		*mod_present_status = A_TRUE;

	return SW_OK;
}

sw_error_t
sfp_phy_medium_status_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t sfp_medium_status)
{
	a_uint32_t sfp_medium_pin = 0;
	int ret = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);
	SW_RTN_ON_NULL(priv);

	sfp_medium_pin = priv->sfp_medium_pin[port_id];
	if(qca_ssdk_gpio_is_invalid(dev_id, sfp_medium_pin))
	{
		return SW_NOT_SUPPORTED;
	}
	ret = gpio_request(sfp_medium_pin, "sfp_medium_pin");
	if(ret)
	{
		SSDK_ERROR("sfp_medium_pin request failed, ret:%d\n", ret);
		return SW_FAIL;
	}
	if(sfp_medium_status)
		ret = gpio_direction_output(sfp_medium_pin, 1);
	else
		ret = gpio_direction_output(sfp_medium_pin, 0);
	gpio_free(sfp_medium_pin);
	if(ret)
	{
		SSDK_ERROR("sfp_medium_pin configure faied, ret:%d\n", ret);
		return SW_FAIL;
	}

	return SW_OK;
}

sw_error_t
sfp_phy_port_status_get(a_uint32_t dev_id, a_uint32_t port_id,
		struct port_phy_status *phy_status)
{
	sw_error_t rv = SW_OK;
	a_uint16_t reg_data = 0, phy_link = 0;

	ADPT_DEV_ID_CHECK(dev_id);

	if (sfp_phy_present_status_check(dev_id, port_id) == A_FALSE) {
		return SW_OK;
	}
	if (sfp_phy_part_number_check(dev_id, port_id) == A_TRUE) {
		rv = sfp_phy_i2c_read(dev_id, SFP_E2PROM_EXTEND_ADDR,
			SFP_EXTEND_LINK_OFFSET, &reg_data);
		SW_RTN_ON_ERROR(rv);

		phy_link = (reg_data >> 8) & 0x1;
		if (phy_link) {
			a_uint16_t speed_data = 0xff, retries = 50;
			phy_status->link_status = PORT_LINK_UP;
			if (reg_data & 0x1) {
				phy_status->duplex = FAL_HALF_DUPLEX;
			} else {
				phy_status->duplex = FAL_FULL_DUPLEX;
			}
			rv = sfp_phy_i2c_read(dev_id, SFP_E2PROM_EXTEND_ADDR,
					SFP_EXTEND_SPEED_OFFSET, &reg_data);
			SW_RTN_ON_ERROR(rv);
			speed_data = (reg_data >> 0x8) & 0x7;
			while ((speed_data != SFP_PHY_SPEED_10000M) &&
				(speed_data != SFP_PHY_SPEED_5000M) &&
				(speed_data != SFP_PHY_SPEED_2500M) &&
				(speed_data != SFP_PHY_SPEED_1000M) &&
				(speed_data != SFP_PHY_SPEED_100M)) {
				aos_mdelay(10);
				if (retries-- == 0) {
					SSDK_ERROR("usxgmii sfp port speed sync failed!\n");
					break;
				}
				rv = sfp_phy_i2c_read(dev_id, SFP_E2PROM_EXTEND_ADDR,
					SFP_EXTEND_SPEED_OFFSET, &reg_data);
				SW_RTN_ON_ERROR(rv);
				speed_data = (reg_data >> 0x8) & 0x7;
			}
			switch (speed_data) {
				case SFP_PHY_SPEED_10000M:
					phy_status->speed = FAL_SPEED_10000;
					break;
				case SFP_PHY_SPEED_5000M:
					phy_status->speed = FAL_SPEED_5000;
					break;
				case SFP_PHY_SPEED_2500M:
					phy_status->speed = FAL_SPEED_2500;
					break;
				case SFP_PHY_SPEED_1000M:
					phy_status->speed = FAL_SPEED_1000;
					break;
				case SFP_PHY_SPEED_100M:
					phy_status->speed = FAL_SPEED_100;
					break;
				default:
					phy_status->speed = FAL_SPEED_BUTT;
					break;
			}
		} else {
			phy_status->link_status = PORT_LINK_DOWN;
			phy_status->speed = FAL_SPEED_BUTT;
			phy_status->duplex = FAL_DUPLEX_BUTT;
		}
	}

	return rv;
}
