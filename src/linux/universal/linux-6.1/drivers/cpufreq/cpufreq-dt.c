// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2014 Linaro.
 * Viresh Kumar <viresh.kumar@linaro.org>
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pm_opp.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/thermal.h>
#include <linux/fab_scaling.h>

#include "cpufreq-dt.h"

static DEFINE_PER_CPU(struct clk *, cpu_cores_clks);

struct private_data {
	struct list_head node;

	cpumask_var_t cpus;
	struct device *cpu_dev;
	struct cpufreq_frequency_table *freq_table;
	bool have_static_opps;
	int opp_token;
	struct mutex lock;

	struct notifier_block opp_nb;
	unsigned long opp_freq;
};

static LIST_HEAD(priv_list);

static struct freq_attr *cpufreq_dt_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,   /* Extra space for boost-attr if required */
	NULL,
};

static struct private_data *cpufreq_dt_find_data(int cpu)
{
	struct private_data *priv;

	list_for_each_entry(priv, &priv_list, node) {
		if (cpumask_test_cpu(cpu, priv->cpus))
			return priv;
	}

	return NULL;
}


static int set_target(struct cpufreq_policy *policy, unsigned int index)
{
	struct private_data *priv = policy->driver_data;
	unsigned long freq = policy->freq_table[index].frequency;
	struct clk *l2_clk = policy->l2_clk;
	struct regulator *l2_regulator = policy->l2_regulator;
	unsigned long l2_freq, target_l2_freq;
	unsigned long l2_vol, target_l2_vol;
	unsigned long target_freq;
	int ret;
	
	mutex_lock(&priv->lock);
	ret = dev_pm_opp_set_rate(priv->cpu_dev, freq * 1000);

	if (!ret) {
		if (policy->l2_rate_set) {
			static unsigned long krait_l2[CONFIG_NR_CPUS] = { };
			int cpu, l2_index, tol = 0;

			target_freq = freq * 1000;

			krait_l2[policy->cpu] = target_freq;
			for_each_present_cpu(cpu)
				target_freq = max(target_freq, krait_l2[cpu]);

			for (l2_index = 2; l2_index >= 0; l2_index--)
				if (target_freq >= policy->l2_cpufreq[l2_index])
					break;

			l2_freq = clk_get_rate(l2_clk);
			target_l2_freq = policy->l2_rate[l2_index];

			if (l2_freq != target_l2_freq) {

				/*
				 * Set to idle bin if switching from normal to high bin 
				 * or vice versa
				 */
				if ( (l2_index == 2 && l2_freq == policy->l2_rate[1]) ||
					 (l2_index == 1 && l2_freq == policy->l2_rate[2]) ) {
					ret = clk_set_rate(l2_clk, policy->l2_rate[0]);
					if (ret)
						goto exit;
				}
				/* scale l2 with the core */
				ret = clk_set_rate(l2_clk, target_l2_freq);
				if (ret)
					goto exit;

				if (policy->l2_volt_set) {

					l2_vol = regulator_get_voltage(l2_regulator);
					target_l2_vol = policy->l2_volt[l2_index];

					if (l2_vol != target_l2_vol) {
						tol = target_l2_vol * policy->l2_volt_tol / 100;
						ret = regulator_set_voltage_tol(l2_regulator,target_l2_vol,tol);
						if (ret)
							goto exit;
					}
				}
			}
		}
	}
	exit:;
	mutex_unlock(&priv->lock);

	return ret;
}

/*
 * An earlier version of opp-v1 bindings used to name the regulator
 * "cpu0-supply", we still need to handle that for backwards compatibility.
 */
static const char *find_supply_name(struct device *dev)
{
	struct device_node *np;
	struct property *pp;
	int cpu = dev->id;
	const char *name = NULL;

	np = of_node_get(dev->of_node);

	/* This must be valid for sure */
	if (WARN_ON(!np))
		return NULL;

	/* Try "cpu0" for older DTs */
	if (!cpu) {
		pp = of_find_property(np, "cpu0-supply", NULL);
		if (pp) {
			name = "cpu0";
			goto node_put;
		}
	}

	pp = of_find_property(np, "cpu-supply", NULL);
	if (pp) {
		name = "cpu";
		goto node_put;
	}

	dev_dbg(dev, "no regulator for cpu%d\n", cpu);
node_put:
	of_node_put(np);
	return name;
}

