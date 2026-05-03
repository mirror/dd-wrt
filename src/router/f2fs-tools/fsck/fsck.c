/**
 * fsck.c
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "fsck.h"
#include "xattr.h"
#include "quotaio.h"
#include <time.h>

char *tree_mark;
uint32_t tree_mark_size = 256;

int f2fs_set_main_bitmap(struct f2fs_sb_info *sbi, u32 blk, int type)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct seg_entry *se;
	int fix = 0;

	se = get_seg_entry(sbi, GET_SEGNO(sbi, blk));
	if (se->type >= NO_CHECK_TYPE)
		fix = 1;
	else if (IS_DATASEG(se->type) != IS_DATASEG(type))
		fix = 1;

	/* just check data and node types */
	if (fix) {
		DBG(1, "Wrong segment type [0x%x] %x -> %x",
				GET_SEGNO(sbi, blk), se->type, type);
		se->type = type;
	}
	return f2fs_set_bit(BLKOFF_FROM_MAIN(sbi, blk), fsck->main_area_bitmap);
}

static inline int f2fs_test_main_bitmap(struct f2fs_sb_info *sbi, u32 blk)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	return f2fs_test_bit(BLKOFF_FROM_MAIN(sbi, blk),
						fsck->main_area_bitmap);
}

static inline int f2fs_clear_main_bitmap(struct f2fs_sb_info *sbi, u32 blk)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	return f2fs_clear_bit(BLKOFF_FROM_MAIN(sbi, blk),
						fsck->main_area_bitmap);
}

static inline int f2fs_test_sit_bitmap(struct f2fs_sb_info *sbi, u32 blk)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	return f2fs_test_bit(BLKOFF_FROM_MAIN(sbi, blk), fsck->sit_area_bitmap);
}

int f2fs_set_sit_bitmap(struct f2fs_sb_info *sbi, u32 blk)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	return f2fs_set_bit(BLKOFF_FROM_MAIN(sbi, blk), fsck->sit_area_bitmap);
}

static int add_into_hard_link_list(struct f2fs_sb_info *sbi,
						u32 nid, u32 link_cnt)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct hard_link_node *node = NULL, *tmp = NULL, *prev = NULL;

	node = calloc(sizeof(struct hard_link_node), 1);
	ASSERT(node != NULL);

	node->nid = nid;
	node->links = link_cnt;
	node->actual_links = 1;
	node->next = NULL;

	if (fsck->hard_link_list_head == NULL) {
		fsck->hard_link_list_head = node;
		goto out;
	}

	tmp = fsck->hard_link_list_head;

	/* Find insertion position */
	while (tmp && (nid < tmp->nid)) {
		ASSERT(tmp->nid != nid);
		prev = tmp;
		tmp = tmp->next;
	}

	if (tmp == fsck->hard_link_list_head) {
		node->next = tmp;
		fsck->hard_link_list_head = node;
	} else {
		prev->next = node;
		node->next = tmp;
	}

out:
	DBG(2, "ino[0x%x] has hard links [0x%x]\n", nid, link_cnt);
	return 0;
}

static int find_and_dec_hard_link_list(struct f2fs_sb_info *sbi, u32 nid)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct hard_link_node *node = NULL, *prev = NULL;

	if (fsck->hard_link_list_head == NULL)
		return -EINVAL;

	node = fsck->hard_link_list_head;

	while (node && (nid < node->nid)) {
		prev = node;
		node = node->next;
	}

	if (node == NULL || (nid != node->nid))
		return -EINVAL;

	/* Decrease link count */
	node->links = node->links - 1;
	node->actual_links++;

	/* if link count becomes one, remove the node */
	if (node->links == 1) {
		if (fsck->hard_link_list_head == node)
			fsck->hard_link_list_head = node->next;
		else
			prev->next = node->next;
		free(node);
	}
	return 0;
}

static int is_valid_ssa_node_blk(struct f2fs_sb_info *sbi, u32 nid,
							u32 blk_addr)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_summary_block *sum_blk;
	struct f2fs_summary *sum_entry;
	struct seg_entry * se;
	u32 segno, offset;
	int need_fix = 0, ret = 0;
	int type;

	if (get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO))
		return 0;

	segno = GET_SEGNO(sbi, blk_addr);
	offset = OFFSET_IN_SEG(sbi, blk_addr);

	sum_blk = get_sum_block(sbi, segno, &type);

	if (type != SEG_TYPE_NODE && type != SEG_TYPE_CUR_NODE) {
		/* can't fix current summary, then drop the block */
		if (!c.fix_on || type < 0) {
			ASSERT_MSG("Summary footer is not for node segment");
			ret = -EINVAL;
			goto out;
		}

		need_fix = 1;
		se = get_seg_entry(sbi, segno);
		if(IS_NODESEG(se->type)) {
			FIX_MSG("Summary footer indicates a node segment: 0x%x", segno);
			sum_blk->footer.entry_type = SUM_TYPE_NODE;
		} else {
			ret = -EINVAL;
			goto out;
		}
	}

	sum_entry = &(sum_blk->entries[offset]);

	if (le32_to_cpu(sum_entry->nid) != nid) {
		if (!c.fix_on || type < 0) {
			DBG(0, "nid                       [0x%x]\n", nid);
			DBG(0, "target blk_addr           [0x%x]\n", blk_addr);
			DBG(0, "summary blk_addr          [0x%x]\n",
						GET_SUM_BLKADDR(sbi,
						GET_SEGNO(sbi, blk_addr)));
			DBG(0, "seg no / offset           [0x%x / 0x%x]\n",
						GET_SEGNO(sbi, blk_addr),
						OFFSET_IN_SEG(sbi, blk_addr));
			DBG(0, "summary_entry.nid         [0x%x]\n",
						le32_to_cpu(sum_entry->nid));
			DBG(0, "--> node block's nid      [0x%x]\n", nid);
			ASSERT_MSG("Invalid node seg summary\n");
			ret = -EINVAL;
		} else {
			FIX_MSG("Set node summary 0x%x -> [0x%x] [0x%x]",
						segno, nid, blk_addr);
			sum_entry->nid = cpu_to_le32(nid);
			need_fix = 1;
		}
	}
	if (need_fix && f2fs_dev_is_writable()) {
		u64 ssa_blk;
		int ret2;

		ssa_blk = GET_SUM_BLKADDR(sbi, segno);
		ret2 = dev_write_block(sum_blk, ssa_blk);
		ASSERT(ret2 >= 0);
	}
out:
	if (type == SEG_TYPE_NODE || type == SEG_TYPE_DATA ||
					type == SEG_TYPE_MAX)
		free(sum_blk);
	return ret;
}

static int is_valid_summary(struct f2fs_sb_info *sbi, struct f2fs_summary *sum,
							u32 blk_addr)
{
	u16 ofs_in_node = le16_to_cpu(sum->ofs_in_node);
	u32 nid = le32_to_cpu(sum->nid);
	struct f2fs_node *node_blk = NULL;
	__le32 target_blk_addr;
	struct node_info ni;
	int ret = 0;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);

	if (!IS_VALID_NID(sbi, nid))
		goto out;

	get_node_info(sbi, nid, &ni);

	if (!IS_VALID_BLK_ADDR(sbi, ni.blk_addr))
		goto out;

	/* read node_block */
	ret = dev_read_block(node_blk, ni.blk_addr);
	ASSERT(ret >= 0);

	if (le32_to_cpu(node_blk->footer.nid) != nid)
		goto out;

	/* check its block address */
	if (node_blk->footer.nid == node_blk->footer.ino) {
		int ofs = get_extra_isize(node_blk);

		if (ofs + ofs_in_node >= DEF_ADDRS_PER_INODE)
			goto out;
		target_blk_addr = node_blk->i.i_addr[ofs + ofs_in_node];
	} else {
		if (ofs_in_node >= DEF_ADDRS_PER_BLOCK)
			goto out;
		target_blk_addr = node_blk->dn.addr[ofs_in_node];
	}

	if (blk_addr == le32_to_cpu(target_blk_addr))
		ret = 1;
out:
	free(node_blk);
	return ret;
}

static int is_valid_ssa_data_blk(struct f2fs_sb_info *sbi, u32 blk_addr,
		u32 parent_nid, u16 idx_in_node, u8 version)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_summary_block *sum_blk;
	struct f2fs_summary *sum_entry;
	struct seg_entry * se;
	u32 segno, offset;
	int need_fix = 0, ret = 0;
	int type;

	if (get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO))
		return 0;

	segno = GET_SEGNO(sbi, blk_addr);
	offset = OFFSET_IN_SEG(sbi, blk_addr);

	sum_blk = get_sum_block(sbi, segno, &type);

	if (type != SEG_TYPE_DATA && type != SEG_TYPE_CUR_DATA) {
		/* can't fix current summary, then drop the block */
		if (!c.fix_on || type < 0) {
			ASSERT_MSG("Summary footer is not for data segment");
			ret = -EINVAL;
			goto out;
		}

		need_fix = 1;
		se = get_seg_entry(sbi, segno);
		if (IS_DATASEG(se->type)) {
			FIX_MSG("Summary footer indicates a data segment: 0x%x", segno);
			sum_blk->footer.entry_type = SUM_TYPE_DATA;
		} else {
			ret = -EINVAL;
			goto out;
		}
	}

	sum_entry = &(sum_blk->entries[offset]);

	if (le32_to_cpu(sum_entry->nid) != parent_nid ||
			sum_entry->version != version ||
			le16_to_cpu(sum_entry->ofs_in_node) != idx_in_node) {
		if (!c.fix_on || type < 0) {
			DBG(0, "summary_entry.nid         [0x%x]\n",
					le32_to_cpu(sum_entry->nid));
			DBG(0, "summary_entry.version     [0x%x]\n",
					sum_entry->version);
			DBG(0, "summary_entry.ofs_in_node [0x%x]\n",
					le16_to_cpu(sum_entry->ofs_in_node));
			DBG(0, "parent nid                [0x%x]\n",
					parent_nid);
			DBG(0, "version from nat          [0x%x]\n", version);
			DBG(0, "idx in parent node        [0x%x]\n",
					idx_in_node);

			DBG(0, "Target data block addr    [0x%x]\n", blk_addr);
			ASSERT_MSG("Invalid data seg summary\n");
			ret = -EINVAL;
		} else if (is_valid_summary(sbi, sum_entry, blk_addr)) {
			/* delete wrong index */
			ret = -EINVAL;
		} else {
			FIX_MSG("Set data summary 0x%x -> [0x%x] [0x%x] [0x%x]",
					segno, parent_nid, version, idx_in_node);
			sum_entry->nid = cpu_to_le32(parent_nid);
			sum_entry->version = version;
			sum_entry->ofs_in_node = cpu_to_le16(idx_in_node);
			need_fix = 1;
		}
	}
	if (need_fix && f2fs_dev_is_writable()) {
		u64 ssa_blk;
		int ret2;

		ssa_blk = GET_SUM_BLKADDR(sbi, segno);
		ret2 = dev_write_block(sum_blk, ssa_blk);
		ASSERT(ret2 >= 0);
	}
out:
	if (type == SEG_TYPE_NODE || type == SEG_TYPE_DATA ||
					type == SEG_TYPE_MAX)
		free(sum_blk);
	return ret;
}

static int __check_inode_mode(u32 nid, enum FILE_TYPE ftype, u16 mode)
{
	if (ftype >= F2FS_FT_MAX)
		return 0;
	/* f2fs_iget will return -EIO if mode is not valid file type */
	if (!S_ISLNK(mode) && !S_ISREG(mode) && !S_ISDIR(mode) &&
	    !S_ISCHR(mode) && !S_ISBLK(mode) && !S_ISFIFO(mode) &&
	    !S_ISSOCK(mode)) {
		ASSERT_MSG("inode [0x%x] unknown file type i_mode [0x%x]",
			   nid, mode);
		return -1;
	}

	if (S_ISLNK(mode) && ftype != F2FS_FT_SYMLINK)
		goto err;
	if (S_ISREG(mode) && ftype != F2FS_FT_REG_FILE)
		goto err;
	if (S_ISDIR(mode) && ftype != F2FS_FT_DIR)
		goto err;
	if (S_ISCHR(mode) && ftype != F2FS_FT_CHRDEV)
		goto err;
	if (S_ISBLK(mode) && ftype != F2FS_FT_BLKDEV)
		goto err;
	if (S_ISFIFO(mode) && ftype != F2FS_FT_FIFO)
		goto err;
	if (S_ISSOCK(mode) && ftype != F2FS_FT_SOCK)
		goto err;
	return 0;
err:
	ASSERT_MSG("inode [0x%x] mismatch i_mode [0x%x vs. 0x%x]",
		   nid, ftype, mode);
	return -1;
}

