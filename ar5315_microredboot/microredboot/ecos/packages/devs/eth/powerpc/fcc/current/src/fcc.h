//==========================================================================
//
//      fcc.h
//
//      PowerPC MPC8xxx fast ethernet (FCC)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003, 2004 Gary Thomas
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
// Contributors: pfine, mtek
// Date:         2003-08-19
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_eth_powerpc_fcc.h>
// The port connected to the ethernet
#define FCC1  0
#define FCC2  1
#define FCC3  2

/* ------------------------ */
/* FCC REGISTER CONSTANTS   */
/* ------------------------ */

// GFMR masks (RESET: 0x00000000)
#define FCC_GFMR_EN_Rx   0x00000020   // Receive enable  
#define FCC_GFMR_EN_Tx   0x00000010   // Transmit enable
#define FCC_GFMR_INIT    0x0000000C   // mode=ethernet

//PSMR masks (RESET: 0x00000000)
#define FCC_PSMR_INIT    0x00000080   // 32-bit CRC

//TODR masks (RESET: 0x0000)
#define FCC_TOD_INIT     0x0000
#define FCC_TOD_SET      0x8000

//DSR masks (RESET: 0x7E7E)
#define FCC_DSR_INIT     0xD555

//FCCE & FCCM (RESET: 0x0000) 
#define FCC_EV_GRA   0x0080  // Graceful stop
#define FCC_EV_RXC   0x0040  // A control frame has been received
#define FCC_EV_TXC   0x0020  // Out of sequence frame sent 
#define FCC_EV_TXE   0x0010  // Error in transmission channel
#define FCC_EV_RXF   0x0008  // A complete frame received
#define FCC_EV_BSY   0x0004  // A received frame discarded due to lack
                             // of buffers
#define FCC_EV_TXB   0x0002  // A buffer sent to ethernet
#define FCC_EV_RXB   0x0001  // A buffer that is a non-complete frame
                             // is received

/* ------------------------------ */
/* FCC PARAMETER RAM CONSTANTS    */
/* ------------------------------ */

#define FCC_FCR_INIT     0x00000000  // Clear the reserved bits
#define FCC_FCR_MOT_BO   0x10000000  // Motorola byte ordering
#define FCC_PRAM_C_MASK  0xDEBB20E3  // Constant MASK for CRC
#define FCC_PRAM_C_PRES  0xFFFFFFFF  // CRC Preset
#define FCC_PRAM_RETLIM  15          // Retry limit
#define FCC_PRAM_PER_LO  5           // Persistance
#define FCC_PRAM_PER_HI  0       
#define FCC_PRAM_MRBLR   1536    
#define FCC_MAX_FLR      1518        // Max frame length
#define FCC_MIN_FLR      64          // Min frame length
#define FCC_PRAM_PAD_CH  0x8888
#define FCC_PRAM_MAXD    1520
#define FCC1_PRAM_OFFSET  0x8400      // Offset of t_Fcc_Pram in 82xx 
#define FCC2_PRAM_OFFSET  0x8500      // Offset of t_Fcc_Pram in 82xx 
#define FCC3_PRAM_OFFSET  0x8600      // Offset of t_Fcc_Pram in 82xx

/* ------------------------------ */
/* BUFFER DESCRIPTOR CONSTANTS    */
/* ------------------------------ */
#define FCC_BD_Rx_Empty      0x8000  // Buffer is empty, FCC can fill
#define FCC_BD_Rx_Wrap       0x2000  // Wrap: Last buffer in ring
#define FCC_BD_Rx_Int        0x1000  // Interrupt
#define FCC_BD_Rx_Last       0x0800  // Last buffer in frame
#define FCC_BD_Rx_Miss       0x0100  // Miss: promiscious mode
#define FCC_BD_Rx_BC         0x0080  // Broadcast address
#define FCC_BD_Rx_MC         0x0040  // Multicast address
#define FCC_BD_Rx_LG         0x0020  // Frame length violation
#define FCC_BD_Rx_NO         0x0010  // Non-octet aligned frame
#define FCC_BD_Rx_SH         0x0008  // Short frame
#define FCC_BD_Rx_CR         0x0004  // CRC error
#define FCC_BD_Rx_OV         0x0002  // Overrun
#define FCC_BD_Rx_TR         0x0001  // Frame truncated. late collision
#define FCC_BD_Rx_ERRORS     (FCC_BD_Rx_LG|FCC_BD_Rx_NO|FCC_BD_Rx_SH|FCC_BD_Rx_CR|FCC_BD_Rx_OV|FCC_BD_Rx_TR)

#define FCC_BD_Tx_Ready      0x8000  // Frame ready
#define FCC_BD_Tx_Pad        0x4000  // Pad short frames
#define FCC_BD_Tx_Wrap       0x2000  // Wrap: Last buffer in ring
#define FCC_BD_Tx_Int        0x1000  // Interrupt
#define FCC_BD_Tx_Last       0x0800  // Last buffer in frame
#define FCC_BD_Tx_TC         0x0400  // Send CRC after data
#define FCC_BD_Tx_DEF        0x0200  // Defer indication
#define FCC_BD_Tx_HB         0x0100  // Heartbeat
#define FCC_BD_Tx_LC         0x0080  // Late collision
#define FCC_BD_Tx_RL         0x0040  // Retransmission limit
#define FCC_BD_Tx_RC         0x003C  // Retry count 
#define FCC_BD_Tx_UN         0x0002  // Underrun
#define FCC_BD_Tx_CSL        0x0001  // Carrier sense lost
#define FCC_BD_Tx_ERRORS     (FCC_BD_Tx_LC|FCC_BD_Tx_RL|FCC_BD_Tx_RC|FCC_BD_Tx_UN|FCC_BD_Tx_CSL)


// Buffer descriptor
struct fcc_bd {
    volatile unsigned short  ctrl;
    volatile unsigned short  length;
    volatile unsigned char  *buffer;
};

//
// Info kept about each interface
//
struct fcc_eth_info { 
    // These fields should be defined by the implementation
    int                       int_vector;
    char                     *esa_key;        // RedBoot 'key' for device ESA
    unsigned char             enaddr[6];
    int                       rxnum;          // Number of Rx buffers
    unsigned char            *rxbuf;          // Rx buffer space
    int                       txnum;          // Number of Tx buffers
    unsigned char            *txbuf;          // Tx buffer space
#ifdef CYGPKG_DEVS_ETH_PHY
    eth_phy_access_t         *phy;            // Routines to access PHY
#endif
    // The rest of the structure is set up at runtime
    volatile struct fcc_regs *fcc_reg;        // See "mpc8260.h"
    struct fcc_bd            *txbd, *rxbd;    // Next Tx,Rx descriptor to use
    struct fcc_bd            *tbase, *rbase;  // First Tx,Rx descriptor
    struct fcc_bd            *tnext, *rnext;  // Next descriptor to check for interrupt
    int                       txsize, rxsize; // Length of individual buffers
    unsigned long             txkey[CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM];
#ifdef CYGPKG_NET
    cyg_interrupt             fcc_eth_interrupt;
    cyg_handle_t              fcc_eth_interrupt_handle;
#endif
};

// CPM_CPCR masks 
#define CPCR_GRSTOP_TX          0x00000005
#define CPCR_MCN_FCC            0x00000300
#define CPCR_READY_TO_RX_CMD   0  /* Ready to receive a command */
