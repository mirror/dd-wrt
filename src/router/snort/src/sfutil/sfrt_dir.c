/****************************************************************************
 *
 * Copyright (C) 2006-2011 Sourcefire, Inc.
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
 * @file    sfdir.c
 * @author  Adam Keeton <akeeton@sourcefire.com>
 * @date    Thu July 20 10:16:26 EDT 2006
 *
 * The implementation uses an multibit-trie that is similar to Gupta et-al's 
 * DIR-n-m.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h> /* For variadic */
#include <stdio.h>
#include <string.h> /* For memset   */
#include "sfrt.h"
#include "sfrt_dir.h"

#if SIZEOF_UNSIGNED_LONG_INT == 8
#define ARCH_WIDTH 64
#else
#define ARCH_WIDTH 32
#endif

#ifdef SUP_IP6
typedef struct {
    IP ip;
    int bits;
} IPLOOKUP;
#else
typedef IP IPLOOKUP;
#endif

/* Create new "sub" table of 2^width entries */
static dir_sub_table_t *_sub_table_new(dir_table_t *root, uint32_t dimension, 
                                       uint32_t prefill, uint32_t bit_length)
{

    int width = root->dimensions[dimension];   
    int len = 1 << width;
    int index;
    dir_sub_table_t *sub;

    /* Check if creating this node will exceed the memory cap.
     * The symbols in the conditional (other than cap), come from the 
     * allocs below. */
    if( root->mem_cap < ( root->allocated + 
                          sizeof(dir_sub_table_t) + 
                          sizeof(word) * len + len ) ||
        bit_length > 128)
    {
        return NULL;
    }

    /* Set up the initial prefilled "sub table" */
    sub = (dir_sub_table_t*)malloc(sizeof(dir_sub_table_t));

    if(!sub)
    {
        return NULL;
    }

    /* This keeps the width readily available rather than recalculating it
     * from the number of entries during an insert or lookup */
    sub->width = width;

    /* need 2^sub->width entries */
    sub->num_entries = len;

    sub->entries = (word*)malloc(sizeof(word) * sub->num_entries);

    if(!sub->entries)
    {
        free(sub);
        return NULL;
    }

    /* A "length" needs to be stored with each entry above.  The length refers
     * to how specific the insertion that set the entry was.  It is necessary
     * so that the entry is not overwritten by less general routing 
     * information if "RT_FAVOR_SPECIFIC" insertions are being performed. */
    sub->lengths = (char*)malloc(sub->num_entries); 

    if(!sub->lengths)
    {
        free(sub->entries);
        free(sub);
        return NULL;
    }

    /* Can't use memset here since prefill is multibyte */
    for(index = 0; index < sub->num_entries; index++)
    {
        sub->entries[index] = prefill;
        sub->lengths[index] = (char)bit_length;
    }

    sub->cur_num = 0;

    root->allocated += sizeof(dir_sub_table_t) + sizeof(word) * sub->num_entries;

    root->cur_num++;

    return sub;
}

/* Create new dir-n-m root table with 'count' depth */
dir_table_t *sfrt_dir_new(uint32_t mem_cap, int count,...)
{
    va_list ap;
    uint32_t val;
    int index;

    dir_table_t* table = (dir_table_t*)malloc(sizeof(dir_table_t));

    if(!table)
    {
        return NULL;
    }

    table->allocated = 0;

    table->dimensions = (int*)malloc(sizeof(int)*count);

    if(!table->dimensions)
    {
        free(table);
        return NULL;
    }

    table->dim_size = count;

    va_start(ap, count);

    for(index=0; index < count; index++)
    {
        val = va_arg(ap, int);
        table->dimensions[index] = val;
    }

    va_end(ap);

    table->mem_cap = mem_cap;

    table->cur_num = 0;

    table->sub_table = _sub_table_new(table, 0, 0, 0);

    if(!table->sub_table)
    {
        free(table->dimensions);
        free(table);
        return NULL;
    }

    table->allocated += sizeof(dir_table_t) + sizeof(int)*count;

    return table;
}

/* Traverse "sub" tables, freeing each */
static void _sub_table_free(uint32_t *allocated, dir_sub_table_t *sub)
{
    int index;

    sub->cur_num--;

    for(index=0; index < sub->num_entries; index++)
    {
        /* The following condition will only be true if 
         * this entry is a pointer  */
        if( !sub->lengths[index] && sub->entries[index] )
        {
            _sub_table_free( allocated, (dir_sub_table_t*) sub->entries[index]);
        }
    }

    if(sub->entries)
    {
        /* This probably does not need to be checked 
         * since if it was not allocated, we would have errored out
         * in _sub_table_new */
        free(sub->entries);

        *allocated -= sizeof(word) * sub->num_entries;
    }

    if(sub->lengths)
    {
        /* This probably does not need to be checked 
         * since if it was not allocated, we would have errored out
         * in _sub_table_new */
        free(sub->lengths);

        *allocated -= sub->num_entries;
    }

    free(sub);

    *allocated -= sizeof(dir_sub_table_t);
}

