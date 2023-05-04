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

#ifndef _DCE2_LIST_H_
#define _DCE2_LIST_H_

#include "dce2_memory.h"
#include "dce2_utils.h"
#include "sf_types.h"
#include "snort_debug.h"

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_ListType
{
    DCE2_LIST_TYPE__NORMAL = 0,  /* Don't do anything special */
    DCE2_LIST_TYPE__SORTED,      /* Sort list by key */
    DCE2_LIST_TYPE__SPLAYED      /* Move most recently accessed node to head */

} DCE2_ListType;

typedef enum _DCE2_ListFlags
{
    DCE2_LIST_FLAG__NO_FLAG  = 0x00,   /* No flags */
    DCE2_LIST_FLAG__NO_DUPS  = 0x01,   /* No duplicate keys in list */
    DCE2_LIST_FLAG__INS_TAIL = 0x02    /* Insert at tail - default is to insert at head */

} DCE2_ListFlags;

/********************************************************************
 * Callbacks
 ********************************************************************/
typedef void (*DCE2_ListDataFree)(void *);
typedef void (*DCE2_ListKeyFree)(void *);
typedef int (*DCE2_ListKeyCompare)(const void *, const void *);

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_ListNode
{
    void *key;
    void *data;
    struct _DCE2_ListNode *prev;
    struct _DCE2_ListNode *next;

} DCE2_ListNode;

typedef struct _DCE2_List
{
    DCE2_ListType type;
    DCE2_MemType mtype;
    uint32_t num_nodes;
    int flags;
    DCE2_ListKeyCompare compare;
    DCE2_ListDataFree data_free;
    DCE2_ListKeyFree key_free;
    struct _DCE2_ListNode *head;
    struct _DCE2_ListNode *tail;
    struct _DCE2_ListNode *current;
    struct _DCE2_ListNode *next;
    struct _DCE2_ListNode *prev;

} DCE2_List;

typedef struct _DCE2_QueueNode
{
    void *data;
    struct _DCE2_QueueNode *prev;
    struct _DCE2_QueueNode *next;

} DCE2_QueueNode;

typedef DCE2_ListDataFree DCE2_QueueDataFree;

typedef struct _DCE2_Queue
{
    uint32_t num_nodes;
    DCE2_MemType mtype;
    DCE2_QueueDataFree data_free;
    struct _DCE2_QueueNode *current;
    struct _DCE2_QueueNode *head;
    struct _DCE2_QueueNode *tail;
    struct _DCE2_QueueNode *next;
    struct _DCE2_QueueNode *prev;

} DCE2_Queue;

typedef struct _DCE2_StackNode
{
    void *data;
    struct _DCE2_StackNode *prev;
    struct _DCE2_StackNode *next;

} DCE2_StackNode;

typedef DCE2_ListDataFree DCE2_StackDataFree;

typedef struct _DCE2_Stack
{
    uint32_t num_nodes;
    DCE2_MemType mtype;
    DCE2_StackDataFree data_free;
    struct _DCE2_StackNode *current;
    struct _DCE2_StackNode *head;
    struct _DCE2_StackNode *tail;

} DCE2_Stack;

/* Circular queue */
typedef DCE2_ListDataFree DCE2_CQueueDataFree;

typedef struct _DCE2_CQueue
{
    uint32_t num_nodes;
    DCE2_MemType mtype;
    DCE2_CQueueDataFree data_free;
    int size;
    int cur_idx;
    void **queue;
    int head_idx;
    int tail_idx;

} DCE2_CQueue;

typedef DCE2_ListDataFree DCE2_CStackDataFree;

typedef struct _DCE2_CStack
{
    uint32_t num_nodes;
    DCE2_MemType mtype;
    DCE2_CStackDataFree data_free;
    int size;
    void **stack;
    int tail_idx;
    int cur_idx;

} DCE2_CStack;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
DCE2_List * DCE2_ListNew(DCE2_ListType, DCE2_ListKeyCompare, DCE2_ListDataFree,
                         DCE2_ListKeyFree, int, DCE2_MemType);
void * DCE2_ListFind(DCE2_List *, void *);
DCE2_Ret DCE2_ListFindKey(DCE2_List *, void *);
DCE2_Ret DCE2_ListInsert(DCE2_List *, void *, void *);
DCE2_Ret DCE2_ListRemove(DCE2_List *, void *);
void * DCE2_ListFirst(DCE2_List *);
void * DCE2_ListNext(DCE2_List *);
void * DCE2_ListLast(DCE2_List *);
void * DCE2_ListPrev(DCE2_List *);
void DCE2_ListRemoveCurrent(DCE2_List *);
static inline int DCE2_ListIsEmpty(DCE2_List *);
void DCE2_ListEmpty(DCE2_List *);
void DCE2_ListDestroy(DCE2_List *);

DCE2_Queue * DCE2_QueueNew(DCE2_QueueDataFree, DCE2_MemType);
DCE2_Ret DCE2_QueueEnqueue(DCE2_Queue *, void *);
void * DCE2_QueueDequeue(DCE2_Queue *);
void * DCE2_QueueFirst(DCE2_Queue *);
void * DCE2_QueueNext(DCE2_Queue *);
void * DCE2_QueueLast(DCE2_Queue *);
void * DCE2_QueuePrev(DCE2_Queue *);
void DCE2_QueueRemoveCurrent(DCE2_Queue *);
static inline int DCE2_QueueIsEmpty(DCE2_Queue *);
void DCE2_QueueEmpty(DCE2_Queue *);
void DCE2_QueueDestroy(DCE2_Queue *);

