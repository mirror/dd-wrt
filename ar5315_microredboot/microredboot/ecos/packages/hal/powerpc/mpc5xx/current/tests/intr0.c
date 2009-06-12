//=================================================================
//
//        intr0.c
//
//        Interrupt test 0
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Bob Koninckx
// Contributors:  Bob Koninckx
// Date:          2002-11-16
// Description:   Simple test of MPC5xx interrupt handling when the
//                kernel has not been configured. Uses timer interrupts.
// Options:
//####DESCRIPTIONEND####

#include <pkgconf/hal.h>
#include <pkgconf/infra.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_trac.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/ppc_regs.h>

#define ID_RTC_SEC 12345
#define ID_RTC_ALR 23451
#define ID_PIT     34512
#define ID_TBA     45123
#define ID_TBB     51234

#define CYG_InterruptHANDLED 1
#define PIT_PERIOD 5000
#define TB_PERIOD  (PIT_PERIOD*160)

// Factor 160 comes from setting SCCR = 0x0300. Thus, TBS = 1   -->> Time base is clocked by 
//                                                                   system clock / 16 = 40MHz / 16
//                                                    RTDIV = 1 -->> RTC/PIT clocked by OSCM / 256
//                                                                   or 4 MHz / 256
//                                                                   Factor of 160 between the two

volatile cyg_uint32 count = 0;
static int pit_count = 0;
static cyg_uint32 count_verify_table[] = {1, 4, 5, 41, 42};

// These are useful for debugging:
static cyg_uint32 count_actual_table[] = { -1, -1, -1, -1, -1};
static cyg_uint32 tbr_actual_table[] = { -1, -1, -1, -1, -1};

hal_mpc5xx_arbitration_data hal_arbitration_data_tb;
hal_mpc5xx_arbitration_data hal_arbitration_data_pit;

static cyg_uint32 isr_tba(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
	CYG_UNUSED_PARAM(CYG_ADDRWORD, data);
	
	CYG_ASSERT (CYGNUM_HAL_INTERRUPT_SIU_TB_A == vector, "Wrong vector!");
    CYG_ASSERT (ID_TBA == data, "Wrong data!");

	HAL_INTERRUPT_ACKNOWLEDGE(vector);
	count = count*3;

	return CYG_InterruptHANDLED;
}

static cyg_uint32 isr_tbb(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
	CYG_UNUSED_PARAM(CYG_ADDRWORD, data);
	
	CYG_ASSERT (CYGNUM_HAL_INTERRUPT_SIU_TB_B == vector, "Wrong vector!");
    CYG_ASSERT (ID_TBB == data, "Wrong data!");
		
	HAL_INTERRUPT_ACKNOWLEDGE(vector);
	count = count*8;

	return CYG_InterruptHANDLED;
}

static cyg_uint32
isr_pit(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
	cyg_uint32 verify_value;
	
	CYG_UNUSED_PARAM(CYG_ADDRWORD, data);
	
	CYG_ASSERT (CYGNUM_HAL_INTERRUPT_SIU_TB_B == vector, "Wrong vector!");
    CYG_ASSERT (ID_PIT == data, "Wrong data!");
	
	HAL_INTERRUPT_ACKNOWLEDGE(vector);

	count++;

	count_actual_table[pit_count] = count;
	{
		cyg_uint32 tbl;
        CYGARC_MFTB (TBL_R, tbl);
        tbr_actual_table[pit_count] = tbl;						
	}

	verify_value = count_verify_table[pit_count++];    
	
	CYG_ASSERT (count == verify_value, "Count wrong!");

	// End of test when count is 42. Mask interrupts and print PASS text.
	if (42 <= count || 5 == pit_count) {
	   HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SIU_PIT);
	   HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SIU_TB_A);
	   HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SIU_TB_B);

	   if(5 == pit_count)
	   {
		   if(42 != count) CYG_TEST_INFO("TB/PIT ratio does not match");
	   }

	   if(5 == pit_count && 42 == count)
	   {
           CYG_TEST_PASS_FINISH("Intr 0 OK");
	   }
	   else
	   {
           CYG_TEST_FAIL_FINISH("Intr 0 FAILED");
	   }
	}                             

	return CYG_InterruptHANDLED;
}

static void
intr0_main( void )
{
    int tb_period = TB_PERIOD;
	cyg_uint32 tbl;
	cyg_uint16 piscr;

	// Install the PIT Interrupt arbiter
	hal_arbitration_data_pit.priority = CYGNUM_HAL_ISR_SOURCE_PRIORITY_PIT;
	hal_arbitration_data_pit.data     = 0;
	hal_arbitration_data_pit.arbiter  = hal_arbitration_isr_pit;
	
	hal_mpc5xx_install_arbitration_isr(&hal_arbitration_data_pit);

	// attach PIT isr
	HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SIU_PIT, &isr_pit, ID_PIT, 0);
	HAL_INTERRUPT_SET_LEVEL (CYGNUM_HAL_INTERRUPT_SIU_PIT, CYGNUM_HAL_ISR_SOURCE_PRIORITY_PIT);

	// Set period
    HAL_WRITE_UINT32 (CYGARC_REG_IMM_PITC, (2*PIT_PERIOD) << CYGARC_REG_IMM_PITC_COUNT_SHIFT);

	// Enable.
    HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
    piscr |= CYGARC_REG_IMM_PISCR_PTE;
    HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
	
	// Clear any pending interrupts and enable them
	HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SIU_PIT);
	HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_PIT);
	
	// Install the Timebase Interrupt Arbiter
	hal_arbitration_data_tb.priority = CYGNUM_HAL_ISR_SOURCE_PRIORITY_TB;
	hal_arbitration_data_tb.data     = 0;
	hal_arbitration_data_tb.arbiter  = hal_arbitration_isr_tb;

	hal_mpc5xx_install_arbitration_isr(&hal_arbitration_data_tb);

	// Attach tb isrs.
	HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SIU_TB_A, &isr_tba, ID_TBA, 0);
	HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SIU_TB_B, &isr_tbb, ID_TBB, 0);
    HAL_INTERRUPT_SET_LEVEL (CYGNUM_HAL_INTERRUPT_SIU_TB_A, CYGNUM_HAL_ISR_SOURCE_PRIORITY_TB);

    // Set reference A & B registers.
    CYGARC_MFTB (TBL_R, tbl);
    tbl += tb_period*3;
    HAL_WRITE_UINT32 (CYGARC_REG_IMM_TBREF0, tbl);
    tbl += tb_period*4;
    HAL_WRITE_UINT32 (CYGARC_REG_IMM_TBREF1, tbl);

	// Clear any pending interrupts and enable them
	HAL_INTERRUPT_ACKNOWLEDGE(CYGNUM_HAL_INTERRUPT_SIU_TB_A);
	HAL_INTERRUPT_ACKNOWLEDGE(CYGNUM_HAL_INTERRUPT_SIU_TB_B);
	HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_TB_A);
	HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_TB_B);

	HAL_ENABLE_INTERRUPTS();

	for(;;);
}

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    intr0_main();
	CYG_TEST_PASS_FINISH("HAL Interrupt test");
}
