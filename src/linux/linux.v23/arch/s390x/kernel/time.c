/*
 *  arch/s390/kernel/time.c
 *
 *  S390 version
 *    Copyright (C) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *    Author(s): Hartmut Penner (hp@de.ibm.com),
 *               Martin Schwidefsky (schwidefsky@de.ibm.com),
 *               Denis Joseph Barrow (djbarrow@de.ibm.com,barrow_dj@yahoo.com)
 *
 *  Derived from "arch/i386/kernel/time.c"
 *    Copyright (C) 1991, 1992, 1995  Linus Torvalds
 */

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/types.h>

#include <asm/uaccess.h>
#include <asm/delay.h>

#include <linux/timex.h>
#include <linux/config.h>

#include <asm/irq.h>
#include <asm/s390_ext.h>

/* change this if you have some constant time drift */
#define USECS_PER_JIFFY     ((unsigned long) 1000000/HZ)
#define CLK_TICKS_PER_JIFFY ((unsigned long) USECS_PER_JIFFY << 12)

/*
 * Create a small time difference between the timer interrupts
 * on the different cpus to avoid lock contention.
 */
#define CPU_DEVIATION       (smp_processor_id() << 12)

#define TICK_SIZE tick

static ext_int_info_t ext_int_info_timer;
static u64 init_timer_cc;
static u64 xtime_cc;

extern rwlock_t xtime_lock;
extern unsigned long wall_jiffies;

void tod_to_timeval(__u64 todval, struct timeval *xtime)
{
        todval >>= 12;
        xtime->tv_sec = todval / 1000000;
        xtime->tv_usec = todval % 1000000;
}

static inline unsigned long do_gettimeoffset(void) 
{
	__u64 now;

	asm volatile ("STCK 0(%0)" : : "a" (&now) : "memory", "cc");
        now = (now - init_timer_cc) >> 12;
	/* We require the offset from the latest update of xtime */
	now -= (__u64) wall_jiffies*USECS_PER_JIFFY;
	return (unsigned long) now;
}

/*
 * This version of gettimeofday has microsecond resolution.
 */
void do_gettimeofday(struct timeval *tv)
{
	unsigned long flags;
	unsigned long usec, sec;

	read_lock_irqsave(&xtime_lock, flags);
	sec = xtime.tv_sec;
	usec = xtime.tv_usec + do_gettimeoffset();
	read_unlock_irqrestore(&xtime_lock, flags);

	while (usec >= 1000000) {
		usec -= 1000000;
		sec++;
	}

	tv->tv_sec = sec;
	tv->tv_usec = usec;
}

void do_settimeofday(struct timeval *tv)
{

	write_lock_irq(&xtime_lock);
	/* This is revolting. We need to set the xtime.tv_usec
	 * correctly. However, the value in this location is
	 * is value at the last tick.
	 * Discover what correction gettimeofday
	 * would have done, and then undo it!
	 */
	tv->tv_usec -= do_gettimeoffset();

	while (tv->tv_usec < 0) {
		tv->tv_usec += 1000000;
		tv->tv_sec--;
	}

	xtime = *tv;
	time_adjust = 0;		/* stop active adjtime() */
	time_status |= STA_UNSYNC;
	time_maxerror = NTP_PHASE_LIMIT;
	time_esterror = NTP_PHASE_LIMIT;
	write_unlock_irq(&xtime_lock);
}

/*
 * timer_interrupt() needs to keep up the real-time clock,
 * as well as call the "do_timer()" routine every clocktick
 */
