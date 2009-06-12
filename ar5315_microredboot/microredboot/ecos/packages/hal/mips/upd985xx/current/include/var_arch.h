#ifndef CYGONCE_HAL_VAR_ARCH_H
#define CYGONCE_HAL_VAR_ARCH_H
//==========================================================================
//
//      var_arch.h
//
//      Architecture specific abstractions
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
// Author(s):    hmt, nickg
// Contributors: nickg
// Date:         2001-05-24
// Purpose:      Define architecture abstractions
// Description:  This file contains any extra or modified definitions for
//               this variant of the architecture.
// Usage:        #include <cyg/hal/var_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================


// -------------------------------------------------------------------------
// Although the VR4100 is really a 64 bit CPU, we have defined
// target_register_t elsewhere to be 32-bits because we only support
// 32-bit mode. Registers will still be sent to GDB as 64-bit, but that's
// not relevant for CYG_HAL_GDB_REG.

#define CYG_HAL_GDB_REG CYG_WORD32

//--------------------------------------------------------------------------
// Now defines for all the system controller registers
// 
// These all exist at base 0x1000_0000 in physical memory.

#define MIPS_KSEG0_BASE      (0x80000000u)
#define MIPS_KSEG1_BASE      (0xA0000000u)

#define UPD985XX_SYSTEM_BASE (0x10000000u)

#define MIPS_IO_BASE         (MIPS_KSEG1_BASE + UPD985XX_SYSTEM_BASE)

#define UPD985XX_SYSCTL_OFF  (0x0000u)
#define UPD985XX_SYSUSB_OFF  (0x1000u)
#define UPD985XX_SYSETH_OFF  (0x2000u)

#define UPD985XX_SYSCTL_REG( n ) ((volatile unsigned int *)\
   (MIPS_IO_BASE + UPD985XX_SYSCTL_OFF + (unsigned)(n)))
#define UPD985XX_SYSUSB_REG( n ) ((volatile unsigned int *)\
   (MIPS_IO_BASE + UPD985XX_SYSUSB_OFF + (unsigned)(n)))
#define UPD985XX_SYSETH_REG( n ) ((volatile unsigned int *)\
   (MIPS_IO_BASE + UPD985XX_SYSETH_OFF + (unsigned)(n)))

// This for the few that we need in assembly
#define UPD985XX_SYSCTL_ADDR( n ) (0xb0000000 + (n))

