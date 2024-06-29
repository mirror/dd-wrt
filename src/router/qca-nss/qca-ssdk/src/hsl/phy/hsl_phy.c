/*
 * Copyright (c) 2015, 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*qca808x_start*/
#include "sw.h"
#include "hsl_phy.h"
#include "hsl.h"
#include "hsl_dev.h"
/*qca808x_end*/
#include "ssdk_dts.h"
#if defined(ISIS) ||defined(ISISC) ||defined(GARUDA)
#include <f1_phy.h>
#endif
#if defined(ATHENA) ||defined(SHIVA) ||defined(HORUS)
#include <f2_phy.h>
#endif
#ifdef MP
#include "mpge_phy.h"
#endif
#ifdef IN_MALIBU_PHY
#include <malibu_phy.h>
#endif
#ifdef IN_AQUANTIA_PHY
#include <aquantia_phy.h>
#endif
#ifdef IN_QCA803X_PHY
#include <qca803x_phy.h>
#endif
#ifdef IN_SFP_PHY
#include <sfp_phy.h>
#endif
#ifdef IN_QCA808X_PHY
/*qca808x_start*/
#include <qca808x_phy.h>
/*qca808x_end*/
#endif
#include <linux/of_gpio.h>
/*qca808x_start*/
#include "sw.h"
#include "ssdk_plat.h"
#include "hsl_port_prop.h"
#include <linux/netdevice.h>
#include <linux/i2c.h>
#include "ssdk_phy_i2c.h"

phy_info_t *phy_info[SW_MAX_NR_DEV] = {0};
a_uint32_t port_bmp[SW_MAX_NR_DEV] = {0};


