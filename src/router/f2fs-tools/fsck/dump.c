/**
 * dump.c
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <inttypes.h>

#include "fsck.h"
#include <locale.h>

#define BUF_SZ	80

const char *seg_type_name[SEG_TYPE_MAX + 1] = {
	"SEG_TYPE_DATA",
	"SEG_TYPE_CUR_DATA",
	"SEG_TYPE_NODE",
	"SEG_TYPE_CUR_NODE",
	"SEG_TYPE_NONE",
};

void nat_dump(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	struct f2fs_nat_block *nat_block;
	struct f2fs_node *node_block;
	u32 nr_nat_blks, nid;
	pgoff_t block_off;
	pgoff_t block_addr;
	char buf[BUF_SZ];
	int seg_off;
	int fd, ret, pack;
	unsigned int i;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	node_block = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	nr_nat_blks = get_sb(segment_count_nat) <<
				(sbi->log_blocks_per_seg - 1);

	fd = open("dump_nat", O_CREAT|O_WRONLY|O_TRUNC, 0666);
	ASSERT(fd >= 0);

	for (block_off = 0; block_off < nr_nat_blks; pack = 1, block_off++) {

		seg_off = block_off >> sbi->log_blocks_per_seg;
		block_addr = (pgoff_t)(nm_i->nat_blkaddr +
			(seg_off << sbi->log_blocks_per_seg << 1) +
			(block_off & ((1 << sbi->log_blocks_per_seg) - 1)));

		if (f2fs_test_bit(block_off, nm_i->nat_bitmap)) {
			block_addr += sbi->blocks_per_seg;
			pack = 2;
		}

		ret = dev_read_block(nat_block, block_addr);
		ASSERT(ret >= 0);

		nid = block_off * NAT_ENTRY_PER_BLOCK;
		for (i = 0; i < NAT_ENTRY_PER_BLOCK; i++) {
			struct f2fs_nat_entry raw_nat;
			struct node_info ni;
			ni.nid = nid + i;

			if(nid + i  == 0 || nid + i  == 1 || nid + i == 2 )
				continue;
			if (lookup_nat_in_journal(sbi, nid + i,
							&raw_nat) >= 0) {
				node_info_from_raw_nat(&ni, &raw_nat);
				ret = dev_read_block(node_block, ni.blk_addr);
				ASSERT(ret >= 0);
				if (ni.blk_addr != 0x0) {
					memset(buf, 0, BUF_SZ);
					snprintf(buf, BUF_SZ,
						"nid:%5u\tino:%5u\toffset:%5u"
						"\tblkaddr:%10u\tpack:%d\n",
						ni.nid, ni.ino,
						le32_to_cpu(node_block->footer.flag) >>
							OFFSET_BIT_SHIFT,
						ni.blk_addr, pack);
					ret = write(fd, buf, strlen(buf));
					ASSERT(ret >= 0);
				}
			} else {
				node_info_from_raw_nat(&ni,
						&nat_block->entries[i]);
				if (ni.blk_addr == 0)
					continue;

				ret = dev_read_block(node_block, ni.blk_addr);
				ASSERT(ret >= 0);
				memset(buf, 0, BUF_SZ);
				snprintf(buf, BUF_SZ,
					"nid:%5u\tino:%5u\toffset:%5u"
					"\tblkaddr:%10u\tpack:%d\n",
					ni.nid, ni.ino,
					le32_to_cpu(node_block->footer.flag) >>
						OFFSET_BIT_SHIFT,
					ni.blk_addr, pack);
				ret = write(fd, buf, strlen(buf));
				ASSERT(ret >= 0);
			}
		}
	}

	free(nat_block);
	free(node_block);

	close(fd);
}

void sit_dump(struct f2fs_sb_info *sbi, unsigned int start_sit,
					unsigned int end_sit)
{
	struct seg_entry *se;
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int segno;
	char buf[BUF_SZ];
	u32 free_segs = 0;;
	u64 valid_blocks = 0;
	int ret;
	int fd, i;
	unsigned int offset;

	fd = open("dump_sit", O_CREAT|O_WRONLY|O_TRUNC, 0666);
	ASSERT(fd >= 0);

	snprintf(buf, BUF_SZ, "segment_type(0:HD, 1:WD, 2:CD, "
						"3:HN, 4:WN, 5:CN)\n");
	ret = write(fd, buf, strlen(buf));
	ASSERT(ret >= 0);

	for (segno = start_sit; segno < end_sit; segno++) {
		se = get_seg_entry(sbi, segno);
		offset = SIT_BLOCK_OFFSET(sit_i, segno);
		memset(buf, 0, BUF_SZ);
		snprintf(buf, BUF_SZ,
		"\nsegno:%8u\tvblocks:%3u\tseg_type:%d\tsit_pack:%d\n\n",
			segno, se->valid_blocks, se->type,
			f2fs_test_bit(offset, sit_i->sit_bitmap) ? 2 : 1);

		ret = write(fd, buf, strlen(buf));
		ASSERT(ret >= 0);

		if (se->valid_blocks == 0x0) {
			free_segs++;
			continue;
		}

		ASSERT(se->valid_blocks <= 512);
		valid_blocks += se->valid_blocks;

		for (i = 0; i < 64; i++) {
			memset(buf, 0, BUF_SZ);
			snprintf(buf, BUF_SZ, "  %02x",
					*(se->cur_valid_map + i));
			ret = write(fd, buf, strlen(buf));
			ASSERT(ret >= 0);

			if ((i + 1) % 16 == 0) {
				snprintf(buf, BUF_SZ, "\n");
				ret = write(fd, buf, strlen(buf));
				ASSERT(ret >= 0);
			}
		}
	}

	memset(buf, 0, BUF_SZ);
	snprintf(buf, BUF_SZ,
		"valid_blocks:[0x%" PRIx64 "]\tvalid_segs:%d\t free_segs:%d\n",
			valid_blocks,
			SM_I(sbi)->main_segments - free_segs,
			free_segs);
	ret = write(fd, buf, strlen(buf));
	ASSERT(ret >= 0);

	close(fd);
}

void ssa_dump(struct f2fs_sb_info *sbi, int start_ssa, int end_ssa)
{
	struct f2fs_summary_block *sum_blk;
	char buf[BUF_SZ];
	int segno, i, ret;
	int fd;

	fd = open("dump_ssa", O_CREAT|O_WRONLY|O_TRUNC, 0666);
	ASSERT(fd >= 0);

	snprintf(buf, BUF_SZ, "Note: dump.f2fs -b blkaddr = 0x%x + segno * "
				" 0x200 + offset\n",
				sbi->sm_info->main_blkaddr);
	ret = write(fd, buf, strlen(buf));
	ASSERT(ret >= 0);

	for (segno = start_ssa; segno < end_ssa; segno++) {
		sum_blk = get_sum_block(sbi, segno, &ret);

		memset(buf, 0, BUF_SZ);
		switch (ret) {
		case SEG_TYPE_CUR_NODE:
			snprintf(buf, BUF_SZ, "\n\nsegno: %x, Current Node\n", segno);
			break;
		case SEG_TYPE_CUR_DATA:
			snprintf(buf, BUF_SZ, "\n\nsegno: %x, Current Data\n", segno);
			break;
		case SEG_TYPE_NODE:
			snprintf(buf, BUF_SZ, "\n\nsegno: %x, Node\n", segno);
			break;
		case SEG_TYPE_DATA:
			snprintf(buf, BUF_SZ, "\n\nsegno: %x, Data\n", segno);
			break;
		}
		ret = write(fd, buf, strlen(buf));
		ASSERT(ret >= 0);

		for (i = 0; i < ENTRIES_IN_SUM; i++) {
			memset(buf, 0, BUF_SZ);
			if (i % 10 == 0) {
				buf[0] = '\n';
				ret = write(fd, buf, strlen(buf));
				ASSERT(ret >= 0);
			}
			snprintf(buf, BUF_SZ, "[%3d: %6x]", i,
					le32_to_cpu(sum_blk->entries[i].nid));
			ret = write(fd, buf, strlen(buf));
			ASSERT(ret >= 0);
		}
		if (ret == SEG_TYPE_NODE || ret == SEG_TYPE_DATA ||
					ret == SEG_TYPE_MAX)
			free(sum_blk);
	}
	close(fd);
}

static void dump_data_blk(struct f2fs_sb_info *sbi, __u64 offset, u32 blkaddr)
{
	char buf[F2FS_BLKSIZE];

	if (blkaddr == NULL_ADDR)
		return;

	/* get data */
	if (blkaddr == NEW_ADDR || !IS_VALID_BLK_ADDR(sbi, blkaddr)) {
		memset(buf, 0, F2FS_BLKSIZE);
	} else {
		int ret;
		ret = dev_read_block(buf, blkaddr);
		ASSERT(ret >= 0);
	}

	/* write blkaddr */
	dev_write_dump(buf, offset, F2FS_BLKSIZE);
}

