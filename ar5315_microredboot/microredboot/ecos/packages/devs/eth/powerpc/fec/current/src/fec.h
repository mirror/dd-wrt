//==========================================================================
//
//      fec.h
//
//      PowerPC MPC8xxT fast ethernet (FEC)
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
// Date:         2001-01-21
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PowerPC FEC (MPC8xxT) Fast Ethernet

// Buffer descriptor
struct fec_bd {
    unsigned short  ctrl;
    unsigned short  length;
    unsigned char  *buffer;
};

// control flags differ for Rx and Tx buffers
#define FEC_BD_Rx_Empty  0x8000  // Buffer is empty [FEC can fill it]
#define FEC_BD_Rx_Wrap   0x2000  // Last buffer in ring [wrap]
#define FEC_BD_Rx_Last   0x0800  // Last buffer in frame
#define FEC_BD_Rx_Miss   0x0100  // 
#define FEC_BD_Rx_BC     0x0080
#define FEC_BD_Rx_MC     0x0040
#define FEC_BD_Rx_LG     0x0020
#define FEC_BD_Rx_NO     0x0010
#define FEC_BD_Rx_SH     0x0008  // Short frame
#define FEC_BD_Rx_CR     0x0004  // CRC error
#define FEC_BD_Rx_OV     0x0002  // Overrun
#define FEC_BD_Rx_TR     0x0001  // Frame truncated

#define FEC_BD_Tx_Ready  0x8000  // Frame ready
#define FEC_BD_Tx_Wrap   0x2000  // Last buffer in ring
#define FEC_BD_Tx_Intr   0x1000  // Generate interrupt
#define FEC_BD_Tx_Last   0x0800  // Last buffer in frame
#define FEC_BD_Tx_TC     0x0400  // Send CRC after data
#define FEC_BD_Tx_DEF    0x0200
#define FEC_BD_Tx_HB     0x0100
#define FEC_BD_Tx_LC     0x0080
#define FEC_BD_Tx_RL     0x0040
#define FEC_BD_Tx_RC     0x003C
#define FEC_BD_Tx_UN     0x0002  // Underrun
#define FEC_BD_Tx_CSL    0x0001  // Carrier sense lost

#define FEC_BD_Tx_STATS  0x03FF  // Status mask

struct fec_eth_info {
    volatile struct fec        *fec;
    volatile struct fec_bd     *txbd, *rxbd;     // Next Tx,Rx descriptor to use
    volatile struct fec_bd     *tbase, *rbase;   // First Tx,Rx descriptor
    volatile struct fec_bd     *tnext, *rnext;   // Next descriptor to check for interrupt
    int                         txsize, rxsize;  // Length of individual buffers
    int                         txactive;        // Count of active Tx buffers
    unsigned long               txkey[CYGNUM_DEVS_ETH_POWERPC_FEC_TxNUM];
};

// Fast Ethernet Controller [in PPC8xxT parameter RAM space]

struct fec {
    unsigned long  addr[2];    // ESA
    unsigned long  hash[2];    // Address hash mask
    volatile struct fec_bd *RxRing;
    volatile struct fec_bd *TxRing;
    unsigned long  RxBufSize;
    unsigned char  _fill0[0x40-0x1C];
    unsigned long  eControl;   // Master control register
    unsigned long  iEvent;     // Interrupt event
    unsigned long  iMask;      // Interrupt mask
    unsigned long  iVector;    // Interrupt vector
    unsigned long  RxUpdate;   // RxRing updated
    unsigned long  TxUpdate;   // TxRing updated
    unsigned char  _fill1[0x80-0x58];
    unsigned long  MiiData;
    unsigned long  MiiSpeed;
    unsigned char  _fill2[0xCC-0x88];
    unsigned long  RxBound;    // End of FIFO RAM
    unsigned long  RxStart;    // Start of FIFO RAM
    unsigned char  _fill3[0xE4-0xD4];
    unsigned long  TxWater;    // Transmit watermark
    unsigned char  _fill4[0xEC-0xE8];
    unsigned long  TxStart;    // Start of Tx FIFO
    unsigned char  _fill5[0x134-0xF0];
    unsigned long  FunCode;    // DMA function codes
    unsigned char  _fill6[0x144-0x138];
    unsigned long  RxControl;  // Receiver control
    unsigned long  RxHash;     // Receive hash
    unsigned char  _fill7[0x184-0x14C];
    unsigned long  TxControl;  // Transmitter control
};

#define FEC_OFFSET 0x0E00      // Offset in 8xx parameter RAM

// Master control register (eControl)
#define eControl_MUX    0x0004 // Select proper pin MUX functions
#define eControl_EN     0x0002 // Enable ethernet controller
#define eControl_RESET  0x0001 // Reset controller

// Receiver control register (RxControl)
#define RxControl_BC_REJ 0x0010 // Reject broadcast frames
#define RxControl_PROM   0x0008 // Promiscuous mode
#define RxControl_MII    0x0004 // MII (1) or 7 wire (0) mode 
#define RxControl_DRT    0x0002 // Disable receive on transmit
#define RxControl_LOOP   0x0001 // Internal loopback

// Interrupt events
#define iEvent_HBERR         0x80000000  // No heartbeat error
#define iEvent_BABR          0x40000000  // Babling receiver
#define iEvent_BABT          0x20000000  // Babling transmitter
#define iEvent_GRA           0x10000000  // Graceful shutdown
#define iEvent_TFINT         0x08000000  // Transmit frame interrupt
#define iEvent_TXB           0x04000000  // Transmit buffer
#define iEvent_RFINT         0x02000000  // Receive frame
#define iEvent_RXB           0x01000000  // Receive buffer
#define iEvent_MII           0x00800000  // MII complete
#define iEvent_EBERR         0x00400000  // Ethernet BUS error
#define iEvent_all           0xFFC00000  // Any interrupt

// MII interface
#define MII_Start            0x40000000
#define MII_Read             0x20000000
#define MII_Write            0x10000000
#define MII_Phy(phy)         (phy << 23)
#define MII_Reg(reg)         (reg << 18)
#define MII_TA               0x00020000

// Transceiver mode
#define PHY_BMCR             0x00    // Register number
#define PHY_BMCR_RESET       0x8000
#define PHY_BMCR_LOOPBACK    0x4000
#define PHY_BMCR_100MB       0x2000
#define PHY_BMCR_AUTO_NEG    0x1000
#define PHY_BMCR_POWER_DOWN  0x0800
#define PHY_BMCR_ISOLATE     0x0400
#define PHY_BMCR_RESTART     0x0200
#define PHY_BMCR_FULL_DUPLEX 0x0100
#define PHY_BMCR_COLL_TEST   0x0080

#define PHY_BMSR             0x01    // Status register
#define PHY_BMSR_AUTO_NEG    0x0020  
#define PHY_BMSR_LINK        0x0004

#define IEEE_8023_MAX_FRAME         1518    // Largest possible ethernet frame
#define IEEE_8023_MIN_FRAME           60    // Smallest possible ethernet frame

