/*
 * UNIPHY is the PCS between MAC and PHY which controls the mode of
 * physical ports. Depends on different SoC, it can support
 * SGMII/SGMII+/USXGMII. What's more, in some SoC it also support
 * QSGMII/PSGMII which combine multi SGMII line into single physical port.
 *
 * =======================================================================
 *        ________________________________
 *       |  _______   IPQ807x             |
 *       | | GMAC0 |__                    |
 *       | |_______|  \                   |               _________
 *       |  _______    \                  |          ____|   GPHY  |
 *       | | GMAC1 |__  \     _________   |         /    | /Switch |
 *       | |_______|  \  \___|         |  |     SGMII(+) |_________|
 *       |  _______    \_____|         |  | P0    /
 *       | | GMAC2 |_________| UNIPHY0 |--|-----or
 *       | |_______|    _____|         |  |       \
 *       |  _______    /   __|         |  |    (Q/P)SGMII __________
 *       | | GMAC3 |__/   /  |_________|  |         \____| (Q/P)PHY |
 *       | |_______|     /                |              |__________|
 *       |  _______     /                 |
 *       | | GMAC4 |--or                  |               _________
 *       | |_______|    \     _________   | P1           | (X)GPHY |
 *       |  ________     or--| UNIPHY1 |--|----SGMII(+)--| /Switch |
 *       | | XGMAC0 |___/    |_________|  |    /USXGMII  |_________|
 *       | |________|                     |
 *       |  ________                      |
 *       | | GMAC5  |___                  |               _________
 *       | |________|   \     _________   | P2           | (X)GPHY |
 *       |  ________     or--| UNIPHY2 |--|----SGMII(+)--| /Switch |
 *       | | XGMAC1 |___/    |_________|  |    /USXGMII  |_________|
 *       | |________|                     |
 *       |________________________________|
 *
 * =======================================================================
 *       _________________________________
 *       |  _______   IPQ50xx   ______    | P0             ______
 *       | | GMAC0 |___________| GPHY |---|------UTP------| RJ45 |
 *       | |_______|           |______|   |               |______|
 *       |  _______           _________   |               _________
 *       | | GMAC1 |_________| UNIPHY0 |  | P1           |   GPHY  |
 *       | |_______|         |_________|--|----SGMII(+)--| /Switch |
 *       |________________________________|              |_________|
 *
 * =======================================================================
 */

#include <dt-bindings/phy/qcom-eth-uniphy.h>
#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/reset.h>


#define TCSR_ETH_CMN				0x0
#define  TCSR_ETH_CMN_ENABLE			BIT(0)


#define CMN_PLL_REFCLK_SRC			0x28
#define  CMN_PLL_REFCLK_SRC_FROM_MASK		GENMASK(9, 8)
#define   CMN_PLL_REFCLK_SRC_FROM(x)		FIELD_PREP(CMN_PLL_REFCLK_SRC_FROM_MASK, (x))
#define   CMN_PLL_REFCLK_SRC_FROM_REG		CMN_PLL_REFCLK_SRC_FROM(0)
#define   CMN_PLL_REFCLK_SRC_FROM_LOGIC		CMN_PLL_REFCLK_SRC_FROM(1)
#define   CMN_PLL_REFCLK_SRC_FROM_PCS		CMN_PLL_REFCLK_SRC_FROM(2)

#define CMN_PLL_REFCLK				0x784
#define  CMN_PLL_REFCLK_EXTERNAL		BIT(9)
#define  CMN_PLL_REFCLK_DIV_MASK		GENMASK(8, 4)
#define   CMN_PLL_REFCLK_DIV(x)			FIELD_PREP(CMN_PLL_REFCLK_DIV_MASK, (x))
#define  CMN_PLL_REFCLK_FREQ_MASK		GENMASK(3, 0)
#define   CMN_PLL_REFCLK_FREQ(x)		FIELD_PREP(CMN_PLL_REFCLK_FREQ_MASK, (x))
#define   CMN_PLL_REFCLK_FREQ_25M		CMN_PLL_REFCLK_FREQ(3)
#define   CMN_PLL_REFCLK_FREQ_31250K		CMN_PLL_REFCLK_FREQ(4)
#define   CMN_PLL_REFCLK_FREQ_40M		CMN_PLL_REFCLK_FREQ(6)
#define   CMN_PLL_REFCLK_FREQ_48M		CMN_PLL_REFCLK_FREQ(7)
#define   CMN_PLL_REFCLK_FREQ_50M		CMN_PLL_REFCLK_FREQ(8)

