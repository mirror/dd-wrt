//==========================================================================
//
//      devs/eth/frv/pdk403/..../include/devs_eth_pdk403.inl
//
//      PDK403 ethernet I/O definitions.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):    jskov, hmt, gthomas
// Contributors: jskov
// Date:         2001-02-28
// Purpose:      PDK403 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR
#include <cyg/hal/hal_if.h>

#ifdef __WANT_CONFIG

#define CYGHWR_NS_DP83902A_PLF_16BIT_DATA
#define CYGHWR_NS_DP83902A_PLF_BROKEN_RX_DMA

#define CYGHWR_NS_DP83902A_PLF_INIT(dp) cyg_ax88796_eth_init(dp)

#ifndef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED

#define CYGHWR_NS_DP83902A_PLF_INT_CLEAR(dp) \
		cyg_drv_interrupt_acknowledge((dp)->interrupt)
#endif

#endif // __WANT_CONFIG

#ifdef __WANT_DEVS

#if defined(CYGSEM_DEVS_ETH_PDK403_ETH0_SET_ESA) 
#if defined(CYGPKG_REDBOOT) 
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("Network hardware address [MAC]",
                      lan_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif  // CYGSEM_REDBOOT_FLASH_CONFIG
#endif  // CYGPKG_REDBOOT
#include <cyg/hal/hal_if.h>
#ifndef CONFIG_ESA
#define CONFIG_ESA 6
#endif
#endif

extern void cyg_ax88796_eth_init(dp83902a_priv_data_t *dp);


#ifdef CYGPKG_DEVS_ETH_PDK403_ETH0

static dp83902a_priv_data_t dp83902a_eth0_priv_data = { 
    base : (cyg_uint8*) 0x10000200,  //
    data : (cyg_uint8*) 0x10000210,  // Filled in at runtime
    reset: (cyg_uint8*) 0x1000021f,  //
    interrupt: CYGNUM_HAL_INTERRUPT_LAN,           //
    tx_buf1: 0x40,          // 
    tx_buf2: 0x48,          // Buffer layout - change with care
    rx_buf_start: 0x50,     //
    rx_buf_end: 0x80,       //
#ifdef CYGSEM_DEVS_ETH_PDK403_ETH0_SET_ESA
    esa : CYGDAT_DEVS_ETH_PDK403_ETH0_ESA,
    hardwired_esa : true,
#else
    hardwired_esa : false,
#endif
};

ETH_DRV_SC(dp83902a_sc,
           &dp83902a_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_PDK403_ETH0_NAME,
           dp83902a_start,
           dp83902a_stop,
           dp83902a_control,
           dp83902a_can_send,
           dp83902a_send,
           dp83902a_recv,
           dp83902a_deliver,     // "pseudoDSR" called from fast net thread
           dp83902a_poll,        // poll function, encapsulates ISR and DSR
           dp83902a_int_vector);

NETDEVTAB_ENTRY(dp83902a_netdev, 
                "dp83902a_" CYGDAT_DEVS_ETH_PDK403_ETH0_NAME,
                dp83902a_init, 
                &dp83902a_sc);

#endif // CYGPKG_DEVS_ETH_PDK403_ETH0

#endif // __WANT_DEVS

// --------------------------------------------------------------

// EOF devs_eth_pdk403.inl
