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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-03-02
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/hal_intr.h>

#include <cyg/hal/hal_io.h>

#include <cyg/hal/plf_stub.h>


#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)

#define CYG_KERNEL_DIAG_GDB

#endif

/*---------------------------------------------------------------------------*/

void hal_diag_init()
{
    cyg_hal_plf_comms_init();
}

/*---------------------------------------------------------------------------*/

void hal_diag_write_char(char c)
{
#ifdef CYG_KERNEL_DIAG_GDB    
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        CYG_INTERRUPT_STATE oldstate;
        HAL_DISABLE_INTERRUPTS(oldstate);
        
        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
            char c1;
        
            cyg_hal_plf_serial_putc(NULL, '$');
            cyg_hal_plf_serial_putc(NULL, 'O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                cyg_hal_plf_serial_putc(NULL, h);
                cyg_hal_plf_serial_putc(NULL, l);
                csum += h;
                csum += l;
            }
            cyg_hal_plf_serial_putc(NULL, '#');
            cyg_hal_plf_serial_putc(NULL, hex[(csum>>4)&0xF]);
            cyg_hal_plf_serial_putc(NULL, hex[csum&0xF]);

            c1 = cyg_hal_plf_serial_getc( NULL );

            if( c1 == '+' ) break;

            if( cyg_hal_is_break( &c1 , 1 ) )
                cyg_hal_user_break( NULL );    

        }
        
        pos = 0;

        // Wait for all data from serial line to drain
        // and clear ready-to-send indication.
//        hal_diag_drain_serial0();

        // Disabling the interrupts for an extended period of time
        // can provoke a spurious interrupt 0. FIXME - why?
#ifdef CYGSEM_HAL_MIPS_VR4300_VRC437X_DIAG_ACKS_INT_0
        HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_VRC437X );
#endif
        
        // And re-enable interrupts
        HAL_RESTORE_INTERRUPTS( oldstate );
        
    }
#else
    cyg_hal_plf_serial_putc(NULL, c);
#endif    
}

/*---------------------------------------------------------------------------*/

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(NULL);
}

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
