// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2015, The Linux Foundation. All rights reserved. */
/* Copyright (c) 2020 Sartura Ltd. */

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio/consumer.h>

#define MDIO_MODE_REG				0x40
#define MDIO_ADDR_REG				0x44
#define MDIO_DATA_WRITE_REG			0x48
#define MDIO_DATA_READ_REG			0x4c
#define MDIO_CMD_REG				0x50
#define MDIO_CMD_ACCESS_BUSY		BIT(16)
#define MDIO_CMD_ACCESS_START		BIT(8)
#define MDIO_CMD_ACCESS_CODE_READ	0
#define MDIO_CMD_ACCESS_CODE_WRITE	1
#define MDIO_CMD_ACCESS_CODE_C45_ADDR	0
#define MDIO_CMD_ACCESS_CODE_C45_WRITE	1
#define MDIO_CMD_ACCESS_CODE_C45_READ	2

/* 0 = Clause 22, 1 = Clause 45 */
#define MDIO_MODE_C45				BIT(8)
/* MDC frequency is SYS_CLK/(MDIO_CLK_DIV_FACTOR + 1), SYS_CLK is 100MHz */
#define MDIO_CLK_DIV_MASK			GENMASK(7, 0)

#define IPQ4019_MDIO_TIMEOUT	10000
#define IPQ4019_MDIO_SLEEP		10

/* MDIO clock source frequency is fixed to 100M */
#define IPQ_MDIO_CLK_RATE	100000000
#define IPQ_UNIPHY_AHB_CLK_RATE	100000000
#define IPQ_UNIPHY_SYS_CLK_RATE	24000000

#define IPQ_PHY_SET_DELAY_US	100000

#define IPQ_HIGH_ADDR_PREFIX	0x18
#define IPQ_LOW_ADDR_PREFIX	0x10

#define PHY_ADDR_LENGTH		5
#define PHY_ADDR_NUM		4
#define UNIPHY_ADDR_NUM		3

/* qca8386 related */
#define EPHY_CFG				0xC90F018
#define GEPHY0_TX_CBCR				0xC800058
#define SRDS0_SYS_CBCR				0xC8001A8
#define SRDS1_SYS_CBCR				0xC8001AC
#define EPHY0_SYS_CBCR				0xC8001B0
#define EPHY1_SYS_CBCR				0xC8001B4
#define EPHY2_SYS_CBCR				0xC8001B8
#define EPHY3_SYS_CBCR				0xC8001BC
#define GCC_GEPHY_MISC				0xC800304
#define QFPROM_RAW_PTE_ROW2_MSB			0xC900014
#define QFPROM_RAW_CALIBRATION_ROW4_LSB 	0xC900048
#define QFPROM_RAW_CALIBRATION_ROW7_LSB 	0xC900060
#define QFPROM_RAW_CALIBRATION_ROW8_LSB 	0xC900068
#define QFPROM_RAW_CALIBRATION_ROW6_MSB 	0xC90005C
#define PHY_DEBUG_PORT_ADDR			0x1d
#define PHY_DEBUG_PORT_DATA			0x1e
#define PHY_LDO_EFUSE_REG			0x180
#define PHY_ICC_EFUSE_REG			0x280
#define PHY_10BT_SG_THRESH_REG			0x3380
#define PHY_MMD1_CTRL2ANA_OPTION2_REG		0x40018102
#define ETH_LDO_RDY_CNT				3

#define CMN_PLL_REFCLK_INDEX			GENMASK(3, 0)
#define CMN_PLL_REFCLK_EXTERNAL			BIT(9)
#define CMN_ANA_EN_SW_RSTN			BIT(6)

#define CMN_PLL_REFERENCE_CLOCK			0x784
#define CMN_PLL_REFCLK_INDEX			GENMASK(3, 0)
#define CMN_PLL_REFCLK_EXTERNAL			BIT(9)

#define CMN_PLL_POWER_ON_AND_RESET		0x780
#define CMN_ANA_EN_SW_RSTN			BIT(6)

#define CMN_PLL_OUTPUT_RELATED_1		0x79c
#define CMN_PLL_CLK25M_EN			BIT(15)
#define CMN_PLL_CMN_PLL_CLK50M_62P5M_EN		BIT(11)
#define CMN_PLL_CMN_PLL_CLK50M_62P5M_EN1	BIT(10)
#define CMN_PLL_CMN_PLL_CLK50M_62P5M_EN2	BIT(14)

#define SWITCH_REG_TYPE_MASK			GENMASK(31, 28)
#define SWITCH_REG_TYPE_QCA8386			0
#define SWITCH_REG_TYPE_QCA8337			1
#define SWITCH_HIGH_ADDR_DFLT			0x200

enum mdio_clk_id {
	MDIO_CLK_MDIO_AHB,
	MDIO_CLK_UNIPHY0_AHB,
	MDIO_CLK_UNIPHY0_SYS,
	MDIO_CLK_UNIPHY1_AHB,
	MDIO_CLK_UNIPHY1_SYS,
	MDIO_CLK_UNIPHY2_AHB,
	MDIO_CLK_UNIPHY2_SYS,
	MDIO_CLK_CNT
};

