/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_sdlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function: int sf_sdlist_init(sfSDlist *list, void (*destroy)(void *data))
 *
 * Purpose: initialize an dlist
 * Args: list - pointer to a dlist structure
 *       destroy - free function ( use NULL for none )
 * Returns:
 *     1 on failure , 0 on success
 */

int sf_sdlist_init(sfSDList *list, void (*destroy)(void *data))
{
    list->destroy = destroy;
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;

    return 0;
}


/* Function: int sf_sdlist_delete(sfSDList *list)
 *
 * Purpose: delete every item of a list
 * Args: list -> pointer to a dlist structure
 *
 * Returns: 1 on failure , 0 on success
 */
int sf_sdlist_delete(sfSDList *list)
{
    while(list->head != NULL)
    {
        sf_sdlist_remove_next(list, NULL);
    }

    return 0;
}

/*
 * Function: int sf_sdlist_insert_next(sfSDList *list, SDListItem *item,
 *                                    void *data, SDListItem *container)
 *
 * Purpose: insert data in container in the list after the item
 * Args: list - dlist structure
 *       item - current position in list structure
 *       data - current data to insert
 *       container - place to put the data
 *
 * Returns: 0 on sucess,  1 on failure
 */
int sf_sdlist_insert_next(sfSDList *list, SDListItem *item, void *data,
                          SDListItem *container)
{
    SDListItem *new = container;

    if(new == NULL) return -1;

    new->data = data;

    if(item == NULL)
    {
        /* We are inserting at the head of the list HEAD */

        if(list->size == 0)
        {
            list->tail = new;
        }

        new->next = list->head;
        list->head = new;
    }
    else
    {
        if(item->next == NULL)
        {
            /* TAIL */
            list->tail = new;
        }

        new->next = item->next;
        item->next = new;
    }

    new->prev = item;

    if(new->next != NULL)
    {
        new->next->prev = new;
    }

    list->size++;
    return 0;
}

int sf_sdlist_append(sfSDList *list, void *data, SDListItem *container)
{
    return sf_sdlist_insert_next(list, list->tail, data, container);
}

int sf_sdlist_remove_next(sfSDList *list, SDListItem *item)
{
    SDListItem *li = NULL;
    void *data;

    if(list->size == 0)
    {
        return -1;
    }

    /* remove the head */
    if(item == NULL)
    {
        li = list->head;
        data = li->data;
        list->head = li->next;
    }
    else
    {
        if(item->next == NULL)
        {
            return -1;
        }

        li = item->next;
        data = li->data;
        item->next = li->next;
    }

    if(li->next != NULL)
    {
        li->next->prev = item;
    }

    if(list->destroy != NULL)
        list->destroy(data);

    list->size--;

    if(list->size == 0) {
        list->tail = NULL;
    }

    return 0;
}


/*
 * Function: int sf_sdlist_remove(sfSDList *list, SDListItem *item)
 *
 * Purpose: remove the item pointed to by item
 * Args: list - list pointer
 *       item - item to unlink from the list
 *
 * Returns: 0 on success , 1 on exception
 *
 */
int sf_sdlist_remove(sfSDList *list, SDListItem *item)
{
    SDListItem *next_item;
    SDListItem *prev_item;

    if(item == NULL)
    {
        return -1;
    }

    next_item = item->next;
    prev_item = item->prev;

    if(next_item != NULL)
    {
        next_item->prev = prev_item;
    } else {
        list->tail = prev_item;
    }

    if(prev_item != NULL)
    {
        prev_item->next = next_item;
    } else {
        /* HEAD */
        list->head = next_item;
    }


    if(list->destroy != NULL)
        list->destroy(item->data);


    list->size--;

    if(list->size == 0)
    {
        list->head = NULL;
        list->tail = NULL;
    }

    return 0;
}

void print_sdlist(sfSDList *a)
{
    SDListItem *li;
    printf("***");
    printf(" size: %d\n", a->size);
    for(li = a->head; li != NULL; li = li->next) {
        printf(" `- %p\n", (void*)li);
    }
}


