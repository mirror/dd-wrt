#ifndef CYGONCE_VAR_INTR_H
#define CYGONCE_VAR_INTR_H
//=============================================================================
//
//      var_intr.h
//
//      Variant HAL interrupt and clock support
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
// Copyright (C) 2003 Jonathan Larmour <jifl@eCosCentric.com>
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
// Author(s):   nickg
// Contributors:nickg, jskov, jlarmour, hmt, gthomas
// Date:        2000-04-02
// Purpose:     Variant interrupt support
// Description: The macros defined here provide the HAL APIs for handling
//              interrupts and the clock on the MPC8xx variant CPUs.
// Usage:       Is included via the architecture interrupt header:
//              #include <cyg/hal/hal_intr.h>
//              ...
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/plf_intr.h>

#include <cyg/infra/cyg_type.h>         // types

#include <cyg/hal/ppc_regs.h>           // register definitions

#include <cyg/hal/hal_io.h>             // io macros
#include <cyg/infra/cyg_ass.h>          // CYG_FAIL

//-----------------------------------------------------------------------------
// Exception vectors.

// Additional exceptions on the MPC8xx CPUs
#define CYGNUM_HAL_VECTOR_RESERVED_F         15
#define CYGNUM_HAL_VECTOR_SW_EMUL            16
#define CYGNUM_HAL_VECTOR_ITLB_MISS          17
#define CYGNUM_HAL_VECTOR_DTLB_MISS          18
#define CYGNUM_HAL_VECTOR_ITLB_ERROR         19
#define CYGNUM_HAL_VECTOR_DTLB_ERROR         20
#define CYGNUM_HAL_VECTOR_RESERVED_15        21
#define CYGNUM_HAL_VECTOR_RESERVED_16        22
#define CYGNUM_HAL_VECTOR_RESERVED_17        23
#define CYGNUM_HAL_VECTOR_RESERVED_18        24
#define CYGNUM_HAL_VECTOR_RESERVED_19        25
#define CYGNUM_HAL_VECTOR_RESERVED_1A        26
#define CYGNUM_HAL_VECTOR_RESERVED_1B        27
#define CYGNUM_HAL_VECTOR_DATA_BP            28
#define CYGNUM_HAL_VECTOR_INSTRUCTION_BP     29
#define CYGNUM_HAL_VECTOR_PERIPHERAL_BP      30
#define CYGNUM_HAL_VECTOR_NMI                31

#define CYGNUM_HAL_VSR_MAX                   CYGNUM_HAL_VECTOR_NMI

// These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_RESERVED_0      CYGNUM_HAL_VECTOR_RESERVED_0
#define CYGNUM_HAL_EXCEPTION_MACHINE_CHECK   CYGNUM_HAL_VECTOR_MACHINE_CHECK
#ifdef CYGHWR_HAL_POWERPC_MPC8XX
// The MPC860 does not generate DSI and ISI: instead it goes to machine
// check, so that a software VM system can then call into vectors 0x300 or
// 0x400 if the address is truly invalid rather than merely not in the TLB
// right now.  Shades of IBM wanting to port OS/MVS here!
// See pp 7-9/10 in "PowerQUICC - MPC860 User's Manual"
# undef CYGNUM_HAL_EXCEPTION_DATA_ACCESS
# define CYGNUM_HAL_EXCEPTION_DATA_ACCESS    CYGNUM_HAL_VECTOR_MACHINE_CHECK
// do not define catchers for DSI and ISI - should never happen.
# undef CYGNUM_HAL_EXCEPTION_MACHINE_CHECK
# undef CYGNUM_HAL_EXCEPTION_CODE_ACCESS
#else
# define CYGNUM_HAL_EXCEPTION_DATA_ACCESS    CYGNUM_HAL_VECTOR_DSI
# define CYGNUM_HAL_EXCEPTION_CODE_ACCESS    CYGNUM_HAL_VECTOR_ISI
#endif
#define CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS  \
           CYGNUM_HAL_VECTOR_ALIGNMENT
