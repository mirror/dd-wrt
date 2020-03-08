// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <linux/buffer_head.h>
#include <linux/slab.h>
#include "apfs.h"

/**
 * apfs_extent_from_query - Read the extent found by a successful query
 * @query:	the query that found the record
 * @extent:	Return parameter.  The extent found.
 *
 * Reads the extent record into @extent and performs some basic sanity checks
 * as a protection against crafted filesystems.  Returns 0 on success or
 * -EFSCORRUPTED otherwise.
 */
int apfs_extent_from_query(struct apfs_query *query,
			   struct apfs_file_extent *extent)
{
	struct super_block *sb = query->node->object.sb;
	struct apfs_file_extent_val *ext;
	struct apfs_file_extent_key *ext_key;
	char *raw = query->node->object.bh->b_data;
	u64 ext_len;

	if (query->len != sizeof(*ext) || query->key_len != sizeof(*ext_key))
		return -EFSCORRUPTED;

	ext = (struct apfs_file_extent_val *)(raw + query->off);
	ext_key = (struct apfs_file_extent_key *)(raw + query->key_off);
	ext_len = le64_to_cpu(ext->len_and_flags) & APFS_FILE_EXTENT_LEN_MASK;

	/* Extent length must be a multiple of the block size */
	if (ext_len & (sb->s_blocksize - 1))
		return -EFSCORRUPTED;

	extent->logical_addr = le64_to_cpu(ext_key->logical_addr);
	extent->phys_block_num = le64_to_cpu(ext->phys_block_num);
	extent->len = ext_len;
	return 0;
}

/**
 * apfs_extent_read - Read the extent record that covers a block
 * @inode:	inode that owns the record
 * @iblock:	logical number of the wanted block
 * @extent:	Return parameter.  The extent found.
 *
 * Finds and caches the extent record.  On success, returns a pointer to the
 * cache record; on failure, returns an error code.
 */
static int apfs_extent_read(struct inode *inode, sector_t iblock,
			    struct apfs_file_extent *extent)
{
	struct super_block *sb = inode->i_sb;
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_inode_info *ai = APFS_I(inode);
	struct apfs_key key;
	struct apfs_query *query;
	struct apfs_file_extent *cache = &ai->i_cached_extent;
	u64 iaddr = iblock << inode->i_blkbits;
	int ret = 0;

	spin_lock(&ai->i_extent_lock);
	if (iaddr >= cache->logical_addr &&
	    iaddr < cache->logical_addr + cache->len) {
		*extent = *cache;
		spin_unlock(&ai->i_extent_lock);
		return 0;
	}
	spin_unlock(&ai->i_extent_lock);

	/* We will search for the extent that covers iblock */
	apfs_init_file_extent_key(ai->i_extent_id, iaddr, &key);

	query = apfs_alloc_query(sbi->s_cat_root, NULL /* parent */);
	if (!query)
		return -ENOMEM;
	query->key = &key;
	query->flags = APFS_QUERY_CAT;

	ret = apfs_btree_query(sb, &query);
	if (ret)
		goto done;

	ret = apfs_extent_from_query(query, extent);
	if (ret) {
		apfs_alert(sb, "bad extent record for inode 0x%llx",
			   apfs_ino(inode));
		goto done;
	}

	spin_lock(&ai->i_extent_lock);
	*cache = *extent;
	spin_unlock(&ai->i_extent_lock);

done:
	apfs_free_query(sb, query);
	return ret;
}

int apfs_get_block(struct inode *inode, sector_t iblock,
		   struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	struct apfs_sb_info *sbi = APFS_SB(sb);
	struct apfs_file_extent ext;
	u64 blk_off, bno, map_len;
	int ret;

	down_read(&sbi->s_big_sem);

	ret = apfs_extent_read(inode, iblock, &ext);
	if (ret)
		goto fail;

	/* Find the block offset of iblock within the extent */
	blk_off = iblock - (ext.logical_addr >> inode->i_blkbits);

	/* Make sure we don't read past the extent boundaries */
	map_len = ext.len - (blk_off << inode->i_blkbits);
	if (bh_result->b_size > map_len)
		bh_result->b_size = map_len;

	/*
	 * Save the requested mapping length as map_bh() replaces it with
	 * the filesystem block size
	 */
	map_len = bh_result->b_size;
	/* Extents representing holes have block number 0 */
	if (ext.phys_block_num != 0) {
		/* Find the block number of iblock within the disk */
		bno = ext.phys_block_num + blk_off;
		map_bh(bh_result, sb, bno);
	}
	bh_result->b_size = map_len;

fail:
	up_read(&sbi->s_big_sem);
	return ret;
}
