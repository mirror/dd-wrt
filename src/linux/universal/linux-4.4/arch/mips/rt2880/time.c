/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     timer setup for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/spinlock.h>

#include <asm/mipsregs.h>
#include <asm/ptrace.h>
#include <asm/hardirq.h>
#include <asm/div64.h>
#include <asm/cpu.h>
#include <asm/time.h>

#include <linux/interrupt.h>
#include <linux/timex.h>
#include <linux/module.h>

#include <asm/rt2880/generic.h>
#include <asm/rt2880/prom.h>
#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboardint.h>

unsigned long surfboard_sysclk;	/* initialized by prom_init_sysclk() */

static unsigned int r4k_offset; /* Amount to increment compare reg each time */
static unsigned int r4k_cur;    /* What counter should be at next timer irq */

extern unsigned int mips_hpt_frequency;
extern u32 mips_cpu_feq;

#define ALLINTS (IE_IRQ0 | IE_IRQ1 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4 | IE_IRQ5)
extern void ralink_gpio_control(int gpio,int level);

void mips_timer_interrupt(void)
{
#ifdef CONFIG_EAP9550
	ralink_gpio_control(11,0);
	ralink_gpio_control(11,1);
#endif
	/*
	if ((timer_tick_count++ % HZ) == 0) {
		mips_display_message(&display_string[display_count++]);
		if (display_count == MAX_DISPLAY_COUNT)
		        display_count = 0;

	}
	*/

	do_IRQ(RALINK_CPU_TIMER_IRQ);
}

/*
 * Figure out the r4k offset, the amount to increment the compare
 * register for each time tick.
 * For SURFBOARD, since there is no RTC present, use the value surfboard_sysclk.
 * surfboard_sysclk by default is set to SURFBOARD_SYSTEM_CLOCK, defined in the
 * file include/asm/surfboard/surfboard.h.  It can be overridden by the using
 * kernel command line option 'sysclk='.
 */
static unsigned int __init cal_r4koff(void)
{
	unsigned long count;
	count = mips_cpu_feq;;
	return (count / HZ);
}

void __init plat_time_init(void)
{
	mips_hpt_frequency = mips_cpu_feq/2;
}

unsigned int __cpuinit get_c0_compare_irq(void)
{
	return CP0_LEGACY_COMPARE_IRQ;
}

/*unsigned int __cpuinit get_c0_compare_int(void)
{
	return RALINK_CPU_TIMER_IRQ;
}*/

u32 get_surfboard_sysclk(void) 
{
	return surfboard_sysclk;
}

EXPORT_SYMBOL(get_surfboard_sysclk);