static int sanity_check_nid(struct f2fs_sb_info *sbi, u32 nid,
			struct f2fs_node *node_blk,
			enum FILE_TYPE ftype, enum NODE_TYPE ntype,
			struct node_info *ni)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	int ret;

	if (!IS_VALID_NID(sbi, nid)) {
		ASSERT_MSG("nid is not valid. [0x%x]", nid);
		return -EINVAL;
	}

	get_node_info(sbi, nid, ni);
	if (ni->ino == 0) {
		ASSERT_MSG("nid[0x%x] ino is 0", nid);
		return -EINVAL;
	}

	if (ni->blk_addr == NEW_ADDR) {
		ASSERT_MSG("nid is NEW_ADDR. [0x%x]", nid);
		return -EINVAL;
	}

	if (!IS_VALID_BLK_ADDR(sbi, ni->blk_addr)) {
		ASSERT_MSG("blkaddress is not valid. [0x%x]", ni->blk_addr);
		return -EINVAL;
	}

	ret = dev_read_block(node_blk, ni->blk_addr);
	ASSERT(ret >= 0);

	if (ntype == TYPE_INODE &&
			node_blk->footer.nid != node_blk->footer.ino) {
		ASSERT_MSG("nid[0x%x] footer.nid[0x%x] footer.ino[0x%x]",
				nid, le32_to_cpu(node_blk->footer.nid),
				le32_to_cpu(node_blk->footer.ino));
		return -EINVAL;
	}
	if (ni->ino != le32_to_cpu(node_blk->footer.ino)) {
		ASSERT_MSG("nid[0x%x] nat_entry->ino[0x%x] footer.ino[0x%x]",
				nid, ni->ino, le32_to_cpu(node_blk->footer.ino));
		return -EINVAL;
	}
	if (ntype != TYPE_INODE &&
			node_blk->footer.nid == node_blk->footer.ino) {
		ASSERT_MSG("nid[0x%x] footer.nid[0x%x] footer.ino[0x%x]",
				nid, le32_to_cpu(node_blk->footer.nid),
				le32_to_cpu(node_blk->footer.ino));
		return -EINVAL;
	}

	if (le32_to_cpu(node_blk->footer.nid) != nid) {
		ASSERT_MSG("nid[0x%x] blk_addr[0x%x] footer.nid[0x%x]",
				nid, ni->blk_addr,
				le32_to_cpu(node_blk->footer.nid));
		return -EINVAL;
	}

	if (ntype == TYPE_XATTR) {
		u32 flag = le32_to_cpu(node_blk->footer.flag);

		if ((flag >> OFFSET_BIT_SHIFT) != XATTR_NODE_OFFSET) {
			ASSERT_MSG("xnid[0x%x] has wrong ofs:[0x%x]",
					nid, flag);
			return -EINVAL;
		}
	}

	if ((ntype == TYPE_INODE && ftype == F2FS_FT_DIR) ||
			(ntype == TYPE_XATTR && ftype == F2FS_FT_XATTR)) {
		/* not included '.' & '..' */
		if (f2fs_test_main_bitmap(sbi, ni->blk_addr) != 0) {
			ASSERT_MSG("Duplicated node blk. nid[0x%x][0x%x]\n",
					nid, ni->blk_addr);
			return -EINVAL;
		}
	}

	/* this if only from fix_hard_links */
	if (ftype == F2FS_FT_MAX)
		return 0;

	if (ntype == TYPE_INODE &&
		__check_inode_mode(nid, ftype, le16_to_cpu(node_blk->i.i_mode)))
		return -EINVAL;

	/* workaround to fix later */
	if (ftype != F2FS_FT_ORPHAN ||
			f2fs_test_bit(nid, fsck->nat_area_bitmap) != 0) {
		f2fs_clear_bit(nid, fsck->nat_area_bitmap);
		/* avoid reusing nid when reconnecting files */
		f2fs_set_bit(nid, NM_I(sbi)->nid_bitmap);
	} else
		ASSERT_MSG("orphan or xattr nid is duplicated [0x%x]\n",
				nid);

	if (is_valid_ssa_node_blk(sbi, nid, ni->blk_addr)) {
		ASSERT_MSG("summary node block is not valid. [0x%x]", nid);
		return -EINVAL;
	}

	if (f2fs_test_sit_bitmap(sbi, ni->blk_addr) == 0)
		ASSERT_MSG("SIT bitmap is 0x0. blk_addr[0x%x]",
				ni->blk_addr);

	if (f2fs_test_main_bitmap(sbi, ni->blk_addr) == 0) {

		fsck->chk.valid_blk_cnt++;
		fsck->chk.valid_node_cnt++;

		/* Progress report */
		if (!c.show_file_map && sbi->total_valid_node_count > 1000) {
			unsigned int p10 = sbi->total_valid_node_count / 10;

			if (sbi->fsck->chk.checked_node_cnt++ % p10)
				return 0;

			printf("[FSCK] Check node %"PRIu64" / %u (%.2f%%)\n",
				sbi->fsck->chk.checked_node_cnt,
				sbi->total_valid_node_count,
				10 * (float)sbi->fsck->chk.checked_node_cnt /
				p10);
		}
	}
	return 0;
}

int fsck_sanity_check_nid(struct f2fs_sb_info *sbi, u32 nid,
			struct f2fs_node *node_blk,
			enum FILE_TYPE ftype, enum NODE_TYPE ntype,
			struct node_info *ni)
{
	return sanity_check_nid(sbi, nid, node_blk, ftype, ntype, ni);
}

static int fsck_chk_xattr_blk(struct f2fs_sb_info *sbi, u32 ino,
					u32 x_nid, u32 *blk_cnt)
{
	struct f2fs_node *node_blk = NULL;
	struct node_info ni;
	int ret = 0;

	if (x_nid == 0x0)
		return 0;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);

	/* Sanity check */
	if (sanity_check_nid(sbi, x_nid, node_blk,
				F2FS_FT_XATTR, TYPE_XATTR, &ni)) {
		ret = -EINVAL;
		goto out;
	}

	*blk_cnt = *blk_cnt + 1;
	f2fs_set_main_bitmap(sbi, ni.blk_addr, CURSEG_COLD_NODE);
	DBG(2, "ino[0x%x] x_nid[0x%x]\n", ino, x_nid);
out:
	free(node_blk);
	return ret;
}

int fsck_chk_node_blk(struct f2fs_sb_info *sbi, struct f2fs_inode *inode,
		u32 nid, enum FILE_TYPE ftype, enum NODE_TYPE ntype,
		u32 *blk_cnt, struct f2fs_compr_blk_cnt *cbc,
		struct child_info *child)
{
	struct node_info ni;
	struct f2fs_node *node_blk = NULL;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);

	if (sanity_check_nid(sbi, nid, node_blk, ftype, ntype, &ni))
		goto err;

	if (ntype == TYPE_INODE) {
		struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

		fsck_chk_inode_blk(sbi, nid, ftype, node_blk, blk_cnt, cbc,
				&ni, child);
		quota_add_inode_usage(fsck->qctx, nid, &node_blk->i);
	} else {
		switch (ntype) {
		case TYPE_DIRECT_NODE:
			f2fs_set_main_bitmap(sbi, ni.blk_addr,
							CURSEG_WARM_NODE);
			fsck_chk_dnode_blk(sbi, inode, nid, ftype, node_blk,
					blk_cnt, cbc, child, &ni);
			break;
		case TYPE_INDIRECT_NODE:
			f2fs_set_main_bitmap(sbi, ni.blk_addr,
							CURSEG_COLD_NODE);
			fsck_chk_idnode_blk(sbi, inode, ftype, node_blk,
					blk_cnt, cbc, child);
			break;
		case TYPE_DOUBLE_INDIRECT_NODE:
			f2fs_set_main_bitmap(sbi, ni.blk_addr,
							CURSEG_COLD_NODE);
			fsck_chk_didnode_blk(sbi, inode, ftype, node_blk,
					blk_cnt, cbc, child);
			break;
		default:
			ASSERT(0);
		}
	}
	free(node_blk);
	return 0;
err:
	free(node_blk);
	return -EINVAL;
}

static inline void get_extent_info(struct extent_info *ext,
					struct f2fs_extent *i_ext)
{
	ext->fofs = le32_to_cpu(i_ext->fofs);
	ext->blk = le32_to_cpu(i_ext->blk_addr);
	ext->len = le32_to_cpu(i_ext->len);
}

static void check_extent_info(struct child_info *child,
						block_t blkaddr, int last)
{
	struct extent_info *ei = &child->ei;
	u32 pgofs = child->pgofs;
	int is_hole = 0;

	if (!ei->len)
		return;

	if (child->state & FSCK_UNMATCHED_EXTENT)
		return;

	if ((child->state & FSCK_INLINE_INODE) && ei->len)
		goto unmatched;

	if (last) {
		/* hole exist in the back of extent */
		if (child->last_blk != ei->blk + ei->len - 1)
			child->state |= FSCK_UNMATCHED_EXTENT;
		return;
	}

	if (blkaddr == NULL_ADDR || blkaddr == NEW_ADDR)
		is_hole = 1;

	if (pgofs >= ei->fofs && pgofs < ei->fofs + ei->len) {
		/* unmatched blkaddr */
		if (is_hole || (blkaddr != pgofs - ei->fofs + ei->blk))
			goto unmatched;

		if (!child->last_blk) {
			/* hole exists in the front of extent */
			if (pgofs != ei->fofs)
				goto unmatched;
		} else if (child->last_blk + 1 != blkaddr) {
			/* hole exists in the middle of extent */
			goto unmatched;
		}
		child->last_blk = blkaddr;
		return;
	}

	if (is_hole)
		return;

	if (blkaddr < ei->blk || blkaddr >= ei->blk + ei->len)
		return;
	/* unmatched file offset */
unmatched:
	child->state |= FSCK_UNMATCHED_EXTENT;
}

void fsck_reada_node_block(struct f2fs_sb_info *sbi, u32 nid)
{
	struct node_info ni;

	if (nid != 0 && IS_VALID_NID(sbi, nid)) {
		get_node_info(sbi, nid, &ni);
		if (IS_VALID_BLK_ADDR(sbi, ni.blk_addr))
			dev_reada_block(ni.blk_addr);
	}
}

void fsck_reada_all_direct_node_blocks(struct f2fs_sb_info *sbi,
						struct f2fs_node *node_blk)
{
	int i;

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		u32 nid = le32_to_cpu(node_blk->in.nid[i]);

		fsck_reada_node_block(sbi, nid);
	}
}

/* start with valid nid and blkaddr */
void fsck_chk_inode_blk(struct f2fs_sb_info *sbi, u32 nid,
		enum FILE_TYPE ftype, struct f2fs_node *node_blk,
		u32 *blk_cnt, struct f2fs_compr_blk_cnt *cbc,
		struct node_info *ni, struct child_info *child_d)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct child_info child;
	enum NODE_TYPE ntype;
	u32 i_links = le32_to_cpu(node_blk->i.i_links);
	u64 i_size = le64_to_cpu(node_blk->i.i_size);
	u64 i_blocks = le64_to_cpu(node_blk->i.i_blocks);
	bool compr_supported = c.feature & cpu_to_le32(F2FS_FEATURE_COMPRESSION);
	u32 i_flags = le32_to_cpu(node_blk->i.i_flags);
	bool compressed = i_flags & F2FS_COMPR_FL;
	bool compr_rel = node_blk->i.i_inline & F2FS_COMPRESS_RELEASED;
	u64 i_compr_blocks = le64_to_cpu(node_blk->i.i_compr_blocks);
	nid_t i_xattr_nid = le32_to_cpu(node_blk->i.i_xattr_nid);
	int ofs;
	char *en;
	u32 namelen;
	unsigned int addrs, idx = 0;
	unsigned short i_gc_failures;
	int need_fix = 0;
	int ret;
	u32 cluster_size = 1 << node_blk->i.i_log_cluster_size;

	if (!compressed)
		goto check_next;

	if (!compr_supported || (node_blk->i.i_inline & F2FS_INLINE_DATA)) {
		/*
		 * The 'compression' flag in i_flags affects the traverse of
		 * the node tree.  Thus, it must be fixed unconditionally
		 * in the memory (node_blk).
		 */
		node_blk->i.i_flags &= ~cpu_to_le32(F2FS_COMPR_FL);
		compressed = false;
		if (c.fix_on) {
			need_fix = 1;
			FIX_MSG("[0x%x] i_flags=0x%x -> 0x%x",
					nid, i_flags, node_blk->i.i_flags);
		}
		i_flags &= ~F2FS_COMPR_FL;
	}
