/*
 ** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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
 **
 ** This is based on the original sfrt.h, but using the flat segment memory.
 ** When allocating memory, it uses memory in the segment, and returns
 ** the offset.
 ** When accessing memory, it must use the base address and offset to
 ** correctly refer to it.
 */

#ifndef _SFRT_FLAT_H_
#define _SFRT_FLAT_H_

#include <stdlib.h>
#include <sys/types.h>
#include "snort_debug.h"
#include "ipv6_port.h"
#include "segment_mem.h"

typedef MEM_OFFSET INFO; /* To be replaced with a pointer to a policy */
typedef MEM_OFFSET FLAT_INDEX;
typedef MEM_OFFSET TABLE_PTR;

typedef enum
{
   SAVE_TO_NEW,
   SAVE_TO_CURRENT
}SaveDest;

typedef int64_t (*updateEntryInfoFunc)(INFO *entryInfo, INFO newInfo,
        SaveDest saveDest, uint8_t *base);
typedef struct {
    FLAT_INDEX index;
    int length;
} tuple_flat_t;

#include "sfrt_flat_dir.h"



/*******************************************************************/
/* Master table struct.  Abstracts DIR and LC-trie methods         */
typedef struct {
    uint32_t num_ent; /* Number of entries in the policy table */
    uint32_t max_size; /* Max size of policies array */
    char ip_type; /* Only IPs of this family will be used */
    char table_flat_type;
    char mem_type;
    uint32_t allocated;
    INFO data; /* data table. Each IP points to an entry here */
    TABLE_PTR rt; /* Actual "routing" table */
    TABLE_PTR rt6; /* Actual "routing" table */
    TABLE_PTR list_info; /* List file information table (entry information)*/

} table_flat_t;
/*******************************************************************/

/* Abstracted routing table API */
table_flat_t * sfrt_flat_new(char table_flat_type, char ip_type,
        long data_size, uint32_t mem_cap);
void sfrt_flat_free(TABLE_PTR table);
GENERIC sfrt_flat_lookup(void *adr, table_flat_t *table);
int sfrt_flat_insert(void *adr, unsigned char len, INFO ptr, int behavior,
        table_flat_t *table, updateEntryInfoFunc updateEntry);
uint32_t sfrt_flat_usage(table_flat_t *table);
uint32_t sfrt_flat_num_entries(table_flat_t *table);

/* Perform a lookup on value contained in "ip"
 * For performance reason, we use this simplified version instead of sfrt_lookup
 * Note: this only applied to table setting: DIR_8x16 (DIR_16_8_4x2 for IPV4), DIR_8x4*/
static inline GENERIC sfrt_flat_dir8x_lookup(void *adr, table_flat_t* table) {
    dir_sub_table_flat_t *subtable;
    DIR_Entry *entry;
    uint8_t *base = (uint8_t *) table;
    int i;
    sfip_t *ip;
    dir_table_flat_t *rt = NULL;
    int index;
    INFO *data = (INFO *) (&base[table->data]);

    ip = adr;
    if (ip->family == AF_INET)
    {
        rt = (dir_table_flat_t *)(&base[table->rt]);
        subtable = (dir_sub_table_flat_t *)(&base[rt->sub_table]);
        /* 16 bits*/
        index = ntohs(ip->ip16[0]);
        entry = (DIR_Entry *)(&base[subtable->entries]);
        if( !entry[index].value || entry[index].length)
        {
            if (data[entry[index].value])
                return (GENERIC) &base[data[entry[index].value]];
            else
                return NULL;
        }
        subtable = (dir_sub_table_flat_t *)(&base[entry[index].value]);

        /* 8 bits*/
        index = ip->ip8[2];
        entry = (DIR_Entry *)(&base[subtable->entries]);
        if( !entry[index].value || entry[index].length)
        {
            if (data[entry[index].value])
                return (GENERIC) &base[data[entry[index].value]];
            else
                return NULL;
        }
        subtable = (dir_sub_table_flat_t *)(&base[entry[index].value]);

        /* 4 bits */
        index = ip->ip8[3] >> 4;
        entry = (DIR_Entry *)(&base[subtable->entries]);
        if( !entry[index].value || entry[index].length)
        {
            if (data[entry[index].value])
                return (GENERIC) &base[data[entry[index].value]];
            else
                return NULL;
        }
        subtable = (dir_sub_table_flat_t *)(&base[entry[index].value]);

        /* 4 bits */
        index = ip->ip8[3] & 0xF;
        entry = (DIR_Entry *)(&base[subtable->entries]);
        if( !entry[index].value || entry[index].length)
        {
            if (data[entry[index].value])
                return (GENERIC) &base[data[entry[index].value]];
            else
                return NULL;
        }
        subtable = (dir_sub_table_flat_t *)(&base[entry[index].value]);

    }
    else if (ip->family == AF_INET6)
    {

        rt = (dir_table_flat_t *)(&base[table->rt6]);
        subtable = (dir_sub_table_flat_t *)(&base[rt->sub_table]);
        for (i = 0; i < 16; i++)
        {
            index = ip->ip8[i];
            entry = (DIR_Entry *)(&base[subtable->entries]);
            if( !entry[index].value || entry[index].length)
            {
                if (data[entry[index].value])
                    return (GENERIC) &base[data[entry[index].value]];
                else
                    return NULL;
            }
            subtable = (dir_sub_table_flat_t *)(&base[entry[index].value]);
        }
    }
return NULL;

}
#endif