phy_driver_instance_t ssdk_phy_driver[] =
{
/*qca808x_end*/
	#if defined(ISIS) ||defined(ISISC) ||defined(GARUDA)
	{F1_PHY_CHIP, {0}, NULL, f1_phy_init, NULL},
	#else
	{F1_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
	#if defined(ATHENA) ||defined(SHIVA) ||defined(HORUS)
	{F2_PHY_CHIP, {0}, NULL, f2_phy_init, NULL},
	#else
	{F2_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
	#ifdef IN_MALIBU_PHY
	{MALIBU_PHY_CHIP, {0}, NULL, malibu_phy_init, NULL},
	#else
	{MALIBU_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
	#ifdef IN_AQUANTIA_PHY
	{AQUANTIA_PHY_CHIP, {0}, NULL, aquantia_phy_init, NULL},
	#else
	{AQUANTIA_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
	#ifdef IN_QCA803X_PHY
	{QCA803X_PHY_CHIP, {0}, NULL, qca803x_phy_init, NULL},
	#else
	{QCA803X_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
	#ifdef IN_SFP_PHY
	{SFP_PHY_CHIP, {0}, NULL, sfp_phy_init, sfp_phy_exit},
	#else
	{SFP_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
	#ifdef MP
	{MPGE_PHY_CHIP, {0}, NULL, mpge_phy_init, NULL},
	#else
	{MPGE_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
	#ifdef IN_QCA808X_PHY
/*qca808x_start*/
	{QCA808X_PHY_CHIP, {0}, NULL, qca808x_phy_init, qca808x_phy_exit},
/*qca808x_end*/
	#else
	{QCA808X_PHY_CHIP, {0}, NULL, NULL, NULL},
	#endif
/*qca808x_start*/
	{MAX_PHY_CHIP, {0}, NULL, NULL, NULL}
};
sw_error_t hsl_phy_api_ops_register(phy_type_t phy_type, hsl_phy_ops_t * phy_api_ops)
{

	ssdk_phy_driver[phy_type].phy_ops = phy_api_ops;

	return SW_OK;

}

sw_error_t hsl_phy_api_ops_unregister(phy_type_t phy_type, hsl_phy_ops_t * phy_api_ops)
{

	ssdk_phy_driver[phy_type].phy_ops = NULL;

	return SW_OK;

}

hsl_phy_ops_t *hsl_phy_api_ops_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	phy_type_t phytype = 0;

	if (dev_id >= SW_MAX_NR_DEV)
		return NULL;

	phytype = phy_info[dev_id]->phy_type[port_id];
	if(phytype == MAX_PHY_CHIP)
	{
		return NULL;
	}

	return ssdk_phy_driver[phytype].phy_ops;

}

sw_error_t phy_api_ops_init(phy_type_t phy_type)
{

	if (MAX_PHY_CHIP <= phy_type)
		return SW_BAD_PARAM;

	if(ssdk_phy_driver[phy_type].phy_ops != NULL)
	{
		kfree(ssdk_phy_driver[phy_type].phy_ops);
		ssdk_phy_driver[phy_type].phy_ops = NULL;
	}
	return SW_OK;
}
/*qca808x_end*/
phy_info_t *hsl_phy_info_get(a_uint32_t dev_id)
{
	return phy_info[dev_id];
}

a_bool_t hsl_port_is_sfp(a_uint32_t dev_id, a_uint32_t port_id)
{
	a_bool_t sfp_port = 0;
	a_uint32_t mode1, mode2;

	sfp_port = hsl_port_feature_get(dev_id, port_id, PHY_F_SFP);
	if (sfp_port == A_TRUE) {
		return A_TRUE;
	}
	else if (A_TRUE == hsl_port_phy_combo_capability_get(dev_id, port_id))
	{
		/* combo port copper mode */
		return A_FALSE;
	}

	mode1 = ssdk_dt_global_get_mac_mode(dev_id, SSDK_UNIPHY_INSTANCE1);
	mode2 = ssdk_dt_global_get_mac_mode(dev_id, SSDK_UNIPHY_INSTANCE2);

	if (((SSDK_PHYSICAL_PORT5 == port_id) &&
			((mode1 == PORT_WRAPPER_10GBASE_R) ||
			(mode1 == PORT_WRAPPER_SGMII_FIBER))) ||
	    ((SSDK_PHYSICAL_PORT6 == port_id) &&
			((mode2 == PORT_WRAPPER_10GBASE_R) ||
			(mode2 == PORT_WRAPPER_SGMII_FIBER))))
		return A_TRUE;
	else
		return A_FALSE;
}

a_bool_t hsl_port_phy_connected(a_uint32_t dev_id, fal_port_t port_id)
{
	a_uint32_t cpu_bmp = ssdk_cpu_bmp_get(dev_id);
	a_bool_t is_fport = hsl_port_feature_get(dev_id, port_id, PHY_F_FORCE);

	if (A_FALSE == hsl_port_validity_check(dev_id, port_id))
		return A_FALSE;

	if ((cpu_bmp & BIT(port_id)) || is_fport == A_TRUE)
		return A_FALSE;

	return A_TRUE;
}

/*qca808x_start*/
a_uint32_t hsl_phyid_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	a_uint16_t org_id = 0, rev_id = 0;
	a_uint32_t reg_pad = 0, phy_id = 0;

/*qca808x_end*/
	if(ssdk_is_emulation(dev_id) && ssdk_emu_chip_ver_get(dev_id) == MP_GEPHY){
		return MP_GEPHY;
	}
	if (hsl_port_is_sfp(dev_id, port_id)){
		return SFP_PHY;
	}
/*qca808x_start*/
	if (phy_info[dev_id]->phy_features[port_id] & PHY_F_CLAUSE45){
		reg_pad = BIT(30) | BIT(16);
	}

	org_id = hsl_phy_mii_reg_read(dev_id,
		phy_info[dev_id]->phy_address[port_id], reg_pad | 2);
	rev_id = hsl_phy_mii_reg_read(dev_id,
		phy_info[dev_id]->phy_address[port_id], reg_pad | 3);

	phy_id = (org_id<<16) | rev_id;
#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port_id) == PHY_I2C_ACCESS) {
		if(phy_id == INVALID_PHY_ID)
			phy_id = QCA8081_PHY_V1_1;
	}
#endif

	return phy_id;
}

phy_type_t hsl_phytype_get_by_phyid(a_uint32_t dev_id, a_uint32_t phy_id)
{
	phy_type_t phytype = MAX_PHY_CHIP;

	switch (phy_id)
	{
/*qca808x_end*/
		case F1V1_PHY:
		case F1V2_PHY:
		case F1V3_PHY:
		case F1V4_PHY:
			phytype = F1_PHY_CHIP;
			break;
		case F2V1_PHY:
			phytype = F2_PHY_CHIP;
			break;
		case MALIBU2PORT_PHY:
		case MALIBU5PORT_PHY:
			phytype = MALIBU_PHY_CHIP;
			break;
		case AQUANTIA_PHY_107:
		case AQUANTIA_PHY_108:
		case AQUANTIA_PHY_109:
		case AQUANTIA_PHY_111:
		case AQUANTIA_PHY_111B0:
		case AQUANTIA_PHY_112:
		case AQUANTIA_PHY_113C_A0:
		case AQUANTIA_PHY_113C_A1:
		case AQUANTIA_PHY_113C_B0:
		case AQUANTIA_PHY_113C_B1:
		case AQUANTIA_PHY_112C:
		case AQUANTIA_PHY_114C_B0:
		case MVL_PHY_X3410:
			phytype = AQUANTIA_PHY_CHIP;
			break;
		case QCA8030_PHY:
		case QCA8033_PHY:
		case QCA8035_PHY:
			phytype = QCA803X_PHY_CHIP;
			break;
		case SFP_PHY:
			phytype = SFP_PHY_CHIP;
			break;
		case MP_GEPHY:
			phytype = MPGE_PHY_CHIP;
			break;
		case QCA8084_PHY:
/*qca808x_start*/
		case QCA8081_PHY_V1_1:
			phytype = QCA808X_PHY_CHIP;
			break;
		default:
			phytype = MAX_PHY_CHIP;
	}

	return phytype;
}
/*qca808x_end*/
sw_error_t hsl_phydriver_update(a_uint32_t dev_id, a_uint32_t port_id)
{
	a_uint32_t phy_id;
	phy_type_t phytype;

	phy_id = hsl_phyid_get(dev_id, port_id);
	phytype = hsl_phytype_get_by_phyid(dev_id, phy_id);
	SSDK_DEBUG("port_id is %x, phy_id is %x, phy_type is:%x\n",
		port_id, phy_id, phytype);
	if (MAX_PHY_CHIP != phytype)
	{
		phy_info[dev_id]->phy_type[port_id] = phytype;
		ssdk_phy_driver[phytype].port_bmp[dev_id] |= BIT(port_id);
	}

	return SW_OK;
}
/*qca808x_start*/
int ssdk_phy_driver_init(a_uint32_t dev_id)
{

	int i = 0;
	a_uint32_t phy_id = 0;
	phy_type_t phytype = MAX_PHY_CHIP;

	for (i = 0; i < SW_MAX_NR_PORT; i++)
	{
		if (port_bmp[dev_id] & (0x1 << i))
		{
/*qca808x_end*/
			if(hsl_port_feature_get(dev_id, i, PHY_F_FORCE)) {
				continue;
			}
/*qca808x_start*/
			phy_id = hsl_phyid_get(dev_id, i);
			phytype = hsl_phytype_get_by_phyid(dev_id, phy_id);
			if (MAX_PHY_CHIP != phytype) {
				phy_info[dev_id]->phy_type[i] = phytype;
				ssdk_phy_driver[phytype].port_bmp[dev_id] |= (0x1 << i);
			} else {
				SSDK_INFO("dev_id = %d, phy_adress = %d, phy_id = 0x%x phy"
					"type doesn't match\n", dev_id,
					phy_info[dev_id]->phy_address[i], phy_id);
			}
		}
	}

	for(i = 0; i < MAX_PHY_CHIP;i++) {
		if(ssdk_phy_driver[i].port_bmp[dev_id] != 0 &&
			ssdk_phy_driver[i].init != NULL) {
			ssdk_phy_driver[i].init(dev_id, ssdk_phy_driver[i].port_bmp[dev_id]);
		}
	}
	return 0;
}

#ifdef QCA808X_PORTS_INFO
typedef struct {
	a_uint32_t port_id;
	a_uint32_t phy_address;
	a_uint8_t phy_access_type;
} qca808x_phy_info_t;
/*5 is port_id, 0x1c is qca808x phy address, PHY_MDIO_ACCESS is mdio
mode to access qca808x phy, PHY_I2C_ACCESS is I2C mode to access
qca808x phy*/
static qca808x_phy_info_t qca808x_phy_info[] = {
	{5,0x7c,PHY_I2C_ACCESS}
};
static int qca_ssdk_qca808x_phy_info_init(a_uint32_t dev_id)
{
	a_uint32_t port_bmp = 0, port_id = 0, port_index = 0, port_index_max = 0;

	port_index_max = sizeof(qca808x_phy_info)/(sizeof(qca808x_phy_info_t));
	for(port_index = 0; port_index < port_index_max; port_index++) {
		port_id = qca808x_phy_info[port_index].port_id;
		port_bmp |= (1 << port_id);
		/*qca808x phy address*/
		phy_info[dev_id]->phy_address[port_id] =
			qca808x_phy_info[port_index].phy_address;
		/*qca808x access mode, 1:i2c, 0:mdio*/
		if(qca808x_phy_info[port_index].phy_access_type == PHY_I2C_ACCESS)
			hsl_port_feature_set(dev_id, port_id, PHY_F_I2C);
		else
			hsl_port_feature_clear(dev_id, port_id, PHY_F_I2C);
	}
	qca_ssdk_port_bmp_set(dev_id, port_bmp);

	return 0;
}
#endif

int qca_ssdk_phy_info_init(a_uint32_t dev_id)
{
	a_uint32_t j = 0;
	phy_info_t *phy_information;

	phy_information = kzalloc(sizeof(phy_info_t), GFP_KERNEL);
	if (phy_information == NULL) {
		SSDK_ERROR("phy_information kzalloc failed!\n");
		return -ENOMEM;
	}
	memset(phy_information, 0, sizeof(*phy_information));
	phy_info[dev_id] = phy_information;

	for (j = SSDK_PHYSICAL_PORT0; j < SW_MAX_NR_PORT; j ++)
	{
		phy_info[dev_id]->phy_type[j] = MAX_PHY_CHIP;
		phy_info[dev_id]->port_link_status[j] = PORT_LINK_DOWN;
		phy_info[dev_id]->port_mode[j] = PORT_INTERFACE_MODE_MAX;
		if(j == SSDK_PHYSICAL_PORT0)
		{
			phy_info[dev_id]->phy_address[j] = INVALID_PHY_ADDR;
		}
		else
		{
			phy_info[dev_id]->phy_address[j] = j - 1;
		}
	}
#ifdef QCA808X_PORTS_INFO
	qca_ssdk_qca808x_phy_info_init(dev_id);
#endif

	return 0;
}
void qca_ssdk_port_bmp_init(a_uint32_t dev_id)
{
	port_bmp[dev_id] = 0x3e;

	return;
}
/*qca808x_end*/
void hsl_phy_address_init(a_uint32_t dev_id, a_uint32_t i,
			a_uint32_t value)
{
	phy_info[dev_id]->phy_address[i] = value;

	return;
}
/*qca808x_start*/
void qca_ssdk_port_bmp_set(a_uint32_t dev_id, a_uint32_t value)
{
	port_bmp[dev_id] = value;

	return;
}

a_uint32_t qca_ssdk_port_bmp_get(a_uint32_t dev_id)
{
	return port_bmp[dev_id];
}
/*qca808x_end*/
a_uint32_t qca_ssdk_phy_type_port_bmp_get(a_uint32_t dev_id,
				phy_type_t phy_type)
{

	return ssdk_phy_driver[phy_type].port_bmp[dev_id];
}

void
qca_ssdk_phy_address_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t phy_addr)
{
	 phy_info[dev_id]->phy_address[port_id] = phy_addr;

	 return;
}

#if defined(IN_PHY_I2C_MODE)
a_uint32_t
qca_ssdk_port_to_phy_mdio_fake_addr(a_uint32_t dev_id, a_uint32_t port_id)
{
	return phy_info[dev_id]->phy_mdio_fake_address[port_id];
}

void qca_ssdk_phy_mdio_fake_address_set(a_uint32_t dev_id, a_uint32_t i,
			a_uint32_t value)
{
	phy_info[dev_id]->phy_mdio_fake_address[i] = value;

	return;
}
#endif
/*qca808x_start*/
a_uint32_t
qca_ssdk_port_to_phy_addr(a_uint32_t dev_id, a_uint32_t port_id)
{
	return phy_info[dev_id]->phy_address[port_id];
}

a_uint32_t
qca_ssdk_phy_addr_to_port(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint32_t i = 0;

	for (i = 0; i < SW_MAX_NR_PORT; i ++)
	{
		if (phy_info[dev_id]->phy_address[i] == phy_addr)
			return i;
#if defined(IN_PHY_I2C_MODE)
		/*for the case that IN_PHY_I2C_MODE was enabled,
		if port id was not found, the mdio fake address can be used*/
		if (phy_info[dev_id]->phy_mdio_fake_address[i] == TO_PHY_ADDR(phy_addr))
			return i;
#endif
	}
	SSDK_DEBUG("doesn't match port_id to specified phy_addr !\n");
	return 0;
}

a_uint32_t
qca_ssdk_phydev_to_port(a_uint32_t dev_id, struct phy_device *phydev)
{
	a_uint32_t miibus_id = 0, phy_addr_e = 0;

	miibus_id = ssdk_miibus_index_get(dev_id, phydev->mdio.bus);
	phy_addr_e = TO_PHY_ADDR_E(phydev->mdio.addr, miibus_id);

	return qca_ssdk_phy_addr_to_port(dev_id, phy_addr_e);
}

a_bool_t
qca_ssdk_gpio_is_invalid(a_uint32_t dev_id, a_uint32_t gpio_num)
{
	return ((gpio_num == SSDK_MAX_GPIO) || (gpio_num == SSDK_INVALID_GPIO));
}

a_bool_t
hsl_port_phy_combo_capability_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	if (dev_id >= SW_MAX_NR_DEV)
		return A_FALSE;

	return hsl_port_feature_get(dev_id, port_id, PHY_F_COMBO);
}

void
hsl_port_phy_combo_capability_set(a_uint32_t dev_id, a_uint32_t port_id,
		a_bool_t enable)
{
	if (dev_id >= SW_MAX_NR_DEV)
		return;
	if(enable)
		hsl_port_feature_set(dev_id, port_id, PHY_F_COMBO);
	else
		hsl_port_feature_clear(dev_id, port_id, PHY_F_COMBO);

	return;
}

a_uint8_t
hsl_port_phy_access_type_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	if (dev_id >= SW_MAX_NR_DEV)
		return 0;
	if(hsl_port_feature_get(dev_id, port_id, PHY_F_I2C))
		return PHY_I2C_ACCESS;

	return PHY_MDIO_ACCESS;
}

void
hsl_port_phy_access_type_set(a_uint32_t dev_id, a_uint32_t port_id,
		a_uint8_t access_type)
{
	if (dev_id >= SW_MAX_NR_DEV)
		return;

	if(access_type == PHY_I2C_ACCESS)
		hsl_port_feature_set(dev_id, port_id, PHY_F_I2C);
	else
		hsl_port_feature_clear(dev_id, port_id, PHY_F_I2C);

	return;
}
/*qca808x_end*/
void
hsl_port_phy_c45_capability_set(a_uint32_t dev_id, a_uint32_t port_id,
		a_bool_t enable)
{
	if(enable)
		hsl_port_feature_set(dev_id, port_id, PHY_F_CLAUSE45);
	else
		hsl_port_feature_clear(dev_id, port_id, PHY_F_CLAUSE45);

	return;
}
sw_error_t
hsl_port_phy_serdes_reset(a_uint32_t dev_id)
{
	int i = 0;
	hsl_phy_ops_t *phy_drv;

	for (i = 0; i < SW_MAX_NR_PORT; i++)
	{
		if (phy_info[dev_id]->phy_type[i] == MALIBU_PHY_CHIP)
		{
			/*if phy_drv is null or phy_serdes_reset is null, then no need to do
			 serdes reset*/
			phy_drv = hsl_phy_api_ops_get (dev_id, i);
			if (phy_drv && phy_drv->phy_serdes_reset)
			{
				return phy_drv->phy_serdes_reset(dev_id);
			}
		}
	}

	return SW_OK;
}

sw_error_t hsl_port_phy_hw_init(a_uint32_t dev_id, a_uint32_t port_id)
{
	phy_type_t phytype;

	phytype = hsl_phy_type_get(dev_id, port_id);

	if(ssdk_phy_driver[phytype].port_bmp[dev_id] != 0 &&
			ssdk_phy_driver[phytype].init != NULL)
	{
		ssdk_phy_driver[phytype].init(dev_id,
			ssdk_phy_driver[phytype].port_bmp[dev_id]);
	}

	return SW_OK;
}
/*qca808x_start*/
a_uint32_t
hsl_port_phyid_get(a_uint32_t dev_id, fal_port_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr, phy_id;
	hsl_phy_ops_t *phy_drv;

	phy_drv = hsl_phy_api_ops_get (dev_id, port_id);
	if (phy_drv == NULL) {
		return INVALID_PHY_ID;
	}
	if (NULL == phy_drv->phy_id_get) {
		return INVALID_PHY_ID;
	}

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	if(rv) {
		return INVALID_PHY_ID;
	}

	rv = phy_drv->phy_id_get (dev_id, phy_addr, &phy_id);
	if(rv) {
		return INVALID_PHY_ID;
	}

	return phy_id;
}
/*qca808x_end*/
sw_error_t
hsl_port_phy_mode_set(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_interface_mode_t mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv;

	phy_drv = hsl_phy_api_ops_get (dev_id, port_id);
	if (NULL == phy_drv || NULL == phy_drv->phy_interface_mode_set)
	{
		/*PHY driver did not register phy_interface_mode_set,
		so no need to configure PHY interface mode*/
		return SW_OK;
	}

	phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
	rv = phy_drv->phy_interface_mode_set(dev_id, phy_addr, mode);

	return rv;
}

phy_type_t hsl_phy_type_get(a_uint32_t dev_id, a_uint32_t port_id)
{

	if (dev_id >= SW_MAX_NR_DEV)
		return MAX_PHY_CHIP;

	return phy_info[dev_id]->phy_type[port_id];
}

a_uint32_t hsl_port_phy_reset_gpio_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	return phy_info[dev_id]->phy_reset_gpio[port_id];
}

