/**
 * mount.c
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "fsck.h"
#include <locale.h>

u32 get_free_segments(struct f2fs_sb_info *sbi)
{
	u32 i, free_segs = 0;

	for (i = 0; i < TOTAL_SEGS(sbi); i++) {
		struct seg_entry *se = get_seg_entry(sbi, i);

		if (se->valid_blocks == 0x0 &&
				!IS_CUR_SEGNO(sbi, i, NO_CHECK_TYPE))
			free_segs++;
	}
	return free_segs;
}

void update_free_segments(struct f2fs_sb_info *sbi)
{
	char *progress = "-*|*-";
	static int i = 0;

	MSG(0, "\r [ %c ] Free segments: 0x%x", progress[i % 5], get_free_segments(sbi));
	fflush(stdout);
	i++;
}

void print_inode_info(struct f2fs_inode *inode, int name)
{
	unsigned char en[F2FS_NAME_LEN + 1];
	unsigned int i = 0;
	int namelen = le32_to_cpu(inode->i_namelen);
	int is_encrypt = file_is_encrypt(inode);

	namelen = convert_encrypted_name(inode->i_name, namelen, en, is_encrypt);
	en[namelen] = '\0';
	if (name && namelen) {
		inode->i_name[namelen] = '\0';
		MSG(0, " - File name         : %s%s\n", en,
				is_encrypt ? " <encrypted>" : "");
		setlocale(LC_ALL, "");
		MSG(0, " - File size         : %'llu (bytes)\n",
				le64_to_cpu(inode->i_size));
		return;
	}

	DISP_u32(inode, i_mode);
	DISP_u32(inode, i_advise);
	DISP_u32(inode, i_uid);
	DISP_u32(inode, i_gid);
	DISP_u32(inode, i_links);
	DISP_u64(inode, i_size);
	DISP_u64(inode, i_blocks);

	DISP_u64(inode, i_atime);
	DISP_u32(inode, i_atime_nsec);
	DISP_u64(inode, i_ctime);
	DISP_u32(inode, i_ctime_nsec);
	DISP_u64(inode, i_mtime);
	DISP_u32(inode, i_mtime_nsec);

	DISP_u32(inode, i_generation);
	DISP_u32(inode, i_current_depth);
	DISP_u32(inode, i_xattr_nid);
	DISP_u32(inode, i_flags);
	DISP_u32(inode, i_inline);
	DISP_u32(inode, i_pino);
	DISP_u32(inode, i_dir_level);

	if (namelen) {
		DISP_u32(inode, i_namelen);
		printf("%-30s\t\t[%s]\n", "i_name", en);
	}

	printf("i_ext: fofs:%x blkaddr:%x len:%x\n",
			le32_to_cpu(inode->i_ext.fofs),
			le32_to_cpu(inode->i_ext.blk_addr),
			le32_to_cpu(inode->i_ext.len));

	DISP_u32(inode, i_addr[0]);	/* Pointers to data blocks */
	DISP_u32(inode, i_addr[1]);	/* Pointers to data blocks */
	DISP_u32(inode, i_addr[2]);	/* Pointers to data blocks */
	DISP_u32(inode, i_addr[3]);	/* Pointers to data blocks */

	for (i = 4; i < ADDRS_PER_INODE(inode); i++) {
		if (inode->i_addr[i] != 0x0) {
			printf("i_addr[0x%x] points data block\r\t\t[0x%4x]\n",
					i, le32_to_cpu(inode->i_addr[i]));
			break;
		}
	}

	DISP_u32(inode, i_nid[0]);	/* direct */
	DISP_u32(inode, i_nid[1]);	/* direct */
	DISP_u32(inode, i_nid[2]);	/* indirect */
	DISP_u32(inode, i_nid[3]);	/* indirect */
	DISP_u32(inode, i_nid[4]);	/* double indirect */

	printf("\n");
}

void print_node_info(struct f2fs_node *node_block, int verbose)
{
	nid_t ino = le32_to_cpu(node_block->footer.ino);
	nid_t nid = le32_to_cpu(node_block->footer.nid);
	/* Is this inode? */
	if (ino == nid) {
		DBG(verbose, "Node ID [0x%x:%u] is inode\n", nid, nid);
		print_inode_info(&node_block->i, verbose);
	} else {
		int i;
		u32 *dump_blk = (u32 *)node_block;
		DBG(verbose,
			"Node ID [0x%x:%u] is direct node or indirect node.\n",
								nid, nid);
		for (i = 0; i <= 10; i++)
			MSG(verbose, "[%d]\t\t\t[0x%8x : %d]\n",
						i, dump_blk[i], dump_blk[i]);
	}
}

static void DISP_label(u_int16_t *name)
{
	char buffer[MAX_VOLUME_NAME];

	utf16_to_utf8(buffer, name, MAX_VOLUME_NAME, MAX_VOLUME_NAME);
	printf("%-30s" "\t\t[%s]\n", "volum_name", buffer);
}

void print_raw_sb_info(struct f2fs_super_block *sb)
{
	if (!c.dbg_lv)
		return;

	printf("\n");
	printf("+--------------------------------------------------------+\n");
	printf("| Super block                                            |\n");
	printf("+--------------------------------------------------------+\n");

	DISP_u32(sb, magic);
	DISP_u32(sb, major_ver);

	DISP_label(sb->volume_name);

	DISP_u32(sb, minor_ver);
	DISP_u32(sb, log_sectorsize);
	DISP_u32(sb, log_sectors_per_block);

	DISP_u32(sb, log_blocksize);
	DISP_u32(sb, log_blocks_per_seg);
	DISP_u32(sb, segs_per_sec);
	DISP_u32(sb, secs_per_zone);
	DISP_u32(sb, checksum_offset);
	DISP_u64(sb, block_count);

	DISP_u32(sb, section_count);
	DISP_u32(sb, segment_count);
	DISP_u32(sb, segment_count_ckpt);
	DISP_u32(sb, segment_count_sit);
	DISP_u32(sb, segment_count_nat);

	DISP_u32(sb, segment_count_ssa);
	DISP_u32(sb, segment_count_main);
	DISP_u32(sb, segment0_blkaddr);

	DISP_u32(sb, cp_blkaddr);
	DISP_u32(sb, sit_blkaddr);
	DISP_u32(sb, nat_blkaddr);
	DISP_u32(sb, ssa_blkaddr);
	DISP_u32(sb, main_blkaddr);

	DISP_u32(sb, root_ino);
	DISP_u32(sb, node_ino);
	DISP_u32(sb, meta_ino);
	DISP_u32(sb, cp_payload);
	DISP("%s", sb, version);
	printf("\n");
}

void print_ckpt_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

	if (!c.dbg_lv)
		return;

	printf("\n");
	printf("+--------------------------------------------------------+\n");
	printf("| Checkpoint                                             |\n");
	printf("+--------------------------------------------------------+\n");

	DISP_u64(cp, checkpoint_ver);
	DISP_u64(cp, user_block_count);
	DISP_u64(cp, valid_block_count);
	DISP_u32(cp, rsvd_segment_count);
	DISP_u32(cp, overprov_segment_count);
	DISP_u32(cp, free_segment_count);

	DISP_u32(cp, alloc_type[CURSEG_HOT_NODE]);
	DISP_u32(cp, alloc_type[CURSEG_WARM_NODE]);
	DISP_u32(cp, alloc_type[CURSEG_COLD_NODE]);
	DISP_u32(cp, cur_node_segno[0]);
	DISP_u32(cp, cur_node_segno[1]);
	DISP_u32(cp, cur_node_segno[2]);

	DISP_u32(cp, cur_node_blkoff[0]);
	DISP_u32(cp, cur_node_blkoff[1]);
	DISP_u32(cp, cur_node_blkoff[2]);


	DISP_u32(cp, alloc_type[CURSEG_HOT_DATA]);
	DISP_u32(cp, alloc_type[CURSEG_WARM_DATA]);
	DISP_u32(cp, alloc_type[CURSEG_COLD_DATA]);
	DISP_u32(cp, cur_data_segno[0]);
	DISP_u32(cp, cur_data_segno[1]);
	DISP_u32(cp, cur_data_segno[2]);

	DISP_u32(cp, cur_data_blkoff[0]);
	DISP_u32(cp, cur_data_blkoff[1]);
	DISP_u32(cp, cur_data_blkoff[2]);

	DISP_u32(cp, ckpt_flags);
	DISP_u32(cp, cp_pack_total_block_count);
	DISP_u32(cp, cp_pack_start_sum);
	DISP_u32(cp, valid_node_count);
	DISP_u32(cp, valid_inode_count);
	DISP_u32(cp, next_free_nid);
	DISP_u32(cp, sit_ver_bitmap_bytesize);
	DISP_u32(cp, nat_ver_bitmap_bytesize);
	DISP_u32(cp, checksum_offset);
	DISP_u64(cp, elapsed_time);

	DISP_u32(cp, sit_nat_version_bitmap[0]);
	printf("\n\n");
}

