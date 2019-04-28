/*
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; version
 *   2 of the License.
 *
 *   This is based on the fib_trie code (which was based on fib_hash),
 *   but all the trie code removed and replaced with a simple
 *   list. In addition it uses kmalloc instead of slab directly
 *   to save some dynamic memory.
 *
 *   This is for systems with very small routing tables, like
 *   your typical single-homed client. You shouldn't be using this for
 *   more than a handful of routes. The main motivation is smaller
 *   code size. Apart from being less scalable and some missing
 *   proc output the functionality should be the same as fib_trie.
 *
 *   Some code is very similar with fib_trie and could be later
 *   factored out into a shared file.
 *
 *   List code by Andi Kleen, original trie file was:
 *
 *   Robert Olsson <robert.olsson@its.uu.se> Uppsala Universitet
 *     & Swedish University of Agricultural Sciences.
 *
 *   Jens Laas <jens.laas@data.slu.se> Swedish University of
 *     Agricultural Sciences.
 *
 *   Hans Liss <hans.liss@its.uu.se>  Uppsala Universitet
 *
 * Code from fib_hash has been reused which includes the following header:
 *
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		IPv4 FIB: lookup engine and maintenance routines.
 *
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Substantial contributions to this work comes from:
 *
 *		David S. Miller, <davem@davemloft.net>
 *		Stephen Hemminger <shemminger@osdl.org>
 *		Paul E. McKenney <paulmck@us.ibm.com>
 *		Patrick McHardy <kaber@trash.net>
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/proc_fs.h>
#include <linux/rcupdate.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/net_namespace.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/sock.h>
#include <net/ip_fib.h>
#include "fib_lookup.h"

/* One entry with the same key/mask */
struct entry {
	struct hlist_node nd;
	u32 key;
	u32 mask;
	int plen;
	struct hlist_head list;
	struct rcu_head rcu;
};

struct entry_info {
	struct hlist_node hlist;
	int plen;
	u32 mask_plen; /* ntohl(inet_make_mask(plen)) */
	struct hlist_head falh;
	struct rcu_head rcu;
};

/* The list is ordered by the prefix length. Larger prefixes come
 * earlier (so the default routes should be always at the end)
 * Protected by the RTNL for writing, reading is handled with standard
 * list RCU.
 */
struct rlist {
	struct hlist_head	list;
};

static void __alias_free_mem(struct rcu_head *head)
{
	struct fib_alias *fa = container_of(head, struct fib_alias, rcu);
	kfree(fa);
}

static inline void alias_free_mem_rcu(struct fib_alias *fa)
{
	call_rcu(&fa->rcu, __alias_free_mem);
}

static void __entry_free_rcu(struct rcu_head *head)
{
	struct entry *l = container_of(head, struct entry, rcu);
	kfree(l);
}

static inline void free_entry(struct entry *entry)
{
	call_rcu(&entry->rcu, __entry_free_rcu);
}

static inline void free_entry_info(struct entry_info *entry)
{
	kfree_rcu(entry, rcu);
}

static struct entry *entry_new(void)
{
	return kzalloc(sizeof(struct entry), GFP_KERNEL);
}

static struct entry_info *entry_info_new(int plen)
{
	struct entry_info *ei = kzalloc(sizeof(struct entry_info),  GFP_KERNEL);
	if (ei) {
		ei->plen = plen;
		ei->mask_plen = ntohl(inet_make_mask(plen));
		INIT_HLIST_HEAD(&ei->falh);
	}
	return ei;
}

/* readside must use rcu_read_lock currently dump routines
 via get_fa_head and dump */

static struct entry_info *find_entry_info(struct entry *e, int plen)
{
	struct hlist_head *head = &e->list;
	struct entry_info *ei;

	hlist_for_each_entry_rcu(ei, head, hlist)
		if (ei->plen == plen)
			return ei;

	return NULL;
}

static inline struct hlist_head *get_fa_head(struct entry *l, int plen)
{
	struct entry_info *li = find_entry_info(l, plen);

	if (!li)
		return NULL;

	return &li->falh;
}

