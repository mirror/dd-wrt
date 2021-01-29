/**
 * cache.c : deal with LRU caches
 *
 * Copyright (c) 2008-2010 Jean-Pierre Andre
 * Copyright (c)      2016 Martin Pommerenke, Jens Krieg, Arwed Meyer,
 *		           Christian René Sechting
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "antfs.h"
#include "types.h"
#include "cache.h"
#include "dir.h"
#include "misc.h"

/*
 *		General functions to deal with LRU caches
 *
 *	The cached data have to be organized in a structure in which
 *	the first fields must follow a mandatory pattern and further
 *	fields may contain any fixed size data. They are stored in an
 *	LRU list.
 *
 *	A compare function must be provided for finding a wanted entry
 *	in the cache. Another function may be provided for invalidating
 *	an entry to facilitate multiple invalidation.
 *
 *	These functions never return error codes. When there is a
 *	shortage of memory, data is simply not cached.
 *	When there is a hashing bug, hashing is dropped, and sequential
 *	searches are used.
 */

/*
 *		Enter a new hash index, after a new record has been inserted
 *
 *	Do not call when a record has been modified (with no key change)
 */
static void inserthashindex(struct CACHE_HEADER *cache,
			    struct CACHED_GENERIC *curr)
{
	int h;
	struct HASH_ENTRY *link;
	struct HASH_ENTRY *first;

	if (cache->dohash) {
		h = cache->dohash(curr);
		if ((h >= 0) && (h < cache->max_hash)) {
			/* get a free link and insert at top of hash list */
			link = cache->free_hash;
			if (link) {
				cache->free_hash = link->next;
				first = cache->first_hash[h];
				if (first)
					link->next = first;
				else
					link->next = NULL;
				link->entry = curr;
				cache->first_hash[h] = link;
			} else {
				antfs_log_error("No more hash entries,"
						" cache %s hashing dropped\n",
						cache->name);
				cache->dohash = (cache_hash) NULL;
			}
		} else {
			antfs_log_error("Illegal hash value,"
					" cache %s hashing dropped\n",
					cache->name);
			cache->dohash = (cache_hash) NULL;
		}
	}
}

/*
 *		Drop a hash index when a record is about to be deleted
 */
static void drophashindex(struct CACHE_HEADER *cache,
			  const struct CACHED_GENERIC *curr, int hash)
{
	struct HASH_ENTRY *link;
	struct HASH_ENTRY *previous;

	if (cache->dohash) {
		if ((hash >= 0) && (hash < cache->max_hash)) {
			/* find the link and unlink */
			link = cache->first_hash[hash];
			previous = (struct HASH_ENTRY *)NULL;
			while (link && (link->entry != curr)) {
				previous = link;
				link = link->next;
			}
			if (link) {
				if (previous)
					previous->next = link->next;
				else
					cache->first_hash[hash] = link->next;
				link->next = cache->free_hash;
				cache->free_hash = link;
			} else {
				antfs_log_error("Bad hash list,"
						" cache %s hashing dropped\n",
						cache->name);
				cache->dohash = (cache_hash) NULL;
			}
		} else {
			antfs_log_error("Illegal hash value,"
					" cache %s hashing dropped\n",
					cache->name);
			cache->dohash = (cache_hash) NULL;
		}
	}
}

/*
 *		Fetch an entry from cache
 *
 *	returns the cache entry, or NULL if not available
 *	The returned entry may be modified, but not freed
 */
struct CACHED_GENERIC *ntfs_fetch_cache(struct CACHE_HEADER *cache,
					const struct CACHED_GENERIC *wanted,
					cache_compare compare)
{
	struct CACHED_GENERIC *curr;
	struct CACHED_GENERIC *previous;
	struct HASH_ENTRY *link;
	int h;

	curr = (struct CACHED_GENERIC *)NULL;
	if (cache) {
		if (cache->dohash) {
			/*
			 * When possible, use the hash table to
			 * locate the entry if present
			 */
			h = cache->dohash(wanted);
			link = cache->first_hash[h];
			while (link && compare(link->entry, wanted))
				link = link->next;
			if (link)
				curr = link->entry;
		}
		if (!cache->dohash) {
			/*
			 * Search sequentially in LRU list if no hash table
			 * or if hashing has just failed
			 */
			curr = cache->most_recent_entry;
			while (curr && compare(curr, wanted))
				curr = curr->next;
		}
		if (curr) {
			previous = curr->previous;
			cache->hits++;
			if (previous) {
				/*
				 * found and not at head of list, unlink from
				 * curr position and relink as head of list
				 */
				previous->next = curr->next;
				if (curr->next)
					curr->next->previous = curr->previous;
				else
					cache->oldest_entry = curr->previous;
				curr->next = cache->most_recent_entry;
				curr->previous = (struct CACHED_GENERIC *)NULL;
				cache->most_recent_entry->previous = curr;
				cache->most_recent_entry = curr;
			}
		}
		cache->reads++;
	}
	return curr;
}

/*
 *		Enter an inode number into cache
 *
 *	@return cache entry or NULL if not possible
 */
