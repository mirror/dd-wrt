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
// Date:        2002-01-21
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// DMA Controller registers

#define CYGARC_REG_SAR0                 0xffffff80
#define CYGARC_REG_DAR0                 0xffffff84
#define CYGARC_REG_DMATCR0              0xffffff88
#define CYGARC_REG_CHCR0                0xffffff8c
#define CYGARC_REG_DRCR0                0xfffffe71
#define CYGARC_REG_SAR1                 0xffffff90
#define CYGARC_REG_DAR1                 0xffffff94
#define CYGARC_REG_DMATCR1              0xffffff98
#define CYGARC_REG_CHCR1                0xffffff9c
#define CYGARC_REG_DRCR1                0xfffffe72

#define CYGARC_REG_DMAOR                0xffffffb0

// Offsets from base register
#define CYGARC_REG_SAR                  0x00
#define CYGARC_REG_DAR                  0x04
#define CYGARC_REG_DMATCR               0x08
#define CYGARC_REG_CHCR                 0x0c


// DMA Channel Control Register. If there's a digit suffix to CHCR the flag
// is only valid in the listed channels.
#define CYGARC_REG_CHCR_DM1             0x00008000 // destination address mode
#define CYGARC_REG_CHCR_DM0             0x00004000
#define CYGARC_REG_CHCR_SM1             0x00002000 // source address mode
#define CYGARC_REG_CHCR_SM0             0x00001000
#define CYGARC_REG_CHCR_TS1             0x00000800 // transmit size
#define CYGARC_REG_CHCR_TS0             0x00000400
#define CYGARC_REG_CHCR_AR              0x00000200 // auto-request mode
#define CYGARC_REG_CHCR_AM              0x00000100 // ack/tx mode
#define CYGARC_REG_CHCR_AL              0x00000080 // ack level
#define CYGARC_REG_CHCR_DS              0x00000040 // DREQ select: level(0)/edge(1)
#define CYGARC_REG_CHCR_DL              0x00000020 // DREQ select: low(0)/high(1)
#define CYGARC_REG_CHCR_TB              0x00000010 // transfer bus mode cycle-steal(0)/bus(1)
#define CYGARC_REG_CHCR_TA              0x00000008 // transfer address mode dual(0)/single(1)
#define CYGARC_REG_CHCR_IE              0x00000004 // interrupt enable
#define CYGARC_REG_CHCR_TE              0x00000002 // transfer end
#define CYGARC_REG_CHCR_DE              0x00000001 // DMAC enable

// Resource select options
#define CYGARC_REG_CHCR_RS_EXT          0x00000000
#define CYGARC_REG_CHCR_RS_SCIF1_RXI    0x00000005
#define CYGARC_REG_CHCR_RS_SCIF1_TXI    0x00000006
#define CYGARC_REG_CHCR_RS_SCIF2_RXI    0x00000009
#define CYGARC_REG_CHCR_RS_SCIF2_TXI    0x0000000a
#define CYGARC_REG_CHCR_RS_TPU_TGI0A    0x0000000c
#define CYGARC_REG_CHCR_RS_TPU_TGI0B    0x0000000d
#define CYGARC_REG_CHCR_RS_TPU_TGI0C    0x0000000e
#define CYGARC_REG_CHCR_RS_TPU_TGI0D    0x0000000f
#define CYGARC_REG_CHCR_RS_SIO0_RDIF    0x00000011
#define CYGARC_REG_CHCR_RS_SIO0_TDEI    0x00000012
#define CYGARC_REG_CHCR_RS_SIO1_RDIF    0x00000015
#define CYGARC_REG_CHCR_RS_SIO1_TDEI    0x00000016
#define CYGARC_REG_CHCR_RS_SIO2_RDIF    0x00000019
#define CYGARC_REG_CHCR_RS_SIO2_TDEI    0x0000001a


// DMA Operation Register
#define CYGARC_REG_DMAOR_PR             0x00000008
#define CYGARC_REG_DMAOR_AE             0x00000004     // address error flag
#define CYGARC_REG_DMAOR_NMIF           0x00000002     // NMI flag
#define CYGARC_REG_DMAOR_DME            0x00000001     // DMA master enable
