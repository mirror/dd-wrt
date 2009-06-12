//=============================================================================
//
//      battery.c
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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

#include <redboot.h>
#include <cyg/hal/hal_io.h>
#include "test_menu.h"

extern void diag_wait(void);

#define TEST_PATTERN 0x55555555

// test battery status
void
battery_status(MENU_ARG arg)
{
    unsigned short status;

    // read battery status port
    status = *IQ80321_BATTERY_STATUS;

    // examine bit b0 BATT_PRES#
    if (status & IQ80321_BATTERY_NOT_PRESENT) {
	diag_printf("No battery installed.\n");
	diag_wait();
	return;
    }
    diag_printf("A battery is installed.\n");

    if (status & IQ80321_BATTERY_CHARGE)			
	diag_printf("Battery is fully charged.\n");
    else
	diag_printf("Battery is charging.\n");

    if (status & IQ80321_BATTERY_DISCHARGE)			
	diag_printf("Battery is fully discharged.\n");
    else
	diag_printf("Battery voltage measures within normal operating range.\n");

    if (status & IQ80321_BATTERY_ENABLE)			
	diag_printf("Battery Backup is enabled.\n");
    else
	diag_printf("Battery Backup is disabled.\n");

    diag_wait();
}


#ifdef CYGSEM_HAL_ARM_IQ80321_BATTERY_TEST
// Data to be written to address and read after the board has
// been powered off and powered back on

static void
battery_test_write (MENU_ARG arg)
{
    *IQ80321_BATTERY_STATUS |= IQ80321_BATTERY_ENABLE;
    if (*IQ80321_BATTERY_STATUS & IQ80321_BATTERY_ENABLE)
        diag_printf("The battery backup has now been enabled.\n");
    else
        diag_printf("Unable to enable battery backup.\n");

    *(volatile unsigned *)SDRAM_BATTERY_TEST_ADDR = TEST_PATTERN;

    diag_printf("The value '%p' is now written to DRAM at address %p.\n",
	   TEST_PATTERN, SDRAM_BATTERY_TEST_ADDR);
    diag_printf("\nYou may now power the board off, wait 60 seconds and power it back on.\n");
    diag_printf("Then come back into the battery test menu and check data in DRAM.\n");
    diag_wait();
}


static void
battery_test_read (MENU_ARG arg)
{
    cyg_uint32 value;	

    *IQ80321_BATTERY_STATUS &= ~IQ80321_BATTERY_ENABLE;
    if (*IQ80321_BATTERY_STATUS & IQ80321_BATTERY_ENABLE)
        diag_printf("Unable to disable battery backup.\n");
    else
        diag_printf("The battery backup has now been disabled.\n");

    value = *(volatile unsigned *)SDRAM_BATTERY_TEST_ADDR;

    diag_printf ("Value written at address %p: %p\n",
		 SDRAM_BATTERY_TEST_ADDR, TEST_PATTERN);
    diag_printf ("Value read: %p\n", value);

    if (value == TEST_PATTERN)
	diag_printf ("\nThe battery test is a success !");
    else {
	diag_printf ("\n****************************\n");
	diag_printf ("* The battery test failed. *\n");
	diag_printf ("****************************\n");
    }

    diag_wait();
}


void
battery_test_menu (MENU_ARG arg)
{
    // Test Menu Table
    static MENU_ITEM batteryMenu[] = {
	{"Write data to SDRAM", battery_test_write, NULL},
	{"Check data from SDRAM", battery_test_read, NULL},
    };

    unsigned int num_menu_items = (sizeof (batteryMenu) / sizeof (batteryMenu[0]));

    char *menu_title = "\n Battery Backup SDRAM memory test.";

    diag_printf ("\n*************************************************************************\n");
    diag_printf ("* This test will enable you to perform a battery test in 4 steps:       *\n"); 
    diag_printf ("*  1/  Select option 1 to write test pattern,                           *\n");
    diag_printf ("*  2/  Power the board off and wait 60 seconds,                         *\n"); 
    diag_printf ("*  3/  Power the board back on,                                         *\n"); 
    diag_printf ("*  4/  Select option 2 to read back and compare test pattern            *\n");
    diag_printf ("*************************************************************************");

    menu (batteryMenu, num_menu_items, menu_title, MENU_OPT_NONE);
    diag_printf ("\n");
}
#endif // CYGSEM_HAL_ARM_IQ80321_BATTERY_TEST