struct ipq4019_mdio_data {
	void __iomem	*membase[2];
	void __iomem *eth_ldo_rdy[ETH_LDO_RDY_CNT];
	int clk_div;
	bool force_c22;
	struct gpio_descs *reset_gpios;
	void (*preinit)(struct mii_bus *bus);
	u32 (*sw_read)(struct mii_bus *bus, u32 reg);
	void (*sw_write)(struct mii_bus *bus, u32 reg, u32 val);
	struct clk *clk[MDIO_CLK_CNT];
};

const char * const ppe_clk_name[] = {
	"gcc_mdio_ahb_clk", "uniphy0_ahb_clk", "uniphy0_sys_clk",
	"uniphy1_ahb_clk", "uniphy1_sys_clk",
	"uniphy2_ahb_clk", "uniphy2_sys_clk"
};

static int ipq4019_mdio_wait_busy(struct mii_bus *bus)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int busy;

	return readl_poll_timeout(priv->membase[0] + MDIO_CMD_REG, busy,
				  (busy & MDIO_CMD_ACCESS_BUSY) == 0,
				  IPQ4019_MDIO_SLEEP, IPQ4019_MDIO_TIMEOUT);
}

static int _ipq4019_mdio_read_c45(struct mii_bus *bus, int mii_id, int mmd,
				  int reg)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int data;
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	data = readl(priv->membase[0] + MDIO_MODE_REG);

	data |= MDIO_MODE_C45;
	data |= FIELD_PREP(MDIO_CLK_DIV_MASK, priv->clk_div);

	writel(data, priv->membase[0] + MDIO_MODE_REG);

	/* issue the phy address and mmd */
	writel((mii_id << 8) | mmd, priv->membase[0] + MDIO_ADDR_REG);

	/* issue reg */
	writel(reg, priv->membase[0] + MDIO_DATA_WRITE_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_ADDR;

	/* issue read command */
	writel(cmd, priv->membase[0] + MDIO_CMD_REG);

	/* Wait read complete */
	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_READ;

	writel(cmd, priv->membase[0] + MDIO_CMD_REG);

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	/* Read and return data */
	return readl(priv->membase[0] + MDIO_DATA_READ_REG);
}

static int ipq4019_mdio_read_c22(struct mii_bus *bus, int mii_id, int regnum)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int data;
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	data = readl(priv->membase[0] + MDIO_MODE_REG);

	data &= ~MDIO_MODE_C45;
	data |= FIELD_PREP(MDIO_CLK_DIV_MASK, priv->clk_div);

	writel(data, priv->membase[0] + MDIO_MODE_REG);

	/* issue the phy address and reg */
	writel((mii_id << 8) | regnum, priv->membase[0] + MDIO_ADDR_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_READ;

	/* issue read command */
	writel(cmd, priv->membase[0] + MDIO_CMD_REG);

	/* Wait read complete */
	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	/* Read and return data */
	return readl(priv->membase[0] + MDIO_DATA_READ_REG);
}

static int _ipq4019_mdio_write_c45(struct mii_bus *bus, int mii_id, int mmd,
				   int reg, u16 value)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int data;
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	data = readl(priv->membase[0] + MDIO_MODE_REG);

	data |= MDIO_MODE_C45;
	data |= FIELD_PREP(MDIO_CLK_DIV_MASK, priv->clk_div);

	writel(data, priv->membase[0] + MDIO_MODE_REG);

	/* issue the phy address and mmd */
	writel((mii_id << 8) | mmd, priv->membase[0] + MDIO_ADDR_REG);

	/* issue reg */
	writel(reg, priv->membase[0] + MDIO_DATA_WRITE_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_ADDR;

	writel(cmd, priv->membase[0] + MDIO_CMD_REG);

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	/* issue write data */
	writel(value, priv->membase[0] + MDIO_DATA_WRITE_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_WRITE;
	writel(cmd, priv->membase[0] + MDIO_CMD_REG);

	/* Wait write complete */
	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	return 0;
}

static int ipq4019_mdio_write_c22(struct mii_bus *bus, int mii_id, int regnum,
				  u16 value)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int data;
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	/* Enter Clause 22 mode */
	data = readl(priv->membase[0] + MDIO_MODE_REG);

	data &= ~MDIO_MODE_C45;
	data |= FIELD_PREP(MDIO_CLK_DIV_MASK, priv->clk_div);

	writel(data, priv->membase[0] + MDIO_MODE_REG);

	/* issue the phy address and reg */
	writel((mii_id << 8) | regnum, priv->membase[0] + MDIO_ADDR_REG);

	/* issue write data */
	writel(value, priv->membase[0] + MDIO_DATA_WRITE_REG);

	/* issue write command */
	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_WRITE;

	writel(cmd, priv->membase[0] + MDIO_CMD_REG);

	/* Wait write complete */
	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	return 0;
}

