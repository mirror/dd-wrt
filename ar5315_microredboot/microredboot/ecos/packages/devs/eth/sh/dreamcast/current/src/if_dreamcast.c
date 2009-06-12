//==========================================================================
//
//      devs/eth/sh/dreamcast/src/devs_eth_sh_dreamcast_rltk8139.c
//
//      Dreamcast Broadband Adapter initialize functions.
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
// Author(s):   Yoshiori Sato
// Contributors:
// Date:        2004-04-21
// Purpose:     dreamcast BbA initialize functions
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/pci.h>
#include <cyg/infra/diag.h>

#include <cyg/devs/eth/src/if_8139.h>

/* DMA BASE address */
#define GAPSPCI_DMA_BASE 0x01840000
#define GAPSPCI_DMA_SIZE 32768
#define BBA_BASE         0x01001700

#define P2BASE           0xa0000000

static cyg_uint32 gapspci_dma_alloc(int size)
{
    static cyg_uint32 top =  GAPSPCI_DMA_BASE;
    cyg_uint32 r;
    if ((GAPSPCI_DMA_SIZE - (top - GAPSPCI_DMA_BASE)) < size)
	r =  0;
    else {
	r = top;
	top += size;
    }
    return r;
}

void __dreamcast_bba_init(struct eth_drv_sc *sc)
{
    Rltk8139_t *rltk8139_info = (Rltk8139_t *)(sc->driver_private);
    int i;

    rltk8139_info->base_address = (BBA_BASE | P2BASE);
    
    rltk8139_info->tx_buffer = (cyg_uint8 *)(gapspci_dma_alloc(TX_BUF_TOT_LEN) | P2BASE);
    rltk8139_info->rx_ring   = (cyg_uint8 *)(gapspci_dma_alloc(RX_BUF_TOT_LEN) | P2BASE);

    for (i = 0; i < 6; ++i)
	HAL_READ_UINT8(rltk8139_info->base_address + IDR0 + i, rltk8139_info->mac[i]);
}

// EOF if_dreamcast.c