// (the noise at the end of these lines is the default value)
#define S_GMR     UPD985XX_SYSCTL_REG( 0x00)   // General Mode Register    00000000H
#define S_GSR     UPD985XX_SYSCTL_REG( 0x04)   // General Status Register    unknown
#define S_ISR     UPD985XX_SYSCTL_REG( 0x08)   // Interrupt Status Register    00000000H
#define S_IMR     UPD985XX_SYSCTL_REG( 0x0C)   // Interrupt Mask Register    00000000H
#define S_NSR     UPD985XX_SYSCTL_REG( 0x10)   // NMI Status Register    00000000H
#define S_NMR     UPD985XX_SYSCTL_REG( 0x14)   // NMI Mask Register    00000000H
#define S_VER     UPD985XX_SYSCTL_REG( 0x18)   // Version Register    00000301H
//         N/A      UPD985XX_SYSCTL_REG( 0x1C)    reserved    00000000H
#define S_GIOER   UPD985XX_SYSCTL_REG( 0x20)   // GPIO Output Enable Register    00000000H
#define S_GOPR    UPD985XX_SYSCTL_REG( 0x24)   // GPIO Output (Write) Register    00000000H
#define S_GIPR    UPD985XX_SYSCTL_REG( 0x28)   // GPIO Input (Read) Register    00000000H
//         N/A      UPD985XX_SYSCTL_REG( 0x2C)   // Reserved    unknown
#define S_WRCR    UPD985XX_SYSCTL_REG( 0x30)   // Warm Reset Control Register    00000000H
#define S_WRSR    UPD985XX_SYSCTL_REG( 0x34)   // Warm Reset Status Register    00000000H
#define S_PWCR    UPD985XX_SYSCTL_REG( 0x38)   // Power Control Register    00000000H
#define S_PWSR    UPD985XX_SYSCTL_REG( 0x3C)   // Power Control Status Register    00000000H
//         N/A      UPD985XX_SYSCTL_REG( 0x40)    Reserved    unknown
#define ITCNTR    UPD985XX_SYSCTL_REG( 0x4C)   // IBUS Timeout Timer Control Register    00000000H
#define ITSETR    UPD985XX_SYSCTL_REG( 0x50)   // IBUS Timeout Timer Set Register    80000000H
//         N/A      UPD985XX_SYSCTL_REG( 0x54)    Reserved    unknown
//                  UPD985XX_SYSCTL_REG( 0x7F)    
#define UARTRBR   UPD985XX_SYSCTL_REG( 0x80)   // UART, Receiver Buffer Register [DLAB=0,READ]    unknown
#define UARTTHR   UPD985XX_SYSCTL_REG( 0x80)   // UART, Transmitter Holding Register [DLAB=0,WRITE]    unknown
#define UARTDLL   UPD985XX_SYSCTL_REG( 0x80)   // UART, Divisor Latch LSB Register [DLAB=1]    unknown
#define UARTIER   UPD985XX_SYSCTL_REG( 0x84)   // UART, Interrupt Enable Register [DLAB=0]    unknown
#define UARTDLM   UPD985XX_SYSCTL_REG( 0x84)   // UART, Divisor Latch MSB Register [DLAB=1]    unknown
#define UARTIIR   UPD985XX_SYSCTL_REG( 0x88)   // UART, Interrupt ID Register [READ]    unknown
// The uPD985xx devices do not support UART FIFOs.
#define UARTFCR   UPD985XX_SYSCTL_REG( 0x88)   // UART, FIFO control Register [WRITE]
#define UARTLCR   UPD985XX_SYSCTL_REG( 0x8C)   // UART, Line control Register    unknown
#define UARTMCR   UPD985XX_SYSCTL_REG( 0x90)   // UART, Modem Control Register    unknown
#define UARTLSR   UPD985XX_SYSCTL_REG( 0x94)   // UART, Line status Register    unknown
#define UARTMSR   UPD985XX_SYSCTL_REG( 0x98)   // UART, Modem Status Register    unknown
#define UARTSCR   UPD985XX_SYSCTL_REG( 0x9C)   // UART, Scratch Register    unknown
#define DSUCNTR   UPD985XX_SYSCTL_REG( 0xA0)   // DSU Control Register    00000000H
#define DSUSETR   UPD985XX_SYSCTL_REG( 0xA4)   // DSU Dead Time Set Register    80000000H
#define DSUCLRR   UPD985XX_SYSCTL_REG( 0xA8)   // DSU Clear Register    00000000H
#define DSUTIMR   UPD985XX_SYSCTL_REG( 0xAC)   // DSU Elapsed Time Register    00000000H
#define TMMR      UPD985XX_SYSCTL_REG( 0xB0)   // Timer Mode Register    00000000H
#define TM0CSR    UPD985XX_SYSCTL_REG( 0xB4)   // Timer CH0 Count Set Register    00000000H
#define TM1CSR    UPD985XX_SYSCTL_REG( 0xB8)   // Timer CH1 Count Set Register    00000000H
#define TM0CCR    UPD985XX_SYSCTL_REG( 0xBC)   // Timer CH0 Current Count Register    FFFFFFFFH
#define TM1CCR    UPD985XX_SYSCTL_REG( 0xC0)   // Timer CH1 Current Count Register    FFFFFFFFH
//         N/A      UPD985XX_SYSCTL_REG( 0xC4)    Reserved    unknown
//                  UPD985XX_SYSCTL_REG( 0xCF)    
#define ECCR      UPD985XX_SYSCTL_REG( 0xD0)   // EEPROM Command Control Register    00000000H
#define ERDR      UPD985XX_SYSCTL_REG( 0xD4)   // EEPROM Read Data Register    80000000H
#define MACAR1    UPD985XX_SYSCTL_REG( 0xD8)   // MAC Address Register 1    00000000H
#define MACAR2    UPD985XX_SYSCTL_REG( 0xDC)   // MAC Address Register 2    00000000H
#define MACAR3    UPD985XX_SYSCTL_REG( 0xE0)   // MAC Address Register 3    00000000H
//         N/A      UPD985XX_SYSCTL_REG( 0xE4)    Reserved    unknown
//                  UPD985XX_SYSCTL_REG( 0xFF)    
#define RMMDR     UPD985XX_SYSCTL_REG(0x100)   // Boot ROM Mode Register    00000000H
#define RMATR     UPD985XX_SYSCTL_REG(0x104)   // Boot ROM Access Timing Register    00000000H
#define SDMDR     UPD985XX_SYSCTL_REG(0x108)   // SDRAM Mode Register    00000330H
#define SDTSR     UPD985XX_SYSCTL_REG(0x10C)   // SDRAM Type Selection Register    00000000H
#define SDPTR     UPD985XX_SYSCTL_REG(0x110)   // SDRAM Precharge Timing Register    00000142H
//         N/A      UPD985XX_SYSCTL_REG(0x114)    ----    ----    Reserved    unknown
//                  UPD985XX_SYSCTL_REG(0x11B)    
#define SDRMR     UPD985XX_SYSCTL_REG(0x11C)   // SDRAM Refresh Mode Register    00000200H
#define SDRCR     UPD985XX_SYSCTL_REG(0x120)   // SDRAM Refresh Timer Count Register    00000200H
#define MBCR      UPD985XX_SYSCTL_REG(0x124)   // Memory Bus Control Register    00000000H
#define MESR      UPD985XX_SYSCTL_REG(0x128)   // Memory Error Status Register    00000000H
#define MEAR      UPD985XX_SYSCTL_REG(0x12C)   // Memory Error Address Register    00000000H

