// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  fatent.c: exFAT FAT entry manager
 */

#include <asm/unaligned.h>

#include "exfat.h"
#include "core.h"

/* All buffer structures are protected w/ fsi->v_sem */

static inline bool is_reserved_clus(u32 clus)
{
	if (IS_CLUS_FREE(clus))
		return true;
	if (IS_CLUS_EOF(clus))
		return true;
	if (IS_CLUS_BAD(clus))
		return true;
	return false;
}

static inline bool is_valid_clus(FS_INFO_T *fsi, u32 clus)
{
	if (clus < CLUS_BASE || fsi->num_clusters <= clus)
		return false;
	return true;
}

s32 exfat_ent_get(struct super_block *sb, u32 loc, u32 *content)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	u32 off, _content;
	u64 sec;
	u8 *fat_sector;

	if (!is_valid_clus(fsi, loc)) {
		exfat_fs_error(sb, "invalid access to FAT (entry 0x%08x)", loc);
		return -EIO;
	}

	sec = fsi->FAT1_start_sector + (loc >> (sb->s_blocksize_bits-2));
	off = (loc << 2) & (u32)(sb->s_blocksize - 1);

	fat_sector = exfat_fcache_getblk(sb, sec);
	if (!fat_sector) {
		exfat_fs_error(sb, "failed to access to FAT "
				"(entry 0x%08x)", loc);
		return -EIO;
	}

	_content = le32_to_cpu(*(__le32 *)(&fat_sector[off]));

	/* remap reserved clusters to simplify code */
	if (_content >= CLUSTER_32(0xFFFFFFF8))
		_content = CLUS_EOF;

	*content = CLUSTER_32(_content);

	if (!is_reserved_clus(*content) && !is_valid_clus(fsi, *content)) {
		exfat_fs_error(sb, "invalid access to FAT (entry 0x%08x) "
			"bogus content (0x%08x)", loc, *content);
		return -EIO;
	}

	return 0;
}

s32 exfat_ent_set(struct super_block *sb, u32 loc, u32 content)
{
	u32 off;
	u64 sec;
	u8 *fat_sector;
	__le32 *fat_entry;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	sec = fsi->FAT1_start_sector + (loc >> (sb->s_blocksize_bits-2));
	off = (loc << 2) & (u32)(sb->s_blocksize - 1);

	fat_sector = exfat_fcache_getblk(sb, sec);
	if (!fat_sector)
		return -EIO;

	fat_entry = (__le32 *)&(fat_sector[off]);
	*fat_entry = cpu_to_le32(content);

	return exfat_fcache_modify(sb, sec);
}

s32 exfat_ent_get_safe(struct super_block *sb, u32 loc, u32 *content)
{
	s32 err = exfat_ent_get(sb, loc, content);

	if (err)
		return err;

	if (IS_CLUS_FREE(*content)) {
		exfat_fs_error(sb, "invalid access to FAT free cluster "
				"(entry 0x%08x)", loc);
		return -EIO;
	}

	if (IS_CLUS_BAD(*content)) {
		exfat_fs_error(sb, "invalid access to FAT bad cluster "
				"(entry 0x%08x)", loc);
		return -EIO;
	}

	return 0;
}
