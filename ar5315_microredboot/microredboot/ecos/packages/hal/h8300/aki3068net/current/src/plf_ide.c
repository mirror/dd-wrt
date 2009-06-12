//==========================================================================
//
//      plf_ide.c
//
//      HAL platform IDE support function
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    ysato
// Contributors: ysato
// Date:         2003-02-28
// Purpose:      HAL IDE support function
// Description:  This file contains IDE support functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types

#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_endian.h>
#include <cyg/hal/plf_intr.h>
#include <cyg/hal/var_arch.h>

/*------------------------------------------------------------------------*/

#if CYGINT_HAL_PLF_IF_IDE != 0

#define IDE_REG_FEATUERS 1
#define IDE_REG_COMMAND  7

int aki3068net_ide_setup(void)
{
#if CYGHWR_HAL_IDE_BUSWIDTH == 8
    aki3068net_write_command(IDE_REG_FEATUERS,0x01); /* 8bit transfer mode */
    aki3068net_write_command(IDE_REG_COMMAND, 0xef); /* set featuers */
#endif
    return HAL_IDE_NUM_CONTROLLERS;
}

void aki3068net_read_command(cyg_uint16 r, cyg_uint8 *d)
{
#if CYGHWR_HAL_IDE_BUSWIDTH == 8
    *d = *(volatile cyg_uint8 *)(CYGHWR_HAL_IDE_REGISTER + r);
#else
    *d = *(volatile cyg_uint8 *)(CYGHWR_HAL_IDE_REGISTER + (r << 1) + 1);
#endif
}

void aki3068net_read_data   (cyg_uint16 r, cyg_uint16 *d)
{
#if CYGHWR_HAL_IDE_BUSWIDTH == 8
    *d = *(volatile cyg_uint16 *)(CYGHWR_HAL_IDE_REGISTER + r);
#else
    cyg_uint16 dt;
    dt = *(volatile cyg_uint16 *)(CYGHWR_HAL_IDE_REGISTER + (r << 1));
    *d = CYG_LE16_TO_CPU(dt);
#endif
}

void aki3068net_read_control(cyg_uint8 *d)
{
#if CYGHWR_HAL_IDE_BUSWIDTH == 8
    *d = *(volatile cyg_uint8 *)(CYGHWR_HAL_IDE_ALT_REGS + 6);
#else
    *d = *(volatile cyg_uint8 *)(CYGHWR_HAL_IDE_ALT_REGS + 12 + 1);
#endif
}

void aki3068net_write_command(cyg_uint16 r, cyg_uint8 d)
{
#if CYGHWR_HAL_IDE_BUSWIDTH == 8
    *(volatile cyg_uint8 *)(CYGHWR_HAL_IDE_REGISTER + r) = d;
#else
    *(volatile cyg_uint16 *)(CYGHWR_HAL_IDE_REGISTER + (r << 1)) = d;
#endif
}

void aki3068net_write_data   (cyg_uint16 r, cyg_uint16 d)
{
#if CYGHWR_HAL_IDE_BUSWIDTH == 8
    *(volatile cyg_uint8 *)(CYGHWR_HAL_IDE_REGISTER + r) = d;
#else
    *(volatile CYG_WORD16 *)(CYGHWR_HAL_IDE_REGISTER + (r <<1)) = CYG_CPU_TO_LE16(d);
#endif
}

void aki3068net_write_control(cyg_uint8 d)
{
#if CYGHWR_HAL_IDE_BUSWIDTH == 8
    *(volatile cyg_uint8 *)(CYGHWR_HAL_IDE_ALT_REGS + 6) = d;
#else
    *(volatile cyg_uint16 *)(CYGHWR_HAL_IDE_ALT_REGS + 12) = d;
#endif
}

#endif //CYGINT_HAL_PLF_IF_IDE != 0
/*------------------------------------------------------------------------*/
/* End of plf_ide.c                                                       */