int sf_sdlist_ins_next(sfSDList *list, SDListItem *item, const void *data)
{
    SDListItem  *new_item;

    // Do not allow a NULL item unless the list is empty.
    if (item == NULL && sf_sdlist_size(list) != 0)
         return -1;

    // Allocate storage for the item.
    if ((new_item = ( SDListItem * ) malloc( sizeof(SDListItem) )) == NULL)
        return -1;

    // Insert the new item into the list.
    new_item->data = (void *)data;

    if (sf_sdlist_size(list) == 0)
    {
        // Handle insertion when the list is empty.
        list->head = new_item;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail = new_item;
    }
    else
    {
        // Handle insertion when the list is not empty.
        new_item->next = item->next;
        new_item->prev = item;

        if (item->next == NULL)
           list->tail = new_item;
        else
           item->next->prev = new_item;

        item->next = new_item;
    }

    // Adjust the size of the list to account for the inserted item.
    list->size++;

    return 0;
}

int sf_sdlist_ins_prev(sfSDList *list, SDListItem *item, const void *data)
{
    SDListItem          *new_item;

    // Do not allow a NULL item unless the list is empty.
    if (item == NULL && sf_sdlist_size(list) != 0)
        return -1;

    // Allocate storage to be managed by the abstract datatype.
    if ((new_item = (SDListItem *) malloc( sizeof(SDListItem) )) == NULL)
        return -1;

    // Insert the new item into the list.
    new_item->data = (void *)data;

    if (sf_sdlist_size(list) == 0)
    {
        // Handle insertion when the list is empty.
        list->head = new_item;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail = new_item;

    }
    else
    {
        // Handle insertion when the list is not empty.
        new_item->next = item;
        new_item->prev = item->prev;

        if (item->prev == NULL)
            list->head = new_item;
        else
            item->prev->next = new_item;

        item->prev = new_item;
    }

    // Adjust the size of the list to account for the new item.
    list->size++;

    return 0;
}

int sf_sdlist_rem_item(sfSDList *list, SDListItem *item, void **data)
{
    // Do not allow a NULL item or removal from an empty list.
    if (item == NULL || sf_sdlist_size(list) == 0)
        return -1;

    *data = item->data;

    if (item == list->head)
    {
        // Handle removal from the head of the list.
        list->head = item->next;

        if (list->head == NULL)
            list->tail = NULL;
        else
            item->next->prev = NULL;

    }
    else
    {
        // Handle removal from other than the head of the list.
        item->prev->next = item->next;

        if (item->next == NULL)
            list->tail = item->prev;
        else
            item->next->prev = item->prev;
    }

    // Free the storage allocated by the abstract datatype.
    free(item);

    // Adjust the size of the list to account for the removed item.
    list->size--;

    return 0;
}

/* Function: int sf_sdlist_purge(sfSDList *list)
 *
 * Purpose: remove every item of a list, free all
 * allocated container memory
 *
 * Args: list -> pointer to a dlist structure
 *
 * Returns: 1 on failure , 0 on success
 */
int sf_sdlist_purge(sfSDList *list)
{
    void *data;

    while(list->head != NULL)
    {
        sf_sdlist_rem_item(list, list->head, &data);
        if(list->destroy != NULL)
            list->destroy(data);
    }

    return 0;
}


#ifdef TEST_SDLIST
void bad(void *d) {
    free(d);
    return;
}

int main(void) {
    sfSDList a;

    SDListItem *li;
    SDListItem listpool[1000];

    sf_sdlist_init(&a, &bad);
    if(sf_sdlist_append(&a, (char *) SnortStrdup("hello"), &listpool[0]))
    {
        printf("error appending!\n");
    }

    sf_sdlist_append(&a, (char *)SnortStrdup("goodbye"), &listpool[1]);

    sf_sdlist_insert_next(&a, NULL, (char *)SnortStrdup("woo"), &listpool[2]);

    printf("list size %d\n", a.size);

    for(li = a.head; li != NULL; li = li->next)
    {
        printf("%s\n", (char *) li->data);
    }


    printf("*** removing ***\n");
    sf_sdlist_remove(&a, &listpool[1]);
    printf("list size %d\n", a.size);
    for(li = a.head; li != NULL; li = li->next)
    {
        printf("%s\n", (char *) li->data);
    }

    sf_sdlist_delete(&a);

    printf("list size %d\n", a.size);
    return 0;
}
#endif /* TEST_SDLIST */
