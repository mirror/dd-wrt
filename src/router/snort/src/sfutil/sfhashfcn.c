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
     sfhashfcn.c

     Each hash table must allocate it's own SFGHASH struct, this is because
     sfghash_new uses the number of rows in the hash table to modulo the random
     values.

     Updates:

     8/31/2006 - man - changed to use sfprimetable.c
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"


#ifndef MODULUS_HASH
# include "snort.h"
#endif

#include "sfhashfcn.h"
#include "sfprimetable.h"


SFHASHFCN * sfhashfcn_new( int m )
{
    SFHASHFCN * p;
    static int one=1;

    if( one ) /* one time init */
    {
        srand( (unsigned) time(0) );
        one = 0;
    }

    // This can make all of the hashing static for testing.
    //#define rand() 0

    p = (SFHASHFCN*) calloc( 1,sizeof(SFHASHFCN) );
    if( !p )
        return 0;

#ifndef MODULUS_HASH
#ifndef DYNAMIC_PREPROC_CONTEXT
    if (ScStaticHash())
    {
        sfhashfcn_static(p);
    }
    else
#endif
#endif
    {
        p->seed     = sf_nearest_prime( (rand()%m)+3191 );
        p->scale    = sf_nearest_prime( (rand()%m)+709 );
        p->hardener = (rand()*rand()) + 133824503;
    }

    p->hash_fcn   = &sfhashfcn_hash;
    p->keycmp_fcn = &memcmp;

    return p;
}

void sfhashfcn_free( SFHASHFCN * p )
{
   if( p )
   {
       free( p);
   }
}

void sfhashfcn_static( SFHASHFCN * p )
{
    p->seed     = 3193;
    p->scale    = 719;
    p->hardener = 133824503;
}

unsigned sfhashfcn_hash( SFHASHFCN * p, unsigned char *d, int n )
{
    unsigned hash = p->seed;
    while( n )
    {
        hash *=  p->scale;
        hash += *d++;
        n--;
    }
    return hash ^ p->hardener;
}

/**
 * Make sfhashfcn use a separate set of operators for the backend.
 *
 * @param h sfhashfcn ptr
 * @param hash_fcn user specified hash function
 * @param keycmp_fcn user specified key comparisoin function
 */
int sfhashfcn_set_keyops( SFHASHFCN *h,
                          unsigned (*hash_fcn)( SFHASHFCN * p,
                                                unsigned char *d,
                                                int n),
                          int (*keycmp_fcn)( const void *s1,
                                             const void *s2,
                                             size_t n))
{
    if(h && hash_fcn && keycmp_fcn)
    {
        h->hash_fcn   = hash_fcn;
        h->keycmp_fcn = keycmp_fcn;

        return 0;
    }

    return -1;
}

