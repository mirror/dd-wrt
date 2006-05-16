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
 * $Id: arp.h,v 1.10 2005/03/20 23:34:28 jordan Exp $
 */

#ifndef __SHAT_ARP_H
#define __SHAT_ARP_H

// /* defined in util.h */
// #define ARP_INBOUND_LOG       0x0004
// #define ARP_INBOUND_DROP      0x0010
// #define ARP_INBOUND_MORE      0x0040
// #define ARP_INBOUND_POLL      0x0100
// #define ARP_BATCH_DEFAULT_LENGTH 300
// #define ARP_SPOOF_REPLIES          7
// #define ARP_SPOOF_REP_INTV        10 /* secs */

#ifdef __ARP_PRIVATE
#define __DEVICE_PRIVATE
#endif

#include "util.h"
#include "device.h"
#include "lookup.h"

#ifdef __ARP_PRIVATE
/* asynchroneous idle time reply ring buffer */
typedef
struct _arply {
    struct _arply    *next;
    time_t             due; /* time to execute */
    int             repeat; /* remaining number of times to send */
    struct in_addr     sip; /* the ip address asked for */
    struct in_addr     dip; /* whom to send */
    struct ether_addr  src; /* the mac address asked for */
    struct ether_addr  dst; /* whom to send */
} ARPLY ;
#endif /* __ARP_PRIVATE */

typedef
struct _arp {
    int      spoof_replies; /* number of arp spoof frames to send */
    int    spoof_send_intv; /* time interval between frames */
#   ifdef __ARP_PRIVATE
    IN_DEV            *dev; /* interface descriptor */
    LOOKUP           *pool; /* indexed assign pool access */
    RANGE          *ignore; /* ignore these ip addresses */
    RANGE           *local; /* arp spoof when asking for these addresses */
    REGISTER         *dhcp; /* registered dhcp clients */
    struct ether_addr fake; /* arp spoof, attact by this address */
    void (*scd)(void*,int); /* idle time scheduler call back */
    void         *scd_data; /* idle time scheduler call back state data */
    ARPLY           *batch; /* things to be done later */
    ARPLY         *garbage; /* garbage collector */
    ARPLY   batch_pool [1]; /* pool to get records, from */
#   endif /* __ARP_PRIVATE */
} ARP ;

extern IN_DEV            *arp_device  (ARP*);
extern LOOKUP            *arp_pool    (ARP*);
extern RANGE             *arp_exclude (ARP*);
extern RANGE             *arp_local   (ARP*);
extern struct ether_addr *arp_mac     (ARP*);
extern void               arp_catch   (ARP*);
extern void               arp_pending (ARP*);
extern void               arp_close   (ARP*);

extern ARP *arp_init (IN_DEV           *idev,
                      LOOKUP           *pool,
                      RANGE          *ignore,
                      RANGE           *local,
                      REGISTER         *dhcp,
                      struct ether_addr *mac,
                      int          queue_len,
                      void  (*scd)(void*,int),
                      void         *scd_data);

#endif /* __SHAT_ARP_H */
