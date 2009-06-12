//=============================================================================
//
//      mod_regs_ser.h
//
//      SCI, SCIF, and IRDA (serial) Module register definitions
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2000-10-30
// Note:        All three serial module definitions kept in the same file
//              since they share some of the information.
//####DESCRIPTIONEND####
//
//=============================================================================

//++++++ Module SCI ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//--------------------------------------------------------------------------
// Serial registers. All 8 bit registers.
#define CYGARC_REG_SCI_SCSMR                0xfffffe80 // serial mode register
#define CYGARC_REG_SCI_SCBRR                0xfffffe82 // bit rate register
#define CYGARC_REG_SCI_SCSCR                0xfffffe84 // serial control register
#define CYGARC_REG_SCI_SCTDR                0xfffffe86 // transmit data register
#define CYGARC_REG_SCI_SCSSR                0xfffffe88 // serial status register
#define CYGARC_REG_SCI_SCRDR                0xfffffe8a // receive data register

// Serial Mode Register
#define CYGARC_REG_SCI_SCSMR_CA             0x80 // communication mode
#define CYGARC_REG_SCI_SCSMR_CHR            0x40 // character length (7 if set)
#define CYGARC_REG_SCI_SCSMR_PE             0x20 // parity enable
#define CYGARC_REG_SCI_SCSMR_OE             0x10 // parity mode
#define CYGARC_REG_SCI_SCSMR_STOP           0x08 // stop bit length
#define CYGARC_REG_SCI_SCSMR_MP             0x04 // multiprocessor mode
#define CYGARC_REG_SCI_SCSMR_CKS1           0x02 // clock select 1
#define CYGARC_REG_SCI_SCSMR_CKS0           0x01 // clock select 0
#define CYGARC_REG_SCI_SCSMR_CKSx_MASK      0x03 // mask

// Serial Control Register
#define CYGARC_REG_SCI_SCSCR_TIE            0x80 // transmit interrupt enable
#define CYGARC_REG_SCI_SCSCR_RIE            0x40 // receive interrupt enable
#define CYGARC_REG_SCI_SCSCR_TE             0x20 // transmit enable
#define CYGARC_REG_SCI_SCSCR_RE             0x10 // receive enable
#define CYGARC_REG_SCI_SCSCR_MPIE           0x08 // multiprocessor interrupt enable
#define CYGARC_REG_SCI_SCSCR_TEIE           0x04 // transmit-end interrupt enable
#define CYGARC_REG_SCI_SCSCR_CKE1           0x02 // clock enable 1
#define CYGARC_REG_SCI_SCSCR_CKE0           0x01 // clock enable 0

// Serial Status Register
#define CYGARC_REG_SCI_SCSSR_TDRE           0x80 // transmit data register empty
#define CYGARC_REG_SCI_SCSSR_RDRF           0x40 // receive data register full
#define CYGARC_REG_SCI_SCSSR_ORER           0x20 // overrun error
#define CYGARC_REG_SCI_SCSSR_FER            0x10 // framing error
#define CYGARC_REG_SCI_SCSSR_PER            0x08 // parity error
#define CYGARC_REG_SCI_SCSSR_TEND           0x04 // transmit end
#define CYGARC_REG_SCI_SCSSR_MPB            0x02 // multiprocessor bit
#define CYGARC_REG_SCI_SCSSR_MPBT           0x01 // multiprocessor bit transfer

// When clearing the status register, always write the value:
// CYGARC_REG_SCSSR_CLEARMASK & ~bit
// to prevent other bits than the one of interest to be cleared.
#define CYGARC_REG_SCI_SCSSR_CLEARMASK      0xf8


#if (CYGARC_SH_MOD_SCI >= 2)
# define CYGARC_REG_SCI_SCSPTR               0xfffffe7c // serial port register
#endif


