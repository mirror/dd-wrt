/*
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regulator/nss-volt-ipq806x.h>

struct nss_data {
	struct regulator *nss_reg;
	u32 nss_core_vdd_nominal;
	u32 nss_core_vdd_high;
	u32 nss_core_threshold_freq;
};

static struct nss_data *data;

int nss_ramp_voltage(unsigned long rate, bool ramp_up)
{
	int ret;
	int curr_uV, uV;
	struct regulator *reg;

	if (!data) {
		pr_err("NSS core regulator not init.\n");
		return -ENODEV;
	}

	reg = data->nss_reg;

	if (!reg) {
		pr_err("NSS core regulator not found.\n");
		return -EINVAL;
	}

	if (rate >= data->nss_core_threshold_freq)
		uV = data->nss_core_vdd_high;
	else 
		uV = data->nss_core_vdd_nominal;

	curr_uV = regulator_get_voltage(reg);

	if (ramp_up) {
		if (uV <= curr_uV)
			return 0;
	} else {
		if (uV >= curr_uV)
			return 0;
	}

	ret = regulator_set_voltage(reg, uV, data->nss_core_vdd_high);
	if (ret)
		pr_err("NSS volt scaling failed (%d)\n", uV);

	return ret;
}

static const struct of_device_id nss_ipq806x_match_table[] = {
	{ .compatible = "qcom,nss-common" },
	{}
};

static int nss_volt_ipq806x_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	int ret;

	if (!np)
		return -ENODEV;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->nss_reg = devm_regulator_get(&pdev->dev, "nss_core");
	ret = PTR_ERR_OR_ZERO(data->nss_reg);
	if (ret) {
		if (ret == -EPROBE_DEFER)
			dev_dbg(&pdev->dev,
				"nss_core regulator not ready, retry\n");
		else
			dev_err(&pdev->dev, "no regulator for nss_core: %d\n",
				ret);

		return ret;
	}

	if (of_property_read_u32(np, "nss_core_vdd_nominal",
				 &data->nss_core_vdd_nominal)) {
		pr_warn("NSS core vdd nominal not found. Using defaults...\n");
		data->nss_core_vdd_nominal = 1100000;
	}

	if (of_property_read_u32(np, "nss_core_vdd_high",
				 &data->nss_core_vdd_high)) {
		pr_warn("NSS core vdd high not found. Using defaults...\n");
		data->nss_core_vdd_high = 1150000;
	}

	if (of_property_read_u32(np, "nss_core_threshold_freq",
				 &data->nss_core_threshold_freq)) {
		pr_warn("NSS core thres freq not found. Using defaults...\n");
		data->nss_core_threshold_freq = 733000000;
	}

	platform_set_drvdata(pdev, data);

	return 0;
}

static struct platform_driver nss_ipq806x_driver = {
	.probe          = nss_volt_ipq806x_probe,
	.driver         = {
		.name   = "nss-volt-ipq806x",
		.owner  = THIS_MODULE,
		.of_match_table = nss_ipq806x_match_table,
	},
};

static int __init nss_ipq806x_init(void)
{
	return platform_driver_register(&nss_ipq806x_driver);
}
late_initcall(nss_ipq806x_init);

static void __exit nss_ipq806x_exit(void)
{
	platform_driver_unregister(&nss_ipq806x_driver);
}
module_exit(nss_ipq806x_exit);

