/*
 *  PMC-Sierra Inc. Stretch Board 
 *  Author: Manish Lachwani (lachwani@pmc-sierra.com)
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
 *  Setup for the PMC-Sierra Stretch board. Stretch makes processors that are
 *  based off the Xtensa core. 
 *
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/timex.h>
#include <asm/time.h>
#include <asm/page.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/ptrace.h>
#include <asm/reboot.h>
#include <linux/version.h>
#include <linux/bootmem.h>
#include "setup.h"

unsigned long cpu_clock;

extern void pmc_stretch_restart(char *command);
extern void pmc_stretch_halt(void);
extern void pmc_stretch_power_off(void);

/* These functions are used for rebooting or halting the machine*/
extern void pmc_stretch_restart(char *command);
extern void pmc_stretch_halt(void);
extern void pmc_stretch_off(void);

void pmc_stretch_time_init(void);

void __init bus_error_init(void) 
{
	/* Do nothing */ 
}

/* setup code for a handoff from a version 2 PMON 2000 PROM */
void PMON_v2_setup(void)
{
	printk("PMON_v2_setup\n");

}

/* Setup the timer interrupt */
void pmc_stretch_timer_setup(struct irqaction *irq)
{
	setup_irq(8, irq);
}

/* FIXME: get_time and set_time routines */
void pmc_stretch_time_init(void)
{
	mips_counter_frequency = cpu_clock / 2;
	printk("pmc_stretch_time_init cpu_clock=%d\n", cpu_clock);
	board_timer_setup = pmc_stretch_timer_setup;

}

/* Board Setup */
void __init pmc_stretch_setup(void)
{
	unsigned int tmpword;

	board_time_init = pmc_stretch_time_init;

	_machine_restart = pmc_stretch_restart;
	_machine_halt = pmc_stretch_halt;
	_machine_power_off = pmc_stretch_power_off;

	/* do handoff reconfiguration */
	PMON_v2_setup();

	printk("CPU : Rm7000 \n");

	/* 128 MB */
	printk("  - SDRAM size: 128 MB\n");
	add_memory_region(0x0, 0x08000000, BOOT_MEM_RAM);
}
