/*
 *  linux/include/asm-arm/arch-sl2312/system.h
 *
 *  Copyright (C) 1999 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <asm/arch/platform.h>
#include <asm/arch/hardware.h>
#include <asm/arch/it8712.h>
#include <asm/io.h>

static void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

extern __inline__ void arch_reset(char mode)
{
	__raw_writel( (int) GLOBAL_RESET|RESET_CPU1, IO_ADDRESS(SL2312_GLOBAL_BASE) + GLOBAL_RESET_REG);
}


void (*pm_power_off)(void);
//{
//	printk("arch_power_off\n");

	// Power off
//	__raw_writel( (int) 0x00000001, IO_ADDRESS(SL2312_POWER_CTRL_BASE) + 0x04);

//}

#endif
