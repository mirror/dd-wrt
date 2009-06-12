//==========================================================================
//
//      usbethdrv.c
//
//      Network device driver for USB-ethernet devices.
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
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>

#define __ECOS 1
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/eth_drv_stats.h>

#include <pkgconf/io_usb_slave_eth.h>
#include <cyg/io/usb/usbs_eth.h>

// ----------------------------------------------------------------------------
// The network driver data structure.
ETH_DRV_SC(usbs_eth_sc0,
           (void*) &usbs_eth0,
           CYGDAT_USBS_ETHDRV_NAME,
           usbs_ethdrv_start,
           usbs_ethdrv_stop,
           usbs_ethdrv_ioctl,
           usbs_ethdrv_can_send,
           usbs_ethdrv_send,
           usbs_ethdrv_recv,
           usbs_ethdrv_deliver,
           usbs_ethdrv_poll,
           usbs_ethdrv_intvector);

NETDEVTAB_ENTRY(usbs_eth_netdev0,
                "usbs_eth0",
                usbs_ethdrv_init,
                &usbs_eth_sc0);

// ----------------------------------------------------------------------------
// Statics gathering. The following macro can be used to increment a
// statistic without having to use a #ifdef for the statistics
// configuration option everywhere.
#ifdef CYGFUN_USBS_ETHDRV_STATISTICS
# define INCR_STAT(a)   \
     CYG_MACRO_START    \
     (a) += 1;          \
     CYG_MACRO_END
#else
# define INCR_STAT(a)      CYG_EMPTY_STATEMENT
#endif

// Various constants related to SNMP statistics. It is not clear
// what these are all for.
#ifdef CYGFUN_USBS_ETHDRV_STATISTICS
# define CYGDAT_USBS_ETHDRV_DESCRIPTION "eCos USB ethernet device"
#endif
// ----------------------------------------------------------------------------
// Utility functions.
//
// The TCP/IP stack works in terms of scatter/gather buffers. USB tends to
// involve DMA operations so it is more convenient to work in terms of
// 1514 byte flat buffers. Actually, the first two bytes of the buffer
// are used to hold the ethernet frame size to work around restrictions
// with certain hardware implementations of USB that may be unable to
// transfer certain packet sizes.

static bool
scatter(unsigned char* buf, struct eth_drv_sg* sg, int sg_len)
{
    unsigned int size;
    
    size = buf[0] | (buf[1] << 8);
    buf++; buf++;

    CYG_ASSERT((size >= CYGNUM_USBS_ETH_MIN_FRAME_SIZE) && (size <= CYGNUM_USBS_ETH_MAX_FRAME_SIZE),\
               "ethernet frame size limits must be observed");
        
    while ((size > 0) && (sg_len > 0)) {
        if (size > sg->len) {
            memcpy((void*) sg->buf, buf, sg->len);
            buf  += sg->len;
            size -= sg->len;
            sg++;
            sg_len--;
        } else {
            memcpy((void*) sg->buf, buf, size);
            size = 0;
        }
    }

    return 0 == size;
}

static bool
gather(unsigned char* buf, unsigned int size, struct eth_drv_sg* sg, int sg_len)
{
    unsigned int    left = size;
    unsigned char*  base = buf;

    buf++; buf++;
    while ((left > 0) && (sg_len > 0)) {
        if (left > sg->len) {
            memcpy(buf, (void*) sg->buf, sg->len);
            buf  += sg->len;
            left -= sg->len;
            sg++;
            sg_len--;
        } else {
            memcpy(buf, (void*) sg->buf, left);
            left = 0;
        }
    }
    size    = size - left;
    base[0] = size & 0x00FF;
    base[1] = (size >> 8) & 0x00FF;

    return 0 == left;
}

    
// ----------------------------------------------------------------------------
// usbs_ethdrv_init()
//
// This function is called during system initialization to decide
// whether or not this particular network device is usable. For
// USB-ethernet this is problematical, the device is only really
// usable once both sides have come up. The typical sequence
// of events is something like:
//
// 1) the eCos peripheral is powered up. Static constructors are
//    run resulting in basic initialization.
//
// 2) the eCos TCP/IP stack initialization happens. Roughly in
//    parallel the eCos USB slave side is initialized as well,
//    i.e. enumeration data is supplied to control endpoints,
//    endpoints are associated with application classes, and so
//    on. The relative order of TCP/IP and USB initialization is
//    not particularly important.
//
//    It is the TCP/IP stack's initialization code that will
//    invoke usbs_eth_init().
//
// 3) host-side USB detects that the eCos peripheral has been
//    connected or powered up. It goes through the enumeration
//    process and will end up loading a host-side network driver.
//    This connects to the eCos-side USB ethernet code to
//    e.g. obtain the MAC address. 
//
// 4) when the host-side is ready, the eCos side can be brought up.
//    The required call is (sc->funs->eth_drv->init)(sc, enaddr)
//
// In practice it is easier for now to invoke the init() function
// immediately. There are not going to be any incoming packets
// until the host is ready, and can_send() can just return false
// for the time being.
//
// Invoked in: thread context only
// ----------------------------------------------------------------------------

