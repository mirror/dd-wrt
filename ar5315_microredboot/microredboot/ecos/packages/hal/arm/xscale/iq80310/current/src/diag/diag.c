//==========================================================================
//
//      diag.c
//
//      Additional RedBoot commands to run board diags.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2000-10-10
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>
#endif
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/hal/hal_tables.h>

#include "iq80310.h"

int pci_config_cycle = 0;	/* skip exception handling when performing pci config cycle */

static void do_hdwr_diag(int argc, char *argv[]);

RedBoot_cmd("diag", 
            "Run board diagnostics", 
            "",
            do_hdwr_diag
    );


void hdwr_diag (void);

void do_hdwr_diag(int arg, char *argv[])
{
    hal_virtual_comm_table_t* __chan;

    // Turn off interrupts on debug channel.
    // All others should already be disabled.
    __chan = CYGACC_CALL_IF_DEBUG_PROCS();
    if (__chan)
	CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_DISABLE);

#ifdef CYGPKG_IO_ETH_DRIVERS
    HAL_INTERRUPT_MASK(eth_drv_int_vector());
#endif

    hdwr_diag();
}

void __disableDCache(void)
{
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
}

void __enableDCache(void)
{
    HAL_DCACHE_ENABLE();
}


void _flushICache(void)
{
    HAL_ICACHE_INVALIDATE_ALL();
}

void __enableICache(void)
{
    HAL_ICACHE_ENABLE();
}

void __disableICache(void)
{
    HAL_ICACHE_DISABLE();
}

void _enableFiqIrq(void)
{
    asm ("mrc p15, 0, r0, c13, c0, 1;"
	 "orr r0, r0, #0x2000;"
	 "mrc p15, 0, r0, c13, c0, 1;"
	 "mrc p13, 0, r0, c0, c0, 0;"
	 "orr		r0, r0, #3;"
	 "mcr	p13, 0, r0, c0, c0, 0;"
	 : : );
}


void _enable_timer(void)
{
    asm("ldr r1, =0x00000005;"
	"mcr p14, 0, r1, c0, c0, 0 ;"
	: : : "r1" );
}

void _disable_timer(void)
{
    asm("ldr r1, =0x00000000;"
	"mcr p14, 0, r1, c0, c0, 0 ;"
	: : : "r1" );
}

void _usec_delay(void)
{
    asm ("ldr	r2, =0x258;"		/* 1 microsec = 600 clocks (600 MHz CPU core) */
	 "0: mrc p14, 0, r0, c1, c0, 0;"	/*read CCNT into r0 */
	 "cmp r2, r0;"	/* compare the current count */
	 "bpl	0b;" /* stay in loop until count is greater */
	 "mrc p14, 0, r1, c0, c0, 0;"
	 "orr	r1, r1, #4;"	/* clear the timer */
	 "mcr p14, 0, r1, c0, c0, 0 ;"
	 : : : "r0","r1","r2");
}

void _msec_delay(void)
{
    asm ("ldr	r2, =0x927c0;"  /* 1 millisec = 600,000 clocks (600 MHz CPU core) */
	 "0: mrc p14, 0, r0, c1, c0, 0;"	/*read CCNT into r0 */
	 "cmp r2, r0;"	/* compare the current count */
	 "bpl	0b;" /* stay in loop until count is greater */
	 "mrc p14, 0, r1, c0, c0, 0;"
	 "orr	r1, r1, #4;"	/* clear the timer */
	 "mcr p14, 0, r1, c0, c0, 0 ;"
	 : : : "r0","r1","r2");
}

unsigned int _read_timer(void)
{
    unsigned x;
    asm("mrc p14, 0, %0, c1, c0, 0;" : "=r"(x) : );
    return x;
}

