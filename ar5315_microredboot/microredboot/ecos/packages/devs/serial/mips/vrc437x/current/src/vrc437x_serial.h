#ifndef CYGONCE_MIPS_VRC437X_SERIAL_H
#define CYGONCE_MIPS_VRC437X_SERIAL_H

// ====================================================================
//
//      vrc437x_serial.h
//
//      Device I/O - Description of Mips VRC437X serial hardware
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           gthomas
// Contributors:        gthomas
// Date:        1999-04-15
// Purpose:     Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on Mips VRC437X
//   Based on Zilog 85C30 SCC

struct serial_port {
    unsigned char _byte[16];
};

#define scc_ctl _byte[0]
#define scc_dat _byte[8]

#define R0	0		/* Register selects */
#define R1	1
#define R2	2
#define R3	3
#define R4	4
#define R5	5
#define R6	6
#define R7	7
#define R8	8
#define R9	9
#define R10	10
#define R11	11
#define R12	12
#define R13	13
#define R14	14
#define R15	15

/* Write Register 0 */
#define WR0_NullCode	0x00	/* Null Code */
#define WR0_PointHigh	0x08	/* Select upper half of registers */
#define WR0_ResExtInt	0x10	/* Reset Ext. Status Interrupts */
#define WR0_SendAbort	0x18	/* HDLC Abort */
#define WR0_ResRxIntFC	0x20	/* Reset RxINT on First Character */
#define WR0_ResTxP	0x28	/* Reset TxINT Pending */
#define WR0_ErrReset	0x30	/* Error Reset */
#define WR0_ResHiIUS	0x38	/* Reset highest IUS */

#define WR0_ResRxCRC	0x40	/* Reset Rx CRC Checker */
#define WR0_ResTxCRC	0x80	/* Reset Tx CRC Checker */
#define WR0_ResEOMlatch	0xC0	/* Reset EOM latch */

/* Write Register 1 */

#define WR1_ExtIntEnab	0x01	/* Ext Int Enable */
#define WR1_TxIntEnab	0x02	/* Tx Int Enable */
#define WR1_ParSpec	0x04	/* Parity is special condition */

#define WR1_RxIntDisab	0x00	/* Rx Int Disable */
#define WR1_RxIntFCE	0x08	/* Rx Int on First Character Only or Error */
#define WR1_IntAllRx	0x10	/* Int on all Rx Characters or error */
#define WR1_IntErrRx	0x18	/* Int on error only */

#define WR1_WtRdyRT	0x20	/* Wait/Ready on R/T */
#define WR1_WtFnRdyFn	0x40	/* Wait/FN/Ready FN */
#define WR1_WtRdyEnab	0x80	/* Wait/Ready Enable */

/* Write Register #2 (Interrupt Vector) */

/* Write Register 3 */

#define WR3_RxEnable	0x01	/* Rx Enable */
#define WR3_SyncInhibit	0x02	/* Sync Character Load Inhibit */
#define WR3_AddrSearch	0x04	/* Address Search Mode (SDLC) */
#define WR3_RxCRC_ENAB	0x08	/* Rx CRC Enable */
#define WR3_EntHuntMode	0x10	/* Enter Hunt Mode */
#define WR3_AutoEnab	0x20	/* Auto Enables */
#define WR3_Rx5		0x00	/* Rx 5 Bits/Character */
#define WR3_Rx7		0x40	/* Rx 7 Bits/Character */
#define WR3_Rx6		0x80	/* Rx 6 Bits/Character */
#define WR3_Rx8		0xc0	/* Rx 8 Bits/Character */
#define WR3_RxNbitsMask	0xc0

/* Write Register 4 */

#define WR4_ParityEn	0x01	/* Parity Enable */
#define WR4_ParityEven	0x02	/* Parity Even/Odd* */

#define WR4_SyncEnable	0x00	/* Sync Modes Enable */
#define WR4_SB1		0x04	/* 1 stop bit/char */
#define WR4_SB15	0x08	/* 1.5 stop bits/char */
#define WR4_SB2		0x0c	/* 2 stop bits/char */
#define WR4_SB_MASK	0x0c

#define WR4_Monsync	0x00	/* 8 Bit Sync character */
#define WR4_Bisync	0x10	/* 16 bit sync character */
#define WR4_SDLC	0x20	/* SDLC Mode (01111110 Sync Flag) */
#define WR4_EXtSync	0x30	/* External Sync Mode */

#define WR4_X1CLK	0x00	/* x1 clock mode */
#define WR4_X16CLK	0x40	/* x16 clock mode */
#define WR4_X32CLK	0x80	/* x32 clock mode */
#define WR4_X64CLK	0xC0	/* x64 clock mode */
#define WR4_XCLK_MASK	0xC0