#define CMN_PLL_CTRL				0x780
#define  CMN_PLL_CTRL_RST_N			BIT(6)

#define CMN_PLL_STATUS				0x64
#define  CMN_PLL_STATUS_LOCKED			BIT(2)


#define IPQ50XX_UNIPHY_CLKOUT			0x74
#define  IPQ50XX_UNIPHY_CLKOUT_DS_MASK		GENMASK(3, 2)
#define   IPQ50XX_UNIPHY_CLKOUT_DS(x)		FIELD_PREP(IPQ50XX_UNIPHY_CLKOUT_DS_MASK, (x))
#define   IPQ50XX_UNIPHY_CLKOUT_DS_2_8V		IPQ50XX_UNIPHY_CLKOUT_DS(0)
#define   IPQ50XX_UNIPHY_CLKOUT_DS_1_5V		IPQ50XX_UNIPHY_CLKOUT_DS(1)
#define  IPQ50XX_UNIPHY_CLKOUT_DIV_MASK		GENMASK(1, 1)
#define   IPQ50XX_UNIPHY_CLKOUT_DIV(x)		FIELD_PREP(IPQ50XX_UNIPHY_CLKOUT_DIV_MASK, (x))
#define   IPQ50XX_UNIPHY_CLKOUT_DIV_50M		IPQ50XX_UNIPHY_CLKOUT_DIV(0)
#define   IPQ50XX_UNIPHY_CLKOUT_DIV_25M		IPQ50XX_UNIPHY_CLKOUT_DIV(1)
#define  IPQ50XX_UNIPHY_CLKOUT_ENABLE		BIT(0)

#define IPQ53XX_UNIPHY_CLKOUT			0x610
#define  IPQ53XX_UNIPHY_CLKOUT_LDO_LEVEL_MASK	GENMASK(10, 8)
#define  IPQ53XX_UNIPHY_CLKOUT_DIV_MASK		GENMASK(5, 5)
#define   IPQ53XX_UNIPHY_CLKOUT_DIV(x)		FIELD_PREP(IPQ53XX_UNIPHY_CLKOUT_DIV_MASK, (x))
#define   IPQ53XX_UNIPHY_CLKOUT_DIV_50M		IPQ53XX_UNIPHY_CLKOUT_DIV(0)
#define   IPQ53XX_UNIPHY_CLKOUT_DIV_25M		IPQ53XX_UNIPHY_CLKOUT_DIV(1)
#define  IPQ53XX_UNIPHY_CLKOUT_PULLDOWN		BIT(3)
#define  IPQ53XX_UNIPHY_CLKOUT_DS_MASK		GENMASK(2, 1)
#define   IPQ53XX_UNIPHY_CLKOUT_DS(x)		FIELD_PREP(IPQ53XX_UNIPHY_CLKOUT_DS_MASK, (x))
#define   IPQ53XX_UNIPHY_CLKOUT_DS_2_8V		IPQ53XX_UNIPHY_CLKOUT_DS(0)
#define   IPQ53XX_UNIPHY_CLKOUT_DS_1_5V		IPQ53XX_UNIPHY_CLKOUT_DS(1)
#define  IPQ53XX_UNIPHY_CLKOUT_ENABLE		BIT(0)


