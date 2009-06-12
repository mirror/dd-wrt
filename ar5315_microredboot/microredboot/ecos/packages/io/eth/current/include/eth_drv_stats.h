#ifndef CYGONCE_IO_ETH_ETH_DRV_STATS_H
#define CYGONCE_IO_ETH_ETH_DRV_STATS_H
//==========================================================================
//
//      include/cyg/io/eth/eth_drv_stats.h
//
//      High level networking driver interfaces - statistics gathering
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
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-08-23
// Purpose:      
// Description:  High level networking driver interfaces - stats gathering
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>

#ifdef CYGPKG_NET
#include <sys/param.h>
#include <sys/socket.h>

#include <net/if.h>

// This information is oriented towards SNMP's needs.

#define DESC_LEN (48)
#define SNMP_CHIPSET_LEN (20)

struct ether_drv_stats {
    struct ifreq ifreq;                 // tell ioctl() which interface.

    char description[ DESC_LEN ];       // Textual description of hardware
    unsigned char snmp_chipset[ SNMP_CHIPSET_LEN ];
                                        // SNMP ID of chipset
    unsigned char duplex;               // 1 = UNKNOWN, 2 = SIMPLEX, 3 = DUPLEX
    unsigned char operational;          // 1 = UNKNOWN, 2 = DOWN, 3 = UP
    // These are general status information:
    unsigned int speed;                 // 10,000,000 or 100,000,000
                                        //     to infinity and beyond?

    // These are typically kept by device hardware - and there may be some
    // cost for getting up to date values:

    unsigned int supports_dot3;  /* Boolean value if the device supports dot3*/
    unsigned int tx_good;
    unsigned int tx_max_collisions;
    unsigned int tx_late_collisions;
    unsigned int tx_underrun;
    unsigned int tx_carrier_loss;
    unsigned int tx_deferred;
    unsigned int tx_sqetesterrors;
    unsigned int tx_single_collisions;
    unsigned int tx_mult_collisions;
    unsigned int tx_total_collisions;
    unsigned int rx_good;
    unsigned int rx_crc_errors;
    unsigned int rx_align_errors;
    unsigned int rx_resource_errors;
    unsigned int rx_overrun_errors;
    unsigned int rx_collisions;
    unsigned int rx_short_frames;
    unsigned int rx_too_long_frames;
    unsigned int rx_symbol_errors;

    // These are typically kept by driver software:
    unsigned int interrupts;
    unsigned int rx_count;
    unsigned int rx_deliver;
    unsigned int rx_resource;
    unsigned int rx_restart;
    unsigned int tx_queue_len;
    unsigned int tx_count;
    unsigned int tx_complete;
    unsigned int tx_dropped;

    // Add any others here...

};
#endif // CYGPKG_NET
#endif // CYGONCE_IO_ETH_ETH_DRV_STATS_H

// EOF include/cyg/io/eth/eth_drv_stats.h
