/**************************************************************************
 * FILE PURPOSE	:  	Linked List library.
 **************************************************************************
 * FILE NAME	:   listlib.c
 *
 * DESCRIPTION	:
 * 	Implementation of a singly linked list.
 *
 *	CALL-INs:
 *
 *	CALL-OUTs:
 *
 *	User-Configurable Items:
 *
 *	(C) Copyright 2002, Texas Instruments, Inc.
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "listlib.h"

/**************************************************************************
 * FUNCTION NAME : list_add
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is called to add a node 'ptr_node' to the 'ptr_list'.
 ***************************************************************************/
void list_add (LIST_NODE **ptr_list, LIST_NODE *ptr_node)
{
	LIST_NODE*	ptr_head;

	/* Check if the list is empty ? */
	if (*ptr_list == NULL)
	{
		/* YES the list is empty. Initialize the links */
		ptr_node->p_next = NULL;

		/* Initialize the LIST */
		*ptr_list = ptr_node;
		return;
	}

	/* No the list was NOT empty. Add the node to the beginning of list. */

	/* Get the pointer to the head of the list. */
	ptr_head = *ptr_list;

	/* The head of the list now points to the last node. */
	ptr_node->p_next  = ptr_head;
	*ptr_list = ptr_node;
	return;
}

/**************************************************************************
 * FUNCTION NAME : list_cat
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is called to concatenate the src list to the end of the 
 *  destination list.
 ***************************************************************************/
void list_cat (LIST_NODE **ptr_dst, LIST_NODE **ptr_src)
{
	LIST_NODE*	ptr_node;
	LIST_NODE*	ptr_prev;

	/* Is the source list empty ? */
	if (*ptr_src == NULL)
		return;

	/* Is the destination list empty ? */
	if (*ptr_dst == NULL)
	{
		/* Make the source now as the destination. */
		*ptr_dst = *ptr_src;
		return;
	}

	/* Both the lists are not empty. */
	ptr_node = *ptr_dst;
	ptr_prev = NULL;

	/* Reach the end of the list. */
	while (ptr_node != NULL)
	{
		ptr_prev = ptr_node;
		ptr_node = ptr_node->p_next;
	}

	/* Link the last element to the source list. */
	ptr_prev->p_next = *ptr_src;
	return;
}

/**************************************************************************
 * FUNCTION NAME : list_remove
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is called to remove the head node from the list.
 *
 * RETURNS		 :
 * 		NULL  - If there are no elements in the list.
 * 		Pointer to the head of the list 
 ***************************************************************************/
LIST_NODE* list_remove (LIST_NODE **ptr_list)
{
	LIST_NODE*	ptr_head;
	LIST_NODE*	ptr_node;

	/* Check if the list is empty ? */
	if (*ptr_list == NULL)
	{
		/* There is nothing in the list. */
		return NULL;
	}

	/* Get the head of the list. */
	ptr_head = *ptr_list;
	ptr_node = *ptr_list;

	/* Move the head to the next element in the list. */
	ptr_head = ptr_head->p_next;
	*ptr_list = ptr_head;

	/* Kill the links before returning the previous head. */
	ptr_node->p_next = NULL;
	return ptr_node;
}

/**************************************************************************
 * FUNCTION NAME : list_remove_node
 **************************************************************************
 * DESCRIPTION   :
 * 	The function is called to the specified 'node' from the list.
 ***************************************************************************/
int list_remove_node (LIST_NODE **ptr_list, LIST_NODE *ptr_remove)
{
	LIST_NODE*	ptr_node;
	LIST_NODE*	ptr_next;
	LIST_NODE*	ptr_prev;

	/* Get the head of the list. */
	ptr_node = *ptr_list;

	/* Initially there is nothing behind this node. */
	ptr_prev = NULL;
	ptr_next = ptr_node->p_next;

	/* Traverse the list till we get a hit */
	while (ptr_node != NULL)
	{
		/* Check if we got a match ? */
		if (ptr_node == ptr_remove)
			break;

		/* Update the links appropriately. */
		ptr_prev = ptr_node;
		ptr_node = ptr_node->p_next;
		ptr_next = ptr_node->p_next;
	}

	/* Was there a hit ? */
	if (ptr_node == NULL)
	{
		/* No hit. There is a problem.. How can we remove an element
		 * which does not exist in our list ? */
		return -1;
	}

	/* There was a HIT. Create the new links properly. */

	/* Border Checking -- Is this the one and only element ? */
	if ((ptr_prev == NULL) && (ptr_next == NULL))
	{
		/* This was the last element in the list. Null the list. */
		*ptr_list = NULL;
		return 0;
	}
	
	/* Is this the first element ? */
	if ((ptr_prev == NULL) && (ptr_next != NULL))
	{
		/* Kill the old links. */
		ptr_remove->p_next = NULL; 

		/* The next element is now a head of the list. */
		*ptr_list = ptr_next;

		/* Successful. */
		return 0;
	}

	/* This could be the last element or somewhere in the middle ! */

	/* Kill the old links. */
	ptr_remove->p_next = NULL; 
	ptr_prev->p_next = ptr_next;

	/* Successful. */
	return 0;
}

/**************************************************************************
 * FUNCTION NAME : list_get_head
 **************************************************************************
 * DESCRIPTION   :
 *	The function is used to get the head of the specific list
 *	
 * RETURNS		 :
 *	NULL		- If the list is empty
 *  Not NULL	- Pointer to the head of the list.
 ***************************************************************************/
LIST_NODE* list_get_head (LIST_NODE **ptr_list)
{
	return *ptr_list;
}

/**************************************************************************
 * FUNCTION NAME : list_get_next
 **************************************************************************
 * DESCRIPTION   :
 *	The function is used to traverse the specific list.
 *	
 * RETURNS		 :
 *	NULL		- If there is no next element.
 *  Not NULL	- Pointer to the next element.
 ***************************************************************************/
LIST_NODE* list_get_next (LIST_NODE *ptr_list)
{	
	return ptr_list->p_next;
}

