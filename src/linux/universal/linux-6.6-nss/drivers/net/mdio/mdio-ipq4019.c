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
#include <linux/reset.h>

#define MDIO_MODE_REG				0x40
#define   MDIO_MODE_MDC_MODE			BIT(12)
/* 0 = Clause 22, 1 = Clause 45 */
#define   MDIO_MODE_C45				BIT(8)
#define   MDIO_MODE_DIV_MASK			GENMASK(7, 0)
#define     MDIO_MODE_DIV(x)			FIELD_PREP(MDIO_MODE_DIV_MASK, (x) - 1)
#define     MDIO_MODE_DIV_1			0x0
#define     MDIO_MODE_DIV_2			0x1
#define     MDIO_MODE_DIV_4			0x3
#define     MDIO_MODE_DIV_8			0x7
#define     MDIO_MODE_DIV_16			0xf
#define     MDIO_MODE_DIV_32			0x1f
#define     MDIO_MODE_DIV_64			0x3f
#define     MDIO_MODE_DIV_128			0x7f
#define     MDIO_MODE_DIV_256			0xff
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

#define IPQ4019_MDIO_TIMEOUT	10000
#define IPQ4019_MDIO_SLEEP		10

#define IPQ_HIGH_ADDR_PREFIX	0x18
#define IPQ_LOW_ADDR_PREFIX	0x10


#define SWITCH_REG_TYPE_MASK			GENMASK(31, 28)
#define SWITCH_REG_TYPE_QCA8386			0
#define SWITCH_REG_TYPE_QCA8337			1
#define SWITCH_HIGH_ADDR_DFLT			0x200

/* MDIO clock source frequency is fixed to 100M */
#define IPQ_MDIO_CLK_RATE	100000000

#define IPQ_PHY_SET_DELAY_US	100000

struct ipq4019_mdio_data {
	void __iomem	*membase;
	void __iomem *eth_ldo_rdy;
	struct reset_control *rst;
	struct clk *mdio_clk;
	unsigned int mdc_rate;
	int clk_div;
	void (*preinit)(struct mii_bus *bus);
	u32 (*sw_read)(struct mii_bus *bus, u32 reg);
	void (*sw_write)(struct mii_bus *bus, u32 reg, u32 val);
};

static int ipq4019_mdio_wait_busy(struct mii_bus *bus)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int busy;

	return readl_poll_timeout(priv->membase + MDIO_CMD_REG, busy,
				  (busy & MDIO_CMD_ACCESS_BUSY) == 0,
				  IPQ4019_MDIO_SLEEP, IPQ4019_MDIO_TIMEOUT);
}

static int ipq4019_mdio_read_c45(struct mii_bus *bus, int mii_id, int mmd,
				 int reg)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int data;
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	data = readl(priv->membase + MDIO_MODE_REG);

	data |= MDIO_MODE_C45;

	writel(data, priv->membase + MDIO_MODE_REG);

	/* issue the phy address and mmd */
	writel((mii_id << 8) | mmd, priv->membase + MDIO_ADDR_REG);

	/* issue reg */
	writel(reg, priv->membase + MDIO_DATA_WRITE_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_ADDR;

	/* issue read command */
	writel(cmd, priv->membase + MDIO_CMD_REG);

	/* Wait read complete */
	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_READ;

	writel(cmd, priv->membase + MDIO_CMD_REG);

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;
	/* Read and return data */
	return readl(priv->membase + MDIO_DATA_READ_REG);
}

static int ipq4019_mdio_read_c22(struct mii_bus *bus, int mii_id, int regnum)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int data;
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	data = readl(priv->membase + MDIO_MODE_REG);

	data &= ~MDIO_MODE_C45;

	writel(data, priv->membase + MDIO_MODE_REG);

	/* issue the phy address and reg */
	writel((mii_id << 8) | regnum, priv->membase + MDIO_ADDR_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_READ;

	/* issue read command */
	writel(cmd, priv->membase + MDIO_CMD_REG);

	/* Wait read complete */
	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	/* Read and return data */
	return readl(priv->membase + MDIO_DATA_READ_REG);
}

