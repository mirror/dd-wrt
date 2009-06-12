//==========================================================================
//
//      dev/dp83816.h
//
//      National Semiconductor DP83816 ethernet chip
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Contributors: 
// Date:         2003-09-29
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_io.h>
#include <pkgconf/devs_eth_ns_dp83816.h>

// ------------------------------------------------------------------------
// Debugging details

// Set to perms of:
// 0 disables all debug output
// 1 for process debug output
// 2 for added data IO output: get_reg, put_reg
// 4 for packet allocation/free output
// 8 for only startup status, so we can tell we're installed OK
#define DEBUG 0x0

#if DEBUG & 1
#define DEBUG_FUNCTION() do { diag_printf("%s\n", __FUNCTION__); } while (0)
#define DEBUG_LINE() do { diag_printf("%d\n", __LINE__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#define DEBUG_LINE() do {} while(0)
#endif

// ------------------------------------------------------------------------
// Buffer descriptors
typedef struct dp83816_bd {
    struct dp83816_bd *next;  // Next descriptor
    unsigned long      stat;  // Buffer status & flags
    unsigned char     *buf;   // Buffer memory
    unsigned long      key;   // Internal use only
} dp83816_bd_t;

// ------------------------------------------------------------------------
// Private driver structure
typedef struct dp83816_priv_data {
    char                     *esa_key;        // RedBoot 'key' for device ESA
    unsigned char            *enaddr;
    int                       rxnum;          // Number of Rx buffers
    unsigned char            *rxbuf;          // Rx buffer space
    dp83816_bd_t             *rxd;            // Rx descriptor pool
    int                       txnum;          // Number of Tx buffers
    unsigned char            *txbuf;          // Tx buffer space
    dp83816_bd_t             *txd;            // Tx descriptor pool
    cyg_uint8                *base;
    int                       interrupt;      // Interrupt vector
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    cyg_handle_t  interrupt_handle;
    cyg_interrupt interrupt_object;
#endif
    dp83816_bd_t             *rxnext;         // Next Rx to interrupt
    dp83816_bd_t             *txfill;         // Next Tx to fill
    dp83816_bd_t             *txint;          // Next Tx to interrupt
    int                       txbusy;         // Number of busy Tx buffers 
} dp83816_priv_data_t;

// ------------------------------------------------------------------------
// Macros for accessing DP registers
// These can be overridden by the platform header
// Note: the only accesses used must be 32 bit little endian.

#ifndef DP_IN
# define DP_IN(_b_, _o_, _d_)  HAL_READ_UINT32((cyg_addrword_t)(_b_)+(_o_), (_d_))
# define DP_OUT(_b_, _o_, _d_) HAL_WRITE_UINT32((cyg_addrword_t)(_b_)+(_o_), (_d_))
#endif

// ------------------------------------------------------------------------
// Macros allowing platform to customize some of the driver details

#ifndef CYGHWR_NS_DP83816_PLF_RESET
# define CYGHWR_NS_DP83816_PLF_RESET(_b_) do { } while (0)
#endif

#ifndef CYGHWR_NS_DP83816_PLF_INT_CLEAR
# define CYGHWR_NS_DP83816_PLF_INT_CLEAR(_dp_)
#endif

#ifndef CYGHWR_NS_DP83816_PLF_INIT
#define CYGHWR_NS_DP83816_PLF_INIT(dp) do { } while (0)
#endif


// ------------------------------------------------------------------------
// Some forward declarations
static void dp83816_poll(struct eth_drv_sc *sc);

// ------------------------------------------------------------------------
// Register offsets

