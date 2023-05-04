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
 * Author: Hui Cao
 */

#ifndef SPP_FILE_SHA_H
#define SPP_FILE_SHA_H

#define SHA256_HASH_STR_SIZE      64
#define SHA256_HASH_SIZE          32

/*
*   ERROR DEFINES
*/
#define SHAHASH_NOMEM    -2
#define SHAHASH_ERR      -1
#define SHAHASH_OK        0
#define SHAHASH_INTABLE   1

/*
 *   HASH NODE
 */
typedef struct _ShaHashNode
{
  struct _ShaHashNode *next;

  void *sha;   /* Pointer to the sha */
  void *data;  /* Pointer to the users data, this is never copied! */

} ShaHashNode;

typedef struct _ShaHash
{
    ShaHashNode **entries;  /* array of node ptr's */
    int           nrows;    /* # rows int the hash table  */
    unsigned      count;    /* total # nodes in table */
    char          sha_size;  /* bytes in sha_size */
} ShaHash;


/* Create SHA table
 *
 * Args:
 *   char sha_bytes: number of bytes in sha
 *
 */
ShaHash *sha_table_new( char sha_bytes);

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
*
*/
int sha_table_add( ShaHash *t, void *sha, void *data );

/*
 *  Find a Node based on the sha, return users data.
 */
void * sha_table_find( ShaHash *t, void *sha);

/*
 *  Delete the hash Table
 *
 *  free key's, free node's, and free the users data, if they
 *  supply a free function
 */
void sha_table_delete( ShaHash *h );

/*
 * Convert a printable str to sha
 *
 * Args:
 *   char *sha256: point to sha to be saved
 *   char *sha_str: point to the string that will be parsed
 *   int max_size: maximum number of bytes to be read from sha_str
 */
int str_to_sha (char *sha_str, char *sha256, int max_size);

/*
 * Convert a SHA256 to a printable str
 *
 * Args:
 *   char *sha256: point to sha in binary
 *   char *sha_str: point to the string that will be saved
 *   int max_size: maximum number of bytes to be saved to sha_str
 */
int sha_to_str (char *sha256, char *sha_str, int max_size);

#endif /* SPP_FILE_SHA_H */
