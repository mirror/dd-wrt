/* GPL v2, lend from the linux kernel */
#ifndef _DD_LINUX_LIST_H
#define _DD_LINUX_LIST_H

#include <stddef.h>

struct dd_list_head {
	struct dd_list_head *next, *prev;
};

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#ifndef container_of
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))
#endif

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define DD_LIST_HEAD_INIT(name)  \
	{                        \
		&(name), &(name) \
	}

#define DD_LIST_HEAD(name) struct dd_list_head name = DD_LIST_HEAD_INIT(name)

void INIT_DD_LIST_HEAD(struct dd_list_head *list);

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __dd_list_add(struct dd_list_head *new, struct dd_list_head *prev, struct dd_list_head *next);

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
#define dd_list_add(new, head) __dd_list_add(new, head, head->next)

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
#define dd_list_add_tail(new, head) __dd_list_add(new, head->prev, head)

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __dd_list_del(struct dd_list_head *prev, struct dd_list_head *next);

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
void dd_list_del(struct dd_list_head *entry);

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
void dd_list_replace(struct dd_list_head *old, struct dd_list_head *new);

void dd_list_replace_init(struct dd_list_head *old, struct dd_list_head *new);

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
void dd_list_del_init(struct dd_list_head *entry);

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
void dd_list_move(struct dd_list_head *list, struct dd_list_head *head);

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
void dd_list_move_tail(struct dd_list_head *list, struct dd_list_head *head);

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */

#define dd_list_is_last(list, head) list->next == head

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
#define dd_list_empty(head) (head->next == head)

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
int dd_list_empty_careful(const struct dd_list_head *head);

void __dd_list_splice(struct dd_list_head *list, struct dd_list_head *head);
/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
void dd_list_splice(struct dd_list_head *list, struct dd_list_head *head);

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
void dd_list_splice_init(struct dd_list_head *list, struct dd_list_head *head);

/**
 * dd_list_entry - get the struct for this entry
 * @ptr:	the &struct dd_list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define dd_list_entry(ptr, type, member) container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define dd_list_first_entry(ptr, type, member) dd_list_entry((ptr)->next, type, member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct dd_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define dd_list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * __list_for_each	-	iterate over a list
 * @pos:	the &struct dd_list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __dd_list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct dd_list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define dd_list_for_each_prev(pos, head) for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct dd_list_head to use as a loop cursor.
 * @n:		another &struct dd_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define dd_list_for_each_safe(pos, n, head) for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct dd_list_head to use as a loop cursor.
 * @n:		another &struct dd_list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define dd_list_for_each_prev_safe(pos, n, head) for (pos = (head)->prev, n = pos->prev; pos != (head); pos = n, n = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define dd_list_for_each_entry(pos, head, member)                                             \
	for (pos = dd_list_entry((head)->next, typeof(*pos), member); &pos->member != (head); \
	     pos = dd_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define dd_list_for_each_entry_reverse(pos, head, member)                                     \
	for (pos = dd_list_entry((head)->prev, typeof(*pos), member); &pos->member != (head); \
	     pos = dd_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define dd_list_prepare_entry(pos, head, member) ((pos) ?: dd_list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define dd_list_for_each_entry_continue(pos, head, member)                                        \
	for (pos = dd_list_entry(pos->member.next, typeof(*pos), member); &pos->member != (head); \
	     pos = dd_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define dd_list_for_each_entry_continue_reverse(pos, head, member)                                \
	for (pos = dd_list_entry(pos->member.prev, typeof(*pos), member); &pos->member != (head); \
	     pos = dd_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define dd_list_for_each_entry_from(pos, head, member) \
	for (; &pos->member != (head); pos = dd_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define dd_list_for_each_entry_safe(pos, n, head, member)                                                                        \
	for (pos = dd_list_entry((head)->next, typeof(*pos), member), n = dd_list_entry(pos->member.next, typeof(*pos), member); \
	     &pos->member != (head); pos = n, n = dd_list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define dd_list_for_each_entry_safe_continue(pos, n, head, member)        \
	for (pos = dd_list_entry(pos->member.next, typeof(*pos), member), \
	    n = dd_list_entry(pos->member.next, typeof(*pos), member);    \
	     &pos->member != (head); pos = n, n = dd_list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_from
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define dd_list_for_each_entry_safe_from(pos, n, head, member)                                  \
	for (n = dd_list_entry(pos->member.next, typeof(*pos), member); &pos->member != (head); \
	     pos = n, n = dd_list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define dd_list_for_each_entry_safe_reverse(pos, n, head, member)                                                                \
	for (pos = dd_list_entry((head)->prev, typeof(*pos), member), n = dd_list_entry(pos->member.prev, typeof(*pos), member); \
	     &pos->member != (head); pos = n, n = dd_list_entry(n->member.prev, typeof(*n), member))

#endif
