//==========================================================================
//
//      devs/eth/mips/malta/include/devs_eth_mips_mips32_malta.inl
//
//      Malta ethernet I/O definitions.
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
// Date:        2001-04-02
// Purpose:     Malta ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR

#ifdef __WANT_CONFIG

#define CYGHWR_AMD_PCNET_PCI_MEM_MAP_BASE CYGARC_UNCACHED_ADDRESS((cyg_uint32)(&CYGMEM_SECTION_pci_window[0]))
#define CYGHWR_AMD_PCNET_PCI_MEM_MAP_SIZE ((cyg_uint32)(CYGMEM_SECTION_pci_window_SIZE))

#endif // __WANT_CONFIG


#ifdef __WANT_DEVS

#ifdef CYGPKG_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0

static pcnet_priv_data amd_pcnet_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0_SET_ESA
    esa : CYGDAT_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0_ESA,
    hardwired_esa : true,
#else
    hardwired_esa : false,
#endif
    config_esa : NULL,
    rx_ring : NULL,
    rx_ring_cnt : (1<<2) /*CYGNUM_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0_RX_RING_SIZE*/,
    rx_ring_log_cnt : 2,
    tx_ring : NULL,
    tx_ring_cnt : (1<<2) /*CYGNUM_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0_TX_RING_SIZE*/,
    tx_ring_log_cnt : 2,
};

ETH_DRV_SC(amd_pcnet_sc,
           &amd_pcnet_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0_NAME,
           pcnet_start,
           pcnet_stop,
           pcnet_control,
           pcnet_can_send,
           pcnet_send,
           pcnet_recv,
           pcnet_deliver,     // "pseudoDSR" called from fast net thread
           pcnet_poll,        // poll function, encapsulates ISR and DSR
           pcnet_int_vector);

NETDEVTAB_ENTRY(pcnet_netdev, 
                "pcnet_" CYGDAT_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0_NAME,
                amd_pcnet_init, 
                &amd_pcnet_sc);
#endif // CYGPKG_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0

// These arrays are used for sanity checking of pointers
struct pcnet_priv_data *
pcnet_priv_array[CYGNUM_DEVS_ETH_AMD_PCNET_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_MIPS_MIPS32_MALTA_ETH0
    &amd_pcnet_eth0_priv_data,
#endif
};

#endif // __WANT_DEVS

// EOF devs_eth_mips_mips32_malta.inl
