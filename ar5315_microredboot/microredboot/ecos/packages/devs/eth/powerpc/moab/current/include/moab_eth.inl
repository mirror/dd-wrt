#ifndef CYGONCE_DEVS_MOAB_ETH_INL
#define CYGONCE_DEVS_MOAB_ETH_INL
//==========================================================================
//
//      moab_eth.inl
//
//      Hardware specifics for TAMS MOAB ethernet support
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
// Contributors: gthomas
// Date:         2003-08-19
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include CYGDAT_DEVS_PPC405_ETH_CDL

#ifdef CYGHWR_DEVS_ETH_POWERPC_MOAB_ETH0
ETH_PHY_REG_LEVEL_ACCESS_FUNS(eth0_phy, 
                              ppc405_eth_phy_init,
                              NULL,
                              ppc405_eth_phy_put_reg,
                              ppc405_eth_phy_get_reg);

// Align buffers on a cache boundary
#define RxBUFSIZE CYGNUM_DEVS_ETH_POWERPC_PPC405_RxNUM*CYGNUM_DEVS_ETH_POWERPC_PPC405_BUFSIZE
#define TxBUFSIZE CYGNUM_DEVS_ETH_POWERPC_PPC405_TxNUM*CYGNUM_DEVS_ETH_POWERPC_PPC405_BUFSIZE
static unsigned char ppc405_eth_rxbufs[RxBUFSIZE] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static unsigned char ppc405_eth_txbufs[TxBUFSIZE] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static mal_bd_t ppc405_eth_rxbd[CYGNUM_DEVS_ETH_POWERPC_PPC405_RxNUM] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static mal_bd_t ppc405_eth_txbd[CYGNUM_DEVS_ETH_POWERPC_PPC405_TxNUM] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));

extern char _moab_eth0_ESA[];
static struct ppc405_eth_info ppc405_eth0_info = {
    0,                                     // Interrupt vector
    "eth0_esa",
    _moab_eth0_ESA,
    CYGNUM_DEVS_ETH_POWERPC_PPC405_RxNUM,  // Number of Rx buffers
    ppc405_eth_rxbufs,                     // Rx buffer space
    ppc405_eth_rxbd,                       // Rx buffer headers
    CYGNUM_DEVS_ETH_POWERPC_PPC405_TxNUM,  // Number of Tx buffers
    ppc405_eth_txbufs,                     // Tx buffer space
    ppc405_eth_txbd,                       // Tx buffer headers
    &eth0_phy,                             // PHY access routines
};

ETH_DRV_SC(ppc405_eth0_sc,
           &ppc405_eth0_info,  // Driver specific data
           "eth0",             // Name for this interface
           ppc405_eth_start,
           ppc405_eth_stop,
           ppc405_eth_control,
           ppc405_eth_can_send,
           ppc405_eth_send,
           ppc405_eth_recv,
           ppc405_eth_deliver,
           ppc405_eth_int,
           ppc405_eth_int_vector);

NETDEVTAB_ENTRY(ppc405_netdev, 
                "eth0", 
                ppc405_eth_init, 
                &ppc405_eth0_sc);

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("eth0 network hardware address [MAC]",
                      eth0_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, _moab_eth0_ESA
    );
#endif // CYGSEM_REDBOOT_FLASH_CONFIG
#endif // CYGPKG_REDBOOT

#endif // CYGHWR_DEVS_ETH_POWERPC_MOAB_ETH0

#endif  // CYGONCE_DEVS_MOAB_ETH_INL
// ------------------------------------------------------------------------