/* Write Register 5 */

#define WR5_TxCRCEnab	0x01	/* Tx CRC Enable */
#define WR5_RTS		0x02	/* RTS */
#define WR5_SDLC_CRC	0x04	/* SDLC/CRC-16 */
#define WR5_TxEnable	0x08	/* Tx Enable */
#define WR5_SendBreak	0x10	/* Send Break */
#define WR5_Tx5		0x00	/* Tx 5 bits (or less)/character */
#define WR5_Tx7		0x20	/* Tx 7 bits/character */
#define WR5_Tx6		0x40	/* Tx 6 bits/character */
#define WR5_Tx8		0x60	/* Tx 8 bits/character */
#define WR5_TxNbitsMask 0x60
#define WR5_DTR		0x80	/* DTR */

/* Write Register 6 (Sync bits 0-7/SDLC Address Field) */

/* Write Register 7 (Sync bits 8-15/SDLC 01111110) */

/* Write Register 8 (transmit buffer) */

/* Write Register 9 (Master interrupt control) */
#define WR9_VIS	        0x01	/* Vector Includes Status */
#define WR9_NoVector	0x02	/* No Vector */
#define WR9_DLC	        0x04	/* Disable Lower Chain */
#define WR9_MIE	        0x08	/* Master Interrupt Enable */
#define WR9_StatHi	0x10	/* Status high */
#define WR9_NoReset	0x00	/* No reset on write to R9 */
#define WR9_ResetB	0x40	/* Reset channel B */
#define WR9_ResetA	0x80	/* Reset channel A */
#define WR9_HwReset	0xc0	/* Force hardware reset */

/* Write Register 10 (misc control bits) */
#define WR10_Bit6	0x01	/* 6 bit/8bit sync */
#define WR10_LoopMode   0x02	/* SDLC Loop mode */
#define WR10_AbrtUnder	0x04	/* Abort/flag on SDLC xmit underrun */
#define WR10_MarkIdle   0x08	/* Mark/flag on idle */
#define WR10_GAOP	0x10	/* Go active on poll */
#define WR10_NRZ	0x00	/* NRZ mode */
#define WR10_NRZI	0x20	/* NRZI mode */
#define WR10_FM1	0x40	/* FM1 (transition = 1) */
#define WR10_FM0	0x60	/* FM0 (transition = 0) */
#define WR10_CRCPS	0x80	/* CRC Preset I/O */

/* Write Register 11 (Clock Mode control) */
#define WR11_TRxCXT	0x00	/* TRxC = Xtal output */
#define WR11_TRxCTC	0x01	/* TRxC = Transmit clock */
#define WR11_TRxCBR	0x02	/* TRxC = BR Generator Output */
#define WR11_TRxCDP	0x03	/* TRxC = DPLL output */
#define WR11_TRxCOI	0x04	/* TRxC O/I */
#define WR11_TxCRTxCP	0x00	/* Transmit clock = RTxC pin */
#define WR11_TxCTRxCP	0x08	/* Transmit clock = TRxC pin */
#define WR11_TxCBR	0x10	/* Transmit clock = BR Generator output */
#define WR11_TxCDPLL	0x18	/* Transmit clock = DPLL output */
#define WR11_RxCRTxCP	0x00	/* Receive clock = RTxC pin */
#define WR11_RxCTRxCP	0x20	/* Receive clock = TRxC pin */
#define WR11_RxCBR	0x40	/* Receive clock = BR Generator output */
#define WR11_RxCDPLL	0x60	/* Receive clock = DPLL output */
#define WR11_RTxCX	0x80	/* RTxC Xtal/No Xtal */

/* Write Register 12 (lower byte of baud rate generator time constant) */

/* Write Register 13 (upper byte of baud rate generator time constant) */

/* Write Register 14 (Misc control bits) */
#define WR14_BRenable	0x01	/* Baud rate generator enable */
#define WR14_BRSRC	0x02	/* Baud rate generator source */
#define WR14_DTRreq	0x04	/* DTR/Request function */
#define WR14_AutoEcho   0x08	/* Auto Echo */
#define WR14_LoopBack	0x10	/* Local loopback */
#define WR14_Search	0x20	/* Enter search mode */
#define WR14_RMC	0x40	/* Reset missing clock */
#define WR14_NoDPLL	0x60	/* Disable DPLL */
#define WR14_SSBR	0x80	/* Set DPLL source = BR generator */
#define WR14_SSRTxC	0xa0	/* Set DPLL source = RTxC */
#define WR14_SFMM	0xc0	/* Set FM mode */
#define WR14_SNRZI	0xe0	/* Set NRZI mode */

