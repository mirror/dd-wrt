#ifndef CYGONCE_HAL_VAR_IO_H
#define CYGONCE_HAL_VAR_IO_H
//=============================================================================
//
//      var_io.h
//
//      Variant specific registers
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Author(s):   jskov
// Contributors:jskov, gthomas, tkoeller, tdrury, nickg
// Date:        2001-07-12
// Purpose:     AT91 variant specific registers
// Description: 
// Usage:       #include <cyg/hal/var_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/plf_io.h>

//=============================================================================
// USART

#ifndef AT91_USART0
#define AT91_USART0 0xFFFD0000
#endif

#ifndef AT91_USART1
#define AT91_USART1 0xFFFCC000
#endif

#define AT91_US_CR  0x00  // Control register
#define AT91_US_CR_RxRESET (1<<2)
#define AT91_US_CR_TxRESET (1<<3)
#define AT91_US_CR_RxENAB  (1<<4)
#define AT91_US_CR_RxDISAB (1<<5)
#define AT91_US_CR_TxENAB  (1<<6)
#define AT91_US_CR_TxDISAB (1<<7)
#define AT91_US_CR_RSTATUS (1<<8)
#define AT91_US_CR_STTTO   (1<<11)
#define AT91_US_MR  0x04  // Mode register
#define AT91_US_MR_CLOCK   4
#define AT91_US_MR_CLOCK_MCK  (0<<AT91_US_MR_CLOCK)
#define AT91_US_MR_CLOCK_MCK8 (1<<AT91_US_MR_CLOCK)
#define AT91_US_MR_CLOCK_SCK  (2<<AT91_US_MR_CLOCK)
#define AT91_US_MR_LENGTH  6
#define AT91_US_MR_LENGTH_5   (0<<AT91_US_MR_LENGTH)
#define AT91_US_MR_LENGTH_6   (1<<AT91_US_MR_LENGTH)
#define AT91_US_MR_LENGTH_7   (2<<AT91_US_MR_LENGTH)
#define AT91_US_MR_LENGTH_8   (3<<AT91_US_MR_LENGTH)
#define AT91_US_MR_SYNC    8
#define AT91_US_MR_SYNC_ASYNC (0<<AT91_US_MR_SYNC)
#define AT91_US_MR_SYNC_SYNC  (1<<AT91_US_MR_SYNC)
#define AT91_US_MR_PARITY  9
#define AT91_US_MR_PARITY_EVEN  (0<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_ODD   (1<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_SPACE (2<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_MARK  (3<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_NONE  (4<<AT91_US_MR_PARITY)
#define AT91_US_MR_PARITY_MULTI (6<<AT91_US_MR_PARITY)
#define AT91_US_MR_STOP   12
#define AT91_US_MR_STOP_1       (0<<AT91_US_MR_STOP)
#define AT91_US_MR_STOP_1_5     (1<<AT91_US_MR_STOP)
#define AT91_US_MR_STOP_2       (2<<AT91_US_MR_STOP)
#define AT91_US_MR_MODE   14
#define AT91_US_MR_MODE_NORMAL  (0<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE_ECHO    (1<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE_LOCAL   (2<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE_REMOTE  (3<<AT91_US_MR_MODE)
#define AT91_US_MR_MODE9  17
#define AT91_US_MR_CLKO   18
#define AT91_US_IER 0x08  // Interrupt enable register
#define AT91_US_IER_RxRDY   (1<<0)  // Receive data ready
#define AT91_US_IER_TxRDY   (1<<1)  // Transmitter ready
#define AT91_US_IER_RxBRK   (1<<2)  // Break received
#define AT91_US_IER_ENDRX   (1<<3)  // Rx end
#define AT91_US_IER_ENDTX   (1<<4)  // Tx end
#define AT91_US_IER_OVRE    (1<<5)  // Rx overflow
#define AT91_US_IER_FRAME   (1<<6)  // Rx framing error
#define AT91_US_IER_PARITY  (1<<7)  // Rx parity
#define AT91_US_IER_TIMEOUT (1<<8)  // Rx timeout
#define AT91_US_IER_TxEMPTY (1<<9)  // Tx empty
#define AT91_US_IDR 0x0C  // Interrupt disable register
#define AT91_US_IMR 0x10  // Interrupt mask register
#define AT91_US_CSR 0x14  // Channel status register
#define AT91_US_CSR_RxRDY 0x01 // Receive data ready
#define AT91_US_CSR_TxRDY 0x02 // Transmit ready
#define AT91_US_CSR_OVRE  0x20 // Overrun error
#define AT91_US_CSR_FRAME 0x40 // Framing error
#define AT91_US_RHR 0x18  // Receive holding register
#define AT91_US_THR 0x1C  // Transmit holding register
#define AT91_US_BRG 0x20  // Baud rate generator
#define AT91_US_RTO 0x24  // Receive time out
#define AT91_US_TTG 0x28  // Transmit timer guard

// PDC US registers may have different addresses in at91 targets (i.e jtst)
#ifndef AT91_US_RPR
#define AT91_US_RPR 0x30  // Receive pointer register
#endif

#ifndef AT91_US_RCR
#define AT91_US_RCR 0x34  // Receive counter register
#endif

#ifndef AT91_US_TPR
#define AT91_US_TPR 0x38  // Transmit pointer register
#endif

#ifndef AT91_US_TCR
#define AT91_US_TCR 0x3c  // Transmit counter register
#endif

// macro could be different from target to target (i.e jtst)
#ifndef AT91_US_BAUD
#define AT91_US_BAUD(baud) ((CYGNUM_HAL_ARM_AT91_CLOCK_SPEED/(8*(baud))+1)/2)
#endif
//=============================================================================
// PIO

