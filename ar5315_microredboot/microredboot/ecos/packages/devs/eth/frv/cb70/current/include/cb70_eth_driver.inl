//==========================================================================
//
//      cb70_eth_drivers.inl
//
//      cb70's DM9000 ethernet I/O definitions.
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
// Author(s):   
// Contributors:msalter
// Date:        2004-03-22
// Purpose:     dm9000 ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHERNET
#include <cyg/hal/hal_cache.h>          // HAL_DCACHE_LINE_SIZE
#include <cyg/hal/plf_io.h>             // CYGARC_UNCACHED_ADDRESS

extern int cyg_hal_dm9000_present(void);

#define CYG_HAL_DM9000_PRESENT() cyg_hal_dm9000_present()

#ifdef CYGPKG_DEVS_ETH_FRV_CB70_ETH0

static struct dm9000 dm9000_eth0_priv_data = {
#if defined(CYGPKG_REDBOOT) && defined(CYGVAR_ETH_DM9000_REDBOOT_HOLDS_ESA_ETH0)
    mac_address: CYGDAT_DEVS_ETH_FRV_CB70_ETH0_DEFAULT_ESA,
#endif
    io_addr: (volatile unsigned char *)0xf0600300,
    io_data: (volatile unsigned char *)0xf0600304
};

ETH_DRV_SC(dm9000_sc0,
           &dm9000_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_FRV_CB70_ETH0_NAME, // Name for device
           dm9000_start,
           dm9000_stop,
           dm9000_ioctl,
           dm9000_can_send,
           dm9000_send,
           dm9000_recv,
           dm9000_deliver,
           dm9000_poll,
           dm9000_int_vector
    );

NETDEVTAB_ENTRY(dm9000_netdev0, 
                "dm9000_" CYGDAT_DEVS_ETH_FRV_CB70_ETH0_NAME,
                dm9000_init, 
                &dm9000_sc0);
#endif // CYGPKG_DEVS_ETH_FRV_CB70_ETH0


// These arrays are used for sanity checking of pointers
struct dm9000 *
dm9000_priv_array[CYGNUM_DEVS_ETH_DAVICOM_DM9000_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_FRV_CB70_ETH0
    &dm9000_eth0_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
dm9000_netdev_array[CYGNUM_DEVS_ETH_DAVICOM_DM9000_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_FRV_CB70_ETH0
    &dm9000_netdev0,
#endif
};

struct eth_drv_sc *
dm9000_sc_array[CYGNUM_DEVS_ETH_DAVICOM_DM9000_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_FRV_CB70_ETH0
    &dm9000_sc0,
#endif
};
#endif // CYGDBG_USE_ASSERTS

// --------------------------------------------------------------
// RedBoot configuration options for managing ESAs for us

// Decide whether to have redboot config vars for it...
#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#ifdef CYGPKG_REDBOOT_NETWORKING
#include <redboot.h>
#include <flash_config.h>

#ifdef CYGVAR_DEVS_ETH_DM9000_REDBOOT_HOLDS_ESA_ETH0
RedBoot_config_option("Network hardware address [MAC] for eth0",
                      eth0_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, dm9000_eth0_priv_data.mac_address
    );
#endif

#endif  // CYGPKG_REDBOOT_NETWORKING
#endif  // CYGSEM_REDBOOT_FLASH_CONFIG
#endif  // CYGPKG_REDBOOT

// and initialization code to read them
// - independent of whether we are building RedBoot right now:
#ifdef CYGPKG_DEVS_ETH_DM9000_ETH_REDBOOT_HOLDS_ESA

#include <cyg/hal/hal_if.h>

#ifndef CONFIG_ESA
#define CONFIG_ESA (6)
#endif

#define CYGHWR_DEVS_ETH_DAVICOM_DM9000_GET_ESA( p_dm9000, mac_address, ok )       \
CYG_MACRO_START                                                                 \
    ok = false;                                                                 \
    if ( 0 == p_dm9000->index )                                                 \
        ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,         \
                                          "eth0_esa", mac_address, CONFIG_ESA); \
    else if ( 1 == p_dm9000->index )                                            \
        ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,         \
                                          "eth1_esa", mac_address, CONFIG_ESA); \
CYG_MACRO_END

#endif // CYGPKG_DEVS_ETH_DM9000_ETH_REDBOOT_HOLDS_ESA