/* Free the DIR-n-m structure */
void sfrt_dir_free(void *tbl)
{
    dir_table_t *table = (dir_table_t*)tbl;

    if(!table)
    {
        return;
    }

    if(table->sub_table)
    {
        _sub_table_free(&table->allocated, table->sub_table);            
    }

    if(table->dimensions)
    {
        free(table->dimensions);
    }

    free(table);
}

static INLINE void _dir_fill_all(uint32_t *allocated, uint32_t index, uint32_t fill, 
                                 word length, uint32_t val, dir_sub_table_t *table)
{

    /* Fill entries */
    for(; index < fill; index++)
    {
        /* Before overwriting this entry, verify there's not an existing
         * pointer ... otherwise free it to avoid a huge memory leak. */
        if( table->entries[index] && !table->lengths[index])
        {
            _sub_table_free(allocated, (dir_sub_table_t*)table->entries[index]); 
        }

        table->entries[index] = val;
        table->lengths[index] = (char)length;
    }
}

static INLINE void _dir_fill_less_specific(int index, int fill, 
                                           word length, uint32_t val, dir_sub_table_t *table)
{

    /* Fill entries */
    for(; index < fill; index++)
    {
        /* If we encounter a pointer, and we're inserting at this level, we 
         * automatically know that this entry refers to more specific 
         * information.  However, there might only be one more specific entry
         * in the entire block, meaning the rest must be filled.
         *
         * For instance, imagine a 24-8 with 1.2.3/24 -> A and 1.2.3.4/32 -> B
         * There will be a pointer at 1.2.3 in the first table. The second
         * table needs to have 255 entries pointing A, and 1 entry pointing to
         * B.
         *
         * Therefore, recurse to this next level. */

        if( !table->lengths[index] && table->entries[index])
        {
            dir_sub_table_t *next = (dir_sub_table_t*)table->entries[index];          
            _dir_fill_less_specific(0, 1 << next->width, length, val, next);
        }
        else if(length >= (word)table->lengths[index])
        {
            table->entries[index] = val;
            table->lengths[index] = (char)length;
        }
    }
}

/* Sub table insertion
 * This is called by dir_insert and recursively to find the the sub table
 * that should house the value "ptr"
 * @param ip        IP address structure
 * @param cur_len   Number of bits of the IP left at this depth
 * @param length    Number of bits of the IP used to specify this CIDR 
 * @param ptr       Information to be associated with this IP range
 * @param master_table    The table that describes all, returned by dir_new */
static int _dir_sub_insert(IPLOOKUP *ip, int length, int cur_len, GENERIC ptr, 
                           int current_depth, int behavior, 
                           dir_sub_table_t *sub_table, dir_table_t *root_table)
{

    word index;
    uint32_t fill;
#ifdef SUP_IP6
    {
        uint32_t local_index, i;
        /* need to handle bits usage across multiple 32bit vals within IPv6. */
        if (ip->ip->family == AF_INET)
        {
            i=0;
        }
        else if (ip->ip->family == AF_INET6)
        {
            if (ip->bits < 32 )
            {
                i=0;
            }
            else if (ip->bits < 64)
            {
                i=1;
            }
            else if (ip->bits < 96)
            {
                i=2;
            }
            else
            {
                i=3;
            }
        }
        else
        {
            return RT_INSERT_FAILURE;
        }
        local_index = ip->ip->ip32[i] << (ip->bits %32);
        index = local_index >> (ARCH_WIDTH - sub_table->width);
    }
#else
    IPLOOKUP iplu;
    /* Index is determined by the highest 'len' bits in 'ip' */
    index = *ip >> (ARCH_WIDTH - sub_table->width);
#endif

    /* Check if this is the last table to traverse to */
    if(sub_table->width >= cur_len)
    {
        /* Calculate how many entries need to be filled 
         * in this table. If the table is 24 bits wide, and the entry
         * is 20 bytes long, 2^4 entries need to be filled. */
        fill = 1 << (sub_table->width - cur_len); 

        index = (index >> (sub_table->width - cur_len)) <<
            (sub_table->width - cur_len);

        fill += index;

        /* Favor most recent CIDR */
        if(behavior == RT_FAVOR_TIME)
        {
            _dir_fill_all(&root_table->allocated, index, fill, length, 
                          (word)ptr, sub_table);
        }
        /* Fill over less specific CIDR */
        else
        {
            _dir_fill_less_specific(index, fill, length, (word)ptr, sub_table);
        }
    }
    /* Need to traverse to a sub-table */
    else
    {
        dir_sub_table_t *next_sub = 
            (dir_sub_table_t *)sub_table->entries[index];

        /* Check if we need to alloc a new sub table. 
         * If next_sub was 0/NULL, there's no entry at this index
         * If the length is non-zero, there is an entry */
        if(!next_sub || sub_table->lengths[index])
        {
            if( root_table->dim_size <= current_depth )
            {
                return RT_INSERT_FAILURE;
            }

            sub_table->entries[index] = 
                (word) _sub_table_new(root_table, current_depth+1, 
                                      (word) next_sub, sub_table->lengths[index]);

            sub_table->cur_num++;

            sub_table->lengths[index] = 0;

            next_sub = (dir_sub_table_t *)sub_table->entries[index];

            if(!next_sub)
            {
                return MEM_ALLOC_FAILURE;
            }
        }
        /* Recurse to next level.  Rightshift off appropriate number of
         * bits and update the length accordingly. */
#ifdef SUP_IP6
        ip->bits += sub_table->width;
        _dir_sub_insert(ip, length,
                        cur_len - sub_table->width, ptr, current_depth+1, 
                        behavior, next_sub, root_table);
#else
        iplu = *ip << sub_table->width;
        _dir_sub_insert(&iplu, length,
                        cur_len - sub_table->width, ptr, current_depth+1, 
                        behavior, next_sub, root_table);
#endif
    }

    return RT_SUCCESS;
}

