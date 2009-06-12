#ifndef CYGONCE_DEVS_ETH_INTEL_I82544_INFO_H
#define CYGONCE_DEVS_ETH_INTEL_I82544_INFO_H
/*==========================================================================
//
//        i82544_info.h
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):     hmt
// Contributors:  hmt
// Date:          2000-05-03
// Description:
//
//####DESCRIPTIONEND####
*/

#include <pkgconf/devs_eth_intel_i82544.h>

#ifdef CYGDBG_DEVS_ETH_INTEL_I82544_KEEP_STATISTICS
# define KEEP_STATISTICS
# define nDISPLAY_STATISTICS
# define nDISPLAY_82544_STATISTICS
#else
# define nKEEP_STATISTICS
# define nDISPLAY_STATISTICS
# define nDISPLAY_82544_STATISTICS
#endif


// ------------------------------------------------------------------------
//
//                       STATISTICAL COUNTER STRUCTURE
//
// ------------------------------------------------------------------------
#ifdef KEEP_STATISTICS
typedef struct {
/*  0 */    cyg_uint32 tx_good;
/*  4 */    cyg_uint32 tx_max_collisions;
/*  8 */    cyg_uint32 tx_late_collisions;
/* 12 */    cyg_uint32 tx_underrun;
/* 16 */    cyg_uint32 tx_carrier_loss;
/* 20 */    cyg_uint32 tx_deferred;
/* 24 */    cyg_uint32 tx_single_collisions;
/* 28 */    cyg_uint32 tx_mult_collisions;
/* 32 */    cyg_uint32 tx_total_collisions;
/* 36 */    cyg_uint32 rx_good;
/* 40 */    cyg_uint32 rx_crc_errors;
/* 44 */    cyg_uint32 rx_align_errors;
/* 48 */    cyg_uint32 rx_resource_errors;
/* 52 */    cyg_uint32 rx_overrun_errors;
/* 56 */    cyg_uint32 rx_collisions; // Always 0
/* 60 */    cyg_uint32 rx_short_frames;
// In this setup; can also be flow-control counts after.
// If these are to be used, a config command (as in set promiscuous mode)
// must be issued at start, to let those stats escape.  Params are in
// comments around the config command setup...
/* 64 */    cyg_uint32 done;
} I82544_COUNTERS;


typedef struct {
    cyg_uint32 interrupts;
    cyg_uint32 rx_count;
    cyg_uint32 rx_deliver;
    cyg_uint32 rx_resource;
    cyg_uint32 rx_restart;
    cyg_uint32 tx_count;
    cyg_uint32 tx_complete;
    cyg_uint32 tx_dropped;
} STATISTICS;


extern STATISTICS statistics[CYGNUM_DEVS_ETH_INTEL_I82544_DEV_COUNT];
#ifdef CYGDBG_DEVS_ETH_INTEL_I82544_KEEP_82544_STATISTICS
extern I82544_COUNTERS i82544_counters[CYGNUM_DEVS_ETH_INTEL_I82544_DEV_COUNT];
#endif

#endif // KEEP_STATISTICS

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------
// The system seems to work OK with as few as 8 of RX and TX descriptors.
// It limps very painfully with only 4.
// Performance is better with more than 8.
// But the size of non-cached (so useless for anything else)
// memory window is 1Mb, so we might as well use it all.
//
// 128 for these uses the whole 1Mb, near enough.

#ifndef MAX_RX_DESCRIPTORS
#define MAX_RX_DESCRIPTORS	128     // number of Rx descriptors
#endif
#ifndef MAX_TX_DESCRIPTORS
#define MAX_TX_DESCRIPTORS	128     // number of Tx descriptors
#endif

typedef struct i82544 {
    cyg_uint8                           // (split up for atomic byte access)
        found:1,                        // was hardware discovered?
        mac_addr_ok:1,                  // can we bring up?
        active:1,                       // has this if been brung up?
        hardwired_esa:1,                // set if ESA is hardwired via CDL
        link:1,                         // set if link is up
        spare1:3; 
    cyg_uint8                           // Count nested sends to reject
        within_send:8;                  //     nested requests to send
    cyg_uint8
        tx_in_progress:1,               // transmit in progress flag
        tx_queue_full:1,                // all Tx descriptors used flag
        spare3:6; 

    cyg_uint8  index;                   // 0 or 1 or whatever
    cyg_uint32 devid;                   // PCI device id
    cyg_uint32 device;                  // Device code from hardware
    cyg_uint32 io_address;              // memory mapped I/O address
    cyg_uint8  mac_address[6];          // mac (hardware) address
    void *ndp;                          // Network Device Pointer

    cyg_int32 next_rx_descriptor;       // descriptor index for callback
    cyg_int32 rx_pointer;               // descriptor index for ring head
    CYG_ADDRESS rx_ring;                // location of Rx descriptors

    cyg_int32 tx_pointer;               // next TXB to check for status.
    CYG_ADDRESS tx_ring;                // location of Tx descriptors
    unsigned long tx_keys[MAX_TX_DESCRIPTORS]; // keys for tx q management

    // Interrupt handling stuff
    cyg_vector_t    vector;             // interrupt vector
    cyg_handle_t    interrupt_handle;   // handle for int.handler
    cyg_interrupt   interrupt_object;

#ifdef KEEP_STATISTICS
    void *p_statistics;                 // pointer to statistical counters
#endif

    cyg_uint32 platform_timeout;        // Some platforms use a timeout
    int tx_descriptor_timeout;          // Is it fixated on this tx?

} I82544;


// ------------------------------------------------------------------------
//
//                   82544 GENERAL STATUS REGISTER
//
// ------------------------------------------------------------------------
#define GEN_STATUS_FDX          0x01    // 1 = full duplex, 0 = half
#define GEN_STATUS_BPS          0xC0    // 0 = 10M, 01 = 100M, 10&11 = 1000M
#define GEN_STATUS_BPS_SHIFT    6  
#define GEN_STATUS_LINK         0x02    // 1 = link up, 0 = link down

extern int i82544_status( struct eth_drv_sc *sc );

// ------------------------------------------------------------------------

#ifdef KEEP_STATISTICS
void update_statistics(struct i82544* p_i82544);
#endif


#ifdef CYGDBG_DEVS_ETH_INTEL_I82544_KEEP_82544_STATISTICS
#define ETH_STATS_INIT( p ) \
        update_statistics( (struct i82544 *)((p)->driver_private) )
#else
#define ETH_STATS_INIT( p ) // otherwise do nothing
#endif

#define CYGDAT_DEVS_ETH_DESCRIPTION "Intel Gigabit Ethernet Controller (i82544)"

#define ETH_DEV_DOT3STATSETHERCHIPSET 1,3,6,1,2,1,10,7,8,2,5

#endif /* ifndef CYGONCE_DEVS_ETH_INTEL_I82544_INFO_H */

/* EOF i82544_info.h */