void hsl_port_phy_reset_gpio_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t phy_reset_gpio)
{
	phy_info[dev_id]->phy_reset_gpio[port_id] = phy_reset_gpio;

	return;
}

void hsl_port_phy_gpio_reset(a_uint32_t dev_id, a_uint32_t port_id)

{
	a_uint32_t gpio_num, ret = 0;

	gpio_num = hsl_port_phy_reset_gpio_get(dev_id, port_id);

	if(qca_ssdk_gpio_is_invalid(dev_id, gpio_num))
	{
		return;
	}
	ret = gpio_request(gpio_num, "phy_reset_gpio");
	if(ret)
	{
		SSDK_ERROR("gpio request failed, ret:%d\n", ret);
		return;
	}
	ret = gpio_direction_output(gpio_num, SSDK_GPIO_RESET);
	if(ret)
	{
		SSDK_ERROR("when reset, gpio set failed, ret:%d\n", ret);
		return;
	}
	gpio_set_value(gpio_num, SSDK_GPIO_RESET);
	msleep(200);
	gpio_set_value(gpio_num, SSDK_GPIO_RELEASE);
	msleep(10);
	SSDK_INFO("GPIO%d reset Port %d done\n", gpio_num, port_id);

	gpio_free(gpio_num);

	return;
}

void
hsl_port_phy_dac_get(a_uint32_t dev_id, a_uint32_t port_id,
	phy_dac_t *phy_dac)
{
	phy_dac->mdac = phy_info[dev_id]->phy_dac[port_id].mdac;
	phy_dac->edac = phy_info[dev_id]->phy_dac[port_id].edac;

	return;
}

void
hsl_port_phy_dac_set(a_uint32_t dev_id, a_uint32_t port_id,
	phy_dac_t phy_dac)
{
	phy_info[dev_id]->phy_dac[port_id].mdac = phy_dac.mdac;
	phy_info[dev_id]->phy_dac[port_id].edac = phy_dac.edac;

	return;
}

sw_error_t
hsl_phy_phydev_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	struct phy_device **phydev)
{
	a_uint32_t pdev_addr;
	const char *pdev_name;
	struct mii_bus *miibus = ssdk_phy_miibus_get(dev_id, phy_addr);

	SW_RTN_ON_NULL(phydev);
	SW_RTN_ON_NULL(miibus);
#if (LINUX_VERSION_CODE < KERNEL_VERSION (5, 0, 0))
	*phydev = miibus->phy_map[phy_addr];
	if(*phydev == NULL)
	{
		SSDK_ERROR("phy_addr %d phydev is NULL\n", phy_addr);
		return SW_NOT_INITIALIZED;
	}
	pdev_addr = (*phydev)->addr;
	pdev_name = dev_name(&((*phydev)->dev));
#else
	phy_addr = TO_PHY_ADDR(phy_addr);
	*phydev = mdiobus_get_phy(miibus, phy_addr);
	if(*phydev == NULL)
	{
		SSDK_ERROR("phy_addr %d phydev is NULL\n", phy_addr);
		return SW_NOT_INITIALIZED;
	}
	pdev_addr = (*phydev)->mdio.addr;
	pdev_name = phydev_name(*phydev);
#endif
	SSDK_DEBUG("phy[%d]: device %s, driver %s\n",
		pdev_addr, pdev_name,
		(*phydev)->drv ? (*phydev)->drv->name : "unknown");

	return SW_OK;
}

sw_error_t
hsl_port_phydev_get(a_uint32_t dev_id, a_uint32_t port_id,
	struct phy_device **phydev)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr;
	SW_RTN_ON_NULL(phydev);

#if defined(IN_PHY_I2C_MODE)
	if (hsl_port_phy_access_type_get(dev_id, port_id) == PHY_I2C_ACCESS)
	{
		phy_addr = qca_ssdk_port_to_phy_mdio_fake_addr(dev_id, port_id);
	}
	else
#endif
	{
		phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
	}

	rv = hsl_phy_phydev_get(dev_id, phy_addr, phydev);
	SW_RTN_ON_ERROR(rv);

	return SW_OK;
}

sw_error_t
hsl_phy_adv_to_linkmode_adv(a_uint32_t autoadv, a_ulong_t *advertising)
{
	linkmode_mod_bit(ETHTOOL_LINK_MODE_Pause_BIT,
			advertising, autoadv & FAL_PHY_ADV_PAUSE);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT,
			advertising, autoadv & FAL_PHY_ADV_ASY_PAUSE);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
			advertising, autoadv & FAL_PHY_ADV_10T_HD);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
			advertising, autoadv & FAL_PHY_ADV_10T_FD);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
			advertising, autoadv & FAL_PHY_ADV_100TX_HD);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
			advertising, autoadv & FAL_PHY_ADV_100TX_FD);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			advertising, autoadv & FAL_PHY_ADV_1000T_FD);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			advertising, autoadv & FAL_PHY_ADV_2500T_FD);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
			advertising, autoadv & FAL_PHY_ADV_5000T_FD);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
			advertising, autoadv & FAL_PHY_ADV_10000T_FD);

	SSDK_DEBUG("autoadv:0x%x, advertising::0x%lx\n",
		autoadv, *advertising);

	return SW_OK;
}

sw_error_t
hsl_phy_linkmode_adv_to_adv(a_ulong_t *advertising, a_uint32_t *autoadv)
{
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_Pause_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_PAUSE;
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_ASY_PAUSE;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_10T_HD;
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_10T_FD;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_100TX_HD;
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_100TX_FD;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_1000T_FD;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_2500T_FD;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_5000T_FD;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT, advertising))
		*autoadv |= FAL_PHY_ADV_10000T_FD;

	return SW_OK;
}

a_bool_t
hsl_phy_autoneg_adv_check(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t adv_supported = 0;
	struct phy_device *phydev = NULL;

	rv = hsl_phy_phydev_get(dev_id, phy_addr, &phydev);
	if(rv == SW_OK) {
		if(!phydev->drv)
			return A_TRUE;
		hsl_phy_linkmode_adv_to_adv(phydev->supported, &adv_supported);
		if((adv & adv_supported) == adv)
			return A_TRUE;
	}

	return A_FALSE;
}

sw_error_t
hsl_phy_phydev_autoneg_update(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t autoneg_en, a_uint32_t autoadv)
{
	sw_error_t rv = SW_OK;
	struct phy_device *phydev;
	a_uint32_t port_id = qca_ssdk_phy_addr_to_port(dev_id, phy_addr);

#if defined(IN_PHY_I2C_MODE)
	/*if phy is accessed by I2C, so the phydev address is mdio fake address*/
	if (hsl_port_phy_access_type_get(dev_id, port_id) == PHY_I2C_ACCESS)
		phy_addr = qca_ssdk_port_to_phy_mdio_fake_addr(dev_id, port_id);
#endif
	rv = hsl_phy_phydev_get(dev_id, phy_addr, &phydev);
	SW_RTN_ON_ERROR(rv);
	if(autoneg_en)
	{
		if(autoadv != 0)
		{
			rv = hsl_phy_adv_to_linkmode_adv(autoadv, phydev->advertising);
			SW_RTN_ON_ERROR(rv);
		}
	}
	/*PHY port need to update autoneg*/
	if(!hsl_port_is_sfp(dev_id, port_id))
	{
		phydev->autoneg = autoneg_en;
	}

	return SW_OK;
}

