//==========================================================================
//
//        lcd_test.c
//
//        SA1110/CerfCube - LCD test
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
// Date:          2000-06-05
// Description:   Tool used to test LCD stuff
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>       // Configuration header
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_io.h>       // IO macros
#include <cyg/hal/hal_arch.h>     // Register state info
#include <cyg/hal/hal_intr.h>     // HAL interrupt macros

#include <cyg/hal/hal_sa11x0.h>   // Board definitions
#include <cyg/hal/cerf.h>
#include <cyg/hal/hal_cache.h>

#include "eCos.xpm"
#include "eCos2.xpm"
#include "redhat.xpm"
#include "redhat2.xpm"
#include "redboot.xpm"
#include "escw.xpm"
#include "logo.xpm"

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define STACK_SIZE 4096
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

// FUNCTIONS

static void
cyg_test_exit(void)
{
    while (TRUE) ;
}

static void
lcd_test(cyg_addrword_t p)
{
    int i, pix, row, col;
    int on;

    diag_printf("LCD test here\n");

    lcd_init(16);
#if 0
    for (i = 0;  i < 16;  i++) {
        on = true;
        diag_printf("Fill with 0x%x\n", i);
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320/2;  col++) {
                if (on) {
                    fp->pixels[row][col] = RGB_RED(i)|RGB_GREEN(i)|RGB_BLUE(i);
                } else {
                    fp->pixels[row][col] = 0xFFFF;
                }
            }
            for (col = 320/2;  col < 320;  col++) {
                if (!on) {
                    fp->pixels[row][col] = RGB_RED(i)|RGB_GREEN(i)|RGB_BLUE(i);
                } else {
                    fp->pixels[row][col] = 0xFFFF;
                }
            }
            if ((row & 0x0F) == 0x0F) {
                if (on) {
                    on = false;
                } else {
                    on = true;
                }
            }
        }
        cyg_thread_delay(100);
    }
#endif
#if 0
    for (i = 0;  i < 4;  i++) {
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320;  col++) {
                switch (row/40) {
                case 0:
                    pix = col / 20;  // 0..15
                    fp->pixels[row][col] = RGB_RED(pix);
                    break;
                case 1:
                    pix = col / 10;  // 0..31
                    fp->pixels[row][col] = RGB_GREEN(pix);
                    break;
                case 2:
                    pix = col / 20;  // 0..15
                    fp->pixels[row][col] = RGB_BLUE(pix);
                    break;
                case 3:
                    pix = col / 20;  // 0..15
                    fp->pixels[row][col] = RGB_BLUE(pix) | RGB_GREEN(pix);
                    break;
                case 4:
                    pix = col / 20;  // 0..15
                    fp->pixels[row][col] = RGB_BLUE(15) | RGB_GREEN(pix);
                    break;
                case 5:
                    fp->pixels[row][col] = 0xFFFF;
                    break;
                }
            }
        }
        cyg_thread_delay(100);
#if 0
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320;  col++) {
                pix = col / 20;  // 0..15
                switch (row/60) {
                case 0:
                    fp->pixels[row][col] = RGB_RED(pix);
                    break;
                case 1:
                    fp->pixels[row][col] = RGB_GREEN(pix);
                    break;
                case 2:
                    fp->pixels[row][col] = RGB_BLUE(pix);
                    break;
                case 3:
                    fp->pixels[row][col] = 0xFFFF;
                    break;
                }
            }
        }
        cyg_thread_delay(100);
#endif
#if 0
        on = true;
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320/2;  col++) {
                if (on) {
                    fp->pixels[row][col] = RGB_GREEN(15);
                } else {
                    fp->pixels[row][col] = RGB_BLUE(8);
                }
            }
            for (col = 320/2;  col < 320;  col++) {
                if (!on) {
                    fp->pixels[row][col] = RGB_GREEN(15);
                } else {
                    fp->pixels[row][col] = RGB_BLUE(8);
                }
            }
            if ((row & 0x0F) == 0x0F) {
                if (on) {
                    on = false;
                } else {
                    on = true;
                }
            }
        }
