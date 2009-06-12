//==========================================================================
//
//      ppc405.h
//
//      PowerPC PPC405GP Ethernet
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
// Contributors: gthomas
// Date:         2003-08-15
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PowerPC PPC405 Ethernet

//
// Ethernet MAC controller registers
//
#define EMAC0_MR0   *(volatile unsigned long *)0xEF600800
#define EMAC0_MR1   *(volatile unsigned long *)0xEF600804
#define EMAC0_TMR0  *(volatile unsigned long *)0xEF600808
#define EMAC0_TMR1  *(volatile unsigned long *)0xEF60080C
#define EMAC0_RMR   *(volatile unsigned long *)0xEF600810
#define EMAC0_ISR   *(volatile unsigned long *)0xEF600814
#define EMAC0_ISER  *(volatile unsigned long *)0xEF600818
#define EMAC0_IAHR  *(volatile unsigned long *)0xEF60081C
#define EMAC0_IALR  *(volatile unsigned long *)0xEF600820
#define EMAC0_VTPID *(volatile unsigned long *)0xEF600824
#define EMAC0_VTCI  *(volatile unsigned long *)0xEF600828
#define EMAC0_PRT   *(volatile unsigned long *)0xEF60082C
#define EMAC0_IAHT1 *(volatile unsigned long *)0xEF600830
#define EMAC0_IAHT2 *(volatile unsigned long *)0xEF600834
#define EMAC0_IAHT3 *(volatile unsigned long *)0xEF600838
#define EMAC0_IAHT4 *(volatile unsigned long *)0xEF60083C
#define EMAC0_GAHT1 *(volatile unsigned long *)0xEF600840
#define EMAC0_GAHT2 *(volatile unsigned long *)0xEF600844
#define EMAC0_GAHT3 *(volatile unsigned long *)0xEF600848
#define EMAC0_GAHT4 *(volatile unsigned long *)0xEF60084C
#define EMAC0_LSAH  *(volatile unsigned long *)0xEF600850
#define EMAC0_LSAL  *(volatile unsigned long *)0xEF600854
#define EMAC0_IPGVR *(volatile unsigned long *)0xEF600858
#define EMAC0_STACR *(volatile unsigned long *)0xEF60085C
#define EMAC0_TRTR  *(volatile unsigned long *)0xEF600860
#define EMAC0_RWMR  *(volatile unsigned long *)0xEF600864
#define EMAC0_OCTX  *(volatile unsigned long *)0xEF600868
#define EMAC0_OCRX  *(volatile unsigned long *)0xEF60086C

//
// Mode Register 0
//
#define EMAC0_MR0_RXI  0x80000000  // RX MAC Idle (1 = RX is idle)
#define EMAC0_MR0_TXI  0x40000000  // TX MAC Idle (1 = TX is idle)
#define EMAC0_MR0_SRST 0x20000000
#define EMAC0_MR0_TXE  0x10000000
#define EMAC0_MR0_RXE  0x08000000
#define EMAC0_MR0_WKE  0x04000000

//
// Mode Register 1
//
#define EMAC0_MR1_FDE  0x80000000
#define EMAC0_MR1_ILE  0x40000000
#define EMAC0_MR1_VLE  0x20000000
#define EMAC0_MR1_EIFC 0x10000000
#define EMAC0_MR1_APP  0x08000000
#define EMAC0_MR1_IST  0x01000000
#define EMAC0_MR1_MF   0x00C00000
#define EMAC0_MR1_MF_10MB  0x00000000
#define EMAC0_MR1_MF_100MB 0x00400000
#define EMAC0_MR1_RFS  0x00300000
#define EMAC0_MR1_RFS_512  0x00000000
#define EMAC0_MR1_RFS_1024 0x00100000
#define EMAC0_MR1_RFS_2048 0x00200000
#define EMAC0_MR1_RFS_4096 0x00300000
#define EMAC0_MR1_TFS  0x000C0000
#define EMAC0_MR1_TFS_1024 0x00040000
#define EMAC0_MR1_TFS_2048 0x00080000
#define EMAC0_MR1_TR0  0x00018000
#define EMAC0_MR1_TR0_SINGLE 0x00000000
#define EMAC0_MR1_TR0_MULTI  0x00008000
#define EMAC0_MR1_TR0_DEP    0x00010000
#define EMAC0_MR1_TR1  0x00006000
#define EMAC0_MR1_TR1_SINGLE 0x00000000
#define EMAC0_MR1_TR1_MULTI  0x00002000
#define EMAC0_MR1_TR1_DEP    0x00004000

//
// Transmit mode register 0
//
#define EMAC0_TMR0_GNP0  0x80000000
#define EMAC0_TMR0_GNP1  0x40000000
#define EMAC0_TMR0_GNPD  0x20000000
#define EMAC0_TMR0_FC    0x10000000

