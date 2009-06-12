//==========================================================================
//
//      quicc_eth.h
//
//      PowerPC QUICC (MPC8xx) ethernet 
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, nickg
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PowerPC QUICC (MPC8xx) Ethernet

#include <cyg/hal/quicc/ppc8xx.h>                  // QUICC structure definitions

struct quicc_eth_info {
    volatile struct ethernet_pram  *pram;            // Parameter RAM pointer
    volatile struct scc_regs       *ctl;             // SCC control registers
    volatile struct cp_bufdesc     *txbd, *rxbd;     // Next Tx,Rx descriptor to use
    struct cp_bufdesc              *tbase, *rbase;   // First Tx,Rx descriptor
    struct cp_bufdesc              *tnext, *rnext;   // Next descriptor to check for interrupt
    int                             txsize, rxsize;  // Length of individual buffers
    int                             txactive;        // Count of active Tx buffers
    unsigned long                   txkey[CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM];

    // Keep some statistics
    cyg_uint32 interrupts;

    cyg_uint32 rx_count;
    cyg_uint32 rx_deliver;
    cyg_uint32 rx_resource;
    cyg_uint32 rx_restart;
    cyg_uint32 rx_good;
    cyg_uint32 rx_crc_errors;
    cyg_uint32 rx_align_errors;
    cyg_uint32 rx_resource_errors;
    cyg_uint32 rx_overrun_errors;
    cyg_uint32 rx_collisions;
    cyg_uint32 rx_short_frames;
    cyg_uint32 rx_long_frames;
    cyg_uint32 rx_miss;

    cyg_uint32 tx_count;
    cyg_uint32 tx_complete;
    cyg_uint32 tx_restart;
    cyg_uint32 tx_good;
    cyg_uint32 tx_dropped;
    cyg_uint32 tx_underrun;
    cyg_uint32 tx_late_collisions;
    cyg_uint32 tx_carrier_loss;
    cyg_uint32 tx_retransmit_error;
    cyg_uint32 tx_heartbeat_loss;
    cyg_uint32 tx_deferred;    
};

// SCC registers - ethernet mode

// General SCC mode register
#define QUICC_SCC_GSMH_IRP          0x00040000  // Infared polarity
#define QUICC_SCC_GSMH_GDE          0x00010000  // Glitch detect enable
#define QUICC_SCC_GSMH_TCRC         0x00008000  // Transparent CRC
#define QUICC_SCC_GSMH_REVD         0x00004000  // Reverse data (transparent)
#define QUICC_SCC_GSMH_TRX          0x00002000  // Transparent Rx
#define QUICC_SCC_GSMH_TTX          0x00001000  // Transparent Tx

#define QUICC_SCC_GSML_TCI          0x10000000  // Transmit clock invert
#define QUICC_SCC_GSML_TPL          0x00E00000  // Tx preamble bits
#define QUICC_SCC_GSML_TPL_8        0x00200000  //    8 bits
#define QUICC_SCC_GSML_TPL_16       0x00400000  //   16 bits
#define QUICC_SCC_GSML_TPL_32       0x00600000  //   32 bits
#define QUICC_SCC_GSML_TPL_48       0x00800000  //   48 bits (used for ethernet)
#define QUICC_SCC_GSML_TPL_64       0x00A00000  //   64 bits
#define QUICC_SCC_GSML_TPL_128      0x00C00000  //  128 bits
#define QUICC_SCC_GSML_TPP          0x00180000  // Tx preamble pattern
#define QUICC_SCC_GSML_TPP_00       0x00000000  //   all zeroes
#define QUICC_SCC_GSML_TPP_01       0x00080000  //   10 repeats (ethernet)
#define QUICC_SCC_GSML_TPP_10       0x00100000  //   01 repeats
#define QUICC_SCC_GSML_TPP_11       0x00180000  //   all ones (localtalk)
#define QUICC_SCC_GSML_ENR          0x00000020  // Enable receiver
#define QUICC_SCC_GSML_ENT          0x00000010  // Enable transmitter
#define QUICC_SCC_GSML_MODE         0x0000000F  // Operating mode
#define QUICC_SCC_GSML_MODE_HDLC    0x00000000
#define QUICC_SCC_GSML_MODE_ATALK   0x00000002
#define QUICC_SCC_GSML_MODE_ENET    0x0000000C