#define UNIPHY_MODE				0x46c
#define  UNIPHY_MODE_USXG			BIT(13)
#define  UNIPHY_MODE_XPCS			BIT(12)
#define  UNIPHY_MODE_SGMIIPLUS			BIT(11)
#define  UNIPHY_MODE_SGMII			BIT(10)
#define  UNIPHY_MODE_PSGMII			BIT(9)
#define  UNIPHY_MODE_QSGMII			BIT(8)
#define  UNIPHY_MODE_CH0_MODE_MASK		GENMASK(6, 4)
#define   UNIPHY_MODE_CH0_MODE(x)		FIELD_PREP(UNIPHY_MODE_CH0_MODE_MASK, (x))
#define   UNIPHY_MODE_CH0_MODE_1000BASEX	UNIPHY_MODE_CH0_MODE(0)
#define   UNIPHY_MODE_CH0_MODE_MAC		UNIPHY_MODE_CH0_MODE(2)
#define  UNIPHY_MODE_SGMII_CHANNEL_MASK		GENMASK(2, 1)
#define   UNIPHY_MODE_SGMII_CHANNEL(x)		FIELD_PREP(UNIPHY_MODE_SGMII_CHANNEL_MASK, (x))
#define   UNIPHY_MODE_SGMII_CHANNEL_0		UNIPHY_MODE_SGMII_CHANNEL(0)
#define   UNIPHY_MODE_SGMII_CHANNEL_1		UNIPHY_MODE_SGMII_CHANNEL(1)
#define   UNIPHY_MODE_SGMII_CHANNEL_4		UNIPHY_MODE_SGMII_CHANNEL(2)
#define  UNIPHY_MODE_AN_MODE_MASK		BIT(0)
#define   UNIPHY_MODE_AN_MODE(x)		FIELD_PREP(UNIPHY_MODE_AN_MODE_MASK, (x))
#define   UNIPHY_MODE_AN_MODE_ATHEROS		UNIPHY_MODE_AN_MODE(0)
#define   UNIPHY_MODE_AN_MODE_STANDARD		UNIPHY_MODE_AN_MODE(1)

#define UNIPHY_PLL_CTRL				0x780
#define  UNIPHY_PLL_CTRL_RST_N			BIT(6)

#define UNIPHY_CALIBRATION			0x1E0
#define  UNIPHY_CALIBRATION_DONE		BIT(7)


#define UNIPHY_CHANNEL(x)			(0x480 + 0x18 * (x))
#define  UNIPHY_CHANNEL_RSTN			BIT(11)
#define  UNIPHY_CHANNEL_FORCE_SPEED_25M		BIT(3)

#define UNIPHY_SGMII				0x218
#define  UNIPHY_SGMII_MODE_MASK			GENMASK(6, 4)
#define   UNIPHY_SGMII_MODE(x)			FIELD_PREP(UNIPHY_SGMII_MODE_MASK, (x))
#define   UNIPHY_SGMII_MODE_SGMII		UNIPHY_SGMII_MODE(3)
#define   UNIPHY_SGMII_MODE_SGMIIPLUS		UNIPHY_SGMII_MODE(5)
#define   UNIPHY_SGMII_MODE_USXGMII		UNIPHY_SGMII_MODE(7)
#define  UNIPHY_SGMII_RATE_MASK			GENMASK(1, 0)
#define   UNIPHY_SGMII_RATE(x)			FIELD_PREP(UNIPHY_SGMII_RATE_MASK, (x))


#define SGMII_CLK_RATE				125000000 /* 125M */
#define SGMII_PLUS_CLK_RATE			312500000 /* 312.5M */


struct qcom_eth_uniphy {
	struct device *dev;
	void __iomem *base;
	int num_clks;
	struct clk_bulk_data *clks;
	struct reset_control *rst;

	int mode;

	struct clk_hw *clk_rx, *clk_tx;
	struct clk_hw_onecell_data *clk_data;
};


#define rmwl(addr, mask, val) \
	writel(((readl(addr) & ~(mask)) | ((val) & (mask))), addr)

