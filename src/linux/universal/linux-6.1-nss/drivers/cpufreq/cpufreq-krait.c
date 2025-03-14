/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * The OPP code in function krait_set_target() is reused from
 * drivers/cpufreq/omap-cpufreq.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpu_cooling.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pm_opp.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/fab_scaling.h>

static unsigned int transition_latency;
static unsigned int voltage_tolerance; /* in percentage */

static struct device *cpu_dev;
static DEFINE_PER_CPU(struct clk *, krait_cpu_clks);
static DEFINE_PER_CPU(struct regulator *, krait_supply_core);
static struct cpufreq_frequency_table *freq_table;
static struct thermal_cooling_device *cdev;

struct cache_points {
	unsigned long cache_freq;
	unsigned int cache_volt;
	unsigned long cpu_freq;
};

static struct regulator *krait_l2_reg;
static struct clk *krait_l2_clk;
static struct cache_points *krait_l2_points;
static int nr_krait_l2_points;

static int krait_parse_cache_points(struct device *dev,
		struct device_node *of_node)
{
	const struct property *prop;
	const __be32 *val;
	int nr, i;

	prop = of_find_property(of_node, "cache-points-kHz", NULL);
	if (!prop)
		return -ENODEV;
	if (!prop->value)
		return -ENODATA;

	/*
	 * Each OPP is a set of tuples consisting of frequency and
	 * cpu-frequency like <freq-kHz volt-uV freq-kHz>.
	 */
	nr = prop->length / sizeof(u32);
	if (nr % 3) {
		dev_err(dev, "%s: Invalid cache points\n", __func__);
		return -EINVAL;
	}
	nr /= 3;

	krait_l2_points = devm_kcalloc(dev, nr, sizeof(*krait_l2_points),
				       GFP_KERNEL);
	if (!krait_l2_points)
		return -ENOMEM;
	nr_krait_l2_points = nr;

	for (i = 0, val = prop->value; i < nr; i++) {
		unsigned long cache_freq = be32_to_cpup(val++) * 1000;
		unsigned int cache_volt = be32_to_cpup(val++);
		unsigned long cpu_freq = be32_to_cpup(val++) * 1000;

		krait_l2_points[i].cache_freq = cache_freq;
		krait_l2_points[i].cache_volt = cache_volt;
		krait_l2_points[i].cpu_freq = cpu_freq;
	}

	return 0;
}

static int krait_set_target(struct cpufreq_policy *policy, unsigned int index)
{
	struct dev_pm_opp *opp;
	unsigned long volt = 0, volt_old = 0, tol = 0;
	unsigned long freq, max_cpu_freq = 0;
	unsigned int old_freq, new_freq;
	long freq_Hz, freq_exact;
	int ret, i;
	struct clk *cpu_clk;
	struct regulator *core;
	unsigned int cpu;

	cpu_clk = per_cpu(krait_cpu_clks, policy->cpu);

	freq_Hz = clk_round_rate(cpu_clk, policy->freq_table[index].frequency * 1000);
	if (freq_Hz <= 0)
		freq_Hz = policy->freq_table[index].frequency * 1000;

	freq_exact = freq_Hz;
	new_freq = freq_Hz / 1000;
	old_freq = clk_get_rate(cpu_clk) / 1000;

	core = per_cpu(krait_supply_core, policy->cpu);

	rcu_read_lock();
	opp = dev_pm_opp_find_freq_ceil(cpu_dev, &freq_Hz);
	if (IS_ERR(opp)) {
		rcu_read_unlock();
		pr_err("failed to find OPP for %ld\n", freq_Hz);
		return PTR_ERR(opp);
	}
	volt = dev_pm_opp_get_voltage(opp);
	rcu_read_unlock();
	tol = volt * voltage_tolerance / 100;
	volt_old = regulator_get_voltage(core);

	pr_debug("%u MHz, %ld mV --> %u MHz, %ld mV\n",
		 old_freq / 1000, volt_old ? volt_old / 1000 : -1,
		 new_freq / 1000, volt ? volt / 1000 : -1);

	/* scaling up?  scale voltage before frequency */
	if (new_freq > old_freq) {
		ret = regulator_set_voltage_tol(core, volt, tol);
		if (ret) {
			pr_err("failed to scale voltage up: %d\n", ret);
			return ret;
		}
	}

	ret = clk_set_rate(cpu_clk, freq_exact);
	if (ret) {
		pr_err("failed to set clock rate: %d\n", ret);
		return ret;
	}

	/* scaling down?  scale voltage after frequency */
	if (new_freq < old_freq) {
		ret = regulator_set_voltage_tol(core, volt, tol);
		if (ret) {
			pr_err("failed to scale voltage down: %d\n", ret);
			clk_set_rate(cpu_clk, old_freq * 1000);
		}
	}

	for_each_possible_cpu(cpu) {
		freq = clk_get_rate(per_cpu(krait_cpu_clks, cpu));
		max_cpu_freq = max(max_cpu_freq, freq);
	}

	for (i = 0; i < nr_krait_l2_points; i++) {
		if (max_cpu_freq >= krait_l2_points[i].cpu_freq) {
			if (krait_l2_reg) {
				ret = regulator_set_voltage_tol(krait_l2_reg,
						krait_l2_points[i].cache_volt,
						tol);
				if (ret) {
					pr_err("failed to scale l2 voltage: %d\n",
						ret);
				}
			}
			ret = clk_set_rate(krait_l2_clk,
					krait_l2_points[i].cache_freq);
			if (ret)
				pr_err("failed to scale l2 clk: %d\n", ret);
			break;
		}

	}

	scale_fabrics();

	return ret;
}

