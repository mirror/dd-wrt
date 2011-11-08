/*
 * Copyright (c) 2006 Silicon Graphics, Inc.
 * All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <xfs/platform_defs.h>
#include <xfs/list.h>
#include <xfs/cache.h>

#define CACHE_DEBUG 1
#undef CACHE_DEBUG
#define CACHE_DEBUG 1
#undef CACHE_ABORT
/* #define CACHE_ABORT 1 */

#define CACHE_SHAKE_COUNT	64

static unsigned int cache_generic_bulkrelse(struct cache *, struct list_head *);

struct cache *
cache_init(
	unsigned int		hashsize,
	struct cache_operations	*cache_operations)
{
	struct cache *		cache;
	unsigned int		i, maxcount;

	maxcount = hashsize * HASH_CACHE_RATIO;

	if (!(cache = malloc(sizeof(struct cache))))
		return NULL;
	if (!(cache->c_hash = calloc(hashsize, sizeof(struct cache_hash)))) {
		free(cache);
		return NULL;
	}

	cache->c_count = 0;
	cache->c_max = 0;
	cache->c_hits = 0;
	cache->c_misses = 0;
	cache->c_maxcount = maxcount;
	cache->c_hashsize = hashsize;
	cache->hash = cache_operations->hash;
	cache->alloc = cache_operations->alloc;
	cache->flush = cache_operations->flush;
	cache->relse = cache_operations->relse;
	cache->compare = cache_operations->compare;
	cache->bulkrelse = cache_operations->bulkrelse ?
		cache_operations->bulkrelse : cache_generic_bulkrelse;
	pthread_mutex_init(&cache->c_mutex, NULL);

	for (i = 0; i < hashsize; i++) {
		list_head_init(&cache->c_hash[i].ch_list);
		cache->c_hash[i].ch_count = 0;
		pthread_mutex_init(&cache->c_hash[i].ch_mutex, NULL);
	}

	for (i = 0; i <= CACHE_MAX_PRIORITY; i++) {
		list_head_init(&cache->c_mrus[i].cm_list);
		cache->c_mrus[i].cm_count = 0;
		pthread_mutex_init(&cache->c_mrus[i].cm_mutex, NULL);
	}
	return cache;
}

void
cache_expand(
	struct cache *		cache)
{
	pthread_mutex_lock(&cache->c_mutex);
#ifdef CACHE_DEBUG
	fprintf(stderr, "doubling cache size to %d\n", 2 * cache->c_maxcount);
#endif
	cache->c_maxcount *= 2;
	pthread_mutex_unlock(&cache->c_mutex);
}

void
cache_walk(
	struct cache *		cache,
	cache_walk_t		visit)
{
	struct cache_hash *	hash;
	struct list_head *	head;
	struct list_head *	pos;
	unsigned int		i;

	for (i = 0; i < cache->c_hashsize; i++) {
		hash = &cache->c_hash[i];
		head = &hash->ch_list;
		pthread_mutex_lock(&hash->ch_mutex);
		for (pos = head->next; pos != head; pos = pos->next)
			visit((struct cache_node *)pos);
		pthread_mutex_unlock(&hash->ch_mutex);
	}
}

#ifdef CACHE_ABORT
#define cache_abort()	abort()
#else
#define cache_abort()	do { } while (0)
#endif

#ifdef CACHE_DEBUG
static void
cache_zero_check(
	struct cache_node *	node)
{
	if (node->cn_count > 0) {
		fprintf(stderr, "%s: refcount is %u, not zero (node=%p)\n",
			__FUNCTION__, node->cn_count, node);
		cache_abort();
	}
}
#define cache_destroy_check(c)	cache_walk((c), cache_zero_check)
#else
#define cache_destroy_check(c)	do { } while (0)
#endif

void
cache_destroy(
	struct cache *		cache)
{
	unsigned int		i;

	cache_destroy_check(cache);
	for (i = 0; i < cache->c_hashsize; i++) {
		list_head_destroy(&cache->c_hash[i].ch_list);
		pthread_mutex_destroy(&cache->c_hash[i].ch_mutex);
	}
	for (i = 0; i <= CACHE_MAX_PRIORITY; i++) {
		list_head_destroy(&cache->c_mrus[i].cm_list);
		pthread_mutex_destroy(&cache->c_mrus[i].cm_mutex);
	}
	pthread_mutex_destroy(&cache->c_mutex);
	free(cache->c_hash);
	free(cache);
}

static unsigned int
cache_generic_bulkrelse(
	struct cache *		cache,
	struct list_head *	list)
{
	struct cache_node *	node;
	unsigned int		count = 0;