static bool
usbs_ethdrv_init(struct cyg_netdevtab_entry* ndp)
{
    struct eth_drv_sc*  sc  = (struct eth_drv_sc*)(ndp->device_instance);
    usbs_eth*           eth = (usbs_eth*)(sc->driver_private);

    (*sc->funs->eth_drv->init)(sc, eth->ecos_MAC);
    return true;
}

// ----------------------------------------------------------------------------
// The receive process that is used to transfer a received ethernet
// packet into the stack. The calling sequence is somewhat convoluted.
// It started off as:
//
//   1) Ethernet hw ISR invoked by hardware, schedules its own
//      hw_dsr(), and blocks further interrupts in the ethernet chip
//   2) hw_dsr() calls generic eth_drv_dsr() from io/eth common package
//   3) eth_drv_dsr() interacts with the TCP/IP stack and allocates mbufs
//      (typically, the TCP/IP stack might not be in use)
//   4) eth_drv_dsr() calls usbs_eth_recv() to transfer the data to mbufs
//   5) eth_drv_dsr() returns to hw_dsr() which reenables interrupts
//   6) hw_dsr() completes and everything can proceed.
//
// The problem with this is that the whole ethernet packet gets copied
// inside a DSR, affecting dispatch latency (but not interrupt latency).
// This is bad. Hence there is an alternative route involving a separate
// thread in the TCP/IP stack.
//
//   1) Ethernet hw ISR runs as before, scheduling hw_dsr()
//   2) hw_dsr() calls up into eth_drv_dsr()
//   3) eth_drv_dsr() wakes up a thread inside the TCP/IP stack
//   4) eth_drv_dsr() returns to hw_dsr(), which performs no further
//      processing. Ethernet chip interrupts remain disabled.
//   5) The TCP/IP thread ends up calling hw_deliver(). This should take
//      care of any pending activity. For every buffered packet there should
//      be a call to a generic recv() function which then goes back into
//      the driver-specific recv() function.
//
// The advantage is that ethernet packet copying now happens at thread
// level rather than DSR level so thread priorities can be used to
// schedule things.
//
// USB-ethernet does not interact directly with any hardware, instead
// it just passes information to lower levels of USB code. The reception
// process is started by usbs_ethdrv_start() when the TCP/IP stack brings
// up the interface. 
//
// When the USB transfer has completed a callback will be invoked, at
// DSR level. Assuming the transfer went ok, the callback will invoke
// eth_drv_dsr() to inform the higher level code. 
//
// The deliver function can check the state of the buffer
// and go through the sc->funs->eth_drv->recv()/recv() sequence
// to transfer the data into the stack. 
//
// usbs_ethdrv_recv() does a scatter from the internal buffer into the
// mbuf, thus freeing up the buffer. This allows it to start another
// receive,
//
// Synchronisation involves the scheduler lock because the recv
// callback is invoked inside a DSR.

static void usbs_ethdrv_halted_callback(void*, int);