void print_cp_state(u32 flag)
{
	MSG(0, "Info: checkpoint state = %x : ", flag);
	if (flag & CP_FSCK_FLAG)
		MSG(0, "%s", " fsck");
	if (flag & CP_ERROR_FLAG)
		MSG(0, "%s", " error");
	if (flag & CP_COMPACT_SUM_FLAG)
		MSG(0, "%s", " compacted_summary");
	if (flag & CP_ORPHAN_PRESENT_FLAG)
		MSG(0, "%s", " orphan_inodes");
	if (flag & CP_FASTBOOT_FLAG)
		MSG(0, "%s", " fastboot");
	if (flag & CP_UMOUNT_FLAG)
		MSG(0, "%s", " unmount");
	else
		MSG(0, "%s", " sudden-power-off");
	MSG(0, "\n");
}

void print_sb_state(struct f2fs_super_block *sb)
{
	__le32 f = sb->feature;
	int i;

	MSG(0, "Info: superblock features = %x : ", f);
	if (f & cpu_to_le32(F2FS_FEATURE_ENCRYPT)) {
		MSG(0, "%s", " encrypt");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_BLKZONED)) {
		MSG(0, "%s", " zoned block device");
	}
	MSG(0, "\n");
	MSG(0, "Info: superblock encrypt level = %d, salt = ",
					sb->encryption_level);
	for (i = 0; i < 16; i++)
		MSG(0, "%02x", sb->encrypt_pw_salt[i]);
	MSG(0, "\n");
}

static inline int sanity_check_area_boundary(struct f2fs_super_block *sb,
							u64 offset)
{
	u32 segment0_blkaddr = get_sb(segment0_blkaddr);
	u32 cp_blkaddr = get_sb(cp_blkaddr);
	u32 sit_blkaddr = get_sb(sit_blkaddr);
	u32 nat_blkaddr = get_sb(nat_blkaddr);
	u32 ssa_blkaddr = get_sb(ssa_blkaddr);
	u32 main_blkaddr = get_sb(main_blkaddr);
	u32 segment_count_ckpt = get_sb(segment_count_ckpt);
	u32 segment_count_sit = get_sb(segment_count_sit);
	u32 segment_count_nat = get_sb(segment_count_nat);
	u32 segment_count_ssa = get_sb(segment_count_ssa);
	u32 segment_count_main = get_sb(segment_count_main);
	u32 segment_count = get_sb(segment_count);
	u32 log_blocks_per_seg = get_sb(log_blocks_per_seg);
	u64 main_end_blkaddr = main_blkaddr +
				(segment_count_main << log_blocks_per_seg);
	u64 seg_end_blkaddr = segment0_blkaddr +
				(segment_count << log_blocks_per_seg);

	if (segment0_blkaddr != cp_blkaddr) {
		MSG(0, "\tMismatch segment0(%u) cp_blkaddr(%u)\n",
				segment0_blkaddr, cp_blkaddr);
		return -1;
	}

	if (cp_blkaddr + (segment_count_ckpt << log_blocks_per_seg) !=
							sit_blkaddr) {
		MSG(0, "\tWrong CP boundary, start(%u) end(%u) blocks(%u)\n",
			cp_blkaddr, sit_blkaddr,
			segment_count_ckpt << log_blocks_per_seg);
		return -1;
	}

	if (sit_blkaddr + (segment_count_sit << log_blocks_per_seg) !=
							nat_blkaddr) {
		MSG(0, "\tWrong SIT boundary, start(%u) end(%u) blocks(%u)\n",
			sit_blkaddr, nat_blkaddr,
			segment_count_sit << log_blocks_per_seg);
		return -1;
	}

	if (nat_blkaddr + (segment_count_nat << log_blocks_per_seg) !=
							ssa_blkaddr) {
		MSG(0, "\tWrong NAT boundary, start(%u) end(%u) blocks(%u)\n",
			nat_blkaddr, ssa_blkaddr,
			segment_count_nat << log_blocks_per_seg);
		return -1;
	}

	if (ssa_blkaddr + (segment_count_ssa << log_blocks_per_seg) !=
							main_blkaddr) {
		MSG(0, "\tWrong SSA boundary, start(%u) end(%u) blocks(%u)\n",
			ssa_blkaddr, main_blkaddr,
			segment_count_ssa << log_blocks_per_seg);
		return -1;
	}

	if (main_end_blkaddr > seg_end_blkaddr) {
		MSG(0, "\tWrong MAIN_AREA, start(%u) end(%u) block(%u)\n",
			main_blkaddr,
			segment0_blkaddr +
				(segment_count << log_blocks_per_seg),
			segment_count_main << log_blocks_per_seg);
		return -1;
	} else if (main_end_blkaddr < seg_end_blkaddr) {
		int err;

		set_sb(segment_count, (main_end_blkaddr -
				segment0_blkaddr) >> log_blocks_per_seg);

		err = dev_write(sb, offset, sizeof(struct f2fs_super_block));
		MSG(0, "Info: Fix alignment: %s, start(%u) end(%u) block(%u)\n",
			err ? "failed": "done",
			main_blkaddr,
			segment0_blkaddr +
				(segment_count << log_blocks_per_seg),
			segment_count_main << log_blocks_per_seg);
	}
	return 0;
}

int sanity_check_raw_super(struct f2fs_super_block *sb, u64 offset)
{
	unsigned int blocksize;

	if (F2FS_SUPER_MAGIC != get_sb(magic))
		return -1;

	if (F2FS_BLKSIZE != PAGE_CACHE_SIZE)
		return -1;

	blocksize = 1 << get_sb(log_blocksize);
	if (F2FS_BLKSIZE != blocksize)
		return -1;

	/* check log blocks per segment */
	if (get_sb(log_blocks_per_seg) != 9)
		return -1;

	/* Currently, support 512/1024/2048/4096 bytes sector size */
	if (get_sb(log_sectorsize) > F2FS_MAX_LOG_SECTOR_SIZE ||
			get_sb(log_sectorsize) < F2FS_MIN_LOG_SECTOR_SIZE)
		return -1;

	if (get_sb(log_sectors_per_block) + get_sb(log_sectorsize) !=
						F2FS_MAX_LOG_SECTOR_SIZE)
		return -1;

	/* check reserved ino info */
	if (get_sb(node_ino) != 1 || get_sb(meta_ino) != 2 ||
					get_sb(root_ino) != 3)
		return -1;

	/* Check zoned block device feature */
	if (c.devices[0].zoned_model == F2FS_ZONED_HM &&
			!(sb->feature & cpu_to_le32(F2FS_FEATURE_BLKZONED))) {
		MSG(0, "\tMissing zoned block device feature\n");
		return -1;
	}

	if (sanity_check_area_boundary(sb, offset))
		return -1;
	return 0;
}

int validate_super_block(struct f2fs_sb_info *sbi, int block)
{
	u64 offset;

	sbi->raw_super = malloc(sizeof(struct f2fs_super_block));

	if (block == 0)
		offset = F2FS_SUPER_OFFSET;
	else
		offset = F2FS_BLKSIZE + F2FS_SUPER_OFFSET;

	if (dev_read(sbi->raw_super, offset, sizeof(struct f2fs_super_block)))
		return -1;

	if (!sanity_check_raw_super(sbi->raw_super, offset)) {
		/* get kernel version */
		if (c.kd >= 0) {
			dev_read_version(c.version, 0, VERSION_LEN);
			get_kernel_version(c.version);
		} else {
			memset(c.version, 0, VERSION_LEN);
		}

		/* build sb version */
		memcpy(c.sb_version, sbi->raw_super->version, VERSION_LEN);
		get_kernel_version(c.sb_version);
		memcpy(c.init_version, sbi->raw_super->init_version, VERSION_LEN);
		get_kernel_version(c.init_version);

		MSG(0, "Info: MKFS version\n  \"%s\"\n", c.init_version);
		MSG(0, "Info: FSCK version\n  from \"%s\"\n    to \"%s\"\n",
					c.sb_version, c.version);
		if (memcmp(c.sb_version, c.version, VERSION_LEN)) {
			int ret;

			memcpy(sbi->raw_super->version,
						c.version, VERSION_LEN);
			ret = dev_write(sbi->raw_super, offset,
					sizeof(struct f2fs_super_block));
			ASSERT(ret >= 0);

			c.auto_fix = 0;
			c.fix_on = 1;
		}
		print_sb_state(sbi->raw_super);
		return 0;
	}

	free(sbi->raw_super);
	sbi->raw_super = NULL;
	MSG(0, "\tCan't find a valid F2FS superblock at 0x%x\n", block);

	return -EINVAL;
}

