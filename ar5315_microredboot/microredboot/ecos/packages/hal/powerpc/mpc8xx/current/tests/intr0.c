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
// Author(s):     jskov
// Contributors:  jskov, gthomas
// Date:          1998-12-01
// Description:   Simple test of MPC860 interrupt handling when the
//                kernel has not been configured. Uses timer interrupts.
// Options:
//####DESCRIPTIONEND####

// #define DEBUG_PRINTFS
#ifdef DEBUG_PRINTFS
#include <cyg/infra/diag.h>
#endif

#include <pkgconf/hal.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>

#include <cyg/hal/hal_intr.h>

#include <cyg/infra/testcase.h>

#ifndef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
#ifdef CYGSEM_HAL_POWERPC_MPC860_CPM_ENABLE

#undef CHECK
#define CHECK(b) CYG_TEST_CHECK(b,#b)

// Can't rely on Cyg_Interrupt class being defined.
#define Cyg_InterruptHANDLED 1

// This is the period between interrupts, measured in decrementer ticks.
// Period must be longer than the time required for setting up all the
// interrupt handlers.
#define PIT_PERIOD 5000

#ifdef CYGPKG_HAL_POWERPC_MBX
#define TB_PERIOD (PIT_PERIOD*384)      // PTA period is 15.36 uS
#else
// This value is based on the relationship between the PIT clock
// and the TB clock.  This is set in the SCCR register and the
// default value seems to be that the TB runs 128 times faster
// than the PIT.  Of course, this doesn't match the documentation :-(
// Also, the basis for this is hardware strappable (set at reset time)
// so the value chosen below is a guess which works on the 860 platforms
// we have seen, other than the Motorola MBX860.
#define TB_PERIOD (PIT_PERIOD*128)       // assuming 512/4 divisors
#endif

#define PIT_IRQ_LEVEL 4
#define PIT_IRQ CYGNUM_HAL_INTERRUPT_SIU_LVL4
#define TB_IRQ_LEVEL 5
#define TB_IRQ CYGNUM_HAL_INTERRUPT_SIU_LVL5

#define ID_RTC_SEC   12345
#define ID_RTC_ALR   23451
#define ID_PIT       34512
#define ID_TBA       45123
#define ID_TBB       51234

volatile cyg_uint32 count = 0;

// Time/PERIOD    0   1   2   3   4   5   6   7   8   9   10
// Interrupt             PIT TBA PIT     PIT TBB PIT     PIT
// pit_count      0   0   0   1   1   2   2   3   3   4   4
// count          0   0   1   3   4   4   5   40  41      42

static cyg_uint32 count_verify_table[] = {1, 4, 5, 41, 42};
static int pit_count = 0;

// These are useful for debugging:
static cyg_uint32 count_actual_table[] = { -1, -1, -1, -1, -1};
static cyg_uint32 tbr_actual_table[] = { -1, -1, -1, -1, -1};

// Periodic timer ISR. Should be executing 5 times.
static cyg_uint32 isr_pit(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 verify_value;

    CYG_UNUSED_PARAM(CYG_ADDRWORD, data);

    CYG_ASSERT (CYGNUM_HAL_INTERRUPT_SIU_PIT == vector, "Wrong vector!");
    CYG_ASSERT (ID_PIT == data, "Wrong data!");

    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SIU_PIT);

    count++;

    count_actual_table[pit_count] = count;
    {
        cyg_uint32 tbl;
        CYGARC_MFTB (TBL_R, tbl);
        tbr_actual_table[pit_count] = tbl;
    }

    verify_value = count_verify_table[pit_count++];

#ifdef DEBUG_PRINTFS
    diag_printf( "ISR_PIT executed %d of 5\n", pit_count );
#endif

    CYG_ASSERT (count == verify_value, "Count wrong!");

    // End of test when count is 42. Mask interrupts and print PASS text.
    if (42 <= count || 5 == pit_count) {
        HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SIU_PIT);
        HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SIU_TB_A);
        HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SIU_TB_B);

#ifdef DEBUG_PRINTFS
        diag_printf( "INFO: Actual counts: %d %d %d %d %d\n",
                     count_actual_table[0],
                     count_actual_table[1],
                     count_actual_table[2],
                     count_actual_table[3],
                     count_actual_table[4] );
        diag_printf( "INFO: Actuals tbrs: %d %d %d %d %d\n",
                     tbr_actual_table[0],
                     tbr_actual_table[1],
                     tbr_actual_table[2],
                     tbr_actual_table[3],
                     tbr_actual_table[4] );
#endif
	if (5 == pit_count) {
#ifndef CYGPKG_HAL_POWERPC_MBX
            if (42 != count) {
#else
            if ((42 != count) && (49 != count)) {
#endif
                CYG_TEST_INFO("TB/PIT ratio does not match");
            }
	}
#ifndef CYGPKG_HAL_POWERPC_MBX
        if (42 == count && 5 == pit_count)
#else
        if (((42 == count) || (49 == count)) && (5 == pit_count))
#endif
            CYG_TEST_PASS_FINISH("Intr 0 OK");
        else
            CYG_TEST_FAIL_FINISH("Intr 0 Failed.");
    }

    return Cyg_InterruptHANDLED;
}

