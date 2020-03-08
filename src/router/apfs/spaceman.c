// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <linux/buffer_head.h>
#include <linux/fs.h>
#include "apfs.h"


/**
 * apfs_spaceman_read_cib_addr - Get the address of a cib from the spaceman
 * @sb:		superblock structure
 * @index:	index of the chunk-info block
 *
 * Returns the block number for the chunk-info block.
 *
 * This is not described in the official documentation; credit for figuring it
 * out should go to Joachim Metz: <https://github.com/libyal/libfsapfs>.
 */
static u64 apfs_spaceman_read_cib_addr(struct super_block *sb, int index)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_spaceman *sm = &sbi->s_spaceman;
	struct apfs_spaceman_phys *sm_raw = sm->sm_raw;
	u32 offset;
	u64 *addr_p;

	offset = sm->sm_addr_offset + index * sizeof(*addr_p);
	addr_p = (void *)sm_raw + offset;
	return *addr_p;
}

/**
 * apfs_spaceman_write_cib_addr - Store the address of a cib in the spaceman
 * @sb:		superblock structure
 * @index:	index of the chunk-info block
 * @addr:	address of the chunk-info block
 */
static void apfs_spaceman_write_cib_addr(struct super_block *sb,
					 int index, u64 addr)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_spaceman *sm = &sbi->s_spaceman;
	struct apfs_spaceman_phys *sm_raw = sm->sm_raw;
	u32 offset;
	u64 *addr_p;

	ASSERT(le64_to_cpu(sm_raw->sm_o.o_xid) == sbi->s_xid);

	offset = sm->sm_addr_offset + index * sizeof(*addr_p);
	addr_p = (void *)sm_raw + offset;
	*addr_p = addr;
}

/**
 * apfs_max_chunks_per_cib - Find the maximum chunk count for a chunk-info block
 * @sb: superblock structure
 */
static inline int apfs_max_chunks_per_cib(struct super_block *sb)
{
	return (sb->s_blocksize - sizeof(struct apfs_chunk_info_block)) /
						sizeof(struct apfs_chunk_info);
}

/**
 * apfs_read_spaceman_dev - Read a space manager device structure
 * @sb:		superblock structure
 * @dev:	on-disk device structure
 *
 * Initializes the in-memory spaceman fields related to the main device; fusion
 * drives are not yet supported.  Returns 0 on success, or a negative error code
 * in case of failure.
 */
static int apfs_read_spaceman_dev(struct super_block *sb,
				  struct apfs_spaceman_device *dev)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_spaceman *spaceman = &sbi->s_spaceman;

	if (dev->sm_cab_count) {
		apfs_err(sb, "large devices are not supported");
		return -EINVAL;
	}

	spaceman->sm_block_count = le64_to_cpu(dev->sm_block_count);
	spaceman->sm_chunk_count = le64_to_cpu(dev->sm_chunk_count);
	spaceman->sm_cib_count = le32_to_cpu(dev->sm_cib_count);
	spaceman->sm_free_count = le64_to_cpu(dev->sm_free_count);
	spaceman->sm_addr_offset = le32_to_cpu(dev->sm_addr_offset);

	/* Check that all the cib addresses fit in the spaceman block */
	if ((long long)spaceman->sm_addr_offset +
	    (long long)spaceman->sm_cib_count * sizeof(u64) > sb->s_blocksize)
		return -EFSCORRUPTED;

	return 0;
}

/**
 * apfs_read_spaceman - Find and read the space manager
 * @sb: superblock structure
 *
 * Reads the space manager structure from disk and initializes its in-memory
 * counterpart; returns 0 on success, or a negative error code in case of
 * failure.
 */