check_next:
	memset(&child, 0, sizeof(child));
	child.links = 2;
	child.p_ino = nid;
	child.pp_ino = le32_to_cpu(node_blk->i.i_pino);
	child.dir_level = node_blk->i.i_dir_level;

	if (f2fs_test_main_bitmap(sbi, ni->blk_addr) == 0)
		fsck->chk.valid_inode_cnt++;

	if (ftype == F2FS_FT_DIR) {
		f2fs_set_main_bitmap(sbi, ni->blk_addr, CURSEG_HOT_NODE);
		namelen = le32_to_cpu(node_blk->i.i_namelen);
		if (namelen > F2FS_NAME_LEN)
			namelen = F2FS_NAME_LEN;
		memcpy(child.p_name, node_blk->i.i_name, namelen);
	} else {
		if (f2fs_test_main_bitmap(sbi, ni->blk_addr) == 0) {
			f2fs_set_main_bitmap(sbi, ni->blk_addr,
							CURSEG_WARM_NODE);
			if (i_links > 1 && ftype != F2FS_FT_ORPHAN &&
					!is_qf_ino(F2FS_RAW_SUPER(sbi), nid)) {
				/* First time. Create new hard link node */
				add_into_hard_link_list(sbi, nid, i_links);
				fsck->chk.multi_hard_link_files++;
			}
		} else {
			DBG(3, "[0x%x] has hard links [0x%x]\n", nid, i_links);
			if (find_and_dec_hard_link_list(sbi, nid)) {
				ASSERT_MSG("[0x%x] needs more i_links=0x%x",
						nid, i_links);
				if (c.fix_on) {
					node_blk->i.i_links =
						cpu_to_le32(i_links + 1);
					need_fix = 1;
					FIX_MSG("File: 0x%x "
						"i_links= 0x%x -> 0x%x",
						nid, i_links, i_links + 1);
				}
				goto skip_blkcnt_fix;
			}
			/* No need to go deep into the node */
			return;
		}
	}

	/* readahead xattr node block */
	fsck_reada_node_block(sbi, i_xattr_nid);

	if (fsck_chk_xattr_blk(sbi, nid, i_xattr_nid, blk_cnt)) {
		if (c.fix_on) {
			node_blk->i.i_xattr_nid = 0;
			need_fix = 1;
			FIX_MSG("Remove xattr block: 0x%x, x_nid = 0x%x",
							nid, i_xattr_nid);
		}
	}

	if (ftype == F2FS_FT_CHRDEV || ftype == F2FS_FT_BLKDEV ||
			ftype == F2FS_FT_FIFO || ftype == F2FS_FT_SOCK)
		goto check;

	/* init extent info */
	get_extent_info(&child.ei, &node_blk->i.i_ext);
	child.last_blk = 0;

	if (f2fs_has_extra_isize(&node_blk->i)) {
		if (c.feature & cpu_to_le32(F2FS_FEATURE_EXTRA_ATTR)) {
			unsigned int isize =
				le16_to_cpu(node_blk->i.i_extra_isize);
			if (isize > 4 * DEF_ADDRS_PER_INODE) {
				ASSERT_MSG("[0x%x] wrong i_extra_isize=0x%x",
						nid, isize);
				if (c.fix_on) {
					FIX_MSG("ino[0x%x] recover i_extra_isize "
						"from %u to %u",
						nid, isize,
						calc_extra_isize());
					node_blk->i.i_extra_isize =
						cpu_to_le16(calc_extra_isize());
					need_fix = 1;
				}
			}
		} else {
			ASSERT_MSG("[0x%x] wrong extra_attr flag", nid);
			if (c.fix_on) {
				FIX_MSG("ino[0x%x] remove F2FS_EXTRA_ATTR "
					"flag in i_inline:%u",
					nid, node_blk->i.i_inline);
				/* we don't support tuning F2FS_FEATURE_EXTRA_ATTR now */
				node_blk->i.i_inline &= ~F2FS_EXTRA_ATTR;
				need_fix = 1;
			}
		}

		if ((c.feature &
			cpu_to_le32(F2FS_FEATURE_FLEXIBLE_INLINE_XATTR)) &&
			(node_blk->i.i_inline & F2FS_INLINE_XATTR)) {
			unsigned int inline_size =
				le16_to_cpu(node_blk->i.i_inline_xattr_size);

			if (!inline_size ||
					inline_size > MAX_INLINE_XATTR_SIZE) {
				ASSERT_MSG("[0x%x] wrong inline_xattr_size:%u",
						nid, inline_size);
				if (c.fix_on) {
					FIX_MSG("ino[0x%x] recover inline xattr size "
						"from %u to %u",
						nid, inline_size,
						DEFAULT_INLINE_XATTR_ADDRS);
					node_blk->i.i_inline_xattr_size =
						cpu_to_le16(DEFAULT_INLINE_XATTR_ADDRS);
					need_fix = 1;
				}
			}
		}
	}
	ofs = get_extra_isize(node_blk);

	if ((node_blk->i.i_flags & cpu_to_le32(F2FS_CASEFOLD_FL)) &&
	    (ftype != F2FS_FT_DIR ||
	     !(c.feature & cpu_to_le32(F2FS_FEATURE_CASEFOLD)))) {
		ASSERT_MSG("[0x%x] unexpected casefold flag", nid);
		if (c.fix_on) {
			FIX_MSG("ino[0x%x] clear casefold flag", nid);
			node_blk->i.i_flags &= ~cpu_to_le32(F2FS_CASEFOLD_FL);
			need_fix = 1;
		}
	}

	if ((node_blk->i.i_inline & F2FS_INLINE_DATA)) {
		unsigned int inline_size = MAX_INLINE_DATA(node_blk);
		if (cur_qtype != -1)
			qf_szchk_type[cur_qtype] = QF_SZCHK_INLINE;
		block_t blkaddr = le32_to_cpu(node_blk->i.i_addr[ofs]);

		if (blkaddr != 0) {
			ASSERT_MSG("[0x%x] wrong inline reserve blkaddr:%u",
					nid, blkaddr);
			if (c.fix_on) {
				FIX_MSG("inline_data has wrong 0'th block = %x",
								blkaddr);
				node_blk->i.i_addr[ofs] = 0;
				node_blk->i.i_blocks = cpu_to_le64(*blk_cnt);
				need_fix = 1;
			}
		}
		if (i_size > inline_size) {
			ASSERT_MSG("[0x%x] wrong inline size:%lu",
					nid, (unsigned long)i_size);
			if (c.fix_on) {
				node_blk->i.i_size = cpu_to_le64(inline_size);
				FIX_MSG("inline_data has wrong i_size %lu",
							(unsigned long)i_size);
				need_fix = 1;
			}
		}
		if (!(node_blk->i.i_inline & F2FS_DATA_EXIST)) {
			char buf[MAX_INLINE_DATA(node_blk)];
			memset(buf, 0, MAX_INLINE_DATA(node_blk));

			if (memcmp(buf, inline_data_addr(node_blk),
						MAX_INLINE_DATA(node_blk))) {
				ASSERT_MSG("[0x%x] junk inline data", nid);
				if (c.fix_on) {
					FIX_MSG("inline_data has DATA_EXIST");
					node_blk->i.i_inline |= F2FS_DATA_EXIST;
					need_fix = 1;
				}
			}
		}
		DBG(3, "ino[0x%x] has inline data!\n", nid);
		child.state |= FSCK_INLINE_INODE;
		goto check;
	}

	if ((node_blk->i.i_inline & F2FS_INLINE_DENTRY)) {
		block_t blkaddr = le32_to_cpu(node_blk->i.i_addr[ofs]);

		DBG(3, "ino[0x%x] has inline dentry!\n", nid);
		if (blkaddr != 0) {
			ASSERT_MSG("[0x%x] wrong inline reserve blkaddr:%u",
								nid, blkaddr);
			if (c.fix_on) {
				FIX_MSG("inline_dentry has wrong 0'th block = %x",
								blkaddr);
				node_blk->i.i_addr[ofs] = 0;
				node_blk->i.i_blocks = cpu_to_le64(*blk_cnt);
				need_fix = 1;
			}
		}

		ret = fsck_chk_inline_dentries(sbi, node_blk, &child);
		if (ret < 0) {
			if (c.fix_on)
				need_fix = 1;
		}
		child.state |= FSCK_INLINE_INODE;
		goto check;
	}

	/* check data blocks in inode */
	addrs = ADDRS_PER_INODE(&node_blk->i);
	if (cur_qtype != -1) {
		u64 addrs_per_blk = (u64)ADDRS_PER_BLOCK(&node_blk->i);
		qf_szchk_type[cur_qtype] = QF_SZCHK_REGFILE;
		qf_maxsize[cur_qtype] = (u64)(addrs + 2 * addrs_per_blk +
				2 * addrs_per_blk * NIDS_PER_BLOCK +
				addrs_per_blk * NIDS_PER_BLOCK *
				NIDS_PER_BLOCK) * F2FS_BLKSIZE;
	}
	for (idx = 0; idx < addrs; idx++, child.pgofs++) {
		block_t blkaddr = le32_to_cpu(node_blk->i.i_addr[ofs + idx]);

		/* check extent info */
		check_extent_info(&child, blkaddr, 0);

		if (blkaddr == NULL_ADDR)
			continue;
		if (blkaddr == COMPRESS_ADDR) {
			if (!compressed || (child.pgofs &
					(cluster_size - 1)) != 0) {
				if (c.fix_on) {
					node_blk->i.i_addr[ofs + idx] =
							NULL_ADDR;
					need_fix = 1;
					FIX_MSG("[0x%x] i_addr[%d] = 0", nid,
							ofs + idx);
				}
				continue;
			}
			if (!compr_rel) {
				fsck->chk.valid_blk_cnt++;
				*blk_cnt = *blk_cnt + 1;
				cbc->cheader_pgofs = child.pgofs;
				cbc->cnt++;
			}
			continue;
		}
		if (!compr_rel && blkaddr == NEW_ADDR &&
				child.pgofs - cbc->cheader_pgofs < cluster_size)
			cbc->cnt++;
		ret = fsck_chk_data_blk(sbi,
				IS_CASEFOLDED(&node_blk->i),
				blkaddr,
				&child, (i_blocks == *blk_cnt),
				ftype, nid, idx, ni->version,
				file_is_encrypt(&node_blk->i));
		if (!ret) {
			*blk_cnt = *blk_cnt + 1;
			if (cur_qtype != -1 && blkaddr != NEW_ADDR)
				qf_last_blkofs[cur_qtype] = child.pgofs;
		} else if (c.fix_on) {
			node_blk->i.i_addr[ofs + idx] = 0;
			need_fix = 1;
			FIX_MSG("[0x%x] i_addr[%d] = 0", nid, ofs + idx);
		}
	}

	/* readahead node blocks */
	for (idx = 0; idx < 5; idx++) {
		u32 nid = le32_to_cpu(node_blk->i.i_nid[idx]);
		fsck_reada_node_block(sbi, nid);
	}

	/* check node blocks in inode */
	for (idx = 0; idx < 5; idx++) {
		nid_t i_nid = le32_to_cpu(node_blk->i.i_nid[idx]);

		if (idx == 0 || idx == 1)
			ntype = TYPE_DIRECT_NODE;
		else if (idx == 2 || idx == 3)
			ntype = TYPE_INDIRECT_NODE;
		else if (idx == 4)
			ntype = TYPE_DOUBLE_INDIRECT_NODE;
		else
			ASSERT(0);

		if (i_nid == 0x0)
			goto skip;

		ret = fsck_chk_node_blk(sbi, &node_blk->i, i_nid,
				ftype, ntype, blk_cnt, cbc, &child);
		if (!ret) {
			*blk_cnt = *blk_cnt + 1;
		} else if (ret == -EINVAL) {
			if (c.fix_on) {
				node_blk->i.i_nid[idx] = 0;
				need_fix = 1;
				FIX_MSG("[0x%x] i_nid[%d] = 0", nid, idx);
			}
skip:
			if (ntype == TYPE_DIRECT_NODE)
				child.pgofs += ADDRS_PER_BLOCK(&node_blk->i);
			else if (ntype == TYPE_INDIRECT_NODE)
				child.pgofs += ADDRS_PER_BLOCK(&node_blk->i) *
								NIDS_PER_BLOCK;
			else
				child.pgofs += ADDRS_PER_BLOCK(&node_blk->i) *
						NIDS_PER_BLOCK * NIDS_PER_BLOCK;
		}

	}

check:
	/* check uncovered range in the back of extent */
	check_extent_info(&child, 0, 1);

	if (child.state & FSCK_UNMATCHED_EXTENT) {
		ASSERT_MSG("ino: 0x%x has wrong ext: [pgofs:%u, blk:%u, len:%u]",
				nid, child.ei.fofs, child.ei.blk, child.ei.len);
		if (c.fix_on)
			need_fix = 1;
	}

	if (i_blocks != *blk_cnt) {
		ASSERT_MSG("ino: 0x%x has i_blocks: %08"PRIx64", "
				"but has %u blocks",
				nid, i_blocks, *blk_cnt);
		if (c.fix_on) {
			node_blk->i.i_blocks = cpu_to_le64(*blk_cnt);
			need_fix = 1;
			FIX_MSG("[0x%x] i_blocks=0x%08"PRIx64" -> 0x%x",
					nid, i_blocks, *blk_cnt);
		}
	}

	if (compressed && i_compr_blocks != cbc->cnt) {
		if (c.fix_on) {
			node_blk->i.i_compr_blocks = cpu_to_le64(cbc->cnt);
			need_fix = 1;
			FIX_MSG("[0x%x] i_compr_blocks=0x%08"PRIx64" -> 0x%x",
					nid, i_compr_blocks, cbc->cnt);
		}
	}

skip_blkcnt_fix:
	en = malloc(F2FS_PRINT_NAMELEN);
	ASSERT(en);

	namelen = le32_to_cpu(node_blk->i.i_namelen);
	if (namelen > F2FS_NAME_LEN) {
		if (child_d && child_d->i_namelen <= F2FS_NAME_LEN) {
			ASSERT_MSG("ino: 0x%x has i_namelen: 0x%x, "
					"but has %d characters for name",
					nid, namelen, child_d->i_namelen);
			if (c.fix_on) {
				FIX_MSG("[0x%x] i_namelen=0x%x -> 0x%x", nid, namelen,
					child_d->i_namelen);
				node_blk->i.i_namelen = cpu_to_le32(child_d->i_namelen);
				need_fix = 1;
			}
			namelen = child_d->i_namelen;
		} else
			namelen = F2FS_NAME_LEN;
	}
	pretty_print_filename(node_blk->i.i_name, namelen, en,
			      file_enc_name(&node_blk->i));
	if (ftype == F2FS_FT_ORPHAN)
		DBG(1, "Orphan Inode: 0x%x [%s] i_blocks: %u\n\n",
				le32_to_cpu(node_blk->footer.ino),
				en, (u32)i_blocks);

	if (is_qf_ino(F2FS_RAW_SUPER(sbi), nid))
		DBG(1, "Quota Inode: 0x%x [%s] i_blocks: %u\n\n",
				le32_to_cpu(node_blk->footer.ino),
				en, (u32)i_blocks);

	if (ftype == F2FS_FT_DIR) {
		DBG(1, "Directory Inode: 0x%x [%s] depth: %d has %d files\n\n",
				le32_to_cpu(node_blk->footer.ino), en,
				le32_to_cpu(node_blk->i.i_current_depth),
				child.files);

		if (i_links != child.links) {
			ASSERT_MSG("ino: 0x%x i_links: %u, real links: %u",
					nid, i_links, child.links);
			if (c.fix_on) {
				node_blk->i.i_links = cpu_to_le32(child.links);
				need_fix = 1;
				FIX_MSG("Dir: 0x%x i_links= 0x%x -> 0x%x",
						nid, i_links, child.links);
			}
		}
		if (child.dots < 2 &&
				!(node_blk->i.i_inline & F2FS_INLINE_DOTS)) {
			ASSERT_MSG("ino: 0x%x dots: %u",
					nid, child.dots);
			if (c.fix_on) {
				node_blk->i.i_inline |= F2FS_INLINE_DOTS;
				need_fix = 1;
				FIX_MSG("Dir: 0x%x set inline_dots", nid);
			}
		}
	}

	i_gc_failures = le16_to_cpu(node_blk->i.i_gc_failures);

	/*
	 * old kernel initialized i_gc_failures as 0x01, in preen mode 2,
	 * let's skip repairing.
	 */
	if (ftype == F2FS_FT_REG_FILE && i_gc_failures &&
		(c.preen_mode != PREEN_MODE_2 || i_gc_failures != 0x01)) {

		DBG(1, "Regular Inode: 0x%x [%s] depth: %d\n\n",
				le32_to_cpu(node_blk->footer.ino), en,
				i_gc_failures);

		if (c.fix_on) {
			node_blk->i.i_gc_failures = cpu_to_le16(0);
			need_fix = 1;
			FIX_MSG("Regular: 0x%x reset i_gc_failures from 0x%x to 0x00",
					nid, i_gc_failures);
		}
	}

	free(en);

	if (ftype == F2FS_FT_SYMLINK && i_size == 0 &&
			i_blocks == (i_xattr_nid ? 3 : 2)) {
		node_blk->i.i_size = cpu_to_le64(F2FS_BLKSIZE);
		need_fix = 1;
		FIX_MSG("Symlink: recover 0x%x with i_size=%lu",
					nid, (unsigned long)F2FS_BLKSIZE);
	}

	if (ftype == F2FS_FT_ORPHAN && i_links) {
		ASSERT_MSG("ino: 0x%x is orphan inode, but has i_links: %u",
				nid, i_links);
		if (c.fix_on) {
			node_blk->i.i_links = 0;
			need_fix = 1;
			FIX_MSG("ino: 0x%x orphan_inode, i_links= 0x%x -> 0",
					nid, i_links);
		}
	}

	/* drop extent information to avoid potential wrong access */
	if (need_fix && f2fs_dev_is_writable())
		node_blk->i.i_ext.len = 0;

	if ((c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM)) &&
				f2fs_has_extra_isize(&node_blk->i)) {
		__u32 provided, calculated;

		provided = le32_to_cpu(node_blk->i.i_inode_checksum);
		calculated = f2fs_inode_chksum(node_blk);

		if (provided != calculated) {
			ASSERT_MSG("ino: 0x%x chksum:0x%x, but calculated one is: 0x%x",
				nid, provided, calculated);
			if (c.fix_on) {
				node_blk->i.i_inode_checksum =
							cpu_to_le32(calculated);
				need_fix = 1;
				FIX_MSG("ino: 0x%x recover, i_inode_checksum= 0x%x -> 0x%x",
						nid, provided, calculated);
			}
		}
	}

	if (need_fix && f2fs_dev_is_writable()) {
		ret = dev_write_block(node_blk, ni->blk_addr);
		ASSERT(ret >= 0);
	}
}