a_uint32_t
hsl_phy_speed_duplex_to_auto_adv(a_uint32_t dev_id,fal_port_speed_t speed,
	fal_port_duplex_t duplex)
{
	a_uint32_t auto_adv = 0;

	switch(speed)
	{
		case FAL_SPEED_10:
			if(duplex == FAL_FULL_DUPLEX)
				auto_adv = FAL_PHY_ADV_10T_FD;
			else
				auto_adv = FAL_PHY_ADV_10T_HD;
			break;
		case FAL_SPEED_100:
			if(duplex == FAL_FULL_DUPLEX)
				auto_adv =FAL_PHY_ADV_100TX_FD;
			else
				auto_adv =FAL_PHY_ADV_100TX_HD;
			break;
		case FAL_SPEED_1000:
			auto_adv = FAL_PHY_ADV_1000T_FD;
			break;
		case FAL_SPEED_2500:
			auto_adv = FAL_PHY_ADV_2500T_FD;
			break;
		case FAL_SPEED_5000:
			auto_adv = FAL_PHY_ADV_5000T_FD;
			break;
		case FAL_SPEED_10000:
			auto_adv = FAL_PHY_ADV_10000T_FD;
			break;
		default:
			break;
	}

	return auto_adv;
}
#ifdef IN_LED
sw_error_t
hsl_port_phy_led_ctrl_pattern_get(a_uint32_t dev_id, led_pattern_group_t group,
	a_uint32_t port_id, led_ctrl_pattern_t * pattern)
{
	sw_error_t rv = SW_OK;

	if(group != LED_MAC_PORT_GROUP)
	{
		SSDK_ERROR("group %x is not supported\n", group);
		return SW_NOT_SUPPORTED;
	}
	rv = hsl_port_phy_led_source_pattern_get(dev_id, port_id, 0, pattern);

	return rv;
}

sw_error_t
hsl_port_phy_led_ctrl_pattern_set(a_uint32_t dev_id, led_pattern_group_t group,
	a_uint32_t port_id, led_ctrl_pattern_t * pattern)
{
	a_uint32_t led_src = 0;

	if(group != LED_MAC_PORT_GROUP)
	{
		SSDK_ERROR("group %x is not supported\n", group);
		return SW_NOT_SUPPORTED;
	}
	for(led_src = 0; led_src < PORT_LED_SOURCE_MAX; led_src++) {
		hsl_port_phy_led_source_pattern_set(dev_id, port_id, led_src,
			pattern);
	}

	return SW_OK;
}

sw_error_t
hsl_port_phy_led_source_pattern_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t source_id, led_ctrl_pattern_t * pattern)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}
	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	SW_RTN_ON_NULL (phy_drv->phy_led_ctrl_source_set);
	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR(rv);
	rv = phy_drv->phy_led_ctrl_source_set(dev_id, phy_addr, source_id,
		pattern);

	return rv;
}

sw_error_t
hsl_port_phy_led_source_pattern_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t source_id, led_ctrl_pattern_t * pattern)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}
	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	SW_RTN_ON_NULL (phy_drv->phy_led_ctrl_source_get);
	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR(rv);
	rv = phy_drv->phy_led_ctrl_source_get(dev_id, phy_addr, source_id,
		pattern);

	return rv;
}
#endif
sw_error_t
hsl_port_phydev_get_status(a_uint32_t dev_id, a_uint32_t port_id,
	struct port_phy_status *phy_status)
{
	sw_error_t rv = SW_OK;
	struct phy_device *phydev = NULL;
	a_uint32_t pause = 0, asym_pause = 0, lp_pause = 0, lp_asym_pause = 0;

	rv = hsl_port_phydev_get(dev_id, port_id, &phydev);
	SW_RTN_ON_ERROR(rv);
	if(!phydev)
	{
		SSDK_ERROR("port %d phydev is not initialized\n", port_id);
		return SW_NOT_INITIALIZED;
	}

	mutex_lock(&phydev->lock);
	phy_status->link_status = phydev->link;
	phy_status->speed = phydev->speed;
	phy_status->duplex = phydev->duplex;
	pause = linkmode_test_bit(ETHTOOL_LINK_MODE_Pause_BIT, phydev->advertising);
	asym_pause = linkmode_test_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT,
		phydev->advertising);
	lp_pause = phydev->pause;
	lp_asym_pause = phydev->asym_pause;
	mutex_unlock(&phydev->lock);

	phy_status->rx_flowctrl = A_FALSE;
	phy_status->tx_flowctrl = A_FALSE;
	if(pause && asym_pause && !lp_pause && lp_asym_pause)
		phy_status->rx_flowctrl = A_TRUE;
	if(!pause && asym_pause && lp_pause && lp_asym_pause)
		phy_status->tx_flowctrl = A_TRUE;
	if((pause && !asym_pause && lp_pause) || (pause && asym_pause && lp_pause))
	{
		phy_status->rx_flowctrl = A_TRUE;
		phy_status->tx_flowctrl = A_TRUE;
	}
	SSDK_DEBUG("dev_id:%d, port:%d, link:%d, speed:%d, duplex:%d,"
		"rx_flowctrl:%d, tx_flowctrl:%d", dev_id, port_id,
		phy_status->link_status, phy_status->speed, phy_status->duplex,
		phy_status->rx_flowctrl, phy_status->tx_flowctrl);

	return SW_OK;
}

sw_error_t
hsl_port_phy_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	struct port_phy_status *phy_status)
{
	sw_error_t rv = 0;
	a_uint32_t phy_id;
	hsl_phy_ops_t *phy_drv;

	phy_drv = hsl_phy_api_ops_get (dev_id, port_id);
	if(phy_drv && phy_drv->phy_get_status)
	{
		rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_id);
		SW_RTN_ON_ERROR (rv);
		rv = phy_drv->phy_get_status(dev_id, phy_id, phy_status);
		SW_RTN_ON_ERROR (rv);
	}
	else
	{
		rv = hsl_port_phydev_get_status(dev_id, port_id, phy_status);
		SW_RTN_ON_ERROR (rv);
	}

	return SW_OK;
}

sw_error_t
hsl_port_phy_function_reset(a_uint32_t dev_id, a_uint32_t port_id)
{
	sw_error_t rv = 0;
	a_uint32_t phy_addr;
	hsl_phy_ops_t *phy_drv;

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_function_reset)
		return SW_NOT_SUPPORTED;
	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);
	rv = phy_drv->phy_function_reset(dev_id, phy_addr, PHY_FIFO_RESET);
	SW_RTN_ON_ERROR(rv);

	return rv;
}

a_uint32_t hsl_port_mode_to_phydev_interface(a_uint32_t dev_id,
	a_uint32_t port_mode)
{
	a_uint32_t interface = PHY_INTERFACE_MODE_MAX;

	switch(port_mode)
	{
		case PHY_SGMII_BASET:
			interface = PHY_INTERFACE_MODE_SGMII;
			break;
		case PORT_SGMII_PLUS:
			interface = PHY_INTERFACE_MODE_2500BASEX;
			break;
		case PORT_SGMII_FIBER:
			interface = PHY_INTERFACE_MODE_1000BASEX;
			break;
		case PORT_USXGMII:
			interface = PHY_INTERFACE_MODE_USXGMII;
			break;
		case PORT_10GBASE_R:
			interface = PHY_INTERFACE_MODE_10GKR;
			break;
		case PORT_QSGMII:
			interface = PHY_INTERFACE_MODE_QSGMII;
			break;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
		case PORT_UQXGMII:
			interface = PHY_INTERFACE_MODE_QUSGMII;
			break;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0))
		case PHY_PSGMII_BASET:
			interface = PHY_INTERFACE_MODE_PSGMII;
			break;
#endif
		default:
			break;
	}

	return interface;
}

a_uint32_t hsl_port_mode_to_uniphy_mode(a_uint32_t dev_id,
	a_uint32_t port_mode)
{
	a_uint32_t uniphy_mode = PORT_WRAPPER_MAX;

	switch(port_mode)
	{
		case PHY_SGMII_BASET:
		case PORT_SGMII_FIBER:
			uniphy_mode = PORT_WRAPPER_SGMII_CHANNEL0;
			break;
		case PORT_SGMII_PLUS:
			uniphy_mode = PORT_WRAPPER_SGMII_PLUS;
			break;
		case PORT_USXGMII:
			uniphy_mode = PORT_WRAPPER_USXGMII;
			break;
		case PORT_10GBASE_R:
			uniphy_mode = PORT_WRAPPER_10GBASE_R;
			break;
		case PORT_QSGMII:
			uniphy_mode = PORT_WRAPPER_QSGMII;
			break;
		case PORT_UQXGMII:
			uniphy_mode = PORT_WRAPPER_UQXGMII;
			break;
		case PHY_PSGMII_BASET:
			uniphy_mode = PORT_WRAPPER_PSGMII;
			break;
		default:
			break;
	}

	return uniphy_mode;
}

