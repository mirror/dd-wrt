//==========================================================================
//
//      fr-v.h
//
//      HAL misc board support definitions for Fujitsu FR-V chips
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Date:         2001-09-07
// Purpose:      Platform register definitions
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#ifndef __HAL_FRV_H__
#define __HAL_FRV_H__ 1

// Common

#if 0
// Processor status register
#define _PSR_PIVL_SHIFT 3
#define _PSR_PIVL_MASK  (0xF<<(_PSR_PIVL_SHIFT))  // Interrupt mask level
#define _PSR_S          (1<<2)                    // Supervisor state
#define _PSR_PS         (1<<1)                    // Previous supervisor state
#define _PSR_ET         (1<<0)                    // Enable interrupts
#define _PSR_CM		(1<<12)			  // Enable conditionals

// Hardware status register
#define _HSR0_ICE       (1<<31)                   // Instruction cache enable
#define _HSR0_DCE       (1<<30)                   // Data cache enable
#define _HSR0_IMMU      (1<<26)                   // Instruction MMU enable
#define _HSR0_DMMU      (1<<25)                   // Data MMU enable
#endif
// Debug Control Register
#define _DCR_EBE        (1 << 30) // Exception break enable bit 
#define _DCR_SE         (1 << 29) // Single-step break enable bit 
#define _DCR_IBM        (1 << 28) // Instruction Break Mask (disable bit)
#define _DCR_DRBE0      (1 << 19) // READ dbar0
#define _DCR_DWBE0      (1 << 18) // WRITE dbar0
#define _DCR_DDBE0      (1 << 17) // Data-match for access to dbar0
#define _DCR_DBASE0     (1 << 17) // offset
#define _DCR_DRBE1      (1 << 16)
#define _DCR_DWBE1      (1 << 15)
#define _DCR_DDBE1      (1 << 14)
#define _DCR_DBASE1     (1 << 14)
//#define _DCR_DRBE2      (1 << 13) // 2 and 3 not supported in real hardware
//#define _DCR_DWBE2      (1 << 12)
//#define _DCR_DDBE2      (1 << 11)
//#define _DCR_DRBE3      (1 << 10)
//#define _DCR_DWBE3      (1 << 9)
//#define _DCR_DDBE3      (1 << 8)
#define _DCR_IBE0       (1 << 7)
#define _DCR_IBCE0      (1 << 6)
#define _DCR_IBE1       (1 << 5)
#define _DCR_IBCE1      (1 << 4)
#define _DCR_IBE2       (1 << 3)
#define _DCR_IBCE2      (1 << 2)
#define _DCR_IBE3       (1 << 1)
#define _DCR_IBCE3      (1 << 0)

// Break PSR Save Register
#define _BPSR_BS	(1 << 12)
#define _BPSR_BET	(1 << 0)

// Break Request Register
#define _BRR_EB         (1 << 30)
#define _BRR_CB         (1 << 29)
#define _BRR_TB         (1 << 28)
#define _BRR_DB0        (1 << 11)
#define _BRR_DB1        (1 << 10)
#define _BRR_IB0        (1 << 7)
#define _BRR_IB1        (1 << 6)
#define _BRR_IB2        (1 << 5)
#define _BRR_IB3        (1 << 4)
#define _BRR_CBB        (1 << 3)
#define _BRR_BB         (1 << 2)
#define _BRR_SB         (1 << 1)
#define _BRR_ST         (1 << 0)

