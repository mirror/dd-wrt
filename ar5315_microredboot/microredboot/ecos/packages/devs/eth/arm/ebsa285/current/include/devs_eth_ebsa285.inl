//==========================================================================
//
//      devs/eth/arm/ebsa285/..../include/devs_eth_ebsa285.inl
//
//      EBSA-285 ethernet I/O definitions.
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
// Author(s):    jskov, hmt
// Contributors: jskov, gthomas
// Date:         2001-02-28
// Purpose:      EBSA285 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_...
#include <pkgconf/hal_arm_ebsa285.h>
#include <cyg/hal/hal_cache.h>          // hal_virt_to_phys_address()

// --------------------------------------------------------------
// Platform specifics:

// support SDRAM with gaps in it.
#define CYGHWR_DEVS_ETH_INTEL_I82559_PCIMEM_DISCONTIGUOUS

// Interrupts are multiplex onto one interrupt pin.
#define CYGNUM_DEVS_ETH_INTEL_I82559_SEPARATE_MUX_INTERRUPT \
          CYGNUM_HAL_INTERRUPT_PCI_IRQ

// This brings on code to perform a selective reset on the device if the CU
// wedges.

#define CYGHWR_DEVS_ETH_INTEL_I82559_DEAD_TO (368640) // 0.1S of OS timer

// The mask on an SA110 is really an enable: 1 => enabled, 0 => masked.
// So to behave nestedly, we only need save the old value of the bits
// of interest.

#define CYGPRI_DEVS_ETH_INTEL_I82559_MASK_INTERRUPTS(p_i82559,old)      \
CYG_MACRO_START                                                         \
    int cpu_intr;                                                       \
    int mybits =                                                        \
        (1 << (p_i82559->vector)) |                                     \
        (1 << CYGNUM_HAL_INTERRUPT_PCI_IRQ);                            \
                                                                        \
    HAL_DISABLE_INTERRUPTS( cpu_intr );                                 \
    old = *SA110_IRQCONT_IRQENABLE;                                     \
    *SA110_IRQCONT_IRQENABLECLEAR = mybits; /* clear mybits */          \
    HAL_RESTORE_INTERRUPTS( cpu_intr );                                 \
CYG_MACRO_END

// We must only unmask (enable) those which were unmasked before,
// according to the bits in old.
#define CYGPRI_DEVS_ETH_INTEL_I82559_UNMASK_INTERRUPTS(p_i82559,old)    \
CYG_MACRO_START                                                         \
    *SA110_IRQCONT_IRQENABLESET = old &                                 \
        ((1 << (p_i82559->vector)) |                                    \
         (1 << CYGNUM_HAL_INTERRUPT_PCI_IRQ));                          \
CYG_MACRO_END

#define CYGPRI_DEVS_ETH_INTEL_I82559_ACK_INTERRUPTS(p_i82559)   \
CYG_MACRO_START                                                 \
CYG_MACRO_END


// --------------------------------------------------------------

#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_BASE CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE
#define CYGHWR_INTEL_I82559_PCI_MEM_MAP_SIZE CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_SIZE

#define CYGHWR_INTEL_I82559_PCI_VIRT_TO_BUS( _x_ ) virt_to_bus((cyg_uint32)_x_)
static inline cyg_uint32 virt_to_bus(cyg_uint32 p_memory)
{    return (p_memory - CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE);    }

// --------------------------------------------------------------
// Construct the two interfaces

#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH0

static I82559 i82559_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_ARM_EBSA285_ETH0_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_ARM_EBSA285_ETH0_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82559_sc0,
           &i82559_eth0_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_EBSA285_ETH0_NAME, // Name for device
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
                "i82559_" CYGDAT_DEVS_ETH_ARM_EBSA285_ETH0_NAME,
                i82559_init, 
                &i82559_sc0);

#endif // CYGPKG_DEVS_ETH_ARM_EBSA285_ETH0

#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH1

static I82559 i82559_eth1_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_ARM_EBSA285_ETH1_SET_ESA
    hardwired_esa: 1,
    mac_address: CYGDAT_DEVS_ETH_ARM_EBSA285_ETH1_ESA
#else
    hardwired_esa: 0,
#endif
};

ETH_DRV_SC(i82559_sc1,
           &i82559_eth1_priv_data,      // Driver specific data
           CYGDAT_DEVS_ETH_ARM_EBSA285_ETH1_NAME, // Name for device
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
                "i82559_" CYGDAT_DEVS_ETH_ARM_EBSA285_ETH1_NAME,
                i82559_init, 
                &i82559_sc1);

#endif // CYGPKG_DEVS_ETH_ARM_EBSA285_ETH1

// --------------------------------------------------------------
// These arrays are used for sanity checking of pointers
I82559 *
i82559_priv_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH0
    &i82559_eth0_priv_data,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH1
    &i82559_eth1_priv_data,
#endif
};

#ifdef CYGDBG_USE_ASSERTS
// These are only used when assertions are enabled
cyg_netdevtab_entry_t *
i82559_netdev_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH0
    &i82559_netdev0,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH1
    &i82559_netdev1,
#endif
};

struct eth_drv_sc *
i82559_sc_array[CYGNUM_DEVS_ETH_INTEL_I82559_DEV_COUNT] = {
#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH0
    &i82559_sc0,
#endif
#ifdef CYGPKG_DEVS_ETH_ARM_EBSA285_ETH1
    &i82559_sc1,
#endif
};
#endif // CYGDBG_USE_ASSERTS

// --------------------------------------------------------------
// Debugging

//#define CYGDBG_DEVS_ETH_INTEL_I82559_CHATTER 1

// --------------------------------------------------------------
// RedBoot configuration options for managing ESAs for us

#define CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM 0
#define CYGHWR_DEVS_ETH_INTEL_I82559_HAS_ONE_EEPROM_MAC_ADJUST (1)

// --------------------------------------------------------------

// EOF devs_eth_ebsa285.inl
