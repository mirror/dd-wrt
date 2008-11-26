#if !defined(COMMON_LIST_H)
#define COMMON_LIST_H

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */


/*
 *
 *
 * Data Structure
 *
 *
 */
typedef struct _List_Head {
	struct _List_Head *prev, *next;
} List_Head, * PList_Head;


/*
 *
 *
 * Exposed Functions
 *
 *
 */
 
#define MV_LIST_HEAD(name) \
	List_Head name = { &(name), &(name) }

#define MV_LIST_HEAD_INIT(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static MV_INLINE void List_Add(List_Head *new_one, List_Head *head);

static MV_INLINE void List_AddTail(List_Head *new_one, List_Head *head);

static MV_INLINE void List_Del(List_Head *entry);

static MV_INLINE void List_DelInit(List_Head *entry);

static MV_INLINE void List_Move(List_Head *list, List_Head *head);

static MV_INLINE void List_MoveTail(List_Head *list,
				  List_Head *head);

static MV_INLINE int List_Empty(const List_Head *head);

/**
 * LIST_ENTRY - get the struct for this entry
 * @ptr:	the &List_Head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
//TBD
/*#define CONTAINER_OF(ptr, type, member) ({			\
*        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
*        (type *)( (char *)__mptr - OFFSET_OF(type,member) );})
*/

#define CONTAINER_OF(ptr, type, member) 			\
        ( (type *)( (char *)(ptr) - OFFSET_OF(type,member) ) )

#define LIST_ENTRY(ptr, type, member) \
	CONTAINER_OF(ptr, type, member)

/**
 * LIST_FOR_EACH	-	iterate over a list
 * @pos:	the &List_Head to use as a loop counter.
 * @head:	the head for your list.
 */
#define LIST_FOR_EACH(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * LIST_FOR_EACH_PREV	-	iterate over a list backwards
 * @pos:	the &List_Head to use as a loop counter.
 * @head:	the head for your list.
 */
#define LIST_FOR_EACH_PREV(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * LIST_FOR_EACH_ENTRY	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define LIST_FOR_EACH_ENTRY(pos, head, member)				\
	for (pos = LIST_ENTRY((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = LIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * LIST_FOR_EACH_ENTRY_PREV - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define LIST_FOR_EACH_ENTRY_PREV(pos, head, member)			\
	for (pos = LIST_ENTRY((head)->prev, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = LIST_ENTRY(pos->member.prev, typeof(*pos), member))

#ifndef _OS_BIOS
#include "com_list.c"
#endif

#define List_GetFirstEntry(head, type, member)	\
	LIST_ENTRY(List_GetFirst(head), type, member)

#define List_GetLastEntry(head, type, member)	\
	LIST_ENTRY(List_GetLast(head), type, member)

#endif /* COMMON_LIST_H */

