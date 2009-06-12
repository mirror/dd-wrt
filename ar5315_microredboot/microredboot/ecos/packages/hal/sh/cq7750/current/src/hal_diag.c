//=============================================================================
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
// Author(s):    nickg, jskov
// Contributors: nickg, jskov
// Date:         1999-03-23
// Purpose:      HAL diagnostic output
// Description:  Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_diag.h>           // our header.

#include <cyg/infra/cyg_type.h>         // base types, externC
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_intr.h>           // Interrupt macros
#include <cyg/hal/sh4_scif.h>            // driver API
#include <cyg/hal/hal_misc.h>           // Helper functions

#define SCI_BASE ((cyg_uint8*)0xffe80000)

//-----------------------------------------------------------------------------


void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_scif_init(0, 0, CYGNUM_HAL_INTERRUPT_SCIF_RXI, SCI_BASE);
}

//=============================================================================
// Led control
//=============================================================================
#define LED   0xb4000000

void
hal_diag_led_on( void )
{
    HAL_WRITE_UINT8(LED, 0x18);
}

void
hal_diag_led_off( void )
{
    HAL_WRITE_UINT8(LED, 0);
}

//=============================================================================
// Compatibility with older stubs
//=============================================================================

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#define CYGPRI_HAL_SH_SH4_SCIF_PRIVATE
#include <cyg/hal/sh4_scif.h>            // driver API

static channel_data_t channel = { (cyg_uint8*)SCI_BASE, 0, 0};

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#include <cyg/hal/hal_stub.h>           // hal_output_gdb_string
#endif

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)

#define CYG_HAL_DIAG_GDB

#elif defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

#define CYG_HAL_DIAG_GDB

#endif

//-----------------------------------------------------------------------------

void hal_diag_init(void)
{
    cyg_hal_plf_scif_init_channel(&channel);
}

void 
hal_diag_write_char_serial( char c )
{
    cyg_hal_plf_scif_putc(&channel, c);
}

void
hal_diag_read_char(char *c)
{
    *c = (char) cyg_hal_plf_scif_getc(&channel);
}

// Packet function
void 
hal_diag_write_char(char c)
{
#ifdef CYG_HAL_DIAG_GDB
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
        
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#else
        HAL_DISABLE_INTERRUPTS(old);
#endif
        
        while(1)
        {
            char c1;
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
        
            cyg_hal_plf_scif_putc(&channel, '$');
            cyg_hal_plf_scif_putc(&channel, 'O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                cyg_hal_plf_scif_putc(&channel, h);
                cyg_hal_plf_scif_putc(&channel, l);
                csum += h;
                csum += l;
            }
            cyg_hal_plf_scif_putc(&channel, '#');
            cyg_hal_plf_scif_putc(&channel, hex[(csum>>4)&0xF]);
            cyg_hal_plf_scif_putc(&channel, hex[csum&0xF]);

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.
            c1 = (char) cyg_hal_plf_scif_getc(&channel);

            if( c1 == '+' )
                break;              // a good acknowledge

            // Check for user break.
            if( cyg_hal_is_break( &c1, 1 ) )
                cyg_hal_user_break( NULL );

            // otherwise, loop round again
        }
        
        pos = 0;

        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif
    }
#else // CYG_HAL_DIAG_GDB
    cyg_hal_plf_scif_putc(&channel, c);
#endif
}

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

//-----------------------------------------------------------------------------
// End of hal_diag.c