int init_sb_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	u64 total_sectors;
	int i;

	sbi->log_sectors_per_block = get_sb(log_sectors_per_block);
	sbi->log_blocksize = get_sb(log_blocksize);
	sbi->blocksize = 1 << sbi->log_blocksize;
	sbi->log_blocks_per_seg = get_sb(log_blocks_per_seg);
	sbi->blocks_per_seg = 1 << sbi->log_blocks_per_seg;
	sbi->segs_per_sec = get_sb(segs_per_sec);
	sbi->secs_per_zone = get_sb(secs_per_zone);
	sbi->total_sections = get_sb(section_count);
	sbi->total_node_count = (get_sb(segment_count_nat) / 2) *
				sbi->blocks_per_seg * NAT_ENTRY_PER_BLOCK;
	sbi->root_ino_num = get_sb(root_ino);
	sbi->node_ino_num = get_sb(node_ino);
	sbi->meta_ino_num = get_sb(meta_ino);
	sbi->cur_victim_sec = NULL_SEGNO;

	for (i = 0; i < MAX_DEVICES; i++) {
		if (!sb->devs[i].path[0])
			break;

		if (i) {
			c.devices[i].path = strdup((char *)sb->devs[i].path);
			if (get_device_info(i))
				ASSERT(0);
		} else {
			ASSERT(!strcmp((char *)sb->devs[i].path,
						(char *)c.devices[i].path));
		}

		c.devices[i].total_segments =
			le32_to_cpu(sb->devs[i].total_segments);
		if (i)
			c.devices[i].start_blkaddr =
				c.devices[i - 1].end_blkaddr + 1;
		c.devices[i].end_blkaddr = c.devices[i].start_blkaddr +
			c.devices[i].total_segments *
			c.blks_per_seg - 1;
		if (i == 0)
			c.devices[i].end_blkaddr += get_sb(segment0_blkaddr);

		c.ndevs = i + 1;
		MSG(0, "Info: Device[%d] : %s blkaddr = %"PRIx64"--%"PRIx64"\n",
				i, c.devices[i].path,
				c.devices[i].start_blkaddr,
				c.devices[i].end_blkaddr);
	}

	total_sectors = get_sb(block_count) << sbi->log_sectors_per_block;
	MSG(0, "Info: total FS sectors = %"PRIu64" (%"PRIu64" MB)\n",
				total_sectors, total_sectors >>
						(20 - get_sb(log_sectorsize)));
	return 0;
}

void *validate_checkpoint(struct f2fs_sb_info *sbi, block_t cp_addr,
				unsigned long long *version)
{
	void *cp_page_1, *cp_page_2;
	struct f2fs_checkpoint *cp;
	unsigned long blk_size = sbi->blocksize;
	unsigned long long cur_version = 0, pre_version = 0;
	unsigned int crc = 0;
	size_t crc_offset;

	/* Read the 1st cp block in this CP pack */
	cp_page_1 = malloc(PAGE_SIZE);
	if (dev_read_block(cp_page_1, cp_addr) < 0)
		goto invalid_cp1;

	cp = (struct f2fs_checkpoint *)cp_page_1;
	crc_offset = get_cp(checksum_offset);
	if (crc_offset >= blk_size)
		goto invalid_cp1;

	crc = le32_to_cpu(*(__le32 *)((unsigned char *)cp + crc_offset));
	if (f2fs_crc_valid(crc, cp, crc_offset))
		goto invalid_cp1;

	pre_version = get_cp(checkpoint_ver);

	/* Read the 2nd cp block in this CP pack */
	cp_page_2 = malloc(PAGE_SIZE);
	cp_addr += get_cp(cp_pack_total_block_count) - 1;

	if (dev_read_block(cp_page_2, cp_addr) < 0)
		goto invalid_cp2;

	cp = (struct f2fs_checkpoint *)cp_page_2;
	crc_offset = get_cp(checksum_offset);
	if (crc_offset >= blk_size)
		goto invalid_cp2;

	crc = le32_to_cpu(*(__le32 *)((unsigned char *)cp + crc_offset));
	if (f2fs_crc_valid(crc, cp, crc_offset))
		goto invalid_cp2;

	cur_version = get_cp(checkpoint_ver);

	if (cur_version == pre_version) {
		*version = cur_version;
		free(cp_page_2);
		return cp_page_1;
	}

invalid_cp2:
	free(cp_page_2);
invalid_cp1:
	free(cp_page_1);
	return NULL;
}

int get_valid_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	void *cp1, *cp2, *cur_page;
	unsigned long blk_size = sbi->blocksize;
	unsigned long long cp1_version = 0, cp2_version = 0, version;
	unsigned long long cp_start_blk_no;
	unsigned int cp_blks = 1 + get_sb(cp_payload);
	int ret;

	sbi->ckpt = malloc(cp_blks * blk_size);
	if (!sbi->ckpt)
		return -ENOMEM;
	/*
	 * Finding out valid cp block involves read both
	 * sets( cp pack1 and cp pack 2)
	 */
	cp_start_blk_no = get_sb(cp_blkaddr);
	cp1 = validate_checkpoint(sbi, cp_start_blk_no, &cp1_version);

	/* The second checkpoint pack should start at the next segment */
	cp_start_blk_no += 1 << get_sb(log_blocks_per_seg);
	cp2 = validate_checkpoint(sbi, cp_start_blk_no, &cp2_version);

	if (cp1 && cp2) {
		if (ver_after(cp2_version, cp1_version)) {
			cur_page = cp2;
			sbi->cur_cp = 2;
			version = cp2_version;
		} else {
			cur_page = cp1;
			sbi->cur_cp = 1;
			version = cp1_version;
		}
	} else if (cp1) {
		cur_page = cp1;
		sbi->cur_cp = 1;
		version = cp1_version;
	} else if (cp2) {
		cur_page = cp2;
		sbi->cur_cp = 2;
		version = cp2_version;
	} else
		goto fail_no_cp;

	MSG(0, "Info: CKPT version = %llx\n", version);

	memcpy(sbi->ckpt, cur_page, blk_size);

	if (cp_blks > 1) {
		unsigned int i;
		unsigned long long cp_blk_no;

		cp_blk_no = get_sb(cp_blkaddr);
		if (cur_page == cp2)
			cp_blk_no += 1 << get_sb(log_blocks_per_seg);

		/* copy sit bitmap */
		for (i = 1; i < cp_blks; i++) {
			unsigned char *ckpt = (unsigned char *)sbi->ckpt;
			ret = dev_read_block(cur_page, cp_blk_no + i);
			ASSERT(ret >= 0);
			memcpy(ckpt + i * blk_size, cur_page, blk_size);
		}
	}
	if (cp1)
		free(cp1);
	if (cp2)
		free(cp2);
	return 0;

fail_no_cp:
	free(sbi->ckpt);
	sbi->ckpt = NULL;
	return -EINVAL;
}

int sanity_check_ckpt(struct f2fs_sb_info *sbi)
{
	unsigned int total, fsmeta;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

	total = get_sb(segment_count);
	fsmeta = get_sb(segment_count_ckpt);
	fsmeta += get_sb(segment_count_sit);
	fsmeta += get_sb(segment_count_nat);
	fsmeta += get_cp(rsvd_segment_count);
	fsmeta += get_sb(segment_count_ssa);

	if (fsmeta >= total)
		return 1;

	return 0;
}

static pgoff_t current_nat_addr(struct f2fs_sb_info *sbi, nid_t start)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	pgoff_t block_off;
	pgoff_t block_addr;
	int seg_off;

	block_off = NAT_BLOCK_OFFSET(start);
	seg_off = block_off >> sbi->log_blocks_per_seg;

	block_addr = (pgoff_t)(nm_i->nat_blkaddr +
			(seg_off << sbi->log_blocks_per_seg << 1) +
			(block_off & ((1 << sbi->log_blocks_per_seg) -1)));

	if (f2fs_test_bit(block_off, nm_i->nat_bitmap))
		block_addr += sbi->blocks_per_seg;

	return block_addr;
}

static int f2fs_init_nid_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	int nid_bitmap_size = (nm_i->max_nid + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_summary_block *sum = curseg->sum_blk;
	struct f2fs_journal *journal = &sum->journal;
	struct f2fs_nat_block nat_block;
	block_t start_blk;
	nid_t nid;
	int i;

	if (!(c.func == SLOAD))
		return 0;

	nm_i->nid_bitmap = (char *)calloc(nid_bitmap_size, 1);
	if (!nm_i->nid_bitmap)
		return -ENOMEM;

	/* arbitrarily set 0 bit */
	f2fs_set_bit(0, nm_i->nid_bitmap);

	memset((void *)&nat_block, 0, sizeof(struct f2fs_nat_block));

	for (nid = 0; nid < nm_i->max_nid; nid++) {
		if (!(nid % NAT_ENTRY_PER_BLOCK)) {
			int ret;

			start_blk = current_nat_addr(sbi, nid);
			ret = dev_read_block((void *)&nat_block, start_blk);
			ASSERT(ret >= 0);
		}

		if (nat_block.entries[nid % NAT_ENTRY_PER_BLOCK].block_addr)
			f2fs_set_bit(nid, nm_i->nid_bitmap);
	}

	for (i = 0; i < nats_in_cursum(journal); i++) {
		block_t addr;

		addr = le32_to_cpu(nat_in_journal(journal, i).block_addr);
		nid = le32_to_cpu(nid_in_journal(journal, i));
		if (addr != NULL_ADDR)
			f2fs_set_bit(nid, nm_i->nid_bitmap);
	}
	return 0;
}

