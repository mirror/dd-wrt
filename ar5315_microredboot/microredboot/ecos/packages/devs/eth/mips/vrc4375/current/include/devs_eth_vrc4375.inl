//==========================================================================
//
//      devs/eth/mips/vrc4375/..../include/devs_eth_vrc4375.inl
//
//      vrc4375 ethernet I/O definitions.
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
// Contributors: 
// Date:         2001-09-17
// Purpose:      vrc4375 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

// --------------------------------------------------------------
// Construct the interface

#ifdef CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH0

static I21143 i21143_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_MIPS_VRC4375_ETH0_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_MIPS_VRC4375_ETH0_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i21143_sc0,
           &i21143_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_MIPS_VRC4375_ETH0_NAME, // Name for device
           i21143_start,
           i21143_stop,
           i21143_ioctl,
           i21143_can_send,
           i21143_send,
           i21143_recv,
           i21143_deliver,
           i21143_poll,
           i21143_int_vector
    );

NETDEVTAB_ENTRY(i21143_netdev0, 
                "i21143_" CYGDAT_DEVS_ETH_MIPS_VRC4375_ETH0_NAME,
                i21143_init, 
                &i21143_sc0);

#endif // CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH0

// --------------------------------------------------------------
// These arrays are used for sanity checking of pointers
I21143 *
i21143_priv_array[CYGNUM_DEVS_ETH_INTEL_I21143_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH0
    &i21143_eth0_priv_data,
#endif
#ifdef CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH1
    &i21143_eth1_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
i21143_netdev_array[CYGNUM_DEVS_ETH_INTEL_I21143_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH0
    &i21143_netdev0,
#endif
#ifdef CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH1
    &i21143_netdev1,
#endif
};

struct eth_drv_sc *
i21143_sc_array[CYGNUM_DEVS_ETH_INTEL_I21143_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH0
    &i21143_sc0,
#endif
#ifdef CYGPKG_DEVS_ETH_MIPS_VRC4375_ETH1
    &i21143_sc1,
#endif
};
#endif // CYGDBG_USE_ASSERTS

// --------------------------------------------------------------
// Debugging

//#define CYGDBG_DEVS_ETH_INTEL_I21143_CHATTER 1

// --------------------------------------------------------------

// EOF devs_eth_vrc4375.inl