static void insert_entry_info(struct hlist_head *head, struct entry_info *new)
{
	struct entry_info *li = NULL, *last = NULL;

	if (hlist_empty(head)) {
		hlist_add_head_rcu(&new->hlist, head);
	} else {
		hlist_for_each_entry(li, head, hlist) {
			if (new->plen > li->plen)
				break;

			last = li;
		}
		if (last)
			hlist_add_behind_rcu(&last->hlist, &new->hlist);
		else
			hlist_add_before_rcu(&new->hlist, &li->hlist);
	}
}

/* rcu_read_lock needs to be hold by caller from readside */
static struct entry *
fib_find_node(struct rlist *rl, u32 key)
{
	struct entry *entry;

	hlist_for_each_entry_rcu (entry, &rl->list, nd) {
		if (key == entry->key)
			return entry;
	}
	return NULL;
}

/* only used from updater-side */

static struct hlist_head *fib_insert_node(struct rlist *rl, u32 key, int plen)
{
	struct entry *entry;
	struct entry_info *ei;
	struct hlist_node *prev = NULL;
	struct hlist_head *fa_head;

	hlist_for_each_entry (entry, &rl->list, nd) {
		if (entry->plen < plen)
			break;
		prev = &entry->nd;
	}
	entry = entry_new();
	if (!entry)
		return NULL;
	entry->key = key;
	entry->mask = ntohl(inet_make_mask(plen));
	entry->plen = plen;
	if (prev)
		hlist_add_behind_rcu(prev, &entry->nd);
	else
		hlist_add_head_rcu(&entry->nd, &rl->list);
	ei = entry_info_new(plen);
	if (!ei)
		return NULL;
	fa_head = &ei->falh;
	insert_entry_info(&entry->list, ei);
	return fa_head;
}

/* Return the first fib alias matching TOS with
 * priority less than or equal to PRIO.
 */
static struct fib_alias *fib_find_alias(struct hlist_head *fah, u8 tos, u32 prio)
{
	if (fah) {
		struct fib_alias *fa;
		hlist_for_each_entry(fa, fah, fa_list) {
			if (fa->fa_tos > tos)
				continue;
			if (fa->fa_info->fib_priority >= prio ||
			    fa->fa_tos < tos)
				return fa;
		}
	}
	return NULL;
}

