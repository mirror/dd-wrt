/*
 * Copyright (C) 2005 Ted Phelps
 *
 * This is a clock driver for the Geode SCx200's 27MHz high-resolution
 * timer as the system clock replacing its buggy time stamp counter.
 *
 * Based on parts of timer_hpet.c, timer_tsc.c and timer_pit.c.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */

#include <asm/timer.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/seq_file.h>
#include <linux/scx200.h>

#define NAME "scx200hr"

/* Read the clock */
#define SCx200HR_CLOCK() inl(scx200_cb_base + SCx200_TIMER_OFFSET)

/* High-resolution timer configuration address */
#define SCx200_TMCNFG_OFFSET (SCx200_TIMER_OFFSET + 5)

/* Set this bit to disable the 27 MHz input clock */
#define HR_TM27MPD (1 << 2)

/* Set this bit to update the count-up timer once per cycle of the
 * 27MHz timer, clear it to update the timer once every 27 cycles
 * (effectively producing a 1MHz counter) */
#define HR_TMCLKSEL (1 << 1)

/* Set this bit to enable the high-resolution timer interrupt */
#define HR_TMEN (1 << 0)

/* The frequency of the timer.	Change this to 27000000 and set
 * HR_TMCLKSEL in scx200hr_enable to run at the faster clock rate.  At
 * this point in time there is no point in doing so since times are
 * recorded in usec except for the monotonic clock, which is only used
 * by the hangcheck-timer. */
#define HR_FREQ 1000000

/* The number of cycles of the high-resolution timer we expect to see
 * in a single tick.  Note that the result is <<8 for greater precision*/
#define HR_CYCLES_PER_TICK \
    (SH_DIV(HR_FREQ / 1000000 * TICK_NSEC, 1000, 8))

/* The number of cycles of the high-resolution timer we expect to see
 * in one microsecond, <<8 */
#define HR_CYCLES_PER_US ((HR_FREQ / 1000000) << 8)


/* The value of the timer at the last interrupt */
static u32 clock_at_last_interrupt;

/* The number of high-resolution clock cycles beyond what we would
 have expected that the last tick occurred, <<8 for greater precision */
static long clock_delay;

/* The total number of timer nanoseconds between the time the timer
 * went live and the most recent tick. */
static unsigned long long total_ns;

/* A lock to guard access to the monotonic clock-related variables
 * (total_ns and clocal_at_last_interrupt).  Note that these are also
 * protected by the xtime lock. */
static seqlock_t hr_lock = SEQLOCK_UNLOCKED;

/* Nonzero if the timer has been selected */
static int enable_scx200hr;

static int __init scx200hr_init(char *override)
{
	/* Watch for a command-line clock= override */
	if (override[0] && strncmp(override, NAME, sizeof(NAME) - 1) != 0) {
		return -ENODEV;
	}

        /* Note that we should try to enable this timer once the
         * configuration block address is known */
        printk(KERN_WARNING NAME ": timer not yet accessible; will probe later.\n");
	enable_scx200hr = 1;
	return -EAGAIN;
}