#endif
    }
#endif
#if 0
    for (row = 0;  row < 240;  row++) {            
        for (col = 0;  col < 320;  col++) {
            if (col == 59) {
                fp->pixels[row][col] = 0x0000;
            } else {
                fp->pixels[row][col] = 0xFFFF;
            }
        }
    }
    cyg_thread_delay(100);
#endif
#if 0
    for (i = 0;  i < 16;  i++) {
        diag_printf("Value 0x%04x\n", (1<<i));
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320;  col++) {
                fp->pixels[row][col] = (1<<i);
            }
        }
        cyg_thread_delay(500);
    }
#endif
#if 0
    for (i = 0;  i < 32;  i++) {
        diag_printf("Red at %d\n", i);
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320;  col++) {
                fp->pixels[row][col] = RGB_RED(i);
            }
        }
        cyg_thread_delay(100);
    }
#endif
#if 0
    for (i = 0;  i < 64;  i++) {
        diag_printf("Green at %d\n", i);
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320;  col++) {
                fp->pixels[row][col] = RGB_GREEN(i);
            }
        }
        cyg_thread_delay(100);
    }
#endif
#if 0
    for (i = 0;  i < 32;  i++) {
        diag_printf("BLUE at %d\n", i);
        for (row = 0;  row < 240;  row++) {            
            for (col = 0;  col < 320;  col++) {
                fp->pixels[row][col] = RGB_BLUE(i);
            }
        }
        cyg_thread_delay(100);
    }
#endif

    while (true) {
    for (i = 0;  i < 1;  i++) {
        show_xpm(redboot_xpm);
	cyg_thread_delay(15);
        show_xpm(eCos_xpm);
	cyg_thread_delay(25);
        show_xpm(redboot_xpm);
	cyg_thread_delay(15);
        show_xpm(redhat_xpm);
	cyg_thread_delay(25);
        show_xpm(redboot_xpm);
	cyg_thread_delay(25);
        show_xpm(redboot_xpm);
	cyg_thread_delay(15);
        show_xpm(escw_xpm);
	cyg_thread_delay(25);
        show_xpm(redboot_xpm);
	cyg_thread_delay(15);
        show_xpm(eCos2_xpm);
	cyg_thread_delay(25);
        show_xpm(redboot_xpm);
	cyg_thread_delay(15);
        show_xpm(redhat2_xpm);
	cyg_thread_delay(25);
        show_xpm(logo_xpm);
	cyg_thread_delay(25);
        show_xpm(redboot_xpm);
	cyg_thread_delay(50);
    }

#if 0
    // This doesn't seem to do anything on my unit
    cerf_BCR(SA1110_BCR_MOTOR, SA1110_BCR_MOTOR_ON);
    cyg_thread_delay(2*100);
    cerf_BCR(SA1110_BCR_MOTOR, SA1110_BCR_MOTOR_OFF);
#endif

        show_xpm(redboot_xpm);
	cyg_thread_delay(15);
    lcd_clear();
    lcd_printf("\n\n**** Hello world!\n");
    cyg_thread_delay(5);
    for (i = 0;  i < 64;  i++) {
        lcd_printf("... testing line #%d\n", i);
    }
    cyg_thread_delay(50);

        show_xpm(redboot_xpm);
	cyg_thread_delay(15);
    set_bg(0,0,0);
    set_fg(31,63,0);
    lcd_clear();
    for (i = 0;  i < 32;  i++) {
        lcd_printf("... testing line #%d\n", i);
    }
    cyg_thread_delay(50);
    }  // while

    lcd_clear();
    lcd_printf("*****");    
    cyg_thread_delay(200);

    cyg_test_exit();
}

externC void
cyg_start( void )
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      lcd_test,          // entry
                      0,                 // entry parameter
                      "LCD test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
} // cyg_package_start()
