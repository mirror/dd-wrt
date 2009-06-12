//==========================================================================
//
//      mpc50_misc.c
//
//      HAL misc board support code for MPC 5.0
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    <knud.woehler@microplex.de>
// Date:         2003-01-06
//
//####DESCRIPTIONEND####
//
//========================================================================*/
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_pxa2x0.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/mpc50.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_mm.h>


void hal_mmu_init(void)
{
#ifdef CYG_HAL_STARTUP_ROM

    typedef cyg_uint32 aptr;        // for arithmetic

	unsigned long ttb_base = PXA2X0_RAM_BANK0_BASE + 0x4000;
	unsigned long *SDRAMConfig;
    int *p_hdsize = (int *)((aptr)(&hal_dram_size) | (0xA00u *SZ_1M));
    int *p_hdtype = (int *)((aptr)(&hal_dram_type) | (0xA00u *SZ_1M));

	*p_hdtype = 0;
	for(SDRAMConfig=0; *SDRAMConfig != MPC50_VAL_MAGIC ;SDRAMConfig++);
	SDRAMConfig += (MPC50_VAL_OFFS_MDCNFG>>2);

	// set TTB Register
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base));

	// erase Page Table
	memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

	// create Page Table

	/*               Actual			Virtual	        Size	Attribute                                                  Function    */
	/*	             Base			Base			MB      cached?          buffered?         access permissions                  */
	/*               xxx00000       xxx00000                                                                                       */
	X_ARM_MMU_SECTION(0x000,		0x500,			16,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Boot flash ROMspace */
	X_ARM_MMU_SECTION(0x040,		0x600,			64,		ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* FPGA Register */
	X_ARM_MMU_SECTION(0x080,		0x680,			64,		ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* LAN Register */
	X_ARM_MMU_SECTION(0x400,		0x400,			192,	ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PXA2x0 Register */

	if((*SDRAMConfig & 0x00000018) == 0x08)	// 32MB per bank
	{
	X_ARM_MMU_SECTION(0xA00,		0x0,			32,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 0 */
	X_ARM_MMU_SECTION(0xA40,		0x020,			32,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 1 */
    *p_hdsize = 64 * SZ_1M; // 64MB
	if((*SDRAMConfig & 0x00030000) != 0)	// 4 banks 
	{
	X_ARM_MMU_SECTION(0xA80,		0x040,			32,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 2 */
	X_ARM_MMU_SECTION(0xAc0,		0x060,			32,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 3 */
	*p_hdsize = 128 * SZ_1M; // 128MB 	
	}
	}else{				// 64MB per bank
	X_ARM_MMU_SECTION(0xA00,		0x0,			64,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 0 */
	X_ARM_MMU_SECTION(0xA40,		0x040,			64,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 1 */
	*p_hdsize = 128 * SZ_1M; // 128MB 	
	if((*SDRAMConfig & 0x00030000) != 0)	// 4 banks 
	{
	X_ARM_MMU_SECTION(0xA80,		0x080,			64,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 2 */
	X_ARM_MMU_SECTION(0xAc0,		0x0c0,			64,		ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* SDRAM Bank 3 */
	*p_hdsize = 256 * SZ_1M; // 256MB 	
	}
	}
	X_ARM_MMU_SECTION(0xA00,		0xA00,			256,	ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Unmapped Memory */

	X_ARM_MMU_SECTION(0xC00,		0xC00,			128,	ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); 
#endif
}

void plf_hardware_init(void)
{
	int t;
	unsigned long int gpioValue ;

	// enable FFUART clock
	gpioValue = *PXA2X0_CKEN ;
	gpioValue = gpioValue | (unsigned long)0x40 ;
	*PXA2X0_CKEN = gpioValue ;

	hal_if_init();				// Set up eCos/ROM interfaces

	mpc50_user_hardware_init();

	return ;
}

#include CYGHWR_MEMORY_LAYOUT_H
typedef void code_fun(void);
void mpc50_program_new_stack(void *func)
{
    register CYG_ADDRESS stack_ptr asm("sp");
    register CYG_ADDRESS old_stack asm("r4");
    register code_fun *new_func asm("r0");
    old_stack = stack_ptr;
    stack_ptr = CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - sizeof(CYG_ADDRESS);
    new_func = (code_fun*)func;
    new_func();
    stack_ptr = old_stack;
    return;
}

//
// Memory layout
//
externC cyg_uint8 *
hal_arm_mem_real_region_top( cyg_uint8 *regionend )
{
    CYG_ASSERT( hal_dram_size > 0, "Didn't detect DRAM size!" );
    CYG_ASSERT( hal_dram_size <=  256<<20, "More than 256MB reported - that can't be right" );
    CYG_ASSERT( 0 == (hal_dram_size & 0xfffff), "hal_dram_size not whole Mb" );
    // is it the "normal" end of the DRAM region? If so, it should be
    // replaced by the real size
    if ( regionend == ((cyg_uint8 *)CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE) )
	{
        regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
    return regionend;
}

void mpc50_user_hardware_init(void) CYGBLD_ATTRIB_WEAK;

void mpc50_user_hardware_init()
{
}



/*------------------------------------------------------------------------*/
// EOF mpc50_misc.c
