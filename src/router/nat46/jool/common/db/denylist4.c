#include "mod/common/db/denylist4.h"

#include "mod/common/dev.h"
#include "mod/common/address.h"
#include "mod/common/log.h"
#include "mod/common/rcu.h"
#include "mod/common/wkmalloc.h"
#include "mod/common/xlator.h"

struct pool_entry {
	struct ipv4_prefix prefix;
	struct list_head list_hook;
};

struct addr4_pool {
	struct list_head __rcu *list;
	struct kref refcounter;
};

/* I can't have per-pool mutexes because of the replace function. */
static DEFINE_MUTEX(lock);

static struct pool_entry *get_entry(struct list_head *node)
{
	return list_entry(node, struct pool_entry, list_hook);
}

static struct list_head *alloc_list(void)
{
	struct list_head *list;

	list = __wkmalloc("IPv4 address pool list", sizeof(*list), GFP_KERNEL);
	if (!list)
		return NULL;
	INIT_LIST_HEAD(list);

	return list;
}

struct addr4_pool *denylist4_alloc(void)
{
	struct addr4_pool *result;
	struct list_head *list;

	result = wkmalloc(struct addr4_pool, GFP_KERNEL);
	if (!result)
		return NULL;

	list = alloc_list();
	if (!list) {
		wkfree(struct addr4_pool, result);
		return NULL;
	}

	RCU_INIT_POINTER(result->list, list);
	kref_init(&result->refcounter);

	return result;
}

void denylist4_get(struct addr4_pool *pool)
{
	kref_get(&pool->refcounter);
}

static void __destroy(struct list_head *list)
{
	struct list_head *node;
	struct list_head *tmp;

	list_for_each_safe(node, tmp, list) {
		list_del(node);
		wkfree(struct pool_entry, get_entry(node));
	}

	__wkfree("IPv4 address pool list", list);
}

static void pool_release(struct kref *refcounter)
{
	struct addr4_pool *pool;
	pool = container_of(refcounter, struct addr4_pool, refcounter);
	__destroy(rcu_dereference_raw(pool->list));
	wkfree(struct addr4_pool, pool);
}

void denylist4_put(struct addr4_pool *pool)
{
	kref_put(&pool->refcounter, pool_release);
}

int denylist4_add(struct addr4_pool *pool, struct ipv4_prefix *prefix,
		bool force)
{
	struct list_head *list;
	struct pool_entry *entry;
	int error;

	error = prefix4_validate(prefix);
	if (error)
		return error;
	error = prefix4_validate_scope(prefix, force);
	if (error)
		return error;

	mutex_lock(&lock);

	entry = wkmalloc(struct pool_entry, GFP_KERNEL);
	if (!entry) {
		error = -ENOMEM;
		goto end;
	}
	entry->prefix = *prefix;

	list = rcu_dereference_protected(pool->list, lockdep_is_held(&lock));
	list_add_tail_rcu(&entry->list_hook, list);

end:
	mutex_unlock(&lock);
	return error;
}

int denylist4_rm(struct addr4_pool *pool, struct ipv4_prefix *prefix)
{
	struct list_head *list;
	struct list_head *node;
	struct pool_entry *entry;

	mutex_lock(&lock);

	list = rcu_dereference_protected(pool->list, lockdep_is_held(&lock));
	list_for_each(node, list) {
		entry = get_entry(node);
		if (prefix4_equals(prefix, &entry->prefix)) {
			list_del_rcu(&entry->list_hook);
			mutex_unlock(&lock);
			synchronize_rcu_bh();
			wkfree(struct pool_entry, entry);
			return 0;
		}
	}

	mutex_unlock(&lock);
	log_err("Could not find the requested entry in the IPv4 pool.");
	return -ESRCH;
}

int denylist4_flush(struct addr4_pool *pool)
{
	struct list_head *old;
	struct list_head *new;

	new = alloc_list();
	if (!new)
		return -ENOMEM;

	mutex_lock(&lock);
	old = rcu_dereference_protected(pool->list, lockdep_is_held(&lock));
	rcu_assign_pointer(pool->list, new);
	mutex_unlock(&lock);

	synchronize_rcu_bh();

	__destroy(old);
	return 0;
}

#define ALLOW_ADDRESS false
#define DENY_ADDRESS true

/* "Check interface address" */
static int check_ifa(struct in_ifaddr *ifa, void const *arg)
{
	struct in_addr const *query = arg;
	struct in_addr ifaddr;

	/* Broadcast */
	/* (RFC3021: /31 and /32 networks lack broadcast) */
	if (ifa->ifa_prefixlen < 31) {
		ifaddr.s_addr = ifa->ifa_local | ~ifa->ifa_mask;
		if (ipv4_addr_cmp(&ifaddr, query) == 0)
			return DENY_ADDRESS;
	}

	/* /32 (https://github.com/NICMx/Jool/issues/342) */
	if (ifa->ifa_prefixlen == 32)
		return ALLOW_ADDRESS;

	/* Address belongs to this node */
	ifaddr.s_addr = ifa->ifa_local;
	if (ipv4_addr_cmp(&ifaddr, query) == 0)
		return DENY_ADDRESS;

	/* Anything else */
	return ALLOW_ADDRESS;
}

/**
 * Is @addr *NOT* translatable, according to the interfaces?
 *
 * The name comes from the fact that interface addresses are usually
 * non-translatable (ie. the traffic is meant for the translator box).
 *
 * Recognizable directed broadcast is also not translatable.
 */
bool interface_contains(struct net *ns, struct in_addr *addr)
{
	return foreach_ifa(ns, check_ifa, addr);
}

bool denylist4_contains(struct addr4_pool *pool, struct in_addr *addr)
{
	struct list_head *list;
	struct list_head *node;
	struct pool_entry *entry;

	rcu_read_lock_bh();

	list = rcu_dereference_bh(pool->list);
	list_for_each_rcu_bh(node, list) {
		entry = get_entry(node);
		if (prefix4_contains(&entry->prefix, addr)) {
			rcu_read_unlock_bh();
			return true;
		}
	}

	rcu_read_unlock_bh();
	return false;
}

int denylist4_foreach(struct addr4_pool *pool,
		int (*func)(struct ipv4_prefix *, void *), void *arg,
		struct ipv4_prefix *offset)
{
	struct list_head *list;
	struct list_head *node;
	struct pool_entry *entry;
	int error = 0;

	rcu_read_lock_bh();

	list = rcu_dereference_bh(pool->list);
	list_for_each_rcu_bh(node, list) {
		entry = get_entry(node);
		if (!offset) {
			error = func(&entry->prefix, arg);
			if (error)
				break;
		} else if (prefix4_equals(offset, &entry->prefix)) {
			offset = NULL;
		}
	}

	rcu_read_unlock_bh();
	return offset ? -ESRCH : error;
}

bool denylist4_is_empty(struct addr4_pool *pool)
{
	struct list_head *list;
	bool result;

	rcu_read_lock_bh();
	list = rcu_dereference_bh(pool->list);
	result = list_empty(list);
	rcu_read_unlock_bh();

	return result;
}