int apfs_read_spaceman(struct super_block *sb)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_nx_superblock *raw_sb = sbi->s_msb_raw;
	struct apfs_spaceman *spaceman = &sbi->s_spaceman;
	struct buffer_head *sm_bh;
	struct apfs_spaceman_phys *sm_raw;
	u32 sm_flags;
	u64 oid = le64_to_cpu(raw_sb->nx_spaceman_oid);
	int err;

	if (sb->s_flags & SB_RDONLY) /* The space manager won't be needed */
		return 0;

	sm_bh = apfs_read_ephemeral_object(sb, oid);
	if (IS_ERR(sm_bh))
		return PTR_ERR(sm_bh);
	sm_raw = (struct apfs_spaceman_phys *)sm_bh->b_data;

	if (sbi->s_flags & APFS_CHECK_NODES &&
	    !apfs_obj_verify_csum(sb, &sm_raw->sm_o)) {
		apfs_err(sb, "bad checksum for the space manager");
		err = -EFSBADCRC;
		goto fail;
	}

	sm_flags = le32_to_cpu(sm_raw->sm_flags);
	if (sm_flags & APFS_SM_FLAG_VERSIONED) {
		apfs_err(sb, "versioned space manager not supported");
		err = -EINVAL;
		goto fail;
	}
	/* Some fields are missing in the non-versioned structure */
	spaceman->sm_struct_size = sizeof(*sm_raw) -
				   sizeof(sm_raw->sm_datazone) -
				   sizeof(sm_raw->sm_struct_size) -
				   sizeof(sm_raw->sm_version);

	/* Only read the main device; fusion drives are not yet supported */
	err = apfs_read_spaceman_dev(sb, &sm_raw->sm_dev[APFS_SD_MAIN]);
	if (err)
		goto fail;

	spaceman->sm_blocks_per_chunk =
				le32_to_cpu(sm_raw->sm_blocks_per_chunk);
	spaceman->sm_chunks_per_cib = le32_to_cpu(sm_raw->sm_chunks_per_cib);
	if (spaceman->sm_chunks_per_cib > apfs_max_chunks_per_cib(sb)) {
		err = -EFSCORRUPTED;
		goto fail;
	}

	spaceman->sm_bh = sm_bh;
	spaceman->sm_raw = sm_raw;
	return 0;

fail:
	brelse(sm_bh);
	return err;
}

/**
 * apfs_write_spaceman - Write the in-memory spaceman fields to the disk buffer
 * @sm: in-memory spaceman structure
 *
 * Copies the updated in-memory fields of the space manager into the on-disk
 * structure; the buffer is not dirtied.
 */
static void apfs_write_spaceman(struct apfs_spaceman *sm)
{
	struct apfs_spaceman_phys *sm_raw = sm->sm_raw;
	struct apfs_spaceman_device *dev_raw = &sm_raw->sm_dev[APFS_SD_MAIN];
	struct apfs_sb_info *sbi;

	sbi = container_of(sm, struct apfs_sb_info, s_spaceman);
	ASSERT(le64_to_cpu(sm_raw->sm_o.o_xid) == sbi->s_xid);

	dev_raw->sm_free_count = cpu_to_le64(sm->sm_free_count);
}

/**
 * apfs_chunk_find_free - Find a free block inside a chunk
 * @sb:		superblock structure
 * @bitmap:	allocation bitmap for the chunk, which should have free blocks
 * @addr:	number of the first block in the chunk
 *
 * Returns the block number for a free block, or 0 in case of corruption.
 */
static u64 apfs_chunk_find_free(struct super_block *sb, char *bitmap, u64 addr)
{
	int bitcount = sb->s_blocksize * 8;
	u64 bno;

	bno = find_next_zero_bit_le(bitmap, bitcount, 0 /* offset */);
	if (bno >= bitcount)
		return 0;
	return addr + bno;
}

/**
 * apfs_chunk_mark_used - Mark a block inside a chunk as used
 * @sb:		superblock structure
 * @bitmap:	allocation bitmap for the chunk
 * @bno:	block number (must belong to the chunk)
 */
static inline void apfs_chunk_mark_used(struct super_block *sb, char *bitmap,
					u64 bno)
{
	int bitcount = sb->s_blocksize * 8;

	__set_bit_le(bno & (bitcount - 1), bitmap);
}

/**
 * apfs_free_queue_insert - Add a block to the free queue
 * @sb:		superblock structure
 * @bno:	block number to free
 *
 * Returns 0 on success or a negative error code in case of failure.
 */
int apfs_free_queue_insert(struct super_block *sb, u64 bno)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_spaceman *sm = &sbi->s_spaceman;
	struct apfs_spaceman_phys *sm_raw = sm->sm_raw;
	struct apfs_spaceman_free_queue *fq = &sm_raw->sm_fq[APFS_SFQ_MAIN];
	struct apfs_node *fq_root;
	struct apfs_query *query = NULL;
	struct apfs_spaceman_free_queue_key raw_key;
	struct apfs_key key;
	int err;

	fq_root = apfs_read_node(sb, le64_to_cpu(fq->sfq_tree_oid),
				 APFS_OBJ_EPHEMERAL, true /* write */);
	if (IS_ERR(fq_root))
		return PTR_ERR(fq_root);

	query = apfs_alloc_query(fq_root, NULL /* parent */);
	if (!query) {
		err = -ENOMEM;
		goto fail;
	}

	apfs_init_free_queue_key(sbi->s_xid, bno, &key);
	query->key = &key;
	query->flags |= APFS_QUERY_FREE_QUEUE;

	err = apfs_btree_query(sb, &query);
	if (err && err != -ENODATA)
		goto fail;

	raw_key.sfqk_xid = cpu_to_le64(sbi->s_xid);
	raw_key.sfqk_paddr = cpu_to_le64(bno);
	/* A lack of value (ghost record) implies a single-block extent */
	err = apfs_btree_insert(query, &raw_key, sizeof(raw_key),
				NULL /* val */, 0 /* val_len */);
	if (err)
		goto fail;

	if (!fq->sfq_oldest_xid)
		fq->sfq_oldest_xid = cpu_to_le64(sbi->s_xid);
	le64_add_cpu(&fq->sfq_count, 1);
	apfs_obj_set_csum(sb, &sm_raw->sm_o);

