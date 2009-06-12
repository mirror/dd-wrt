//==========================================================================
//
//      devs_eth_refidt334.inl
//
//      IDT79S334a i82559 ethernet I/O definitions.
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
// Author(s):   Tim Michals
// Contributors:jskov
// Date:        2003-02-13
// Purpose:     IDT79s334A i82559 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR

#if 1 < CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT
#define CYGHWR_DEVS_ETH_INTEL_I82559_DEMUX_ALL
#endif // multiple devs, so demux_all needed


#if defined(CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0) || defined(CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1)

#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE 0xa0100000
#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE 0x100000 //CYGMEM_SECTION_pci_window_SIZE

#define CYGHWR_INTEL_I82559_PCI_VIRT_TO_BUS( _x_ ) (((cyg_uint32)(_x_))|0)
#define CYGHWR_INTEL_I82559_PCI_BUS_TO_VIRT( _x_ ) (((cyg_uint32)(_x_))&~0)
#endif


#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0

static I82559 i82559_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGSEM_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0_SET_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82559_sc0,
           &i82559_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0_NAME, // Name for device
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
                "i82559_" CYGDAT_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0_NAME,
                i82559_init, 
                &i82559_sc0);

#endif // CYGPKG_DEVS_ETH_XXX_ETH0




#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1

static I82559 i82559_eth1_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1_SET_ESA
    hardwired_esa:2,
    mac_address: CYGSEM_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1_SET_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82559_sc1,
           &i82559_eth1_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1_NAME, // Name for device
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

NETDEVTAB_ENTRY(i82559_netdev1, 
                "i82559_" CYGDAT_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1_NAME,
                i82559_init, 
                &i82559_sc1);

#endif // CYGPKG_DEVS_ETH_XXX_ETH1




// These arrays are used for sanity checking of pointers
I82559 *
i82559_priv_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0
    &i82559_eth0_priv_data,
#endif
#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1
    &i82559_eth1_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
i82559_netdev_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0
    &i82559_netdev0,
#endif

#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1
    &i82559_netdev1,
#endif

};

struct eth_drv_sc *
i82559_sc_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH0
    &i82559_sc0,
#endif
#ifdef CYGPKG_DEVS_ETH_MIPS_REFIDT334_I82559_ETH1
    &i82559_sc1,
#endif
};
#endif 

// EOF devs_eth_refidt334.inl
