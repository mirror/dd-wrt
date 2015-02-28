/*
 *	BIRD Library -- Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_LISTS_H_
#define _BIRD_LISTS_H_

/*
 * I admit the list structure is very tricky and also somewhat awkward,
 * but it's both efficient and easy to manipulate once one understands the
 * basic trick: The list head always contains two synthetic nodes which are
 * always present in the list: the head and the tail. But as the `next'
 * entry of the tail and the `prev' entry of the head are both NULL, the
 * nodes can overlap each other:
 *
 *     head    head_node.next
 *     null    head_node.prev  tail_node.next
 *     tail                    tail_node.prev
 */

typedef struct node {
  struct node *next, *prev;
} node;

typedef struct list {			/* In fact two overlayed nodes */
  struct node *head, *null, *tail;
} list;

#define NODE (node *)
#define HEAD(list) ((void *)((list).head))
#define TAIL(list) ((void *)((list).tail))
#define NODE_NEXT(n) ((void *)((NODE (n))->next))
#define NODE_VALID(n) ((NODE (n))->next)
#define WALK_LIST(n,list) for(n=HEAD(list); NODE_VALID(n); n=NODE_NEXT(n))
#define WALK_LIST2(n,nn,list,pos) \
  for(nn=(list).head; NODE_VALID(nn) && (n=SKIP_BACK(typeof(*n),pos,nn)); nn=nn->next)
#define WALK_LIST_DELSAFE(n,nxt,list) \
     for(n=HEAD(list); nxt=NODE_NEXT(n); n=(void *) nxt)
/* WALK_LIST_FIRST supposes that called code removes each processed node */
#define WALK_LIST_FIRST(n,list) \
     while(n=HEAD(list), (NODE (n))->next)
#define WALK_LIST_BACKWARDS(n,list) for(n=TAIL(list);(NODE (n))->prev; \
				n=(void *)((NODE (n))->prev))
#define WALK_LIST_BACKWARDS_DELSAFE(n,prv,list) \
     for(n=TAIL(list); prv=(void *)((NODE (n))->prev); n=(void *) prv)

#define EMPTY_LIST(list) (!(list).head->next)

void add_tail(list *, node *);
void add_head(list *, node *);
void rem_node(node *);
void rem2_node(node *);
void add_tail_list(list *, list *);
void init_list(list *);
void insert_node(node *, node *);

#ifndef _BIRD_LISTS_C_
#define LIST_INLINE extern inline
#include "lib/lists.c"
#undef LIST_INLINE
#else
#define LIST_INLINE
#endif

#endif