static int ipq4019_mdio_read_c45(struct mii_bus *bus, int mii_id, int mmd,
				 int reg)
{
	struct ipq4019_mdio_data *priv = bus->priv;

	if (priv && priv->force_c22) {
		ipq4019_mdio_write_c22(bus, mii_id, MII_MMD_CTRL, mmd);
		ipq4019_mdio_write_c22(bus, mii_id, MII_MMD_DATA, reg);
		ipq4019_mdio_write_c22(bus, mii_id, MII_MMD_CTRL,
				       mmd | MII_MMD_CTRL_NOINCR);

		return ipq4019_mdio_read_c22(bus, mii_id, MII_MMD_DATA);
	}

	return _ipq4019_mdio_read_c45(bus, mii_id, mmd, reg);
}

static int ipq4019_mdio_write_c45(struct mii_bus *bus, int mii_id, int mmd,
				  int reg, u16 value)
{
	struct ipq4019_mdio_data *priv = bus->priv;

	if (priv && priv->force_c22) {
		ipq4019_mdio_write_c22(bus, mii_id, MII_MMD_CTRL, mmd);
		ipq4019_mdio_write_c22(bus, mii_id, MII_MMD_DATA, reg);
		ipq4019_mdio_write_c22(bus, mii_id, MII_MMD_CTRL,
				       mmd | MII_MMD_CTRL_NOINCR);

		return ipq4019_mdio_write_c22(bus, mii_id, MII_MMD_DATA, value);
	}

	return _ipq4019_mdio_write_c45(bus, mii_id, mmd, reg, value);
}

static inline void qca8337_split_addr(u32 regaddr, u16 *r1, u16 *r2, u16 *page)
{
	regaddr >>= 1;
	*r1 = regaddr & 0x1e;

	regaddr >>= 5;
	*r2 = regaddr & 0x7;

	regaddr >>= 3;
	*page = regaddr & 0x3ff;
}

u32 qca8337_read(struct mii_bus *mii_bus, u32 reg)
{
	u16 r1, r2, page;
	u16 lo, hi;

	qca8337_split_addr(reg, &r1, &r2, &page);
	mii_bus->write(mii_bus, IPQ_HIGH_ADDR_PREFIX, 0, page);
	udelay(100);

	lo = mii_bus->read(mii_bus, IPQ_LOW_ADDR_PREFIX | r2, r1);
	hi = mii_bus->read(mii_bus, IPQ_LOW_ADDR_PREFIX | r2, r1 + 1);

	mii_bus->write(mii_bus, IPQ_HIGH_ADDR_PREFIX, 0, SWITCH_HIGH_ADDR_DFLT);
	return (hi << 16) | lo;
}

void qca8337_write(struct mii_bus *mii_bus, u32 reg, u32 val)
{
	u16 r1, r2, page;

	qca8337_split_addr(reg, &r1, &r2, &page);
	mii_bus->write(mii_bus, IPQ_HIGH_ADDR_PREFIX, 0, page);
	udelay(100);

	mii_bus->write(mii_bus, IPQ_LOW_ADDR_PREFIX | r2, r1, val & 0xffff);
	mii_bus->write(mii_bus, IPQ_LOW_ADDR_PREFIX | r2, r1 + 1, (u16)(val >> 16));

	mii_bus->write(mii_bus, IPQ_HIGH_ADDR_PREFIX, 0, SWITCH_HIGH_ADDR_DFLT);
}

static inline void split_addr(u32 regaddr, u16 *r1, u16 *r2, u16 *page, u16 *sw_addr)
{
	*r1 = regaddr & 0x1c;

	regaddr >>= 5;
	*r2 = regaddr & 0x7;

	regaddr >>= 3;
	*page = regaddr & 0xffff;

	regaddr >>= 16;
	*sw_addr = regaddr & 0xff;
}

u32 qca8386_read(struct mii_bus *bus, unsigned int reg)
{
	u16 r1, r2, page, sw_addr;
	u16 lo, hi;

	split_addr(reg, &r1, &r2, &page, &sw_addr);

	/* There is no competition, so the lock is not needed.
	 * since this function is only called before mii_bus registered.
	 */
	bus->write(bus, IPQ_HIGH_ADDR_PREFIX | (sw_addr >> 5), sw_addr & 0x1f, page);

	lo = bus->read(bus, IPQ_LOW_ADDR_PREFIX | r2, r1);
	hi = bus->read(bus, IPQ_LOW_ADDR_PREFIX | r2, r1 | BIT(1));

	return hi << 16 | lo;
};

int qca8386_write(struct mii_bus *bus, unsigned int reg, unsigned int val)
{
	u16 r1, r2, page, sw_addr;
	u16 lo, hi;

	lo = val & 0xffff;
	hi = (u16)(val >> 16);

	split_addr(reg, &r1, &r2, &page, &sw_addr);

	/* There is no competition, so the lock is not needed.
	 * since this function is only called before mii_bus registered.
	 */
	bus->write(bus, IPQ_HIGH_ADDR_PREFIX | (sw_addr >> 5), sw_addr & 0x1f, page);

	bus->write(bus, IPQ_LOW_ADDR_PREFIX | r2, r1, lo);
	bus->write(bus, IPQ_LOW_ADDR_PREFIX | r2, r1 | BIT(1), hi);

	return 0;
};

