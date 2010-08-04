/*
 *  linux/arch/arm/mach-cns3xxx/core.h
 *
 *  Copyright (c) 2008 Cavium Networks 
 *  Copyright (C) 2004 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
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

#ifndef __ASM_ARCH_CNS3XXX_H
#define __ASM_ARCH_CNS3XXX_H

void __init cns3xxx_map_io(void);
void cns3xxx_power_off(void);
void __init cns3xxx_init_irq(void);

extern struct sys_timer cns3xxx_timer;
#endif
