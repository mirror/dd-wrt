/* $Id$ */
/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#ifndef _SF_SDLIST
#define _SF_SDLIST

/* based off Linked List structure p. 57  _Mastering algorithms in C_
 *
 * Differs from sf_list by using static listitem blocks.
 *
 * Use mempool as the interface to this code instead of trying to use it directly
 * 
 */

typedef struct _SDListItem {
    void *data;
    struct _SDListItem *next;
    struct _SDListItem *prev;
} SDListItem;


typedef struct sfSDList {
    int size;
    SDListItem *head;
    SDListItem *tail;
    void (*destroy)(void *data); /* delete function called for each
                                    member of the linked list */
} sfSDList;


/* initialize a DList */
int sf_sdlist_init(sfSDList *list, void (*destroy)(void *data));

/* delete an DList */
int sf_sdlist_delete(sfSDList *list);

/* insert item, putting data in container */
int sf_sdlist_insert_next(sfSDList *list, SDListItem *item, void *data,
                          SDListItem *container);

/* remove the item after the item */
int sf_sdlist_remove_next(sfSDList *list, SDListItem *item);

/* remove this item from the list */
int sf_sdlist_remove(sfSDList *list, SDListItem *item);

/* append at the end of the list */
int sf_sdlist_append(sfSDList *list, void *data, SDListItem *container);

void print_sdlist(sfSDList *list);

#endif /* _SF_DLIST */