#define DP_CR          0x00  // Command register
#define   _CR_RST         0x100  // Chip reset
#define   _CR_RXR         0x020  // Rx reset
#define   _CR_TXR         0x010  // Tx reset
#define   _CR_RXD         0x008  // Rx disable
#define   _CR_RXE         0x004  // Rx enable
#define   _CR_TXD         0x002  // Tx disable
#define   _CR_TXE         0x001  // Tx enable
#define DP_CFG         0x04  // Configuration register
#define   _CFG_LNKSTS  (1<<31) // Link status
#define   _CFG_SPD100  (1<<30) // 100Mb 
#define   _CFG_FDUP    (1<<29) // Full duplex
#define   _CFG_POL     (1<<28) // 10Mb polarity
#define   _CFG_ANDONE  (1<<27) // Auto-negotiation done
#define DP_ISR         0x10  // Interrupt status
#define   _ISR_TXRCMP (1<<25) // Tx reset complete
#define   _ISR_RXRCMP (1<<24) // Rx reset complete
#define   _ISR_DPERR  (1<<23) // Detected parity error
#define   _ISR_SSERR  (1<<22) // Signalled system error
#define   _ISR_RMABT  (1<<21) // Received master abort
#define   _ISR_RTABT  (1<<20) // Received target abort
#define   _ISR_RXSOVR (1<<16) // Rx status FIFO overrun
#define   _ISR_HIBERR (1<<15) // High bits set (25-16)
#define   _ISR_PHY    (1<<14) // PHY interrupt
#define   _ISR_PME    (1<<13) // Power management event
#define   _ISR_SWI    (1<<12) // Software interrpt
#define   _ISR_MIB    (1<<11) // MII service 
#define   _ISR_TXURN  (1<<10) // Tx underrun
#define   _ISR_TXIDLE  (1<<9) // Tx idle (end of list)
#define   _ISR_TXERR   (1<<8) // Tx packet error
#define   _ISR_TXDESC  (1<<7) // Tx descriptor with INTS
#define   _ISR_TXOK    (1<<6) // Last Tx descriptor done
#define   _ISR_RXORN   (1<<5) // Rx overrun
#define   _ISR_RXIDLE  (1<<4) // Rx idle (end of list)
#define   _ISR_RXEARLY (1<<3) // Rx early threshold met
#define   _ISR_RXERR   (1<<2) // Rx packet error
#define   _ISR_RXDESC  (1<<1) // Rx descriptor with INTS
#define   _ISR_RXOK    (1<<0) // Last Rx descriptor done
#define DP_IMR         0x14  // Interrupt mask
#define DP_IER         0x18  // Interrupt enable
#define DP_IHR         0x1C  // Interrupt hold
#define DP_TXDP        0x20  // Tx descriptor pointer
#define DP_TXCFG       0x24  // Tx configuration
#define   _TXCFG_CSI    (1<<31) // Ignore carrier sense
#define   _TXCFG_HBI    (1<<30) // Ignore heartbeat
#define   _TXCFG_MLB    (1<<29) // Loopback
#define   _TXCFG_ATP    (1<<28) // Automatic padding
#define   _TXCFG_ECRTRY (1<<23) // Excessive collision enable
#define   _TXCFG_MXDMA_SHIFT 20
#define   _TXCFG_MXDMA_MASK 0x7
#define   _TXCFG_MXDMA_512  (0x0<<20)
#define   _TXCFG_MXDMA_4    (0x1<<20)
#define   _TXCFG_MXDMA_8    (0x2<<20)
#define   _TXCFG_MXDMA_16   (0x3<<20)
#define   _TXCFG_MXDMA_32   (0x4<<20)
#define   _TXCFG_MXDMA_64   (0x5<<20)
#define   _TXCFG_MXDMA_128  (0x6<<20)
#define   _TXCFG_MXDMA_256  (0x7<<20)
#define   _TXCFG_FLTH_SHIFT   8
#define   _TXCFG_FLTH_MASK 0x3F
#define   _TXCFG_DRTH_SHIFT   0
#define   _TXCFG_DRTH_MASK 0x3F
#define DP_RXDP        0x30  // Rx descriptor pointer
#define DP_RXCFG       0x34  // Rx configuration
#define   _RXCFG_AEP    (1<<31) // Accept error packets
#define   _RXCFG_ARP    (1<<30) // Accept runt packets
#define   _RXCFG_ATX    (1<<28) // Accept Tx packets (loopback)
#define   _RXCFG_ALP    (1<<27) // Accpet long packets (> 1518 bytes)
#define   _RXCFG_MXDMA_SHIFT 20
#define   _RXCFG_MXDMA_MASK 0x7
#define   _RXCFG_MXDMA_512  (0x0<<20)
#define   _RXCFG_MXDMA_4    (0x1<<20)
#define   _RXCFG_MXDMA_8    (0x2<<20)
#define   _RXCFG_MXDMA_16   (0x3<<20)
#define   _RXCFG_MXDMA_32   (0x4<<20)
#define   _RXCFG_MXDMA_64   (0x5<<20)
#define   _RXCFG_MXDMA_128  (0x6<<20)
#define   _RXCFG_MXDMA_256  (0x7<<20)
#define   _RXCFG_FLTH_SHIFT   8
#define   _RXCFG_FLTH_MASK 0x3F
#define   _RXCFG_DRTH_SHIFT   0
#define   _RXCFG_DRTH_MASK 0x3F
#define DP_RFCR        0x48  // Receive filter control
#define   _RFCR_RFEN  (1<<31) // Rx filter enable
#define   _RFCR_AAB   (1<<30) // Accept all broadcast
#define   _RFCR_AAM   (1<<29) // Accept all multicast
#define   _RFCR_AAU   (1<<28) // Accept all unicast
#define   _RFCR_APM   (1<<27) // Accept on perfect match
#define   _RFCR_APAT  (1<<26) // Accept on patern match
#define   _RFCR_AARP  (1<<22) // Accept ARP
#define   _RFCR_MHEN  (1<<21) // Multicast hash enable
#define   _RFCR_UHEN  (1<<20) // Unicast hash enable
#define   _RFCR_ULM   (1<<19) // U/L bit ignore
#define DP_RFDR        0x4C  // Receive filter data register

