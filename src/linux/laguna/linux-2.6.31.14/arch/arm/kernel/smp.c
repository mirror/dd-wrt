/*
 *  linux/arch/arm/kernel/smp.c
 *
 *  Copyright (C) 2002 ARM Limited, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/cache.h>
#include <linux/profile.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/percpu.h>
#include <linux/clockchips.h>

#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/cpu.h>
#include <asm/cputype.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/processor.h>
#include <asm/tlbflush.h>
#include <asm/ptrace.h>
#include <asm/localtimer.h>

/*
 * as from 2.5, kernels no longer have an init_tasks structure
 * so we need some other way of telling a new secondary core
 * where to place its SVC stack
 */
struct secondary_data secondary_data;

/*
 * structures for inter-processor calls
 * - A collection of single bit ipi messages.
 */
struct ipi_data {
	spinlock_t lock;
	unsigned long ipi_count;
	unsigned long bits;
};

static DEFINE_PER_CPU(struct ipi_data, ipi_data) = {
	.lock	= SPIN_LOCK_UNLOCKED,
};

#ifdef CONFIG_CPU_NO_CACHE_BCAST_DEBUG
static DEFINE_PER_CPU(unsigned long,dma_cache_counter) = 0;
unsigned long bcache_bitmap = 0;
#endif

enum ipi_msg_type {
	IPI_TIMER,
	IPI_RESCHEDULE,
	IPI_CALL_FUNC,
	IPI_CALL_FUNC_SINGLE,
	IPI_CPU_STOP,
#ifdef CONFIG_CPU_NO_CACHE_BCAST
	IPI_DMA_CACHE,
#endif
};

int __cpuinit __cpu_up(unsigned int cpu)
{
	struct cpuinfo_arm *ci = &per_cpu(cpu_data, cpu);
	struct task_struct *idle = ci->idle;
	pgd_t *pgd;
	pmd_t *pmd;
	int ret;

	/*
	 * Spawn a new process manually, if not already done.
	 * Grab a pointer to its task struct so we can mess with it
	 */
	if (!idle) {
		idle = fork_idle(cpu);
		if (IS_ERR(idle)) {
			printk(KERN_ERR "CPU%u: fork() failed\n", cpu);
			return PTR_ERR(idle);
		}
		ci->idle = idle;
	}

	/*
	 * Allocate initial page tables to allow the new CPU to
	 * enable the MMU safely.  This essentially means a set
	 * of our "standard" page tables, with the addition of
	 * a 1:1 mapping for the physical address of the kernel.
	 */
	pgd = pgd_alloc(&init_mm);
	pmd = pmd_offset(pgd + pgd_index(PHYS_OFFSET), PHYS_OFFSET);
	*pmd = __pmd((PHYS_OFFSET & PGDIR_MASK) |
		     PMD_TYPE_SECT | PMD_SECT_AP_WRITE);
	flush_pmd_entry(pmd);

	/*
	 * We need to tell the secondary core where to find
	 * its stack and the page tables.
	 */
	secondary_data.stack = task_stack_page(idle) + THREAD_START_SP;
	secondary_data.pgdir = virt_to_phys(pgd);
	wmb();

	/*
	 * Now bring the CPU into our world.
	 */
	ret = boot_secondary(cpu, idle);
	if (ret == 0) {
		unsigned long timeout;

		/*
		 * CPU was successfully started, wait for it
		 * to come online or time out.
		 */
		timeout = jiffies + HZ;
		while (time_before(jiffies, timeout)) {
			if (cpu_online(cpu))
				break;

			udelay(10);
			barrier();
		}

		if (!cpu_online(cpu))
			ret = -EIO;
	}

	secondary_data.stack = NULL;
	secondary_data.pgdir = 0;

	*pmd = __pmd(0);
	clean_pmd_entry(pmd);
	pgd_free(&init_mm, pgd);

	if (ret) {
		printk(KERN_CRIT "CPU%u: processor failed to boot\n", cpu);

		/*
		 * FIXME: We need to clean up the new idle thread. --rmk
		 */
	}

	return ret;
}