static int opp_notifier(struct notifier_block *nb, unsigned long event,
			void *data)
{
	struct dev_pm_opp *opp = data;
	struct private_data *priv = container_of(nb, struct private_data,
						 opp_nb);
	struct device *cpu_dev = priv->cpu_dev;
	struct regulator *cpu_reg;
	unsigned long volt, freq;
	int ret = 0;

	if (event == OPP_EVENT_ADJUST_VOLTAGE) {
		cpu_reg = dev_pm_opp_get_regulator(cpu_dev);
		if (IS_ERR(cpu_reg)) {
			ret = PTR_ERR(cpu_reg);
			goto out;
		}
		volt = dev_pm_opp_get_voltage(opp);
		freq = dev_pm_opp_get_freq(opp);

		mutex_lock(&priv->lock);
		if (freq == priv->opp_freq) {
			ret = regulator_set_voltage_triplet(cpu_reg, volt, volt, volt);
		}
		mutex_unlock(&priv->lock);
		if (ret)
			dev_err(cpu_dev, "failed to scale voltage: %d\n", ret);
	}

out:
	return notifier_from_errno(ret);
}

static int cpufreq_init(struct cpufreq_policy *policy)
{
	struct private_data *priv;
	struct device *cpu_dev;
	struct clk *cpu_clk;
	struct device_node *np;
	unsigned int transition_latency;
	int ret, cpu;
	struct device_node *l2_np;
	struct clk *l2_clk = NULL;
	struct regulator *l2_regulator = NULL;
	

	priv = cpufreq_dt_find_data(policy->cpu);
	if (!priv) {
		pr_err("failed to find data for cpu%d\n", policy->cpu);
		return -ENODEV;
	}
	cpu_dev = priv->cpu_dev;

	for_each_possible_cpu(cpu)
		per_cpu(cpu_cores_clks, cpu) = clk_get(get_cpu_device(cpu), NULL);

	cpu_clk = per_cpu(cpu_cores_clks, policy->cpu);
	if (IS_ERR(cpu_clk)) {
		ret = PTR_ERR(cpu_clk);
		dev_err(cpu_dev, "%s: failed to get clk: %d\n", __func__, ret);
		return ret;
	}

	transition_latency = dev_pm_opp_get_max_transition_latency(cpu_dev);
	if (!transition_latency)
		transition_latency = CPUFREQ_ETERNAL;

	cpumask_copy(policy->cpus, priv->cpus);
	policy->driver_data = priv;
	policy->clk = cpu_clk;
	policy->freq_table = priv->freq_table;
	policy->suspend_freq = dev_pm_opp_get_suspend_opp_freq(cpu_dev) / 1000;
	policy->cpuinfo.transition_latency = transition_latency;
	policy->dvfs_possible_from_any_cpu = true;

	policy->l2_rate_set = false;
	policy->l2_volt_set = false;

	l2_clk = clk_get(cpu_dev, "l2");
	if (!IS_ERR(l2_clk))
		policy->l2_clk = l2_clk;

	l2_np = of_find_node_by_name(NULL, "l2-cache");
	if (l2_np) {
		struct device_node *vdd;
		np = of_node_get(priv->cpu_dev->of_node);

		if (np)
			of_property_read_u32(np, "voltage-tolerance", &policy->l2_volt_tol);

		of_property_read_u32_array(l2_np, "qcom,l2-rates", policy->l2_rate, 3);
		if (policy->l2_rate[0] && policy->l2_rate[1] && policy->l2_rate[2]) {
			policy->l2_rate_set = true;
			of_property_read_u32_array(l2_np, "qcom,l2-cpufreq", policy->l2_cpufreq, 3);
			of_property_read_u32_array(l2_np, "qcom,l2-volt", policy->l2_volt, 3);
		} else
			pr_warn("L2: failed to parse L2 rates\n");

		if (!policy->l2_cpufreq[0] && !policy->l2_cpufreq[1] && 
			!policy->l2_cpufreq[2] && policy->l2_rate_set) {
			int i;

			pr_warn("L2: failed to parse target cpu freq, using defaults\n");
			for (i = 0; i < 3; i++)
				policy->l2_cpufreq[i] = policy->l2_rate[i];
		}

		if (policy->l2_volt[0] && policy->l2_volt[1] && policy->l2_volt[2] &&
			policy->l2_volt_tol && policy->l2_rate_set) {
			vdd = of_parse_phandle(l2_np, "qcom,l2-supply", 0);

			if (vdd) {
				l2_regulator = devm_regulator_get(cpu_dev, vdd->name);
				if (!IS_ERR(l2_regulator)) {
					policy->l2_regulator = l2_regulator;
					policy->l2_volt_set = true;
				} else {
					pr_warn("failed to get l2 supply\n");
					l2_regulator = NULL;
				}

				of_node_put(vdd);
			}
		}
	} else {
		pr_warn("L2: disable fab scaling\n");
	}

	/* Support turbo/boost mode */
	if (policy_has_boost_freq(policy)) {
		/* This gets disabled by core on driver unregister */
		ret = cpufreq_enable_boost_support();
		if (ret)
			goto out_clk_put;
		cpufreq_dt_attr[1] = &cpufreq_freq_attr_scaling_boost_freqs;
	}

	return 0;

out_clk_put:
	clk_put(cpu_clk);

	return ret;
}

