/*
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  based on linux/arch/arm/mach-realview/platsmp.c
 */
#ifdef	CONFIG_SMP
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/mach-types.h>
#include <asm/unified.h>
#include <asm/smp.h>
#include <asm/smp_scu.h>
#include <asm/cacheflush.h>

#include <plat/mpcore.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/clockchips.h>
//#include <mach/hardware.h>
//#include <mach/smp.h>

//volatile int __cpuinitdata pen_release = -1;

static inline unsigned int get_core_count(void)
{
	void __iomem *scu_base = scu_base_addr();
	if (scu_base)
		return scu_get_core_count(scu_base);
	return 1;
}

static void write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}



static DEFINE_RAW_SPINLOCK(boot_lock);

static void brcm_secondary_init(unsigned int cpu)
{

	trace_hardirqs_off();

	/*
	 * if any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */
	mpcore_cpu_init();

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	raw_spin_lock(&boot_lock);
	raw_spin_unlock(&boot_lock);
}

static int brcm_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
    	unsigned long timeout;
	/*
	 * set synchronisation state between this boot processor
	 * and the secondary one
	 */
	raw_spin_lock(&boot_lock);

	/*
	 * The secondary processor is waiting to be released from
	 * the holding pen - release it, then wait for it to flag
	 * that it has been released by resetting pen_release.
	 *
	 * Note that "pen_release" is the hardware CPU ID, whereas
	 * "cpu" is Linux's internal ID.
	 */
	write_pen_release(cpu);

	dsb_sev();

	udelay(100);
	/*
	 * If the secondary CPU was waiting on WFE, it should
	 * be already watching <pen_release>, or it could be
	 * waiting in WFI, send it an IPI to be sure it wakes.
	 */
//	arch_send_wakeup_ipi_mask(cpumask_of(cpu));
	if (pen_release != -1)
		tick_broadcast(cpumask_of(cpu));


	/*
	 * Timeout set on purpose in jiffies so that on slow processors
	 * that must also have low HZ it will wait longer.
	 */
	timeout = jiffies + (HZ * 10);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	raw_spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init brcm_smp_init_cpus(void)
{
	unsigned int i, ncores = get_core_count();

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

//	set_smp_cross_call(gic_raise_softirq);

}
extern void platform_secondary_startup(void);

static void __init  brcm_smp_prepare_cpus(unsigned int max_cpus)
{
	unsigned int ncores = get_core_count();
	unsigned int cpu = smp_processor_id();
	int i;

	
	/* sanity check */
	if (ncores == 0) {
		printk(KERN_ERR
		       "MPCORE: strange CPU count of 0? Default to 1\n");

		ncores = 1;
	}

	if (ncores > NR_CPUS) {
		printk(KERN_WARNING
		       "MPCORE: no. of cores (%d) greater than configured "
		       "maximum of %d - clipping\n",
		       ncores, NR_CPUS);
		ncores = NR_CPUS;
	}
	printk(KERN_INFO "%d cores has been found\n",ncores);
	/*
	 * are we trying to boot more cores than exist?
	 */
	if (max_cpus > ncores)
		max_cpus = ncores;

	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);

	/*
	 * Initialise the SCU if there are more than one CPU and let
	 * them know where to start. Note that, on modern versions of
	 * MILO, the "poke" doesn't actually do anything until each
	 * individual core is sent a soft interrupt to get it out of
	 * WFI
	 */
	if (max_cpus > 1) {
		/* nobody is to be released from the pen yet */
		pen_release = -1;

		/*
		 * Enable the local timer or broadcast device for the
		 * boot CPU, but only if we have more than one CPU.
		 */
//		percpu_timer_setup();

		scu_enable(scu_base_addr());

		/* Wakeup other cores in an SoC-specific manner */
		plat_wake_secondary_cpu( max_cpus, platform_secondary_startup );

	}
}

struct smp_operations  brcm_smp_ops __initdata = {
	.smp_init_cpus		= brcm_smp_init_cpus,
	.smp_prepare_cpus	= brcm_smp_prepare_cpus,
	.smp_secondary_init	= brcm_secondary_init,
	.smp_boot_secondary	= brcm_boot_secondary,
};


#endif	/* CONFIG_SMP */