u32 ipq_mii_read(struct mii_bus *mii_bus, u32 reg)
{
	u32 val = 0xffffffff;
	switch (FIELD_GET(SWITCH_REG_TYPE_MASK, reg)) {
		case SWITCH_REG_TYPE_QCA8337:
			val = qca8337_read(mii_bus, reg);
			break;
		case SWITCH_REG_TYPE_QCA8386:
		default:
			val = qca8386_read(mii_bus, reg);
			break;
	}

	return val;
}

void ipq_mii_write(struct mii_bus *mii_bus, u32 reg, u32 val)
{
	switch (FIELD_GET(SWITCH_REG_TYPE_MASK, reg)) {
		case SWITCH_REG_TYPE_QCA8337:
			qca8337_write(mii_bus, reg, val);
			break;
		case SWITCH_REG_TYPE_QCA8386:
		default:
			qca8386_write(mii_bus, reg, val);
			break;
	}
}

static inline void ipq_qca8386_clk_enable(struct mii_bus *mii_bus, u32 reg)
{
	u32 val;

	val = ipq_mii_read(mii_bus, reg);
	val |= BIT(0);
	ipq_mii_write(mii_bus, reg, val);
}

static inline void ipq_qca8386_clk_disable(struct mii_bus *mii_bus, u32 reg)
{
	u32 val;

	val = ipq_mii_read(mii_bus, reg);
	val &= ~BIT(0);
	ipq_mii_write(mii_bus, reg, val);
}

static inline void ipq_qca8386_clk_reset(struct mii_bus *mii_bus, u32 reg)
{
	u32 val;

	val = ipq_mii_read(mii_bus, reg);
	val |= BIT(2);
	ipq_mii_write(mii_bus, reg, val);

	usleep_range(20000, 21000);

	val &= ~BIT(2);
	ipq_mii_write(mii_bus, reg, val);
}

static u16 ipq_phy_debug_read(struct mii_bus *mii_bus, u32 phy_addr, u32 reg_id)
{
	mii_bus->write(mii_bus, phy_addr, PHY_DEBUG_PORT_ADDR, reg_id);

	return mii_bus->read(mii_bus, phy_addr, PHY_DEBUG_PORT_DATA);
}

static void ipq_phy_debug_write(struct mii_bus *mii_bus, u32 phy_addr, u32 reg_id, u16 reg_val)
{

	mii_bus->write(mii_bus, phy_addr, PHY_DEBUG_PORT_ADDR, reg_id);

	mii_bus->write(mii_bus, phy_addr, PHY_DEBUG_PORT_DATA, reg_val);
}


static void ipq_phy_addr_fixup(struct mii_bus *bus, struct device_node *np)
{
	void __iomem *ephy_cfg_base;
	struct device_node *child;
	int phy_index, addr, len;
	const __be32 *phy_cfg, *uniphy_cfg;
	u32 val;
	bool mdio_access = false;
	unsigned long phyaddr_mask = 0;

	phy_cfg = of_get_property(np, "phyaddr_fixup", &len);
	uniphy_cfg = of_get_property(np, "uniphyaddr_fixup", NULL);

	/*
	 * For MDIO access, phyaddr_fixup only provides the register address,
	 * as for local bus, the register length also needs to be provided
	 */
	if(!phy_cfg || (len != (2 * sizeof(__be32)) && len != sizeof(__be32)))
		return;

	if (len == sizeof(__be32))
		mdio_access = true;

	if (!mdio_access) {
		ephy_cfg_base = ioremap(be32_to_cpup(phy_cfg), be32_to_cpup(phy_cfg + 1));
		if (!ephy_cfg_base)
			return;
		val = readl(ephy_cfg_base);
	} else
		val = ipq_mii_read(bus, be32_to_cpup(phy_cfg));

	phy_index = 0;
	addr = 0;
	for_each_available_child_of_node(np, child) {
		if (phy_index >= PHY_ADDR_NUM)
			break;

		addr = of_mdio_parse_addr(&bus->dev, child);
		if (addr < 0) {
			continue;
		}
		phyaddr_mask |= BIT(addr);

		if (!of_find_property(child, "fixup", NULL))
			continue;

		addr &= GENMASK(4, 0);
		val &= ~(GENMASK(4, 0) << (phy_index * PHY_ADDR_LENGTH));
		val |= addr << (phy_index * PHY_ADDR_LENGTH);
		phy_index++;
	}

	/* Programe the PHY address */
	dev_info(bus->parent, "Program EPHY reg 0x%x with 0x%x\n",
			be32_to_cpup(phy_cfg), val);

	if (!mdio_access) {
		writel(val, ephy_cfg_base);
		iounmap(ephy_cfg_base);
	} else {
		ipq_mii_write(bus, be32_to_cpup(phy_cfg), val);

		/* Programe the UNIPHY address if uniphyaddr_fixup specified.
		 * the UNIPHY address will select three MDIO address from
		 * unoccupied MDIO address space. */
		if (uniphy_cfg) {
			val = ipq_mii_read(bus, be32_to_cpup(uniphy_cfg));

			/* For qca8386, the switch occupies the other 16 MDIO address,
			 * for example, if the phy address is in the range of 0 to 15,
			 * the switch will occupy the MDIO address from 16 to 31. */
			if (addr > 15)
				phyaddr_mask |= GENMASK(15, 0);
			else
				phyaddr_mask |= GENMASK(31, 16);

			phy_index = 0;
			for_each_clear_bit_from(addr, &phyaddr_mask, PHY_MAX_ADDR) {
				if (phy_index >= UNIPHY_ADDR_NUM)
					break;

				val &= ~(GENMASK(4, 0) << (phy_index * PHY_ADDR_LENGTH));
				val |= addr << (phy_index * PHY_ADDR_LENGTH);
				phy_index++;
			}

			if (phy_index < UNIPHY_ADDR_NUM) {
				for_each_clear_bit(addr, &phyaddr_mask, PHY_MAX_ADDR) {
					if (phy_index >= UNIPHY_ADDR_NUM)
						break;

					val &= ~(GENMASK(4, 0) << (phy_index * PHY_ADDR_LENGTH));
					val |= addr << (phy_index * PHY_ADDR_LENGTH);
					phy_index++;
				}
			}

			dev_info(bus->parent, "Program UNIPHY reg 0x%x with 0x%x\n",
					be32_to_cpup(uniphy_cfg), val);

			ipq_mii_write(bus, be32_to_cpup(uniphy_cfg), val);
		}
	}
}

