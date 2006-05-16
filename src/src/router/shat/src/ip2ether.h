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
 * $Id: ip2ether.h,v 1.9 2005/03/20 23:34:28 jordan Exp $
 */


// /* defined in util.h */
// #define IP2E_LEARNING_PHASE    300
// #define IP2E_INBOUND_LOG    0x0004
// #define IP2E_OUTBOUND_LOG   0x0008
// #define IP2E_INBOUND_DROP   0x0010
// #define IP2E_OUTBOUND_DROP  0x0020
// #define IP2E_INBOUND_MORE   0x0040
// #define IP2E_OUTBOUND_MORE  0x0080
// #define IP2E_INBOUND_POLL   0x0100
// #define IP2E_OUTBOUND_POLL  0x0200

#ifndef __SHAT_IP2ETHER_H
#define __SHAT_IP2ETHER_H

#ifdef __IP2ETHER_PRIVATE
#define __DEVICE_PRIVATE
#endif

#include "device.h"
#include "lookup.h"


typedef
struct _ip2e {
#   ifdef __IP2ETHER_PRIVATE
    IN_DEV           *idev; /* interface descriptor */
    TUN_DEV          *tdev;
    LOOKUP           *pool; /* indexed assign pool access */
    RANGE          *ignore; /* ignore these ip addresses */
    REGISTER         *dhcp; /* register dhcp mac/ip assignments */
    time_t    end_start_up;	/* register all while starting up */
#   else
    char any ;
#   endif /* __IP2ETHER_PRIVATE */
} IP2E ;

extern IP2E    *ip2ether_init (TUN_DEV*,
                               IN_DEV*,
                               LOOKUP*,
                               RANGE*,
                               REGISTER*);

extern void     ip2ether_close    (IP2E*);
extern void     ip2ether_injector (IP2E*);
extern void     ip2ether_xtractor (IP2E*);
extern void     ip2ether_ping     (IP2E*,MAC_REC*);

extern IN_DEV  *ip2ether_device   (IP2E*);
extern TUN_DEV *ip2ether_tundev   (IP2E*);
extern LOOKUP  *ip2ether_pool     (IP2E*);
extern RANGE   *ip2ether_exclude  (IP2E*);

#endif /* __SHAT_IP2ETHER_H */