static void dump_node_blk(struct f2fs_sb_info *sbi, int ntype,
						u32 nid, u64 *ofs)
{
	struct node_info ni;
	struct f2fs_node *node_blk;
	u32 skip = 0;
	u32 i, idx;

	switch (ntype) {
	case TYPE_DIRECT_NODE:
		skip = idx = ADDRS_PER_BLOCK;
		break;
	case TYPE_INDIRECT_NODE:
		idx = NIDS_PER_BLOCK;
		skip = idx * ADDRS_PER_BLOCK;
		break;
	case TYPE_DOUBLE_INDIRECT_NODE:
		skip = 0;
		idx = NIDS_PER_BLOCK;
		break;
	}

	if (nid == 0) {
		*ofs += skip;
		return;
	}

	get_node_info(sbi, nid, &ni);

	node_blk = calloc(BLOCK_SZ, 1);
	dev_read_block(node_blk, ni.blk_addr);

	for (i = 0; i < idx; i++, (*ofs)++) {
		switch (ntype) {
		case TYPE_DIRECT_NODE:
			dump_data_blk(sbi, *ofs * F2FS_BLKSIZE,
					le32_to_cpu(node_blk->dn.addr[i]));
			break;
		case TYPE_INDIRECT_NODE:
			dump_node_blk(sbi, TYPE_DIRECT_NODE,
					le32_to_cpu(node_blk->in.nid[i]), ofs);
			break;
		case TYPE_DOUBLE_INDIRECT_NODE:
			dump_node_blk(sbi, TYPE_INDIRECT_NODE,
					le32_to_cpu(node_blk->in.nid[i]), ofs);
			break;
		}
	}
	free(node_blk);
}

