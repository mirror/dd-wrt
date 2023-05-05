/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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
 ****************************************************************************
 * Provides list, queue and stack data structures and methods for use
 * with the preprocessor.
 *
 * 8/17/2008 - Initial implementation ... Todd Wease <twease@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "dce2_list.h"
#include "dce2_memory.h"
#include "dce2_debug.h"
#include "dce2_utils.h"
#include "sf_types.h"

/********************************************************************
 * Private function prototyes
 ********************************************************************/
static void DCE2_ListInsertTail(DCE2_List *, DCE2_ListNode *);
static void DCE2_ListInsertHead(DCE2_List *, DCE2_ListNode *);
static void DCE2_ListInsertBefore(DCE2_List *, DCE2_ListNode *, DCE2_ListNode *);

/********************************************************************
 * Function: DCE2_ListNew()
 *
 * Creates and returns a new list object.
 *
 * Arguments:
 *  DCE2_ListType
 *      The type of list this should be - sorted, splayed, etc.
 *  DCE2_ListKeyCompare
 *      The comparison function to call when comparing two keys
 *      for inserting, finding, etc.
 *  DCE2_ListDataFree
 *      An optional function to call to free data in the list.
 *      If NULL is passed in, the user will have to manually free
 *      the data.
 *  DCE2_ListKeyFree
 *      An optional function to call to free keys used in the list.
 *      If NULL is passed in, the user will have to manually free
 *      the keys.
 *  int
 *      Flags that affect processing of the list.
 *      See DCE2_ListFlags for possible combinations.
 *  DCE2_MemType
 *      The memory type that dynamically allocated data should be
 *      associated with.
 *
 * Returns:
 *  DCE2_List *
 *      Pointer to a valid list object.
 *      NULL if an error occurs.
 *
 ********************************************************************/
DCE2_List * DCE2_ListNew(DCE2_ListType type, DCE2_ListKeyCompare kc,
                         DCE2_ListDataFree df, DCE2_ListKeyFree kf,
                         int flags, DCE2_MemType mtype)
{
    DCE2_List *list;

    /* Must have a key compare function */
    if (kc == NULL)
        return NULL;

    list = (DCE2_List *)DCE2_Alloc(sizeof(DCE2_List), mtype);
    if (list == NULL)
        return NULL;

    list->type = type;
    list->compare = kc;
    list->data_free = df;
    list->key_free = kf;
    list->flags = flags;
    list->mtype = mtype;

    return list;
}

/********************************************************************
 * Function: DCE2_ListFind()
 *
 * Trys to find a node in the list using key passed in.  If list
 * is splayed, found node is moved to front of list.  The data
 * associated with the node is returned.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *  void *
 *      Pointer to a key.
 *
 * Returns:
 *  void *
 *      If the key is found, the data associated with the node
 *          is returned.
 *      NULL is returned if the item cannot be found given the key.
 *
 ********************************************************************/