int init_node_manager(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	unsigned char *version_bitmap;
	unsigned int nat_segs, nat_blocks;

	nm_i->nat_blkaddr = get_sb(nat_blkaddr);

	/* segment_count_nat includes pair segment so divide to 2. */
	nat_segs = get_sb(segment_count_nat) >> 1;
	nat_blocks = nat_segs << get_sb(log_blocks_per_seg);
	nm_i->max_nid = NAT_ENTRY_PER_BLOCK * nat_blocks;
	nm_i->fcnt = 0;
	nm_i->nat_cnt = 0;
	nm_i->init_scan_nid = get_cp(next_free_nid);
	nm_i->next_scan_nid = get_cp(next_free_nid);

	nm_i->bitmap_size = __bitmap_size(sbi, NAT_BITMAP);

	nm_i->nat_bitmap = malloc(nm_i->bitmap_size);
	if (!nm_i->nat_bitmap)
		return -ENOMEM;
	version_bitmap = __bitmap_ptr(sbi, NAT_BITMAP);
	if (!version_bitmap)
		return -EFAULT;

	/* copy version bitmap */
	memcpy(nm_i->nat_bitmap, version_bitmap, nm_i->bitmap_size);
	return f2fs_init_nid_bitmap(sbi);
}

int build_node_manager(struct f2fs_sb_info *sbi)
{
	int err;
	sbi->nm_info = malloc(sizeof(struct f2fs_nm_info));
	if (!sbi->nm_info)
		return -ENOMEM;

	err = init_node_manager(sbi);
	if (err)
		return err;

	return 0;
}

int build_sit_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct sit_info *sit_i;
	unsigned int sit_segs, start;
	char *src_bitmap, *dst_bitmap;
	unsigned int bitmap_size;

	sit_i = malloc(sizeof(struct sit_info));
	if (!sit_i)
		return -ENOMEM;

	SM_I(sbi)->sit_info = sit_i;

	sit_i->sentries = calloc(TOTAL_SEGS(sbi) * sizeof(struct seg_entry), 1);
	if (!sit_i->sentries)
		return -ENOMEM;

	for (start = 0; start < TOTAL_SEGS(sbi); start++) {
		sit_i->sentries[start].cur_valid_map
			= calloc(SIT_VBLOCK_MAP_SIZE, 1);
		sit_i->sentries[start].ckpt_valid_map
			= calloc(SIT_VBLOCK_MAP_SIZE, 1);
		if (!sit_i->sentries[start].cur_valid_map
				|| !sit_i->sentries[start].ckpt_valid_map)
			return -ENOMEM;
	}

	sit_segs = get_sb(segment_count_sit) >> 1;
	bitmap_size = __bitmap_size(sbi, SIT_BITMAP);
	src_bitmap = __bitmap_ptr(sbi, SIT_BITMAP);

	dst_bitmap = malloc(bitmap_size);
	memcpy(dst_bitmap, src_bitmap, bitmap_size);

	sit_i->sit_base_addr = get_sb(sit_blkaddr);
	sit_i->sit_blocks = sit_segs << sbi->log_blocks_per_seg;
	sit_i->written_valid_blocks = get_cp(valid_block_count);
	sit_i->sit_bitmap = dst_bitmap;
	sit_i->bitmap_size = bitmap_size;
	sit_i->dirty_sentries = 0;
	sit_i->sents_per_block = SIT_ENTRY_PER_BLOCK;
	sit_i->elapsed_time = get_cp(elapsed_time);
	return 0;
}

void reset_curseg(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct summary_footer *sum_footer;
	struct seg_entry *se;

	sum_footer = &(curseg->sum_blk->footer);
	memset(sum_footer, 0, sizeof(struct summary_footer));
	if (IS_DATASEG(type))
		SET_SUM_TYPE(sum_footer, SUM_TYPE_DATA);
	if (IS_NODESEG(type))
		SET_SUM_TYPE(sum_footer, SUM_TYPE_NODE);
	se = get_seg_entry(sbi, curseg->segno);
	se->type = type;
}

static void read_compacted_summaries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg;
	unsigned int i, j, offset;
	block_t start;
	char *kaddr;
	int ret;

	start = start_sum_block(sbi);

	kaddr = (char *)malloc(PAGE_SIZE);
	ret = dev_read_block(kaddr, start++);
	ASSERT(ret >= 0);

	curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	memcpy(&curseg->sum_blk->journal.n_nats, kaddr, SUM_JOURNAL_SIZE);

	curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	memcpy(&curseg->sum_blk->journal.n_sits, kaddr + SUM_JOURNAL_SIZE,
						SUM_JOURNAL_SIZE);

	offset = 2 * SUM_JOURNAL_SIZE;
	for (i = CURSEG_HOT_DATA; i <= CURSEG_COLD_DATA; i++) {
		unsigned short blk_off;
		struct curseg_info *curseg = CURSEG_I(sbi, i);

		reset_curseg(sbi, i);

		if (curseg->alloc_type == SSR)
			blk_off = sbi->blocks_per_seg;
		else
			blk_off = curseg->next_blkoff;

		for (j = 0; j < blk_off; j++) {
			struct f2fs_summary *s;
			s = (struct f2fs_summary *)(kaddr + offset);
			curseg->sum_blk->entries[j] = *s;
			offset += SUMMARY_SIZE;
			if (offset + SUMMARY_SIZE <=
					PAGE_CACHE_SIZE - SUM_FOOTER_SIZE)
				continue;
			memset(kaddr, 0, PAGE_SIZE);
			ret = dev_read_block(kaddr, start++);
			ASSERT(ret >= 0);
			offset = 0;
		}
	}
	free(kaddr);
}

static void restore_node_summary(struct f2fs_sb_info *sbi,
		unsigned int segno, struct f2fs_summary_block *sum_blk)
{
	struct f2fs_node *node_blk;
	struct f2fs_summary *sum_entry;
	block_t addr;
	unsigned int i;
	int ret;

	node_blk = malloc(F2FS_BLKSIZE);
	ASSERT(node_blk);

	/* scan the node segment */
	addr = START_BLOCK(sbi, segno);
	sum_entry = &sum_blk->entries[0];

	for (i = 0; i < sbi->blocks_per_seg; i++, sum_entry++) {
		ret = dev_read_block(node_blk, addr);
		ASSERT(ret >= 0);
		sum_entry->nid = node_blk->footer.nid;
		addr++;
	}
	free(node_blk);
}

