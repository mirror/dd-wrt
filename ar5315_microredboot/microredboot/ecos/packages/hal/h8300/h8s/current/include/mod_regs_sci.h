#ifndef CYGONCE_MOD_REGS_SCI_H
#define CYGONCE_MOD_REGS_SCI_H

//==========================================================================
//
//      mod_regs_sci.h
//
//      Serial Communication Interface Register
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
// Author(s):    yoshinori sato
// Contributors: yoshinori sato
// Date:         2002-02-19
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGARC_SMR0  0xFFFF78
#define CYGARC_BRR0  0xFFFF79
#define CYGARC_SCR0  0xFFFF7A
#define CYGARC_TDR0  0xFFFF7B
#define CYGARC_SSR0  0xFFFF7C
#define CYGARC_RDR0  0xFFFF7D
#define CYGARC_SCMR0 0xFFFF7E
#define CYGARC_SMR1  0xFFFF80
#define CYGARC_BRR1  0xFFFF81
#define CYGARC_SCR1  0xFFFF82
#define CYGARC_TDR1  0xFFFF83
#define CYGARC_SSR1  0xFFFF84
#define CYGARC_RDR1  0xFFFF85
#define CYGARC_SCMR1 0xFFFF86
#define CYGARC_SMR2  0xFFFF88
#define CYGARC_BRR2  0xFFFF89
#define CYGARC_SCR2  0xFFFF8A
#define CYGARC_TDR2  0xFFFF8B
#define CYGARC_SSR2  0xFFFF8C
#define CYGARC_RDR2  0xFFFF8D
#define CYGARC_SCMR2 0xFFFF8E

#define CYGARC_IRCR0 0xFFFE1E
#define CYGARC_SEMR  0xFFFDA8

// Serial Mode Register
#define CYGARC_REG_SCSMR_CA             0x80 // communication mode
#define CYGARC_REG_SCSMR_CHR            0x40 // character length (7 if set)
#define CYGARC_REG_SCSMR_PE             0x20 // parity enable
#define CYGARC_REG_SCSMR_OE             0x10 // parity mode
#define CYGARC_REG_SCSMR_STOP           0x08 // stop bit length
#define CYGARC_REG_SCSMR_MP             0x04 // multiprocessor mode
#define CYGARC_REG_SCSMR_CKS1           0x02 // clock select 1
#define CYGARC_REG_SCSMR_CKS0           0x01 // clock select 0
#define CYGARC_REG_SCSMR_CKSx_MASK      0x03 // mask

// Serial Control Register
#define CYGARC_REG_SCSCR_TIE            0x80 // transmit interrupt enable
#define CYGARC_REG_SCSCR_RIE            0x40 // receive interrupt enable
#define CYGARC_REG_SCSCR_TE             0x20 // transmit enable
#define CYGARC_REG_SCSCR_RE             0x10 // receive enable
#define CYGARC_REG_SCSCR_MPIE           0x08 // multiprocessor interrupt enable
#define CYGARC_REG_SCSCR_TEIE           0x04 // transmit-end interrupt enable
#define CYGARC_REG_SCSCR_CKE1           0x02 // clock enable 1
#define CYGARC_REG_SCSCR_CKE0           0x01 // clock enable 0

// Serial Status Register
#define CYGARC_REG_SCSSR_TDRE           0x80 // transmit data register empty
#define CYGARC_REG_SCSSR_RDRF           0x40 // receive data register full
#define CYGARC_REG_SCSSR_ORER           0x20 // overrun error
#define CYGARC_REG_SCSSR_FER            0x10 // framing error
#define CYGARC_REG_SCSSR_PER            0x08 // parity error
#define CYGARC_REG_SCSSR_TEND           0x04 // transmit end
#define CYGARC_REG_SCSSR_MPB            0x02 // multiprocessor bit
#define CYGARC_REG_SCSSR_MPBT           0x01 // multiprocessor bit transfer

// When clearing the status register, always write the value:
// CYGARC_REG_SCSSR_CLEARMASK & ~bit
// to prevent other bits than the one of interest to be cleared.
#define CYGARC_REG_SCSSR_CLEARMASK      0xf8

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
((((CYGHWR_HAL_H8300_PROCESSOR_SPEED/32/1/(_b_))-1)<256) ? 1 : \
 (((CYGHWR_HAL_H8300_PROCESSOR_SPEED/32/4/(_b_))-1)<256) ? 4 : \
 (((CYGHWR_HAL_H8300_PROCESSOR_SPEED/32/16/(_b_))-1)<256) ? 16 : 64)

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
       (((CYGHWR_HAL_H8300_PROCESSOR_SPEED+CYGARC_SCBRR_ROUNDING(_b_))/32/CYGARC_SCBRR_PRESCALE(_b_)/(_b_))-1))

#endif
