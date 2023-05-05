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
#include "sf_types.h"
#include "sfrt.h"
#include "sfrt_dir.h"

typedef struct {
    uint32_t* adr;
    int bits;
} IPLOOKUP;

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
    sub->lengths = (uint8_t*)malloc(sub->num_entries);

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
        sub->lengths[index] = (uint8_t)bit_length;
    }

    sub->cur_num = 0;

    if (prefill)
        sub->filledEntries = sub->num_entries;
    else
        sub->filledEntries = 0;

    root->allocated += sizeof(dir_sub_table_t) + sizeof(word) * sub->num_entries + sub->num_entries;

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

static inline void _dir_fill_all(uint32_t *allocated, uint32_t index, uint32_t fill,
                                 word length, uint32_t val, dir_sub_table_t *table)
{

    /* Fill entries */
    for(; index < fill; index++)
    {
        /* Before overwriting this entry, verify there's not an existing
         * pointer ... otherwise free it to avoid a huge memory leak. */
        if(table->entries[index])
        {
            if (!table->lengths[index])
            {
                _sub_table_free(allocated, (dir_sub_table_t*)table->entries[index]);
            }
        }
        else
        {
            table->filledEntries++;
        }

        table->entries[index] = val;
        table->lengths[index] = (uint8_t)length;
    }
}

static inline void _dir_fill_less_specific(int index, int fill,
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
            if (!table->entries[index])
            {
                table->filledEntries++;
            }
            table->entries[index] = val;
            table->lengths[index] = (char)length;
        }
    }
}

/*Remove entries all this level and discard any more specific entries.
 *
 * @note RT_FAVOR_TIME behavior can cause hung or crosslinked entries if part of a subnet
 * (which was added) are deleted. Same issue is there when a more general subnet overwrites
 * a specific subnet. table->data[] entry for more specific subnet is not cleared.
 *
 * @note RT_FAVOR_TIME can cause orphaned table->data[] entries if the entire subnet
 * is replaced by more specific sudnets.
 */
static inline uint32_t _dir_remove_all(uint32_t *allocated, uint32_t index, uint32_t fill,
                                 word length, dir_sub_table_t *table)
{
    uint32_t valueIndex = 0;

    /* Fill entries */
    for(; index < fill; index++)
    {
        /* Before overwriting this entry, verify there's not an existing
         * pointer ... otherwise free it to avoid a huge memory leak. */
        if (table->entries[index])
        {
            if (!table->lengths[index])
            {
                _sub_table_free(allocated, (dir_sub_table_t*)table->entries[index]);
            }

            if(length == (word)table->lengths[index])
            {
                valueIndex = table->entries[index];
            }

            table->filledEntries--;

            //zero value here works since sfrt uses 0 for failed entries.
            table->entries[index] = 0;
            table->lengths[index] = 0;
        }
    }

    return valueIndex;
}

/**Remove entries which match in address/length in all subtables.
 * @note RT_FAVOR_SPECIFIC can cause orphaned table->data[] entries if the entire subnet
 * is replaced by more specific subnets.
 */
static inline uint32_t _dir_remove_less_specific(uint32_t *allocated, int index, int fill,
                                           word length, dir_sub_table_t *table)
{
    uint32_t valueIndexRet = 0;
    uint32_t valueIndex = 0;

    for(; index < fill; index++)
    {
        if( !table->lengths[index] && table->entries[index])
        {
            dir_sub_table_t *next = (dir_sub_table_t*)table->entries[index];
            valueIndex = _dir_remove_less_specific(allocated, 0, 1 << next->width, length, next);
            if (valueIndex)
            {
                valueIndexRet = valueIndex;
            }

            if (!next->filledEntries)    //table can be collapsed.
            {
                _sub_table_free(allocated, next);
                table->entries[index] = 0;
                table->lengths[index] = 0;
                table->filledEntries--;

            }
        }
        else if(length == (word)table->lengths[index])
        {
            if (table->entries[index])
            {
                table->filledEntries--;
                valueIndexRet = table->entries[index];
            }
            table->entries[index] = 0;
            table->lengths[index] = 0;
        }
    }

    return valueIndexRet;
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
    {
        uint32_t local_index, i;
        /* need to handle bits usage across multiple 32bit vals within IPv6. */
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
        local_index = ip->adr[i] << (ip->bits %32);
        index = local_index >> (sizeof(local_index)*8 - sub_table->width);
    }

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

            if (!next_sub)
            {
                sub_table->filledEntries++;
            }

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
        ip->bits += sub_table->width;
        return (_dir_sub_insert(ip, length,
                        cur_len - sub_table->width, ptr, current_depth+1,
                        behavior, next_sub, root_table));
    }

    return RT_SUCCESS;
}

