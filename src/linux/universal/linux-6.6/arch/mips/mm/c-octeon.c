/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005-2007 Cavium Networks
 */
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/mm.h>
#include <linux/bitops.h>
#include <linux/cpu.h>
#include <linux/io.h>

#include <asm/bcache.h>
#include <asm/bootinfo.h>
#include <asm/cacheops.h>
#include <asm/cpu-features.h>
#include <asm/cpu-type.h>
#include <asm/page.h>
#include <asm/r4kcache.h>
#include <asm/traps.h>
#include <asm/mmu_context.h>

#include <asm/octeon/octeon.h>

/*
 * 32 is the maximum number of cores on OCTEON II CPUS, which are the
 * only ones that used this array.
 */
unsigned long long cache_err_dcache[32];
EXPORT_SYMBOL_GPL(cache_err_dcache);

static RAW_NOTIFIER_HEAD(co_cache_error_chain);

/*
 * Octeon automatically flushes the dcache on tlb changes, so
 * from Linux's viewpoint it acts much like a physically
 * tagged cache. No flushing is needed
 *
 */
static void octeon_flush_data_cache_page(unsigned long addr)
{
    /* Nothing to do */
	mb(); /* SYNC */
}

static void octeon_local_flush_icache(void)
{
	asm volatile ("synci 0($0)");
}

/*
 * Flush local I-cache for the specified range.
 */
static void octeon_local_flush_icache_range(unsigned long start,
					    unsigned long end)
{
	mb();
	octeon_local_flush_icache();
}

/**
 * octeon_flush_icache_all_cores -  Flush caches as necessary for all cores
 * affected by a vma. If no vma is supplied, all cores are flushed.
 *
 * @vma:    VMA to flush or NULL to flush all icaches.
 */
static void octeon_flush_icache_all_cores(struct vm_area_struct *vma)
{
	extern void octeon_send_ipi_single(int cpu, unsigned int action);
#ifdef CONFIG_SMP
	int cpu;
	cpumask_t mask;
#endif

	mb();
	octeon_local_flush_icache();
#ifdef CONFIG_SMP
	preempt_disable();
	cpu = smp_processor_id();

	/*
	 * If we have a vma structure, we only need to worry about
	 * cores it has been used on
	 */
	if (vma)
		mask = *mm_cpumask(vma->vm_mm);
	else
		mask = *cpu_online_mask;
	cpumask_clear_cpu(cpu, &mask);
#ifdef CONFIG_CAVIUM_OCTEON_SOC
	for_each_cpu(cpu, &mask)
		octeon_send_ipi_single(cpu, SMP_ICACHE_FLUSH);
#else
	smp_call_function_many(&mask, (smp_call_func_t)octeon_local_flush_icache,
			       NULL, 1);
#endif

	preempt_enable();
#endif
}


/*
 * Called to flush the icache on all cores
 */
static void octeon_flush_icache_all(void)
{
	octeon_flush_icache_all_cores(NULL);
}


/**
 * octeon_flush_cache_mm - flush all memory associated with a memory context.
 *
 * @mm:	    Memory context to flush
 */
static void octeon_flush_cache_mm(struct mm_struct *mm)
{
	/*
	 * According to the R4K version of this file, CPUs without
	 * dcache aliases don't need to do anything here
	 */
}


/*
 * Flush a range of kernel addresses out of the icache
 *
 */
static void octeon_flush_icache_range(unsigned long start, unsigned long end)
{
	octeon_flush_icache_all_cores(NULL);
}


/**
 * octeon_flush_cache_range - Flush a range out of a vma
 *
 * @vma:    VMA to flush
 * @start:  beginning address for flush
 * @end:    ending address for flush
 */
static void octeon_flush_cache_range(struct vm_area_struct *vma,
				     unsigned long start, unsigned long end)
{
	if (vma->vm_flags & VM_EXEC)
		octeon_flush_icache_all_cores(vma);
}


/**
 * octeon_flush_cache_page - Flush a specific page of a vma
 *
 * @vma:    VMA to flush page for
 * @page:   Page to flush
 * @pfn:    Page frame number
 */
static void octeon_flush_cache_page(struct vm_area_struct *vma,
				    unsigned long page, unsigned long pfn)
{
	if (vma->vm_flags & VM_EXEC)
		octeon_flush_icache_all_cores(vma);
}

static void octeon_flush_kernel_vmap_range(unsigned long vaddr, int size)
{
	BUG();
}

/*
 * Octeon specific bus error handler, as write buffer parity errors
 * trigger bus errors.  These are fatal since the copy in the write buffer
 * is the only copy of the data.
 */