static int cpufreq_online(struct cpufreq_policy *policy)
{
	/* We did light-weight tear down earlier, nothing to do here */
	return 0;
}

static int cpufreq_offline(struct cpufreq_policy *policy)
{
	/*
	 * Preserve policy->driver_data and don't free resources on light-weight
	 * tear down.
	 */
	return 0;
}

static int cpufreq_exit(struct cpufreq_policy *policy)
{
	clk_put(policy->clk);
	return 0;
}

static struct cpufreq_driver dt_cpufreq_driver = {
	.flags = CPUFREQ_NEED_INITIAL_FREQ_CHECK |
		 CPUFREQ_IS_COOLING_DEV,
	.verify = cpufreq_generic_frequency_table_verify,
	.target_index = set_target,
	.get = cpufreq_generic_get,
	.init = cpufreq_init,
	.exit = cpufreq_exit,
	.online = cpufreq_online,
	.offline = cpufreq_offline,
	.register_em = cpufreq_register_em_with_opp,
	.name = "cpufreq-dt",
	.attr = cpufreq_dt_attr,
	.suspend = cpufreq_generic_suspend,
};

static int dt_cpufreq_early_init(struct device *dev, int cpu)
{
	struct private_data *priv;
	struct device *cpu_dev;
	bool fallback = false;
	const char *reg_name[] = { NULL, NULL };
	int ret;

	/* Check if this CPU is already covered by some other policy */
	if (cpufreq_dt_find_data(cpu))
		return 0;

	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev)
		return -EPROBE_DEFER;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	if (!alloc_cpumask_var(&priv->cpus, GFP_KERNEL))
		return -ENOMEM;

	cpumask_set_cpu(cpu, priv->cpus);
	priv->cpu_dev = cpu_dev;
	/*
	 * OPP layer will be taking care of regulators now, but it needs to know
	 * the name of the regulator first.
	 */
	reg_name[0] = find_supply_name(cpu_dev);
	if (reg_name[0]) {
		priv->opp_token = dev_pm_opp_set_regulators(cpu_dev, reg_name);
		if (priv->opp_token < 0) {
			ret = dev_err_probe(cpu_dev, priv->opp_token,
					    "failed to set regulators\n");
			goto free_cpumask;
		}
	}

	/* Get OPP-sharing information from "operating-points-v2" bindings */
	ret = dev_pm_opp_of_get_sharing_cpus(cpu_dev, priv->cpus);
	if (ret) {
		if (ret != -ENOENT)
			goto out;

		/*
		 * operating-points-v2 not supported, fallback to all CPUs share
		 * OPP for backward compatibility if the platform hasn't set
		 * sharing CPUs.
		 */
		if (dev_pm_opp_get_sharing_cpus(cpu_dev, priv->cpus))
			fallback = true;
	}

	/*
	 * Initialize OPP tables for all priv->cpus. They will be shared by
	 * all CPUs which have marked their CPUs shared with OPP bindings.
	 *
	 * For platforms not using operating-points-v2 bindings, we do this
	 * before updating priv->cpus. Otherwise, we will end up creating
	 * duplicate OPPs for the CPUs.
	 *
	 * OPPs might be populated at runtime, don't fail for error here unless
	 * it is -EPROBE_DEFER.
	 */
	ret = dev_pm_opp_of_cpumask_add_table(priv->cpus);
	if (!ret) {
		priv->have_static_opps = true;
	} else if (ret == -EPROBE_DEFER) {
		goto out;
	}

	/*
	 * The OPP table must be initialized, statically or dynamically, by this
	 * point.
	 */
	ret = dev_pm_opp_get_opp_count(cpu_dev);
	if (ret <= 0) {
		dev_err(cpu_dev, "OPP table can't be empty\n");
		ret = -ENODEV;
		goto out;
	}

	if (fallback) {
		cpumask_setall(priv->cpus);
		ret = dev_pm_opp_set_sharing_cpus(cpu_dev, priv->cpus);
		if (ret)
			dev_err(cpu_dev, "%s: failed to mark OPPs as shared: %d\n",
				__func__, ret);
	}

	mutex_init(&priv->lock);
	priv->opp_nb.notifier_call = opp_notifier;
	dev_pm_opp_register_notifier(cpu_dev, &priv->opp_nb);

	ret = dev_pm_opp_init_cpufreq_table(cpu_dev, &priv->freq_table);
	if (ret) {
		dev_err(cpu_dev, "failed to init cpufreq table: %d\n", ret);
		goto out_unregister_nb;
	}

	list_add(&priv->node, &priv_list);
	return 0;

