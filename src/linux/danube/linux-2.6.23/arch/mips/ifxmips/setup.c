/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2004 peng.liu@infineon.com 
 *   Copyright (C) 2007 John Crispin <blogic@openwrt.org> 
 */

#include <linux/init.h>

#include <asm/time.h>
#include <asm/traps.h>
#include <asm/cpu.h>
#include <asm/irq.h>
#include <asm/bootinfo.h>
#include <asm/ifxmips/ifxmips.h>
#include <asm/ifxmips/ifxmips_irq.h>
#include <asm/ifxmips/ifxmips_pmu.h>
#include <asm/ifxmips/ifxmips_cgu.h>
#include <asm/ifxmips/ifxmips_prom.h>

static unsigned int r4k_offset;
static unsigned int r4k_cur;

extern void ifxmips_reboot_setup(void);

unsigned int
ifxmips_get_cpu_ver(void)
{
	return (ifxmips_r32(IFXMIPS_MPS_CHIPID) & 0xF0000000) >> 28;
}
EXPORT_SYMBOL(ifxmips_get_cpu_ver);

void
__init bus_error_init (void)
{
		/* nothing yet */
}

static __inline__ u32
ifxmips_get_counter_resolution(void)
{
	u32 res;
	__asm__ __volatile__(
		".set   push\n"
		".set   mips32r2\n"
		".set   noreorder\n"
		"rdhwr  %0, $3\n"
		"ehb\n"
		".set pop\n"
		: "=&r" (res)
		: /* no input */
		: "memory");
		instruction_hazard();
		return res;
}



/* ISR GPTU Timer 6 for high resolution timer */
static irqreturn_t
danube_timer6_interrupt(int irq, void *dev_id)
{
	timer_interrupt(IFXMIPS_TIMER6_INT, NULL);

	return IRQ_HANDLED;
}

static struct irqaction hrt_irqaction = {
	.handler = danube_timer6_interrupt,
	.flags = IRQF_DISABLED,
	.name = "hrt",
};

/*
 * THe CPU counter for System timer, set to HZ
 * GPTU Timer 6 for high resolution timer, set to hr_time_resolution
 * Also misuse this routine to print out the CPU type and clock.
 */
void __init
plat_timer_setup (struct irqaction *irq)
{
	unsigned int retval;

	setup_irq(MIPS_CPU_TIMER_IRQ, irq);

	r4k_cur = (read_c0_count() + r4k_offset);
	write_c0_compare(r4k_cur);

	writel(readl(IFXMIPS_PMU_PWDCR) & ~(IFXMIPS_PMU_PWDCR_GPT|IFXMIPS_PMU_PWDCR_FPI), IFXMIPS_PMU_PWDCR);

	writel(0x100, IFXMIPS_GPTU_GPT_CLC);

	writel(0xffff, IFXMIPS_GPTU_GPT_CAPREL);
	writel(0x80C0, IFXMIPS_GPTU_GPT_T6CON);

	retval = setup_irq(IFXMIPS_TIMER6_INT, &hrt_irqaction);

	if (retval)
	{
		prom_printf("reqeust_irq failed %d. HIGH_RES_TIMER is diabled\n", IFXMIPS_TIMER6_INT);
	}
}

void
danube_time_init (void)
{
	mips_hpt_frequency = ifxmips_get_cpu_hz() / ifxmips_get_counter_resolution();
	r4k_offset = mips_hpt_frequency / HZ;
	printk("mips_hpt_frequency:%d\n", mips_hpt_frequency);
	printk("r4k_offset: %08x(%d)\n", r4k_offset, r4k_offset);
}

int
danube_be_handler(struct pt_regs *regs, int is_fixup)
{
	/*TODO*/
	printk(KERN_ERR "TODO: BUS error\n");

	return MIPS_BE_FATAL;
}

void __init
plat_mem_setup(void)
{
	u32 status;
	prom_printf("This %s has a cpu rev of 0x%X\n", get_system_type(), ifxmips_get_cpu_ver());

	status = read_c0_status();
	status &= (~(1<<25));
	write_c0_status(status);

	ifxmips_reboot_setup();
	board_time_init = danube_time_init;
	board_be_handler = &danube_be_handler;

	ioport_resource.start = IOPORT_RESOURCE_START;
	ioport_resource.end = IOPORT_RESOURCE_END;
	iomem_resource.start = IOMEM_RESOURCE_START;
	iomem_resource.end = IOMEM_RESOURCE_END;
}
