//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    adrian
// Contributors: based on existing files for other OS
// Date:         2003-10-20
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>

#include <cyg/hal/ar7100_soc.h>

typedef cyg_uint32 u32;
typedef cyg_uint16 u16;
typedef cyg_uint8  u8;

#include "redboot.h"

RedBoot_cmd("ar7100_reg_rd",
            "Read AR7100 register", "ar7100_reg_r <reg_addr>",
            do_ath_reg_rd
    );

static void
do_ath_reg_rd (int argc, char *argv[])
{
    unsigned long addr;
    char delim = '\0';
    if (argc != 2) {
        diag_printf ("usage: ar7100_reg_rd <hex_addr>\n");
        return;
    }
    if (parse_num (argv[1], &addr, 0, 0) == 0) {
        diag_printf ("invalid addr\n");
        return;
    }
    diag_printf ("reg %x value = %x\n", addr, ar7100_reg_rd(addr));
}

RedBoot_cmd("ar7100_reg_wr",
            "Write AR7100 register", "ar7100_reg_wr <reg_addr> <val>",
            do_ath_reg_wr
    );

static void
do_ath_reg_wr (int argc, char *argv[])
{
    unsigned long addr;
    unsigned long value;
    if (argc != 3) {
        diag_printf ("usage: ar7100_reg_wr <hex_addr> <val>\n");
        return;
    }
    if (parse_num(argv[1], &addr, 0, 0) == 0) {
        diag_printf ("invalid addr\n");
        return;
    }
    if (parse_num(argv[2], &value, 0, 0) == 0) {
        diag_printf ("invalid value\n");
        return;
    }
    ar7100_reg_wr(addr, value);
}



void
hal_platform_init(void)
{
    // Board specific initialization
    hal_ar7100_board_init();
    // Set up eCos/ROM interfaces
    hal_if_init();

#if 0
    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
#endif
}


/*
 * We extract the pll divider, multiply it by the base freq 40.
 * The cpu and ahb are divided off of that.
 */
unsigned int
hal_ar7100_sys_frequency(void)
{
    u32 ar7100_cpu_freq, pll, pll_div, cpu_div, ahb_div, freq;
    static u32 ar7100_ahb_freq=0;
      
    if (ar7100_ahb_freq)
        return ar7100_ahb_freq;

    pll = ar7100_reg_rd(AR7100_PLL_CONFIG);

    pll_div  = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK) + 1;
    freq     = pll_div * 40000000;
    cpu_div  = ((pll >> CPU_DIV_SHIFT) & CPU_DIV_MASK) + 1;
    ahb_div  = (((pll >> AHB_DIV_SHIFT) & AHB_DIV_MASK) + 1)*2;

    ar7100_cpu_freq = freq/cpu_div;
    ar7100_ahb_freq = ar7100_cpu_freq/ahb_div;

    return ar7100_ahb_freq;
}


/*
 * Fetch a pointer to an ethernet's MAC address
 * in the Board Configuration data (in flash).
 */
unsigned char *
enet_mac_address_get(int unit)
{
    /* TBD: get address from flash */
    return NULL;
}

/*------------------------------------------------------------------------*/
/* Reset support                                                          */

void
hal_ar7100_reset(void)
{
    /* Check if this is correct */
    for(;;) {
        ar7100_reg_wr(AR7100_RESET, AR7100_RESET_FULL_CHIP);
    }
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