static void
usbs_ethdrv_recv_callback(usbs_eth* eth, void* callback_data, int size)
{
    cyg_bool resubmit = true;
    
    struct eth_drv_sc* sc = (struct eth_drv_sc*) callback_data;
    CYG_ASSERT( eth == (usbs_eth*)(sc->driver_private), "USB and TCP/IP worlds need to be consistent");

    INCR_STAT(eth->interrupts);
    if (!eth->ecos_up) {
        // This message should just be discarded since the eCos TCP/IP
        // stack is not expecting anything from this interface.
        // Reception will resume when the interface comes back up.
        eth->rx_active  = false;
        resubmit        = false;      
    } else if (size < 0) {
        // An error has occurred. The likely possibilities are:
        // -EPIPE:      connection to the host has been broken
        // -EAGAIN:     the endpoint is haltedn
        // -EMSGSIZE:   bogus message from host
        // -EIO:        other

        if (-EAGAIN == size) {
            // EAGAIN should be handled by waiting for the endpoint to be reset.
            resubmit = false;
            usbs_start_rx_endpoint_wait(eth->rx_endpoint, &usbs_ethdrv_halted_callback, (void*) sc);
        } else if (-EMSGSIZE == size) {
            // Do nothing for now
        } else {
            // EPIPE should be resubmitted, the usbseth.c will use the
            // pending rx support. EIO could mean anything.
        }
    } else if (0 == size) {
        // The endpoint is no longer halted. Just do the resubmit at
        // the end.
    } else {
        // A packet has been received. Now do a size sanity check
        // based on the first two bytes.
        int real_size = eth->rx_bufptr[0] + (eth->rx_bufptr[1] << 8);
        if (real_size < CYGNUM_USBS_ETH_MIN_FRAME_SIZE) {
            INCR_STAT(eth->rx_short_frames);
        } else if (real_size > CYGNUM_USBS_ETH_MAX_FRAME_SIZE) {
            INCR_STAT(eth->rx_too_long_frames);
        } else {
            // The packet appears to be valid. Inform higher level
            // code and mark the buffer as in use.
            resubmit            = false;
            eth->rx_buffer_full = true;
            eth->rx_active      = false;
            eth_drv_dsr(0, 0, (cyg_addrword_t) sc);
        }
    }
    
    if (resubmit) {
        eth->rx_active = true;
        usbs_eth_start_rx(eth, eth->rx_bufptr, &usbs_ethdrv_recv_callback, callback_data);
    }
}

// Another callback, used to wait while an endpoint is halted.
static void
usbs_ethdrv_halted_callback(void* callback_data, int size)
{
    struct eth_drv_sc* sc = (struct eth_drv_sc*) callback_data;
    usbs_ethdrv_recv_callback((usbs_eth*) sc->driver_private, callback_data, 0);
}

// Start a receive operation. It is not possible to abort an existing
// rx operation, so a valid sequence of events is: start, rx ongoing,
// stop, restart. The rx_active field is used to keep track of whether
// or not there is still a receive in progress. The receive callback
// will just discard incoming data if the eCos stack is not currently
// running.
static void
usbs_ethdrv_start_recv(struct eth_drv_sc* sc, usbs_eth* eth)
{
    cyg_drv_dsr_lock();
    if (!eth->rx_active) {
        eth->rx_active = true;
        usbs_eth_start_rx(eth, eth->rx_bufptr, &usbs_ethdrv_recv_callback, (void*) sc);
    }
    cyg_drv_dsr_unlock();
}

// This is invoked from the delivery thread when a valid buffer
// has been received. The buffer should be scattered into the
// supplied list, then another receive should be started.

static void
usbs_ethdrv_recv(struct eth_drv_sc* sc,
                 struct eth_drv_sg* sg_list, int sg_len)
{
    usbs_eth* eth = (usbs_eth*)(sc->driver_private);

    CYG_ASSERT( eth->rx_buffer_full, "This function should only be called when there is a buffer available");
    (void) scatter(eth->rx_bufptr, sg_list, sg_len);
    eth->rx_buffer_full = false;
    eth->rx_active      = true;
    usbs_eth_start_rx(eth, eth->rx_bufptr, &usbs_ethdrv_recv_callback, (void*) sc);
}

// ----------------------------------------------------------------------------
// Now for the transmit process.
//
// When an application thread writes down a socket the data gets moved
// into mbufs, and then passed to the appropriate device driver - which
// may or may not be able to process it immediately. There is also a
// timeout thread within the TCP/IP to handle retransmits etc.
//
// The stack will start by calling usbs_ethdrv_can_send() to determine
// whether or not the driver can accept the packet. For the purposes
// of the USB-ethernet driver this is true provided both host
// and target are up and there is a spare buffer available.
//
// If the usbs_eth_can_send() returns true then there will be a call
// to usbs_ethdrv_send(). This gathers the data into a single
// buffer. If there is no transmit in progress yet then one is started.
//
// At some point the packet will have been transmitted and a callback
// gets invoked. This needs to call eth_drv_dsr(), waking up the
// delivery thread. The deliver() function can then check which
// transmissions have completed and inform the higher level code
// via sc->funs->eth_drv->tx_done(). The buffer can be re-used at
// that point.

