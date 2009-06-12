//==========================================================================
//
//      grg_i82559.inl
//
//      GRG ethernet I/O definitions.
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
// Date:        2003-02-05
// Purpose:     grg ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHERNET
#include <cyg/hal/hal_cache.h>          // HAL_DCACHE_LINE_SIZE
#include <cyg/hal/plf_io.h>             // CYGARC_UNCACHED_ADDRESS

// Bus hardware takes care of IO endianess issues.
#define CYGHWR_DEVS_ETH_INTEL_I82559_ENDIAN_NEUTRAL_IO 1

// NB: Bus masters can only get to first 64MB SDRAM
#define CYGHWR_INTEL_I82559_PCI_VIRT_TO_BUS( _x_ ) \
                 ((cyg_uint32)CYGARC_VIRT_TO_BUS(_x_))

#define MAX_PACKET_SIZE   1536
#define SIZEOF_DESCRIPTOR 16
#define MISC_MEM          1128     // selftest, ioctl and statistics

#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE \
  (((MAX_PACKET_SIZE + SIZEOF_DESCRIPTOR) * \
     (CYGNUM_DEVS_ETH_INTEL_I82559_MAX_TX_DESCRIPTORS + \
      CYGNUM_DEVS_ETH_INTEL_I82559_MAX_RX_DESCRIPTORS)) + \
   MISC_MEM)*CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT

// The PCI Window is 64MB starting at 0x00000000, so as long as the bss
// section is within addresses 0x00000000-0x04000000, pci_mem_buffer[] will be
// in the true PCI Window.  Because pci_mem_buffer[] is in the bss section,
// wanted its size to be only as large as required so as to not waste memory.
// Therefore, had to compute it's size in this file and not in the memory
// layout files.  So no __pci_window label was ever defined, therefore,
// CYGMEM_SECTION_pci_window does not exist.  Define CYGMEM_SECTION_pci_window
// here in order to satisfy the ASSERT within if_i82559.c.
static char pci_mem_buffer[CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE 
                           + HAL_DCACHE_LINE_SIZE];

#define CYGMEM_SECTION_pci_window (((unsigned)pci_mem_buffer         \
                                       + HAL_DCACHE_LINE_SIZE - 1)   \
                                   & ~(HAL_DCACHE_LINE_SIZE - 1))  

#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE \
  (CYGARC_UNCACHED_ADDRESS(CYGMEM_SECTION_pci_window))


#ifdef CYGPKG_DEVS_ETH_ARM_GRG_I82559_ETH0

static I82559 i82559_eth0_priv_data = { 
    hardwired_esa: 0,
#if defined(CYGPKG_REDBOOT) && defined(CYGVAR_DEVS_ETH_I82559_ETH_REDBOOT_HOLDS_ESA_ETH0)
    mac_address: CYGDAT_DEVS_ETH_ARM_GRG_ETH0_DEFAULT_ESA
#endif
};

ETH_DRV_SC(i82559_sc0,
           &i82559_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_GRG_I82559_ETH0_NAME, // Name for device
           i82559_start,
           i82559_stop,
           i82559_ioctl,
           i82559_can_send,
           i82559_send,
           i82559_recv,
           i82559_deliver,
           i82559_poll,
           i82559_int_vector
    );

NETDEVTAB_ENTRY(i82559_netdev0, 
                "i82559_" CYGDAT_DEVS_ETH_ARM_GRG_I82559_ETH0_NAME,
                i82559_init, 
                &i82559_sc0);
#endif // CYGPKG_DEVS_ETH_ARM_IXP425_I82559_ETH0


// These arrays are used for sanity checking of pointers
I82559 *
i82559_priv_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_GRG_I82559_ETH0
    &i82559_eth0_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
i82559_netdev_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_GRG_I82559_ETH0
    &i82559_netdev0,
#endif
};

struct eth_drv_sc *
i82559_sc_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_GRG_I82559_ETH0
    &i82559_sc0,
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

#ifdef CYGVAR_DEVS_ETH_I82559_ETH_REDBOOT_HOLDS_ESA_ETH0
RedBoot_config_option("Network hardware address [MAC] for eth0",
                      eth0_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, i82559_eth0_priv_data.mac_address
    );
#endif

#endif  // CYGPKG_REDBOOT_NETWORKING
#endif  // CYGSEM_REDBOOT_FLASH_CONFIG
#endif  // CYGPKG_REDBOOT

// and initialization code to read them
// - independent of whether we are building RedBoot right now:
#ifdef CYGPKG_DEVS_ETH_I82559_ETH_REDBOOT_HOLDS_ESA

#include <cyg/hal/hal_if.h>

#ifndef CONFIG_ESA
#define CONFIG_ESA (6)
#endif

#define CYGHWR_DEVS_ETH_INTEL_I82559_GET_ESA( p_i82559, mac_address, ok )       \
CYG_MACRO_START                                                                 \
    ok = false;                                                                 \
    if ( 0 == p_i82559->index )                                                 \
        ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,         \
                                          "eth0_esa", mac_address, CONFIG_ESA); \
    else if ( 1 == p_i82559->index )                                            \
        ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,         \
                                          "eth1_esa", mac_address, CONFIG_ESA); \
CYG_MACRO_END

#endif // CYGPKG_DEVS_ETH_I82559_ETH_REDBOOT_HOLDS_ESA
