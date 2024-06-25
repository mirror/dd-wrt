/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
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

/*qca808x_start*/
#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#include "fal.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
/*qca808x_end*/
#include "ssdk_dts.h"
#if (defined(HPPE) || defined(MP))
#include "hppe_init.h"
#endif
#include <linux/kconfig.h>
/*qca808x_start*/
#include <linux/version.h>
/*qca808x_end*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
//#include <asm/mach-types.h>
#include <generated/autoconf.h>
#include <linux/if_arp.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/string.h>

#if defined(ISIS) ||defined(ISISC) ||defined(GARUDA)
#include <f1_phy.h>
#endif
#if defined(ATHENA) ||defined(SHIVA) ||defined(HORUS)
#include <f2_phy.h>
#endif
#ifdef IN_MALIBU_PHY
#include <malibu_phy.h>
#endif
/*qca808x_start*/
#include "ssdk_plat.h"
#include "hsl_phy.h"
/*qca808x_end*/
#include "ssdk_clk.h"
#include "ref_vlan.h"
#include "ref_fdb.h"
#include "ref_mib.h"
#include "ref_port_ctrl.h"
#include "ref_misc.h"
#include "ref_uci.h"
#include "shell.h"
#ifdef IN_IP
#if defined (CONFIG_NF_FLOW_COOKIE)
#include "fal_flowcookie.h"
#ifdef IN_SFE
#include <shortcut-fe/sfe.h>
#endif
#endif
#endif

#ifdef IN_RFS
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#include <linux/if_vlan.h>
#endif
#include <qca-rfs/rfs_dev.h>
#ifdef IN_IP
#include "fal_rfs.h"
#endif
#endif

#if defined(MHT)
#include "ssdk_mht_clk.h"
#endif

#include "adpt.h"

#ifdef IN_LINUX_STD_PTP
#include "hsl_ptp.h"
#include "qca808x.h"
#endif
#include "hsl_port_prop.h"
/*qca808x_start*/

extern struct qca_phy_priv **qca_phy_priv_global;
/*qca808x_end*/

#ifdef BOARD_IPQ806X
#define PLATFORM_MDIO_BUS_NAME		"mdio-gpio"
#endif
/*qca808x_start*/
#define MDIO_BUS_0					0
#define MDIO_BUS_1					1
/*qca808x_end*/
#define PLATFORM_MDIO_BUS_NUM		MDIO_BUS_0

#define ISIS_CHIP_ID 0x18
#define ISIS_CHIP_REG 0
#define SHIVA_CHIP_ID 0x1f
#define SHIVA_CHIP_REG 0x10
#define HIGH_ADDR_DFLT	0x200

static int ssdk_dev_id = 0;
/*qca808x_start*/
a_uint32_t ssdk_log_level = SSDK_LOG_LEVEL_DEFAULT;
/*qca808x_end*/

sw_error_t qca_mii_bus_lock(a_uint32_t dev_id, a_bool_t enable)
{
	struct mii_bus *miibus = ssdk_miibus_get(dev_id, SSDK_MII_DEFAULT_BUS_ID);

	SW_RTN_ON_NULL(miibus);
	if(enable)
		mutex_lock(&miibus->mdio_lock);
	else
		mutex_unlock(&miibus->mdio_lock);

	return SW_OK;
}

sw_error_t
__qca_mii_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint8_t value[], a_uint32_t value_len)
{
	a_uint32_t reg_val = 0;

	if (value_len != sizeof (a_uint32_t))
		return SW_BAD_LEN;

	reg_val = __qca_mii_read(dev_id, reg_addr);

	aos_mem_copy(value, &reg_val, sizeof (a_uint32_t));

	return SW_OK;
}

sw_error_t
__qca_mii_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
                   a_uint32_t value_len)
{
	a_uint32_t reg_val = 0;

	if (value_len != sizeof (a_uint32_t))
		return SW_BAD_LEN;

	aos_mem_copy(&reg_val, value, sizeof (a_uint32_t));

	__qca_mii_write(dev_id, reg_addr, reg_val);

	return SW_OK;
}

sw_error_t
__qca_mii_field_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                    a_uint32_t bit_offset, a_uint32_t field_len,
                    a_uint8_t value[], a_uint32_t value_len)
{
	a_uint32_t reg_val = 0;

	if ((bit_offset >= 32 || (field_len > 32)) || (field_len == 0))
		return SW_OUT_OF_RANGE;

	if (value_len != sizeof (a_uint32_t))
		return SW_BAD_LEN;

	reg_val = __qca_mii_read(dev_id, reg_addr);

	if(32 == field_len) {
		*((a_uint32_t *) value) = reg_val;
	} else  {
		*((a_uint32_t *) value) = SW_REG_2_FIELD(reg_val, bit_offset, field_len);
	}

	return SW_OK;
}

sw_error_t
__qca_mii_field_set(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint32_t bit_offset, a_uint32_t field_len,
                   const a_uint8_t value[], a_uint32_t value_len)
{
	a_uint32_t reg_val = 0;
	a_uint32_t field_val = *((a_uint32_t *) value);

	if ((bit_offset >= 32 || (field_len > 32)) || (field_len == 0))
		return SW_OUT_OF_RANGE;

	if (value_len != sizeof (a_uint32_t))
		return SW_BAD_LEN;

	reg_val = __qca_mii_read(dev_id, reg_addr);

	if(32 == field_len) {
		reg_val = field_val;
	} else {
		SW_REG_SET_BY_FIELD_U32(reg_val, field_val, bit_offset, field_len);
	}

	__qca_mii_write(dev_id, reg_addr, reg_val);

	return SW_OK;
}

sw_error_t
qca_mii_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint8_t value[], a_uint32_t value_len)
{
	sw_error_t rv = SW_OK;

	qca_mii_bus_lock(dev_id, A_TRUE);
	rv = __qca_mii_reg_get(dev_id, reg_addr, value, value_len);
	qca_mii_bus_lock(dev_id, A_FALSE);

	return rv;
}

sw_error_t
qca_mii_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
                   a_uint32_t value_len)
{
	sw_error_t rv = SW_OK;

	qca_mii_bus_lock(dev_id, A_TRUE);
	rv = __qca_mii_reg_set(dev_id, reg_addr, value, value_len);
	qca_mii_bus_lock(dev_id, A_FALSE);

	return rv;
}

sw_error_t
qca_mii_field_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                    a_uint32_t bit_offset, a_uint32_t field_len,
                    a_uint8_t value[], a_uint32_t value_len)
{
	sw_error_t rv = SW_OK;

	qca_mii_bus_lock(dev_id, A_TRUE);
	rv = __qca_mii_field_get(dev_id, reg_addr, bit_offset, field_len, value, value_len);
	qca_mii_bus_lock(dev_id, A_FALSE);

	return rv;
}

sw_error_t
qca_mii_field_set(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint32_t bit_offset, a_uint32_t field_len,
                   const a_uint8_t value[], a_uint32_t value_len)
{
	sw_error_t rv = SW_OK;

	qca_mii_bus_lock(dev_id, A_TRUE);
	rv = __qca_mii_field_set(dev_id, reg_addr, bit_offset, field_len, value, value_len);
	qca_mii_bus_lock(dev_id, A_FALSE);

	return rv;
}

static void qca_mii_reg_convert(a_uint32_t dev_id, a_uint32_t *reg)
{
	ssdk_chip_type chip_type = hsl_get_current_chip_type(dev_id);

	switch (chip_type) {
		case CHIP_ISISC:
			*reg |= SSDK_SWITCH_REG_TYPE_QCA8337;
			break;
		default:
			*reg |= SSDK_SWITCH_REG_TYPE_QCA8386;
			break;
	}
}

