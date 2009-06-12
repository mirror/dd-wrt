//==========================================================================
//
//      devs_eth_sh_se77x9.inl
//
//      SE77X9 ethernet I/O definitions.
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
// Date:        2001-06-13
// Purpose:     SE77X9 ethernet defintions
//
// FIXME:       M17 contains an EPROM. May contain the ESA, but don't
//              know how to access it.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR
#include <cyg/hal/hal_if.h>

#ifdef __WANT_CONFIG



#define CYGHWR_NS_DP83902A_PLF_RESET(_dp_)      \
    CYG_MACRO_START                             \
    HAL_WRITE_UINT16(_dp_->reset, 0x0000);      \
    CYGACC_CALL_IF_DELAY_US(10);                \
    HAL_WRITE_UINT16(_dp_->reset, 0x0100);      \
    CYG_MACRO_END

#define DP_IN(_b_, _o_, _d_)                                    \
    CYG_MACRO_START                                             \
    cyg_uint16 _t;                                              \
    HAL_READ_UINT16 ((cyg_addrword_t)(_b_)+2*(_o_), _t);        \
    (_d_) = (_t >> 8) & 0xff;                                   \
    CYG_MACRO_END

#define DP_OUT(_b_, _o_, _d_)                                   \
    CYG_MACRO_START                                             \
    HAL_WRITE_UINT16((cyg_addrword_t)(_b_)+2*(_o_), (_d_)<<8);  \
    CYG_MACRO_END

#define DP_IN_DATA(_b_, _d_)                                    \
    CYG_MACRO_START                                             \
    cyg_uint16 _t;                                              \
    HAL_READ_UINT16 ((cyg_addrword_t)(_b_), _t);                \
    (_d_) = ((_t >> 8) & 0xff) | ((_t & 0xff) << 8);            \
    CYG_MACRO_END

#define DP_OUT_DATA(_b_, _d_)                                   \
    CYG_MACRO_START                                             \
    cyg_uint16 _t;                                              \
    _t = (_d_);                                                 \
    (_t) = (((_t) >> 8) & 0xff) | ((_t & 0xff) << 8);           \
    HAL_WRITE_UINT16((cyg_addrword_t)(_b_), _t);                \
    CYG_MACRO_END

#define CYGHWR_NS_DP83902A_PLF_16BIT_DATA
#define CYGHWR_NS_DP83902A_PLF_BROKEN_TX_DMA

#endif // __WANT_CONFIG


#ifdef __WANT_DEVS

#ifdef CYGPKG_DEVS_ETH_SH_SE77X9_ETH0

static dp83902a_priv_data_t dp83902a_eth0_priv_data = { 
    base : (cyg_uint8*) 0xb0000000,
    data : (cyg_uint8*) 0xb0040000,
    reset: (cyg_uint8*) 0xb0080000,
    interrupt: CYGNUM_HAL_INTERRUPT_LAN,
    tx_buf1: 0x80,
    tx_buf2: 0x88,
    rx_buf_start: 0x90,
    rx_buf_end: 0xff,
#ifdef CYGSEM_DEVS_ETH_SH_SE77X9_ETH0_SET_ESA
    esa : CYGDAT_DEVS_ETH_SH_SE77X9_ETH0_ESA,
    hardwired_esa : true,
#else
    hardwired_esa : false,
#endif
};

ETH_DRV_SC(dp83902a_sc,
           &dp83902a_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_SH_SE77X9_ETH0_NAME,
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
                "dp83902a_" CYGDAT_DEVS_ETH_SH_SE77X9_ETH0_NAME,
                dp83902a_init, 
                &dp83902a_sc);
#endif // CYGPKG_DEVS_ETH_SH_SE77X9_ETH0

#endif // __WANT_DEVS

// EOF devs_eth_sh_se77x9.inl
