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
// Copyright (C) 2003 Nick Garnett 
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
// Contributors:jskov, nickg
// Date:        2000-10-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// Interrupt registers, module type 1

#define CYGARC_REG_TRA                  0xFF000020
#define CYGARC_REG_EXCEVT               0xFF000024
#define CYGARC_REG_INTEVT               0xFF000028

#define CYGARC_REG_ICR                  0xFFD00000
#define CYGARC_REG_IPRA                 0xFFD00004
#define CYGARC_REG_IPRB                 0xFFD00008
#define CYGARC_REG_IPRC                 0xFFD0000C

#define CYGARC_REG_ICR_NMIL             0x8000
#define CYGARC_REG_ICR_MAI              0x4000
#define CYGARC_REG_ICR_NMIB             0x0200
#define CYGARC_REG_ICR_NMIE             0x0100
#define CYGARC_REG_ICR_IRLM             0x0080
#define CYGARC_REG_ICR_SRST             0x0001

#define CYGARC_REG_IPRA_TMU0_MASK       0xf000
#define CYGARC_REG_IPRA_TMU0_PRI1       0x1000
#define CYGARC_REG_IPRA_TMU1_MASK       0x0f00
#define CYGARC_REG_IPRA_TMU1_PRI1       0x0100
#define CYGARC_REG_IPRA_TMU2_MASK       0x00f0
#define CYGARC_REG_IPRA_TMU2_PRI1       0x0010
#define CYGARC_REG_IPRA_RTC_MASK        0x000f
#define CYGARC_REG_IPRA_RTC_PRI1        0x0001

#define CYGARC_REG_IPRB_WDT_MASK        0xf000
#define CYGARC_REG_IPRB_WDT_PRI1        0x1000
#define CYGARC_REG_IPRB_REF_MASK        0x0f00
#define CYGARC_REG_IPRB_REF_PRI1        0x0100
#define CYGARC_REG_IPRB_SCI_MASK        0x00f0
#define CYGARC_REG_IPRB_SCI_PRI1        0x0010

#define CYGARC_REG_IPRC_GPIO_MASK       0xF000
#define CYGARC_REG_IPRC_GPIO_PRI1       0x1000
#define CYGARC_REG_IPRC_DMAC_MASK       0x0F00
#define CYGARC_REG_IPRC_DMAC_PRI1       0x0100
#define CYGARC_REG_IPRC_SCIF_MASK       0x00F0
#define CYGARC_REG_IPRC_SCIF_PRI1       0x0010
#define CYGARC_REG_IPRC_HUDI_MASK       0x000F
#define CYGARC_REG_IPRC_HUDI_PRI1       0x0001

#if (CYGARC_SH_MOD_INTC == 2)
#define CYGARC_REG_IPRD                  0xffd00010
#define CYGARC_REG_INTPRI00              0xfe080000
#define CYGARC_REG_INTREQ00              0xfe080020
#define CYGARC_REG_INTMSK00              0xfe080040
#define CYGARC_REG_INTMSKCLR00           0xfe080060

#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
#define CYGARC_REG_IPRD_IRL0_MASK        0xf000
#define CYGARC_REG_IPRD_IRL0_PRI1        0x1000
#define CYGARC_REG_IPRD_IRL1_MASK        0x0f00
#define CYGARC_REG_IPRD_IRL1_PRI1        0x0100
#define CYGARC_REG_IPRD_IRL2_MASK        0x00f0
#define CYGARC_REG_IPRD_IRL2_PRI1        0x0010
#define CYGARC_REG_IPRD_IRL3_MASK        0x000f
#define CYGARC_REG_IPRD_IRL3_PRI1        0x0001
#endif

#define CYGARC_REG_INTPRI00_PCISERR_MASK 0x0000000f
#define CYGARC_REG_INTPRI00_PCISERR_PRI1 0x00000001
#define CYGARC_REG_INTPRI00_PCIERR_MASK  0x000000f0
#define CYGARC_REG_INTPRI00_PCIERR_PRI1  0x00000010
#define CYGARC_REG_INTPRI00_TUNI3_MASK   0x00000f00
#define CYGARC_REG_INTPRI00_TUNI3_PRI1   0x00000100
#define CYGARC_REG_INTPRI00_TUNI4_MASK   0x0000f000
#define CYGARC_REG_INTPRI00_TUNI4_PRI1   0x00001000

#endif

// The (initial) IRQ mode is controlled by configuration.
#ifdef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
# define CYGARC_REG_ICR_INIT 0x0000
#else
# define CYGARC_REG_ICR_INIT (CYGARC_REG_ICR_IRLM)
#endif