static void read_normal_summaries(struct f2fs_sb_info *sbi, int type)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_summary_block *sum_blk;
	struct curseg_info *curseg;
	unsigned int segno = 0;
	block_t blk_addr = 0;
	int ret;

	if (IS_DATASEG(type)) {
		segno = get_cp(cur_data_segno[type]);
		if (is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
			blk_addr = sum_blk_addr(sbi, NR_CURSEG_TYPE, type);
		else
			blk_addr = sum_blk_addr(sbi, NR_CURSEG_DATA_TYPE, type);
	} else {
		segno = get_cp(cur_node_segno[type - CURSEG_HOT_NODE]);
		if (is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
			blk_addr = sum_blk_addr(sbi, NR_CURSEG_NODE_TYPE,
							type - CURSEG_HOT_NODE);
		else
			blk_addr = GET_SUM_BLKADDR(sbi, segno);
	}

	sum_blk = (struct f2fs_summary_block *)malloc(PAGE_SIZE);
	ret = dev_read_block(sum_blk, blk_addr);
	ASSERT(ret >= 0);

	if (IS_NODESEG(type) && !is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
		restore_node_summary(sbi, segno, sum_blk);

	curseg = CURSEG_I(sbi, type);
	memcpy(curseg->sum_blk, sum_blk, PAGE_CACHE_SIZE);
	reset_curseg(sbi, type);
	free(sum_blk);
}

void update_sum_entry(struct f2fs_sb_info *sbi, block_t blk_addr,
					struct f2fs_summary *sum)
{
	struct f2fs_summary_block *sum_blk;
	u32 segno, offset;
	int type, ret;
	struct seg_entry *se;

	segno = GET_SEGNO(sbi, blk_addr);
	offset = OFFSET_IN_SEG(sbi, blk_addr);

	se = get_seg_entry(sbi, segno);

	sum_blk = get_sum_block(sbi, segno, &type);
	memcpy(&sum_blk->entries[offset], sum, sizeof(*sum));
	sum_blk->footer.entry_type = IS_NODESEG(se->type) ? SUM_TYPE_NODE :
							SUM_TYPE_DATA;

	/* write SSA all the time */
	if (type < SEG_TYPE_MAX) {
		u64 ssa_blk = GET_SUM_BLKADDR(sbi, segno);
		ret = dev_write_block(sum_blk, ssa_blk);
		ASSERT(ret >= 0);
	}

	if (type == SEG_TYPE_NODE || type == SEG_TYPE_DATA ||
					type == SEG_TYPE_MAX)
		free(sum_blk);
}

static void restore_curseg_summaries(struct f2fs_sb_info *sbi)
{
	int type = CURSEG_HOT_DATA;

	if (is_set_ckpt_flags(F2FS_CKPT(sbi), CP_COMPACT_SUM_FLAG)) {
		read_compacted_summaries(sbi);
		type = CURSEG_HOT_NODE;
	}

	for (; type <= CURSEG_COLD_NODE; type++)
		read_normal_summaries(sbi, type);
}

static void build_curseg(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct curseg_info *array;
	unsigned short blk_off;
	unsigned int segno;
	int i;

	array = malloc(sizeof(*array) * NR_CURSEG_TYPE);
	ASSERT(array);

	SM_I(sbi)->curseg_array = array;

	for (i = 0; i < NR_CURSEG_TYPE; i++) {
		array[i].sum_blk = malloc(PAGE_CACHE_SIZE);
		ASSERT(array[i].sum_blk);
		if (i <= CURSEG_COLD_DATA) {
			blk_off = get_cp(cur_data_blkoff[i]);
			segno = get_cp(cur_data_segno[i]);
		}
		if (i > CURSEG_COLD_DATA) {
			blk_off = get_cp(cur_node_blkoff[i - CURSEG_HOT_NODE]);
			segno = get_cp(cur_node_segno[i - CURSEG_HOT_NODE]);
		}
		array[i].segno = segno;
		array[i].zone = GET_ZONENO_FROM_SEGNO(sbi, segno);
		array[i].next_segno = NULL_SEGNO;
		array[i].next_blkoff = blk_off;
		array[i].alloc_type = cp->alloc_type[i];
	}
	restore_curseg_summaries(sbi);
}

static inline void check_seg_range(struct f2fs_sb_info *sbi, unsigned int segno)
{
	unsigned int end_segno = SM_I(sbi)->segment_count - 1;
	ASSERT(segno <= end_segno);
}

struct f2fs_sit_block *get_current_sit_page(struct f2fs_sb_info *sbi,
						unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int offset = SIT_BLOCK_OFFSET(sit_i, segno);
	block_t blk_addr = sit_i->sit_base_addr + offset;
	struct f2fs_sit_block *sit_blk;
	int ret;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	check_seg_range(sbi, segno);

	/* calculate sit block address */
	if (f2fs_test_bit(offset, sit_i->sit_bitmap))
		blk_addr += sit_i->sit_blocks;

	ret = dev_read_block(sit_blk, blk_addr);
	ASSERT(ret >= 0);

	return sit_blk;
}

void rewrite_current_sit_page(struct f2fs_sb_info *sbi,
			unsigned int segno, struct f2fs_sit_block *sit_blk)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int offset = SIT_BLOCK_OFFSET(sit_i, segno);
	block_t blk_addr = sit_i->sit_base_addr + offset;
	int ret;

	/* calculate sit block address */
	if (f2fs_test_bit(offset, sit_i->sit_bitmap))
		blk_addr += sit_i->sit_blocks;

	ret = dev_write_block(sit_blk, blk_addr);
	ASSERT(ret >= 0);
}

void check_block_count(struct f2fs_sb_info *sbi,
		unsigned int segno, struct f2fs_sit_entry *raw_sit)
{
	struct f2fs_sm_info *sm_info = SM_I(sbi);
	unsigned int end_segno = sm_info->segment_count - 1;
	int valid_blocks = 0;
	unsigned int i;

	/* check segment usage */
	if (GET_SIT_VBLOCKS(raw_sit) > sbi->blocks_per_seg)
		ASSERT_MSG("Invalid SIT vblocks: segno=0x%x, %u",
				segno, GET_SIT_VBLOCKS(raw_sit));

	/* check boundary of a given segment number */
	if (segno > end_segno)
		ASSERT_MSG("Invalid SEGNO: 0x%x", segno);

	/* check bitmap with valid block count */
	for (i = 0; i < SIT_VBLOCK_MAP_SIZE; i++)
		valid_blocks += get_bits_in_byte(raw_sit->valid_map[i]);

	if (GET_SIT_VBLOCKS(raw_sit) != valid_blocks)
		ASSERT_MSG("Wrong SIT valid blocks: segno=0x%x, %u vs. %u",
				segno, GET_SIT_VBLOCKS(raw_sit), valid_blocks);

	if (GET_SIT_TYPE(raw_sit) >= NO_CHECK_TYPE)
		ASSERT_MSG("Wrong SIT type: segno=0x%x, %u",
				segno, GET_SIT_TYPE(raw_sit));
}

void seg_info_from_raw_sit(struct seg_entry *se,
		struct f2fs_sit_entry *raw_sit)
{
	se->valid_blocks = GET_SIT_VBLOCKS(raw_sit);
	se->ckpt_valid_blocks = GET_SIT_VBLOCKS(raw_sit);
	memcpy(se->cur_valid_map, raw_sit->valid_map, SIT_VBLOCK_MAP_SIZE);
	memcpy(se->ckpt_valid_map, raw_sit->valid_map, SIT_VBLOCK_MAP_SIZE);
	se->type = GET_SIT_TYPE(raw_sit);
	se->orig_type = GET_SIT_TYPE(raw_sit);
	se->mtime = le64_to_cpu(raw_sit->mtime);
}

struct seg_entry *get_seg_entry(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	return &sit_i->sentries[segno];
}

struct f2fs_summary_block *get_sum_block(struct f2fs_sb_info *sbi,
				unsigned int segno, int *ret_type)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_summary_block *sum_blk;
	struct curseg_info *curseg;
	int type, ret;
	u64 ssa_blk;

	*ret_type= SEG_TYPE_MAX;

	ssa_blk = GET_SUM_BLKADDR(sbi, segno);
	for (type = 0; type < NR_CURSEG_NODE_TYPE; type++) {
		if (segno == get_cp(cur_node_segno[type])) {
			curseg = CURSEG_I(sbi, CURSEG_HOT_NODE + type);
			if (!IS_SUM_NODE_SEG(curseg->sum_blk->footer)) {
				ASSERT_MSG("segno [0x%x] indicates a data "
						"segment, but should be node",
						segno);
				*ret_type = -SEG_TYPE_CUR_NODE;
			} else {
				*ret_type = SEG_TYPE_CUR_NODE;
			}
			return curseg->sum_blk;
		}
	}

	for (type = 0; type < NR_CURSEG_DATA_TYPE; type++) {
		if (segno == get_cp(cur_data_segno[type])) {
			curseg = CURSEG_I(sbi, type);
			if (IS_SUM_NODE_SEG(curseg->sum_blk->footer)) {
				ASSERT_MSG("segno [0x%x] indicates a node "
						"segment, but should be data",
						segno);
				*ret_type = -SEG_TYPE_CUR_DATA;
			} else {
				*ret_type = SEG_TYPE_CUR_DATA;
			}
			return curseg->sum_blk;
		}
	}

	sum_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sum_blk);

	ret = dev_read_block(sum_blk, ssa_blk);
	ASSERT(ret >= 0);

	if (IS_SUM_NODE_SEG(sum_blk->footer))
		*ret_type = SEG_TYPE_NODE;
	else if (IS_SUM_DATA_SEG(sum_blk->footer))
		*ret_type = SEG_TYPE_DATA;

	return sum_blk;
}

int get_sum_entry(struct f2fs_sb_info *sbi, u32 blk_addr,
				struct f2fs_summary *sum_entry)
{
	struct f2fs_summary_block *sum_blk;
	u32 segno, offset;
	int type;

	segno = GET_SEGNO(sbi, blk_addr);
	offset = OFFSET_IN_SEG(sbi, blk_addr);

	sum_blk = get_sum_block(sbi, segno, &type);
	memcpy(sum_entry, &(sum_blk->entries[offset]),
				sizeof(struct f2fs_summary));
	if (type == SEG_TYPE_NODE || type == SEG_TYPE_DATA ||
					type == SEG_TYPE_MAX)
		free(sum_blk);
	return type;
}

static void get_nat_entry(struct f2fs_sb_info *sbi, nid_t nid,
				struct f2fs_nat_entry *raw_nat)
{
	struct f2fs_nat_block *nat_block;
	pgoff_t block_addr;
	int entry_off;
	int ret;

