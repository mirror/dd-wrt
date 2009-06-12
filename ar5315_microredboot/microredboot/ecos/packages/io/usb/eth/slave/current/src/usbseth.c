//==========================================================================
//
//      usbseth.c
//
//      Support for USB-ethernet devices, slave-side.
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
// Author(s):    bartv
// Contributors: bartv
// Date:         2000-10-04
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>

#include <pkgconf/io_usb_slave_eth.h>

#define __ECOS 1
#include <cyg/io/usb/usbs_eth.h>

#ifdef CYGPKG_USBS_ETHDRV
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#endif

// ----------------------------------------------------------------------------
// Static data.
//
// usbs_eth0 contains the per-device data, both the low-level data
// such as which endpoints to use and the network-driver data such as
// SNMP statistics. If this package is loaded then the assumption
// is that there should be at least one USB-ethernet device. Additional
// ones can be instantiated in application code if necessary. A call
// to usbs_eth_init() is required for initialization.
usbs_eth usbs_eth0;

// ----------------------------------------------------------------------------
// Initialization. This should be called explicitly by application code
// at an appropriate point in the system startup.
void
usbs_eth_init(usbs_eth* eth, usbs_control_endpoint* ctrl, usbs_rx_endpoint* rx, usbs_tx_endpoint* tx, unsigned char* mac)
{
    eth->control_endpoint       = ctrl;
    eth->rx_endpoint            = rx;
    eth->tx_endpoint            = tx;
    eth->host_up                = false;
    eth->host_promiscuous       = false;
    memcpy(eth->host_MAC, mac, 6);
    eth->rx_pending_buf         = (unsigned char*) 0;
    
    // Install default handlers for some messages. Higher level code
    // may override this.
    ctrl->state_change_fn       = &usbs_eth_state_change_handler;
    ctrl->state_change_data     = (void*) eth;
    ctrl->class_control_fn      = &usbs_eth_class_control_handler;
    ctrl->class_control_data    = (void*) eth;
    
#ifdef CYGPKG_USBS_ETHDRV
    eth->ecos_up                = false;
    eth->rx_active              = false;
# ifdef CYGFUN_USBS_ETHDRV_STATISTICS    
    eth->interrupts             = 0;
    eth->tx_count               = 0;
    eth->rx_count               = 0;
# endif
# ifndef HAL_DCACHE_LINE_SIZE
    eth->rx_bufptr              = eth->rx_buffer;
# else
# endif    
    eth->rx_bufptr              = (unsigned char*) ((((cyg_uint32)eth->rx_buffer) + HAL_DCACHE_LINE_SIZE - 1)
                                                    & ~(HAL_DCACHE_LINE_SIZE - 1));
    eth->rx_buffer_full         = false;
    eth->tx_in_send             = false;
    eth->tx_buffer_full         = false;
    eth->tx_done                = false;
#endif
}


// ----------------------------------------------------------------------------
// Generic transmit and receive operations. These can be called
// explicitly by application code, or implicitly via the eCos ethernet
// device driver code in usbsethdrv.c. These two modes of operation
// should not be mixed since the routines do not perform any
// synchronization themselves, instead they rely on higher level code.

// Packet transmission. The exported function is usbs_eth_start_tx(),
// which can be invoked from thread context or DSR context. The
// supplied buffer must already be in a form that can be transmitted
// directly out of the USB endpoint with no further processing
// (although it is necessary to extract the size information from the
// buffer).
//
// When the underlying USB transfer has completed the USB code will invoke
// usbs_eth_tx_callback(), usually in DSR context although possibly in
// thread context depending on the specific USB implementation. The
// underlying USB driver may have had to do some padding so the amount
// transferred may be slightly greater than requested.