sw_error_t qca_mii_raw_read(struct mii_bus *bus, a_uint32_t reg, a_uint32_t *val)
{
	struct qca_mdio_data *mdio_priv = bus->priv;

	if (mdio_priv && mdio_priv->sw_read) {
		*val = mdio_priv->sw_read(bus, reg);
		return SW_OK;
	}

	return SW_FAIL;
}

sw_error_t qca_mii_raw_write(struct mii_bus *bus, a_uint32_t reg, a_uint32_t val)
{
	struct qca_mdio_data *mdio_priv = bus->priv;

	if (mdio_priv && mdio_priv->sw_write) {
		mdio_priv->sw_write(bus, reg, val);
		return SW_OK;
	}

	return SW_FAIL;
}

sw_error_t qca_mii_raw_update(struct mii_bus *bus, a_uint32_t reg,
		a_uint32_t clear, a_uint32_t set)
{
	struct qca_mdio_data *mdio_priv = bus->priv;

	if (mdio_priv && mdio_priv->sw_read && mdio_priv->sw_write) {
		a_uint32_t val;

		val = mdio_priv->sw_read(bus, reg);
		val &= ~clear;
		val |= set;
		mdio_priv->sw_write(bus, reg, val);

		return SW_OK;
	}

	return SW_FAIL;
}

a_uint32_t __qca_mii_read(a_uint32_t dev_id, a_uint32_t reg)
{
	a_uint32_t val = 0xffffffff;
	struct mii_bus *bus = NULL;

	bus = ssdk_miibus_get(dev_id, SSDK_MII_DEFAULT_BUS_ID);
	if (!bus)
		return val;

	qca_mii_reg_convert(dev_id, &reg);
	qca_mii_raw_read(bus, reg, &val);
	return val;
}

void __qca_mii_write(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t val)
{
	struct mii_bus *bus = NULL;

	bus = ssdk_miibus_get(dev_id, SSDK_MII_DEFAULT_BUS_ID);
	if (!bus)
		return;

	qca_mii_reg_convert(dev_id, &reg);
	qca_mii_raw_write(bus, reg, val);
}

int __qca_mii_update(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t mask, a_uint32_t val)
{
	struct mii_bus *bus = NULL;

	bus = ssdk_miibus_get(dev_id, SSDK_MII_DEFAULT_BUS_ID);
	if (!bus)
		return -1;

	qca_mii_reg_convert(dev_id, &reg);
	qca_mii_raw_update(bus, reg, mask, val);
	return 0;
}

a_uint32_t qca_mii_read(a_uint32_t dev_id, a_uint32_t reg)
{
	a_uint32_t val = 0xffffffff;
	struct mii_bus *bus = NULL;

	bus = ssdk_miibus_get(dev_id, SSDK_MII_DEFAULT_BUS_ID);
	if (!bus)
		return val;

	mutex_lock(&bus->mdio_lock);
	qca_mii_reg_convert(dev_id, &reg);
	qca_mii_raw_read(bus, reg, &val);
	mutex_unlock(&bus->mdio_lock);

	return val;
}

void qca_mii_write(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t val)
{
	struct mii_bus *bus = NULL;

	bus = ssdk_miibus_get(dev_id, SSDK_MII_DEFAULT_BUS_ID);
	if (!bus)
		return;

	mutex_lock(&bus->mdio_lock);
	qca_mii_reg_convert(dev_id, &reg);
	qca_mii_raw_write(bus, reg, val);
	mutex_unlock(&bus->mdio_lock);
}

int qca_mii_update(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t mask, a_uint32_t val)
{
	struct mii_bus *bus = NULL;

	bus = ssdk_miibus_get(dev_id, SSDK_MII_DEFAULT_BUS_ID);
	if (!bus)
		return -1;

	mutex_lock(&bus->mdio_lock);
	qca_mii_reg_convert(dev_id, &reg);
	qca_mii_raw_update(bus, reg, mask, val);
	mutex_unlock(&bus->mdio_lock);

	return 0;
}

#if defined(SSDK_PCIE_BUS)
extern u32 ppe_mem_read(u32 reg);
extern void ppe_mem_write(u32 reg, u32 val);
#endif
sw_error_t
qca_switch_reg_read(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
	uint32_t reg_val = 0;

	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if ((reg_addr%4)!= 0)
	return SW_BAD_PARAM;

#if defined(SSDK_PCIE_BUS)
	if (HSL_REG_PCIE_BUS == ssdk_switch_reg_access_mode_get(dev_id)) {
		uint32_t pcie_base = ssdk_switch_pcie_base_get(dev_id);
		reg_val = ppe_mem_read(pcie_base + reg_addr);
	} else
#endif
		reg_val = readl(qca_phy_priv_global[dev_id]->hw_addr + reg_addr);

	aos_mem_copy(reg_data, &reg_val, sizeof (a_uint32_t));
	return 0;
}

sw_error_t
qca_switch_reg_write(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
	uint32_t reg_val = 0;
	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if ((reg_addr%4)!= 0)
	return SW_BAD_PARAM;

	aos_mem_copy(&reg_val, reg_data, sizeof (a_uint32_t));

#if defined(SSDK_PCIE_BUS)
	if (HSL_REG_PCIE_BUS == ssdk_switch_reg_access_mode_get(dev_id)) {
		uint32_t pcie_base = ssdk_switch_pcie_base_get(dev_id);
		ppe_mem_write(pcie_base + reg_addr, reg_val);
	} else
#endif
		writel(reg_val, qca_phy_priv_global[dev_id]->hw_addr + reg_addr);
	return 0;
}

sw_error_t
qca_psgmii_reg_read(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
#ifdef DESS
	uint32_t reg_val = 0;

	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if((reg_addr%4)!=0)
	return SW_BAD_PARAM;

	if (qca_phy_priv_global[dev_id]->psgmii_hw_addr == NULL)
		return SW_NOT_SUPPORTED;

	reg_val = readl(qca_phy_priv_global[dev_id]->psgmii_hw_addr + reg_addr);

	aos_mem_copy(reg_data, &reg_val, sizeof (a_uint32_t));
#endif
	return 0;
}

sw_error_t
qca_psgmii_reg_write(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
#ifdef DESS
	uint32_t reg_val = 0;
	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if((reg_addr%4)!=0)
	return SW_BAD_PARAM;

	if (qca_phy_priv_global[dev_id]->psgmii_hw_addr == NULL)
		return SW_NOT_SUPPORTED;

	aos_mem_copy(&reg_val, reg_data, sizeof (a_uint32_t));
	writel(reg_val, qca_phy_priv_global[dev_id]->psgmii_hw_addr + reg_addr);
#endif
	return 0;
}

