/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

#ifdef SUP_IP6
#include "ipv6_port.h"
#endif

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

#ifdef SUP_IP6
typedef struct {
    sfip_t ip;
    PORTSET portset;
    char notflag;
} IP_PORT;

typedef struct {
    SF_LIST ip_list;
} IPSET;
#else
enum {
  NOFAMILY,
  IPV4_FAMILY,
  IPV6_FAMILY
};

enum {
  IPV4_LEN=4,
  IPV6_LEN=16
};

typedef struct {

  int family;
  unsigned char ip[IPV6_LEN];

}IPADDRESS ;


typedef struct {

  int family;
  unsigned char ip[IPV4_LEN];

}IPADDRESS4 ;

typedef struct {

  int family;
  unsigned char ip[IPV6_LEN];

}IPADDRESS6 ;

typedef struct {
   unsigned mask;
   unsigned ip;
   PORTSET  portset;
   int      notflag;
}CIDRBLOCK;

typedef struct {
   unsigned short mask[8];
   unsigned short ip[8];
   PORTSET        portset;
   int            notflag;
}CIDRBLOCK6;

typedef struct {

  int       family;
  SF_LIST   cidr_list;

}IPSET;

/*

	IP ADDRESS OBJECT
	
	This interface is meant to hide the differences between ipv4
	and ipv6.  The assumption is that when we get a raw address we
	can stuff it into a generic IPADDRESS.  When we need to test
	an IPADDRESS against a raw address we know the family opf the
	raw address.  It's either ipv4 or ipv6.

*/
int ip_familysize( int family );

int ip4_sprintx( char * s, int slen, void * ip4 );
int ip6_sprintx( char * s, int slen, void * ip6 );


IPADDRESS * ip_new   ( int family );
void        ip_free  ( IPADDRESS * p );
int         ip_family( IPADDRESS * p );
int         ip_size  ( IPADDRESS * p );
int         ip_set   ( IPADDRESS * ia, void * ip, int family );
int         ip_get   ( IPADDRESS * ia, void * ip, int family );
int         ip_equal ( IPADDRESS * ia, void * ip, int family );
int         ip_eq    ( IPADDRESS * ia, IPADDRESS * ib );
int         ip_sprint( char * s, int slen, IPADDRESS * p );
int         ip_fprint( FILE * fp, IPADDRESS * p );
#endif /* SUP_IP6 */


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
#ifdef SUP_IP6
IPSET * ipset_new     (void);
int     ipset_add     ( IPSET * ipset, sfip_t *ip, void * port, int notflag);
int     ipset_contains( IPSET * ipset, sfip_t *ip, void * port);
#else
IPSET * ipset_new     ( int family );
int     ipset_add     ( IPSET * ipset, void * ip, void * mask, void * port, int notflag, int family );
int     ipset_contains( IPSET * ipset, void * ip, void * port, int family );
#endif
IPSET * ipset_copy    ( IPSET * ipset );
void    ipset_free    ( IPSET * ipset );
int     ipset_print   ( IPSET * ipset );
#ifndef SUP_IP6
int     ipset_family  ( IPSET * ipset );
#endif

/* helper functions -- all the sets work in host order   
*/
int      ipset_parse(IPSET * ipset, char *ipstr);

#endif
