//=============================================================================
//
//      mod_regs_intc_2.h
//
//      INTC (interrupt controller) Module (type 2) register definitions
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
// Date:        2002-01-15
//              
//####DESCRIPTIONEND####
//
//=============================================================================

// NOTE: This'll need restructuring as other variants are added. Type 1 should
// be fewest common registers.

//--------------------------------------------------------------------------
// Interrupt registers, module type 1
#define CYGARC_REG_IPRA                   0xffff8348
#define CYGARC_REG_IPRB                   0xffff834a
#define CYGARC_REG_IPRC                   0xffff834c
#define CYGARC_REG_IPRD                   0xffff834e
#define CYGARC_REG_IPRE                   0xffff8350
#define CYGARC_REG_IPRF                   0xffff8352
#define CYGARC_REG_IPRG                   0xffff8354
#define CYGARC_REG_IPRH                   0xffff8356
#define CYGARC_REG_ICR                    0xffff8358
#define CYGARC_REG_ISR                    0xffff835a


#define CYGARC_REG_ICR_NMIL               0x8000
#define CYGARC_REG_ICR_NMIE               0x0100
#define CYGARC_REG_ICR_IRQ0S              0x0080
#define CYGARC_REG_ICR_IRQ1S              0x0040
#define CYGARC_REG_ICR_IRQ2S              0x0020
#define CYGARC_REG_ICR_IRQ3S              0x0010
#define CYGARC_REG_ICR_IRQ4S              0x0008
#define CYGARC_REG_ICR_IRQ5S              0x0004
#define CYGARC_REG_ICR_IRQ6S              0x0002
#define CYGARC_REG_ICR_IRQ7S              0x0001

#define CYGARC_REG_IPRA_IRQ0_MASK         0xf000
#define CYGARC_REG_IPRA_IRQ0_PRI1         0x1000
#define CYGARC_REG_IPRA_IRQ1_MASK         0x0f00
#define CYGARC_REG_IPRA_IRQ1_PRI1         0x0100
#define CYGARC_REG_IPRA_IRQ2_MASK         0x00f0
#define CYGARC_REG_IPRA_IRQ2_PRI1         0x0010
#define CYGARC_REG_IPRA_IRQ3_MASK         0x000f
#define CYGARC_REG_IPRA_IRQ3_PRI1         0x0001

#define CYGARC_REG_IPRB_IRQ4_MASK         0xf000
#define CYGARC_REG_IPRB_IRQ4_PRI1         0x1000
#define CYGARC_REG_IPRB_IRQ5_MASK         0x0f00
#define CYGARC_REG_IPRB_IRQ5_PRI1         0x0100
#define CYGARC_REG_IPRB_IRQ6_MASK         0x00f0
#define CYGARC_REG_IPRB_IRQ6_PRI1         0x0010
#define CYGARC_REG_IPRB_IRQ7_MASK         0x000f
#define CYGARC_REG_IPRB_IRQ7_PRI1         0x0001

#define CYGARC_REG_IPRC_DMAC0_MASK         0xf000
#define CYGARC_REG_IPRC_DMAC0_PRI1         0x1000
#define CYGARC_REG_IPRC_DMAC1_MASK         0x0f00
#define CYGARC_REG_IPRC_DMAC1_PRI1         0x0100
#define CYGARC_REG_IPRC_DMAC2_MASK         0x00f0
#define CYGARC_REG_IPRC_DMAC2_PRI1         0x0010
#define CYGARC_REG_IPRC_DMAC3_MASK         0x000f
#define CYGARC_REG_IPRC_DMAC3_PRI1         0x0001

#define CYGARC_REG_IPRD_MTU0A_MASK         0xf000
#define CYGARC_REG_IPRD_MTU0A_PRI1         0x1000
#define CYGARC_REG_IPRD_MTU0B_MASK         0x0f00
#define CYGARC_REG_IPRD_MTU0B_PRI1         0x0100
#define CYGARC_REG_IPRD_MTU1A_MASK         0x00f0
#define CYGARC_REG_IPRD_MTU1A_PRI1         0x0010
#define CYGARC_REG_IPRD_MTU1B_MASK         0x000f
#define CYGARC_REG_IPRD_MTU1B_PRI1         0x0001

#define CYGARC_REG_IPRE_MTU2A_MASK         0xf000
#define CYGARC_REG_IPRE_MTU2A_PRI1         0x1000
#define CYGARC_REG_IPRE_MTU2B_MASK         0x0f00
#define CYGARC_REG_IPRE_MTU2B_PRI1         0x0100
#define CYGARC_REG_IPRE_MTU3A_MASK         0x00f0
#define CYGARC_REG_IPRE_MTU3A_PRI1         0x0010
#define CYGARC_REG_IPRE_MTU3B_MASK         0x000f
#define CYGARC_REG_IPRE_MTU3B_PRI1         0x0001

#define CYGARC_REG_IPRF_MTU4A_MASK         0xf000
#define CYGARC_REG_IPRF_MTU4A_PRI1         0x1000
#define CYGARC_REG_IPRF_MTU4B_MASK         0x0f00
#define CYGARC_REG_IPRF_MTU4B_PRI1         0x0100
#define CYGARC_REG_IPRF_SCI0_MASK          0x00f0
#define CYGARC_REG_IPRF_SCI0_PRI1          0x0010
#define CYGARC_REG_IPRF_SCI1_MASK          0x000f
#define CYGARC_REG_IPRF_SCI1_PRI1          0x0001

#define CYGARC_REG_IPRG_AD_MASK            0xf000
#define CYGARC_REG_IPRG_AD_PRI1            0x1000
#define CYGARC_REG_IPRG_DTC_MASK           0x0f00
#define CYGARC_REG_IPRG_DTC_PRI1           0x0100
#define CYGARC_REG_IPRG_CMT0_MASK          0x00f0
#define CYGARC_REG_IPRG_CMT0_PRI1          0x0010
#define CYGARC_REG_IPRG_CMT1_MASK          0x000f
#define CYGARC_REG_IPRG_CMT1_PRI1          0x0001

#define CYGARC_REG_IPRH_WDT_MASK           0xf000
#define CYGARC_REG_IPRH_WDT_PRI1           0x1000
#define CYGARC_REG_IPRH_IO_MASK            0x0f00
#define CYGARC_REG_IPRH_IO_PRI1            0x0100

// The (initial) IRQ mode
#define CYGARC_REG_ICR_INIT 0x0000

#define CYGARC_REG_ISR_IRQ0F           0x0080
#define CYGARC_REG_ISR_IRQ1F           0x0040
#define CYGARC_REG_ISR_IRQ2F           0x0020
#define CYGARC_REG_ISR_IRQ3F           0x0010
#define CYGARC_REG_ISR_IRQ4F           0x0008
#define CYGARC_REG_ISR_IRQ5F           0x0004
#define CYGARC_REG_ISR_IRQ6F           0x0002
#define CYGARC_REG_ISR_IRQ7F           0x0001
