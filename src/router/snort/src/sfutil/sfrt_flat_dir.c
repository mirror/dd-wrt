/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2011-2013 Sourcefire, Inc.
 **
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 ** 9/7/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h> /* For variadic */
#include <stdio.h>
#include <string.h> /* For memset   */
#include "sf_types.h"
#include "sfrt_flat.h"
#include "sfrt_flat_dir.h"

#if SIZEOF_UNSIGNED_LONG_INT == 8
#define ARCH_WIDTH 64
#else
#define ARCH_WIDTH 32
#endif

typedef struct {
    uint32_t* adr;
    int bits;
} IPLOOKUP;

/* Create new "sub" table of 2^width entries */
static TABLE_PTR _sub_table_flat_new(dir_table_flat_t *root, uint32_t dimension,
        uint32_t prefill, uint32_t bit_length)
{

    int width = root->dimensions[dimension];
    int len = 1 << width;
    int index;
    dir_sub_table_flat_t *sub;
    TABLE_PTR sub_ptr;
    uint8_t *base;
    Entry_Value *entries_value;
    Entry_Len *entries_length;

    /* Check if creating this node will exceed the memory cap.
     * The symbols in the conditional (other than cap), come from the
     * allocs below. */
    if( root->mem_cap < ( root->allocated +
            sizeof(dir_sub_table_flat_t) +
            sizeof(Entry_Value) * len + sizeof(Entry_Len) * len) ||
            bit_length > 128)
    {
        return 0;
    }

    /* Set up the initial prefilled "sub table" */
    sub_ptr = segment_malloc(sizeof(dir_sub_table_flat_t));

    if(!sub_ptr)
    {
        return 0;
    }

    base = (uint8_t *)segment_basePtr();
    sub = (dir_sub_table_flat_t *)(&base[sub_ptr]);

    /* This keeps the width readily available rather than recalculating it
     * from the number of entries during an insert or lookup */
    sub->width = width;

    /* need 2^sub->width entries */
    /* A "length" needs to be stored with each entry above.  The length refers
     * to how specific the insertion that set the entry was.  It is necessary
     * so that the entry is not overwritten by less general routing
     * information if "RT_FAVOR_SPECIFIC" insertions are being performed. */

    sub->entries_value = segment_malloc(sizeof(MEM_OFFSET) * len);

    if(!sub->entries_value)
    {
        segment_free(sub_ptr);
        return 0;
    }

    sub->entries_length = segment_malloc(sizeof(Entry_Len) * len);
    if(!sub->entries_length)
    {
        segment_free(sub_ptr);
        return 0;
    }

    entries_value = (Entry_Value *)(&base[sub->entries_value]);
    entries_length = (Entry_Len *)(&base[sub->entries_length]);
    /* Can't use memset here since prefill is multibyte */
    for(index = 0; index < len; index++)
    {
        entries_value[index] = prefill;
        entries_length[index] = (uint8_t)bit_length;
    }

    root->allocated += sizeof(dir_sub_table_flat_t) + sizeof(Entry_Value) * len
        + sizeof(Entry_Len) * len;

    root->cur_num++;

    return sub_ptr;
}

/* Create new dir-n-m root table with 'count' depth */
TABLE_PTR sfrt_dir_flat_new(uint32_t mem_cap, int count,...)
{
    va_list ap;
    uint32_t val;
    int index;
    TABLE_PTR table_ptr;
    dir_table_flat_t* table;
    uint8_t *base;

    table_ptr = segment_malloc(sizeof(dir_table_flat_t));

    if(!table_ptr)
    {
        return 0;
    }

    base = (uint8_t *)segment_basePtr();
    table = (dir_table_flat_t *)(&base[table_ptr]);

    table->allocated = 0;

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

    table->sub_table = _sub_table_flat_new(table, 0, 0, 0);

    if(!table->sub_table)
    {
        segment_free(table_ptr);
        return 0;
    }

    table->allocated += sizeof(dir_table_flat_t) + sizeof(int)*count;

    return table_ptr;
}

