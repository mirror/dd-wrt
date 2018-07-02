/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>

#include <linux/reset-controller.h>
#include <dt-bindings/reset/qcom,gcc-ipq40xx.h>
#include <dt-bindings/clock/qcom,gcc-dummy.h>

#include "common.h"
#include "clk-regmap.h"
#include "reset.h"

struct clk *clk;

static int clk_dummy_is_enabled(struct clk_hw *hw)
{
	return 1;
};

static int clk_dummy_enable(struct clk_hw *hw)
{
	return 0;
};

static void clk_dummy_disable(struct clk_hw *hw)
{
	return;
};

static u8 clk_dummy_get_parent(struct clk_hw *hw)
{
	return 0;
};

static int clk_dummy_set_parent(struct clk_hw *hw, u8 index)
{
	return 0;
};

static int clk_dummy_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	return 0;
};

static long clk_dummy_determine_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long *p_rate, struct clk **p)
{
	return rate;
};

static unsigned long clk_dummy_recalc_rate(struct clk_hw *hw,
					   unsigned long parent_rate)
{
	return parent_rate;
};

const struct clk_ops clk_dummy_ops = {
	.is_enabled = clk_dummy_is_enabled,
	.enable = clk_dummy_enable,
	.disable = clk_dummy_disable,
	.get_parent = clk_dummy_get_parent,
	.set_parent = clk_dummy_set_parent,
	.set_rate = clk_dummy_set_rate,
	.recalc_rate = clk_dummy_recalc_rate,
	.determine_rate = clk_dummy_determine_rate,
};

static struct clk_regmap dummy = {
	.hw.init = &(struct clk_init_data){
		.name = "dummy_clk_src",
		.parent_names = (const char *[]){ "xo"},
		.num_parents = 1,
		.ops = &clk_dummy_ops,
	},
};

static const struct of_device_id gcc_dummy_match_table[] = {
	{ .compatible = "qcom,gcc-dummy" },
	{ }
};
MODULE_DEVICE_TABLE(of, gcc_dummy_match_table);

static struct clk_regmap *gcc_ipq40xx_clks[] = {
	[GCC_DUMMY_CLK] = &dummy,
};

static const struct qcom_reset_map gcc_ipq40xx_resets[] = {
	[WIFI0_CPU_INIT_RESET] = { 0x1f008, 5 },
	[WIFI0_RADIO_SRIF_RESET] = { 0x1f008, 4 },
	[WIFI0_RADIO_WARM_RESET] = { 0x1f008, 3 },
	[WIFI0_RADIO_COLD_RESET] = { 0x1f008, 2 },
	[WIFI0_CORE_WARM_RESET] = { 0x1f008, 1 },
	[WIFI0_CORE_COLD_RESET] = { 0x1f008, 0 },
	[WIFI1_CPU_INIT_RESET] = { 0x20008, 5 },
	[WIFI1_RADIO_SRIF_RESET] = { 0x20008, 4 },
	[WIFI1_RADIO_WARM_RESET] = { 0x20008, 3 },
	[WIFI1_RADIO_COLD_RESET] = { 0x20008, 2 },
	[WIFI1_CORE_WARM_RESET] = { 0x20008, 1 },
	[WIFI1_CORE_COLD_RESET] = { 0x20008, 0 },
	[USB3_UNIPHY_PHY_ARES] = { 0x1e038, 5 },
	[USB3_HSPHY_POR_ARES] = { 0x1e038, 4 },
	[USB3_HSPHY_S_ARES] = { 0x1e038, 2 },
	[USB2_HSPHY_POR_ARES] = { 0x1e01c, 4 },
	[USB2_HSPHY_S_ARES] = { 0x1e01c, 2 },
	[PCIE_PHY_AHB_ARES] = { 0x1d010, 11 },
	[PCIE_AHB_ARES] = { 0x1d010, 10 },
	[PCIE_PWR_ARES] = { 0x1d010, 9 },
	[PCIE_PIPE_STICKY_ARES] = { 0x1d010, 8 },
	[PCIE_AXI_M_STICKY_ARES] = { 0x1d010, 7 },
	[PCIE_PHY_ARES] = { 0x1d010, 6 },
	[PCIE_PARF_XPU_ARES] = { 0x1d010, 5 },
	[PCIE_AXI_S_XPU_ARES] = { 0x1d010, 4 },
	[PCIE_AXI_M_VMIDMT_ARES] = { 0x1d010, 3 },
	[PCIE_PIPE_ARES] = { 0x1d010, 2 },
	[PCIE_AXI_S_ARES] = { 0x1d010, 1 },
	[PCIE_AXI_M_ARES] = { 0x1d010, 0 },
	[ESS_RESET] = { 0x12008 },
	[AUDIO_BLK_ARES] = { 0x1B008 },
};

static const struct regmap_config gcc_ipq40xx_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0xffffc,
	.fast_io	= true,
};

static const struct qcom_cc_desc gcc_ipq40xx_desc = {
	.config = &gcc_ipq40xx_regmap_config,
	.clks = gcc_ipq40xx_clks,
	.num_clks = ARRAY_SIZE(gcc_ipq40xx_clks),
	.resets = gcc_ipq40xx_resets,
	.num_resets = ARRAY_SIZE(gcc_ipq40xx_resets),
};

static int gcc_dummy_probe(struct platform_device *pdev)
{
	int ret;

	clk = clk_register_fixed_rate(&pdev->dev, "xo", NULL, CLK_IS_ROOT,
				      19200000);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	ret = qcom_cc_probe(pdev, &gcc_ipq40xx_desc);

	dev_dbg(&pdev->dev, "Registered dummy clock provider\n");
	return ret;
}

static int gcc_dummy_remove(struct platform_device *pdev)
{
	qcom_cc_remove(pdev);
	return 0;
}

static struct platform_driver gcc_dummy_driver = {
	.probe		= gcc_dummy_probe,
	.remove		= gcc_dummy_remove,
	.driver		= {
		.name	= "gcc-dummy",
		.owner	= THIS_MODULE,
		.of_match_table = gcc_dummy_match_table,
	},
};

static int __init gcc_dummy_init(void)
{
	return platform_driver_register(&gcc_dummy_driver);
}
core_initcall(gcc_dummy_init);

static void __exit gcc_dummy_exit(void)
{
	platform_driver_unregister(&gcc_dummy_driver);
}
module_exit(gcc_dummy_exit);

MODULE_DESCRIPTION("QCOM GCC Dummy Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:gcc-dummy");