a_uint32_t hsl_uniphy_mode_to_port_mode(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t uniphy_mode)
{
	a_uint32_t port_mode = 0;

	switch(uniphy_mode)
	{
		case PORT_WRAPPER_PSGMII:
		case PORT_WRAPPER_PSGMII_FIBER:
			if(port_id >= SSDK_PHYSICAL_PORT1 && port_id <= SSDK_PHYSICAL_PORT4)
				port_mode = PHY_PSGMII_BASET;
			if(port_id == SSDK_PHYSICAL_PORT5) {
				if(uniphy_mode == PORT_WRAPPER_PSGMII)
					port_mode = PHY_PSGMII_BASET;
				else
					port_mode = PHY_PSGMII_FIBER;
			}
			break;
		case PORT_WRAPPER_QSGMII:
			port_mode = PORT_QSGMII;
			break;
		case PORT_WRAPPER_SGMII_PLUS:
			port_mode = PORT_SGMII_PLUS;
			break;
		case PORT_WRAPPER_USXGMII:
			port_mode = PORT_USXGMII;
			break;
		case PORT_WRAPPER_10GBASE_R:
			port_mode = PORT_10GBASE_R;
			break;
		case PORT_WRAPPER_SGMII_CHANNEL0:
		case PORT_WRAPPER_SGMII_CHANNEL1:
		case PORT_WRAPPER_SGMII_CHANNEL4:
			port_mode = PHY_SGMII_BASET;
			break;
		case PORT_WRAPPER_SGMII_FIBER:
			port_mode = PORT_SGMII_FIBER;
			break;
		case PORT_WRAPPER_UQXGMII:
		case PORT_WRAPPER_UDXGMII:
			port_mode = PORT_UQXGMII;
			break;
		case PORT_WRAPPER_MAX:
			port_mode = PORT_INTERFACE_MODE_MAX;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return port_mode;
}

a_uint32_t hsl_port_to_uniphy(a_uint32_t dev_id, a_uint32_t port_id)
{
	a_uint32_t uniphy_index = SSDK_MAX_UNIPHY_INSTANCE;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		SSDK_ERROR("port%d is not supported\n", port_id);
		return SSDK_MAX_UNIPHY_INSTANCE;
	}

	if(port_id <= SSDK_PHYSICAL_PORT4)
	{
		uniphy_index = SSDK_UNIPHY_INSTANCE0;
#ifdef MPPE
		if(port_id == SSDK_PHYSICAL_PORT2)
			uniphy_index = SSDK_UNIPHY_INSTANCE1;
#endif
	}
	else if(port_id == SSDK_PHYSICAL_PORT5)
	{
		if(ssdk_dt_global_get_mac_mode(dev_id, SSDK_UNIPHY_INSTANCE1) !=
			PORT_WRAPPER_MAX)
			uniphy_index = SSDK_UNIPHY_INSTANCE1;
		else
			uniphy_index = SSDK_UNIPHY_INSTANCE0;
	}
	else if(port_id == SSDK_PHYSICAL_PORT6)
		uniphy_index = SSDK_UNIPHY_INSTANCE2;

	return uniphy_index;
}

static sw_error_t
hsl_port_phydev_interface_mode_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_interface_mode_t *interface_mode_status)
{
	sw_error_t rv = SW_OK;
	a_uint32_t interface = 0;
	struct phy_device *phydev = NULL;

	rv = hsl_port_phydev_get(dev_id, port_id, &phydev);
	SW_RTN_ON_ERROR(rv);
	if(!phydev)
	{
		SSDK_ERROR("port %d phydev is not initialized\n", port_id);
		return SW_NOT_INITIALIZED;
	}
	mutex_lock(&phydev->lock);
	interface = phydev->interface;
	mutex_unlock(&phydev->lock);
	switch(interface)
	{
		case PHY_INTERFACE_MODE_SGMII:
			*interface_mode_status = PHY_SGMII_BASET;
			break;
		case PHY_INTERFACE_MODE_2500BASEX:
			*interface_mode_status = PORT_SGMII_PLUS;
			break;
		case PHY_INTERFACE_MODE_1000BASEX:
			*interface_mode_status = PORT_SGMII_FIBER;
			break;
		case PHY_INTERFACE_MODE_USXGMII:
			*interface_mode_status = PORT_USXGMII;
			break;
		case PHY_INTERFACE_MODE_10GKR:
			*interface_mode_status = PORT_10GBASE_R;
			break;
		case PHY_INTERFACE_MODE_QSGMII:
			*interface_mode_status = PORT_QSGMII;
			break;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
		case PHY_INTERFACE_MODE_QUSGMII:
			*interface_mode_status = PORT_UQXGMII;
			break;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0))
		case PHY_INTERFACE_MODE_PSGMII:
			*interface_mode_status = PHY_PSGMII_BASET;
			break;
#endif
		default:
			break;
	}
	SSDK_DEBUG("dev_id:%d, port:%d, phydev interface:0x%x, "
		"ssdk interface:0x%x\n", dev_id, port_id, interface,
		*interface_mode_status);

	return SW_OK;
}

/*qca808x_start*/
sw_error_t
hsl_port_phy_interface_mode_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_interface_mode_t *interface_mode_status)
{
	sw_error_t rv = 0;
	a_uint32_t phy_addr;
	hsl_phy_ops_t *phy_drv;

	phy_drv = hsl_phy_api_ops_get(dev_id, port_id);
	if(phy_drv && phy_drv->phy_interface_mode_status_get)
	{
		rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
		SW_RTN_ON_ERROR (rv);
		rv = phy_drv->phy_interface_mode_status_get(dev_id, phy_addr,
			interface_mode_status);
		SW_RTN_ON_ERROR(rv);
	}
/*qca808x_end*/
	else
	{
		rv = hsl_port_phydev_interface_mode_status_get(dev_id, port_id,
			interface_mode_status);
		SW_RTN_ON_ERROR(rv);
	}
/*qca808x_start*/
	return SW_OK;
}

sw_error_t
hsl_port_phy_autoadv_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t *autoadv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}
	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_autoneg_adv_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	*autoadv = 0;
	return phy_drv->phy_autoneg_adv_get(dev_id, phy_addr, autoadv);
}

sw_error_t
hsl_port_phy_autoadv_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t autoadv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv  = NULL;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}
	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_autoneg_adv_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);
	return phy_drv->phy_autoneg_adv_set(dev_id, phy_addr, autoadv);
}

sw_error_t
hsl_port_phy_autoneg_restart(a_uint32_t dev_id, a_uint32_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_restart_autoneg)
		return SW_NOT_SUPPORTED;
	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_restart_autoneg(dev_id, phy_addr);
}

sw_error_t
hsl_port_phy_autoneg_status_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * status)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(status);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	if (hsl_port_feature_get(dev_id, port_id, PHY_F_FORCE) == A_TRUE) {
		*status = A_FALSE;
		return SW_OK;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_autoneg_status_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	*status = phy_drv->phy_autoneg_status_get (dev_id, phy_addr);

	return SW_OK;
}

sw_error_t
hsl_port_phy_autoneg_enable(a_uint32_t dev_id, fal_port_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_autoneg_enable_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_autoneg_enable_set (dev_id, phy_addr);
}

sw_error_t
hsl_port_phy_duplex_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t duplex)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_duplex_set)
		return SW_NOT_SUPPORTED;
	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_duplex_set (dev_id, phy_addr, duplex);
}

sw_error_t
hsl_port_phy_duplex_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_duplex_t *duplex)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;
	a_bool_t link_status = A_FALSE;

	*duplex = FAL_DUPLEX_BUTT;
	rv = hsl_port_phy_link_status_get(dev_id, port_id, &link_status);
	SW_RTN_ON_ERROR(rv);
	if(!link_status)
		return SW_OK;

	HSL_DEV_ID_CHECK(dev_id);
	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_duplex_get)
		return SW_NOT_SUPPORTED;
	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_duplex_get (dev_id, phy_addr, duplex);
}

sw_error_t
hsl_port_phy_cdt(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t mdi_pair,
	fal_cable_status_t * cable_status, a_uint32_t * cable_len)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(cable_status);
	SW_RTN_ON_NULL(cable_len);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	  {
		return SW_BAD_PARAM;
	  }

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_cdt)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_cdt (dev_id, phy_addr, mdi_pair, cable_status, cable_len);
}

sw_error_t
hsl_port_phy_link_status_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * status)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(status);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}
	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_link_status_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	*status = phy_drv->phy_link_status_get (dev_id, phy_addr);

	return SW_OK;

}

sw_error_t
hsl_port_phy_power_on(a_uint32_t dev_id, fal_port_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_power_on)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_power_on(dev_id, phy_addr);
}

sw_error_t
hsl_port_phy_power_off(a_uint32_t dev_id, fal_port_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_power_off)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_power_off(dev_id, phy_addr);

}

sw_error_t
hsl_port_phy_speed_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t speed)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_speed_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_speed_set (dev_id, phy_addr, speed);
}

sw_error_t
hsl_port_phy_speed_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_speed_t * pspeed)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;
	a_bool_t link_status = A_FALSE;

	*pspeed = FAL_SPEED_BUTT;
	rv = hsl_port_phy_link_status_get(dev_id, port_id, &link_status);
	SW_RTN_ON_ERROR(rv);
	if(!link_status)
		return SW_OK;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(pspeed);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_speed_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);
	return phy_drv->phy_speed_get (dev_id, phy_addr, pspeed);
}
/*qca808x_end*/
sw_error_t
hsl_port_phy_pll_on(a_uint32_t dev_id, a_uint32_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
		return SW_BAD_PARAM;

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_pll_on)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	rv = phy_drv->phy_pll_on(dev_id, phy_addr);
	return rv;
}

sw_error_t
hsl_port_phy_pll_off(a_uint32_t dev_id, a_uint32_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
		return SW_BAD_PARAM;

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_pll_off)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	rv = phy_drv->phy_pll_off(dev_id, phy_addr);
	return rv;
}

sw_error_t
hsl_port_phy_ldo_set(a_uint32_t dev_id, a_uint32_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
		return SW_BAD_PARAM;

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_ldo_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	rv = phy_drv->phy_ldo_set(dev_id, phy_addr, enable);
	return rv;
}

sw_error_t
hsl_port_phy_combo_prefer_medium_set(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_medium_t medium)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_combo_prefer_medium_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_combo_prefer_medium_set (dev_id, phy_addr, medium);
}

sw_error_t
hsl_port_phy_combo_prefer_medium_get(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_medium_t *medium)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(medium);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_combo_prefer_medium_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_combo_prefer_medium_get (dev_id, phy_addr, medium);
}
/*qca808x_start*/
sw_error_t
hsl_port_phy_reset(a_uint32_t dev_id, fal_port_t port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_reset)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_reset(dev_id, phy_addr);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0))
sw_error_t
hsl_phydev_eee_update(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t adv)
{
	struct phy_device *phydev = NULL;
	sw_error_t rv = SW_OK;

	rv = hsl_phy_phydev_get(dev_id, phy_addr, &phydev);
	SW_RTN_ON_ERROR(rv);

	phydev->eee_enabled = (adv == 0 ? A_FALSE : A_TRUE);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
		phydev->advertising_eee, adv & FAL_PHY_EEE_100BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
		phydev->advertising_eee, adv & FAL_PHY_EEE_1000BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
		phydev->advertising_eee, adv & FAL_PHY_EEE_2500BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
		phydev->advertising_eee, adv & FAL_PHY_EEE_5000BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
		phydev->advertising_eee, adv & FAL_PHY_EEE_10000BASE_T);

	return SW_OK;
}
#endif