static int cmn_init(struct platform_device *pdev)
{
	struct resource *res;
	void __iomem *cmn_base;
	void __iomem *tcsr_base;
	u32 val;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cmn");
	if (!res)
		return 0;

	cmn_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR_OR_NULL(cmn_base))
		return PTR_ERR(cmn_base);

	/* For IPQ50xx, tcsr is necessary to enable cmn block */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "tcsr");
	if (res) {
		tcsr_base = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR_OR_NULL(tcsr_base))
			return PTR_ERR(tcsr_base);

		rmwl((tcsr_base + TCSR_ETH_CMN), TCSR_ETH_CMN_ENABLE,
		     TCSR_ETH_CMN_ENABLE);
	}

	rmwl((cmn_base + CMN_PLL_REFCLK_SRC),
	     CMN_PLL_REFCLK_SRC_FROM_MASK,
	     CMN_PLL_REFCLK_SRC_FROM_REG);
	rmwl((cmn_base + CMN_PLL_REFCLK),
	     (CMN_PLL_REFCLK_EXTERNAL | CMN_PLL_REFCLK_FREQ_MASK
	      | CMN_PLL_REFCLK_DIV_MASK),
	     (CMN_PLL_REFCLK_FREQ_48M | CMN_PLL_REFCLK_DIV(2)));

	rmwl((cmn_base + CMN_PLL_CTRL), CMN_PLL_CTRL_RST_N, 0);
	msleep(1);
	rmwl((cmn_base + CMN_PLL_CTRL), CMN_PLL_CTRL_RST_N,
	     CMN_PLL_CTRL_RST_N);
	msleep(1);

	return read_poll_timeout(readl, val,
				 (val & CMN_PLL_STATUS_LOCKED),
				 100, 200000, false,
				 (cmn_base + CMN_PLL_STATUS));
	
	return 0;
}


static void uniphy_write(struct qcom_eth_uniphy *uniphy, int addr, u32 val)
{
	writel(val, (uniphy->base + addr));
}

static u32 uniphy_read(struct qcom_eth_uniphy *uniphy, int addr)
{
	return readl((uniphy->base + addr));
}

static void uniphy_rmw(struct qcom_eth_uniphy *uniphy, int addr, u32 mask, u32 val)
{
	u32 v = uniphy_read(uniphy, addr);
	v &= ~mask;
	v |= val & mask;
	uniphy_write(uniphy, addr, v);
}

static int uniphy_clkout_init(struct qcom_eth_uniphy *uniphy)
{
	u32 val;
	int ret;

	ret = of_property_read_u32(uniphy->dev->of_node, "clkout-frequency", &val);
	if (ret == -EINVAL)
		return 0;
	else if (ret < 0)
		return ret;

	switch(val) {
		case QCOM_ETH_UNIPHY_CLKOUT_FREQ_25M:
			uniphy_rmw(uniphy, IPQ50XX_UNIPHY_CLKOUT,
				   IPQ50XX_UNIPHY_CLKOUT_DIV_MASK,
				   IPQ50XX_UNIPHY_CLKOUT_DIV_25M);
			break;
		case QCOM_ETH_UNIPHY_CLKOUT_FREQ_50M:
			uniphy_rmw(uniphy, IPQ50XX_UNIPHY_CLKOUT,
				   IPQ50XX_UNIPHY_CLKOUT_DIV_MASK,
				   IPQ50XX_UNIPHY_CLKOUT_DIV_50M);
			break;
		default:
			dev_err(uniphy->dev, "Unsupported clkout-frequency: %d\n", val);
			return -EINVAL;
	}

	ret = of_property_read_u32(uniphy->dev->of_node, "clkout-drive-strength", &val);
	if (ret != -EINVAL) {
		if (ret < 0)
			return ret;

		switch(val) {
			case QCOM_ETH_UNIPHY_CLKOUT_DS_1_5V:
				uniphy_rmw(uniphy, IPQ50XX_UNIPHY_CLKOUT,
					   IPQ50XX_UNIPHY_CLKOUT_DS_MASK,
					   IPQ50XX_UNIPHY_CLKOUT_DS_1_5V);
				break;
			case QCOM_ETH_UNIPHY_CLKOUT_DS_2_8V:
				uniphy_rmw(uniphy, IPQ50XX_UNIPHY_CLKOUT,
					   IPQ50XX_UNIPHY_CLKOUT_DS_MASK,
					   IPQ50XX_UNIPHY_CLKOUT_DS_2_8V);
				break;
			default:
				dev_err(uniphy->dev, "Unsupported clkout-drive-strength: %d\n", val);
				return -EINVAL;
		}

	}

	uniphy_rmw(uniphy, IPQ50XX_UNIPHY_CLKOUT,
		   IPQ50XX_UNIPHY_CLKOUT_ENABLE,
		   IPQ50XX_UNIPHY_CLKOUT_ENABLE);

	return 0;
}

