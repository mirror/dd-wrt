/**************************************************************************
 * FILE PURPOSE	:  	Header file for the linked list library.
 **************************************************************************
 * FILE NAME	:   listlib.h
 *
 * DESCRIPTION	:
 * 	Contains structures and exported function that are used by the linked
 * 	list library.
 *
 *	(C) Copyright 2002, Texas Instruments, Inc.
 *************************************************************************/

#ifndef __LISTLIB_H__
#define __LISTLIB_H__

/**************************************************************************
 * STRUCTURE -  LIST_NODE
 **************************************************************************
 *	The structure defines a LIST NODE structure that contains links to the 
 *	previous and next element in the list.
 **************************************************************************/
typedef struct LIST_NODE
{
	void*	p_next;		/* Pointer to the next element in the list. 	*/	
}LIST_NODE;

/************************ EXTERN Functions *********************************/

extern void list_add (LIST_NODE **ptr_list, LIST_NODE *ptr_node);
extern LIST_NODE* list_remove (LIST_NODE **ptr_list);
extern LIST_NODE* list_get_head (LIST_NODE **ptr_list);
extern LIST_NODE* list_get_next (LIST_NODE *ptr_list);
extern int list_remove_node (LIST_NODE **ptr_list, LIST_NODE *ptr_remove);
extern void list_cat (LIST_NODE **ptr_dst, LIST_NODE **ptr_src);

#endif	/* __LISTLIB_H__ */


