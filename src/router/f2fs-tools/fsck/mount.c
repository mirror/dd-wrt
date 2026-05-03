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
#include "node.h"
#include "xattr.h"
#include <locale.h>
#include <stdbool.h>
#include <time.h>
#ifdef HAVE_LINUX_POSIX_ACL_H
#include <linux/posix_acl.h>
#endif
#ifdef HAVE_SYS_ACL_H
#include <sys/acl.h>
#endif

#ifndef ACL_UNDEFINED_TAG
#define ACL_UNDEFINED_TAG	(0x00)
#define ACL_USER_OBJ		(0x01)
#define ACL_USER		(0x02)
#define ACL_GROUP_OBJ		(0x04)
#define ACL_GROUP		(0x08)
#define ACL_MASK		(0x10)
#define ACL_OTHER		(0x20)
#endif

#ifdef HAVE_LINUX_BLKZONED_H

static int get_device_idx(struct f2fs_sb_info *sbi, uint32_t segno)
{
	block_t seg_start_blkaddr;
	int i;

	seg_start_blkaddr = SM_I(sbi)->main_blkaddr +
				segno * DEFAULT_BLOCKS_PER_SEGMENT;
	for (i = 0; i < c.ndevs; i++)
		if (c.devices[i].start_blkaddr <= seg_start_blkaddr &&
			c.devices[i].end_blkaddr > seg_start_blkaddr)
			return i;
	return 0;
}

static int get_zone_idx_from_dev(struct f2fs_sb_info *sbi,
					uint32_t segno, uint32_t dev_idx)
{
	block_t seg_start_blkaddr = START_BLOCK(sbi, segno);

	return (seg_start_blkaddr - c.devices[dev_idx].start_blkaddr) >>
			log_base_2(sbi->segs_per_sec * sbi->blocks_per_seg);
}

bool is_usable_seg(struct f2fs_sb_info *sbi, unsigned int segno)
{
	unsigned int secno = segno / sbi->segs_per_sec;
	block_t seg_start = START_BLOCK(sbi, segno);
	block_t blocks_per_sec = sbi->blocks_per_seg * sbi->segs_per_sec;
	unsigned int dev_idx = get_device_idx(sbi, segno);
	unsigned int zone_idx = get_zone_idx_from_dev(sbi, segno, dev_idx);
	unsigned int sec_off = SM_I(sbi)->main_blkaddr >>
						log_base_2(blocks_per_sec);

	if (zone_idx < c.devices[dev_idx].nr_rnd_zones)
		return true;

	if (c.devices[dev_idx].zoned_model != F2FS_ZONED_HM)
		return true;

	return seg_start < ((sec_off + secno) * blocks_per_sec) +
				c.devices[dev_idx].zone_cap_blocks[zone_idx];
}

unsigned int get_usable_seg_count(struct f2fs_sb_info *sbi)
{
	unsigned int i, usable_seg_count = 0;

	for (i = 0; i < MAIN_SEGS(sbi); i++)
		if (is_usable_seg(sbi, i))
			usable_seg_count++;

	return usable_seg_count;
}

#else

bool is_usable_seg(struct f2fs_sb_info *UNUSED(sbi), unsigned int UNUSED(segno))
{
	return true;
}

unsigned int get_usable_seg_count(struct f2fs_sb_info *sbi)
{
	return MAIN_SEGS(sbi);
}

#endif

u32 get_free_segments(struct f2fs_sb_info *sbi)
{
	u32 i, free_segs = 0;

	for (i = 0; i < MAIN_SEGS(sbi); i++) {
		struct seg_entry *se = get_seg_entry(sbi, i);

		if (se->valid_blocks == 0x0 && !IS_CUR_SEGNO(sbi, i) &&
							is_usable_seg(sbi, i))
			free_segs++;
	}
	return free_segs;
}

void update_free_segments(struct f2fs_sb_info *sbi)
{
	char *progress = "-*|*-";
	static int i = 0;

	if (c.dbg_lv)
		return;

	MSG(0, "\r [ %c ] Free segments: 0x%x", progress[i % 5], get_free_segments(sbi));
	fflush(stdout);
	i++;
}

#if defined(HAVE_LINUX_POSIX_ACL_H) || defined(HAVE_SYS_ACL_H)
static void print_acl(const u8 *value, int size)
{
	const struct f2fs_acl_header *hdr = (struct f2fs_acl_header *)value;
	const struct f2fs_acl_entry *entry = (struct f2fs_acl_entry *)(hdr + 1);
	const u8 *end = value + size;
	int i, count;

	if (hdr->a_version != cpu_to_le32(F2FS_ACL_VERSION)) {
		MSG(0, "Invalid ACL version [0x%x : 0x%x]\n",
				le32_to_cpu(hdr->a_version), F2FS_ACL_VERSION);
		return;
	}

	count = f2fs_acl_count(size);
	if (count <= 0) {
		MSG(0, "Invalid ACL value size %d\n", size);
		return;
	}

	for (i = 0; i < count; i++) {
		if ((u8 *)entry > end) {
			MSG(0, "Invalid ACL entries count %d\n", count);
			return;
		}

		switch (le16_to_cpu(entry->e_tag)) {
		case ACL_USER_OBJ:
		case ACL_GROUP_OBJ:
		case ACL_MASK:
		case ACL_OTHER:
			MSG(0, "tag:0x%x perm:0x%x\n",
					le16_to_cpu(entry->e_tag),
					le16_to_cpu(entry->e_perm));
			entry = (struct f2fs_acl_entry *)((char *)entry +
					sizeof(struct f2fs_acl_entry_short));
			break;
		case ACL_USER:
			MSG(0, "tag:0x%x perm:0x%x uid:%u\n",
					le16_to_cpu(entry->e_tag),
					le16_to_cpu(entry->e_perm),
					le32_to_cpu(entry->e_id));
			entry = (struct f2fs_acl_entry *)((char *)entry +
					sizeof(struct f2fs_acl_entry));
			break;
		case ACL_GROUP:
			MSG(0, "tag:0x%x perm:0x%x gid:%u\n",
					le16_to_cpu(entry->e_tag),
					le16_to_cpu(entry->e_perm),
					le32_to_cpu(entry->e_id));
			entry = (struct f2fs_acl_entry *)((char *)entry +
					sizeof(struct f2fs_acl_entry));
			break;
		default:
			MSG(0, "Unknown ACL tag 0x%x\n",
					le16_to_cpu(entry->e_tag));
			return;
		}
	}
}
#endif /* HAVE_LINUX_POSIX_ACL_H || HAVE_SYS_ACL_H */

static void print_xattr_entry(const struct f2fs_xattr_entry *ent)
{
	const u8 *value = (const u8 *)&ent->e_name[ent->e_name_len];
	const int size = le16_to_cpu(ent->e_value_size);
	const struct fscrypt_context *ctx;
	int i;

	MSG(0, "\nxattr: e_name_index:%d e_name:", ent->e_name_index);
	for (i = 0; i < ent->e_name_len; i++)
		MSG(0, "%c", ent->e_name[i]);
	MSG(0, " e_name_len:%d e_value_size:%d e_value:\n",
			ent->e_name_len, size);

	switch (ent->e_name_index) {
#if defined(HAVE_LINUX_POSIX_ACL_H) || defined(HAVE_SYS_ACL_H)
	case F2FS_XATTR_INDEX_POSIX_ACL_ACCESS:
	case F2FS_XATTR_INDEX_POSIX_ACL_DEFAULT:
		print_acl(value, size);
		return;
#endif
	case F2FS_XATTR_INDEX_ENCRYPTION:
		ctx = (const struct fscrypt_context *)value;
		if (size != sizeof(*ctx) ||
		    ctx->format != FS_ENCRYPTION_CONTEXT_FORMAT_V1)
			break;
		MSG(0, "format: %d\n", ctx->format);
		MSG(0, "contents_encryption_mode: 0x%x\n", ctx->contents_encryption_mode);
		MSG(0, "filenames_encryption_mode: 0x%x\n", ctx->filenames_encryption_mode);
		MSG(0, "flags: 0x%x\n", ctx->flags);
		MSG(0, "master_key_descriptor: ");
		for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; i++)
			MSG(0, "%02X", ctx->master_key_descriptor[i]);
		MSG(0, "\nnonce: ");
		for (i = 0; i < FS_KEY_DERIVATION_NONCE_SIZE; i++)
			MSG(0, "%02X", ctx->nonce[i]);
		MSG(0, "\n");
		return;
	}
	for (i = 0; i < size; i++)
		MSG(0, "%02X", value[i]);
	MSG(0, "\n");
}

void print_inode_info(struct f2fs_sb_info *sbi,
			struct f2fs_node *node, int name)
{
	struct f2fs_inode *inode = &node->i;
	void *xattr_addr;
	struct f2fs_xattr_entry *ent;
	char en[F2FS_PRINT_NAMELEN];
	unsigned int i = 0;
	u32 namelen = le32_to_cpu(inode->i_namelen);
	int enc_name = file_enc_name(inode);
	int ofs = get_extra_isize(node);

	pretty_print_filename(inode->i_name, namelen, en, enc_name);
	if (name && en[0]) {
		MSG(0, " - File name         : %s%s\n", en,
				enc_name ? " <encrypted>" : "");
		setlocale(LC_ALL, "");
		MSG(0, " - File size         : %'" PRIu64 " (bytes)\n",
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

	if (en[0]) {
		DISP_u32(inode, i_namelen);
		printf("%-30s\t\t[%s]\n", "i_name", en);
	}

	printf("i_ext: fofs:%x blkaddr:%x len:%x\n",
			le32_to_cpu(inode->i_ext.fofs),
			le32_to_cpu(inode->i_ext.blk_addr),
			le32_to_cpu(inode->i_ext.len));

	if (c.feature & cpu_to_le32(F2FS_FEATURE_EXTRA_ATTR)) {
		DISP_u16(inode, i_extra_isize);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_FLEXIBLE_INLINE_XATTR))
			DISP_u16(inode, i_inline_xattr_size);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_PRJQUOTA))
			DISP_u32(inode, i_projid);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM))
			DISP_u32(inode, i_inode_checksum);
		if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CRTIME)) {
			DISP_u64(inode, i_crtime);
			DISP_u32(inode, i_crtime_nsec);
		}
		if (c.feature & cpu_to_le32(F2FS_FEATURE_COMPRESSION)) {
			DISP_u64(inode, i_compr_blocks);
			DISP_u32(inode, i_compress_algrithm);
			DISP_u32(inode, i_log_cluster_size);
			DISP_u32(inode, i_padding);
		}
	}

	for (i = 0; i < ADDRS_PER_INODE(inode); i++) {
		block_t blkaddr;
		char *flag = "";

		if (i + ofs >= DEF_ADDRS_PER_INODE)
			break;

		blkaddr = le32_to_cpu(inode->i_addr[i + ofs]);

		if (blkaddr == 0x0)
			continue;
		if (blkaddr == COMPRESS_ADDR)
			flag = "cluster flag";
		else if (blkaddr == NEW_ADDR)
			flag = "reserved flag";
		printf("i_addr[0x%x] %-16s\t\t[0x%8x : %u]\n", i + ofs, flag,
				blkaddr, blkaddr);
	}

	DISP_u32(inode, i_nid[0]);	/* direct */
	DISP_u32(inode, i_nid[1]);	/* direct */
	DISP_u32(inode, i_nid[2]);	/* indirect */
	DISP_u32(inode, i_nid[3]);	/* indirect */
	DISP_u32(inode, i_nid[4]);	/* double indirect */

	xattr_addr = read_all_xattrs(sbi, node);
	if (xattr_addr) {
		list_for_each_xattr(ent, xattr_addr) {
			print_xattr_entry(ent);
		}
		free(xattr_addr);
	}

	printf("\n");
}

void print_node_info(struct f2fs_sb_info *sbi,
			struct f2fs_node *node_block, int verbose)
{
	nid_t ino = le32_to_cpu(node_block->footer.ino);
	nid_t nid = le32_to_cpu(node_block->footer.nid);
	/* Is this inode? */
	if (ino == nid) {
		DBG(verbose, "Node ID [0x%x:%u] is inode\n", nid, nid);
		print_inode_info(sbi, node_block, verbose);
	} else {
		int i;
		u32 *dump_blk = (u32 *)node_block;
		DBG(verbose,
			"Node ID [0x%x:%u] is direct node or indirect node.\n",
								nid, nid);
		for (i = 0; i < DEF_ADDRS_PER_BLOCK; i++)
			MSG(verbose, "[%d]\t\t\t[0x%8x : %d]\n",
						i, dump_blk[i], dump_blk[i]);
	}
}

