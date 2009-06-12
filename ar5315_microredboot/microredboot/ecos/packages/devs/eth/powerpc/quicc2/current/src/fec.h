//==========================================================================
//
//      fec.h
//
//      PowerPC MPC8260 fast ethernet (FEC)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    mtek
// Contributors: pfine
// Date:         2002-02-20
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// The port connected to the ethernet
#define QUICC2_VADS_IMM_BASE  0x04700000
#define FCC2  1

/* ------------------------ */
/* FCC REGISTER CONSTANTS   */
/* ------------------------ */

// GFMR masks (RESET: 0x00000000)
#define FEC_GFMR_EN_Rx   0x00000020   // Receive enable  
#define FEC_GFMR_EN_Tx   0x00000010   // Transmit enable
#define FEC_GFMR_INIT    0x0000000C   // mode=ethernet
#define FEC_GFMR_OFFSET  0x11320

//PSMR masks (RESET: 0x00000000)
#define FEC_PSMR_INIT    0x00000080   // 32-bit CRC
#define FEC_PSMR_OFFSET  0x11324

//TODR masks (RESET: 0x0000)
#define FEC_TOD_INIT     0x0000
#define FEC_TOD_SET      0x8000
#define FEC_TOD_OFFSET   0x11328

//DSR masks (RESET: 0x7E7E)
#define FEC_DSR_INIT     0xD555
#define FEC_DSR_OFFSET   0x1132C

//FCCE & FCCM (RESET: 0x0000) 
#define FEC_EV_GRA   0x00800000  // Graceful stop
#define FEC_EV_RXC   0x00400000  // A control frame has been received
#define FEC_EV_TXC   0x00200000  // Out of sequence frame sent 
#define FEC_EV_TXE   0x00100000  // Error in transmission channel
#define FEC_EV_RXF   0x00080000  // A complete frame received
#define FEC_EV_BSY   0x00040000  // A received frame discarded due to lack
                                 // of buffers
#define FEC_EV_TXB   0x00020000  // A buffer sent to ethernet
#define FEC_EV_RXB   0x00010000  // A buffer that is a non-complete frame
                                 // is received
#define FEC_FCCE_OFFSET  0x11330
#define FEC_FCCM_OFFSET  0x11334

/* ------------------------------ */
/* FCC PARAMETER RAM CONSTANTS    */
/* ------------------------------ */

#define FEC_PRAM_RIPTR   0x3000      // 32 byte buffer in dual port RAM
#define FEC_PRAM_TIPTR   0xB000      // 32 byte buffer in dual port RAM
#define FEC_FCR_INIT     0x00000000  // Clear the reserved bits
#define FEC_FCR_MOT_BO   0x10000000  // Motorola byte ordering
#define FEC_PRAM_C_MASK  0xDEBB20E3  // Constant MASK for CRC
#define FEC_PRAM_C_PRES  0xFFFFFFFF  // CRC Preset
#define FEC_PRAM_RETLIM  15          // Retry limit
#define FEC_PRAM_PER_LO  5           // Persistance
#define FEC_PRAM_PER_HI  0       
#define FEC_PRAM_MRBLR   1536    
#define FEC_MAX_FLR      1518        // Max frame length
#define FEC_MIN_FLR      64          // Min frame length
#define FEC_PRAM_PAD_CH  0x8888
#define FEC_PRAM_MAXD    1520
#define FEC_PRAM_OFFSET  0x8500      // Offset of t_Fcc_Pram in 82xx 

/* ------------------------------ */
/* BUFFER DESCRIPTOR CONSTANTS    */
/* ------------------------------ */
#define FEC_PRAM_RxBD_Base   (FEC_PRAM_RIPTR + 0x400)
#define FEC_BD_Rx_Empty      0x8000  // Buffer is empty, FEC can fill
#define FEC_BD_Rx_Wrap       0x2000  // Wrap: Last buffer in ring
#define FEC_BD_Rx_Int        0x1000  // Interrupt
#define FEC_BD_Rx_Last       0x0800  // Last buffer in frame
#define FEC_BD_Rx_Miss       0x0100  // Miss: promiscious mode
#define FEC_BD_Rx_BC         0x0080  // Broadcast address
#define FEC_BD_Rx_MC         0x0040  // Multicast address
#define FEC_BD_Rx_LG         0x0020  // Frame length violation
#define FEC_BD_Rx_NO         0x0010  // Non-octet aligned frame
#define FEC_BD_Rx_SH         0x0008  // Short frame
#define FEC_BD_Rx_CR         0x0004  // CRC error
#define FEC_BD_Rx_OV         0x0002  // Overrun
#define FEC_BD_Rx_TR         0x0001  // Frame truncated. late collision

#define FEC_PRAM_TxBD_Base   (FEC_PRAM_TIPTR + 0x400)
#define FEC_BD_Tx_Ready      0x8000  // Frame ready
#define FEC_BD_Tx_Pad        0x4000  // Pad short frames
#define FEC_BD_Tx_Wrap       0x2000  // Wrap: Last buffer in ring
#define FEC_BD_Tx_Int        0x1000  // Interrupt
#define FEC_BD_Tx_Last       0x0800  // Last buffer in frame
#define FEC_BD_Tx_TC         0x0400  // Send CRC after data
#define FEC_BD_Tx_DEF        0x0200  // Defer indication
#define FEC_BD_Tx_HB         0x0100  // Heartbeat
#define FEC_BD_Tx_LC         0x0080  // Late collision
#define FEC_BD_Tx_RL         0x0040  // Retransmission limit
#define FEC_BD_Tx_RC         0x003C  // Retry count 
#define FEC_BD_Tx_UN         0x0002  // Underrun
#define FEC_BD_Tx_CSL        0x0001  // Carrier sense lost


// Buffer descriptor
struct fec_bd {
    volatile unsigned short  ctrl;
    volatile unsigned short  length;
    volatile unsigned char  *buffer;
};


struct fec_eth_info { 
  volatile struct fcc_regs *fcc_reg;        // See "mpc8260.h"
  struct fec_bd   *txbd, *rxbd;    // Next Tx,Rx descriptor to use
  struct fec_bd   *tbase, *rbase;  // First Tx,Rx descriptor
  struct fec_bd   *tnext, *rnext;  // Next descriptor to check for interrupt
  int                       txsize, rxsize; // Length of individual buffers
  unsigned long             txkey[CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM];
};

// CPM_CPCR masks 
#define CPCR_FLG                0x00010000
#define CPCR_FCC2_CH            0x16200000
#define CPCR_GRSTOP_TX          0x00000005
#define CPCR_INIT_TX_RX_PARAMS  0x00000000
#define CPCR_MCN_FEC            0x00000300
#define CPCR_READY_TO_RX_CMD   0  /* Ready to receive a command */