#ifndef AT91_PIO
#define AT91_PIO      0xFFFF0000
#endif

#define AT91_PIO_PER  0x00  // PIO enable
#define AT91_PIO_PDR  0x04  // PIO disable
#define AT91_PIO_PSR  0x08  // PIO status

#if defined(CYGHWR_HAL_ARM_AT91_M55800A)

// PIOA
#define AT91_PIO_PSR_TCLK3   0x00000001 // Timer 3 Clock signal
#define AT91_PIO_PSR_TIOA3   0x00000002 // Timer 3 Signal A
#define AT91_PIO_PSR_TIOB3   0x00000004 // Timer 3 Signal B
#define AT91_PIO_PSR_TCLK4   0x00000008 // Timer 4 Clock signal
#define AT91_PIO_PSR_TIOA4   0x00000010 // Timer 4 Signal A
#define AT91_PIO_PSR_TIOB4   0x00000020 // Timer 4 Signal B
#define AT91_PIO_PSR_TCLK5   0x00000040 // Timer 5 Clock signal
#define AT91_PIO_PSR_TIOA5   0x00000080 // Timer 5 Signal A
#define AT91_PIO_PSR_TIOB5   0x00000100 // Timer 5 Signal B
#define AT91_PIO_PSR_IRQ0    0x00000200 // External Interrupt 0
#define AT91_PIO_PSR_IRQ1    0x00000400 // External Interrupt 1
#define AT91_PIO_PSR_IRQ2    0x00000800 // External Interrupt 2
#define AT91_PIO_PSR_IRQ3    0x00001000 // External Interrupt 3
#define AT91_PIO_PSR_FIQ     0x00002000 // Fast Interrupt
#define AT91_PIO_PSR_SCK0    0x00004000 // USART 0 Clock signal
#define AT91_PIO_PSR_TXD0    0x00008000 // USART 0 transmit data
#define AT91_PIO_PSR_RXD0    0x00010000 // USART 0 receive data
#define AT91_PIO_PSR_SCK1    0x00020000 // USART 1 Clock signal
#define AT91_PIO_PSR_TXD1    0x00040000 // USART 1 transmit data
#define AT91_PIO_PSR_RXD1    0x00080000 // USART 1 receive data
#define AT91_PIO_PSR_SCK2    0x00100000 // USART 2 Clock signal
#define AT91_PIO_PSR_TXD2    0x00200000 // USART 2 transmit data
#define AT91_PIO_PSR_RXD2    0x00400000 // USART 2 receive data
#define AT91_PIO_PSR_SPCK    0x00800000 // SPI Clock signal 
#define AT91_PIO_PSR_MISO    0x01000000 // SPI Master In Slave Out 
#define AT91_PIO_PSR_MOIS    0x02000000 // SPI Master Out Slave In 
#define AT91_PIO_PSR_NPCS0   0x04000000 // SPI Peripheral Chip Select 0
#define AT91_PIO_PSR_NPCS1   0x08000000 // SPI Peripheral Chip Select 1
#define AT91_PIO_PSR_NPCS2   0x10000000 // SPI Peripheral Chip Select 2
#define AT91_PIO_PSR_NPCS3   0x20000000 // SPI Peripheral Chip Select 3

// PIOB
#define AT91_PIO_PSR_IRQ4    0x00000008 // External Interrupt 4
#define AT91_PIO_PSR_IRQ5    0x00000010 // External Interrupt 5
#define AT91_PIO_PSR_AD0TRIG 0x00000040 // ADC0 External Trigger
#define AT91_PIO_PSR_AD1TRIG 0x00000080 // ADC1 External Trigger
#define AT91_PIO_PSR_BMS     0x00040000 // Boot Mode Select
#define AT91_PIO_PSR_TCLK0   0x00080000 // Timer 0 Clock signal
#define AT91_PIO_PSR_TIOA0   0x00100000 // Timer 0 Signal A
#define AT91_PIO_PSR_TIOB0   0x00200000 // Timer 0 Signal B
#define AT91_PIO_PSR_TCLK1   0x00400000 // Timer 1 Clock signal
#define AT91_PIO_PSR_TIOA1   0x00800000 // Timer 1 Signal A
#define AT91_PIO_PSR_TIOB1   0x01000000 // Timer 1 Signal B
#define AT91_PIO_PSR_TCLK2   0x02000000 // Timer 2 Clock signal
#define AT91_PIO_PSR_TIOA2   0x04000000 // Timer 2 Signal A
#define AT91_PIO_PSR_TIOB2   0x08000000 // Timer 2 Signal B

#else

