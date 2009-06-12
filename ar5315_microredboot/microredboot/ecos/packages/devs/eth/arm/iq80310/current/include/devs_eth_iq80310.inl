//==========================================================================
//
//      devs/eth/arm/iq80310/include/devs_eth_arm_iq80310.inl
//
//      IQ80310 ethernet I/O definitions.
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
// Author(s):   msalter
// Contributors:msalter
// Date:        2001-12-20
// Purpose:     IQ80310 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHERNET
#include <cyg/hal/hal_cache.h>          // HAL_DCACHE_LINE_SIZE
#include <cyg/hal/plf_io.h>             // CYGARC_UNCACHED_ADDRESS

#ifdef CYGPKG_DEVS_ETH_ARM_IQ80310_ETH0

// Bus masters can get to all of SDRAM using direct mapping.
#define CYGHWR_INTEL_I82559_PCI_VIRT_TO_BUS( _x_ ) ((cyg_uint32)CYGARC_VIRT_TO_BUS(_x_))

#ifndef CYGSEM_DEVS_ETH_ARM_IQ80310_ETH0_SET_ESA
# define CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM 0
# define CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM_WITHOUT_CRC
#endif

#define MAX_PACKET_SIZE   1536
#define SIZEOF_DESCRIPTOR 16
#define MISC_MEM          1128     // selftest, ioctl and statistics

#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE \
  (((MAX_PACKET_SIZE + SIZEOF_DESCRIPTOR) * \
     (CYGNUM_DEVS_ETH_INTEL_I82559_MAX_TX_DESCRIPTORS + \
      CYGNUM_DEVS_ETH_INTEL_I82559_MAX_RX_DESCRIPTORS)) + \
   MISC_MEM)

static char pci_mem_buffer[CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE + HAL_DCACHE_LINE_SIZE];

#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE \
  (CYGARC_UNCACHED_ADDRESS(((unsigned)pci_mem_buffer + HAL_DCACHE_LINE_SIZE - 1) & ~(HAL_DCACHE_LINE_SIZE - 1)))

static I82559 i82559_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_ARM_IQ80310_ETH0_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_ARM_IQ80310_ETH0_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82559_sc0,
           &i82559_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_IQ80310_ETH0_NAME, // Name for device
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
                "i82559_" CYGDAT_DEVS_ETH_ARM_IQ80310_ETH0_NAME,
                i82559_init, 
                &i82559_sc0);

#endif // CYGPKG_DEVS_ETH_ARM_IQ80310_ETH0


// These arrays are used for sanity checking of pointers
I82559 *
i82559_priv_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80310_ETH0
    &i82559_eth0_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
i82559_netdev_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80310_ETH0
    &i82559_netdev0,
#endif
};

struct eth_drv_sc *
i82559_sc_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_IQ80310_ETH0
    &i82559_sc0,
#endif
};
#endif // CYGDBG_USE_ASSERTS

// EOF devs_eth_arm_iq80310.inl