static void dump_inode_blk(struct f2fs_sb_info *sbi, u32 nid,
					struct f2fs_node *node_blk)
{
	u32 i = 0;
	u64 ofs = 0;

	/* TODO: need to dump xattr */

	if((node_blk->i.i_inline & F2FS_INLINE_DATA)){
		DBG(3, "ino[0x%x] has inline data!\n", nid);
		/* recover from inline data */
		dev_write_dump(((unsigned char *)node_blk) + INLINE_DATA_OFFSET,
							0, MAX_INLINE_DATA);
		return;
	}

	/* check data blocks in inode */
	for (i = 0; i < ADDRS_PER_INODE(&node_blk->i); i++, ofs++)
		dump_data_blk(sbi, ofs * F2FS_BLKSIZE,
				le32_to_cpu(node_blk->i.i_addr[i]));

	/* check node blocks in inode */
	for (i = 0; i < 5; i++) {
		if (i == 0 || i == 1)
			dump_node_blk(sbi, TYPE_DIRECT_NODE,
					le32_to_cpu(node_blk->i.i_nid[i]), &ofs);
		else if (i == 2 || i == 3)
			dump_node_blk(sbi, TYPE_INDIRECT_NODE,
					le32_to_cpu(node_blk->i.i_nid[i]), &ofs);
		else if (i == 4)
			dump_node_blk(sbi, TYPE_DOUBLE_INDIRECT_NODE,
					le32_to_cpu(node_blk->i.i_nid[i]), &ofs);
		else
			ASSERT(0);
	}
}

static void dump_file(struct f2fs_sb_info *sbi, struct node_info *ni,
				struct f2fs_node *node_blk, int force)
{
	struct f2fs_inode *inode = &node_blk->i;
	u32 imode = le32_to_cpu(inode->i_mode);
	u32 namelen = le32_to_cpu(inode->i_namelen);
	unsigned char name[F2FS_NAME_LEN + 1] = {0};
	char path[1024] = {0};
	char ans[255] = {0};
	int is_encrypt = file_is_encrypt(inode);
	int ret;

	if (!S_ISREG(imode) || namelen == 0 || namelen > F2FS_NAME_LEN) {
		MSG(force, "Not a regular file or wrong name info\n\n");
		return;
	}
	if (force)
		goto dump;

	printf("Do you want to dump this file into ./lost_found/? [Y/N] ");
	ret = scanf("%s", ans);
	ASSERT(ret >= 0);