#define AT91_PIO_PSR_TCLK0    0x00000001 // Timer #0 clock
#define AT91_PIO_PSR_TIOA0    0x00000002 // Timer #0 signal A
#define AT91_PIO_PSR_TIOB0    0x00000004 // Timer #0 signal B
#define AT91_PIO_PSR_TCLK1    0x00000008 // Timer #1 clock
#define AT91_PIO_PSR_TIOA1    0x00000010 // Timer #1 signal A
#define AT91_PIO_PSR_TIOB1    0x00000020 // Timer #1 signal B
#define AT91_PIO_PSR_TCLK2    0x00000040 // Timer #2 clock
#define AT91_PIO_PSR_TIOA2    0x00000080 // Timer #2 signal A
#define AT91_PIO_PSR_TIOB2    0x00000100 // Timer #2 signal B
#define AT91_PIO_PSR_IRQ0     0x00000200 // IRQ #0
#define AT91_PIO_PSR_IRQ1     0x00000400 // IRQ #1
#define AT91_PIO_PSR_IRQ2     0x00000800 // IRQ #2
#define AT91_PIO_PSR_FIQ      0x00001000 // FIQ
#define AT91_PIO_PSR_SCK0     0x00002000 // Serial port #0 clock
#define AT91_PIO_PSR_TXD0     0x00004000 // Serial port #0 TxD
#define AT91_PIO_PSR_RXD0     0x00008000 // Serial port #0 RxD
#define AT91_PIO_PSR_P16      0x00010000 // PIO port #16
#define AT91_PIO_PSR_P17      0x00020000 // PIO port #17
#define AT91_PIO_PSR_P18      0x00040000 // PIO port #18
#define AT91_PIO_PSR_P19      0x00080000 // PIO port #19
#define AT91_PIO_PSR_SCK1     0x00100000 // Serial port #1 clock
#define AT91_PIO_PSR_TXD1     0x00200000 // Serial port #1 TxD
#define AT91_PIO_PSR_RXD1     0x00400000 // Serial port #1 RxD
#define AT91_PIO_PSR_P23      0x00800000 // PIO port #23
#define AT91_PIO_PSR_P24      0x01000000 // PIO port #24
#define AT91_PIO_PSR_MCKO     0x02000000 // Master clock out
#define AT91_PIO_PSR_NCS2     0x04000000 // Chip select #2
#define AT91_PIO_PSR_NCS3     0x08000000 // Chip select #3
#define AT91_PIO_PSR_CS7_A20  0x10000000 // Chip select #7 or A20
#define AT91_PIO_PSR_CS6_A21  0x20000000 // Chip select #6 or A21
#define AT91_PIO_PSR_CS5_A22  0x40000000 // Chip select #5 or A22
#define AT91_PIO_PSR_CS4_A23  0x80000000 // Chip select #4 or A23

#endif

#define AT91_PIO_OER  0x10  // Output enable
#define AT91_PIO_ODR  0x14  // Output disable
#define AT91_PIO_OSR  0x18  // Output status
#define AT91_PIO_IFER 0x20  // Input Filter enable
#define AT91_PIO_IFDR 0x24  // Input Filter disable
#define AT91_PIO_IFSR 0x28  // Input Filter status
#define AT91_PIO_SODR 0x30  // Set out bits
#define AT91_PIO_CODR 0x34  // Clear out bits
#define AT91_PIO_ODSR 0x38  // Output data status
#define AT91_PIO_PDSR 0x3C  // Pin data status
#define AT91_PIO_IER  0x40  // Interrupt enable
#define AT91_PIO_IDR  0x44  // Interrupt disable
#define AT91_PIO_IMR  0x48  // Interrupt mask
#define AT91_PIO_ISR  0x4C  // Interrupt status

//=============================================================================
// Advanced Interrupt Controller (AIC)

#ifndef AT91_AIC
#define AT91_AIC      0xFFFFF000
#endif

#define AT91_AIC_SMR0   ((0*4)+0x000)
#define AT91_AIC_SMR1   ((1*4)+0x000)
#define AT91_AIC_SMR2   ((2*4)+0x000)
#define AT91_AIC_SMR3   ((3*4)+0x000)
#define AT91_AIC_SMR4   ((4*4)+0x000)
#define AT91_AIC_SMR5   ((5*4)+0x000)
#define AT91_AIC_SMR6   ((6*4)+0x000)
#define AT91_AIC_SMR7   ((7*4)+0x000)
#define AT91_AIC_SMR8   ((8*4)+0x000)
#define AT91_AIC_SMR9   ((9*4)+0x000)
#define AT91_AIC_SMR10  ((10*4)+0x000)
#define AT91_AIC_SMR11  ((11*4)+0x000)
#define AT91_AIC_SMR12  ((12*4)+0x000)
#define AT91_AIC_SMR13  ((13*4)+0x000)
#define AT91_AIC_SMR14  ((14*4)+0x000)
#define AT91_AIC_SMR15  ((15*4)+0x000)
#define AT91_AIC_SMR16  ((16*4)+0x000)
#define AT91_AIC_SMR17  ((17*4)+0x000)
#define AT91_AIC_SMR18  ((18*4)+0x000)
#define AT91_AIC_SMR19  ((19*4)+0x000)
#define AT91_AIC_SMR20  ((20*4)+0x000)
#define AT91_AIC_SMR21  ((21*4)+0x000)
#define AT91_AIC_SMR22  ((22*4)+0x000)
#define AT91_AIC_SMR23  ((23*4)+0x000)
#define AT91_AIC_SMR24  ((24*4)+0x000)
#define AT91_AIC_SMR25  ((25*4)+0x000)
#define AT91_AIC_SMR26  ((26*4)+0x000)
#define AT91_AIC_SMR27  ((27*4)+0x000)
#define AT91_AIC_SMR28  ((28*4)+0x000)
#define AT91_AIC_SMR29  ((29*4)+0x000)
#define AT91_AIC_SMR30  ((30*4)+0x000)
#define AT91_AIC_SMR31  ((31*4)+0x000)
#define AT91_AIC_SMR_LEVEL_LOW  (0<<5)
#define AT91_AIC_SMR_LEVEL_HI   (2<<5)
#define AT91_AIC_SMR_EDGE_NEG   (1<<5)
#define AT91_AIC_SMR_EDGE_POS   (3<<5)
#define AT91_AIC_SMR_PRIORITY   0x07
#define AT91_AIC_SVR0   ((0*4)+0x080)
#define AT91_AIC_SVR1   ((1*4)+0x080)
#define AT91_AIC_SVR2   ((2*4)+0x080)
#define AT91_AIC_SVR3   ((3*4)+0x080)
#define AT91_AIC_SVR4   ((4*4)+0x080)
#define AT91_AIC_SVR5   ((5*4)+0x080)
#define AT91_AIC_SVR6   ((6*4)+0x080)
#define AT91_AIC_SVR7   ((7*4)+0x080)
#define AT91_AIC_SVR8   ((8*4)+0x080)
#define AT91_AIC_SVR9   ((9*4)+0x080)
#define AT91_AIC_SVR10  ((10*4)+0x080)
#define AT91_AIC_SVR11  ((11*4)+0x080)
#define AT91_AIC_SVR12  ((12*4)+0x080)
#define AT91_AIC_SVR13  ((13*4)+0x080)
#define AT91_AIC_SVR14  ((14*4)+0x080)
#define AT91_AIC_SVR15  ((15*4)+0x080)
#define AT91_AIC_SVR16  ((16*4)+0x080)
#define AT91_AIC_SVR17  ((17*4)+0x080)
#define AT91_AIC_SVR18  ((18*4)+0x080)
#define AT91_AIC_SVR19  ((19*4)+0x080)
#define AT91_AIC_SVR20  ((20*4)+0x080)
#define AT91_AIC_SVR21  ((21*4)+0x080)
#define AT91_AIC_SVR22  ((22*4)+0x080)
#define AT91_AIC_SVR23  ((23*4)+0x080)
#define AT91_AIC_SVR24  ((24*4)+0x080)
#define AT91_AIC_SVR25  ((25*4)+0x080)
#define AT91_AIC_SVR26  ((26*4)+0x080)
#define AT91_AIC_SVR27  ((27*4)+0x080)
#define AT91_AIC_SVR28  ((28*4)+0x080)
#define AT91_AIC_SVR29  ((29*4)+0x080)
#define AT91_AIC_SVR30  ((30*4)+0x080)
#define AT91_AIC_SVR31  ((31*4)+0x080)
#define AT91_AIC_IVR    0x100
#define AT91_AIC_FVR    0x104
#define AT91_AIC_ISR    0x108
#define AT91_AIC_IPR    0x10C
#define AT91_AIC_IMR    0x110
#define AT91_AIC_CISR   0x114
#define AT91_AIC_IECR   0x120
#define AT91_AIC_IDCR   0x124
#define AT91_AIC_ICCR   0x128
#define AT91_AIC_ISCR   0x12C
#define AT91_AIC_EOI    0x130
#define AT91_AIC_SVR    0x134

