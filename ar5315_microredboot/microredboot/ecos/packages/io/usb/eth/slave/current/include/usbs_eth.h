#ifndef CYGONCE_USBS_ETH_H
#define  CYGONCE_USBS_ETH_H_
//==========================================================================
//
//      include/usbs_eth.h
//
//      Description of the USB slave-side ethernet support
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
// Purpose:
// Description:  USB slave-side ethernet support
//
//
//####DESCRIPTIONEND####
//==========================================================================

#ifdef __cplusplus
extern "C" {
#endif
    
//
// The primary purpose of the USB slave-side ethernet code is to
// provide an ethernet service for the host. Essentially this means
// the following:
//
// 1) the host can transmit an ethernet frame to the USB peripheral.
//    This frame is received by the code in this package and then
//    passed up to higher-level code for processing. Typically the
//    frame will originate from a TCP/IP stack running inside the
//    host, and the higher-level code will forward the frame via a
//    real ethernet chip or some other ethernet-style device.
//
// 2) higher-level code will provide ethernet frames to be sent to
//    the host, usually to a TCP/IP stack running on the host. The
//    exact source of the ethernet frame is not known.
//
// 3) the host may initiate a number of control operations, for
//    example it may request the MAC address or it may want to
//    control the filtering mode (e.g. enable promiscuous mode).
//
// 4) there are USB control-related operations, for example actions
//    to be taken when the peripheral is disconnected from the
//    bus or when the host wants to disable the ethernet interface.
//
// It is possible to develop a USB ethernet peripheral that does not
// involve a TCP/IP stack inside the peripheral, in fact that is the
// most common implementation. Instead a typical peripheral would
// involve a USB port, an ethernet port, and a cheap microcontroller
// just powerful enough to forward packets between the two. The eCos
// USB code can be used in this way, and the primary external
// interface provides enough functionality for this to work.
//
//                  +---------------+   ethernet
//   +----+         |               |     |
//   |    |   USB   |      app      |     |
//   |host|---------|     /   \     |-----o
//   |    |         |    /     \    |     |
//   +----+         | USB-eth   eth |     |
//                  +---------------+     |
//                   USB peripheral
//
// Note that the USB-ethernet code does not know anything about the
// real ethernet device or what the application gets up to, it just
// provides an interface to the app. The above represents just one
// possible use for a USB-ethernet device.
//
// Also worth mentioning: when the host TCP/IP stack requests the MAC
// address USB-eth would normally respond with the MAC address for the
// real ethernet device. That way things like host-side DHCP should
// just work.
//
// Alternatively for some applications it is desirable to run a TCP/IP
// stack inside the peripheral as well as on the host. This makes
// things a fair bit more complicated, something like this.
//
//                  +---------------+
//                  |      app      |
//                  |       |       |  ethernet
//   +----+         |       |       |     |
//   |    |   USB   |     TCP/IP    |     |
//   |host|---------|     /   \     |-----o
//   |    |         |    /     \    |     |
//   +----+         | USB-eth   eth |     |
//                  +---------------+     |
//                   USB peripheral
//
// 
// Usually this will involve enabling the bridge code in the TCP/IP
// stack, or possibly performing some sort of bridging below the
// TCP/IP stack. One way of getting things to work is to view the
// USB connection as a small ethernet segment with just two
// attached machines, the host and the peripheral. The two will
// need separate MAC addresses, in addition to the MAC address
// for the real ethernet device. This way the bridge code
// sees things the way it expects.    
//
// There will still be some subtle differences between a setup like
// this and a conventional ethernet bridge, mainly because there
// is a host-side TCP/IP stack which can perform control operations.
// For example the host stack may request that USB-eth go into
// promiscuous mode. A conventional ethernet bridge just deals
// with ethernet segments and does not need to worry about
// control requests coming in from one of the segments.
//
// It is not absolutely essential that there is another network.
// However without another network this setup would look to the host
// like an ethernet segment with just two machines attached to it, the
// host itself and the USB peripheral, yet it still involves all the
// complexities of ethernet such as broadcast masks and IP subnets.
// Anything along these lines is likely to prove somewhat confusing,
// and the USB peripheral should probably act like some other class
// of USB device instead.
//
// One special setup has the host acting as a bridge to another
// network, rather than the peripheral. This might make sense for
// mobile peripherals such as PDA's, as a way of connecting the
// peripheral to an existing LAN without needing a LAN adapter.
// Enabling bridging in the host may be a complex operation, limiting
// the applicability of such a setup.
//
// This package will only implement the eCos network driver interface
// if explicitly enabled. The package-specific interface is always
// provided, although trying to mix and match the two may lead to
// terrible confusion: once the network driver is active nothing else
// should use the lower-level USB ethernet code. However application
// code is responsible for initializing the package, and specifically
// for providing details of the USB endpoints that should be used.
//
// The package assumes that it needs to provide just one
// instantiation. Conceivably there may be applications where it makes
// sense for a USB peripheral to supply two separate ethernet devices
// to the host, but that would be an unusual setup. Also a peripheral
// might provide two or more USB slave ports to allow multiple hosts
// to be connected, with a separate USB-ethernet instantiation for
// each port, but again that would be an unusual setup. Applications
// which do require more than one instantiation are responsible
// for doing this inside the application code.

// The public interface depends on configuration options.
#include <pkgconf/io_usb_slave_eth.h>

// Define the interface in terms of eCos data types.
#include <cyg/infra/cyg_type.h>

// The generic USB support
#include <cyg/io/usb/usbs.h>

// Network driver definition, to support cloning of usbs_eth_netdev0
#ifdef CYGPKG_USBS_ETHDRV
# include <cyg/io/eth/netdev.h>
#endif

// Cache details, to allow alignment to cache line boundaries etc.
#include <cyg/hal/hal_cache.h>    
    
// ----------------------------------------------------------------------------
// Maximum transfer size. This is not specified by io/eth. It can be
// determined from <netinet/if_ether.h> but the TCP/IP stack may not
// be loaded so that header file cannot be used.
//
// Some (most?) USB implementations have implementation problems. For
// example the SA11x0 family cannot support transfers that are exact
// multiples of the 64-byte USB bulk packet size, instead it is
// necessary to add explicit size information. This can be encoded
// conveniently at the start of the buffer.
//
// So the actual MTU consists of:
//  1) a 1500 byte payload
//  2) the usual ethernet header with a six-byte source MAC
//     address, a six-byte destination MAC address, and a
//     two-byte protocol or length field, for a total header
//     size of 14 bytes.
//  3) an extra two bytes of size info.
//
// For a total of 1516 bytes.
#define CYGNUM_USBS_ETH_MAX_FRAME_SIZE 1514
#define CYGNUM_USBS_ETH_MAXTU (CYGNUM_USBS_ETH_MAX_FRAME_SIZE + 2)
    
// Although the minimum ethernet frame size is 60 bytes, this includes
// padding which is not needed when transferring over USB. Hence the
// actual minimum is just the 14 byte ethernet header plus two bytes
// for the length.
#define CYGNUM_USBS_ETH_MIN_FRAME_SIZE 14
#define CYGNUM_USBS_ETH_MINTU (CYGNUM_USBS_ETH_MIN_FRAME_SIZE + 2)

// Typical USB devices involve DMA operations and hence confusion
// between cached and uncached memory. To make life easier for
// the underlying USB device drivers, this package ensures that
// receive operations always involve buffers that are aligned to
// a cache-line boundary and that are a multiple of the cacheline
// size.
#ifndef HAL_DCACHE_LINE_SIZE
# define CYGNUM_USBS_ETH_RXBUFSIZE      CYGNUM_USBS_ETH_MAXTU
# define CYGNUM_USBS_ETH_RXSIZE         CYGNUM_USBS_ETH_MAXTU    
#else
# define CYGNUM_USBS_ETH_RXBUFSIZE      ((CYGNUM_USBS_ETH_MAXTU + HAL_DCACHE_LINE_SIZE + HAL_DCACHE_LINE_SIZE - 1) \
                                         & ~(HAL_DCACHE_LINE_SIZE - 1))