sw_error_t
qca_uniphy_reg_read(a_uint32_t dev_id, a_uint32_t uniphy_index,
				a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
#if (defined(HPPE) || defined(MP))
	uint32_t reg_val = 0;
	void __iomem *hppe_uniphy_base = NULL;
	a_uint32_t reg_addr1, reg_addr2;

	if(ssdk_is_emulation(dev_id)){
		return SW_OK;
	}

	SSDK_DEBUG("qca_uniphy_reg_read function reg:0x%x\n and value:0x%x", reg_addr, *reg_data);
	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if (SSDK_UNIPHY_INSTANCE0 == uniphy_index)
		hppe_uniphy_base = qca_phy_priv_global[dev_id]->uniphy_hw_addr;
	else if (SSDK_UNIPHY_INSTANCE1 == uniphy_index)
		hppe_uniphy_base = qca_phy_priv_global[dev_id]->uniphy_hw_addr + HPPE_UNIPHY_BASE1;

	else if (SSDK_UNIPHY_INSTANCE2 == uniphy_index)
		hppe_uniphy_base = qca_phy_priv_global[dev_id]->uniphy_hw_addr + HPPE_UNIPHY_BASE2;
	else
		return SW_BAD_PARAM;

	if ( reg_addr > HPPE_UNIPHY_MAX_DIRECT_ACCESS_REG)
	{
		// uniphy reg indireclty access
		reg_addr1 = (reg_addr & 0xffffff) >> 8;
		writel(reg_addr1, hppe_uniphy_base + HPPE_UNIPHY_INDIRECT_REG_ADDR);

		reg_addr2 = reg_addr & HPPE_UNIPHY_INDIRECT_LOW_ADDR;
		reg_addr = (HPPE_UNIPHY_INDIRECT_DATA << 10) | (reg_addr2 << 2);

		reg_val = readl(hppe_uniphy_base + reg_addr);
		aos_mem_copy(reg_data, &reg_val, sizeof (a_uint32_t));
	}
	else
	{	// uniphy reg directly access
		reg_val = readl(hppe_uniphy_base + reg_addr);
		aos_mem_copy(reg_data, &reg_val, sizeof (a_uint32_t));
	}
#endif
	return 0;
}

sw_error_t
qca_uniphy_reg_write(a_uint32_t dev_id, a_uint32_t uniphy_index,
				a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
#if (defined(HPPE) || defined(MP))
	void __iomem *hppe_uniphy_base = NULL;
	a_uint32_t reg_addr1, reg_addr2;
	uint32_t reg_val = 0;

	if(ssdk_is_emulation(dev_id)){
		return SW_OK;
	}

	SSDK_DEBUG("qca_uniphy_reg_write function reg:0x%x\n and value:0x%x", reg_addr, *reg_data);
	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if (SSDK_UNIPHY_INSTANCE0 == uniphy_index)
		hppe_uniphy_base = qca_phy_priv_global[dev_id]->uniphy_hw_addr;
	else if (SSDK_UNIPHY_INSTANCE1 == uniphy_index)
		hppe_uniphy_base = qca_phy_priv_global[dev_id]->uniphy_hw_addr + HPPE_UNIPHY_BASE1;

	else if (SSDK_UNIPHY_INSTANCE2 == uniphy_index)
		hppe_uniphy_base = qca_phy_priv_global[dev_id]->uniphy_hw_addr + HPPE_UNIPHY_BASE2;
	else
		return SW_BAD_PARAM;

	if ( reg_addr > HPPE_UNIPHY_MAX_DIRECT_ACCESS_REG)
	{
		// uniphy reg indireclty access
		reg_addr1 = (reg_addr & 0xffffff) >> 8;
		writel(reg_addr1, hppe_uniphy_base + HPPE_UNIPHY_INDIRECT_REG_ADDR);

		reg_addr2 = reg_addr & HPPE_UNIPHY_INDIRECT_LOW_ADDR;
		reg_addr = (HPPE_UNIPHY_INDIRECT_DATA << 10) | (reg_addr2 << 2);
		aos_mem_copy(&reg_val, reg_data, sizeof (a_uint32_t));
		writel(reg_val, hppe_uniphy_base + reg_addr);
	}
	else
	{	// uniphy reg directly access
		aos_mem_copy(&reg_val, reg_data, sizeof (a_uint32_t));
		writel(reg_val, hppe_uniphy_base + reg_addr);
	}
#endif
	return 0;
}
/*qca808x_start*/
struct mii_bus *ssdk_miibus_get(a_uint32_t dev_id, a_uint32_t index)
{
	return qca_phy_priv_global[dev_id]->miibus[index];
}

struct mii_bus *ssdk_phy_miibus_get(a_uint32_t dev_id, a_uint32_t phy_addr_e)
{
	a_uint32_t index = 0;

	index = TO_MIIBUS_INDEX(phy_addr_e);
	if(index >= SSDK_MII_INVALID_BUS_ID)
		return NULL;

	return ssdk_miibus_get(dev_id, index);
}

struct mii_bus *ssdk_port_miibus_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	a_uint32_t phy_addr_e = 0;

	phy_addr_e = qca_ssdk_port_to_phy_addr(dev_id, port_id);

	return ssdk_phy_miibus_get(dev_id, phy_addr_e);
}

sw_error_t ssdk_miibus_add(a_uint32_t dev_id, struct mii_bus *miibus,
	a_uint32_t *index)
{
	a_uint32_t i = 0;

	if(!miibus)
		return SW_BAD_PTR;

	for(i = 0; i < SSDK_MII_BUS_MAX; i++)
	{
		if(qca_phy_priv_global[dev_id]->miibus[i] == miibus ||
			qca_phy_priv_global[dev_id]->miibus[i] == NULL)
		{
			*index = i;
			qca_phy_priv_global[dev_id]->miibus[i] = miibus;
			return SW_OK;
		}
	}

	return SW_OUT_OF_RANGE;
}

a_uint32_t ssdk_miibus_index_get(a_uint32_t dev_id, struct mii_bus *miibus)
{
	a_uint32_t index = 0;

	for(index = 0; index < SSDK_MII_BUS_MAX; index++)
	{
		if(qca_phy_priv_global[dev_id]->miibus[index] == miibus)
			return index;
	}

	return SSDK_MII_INVALID_BUS_ID;
}
/*qca808x_end*/
sw_error_t ssdk_miibus_freq_get(a_uint32_t dev_id, a_uint32_t index,
	a_uint32_t *freq)
{
	struct mii_bus *bus = NULL;
	struct qca_mdio_data *mdio_priv = NULL;

	bus = ssdk_miibus_get(dev_id, index);
	if (!bus) {
		SSDK_ERROR("Can't get MDIO bus of device id %d\n", dev_id);
		return SW_BAD_PTR;
	}

	mdio_priv = bus->priv;
	if (!mdio_priv) {
		SSDK_ERROR("MDIO bus private data is NULL\n");
		return SW_BAD_PTR;
	}

	*freq = mdio_priv->clk_div;
	return SW_OK;
}

sw_error_t ssdk_miibus_freq_set(a_uint32_t dev_id, a_uint32_t index,
	a_uint32_t freq)
{
	struct mii_bus *bus = NULL;
	struct qca_mdio_data *mdio_priv = NULL;

	bus = ssdk_miibus_get(dev_id, index);
	if (!bus) {
		SSDK_ERROR("Can't get MDIO bus of device id %d\n", dev_id);
		return SW_BAD_PTR;
	}

	mdio_priv = bus->priv;
	if (!mdio_priv) {
		SSDK_ERROR("MDIO bus private data is NULL\n");
		return SW_BAD_PTR;
	}

	mdio_priv->clk_div = freq;
	return SW_OK;
}

static ssize_t ssdk_dev_id_get(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count;
	a_uint32_t num;

	num = (a_uint32_t)ssdk_dev_id;

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%u", num);
	return count;
}

static ssize_t ssdk_dev_id_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char num_buf[12];
	a_uint32_t num;

	if (count >= sizeof(num_buf)) return 0;
	memcpy(num_buf, buf, count);
	num_buf[count] = '\0';
	sscanf(num_buf, "%u", &num);

	ssdk_dev_id = num;

	return count;
}

static ssize_t ssdk_log_level_get(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count;
	a_uint32_t num;

	num = ssdk_log_level;

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%u", num);
	return count;
}

