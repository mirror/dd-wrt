//=============================================================================
//
//      hal_aux.c
//
//      HAL auxiliary objects and code; per platform
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
// Author(s):   pfine
// Contributors:hmt
// Date:        2002-02-27
// Purpose:     HAL aux objects: startup tables.
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_if.h>             // hal_if_init
#include <cyg/hal/hal_io.h>             // hal_if_init
#include <cyg/hal/hal_misc.h>           // cyg_hal_is_break

#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/mpc8260.h>            // Needed for IMMR structure

#ifdef CYG_HAL_STARTUP_ROM
#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#include <redboot.h>
// Exported CLI function
RedBoot_cmd("sdram_test", 
            "test the sdram", 
            "",
            do_sdram_test 
    );

void
do_sdram_test(int argc, char *argv[])
{
    unsigned long oldints;

    diag_printf("Starting test for SDRAM.\n");
    diag_printf("D18 will indicate PASS/FAIL (Green/Red).\n");
    diag_printf("Resetting board will be necessary after test completion.\n");
    HAL_DISABLE_INTERRUPTS(oldints);
    memory_test();
}
#endif
#endif
// For Baud Rate Calculation, see MPC8260 PowerQUICC II User's Manual
// 16.3 UART Baud Rate Examples, page 16-5.
#define UART_BIT_RATE(n) \
    (((int)(CYGHWR_HAL_POWERPC_BOARD_SPEED*1000000))/(n * 64))
#define UART_BAUD_RATE CYGNUM_HAL_TS6_DIAG_BAUD

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    // Mapping for the Motorola MPC8260 development board
    CYGARC_MEMDESC_CACHE( 0x00000000, 0x04000000 ), // Main memory 60x SDRAM
    // Mapping for the FPGA and the MPC8260 Internal memory
    CYGARC_MEMDESC_NOCACHE( 0x04500000, 0x00400000 ),
    // Mapping for the Cluster Bus
    CYGARC_MEMDESC_NOCACHE( 0xE0000000, 0x10000000 ),
    // Mapping for the FLASH
    CYGARC_MEMDESC_NOCACHE( 0xff800000, 0x00800000 ), // ROM region

    CYGARC_MEMDESC_TABLE_END
};


/***********************/
/* Global Declarations */
/***********************/
//#define USE_SMC1

volatile t_PQ2IMM  *IMM;   /* IMM base pointer */

//--------------------------------------------------------------------------
// Platform init code.
void
hal_platform_init(void)
{

// Ports and Pins assigned for General Purpose I/O
// PORT  PIN(s) DIR (I/O)   Name      Init Value Comment
//-------------------------------------------------------------------
//  A    30-31     TBD                    TBD    Wires to RF Board
//  A      29       O     TS_RESET_L       0     Tiger Sharc Reset 
//  A      28       O     LP_ENB_L         1     Link Port Buffer Enable
//  A    26-27     TBD                    TBD    Wires to RF Board
//  A    18-23     TBD                    TBD    Wires to each Tiger Sharc
//  A    12-17      O     xxxxxxxxxx       0     Tiger Sharc Interrupt
//  B     4-7       O     xxxxxxxxxx      TBD    User controlled LEDs
//  C      8        I     xxxxxxxxxx      xxx    RF Board Interrupt
//  C      6        I     xxxxxxxxxx      xxx    FPGA Interrupt
//  C     0-5       I     xxxxxxxxxx      xxx    TSn Interrupt
//  D      20       ?     PPC_WAIT        TBD    Open Drain == High Z
//  D     7-9       O     CIMP 0-2        110    TS Impedance ctrl
//  D     4-6       O     DS 0-2          100    TS Drive Strength


// Ports and Pins assigned for Dedicated Pin Assignment
// PORT  PIN(s) DIR (I/O)   Name      Init Value Comment
//-------------------------------------------------------------------
//  D    16-19      x                     TBD    SPI to RF Board
//  D    14-15      x                     TBD    I2C



#define TS6_PPARA_INIT_MASK 0xFFF00000
#define TS6_PDIRA_INIT_MASK 0x3003F000
#define TS6_PDATA_INIT_MASK 0x1003F000
    IMM =  (t_PQ2IMM *)0x04700000;  /* MPC8260 internal register map  */

   /*-------------------------------------------*/
	/* Program the Port Pin Registers */
   /*-------------------------------------------*/

    IMM->io_regs[PORT_A].ppar &= 0xFFF00000;  /* Clear bits for GPIO */
    IMM->io_regs[PORT_A].pdir |= 0x000FC00C;  /* Set bits on outputs */
    IMM->io_regs[PORT_A].pdat |= 0x00000008;  /* Set high outputs bits */
    IMM->io_regs[PORT_A].pdat &= 0xFFF03FFB;  /* Clear low output bits */

// Initialize Port B Pins 4,5,6,7 general purpose IO
// Pin == 0 ==> LED on
// Pin 4 LED 18, Red
// Pin 5 LED 18, Green
// Pin 6 LED 17, Red
// Pin 7 LED 17, Green
    IMM->io_regs[PORT_B].ppar &= 0xF0FFFFFF;  /* Clear 4-7 */
    IMM->io_regs[PORT_B].pdir |= 0x0F000000;  /* Set  4-7 as outputs */
    IMM->io_regs[PORT_B].pdat |= 0x0F000000;  /* Clear LED's */
    IMM->io_regs[PORT_B].pdat &= 0xFDFFFFFF;  /* Set LED's 17 to green */

    IMM->io_regs[PORT_C].ppar &= 0x007FFFFF;  /* Clear 0 -8 */
    IMM->io_regs[PORT_C].pdir &= 0x007FFFFF;  /* Configure as inputs */
    //IMM->io_regs[PORT_C].podr &= 0xFF800000;  /* Configure as inputs */
    
    IMM->io_regs[PORT_D].ppar &= 0xF03FF7FF;   
    IMM->io_regs[PORT_D].pdir |= 0x0FC00000;
    IMM->io_regs[PORT_D].podr |= 0x00000800;
    IMM->io_regs[PORT_D].pdat |= 0x09800000;

#ifdef USE_CPM_SPI_CONTROLLER
    // Dedicated Pin assignments for SPI
    IMM->io_regs[PORT_D].ppar |= 0x0000F000;
    IMM->io_regs[PORT_D].podr &= 0xFFFF0FFF;
    IMM->io_regs[PORT_D].pdir |= 0x0000F000;
#else
    // The ts6 board does not use the SPI controller provided by the CPM,
    // instead it is left up to the application to control the SPI.
    // Therefore, initialize the SPI specific pins as General Purpose I/O
    // and Bi-directional.
    IMM->io_regs[PORT_D].ppar &= 0xFFFF0FFF; /* General Purpose I/O */
    IMM->io_regs[PORT_D].pdir &= 0xFFFF0FFF; /* input or Bi-directional */
#endif

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    hal_if_init();
#endif
   
}
// EOF hal_aux.c
