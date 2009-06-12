//==========================================================================
//
//      devs/eth/sh/dreamcast/include/devs_eth_sh_dreamcast_rltk8139.inl
//
//      SEGA Dreamcast Broadband Adapter I/O definitions.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Red Hat, Inc.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Yoshinori Sato
// Contributors:
// Date:        2004-04-22
// Purpose:     SEGA Dreamcast Broadband Adapter definitions
//####DESCRIPTIONEND####
//==========================================================================

#ifdef CYGPKG_DEVS_ETH_SH_DREAMCAST_RLTK8139_ETH0

static Rltk8139_t rltk8139_eth0_priv_data = {
	0, 0, 0
};

ETH_DRV_SC(rltk8139_sc0,
           &rltk8139_eth0_priv_data,
           CYGDAT_DEVS_ETH_SH_DREAMCAST_RLTK8139_ETH0_NAME,
           rltk8139_start,
           rltk8139_stop,
           rltk8139_control,
           rltk8139_can_send,
           rltk8139_send,
           rltk8139_recv,
           rltk8139_deliver,
           rltk8139_poll,
           rltk8139_int_vector
           );

NETDEVTAB_ENTRY(rltk8139_netdev0,
                "rltk8139_" CYGDAT_DEVS_ETH_SH_DREAMCAST_RLTK8139_ETH0_NAME,
                rltk8139_init,
                &rltk8139_sc0);

void __dreamcast_bba_init(struct eth_drv_sc *sc);
#define CYGHWR_RLTK_8139_PLF_INIT(sc) __dreamcast_bba_init(sc);

#endif // CYGPKG_DEVS_ETH_SH_DREAMCAST_RLTK8139_ETH0

// EOF devs_eth_sh_dreamcast_rltk8139.inl