static ssize_t ssdk_log_level_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char num_buf[12];
	a_uint32_t num;

	if (count >= sizeof(num_buf))
		return 0;
	memcpy(num_buf, buf, count);
	num_buf[count] = '\0';
	sscanf(num_buf, "%u", &num);

	ssdk_log_level = (a_uint32_t)num;

	return count;
}

static ssize_t ssdk_packet_counter_get(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count;
	adpt_api_t *p_api;

	p_api = adpt_api_ptr_get(ssdk_dev_id);
	if (p_api == NULL || p_api->adpt_debug_counter_get == NULL)
	{
		count = snprintf(buf, (ssize_t)PAGE_SIZE, "Unsupported\n");
		return count;
	}

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "\n");

	p_api->adpt_debug_counter_get(ssdk_dev_id, A_FALSE);

	return count;
}

static ssize_t ssdk_packet_counter_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char num_buf[12];
	adpt_api_t *p_api;

	p_api = adpt_api_ptr_get(ssdk_dev_id);
	if (p_api == NULL || p_api->adpt_debug_counter_set == NULL) {
		SSDK_WARN("Unsupported\n");
		return count;
	}

	p_api->adpt_debug_counter_set(ssdk_dev_id);

	if (count >= sizeof(num_buf))
		return 0;
	memcpy(num_buf, buf, count);
	num_buf[count] = '\0';


	return count;
}

static ssize_t ssdk_byte_counter_get(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count;
	adpt_api_t *p_api;

	p_api = adpt_api_ptr_get(ssdk_dev_id);
	if (p_api == NULL || p_api->adpt_debug_counter_get == NULL)
	{
		count = snprintf(buf, (ssize_t)PAGE_SIZE, "Unsupported\n");
		return count;
	}

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "\n");

	p_api->adpt_debug_counter_get(ssdk_dev_id, A_TRUE);

	return count;
}

static ssize_t ssdk_byte_counter_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char num_buf[12];
	adpt_api_t *p_api;

	p_api = adpt_api_ptr_get(ssdk_dev_id);
	if (p_api == NULL || p_api->adpt_debug_counter_set == NULL) {
		SSDK_WARN("Unsupported\n");
		return count;
	}

	p_api->adpt_debug_counter_set(ssdk_dev_id);

	if (count >= sizeof(num_buf))
		return 0;
	memcpy(num_buf, buf, count);
	num_buf[count] = '\0';


	return count;
}

#ifdef HPPE
#ifdef IN_QOS
void ssdk_dts_port_scheduler_dump(a_uint32_t dev_id)
{
	a_uint32_t i;
	ssdk_dt_portscheduler_cfg *portscheduler_cfg;
	ssdk_dt_scheduler_cfg *scheduler_cfg;
	a_uint8_t srcmsg[7][16];

	scheduler_cfg = ssdk_bootup_shceduler_cfg_get(dev_id);

	printk("===============================port_scheduler_resource===========================\n");
	printk("portid     ucastq     mcastq     10sp     10cdrr     10edrr     11cdrr     11edrr\n");
	for (i = 0; i < SSDK_MAX_PORT_NUM; i++)
	{
		portscheduler_cfg = &scheduler_cfg->pool[i];
		snprintf(srcmsg[0], sizeof(srcmsg[0]), "<%d %d>", portscheduler_cfg->ucastq_start,
				portscheduler_cfg->ucastq_end);
		snprintf(srcmsg[1], sizeof(srcmsg[1]), "<%d %d>", portscheduler_cfg->mcastq_start,
				portscheduler_cfg->mcastq_end);
		snprintf(srcmsg[2], sizeof(srcmsg[2]), "<%d %d>", portscheduler_cfg->l0sp_start,
				portscheduler_cfg->l0sp_end);
		snprintf(srcmsg[3], sizeof(srcmsg[3]), "<%d %d>", portscheduler_cfg->l0cdrr_start,
				portscheduler_cfg->l0cdrr_end);
		snprintf(srcmsg[4], sizeof(srcmsg[4]), "<%d %d>", portscheduler_cfg->l0edrr_start,
				portscheduler_cfg->l0edrr_end);
		snprintf(srcmsg[5], sizeof(srcmsg[5]), "<%d %d>", portscheduler_cfg->l1cdrr_start,
				portscheduler_cfg->l1cdrr_end);
		snprintf(srcmsg[6], sizeof(srcmsg[6]), "<%d %d>", portscheduler_cfg->l1edrr_start,
				portscheduler_cfg->l1edrr_end);
		printk("%6d%11s%11s%9s%11s%11s%11s%11s\n", i, srcmsg[0], srcmsg[1], srcmsg[2], srcmsg[3],
				srcmsg[4], srcmsg[5], srcmsg[6]);
	}
}

void ssdk_dts_reserved_scheduler_dump(a_uint32_t dev_id)
{
	ssdk_dt_portscheduler_cfg *reserved_cfg;
	ssdk_dt_scheduler_cfg *scheduler_cfg;
	a_uint8_t srcmsg[7][16];

	scheduler_cfg = ssdk_bootup_shceduler_cfg_get(dev_id);
	if (!scheduler_cfg) {
		return;
	}

	reserved_cfg = &scheduler_cfg->reserved_pool;

	printk("=============================reserved_scheduler_resource========================="
			"\n");
	printk("reserved   ucastq     mcastq     10sp     10cdrr     10edrr     11cdrr     11edrr"
			"\n");
	snprintf(srcmsg[0], sizeof(srcmsg[0]), "<%d %d>", reserved_cfg->ucastq_start,
			reserved_cfg->ucastq_end);
	snprintf(srcmsg[1], sizeof(srcmsg[1]), "<%d %d>", reserved_cfg->mcastq_start,
			reserved_cfg->mcastq_end);
	snprintf(srcmsg[2], sizeof(srcmsg[2]), "<%d %d>", reserved_cfg->l0sp_start,
			reserved_cfg->l0sp_end);
	snprintf(srcmsg[3], sizeof(srcmsg[3]), "<%d %d>", reserved_cfg->l0cdrr_start,
			reserved_cfg->l0cdrr_end);
	snprintf(srcmsg[4], sizeof(srcmsg[4]), "<%d %d>", reserved_cfg->l0edrr_start,
			reserved_cfg->l0edrr_end);
	snprintf(srcmsg[5], sizeof(srcmsg[5]), "<%d %d>", reserved_cfg->l1cdrr_start,
			reserved_cfg->l1cdrr_end);
	snprintf(srcmsg[6], sizeof(srcmsg[6]), "<%d %d>", reserved_cfg->l1edrr_start,
			reserved_cfg->l1edrr_end);
	printk("      %11s%11s%9s%11s%11s%11s%11s\n", srcmsg[0], srcmsg[1], srcmsg[2], srcmsg[3],
			srcmsg[4], srcmsg[5], srcmsg[6]);
}

void ssdk_dts_l0scheduler_dump(a_uint32_t dev_id)
{
	a_uint32_t i;
	ssdk_dt_l0scheduler_cfg *scheduler_cfg;
	ssdk_dt_scheduler_cfg *cfg;

	cfg = ssdk_bootup_shceduler_cfg_get(dev_id);
	printk("==========================l0scheduler_cfg===========================\n");
	printk("queue     portid     cpri     cdrr_id     epri     edrr_id     sp_id\n");
	for (i = 0; i < SSDK_L0SCHEDULER_CFG_MAX; i++)
	{
		scheduler_cfg = &cfg->l0cfg[i];
		if (scheduler_cfg->valid == 1)
			printk("%5d%11d%9d%12d%9d%12d%10d\n", i, scheduler_cfg->port_id,
				scheduler_cfg->cpri, scheduler_cfg->cdrr_id, scheduler_cfg->epri,
				scheduler_cfg->edrr_id, scheduler_cfg->sp_id);
	}
}

