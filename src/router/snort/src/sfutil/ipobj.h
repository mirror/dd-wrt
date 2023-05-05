/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/*

	ipobj.h

	IP address encapsulation interface

	This module provides encapsulation of single IP ADDRESSes as objects,
	and collections of IP ADDRESSes as objects

        Interaction with this library should be done in HOST byte order.

*/
#ifndef IPOBJ_SNORT
#define IPOBJ_SNORT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sflsq.h"

#include "ipv6_port.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

typedef struct {
   unsigned port_lo;
   unsigned port_hi;
}PORTRANGE;

typedef struct {
   SF_LIST port_list;
}PORTSET;

typedef struct {
    sfcidr_t ip;
    PORTSET portset;
    char notflag;
} IP_PORT;

typedef struct {
    SF_LIST ip_list;
} IPSET;


/*

  IP ADDRESS SET OBJECTS


   Snort Accepts:

	IP-Address		192.168.1.1
	IP-Address/MaskBits	192.168.1.0/24
	IP-Address/Mask		192.168.1.0/255.255.255.0


   These can all be handled via the CIDR block notation : IP/MaskBits

   We use collections (lists) of cidr blocks to represent address blocks
   and indivdual addresses.

   For a single IPAddress the implied Mask is 32 bits,or
   255.255.255.255, or 0xffffffff, or -1.
*/
IPSET * ipset_new     (void);
int     ipset_add     ( IPSET * ipset, sfcidr_t *ip, void * port, int notflag);
int     ipset_contains( IPSET * ipset, sfaddr_t *ip, void * port);
IPSET * ipset_copy    ( IPSET * ipset );
void    ipset_free    ( IPSET * ipset );
int     ipset_print   ( IPSET * ipset );

/* helper functions -- all the sets work in host order
*/
int      ipset_parse(IPSET * ipset, char *ipstr);

#endif