#ifdef CONFIG_HOTPLUG_CPU
/*
 * __cpu_disable runs on the processor to be shutdown.
 */
int __cpuexit __cpu_disable(void)
{
	unsigned int cpu = smp_processor_id();
	struct task_struct *p;
	int ret;

	ret = mach_cpu_disable(cpu);
	if (ret)
		return ret;

	/*
	 * Take this CPU offline.  Once we clear this, we can't return,
	 * and we must not schedule until we're ready to give up the cpu.
	 */
	set_cpu_online(cpu, false);

	/*
	 * OK - migrate IRQs away from this CPU
	 */
	migrate_irqs();

	/*
	 * Stop the local timer for this CPU.
	 */
	local_timer_stop();

	/*
	 * Flush user cache and TLB mappings, and then remove this CPU
	 * from the vm mask set of all processes.
	 */
	flush_cache_all();
	local_flush_tlb_all();

	read_lock(&tasklist_lock);
	for_each_process(p) {
		if (p->mm)
			cpu_clear(cpu, p->mm->cpu_vm_mask);
	}
	read_unlock(&tasklist_lock);

	return 0;
}

/*
 * called on the thread which is asking for a CPU to be shutdown -
 * waits until shutdown has completed, or it is timed out.
 */
void __cpuexit __cpu_die(unsigned int cpu)
{
	if (!platform_cpu_kill(cpu))
		printk("CPU%u: unable to kill\n", cpu);
}

/*
 * Called from the idle thread for the CPU which has been shutdown.
 *
 * Note that we disable IRQs here, but do not re-enable them
 * before returning to the caller. This is also the behaviour
 * of the other hotplug-cpu capable cores, so presumably coming
 * out of idle fixes this.
 */
void __cpuexit cpu_die(void)
{
	unsigned int cpu = smp_processor_id();

	local_irq_disable();
	idle_task_exit();

	/*
	 * actual CPU shutdown procedure is at least platform (if not
	 * CPU) specific
	 */
	platform_cpu_die(cpu);

	/*
	 * Do not return to the idle loop - jump back to the secondary
	 * cpu initialisation.  There's some initialisation which needs
	 * to be repeated to undo the effects of taking the CPU offline.
	 */
	__asm__("mov	sp, %0\n"
	"	b	secondary_start_kernel"
		:
		: "r" (task_stack_page(current) + THREAD_SIZE - 8));
}
#endif /* CONFIG_HOTPLUG_CPU */

/*
 * This is the secondary CPU boot entry.  We're using this CPUs
 * idle thread stack, but a set of temporary page tables.
 */
asmlinkage void __cpuinit secondary_start_kernel(void)
{
	struct mm_struct *mm = &init_mm;
	unsigned int cpu = smp_processor_id();

	printk("CPU%u: Booted secondary processor\n", cpu);

	/*
	 * All kernel threads share the same mm context; grab a
	 * reference and switch to it.
	 */
	atomic_inc(&mm->mm_users);
	atomic_inc(&mm->mm_count);
	current->active_mm = mm;
	cpu_set(cpu, mm->cpu_vm_mask);
	cpu_switch_mm(mm->pgd, mm);
	enter_lazy_tlb(mm, current);
	local_flush_tlb_all();

	cpu_init();
	preempt_disable();

	/*
	 * Give the platform a chance to do its own initialisation.
	 */
	platform_secondary_init(cpu);

	/*
	 * Enable local interrupts.
	 */
	notify_cpu_starting(cpu);
	local_irq_enable();
	local_fiq_enable();

	/*
	 * Setup the percpu timer for this CPU.
	 */
	percpu_timer_setup();

	calibrate_delay();

	smp_store_cpu_info(cpu);

	/*
	 * OK, now it's safe to let the boot CPU continue
	 */
	set_cpu_online(cpu, true);

	/*
	 * OK, it's off to the idle thread for us
	 */
	cpu_idle();
}

/*
 * Called by both boot and secondaries to move global data into
 * per-processor storage.
 */