//=============================================================================
// Timer / counter

#ifndef AT91_TC
#define AT91_TC         0xFFFE0000
#endif

#define AT91_TC_TC0     0x00
#define AT91_TC_CCR     0x00
#define AT91_TC_CCR_CLKEN  0x01
#define AT91_TC_CCR_CLKDIS 0x02
#define AT91_TC_CCR_TRIG   0x04
// Channel Mode Register
#define AT91_TC_CMR		   0x04
#define AT91_TC_CMR_CLKS	   0
#define AT91_TC_CMR_CLKS_MCK2      (0<<0)
#define AT91_TC_CMR_CLKS_MCK8      (1<<0)
#define AT91_TC_CMR_CLKS_MCK32     (2<<0)
#define AT91_TC_CMR_CLKS_MCK128    (3<<0)
#define AT91_TC_CMR_CLKS_MCK1024   (4<<0)
#define AT91_TC_CMR_CLKS_XC0       (5<<0)
#define AT91_TC_CMR_CLKS_XC1       (6<<0)
#define AT91_TC_CMR_CLKS_XC2       (7<<0)
#define AT91_TC_CMR_CLKI           (1<<3)
#define AT91_TC_CMR_BURST_NONE     (0<<4)
#define AT91_TC_CMR_BURST_XC0      (1<<4)
#define AT91_TC_CMR_BURST_XC1      (2<<4)
#define AT91_TC_CMR_BURST_XC2      (3<<4)
// Capture mode definitions
#define AT91_TC_CMR_LDBSTOP        (1<<6)
#define AT91_TC_CMR_LDBDIS         (1<<7)
#define AT91_TC_CMR_TRIG_NONE      (0<<8)
#define AT91_TC_CMR_TRIG_NEG       (1<<8)
#define AT91_TC_CMR_TRIG_POS       (2<<8)
#define AT91_TC_CMR_TRIG_BOTH      (3<<8)
#define AT91_TC_CMR_EXT_TRIG_TIOB  (0<<10)
#define AT91_TC_CMR_EXT_TRIG_TIOA  (1<<10)
#define AT91_TC_CMR_CPCTRG         (1<<14)
#define AT91_TC_CMR_LDRA_NONE      (0<<16)
#define AT91_TC_CMR_LDRA_TIOA_NEG  (1<<16)
#define AT91_TC_CMR_LDRA_TIOA_POS  (2<<16)
#define AT91_TC_CMR_LDRA_TIOA_BOTH (3<<16)
#define AT91_TC_CMR_LDRB_NONE      (0<<16)
#define AT91_TC_CMR_LDRB_TIOA_NEG  (1<<16)
#define AT91_TC_CMR_LDRB_TIOA_POS  (2<<16)
#define AT91_TC_CMR_LDRB_TIOA_BOTH (3<<16)
// Waveform mode definitions
#define AT91_TC_CMR_CPCSTOP        (1<<6)
#define AT91_TC_CMR_CPCDIS	   (1<<7)
#define AT91_TC_CMR_EEVTEDG_NONE   (0<<8)
#define AT91_TC_CMR_EEVTEDG_NEG    (1<<8)
#define AT91_TC_CMR_EEVTEDG_POS    (2<<8)
#define AT91_TC_CMR_EEVTEDG_BOTH   (3<<8)
#define AT91_TC_CMR_EEVT_TIOB	   (0<<10)
#define AT91_TC_CMR_EEVT_XC0       (1<<10)
#define AT91_TC_CMR_EEVT_XC1       (2<<10)
#define AT91_TC_CMR_EEVT_XC2       (3<<10)
#define AT91_TC_CMR_ENETRG	   (1<<12)
#define AT91_TC_CMR_CPCTRG	   (1<<14)
#define AT91_TC_CMR_WAVE	   (1<<15)
#define AT91_TC_CMR_ACPA_NONE	   (0<<16)
#define AT91_TC_CMR_ACPA_SET	   (1<<16)
#define AT91_TC_CMR_ACPA_CLEAR	   (2<<16)
#define AT91_TC_CMR_ACPA_TOGGLE	   (3<<16)
#define AT91_TC_CMR_ACPC_NONE	   (0<<18)
#define AT91_TC_CMR_ACPC_SET	   (1<<18)
#define AT91_TC_CMR_ACPC_CLEAR	   (2<<18)
#define AT91_TC_CMR_ACPC_TOGGLE	   (3<<18)
#define AT91_TC_CMR_AEEVT_NONE	   (0<<20)
#define AT91_TC_CMR_AEEVT_SET	   (1<<20)
#define AT91_TC_CMR_AEEVT_CLEAR	   (2<<20)
#define AT91_TC_CMR_AEEVT_TOGGLE   (3<<20)
#define AT91_TC_CMR_ASWTRG_NONE	   (0<<22)
#define AT91_TC_CMR_ASWTRG_SET	   (1<<22)
#define AT91_TC_CMR_ASWTRG_CLEAR   (2<<22)
#define AT91_TC_CMR_ASWTRG_TOGGLE  (3<<22)
#define AT91_TC_CMR_BCPB_NONE	   (0<<24)
#define AT91_TC_CMR_BCPB_SET	   (1<<24)
#define AT91_TC_CMR_BCPB_CLEAR	   (2<<24)
#define AT91_TC_CMR_BCPB_TOGGLE	   (3<<24)
#define AT91_TC_CMR_BCPC_NONE	   (0<<26)
#define AT91_TC_CMR_BCPC_SET	   (1<<26)
#define AT91_TC_CMR_BCPC_CLEAR	   (2<<26)
#define AT91_TC_CMR_BCPC_TOGGLE	   (3<<26)
#define AT91_TC_CMR_BEEVT_NONE	   (0<<28)
#define AT91_TC_CMR_BEEVT_SET	   (1<<28)
#define AT91_TC_CMR_BEEVT_CLEAR	   (2<<28)
#define AT91_TC_CMR_BEEVT_TOGGLE   (3<<28)
#define AT91_TC_CMR_BSWTRG_NONE	   (0<<30)
#define AT91_TC_CMR_BSWTRG_SET	   (1<<30)
#define AT91_TC_CMR_BSWTRG_CLEAR   (2<<30)
#define AT91_TC_CMR_BSWTRG_TOGGLE  (3<<30)