	if (lookup_nat_in_journal(sbi, nid, raw_nat) >= 0)
		return;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	memcpy(raw_nat, &nat_block->entries[entry_off],
					sizeof(struct f2fs_nat_entry));
	free(nat_block);
}

void update_data_blkaddr(struct f2fs_sb_info *sbi, nid_t nid,
				u16 ofs_in_node, block_t newaddr)
{
	struct f2fs_node *node_blk = NULL;
	struct node_info ni;
	block_t oldaddr, startaddr, endaddr;
	int ret;

	node_blk = (struct f2fs_node *)calloc(BLOCK_SZ, 1);
	ASSERT(node_blk);

	get_node_info(sbi, nid, &ni);

	/* read node_block */
	ret = dev_read_block(node_blk, ni.blk_addr);
	ASSERT(ret >= 0);

	/* check its block address */
	if (node_blk->footer.nid == node_blk->footer.ino) {
		oldaddr = le32_to_cpu(node_blk->i.i_addr[ofs_in_node]);
		node_blk->i.i_addr[ofs_in_node] = cpu_to_le32(newaddr);
	} else {
		oldaddr = le32_to_cpu(node_blk->dn.addr[ofs_in_node]);
		node_blk->dn.addr[ofs_in_node] = cpu_to_le32(newaddr);
	}

	ret = dev_write_block(node_blk, ni.blk_addr);
	ASSERT(ret >= 0);

	/* check extent cache entry */
	if (node_blk->footer.nid != node_blk->footer.ino) {
		get_node_info(sbi, le32_to_cpu(node_blk->footer.ino), &ni);

		/* read inode block */
		ret = dev_read_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	}

	startaddr = le32_to_cpu(node_blk->i.i_ext.blk_addr);
	endaddr = startaddr + le32_to_cpu(node_blk->i.i_ext.len);
	if (oldaddr >= startaddr && oldaddr < endaddr) {
		node_blk->i.i_ext.len = 0;

		/* update inode block */
		ret = dev_write_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	}
	free(node_blk);
}

void update_nat_blkaddr(struct f2fs_sb_info *sbi, nid_t ino,
					nid_t nid, block_t newaddr)
{
	struct f2fs_nat_block *nat_block;
	pgoff_t block_addr;
	int entry_off;
	int ret;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	if (ino)
		nat_block->entries[entry_off].ino = cpu_to_le32(ino);
	nat_block->entries[entry_off].block_addr = cpu_to_le32(newaddr);

	ret = dev_write_block(nat_block, block_addr);
	ASSERT(ret >= 0);
	free(nat_block);
}

void get_node_info(struct f2fs_sb_info *sbi, nid_t nid, struct node_info *ni)
{
	struct f2fs_nat_entry raw_nat;
	get_nat_entry(sbi, nid, &raw_nat);
	ni->nid = nid;
	node_info_from_raw_nat(ni, &raw_nat);
}

void build_sit_entries(struct f2fs_sb_info *sbi)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	struct seg_entry *se;
	struct f2fs_sit_entry sit;
	unsigned int i, segno;

	for (segno = 0; segno < TOTAL_SEGS(sbi); segno++) {
		se = &sit_i->sentries[segno];
		struct f2fs_sit_block *sit_blk;

		sit_blk = get_current_sit_page(sbi, segno);
		sit = sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];
		free(sit_blk);

		check_block_count(sbi, segno, &sit);
		seg_info_from_raw_sit(se, &sit);
	}

	for (i = 0; i < sits_in_cursum(journal); i++) {
		segno = le32_to_cpu(segno_in_journal(journal, i));
		se = &sit_i->sentries[segno];
		sit = sit_in_journal(journal, i);

		check_block_count(sbi, segno, &sit);
		seg_info_from_raw_sit(se, &sit);
	}

}

int build_segment_manager(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_sm_info *sm_info;

	sm_info = malloc(sizeof(struct f2fs_sm_info));
	if (!sm_info)
		return -ENOMEM;

	/* init sm info */
	sbi->sm_info = sm_info;
	sm_info->seg0_blkaddr = get_sb(segment0_blkaddr);
	sm_info->main_blkaddr = get_sb(main_blkaddr);
	sm_info->segment_count = get_sb(segment_count);
	sm_info->reserved_segments = get_cp(rsvd_segment_count);
	sm_info->ovp_segments = get_cp(overprov_segment_count);
	sm_info->main_segments = get_sb(segment_count_main);
	sm_info->ssa_blkaddr = get_sb(ssa_blkaddr);

	build_sit_info(sbi);

	build_curseg(sbi);

	build_sit_entries(sbi);

	return 0;
}

void build_sit_area_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_sm_info *sm_i = SM_I(sbi);
	unsigned int segno = 0;
	char *ptr = NULL;
	u32 sum_vblocks = 0;
	u32 free_segs = 0;
	struct seg_entry *se;

	fsck->sit_area_bitmap_sz = sm_i->main_segments * SIT_VBLOCK_MAP_SIZE;
	fsck->sit_area_bitmap = calloc(1, fsck->sit_area_bitmap_sz);
	ASSERT(fsck->sit_area_bitmap);
	ptr = fsck->sit_area_bitmap;

	ASSERT(fsck->sit_area_bitmap_sz == fsck->main_area_bitmap_sz);

	for (segno = 0; segno < TOTAL_SEGS(sbi); segno++) {
		se = get_seg_entry(sbi, segno);

		memcpy(ptr, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		ptr += SIT_VBLOCK_MAP_SIZE;

		if (se->valid_blocks == 0x0) {
			if (le32_to_cpu(sbi->ckpt->cur_node_segno[0]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_data_segno[0]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_node_segno[1]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_data_segno[1]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_node_segno[2]) == segno ||
				le32_to_cpu(sbi->ckpt->cur_data_segno[2]) == segno) {
				continue;
			} else {
				free_segs++;
			}
		} else {
			sum_vblocks += se->valid_blocks;
		}
	}
	fsck->chk.sit_valid_blocks = sum_vblocks;
	fsck->chk.sit_free_segs = free_segs;

	DBG(1, "Blocks [0x%x : %d] Free Segs [0x%x : %d]\n\n",
			sum_vblocks, sum_vblocks,
			free_segs, free_segs);
}

void rewrite_sit_area_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int segno = 0;
	struct f2fs_summary_block *sum = curseg->sum_blk;
	char *ptr = NULL;

	/* remove sit journal */
	sum->journal.n_sits = 0;

	ptr = fsck->main_area_bitmap;

	for (segno = 0; segno < TOTAL_SEGS(sbi); segno++) {
		struct f2fs_sit_block *sit_blk;
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;
		u16 valid_blocks = 0;
		u16 type;
		int i;

		sit_blk = get_current_sit_page(sbi, segno);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];
		memcpy(sit->valid_map, ptr, SIT_VBLOCK_MAP_SIZE);

		/* update valid block count */
		for (i = 0; i < SIT_VBLOCK_MAP_SIZE; i++)
			valid_blocks += get_bits_in_byte(sit->valid_map[i]);

		se = get_seg_entry(sbi, segno);
		memcpy(se->cur_valid_map, ptr, SIT_VBLOCK_MAP_SIZE);
		se->valid_blocks = valid_blocks;
		type = se->type;
		if (type >= NO_CHECK_TYPE) {
			ASSERT_MSG("Invalide type and valid blocks=%x,%x",
					segno, valid_blocks);
			type = 0;
		}
		sit->vblocks = cpu_to_le16((type << SIT_VBLOCKS_SHIFT) |
								valid_blocks);
		rewrite_current_sit_page(sbi, segno, sit_blk);
		free(sit_blk);

		ptr += SIT_VBLOCK_MAP_SIZE;
	}
}

static int flush_sit_journal_entries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int segno;
	int i;

	for (i = 0; i < sits_in_cursum(journal); i++) {
		struct f2fs_sit_block *sit_blk;
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;

		segno = segno_in_journal(journal, i);
		se = get_seg_entry(sbi, segno);

		sit_blk = get_current_sit_page(sbi, segno);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];

		memcpy(sit->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		sit->mtime = cpu_to_le64(se->mtime);

		rewrite_current_sit_page(sbi, segno, sit_blk);
		free(sit_blk);
	}

	journal->n_sits = 0;
	return i;
}

static int flush_nat_journal_entries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	struct f2fs_nat_block *nat_block;
	pgoff_t block_addr;
	int entry_off;
	nid_t nid;
	int ret;
	int i = 0;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);
next:
	if (i >= nats_in_cursum(journal)) {
		free(nat_block);
		journal->n_nats = 0;
		return i;
	}

	nid = le32_to_cpu(nid_in_journal(journal, i));

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	memcpy(&nat_block->entries[entry_off], &nat_in_journal(journal, i),
					sizeof(struct f2fs_nat_entry));

	ret = dev_write_block(nat_block, block_addr);
	ASSERT(ret >= 0);
	i++;
	goto next;
}