/* Write Register 15 (external/status interrupt control) */
#define WR15_ZCIE	0x02	/* Zero count IE */
#define WR15_DCDIE	0x08	/* DCD IE */
#define WR15_SYNCIE	0x10	/* Sync/hunt IE */
#define WR15_CTSIE	0x20	/* CTS IE */
#define WR15_TxUIE	0x40	/* Tx Underrun/EOM IE */
#define WR15_BRKIE	0x80	/* Break/Abort IE */

/* Read Register 0 */
#define RR0_RxAvail	0x01	/* Rx Character Available */
#define RR0_Zcount	0x02	/* Zero count */
#define RR0_TxEmpty	0x04	/* Tx Buffer empty */
#define RR0_DCD		0x08	/* DCD */
#define RR0_SyncHunt	0x10	/* Sync/hunt */
#define RR0_CTS		0x20	/* CTS */
#define RR0_TxEOM	0x40	/* Tx underrun */
#define RR0_BrkAbort	0x80	/* Break/Abort */

/* Read Register 1 */
#define RR1_AllSent	0x01	/* All sent */
/* Residue Data for 8 Rx bits/char programmed */
#define RR1_RES3	0x08	/* 0/3 */
#define RR1_RES4	0x04	/* 0/4 */
#define RR1_RES5	0x0c	/* 0/5 */
#define RR1_RES6	0x02	/* 0/6 */
#define RR1_RES7	0x0a	/* 0/7 */
#define RR1_RES8	0x06	/* 0/8 */
#define RR1_RES18	0x0e	/* 1/8 */
#define RR1_RES28	0x00	/* 2/8 */
/* Special Rx Condition Interrupts */
#define RR1_PariryError	0x10	/* Parity error */
#define RR1_RxOverrun	0x20	/* Rx Overrun Error */
#define RR1_FrameError	0x40	/* CRC/Framing Error */
#define RR1_EndOfFrame	0x80	/* End of Frame (SDLC) */

/* Read Register 2 (channel b only) - Interrupt vector */

/* Read Register 3 (interrupt pending register) ch a only */
#define RR3_BExt	0x01		/* Channel B Ext/Stat IP */
#define RR3_BTxIP	0x02		/* Channel B Tx IP */
#define RR3_BRxIP	0x04		/* Channel B Rx IP */
#define RR3_AExt	0x08		/* Channel A Ext/Stat IP */
#define RR3_ATxIP	0x10		/* Channel A Tx IP */
#define RR3_ARxIP	0x20		/* Channel A Rx IP */

/* Read Register 8 (receive data register) */

/* Read Register 10  (misc status bits) */
#define RR10_OnLoop	0x02		/* On loop */
#define RR10_LoopSend   0x10		/* Loop sending */
#define RR10_Clk2Mis	0x40		/* Two clocks missing */
#define RR10_Clk1Mis	0x80		/* One clock missing */

/* Read Register 12 (lower byte of baud rate generator constant) */

/* Read Register 13 (upper byte of baud rate generator constant) */

/* Read Register 15 (value of WR 15) */

#define BRTC(brate) (( ((unsigned) DUART_CLOCK) / (2*(brate)*SCC_CLKMODE_TC)) - 2)
#define DUART_CLOCK          4915200         /* Z8530 duart */
#define SCC_CLKMODE_TC       16              /* Always run x16 clock for async modes */

static unsigned char select_word_length_WR3[] = {
    WR3_Rx5,            // 5 bits / word (char)
    WR3_Rx6,
    WR3_Rx7,
    WR3_Rx8
};

static unsigned char select_word_length_WR5[] = {
    WR5_Tx5,            // 5 bits / word (char)
    WR5_Tx6,
    WR5_Tx7,
    WR5_Tx8
};

static unsigned char select_stop_bits[] = {
    0,
    WR4_SB1,    // 1 stop bit
    WR4_SB15,   // 1.5 stop bit
    WR4_SB2     // 2 stop bits
};

static unsigned char select_parity[] = {
    0,                                 // No parity
    WR4_ParityEn | WR4_ParityEven,     // Even parity
    WR4_ParityEn,                      // Odd parity
    0xFF,                              // Mark parity
    0xFF,                              // Space parity
};

static cyg_int32 select_baud[] = {
    0,      // Unused
    50,     // 50
    75,     // 75
    110,    // 110
    0,      // 134.5
    150,    // 150
    200,    // 200
    300,    // 300
    600,    // 600
    1200,   // 1200
    1800,   // 1800
    2400,   // 2400
    3600,   // 3600
    4800,   // 4800
    7200,   // 7200
    9600,   // 9600
    14400,  // 14400
    19200,  // 19200
    38400,  // 38400
    0,      // 57600
    0,      // 115200
    0,      // 230400
};

#endif // CYGONCE_MIPS_VRC437X_SERIAL_H
