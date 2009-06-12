//==========================================================================
//
//        kbd_test.c
//
//        Cirrus Logic EDB7xxx eval board ASCII keyboard test
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1999-05-15
// Description:   Tool used to test ASCII keyboard
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>   // Configuration header
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_edb7xxx.h>  // Board definitions

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // HAL interrupt macros
#include <cyg/hal/drv_api.h>            // HAL ISR support

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define STACK_SIZE 4096
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

#include "kbd_support.c"

// FUNCTIONS

static void
cyg_test_exit(void)
{
    while (TRUE) ;
}

static void
kbd_exercise(cyg_addrword_t p)
{
    cyg_uint8 ch;
    diag_printf("KBD test here!\n");

    // Initialize keyboard driver
    kbd_init();

    while (TRUE) {
        ch = kbd_getc();
        diag_printf("CH = 0x%x:'%c'\n", ch, ch);
    }

    diag_printf("All done!\n");
    cyg_test_exit();
}

externC void
cyg_start( void )
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(11,                // Priority - just a number
                      kbd_exercise,      // entry
                      0,                 // initial parameter
                      "KBD_thread",      // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    // Let 'em fly...
    cyg_scheduler_start();
} // cyg_package_start()