# define CYGNUM_USBS_ETH_RXSIZE         ((CYGNUM_USBS_ETH_MAXTU + HAL_DCACHE_LINE_SIZE - 1) & ~(HAL_DCACHE_LINE_SIZE - 1))
#endif    
    
// ----------------------------------------------------------------------------
// This data structure serves two purposes. First, it keeps track of
// the information needed by the low-level USB ethernet code, for
// example which endpoints should be used for incoming and outgoing
// packets. Second, if the support for the TCP/IP stack is enabled
// then there are additional fields to support that (e.g. for keeping
// track of statistics).
//
// Arguably the two uses should be separated into distinct data
// structures. That would make it possible to instantiate multiple
// low-level USB-ethernet devices but only have a network driver for
// one of them. Achieving that flexibility would require some extra
// indirection, affecting performance and code-size, and it is not
// clear that that flexibility would ever prove useful. For now having
// a single data structure seems more appropriate.

typedef struct usbs_eth {

    // What endpoints should be used for communication?
    usbs_control_endpoint*      control_endpoint;
    usbs_rx_endpoint*           rx_endpoint;
    usbs_tx_endpoint*           tx_endpoint;
    
    // Is the host ready to receive packets? This state is determined
    // largely by control packets sent from the host. It can change at
    // DSR level.
    volatile cyg_bool   host_up;

    // Has the host-side set promiscuous mode? This is relevant to the
    // network driver which may need to do filtering based on the MAC
    // address and host-side promiscuity.
    volatile cyg_bool   host_promiscuous;

    // The host MAC address. This is the address supplied to the
    // host's TCP/IP stack and filled in by the init function. There
    // is no real hardware to extract the address from.
    unsigned char       host_MAC[6];

    // Needed for callback operations.
    void                (*tx_callback_fn)(struct usbs_eth*, void*, int);
    void*               tx_callback_arg;

    void                (*rx_callback_fn)(struct usbs_eth*, void*, int);
    void*               rx_callback_arg;

    // RX operations just block if the host is not connected, resuming
    // when a connection is established. This means saving the buffer
    // pointer so that when the host comes back up the rx operation
    // proper can start. This is not quite consistent because if the
    // connection breaks while an RX is in progress there will be a
    // callback with an error code whereas an RX on a broken
    // connection just blocks, but this does fit neatly into an
    // event-driven I/O model.
    unsigned char*      rx_pending_buf;
    
#ifdef CYGPKG_USBS_ETHDRV
    // Has the TCP/IP stack brought up this interface yet?
    cyg_bool            ecos_up;

    // Is there an ongoing receive? Cancelling a receive operation
    // during a stop() may be difficult, and a stop() may be followed
    // immediately by a restart.
    cyg_bool            rx_active;
    
    // The eCos-side MAC. If the host and the eCos stack are to
    // communicate then they must be able to address each other, i.e.
    // they need separate addresses. Again there is no real hardware
    // to extract the address from so it has to be supplied by higher
    // level code via e.g. an ioctl().
    unsigned char       ecos_MAC[6];
    
    // SNMP statistics
# ifdef CYGFUN_USBS_ETHDRV_STATISTICS
    unsigned int        interrupts;
    unsigned int        tx_count;
    unsigned int        rx_count;
    unsigned int        rx_short_frames;
    unsigned int        rx_too_long_frames;
# endif
    
    // The need for a receive buffer is unavoidable for now because
    // the network driver interface does not support pre-allocating an
    // mbuf and then passing it back to the stack later. Ideally the
    // rx operation would read a single USB packet, determine the
    // required mbuf size from the 2-byte header, copy the initial
    // data, and then read more USB packets. Alternatively, a
    // 1516 byte mbuf could be pre-allocated and then the whole
    // transfer could go there, potentially wasting some mbuf space.
    // None of this is possible at present.
    //
    // Also, typically there will be complications because of
    // dependencies on DMA, cached vs. uncached memory, etc.
    unsigned char       rx_buffer[CYGNUM_USBS_ETH_RXBUFSIZE];
    unsigned char*      rx_bufptr;
    cyg_bool            rx_buffer_full;

    // It should be possible to eliminate the tx buffer. The problem
    // is that the protocol requires 2 bytes to be prepended, and that
    // may not be possible with the buffer supplied by higher-level
    // code. Eliminating this buffer would either require USB
    // device drivers to implement gather functionality on transmits,
    // or it would impose a dependency on higher-level code.
    unsigned char       tx_buffer[CYGNUM_USBS_ETH_MAXTU];
    cyg_bool            tx_buffer_full;
    cyg_bool            tx_done;
    unsigned long       tx_key;
    
    // Prevent recursion send()->tx_done()->can_send()/send()
    cyg_bool            tx_in_send;
#endif
    
} usbs_eth;

