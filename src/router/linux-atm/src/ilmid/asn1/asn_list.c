/*
 * asn_list.c  - borrowed from Murray Goldberg
 * 
 * the following routines implement the list data structure 
 *
 * Copyright (C) 1992 the University of British Columbia
 * 
 * This library is free software; you can redistribute it and/or
 * modify it provided that this copyright/license information is retained
 * in original form.
 *
 * If you modify this file, you must clearly indicate your changes.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include "asn_config.h"
#include "asn_list.h"



/*
 * this routine removes the current node from the list. After removal the
 * current pointer will point to the next node in line, or NULL if the
 * removed item was at the tail of the list.
 */
void
AsnListRemove PARAMS( (list),
AsnList* list )
{
    AsnListNode* node;

    if( list->curr )
	{
	if( list->curr->next )
	    list->curr->next->prev = list->curr->prev;
	else
	    list->last = list->curr->prev;

	if( list->curr->prev )
	    list->curr->prev->next = list->curr->next;
	else
	    list->first = list->curr->next;

	node       = list->curr;

	list->curr = list->curr->next;
	list->count--;

	free( node );
	}
}

/*
 * this creates a new node after the current node and returns the
 * address of the memory allocated for data. The current pointer is changed
 * to point to the newly added node in the list. If the current pointer is
 * initially off the list then this operation fails.
 */
void*
AsnListAdd PARAMS((list),
AsnList* list )
{
    AsnListNode* newNode;
    void*      dataAddr;

    if( list->curr )
        {
	newNode  = (AsnListNode *) Asn1Alloc( sizeof(AsnListNode) + list->dataSize );
	dataAddr = (void *) &(newNode->data);

	newNode->next = list->curr->next;
	newNode->prev = list->curr;
	if( list->curr->next )
	    list->curr->next->prev = newNode;
	else
	    list->last = newNode;
	list->curr->next = newNode;

	list->curr = newNode;
	list->count++;
	}

    else
	dataAddr = NULL;

    return( dataAddr );
}

/*
 * this creates a new node before the current node and returns the
 * address of the memory allocated for data. The current pointer is changed 
 * to point to the newly added node in the list. If the current pointer is
 * initially off the list then this operation fails.
 */
void*
AsnListInsert PARAMS( (list),
AsnList* list )
{
    AsnListNode* newNode;
    void*      dataAddr;

    if( list->curr )
        {
	newNode  = (AsnListNode *) Asn1Alloc( sizeof(AsnListNode) + list->dataSize );
	dataAddr = (void *) &(newNode->data);

	newNode->next = list->curr;
	newNode->prev = list->curr->prev;
	if( list->curr->prev )
	    list->curr->prev->next = newNode;
	else
	    list->first  = newNode;
	list->curr->prev = newNode;

	list->curr = newNode;
	list->count++;
	}

    else
	dataAddr = NULL;

    return( dataAddr );
}


void
AsnListInit PARAMS((list, dataSize),
AsnList* list _AND_
int dataSize)
{
    list->first = list->last = list->curr = NULL;
    list->count = 0;
    list->dataSize = dataSize;

}  /* AsnListInit */


AsnList*
AsnListNew PARAMS( (dataSize),
int dataSize )
{
    AsnList* list;

    list = (AsnList *) Asn1Alloc( sizeof(AsnList) );
    list->first = list->last = list->curr = NULL;
    list->count = 0;
    list->dataSize = dataSize;

    return( list );
}

/*
 * backs up the current pointer by one and returns the data address of the new
 * current node. If the current pointer is off the list, the new current node
 * will be the last node of the list (unless the list is empty).
 */
void*
AsnListPrev PARAMS( (list),
AsnList* list )
{
    void* retVal;

    if( list->curr == NULL )
	list->curr = list->last;
    else
	list->curr = list->curr->prev;

    if( list->curr == NULL )
	retVal = NULL;
    else
	retVal = (void *) &(list->curr->data);

    return( retVal );
}

/*
 * advances the current pointer by one and returns the data address of the new
 * current node. If the current pointer is off the list, the new current node
 * will be the first node of the list (unless the list is empty).
 */
void*
AsnListNext PARAMS( (list),
AsnList* list )
{
    void* retVal;

    if( list->curr == NULL )
	list->curr = list->first;
    else
	list->curr = list->curr->next;

    if( list->curr == NULL )
	retVal = NULL;
    else
	retVal = (void *) &(list->curr->data);

    return( retVal );
}

/*
 * returns the data address of the last node (if there is one) and sets the
 * current pointer to this node.
 */
void*
AsnListLast PARAMS((list),
AsnList* list )
{
    void* retVal;

    list->curr = list->last;

    if( list->curr == NULL )
	retVal = NULL;
    else
	retVal = (void *) &(list->curr->data);

    return( retVal );
}

/*
 * returns the data address of the first node (if there is one) and sets the
 * current pointer to this node.
 */
void*
AsnListFirst PARAMS( (list),
AsnList* list )
{
    void* retVal;

    list->curr = list->first;

    if( list->curr == NULL )
	retVal = NULL;
    else
	retVal = (void *) &(list->curr->data);

    return( retVal );
}

/*
 * this creates a new node at the beginning of the list and returns the
 * address of the memory allocated for data. The current pointer is changed 
 * to point to the newly added node in the list.
 */
void*
AsnListPrepend PARAMS( (list),
AsnList* list )
{
    AsnListNode* newNode;
    void*      dataAddr;

    newNode  = (AsnListNode *) Asn1Alloc( sizeof(AsnListNode) + list->dataSize );
    dataAddr = (void *) &(newNode->data);

    newNode->prev = NULL;

    if( list->first == NULL )
	{
	newNode->next = NULL;
	list->first   = list->last = newNode;
	}
    else
	{
	newNode->next     = list->first;
	list->first->prev = newNode;
	list->first       = newNode;
	}

    list->curr = newNode;
    list->count++;

    return( dataAddr );
}

/*
 * this creates a new node at the end of the list and returns the
 * address of the memory allocated for data. The current pointer is changed
 * to point to the newly added node in the list.
 */
void*
AsnListAppend PARAMS( (list),
AsnList* list )
{
    AsnListNode* newNode;
    void*      dataAddr;

    newNode  = (AsnListNode *) Asn1Alloc( sizeof(AsnListNode) + list->dataSize );
    dataAddr = (void *) &(newNode->data);

    newNode->next = NULL;

    if( list->last == NULL )
        {
        newNode->prev = NULL;
        list->first   = list->last = newNode;
        }
    else
        {
        newNode->prev     = list->last;
        list->last->next  = newNode;
        list->last        = newNode;
        }

    list->curr = newNode;
    list->count++;

    return( dataAddr );
}

void*
AsnListCurr PARAMS( (list),
AsnList* list )
{
    void* retVal;

    if( list->curr )
	retVal = (void *) &(list->curr->data);
    else
	retVal = NULL;

    return( retVal );
}

int
AsnListCount PARAMS( (list),
AsnList* list )
{
    return( list->count );
}





