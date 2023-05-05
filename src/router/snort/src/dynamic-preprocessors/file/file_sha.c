/* $Id */

/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2013-2013 Sourcefire, Inc.
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
 */

/*
 * Simple SHA hash table
 *
 * We use first two bytes of SHA as index
 * Author: Hui Cao
 */

#include "sf_types.h"
#include "file_sha.h"
#include "memory_stats.h"
#include "preprocids.h"
#include "sf_dynamic_preprocessor.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef uint16_t ShaKeyType;       /*16 bits as index, we will have 65536 rows*/

#define SHA_TABLE_ROWS  (1<<(sizeof(ShaKeyType)*8))

char conv_to_hex[UINT8_MAX + 1];

/* Create SHA table
 *
 * Args:
 *   int sha_bits: number of bytes in sha
 */
ShaHash *sha_table_new(char sha_bytes)
{
    ShaHash *table = _dpd.snortAlloc(1, sizeof(ShaHash), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION); 

    if (!table)
        return NULL;

    table->nrows = SHA_TABLE_ROWS;

    table->entries = _dpd.snortAlloc(table->nrows, sizeof(*(table->entries)), PP_FILE_INSPECT,
            PP_MEM_CATEGORY_SESSION);
 
    if (!table->entries)
    {
        _dpd.snortFree(table, sizeof(ShaHash), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
        return NULL;
    }

    table->sha_size = sha_bytes;
    table->count = 0;

    return table;
}

static int sha_cmp (void *sha1, void *sha2, char sha_bytes)
{
    return memcmp(sha1, sha2, sha_bytes);
}

static ShaHashNode *sha_table_node_by_sha(ShaHash *table, void *sha)
{
    int index;
    ShaHashNode *node;

    index = *(ShaKeyType *)sha;

    if (!table || !table->entries)
        return NULL;

    node = (ShaHashNode *)table->entries[index];

    while(node && sha_cmp(node->sha, sha, table->sha_size))
    {
        node = node->next;
    }

    if (node)
    {
        return node;
    }
    else
    {
        return NULL;
    }
}

/*
 *  Find a Node based on the key, return users data.
 */
void * sha_table_find(ShaHash *table, void *sha)
{

    ShaHashNode *node;

    node = sha_table_node_by_sha(table, sha);

    if (node)
    {
        return node->data;
    }
    else
    {
        return NULL;
    }
}

/*
 *  Add a key + data pair
 *  ---------------------
 *
 *  key + data should both be non-zero, although data can be zero
 *
 *  t    - hash table
 *  sha  - sha value
 *  data - users data pointer
 *
 *  returns  SHAHASH_NOMEM: alloc error
 *           SHAHASH_INTABLE : key already in table
 *           SHAHASH_OK: added a node for this key + data pair
 */
int sha_table_add( ShaHash *table, void *sha, void *data )
{

    ShaHashNode *oldNode;
    ShaHashNode *newNode;
    int index;

    /*check whether existing*/
    oldNode = sha_table_node_by_sha(table, sha);

    if (oldNode)
        return SHAHASH_INTABLE;

    /* Add to the head of entry*/
    index = *(ShaKeyType *)sha;
    oldNode = (ShaHashNode *)table->entries[index];

    newNode = _dpd.snortAlloc(1, sizeof (*newNode), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
    if (!newNode)
        return SHAHASH_NOMEM;

    newNode->next = oldNode;
    newNode->sha = sha;
    newNode->data = data;

    table->entries[index] = newNode;

    table->count++;

    return SHAHASH_OK;
}

/*
 *  Delete the hash Table
 *
 *  free key's, free node's, and free the users data, if they
 *  supply a free function
 */
void sha_table_delete( ShaHash *table )
{
    int i;

    if (!table || !table->entries)
        return;

    for (i = 0; i < table->nrows; i++)
    {
        ShaHashNode *node;
        ShaHashNode *old;
        node = (ShaHashNode *)table->entries[i];
        while(node)
        {
            old = node;
            node = node->next;
            _dpd.snortFree(old->sha, SHA256_HASH_SIZE, PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
            _dpd.snortFree(old, sizeof(ShaHashNode), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
        }
    }
    _dpd.snortFree(table->entries, sizeof(ShaHashNode) * table->count, PP_FILE_INSPECT, 
            PP_MEM_CATEGORY_SESSION);
    _dpd.snortFree(table, sizeof(ShaHash), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
}

static void init_char_to_hex_array(void)
{
    size_t i;
    char conv1[] = "0123456789ABCDEF";
    char conv2[] = "abcdef";

    /*initializate to unset*/
    for (i = 0; i < sizeof (conv_to_hex); i++)
    {
        conv_to_hex[i] = -1;
    }
    /* number and upper case letter*/
    for (i = 0; i < sizeof (conv1); i++)
    {
        conv_to_hex[(int)conv1[i]] = i;
    }

    /* lower case number */
    for (i = 0; i < sizeof (conv2); i++)
    {
        conv_to_hex[(int)conv2[i]] = i + 10;
    }
}
/*
 * Convert a printable str to sha
 */
int str_to_sha (char *sha_str, char *sha256, int max_size)
{
    char *index = sha_str;
    char *end = sha_str + max_size;
    int i;
    char *dest = sha256;
    static bool initialized = false;

    if (!initialized)
    {
        init_char_to_hex_array();
        initialized = true;
    }

    /*trim spaces in the head of signature*/
    while ((index < end) && isspace((int)*index))
    {
        index++;
    }

    /* make sure length is correct */
    if (index + SHA256_HASH_STR_SIZE > end)
        return -1;

    /* convert it to hex */
    for (i = 0; i < SHA256_HASH_SIZE; i++)
    {
        char value;

        if (conv_to_hex[(int)*index] < 0)
            return -1;

        value = conv_to_hex[(int)*(index++)];

        if (conv_to_hex[(int)*index] < 0)
            return -1;

        *(dest++) = conv_to_hex[(int)*(index++)] + (value << 4);
    }

    /*Check end of string*/
    while ((index < end) && isspace((int)*index))
    {
        index++;
    }

    if (index != end)
        return -1;

    return 0;
}

/*
 * Create file name based on SHA
 */

int sha_to_str (char *sha256, char *sha_str, int max_size)
{
    char conv[] = "0123456789ABCDEF";
    const char *index;
    const char *end;
    char *ridx;
    int len;

    if (max_size < 1)
        return 0;

    index = sha256;

    if (max_size < SHA256_HASH_SIZE)
        len =  max_size - 1;
    else
        len =  SHA256_HASH_SIZE;

    ridx = sha_str;
    end = index + len;

    while(index < end)
    {
        *ridx++ = conv[((*index & 0xFF)>>4)];
        *ridx++ = conv[((*index & 0xFF)&0x0F)];
        index++;
    }

    *ridx = '\0';
    return len;
}
