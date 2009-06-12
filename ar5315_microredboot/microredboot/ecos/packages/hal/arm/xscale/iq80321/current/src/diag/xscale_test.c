//=============================================================================
//
//      xscale_test.c - Cyclone Diagnostics
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
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors: Mark Salter
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/************************************************************************/
/*									*/
/* Modification History							*/
/* --------------------							*/
/* 11oct00, ejb, Created for IQ80310 StrongARM2				*/
/* 18dec00  jwf                                                         */
/* 02feb01  jwf	added tests: _coy_tight_loop, cache_loop, LoopMemTest,  */
/*              special_mem_test written by snc	 		        */     
/* 07feb01  jwf added function calls to a variable delay time generator */
/* 09feb01  jwf added function version_info to show version information */
/*              about OS, BOARD, CPLD, 80200 ID, 80312 ID.              */
/************************************************************************/


#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_verde.h>          // Hardware definitions
#include <cyg/hal/iq80321.h>            // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf
#include <redboot.h>
#include <cyg/io/pci.h>

#include "test_menu.h"

extern CYG_ADDRWORD hexIn(void);

extern int memTest (long startAddr, long endAddr);
extern int LoopMemTest (long startAddr, long endAddr);

void hdwr_diag (void);

extern void enet_setup (MENU_ARG arg);

static void memory_tests (MENU_ARG arg);
static void repeat_mem_test (MENU_ARG arg);
static void special_mem_test (MENU_ARG arg);
static void rotary_switch (MENU_ARG arg);
static void seven_segment_display (MENU_ARG arg);
static void cache_loop (MENU_ARG arg);
extern void pci_test (MENU_ARG arg);
extern void battery_status(MENU_ARG arg);
extern void battery_test_menu (MENU_ARG arg);
extern void timer_test (MENU_ARG arg);


/* Test Menu Table */
static MENU_ITEM testMenu[] =
{
    {"Memory Tests", memory_tests, 0},
    {"Repeating Memory Tests", repeat_mem_test, 0},
    {"Repeat-On-Fail Memory Test", special_mem_test, 0},
    {"Rotary Switch S1 Test", rotary_switch, 0},
    {"7 Segment LED Tests", seven_segment_display, 0},
    {"i82544 Ethernet Configuration", enet_setup, 0},
    {"Battery Status Test", battery_status, 0},
#ifdef CYGSEM_HAL_ARM_IQ80321_BATTERY_TEST
    {"Battery Backup SDRAM Memory Test", battery_test_menu, 0},
#endif
    {"Timer Test", timer_test, 0},
    {"PCI Bus test", pci_test, 0},
    {"CPU Cache Loop (No return)", cache_loop, 0}
};

#define NUM_MENU_ITEMS	(sizeof (testMenu) / sizeof (testMenu[0]))
#define MENU_TITLE	"\n  IQ80321 Hardware Tests"

void
diag_wait(void)
{
    char buf[1];
    diag_printf ("Press return to continue.\n");
    while (_rb_gets(buf, sizeof(buf), 0) != _GETS_OK)
	;
}

void
hdwr_diag (void)
{
    diag_printf ("Entering Hardware Diagnostics - Disabling Data Cache!\n");

    cyg_pci_init(); 

    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
	
    menu (testMenu, NUM_MENU_ITEMS, MENU_TITLE, MENU_OPT_NONE);

    diag_printf ("Exiting Hardware Diagnostics!\n\n");
    HAL_DCACHE_ENABLE();
}

static void
cache_loop (MENU_ARG arg)
{
    diag_printf ("Putting Processor in a Tight Loop Forever...\n\n");

    asm ( "0: mov r0,r0\n"
	  "b 0b\n");
}


// ***************************************************************************
// memory_tests - Basic Memory Tests                       
//
// Memory tests can be run one of two ways - with the cache turned OFF to test
// physical memory, or with cache turned ON to test the caching
//
static void
memory_tests (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, end_addr;
    long mem_size;

    diag_printf("Base address of memory to test (in hex): ");
    start_addr = hexIn();

    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();

    end_addr = start_addr + mem_size - 1;

    diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);
    memTest(start_addr, end_addr);
    diag_printf ("\nMemory test done.\n");
}


// ***************************************************************************
// repeat_mem_test - Repeating Memory Tests                       
//
static void
repeat_mem_test (MENU_ARG arg)
{
    CYG_ADDRWORD start_addr, mem_size, end_addr;
    char	cache_disable[10];

    diag_printf ("Turn off Data Cache? (y/n): ");
    while (_rb_gets(cache_disable, sizeof(cache_disable), 0) != _GETS_OK)
	;
    diag_printf ("\nBase address of memory to test (in hex): ");
    start_addr = hexIn();
    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();
    end_addr = start_addr + mem_size - 1;
    diag_printf("\nTesting memory from %p to %p", start_addr, end_addr);
    while (memTest (start_addr, end_addr))
        ;
}