int fib_table_insert(struct fib_table *tb, struct fib_config *cfg)
{
	struct rlist *rl = (struct rlist *) tb->tb_data;
	struct fib_alias *fa, *new_fa;
	struct hlist_head *fa_head = NULL;
	struct fib_info *fi;
	int plen = cfg->fc_dst_len;
	u8 tos = cfg->fc_tos;
	u32 key, mask;
	int err;
	struct entry *l;
	bool found = false;
	struct hlist_node **pprev;

	if (plen > 32)
		return -EINVAL;

	key = ntohl(cfg->fc_dst);

	pr_debug("Insert table=%u %08x/%d\n", tb->tb_id, key, plen);

	mask = ntohl(inet_make_mask(plen));

	if (key & ~mask)
		return -EINVAL;

	fi = fib_create_info(cfg);
	if (IS_ERR(fi)) {
		err = PTR_ERR(fi);
		goto err;
	}

	hlist_for_each_entry (l, &rl->list, nd) {
		if (l->key == key) {
			found = true;
			break;
		}
	}
	fa = NULL;
	if (!found)
		l = NULL;

	if (l) {
		fa_head = get_fa_head(l, plen);
		fa = fib_find_alias(fa_head, tos, fi->fib_priority);
	}

	/* Now fa, if non-NULL, points to the first fib alias
	 * with the same keys [prefix,tos,priority], if such key already
	 * exists or to the node before which we will insert new one.
	 *
	 * If fa is NULL, we will need to allocate a new one and
	 * insert to the head of f.
	 *
	 * If f is NULL, no fib node matched the destination key
	 * and we need to allocate a new one of those as well.
	 */

	if (fa && fa->fa_tos == tos &&
	    fa->fa_info->fib_priority == fi->fib_priority) {
		struct fib_alias *fa_first, *fa_match;
		int iter = 0;

		err = -EEXIST;
		if (cfg->fc_nlflags & NLM_F_EXCL)
			goto out;

		/* We have 2 goals:
		 * 1. Find exact match for type, scope, fib_info to avoid
		 * duplicate routes
		 * 2. Find next 'fa' (or head), NLM_F_APPEND inserts before it
		 */
		fa_match = NULL;
		fa_first = fa;
		pprev = fa->fa_list.pprev;
		fa = hlist_entry(pprev, typeof(*fa), fa_list.next);
		hlist_for_each_entry_continue(fa, fa_list) {
			if (fa->fa_tos != tos)
				break;
			if (fa->fa_info->fib_priority != fi->fib_priority)
				break;
			if (fa->fa_type == cfg->fc_type &&
			    fa->fa_info == fi) {
				fa_match = fa;
				break;
			}
			iter++;
		}
		if (cfg->fc_nlflags & NLM_F_REPLACE) {
			struct fib_info *fi_drop;
			u8 state;

			fa = fa_first;
			if (fa_match) {
				if (fa == fa_match)
					err = 0;
				goto out;
			}
			err = -ENOBUFS;
			new_fa = kmalloc(sizeof(struct fib_alias), GFP_KERNEL);
			if (new_fa == NULL)
				goto out;

			fi_drop = fa->fa_info;
			new_fa->fa_tos = fa->fa_tos;
			new_fa->fa_info = fi;
			new_fa->fa_type = cfg->fc_type;
			state = fa->fa_state;
			new_fa->fa_state = state & ~FA_S_ACCESSED;

			hlist_replace_rcu(&fa->fa_list, &new_fa->fa_list);
			alias_free_mem_rcu(fa);

			fib_release_info(fi_drop);
			if (state & FA_S_ACCESSED)
				rt_cache_flush(cfg->fc_nlinfo.nl_net);
			rtmsg_fib(RTM_NEWROUTE, htonl(key), new_fa, plen,
				tb->tb_id, &cfg->fc_nlinfo, NLM_F_REPLACE);

			goto succeeded;
		}
		/* Error if we find a perfect match which
		 * uses the same scope, type, and nexthop
		 * information.
		 */
		if (fa_match) {
			goto out;
		}

		if (!(cfg->fc_nlflags & NLM_F_APPEND))
			fa = fa_first;
	}
	err = -ENOENT;
	if (!(cfg->fc_nlflags & NLM_F_CREATE))
		goto out;

	err = -ENOBUFS;
	new_fa = kmalloc(sizeof(struct fib_alias), GFP_KERNEL);
	if (new_fa == NULL)
		goto out;

	new_fa->fa_info = fi;
	new_fa->fa_tos = tos;
	new_fa->fa_type = cfg->fc_type;
	new_fa->fa_state = 0;

	/*
	 * Insert new entry to the list.
	 */

	if (!fa_head) {
		fa_head = fib_insert_node(rl, key, plen);
		if (unlikely(!fa_head)) {
			err = -ENOMEM;
			goto out_free_new_fa;
		}
	}
	if (!plen)
		tb->tb_num_default++;

	hlist_add_tail_rcu(&new_fa->fa_list,
			  (fa ? &fa->fa_list : fa_head));

	rt_cache_flush(cfg->fc_nlinfo.nl_net);
	rtmsg_fib(RTM_NEWROUTE, htonl(key), new_fa, plen, tb->tb_id,
		  &cfg->fc_nlinfo, 0);
succeeded:
	return 0;

out_free_new_fa:
	kfree(new_fa);
out:
	fib_release_info(fi);
err:
	pr_debug("insert err %d\n", err);
	return err;
}