int fsck_chk_dnode_blk(struct f2fs_sb_info *sbi, struct f2fs_inode *inode,
		u32 nid, enum FILE_TYPE ftype, struct f2fs_node *node_blk,
		u32 *blk_cnt, struct f2fs_compr_blk_cnt *cbc,
		struct child_info *child, struct node_info *ni)
{
	int idx, ret;
	int need_fix = 0;
	child->p_ino = nid;
	child->pp_ino = le32_to_cpu(inode->i_pino);
	u32 i_flags = le32_to_cpu(inode->i_flags);
	bool compressed = i_flags & F2FS_COMPR_FL;
	bool compr_rel = inode->i_inline & F2FS_COMPRESS_RELEASED;
	u32 cluster_size = 1 << inode->i_log_cluster_size;

	for (idx = 0; idx < ADDRS_PER_BLOCK(inode); idx++, child->pgofs++) {
		block_t blkaddr = le32_to_cpu(node_blk->dn.addr[idx]);

		check_extent_info(child, blkaddr, 0);

		if (blkaddr == NULL_ADDR)
			continue;
		if (blkaddr == COMPRESS_ADDR) {
			if (!compressed || (child->pgofs &
					(cluster_size - 1)) != 0) {
				if (c.fix_on) {
					node_blk->dn.addr[idx] = NULL_ADDR;
					need_fix = 1;
					FIX_MSG("[0x%x] dn.addr[%d] = 0", nid,
							idx);
				}
				continue;
			}
			if (!compr_rel) {
				F2FS_FSCK(sbi)->chk.valid_blk_cnt++;
				*blk_cnt = *blk_cnt + 1;
				cbc->cheader_pgofs = child->pgofs;
				cbc->cnt++;
			}
			continue;
		}
		if (!compr_rel && blkaddr == NEW_ADDR && child->pgofs -
				cbc->cheader_pgofs < cluster_size)
			cbc->cnt++;
		ret = fsck_chk_data_blk(sbi, IS_CASEFOLDED(inode),
			blkaddr, child,
			le64_to_cpu(inode->i_blocks) == *blk_cnt, ftype,
			nid, idx, ni->version,
			file_is_encrypt(inode));
		if (!ret) {
			*blk_cnt = *blk_cnt + 1;
			if (cur_qtype != -1 && blkaddr != NEW_ADDR)
				qf_last_blkofs[cur_qtype] = child->pgofs;
		} else if (c.fix_on) {
			node_blk->dn.addr[idx] = NULL_ADDR;
			need_fix = 1;
			FIX_MSG("[0x%x] dn.addr[%d] = 0", nid, idx);
		}
	}
	if (need_fix && f2fs_dev_is_writable()) {
		ret = dev_write_block(node_blk, ni->blk_addr);
		ASSERT(ret >= 0);
	}
	return 0;
}

int fsck_chk_idnode_blk(struct f2fs_sb_info *sbi, struct f2fs_inode *inode,
		enum FILE_TYPE ftype, struct f2fs_node *node_blk, u32 *blk_cnt,
		struct f2fs_compr_blk_cnt *cbc, struct child_info *child)
{
	int need_fix = 0, ret;
	int i = 0;

	fsck_reada_all_direct_node_blocks(sbi, node_blk);

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		if (le32_to_cpu(node_blk->in.nid[i]) == 0x0)
			goto skip;
		ret = fsck_chk_node_blk(sbi, inode,
				le32_to_cpu(node_blk->in.nid[i]),
				ftype, TYPE_DIRECT_NODE, blk_cnt,
				cbc, child);
		if (!ret)
			*blk_cnt = *blk_cnt + 1;
		else if (ret == -EINVAL) {
			if (!c.fix_on)
				printf("should delete in.nid[i] = 0;\n");
			else {
				node_blk->in.nid[i] = 0;
				need_fix = 1;
				FIX_MSG("Set indirect node 0x%x -> 0", i);
			}
skip:
			child->pgofs += ADDRS_PER_BLOCK(&node_blk->i);
		}
	}

	if (need_fix && f2fs_dev_is_writable()) {
		struct node_info ni;
		nid_t nid = le32_to_cpu(node_blk->footer.nid);

		get_node_info(sbi, nid, &ni);
		ret = dev_write_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	}

	return 0;
}

int fsck_chk_didnode_blk(struct f2fs_sb_info *sbi, struct f2fs_inode *inode,
		enum FILE_TYPE ftype, struct f2fs_node *node_blk, u32 *blk_cnt,
		struct f2fs_compr_blk_cnt *cbc, struct child_info *child)
{
	int i = 0;
	int need_fix = 0, ret = 0;

	fsck_reada_all_direct_node_blocks(sbi, node_blk);

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		if (le32_to_cpu(node_blk->in.nid[i]) == 0x0)
			goto skip;
		ret = fsck_chk_node_blk(sbi, inode,
				le32_to_cpu(node_blk->in.nid[i]),
				ftype, TYPE_INDIRECT_NODE, blk_cnt, cbc, child);
		if (!ret)
			*blk_cnt = *blk_cnt + 1;
		else if (ret == -EINVAL) {
			if (!c.fix_on)
				printf("should delete in.nid[i] = 0;\n");
			else {
				node_blk->in.nid[i] = 0;
				need_fix = 1;
				FIX_MSG("Set double indirect node 0x%x -> 0", i);
			}
skip:
			child->pgofs += ADDRS_PER_BLOCK(&node_blk->i) *
							NIDS_PER_BLOCK;
		}
	}

	if (need_fix && f2fs_dev_is_writable()) {
		struct node_info ni;
		nid_t nid = le32_to_cpu(node_blk->footer.nid);

		get_node_info(sbi, nid, &ni);
		ret = dev_write_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	}

	return 0;
}

static const char *lookup_table =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

/**
 * base64_encode() -
 *
 * Encodes the input string using characters from the set [A-Za-z0-9+,].
 * The encoded string is roughly 4/3 times the size of the input string.
 */
static int base64_encode(const u8 *src, int len, char *dst)
{
	int i, bits = 0, ac = 0;
	char *cp = dst;

	for (i = 0; i < len; i++) {
		ac += src[i] << bits;
		bits += 8;
		do {
			*cp++ = lookup_table[ac & 0x3f];
			ac >>= 6;
			bits -= 6;
		} while (bits >= 6);
	}
	if (bits)
		*cp++ = lookup_table[ac & 0x3f];
	return cp - dst;
}

void pretty_print_filename(const u8 *raw_name, u32 len,
			   char out[F2FS_PRINT_NAMELEN], int enc_name)
{
	len = min(len, (u32)F2FS_NAME_LEN);

	if (enc_name)
		len = base64_encode(raw_name, len, out);
	else
		memcpy(out, raw_name, len);
	out[len] = 0;
}

static void print_dentry(struct f2fs_sb_info *sbi, __u8 *name,
		u8 *bitmap, struct f2fs_dir_entry *dentry,
		int max, int idx, int last_blk, int enc_name)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	u32 depth = fsck->dentry_depth;
	int last_de = 0;
	int next_idx = 0;
	u32 name_len;
	unsigned int i;
	int bit_offset;
	char new[F2FS_PRINT_NAMELEN];

	if (!c.show_dentry && !c.show_file_map)
		return;

	name_len = le16_to_cpu(dentry[idx].name_len);
	next_idx = idx + (name_len + F2FS_SLOT_LEN - 1) / F2FS_SLOT_LEN;

	bit_offset = find_next_bit_le(bitmap, max, next_idx);
	if (bit_offset >= max && last_blk)
		last_de = 1;

	if (tree_mark_size <= depth) {
		tree_mark_size *= 2;
		ASSERT(tree_mark_size != 0);
		tree_mark = realloc(tree_mark, tree_mark_size);
		ASSERT(tree_mark != NULL);
	}

	if (last_de)
		tree_mark[depth] = '`';
	else
		tree_mark[depth] = '|';

	if (tree_mark[depth - 1] == '`')
		tree_mark[depth - 1] = ' ';

	pretty_print_filename(name, name_len, new, enc_name);

	if (c.show_file_map) {
		struct f2fs_dentry *d = fsck->dentry;

		if (dentry[idx].file_type != F2FS_FT_REG_FILE)
			return;

		while (d) {
			if (d->depth > 1)
				printf("/%s", d->name);
			d = d->next;
		}
		printf("/%s", new);
		if (dump_node(sbi, le32_to_cpu(dentry[idx].ino), 0))
			printf("\33[2K\r");
	} else {
		for (i = 1; i < depth; i++)
			printf("%c   ", tree_mark[i]);

		printf("%c-- %s <ino = 0x%x>, <encrypted (%d)>\n",
			last_de ? '`' : '|',
			new, le32_to_cpu(dentry[idx].ino),
			enc_name);
	}
}

static int f2fs_check_hash_code(int encoding, int casefolded,
			struct f2fs_dir_entry *dentry,
			const unsigned char *name, u32 len, int enc_name)
{
	/* Casefolded Encrypted names require a key to compute siphash */
	if (enc_name && casefolded)
		return 0;

	f2fs_hash_t hash_code = f2fs_dentry_hash(encoding, casefolded, name, len);
	/* fix hash_code made by old buggy code */
	if (dentry->hash_code != hash_code) {
		char new[F2FS_PRINT_NAMELEN];

		pretty_print_filename(name, len, new, enc_name);
		FIX_MSG("Mismatch hash_code for \"%s\" [%x:%x]",
				new, le32_to_cpu(dentry->hash_code),
				hash_code);
		dentry->hash_code = cpu_to_le32(hash_code);
		return 1;
	}
	return 0;
}


static int __get_current_level(int dir_level, u32 pgofs)
{
	unsigned int bidx = 0;
	int i;

	for (i = 0; i < MAX_DIR_HASH_DEPTH; i++) {
		bidx += dir_buckets(i, dir_level) * bucket_blocks(i);
		if (bidx > pgofs)
			break;
	}
	return i;
}

static int f2fs_check_dirent_position(const struct f2fs_dir_entry *dentry,
				      const char *printable_name,
				      u32 pgofs, u8 dir_level, u32 pino)
{
	unsigned int nbucket, nblock;
	unsigned int bidx, end_block;
	int level;

	level = __get_current_level(dir_level, pgofs);

	nbucket = dir_buckets(level, dir_level);
	nblock = bucket_blocks(level);

	bidx = dir_block_index(level, dir_level,
			       le32_to_cpu(dentry->hash_code) % nbucket);
	end_block = bidx + nblock;

	if (pgofs >= bidx && pgofs < end_block)
		return 0;

	ASSERT_MSG("Wrong position of dirent pino:%u, name:%s, level:%d, "
		"dir_level:%d, pgofs:%u, correct range:[%u, %u]\n",
		pino, printable_name, level, dir_level, pgofs, bidx,
		end_block - 1);
	return 1;
}

static int __chk_dots_dentries(struct f2fs_sb_info *sbi,
			       int casefolded,
			       struct f2fs_dir_entry *dentry,
			       struct child_info *child,
			       u8 *name, int len,
			       __u8 (*filename)[F2FS_SLOT_LEN],
			       int enc_name)
{
	int fixed = 0;

	if ((name[0] == '.' && len == 1)) {
		if (le32_to_cpu(dentry->ino) != child->p_ino) {
			ASSERT_MSG("Bad inode number[0x%x] for '.', parent_ino is [0x%x]\n",
				le32_to_cpu(dentry->ino), child->p_ino);
			dentry->ino = cpu_to_le32(child->p_ino);
			fixed = 1;
		}
	}

	if (name[0] == '.' && name[1] == '.' && len == 2) {
		if (child->p_ino == F2FS_ROOT_INO(sbi)) {
			if (le32_to_cpu(dentry->ino) != F2FS_ROOT_INO(sbi)) {
				ASSERT_MSG("Bad inode number[0x%x] for '..'\n",
					le32_to_cpu(dentry->ino));
				dentry->ino = cpu_to_le32(F2FS_ROOT_INO(sbi));
				fixed = 1;
			}
		} else if (le32_to_cpu(dentry->ino) != child->pp_ino) {
			ASSERT_MSG("Bad inode number[0x%x] for '..', parent parent ino is [0x%x]\n",
				le32_to_cpu(dentry->ino), child->pp_ino);
			dentry->ino = cpu_to_le32(child->pp_ino);
			fixed = 1;
		}
	}

	if (f2fs_check_hash_code(get_encoding(sbi), casefolded, dentry, name, len, enc_name))
		fixed = 1;

	if (name[len] != '\0') {
		ASSERT_MSG("'.' is not NULL terminated\n");
		name[len] = '\0';
		memcpy(*filename, name, len);
		fixed = 1;
	}
	return fixed;
}

static void nullify_dentry(struct f2fs_dir_entry *dentry, int offs,
			   __u8 (*filename)[F2FS_SLOT_LEN], u8 **bitmap)
{
	memset(dentry, 0, sizeof(struct f2fs_dir_entry));
	test_and_clear_bit_le(offs, *bitmap);
	memset(*filename, 0, F2FS_SLOT_LEN);
}

