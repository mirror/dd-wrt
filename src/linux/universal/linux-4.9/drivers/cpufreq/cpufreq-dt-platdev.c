/*
 * Copyright (C) 2016 Linaro.
 * Viresh Kumar <viresh.kumar@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/of.h>
#include <linux/platform_device.h>

static const struct of_device_id machines[] __initconst = {
	{ .compatible = "qcom,ipq4019" },
	{ .compatible = "qcom,ipq807x" },
	{ .compatible = "qcom,ipq8064" },

	{ /* sentinel */ }
};

static int __init cpufreq_dt_platdev_init(void)
{
	struct device_node *np = of_find_node_by_path("/");
	const struct of_device_id *match;

	if (!np)
		return -ENODEV;

	match = of_match_node(machines, np);
	of_node_put(np);
	if (!match)
		return -ENODEV;

	return PTR_ERR_OR_ZERO(platform_device_register_simple("cpufreq-dt", -1,
							       NULL, 0));
}
device_initcall(cpufreq_dt_platdev_init);
