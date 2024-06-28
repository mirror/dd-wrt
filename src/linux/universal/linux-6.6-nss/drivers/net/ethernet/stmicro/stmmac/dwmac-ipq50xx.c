#include <linux/clk.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/stmmac.h>

#include "stmmac_platform.h"

enum {
	IPQ50XX_GMAC_CLK_SYS,
	IPQ50XX_GMAC_CLK_CFG,
	IPQ50XX_GMAC_CLK_AHB,
	IPQ50XX_GMAC_CLK_AXI,
	IPQ50XX_GMAC_CLK_RX,
	IPQ50XX_GMAC_CLK_TX,
	IPQ50XX_GMAC_CLK_PTP,
};

static const struct clk_bulk_data ipq50xx_gmac_clks[] = {
	[IPQ50XX_GMAC_CLK_SYS]	= { .id = "sys" },
	[IPQ50XX_GMAC_CLK_CFG]	= { .id = "cfg" },
	[IPQ50XX_GMAC_CLK_AHB]	= { .id = "ahb" },
	[IPQ50XX_GMAC_CLK_AXI]	= { .id = "axi" },
	[IPQ50XX_GMAC_CLK_RX]	= { .id = "rx" },
	[IPQ50XX_GMAC_CLK_TX]	= { .id = "tx" },
	[IPQ50XX_GMAC_CLK_PTP]	= { .id = "ptp" },
};

struct ipq50xx_gmac {
	struct device *dev;
	struct clk_bulk_data clks[ARRAY_SIZE(ipq50xx_gmac_clks)];
	struct reset_control *rst;
	struct phy *uniphy;
};

static int ipq50xx_gmac_powerup(struct net_device *ndev, void *priv)
{
	struct ipq50xx_gmac *gmac = priv;
	int ret;

	ret = phy_init(gmac->uniphy);
	if (ret)
		return ret;

	ret = phy_power_on(gmac->uniphy);
	if (ret)
		return ret;

	return 0;
}

static void ipq50xx_gmac_fix_speed(void *priv, unsigned int speed, unsigned int mode)
{
	struct ipq50xx_gmac *gmac = priv;
	unsigned long rate;

	switch(speed) {
		case SPEED_10:
			rate = 2500000;
			break;
		case SPEED_100:
			rate = 25000000;
			break;
		case SPEED_1000:
			rate = 125000000;
			break;
		case SPEED_2500:
			rate = 312500000;
			break;
		default:
			dev_err(gmac->dev, "Unsupported speed: %d\n", speed);
			rate = 125000000;
			break;
	}

	clk_set_rate(gmac->clks[IPQ50XX_GMAC_CLK_RX].clk, rate);
	clk_set_rate(gmac->clks[IPQ50XX_GMAC_CLK_TX].clk, rate);
	phy_calibrate(gmac->uniphy);
}

static int ipq50xx_gmac_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct stmmac_resources stmmac_res;
	struct plat_stmmacenet_data *plat_dat;
	struct ipq50xx_gmac *gmac;
	int ret;

	ret = stmmac_get_platform_resources(pdev, &stmmac_res);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to get stmmac platform resources\n");

	plat_dat = stmmac_probe_config_dt(pdev, stmmac_res.mac);
	if (IS_ERR_OR_NULL(plat_dat))
		return dev_err_probe(dev, PTR_ERR(plat_dat),
				     "failed to parse stmmac dt parameters\n");

	gmac = devm_kzalloc(dev, sizeof(*gmac), GFP_KERNEL);
	if (!gmac)
		return dev_err_probe(dev, -ENOMEM,
				     "failed to allocate priv\n");

	gmac->dev = dev;

	memcpy(gmac->clks, ipq50xx_gmac_clks, sizeof(gmac->clks));
	ret = devm_clk_bulk_get_optional(dev, ARRAY_SIZE(gmac->clks), gmac->clks);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to acquire clocks\n");

	ret = clk_bulk_prepare_enable(ARRAY_SIZE(gmac->clks), gmac->clks);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to enable clocks\n");

	gmac->rst = devm_reset_control_array_get_exclusive(dev);
	if (IS_ERR_OR_NULL(gmac->rst))
		return dev_err_probe(dev, PTR_ERR(gmac->rst),
				     "failed to acquire reset\n");

	ret = reset_control_reset(gmac->rst);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to reset\n");

	gmac->uniphy = devm_phy_optional_get(dev, "uniphy");
	if (IS_ERR(gmac->uniphy))
		return dev_err_probe(dev, PTR_ERR(gmac->uniphy),
				     "failed to acquire uniphy\n");

	plat_dat->bsp_priv = gmac;
	plat_dat->serdes_powerup = ipq50xx_gmac_powerup;
	plat_dat->fix_mac_speed = ipq50xx_gmac_fix_speed;

	return stmmac_dvr_probe(dev, plat_dat, &stmmac_res);
}

static const struct of_device_id ipq50xx_gmac_dwmac_match[] = {
	{ .compatible = "qcom,ipq50xx-gmac" },
	{ }
};
MODULE_DEVICE_TABLE(of, ipq50xx_gmac_dwmac_match);

static struct platform_driver ipq50xx_gmac_dwmac_driver = {
	.probe = ipq50xx_gmac_probe,
	.driver = {
		.name		= "ipq50xx-gmac-dwmac",
		.of_match_table	= ipq50xx_gmac_dwmac_match,
	},
};
module_platform_driver(ipq50xx_gmac_dwmac_driver);

MODULE_DESCRIPTION("Qualcomm IPQ50xx DWMAC driver");
MODULE_AUTHOR("Ziyang Huang <hzyitc@outlook.com>");
