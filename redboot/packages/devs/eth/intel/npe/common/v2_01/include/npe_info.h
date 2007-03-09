#ifndef CYGONCE_DEVS_ETH_INTEL_NPE_INFO_H
#define CYGONCE_DEVS_ETH_INTEL_NPE_INFO_H
/*==========================================================================
//
//        npe_info.h
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005 Red Hat, Inc.
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
// Author(s):     msalter
// Contributors:  
// Date:          2003-03-20
// Description:
//
//####DESCRIPTIONEND####
*/

#include <pkgconf/devs_eth_intel_npe.h>

#define NPE_PHY_UNKNOWN 0xff

struct npe {
    cyg_uint8  active;           // NPE active
    cyg_uint8  eth_id;           // IX_ETH_PORT_1 or IX_ETH_PORT_2
    cyg_uint8  phy_no;           // which PHY (0 - 31)
    cyg_uint8  mac_address[6];

    struct eth_drv_sc *sc;

    IX_OSAL_MBUF  *rxQHead;
    IX_OSAL_MBUF  *txQHead;

    cyg_uint8     *tx_pkts;
    cyg_uint8     *rx_pkts;
    IX_OSAL_MBUF  *rx_mbufs;
    IX_OSAL_MBUF  *tx_mbufs;
};

#define CYGDAT_DEVS_ETH_DESCRIPTION "Intel Network Processor Engine"

#endif /* ifndef CYGONCE_DEVS_ETH_INTEL_NPE_INFO_H */

/* EOF npe_info.h */

