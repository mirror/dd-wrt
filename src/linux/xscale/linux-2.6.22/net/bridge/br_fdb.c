/*
 *	Forwarding database
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_fdb.c,v 1.6 2002/01/17 00:57:07 davem Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include "br_private.h"

static struct kmem_cache *br_fdb_cache __read_mostly;
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		      const unsigned char *addr);

static u32 fdb_salt __read_mostly;

int __init br_fdb_init(void)
{
	br_fdb_cache = kmem_cache_create("bridge_fdb_cache",
					 sizeof(struct net_bridge_fdb_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL, NULL);
	if (!br_fdb_cache)
		return -ENOMEM;

	get_random_bytes(&fdb_salt, sizeof(fdb_salt));
	return 0;
}

void __exit br_fdb_fini(void)
{
	kmem_cache_destroy(br_fdb_cache);
}


/* if topology_changing then use forward_delay (default 15 sec)
 * otherwise keep longer (default 5 minutes)
 */
static inline unsigned long hold_time(const struct net_bridge *br)
{
    /* We use forward_delay=0. If code unchanged, every entry in fdb will expire immidately */
    /* and every packet flood the local network for a period of bridge_max_age afterboot up */
    /* So we decoulpe this timer from forward_delay. */
	return br->topology_change ? (15*HZ) : br->ageing_time;
	//return br->topology_change ? br->forward_delay : br->ageing_time;
}

static inline int has_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb)
{
	return !fdb->is_static
		&& time_before_eq(fdb->ageing_timer + hold_time(br), jiffies);
}

static inline int br_mac_hash(const unsigned char *mac)
{
	/* use 1 byte of OUI cnd 3 bytes of NIC */
	u32 key = get_unaligned((u32 *)(mac + 2));
	return jhash_1word(key, fdb_salt) & (BR_HASH_SIZE - 1);
}

static inline void fdb_delete(struct net_bridge_fdb_entry *f)
{
	hlist_del_rcu(&f->hlist);
	br_fdb_put(f);
}


void dolist(struct net_bridge *br)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;

	
	if (!br)
	    return;

	printk(KERN_EMERG "bridge	device	group			source			timeout\n");
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    printk(KERN_EMERG "%s	%s  	", br->dev->name, dst->dst->dev->name);
	    addr_debug((unsigned char *) &dst->addr);
	    printk(KERN_EMERG "	");
	    addr_debug((unsigned char *) &dst->host);
	    printk(KERN_EMERG "	%d\n", (int) (dst->tstamp - jiffies)/HZ);
	}
}

int br_mc_fdb_update(struct net_bridge *br, struct net_bridge_port *prt, const unsigned char *dest, unsigned char *host)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	int ret = 0;
    
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if (!memcmp(&dst->addr, dest, ETH_ALEN)) {
		dst->tstamp = jiffies + QUERY_TIMEOUT*HZ;
		if (!memcmp(&dst->host, host, ETH_ALEN))
		    ret = 1;
	    }
	}
	
	return ret;
}

struct net_bridge_mc_fdb_entry *br_mc_fdb_get(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
    
	list_for_each_rcu(lh, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) && (!memcmp(&dst->host, host, ETH_ALEN))) {
		if (dst->dst == prt)
		    return dst;
	    }
	}
	
	return NULL;
}

extern mac_addr upnp_addr;

int br_mc_fdb_add(struct net_bridge *br, struct net_bridge_port *prt, const unsigned char *dest, unsigned char *host)
{
	struct net_bridge_mc_fdb_entry *mc_fdb;

	//printk("--- add mc entry ---\n");

	if (!memcmp(dest, &upnp_addr, ETH_ALEN))
	    return 0;
	    
	if (br_mc_fdb_update(br, prt, dest, host))
	    return 0;
	    
	mc_fdb = kmalloc(sizeof(struct net_bridge_mc_fdb_entry), GFP_KERNEL);
	if (!mc_fdb)
	    return ENOMEM;
	memcpy(mc_fdb->addr.addr, dest, ETH_ALEN);
	memcpy(mc_fdb->host.addr, host, ETH_ALEN);
	mc_fdb->dst = prt;
	mc_fdb->tstamp = jiffies + QUERY_TIMEOUT*HZ;;
	spin_lock_bh(&br->mcl_lock);
	list_add_tail_rcu(&mc_fdb->list, &br->mc_list);
	spin_unlock_bh(&br->mcl_lock);

	if (!br->start_timer) {
    	    init_timer(&br->igmp_timer);
	    br->igmp_timer.expires = jiffies + TIMER_CHECK_TIMEOUT*HZ;
	    br->igmp_timer.function = query_timeout;
	    br->igmp_timer.data = (unsigned long) br;
	    add_timer(&br->igmp_timer);
	    br->start_timer = 1;
	}

	return 1;
}

