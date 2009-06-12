//==========================================================================
//
//      devs/eth/arm/flexanet/..../include/devs_eth_flexanet.inl
//
//      Flexanet ethernet I/O definitions.
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
// Author(s):    Jordi Colomer <jco@ict.es>
// Contributors: Jordi Colomer
// Date:         2001-06-18
// Purpose:      Flexanet ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_arm_flexanet.h>
#include <cyg/hal/hal_intr.h>          
#include <cyg/hal/flexanet.h>

#define CYGNUM_DEVS_ETH_SMSC_LAN91CXX_SHIFT_ADDR    2

// MAC address is stored as a Redboot config option
#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>

RedBoot_config_option("Network hardware address [MAC]",
                      flexanet_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif
#endif

// ESA address fetch function
static void flexanet_get_ESA(struct lan91cxx_priv_data *cpd)
{
    // Fetch hardware address from RedBoot config
#if defined(CYGSEM_DEVS_ETH_ARM_FLEXANET_REDBOOT_ESA)
#if defined(CYGPKG_REDBOOT) && \
    defined(CYGSEM_REDBOOT_FLASH_CONFIG)
    flash_get_config("flexanet_esa", cpd->enaddr, CONFIG_ESA);
#else
#error "No RedBoot flash configuration to store ESA"
#endif
#else
    memcpy(cpd->enaddr, static_esa, 6);
#endif
}

static lan91cxx_priv_data lan91cxx_eth0_priv_data = { 

    config_enaddr : flexanet_get_ESA,
#ifndef CYGSEM_DEVS_ETH_ARM_FLEXANET_REDBOOT_ESA    
    enaddr: CYGDAT_DEVS_ETH_ARM_FLEXANET_ESA,
#endif
    base : (unsigned short *) SA1110_FHH_ETH_IOBASE,
    attbase : (unsigned char *) SA1110_FHH_ETH_MMBASE,
    interrupt : SA1110_IRQ_GPIO_ETH
};

ETH_DRV_SC(lan91cxx_sc,
           &lan91cxx_eth0_priv_data,          // Driver specific data
           CYGDAT_DEVS_ETH_ARM_FLEXANET_NAME, // Name for device
           lan91cxx_start,
           lan91cxx_stop,
           lan91cxx_control,
           lan91cxx_can_send,
           lan91cxx_send,
           lan91cxx_recv,
           lan91cxx_deliver,
           lan91cxx_poll,
           lan91cxx_int_vector
);

NETDEVTAB_ENTRY(lan91cxx_netdev, 
                "lan91cxx_" CYGDAT_DEVS_ETH_ARM_FLEXANET_NAME,
                smsc_lan91cxx_init,
                &lan91cxx_sc);

//EOF devs_eth_flexanet.inl