#define AT91_TC_CV      0x10
#define AT91_TC_RA      0x14
#define AT91_TC_RB      0x18
#define AT91_TC_RC      0x1C
#define AT91_TC_SR      0x20
#define AT91_TC_SR_COVF    (1<<0)  // Counter overrun
#define AT91_TC_SR_LOVR    (1<<1)  // Load overrun
#define AT91_TC_SR_CPA     (1<<2)  // RA compare
#define AT91_TC_SR_CPB     (1<<3)  // RB compare
#define AT91_TC_SR_CPC     (1<<4)  // RC compare
#define AT91_TC_SR_LDRA    (1<<5)  // Load A status
#define AT91_TC_SR_LDRB    (1<<6)  // Load B status
#define AT91_TC_SR_EXT     (1<<7)  // External trigger
#define AT91_TC_SR_CLKSTA  (1<<16) // Clock enable/disable status
#define AT91_TC_SR_MTIOA   (1<<17) // TIOA mirror
#define AT91_TC_SR_MTIOB   (1<<18) // TIOB mirror
#define AT91_TC_IER     0x24
#define AT91_TC_IER_COVF   (1<<0)  // Counter overrun
#define AT91_TC_IER_LOVR   (1<<1)  // Load overrun
#define AT91_TC_IER_CPA    (1<<2)  // RA compare
#define AT91_TC_IER_CPB    (1<<3)  // RB compare
#define AT91_TC_IER_CPC    (1<<4)  // RC compare
#define AT91_TC_IER_LDRA   (1<<5)  // Load A status
#define AT91_TC_IER_LDRB   (1<<6)  // Load B status
#define AT91_TC_IER_EXT    (1<<7)  // External trigger
#define AT91_TC_IDR     0x28
#define AT91_TC_IMR     0x2C
#define AT91_TC_TC1     0x40
#define AT91_TC_TC2     0x80
#define AT91_TC_BCR     0xC0
#define AT91_TC_BCR_SYNC   0x01
#define AT91_TC_BMR     0xC4

//=============================================================================
// External Bus Interface

#ifndef AT91_EBI
#define AT91_EBI        0xFFE00000
#endif