	if (!strcasecmp(ans, "y")) {
dump:
		ret = system("mkdir -p ./lost_found");
		ASSERT(ret >= 0);

		/* make a file */
		namelen = convert_encrypted_name(inode->i_name, namelen,
							name, is_encrypt);
		name[namelen] = 0;
		sprintf(path, "./lost_found/%s", name);

		c.dump_fd = open(path, O_TRUNC|O_CREAT|O_RDWR, 0666);
		ASSERT(c.dump_fd >= 0);

		/* dump file's data */
		dump_inode_blk(sbi, ni->ino, node_blk);

		/* adjust file size */
		ret = ftruncate(c.dump_fd, le32_to_cpu(inode->i_size));
		ASSERT(ret >= 0);

		close(c.dump_fd);
	}
}

void dump_node(struct f2fs_sb_info *sbi, nid_t nid, int force)
{
	struct node_info ni;
	struct f2fs_node *node_blk;

	get_node_info(sbi, nid, &ni);

	node_blk = calloc(BLOCK_SZ, 1);
	dev_read_block(node_blk, ni.blk_addr);

	DBG(1, "Node ID               [0x%x]\n", nid);
	DBG(1, "nat_entry.block_addr  [0x%x]\n", ni.blk_addr);
	DBG(1, "nat_entry.version     [0x%x]\n", ni.version);
	DBG(1, "nat_entry.ino         [0x%x]\n", ni.ino);

	if (ni.blk_addr == 0x0)
		MSG(force, "Invalid nat entry\n\n");

	DBG(1, "node_blk.footer.ino [0x%x]\n", le32_to_cpu(node_blk->footer.ino));
	DBG(1, "node_blk.footer.nid [0x%x]\n", le32_to_cpu(node_blk->footer.nid));

	if (le32_to_cpu(node_blk->footer.ino) == ni.ino &&
			le32_to_cpu(node_blk->footer.nid) == ni.nid &&
			ni.ino == ni.nid) {
		print_node_info(node_blk, force);
		dump_file(sbi, &ni, node_blk, force);
	} else {
		print_node_info(node_blk, force);
		MSG(force, "Invalid (i)node block\n\n");
	}

	free(node_blk);
}

static void dump_node_from_blkaddr(u32 blk_addr)
{
	struct f2fs_node *node_blk;
	int ret;

	node_blk = calloc(BLOCK_SZ, 1);
	ASSERT(node_blk);

	ret = dev_read_block(node_blk, blk_addr);
	ASSERT(ret >= 0);

	if (c.dbg_lv > 0)
		print_node_info(node_blk, 0);
	else
		print_inode_info(&node_blk->i, 1);

	free(node_blk);
}

static void dump_data_offset(u32 blk_addr, int ofs_in_node)
{
	struct f2fs_node *node_blk;
	unsigned int indirect_blks = 2 * NIDS_PER_BLOCK + 4;
	unsigned int bidx = 0;
	unsigned int node_ofs;
	int ret;

	node_blk = calloc(BLOCK_SZ, 1);
	ASSERT(node_blk);

	ret = dev_read_block(node_blk, blk_addr);
	ASSERT(ret >= 0);

	node_ofs = ofs_of_node(node_blk);

	if (node_ofs == 0)
		goto got_it;

	if (node_ofs > 0 && node_ofs <= 2) {
		bidx = node_ofs - 1;
	} else if (node_ofs <= indirect_blks) {
		int dec = (node_ofs - 4) / (NIDS_PER_BLOCK + 1);
		bidx = node_ofs - 2 - dec;
	} else {
		int dec = (node_ofs - indirect_blks - 3) / (NIDS_PER_BLOCK + 1);
		bidx = node_ofs - 5 - dec;
	}
	bidx = bidx * ADDRS_PER_BLOCK + ADDRS_PER_INODE(&node_blk->i);
got_it:
	bidx +=  ofs_in_node;

	setlocale(LC_ALL, "");
	MSG(0, " - Data offset       : 0x%x (4KB), %'u (bytes)\n",
				bidx, bidx * 4096);
	free(node_blk);
}

static void dump_node_offset(u32 blk_addr)
{
	struct f2fs_node *node_blk;
	int ret;

	node_blk = calloc(BLOCK_SZ, 1);
	ASSERT(node_blk);

	ret = dev_read_block(node_blk, blk_addr);
	ASSERT(ret >= 0);

	MSG(0, " - Node offset       : 0x%x\n", ofs_of_node(node_blk));
	free(node_blk);
}

