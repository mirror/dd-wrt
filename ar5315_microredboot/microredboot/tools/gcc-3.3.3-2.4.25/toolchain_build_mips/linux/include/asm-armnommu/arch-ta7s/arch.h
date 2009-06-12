/*
 * include/asm-armnommu/arch-ta7s/arch.h
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall WireSpeed
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Craig Hackney
 * 
 */

#ifndef _ARCH_INIT_H
#include <linux/autoconf.h>
#include <asm/arch/triscend_a7.h>

#ifndef __ASSEMBLER__

extern void ta7_arch_init( void );

#ifdef CONFIG_USE_A7HAL
#include <asm/arch/a7hal/soft/lancs.h>
#else
extern void a7hal_lancs8900_reset( int interface );
#endif

#endif

#endif