// Function code
#define QUICC_SCC_FCR_BE            0x0010  // Big Endian operation

// Event register
#define QUICC_SCCE_GRC              0x0080  // Gracefull stop complete
#define QUICC_SCCE_TXE              0x0010  // Transmit error
#define QUICC_SCCE_RXF              0x0008  // Received full frame
#define QUICC_SCCE_BSY              0x0004  // No free receive buffers
#define QUICC_SCCE_TX               0x0002  // Buffer transmit complete
#define QUICC_SCCE_RX               0x0001  // Buffer received
#define QUICC_SCCE_INTS (QUICC_SCCE_TXE | QUICC_SCCE_RXF | QUICC_SCCE_TX)

// Protocol specific mode register
#define QUICC_PMSR_HEARTBEAT        0x8000  // Enable heartbeat
#define QUICC_PMSR_FORCE_COLLISION  0x4000  // Force a collision
#define QUICC_PMSR_RCV_SHORT_FRAMES 0x2000  // Accept short frames
#define QUICC_PMSR_INDIV_ADDR_MODE  0x1000  // Check individual address (hash)
#define QUICC_PMSR_ENET_CRC         0x0800  // Enable ethernet CRC mode
#define QUICC_PMSR_PROMISCUOUS      0x0200  // Enable promiscuous mode
#define QUICC_PMSR_BROADCAST        0x0100  // Accept broadcast packets
#define QUICC_PMSR_SPECIAL_BACKOFF  0x0080  // Enable special backoff timer
#define QUICC_PMSR_LOOPBACK         0x0040  // Enable loopback mode
#define QUICC_PMSR_SAMPLE_INPUTS    0x0020  // Discretely look at input pins
#define QUICC_PMSR_LATE_COLLISION   0x0010  // Enable late collision window
#define QUICC_PMSR_SEARCH_AFTER_22  0x000A  // Start frame search after 22 bits
#define QUICC_PMSR_FULL_DUPLEX      0x0001  // Full duplex mode

// Receive buffer status
#define QUICC_BD_RX_LAST            0x0800  // Last buffer in chain
#define QUICC_BD_RX_FIRST           0x0400  // First buffer in chain
#define QUICC_BD_RX_MISS            0x0100  // Missed data
#define QUICC_BD_RX_LG              0x0020  // Rx frame too long
#define QUICC_BD_RX_NO              0x0010  // Rx frame not properly aligned
#define QUICC_BD_RX_SH              0x0008  // Rx frame too short
#define QUICC_BD_RX_CR              0x0004  // Bad CRC
#define QUICC_BD_RX_OV              0x0002  // Rx overrun
#define QUICC_BD_RX_CL              0x0001  // Collision during frame  

#define QUICC_BD_RX_ERRORS          ( QUICC_BD_RX_CL | QUICC_BD_RX_OV | \
                                      QUICC_BD_RX_CR | QUICC_BD_RX_SH | \
                                      QUICC_BD_RX_NO | QUICC_BD_RX_LG | \
                                      QUICC_BD_RX_MISS )

// Transmit buffer status
#define QUICC_BD_TX_PAD             0x4000  // Pad short packets
#define QUICC_BD_TX_LAST            0x0800  // Last buffer in chain
#define QUICC_BD_TX_TC              0x0400  // Transmit CRC after buffer
#define QUICC_BD_TX_DEF             0x0200  // Transmission was deferred
#define QUICC_BD_TX_HB              0x0100  // Heartbeat detected
#define QUICC_BD_TX_LC              0x0080  // Late collision
#define QUICC_BD_TX_RL              0x0040  // Retransmit limit exceeded
#define QUICC_BD_TX_RC              0x003C  // Retry count
#define QUICC_BD_TX_UN              0x0002  // Tx underrun
#define QUICC_BD_TX_CSL             0x0001  // Carrier lost

#define QUICC_BD_TX_ERRORS          (QUICC_BD_TX_CSL | QUICC_BD_TX_UN | \
                                     QUICC_BD_TX_RL | QUICC_BD_TX_LC  | \
                                     QUICC_BD_TX_HB | QUICC_BD_TX_DEF )

#include CYGDAT_DEVS_QUICC_ETH_INL  // Platform specifics

#define IEEE_8023_MAX_FRAME         1518    // Largest possible ethernet frame
#define IEEE_8023_MIN_FRAME           64    // Smallest possible ethernet frame

