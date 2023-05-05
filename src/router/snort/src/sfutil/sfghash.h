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
*
*  sfghash.h
*
*  generic hash table - stores and maps key + data pairs
*
*  Author: Marc Norton
*
*/

#ifndef _SFGHASH_
#define _SFGHASH_

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sfhashfcn.h"

/*
*   ERROR DEFINES
*/
#define SFGHASH_NOMEM    -2
#define SFGHASH_ERR      -1
#define SFGHASH_OK        0
#define SFGHASH_INTABLE   1

/*
*  Flags for ghash_new: userkeys
*/
#define GH_COPYKEYS 0
#define GH_USERKEYS 1

/*
*   Generic HASH NODE
*/
typedef struct _sfghash_node
{
  struct _sfghash_node * next, * prev;

  const void * key;   /* Copy of, or Pointer to, the Users key */
  void *data;  /* The users data, this is never copied! */

} SFGHASH_NODE;

/*
*    Generic HASH table
*/
typedef struct _sfghash
{
  SFHASHFCN    * sfhashfcn;
  int          keysize;   /* bytes in key, if < 0 -> keys are strings */
  int          userkey;   /* user owns the key */

  SFGHASH_NODE ** table;  /* array of node ptr's */
  int             nrows;  /* # rows int the hash table use a prime number 211, 9871 */

  unsigned       count;  /* total # nodes in table */

  void         (*userfree)( void * );

  int            crow;    /* findfirst/next row in table */
  SFGHASH_NODE * cnode; /* findfirst/next node ptr */

  int splay;

} SFGHASH, SFDICT;


/*
*   HASH PROTOTYPES
*/
SFGHASH * sfghash_new( int nrows, int keysize, int userkeys, void (*userfree)(void*p) );
void      sfghash_delete( SFGHASH * h );
int       sfghash_add( SFGHASH * t, const void * const key, void * const data );
int       sfghash_remove( SFGHASH * h, const void * const key);
int       sfghash_count( SFGHASH * h);
void    * sfghash_find( SFGHASH * h, const void * const key );
SFGHASH_NODE * sfghash_find_node( SFGHASH * t, const void * const key);
int       sfghash_find2(SFGHASH *, void *, void **);
SFGHASH_NODE * sfghash_findfirst( SFGHASH * h );
SFGHASH_NODE * sfghash_findnext ( SFGHASH * h );

int sfghash_set_keyops( SFGHASH *h ,
                        unsigned (*hash_fcn)( SFHASHFCN * p,
                                              unsigned char *d,
                                              int n),
                        int (*keycmp_fcn)( const void *s1,
                                           const void *s2,
                                           size_t n));


#endif

