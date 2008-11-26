#include "mv_include.h"

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static MV_INLINE void __List_Add(List_Head *new_one,
			      List_Head *prev,
			      List_Head *next)
{
	next->prev = new_one;
	new_one->next = next;
	new_one->prev = prev;
	prev->next = new_one;
}

/**
 * List_Add - add a new entry
 * @new_one: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static MV_INLINE void List_Add(List_Head *new_one, List_Head *head)
{
	__List_Add(new_one, head, head->next);
}

/**
 * List_AddTail - add a new entry
 * @new_one: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static MV_INLINE void List_AddTail(List_Head *new_one, List_Head *head)
{
	__List_Add(new_one, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static MV_INLINE void __List_Del(List_Head * prev, List_Head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * List_Del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: List_Empty on entry does not return true after this, the entry is
 * in an undefined state.
 */
static MV_INLINE void List_Del(List_Head *entry)
{
	__List_Del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

/**
 * List_DelInit - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static MV_INLINE void List_DelInit(List_Head *entry)
{
	__List_Del(entry->prev, entry->next);
	MV_LIST_HEAD_INIT(entry);
}

/**
 * List_Move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static MV_INLINE void List_Move(List_Head *list, List_Head *head)
{
        __List_Del(list->prev, list->next);
        List_Add(list, head);
}

/**
 * List_MoveTail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static MV_INLINE void List_MoveTail(List_Head *list,
				  List_Head *head)
{
        __List_Del(list->prev, list->next);
        List_AddTail(list, head);
}

/**
 * List_Empty - tests whether a list is empty
 * @head: the list to test.
 */
static MV_INLINE int List_Empty(const List_Head *head)
{
	return head->next == head;
}

static MV_INLINE int List_GetCount(const List_Head *head)
{
	int i=0;
	List_Head *pos;
	LIST_FOR_EACH(pos, head) {
		i++;
	}
	return i;
}

static MV_INLINE List_Head* List_GetFirst(List_Head *head)
{
	List_Head * one = NULL;
	if ( List_Empty(head) ) return NULL;

	one = head->next;
	List_Del(one);
	return one;
}

static MV_INLINE List_Head* List_GetLast(List_Head *head)
{
	List_Head * one = NULL;
	if ( List_Empty(head) ) return NULL;

	one = head->prev;
	List_Del(one);
	return one;
}

static MV_INLINE void __List_Splice(List_Head *list,
				 List_Head *head)
{
	List_Head *first = list->next;
	List_Head *last = list->prev;
	List_Head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
 * List_Splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static MV_INLINE void List_Splice(List_Head *list, List_Head *head)
{
	if (!List_Empty(list))
		__List_Splice(list, head);
}

/**
 * List_Splice_Init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static MV_INLINE void List_Splice_Init(List_Head *list,
				    List_Head *head)
{
	if (!List_Empty(list)) {
		__List_Splice(list, head);
		MV_LIST_HEAD_INIT(list);
	}
}

