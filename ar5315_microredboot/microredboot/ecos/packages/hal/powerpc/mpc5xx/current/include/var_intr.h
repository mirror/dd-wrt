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
// Author(s):   Bob Koninckx
// Contributors:Bob Koninckx
// Date:        2001-12-15
// Purpose:     Variant interrupt support
// Description: The macros defined here provide the HAL APIs for handling
//              interrupts and the clock on the MPC5xx variant CPUs.
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
// Special IMB3 arbitration code
typedef struct t_hal_mpc5xx_arbitration_data {
  cyg_uint32    priority;
  CYG_ADDRWORD  data;
  cyg_uint32   (* arbiter)(CYG_ADDRWORD, CYG_ADDRWORD);
  void *        reserved;
} hal_mpc5xx_arbitration_data;

externC void 
hal_mpc5xx_install_arbitration_isr(hal_mpc5xx_arbitration_data *adata);
  
externC hal_mpc5xx_arbitration_data *
hal_mpc5xx_remove_arbitration_isr(cyg_uint32 apriority);

//-----------------------------------------------------------------------------
// Exception vectors.

// Additional exceptions on the MPC5xx CPUs
#define CYGNUM_HAL_VECTOR_RESERVED_F         15
#define CYGNUM_HAL_VECTOR_SW_EMUL            16
#define CYGNUM_HAL_VECTOR_RESERVED_11        17
#define CYGNUM_HAL_VECTOR_RESERVED_12        18
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
#define CYGNUM_HAL_EXCEPTION_RESERVED_0             CYGNUM_HAL_VECTOR_RESERVED_0
#define CYGNUM_HAL_EXCEPTION_MACHINE_CHECK          CYGNUM_HAL_VECTOR_MACHINE_CHECK
#define CYGNUM_HAL_EXCEPTION_RESERVED_3             CYGNUM_HAL_VECTOR_RESERVED_3
#define CYGNUM_HAL_EXCEPTION_RESERVED_4             CYGNUM_HAL_VECTOR_RESERVED_4
#define CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS  CYGNUM_HAL_VECTOR_ALIGNMENT 
#define CYGNUM_HAL_EXCEPTION_FPU_NOT_AVAIL          CYGNUM_HAL_VECTOR_FP_UNAVAILABLE
#define CYGNUM_HAL_EXCEPTION_RESERVED_A             CYGNUM_HAL_VECTOR_RESERVED_A
#define CYGNUM_HAL_EXCEPTION_RESERVED_B             CYGNUM_HAL_VECTOR_RESERVED_B
#define CYGNUM_HAL_EXCEPTION_SYSTEM_CALL            CYGNUM_HAL_VECTOR_SYSTEM_CALL
#define CYGNUM_HAL_EXCEPTION_TRACE                  CYGNUM_HAL_VECTOR_TRACE
#define CYGNUM_HAL_EXCEPTION_FP_ASSIST              CYGNUM_HAL_VECTOR_FP_ASSIST
#define CYGNUM_HAL_EXCEPTION_RESERVED_F             CYGNUM_HAL_VECTOR_RESERVED_F
#define CYGNUM_HAL_EXCEPTION_SW_EMUL                CYGNUM_HAL_VECTOR_SW_EMUL
#define CYGNUM_HAL_EXCEPTION_RESERVED_11            CYGNUM_HAL_VECTOR_RESERVED_11
#define CYGNUM_HAL_EXCEPTION_RESERVED_12            CYGNUM_HAL_VECTOR_RESERVED_12
#define CYGNUM_HAL_EXCEPTION_CODE_TLBERROR_ACCESS   CYGNUM_HAL_VECTOR_ITLB_ERROR
#define CYGNUM_HAL_EXCEPTION_DATA_TLBERROR_ACCESS   CYGNUM_HAL_VECTOR_DTLB_ERROR
#define CYGNUM_HAL_EXCEPTION_RESERVED_15            CYGNUM_HAL_VECTOR_RESERVED_15
#define CYGNUM_HAL_EXCEPTION_RESERVED_16            CYGNUM_HAL_VECTOR_RESERVED_16
#define CYGNUM_HAL_EXCEPTION_RESERVED_17            CYGNUM_HAL_VECTOR_RESERVED_17
#define CYGNUM_HAL_EXCEPTION_RESERVED_18            CYGNUM_HAL_VECTOR_RESERVED_18
#define CYGNUM_HAL_EXCEPTION_RESERVED_19            CYGNUM_HAL_VECTOR_RESERVED_19
#define CYGNUM_HAL_EXCEPTION_RESERVED_1A            CYGNUM_HAL_VECTOR_RESERVED_1A
#define CYGNUM_HAL_EXCEPTION_RESERVED_1B            CYGNUM_HAL_VECTOR_RESERVED_1B
#define CYGNUM_HAL_EXCEPTION_DATA_BP                CYGNUM_HAL_VECTOR_DATA_BP
#define CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP         CYGNUM_HAL_VECTOR_INSTRUCTION_BP
#define CYGNUM_HAL_EXCEPTION_PERIPHERAL_BP          CYGNUM_HAL_VECTOR_PERIPHERAL_BP
#define CYGNUM_HAL_EXCEPTION_NMI                    CYGNUM_HAL_VECTOR_NMI

// decoded exception vectors (decoded program exception)
#define CYGNUM_HAL_EXCEPTION_TRAP                    (-1)
#define CYGNUM_HAL_EXCEPTION_PRIVILEGED_INSTRUCTION  (-2)
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION     (-3)
#define CYGNUM_HAL_EXCEPTION_FPU                     (-4)

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