fail:
	apfs_free_query(sb, query);
	apfs_node_put(fq_root);
	return err;
}

/**
 * apfs_chunk_allocate_block - Allocate a single block from a chunk
 * @sb:		superblock structure
 * @cib_bh:	buffer head for the chunk-info block
 * @index:	index of this chunk's info structure inside @cib
 * @bno:	on return, the allocated block number
 *
 * Finds a free block in the chunk and marks it as used; the buffer at @cib_bh
 * may be replaced if needed for copy-on-write.  Returns 0 on success, or a
 * negative error code in case of failure.
 */
static int apfs_chunk_allocate_block(struct super_block *sb,
				     struct buffer_head **cib_bh,
				     int index, u64 *bno)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_spaceman *sm = &sbi->s_spaceman;
	struct apfs_chunk_info_block *cib;
	struct apfs_chunk_info *ci;
	struct buffer_head *bmap_bh = NULL;
	char *bmap = NULL;
	bool old_cib = false;
	bool old_bmap = false;
	int blocks_needed;
	int err = 0;

	cib = (struct apfs_chunk_info_block *)(*cib_bh)->b_data;
	ci = &cib->cib_chunk_info[index];

	/* Cibs and bitmaps from old transactions can't be modified in place */
	if (le64_to_cpu(cib->cib_o.o_xid) < sbi->s_xid)
		old_cib = true;
	if (le64_to_cpu(ci->ci_xid) < sbi->s_xid)
		old_bmap = true;
	/* Allocate the new cib and bitmap in the same chunk */
	blocks_needed = 1 + old_cib + old_bmap;
	if (le32_to_cpu(ci->ci_free_count) < blocks_needed)
		return -ENOSPC;

	/* Read the current bitmap, or allocate it if necessary */
	if (!ci->ci_bitmap_addr) /* All blocks in this chunk are free */
		bmap_bh = sb_bread(sb, le64_to_cpu(ci->ci_addr));
	else
		bmap_bh = sb_bread(sb, le64_to_cpu(ci->ci_bitmap_addr));
	if (!bmap_bh)
		return -EIO;
	bmap = bmap_bh->b_data;
	if (!ci->ci_bitmap_addr) {
		memset(bmap, 0, sb->s_blocksize);
		old_bmap = false;
	}

	/* Write the bitmap to its location for the next transaction */
	if (old_bmap) {
		struct buffer_head *new_bmap_bh;
		u64 new_bmap_bno;

		new_bmap_bno = apfs_chunk_find_free(sb, bmap,
						    le64_to_cpu(ci->ci_addr));
		if (!new_bmap_bno) {
			err = -EFSCORRUPTED;
			goto fail;
		}

		new_bmap_bh = sb_bread(sb, new_bmap_bno);
		if (!new_bmap_bh) {
			err = -EIO;
			goto fail;
		}
		memcpy(new_bmap_bh->b_data, bmap, sb->s_blocksize);
		err = apfs_free_queue_insert(sb, bmap_bh->b_blocknr);
		brelse(bmap_bh);
		bmap_bh = new_bmap_bh;
		if (err)
			goto fail;
		bmap = bmap_bh->b_data;
	}
	apfs_chunk_mark_used(sb, bmap, bmap_bh->b_blocknr);

	/* Write the cib to its location for the next transaction */
	if (old_cib) {
		struct buffer_head *new_cib_bh;
		u64 new_cib_bno;

		new_cib_bno = apfs_chunk_find_free(sb, bmap,
						   le64_to_cpu(ci->ci_addr));
		if (!new_cib_bno) {
			err = -EFSCORRUPTED;
			goto fail;
		}

		new_cib_bh = sb_bread(sb, new_cib_bno);
		if (!new_cib_bh) {
			err = -EIO;
			goto fail;
		}
		memcpy(new_cib_bh->b_data, (*cib_bh)->b_data, sb->s_blocksize);
		err = apfs_free_queue_insert(sb, (*cib_bh)->b_blocknr);
		brelse(*cib_bh);
		*cib_bh = new_cib_bh;
		if (err)
			goto fail;

		cib = (struct apfs_chunk_info_block *)(*cib_bh)->b_data;
		ci = &cib->cib_chunk_info[index];
		cib->cib_o.o_oid = cpu_to_le64(new_cib_bno);
		cib->cib_o.o_xid = cpu_to_le64(sbi->s_xid);

		apfs_chunk_mark_used(sb, bmap, new_cib_bno);
	}

	/* The chunk info can be updated now */
	ASSERT(le64_to_cpu(cib->cib_o.o_xid) == sbi->s_xid);
	ci->ci_xid = cpu_to_le64(sbi->s_xid);
	le32_add_cpu(&ci->ci_free_count, -blocks_needed);
	ci->ci_bitmap_addr = cpu_to_le64(bmap_bh->b_blocknr);
	apfs_obj_set_csum(sb, &cib->cib_o);
	mark_buffer_dirty(*cib_bh);

	/* Finally, allocate the actual block that was requested */
	*bno = apfs_chunk_find_free(sb, bmap, le64_to_cpu(ci->ci_addr));
	if (!*bno) {
		err = -EFSCORRUPTED;
		goto fail;
	}
	apfs_chunk_mark_used(sb, bmap, *bno);
	mark_buffer_dirty(bmap_bh);
	sm->sm_free_count -= blocks_needed;