void br_mc_fdb_cleanup(struct net_bridge *br)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	struct list_head *tmp;
    
	spin_lock_bh(&br->mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    list_del_rcu(&dst->list);
	    kfree(dst);
	}
	spin_unlock_bh(&br->mcl_lock);
}

void br_mc_fdb_remove_grp(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest)
{
	struct net_bridge_mc_fdb_entry *dst;
	struct list_head *lh;
	struct list_head *tmp;

	spin_lock_bh(&br->mcl_lock);
	list_for_each_safe_rcu(lh, tmp, &br->mc_list) {
	    dst = (struct net_bridge_mc_fdb_entry *) list_entry(lh, struct net_bridge_mc_fdb_entry, list);
	    if ((!memcmp(&dst->addr, dest, ETH_ALEN)) && (dst->dst == prt)) {
		list_del_rcu(&dst->list);
		kfree(dst);
	    }
	}
	spin_unlock_bh(&br->mcl_lock);
}

int br_mc_fdb_remove(struct net_bridge *br, struct net_bridge_port *prt, unsigned char *dest, unsigned char *host)
{
	struct net_bridge_mc_fdb_entry *mc_fdb;

	//printk("--- remove mc entry ---\n");
	
	if ((mc_fdb = br_mc_fdb_get(br, prt, dest, host))) {
	    spin_lock_bh(&br->mcl_lock);
	    list_del_rcu(&mc_fdb->list);
	    kfree(mc_fdb);
	    spin_unlock_bh(&br->mcl_lock);

	    return 1;
	}
	
	return 0;
}

void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr)
{
	struct net_bridge *br = p->br;
	int i;

	spin_lock_bh(&br->hash_lock);

	/* Search all chains since old address/hash is unknown */
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h;
		hlist_for_each(h, &br->hash[i]) {
			struct net_bridge_fdb_entry *f;

			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst == p && f->is_local) {
				/* maybe another port has same hw addr? */
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p &&
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto insert;
					}
				}

				/* delete old one */
				fdb_delete(f);
				goto insert;
			}
		}
	}
 insert:
	/* insert new address,  may fail if invalid address or dup. */
	fdb_insert(br, p, newaddr);

	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_cleanup(unsigned long _data)
{
	struct net_bridge *br = (struct net_bridge *)_data;
	unsigned long delay = hold_time(br);
	unsigned long next_timer = jiffies + br->forward_delay;
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *h, *n;

		hlist_for_each_entry_safe(f, h, n, &br->hash[i], hlist) {
			unsigned long this_timer;
			if (f->is_static)
				continue;
			this_timer = f->ageing_timer + delay;
			if (time_before_eq(this_timer, jiffies))
				fdb_delete(f);
			else if (this_timer < next_timer)
				next_timer = this_timer;
		}
	}
	spin_unlock_bh(&br->hash_lock);

	/* Add HZ/4 to ensure we round the jiffies upwards to be after the next
	 * timer, otherwise we might round down and will have no-op run. */
	mod_timer(&br->gc_timer, round_jiffies(next_timer + HZ/4));
}

/* Completely flush all dynamic entries in forwarding database.*/
void br_fdb_flush(struct net_bridge *br)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *h, *n;
		hlist_for_each_entry_safe(f, h, n, &br->hash[i], hlist) {
			if (!f->is_static)
				fdb_delete(f);
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* Flush all entries refering to a specific port.
 * if do_all is set also flush static entries
 */
void br_fdb_delete_by_port(struct net_bridge *br,
			   const struct net_bridge_port *p,
			   int do_all)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			struct net_bridge_fdb_entry *f
				= hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst != p)
				continue;

			if (f->is_static && !do_all)
				continue;
			/*
			 * if multiple ports all have the same device address
			 * then when one port is deleted, assign
			 * the local entry to other port
			 */
			if (f->is_local) {
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p &&
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto skip_delete;
					}
				}
			}

			fdb_delete(f);
		skip_delete: ;
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* No locking or refcounting, assumes caller has no preempt (rcu_read_lock) */
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, &br->hash[br_mac_hash(addr)], hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr)) {
			if (unlikely(has_expired(br, fdb)))
				break;
			return fdb;
		}
	}

	return NULL;
}

