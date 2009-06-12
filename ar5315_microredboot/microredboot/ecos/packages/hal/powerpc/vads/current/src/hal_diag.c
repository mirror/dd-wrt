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
// Contributors:hmt, jskov
// Date:        1999-06-08
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt macros

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#include <cyg/hal/hal_stub.h>           // hal_output_gdb_string
#endif

#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/mpc8260.h>
#ifdef CYGPKG_HAL_QUICC
#include <cyg/hal/quicc/quicc_smc1.h>
#endif

// This prototype probably needs to be in an include file
void
cyg_hal_plf_serial_init(void);


void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;
    initialized = 1;

/* This is where I will put the SCC1 initialization code */
#define PF_ADDING_VIRTUAL_VECTORS
#ifdef PF_ADDING_VIRTUAL_VECTORS
    cyg_hal_plf_serial_init();
#else
# ifdef CYGPKG_HAL_QUICC
    cyg_hal_plf_serial_init();
# endif
#endif
}


#ifdef LATER
#if !defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)

//-----------------------------------------------------------------------------
// Select default diag channel to use

//#define CYG_KERNEL_DIAG_ROMART
//#define CYG_KERNEL_DIAG_SERIAL

#if !defined(CYG_KERNEL_DIAG_SERIAL)
#define CYG_KERNEL_DIAG_SERIAL
#endif

#ifdef CYGDBG_DIAG_BUF
// Keep diag messages in a buffer for later [re]display

int enable_diag_uart = 1;
int enable_diag_buf = 1;
static char diag_buf[40960*4];
static int  diag_buf_ptr = 0;

static void
diag_putc(char c)
{
    if (enable_diag_buf) {
        diag_buf[diag_buf_ptr++] = c;
        if (diag_buf_ptr == sizeof(diag_buf)) diag_buf_ptr--;
    }
}

void
dump_diag_buf(int start, int len)
{
    int i;
    enable_diag_uart = 1;
    enable_diag_buf = 0;
    if (len == 0) len = diag_buf_ptr;
    diag_printf("\nDiag buf\n");
    for (i = start;  i < len;  i++) {
        hal_diag_write_char(diag_buf[i]);
    }
}
#endif // CYGDBG_DIAG_BUF


//-----------------------------------------------------------------------------
// MBX board specific serial output; using GDB protocol by default:


#if defined(CYG_KERNEL_DIAG_SERIAL)

t_PQ2IMM *IMM;

void hal_diag_init(void)
{
    static int init = 0;
    if (init) return;
    init++;

    // hardwired base
    //eppc = eppc_base();
    IMM = (t_PQ2IMM *) 0x04700000;

    // init the actual serial port
    cyg_hal_plf_serial_init_channel();
#ifdef CYGSEM_HAL_DIAG_MANGLER_GDB
#ifndef CYG_HAL_STARTUP_ROM
    // We are talking to GDB; ack the "go" packet!
    cyg_hal_plf_serial_putc('+');
#endif
#endif
}

void hal_diag_write_char_serial( char c )
{
    unsigned long __state;
    HAL_DISABLE_INTERRUPTS(__state);
    cyg_hal_plf_serial_putc(c);
    HAL_RESTORE_INTERRUPTS(__state);
}

#if defined(CYG_HAL_STARTUP_ROM) || !defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
void hal_diag_write_char(char c)
{
#ifdef CYGDBG_DIAG_BUF
    diag_putc(c);
    if (!enable_diag_uart) return;
#endif // CYGDBG_DIAG_BUF
    hal_diag_write_char_serial(c);
}

#else // RAM start so encode for GDB

void hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

#ifdef CYGDBG_DIAG_BUF
    diag_putc(c);
    if (!enable_diag_uart) return;
#endif // CYGDBG_DIAG_BUF

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
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
        
            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

#ifndef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            // only gobble characters if no interrupt handler to grab ^Cs
            // is installed (which is exclusive with device driver use)

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            c = cyg_hal_plf_serial_getc(eppc);

            if( c == '+' )
                break;              // a good acknowledge
#if 0
            if( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                breakpoint();
                break;
            }
#endif
            // otherwise, loop round again
#else
            break;
#endif
        }
        
        pos = 0;

        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif
        
    }
}
#endif // NOT def CYG_HAL_STARTUP_ROM

#if 0
// These should not need to be prototyped here, but where????
cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data);

void 
cyg_hal_plf_serial_putc(void* __ch_data, cyg_uint8 ch);
#endif

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc();
}

#endif // CYG_KERNEL_DIAG_SERIAL

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG
#endif

// EOF hal_diag.c