static void
usbs_ethdrv_send_callback(usbs_eth* eth, void* callback_data, int size)
{
    struct eth_drv_sc* sc = (struct eth_drv_sc*) callback_data;
    CYG_ASSERT( eth == (usbs_eth*)(sc->driver_private), "USB and TCP/IP worlds need to be consistent");

    INCR_STAT(eth->interrupts);

    // There are a variety of possible error codes. -EAGAIN indicates
    // that the endpoint is stalled. -EPIPE indicates that the
    // connection to the host has been lost. These are not really
    // particularly interesting. Whatever happens the buffer
    // must be cleared and higher-level code informed so that
    // the mbufs can be released.
    if (size > 0) {
        INCR_STAT(eth->tx_count);
    }
    eth->tx_done = true;
    eth_drv_dsr(0, 0, (cyg_addrword_t) sc);
}

// Is it possible to send an ethernet frame? This requires
// an empty buffer, i.e. there should be no existing
// transmit in progress. It also requires that the host
// is connected and that the endpoint is not currently halted.
static int
usbs_ethdrv_can_send(struct eth_drv_sc* sc)
{
    usbs_eth* eth = (usbs_eth*)(sc->driver_private);
    return eth->host_up && !eth->tx_buffer_full && !eth->tx_endpoint->halted;
}

// Actually start a packet transmission. This means collecting
// all the data into a single buffer and then invoking the
// lower-level code. The latter may discard the packet immediately
// if the MAC is not appropriate: it would be more efficient to
// catch that here, especially for large packets, but the check
// has to happen inside the lower-level code anyway in case
// that is being invoked directly rather than via the driver.
//
// There is a possible recursion problem,
// send->start_tx->tx_done->can_send->send, which is guarded
// against using the tx_in_send flag.

static void
usbs_ethdrv_send(struct eth_drv_sc* sc,
              struct eth_drv_sg* sgl_list, int sg_len, int total_len,
              unsigned long key)
{
    usbs_eth* eth = (usbs_eth*)(sc->driver_private);
    
    CYG_ASSERT( 0 == eth->tx_in_send, "send() should not be invoked recursively");
    CYG_ASSERT( total_len <= CYGNUM_USBS_ETH_MAX_FRAME_SIZE, "ethernet maximum frame size should be observed");
    CYG_ASSERT( CYGNUM_USBS_ETH_MIN_FRAME_SIZE <= total_len, "ethernet minimum frame size should be observed");

    eth->tx_in_send = true;
    CYG_ASSERT( !eth->tx_buffer_full, "the transmit buffer should be empty");
    gather(eth->tx_buffer, CYGNUM_USBS_ETH_MAX_FRAME_SIZE, sgl_list, sg_len);
    eth->tx_buffer_full = true;
    eth->tx_done        = false;
    eth->tx_key         = key;
    usbs_eth_start_tx(eth, eth->tx_buffer, &usbs_ethdrv_send_callback, (void*) sc);
    eth->tx_in_send = false;
}

// ----------------------------------------------------------------------------
// Deliver needs to take into account both receive and transmit buffers.

static void
usbs_ethdrv_deliver(struct eth_drv_sc* sc)
{
    usbs_eth* eth = (usbs_eth*)(sc->driver_private);

    if (eth->rx_buffer_full) {
        int size = eth->rx_bufptr[0] + (eth->rx_bufptr[1] << 8);
        (*sc->funs->eth_drv->recv)(sc, size);
    }
    if (eth->tx_done) {
        unsigned long key   = eth->tx_key;
        eth->tx_buffer_full = false;
        eth->tx_done        = false;
        (*sc->funs->eth_drv->tx_done)(sc, key, 1);
    }
}

// ----------------------------------------------------------------------------
// usbs_ethdrv_start()
//
// This gets called by the TCP/IP stack later on during
// initialization, when the stack is ready to send and receive
// packets. It may get called multiple times while the stack
// is running, with different flags values.
//
// As far as transmits are concerned, nothing needs to be done. If no
// transmit is in progress then everything is fine anyway. If a
// transmit is already in progress then it must be allowed to complete
// via the usual route. Receives should however be restarted, the
// start function has appropriate safeguards.
//
// Invoked in: thread context only
// ----------------------------------------------------------------------------

