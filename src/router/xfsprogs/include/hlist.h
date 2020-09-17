// SPDX-License-Identifier: GPL-2.0
/*
 * double-linked hash list with single head implementation taken from linux
 * kernel headers as of 2.6.38-rc1.
 */
#ifndef __HLIST_H__
#define __HLIST_H__

struct hlist_node {
	struct hlist_node *next;
	struct hlist_node **pprev;
};
struct hlist_head {
	struct hlist_node *first;
};

#define HLIST_HEAD_INIT { .first = NULL }
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
        struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
}

#define hlist_entry(ptr, type, member)  ({ \
		const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
		(type *)( (char *)__mptr - offsetof(type,member) );})


#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_entry(tpos, pos, head, member)                    \
        for (pos = (head)->first;                                        \
	     pos && ({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)


#endif	/* __LIST_H__ */