// ****************************************************************************
// special_mem_test - Repeat-On-Fail Memory Test                     
//
// Memory tests can be run one of two ways - with the cache turned OFF to test
// physical memory, or with cache turned ON to test the caching
//
static void
special_mem_test (MENU_ARG arg)
{
    long	start_addr;
    long	mem_size;
    long	end_addr;

    diag_printf ("Base address of memory to test (in hex): ");
    start_addr = hexIn();
    diag_printf ("\nSize of memory to test (in hex): ");
    mem_size = hexIn();

    end_addr = start_addr + mem_size - 1;
    diag_printf("\nTesting memory from %p to %p.\n", start_addr, end_addr);

    LoopMemTest(start_addr, end_addr);

    diag_printf ("\n\nMemory test done.\n");
    diag_wait();
}

static unsigned char led_data[] = {
    DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3, DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
    DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B, DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F,
    DISPLAY_PERIOD, DISPLAY_OFF
};

// sequential test for LSD and MSD 7 segment Leds
//
void
seven_segment_display (MENU_ARG arg)
{
    unsigned char SevSegDecode;
    int DisplaySequence;
    int SelectLed;

    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_OFF;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_OFF;
    
    SelectLed=0; /* initialize 7 segment LED selection */
	
    for (SelectLed = 0; SelectLed < 2; SelectLed++) {
	/* run test data sequence for a 7 segment LED */
	for (DisplaySequence = 0; DisplaySequence <= 17; ++DisplaySequence ) {
	    /* fetch 7 segment decode byte */
	    SevSegDecode = led_data[DisplaySequence];

	    /* display test data on selected 7 segment LED */
	    /* the test data sequence for a 7 segment led will be seen as:*/
	    /* 0 1 2 3 4 5 6 7 8 9 A b C d e F . */
	    if (SelectLed)
		*(volatile unsigned char *) DISPLAY_LEFT = SevSegDecode;
	    else
		*(volatile unsigned char *) DISPLAY_RIGHT = SevSegDecode;

	    CYGACC_CALL_IF_DELAY_US((cyg_int32)4*100000);
	}
    }

    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_S;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_S;
}


// tests rotary switch status, S1 positions 0-3, a 2 bit output code
static void
rotary_switch (MENU_ARG arg)
{
    char recv_data[1];
    const unsigned char MAX_SWITCH_SAMPLES = 9;
    unsigned char RotarySwitch[MAX_SWITCH_SAMPLES]; // multiple samples of a 4 bit switch code
    unsigned char index;		// index for Rotary Switch array
    unsigned char debounce;		// keeps tally of equal rotary switch data reads in a loop
    unsigned char SevSegDecode;		// holds decode data for a 7 segment LED display

    *(volatile unsigned char *)DISPLAY_LEFT = DISPLAY_OFF;
    *(volatile unsigned char *)DISPLAY_RIGHT = DISPLAY_OFF;
    diag_printf("\n\nThe 7-Segment LSD LED shows the Rotary Switch position selected, i.e., 0-F.");
    diag_printf("\n\nSlowly dial the Rotary Switch through each position 0-F and confirm reading.");

    diag_printf( "\n\nStrike <CR> to exit this test." );
    while (_rb_gets(recv_data, sizeof(recv_data), 50) != _GETS_OK) {
	do {
	    for(index = 0; index <= MAX_SWITCH_SAMPLES; index++) {
		RotarySwitch[index] = *(volatile unsigned char *) IQ80321_ROTARY_SWITCH_ADDR;
		RotarySwitch[index] &= 0x0f;
	    }
	    debounce = 0;
	    for(index = 1; index <= MAX_SWITCH_SAMPLES; index++) {
		if (RotarySwitch[0] == RotarySwitch[index])
		    debounce++;	// keep tally of equal rotary switch code samples
	    }
	} while (debounce < (MAX_SWITCH_SAMPLES - 1));

	// decipher state of rotary switch position
	if (RotarySwitch[0] > 16)
	    RotarySwitch[0] = 16;
	SevSegDecode = led_data[RotarySwitch[0]];
	
	// display the rotary switch position on the 7 segment LSD LED as: 0, 1, 2, 3, etc.
	*(volatile unsigned char *)DISPLAY_RIGHT = SevSegDecode;
    }

} // end rotary_switch()




