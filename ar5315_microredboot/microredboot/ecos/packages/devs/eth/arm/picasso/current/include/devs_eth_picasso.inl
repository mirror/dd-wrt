//==========================================================================
//
//      devs/eth/arm/picasso/..../include/devs_eth_picasso.inl
//
//      NMI picasso ethernet I/O definitions.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// Purpose:      FRV400 ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR
#include <cyg/hal/hal_if.h>
#include <cyg/io/pci.h>

#ifdef __WANT_CONFIG

#define CYGHWR_NS_DP83902A_PLF_RESET(_dp_)      \
    CYG_MACRO_START                             \
    cyg_uint8 _t;                               \
    HAL_READ_UINT8(_dp_->reset, _t);            \
    CYGACC_CALL_IF_DELAY_US(10);                \
    HAL_WRITE_UINT8(_dp_->reset, _t);           \
    CYGACC_CALL_IF_DELAY_US(10000);                \
    CYG_MACRO_END

#define DP_IN(_b_, _o_, _d_)                                    \
    CYG_MACRO_START                                             \
    _d_ = pci_io_read_8((cyg_addrword_t)(_b_)+(_o_));    \
    CYG_MACRO_END

#define DP_OUT(_b_, _o_, _d_)                                   \
    CYG_MACRO_START                                             \
    pci_io_write_8((cyg_addrword_t)(_b_)+(_o_), _d_);   \
    CYG_MACRO_END

#define DP_IN_DATA(_b_, _d_)                                    \
    CYG_MACRO_START                                             \
    _d_ = pci_io_read_16((cyg_addrword_t)(_b_));    \
    CYG_MACRO_END

#define DP_OUT_DATA(_b_, _d_)                                   \
    CYG_MACRO_START                                             \
    pci_io_write_16((cyg_addrword_t)(_b_), _d_);   \
    CYG_MACRO_END

//#define CYGHWR_NS_DP83902A_PLF_16BIT_DATA
//#define CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA

#endif // __WANT_CONFIG

#ifdef __WANT_DEVS

#if defined(CYGSEM_DEVS_ETH_PICASSO_ETH0_SET_ESA) 
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

static cyg_bool
find_rtl8029_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
    return ((v == 0x10EC) && (d == 0x8029));
}

static void
_picasso_eth_init(dp83902a_priv_data_t *dp)
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;
#if defined(CYGSEM_DEVS_ETH_PICASSO_ETH0_SET_ESA) 
    cyg_bool esa_ok;
    unsigned char _esa[6];
#else
    unsigned char prom[32];
    int i;
#endif

    devid = CYG_PCI_NULL_DEVID;
    if (cyg_pci_find_matching( &find_rtl8029_match_func, NULL, &devid )) {
        cyg_pci_get_device_info(devid, &dev_info);
        cyg_pci_translate_interrupt(&dev_info, &dp->interrupt);
        dp->base = (cyg_uint8 *)(dev_info.base_map[0] & ~1);
        dp->data = dp->base + 0x10;
        dp->reset = dp->base + 0x1F;
#if 0
        diag_printf("RTL8029 at %p, interrupt: %x\n", dp->base, dp->interrupt);
#endif
#if defined(CYGSEM_DEVS_ETH_PICASSO_ETH0_SET_ESA) 
        esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
                                             "lan_esa", _esa, CONFIG_ESA);
        if (esa_ok) {
            memcpy(dp->esa, _esa, sizeof(_esa));
        }
#else
        // Read ESA from EEPROM
        DP_OUT(dp->base, DP_DCR, 0x49);  // Wordwide access
        DP_OUT(dp->base, DP_RBCH, 0);    // Remote byte count
        DP_OUT(dp->base, DP_RBCL, 0);
        DP_OUT(dp->base, DP_ISR, 0xFF);  // Clear any pending interrupts
        DP_OUT(dp->base, DP_IMR, 0x00);  // Mask all interrupts 
        DP_OUT(dp->base, DP_RCR, 0x20);  // Monitor
        DP_OUT(dp->base, DP_TCR, 0x02);  // loopback
        DP_OUT(dp->base, DP_RBCH, 32);   // Remote byte count
        DP_OUT(dp->base, DP_RBCL, 0);
        DP_OUT(dp->base, DP_RSAL, 0);    // Remote address
        DP_OUT(dp->base, DP_RSAH, 0);
        DP_OUT(dp->base, DP_CR, DP_CR_START|DP_CR_RDMA);  // Read data
        for (i = 0;  i < 32;  i++) {
            cyg_uint16 _val;
            HAL_READ_UINT16(dp->data, _val);
            prom[i] = _val;
        }
        // Set ESA into chip
        DP_OUT(dp->base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1);  // Select page 1
        for (i = 0; i < 6; i++) {
            DP_OUT(dp->base, DP_P1_PAR0+i, prom[i]);
        }
        DP_OUT(dp->base, DP_CR, DP_CR_NODMA | DP_CR_PAGE0);  // Select page 0
#endif
    }
}

#undef  CYGHWR_NS_DP83902A_PLF_INIT
#define CYGHWR_NS_DP83902A_PLF_INIT(dp) _picasso_eth_init(dp)

#ifndef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
static void
_picasso_eth_int_clear(dp83902a_priv_data_t *dp)
{
    cyg_drv_interrupt_acknowledge(dp->interrupt);
}

#undef  CYGHWR_NS_DP83902A_PLF_INT_CLEAR
#define CYGHWR_NS_DP83902A_PLF_INT_CLEAR(dp) _picasso_eth_int_clear(dp)
#endif

#ifdef CYGPKG_DEVS_ETH_PICASSO_ETH0

static dp83902a_priv_data_t dp83902a_eth0_priv_data = { 
    base : (cyg_uint8*) 0,  //
    data : (cyg_uint8*) 0,  // Filled in at runtime
    reset: (cyg_uint8*) 0,  //
    interrupt: 0,           //
    tx_buf1: 0x40,          // 
    tx_buf2: 0x48,          // Buffer layout - change with care
    rx_buf_start: 0x50,     //
    rx_buf_end: 0x80,       //
#ifdef CYGSEM_DEVS_ETH_PICASSO_ETH0_SET_ESA
    esa : CYGDAT_DEVS_ETH_PICASSO_ETH0_ESA,
    hardwired_esa : true,
#else
    hardwired_esa : false,
#endif
};

ETH_DRV_SC(dp83902a_sc,
           &dp83902a_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_PICASSO_ETH0_NAME,
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
                "dp83902a_" CYGDAT_DEVS_ETH_PICASSO_ETH0_NAME,
                dp83902a_init, 
                &dp83902a_sc);

#endif // CYGPKG_DEVS_ETH_PICASSO_ETH0

#endif // __WANT_DEVS

// --------------------------------------------------------------

// EOF devs_eth_picasso.inl