static int octeon2_be_handler(struct pt_regs *regs, int is_fixup)
{
	u64 dcache_err;
	u64 wbfperr_mask = 1ULL << 1;

	dcache_err = read_octeon_c0_dcacheerr();
	if (dcache_err & wbfperr_mask) {
		int rv = raw_notifier_call_chain(&co_cache_error_chain,
						 CO_CACHE_ERROR_WB_PARITY,
						 NULL);
		if ((rv & ~NOTIFY_STOP_MASK) != NOTIFY_OK) {
			unsigned int coreid = cvmx_get_core_num();

			pr_err("Core%u: Write buffer parity error:\n", coreid);
			pr_err("CacheErr (Dcache) == %llx\n", dcache_err);
		}

		write_octeon_c0_dcacheerr(wbfperr_mask);
		return MIPS_BE_FATAL;
	}
	if (is_fixup)
		return MIPS_BE_FIXUP;
	else
		return MIPS_BE_FATAL;
}

/*
 * Octeon specific MachineCheck handler, as TLB parity errors
 * trigger MachineCheck errors.
 */
static int octeon2_mcheck_handler(struct pt_regs *regs)
{
	u64 dcache_err;
	u64 tlbperr_mask = 1ULL << 5;
	dcache_err = read_octeon_c0_dcacheerr();
	if (dcache_err & tlbperr_mask) {
		int rv;
		union octeon_cvmemctl cvmmemctl;

		/* Clear the indicator */
		write_octeon_c0_dcacheerr(tlbperr_mask);
		/*
		 * Blow everything away to (hopefully) write good
		 * parity to all TLB entries
		 */
		local_flush_tlb_all();
		/* Reenable TLB parity error reporting. */
		cvmmemctl.u64 = read_c0_cvmmemctl();
		cvmmemctl.s.tlbperrena = 1;
		write_c0_cvmmemctl(cvmmemctl.u64);

		rv = raw_notifier_call_chain(&co_cache_error_chain,
					     CO_CACHE_ERROR_TLB_PARITY,
					     NULL);
		if ((rv & ~NOTIFY_STOP_MASK) != NOTIFY_OK) {
			unsigned int coreid = cvmx_get_core_num();

			pr_err("Core%u: TLB parity error:\n", coreid);
			return MIPS_MC_FATAL;
		}

		return MIPS_MC_DISCARD;
	}
	return MIPS_MC_NOT_HANDLED;
}

/*
 * Octeon3 specific bus error handler, as write buffer parity errors
 * trigger bus errors.  These are fatal since the copy in the write buffer
 * is the only copy of the data.
 */
static int octeon3_be_handler(struct pt_regs *regs, int is_fixup)
{
	u64 dcache_err;
	u64 wbfperr_mask = 1ULL << 9;

	dcache_err = read_octeon_c0_errctl();
	if (dcache_err & wbfperr_mask) {
		int rv = raw_notifier_call_chain(&co_cache_error_chain,
						 CO_CACHE_ERROR_WB_PARITY,
						 NULL);
		if ((rv & ~NOTIFY_STOP_MASK) != NOTIFY_OK) {
			unsigned int coreid = cvmx_get_core_num();

			pr_err("Core%u: Write buffer parity error:\n", coreid);
			pr_err("CacheErr (Dcache) == %llx\n", dcache_err);
		}

		write_octeon_c0_errctl(dcache_err | wbfperr_mask);
		return MIPS_BE_FATAL;
	}
	if (is_fixup)
		return MIPS_BE_FIXUP;
	else
		return MIPS_BE_FATAL;
}

/*
 * Octeon3 specific MachineCheck handler, as TLB parity errors
 * trigger MachineCheck errors.
 */
static int octeon3_mcheck_handler(struct pt_regs *regs)
{
	u64 dcache_err;
	u64 tlbperr_mask = 1ULL << 14;
	dcache_err = read_octeon_c0_errctl();
	if (dcache_err & tlbperr_mask) {
		int rv;
		union octeon_cvmemctl cvmmemctl;

		/* Clear the indicator */
		write_octeon_c0_errctl(dcache_err | tlbperr_mask);
		/*
		 * Blow everything away to (hopefully) write good
		 * parity to all TLB entries
		 */
		local_flush_tlb_all();
		/* Reenable TLB parity error reporting. */
		cvmmemctl.u64 = read_c0_cvmmemctl();
		cvmmemctl.s.tlbperrena = 1;
		write_c0_cvmmemctl(cvmmemctl.u64);

		rv = raw_notifier_call_chain(&co_cache_error_chain,
					     CO_CACHE_ERROR_TLB_PARITY,
					     NULL);
		if ((rv & ~NOTIFY_STOP_MASK) != NOTIFY_OK) {
			unsigned int coreid = cvmx_get_core_num();

			pr_err("Core%u: TLB parity error:\n", coreid);
			return MIPS_MC_FATAL;
		}

		return MIPS_MC_DISCARD;
	}
	return MIPS_MC_NOT_HANDLED;
}

