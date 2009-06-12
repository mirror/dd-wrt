//==========================================================================
//
//      include/cyg/io/eth/eth_drv.h
//
//      High level networking driver interfaces
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  High level networking driver interfaces
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet driver structure

#ifndef _ETH_DRV_H_
#define _ETH_DRV_H_

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>

#ifdef CYGPKG_NET
#include <sys/param.h>
#include <sys/socket.h>

#include <net/if.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#ifndef NBPFILTER
#define NBPFILTER 0
#endif

#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

#else // !CYGPKG_NET
#include <cyg/hal/drv_api.h>
#endif
#ifdef CYGPKG_NET_LWIP
#include "lwip/netif.h"
#endif

struct eth_drv_sg {
    CYG_ADDRESS  buf;
    CYG_ADDRWORD len;
};

#define MAX_ETH_DRV_SG CYGNUM_IO_ETH_DRIVERS_SG_LIST_SIZE

struct eth_drv_sc;

struct eth_drv_funs {
    // Logical driver - initialization
    void (*init)(struct eth_drv_sc *sc, 
                 unsigned char *enaddr);
    // Logical driver - incoming packet notifier
    void (*recv)(struct eth_drv_sc *sc, 
                 int total_len);
    // Logical driver - outgoing packet notifier
    void (*tx_done)(struct eth_drv_sc *sc, 
                    CYG_ADDRESS key, 
                    int status);
};

struct eth_hwr_funs {
    // Initialize hardware (including startup)
    void (*start)(struct eth_drv_sc *sc,
                  unsigned char *enaddr,
                  int flags);
    // Shut down hardware
    void (*stop)(struct eth_drv_sc *sc);
    // Device control (ioctl pass-thru)
    int  (*control)(struct eth_drv_sc *sc,
                    unsigned long key,
                    void *data,
                    int   data_length);
    // Query - can a packet be sent?
    int  (*can_send)(struct eth_drv_sc *sc);
    // Send a packet of data
    void (*send)(struct eth_drv_sc *sc,
                 struct eth_drv_sg *sg_list,
                 int sg_len,
                 int total_len,
                 unsigned long key);
    // Receive [unload] a packet of data
    void (*recv)(struct eth_drv_sc *sc,
                 struct eth_drv_sg *sg_list,
                 int sg_len);
    // Deliver data to/from device from/to stack memory space
    // (moves lots of memcpy()s out of DSRs into thread)
    void (*deliver)(struct eth_drv_sc *sc);
    // Poll for interrupts/device service
    void (*poll)(struct eth_drv_sc *sc);
    // Get interrupt information from hardware driver
    int (*int_vector)(struct eth_drv_sc *sc);
    // Logical driver interface
    struct eth_drv_funs *eth_drv, *eth_drv_old;
};

#ifdef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
struct arpcom {
    unsigned char esa[6];
};
#endif

#ifdef CYGPKG_NET_LWIP
struct arpcom {
    struct netif ac_if;
};
#endif

struct eth_drv_sc {
    struct eth_hwr_funs *funs;
    void                *driver_private;
    const char          *dev_name;
    int                  state;
    struct arpcom        sc_arpcom; /* ethernet common */
};

#define ETH_DRV_SC(sc,priv,name,start,stop,control,can_send,send,recv,deliver,poll,int_vector) \
static void start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags); \
static void stop(struct eth_drv_sc *sc); \
static int  control(struct eth_drv_sc *sc, unsigned long key, void *data, int data_length); \
static int  can_send(struct eth_drv_sc *sc); \
static void send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, int total, unsigned long key); \
static void recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len); \
static void deliver(struct eth_drv_sc *sc); \
static void poll(struct eth_drv_sc *sc); \
static int  int_vector(struct eth_drv_sc *sc); \
static struct eth_hwr_funs sc##_funs = {        \
    start,                                      \
    stop,                                       \
    control,                                    \
    can_send,                                   \
    send,                                       \
    recv,                                       \
    deliver,                                    \
    poll,                                       \
    int_vector,                                 \
    &eth_drv_funs,                              \
    (struct eth_drv_funs *)0 };                 \
struct eth_drv_sc sc = {&sc##_funs, priv, name};

#define ETH_DRV_STATE_ACTIVE   0x0001
#define ETH_DRV_NEEDS_DELIVERY 0x0002
#define ETH_DRV_STATE_DEBUG    0x1000

// Register this as your DSR within your driver: it will cause your deliver
// routine to be called from the network thread.  The "data" parameter
// *must* be your own "struct eth_drv_sc *sc" pointer.
extern void eth_drv_dsr(cyg_vector_t vector,
                        cyg_ucount32 count,
                        cyg_addrword_t data);

extern struct eth_drv_funs eth_drv_funs;

#ifdef CYGPKG_IO_PCMCIA
#ifdef CYGPKG_NET
#include <cyg/io/eth/netdev.h>
cyg_netdevtab_entry_t *eth_drv_netdev(char *name);
#endif
#endif // CYGPKG_IO_PCMCIA

// Control 'key's
#define ETH_DRV_SET_MAC_ADDRESS 0x0100

#ifdef CYGPKG_NET
#define ETH_DRV_GET_IF_STATS_UD 0x0101
#define ETH_DRV_GET_IF_STATS    0x0102
#include <cyg/io/eth/eth_drv_stats.h> // The struct * for these ops.
#endif

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

#define ETH_DRV_SET_MC_LIST     0x0110   // Set multicast list
#define ETH_DRV_SET_MC_ALL      0x0111   // Set multicast all mode
#define ETH_DRV_MAX_MC 8
struct eth_drv_mc_list {
    int len;
    unsigned char addrs[ETH_DRV_MAX_MC][ETHER_ADDR_LEN];
};

#ifndef CYGPKG_NET
extern void eth_drv_buffers_init(void);
extern int  eth_drv_read(char *eth_hdr, char *buf, int len);
extern void eth_drv_write(char *eth_hdr, char *buf, int len);
extern int  eth_drv_int_vector(void);
extern void eth_drv_stop(void);
extern unsigned char __local_enet_addr[];
extern struct eth_drv_sc *__local_enet_sc;
#endif

#endif // _ETH_DRV_H_