#define CYGNUM_HAL_EXCEPTION_FPU_NOT_AVAIL   CYGNUM_HAL_VECTOR_FP_UNAVAILABLE
#define CYGNUM_HAL_EXCEPTION_RESERVED_A      CYGNUM_HAL_VECTOR_RESERVED_A
#define CYGNUM_HAL_EXCEPTION_RESERVED_B      CYGNUM_HAL_VECTOR_RESERVED_B
#define CYGNUM_HAL_EXCEPTION_SYSTEM_CALL     CYGNUM_HAL_VECTOR_SYSTEM_CALL
#define CYGNUM_HAL_EXCEPTION_TRACE           CYGNUM_HAL_VECTOR_TRACE
#define CYGNUM_HAL_EXCEPTION_FP_ASSIST       CYGNUM_HAL_VECTOR_FP_ASSIST
#define CYGNUM_HAL_EXCEPTION_RESERVED_F      CYGNUM_HAL_VECTOR_RESERVED_F
#define CYGNUM_HAL_EXCEPTION_SW_EMUL         CYGNUM_HAL_VECTOR_SW_EMUL
#define CYGNUM_HAL_EXCEPTION_CODE_TLBMISS_ACCESS  CYGNUM_HAL_VECTOR_ITLB_MISS
#define CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS  CYGNUM_HAL_VECTOR_DTLB_MISS
#define CYGNUM_HAL_EXCEPTION_CODE_TLBERROR_ACCESS \
               CYGNUM_HAL_VECTOR_ITLB_ERROR
#define CYGNUM_HAL_EXCEPTION_DATA_TLBERROR_ACCESS \
           CYGNUM_HAL_VECTOR_DTLB_ERROR
#define CYGNUM_HAL_EXCEPTION_RESERVED_15     CYGNUM_HAL_VECTOR_RESERVED_15
#define CYGNUM_HAL_EXCEPTION_RESERVED_16     CYGNUM_HAL_VECTOR_RESERVED_16
#define CYGNUM_HAL_EXCEPTION_RESERVED_17     CYGNUM_HAL_VECTOR_RESERVED_17
#define CYGNUM_HAL_EXCEPTION_RESERVED_18     CYGNUM_HAL_VECTOR_RESERVED_18
#define CYGNUM_HAL_EXCEPTION_RESERVED_19     CYGNUM_HAL_VECTOR_RESERVED_19
#define CYGNUM_HAL_EXCEPTION_RESERVED_1A     CYGNUM_HAL_VECTOR_RESERVED_1A
#define CYGNUM_HAL_EXCEPTION_RESERVED_1B     CYGNUM_HAL_VECTOR_RESERVED_1B
#define CYGNUM_HAL_EXCEPTION_DATA_BP         CYGNUM_HAL_VECTOR_DATA_BP
#define CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP  CYGNUM_HAL_VECTOR_INSTRUCTION_BP
#define CYGNUM_HAL_EXCEPTION_PERIPHERAL_BP   CYGNUM_HAL_VECTOR_PERIPHERAL_BP
#define CYGNUM_HAL_EXCEPTION_NMI             CYGNUM_HAL_VECTOR_NMI

#define CYGNUM_HAL_EXCEPTION_MIN             CYGNUM_HAL_EXCEPTION_RESERVED_0
#define CYGNUM_HAL_EXCEPTION_MAX             CYGNUM_HAL_EXCEPTION_NMI

#define CYGHWR_HAL_EXCEPTION_VECTORS_DEFINED

//-----------------------------------------------------------------------------
// Interrupts

// The first level of external interrupts
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ0            1
#define CYGNUM_HAL_INTERRUPT_SIU_LVL0            2
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ1            3
#define CYGNUM_HAL_INTERRUPT_SIU_LVL1            4
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ2            5
#define CYGNUM_HAL_INTERRUPT_SIU_LVL2            6
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ3            7
#define CYGNUM_HAL_INTERRUPT_SIU_LVL3            8
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ4            9
#define CYGNUM_HAL_INTERRUPT_SIU_LVL4           10
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ5           11
#define CYGNUM_HAL_INTERRUPT_SIU_LVL5           12
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ6           13
#define CYGNUM_HAL_INTERRUPT_SIU_LVL6           14
#define CYGNUM_HAL_INTERRUPT_SIU_IRQ7           15
#define CYGNUM_HAL_INTERRUPT_SIU_LVL7           16

