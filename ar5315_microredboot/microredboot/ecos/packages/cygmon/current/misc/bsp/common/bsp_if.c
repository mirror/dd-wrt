//==========================================================================
//
//      bsp_if.c
//
//      Miscellaneous BSP Interfaces.
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
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Miscellaneous BSP Interfaces.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <bsp/bsp.h>
#include "bsp_if.h"

/*
 * Install a debug handler.
 * Returns old handler being replaced.
 */
bsp_handler_t
bsp_install_dbg_handler(bsp_handler_t new_handler)
{
    bsp_handler_t old_handler;

    old_handler = *bsp_shared_data->__dbg_vector;
    *bsp_shared_data->__dbg_vector = new_handler;

    return old_handler;
}

/*
 * Sometimes it is desireable to call the debug handler directly. This routine
 * accomplishes that. It is the responsibility of the caller to insure that
 * interrupts are disabled before calling this routine.
 */
void
bsp_invoke_dbg_handler(int exc_nr, void *regs)
{
    (*bsp_shared_data->__dbg_vector)(exc_nr, regs);
}

/*
 * Install a 'kill' handler.
 * Returns old handler being replaced.
 */
bsp_handler_t
bsp_install_kill_handler(bsp_handler_t new_handler)
{
    bsp_handler_t old_handler;

    old_handler = bsp_shared_data->__kill_vector;
    bsp_shared_data->__kill_vector = new_handler;

    return old_handler;
}


void *
bsp_cpu_data(void)
{
  return bsp_shared_data->__cpu_data;
}


void *
bsp_board_data(void)
{
    return bsp_shared_data->__board_data;
}


int
bsp_sysinfo(enum bsp_info_id id, ...)
{
    int     retval;
    va_list ap;

    va_start (ap, id);
    retval = bsp_shared_data->__sysinfo(id, ap);
    va_end(ap);
    return retval;
}

int
bsp_set_debug_comm(int id)
{
    return bsp_shared_data->__set_debug_comm(id);
}

int
bsp_set_console_comm(int id)
{
    return bsp_shared_data->__set_console_comm(id);
}

int
bsp_set_serial_baud(int id, int baud)
{
    return bsp_shared_data->__set_serial_baud(id, baud);
}


#if !defined(NDEBUG)

void _bsp_assert(const char *file, const int line, const char *condition)
{
    bsp_printf("Assertion \"%s\" failed\n", condition);
    bsp_printf("File \"%s\"\n", file);
    bsp_printf("Line %d\n", line);
#if defined(PORT_TOGGLE_DEBUG)
    PORT_TOGGLE_DEBUG();
#else
    while(1) ;
#endif /* defined(PORT_TOGGLE_DEBUG) */
}

#endif /* !defined(NDEBUG) */