// The package automatically instantiates one USB ethernet device.
extern usbs_eth usbs_eth0;

// ----------------------------------------------------------------------------
// If the network driver option is enabled then the package also
// provides a single cyg_netdevtab_entry. This is exported so that
// application code can clone the entry.
#ifdef CYGPKG_USBS_ETHDRV
extern cyg_netdevtab_entry_t usbs_eth_netdev0;    
#endif    
    
// ----------------------------------------------------------------------------
// A C interface to the low-level USB code.
    
// Initialize the USBS-eth support for a particular usbs_eth device.
// This associates a usbs_eth structure with specific endpoints.
extern void usbs_eth_init(usbs_eth*, usbs_control_endpoint*, usbs_rx_endpoint*, usbs_tx_endpoint*, unsigned char*);
    
// Start an asynchronous transmit of a single buffer of up to
// CYGNUM_USBS_ETH_MAXTU bytes. This buffer should contain a 2-byte
// size field, a 14-byte ethernet header, and upto 1500 bytes of
// payload. When the transmit has completed the callback function (if
// any) will be invoked with the specified pointer. NOTE: figure out
// what to do about error reporting
extern void usbs_eth_start_tx(usbs_eth*, unsigned char*, void (*)(usbs_eth*, void*, int), void*);

// Start an asynchronous receive of an ethernet packet. The supplied
// buffer should be at least CYGNUM_USBS_ETH_MAXTU bytes. When a
// complete ethernet frame has been received or when some sort of
// error occurs the callback function will be invoked. The third
// argument
extern void usbs_eth_start_rx(usbs_eth*, unsigned char*, void (*)(usbs_eth*, void*, int), void*);

// The handler for application class control messages. The init call
// will install this in the control endpoint by default. However the
// handler is fairly dumb: it assumes that all application control
// messages are for the ethernet interface and does not bother to
// check the control message's destination. This is fine for simple
// USB ethernet devices, but for any kind of multi-function peripheral
// higher-level code will have to perform multiplexing and invoke this
// handler only when appropriate.
extern usbs_control_return usbs_eth_class_control_handler(usbs_control_endpoint*, void*);

// Similarly a handler for state change messages. Installing this
// means that the ethernet code will have sufficient knowledge about
// the state of the USB connection for simple ethernet-only
// peripherals, but not for anything more complicated. In the latter
// case higher-level code will need to keep track of which
// configuration, interfaces, etc. are currently active and explicitly
// enable or disable the ethernet device using the functions below.
extern void usbs_eth_state_change_handler(usbs_control_endpoint*, void*, usbs_state_change, int);
extern void usbs_eth_disable(usbs_eth*);
extern void usbs_eth_enable(usbs_eth*);    
    
#ifdef __cplusplus
} // extern "C"
#endif

#endif // CYGONCE_USBS_ETH_H_