// Programmable timers
#define _FRVGEN_TCSR0   0xFEFF9400                // Timer 0 control/status
#define _FRVGEN_TCSR1   0xFEFF9408                // Timer 1 control/status
#define _FRVGEN_TCSR2   0xFEFF9410                // Timer 2 control/status
#define _FRVGEN_TCxSR_TOUT 0x80                     // Status - TOUT signal 
#define _FRVGEN_TCTR    0xFEFF9418                // Timer control
#define _FRVGEN_TCTR_SEL0 (0<<6)                    // Select timer 0
#define _FRVGEN_TCTR_SEL1 (1<<6)                    // Select timer 1
#define _FRVGEN_TCTR_SEL2 (2<<6)                    // Select timer 2
#define _FRVGEN_TCTR_RB   (3<<6)                    // Timer read back
#define _FRVGEN_TCTR_RB_NCOUNT   (1<<5)                    // Count data suppress
#define _FRVGEN_TCTR_RB_NSTATUS  (1<<4)                    // Status data suppress
#define _FRVGEN_TCTR_RB_CTR2     (1<<3)                    // Read data for counter #2
#define _FRVGEN_TCTR_RB_CTR1     (1<<2)                    // Read data for counter #1
#define _FRVGEN_TCTR_RB_CTR0     (1<<1)                    // Read data for counter #0
#define _FRVGEN_TCTR_LATCH (0<<4)                   // Counter latch command
#define _FRVGEN_TCTR_R8LO  (1<<4)                   // Read low 8 bits
#define _FRVGEN_TCTR_R8HI  (2<<4)                   // Read high 8 bits
#define _FRVGEN_TCTR_RLOHI (3<<4)                   // Read/write 8 lo then 8 hi
#define _FRVGEN_TCTR_MODE0 (0<<1)                   // Mode 0 - terminal interrupt count
#define _FRVGEN_TCTR_MODE2 (2<<1)                   // Mode 2 - rate generator
#define _FRVGEN_TCTR_MODE4 (4<<1)                   // Mode 4 - software trigger strobe
#define _FRVGEN_TCTR_MODE5 (5<<1)                   // Mode 5 - hardware trigger strobe
#define _FRVGEN_TPRV    0xFEFF9420                // Timer prescale
#define _FRVGEN_TPRCKSL 0xFEFF9428                // Prescale clock
#define _FRVGEN_TCKSL0  0xFEFF9430                // Timer 0 clock select
#define _FRVGEN_TCKSL1  0xFEFF9438                // Timer 1 clock select
#define _FRVGEN_TCKSL2  0xFEFF9440                // Timer 2 clock select

// Interrupt & clock control
#define _FRVGEN_CLK_CTRL 0xFEFF9A00               // Clock control
#define _FRVGEN_CLK_CTRL_P0 (1<<8)                   // division rate of bus and resource clocks
#define _FRVGEN_IRC_TM0  0xFEFF9800               // Trigger mode 0 register (unused)
#define _FRVGEN_IRC_TM1  0xFEFF9808               // Trigger mode 1 register
#define _FRVGEN_IRC_RS   0xFEFF9810               // Request sense
#define _FRVGEN_IRC_RC   0xFEFF9818               // Request clear
#define _FRVGEN_IRC_MASK 0xFEFF9820               // Mask
#define _FRVGEN_IRC_IRL  0xFEFF9828               // Interrupt level read (encoded)
#define _FRVGEN_IRC_IRR0 0xFEFF9840               // Interrupt routing #0 (unused)
#define _FRVGEN_IRC_IRR1 0xFEFF9848               // Interrupt routing #1 (unused)
#define _FRVGEN_IRC_IRR2 0xFEFF9850               // Interrupt routing #2 (unused)
#define _FRVGEN_IRC_IRR3 0xFEFF9858               // Interrupt routing #3
#define _FRVGEN_IRC_IRR4 0xFEFF9860               // Interrupt routing #4
#define _FRVGEN_IRC_IRR5 0xFEFF9868               // Interrupt routing #5
#define _FRVGEN_IRC_IRR6 0xFEFF9870               // Interrupt routing #6
#define _FRVGEN_IRC_IRR7 0xFEFF9878               // Interrupt routing #7
#define _FRVGEN_IRC_ITM0 0xFEFF9880               // Internal trigger mode #0
#define _FRVGEN_IRC_ITM1 0xFEFF9888               // Internal trigger mode #1

// Serial ports - 16550 compatible
#define _FRVGEN_UART0   0xFEFF9C00
#define _FRVGEN_UART1   0xFEFF9C40

// Serial port prescaler
#define _FRVGEN_UCPSR   0xFEFF9C90
#define _FRVGEN_UCPVR   0xFEFF9C98

// Reset register
#define _FRVGEN_HW_RESET_STAT_P (1<<10)              // Last reset was power-on
#define _FRVGEN_HW_RESET_STAT_H (1<<9)               // Last reset was hard reset
#define _FRVGEN_HW_RESET_STAT_S (1<<8)               // Last reset was soft reset
#define _FRVGEN_HW_RESET_HR     (1<<1)               // Force hard reset
#define _FRVGEN_HW_RESET_SR     (1<<0)               // Force soft reset

#endif // __HAL_FRV_H__