sw_error_t
hsl_port_phy_eee_adv_set(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t adv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_eee_adv_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_eee_adv_set (dev_id, phy_addr, adv);
}

sw_error_t
hsl_port_phy_eee_adv_get(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t *adv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_eee_adv_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_eee_adv_get (dev_id, phy_addr, adv);
}

sw_error_t
hsl_port_phy_eee_partner_adv_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t *adv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_eee_partner_adv_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_eee_partner_adv_get (dev_id, phy_addr, adv);
}

sw_error_t
hsl_port_phy_eee_cap_get(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t *cap)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_eee_cap_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_eee_cap_get (dev_id, phy_addr, cap);
}

sw_error_t
hsl_port_phy_eee_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t *status)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_eee_status_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_eee_status_get (dev_id, phy_addr, status);
}
/*qca808x_end*/
#ifndef IN_PORTCONTROL_MINI
/*qca808x_start*/
sw_error_t
hsl_port_phy_local_loopback_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_local_loopback_set)
		return SW_NOT_SUPPORTED;
	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_local_loopback_set (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_local_loopback_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(enable);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_local_loopback_get)
		return SW_NOT_SUPPORTED;
	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_local_loopback_get (dev_id, phy_addr, enable);
}
/*qca808x_end*/
sw_error_t
hsl_port_phy_combo_fiber_mode_set(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_fiber_mode_t mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_combo_fiber_mode_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_combo_fiber_mode_set (dev_id, phy_addr, mode);

}

sw_error_t
hsl_port_phy_combo_fiber_mode_get(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_fiber_mode_t * mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(mode);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_combo_fiber_mode_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	rv = phy_drv->phy_combo_fiber_mode_get (dev_id, phy_addr, mode);

	return rv;

}

sw_error_t
hsl_port_phy_combo_medium_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_medium_t *medium)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(medium);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_combo_medium_status_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_combo_medium_status_get (dev_id, phy_addr, medium);
}

sw_error_t
hsl_port_phy_powersave_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_powersave_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_powersave_set (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_powersave_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(enable);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_powersave_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_powersave_get (dev_id, phy_addr, enable);
}
/*qca808x_start*/
sw_error_t
hsl_port_phy_magic_frame_mac_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_mac_addr_t * mac)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(mac);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_magic_frame_mac_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_magic_frame_mac_set (dev_id, phy_addr, mac);
}

sw_error_t
hsl_port_phy_magic_frame_mac_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_mac_addr_t * mac)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(mac);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_magic_frame_mac_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_magic_frame_mac_get (dev_id, phy_addr, mac);
}