static void ipq_qca8386_efuse_loading(struct mii_bus *mii_bus, u8 ethphy)
{
	u32 val = 0, ldo_efuse = 0, icc_efuse = 0, phy_addr = 0;
	u16 reg_val = 0;

	phy_addr = ipq_mii_read(mii_bus, EPHY_CFG);
	phy_addr = (phy_addr >> (ethphy * PHY_ADDR_LENGTH)) & GENMASK(4, 0);

	switch(ethphy) {
	case 0:
		val = ipq_mii_read(mii_bus, QFPROM_RAW_CALIBRATION_ROW4_LSB);
		ldo_efuse = FIELD_GET(GENMASK(21, 18), val);
		icc_efuse = FIELD_GET(GENMASK(26, 22), val);
		break;
	case 1:
		val = ipq_mii_read(mii_bus, QFPROM_RAW_CALIBRATION_ROW7_LSB);
		ldo_efuse = FIELD_GET(GENMASK(26, 23), val);
		icc_efuse = FIELD_GET(GENMASK(31, 27), val);
		break;
	case 2:
		val = ipq_mii_read(mii_bus, QFPROM_RAW_CALIBRATION_ROW8_LSB);
		ldo_efuse = FIELD_GET(GENMASK(26, 23), val);
		icc_efuse = FIELD_GET(GENMASK(31, 27), val);
		break;
	case 3:
		val = ipq_mii_read(mii_bus, QFPROM_RAW_CALIBRATION_ROW6_MSB);
		ldo_efuse = FIELD_GET(GENMASK(17, 14), val);
		icc_efuse = FIELD_GET(GENMASK(22, 18), val);
		break;
	}
	reg_val = ipq_phy_debug_read(mii_bus, phy_addr, PHY_LDO_EFUSE_REG);
	reg_val &= ~GENMASK(7, 4);
	reg_val |= FIELD_PREP(GENMASK(7, 4), ldo_efuse);
	ipq_phy_debug_write(mii_bus, phy_addr, PHY_LDO_EFUSE_REG, reg_val);

	reg_val = ipq_phy_debug_read(mii_bus, phy_addr, PHY_ICC_EFUSE_REG);
	reg_val &= ~GENMASK(4, 0);
	reg_val |= FIELD_PREP(GENMASK(4, 0), icc_efuse);
	ipq_phy_debug_write(mii_bus, phy_addr, PHY_ICC_EFUSE_REG, reg_val);
}

static void ipq_qca8386_ethphy_ana_fixup(struct mii_bus *mii_bus, u8 ethphy)
{
	u32 phy_addr = 0;
	u16 reg_val = 0;

	phy_addr = ipq_mii_read(mii_bus, EPHY_CFG);
	phy_addr = (phy_addr >> (ethphy * PHY_ADDR_LENGTH)) & GENMASK(4, 0);

	/*increase 100BT tx amplitude*/
	reg_val = mii_bus->read(mii_bus, phy_addr, PHY_MMD1_CTRL2ANA_OPTION2_REG);
	mii_bus->write(mii_bus, phy_addr, PHY_MMD1_CTRL2ANA_OPTION2_REG, reg_val | 0x7f);

	/*increase 10BT signal detect threshold*/
	reg_val = ipq_phy_debug_read(mii_bus, phy_addr, PHY_10BT_SG_THRESH_REG);
	ipq_phy_debug_write(mii_bus, phy_addr, PHY_10BT_SG_THRESH_REG, reg_val | 0x1);
}