// Further decoded interrups
#define CYGNUM_HAL_INTERRUPT_SIU_TB_A           17
#define CYGNUM_HAL_INTERRUPT_SIU_TB_B           18
#define CYGNUM_HAL_INTERRUPT_SIU_PIT            19
#define CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC        20
#define CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR        21
#define CYGNUM_HAL_INTERRUPT_SIU_PCMCIA_A_IRQ   22
#define CYGNUM_HAL_INTERRUPT_SIU_PCMCIA_A_CHLVL 23
#define CYGNUM_HAL_INTERRUPT_SIU_PCMCIA_B_IRQ   24
#define CYGNUM_HAL_INTERRUPT_SIU_PCMCIA_B_CHLVL 25
#define CYGNUM_HAL_INTERRUPT_SIU_CPM            26

// Even further...
#define CYGNUM_HAL_INTERRUPT_CPM_PC15           27
#define CYGNUM_HAL_INTERRUPT_CPM_SCC1           28
#define CYGNUM_HAL_INTERRUPT_CPM_SCC2           29
#define CYGNUM_HAL_INTERRUPT_CPM_SCC3           30
#define CYGNUM_HAL_INTERRUPT_CPM_SCC4           31
#define CYGNUM_HAL_INTERRUPT_CPM_PC14           32
#define CYGNUM_HAL_INTERRUPT_CPM_TIMER1         33
#define CYGNUM_HAL_INTERRUPT_CPM_PC13           34
#define CYGNUM_HAL_INTERRUPT_CPM_PC12           35
#define CYGNUM_HAL_INTERRUPT_CPM_SDMA           36
#define CYGNUM_HAL_INTERRUPT_CPM_IDMA1          37
#define CYGNUM_HAL_INTERRUPT_CPM_IDMA2          38
#define CYGNUM_HAL_INTERRUPT_CPM_RESERVED_13    39
#define CYGNUM_HAL_INTERRUPT_CPM_TIMER2         40
#define CYGNUM_HAL_INTERRUPT_CPM_RISCTT         41
#define CYGNUM_HAL_INTERRUPT_CPM_I2C            42
#define CYGNUM_HAL_INTERRUPT_CPM_PC11           43
#define CYGNUM_HAL_INTERRUPT_CPM_PC10           44
#define CYGNUM_HAL_INTERRUPT_CPM_RESERVED_0D    45
#define CYGNUM_HAL_INTERRUPT_CPM_TIMER3         46
#define CYGNUM_HAL_INTERRUPT_CPM_PC9            47
#define CYGNUM_HAL_INTERRUPT_CPM_PC8            48
#define CYGNUM_HAL_INTERRUPT_CPM_PC7            49
#define CYGNUM_HAL_INTERRUPT_CPM_RESERVED_08    50
#define CYGNUM_HAL_INTERRUPT_CPM_TIMER4         51
#define CYGNUM_HAL_INTERRUPT_CPM_PC6            52
#define CYGNUM_HAL_INTERRUPT_CPM_SPI            53
#define CYGNUM_HAL_INTERRUPT_CPM_SMC1           54
#define CYGNUM_HAL_INTERRUPT_CPM_SMC2_PIP       55
#define CYGNUM_HAL_INTERRUPT_CPM_PC5            56
#define CYGNUM_HAL_INTERRUPT_CPM_PC4            57
#define CYGNUM_HAL_INTERRUPT_CPM_ERROR          58

#define CYGNUM_HAL_INTERRUPT_CPM_FIRST       CYGNUM_HAL_INTERRUPT_CPM_PC15
#define CYGNUM_HAL_INTERRUPT_CPM_LAST        CYGNUM_HAL_INTERRUPT_CPM_ERROR

#define CYGNUM_HAL_ISR_MAX                   CYGNUM_HAL_INTERRUPT_CPM_LAST
#endif


//--------------------------------------------------------------------------
// Interrupt controller access

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#ifdef CYGHWR_HAL_POWERPC_MPC8XX

