/*
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
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/proc-fns.h>


#if	defined (CONFIG_ARCH_MV88f1181)
#	include "../arch/arm/mach-mv88fxx81/mv88f1181/mvSysHwConfig.h"
	/* versions */
#	define TEST_UBOOT_VER 0x01020400 /* 1.2.4 */
#elif defined (CONFIG_ARCH_MV88f5181)
#       include "../arch/arm/mach-mv88fxx81/mv88f5181/mvSysHwConfig.h"
	/* versions */
#	define TEST_UBOOT_VER 0x01070300 /* 1.7.3 */
#endif

#define LSP_VERSION "1.8.5"


static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

#define CPU_RSTOUTN_MASK_REG                     0x20108
#define CPU_SYS_SOFT_RST_REG                     0x2010C

#ifdef __BIG_ENDIAN
#define MV_ARM_32BIT_LE(X) ((((X)&0xff)<<24) |                       \
                               (((X)&0xff00)<<8) |                      \
                               (((X)&0xff0000)>>8) |                    \
                               (((X)&0xff000000)>>24))
#else
#define MV_ARM_32BIT_LE(X) (X)
#endif

static inline void arch_reset(char mode)
{
	u32 temp;
	printk("Reseting !! \n");
	temp = *(volatile unsigned int*)(INTER_REGS_BASE + CPU_RSTOUTN_MASK_REG);
	temp |= MV_ARM_32BIT_LE(0x04);
	*(volatile unsigned int*)(INTER_REGS_BASE + CPU_RSTOUTN_MASK_REG) = temp;

        temp = *(volatile unsigned int*)(INTER_REGS_BASE + CPU_SYS_SOFT_RST_REG);
	temp |= MV_ARM_32BIT_LE(0x1);
        *(volatile unsigned int*)(INTER_REGS_BASE + CPU_SYS_SOFT_RST_REG) = temp;
}

#endif
