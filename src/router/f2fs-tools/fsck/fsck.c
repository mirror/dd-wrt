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

char *tree_mark;
uint32_t tree_mark_size = 256;

static inline int f2fs_set_main_bitmap(struct f2fs_sb_info *sbi, u32 blk,
								int type)
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

static inline int f2fs_test_sit_bitmap(struct f2fs_sb_info *sbi, u32 blk)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

	return f2fs_test_bit(BLKOFF_FROM_MAIN(sbi, blk), fsck->sit_area_bitmap);
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
	struct f2fs_summary_block *sum_blk;
	struct f2fs_summary *sum_entry;
	struct seg_entry * se;
	u32 segno, offset;
	int need_fix = 0, ret = 0;
	int type;

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
	if (need_fix && !c.ro) {
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
	if (node_blk->footer.nid == node_blk->footer.ino)
		target_blk_addr = node_blk->i.i_addr[ofs_in_node];
	else
		target_blk_addr = node_blk->dn.addr[ofs_in_node];

	if (blk_addr == le32_to_cpu(target_blk_addr))
		ret = 1;
out:
	free(node_blk);
	return ret;
}

static int is_valid_ssa_data_blk(struct f2fs_sb_info *sbi, u32 blk_addr,
		u32 parent_nid, u16 idx_in_node, u8 version)
{
	struct f2fs_summary_block *sum_blk;
	struct f2fs_summary *sum_entry;
	struct seg_entry * se;
	u32 segno, offset;
	int need_fix = 0, ret = 0;
	int type;

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
	if (need_fix && !c.ro) {
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

static int __check_inode_mode(u32 nid, enum FILE_TYPE ftype, u32 mode)
{
	if (ftype >= F2FS_FT_MAX)
		return 0;
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
	ASSERT_MSG("mismatch i_mode [0x%x] [0x%x vs. 0x%x]", nid, ftype, mode);
	return -1;
}

static int sanity_check_nid(struct f2fs_sb_info *sbi, u32 nid,
			struct f2fs_node *node_blk,
			enum FILE_TYPE ftype, enum NODE_TYPE ntype,
			struct node_info *ni, u8 *name)
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
		__check_inode_mode(nid, ftype, le32_to_cpu(node_blk->i.i_mode)))
		return -EINVAL;

	/* workaround to fix later */
	if (ftype != F2FS_FT_ORPHAN ||
			f2fs_test_bit(nid, fsck->nat_area_bitmap) != 0)
		f2fs_clear_bit(nid, fsck->nat_area_bitmap);
	else
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
	}
	return 0;
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
				F2FS_FT_XATTR, TYPE_XATTR, &ni, NULL)) {
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
		u32 nid, u8 *name, enum FILE_TYPE ftype, enum NODE_TYPE ntype,
		u32 *blk_cnt, struct child_info *child)
{
	struct node_info ni;
	struct f2fs_node *node_blk = NULL;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk != NULL);

	if (sanity_check_nid(sbi, nid, node_blk, ftype, ntype, &ni, name))
		goto err;

	if (ntype == TYPE_INODE) {
		fsck_chk_inode_blk(sbi, nid, ftype, node_blk, blk_cnt, &ni);
	} else {
		switch (ntype) {
		case TYPE_DIRECT_NODE:
			f2fs_set_main_bitmap(sbi, ni.blk_addr,
							CURSEG_WARM_NODE);
			fsck_chk_dnode_blk(sbi, inode, nid, ftype, node_blk,
					blk_cnt, child, &ni);
			break;
		case TYPE_INDIRECT_NODE:
			f2fs_set_main_bitmap(sbi, ni.blk_addr,
							CURSEG_COLD_NODE);
			fsck_chk_idnode_blk(sbi, inode, ftype, node_blk,
					blk_cnt, child);
			break;
		case TYPE_DOUBLE_INDIRECT_NODE:
			f2fs_set_main_bitmap(sbi, ni.blk_addr,
							CURSEG_COLD_NODE);
			fsck_chk_didnode_blk(sbi, inode, ftype, node_blk,
					blk_cnt, child);
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

/* start with valid nid and blkaddr */
void fsck_chk_inode_blk(struct f2fs_sb_info *sbi, u32 nid,
		enum FILE_TYPE ftype, struct f2fs_node *node_blk,
		u32 *blk_cnt, struct node_info *ni)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct child_info child;
	enum NODE_TYPE ntype;
	u32 i_links = le32_to_cpu(node_blk->i.i_links);
	u64 i_size = le64_to_cpu(node_blk->i.i_size);
	u64 i_blocks = le64_to_cpu(node_blk->i.i_blocks);
	unsigned char en[F2FS_NAME_LEN + 1];
	int namelen;
	unsigned int idx = 0;
	int need_fix = 0;
	int ret;

	memset(&child, 0, sizeof(child));
	child.links = 2;
	child.p_ino = nid;
	child.pp_ino = le32_to_cpu(node_blk->i.i_pino);
	child.dir_level = node_blk->i.i_dir_level;

	if (f2fs_test_main_bitmap(sbi, ni->blk_addr) == 0)
		fsck->chk.valid_inode_cnt++;

	if (ftype == F2FS_FT_DIR) {
		f2fs_set_main_bitmap(sbi, ni->blk_addr, CURSEG_HOT_NODE);
	} else {
		if (f2fs_test_main_bitmap(sbi, ni->blk_addr) == 0) {
			f2fs_set_main_bitmap(sbi, ni->blk_addr,
							CURSEG_WARM_NODE);
			if (i_links > 1 && ftype != F2FS_FT_ORPHAN) {
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

	if (fsck_chk_xattr_blk(sbi, nid,
			le32_to_cpu(node_blk->i.i_xattr_nid), blk_cnt) &&
			c.fix_on) {
		node_blk->i.i_xattr_nid = 0;
		need_fix = 1;
		FIX_MSG("Remove xattr block: 0x%x, x_nid = 0x%x",
				nid, le32_to_cpu(node_blk->i.i_xattr_nid));
	}

	if (ftype == F2FS_FT_CHRDEV || ftype == F2FS_FT_BLKDEV ||
			ftype == F2FS_FT_FIFO || ftype == F2FS_FT_SOCK)
		goto check;

	if((node_blk->i.i_inline & F2FS_INLINE_DATA)) {
		if (le32_to_cpu(node_blk->i.i_addr[0]) != 0) {
			/* should fix this bug all the time */
			FIX_MSG("inline_data has wrong 0'th block = %x",
					le32_to_cpu(node_blk->i.i_addr[0]));
			node_blk->i.i_addr[0] = 0;
			node_blk->i.i_blocks = cpu_to_le64(*blk_cnt);
			need_fix = 1;
		}
		if (!(node_blk->i.i_inline & F2FS_DATA_EXIST)) {
			char buf[MAX_INLINE_DATA];
			memset(buf, 0, MAX_INLINE_DATA);

			if (memcmp(buf, &node_blk->i.i_addr[1],
							MAX_INLINE_DATA)) {
				FIX_MSG("inline_data has DATA_EXIST");
				node_blk->i.i_inline |= F2FS_DATA_EXIST;
				need_fix = 1;
			}
		}
		DBG(3, "ino[0x%x] has inline data!\n", nid);
		goto check;
	}
	if((node_blk->i.i_inline & F2FS_INLINE_DENTRY)) {
		DBG(3, "ino[0x%x] has inline dentry!\n", nid);
		ret = fsck_chk_inline_dentries(sbi, node_blk, &child);
		if (ret < 0) {
			/* should fix this bug all the time */
			need_fix = 1;
		}
		goto check;
	}

	/* readahead node blocks */
	for (idx = 0; idx < 5; idx++) {
		u32 nid = le32_to_cpu(node_blk->i.i_nid[idx]);

		if (nid != 0) {
			struct node_info ni;

			get_node_info(sbi, nid, &ni);
			if (IS_VALID_BLK_ADDR(sbi, ni.blk_addr))
				dev_reada_block(ni.blk_addr);
		}
	}

	/* init extent info */
	get_extent_info(&child.ei, &node_blk->i.i_ext);
	child.last_blk = 0;

	/* check data blocks in inode */
	for (idx = 0; idx < ADDRS_PER_INODE(&node_blk->i);
						idx++, child.pgofs++) {
		block_t blkaddr = le32_to_cpu(node_blk->i.i_addr[idx]);

		/* check extent info */
		check_extent_info(&child, blkaddr, 0);

		if (blkaddr != 0) {
			ret = fsck_chk_data_blk(sbi,
					blkaddr,
					&child, (i_blocks == *blk_cnt),
					ftype, nid, idx, ni->version,
					file_is_encrypt(&node_blk->i));
			if (!ret) {
				*blk_cnt = *blk_cnt + 1;
			} else if (c.fix_on) {
				node_blk->i.i_addr[idx] = 0;
				need_fix = 1;
				FIX_MSG("[0x%x] i_addr[%d] = 0", nid, idx);
			}
		}
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
				NULL, ftype, ntype, blk_cnt, &child);
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
				child.pgofs += ADDRS_PER_BLOCK;
			else if (ntype == TYPE_INDIRECT_NODE)
				child.pgofs += ADDRS_PER_BLOCK * NIDS_PER_BLOCK;
			else
				child.pgofs += ADDRS_PER_BLOCK *
						NIDS_PER_BLOCK * NIDS_PER_BLOCK;
		}

	}

	/* check uncovered range in the back of extent */
	check_extent_info(&child, 0, 1);

	if (child.state & FSCK_UNMATCHED_EXTENT) {
		ASSERT_MSG("ino: 0x%x has wrong ext: [pgofs:%u, blk:%u, len:%u]",
				nid, child.ei.fofs, child.ei.blk, child.ei.len);
		if (c.fix_on)
			need_fix = 1;
	}
check:
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
skip_blkcnt_fix:
	namelen = convert_encrypted_name(node_blk->i.i_name,
					le32_to_cpu(node_blk->i.i_namelen),
					en, file_is_encrypt(&node_blk->i));
	en[namelen] = '\0';
	if (ftype == F2FS_FT_ORPHAN)
		DBG(1, "Orphan Inode: 0x%x [%s] i_blocks: %u\n\n",
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
	if (ftype == F2FS_FT_SYMLINK && i_blocks && i_size == 0) {
		DBG(1, "ino: 0x%x i_blocks: %lu with zero i_size",
							nid, i_blocks);
		if (c.fix_on) {
			u64 i_size = i_blocks * F2FS_BLKSIZE;

			node_blk->i.i_size = cpu_to_le64(i_size);
			need_fix = 1;
			FIX_MSG("Symlink: recover 0x%x with i_size=%lu",
							nid, i_size);
		}
	}

	if (ftype == F2FS_FT_ORPHAN && i_links) {
		MSG(0, "ino: 0x%x is orphan inode, but has i_links: %u",
				nid, i_links);
		if (c.fix_on) {
			node_blk->i.i_links = 0;
			need_fix = 1;
			FIX_MSG("ino: 0x%x orphan_inode, i_links= 0x%x -> 0",
					nid, i_links);
		}
	}
	if (need_fix && !c.ro) {
		/* drop extent information to avoid potential wrong access */
		node_blk->i.i_ext.len = 0;
		ret = dev_write_block(node_blk, ni->blk_addr);
		ASSERT(ret >= 0);
	}
}

int fsck_chk_dnode_blk(struct f2fs_sb_info *sbi, struct f2fs_inode *inode,
		u32 nid, enum FILE_TYPE ftype, struct f2fs_node *node_blk,
		u32 *blk_cnt, struct child_info *child, struct node_info *ni)
{
	int idx, ret;
	int need_fix = 0;
	child->p_ino = nid;
	child->pp_ino = le32_to_cpu(inode->i_pino);

	for (idx = 0; idx < ADDRS_PER_BLOCK; idx++, child->pgofs++) {
		block_t blkaddr = le32_to_cpu(node_blk->dn.addr[idx]);

		check_extent_info(child, blkaddr, 0);

		if (blkaddr == 0x0)
			continue;
		ret = fsck_chk_data_blk(sbi,
			blkaddr, child,
			le64_to_cpu(inode->i_blocks) == *blk_cnt, ftype,
			nid, idx, ni->version,
			file_is_encrypt(inode));
		if (!ret) {
			*blk_cnt = *blk_cnt + 1;
		} else if (c.fix_on) {
			node_blk->dn.addr[idx] = 0;
			need_fix = 1;
			FIX_MSG("[0x%x] dn.addr[%d] = 0", nid, idx);
		}
	}
	if (need_fix && !c.ro) {
		ret = dev_write_block(node_blk, ni->blk_addr);
		ASSERT(ret >= 0);
	}
	return 0;
}

int fsck_chk_idnode_blk(struct f2fs_sb_info *sbi, struct f2fs_inode *inode,
		enum FILE_TYPE ftype, struct f2fs_node *node_blk, u32 *blk_cnt,
		struct child_info *child)
{
	int need_fix = 0, ret;
	int i = 0;

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		if (le32_to_cpu(node_blk->in.nid[i]) == 0x0)
			goto skip;
		ret = fsck_chk_node_blk(sbi, inode,
				le32_to_cpu(node_blk->in.nid[i]), NULL,
				ftype, TYPE_DIRECT_NODE, blk_cnt, child);
		if (!ret)
			*blk_cnt = *blk_cnt + 1;
		else if (ret == -EINVAL) {
			if (!c.fix_on)
				printf("should delete in.nid[i] = 0;\n");
			else {
				node_blk->in.nid[i] = 0;
				need_fix = 1;
				FIX_MSG("Set indirect node 0x%x -> 0\n", i);
			}
skip:
			child->pgofs += ADDRS_PER_BLOCK;
		}
	}

	if (need_fix && !c.ro) {
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
		struct child_info *child)
{
	int i = 0;
	int need_fix = 0, ret = 0;

	for (i = 0; i < NIDS_PER_BLOCK; i++) {
		if (le32_to_cpu(node_blk->in.nid[i]) == 0x0)
			goto skip;
		ret = fsck_chk_node_blk(sbi, inode,
				le32_to_cpu(node_blk->in.nid[i]), NULL,
				ftype, TYPE_INDIRECT_NODE, blk_cnt, child);
		if (!ret)
			*blk_cnt = *blk_cnt + 1;
		else if (ret == -EINVAL) {
			if (!c.fix_on)
				printf("should delete in.nid[i] = 0;\n");
			else {
				node_blk->in.nid[i] = 0;
				need_fix = 1;
				FIX_MSG("Set double indirect node 0x%x -> 0\n", i);
			}
skip:
			child->pgofs += ADDRS_PER_BLOCK * NIDS_PER_BLOCK;
		}
	}

	if (need_fix && !c.ro) {
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
 * digest_encode() -
 *
 * Encodes the input digest using characters from the set [a-zA-Z0-9_+].
 * The encoded string is roughly 4/3 times the size of the input string.
 */
static int digest_encode(const char *src, int len, char *dst)
{
	int i = 0, bits = 0, ac = 0;
	char *cp = dst;

	while (i < len) {
		ac += (((unsigned char) src[i]) << bits);
		bits += 8;
		do {
			*cp++ = lookup_table[ac & 0x3f];
			ac >>= 6;
			bits -= 6;
		} while (bits >= 6);
		i++;
	}
	if (bits)
		*cp++ = lookup_table[ac & 0x3f];
	*cp = 0;
	return cp - dst;
}

int convert_encrypted_name(unsigned char *name, int len,
				unsigned char *new, int encrypted)
{
	if (!encrypted) {
		memcpy(new, name, len);
		new[len] = 0;
		return len;
	}

	*new = '_';
	return digest_encode((const char *)name, 24, (char *)new + 1);
}

static void print_dentry(__u32 depth, __u8 *name,
		u8 *bitmap, struct f2fs_dir_entry *dentry,
		int max, int idx, int last_blk, int encrypted)
{
	int last_de = 0;
	int next_idx = 0;
	int name_len;
	unsigned int i;
	int bit_offset;
	unsigned char new[F2FS_NAME_LEN + 1];

	if (c.dbg_lv != -1)
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

	for (i = 1; i < depth; i++)
		printf("%c   ", tree_mark[i]);

	convert_encrypted_name(name, name_len, new, encrypted);

	printf("%c-- %s <ino = 0x%x>, <encrypted (%d)>\n",
			last_de ? '`' : '|',
			new, le32_to_cpu(dentry[idx].ino),
			encrypted);
}

static int f2fs_check_hash_code(struct f2fs_dir_entry *dentry,
			const unsigned char *name, u32 len, int encrypted)
{
	f2fs_hash_t hash_code = f2fs_dentry_hash(name, len);

	/* fix hash_code made by old buggy code */
	if (dentry->hash_code != hash_code) {
		unsigned char new[F2FS_NAME_LEN + 1];

		convert_encrypted_name((unsigned char *)name, len,
							new, encrypted);
		FIX_MSG("Mismatch hash_code for \"%s\" [%x:%x]",
				new, le32_to_cpu(dentry->hash_code),
				hash_code);
		dentry->hash_code = cpu_to_le32(hash_code);
		return 1;
	}
	return 0;
}

static unsigned int dir_buckets(unsigned int level, int dir_level)
{
	if (level + dir_level < MAX_DIR_HASH_DEPTH / 2)
		return 1 << (level + dir_level);
	else
		return MAX_DIR_BUCKETS;
}

static unsigned int bucket_blocks(unsigned int level)
{
	if (level < MAX_DIR_HASH_DEPTH / 2)
		return 2;
	else
		return 4;
}

static unsigned long dir_block_index(unsigned int level,
				int dir_level, unsigned int idx)
{
	unsigned long i;
	unsigned long bidx = 0;

	for (i = 0; i < level; i++)
		bidx += dir_buckets(i, dir_level) * bucket_blocks(i);
	bidx += idx * bucket_blocks(level);
	return bidx;
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

static int f2fs_check_dirent_position(u8 *name, u16 name_len, u32 pgofs,
						u8 dir_level, u32 pino)
{
	f2fs_hash_t namehash = f2fs_dentry_hash(name, name_len);
	unsigned int nbucket, nblock;
	unsigned int bidx, end_block;
	int level;

	level = __get_current_level(dir_level, pgofs);

	nbucket = dir_buckets(level, dir_level);
	nblock = bucket_blocks(level);

	bidx = dir_block_index(level, dir_level,
					le32_to_cpu(namehash) % nbucket);
	end_block = bidx + nblock;

	if (pgofs >= bidx && pgofs < end_block)
		return 0;

	ASSERT_MSG("Wrong position of dirent pino:%u, name:%s, level:%d, "
		"dir_level:%d, pgofs:%u, correct range:[%u, %u]\n",
		pino, name, level, dir_level, pgofs, bidx, end_block - 1);
	return 1;
}

static int __chk_dots_dentries(struct f2fs_sb_info *sbi,
			       struct f2fs_dir_entry *dentry,
			       struct child_info *child,
			       u8 *name, int len,
			       __u8 (*filename)[F2FS_SLOT_LEN],
			       int encrypted)
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

	if (f2fs_check_hash_code(dentry, name, len, encrypted))
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

static int __chk_dentries(struct f2fs_sb_info *sbi, struct child_info *child,
			u8 *bitmap, struct f2fs_dir_entry *dentry,
			__u8 (*filenames)[F2FS_SLOT_LEN],
			int max, int last_blk, int encrypted)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	enum FILE_TYPE ftype;
	int dentries = 0;
	u32 blk_cnt;
	u8 *name;
	unsigned char en[F2FS_NAME_LEN + 1];
	u16 name_len, en_len;
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
		memcpy(name, filenames[i], name_len);
		slots = (name_len + F2FS_SLOT_LEN - 1) / F2FS_SLOT_LEN;

		/* Becareful. 'dentry.file_type' is not imode. */
		if (ftype == F2FS_FT_DIR) {
			if ((name[0] == '.' && name_len == 1) ||
				(name[0] == '.' && name[1] == '.' &&
							name_len == 2)) {
				ret = __chk_dots_dentries(sbi, &dentry[i],
					child, name, name_len, &filenames[i],
					encrypted);
				switch (ret) {
				case 1:
					fixed = 1;
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

		if (f2fs_check_hash_code(dentry + i, name, name_len, encrypted))
			fixed = 1;

		if (max == NR_DENTRY_IN_BLOCK) {
			ret = f2fs_check_dirent_position(name, name_len,
					child->pgofs,
					child->dir_level, child->p_ino);
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

		en_len = convert_encrypted_name(name, name_len, en, encrypted);
		en[en_len] = '\0';
		DBG(1, "[%3u]-[0x%x] name[%s] len[0x%x] ino[0x%x] type[0x%x]\n",
				fsck->dentry_depth, i, en, name_len,
				le32_to_cpu(dentry[i].ino),
				dentry[i].file_type);

		print_dentry(fsck->dentry_depth, name, bitmap,
				dentry, max, i, last_blk, encrypted);

		blk_cnt = 1;
		ret = fsck_chk_node_blk(sbi,
				NULL, le32_to_cpu(dentry[i].ino), name,
				ftype, TYPE_INODE, &blk_cnt, NULL);

		if (ret && c.fix_on) {
			int j;

			for (j = 0; j < slots; j++)
				test_and_clear_bit_le(i + j, bitmap);
			FIX_MSG("Unlink [0x%x] - %s len[0x%x], type[0x%x]",
					le32_to_cpu(dentry[i].ino),
					name, name_len,
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
	struct f2fs_inline_dentry *de_blk;
	int dentries;

	de_blk = inline_data_addr(node_blk);
	ASSERT(de_blk != NULL);

	fsck->dentry_depth++;
	dentries = __chk_dentries(sbi, child,
			de_blk->dentry_bitmap,
			de_blk->dentry, de_blk->filename,
			NR_INLINE_DENTRY, 1,
			file_is_encrypt(&node_blk->i));
	if (dentries < 0) {
		DBG(1, "[%3d] Inline Dentry Block Fixed hash_codes\n\n",
			fsck->dentry_depth);
	} else {
		DBG(1, "[%3d] Inline Dentry Block Done : "
				"dentries:%d in %d slots (len:%d)\n\n",
			fsck->dentry_depth, dentries,
			(int)NR_INLINE_DENTRY, F2FS_NAME_LEN);
	}
	fsck->dentry_depth--;
	return dentries;
}

int fsck_chk_dentry_blk(struct f2fs_sb_info *sbi, u32 blk_addr,
		struct child_info *child, int last_blk, int encrypted)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_dentry_block *de_blk;
	int dentries, ret;

	de_blk = (struct f2fs_dentry_block *)calloc(BLOCK_SZ, 1);
	ASSERT(de_blk != NULL);

	ret = dev_read_block(de_blk, blk_addr);
	ASSERT(ret >= 0);

	fsck->dentry_depth++;
	dentries = __chk_dentries(sbi, child,
			de_blk->dentry_bitmap,
			de_blk->dentry, de_blk->filename,
			NR_DENTRY_IN_BLOCK, last_blk, encrypted);

	if (dentries < 0 && !c.ro) {
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
	fsck->dentry_depth--;
	free(de_blk);
	return 0;
}

int fsck_chk_data_blk(struct f2fs_sb_info *sbi, u32 blk_addr,
		struct child_info *child, int last_blk,
		enum FILE_TYPE ftype, u32 parent_nid, u16 idx_in_node, u8 ver,
		int encrypted)
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
		return fsck_chk_dentry_blk(sbi, blk_addr, child,
						last_blk, encrypted);
	} else {
		f2fs_set_main_bitmap(sbi, blk_addr, CURSEG_WARM_DATA);
	}
	return 0;
}

int fsck_chk_orphan_node(struct f2fs_sb_info *sbi)
{
	u32 blk_cnt = 0;
	block_t start_blk, orphan_blkaddr, i, j;
	struct f2fs_orphan_block *orphan_blk, *new_blk;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	u32 entry_count;

	if (!is_set_ckpt_flags(F2FS_CKPT(sbi), CP_ORPHAN_PRESENT_FLAG))
		return 0;

	start_blk = __start_cp_addr(sbi) + 1 + get_sb(cp_payload);
	orphan_blkaddr = __start_sum_addr(sbi) - 1 - get_sb(cp_payload);

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

			if (c.preen_mode == PREEN_MODE_1 && !c.fix_on) {
				get_node_info(sbi, ino, &ni);
				if (!IS_VALID_NID(sbi, ino) ||
						!IS_VALID_BLK_ADDR(sbi, ni.blk_addr))
					return -EINVAL;

				continue;
			}

			ret = fsck_chk_node_blk(sbi, NULL, ino, NULL,
					F2FS_FT_ORPHAN, TYPE_INODE, &blk_cnt,
					NULL);
			if (!ret)
				new_blk->ino[new_entry_count++] =
							orphan_blk->ino[j];
			else if (ret && c.fix_on)
				FIX_MSG("[0x%x] remove from orphan list", ino);
			else if (ret)
				ASSERT_MSG("[0x%x] wrong orphan inode", ino);
		}
		if (!c.ro && c.fix_on &&
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

int fsck_chk_meta(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct seg_entry *se;
	unsigned int sit_valid_segs = 0, sit_node_blks = 0;
	unsigned int i;

	/* 1. check sit usage with CP: curseg is lost? */
	for (i = 0; i < TOTAL_SEGS(sbi); i++) {
		se = get_seg_entry(sbi, i);
		if (se->valid_blocks != 0)
			sit_valid_segs++;
		else if (IS_CUR_SEGNO(sbi, i, NO_CHECK_TYPE)) {
			/* curseg has not been written back to device */
			MSG(1, "\tInfo: curseg %u is counted in valid segs\n", i);
			sit_valid_segs++;
		}
		if (IS_NODESEG(se->type))
			sit_node_blks += se->valid_blocks;
	}
	if (fsck->chk.sit_free_segs + sit_valid_segs != TOTAL_SEGS(sbi)) {
		ASSERT_MSG("SIT usage does not match: sit_free_segs %u, "
				"sit_valid_segs %u, total_segs %u",
			fsck->chk.sit_free_segs, sit_valid_segs,
			TOTAL_SEGS(sbi));
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

	if (fsck->nat_valid_inode_cnt != le32_to_cpu(cp->valid_inode_count)) {
		ASSERT_MSG("valid inode does not match: nat_valid_inode_cnt %u,"
				" valid_inode_count %u",
				fsck->nat_valid_inode_cnt,
				le32_to_cpu(cp->valid_inode_count));
		return -EINVAL;
	}

	/*check nat entry with sit_area_bitmap*/
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

	return 0;
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
					F2FS_FT_MAX, TYPE_INODE, &ni, NULL))
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
	int i;

	/* update curseg sit entries, since we may change
	 * a segment type in move_curseg_info
	 */
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);
		struct f2fs_sit_block *sit_blk;
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;

		se = get_seg_entry(sbi, curseg->segno);
		sit_blk = get_current_sit_page(sbi, curseg->segno);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, curseg->segno)];
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		rewrite_current_sit_page(sbi, curseg->segno, sit_blk);
		free(sit_blk);
	}
}

static void fix_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	unsigned long long cp_blk_no;
	u32 flags = CP_UMOUNT_FLAG;
	block_t orphan_blks = 0;
	u32 i;
	int ret;
	u_int32_t crc = 0;

	if (is_set_ckpt_flags(cp, CP_ORPHAN_PRESENT_FLAG)) {
		orphan_blks = __start_sum_addr(sbi) - 1;
		flags |= CP_ORPHAN_PRESENT_FLAG;
	}

	set_cp(ckpt_flags, flags);
	set_cp(cp_pack_total_block_count, 8 + orphan_blks + get_sb(cp_payload));

	set_cp(free_segment_count, get_free_segments(sbi));
	set_cp(valid_block_count, fsck->chk.valid_blk_cnt);
	set_cp(valid_node_count, fsck->chk.valid_node_cnt);
	set_cp(valid_inode_count, fsck->chk.valid_inode_cnt);

	crc = f2fs_cal_crc32(F2FS_SUPER_MAGIC, cp, CHECKSUM_OFFSET);
	*((__le32 *)((unsigned char *)cp + CHECKSUM_OFFSET)) = cpu_to_le32(crc);

	cp_blk_no = get_sb(cp_blkaddr);
	if (sbi->cur_cp == 2)
		cp_blk_no += 1 << get_sb(log_blocks_per_seg);

	ret = dev_write_block(cp, cp_blk_no++);
	ASSERT(ret >= 0);

	for (i = 0; i < get_sb(cp_payload); i++) {
		ret = dev_write_block(((unsigned char *)cp) + i * F2FS_BLKSIZE,
								cp_blk_no++);
		ASSERT(ret >= 0);
	}

	cp_blk_no += orphan_blks;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);

		ret = dev_write_block(curseg->sum_blk, cp_blk_no++);
		ASSERT(ret >= 0);
	}

	ret = dev_write_block(cp, cp_blk_no++);
	ASSERT(ret >= 0);
}

int check_curseg_offset(struct f2fs_sb_info *sbi)
{
	int i;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);
		struct seg_entry *se;
		int j, nblocks;

		se = get_seg_entry(sbi, curseg->segno);
		if (f2fs_test_bit(curseg->next_blkoff,
					(const char *)se->cur_valid_map)) {
			ASSERT_MSG("Next block offset is not free, type:%d", i);
			return -EINVAL;
		}
		if (curseg->alloc_type == SSR)
			return 0;

		nblocks = sbi->blocks_per_seg;
		for (j = curseg->next_blkoff + 1; j < nblocks; j++) {
			if (f2fs_test_bit(j, (const char *)se->cur_valid_map)) {
				ASSERT_MSG("LFS must have free section:%d", i);
				return -EINVAL;
			}
		}
	}
	return 0;
}

