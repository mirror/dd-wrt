//=============================================================================
//
//      hal_aux.c
//
//      HAL auxiliary objects and code; per platform
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):   hmt
// Contributors:hmt
// Date:        1999-06-08
// Purpose:     HAL aux objects: startup tables.
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#include <cyg/infra/cyg_type.h>

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    // Mapping for the Motorola MPC8260 development board
    CYGARC_MEMDESC_CACHE( 0x00000000, 0x01000000 ), // Main memory 60x SDRAM
    CYGARC_MEMDESC_NOCACHEGUARD( 0x04000000, 0x00400000 ), // Local Bus SDRAM
    // The BCSR are actually only 8 registers, but they repeatedly map
    // into the 1 MB space described here.
    // The BAT register uses a Block Length of 4MB so that both the BCSRs
    // and the IMMR space is Managed thru the MMU.  In fact, the ATM UNI
    // Proc. Control is also mapped into this block.
    // See MPC8260 PowerQUICC II ADS User's Manual, p. 3-35, note 2
    CYGARC_MEMDESC_NOCACHE( 0x04500000, 0x00400000 ), // BCSR registers
    CYGARC_MEMDESC_NOCACHE( 0xff800000, 0x00800000 ), // ROM region

    CYGARC_MEMDESC_TABLE_END
};

static volatile CYG_WORD  *BCSR0 = (CYG_WORD *)0x04500000; 
static volatile CYG_WORD  *BCSR1 = (CYG_WORD *)0x04500004;
static volatile CYG_WORD  *BCSR2 = (CYG_WORD *)0x04500008;

// Some macros used for debugging
#define GREEN_LED_ON  (*BCSR0 &= 0xFDFFFFFF)
#define RED_LED_OFF   (*BCSR0 |= 0x01000000)
#define GREEN_LED_OFF (*BCSR0 |= 0x02000000)
#define RED_LED_ON    (*BCSR0 &= 0xFEFFFFFF)

//--------------------------------------------------------------------------
// Platform init code.
void
hal_platform_init(void)
{
   /*----------------------------------------------------------------------*/
   /* Enable RS232 interface on the VADS board via BCSR1. BCSR1 is a Board */
   /* Control and Status Register that resides in a programmable logic     */
   /* device.                                                              */    
   /*----------------------------------------------------------------------*/

   //*BCSR1 &= 0xFFFFFFFD;  /* Assert the RS232EN_1/ bit */
   /* The Pilot revision of the MPC8260ADS board has moved the 8 bits
    * in the 32 bit BCSR0 and BCSR1 from D24-D31 to D0-D7.
    */
   *BCSR1 &= 0xFDFFFFFF;  /* Assert the RS232EN_1/ bit */

    hal_if_init();
}

// EOF hal_aux.c