void ssdk_dts_l1scheduler_dump(a_uint32_t dev_id)
{
	a_uint32_t i;
	ssdk_dt_l1scheduler_cfg *scheduler_cfg;
	ssdk_dt_scheduler_cfg *cfg;

	cfg = ssdk_bootup_shceduler_cfg_get(dev_id);

	printk("=====================l1scheduler_cfg=====================\n");
	printk("flow     portid     cpri     cdrr_id     epri     edrr_id\n");
	for (i = 0; i < SSDK_L1SCHEDULER_CFG_MAX; i++)
	{
		scheduler_cfg = &cfg->l1cfg[i];
		if (scheduler_cfg->valid == 1)
			printk("%4d%11d%9d%12d%9d%12d\n", i, scheduler_cfg->port_id,
				scheduler_cfg->cpri, scheduler_cfg->cdrr_id,
				scheduler_cfg->epri, scheduler_cfg->edrr_id);
	}
}
#endif
#endif
static const a_int8_t *qca_phy_feature_str[QCA_PHY_FEATURE_MAX] = {
	"PHY_CLAUSE45",
	"PHY_COMBO",
	"PHY_QGMAC",
	"PHY_XGMAC",
	"PHY_I2C",
	"PHY_INIT",
	"PHY_FORCE",
	"PHY_SFP",
	"PHY_SFP_SGMII",
};

void ssdk_dts_phyinfo_dump(a_uint32_t dev_id)
{
	a_uint32_t i, j;
	phy_info_t *phy_info = hsl_phy_info_get(dev_id);

	printk("=====================port phyinfo========================\n");
	printk("portid     phy_addr     features\n");

	if(!phy_info)
		return;
	for (i = 0; i <= SSDK_MAX_PORT_NUM; i++) {
		if (A_TRUE == hsl_port_prop_check(dev_id, i, HSL_PP_PHY)) {
			printk("%6d%13d%*s", i,
					TO_PHY_ADDR(phy_info->phy_address[i]), 5, "");
			for (j = 0; j < QCA_PHY_FEATURE_MAX; j++) {
				if (phy_info->phy_features[i] & BIT(j) && BIT(j) != PHY_F_INIT) {
					printk(KERN_CONT "%s ", qca_phy_feature_str[j]);
					if (BIT(j) == PHY_F_FORCE) {
						printk(KERN_CONT "(speed: %d, duplex: %s) ",
								phy_info->port_force_speed[i],
								phy_info->port_force_duplex[i] > 0 ?
								"full" : "half");
					}
				}
			}
			printk(KERN_CONT "\n");
		}
	}
}

static ssize_t ssdk_dts_dump(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count;
	a_uint32_t dev_id, dev_num;
	ssdk_reg_map_info map;
	hsl_reg_mode mode;

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "\n");

	dev_num = ssdk_switch_device_num_get();
	for (dev_id = 0; dev_id < dev_num; dev_id ++)
	{
		ssdk_switch_reg_map_info_get(dev_id, &map);
		mode = ssdk_switch_reg_access_mode_get(dev_id);
		printk("=======================================================\n");
		printk("ess-switch\n");
		printk("        reg = <0x%x 0x%x>\n", map.base_addr, map.size);
		if (mode == HSL_REG_LOCAL_BUS)
			printk("        switch_access_mode = <local bus>\n");
		else if (mode == HSL_REG_MDIO)
			printk("        switch_access_mode = <mdio bus>\n");
		else
			printk("        switch_access_mode = <(null)>\n");
		printk("        switch_cpu_bmp = <0x%x>\n", ssdk_cpu_bmp_get(dev_id));
		printk("        switch_lan_bmp = <0x%x>\n", ssdk_lan_bmp_get(dev_id));
		printk("        switch_wan_bmp = <0x%x>\n", ssdk_wan_bmp_get(dev_id));
		printk("        switch_inner_bmp = <0x%x>\n", ssdk_inner_bmp_get(dev_id));
		printk("        switch_mac_mode = <0x%x>\n", ssdk_dt_global_get_mac_mode(dev_id, 0));
		printk("        switch_mac_mode1 = <0x%x>\n", ssdk_dt_global_get_mac_mode(dev_id, 1));
		printk("        switch_mac_mode2 = <0x%x>\n", ssdk_dt_global_get_mac_mode(dev_id, 2));
#ifdef IN_BM
		printk("        bm_tick_mode = <0x%x>\n", ssdk_bm_tick_mode_get(dev_id));
#endif
#ifdef HPPE
#ifdef IN_QOS
		printk("        tm_tick_mode = <0x%x>\n", ssdk_tm_tick_mode_get(dev_id));
#endif
#endif
#ifdef DESS
		printk("ess-psgmii\n");
		ssdk_psgmii_reg_map_info_get(dev_id, &map);
		mode = ssdk_psgmii_reg_access_mode_get(dev_id);
		printk("        reg = <0x%x 0x%x>\n", map.base_addr, map.size);
		if (mode == HSL_REG_LOCAL_BUS)
			printk("        psgmii_access_mode = <local bus>\n");
		else if (mode == HSL_REG_MDIO)
			printk("        psgmii_access_mode = <mdio bus>\n");
		else
			printk("        psgmii_access_mode = <(null)>\n");
#endif
#ifdef IN_UNIPHY
		printk("ess-uniphy\n");
		ssdk_uniphy_reg_map_info_get(dev_id, &map);
		mode = ssdk_uniphy_reg_access_mode_get(dev_id);
		printk("        reg = <0x%x 0x%x>\n", map.base_addr, map.size);
		if (mode == HSL_REG_LOCAL_BUS)
			printk("        uniphy_access_mode = <local bus>\n");
		else if (mode == HSL_REG_MDIO)
			printk("        uniphy_access_mode = <mdio bus>\n");
		else
			printk("        uniphy_access_mode = <(null)>\n");
#endif
#ifdef HPPE
#ifdef IN_QOS
		printk("\n");
		ssdk_dts_port_scheduler_dump(dev_id);
		printk("\n");
		ssdk_dts_reserved_scheduler_dump(dev_id);
		printk("\n");
		ssdk_dts_l0scheduler_dump(dev_id);
		printk("\n");
		ssdk_dts_l1scheduler_dump(dev_id);
#endif
#endif
		printk("\n");
		ssdk_dts_phyinfo_dump(dev_id);
	}

	return count;
}

static a_uint16_t phy_reg_val = 0;
static ssize_t ssdk_phy_write_reg_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char phy_buf[32];
	char *this_opt;
	char *options = phy_buf;
	unsigned int phy_addr, reg_addr, reg_value;
	int ret;

	if (count >= sizeof(phy_buf))
		return 0;
	memcpy(phy_buf, buf, count);
	phy_buf[count] = '\0';

	this_opt = strsep(&options, " ");
	if (!this_opt)
		goto fail;

	ret = kstrtouint(this_opt, 0, &phy_addr);
	if (ret)
		goto fail;

	if ((options - phy_buf) >= (count - 1))
		goto fail;

	this_opt = strsep(&options, " ");
	if (!this_opt)
		goto fail;

	ret = kstrtouint(this_opt, 0, &reg_addr);
	if (ret)
		goto fail;

	if ((options - phy_buf) >= (count - 1))
		goto fail;

	this_opt = strsep(&options, " ");
	if (!this_opt)
		goto fail;

	ret = kstrtouint(this_opt, 0, &reg_value);
	if (ret)
		goto fail;

	hsl_phy_mii_reg_write(0, phy_addr, reg_addr, reg_value);

	return count;

