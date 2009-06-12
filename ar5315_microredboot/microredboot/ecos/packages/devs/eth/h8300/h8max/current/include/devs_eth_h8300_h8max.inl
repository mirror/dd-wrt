//==========================================================================
//
//      devs_eth_h8300_h8max.inl
//
//      DP83902(RTL8019AS) ethernet I/O definitions.
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
// Author(s): Yoshinori Sato
// Contributors: Yoshinori Sato
// Date: 2002-08-10
// Purpose:     h8max ethernet defintions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_INTERRUPT_ETHR
#include <cyg/hal/hal_if.h>

#define CYGHWR_NS_DP83902A_PLF_INIT ns_dp83902a_plf_init
#define CYGHWR_NS_DP83902A_PLF_16BIT_DATA
#define BIGEND
#define DELAY() 
#define DP_IN(_b_, _o_, _d_)  HAL_READ_UINT8 ((cyg_addrword_t)(_b_)+((_o_)<<1)+1, (_d_))
#define DP_OUT(_b_, _o_, _d_) HAL_WRITE_UINT16((cyg_addrword_t)(_b_)+((_o_)<<1), (_d_))

#ifdef __WANT_DEVS

#ifdef CYGPKG_DEVS_ETH_H8300_H8MAX_ETH0

static dp83902a_priv_data_t dp83902a_eth0_priv_data = { 
    base: (cyg_uint8 *)0x800600,
    interrupt: 16,
    tx_buf1: 0x40,
    tx_buf2: 0x48,
    rx_buf_start: 0x50,
    rx_buf_end: 0x80,
#ifdef CYGSEM_DEVS_ETH_H8300_H8MAX_ETH0_SET_ESA
    hardwired_esa: true,
    mac_address: CYGDAT_DEVS_ETH_H8300_H8MAX_ETH0_ESA
#else
    hardwired_esa: false,
#endif
};

ETH_DRV_SC(dp83902a_sc,
           &dp83902a_eth0_priv_data, // Driver specific data
           CYGDAT_DEVS_ETH_H8300_H8MAX_ETH0_NAME,
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
                "dp83902a_" CYGDAT_DEVS_ETH_H8300_H8MAX_ETH0_NAME,
                dp83902a_init, 
                &dp83902a_sc);

void ns_dp83902a_plf_init(dp83902a_priv_data_t *dp);

#endif // CYGPKG_DEVS_ETH_H8300_H8MAX_ETH0
#endif
// EOF devs_eth_h8300_h8max.inl