#define AT91_EBI_CSR0 	0x00
#define AT91_EBI_CSR1 	0x04
#define AT91_EBI_CSR2 	0x08
#define AT91_EBI_CSR3 	0x0C
#define AT91_EBI_CSR4 	0x10
#define AT91_EBI_CSR5 	0x14
#define AT91_EBI_CSR6 	0x18
#define AT91_EBI_CSR7 	0x1C  	   // Chip select
#define AT91_EBI_CSR_DBW_16 0x1    // Data bus 16 bits wide
#define AT91_EBI_CSR_DBW_8  0x2    // Data bus  8 bits wide
#define AT91_EBI_CSR_NWS_1  (0x0 << 2)
#define AT91_EBI_CSR_NWS_2  (0x1 << 2)
#define AT91_EBI_CSR_NWS_3  (0x2 << 2)
#define AT91_EBI_CSR_NWS_4  (0x3 << 2)
#define AT91_EBI_CSR_NWS_5  (0x4 << 2)
#define AT91_EBI_CSR_NWS_6  (0x5 << 2)
#define AT91_EBI_CSR_NWS_7  (0x6 << 2)
#define AT91_EBI_CSR_NWS_8  (0x7 << 2)	// Number of wait states
#define AT91_EBI_CSR_WSE    (0x1 << 5)	// Wait state enable
#define AT91_EBI_CSR_PAGES_1M  (0x0 << 7)
#define AT91_EBI_CSR_PAGES_4M  (0x1 << 7)
#define AT91_EBI_CSR_PAGES_16M (0x2 << 7)
#define AT91_EBI_CSR_PAGES_64M (0x3 << 7) // Page size
#define AT91_EBI_CSR_TDF_0  (0x0 << 9)
#define AT91_EBI_CSR_TDF_1  (0x1 << 9)
#define AT91_EBI_CSR_TDF_2  (0x2 << 9)
#define AT91_EBI_CSR_TDF_3  (0x3 << 9)
#define AT91_EBI_CSR_TDF_4  (0x4 << 9)
#define AT91_EBI_CSR_TDF_5  (0x5 << 9)
#define AT91_EBI_CSR_TDF_6  (0x6 << 9)
#define AT91_EBI_CSR_TDF_7  (0x7 << 9)	// Data float output time
#define AT91_EBI_CSR_BAT    (0x1 << 12) // Byte access type
#define AT91_EBI_CSR_CSEN   (0x1 << 13) // Chip select enable
#define AT91_EBI_CSR_BA     (0xFFF << 20) // Base address
#define AT91_EBI_RCR    0x20       // Reset control
#define AT91_EBI_RCR_RCB    0x1    // Remap command bit
#define AT91_EBI_MCR  	0x24  	   // Memory control
#define AT91_EBI_MCR_ALE_16M  0x0
#define AT91_EBI_MCR_ALE_8M   0x4
#define AT91_EBI_MCR_ALE_4M   0x5
#define AT91_EBI_MCR_ALE_2M   0x6
#define AT91_EBI_MCR_ALE_1M   0x7   // Address line enable
#define AT91_EBI_MCR_DRP      (0x1 << 4)  // Data read protocol


//=============================================================================
// Power Saving or Management

#if defined(CYGHWR_HAL_ARM_AT91_R40807) || \
    defined(CYGHWR_HAL_ARM_AT91_R40008)

// Power Saving

#ifndef AT91_PS
#define AT91_PS         0xFFFF4000
#endif

#define AT91_PS_CR        0x000    // Control
#define AT91_PS_CR_CPU    (1<<0)   // Disable CPU clock (idle mode)
#define AT91_PS_PCER      0x004    // Peripheral clock enable
#define AT91_PS_PCDR      0x008    // Peripheral clock disable
#define AT91_PS_PCSR      0x00c    // Peripheral clock status

#elif defined(CYGHWR_HAL_ARM_AT91_M42800A) || \
      defined(CYGHWR_HAL_ARM_AT91_M55800A)

// (Advanced) Power Management

#ifndef AT91_PMC
#define AT91_PMC        0xFFFF4000
#endif

#define AT91_PMC_SCER           0x00
#define AT91_PMC_SCDR           0x04
#define AT91_PMC_SCSR           0x08

#define AT91_PMC_PCER           0x10
#define AT91_PMC_PCDR           0x14
#define AT91_PMC_PCSR           0x18

#define AT91_PMC_CGMR           0x20
    
#define AT91_PMC_SR             0x30
#define AT91_PMC_IER            0x34
#define AT91_PMC_IDR            0x38
#define AT91_PMC_IMR            0x3c

#if defined(CYGHWR_HAL_ARM_AT91_M42800A)

#define AT91_PMC_PCER_US0       (1<<2)
#define AT91_PMC_PCER_US1       (1<<3)
#define AT91_PMC_PCER_SPIA      (1<<4)
#define AT91_PMC_PCER_SPIB      (1<<5)
#define AT91_PMC_PCER_TC0       (1<<6)
#define AT91_PMC_PCER_TC1       (1<<7)
#define AT91_PMC_PCER_TC2       (1<<8)
#define AT91_PMC_PCER_TC3       (1<<9)
#define AT91_PMC_PCER_TC4       (1<<10)
#define AT91_PMC_PCER_TC5       (1<<11)
#define AT91_PMC_PCER_PIOA      (1<<13)
#define AT91_PMC_PCER_PIOB      (1<<14)
    
