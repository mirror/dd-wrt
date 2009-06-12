//==========================================================================
//
//      moab_eth_dp83816.inl
//
//      MOAB ethernet I/O definitions.
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
// Purpose:      MOAB ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci.h>

#undef DP_IN
#undef DP_OUT
#define DP_IN(_b_, _o_, _d_)  HAL_READ_UINT32LE((cyg_addrword_t)(_b_)+(_o_), (_d_))
#define DP_OUT(_b_, _o_, _d_) HAL_WRITE_UINT32LE((cyg_addrword_t)(_b_)+(_o_), (_d_))

static cyg_bool
find_rtl8381x_match_func(cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p)
{
    return ((v == 0x100B) && (d == 0x0020));
}

static void
_moab_eth_init(dp83816_priv_data_t *dp)
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;

    devid = CYG_PCI_NULL_DEVID;
    if (cyg_pci_find_matching( &find_rtl8381x_match_func, NULL, &devid )) {
        cyg_pci_get_device_info(devid, &dev_info);
        cyg_pci_translate_interrupt(&dev_info, &dp->interrupt);
        dp->base = (cyg_uint8 *)(dev_info.base_map[0] & ~1);
        diag_printf("DP83816 at %p, interrupt: %x\n", dp->base, dp->interrupt);
    }
}

#undef  CYGHWR_NS_DP83816_PLF_INIT
#define CYGHWR_NS_DP83816_PLF_INIT(dp) _moab_eth_init(dp)

// Map a 32 bit host quantity to little endian
unsigned long
_h2le(unsigned long val)
{
    unsigned long res; 
    unsigned long *addr = &val;
    __asm__ __volatile__ ("lwbrx %0,0,%1" : "=r" (res) : "r" (addr), "m" (*addr));
    return res;
}

// Map a 32 bit little endian quantity to host representation
unsigned long
_le2h(unsigned long val)
{
    unsigned long res; 
    unsigned long *addr = &val;
    __asm__ __volatile__ ("lwbrx %0,0,%1" : "=r" (res) : "r" (addr), "m" (*addr));
    return res;
}

// Align buffers on a cache boundary
#define RxBUFSIZE CYGNUM_DEVS_ETH_MOAB_DP83816_RxNUM*_DP83816_BUFSIZE
#define TxBUFSIZE CYGNUM_DEVS_ETH_MOAB_DP83816_TxNUM*_DP83816_BUFSIZE
static unsigned char dp83816_eth_rxbufs[RxBUFSIZE] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static unsigned char dp83816_eth_txbufs[TxBUFSIZE] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static dp83816_bd_t dp83816_eth_rxbd[CYGNUM_DEVS_ETH_MOAB_DP83816_RxNUM] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static dp83816_bd_t dp83816_eth_txbd[CYGNUM_DEVS_ETH_MOAB_DP83816_TxNUM] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));

extern char _moab_eth1_ESA[];
static dp83816_priv_data_t dp83816_eth1_priv_data = { 
    "eth1_esa",
    _moab_eth1_ESA,
    CYGNUM_DEVS_ETH_MOAB_DP83816_RxNUM,    // Number of Rx buffers
    dp83816_eth_rxbufs,                    // Rx buffer space
    dp83816_eth_rxbd,                      // Rx buffer headers
    CYGNUM_DEVS_ETH_MOAB_DP83816_TxNUM,    // Number of Tx buffers
    dp83816_eth_txbufs,                    // Tx buffer space
    dp83816_eth_txbd,                      // Tx buffer headers
};

ETH_DRV_SC(dp83816_sc,
           &dp83816_eth1_priv_data, // Driver specific data
           "eth1",
           dp83816_start,
           dp83816_stop,
           dp83816_control,
           dp83816_can_send,
           dp83816_send,
           dp83816_recv,
           dp83816_deliver,     // "pseudoDSR" called from fast net thread
           dp83816_poll,        // poll function, encapsulates ISR and DSR
           dp83816_int_vector);

NETDEVTAB_ENTRY(dp83816_netdev, 
                "eth1",
                dp83816_init, 
                &dp83816_sc);

#if defined(CYGPKG_REDBOOT) 
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("eth1 network hardware address [MAC]",
                      eth1_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, _moab_eth1_ESA
    );
#endif  // CYGSEM_REDBOOT_FLASH_CONFIG
#else
#ifndef CONFIG_ESA
#define CONFIG_ESA 6
#endif
#endif  // CYGPKG_REDBOOT


// EOF moab_eth_dp83816.inl