static void
usbs_ethdrv_start(struct eth_drv_sc* sc, unsigned char* enaddr, int flags)
{
    usbs_eth* eth = (usbs_eth*)(sc->driver_private);
    if (!eth->ecos_up) {
        eth->ecos_up = true;
        usbs_ethdrv_start_recv(sc, eth);
    }
}

// ----------------------------------------------------------------------------
// usbs_ethdrv_stop()
//
// Similarly this gets called by the TCP/IP stack to bring the network
// interface down. Nothing should happen for any packets currently
// being transmitted or received, that would cause confusion everywhere.
// The receive callback checks the ecos_up flag and does the right
// thing. The TCP/IP stack should not call can_send() after taking
// the interface down so no new transmits will be initiated.
//
// Invoked in: thread context only
// ----------------------------------------------------------------------------

static void
usbs_ethdrv_stop(struct eth_drv_sc* sc)
{
    usbs_eth* eth = (usbs_eth*)(sc->driver_private);
    eth->ecos_up = false;
}

// ----------------------------------------------------------------------------
// usbs_eth_ioctl()
//
// The operations to worry about here are:
//
//    SET_MAC_ADDRESS,via the SIOCSIFHWADDR ioctl
//
//    GET_IF_STATS and GET_IF_STATS_UD, to report gathered statistics.
//
// Invoked in: thread context only
// ----------------------------------------------------------------------------

static int
usbs_ethdrv_ioctl(struct eth_drv_sc* sc, unsigned long key, void* data, int data_length)
{
    usbs_eth* eth = (usbs_eth*)(sc->driver_private);
    int       result = EINVAL;
    
    switch(key) {
      case ETH_DRV_SET_MAC_ADDRESS:
        {
            if (6 == data_length) {
                memcpy(eth->ecos_MAC, data, 6);
                result = 0;
            }
        }
        break;
#if defined(CYGFUN_USBS_ETHDRV_STATISTICS) && defined(ETH_DRV_GET_IF_STATS_UD)
      case ETH_DRV_GET_IF_STATS_UD:
      case ETH_DRV_GET_IF_STATS:
        {
            static unsigned char my_chipset[] = { 0, 0 };
            struct ether_drv_stats *p = (struct ether_drv_stats*) data;
            int    i;
            strcpy(p->description, CYGDAT_USBS_ETHDRV_DESCRIPTION);
            for ( i = 0; i < SNMP_CHIPSET_LEN; i++ ) {
                if ( 0 == (p->snmp_chipset[i] = my_chipset[i]) ) {
                    break;
                }
            }
            p->duplex               = 3;        // 3 == duplex
            p->operational          = (eth->host_up && eth->ecos_up) ? 3 : 2;   // 3 == up, 2 == down
            p->speed                = 10 * 1000000;
            p->supports_dot3        = 1;
            p->rx_too_long_frames   = eth->rx_too_long_frames;
            p->rx_short_frames      = eth->rx_short_frames;
            p->interrupts           = eth->interrupts;
            p->rx_count             = eth->rx_count;
            p->tx_count             = eth->tx_count;
            p->tx_queue_len         = 1;
            result=0;
        }
        break;
#endif
        
      default:
        break;
    }

    return result;
}

                 
// ----------------------------------------------------------------------------
// usbs_ethdrv_poll()
//
// On real ethernet hardware this is used by RedBoot once the
// application has started running, so that the network device can be
// used for debugging purposes as well as for the application's own
// needs. The lower-level USB device may supply a poll function as well.
// ----------------------------------------------------------------------------
static void
usbs_ethdrv_poll(struct eth_drv_sc* sc)
{
    usbs_eth*   eth = (usbs_eth*)(sc->driver_private);
    (*eth->control_endpoint->poll_fn)(eth->control_endpoint);
}

// ----------------------------------------------------------------------------
// usbs_ethdrv_intvector()
//
// See usbs_eth_poll().
// ----------------------------------------------------------------------------

static int
usbs_ethdrv_intvector(struct eth_drv_sc* sc)
{
    usbs_eth*   eth = (usbs_eth*)(sc->driver_private);
    return eth->control_endpoint->interrupt_vector;
}


