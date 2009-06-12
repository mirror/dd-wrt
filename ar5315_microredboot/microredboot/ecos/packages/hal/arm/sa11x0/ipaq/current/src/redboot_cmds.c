//==========================================================================
//
//      redboot_cmds.c
//
//      iPAQ [platform] specific RedBoot commands
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
// Author(s):    gthomas
// Contributors: gthomas
//               Richard Panton <richard.panton@3glab.com>
// Date:         2001-02-24
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#include <cyg/hal/hal_sa11x0.h>   // Board definitions
#include <cyg/hal/ipaq.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

// Exported CLI function(s)
static void do_gpio(int argc, char *argv[]);
RedBoot_cmd("gpio",
	    "Query and/or set the GPIO status",
	    "[-s bits] [-c bits]",
	    do_gpio
    );

static void do_egpio(int argc, char *argv[]);
RedBoot_cmd("egpio",
	    "Query and/or set the EGPIO status",
	    "[-s bits] [-c bits]",
	    do_egpio
    );

static void do_mem(int argc, char *argv[]);
RedBoot_cmd("mem",
	    "Set a memory location",
	    "[-h|-b] [-a <address>] <data>",
	    do_mem
    );

static void do_physaddr(int argc, char *argv[]);
RedBoot_cmd("physaddr",
	    "Converts a virtual to a physical address",
	    "<address>",
	    do_physaddr
    );

static void
do_gpio(int argc,char *argv[]) {
    struct option_info opts[2];
    bool set_bits_set, clr_bits_set;
    int set_bits, clr_bits;
    init_opts(&opts[0], 's', true, OPTION_ARG_TYPE_NUM, 
              (void **)&set_bits, (bool *)&set_bits_set, "bits to set");
    init_opts(&opts[1], 'c', true, OPTION_ARG_TYPE_NUM, 
              (void **)&clr_bits, (bool *)&clr_bits_set, "bits to clear");
    if (!scan_opts(argc, argv, 1, opts, 2, NULL, 0, NULL))
    {
        return;
    }
    if ( !set_bits_set && !clr_bits_set ) {
	// display only
	diag_printf("  gpio = 0x%08lX\n", *SA11X0_GPIO_PIN_LEVEL);
	diag_printf("         0x%08lX are output\n", *SA11X0_GPIO_PIN_DIRECTION);
	diag_printf("         0x%08lX rising edge detect\n", *SA11X0_GPIO_RISING_EDGE_DETECT);
	diag_printf("         0x%08lX falling edge detect\n", *SA11X0_GPIO_FALLING_EDGE_DETECT);
	diag_printf("         0x%08lX edge detect status\n", *SA11X0_GPIO_EDGE_DETECT_STATUS);
	diag_printf("         0x%08lX alternate function\n", *SA11X0_GPIO_ALTERNATE_FUNCTION);
	return;
    }
    diag_printf( "  gpio 0x%08lX, ", *SA11X0_GPIO_PIN_LEVEL);
    if ( set_bits_set ) {
	diag_printf("set(0x%08X) ",set_bits);
	*SA11X0_GPIO_PIN_OUTPUT_SET = set_bits;
    }
    if ( clr_bits_set ) {
	diag_printf("clear(0x%08X) ",clr_bits);
	*SA11X0_GPIO_PIN_OUTPUT_CLEAR = clr_bits;
    }
    diag_printf( "gives 0x%08lX\n", *SA11X0_GPIO_PIN_LEVEL);
}