static int check_entry(struct fib_table *tb, struct entry *l,
		       u32 key,  const struct flowi4 *flp,
		       struct fib_result *res, int fib_flags)
{
	struct entry_info *li;
	struct hlist_head *hhead = &l->list;

	hlist_for_each_entry_rcu(li, hhead, hlist) {
		struct fib_alias *fa;

		list_for_each_entry_rcu(fa, &li->falh, fa_list) {
			struct fib_info *fi = fa->fa_info;
			int nhsel, err;

			if (fa->fa_tos && fa->fa_tos != flp->flowi4_tos)
				continue;
			if (fi->fib_dead)
				continue;
			if (fa->fa_info->fib_scope < flp->flowi4_scope)
				continue;
			fib_alias_accessed(fa);
			err = fib_props[fa->fa_type].error;
			if (err) {
				return err;
			}
			if (fi->fib_flags & RTNH_F_DEAD)
				continue;
			for (nhsel = 0; nhsel < fi->fib_nhs; nhsel++) {
				const struct fib_nh *nh = &fi->fib_nh[nhsel];

				if (nh->nh_flags & RTNH_F_DEAD)
					continue;
				if (flp->flowi4_oif && flp->flowi4_oif != nh->nh_oif)
					continue;

				res->prefixlen = li->plen;
				res->nh_sel = nhsel;
				res->type = fa->fa_type;
				res->scope = fa->fa_info->fib_scope;
				res->fi = fi;
				res->table = tb;
				res->fa_head = &li->falh;
				if (!(fib_flags & FIB_LOOKUP_NOREF))
					atomic_inc(&fi->fib_clntref);
				return 0;
			}
		}
	}

	return 1;
}

int fib_table_lookup(struct fib_table *tb, const struct flowi4 *flp,
		     struct fib_result *res, int fib_flags)
{
	struct entry *entry;
	struct rlist *rl = (struct rlist *)tb->tb_data;
	u32 key = ntohl(flp->daddr);
	int ret = 1;

	rcu_read_lock();
	hlist_for_each_entry_rcu (entry, &rl->list, nd) {
		if ((key & entry->mask) == entry->key) {
			ret = check_entry(tb, entry, key, flp, res, fib_flags);
			if (ret <= 0)
				break;
		}
	}
	rcu_read_unlock();
	return ret;
}
EXPORT_SYMBOL_GPL(fib_table_lookup);

static void entry_remove(struct entry *entry)
{
	hlist_del_rcu(&entry->nd);
	free_entry(entry);
}

/*
 * Caller must hold RTNL.
 */
int fib_table_delete(struct fib_table *tb, struct fib_config *cfg)
{
	struct rlist *rl = (struct rlist *) tb->tb_data;
	u32 key, mask;
	int plen = cfg->fc_dst_len;
	u8 tos = cfg->fc_tos;
	struct fib_alias *fa, *fa_to_delete;
	struct hlist_head *fa_head;
	struct entry *l;
	struct entry_info *li;

	if (plen > 32)
		return -EINVAL;

	key = ntohl(cfg->fc_dst);
	mask = ntohl(inet_make_mask(plen));

	if (key & ~mask)
		return -EINVAL;

	l = fib_find_node(rl, key);

	if (!l)
		return -ESRCH;

	li = find_entry_info(l, plen);

	if (!li)
		return -ESRCH;

	fa_head = &li->falh;
	fa = fib_find_alias(fa_head, tos, 0);

	if (!fa)
		return -ESRCH;

	pr_debug("Deleting %08x/%d tos=%d\n", key, plen, tos);

	fa_to_delete = NULL;
	fa = hlist_entry(fa->fa_list.pprev, struct fib_alias, fa_list);
	hlist_for_each_entry_continue(fa, fa_list) {
		struct fib_info *fi = fa->fa_info;

		if (fa->fa_tos != tos)
			break;

		if ((!cfg->fc_type || fa->fa_type == cfg->fc_type) &&
		    (cfg->fc_scope == RT_SCOPE_NOWHERE ||
		     fa->fa_info->fib_scope == cfg->fc_scope) &&
		    (!cfg->fc_prefsrc ||
		     fi->fib_prefsrc == cfg->fc_prefsrc) &&
		    (!cfg->fc_protocol ||
		     fi->fib_protocol == cfg->fc_protocol) &&
		    fib_nh_match(cfg, fi) == 0) {
			fa_to_delete = fa;
			break;
		}
	}

	if (!fa_to_delete)
		return -ESRCH;

	fa = fa_to_delete;
	rtmsg_fib(RTM_DELROUTE, htonl(key), fa, plen, tb->tb_id,
		  &cfg->fc_nlinfo, 0);

	list_del_rcu(&fa->fa_list);

	if (!plen)
		tb->tb_num_default--;

	if (list_empty(fa_head)) {
		hlist_del_rcu(&li->hlist);
		free_entry_info(li);
	}

	if (hlist_empty(&l->list))
		entry_remove(l);

	if (fa->fa_state & FA_S_ACCESSED)
		rt_cache_flush(cfg->fc_nlinfo.nl_net);

	fib_release_info(fa->fa_info);
	alias_free_mem_rcu(fa);
	return 0;
}

