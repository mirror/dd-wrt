// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/devfreq.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/pm_opp.h>

#include "governor.h"

struct ipq806x_fab_data {
	struct clk *fab_clk;
	struct clk *ddr_clk;
};

static int ipq806x_fab_get_cur_freq(struct device *dev, unsigned long *freq)
{
	struct ipq806x_fab_data *data = dev_get_drvdata(dev);

	*freq = clk_get_rate(data->fab_clk);

	return 0;
};

static int ipq806x_fab_target(struct device *dev, unsigned long *freq,
			      u32 flags)
{
	struct ipq806x_fab_data *data = dev_get_drvdata(dev);
	struct dev_pm_opp *opp;
	int ret;

	opp = dev_pm_opp_find_freq_ceil(dev, freq);
	if (unlikely(IS_ERR(opp)))
		return PTR_ERR(opp);

	dev_pm_opp_put(opp);

	ret = clk_set_rate(data->fab_clk, *freq);
	if (ret)
		return ret;

	return clk_set_rate(data->ddr_clk, *freq);
};

static int ipq806x_fab_get_dev_status(struct device *dev,
				      struct devfreq_dev_status *stat)
{
	struct ipq806x_fab_data *data = dev_get_drvdata(dev);

	stat->busy_time = 0;
	stat->total_time = 0;
	stat->current_frequency = clk_get_rate(data->fab_clk);

	return 0;
};

static struct devfreq_dev_profile ipq806x_fab_devfreq_profile = {
	.target = ipq806x_fab_target,
	.get_dev_status = ipq806x_fab_get_dev_status,
	.get_cur_freq = ipq806x_fab_get_cur_freq
};

static struct devfreq_passive_data devfreq_gov_data = {
	.parent_type = CPUFREQ_PARENT_DEV,
};

static int ipq806x_fab_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ipq806x_fab_data *data;
	struct devfreq *devfreq;
	struct clk *clk;
	int ret;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	clk = devm_clk_get(dev, "apps-fab-clk");
	if (IS_ERR(clk)) {
		dev_err_probe(dev, PTR_ERR(clk), "failed to get apps fab clk\n");
		return PTR_ERR(clk);
	}

	clk_prepare_enable(clk);
	data->fab_clk = clk;

	clk = devm_clk_get(dev, "ddr-fab-clk");
	if (IS_ERR(clk)) {
		dev_err_probe(dev, PTR_ERR(clk), "failed to get ddr fab clk\n");
		goto err_ddr;
	}

	clk_prepare_enable(clk);
	data->ddr_clk = clk;

	ret = dev_pm_opp_of_add_table(dev);
	if (ret) {
		dev_err(dev, "failed to parse fab freq thresholds\n");
		return ret;
	}

	dev_set_drvdata(dev, data);

	devfreq = devm_devfreq_add_device(&pdev->dev, &ipq806x_fab_devfreq_profile,
					  DEVFREQ_GOV_PASSIVE, &devfreq_gov_data);
	if (IS_ERR(devfreq))
		dev_pm_opp_remove_table(dev);

	return PTR_ERR_OR_ZERO(devfreq);

err_ddr:
	clk_unprepare(data->fab_clk);
	clk_put(data->fab_clk);
	return PTR_ERR(clk);
};

static int ipq806x_fab_remove(struct platform_device *pdev)
{
	struct ipq806x_fab_data *data = dev_get_drvdata(&pdev->dev);

	clk_unprepare(data->fab_clk);
	clk_put(data->fab_clk);

	clk_unprepare(data->ddr_clk);
	clk_put(data->ddr_clk);

	dev_pm_opp_remove_table(&pdev->dev);

	return 0;
};

static const struct of_device_id ipq806x_fab_match_table[] = {
	{ .compatible = "qcom,fab-scaling" },
	{}
};

static struct platform_driver ipq806x_fab_driver = {
	.probe		= ipq806x_fab_probe,
	.remove		= ipq806x_fab_remove,
	.driver		= {
		.name   = "ipq806x-fab-scaling",
		.of_match_table = ipq806x_fab_match_table,
	},
};
module_platform_driver(ipq806x_fab_driver);

MODULE_DESCRIPTION("ipq806x Fab Scaling driver");
MODULE_AUTHOR("Christian Marangi <ansuelsmth@gmail.com>");
MODULE_LICENSE("GPL v2");