// The few that we need from assembly
#define S_ISR_ADR UPD985XX_SYSCTL_ADDR( 0x08)   // Interrupt Status Register    00000000H
#define S_IMR_ADR UPD985XX_SYSCTL_ADDR( 0x0C)   // Interrupt Mask Register    00000000H

#define RMMDR_ADR UPD985XX_SYSCTL_ADDR(0x100)   // Boot ROM Mode Register    00000000H
#define RMATR_ADR UPD985XX_SYSCTL_ADDR(0x104)   // Boot ROM Access Timing Register    00000000H
#define SDMDR_ADR UPD985XX_SYSCTL_ADDR(0x108)   // SDRAM Mode Register    00000330H
#define SDTSR_ADR UPD985XX_SYSCTL_ADDR(0x10C)   // SDRAM Type Selection Register    00000000H
#define SDPTR_ADR UPD985XX_SYSCTL_ADDR(0x110)   // SDRAM Precharge Timing Register    00000142H

#define SDRMR_ADR UPD985XX_SYSCTL_ADDR(0x11C)   // SDRAM Refresh Mode Register    00000200H
#define SDRCR_ADR UPD985XX_SYSCTL_ADDR(0x120)   // SDRAM Refresh Timer Count Register    00000200H
#define  MBCR_ADR UPD985XX_SYSCTL_ADDR(0x124)   // Memory Bus Control Register    00000000H
#define  MESR_ADR UPD985XX_SYSCTL_ADDR(0x128)   // Memory Error Status Register    00000000H
#define  MEAR_ADR UPD985XX_SYSCTL_ADDR(0x12C)   // Memory Error Address Register    00000000H

// ---------------------------------------------------------------------------
// Contents of all these glorious registers:

// --------- general ---------
// S_GMR     General Mode Register    00000000H
#define S_GMR_CRST    1     // Cold Reset
#define S_GMR_IAEN    2     // IBUS Arbiter Enable
#define S_GMR_MPFD    4     // Memory-to-CPU Prefetch FIFO disable
#define S_GMR_UCSEL   8     // UART Source Clock Selection (0 = CPU, 1 = ext)
#define S_GMR_HSWP   (1<<8) // HIF Block Data swap function disable
#define S_GMR_MSWP   (1<<9) // MIF Block Data swap function disable 

// S_GSR     General Status Register
#define S_GSR_ENDCEN  1
#define S_GSR_CCLKSEL 2

// S_ISR     Interrupt Status Register    00000000H
#define S_ISR_TM0IS  (1<<0) // TIMER CH0 interrupt.
#define S_ISR_TM1IS  (1<<1) // TIMER CH1 interrupt.
#define S_ISR_UARTIS (1<<2) // UART interrupt.
#define S_ISR_EXTIS  (1<<3) // External Interrupt.
#define S_ISR_WUIS   (1<<4) // Wakeup Interrupt.