sw_error_t
hsl_port_phy_hibernate_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_hibernation_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_hibernation_set (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_hibernate_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(enable);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_hibernation_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_hibernation_get (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_8023az_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_8023az_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_8023az_set (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_8023az_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(enable);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_8023az_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_8023az_get (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_remote_loopback_set(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_remote_loopback_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_remote_loopback_set (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_remote_loopback_get(a_uint32_t dev_id, fal_port_t port_id,
	a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(enable);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_remote_loopback_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_remote_loopback_get (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_mdix_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_mdix_mode_t mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_mdix_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_mdix_set (dev_id, phy_addr, mode);
}

sw_error_t
hsl_port_phy_mdix_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_mdix_mode_t * mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(mode);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_mdix_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_mdix_get (dev_id, phy_addr, mode);
}

sw_error_t
hsl_port_phy_mdix_status_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_mdix_status_t * mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(mode);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_mdix_status_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_mdix_status_get (dev_id, phy_addr, mode);
}

sw_error_t
hsl_port_phy_phyid_get(a_uint32_t dev_id, fal_port_t port_id, a_uint16_t * org_id,
	a_uint16_t * rev_id)
{
	a_uint32_t phy_id = 0;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(org_id);
	SW_RTN_ON_NULL(rev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	phy_id = hsl_port_phyid_get(dev_id, port_id);

	*org_id = (phy_id >> 16) & 0xffff;
	*rev_id = phy_id & 0xffff;

	return SW_OK;

}

sw_error_t
hsl_port_phy_counter_set(a_uint32_t dev_id, a_uint32_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	if(A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL(phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if(NULL == phy_drv->phy_counter_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR(rv);

	return phy_drv->phy_counter_set(dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_counter_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t *enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if(A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL(phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if(NULL == phy_drv->phy_counter_get)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR(rv);

	return phy_drv->phy_counter_get(dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_counter_show(a_uint32_t dev_id, fal_port_t port_id,
	fal_port_counter_info_t *counter_info)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK (dev_id);

	if(A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL(phy_drv = hsl_phy_api_ops_get(dev_id, port_id));
	if (NULL == phy_drv->phy_counter_show)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid(dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR(rv);

	return phy_drv->phy_counter_show(dev_id, phy_addr, counter_info);
}

sw_error_t
hsl_port_phy_wol_status_set(a_uint32_t dev_id, fal_port_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_wol_status_set)
		return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_wol_status_set (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_wol_status_get(a_uint32_t dev_id, fal_port_t port_id, a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv = NULL;

	HSL_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_NULL(enable);
	if (A_TRUE != hsl_port_prop_check (dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	if (NULL == phy_drv->phy_wol_status_get)
	  return SW_NOT_SUPPORTED;

	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_wol_status_get (dev_id, phy_addr, enable);
}

sw_error_t
hsl_port_phy_mode_get(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_interface_mode_t *mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t phy_addr = 0;
	hsl_phy_ops_t *phy_drv;

	SW_RTN_ON_NULL (phy_drv = hsl_phy_api_ops_get (dev_id, port_id));
	rv = hsl_port_prop_get_phyid (dev_id, port_id, &phy_addr);
	SW_RTN_ON_ERROR (rv);

	return phy_drv->phy_interface_mode_get(dev_id, phy_addr, mode);
}
/*qca808x_end*/
#endif
/*qca808x_start*/
sw_error_t ssdk_phy_driver_cleanup(a_uint32_t dev_id)
{
	a_uint32_t i = 0;

	for (i = 0; i < MAX_PHY_CHIP;i++) {
		if (ssdk_phy_driver[i].port_bmp[dev_id] != 0 &&
				ssdk_phy_driver[i].exit != NULL) {
			ssdk_phy_driver[i].exit(dev_id,
					ssdk_phy_driver[i].port_bmp[dev_id]);
		}
		if(ssdk_phy_driver[i].phy_ops != NULL) {
			kfree(ssdk_phy_driver[i].phy_ops);
			ssdk_phy_driver[i].phy_ops = NULL;
		}
	}

	if(phy_info[dev_id] != NULL)
	{
		kfree(phy_info[dev_id]);
		phy_info[dev_id] = NULL;
	}
	return SW_OK;
}
/*qca808x_end*/

sw_error_t
hsl_port_phy_adv_update(a_uint32_t dev_id, a_uint32_t port_id,
	a_uint32_t adv_mask, a_uint32_t adv)
{
	sw_error_t rv = SW_OK;
	a_uint32_t new_adv = 0;

	rv = hsl_port_phy_autoadv_get(dev_id, port_id, &new_adv);
	SW_RTN_ON_ERROR (rv);
	new_adv &= ~adv_mask;
	new_adv |= adv;
	rv = hsl_port_phy_autoadv_set(dev_id, port_id, new_adv);

	return rv;
}

sw_error_t
hsl_port_phy_txfc_set(a_uint32_t dev_id, a_uint32_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;

	if(enable)
	{
		rv = hsl_port_phy_adv_update(dev_id, port_id, FAL_PHY_ADV_ASY_PAUSE,
			FAL_PHY_ADV_ASY_PAUSE);
		SW_RTN_ON_ERROR(rv);
	}
	else
	{
		rv = hsl_port_phy_adv_update(dev_id, port_id, FAL_PHY_ADV_ASY_PAUSE,
			0);
		SW_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
hsl_port_phy_rxfc_set(a_uint32_t dev_id, a_uint32_t port_id, a_bool_t enable)
{
	sw_error_t rv = SW_OK;

	if(enable)
	{
		rv = hsl_port_phy_adv_update(dev_id, port_id, FAL_PHY_ADV_ASY_PAUSE|
			FAL_PHY_ADV_PAUSE, FAL_PHY_ADV_ASY_PAUSE | FAL_PHY_ADV_PAUSE);
		SW_RTN_ON_ERROR(rv);
	}
	else
	{
		rv = hsl_port_phy_adv_update(dev_id, port_id, FAL_PHY_ADV_ASY_PAUSE |
			FAL_PHY_ADV_PAUSE, 0);
		SW_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t hsl_port_combo_phy_link_status_get(a_uint32_t dev_id,
	a_uint32_t port_id, fal_port_combo_link_status_t * status)
{
	phy_type_t copper_phy_type;
	hsl_phy_ops_t *copper_phy_drv;
#if defined(IN_SFP_PHY)
	a_bool_t sfp_rx_los_status;
#endif

	if (dev_id >= SW_MAX_NR_DEV)
		return SW_BAD_PARAM;

	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}
	if (A_TRUE != hsl_port_phy_combo_capability_get(dev_id, port_id))
	{
		return SW_BAD_PARAM;
	}

	/*get fiber link status*/
#if defined(IN_SFP_PHY)
	SW_RTN_ON_ERROR(sfp_phy_rx_los_status_get(dev_id, port_id, &sfp_rx_los_status));
	status->fiber_link_status = !sfp_rx_los_status;
#endif

	/*get copper link status*/
	if (phy_info[dev_id]->phy_type[port_id] == SFP_PHY_CHIP)
	{
		copper_phy_type = phy_info[dev_id]->combo_phy_type[port_id];
	}
	else
	{
		copper_phy_type = phy_info[dev_id]->phy_type[port_id];
	}
	copper_phy_drv = ssdk_phy_driver[copper_phy_type].phy_ops;
	if (copper_phy_drv && copper_phy_drv->phy_link_status_get)
	{
		status->copper_link_status =
		copper_phy_drv->phy_link_status_get(dev_id, phy_info[dev_id]->phy_address[port_id]);
	}

	return SW_OK;
}

sw_error_t
hsl_port_combo_phy_driver_update(a_uint32_t dev_id,
	a_uint32_t port_id, fal_port_medium_t medium)
{
	phy_type_t phytype = 0;
	struct phy_device *phydev;
	struct device *dev;
	struct net_device *eth_dev = NULL;

	if (dev_id >= SW_MAX_NR_DEV)
	{
		return SW_BAD_PARAM;
	}
	if (A_TRUE != hsl_port_prop_check(dev_id, port_id, HSL_PP_PHY))
	{
		return SW_BAD_PARAM;
	}
	if (A_TRUE != hsl_port_phy_combo_capability_get(dev_id, port_id))
	{
		return SW_BAD_PARAM;
	}

	/*get current phytype*/
	phytype = phy_info[dev_id]->phy_type[port_id];

	if ((phytype == SFP_PHY_CHIP && medium == PHY_MEDIUM_FIBER) ||
		(phytype != SFP_PHY_CHIP && medium == PHY_MEDIUM_COPPER))
	{
		return SW_OK;
	}

	/*update ssdk port phy info*/
	if (medium == PHY_MEDIUM_FIBER)
	{
		hsl_port_feature_set(dev_id, port_id, PHY_F_SFP);
		phy_info[dev_id]->phy_type[port_id] = SFP_PHY_CHIP;
#if defined(IN_SFP_PHY)
		/*register sfp phy driver*/
		sfp_phy_driver_register();
		/*gpio select sfp*/
		sfp_phy_medium_status_set(dev_id, port_id, A_TRUE);
#endif
	}
	else
	{
		hsl_port_feature_clear(dev_id, port_id, PHY_F_SFP);
		phy_info[dev_id]->phy_type[port_id] =
			phy_info[dev_id]->combo_phy_type[port_id];
#if defined(IN_SFP_PHY)
		/*gpio select copper*/
		sfp_phy_medium_status_set(dev_id, port_id, A_FALSE);
#endif
	}
	phy_info[dev_id]->combo_phy_type[port_id] = phytype;
	ssdk_phy_driver[phytype].port_bmp[dev_id] &= ~BIT(port_id);
	ssdk_phy_driver[phy_info[dev_id]->phy_type[port_id]].port_bmp[dev_id] |=
			BIT(port_id);

	/*update phydev info*/
	SW_RTN_ON_ERROR(hsl_port_phydev_get(dev_id, port_id, &phydev));

	mutex_lock(&phydev->lock);
	eth_dev = phydev->attached_dev;
	dev = &phydev->mdio.dev;
	linkmode_zero(phydev->supported);
	linkmode_zero(phydev->advertising);
	linkmode_zero(phydev->lp_advertising);
	phydev->autoneg = AUTONEG_ENABLE;

	if (phy_info[dev_id]->phy_type[port_id] == SFP_PHY_CHIP)
	{
		/*update SFP phyid and c45 info*/
		phydev->phy_id = SFP_PHY;
		phydev->is_c45 = A_FALSE;
		/*update sfp specific phy private data*/
		phydev->priv = ssdk_phy_priv_data_get(dev_id);
	}
	else if (phy_info[dev_id]->phy_type[port_id] == AQUANTIA_PHY_CHIP)
	{
		/*update AQR phyid and c45 info*/
		phydev->phy_id = 0;
		phydev->is_c45 = A_TRUE;
	}
	mutex_unlock(&phydev->lock);

	/* reprobe phy driver, the return value must be checked incase of warning. */
	if (device_reprobe(dev))
		SSDK_ERROR("reprobe failed\n");

	SSDK_DEBUG("combo phy switched to: phy_type %d, phyid 0x%x, is_c45 %d, "
		"phydrv %s, phydev state %d\n", phy_info[dev_id]->phy_type[port_id],
		phydev->phy_id, phydev->is_c45, phydev->drv->name, phydev->state);

	/*start phy and state machine*/
	if (eth_dev->flags & IFF_UP)
	{
		phy_start(phydev);
	}

	return SW_OK;
}

a_uint32_t
hsl_port_force_speed_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	return phy_info[dev_id]->port_force_speed[port_id];
}

void
hsl_port_force_speed_set(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t speed)
{
	hsl_port_feature_set(dev_id, port_id, PHY_F_FORCE);
	phy_info[dev_id]->port_force_speed[port_id] = speed;

	return;
}

a_uint8_t
hsl_port_force_duplex_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	return phy_info[dev_id]->port_force_duplex[port_id];
}

void
hsl_port_force_duplex_set(a_uint32_t dev_id, a_uint32_t port_id, a_uint8_t duplex)
{
	hsl_port_feature_set(dev_id, port_id, PHY_F_FORCE);
	phy_info[dev_id]->port_force_duplex[port_id] = duplex;

	return;
}

/*qca808x_start*/
/*
 * @brief Get feature on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] feature port feature, can be one or more features,
 * will return true when one feature is true
 * @return True or False
 */
a_bool_t
hsl_port_feature_get(a_uint32_t dev_id, a_uint32_t port_id, phy_features_t feature)
{
	if (phy_info[dev_id]->phy_features[port_id] & feature) {
		return A_TRUE;
	}
	return A_FALSE;
}
/*
 * @brief Set feature on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] feature port feature, can be one or more features
 * @return SW_OK or error code
 */
sw_error_t
hsl_port_feature_set(a_uint32_t dev_id, a_uint32_t port_id, phy_features_t feature)
{
	phy_info[dev_id]->phy_features[port_id] |= feature;

	return SW_OK;
}
/*
 * @brief Clear feature on a particular port.
 * @param[in] dev_id device id
 * @param[in] port_id port id
 * @param[in] feature port feature, can be one or more features
 * @return SW_OK or error code
 */
sw_error_t
hsl_port_feature_clear(a_uint32_t dev_id, a_uint32_t port_id, phy_features_t feature)
{
	phy_info[dev_id]->phy_features[port_id] &= ~feature;

	return SW_OK;
}
/*********************APIs to access PHY with MDIO and I2C*********************/
static sw_error_t
hsl_phy_lock(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
#if defined(IN_PHY_I2C_MODE)
	if(IS_I2C_PHY_ADDR(phy_addr))
	{
		struct i2c_adapter *adapt = i2c_get_adapter(I2C_ADAPTER_DEFAULT_ID);
		SW_RTN_ON_NULL(adapt);
		if(enable)
			i2c_lock_bus(adapt, I2C_LOCK_SEGMENT);
		else
			i2c_unlock_bus(adapt, I2C_LOCK_SEGMENT);
	}
	else
#endif
	{
		struct mii_bus *miibus = ssdk_phy_miibus_get(dev_id, phy_addr);
		SW_RTN_ON_NULL(miibus);
		if(enable)
			mutex_lock(&miibus->mdio_lock);
		else
			mutex_unlock(&miibus->mdio_lock);
	}

	return SW_OK;
}
/*
 * @brief read mii register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] mii_reg mii register id
 * @return mii register value
 */
a_uint16_t
__hsl_phy_mii_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mii_reg)
{
	a_uint16_t phy_data = 0;

#if defined(IN_PHY_I2C_MODE)
	if(IS_I2C_PHY_ADDR(phy_addr))
	{
		if(__qca_phy_i2c_read(dev_id, phy_addr, mii_reg, &phy_data))
			return PHY_INVALID_DATA;
	}
	else
#endif
	{
		struct mii_bus *miibus = NULL;

		miibus = ssdk_phy_miibus_get(dev_id, phy_addr);
		if(!miibus)
			return PHY_INVALID_DATA;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0))
		if (mii_reg & SSDK_ADDR_C45)
			phy_data = __mdiobus_c45_read(miibus, phy_addr,
					FIELD_GET(SSDK_DEVADDR_C45_MASK, mii_reg),
					FIELD_GET(SSDK_REGADDR_C45_MASK, mii_reg));
		else
#endif
			phy_data = __mdiobus_read(miibus, phy_addr, mii_reg);
	}

	return phy_data;
}
/*
 * @brief write mii register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] mii_reg mii register id
 * @param[in] reg_val write to mii register
 * @return SW_OK or error code
 */
sw_error_t
__hsl_phy_mii_reg_write(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mii_reg,
	a_uint16_t reg_val)
{
	struct mii_bus *miibus = NULL;
	int ret;

#if defined(IN_PHY_I2C_MODE)
	if(IS_I2C_PHY_ADDR(phy_addr))
	{
		if(__qca_phy_i2c_write(dev_id, phy_addr, mii_reg, reg_val))
			return SW_WRITE_ERROR;
	}
	else
#endif
	{
		miibus = ssdk_phy_miibus_get(dev_id, phy_addr);
		SW_RTN_ON_NULL(miibus);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0))
		if (mii_reg & SSDK_ADDR_C45)
			ret = __mdiobus_c45_write(miibus, phy_addr,
					FIELD_GET(SSDK_DEVADDR_C45_MASK, mii_reg),
					FIELD_GET(SSDK_REGADDR_C45_MASK, mii_reg),
					reg_val);
		else
#endif
			ret = __mdiobus_write(miibus, phy_addr, mii_reg, reg_val);

		if (ret)
			return ret;
	}

	return SW_OK;
}
/*
 * @brief modify mii register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] mii_reg mii register id
 * @param[in] mask mask of bits to clear
 * @param[in] value new value of bits
 * @return SW_OK or error code
 */
sw_error_t
__hsl_phy_modify_mii(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mii_reg,
	a_uint16_t mask, a_uint16_t value)
{
	a_uint16_t phy_data = 0, new_phy_data = 0;

	phy_data = __hsl_phy_mii_reg_read(dev_id, phy_addr, mii_reg);
	PHY_RTN_ON_READ_ERROR(phy_data);
	new_phy_data = (phy_data & ~mask) | value;
	return __hsl_phy_mii_reg_write(dev_id, phy_addr, mii_reg, new_phy_data);
}
/*
 * @brief read mii register with lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] mii_reg mii register id
 * @return mii register value
 */
a_uint16_t
hsl_phy_mii_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mii_reg)
{
	a_uint16_t phy_data = 0;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	phy_data = __hsl_phy_mii_reg_read(dev_id, phy_addr, mii_reg);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return phy_data;
}
/*
 * @brief write mii register with lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] mii_reg mii register id
 * @param[in] reg_val write to mii register
 * @return SW_OK or error code
 */
sw_error_t
hsl_phy_mii_reg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t mii_reg, a_uint16_t reg_val)
{
	sw_error_t rv = SW_OK;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	rv = __hsl_phy_mii_reg_write(dev_id, phy_addr, mii_reg, reg_val);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return rv;
}
/*
 * @brief modify mii register with lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] mii_reg mii register id
 * @param[in] mask mask of bits to clear
 * @param[in] value new value of bits
 * @return SW_OK or error code
 */
sw_error_t
hsl_phy_modify_mii(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mii_reg,
	a_uint16_t mask, a_uint16_t value)
{
	sw_error_t rv = SW_OK;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	rv = __hsl_phy_modify_mii(dev_id, phy_addr, mii_reg, mask, value);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return rv;
}

static a_uint16_t
__hsl_phy_c45_mmd_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t mmd_num, a_uint32_t mmd_reg)
{
	return __hsl_phy_mii_reg_read(dev_id, phy_addr,
		HSL_PHY_REG_C45_ADDR(mmd_num, mmd_reg));
}

static a_uint16_t
__hsl_phy_c22_mmd_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t mmd_num, a_uint32_t mmd_reg)
{
	sw_error_t rv = SW_OK;

	rv = __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_MMD_CTRL_REG, mmd_num);
	rv |= __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_MMD_DATA_REG, mmd_reg);
	rv |= __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_MMD_CTRL_REG,
		0x4000 | mmd_num);
	if(rv != SW_OK)
		return PHY_INVALID_DATA;
	return __hsl_phy_mii_reg_read(dev_id, phy_addr, HSL_PHY_MMD_DATA_REG);
}

static sw_error_t
__hsl_phy_c45_mmd_reg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint16_t mmd_num, a_uint16_t mmd_reg, a_uint16_t reg_val)
{
	return __hsl_phy_mii_reg_write(dev_id, phy_addr,
		HSL_PHY_REG_C45_ADDR(mmd_num, mmd_reg), reg_val);
}

static a_uint16_t
__hsl_phy_c22_mmd_reg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t mmd_num, a_uint32_t mmd_reg, a_uint16_t reg_val)
{
	sw_error_t rv = SW_OK;

	rv = __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_MMD_CTRL_REG, mmd_num);
	rv |= __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_MMD_DATA_REG, mmd_reg);
	rv |= __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_MMD_CTRL_REG,
		0x4000 | mmd_num);
	rv |= __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_MMD_DATA_REG, reg_val);

	return rv;
}
/*
 * @brief read mmd register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] is_c45 is c45 access or not
 * @param[in] mmd_num mmd number
 * @param[in] mmd_reg mmd register id
 * @return mmd register value
 */
a_uint16_t
__hsl_phy_mmd_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t is_c45,
	a_uint32_t mmd_num, a_uint32_t mmd_reg)
{
	if(is_c45)
		return __hsl_phy_c45_mmd_reg_read(dev_id, phy_addr, mmd_num, mmd_reg);
	else
		return __hsl_phy_c22_mmd_reg_read(dev_id, phy_addr, mmd_num, mmd_reg);
}
/*
 * @brief write mmd register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] is_c45 is c45 access or not
 * @param[in] mmd_num mmd number
 * @param[in] mmd_reg mmd register id
 * @param[in] reg_val write to mmd register
 * @return SW_OK or error code
 */
