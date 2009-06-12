//==========================================================================
//
//      devs/eth/h8300/edosk2674/...../include/devs_eth_edosk2674.inl
//
//      EDOSK-2674 ethernet I/O definitions.
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
// Author(s):    Yoshinori Sato <ysato@users.sourceforge.jp>
// Contributors: ysato
// Date:         2003-02-25
// Purpose:      EDOSK-2674 ethernet definitions
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_h8300_edosk2674.h>
#include <cyg/hal/hal_intr.h>          

// MAC address is stored as a Redboot config option
#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

static lan91cxx_priv_data lan91cxx_eth0_priv_data = { 
#ifdef CYGSEM_DEVS_ETH_H8300_EDOSK2674_ETH0_SET_ESA
    enaddr : CYGDAT_DEVS_ETH_H8300_EDOSK2674_ESA,
#endif
    base : (unsigned short *)0xf80000,
    interrupt : 16
};

ETH_DRV_SC(lan91cxx_sc,
           &lan91cxx_eth0_priv_data,             // Driver specific data
           CYGDAT_DEVS_ETH_H8300_EDOSK2674_NAME, // Name for device
           lan91cxx_start,
           lan91cxx_stop,
           lan91cxx_control,
           lan91cxx_can_send,
           lan91cxx_send,
           lan91cxx_recv,
           lan91cxx_deliver,
           lan91cxx_poll,
           lan91cxx_int_vector
);

NETDEVTAB_ENTRY(lan91cxx_netdev, 
                "lan91cxx_" CYGDAT_DEVS_ETH_H8300_EDOSK2674_NAME,
                smsc_lan91cxx_init,
                &lan91cxx_sc);

//EOF devs_eth_flexanet.inl