static int __chk_dentries(struct f2fs_sb_info *sbi, int casefolded,
			struct child_info *child,
			u8 *bitmap, struct f2fs_dir_entry *dentry,
			__u8 (*filenames)[F2FS_SLOT_LEN],
			int max, int last_blk, int enc_name)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	enum FILE_TYPE ftype;
	int dentries = 0;
	u32 blk_cnt;
	struct f2fs_compr_blk_cnt cbc;
	u8 *name;
	char en[F2FS_PRINT_NAMELEN];
	u16 name_len;
	int ret = 0;
	int fixed = 0;
	int i, slots;

	/* readahead inode blocks */
	for (i = 0; i < max; i++) {
		u32 ino;

		if (test_bit_le(i, bitmap) == 0)
			continue;

		ino = le32_to_cpu(dentry[i].ino);

		if (IS_VALID_NID(sbi, ino)) {
			struct node_info ni;

			get_node_info(sbi, ino, &ni);
			if (IS_VALID_BLK_ADDR(sbi, ni.blk_addr)) {
				dev_reada_block(ni.blk_addr);
				name_len = le16_to_cpu(dentry[i].name_len);
				if (name_len > 0)
					i += (name_len + F2FS_SLOT_LEN - 1) / F2FS_SLOT_LEN - 1;
			}
		}
	}

	for (i = 0; i < max;) {
		if (test_bit_le(i, bitmap) == 0) {
			i++;
			continue;
		}
		if (!IS_VALID_NID(sbi, le32_to_cpu(dentry[i].ino))) {
			ASSERT_MSG("Bad dentry 0x%x with invalid NID/ino 0x%x",
				    i, le32_to_cpu(dentry[i].ino));
			if (c.fix_on) {
				FIX_MSG("Clear bad dentry 0x%x with bad ino 0x%x",
					i, le32_to_cpu(dentry[i].ino));
				test_and_clear_bit_le(i, bitmap);
				fixed = 1;
			}
			i++;
			continue;
		}

		ftype = dentry[i].file_type;
		if ((ftype <= F2FS_FT_UNKNOWN || ftype > F2FS_FT_LAST_FILE_TYPE)) {
			ASSERT_MSG("Bad dentry 0x%x with unexpected ftype 0x%x",
						le32_to_cpu(dentry[i].ino), ftype);
			if (c.fix_on) {
				FIX_MSG("Clear bad dentry 0x%x with bad ftype 0x%x",
					i, ftype);
				test_and_clear_bit_le(i, bitmap);
				fixed = 1;
			}
			i++;
			continue;
		}

		name_len = le16_to_cpu(dentry[i].name_len);

		if (name_len == 0 || name_len > F2FS_NAME_LEN) {
			ASSERT_MSG("Bad dentry 0x%x with invalid name_len", i);
			if (c.fix_on) {
				FIX_MSG("Clear bad dentry 0x%x", i);
				test_and_clear_bit_le(i, bitmap);
				fixed = 1;
			}
			i++;
			continue;
		}
		name = calloc(name_len + 1, 1);
		ASSERT(name);

		memcpy(name, filenames[i], name_len);
		slots = (name_len + F2FS_SLOT_LEN - 1) / F2FS_SLOT_LEN;

		/* Becareful. 'dentry.file_type' is not imode. */
		if (ftype == F2FS_FT_DIR) {
			if ((name[0] == '.' && name_len == 1) ||
				(name[0] == '.' && name[1] == '.' &&
							name_len == 2)) {
				ret = __chk_dots_dentries(sbi, casefolded, &dentry[i],
					child, name, name_len, &filenames[i],
					enc_name);
				switch (ret) {
				case 1:
					fixed = 1;
					fallthrough;
				case 0:
					child->dots++;
					break;
				}

				if (child->dots > 2) {
					ASSERT_MSG("More than one '.' or '..', should delete the extra one\n");
					nullify_dentry(&dentry[i], i,
						       &filenames[i], &bitmap);
					child->dots--;
					fixed = 1;
				}

				i++;
				free(name);
				continue;
			}
		}

		if (f2fs_check_hash_code(get_encoding(sbi), casefolded, dentry + i, name, name_len, enc_name))
			fixed = 1;

		pretty_print_filename(name, name_len, en, enc_name);

		if (max == NR_DENTRY_IN_BLOCK) {
			ret = f2fs_check_dirent_position(dentry + i, en,
					child->pgofs, child->dir_level,
					child->p_ino);
			if (ret) {
				if (c.fix_on) {
					FIX_MSG("Clear bad dentry 0x%x", i);
					test_and_clear_bit_le(i, bitmap);
					fixed = 1;
				}
				i++;
				free(name);
				continue;
			}
		}

		DBG(1, "[%3u]-[0x%x] name[%s] len[0x%x] ino[0x%x] type[0x%x]\n",
				fsck->dentry_depth, i, en, name_len,
				le32_to_cpu(dentry[i].ino),
				dentry[i].file_type);

		print_dentry(sbi, name, bitmap,
				dentry, max, i, last_blk, enc_name);

		blk_cnt = 1;
		cbc.cnt = 0;
		cbc.cheader_pgofs = CHEADER_PGOFS_NONE;
		child->i_namelen = name_len;
		ret = fsck_chk_node_blk(sbi,
				NULL, le32_to_cpu(dentry[i].ino),
				ftype, TYPE_INODE, &blk_cnt, &cbc, child);

		if (ret && c.fix_on) {
			int j;

			for (j = 0; j < slots; j++)
				test_and_clear_bit_le(i + j, bitmap);
			FIX_MSG("Unlink [0x%x] - %s len[0x%x], type[0x%x]",
					le32_to_cpu(dentry[i].ino),
					en, name_len,
					dentry[i].file_type);
			fixed = 1;
		} else if (ret == 0) {
			if (ftype == F2FS_FT_DIR)
				child->links++;
			dentries++;
			child->files++;
		}

		i += slots;
		free(name);
	}
	return fixed ? -1 : dentries;
}

int fsck_chk_inline_dentries(struct f2fs_sb_info *sbi,
		struct f2fs_node *node_blk, struct child_info *child)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_dentry *cur_dentry = fsck->dentry_end;
	struct f2fs_dentry *new_dentry;
	struct f2fs_dentry_ptr d;
	void *inline_dentry;
	int dentries;

	inline_dentry = inline_data_addr(node_blk);
	ASSERT(inline_dentry != NULL);

	make_dentry_ptr(&d, node_blk, inline_dentry, 2);

	fsck->dentry_depth++;
	new_dentry = calloc(sizeof(struct f2fs_dentry), 1);
	ASSERT(new_dentry != NULL);

	new_dentry->depth = fsck->dentry_depth;
	memcpy(new_dentry->name, child->p_name, F2FS_NAME_LEN);
	cur_dentry->next = new_dentry;
	fsck->dentry_end = new_dentry;

	dentries = __chk_dentries(sbi, IS_CASEFOLDED(&node_blk->i), child,
			d.bitmap, d.dentry, d.filename, d.max, 1,
			file_is_encrypt(&node_blk->i));// pass through
	if (dentries < 0) {
		DBG(1, "[%3d] Inline Dentry Block Fixed hash_codes\n\n",
			fsck->dentry_depth);
	} else {
		DBG(1, "[%3d] Inline Dentry Block Done : "
				"dentries:%d in %d slots (len:%d)\n\n",
			fsck->dentry_depth, dentries,
			d.max, F2FS_NAME_LEN);
	}
	fsck->dentry = cur_dentry;
	fsck->dentry_end = cur_dentry;
	cur_dentry->next = NULL;
	free(new_dentry);
	fsck->dentry_depth--;
	return dentries;
}

int fsck_chk_dentry_blk(struct f2fs_sb_info *sbi, int casefolded, u32 blk_addr,
		struct child_info *child, int last_blk, int enc_name)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_dentry_block *de_blk;
	struct f2fs_dentry *cur_dentry = fsck->dentry_end;
	struct f2fs_dentry *new_dentry;
	int dentries, ret;

	de_blk = (struct f2fs_dentry_block *)calloc(BLOCK_SZ, 1);
	ASSERT(de_blk != NULL);

	ret = dev_read_block(de_blk, blk_addr);
	ASSERT(ret >= 0);

	fsck->dentry_depth++;
	new_dentry = calloc(sizeof(struct f2fs_dentry), 1);
	ASSERT(new_dentry != NULL);
	new_dentry->depth = fsck->dentry_depth;
	memcpy(new_dentry->name, child->p_name, F2FS_NAME_LEN);
	cur_dentry->next = new_dentry;
	fsck->dentry_end = new_dentry;

	dentries = __chk_dentries(sbi, casefolded, child,
			de_blk->dentry_bitmap,
			de_blk->dentry, de_blk->filename,
			NR_DENTRY_IN_BLOCK, last_blk, enc_name);

	if (dentries < 0 && f2fs_dev_is_writable()) {
		ret = dev_write_block(de_blk, blk_addr);
		ASSERT(ret >= 0);
		DBG(1, "[%3d] Dentry Block [0x%x] Fixed hash_codes\n\n",
			fsck->dentry_depth, blk_addr);
	} else {
		DBG(1, "[%3d] Dentry Block [0x%x] Done : "
				"dentries:%d in %d slots (len:%d)\n\n",
			fsck->dentry_depth, blk_addr, dentries,
			NR_DENTRY_IN_BLOCK, F2FS_NAME_LEN);
	}
	fsck->dentry = cur_dentry;
	fsck->dentry_end = cur_dentry;
	cur_dentry->next = NULL;
	free(new_dentry);
	fsck->dentry_depth--;
	free(de_blk);
	return 0;
}

int fsck_chk_data_blk(struct f2fs_sb_info *sbi, int casefolded,
		u32 blk_addr, struct child_info *child, int last_blk,
		enum FILE_TYPE ftype, u32 parent_nid, u16 idx_in_node, u8 ver,
		int enc_name)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	/* Is it reserved block? */
	if (blk_addr == NEW_ADDR) {
		fsck->chk.valid_blk_cnt++;
		return 0;
	}

	if (!IS_VALID_BLK_ADDR(sbi, blk_addr)) {
		ASSERT_MSG("blkaddress is not valid. [0x%x]", blk_addr);
		return -EINVAL;
	}

	if (is_valid_ssa_data_blk(sbi, blk_addr, parent_nid,
						idx_in_node, ver)) {
		ASSERT_MSG("summary data block is not valid. [0x%x]",
						parent_nid);
		return -EINVAL;
	}

	if (f2fs_test_sit_bitmap(sbi, blk_addr) == 0)
		ASSERT_MSG("SIT bitmap is 0x0. blk_addr[0x%x]", blk_addr);

	if (f2fs_test_main_bitmap(sbi, blk_addr) != 0)
		ASSERT_MSG("Duplicated data [0x%x]. pnid[0x%x] idx[0x%x]",
				blk_addr, parent_nid, idx_in_node);

	fsck->chk.valid_blk_cnt++;

	if (ftype == F2FS_FT_DIR) {
		f2fs_set_main_bitmap(sbi, blk_addr, CURSEG_HOT_DATA);
		return fsck_chk_dentry_blk(sbi, casefolded, blk_addr, child,
						last_blk, enc_name);
	} else {
		f2fs_set_main_bitmap(sbi, blk_addr, CURSEG_WARM_DATA);
	}
	return 0;
}

int fsck_chk_orphan_node(struct f2fs_sb_info *sbi)
{
	u32 blk_cnt = 0;
	struct f2fs_compr_blk_cnt cbc = {0, CHEADER_PGOFS_NONE};
	block_t start_blk, orphan_blkaddr, i, j;
	struct f2fs_orphan_block *orphan_blk, *new_blk;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	u32 entry_count;

	if (!is_set_ckpt_flags(F2FS_CKPT(sbi), CP_ORPHAN_PRESENT_FLAG))
		return 0;

	start_blk = __start_cp_addr(sbi) + 1 + get_sb(cp_payload);
	orphan_blkaddr = __start_sum_addr(sbi) - 1 - get_sb(cp_payload);

	f2fs_ra_meta_pages(sbi, start_blk, orphan_blkaddr, META_CP);

	orphan_blk = calloc(BLOCK_SZ, 1);
	ASSERT(orphan_blk);

	new_blk = calloc(BLOCK_SZ, 1);
	ASSERT(new_blk);

	for (i = 0; i < orphan_blkaddr; i++) {
		int ret = dev_read_block(orphan_blk, start_blk + i);
		u32 new_entry_count = 0;

		ASSERT(ret >= 0);
		entry_count = le32_to_cpu(orphan_blk->entry_count);

		for (j = 0; j < entry_count; j++) {
			nid_t ino = le32_to_cpu(orphan_blk->ino[j]);
			DBG(1, "[%3d] ino [0x%x]\n", i, ino);
			struct node_info ni;
			blk_cnt = 1;
			cbc.cnt = 0;
			cbc.cheader_pgofs = CHEADER_PGOFS_NONE;

			if (c.preen_mode == PREEN_MODE_1 && !c.fix_on) {
				get_node_info(sbi, ino, &ni);
				if (!IS_VALID_NID(sbi, ino) ||
				    !IS_VALID_BLK_ADDR(sbi, ni.blk_addr)) {
					free(orphan_blk);
					free(new_blk);
					return -EINVAL;
				}

				continue;
			}

			ret = fsck_chk_node_blk(sbi, NULL, ino,
					F2FS_FT_ORPHAN, TYPE_INODE, &blk_cnt,
					&cbc, NULL);
			if (!ret)
				new_blk->ino[new_entry_count++] =
							orphan_blk->ino[j];
			else if (ret && c.fix_on)
				FIX_MSG("[0x%x] remove from orphan list", ino);
			else if (ret)
				ASSERT_MSG("[0x%x] wrong orphan inode", ino);
		}
		if (f2fs_dev_is_writable() && c.fix_on &&
				entry_count != new_entry_count) {
			new_blk->entry_count = cpu_to_le32(new_entry_count);
			ret = dev_write_block(new_blk, start_blk + i);
			ASSERT(ret >= 0);
		}
		memset(orphan_blk, 0, BLOCK_SZ);
		memset(new_blk, 0, BLOCK_SZ);
	}
	free(orphan_blk);
	free(new_blk);

	return 0;
}

int fsck_chk_quota_node(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	enum quota_type qtype;
	int ret = 0;
	u32 blk_cnt = 0;
	struct f2fs_compr_blk_cnt cbc = {0, CHEADER_PGOFS_NONE};

	for (qtype = 0; qtype < F2FS_MAX_QUOTAS; qtype++) {
		cur_qtype = qtype;
		if (sb->qf_ino[qtype] == 0)
			continue;
		nid_t ino = QUOTA_INO(sb, qtype);
		struct node_info ni;

		DBG(1, "qtype [%d] ino [0x%x]\n", qtype, ino);
		blk_cnt = 1;
		cbc.cnt = 0;
		cbc.cheader_pgofs = CHEADER_PGOFS_NONE;

		if (c.preen_mode == PREEN_MODE_1 && !c.fix_on) {
			get_node_info(sbi, ino, &ni);
			if (!IS_VALID_NID(sbi, ino) ||
					!IS_VALID_BLK_ADDR(sbi, ni.blk_addr))
				return -EINVAL;
			continue;
		}
		ret = fsck_chk_node_blk(sbi, NULL, ino,
				F2FS_FT_REG_FILE, TYPE_INODE, &blk_cnt,
				&cbc, NULL);
		if (ret) {
			ASSERT_MSG("wrong quota inode, qtype [%d] ino [0x%x]",
								qtype, ino);
			qf_szchk_type[qtype] = QF_SZCHK_ERR;
			if (c.fix_on)
				f2fs_rebuild_qf_inode(sbi, qtype);
		}
	}
	cur_qtype = -1;
	return ret;
}

