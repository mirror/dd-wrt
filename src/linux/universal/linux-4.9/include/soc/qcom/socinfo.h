/* Copyright (c) 2009-2014, 2016 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _ARCH_ARM_MACH_MSM_SOCINFO_H_
#define _ARCH_ARM_MACH_MSM_SOCINFO_H_

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/of_fdt.h>
#include <linux/of.h>

#include <asm/cputype.h>

#define CPU_IPQ4018 272
#define CPU_IPQ4019 273
#define CPU_IPQ4028 287
#define CPU_IPQ4029 288

#define CPU_IPQ8062 201
#define CPU_IPQ8064 202
#define CPU_IPQ8066 203
#define CPU_IPQ8065 280
#define CPU_IPQ8068 204
#define CPU_IPQ8069 281

#define CPU_IPQ8074 323
#define CPU_IPQ8072 342
#define CPU_IPQ8076 343
#define CPU_IPQ8078 344
#define CPU_IPQ8070 375
#define CPU_IPQ8071 376

#define CPU_IPQ8072A 389
#define CPU_IPQ8074A 390
#define CPU_IPQ8076A 391
#define CPU_IPQ8078A 392
#define CPU_IPQ8070A 395
#define CPU_IPQ8071A 396

#define CPU_IPQ8172  397
#define CPU_IPQ8173  398
#define CPU_IPQ8174  399

static inline const int* read_ipq_soc_version_major(void)
{
	const int *prop;
	prop = of_get_property(of_find_node_by_path("/"), "soc_version_major",
				NULL);

	return prop;
}

static inline int read_ipq_cpu_type(void)
{
	const int *prop;
	prop = of_get_property(of_find_node_by_path("/"), "cpu_type", NULL);
	/*
	 * Return Default CPU type if "cpu_type" property is not found in DTSI
	 */
	if (!prop)
		return CPU_IPQ8064;
	return *prop;
}

static inline int cpu_is_ipq4018(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ4018;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq4019(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ4019;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq4028(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ4028;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq4029(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ4029;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq40xx(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq4018() || cpu_is_ipq4019() ||
		cpu_is_ipq4028() || cpu_is_ipq4029();
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8062(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8062;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8064(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8064;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8066(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8066;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8068(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8068;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8065(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8065;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8069(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8069;
#else
	return 0;
#endif
}
static inline int cpu_is_ipq806x(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq8062() || cpu_is_ipq8064() ||
		cpu_is_ipq8066() || cpu_is_ipq8068() ||
		cpu_is_ipq8065() || cpu_is_ipq8069();
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8070(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8070;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8071(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8071;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8072(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8072;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8074(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8074;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8076(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8076;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8078(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8078;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8072a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8072A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8074a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8074A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8076a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8076A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8078a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8078A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8070a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8070A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8071a(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8071A;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8172(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8172;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8173(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8173;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq8174(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ8174;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq807x(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq8072() || cpu_is_ipq8074() ||
		cpu_is_ipq8076() || cpu_is_ipq8078() ||
		cpu_is_ipq8070() || cpu_is_ipq8071() ||
		cpu_is_ipq8072a() || cpu_is_ipq8074a() ||
		cpu_is_ipq8076a() || cpu_is_ipq8078a() ||
		cpu_is_ipq8070a() || cpu_is_ipq8071a() ||
		cpu_is_ipq8172() || cpu_is_ipq8173() ||
		cpu_is_ipq8174();
#else
	return 0;
#endif
}

#endif /* _ARCH_ARM_MACH_MSM_SOCINFO_H_ */