void * DCE2_ListFind(DCE2_List *list, void *key)
{
    DCE2_ListNode *n;

    if (list == NULL)
        return NULL;

    for (n = list->head; n != NULL; n = n->next)
    {
        int comp = list->compare(key, n->key);
        if (comp == 0)
        {
            /* Found it, break out */
            break;
        }
        else if ((comp < 0) && (list->type == DCE2_LIST_TYPE__SORTED))
        {
            /* Don't look any more if the list is sorted */
            return NULL;
        }
    }

    if (n != NULL)
    {
        /* If list is splayed, move found node to front of list */
        if ((list->type == DCE2_LIST_TYPE__SPLAYED) &&
            (n != list->head))
        {
            n->prev->next = n->next;

            if (n->next != NULL)
                n->next->prev = n->prev;
            else  /* it's the tail */
                list->tail = n->prev;

            n->prev = NULL;
            n->next = list->head;
            list->head->prev = n;
            list->head = n;
        }

        return n->data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_ListFindKey()
 *
 * Trys to find a node in the list using key passed in.  If list
 * is splayed, found node is moved to front of list.  Returns
 * whether or not the key is associated with a node in the list.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *  void *
 *      Pointer to a key.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__SUCCESS if the key is found.
 *      DCE2_RET__ERROR if the key is not found.
 *
 ********************************************************************/
DCE2_Ret DCE2_ListFindKey(DCE2_List *list, void *key)
{
    DCE2_ListNode *n;

    if (list == NULL)
        return DCE2_RET__ERROR;

    for (n = list->head; n != NULL; n = n->next)
    {
        int comp = list->compare(key, n->key);
        if (comp == 0)
        {
            /* Found it, break out */
            break;
        }
        else if ((comp < 0) && (list->type == DCE2_LIST_TYPE__SORTED))
        {
            /* Don't look any more if the list is sorted */
            return DCE2_RET__ERROR;
        }
    }

    if (n != NULL)
    {
        /* If list is splayed, move found node to front of list */
        if ((list->type == DCE2_LIST_TYPE__SPLAYED) &&
            (n != list->head))
        {
            n->prev->next = n->next;

            if (n->next != NULL)
                n->next->prev = n->prev;
            else  /* it's the tail */
                list->tail = n->prev;

            n->prev = NULL;
            n->next = list->head;
            list->head->prev = n;
            list->head = n;
        }

        return DCE2_RET__SUCCESS;
    }

    return DCE2_RET__ERROR;
}

/********************************************************************
 * Function: DCE2_ListInsert()
 *
 * Adds a new node to the list with the key and data supplied.
 * If no duplicates are allowed in the key is searched for first
 * to see if a node is already present in the list.  If sorted,
 * the node is inserted into the list based on the key compare
 * function associated with the list object.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *  void *
 *      Pointer to a key to associate with data.
 *  void *
 *      Pointer to the data to insert into the list.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__DUPLICATE if an entry with the key is already
 *          in the list and no duplicates are allowed.
 *      DCE2_RET__SUCCESS if a new node with key and data is
 *          successfully inserted into the list.
 *      DCE2_RET__ERROR if memory cannot be allocated for the
 *          new node or a NULL list object was passed in.
 *
 ********************************************************************/
DCE2_Ret DCE2_ListInsert(DCE2_List *list, void *key, void *data)
{
    DCE2_ListNode *n;
    DCE2_ListNode *last = NULL;
    int dup_check = 0;

    if (list == NULL)
        return DCE2_RET__ERROR;

    if (list->flags & DCE2_LIST_FLAG__NO_DUPS)
    {
        for (last = list->head; last != NULL; last = last->next)
        {
            int comp = list->compare(key, last->key);
            if (comp == 0)
            {
                /* It's already in the list */
                return DCE2_RET__DUPLICATE;
            }
            else if ((comp < 0) && (list->type == DCE2_LIST_TYPE__SORTED))
            {
                /* Break out here so as to insert after this node since
                 * the list is sorted */
                break;
            }
        }

        dup_check = 1;
    }

    n = (DCE2_ListNode *)DCE2_Alloc(sizeof(DCE2_ListNode), list->mtype);
    if (n == NULL)
        return DCE2_RET__ERROR;

    n->key = key;
    n->data = data;

    if ((list->type != DCE2_LIST_TYPE__SORTED) ||
        (list->head == NULL))
    {
        if (list->flags & DCE2_LIST_FLAG__INS_TAIL)
            DCE2_ListInsertTail(list, n);
        else
            DCE2_ListInsertHead(list, n);
    }
    else if (dup_check)  /* and the list is sorted */
    {
        if (last == NULL)
            DCE2_ListInsertTail(list, n);
        else
            DCE2_ListInsertBefore(list, n, last);
    }
    else
    {
        DCE2_ListNode *tmp;

        for (tmp = list->head; tmp != NULL; tmp = tmp->next)
        {
            if (list->compare(key, tmp->key) <= 0)
                break;
        }

        if (tmp == NULL)
            DCE2_ListInsertTail(list, n);
        else if (tmp == list->head)
            DCE2_ListInsertHead(list, n);
        else
            DCE2_ListInsertBefore(list, n, tmp);
    }

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ListRemove()
 *
 * Removes the node in the list with the specified key.  If
 * data free and key free functions were given with the creation
 * of the list object, they are called with the data and key
 * respectively.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *  void *
 *      Pointer to a key.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if a node in the list with the specified
 *          key cannot be found or the list object passed in is NULL.
 *      DCE2_RET__SUCCESS if the node is successfully removed from
 *          the list.
 *
 ********************************************************************/
DCE2_Ret DCE2_ListRemove(DCE2_List *list, void *key)
{
    DCE2_ListNode *n;

    if (list == NULL)
        return DCE2_RET__ERROR;

    for (n = list->head; n != NULL; n = n->next)
    {
        int comp = list->compare(key, n->key);
        if (comp == 0)
        {
            /* Found it */
            break;
        }
        else if ((comp < 0) && (list->type == DCE2_LIST_TYPE__SORTED))
        {
            /* Won't find it after this since the list is sorted */
            return DCE2_RET__ERROR;
        }
    }

    if (n == NULL)
        return DCE2_RET__ERROR;

    if (n == list->head)
        list->head = n->next;
    if (n == list->tail)
        list->tail = n->prev;
    if (n->prev != NULL)
        n->prev->next = n->next;
    if (n->next != NULL)
        n->next->prev = n->prev;

    if (list->key_free != NULL)
        list->key_free(n->key);

    if (list->data_free != NULL)
        list->data_free(n->data);

    DCE2_Free((void *)n, sizeof(DCE2_ListNode), list->mtype);

    list->num_nodes--;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_ListFirst()
 *
 * Returns a pointer to the data of the first node in the list.
 * Sets a current pointer to the first node in the list for
 * iterating over the list.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns:
 *  void *
 *      The data in the first node in the list.
 *      NULL if the list object passed in is NULL, or there are
 *          no items in the list.
 *
 ********************************************************************/
void * DCE2_ListFirst(DCE2_List *list)
{
    if (list == NULL)
        return NULL;

    list->current = list->head;
    list->next = NULL;

    if (list->current != NULL)
        return list->current->data;

    return NULL;
}

/********************************************************************
 * Function: DCE2_ListNext()
 *
 * Increments the current pointer in the list to the next node in
 * the list and returns the data associated with it.  This in
 * combination with DCE2_ListFirst is useful in a for loop to
 * iterate over the items in a list.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns:
 *  void *
 *      The data in the next node in the list.
 *      NULL if the list object passed in is NULL, or we are at
 *          the end of the list and there are no next nodes.
 *
 ********************************************************************/
void * DCE2_ListNext(DCE2_List *list)
{
    if (list == NULL)
        return NULL;

    if (list->next != NULL)
    {
        list->current = list->next;
        list->next = NULL;
        return list->current->data;
    }
    else if (list->current != NULL)
    {
        list->current = list->current->next;
        if (list->current != NULL)
            return list->current->data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_ListLast()
 *
 * Returns a pointer to the data of the last node in the list.
 * Sets a current pointer to the last node in the list for
 * iterating over the list backwards.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns:
 *  void *
 *      The data in the last node in the list.
 *      NULL if the list object passed in is NULL, or there are
 *          no items in the list.
 *
 ********************************************************************/
void * DCE2_ListLast(DCE2_List *list)
{
    if (list == NULL)
        return NULL;

    list->current = list->tail;
    list->prev = NULL;

    if (list->current != NULL)
        return list->current->data;

    return NULL;
}

/********************************************************************
 * Function: DCE2_ListPrev()
 *
 * Puts the current pointer in the list to the previous node in
 * the list and returns the data associated with it.  This in
 * combination with DCE2_ListLast is useful in a for loop to
 * iterate over the items in a list in backwards order.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns:
 *  void *
 *      The data in the previous node in the list.
 *      NULL if the list object passed in is NULL, or we are at
 *          the beginning of the list and there are no previous nodes.
 *
 ********************************************************************/
void * DCE2_ListPrev(DCE2_List *list)
{
    if (list == NULL)
        return NULL;

    if (list->prev != NULL)
    {
        list->current = list->prev;
        list->prev = NULL;
        return list->current->data;
    }
    else if (list->current != NULL)
    {
        list->current = list->current->prev;
        if (list->current != NULL)
            return list->current->data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_ListRemoveCurrent()
 *
 * Removes the current node pointed to in the list.  This is set
 * when a call to DCE2_ListFirst or DCE2_ListNext is called.  For
 * either of these if data is returned and the user want to remove
 * that data from the list, this function should be called.
 * Sets a next pointer, so a next call to DCE2_ListNext will point
 * to the node after the deleted one.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_ListRemoveCurrent(DCE2_List *list)
{
    if (list == NULL)
        return;

    if (list->current == NULL)
        return;

    list->next = list->current->next;
    list->prev = list->current->prev;

    if (list->current == list->head)
        list->head = list->current->next;
    if (list->current == list->tail)
        list->tail = list->current->prev;
    if (list->current->prev != NULL)
        list->current->prev->next = list->current->next;
    if (list->current->next != NULL)
        list->current->next->prev = list->current->prev;

    if (list->key_free != NULL)
        list->key_free(list->current->key);

    if (list->data_free != NULL)
        list->data_free(list->current->data);

    DCE2_Free((void *)list->current, sizeof(DCE2_ListNode), list->mtype);
    list->current = NULL;

    list->num_nodes--;
}

/********************************************************************
 * Function: DCE2_ListEmpty()
 *
 * Removes all of the nodes in a list.  Does not delete the list
 * object itself.  Calls data free and key free functions for
 * data and key if they are not NULL.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_ListEmpty(DCE2_List *list)
{
    DCE2_ListNode *n;

    if (list == NULL)
        return;

    n = list->head;

    while (n != NULL)
    {
        DCE2_ListNode *tmp = n->next;

        if (list->data_free != NULL)
            list->data_free(n->data);

        if (list->key_free != NULL)
            list->key_free(n->key);

        DCE2_Free((void *)n, sizeof(DCE2_ListNode), list->mtype);
        n = tmp;
    }

    list->head = list->tail = list->current = NULL;
    list->num_nodes = 0;
}

/********************************************************************
 * Function: DCE2_ListDestroy()
 *
 * Destroys the list object and all of the data associated with it.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_ListDestroy(DCE2_List *list)
{
    if (list == NULL)
        return;

    DCE2_ListEmpty(list);
    DCE2_Free(list, sizeof(DCE2_List), list->mtype);
}

/********************************************************************
 * Function: DCE2_ListInsertTail()
 *
 * Private function for inserting a node at the end of the list.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *  DCE2_ListNode *
 *      A pointer to the list node to insert.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ListInsertTail(DCE2_List *list, DCE2_ListNode *n)
{
    if ((list == NULL) || (n == NULL))
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) List and/or list node passed in was NULL",
                 __FILE__, __LINE__);
        return;
    }

    if (list->tail == NULL)
    {
        list->tail = list->head = n;
        n->prev = n->next = NULL;
    }
    else
    {
        n->prev = list->tail;
        n->next = NULL;
        list->tail->next = n;
        list->tail = n;
    }

    list->num_nodes++;
}

/********************************************************************
 * Function: DCE2_ListInsertHead()
 *
 * Private function for inserting a node at the front of the list.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *  DCE2_ListNode *
 *      A pointer to the list node to insert.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ListInsertHead(DCE2_List *list, DCE2_ListNode *n)
{
    if ((list == NULL) || (n == NULL))
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) List and/or list node passed in was NULL",
                 __FILE__, __LINE__);
        return;
    }

    if (list->head == NULL)
    {
        list->head = list->tail = n;
        n->prev = n->next = NULL;
    }
    else
    {
        n->prev = NULL;
        n->next = list->head;
        list->head->prev = n;
        list->head = n;
    }

    list->num_nodes++;
}

/********************************************************************
 * Function: DCE2_ListInsertBefore()
 *
 * Private function for inserting a node before a given node in
 * the list.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *  DCE2_ListNode *
 *      A pointer to the list node to insert.
 *  DCE2_ListNode *
 *      A pointer to the list node to insert this node before.
 *
 * Returns: None
 *
 ********************************************************************/
static void DCE2_ListInsertBefore(DCE2_List *list, DCE2_ListNode *insert, DCE2_ListNode *front)
{
    if ((list == NULL) || (insert == NULL) || (front == NULL))
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) List, insert node and/or front node passed in "
                 "was NULL", __FILE__, __LINE__);
        return;
    }

    if (front == list->head)
    {
        DCE2_ListInsertHead(list, insert);
    }
    else
    {
        insert->prev = front->prev;
        insert->next = front;
        front->prev->next = insert;
        front->prev = insert;

        list->num_nodes++;
    }
}

/********************************************************************
 * Function: DCE2_QueueNew()
 *
 * Creates and initializes a new queue object.
 *
 * Arguments:
 *  DCE2_QueueDataFree
 *      An optional free function for the data inserted into
 *      the queue.  If NULL is passed in, the user will be
 *      responsible for freeing data left in the queue.
 *  DCE2_MemType
 *      The type of memory to associate dynamically allocated
 *      memory with.
 *
 * Returns:
 *  DCE2_Queue *
 *      Pointer to a new queue object.
 *      NULL if unable to allocate memory for the object.
 *
 ********************************************************************/
DCE2_Queue * DCE2_QueueNew(DCE2_QueueDataFree df, DCE2_MemType mtype)
{
    DCE2_Queue *queue;

    queue = (DCE2_Queue *)DCE2_Alloc(sizeof(DCE2_Queue), mtype);
    if (queue == NULL)
        return NULL;

    queue->data_free = df;
    queue->mtype = mtype;

    return queue;
}

/********************************************************************
 * Function: DCE2_QueueEnqueue()
 *
 * Inserts data into the queue.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *  void *
 *      Pointer to the data to insert into the queue.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if memory cannot be allocated for a new
 *          queue node or the queue object passed in is NULL.
 *      DCE2_RET__SUCCESS if the data is successfully added to
 *          the queue.
 *
 ********************************************************************/
DCE2_Ret DCE2_QueueEnqueue(DCE2_Queue *queue, void *data)
{
    DCE2_QueueNode *n;

    if (queue == NULL)
        return DCE2_RET__ERROR;

    n = (DCE2_QueueNode *)DCE2_Alloc(sizeof(DCE2_QueueNode), queue->mtype);
    if (n == NULL)
        return DCE2_RET__ERROR;

    n->data = data;

    if (queue->tail == NULL)
    {
        queue->head = queue->tail = n;
        n->next = NULL;
    }
    else
    {
        queue->tail->next = n;
        n->prev = queue->tail;
        queue->tail = n;
    }

    queue->num_nodes++;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_QueueDequeue()
 *
 * Removes and returns the data in the first node in the queue.
 * Note that the user will have to free the data returned.  The
 * data free function only applies to data that is in the queue
 * when it is emptied or destroyed.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the first node in the queue.
 *      NULL if there are no items in the queue or the queue object
 *          passed in is NULL.
 *
 ********************************************************************/
void * DCE2_QueueDequeue(DCE2_Queue *queue)
{
    DCE2_QueueNode *n;

    if (queue == NULL)
        return NULL;

    n = queue->head;

    if (n != NULL)
    {
        void *data = n->data;

        if (queue->head == queue->tail)
        {
            queue->head = queue->tail = NULL;
        }
        else
        {
            queue->head->next->prev = NULL;
            queue->head = queue->head->next;
        }

        DCE2_Free((void *)n, sizeof(DCE2_QueueNode), queue->mtype);

        queue->num_nodes--;

        return data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_QueueFirst()
 *
 * Returns a pointer to the data of the first node in the queue.
 * Sets a current pointer to the first node in the queue for
 * iterating over the queue.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the first node in the queue.
 *      NULL if the queue object passed in is NULL, or there are
 *          no items in the queue.
 *
 ********************************************************************/
void * DCE2_QueueFirst(DCE2_Queue *queue)
{
    if (queue == NULL)
        return NULL;

    queue->current = queue->head;
    queue->next = NULL;

    if (queue->current != NULL)
        return queue->current->data;

    return NULL;
}

/********************************************************************
 * Function: DCE2_QueueNext()
 *
 * Increments the current pointer in the queue to the next node in
 * the queue and returns the data associated with it.  This in
 * combination with DCE2_QueueFirst is useful in a for loop to
 * iterate over the items in a queue.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the next node in the queue.
 *      NULL if the queue object passed in is NULL, or we are at
 *          the end of the queue and there are no next nodes.
 *
 ********************************************************************/
void * DCE2_QueueNext(DCE2_Queue *queue)
{
    if (queue == NULL)
        return NULL;

    if (queue->next != NULL)
    {
        queue->current = queue->next;
        queue->next = NULL;
        return queue->current->data;
    }
    else if (queue->current != NULL)
    {
        queue->current = queue->current->next;
        if (queue->current != NULL)
            return queue->current->data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_QueueLast()
 *
 * Returns a pointer to the data of the last node in the queue.
 * Sets a current pointer to the last node in the queue for
 * iterating over the queue backwards.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the last node in the queue.
 *      NULL if the queue object passed in is NULL, or there are
 *          no items in the queue.
 *
 ********************************************************************/
void * DCE2_QueueLast(DCE2_Queue *queue)
{
    if (queue == NULL)
        return NULL;

    queue->current = queue->tail;
    queue->prev = NULL;

    if (queue->current != NULL)
        return queue->current->data;

    return NULL;
}

/********************************************************************
 * Function: DCE2_QueuePrev()
 *
 * Puts the current pointer in the queue to the previous node in
 * the queue and returns the data associated with it.  This in
 * combination with DCE2_QueueLast is useful in a for loop to
 * iterate over the items in a queue in backwards order.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the previous node in the queue.
 *      NULL if the queue object passed in is NULL, or we are at
 *          the beginning of the queue and there are no previous nodes.
 *
 ********************************************************************/
void * DCE2_QueuePrev(DCE2_Queue *queue)
{
    if (queue == NULL)
        return NULL;

    if (queue->prev != NULL)
    {
        queue->current = queue->prev;
        queue->prev = NULL;
        return queue->current->data;
    }
    else if (queue->current != NULL)
    {
        queue->current = queue->current->prev;
        if (queue->current != NULL)
            return queue->current->data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_QueueRemoveCurrent()
 *
 * Removes the current node pointed to in the queue.  This is set
 * when a call to DCE2_QueueFirst or DCE2_QueueNext is called.  For
 * either of these if data is returned and the user want to remove
 * that data from the queue, this function should be called.
 * Sets a next pointer, so a next call to DCE2_QueueNext will point
 * to the node after the deleted one.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the list object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_QueueRemoveCurrent(DCE2_Queue *queue)
{
    if (queue == NULL)
        return;

    if (queue->current == NULL)
        return;

    queue->next = queue->current->next;
    queue->prev = queue->current->prev;

    if (queue->current == queue->head)
        queue->head = queue->current->next;
    if (queue->current == queue->tail)
        queue->tail = queue->current->prev;
    if (queue->current->prev != NULL)
        queue->current->prev->next = queue->current->next;
    if (queue->current->next != NULL)
        queue->current->next->prev = queue->current->prev;

    if (queue->data_free != NULL)
        queue->data_free(queue->current->data);

    DCE2_Free((void *)queue->current, sizeof(DCE2_QueueNode), queue->mtype);
    queue->current = NULL;

    queue->num_nodes--;
}

/********************************************************************
 * Function: DCE2_QueueEmpty()
 *
 * Removes all of the nodes in a queue.  Does not delete the queue
 * object itself.  Calls data free function for data if it is
 * not NULL.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_QueueEmpty(DCE2_Queue *queue)
{
    DCE2_QueueNode *n;

    if (queue == NULL)
        return;

    n = queue->head;

    while (n != NULL)
    {
        DCE2_QueueNode *tmp = n->next;

        if (queue->data_free != NULL)
            queue->data_free(n->data);

        DCE2_Free((void *)n, sizeof(DCE2_QueueNode), queue->mtype);
        n = tmp;
    }

    queue->head = queue->tail = queue->current = NULL;
    queue->num_nodes = 0;
}

/********************************************************************
 * Function: DCE2_QueueDestroy()
 *
 * Destroys the queue object and all of the data associated with it.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_QueueDestroy(DCE2_Queue *queue)
{
    if (queue == NULL)
        return;

    DCE2_QueueEmpty(queue);
    DCE2_Free((void *)queue, sizeof(DCE2_Queue), queue->mtype);
}

/********************************************************************
 * Function: DCE2_StackNew()
 *
 * Creates and initializes a new stack object.
 *
 * Arguments:
 *  DCE2_StackDataFree
 *      An optional free function for the data inserted into
 *      the stack.  If NULL is passed in, the user will be
 *      responsible for freeing data left in the stack.
 *  DCE2_MemType
 *      The type of memory to associate dynamically allocated
 *      memory with.
 *
 * Returns:
 *  DCE2_Stack *
 *      Pointer to a new stack object.
 *      NULL if unable to allocate memory for the object.
 *
 ********************************************************************/
DCE2_Stack * DCE2_StackNew(DCE2_StackDataFree df, DCE2_MemType mtype)
{
    DCE2_Stack *stack;

    stack = (DCE2_Stack *)DCE2_Alloc(sizeof(DCE2_Stack), mtype);
    if (stack == NULL)
        return NULL;

    stack->data_free = df;
    stack->mtype = mtype;

    return stack;
}

/********************************************************************
 * Function: DCE2_StackPush()
 *
 * Inserts data onto the stack.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *  void *
 *      Pointer to the data to insert onto the stack.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if memory cannot be allocated for a new
 *          stack node or the stack object passed in is NULL.
 *      DCE2_RET__SUCCESS if the data is successfully added to
 *          the stack.
 *
 ********************************************************************/
DCE2_Ret DCE2_StackPush(DCE2_Stack *stack, void *data)
{
    DCE2_StackNode *n;

    if (stack == NULL)
        return DCE2_RET__ERROR;

    n = (DCE2_StackNode *)DCE2_Alloc(sizeof(DCE2_StackNode), stack->mtype);
    if (n == NULL)
        return DCE2_RET__ERROR;

    n->data = data;

    if (stack->tail == NULL)
    {
        stack->tail = stack->head = n;
        n->prev = NULL;
        n->next = NULL;
    }
    else
    {
        stack->tail->next = n;
        n->prev = stack->tail;
        stack->tail = n;
    }

    stack->num_nodes++;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_StackPop()
 *
 * Removes and returns the data in the last node in the stack.
 * Note that the user will have to free the data returned.  The
 * data free function only applies to data that is in the stack
 * when it is emptied or destroyed.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the last node in the stack.
 *      NULL if there are no items in the stack or the stack object
 *          passed in is NULL.
 *
 ********************************************************************/
void * DCE2_StackPop(DCE2_Stack *stack)
{
    DCE2_StackNode *n;

    if (stack == NULL)
        return NULL;

    n = stack->tail;

    if (n != NULL)
    {
        void *data = n->data;

        stack->tail = stack->tail->prev;
        if (stack->tail == NULL)
            stack->head = NULL;

        DCE2_Free((void *)n, sizeof(DCE2_StackNode), stack->mtype);

        stack->num_nodes--;

        return data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_StackFirst()
 *
 * Returns a pointer to the data of the first node in the stack.
 * Sets a current pointer to the first node in the stack for
 * iterating over the stack.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the first node in the stack.
 *      NULL if the stack object passed in is NULL, or there are
 *          no items in the stack.
 *
 ********************************************************************/
void * DCE2_StackFirst(DCE2_Stack *stack)
{
    if (stack == NULL)
        return NULL;

    stack->current = stack->head;

    if (stack->current != NULL)
        return stack->current->data;

    return NULL;
}

/********************************************************************
 * Function: DCE2_StackNext()
 *
 * Increments the current pointer in the stack to the next node in
 * the stack and returns the data associated with it.  This in
 * combination with DCE2_StackFirst is useful in a for loop to
 * iterate over the items in a stack.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the next node in the stack.
 *      NULL if the stack object passed in is NULL, or we are at
 *          the end of the stack and there are no next nodes.
 *
 ********************************************************************/
void * DCE2_StackNext(DCE2_Stack *stack)
{
    if (stack == NULL)
        return NULL;

    if (stack->current != NULL)
    {
        stack->current = stack->current->next;
        if (stack->current != NULL)
            return stack->current->data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_StackLast()
 *
 * Returns a pointer to the data of the last node in the stack.
 * Sets a current pointer to the last node in the stack for
 * iterating over the stack backwards.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the last node in the stack.
 *      NULL if the stack object passed in is NULL, or there are
 *          no items in the stack.
 *
 ********************************************************************/
void * DCE2_StackLast(DCE2_Stack *stack)
{
    if (stack == NULL)
        return NULL;

    stack->current = stack->tail;

    if (stack->current != NULL)
        return stack->current->data;

    return NULL;
}

/********************************************************************
 * Function: DCE2_StackPrev()
 *
 * Puts the current pointer in the stack to the previous node in
 * the stack and returns the data associated with it.  This in
 * combination with DCE2_StackLast is useful in a for loop to
 * iterate over the items in a stack in backwards order.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the previous node in the stack.
 *      NULL if the stack object passed in is NULL, or we are at
 *          the beginning of the stack and there are no previous nodes.
 *
 ********************************************************************/
void * DCE2_StackPrev(DCE2_Stack *stack)
{
    if (stack == NULL)
        return NULL;

    if (stack->current != NULL)
    {
        stack->current = stack->current->prev;
        if (stack->current != NULL)
            return stack->current->data;
    }

    return NULL;
}

/********************************************************************
 * Function: DCE2_StackEmpty()
 *
 * Removes all of the nodes in a stack.  Does not delete the stack
 * object itself.  Calls data free function for data if it is
 * not NULL.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_StackEmpty(DCE2_Stack *stack)
{
    DCE2_StackNode *n;

    if (stack == NULL)
        return;

    n = stack->head;

    while (n != NULL)
    {
        DCE2_StackNode *tmp = n->next;

        if (stack->data_free != NULL)
            stack->data_free(n->data);

        DCE2_Free((void *)n, sizeof(DCE2_StackNode), stack->mtype);
        n = tmp;
    }

    stack->head = stack->tail = stack->current = NULL;
    stack->num_nodes = 0;
}

/********************************************************************
 * Function: DCE2_StackDestroy()
 *
 * Destroys the stack object and all of the data associated with it.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_StackDestroy(DCE2_Stack *stack)
{
    if (stack == NULL)
        return;

    DCE2_StackEmpty(stack);
    DCE2_Free((void *)stack, sizeof(DCE2_Stack), stack->mtype);
}

/********************************************************************
 * Function: DCE2_CQueueNew()
 *
 * Creates and initializes a new circular queue object.  The
 * circular queue uses a fixed size array and uses indexes to
 * indicate the start and end of the queue.  This type of
 * queue can become full since it is a fixed size.  Used for
 * performance reasons since new nodes do not need to be
 * allocated on the fly.
 *
 * Arguments:
 *  int
 *      The size that should be allocated for the circular
 *      queue storage.
 *  DCE2_CQueueDataFree
 *      An optional free function for the data inserted into
 *      the queue.  If NULL is passed in, the user will be
 *      responsible for freeing data left in the queue.
 *  DCE2_MemType
 *      The type of memory to associate dynamically allocated
 *      memory with.
 *
 * Returns:
 *  DCE2_CQueue *
 *      Pointer to a new queue object.
 *      NULL if unable to allocate memory for the object or the
 *          array for storage.
 *
 ********************************************************************/
DCE2_CQueue * DCE2_CQueueNew(int size, DCE2_CQueueDataFree df, DCE2_MemType mtype)
{
    DCE2_CQueue *cqueue;

    if (size <= 0)
        return NULL;

    cqueue = (DCE2_CQueue *)DCE2_Alloc(sizeof(DCE2_CQueue), mtype);
    if (cqueue == NULL)
        return NULL;

    cqueue->data_free = df;
    cqueue->mtype = mtype;

    cqueue->queue = DCE2_Alloc(size * sizeof(void *), mtype);
    if (cqueue->queue == NULL)
    {
        DCE2_Free(cqueue, sizeof(DCE2_CQueue), mtype);
        return NULL;
    }

    cqueue->size = size;
    cqueue->head_idx = 0;
    cqueue->tail_idx = DCE2_SENTINEL;
    cqueue->cur_idx = DCE2_SENTINEL;

    return cqueue;
}

/********************************************************************
 * Function: DCE2_CQueueEnqueue()
 *
 * Inserts data into the circular queue.
 *
 * Arguments:
 *  DCE2_CQueue *
 *      A pointer to the queue object.
 *  void *
 *      Pointer to the data to insert into the queue.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if the queue is full or the queue object
 *          passed in is NULL.
 *      DCE2_RET__SUCCESS if the data is successfully added to
 *          the queue.
 *
 ********************************************************************/
DCE2_Ret DCE2_CQueueEnqueue(DCE2_CQueue *cqueue, void *data)
{
    if (cqueue == NULL)
        return DCE2_RET__ERROR;

    if (cqueue->num_nodes == (uint32_t)cqueue->size)
        return DCE2_RET__ERROR;

    if (cqueue->tail_idx == DCE2_SENTINEL)
        cqueue->tail_idx = cqueue->head_idx;

    cqueue->queue[cqueue->tail_idx] = data;

    if ((cqueue->tail_idx + 1) == cqueue->size)
        cqueue->tail_idx = 0;
    else
        cqueue->tail_idx++;

    cqueue->num_nodes++;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_CQueueDequeue()
 *
 * Removes and returns the data in the first node in the queue.
 * Note that the user will have to free the data returned.  The
 * data free function only applies to data that is in the queue
 * when it is emptied or destroyed.
 *
 * Arguments:
 *  DCE2_CQueue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the first node in the queue.
 *      NULL if there are no items in the queue or the queue object
 *          passed in is NULL.
 *
 ********************************************************************/
void * DCE2_CQueueDequeue(DCE2_CQueue *cqueue)
{
    void *data;

    if (cqueue == NULL)
        return NULL;

    if (cqueue->num_nodes == 0)
        return NULL;

    data = cqueue->queue[cqueue->head_idx];
    cqueue->queue[cqueue->head_idx] = NULL;

    if ((cqueue->head_idx + 1) == cqueue->size)
        cqueue->head_idx = 0;
    else
        cqueue->head_idx++;

    if (cqueue->head_idx == cqueue->tail_idx)
        cqueue->tail_idx = DCE2_SENTINEL;

    cqueue->num_nodes--;

    return data;
}

/********************************************************************
 * Function: DCE2_CQueueFirst()
 *
 * Returns a pointer to the data of the first node in the queue.
 * Sets a current index to the first node in the queue for
 * iterating over the queue.
 *
 * Arguments:
 *  DCE2_CQueue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the first node in the queue.
 *      NULL if the queue object passed in is NULL, or there are
 *          no items in the queue.
 *
 ********************************************************************/
void * DCE2_CQueueFirst(DCE2_CQueue *cqueue)
{
    if (cqueue == NULL)
        return NULL;

    if (cqueue->tail_idx == DCE2_SENTINEL)
        return NULL;

    cqueue->cur_idx = cqueue->head_idx;

    return cqueue->queue[cqueue->cur_idx];
}

/********************************************************************
 * Function: DCE2_CQueueNext()
 *
 * Increments the current index in the queue to the next node in
 * the queue and returns the data associated with it.  This in
 * combination with DCE2_CQueueFirst is useful in a for loop to
 * iterate over the items in a queue.
 *
 * Arguments:
 *  DCE2_CQueue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  void *
 *      The data in the next node in the queue.
 *      NULL if the queue object passed in is NULL, or we are at
 *          the end of the queue and there are no next nodes.
 *
 ********************************************************************/
void * DCE2_CQueueNext(DCE2_CQueue *cqueue)
{
    if (cqueue == NULL)
        return NULL;

    if ((cqueue->tail_idx == DCE2_SENTINEL) ||
        (cqueue->cur_idx == DCE2_SENTINEL))
        return NULL;

    if ((cqueue->cur_idx + 1) == cqueue->size)
        cqueue->cur_idx = 0;
    else
        cqueue->cur_idx++;

    if (cqueue->cur_idx == cqueue->tail_idx)
    {
        cqueue->cur_idx = DCE2_SENTINEL;
        return NULL;
    }

    return cqueue->queue[cqueue->cur_idx];
}

/********************************************************************
 * Function: DCE2_CQueueEmpty()
 *
 * Removes all of the nodes in a queue.  Does not delete the queue
 * object itself or the storage array.  Calls data free function
 * for data if it is not NULL.
 *
 * Arguments:
 *  DCE2_CQueue *
 *      A pointer to the queue object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CQueueEmpty(DCE2_CQueue *cqueue)
{
    if (cqueue == NULL)
        return;

    while (!DCE2_CQueueIsEmpty(cqueue))
    {
        void *data = DCE2_CQueueDequeue(cqueue);

        if ((data != NULL) && (cqueue->data_free != NULL))
            cqueue->data_free(data);
    }

    cqueue->num_nodes = 0;
    cqueue->head_idx = 0;
    cqueue->tail_idx = DCE2_SENTINEL;
    cqueue->cur_idx = DCE2_SENTINEL;
}

/********************************************************************
 * Function: DCE2_CQueueDestroy()
 *
 * Destroys the queue object and all of the data associated with it.
 *
 * Arguments:
 *  DCE2_CQueue *
 *      A pointer to the queue object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CQueueDestroy(DCE2_CQueue *cqueue)
{
    if (cqueue == NULL)
        return;

    DCE2_CQueueEmpty(cqueue);
    DCE2_Free((void *)cqueue->queue, (cqueue->size * sizeof(void *)), cqueue->mtype);
    DCE2_Free((void *)cqueue, sizeof(DCE2_CQueue), cqueue->mtype);
}

/********************************************************************
 * Function: DCE2_CStackNew()
 *
 * Creates and initializes a new static sized stack object.  The
 * static stack uses a fixed size array and uses indexes to
 * indicate the start and end of the stack.  This type of
 * stack can become full since it is a fixed size.  Used for
 * performance reasons since new nodes do not need to be
 * allocated on the fly.
 *
 * Arguments:
 *  int
 *      The size that should be allocated for the static
 *      stack storage.
 *  DCE2_CStackDataFree
 *      An optional free function for the data inserted into
 *      the stack.  If NULL is passed in, the user will be
 *      responsible for freeing data left in the stack.
 *  DCE2_MemType
 *      The type of memory to associate dynamically allocated
 *      memory with.
 *
 * Returns:
 *  DCE2_CStack *
 *      Pointer to a new stack object.
 *      NULL if unable to allocate memory for the object or the
 *          array for storage.
 *
 ********************************************************************/
DCE2_CStack * DCE2_CStackNew(int size, DCE2_CStackDataFree df, DCE2_MemType mtype)
{
    DCE2_CStack *cstack;

    if (size <= 0)
        return NULL;

    cstack = (DCE2_CStack *)DCE2_Alloc(sizeof(DCE2_CStack), mtype);
    if (cstack == NULL)
        return NULL;

    cstack->data_free = df;
    cstack->mtype = mtype;

    cstack->stack = DCE2_Alloc(size * sizeof(void *), mtype);
    if (cstack->stack == NULL)
    {
        DCE2_Free(cstack, sizeof(DCE2_CStack), mtype);
        return NULL;
    }

    cstack->size = size;
    cstack->tail_idx = DCE2_SENTINEL;
    cstack->cur_idx = DCE2_SENTINEL;

    return cstack;
}

/********************************************************************
 * Function: DCE2_CStackPush()
 *
 * Inserts data into the static stack.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *  void *
 *      Pointer to the data to insert into the stack.
 *
 * Returns:
 *  DCE2_Ret
 *      DCE2_RET__ERROR if the stack is full or the stack object
 *          passed in is NULL.
 *      DCE2_RET__SUCCESS if the data is successfully added to
 *          the stack.
 *
 ********************************************************************/
DCE2_Ret DCE2_CStackPush(DCE2_CStack *cstack, void *data)
{
    if (cstack == NULL)
        return DCE2_RET__ERROR;

    if (cstack->num_nodes == (uint32_t)cstack->size)
        return DCE2_RET__ERROR;

    if (cstack->tail_idx == DCE2_SENTINEL)
        cstack->tail_idx = 0;
    else
        cstack->tail_idx++;

    cstack->stack[cstack->tail_idx] = data;
    cstack->num_nodes++;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function: DCE2_CStackPop()
 *
 * Removes and returns the data in the last node in the stack.
 * Note that the user will have to free the data returned.  The
 * data free function only applies to data that is in the stack
 * when it is emptied or destroyed.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the last node in the stack.
 *      NULL if there are no items in the stack or the stack object
 *          passed in is NULL.
 *
 ********************************************************************/
void * DCE2_CStackPop(DCE2_CStack *cstack)
{
    void *data;

    if (cstack == NULL)
        return NULL;

    if (cstack->num_nodes == 0)
        return NULL;

    data = cstack->stack[cstack->tail_idx];
    cstack->stack[cstack->tail_idx] = NULL;

    if (cstack->tail_idx == 0)
        cstack->tail_idx = DCE2_SENTINEL;
    else
        cstack->tail_idx--;

    cstack->num_nodes--;

    return data;
}

/********************************************************************
 * Function: DCE2_CStackTop()
 *
 * Returns the data on top of the stack.  Does not remove the data
 * from the stack.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data on top of the stack.
 *      NULL if there are no items in the stack or the stack object
 *          passed in is NULL.
 *
 ********************************************************************/
void * DCE2_CStackTop(DCE2_CStack *cstack)
{
    if (cstack == NULL)
        return NULL;

    if (cstack->num_nodes == 0)
        return NULL;

    return cstack->stack[cstack->tail_idx];
}

/********************************************************************
 * Function: DCE2_CStackFirst()
 *
 * Returns a pointer to the data of the first node in the stack
 * array.  Sets a current index to the first node in the stack for
 * iterating over the stack.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the first node in the stack.
 *      NULL if the stack object passed in is NULL, or there are
 *          no items in the stack.
 *
 ********************************************************************/
void * DCE2_CStackFirst(DCE2_CStack *cstack)
{
    if (cstack == NULL)
        return NULL;

    if (cstack->num_nodes == 0)
        return NULL;

    cstack->cur_idx = 0;

    return cstack->stack[cstack->cur_idx];
}

/********************************************************************
 * Function: DCE2_CStackNext()
 *
 * Increments the current index in the stack to the next node in
 * the stack and returns the data associated with it.  This in
 * combination with DCE2_CStackFirst is useful in a for loop to
 * iterate over the items in a stack.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  void *
 *      The data in the next node in the stack.
 *      NULL if the stack object passed in is NULL, or we are at
 *          the end of the stack and there are no next nodes.
 *
 ********************************************************************/
void * DCE2_CStackNext(DCE2_CStack *cstack)
{
    if (cstack == NULL)
        return NULL;

    if ((uint32_t)(cstack->cur_idx + 1) == cstack->num_nodes)
        return NULL;

    cstack->cur_idx++;

    return cstack->stack[cstack->cur_idx];
}

/********************************************************************
 * Function: DCE2_CStackEmpty()
 *
 * Removes all of the nodes in a stack.  Does not delete the stack
 * object itself or the storage array.  Calls data free function
 * for data if it is not NULL.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CStackEmpty(DCE2_CStack *cstack)
{
    if (cstack == NULL)
        return;

    while (!DCE2_CStackIsEmpty(cstack))
    {
        void *data = DCE2_CStackPop(cstack);

        if ((data != NULL) && (cstack->data_free != NULL))
            cstack->data_free(data);
    }

    cstack->num_nodes = 0;
    cstack->tail_idx = DCE2_SENTINEL;
    cstack->cur_idx = DCE2_SENTINEL;
}

/********************************************************************
 * Function: DCE2_CStackDestroy()
 *
 * Destroys the stack object and all of the data associated with it.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_CStackDestroy(DCE2_CStack *cstack)
{
    if (cstack == NULL)
        return;

    DCE2_CStackEmpty(cstack);
    DCE2_Free((void *)cstack->stack, (cstack->size * sizeof(void *)), cstack->mtype);
    DCE2_Free((void *)cstack, sizeof(DCE2_CStack), cstack->mtype);
}

