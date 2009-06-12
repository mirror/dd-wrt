//==========================================================================
//
//      if_aki3068net.c
//
//      Ethernet device driver for Akizuki H8/3068 Netwotk micom
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):
// Contributors:
// Date:
// Purpose:      
// Description:
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>

#define DP_CARD_RESET 0x1f

#include <cyg/io/dp83902a.h>

void ns_dp83902a_plf_init(dp83902a_priv_data_t *dp)
{
    static struct {
	unsigned short offset;
	unsigned short data;    
    } init_data[] = {
        {DP_DCR, 0x48},  // Bytewide access
        {DP_RBCH, 0},    // Remote byte count
        {DP_RBCL, 0},
        {DP_ISR, 0xFF},  // Clear any pending interrupts
        {DP_IMR, 0x00},  // Mask all interrupts 
        {DP_RCR, 0x20},  // Monitor
        {DP_TCR, 0x02},  // loopback
        {DP_RBCH, 32},   // Remote byte count
        {DP_RBCL, 0},
        {DP_RSAL, 0},    // Remote address
        {DP_RSAH, 0},
        {DP_CR, DP_CR_START|DP_CR_RDMA}  // Read data
    };
    unsigned char prom[32];
    int cnt,tmp;

    HAL_READ_UINT8(dp->base+DP_CARD_RESET, tmp);
    HAL_WRITE_UINT8(dp->base+DP_CARD_RESET, tmp);
    dp->data = dp->base+DP_DATAPORT;
    // Wait for card
    do {
	int cnt;
        DP_IN(dp->base, DP_ISR, tmp);
	for (cnt=0; cnt< 1024; cnt++);
    } while (0 == (tmp & DP_ISR_RESET));

    if (dp->hardwired_esa)
        return ;

    for (cnt=0; cnt<sizeof(init_data)/sizeof(init_data[0]); cnt++)
        DP_OUT(dp->base, init_data[cnt].offset, init_data[cnt].data);
    for (cnt = 0;  cnt < 32;  cnt++)
        DP_IN_DATA(dp->data, prom[cnt]);
    if ((prom[0] == 0xff) && (prom[2] == 0xff) && (prom[4] == 0xff)) {
        dp->base = 0;
	return ;
    }
    DP_OUT(dp->base, DP_CR, DP_CR_NODMA | DP_CR_PAGE1);
    for (cnt = 0; cnt < 6; cnt ++) {
	DP_OUT(dp->base, DP_P1_PAR0+cnt, prom[cnt*2]);
    }
}
