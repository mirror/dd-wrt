/*
 *	BIRD Library -- Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Linked lists
 *
 * The BIRD library provides a set of functions for operating on linked
 * lists. The lists are internally represented as standard doubly linked
 * lists with synthetic head and tail which makes all the basic operations
 * run in constant time and contain no extra end-of-list checks. Each list
 * is described by a &list structure, nodes can have any format as long
 * as they start with a &node structure. If you want your nodes to belong
 * to multiple lists at once, you can embed multiple &node structures in them
 * and use the SKIP_BACK() macro to calculate a pointer to the start of the
 * structure from a &node pointer, but beware of obscurity.
 *
 * There also exist safe linked lists (&slist, &snode and all functions
 * being prefixed with |s_|) which support asynchronous walking very
 * similar to that used in the &fib structure.
 */

#define _BIRD_LISTS_C_

#include "nest/bird.h"
#include "lib/lists.h"

/**
 * add_tail - append a node to a list
 * @l: linked list
 * @n: list node
 *
 * add_tail() takes a node @n and appends it at the end of the list @l.
 */
LIST_INLINE void
add_tail(list *l, node *n)
{
  node *z = l->tail;

  n->next = (node *) &l->null;
  n->prev = z;
  z->next = n;
  l->tail = n;
}

/**
 * add_head - prepend a node to a list
 * @l: linked list
 * @n: list node
 *
 * add_head() takes a node @n and prepends it at the start of the list @l.
 */
LIST_INLINE void
add_head(list *l, node *n)
{
  node *z = l->head;

  n->next = z;
  n->prev = (node *) &l->head;
  z->prev = n;
  l->head = n;
}

/**
 * insert_node - insert a node to a list
 * @n: a new list node
 * @after: a node of a list
 *
 * Inserts a node @n to a linked list after an already inserted
 * node @after.
 */
LIST_INLINE void
insert_node(node *n, node *after)
{
  node *z = after->next;

  n->next = z;
  n->prev = after;
  after->next = n;
  z->prev = n;
}

/**
 * rem_node - remove a node from a list
 * @n: node to be removed
 *
 * Removes a node @n from the list it's linked in.
 */
LIST_INLINE void
rem_node(node *n)
{
  node *z = n->prev;
  node *x = n->next;

  z->next = x;
  x->prev = z;
}

/**
 * rem2_node - remove a node from a list, with cleanup
 * @n: node to be removed
 *
 * Removes a node @n from the list it's linked in and resets its pointers to NULL.
 * Useful if you want to distinguish between linked and unlinked nodes.
 */
LIST_INLINE void
rem2_node(node *n)
{
  node *z = n->prev;
  node *x = n->next;

  z->next = x;
  x->prev = z;
  n->next = NULL;
  n->prev = NULL;
}

/**
 * replace_node - replace a node in a list with another one
 * @old: node to be removed
 * @new: node to be inserted
 *
 * Replaces node @old in the list it's linked in with node @new.  Node
 * @old may be a copy of the original node, which is not accessed
 * through the list. The function could be called with @old == @new,
 * which just fixes neighbors' pointers in the case that the node
 * was reallocated.
 */
LIST_INLINE void
replace_node(node *old, node *new)
{
  old->next->prev = new;
  old->prev->next = new;

  new->prev = old->prev;
  new->next = old->next;
}

/**
 * init_list - create an empty list
 * @l: list
 *
 * init_list() takes a &list structure and initializes its
 * fields, so that it represents an empty list.
 */
LIST_INLINE void
init_list(list *l)
{
  l->head = (node *) &l->null;
  l->null = NULL;
  l->tail = (node *) &l->head;
}

/**
 * add_tail_list - concatenate two lists
 * @to: destination list
 * @l: source list
 *
 * This function appends all elements of the list @l to
 * the list @to in constant time.
 */
LIST_INLINE void
add_tail_list(list *to, list *l)
{
  node *p = to->tail;
  node *q = l->head;

  p->next = q;
  q->prev = p;
  q = l->tail;
  q->next = (node *) &to->null;
  to->tail = q;
}