// S_IMR     Interrupt Mask Register    00000000H
// see S_ISR; write a 1 to unmask, 0 to mask.

// S_NSR     NMI Status Register    00000000H
// S_NMR     NMI Mask Register    00000000H

// S_VER     Version Register    00000301H

// --------- GPIO ---------
// S_GIOER   GPIO Output Enable Register    00000000H
// S_GOPR    GPIO Output (Write) Register    00000000H
// S_GIPR    GPIO Input (Read) Register    00000000H
// 16-bit regsiters that do the utterly obvious thing.

// --------- reset ---------
// S_WRCR    Warm Reset Control Register    00000000H
#define S_WRCR_USBWR    (1<<0) // Warm Reset request for USB Controller
#define S_WRCR_MACWR    (1<<1) // Warm Reset request for Ethernet Controller
#define S_WRCR_UARTWR   (1<<4) // Warm Reset request for UART

// S_WRSR    Warm Reset Status Register    00000000H
// See S_WRCR; 1 <=> Ready, 0 <=> performing warm reset.

// --------- power control of USB/ETH peripherals ---------
// S_PWCR    Power Control Register    00000000H
// S_PWSR    Power Control Status Register    00000000H

// --------- bus timouts ---------
// ITCNTR    IBUS Timeout Timer Control Register    00000000H
// ITSETR    IBUS Timeout Timer Set Register    80000000H

// --------- UART ---------
// UARTRBR   UART, Receiver Buffer Register [DLAB=0,READ]
// UARTTHR   UART, Transmitter Holding Register [DLAB=0,WRITE]
// UARTDLL   UART, Divisor Latch LSB Register [DLAB=1]
// The external (18.432MHz) clock is not present. See S_GMR_UCSEL.
// See also UARTDLM below.  So we use the internal 50MHz clock.

//#define UARTCLOCK (18432000)
#define UARTCLOCK (50 * 1000 * 1000)
#define UARTDLL_VAL( _baud_ ) ((UARTCLOCK / 16) / (_baud_) )

// UARTIER   UART, Interrupt Enable Register [DLAB=0]
#define UARTIER_ERBFI    (1<<0) // UART Receive data Buffer Full Interrupt
#define UARTIER_ERBEI    (1<<1) // UART Transmitter Buffer empty Interrupt
#define UARTIER_ERBLI    (1<<2) // UART Line status Interrupts
#define UARTIER_ERBMI    (1<<3) // UART Modem status Interrupts

// UARTDLM   UART, Divisor Latch MSB Register [DLAB=1]
#define UARTDLM_ANY_BAUD (0)
#define UARTDLM_VAL( _baud_ )  (UARTDLL_VAL( _baud_ ) >> 8)

// UARTIIR   UART, Interrupt ID Register [READ]
#define UARTIIR_INTPENDL   (1<<0) // No Pending interrupts

// mask to give one of:
#define UARTIIR_UIID_MASK  (7<<1) // Indicates the priority level of pending interrupt.

#define UARTIIR_RXERROR    (3<<1) // Receiver Line Error
#define UARTIIR_RXD_AVAIL  (2<<1) // Received data available
#define UARTIIR_CHAR_TO    (6<<1) // Character timeout
#define UARTIIR_TX_EMPTY   (1<<1) // Transmitter Register Empty
#define UARTIIR_MODEM      (0<<1) // Modem Status: CTS_L, DSR_L or DCD_L

// UARTFCR   UART, FIFO control Register [WRITE]
// ...is not supported.  But nontheless it appears necessary to write it.
#define UARTFCR_16550_MODE (6)    // and clear the FIFOs.

// UARTLCR   UART, Line control Register
// Word length
#define UARTLCR_8 (0x03)
#define UARTLCR_7 (0x02)
// Stop bits
#define UARTLCR_STB1 (0x00)
#define UARTLCR_STB2 (0x04)
// Parity
#define UARTLCR_NOP (0x00)
#define UARTLCR_EP  (0x18)
#define UARTLCR_OP  (0x08)

// Just predefine the pattern for 8-N-1...
#define UARTLCR_8N1 (0x03)

// Divisor latch access bit; or this with one of the above.
#define UARTLCR_DLAB (0x80)

// UARTMCR   UART, Modem Control Register
#define UARTMCR_DTR (1<<0) // Data Terminal Ready.
#define UARTMCR_RTS (1<<1) // Request To Send.