//
// Transmit mode register 1
//
#define EMAC0_TMR1_TLR   0xF8000000
#define EMAC0_TMR1_TLR_SHIFT (32-5)
#define EMAC0_TMR1_TUR   0x00FF0000
#define EMAC0_TMR1_TUR_SHIFT (32-16)

//
// Receive mode register
//
#define EMAC0_RMR_SP   0x80000000
#define EMAC0_RMR_SFCS 0x40000000
#define EMAC0_RMR_RRP  0x20000000
#define EMAC0_RMR_RFP  0x10000000
#define EMAC0_RMR_ROP  0x08000000
#define EMAC0_RMR_RPIR 0x04000000
#define EMAC0_RMR_PPP  0x02000000
#define EMAC0_RMR_PME  0x01000000
#define EMAC0_RMR_PMME 0x00800000
#define EMAC0_RMR_IAE  0x00400000
#define EMAC0_RMR_MIAE 0x00200000
#define EMAC0_RMR_BAE  0x00100000
#define EMAC0_RMR_MAE  0x00080000

//
// Interrupt status
//
#define EMAC0_ISR_OVR  0x02000000
#define EMAC0_ISR_PP   0x01000000
#define EMAC0_ISR_BP   0x00800000
#define EMAC0_ISR_RP   0x00400000
#define EMAC0_ISR_SE   0x00200000
#define EMAC0_ISR_ALE  0x00100000
#define EMAC0_ISR_BFCS 0x00080000
#define EMAC0_ISR_PTLE 0x00040000
#define EMAC0_ISR_ORE  0x00020000
#define EMAC0_ISR_IRE  0x00010000
#define EMAC0_ISR_DBDM 0x00000200
#define EMAC0_ISR_DB0  0x00000100
#define EMAC0_ISR_SE0  0x00000080
#define EMAC0_ISR_TE0  0x00000040
#define EMAC0_ISR_DB1  0x00000020
#define EMAC0_ISR_SE1  0x00000010
#define EMAC0_ISR_TE1  0x00000008
#define EMAC0_ISR_MOS  0x00000002
#define EMAC0_ISR_MOF  0x00000001

//
// Interrupt status enable - same as interrupt status
//

//
// STA control register - MII interface
//
#define EMAC0_STACR_PHYD 0xFFFF0000
#define EMAC0_STACR_PHYD_SHIFT (32-16)
#define EMAC0_STACR_OC   0x00008000
#define EMAC0_STACR_PHYE 0x00004000
#define EMAC0_STACR_STAC 0x00003000
#define EMAC0_STACR_STAC_READ  0x00001000
#define EMAC0_STACR_STAC_WRITE 0x00002000
#define EMAC0_STACR_OPBC 0x00000C00
#define EMAC0_STACR_OPBC_50    0x00000000
#define EMAC0_STACR_OPBC_66    0x00000400
#define EMAC0_STACR_OPBC_83    0x00000800
#define EMAC0_STACR_OPBC_100   0x00000C00
#define EMAC0_STACR_PCDA 0x000003E0
#define EMAC0_STACR_PCDA_SHIFT (32-27)
#define EMAC0_STACR_PRA  0x0000001F

//
// Transmit request threshold
//
#define EMAC0_TRTR_TRT   0xF8000000   // 0=64, 1=128, 2=192, etc
#define EMAC0_TRTR_TRT_SHIFT (32-5)
#define EMAC0_TRTR_TRT_SCALE 64     

//
// Receive high/low water marks
//
#define EMAC0_RWMR_RLWM 0xFF800000
#define EMAC0_RWMR_RLWM_SHIFT (32-9)
#define EMAC0_RWMR_RHWM 0x0000FF80
#define EMAC0_RWMR_RHWM_SHIFT (32-25)

//
// Memory Access Layer (MAL) - in DCR space
//
#define MAL0_CFG      0x180
#define MAL0_ESR      0x181
#define MAL0_IER      0x182
#define MAL0_TXCASR   0x184
#define MAL0_TXCARR   0x185
#define MAL0_TXEOBISR 0x186
#define MAL0_TXDEIR   0x187
#define MAL0_RXCASR   0x190
#define MAL0_RXCARR   0x191
#define MAL0_RXEOBISR 0x192
#define MAL0_RXDEIR   0x193
#define MAL0_TXCTP0R  0x1A0
#define MAL0_TXCTP1R  0x1A1
#define MAL0_RXCTP0R  0x1C0
#define MAL0_RXBS0    0x1E0

