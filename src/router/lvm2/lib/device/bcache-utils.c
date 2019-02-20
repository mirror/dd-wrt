/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/device/bcache.h"

// FIXME: need to define this in a common place (that doesn't pull in deps)
#ifndef SECTOR_SHIFT
#define SECTOR_SHIFT 9
#endif

//----------------------------------------------------------------

static void byte_range_to_block_range(struct bcache *cache, uint64_t start, size_t len,
				      block_address *bb, block_address *be)
{
	block_address block_size = bcache_block_sectors(cache) << SECTOR_SHIFT;
	*bb = start / block_size;
	*be = (start + len + block_size - 1) / block_size;
}

static uint64_t _min(uint64_t lhs, uint64_t rhs)
{
	if (rhs < lhs)
		return rhs;

	return lhs;
}

//----------------------------------------------------------------

void bcache_prefetch_bytes(struct bcache *cache, int fd, uint64_t start, size_t len)
{
	block_address bb, be;

	byte_range_to_block_range(cache, start, len, &bb, &be);
	while (bb < be) {
		bcache_prefetch(cache, fd, bb);
		bb++;
	}
}

//----------------------------------------------------------------

bool bcache_read_bytes(struct bcache *cache, int fd, uint64_t start, size_t len, void *data)
{
	struct block *b;
	block_address bb, be;
	uint64_t block_size = bcache_block_sectors(cache) << SECTOR_SHIFT;
	uint64_t block_offset = start % block_size;

	bcache_prefetch_bytes(cache, fd, start, len);

	byte_range_to_block_range(cache, start, len, &bb, &be);

	for (; bb != be; bb++) {
        	if (!bcache_get(cache, fd, bb, 0, &b))
			return false;

		size_t blen = _min(block_size - block_offset, len);
		memcpy(data, ((unsigned char *) b->data) + block_offset, blen);
		bcache_put(b);

		block_offset = 0;
		len -= blen;
		data = ((unsigned char *) data) + blen;
	}

	return true;
}

//----------------------------------------------------------------

// Writing bytes and zeroing bytes are very similar, so we factor out
// this common code.
 
struct updater;

typedef bool (*partial_update_fn)(struct updater *u, int fd, block_address bb, uint64_t offset, size_t len);
typedef bool (*whole_update_fn)(struct updater *u, int fd, block_address bb, block_address be);

struct updater {
	struct bcache *cache;
	partial_update_fn partial_fn;
	whole_update_fn whole_fn;
	void *data;
};

static bool _update_bytes(struct updater *u, int fd, uint64_t start, size_t len)
{
        struct bcache *cache = u->cache;
	block_address bb, be;
	uint64_t block_size = bcache_block_sectors(cache) << SECTOR_SHIFT;
	uint64_t block_offset = start % block_size;
	uint64_t nr_whole;

	byte_range_to_block_range(cache, start, len, &bb, &be);

	// If the last block is partial, we will require a read, so let's 
	// prefetch it.
	if ((start + len) % block_size)
        	bcache_prefetch(cache, fd, (start + len) / block_size);

	// First block may be partial
	if (block_offset) {
        	size_t blen = _min(block_size - block_offset, len);
		if (!u->partial_fn(u, fd, bb, block_offset, blen))
        		return false;

		len -= blen;
        	if (!len)
                	return true;

                bb++;
	}

        // Now we write out a set of whole blocks
        nr_whole = len / block_size;
        if (!u->whole_fn(u, fd, bb, bb + nr_whole))
                return false;

	bb += nr_whole;
	len -= nr_whole * block_size;

	if (!len)
        	return true;

        // Finally we write a partial end block
        return u->partial_fn(u, fd, bb, 0, len);
}

//----------------------------------------------------------------

static bool _write_partial(struct updater *u, int fd, block_address bb,
                           uint64_t offset, size_t len)
{
	struct block *b;

	if (!bcache_get(u->cache, fd, bb, GF_DIRTY, &b))
		return false;

	memcpy(((unsigned char *) b->data) + offset, u->data, len);
	u->data = ((unsigned char *) u->data) + len;

	bcache_put(b);
	return true;
}

static bool _write_whole(struct updater *u, int fd, block_address bb, block_address be)
{
	struct block *b;
	uint64_t block_size = bcache_block_sectors(u->cache) << SECTOR_SHIFT;

	for (; bb != be; bb++) {
        	// We don't need to read the block since we are overwriting
        	// it completely.
		if (!bcache_get(u->cache, fd, bb, GF_ZERO, &b))
        		return false;
		memcpy(b->data, u->data, block_size);
		u->data = ((unsigned char *) u->data) + block_size;
        	bcache_put(b);
	}

	return true;
}

bool bcache_write_bytes(struct bcache *cache, int fd, uint64_t start, size_t len, void *data)
{
        struct updater u;

        u.cache = cache;
        u.partial_fn = _write_partial;
        u.whole_fn = _write_whole;
        u.data = data;

	return _update_bytes(&u, fd, start, len);
}

//----------------------------------------------------------------

static bool _zero_partial(struct updater *u, int fd, block_address bb, uint64_t offset, size_t len)
{
	struct block *b;

	if (!bcache_get(u->cache, fd, bb, GF_DIRTY, &b))
		return false;

	memset(((unsigned char *) b->data) + offset, 0, len);
	bcache_put(b);

	return true;
}

static bool _zero_whole(struct updater *u, int fd, block_address bb, block_address be)
{
	struct block *b;

	for (; bb != be; bb++) {
		if (!bcache_get(u->cache, fd, bb, GF_ZERO, &b))
        		return false;
        	bcache_put(b);
	}

	return true;
}

bool bcache_zero_bytes(struct bcache *cache, int fd, uint64_t start, size_t len)
{
        struct updater u;

        u.cache = cache;
        u.partial_fn = _zero_partial;
        u.whole_fn = _zero_whole;
        u.data = NULL;

	return _update_bytes(&u, fd, start, len);
}

//----------------------------------------------------------------

static bool _set_partial(struct updater *u, int fd, block_address bb, uint64_t offset, size_t len)
{
	struct block *b;
	uint8_t val = *((uint8_t *) u->data);

	if (!bcache_get(u->cache, fd, bb, GF_DIRTY, &b))
		return false;

	memset(((unsigned char *) b->data) + offset, val, len);
	bcache_put(b);

	return true;
}

static bool _set_whole(struct updater *u, int fd, block_address bb, block_address be)
{
	struct block *b;
	uint8_t val = *((uint8_t *) u->data);
        uint64_t len = bcache_block_sectors(u->cache) * 512;

	for (; bb != be; bb++) {
		if (!bcache_get(u->cache, fd, bb, GF_ZERO, &b))
        		return false;
        	memset((unsigned char *) b->data, val, len);
        	bcache_put(b);
	}

	return true;
}

bool bcache_set_bytes(struct bcache *cache, int fd, uint64_t start, size_t len, uint8_t val)
{
        struct updater u;

        u.cache = cache;
        u.partial_fn = _set_partial;
        u.whole_fn = _set_whole;
        u.data = &val;

	return _update_bytes(&u, fd, start, len);
}

