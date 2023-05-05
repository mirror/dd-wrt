/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2006-2013 Sourcefire, Inc.
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
 * @file    sfrt.h
 * @author  Adam Keeton <akeeton@sourcefire.com>
 * @date    Thu July 20 10:16:26 EDT 2006
 *
 * SFRT implements two different routing table lookup methods that have been
 * adapted to return a void pointers. Any generic information may be
 * associated with a given IP or CIDR block.
 *
 * As of this writing, the two methods used are Stefan Nilsson and Gunnar
 * Karlsson's LC-trie, and a multibit-trie method similar to Gupta et-al.'s
 * DIR-n-m.  Presently, the LC-trie is used for testing purposes as the
 * current implementation does not allow for fast, dynamic inserts.
 *
 * The intended use is to associate large IP blocks with specific information;
 * such as what may be written into the table by RNA.
 *
 * NOTE: information should only move from less specific to more specific, ie:
 *
 *      First insert:  1.1.0.0/16  ->  some data
 *      Second insert: 1.1.2.3     ->  some other data
 *
 * As opposed to:
 *
 *      First insert:  1.1.2.3     ->  some other data
 *      Second insert: 1.1.0.0/16  ->  some data
 *
 * If more general information is to overwrite existing entries, the table
 * should be free'ed and rebuilt.  This is due to the difficulty of cleaning
 * out stale entries with the current implementation.  At runtime, this won't
 * be a significant issue since inserts should apply to specific IP addresses
 * and not entire blocks of IPs.
 *
 *
 * Implementation:
 *
 * The routing tables associate an index into a "data" table with each CIDR.
 * Each entry in the data table stores a pointer to actual data.  This
 * implementation was chosen so each routing entry only needs one word to
 * either index the data array, or point to another table.
 *
 * Inserts are performed by specifying a CIDR and a pointer to its associated
 * data.  Since a new routing table entry may overwrite previous entries,
 * a flag selects whether the insert favors the most recent or favors the most
 * specific.  Favoring most specific should be the default behvior.  If
 * the user wishes to overwrite routing entries with more general data, the
 * table should be flushed, rather than using favor-most-recent.
 *
 * Before modifying the routing or data tables, the insert function performs a
 * lookup on the CIDR-to-be-insertted.  If no entry or an entry *of differing
 * bit length* is found, the data is insertted into the data table, and its
 * index is used for the new routing table entry.  If an entry is found that
 * is as specific as the new CIDR, the index stored points to where the new
 * data is written into the data table.
 *
 * If more specific CIDR blocks overwrote the data table, then the more
 * general routing table entries that were not overwritten will be referencing
 * the wrong data.  Alternatively, less specific entries can only overwrite
 * existing routing table entries if favor-most-recent inserts are used.
 *
 * Because there is no quick way to clean the data-table if a user wishes to
 * use a favor-most-recent insert for more general data, the user should flush
 * the table with sfrt_free and create one anew.  Alternatively, a small
 * memory leak occurs with the data table, as it will be storing pointers that
 * no routing table entry cares about.
 *
 *
 * The API calls that should be used are:
 *  sfrt_new    - create new table
 *  sfrt_insert - insert entry
 *  sfrt_lookup - lookup entry
 *  sfrt_free   - free table
*/

#ifndef _SFRT_H_
#define _SFRT_H_

#include <stdlib.h>
#include <sys/types.h>
#include "sfrt_trie.h"
#include "snort_debug.h"
#include "ipv6_port.h"

typedef sfcidr_t *IP;
typedef void* GENERIC;   /* To be replaced with a pointer to a policy */
typedef struct
{
    word index;
    word length;
} tuple_t;


#include "sfrt_dir.h"
/*#define SUPPORT_LCTRIE */
#ifdef SUPPORT_LCTRIE
#include "sfrt_lctrie.h"
#endif

enum types
{
#ifdef SUPPORT_LCTRIE
   LCT,
#endif
   DIR_24_8,
   DIR_16x2,
   DIR_16_8x2,
   DIR_16_4x4,
   DIR_8x4,
   DIR_4x8,
   DIR_2x16,
   DIR_16_4x4_16x5_4x4,
   DIR_16x7_4x4,
   DIR_16x8,
   DIR_8x16,
   IPv4,
   IPv6
};

enum return_codes
{
   RT_SUCCESS=0,
   RT_INSERT_FAILURE,
   RT_POLICY_TABLE_EXCEEDED,
   DIR_INSERT_FAILURE,
   DIR_LOOKUP_FAILURE,
   MEM_ALLOC_FAILURE,
#ifdef SUPPORT_LCTRIE
   LCT_COMPILE_FAILURE,
   LCT_INSERT_FAILURE,
   LCT_LOOKUP_FAILURE,
#endif
   RT_REMOVE_FAILURE
};

/* Defined in sfrt.c */
extern char *rt_error_messages[];

enum
{
   RT_FAVOR_TIME,
   RT_FAVOR_SPECIFIC,
   RT_FAVOR_ALL
};