#define AT91_PMC_CGMR_PRES_NONE       0
#define AT91_PMC_CGMR_PRES_DIV2       1
#define AT91_PMC_CGMR_PRES_DIV4       2
#define AT91_PMC_CGMR_PRES_DIV8       3
#define AT91_PMC_CGMR_PRES_DIV16      4
#define AT91_PMC_CGMR_PRES_DIV32      5
#define AT91_PMC_CGMR_PRES_DIV64      6
#define AT91_PMC_CGMR_PRES_RES        7
#define AT91_PMC_CGMR_PLLA         0x00
#define AT91_PMC_CGMR_PLLB         0x08
#define AT91_PMC_CGMR_MCK_SLCK   (0<<4)
#define AT91_PMC_CGMR_MCK_MCK    (1<<4)
#define AT91_PMC_CGMR_MCK_MCKINV (2<<4)
#define AT91_PMC_CGMR_MCK_MCKD2  (3<<4)
#define AT91_PMC_CGMR_MCKO_ENA   (0<<6)
#define AT91_PMC_CGMR_MCKO_DIS   (1<<6)
#define AT91_PMC_CGMR_CSS_SLCK   (0<<7)
#define AT91_PMC_CGMR_CSS_PLL    (1<<7)

#define AT91_PMC_CGMR_PLL_MUL(x) ((x)<<8)
#define AT91_PMC_CGMR_PLL_CNT(x) ((x)<<24)

#define AT91_PMC_SR_LOCK        0x01
    
#elif defined(CYGHWR_HAL_ARM_AT91_M55800A)

#define AT91_PMC_PCER_US0       (1<<2)
#define AT91_PMC_PCER_US1       (1<<3)
#define AT91_PMC_PCER_US2       (1<<4)
#define AT91_PMC_PCER_SPI       (1<<5)
#define AT91_PMC_PCER_TC0       (1<<6)
#define AT91_PMC_PCER_TC1       (1<<7)
#define AT91_PMC_PCER_TC2       (1<<8)
#define AT91_PMC_PCER_TC3       (1<<9)
#define AT91_PMC_PCER_TC4       (1<<10)
#define AT91_PMC_PCER_TC5       (1<<11)
#define AT91_PMC_PCER_PIOA      (1<<13)
#define AT91_PMC_PCER_PIOB      (1<<14)
#define AT91_PMC_PCER_ADC0      (1<<15)
#define AT91_PMC_PCER_ADC1      (1<<16)
#define AT91_PMC_PCER_DAC0      (1<<17)
#define AT91_PMC_PCER_DAC1      (1<<18)

#define AT91_PMC_CGMR_MOSC_XTAL       0
#define AT91_PMC_CGMR_MOSC_BYP        1
#define AT91_PMC_CGMR_MOSC_DIS   (0<<1)
#define AT91_PMC_CGMR_MOSC_ENA   (1<<1)
#define AT91_PMC_CGMR_MCKO_ENA   (0<<2)
#define AT91_PMC_CGMR_MCKO_DIS   (1<<2)
#define AT91_PMC_CGMR_PRES_NONE  (0<<4)
#define AT91_PMC_CGMR_PRES_DIV2  (1<<4)
#define AT91_PMC_CGMR_PRES_DIV4  (2<<4)
#define AT91_PMC_CGMR_PRES_DIV8  (3<<4)
#define AT91_PMC_CGMR_PRES_DIV16 (4<<4)
#define AT91_PMC_CGMR_PRES_DIV32 (5<<4)
#define AT91_PMC_CGMR_PRES_DIV64 (6<<4)
#define AT91_PMC_CGMR_PRES_RES   (7<<4)
#define AT91_PMC_CGMR_CSS_LF     (0<<14)
#define AT91_PMC_CGMR_CSS_MOSC   (1<<14)
#define AT91_PMC_CGMR_CSS_PLL    (2<<14)
#define AT91_PMC_CGMR_CSS_RES    (3<<14)
    
#define AT91_PMC_CGMR_PLL_MUL(x) ((x)<<8)
#define AT91_PMC_CGMR_OSC_CNT(x) ((x)<<16)
#define AT91_PMC_CGMR_PLL_CNT(x) ((x)<<24)

#define AT91_PMC_PCR            0x28
#define AT91_PMC_PCR_SHDALC     1
#define AT91_PMC_PCR_WKACKC     2
    
#define AT91_PMC_PMR            0x2c
#define AT91_PMC_PMR_SHDALS_TRI         0
#define AT91_PMC_PMR_SHDALS_LEVEL0      1
#define AT91_PMC_PMR_SHDALS_LEVEL1      2
#define AT91_PMC_PMR_SHDALS_RES         3
#define AT91_PMC_PMR_WKACKS_TRI    (0<<2)
#define AT91_PMC_PMR_WKACKS_LEVEL0 (1<<2)
#define AT91_PMC_PMR_WKACKS_LEVEL1 (2<<2)
#define AT91_PMC_PMR_WKACKS_RES    (3<<2)
#define AT91_PMC_PMR_ALWKEN        (1<<4)
#define AT91_PMC_PMR_ALSHEN        (1<<5)

#define AT91_PMC_PMR_WKEDG_NONE    (0<<6)
#define AT91_PMC_PMR_WKEDG_POS     (1<<6)
#define AT91_PMC_PMR_WKEDG_NEG     (2<<6)
#define AT91_PMC_PMR_WKEDG_BOTH    (3<<6)

#define AT91_PMC_SR_MOSCS       0x01
#define AT91_PMC_SR_LOCK        0x02

#endif

#elif defined(CYGHWR_HAL_ARM_AT91_JTST)
// Now power management control for the JTST
#else

#error Unknown AT91 variant

#endif


//=============================================================================
// Watchdog

#ifndef AT91_WD
#define AT91_WD             0xFFFF8000
#endif