static void ipq_qca8386_clock_init(struct mii_bus *mii_bus)
{
	u32 val = 0;
	int i;

	/* Enable serdes */
	ipq_qca8386_clk_enable(mii_bus, SRDS0_SYS_CBCR);
	ipq_qca8386_clk_enable(mii_bus, SRDS1_SYS_CBCR);

	/* Reset serdes */
	ipq_qca8386_clk_reset(mii_bus, SRDS0_SYS_CBCR);
	ipq_qca8386_clk_reset(mii_bus, SRDS1_SYS_CBCR);

	/* Disable EPHY GMII clock */
	i = 0;
	while (i < 2 * PHY_ADDR_NUM) {
		ipq_qca8386_clk_disable(mii_bus, GEPHY0_TX_CBCR + i*0x20);
		i++;
	}

	/* Enable ephy */
	ipq_qca8386_clk_enable(mii_bus, EPHY0_SYS_CBCR);
	ipq_qca8386_clk_enable(mii_bus, EPHY1_SYS_CBCR);
	ipq_qca8386_clk_enable(mii_bus, EPHY2_SYS_CBCR);
	ipq_qca8386_clk_enable(mii_bus, EPHY3_SYS_CBCR);

	/* Reset ephy */
	ipq_qca8386_clk_reset(mii_bus, EPHY0_SYS_CBCR);
	ipq_qca8386_clk_reset(mii_bus, EPHY1_SYS_CBCR);
	ipq_qca8386_clk_reset(mii_bus, EPHY2_SYS_CBCR);
	ipq_qca8386_clk_reset(mii_bus, EPHY3_SYS_CBCR);

	/* Deassert EPHY DSP */
	val = ipq_mii_read(mii_bus, GCC_GEPHY_MISC);
	val &= ~GENMASK(4, 0);
	ipq_mii_write(mii_bus, GCC_GEPHY_MISC, val);
	/*for ES chips, need to load efuse manually*/
	val = ipq_mii_read(mii_bus, QFPROM_RAW_PTE_ROW2_MSB);
	val = FIELD_GET(GENMASK(23, 16), val);
	if(val == 1 || val == 2) {
		for(i = 0; i < 4; i++)
			ipq_qca8386_efuse_loading(mii_bus, i);
	}
	/*fix 100BT template issue and 10BT threshold issue*/
	for(i = 0; i < 4; i++)
		ipq_qca8386_ethphy_ana_fixup(mii_bus, i);
	/* Enable efuse loading into analog circuit */
	val = ipq_mii_read(mii_bus, EPHY_CFG);
	/* BIT20 for PHY0 and PHY1, BIT21 for PHY2 and PHY3 */
	val &= ~GENMASK(21, 20);
	ipq_mii_write(mii_bus, EPHY_CFG, val);

	/* Sleep 10ms */
	usleep_range(10000, 11000);
}

void ipq_mii_preinit(struct mii_bus *bus)
{
	struct device_node *np = bus->parent->of_node;
	if (!np)
		return;

	ipq_phy_addr_fixup(bus, np);
	if (of_property_read_bool(np, "mdio_clk_fixup"))
		ipq_qca8386_clock_init(bus);

	return;
}
EXPORT_SYMBOL_GPL(ipq_mii_preinit);