static void DISP_label(uint16_t *name)
{
	char buffer[MAX_VOLUME_NAME];

	utf16_to_utf8(buffer, name, MAX_VOLUME_NAME, MAX_VOLUME_NAME);
	if (c.layout)
		printf("%-30s %s\n", "Filesystem volume name:", buffer);
	else
		printf("%-30s" "\t\t[%s]\n", "volum_name", buffer);
}

void print_raw_sb_info(struct f2fs_super_block *sb)
{
	if (c.layout)
		goto printout;
	if (!c.dbg_lv)
		return;

	printf("\n");
	printf("+--------------------------------------------------------+\n");
	printf("| Super block                                            |\n");
	printf("+--------------------------------------------------------+\n");
printout:
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
	DISP_u32(sb, crc);
	DISP("%-.252s", sb, version);
	printf("\n");
}

void print_ckpt_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);

	if (c.layout)
		goto printout;
	if (!c.dbg_lv)
		return;

	printf("\n");
	printf("+--------------------------------------------------------+\n");
	printf("| Checkpoint                                             |\n");
	printf("+--------------------------------------------------------+\n");
printout:
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
	if (c.show_file_map)
		return;

	MSG(0, "Info: checkpoint state = %x : ", flag);
	if (flag & CP_QUOTA_NEED_FSCK_FLAG)
		MSG(0, "%s", " quota_need_fsck");
	if (flag & CP_LARGE_NAT_BITMAP_FLAG)
		MSG(0, "%s", " large_nat_bitmap");
	if (flag & CP_NOCRC_RECOVERY_FLAG)
		MSG(0, "%s", " allow_nocrc");
	if (flag & CP_TRIMMED_FLAG)
		MSG(0, "%s", " trimmed");
	if (flag & CP_NAT_BITS_FLAG)
		MSG(0, "%s", " nat_bits");
	if (flag & CP_CRC_RECOVERY_FLAG)
		MSG(0, "%s", " crc");
	if (flag & CP_FASTBOOT_FLAG)
		MSG(0, "%s", " fastboot");
	if (flag & CP_FSCK_FLAG)
		MSG(0, "%s", " fsck");
	if (flag & CP_ERROR_FLAG)
		MSG(0, "%s", " error");
	if (flag & CP_COMPACT_SUM_FLAG)
		MSG(0, "%s", " compacted_summary");
	if (flag & CP_ORPHAN_PRESENT_FLAG)
		MSG(0, "%s", " orphan_inodes");
	if (flag & CP_DISABLED_FLAG)
		MSG(0, "%s", " disabled");
	if (flag & CP_RESIZEFS_FLAG)
		MSG(0, "%s", " resizefs");
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
	if (f & cpu_to_le32(F2FS_FEATURE_VERITY)) {
		MSG(0, "%s", " verity");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_BLKZONED)) {
		MSG(0, "%s", " blkzoned");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_EXTRA_ATTR)) {
		MSG(0, "%s", " extra_attr");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_PRJQUOTA)) {
		MSG(0, "%s", " project_quota");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM)) {
		MSG(0, "%s", " inode_checksum");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_FLEXIBLE_INLINE_XATTR)) {
		MSG(0, "%s", " flexible_inline_xattr");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_QUOTA_INO)) {
		MSG(0, "%s", " quota_ino");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_INODE_CRTIME)) {
		MSG(0, "%s", " inode_crtime");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_LOST_FOUND)) {
		MSG(0, "%s", " lost_found");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_SB_CHKSUM)) {
		MSG(0, "%s", " sb_checksum");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_CASEFOLD)) {
		MSG(0, "%s", " casefold");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_COMPRESSION)) {
		MSG(0, "%s", " compression");
	}
	if (f & cpu_to_le32(F2FS_FEATURE_RO)) {
		MSG(0, "%s", " ro");
	}
	MSG(0, "\n");
	MSG(0, "Info: superblock encrypt level = %d, salt = ",
					sb->encryption_level);
	for (i = 0; i < 16; i++)
		MSG(0, "%02x", sb->encrypt_pw_salt[i]);
	MSG(0, "\n");
}

static char *stop_reason_str[] = {
	[STOP_CP_REASON_SHUTDOWN]		= "shutdown",
	[STOP_CP_REASON_FAULT_INJECT]		= "fault_inject",
	[STOP_CP_REASON_META_PAGE]		= "meta_page",
	[STOP_CP_REASON_WRITE_FAIL]		= "write_fail",
	[STOP_CP_REASON_CORRUPTED_SUMMARY]	= "corrupted_summary",
	[STOP_CP_REASON_UPDATE_INODE]		= "update_inode",
	[STOP_CP_REASON_FLUSH_FAIL]		= "flush_fail",
};

void print_sb_stop_reason(struct f2fs_super_block *sb)
{
	u8 *reason = sb->s_stop_reason;
	int i;

	if (!c.force_stop)
		return;

	MSG(0, "Info: checkpoint stop reason: ");

	for (i = 0; i < STOP_CP_REASON_MAX; i++) {
		if (reason[i])
			MSG(0, "%s(%d) ", stop_reason_str[i], reason[i]);
	}

	MSG(0, "\n");
}

static char *errors_str[] = {
	[ERROR_CORRUPTED_CLUSTER]		= "corrupted_cluster",
	[ERROR_FAIL_DECOMPRESSION]		= "fail_decompression",
	[ERROR_INVALID_BLKADDR]			= "invalid_blkaddr",
	[ERROR_CORRUPTED_DIRENT]		= "corrupted_dirent",
	[ERROR_CORRUPTED_INODE]			= "corrupted_inode",
	[ERROR_INCONSISTENT_SUMMARY]		= "inconsistent_summary",
	[ERROR_INCONSISTENT_FOOTER]		= "inconsistent_footer",
	[ERROR_INCONSISTENT_SUM_TYPE]		= "inconsistent_sum_type",
	[ERROR_CORRUPTED_JOURNAL]		= "corrupted_journal",
	[ERROR_INCONSISTENT_NODE_COUNT]		= "inconsistent_node_count",
	[ERROR_INCONSISTENT_BLOCK_COUNT]	= "inconsistent_block_count",
	[ERROR_INVALID_CURSEG]			= "invalid_curseg",
	[ERROR_INCONSISTENT_SIT]		= "inconsistent_sit",
	[ERROR_CORRUPTED_VERITY_XATTR]		= "corrupted_verity_xattr",
	[ERROR_CORRUPTED_XATTR]			= "corrupted_xattr",
};

void print_sb_errors(struct f2fs_super_block *sb)
{
	u8 *errors = sb->s_errors;
	int i;

	if (!c.fs_errors)
		return;

	MSG(0, "Info: fs errors: ");

	for (i = 0; i < ERROR_MAX; i++) {
		if (test_bit_le(i, errors))
			MSG(0, "%s ",  errors_str[i]);
	}

	MSG(0, "\n");
}

bool f2fs_is_valid_blkaddr(struct f2fs_sb_info *sbi,
					block_t blkaddr, int type)
{
	switch (type) {
	case META_NAT:
		break;
	case META_SIT:
		if (blkaddr >= SIT_BLK_CNT(sbi))
			return 0;
		break;
	case META_SSA:
		if (blkaddr >= MAIN_BLKADDR(sbi) ||
			blkaddr < SM_I(sbi)->ssa_blkaddr)
			return 0;
		break;
	case META_CP:
		if (blkaddr >= SIT_I(sbi)->sit_base_addr ||
			blkaddr < __start_cp_addr(sbi))
			return 0;
		break;
	case META_POR:
		if (blkaddr >= MAX_BLKADDR(sbi) ||
			blkaddr < MAIN_BLKADDR(sbi))
			return 0;
		break;
	default:
		ASSERT(0);
	}

	return 1;
}

static inline block_t current_sit_addr(struct f2fs_sb_info *sbi,
						unsigned int start);

/*
 * Readahead CP/NAT/SIT/SSA pages
 */
int f2fs_ra_meta_pages(struct f2fs_sb_info *sbi, block_t start, int nrpages,
							int type)
{
	block_t blkno = start;
	block_t blkaddr, start_blk = 0, len = 0;

	for (; nrpages-- > 0; blkno++) {

		if (!f2fs_is_valid_blkaddr(sbi, blkno, type))
			goto out;

		switch (type) {
		case META_NAT:
			if (blkno >= NAT_BLOCK_OFFSET(NM_I(sbi)->max_nid))
				blkno = 0;
			/* get nat block addr */
			blkaddr = current_nat_addr(sbi,
					blkno * NAT_ENTRY_PER_BLOCK, NULL);
			break;
		case META_SIT:
			/* get sit block addr */
			blkaddr = current_sit_addr(sbi,
					blkno * SIT_ENTRY_PER_BLOCK);
			break;
		case META_SSA:
		case META_CP:
		case META_POR:
			blkaddr = blkno;
			break;
		default:
			ASSERT(0);
		}

		if (!len) {
			start_blk = blkaddr;
			len = 1;
		} else if (start_blk + len == blkaddr) {
			len++;
		} else {
			dev_readahead(start_blk << F2FS_BLKSIZE_BITS,
						len << F2FS_BLKSIZE_BITS);
		}
	}
out:
	if (len)
		dev_readahead(start_blk << F2FS_BLKSIZE_BITS,
					len << F2FS_BLKSIZE_BITS);
	return blkno - start;
}

void update_superblock(struct f2fs_super_block *sb, int sb_mask)
{
	int addr, ret;
	uint8_t *buf;
	u32 old_crc, new_crc;

	buf = calloc(BLOCK_SZ, 1);
	ASSERT(buf);

	if (get_sb(feature) & F2FS_FEATURE_SB_CHKSUM) {
		old_crc = get_sb(crc);
		new_crc = f2fs_cal_crc32(F2FS_SUPER_MAGIC, sb,
						SB_CHKSUM_OFFSET);
		set_sb(crc, new_crc);
		MSG(1, "Info: SB CRC is updated (0x%x -> 0x%x)\n",
							old_crc, new_crc);
	}

	memcpy(buf + F2FS_SUPER_OFFSET, sb, sizeof(*sb));
	for (addr = SB0_ADDR; addr < SB_MAX_ADDR; addr++) {
		if (SB_MASK(addr) & sb_mask) {
			ret = dev_write_block(buf, addr);
			ASSERT(ret >= 0);
		}
	}

	free(buf);
	DBG(0, "Info: Done to update superblock\n");
}

static inline int sanity_check_area_boundary(struct f2fs_super_block *sb,
							enum SB_ADDR sb_addr)
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
		set_sb(segment_count, (main_end_blkaddr -
				segment0_blkaddr) >> log_blocks_per_seg);

		update_superblock(sb, SB_MASK(sb_addr));
		MSG(0, "Info: Fix alignment: start(%u) end(%u) block(%u)\n",
			main_blkaddr,
			segment0_blkaddr +
				(segment_count << log_blocks_per_seg),
			segment_count_main << log_blocks_per_seg);
	}
	return 0;
}

static int verify_sb_chksum(struct f2fs_super_block *sb)
{
	if (SB_CHKSUM_OFFSET != get_sb(checksum_offset)) {
		MSG(0, "\tInvalid SB CRC offset: %u\n",
					get_sb(checksum_offset));
		return -1;
	}
	if (f2fs_crc_valid(get_sb(crc), sb,
			get_sb(checksum_offset))) {
		MSG(0, "\tInvalid SB CRC: 0x%x\n", get_sb(crc));
		return -1;
	}
	return 0;
}

