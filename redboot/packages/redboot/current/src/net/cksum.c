//==========================================================================
//
//      net/cksum.c
//
//      Stand-alone network checksum support for RedBoot
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
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <net/net.h>

/*
 * Do a one's complement checksum.
 * The data being checksum'd is in network byte order.
 * The returned checksum is in network byte order.
 */
unsigned short
__sum(word *w, int len, int init_sum)
{
    int sum = init_sum;

    union {
	volatile unsigned char c[2];
	volatile unsigned short s;
    } su;

    union {
	volatile unsigned short s[2];
	volatile int i;
    } iu;

    while ((len -= 2) >= 0)
	sum += *w++;

    if (len == -1) {
	su.c[0] = *(char *)w;
	su.c[1] = 0;
	sum += su.s;
    }

    iu.i = sum;
    sum = iu.s[0] + iu.s[1];
    if (sum > 65535)
	sum -= 65535;

    su.s = ~sum;

    return (su.c[0] << 8) | su.c[1];
}


/*
 * Compute a partial checksum for the UDP/TCP pseudo header.
 */
int
__pseudo_sum(ip_header_t *ip)
{
    int    sum;
    word   *p;

    union {
	volatile unsigned char c[2];
	volatile unsigned short s;
    } su;
    
    p = (word *)ip->source;
    sum  = *p++;
    sum += *p++;
    sum += *p++;
    sum += *p++;
    
    su.c[0] = 0;
    su.c[1] = ip->protocol;
    sum += su.s;

    sum += ip->length;
    
    return sum;
}
