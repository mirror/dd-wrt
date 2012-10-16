/*
 * Copyright 2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <mach/system.h>
#include <mach/cns3xxx.h>
#include <linux/cpufreq.h>


void cns3xxx_pwr_change_cpu_clock(unsigned int cpu_sel, unsigned int div_sel);

static struct cpufreq_frequency_table *cns_freqs_table;

static struct cpufreq_frequency_table cns_frequency_table[] = {
    {0, CPUFREQ_ENTRY_INVALID},
    {1, CPUFREQ_ENTRY_INVALID},
    {2, CPUFREQ_ENTRY_INVALID},
    {3, 400000},
    {4, CPUFREQ_ENTRY_INVALID},
    {6, CPUFREQ_ENTRY_INVALID},
    {6, 500000},
    {7, CPUFREQ_ENTRY_INVALID},
    {8, CPUFREQ_ENTRY_INVALID},
    {9, 600000},
    {10, CPUFREQ_TABLE_END},
};

static int cns_cpufreq_verify(struct cpufreq_policy *policy)
{
    return cpufreq_frequency_table_verify(policy, cns_freqs_table);
}

static unsigned int cns_cpufreq_get(unsigned int cpu)
{
    /* return CPU frequency in KHz*/
    return cns3xxx_cpu_clock()*1000;;
}

static spinlock_t cpufreq_lock;
static int cns_cpufreq_target(struct cpufreq_policy *policy,
                  unsigned int target_freq,
                  unsigned int relation)
{
    struct cpufreq_freqs freqs;
    unsigned long flags;
    int idx;

    if (policy->cpu != 0)
        return -EINVAL;

    /* Lookup the next frequency */
    if (cpufreq_frequency_table_target(policy, cns_freqs_table,
                target_freq, relation, &idx))
        return -EINVAL;

    freqs.old = policy->cur;
    freqs.new = cns_freqs_table[idx].frequency;
    freqs.cpu = policy->cpu;

    pr_debug("CPU frequency from %d MHz to %d MHz%s\n",
            freqs.old / 1000, freqs.new / 1000,
            (freqs.old == freqs.new) ? " (skipped)" : "");

    if (freqs.old == target_freq)
        return 0;
    if (freqs.old == freqs.new)
        return 0;

    cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

    spin_lock_irqsave(&cpufreq_lock, flags);
    cns3xxx_pwr_change_cpu_clock(idx, CNS3XXX_PWR_CPU_CLK_DIV_BY1);
#if 0
    /* XXX: this is for debug */
    {
    #define MAX_NOP 100
        int i, cnt = 0;
        for (i=0; i<MAX_NOP; i++)
            asm volatile("nop\n");

        /* wait for DMC ready */
#define DMC_REG_VALUE(offset) (*((volatile unsigned int *)(CNS3XXX_DMC_BASE_VIRT+offset)))
        while (0x1 != (DMC_REG_VALUE(0x0) & 0x3)) {
            cnt++;
            if (cnt > 1024) {
                printk("%s: cnt is %d\n", __FUNCTION__, cnt);
                BUG();
            }
        };
    }
#endif
spin_unlock_irqrestore(&cpufreq_lock, flags);
    cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
    return 0;
}

static __init int cns_cpufreq_init(struct cpufreq_policy *policy)
{
    if (0 != policy->cpu)
        return -EINVAL;

    cns_freqs_table = cns_frequency_table;
    /* what is the actual transition time ?*/
    policy->cpuinfo.transition_latency = 10*1000*1000;
    policy->cur = policy->min = policy->max = cns3xxx_cpu_clock()*1000; /* unit: kHz */
    cpufreq_frequency_table_cpuinfo(policy, cns_freqs_table);
    spin_lock_init(&cpufreq_lock);
    return 0;
};

static struct cpufreq_driver cns_cpu_freq_driver = {
    .verify     = cns_cpufreq_verify,
    .target     = cns_cpufreq_target,
    .init       = cns_cpufreq_init,
    .get        = cns_cpufreq_get,
    .name       = "cns-cpufreq",
};

struct laguna_board_info {
	char model[16];
	u32 config_bitmap;
	u32 config2_bitmap;
	u8 nor_flash_size;
	u8 spi_flash_size;
};

extern struct laguna_board_info laguna_info __initdata;

static int __init cns3xxx_cpufreq_init(void)
{
	return cpufreq_register_driver(&cns_cpu_freq_driver);
}

module_init(cns3xxx_cpufreq_init);


MODULE_AUTHOR("Cavium");
MODULE_LICENSE("GPL");