	while (!list_empty(list)) {
		node = list_entry(list->next, struct cache_node, cn_mru);
		pthread_mutex_destroy(&node->cn_mutex);
		list_del_init(&node->cn_mru);
		cache->relse(node);
		count++;
	}

	return count;
}

/*
 * We've hit the limit on cache size, so we need to start reclaiming
 * nodes we've used. The MRU specified by the priority is shaken.
 * Returns new priority at end of the call (in case we call again).
 */
static unsigned int
cache_shake(
	struct cache *		cache,
	unsigned int		priority,
	int			all)
{
	struct cache_mru	*mru;
	struct cache_hash *	hash;
	struct list_head	temp;
	struct list_head *	head;
	struct list_head *	pos;
	struct list_head *	n;
	struct cache_node *	node;
	unsigned int		count;

	ASSERT(priority <= CACHE_MAX_PRIORITY);
	if (priority > CACHE_MAX_PRIORITY)
		priority = 0;

	mru = &cache->c_mrus[priority];
	count = 0;
	list_head_init(&temp);
	head = &mru->cm_list;

	pthread_mutex_lock(&mru->cm_mutex);
	for (pos = head->prev, n = pos->prev; pos != head;
						pos = n, n = pos->prev) {
		node = list_entry(pos, struct cache_node, cn_mru);

		if (pthread_mutex_trylock(&node->cn_mutex) != 0)
			continue;

		hash = cache->c_hash + node->cn_hashidx;
		if (pthread_mutex_trylock(&hash->ch_mutex) != 0) {
			pthread_mutex_unlock(&node->cn_mutex);
			continue;
		}
		ASSERT(node->cn_count == 0);
		ASSERT(node->cn_priority == priority);
		node->cn_priority = -1;

		list_move(&node->cn_mru, &temp);
		list_del_init(&node->cn_hash);
		hash->ch_count--;
		mru->cm_count--;
		pthread_mutex_unlock(&hash->ch_mutex);
		pthread_mutex_unlock(&node->cn_mutex);

		count++;
		if (!all && count == CACHE_SHAKE_COUNT)
			break;
	}
	pthread_mutex_unlock(&mru->cm_mutex);

	if (count > 0) {
		cache->bulkrelse(cache, &temp);

		pthread_mutex_lock(&cache->c_mutex);
		cache->c_count -= count;
		pthread_mutex_unlock(&cache->c_mutex);
	}

	return (count == CACHE_SHAKE_COUNT) ? priority : ++priority;
}

/*
 * Allocate a new hash node (updating atomic counter in the process),
 * unless doing so will push us over the maximum cache size.
 */
static struct cache_node *
cache_node_allocate(
	struct cache *		cache,
	cache_key_t		key)
{
	unsigned int		nodesfree;
	struct cache_node *	node;

	pthread_mutex_lock(&cache->c_mutex);
	nodesfree = (cache->c_count < cache->c_maxcount);
	if (nodesfree) {
		cache->c_count++;
		if (cache->c_count > cache->c_max)
			cache->c_max = cache->c_count;
	}
	cache->c_misses++;
	pthread_mutex_unlock(&cache->c_mutex);
	if (!nodesfree)
		return NULL;
	node = cache->alloc(key);
	if (node == NULL) {	/* uh-oh */
		pthread_mutex_lock(&cache->c_mutex);
		cache->c_count--;
		pthread_mutex_unlock(&cache->c_mutex);
		return NULL;
	}
	pthread_mutex_init(&node->cn_mutex, NULL);
	list_head_init(&node->cn_mru);
	node->cn_count = 1;
	node->cn_priority = 0;
	return node;
}

int
cache_overflowed(
	struct cache *		cache)
{
	return (cache->c_maxcount == cache->c_max);
}

/*
 * Lookup in the cache hash table.  With any luck we'll get a cache
 * hit, in which case this will all be over quickly and painlessly.
 * Otherwise, we allocate a new node, taking care not to expand the
 * cache beyond the requested maximum size (shrink it if it would).
 * Returns one if hit in cache, otherwise zero.  A node is _always_
 * returned, however.
 */
int
cache_node_get(
	struct cache *		cache,
	cache_key_t		key,
	struct cache_node **	nodep)
{
	struct cache_node *	node = NULL;
	struct cache_hash *	hash;
	struct cache_mru *	mru;
	struct list_head *	head;
	struct list_head *	pos;
	unsigned int		hashidx;
	int			priority = 0;

	hashidx = cache->hash(key, cache->c_hashsize);
	hash = cache->c_hash + hashidx;
	head = &hash->ch_list;