// UARTLSR   UART, Line status Register
#define UARTLSR_DR    (1<<0) // Receive-Data Ready.
#define UARTLSR_OE    (1<<1) // Receive-Data Overrun Error.
#define UARTLSR_PE    (1<<2) // Receive-Data Parity Error.
#define UARTLSR_FE    (1<<3) // Receive-Data Framing Error.
#define UARTLSR_BI    (1<<4) // Break Interrupt.
#define UARTLSR_THRE  (1<<5) // Transmitter Holding Register Empty.
#define UARTLSR_TEMT  (1<<6) // Transmitter Empty.
#define UARTLSR_RFERR (1<<7) //  Receiver FIFO Error.

// UARTMSR   UART, Modem Status Register
#define UARTMSR_DCTS (1<<0) // Delta Clear To Send.
#define UARTMSR_DDSR (1<<1) // Delta Data Set Ready.
#define UARTMSR_TERI (1<<2) // Trailing Edge Ring Indicato
#define UARTMSR_DDCD (1<<3) // Delta Data Carrier Detect.
#define UARTMSR_CTS  (1<<4) // Clear To Send.
#define UARTMSR_DSR  (1<<5) // Data Set Ready.
#define UARTMSR_RI   (1<<6) // Ring Indicator.
#define UARTMSR_DCD  (1<<7) // Data Carrier Detect.

// UARTSCR   UART, Scratch Register    unknown


// --------- watchdog aka dead man's switch ---------
// DSUCNTR   DSU Control Register    00000000H
// DSUSETR   DSU Dead Time Set Register    80000000H
// DSUCLRR   DSU Clear Register    00000000H
// DSUTIMR   DSU Elapsed Time Register    00000000H

// --------- additional timers ---------
// TMMR      Timer Mode Register    00000000H
// TM0CSR    Timer CH0 Count Set Register    00000000H
// TM1CSR    Timer CH1 Count Set Register    00000000H
// TM0CCR    Timer CH0 Current Count Register    FFFFFFFFH
// TM1CCR    Timer CH1 Current Count Register    FFFFFFFFH

// --------- serial eeprom ---------
// ECCR      EEPROM Command Control Register    00000000H
// ERDR      EEPROM Read Data Register    80000000H
// MACAR1    MAC Address Register 1    00000000H
// MACAR2    MAC Address Register 2    00000000H
// MACAR3    MAC Address Register 3    00000000H

// --------- memory control ---------
// RMMDR     Boot ROM Mode Register    00000000H
// RMATR     Boot ROM Access Timing Register    00000000H
#define RMMDR_FLASH_WRITE_ENABLE (0x100)

#define RMMDR_28F640      (0)
#define RMMDR_28F320      (0)
#define RMMDR_29LV160_120 (3) // sic. from customer doc
#define RMMDR_29LV160_90  (3) // even though "3" is a reserved value
#define RMMDR_29LV160_70  (3) // maybe it means "1".

#define RMATR_28F640      (5)
#define RMATR_28F320      (4)
#define RMATR_29LV160_120 (5)
#define RMATR_29LV160_90  (4)
#define RMATR_29LV160_70  (3)

// SDMDR     SDRAM Mode Register    00000330H
// SDTSR     SDRAM Type Selection Register    00000000H
// SDPTR     SDRAM Precharge Timing Register    00000142H
// SDRMR     SDRAM Refresh Mode Register    00000200H
// SDRCR     SDRAM Refresh Timer Count Register    00000200H

#if 1
// initial settings from customer doc.
#define SDMDR_INIT (0x230)  // 230 from the hardware, 330 from doc
#define SDTSR_INIT (0x180 | 0x20 | 0x1) 
#define SDPTR_INIT (0x111) 
#define SDRMR_INIT (0x600) 
#else
// optimized setting "don't be used before qualification"
#define SDMDR_INIT (0x120) 
#define SDTSR_INIT (0x180 | 0x20 | 0x1) 
#define SDPTR_INIT (0x100) 
#define SDRMR_INIT (0x600) 
#endif

// These are used for decoding SEGV types.
// MBCR      Memory Bus Control Register    00000000H
// MESR      Memory Error Status Register    00000000H
// MEAR      Memory Error Address Register    00000000H

// --------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