sw_error_t
__hsl_phy_mmd_reg_write(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t is_c45,
	a_uint32_t mmd_num, a_uint32_t mmd_reg, a_uint16_t reg_val)
{
	if(is_c45)
		return __hsl_phy_c45_mmd_reg_write(dev_id, phy_addr, mmd_num, mmd_reg,
			reg_val);
	else
		return __hsl_phy_c22_mmd_reg_write(dev_id, phy_addr, mmd_num, mmd_reg,
			reg_val);
}
/*
 * @brief modify mmd register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] is_c45 is c45 access or not
 * @param[in] mmd_num mmd number
 * @param[in] mmd_reg mmd register id
 * @param[in] mask mask of bits to clear
 * @param[in] value new value of bits
 * @return SW_OK or error code
 */
sw_error_t
__hsl_phy_modify_mmd(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t is_c45,
	a_uint32_t mmd_num, a_uint32_t mmd_reg, a_uint16_t mask, a_uint16_t value)
{
	a_uint16_t phy_data = 0, new_phy_data = 0;

	phy_data = __hsl_phy_mmd_reg_read(dev_id, phy_addr, is_c45, mmd_num, mmd_reg);
	PHY_RTN_ON_READ_ERROR(phy_data);
	new_phy_data = (phy_data & ~mask) | value;
	return __hsl_phy_mmd_reg_write(dev_id, phy_addr, is_c45, mmd_num, mmd_reg,
		new_phy_data);
}
/*
 * @brief read mmd register with lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] is_c45 is c45 access or not
 * @param[in] mmd_num mmd number
 * @param[in] mmd_reg mmd register id
 * @return mmd register value
 */
a_uint16_t
hsl_phy_mmd_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t is_c45,
	a_uint32_t mmd_num, a_uint32_t mmd_reg)
{
	a_uint16_t phy_data = 0;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	phy_data = __hsl_phy_mmd_reg_read(dev_id, phy_addr, is_c45, mmd_num,
		mmd_reg);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return phy_data;
}
/*
 * @brief write mmd register with lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] is_c45 is c45 access or not
 * @param[in] mmd_num mmd number
 * @param[in] mmd_reg mmd register id
 * @param[in] reg_val write to mmd register
 * @return SW_OK or error code
 */
sw_error_t
hsl_phy_mmd_reg_write(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t is_c45,
	a_uint32_t mmd_num, a_uint32_t mmd_reg, a_uint16_t reg_val)
{
	sw_error_t rv = SW_OK;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	rv = __hsl_phy_mmd_reg_write(dev_id, phy_addr, is_c45, mmd_num, mmd_reg,
		reg_val);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return rv;
}
/*
 * @brief modify mmd register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] is_c45 is c45 access or not
 * @param[in] mmd_num mmd number
 * @param[in] mmd_reg mmd register id
 * @param[in] mask mask of bits to clear
 * @param[in] value new value of bits
 * @return SW_OK or error code
 */
sw_error_t
hsl_phy_modify_mmd(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t is_c45,
	a_uint32_t mmd_num, a_uint32_t mmd_reg, a_uint16_t mask, a_uint16_t value)
{
	sw_error_t rv = SW_OK;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	rv = __hsl_phy_modify_mmd(dev_id, phy_addr, is_c45, mmd_num, mmd_reg,
		mask, value);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return rv;
}
/*
 * @brief read debug register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] debug_reg debug register id
 * @return debug register value
 */
a_uint16_t
__hsl_phy_debug_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t debug_reg)
{
	sw_error_t rv = SW_OK;

	rv = __hsl_phy_mii_reg_write(dev_id, phy_addr, HSL_PHY_DEBUG_PORT_ADDRESS,
		debug_reg);
	if(rv != SW_OK)
		return PHY_INVALID_DATA;
	return __hsl_phy_mii_reg_read(dev_id, phy_addr, HSL_PHY_DEBUG_PORT_DATA);
}
/*
 * @brief write debug register without lock.
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] debug_reg debug register id
 * @param[in] reg_val write to debug register
 * @return SW_OK or error code
 */
sw_error_t
__hsl_phy_debug_reg_write(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t debug_reg, a_uint16_t reg_val)
{
	sw_error_t rv = SW_OK;

	rv = __hsl_phy_mii_reg_write(dev_id, phy_id, HSL_PHY_DEBUG_PORT_ADDRESS,
		debug_reg);

	rv |= __hsl_phy_mii_reg_write(dev_id, phy_id, HSL_PHY_DEBUG_PORT_DATA,
		reg_val);

	return rv;
}
/*
 * @brief modify debug register without lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] debug_reg debug register id
 * @param[in] mask mask of bits to clear
 * @param[in] value new value of bits
 * @return SW_OK or error code
 */
sw_error_t
__hsl_phy_modify_debug(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t debug_reg, a_uint16_t mask, a_uint16_t value)
{
	a_uint16_t phy_data = 0, new_phy_data = 0;

	phy_data = __hsl_phy_debug_reg_read(dev_id, phy_addr, debug_reg);
	PHY_RTN_ON_READ_ERROR(phy_data);
	new_phy_data = (phy_data & ~mask) | value;
	return __hsl_phy_debug_reg_write (dev_id, phy_addr, debug_reg,
		new_phy_data);
}
/*
 * @brief read debug register with lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] debug_reg debug register id
 * @return debug register value
 */
a_uint16_t
hsl_phy_debug_reg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t debug_reg)
{
	a_uint16_t phy_data = 0;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	phy_data = __hsl_phy_debug_reg_read(dev_id, phy_addr, debug_reg);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return phy_data;
}
/*
 * @brief write debug register with lock.
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] debug_reg debug register id
 * @param[in] reg_val write to debug register
 * @return SW_OK or error code
 */
sw_error_t
hsl_phy_debug_reg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t debug_reg, a_uint16_t reg_val)
{
	sw_error_t rv = SW_OK;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	rv = __hsl_phy_debug_reg_write(dev_id, phy_addr, debug_reg, reg_val);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return rv;
}
/*
 * @brief modify debug register with lock
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] debug_reg debug register id
 * @param[in] mask mask of bits to clear
 * @param[in] value new value of bits
 * @return SW_OK or error code
 */
sw_error_t
hsl_phy_modify_debug(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t debug_reg, a_uint16_t mask, a_uint16_t value)
{
	sw_error_t rv = SW_OK;

	hsl_phy_lock(dev_id, phy_addr, A_TRUE);
	rv = __hsl_phy_modify_debug(dev_id, phy_addr, debug_reg, mask, value);
	hsl_phy_lock(dev_id, phy_addr, A_FALSE);

	return rv;
}
/*qca808x_end*/
