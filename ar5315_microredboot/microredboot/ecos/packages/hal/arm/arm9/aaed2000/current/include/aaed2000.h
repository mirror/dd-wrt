#ifndef CYGONCE_AAED2000_H
#define CYGONCE_AAED2000_H
/*=============================================================================
//
//      aaed2000.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
// Date:         2001-10-30
// Purpose:      Agilent/AAED2000 platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/aaed2000.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

//---------------------------------------------------------------------------
// Memory layout details needed by conversion macro
#define AAED2000_SDRAM_PHYS_BASE         0xF0000000
#define AAED2000_SDRAM_VIRT_BASE         0x00000000
#define AAED2000_SDRAM_SIZE              0x02000000
#define AAED2000_SDRAM_MASK              (AAED2000_SDRAM_SIZE-1)

#define AAED2000_FLASH_PHYS_BASE         0x00000000
#define AAED2000_FLASH_VIRT_BASE         0x60000000
#define AAED2000_FLASH_SIZE              0x02000000
#define AAED2000_FLASH_MASK              (AAED2000_FLASH_SIZE-1)

//---------------------------------------------------------------------------
// Clock and state controller
#define AAEC_CSC_BLEOI            0x80000410 // battery low end of interrupt
#define AAEC_CSC_MCEOI            0x80000414 // media changed end of interrupt
#define AAEC_CSC_TEOI             0x80000418 // tick end of interrupt
#define AAEC_CSC_CLKSET           0x80000420

#define AAEC_CSC_CLKSET_PLL       0x80000000
#define AAEC_CSC_CLKSET_SMCROM    0x01000000
#define AAEC_CSC_CLKSET_PS(_n_)       ((_n_)<<18) // values 0-3
#define AAEC_CSC_CLKSET_PCLKDIV(_n_)  ((_n_)<<16) // values 0-3
#define AAEC_CSC_CLKSET_MAINDIV2(_n_) ((_n_)<<11) // values 0-31
#define AAEC_CSC_CLKSET_MAINDIV1(_n_) ((_n_)<<7)  // values 0-15
#define AAEC_CSC_CLKSET_PREDIV(_n_)   ((_n_)<<2)  // values 0-31
#define AAEC_CSC_CLKSET_HCLKDIV(_n_)  ((_n_))     // values 0-3


#define CYGNUM_HAL_ARM_AAED2000_BUS_CLOCK_MHZ ((CYGNUM_HAL_ARM_AAED2000_BUS_CLOCK+500000)/1000000)

#define AAEC_CSC_CLKSET_INIT                                            \
  (  AAEC_CSC_CLKSET_HCLKDIV(CYGNUM_HAL_ARM_AAED2000_CLOCK_HCLKDIV)     \
   | AAEC_CSC_CLKSET_PREDIV(CYGNUM_HAL_ARM_AAED2000_CLOCK_PREDIV)       \
   | AAEC_CSC_CLKSET_MAINDIV1(CYGNUM_HAL_ARM_AAED2000_CLOCK_MAINDIV1)   \
   | AAEC_CSC_CLKSET_MAINDIV2(CYGNUM_HAL_ARM_AAED2000_CLOCK_MAINDIV2)   \
   | AAEC_CSC_CLKSET_PCLKDIV(CYGNUM_HAL_ARM_AAED2000_CLOCK_PCLKDIV)     \
   | AAEC_CSC_CLKSET_PS(CYGNUM_HAL_ARM_AAED2000_CLOCK_PS)               \
   | AAEC_CSC_CLKSET_SMCROM)

//---------------------------------------------------------------------------
// Interrupt controller
#define AAEC_INT_SR               0x80000500
#define AAEC_INT_RSR              0x80000504  // Raw [unmasked] interrupt status
#define AAEC_INT_ENS              0x80000508
#define AAEC_INT_ENC              0x8000050c
#define AAEC_INT_TEST1            0x80000514
#define AAEC_INT_TEST2            0x80000518

#define AAEC_INTS_T3OI            CYGNUM_HAL_INTERRUPT_TC3OI // Timer #3 overflow

//---------------------------------------------------------------------------
// UARTs
#define AAEC_UART1                0x80000600
#define AAEC_UART2                0x80000700
#define AAEC_UART3                0x80000800

#define AAEC_UART2_UMS2EOI        0x80000714 // modem end of interrupt
#define AAEC_UART2_UMS3EOI        0x80000814 // modem end of interrupt

#define AAEC_UART_DATA    0x0000  // Data/FIFO register
#define AAEC_UART_LCR     0x0004  // Control register
#define AAEC_UART_LCR_BRK      0x0001 // Send break
#define AAEC_UART_LCR_PEN      0x0002 // Enable parity
#define AAEC_UART_LCR_EP       0x0004 // Odd/Even parity
#define AAEC_UART_LCR_S2       0x0008 // One/Two stop bits
#define AAEC_UART_LCR_FIFO     0x0010 // Enable FIFO
#define AAEC_UART_LCR_WL5      0x0000 // Word length - 5 bits
#define AAEC_UART_LCR_WL6      0x0020 // Word length - 6 bits
#define AAEC_UART_LCR_WL7      0x0040 // Word length - 7 bits
#define AAEC_UART_LCR_WL8      0x0060 // Word length - 8 bits
#define AAEC_UART_BAUD    0x0008  // Baud rate
#define AAEC_UART_CTRL    0x000C  // Control register
#define AAEC_UART_CTRL_ENAB    0x0001 // Enable uart
#define AAEC_UART_CTRL_SIR     0x0002 // Enable SIR IrDA
#define AAEC_UART_CTRL_SIRLP   0x0004 // Enable low power IrDA
#define AAEC_UART_CTRL_RXP     0x0008 // Receive pin polarity
#define AAEC_UART_CTRL_TXP     0x0010 // Transmit pin polarity
#define AAEC_UART_CTRL_MXP     0x0020 // Modem pin polarity
#define AAEC_UART_CTRL_LOOP    0x0040 // Loopback mode
#define AAEC_UART_CTRL_SIRBD   0x0080 // blanking disable
#define AAEC_UART_STATUS  0x0010  // Status
#define AAEC_UART_STATUS_CTS   0x0001 // Clear-to-send status
#define AAEC_UART_STATUS_DSR   0x0002 // Data-set-ready status
#define AAEC_UART_STATUS_DCD   0x0004 // Data-carrier-detect status
#define AAEC_UART_STATUS_TxBSY 0x0008 // Transmitter busy
#define AAEC_UART_STATUS_RxFE  0x0010 // Receive FIFO empty
#define AAEC_UART_STATUS_TxFF  0x0020 // Transmit FIFO full
#define AAEC_UART_STATUS_RxFF  0x0040 // Receive FIFO full
#define AAEC_UART_STATUS_TxFE  0x0080 // Transmit FIFO empty
#define AAEC_UART_INT     0x0014  // Interrupt status
#define AAEC_UART_INTM    0x0018  // Interrupt mask register
#define AAEC_UART_INTRES  0x001c  // Interrupt result (masked interrupt status)
#define AAEC_UART_INT_RIS      0x0001 // Rx interrupt
#define AAEC_UART_INT_TIS      0x0002 // Tx interrupt
#define AAEC_UART_INT_MIS      0x0004 // Modem status interrupt
#define AAEC_UART_INT_RTIS     0x0008 // Rx timeout interrupt
//#define AAEC_UART_MCTRL   0x0100  // Modem control

//---------------------------------------------------------------------------
// Pump control
#define AAEC_PUMP_CONTROL         0x80000900   // Control
#define AAEC_PUMP_FREQUENCY       0x80000908   // Frequency

//---------------------------------------------------------------------------
// Codec
#define AAEC_COD_CDEOI            0x80000a0c // codec end of interrupt

//---------------------------------------------------------------------------
// Synchronous Serial Peripheral (SSP)
#define AAEC_SSP_CR0              0x80000B00   // Control Register 0
#define AAEC_SSP_CR0_SCR            8            // Serial clock rate - Bits 15..8
#define AAEC_SSP_CR0_SCR_MASK       (0x7F<<AAEC_SSP_CR0_SCR)    
#define AAEC_SSP_CR0_SSE            7            // SSP enable/disable
#define AAEC_SSP_CR0_SSE_MASK       (1<<AAEC_SSP_CR0_SSE)
#define AAEC_SSP_CR0_SSE_ON         1
#define AAEC_SSP_CR0_SSE_OFF        0
#define AAEC_SSP_CR0_FRF            4            // Frame format
#define AAEC_SSP_CR0_FRF_MASK       (0x3<<AAEC_SSP_CR0_FRF)
#define AAEC_SSP_CR0_FRF_MOT        0               // Motorola SPI
#define AAEC_SSP_CR0_FRF_TI         1               // TI synchronous serial frame
#define AAEC_SSP_CR0_FRF_NAT        2               // National microwire
#define AAEC_SSP_CR0_SIZE           0            // Data size
#define AAEC_SSP_CR0_SIZE_MASK      (0xF<<AAEC_SSP_CR0_SIZE)
#define AAEC_SSP_CR1              0x80000B04   // Control Register 1
#define AAEC_SSP_CR1_TXIDLE         7            // Tx idle interrupt
#define AAEC_SSP_CR1_FEN            6            // FIFO enable
#define AAEC_SSP_CR1_RORIE          5            // Rx FIFO overrun interrupt
#define AAEC_SSP_CR1_SPH            4            // SCLK phase
#define AAEC_SSP_CR1_SPO            3            // SCLK polarity
#define AAEC_SSP_CR1_LBM            2            // Lookpback
#define AAEC_SSP_CR1_TIE            1            // Tx interrupt
#define AAEC_SSP_CR1_RIE            0            // Rx Interrupt
#define AAEC_SSP_IIR              0x80000B08   // Interrupt ID register (read)
#define AAEC_SSP_IIR_TXIDLE         7            // Tx idle interrupt
#define AAEC_SSP_IIR_ROR            6            // Rx overrun
#define AAEC_SSP_IIR_TI             1            // Tx FIFO less than half full
#define AAEC_SSP_IIR_RI             0            // Rx FIFO more than half full
#define AAEC_SSP_ICR              0x80000B08   // Interrupt Clear register (write)
#define AAEC_SSP_DR               0x80000B0C   // Data [FIFO] register
#define AAEC_SSP_CPSR             0x80000B10   // Clock prescale
#define AAEC_SSP_SR               0x80000B14   // Status register
#define AAEC_SSP_SR_RFF             8            // Rx FIFO full
#define AAEC_SSP_SR_TFE             7            // Tx FIFO empty
#define AAEC_SSP_SR_ROR             6            // Rx FIFO overrun
#define AAEC_SSP_SR_RHF             5            // Rx FIFO half full
#define AAEC_SSP_SR_THE             4            // Tx FIFO half empty
#define AAEC_SSP_SR_BSY             3            // SSP is busy
#define AAEC_SSP_SR_RNE             2            // Rx FIFO not empty
#define AAEC_SSP_SR_TNF             1            // Tx FIFO not full

//---------------------------------------------------------------------------
// Timer/counter
#define AAEC_TMR_T1_BASE          0x80000C00   // Timer #1 - preload
#define AAEC_TMR_T1LOAD           0x80000C00   // Timer #1 - preload
#define AAEC_TMR_T1VALUE          0x80000C04   // Timer #1 - current value
#define AAEC_TMR_T1CONTROL        0x80000C08   // Timer #1 - control
#define AAEC_TMR_T1EOI            0x80000C0C   // Timer #1 - clear [end] interrupt
#define AAEC_TMR_T2LOAD           0x80000C20   // Timer #2 - preload
#define AAEC_TMR_T2VALUE          0x80000C24   // Timer #2 - current value
#define AAEC_TMR_T2CONTROL        0x80000C28   // Timer #2 - control
#define AAEC_TMR_T2EOI            0x80000C2C   // Timer #2 - clear [end] interrupt
#define AAEC_TMR_BZCONT           0x80000C40
#define AAEC_TMR_T3LOAD           0x80000C80   // Timer #3 - preload
#define AAEC_TMR_T3VALUE          0x80000C84   // Timer #3 - current value
#define AAEC_TMR_T3CONTROL        0x80000C88   // Timer #3 - control
#define AAEC_TMR_T3EOI            0x80000C8C   // Timer #3 - clear [end] interrupt

#define AAEC_TMR_TxLOAD_OFFSET    0
#define AAEC_TMR_TxVALUE_OFFSET   4
#define AAEC_TMR_TxCONTROL_OFFSET 8
#define AAEC_TMR_TxEOI_OFFSET     12

#define AAEC_TMR_TxCONTROL_ENABLE   (1<<7)       // Enable (start) timer
#define AAEC_TMR_TxCONTROL_MODE     (1<<6)       // Operating mode
#define AAEC_TMR_TxCONTROL_MODE_FREE     (0x00&AAEC_TMR_TxCONTROL_MODE)
#define AAEC_TMR_TxCONTROL_MODE_PERIODIC (0xFF&AAEC_TMR_TxCONTROL_MODE)
#define AAEC_TMR_TxCONTROL_CLKSEL   (1<<3)       // Clock select (timer 1,2)
#define AAEC_TMR_TxCONTROL_508KHZ   (1<<3)
#define AAEC_TMR_TxCONTROL_2KHZ     (0<<3)

#define AAEC_TMR_TxCONTROL_508KHZ_uS(_n_) ((_n_)*508000/1000000)

//---------------------------------------------------------------------------
// RTC
#define AAEC_RTC_RTCEOI           0x80000d10   // RTC end of interrupt

//---------------------------------------------------------------------------
// GPIO registers
#define AAEC_PCDR                 0x80000e08
#define AAEC_PBDDR                0x80000e14
#define AAEC_PCCDR                0x80000e18
#define AAEC_KSCAN                0x80000e28
#define AAEC_PINMUX               0x80000e2c
#define AAEC_PFDR                 0x80000e30
#define AAEC_PFDDR                0x80000e34
#define AAEC_GPIO_INT_TYPE1       0x80000e4c
#define AAEC_GPIO_INT_TYPE2       0x80000e50
#define AAEC_GPIO_FEOI            0x80000e54
#define AAEC_GPIO_INTEN           0x80000e58
#define AAEC_GPIO_INT_STATUS      0x80000e5c
#define AAEC_PINMUX_UART3CON      0x00000008
#define AAEC_PINMUX_CODECON       0x00000004
#define AAEC_PINMUX_PD0CON        0x00000002
#define AAEC_PINMUX_PE0CON        0x00000001


//---------------------------------------------------------------------------
// Static memory controller
#define AAEC_SMCBCR0              0x80002000
#define AAEC_SMCBCR1              0x80002004
#define AAEC_SMCBCR2              0x80002008
#define AAEC_SMCBCR3              0x8000200c

#define AAEC_SMCBCR_MW8           0x00000000
#define AAEC_SMCBCR_MW16          0x10000000
#define AAEC_SMCBCR_MW32          0x30000000
#define AAEC_SMCBCR_PME           0x08000000
#define AAEC_SMCBCR_WP            0x04000000
#define AAEC_SMCBCR_WPERR         0x02000000
#define AAEC_SMCBCR_WST(_n_)      (((((_n_)-1)&0x1f)<<11) | ((((_n_)-1)&0x1f)<<5)) // for n 1-32
#define AAEC_SMCBCR_IDCY(_n_)     ((((_n_)-1)&0x0f)<<0)  // for n 1-16

// These settings come from the Agilent startup.s file
// [note, the WST values match their values, not the comments]
// CS0: Flash, access=90ns, hold=30ns
// CS1: ethernet, access=162ns, hold=47ns
// CS2: GPIO, access=14ns, hold=14ns
#if (75 == CYGNUM_HAL_ARM_AAED2000_BUS_CLOCK_MHZ)
# define _CS0_WST   8
# define _CS0_IDCY  3
# define _CS1_WST  14
# define _CS1_IDCY  4
# define _CS3_WST   3
# define _CS3_IDCY  2
#elif (83 == CYGNUM_HAL_ARM_AAED2000_BUS_CLOCK_MHZ)
# define _CS0_WST   9
# define _CS0_IDCY  3
# define _CS1_WST  15
# define _CS1_IDCY  4
# define _CS3_WST   3
# define _CS3_IDCY  2
#else
# error "Unsupported clocking"
#endif


//---------------------------------------------------------------------------
// Synchronous memory controller
#define AAEC_SMC_GLOBAL           0x80002404
#define AAEC_SMC_REFRESH_TIME     0x80002408
#define AAEC_SMC_BOOT_STATUS      0x8000240c
#define AAEC_SMC_DEV0             0x80002410
#define AAEC_SMC_DEV1             0x80002414
#define AAEC_SMC_DEV2             0x80002418
#define AAEC_SMC_DEV3             0x8000241c

#define AAEC_SMC_GLOBAL_CKE       0x80000000
#define AAEC_SMC_GLOBAL_CS        0x40000000
#define AAEC_SMC_GLOBAL_LCR       0x00000040
#define AAEC_SMC_GLOBAL_BUSY      0x00000020
#define AAEC_SMC_GLOBAL_MRS       0x00000002
#define AAEC_SMC_GLOBAL_INIT      0x00000001

#define AAEC_SMC_GLOBAL_CMD_NOP    (AAEC_SMC_GLOBAL_INIT|AAEC_SMC_GLOBAL_MRS)
#define AAEC_SMC_GLOBAL_CMD_PREALL (AAEC_SMC_GLOBAL_INIT)
#define AAEC_SMC_GLOBAL_CMD_MODE   (AAEC_SMC_GLOBAL_MRS)
#define AAEC_SMC_GLOBAL_CMD_ENABLE (AAEC_SMC_GLOBAL_CKE)


#define AAEC_SMC_DEV_AUTOP        0x01000000
#define AAEC_SMC_DEV_RAS_2        0x00200000
#define AAEC_SMC_DEV_RAS_3        0x00300000
#define AAEC_SMC_DEV_WBL_4        0x00080000
#define AAEC_SMC_DEV_WBL_1        0x00000000
#define AAEC_SMC_DEV_CASLAT(_n_)  (((_n_)-1)<<16) // 2-7
#define AAEC_SMC_DEV_2KPAGE       0x00000040
#define AAEC_SMC_DEV_SROMLL       0x00000020
#define AAEC_SMC_DEV_SROM512      0x00000010
#define AAEC_SMC_DEV_BANKS_2      0x00000008
#define AAEC_SMC_DEV_BANKS_4      0x00000000
#define AAEC_SMC_DEV_WIDTH16      0x00000004
#define AAEC_SMC_DEV_WIDTH32      0x00000000

#define AAEC_SMC_DEV_INIT         ( AAEC_SMC_DEV_RAS_2 \
                                   |AAEC_SMC_DEV_CASLAT(3) \
                                   |AAEC_SMC_DEV_BANKS_2)

//---------------------------------------------------------------------------
// LCD controller
#define AAEC_LCD_TIMING0          0x80003000   // Timing registers
#define AAEC_LCD_TIMING1          0x80003004
#define AAEC_LCD_TIMING2          0x80003008
#define AAEC_LCD_TIMING3          0x8000300C
#define AAEC_LCD_UPBASE           0x80003010   // Upper panel DMA address
#define AAEC_LCD_LPBASE           0x80003014   // Lower panel DMA address
#define AAEC_LCD_MASK             0x80003018   // Status mask
#define AAEC_LCD_CONTROL          0x8000301C   // Control
#define AAEC_LCD_CONTROL_ENAB       0x00000001    // Enable controller
#define AAEC_LCD_CONTROL_PWR_ENAB   0x00000800    // Enables signals
#define AAEC_LCD_STATUS           0x80003020   // Status
#define AAEC_LCD_INTERRUPT        0x80003024   // Interrupts
#define AAEC_LCD_UPCURR           0x80003028   // Upper panel current address
#define AAEC_LCD_LPCURR           0x8000302C   // Lower panel current address
#define AAEC_LCD_LPOVERFLOW       0x80003030   // Panel overflow 
#define AAEC_LCD_PALETTE          0x80003200   // Palette

//---------------------------------------------------------------------------
// Extended GPIO bits [platform specific]
#define AAED_EXT_GPIO             0x30000000
#define AAED_EXT_GPIO_KBD_SCAN      0x00003FFF // Keyboard scan data
#define AAED_EXT_GPIO_PWR_INT       0x00008FFF // Smart battery charger interrupt
#define AAED_EXT_GPIO_SWITCHES      0x000F0000 // DIP switches
#define AAED_EXT_GPIO_SWITCHES_SHIFT 16
#define AAED_EXT_GPIO_USB_VBUS      0x00400000 // USB Vbus sense
#define AAED_EXT_GPIO_LCD_PWR_EN    0x02000000 // LCD (& backlight) power enable
#define AAED_EXT_GPIO_LED0          0x20000000 // LED 0 (0=>ON, 1=>OFF)
#define AAED_EXT_GPIO_LED1          0x40000000 // LED 1 (0=>ON, 1=>OFF)
#define AAED_EXT_GPIO_LED2          0x80000000 // LED 2 (0=>ON, 1=>OFF)

/*---------------------------------------------------------------------------*/
/* end of aaed2000.h                                                          */
#endif /* CYGONCE_AAED2000_H */