int sanity_check_raw_super(struct f2fs_super_block *sb, enum SB_ADDR sb_addr)
{
	unsigned int blocksize;
	unsigned int segment_count, segs_per_sec, secs_per_zone, segs_per_zone;
	unsigned int total_sections, blocks_per_seg;

	if (F2FS_SUPER_MAGIC != get_sb(magic)) {
		MSG(0, "Magic Mismatch, valid(0x%x) - read(0x%x)\n",
			F2FS_SUPER_MAGIC, get_sb(magic));
		return -1;
	}

	if ((get_sb(feature) & F2FS_FEATURE_SB_CHKSUM) &&
					verify_sb_chksum(sb))
		return -1;

	blocksize = 1 << get_sb(log_blocksize);
	if (F2FS_BLKSIZE != blocksize) {
		MSG(0, "Invalid blocksize (%u), supports only 4KB\n",
			blocksize);
		return -1;
	}

	/* check log blocks per segment */
	if (get_sb(log_blocks_per_seg) != 9) {
		MSG(0, "Invalid log blocks per segment (%u)\n",
			get_sb(log_blocks_per_seg));
		return -1;
	}

	/* Currently, support 512/1024/2048/4096 bytes sector size */
	if (get_sb(log_sectorsize) > F2FS_MAX_LOG_SECTOR_SIZE ||
			get_sb(log_sectorsize) < F2FS_MIN_LOG_SECTOR_SIZE) {
		MSG(0, "Invalid log sectorsize (%u)\n", get_sb(log_sectorsize));
		return -1;
	}

	if (get_sb(log_sectors_per_block) + get_sb(log_sectorsize) !=
						F2FS_MAX_LOG_SECTOR_SIZE) {
		MSG(0, "Invalid log sectors per block(%u) log sectorsize(%u)\n",
			get_sb(log_sectors_per_block),
			get_sb(log_sectorsize));
		return -1;
	}

	segment_count = get_sb(segment_count);
	segs_per_sec = get_sb(segs_per_sec);
	secs_per_zone = get_sb(secs_per_zone);
	total_sections = get_sb(section_count);
	segs_per_zone = segs_per_sec * secs_per_zone;

	/* blocks_per_seg should be 512, given the above check */
	blocks_per_seg = 1 << get_sb(log_blocks_per_seg);

	if (segment_count > F2FS_MAX_SEGMENT ||
			segment_count < F2FS_MIN_SEGMENTS) {
		MSG(0, "\tInvalid segment count (%u)\n", segment_count);
		return -1;
	}

	if (!(get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO)) &&
			(total_sections > segment_count ||
			total_sections < F2FS_MIN_SEGMENTS ||
			segs_per_sec > segment_count || !segs_per_sec)) {
		MSG(0, "\tInvalid segment/section count (%u, %u x %u)\n",
			segment_count, total_sections, segs_per_sec);
		return 1;
	}

	if ((segment_count / segs_per_sec) < total_sections) {
		MSG(0, "Small segment_count (%u < %u * %u)\n",
			segment_count, segs_per_sec, total_sections);
		return 1;
	}

	if (segment_count > (get_sb(block_count) >> 9)) {
		MSG(0, "Wrong segment_count / block_count (%u > %llu)\n",
			segment_count, get_sb(block_count));
		return 1;
	}

	if (sb->devs[0].path[0]) {
		unsigned int dev_segs = le32_to_cpu(sb->devs[0].total_segments);
		int i = 1;

		while (i < MAX_DEVICES && sb->devs[i].path[0]) {
			dev_segs += le32_to_cpu(sb->devs[i].total_segments);
			i++;
		}
		if (segment_count != dev_segs / segs_per_zone * segs_per_zone) {
			MSG(0, "Segment count (%u) mismatch with total segments from devices (%u)",
				segment_count, dev_segs);
			return 1;
		}
	}

	if (secs_per_zone > total_sections || !secs_per_zone) {
		MSG(0, "Wrong secs_per_zone / total_sections (%u, %u)\n",
			secs_per_zone, total_sections);
		return 1;
	}
	if (get_sb(extension_count) > F2FS_MAX_EXTENSION ||
			sb->hot_ext_count > F2FS_MAX_EXTENSION ||
			get_sb(extension_count) +
			sb->hot_ext_count > F2FS_MAX_EXTENSION) {
		MSG(0, "Corrupted extension count (%u + %u > %u)\n",
			get_sb(extension_count),
			sb->hot_ext_count,
			F2FS_MAX_EXTENSION);
		return 1;
	}

	if (get_sb(cp_payload) > (blocks_per_seg - F2FS_CP_PACKS)) {
		MSG(0, "Insane cp_payload (%u > %u)\n",
			get_sb(cp_payload), blocks_per_seg - F2FS_CP_PACKS);
		return 1;
	}

	/* check reserved ino info */
	if (get_sb(node_ino) != 1 || get_sb(meta_ino) != 2 ||
						get_sb(root_ino) != 3) {
		MSG(0, "Invalid Fs Meta Ino: node(%u) meta(%u) root(%u)\n",
			get_sb(node_ino), get_sb(meta_ino), get_sb(root_ino));
		return -1;
	}

	/* Check zoned block device feature */
	if (c.devices[0].zoned_model != F2FS_ZONED_NONE &&
			!(sb->feature & cpu_to_le32(F2FS_FEATURE_BLKZONED))) {
		MSG(0, "\tMissing zoned block device feature\n");
		return -1;
	}

	if (sanity_check_area_boundary(sb, sb_addr))
		return -1;
	return 0;
}

#define CHECK_PERIOD (3600 * 24 * 30)	// one month by default

int validate_super_block(struct f2fs_sb_info *sbi, enum SB_ADDR sb_addr)
{
	char buf[F2FS_BLKSIZE];

	sbi->raw_super = malloc(sizeof(struct f2fs_super_block));
	if (!sbi->raw_super)
		return -ENOMEM;

	if (dev_read_block(buf, sb_addr))
		return -1;

	memcpy(sbi->raw_super, buf + F2FS_SUPER_OFFSET,
					sizeof(struct f2fs_super_block));

	if (!sanity_check_raw_super(sbi->raw_super, sb_addr)) {
		/* get kernel version */
		if (c.kd >= 0) {
			dev_read_version(c.version, 0, VERSION_NAME_LEN);
			get_kernel_version(c.version);
		} else {
			get_kernel_uname_version(c.version);
		}

		/* build sb version */
		memcpy(c.sb_version, sbi->raw_super->version, VERSION_NAME_LEN);
		get_kernel_version(c.sb_version);
		memcpy(c.init_version, sbi->raw_super->init_version,
				VERSION_NAME_LEN);
		get_kernel_version(c.init_version);

		c.force_stop = is_checkpoint_stop(sbi->raw_super, false);
		c.abnormal_stop = is_checkpoint_stop(sbi->raw_super, true);
		c.fs_errors = is_inconsistent_error(sbi->raw_super);

		MSG(0, "Info: MKFS version\n  \"%s\"\n", c.init_version);
		MSG(0, "Info: FSCK version\n  from \"%s\"\n    to \"%s\"\n",
					c.sb_version, c.version);
		print_sb_state(sbi->raw_super);
		print_sb_stop_reason(sbi->raw_super);
		print_sb_errors(sbi->raw_super);
		return 0;
	}

	free(sbi->raw_super);
	sbi->raw_super = NULL;
	MSG(0, "\tCan't find a valid F2FS superblock at 0x%x\n", sb_addr);

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

		if (c.zoned_model == F2FS_ZONED_NONE) {
			if (c.devices[i].zoned_model == F2FS_ZONED_HM)
				c.zoned_model = F2FS_ZONED_HM;
			else if (c.devices[i].zoned_model == F2FS_ZONED_HA &&
					c.zoned_model != F2FS_ZONED_HM)
				c.zoned_model = F2FS_ZONED_HA;
		}

		c.ndevs = i + 1;
		MSG(0, "Info: Device[%d] : %s blkaddr = %"PRIx64"--%"PRIx64"\n",
				i, c.devices[i].path,
				c.devices[i].start_blkaddr,
				c.devices[i].end_blkaddr);
	}

	total_sectors = get_sb(block_count) << sbi->log_sectors_per_block;
	MSG(0, "Info: Segments per section = %d\n", sbi->segs_per_sec);
	MSG(0, "Info: Sections per zone = %d\n", sbi->secs_per_zone);
	MSG(0, "Info: total FS sectors = %"PRIu64" (%"PRIu64" MB)\n",
				total_sectors, total_sectors >>
						(20 - get_sb(log_sectorsize)));
	return 0;
}

static int verify_checksum_chksum(struct f2fs_checkpoint *cp)
{
	unsigned int chksum_offset = get_cp(checksum_offset);
	unsigned int crc, cal_crc;

	if (chksum_offset < CP_MIN_CHKSUM_OFFSET ||
			chksum_offset > CP_CHKSUM_OFFSET) {
		MSG(0, "\tInvalid CP CRC offset: %u\n", chksum_offset);
		return -1;
	}

	crc = le32_to_cpu(*(__le32 *)((unsigned char *)cp + chksum_offset));
	cal_crc = f2fs_checkpoint_chksum(cp);
	if (cal_crc != crc) {
		MSG(0, "\tInvalid CP CRC: offset:%u, crc:0x%x, calc:0x%x\n",
			chksum_offset, crc, cal_crc);
		return -1;
	}
	return 0;
}

static void *get_checkpoint_version(block_t cp_addr)
{
	void *cp_page;

	cp_page = malloc(F2FS_BLKSIZE);
	ASSERT(cp_page);

	if (dev_read_block(cp_page, cp_addr) < 0)
		ASSERT(0);

	if (verify_checksum_chksum((struct f2fs_checkpoint *)cp_page))
		goto out;
	return cp_page;
out:
	free(cp_page);
	return NULL;
}

