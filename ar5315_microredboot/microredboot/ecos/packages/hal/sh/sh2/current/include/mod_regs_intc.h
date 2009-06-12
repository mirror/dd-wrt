//=============================================================================
//
//      mod_regs_intc.h
//
//      INTC (interrupt controller) Module register definitions
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
#define CYGARC_REG_ICR                    0xfffffee0
#define CYGARC_REG_IRQCSR                 0xfffffee8
#define CYGARC_REG_IPRA                   0xfffffee2
#define CYGARC_REG_IPRB                   0xfffffe60
#define CYGARC_REG_IPRC                   0xfffffee6
#define CYGARC_REG_IPRD                   0xfffffe40
#define CYGARC_REG_IPRE                   0xfffffec0

#define CYGARC_REG_VCRA                   0xfffffe62
#define CYGARC_REG_VCRB                   0xfffffe64
#define CYGARC_REG_VCRC                   0xfffffe66
#define CYGARC_REG_VCRD                   0xfffffe68
#define CYGARC_REG_VCRE                   0xfffffe42
#define CYGARC_REG_VCRF                   0xfffffe44
#define CYGARC_REG_VCRG                   0xfffffe46
#define CYGARC_REG_VCRH                   0xfffffe48
#define CYGARC_REG_VCRI                   0xfffffe4a
#define CYGARC_REG_VCRJ                   0xfffffe4c
#define CYGARC_REG_VCRK                   0xfffffe4e
#define CYGARC_REG_VCRL                   0xfffffe50
#define CYGARC_REG_VCRM                   0xfffffe52
#define CYGARC_REG_VCRN                   0xfffffe54
#define CYGARC_REG_VCRO                   0xfffffe56
#define CYGARC_REG_VCRP                   0xfffffec2
#define CYGARC_REG_VCRQ                   0xfffffec4
#define CYGARC_REG_VCRR                   0xfffffec6
#define CYGARC_REG_VCRS                   0xfffffec8
#define CYGARC_REG_VCRT                   0xfffffeca
#define CYGARC_REG_VCRU                   0xfffffecc
#define CYGARC_REG_VCRWDT                 0xfffffee4
#define CYGARC_REG_VCRDMA0                0xffffffa0 // 32 bit
#define CYGARC_REG_VCRDMA1                0xffffffa8 // 32 bit

#define CYGARC_REG_ICR_NMIL               0x8000
#define CYGARC_REG_ICR_NMIE               0x0100
#define CYGARC_REG_ICR_EXIMD              0x0002
#define CYGARC_REG_ICR_VECMD              0x0001

#define CYGARC_REG_IPRA_DMAC_MASK         0x0f00
#define CYGARC_REG_IPRA_DMAC_PRI1         0x0100
#define CYGARC_REG_IPRA_WDT_MASK          0x00f0
#define CYGARC_REG_IPRA_WDT_PRI1          0x0010

#define CYGARC_REG_IPRB_EDMAC_MASK        0xf000
#define CYGARC_REG_IPRB_EDMAC_PRI1        0x1000
#define CYGARC_REG_IPRB_FRT_MASK          0x0f00
#define CYGARC_REG_IPRB_FRT_PRI1          0x0100

#define CYGARC_REG_IPRC_IRQ3_MASK         0xf000
#define CYGARC_REG_IPRC_IRQ3_PRI1         0x1000
#define CYGARC_REG_IPRC_IRQ2_MASK         0x0f00
#define CYGARC_REG_IPRC_IRQ2_PRI1         0x0100
#define CYGARC_REG_IPRC_IRQ1_MASK         0x00f0
#define CYGARC_REG_IPRC_IRQ1_PRI1         0x0010
#define CYGARC_REG_IPRC_IRQ0_MASK         0x000f
#define CYGARC_REG_IPRC_IRQ0_PRI1         0x0001

#define CYGARC_REG_IPRD_TPU0_MASK         0xf000
#define CYGARC_REG_IPRD_TPU0_PRI1         0x1000
#define CYGARC_REG_IPRD_TPU1_MASK         0x0f00
#define CYGARC_REG_IPRD_TPU1_PRI1         0x0100
#define CYGARC_REG_IPRD_TPU2_MASK         0x00f0
#define CYGARC_REG_IPRD_TPU2_PRI1         0x0010
#define CYGARC_REG_IPRD_SCIF1_MASK        0x000f
#define CYGARC_REG_IPRD_SCIF1_PRI1        0x0001

#define CYGARC_REG_IPRE_SCIF2_MASK        0xf000
#define CYGARC_REG_IPRE_SCIF2_PRI1        0x1000
#define CYGARC_REG_IPRE_SIO0_MASK         0x0f00
#define CYGARC_REG_IPRE_SIO0_PRI1         0x0100
#define CYGARC_REG_IPRE_SIO1_MASK         0x00f0
#define CYGARC_REG_IPRE_SIO1_PRI1         0x0010
#define CYGARC_REG_IPRE_SIO2_MASK         0x000f
#define CYGARC_REG_IPRE_SIO2_PRI1         0x0001

// The (initial) IRQ mode is controlled by configuration.
#ifdef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
# define CYGARC_REG_ICR_INIT (CYGARC_REG_ICR_EXIMD)
#else
# define CYGARC_REG_ICR_INIT 0x0000
#endif

#define CYGARC_REG_IRQCSR_IRQ_LOWLEVEL    0
#define CYGARC_REG_IRQCSR_IRQ_RISING      1
#define CYGARC_REG_IRQCSR_IRQ_FALLING     2
#define CYGARC_REG_IRQCSR_IRQ_BOTHEDGES   3
#define CYGARC_REG_IRQCSR_IRQ_mask        3

#define CYGARC_REG_IRQCSR_IRQ3_mask       0xc000
#define CYGARC_REG_IRQCSR_IRQ3_shift      14
#define CYGARC_REG_IRQCSR_IRQ2_mask       0x3000
#define CYGARC_REG_IRQCSR_IRQ2_shift      12
#define CYGARC_REG_IRQCSR_IRQ1_mask       0x0c00
#define CYGARC_REG_IRQCSR_IRQ1_shift      10
#define CYGARC_REG_IRQCSR_IRQ0_mask       0x0300
#define CYGARC_REG_IRQCSR_IRQ0_shift      8

#define CYGARC_REG_IRQCSR_IRL3PS          0x0080
#define CYGARC_REG_IRQCSR_IRL2PS          0x0040
#define CYGARC_REG_IRQCSR_IRL1PS          0x0020
#define CYGARC_REG_IRQCSR_IRL0PS          0x0010

#define CYGARC_REG_IRQCSR_IRQ3F           0x0008
#define CYGARC_REG_IRQCSR_IRQ2F           0x0004
#define CYGARC_REG_IRQCSR_IRQ1F           0x0002
#define CYGARC_REG_IRQCSR_IRQ0F           0x0001