static int flush_list(struct list_head *head)
{
	struct fib_alias *fa, *fa_node;
	int found = 0;

	list_for_each_entry_safe(fa, fa_node, head, fa_list) {
		struct fib_info *fi = fa->fa_info;

		if (fi && (fi->fib_flags & RTNH_F_DEAD)) {
			list_del_rcu(&fa->fa_list);
			fib_release_info(fa->fa_info);
			alias_free_mem_rcu(fa);
			found++;
		}
	}
	return found;
}

static int flush_entry(struct entry *e)
{
	int found = 0;
	struct hlist_head *lih = &e->list;
	struct hlist_node *tmp;
	struct entry_info *li = NULL;

	hlist_for_each_entry_safe(li, tmp, lih, hlist) {
		found += flush_list(&li->falh);

		if (list_empty(&li->falh)) {
			hlist_del_rcu(&li->hlist);
			free_entry_info(li);
		}
	}
	return found;
}

static struct entry *first_entry(struct rlist *rl)
{
	struct hlist_node *first;

	first = rcu_dereference(hlist_first_rcu(&rl->list));
	return first ? hlist_entry(first, struct entry, nd) : NULL;
}

static struct entry *next_entry(struct entry *e)
{
	struct hlist_node *next = hlist_next_rcu(&e->nd);

	return next ? hlist_entry(next, struct entry, nd) : NULL;
}

static struct entry *entry_index(struct rlist *rl, int index)
{
	struct entry *e;

	hlist_for_each_entry_rcu (e, &rl->list, nd) {
		if (index-- <= 0)
			break;
	}
	return e;
}

/*
 * Caller must hold RTNL.
 */
int fib_table_flush(struct fib_table *tb)
{
	struct rlist *rl = (struct rlist *) tb->tb_data;
	struct entry *ei;
	struct hlist_node *tmp;
	int found = 0;

	hlist_for_each_entry_safe (ei, tmp, &rl->list, nd) {
		found += flush_entry(ei);
		if (hlist_empty(&ei->list))
			entry_remove(ei);
	}
	pr_debug("flush found=%d\n", found);
	return found;
}

void fib_free_table(struct fib_table *tb)
{
	kfree(tb);
}

static int fn_list_dump_fa(u32 key, int plen, struct list_head *fah,
			   struct fib_table *tb,
			   struct sk_buff *skb, struct netlink_callback *cb)
{
	int i, s_i;
	struct fib_alias *fa;
	__be32 xkey = htonl(key);

	s_i = cb->args[5];
	i = 0;

	/* rcu_read_lock is hold by caller */