struct CACHED_GENERIC *ntfs_enter_cache(struct CACHE_HEADER *cache,
					const struct CACHED_GENERIC *item,
					cache_compare compare)
{
	struct CACHED_GENERIC *curr;
	struct CACHED_GENERIC *before;
	struct HASH_ENTRY *link;
	int h;

	curr = (struct CACHED_GENERIC *)NULL;
	if (cache) {
		if (cache->dohash) {
			/*
			 * When possible, use the hash table to
			 * find out whether the entry if present
			 */
			h = cache->dohash(item);
			link = cache->first_hash[h];
			while (link && compare(link->entry, item))
				link = link->next;
			if (link)
				curr = link->entry;
		}
		if (!cache->dohash) {
			/*
			 * Search sequentially in LRU list to locate the end,
			 * and find out whether the entry is already in list
			 * As we normally go to the end, no statistics is
			 * kept.
			 */
			curr = cache->most_recent_entry;
			while (curr && compare(curr, item))
				curr = curr->next;
		}

		if (!curr) {
			/*
			 * Not in list, get a free entry or reuse the
			 * last entry, and relink as head of list
			 * Note : we assume at least three entries, so
			 * before, previous and first are different when
			 * an entry is reused.
			 */

			if (cache->free_entry) {
				curr = cache->free_entry;
				cache->free_entry = cache->free_entry->next;
				if (item->varsize) {
					curr->variable =
					    ntfs_malloc(item->varsize);
				} else
					curr->variable = (void *)NULL;
				curr->varsize = item->varsize;
				if (!cache->oldest_entry)
					cache->oldest_entry = curr;
			} else {
				/* reusing the oldest entry */
				curr = cache->oldest_entry;
				before = curr->previous;
				before->next = (struct CACHED_GENERIC *)NULL;
				if (cache->dohash)
					drophashindex(cache, curr,
						      cache->dohash(curr));
				if (cache->dofree)
					cache->dofree(curr);
				cache->oldest_entry = curr->previous;
				if (item->varsize) {
					if (curr->varsize)
						curr->variable =
						    ntfs_realloc(curr->variable,
							    item->varsize);
					else
						curr->variable =
						    ntfs_malloc(item->varsize);
				} else {
					if (curr->varsize)
						ntfs_free(curr->variable);
					curr->variable = (void *)NULL;
				}
				curr->varsize = item->varsize;
			}
			curr->next = cache->most_recent_entry;
			curr->previous = (struct CACHED_GENERIC *)NULL;
			if (cache->most_recent_entry)
				cache->most_recent_entry->previous = curr;
			cache->most_recent_entry = curr;
			memcpy(curr->payload, item->payload, cache->fixed_size);
			if (item->varsize) {
				if (curr->variable) {
					memcpy(curr->variable,
					       item->variable, item->varsize);
				} else {
					/*
					 * no more memory for variable part
					 * recycle entry in free list
					 * not an error, just uncacheable
					 */
					cache->most_recent_entry = curr->next;
					curr->next = cache->free_entry;
					cache->free_entry = curr;
					curr = (struct CACHED_GENERIC *)NULL;
				}
			} else {
				curr->variable = (void *)NULL;
				curr->varsize = 0;
			}
			if (cache->dohash && curr)
				inserthashindex(cache, curr);
		}
		cache->writes++;
	}
	return curr;
}

/*
 *		Invalidate a cache entry
 *	The entry is moved to the free entry list
 *	A specific function may be called for entry deletion
 */
static void do_invalidate(struct CACHE_HEADER *cache,
			  struct CACHED_GENERIC *curr, int flags)
{
	struct CACHED_GENERIC *previous;

	previous = curr->previous;
	if ((flags & CACHE_FREE) && cache->dofree)
		cache->dofree(curr);
	/*
	 * Relink into free list
	 */
	if (curr->next)
		curr->next->previous = curr->previous;
	else
		cache->oldest_entry = curr->previous;
	if (previous)
		previous->next = curr->next;
	else
		cache->most_recent_entry = curr->next;
	curr->next = cache->free_entry;
	cache->free_entry = curr;
	if (curr->variable)
		ntfs_free(curr->variable);
	curr->varsize = 0;
}

/*
 *		Invalidate entries in cache
 *
 *	Several entries may have to be invalidated (at least for inodes
 *	associated to directories which have been renamed), a different
 *	compare function may be provided to select entries to invalidate
 *
 *	Returns the number of deleted entries, this can be used by
 *	the caller to signal a cache corruption if the entry was
 *	supposed to be found.
 */
int ntfs_invalidate_cache(struct CACHE_HEADER *cache,
			  const struct CACHED_GENERIC *item,
			  cache_compare compare, int flags)
{
	struct CACHED_GENERIC *curr;
	struct CACHED_GENERIC *next;
	struct HASH_ENTRY *link;
	int count;
	int h;

	curr = (struct CACHED_GENERIC *)NULL;
	count = 0;
	if (cache) {
		if (!(flags & CACHE_NOHASH) && cache->dohash) {
			/*
			 * When possible, use the hash table to
			 * find out whether the entry if present
			 */
			h = cache->dohash(item);
			link = cache->first_hash[h];
			while (link) {
				if (compare(link->entry, item))
					link = link->next;
				else {
					curr = link->entry;
					link = link->next;
					if (curr) {
						drophashindex(cache, curr, h);
						do_invalidate(cache,
							      curr, flags);
						count++;
					}
				}
			}
		}
		if ((flags & CACHE_NOHASH) || !cache->dohash) {
			/*
			 * Search sequentially in LRU list
			 */
			curr = cache->most_recent_entry;
			while (curr) {
				if (!compare(curr, item)) {
					next = curr->next;
					if (cache->dohash)
						drophashindex(cache, curr,
							      cache->
							      dohash(curr));
					do_invalidate(cache, curr, flags);
					curr = next;
					count++;
				} else {
					curr = curr->next;
				}
			}
		}
	}
	return count;
}

int ntfs_remove_cache(struct CACHE_HEADER *cache,
		      struct CACHED_GENERIC *item, int flags)
{
	int count;

	count = 0;
	if (cache) {
		if (cache->dohash)
			drophashindex(cache, item, cache->dohash(item));
		do_invalidate(cache, item, flags);
		count++;
	}
	return count;
}
