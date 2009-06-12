//==========================================================================
//
//      var_misc.c
//
//      HAL implementation miscellaneous functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Author(s):    pfine
// Contributors: jskov, gthomas
// Date:         2001-12-12
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_mem.h>
#include <cyg/hal/mpc8xxx.h>

volatile t_PQ2IMM  *IMM;   /* IMM base pointer */

//--------------------------------------------------------------------------
void hal_variant_init(void)
{
    IMM =  (t_PQ2IMM *)CYGARC_IMM_BASE;  /* MPC8xxx internal register map  */
#ifndef CYGSEM_HAL_USE_ROM_MONITOR
    // Reset CPM
    _mpc8xxx_reset_cpm();
#endif
}


//--------------------------------------------------------------------------
// Variant specific idle thread action.
bool
hal_variant_idle_thread_action( cyg_uint32 count )
{
    // Let architecture idle thread action run
    return true;
}

//---------------------------------------------------------------------------
// Use MMU resources to map memory regions.  
// Takes and returns an int used to ID the MMU resource to use. This ID
// is increased as resources are used and should be used for subsequent
// invocations.
int
cyg_hal_map_memory (int id,CYG_ADDRESS virt, CYG_ADDRESS phys, 
                    cyg_int32 size, cyg_uint8 flags)
{
    // Use BATs to map the memory.
    cyg_uint32 ubat, lbat;

    ubat = (virt & UBAT_BEPIMASK) | UBAT_VS | UBAT_VP;
    lbat = (phys & LBAT_BRPNMASK);
    if (flags & CYGARC_MEMDESC_CI) 
        lbat |= LBAT_I;
    if (flags & CYGARC_MEMDESC_GUARDED) 
        lbat |= LBAT_G;
#define IWASPATRICK
#ifdef IWASPATRICK
    lbat |= LBAT_PP_RW; // Always enable for Read-Write
#else
    if (flags & CYGARC_MEMDESC_RO) // Memory is Read Only
        lbat |= LBAT_PP_RO;
    if (flags & CYGARC_MEMDESC_RW) // Memory is RW 
        lbat |= LBAT_PP_RW;
#endif
    // There are 4 BATs, size is programmable.
    while (id < 4 && size > 0) {
        cyg_uint32 blk_size = 128*1024;
        cyg_uint32 bl = 0;
        while (blk_size < 256*1024*1024 && blk_size < size) {
            blk_size *= 2;
            bl = (bl << 1) | 1;
        }
        ubat = (ubat & ~UBAT_BLMASK) | (bl << 2);
        
        switch (id) {
        case 0:
            CYGARC_MTSPR (IBAT0U, ubat);
            CYGARC_MTSPR (IBAT0L, lbat);
            CYGARC_MTSPR (DBAT0U, ubat);
            CYGARC_MTSPR (DBAT0L, lbat);
            break;
        case 1:
            CYGARC_MTSPR (IBAT1U, ubat);
            CYGARC_MTSPR (IBAT1L, lbat);
            CYGARC_MTSPR (DBAT1U, ubat);
            CYGARC_MTSPR (DBAT1L, lbat);
            break;
        case 2:
            CYGARC_MTSPR (IBAT2U, ubat);
            CYGARC_MTSPR (IBAT2L, lbat);
            CYGARC_MTSPR (DBAT2U, ubat);
            CYGARC_MTSPR (DBAT2L, lbat);
            break;
        case 3:
            CYGARC_MTSPR (IBAT3U, ubat);
            CYGARC_MTSPR (IBAT3L, lbat);
            CYGARC_MTSPR (DBAT3U, ubat);
            CYGARC_MTSPR (DBAT3L, lbat);
            break;
        }
        
        size -= blk_size;
        id++;
    }

    return id;
}


// Initialize MMU to a sane (NOP) state.
void
cyg_hal_clear_MMU (void)
{
    cyg_uint32 ubat, lbat;
        
    // Initialize BATs with 0 -- VS&VP are unset, making all matches fail
    ubat = 0;
    lbat = 0;

    CYGARC_MTSPR (IBAT0U, ubat);
    CYGARC_MTSPR (IBAT0L, lbat);
    CYGARC_MTSPR (DBAT0U, ubat);
    CYGARC_MTSPR (DBAT0L, lbat);
    CYGARC_MTSPR (IBAT1U, ubat);
    CYGARC_MTSPR (IBAT1L, lbat);
    CYGARC_MTSPR (DBAT1U, ubat);
    CYGARC_MTSPR (DBAT1L, lbat);
    CYGARC_MTSPR (IBAT2U, ubat);
    CYGARC_MTSPR (IBAT2L, lbat);
    CYGARC_MTSPR (DBAT2U, ubat);
    CYGARC_MTSPR (DBAT2L, lbat);
    CYGARC_MTSPR (IBAT3U, ubat);
    CYGARC_MTSPR (IBAT3L, lbat);
    CYGARC_MTSPR (DBAT3U, ubat);
    CYGARC_MTSPR (DBAT3L, lbat);
}

//--------------------------------------------------------------------------
// End of var_misc.c