out:
	if (priv->have_static_opps)
		dev_pm_opp_of_cpumask_remove_table(priv->cpus);
	dev_pm_opp_put_regulators(priv->opp_token);
out_unregister_nb:
	dev_pm_opp_unregister_notifier(cpu_dev, &priv->opp_nb);
free_cpumask:
	free_cpumask_var(priv->cpus);
	return ret;
}

static void dt_cpufreq_release(void)
{
	struct private_data *priv, *tmp;

	list_for_each_entry_safe(priv, tmp, &priv_list, node) {
		dev_pm_opp_free_cpufreq_table(priv->cpu_dev, &priv->freq_table);
		if (priv->have_static_opps)
			dev_pm_opp_of_cpumask_remove_table(priv->cpus);
		dev_pm_opp_put_regulators(priv->opp_token);
		free_cpumask_var(priv->cpus);
		list_del(&priv->node);
	}
}

static int dt_cpufreq_probe(struct platform_device *pdev)
{
	struct cpufreq_dt_platform_data *data = dev_get_platdata(&pdev->dev);
	int ret, cpu;

	/* Request resources early so we can return in case of -EPROBE_DEFER */
	for_each_possible_cpu(cpu) {
		ret = dt_cpufreq_early_init(&pdev->dev, cpu);
		if (ret)
			goto err;
	}

	if (data) {
		if (data->have_governor_per_policy)
			dt_cpufreq_driver.flags |= CPUFREQ_HAVE_GOVERNOR_PER_POLICY;

		dt_cpufreq_driver.resume = data->resume;
		if (data->suspend)
			dt_cpufreq_driver.suspend = data->suspend;
		if (data->get_intermediate) {
			dt_cpufreq_driver.target_intermediate = data->target_intermediate;
			dt_cpufreq_driver.get_intermediate = data->get_intermediate;
		}
	}

	ret = cpufreq_register_driver(&dt_cpufreq_driver);
	if (ret) {
		dev_err(&pdev->dev, "failed register driver: %d\n", ret);
		goto err;
	}
	return 0;
err:
	dt_cpufreq_release();
	return ret;
}

static int dt_cpufreq_remove(struct platform_device *pdev)
{
	cpufreq_unregister_driver(&dt_cpufreq_driver);
	dt_cpufreq_release();
	return 0;
}

static struct platform_driver dt_cpufreq_platdrv = {
	.driver = {
		.name	= "cpufreq-dt",
	},
	.probe		= dt_cpufreq_probe,
	.remove		= dt_cpufreq_remove,
};
module_platform_driver(dt_cpufreq_platdrv);

MODULE_ALIAS("platform:cpufreq-dt");
MODULE_AUTHOR("Viresh Kumar <viresh.kumar@linaro.org>");
MODULE_AUTHOR("Shawn Guo <shawn.guo@linaro.org>");
MODULE_DESCRIPTION("Generic cpufreq driver");
MODULE_LICENSE("GPL");