void __cpuinit smp_store_cpu_info(unsigned int cpuid)
{
	struct cpuinfo_arm *cpu_info = &per_cpu(cpu_data, cpuid);

	cpu_info->loops_per_jiffy = loops_per_jiffy;
}

void __init smp_cpus_done(unsigned int max_cpus)
{
	int cpu;
	unsigned long bogosum = 0;

	for_each_online_cpu(cpu)
		bogosum += per_cpu(cpu_data, cpu).loops_per_jiffy;

	printk(KERN_INFO "SMP: Total of %d processors activated "
	       "(%lu.%02lu BogoMIPS).\n",
	       num_online_cpus(),
	       bogosum / (500000/HZ),
	       (bogosum / (5000/HZ)) % 100);
}

void __init smp_prepare_boot_cpu(void)
{
	unsigned int cpu = smp_processor_id();

	per_cpu(cpu_data, cpu).idle = current;
}

static void send_ipi_message(const struct cpumask *mask, enum ipi_msg_type msg)
{
	unsigned long flags;
	unsigned int cpu;

	local_irq_save(flags);

	for_each_cpu(cpu, mask) {
		struct ipi_data *ipi = &per_cpu(ipi_data, cpu);

		spin_lock(&ipi->lock);
		ipi->bits |= 1 << msg;
		spin_unlock(&ipi->lock);
	}

	/*
	 * Call the platform specific cross-CPU call function.
	 */
	smp_cross_call(mask);
	local_irq_restore(flags);
}

static void send_ipi_message_cache(const struct cpumask *mask)
{
	unsigned long flags;

	local_irq_save(flags);
	smp_cross_call_cache(mask);
	local_irq_restore(flags);
}
void arch_send_call_function_ipi_mask(const struct cpumask *mask)
{
	send_ipi_message(mask, IPI_CALL_FUNC);
}

void arch_send_call_function_single_ipi(int cpu)
{
	send_ipi_message(cpumask_of(cpu), IPI_CALL_FUNC_SINGLE);
}

void show_ipi_list(struct seq_file *p)
{
	unsigned int cpu;

	seq_puts(p, "IPI:");

	for_each_present_cpu(cpu)
		seq_printf(p, " %10lu", per_cpu(ipi_data, cpu).ipi_count);

	seq_putc(p, '\n');

#ifdef CONFIG_CPU_NO_CACHE_BCAST_DEBUG
	seq_puts(p, " dc: ");
	for_each_present_cpu(cpu)
		seq_printf(p, " %10lu", per_cpu(dma_cache_counter, cpu));
	seq_putc(p, '\n');
#endif
}

void show_local_irqs(struct seq_file *p)
{
	unsigned int cpu;

	seq_printf(p, "LOC: ");

	for_each_present_cpu(cpu)
		seq_printf(p, "%10u ", irq_stat[cpu].local_timer_irqs);

	seq_putc(p, '\n');
}

/*
 * Timer (local or broadcast) support
 */
static DEFINE_PER_CPU(struct clock_event_device, percpu_clockevent);

static void ipi_timer(void)
{
	struct clock_event_device *evt = &__get_cpu_var(percpu_clockevent);
	irq_enter();
	evt->event_handler(evt);
	irq_exit();
}

#ifdef CONFIG_LOCAL_TIMERS
asmlinkage void __exception do_local_timer(struct pt_regs *regs)
{
	struct pt_regs *old_regs = set_irq_regs(regs);
	int cpu = smp_processor_id();

	if (local_timer_ack()) {
		irq_stat[cpu].local_timer_irqs++;
		ipi_timer();
	}

	set_irq_regs(old_regs);
}
#endif

#ifdef CONFIG_GENERIC_CLOCKEVENTS_BROADCAST
static void smp_timer_broadcast(const struct cpumask *mask)
{
	send_ipi_message(mask, IPI_TIMER);
}

static void broadcast_timer_set_mode(enum clock_event_mode mode,
	struct clock_event_device *evt)
{
}