static void
usbs_eth_tx_callback(void* usbs_callback_arg, int size)
{
    usbs_eth*   eth = (usbs_eth*) usbs_callback_arg;
    CYG_ASSERT( (size < 0) || (size >= CYGNUM_USBS_ETH_MINTU), "returned size must be valid.");
    (*eth->tx_callback_fn)(eth, eth->tx_callback_arg, size);
}

void
usbs_eth_start_tx(usbs_eth* eth, unsigned char* buf, void (*callback_fn)(usbs_eth*, void*, int), void* callback_arg)
{
    int      size;
    cyg_bool address_ok = false;
    static const unsigned char broadcast_mac[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    size = buf[0] + (buf[1] << 8);
    CYG_ASSERT( (size < 0) || ((size >= CYGNUM_USBS_ETH_MIN_FRAME_SIZE) && (size <= CYGNUM_USBS_ETH_MAX_FRAME_SIZE)), \
                "ethernet frame size constraints must be observed");

    if ((0 == memcmp(buf + 2, eth->host_MAC, 6)) ||
        (0 == memcmp(buf + 2, broadcast_mac, 6))) {
        address_ok = true;
    }

    // The following checks involve data that can change as a result
    // of control operations, so it is necessary to synchronize with
    // those. The control operations will typically run at DSR level
    // so a DSR lock has to be used. 
    
    cyg_drv_dsr_lock();
    if (eth->host_up && (address_ok || eth->host_promiscuous)) {
        
        eth->tx_callback_fn             = callback_fn;
        eth->tx_callback_arg            = callback_arg;
        eth->tx_endpoint->buffer        = buf;
        eth->tx_endpoint->buffer_size   = size + 2;
        eth->tx_endpoint->complete_fn   = &usbs_eth_tx_callback;
        eth->tx_endpoint->complete_data = (void*) eth;
        (*(eth->tx_endpoint->start_tx_fn))(eth->tx_endpoint);
        
    } else {
        // Packets not intended for the host can be discarded quietly.
        // A broken connection needs to be reported.
        (*callback_fn)(eth, callback_arg, eth->host_up ? size : -EPIPE);
    }
    cyg_drv_dsr_unlock();
}

// Packet reception. This simply involves starting a transfer for
// up to the maximum ethernet frame size. The lower-level USB code
// will detect the end of the transfer. The exported function is
// usbs_eth_start_rx(). 
static void
usbs_eth_rx_callback(void* usbs_callback_arg, int size)
{
    usbs_eth*   eth = (usbs_eth*) usbs_callback_arg;

    CYG_ASSERT( (size <= 0) || ((size >= CYGNUM_USBS_ETH_MINTU) && (size <= CYGNUM_USBS_ETH_MAXTU)), \
                "ethernet frame size constraints must be observed");
    
    (*eth->rx_callback_fn)(eth, eth->rx_callback_arg, size);
}

void
usbs_eth_start_rx(usbs_eth* eth, unsigned char* buf, void (*callback_fn)(usbs_eth*, void*, int), void* callback_arg)
{
    eth->rx_callback_fn  = callback_fn;
    eth->rx_callback_arg = callback_arg;

    cyg_drv_dsr_lock();
    if (eth->host_up) {
        eth->rx_endpoint->buffer        = buf;
        eth->rx_endpoint->buffer_size   = CYGNUM_USBS_ETH_RXSIZE;
        eth->rx_endpoint->complete_fn   = &usbs_eth_rx_callback;
        eth->rx_endpoint->complete_data = (void*) eth;
        (*(eth->rx_endpoint->start_rx_fn))(eth->rx_endpoint);
    } else {
        CYG_ASSERT( (void*) 0 == eth->rx_pending_buf, "No RX operation should be in progress");
        eth->rx_pending_buf = buf;
    }
    cyg_drv_dsr_unlock();
}
 
// ----------------------------------------------------------------------------
// Control operations. The host may send two types of application-specific
// control messages, one to get the MAC address and one to enable/disable
// promiscuous mode on the host side. This callback will typically be invoked
// in DSR context.

// These constants need to be shared somehow with the driver in ../host/,
// but if some variant of that driver becomes part of the Linux kernel
// then its sources must be self-contained with no dependencies on
// eCos sources or headers. Hence a duplicate definition for now.
#define USBS_ETH_CONTROL_GET_MAC_ADDRESS        0x01
#define USBS_ETH_CONTROL_SET_PROMISCUOUS_MODE   0x02

usbs_control_return
usbs_eth_class_control_handler(usbs_control_endpoint* endpoint, void* callback_data)
{
    usbs_control_return result = USBS_CONTROL_RETURN_STALL;
    
    usbs_eth*   eth    = (usbs_eth*)   callback_data;
    usb_devreq* devreq = (usb_devreq*) endpoint->control_buffer;
    int         size   = (devreq->length_hi << 8) + devreq->length_lo;

    CYG_ASSERT(endpoint == eth->control_endpoint, "USB ethernet control messages correctly routed");

    if (USBS_ETH_CONTROL_GET_MAC_ADDRESS == devreq->request) {
        // This should be an IN operation for at least six bytes.
        if ((size >= 6) &&
            (USB_DEVREQ_DIRECTION_IN == (devreq->type & USB_DEVREQ_DIRECTION_MASK))) {

            endpoint->buffer      = eth->host_MAC;
            endpoint->buffer_size = 6;
            result = USBS_CONTROL_RETURN_HANDLED;
        }
        // Otherwise drop through with a return value of STALL
        
    } else if (USBS_ETH_CONTROL_SET_PROMISCUOUS_MODE == devreq->request) {
        // The length should be 0, no more data is expected by either side.
        if (0 == size) {
            // The new promiscuity mode is encoded in value_lo;
            eth->host_promiscuous = devreq->value_lo;
            result = USBS_CONTROL_RETURN_HANDLED;
        }
    } 

    return result;
}

// State changes. As far as the ethernet code is concerned, if there
// is a change to CONFIGURED state then the device has come up,
// otherwise if there is a change from CONFIGURED state it has gone
// down. All other state changes are irrelevant.
void
usbs_eth_state_change_handler(usbs_control_endpoint* endpoint, void* callback_data, usbs_state_change change, int old_state)
{
    usbs_eth* eth       = (usbs_eth*) callback_data;
    CYG_ASSERT(endpoint == eth->control_endpoint, "USB ethernet state changes correctly routed");

    if (USBS_STATE_CHANGE_CONFIGURED == change) {
        if (USBS_STATE_CONFIGURED != old_state) {
            usbs_eth_enable(eth);
        }
    } else if ((USBS_STATE_CHANGE_RESUMED == change) && (USBS_STATE_CONFIGURED == (USBS_STATE_MASK & old_state))) {
        usbs_eth_enable(eth);
    } else if (eth->host_up) {
        usbs_eth_disable(eth);
    }
}

// Disabling the ethernet device means clearing the host_up flag.
// This will block future transmits and receives but not any
// that are currently underway.
void
usbs_eth_disable(usbs_eth* eth)
{
    eth->host_up = false;
}

// Enabling the ethernet device means setting the host_up flag and
// possibly activating a pending rx operation.
void
usbs_eth_enable(usbs_eth* eth)
{
    if (!eth->host_up) {
        eth->host_up            = true;
        eth->host_promiscuous   = false;
        if ((void*) 0 != eth->rx_pending_buf) {
            eth->rx_endpoint->buffer            = eth->rx_pending_buf;
            eth->rx_endpoint->buffer_size       = CYGNUM_USBS_ETH_RXSIZE;
            eth->rx_endpoint->complete_fn       = &usbs_eth_rx_callback;
            eth->rx_endpoint->complete_data     = (void*) eth;
            eth->rx_pending_buf = (void*) 0;
            (*(eth->rx_endpoint->start_rx_fn))(eth->rx_endpoint);
        }
    }
}

