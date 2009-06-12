//==========================================================================
//
//      ks32c5000.h
//
//      
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

/*
 * structure definitions for Samsung KS32C5000 peripheral registers
 */

typedef volatile unsigned int reg;

#define Bit(n) (1<<(n))


/*
 * Interrupt Controller
 */

typedef struct
{
  reg intmod;          // Interrupt mode register
  reg intpnd;          // Interrupt pending register
  reg intmsk;          // Interrupt mask register
  reg intpri0;         // Interrupt priority register
  reg intpri1;
  reg intpri2;
  reg intpri3;
  reg intpri4;
  reg intpri5;
  reg intoffset;       // Interrupt offset register
} tInterruptController;


/* The following bit masks are for use in the intmod, intpnd, and
 * intmsk registers
 */

#define IntMaskExt0     Bit(0)
#define IntMaskExt1     Bit(1)
#define IntMaskExt2     Bit(2)
#define IntMaskExt3     Bit(3)
#define IntMaskUart0Tx  Bit(4)
#define IntMaskUart0Rx  Bit(5)
#if defined(CYG_HAL_CPUTYPE_KS32C5000A)
#define IntMaskUart0Err Bit(6)
#define IntMaskUart1Tx  Bit(7)
#define IntMaskUart1Rx  Bit(8)
#define IntMaskUart1Err Bit(9)
#define IntMaskDma0     Bit(10)
#define IntMaskDma1     Bit(11)
#define IntMaskTimer0   Bit(12)
#define IntMaskTimer1   Bit(13)
#define IntMaskHDLCA    Bit(14)
#define IntMaskHDLCB    Bit(15)
#else
#define IntMaskUart1Tx  Bit(6)
#define IntMaskUart1Rx  Bit(7)
#define IntMaskDma0     Bit(8)
#define IntMaskDma1     Bit(9)
#define IntMaskTimer0   Bit(10)
#define IntMaskTimer1   Bit(11)
#define IntMaskHDLCATx  Bit(12)
#define IntMaskHDLCARx  Bit(13)
#define IntMaskHDLCBTx  Bit(14)
#define IntMaskHDLCBRx  Bit(15)
#endif
#define IntMaskEtherBDMATx  Bit(16)
#define IntMaskEtherBDMARx  Bit(17)
#define IntMaskEtherMacTx   Bit(18)
#define IntMaskEtherMacRx   Bit(19)
#define IntMaskI2C      Bit(20)
#define IntMaskGlobal   Bit(21)

/*
 * Timers
 */

typedef struct
{
  reg tmod;      // Timer mode
  reg tdata0;    // Timer 0 Data
  reg tdata1;    // Timer 1 Data
  reg tcnt0;     // Timer 0 Count
  reg tcnt1;     // Timer 1 Count
}tTimers;

#define TimerModeEnable0  Bit(0)
#define TimerModeToggle0  Bit(1)
#define TimerModeInitOut0 Bit(2)
#define TimerModeEnable1  Bit(3)
#define TimerModeToggle1  Bit(4)
#define TimerModeInitOut1 Bit(5)


/*
 * UART
 */

typedef struct
{
  reg ulcon;   // UART Line Control
  reg ucon;    // UART Control
  reg ustat;   // UART Status
  reg utxbuf;  // UART Tx Buffer
  reg urxbuf;  // UART Rx Buffer
  reg brdiv;   // UART Baud Rate Divisor
  reg brdcnt;  // UART Baud Rate Counter
  reg brdclk;  // UART Baud Rate Clock
}tUart;

/* UART Line Control */

#define UartLineWordLenMask 0x03
#define UartLineWordLen5       0
#define UartLineWordLen6       1
#define UartLineWordLen7       2
#define UartLineWordLen8       3

#define UartLineStopMask    (1 << 2)
#define UartLineStop1       (0 << 2)
#define UartLineStop2       (1 << 2)

#define UartLineParityMask  (0x07 << 3)
#define UartLineParityNone   (0 << 3)
#define UartLineParityOdd    (4 << 3)
#define UartLineParityEven   (5 << 3)
#define UartLineParityMark   (6 << 3)
#define UartLineParitySpace  (7 << 3)

#define UartLineInfraRedEnable Bit(7)


/* UART Control */


#define UartControlRxModeMask      3
#define UartControlRxModeDisabled  0
#define UartControlRxModeInterrupt 1
#define UartControlRxModeDma0      2
#define UartControlRxModeDma1      3

#define UartControlRxStatusIntEnable Bit(2)

#define UartControlTxModeMask      (0x3 <<3)
#define UartControlTxModeDisable   (0 <<3)
#define UartControlTxModeInterrupt (1 <<3)
#define UartControlTxModeDma0      (2 <<3)
#define UartControlTxModeDma1      (3 <<3)

#define UartControlDSR             Bit(5)

#define UartControlSendBreak       Bit(6)

#define UartControlLoopback        Bit(7)


/* UART Status */

#define UartStatusRxOverrunError  Bit(0)
#define UartStatusRxParityError   Bit(1)
#define UartStatusRxFrameError    Bit(2)
#define UartStatusRxBreak         Bit(3)
#define UartStatusDTR             Bit(4)
#define UartStatusRxDataAvail     Bit(5)
#define UartStatusTxBufEmpty      Bit(6)
#define UartStatusTxDone          Bit(7)




    
// macros for external timing control registers

#define Tcos0(n) (((n)&7)<<0)
#define Tacs0(n) (((n)&7)<<3)
#define Tcoh0(n) (((n)&7)<<6)
#define Tacc0(n) (((n)&7)<<9)

#define Tcos1(n) (((n)&7)<<16)
#define Tacs1(n) (((n)&7)<<19)
#define Tcoh1(n) (((n)&7)<<22)
#define Tacc1(n) (((n)&7)<<25)
    
#define Tcos2(n) (((n)&7)<<0)
#define Tacs2(n) (((n)&7)<<3)
#define Tcoh2(n) (((n)&7)<<6)
#define Tacc2(n) (((n)&7)<<9)

#define Tcos3(n) (((n)&7)<<16)
#define Tacs3(n) (((n)&7)<<19)
#define Tcoh3(n) (((n)&7)<<22)
#define Tacc3(n) (((n)&7)<<25)
