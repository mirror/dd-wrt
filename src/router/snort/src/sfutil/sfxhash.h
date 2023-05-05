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
*  sfxhash.h
*
*  generic hash table - stores and maps key + data pairs
*  (supports memcap and automatic memory recovery when out of memory)
*
*  Author: Marc Norton
*
*/

#ifndef _SFXHASH_
#define _SFXHASH_

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sfmemcap.h"
#include "sfhashfcn.h"
/*
*   ERROR DEFINES
*/
#define SFXHASH_NOMEM    -2
#define SFXHASH_ERR      -1
#define SFXHASH_OK        0
#define SFXHASH_INTABLE   1
#define SFXHASH_PENDING   2

/**
*   HASH NODE
*/
typedef struct _sfxhash_node
{
  struct _sfxhash_node * gnext, * gprev; /// global node list - used for ageing nodes
  struct _sfxhash_node * next,  * prev;  /// row node list

  int    rindex; /// row index of table this node belongs to.

  void * key;   /// Pointer to the key.
  void * data;  /// Pointer to the users data, this is not copied !

} SFXHASH_NODE;

typedef int (*SFXHASH_FREE_FCN)( void * key, void * data );
/**
*    SFGX HASH Table
*/
typedef struct _sfxhash
{
  SFHASHFCN     * sfhashfcn; /// hash function
  int             keysize;   /// bytes in key, if <= 0 -> keys are strings
  int             datasize;  /// bytes in key, if == 0 -> user data
  SFXHASH_NODE ** table;     /// array of node ptr's */
  unsigned        nrows;     /// # rows int the hash table use a prime number 211, 9871
  unsigned        count;     /// total # nodes in table

  unsigned        crow;    /// findfirst/next row in table
  unsigned        pad;
  SFXHASH_NODE  * cnode;   /// findfirst/next node ptr
  int             splay;   /// whether to splay nodes with same hash bucket

  unsigned        max_nodes; ///maximum # of nodes within a hash
  MEMCAP          mc;
  unsigned        overhead_bytes;  /// # of bytes that will be unavailable for nodes inside the table
  unsigned        overhead_blocks; /// # of blocks consumed by the table
  unsigned        find_fail;
  unsigned        find_success;

  SFXHASH_NODE  * ghead, * gtail;  /// global - root of all nodes allocated in table

  SFXHASH_NODE  * fhead, * ftail;  /// list of free nodes, which are recyled
  SFXHASH_NODE  * gnode;   /* gfirst/gnext node ptr */
  int             recycle_nodes;   /// recycle nodes. Nodes are not freed, but are used for subsequent new nodes

  /**Automatic Node Recover (ANR): When number of nodes in hash is equal to max_nodes, remove the least recently
   * used nodes and use it for the new node. anr_tries indicates # of ANR tries.*/
  unsigned        anr_tries;
  unsigned        anr_count; /// # ANR ops performaed
  int             anr_flag;  /// 0=off, !0=on
  unsigned        free_count; /// total # nodes in table

  SFXHASH_FREE_FCN anrfree;
  SFXHASH_FREE_FCN usrfree;

} SFXHASH;

/*
*   HASH PROTOTYPES
*/
int             sfxhash_calcrows(int num);
SFXHASH       * sfxhash_new( unsigned nrows, int keysize, int datasize, unsigned long memcap,
                             int anr_flag,
                             SFXHASH_FREE_FCN anrfunc,
                             SFXHASH_FREE_FCN usrfunc,
                             int recycle_flag );

void            sfxhash_set_max_nodes( SFXHASH *h, int max_nodes );

void            sfxhash_delete( SFXHASH * h );
int             sfxhash_make_empty(SFXHASH *);

int             sfxhash_add ( SFXHASH * h, void * key, void * data );
SFXHASH_NODE * sfxhash_get_node( SFXHASH * t, const void * key );
int             sfxhash_remove( SFXHASH * h, void * key );

/*!
 *  Get the # of Nodes in HASH the table
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_count( SFXHASH * t )
{
    return t->count;
}

/*!
 *  Get the # of Nodes in HASH the table
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_total_count( SFXHASH * t )
{
    return t->count + t->anr_count; 
}

/*!
 *  Get the # auto recovery
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_anr_count( SFXHASH * t )
{
    return t->anr_count;
}

/*!
 *  Get the # finds
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_find_total( SFXHASH * t )
{
    return t->find_success + t->find_fail;
}

/*!
 *  Get the # unsucessful finds
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_find_fail( SFXHASH * t )
{
    return t->find_fail;
}

/*!
 *  Get the # sucessful finds
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_find_success( SFXHASH * t )
{
    return t->find_success;
}

/*!
 *  Get the # of overhead bytes
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_overhead_bytes( SFXHASH * t )
{
    return t->overhead_bytes;
}

/*!
 *  Get the # of overhead blocks
 *
 * @param t SFXHASH table pointer
 *
 */
static inline unsigned sfxhash_overhead_blocks( SFXHASH * t )
{
    return t->overhead_blocks;
}

void          * sfxhash_mru( SFXHASH * t );
void          * sfxhash_lru( SFXHASH * t );
SFXHASH_NODE  * sfxhash_mru_node( SFXHASH * t );
SFXHASH_NODE  * sfxhash_lru_node( SFXHASH * t );
void          * sfxhash_find( SFXHASH * h, void * key );
SFXHASH_NODE  * sfxhash_find_node( SFXHASH * t, const void * key);

SFXHASH_NODE  * sfxhash_findfirst( SFXHASH * h );
SFXHASH_NODE  * sfxhash_findnext ( SFXHASH * h );

SFXHASH_NODE  * sfxhash_ghead( SFXHASH * h );
SFXHASH_NODE  * sfxhash_gnext( SFXHASH_NODE * n );
void sfxhash_gmovetofront( SFXHASH *t, SFXHASH_NODE * hnode );


void            sfxhash_splaymode( SFXHASH * h, int mode );

void          * sfxhash_alloc( SFXHASH * t, unsigned nbytes );
void            sfxhash_free( SFXHASH * t, void * p );
int             sfxhash_free_node(SFXHASH *t, SFXHASH_NODE *node);
/* sfxhash_free_anr_lru frees a node from the free list or the LRU node */
int             sfxhash_free_anr_lru(SFXHASH *t);
/* sfxhash_free_anr frees a node from the free list */
int             sfxhash_free_anr(SFXHASH *t);

unsigned        sfxhash_maxdepth( SFXHASH * t );


int sfxhash_set_keyops( SFXHASH *h ,
                        unsigned (*hash_fcn)( SFHASHFCN * p,
                                              unsigned char *d,
                                              int n),
                        int (*keycmp_fcn)( const void *s1,
                                           const void *s2,
                                           size_t n));


SFXHASH_NODE *sfxhash_gfindfirst( SFXHASH * t );
SFXHASH_NODE *sfxhash_gfindnext( SFXHASH * t );
int sfxhash_add_return_data_ptr( SFXHASH * t, const void * key, void **data );
unsigned sfxhash_calc_maxmem(unsigned num_entries, unsigned entry_cost);
int sfxhash_change_memcap( SFXHASH * t, unsigned long new_memcap, unsigned *max_work );
#endif

