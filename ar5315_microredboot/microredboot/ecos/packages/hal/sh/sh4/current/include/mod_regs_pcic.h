//=============================================================================
//
//      mod_regs_pcic.h
//
//      PCIC (PCI controller) Module register definitions
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
// Date:        2001-07-10
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// PCI control registers
#define CYGARC_REG_PCIC_BASE           0xfe200000
#define CYGARC_REG_PCIC_IO_BASE        0xfe240000
#define CYGARC_REG_PCIC_IO_BASE_MASK   0x0003ffff
#define CYGARC_REG_PCIC_MEM_BASE       0xfd000000



#define CYGARC_REG_PCIC_CFG            (CYGARC_REG_PCIC_BASE)

#define CYGARC_REG_PCIC_CR             (CYGARC_REG_PCIC_BASE + 0x100)
#define CYGARC_REG_PCIC_LSR0           (CYGARC_REG_PCIC_BASE + 0x104)
#define CYGARC_REG_PCIC_LSR1           (CYGARC_REG_PCIC_BASE + 0x108)
#define CYGARC_REG_PCIC_LAR0           (CYGARC_REG_PCIC_BASE + 0x10c)
#define CYGARC_REG_PCIC_LAR1           (CYGARC_REG_PCIC_BASE + 0x110)
#define CYGARC_REG_PCIC_INT            (CYGARC_REG_PCIC_BASE + 0x114)
#define CYGARC_REG_PCIC_INTM           (CYGARC_REG_PCIC_BASE + 0x118)
#define CYGARC_REG_PCIC_ALR            (CYGARC_REG_PCIC_BASE + 0x11c)
#define CYGARC_REG_PCIC_CLR            (CYGARC_REG_PCIC_BASE + 0x120)
#define CYGARC_REG_PCIC_AINT           (CYGARC_REG_PCIC_BASE + 0x130)
#define CYGARC_REG_PCIC_AINTM          (CYGARC_REG_PCIC_BASE + 0x134)
#define CYGARC_REG_PCIC_BMLR           (CYGARC_REG_PCIC_BASE + 0x138)
#define CYGARC_REG_PCIC_DMABT          (CYGARC_REG_PCIC_BASE + 0x140)
#define CYGARC_REG_PCIC_DPA0           (CYGARC_REG_PCIC_BASE + 0x180)
#define CYGARC_REG_PCIC_DLA0           (CYGARC_REG_PCIC_BASE + 0x184)
#define CYGARC_REG_PCIC_DTC0           (CYGARC_REG_PCIC_BASE + 0x188)
#define CYGARC_REG_PCIC_DCR0           (CYGARC_REG_PCIC_BASE + 0x18c)
#define CYGARC_REG_PCIC_DPA1           (CYGARC_REG_PCIC_BASE + 0x190)
#define CYGARC_REG_PCIC_DLA1           (CYGARC_REG_PCIC_BASE + 0x194)
#define CYGARC_REG_PCIC_DTC1           (CYGARC_REG_PCIC_BASE + 0x198)
#define CYGARC_REG_PCIC_DCR1           (CYGARC_REG_PCIC_BASE + 0x19c)
#define CYGARC_REG_PCIC_DPA2           (CYGARC_REG_PCIC_BASE + 0x1a0)
#define CYGARC_REG_PCIC_DLA2           (CYGARC_REG_PCIC_BASE + 0x1a4)
#define CYGARC_REG_PCIC_DTC2           (CYGARC_REG_PCIC_BASE + 0x1a8)
#define CYGARC_REG_PCIC_DCR2           (CYGARC_REG_PCIC_BASE + 0x1ac)
#define CYGARC_REG_PCIC_DPA3           (CYGARC_REG_PCIC_BASE + 0x1b0)
#define CYGARC_REG_PCIC_DLA3           (CYGARC_REG_PCIC_BASE + 0x1b4)
#define CYGARC_REG_PCIC_DTC3           (CYGARC_REG_PCIC_BASE + 0x1b8)
#define CYGARC_REG_PCIC_DCR3           (CYGARC_REG_PCIC_BASE + 0x1bc)
#define CYGARC_REG_PCIC_PAR            (CYGARC_REG_PCIC_BASE + 0x1c0)
#define CYGARC_REG_PCIC_MBR            (CYGARC_REG_PCIC_BASE + 0x1c4)
#define CYGARC_REG_PCIC_IOBR           (CYGARC_REG_PCIC_BASE + 0x1c8)
#define CYGARC_REG_PCIC_PINT           (CYGARC_REG_PCIC_BASE + 0x1cc)
#define CYGARC_REG_PCIC_PINTM          (CYGARC_REG_PCIC_BASE + 0x1d0)
#define CYGARC_REG_PCIC_CLKR           (CYGARC_REG_PCIC_BASE + 0x1d4)
#define CYGARC_REG_PCIC_BCR1           (CYGARC_REG_PCIC_BASE + 0x1e0)
#define CYGARC_REG_PCIC_BCR2           (CYGARC_REG_PCIC_BASE + 0x1e4)
#define CYGARC_REG_PCIC_WCR1           (CYGARC_REG_PCIC_BASE + 0x1e8)
#define CYGARC_REG_PCIC_WCR2           (CYGARC_REG_PCIC_BASE + 0x1ec)
#define CYGARC_REG_PCIC_WCR3           (CYGARC_REG_PCIC_BASE + 0x1f0)
#define CYGARC_REG_PCIC_MCR            (CYGARC_REG_PCIC_BASE + 0x1f4)
#define CYGARC_REG_PCIC_PCTR           (CYGARC_REG_PCIC_BASE + 0x200)
#define CYGARC_REG_PCIC_PDTR           (CYGARC_REG_PCIC_BASE + 0x204)
#define CYGARC_REG_PCIC_PDR            (CYGARC_REG_PCIC_BASE + 0x220)