/* Insert entry into DIR-n-m tables
 * @param ip        IP address structure
 * @param len       Number of bits of the IP used for lookup
 * @param ptr       Information to be associated with this IP range
 * @param master_table    The table that describes all, returned by dir_new */
int sfrt_dir_insert(IP ip, int len, word data_index, 
                    int behavior, void *table)
{
    dir_table_t *root = (dir_table_t*)table;
#ifdef SUP_IP6
    IPLOOKUP iplu;
    iplu.ip = ip;
    iplu.bits = 0;
#else
    IPLOOKUP iplu = ip;
#endif

    /* Validate arguments */
    if(!root || !root->sub_table)
    {
        return DIR_INSERT_FAILURE;
    }

    /* Find the sub table in which to insert */
    return _dir_sub_insert(&iplu, len, len, (GENERIC)data_index,
                           0, behavior, root->sub_table, root);
}

/* Traverse sub tables looking for match */
/* Called by dir_lookup and recursively */
static tuple_t _dir_sub_lookup(IPLOOKUP *ip, dir_sub_table_t *table)
{
    word index;
#ifdef SUP_IP6
    {
        uint32_t local_index, i;
        /* need to handle bits usage across multiple 32bit vals within IPv6. */
        if (ip->ip->family == AF_INET)
        {
            i=0;
        }
        else if (ip->ip->family == AF_INET6)
        {
            if (ip->bits < 32 )
            {
                i=0;
            }
            else if (ip->bits < 64)
            {
                i=1;
            }
            else if (ip->bits < 96)
            {
                i=2;
            }
            else
            {
                i=3;
            }
        }
        else
        {
            tuple_t ret = { 0, 0 };
            return ret;
        }
        local_index = ip->ip->ip32[i] << (ip->bits %32);
        index = local_index >> (ARCH_WIDTH - table->width);
    }
#else
    IPLOOKUP iplu;
    index = *ip >> (ARCH_WIDTH - table->width);
#endif

    if( !table->entries[index] || table->lengths[index] )
    {
        tuple_t ret;
        ret.index = table->entries[index];
        ret.length = (word)table->lengths[index];

        return ret;
    }

#ifdef SUP_IP6
    ip->bits += table->width;
    return _dir_sub_lookup( ip, (dir_sub_table_t *)table->entries[index]);
#else
    iplu = *ip << table->width;
    return _dir_sub_lookup( &iplu, (dir_sub_table_t *)table->entries[index]);
#endif
}

/* Lookup information associated with the value "ip" */
tuple_t sfrt_dir_lookup(IP ip, void *tbl)
{
    dir_table_t *root = (dir_table_t*)tbl;
#ifdef SUP_IP6
    IPLOOKUP iplu;
    iplu.ip = ip;
    iplu.bits = 0;
#else
    IPLOOKUP iplu = ip;
#endif

    if(!root || !root->sub_table)
    {
        tuple_t ret = { 0, 0 };

        return ret;
    }

    return _dir_sub_lookup(&iplu, root->sub_table);
}


uint32_t sfrt_dir_usage(void *table)
{
    if(!table)
    {
        return 0;
    }

    return ((dir_table_t*)(table))->allocated;
}

