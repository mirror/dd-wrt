/*
 * include/asm-armnommu/arch-netarm/hardware.h
 *
 * Copyright (C) 2000 NETsilicon, Inc.
 * Copyright (C) 2000 WireSpeed Communications Corporation
 *
 * This software is copyrighted by WireSpeed. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
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
 * author(s) : Joe deBlaquiere
 */

#ifndef __ASM_ARCH_NETARM_HARDWARE_H
#define __ASM_ARCH_NETARM_HARDWARE_H
#include <asm/arch/netarm_registers.h> 
/*
 * What hardware must be present
 */
#ifndef __ASSEMBLER__

typedef unsigned long u_32;

/*
 * RAM definitions
 */
#define MAPTOPHYS(a)		((unsigned long)a)
#define KERNTOPHYS(a)		((unsigned long)(&a))
#define GET_MEMORY_END(p)	((p->u1.s.page_size) * (p->u1.s.nr_pages))
#define PARAMS_BASE			0x1000
/*#define KERNEL_BASE		(PAGE_OFFSET + 0x80000)*/

#endif

/*
 * HARD_RESET_NOW -- used in blkmem.c. Should call arch_hard_reset(), but I 
 * don't appear to have one ;).
 * --gmcnutt
 */
#if !defined(CONFIG_NETARM_NS7520) 
#define HARD_RESET_NOW()
#else  /* This might actually work for other platforms */
#define HARD_RESET_NOW() {		\
    cli();				\
    *(get_gen_reg_addr( NETARM_GEN_PLL_CONTROL )) &= 0x0000ffff; \
    mdelay( 100 );  \
    *(get_gen_reg_addr( NETARM_GEN_SYSTEM_CONTROL )) |= \
            (NETARM_GEN_SYS_CFG_WDOG_EN|NETARM_GEN_SYS_CFG_WDOG_RST| \
             NETARM_GEN_SYS_CFG_WDOG_24); \
}

#endif

#endif

