/* Copyright (c) 2009-2014, 2016, 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/of.h>

#define CPU_IPQ9514 510
#define CPU_IPQ9554 512
#define CPU_IPQ9570 513
#define CPU_IPQ9574 514
#define CPU_IPQ9550 511
#define CPU_IPQ9510 521

#define CPU_IPQ5332 592
#define CPU_IPQ5322 593
#define CPU_IPQ5312 594
#define CPU_IPQ5302 595
#define CPU_IPQ5300 624
#define CPU_IPQ5321 650

static inline int read_ipq_soc_version_major(void)
{
	const int *prop;
	prop = of_get_property(of_find_node_by_path("/"), "soc_version_major",
				NULL);

	if (!prop)
		return -EINVAL;

	return le32_to_cpu(*prop);
}

static inline int read_ipq_cpu_type(void)
{
	const int *prop;
	prop = of_get_property(of_find_node_by_path("/"), "cpu_type", NULL);
	/*
	 * Return Default CPU type if "cpu_type" property is not found in DTSI
	 */
	if (!prop)
		return CPU_IPQ9574;

	return le32_to_cpu(*prop);
}

static inline int cpu_is_ipq5332(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5332;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5322(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5322;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5312(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5312;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5302(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5302;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5300(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5300;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq5321(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ5321;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9514(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9514;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9554(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9554;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9570(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9570;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9574(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9574;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9550(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9550;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq9510(void)
{
#ifdef CONFIG_ARCH_QCOM
	return read_ipq_cpu_type() == CPU_IPQ9510;
#else
	return 0;
#endif
}

static inline int cpu_is_ipq53xx(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq5332() || cpu_is_ipq5322() ||
		cpu_is_ipq5312() || cpu_is_ipq5302() ||
		cpu_is_ipq5300() || cpu_is_ipq5321();
#else
	return 0;
#endif
}

static inline int cpu_is_ipq95xx(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq9514() || cpu_is_ipq9554() ||
		cpu_is_ipq9570() || cpu_is_ipq9574() ||
		cpu_is_ipq9550() || cpu_is_ipq9510();
#else
	return 0;
#endif
}

static inline int cpu_is_nss_crypto_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return	cpu_is_ipq5322() || cpu_is_ipq9570() ||
		cpu_is_ipq9550() || cpu_is_ipq9574() ||
		cpu_is_ipq9554() || cpu_is_ipq5332() ||
		cpu_is_ipq5300() || cpu_is_ipq5321();
#else
	return 0;
#endif
}

static inline int cpu_is_internal_wifi_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq53xx() || cpu_is_ipq9514() ||
		cpu_is_ipq9554() || cpu_is_ipq9574();
#else
	return 0;
#endif
}

static inline int cpu_is_uniphy1_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq9570() || cpu_is_ipq9574() ||
		cpu_is_ipq53xx();
#else
	return 0;
#endif
}

static inline int cpu_is_uniphy2_enabled(void)
{
#ifdef CONFIG_ARCH_QCOM
	return  cpu_is_ipq9554() || cpu_is_ipq9570() ||
		cpu_is_ipq9574() || cpu_is_ipq9550();
#else
	return 0;
#endif
}

#endif /* _ARCH_ARM_MACH_MSM_SOCINFO_H_ */