	list_for_each_entry_rcu(fa, fah, fa_list) {
		if (i < s_i) {
			i++;
			continue;
		}

		if (fib_dump_info(skb, NETLINK_CB(cb->skb).portid,
				  cb->nlh->nlmsg_seq,
				  RTM_NEWROUTE,
				  tb->tb_id,
				  fa->fa_type,
				  xkey,
				  plen,
				  fa->fa_tos,
				  fa->fa_info, NLM_F_MULTI) < 0) {
			cb->args[5] = i;
			return -1;
		}
		i++;
	}
	cb->args[5] = i;
	return skb->len;
}

static int fn_dump_entry(struct entry *l, struct fib_table *tb,
			struct sk_buff *skb, struct netlink_callback *cb)
{
	struct entry_info *li;
	int i, s_i;

	s_i = cb->args[4];
	i = 0;

	/* rcu_read_lock is hold by caller */
	hlist_for_each_entry_rcu(li, &l->list, hlist) {
		if (i < s_i) {
			i++;
			continue;
		}

		if (i > s_i)
			cb->args[5] = 0;

		if (list_empty(&li->falh))
			continue;

		if (fn_list_dump_fa(l->key, li->plen, &li->falh, tb, skb, cb) < 0) {
			cb->args[4] = i;
			return -1;
		}
		i++;
	}

	cb->args[4] = i;
	return skb->len;
}

int fib_table_dump(struct fib_table *tb, struct sk_buff *skb,
		   struct netlink_callback *cb)
{
	struct entry *l;
	struct rlist *rl = (struct rlist *) tb->tb_data;
	u32 key = cb->args[2];
	int count = cb->args[3];

	rcu_read_lock();
	if (count == 0)
		l = first_entry(rl);
	else {
		/* Normally, continue from last key, but if that is missing
		 * fallback to using slow rescan
		 */
		l = fib_find_node(rl, key);
		if (!l)
			l = entry_index(rl, count);
	}

	while (l) {
		cb->args[2] = l->key;
		if (fn_dump_entry(l, tb, skb, cb) < 0) {
			cb->args[3] = count;
			rcu_read_unlock();
			return -1;
		}

		++count;
		l = next_entry(l);
		memset(&cb->args[4], 0,
		       sizeof(cb->args) - 4*sizeof(cb->args[0]));
	}
	cb->args[3] = count;
	rcu_read_unlock();

	return skb->len;
}

/* new name? */
struct fib_table *fib_trie_table(u32 id)
{
	struct fib_table *tb;

	tb = kzalloc(sizeof(struct fib_table) + sizeof(struct rtable),
		     GFP_KERNEL);
	if (tb == NULL)
		return NULL;

	tb->tb_id = id;
	tb->tb_default = -1;
	tb->tb_num_default = 0;

	return tb;
}

#ifdef CONFIG_PROC_FS

struct fib_route_iter {
	struct seq_net_private p;
	struct rlist *rl;
	loff_t	pos;
	u32	key;
};

static struct entry *fib_route_get_idx(struct fib_route_iter *iter, loff_t pos)
{
	struct entry *e = NULL;

	/* use cache location of last found key */
	if (iter->pos > 0 && pos >= iter->pos && (e = fib_find_node(iter->rl, iter->key)) != NULL)
		pos -= iter->pos;
	else {
		iter->pos = 0;
		e = first_entry(iter->rl);
	}

	while (e && pos-- > 0) {
		iter->pos++;
		e = next_entry(e);
	}

	if (e)
		iter->key = pos;	/* remember it */
	else
		iter->pos = 0;		/* forget it */

	return e;
}

static void *fib_route_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(RCU)
{
	struct fib_route_iter *iter = seq->private;
	struct fib_table *tb;

	rcu_read_lock();
	tb = fib_get_table(seq_file_net(seq), RT_TABLE_MAIN);
	if (!tb)
		return NULL;

	iter->rl = (struct rlist *) tb->tb_data;
	if (*pos == 0)
		return SEQ_START_TOKEN;
	else
		return fib_route_get_idx(iter, *pos - 1);
}

