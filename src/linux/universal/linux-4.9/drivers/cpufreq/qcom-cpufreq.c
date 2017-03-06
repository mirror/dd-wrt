/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/cpu.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/slab.h>
#include <linux/cpufreq-dt.h>

static void __init get_krait_bin_format_a(int *speed, int *pvs, int *pvs_ver)
{
	void __iomem *base;
	u32 pte_efuse;

	*speed = *pvs = *pvs_ver = 0;

	base = ioremap(0x007000c0, 4);
	if (!base) {
		pr_warn("Unable to read efuse data. Defaulting to 0!\n");
		return;
	}

	pte_efuse = readl_relaxed(base);
	iounmap(base);

	*speed = pte_efuse & 0xf;
	if (*speed == 0xf)
		*speed = (pte_efuse >> 4) & 0xf;

	if (*speed == 0xf) {
		*speed = 0;
		pr_warn("Speed bin: Defaulting to %d\n", *speed);
	} else {
		pr_info("Speed bin: %d\n", *speed);
	}

	*pvs = (pte_efuse >> 10) & 0x7;
	if (*pvs == 0x7)
		*pvs = (pte_efuse >> 13) & 0x7;

	if (*pvs == 0x7) {
		*pvs = 0;
		pr_warn("PVS bin: Defaulting to %d\n", *pvs);
	} else {
		pr_info("PVS bin: %d\n", *pvs);
	}
}

static void __init get_krait_bin_format_b(int *speed, int *pvs, int *pvs_ver)
{
	u32 pte_efuse, redundant_sel;
	void __iomem *base;

	*speed = 0;
	*pvs = 0;
	*pvs_ver = 0;

	base = ioremap(0xfc4b80b0, 8);
	if (!base) {
		pr_warn("Unable to read efuse data. Defaulting to 0!\n");
		return;
	}

	pte_efuse = readl_relaxed(base);
	redundant_sel = (pte_efuse >> 24) & 0x7;
	*speed = pte_efuse & 0x7;
	/* 4 bits of PVS are in efuse register bits 31, 8-6. */
	*pvs = ((pte_efuse >> 28) & 0x8) | ((pte_efuse >> 6) & 0x7);
	*pvs_ver = (pte_efuse >> 4) & 0x3;

	switch (redundant_sel) {
	case 1:
		*speed = (pte_efuse >> 27) & 0xf;
		break;
	case 2:
		*pvs = (pte_efuse >> 27) & 0xf;
		break;
	}

	/* Check SPEED_BIN_BLOW_STATUS */
	if (pte_efuse & BIT(3)) {
		pr_info("Speed bin: %d\n", *speed);
	} else {
		pr_warn("Speed bin not set. Defaulting to 0!\n");
		*speed = 0;
	}

	/* Check PVS_BLOW_STATUS */
	pte_efuse = readl_relaxed(base + 0x4) & BIT(21);
	if (pte_efuse) {
		pr_info("PVS bin: %d\n", *pvs);
	} else {
		pr_warn("PVS bin not set. Defaulting to 0!\n");
		*pvs = 0;
	}

	pr_info("PVS version: %d\n", *pvs_ver);
	iounmap(base);
}

static int __init qcom_cpufreq_populate_opps(void)
{
	int len, rows, cols, i, k, speed, pvs, pvs_ver;
	char table_name[] = "qcom,speedXX-pvsXX-bin-vXX";
	struct device_node *np;
	struct device *dev;
	int cpu = 0;

	np = of_find_node_by_name(NULL, "qcom,pvs");
	if (!np)
		return -ENODEV;

	if (of_property_read_bool(np, "qcom,pvs-format-a")) {
		get_krait_bin_format_a(&speed, &pvs, &pvs_ver);
		cols = 2;
	} else if (of_property_read_bool(np, "qcom,pvs-format-b")) {
		get_krait_bin_format_b(&speed, &pvs, &pvs_ver);
		cols = 3;
	} else {
		return -ENODEV;
	}

	snprintf(table_name, sizeof(table_name),
			"qcom,speed%d-pvs%d-bin-v%d", speed, pvs, pvs_ver);

	if (!of_find_property(np, table_name, &len))
		return -EINVAL;

	len /= sizeof(u32);
	if (len % cols || len == 0)
		return -EINVAL;

	rows = len / cols;

	for (i = 0, k = 0; i < rows; i++) {
		u32 freq, volt;

		of_property_read_u32_index(np, table_name, k++, &freq);
		of_property_read_u32_index(np, table_name, k++, &volt);
		while (k % cols)
			k++; /* Skip uA entries if present */
		for (cpu = 0; cpu < num_possible_cpus(); cpu++) {
			dev = get_cpu_device(cpu);
			if (!dev)
				return -ENODEV;
			if (dev_pm_opp_add(dev, freq, volt))
				pr_warn("failed to add OPP %u\n", freq);
		}
	}

	return 0;
}

static int __init qcom_cpufreq_driver_init(void)
{
	struct platform_device_info devinfo = {
		.name = "cpufreq-krait",
	};
	struct device *cpu_dev;
	struct device_node *np;
	int ret;

	cpu_dev = get_cpu_device(0);
	if (!cpu_dev)
		return -ENODEV;

	np = of_node_get(cpu_dev->of_node);
	if (!np)
		return -ENOENT;

	if (!of_device_is_compatible(np, "qcom,krait")) {
		of_node_put(np);
		return -ENODEV;
	}
	of_node_put(np);

	ret = qcom_cpufreq_populate_opps();
	if (ret)
		return ret;

	return PTR_ERR_OR_ZERO(platform_device_register_full(&devinfo));
}
module_init(qcom_cpufreq_driver_init);

MODULE_DESCRIPTION("Qualcomm CPUfreq driver");
MODULE_LICENSE("GPL v2");