int check_sit_types(struct f2fs_sb_info *sbi)
{
	unsigned int i;
	int err = 0;

	for (i = 0; i < TOTAL_SEGS(sbi); i++) {
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

int fsck_verify(struct f2fs_sb_info *sbi)
{
	unsigned int i = 0;
	int ret = 0;
	int force = 0;
	u32 nr_unref_nid = 0;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct hard_link_node *node = NULL;

	printf("\n");

	for (i = 0; i < fsck->nr_nat_entries; i++) {
		if (f2fs_test_bit(i, fsck->nat_area_bitmap) != 0) {
			printf("NID[0x%x] is unreachable\n", i);
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

	printf("[FSCK] Unreachable nat entries                       ");
	if (nr_unref_nid == 0x0) {
		printf(" [Ok..] [0x%x]\n", nr_unref_nid);
	} else {
		printf(" [Fail] [0x%x]\n", nr_unref_nid);
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] SIT valid block bitmap checking                ");
	if (memcmp(fsck->sit_area_bitmap, fsck->main_area_bitmap,
					fsck->sit_area_bitmap_sz) == 0x0) {
		printf("[Ok..]\n");
	} else {
		printf("[Fail]\n");
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] Hard link checking for regular file           ");
	if (fsck->hard_link_list_head == NULL) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.multi_hard_link_files);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.multi_hard_link_files);
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] valid_block_count matching with CP            ");
	if (sbi->total_valid_block_count == fsck->chk.valid_blk_cnt) {
		printf(" [Ok..] [0x%x]\n", (u32)fsck->chk.valid_blk_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", (u32)fsck->chk.valid_blk_cnt);
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] valid_node_count matcing with CP (de lookup)  ");
	if (sbi->total_valid_node_count == fsck->chk.valid_node_cnt) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.valid_node_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.valid_node_cnt);
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] valid_node_count matcing with CP (nat lookup) ");
	if (sbi->total_valid_node_count == fsck->chk.valid_nat_entry_cnt) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.valid_nat_entry_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.valid_nat_entry_cnt);
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] valid_inode_count matched with CP             ");
	if (sbi->total_valid_inode_count == fsck->chk.valid_inode_cnt) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.valid_inode_cnt);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.valid_inode_cnt);
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] free segment_count matched with CP            ");
	if (le32_to_cpu(F2FS_CKPT(sbi)->free_segment_count) ==
						fsck->chk.sit_free_segs) {
		printf(" [Ok..] [0x%x]\n", fsck->chk.sit_free_segs);
	} else {
		printf(" [Fail] [0x%x]\n", fsck->chk.sit_free_segs);
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
	}

	printf("[FSCK] next block offset is free                     ");
	if (check_curseg_offset(sbi) == 0) {
		printf(" [Ok..]\n");
	} else {
		printf(" [Fail]\n");
		ret = EXIT_ERR_CODE;
		c.bug_on = 1;
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

#ifndef WITH_ANDROID
	if (nr_unref_nid && !c.ro) {
		char ans[255] = {0};

		printf("\nDo you want to restore lost files into ./lost_found/? [Y/N] ");
		ret = scanf("%s", ans);
		ASSERT(ret >= 0);
		if (!strcasecmp(ans, "y")) {
			for (i = 0; i < fsck->nr_nat_entries; i++) {
				if (f2fs_test_bit(i, fsck->nat_area_bitmap))
					dump_node(sbi, i, 1);
			}
		}
	}
#endif
	/* fix global metadata */
	if (force || (c.fix_on && !c.ro)) {
		struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

		if (force || c.bug_on) {
			fix_hard_links(sbi);
			fix_nat_entries(sbi);
			rewrite_sit_area_bitmap(sbi);
			move_curseg_info(sbi, SM_I(sbi)->main_blkaddr);
			write_curseg_info(sbi);
			flush_curseg_sit_entries(sbi);
			fix_checkpoint(sbi);
		} else if (is_set_ckpt_flags(cp, CP_FSCK_FLAG)) {
			write_checkpoint(sbi);
		}
	}
	return ret;
}

void fsck_free(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
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
}