static void local_timer_setup(struct clock_event_device *evt)
{
	evt->name	= "dummy_timer";
	evt->features	= CLOCK_EVT_FEAT_ONESHOT |
			  CLOCK_EVT_FEAT_PERIODIC |
			  CLOCK_EVT_FEAT_DUMMY;
	evt->rating	= 400;
	evt->mult	= 1;
	evt->set_mode	= broadcast_timer_set_mode;
	evt->broadcast	= smp_timer_broadcast;

	clockevents_register_device(evt);
}
#endif

void __cpuinit percpu_timer_setup(void)
{
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *evt = &per_cpu(percpu_clockevent, cpu);

	evt->cpumask = cpumask_of(cpu);

	local_timer_setup(evt);
}

static DEFINE_SPINLOCK(stop_lock);

/*
 * ipi_cpu_stop - handle IPI from smp_send_stop()
 */
static void ipi_cpu_stop(unsigned int cpu)
{
	spin_lock(&stop_lock);
	printk(KERN_CRIT "CPU%u: stopping\n", cpu);
	dump_stack();
	spin_unlock(&stop_lock);

	set_cpu_online(cpu, false);

	local_fiq_disable();
	local_irq_disable();

	while (1)
		cpu_relax();
}

#ifdef CONFIG_CPU_NO_CACHE_BCAST
static void ipi_dma_cache_op(unsigned int cpu);
#endif

/*
 * Main handler for inter-processor interrupts
 *
 * For ARM, the ipimask now only identifies a single
 * category of IPI (Bit 1 IPIs have been replaced by a
 * different mechanism):
 *
 *  Bit 0 - Inter-processor function call
 */
asmlinkage void __exception do_IPI(struct pt_regs *regs)
{
	unsigned int cpu = smp_processor_id();
	struct ipi_data *ipi = &per_cpu(ipi_data, cpu);
	struct pt_regs *old_regs = set_irq_regs(regs);

	ipi->ipi_count++;

	for (;;) {
		unsigned long msgs;

		spin_lock(&ipi->lock);
		msgs = ipi->bits;
		ipi->bits = 0;
		spin_unlock(&ipi->lock);

		if (!msgs)
			break;

		do {
			unsigned nextmsg;

			nextmsg = msgs & -msgs;
			msgs &= ~nextmsg;
			nextmsg = ffz(~nextmsg);

			switch (nextmsg) {
			case IPI_TIMER:
				ipi_timer();
				break;

			case IPI_RESCHEDULE:
				/*
				 * nothing more to do - eveything is
				 * done on the interrupt return path
				 */
				break;

			case IPI_CALL_FUNC:
				generic_smp_call_function_interrupt();
				break;

			case IPI_CALL_FUNC_SINGLE:
				generic_smp_call_function_single_interrupt();
				break;

			case IPI_CPU_STOP:
				ipi_cpu_stop(cpu);
				break;

#ifdef CONFIG_CPU_NO_CACHE_BCAST
			case IPI_DMA_CACHE:
#ifdef CONFIG_CPU_NO_CACHE_BCAST_DEBUG
				//get_cpu_var(dma_cache_counter)++;
				//put_cpu_var(dma_cache_counter);
#endif
				ipi_dma_cache_op(cpu);
				break;
#endif

			default:
				printk(KERN_CRIT "CPU%u: Unknown IPI message 0x%x\n",
				       cpu, nextmsg);
				break;
			}
		} while (msgs);
	}

	set_irq_regs(old_regs);
}

asmlinkage void __exception do_cache_IPI(struct pt_regs *regs)
{
	unsigned int cpu = smp_processor_id();
	struct ipi_data *ipi = &per_cpu(ipi_data, cpu);
	struct pt_regs *old_regs = set_irq_regs(regs);

	ipi->ipi_count++;

	ipi_dma_cache_op(cpu);

	set_irq_regs(old_regs);
}

void smp_send_reschedule(int cpu)
{
	send_ipi_message(cpumask_of(cpu), IPI_RESCHEDULE);
}

void smp_send_stop(void)
{
	cpumask_t mask = cpu_online_map;
	cpu_clear(smp_processor_id(), mask);
	send_ipi_message(&mask, IPI_CPU_STOP);
}

