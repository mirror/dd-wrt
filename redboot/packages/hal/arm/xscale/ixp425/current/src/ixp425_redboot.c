//==========================================================================
//
//      ixp425_redboot.c
//
//      RedBoot board support code for Intel IXP425 Network Processor
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.
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
// Date:         2004-08-30
// Purpose:      RedBoot board-specific support
// Description:  Implementations of board-specic RedBoot support.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <redboot.h>
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif

#ifdef CYGOPT_REDBOOT_FIS
extern void *fis_addr;
#endif

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
extern void *cfg_base;
#endif

#ifdef CYGPKG_IO_FLASH

//
// Little endian mode requires some trickery due to the way the IXP4xx
// AHB and expansion busses work.
//
int
hal_flash_read(void *addr, void *data, int len, void **err)
{
    int retval;

    retval = flash_read(addr, data, len, err);

    if (0
#if (CYG_BYTEORDER == CYG_LSBFIRST) && defined(CYGOPT_REDBOOT_FLASH_BYTEORDER_MSBFIRST)
#ifdef CYGOPT_REDBOOT_FIS
	|| addr == fis_addr
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
	|| addr == cfg_base
#endif
#endif
	) {
	cyg_uint32 *p;
	int i;

	for (i = 0, p = data; i < len; i += 4, ++p)
	    *p = CYG_SWAP32(*p);
    }
    return retval;
}

// Try to figure out if RedBoot is re-flashing a RedBoot with
// a different endianess. This is obviously not foolproof, but
// should be good enough.
static inline int
is_swabbed_redboot(void *faddr, cyg_uint32 *p)
{
    if (faddr == (void *)0x50000000
	    && (CYG_SWAP32(p[1]) == 0xe59ff018)
	    && (CYG_SWAP32(p[2]) == 0xe59ff018)
	    && (CYG_SWAP32(p[3]) == 0xe59ff018)
	    && (CYG_SWAP32(p[4]) == 0xe59ff018)
	    && (p[5] == 0))
	return 1;
    return 0;
}

int
hal_flash_program(void *addr, void *data, int len, void **err)
{
    int swabbed = 0;
    cyg_uint32 *p;
    int i, retval;

    if (is_swabbed_redboot(addr, data)
#if (CYG_BYTEORDER == CYG_LSBFIRST) && defined(CYGOPT_REDBOOT_FLASH_BYTEORDER_MSBFIRST)
#ifdef CYGOPT_REDBOOT_FIS
	|| addr == fis_addr
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
	|| addr == cfg_base
#endif
#endif
	) {
	swabbed = 1;
	for (i = 0, p = data; i < len; i += 4, ++p)
	    *p = CYG_SWAP32(*p);
    }

    retval = flash_program(addr, data, len, err);

    if (swabbed) {
	for (i = 0, p = data; i < len; i += 4, ++p)
	    *p = CYG_SWAP32(*p);
    }

    return retval;
}

#endif // CYGPKG_IO_FLASH

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
static const struct {
    unsigned int fuse;
    const char *cpu;
} cpu_name_table[] = {
    { 0x003889bc, "IXP450 533MHz" },
    { 0x007889bc, "IXP450 400MHz" },
    { 0x00f889bc, "IXP450 266MHz" },
    { 0x00f3a41c, "IXP451 266MHz" },
    { 0x00f889a0, "IXP452 266MHz" },
    { 0x00108000, "IXP455 533MHz" },
    { 0x00508000, "IXP455 400MHz" },
    { 0x00d08000, "IXP455 266MHz" },
    { 0x00b809bc, "IXP460 667MHz" },
    { 0x003809bc, "IXP460 533MHz" },
    { 0x007809bc, "IXP460 400MHz" },
    { 0x00f809bc, "IXP460 266MHz" },
    { 0x00800000, "IXP465 667MHz" },
    { 0x00000000, "IXP465 533MHz" },
    { 0x00400000, "IXP465 400MHz" },
    { 0x00c00000, "IXP465 266MHz" },
    { 0xffffffff, "IXP46X/IXP45X" },
};

extern unsigned int _ixp4xx_fuse_val;
#endif

// Return name of CPU
char *
cyg_hal_platform_cpu(void)
{
    unsigned long cpuid, prod_num, prod_rev;

    asm volatile ("mrc p15, 0, %0, c0, c0, 0\n"
		  : "=r" (cpuid) 
		  : );

    if ((cpuid & 0xfffffc00) != 0x69054000)
	return "Unknown";

    prod_num = (cpuid >> 4) & 0x3f;
    prod_rev = cpuid & 0xf;

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    if (prod_num == 0x20 && prod_rev == 0) {
	int i;

	for (i = 0; cpu_name_table[i].fuse != 0xffffffff; i++)
	    if (cpu_name_table[i].fuse == _ixp4xx_fuse_val)
		break;
	
	return cpu_name_table[i].cpu;
    }
#endif

    if (prod_num == 0x1c && prod_rev == 1)
	return "IXP42X 533MHz";

    if (prod_num == 0x1d && prod_rev == 1)
	return "IXP42X 400MHz";

    if (prod_num == 0x1f && prod_rev == 1)
	return "IXP42X 266MHz";

    return "XScale";
}

/*------------------------------------------------------------------------*/
// EOF ixp425_redboot.c