// Baud rate values calculation, depending on peripheral clock (Pf)
// n is CKS setting (0-3)
// N = (Pf/(64*2^(2n-1)*B))-1
// With CYGARC_SCBRR_CKSx providing the values 1, 4, 16, 64 we get
//       N = (Pf/(32*_CKS*B))-1
//
// The CYGARC_SCBRR_OPTIMAL_CKS macro should compute the minimal CKS
// setting for the given baud rate and peripheral clock.
//
// The error of the CKS+count value can be computed by:
//  E(%) = ((Pf/((N+1)*B*(64^(n-1)))-1)*100 
//
#define CYGARC_SCBRR_PRESCALE(_b_) \
((((CYGHWR_HAL_SH_ONCHIP_PERIPHERAL_SPEED/32/1/(_b_))-1)<256) ? 1 : \
 (((CYGHWR_HAL_SH_ONCHIP_PERIPHERAL_SPEED/32/4/(_b_))-1)<256) ? 4 : \
 (((CYGHWR_HAL_SH_ONCHIP_PERIPHERAL_SPEED/32/16/(_b_))-1)<256) ? 16 : 64)

// Add half the divisor to reduce rounding errors to .5
#define CYGARC_SCBRR_ROUNDING(_b_) \
  16*CYGARC_SCBRR_PRESCALE(_b_)*(_b_)

// These two macros provide the static values we need to stuff into the
// registers.
#define CYGARC_SCBRR_CKSx(_b_) \
    ((1 == CYGARC_SCBRR_PRESCALE(_b_)) ? 0 : \
     (4 == CYGARC_SCBRR_PRESCALE(_b_)) ? 1 : \
     (16 == CYGARC_SCBRR_PRESCALE(_b_)) ? 2 : 3)
#define CYGARC_SCBRR_N(_b_)     \
    (((_b_) < 4800) ? 0 :       \
      ((_b_) > 115200) ? 0 :    \
       (((CYGHWR_HAL_SH_ONCHIP_PERIPHERAL_SPEED+CYGARC_SCBRR_ROUNDING(_b_))/32/CYGARC_SCBRR_PRESCALE(_b_)/(_b_))-1))


//++++++ Module SCIF +++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef CYGARC_SH_MOD_SCIF

//--------------------------------------------------------------------------
// Serial w FIFO registers (and IRDA)

// SCIF1
#define CYGARC_REG_SCIF_SCSMR1               0xa4000140 // Serial mode          
#define CYGARC_REG_SCIF_SCBRR1               0xa4000142 // Bit rate             
#define CYGARC_REG_SCIF_SCSCR1               0xa4000144 // Serial control       
#define CYGARC_REG_SCIF_SCFTDR1              0xa4000146 // Transmit FIFO data   
#define CYGARC_REG_SCIF_SCSSR1               0xa4000148 // Serial status        
#define CYGARC_REG_SCIF_SCFRDR1              0xa400014a // Receive data FIFO    
#define CYGARC_REG_SCIF_SCFCR1               0xa400014c // FIFO control register
#define CYGARC_REG_SCIF_SCFDR1               0xa400014e // FIFO data count set

// SCIF2
#define CYGARC_REG_SCIF_SCSMR2               0xa4000150 // Serial mode
#define CYGARC_REG_SCIF_SCBRR2               0xa4000152 // Bit rate
#define CYGARC_REG_SCIF_SCSCR2               0xa4000154 // Serial control
#define CYGARC_REG_SCIF_SCFTDR2              0xa4000156 // Transmit FIFO data
#define CYGARC_REG_SCIF_SCSSR2               0xa4000158 // Serial status
#define CYGARC_REG_SCIF_SCFRDR2              0xa400015a // Receive data FIFO
#define CYGARC_REG_SCIF_SCFCR2               0xa400015c // FIFO control register
#define CYGARC_REG_SCIF_SCFDR2               0xa400015e // FIFO data count set


// Serial Mode Register - normal mode
#define CYGARC_REG_SCIF_SCSMR_CHR             0x40 // character length (7 if set)
#define CYGARC_REG_SCIF_SCSMR_PE              0x20 // parity enable
#define CYGARC_REG_SCIF_SCSMR_OE              0x10 // parity mode
#define CYGARC_REG_SCIF_SCSMR_STOP            0x08 // stop bit length
#define CYGARC_REG_SCIF_SCSMR_CKS1            0x02 // clock select 1
#define CYGARC_REG_SCIF_SCSMR_CKS0            0x01 // clock select 0
#define CYGARC_REG_SCIF_SCSMR_CKSx_MASK       0x03 // mask

