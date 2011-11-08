/*
 * double-linked hash list with single head implementation taken from linux
 * kernel headers as of 2.6.38-rc1.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