// TimeBase A ISR. Should be executing once.
static cyg_uint32 isr_tba(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    CYG_UNUSED_PARAM(CYG_ADDRWORD, data);

    CYG_ASSERT (CYGNUM_HAL_INTERRUPT_SIU_TB_A == vector, "Wrong vector!");
    CYG_ASSERT (ID_TBA == data, "Wrong data!");

    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SIU_TB_A);

    count = count * 3;

#ifdef DEBUG_PRINTFS
    diag_printf( "ISR_TBA executed, count now %d\n", count );
#endif

    return Cyg_InterruptHANDLED;
}

// TimeBase B ISR. Should be executing once.
static cyg_uint32 isr_tbb(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    CYG_UNUSED_PARAM(CYG_ADDRWORD, data);

    CYG_ASSERT (CYGNUM_HAL_INTERRUPT_SIU_TB_B == vector, "Wrong vector!");
    CYG_ASSERT (ID_TBB == data, "Wrong data!");

    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SIU_TB_B);

    count = count * 8;

#ifdef DEBUG_PRINTFS
    diag_printf( "ISR_TBB executed, count now %d\n", count );
#endif

    return Cyg_InterruptHANDLED;
}

void intr0_main( void )
{
#ifndef CYGPKG_HAL_POWERPC_MBX
    unsigned long sccr = *(volatile unsigned long *)CYGARC_REG_IMM_SCCR;
#endif
    int tb_period = TB_PERIOD;
    CYG_TEST_INIT();

#ifndef CYGPKG_HAL_POWERPC_MBX
#ifdef DEBUG_PRINTFS
    diag_printf("sccr = %x\n", sccr);
#endif
    if (sccr & 0x01000000) tb_period /= 4;
#endif

#if 0
    // The A.3 revision of the CPU I'm using at the moment generates a
    // machine check exception when writing to IMM_RTCSC.  Smells a
    // bit like the "SIU4. Spurious External Bus Transaction Following
    // PLPRCR Write." CPU errata. Have to find out for sure.  Run real
    // time clock interrupts on level 0
    {
        // Still to do.
    }
#endif

    // Run periodic timer interrupt on level 1
    {
        cyg_uint16 piscr;

        // Attach pit arbiter.
        HAL_INTERRUPT_ATTACH (PIT_IRQ,
                              &hal_arbitration_isr_pit, ID_PIT, 0);
        HAL_INTERRUPT_UNMASK (PIT_IRQ);

        // Attach pit isr.
        HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SIU_PIT, &isr_pit,
                              ID_PIT, 0);
        HAL_INTERRUPT_SET_LEVEL (CYGNUM_HAL_INTERRUPT_SIU_PIT, PIT_IRQ_LEVEL);
        HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_PIT);


        // Set period.
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_PITC, 
                          (2*PIT_PERIOD) << CYGARC_REG_IMM_PITC_COUNT_SHIFT);

#ifdef DEBUG_PRINTFS
        diag_printf( "PIT set to %d\n", 2*PIT_PERIOD );
#endif
        // Enable.
        HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        piscr |= CYGARC_REG_IMM_PISCR_PTE;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
    }

    // Run timebase interrupts on level 2
    {
        cyg_uint16 tbscr;
        cyg_uint32 tbl;

        // Attach tb arbiter.
        HAL_INTERRUPT_ATTACH (TB_IRQ, 
                              &hal_arbitration_isr_tb, ID_TBA, 0);
        HAL_INTERRUPT_UNMASK (TB_IRQ);

        // Attach tb isrs.
        HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SIU_TB_A, &isr_tba,
                              ID_TBA, 0);
        HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SIU_TB_B, &isr_tbb,
                              ID_TBB, 0);
        HAL_INTERRUPT_SET_LEVEL (CYGNUM_HAL_INTERRUPT_SIU_TB_A, TB_IRQ_LEVEL);
        HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_TB_A);
        HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_TB_B);

        // Set reference A & B registers.
        CYGARC_MFTB (TBL_R, tbl);
        tbl += tb_period*3;
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_TBREF0, tbl);
        tbl += tb_period*4;
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_TBREF1, tbl);

#ifdef DEBUG_PRINTFS
        diag_printf( "TB initial %d, !1 %d !2 %d\n",
                     tbl - 7*tb_period,
                     tbl - 4*tb_period,
                     tbl - 0*tb_period );
#endif
        // Enable.
        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= (CYGARC_REG_IMM_TBSCR_REFA | CYGARC_REG_IMM_TBSCR_REFB |
                  CYGARC_REG_IMM_TBSCR_TBE);
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= CYGARC_REG_IMM_TBSCR_REFAE | CYGARC_REG_IMM_TBSCR_REFBE;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
    }

    HAL_ENABLE_INTERRUPTS();

    for (;;);
}

externC void
cyg_start( void )
{
    intr0_main();
}

#else  // ifdef CYGSEM_HAL_POWERPC_MPC860_CPM_ENABLE

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_PASS_FINISH("N/A: CYGSEM_HAL_POWERPC_MPC860_CPM_ENABLE disabled");
}

#endif // ifdef CYGSEM_HAL_POWERPC_MPC860_CPM_ENABLE
#else

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_PASS_FINISH("N/A: CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN enabled");
}
#endif // ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

// EOF intr0.c