/* Traverse "sub" tables, freeing each */
static void _sub_table_flat_free(uint32_t *allocated, SUB_TABLE_PTR sub_ptr)
{
    int index;
    dir_sub_table_flat_t *sub;
    uint8_t *base;
    int len;
    base = (uint8_t *)segment_basePtr();
    sub = (dir_sub_table_flat_t *)(&base[sub_ptr]);
    len = 1 << sub->width;

    for(index=0; index < len; index++)
    {
        /* The following condition will only be true if
         * this entry is a pointer  */
        Entry_Value *entries_value = (Entry_Value *)(&base[sub->entries_value]);
        Entry_Len *entries_length = (Entry_Len *)(&base[sub->entries_length]);

        if( !entries_value[index] && entries_length[index] )
        {
            _sub_table_flat_free( allocated, entries_value[index] );
        }
    }

    if(sub->entries_value)
    {
        /* This probably does not need to be checked
         * since if it was not allocated, we would have errored out
         * in _sub_table_flat_new */
        segment_free(sub->entries_value);

        *allocated -= sizeof(Entry_Value) * len;
    }

    if(sub->entries_length)
    {
        /* This probably does not need to be checked
         * since if it was not allocated, we would have errored out
         * in _sub_table_flat_new */
        segment_free(sub->entries_length);

        *allocated -= sizeof(Entry_Len) * len;
    }

    segment_free(sub_ptr);

    *allocated -= sizeof(dir_sub_table_flat_t);
}

/* Free the DIR-n-m structure */
void sfrt_dir_flat_free(TABLE_PTR tbl_ptr)
{
    dir_table_flat_t *table;
    uint8_t *base;

    if(!tbl_ptr)
    {
        return;
    }

    base = (uint8_t *)segment_basePtr();
    table = (dir_table_flat_t *)(&base[tbl_ptr]);

    if(table->sub_table)
    {
        _sub_table_flat_free(&table->allocated, table->sub_table);
    }

    segment_free(tbl_ptr);
}

static inline void _dir_fill_all(uint32_t *allocated, uint32_t index, uint32_t fill,
        word length, uint32_t val, SUB_TABLE_PTR sub_ptr)
{
    dir_sub_table_flat_t *subtable;
    uint8_t *base;

    base = (uint8_t *)segment_basePtr();
    subtable = (dir_sub_table_flat_t *)(&base[sub_ptr]);

    /* Fill entries */
    for(; index < fill; index++)
    {
        /* Before overwriting this entry, verify there's not an existing
         * pointer ... otherwise free it to avoid a huge memory leak. */
        Entry_Value *entries_value = (Entry_Value *)(&base[subtable->entries_value]);
        Entry_Len *entries_length = (Entry_Len *)(&base[subtable->entries_length]);

        if( entries_value[index] && !entries_length[index] )
        {
            _sub_table_flat_free(allocated, entries_value[index]);
        }

        entries_value[index] = val;
        entries_length[index] = (uint8_t)length;
    }
}

static inline void _dir_fill_less_specific(int index, int fill,
        word length, uint32_t val, SUB_TABLE_PTR sub_ptr)
{

    dir_sub_table_flat_t *subtable;
    uint8_t *base;

    base = (uint8_t *)segment_basePtr();
    subtable = (dir_sub_table_flat_t *)(&base[sub_ptr]);

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
        Entry_Value *entries_value = (Entry_Value *)(&base[subtable->entries_value]);
        Entry_Len *entries_length = (Entry_Len *)(&base[subtable->entries_length]);
        if( entries_value[index] && !entries_length[index] )
        {
            dir_sub_table_flat_t *next = (dir_sub_table_flat_t*)(&base[entries_value[index]]);
            _dir_fill_less_specific(0, 1 << next->width, length, val, entries_value[index]);
        }
        else if(length >= (unsigned)entries_length[index])
        {
            entries_value[index] = val;
            entries_length[index] = (char)length;
        }
    }
}