void account_ticks(struct pt_regs *regs)
{
	int cpu = smp_processor_id();
	__u64 tmp;
	__u32 ticks;

	/* Calculate how many ticks have passed. */
	tmp = S390_lowcore.int_clock - S390_lowcore.jiffy_timer;
	if (tmp >= 2*CLK_TICKS_PER_JIFFY) {  /* more than one tick ? */
		ticks = tmp / CLK_TICKS_PER_JIFFY + 1;
		S390_lowcore.jiffy_timer +=
			CLK_TICKS_PER_JIFFY * (__u64) ticks;
	} else if (tmp > CLK_TICKS_PER_JIFFY) {
		ticks = 2;
		S390_lowcore.jiffy_timer += 2*CLK_TICKS_PER_JIFFY;
	} else {
		ticks = 1;
		S390_lowcore.jiffy_timer += CLK_TICKS_PER_JIFFY;
	}

	/* set clock comparator for next tick */
	tmp = S390_lowcore.jiffy_timer + CPU_DEVIATION;
	asm volatile ("SCKC %0" : : "m" (tmp));

	irq_enter(cpu, 0);

#ifdef CONFIG_SMP
	/*
	 * Do not rely on the boot cpu to do the calls to do_timer.
	 * Spread it over all cpus instead.
	 */
	write_lock(&xtime_lock);
	if (S390_lowcore.jiffy_timer > xtime_cc) {
		__u32 xticks;

		tmp = S390_lowcore.jiffy_timer - xtime_cc;
		if (tmp >= 2*CLK_TICKS_PER_JIFFY) {
			xticks = tmp / CLK_TICKS_PER_JIFFY;
			xtime_cc += (__u64) xticks * CLK_TICKS_PER_JIFFY;
		} else {
			xticks = 1;
			xtime_cc += CLK_TICKS_PER_JIFFY;
		}
		while (xticks--)
			do_timer(regs);
	}
	write_unlock(&xtime_lock);
	while (ticks--)
		update_process_times(user_mode(regs));
#else
	while (ticks--)
		do_timer(regs);
#endif

	irq_exit(cpu, 0);
}

/*
 * Start the clock comparator on the current CPU
 */
void init_cpu_timer(void)
{
        unsigned long cr0;
	__u64 timer;

        timer = init_timer_cc + (__u64) jiffies * CLK_TICKS_PER_JIFFY;
        S390_lowcore.jiffy_timer = timer + CLK_TICKS_PER_JIFFY;
        timer += CLK_TICKS_PER_JIFFY + CPU_DEVIATION;
        asm volatile ("SCKC %0" : : "m" (timer));
        /* allow clock comparator timer interrupt */
        asm volatile ("STCTG 0,0,%0" : "=m" (cr0) : : "memory");
        cr0 |= 0x800;
        asm volatile ("LCTLG 0,0,%0" : : "m" (cr0) : "memory");
}

/*
 * Initialize the TOD clock and the CPU timer of
 * the boot cpu.
 */
void __init time_init(void)
{
        __u64 set_time_cc;
	int cc;

        /* kick the TOD clock */
        asm volatile ("STCK 0(%1)\n\t"
                      "IPM  %0\n\t"
                      "SRL  %0,28" : "=r" (cc) : "a" (&init_timer_cc) 
				   : "memory", "cc");
        switch (cc) {
        case 0: /* clock in set state: all is fine */
                break;
        case 1: /* clock in non-set state: FIXME */
                printk("time_init: TOD clock in non-set state\n");
                break;
        case 2: /* clock in error state: FIXME */
                printk("time_init: TOD clock in error state\n");
                break;
        case 3: /* clock in stopped or not-operational state: FIXME */
                printk("time_init: TOD clock stopped/non-operational\n");
                break;
        }

	/* set xtime */
	xtime_cc = init_timer_cc + CLK_TICKS_PER_JIFFY;
        set_time_cc = init_timer_cc - 0x8126d60e46000000LL +
                      (0x3c26700LL*1000000*4096);
        tod_to_timeval(set_time_cc, &xtime);

        /* request the 0x1004 external interrupt */
        if (register_early_external_interrupt(0x1004, NULL,
					      &ext_int_info_timer) != 0)
                panic("Couldn't request external interrupt 0x1004");

        /* init CPU timer */
        init_cpu_timer();
}