DCE2_Stack * DCE2_StackNew(DCE2_StackDataFree, DCE2_MemType);
DCE2_Ret DCE2_StackPush(DCE2_Stack *, void *);
void * DCE2_StackPop(DCE2_Stack *);
void * DCE2_StackFirst(DCE2_Stack *);
void * DCE2_StackNext(DCE2_Stack *);
void * DCE2_StackLast(DCE2_Stack *);
void * DCE2_StackPrev(DCE2_Stack *);
static inline int DCE2_StackIsEmpty(DCE2_Stack *);
void DCE2_StackEmpty(DCE2_Stack *);
void DCE2_StackDestroy(DCE2_Stack *);

DCE2_CQueue * DCE2_CQueueNew(int, DCE2_CQueueDataFree, DCE2_MemType);
DCE2_Ret DCE2_CQueueEnqueue(DCE2_CQueue *, void *);
void * DCE2_CQueueDequeue(DCE2_CQueue *);
void * DCE2_CQueueFirst(DCE2_CQueue *);
void * DCE2_CQueueNext(DCE2_CQueue *);
static inline int DCE2_CQueueIsEmpty(DCE2_CQueue *);
void DCE2_CQueueEmpty(DCE2_CQueue *);
void DCE2_CQueueDestroy(DCE2_CQueue *);

DCE2_CStack * DCE2_CStackNew(int, DCE2_CStackDataFree, DCE2_MemType);
DCE2_Ret DCE2_CStackPush(DCE2_CStack *, void *);
void * DCE2_CStackPop(DCE2_CStack *);
void * DCE2_CStackTop(DCE2_CStack *);
void * DCE2_CStackFirst(DCE2_CStack *);
void * DCE2_CStackNext(DCE2_CStack *);
static inline int DCE2_CStackIsEmpty(DCE2_CStack *);
void DCE2_CStackEmpty(DCE2_CStack *);
void DCE2_CStackDestroy(DCE2_CStack *);

/********************************************************************
 * Function: DCE2_ListIsEmpty()
 *
 * Determines whether or not the list has any items in it
 * currently.
 *
 * Arguments:
 *  DCE2_List *
 *      A pointer to the list object.
 *
 * Returns:
 *  int
 *      1 if the list has zero nodes in it or the list object
 *          passed in is NULL.
 *      0 if the list has one or more nodes in it.
 *
 ********************************************************************/
static inline int DCE2_ListIsEmpty(DCE2_List *list)
{
    if (list == NULL) return 1;
    if (list->num_nodes == 0) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_QueueIsEmpty()
 *
 * Determines whether or not the queue has any items in it
 * currently.
 *
 * Arguments:
 *  DCE2_Queue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  int
 *      1 if the queue has zero nodes in it or the queue object
 *          passed in is NULL.
 *      0 if the queue has one or more nodes in it.
 *
 ********************************************************************/
static inline int DCE2_QueueIsEmpty(DCE2_Queue *queue)
{
    if (queue == NULL) return 1;
    if (queue->num_nodes == 0) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_StackIsEmpty()
 *
 * Determines whether or not the stack has any items in it
 * currently.
 *
 * Arguments:
 *  DCE2_Stack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  int
 *      1 if the stack has zero nodes in it or the stack object
 *          passed in is NULL.
 *      0 if the stack has one or more nodes in it.
 *
 ********************************************************************/
static inline int DCE2_StackIsEmpty(DCE2_Stack *stack)
{
    if (stack == NULL) return 1;
    if (stack->num_nodes == 0) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_CQueueIsEmpty()
 *
 * Determines whether or not the queue has any items in it
 * currently.
 *
 * Arguments:
 *  DCE2_CQueue *
 *      A pointer to the queue object.
 *
 * Returns:
 *  int
 *      1 if the queue has zero nodes in it or the queue object
 *          passed in is NULL.
 *      0 if the queue has one or more nodes in it.
 *
 ********************************************************************/
static inline int DCE2_CQueueIsEmpty(DCE2_CQueue *cqueue)
{
    if (cqueue == NULL) return 1;
    if (cqueue->num_nodes == 0) return 1;
    return 0;
}

/********************************************************************
 * Function: DCE2_CStackIsEmpty()
 *
 * Determines whether or not the stack has any items in it
 * currently.
 *
 * Arguments:
 *  DCE2_CStack *
 *      A pointer to the stack object.
 *
 * Returns:
 *  int
 *      1 if the stack has zero nodes in it or the stack object
 *          passed in is NULL.
 *      0 if the stack has one or more nodes in it.
 *
 ********************************************************************/
static inline int DCE2_CStackIsEmpty(DCE2_CStack *cstack)
{
    if (cstack == NULL) return 1;
    if (cstack->num_nodes == 0) return 1;
    return 0;
}

#endif   /* _DCE2_LIST_H_ */

