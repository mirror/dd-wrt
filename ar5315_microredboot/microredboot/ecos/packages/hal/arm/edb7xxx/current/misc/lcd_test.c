//==========================================================================
//
//        lcd_test.c
//
//        Cirrus EDB7XXX eval board LCD test code
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
// Date:          1999-09-01
// Description:   Tool used to test LCD controller
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>   // Configuration header
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_edb7xxx.h>  // Board definitions

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define STACK_SIZE 4096
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

#include "lcd_support.c"

// FUNCTIONS

static void
cyg_test_exit(void)
{
    while (TRUE) ;
}

static void
lcd_test(int depth)
{
    int i, j;
    diag_printf("Set depth %d\n", depth);
    lcd_on(depth);
    lcd_clear();
    for (j = 0;  j < 5;  j++) {
        for (i = 0;  i < width;  i++) {
            lcd_putc('A');
        }
    }
    lcd_putc('\n');
    lcd_putc('\n');
    cyg_thread_delay(5*100);
    for (j = 0;  j < 5;  j++) {
        for (i = FIRST_CHAR;  i <= LAST_CHAR;  i++) {
            lcd_putc(i);
        }
    }
    cyg_thread_delay(5*100);
}

static void
lcd_exercise(cyg_addrword_t p)
{
    diag_printf("LCD test here!\n");

//    lcd_test(1);
//    lcd_test(2);
    lcd_test(4);

    diag_printf("All done!\n");
    cyg_test_exit();
}

externC void
cyg_start( void )
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      lcd_exercise,      // entry
                      0,                 // entry parameter
                      "LCD_test_thread", // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
} // cyg_package_start()

