#ifndef _IF_MCF5272_PRIVATE_DATA_H_
#define _IF_MCF5272_PRIVATE_DATA_H_
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

#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/devs/eth/if_mcf5272.h>
#include <cyg/devs/eth/nbuf.h>

/*      Device driver private data.                                         */

struct MCF5272_fec_priv_data_t
{

    buf_info_t nbuffer;

    /*   Diagnositc supplement variables.                                   */

    struct
    {
        unsigned int old_tx_pk_cnt;
        unsigned int old_rx_pk_cnt;
        unsigned int old_tx_bytes_cnt;
        unsigned int old_rx_bytes_cnt;
    }diag_info_sup;

    /*   1 second alarm variables                                           */

    cyg_handle_t alarm_h;
    cyg_alarm   alarm;
    cyg_handle_t counter_h;

    /*   Diagnostic counters                                                */

    MCF5272_FEC_DIAG    diag_counters;

    /*   This flag is set if the device is operational                      */

    enum eth_drv_status_t operational;

    /*   Unknown, Half Duplex or Full duplex.                               */

    enum eth_drv_mode_t duplex;

    /*   Ethernet bandwidth.                                                */

    u32_t   speed;

    /*   Temporary packet buffer.                                           */

    eth_frame_hdr pkt_buf;

};
typedef struct MCF5272_fec_priv_data_t MCF5272_fec_priv_data_t;

#define PMCF5272_FEC_DATA(_SC_) \
    ((MCF5272_fec_priv_data_t*)(((struct eth_drv_sc*)(_SC_))->driver_private))
#define PBUF_INFO(_SC_) \
    (&(((MCF5272_fec_priv_data_t*)((_SC_)->driver_private))->nbuffer))

#endif /* _IF_MCF5272_PRIVATE_DATA_H_ */