	for (;;) {
		pthread_mutex_lock(&hash->ch_mutex);
		for (pos = head->next; pos != head; pos = pos->next) {
			node = list_entry(pos, struct cache_node, cn_hash);
			if (!cache->compare(node, key))
				continue;
			/*
			 * node found, bump node's reference count, remove it
			 * from its MRU list, and update stats.
			 */
			pthread_mutex_lock(&node->cn_mutex);

			if (node->cn_count == 0) {
				ASSERT(node->cn_priority >= 0);
				ASSERT(!list_empty(&node->cn_mru));
				mru = &cache->c_mrus[node->cn_priority];
				pthread_mutex_lock(&mru->cm_mutex);
				mru->cm_count--;
				list_del_init(&node->cn_mru);
				pthread_mutex_unlock(&mru->cm_mutex);
			}
			node->cn_count++;

			pthread_mutex_unlock(&node->cn_mutex);
			pthread_mutex_unlock(&hash->ch_mutex);

			pthread_mutex_lock(&cache->c_mutex);
			cache->c_hits++;
			pthread_mutex_unlock(&cache->c_mutex);

			*nodep = node;
			return 0;
		}
		pthread_mutex_unlock(&hash->ch_mutex);
		/*
		 * not found, allocate a new entry
		 */
		node = cache_node_allocate(cache, key);
		if (node)
			break;
		priority = cache_shake(cache, priority, 0);
		/*
		 * We start at 0; if we free CACHE_SHAKE_COUNT we get
		 * back the same priority, if not we get back priority+1.
		 * If we exceed CACHE_MAX_PRIORITY all slots are full; grow it.
		 */
		if (priority > CACHE_MAX_PRIORITY) {
			priority = 0;
			cache_expand(cache);
		}
	}

	node->cn_hashidx = hashidx;

	/* add new node to appropriate hash */
	pthread_mutex_lock(&hash->ch_mutex);
	hash->ch_count++;
	list_add(&node->cn_hash, &hash->ch_list);
	pthread_mutex_unlock(&hash->ch_mutex);

	*nodep = node;
	return 1;
}

void
cache_node_put(
	struct cache *		cache,
	struct cache_node *	node)
{
	struct cache_mru *	mru;

	pthread_mutex_lock(&node->cn_mutex);
#ifdef CACHE_DEBUG
	if (node->cn_count < 1) {
		fprintf(stderr, "%s: node put on refcount %u (node=%p)\n",
				__FUNCTION__, node->cn_count, node);
		cache_abort();
	}
	if (!list_empty(&node->cn_mru)) {
		fprintf(stderr, "%s: node put on node (%p) in MRU list\n",
				__FUNCTION__, node);
		cache_abort();
	}
#endif
	node->cn_count--;

	if (node->cn_count == 0) {
		/* add unreferenced node to appropriate MRU for shaker */
		mru = &cache->c_mrus[node->cn_priority];
		pthread_mutex_lock(&mru->cm_mutex);
		mru->cm_count++;
		list_add(&node->cn_mru, &mru->cm_list);
		pthread_mutex_unlock(&mru->cm_mutex);
	}

	pthread_mutex_unlock(&node->cn_mutex);
}

void
cache_node_set_priority(
	struct cache *		cache,
	struct cache_node *	node,
	int			priority)
{
	if (priority < 0)
		priority = 0;
	else if (priority > CACHE_MAX_PRIORITY)
		priority = CACHE_MAX_PRIORITY;

	pthread_mutex_lock(&node->cn_mutex);
	ASSERT(node->cn_count > 0);
	node->cn_priority = priority;
	pthread_mutex_unlock(&node->cn_mutex);
}

int
cache_node_get_priority(
	struct cache_node *	node)
{
	int			priority;

	pthread_mutex_lock(&node->cn_mutex);
	priority = node->cn_priority;
	pthread_mutex_unlock(&node->cn_mutex);

	return priority;
}


/*
 * Purge a specific node from the cache.  Reference count must be zero.
 */
int
cache_node_purge(
	struct cache *		cache,
	cache_key_t		key,
	struct cache_node *	node)
{
	struct list_head *	head;
	struct list_head *	pos;
	struct list_head *	n;
	struct cache_hash *	hash;
	struct cache_mru *	mru;
	int			count = -1;

	hash = cache->c_hash + cache->hash(key, cache->c_hashsize);
	head = &hash->ch_list;
	pthread_mutex_lock(&hash->ch_mutex);
	for (pos = head->next, n = pos->next; pos != head;
						pos = n, n = pos->next) {
		if ((struct cache_node *)pos != node)
			continue;

		pthread_mutex_lock(&node->cn_mutex);
		count = node->cn_count;
		if (count != 0) {
			pthread_mutex_unlock(&node->cn_mutex);
			break;
		}
		mru = &cache->c_mrus[node->cn_priority];
		pthread_mutex_lock(&mru->cm_mutex);
		list_del_init(&node->cn_mru);
		mru->cm_count--;
		pthread_mutex_unlock(&mru->cm_mutex);

		pthread_mutex_unlock(&node->cn_mutex);
		pthread_mutex_destroy(&node->cn_mutex);
		list_del_init(&node->cn_hash);
		hash->ch_count--;
		cache->relse(node);
		break;
	}
	pthread_mutex_unlock(&hash->ch_mutex);

