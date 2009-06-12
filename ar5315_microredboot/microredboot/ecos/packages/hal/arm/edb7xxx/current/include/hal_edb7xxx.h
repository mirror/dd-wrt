#ifndef CYGONCE_HAL_EDB7XXX_H
#define CYGONCE_HAL_EDB7XXX_H

/*=============================================================================
//
//      hal_edb7xxx.h
//
//      HAL Support for Kernel Diagnostic Routines
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
// Contributors: gthomas
// Date:         1999-04-19
// Purpose:      Cirrus Logic EDB7XXX hardware description
// Description:
// Usage:        #include <cyg/hal/hal_edb7xxx.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

// Note: these defintions match the documentation, thus no attempt is made
// to sanitise (mangle) the names.  Also, care should be taken to keep this
// clean for use in assembly code (no "C" constructs).

// Comment above notwithstanding, this is needed for the clock UART_DIVISOR
// selection below.
#include <pkgconf/hal_arm_edb7xxx.h>

#define PADR    0x80000000 // Port A data register
#define PBDR    0x80000001 // Port B data register
#define PDDR    0x80000003 // Port D data register
#define PADDR   0x80000040 // Port A data direction register
#define PBDDR   0x80000041 // Port B data direction register
#define PDDDR   0x80000043 // Port D data direction register
#define PEDR    0x80000080 // Port E data register
#define PEDDR   0x800000C0 // Port E data direction register

#define SYSCON1 0x80000100 // System control register #1
#define SYSCON1_KBD_CTL     0xF // Keyboard scan - mask
#define SYSCON1_KBD_HIGH      0 // Keyboard scan - all columns high
#define SYSCON1_KBD_LOW       1 // Keyboard scan - all columns low
#define SYSCON1_KBD_TRISTATE  2 // Keyboard scan - all columns tri-state
#define SYSCON1_KBD_COL(n) (n+8)// Keyboard scan - select column 'n'
#define SYSCON1_TC1M    (1<<4)  // Timer/counter #1 - prescale mode
#define SYSCON1_TC1S    (1<<5)  // Timer/counter #1 - source (1=512KHz,0=2KHz)
#define SYSCON1_TC2M    (1<<6)  // Timer/counter #2 - prescale mode
#define SYSCON1_TC2S    (1<<7)  // Timer/counter #2 - source (1=512KHz,0=2KHz)
#define SYSCON1_UART1EN (1<<8)  // UART #1 enable
#define SYSCON1_BZTOG   (1<<9)  // Buzzer bit
#define SYSCON1_BZMOD   (1<<10) // Buzzer mode (0=direct,1=TC1)
#define SYSCON1_DBGEN   (1<<11) // Enable debug mode
#define SYSCON1_LCDEN   (1<<12) // Enable LCD
#define SYSCON1_CDENTX  (1<<13) // Enable Tx on CODEC
#define SYSCON1_CDENRX  (1<<14) // Enable Rx on CODEC
#define SYSCON1_SIREN   (1<<15) // Enable SIR protocol on UART #1
#define SYSCON1_ADCKSEL (3<<16) // Microwire clock
#define SYSCON1_ADC_CLOCK_4kHZ    (0<<16)
#define SYSCON1_ADC_CLOCK_16kHZ   (1<<16)
#define SYSCON1_ADC_CLOCK_64kHZ   (2<<16)
#define SYSCON1_ADC_CLOCK_128kHZ  (3<<16)
#define SYSCON1_EXCKEN  (1<<18) // External expansion clock enable
#define SYSCON1_WAKEDIS (1<<19) // Disable wakeup from snooze (do not disturb)
#define SYSCON1_IRTXM   (1<<20) // IrDA Tx mode

#define SYSCON2 0x80001100 // System control register #2
#define SYSCON2_SERSEL  (1<<0)  // 0=master/slave SSI, 1=CODEC
#define SYSCON2_KBD6    (1<<1)  // 0=8 bit keyboard, 1=6 bit
#define SYSCON2_DRAMSZ  (1<<2)  // DRAM width 0=32, 1=16
#define SYSCON2_KBWEN   (1<<3)  // 1=allow wake up from keyboard
#define SYSCON2_SS2TXEN (1<<4)  // Enable Tx on SS2
#define SYSCON2_PCMCIA1 (1<<5)  // Enable PCMCIA slot #1
#define SYSCON2_PCMCIA2 (1<<6)  // Enable PCMCIA slot #2
#define SYSCON2_SS2RXEN (1<<7)  // Enable Rx on SS2
#define SYSCON2_UART2EN (1<<8)  // Enable UART #2
#define SYSCON2_SS2MAEN (1<<9)  // Enable master mode on SS2
#define SYSCON2_SNZPOL  (1<<10) // Polarity of LCD during snooze
#define SYSCON2_LCDSNZE (1<<11)
#define SYSCON2_OSTB    (1<<12) // Operating system timing 0=512KHz, 1=500KHz
#define SYSCON2_CLKENSL (1<<13) // Source of run/clken signal 0=CLKEN, 1=RUN
#define SYSCON2_BUZFREQ (1<<14) // Buzzer frequency 0=timer, 1=fixed PLL/xtal

#define SYSCON3 0x80002200 // System control #3
#define SYSCON3_ADCCON    (1<<0) // ADC enable
#define SYSCON3_CLKCTL(n) (n<<1) // Processor block speed ((n+1)*18.432)MHz
#if defined(__EDB7209)
#define SYSCON3_I2SSEL    (1<<3) // Enable i2s instead of ssi#2
#define SYSCON3_FASTWAKE  (1<<8) // Determines wakeup in relationship
                                 //    to the 4 kHZ or 8 Hz clock
#define SYSCON3_DAIEN     (1<<9) // Enables the DAI
#endif
#if defined(__EDB7312) || defined(__EDB7209)
#define SYSCON3_ADCCKNSEN    (1<<4)  // Determines on which edge ADC
                                     // data is transmitted and read
                                     // in relationship to ADCCLK
#if defined(__EDB7312)
#define SYSCON3_DAISEL    (1<<3)  // Enable DAI instead of SSI
#define SYSCON3_128FS     (1<<9)  // Select DAI frame size
#define SYSCON3_ENPD67    (1<<10) // Configures Port D bits 6 and 7
#endif
#endif

#define SYSFLG1 0x80000140 // System flags #1
#define SYSFLG1_MCDR     (1<<0) // Media changed - direct read
#define SYSFLG1_DCDET    (1<<1) // 0=mains power, 1=battery
#define SYSFLG1_WUDR     (1<<2) // Wakeup direct
#define SYSFLG1_WUON     (1<<3) // Woken up
#define SYSFLG1_DID      (0xF<<4) // Display ID
#define SYSFLG1_CTS      (1<<8) // UART1 CTS
#define SYSFLG1_DSR      (1<<9) // UART1 DSR
#define SYSFLG1_DCD      (1<<10) // UART1 DCD
#define SYSFLG1_UBUSY1   (1<<11) // UART1 Tx busy
#define SYSFLG1_NBFLG    (1<<12) // New battery flag
#define SYSFLG1_RSTFLG   (1<<13) // Reset (button) flag
#define SYSFLG1_PFFLG    (1<<14) // Power fail flag
#define SYSFLG1_CLDFLG   (1<<15) // Cold start flag
#define SYSFLG1_RTCDIV   (0x3F<<16) // Real time clock divider (counter)
#define SYSFLG1_URXFE1   (1<<22) // UART1 Rx FIFO empty
#define SYSFLG1_UTXFF1   (1<<23) // UART1 Tx FIFO full
#define SYSFLG1_CRXFE    (1<<24) // CODEC Rx FIFO empty
#define SYSFLG1_CTXFF    (1<<25) // CODEC Tx FIFO full
#define SYSFLG1_SSIBUSY  (1<<26) // Synchronous serial interface busy
#define SYSFLG1_BOOTBITS (3<<27) // Boot mode (0=32,1=8,2=16,3=XX)
#define SYSFLG1_ID       (1<<29) // ID=1
#define SYSFLG1_VERID    (3<<30) // Board version

#define SYSFLG2 0x80001140 // System flags #2

#ifdef __EDB7312
#define SDCONF 0x80002300  // SDRAM configuration
#define SDRFOR 0x80002340  // SDRAM refresh
#endif

#define INTSR1  0x80000240 // Interrupt status register #1
#define INTSR1_EXTFIQ  (1<<0)  // External fast interrupt
#define INTSR1_BLINT   (1<<1)  // Battery low interrupt
#define INTSR1_WEINT   (1<<2)  // Watchdog expired interrupt
#define INTSR1_MCINT   (1<<3)  // Media changed interrupt
#define INTSR1_CSINT   (1<<4)  // CODEC sound interrupt
#define INTSR1_EINT1   (1<<5)  // External interrupt #1
#define INTSR1_EINT2   (1<<6)  // External interrupt #2
#define INTSR1_EINT3   (1<<7)  // External interrupt #3
#define INTSR1_TC1OI   (1<<8)  // Timer/counter #1 underflow
#define INTSR1_TC2OI   (1<<9)  // Timer/counter #2 underflow
#define INTSR1_RTCMI   (1<<10) // Real time clock match
#define INTSR1_TINT    (1<<11) // 64Hz tick
#define INTSR1_UTXINT1 (1<<12) // UART1 Tx interrupt
#define INTSR1_URXINT1 (1<<13) // UART1 Rx interrupt
#define INTSR1_UMSINT  (1<<14) // UART1 modem line change
#define INTSR1_SSEOTI  (1<<15) // Synchronous serial end of transfer
#define INTMR1  0x80000280 // Interrupt mask register #1

#define INTSR2  0x80001240 // Interrupt status #2
#define INTSR2_KBDINT  (1<<0)  // Keyboard interrupt
#define INTSR2_SS2RX   (1<<1)  // Synchronous serial #2 Rx
#define INTSR2_SS2TX   (1<<2)  // Synchronous serial #2 Tx
#define INTSR2_UTXINT2 (1<<12) // UART #2 Tx interrupt
#define INTSR2_URXINT2 (1<<13) // UART #2 Rx interrupt
#define INTMR2  0x80001280 // Interrupt mask #2

#define INTSR3  0x80002240 // Interrupt status #3
#if defined(__EDB7211)
#define INTSR3_MCPINT (1<<0) // MCP interrupt
#endif
#if defined(__EDB7209)
#define INTSR3_I2SINT (1<<0) // I2S interface interrupt
#endif
#if defined(__EDB7312)
#define INTSR3_DAIINT (1<<0) // DAI interface interrupt
#endif
#define INTMR3  0x80002280 // Interrupt mask #3

#define UARTDR1 0x80000480 // UART #1 data register
#define UARTDR2 0x80001480 // UART #2 data register
#define UBLCR1  0x800004C0 // UART #1 baud rate / line control
#define UBLCR2  0x800014C0 // UART #2 baud rate / line control
#define UBLCR_BRDV    0xFFF   // Baud rate
#define UBLCR_BREAK   (1<<12) // Generate break signal
#define UBLCR_PRTEN   (1<<13) // Enable parity
#define UBLCR_EVENPRT (1<<14) // 1=even parity, 0=odd
#define UBLCR_XSTOP   (1<<15) // 0=1 stop bit, 1=2 stop bits
#define UBLCR_FIFOEN  (1<<16) // Enable Tx/Rx FIFOs
#define UBLCR_WRDLEN  (3<<17) // Word (character) length field
#define UBLCR_WRDLEN5 (0<<17)
#define UBLCR_WRDLEN6 (1<<17)
#define UBLCR_WRDLEN7 (2<<17)
#define UBLCR_WRDLEN8 (3<<17)
#define UART_DIVISOR  230400
#define UART_BITRATE(baud) ((UART_DIVISOR/(baud))-1)
#if 90317 == CYGHWR_HAL_ARM_EDB7XXX_PROCESSOR_CLOCK
#undef UART_DIVISOR
// The speed enhancement is 22.5%
#define UART_DIVISOR  (230400 * 1225/1000)
#endif

#define MEMCFG1 0x80000180 // Memory configuration register #1
#define MEMCFG2 0x800001C0 // Memory configuration register #2

#define DRFPR   0x80000200 // DRAM refresh period

#define LCDCON                0x800002C0  // LDC control
#define LCDCON_BUFSIZ         0x00001FFF  // Video buffer size
#define LCDCON_BUFSIZ_S       0           // Position of buffer size
#define LCDCON_LINE_LENGTH    0x0007E000  // Line length
#define LCDCON_LINE_LENGTH_S  13          // Position of line length
#define LCDCON_PIX_PRESCALE   0x01F80000  // Pixel prescale value
#define LCDCON_PIX_PRESCALE_S 19          // Position of prescale value
#define LCDCON_AC_PRESCALE    0x3E000000  // LCD AC bias frequency
#define LCDCON_AC_PRESCALE_S  25          // Position to AC bias
#define LCDCON_GSEN           0x40000000  // Enable greyscale
#define LCDCON_GSMD           0x80000000  // Greyscale mode
                                          // 00 - 1bpp, 01 - 2bpp, 11 - 4 bpp

#define TC1D    0x80000300 // Timer/Counter #1 register
#define TC2D    0x80000340 // Timer/Counter #2 register
#define RTCDR   0x80000380 // Real time clock data register
#define RTCMR   0x800003C0 // Real time clock match register

#define PMPCON  0x80000400 // DC-DC pump control

#define CODR    0x80000440 // CODEC data register

#define SYNCIO  0x80000500 // Synchronous I/O data register

#define PALLSW  0x80000540 // LCD palette - LSW (Pixel values 0..7)
#define PALMSW  0x80000580 // LCD palette - MSW (Pixel values 8..15)

#define STFCLR  0x800005C0 // Clear startup reason flags
#define BLEOI   0x80000600 // Clear battery low interrupt
#define MCEOI   0x80000640 // Clear media changed interrupt
#define TEOI    0x80000680 // Clear tick/watchdog interrupt
#define TC1EOI  0x800006C0 // Clear timer/counter #1 interrupt
#define TC2EOI  0x80000700 // Clear timer/counter #2 interrupt
#define RTCEOI  0x80000740 // Clear real time clock interrupt
#define UMSEOI  0x80000780 // Clear UART modem status change interrupt
#define COEOI   0x800007C0 // Clear CODEC sound interrupt

#define HALT    0x80000800 // Enter 'idle' state
#define STDBY   0x80000840 // Enter 'standby' state

#define FRBADDR 0x80001000 // LCD frame buffer start

#define SNZDISP 0x800012C0 // Snooze display size

#define SS2DR   0x80001500 // Master/slave SSI2 register

#define SRXEOF  0x80001600 // Clear Rx FIFO overflow flag
#define SS2POP  0x800016C0 // Pop SSI2 residual byte into FIFO

#define KBDEOI  0x80001700 // Clear keyboard interrupt

#define SNOOZE  0x80001800 // Enter 'snooze' state

#if defined(__EDB7211)
#define MCCR    0x80002000 // MCP control register
#define MCDR0   0x80002040 // MCP data register #0
#define MCDR1   0x80002080 // MCP data register #1
#define MCDR2   0x800020C0 // MCP data register #2
#define MCSR    0x80002100 // MCP status register
#endif

#if defined(__EDB7209) || \
defined(__EDB7312) || \
1

#define I2S_CTL         0x80002000 // I2S (Audio interface) control
#define I2S_CTL_FLAG    0x0404     // Magic
#define I2S_CTL_EN      (1<<16)    // Enable interface
#define I2S_CTL_ECS     (1<<17)    // External clock select
#define I2S_CTL_LCTM    (1<<19)    // Left channel transmit interrupt
#define I2S_CTL_LCRM    (1<<20)    // Left channel receive interrupt
#define I2S_CTL_RCTM    (1<<21)    // Right channel transmit interrupt
#define I2S_CTL_RCRM    (1<<22)    // Right channel receive interrupt
#if defined(__EDB7209)
#define I2S_CTL_LBM     (1<<23)    // Loop-back mode
#endif
#define I2S_RIGHT_FIFO  0x80002040 // Right channel FIFO access
#define I2S_LEFT_FIFO   0x80002080 // Left channel FIFO access
#define I2S_FIFO_CTL    0x800020C0 // FIFO control
#define I2S_FIFO_CTL_RIGHT_ENABLE  0x00118000
#define I2S_FIFO_CTL_RIGHT_DISABLE 0x00110000
#define I2S_FIFO_CTL_LEFT_ENABLE   0x000D8000
#define I2S_FIFO_CTL_LEFT_DISABLE  0x000D0000
#define I2S_STAT        0x80002100 // I2S interface status
#define I2S_STAT_RCTSR  (1<<0)     // Right channel transmit service request
#define I2S_STAT_RCRSR  (1<<1)     // Right channel receive service request
#define I2S_STAT_LCTSR  (1<<2)     // Left channel transmit service request
#define I2S_STAT_LCRSR  (1<<3)     // Left channel receive service request
#define I2S_STAT_RCTUR  (1<<4)     // Right channel transmit FIFO underrun
#define I2S_STAT_RCROR  (1<<5)     // Right channel receive FIFO overrun
#define I2S_STAT_LCTUR  (1<<6)     // Left channel transmit FIFO underrun
#define I2S_STAT_LCROR  (1<<7)     // Left channel receive FIFO overrun
#define I2S_STAT_RCTNF  (1<<8)     // Right channel transmit FIFO not full
#define I2S_STAT_RCRNE  (1<<9)     // Right channel receive FIFO not empty
#define I2S_STAT_LCTNF  (1<<10)    // Left channel transmit FIFO not full
#define I2S_STAT_LCRNE  (1<<11)    // Left channel receive FIFO not empty
#define I2S_STAT_FIFO   (1<<12)    // A FIFO operation has completed
#endif

#ifdef __EDB7312
// If DAI_ headers wishes to be used instead (for consistency)
#define DAI_CTL       I2S_CTL
#define DAI_CTL_FLAG  I2S_CTL_FLAG
#define DAI_CTL_EN    I2S_CTL_EN
#define DAI_CTL_ECS   I2S_CTL_ECS
#define DAI_CTL_LCTM  I2S_CTL_LCTM
#define DAI_CTL_LCRM  I2S_CTL_LCRM
#define DAI_CTL_RCTM  I2S_CTL_RCTM
#define DAI_CTL_RCRM  I2S_CTL_RCRM

#define DAI_RIGHT_FIFO  I2S_RIGHT_FIFO
#define DAI_LEFT_FIFO   I2S_LEFT_FIFO

#define DAI_FIFO_CTL                I2S_FIFO_CTL
#define DAI_FIFO_CTL_RIGHT_ENABLE   I2S_FIFO_CTL_RIGHT_ENABLE
#define DAI_FIFO_CTL_RIGHT_DISABLE  I2S_FIFO_CTL_RIGHT_DISABLE
#define DAI_FIFO_CTL_LEFT_ENABLE    I2S_FIFO_CTL_LEFT_ENABLE
#define DAI_FIFO_CTL_LEFT_DISABLE   I2S_FIFO_CTL_LEFT_DISABLE

#define DAI_STAT        I2S_STAT
#define DAI_STAT_RCTSR  I2S_STAT_RCTSR
#define DAI_STAT_RCRSR  I2S_STAT_RCRSR
#define DAI_STAT_LCTSR  I2S_STAT_LCTSR
#define DAI_STAT_LCRSR  I2S_STAT_LCRSR
#define DAI_STAT_RCTUR  I2S_STAT_RCTUR
#define DAI_STAT_RCROR  I2S_STAT_RCROR
#define DAI_STAT_LCTUR  I2S_STAT_LCTUR
#define DAI_STAT_LCROR  I2S_STAT_LCROR
#define DAI_STAT_RCTNF  I2S_STAT_RCTNF
#define DAI_STAT_RCRNE  I2S_STAT_RCRNE
#define DAI_STAT_LCTNF  I2S_STAT_LCTNF
#define DAI_STAT_LCRNE  I2S_STAT_LCRNE
#define DAI_STAT_FIFO   I2S_STAT_FIFO

// Additional 7312 register
#define DAI_MODE        0x80002600  // I2S mode control register
#define DAI_MODE_I2SF64  (1<<0)     // Frame size
#define DAI_MODE_CLKEN   (1<<1)     // Enable audio clock generator
#define DAI_MODE_CLKSRC  (1<<2)     // Select audio clock source
#define DAI_MODE_MCLK    (1<<3)     // Enables MCLK (BUZ) (256 frame size)
#define DAI_MODE_LBM     (1<<5)     // Loopback mode

#define DAI_MODE_AUDDIV_MASK  0x7F00  // Mask for the frequency divisor
                                      //    for the sample frequency and
                                      //    bit clock

#define SYSCON3_I2SSEL SYSCON3_DAISEL  // Backward compatiblity
                                       //    for sample i2s_audio_fiq.s

#define INTSR3_I2SINT INTSR3_DAIINT  // Backward compatiblity
                                     //    for sample i2s_audio_fiq.s


// Further additional 7312 register for special 90MHz variant:
// PLL_Multiplier_Register 0x80002610  - the location of the PLL multiplier register
// Value_For_90_MHz_Operation 0x31000000
#define EP7312_PLL_MR            0x80002610
#define EP7312_PLL_MR_FOR_90MHz  0x31000000

#endif

#define LEDFLSH 0x800022C0 // LED flash control
#define LEDFLSH_ENABLE      (1<<6)      // LED enabled
#define LEDFLSH_DUTY(n)     ((n-1)<<2)  // LED on ratio
#define LEDFLSH_PERIOD(n)   (n-1)       // LED active time (1..4)

#define KBD_PORT 0x30010000 // Extra 8 bits of keyboard data

/*---------------------------------------------------------------------------*/
/* end of hal_edb7xxx.h                                                         */
#endif /* CYGONCE_HAL_EDB7XXX_H */