/*******************************************************************/
/* Master table struct.  Abstracts DIR and LC-trie methods         */
typedef struct
{
    GENERIC *data;      /* data table. Each IP points to an entry here */
    uint32_t num_ent;  /* Number of entries in the policy table */
    uint32_t max_size; /* Max size of policies array */
    uint32_t lastAllocatedIndex; /* Index allocated last. Search for unused index
                                    starts from this value and then wraps around at max_size.*/
    char ip_type;       /* Only IPs of this family will be used */
    char table_type;
    uint32_t allocated;

    void *rt;            /* Actual "routing" table */
    void *rt6;            /* Actual "routing" table */

    tuple_t (*lookup)(uint32_t* adr, int numAdrDwords, GENERIC tbl);
    int (*insert)(uint32_t* adr, int numAdrDwords, int len, word index, int behavior, GENERIC tbl);
    void (*free)(GENERIC tbl);
    uint32_t (*usage)(GENERIC tbl);
    void     (*print)(GENERIC tbl);
    word (*remove)(uint32_t* adr, int numAdrDwords, int len, int behavior, GENERIC tbl);
} table_t;
/*******************************************************************/

/* Abstracted routing table API */
table_t * sfrt_new(char type, char ip_type, long data_size, uint32_t mem_cap);
void      sfrt_free(table_t *table);
GENERIC sfrt_lookup(sfaddr_t* ip, table_t* table);
GENERIC sfrt_search(sfaddr_t* ip, table_t *table);
typedef void (*sfrt_iterator_callback)(void *);
struct _SnortConfig;
typedef void (*sfrt_sc_iterator_callback)(struct _SnortConfig *, void *);
typedef int (*sfrt_sc_iterator_callback3)(struct _SnortConfig *, void *);
typedef void (*sfrt_iterator_callback2)(void *, void *);
typedef int  (*sfrt_iterator_callback3)(void *);
void    sfrt_iterate(table_t* table, sfrt_iterator_callback userfunc);
void    sfrt_iterate_with_snort_config(struct _SnortConfig *sc, table_t* table, sfrt_sc_iterator_callback userfunc);
int     sfrt_iterate2(table_t* table, sfrt_iterator_callback3 userfunc);
int     sfrt_iterate2_with_snort_config(struct _SnortConfig *sc, table_t* table, sfrt_sc_iterator_callback3 userfunc);
void    sfrt_cleanup(table_t* table, sfrt_iterator_callback userfunc);
void    sfrt_cleanup2(table_t*, sfrt_iterator_callback2, void *);
int     sfrt_insert(sfcidr_t* ip, unsigned char len, GENERIC ptr,
                        int behavior, table_t *table);
int     sfrt_remove(sfcidr_t* ip, unsigned char len, GENERIC *ptr,
                        int behavior, table_t *table);
uint32_t     sfrt_usage(table_t *table);
void    sfrt_print(table_t *table);
uint32_t     sfrt_num_entries(table_t *table);

/* Perform a lookup on value contained in "ip"
 * For performance reason, we use this simplified version instead of sfrt_lookup
 * Note: this only applied to table setting: DIR_8x16 (DIR_16_8_4x2 for IPV4), DIR_8x4*/
static inline GENERIC sfrt_dir8x_lookup(sfaddr_t *ip, table_t* table)
{
    dir_sub_table_t *subtable;
    int i;
    void *rt = NULL;
    int index;

    if (sfaddr_family(ip) == AF_INET)
    {
        rt =  table->rt;
        subtable = ((dir_table_t *)rt)->sub_table;
         /* 16 bits*/
        index = ntohs(ip->ia16[6]);
        if( !subtable->entries[index] || subtable->lengths[index] )
        {
            return table->data[subtable->entries[index]];
        }
        subtable = (dir_sub_table_t *) subtable->entries[index];

        /* 8 bits*/
        index = ip->ia8[14];
        if( !subtable->entries[index] || subtable->lengths[index] )
        {
            return table->data[subtable->entries[index]];
        }
        subtable = (dir_sub_table_t *) subtable->entries[index];

        /* 4 bits */
        index = ip->ia8[15] >> 4;
        if( !subtable->entries[index] || subtable->lengths[index] )
        {
            return table->data[subtable->entries[index]];
        }
        subtable = (dir_sub_table_t *) subtable->entries[index];

        /* 4 bits */
        index = ip->ia8[15] & 0xF;
        if( !subtable->entries[index] || subtable->lengths[index] )
        {
            return table->data[subtable->entries[index]];
        }
    }
    else
    {

        rt =  table->rt6;
        subtable = ((dir_table_t *)rt)->sub_table;
        for (i = 0; i < 16; i++)
        {
            index = ip->ia8[i];
            if( !subtable->entries[index] || subtable->lengths[index] )
            {
                return table->data[subtable->entries[index]];
            }
            subtable = (dir_sub_table_t *) subtable->entries[index];
        }
    }
    return NULL;

}

#endif

