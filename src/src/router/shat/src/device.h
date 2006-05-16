/*
 * Copyright (C) 2003-2005 Maxina GmbH - Jordan Hrycaj
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Jordan Hrycaj <jordan@mjh.teddy.net.com>
 *
 * $Id: device.h,v 1.13 2005/03/20 23:34:28 jordan Exp $
 */

#ifndef __SHAT_DEVICE_H
#define __SHAT_DEVICE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pool.h"
#include "util.h"

// /* defined in util.h */
// #define DEFAULT_TUN_IP    "2.2.2.2"
// #define DEFAULT_INBOUND   "eth1"
// #define DEFAULT_TUNBOUND  "shat0"
// #define TUN_HEADER_LENGTH 4
// #define TUN_HEADER_SIGHL  0x800 /* tun signature, long host notation */

typedef
struct _in_dev {
    int                   arp_fd;   /* arp socket instance */
    int                   eth_fd;   /* ethernet socket instance */
#ifdef __DEVICE_PRIVATE
    char         name [IFNAMSIZ];   /* interface name */
    struct in_addr           ipa;   /* primary ip address of the device */
    struct in_addr      ipn, ipm;   /* network/netmask of the device */
    struct ether_addr        mac;   /* hardware address */
    short    mtu, ifindex, flags;   /* device id, mtu */
    char      rpfilter, proxyarp;   /* restore settings on clean up */
    unsigned          size_iobuf;   /* max mtu + some extra */
    uint8_t            iobuf [1];	/* io buffer */
#endif
} IN_DEV ;


typedef
struct _tun_dev {
    int                   tun_fd;   /* standard ethernet socket instance */
#ifdef __DEVICE_PRIVATE
    char         name [IFNAMSIZ];   /* interface name */
    struct in_addr           ipa;   /* network/netmask of the device */
    short    mtu, ifindex, flags;   /* device id, mtu */
    unsigned          size_iobuf;   /* max mtu + some extra */
    int                 ioctl_fd;   /* keep open for interface operations */
    uint8_t            iobuf [1];	/* io buffer */
    /* open end */
#endif
} TUN_DEV ;


extern   void  inbound_if_close (IN_DEV *);
extern IN_DEV *inbound_if_init (const char *name,
                                IP4_POOL *ifaddr,
                                IP4_POOL *ifnetw,
                                int  capture_all,
                                int          xit);

extern TUN_DEV *tunnel_if_init  (const char *name,
                                 in_addr_t   bind, /* in host order */
                                 int          xit);

extern    void  tunnel_if_close (TUN_DEV*);
extern unsigned tunnel_route    (TUN_DEV*,
                                 in_addr_t from_ip,
                                 in_addr_t   to_ip, /* in host order */
                                 int           add,
                                 int           xit);

#endif /* __SHAT_DEVICE_H */