void *validate_checkpoint(struct f2fs_sb_info *sbi, block_t cp_addr,
				unsigned long long *version)
{
	void *cp_page_1, *cp_page_2;
	struct f2fs_checkpoint *cp;
	unsigned long long cur_version = 0, pre_version = 0;

	/* Read the 1st cp block in this CP pack */
	cp_page_1 = get_checkpoint_version(cp_addr);
	if (!cp_page_1)
		return NULL;

	cp = (struct f2fs_checkpoint *)cp_page_1;
	if (get_cp(cp_pack_total_block_count) > sbi->blocks_per_seg)
		goto invalid_cp1;

	pre_version = get_cp(checkpoint_ver);

	/* Read the 2nd cp block in this CP pack */
	cp_addr += get_cp(cp_pack_total_block_count) - 1;
	cp_page_2 = get_checkpoint_version(cp_addr);
	if (!cp_page_2)
		goto invalid_cp1;

	cp = (struct f2fs_checkpoint *)cp_page_2;
	cur_version = get_cp(checkpoint_ver);

	if (cur_version == pre_version) {
		*version = cur_version;
		free(cp_page_2);
		return cp_page_1;
	}

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
	unsigned int cp_payload, cp_blks;
	int ret;

	cp_payload = get_sb(cp_payload);
	if (cp_payload > F2FS_BLK_ALIGN(MAX_CP_PAYLOAD))
		return -EINVAL;

	cp_blks = 1 + cp_payload;
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

bool is_checkpoint_stop(struct f2fs_super_block *sb, bool abnormal)
{
	int i;

	for (i = 0; i < STOP_CP_REASON_MAX; i++) {
		if (abnormal && i == STOP_CP_REASON_SHUTDOWN)
			continue;
		if (sb->s_stop_reason[i])
			return true;
	}

	return false;
}

bool is_inconsistent_error(struct f2fs_super_block *sb)
{
	int i;

	for (i = 0; i < MAX_F2FS_ERRORS; i++) {
		if (sb->s_errors[i])
			return true;
	}

	return false;
}

/*
 * For a return value of 1, caller should further check for c.fix_on state
 * and take appropriate action.
 */
static int f2fs_should_proceed(struct f2fs_super_block *sb, u32 flag)
{
	if (!c.fix_on && (c.auto_fix || c.preen_mode)) {
		if (flag & CP_FSCK_FLAG ||
			flag & CP_QUOTA_NEED_FSCK_FLAG ||
			c.abnormal_stop || c.fs_errors ||
			(exist_qf_ino(sb) && (flag & CP_ERROR_FLAG))) {
			c.fix_on = 1;
		} else if (!c.preen_mode) {
			print_cp_state(flag);
			return 0;
		}
	}
	return 1;
}

int sanity_check_ckpt(struct f2fs_sb_info *sbi)
{
	unsigned int total, fsmeta;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	unsigned int flag = get_cp(ckpt_flags);
	unsigned int ovp_segments, reserved_segments;
	unsigned int main_segs, blocks_per_seg;
	unsigned int sit_segs, nat_segs;
	unsigned int sit_bitmap_size, nat_bitmap_size;
	unsigned int log_blocks_per_seg;
	unsigned int segment_count_main;
	unsigned int cp_pack_start_sum, cp_payload;
	block_t user_block_count;
	int i;

	total = get_sb(segment_count);
	fsmeta = get_sb(segment_count_ckpt);
	sit_segs = get_sb(segment_count_sit);
	fsmeta += sit_segs;
	nat_segs = get_sb(segment_count_nat);
	fsmeta += nat_segs;
	fsmeta += get_cp(rsvd_segment_count);
	fsmeta += get_sb(segment_count_ssa);

	if (fsmeta >= total)
		return 1;

	ovp_segments = get_cp(overprov_segment_count);
	reserved_segments = get_cp(rsvd_segment_count);

	if (!(get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO)) &&
		(fsmeta < F2FS_MIN_SEGMENT || ovp_segments == 0 ||
					reserved_segments == 0)) {
		MSG(0, "\tWrong layout: check mkfs.f2fs version\n");
		return 1;
	}

	user_block_count = get_cp(user_block_count);
	segment_count_main = get_sb(segment_count_main) +
				(cpu_to_le32(F2FS_FEATURE_RO) ? 1 : 0);
	log_blocks_per_seg = get_sb(log_blocks_per_seg);
	if (!user_block_count || user_block_count >=
			segment_count_main << log_blocks_per_seg) {
		ASSERT_MSG("\tWrong user_block_count(%u)\n", user_block_count);

		if (!f2fs_should_proceed(sb, flag))
			return 1;
		if (!c.fix_on)
			return 1;

		if (flag & (CP_FSCK_FLAG | CP_RESIZEFS_FLAG)) {
			u32 valid_user_block_cnt;
			u32 seg_cnt_main = get_sb(segment_count) -
					(get_sb(segment_count_ckpt) +
					 get_sb(segment_count_sit) +
					 get_sb(segment_count_nat) +
					 get_sb(segment_count_ssa));

			/* validate segment_count_main in sb first */
			if (seg_cnt_main != get_sb(segment_count_main)) {
				MSG(0, "Inconsistent segment_cnt_main %u in sb\n",
						segment_count_main << log_blocks_per_seg);
				return 1;
			}
			valid_user_block_cnt = ((get_sb(segment_count_main) -
						get_cp(overprov_segment_count)) * c.blks_per_seg);
			MSG(0, "Info: Fix wrong user_block_count in CP: (%u) -> (%u)\n",
					user_block_count, valid_user_block_cnt);
			set_cp(user_block_count, valid_user_block_cnt);
			c.bug_on = 1;
		}
	}

	main_segs = get_sb(segment_count_main);
	blocks_per_seg = sbi->blocks_per_seg;

	for (i = 0; i < NR_CURSEG_NODE_TYPE; i++) {
		if (get_cp(cur_node_segno[i]) >= main_segs ||
			get_cp(cur_node_blkoff[i]) >= blocks_per_seg)
			return 1;
	}
	for (i = 0; i < NR_CURSEG_DATA_TYPE; i++) {
		if (get_cp(cur_data_segno[i]) >= main_segs ||
			get_cp(cur_data_blkoff[i]) >= blocks_per_seg)
			return 1;
	}

	sit_bitmap_size = get_cp(sit_ver_bitmap_bytesize);
	nat_bitmap_size = get_cp(nat_ver_bitmap_bytesize);

	if (sit_bitmap_size != ((sit_segs / 2) << log_blocks_per_seg) / 8 ||
		nat_bitmap_size != ((nat_segs / 2) << log_blocks_per_seg) / 8) {
		MSG(0, "\tWrong bitmap size: sit(%u), nat(%u)\n",
				sit_bitmap_size, nat_bitmap_size);
		return 1;
	}

	cp_pack_start_sum = __start_sum_addr(sbi);
	cp_payload = __cp_payload(sbi);
	if (cp_pack_start_sum < cp_payload + 1 ||
		cp_pack_start_sum > blocks_per_seg - 1 -
			NR_CURSEG_TYPE) {
		MSG(0, "\tWrong cp_pack_start_sum(%u) or cp_payload(%u)\n",
			cp_pack_start_sum, cp_payload);
		if ((get_sb(feature) & F2FS_FEATURE_SB_CHKSUM))
			return 1;
		set_sb(cp_payload, cp_pack_start_sum - 1);
		update_superblock(sb, SB_MASK_ALL);
	}

	return 0;
}

pgoff_t current_nat_addr(struct f2fs_sb_info *sbi, nid_t start, int *pack)
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
	if (pack)
		*pack = 1;

	if (f2fs_test_bit(block_off, nm_i->nat_bitmap)) {
		block_addr += sbi->blocks_per_seg;
		if (pack)
			*pack = 2;
	}

	return block_addr;
}

/* will not init nid_bitmap from nat */
static int f2fs_early_init_nid_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	int nid_bitmap_size = (nm_i->max_nid + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_summary_block *sum = curseg->sum_blk;
	struct f2fs_journal *journal = &sum->journal;
	nid_t nid;
	int i;

	if (!(c.func == SLOAD || c.func == FSCK))
		return 0;

	nm_i->nid_bitmap = (char *)calloc(nid_bitmap_size, 1);
	if (!nm_i->nid_bitmap)
		return -ENOMEM;

	/* arbitrarily set 0 bit */
	f2fs_set_bit(0, nm_i->nid_bitmap);

	if (nats_in_cursum(journal) > NAT_JOURNAL_ENTRIES) {
		MSG(0, "\tError: f2fs_init_nid_bitmap truncate n_nats(%u) to "
			"NAT_JOURNAL_ENTRIES(%zu)\n",
			nats_in_cursum(journal), NAT_JOURNAL_ENTRIES);
		journal->n_nats = cpu_to_le16(NAT_JOURNAL_ENTRIES);
		c.fix_on = 1;
	}

	for (i = 0; i < nats_in_cursum(journal); i++) {
		block_t addr;

		addr = le32_to_cpu(nat_in_journal(journal, i).block_addr);
		if (!IS_VALID_BLK_ADDR(sbi, addr)) {
			MSG(0, "\tError: f2fs_init_nid_bitmap: addr(%u) is invalid!!!\n", addr);
			journal->n_nats = cpu_to_le16(i);
			c.fix_on = 1;
			continue;
		}

		nid = le32_to_cpu(nid_in_journal(journal, i));
		if (!IS_VALID_NID(sbi, nid)) {
			MSG(0, "\tError: f2fs_init_nid_bitmap: nid(%u) is invalid!!!\n", nid);
			journal->n_nats = cpu_to_le16(i);
			c.fix_on = 1;
			continue;
		}
		if (addr != NULL_ADDR)
			f2fs_set_bit(nid, nm_i->nid_bitmap);
	}
	return 0;
}

/* will init nid_bitmap from nat */
static int f2fs_late_init_nid_bitmap(struct f2fs_sb_info *sbi)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	struct f2fs_nat_block *nat_block;
	block_t start_blk;
	nid_t nid;

	if (!(c.func == SLOAD || c.func == FSCK))
		return 0;

	nat_block = malloc(F2FS_BLKSIZE);
	if (!nat_block) {
		free(nm_i->nid_bitmap);
		return -ENOMEM;
	}

	f2fs_ra_meta_pages(sbi, 0, NAT_BLOCK_OFFSET(nm_i->max_nid),
							META_NAT);
	for (nid = 0; nid < nm_i->max_nid; nid++) {
		if (!(nid % NAT_ENTRY_PER_BLOCK)) {
			int ret;

			start_blk = current_nat_addr(sbi, nid, NULL);
			ret = dev_read_block(nat_block, start_blk);
			ASSERT(ret >= 0);
		}

		if (nat_block->entries[nid % NAT_ENTRY_PER_BLOCK].block_addr)
			f2fs_set_bit(nid, nm_i->nid_bitmap);
	}

	free(nat_block);
	return 0;
}

u32 update_nat_bits_flags(struct f2fs_super_block *sb,
				struct f2fs_checkpoint *cp, u32 flags)
{
	uint32_t nat_bits_bytes, nat_bits_blocks;

	nat_bits_bytes = get_sb(segment_count_nat) << 5;
	nat_bits_blocks = F2FS_BYTES_TO_BLK((nat_bits_bytes << 1) + 8 +
						F2FS_BLKSIZE - 1);
	if (get_cp(cp_pack_total_block_count) <=
			(1 << get_sb(log_blocks_per_seg)) - nat_bits_blocks)
		flags |= CP_NAT_BITS_FLAG;
	else
		flags &= (~CP_NAT_BITS_FLAG);

	return flags;
}

/* should call flush_journal_entries() bfore this */
void write_nat_bits(struct f2fs_sb_info *sbi,
	struct f2fs_super_block *sb, struct f2fs_checkpoint *cp, int set)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	uint32_t nat_blocks = get_sb(segment_count_nat) <<
				(get_sb(log_blocks_per_seg) - 1);
	uint32_t nat_bits_bytes = nat_blocks >> 3;
	uint32_t nat_bits_blocks = F2FS_BYTES_TO_BLK((nat_bits_bytes << 1) +
					8 + F2FS_BLKSIZE - 1);
	unsigned char *nat_bits, *full_nat_bits, *empty_nat_bits;
	struct f2fs_nat_block *nat_block;
	uint32_t i, j;
	block_t blkaddr;
	int ret;

	nat_bits = calloc(F2FS_BLKSIZE, nat_bits_blocks);
	ASSERT(nat_bits);

	nat_block = malloc(F2FS_BLKSIZE);
	ASSERT(nat_block);

	full_nat_bits = nat_bits + 8;
	empty_nat_bits = full_nat_bits + nat_bits_bytes;

	memset(full_nat_bits, 0, nat_bits_bytes);
	memset(empty_nat_bits, 0, nat_bits_bytes);

	for (i = 0; i < nat_blocks; i++) {
		int seg_off = i >> get_sb(log_blocks_per_seg);
		int valid = 0;

		blkaddr = (pgoff_t)(get_sb(nat_blkaddr) +
				(seg_off << get_sb(log_blocks_per_seg) << 1) +
				(i & ((1 << get_sb(log_blocks_per_seg)) - 1)));

		/*
		 * Should consider new nat_blocks is larger than old
		 * nm_i->nat_blocks, since nm_i->nat_bitmap is based on
		 * old one.
		 */
		if (i < nm_i->nat_blocks && f2fs_test_bit(i, nm_i->nat_bitmap))
			blkaddr += (1 << get_sb(log_blocks_per_seg));

		ret = dev_read_block(nat_block, blkaddr);
		ASSERT(ret >= 0);

		for (j = 0; j < NAT_ENTRY_PER_BLOCK; j++) {
			if ((i == 0 && j == 0) ||
				nat_block->entries[j].block_addr != NULL_ADDR)
				valid++;
		}
		if (valid == 0)
			test_and_set_bit_le(i, empty_nat_bits);
		else if (valid == NAT_ENTRY_PER_BLOCK)
			test_and_set_bit_le(i, full_nat_bits);
	}
	*(__le64 *)nat_bits = get_cp_crc(cp);
	free(nat_block);

	blkaddr = get_sb(segment0_blkaddr) + (set <<
				get_sb(log_blocks_per_seg)) - nat_bits_blocks;

	DBG(1, "\tWriting NAT bits pages, at offset 0x%08x\n", blkaddr);

	for (i = 0; i < nat_bits_blocks; i++) {
		if (dev_write_block(nat_bits + i * F2FS_BLKSIZE, blkaddr + i))
			ASSERT_MSG("\tError: write NAT bits to disk!!!\n");
	}
	MSG(0, "Info: Write valid nat_bits in checkpoint\n");

	free(nat_bits);
}

static int check_nat_bits(struct f2fs_sb_info *sbi,
	struct f2fs_super_block *sb, struct f2fs_checkpoint *cp)
{
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	uint32_t nat_blocks = get_sb(segment_count_nat) <<
				(get_sb(log_blocks_per_seg) - 1);
	uint32_t nat_bits_bytes = nat_blocks >> 3;
	uint32_t nat_bits_blocks = F2FS_BYTES_TO_BLK((nat_bits_bytes << 1) +
					8 + F2FS_BLKSIZE - 1);
	unsigned char *nat_bits, *full_nat_bits, *empty_nat_bits;
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_HOT_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	uint32_t i, j;
	block_t blkaddr;
	int err = 0;