fail:
	brelse(bmap_bh);
	return err;
}

/**
 * apfs_cib_allocate_block - Allocate a single block from a cib
 * @sb:		superblock structure
 * @cib_bh:	buffer head for the chunk-info block
 * @bno:	on return, the allocated block number
 *
 * Finds a free block among all the chunks in the cib and marks it as used; the
 * buffer at @cib_bh may be replaced if needed for copy-on-write.  Returns 0 on
 * success, or a negative error code in case of failure.
 */
static int apfs_cib_allocate_block(struct super_block *sb,
				   struct buffer_head **cib_bh, u64 *bno)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_spaceman *sm = &sbi->s_spaceman;
	struct apfs_chunk_info_block *cib;
	u32 chunk_count;
	int i;

	cib = (struct apfs_chunk_info_block *)(*cib_bh)->b_data;
	if (sbi->s_flags & APFS_CHECK_NODES &&
	    !apfs_obj_verify_csum(sb, &cib->cib_o)) {
		apfs_err(sb, "bad checksum for chunk-info block");
		return -EFSBADCRC;
	}

	/* Avoid out-of-bounds operations on corrupted cibs */
	chunk_count = le32_to_cpu(cib->cib_chunk_info_count);
	if (chunk_count > sm->sm_chunks_per_cib)
		return -EFSCORRUPTED;

	for (i = 0; i < chunk_count; ++i) {
		int err;

		err = apfs_chunk_allocate_block(sb, cib_bh, i, bno);
		if (err == -ENOSPC) /* This chunk is full */
			continue;
		return err;
	}
	return -ENOSPC;
}

/**
 * apfs_spaceman_allocate_block - Allocate a single on-disk block
 * @sb:		superblock structure
 * @bno:	on return, the allocated block number
 *
 * Finds a free block among the spaceman bitmaps and marks it as used.  Returns
 * 0 on success, or a negative error code in case of failure.
 */
int apfs_spaceman_allocate_block(struct super_block *sb, u64 *bno)
{
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_spaceman *sm = &sbi->s_spaceman;
	struct apfs_spaceman_phys *sm_raw = sm->sm_raw;
	int i;

	for (i = 0; i < sm->sm_cib_count; ++i) {
		struct buffer_head *cib_bh;
		u64 cib_bno;
		int err;

		cib_bno = apfs_spaceman_read_cib_addr(sb, i);
		cib_bh = sb_bread(sb, cib_bno);
		if (!cib_bh)
			return -EIO;

		err = apfs_cib_allocate_block(sb, &cib_bh, bno);
		if (!err) {
			/* The cib may have been moved */
			apfs_spaceman_write_cib_addr(sb, i, cib_bh->b_blocknr);
			/* The free block count has changed */
			apfs_write_spaceman(sm);
			apfs_obj_set_csum(sb, &sm_raw->sm_o);
		}
		brelse(cib_bh);
		if (err == -ENOSPC) /* This cib is full */
			continue;
		return err;
	}
	return -ENOSPC;
}
