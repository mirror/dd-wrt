/*
 *  linux/include/asm-arm/arch-epxa10/hardware.h
 *
 *  This file contains the hardware definitions of the Integrator.
 *
 *  Copyright (C) 1999 ARM Limited.
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
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/arch/platform.h>

#define pcibios_assign_all_busses()	1

/*
 * Where in virtual memory the IO devices (timers, system controllers
 * and so on)
 *
 * macro to get at IO space when running virtually
*/

#define IO_ADDRESS(x)      (((x&0xfff00000)>>4)|(x & 0x000fffff)|0xF0000000)
#define FLASH_VBASE         0xFE000000
#define FLASH_SIZE 0x1000000// 8M
#define FLASH_START         SL2312_FLASH_BASE
#define FLASH_VADDR(x)      ((x & 0x00ffffff)|0xFE000000)       // flash virtual address

#define PCIBIOS_MIN_IO					0x100		// 0x000-0x100 AHB reg and PCI config, data
#define PCIBIOS_MIN_MEM					0

#endif