	nat_bits = calloc(F2FS_BLKSIZE, nat_bits_blocks);
	ASSERT(nat_bits);

	full_nat_bits = nat_bits + 8;
	empty_nat_bits = full_nat_bits + nat_bits_bytes;

	blkaddr = get_sb(segment0_blkaddr) + (sbi->cur_cp <<
				get_sb(log_blocks_per_seg)) - nat_bits_blocks;

	for (i = 0; i < nat_bits_blocks; i++) {
		if (dev_read_block(nat_bits + i * F2FS_BLKSIZE, blkaddr + i))
			ASSERT_MSG("\tError: read NAT bits to disk!!!\n");
	}

	if (*(__le64 *)nat_bits != get_cp_crc(cp) || nats_in_cursum(journal)) {
		/*
		 * if there is a journal, f2fs was not shutdown cleanly. Let's
		 * flush them with nat_bits.
		 */
		if (c.fix_on)
			err = -1;
		/* Otherwise, kernel will disable nat_bits */
		goto out;
	}

	for (i = 0; i < nat_blocks; i++) {
		uint32_t start_nid = i * NAT_ENTRY_PER_BLOCK;
		uint32_t valid = 0;
		int empty = test_bit_le(i, empty_nat_bits);
		int full = test_bit_le(i, full_nat_bits);

		for (j = 0; j < NAT_ENTRY_PER_BLOCK; j++) {
			if (f2fs_test_bit(start_nid + j, nm_i->nid_bitmap))
				valid++;
		}
		if (valid == 0) {
			if (!empty || full) {
				err = -1;
				goto out;
			}
		} else if (valid == NAT_ENTRY_PER_BLOCK) {
			if (empty || !full) {
				err = -1;
				goto out;
			}
		} else {
			if (empty || full) {
				err = -1;
				goto out;
			}
		}
	}
out:
	free(nat_bits);
	if (!err) {
		MSG(0, "Info: Checked valid nat_bits in checkpoint\n");
	} else {
		c.bug_nat_bits = 1;
		MSG(0, "Info: Corrupted valid nat_bits in checkpoint\n");
	}
	return err;
}

int init_node_manager(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_nm_info *nm_i = NM_I(sbi);
	unsigned char *version_bitmap;
	unsigned int nat_segs;

	nm_i->nat_blkaddr = get_sb(nat_blkaddr);

	/* segment_count_nat includes pair segment so divide to 2. */
	nat_segs = get_sb(segment_count_nat) >> 1;
	nm_i->nat_blocks = nat_segs << get_sb(log_blocks_per_seg);
	nm_i->max_nid = NAT_ENTRY_PER_BLOCK * nm_i->nat_blocks;
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
	return f2fs_early_init_nid_bitmap(sbi);
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
	unsigned int sit_segs;
	int start;
	char *src_bitmap, *dst_bitmap;
	unsigned char *bitmap;
	unsigned int bitmap_size;

	sit_i = malloc(sizeof(struct sit_info));
	if (!sit_i) {
		MSG(1, "\tError: Malloc failed for build_sit_info!\n");
		return -ENOMEM;
	}

	SM_I(sbi)->sit_info = sit_i;

	sit_i->sentries = calloc(MAIN_SEGS(sbi) * sizeof(struct seg_entry), 1);
	if (!sit_i->sentries) {
		MSG(1, "\tError: Calloc failed for build_sit_info!\n");
		goto free_sit_info;
	}

	bitmap_size = MAIN_SEGS(sbi) * SIT_VBLOCK_MAP_SIZE;

	if (need_fsync_data_record(sbi))
		bitmap_size += bitmap_size;

	sit_i->bitmap = calloc(bitmap_size, 1);
	if (!sit_i->bitmap) {
		MSG(1, "\tError: Calloc failed for build_sit_info!!\n");
		goto free_sentries;
	}

	bitmap = sit_i->bitmap;

	for (start = 0; start < MAIN_SEGS(sbi); start++) {
		sit_i->sentries[start].cur_valid_map = bitmap;
		bitmap += SIT_VBLOCK_MAP_SIZE;

		if (need_fsync_data_record(sbi)) {
			sit_i->sentries[start].ckpt_valid_map = bitmap;
			bitmap += SIT_VBLOCK_MAP_SIZE;
		}
	}

	sit_segs = get_sb(segment_count_sit) >> 1;
	bitmap_size = __bitmap_size(sbi, SIT_BITMAP);
	src_bitmap = __bitmap_ptr(sbi, SIT_BITMAP);

	dst_bitmap = malloc(bitmap_size);
	if (!dst_bitmap) {
		MSG(1, "\tError: Malloc failed for build_sit_info!!\n");
		goto free_validity_maps;
	}

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

free_validity_maps:
	free(sit_i->bitmap);
free_sentries:
	free(sit_i->sentries);
free_sit_info:
	free(sit_i);

	return -ENOMEM;
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
	se->dirty = 1;
}

static void read_compacted_summaries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg;
	unsigned int i, j, offset;
	block_t start;
	char *kaddr;
	int ret;

	start = start_sum_block(sbi);

	kaddr = malloc(F2FS_BLKSIZE);
	ASSERT(kaddr);

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

		ASSERT(blk_off <= ENTRIES_IN_SUM);

		for (j = 0; j < blk_off; j++) {
			struct f2fs_summary *s;
			s = (struct f2fs_summary *)(kaddr + offset);
			curseg->sum_blk->entries[j] = *s;
			offset += SUMMARY_SIZE;
			if (offset + SUMMARY_SIZE <=
					F2FS_BLKSIZE - SUM_FOOTER_SIZE)
				continue;
			memset(kaddr, 0, F2FS_BLKSIZE);
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

	sum_blk = malloc(sizeof(*sum_blk));
	ASSERT(sum_blk);

	ret = dev_read_block(sum_blk, blk_addr);
	ASSERT(ret >= 0);

	if (IS_NODESEG(type) && !is_set_ckpt_flags(cp, CP_UMOUNT_FLAG))
		restore_node_summary(sbi, segno, sum_blk);

	curseg = CURSEG_I(sbi, type);
	memcpy(curseg->sum_blk, sum_blk, sizeof(*sum_blk));
	reset_curseg(sbi, type);
	free(sum_blk);
}

void update_sum_entry(struct f2fs_sb_info *sbi, block_t blk_addr,
					struct f2fs_summary *sum)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_summary_block *sum_blk;
	u32 segno, offset;
	int type, ret;
	struct seg_entry *se;

	if (get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO))
		return;

	segno = GET_SEGNO(sbi, blk_addr);
	offset = OFFSET_IN_SEG(sbi, blk_addr);

	se = get_seg_entry(sbi, segno);

	sum_blk = get_sum_block(sbi, segno, &type);
	memcpy(&sum_blk->entries[offset], sum, sizeof(*sum));
	sum_blk->footer.entry_type = IS_NODESEG(se->type) ? SUM_TYPE_NODE :
							SUM_TYPE_DATA;

	/* write SSA all the time */
	ret = dev_write_block(sum_blk, GET_SUM_BLKADDR(sbi, segno));
	ASSERT(ret >= 0);

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

static int build_curseg(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct curseg_info *array;
	unsigned short blk_off;
	unsigned int segno;
	int i;

	array = malloc(sizeof(*array) * NR_CURSEG_TYPE);
	if (!array) {
		MSG(1, "\tError: Malloc failed for build_curseg!\n");
		return -ENOMEM;
	}

	SM_I(sbi)->curseg_array = array;

	for (i = 0; i < NR_CURSEG_TYPE; i++) {
		array[i].sum_blk = calloc(sizeof(*(array[i].sum_blk)), 1);
		if (!array[i].sum_blk) {
			MSG(1, "\tError: Calloc failed for build_curseg!!\n");
			goto seg_cleanup;
		}

		if (i <= CURSEG_COLD_DATA) {
			blk_off = get_cp(cur_data_blkoff[i]);
			segno = get_cp(cur_data_segno[i]);
		}
		if (i > CURSEG_COLD_DATA) {
			blk_off = get_cp(cur_node_blkoff[i - CURSEG_HOT_NODE]);
			segno = get_cp(cur_node_segno[i - CURSEG_HOT_NODE]);
		}
		ASSERT(segno < MAIN_SEGS(sbi));
		ASSERT(blk_off < DEFAULT_BLOCKS_PER_SEGMENT);

		array[i].segno = segno;
		array[i].zone = GET_ZONENO_FROM_SEGNO(sbi, segno);
		array[i].next_segno = NULL_SEGNO;
		array[i].next_blkoff = blk_off;
		array[i].alloc_type = cp->alloc_type[i];
	}
	restore_curseg_summaries(sbi);
	return 0;

seg_cleanup:
	for(--i ; i >=0; --i)
		free(array[i].sum_blk);
	free(array);

	return -ENOMEM;
}

static inline void check_seg_range(struct f2fs_sb_info *sbi, unsigned int segno)
{
	unsigned int end_segno = SM_I(sbi)->segment_count - 1;
	ASSERT(segno <= end_segno);
}

static inline block_t current_sit_addr(struct f2fs_sb_info *sbi,
						unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	unsigned int offset = SIT_BLOCK_OFFSET(sit_i, segno);
	block_t blk_addr = sit_i->sit_base_addr + offset;

	check_seg_range(sbi, segno);

	/* calculate sit block address */
	if (f2fs_test_bit(offset, sit_i->sit_bitmap))
		blk_addr += sit_i->sit_blocks;

	return blk_addr;
}

void get_current_sit_page(struct f2fs_sb_info *sbi,
			unsigned int segno, struct f2fs_sit_block *sit_blk)
{
	block_t blk_addr = current_sit_addr(sbi, segno);

	ASSERT(dev_read_block(sit_blk, blk_addr) >= 0);
}