int fsck_chk_quota_files(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	enum quota_type qtype;
	f2fs_ino_t ino;
	int ret = 0;
	int needs_writeout;

	/* Return if quota feature is disabled */
	if (!fsck->qctx)
		return 0;

	for (qtype = 0; qtype < F2FS_MAX_QUOTAS; qtype++) {
		ino = sb->qf_ino[qtype];
		if (!ino)
			continue;

	        DBG(1, "Checking Quota file ([%3d] ino [0x%x])\n", qtype, ino);
		needs_writeout = 0;
		ret = quota_compare_and_update(sbi, qtype, &needs_writeout,
						c.preserve_limits);
		if (ret == 0 && needs_writeout == 0) {
			DBG(1, "OK\n");
			continue;
		}

		/* Something is wrong */
		if (c.fix_on) {
			DBG(0, "Fixing Quota file ([%3d] ino [0x%x])\n",
							qtype, ino);
			f2fs_filesize_update(sbi, ino, 0);
			ret = quota_write_inode(sbi, qtype);
			if (!ret) {
				c.quota_fixed = true;
				DBG(1, "OK\n");
			} else {
				ASSERT_MSG("Unable to write quota file");
			}
		} else {
			ASSERT_MSG("Quota file is missing or invalid"
					" quota file content found.");
		}
	}
	return ret;
}

int fsck_chk_meta(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct seg_entry *se;
	unsigned int sit_valid_segs = 0, sit_node_blks = 0;
	unsigned int i;

	/* 1. check sit usage with CP: curseg is lost? */
	for (i = 0; i < MAIN_SEGS(sbi); i++) {
		se = get_seg_entry(sbi, i);
		if (se->valid_blocks != 0)
			sit_valid_segs++;
		else if (IS_CUR_SEGNO(sbi, i)) {
			/* curseg has not been written back to device */
			MSG(1, "\tInfo: curseg %u is counted in valid segs\n", i);
			sit_valid_segs++;
		}
		if (IS_NODESEG(se->type))
			sit_node_blks += se->valid_blocks;
	}
	if (fsck->chk.sit_free_segs + sit_valid_segs !=
				get_usable_seg_count(sbi)) {
		ASSERT_MSG("SIT usage does not match: sit_free_segs %u, "
				"sit_valid_segs %u, total_segs %u",
			fsck->chk.sit_free_segs, sit_valid_segs,
			get_usable_seg_count(sbi));
		return -EINVAL;
	}

	/* 2. check node count */
	if (fsck->chk.valid_nat_entry_cnt != sit_node_blks) {
		ASSERT_MSG("node count does not match: valid_nat_entry_cnt %u,"
			" sit_node_blks %u",
			fsck->chk.valid_nat_entry_cnt, sit_node_blks);
		return -EINVAL;
	}

	/* 3. check SIT with CP */
	if (fsck->chk.sit_free_segs != le32_to_cpu(cp->free_segment_count)) {
		ASSERT_MSG("free segs does not match: sit_free_segs %u, "
				"free_segment_count %u",
				fsck->chk.sit_free_segs,
				le32_to_cpu(cp->free_segment_count));
		return -EINVAL;
	}

	/* 4. check NAT with CP */
	if (fsck->chk.valid_nat_entry_cnt !=
					le32_to_cpu(cp->valid_node_count)) {
		ASSERT_MSG("valid node does not match: valid_nat_entry_cnt %u,"
				" valid_node_count %u",
				fsck->chk.valid_nat_entry_cnt,
				le32_to_cpu(cp->valid_node_count));
		return -EINVAL;
	}

	/* 4. check orphan inode simply */
	if (fsck_chk_orphan_node(sbi))
		return -EINVAL;

	/* 5. check nat entry -- must be done before quota check */
	for (i = 0; i < fsck->nr_nat_entries; i++) {
		u32 blk = le32_to_cpu(fsck->entries[i].block_addr);
		nid_t ino = le32_to_cpu(fsck->entries[i].ino);

		if (!blk)
			/*
			 * skip entry whose ino is 0, otherwise, we will
			 * get a negative number by BLKOFF_FROM_MAIN(sbi, blk)
			 */
			continue;

		if (!IS_VALID_BLK_ADDR(sbi, blk)) {
			MSG(0, "\tError: nat entry[ino %u block_addr 0x%x]"
				" is in valid\n",
				ino, blk);
			return -EINVAL;
		}

		if (!f2fs_test_sit_bitmap(sbi, blk)) {
			MSG(0, "\tError: nat entry[ino %u block_addr 0x%x]"
				" not find it in sit_area_bitmap\n",
				ino, blk);
			return -EINVAL;
		}

		if (!IS_VALID_NID(sbi, ino)) {
			MSG(0, "\tError: nat_entry->ino %u exceeds the range"
				" of nat entries %u\n",
				ino, fsck->nr_nat_entries);
			return -EINVAL;
		}

		if (!f2fs_test_bit(ino, fsck->nat_area_bitmap)) {
			MSG(0, "\tError: nat_entry->ino %u is not set in"
				" nat_area_bitmap\n", ino);
			return -EINVAL;
		}
	}

	/* 6. check quota inode simply */
	if (fsck_chk_quota_node(sbi))
		return -EINVAL;

	if (fsck->nat_valid_inode_cnt != le32_to_cpu(cp->valid_inode_count)) {
		ASSERT_MSG("valid inode does not match: nat_valid_inode_cnt %u,"
				" valid_inode_count %u",
				fsck->nat_valid_inode_cnt,
				le32_to_cpu(cp->valid_inode_count));
		return -EINVAL;
	}

	return 0;
}

void fsck_chk_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

	if (get_cp(ckpt_flags) & CP_LARGE_NAT_BITMAP_FLAG) {
		if (get_cp(checksum_offset) != CP_MIN_CHKSUM_OFFSET) {
			ASSERT_MSG("Deprecated layout of large_nat_bitmap, "
				"chksum_offset:%u", get_cp(checksum_offset));
			c.fix_chksum = 1;
		}
	}
}

void fsck_init(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_sm_info *sm_i = SM_I(sbi);

	/*
	 * We build three bitmap for main/sit/nat so that may check consistency
	 * of filesystem.
	 * 1. main_area_bitmap will be used to check whether all blocks of main
	 *    area is used or not.
	 * 2. nat_area_bitmap has bitmap information of used nid in NAT.
	 * 3. sit_area_bitmap has bitmap information of used main block.
	 * At Last sequence, we compare main_area_bitmap with sit_area_bitmap.
	 */
	fsck->nr_main_blks = sm_i->main_segments << sbi->log_blocks_per_seg;
	fsck->main_area_bitmap_sz = (fsck->nr_main_blks + 7) / 8;
	fsck->main_area_bitmap = calloc(fsck->main_area_bitmap_sz, 1);
	ASSERT(fsck->main_area_bitmap != NULL);

	build_nat_area_bitmap(sbi);

	build_sit_area_bitmap(sbi);

	ASSERT(tree_mark_size != 0);
	tree_mark = calloc(tree_mark_size, 1);
	ASSERT(tree_mark != NULL);
	fsck->dentry = calloc(sizeof(struct f2fs_dentry), 1);
	ASSERT(fsck->dentry != NULL);
	memcpy(fsck->dentry->name, "/", 1);
	fsck->dentry_end = fsck->dentry;

	c.quota_fixed = false;
}

static void fix_hard_links(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct hard_link_node *tmp, *node;
	struct f2fs_node *node_blk = NULL;
	struct node_info ni;
	int ret;

	if (fsck->hard_link_list_head == NULL)
		return;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);

	node = fsck->hard_link_list_head;
	while (node) {
		/* Sanity check */
		if (sanity_check_nid(sbi, node->nid, node_blk,
					F2FS_FT_MAX, TYPE_INODE, &ni))
			FIX_MSG("Failed to fix, rerun fsck.f2fs");

		node_blk->i.i_links = cpu_to_le32(node->actual_links);

		FIX_MSG("File: 0x%x i_links= 0x%x -> 0x%x",
				node->nid, node->links, node->actual_links);

		ret = dev_write_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
		tmp = node;
		node = node->next;
		free(tmp);
	}
	free(node_blk);
}

static void fix_nat_entries(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	u32 i;

	for (i = 0; i < fsck->nr_nat_entries; i++)
		if (f2fs_test_bit(i, fsck->nat_area_bitmap) != 0)
			nullify_nat_entry(sbi, i);
}

static void flush_curseg_sit_entries(struct f2fs_sb_info *sbi)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_sit_block *sit_blk;
	int i;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	/* update curseg sit entries, since we may change
	 * a segment type in move_curseg_info
	 */
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;

		se = get_seg_entry(sbi, curseg->segno);
		get_current_sit_page(sbi, curseg->segno, sit_blk);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, curseg->segno)];
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		rewrite_current_sit_page(sbi, curseg->segno, sit_blk);
	}

	free(sit_blk);
}

static void fix_checksum(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	struct sit_info *sit_i = SIT_I(sbi);
	void *bitmap_offset;

	if (!c.fix_chksum)
		return;

	bitmap_offset = cp->sit_nat_version_bitmap + sizeof(__le32);

	memcpy(bitmap_offset, nm_i->nat_bitmap, nm_i->bitmap_size);
	memcpy(bitmap_offset + nm_i->bitmap_size,
			sit_i->sit_bitmap, sit_i->bitmap_size);
}

static void fix_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	unsigned long long cp_blk_no;
	u32 flags = c.alloc_failed ? CP_FSCK_FLAG: CP_UMOUNT_FLAG;
	block_t orphan_blks = 0;
	block_t cp_blocks;
	u32 i;
	int ret;
	uint32_t crc = 0;

	/* should call from fsck */
	ASSERT(c.func == FSCK);

	if (is_set_ckpt_flags(cp, CP_ORPHAN_PRESENT_FLAG)) {
		orphan_blks = __start_sum_addr(sbi) - 1;
		flags |= CP_ORPHAN_PRESENT_FLAG;
	}
	if (is_set_ckpt_flags(cp, CP_TRIMMED_FLAG))
		flags |= CP_TRIMMED_FLAG;
	if (is_set_ckpt_flags(cp, CP_DISABLED_FLAG))
		flags |= CP_DISABLED_FLAG;
	if (is_set_ckpt_flags(cp, CP_LARGE_NAT_BITMAP_FLAG)) {
		flags |= CP_LARGE_NAT_BITMAP_FLAG;
		set_cp(checksum_offset, CP_MIN_CHKSUM_OFFSET);
	} else {
		set_cp(checksum_offset, CP_CHKSUM_OFFSET);
	}

	if (flags & CP_UMOUNT_FLAG)
		cp_blocks = 8;
	else
		cp_blocks = 5;

	set_cp(cp_pack_total_block_count, cp_blocks +
				orphan_blks + get_sb(cp_payload));

	flags = update_nat_bits_flags(sb, cp, flags);
	flags |= CP_NOCRC_RECOVERY_FLAG;
	set_cp(ckpt_flags, flags);

	set_cp(free_segment_count, get_free_segments(sbi));
	set_cp(valid_block_count, fsck->chk.valid_blk_cnt);
	set_cp(valid_node_count, fsck->chk.valid_node_cnt);
	set_cp(valid_inode_count, fsck->chk.valid_inode_cnt);

	crc = f2fs_checkpoint_chksum(cp);
	*((__le32 *)((unsigned char *)cp + get_cp(checksum_offset))) =
							cpu_to_le32(crc);

	cp_blk_no = get_sb(cp_blkaddr);
	if (sbi->cur_cp == 2)
		cp_blk_no += 1 << get_sb(log_blocks_per_seg);

	ret = dev_write_block(cp, cp_blk_no++);
	ASSERT(ret >= 0);

	for (i = 0; i < get_sb(cp_payload); i++) {
		ret = dev_write_block(((unsigned char *)cp) +
					(i + 1) * F2FS_BLKSIZE, cp_blk_no++);
		ASSERT(ret >= 0);
	}

	cp_blk_no += orphan_blks;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);

		if (!(flags & CP_UMOUNT_FLAG) && IS_NODESEG(i))
			continue;

		ret = dev_write_block(curseg->sum_blk, cp_blk_no++);
		ASSERT(ret >= 0);
	}

	/* Write nat bits */
	if (flags & CP_NAT_BITS_FLAG)
		write_nat_bits(sbi, sb, cp, sbi->cur_cp);

	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);

	ret = dev_write_block(cp, cp_blk_no++);
	ASSERT(ret >= 0);

	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);
}

static void fix_checkpoints(struct f2fs_sb_info *sbi)
{
	/* copy valid checkpoint to its mirror position */
	duplicate_checkpoint(sbi);

	/* repair checkpoint at CP #0 position */
	sbi->cur_cp = 1;
	fix_checkpoint(sbi);
}

#ifdef HAVE_LINUX_BLKZONED_H

/*
 * Refer valid block map and return offset of the last valid block in the zone.
 * Obtain valid block map from SIT and fsync data.
 * If there is no valid block in the zone, return -1.
 */
static int last_vblk_off_in_zone(struct f2fs_sb_info *sbi,
				 unsigned int zone_segno)
{
	int s, b;
	unsigned int segs_per_zone = sbi->segs_per_sec * sbi->secs_per_zone;
	struct seg_entry *se;

	for (s = segs_per_zone - 1; s >= 0; s--) {
		se = get_seg_entry(sbi, zone_segno + s);

		/*
		 * Refer not cur_valid_map but ckpt_valid_map which reflects
		 * fsync data.
		 */
		ASSERT(se->ckpt_valid_map);
		for (b = sbi->blocks_per_seg - 1; b >= 0; b--)
			if (f2fs_test_bit(b, (const char*)se->ckpt_valid_map))
				return b + (s << sbi->log_blocks_per_seg);
	}

	return -1;
}