static inline int64_t _dir_update_info(int index, int fill,
        word length, uint32_t val, SUB_TABLE_PTR sub_ptr, updateEntryInfoFunc updateEntry, INFO *data)
{

    dir_sub_table_flat_t *subtable;
    uint8_t *base;
    int64_t bytesAllocatedTotal = 0;

    base = (uint8_t *)segment_basePtr();
    subtable = (dir_sub_table_flat_t *)(&base[sub_ptr]);

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
        Entry_Value *entries_value = (Entry_Value *)(&base[subtable->entries_value]);
        Entry_Len *entries_length = (Entry_Len *)(&base[subtable->entries_length]);
        if( entries_value[index] && !entries_length[index] )
        {
            int64_t bytesAllocated;
            dir_sub_table_flat_t *next = (dir_sub_table_flat_t*)(&base[entries_value[index]]);
            bytesAllocated = _dir_update_info(0, 1 << next->width, length, val,
                    entries_value[index], updateEntry, data);
            if (bytesAllocated < 0)
                return bytesAllocated;
            else
                bytesAllocatedTotal += bytesAllocated;
        }
        else if(length > (unsigned)entries_length[index])
        {
            if(entries_value[index]) 
            {
               int64_t bytesAllocated;
               bytesAllocated = updateEntry(&data[entries_value[index]], data[val],
                       SAVE_TO_NEW, base);
               if (bytesAllocated < 0)
                   return bytesAllocated;
               else
                   bytesAllocatedTotal += bytesAllocated;
           }

           entries_value[index] = val;
           entries_length[index] = (uint8_t)length;
        }
        else if(entries_value[index])
        {
            int64_t bytesAllocated;
            bytesAllocated = updateEntry(&data[entries_value[index]], data[val],
                    SAVE_TO_CURRENT,  base);
            if (bytesAllocated < 0)
                return bytesAllocated;
            else
                bytesAllocatedTotal += bytesAllocated;
        }
    }

    return bytesAllocatedTotal;
}
/* Sub table insertion
 * This is called by dir_insert and recursively to find the the sub table
 * that should house the value "ptr"
 * @param ip        IP address structure
 * @param cur_len   Number of bits of the IP left at this depth
 * @param length    Number of bits of the IP used to specify this CIDR
 * @param ptr       Information to be associated with this IP range
 * @param master_table    The table that describes all, returned by dir_new */
static int _dir_sub_insert(IPLOOKUP *ip, int length, int cur_len, INFO ptr,
        int current_depth, int behavior,
        SUB_TABLE_PTR sub_ptr, dir_table_flat_t *root_table,updateEntryInfoFunc updateEntry, INFO *data)
{

    word index;
    uint32_t fill;
    uint8_t *base = (uint8_t *)segment_basePtr();
    dir_sub_table_flat_t *sub_table = (dir_sub_table_flat_t *)(&base[sub_ptr]);

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
                    (word)ptr, sub_ptr);
        }
        /* Fill over less specific CIDR */
        else if (behavior == RT_FAVOR_SPECIFIC)
        {
            _dir_fill_less_specific(index, fill, length, (word)ptr, sub_ptr);
        }
        else if (behavior == RT_FAVOR_ALL)
        {
            int64_t bytesAllocated;

            bytesAllocated = _dir_update_info(index, fill, length, (word)ptr,
                    sub_ptr, updateEntry, data);

            if (bytesAllocated < 0)
                return MEM_ALLOC_FAILURE;

            root_table->allocated += (uint32_t)bytesAllocated;

            if( root_table->mem_cap < root_table->allocated)
                return MEM_ALLOC_FAILURE;
        }
    }
    /* Need to traverse to a sub-table */
    else
    {
        Entry_Value *entries_value = (Entry_Value *)(&base[sub_table->entries_value]);
        Entry_Len *entries_length = (Entry_Len *)(&base[sub_table->entries_length]);

        /* Check if we need to alloc a new sub table.
         * If next_sub was 0/NULL, there's no entry at this index
         * If the length is non-zero, there is an entry */
        if(!entries_value[index] || entries_length[index])
        {
            if( root_table->dim_size <= current_depth )
            {
                return RT_INSERT_FAILURE;
            }

            entries_value[index] = 
                    (word) _sub_table_flat_new(root_table, current_depth+1,
                            (word) entries_value[index], entries_length[index]);

            entries_length[index] = 0;

            if(!entries_value[index])
            {
                return MEM_ALLOC_FAILURE;
            }
        }
        /* Recurse to next level.  Rightshift off appropriate number of
         * bits and update the length accordingly. */
        ip->bits += sub_table->width;
        return (_dir_sub_insert(ip, length,
                cur_len - sub_table->width, ptr, current_depth+1,
                behavior, entries_value[index], root_table, updateEntry, data));
    }

    return RT_SUCCESS;
}

