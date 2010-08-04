/*
 *  arch/arm/mach-cns3xxx/include/mach/io.h
 *
 *  Copyright (c) 2008 Cavium Networks 
 *  Copyright (C) 2003 ARM Limited
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 */
#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#include "board.h"

#define IO_SPACE_LIMIT 0xffffffff

#if 1
static inline void __iomem *__io(unsigned long addr)
{
  return (void __iomem *)((addr - CNS3XXX_PCIE0_IO_BASE)
          + CNS3XXX_PCIE0_IO_BASE_VIRT);
}
#endif
#define __io(a)     __io(a)
#define __mem_pci(a)		(a)

#endif