//
// MAL configuration
//
#define MAL_CFG_SR    0x80000000
#define MAL_CFG_PLBP  0x00C00000
#define MAL_CFG_PLBP_0 0x00000000
#define MAL_CFG_PLBP_1 0x00400000
#define MAL_CFG_PLBP_2 0x00800000
#define MAL_CFG_PLBP_3 0x00C00000
#define MAL_CFG_GA    0x00200000
#define MAL_CFG_OA    0x00100000
#define MAL_CFG_PLBLE 0x00080000
#define MAL_CFG_PLBLT 0x00078000
#define MAL_CFG_PLBLT_SHIFT (32-17)
#define MAL_CFG_PLBT_DEFAULT (0x07<<MAL_CFG_PLBLT_SHIFT)
#define MAL_CFG_PLBB  0x00004000
#define MAL_CFG_OPBBL 0x00000080
#define MAL_CFG_EOPIE 0x00000004
#define MAL_CFG_LEA   0x00000002
#define MAL_CFG_SD    0x00000001

//
// Channel active set/reset
//
#define MAL_CASR_C0   0x80000000
#define MAL_CASR_C1   0x40000000

//
// Error and interrupt status
//
#define MAL_ESR_EVB   0x80000000
#define MAL_ESR_CID   0x7E000000
#define MAL_ESR_CID_SHIFT (32-7)
#define MAL_ESR_DE    0x00100000
#define MAL_ESR_ONE   0x00080000
#define MAL_ESR_OTE   0x00040000
#define MAL_ESR_OSE   0x00020000
#define MAL_ESR_PEIN  0x00010000
#define MAL_ESR_DEI   0x00000010
#define MAL_ESR_ONEI  0x00000008
#define MAL_ESR_OTEI  0x00000004
#define MAL_ESR_OSEI  0x00000002
#define MAL_ESR_PBEI  0x00000001
#define MAL_ESR_INT_MASK 0x0000001F


//
// MAL Buffer Descriptor
//
typedef struct mal_bd {
    unsigned short status;
    unsigned short length;
    unsigned long  buffer;
} mal_bd_t;

//
// Status flags
//
#define MAL_BD_R   0x8000
#define MAL_BD_W   0x4000
#define MAL_BD_CM  0x2000
#define MAL_BD_L   0x1000
#define MAL_BD_F   0x0800
#define MAL_BD_I   0x0400
// EMAC TX bits (command - set before activating buffer)
#define MAL_BD_TX_GFCS 0x0200
#define MAL_BD_TX_GPAD 0x0100
#define MAL_BD_TX_ISA  0x0080
#define MAL_BD_TX_RSA  0x0040
#define MAL_BD_TX_IVLA 0x0020
#define MAL_BD_TX_RVLA 0x0010
// EMAC TX bits (status - valid after buffer completes)
#define MAL_BD_TX_BFCS 0x0200
#define MAL_BD_TX_BPP  0x0100
#define MAL_BD_TX_LOC  0x0080
#define MAL_BD_TX_EDEF 0x0040
#define MAL_BD_TX_ECOL 0x0020
#define MAL_BD_TX_LATE 0x0010
#define MAL_BD_TX_MULT 0x0008
#define MAL_BD_TX_SNGL 0x0004
#define MAL_BD_TX_URUN 0x0002
#define MAL_BD_TX_SQE  0x0001
// EMAC RX bits (only after buffer completes)
#define MAL_BD_RX_ORUN 0x0200
#define MAL_BD_RX_PP   0x0100
#define MAL_BD_RX_BP   0x0080
#define MAL_BD_RX_RP   0x0040
#define MAL_BD_RX_SE   0x0020
#define MAL_BD_RX_ALE  0x0010
#define MAL_BD_RX_BFCS 0x0008
#define MAL_BD_RX_PTL  0x0004
#define MAL_BD_RX_ORNG 0x0002
#define MAL_BD_RX_IRNG 0x0001

//
// Private information kept about interface
//
struct ppc405_eth_info {
    // These fields should be defined by the implementation
    int                 int_vector;
    char               *esa_key;        // RedBoot 'key' for device ESA
    unsigned char      *enaddr;
    int                 rxnum;          // Number of Rx buffers
    unsigned char      *rxbuf;          // Rx buffer space
    mal_bd_t           *rxbd_table;     // Rx buffer headers
    int                 txnum;          // Number of Tx buffers
    unsigned char      *txbuf;          // Tx buffer space
    mal_bd_t           *txbd_table;     // Tx buffer headers
    eth_phy_access_t   *phy;            // Routines to access PHY
    // The following fields are maintained by the driver
    volatile mal_bd_t  *txbd, *rxbd;     // Next Tx,Rx descriptor to use
    volatile mal_bd_t  *tbase, *rbase;   // First Tx,Rx descriptor
    volatile mal_bd_t  *tnext, *rnext;   // Next descriptor to check for interrupt
    int                 txsize, rxsize;  // Length of individual buffers
    int                 txactive;        // Count of active Tx buffers
    unsigned long       txkey[CYGNUM_DEVS_ETH_POWERPC_PPC405_TxNUM];
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    unsigned long       ints;            // Mask of interrupts in progress
#endif
};