static int ipq4019_mdio_write_c45(struct mii_bus *bus, int mii_id, int mmd,
				  int reg, u16 value)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	unsigned int data;
	unsigned int cmd;

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	data = readl(priv->membase + MDIO_MODE_REG);

	data |= MDIO_MODE_C45;

	writel(data, priv->membase + MDIO_MODE_REG);

	/* issue the phy address and mmd */
	writel((mii_id << 8) | mmd, priv->membase + MDIO_ADDR_REG);

	/* issue reg */
	writel(reg, priv->membase + MDIO_DATA_WRITE_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_ADDR;

	writel(cmd, priv->membase + MDIO_CMD_REG);

	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	/* issue write data */
	writel(value, priv->membase + MDIO_DATA_WRITE_REG);

	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_C45_WRITE;
	writel(cmd, priv->membase + MDIO_CMD_REG);

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
	data = readl(priv->membase + MDIO_MODE_REG);

	data &= ~MDIO_MODE_C45;

	writel(data, priv->membase + MDIO_MODE_REG);

	/* issue the phy address and reg */
	writel((mii_id << 8) | regnum, priv->membase + MDIO_ADDR_REG);

	/* issue write data */
	writel(value, priv->membase + MDIO_DATA_WRITE_REG);

	/* issue write command */
	cmd = MDIO_CMD_ACCESS_START | MDIO_CMD_ACCESS_CODE_WRITE;

	writel(cmd, priv->membase + MDIO_CMD_REG);

	/* Wait write complete */
	if (ipq4019_mdio_wait_busy(bus))
		return -ETIMEDOUT;

	return 0;
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

void ipq_mii_preinit(struct mii_bus *bus)
{
	struct device_node *np = bus->parent->of_node;
	if (!np)
		return;

	return;
}

static int ipq4019_mdio_set_div(struct ipq4019_mdio_data *priv)
{
	unsigned long ahb_rate;
	int div;
	u32 val;

	/* If we don't have a clock for AHB use the fixed value */
	ahb_rate = IPQ_MDIO_CLK_RATE;
	if (priv->mdio_clk)
		ahb_rate = clk_get_rate(priv->mdio_clk);

	/* MDC rate is ahb_rate/(MDIO_MODE_DIV + 1)
	 * While supported, internal documentation doesn't
	 * assure correct functionality of the MDIO bus
	 * with divider of 1, 2 or 4.
	 */
	for (div = 8; div <= 256; div *= 2) {
		/* The requested rate is supported by the div */
		if (priv->mdc_rate == DIV_ROUND_UP(ahb_rate, div)) {
			val = readl(priv->membase + MDIO_MODE_REG);
			val &= ~MDIO_MODE_DIV_MASK;
			val |= MDIO_MODE_DIV(div);
			writel(val, priv->membase + MDIO_MODE_REG);

			return 0;
		}
	}

	/* The requested rate is not supported */
	return -EINVAL;
}

static int ipq_mdio_reset(struct mii_bus *bus)
{
	struct ipq4019_mdio_data *priv = bus->priv;
	u32 val;
	int ret;

	/* To indicate CMN_PLL that ethernet_ldo has been ready if platform resource 1
	 * is specified in the device tree.
	 */
	if (priv->eth_ldo_rdy) {
		val = readl(priv->eth_ldo_rdy);
		val |= BIT(0);
		writel(val, priv->eth_ldo_rdy);
		fsleep(IPQ_PHY_SET_DELAY_US);
	}

	ret = reset_control_reset(priv->rst);
	if (ret)
		return ret;

	/* Configure MDIO clock source frequency if clock is specified in the device tree */
	ret = clk_set_rate(priv->mdio_clk, IPQ_MDIO_CLK_RATE);
	if (ret)
		return ret;

	ret = clk_prepare_enable(priv->mdio_clk);
	if (ret)
		return ret;

	mdelay(10);

	/* Restore MDC rate */
	return ipq4019_mdio_set_div(priv);
}

static void ipq4019_mdio_select_mdc_rate(struct platform_device *pdev,
					 struct ipq4019_mdio_data *priv)
{
	unsigned long ahb_rate;
	int div;
	u32 val;

	/* MDC rate defined in DT, we don't have to decide a default value */
	if (!of_property_read_u32(pdev->dev.of_node, "clock-frequency",
				  &priv->mdc_rate))
		return;

	/* If we don't have a clock for AHB use the fixed value */
	ahb_rate = IPQ_MDIO_CLK_RATE;
	if (priv->mdio_clk)
		ahb_rate = clk_get_rate(priv->mdio_clk);

	/* Check what is the current div set */
	val = readl(priv->membase + MDIO_MODE_REG);
	div = FIELD_GET(MDIO_MODE_DIV_MASK, val);

	/* div is not set to the default value of /256
	 * Probably someone changed that (bootloader, other drivers)
	 * Keep this and don't overwrite it.
	 */
	if (div != MDIO_MODE_DIV_256) {
		priv->mdc_rate = DIV_ROUND_UP(ahb_rate, div + 1);
		return;
	}

	/* If div is /256 assume nobody have set this value and
	 * try to find one MDC rate that is close the 802.3 spec of
	 * 2.5MHz
	 */
	for (div = 256; div >= 8; div /= 2) {
		/* Stop as soon as we found a divider that
		 * reached the closest value to 2.5MHz
		 */
		if (DIV_ROUND_UP(ahb_rate, div) > 2500000)
			break;

		priv->mdc_rate = DIV_ROUND_UP(ahb_rate, div);
	}
}


static int ipq4019_mdio_probe(struct platform_device *pdev)
{
	struct ipq4019_mdio_data *priv;
	struct mii_bus *bus;
	struct resource *res;
	int ret;

	bus = devm_mdiobus_alloc_size(&pdev->dev, sizeof(*priv));
	if (!bus)
		return -ENOMEM;

	priv = bus->priv;

	priv->membase = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->membase))
		return PTR_ERR(priv->membase);

	priv->rst = devm_reset_control_array_get_optional_exclusive(&pdev->dev);
	if (IS_ERR(priv->rst))
		return PTR_ERR(priv->rst);

	priv->mdio_clk = devm_clk_get_optional(&pdev->dev, "gcc_mdio_ahb_clk");
	if (IS_ERR(priv->mdio_clk))
		return PTR_ERR(priv->mdio_clk);

	ipq4019_mdio_select_mdc_rate(pdev, priv);
	ret = ipq4019_mdio_set_div(priv);
	if (ret)
		return ret;

	/* The platform resource is provided on the chipset IPQ5018 */
	/* This resource is optional */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res) {
		priv->eth_ldo_rdy = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(priv->eth_ldo_rdy))
			return PTR_ERR(priv->eth_ldo_rdy);
	}

	priv->clk_div = 0xf;
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

static int ipq4019_mdio_remove(struct platform_device *pdev)
{
	struct mii_bus *bus = platform_get_drvdata(pdev);

	mdiobus_unregister(bus);

	return 0;
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