void flush_journal_entries(struct f2fs_sb_info *sbi)
{
	int n_nats = flush_nat_journal_entries(sbi);
	int n_sits = flush_sit_journal_entries(sbi);

	if (n_nats || n_sits)
		write_checkpoint(sbi);
}

void flush_sit_entries(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int segno = 0;
	u32 free_segs = 0;

	/* update free segments */
	for (segno = 0; segno < TOTAL_SEGS(sbi); segno++) {
		struct f2fs_sit_block *sit_blk;
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;

		se = get_seg_entry(sbi, segno);

		if (!se->dirty)
			continue;

		sit_blk = get_current_sit_page(sbi, segno);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];
		memcpy(sit->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		rewrite_current_sit_page(sbi, segno, sit_blk);
		free(sit_blk);

		if (se->valid_blocks == 0x0 &&
				!IS_CUR_SEGNO(sbi, segno, NO_CHECK_TYPE))
			free_segs++;
	}

	set_cp(free_segment_count, free_segs);
}

int find_next_free_block(struct f2fs_sb_info *sbi, u64 *to, int left, int type)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct seg_entry *se;
	u32 segno;
	u64 offset;
	int not_enough = 0;
	u64 end_blkaddr = (get_sb(segment_count_main) <<
			get_sb(log_blocks_per_seg)) + get_sb(main_blkaddr);

	if (get_free_segments(sbi) <= SM_I(sbi)->reserved_segments + 1)
		not_enough = 1;

	while (*to >= SM_I(sbi)->main_blkaddr && *to < end_blkaddr) {
		segno = GET_SEGNO(sbi, *to);
		offset = OFFSET_IN_SEG(sbi, *to);

		se = get_seg_entry(sbi, segno);

		if (se->valid_blocks == sbi->blocks_per_seg ||
				IS_CUR_SEGNO(sbi, segno, type)) {
			*to = left ? START_BLOCK(sbi, segno) - 1:
						START_BLOCK(sbi, segno + 1);
			continue;
		}

		if (se->valid_blocks == 0 && not_enough) {
			*to = left ? START_BLOCK(sbi, segno) - 1:
						START_BLOCK(sbi, segno + 1);
			continue;
		}

		if (se->valid_blocks == 0 && !(segno % sbi->segs_per_sec)) {
			struct seg_entry *se2;
			unsigned int i;

			for (i = 1; i < sbi->segs_per_sec; i++) {
				se2 = get_seg_entry(sbi, segno + i);
				if (se2->valid_blocks)
					break;
			}
			if (i == sbi->segs_per_sec)
				return 0;
		}

		if (se->type == type &&
			!f2fs_test_bit(offset, (const char *)se->cur_valid_map))
			return 0;

		*to = left ? *to - 1: *to + 1;
	}
	return -1;
}

void move_curseg_info(struct f2fs_sb_info *sbi, u64 from)
{
	int i, ret;

	/* update summary blocks having nullified journal entries */
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);
		struct f2fs_summary_block buf;
		u32 old_segno;
		u64 ssa_blk, to;

		/* update original SSA too */
		ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
		ret = dev_write_block(curseg->sum_blk, ssa_blk);
		ASSERT(ret >= 0);

		to = from;
		ret = find_next_free_block(sbi, &to, 0, i);
		ASSERT(ret == 0);

		old_segno = curseg->segno;
		curseg->segno = GET_SEGNO(sbi, to);
		curseg->next_blkoff = OFFSET_IN_SEG(sbi, to);
		curseg->alloc_type = SSR;

		/* update new segno */
		ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
		ret = dev_read_block(&buf, ssa_blk);
		ASSERT(ret >= 0);

		memcpy(curseg->sum_blk, &buf, SUM_ENTRIES_SIZE);

		/* update se->types */
		reset_curseg(sbi, i);

		DBG(1, "Move curseg[%d] %x -> %x after %"PRIx64"\n",
				i, old_segno, curseg->segno, from);
	}
}

void zero_journal_entries(struct f2fs_sb_info *sbi)
{
	int i;

	for (i = 0; i < NO_CHECK_TYPE; i++)
		CURSEG_I(sbi, i)->sum_blk->journal.n_nats = 0;
}

void write_curseg_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	int i;

	for (i = 0; i < NO_CHECK_TYPE; i++) {
		cp->alloc_type[i] = CURSEG_I(sbi, i)->alloc_type;
		if (i < CURSEG_HOT_NODE) {
			set_cp(cur_data_segno[i], CURSEG_I(sbi, i)->segno);
			set_cp(cur_data_blkoff[i],
					CURSEG_I(sbi, i)->next_blkoff);
		} else {
			int n = i - CURSEG_HOT_NODE;

			set_cp(cur_node_segno[n], CURSEG_I(sbi, i)->segno);
			set_cp(cur_node_blkoff[n],
					CURSEG_I(sbi, i)->next_blkoff);
		}
	}
}

int lookup_nat_in_journal(struct f2fs_sb_info *sbi, u32 nid,
					struct f2fs_nat_entry *raw_nat)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	int i = 0;

	for (i = 0; i < nats_in_cursum(journal); i++) {
		if (le32_to_cpu(nid_in_journal(journal, i)) == nid) {
			memcpy(raw_nat, &nat_in_journal(journal, i),
						sizeof(struct f2fs_nat_entry));
			DBG(3, "==> Found nid [0x%x] in nat cache\n", nid);
			return i;
		}
	}
	return -1;
}

void nullify_nat_entry(struct f2fs_sb_info *sbi, u32 nid)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	struct f2fs_nat_block *nat_block;
	pgoff_t block_addr;
	int entry_off;
	int ret;
	int i = 0;

	/* check in journal */
	for (i = 0; i < nats_in_cursum(journal); i++) {
		if (le32_to_cpu(nid_in_journal(journal, i)) == nid) {
			memset(&nat_in_journal(journal, i), 0,
					sizeof(struct f2fs_nat_entry));
			FIX_MSG("Remove nid [0x%x] in nat journal\n", nid);
			return;
		}
	}
	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	memset(&nat_block->entries[entry_off], 0,
					sizeof(struct f2fs_nat_entry));

	ret = dev_write_block(nat_block, block_addr);
	ASSERT(ret >= 0);
	free(nat_block);
}

void write_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	block_t orphan_blks = 0;
	unsigned long long cp_blk_no;
	u32 flags = CP_UMOUNT_FLAG;
	int i, ret;
	u_int32_t crc = 0;

	if (is_set_ckpt_flags(cp, CP_ORPHAN_PRESENT_FLAG)) {
		orphan_blks = __start_sum_addr(sbi) - 1;
		flags |= CP_ORPHAN_PRESENT_FLAG;
	}

	set_cp(ckpt_flags, flags);

	set_cp(free_segment_count, get_free_segments(sbi));
	set_cp(valid_block_count, sbi->total_valid_block_count);
	set_cp(cp_pack_total_block_count, 8 + orphan_blks + get_sb(cp_payload));

	crc = f2fs_cal_crc32(F2FS_SUPER_MAGIC, cp, CHECKSUM_OFFSET);
	*((__le32 *)((unsigned char *)cp + CHECKSUM_OFFSET)) = cpu_to_le32(crc);

	cp_blk_no = get_sb(cp_blkaddr);
	if (sbi->cur_cp == 2)
		cp_blk_no += 1 << get_sb(log_blocks_per_seg);

	/* write the first cp */
	ret = dev_write_block(cp, cp_blk_no++);
	ASSERT(ret >= 0);

	/* skip payload */
	cp_blk_no += get_sb(cp_payload);
	/* skip orphan blocks */
	cp_blk_no += orphan_blks;

	/* update summary blocks having nullified journal entries */
	for (i = 0; i < NO_CHECK_TYPE; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);
		u64 ssa_blk;

		ret = dev_write_block(curseg->sum_blk, cp_blk_no++);
		ASSERT(ret >= 0);

		/* update original SSA too */
		ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
		ret = dev_write_block(curseg->sum_blk, ssa_blk);
		ASSERT(ret >= 0);
	}

	/* write the last cp */
	ret = dev_write_block(cp, cp_blk_no++);
	ASSERT(ret >= 0);
}

