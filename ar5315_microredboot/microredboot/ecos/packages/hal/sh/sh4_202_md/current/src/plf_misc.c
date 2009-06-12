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
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
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
// Author(s):    nickg
// Contributors: nickg
// Date:         2003-08-20
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_if.h>             // interfacing API
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_io.h>

#include <cyg/hal/hal_intr.h>

//--------------------------------------------------------------------------

void
hal_platform_init(void)
{
    /* Enable the Ethernet interrupt on the INTC on the CPU-board FPGA */
    
    volatile cyg_uint32* const intDisableReg = (cyg_uint32*)FPGA_INTDSB_REG;    
    volatile cyg_uint32* const intPriorityReg = (cyg_uint32*)FPGA_INTPRI_REG(FPGA_ETHERNET_INT);

    *intDisableReg = 0;
    
    /* Set the interrupt priority level for the Ethenet chip on the FPGA */
    *intPriorityReg &= ~FPGA_INTPRI_MASK(FPGA_ETHERNET_INT);
    *intPriorityReg |= FPGA_INTPRI_LEVEL(FPGA_ETHERNET_INT,ETHERNET_INT_PRIORITY);
    
    hal_if_init();
}

//--------------------------------------------------------------------------
// eof plf_misc.c