void rewrite_current_sit_page(struct f2fs_sb_info *sbi,
			unsigned int segno, struct f2fs_sit_block *sit_blk)
{
	block_t blk_addr = current_sit_addr(sbi, segno);

	ASSERT(dev_write_block(sit_blk, blk_addr) >= 0);
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

void __seg_info_from_raw_sit(struct seg_entry *se,
		struct f2fs_sit_entry *raw_sit)
{
	se->valid_blocks = GET_SIT_VBLOCKS(raw_sit);
	memcpy(se->cur_valid_map, raw_sit->valid_map, SIT_VBLOCK_MAP_SIZE);
	se->type = GET_SIT_TYPE(raw_sit);
	se->orig_type = GET_SIT_TYPE(raw_sit);
	se->mtime = le64_to_cpu(raw_sit->mtime);
}

void seg_info_from_raw_sit(struct f2fs_sb_info *sbi, struct seg_entry *se,
						struct f2fs_sit_entry *raw_sit)
{
	__seg_info_from_raw_sit(se, raw_sit);

	if (!need_fsync_data_record(sbi))
		return;
	se->ckpt_valid_blocks = se->valid_blocks;
	memcpy(se->ckpt_valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
	se->ckpt_type = se->type;
}

struct seg_entry *get_seg_entry(struct f2fs_sb_info *sbi,
		unsigned int segno)
{
	struct sit_info *sit_i = SIT_I(sbi);
	return &sit_i->sentries[segno];
}

unsigned short get_seg_vblocks(struct f2fs_sb_info *sbi, struct seg_entry *se)
{
	if (!need_fsync_data_record(sbi))
		return se->valid_blocks;
	else
		return se->ckpt_valid_blocks;
}

unsigned char *get_seg_bitmap(struct f2fs_sb_info *sbi, struct seg_entry *se)
{
	if (!need_fsync_data_record(sbi))
		return se->cur_valid_map;
	else
		return se->ckpt_valid_map;
}

unsigned char get_seg_type(struct f2fs_sb_info *sbi, struct seg_entry *se)
{
	if (!need_fsync_data_record(sbi))
		return se->type;
	else
		return se->ckpt_type;
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
	block_addr = current_nat_addr(sbi, nid, NULL);

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
		int ofs = get_extra_isize(node_blk);

		oldaddr = le32_to_cpu(node_blk->i.i_addr[ofs + ofs_in_node]);
		node_blk->i.i_addr[ofs + ofs_in_node] = cpu_to_le32(newaddr);
		ret = write_inode(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	} else {
		oldaddr = le32_to_cpu(node_blk->dn.addr[ofs_in_node]);
		node_blk->dn.addr[ofs_in_node] = cpu_to_le32(newaddr);
		ret = dev_write_block(node_blk, ni.blk_addr);
		ASSERT(ret >= 0);
	}

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
		ASSERT(write_inode(node_blk, ni.blk_addr) >= 0);
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
	block_addr = current_nat_addr(sbi, nid, NULL);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	if (ino)
		nat_block->entries[entry_off].ino = cpu_to_le32(ino);
	nat_block->entries[entry_off].block_addr = cpu_to_le32(newaddr);
	if (c.func == FSCK)
		F2FS_FSCK(sbi)->entries[nid] = nat_block->entries[entry_off];

	ret = dev_write_block(nat_block, block_addr);
	ASSERT(ret >= 0);
	free(nat_block);
}

void get_node_info(struct f2fs_sb_info *sbi, nid_t nid, struct node_info *ni)
{
	struct f2fs_nat_entry raw_nat;

	ni->nid = nid;
	if (c.func == FSCK && F2FS_FSCK(sbi)->nr_nat_entries) {
		node_info_from_raw_nat(ni, &(F2FS_FSCK(sbi)->entries[nid]));
		if (ni->blk_addr)
			return;
		/* nat entry is not cached, read it */
	}

	get_nat_entry(sbi, nid, &raw_nat);
	node_info_from_raw_nat(ni, &raw_nat);
}

static int build_sit_entries(struct f2fs_sb_info *sbi)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	struct f2fs_sit_block *sit_blk;
	struct seg_entry *se;
	struct f2fs_sit_entry sit;
	int sit_blk_cnt = SIT_BLK_CNT(sbi);
	unsigned int i, segno, end;
	unsigned int readed, start_blk = 0;

	sit_blk = calloc(BLOCK_SZ, 1);
	if (!sit_blk) {
		MSG(1, "\tError: Calloc failed for build_sit_entries!\n");
		return -ENOMEM;
	}

	do {
		readed = f2fs_ra_meta_pages(sbi, start_blk, MAX_RA_BLOCKS,
								META_SIT);

		segno = start_blk * sit_i->sents_per_block;
		end = (start_blk + readed) * sit_i->sents_per_block;

		for (; segno < end && segno < MAIN_SEGS(sbi); segno++) {
			se = &sit_i->sentries[segno];

			get_current_sit_page(sbi, segno, sit_blk);
			sit = sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];

			check_block_count(sbi, segno, &sit);
			seg_info_from_raw_sit(sbi, se, &sit);
		}
		start_blk += readed;
	} while (start_blk < sit_blk_cnt);


	free(sit_blk);

	if (sits_in_cursum(journal) > SIT_JOURNAL_ENTRIES) {
		MSG(0, "\tError: build_sit_entries truncate n_sits(%u) to "
			"SIT_JOURNAL_ENTRIES(%zu)\n",
			sits_in_cursum(journal), SIT_JOURNAL_ENTRIES);
		journal->n_sits = cpu_to_le16(SIT_JOURNAL_ENTRIES);
		c.fix_on = 1;
	}

	for (i = 0; i < sits_in_cursum(journal); i++) {
		segno = le32_to_cpu(segno_in_journal(journal, i));

		if (segno >= MAIN_SEGS(sbi)) {
			MSG(0, "\tError: build_sit_entries: segno(%u) is invalid!!!\n", segno);
			journal->n_sits = cpu_to_le16(i);
			c.fix_on = 1;
			continue;
		}

		se = &sit_i->sentries[segno];
		sit = sit_in_journal(journal, i);

		check_block_count(sbi, segno, &sit);
		seg_info_from_raw_sit(sbi, se, &sit);
	}
	return 0;
}

static int early_build_segment_manager(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_sm_info *sm_info;

	sm_info = malloc(sizeof(struct f2fs_sm_info));
	if (!sm_info) {
		MSG(1, "\tError: Malloc failed for build_segment_manager!\n");
		return -ENOMEM;
	}

	/* init sm info */
	sbi->sm_info = sm_info;
	sm_info->seg0_blkaddr = get_sb(segment0_blkaddr);
	sm_info->main_blkaddr = get_sb(main_blkaddr);
	sm_info->segment_count = get_sb(segment_count);
	sm_info->reserved_segments = get_cp(rsvd_segment_count);
	sm_info->ovp_segments = get_cp(overprov_segment_count);
	sm_info->main_segments = get_sb(segment_count_main);
	sm_info->ssa_blkaddr = get_sb(ssa_blkaddr);

	if (build_sit_info(sbi) || build_curseg(sbi)) {
		free(sm_info);
		return -ENOMEM;
	}

	return 0;
}

static int late_build_segment_manager(struct f2fs_sb_info *sbi)
{
	if (sbi->seg_manager_done)
		return 1; /* this function was already called */

	sbi->seg_manager_done = true;
	if (build_sit_entries(sbi)) {
		free (sbi->sm_info);
		return -ENOMEM;
	}

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

	for (segno = 0; segno < MAIN_SEGS(sbi); segno++) {
		se = get_seg_entry(sbi, segno);

		memcpy(ptr, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		ptr += SIT_VBLOCK_MAP_SIZE;

		if (se->valid_blocks == 0x0 && is_usable_seg(sbi, segno)) {
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
	struct f2fs_sit_block *sit_blk;
	unsigned int segno = 0;
	struct f2fs_summary_block *sum = curseg->sum_blk;
	char *ptr = NULL;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	/* remove sit journal */
	sum->journal.n_sits = 0;

	ptr = fsck->main_area_bitmap;

	for (segno = 0; segno < MAIN_SEGS(sbi); segno++) {
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;
		u16 valid_blocks = 0;
		u16 type;
		int i;

		get_current_sit_page(sbi, segno, sit_blk);
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
			ASSERT_MSG("Invalid type and valid blocks=%x,%x",
					segno, valid_blocks);
			type = 0;
		}
		sit->vblocks = cpu_to_le16((type << SIT_VBLOCKS_SHIFT) |
								valid_blocks);
		rewrite_current_sit_page(sbi, segno, sit_blk);

		ptr += SIT_VBLOCK_MAP_SIZE;
	}

	free(sit_blk);
}

static int flush_sit_journal_entries(struct f2fs_sb_info *sbi)
{
	struct curseg_info *curseg = CURSEG_I(sbi, CURSEG_COLD_DATA);
	struct f2fs_journal *journal = &curseg->sum_blk->journal;
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_sit_block *sit_blk;
	unsigned int segno;
	int i;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	for (i = 0; i < sits_in_cursum(journal); i++) {
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;

		segno = segno_in_journal(journal, i);
		se = get_seg_entry(sbi, segno);

		get_current_sit_page(sbi, segno, sit_blk);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];

		memcpy(sit->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		sit->mtime = cpu_to_le64(se->mtime);

		rewrite_current_sit_page(sbi, segno, sit_blk);
	}

	free(sit_blk);
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
	block_addr = current_nat_addr(sbi, nid, NULL);

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
		write_checkpoints(sbi);
}

void flush_sit_entries(struct f2fs_sb_info *sbi)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_sit_block *sit_blk;
	unsigned int segno = 0;

	sit_blk = calloc(BLOCK_SZ, 1);
	ASSERT(sit_blk);
	/* update free segments */
	for (segno = 0; segno < MAIN_SEGS(sbi); segno++) {
		struct f2fs_sit_entry *sit;
		struct seg_entry *se;

		se = get_seg_entry(sbi, segno);

		if (!se->dirty)
			continue;

		get_current_sit_page(sbi, segno, sit_blk);
		sit = &sit_blk->entries[SIT_ENTRY_OFFSET(sit_i, segno)];
		memcpy(sit->valid_map, se->cur_valid_map, SIT_VBLOCK_MAP_SIZE);
		sit->vblocks = cpu_to_le16((se->type << SIT_VBLOCKS_SHIFT) |
							se->valid_blocks);
		rewrite_current_sit_page(sbi, segno, sit_blk);
	}

	free(sit_blk);
}

int relocate_curseg_offset(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct seg_entry *se = get_seg_entry(sbi, curseg->segno);
	unsigned int i;

	if (c.zoned_model == F2FS_ZONED_HM)
		return -EINVAL;

	for (i = 0; i < sbi->blocks_per_seg; i++) {
		if (!f2fs_test_bit(i, (const char *)se->cur_valid_map))
			break;
	}

	if (i == sbi->blocks_per_seg)
		return -EINVAL;

	DBG(1, "Update curseg[%d].next_blkoff %u -> %u, alloc_type %s -> SSR\n",
			type, curseg->next_blkoff, i,
			curseg->alloc_type == LFS ? "LFS" : "SSR");

	curseg->next_blkoff = i;
	curseg->alloc_type = SSR;

	return 0;
}

void set_section_type(struct f2fs_sb_info *sbi, unsigned int segno, int type)
{
	unsigned int i;

	if (sbi->segs_per_sec == 1)
		return;

	for (i = 0; i < sbi->segs_per_sec; i++) {
		struct seg_entry *se = get_seg_entry(sbi, segno + i);

		se->type = type;
	}
}

#ifdef HAVE_LINUX_BLKZONED_H

static bool write_pointer_at_zone_start(struct f2fs_sb_info *sbi,
					unsigned int zone_segno)
{
	uint64_t sector;
	struct blk_zone blkz;
	block_t block = START_BLOCK(sbi, zone_segno);
	int log_sectors_per_block = sbi->log_blocksize - SECTOR_SHIFT;
	int ret, j;

	if (c.zoned_model != F2FS_ZONED_HM)
		return true;

	for (j = 0; j < MAX_DEVICES; j++) {
		if (!c.devices[j].path)
			break;
		if (c.devices[j].start_blkaddr <= block &&
		    block <= c.devices[j].end_blkaddr)
			break;
	}

	if (j >= MAX_DEVICES)
		return false;

	sector = (block - c.devices[j].start_blkaddr) << log_sectors_per_block;
	ret = f2fs_report_zone(j, sector, &blkz);
	if (ret)
		return false;

	if (blk_zone_type(&blkz) != BLK_ZONE_TYPE_SEQWRITE_REQ)
		return true;

	return blk_zone_sector(&blkz) == blk_zone_wp_sector(&blkz);
}

#else

static bool write_pointer_at_zone_start(struct f2fs_sb_info *UNUSED(sbi),
					unsigned int UNUSED(zone_segno))
{
	return true;
}

#endif

int find_next_free_block(struct f2fs_sb_info *sbi, u64 *to, int left,
						int want_type, bool new_sec)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct seg_entry *se;
	u32 segno;
	u32 offset;
	int not_enough = 0;
	u64 end_blkaddr = (get_sb(segment_count_main) <<
			get_sb(log_blocks_per_seg)) + get_sb(main_blkaddr);

	if (*to > 0)
		*to -= left;
	if (get_free_segments(sbi) <= SM_I(sbi)->reserved_segments + 1)
		not_enough = 1;

	while (*to >= SM_I(sbi)->main_blkaddr && *to < end_blkaddr) {
		unsigned short vblocks;
		unsigned char *bitmap;
		unsigned char type;

		segno = GET_SEGNO(sbi, *to);
		offset = OFFSET_IN_SEG(sbi, *to);

		se = get_seg_entry(sbi, segno);

		vblocks = get_seg_vblocks(sbi, se);
		bitmap = get_seg_bitmap(sbi, se);
		type = get_seg_type(sbi, se);

		if (vblocks == sbi->blocks_per_seg) {
next_segment:
			*to = left ? START_BLOCK(sbi, segno) - 1:
						START_BLOCK(sbi, segno + 1);
			continue;
		}
		if (!(get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO)) &&
						IS_CUR_SEGNO(sbi, segno))
			goto next_segment;
		if (vblocks == 0 && not_enough)
			goto next_segment;

		if (vblocks == 0 && !(segno % sbi->segs_per_sec)) {
			struct seg_entry *se2;
			unsigned int i;

			for (i = 1; i < sbi->segs_per_sec; i++) {
				se2 = get_seg_entry(sbi, segno + i);
				if (get_seg_vblocks(sbi, se2))
					break;
			}

			if (i == sbi->segs_per_sec &&
			    write_pointer_at_zone_start(sbi, segno)) {
				set_section_type(sbi, segno, want_type);
				return 0;
			}
		}

		if (type == want_type && !new_sec &&
			!f2fs_test_bit(offset, (const char *)bitmap))
			return 0;

		*to = left ? *to - 1: *to + 1;
	}
	return -1;
}