static void *fib_route_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct fib_route_iter *iter = seq->private;
	struct entry *e = v;

	++*pos;
	if (v == SEQ_START_TOKEN) {
		iter->pos = 0;
		e = first_entry(iter->rl);
	} else {
		iter->pos++;
		e = next_entry(e);
	}

	if (e)
		iter->key = e->key;
	else
		iter->pos = 0;
	return e;
}

static void fib_route_seq_stop(struct seq_file *seq, void *v)
	__releases(RCU)
{
	rcu_read_unlock();
}

static unsigned int fib_flag_trans(int type, __be32 mask, const struct fib_info *fi)
{
	unsigned int flags = 0;

	if (type == RTN_UNREACHABLE || type == RTN_PROHIBIT)
		flags = RTF_REJECT;
	if (fi && fi->fib_nh->nh_gw)
		flags |= RTF_GATEWAY;
	if (mask == htonl(0xFFFFFFFF))
		flags |= RTF_HOST;
	flags |= RTF_UP;
	return flags;
}

/* This outputs /proc/net/route.
 * The format of the file is not supposed to be changed
 * and needs to be same as fib_hash output to avoid breaking
 * legacy utilities.
 */
static int fib_route_seq_show(struct seq_file *seq, void *v)
{
	struct entry *entry = v;
	struct entry_info *ei;

	if (v == SEQ_START_TOKEN) {
		seq_printf(seq, "%-127s\n", "Iface\tDestination\tGateway "
			   "\tFlags\tRefCnt\tUse\tMetric\tMask\t\tMTU"
			   "\tWindow\tIRTT");
		return 0;
	}

	hlist_for_each_entry_rcu(ei, &entry->list, hlist) {
		struct fib_alias *fa;
		__be32 mask, prefix;

		mask = inet_make_mask(ei->plen);
		prefix = htonl(entry->key);

		list_for_each_entry_rcu(fa, &ei->falh, fa_list) {
			const struct fib_info *fi = fa->fa_info;
			unsigned int flags = fib_flag_trans(fa->fa_type, mask, fi);

			if (fa->fa_type == RTN_BROADCAST
			    || fa->fa_type == RTN_MULTICAST)
				continue;

			seq_setwidth(seq, 127);

			if (fi)
				seq_printf(seq,
					 "%s\t%08X\t%08X\t%04X\t%d\t%u\t"
					 "%d\t%08X\t%d\t%u\t%u",
					 fi->fib_dev ? fi->fib_dev->name : "*",
					 prefix,
					 fi->fib_nh->nh_gw, flags, 0, 0,
					 fi->fib_priority,
					 mask,
					 (fi->fib_advmss ?
					  fi->fib_advmss + 40 : 0),
					 fi->fib_window,
					 fi->fib_rtt >> 3);
			else
				seq_printf(seq,
					 "*\t%08X\t%08X\t%04X\t%d\t%u\t"
					 "%d\t%08X\t%d\t%u\t%u",
					 prefix, 0, flags, 0, 0, 0,
					 mask, 0, 0, 0);

			seq_pad(seq, '\n');
		}
	}

	return 0;
}

static const struct seq_operations fib_route_seq_ops = {
	.start  = fib_route_seq_start,
	.next   = fib_route_seq_next,
	.stop   = fib_route_seq_stop,
	.show   = fib_route_seq_show,
};

static int fib_route_seq_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &fib_route_seq_ops,
			    sizeof(struct fib_route_iter));
}

static const struct file_operations fib_route_fops = {
	.owner  = THIS_MODULE,
	.open   = fib_route_seq_open,
	.read   = seq_read,
	.llseek = seq_lseek,
	.release = seq_release_net,
};

int __net_init fib_proc_init(struct net *net)
{
	if (!proc_create("route", S_IRUGO, net->proc_net, &fib_route_fops))
		return -ENOMEM;

	return 0;
}

void __net_exit fib_proc_exit(struct net *net)
{
	remove_proc_entry("route", net->proc_net);
}

#endif /* CONFIG_PROC_FS */

void __init fib_trie_init(void) {}