// Further decoded interrupts
#define CYGNUM_HAL_INTERRUPT_SIU_TB_A           17  // Time base reference A
#define CYGNUM_HAL_INTERRUPT_SIU_TB_B           18  // Time base reference B
#define CYGNUM_HAL_INTERRUPT_SIU_PIT            19  // Periodic interrupt timer
#define CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC        20  // Real time clock once per second
#define CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR        21  // Real time clock alarm
#define CYGNUM_HAL_INTERRUPT_SIU_COL            22  // Change of lock of the PLL

// Even further decoded interrupts
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI1    23  // QUADCA queue 1 completion
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI1    24  // QUADCA queue 1 pause
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI2    25  // QUADCA queue 2 completion
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI2    26  // QUADCA queue 2 pause
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI1    27  // QUADCB queue 1 completion
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI1    28  // QUADCB queue 1 pause
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI2    29  // QUADCB queue 2 completion
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI2    30  // QUADCB queue 2 pause
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX       31  // SCI 0 transmit
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TXC      32  // SCI 0 transmit complete
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX       33  // SCI 0 receiver full
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_IDLE     34  // SCI 0 idle line detected
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX       35  // SCI 1 transmit
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXC      36  // SCI 1 transmit complete
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX       37  // SCI 1 receiver full
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_IDLE     38  // SCI 1 idle line detected
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQTHF   39  // SCI 1 RX Queue top half full
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQBHF   40  // SCI 1 RX Queue bottom half full
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQTHE   41  // SCI 1 TX Queue top half full
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQBHE   42  // SCI 1 TX Queue bottom half full
#define CYGNUM_HAL_INTERRUPT_IMB3_SPI_FI        43  // SPI finished
#define CYGNUM_HAL_INTERRUPT_IMB3_SPI_MODF      44  // SPI Mode fault
#define CYGNUM_HAL_INTERRUPT_IMB3_SPI_HALTA     45  // SPI Halt Acknowledge
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_BOFF  46  // TOUCANA buss off
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_ERR   47  // TOUCANA error
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_WU    48  // TOUCANA wake up
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0    49  // TOUCANA buffer 0
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B1    50  // TOUCANA buffer 1
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B2    51  // TOUCANA buffer 2
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B3    52  // TOUCANA buffer 3
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B4    53  // TOUCANA buffer 4
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B5    54  // TOUCANA buffer 5
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B6    55  // TOUCANA buffer 6
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B7    56  // TOUCANA buffer 7
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B8    57  // TOUCANA buffer 8
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B9    58  // TOUCANA buffer 9
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B10   59  // TOUCANA buffer 10
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B11   60  // TOUCANA buffer 11
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B12   61  // TOUCANA buffer 12
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B13   62  // TOUCANA buffer 13
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B14   63  // TOUCANA buffer 14
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B15   64  // TOUCANA buffer 15
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_BOFF  65  // TOUCANB buss off
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_ERR   66  // TOUCANB error
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_WU    67  // TOUCANB wake up
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0    68  // TOUCANB buffer 0
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B1    69  // TOUCANB buffer 1
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B2    70  // TOUCANB buffer 2
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B3    71  // TOUCANB buffer 3
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B4    72  // TOUCANB buffer 4
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B5    73  // TOUCANB buffer 5
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B6    74  // TOUCANB buffer 6
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B7    75  // TOUCANB buffer 7
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B8    76  // TOUCANB buffer 8
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B9    77  // TOUCANB buffer 9
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B10   78  // TOUCANB buffer 10
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B11   79  // TOUCANB buffer 11
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B12   80  // TOUCANB buffer 12
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B13   81  // TOUCANB buffer 13
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B14   82  // TOUCANB buffer 14
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B15   83  // TOUCANB buffer 15
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0      84  // TPU A channel 0
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH1      85  // TPU A channel 1
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH2      86  // TPU A channel 2
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH3      87  // TPU A channel 3
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH4      88  // TPU A channel 4
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH5      89  // TPU A channel 5
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH6      90  // TPU A channel 6
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH7      91  // TPU A channel 7
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH8      92  // TPU A channel 8
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH9      93  // TPU A channel 9
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH10     94  // TPU A channel 10
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH11     95  // TPU A channel 11
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH12     96  // TPU A channel 12
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH13     97  // TPU A channel 13
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH14     98  // TPU A channel 14
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH15     99  // TPU A channel 15
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0      100 // TPU B channel 0
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH1      101 // TPU B channel 1
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH2      102 // TPU B channel 2
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH3      103 // TPU B channel 3
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH4      104 // TPU B channel 4
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH5      105 // TPU B channel 5
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH6      106 // TPU B channel 6
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH7      107 // TPU B channel 7
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH8      108 // TPU B channel 8
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH9      109 // TPU B channel 9
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH10     110 // TPU B channel 10
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH11     111 // TPU B channel 11
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH12     112 // TPU B channel 12
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH13     113 // TPU B channel 13
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH14     114 // TPU B channel 14
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH15     115 // TPU B channel 15
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM0    116 // MIOS PWM0
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM1    117 // MIOS PWM1
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM2    118 // MIOS PWM2
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM3    119 // MIOS PWM3
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM6   120 // MIOS MCSM6
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM11  121 // MIOS MDASM11
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM12  122 // MIOS MDASM12
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM13  123 // MIOS MDASM13
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM14  124 // MIOS MDASM14
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM15  125 // MIOS MDASM15
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM16   126 // MIOS PWM16
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM17   127 // MIOS PWM17
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM18   128 // MIOS PWM18
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM19   129 // MIOS PWM19
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM22  130 // MIOS MCSM22
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM27  131 // MIOS MDASM27
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM28  132 // MIOS MDASM28
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM29  133 // MIOS MDASM29
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM30  134 // MIOS MDASM30
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM31  135 // MIOS MDASM31