fail:
	printk("Format: phy_addr reg_addr reg_value\n");
	return -EINVAL;
}

static ssize_t ssdk_phy_read_reg_get(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count;

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "reg_val = 0x%x\n", phy_reg_val);
	return count;
}

static ssize_t ssdk_phy_read_reg_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char phy_buf[32];
	char *this_opt;
	char *options = phy_buf;
	unsigned int phy_addr, reg_addr;
	int ret;

	if (count >= sizeof(phy_buf))
		return 0;
	memcpy(phy_buf, buf, count);
	phy_buf[count] = '\0';

	this_opt = strsep(&options, " ");
	if (!this_opt)
		goto fail;

	ret = kstrtouint(this_opt, 0, &phy_addr);
	if (ret)
		goto fail;

	if ((options - phy_buf) >= (count - 1))
		goto fail;

	this_opt = strsep(&options, " ");
	if (!this_opt)
		goto fail;

	ret = kstrtouint(this_opt, 0, &reg_addr);
	if (ret)
		goto fail;

	phy_reg_val = hsl_phy_mii_reg_read(0, phy_addr, reg_addr);

	return count;

fail:
	printk("Format: phy_addr reg_addr\n");
	return -EINVAL;
}

#ifdef IN_LINUX_STD_PTP
static ssize_t ssdk_ptp_counter_get(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count = 0;

	snprintf(buf + PAGE_SIZE - 5, 5, "%zd", count);
	hsl_ptp_event_stat_operation(QCA808X_SSDK_PHY_DRIVER_NAME, buf);

	/* the last 5 bytes save the length of data bytes */
	sscanf(buf + PAGE_SIZE - 5, "%zd", &count);

	return count;
}

static ssize_t ssdk_ptp_counter_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char *op_set = "set";
	hsl_ptp_event_stat_operation(QCA808X_SSDK_PHY_DRIVER_NAME, op_set);

	return count;
}
#endif

#if defined(MHT)
static ssize_t ssdk_clk_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	return ssdk_mht_clk_dump(ssdk_dev_id, buf);
}

static ssize_t ssdk_clk_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char *clk_str, *clk_id, *op_str, *val_str = NULL;
	a_uint32_t op_val;

	clk_str = kstrndup(buf, count, GFP_KERNEL);
	if (!clk_str)
		return -ENOMEM;

	if (clk_str[count - 1] == '\n')
		clk_str[count - 1] = '\0';

	clk_id = strsep(&clk_str, " ");
	if (!clk_id)
		goto parse_fail;

	op_str = strsep(&clk_str, " ");
	if (!op_str)
		goto parse_fail;

	/* the op_val is optinal */
	val_str = strsep(&clk_str, " ");
	if (val_str) {
		if (kstrtou32(val_str, 0, &op_val) < 0)
			goto parse_fail;
	}

	SSDK_DEBUG("clk_id: %s, option: %s %s\n", clk_id, op_str, val_str ? val_str : "");

	if (strncasecmp(op_str, "parent", 6) == 0) {
		if (val_str != NULL)
			ssdk_mht_clk_parent_set(ssdk_dev_id, clk_id, op_val);
		else {
			SSDK_ERROR("parent value needed\n");
			goto parse_fail;
		}
	}

	if (strncasecmp(op_str, "rate", 4) == 0) {
		if (val_str != NULL)
			ssdk_mht_clk_rate_set(ssdk_dev_id, clk_id, op_val);
		else {
			SSDK_ERROR("rate value needed\n");
			goto parse_fail;
		}
	}

	if (strncasecmp(op_str, "reset", 5) == 0) {
		ssdk_mht_clk_reset(ssdk_dev_id, clk_id);
	}

	if (strncasecmp(op_str, "deassert", 8) == 0) {
		ssdk_mht_clk_deassert(ssdk_dev_id, clk_id);
	}

	if (strncasecmp(op_str, "assert", 6) == 0) {
		ssdk_mht_clk_assert(ssdk_dev_id, clk_id);
	}

	if (strncasecmp(op_str, "enable", 6) == 0) {
		ssdk_mht_clk_enable(ssdk_dev_id, clk_id);
	}

	if (strncasecmp(op_str, "disable", 7) == 0) {
		ssdk_mht_clk_disable(ssdk_dev_id, clk_id);
	}

	kfree(clk_str);
	return count;

parse_fail:
	if (clk_str)
		kfree(clk_str);

	SSDK_INFO("clk_cfg supported options:\n"
			"clock_id parent parent_value[0-6]\n"
			"Example: echo mht_gcc_mac1_tx_clk parent 6 > /sys/ssdk/clk_cfg\n"
			"clock_id rate rate_value\n"
			"Example: echo mht_gcc_mac1_tx_clk rate 312500000 > /sys/ssdk/clk_cfg\n"
			"clock_id reset\n"
			"Example: echo mht_gcc_mac1_tx_clk reset > /sys/ssdk/clk_cfg\n"
			"clock_id deassert\n"
			"Example: echo mht_gcc_mac1_tx_clk deassert > /sys/ssdk/clk_cfg\n"
			"clock_id assert\n"
			"Example: echo mht_gcc_mac1_tx_clk assert > /sys/ssdk/clk_cfg\n"
			"clock_id enable\n"
			"Example: echo mht_gcc_mac1_tx_clk enable > /sys/ssdk/clk_cfg\n"
			"clock_id disable\n"
			"Example: echo mht_gcc_mac1_tx_clk disable > /sys/ssdk/clk_cfg\n");

	return -EINVAL;
}
#endif

static const struct device_attribute ssdk_dev_id_attr =
	__ATTR(dev_id, 0660, ssdk_dev_id_get, ssdk_dev_id_set);
static const struct device_attribute ssdk_log_level_attr =
	__ATTR(log_level, 0660, ssdk_log_level_get, ssdk_log_level_set);
static const struct device_attribute ssdk_packet_counter_attr =
	__ATTR(packet_counter, 0660, ssdk_packet_counter_get, ssdk_packet_counter_set);
static const struct device_attribute ssdk_byte_counter_attr =
	__ATTR(byte_counter, 0660, ssdk_byte_counter_get, ssdk_byte_counter_set);
static const struct device_attribute ssdk_dts_dump_attr =
	__ATTR(dts_dump, 0660, ssdk_dts_dump, NULL);
static const struct device_attribute ssdk_phy_write_reg_attr =
	__ATTR(phy_write_reg, 0660, NULL, ssdk_phy_write_reg_set);
static const struct device_attribute ssdk_phy_read_reg_attr =
	__ATTR(phy_read_reg, 0660, ssdk_phy_read_reg_get, ssdk_phy_read_reg_set);
#ifdef IN_LINUX_STD_PTP
static const struct device_attribute ssdk_ptp_counter_attr =
	__ATTR(ptp_packet_counter, 0660, ssdk_ptp_counter_get, ssdk_ptp_counter_set);
#endif
#if defined(MHT)
static const struct device_attribute ssdk_clk_cfg_attr =
	__ATTR(clk_cfg, 0660, ssdk_clk_show, ssdk_clk_store);
#endif

struct kobject *ssdk_sys = NULL;