	if (count == 0) {
		pthread_mutex_lock(&cache->c_mutex);
		cache->c_count--;
		pthread_mutex_unlock(&cache->c_mutex);
	}
#ifdef CACHE_DEBUG
	if (count >= 1) {
		fprintf(stderr, "%s: refcount was %u, not zero (node=%p)\n",
				__FUNCTION__, count, node);
		cache_abort();
	}
	if (count == -1) {
		fprintf(stderr, "%s: purge node not found! (node=%p)\n",
			__FUNCTION__, node);
		cache_abort();
	}
#endif
	return (count == 0);
}

/*
 * Purge all nodes from the cache.  All reference counts must be zero.
 */
void
cache_purge(
	struct cache *		cache)
{
	int			i;

	for (i = 0; i <= CACHE_MAX_PRIORITY; i++)
		cache_shake(cache, i, 1);

#ifdef CACHE_DEBUG
	if (cache->c_count != 0) {
		/* flush referenced nodes to disk */
		cache_flush(cache);
		fprintf(stderr, "%s: shake on cache %p left %u nodes!?\n",
				__FUNCTION__, cache, cache->c_count);
		cache_abort();
	}
#endif
}

/*
 * Flush all nodes in the cache to disk.
 */
void
cache_flush(
	struct cache *		cache)
{
	struct cache_hash *	hash;
	struct list_head *	head;
	struct list_head *	pos;
	struct cache_node *	node;
	int			i;

	if (!cache->flush)
		return;

	for (i = 0; i < cache->c_hashsize; i++) {
		hash = &cache->c_hash[i];

		pthread_mutex_lock(&hash->ch_mutex);
		head = &hash->ch_list;
		for (pos = head->next; pos != head; pos = pos->next) {
			node = (struct cache_node *)pos;
			pthread_mutex_lock(&node->cn_mutex);
			cache->flush(node);
			pthread_mutex_unlock(&node->cn_mutex);
		}
		pthread_mutex_unlock(&hash->ch_mutex);
	}
}

#define	HASH_REPORT	(3 * HASH_CACHE_RATIO)
void
cache_report(
	FILE 			*fp,
	const char 		*name,
	struct cache 		*cache)
{
	int 			i;
	unsigned long 		count, index, total;
	unsigned long 		hash_bucket_lengths[HASH_REPORT + 2];

	if ((cache->c_hits + cache->c_misses) == 0)
		return;

	/* report cache summary */
	fprintf(fp, "%s: %p\n"
			"Max supported entries = %u\n"
			"Max utilized entries = %u\n"
			"Active entries = %u\n"
			"Hash table size = %u\n"
			"Hits = %llu\n"
			"Misses = %llu\n"
			"Hit ratio = %5.2f\n",
			name, cache,
			cache->c_maxcount,
			cache->c_max,
			cache->c_count,
			cache->c_hashsize,
			cache->c_hits,
			cache->c_misses,
			(double)cache->c_hits * 100 /
				(cache->c_hits + cache->c_misses)
	);

	for (i = 0; i <= CACHE_MAX_PRIORITY; i++)
		fprintf(fp, "MRU %d entries = %6u (%3u%%)\n",
			i, cache->c_mrus[i].cm_count,
			cache->c_mrus[i].cm_count * 100 / cache->c_count);

	/* report hash bucket lengths */
	bzero(hash_bucket_lengths, sizeof(hash_bucket_lengths));

	for (i = 0; i < cache->c_hashsize; i++) {
		count = cache->c_hash[i].ch_count;
		if (count > HASH_REPORT)
			index = HASH_REPORT + 1;
		else
			index = count;
		hash_bucket_lengths[index]++;
	}

	total = 0;
	for (i = 0; i < HASH_REPORT + 1; i++) {
		total += i * hash_bucket_lengths[i];
		if (hash_bucket_lengths[i] == 0)
			continue;
		fprintf(fp, "Hash buckets with  %2d entries %6ld (%3ld%%)\n",
			i, hash_bucket_lengths[i],
			(i * hash_bucket_lengths[i] * 100) / cache->c_count);
	}
	if (hash_bucket_lengths[i])	/* last report bucket is the overflow bucket */
		fprintf(fp, "Hash buckets with >%2d entries %6ld (%3ld%%)\n",
			i - 1, hash_bucket_lengths[i],
			((cache->c_count - total) * 100) / cache->c_count);
}