static int check_curseg_write_pointer(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct blk_zone blkz;
	block_t cs_block, wp_block, zone_last_vblock;
	uint64_t cs_sector, wp_sector;
	int i, ret;
	unsigned int zone_segno;
	int log_sectors_per_block = sbi->log_blocksize - SECTOR_SHIFT;

	/* get the device the curseg points to */
	cs_block = START_BLOCK(sbi, curseg->segno) + curseg->next_blkoff;
	for (i = 0; i < MAX_DEVICES; i++) {
		if (!c.devices[i].path)
			break;
		if (c.devices[i].start_blkaddr <= cs_block &&
		    cs_block <= c.devices[i].end_blkaddr)
			break;
	}

	if (i >= MAX_DEVICES)
		return -EINVAL;

	if (c.devices[i].zoned_model != F2FS_ZONED_HM)
		return 0;

	/* get write pointer position of the zone the curseg points to */
	cs_sector = (cs_block - c.devices[i].start_blkaddr)
		<< log_sectors_per_block;
	ret = f2fs_report_zone(i, cs_sector, &blkz);
	if (ret)
		return ret;

	if (blk_zone_type(&blkz) != BLK_ZONE_TYPE_SEQWRITE_REQ)
		return 0;

	/* check consistency between the curseg and the write pointer */
	wp_block = c.devices[i].start_blkaddr +
		(blk_zone_wp_sector(&blkz) >> log_sectors_per_block);
	wp_sector = blk_zone_wp_sector(&blkz);

	if (cs_sector == wp_sector)
		return 0;

	if (cs_sector > wp_sector) {
		MSG(0, "Inconsistent write pointer with curseg %d: "
		    "curseg %d[0x%x,0x%x] > wp[0x%x,0x%x]\n",
		    type, type, curseg->segno, curseg->next_blkoff,
		    GET_SEGNO(sbi, wp_block), OFFSET_IN_SEG(sbi, wp_block));
		fsck->chk.wp_inconsistent_zones++;
		return -EINVAL;
	}

	MSG(0, "Write pointer goes advance from curseg %d: "
	    "curseg %d[0x%x,0x%x] wp[0x%x,0x%x]\n",
	    type, type, curseg->segno, curseg->next_blkoff,
	    GET_SEGNO(sbi, wp_block), OFFSET_IN_SEG(sbi, wp_block));

	zone_segno = GET_SEG_FROM_SEC(sbi,
				      GET_SEC_FROM_SEG(sbi, curseg->segno));
	zone_last_vblock = START_BLOCK(sbi, zone_segno) +
		last_vblk_off_in_zone(sbi, zone_segno);

	/*
	 * If valid blocks exist between the curseg position and the write
	 * pointer, they are fsync data. This is not an error to fix. Leave it
	 * for kernel to recover later.
	 * If valid blocks exist between the curseg's zone start and the curseg
	 * position, or if there is no valid block in the curseg's zone, fix
	 * the inconsistency between the curseg and the writ pointer.
	 * Of Note is that if there is no valid block in the curseg's zone,
	 * last_vblk_off_in_zone() returns -1 and zone_last_vblock is always
	 * smaller than cs_block.
	 */
	if (cs_block <= zone_last_vblock && zone_last_vblock < wp_block) {
		MSG(0, "Curseg has fsync data: curseg %d[0x%x,0x%x] "
		    "last valid block in zone[0x%x,0x%x]\n",
		    type, curseg->segno, curseg->next_blkoff,
		    GET_SEGNO(sbi, zone_last_vblock),
		    OFFSET_IN_SEG(sbi, zone_last_vblock));
		return 0;
	}

	fsck->chk.wp_inconsistent_zones++;
	return -EINVAL;
}

#else

static int check_curseg_write_pointer(struct f2fs_sb_info *UNUSED(sbi),
					int UNUSED(type))
{
	return 0;
}

#endif

int check_curseg_offset(struct f2fs_sb_info *sbi, int type)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct seg_entry *se;
	int j, nblocks;

	if (get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO) &&
			type != CURSEG_HOT_DATA && type != CURSEG_HOT_NODE)
		return 0;

	if ((curseg->next_blkoff >> 3) >= SIT_VBLOCK_MAP_SIZE) {
		ASSERT_MSG("Next block offset:%u is invalid, type:%d",
			curseg->next_blkoff, type);
		return -EINVAL;
	}
	se = get_seg_entry(sbi, curseg->segno);
	if (f2fs_test_bit(curseg->next_blkoff,
				(const char *)se->cur_valid_map)) {
		ASSERT_MSG("Next block offset is not free, type:%d", type);
		return -EINVAL;
	}
	if (curseg->alloc_type == SSR)
		return 0;

	nblocks = sbi->blocks_per_seg;
	for (j = curseg->next_blkoff + 1; j < nblocks; j++) {
		if (f2fs_test_bit(j, (const char *)se->cur_valid_map)) {
			ASSERT_MSG("For LFS curseg, space after .next_blkoff "
				"should be unused, type:%d", type);
			return -EINVAL;
		}
	}

	if (c.zoned_model == F2FS_ZONED_HM)
		return check_curseg_write_pointer(sbi, type);

	return 0;
}

int check_curseg_offsets(struct f2fs_sb_info *sbi)
{
	int i, ret;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		ret = check_curseg_offset(sbi, i);
		if (ret)
			return ret;
	}
	return 0;
}

static void fix_curseg_info(struct f2fs_sb_info *sbi)
{
	int i, need_update = 0;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		if (check_curseg_offset(sbi, i)) {
			update_curseg_info(sbi, i);
			need_update = 1;
		}
	}

	if (need_update) {
		write_curseg_info(sbi);
		flush_curseg_sit_entries(sbi);
	}
}

int check_sit_types(struct f2fs_sb_info *sbi)
{
	unsigned int i;
	int err = 0;

	for (i = 0; i < MAIN_SEGS(sbi); i++) {
		struct seg_entry *se;

		se = get_seg_entry(sbi, i);
		if (se->orig_type != se->type) {
			if (se->orig_type == CURSEG_COLD_DATA &&
					se->type <= CURSEG_COLD_DATA) {
				se->type = se->orig_type;
			} else {
				FIX_MSG("Wrong segment type [0x%x] %x -> %x",
						i, se->orig_type, se->type);
				err = -EINVAL;
			}
		}
	}
	return err;
}

static struct f2fs_node *fsck_get_lpf(struct f2fs_sb_info *sbi)
{
	struct f2fs_node *node;
	struct node_info ni;
	nid_t lpf_ino;
	int err;

	/* read root inode first */
	node = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node);
	get_node_info(sbi, F2FS_ROOT_INO(sbi), &ni);
	err = dev_read_block(node, ni.blk_addr);
	ASSERT(err >= 0);

	/* lookup lost+found in root directory */
	lpf_ino = f2fs_lookup(sbi, node, (u8 *)LPF, strlen(LPF));
	if (lpf_ino) { /* found */
		get_node_info(sbi, lpf_ino, &ni);
		err = dev_read_block(node, ni.blk_addr);
		ASSERT(err >= 0);
		DBG(1, "Found lost+found 0x%x at blkaddr [0x%x]\n",
		    lpf_ino, ni.blk_addr);
		if (!S_ISDIR(le16_to_cpu(node->i.i_mode))) {
			ASSERT_MSG("lost+found is not directory [0%o]\n",
				   le16_to_cpu(node->i.i_mode));
			/* FIXME: give up? */
			goto out;
		}
	} else { /* not found, create it */
		struct dentry de;

		memset(&de, 0, sizeof(de));
		de.name = (u8 *) LPF;
		de.len = strlen(LPF);
		de.mode = 0x41c0;
		de.pino = F2FS_ROOT_INO(sbi),
		de.file_type = F2FS_FT_DIR,
		de.uid = getuid();
		de.gid = getgid();
		de.mtime = time(NULL);

		err = f2fs_mkdir(sbi, &de);
		if (err) {
			ASSERT_MSG("Failed create lost+found");
			goto out;
		}

		get_node_info(sbi, de.ino, &ni);
		err = dev_read_block(node, ni.blk_addr);
		ASSERT(err >= 0);
		DBG(1, "Create lost+found 0x%x at blkaddr [0x%x]\n",
		    de.ino, ni.blk_addr);
	}

	c.lpf_ino = le32_to_cpu(node->footer.ino);
	return node;
out:
	free(node);
	return NULL;
}

static int fsck_do_reconnect_file(struct f2fs_sb_info *sbi,
				  struct f2fs_node *lpf,
				  struct f2fs_node *fnode)
{
	char name[80];
	size_t namelen;
	nid_t ino = le32_to_cpu(fnode->footer.ino);
	struct node_info ni;
	int ftype, ret;

	namelen = snprintf(name, 80, "%u", ino);
	if (namelen >= 80)
		/* ignore terminating '\0', should never happen */
		namelen = 79;

	if (f2fs_lookup(sbi, lpf, (u8 *)name, namelen)) {
		ASSERT_MSG("Name %s already exist in lost+found", name);
		return -EEXIST;
	}

	get_node_info(sbi, le32_to_cpu(lpf->footer.ino), &ni);
	ftype = map_de_type(le16_to_cpu(fnode->i.i_mode));
	ret = f2fs_add_link(sbi, lpf, (unsigned char *)name, namelen,
			    ino, ftype, ni.blk_addr, 0);
	if (ret) {
		ASSERT_MSG("Failed to add inode [0x%x] to lost+found", ino);
		return -EINVAL;
	}

	/* update fnode */
	memcpy(fnode->i.i_name, name, namelen);
	fnode->i.i_namelen = cpu_to_le32(namelen);
	fnode->i.i_pino = c.lpf_ino;
	get_node_info(sbi, le32_to_cpu(fnode->footer.ino), &ni);
	ret = dev_write_block(fnode, ni.blk_addr);
	ASSERT(ret >= 0);

	DBG(1, "Reconnect inode [0x%x] to lost+found\n", ino);
	return 0;
}

static void fsck_failed_reconnect_file_dnode(struct f2fs_sb_info *sbi,
					     nid_t nid)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_node *node;
	struct node_info ni;
	u32 addr;
	int i, err;

	node = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node);

	get_node_info(sbi, nid, &ni);
	err = dev_read_block(node, ni.blk_addr);
	ASSERT(err >= 0);

	fsck->chk.valid_node_cnt--;
	fsck->chk.valid_blk_cnt--;
	f2fs_clear_main_bitmap(sbi, ni.blk_addr);

	for (i = 0; i < ADDRS_PER_BLOCK(&node->i); i++) {
		addr = le32_to_cpu(node->dn.addr[i]);
		if (!addr)
			continue;
		fsck->chk.valid_blk_cnt--;
		if (addr == NEW_ADDR)
			continue;
		f2fs_clear_main_bitmap(sbi, addr);
	}

	free(node);
}

static void fsck_failed_reconnect_file_idnode(struct f2fs_sb_info *sbi,
					      nid_t nid)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_node *node;
	struct node_info ni;
	nid_t tmp;
	int i, err;

	node = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node);

	get_node_info(sbi, nid, &ni);
	err = dev_read_block(node, ni.blk_addr);
	ASSERT(err >= 0);

	fsck->chk.valid_node_cnt--;
	fsck->chk.valid_blk_cnt--;
	f2fs_clear_main_bitmap(sbi, ni.blk_addr);

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		tmp = le32_to_cpu(node->in.nid[i]);
		if (!tmp)
			continue;
		fsck_failed_reconnect_file_dnode(sbi, tmp);
	}

	free(node);
}

static void fsck_failed_reconnect_file_didnode(struct f2fs_sb_info *sbi,
					       nid_t nid)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_node *node;
	struct node_info ni;
	nid_t tmp;
	int i, err;

	node = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node);

	get_node_info(sbi, nid, &ni);
	err = dev_read_block(node, ni.blk_addr);
	ASSERT(err >= 0);

	fsck->chk.valid_node_cnt--;
	fsck->chk.valid_blk_cnt--;
	f2fs_clear_main_bitmap(sbi, ni.blk_addr);

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		tmp = le32_to_cpu(node->in.nid[i]);
		if (!tmp)
			continue;
		fsck_failed_reconnect_file_idnode(sbi, tmp);
	}

	free(node);
}

/*
 * Counters and main_area_bitmap are already changed during checking
 * inode block, so clear them. There is no need to clear new blocks
 * allocted to lost+found.
 */
static void fsck_failed_reconnect_file(struct f2fs_sb_info *sbi, nid_t ino)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_node *node;
	struct node_info ni;
	nid_t nid;
	int ofs, i, err;

	node = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node);

	get_node_info(sbi, ino, &ni);
	err = dev_read_block(node, ni.blk_addr);
	ASSERT(err >= 0);

	/* clear inode counters */
	fsck->chk.valid_inode_cnt--;
	fsck->chk.valid_node_cnt--;
	fsck->chk.valid_blk_cnt--;
	f2fs_clear_main_bitmap(sbi, ni.blk_addr);

	/* clear xnid counters */
	if (node->i.i_xattr_nid) {
		nid = le32_to_cpu(node->i.i_xattr_nid);
		fsck->chk.valid_node_cnt--;
		fsck->chk.valid_blk_cnt--;
		get_node_info(sbi, nid, &ni);
		f2fs_clear_main_bitmap(sbi, ni.blk_addr);
	}

	/* clear data counters */
	if(!(node->i.i_inline & F2FS_INLINE_DATA)) {
		ofs = get_extra_isize(node);
		for (i = 0; i < ADDRS_PER_INODE(&node->i); i++) {
			block_t addr = le32_to_cpu(node->i.i_addr[ofs + i]);
			if (!addr)
				continue;
			fsck->chk.valid_blk_cnt--;
			if (addr == NEW_ADDR)
				continue;
			f2fs_clear_main_bitmap(sbi, addr);
		}
	}

	for (i = 0; i < 5; i++) {
		nid = le32_to_cpu(node->i.i_nid[i]);
		if (!nid)
			continue;

		switch (i) {
		case 0: /* direct node */
		case 1:
			fsck_failed_reconnect_file_dnode(sbi, nid);
			break;
		case 2: /* indirect node */
		case 3:
			fsck_failed_reconnect_file_idnode(sbi, nid);
			break;
		case 4: /* double indirect node */
			fsck_failed_reconnect_file_didnode(sbi, nid);
			break;
		}
	}

	free(node);
}

/*
 * Scan unreachable nids and find only regular file inodes. If these files
 * are not corrupted, reconnect them to lost+found.
 *
 * Since all unreachable nodes are already checked, we can allocate new
 * blocks safely.
 *
 * This function returns the number of files been reconnected.
 */
static int fsck_reconnect_file(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_node *lpf_node, *node;
	struct node_info ni;
	char *reconnect_bitmap;
	u32 blk_cnt;
	struct f2fs_compr_blk_cnt cbc;
	nid_t nid;
	int err, cnt = 0, ftype;

	node = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node);

	reconnect_bitmap = calloc(fsck->nat_area_bitmap_sz, 1);
	ASSERT(reconnect_bitmap);

	for (nid = 0; nid < fsck->nr_nat_entries; nid++) {
		if (f2fs_test_bit(nid, fsck->nat_area_bitmap)) {
			if (is_qf_ino(F2FS_RAW_SUPER(sbi), nid)) {
				DBG(1, "Not support quota inode [0x%x]\n",
				    nid);
				continue;
			}

			get_node_info(sbi, nid, &ni);
			err = dev_read_block(node, ni.blk_addr);
			ASSERT(err >= 0);

			/* reconnection will restore these nodes if needed */
			if (node->footer.ino != node->footer.nid) {
				DBG(1, "Not support non-inode node [0x%x]\n",
				    nid);
				continue;
			}

			if (S_ISDIR(le16_to_cpu(node->i.i_mode))) {
				DBG(1, "Not support directory inode [0x%x]\n",
				    nid);
				continue;
			}

			ftype = map_de_type(le16_to_cpu(node->i.i_mode));
			if (sanity_check_nid(sbi, nid, node, ftype,
					     TYPE_INODE, &ni)) {
				ASSERT_MSG("Invalid nid [0x%x]\n", nid);
				continue;
			}

			DBG(1, "Check inode 0x%x\n", nid);
			blk_cnt = 1;
			cbc.cnt = 0;
			cbc.cheader_pgofs = CHEADER_PGOFS_NONE;
			fsck_chk_inode_blk(sbi, nid, ftype, node,
					   &blk_cnt, &cbc, &ni, NULL);

			f2fs_set_bit(nid, reconnect_bitmap);
		}
	}

	lpf_node = fsck_get_lpf(sbi);
	if (!lpf_node)
		goto out;

	for (nid = 0; nid < fsck->nr_nat_entries; nid++) {
		if (f2fs_test_bit(nid, reconnect_bitmap)) {
			get_node_info(sbi, nid, &ni);
			err = dev_read_block(node, ni.blk_addr);
			ASSERT(err >= 0);

			if (fsck_do_reconnect_file(sbi, lpf_node, node)) {
				DBG(1, "Failed to reconnect inode [0x%x]\n",
				    nid);
				fsck_failed_reconnect_file(sbi, nid);
				continue;
			}

			quota_add_inode_usage(fsck->qctx, nid, &node->i);

			DBG(1, "Reconnected inode [0x%x] to lost+found\n", nid);
			cnt++;
		}
	}