void build_nat_area_bitmap(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	struct f2fs_fsck *fsck = F2FS_FSCK(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	struct f2fs_nat_block *nat_block;
	struct node_info ni;
	u32 nid, nr_nat_blks;
	pgoff_t block_off;
	pgoff_t block_addr;
	int seg_off;
	int ret;
	unsigned int i;

	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	/* Alloc & build nat entry bitmap */
	nr_nat_blks = (get_sb(segment_count_nat) / 2) <<
					sbi->log_blocks_per_seg;

	fsck->nr_nat_entries = nr_nat_blks * NAT_ENTRY_PER_BLOCK;
	fsck->nat_area_bitmap_sz = (fsck->nr_nat_entries + 7) / 8;
	fsck->nat_area_bitmap = calloc(fsck->nat_area_bitmap_sz, 1);
	ASSERT(fsck->nat_area_bitmap);

	fsck->entries = calloc(sizeof(struct f2fs_nat_entry),
					fsck->nr_nat_entries);
	ASSERT(fsck->entries);

	for (block_off = 0; block_off < nr_nat_blks; block_off++) {

		seg_off = block_off >> sbi->log_blocks_per_seg;
		block_addr = (pgoff_t)(nm_i->nat_blkaddr +
			(seg_off << sbi->log_blocks_per_seg << 1) +
			(block_off & ((1 << sbi->log_blocks_per_seg) - 1)));

		if (f2fs_test_bit(block_off, nm_i->nat_bitmap))
			block_addr += sbi->blocks_per_seg;

		ret = dev_read_block(nat_block, block_addr);
		ASSERT(ret >= 0);

		nid = block_off * NAT_ENTRY_PER_BLOCK;
		for (i = 0; i < NAT_ENTRY_PER_BLOCK; i++) {
			ni.nid = nid + i;

			if ((nid + i) == F2FS_NODE_INO(sbi) ||
					(nid + i) == F2FS_META_INO(sbi)) {
				/* block_addr of node/meta inode should be 0x1 */
				if (le32_to_cpu(nat_block->entries[i].block_addr) != 0x1) {
					FIX_MSG("ino: 0x%x node/meta inode, block_addr= 0x%x -> 0x1",
							nid + i, le32_to_cpu(nat_block->entries[i].block_addr));
					nat_block->entries[i].block_addr = cpu_to_le32(0x1);
					ret = dev_write_block(nat_block, block_addr);
					ASSERT(ret >= 0);
				}
				continue;
			}

			node_info_from_raw_nat(&ni, &nat_block->entries[i]);
			if (ni.blk_addr == 0x0)
				continue;
			if (ni.ino == 0x0) {
				ASSERT_MSG("\tError: ino[0x%8x] or blk_addr[0x%16x]"
					" is invalid\n", ni.ino, ni.blk_addr);
			}
			if (ni.ino == (nid + i)) {
				fsck->nat_valid_inode_cnt++;
				DBG(3, "ino[0x%8x] maybe is inode\n", ni.ino);
			}
			if (nid + i == 0) {
				/*
				 * nat entry [0] must be null.  If
				 * it is corrupted, set its bit in
				 * nat_area_bitmap, fsck_verify will
				 * nullify it
				 */
				ASSERT_MSG("Invalid nat entry[0]: "
					"blk_addr[0x%x]\n", ni.blk_addr);
				c.fix_on = 1;
				fsck->chk.valid_nat_entry_cnt--;
			}

			DBG(3, "nid[0x%8x] addr[0x%16x] ino[0x%8x]\n",
				nid + i, ni.blk_addr, ni.ino);
			f2fs_set_bit(nid + i, fsck->nat_area_bitmap);
			fsck->chk.valid_nat_entry_cnt++;

			fsck->entries[nid + i] = nat_block->entries[i];
		}
	}

	/* Traverse nat journal, update the corresponding entries */
	for (i = 0; i < nats_in_cursum(journal); i++) {
		struct f2fs_nat_entry raw_nat;
		nid = le32_to_cpu(nid_in_journal(journal, i));
		ni.nid = nid;

		DBG(3, "==> Found nid [0x%x] in nat cache, update it\n", nid);

		/* Clear the original bit and count */
		if (fsck->entries[nid].block_addr != 0x0) {
			fsck->chk.valid_nat_entry_cnt--;
			f2fs_clear_bit(nid, fsck->nat_area_bitmap);
			if (fsck->entries[nid].ino == nid)
				fsck->nat_valid_inode_cnt--;
		}

		/* Use nat entries in journal */
		memcpy(&raw_nat, &nat_in_journal(journal, i),
					sizeof(struct f2fs_nat_entry));
		node_info_from_raw_nat(&ni, &raw_nat);
		if (ni.blk_addr != 0x0) {
			if (ni.ino == 0x0)
				ASSERT_MSG("\tError: ino[0x%8x] or blk_addr[0x%16x]"
					" is invalid\n", ni.ino, ni.blk_addr);
			if (ni.ino == nid) {
				fsck->nat_valid_inode_cnt++;
				DBG(3, "ino[0x%8x] maybe is inode\n", ni.ino);
			}
			f2fs_set_bit(nid, fsck->nat_area_bitmap);
			fsck->chk.valid_nat_entry_cnt++;
			DBG(3, "nid[0x%x] in nat cache\n", nid);
		}
		fsck->entries[nid] = raw_nat;
	}
	free(nat_block);

	DBG(1, "valid nat entries (block_addr != 0x0) [0x%8x : %u]\n",
			fsck->chk.valid_nat_entry_cnt,
			fsck->chk.valid_nat_entry_cnt);
}

static int check_sector_size(struct f2fs_super_block *sb)
{
	int index;
	u_int32_t log_sectorsize, log_sectors_per_block;
	u_int8_t *zero_buff;

	log_sectorsize = log_base_2(c.sector_size);
	log_sectors_per_block = log_base_2(c.sectors_per_blk);

	if (log_sectorsize == get_sb(log_sectorsize) &&
			log_sectors_per_block == get_sb(log_sectors_per_block))
		return 0;

	zero_buff = calloc(F2FS_BLKSIZE, 1);
	ASSERT(zero_buff);

	set_sb(log_sectorsize, log_sectorsize);
	set_sb(log_sectors_per_block, log_sectors_per_block);

	memcpy(zero_buff + F2FS_SUPER_OFFSET, sb, sizeof(*sb));
	DBG(1, "\tWriting super block, at offset 0x%08x\n", 0);
	for (index = 0; index < 2; index++) {
		if (dev_write(zero_buff, index * F2FS_BLKSIZE, F2FS_BLKSIZE)) {
			MSG(1, "\tError: Failed while writing supe_blk "
				"on disk!!! index : %d\n", index);
			free(zero_buff);
			return -1;
		}
	}

	free(zero_buff);
	return 0;
}

int f2fs_do_mount(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = NULL;
	int ret;

	sbi->active_logs = NR_CURSEG_TYPE;
	ret = validate_super_block(sbi, 0);
	if (ret) {
		ret = validate_super_block(sbi, 1);
		if (ret)
			return -1;
	}

	ret = check_sector_size(sbi->raw_super);
	if (ret)
		return -1;

	print_raw_sb_info(F2FS_RAW_SUPER(sbi));

	init_sb_info(sbi);

	ret = get_valid_checkpoint(sbi);
	if (ret) {
		ERR_MSG("Can't find valid checkpoint\n");
		return -1;
	}

	if (sanity_check_ckpt(sbi)) {
		ERR_MSG("Checkpoint is polluted\n");
		return -1;
	}
	cp = F2FS_CKPT(sbi);

	print_ckpt_info(sbi);

	if (c.auto_fix || c.preen_mode) {
		u32 flag = get_cp(ckpt_flags);

		if (flag & CP_FSCK_FLAG)
			c.fix_on = 1;
		else if (!c.preen_mode)
			return 1;
	}

	c.bug_on = 0;

	sbi->total_valid_node_count = get_cp(valid_node_count);
	sbi->total_valid_inode_count = get_cp(valid_inode_count);
	sbi->user_block_count = get_cp(user_block_count);
	sbi->total_valid_block_count = get_cp(valid_block_count);
	sbi->last_valid_block_count = sbi->total_valid_block_count;
	sbi->alloc_valid_block_count = 0;

	if (build_segment_manager(sbi)) {
		ERR_MSG("build_segment_manager failed\n");
		return -1;
	}

	if (build_node_manager(sbi)) {
		ERR_MSG("build_node_manager failed\n");
		return -1;
	}

	return 0;
}

void f2fs_do_umount(struct f2fs_sb_info *sbi)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_sm_info *sm_i = SM_I(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	unsigned int i;

	/* free nm_info */
	if (c.func == SLOAD)
		free(nm_i->nid_bitmap);
	free(nm_i->nat_bitmap);
	free(sbi->nm_info);

	/* free sit_info */
	for (i = 0; i < TOTAL_SEGS(sbi); i++) {
		free(sit_i->sentries[i].cur_valid_map);
		free(sit_i->sentries[i].ckpt_valid_map);
	}
	free(sit_i->sit_bitmap);
	free(sm_i->sit_info);

	/* free sm_info */
	for (i = 0; i < NR_CURSEG_TYPE; i++)
		free(sm_i->curseg_array[i].sum_blk);

	free(sm_i->curseg_array);
	free(sbi->sm_info);

	free(sbi->ckpt);
	free(sbi->raw_super);
}