/*
 * not supported here
 */
int setup_profiling_timer(unsigned int multiplier)
{
	return -EINVAL;
}

static void
on_each_cpu_mask(void (*func)(void *), void *info, int wait,
		const struct cpumask *mask)
{
	preempt_disable();

	smp_call_function_many(mask, func, info, wait);
	if (cpumask_test_cpu(smp_processor_id(), mask))
		func(info);

	preempt_enable();
}

/**********************************************************************/

/*
 * TLB operations
 */
struct tlb_args {
	struct vm_area_struct *ta_vma;
	unsigned long ta_start;
	unsigned long ta_end;
};

/* all SMP configurations have the extended CPUID registers */
static inline int tlb_ops_need_broadcast(void)
{
	return ((read_cpuid_ext(CPUID_EXT_MMFR3) >> 12) & 0xf) < 2;
}

static inline void ipi_flush_tlb_all(void *ignored)
{
	local_flush_tlb_all();
}

static inline void ipi_flush_tlb_mm(void *arg)
{
	struct mm_struct *mm = (struct mm_struct *)arg;

	local_flush_tlb_mm(mm);
}

static inline void ipi_flush_tlb_page(void *arg)
{
	struct tlb_args *ta = (struct tlb_args *)arg;

	local_flush_tlb_page(ta->ta_vma, ta->ta_start);
}

static inline void ipi_flush_tlb_kernel_page(void *arg)
{
	struct tlb_args *ta = (struct tlb_args *)arg;

	local_flush_tlb_kernel_page(ta->ta_start);
}

static inline void ipi_flush_tlb_range(void *arg)
{
	struct tlb_args *ta = (struct tlb_args *)arg;

	local_flush_tlb_range(ta->ta_vma, ta->ta_start, ta->ta_end);
}

static inline void ipi_flush_tlb_kernel_range(void *arg)
{
	struct tlb_args *ta = (struct tlb_args *)arg;

	local_flush_tlb_kernel_range(ta->ta_start, ta->ta_end);
}

void flush_tlb_all(void)
{
	if (tlb_ops_need_broadcast())
		on_each_cpu(ipi_flush_tlb_all, NULL, 1);
	else
		local_flush_tlb_all();
}

void flush_tlb_mm(struct mm_struct *mm)
{
	if (tlb_ops_need_broadcast())
		on_each_cpu_mask(ipi_flush_tlb_mm, mm, 1, &mm->cpu_vm_mask);
	else
		local_flush_tlb_mm(mm);
}

void flush_tlb_page(struct vm_area_struct *vma, unsigned long uaddr)
{
	if (tlb_ops_need_broadcast()) {
		struct tlb_args ta;
		ta.ta_vma = vma;
		ta.ta_start = uaddr;
		on_each_cpu_mask(ipi_flush_tlb_page, &ta, 1, &vma->vm_mm->cpu_vm_mask);
	} else
		local_flush_tlb_page(vma, uaddr);
}

void flush_tlb_kernel_page(unsigned long kaddr)
{
	if (tlb_ops_need_broadcast()) {
		struct tlb_args ta;
		ta.ta_start = kaddr;
		on_each_cpu(ipi_flush_tlb_kernel_page, &ta, 1);
	} else
		local_flush_tlb_kernel_page(kaddr);
}

void flush_tlb_range(struct vm_area_struct *vma,
                     unsigned long start, unsigned long end)
{
	if (tlb_ops_need_broadcast()) {
		struct tlb_args ta;
		ta.ta_vma = vma;
		ta.ta_start = start;
		ta.ta_end = end;
		on_each_cpu_mask(ipi_flush_tlb_range, &ta, 1, &vma->vm_mm->cpu_vm_mask);
	} else
		local_flush_tlb_range(vma, start, end);
}

void flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
	if (tlb_ops_need_broadcast()) {
		struct tlb_args ta;
		ta.ta_start = start;
		ta.ta_end = end;
		on_each_cpu(ipi_flush_tlb_kernel_range, &ta, 1);
	} else
		local_flush_tlb_kernel_range(start, end);
}