/* Insert entry into DIR-n-m tables
 * @param ip        IP address structure
 * @param len       Number of bits of the IP used for lookup
 * @param ptr       Information to be associated with this IP range
 * @param master_table    The table that describes all, returned by dir_new */
int sfrt_dir_insert(uint32_t* adr, int numAdrDwords, int len, word data_index,
                    int behavior, void *table)
{
    dir_table_t *root = (dir_table_t*)table;
    uint32_t h_adr[4];
    IPLOOKUP iplu;
    iplu.adr = h_adr;
    iplu.bits = 0;

    /* Validate arguments */
    if(!root || !root->sub_table)
    {
        return DIR_INSERT_FAILURE;
    }

    h_adr[0] = ntohl(adr[0]);
    if (len > 96)
    {
        h_adr[1] = ntohl(adr[1]);
        h_adr[2] = ntohl(adr[2]);
        h_adr[3] = ntohl(adr[3]);
    }
    else if (len > 64)
    {
        h_adr[1] = ntohl(adr[1]);
        h_adr[2] = ntohl(adr[2]);
    }
    else if (len > 32)
    {
        h_adr[1] = ntohl(adr[1]);
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
    {
        uint32_t local_index, i;
        /* need to handle bits usage across multiple 32bit vals within IPv6. */
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
        local_index = ip->adr[i] << (ip->bits %32);
        index = local_index >> (sizeof(local_index)*8 - table->width);
    }

    if( !table->entries[index] || table->lengths[index] )
    {
        tuple_t ret;
        ret.index = table->entries[index];
        ret.length = (word)table->lengths[index];

        return ret;
    }

    ip->bits += table->width;
    return _dir_sub_lookup( ip, (dir_sub_table_t *)table->entries[index]);
}

/* Lookup information associated with the value "ip" */
tuple_t sfrt_dir_lookup(uint32_t* adr, int numAdrDwords, void *tbl)
{
    dir_table_t *root = (dir_table_t*)tbl;
    uint32_t h_adr[4];
    int i;
    IPLOOKUP iplu;
    iplu.adr = h_adr;
    iplu.bits = 0;

    if(!root || !root->sub_table)
    {
        tuple_t ret = { 0, 0 };

        return ret;
    }

    for (i = 0; i < numAdrDwords; i++)
    {
        h_adr[i] = ntohl(adr[i]);
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

static void _sub_table_print(dir_sub_table_t *sub, uint32_t level, dir_table_t *table) {
    int index;

    char label[100];

    memset(label, ' ', sizeof(label));
    label[level*5] = '\0';

    printf("%sCurrent Nodes: %d, Filled Entries: %d, table Width: %d\n", label, sub->cur_num, sub->filledEntries, sub->width);
    for(index=0; index < sub->num_entries; index++)
    {
        if (sub->lengths[index] || sub->entries[index])
            printf("%sIndex: %d, Length: %d, dataIndex: %d\n", label, index, sub->lengths[index],
                    (uint32_t)sub->entries[index]);

        if( !sub->lengths[index] && sub->entries[index] ) {
            _sub_table_print((dir_sub_table_t*) sub->entries[index], level+1, table);
        }
    }
}

/* Print a table.
 * Prints a table and its subtable. This is used for debugging purpose only.
 * @param table The table that describes all, returned by dir_new
 */
void sfrt_dir_print(void *tbl) {
    dir_table_t *table = (dir_table_t*)tbl;

    if(!table) {
        return;
    }

    printf ("Nodes in use: %d\n", table->cur_num);
    if(table->sub_table) {
        _sub_table_print(table->sub_table, 1, table);
    }
}

/* Sub table removal
 * Recursive function to drill down to subnet table and remove entries.
 * @param ip        IP address structure
 * @param length    Number of bits of the IP used to specify this CIDR
 * @param cur_len   Number of bits of the IP left at this depth
 * @param current_depth Number of levels down from root_table.
 * @param behavior  RT_FAVOR_SPECIFIC or RT_FAVOR_TIME
 * @param root_table  The table that describes all, returned by dir_new
 * @returns index of entry removed. Returns 0, which is a valid index, as failure code.
 * Calling function should treat 0 index as failure case.*/

static int _dir_sub_remove(IPLOOKUP *ip, int length, int cur_len,
                           int current_depth, int behavior,
                           dir_sub_table_t *sub_table, dir_table_t *root_table)
{

    word index;
    uint32_t fill;
    uint32_t valueIndex = 0;

    {
        uint32_t local_index, i;
        /* need to handle bits usage across multiple 32bit vals within IPv6. */
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
        local_index = ip->adr[i] << (ip->bits %32);
        index = local_index >> (sizeof(local_index)*8 - sub_table->width);
    }

    /* Check if this is the last table to traverse to */
    if(sub_table->width >= cur_len)
    {

        /* Calculate how many entries need to be removed (filled with 0)
         * in this table. If the table is 24 bits wide, and the entry
         * is 20 bytes long, 2^4 entries need to be filled. */
        fill = 1 << (sub_table->width - cur_len);

        index = (index >> (sub_table->width - cur_len)) <<
            (sub_table->width - cur_len);

        fill += index;

        /* Remove and overwrite without consedering CIDR specificity*/
        if(behavior == RT_FAVOR_TIME)
        {
            valueIndex = _dir_remove_all(&root_table->allocated, index, fill, length, sub_table);
        }
        /* Remove and overwrite only less specific CIDR */
        else
        {
            valueIndex = _dir_remove_less_specific(&root_table->allocated, index, fill, length, sub_table);
        }
    }
    else
    {
        /* traverse to a next sub-table down*/

        dir_sub_table_t *next_sub = (dir_sub_table_t *)sub_table->entries[index];

        /*subtable was never added. */
        if(!next_sub || sub_table->lengths[index])
        {
            return 0;
        }
        /* Recurse to next level.  Rightshift off appropriate number of
         * bits and update the length accordingly. */
        ip->bits += sub_table->width;
        valueIndex = _dir_sub_remove(ip, length,
                        cur_len - sub_table->width, current_depth+1,
                        behavior, next_sub, root_table);
        if (!next_sub->filledEntries)
        {
            _sub_table_free(&root_table->allocated, next_sub);
            sub_table->entries[index] = 0;
            sub_table->lengths[index] = 0;
            sub_table->filledEntries--;
            root_table->cur_num--;
        }
    }

    return valueIndex;
}

/* Remove entry into DIR-n-m tables
 * @param ip    IP address structure
 * @param len   Number of bits of the IP used for lookup
 * @param behavior  RT_FAVOR_SPECIFIC or RT_FAVOR_TIME
 * @param table The table that describes all, returned by dir_new
 * @return index to data or 0 on failure. Calling function should check for 0 since
 * this is valid index for failed operation.
 */
word sfrt_dir_remove(uint32_t* adr, int numAdrDwords, int len, int behavior, void *table)
{
    dir_table_t *root = (dir_table_t*)table;
    uint32_t h_adr[4];
    IPLOOKUP iplu;
    iplu.adr = h_adr;
    iplu.bits = 0;

    /* Validate arguments */
    if(!root || !root->sub_table)
    {
        return 0;
    }

    h_adr[0] = ntohl(adr[0]);
    if (len > 96)
    {
        h_adr[1] = ntohl(adr[1]);
        h_adr[2] = ntohl(adr[2]);
        h_adr[3] = ntohl(adr[3]);
    }
	else if (len > 64)
    {
        h_adr[1] = ntohl(adr[1]);
        h_adr[2] = ntohl(adr[2]);
    }
	else if (len > 32)
    {
        h_adr[1] = ntohl(adr[1]);
    }

    /* Find the sub table in which to remove */
    return _dir_sub_remove(&iplu, len, len, 0, behavior, root->sub_table, root);
}

