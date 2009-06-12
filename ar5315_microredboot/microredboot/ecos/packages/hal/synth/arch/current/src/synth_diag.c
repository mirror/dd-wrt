//=============================================================================
//
//      synth_diag.c
//
//      Synthetic target diagnostic output code
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
// Author(s):   proven
// Contributors:proven, bartv
// Date:        1998-10-05
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//              There are two possible ways of performing I/O. The first
//              involves simply writing to stdout. This is robust, but
//              has some disadvantages such as the output getting mixed up
//              with gdb output. The second involves sending the data on
//              to the auxiliary, less robust but much more flexible.
//
//              Similarly, input can be handled by reading from stdin or
//              by a suitable device in the auxiliary.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_io.h>
#include <cyg/infra/cyg_ass.h>

//-----------------------------------------------------------------------------
// If the auxiliary exists, hal_diag_init() will try to contact it and
// instantiate a console device. Subsequent console writes will be
// redirected to that device, as long as the auxiliary is up and running.
// If the auxiliary is not being used or has exited, console writes
// will instead go to stdout.
//
// This code also contains an implementation of hal_diag_read_char()
// which is probably not very useful. Currently it works by reading
// from stdin, but no attempt is made to set the tty into raw mode
// or anything like that. 

static int auxiliary_console_id = -1;

void hal_diag_init( void )
{
    if (synth_auxiliary_running) {
        auxiliary_console_id = synth_auxiliary_instantiate("hal/synth/arch", SYNTH_MAKESTRING(CYGPKG_HAL_SYNTH), "console",
                                                      (const char*) 0, (const char*) 0);
    }
}

// Output a single character.
//
// The calling code will output one character at a time. Output
// involves at least one system call, and this is expensive for
// a single character (especially when used in conjunction with
// I/O intensive facilities like unbuffered tracing). Therefore
// this code will buffer lines up to 128 characters before
// doing the I/O.
//
// NOTE: one problem is that there is no support for flushing buffers
// at this level. Therefore if say C library stdio ends up mapped to
// HAL diagnostics I/O then functions like fflush() and setvbuf() will
// not behave the way they should. There is no simple workaround at
// this level, the required information is not available.

void hal_diag_write_char(char c)
{
    static int  diag_index = 0;
    static unsigned char diag_buffer[128];

    CYG_ASSERT(diag_index < 128, "Diagnostic buffer overflow");
    
    diag_buffer[diag_index++] = (unsigned char) c;
    if (('\n' == c) || (128 == diag_index)) {
        if ((-1 != auxiliary_console_id) && synth_auxiliary_running) {
            synth_auxiliary_xchgmsg(auxiliary_console_id, 0, 0, 0, diag_buffer, diag_index, (int *) 0, (unsigned char*) 0, (int *)0, 0);
            diag_index = 0;
        } else {
            int     written;
            char*   next    = diag_buffer;

            while (diag_index > 0) {
                written = cyg_hal_sys_write(1, next, diag_index);
                if (written > 0) {
                    diag_index -= written;
                    next       += written;
                } else if ((-CYG_HAL_SYS_EINTR != written) && (-CYG_HAL_SYS_EAGAIN != written)) {
                    CYG_FAIL("Unexpected error writing to stdout.");
                    diag_index = 0;
                    break;
                }
            }
            CYG_ASSERT(0 == diag_index, "All data should have been written out");
            diag_index = 0;
            cyg_hal_sys_fdatasync(1);
        }
    }
}

// Diagnostic input. It is not clear that this is actually useful,
// input would normally go to gdb rather than to the application. If
// keyboard input really is required then that should be handled via a
// suitable device driver interacting with the auxiliary, not at the
// HAL level. The read syscall will get woken up by the itimer alarm,
// but we don't want to stop reading if that's the case

void hal_diag_read_char(char *c)
{
    int rc;
    do {
        rc = cyg_hal_sys_read(0, c, 1);
    } while ((-CYG_HAL_SYS_EINTR == rc) || (-CYG_HAL_SYS_EAGAIN == rc));
}

//-----------------------------------------------------------------------------
// End of hal_diag.c
