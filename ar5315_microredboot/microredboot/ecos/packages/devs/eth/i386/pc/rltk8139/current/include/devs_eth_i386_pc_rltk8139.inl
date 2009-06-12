//==========================================================================
//
//      devs/eth/i386/pc/rltk8139/include/devs_eth_i386_pc_rltk8139.inl
//
//      i386 PC RealTek 8139 ethernet I/O definitions.
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
// Author(s):   Eric Doenges
// Contributors:
// Date:        2003-07-16
// Purpose:     i386 PC RealTek 8139 ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

// 
// CAUTION! This driver has *not* been tested on PC hardware.  It may work
// or not :-)  If there are problems, they are probably cache related.
// If you find such, please let us know.
//
//#define CYGPKG_DEVS_ETH_RLTK_8139_SOFTWARE_CACHE_COHERENCY

#define CACHE_ALIGNED __attribute__ ((aligned (HAL_DCACHE_LINE_SIZE)))

#ifdef CYGPKG_DEVS_ETH_I386_PC_RLTK8139_ETH0

static cyg_uint8 rltk8139_eth0_rx_ring[RX_BUF_TOT_LEN] CACHE_ALIGNED;
static cyg_uint8
rltk8139_eth0_tx_buffer[(TX_BUF_TOT_LEN + HAL_DCACHE_LINE_SIZE - 1)
                        & ~(HAL_DCACHE_LINE_SIZE - 1)] CACHE_ALIGNED;
static Rltk8139_t rltk8139_eth0_priv_data = {
  0, rltk8139_eth0_rx_ring, rltk8139_eth0_tx_buffer
};

ETH_DRV_SC(rltk8139_sc0,
           &rltk8139_eth0_priv_data,
           CYGDAT_DEVS_ETH_I386_PC_RLTK8139_ETH0_NAME,
           rltk8139_start,
           rltk8139_stop,
           rltk8139_control,
           rltk8139_can_send,
           rltk8139_send,
           rltk8139_recv,
           rltk8139_deliver,
           rltk8139_poll,
           rltk8139_int_vector
           );

NETDEVTAB_ENTRY(rltk8139_netdev0,
                "rltk8139_" CYGDAT_DEVS_ETH_I386_PC_RLTK8139_ETH0_NAME,
                rltk8139_init,
                &rltk8139_sc0);

#endif // CYGPKG_DEVS_ETH_I386_PC_RLTK8139_ETH0

// EOF devs_eth_i386_pc_rltk8139.inl