static void ipq_cmn_clk_reset(struct mii_bus *bus)
{
	u32 reg_val, clk_en;
	const char *cmn_ref_clk;
	struct device_node *child;
	struct ipq4019_mdio_data *priv = bus->priv;

	if (priv && priv->membase[1]) {
		/* Select reference clock source */
		reg_val = readl(priv->membase[1] + CMN_PLL_REFERENCE_CLOCK);
		reg_val &= ~(CMN_PLL_REFCLK_EXTERNAL | CMN_PLL_REFCLK_INDEX);

		cmn_ref_clk = of_get_property(bus->parent->of_node, "cmn_ref_clk", NULL);
		if (!cmn_ref_clk) {
			/* Internal 48MHZ selected by default */
			reg_val |= FIELD_PREP(CMN_PLL_REFCLK_INDEX, 7);
		} else {
			if (!strcmp(cmn_ref_clk, "external_25MHz"))
				reg_val |= (CMN_PLL_REFCLK_EXTERNAL |
						FIELD_PREP(CMN_PLL_REFCLK_INDEX, 3));
			else if (!strcmp(cmn_ref_clk, "external_31250KHz"))
				reg_val |= (CMN_PLL_REFCLK_EXTERNAL |
						FIELD_PREP(CMN_PLL_REFCLK_INDEX, 4));
			else if (!strcmp(cmn_ref_clk, "external_40MHz"))
				reg_val |= (CMN_PLL_REFCLK_EXTERNAL |
						FIELD_PREP(CMN_PLL_REFCLK_INDEX, 6));
			else if (!strcmp(cmn_ref_clk, "external_48MHz"))
				reg_val |= (CMN_PLL_REFCLK_EXTERNAL |
						FIELD_PREP(CMN_PLL_REFCLK_INDEX, 7));
			else if (!strcmp(cmn_ref_clk, "external_50MHz"))
				reg_val |= (CMN_PLL_REFCLK_EXTERNAL |
						FIELD_PREP(CMN_PLL_REFCLK_INDEX, 8));
			else
				reg_val |= FIELD_PREP(CMN_PLL_REFCLK_INDEX, 7);
		}

		writel(reg_val, priv->membase[1] + CMN_PLL_REFERENCE_CLOCK);

		/* Do the cmn clock reset */
		reg_val = readl(priv->membase[1] + CMN_PLL_POWER_ON_AND_RESET);
		reg_val &= ~CMN_ANA_EN_SW_RSTN;
		writel(reg_val, priv->membase[1]);
		msleep(1);

		reg_val |= CMN_ANA_EN_SW_RSTN;
		writel(reg_val, priv->membase[1] + CMN_PLL_POWER_ON_AND_RESET);
		msleep(1);

		dev_info(bus->parent, "CMN clock reset done\n");

		clk_en = 0;
		for_each_available_child_of_node(bus->parent->of_node, child) {
			if (of_find_property(child, "ref_clk_25m", NULL))
				clk_en |= CMN_PLL_CLK25M_EN;
			else if (of_find_property(child, "ref_clk_50m", NULL))
				clk_en |= CMN_PLL_CMN_PLL_CLK50M_62P5M_EN;
			else if (of_find_property(child, "ref_clk_50m_1", NULL))
				clk_en |= CMN_PLL_CMN_PLL_CLK50M_62P5M_EN1;
			else if (of_find_property(child, "ref_clk_50m_2", NULL))
				clk_en |= CMN_PLL_CMN_PLL_CLK50M_62P5M_EN2;
		}

		if (clk_en) {
			reg_val = readl(priv->membase[1] + CMN_PLL_OUTPUT_RELATED_1);
			reg_val &= ~(CMN_PLL_CLK25M_EN | CMN_PLL_CMN_PLL_CLK50M_62P5M_EN |
					CMN_PLL_CMN_PLL_CLK50M_62P5M_EN1 |
					CMN_PLL_CMN_PLL_CLK50M_62P5M_EN2);

			reg_val |= clk_en;
			writel(reg_val, priv->membase[1] + CMN_PLL_OUTPUT_RELATED_1);

			dev_info(bus->parent, "CMN output clock select %x\n", clk_en);
		}
	}
}

static int ipq_mdio_reset(struct mii_bus *bus)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	u32 val;
	int ret, i;

	ipq_cmn_clk_reset(bus);

	/* For the platform ipq5332, the uniphy clock should be configured for resetting
	 * the connected device such as qca8386 switch or qca8081 PHY.
	 */
	if (of_machine_is_compatible("qcom,ipq5332")) {
		unsigned long rate = 0;

		for (i = MDIO_CLK_UNIPHY0_AHB; i < MDIO_CLK_CNT; i++) {
			switch (i) {
			case MDIO_CLK_UNIPHY0_AHB:
			case MDIO_CLK_UNIPHY1_AHB:
			case MDIO_CLK_UNIPHY2_AHB:
				rate = IPQ_UNIPHY_AHB_CLK_RATE;
				break;
			case MDIO_CLK_UNIPHY0_SYS:
			case MDIO_CLK_UNIPHY1_SYS:
			case MDIO_CLK_UNIPHY2_SYS:
				rate = IPQ_UNIPHY_SYS_CLK_RATE;
				break;
			default:
				break;
			}
			ret = clk_set_rate(priv->clk[i], rate);
			if (ret)
				continue;

			ret = clk_prepare_enable(priv->clk[i]);
		}
	}

	/* To indicate CMN_PLL that ethernet_ldo has been ready if the additional
	 * platform resources are specified in the device tree.
	 */
	for (i = 0; i < ETH_LDO_RDY_CNT; i++) {
		if (priv->eth_ldo_rdy[i]) {
			val = readl(priv->eth_ldo_rdy[i]);
			val |= BIT(0);
			writel(val, priv->eth_ldo_rdy[i]);
			fsleep(IPQ_PHY_SET_DELAY_US);
		}
	}

	/* Do the optional reset on the devices connected with MDIO bus */
	if (priv->reset_gpios) {
		unsigned long *values = bitmap_zalloc(priv->reset_gpios->ndescs, GFP_KERNEL);
		if (!values)
			return -ENOMEM;

		bitmap_fill(values, priv->reset_gpios->ndescs);
		gpiod_set_array_value_cansleep(priv->reset_gpios->ndescs, priv->reset_gpios->desc,
				priv->reset_gpios->info, values);

		fsleep(IPQ_PHY_SET_DELAY_US);

		bitmap_zero(values, priv->reset_gpios->ndescs);
		gpiod_set_array_value_cansleep(priv->reset_gpios->ndescs, priv->reset_gpios->desc,
				priv->reset_gpios->info, values);
		bitmap_free(values);
	}

	/* Configure MDIO clock source frequency if clock is specified in the device tree */
	ret = clk_set_rate(priv->clk[MDIO_CLK_MDIO_AHB], IPQ_MDIO_CLK_RATE);
	if (ret)
		return ret;

	ret = clk_prepare_enable(priv->clk[MDIO_CLK_MDIO_AHB]);
	if (ret == 0) {
		mdelay(10);

		/* Configure the fixup PHY address and clocks for qca8386 chip if specified */
		priv->preinit(bus);
	}

	return ret;
}