#ifdef CONFIG_CPU_NO_CACHE_BCAST
/*
 * DMA cache maintenance operations on SMP if the automatic hardware
 * broadcasting is not available
 */
struct smp_dma_cache_struct {
	int type;
	const void *start;
	const void *end;
	cpumask_t unfinished;
};

static struct smp_dma_cache_struct *smp_dma_cache_data;
static DEFINE_RWLOCK(smp_dma_cache_data_lock);
static DEFINE_SPINLOCK(smp_dma_cache_lock);

static void local_dma_cache_op(int type, const void *start, const void *end)
{
	switch (type) {
	case SMP_DMA_CACHE_INV:
		dmac_inv_range(start, end);
		break;
	case SMP_DMA_CACHE_CLEAN:
		dmac_clean_range(start, end);
		break;
	case SMP_DMA_CACHE_FLUSH:
		dmac_flush_range(start, end);
		break;
	default:
		printk(KERN_CRIT "CPU%u: Unknown SMP DMA cache type %d\n",
		       smp_processor_id(), type);
	}
}

/*
 * This function must be executed with interrupts disabled.
 */
static void ipi_dma_cache_op(unsigned int cpu)
{
	read_lock(&smp_dma_cache_data_lock);

	/* check for spurious IPI */
	if ((smp_dma_cache_data == NULL) ||
	    (!cpu_isset(cpu, smp_dma_cache_data->unfinished)))
		goto out;
	local_dma_cache_op(smp_dma_cache_data->type,
			   smp_dma_cache_data->start, smp_dma_cache_data->end);
	cpu_clear(cpu, smp_dma_cache_data->unfinished);
 out:
	read_unlock(&smp_dma_cache_data_lock);
}

/*
 * Execute the DMA cache operations on all online CPUs. This function
 * can be called with interrupts disabled or from interrupt context.
 */
static void __smp_dma_cache_op(int type, const void *start, const void *end)
{
	struct smp_dma_cache_struct data;
	cpumask_t callmap = cpu_online_map;
	unsigned int cpu = get_cpu();
	unsigned long flags;

	cpu_clear(cpu, callmap);
	data.type = type;
	data.start = start;
	data.end = end;
	data.unfinished = callmap;

	/*
	 * If the spinlock cannot be acquired, other CPU is trying to
	 * send an IPI. If the interrupts are disabled, we have to
	 * poll for an incoming IPI.
	 */
	while (!spin_trylock_irqsave(&smp_dma_cache_lock, flags)) {
		if (irqs_disabled())
			ipi_dma_cache_op(cpu);
	}

	write_lock(&smp_dma_cache_data_lock);
	smp_dma_cache_data = &data;
	write_unlock(&smp_dma_cache_data_lock);

	if (!cpus_empty(callmap))
		send_ipi_message(&callmap, IPI_DMA_CACHE);
	/* run the local operation in parallel with the other CPUs */
	local_dma_cache_op(type, start, end);

	while (!cpus_empty(data.unfinished))
		barrier();

	write_lock(&smp_dma_cache_data_lock);
	smp_dma_cache_data = NULL;
	write_unlock(&smp_dma_cache_data_lock);

	spin_unlock_irqrestore(&smp_dma_cache_lock, flags);
	put_cpu();
}

#define DMA_MAX_RANGE		SZ_4K

/*
 * Split the cache range in smaller pieces if interrupts are enabled
 * to reduce the latency caused by disabling the interrupts during the
 * broadcast.
 */
void smp_dma_cache_op(int type, const void *start, const void *end)
{
	if (irqs_disabled() || (end - start <= DMA_MAX_RANGE))
		__smp_dma_cache_op(type, start, end);
	else {
		const void *ptr;
		for (ptr = start; ptr < end - DMA_MAX_RANGE;
		     ptr += DMA_MAX_RANGE)
			__smp_dma_cache_op(type, ptr, ptr + DMA_MAX_RANGE);
		__smp_dma_cache_op(type, ptr, end);
	}
}
#endif
