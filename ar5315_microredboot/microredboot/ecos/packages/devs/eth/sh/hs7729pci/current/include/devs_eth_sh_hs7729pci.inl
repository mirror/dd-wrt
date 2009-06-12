//==========================================================================
//
//      devs_eth_sh_hs7729pci.inl
//
//      HS7729PCI ethernet I/O definitions.
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
// Date:        2001-05-31
// Purpose:     HS7729PCI ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR

#ifdef __WANT_CONFIG

#define CYGHWR_VIA_RHINE_PCI_MEM_MAP_BASE (CYGARC_UNCACHED_ADDRESS(&CYGMEM_SECTION_pci_window[0]))
#define CYGHWR_VIA_RHINE_PCI_MEM_MAP_SIZE (CYGMEM_SECTION_pci_window_SIZE)

#endif // __WANT_CONFIG


#ifdef __WANT_DEVS

#ifdef CYGPKG_DEVS_ETH_SH_HS7729PCI_ETH0

static rhine_priv_data via_rhine_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_SH_HS7729PCI_ETH0_SET_ESA
    enaddr : CYGDAT_DEVS_ETH_SH_HS7729PCI_ETH0_ESA,
#endif
    config_esa : NULL,             // rely on the hardwired address for now
    rx_ring : NULL,
    rx_ring_cnt : (1<<2) /*CYGNUM_DEVS_ETH_SH_HS7729PCI_ETH0_RX_RING_SIZE*/,
    rx_ring_log_cnt : 2,
    tx_ring : NULL,
    tx_ring_cnt : (1<<2) /*CYGNUM_DEVS_ETH_SH_HS7729PCI_ETH0_TX_RING_SIZE*/,
    tx_ring_log_cnt : 2,
};

static rhine_priv_data *rhine_priv_array[1] = {&via_rhine_eth0_priv_data};

ETH_DRV_SC(via_rhine_sc,
           &via_rhine_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_SH_HS7729PCI_ETH0_NAME,
           rhine_start,
           rhine_stop,
           rhine_control,
           rhine_can_send,
           rhine_send,
           rhine_recv,
           rhine_deliver,     // "pseudoDSR" called from fast net thread
           rhine_poll,        // poll function, encapsulates ISR and DSR
           rhine_int_vector);

NETDEVTAB_ENTRY(rhine_netdev, 
                "rhine_" CYGDAT_DEVS_ETH_SH_HS7729PCI_ETH0_NAME,
                via_rhine_init, 
                &via_rhine_sc);
#endif // CYGPKG_DEVS_ETH_SH_HS7729PCI_ETH0

#endif // __WANT_DEVS

// EOF devs_eth_sh_hs7729pci.inl