static void
do_egpio(int argc,char *argv[]) {
    struct option_info opts[2];
    bool set_bits_set, clr_bits_set;
    int set_bits, clr_bits;
    init_opts(&opts[0], 's', true, OPTION_ARG_TYPE_NUM, 
              (void **)&set_bits, (bool *)&set_bits_set, "bits to set");
    init_opts(&opts[1], 'c', true, OPTION_ARG_TYPE_NUM, 
              (void **)&clr_bits, (bool *)&clr_bits_set, "bits to clear");
    if (!scan_opts(argc, argv, 1, opts, 2, NULL, 0, NULL)) return;
    if ( !set_bits_set && !clr_bits_set ) {
	// display only
	diag_printf("  egpio = 0x%04X\n", (int)(_ipaq_EGPIO & 0xffff));
	return;
    }
    diag_printf( "  egpio 0x%04X, ", (int)(_ipaq_EGPIO & 0xffff));
    if ( set_bits_set ) {
	diag_printf("set(0x%08X) ",set_bits);
	ipaq_EGPIO( set_bits, set_bits );
    }
    if ( clr_bits_set ) {
	diag_printf("clear(0x%08X) ",clr_bits);
	ipaq_EGPIO( clr_bits, 0x0000 );
    }
    diag_printf( "gives 0x%04X\n", (int)(_ipaq_EGPIO & 0xffff));
}

static void
do_mem(int argc, char *argv[]) {
    struct option_info opts[3];
    bool mem_half_word, mem_byte;
    static int address = 0x00000000;
    int value;
    init_opts(&opts[0], 'b', false, OPTION_ARG_TYPE_FLG,
	      (void**)&mem_byte, 0, "write a byte");
    init_opts(&opts[1], 'h', false, OPTION_ARG_TYPE_FLG,
	      (void**)&mem_half_word, 0, "write a half-word");
    init_opts(&opts[2], 'a', true, OPTION_ARG_TYPE_NUM,
	      (void**)&address, NULL, "address to write at");
    if (!scan_opts(argc, argv, 1, opts, 3, (void*)&value, OPTION_ARG_TYPE_NUM, "address to set"))
	return;
    if ( mem_byte && mem_half_word ) {
	diag_printf("*ERR: Should not specify both byte and half-word access\n");
    } else if ( mem_byte ) {
	*(cyg_uint8*)address = (cyg_uint8)(value & 255);
	diag_printf("  Set 0x%08X to 0x%02X (result 0x%02X)\n", address, value & 255, (int)*(cyg_uint8*)address );
    } else if ( mem_half_word ) {
	if ( address & 1 ) {
	    diag_printf( "*ERR: Badly aligned address 0x%08X for half-word store\n", address );
	} else {
	    *(cyg_uint16*)address = (cyg_uint16)(value & 0xffff);
	    diag_printf("  Set 0x%08X to 0x%04X (result 0x%04X)\n", address, value & 0xffff, (int)*(cyg_uint16*)address );
	}
    } else {
	if ( address & 3 ) {
	    diag_printf( "*ERR: Badly aligned address 0x%08X for word store\n", address );
	} else {
	    *(cyg_uint32*)address = (cyg_uint32)value;
	    diag_printf("  Set 0x%08X to 0x%08X (result 0x%08X)\n", address, value, (int)*(cyg_uint32*)address );
	}
    }
}

static void
do_physaddr(int argc, char *argv[]) {
    unsigned long phys_addr, virt_addr;

    if ( !scan_opts(argc,argv,1,0,0,(void*)&virt_addr, OPTION_ARG_TYPE_NUM, "virtual address") )
	return;
    phys_addr = hal_virt_to_phys_address(virt_addr);
    diag_printf("Virtual addr %p = physical addr %p\n", virt_addr, phys_addr);
}

// Get here when RedBoot is idle.  If it's been long enough, then
// dim the LCD.  The problem is - how to determine other activities
// so at this doesn't get in the way.  In the default case, this will
// be called from RedBoot every 10ms (CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT)

#define MAX_IDLE_TIME (30*100)
#ifdef CYGSEM_IPAQ_LCD_COMM
extern void lcd_on(bool);
#endif

static void
idle(bool is_idle)
{
    static int idle_time = 0;
    static bool was_idled = false;

    if (is_idle) {
        if (!was_idled) {
            if (++idle_time == MAX_IDLE_TIME) {
                was_idled = true;
#ifdef CYGSEM_IPAQ_LCD_COMM
                lcd_on(false);
#endif
            }
        }
    } else {        
        idle_time = 0;
        if (was_idled) {
            was_idled = false;
#ifdef CYGSEM_IPAQ_LCD_COMM
                lcd_on(true);
#endif
        }
    }
}

RedBoot_idle(idle, RedBoot_AFTER_NETIO);
