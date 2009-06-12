//==========================================================================
//
//      devs/eth/i386/pc/include/devs_eth_i386_pc_lancepci.inl
//
//      PC Lance PCI ethernet I/O definitions.
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
// Author(s):   iz
// Contributors:
// Date:        2002-07-17
// Purpose:     PC Lance PCI (VLANCE device in vmWare) ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR

#ifdef __WANT_CONFIG
#define CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_BASE (CYGARC_UNCACHED_ADDRESS(CYGMEM_SECTION_pci_window))
#define CYGHWR_AMD_LANCEPCI_PCI_MEM_MAP_SIZE CYGMEM_SECTION_pci_window_SIZE
#define HAL_PCI_CPU_TO_BUS(__cpu_addr, __bus_addr)   \
    CYG_MACRO_START                                  \
    (__bus_addr) = CYGARC_PHYSICAL_ADDRESS(__cpu_addr);   \
    CYG_MACRO_END
#endif // __WANT_CONFIG


#ifdef __WANT_DEVS

#ifdef CYGPKG_DEVS_ETH_I386_PC_LANCEPCI_ETH0

static lancepci_priv_data amd_lancepci_eth0_priv_data = {
#ifdef CYGSEM_DEVS_ETH_I386_PC_LANCEPCI_ETH0_SET_ESA
    esa : CYGDAT_DEVS_ETH_I386_PC_LANCEPCI_ETH0_ESA,
    hardwired_esa : true,
#else
    hardwired_esa : false,
#endif
    config_esa : NULL,             // rely on the hardwired address for now
    rx_ring : NULL,
    rx_ring_cnt : CYGNUM_DEVS_ETH_I386_PC_LANCEPCI_ETH0_RX_RING_SIZE,
    rx_ring_log_cnt : 2,
    tx_ring : NULL,
    tx_ring_cnt : CYGNUM_DEVS_ETH_I386_PC_LANCEPCI_ETH0_TX_RING_SIZE,
    tx_ring_log_cnt : 2,
};

ETH_DRV_SC(amd_lancepci_sc,
           &amd_lancepci_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_I386_PC_LANCEPCI_ETH0_NAME,
           lancepci_start,
           lancepci_stop,
           lancepci_control,
           lancepci_can_send,
           lancepci_send,
           lancepci_recv,
           lancepci_deliver,     // "pseudoDSR" called from fast net thread
           lancepci_poll,        // poll function, encapsulates ISR and DSR
           lancepci_int_vector);

NETDEVTAB_ENTRY(lancepci_netdev,
                "lancepci_" CYGDAT_DEVS_ETH_I386_PC_LANCEPCI_ETH0_NAME,
                amd_lancepci_init,
                &amd_lancepci_sc);
#endif // CYGPKG_DEVS_ETH_I386_PC_LANCEPCI_ETH0

// These arrays are used for sanity checking of pointers
struct lancepci_priv_data *
lancepci_priv_array[CYGNUM_DEVS_ETH_AMD_LANCEPCI_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_I386_PC_LANCEPCI_ETH0
    &amd_lancepci_eth0_priv_data,
#endif
};

#endif // __WANT_DEVS

// EOF devs_eth_pc_lancepci.inl
