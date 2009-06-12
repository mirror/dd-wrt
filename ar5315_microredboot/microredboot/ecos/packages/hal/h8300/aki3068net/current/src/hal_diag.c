/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
// Author(s):   ysato
// Contributors: ysato
// Date:        2002-04-05
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/hal/hal_diag.h>
#include <cyg/hal/h8_sci.h>
#include <cyg/hal/var_intr.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_misc.h>

#define SCI_BASE ((cyg_uint8*)CYGHAL_PLF_SCI_BASE)

static channel_data_t channel = { (cyg_uint8*)SCI_BASE, 0, 0};

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_sci_init(0, 0, CYGNUM_HAL_INTERRUPT_RXI1, SCI_BASE);
}

//=============================================================================
// Led control
//=============================================================================
#define LED   CYGARC_PADR

void
hal_diag_led_on( void )
{
    HAL_WRITE_UINT8(LED, 0x01);
}

void
hal_diag_led_off( void )
{
    HAL_WRITE_UINT8(LED, 0);
}

//=============================================================================
// Compatibility with older stubs
//=============================================================================

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#include <cyg/hal/hal_stub.h>           // hal_output_gdb_string
#endif

void hal_diag_init(void)
{
    cyg_hal_plf_sci_init_channel(&channel);
}

void 
hal_diag_write_char( cyg_uint8 c )
{
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
        
        while(1)
        {
            char c1;
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
        
            cyg_hal_plf_sci_putc(&channel, '$');
            cyg_hal_plf_sci_putc(&channel, 'O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                cyg_hal_plf_sci_putc(&channel, h);
                cyg_hal_plf_sci_putc(&channel, l);
                csum += h;
                csum += l;
            }
            cyg_hal_plf_sci_putc(&channel, '#');
            cyg_hal_plf_sci_putc(&channel, hex[(csum>>4)&0xF]);
            cyg_hal_plf_sci_putc(&channel, hex[csum&0xF]);

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.
            c1 = (char) cyg_hal_plf_sci_getc(&channel);

            if( c1 == '+' )
                break;              // a good acknowledge

            // Check for user break.
            if( cyg_hal_is_break( &c1, 1 ) )
                cyg_hal_user_break( NULL );

            // otherwise, loop round again
        }
        
        pos = 0;

        // And re-enable interrupts
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
    }
#else // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
    cyg_hal_plf_sci_putc(&channel, c);
#endif
}

void
hal_diag_read_char(cyg_uint8 *c)
{
    *c = (char) cyg_hal_plf_sci_getc(&channel);
}

/*===========================================================================*/
/* EOF hal_diag.c                                                            */

