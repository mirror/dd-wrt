#include <dd_list.h>

void INIT_DD_LIST_HEAD(struct dd_list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __dd_list_add(struct dd_list_head *new, struct dd_list_head *prev,
		   struct dd_list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __dd_list_del(struct dd_list_head *prev, struct dd_list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
void dd_list_del(struct dd_list_head *entry)
{
	__dd_list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
void dd_list_replace(struct dd_list_head *old, struct dd_list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

void dd_list_replace_init(struct dd_list_head *old, struct dd_list_head *new)
{
	dd_list_replace(old, new);
	INIT_DD_LIST_HEAD(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
void dd_list_del_init(struct dd_list_head *entry)
{
	__dd_list_del(entry->prev, entry->next);
	INIT_DD_LIST_HEAD(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
void dd_list_move(struct dd_list_head *list, struct dd_list_head *head)
{
	__dd_list_del(list->prev, list->next);
	dd_list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
void dd_list_move_tail(struct dd_list_head *list, struct dd_list_head *head)
{
	__dd_list_del(list->prev, list->next);
	dd_list_add_tail(list, head);
}

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
int dd_list_empty_careful(const struct dd_list_head *head)
{
	struct dd_list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

void __dd_list_splice(struct dd_list_head *list, struct dd_list_head *head)
{
	struct dd_list_head *first = list->next;
	struct dd_list_head *last = list->prev;
	struct dd_list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
void dd_list_splice(struct dd_list_head *list, struct dd_list_head *head)
{
	if (!dd_list_empty(list))
		__dd_list_splice(list, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
void dd_list_splice_init(struct dd_list_head *list, struct dd_list_head *head)
{
	if (!dd_list_empty(list)) {
		__dd_list_splice(list, head);
		INIT_DD_LIST_HEAD(list);
	}
}
