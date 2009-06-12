//==========================================================================
//
//      devs/eth/mips/ocelot/include/devs_eth_mips_rm7000_ocelot.inl
//
//      PC i82559 ethernet I/O definitions.
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-01-25
// Purpose:     PC i82559 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR

#ifdef CYGPKG_DEVS_ETH_I386_PC_I82559_ETH0

#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE (CYGARC_UNCACHED_ADDRESS(CYGMEM_SECTION_pci_window))
#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE CYGMEM_SECTION_pci_window_SIZE

static I82559 i82559_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_I386_PC_I82559_ETH0_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_I386_PC_I82559_ETH0_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82559_sc0,
           &i82559_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_I386_PC_I82559_ETH0_NAME, // Name for device
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
                "i82559_" CYGDAT_DEVS_ETH_I386_PC_I82559_ETH0_NAME,
                i82559_init, 
                &i82559_sc0);

#endif // CYGPKG_DEVS_ETH_I386_PC_I82559_ETH0


// These arrays are used for sanity checking of pointers
I82559 *
i82559_priv_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_I386_PC_I82559_ETH0
    &i82559_eth0_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
i82559_netdev_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_I386_PC_I82559_ETH0
    &i82559_netdev0,
#endif
};

struct eth_drv_sc *
i82559_sc_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_I386_PC_I82559_ETH0
    &i82559_sc0,
#endif
};
#endif // CYGDBG_USE_ASSERTS

// EOF devs_eth_i386_pc_i82559.inl