static void move_one_curseg_info(struct f2fs_sb_info *sbi, u64 from, int left,
				 int i)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	struct curseg_info *curseg = CURSEG_I(sbi, i);
	struct f2fs_summary_block buf;
	u32 old_segno;
	u64 ssa_blk, to;
	int ret;

	if ((get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO))) {
		if (i != CURSEG_HOT_DATA && i != CURSEG_HOT_NODE)
			return;

		if (i == CURSEG_HOT_DATA) {
			left = 0;
			from = SM_I(sbi)->main_blkaddr;
		} else {
			left = 1;
			from = __end_block_addr(sbi);
		}
		goto bypass_ssa;
	}

	/* update original SSA too */
	ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
	ret = dev_write_block(curseg->sum_blk, ssa_blk);
	ASSERT(ret >= 0);
bypass_ssa:
	to = from;
	ret = find_next_free_block(sbi, &to, left, i,
				   c.zoned_model == F2FS_ZONED_HM);
	ASSERT(ret == 0);

	old_segno = curseg->segno;
	curseg->segno = GET_SEGNO(sbi, to);
	curseg->next_blkoff = OFFSET_IN_SEG(sbi, to);
	curseg->alloc_type = c.zoned_model == F2FS_ZONED_HM ? LFS : SSR;

	/* update new segno */
	ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
	ret = dev_read_block(&buf, ssa_blk);
	ASSERT(ret >= 0);

	memcpy(curseg->sum_blk, &buf, SUM_ENTRIES_SIZE);

	/* update se->types */
	reset_curseg(sbi, i);

	FIX_MSG("Move curseg[%d] %x -> %x after %"PRIx64"\n",
		i, old_segno, curseg->segno, from);
}

void move_curseg_info(struct f2fs_sb_info *sbi, u64 from, int left)
{
	int i;

	/* update summary blocks having nullified journal entries */
	for (i = 0; i < NO_CHECK_TYPE; i++)
		move_one_curseg_info(sbi, from, left, i);
}

void update_curseg_info(struct f2fs_sb_info *sbi, int type)
{
	if (!relocate_curseg_offset(sbi, type))
		return;
	move_one_curseg_info(sbi, SM_I(sbi)->main_blkaddr, 0, type);
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
			FIX_MSG("Remove nid [0x%x] in nat journal", nid);
			return;
		}
	}
	nat_block = (struct f2fs_nat_block *)calloc(BLOCK_SZ, 1);
	ASSERT(nat_block);

	entry_off = nid % NAT_ENTRY_PER_BLOCK;
	block_addr = current_nat_addr(sbi, nid, NULL);

	ret = dev_read_block(nat_block, block_addr);
	ASSERT(ret >= 0);

	if (nid == F2FS_NODE_INO(sbi) || nid == F2FS_META_INO(sbi)) {
		FIX_MSG("nid [0x%x] block_addr= 0x%x -> 0x1", nid,
			le32_to_cpu(nat_block->entries[entry_off].block_addr));
		nat_block->entries[entry_off].block_addr = cpu_to_le32(0x1);
	} else {
		memset(&nat_block->entries[entry_off], 0,
					sizeof(struct f2fs_nat_entry));
		FIX_MSG("Remove nid [0x%x] in NAT", nid);
	}

	ret = dev_write_block(nat_block, block_addr);
	ASSERT(ret >= 0);
	free(nat_block);
}

void duplicate_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	unsigned long long dst, src;
	void *buf;
	unsigned int seg_size = 1 << get_sb(log_blocks_per_seg);
	int ret;

	if (sbi->cp_backuped)
		return;

	buf = malloc(F2FS_BLKSIZE * seg_size);
	ASSERT(buf);

	if (sbi->cur_cp == 1) {
		src = get_sb(cp_blkaddr);
		dst = src + seg_size;
	} else {
		dst = get_sb(cp_blkaddr);
		src = dst + seg_size;
	}

	ret = dev_read(buf, src << F2FS_BLKSIZE_BITS,
				seg_size << F2FS_BLKSIZE_BITS);
	ASSERT(ret >= 0);

	ret = dev_write(buf, dst << F2FS_BLKSIZE_BITS,
				seg_size << F2FS_BLKSIZE_BITS);
	ASSERT(ret >= 0);

	free(buf);

	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);

	sbi->cp_backuped = 1;

	MSG(0, "Info: Duplicate valid checkpoint to mirror position "
		"%llu -> %llu\n", src, dst);
}

void write_checkpoint(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);
	block_t orphan_blks = 0;
	unsigned long long cp_blk_no;
	u32 flags = CP_UMOUNT_FLAG;
	int i, ret;
	uint32_t crc = 0;

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

	set_cp(free_segment_count, get_free_segments(sbi));
	if (c.func == FSCK) {
		struct f2fs_fsck *fsck = F2FS_FSCK(sbi);

		set_cp(valid_block_count, fsck->chk.valid_blk_cnt);
		set_cp(valid_node_count, fsck->chk.valid_node_cnt);
		set_cp(valid_inode_count, fsck->chk.valid_inode_cnt);
	} else {
		set_cp(valid_block_count, sbi->total_valid_block_count);
		set_cp(valid_node_count, sbi->total_valid_node_count);
		set_cp(valid_inode_count, sbi->total_valid_inode_count);
	}
	set_cp(cp_pack_total_block_count, 8 + orphan_blks + get_sb(cp_payload));

	flags = update_nat_bits_flags(sb, cp, flags);
	set_cp(ckpt_flags, flags);

	crc = f2fs_checkpoint_chksum(cp);
	*((__le32 *)((unsigned char *)cp + get_cp(checksum_offset))) =
							cpu_to_le32(crc);

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

		if (!(get_sb(feature) & cpu_to_le32(F2FS_FEATURE_RO))) {
			/* update original SSA too */
			ssa_blk = GET_SUM_BLKADDR(sbi, curseg->segno);
			ret = dev_write_block(curseg->sum_blk, ssa_blk);
			ASSERT(ret >= 0);
		}
	}

	/* Write nat bits */
	if (flags & CP_NAT_BITS_FLAG)
		write_nat_bits(sbi, sb, cp, sbi->cur_cp);

	/* in case of sudden power off */
	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);

	/* write the last cp */
	ret = dev_write_block(cp, cp_blk_no++);
	ASSERT(ret >= 0);

	ret = f2fs_fsync_device();
	ASSERT(ret >= 0);
}

void write_checkpoints(struct f2fs_sb_info *sbi)
{
	/* copy valid checkpoint to its mirror position */
	duplicate_checkpoint(sbi);

	/* repair checkpoint at CP #0 position */
	sbi->cur_cp = 1;
	write_checkpoint(sbi);
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
				/*
				 * block_addr of node/meta inode should be 0x1.
				 * Set this bit, and fsck_verify will fix it.
				 */
				if (le32_to_cpu(nat_block->entries[i].block_addr) != 0x1) {
					ASSERT_MSG("\tError: ino[0x%x] block_addr[0x%x] is invalid\n",
							nid + i, le32_to_cpu(nat_block->entries[i].block_addr));
					f2fs_set_bit(nid + i, fsck->nat_area_bitmap);
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
	uint32_t log_sectorsize, log_sectors_per_block;

	log_sectorsize = log_base_2(c.sector_size);
	log_sectors_per_block = log_base_2(c.sectors_per_blk);

	if (log_sectorsize == get_sb(log_sectorsize) &&
			log_sectors_per_block == get_sb(log_sectors_per_block))
		return 0;

	set_sb(log_sectorsize, log_sectorsize);
	set_sb(log_sectors_per_block, log_sectors_per_block);

	update_superblock(sb, SB_MASK_ALL);
	return 0;
}

static int tune_sb_features(struct f2fs_sb_info *sbi)
{
	int sb_changed = 0;
	struct f2fs_super_block *sb = F2FS_RAW_SUPER(sbi);

	if (!(sb->feature & cpu_to_le32(F2FS_FEATURE_ENCRYPT)) &&
			c.feature & cpu_to_le32(F2FS_FEATURE_ENCRYPT)) {
		sb->feature |= cpu_to_le32(F2FS_FEATURE_ENCRYPT);
		MSG(0, "Info: Set Encryption feature\n");
		sb_changed = 1;
	}
	if (!(sb->feature & cpu_to_le32(F2FS_FEATURE_CASEFOLD)) &&
		c.feature & cpu_to_le32(F2FS_FEATURE_CASEFOLD)) {
		if (!c.s_encoding) {
			ERR_MSG("ERROR: Must specify encoding to enable casefolding.\n");
			return -1;
		}
		sb->feature |= cpu_to_le32(F2FS_FEATURE_CASEFOLD);
		MSG(0, "Info: Set Casefold feature\n");
		sb_changed = 1;
	}
	/* TODO: quota needs to allocate inode numbers */

	c.feature = sb->feature;
	if (!sb_changed)
		return 0;

	update_superblock(sb, SB_MASK_ALL);
	return 0;
}

static struct fsync_inode_entry *get_fsync_inode(struct list_head *head,
								nid_t ino)
{
	struct fsync_inode_entry *entry;

	list_for_each_entry(entry, head, list)
		if (entry->ino == ino)
			return entry;

	return NULL;
}

static struct fsync_inode_entry *add_fsync_inode(struct list_head *head,
								nid_t ino)
{
	struct fsync_inode_entry *entry;

	entry = calloc(sizeof(struct fsync_inode_entry), 1);
	if (!entry)
		return NULL;
	entry->ino = ino;
	list_add_tail(&entry->list, head);
	return entry;
}

static void del_fsync_inode(struct fsync_inode_entry *entry)
{
	list_del(&entry->list);
	free(entry);
}

static void destroy_fsync_dnodes(struct list_head *head)
{
	struct fsync_inode_entry *entry, *tmp;

	list_for_each_entry_safe(entry, tmp, head, list)
		del_fsync_inode(entry);
}

static int find_fsync_inode(struct f2fs_sb_info *sbi, struct list_head *head)
{
	struct curseg_info *curseg;
	struct f2fs_node *node_blk;
	block_t blkaddr;
	unsigned int loop_cnt = 0;
	unsigned int free_blocks = MAIN_SEGS(sbi) * sbi->blocks_per_seg -
						sbi->total_valid_block_count;
	int err = 0;

	/* get node pages in the current segment */
	curseg = CURSEG_I(sbi, CURSEG_WARM_NODE);
	blkaddr = NEXT_FREE_BLKADDR(sbi, curseg);

	node_blk = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node_blk);

	while (1) {
		struct fsync_inode_entry *entry;

		if (!f2fs_is_valid_blkaddr(sbi, blkaddr, META_POR))
			break;

		err = dev_read_block(node_blk, blkaddr);
		if (err)
			break;

		if (!is_recoverable_dnode(sbi, node_blk))
			break;

		if (!is_fsync_dnode(node_blk))
			goto next;

		entry = get_fsync_inode(head, ino_of_node(node_blk));
		if (!entry) {
			entry = add_fsync_inode(head, ino_of_node(node_blk));
			if (!entry) {
				err = -1;
				break;
			}
		}
		entry->blkaddr = blkaddr;

		if (IS_INODE(node_blk) && is_dent_dnode(node_blk))
			entry->last_dentry = blkaddr;
next:
		/* sanity check in order to detect looped node chain */
		if (++loop_cnt >= free_blocks ||
			blkaddr == next_blkaddr_of_node(node_blk)) {
			MSG(0, "\tdetect looped node chain, blkaddr:%u, next:%u\n",
				    blkaddr,
				    next_blkaddr_of_node(node_blk));
			err = -1;
			break;
		}

		blkaddr = next_blkaddr_of_node(node_blk);
	}

	free(node_blk);
	return err;
}