int dump_info_from_blkaddr(struct f2fs_sb_info *sbi, u32 blk_addr)
{
	nid_t nid;
	int type;
	struct f2fs_summary sum_entry;
	struct node_info ni, ino_ni;
	int ret = 0;

	MSG(0, "\n== Dump data from block address ==\n\n");

	if (blk_addr < SM_I(sbi)->seg0_blkaddr) {
		MSG(0, "\nFS Reserved Area for SEG #0: ");
		ret = -EINVAL;
	} else if (blk_addr < SIT_I(sbi)->sit_base_addr) {
		MSG(0, "\nFS Metadata Area: ");
		ret = -EINVAL;
	} else if (blk_addr < NM_I(sbi)->nat_blkaddr) {
		MSG(0, "\nFS SIT Area: ");
		ret = -EINVAL;
	} else if (blk_addr < SM_I(sbi)->ssa_blkaddr) {
		MSG(0, "\nFS NAT Area: ");
		ret = -EINVAL;
	} else if (blk_addr < SM_I(sbi)->main_blkaddr) {
		MSG(0, "\nFS SSA Area: ");
		ret = -EINVAL;
	} else if (blk_addr > __end_block_addr(sbi)) {
		MSG(0, "\nOut of address space: ");
		ret = -EINVAL;
	}

	if (ret) {
		MSG(0, "User data is from 0x%x to 0x%x\n\n",
			SM_I(sbi)->main_blkaddr,
			__end_block_addr(sbi));
		return ret;
	}

	type = get_sum_entry(sbi, blk_addr, &sum_entry);
	nid = le32_to_cpu(sum_entry.nid);

	get_node_info(sbi, nid, &ni);

	DBG(1, "Note: blkaddr = main_blkaddr + segno * 512 + offset\n");
	DBG(1, "Block_addr            [0x%x]\n", blk_addr);
	DBG(1, " - Segno              [0x%x]\n", GET_SEGNO(sbi, blk_addr));
	DBG(1, " - Offset             [0x%x]\n", OFFSET_IN_SEG(sbi, blk_addr));
	DBG(1, "SUM.nid               [0x%x]\n", nid);
	DBG(1, "SUM.type              [%s]\n", type >= 0 ?
						seg_type_name[type] :
						"Broken");
	DBG(1, "SUM.version           [%d]\n", sum_entry.version);
	DBG(1, "SUM.ofs_in_node       [0x%x]\n", sum_entry.ofs_in_node);
	DBG(1, "NAT.blkaddr           [0x%x]\n", ni.blk_addr);
	DBG(1, "NAT.ino               [0x%x]\n", ni.ino);

	get_node_info(sbi, ni.ino, &ino_ni);

	/* inode block address */
	if (ni.blk_addr == NULL_ADDR || ino_ni.blk_addr == NULL_ADDR) {
		MSG(0, "FS Userdata Area: Obsolete block from 0x%x\n",
			blk_addr);
		return -EINVAL;
	}

	/* print inode */
	if (c.dbg_lv > 0)
		dump_node_from_blkaddr(ino_ni.blk_addr);

	if (type == SEG_TYPE_CUR_DATA || type == SEG_TYPE_DATA) {
		MSG(0, "FS Userdata Area: Data block from 0x%x\n", blk_addr);
		MSG(0, " - Direct node block : id = 0x%x from 0x%x\n",
					nid, ni.blk_addr);
		MSG(0, " - Inode block       : id = 0x%x from 0x%x\n",
					ni.ino, ino_ni.blk_addr);
		dump_node_from_blkaddr(ino_ni.blk_addr);
		dump_data_offset(ni.blk_addr,
			le16_to_cpu(sum_entry.ofs_in_node));
	} else {
		MSG(0, "FS Userdata Area: Node block from 0x%x\n", blk_addr);
		if (ni.ino == ni.nid) {
			MSG(0, " - Inode block       : id = 0x%x from 0x%x\n",
					ni.ino, ino_ni.blk_addr);
			dump_node_from_blkaddr(ino_ni.blk_addr);
		} else {
			MSG(0, " - Node block        : id = 0x%x from 0x%x\n",
					nid, ni.blk_addr);
			MSG(0, " - Inode block       : id = 0x%x from 0x%x\n",
					ni.ino, ino_ni.blk_addr);
			dump_node_from_blkaddr(ino_ni.blk_addr);
			dump_node_offset(ni.blk_addr);
		}
	}

	return 0;
}