out:
	free(node);
	free(lpf_node);
	free(reconnect_bitmap);
	return cnt;
}

#ifdef HAVE_LINUX_BLKZONED_H

struct write_pointer_check_data {
	struct f2fs_sb_info *sbi;
	int dev_index;
};

static int chk_and_fix_wp_with_sit(int UNUSED(i), void *blkzone, void *opaque)
{
	struct blk_zone *blkz = (struct blk_zone *)blkzone;
	struct write_pointer_check_data *wpd = opaque;
	struct f2fs_sb_info *sbi = wpd->sbi;
	struct device_info *dev = c.devices + wpd->dev_index;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	block_t zone_block, wp_block, wp_blkoff;
	unsigned int zone_segno, wp_segno;
	struct curseg_info *cs;
	int cs_index, ret, last_valid_blkoff;
	int log_sectors_per_block = sbi->log_blocksize - SECTOR_SHIFT;
	unsigned int segs_per_zone = sbi->segs_per_sec * sbi->secs_per_zone;

	if (blk_zone_conv(blkz))
		return 0;

	zone_block = dev->start_blkaddr
		+ (blk_zone_sector(blkz) >> log_sectors_per_block);
	zone_segno = GET_SEGNO(sbi, zone_block);
	if (zone_segno >= MAIN_SEGS(sbi))
		return 0;

	wp_block = dev->start_blkaddr
		+ (blk_zone_wp_sector(blkz) >> log_sectors_per_block);
	wp_segno = GET_SEGNO(sbi, wp_block);
	wp_blkoff = wp_block - START_BLOCK(sbi, wp_segno);

	/* if a curseg points to the zone, skip the check */
	for (cs_index = 0; cs_index < NO_CHECK_TYPE; cs_index++) {
		cs = &SM_I(sbi)->curseg_array[cs_index];
		if (zone_segno <= cs->segno &&
		    cs->segno < zone_segno + segs_per_zone)
			return 0;
	}

	last_valid_blkoff = last_vblk_off_in_zone(sbi, zone_segno);

	/*
	 * When there is no valid block in the zone, check write pointer is
	 * at zone start. If not, reset the write pointer.
	 */
	if (last_valid_blkoff < 0 &&
	    blk_zone_wp_sector(blkz) != blk_zone_sector(blkz)) {
		if (!c.fix_on) {
			MSG(0, "Inconsistent write pointer: wp[0x%x,0x%x]\n",
			    wp_segno, wp_blkoff);
			fsck->chk.wp_inconsistent_zones++;
			return 0;
		}

		FIX_MSG("Reset write pointer of zone at segment 0x%x",
			zone_segno);
		ret = f2fs_reset_zone(wpd->dev_index, blkz);
		if (ret) {
			printf("[FSCK] Write pointer reset failed: %s\n",
			       dev->path);
			return ret;
		}
		fsck->chk.wp_fixed = 1;
		return 0;
	}

	/*
	 * If valid blocks exist in the zone beyond the write pointer, it
	 * is a bug. No need to fix because the zone is not selected for the
	 * write. Just report it.
	 */
	if (last_valid_blkoff + zone_block > wp_block) {
		MSG(0, "Unexpected invalid write pointer: wp[0x%x,0x%x]\n",
		    wp_segno, wp_blkoff);
		return 0;
	}

	return 0;
}

static void fix_wp_sit_alignment(struct f2fs_sb_info *sbi)
{
	unsigned int i;
	struct write_pointer_check_data wpd = {	sbi, 0 };

	if (c.zoned_model != F2FS_ZONED_HM)
		return;

	for (i = 0; i < MAX_DEVICES; i++) {
		if (!c.devices[i].path)
			break;
		if (c.devices[i].zoned_model != F2FS_ZONED_HM)
			break;

		wpd.dev_index = i;
		if (f2fs_report_zones(i, chk_and_fix_wp_with_sit, &wpd)) {
			printf("[FSCK] Write pointer check failed: %s\n",
			       c.devices[i].path);
			return;
		}
	}
}

#else

static void fix_wp_sit_alignment(struct f2fs_sb_info *UNUSED(sbi))
{
	return;
}

#endif

/*
 * Check and fix consistency with write pointers at the beginning of
 * fsck so that following writes by fsck do not fail.
 */
void fsck_chk_and_fix_write_pointers(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	if (c.zoned_model != F2FS_ZONED_HM)
		return;

	if (check_curseg_offsets(sbi) && c.fix_on) {
		fix_curseg_info(sbi);
		fsck->chk.wp_fixed = 1;
	}

	fix_wp_sit_alignment(sbi);
}

int fsck_chk_curseg_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct curseg_info *curseg;
	struct seg_entry *se;
	struct f2fs_summary_block *sum_blk;
	int i, ret = 0;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		curseg = CURSEG_I(sbi, i);
		se = get_seg_entry(sbi, curseg->segno);
		sum_blk = curseg->sum_blk;

		if ((get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO)) &&
			(i != CURSEG_HOT_DATA && i != CURSEG_HOT_NODE))
			continue;

		if (se->type != i) {
			ASSERT_MSG("Incorrect curseg [%d]: segno [0x%x] "
				   "type(SIT) [%d]", i, curseg->segno,
				   se->type);
			if (c.fix_on || c.preen_mode)
				se->type = i;
			ret = -1;
		}
		if (i <= CURSEG_COLD_DATA && IS_SUM_DATA_SEG(sum_blk->footer)) {
			continue;
		} else if (i > CURSEG_COLD_DATA && IS_SUM_NODE_SEG(sum_blk->footer)) {
			continue;
		} else {
			ASSERT_MSG("Incorrect curseg [%d]: segno [0x%x] "
				   "type(SSA) [%d]", i, curseg->segno,
				   sum_blk->footer.entry_type);
			if (c.fix_on || c.preen_mode)
				sum_blk->footer.entry_type =
					i <= CURSEG_COLD_DATA ?
					SUM_TYPE_DATA : SUM_TYPE_NODE;
			ret = -1;
		}
	}

	return ret;
}

int fsck_verify(struct f2fs_sb_info *sbi)
{
	unsigned int i = 0;
	int ret = 0;
	int force = 0;
	u32 nr_unref_nid = 0;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct hard_link_node *node = NULL;
	bool verify_failed = false;
	uint64_t max_blks, data_secs, node_secs, free_blks;

	if (c.show_file_map)
		return 0;

	printf("\n");

	if (c.zoned_model == F2FS_ZONED_HM) {
		printf("[FSCK] Write pointers consistency                    ");
		if (fsck->chk.wp_inconsistent_zones == 0x0) {
			printf(" [Ok..]\n");
		} else {
			printf(" [Fail] [0x%x]\n",
			       fsck->chk.wp_inconsistent_zones);
			verify_failed = true;
		}

		if (fsck->chk.wp_fixed && c.fix_on)
			force = 1;
	}

	if (c.feature & cpu_to_le32(F2FS_FEATURE_LOST_FOUND)) {
		for (i = 0; i < fsck->nr_nat_entries; i++)
			if (f2fs_test_bit(i, fsck->nat_area_bitmap) != 0)
				break;
		if (i < fsck->nr_nat_entries) {
			i = fsck_reconnect_file(sbi);
			printf("[FSCK] Reconnect %u files to lost+found\n", i);
		}
	}

	for (i = 0; i < fsck->nr_nat_entries; i++) {
		if (f2fs_test_bit(i, fsck->nat_area_bitmap) != 0) {
			struct node_info ni;

			get_node_info(sbi, i, &ni);
			printf("NID[0x%x] is unreachable, blkaddr:0x%x\n",
							i, ni.blk_addr);
			nr_unref_nid++;
		}
	}

	if (fsck->hard_link_list_head != NULL) {
		node = fsck->hard_link_list_head;
		while (node) {
			printf("NID[0x%x] has [0x%x] more unreachable links\n",
					node->nid, node->links);
			node = node->next;
		}
		c.bug_on = 1;
	}

	data_secs = round_up(sbi->total_valid_node_count, BLKS_PER_SEC(sbi));
	node_secs = round_up(sbi->total_valid_block_count -
				sbi->total_valid_node_count, BLKS_PER_SEC(sbi));
	free_blks = (sbi->total_sections - data_secs - node_secs) *
							BLKS_PER_SEC(sbi);
	max_blks = SM_I(sbi)->main_blkaddr + (data_secs + node_secs) *
							BLKS_PER_SEC(sbi);
	printf("[FSCK] Max image size: %"PRIu64" MB, Free space: %"PRIu64" MB\n",
						max_blks >> 8, free_blks >> 8);
	printf("[FSCK] Unreachable nat entries                       ");
	if (nr_unref_nid == 0x0) {
		printf(" [Ok..] [0x%x]\n", nr_unref_nid);
	} else {
		printf(" [Fail] [0x%x]\n", nr_unref_nid);
		verify_failed = true;
	}

	printf("[FSCK] SIT valid block bitmap checking                ");
	if (memcmp(fsck->sit_area_bitmap, fsck->main_area_bitmap,
					fsck->sit_area_bitmap_sz) == 0x0) {
		printf("[Ok..]\n");
	} else {
		printf("[Fail]\n");
		verify_failed = true;
	}

	printf("[FSCK] Hard link checking for regular file           ");
	if (fsck->hard_link_list_head == NULL) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.multi_hard_link_files);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.multi_hard_link_files);
		verify_failed = true;
	}

	printf("[FSCK] valid_block_count matching with CP            ");
	if (sbi->total_valid_block_count == fsck->chk.valid_blk_cnt) {
		printf(" [Ok..] [0x%x]\n", (u32)fsck->chk.valid_blk_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", (u32)fsck->chk.valid_blk_cnt);
		verify_failed = true;
	}

	printf("[FSCK] valid_node_count matching with CP (de lookup) ");
	if (sbi->total_valid_node_count == fsck->chk.valid_node_cnt) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.valid_node_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.valid_node_cnt);
		verify_failed = true;
	}

	printf("[FSCK] valid_node_count matching with CP (nat lookup)");
	if (sbi->total_valid_node_count == fsck->chk.valid_nat_entry_cnt) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.valid_nat_entry_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.valid_nat_entry_cnt);
		verify_failed = true;
	}

	printf("[FSCK] valid_inode_count matched with CP             ");
	if (sbi->total_valid_inode_count == fsck->chk.valid_inode_cnt) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.valid_inode_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.valid_inode_cnt);
		verify_failed = true;
	}

	printf("[FSCK] free segment_count matched with CP            ");
	if (le32_to_cpu(F2FS_CKPT(sbi)->free_segment_count) ==
						fsck->chk.sit_free_segs) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.sit_free_segs);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.sit_free_segs);
		verify_failed = true;
	}

	printf("[FSCK] next block offset is free                     ");
	if (check_curseg_offsets(sbi) == 0) {
		printf(" [Ok..]\n");
	} else {
		printf(" [Fail]\n");
		verify_failed = true;
	}

	printf("[FSCK] fixing SIT types\n");
	if (check_sit_types(sbi) != 0)
		force = 1;

	printf("[FSCK] other corrupted bugs                          ");
	if (c.bug_on == 0) {
		printf(" [Ok..]\n");
	} else {
		printf(" [Fail]\n");
		ret = EXIT_ERR_CODE;
	}

	if (verify_failed) {
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

#ifndef WITH_ANDROID
	if (nr_unref_nid && !c.ro) {
		char ans[255] = {0};
		int res;

		printf("\nDo you want to restore lost files into ./lost_found/? [Y/N] ");
		res = scanf("%s", ans);
		ASSERT(res >= 0);
		if (!strcasecmp(ans, "y")) {
			for (i = 0; i < fsck->nr_nat_entries; i++) {
				if (f2fs_test_bit(i, fsck->nat_area_bitmap))
					dump_node(sbi, i, 1);
			}
		}
	}
#endif

	/* fix global metadata */
	if (force || (c.fix_on && f2fs_dev_is_writable())) {
		struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
		struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

		if (force || c.bug_on || c.bug_nat_bits || c.quota_fixed) {
			/* flush nats to write_nit_bits below */
			flush_journal_entries(sbi);
			fix_hard_links(sbi);
			fix_nat_entries(sbi);
			rewrite_sit_area_bitmap(sbi);
			fix_wp_sit_alignment(sbi);
			fix_curseg_info(sbi);
			fix_checksum(sbi);
			fix_checkpoints(sbi);
		} else if (is_set_ckpt_flags(cp, CP_FSCK_FLAG) ||
			is_set_ckpt_flags(cp, CP_QUOTA_NEED_FSCK_FLAG)) {
			write_checkpoints(sbi);
		}

		if (c.abnormal_stop)
			memset(sb->s_stop_reason, 0, MAX_STOP_REASON);

		if (c.fs_errors)
			memset(sb->s_errors, 0, MAX_F2FS_ERRORS);

		if (c.abnormal_stop || c.fs_errors)
			update_superblock(sb, SB_MASK_ALL);

		/* to return FSCK_ERROR_CORRECTED */
		ret = 0;
	}
	return ret;
}

void fsck_free(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	if (fsck->qctx)
		quota_release_context(&fsck->qctx);

	if (fsck->main_area_bitmap)
		free(fsck->main_area_bitmap);

	if (fsck->nat_area_bitmap)
		free(fsck->nat_area_bitmap);

	if (fsck->sit_area_bitmap)
		free(fsck->sit_area_bitmap);

	if (fsck->entries)
		free(fsck->entries);

	if (tree_mark)
		free(tree_mark);

	while (fsck->dentry) {
		struct f2fs_dentry *dentry = fsck->dentry;

		fsck->dentry = fsck->dentry->next;
		free(dentry);
	}
}
