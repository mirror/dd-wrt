/**
 * segment.c
 *
 * Many parts of codes are copied from Linux kernel/fs/f2fs.
 *
 * Copyright (C) 2015 Huawei Ltd.
 * Witten by:
 *   Hou Pengyang <houpengyang@huawei.com>
 *   Liu Shuoran <liushuoran@huawei.com>
 *   Jaegeuk Kim <jaegeuk@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "fsck.h"
#include "node.h"

void reserve_new_block(struct f2fs_sb_info *sbi, block_t *to,
			struct f2fs_summary *sum, int type)
{
	struct seg_entry *se;
	u64 blkaddr;
	u64 offset;

	blkaddr = SM_I(sbi)->main_blkaddr;

	if (find_next_free_block(sbi, &blkaddr, 0, type)) {
		ERR_MSG("Not enough space to allocate blocks");
		ASSERT(0);
	}

	se = get_seg_entry(sbi, GET_SEGNO(sbi, blkaddr));
	offset = OFFSET_IN_SEG(sbi, blkaddr);
	se->type = type;
	se->valid_blocks++;
	f2fs_set_bit(offset, (char *)se->cur_valid_map);
	sbi->total_valid_block_count++;
	se->dirty = 1;

	/* read/write SSA */
	*to = (block_t)blkaddr;
	update_sum_entry(sbi, *to, sum);
}

void new_data_block(struct f2fs_sb_info *sbi, void *block,
				struct dnode_of_data *dn, int type)
{
	struct f2fs_summary sum;
	struct node_info ni;

	ASSERT(dn->node_blk);
	memset(block, 0, BLOCK_SZ);

	get_node_info(sbi, dn->nid, &ni);
	set_summary(&sum, dn->nid, dn->ofs_in_node, ni.version);
	reserve_new_block(sbi, &dn->data_blkaddr, &sum, type);

	inc_inode_blocks(dn);
	set_data_blkaddr(dn);
}

static void f2fs_write_block(struct f2fs_sb_info *sbi, nid_t ino, void *buffer,
					u64 count, pgoff_t offset)
{
	u64 start = F2FS_BYTES_TO_BLK(offset);
	u64 len = F2FS_BYTES_TO_BLK(count);
	u64 end_offset;
	u64 off_in_block, len_in_block, len_already;
	struct dnode_of_data dn = {0};
	void *data_blk;
	struct node_info ni;
	struct f2fs_node *inode;
	int ret = -1;

	get_node_info(sbi, ino, &ni);
	inode = calloc(BLOCK_SZ, 1);
	ASSERT(inode);

	ret = dev_read_block(inode, ni.blk_addr);
	ASSERT(ret >= 0);

	if (S_ISDIR(le16_to_cpu(inode->i.i_mode)) ||
			S_ISLNK(le16_to_cpu(inode->i.i_mode)))
		ASSERT(0);

	off_in_block = offset & ((1 << F2FS_BLKSIZE_BITS) - 1);
	len_in_block = (1 << F2FS_BLKSIZE_BITS) - off_in_block;
	len_already = 0;

	/*
	 * When calculate how many blocks this 'count' stride accross,
	 * We should take offset in a block in account.
	 */
	len = F2FS_BYTES_TO_BLK(count + off_in_block
			+ ((1 << F2FS_BLKSIZE_BITS) - 1));

	data_blk = calloc(BLOCK_SZ, 1);
	ASSERT(data_blk);

	set_new_dnode(&dn, inode, NULL, ino);

	while (len) {
		if (dn.node_blk != dn.inode_blk)
			free(dn.node_blk);

		set_new_dnode(&dn, inode, NULL, ino);
		get_dnode_of_data(sbi, &dn, start, ALLOC_NODE);

		end_offset = ADDRS_PER_PAGE(dn.node_blk);

		while (dn.ofs_in_node < end_offset && len) {
			block_t blkaddr;

			blkaddr = datablock_addr(dn.node_blk, dn.ofs_in_node);

			/* A new page from WARM_DATA */
			if (blkaddr == NULL_ADDR)
				new_data_block(sbi, data_blk, &dn,
							CURSEG_WARM_DATA);

			/* Copy data from buffer to file */
			ret = dev_read_block(data_blk, dn.data_blkaddr);
			ASSERT(ret >= 0);

			memcpy(data_blk + off_in_block, buffer, len_in_block);

			ret = dev_write_block(data_blk, dn.data_blkaddr);
			ASSERT(ret >= 0);

			off_in_block = 0;
			len_already += len_in_block;
			if ((count - len_already) > (1 << F2FS_BLKSIZE_BITS))
				len_in_block = 1 << F2FS_BLKSIZE_BITS;
			else
				len_in_block = count - len_already;
			len--;
			start++;
			dn.ofs_in_node++;
		}
		/* Update the direct node */
		if (dn.ndirty) {
			ret = dev_write_block(dn.node_blk, dn.node_blkaddr);
			ASSERT(ret >= 0);
		}
	}

	/* Update the inode info */
	if (le64_to_cpu(inode->i.i_size) < offset + count) {
		inode->i.i_size = cpu_to_le64(offset + count);
		dn.idirty = 1;
	}

	if (dn.idirty) {
		ASSERT(inode == dn.inode_blk);
		ret = dev_write_block(inode, ni.blk_addr);
		ASSERT(ret >= 0);
	}

	if (dn.node_blk && dn.node_blk != dn.inode_blk)
		free(dn.node_blk);
	free(data_blk);
	free(inode);
}

int f2fs_build_file(struct f2fs_sb_info *sbi, struct dentry *de)
{
	int fd, n;
	pgoff_t off = 0;
	char buffer[BLOCK_SZ];

	if (de->ino == 0)
		return -1;

	fd = open(de->full_path, O_RDONLY);
	if (fd < 0) {
		MSG(0, "Skip: Fail to open %s\n", de->full_path);
		return -1;
	}

	/* inline_data support */
	if (de->size <= MAX_INLINE_DATA) {
		struct node_info ni;
		struct f2fs_node *node_blk;
		int ret;

		get_node_info(sbi, de->ino, &ni);

		node_blk = calloc(BLOCK_SZ, 1);
		ASSERT(node_blk);

		ret = dev_read_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);

		node_blk->i.i_inline |= F2FS_INLINE_DATA;
		node_blk->i.i_inline |= F2FS_DATA_EXIST;
		n = read(fd, buffer, BLOCK_SZ);
		ASSERT(n == de->size);
		memcpy(&node_blk->i.i_addr[1], buffer, de->size);

		node_blk->i.i_size = cpu_to_le64(de->size);

		ret = dev_write_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
		free(node_blk);
	} else {
		while ((n = read(fd, buffer, BLOCK_SZ)) > 0) {
			f2fs_write_block(sbi, de->ino, buffer, n, off);
			off += n;
		}
	}

	close(fd);
	if (n < 0)
		return -1;

	update_free_segments(sbi);

	MSG(1, "Info: built a file %s, size=%lu\n", de->full_path, de->size);
	return 0;
}