/* Called by the timer interrupt.  The xtime_lock will be held. */
static void mark_offset_scx200hr(void)
{
	u32 now, delta;

	/* Avoid races between the interrupt handler and monotonic_clock */
	write_seqlock(&hr_lock);

	/* Determine how many cycles have elapsed since the last interrupt */
	now = SCx200HR_CLOCK();
	delta = (now - clock_at_last_interrupt) << 8;
	clock_at_last_interrupt = now;

	/* Update the total us count and remainder */
	total_ns += (delta * 1000) / HR_CYCLES_PER_US;

	/* The monotonic clock is safe now */
	write_sequnlock(&hr_lock);

	/* Adjust for interrupt handling delay */
	delta += clock_delay;

	/* The high-resolution timer is driven by a different crystal
	 * to the main CPU, so there's no guarantee that the 1KHz
	 * interrupt rate will coincide with the timer.  This keeps
	 * the jiffies count in line with the high-resolution timer,
	 * which makes it possible for NTP to do its magic */
	if (delta < HR_CYCLES_PER_TICK) {
#if 1
		/* Didn't go over 1000us: decrement jiffies to balance
		 * out increment in do_timer.  This will cause some
		 * jitter if the frequency offset is large, as that
		 * adjustment will be applied about 1ms late. */
		jiffies_64--;
		clock_delay = delta;
#else /* !1 */
                clock_delay = 0;
#endif /* 1 */
	} else if (delta < (HR_CYCLES_PER_TICK << 1) + (HR_CYCLES_PER_TICK >> 1)) {
		clock_delay = delta - HR_CYCLES_PER_TICK;
	} else {
		jiffies_64 += delta / HR_CYCLES_PER_TICK - 2;
		clock_delay = HR_CYCLES_PER_TICK + delta % HR_CYCLES_PER_TICK;
	}
}

/* Called by gettimeofday().  Returns the number of microseconds since
 * the last interrupt.	This is called with the xtime_lock held.*/
static unsigned long get_offset_scx200hr(void)
{
	u32 delta;

	/* Get the time now and determine how many cycles have
	 * transpired since the interrupt, adjusting for timer
	 * interrupt jitter. */
	delta = ((SCx200HR_CLOCK() - clock_at_last_interrupt) << 8) + clock_delay;

	/* Convert from cycles<<8 to microseconds */
	return delta / HR_CYCLES_PER_US;
}

/* Returns the number of nanoseconds since the init of the timer. */
static unsigned long long monotonic_clock_scx200hr(void)
{
	u32 delta, seq;
	unsigned long long ns;

	/* This function is *not* called with xtime_lock held, so we
	 * need to get the hr_lock to ensure we're not competing with
	 * mark_offset_scx200hr. */
	do {
		seq = read_seqbegin(&hr_lock);
		ns = total_ns;
		delta = SCx200HR_CLOCK() - clock_at_last_interrupt;
	} while (read_seqretry(&hr_lock, seq));

	/* Convert cycles to microseconds and add. */
	return ns + delta * 1000 / HR_CYCLES_PER_US;
}

/* scx200hr timer_opts struct */
struct timer_opts timer_scx200hr = {
	.name = NAME,
	.mark_offset = mark_offset_scx200hr, 
	.get_offset = get_offset_scx200hr,
	.monotonic_clock = monotonic_clock_scx200hr,
	.delay = NULL
};

/* And the init_timer struct */
struct init_timer_opts __devinitdata timer_scx200hr_init = {
	.init = scx200hr_init,
	.opts = &timer_scx200hr
};


/* Switch from the original timer to the high-resolution timer */
void __devinit scx200hr_timer_enable(void)
{
        /* Make sure the timer was requested and that the
         * configuration block is present */
	if (!enable_scx200hr || !scx200_cb_present()) {
		return;
	}

	/* Reserve the timer region for ourselves */
	if (!request_region(scx200_cb_base + SCx200_TIMER_OFFSET,
			    SCx200_TIMER_SIZE,
			    "NatSemi SCx200 High-Resolution Timer")) {
		printk(KERN_WARNING NAME ": unable to lock timer region\n");
		return;
	}

	/* Configure the timer */
	outb(0, scx200_cb_base + SCx200_TMCNFG_OFFSET);

	/* Record the current value of the timer. */
	clock_at_last_interrupt = SCx200HR_CLOCK();

        /* Get the current value of the monotonic clock */
	total_ns = cur_timer->monotonic_clock();

        /* Switch from the original timer functions to ours, but keep
         * the current delay function since loops_per_jiffy will have
         * been computed using that */
        timer_scx200hr.delay = cur_timer->delay;
	cur_timer = &timer_scx200hr;

	printk(KERN_INFO "switching to scx200 high-resolution timer (%lu cpt)\n",
                HR_CYCLES_PER_TICK);
}