static int ipq4019_mdio_probe(struct platform_device *pdev)
{
	struct ipq4019_mdio_data *priv;
	struct mii_bus *bus;
	struct resource *res;
	int ret, i;
	printk(KERN_INFO "reprobe mdio\n");
	bus = devm_mdiobus_alloc_size(&pdev->dev, sizeof(*priv));
	if (!bus)
		return -ENOMEM;

	priv = bus->priv;

	priv->membase[0] = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->membase[0]))
		return PTR_ERR(priv->membase[0]);

	/* The CMN block resource is for providing clock to ethernet, which is only
	 * for the platform ipq95xx/ipq53xx.
	 */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cmn_blk");
	if (res) {
		priv->membase[1] = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(priv->membase[1]))
			return PTR_ERR(priv->membase[1]);
	}

	for (i = 0; i < MDIO_CLK_CNT; i++) {
		priv->clk[i] = devm_clk_get_optional(&pdev->dev, ppe_clk_name[i]);
		if (IS_ERR(priv->clk[i]))
			return PTR_ERR(priv->clk[i]);
	}

	/* The platform resource is provided on the chipset IPQ5018 */
	/* This resource is optional */
	for (i = 0; i < ETH_LDO_RDY_CNT; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i + 1);
		if (res && strcmp(res->name, "cmn_blk")) {
			priv->eth_ldo_rdy[i] = devm_ioremap_resource(&pdev->dev, res);
			if (IS_ERR(priv->eth_ldo_rdy[i]))
				return PTR_ERR(priv->eth_ldo_rdy[i]);
		}
	}

	priv->reset_gpios= devm_gpiod_get_array_optional(&pdev->dev, "phy-reset", GPIOD_OUT_LOW);
	if (IS_ERR(priv->reset_gpios)) {
		ret = dev_err_probe(&pdev->dev, PTR_ERR(priv->reset_gpios),
				    "mii_bus %s couldn't get reset GPIO\n",
				    bus->id);
		return ret;
	}
//	ipq_mdio_reset(bus);
	/* MDIO default frequency is 6.25MHz */
	priv->clk_div = 0xf;
	priv->force_c22 = of_property_read_bool(pdev->dev.of_node, "force_clause22");

	priv->preinit = ipq_mii_preinit;
	priv->sw_read = ipq_mii_read;
	priv->sw_write = ipq_mii_write;

	bus->name = "ipq4019_mdio";
	bus->read = ipq4019_mdio_read_c22;
	bus->write = ipq4019_mdio_write_c22;
	bus->read_c45 = ipq4019_mdio_read_c45;
	bus->write_c45 = ipq4019_mdio_write_c45;
	bus->reset = ipq_mdio_reset;
	bus->parent = &pdev->dev;
	snprintf(bus->id, MII_BUS_ID_SIZE, "%s%d", pdev->name, pdev->id);

	ret = of_mdiobus_register(bus, pdev->dev.of_node);
	if (ret) {
		dev_err(&pdev->dev, "Cannot register MDIO bus!\n");
		return ret;
	}

	platform_set_drvdata(pdev, bus);

	return 0;
}

static void ipq4019_mdio_remove(struct platform_device *pdev)
{
	struct mii_bus *bus = platform_get_drvdata(pdev);

	mdiobus_unregister(bus);

	return;
}

static const struct of_device_id ipq4019_mdio_dt_ids[] = {
	{ .compatible = "qcom,ipq4019-mdio" },
	{ .compatible = "qcom,ipq5018-mdio" },
	{ .compatible = "qcom,qca-mdio" },
	{ }
};
MODULE_DEVICE_TABLE(of, ipq4019_mdio_dt_ids);

static struct platform_driver ipq4019_mdio_driver = {
	.probe = ipq4019_mdio_probe,
	.remove = ipq4019_mdio_remove,
	.driver = {
		.name = "ipq4019-mdio",
		.of_match_table = ipq4019_mdio_dt_ids,
	},
};

module_platform_driver(ipq4019_mdio_driver);

MODULE_DESCRIPTION("ipq4019 MDIO interface driver");
MODULE_AUTHOR("Qualcomm Atheros");
MODULE_LICENSE("Dual BSD/GPL");
