//=============================================================================
//
//      mod_regs_dma.h
//
//      DMA (direct memory access) Module register definitions
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
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// DMA Controller registers

#define CYGARC_REG_SAR0                 0xa4000020
#define CYGARC_REG_DAR0                 0xa4000024
#define CYGARC_REG_DMATCR0              0xa4000028
#define CYGARC_REG_CHCR0                0xa400002c
#define CYGARC_REG_SAR1                 0xa4000030
#define CYGARC_REG_DAR1                 0xa4000034
#define CYGARC_REG_DMATCR1              0xa4000038
#define CYGARC_REG_CHCR1                0xa400003c
#define CYGARC_REG_SAR2                 0xa4000040
#define CYGARC_REG_DAR2                 0xa4000044
#define CYGARC_REG_DMATCR2              0xa4000048
#define CYGARC_REG_CHCR2                0xa400004c
#define CYGARC_REG_SAR3                 0xa4000050
#define CYGARC_REG_DAR3                 0xa4000054
#define CYGARC_REG_DMATCR3              0xa4000058
#define CYGARC_REG_CHCR3                0xa400005c
#define CYGARC_REG_DMAOR                0xa4000060

// Offsets from base register
#define CYGARC_REG_SAR                  0x00
#define CYGARC_REG_DAR                  0x04
#define CYGARC_REG_DMATCR               0x08
#define CYGARC_REG_CHCR                 0x0c


// DMA Channel Control Register. If there's a digit suffix to CHCR the flag
// is only valid in the listed channels.
#define CYGARC_REG_CHCR3_DI             0x00100000 // direct/indirect selection
#define CYGARC_REG_CHCR2_RO             0x00080000 // source address reload
#define CYGARC_REG_CHCR01_RL            0x00040000 // request check level
#define CYGARC_REG_CHCR01_AM            0x00020000 // acknowledge mode
#define CYGARC_REG_CHCR01_AL            0x00010000 // acknowledge level
#define CYGARC_REG_CHCR_DM1             0x00008000 // destination address mode
#define CYGARC_REG_CHCR_DM0             0x00004000
#define CYGARC_REG_CHCR_SM1             0x00002000 // source address mode
#define CYGARC_REG_CHCR_SM0             0x00001000
#define CYGARC_REG_CHCR_RS3             0x00000800 // resource select
#define CYGARC_REG_CHCR_RS2             0x00000400
#define CYGARC_REG_CHCR_RS1             0x00000200
#define CYGARC_REG_CHCR_RS0             0x00000100
#define CYGARC_REG_CHCR01_DS            0x00000040 // DREQ select
#define CYGARC_REG_CHCR_TM              0x00000020 // transmit mode
#define CYGARC_REG_CHCR_TS1             0x00000010 // transmit size
#define CYGARC_REG_CHCR_TS0             0x00000008
#define CYGARC_REG_CHCR_IE              0x00000004 // interrupt enable
#define CYGARC_REG_CHCR_TE              0x00000002 // transfer end
#define CYGARC_REG_CHCR_DE              0x00000001 // DMAC enable

// Resource select options
#define CYGARC_REG_CHCR_RS_EXT_DUAL     0x00000000
#define CYGARC_REG_CHCR_RS_EXT_EX_DAC   0x00000100
#define CYGARC_REG_CHCR_RS_EXT_DAC_EX   0x00000300
#define CYGARC_REG_CHCR_RS_AUTO         0x00000400
#define CYGARC_REG_CHCR_RS_IRDA_TX      0x00000a00
#define CYGARC_REG_CHCR_RS_IRDA_RX      0x00000b00
#define CYGARC_REG_CHCR_RS_SCIF_TX      0x00000c00
#define CYGARC_REG_CHCR_RS_SCIF_RX      0x00000d00
#define CYGARC_REG_CHCR_RS_AD           0x00000e00
#define CYGARC_REG_CHCR_RS_CMT          0x00000f00


// DMA Operation Register
#define CYGARC_REG_DMAOR_PR1            0x0200     // priority level
#define CYGARC_REG_DMAOR_PR0            0x0100
#define CYGARC_REG_DMAOR_AE             0x0004     // address error flag
#define CYGARC_REG_DMAOR_NMIF           0x0002     // NMI flag
#define CYGARC_REG_DMAOR_DME            0x0001     // DMA master enable
