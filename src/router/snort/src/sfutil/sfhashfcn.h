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
	sfhashfcn.h
*/
#ifndef SFHASHFCN_INCLUDE 
#define SFHASHFCN_INCLUDE 

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}   

typedef struct _SFHASHFCN {

 unsigned seed;
 unsigned scale;
 unsigned hardener;
 unsigned (*hash_fcn)(struct _SFHASHFCN * p,
                      unsigned char *d,
                      int n );
 int      (*keycmp_fcn)( const void *s1,
                         const void *s2,
                         size_t n);
} SFHASHFCN;

SFHASHFCN * sfhashfcn_new( int nrows );
void sfhashfcn_free( SFHASHFCN * p );
void sfhashfcn_static( SFHASHFCN * p );

unsigned sfhashfcn_hash( SFHASHFCN * p, unsigned char *d, int n );

int sfhashfcn_set_keyops( SFHASHFCN * p,
                          unsigned (*hash_fcn)( SFHASHFCN * p,
                                                unsigned char *d,
                                                int n),
                          int (*keycmp_fcn)( const void *s1,
                                             const void *s2,
                                             size_t n));



#endif