// Serial Mode Register - IrDA mode alternative definitions
#define CYGARC_REG_SCIF_SCSMR_IRMOD           0x80 // IrDA Mode
#define CYGARC_REG_SCIF_SCSMR_ICK_MASK        0x78 // IR pulse width
#define CYGARC_REG_SCIF_SCSMR_PSEL            0x04 // IR pulse selector(?)

// Serial Control Register
#define CYGARC_REG_SCIF_SCSCR_TIE             0x80 // transmit interrupt enable
#define CYGARC_REG_SCIF_SCSCR_RIE             0x40 // receive interrupt enable
#define CYGARC_REG_SCIF_SCSCR_TE              0x20 // transmit enable
#define CYGARC_REG_SCIF_SCSCR_RE              0x10 // receive enable
#define CYGARC_REG_SCIF_SCSCR_CKE1            0x02 // clock enable 1
#define CYGARC_REG_SCIF_SCSCR_CKE0            0x01 // clock enable 0

// Serial Status Register
#define CYGARC_REG_SCIF_SCSSR_PER_MASK        0xf000 // number of parity errors
#define CYGARC_REG_SCIF_SCSSR_PER_shift       12
#define CYGARC_REG_SCIF_SCSSR_FER_MASK        0x0f00 // number of framing errors
#define CYGARC_REG_SCIF_SCSSR_FER_shift       8
#define CYGARC_REG_SCIF_SCSSR_ER              0x0080 // receive error
#define CYGARC_REG_SCIF_SCSSR_TEND            0x0040 // transmit end
#define CYGARC_REG_SCIF_SCSSR_TDFE            0x0020 // transmit fifo data empty
#define CYGARC_REG_SCIF_SCSSR_BRK             0x0010 // break detection
#define CYGARC_REG_SCIF_SCSSR_FER             0x0008 // framing error
#define CYGARC_REG_SCIF_SCSSR_PER             0x0004 // parity error
#define CYGARC_REG_SCIF_SCSSR_RDF             0x0002 // receive fifo data full
#define CYGARC_REG_SCIF_SCSSR_DR              0x0001 // receive data ready

// When clearing the status register, always write the value:
// CYGARC_REG_SCSSR_CLEARMASK & ~bit
// to prevent other bits than the one of interest to be cleared.
#define CYGARC_REG_SCIF_SCSSR_CLEARMASK       0x00f3

// Serial FIFO Control Register
#define CYGARC_REG_SCIF_SCFCR_RTRG_MASK       0xc0   // receive fifo data trigger
#define CYGARC_REG_SCIF_SCFCR_RTRG_1          0x00   // trigger on 1 char 
#define CYGARC_REG_SCIF_SCFCR_RTRG_4          0x40   // trigger on 4 chars
#define CYGARC_REG_SCIF_SCFCR_RTRG_8          0x80   // trigger on 8 chars
#define CYGARC_REG_SCIF_SCFCR_RTRG_14         0xc0   // trigger on 14 chars 

#define CYGARC_REG_SCIF_SCFCR_TTRG_MASK       0x30   // transmit fifo data trigger
#define CYGARC_REG_SCIF_SCFCR_TTRG_8          0x00   // trigger on 8 chars 
#define CYGARC_REG_SCIF_SCFCR_TTRG_4          0x10   // trigger on 4 chars
#define CYGARC_REG_SCIF_SCFCR_TTRG_2          0x20   // trigger on 2 chars
#define CYGARC_REG_SCIF_SCFCR_TTRG_1          0x30   // trigger on 1 char

#define CYGARC_REG_SCIF_SCFCR_MCE             0x08   // modem control enable
#define CYGARC_REG_SCIF_SCFCR_TFRST           0x04   // transmit fifo reset
#define CYGARC_REG_SCIF_SCFCR_RFRST           0x02   // receive fifo reset
#define CYGARC_REG_SCIF_SCFCR_LOOP            0x01   // loop back test

// Serial FIFO Data Count Set Register
#define CYGARC_REG_SCIF_SCFDR_RCOUNT_MASK    0x001f // number of chars in r fifo
#define CYGARC_REG_SCIF_SCFDR_RCOUNT_shift   0
#define CYGARC_REG_SCIF_SCFDR_TCOUNT_MASK    0x1f00 // number of chars in t fifo
#define CYGARC_REG_SCIF_SCFDR_TCOUNT_shift   8

#endif /* CYGARC_SH_MOD_SCIF */
