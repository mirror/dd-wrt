/*
 *	BIRD Library -- Safe Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_SLISTS_H_
#define _BIRD_SLISTS_H_

/*
 *  These linked lists work in a way similar to standard lists defined
 *  in lib/lists.h, but in addition to all usual list functions they
 *  provide fast deletion/insertion/everything-safe asynchronous
 *  walking.
 *
 *  Example:
 *		slist l;
 *		siterator i;
 *		snode *n;
 *
 *	       	s_init(&i, &l);		// Initialize iteration
 *		...
 *		n = s_get(&i);		// Some time later, fetch present
 *					// value of the iterator and unlink it
 *					// from the list.
 *		while (n->next) {
 *		     ...
 *		     if (decided_to_stop) {
 *			s_put(&i, n);	// Store current position (maybe even
 *					// that we stay at list end)
 *			return;		// and return
 *		     }
 *		     ...
 *		}
 *		// After finishing, don't link the iterator back
 */

typedef struct snode {
  struct snode *next, *prev;
  struct siterator *readers;
} snode;

typedef struct slist {			/* In fact two overlayed snodes */
  struct snode *head, *null, *tail;
  struct siterator *tail_readers;
} slist;

typedef struct siterator {
  /*
   * Caution: Layout of this structure depends hard on layout of the
   *	      snode. Our `next' must be at position of snode `readers'
   *	      field, our `null' must be at position of `prev' and it must
   *	      contain NULL in order to distinguish between siterator
   *	      and snode (snodes with NULL `prev' field never carry
   *	      iterators). You are not expected to understand this.
   */
  struct siterator *prev, *null, *next;
  /*
   * For recently merged nodes this can be NULL, but then it's NULL
   * for all successors as well. This is done to speed up iterator
   * merging when there are lots of deletions.
   */
  snode *node;
} siterator;

#define SNODE (snode *)
#define SHEAD(list) ((void *)((list).head))
#define STAIL(list) ((void *)((list).tail))
#define WALK_SLIST(n,list) for(n=SHEAD(list);(SNODE (n))->next; \
				n=(void *)((SNODE (n))->next))
#define WALK_SLIST_DELSAFE(n,nxt,list) \
     for(n=SHEAD(list); nxt=(void *)((SNODE (n))->next); n=(void *) nxt)
#define EMPTY_SLIST(list) (!(list).head->next)

void s_add_tail(slist *, snode *);
void s_add_head(slist *, snode *);
void s_rem_node(snode *);
void s_add_tail_list(slist *, slist *);
void s_init_list(slist *);
void s_insert_node(snode *, snode *);

snode *s_get(siterator *);
void s_put(siterator *, snode *n);
static inline void s_init(siterator *i, slist *l) { s_put(i, SHEAD(*l)); }
static inline int s_is_used(siterator *i) { return (i->prev != NULL); }

#endif