static int krait_cpufreq_init(struct cpufreq_policy *policy)
{
	int ret;

	policy->clk = per_cpu(krait_cpu_clks, policy->cpu);
	policy->freq_table = freq_table;
	ret = cpufreq_table_validate_and_sort(policy);
	if (ret) {
		pr_err("%s: invalid frequency table: %d\n", __func__, ret);
		return ret;
	}

	policy->cpuinfo.transition_latency = transition_latency;

	return 0;
}

static void krait_cpufreq_ready(struct cpufreq_policy *policy)
{
	struct device_node *np = of_get_cpu_node(policy->cpu, NULL);

	if (of_find_property(np, "#cooling-cells", NULL)) {
		policy->cdev = cdev = of_cpufreq_cooling_register(policy);
		
		if (PTR_ERR(policy->cdev) != -ENOSYS) {
			pr_err("cpu%d is not running as cooling device: %ld\n",
					policy->cpu, PTR_ERR(policy->cdev));

			policy->cdev = NULL;
		}
	}

	of_node_put(np);
}

static struct cpufreq_driver krait_cpufreq_driver = {
	.verify = cpufreq_generic_frequency_table_verify,
	.target_index = krait_set_target,
	.get = cpufreq_generic_get,
	.ready = krait_cpufreq_ready,
	.init = krait_cpufreq_init,
	.name = "generic_krait",
	.attr = cpufreq_generic_attr,
};