#define CYGNUM_HAL_ISR_MIN      CYGNUM_HAL_INTERRUPT_DECREMENTER
#define CYGNUM_HAL_ISR_MAX      CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM31

#define CYGARC_SIU_PRIORITY_HIGH                 7 // Maximum interrupt priority on SIU 
#define CYGARC_SIU_PRIORITY_LOW                  0 // Minimum interrupt prioeirt on SIU
#define CYGARC_IMB3_PRIORITY_HIGH               31 // Maximum interrupt priority on IMB3
#define CYGARC_IMB3_PRIORITY_LOW                 0 // Minimum interrupt priority on IMB3


//--------------------------------------------------------------------------
// Interrupt controller access
static __inline__ void
cyg_hal_interrupt_mask ( cyg_uint32 vector )
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_SIU_IRQ0 ... CYGNUM_HAL_INTERRUPT_SIU_LVL7:
    {
        // SIU interrupt vectors
        cyg_uint32 simask;

        HAL_READ_UINT32 (CYGARC_REG_IMM_SIMASK, simask);
        simask &= ~(((cyg_uint32) CYGARC_REG_IMM_SIMASK_IRM0) 
                    >> (vector - CYGNUM_HAL_INTERRUPT_SIU_IRQ0));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_SIMASK, simask);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_A :
    {
        // TimeBase A interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFAE);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFA); // Prevent from clearing interrupt flags
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFB); // accidently. Just do what is asked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_B :
    {
        // TimeBase B interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFBE);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFA); // Prevent from clearing interrupt flags
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFB); // accidently. Just do what is asked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_PIT:
    {
        // Periodic Interrupt
        cyg_uint16 piscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        piscr &= ~(CYGARC_REG_IMM_PISCR_PIE);
        piscr &= ~(CYGARC_REG_IMM_PISCR_PS); // Prevent from clearing interrupt flag.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC:
    {
        // Real Time Clock Second
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SIE);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SEC); // Prevent from clearing interrupt flags
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALR); // Accidently. Just do what is asked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    {
        // Real Time Clock Alarm
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALE);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SEC); // Prevent from clearing interrupt flags
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALR); // accidently. Just do what is asked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_SIU_COL:
    {
        // PLL change of lock
        cyg_uint16 colir;

        HAL_READ_UINT16 (CYGARC_REG_IMM_COLIR, colir);
        colir &= ~(CYGARC_REG_IMM_COLIR_COLIE);
        colir &= ~(CYGARC_REG_IMM_COLIR_COLIS); // Prevent from clearing interrupt flag accidently.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_COLIR, colir);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        quacr1 &= ~(CYGARC_REG_IMM_QUACR1_CIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        quacr1 &= ~(CYGARC_REG_IMM_QUACR1_PIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI2:
    {
        cyg_uint16 quacr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        quacr2 &= ~(CYGARC_REG_IMM_QUACR2_CIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI2:
    {
        cyg_uint16 quacr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        quacr2 &= ~(CYGARC_REG_IMM_QUACR2_PIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        quacr1 &= ~(CYGARC_REG_IMM_QUACR1_CIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        quacr1 &= ~(CYGARC_REG_IMM_QUACR1_PIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI2:
    {
        cyg_uint16 quacr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        quacr2 &= ~(CYGARC_REG_IMM_QUACR2_CIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI2:
    {
        cyg_uint16 quacr2;
 
        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        quacr2 &= ~(CYGARC_REG_IMM_QUACR2_PIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_TIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TXC:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_TCIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_RIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_IDLE:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_ILIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_TIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXC:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_TCIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }
 
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_RIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_IDLE:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 &= ~(CYGARC_REG_IMM_SCCxR1_ILIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQTHF:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr &= ~(CYGARC_REG_IMM_QSCI1CR_QTHFI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQBHF:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr &= ~(CYGARC_REG_IMM_QSCI1CR_QBHFI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQTHE:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr &= ~(CYGARC_REG_IMM_QSCI1CR_QTHEI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQBHE:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr &= ~(CYGARC_REG_IMM_QSCI1CR_QBHEI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_FI:
    {
        cyg_uint16 spcr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_SPCR2, spcr2);
        spcr2 &= ~(CYGARC_REG_IMM_SPCR2_SPIFIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SPCR2, spcr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_MODF:
    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_HALTA:
    {
        cyg_uint16 spcr3;

        HAL_READ_UINT16(CYGARC_REG_IMM_SPCR3, spcr3);
        spcr3 &= ~(CYGARC_REG_IMM_SPCR3_HMIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SPCR3, spcr3);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_BOFF:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        canctrl0 &= ~(CYGARC_REG_IMM_CANCTRL0_BOFFMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        break;
    }
 
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_ERR:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        canctrl0 &= ~(CYGARC_REG_IMM_CANCTRL0_ERRMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_WU:
    {
        cyg_uint16 tcnmcr;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_TCNMCR_A, tcnmcr);
        tcnmcr &= ~(CYGARC_REG_IMM_TCNMCR_WAKEMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_TCNMCR_A, tcnmcr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B15:
    {
        cyg_uint16 imask;

        HAL_READ_UINT16(CYGARC_REG_IMM_IMASK_A, imask);
        imask &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_IMASK_A, imask);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_BOFF:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        canctrl0 &= ~(CYGARC_REG_IMM_CANCTRL0_BOFFMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_ERR:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        canctrl0 &= ~(CYGARC_REG_IMM_CANCTRL0_ERRMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_WU:
    {
        cyg_uint16 tcnmcr;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_TCNMCR_B, tcnmcr);
        tcnmcr &= ~(CYGARC_REG_IMM_TCNMCR_WAKEMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_TCNMCR_B, tcnmcr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B15:
    {
        cyg_uint16 imask;

        HAL_READ_UINT16(CYGARC_REG_IMM_IMASK_B, imask);
        imask &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_IMASK_B, imask);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH15:
    {
        cyg_uint16 cier;
     
        HAL_READ_UINT16(CYGARC_REG_IMM_CIER_A, cier);
        cier &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CIER_A, cier);
        break;
    }
    
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH15:
    {
        cyg_uint16 cier;
     
        HAL_READ_UINT16(CYGARC_REG_IMM_CIER_B, cier);
        cier &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CIER_B, cier);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM0:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN0);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM1:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM2:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM3:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN3);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM6:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN6);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM11:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN11);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM12:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN12);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM13:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN13);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM14:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN14);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM15:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 &= ~(CYGARC_REG_IMM_MIOS1ER0_EN15);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM16:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN16);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM17:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN17);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM18:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN18);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM19:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN19);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM22:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN22);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM27:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN27);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM28:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN28);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM29:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN29);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM30:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN30);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM31:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 &= ~(CYGARC_REG_IMM_MIOS1ER1_EN31);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
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
        simask |= (((cyg_uint32) CYGARC_REG_IMM_SIMASK_IRM0) 
                    >> (vector - CYGNUM_HAL_INTERRUPT_SIU_IRQ0));
        HAL_WRITE_UINT32 (CYGARC_REG_IMM_SIMASK, simask);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_A :
    {
        // TimeBase A interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= (CYGARC_REG_IMM_TBSCR_REFAE);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFA); // Prevent from clearing interrupt flags
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFB); // accidently. Just do what is asked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_TB_B :
    {
        // TimeBase B interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= (CYGARC_REG_IMM_TBSCR_REFBE);
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFA); // Prevent from clearing interrupt flags
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFB); // accidently. Just do what is asked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_PIT:
    {
        // Periodic Interrupt
        cyg_uint16 piscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        piscr |= (CYGARC_REG_IMM_PISCR_PIE);
        piscr &= ~(CYGARC_REG_IMM_PISCR_PS); // Prevent from clearing interrupt flag.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC:
    {
        // Real Time Clock Second
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc |= (CYGARC_REG_IMM_RTCSC_SIE);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SEC); // Prevent from clearing interrupt flags
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALR); // accidently. Just do what is asdked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    {
        // Real Time Clock Alarm
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc |= (CYGARC_REG_IMM_RTCSC_ALE);
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SEC); // Prevent from clearing interrupt flags
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALR); // accidently. Just do what is asdked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_SIU_COL:
    {
        // PLL change of lock
        cyg_uint16 colir;

        HAL_READ_UINT16 (CYGARC_REG_IMM_COLIR, colir);
        colir |= (CYGARC_REG_IMM_COLIR_COLIE);
        colir &= ~(CYGARC_REG_IMM_COLIR_COLIS); // Prevent from clearing interrupt flag accidently.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_COLIR, colir);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        quacr1 |= (CYGARC_REG_IMM_QUACR1_CIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        quacr1 |= (CYGARC_REG_IMM_QUACR1_PIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_A, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI2:
    {
        cyg_uint16 quacr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        quacr2 |= (CYGARC_REG_IMM_QUACR2_CIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI2:
    {
        cyg_uint16 quacr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        quacr2 |= (CYGARC_REG_IMM_QUACR2_PIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_A, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        quacr1 |= (CYGARC_REG_IMM_QUACR1_CIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI1:
    {
        cyg_uint16 quacr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        quacr1 |= (CYGARC_REG_IMM_QUACR1_PIE1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR1_B, quacr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI2:
    {
        cyg_uint16 quacr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        quacr2 |= (CYGARC_REG_IMM_QUACR2_CIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI2:
    {
        cyg_uint16 quacr2;
 
        HAL_READ_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        quacr2 |= (CYGARC_REG_IMM_QUACR2_PIE2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUACR2_B, quacr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_TIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TXC:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_TCIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_RIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_IDLE:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_ILIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC1R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_TIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXC:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_TCIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }
 
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_RIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_IDLE:
    {
        cyg_uint16 sccxr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        sccxr1 |= (CYGARC_REG_IMM_SCCxR1_ILIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SCC2R1, sccxr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQTHF:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr |= (CYGARC_REG_IMM_QSCI1CR_QTHFI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQBHF:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr |= (CYGARC_REG_IMM_QSCI1CR_QBHFI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQTHE:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr |= (CYGARC_REG_IMM_QSCI1CR_QTHEI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQBHE:
    {
        cyg_uint16 qsci1cr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        qsci1cr |= (CYGARC_REG_IMM_QSCI1CR_QBHEI);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1CR, qsci1cr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_FI:
    {
        cyg_uint16 spcr2;

        HAL_READ_UINT16(CYGARC_REG_IMM_SPCR2, spcr2);
        spcr2 |= (CYGARC_REG_IMM_SPCR2_SPIFIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SPCR2, spcr2);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_MODF:
    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_HALTA:
    {
        cyg_uint16 spcr3;

        HAL_READ_UINT16(CYGARC_REG_IMM_SPCR3, spcr3);
        spcr3 |= (CYGARC_REG_IMM_SPCR3_HMIE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SPCR3, spcr3);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_BOFF:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        canctrl0 |= (CYGARC_REG_IMM_CANCTRL0_BOFFMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        break;
    }
 
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_ERR:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        canctrl0 |= (CYGARC_REG_IMM_CANCTRL0_ERRMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A, canctrl0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_WU:
    {
        cyg_uint16 tcnmcr;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_TCNMCR_A, tcnmcr);
        tcnmcr |= (CYGARC_REG_IMM_TCNMCR_WAKEMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_TCNMCR_A, tcnmcr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B15:
    {
        cyg_uint16 imask;

        HAL_READ_UINT16(CYGARC_REG_IMM_IMASK_A, imask);
        imask |= (((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_IMASK_A, imask);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_BOFF:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        canctrl0 |= (CYGARC_REG_IMM_CANCTRL0_BOFFMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_ERR:
    {
        cyg_uint16 canctrl0;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        canctrl0 |= (CYGARC_REG_IMM_CANCTRL0_ERRMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B, canctrl0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_WU:
    {
        cyg_uint16 tcnmcr;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_TCNMCR_B, tcnmcr);
        tcnmcr |= (CYGARC_REG_IMM_TCNMCR_WAKEMSK);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_TCNMCR_B, tcnmcr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B15:
    {
        cyg_uint16 imask;

        HAL_READ_UINT16(CYGARC_REG_IMM_IMASK_B, imask);
        imask |= (((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_IMASK_B, imask);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH15:
    {
        cyg_uint16 cier;
     
        HAL_READ_UINT16(CYGARC_REG_IMM_CIER_A, cier);
        cier |= (((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CIER_A, cier);
        break;
    }
    
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH15:
    {
        cyg_uint16 cier;
     
        HAL_READ_UINT16(CYGARC_REG_IMM_CIER_B, cier);
        cier |= (((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CIER_B, cier);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM0:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN0);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM1:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM2:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM3:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN3);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM6:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN6);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM11:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN11);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM12:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN12);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM13:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN13);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM14:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN14);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM15:
    {
        cyg_uint16 mios1er0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        mios1er0 |= (CYGARC_REG_IMM_MIOS1ER0_EN15);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER0, mios1er0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM16:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN16);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM17:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN17);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM18:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN18);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM19:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN19);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM22:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN22);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM27:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN27);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM28:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN28);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM29:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN29);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM30:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN30);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM31:
    {
        cyg_uint16 mios1er1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
        mios1er1 |= (CYGARC_REG_IMM_MIOS1ER1_EN31);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1ER1, mios1er1);
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
        HAL_READ_UINT32 (CYGARC_REG_IMM_SIPEND, sipend);
        sipend |= (((cyg_uint32) CYGARC_REG_IMM_SIPEND_IRQ0) 
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
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFB); // Only acknowledge the requested one
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_SIU_TB_B:
    {
        // TimeBase B interrupt
        cyg_uint16 tbscr;

        HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
        tbscr |= CYGARC_REG_IMM_TBSCR_REFB;
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFA); // Only acknowledge the requested one.
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
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALR); // Only acknowledge the requested one
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR:
    {
        // Real Time Clock Alarm
        cyg_uint16 rtcsc;

        HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        rtcsc |= CYGARC_REG_IMM_RTCSC_ALR;
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SEC); // Only acknowledge the requested one
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_SIU_COL:
    {
        cyg_uint16 colir;

        HAL_READ_UINT16(CYGARC_REG_IMM_COLIR, colir);
        colir |= CYGARC_REG_IMM_COLIR_COLIS;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_COLIR, colir);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI1:
    {
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_CF1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI1:
    { 
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_PF1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI2:
    {
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_CF2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI2:
    {
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_PF2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_A, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI1:
    {
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_CF1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI1:
    {
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_PF1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI2:
    {
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_CF2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI2:
    {
        cyg_uint16 quasr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        quasr0 &= ~(CYGARC_REG_IMM_QUASR0_PF2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUASR0_B, quasr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TXC:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX:
        // Nothing needs to be done here
        break;

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_IDLE:
    {
        cyg_uint16 scxsr;
   
        HAL_READ_UINT16(CYGARC_REG_IMM_SC1SR, scxsr);
        scxsr &= ~(CYGARC_REG_IMM_SCxSR_IDLE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SC1SR, scxsr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXC:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX:
        // nothing needs to be done here
        break;

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_IDLE:
    {
        cyg_uint16 scxsr;
   
        HAL_READ_UINT16(CYGARC_REG_IMM_SC2SR, scxsr);
        scxsr &= ~(CYGARC_REG_IMM_SCxSR_IDLE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SC2SR, scxsr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQTHF:
    {
        cyg_uint16 qsci1sr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        qsci1sr &= ~(CYGARC_REG_IMM_QSCI1SR_QTHF);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQBHF:
    {
        cyg_uint16 qsci1sr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        qsci1sr &= ~(CYGARC_REG_IMM_QSCI1SR_QBHF);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQTHE:
    {
        cyg_uint16 qsci1sr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        qsci1sr &= ~(CYGARC_REG_IMM_QSCI1SR_QTHE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQBHE:
    {
        cyg_uint16 qsci1sr;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        qsci1sr &= ~(CYGARC_REG_IMM_QSCI1SR_QBHE);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSCI1SR, qsci1sr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_FI:
    {
        cyg_uint16 spsr;
  
        HAL_READ_UINT16(CYGARC_REG_IMM_SPSR, spsr);
        spsr &= ~(CYGARC_REG_IMM_SPSR_SPIF);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SPSR, spsr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_MODF:
    {
        cyg_uint16 spsr;
  
        HAL_READ_UINT16(CYGARC_REG_IMM_SPSR, spsr);
        spsr &= ~(CYGARC_REG_IMM_SPSR_MODF);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SPSR, spsr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_HALTA:
    {
        cyg_uint16 spsr;
  
        HAL_READ_UINT16(CYGARC_REG_IMM_SPSR, spsr);
        spsr &= ~(CYGARC_REG_IMM_SPSR_HALTA);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_SPSR, spsr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_BOFF:
    {
        cyg_uint16 estat;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_ESTAT_A, estat);     // Read the flag as a one
        estat &= ~(CYGARC_REG_IMM_ESTAT_BOFFINT);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_ESTAT_A, estat);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_ERR:
    {
        cyg_uint16 estat;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_ESTAT_A, estat);     // Read the flag as a one
        estat &= ~(CYGARC_REG_IMM_ESTAT_ERRINT);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_ESTAT_A, estat);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_WU:
    {
        cyg_uint16 estat;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_ESTAT_A, estat);     // Read tthe flag as a one
        estat &= ~(CYGARC_REG_IMM_ESTAT_WAKEINT);           
        HAL_WRITE_UINT16(CYGARC_REG_IMM_ESTAT_A, estat);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B15:
    {
        cyg_uint16 iflag;

        HAL_READ_UINT16(CYGARC_REG_IMM_IFLAG_A, iflag);     // Read the flag as a one
        iflag &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_IFLAG_A, iflag);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_BOFF:
    {
        cyg_uint16 estat;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_ESTAT_B, estat);     // Read the flag as a one
        estat &= ~(CYGARC_REG_IMM_ESTAT_BOFFINT);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_ESTAT_B, estat);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_ERR:
    {
        cyg_uint16 estat;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_ESTAT_B, estat);     // Read the flag as a one
        estat &= ~(CYGARC_REG_IMM_ESTAT_ERRINT);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_ESTAT_B, estat);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_WU:
    {
        cyg_uint16 estat;
        
        HAL_READ_UINT16(CYGARC_REG_IMM_ESTAT_B, estat);     // Read the flag as a one
        estat &= ~(CYGARC_REG_IMM_ESTAT_WAKEINT);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_ESTAT_B, estat);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B15:
    {
        cyg_uint16 iflag;

        HAL_READ_UINT16(CYGARC_REG_IMM_IFLAG_B, iflag);     // Read the flag as a one
        iflag &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_IFLAG_B, iflag);    // And write it as a zero
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH15:
    {
        cyg_uint16 cisr;
     
        HAL_READ_UINT16(CYGARC_REG_IMM_CISR_A, cisr);
        cisr &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CISR_A, cisr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH15:
    {
        cyg_uint16 cisr;
     
        HAL_READ_UINT16(CYGARC_REG_IMM_CISR_B, cisr);
        cisr &= ~(((cyg_uint16)0x0001) << (vector - CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0));
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CISR_B, cisr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM0:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL0);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM1:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL1);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM2:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL2);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM3:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL3);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM6:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL6);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM11:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL11);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM12:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL12);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM13:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL13);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM14:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL14);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM15:
    {
        cyg_uint16 mios1sr0;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        mios1sr0 &= ~(CYGARC_REG_IMM_MIOS1SR0_FL15);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR0, mios1sr0);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM16:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL16);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM17:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL17);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM18:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL18);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM19:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL19);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM22:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL22);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM27:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL27);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM28:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL28);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM29:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL29);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM30:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL30);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM31:
    {
        cyg_uint16 mios1sr1;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
        mios1sr1 &= ~(CYGARC_REG_IMM_MIOS1SR1_FL31);
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1SR1, mios1sr1);
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
    // Only external interrupts can be fully configured in the true meaning of the word
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

        bit = (((cyg_uint32) CYGARC_REG_IMM_SIEL_ED0) 
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

    // Attempts to configure all other interrupt sources should fail
    default:
        CYG_FAIL("Unknown Interrupt!!!");
        break;
    }
}

static __inline__ void
cyg_hal_interrupt_set_level ( cyg_uint32 vector, cyg_uint32 level )
{
    if(vector < CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI1)
    {
        // Note: highest priority has the lowest numerical value.
        CYG_ASSERT( level >= CYGARC_SIU_PRIORITY_HIGH, "Invalid priority");
        CYG_ASSERT( level <= CYGARC_SIU_PRIORITY_LOW, "Invalid priority");
    }
    else
    {
        CYG_ASSERT( level >= CYGARC_IMB3_PRIORITY_HIGH, "Invalid priority");
        CYG_ASSERT( level <= CYGARC_IMB3_PRIORITY_LOW, "Invalid priority");
    }

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
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFA); // Prevent fom clearing interrupt flags
        tbscr &= ~(CYGARC_REG_IMM_TBSCR_REFB); // accidently. Just do what is asked.
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
        piscr &= ~(CYGARC_REG_IMM_PISCR_PS); // Prevent from clearing interrupt flag.
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
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_SEC); // Prevent from clearing interrupt flags
        rtcsc &= ~(CYGARC_REG_IMM_RTCSC_ALR); // accidently. Just do wahat is asked.
        HAL_WRITE_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_SIU_COL:
    {
        cyg_uint16 colir;
       
        HAL_READ_UINT16(CYGARC_REG_IMM_COLIR, colir);
        colir &= ~(CYGARC_REG_IMM_COLIR_COLIRQ);      // mask out the level
        colir |= CYGARC_REG_IMM_COLIR_IRQ0 >> level;  // and set the new level
        colir &= ~(CYGARC_REG_IMM_COLIR_COLIS); // Prevent from clearing interrupt flag accidently.
        HAL_WRITE_UINT16(CYGARC_REG_IMM_COLIR, colir);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI1:
    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI1:
    {
        cyg_uint16 quadc64int;
        cyg_uint16 the_level = level << CYGARC_REG_IMM_QUADC64INT_IRL1_SHIFT;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUADC64INT_A, quadc64int);
        quadc64int &= ~(CYGARC_REG_IMM_QUADC64INT_IRL1);
        quadc64int |= the_level;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUADC64INT_A, quadc64int);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI2:
    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI2:
    {
        cyg_uint16 quadc64int;
        cyg_uint16 the_level = level << CYGARC_REG_IMM_QUADC64INT_IRL2_SHIFT;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUADC64INT_A, quadc64int);
        quadc64int &= ~(CYGARC_REG_IMM_QUADC64INT_IRL2);
        quadc64int |= the_level;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUADC64INT_A, quadc64int);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI1:
    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI1:
    {
        cyg_uint16 quadc64int;
        cyg_uint16 the_level = level << CYGARC_REG_IMM_QUADC64INT_IRL1_SHIFT;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUADC64INT_B, quadc64int);
        quadc64int &= ~(CYGARC_REG_IMM_QUADC64INT_IRL1);
        quadc64int |= the_level;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUADC64INT_B, quadc64int);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI2:
    case CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI2:
    {
        cyg_uint16 quadc64int;
        cyg_uint16 the_level = level << CYGARC_REG_IMM_QUADC64INT_IRL2_SHIFT;

        HAL_READ_UINT16(CYGARC_REG_IMM_QUADC64INT_B, quadc64int);
        quadc64int &= ~(CYGARC_REG_IMM_QUADC64INT_IRL2);
        quadc64int |= the_level;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QUADC64INT_B, quadc64int);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TXC:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI0_IDLE:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXC:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_IDLE:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQTHF:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQBHF:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQTHE:
    case CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQBHE:
    {
        cyg_uint16 qdsci_il;
        cyg_uint16 the_level = level << CYGARC_REG_IMM_QDSCI_IL_ILDSCI_SHIFT;

        HAL_READ_UINT16(CYGARC_REG_IMM_QDSCI_IL, qdsci_il);
        qdsci_il &= ~(CYGARC_REG_IMM_QDSCI_IL_ILDSCI);
        qdsci_il |= the_level;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QDSCI_IL, qdsci_il);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_FI:
    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_MODF:
    case CYGNUM_HAL_INTERRUPT_IMB3_SPI_HALTA:
    {
        cyg_uint16 qspi_il;
        cyg_uint16 the_level = level << CYGARC_REG_IMM_QSPI_IL_ILQSPI_SHIFT;

        HAL_READ_UINT16(CYGARC_REG_IMM_QSPI_IL, qspi_il);
        qspi_il &= ~(CYGARC_REG_IMM_QSPI_IL_ILQSPI);
        qspi_il |= the_level;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_QSPI_IL, qspi_il);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_BOFF:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_ERR:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_WU:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B15:
    {
        cyg_uint16 irl  = (level % 8) << CYGARC_REG_IMM_CANICR_IRL_SHIFT;
        cyg_uint16 ilsb = (level / 8) << CYGARC_REG_IMM_CANICR_ILBS_SHIFT;
        cyg_uint16 canicr;

        HAL_READ_UINT16(CYGARC_REG_IMM_CANICR_A, canicr);
        canicr &= ~(CYGARC_REG_IMM_CANICR_IRL);
        canicr &= ~(CYGARC_REG_IMM_CANICR_ILBS);
        canicr |= irl;
        canicr |= ilsb;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANICR_A, canicr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_BOFF:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_ERR:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_WU:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B15:
    {
        cyg_uint16 irl  = (level % 8) << CYGARC_REG_IMM_CANICR_IRL_SHIFT;
        cyg_uint16 ilsb = (level / 8) << CYGARC_REG_IMM_CANICR_ILBS_SHIFT;
        cyg_uint16 canicr;

        HAL_READ_UINT16(CYGARC_REG_IMM_CANICR_B, canicr);
        canicr &= ~(CYGARC_REG_IMM_CANICR_IRL);
        canicr &= ~(CYGARC_REG_IMM_CANICR_ILBS);
        canicr |= irl;
        canicr |= ilsb;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_CANICR_B, canicr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH15:
    {
        cyg_uint16 cirl = (level % 8) << CYGARC_REG_IMM_TICR_CIRL_SHIFT;
        cyg_uint16 ilsb = (level / 8) << CYGARC_REG_IMM_TICR_ILBS_SHIFT;
        cyg_uint16 ticr;

        HAL_READ_UINT16(CYGARC_REG_IMM_TICR_A, ticr);
        ticr &= ~(CYGARC_REG_IMM_TICR_CIRL);
        ticr &= ~(CYGARC_REG_IMM_TICR_ILBS);
        ticr |= cirl;
        ticr |= ilsb;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_TICR_A, ticr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH1:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH2:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH3:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH4:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH5:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH6:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH7:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH8:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH9:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH10:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH11:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH12:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH13:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH14:
    case CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH15:
    {
        cyg_uint16 cirl = (level % 8) << CYGARC_REG_IMM_TICR_CIRL_SHIFT;
        cyg_uint16 ilsb = (level / 8) << CYGARC_REG_IMM_TICR_ILBS_SHIFT;
        cyg_uint16 ticr;

        HAL_READ_UINT16(CYGARC_REG_IMM_TICR_B, ticr);
        ticr &= ~(CYGARC_REG_IMM_TICR_CIRL);
        ticr &= ~(CYGARC_REG_IMM_TICR_ILBS);
        ticr |= cirl;
        ticr |= ilsb;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_TICR_B, ticr);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM0:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM1:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM2:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM3:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM6:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM11:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM12:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM13:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM14:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM15:
    {
        cyg_uint16 lvl = (level % 8) << CYGARC_REG_IMM_MIOS1LVL_LVL_SHIFT;
        cyg_uint16 tm  = (level / 8) << CYGARC_REG_IMM_MIOS1LVL_TM_SHIFT;
        cyg_uint16 mios1lvl;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1LVL0, mios1lvl);
        mios1lvl &= ~(CYGARC_REG_IMM_MIOS1LVL_LVL);
        mios1lvl &= ~(CYGARC_REG_IMM_MIOS1LVL_TM);
        mios1lvl |= lvl;
        mios1lvl |= tm;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1LVL0, mios1lvl);
        break;
    }

    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM16:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM17:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM18:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM19:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM22:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM27:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM28:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM29:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM30:
    case CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM31:
    {
        cyg_uint16 lvl = (level % 8) << CYGARC_REG_IMM_MIOS1LVL_LVL_SHIFT;
        cyg_uint16 tm  = (level / 8) << CYGARC_REG_IMM_MIOS1LVL_TM_SHIFT;
        cyg_uint16 mios1lvl;

        HAL_READ_UINT16(CYGARC_REG_IMM_MIOS1LVL1, mios1lvl);
        mios1lvl &= ~(CYGARC_REG_IMM_MIOS1LVL_LVL);
        mios1lvl &= ~(CYGARC_REG_IMM_MIOS1LVL_TM);
        mios1lvl |= lvl;
        mios1lvl |= tm;
        HAL_WRITE_UINT16(CYGARC_REG_IMM_MIOS1LVL1, mios1lvl);
        break;
    }

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

//--------------------------------------------------------------------------
// Interrupt arbiters
externC cyg_uint32 hal_arbitration_isr_tb (CYG_ADDRWORD vector, 
                                           CYG_ADDRWORD data);
externC cyg_uint32 hal_arbitration_isr_pit (CYG_ADDRWORD vector,
                                            CYG_ADDRWORD data);
externC cyg_uint32 hal_arbitration_isr_rtc (CYG_ADDRWORD vector,
                                            CYG_ADDRWORD data);
externC cyg_uint32 hal_arbitration_isr_sci (CYG_ADDRWORD vector,
		                                    CYG_ADDRWORD data);

//-----------------------------------------------------------------------------
// Symbols used by assembly code
#define CYGARC_VARIANT_DEFS                                     \
    DEFINE(CYGNUM_HAL_VECTOR_NMI, CYGNUM_HAL_VECTOR_NMI);

//-----------------------------------------------------------------------------
// Interrupt source priorities
// Maybe this is not the best way to do this. Fact remains that these priorities
// are not hard wired by the board layout, but are all software configurable. Not
// so easy to combine flexibility with generality
#define CYGNUM_HAL_INTERRUPT_SIU_TB_A_PRIORITY          CYGNUM_HAL_ISR_SOURCE_PRIORITY_TB
#define CYGNUM_HAL_INTERRUPT_SIU_TB_B_PRIORITY          CYGNUM_HAL_ISR_SOURCE_PRIORITY_TB
#define CYGNUM_HAL_INTERRUPT_SIU_PIT_PRIORITY           CYGNUM_HAL_ISR_SOURCE_PRIORITY_PIT
#define CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC_PRIORITY       CYGNUM_HAL_ISR_SOURCE_PRIORITY_RTC
#define CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR_PRIORITY       CYGNUM_HAL_ISR_SOURCE_PRIORITY_RTC
#define CYGNUM_HAL_INTERRUPT_SIU_COL_PRIORITY           CYGNUM_HAL_ISR_SOURCE_PRIORITY_PLL
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI1_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_A_QUEUE1
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI1_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_A_QUEUE1
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_CI2_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_A_QUEUE2
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCA_PI2_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_A_QUEUE2
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI1_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_B_QUEUE1
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI1_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_B_QUEUE1
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_CI2_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_B_QUEUE2
#define CYGNUM_HAL_INTERRUPT_IMB3_QUADCB_PI2_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_QUADC_B_QUEUE2
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TX_PRIORITY      CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_TXC_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_RX_PRIORITY      CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI 
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI0_IDLE_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TX_PRIORITY      CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXC_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RX_PRIORITY      CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_IDLE_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI 
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQTHF_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_RXQBHF_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI  
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQTHE_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI 
#define CYGNUM_HAL_INTERRUPT_IMB3_SCI1_TXQBHE_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSCI
#define CYGNUM_HAL_INTERRUPT_IMB3_SPI_FI_PRIORITY       CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSPI
#define CYGNUM_HAL_INTERRUPT_IMB3_SPI_MODF_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSPI
#define CYGNUM_HAL_INTERRUPT_IMB3_SPI_HALTA_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_QSPI
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_BOFF_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_ERR_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_WU_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B0_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B1_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B2_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B3_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B4_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B5_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B6_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B7_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B8_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B9_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B10_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B11_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B12_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B13_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B14_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANA_B15_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_BOFF_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_ERR_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_WU_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B0_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B1_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B2_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B3_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B4_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B5_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B6_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B7_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B8_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B9_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B10_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B11_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B12_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B13_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B14_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TOUCANB_B15_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_TOUCAN_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH0_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH1_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH2_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH3_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH4_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH5_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH6_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH7_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH8_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH9_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH10_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH11_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH12_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH13_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH14_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUA_CH15_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_A
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH0_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH1_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH2_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH3_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH4_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH5_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH6_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH7_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH8_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH9_PRIORITY     CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH10_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH11_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH12_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH13_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH14_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_TPUB_CH15_PRIORITY    CYGNUM_HAL_ISR_SOURCE_PRIORITY_TPU_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM0_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM1_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM2_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM3_PRIORITY   CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM6_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM11_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM12_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM13_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM14_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM15_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_A
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM16_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM17_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM18_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MPWM19_PRIORITY  CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MMCSM22_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM27_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM28_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM29_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM30_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B
#define CYGNUM_HAL_INTERRUPT_IMB3_MIOS_MDASM31_PRIORITY CYGNUM_HAL_ISR_SOURCE_PRIORITY_MIOS_B

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_INTR_H
// End of var_intr.h
