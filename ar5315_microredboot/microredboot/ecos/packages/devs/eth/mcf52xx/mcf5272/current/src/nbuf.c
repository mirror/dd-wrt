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

#include <cyg/devs/eth/nbuf.h>
/*
 * Functions to manipulate the network buffers.
 */

#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/devs/eth/if_mcf5272.h>
#include <cyg/devs/eth/nbuf.h>
#include <cyg/infra/cyg_ass.h>

/*******************************************************************************
nbuf_init() - Initalize the networ buffers.

INPUT: pBuf - Pointer to the network buffer data.
*/
void
nbuf_init (buf_info_t* pBuf)
{
	uint_t i = 0;
    uint8 *buf = (uint8*)&(pBuf->RxBuffer);
    buf = (uint8*)((((uint32)buf+15)/16)*16);

    /* Initialize RxNBUF and TxNBUF */
    pBuf->RxNBUF = (NBUF*)((((uint32)(&pBuf->nrxbuf[0])+3)/4)*4);
    pBuf->TxNBUF = (NBUF*)((((uint32)(&pBuf->ntxbuf[0])+3)/4)*4);

	/* Initialize receive descriptor ring */
	for (i = 0; i < NUM_RXBDS; i++)
	{
		pBuf->RxNBUF[i].status = RX_BD_E;
		pBuf->RxNBUF[i].length = 0;
		pBuf->RxNBUF[i].data = buf;
        buf += RX_BUFFER_SIZE;
	}
	/* Set the Wrap bit on the last one in the ring */
	pBuf->RxNBUF[NUM_RXBDS - 1].status |= RX_BD_W;

	/* Initialize transmit descriptor ring */
	for (i = 0; i < NUM_TXBDS; i++)
	{
		pBuf->TxNBUF[i].length = 0;
		pBuf->TxNBUF[i].data = 0L;
        pBuf->TxNBUF[i].status = 0;
	}
	/* Set the Wrap bit on the last one in the ring */
	pBuf->TxNBUF[NUM_TXBDS - 1].status |= TX_BD_W;

	/* Initialize the buffer descriptor indexes */
	pBuf->iTxbd = pBuf->iRxbd = 0;

    /* Intiaize the transmit key queue to zero. */
    pBuf->tq_front = pBuf->tq_rear = 0;

    // Initalize the nubmer of transmit buffer descriptors used to zero.

    pBuf->num_busy_bd = 0;
}




