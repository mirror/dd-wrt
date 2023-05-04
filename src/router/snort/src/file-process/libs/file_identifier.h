/*
**
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2012-2013 Sourcefire, Inc.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
**  Author(s):  Hui Cao <hcao@sourcefire.com>
**
**  NOTES
**  5.25.2012 - Initial Source Code. Hcao
*/

#ifndef __FILE_IDENTIFIER_H__
#define __FILE_IDENTIFIER_H__
#include "file_lib.h"

#define MAX_BRANCH (UINT8_MAX + 1)   /* 256 identifiers pointers*/

typedef enum _IdNodeState
{
    ID_NODE_NEW,
    ID_NODE_USED,
    ID_NODE_SHARED
} IdNodeState;

typedef struct _IdentifierNode
{
    uint32_t type_id;           /* magic content to match*/
    IdNodeState state;
    uint32_t offset;            /* offset from file start */
    struct _IdentifierNode *next[MAX_BRANCH]; /* next levels*/

} IdentifierNode;

typedef struct _IdentifierNodeHead
{
    int offset;            /* offset from file start */
    IdentifierNode *start;  /* start node for the trie at this offset*/
    struct _IdentifierNodeHead *nextHead; /* pointer to next offset head*/

} IdentifierNodeHead;

/* Initialize file type id states
 *
 * Args: None
 * Return: None
 */
void file_identifers_init(void);

/* Insert one file rule to file identifiers trie, update file identifiers
 *
 * Args:
 *   RuleInfo *rule: file magic rule information
 *   void *conf: file configuration
 *
 * Return:
 *   None
 */
void file_identifers_update(RuleInfo *rule, void *conf);

/* Memory usage for all file magics
 *
 * Args: None
 * Return:
 *   uint32_t: memory usage in bytes
 */
uint32_t file_identifiers_usage(void);

/* Main process for file type identification.
 * This identification is stateful
 *
 * Args:
 *   uint8_t *buf: data buffer pointer
 *   int len: length of data buffer
 *   FileContext *context: context for saving state
 * Return:
 *   uint32_t: file type ID
 */
uint32_t file_identifiers_match(uint8_t *buf, int len, FileContext *cxt);

/* Free file identifiers stored in conf
 *
 * Args:
 *   void *conf: file configuration
 *
 * Return:
 *   None
 */
void file_identifiers_free(void* conf);

#ifdef DEBUG_MSGS
void file_identifiers_print(IdentifierNode*);
#endif

#endif