int ssdk_sysfs_init (void)
{
	int ret = 0;

	/* create /sys/ssdk/ dir */
	ssdk_sys = kobject_create_and_add("ssdk", NULL);
	if (!ssdk_sys) {
		printk("Failed to register SSDK sysfs\n");
		return ret;
	}

	/* create /sys/ssdk/dev_id file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_dev_id_attr.attr);
	if (ret) {
		printk("Failed to register SSDK dev id SysFS file\n");
		goto CLEANUP_1;
	}

	/* create /sys/ssdk/log_level file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_log_level_attr.attr);
	if (ret) {
		printk("Failed to register SSDK log level SysFS file\n");
		goto CLEANUP_2;
	}

	/* create /sys/ssdk/packet_counter file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_packet_counter_attr.attr);
	if (ret) {
		printk("Failed to register SSDK switch counter SysFS file\n");
		goto CLEANUP_3;
	}

	/* create /sys/ssdk/byte_counter file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_byte_counter_attr.attr);
	if (ret) {
		printk("Failed to register SSDK switch counter bytes SysFS file\n");
		goto CLEANUP_4;
	}

	/* create /sys/ssdk/dts_dump file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_dts_dump_attr.attr);
	if (ret) {
		printk("Failed to register SSDK switch show dts SysFS file\n");
		goto CLEANUP_5;
	}

	/* create /sys/ssdk/phy_write_reg file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_phy_write_reg_attr.attr);
	if (ret) {
		printk("Failed to register SSDK phy write reg file\n");
		goto CLEANUP_6;
	}

	/* create /sys/ssdk/phy_read_reg file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_phy_read_reg_attr.attr);
	if (ret) {
		printk("Failed to register SSDK phy read reg file\n");
		goto CLEANUP_7;
	}

#ifdef IN_LINUX_STD_PTP
	/* create /sys/ssdk/ptp_packet_counter file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_ptp_counter_attr.attr);
	if (ret) {
		printk("Failed to register SSDK ptp counter file\n");
		goto CLEANUP_8;
	}
#endif

#if defined(MHT)
	/* create /sys/ssdk/dts_clk file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_clk_cfg_attr.attr);
	if (ret) {
		printk("Failed to register SSDK clk_cfg file\n");
		goto CLEANUP_9;
	}
#endif

	return 0;

#if defined(MHT)
CLEANUP_9:
#if defined(IN_LINUX_STD_PTP)
	sysfs_remove_file(ssdk_sys, &ssdk_ptp_counter_attr.attr);
#endif
#endif

#ifdef IN_LINUX_STD_PTP
CLEANUP_8:
	sysfs_remove_file(ssdk_sys, &ssdk_phy_read_reg_attr.attr);
#endif
CLEANUP_7:
	sysfs_remove_file(ssdk_sys, &ssdk_phy_write_reg_attr.attr);
CLEANUP_6:
	sysfs_remove_file(ssdk_sys, &ssdk_dts_dump_attr.attr);
CLEANUP_5:
	sysfs_remove_file(ssdk_sys, &ssdk_byte_counter_attr.attr);
CLEANUP_4:
	sysfs_remove_file(ssdk_sys, &ssdk_packet_counter_attr.attr);
CLEANUP_3:
	sysfs_remove_file(ssdk_sys, &ssdk_log_level_attr.attr);
CLEANUP_2:
	sysfs_remove_file(ssdk_sys, &ssdk_dev_id_attr.attr);
CLEANUP_1:
	kobject_put(ssdk_sys);

	return ret;
}

void ssdk_sysfs_exit (void)
{
#if defined(MHT)
	sysfs_remove_file(ssdk_sys, &ssdk_clk_cfg_attr.attr);
#endif

#ifdef IN_LINUX_STD_PTP
	sysfs_remove_file(ssdk_sys, &ssdk_ptp_counter_attr.attr);
#endif
	sysfs_remove_file(ssdk_sys, &ssdk_phy_read_reg_attr.attr);
	sysfs_remove_file(ssdk_sys, &ssdk_phy_write_reg_attr.attr);
	sysfs_remove_file(ssdk_sys, &ssdk_dts_dump_attr.attr);
	sysfs_remove_file(ssdk_sys, &ssdk_byte_counter_attr.attr);
	sysfs_remove_file(ssdk_sys, &ssdk_packet_counter_attr.attr);
	sysfs_remove_file(ssdk_sys, &ssdk_log_level_attr.attr);
	sysfs_remove_file(ssdk_sys, &ssdk_dev_id_attr.attr);
	kobject_put(ssdk_sys);
}

/*qca808x_start*/
int
ssdk_plat_init(ssdk_init_cfg *cfg, a_uint32_t dev_id)
{
/*qca808x_end*/
	hsl_reg_mode reg_mode;
	ssdk_reg_map_info map;
	struct clk *  ess_clk;
	struct clk *  cmn_clk;

#ifdef IN_UNIPHY
	reg_mode = ssdk_uniphy_reg_access_mode_get(dev_id);
	if(reg_mode == HSL_REG_LOCAL_BUS) {
		ssdk_uniphy_reg_map_info_get(dev_id, &map);
		qca_phy_priv_global[dev_id]->uniphy_hw_addr = ioremap(map.base_addr,
									map.size);
		if (!qca_phy_priv_global[dev_id]->uniphy_hw_addr) {
			SSDK_ERROR("%s ioremap fail.", __func__);
			cfg->reg_func.uniphy_reg_set = NULL;
			cfg->reg_func.uniphy_reg_get = NULL;
			return -1;
		}
		cfg->reg_func.uniphy_reg_set = qca_uniphy_reg_write;
		cfg->reg_func.uniphy_reg_get = qca_uniphy_reg_read;
	}
#endif
	reg_mode = ssdk_switch_reg_access_mode_get(dev_id);
	if (reg_mode == HSL_REG_LOCAL_BUS) {
		ssdk_switch_reg_map_info_get(dev_id, &map);
		qca_phy_priv_global[dev_id]->hw_addr = ioremap(map.base_addr,
								map.size);
		if (!qca_phy_priv_global[dev_id]->hw_addr) {
			SSDK_ERROR("%s ioremap fail.", __func__);
			return -1;
		}
		ess_clk = ssdk_dts_essclk_get(dev_id);
		cmn_clk = ssdk_dts_cmnclk_get(dev_id);
		if (!IS_ERR(ess_clk)) {
			/* Enable ess clock here */
			SSDK_INFO("Enable ess clk\n");
			clk_prepare_enable(ess_clk);
		} else if (!IS_ERR(cmn_clk)) {
#if defined(HPPE) || defined(MP)
			/* clock ID cmn_ahb_clk defined in DTS */
			ssdk_gcc_clock_init();
#endif
		}

		cfg->reg_mode = HSL_HEADER;
	} else if (reg_mode == HSL_REG_MDIO) {
		cfg->reg_mode = HSL_MDIO;
	}

#ifdef DESS
	reg_mode = ssdk_psgmii_reg_access_mode_get(dev_id);
	if(reg_mode == HSL_REG_LOCAL_BUS) {
		ssdk_psgmii_reg_map_info_get(dev_id, &map);
		if (!request_mem_region(map.base_addr,
					map.size, "psgmii_mem")) {
			SSDK_ERROR("%s Unable to request psgmii resource.", __func__);
			return -1;
		}

		qca_phy_priv_global[dev_id]->psgmii_hw_addr = ioremap(map.base_addr,
								map.size);
		if (!qca_phy_priv_global[dev_id]->psgmii_hw_addr) {
			SSDK_ERROR("%s ioremap fail.", __func__);
			cfg->reg_func.psgmii_reg_set = NULL;
			cfg->reg_func.psgmii_reg_get = NULL;
			return -1;
		}

		cfg->reg_func.psgmii_reg_set = qca_psgmii_reg_write;
		cfg->reg_func.psgmii_reg_get = qca_psgmii_reg_read;
	}
#endif
/*qca808x_start*/

	return 0;
}

void
ssdk_plat_exit(a_uint32_t dev_id)
{
/*qca808x_end*/
	hsl_reg_mode reg_mode;
#ifdef DESS
	ssdk_reg_map_info map;
#endif
/*qca808x_start*/
	SSDK_INFO("ssdk_plat_exit\n");
/*qca808x_end*/
	reg_mode = ssdk_switch_reg_access_mode_get(dev_id);
	if (reg_mode == HSL_REG_LOCAL_BUS) {
		struct clk *cmn_clk = NULL;

		iounmap(qca_phy_priv_global[dev_id]->hw_addr);
		cmn_clk = ssdk_dts_cmnclk_get(dev_id);
		if (!IS_ERR(cmn_clk)) {
#if defined(HPPE) || defined(MP)
			ssdk_gcc_clock_exit();
#endif
		}
	}

#ifdef DESS
	reg_mode = ssdk_psgmii_reg_access_mode_get(dev_id);
	if (reg_mode == HSL_REG_LOCAL_BUS) {
		ssdk_psgmii_reg_map_info_get(dev_id, &map);
		iounmap(qca_phy_priv_global[dev_id]->psgmii_hw_addr);
		release_mem_region(map.base_addr,
                                        map.size);
	}
#endif
#ifdef IN_UNIPHY
	reg_mode = ssdk_uniphy_reg_access_mode_get(dev_id);
	if (reg_mode == HSL_REG_LOCAL_BUS) {
		iounmap(qca_phy_priv_global[dev_id]->uniphy_hw_addr);
	}
#endif
/*qca808x_start*/
}
/*qca808x_end*/

int ssdk_uniphy_valid_check(a_uint32_t dev_id,
		a_uint32_t index, a_uint32_t mode)
{
	int ret = A_TRUE;
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
	a_uint32_t soc_id = 0;
	int rv = 0;
#endif
	if (ssdk_is_emulation(dev_id))
		return A_TRUE;

	if (index > SSDK_UNIPHY_INSTANCE2)
		return A_FALSE;
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
	rv = qcom_smem_get_soc_id(&soc_id);
	if (rv)
		return A_FALSE;
	switch(soc_id) {
		case QCOM_ID_IPQ8070:
		case QCOM_ID_IPQ8071:
		case QCOM_ID_IPQ8072:
		case QCOM_ID_IPQ8074:
		case QCOM_ID_IPQ8076:
		case QCOM_ID_IPQ8078:
		case QCOM_ID_IPQ8070A:
		case QCOM_ID_IPQ8071A:
		case QCOM_ID_IPQ8072A:
		case QCOM_ID_IPQ8074A:
		case QCOM_ID_IPQ8076A:
		case QCOM_ID_IPQ8078A:
			if (index == SSDK_UNIPHY_INSTANCE0) {
				if ((mode == PORT_WRAPPER_USXGMII) ||
					(mode == PORT_WRAPPER_10GBASE_R))
					ret = A_FALSE;
			}
			if ((mode == PORT_WRAPPER_UQXGMII) ||
				(mode == PORT_WRAPPER_UDXGMII))
				ret = A_FALSE;
			break;
		case QCOM_ID_IPQ6000:
		case QCOM_ID_IPQ6005:
		case QCOM_ID_IPQ6010:
		case QCOM_ID_IPQ6018:
		case QCOM_ID_IPQ6028:
			if (index == SSDK_UNIPHY_INSTANCE0) {
				if ((mode == PORT_WRAPPER_USXGMII) ||
					(mode == PORT_WRAPPER_10GBASE_R))
					ret = A_FALSE;
			}
			if (index > SSDK_UNIPHY_INSTANCE1)
				ret = A_FALSE;
			if ((mode == PORT_WRAPPER_UQXGMII) ||
				(mode == PORT_WRAPPER_UDXGMII))
				ret = A_FALSE;
			break;
		case QCOM_ID_IPQ9570:
		case QCOM_ID_IPQ9574:
			break;
		case QCOM_ID_IPQ9550:
		case QCOM_ID_IPQ9554:
			if (index == SSDK_UNIPHY_INSTANCE1)
				ret = A_FALSE;
			break;
		case QCOM_ID_IPQ9510:
		case QCOM_ID_IPQ9514:
			if ((index == SSDK_UNIPHY_INSTANCE1) ||
				(index == SSDK_UNIPHY_INSTANCE2))
				ret = A_FALSE;
			break;
		case QCOM_ID_IPQ5302:
		case QCOM_ID_IPQ5312:
			if (index == SSDK_UNIPHY_INSTANCE2)
				ret = A_FALSE;
			if ((mode == PORT_WRAPPER_USXGMII) ||
				(mode == PORT_WRAPPER_10GBASE_R) ||
				(mode == PORT_WRAPPER_UQXGMII) ||
				(mode == PORT_WRAPPER_UDXGMII))
				ret = A_FALSE;
			break;
		case QCOM_ID_IPQ5300:
		case QCOM_ID_IPQ5322:
		case QCOM_ID_IPQ5332:
		case QCOM_ID_IPQ5321:
			if (index == SSDK_UNIPHY_INSTANCE2)
				ret = A_FALSE;
			break;
		default:
			ret = A_FALSE;
			break;
	}
#else
	switch(index) {
		case SSDK_UNIPHY_INSTANCE0:
			if ((cpu_is_ipq807x() == A_TRUE) ||
				(cpu_is_ipq60xx() == A_TRUE)) {
				if ((mode == PORT_WRAPPER_USXGMII) ||
					(mode == PORT_WRAPPER_10GBASE_R) ||
					(mode == PORT_WRAPPER_UQXGMII) ||
					(mode == PORT_WRAPPER_UDXGMII))
					ret = A_FALSE;
			}
			if (cpu_is_ipq53xx() == A_TRUE) {
				if ((mode == PORT_WRAPPER_UQXGMII) ||
				(mode == PORT_WRAPPER_UDXGMII)) {
					ret = A_FALSE;
				}
				if ((cpu_is_ipq5302() == A_TRUE) ||
					(cpu_is_ipq5312() == A_TRUE)) {
					if ((mode == PORT_WRAPPER_10GBASE_R) ||
						(mode == PORT_WRAPPER_USXGMII))
						ret = A_FALSE;
				}
			}
			break;
		case SSDK_UNIPHY_INSTANCE1:
			ret = cpu_is_uniphy1_enabled();
			if ((cpu_is_ipq807x() == A_TRUE) ||
				(cpu_is_ipq60xx() == A_TRUE) ||
				(cpu_is_ipq53xx() == A_TRUE) ||
				(cpu_is_ipq95xx() == A_TRUE)) {
				if ((mode == PORT_WRAPPER_UQXGMII) ||
				(mode == PORT_WRAPPER_UDXGMII))
					ret = A_FALSE;
			}
			if ((cpu_is_ipq5302() == A_TRUE) ||
				(cpu_is_ipq5312() == A_TRUE)) {
				if ((mode == PORT_WRAPPER_10GBASE_R) ||
					(mode == PORT_WRAPPER_USXGMII))
					ret = A_FALSE;
			}
			break;
		case SSDK_UNIPHY_INSTANCE2:
			ret = cpu_is_uniphy2_enabled();
			if ((cpu_is_ipq807x() == A_TRUE) ||
				(cpu_is_ipq60xx() == A_TRUE) ||
				(cpu_is_ipq95xx() == A_TRUE)) {
				if ((mode == PORT_WRAPPER_UQXGMII) ||
				(mode == PORT_WRAPPER_UDXGMII))
					ret = A_FALSE;
			}
			break;
		default:
			ret = A_FALSE;
			break;
	}
#endif
	return ret;
}