// Buffer descriptor status/flags
#define BD_OWN        (1<<31) // Owned by producer (Tx=driver, Rx=DP83816)
#define BD_MORE       (1<<30) // More descriptors in this frame
#define BD_INTR       (1<<29) // Interrupt when this descriptor processed
#define BD_CRC        (1<<28) // Include CRC
#define BD_OK         (1<<27) // Packet OK
// Tx buffer flags
#define BD_TXA        (1<<26) // Tx abort
#define BD_TFU        (1<<25) // Tx underrun
#define BD_CRS        (1<<24) // Carrier sense lost
#define BD_TD         (1<<23) // Transmission deferred
#define BD_ED         (1<<22) // Excessive Tx deferrals
#define BD_OWC        (1<<21) // Out of window collision
#define BD_EC         (1<<20) // Excessive collisions
#define BD_CCNT_MASK    0x0F
#define BD_CCNT_SHIFT     16  // Collision count
// Rx buffer flags
#define BD_RXA         (1<<26) // Rx abort
#define BD_RXO         (1<<25) // Rx overrun
#define BD_DEST_MASK   (3<<23)
#define BD_DEST_REJECT    (0<<23) // Packet rejected
#define BD_DEST_UNICAST   (1<<23) // Unicast packet
#define BD_DEST_MULTICAST (2<<23) // Multicast packet
#define BD_DEST_BROADCAST (3<<23) // Broadcast packet
#define BD_LONG        (1<<22) // Too long packet received
#define BD_RUNT        (1<<21) // Runt packet
#define BD_ISE         (1<<20) // Illegal symbol
#define BD_CRCE        (1<<19) // CRC error
#define BD_FAE         (1<<18) // Frame alignment error
#define BD_LBP         (1<<17) // Loopback frame
#define BD_COL         (1<<16) // Collision during frame
// Length field
#define BD_LENGTH_MASK 0x0FFF

#define IEEE_8023_MAX_FRAME         1518    // Largest possible ethernet frame
#define IEEE_8023_MIN_FRAME           64    // Smallest possible ethernet frame

#define _DP83816_BUFSIZE            1540    // Size of buffers