#define AT91_WD_OMR         0x00
#define AT91_WD_OMR_WDEN    0x00000001
#define AT91_WD_OMR_RSTEN   0x00000002
#define AT91_WD_OMR_IRQEN   0x00000004
#define AT91_WD_OMR_EXTEN   0x00000008
#define AT91_WD_OMR_OKEY    (0x00000234 << 4)
#define AT91_WD_CMR         0x04
#define AT91_WD_CMR_WDCLKS  0x00000003
#define AT91_WD_CMR_HPCV    0x0000003C
#define AT91_WD_CMR_CKEY    (0x0000006E << 7)
#define AT91_WD_CR          0x08
#define AT91_WD_CR_RSTKEY   0x0000C071
#define AT91_WD_SR          0x0C
#define AT91_WD_SR_WDOVF    0x00000001

//=============================================================================
// SPI 

#ifndef AT91_SPI
#define AT91_SPI               0xFFFBC000
#endif

#define AT91_SPI_CR            0x00              // Control Register 
#define AT91_SPI_CR_SPIEN      0x00000001        // SPI Enable
#define AT91_SPI_CR_SPIDIS     0x00000002        // SPI Disable
#define AT91_SPI_CR_SWRST      0x00000080        // SPI Software reset
#define AT91_SPI_MR            0x04              // Mode Register
#define AT91_SPI_MR_MSTR       0x00000001        // Master/Slave Mode 
#define AT91_SPI_MR_PS         0x00000002        // Peripheral Select
#define AT91_SPI_MR_PCSDEC     0x00000004        // Chip Select Decode
#define AT91_SPI_MR_DIV32      0x00000008        // Clock Selection 
#define AT91_SPI_MR_LLB        0x00000080        // Local Loopback Enable
#define AT91_SPI_MR_PCS(x)     (((x)&0x0F)<<16)  // Peripheral Chip Select
#define AT91_SPI_MR_DLYBCS(x)  (((x)&0xFF)<<24)  // Delay Between Chip Selects
#define AT91_SPI_RDR           0x08              // Receive Data Register
#define AT91_SPI_TDR           0x0C              // Transmit Data Register
#define AT91_SPI_SR            0x10              // Status Register
#define AT91_SPI_SR_RDRF       0x00000001        // Receive Data Register Full
#define AT91_SPI_SR_TDRE       0x00000002        // Transmit Data Register Empty
#define AT91_SPI_SR_MODF       0x00000004        // Mode Fault Error
#define AT91_SPI_SR_OVRES      0x00000008        // Overrun Error Status
#define AT91_SPI_SR_ENDRX      0x00000010        // End of Receiver Transfer
#define AT91_SPI_SR_ENDTX      0x00000020        // End of Transmitter Transfer
#define AT91_SPI_SR_SPIENS     0x00010000        // SPI Enable Status
#define AT91_SPI_IER           0x14              // Interrupt Enable Register
#define AT91_SPI_IDR           0x18              // Interrupt Disable Register
#define AT91_SPI_IMR           0x1C              // Interrupt Mask Register
// DMA registers are PDC registers
// can be different from target to target
#ifndef AT91_SPI_RPR
#define AT91_SPI_RPR           0x20              // Receive Pointer Register
#endif
#ifndef AT91_SPI_RCR
#define AT91_SPI_RCR           0x24              // Receive Counter Register
#endif
#ifndef AT91_SPI_TPR
#define AT91_SPI_TPR           0x28              // Transmit Pointer Register
#endif
#ifndef AT91_SPI_TCR
#define AT91_SPI_TCR           0x2C              // Transmit Counter Register
#endif
#define AT91_SPI_CSR0          0x30              // Chip Select Register 0
#define AT91_SPI_CSR1          0x34              // Chip Select Register 1
#define AT91_SPI_CSR2          0x38              // Chip Select Register 2
#define AT91_SPI_CSR3          0x3C              // Chip Select Register 3
#define AT91_SPI_CSR_CPOL      0x00000001        // Clock Polarity
#define AT91_SPI_CSR_NCPHA     0x00000002        // Clock Phase
#define AT91_SPI_CSR_BITS(x)   (((x)&0x0F)<<4)   // Bits Per Transfer
#define AT91_SPI_CSR_BITS8     AT91_SPI_CSR_BITS(0)
#define AT91_SPI_CSR_BITS9     AT91_SPI_CSR_BITS(1)
#define AT91_SPI_CSR_BITS10    AT91_SPI_CSR_BITS(2)
#define AT91_SPI_CSR_BITS11    AT91_SPI_CSR_BITS(3)
#define AT91_SPI_CSR_BITS12    AT91_SPI_CSR_BITS(4)
#define AT91_SPI_CSR_BITS13    AT91_SPI_CSR_BITS(5)
#define AT91_SPI_CSR_BITS14    AT91_SPI_CSR_BITS(6)
#define AT91_SPI_CSR_BITS15    AT91_SPI_CSR_BITS(7)
#define AT91_SPI_CSR_BITS16    AT91_SPI_CSR_BITS(8)
#define AT91_SPI_CSR_SCBR(x)   (((x)&0xFF)<<8)   // Serial Clock Baud Rate 
#define AT91_SPI_CSR_DLYBS(x)  (((x)&0xFF)<<16)  // Delay Before SPCK
#define AT91_SPI_CSR_DLYBCT(x) (((x)&0xFF)<<24)  // Delay Between two transfers
 
#if defined(CYGHWR_HAL_ARM_AT91_M55800A)
 
#define AT91_SPI_PIO         AT91_PIOA
#define AT91_SPI_PIO_NPCS(x) (((x)&0x0F)<<26)
#endif

//=============================================================================
// FIQ interrupt vector which is shared by all HAL varients.

#define CYGNUM_HAL_INTERRUPT_FIQ 0
//-----------------------------------------------------------------------------
// end of var_io.h
#endif // CYGONCE_HAL_VAR_IO_H