static __inline__ void
cyg_hal_interrupt_mask ( cyg_uint32 vector )
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ0 ... CYGNUM_HAL_INTERRUPT_SIU_LVL7:
    {
        // SIU interrupt vectors
        cyg_uint32 simask;

        HAL_READ_UINT32 (CYGARC_REG_IMM_SIMASK, simask);
        simask &= ~(((cyg_uint32) CYGARC_REG_IMM_SIMASK_IRQ0) 
                    >> (vector - CYGNUM_HAL_INTERRUPT_SIU_IRQ0));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_SIMASK, simask);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_A:
    {
        // TimeBase A interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFAE);
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_B:
    {
        // TimeBase B interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFBE);
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_PIT:
    {
        // Periodic Interrupt
        cyg_uint16 piscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        piscr &= ~(CYGARC_REG_IMM_PISCR_PIE);
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC:
    {
        // Real Time Clock Second
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SIE);
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    {
        // Real Time Clock Alarm
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALE);
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    // PCMCIA_A_IRQ
    // PCMCIA_A_CHLVL
    // PCMCIA_B_IRQ
    // PCMCIA_B_CHLVL
    case CYGNUM_HAL_INTERRUPT_SIU_CPM:
    {
        // Communications Processor Module
        cyg_uint32 cicr;

        HAL_READ_UINT32 (CYGARC_REG_IMM_CICR, cicr);
        cicr &= ~(CYGARC_REG_IMM_CICR_IEN);
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_CICR, cicr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_CPM_FIRST ... CYGNUM_HAL_INTERRUPT_CPM_LAST:
    {
        // CPM interrupts
        cyg_uint32 cimr;

        HAL_READ_UINT32 (CYGARC_REG_IMM_CIMR, cimr);
        cimr &= ~(((cyg_uint32) 0x80000000) 
                  >> (vector - CYGNUM_HAL_INTERRUPT_CPM_FIRST));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_CIMR, cimr);
        break;
    }
    default:
        CYG_FAIL("Unknown Interrupt!!!");
        break;
    }
}

static __inline__ void
cyg_hal_interrupt_unmask ( cyg_uint32 vector )
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ0 ... CYGNUM_HAL_INTERRUPT_SIU_LVL7:
    {
        // SIU interrupt vectors
        cyg_uint32 simask;

        HAL_READ_UINT32 (CYGARC_REG_IMM_SIMASK, simask);
        simask |= (((cyg_uint32) CYGARC_REG_IMM_SIMASK_IRQ0) 
                   >> (vector - CYGNUM_HAL_INTERRUPT_SIU_IRQ0));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_SIMASK, simask);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_A:
    {
        // TimeBase A interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= CYGARC_REG_IMM_TBSCR_REFAE;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_B:
    {
        // TimeBase B interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= CYGARC_REG_IMM_TBSCR_REFBE;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_PIT:
    {
        // Periodic Interrupt
        cyg_uint16 piscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        piscr |= CYGARC_REG_IMM_PISCR_PIE;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC:
    {
        // Real Time Clock Second
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc |= CYGARC_REG_IMM_RTCSC_SIE;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    {
        // Real Time Clock Alarm
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc |= CYGARC_REG_IMM_RTCSC_ALE;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    // PCMCIA_A_IRQ
    // PCMCIA_A_CHLVL
    // PCMCIA_B_IRQ
    // PCMCIA_B_CHLVL
    case CYGNUM_HAL_INTERRUPT_SIU_CPM:
    {
        // Communications Processor Module
        cyg_uint32 cicr;

        HAL_READ_UINT32 (CYGARC_REG_IMM_CICR, cicr);
        cicr |= CYGARC_REG_IMM_CICR_IEN;
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_CICR, cicr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_CPM_FIRST ... CYGNUM_HAL_INTERRUPT_CPM_LAST:
    {
        // CPM interrupts
        cyg_uint32 cimr;

        HAL_READ_UINT32 (CYGARC_REG_IMM_CIMR, cimr);
        cimr |= (((cyg_uint32) 0x80000000) 
                 >> (vector - CYGNUM_HAL_INTERRUPT_CPM_FIRST));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_CIMR, cimr);
        break;
    }
    default:
        CYG_FAIL("Unknown Interrupt!!!");
        break;
    }
}

static __inline__ void
cyg_hal_interrupt_acknowledge ( cyg_uint32 vector )
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ0 ... CYGNUM_HAL_INTERRUPT_SIU_LVL7:
    {
        // SIU interrupt vectors
        cyg_uint32 sipend;

        // When IRQx is configured as an edge interrupt it needs to be
        // cleared. Write to INTx and IRQ/level bits are ignore so
        // it's safe to do always.
        sipend = (((cyg_uint32) CYGARC_REG_IMM_SIPEND_IRQ0) 
                   >> (vector - CYGNUM_HAL_INTERRUPT_SIU_IRQ0));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_SIPEND, sipend);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_A:
    {
        // TimeBase A interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= CYGARC_REG_IMM_TBSCR_REFA;
        // take care not to accidently reset potential pending REFB
        tbscr &= ~CYGARC_REG_IMM_TBSCR_REFB;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_B:
    {
        // TimeBase B interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= CYGARC_REG_IMM_TBSCR_REFB;
        // take care not to accidently reset potential pending REFA
        tbscr &= ~CYGARC_REG_IMM_TBSCR_REFA;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_PIT:
    {
        // Periodic Interrupt
        cyg_uint16 piscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        piscr |= CYGARC_REG_IMM_PISCR_PS;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC:
    {
        // Real Time Clock Second
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc |= CYGARC_REG_IMM_RTCSC_SEC;
        // take care not to accidently reset potential pending RTCSC_ALR
        rtcsc &= ~CYGARC_REG_IMM_RTCSC_ALR;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    {
        // Real Time Clock Alarm
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc |= CYGARC_REG_IMM_RTCSC_ALR;
        // take care not to accidently reset potential pending RTCSC_SEC
        rtcsc &= ~CYGARC_REG_IMM_RTCSC_SEC;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    // PCMCIA_A_IRQ
    // PCMCIA_A_CHLVL
    // PCMCIA_B_IRQ
    // PCMCIA_B_CHLVL
    case CYGNUM_HAL_INTERRUPT_SIU_CPM:
        // Communications Processor Module
        // The CPM interrupts must be acknowledged individually.
        break;
    case CYGNUM_HAL_INTERRUPT_CPM_FIRST ... CYGNUM_HAL_INTERRUPT_CPM_LAST:
    {
        // CPM interrupts
        cyg_uint32 cisr;

        cisr = (((cyg_uint32) 0x80000000) 
                 >> (vector - CYGNUM_HAL_INTERRUPT_CPM_FIRST));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_CISR, cisr);
        break;
    }
    default:
        CYG_FAIL("Unknown Interrupt!!!");
        break;
    }
}

static __inline__ void
cyg_hal_interrupt_configure ( cyg_uint32 vector,
                              cyg_bool level,
                              cyg_bool up )
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ0:
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ1:
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ2:
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ3:
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ4:
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ5:
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ6:
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ7:
    {
        // External interrupts
        cyg_uint32 siel, bit;

        CYG_ASSERT( level || !up, "Only falling edge is supported");    

        bit = (((cyg_uint32) CYGARC_REG_IMM_SIEL_IRQ0) 
               >> (vector - CYGNUM_HAL_INTERRUPT_SIU_IRQ0));

        HAL_READ_UINT32 (CYGARC_REG_IMM_SIEL, siel);
        siel &= ~bit;
        if (!level) {
            // Set edge detect bit.
            siel |= bit;
        }
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_SIEL, siel);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_LVL0:
    case CYGNUM_HAL_INTERRUPT_SIU_LVL1:
    case CYGNUM_HAL_INTERRUPT_SIU_LVL2:
    case CYGNUM_HAL_INTERRUPT_SIU_LVL3:
    case CYGNUM_HAL_INTERRUPT_SIU_LVL4:
    case CYGNUM_HAL_INTERRUPT_SIU_LVL5:
    case CYGNUM_HAL_INTERRUPT_SIU_LVL6:
    case CYGNUM_HAL_INTERRUPT_SIU_LVL7:
    case CYGNUM_HAL_INTERRUPT_SIU_TB_A:
    case CYGNUM_HAL_INTERRUPT_SIU_TB_B:
    case CYGNUM_HAL_INTERRUPT_SIU_PIT:
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC:
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    // PCMCIA_A_IRQ
    // PCMCIA_A_CHLVL
    // PCMCIA_B_IRQ
    // PCMCIA_B_CHLVL
    case CYGNUM_HAL_INTERRUPT_SIU_CPM:
    case CYGNUM_HAL_INTERRUPT_CPM_FIRST ... CYGNUM_HAL_INTERRUPT_CPM_LAST:
        // These cannot be configured.
        break;

    default:
        CYG_FAIL("Unknown Interrupt!!!");
        break;
    }
}

static __inline__ void
cyg_hal_interrupt_set_level ( cyg_uint32 vector, cyg_uint32 level )
{
    // Note: highest priority has the lowest numerical value.
    CYG_ASSERT( level >= CYGARC_SIU_PRIORITY_HIGH, "Invalid priority");
    CYG_ASSERT( level <= CYGARC_SIU_PRIORITY_LOW, "Invalid priority");

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ0 ... CYGNUM_HAL_INTERRUPT_SIU_LVL7:
        // These cannot be configured.
        break;
    case CYGNUM_HAL_INTERRUPT_SIU_TB_A:
    case CYGNUM_HAL_INTERRUPT_SIU_TB_B:
    {
        // TimeBase A+B interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_IRQMASK);
        tbscr |= CYGARC_REG_IMM_TBSCR_IRQ0 >> level;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_PIT:
    {
        // Periodic Interrupt
        cyg_uint16 piscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        piscr &= ~(CYGARC_REG_IMM_PISCR_IRQMASK);
        piscr |= CYGARC_REG_IMM_PISCR_IRQ0 >> level;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC:
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    {
        // Real Time Clock Second & Real Time Clock Alarm
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_IRQMASK);
        rtcsc |= CYGARC_REG_IMM_RTCSC_IRQ0 >> level;
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    // PCMCIA_A_IRQ
    // PCMCIA_A_CHLVL
    // PCMCIA_B_IRQ
    // PCMCIA_B_CHLVL
    case CYGNUM_HAL_INTERRUPT_SIU_CPM:
    {
        // Communications Processor Module
        cyg_uint32 cicr;

        HAL_READ_UINT32 (CYGARC_REG_IMM_CICR, cicr);
        cicr &= ~(CYGARC_REG_IMM_CICR_IRQMASK);
        cicr |= level << CYGARC_REG_IMM_CICR_IRQ_SHIFT;
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_CICR, cicr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_CPM_FIRST ... CYGNUM_HAL_INTERRUPT_CPM_LAST:
        // CPM interrupt levels are for internal priority. All interrupts
        // fed to the CPU use the CPM level set above.
        // FIXME: Control of SCdP ordering.
        break;
    default:
        CYG_FAIL("Unknown Interrupt!!!");
        break;
    }
}

// The decrementer interrupt cannnot be masked, configured or acknowledged.

#define HAL_INTERRUPT_MASK( _vector_ )                    \
    CYG_MACRO_START                                       \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))   \
        cyg_hal_interrupt_mask ( (_vector_) );            \
    CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )                  \
    CYG_MACRO_START                                       \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))   \
        cyg_hal_interrupt_unmask ( (_vector_) );          \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )             \
    CYG_MACRO_START                                       \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))   \
        cyg_hal_interrupt_acknowledge ( (_vector_) );     \
    CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )              \
    CYG_MACRO_START                                                     \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))                 \
        cyg_hal_interrupt_configure ( (_vector_), (_level_), (_up_) );  \
    CYG_MACRO_END

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )            \
    CYG_MACRO_START                                             \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))         \
        cyg_hal_interrupt_set_level ( (_vector_) , (_level_) ); \
    CYG_MACRO_END

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#endif

//--------------------------------------------------------------------------
// Interrupt arbiters

#ifdef CYGHWR_HAL_POWERPC_MPC8XX

externC cyg_uint32 hal_arbitration_isr_tb (CYG_ADDRWORD vector, 
                                           CYG_ADDRWORD data);
externC cyg_uint32 hal_arbitration_isr_pit (CYG_ADDRWORD vector,
                                            CYG_ADDRWORD data);
externC cyg_uint32 hal_arbitration_isr_rtc (CYG_ADDRWORD vector,
                                            CYG_ADDRWORD data);
externC cyg_uint32 hal_arbitration_isr_cpm (CYG_ADDRWORD vector,
                                            CYG_ADDRWORD data);

#endif // ifdef CYGHWR_HAL_POWERPC_MPC8XX

//-----------------------------------------------------------------------------
// Symbols used by assembly code
#define CYGARC_VARIANT_DEFS                                     \
    DEFINE(CYGNUM_HAL_VECTOR_NMI, CYGNUM_HAL_VECTOR_NMI);

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_INTR_H
// End of var_intr.h