static int uniphy_mode_set(struct qcom_eth_uniphy *uniphy)
{
	int ret;

	ret = of_property_read_u32(uniphy->dev->of_node, "mode",
				   &uniphy->mode);
	if (ret < 0)
		return ret;

	switch(uniphy->mode) {
		case QCOM_ETH_UNIPHY_MODE_SGMII:
			uniphy_write(uniphy, UNIPHY_MODE,
				     UNIPHY_MODE_SGMII);
			uniphy_rmw(uniphy, UNIPHY_SGMII,
				   UNIPHY_SGMII_MODE_MASK,
				   UNIPHY_SGMII_MODE_SGMII);
			break;
		default:
			dev_err(uniphy->dev, "Unsupported mode: %d\n",
				uniphy->mode);
			return -EINVAL;
	}

	return 0;
}

static int uniphy_calibrate(struct qcom_eth_uniphy *uniphy)
{
	u32 val;

	uniphy_rmw(uniphy, UNIPHY_PLL_CTRL, UNIPHY_PLL_CTRL_RST_N, 0);
	msleep(1);
	uniphy_rmw(uniphy, UNIPHY_PLL_CTRL, UNIPHY_PLL_CTRL_RST_N,
		   UNIPHY_PLL_CTRL_RST_N);
	msleep(1);

	return read_poll_timeout(uniphy_read, val,
				 (val & UNIPHY_CALIBRATION_DONE),
				 100, 200000, false,
				 uniphy, UNIPHY_CALIBRATION);
}

static int uniphy_clk_register(struct qcom_eth_uniphy *uniphy)
{
	unsigned long rate;
	char name[64];
	int ret;

	switch (uniphy->mode) {
		case QCOM_ETH_UNIPHY_MODE_SGMII:
			rate = SGMII_CLK_RATE;
			break;
	}

	snprintf(name, sizeof(name), "%s#rx", dev_name(uniphy->dev));
	uniphy->clk_rx = clk_hw_register_fixed_rate(uniphy->dev, name,
						    NULL, 0, rate);
	if (IS_ERR_OR_NULL(uniphy->clk_rx))
		return dev_err_probe(uniphy->dev, PTR_ERR(uniphy->clk_rx),
				     "failed to register rx clock\n");

	snprintf(name, sizeof(name), "%s#tx", dev_name(uniphy->dev));
	uniphy->clk_tx = clk_hw_register_fixed_rate(uniphy->dev, name,
						    NULL, 0, rate);
	if (IS_ERR_OR_NULL(uniphy->clk_tx))
		return dev_err_probe(uniphy->dev, PTR_ERR(uniphy->clk_tx),
				     "failed to register rx clock\n");

	uniphy->clk_data = devm_kzalloc(uniphy->dev,
					struct_size(uniphy->clk_data, hws, 2),
					GFP_KERNEL);
	if (!uniphy->clk_data)
		return dev_err_probe(uniphy->dev, -ENOMEM,
				     "failed to allocate clk_data\n");

	uniphy->clk_data->num = 2;
	uniphy->clk_data->hws[0] = uniphy->clk_rx;
	uniphy->clk_data->hws[1] = uniphy->clk_tx;
	ret = of_clk_add_hw_provider(uniphy->dev->of_node,
				     of_clk_hw_onecell_get,
				     uniphy->clk_data);
	if (ret)
		return dev_err_probe(uniphy->dev, ret,
				     "fail to register clock provider\n");

	return 0;
}