#define CYGARC_REG_PCIC_CR_MAGIC          0xa5000000
#define CYGARC_REG_PCIC_CR_TRDSGL         0x00000200
#define CYGARC_REG_PCIC_CR_BYTESWAP       0x00000100
#define CYGARC_REG_PCIC_CR_PCIPUP         0x00000080
#define CYGARC_REG_PCIC_CR_BMABT          0x00000040
#define CYGARC_REG_PCIC_CR_MD10           0x00000020
#define CYGARC_REG_PCIC_CR_MD9            0x00000010
#define CYGARC_REG_PCIC_CR_SERR           0x00000008
#define CYGARC_REG_PCIC_CR_INTA           0x00000004
#define CYGARC_REG_PCIC_CR_PCIRST         0x00000002
#define CYGARC_REG_PCIC_CR_CFINIT         0x00000001
#define CYGARC_REG_PCIC_CR_INIT           0xa5000001

#define CYGARC_REG_PCIC_IOBR_MASK         0xfffc0000

#define CYGARC_REG_PCIC_PAR_ENABLE        0x80000000
#define CYGARC_REG_PCIC_PAR_BUSNO_shift   16
#define CYGARC_REG_PCIC_PAR_FUNC_shift    8


#define CYGARC_REG_PCIC_INTM_M_LOCKON     0x00008000
#define CYGARC_REG_PCIC_INTM_T_TGT_ABORT  0x00004000
#define CYGARC_REG_PCIC_INTM_TGT_RETRY    0x00000200
#define CYGARC_REG_PCIC_INTM_MST_DIS      0x00000100
#define CYGARC_REG_PCIC_INTM_ADRPERR      0x00000080
#define CYGARC_REG_PCIC_INTM_SERR_DET     0x00000040
#define CYGARC_REG_PCIC_INTM_T_DPERR_WT   0x00000020
#define CYGARC_REG_PCIC_INTM_T_PERR_DET   0x00000010
#define CYGARC_REG_PCIC_INTM_M_TGT_ABORT  0x00000008
#define CYGARC_REG_PCIC_INTM_M_MST_ABORT  0x00000004
#define CYGARC_REG_PCIC_INTM_M_DPERR_WT   0x00000002
#define CYGARC_REG_PCIC_INTM_M_DPERR_RD   0x00000001
#define CYGARC_REG_PCIC_INTM_INIT         0x0000c3ff

#define CYGARC_REG_PCIC_AINTM_MST_BRKN    0x00002000
#define CYGARC_REG_PCIC_AINTM_TGT_BUSTO   0x00001000
#define CYGARC_REG_PCIC_AINTM_MST_BUSTO   0x00000800
#define CYGARC_REG_PCIC_AINTM_TGT_ABORT   0x00000008
#define CYGARC_REG_PCIC_AINTM_MST_ABORT   0x00000004
#define CYGARC_REG_PCIC_AINTM_DPERR_WT    0x00000002
#define CYGARC_REG_PCIC_AINTM_DPERR_RD    0x00000001
#define CYGARC_REG_PCIC_AINTM_INIT        0x0000380f