void (*octeon_scache_init)(void);

/*
 * Probe Octeon's caches
 *
 */
static void probe_octeon(void)
{
	unsigned long icache_size;
	unsigned long dcache_size;
	unsigned long scache_size;
	unsigned int config1;
	struct cpuinfo_mips *c = &current_cpu_data;
	int cputype = current_cpu_type();

	config1 = read_c0_config1();
	switch (cputype) {
	case CPU_CAVIUM_OCTEON:
	case CPU_CAVIUM_OCTEON_PLUS:
		c->icache.linesz = 2 << ((config1 >> 19) & 7);
		c->icache.sets = 64 << ((config1 >> 22) & 7);
		c->icache.ways = 1 + ((config1 >> 16) & 7);
		c->icache.flags |= MIPS_CACHE_VTAG;
		icache_size =
			c->icache.sets * c->icache.ways * c->icache.linesz;
		c->icache.waybit = ffs(icache_size / c->icache.ways) - 1;
		c->dcache.linesz = 128;
		if (cputype == CPU_CAVIUM_OCTEON_PLUS)
			c->dcache.sets = 2; /* CN5XXX has two Dcache sets */
		else
			c->dcache.sets = 1; /* CN3XXX has one Dcache set */
		c->dcache.ways = 64;
		dcache_size =
			c->dcache.sets * c->dcache.ways * c->dcache.linesz;
		c->dcache.waybit = ffs(dcache_size / c->dcache.ways) - 1;
		c->options |= MIPS_CPU_PREFETCH;
		break;

	case CPU_CAVIUM_OCTEON2:
		c->icache.linesz = 2 << ((config1 >> 19) & 7);
		c->icache.sets = 8;
		c->icache.ways = 37;
		c->icache.flags |= MIPS_CACHE_VTAG;
		icache_size = c->icache.sets * c->icache.ways * c->icache.linesz;

		c->dcache.linesz = 128;
		c->dcache.ways = 32;
		c->dcache.sets = 8;
		dcache_size = c->dcache.sets * c->dcache.ways * c->dcache.linesz;
		c->options |= MIPS_CPU_PREFETCH;
		mips_set_be_handler(octeon2_be_handler);
		board_mcheck_handler = octeon2_mcheck_handler;
		break;

	case CPU_CAVIUM_OCTEON3:
		c->icache.linesz = 128;
		c->icache.sets = 16;
		c->icache.ways = 39;
		c->icache.flags |= MIPS_CACHE_VTAG;
		icache_size = c->icache.sets * c->icache.ways * c->icache.linesz;

		c->dcache.linesz = 128;
		c->dcache.ways = 32;
		c->dcache.sets = 8;
		dcache_size = c->dcache.sets * c->dcache.ways * c->dcache.linesz;
		c->options |= MIPS_CPU_PREFETCH;
		mips_set_be_handler(octeon3_be_handler);
		board_mcheck_handler = octeon3_mcheck_handler;
		break;

	default:
		panic("Unsupported Cavium Networks CPU type");
		break;
	}

	/* compute a couple of other cache variables */
	c->icache.waysize = icache_size / c->icache.ways;
	c->dcache.waysize = dcache_size / c->dcache.ways;

	c->icache.sets = icache_size / (c->icache.linesz * c->icache.ways);
	c->dcache.sets = dcache_size / (c->dcache.linesz * c->dcache.ways);

	scache_size = cvmx_l2c_get_cache_size_bytes();

	c->scache.sets = cvmx_l2c_get_num_sets();
	c->scache.ways = cvmx_l2c_get_num_assoc();
	c->scache.waybit = ffs(scache_size / c->scache.ways) - 1;
	c->scache.waysize = scache_size / c->scache.ways;
	c->scache.linesz = 128;
	c->scache.flags |= MIPS_CPU_PREFETCH;

	c->tcache.flags |= MIPS_CACHE_NOT_PRESENT;

	if (smp_processor_id() == 0) {
		pr_info("Primary instruction cache %ldkB, %s, %d way, "
			"%d sets, linesize %d bytes.\n",
			icache_size >> 10,
			cpu_has_vtag_icache ?
				"virtually tagged" : "physically tagged",
			c->icache.ways, c->icache.sets, c->icache.linesz);

		pr_info("Primary data cache %ldkB, %d-way, %d sets, "
			"linesize %d bytes.\n",
			dcache_size >> 10, c->dcache.ways,
			c->dcache.sets, c->dcache.linesz);
		pr_notice("Secondary unified cache %ldkB, %d-way, %d sets, linesize %d bytes.\n",
			  scache_size >> 10, c->scache.ways,
			  c->scache.sets, c->scache.linesz);
	}
	if (octeon_scache_init)
		octeon_scache_init();
}