static int do_record_fsync_data(struct f2fs_sb_info *sbi,
					struct f2fs_node *node_blk,
					block_t blkaddr)
{
	unsigned int segno, offset;
	struct seg_entry *se;
	unsigned int ofs_in_node = 0;
	unsigned int start, end;
	int err = 0, recorded = 0;

	segno = GET_SEGNO(sbi, blkaddr);
	se = get_seg_entry(sbi, segno);
	offset = OFFSET_IN_SEG(sbi, blkaddr);

	if (f2fs_test_bit(offset, (char *)se->cur_valid_map)) {
		ASSERT(0);
		return -1;
	}
	if (f2fs_test_bit(offset, (char *)se->ckpt_valid_map)) {
		ASSERT(0);
		return -1;
	}

	if (!se->ckpt_valid_blocks)
		se->ckpt_type = CURSEG_WARM_NODE;

	se->ckpt_valid_blocks++;
	f2fs_set_bit(offset, (char *)se->ckpt_valid_map);

	MSG(1, "do_record_fsync_data: [node] ino = %u, nid = %u, blkaddr = %u\n",
	    ino_of_node(node_blk), ofs_of_node(node_blk), blkaddr);

	/* inline data */
	if (IS_INODE(node_blk) && (node_blk->i.i_inline & F2FS_INLINE_DATA))
		return 0;
	/* xattr node */
	if (ofs_of_node(node_blk) == XATTR_NODE_OFFSET)
		return 0;

	/* step 3: recover data indices */
	start = start_bidx_of_node(ofs_of_node(node_blk), node_blk);
	end = start + ADDRS_PER_PAGE(sbi, node_blk, NULL);

	for (; start < end; start++, ofs_in_node++) {
		blkaddr = datablock_addr(node_blk, ofs_in_node);

		if (!is_valid_data_blkaddr(blkaddr))
			continue;

		if (!f2fs_is_valid_blkaddr(sbi, blkaddr, META_POR)) {
			err = -1;
			goto out;
		}

		segno = GET_SEGNO(sbi, blkaddr);
		se = get_seg_entry(sbi, segno);
		offset = OFFSET_IN_SEG(sbi, blkaddr);

		if (f2fs_test_bit(offset, (char *)se->cur_valid_map))
			continue;
		if (f2fs_test_bit(offset, (char *)se->ckpt_valid_map))
			continue;

		if (!se->ckpt_valid_blocks)
			se->ckpt_type = CURSEG_WARM_DATA;

		se->ckpt_valid_blocks++;
		f2fs_set_bit(offset, (char *)se->ckpt_valid_map);

		MSG(1, "do_record_fsync_data: [data] ino = %u, nid = %u, blkaddr = %u\n",
		    ino_of_node(node_blk), ofs_of_node(node_blk), blkaddr);

		recorded++;
	}
out:
	MSG(1, "recover_data: ino = %u, nid = %u, recorded = %d, err = %d\n",
		    ino_of_node(node_blk), ofs_of_node(node_blk),
		    recorded, err);
	return err;
}

static int traverse_dnodes(struct f2fs_sb_info *sbi,
				struct list_head *inode_list)
{
	struct curseg_info *curseg;
	struct f2fs_node *node_blk;
	block_t blkaddr;
	int err = 0;

	/* get node pages in the current segment */
	curseg = CURSEG_I(sbi, CURSEG_WARM_NODE);
	blkaddr = NEXT_FREE_BLKADDR(sbi, curseg);

	node_blk = calloc(F2FS_BLKSIZE, 1);
	ASSERT(node_blk);

	while (1) {
		struct fsync_inode_entry *entry;

		if (!f2fs_is_valid_blkaddr(sbi, blkaddr, META_POR))
			break;

		err = dev_read_block(node_blk, blkaddr);
		if (err)
			break;

		if (!is_recoverable_dnode(sbi, node_blk))
			break;

		entry = get_fsync_inode(inode_list,
					ino_of_node(node_blk));
		if (!entry)
			goto next;

		err = do_record_fsync_data(sbi, node_blk, blkaddr);
		if (err)
			break;

		if (entry->blkaddr == blkaddr)
			del_fsync_inode(entry);
next:
		blkaddr = next_blkaddr_of_node(node_blk);
	}

	free(node_blk);
	return err;
}

static int record_fsync_data(struct f2fs_sb_info *sbi)
{
	struct list_head inode_list = LIST_HEAD_INIT(inode_list);
	int ret;

	if (!need_fsync_data_record(sbi))
		return 0;

	ret = find_fsync_inode(sbi, &inode_list);
	if (ret)
		goto out;

	ret = late_build_segment_manager(sbi);
	if (ret < 0) {
		ERR_MSG("late_build_segment_manager failed\n");
		goto out;
	}

	ret = traverse_dnodes(sbi, &inode_list);
out:
	destroy_fsync_dnodes(&inode_list);
	return ret;
}

int f2fs_do_mount(struct f2fs_sb_info *sbi)
{
	struct f2fs_checkpoint *cp = NULL;
	struct f2fs_super_block *sb = NULL;
	int ret;

	sbi->active_logs = NR_CURSEG_TYPE;
	ret = validate_super_block(sbi, SB0_ADDR);
	if (ret) {
		ret = validate_super_block(sbi, SB1_ADDR);
		if (ret)
			return -1;
	}
	sb = F2FS_RAW_SUPER(sbi);

	ret = check_sector_size(sb);
	if (ret)
		return -1;

	print_raw_sb_info(sb);

	init_sb_info(sbi);

	ret = get_valid_checkpoint(sbi);
	if (ret) {
		ERR_MSG("Can't find valid checkpoint\n");
		return -1;
	}

	c.bug_on = 0;

	if (sanity_check_ckpt(sbi)) {
		ERR_MSG("Checkpoint is polluted\n");
		return -1;
	}
	cp = F2FS_CKPT(sbi);

	if (c.func != FSCK && c.func != DUMP &&
		!is_set_ckpt_flags(F2FS_CKPT(sbi), CP_UMOUNT_FLAG)) {
		ERR_MSG("Mount unclean image to replay log first\n");
		return -1;
	}

	if (c.func == FSCK) {
#if defined(__APPLE__)
		if (!c.no_kernel_check &&
			memcmp(c.sb_version, c.version,	VERSION_NAME_LEN)) {
			c.auto_fix = 0;
			c.fix_on = 1;
			memcpy(sbi->raw_super->version,
					c.version, VERSION_NAME_LEN);
			update_superblock(sbi->raw_super, SB_MASK_ALL);
		}
#else
		if (!c.no_kernel_check) {
			u32 prev_time, cur_time, time_diff;
			__le32 *ver_ts_ptr = (__le32 *)(sbi->raw_super->version
						+ VERSION_NAME_LEN);

			cur_time = (u32)get_cp(elapsed_time);
			prev_time = le32_to_cpu(*ver_ts_ptr);

			MSG(0, "Info: version timestamp cur: %u, prev: %u\n",
					cur_time, prev_time);
			if (!memcmp(c.sb_version, c.version,
						VERSION_NAME_LEN)) {
				/* valid prev_time */
				if (prev_time != 0 && cur_time > prev_time) {
					time_diff = cur_time - prev_time;
					if (time_diff < CHECK_PERIOD)
						goto out;
					c.auto_fix = 0;
					c.fix_on = 1;
				}
			} else {
				memcpy(sbi->raw_super->version,
						c.version, VERSION_NAME_LEN);
			}

			*ver_ts_ptr = cpu_to_le32(cur_time);
			update_superblock(sbi->raw_super, SB_MASK_ALL);
		}
#endif
	}
out:
	print_ckpt_info(sbi);

	if (c.quota_fix) {
		if (get_cp(ckpt_flags) & CP_QUOTA_NEED_FSCK_FLAG)
			c.fix_on = 1;
	}
	if (c.layout)
		return 1;

	if (tune_sb_features(sbi))
		return -1;

	/* precompute checksum seed for metadata */
	if (c.feature & cpu_to_le32(F2FS_FEATURE_INODE_CHKSUM))
		c.chksum_seed = f2fs_cal_crc32(~0, sb->uuid, sizeof(sb->uuid));

	sbi->total_valid_node_count = get_cp(valid_node_count);
	sbi->total_valid_inode_count = get_cp(valid_inode_count);
	sbi->user_block_count = get_cp(user_block_count);
	sbi->total_valid_block_count = get_cp(valid_block_count);
	sbi->last_valid_block_count = sbi->total_valid_block_count;
	sbi->alloc_valid_block_count = 0;

	if (early_build_segment_manager(sbi)) {
		ERR_MSG("early_build_segment_manager failed\n");
		return -1;
	}

	if (build_node_manager(sbi)) {
		ERR_MSG("build_node_manager failed\n");
		return -1;
	}

	if (record_fsync_data(sbi)) {
		ERR_MSG("record_fsync_data failed\n");
		return -1;
	}

	if (!f2fs_should_proceed(sb, get_cp(ckpt_flags)))
		return 1;

	if (late_build_segment_manager(sbi) < 0) {
		ERR_MSG("late_build_segment_manager failed\n");
		return -1;
	}

	if (f2fs_late_init_nid_bitmap(sbi)) {
		ERR_MSG("f2fs_late_init_nid_bitmap failed\n");
		return -1;
	}

	/* Check nat_bits */
	if (c.func == FSCK && is_set_ckpt_flags(cp, CP_NAT_BITS_FLAG)) {
		if (check_nat_bits(sbi, sb, cp) && c.fix_on)
			write_nat_bits(sbi, sb, cp, sbi->cur_cp);
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
	if (c.func == SLOAD || c.func == FSCK)
		free(nm_i->nid_bitmap);
	free(nm_i->nat_bitmap);
	free(sbi->nm_info);

	/* free sit_info */
	free(sit_i->bitmap);
	free(sit_i->sit_bitmap);
	free(sit_i->sentries);
	free(sm_i->sit_info);

	/* free sm_info */
	for (i = 0; i < NR_CURSEG_TYPE; i++)
		free(sm_i->curseg_array[i].sum_blk);

	free(sm_i->curseg_array);
	free(sbi->sm_info);

	free(sbi->ckpt);
	free(sbi->raw_super);
}

#ifdef WITH_ANDROID
int f2fs_sparse_initialize_meta(struct f2fs_sb_info *sbi)
{
	struct f2fs_super_block *sb = sbi->raw_super;
	uint32_t sit_seg_count, sit_size;
	uint32_t nat_seg_count, nat_size;
	uint64_t sit_seg_addr, nat_seg_addr, payload_addr;
	uint32_t seg_size = 1 << get_sb(log_blocks_per_seg);
	int ret;

	if (!c.sparse_mode)
		return 0;

	sit_seg_addr = get_sb(sit_blkaddr);
	sit_seg_count = get_sb(segment_count_sit);
	sit_size = sit_seg_count * seg_size;

	DBG(1, "\tSparse: filling sit area at block offset: 0x%08"PRIx64" len: %u\n",
							sit_seg_addr, sit_size);
	ret = dev_fill(NULL, sit_seg_addr * F2FS_BLKSIZE,
					sit_size * F2FS_BLKSIZE);
	if (ret) {
		MSG(1, "\tError: While zeroing out the sit area "
				"on disk!!!\n");
		return -1;
	}

	nat_seg_addr = get_sb(nat_blkaddr);
	nat_seg_count = get_sb(segment_count_nat);
	nat_size = nat_seg_count * seg_size;

	DBG(1, "\tSparse: filling nat area at block offset 0x%08"PRIx64" len: %u\n",
							nat_seg_addr, nat_size);
	ret = dev_fill(NULL, nat_seg_addr * F2FS_BLKSIZE,
					nat_size * F2FS_BLKSIZE);
	if (ret) {
		MSG(1, "\tError: While zeroing out the nat area "
				"on disk!!!\n");
		return -1;
	}

	payload_addr = get_sb(segment0_blkaddr) + 1;

	DBG(1, "\tSparse: filling bitmap area at block offset 0x%08"PRIx64" len: %u\n",
					payload_addr, get_sb(cp_payload));
	ret = dev_fill(NULL, payload_addr * F2FS_BLKSIZE,
					get_sb(cp_payload) * F2FS_BLKSIZE);
	if (ret) {
		MSG(1, "\tError: While zeroing out the nat/sit bitmap area "
				"on disk!!!\n");
		return -1;
	}

	payload_addr += seg_size;

	DBG(1, "\tSparse: filling bitmap area at block offset 0x%08"PRIx64" len: %u\n",
					payload_addr, get_sb(cp_payload));
	ret = dev_fill(NULL, payload_addr * F2FS_BLKSIZE,
					get_sb(cp_payload) * F2FS_BLKSIZE);
	if (ret) {
		MSG(1, "\tError: While zeroing out the nat/sit bitmap area "
				"on disk!!!\n");
		return -1;
	}
	return 0;
}
#else
int f2fs_sparse_initialize_meta(struct f2fs_sb_info *sbi) { return 0; }
#endif