static int qcom_eth_uniphy_calibrate(struct phy *phy)
{
	struct qcom_eth_uniphy *uniphy = phy_get_drvdata(phy);
	dev_info(uniphy->dev, "calibrating\n");
	return uniphy_calibrate(uniphy);
}

static const struct phy_ops qcom_eth_uniphy_ops = {
	.calibrate = qcom_eth_uniphy_calibrate,
	.owner = THIS_MODULE,
};

static int qcom_eth_uniphy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct qcom_eth_uniphy *uniphy;
	struct phy *phy;
	struct phy_provider *phy_provider;
	int ret;
	printk(KERN_INFO "init %s\n", __func__);
	ret = cmn_init(pdev);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to init cmn block\n");

	uniphy = devm_kzalloc(dev, sizeof(*uniphy), GFP_KERNEL);
	if (!uniphy)
		return dev_err_probe(dev, -ENOMEM,
				     "failed to allocate priv\n");

	uniphy->dev = dev;
	uniphy->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(uniphy->base))
		return dev_err_probe(dev, PTR_ERR(uniphy->base),
				     "failed to ioremap base\n");

	uniphy->num_clks = devm_clk_bulk_get_all(uniphy->dev, &uniphy->clks);
	if (uniphy->num_clks < 0)
		return dev_err_probe(uniphy->dev, uniphy->num_clks,
				     "failed to acquire clocks\n");

	ret = clk_bulk_prepare_enable(uniphy->num_clks, uniphy->clks);
	if (ret)
		return dev_err_probe(uniphy->dev, ret,
				     "failed to enable clocks\n");

	uniphy->rst = devm_reset_control_array_get_exclusive(uniphy->dev);
	if (IS_ERR_OR_NULL(uniphy->rst))
		return dev_err_probe(uniphy->dev, PTR_ERR(uniphy->rst),
				     "failed to acquire reset\n");

	ret = reset_control_reset(uniphy->rst);
	if (ret)
		return dev_err_probe(uniphy->dev, ret,
				     "failed to reset\n");

	ret = uniphy_clkout_init(uniphy);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to init clkout\n");

	ret = uniphy_mode_set(uniphy);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to set mode\n");

	ret = uniphy_calibrate(uniphy);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to calibrate\n");

	ret = uniphy_clk_register(uniphy);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to register clocks\n");

	phy = devm_phy_create(dev, dev->of_node, &qcom_eth_uniphy_ops);
	if (IS_ERR(phy))
		return dev_err_probe(dev, PTR_ERR(phy),
				     "failed to register phy\n");

	phy_set_drvdata(phy, uniphy);

	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(phy_provider))
		return dev_err_probe(dev, PTR_ERR(phy_provider),
				     "failed to register phy provider\n");


	return 0;
}

static const struct of_device_id qcom_eth_uniphy_of_match[] = {
	{ .compatible = "qcom,ipq5018-eth-uniphy" },
	{}
};
MODULE_DEVICE_TABLE(of, qcom_eth_uniphy_of_match);

static struct platform_driver qcom_eth_uniphy_driver = {
	.probe	= qcom_eth_uniphy_probe,
	.driver	= {
		.name		= "qcom-eth-uniphy",
		.of_match_table	= qcom_eth_uniphy_of_match,
	},
};
module_platform_driver(qcom_eth_uniphy_driver);

MODULE_DESCRIPTION("Qualcomm ethernet uniphy driver");
MODULE_AUTHOR("Ziyang Huang <hzyitc@outlook.com>");