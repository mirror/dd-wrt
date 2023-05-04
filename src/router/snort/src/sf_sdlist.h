/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
**
** This is hi
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


#ifndef _SF_SDLIST
#define _SF_SDLIST

#include "sf_sdlist_types.h"

/* based off Linked List structure p. 57  _Mastering algorithms in C_
 *
 * Differs from sf_list by using static listitem blocks.
 *
 * Use mempool as the interface to this code instead of trying to use it directly
 *
 */

/* initialize a DList */
int sf_sdlist_init(sfSDList *list, void (*destroy)(void *data));

/* delete an DList */
int sf_sdlist_delete(sfSDList *list);

/* insert item, putting data in container */
int sf_sdlist_insert_next(sfSDList *list, SDListItem *item, void *data, SDListItem *container);

/* remove the item after the item */
int sf_sdlist_remove_next(sfSDList *list, SDListItem *item);

/* remove this item from the list */
int sf_sdlist_remove(sfSDList *list, SDListItem *item);

/* append at the end of the list */
int sf_sdlist_append(sfSDList *list, void *data, SDListItem *container);

void print_sdlist(sfSDList *list);

// list functions that handle memory allocation for inserted list items
int sf_sdlist_ins_next(sfSDList *list, SDListItem *item, const void *data);
int sf_sdlist_ins_prev(sfSDList *list, SDListItem *item, const void *data);
int sf_sdlist_rem_item(sfSDList *list, SDListItem *item, void **data);
int sf_sdlist_purge(sfSDList *list);


// macro implementation of simple doubly linked list operations
#define sf_sdlist_size(list) ((list)->size)
#define sf_sdlist_head(list) ((list)->head)
#define sf_sdlist_tail(list) ((list)->tail)
#define sf_sdlist_is_head(item) ((item)->prev == NULL ? 1 : 0)
#define sf_sdlist_is_tail(item) ((item)->next == NULL ? 1 : 0)
#define sf_sdlist_data(item) ((item)->data)
#define sf_sdlist_next(item) ((item)->next)
#define sf_sdlist_prev(item) ((item)->prev)

#endif /* _SF_DLIST */