static void  octeon_cache_error_setup(void)
{
	if (current_cpu_type() == CPU_CAVIUM_OCTEON3) {
		extern char except_vec2_octeon3;
		set_handler(0x100, &except_vec2_octeon3, 0x80);
	} else {
		extern char except_vec2_octeon;
		set_handler(0x100, &except_vec2_octeon, 0x80);
	}
}

/*
 * Setup the Octeon cache flush routines
 *
 */
void octeon_cache_init(void)
{
	probe_octeon();

	shm_align_mask = PAGE_SIZE - 1;

	flush_cache_all			= octeon_flush_icache_all;
	__flush_cache_all		= octeon_flush_icache_all;
	flush_cache_mm			= octeon_flush_cache_mm;
	flush_cache_page		= octeon_flush_cache_page;
	flush_cache_range		= octeon_flush_cache_range;
	flush_icache_all		= octeon_flush_icache_all;
	flush_data_cache_page		= octeon_flush_data_cache_page;
	flush_icache_range		= octeon_flush_icache_range;
	local_flush_icache_range	= octeon_local_flush_icache_range;
	__flush_icache_user_range	= octeon_flush_icache_range;
	__local_flush_icache_user_range	= octeon_local_flush_icache_range;

	__flush_kernel_vmap_range	= octeon_flush_kernel_vmap_range;

	build_clear_page();
	build_copy_page();

	board_cache_error_setup = octeon_cache_error_setup;
}

int register_co_cache_error_notifier(struct notifier_block *nb)
{
	return raw_notifier_chain_register(&co_cache_error_chain, nb);
}
EXPORT_SYMBOL_GPL(register_co_cache_error_notifier);

int unregister_co_cache_error_notifier(struct notifier_block *nb)
{
	return raw_notifier_chain_unregister(&co_cache_error_chain, nb);
}
EXPORT_SYMBOL_GPL(unregister_co_cache_error_notifier);

static void co_cache_error_call_notifiers(unsigned long val)
{
	int rv = raw_notifier_call_chain(&co_cache_error_chain, val, NULL);
	if ((rv & ~NOTIFY_STOP_MASK) != NOTIFY_OK) {
		u64 dcache_err;
		unsigned long coreid = cvmx_get_core_num();
		u64 icache_err = read_octeon_c0_icacheerr();

		if (val && current_cpu_type() != CPU_CAVIUM_OCTEON3) {
			dcache_err = cache_err_dcache[coreid];
			cache_err_dcache[coreid] = 0;
		} else {
			if (current_cpu_type() == CPU_CAVIUM_OCTEON3) {
				dcache_err = read_octeon_c0_errctl();
				write_octeon_c0_errctl(dcache_err | 1);
			} else {
				dcache_err = read_octeon_c0_dcacheerr();
			}
		}

		pr_err("Core%lu: Cache error exception:\n", coreid);
		pr_err("cp0_errorepc == %lx\n", read_c0_errorepc());
		if (icache_err & 1) {
			pr_err("CacheErr (Icache) == %llx\n",
			       (unsigned long long)icache_err);
			write_octeon_c0_icacheerr(0);
		}
		if (dcache_err & 1) {
			pr_err("CacheErr (Dcache) == %llx\n",
			       (unsigned long long)dcache_err);
		}
	}
}

/*
 * Called when the exception is recoverable
 */

asmlinkage void cache_parity_error_octeon_recoverable(void)
{
	co_cache_error_call_notifiers(CO_CACHE_ERROR_RECOVERABLE);
}

/*
 * Called when the exception is not recoverable
 */

asmlinkage void cache_parity_error_octeon_non_recoverable(void)
{
	co_cache_error_call_notifiers(CO_CACHE_ERROR_UNRECOVERABLE);
	panic("Can't handle cache error: nested exception");
}