/* Insert entry into DIR-n-m tables
 * @param ip        IP address structure
 * @param len       Number of bits of the IP used for lookup
 * @param ptr       Information to be associated with this IP range
 * @param master_table    The table that describes all, returned by dir_new */
int sfrt_dir_flat_insert(uint32_t* adr, int numAdrDwords, int len, word data_index,
        int behavior, TABLE_PTR table_ptr, updateEntryInfoFunc updateEntry, INFO *data)
{
    dir_table_flat_t *root;
    uint8_t *base;
    uint32_t h_adr[4];
    IPLOOKUP iplu;
    iplu.adr = h_adr;
    iplu.bits = 0;

    base = (uint8_t *)segment_basePtr();
    root = (dir_table_flat_t *)(&base[table_ptr]);
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
    return _dir_sub_insert(&iplu, len, len, data_index,
            0, behavior, root->sub_table, root, updateEntry, data);
}

/* Traverse sub tables looking for match */
/* Called by dir_lookup and recursively */
static tuple_flat_t _dir_sub_flat_lookup(IPLOOKUP *ip, TABLE_PTR table_ptr)
{
    word index;
    uint8_t *base = (uint8_t *)segment_basePtr();
    Entry_Value *entries_value;
    Entry_Len *entries_length;

    dir_sub_table_flat_t *table = (dir_sub_table_flat_t *)(&base[table_ptr]);

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

    entries_value = (Entry_Value *)(&base[table->entries_value]);
    entries_length = (Entry_Len *)(&base[table->entries_length]);

    if( !entries_value[index] || entries_length[index] )
    {
        tuple_flat_t ret;
        ret.index = entries_value[index];
        ret.length = (word)entries_length[index];
        return ret;
    }

    ip->bits += table->width;
    return _dir_sub_flat_lookup( ip, entries_value[index] );
}

/* Lookup information associated with the value "ip" */
tuple_flat_t sfrt_dir_flat_lookup(uint32_t* adr, int numAdrDwords, TABLE_PTR table_ptr)
{
    dir_table_flat_t *root;
    uint8_t *base = (uint8_t *)segment_basePtr();
    uint32_t h_adr[4];
    int i;
    IPLOOKUP iplu;
    iplu.adr = h_adr;
    iplu.bits = 0;

    if(!table_ptr )
    {
        tuple_flat_t ret = { 0, 0 };
        return ret;
    }

    root = (dir_table_flat_t *)(&base[table_ptr]);

    if(!root->sub_table)
    {
        tuple_flat_t ret = { 0, 0 };
        return ret;
    }

    for (i = 0; i < numAdrDwords; i++)
    {
        h_adr[i] = ntohl(adr[i]);
    }

    return _dir_sub_flat_lookup(&iplu, root->sub_table);
}


uint32_t sfrt_dir_flat_usage(TABLE_PTR table_ptr)
{
    dir_table_flat_t *table;
    uint8_t *base;
    if(!table_ptr)
    {
        return 0;
    }
    base = (uint8_t *)segment_basePtr();
    table = (dir_table_flat_t *)(&base[table_ptr]);
    return ((dir_table_flat_t*)(table))->allocated;
}