/* Interface used by ATM hook that keeps a ref count */
struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					unsigned char *addr)
{
	struct net_bridge_fdb_entry *fdb;

	rcu_read_lock();
	fdb = __br_fdb_get(br, addr);
	if (fdb && !atomic_inc_not_zero(&fdb->use_count))
		fdb = NULL;
	rcu_read_unlock();
	return fdb;
}

static void fdb_rcu_free(struct rcu_head *head)
{
	struct net_bridge_fdb_entry *ent
		= container_of(head, struct net_bridge_fdb_entry, rcu);
	kmem_cache_free(br_fdb_cache, ent);
}

/* Set entry up for deletion with RCU  */
void br_fdb_put(struct net_bridge_fdb_entry *ent)
{
	if (atomic_dec_and_test(&ent->use_count))
		call_rcu(&ent->rcu, fdb_rcu_free);
}

/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int br_fdb_fillbuf(struct net_bridge *br, void *buf,
		   unsigned long maxnum, unsigned long skip)
{
	struct __fdb_entry *fe = buf;
	int i, num = 0;
	struct hlist_node *h;
	struct net_bridge_fdb_entry *f;

	memset(buf, 0, maxnum*sizeof(struct __fdb_entry));

	rcu_read_lock();
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, h, &br->hash[i], hlist) {
			if (num >= maxnum)
				goto out;

			if (has_expired(br, f))
				continue;

			if (skip) {
				--skip;
				continue;
			}

			/* convert from internal format to API */
			memcpy(fe->mac_addr, f->addr.addr, ETH_ALEN);
			fe->port_no = f->dst->port_no;
			fe->is_local = f->is_local;
			if (!f->is_static)
				fe->ageing_timer_value = jiffies_to_clock_t(jiffies - f->ageing_timer);
			++fe;
			++num;
		}
	}

 out:
	rcu_read_unlock();

	return num;
}

static inline struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
						    const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, head, hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr))
			return fdb;
	}
	return NULL;
}

static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       int is_local)
{
	struct net_bridge_fdb_entry *fdb;

	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) {
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		atomic_set(&fdb->use_count, 1);
		hlist_add_head_rcu(&fdb->hlist, head);

		fdb->dst = source;
		fdb->is_local = is_local;
		fdb->is_static = is_local;
		fdb->ageing_timer = jiffies;
	}
	return fdb;
}

static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	fdb = fdb_find(head, addr);
	if (fdb) {
		/* it is okay to have multiple ports with same
		 * address, just use the first one.
		 */
		if (fdb->is_local)
			return 0;

		printk(KERN_WARNING "%s adding interface with same address "
		       "as a received packet\n",
		       source->dev->name);
		fdb_delete(fdb);
	}

	if (!fdb_create(head, source, addr, 1))
		return -ENOMEM;

	return 0;
}

int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	int ret;

	spin_lock_bh(&br->hash_lock);
	ret = fdb_insert(br, source, addr);
	spin_unlock_bh(&br->hash_lock);
	return ret;
}

void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	/* some users want to always flood. */
	if (hold_time(br) == 0)
		return;

	fdb = fdb_find(head, addr);
	if (likely(fdb)) {
		/* attempt to update an entry for a local interface */
		if (unlikely(fdb->is_local)) {
		#if 0 
			if (net_ratelimit())
				printk(KERN_WARNING "%s: received packet with "
				       " own address as source address\n",
				       source->dev->name);
		#endif
		} else {
			/* fastpath: update of existing entry */
			fdb->dst = source;
			fdb->ageing_timer = jiffies;
		}
	} else {
		spin_lock(&br->hash_lock);
		if (!fdb_find(head, addr))
			fdb_create(head, source, addr, 0);
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock(&br->hash_lock);
	}
}