static int krait_cpufreq_probe(struct platform_device *pdev)
{
	struct device_node *np, *cache;
	int ret, i;
	unsigned int cpu;
	struct device *dev;
	struct clk *clk;
	struct regulator *core;
	unsigned long freq_Hz, freq, max_cpu_freq = 0;
	struct dev_pm_opp *opp;
	unsigned long volt, tol;
	struct fab_scaling_info fab_data;

	cpu_dev = get_cpu_device(0);
	if (!cpu_dev) {
		pr_err("failed to get krait device\n");
		return -ENODEV;
	}

	np = of_node_get(cpu_dev->of_node);
	if (!np) {
		pr_err("failed to find krait node\n");
		return -ENOENT;
	}

	ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &freq_table);
	if (ret) {
		pr_err("failed to init cpufreq table: %d\n", ret);
		goto out_put_node;
	}

	of_property_read_u32(np, "voltage-tolerance", &voltage_tolerance);

	if (of_property_read_u32(np, "clock-latency", &transition_latency))
		transition_latency = CPUFREQ_ETERNAL;

	cache = of_find_next_cache_node(np);
	if (cache) {
		struct device_node *vdd;

		vdd = of_parse_phandle(cache, "vdd_dig-supply", 0);
		if (vdd) {
			krait_l2_reg = regulator_get(NULL, vdd->name);
			if (IS_ERR(krait_l2_reg)) {
				pr_warn("failed to get l2 vdd_dig supply\n");
				krait_l2_reg = NULL;
			}
			of_node_put(vdd);
		}

		krait_l2_clk = of_clk_get(cache, 0);
		if (!IS_ERR(krait_l2_clk)) {
			ret = krait_parse_cache_points(&pdev->dev, cache);
			if (ret)
				clk_put(krait_l2_clk);
		}
		if (IS_ERR(krait_l2_clk) || ret)
			krait_l2_clk = NULL;
	}

	for_each_possible_cpu(cpu) {
		dev = get_cpu_device(cpu);
		if (!dev) {
			pr_err("failed to get krait device\n");
			ret = -ENOENT;
			goto out_free_table;
		}
		per_cpu(krait_cpu_clks, cpu) = clk = devm_clk_get(dev, NULL);
		if (IS_ERR(clk)) {
			ret = PTR_ERR(clk);
			goto out_free_table;
		}
		core = devm_regulator_get(dev, "core");
		if (IS_ERR(core)) {
			pr_debug("failed to get core regulator\n");
			ret = PTR_ERR(core);
			goto out_free_table;
		}
		per_cpu(krait_supply_core, cpu) = core;

		freq = freq_Hz = clk_get_rate(clk);

		rcu_read_lock();
		opp = dev_pm_opp_find_freq_ceil(cpu_dev, &freq_Hz);
		if (IS_ERR(opp)) {
			rcu_read_unlock();
			pr_err("failed to find OPP for %ld\n", freq_Hz);
			ret = PTR_ERR(opp);
			goto out_free_table;
		}
		volt = dev_pm_opp_get_voltage(opp);
		rcu_read_unlock();

		tol = volt * voltage_tolerance / 100;
		ret = regulator_set_voltage_tol(core, volt, tol);
		if (ret) {
			pr_err("failed to scale voltage up: %d\n", ret);
			goto out_free_table;
		}
		ret = regulator_enable(core);
		if (ret) {
			pr_err("failed to enable regulator: %d\n", ret);
			goto out_free_table;
		}
		max_cpu_freq = max(max_cpu_freq, freq);

		if (!of_property_read_u32(np, "cpu_fab_threshold",
						&fab_data.idle_freq)) {
			fab_data.clk = clk;
			fab_scaling_register(&fab_data);
		}
	}

	for (i = 0; i < nr_krait_l2_points; i++) {
		if (max_cpu_freq >= krait_l2_points[i].cpu_freq) {
			if (krait_l2_reg) {
				ret = regulator_set_voltage_tol(krait_l2_reg,
						krait_l2_points[i].cache_volt,
						tol);
				if (ret)
					pr_err("failed to scale l2 voltage: %d\n",
							ret);
				ret = regulator_enable(krait_l2_reg);
				if (ret)
					pr_err("failed to enable l2 voltage: %d\n",
							ret);
			}
			break;
		}

	}

	ret = cpufreq_register_driver(&krait_cpufreq_driver);
	if (ret) {
		pr_err("failed register driver: %d\n", ret);
		goto out_free_table;
	}
	of_node_put(np);

	/*
	 * For now, just loading the cooling device;
	 * thermal DT code takes care of matching them.
	 */
/*	for_each_possible_cpu(cpu) {
		dev = get_cpu_device(cpu);
		if (!dev) {
			pr_err("failed to get krait device\n");
			ret = -ENOENT;
			goto out_free_table;
		}
		np = of_node_get(dev->of_node);
		if (of_find_property(np, "#cooling-cells", NULL)) {
			cdev = of_cpufreq_cooling_register(np, cpumask_of(cpu));
			if (IS_ERR(cdev))
				pr_err("running cpufreq without cooling device: %ld\n",
						PTR_ERR(cdev));
		}
		of_node_put(np);
	}*/

	return 0;

out_free_table:
	regulator_put(krait_l2_reg);
	clk_put(krait_l2_clk);
	dev_pm_opp_free_cpufreq_table(cpu_dev, &freq_table);
out_put_node:
	of_node_put(np);
	return ret;
}


static int krait_cpufreq_remove(struct platform_device *pdev)
{
	unsigned int cpu;
	struct clk *clk;

	for_each_possible_cpu(cpu) {
		clk = per_cpu(krait_cpu_clks, cpu);
		fab_scaling_unregister(clk);
	}
	if (cdev)
		cpufreq_cooling_unregister(cdev);
	cpufreq_unregister_driver(&krait_cpufreq_driver);
	dev_pm_opp_free_cpufreq_table(cpu_dev, &freq_table);
	clk_put(krait_l2_clk);
	regulator_put(krait_l2_reg);

	return 0;
}

static struct platform_driver krait_cpufreq_platdrv = {
	.driver = {
		.name	= "cpufreq-krait",
		.owner	= THIS_MODULE,
	},
	.probe		= krait_cpufreq_probe,
	.remove		= krait_cpufreq_remove,
};
module_platform_driver(krait_cpufreq_platdrv);

MODULE_DESCRIPTION("Krait CPUfreq driver");
MODULE_LICENSE("GPL v2");
