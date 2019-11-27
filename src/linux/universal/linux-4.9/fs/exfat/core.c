// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  core.c: FAT-common core code
 */

#include <linux/version.h>
#include <linux/blkdev.h>
#include <linux/workqueue.h>
#include <linux/writeback.h>
#include <linux/kernel.h>
#include <linux/log2.h>

#include "exfat.h"
#include "core.h"
#include <asm/byteorder.h>
#include <asm/unaligned.h>

void exfat_set_sb_dirty(struct super_block *sb)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
	sb->s_dirt = 1;
#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0) */
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	sbi->s_dirt = 1;
	/* Insert work */
	spin_lock(&sbi->work_lock);
	if (!sbi->write_super_queued) {
		unsigned long delay;

		delay = msecs_to_jiffies(CONFIG_EXFAT_WRITE_SB_INTERVAL_CSECS * 10);
		queue_delayed_work(system_long_wq, &sbi->write_super_work, delay);
		sbi->write_super_queued = 1;
	}
	spin_unlock(&sbi->work_lock);
#endif
}

static s32 check_type_size(void)
{
	/* critical check for system requirement on size of DENTRY_T structure */
	if (sizeof(DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(DOS_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(EXT_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(FILE_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(STRM_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(NAME_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(BMAP_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(CASE_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	if (sizeof(VOLM_DENTRY_T) != DENTRY_SIZE)
		return -EINVAL;

	return 0;
}

static s32 __fs_set_vol_flags(struct super_block *sb, u16 new_flag, s32 always_sync)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	pbr64_t *bpb;
	s32 err;
	s32 sync = 0;

	/* flags are not changed */
	if (fsi->vol_flag == new_flag)
		return 0;

	fsi->vol_flag = new_flag;

	/* skip updating volume dirty flag,
	 * if this volume has been mounted with read-only
	 */
	if (EXFAT_IS_SB_RDONLY(sb))
		return 0;

	if (!fsi->pbr_bh) {
		err = exfat_read_sect(sb, 0, &(fsi->pbr_bh), 1);
		if (err) {
			EMSG("%s : failed to read boot sector\n", __func__);
			return err;
		}
	}

	bpb = (pbr64_t *)fsi->pbr_bh->b_data;
	bpb->bsx.vol_flags = cpu_to_le16(new_flag);

	if (always_sync)
		sync = 1;
	else if ((new_flag == VOL_DIRTY) && (!buffer_dirty(fsi->pbr_bh)))
		sync = 1;
	else
		sync = 0;

	err = exfat_write_sect(sb, 0, fsi->pbr_bh, sync);
	if (err)
		EMSG("%s : failed to modify volume flag\n", __func__);

	return err;
}

static s32 fs_set_vol_flags(struct super_block *sb, u16 new_flag)
{
	return __fs_set_vol_flags(sb, new_flag, 0);
}

s32 exfat_fscore_set_vol_flags(struct super_block *sb, u16 new_flag, s32 always_sync)
{
	return __fs_set_vol_flags(sb, new_flag, always_sync);
}

static s32 fs_sync(struct super_block *sb, s32 do_sync)
{
	s32 err;

	if (!do_sync)
		return 0;

	err = exfat_bdev_sync_all(sb);
	if (err)
		EMSG("%s : failed to sync. (err:%d)\n", __func__, err);

	return err;
}

/*
 *  Cluster Management Functions
 */

static s32 __clear_cluster(struct inode *inode, u32 clu)
{
	u64 s, n;
	struct super_block *sb = inode->i_sb;
	u32 sect_size = (u32)sb->s_blocksize;
	s32 ret = 0;
	struct buffer_head *tmp_bh = NULL;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (IS_CLUS_FREE(clu)) { /* FAT16 root_dir */
		s = fsi->root_start_sector;
		n = fsi->data_start_sector;
	} else {
		s = CLUS_TO_SECT(fsi, clu);
		n = s + fsi->sect_per_clus;
	}

	if (IS_DIRSYNC(inode)) {
		ret = exfat_write_msect_zero(sb, s, (u64)fsi->sect_per_clus);
		if (ret != -EAGAIN)
			return ret;
	}

	/* Trying buffered zero writes
	 * if it doesn't have DIRSYNC or exfat_write_msect_zero() returned -EAGAIN
	 */
	for ( ; s < n; s++) {
#if 0
		exfat_dcache_release(sb, s);
#endif
		ret = exfat_read_sect(sb, s, &tmp_bh, 0);
		if (ret)
			goto out;

		memset((u8 *)tmp_bh->b_data, 0x0, sect_size);
		ret = exfat_write_sect(sb, s, tmp_bh, 0);
		if (ret)
			goto out;
	}
out:
	brelse(tmp_bh);
	return ret;
}

static s32 __find_last_cluster(struct super_block *sb, CHAIN_T *p_chain, u32 *ret_clu)
{
	u32 clu, next;
	u32 count = 0;

	next = p_chain->dir;
	if (p_chain->flags == 0x03) {
		*ret_clu = next + p_chain->size - 1;
		return 0;
	}

	do {
		count++;
		clu = next;
		if (exfat_ent_get_safe(sb, clu, &next))
			return -EIO;
	} while (!IS_CLUS_EOF(next));

	if (p_chain->size != count) {
		exfat_fs_error(sb, "bogus directory size "
				"(clus : ondisk(%d) != counted(%d))",
				p_chain->size, count);
		exfat_debug_bug_on(1);
		return -EIO;
	}

	*ret_clu = clu;
	return 0;
}


static s32 __count_num_clusters(struct super_block *sb, CHAIN_T *p_chain, u32 *ret_count)
{
	u32 i, count;
	u32 clu;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (!p_chain->dir || IS_CLUS_EOF(p_chain->dir)) {
		*ret_count = 0;
		return 0;
	}

	if (p_chain->flags == 0x03) {
		*ret_count = p_chain->size;
		return 0;
	}

	clu = p_chain->dir;
	count = 0;
	for (i = CLUS_BASE; i < fsi->num_clusters; i++) {
		count++;
		if (exfat_ent_get_safe(sb, clu, &clu))
			return -EIO;
		if (IS_CLUS_EOF(clu))
			break;
	}

	*ret_count = count;
	return 0;
}

/*
 *  Upcase table Management Functions
 */
static void free_upcase_table(struct super_block *sb)
{
	u32 i;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	u16 **upcase_table;

	upcase_table = fsi->vol_utbl;
	for (i = 0 ; i < UTBL_COL_COUNT ; i++) {
		/* kfree(NULL) is safe */
		kfree(upcase_table[i]);
		upcase_table[i] = NULL;
	}

	/* kfree(NULL) is safe */
	kfree(fsi->vol_utbl);
	fsi->vol_utbl = NULL;
}

static s32 __load_upcase_table(struct super_block *sb, u64 sector, u64 num_sectors, u32 utbl_checksum)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	struct buffer_head *tmp_bh = NULL;
	u32 sect_size = (u32)sb->s_blocksize;
	s32 ret = -EIO;
	u32 i, j;

	u8 skip = false;
	u32 index = 0;
	u32 checksum = 0;
	u16 **upcase_table = kzalloc((UTBL_COL_COUNT * sizeof(u16 *)), GFP_KERNEL);

	if (!upcase_table)
		return -ENOMEM;
	/* thanks for kzalloc
	 * memset(upcase_table, 0, UTBL_COL_COUNT * sizeof(u16 *));
	 */

	fsi->vol_utbl = upcase_table;
	num_sectors += sector;

	while (sector < num_sectors) {
		ret = exfat_read_sect(sb, sector, &tmp_bh, 1);
		if (ret) {
			EMSG("%s: failed to read sector(0x%llx)\n",
						__func__, sector);
			goto error;
		}
		sector++;

		for (i = 0; i < sect_size && index <= 0xFFFF; i += 2) {
			/* FIXME : is __le16 ok? */
			//u16 uni = le16_to_cpu(((__le16*)(tmp_bh->b_data))[i]);
			u16 uni = get_unaligned_le16((u8 *)tmp_bh->b_data+i);

			checksum = ((checksum & 1) ? 0x80000000 : 0) +
				(checksum >> 1) + *(((u8 *)tmp_bh->b_data)+i);
			checksum = ((checksum & 1) ? 0x80000000 : 0) +
				(checksum >> 1) + *(((u8 *)tmp_bh->b_data)+(i+1));

			if (skip) {
				MMSG("skip from 0x%X to 0x%X(amount of 0x%X)\n",
					index, index+uni, uni);
				index += uni;
				skip = false;
			} else if (uni == index) {
				index++;
			} else if (uni == 0xFFFF) {
				skip = true;
			} else { /* uni != index , uni != 0xFFFF */
				u16 col_index = exfat_get_col_index(index);

				if (!upcase_table[col_index]) {
					upcase_table[col_index] =
						kmalloc((UTBL_ROW_COUNT * sizeof(u16)), GFP_KERNEL);
					if (!upcase_table[col_index]) {
						EMSG("failed to allocate memory"
							" for column 0x%X\n",
							col_index);
						ret = -ENOMEM;
						goto error;
					}

					for (j = 0; j < UTBL_ROW_COUNT; j++)
						upcase_table[col_index][j] = (col_index << LOW_INDEX_BIT) | j;
				}

				upcase_table[col_index][exfat_get_row_index(index)] = uni;
				index++;
			}
		}
	}
	if (index >= 0xFFFF && utbl_checksum == checksum) {
		DMSG("%s: load upcase table successfully"
			"(idx:0x%08x, utbl_chksum:0x%08x)\n",
			__func__, index, utbl_checksum);
		if (tmp_bh)
			brelse(tmp_bh);
		return 0;
	}

	EMSG("%s: failed to load upcase table"
		"(idx:0x%08x, chksum:0x%08x, utbl_chksum:0x%08x)\n",
		__func__, index, checksum, utbl_checksum);

	ret = -EINVAL;
error:
	if (tmp_bh)
		brelse(tmp_bh);
	free_upcase_table(sb);
	return ret;
}

static s32 __load_default_upcase_table(struct super_block *sb)
{
	s32 i, ret = -EIO;
	u32 j;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	u8 skip = false;
	u32 index = 0;
	u16 uni = 0;
	u16 **upcase_table;

	upcase_table = kmalloc((UTBL_COL_COUNT * sizeof(u16 *)), GFP_KERNEL);
	if (!upcase_table)
		return -ENOMEM;

	fsi->vol_utbl = upcase_table;
	memset(upcase_table, 0, UTBL_COL_COUNT * sizeof(u16 *));

	for (i = 0; index <= 0xFFFF && i < EXFAT_NUM_UPCASE*2; i += 2) {
		/* FIXME : is __le16 ok? */
		//uni = le16_to_cpu(((__le16*)uni_def_upcase)[i>>1]);
		uni = get_unaligned_le16((u8 *)uni_def_upcase+i);
		if (skip) {
			MMSG("skip from 0x%x ", index);
			index += uni;
			MMSG("to 0x%x (amount of 0x%x)\n", index, uni);
			skip = false;
		} else if (uni == index) {
			index++;
		} else if (uni == 0xFFFF) {
			skip = true;
		} else { /* uni != index , uni != 0xFFFF */
			u16 col_index = exfat_get_col_index(index);

			if (!upcase_table[col_index]) {
				upcase_table[col_index] = kmalloc((UTBL_ROW_COUNT * sizeof(u16)), GFP_KERNEL);
				if (!upcase_table[col_index]) {
					EMSG("failed to allocate memory for "
						"new column 0x%x\n", col_index);
					ret = -ENOMEM;
					goto error;
				}

				for (j = 0; j < UTBL_ROW_COUNT; j++)
					upcase_table[col_index][j] = (col_index << LOW_INDEX_BIT) | j;
			}

			upcase_table[col_index][exfat_get_row_index(index)] = uni;
			index++;
		}
	}

	if (index >= 0xFFFF)
		return 0;

error:
	/* FATAL error: default upcase table has error */
	free_upcase_table(sb);
	return ret;
}

static s32 load_upcase_table(struct super_block *sb)
{
	s32 i, ret;
	u32 tbl_clu, type;
	u64 sector, tbl_size, num_sectors;
	u8 blksize_bits = sb->s_blocksize_bits;
	CHAIN_T clu;
	CASE_DENTRY_T *ep;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	clu.dir = fsi->root_dir;
	clu.flags = 0x01;

	while (!IS_CLUS_EOF(clu.dir)) {
		for (i = 0; i < fsi->dentries_per_clu; i++) {
			ep = (CASE_DENTRY_T *) exfat_get_dentry_in_dir(sb, &clu, i, NULL);
			if (!ep)
				return -EIO;

			type = fsi->fs_func->get_entry_type((DENTRY_T *) ep);

			if (type == TYPE_UNUSED)
				break;
			if (type != TYPE_UPCASE)
				continue;

			tbl_clu  = le32_to_cpu(ep->start_clu);
			tbl_size = le64_to_cpu(ep->size);

			sector = CLUS_TO_SECT(fsi, tbl_clu);
			num_sectors = ((tbl_size - 1) >> blksize_bits) + 1;
			ret = __load_upcase_table(sb, sector, num_sectors,
						le32_to_cpu(ep->checksum));

			if (ret && (ret != -EIO))
				goto load_default;

			/* load successfully */
			return ret;
		}

		if (get_next_clus_safe(sb, &(clu.dir)))
			return -EIO;
	}

load_default:
	exfat_log_msg(sb, KERN_INFO, "trying to load default upcase table");
	/* load default upcase table */
	return __load_default_upcase_table(sb);
}


/*
 *  Directory Entry Management Functions
 */
s32 exfat_walk_fat_chain(struct super_block *sb, CHAIN_T *p_dir, u32 byte_offset, u32 *clu)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	u32 clu_offset;
	u32 cur_clu;

	clu_offset = byte_offset >> fsi->cluster_size_bits;
	cur_clu = p_dir->dir;

	if (p_dir->flags == 0x03) {
		cur_clu += clu_offset;
	} else {
		while (clu_offset > 0) {
			if (get_next_clus_safe(sb, &cur_clu))
				return -EIO;
			if (IS_CLUS_EOF(cur_clu)) {
				exfat_fs_error(sb, "invalid dentry access "
					"beyond EOF (clu : %u, eidx : %d)",
					p_dir->dir,
					byte_offset >> DENTRY_SIZE_BITS);
				return -EIO;
			}
			clu_offset--;
		}
	}

	if (clu)
		*clu = cur_clu;
	return 0;
}

static s32 find_location(struct super_block *sb, CHAIN_T *p_dir, s32 entry, u64 *sector, s32 *offset)
{
	s32 ret;
	u32 off, clu = 0;
	u32 blksize_mask = (u32)(sb->s_blocksize-1);
	u8 blksize_bits = sb->s_blocksize_bits;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	off = entry << DENTRY_SIZE_BITS;

	/* FAT16 root_dir */
	if (IS_CLUS_FREE(p_dir->dir)) {
		*offset = off & blksize_mask;
		*sector = off >> blksize_bits;
		*sector += fsi->root_start_sector;
		return 0;
	}

	ret = exfat_walk_fat_chain(sb, p_dir, off, &clu);
	if (ret)
		return ret;

	/* byte offset in cluster */
	off &= (fsi->cluster_size - 1);

	/* byte offset in sector    */
	*offset = off & blksize_mask;

	/* sector offset in cluster */
	*sector = off >> blksize_bits;
	*sector += CLUS_TO_SECT(fsi, clu);
	return 0;
}

DENTRY_T *exfat_get_dentry_in_dir(struct super_block *sb, CHAIN_T *p_dir, s32 entry, u64 *sector)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	u32 dentries_per_page = PAGE_SIZE >> DENTRY_SIZE_BITS;
	s32 off;
	u64 sec;
	u8 *buf;

	if (p_dir->dir == DIR_DELETED) {
		EMSG("%s : abnormal access to deleted dentry\n", __func__);
		BUG_ON(!fsi->prev_eio);
		return NULL;
	}

	if (find_location(sb, p_dir, entry, &sec, &off))
		return NULL;

	/* DIRECTORY READAHEAD :
	 * Try to read ahead per a page except root directory of fat12/16
	 */
	if ((!IS_CLUS_FREE(p_dir->dir)) &&
		!(entry & (dentries_per_page - 1)))
		exfat_dcache_readahead(sb, sec);

	buf = exfat_dcache_getblk(sb, sec);
	if (!buf)
		return NULL;

	if (sector)
		*sector = sec;
	return (DENTRY_T *)(buf + off);
}

/* used only in search empty_slot() */
#define CNT_UNUSED_NOHIT	(-1)
#define CNT_UNUSED_HIT		(-2)
/* search EMPTY CONTINUOUS "num_entries" entries */
static s32 search_empty_slot(struct super_block *sb, HINT_FEMP_T *hint_femp, CHAIN_T *p_dir, s32 num_entries)
{
	s32 i, dentry, num_empty = 0;
	s32 dentries_per_clu;
	u32 type;
	CHAIN_T clu;
	DENTRY_T *ep;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (IS_CLUS_FREE(p_dir->dir)) /* FAT16 root_dir */
		dentries_per_clu = fsi->dentries_in_root;
	else
		dentries_per_clu = fsi->dentries_per_clu;

	ASSERT(-1 <= hint_femp->eidx);

	if (hint_femp->eidx != -1) {
		clu.dir = hint_femp->cur.dir;
		clu.size = hint_femp->cur.size;
		clu.flags = hint_femp->cur.flags;

		dentry = hint_femp->eidx;

		if (num_entries <= hint_femp->count) {
			MMSG("%s: empty slot(HIT) - found "
				"(clu : 0x%08x eidx : %d)\n",
				__func__, hint_femp->cur.dir, hint_femp->eidx);
			hint_femp->eidx = -1;

			return dentry;
		}
		MMSG("%s: empty slot(HIT) - search from "
		       "(clu : 0x%08x eidx : %d)\n",
			__func__, hint_femp->cur.dir, hint_femp->eidx);
	} else {
		MMSG("%s: empty slot(MISS) - search from "
			"(clu:0x%08x eidx : 0)\n",
			__func__, p_dir->dir);

		clu.dir = p_dir->dir;
		clu.size = p_dir->size;
		clu.flags = p_dir->flags;

		dentry = 0;
	}

	while (!IS_CLUS_EOF(clu.dir)) {
		/* FAT16 root_dir */
		if (IS_CLUS_FREE(p_dir->dir))
			i = dentry % dentries_per_clu;
		else
			i = dentry & (dentries_per_clu-1);

		for ( ; i < dentries_per_clu; i++, dentry++) {
			ep = exfat_get_dentry_in_dir(sb, &clu, i, NULL);
			if (!ep)
				return -EIO;

			type = fsi->fs_func->get_entry_type(ep);

			if ((type == TYPE_UNUSED) || (type == TYPE_DELETED)) {
				num_empty++;
				if (hint_femp->eidx == -1) {
					hint_femp->eidx = dentry;
					hint_femp->count = CNT_UNUSED_NOHIT;

					hint_femp->cur.dir = clu.dir;
					hint_femp->cur.size = clu.size;
					hint_femp->cur.flags = clu.flags;
				}

				if ((type == TYPE_UNUSED) &&
					(hint_femp->count != CNT_UNUSED_HIT)) {
					hint_femp->count = CNT_UNUSED_HIT;
				}
			} else {
				if ((hint_femp->eidx != -1) &&
					(hint_femp->count == CNT_UNUSED_HIT)) {
					/* unused empty group means
					 * an empty group which includes
					 * unused dentry
					 */
					exfat_fs_error(sb,
						"found bogus dentry(%d) "
						"beyond unused empty group(%d) "
						"(start_clu : %u, cur_clu : %u)",
						dentry, hint_femp->eidx, p_dir->dir,
						clu.dir);
					return -EIO;
				}

				num_empty = 0;
				hint_femp->eidx = -1;
			}

			if (num_empty >= num_entries) {
				/* found and invalidate hint_femp */
				hint_femp->eidx = -1;

				return (dentry - (num_entries-1));
			}
		}

		if (IS_CLUS_FREE(p_dir->dir))
			break; /* FAT16 root_dir */

		if (clu.flags == 0x03) {
			if ((--clu.size) > 0)
				clu.dir++;
			else
				clu.dir = CLUS_EOF;
		} else {
			if (get_next_clus_safe(sb, &(clu.dir)))
				return -EIO;
		}
	}

	return -ENOSPC;
}

/*
 * find empty directory entry.
 * if there isn't any empty slot, expand cluster chain.
 */
static s32 find_empty_entry(struct inode *inode, CHAIN_T *p_dir, s32 num_entries)
{
	s32 dentry;
	u32 ret, last_clu;
	u64 sector;
	u64 size = 0;
	CHAIN_T clu;
	DENTRY_T *ep = NULL;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	HINT_FEMP_T hint_femp;

	hint_femp.eidx = -1;

	ASSERT(-1 <= fid->hint_femp.eidx);

	if (fid->hint_femp.eidx != -1) {
		memcpy(&hint_femp, &fid->hint_femp, sizeof(HINT_FEMP_T));
		fid->hint_femp.eidx = -1;
	}

	/* FAT16 root_dir */
	if (IS_CLUS_FREE(p_dir->dir))
		return search_empty_slot(sb, &hint_femp, p_dir, num_entries);

	while ((dentry = search_empty_slot(sb, &hint_femp, p_dir, num_entries)) < 0) {
		if (dentry == -EIO)
			break;

		if (fsi->fs_func->check_max_dentries(fid))
			return -ENOSPC;

		/* we trust p_dir->size regardless of FAT type */
		if (__find_last_cluster(sb, p_dir, &last_clu))
			return -EIO;

		/*
		 * Allocate new cluster to this directory
		 */
		clu.dir = last_clu + 1;
		clu.size = 0; /* UNUSED */
		clu.flags = p_dir->flags;

		/* (0) check if there are reserved clusters
		 * Refer to create_dir's comments
		 */
		if (!IS_CLUS_EOF(fsi->used_clusters) &&
			((fsi->used_clusters + fsi->reserved_clusters) >= (fsi->num_clusters - 2)))
			return -ENOSPC;

		/* (1) allocate a cluster */
		ret = fsi->fs_func->alloc_cluster(sb, 1, &clu, ALLOC_HOT);
		if (ret)
			return ret;

		if (__clear_cluster(inode, clu.dir))
			return -EIO;

		/* (2) append to the FAT chain */
		if (clu.flags != p_dir->flags) {
			/* no-fat-chain bit is disabled,
			 * so fat-chain should be synced with alloc-bmp
			 */
			exfat_chain_cont_cluster(sb, p_dir->dir, p_dir->size);
			p_dir->flags = 0x01;
			hint_femp.cur.flags = 0x01;
		}

		if (clu.flags == 0x01)
			if (exfat_ent_set(sb, last_clu, clu.dir))
				return -EIO;

		if (hint_femp.eidx == -1) {
			/* the special case that new dentry
			 * should be allocated from the start of new cluster
			 */
			hint_femp.eidx = (s32)(p_dir->size <<
				(fsi->cluster_size_bits - DENTRY_SIZE_BITS));
			hint_femp.count = fsi->dentries_per_clu;

			hint_femp.cur.dir = clu.dir;
			hint_femp.cur.size = 0;
			hint_femp.cur.flags = clu.flags;
		}
		hint_femp.cur.size++;
		p_dir->size++;
		size = (p_dir->size << fsi->cluster_size_bits);

		/* (3) update the directory entry */
		if (p_dir->dir != fsi->root_dir) {
			ep = exfat_get_dentry_in_dir(sb,
					&(fid->dir), fid->entry+1, &sector);
			if (!ep)
				return -EIO;
			fsi->fs_func->set_entry_size(ep, size);
			fsi->fs_func->set_entry_flag(ep, p_dir->flags);
			if (exfat_dcache_modify(sb, sector))
				return -EIO;

			if (exfat_update_dir_chksum(sb, &(fid->dir), fid->entry))
				return -EIO;
		}

		/* directory inode should be updated in here */
		i_size_write(inode, (loff_t)size);
		EXFAT_I(inode)->i_size_ondisk += fsi->cluster_size;
		EXFAT_I(inode)->i_size_aligned += fsi->cluster_size;
		EXFAT_I(inode)->fid.size = size;
		EXFAT_I(inode)->fid.flags = p_dir->flags;
		inode->i_blocks += 1 << (fsi->cluster_size_bits - sb->s_blocksize_bits);
	}

	return dentry;
}

#define EXFAT_MIN_SUBDIR	(2)

static s32 __count_dos_name_entries(struct super_block *sb, CHAIN_T *p_dir, u32 type, u32 *dotcnt)
{
	s32 i, count = 0, check_dot = 0;
	s32 dentries_per_clu;
	u32 entry_type;
	CHAIN_T clu;
	DENTRY_T *ep;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (IS_CLUS_FREE(p_dir->dir)) /* FAT16 root_dir */
		dentries_per_clu = fsi->dentries_in_root;
	else
		dentries_per_clu = fsi->dentries_per_clu;

	clu.dir = p_dir->dir;
	clu.size = p_dir->size;
	clu.flags = p_dir->flags;

	if (dotcnt)
		*dotcnt = 0;

	while (!IS_CLUS_EOF(clu.dir)) {
		for (i = 0; i < dentries_per_clu; i++) {
			ep = exfat_get_dentry_in_dir(sb, &clu, i, NULL);
			if (!ep)
				return -EIO;

			entry_type = fsi->fs_func->get_entry_type(ep);

			if (entry_type == TYPE_UNUSED)
				return count;
			if (!(type & TYPE_CRITICAL_PRI) && !(type & TYPE_BENIGN_PRI))
				continue;

			if ((type != TYPE_ALL) && (type != entry_type))
				continue;

			count++;
		}

		/* FAT16 root_dir */
		if (IS_CLUS_FREE(p_dir->dir))
			break;

		if (clu.flags == 0x03) {
			if ((--clu.size) > 0)
				clu.dir++;
			else
				clu.dir = CLUS_EOF;
		} else {
			if (get_next_clus_safe(sb, &(clu.dir)))
				return -EIO;
		}

		check_dot = 0;
	}

	return count;
}

s32 check_dir_empty(struct super_block *sb, CHAIN_T *p_dir)
{
	s32 i;
	s32 dentries_per_clu;
	u32 type;
	CHAIN_T clu;
	DENTRY_T *ep;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (IS_CLUS_FREE(p_dir->dir)) /* FAT16 root_dir */
		dentries_per_clu = fsi->dentries_in_root;
	else
		dentries_per_clu = fsi->dentries_per_clu;

	clu.dir = p_dir->dir;
	clu.size = p_dir->size;
	clu.flags = p_dir->flags;

	while (!IS_CLUS_EOF(clu.dir)) {
		for (i = 0; i < dentries_per_clu; i++) {
			ep = exfat_get_dentry_in_dir(sb, &clu, i, NULL);
			if (!ep)
				return -EIO;

			type = fsi->fs_func->get_entry_type(ep);

			if (type == TYPE_UNUSED)
				return 0;

			if ((type != TYPE_FILE) && (type != TYPE_DIR))
				continue;

			return -ENOTEMPTY;
		}

		/* FAT16 root_dir */
		if (IS_CLUS_FREE(p_dir->dir))
			return -ENOTEMPTY;

		if (clu.flags == 0x03) {
			if ((--clu.size) > 0)
				clu.dir++;
			else
				clu.dir = CLUS_EOF;
		} else {
			if (get_next_clus_safe(sb, &(clu.dir)))
				return -EIO;
		}
	}

	return 0;
}

/* input  : dir, uni_name
 * output : num_of_entry, dos_name(format : aaaaaa~1.bbb)
 */
static s32 get_num_entries_and_dos_name(struct super_block *sb, CHAIN_T *p_dir,
					UNI_NAME_T *p_uniname, s32 *entries,
					DOS_NAME_T *p_dosname, s32 lookup)
{
	s32 num_entries;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	/* Init null char. */
	p_dosname->name[0] = '\0';

	num_entries = fsi->fs_func->calc_num_entries(p_uniname);
	if (num_entries == 0)
		return -EINVAL;

	*entries = num_entries;
	return 0;
}

void exfat_get_uniname_from_dos_entry(struct super_block *sb, DOS_DENTRY_T *ep, UNI_NAME_T *p_uniname, u8 mode)
{
	DOS_NAME_T dos_name;

	if (mode == 0x0)
		dos_name.name_case = 0x0;
	else
		dos_name.name_case = ep->lcase;

	memcpy(dos_name.name, ep->name, DOS_NAME_LENGTH);
	exfat_nls_sfn_to_uni16s(sb, &dos_name, p_uniname);
}

/* returns the length of a struct qstr, ignoring trailing dots */
static inline unsigned int __striptail_len(unsigned int len, const char *name)
{
	while (len && name[len - 1] == '.')
		len--;
	return len;
}

/*
 * Name Resolution Functions :
 * Zero if it was successful; otherwise nonzero.
 */
static s32 __resolve_path(struct inode *inode, const u8 *path, CHAIN_T *p_dir, UNI_NAME_T *p_uniname, int lookup)
{
	s32 namelen;
	s32 lossy = NLS_NAME_NO_LOSSY;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);

	/* DOT and DOTDOT are handled by VFS layer */

	/* strip all trailing spaces */
	/* DO NOTHING : Is needed? */

	/* strip all trailing periods */
	namelen = __striptail_len(strlen(path), path);
	if (!namelen)
		return -ENOENT;

	/* the limitation of linux? */
	if (strlen(path) > (MAX_NAME_LENGTH * MAX_CHARSET_SIZE))
		return -ENAMETOOLONG;

	/*
	 * strip all leading spaces :
	 * "MS windows 7" supports leading spaces.
	 * So we should skip this preprocessing for compatibility.
	 */

	/* file name conversion :
	 * If lookup case, we allow bad-name for compatibility.
	 */
	namelen = exfat_nls_vfsname_to_uni16s(sb, path, namelen, p_uniname, &lossy);
	if (namelen < 0)
		return namelen; /* return error value */

	if ((lossy && !lookup) || !namelen)
		return -EINVAL;

	exfat_debug_bug_on(fid->size != i_size_read(inode));
//	fid->size = i_size_read(inode);

	p_dir->dir = fid->start_clu;
	p_dir->size = (u32)(fid->size >> fsi->cluster_size_bits);
	p_dir->flags = fid->flags;

	return 0;
}

static inline s32 resolve_path(struct inode *inode, const u8 *path, CHAIN_T *dir, UNI_NAME_T *uni)
{
	return __resolve_path(inode, path, dir, uni, 0);
}

static inline s32 resolve_path_for_lookup(struct inode *inode, const u8 *path, CHAIN_T *dir, UNI_NAME_T *uni)
{
	return __resolve_path(inode, path, dir, uni, 1);
}

static s32 create_dir(struct inode *inode, CHAIN_T *p_dir, UNI_NAME_T *p_uniname, FILE_ID_T *fid)
{
	s32 dentry, num_entries;
	u64 ret;
	u64 size;
	CHAIN_T clu;
	DOS_NAME_T dos_name;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	ret = get_num_entries_and_dos_name(sb, p_dir, p_uniname, &num_entries, &dos_name, 0);
	if (ret)
		return ret;

	/* find_empty_entry must be called before alloc_cluster */
	dentry = find_empty_entry(inode, p_dir, num_entries);
	if (dentry < 0)
		return dentry; /* -EIO or -ENOSPC */

	clu.dir = CLUS_EOF;
	clu.size = 0;
	clu.flags = 0x03;

	/* (0) Check if there are reserved clusters up to max. */
	if ((fsi->used_clusters + fsi->reserved_clusters) >= (fsi->num_clusters - CLUS_BASE))
		return -ENOSPC;

	/* (1) allocate a cluster */
	ret = fsi->fs_func->alloc_cluster(sb, 1, &clu, ALLOC_HOT);
	if (ret)
		return ret;

	ret = __clear_cluster(inode, clu.dir);
	if (ret)
		return ret;

	size = fsi->cluster_size;

	/* (2) update the directory entry */
	/* make sub-dir entry in parent directory */
	ret = fsi->fs_func->init_dir_entry(sb, p_dir, dentry, TYPE_DIR, clu.dir, size);
	if (ret)
		return ret;

	ret = fsi->fs_func->init_ext_entry(sb, p_dir, dentry, num_entries, p_uniname, &dos_name);
	if (ret)
		return ret;

	fid->dir.dir = p_dir->dir;
	fid->dir.size = p_dir->size;
	fid->dir.flags = p_dir->flags;
	fid->entry = dentry;

	fid->attr = ATTR_SUBDIR;
	fid->flags = 0x03;
	fid->size = size;
	fid->start_clu = clu.dir;

	fid->type = TYPE_DIR;
	fid->rwoffset = 0;
	fid->hint_bmap.off = CLUS_EOF;

	/* hint_stat will be used if this is directory. */
	fid->version = 0;
	fid->hint_stat.eidx = 0;
	fid->hint_stat.clu = fid->start_clu;
	fid->hint_femp.eidx = -1;

	return 0;
}

static s32 create_file(struct inode *inode, CHAIN_T *p_dir, UNI_NAME_T *p_uniname, u8 mode, FILE_ID_T *fid)
{
	s32 ret, dentry, num_entries;
	DOS_NAME_T dos_name;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	ret = get_num_entries_and_dos_name(sb, p_dir, p_uniname, &num_entries, &dos_name, 0);
	if (ret)
		return ret;

	/* find_empty_entry must be called before alloc_cluster() */
	dentry = find_empty_entry(inode, p_dir, num_entries);
	if (dentry < 0)
		return dentry; /* -EIO or -ENOSPC */

	/* (1) update the directory entry */
	/* fill the dos name directory entry information of the created file.
	 * the first cluster is not determined yet. (0)
	 */
	ret = fsi->fs_func->init_dir_entry(sb, p_dir, dentry, TYPE_FILE | mode, CLUS_FREE, 0);
	if (ret)
		return ret;

	ret = fsi->fs_func->init_ext_entry(sb, p_dir, dentry, num_entries, p_uniname, &dos_name);
	if (ret)
		return ret;

	fid->dir.dir = p_dir->dir;
	fid->dir.size = p_dir->size;
	fid->dir.flags = p_dir->flags;
	fid->entry = dentry;

	fid->attr = ATTR_ARCHIVE | mode;
	fid->flags = 0x03;
	fid->size = 0;
	fid->start_clu = CLUS_EOF;

	fid->type = TYPE_FILE;
	fid->rwoffset = 0;
	fid->hint_bmap.off = CLUS_EOF;

	/* hint_stat will be used if this is directory. */
	fid->version = 0;
	fid->hint_stat.eidx = 0;
	fid->hint_stat.clu = fid->start_clu;
	fid->hint_femp.eidx = -1;

	return 0;
}

static s32 remove_file(struct inode *inode, CHAIN_T *p_dir, s32 entry)
{
	s32 num_entries;
	u64 sector;
	DENTRY_T *ep;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	ep = exfat_get_dentry_in_dir(sb, p_dir, entry, &sector);
	if (!ep)
		return -EIO;

	exfat_dcache_lock(sb, sector);

	/* exfat_dcache_lock() before call count_ext_entries() */
	num_entries = fsi->fs_func->count_ext_entries(sb, p_dir, entry, ep);
	if (num_entries < 0) {
		exfat_dcache_unlock(sb, sector);
		return -EIO;
	}
	num_entries++;

	exfat_dcache_unlock(sb, sector);

	/* (1) update the directory entry */
	return fsi->fs_func->delete_dir_entry(sb, p_dir, entry, 0, num_entries);
}

static s32 rename_file(struct inode *inode, CHAIN_T *p_dir, s32 oldentry, UNI_NAME_T *p_uniname, FILE_ID_T *fid)
{
	s32 ret, newentry = -1, num_old_entries, num_new_entries;
	u64 sector_old, sector_new;
	DOS_NAME_T dos_name;
	DENTRY_T *epold, *epnew;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	epold = exfat_get_dentry_in_dir(sb, p_dir, oldentry, &sector_old);
	if (!epold)
		return -EIO;

	exfat_dcache_lock(sb, sector_old);

	/* exfat_dcache_lock() before call count_ext_entries() */
	num_old_entries = fsi->fs_func->count_ext_entries(sb, p_dir, oldentry, epold);
	if (num_old_entries < 0) {
		exfat_dcache_unlock(sb, sector_old);
		return -EIO;
	}
	num_old_entries++;

	ret = get_num_entries_and_dos_name(sb, p_dir, p_uniname, &num_new_entries, &dos_name, 0);
	if (ret) {
		exfat_dcache_unlock(sb, sector_old);
		return ret;
	}

	if (num_old_entries < num_new_entries) {
		newentry = find_empty_entry(inode, p_dir, num_new_entries);
		if (newentry < 0) {
			exfat_dcache_unlock(sb, sector_old);
			return newentry; /* -EIO or -ENOSPC */
		}

		epnew = exfat_get_dentry_in_dir(sb, p_dir, newentry, &sector_new);
		if (!epnew) {
			exfat_dcache_unlock(sb, sector_old);
			return -EIO;
		}

		memcpy((void *) epnew, (void *) epold, DENTRY_SIZE);
		if (fsi->fs_func->get_entry_type(epnew) == TYPE_FILE) {
			fsi->fs_func->set_entry_attr(epnew, fsi->fs_func->get_entry_attr(epnew) | ATTR_ARCHIVE);
			fid->attr |= ATTR_ARCHIVE;
		}
		exfat_dcache_modify(sb, sector_new);
		exfat_dcache_unlock(sb, sector_old);

		epold = exfat_get_dentry_in_dir(sb, p_dir, oldentry+1, &sector_old);
		exfat_dcache_lock(sb, sector_old);
		epnew = exfat_get_dentry_in_dir(sb, p_dir, newentry+1, &sector_new);

		if (!epold || !epnew) {
			exfat_dcache_unlock(sb, sector_old);
			return -EIO;
		}

		memcpy((void *) epnew, (void *) epold, DENTRY_SIZE);
		exfat_dcache_modify(sb, sector_new);
		exfat_dcache_unlock(sb, sector_old);

		ret = fsi->fs_func->init_ext_entry(sb, p_dir, newentry, num_new_entries, p_uniname, &dos_name);
		if (ret)
			return ret;

		fsi->fs_func->delete_dir_entry(sb, p_dir, oldentry, 0, num_old_entries);
		fid->entry = newentry;
	} else {
		if (fsi->fs_func->get_entry_type(epold) == TYPE_FILE) {
			fsi->fs_func->set_entry_attr(epold, fsi->fs_func->get_entry_attr(epold) | ATTR_ARCHIVE);
			fid->attr |= ATTR_ARCHIVE;
		}
		exfat_dcache_modify(sb, sector_old);
		exfat_dcache_unlock(sb, sector_old);

		ret = fsi->fs_func->init_ext_entry(sb, p_dir, oldentry, num_new_entries, p_uniname, &dos_name);
		if (ret)
			return ret;

		fsi->fs_func->delete_dir_entry(sb, p_dir, oldentry, num_new_entries, num_old_entries);
	}

	return 0;
}

static s32 move_file(struct inode *inode, CHAIN_T *p_olddir, s32 oldentry,
		CHAIN_T *p_newdir, UNI_NAME_T *p_uniname, FILE_ID_T *fid)
{
	s32 ret, newentry, num_new_entries, num_old_entries;
	u64 sector_mov, sector_new;
	DOS_NAME_T dos_name;
	DENTRY_T *epmov, *epnew;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	epmov = exfat_get_dentry_in_dir(sb, p_olddir, oldentry, &sector_mov);
	if (!epmov)
		return -EIO;

	/* check if the source and target directory is the same */
	if (fsi->fs_func->get_entry_type(epmov) == TYPE_DIR &&
		fsi->fs_func->get_entry_clu0(epmov) == p_newdir->dir)
		return -EINVAL;

	exfat_dcache_lock(sb, sector_mov);

	/* exfat_dcache_lock() before call count_ext_entries() */
	num_old_entries = fsi->fs_func->count_ext_entries(sb, p_olddir, oldentry, epmov);
	if (num_old_entries < 0) {
		exfat_dcache_unlock(sb, sector_mov);
		return -EIO;
	}
	num_old_entries++;

	ret = get_num_entries_and_dos_name(sb, p_newdir, p_uniname, &num_new_entries, &dos_name, 0);
	if (ret) {
		exfat_dcache_unlock(sb, sector_mov);
		return ret;
	}

	newentry = find_empty_entry(inode, p_newdir, num_new_entries);
	if (newentry < 0) {
		exfat_dcache_unlock(sb, sector_mov);
		return newentry; /* -EIO or -ENOSPC */
	}

	epnew = exfat_get_dentry_in_dir(sb, p_newdir, newentry, &sector_new);
	if (!epnew) {
		exfat_dcache_unlock(sb, sector_mov);
		return -EIO;
	}

	memcpy((void *) epnew, (void *) epmov, DENTRY_SIZE);
	if (fsi->fs_func->get_entry_type(epnew) == TYPE_FILE) {
		fsi->fs_func->set_entry_attr(epnew, fsi->fs_func->get_entry_attr(epnew) | ATTR_ARCHIVE);
		fid->attr |= ATTR_ARCHIVE;
	}
	exfat_dcache_modify(sb, sector_new);
	exfat_dcache_unlock(sb, sector_mov);

	epmov = exfat_get_dentry_in_dir(sb, p_olddir, oldentry+1, &sector_mov);
	exfat_dcache_lock(sb, sector_mov);
	epnew = exfat_get_dentry_in_dir(sb, p_newdir, newentry+1, &sector_new);
	if (!epmov || !epnew) {
		exfat_dcache_unlock(sb, sector_mov);
		return -EIO;
	}

	memcpy((void *) epnew, (void *) epmov, DENTRY_SIZE);
	exfat_dcache_modify(sb, sector_new);
	exfat_dcache_unlock(sb, sector_mov);

	ret = fsi->fs_func->init_ext_entry(sb, p_newdir, newentry, num_new_entries, p_uniname, &dos_name);
	if (ret)
		return ret;

	fsi->fs_func->delete_dir_entry(sb, p_olddir, oldentry, 0, num_old_entries);

	fid->dir.dir = p_newdir->dir;
	fid->dir.size = p_newdir->size;
	fid->dir.flags = p_newdir->flags;

	fid->entry = newentry;

	return 0;
}

/* roll back to the initial state of the file system */
s32 exfat_fscore_init(void)
{
	s32 ret;

	ret = check_type_size();
	if (ret)
		return ret;

	return exfat_extent_cache_init();
}

/* make free all memory-alloced global buffers */
s32 exfat_fscore_shutdown(void)
{
	exfat_extent_cache_shutdown();
	return 0;
}

/* check device is ejected */
s32 exfat_fscore_check_bdi_valid(struct super_block *sb)
{
	return exfat_bdev_check_bdi_valid(sb);
}

static bool is_exfat(pbr_t *pbr)
{
	int i = 53;

	do {
		if (pbr->bpb.f64.res_zero[i-1])
			break;
	} while (--i);
	return i ? false : true;
}

inline pbr_t *read_pbr_with_logical_sector(struct super_block *sb, struct buffer_head **prev_bh)
{
	pbr_t *p_pbr = (pbr_t *) (*prev_bh)->b_data;
	u16 logical_sect = 0;

	if (is_exfat(p_pbr))
		logical_sect = 1 << p_pbr->bsx.f64.sect_size_bits;
	else
		logical_sect = get_unaligned_le16(&p_pbr->bpb.f16.sect_size);

	/* is x a power of 2?
	 * (x) != 0 && (((x) & ((x) - 1)) == 0)
	 */
	if (!is_power_of_2(logical_sect)
			|| (logical_sect < 512)
			|| (logical_sect > 4096)) {
		exfat_log_msg(sb, KERN_ERR, "bogus logical sector size %u",
						logical_sect);
		return NULL;
	}

	if (logical_sect < sb->s_blocksize) {
		exfat_log_msg(sb, KERN_ERR,
			"logical sector size too small for device"
			" (logical sector size = %u)", logical_sect);
		return NULL;
	}

	if (logical_sect > sb->s_blocksize) {
		struct buffer_head *bh = NULL;

		__brelse(*prev_bh);
		*prev_bh = NULL;

		if (!sb_set_blocksize(sb, logical_sect)) {
			exfat_log_msg(sb, KERN_ERR,
				"unable to set blocksize %u", logical_sect);
			return NULL;
		}
		bh = sb_bread(sb, 0);
		if (!bh) {
			exfat_log_msg(sb, KERN_ERR,
				"unable to read boot sector "
				"(logical sector size = %lu)", sb->s_blocksize);
			return NULL;
		}

		*prev_bh = bh;
		p_pbr = (pbr_t *) bh->b_data;
	}

	exfat_log_msg(sb, KERN_INFO,
		"set logical sector size  : %lu", sb->s_blocksize);

	return p_pbr;
}

/* mount the file system volume */
s32 exfat_fscore_mount(struct super_block *sb)
{
	s32 ret;
	pbr_t *p_pbr;
	struct buffer_head *tmp_bh = NULL;
	struct gendisk *disk = sb->s_bdev->bd_disk;
	struct hd_struct *part = sb->s_bdev->bd_part;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	/* initialize previous I/O error */
	fsi->prev_eio = 0;

	/* open the block device */
	if (exfat_bdev_open_dev(sb))
		return -EIO;

	/* set block size to read super block */
	sb_min_blocksize(sb, 512);

	/* read boot sector */
	ret = exfat_read_sect(sb, 0, &tmp_bh, 1);
	if (ret) {
		exfat_log_msg(sb, KERN_ERR, "unable to read boot sector");
		ret = -EIO;
		goto bd_close;
	}

	/* PRB is read */
	p_pbr = (pbr_t *) tmp_bh->b_data;

	/* check the validity of PBR */
	if (le16_to_cpu((p_pbr->signature)) != PBR_SIGNATURE) {
		exfat_log_msg(sb, KERN_ERR, "invalid boot record signature");
		brelse(tmp_bh);
		ret = -EINVAL;
		goto bd_close;
	}

	/* check logical sector size */
	p_pbr = read_pbr_with_logical_sector(sb, &tmp_bh);
	if (!p_pbr) {
		brelse(tmp_bh);
		ret = -EIO;
		goto bd_close;
	}

	/* fill fs_struct */
	if (!is_exfat(p_pbr)) {
		ret = -EINVAL;
		goto free_bh;
	}

	/* set maximum file size for exFAT */
	sb->s_maxbytes = 0x7fffffffffffffffLL;
	ret = mount_exfat(sb, p_pbr);

free_bh:
	brelse(tmp_bh);
	if (ret) {
		exfat_log_msg(sb, KERN_ERR, "failed to mount fs-core");
		goto bd_close;
	}

	/* warn misaligned data data start sector must be a multiple of clu_size */
	exfat_log_msg(sb, KERN_INFO,
		"(bps : %lu, spc : %u, data start : %llu, %s)",
		sb->s_blocksize, fsi->sect_per_clus, fsi->data_start_sector,
		(fsi->data_start_sector & (fsi->sect_per_clus - 1)) ?
		"misaligned" : "aligned");

	exfat_log_msg(sb, KERN_INFO,
		"detected volume size     : %llu KB (disk : %llu KB, "
		"part : %llu KB)",
		(fsi->num_sectors * (sb->s_blocksize >> SECTOR_SIZE_BITS)) >> 1,
		disk ? (u64)((disk->part0.nr_sects) >> 1) : 0,
		part ? (u64)((part->nr_sects) >> 1) : 0);

	ret = load_upcase_table(sb);
	if (ret) {
		exfat_log_msg(sb, KERN_ERR, "failed to load upcase table");
		goto bd_close;
	}

	/* allocate-bitmap is only for exFAT */
	ret = exfat_load_alloc_bmp(sb);
	if (ret) {
		exfat_log_msg(sb, KERN_ERR, "failed to load alloc-bitmap");
		goto free_upcase;
	}

	if (fsi->used_clusters == UINT_MAX) {
		ret = fsi->fs_func->count_used_clusters(sb, &fsi->used_clusters);
		if (ret) {
			exfat_log_msg(sb, KERN_ERR, "failed to scan clusters");
			goto exfat_free_alloc_bmp;
		}
	}

	return 0;
exfat_free_alloc_bmp:
	exfat_free_alloc_bmp(sb);
free_upcase:
	free_upcase_table(sb);
bd_close:
	exfat_bdev_close_dev(sb);
	return ret;
}

/* umount the file system volume */
s32 exfat_fscore_umount(struct super_block *sb)
{
	s32 ret = 0;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (fs_sync(sb, 0))
		ret = -EIO;

	if (fs_set_vol_flags(sb, VOL_CLEAN))
		ret = -EIO;

	free_upcase_table(sb);

	exfat_free_alloc_bmp(sb);

	if (exfat_fcache_release_all(sb))
		ret = -EIO;

	if (exfat_dcache_release_all(sb))
		ret = -EIO;

	if (fsi->prev_eio)
		ret = -EIO;
	/* close the block device */
	exfat_bdev_close_dev(sb);
	return ret;
}

/* get the information of a file system volume */
s32 exfat_fscore_statfs(struct super_block *sb, VOL_INFO_T *info)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (fsi->used_clusters == UINT_MAX) {
		if (fsi->fs_func->count_used_clusters(sb, &fsi->used_clusters))
			return -EIO;
	}

	info->ClusterSize = fsi->cluster_size;
	info->NumClusters = fsi->num_clusters - 2; /* clu 0 & 1 */
	info->UsedClusters = fsi->used_clusters + fsi->reserved_clusters;
	info->FreeClusters = info->NumClusters - info->UsedClusters;

	return 0;
}

/* synchronize all file system volumes */
s32 exfat_fscore_sync_fs(struct super_block *sb, s32 do_sync)
{
	/* synchronize the file system */
	if (fs_sync(sb, do_sync))
		return -EIO;

	if (fs_set_vol_flags(sb, VOL_CLEAN))
		return -EIO;

	return 0;
}

/* stat allocation unit of a file system volume */
u32 exfat_fscore_get_au_stat(struct super_block *sb, s32 mode)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (fsi->fs_func->get_au_stat)
		return fsi->fs_func->get_au_stat(sb, mode);

	/* No error, just returns 0 */
	return 0;
}

/* lookup a file */
s32 exfat_fscore_lookup(struct inode *inode, u8 *path, FILE_ID_T *fid)
{
	s32 ret, dentry, num_entries;
	CHAIN_T dir;
	UNI_NAME_T uni_name;
	DOS_NAME_T dos_name;
	DENTRY_T *ep, *ep2;
	ENTRY_SET_CACHE_T *es = NULL;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *dir_fid = &(EXFAT_I(inode)->fid);

	/* check the validity of directory name in the given pathname */
	ret = resolve_path_for_lookup(inode, path, &dir, &uni_name);
	if (ret)
		return ret;

	ret = get_num_entries_and_dos_name(sb, &dir, &uni_name, &num_entries, &dos_name, 1);
	if (ret)
		return ret;

	/* check the validation of hint_stat and initialize it if required */
	if (dir_fid->version != (u32)(GET_IVERSION(inode) & 0xffffffff)) {
		dir_fid->hint_stat.clu = dir.dir;
		dir_fid->hint_stat.eidx = 0;
		dir_fid->version = (u32)(GET_IVERSION(inode) & 0xffffffff);
		dir_fid->hint_femp.eidx = -1;
	}

	/* search the file name for directories */
	dentry = fsi->fs_func->find_dir_entry(sb, dir_fid, &dir, &uni_name,
				num_entries, &dos_name, TYPE_ALL);

	if ((dentry < 0) && (dentry != -EEXIST))
		return dentry; /* -error value */

	fid->dir.dir = dir.dir;
	fid->dir.size = dir.size;
	fid->dir.flags = dir.flags;
	fid->entry = dentry;

	/* root directory itself */
	if (unlikely(dentry == -EEXIST)) {
		fid->type = TYPE_DIR;
		fid->rwoffset = 0;
		fid->hint_bmap.off = CLUS_EOF;

		fid->attr = ATTR_SUBDIR;
		fid->flags = 0x01;
		fid->size = 0;
		fid->start_clu = fsi->root_dir;
	} else {
		es = exfat_get_dentry_set_in_dir(sb, &dir, dentry, ES_2_ENTRIES, &ep);
		if (!es)
			return -EIO;
		ep2 = ep+1;

		fid->type = fsi->fs_func->get_entry_type(ep);
		fid->rwoffset = 0;
		fid->hint_bmap.off = CLUS_EOF;
		fid->attr = fsi->fs_func->get_entry_attr(ep);

		fid->size = fsi->fs_func->get_entry_size(ep2);
		if ((fid->type == TYPE_FILE) && (fid->size == 0)) {
			fid->flags = 0x03;
			fid->start_clu = CLUS_EOF;
		} else {
			fid->flags = fsi->fs_func->get_entry_flag(ep2);
			fid->start_clu = fsi->fs_func->get_entry_clu0(ep2);
		}

		/* FOR GRACEFUL ERROR HANDLING */
		if (IS_CLUS_FREE(fid->start_clu)) {
			exfat_fs_error(sb,
				"non-zero size file starts with zero cluster "
				"(size : %llu, p_dir : %u, entry : 0x%08x)",
				fid->size, fid->dir.dir, fid->entry);
			exfat_debug_bug_on(1);
			return -EIO;
		}

		exfat_release_dentry_set(es);
	}

	/* hint_stat will be used if this is directory. */
	fid->version = 0;
	fid->hint_stat.eidx = 0;
	fid->hint_stat.clu = fid->start_clu;
	fid->hint_femp.eidx = -1;

	return 0;
}

/* create a file */
s32 exfat_fscore_create(struct inode *inode, u8 *path, u8 mode, FILE_ID_T *fid)
{
	s32 ret/*, dentry*/;
	CHAIN_T dir;
	UNI_NAME_T uni_name;
	struct super_block *sb = inode->i_sb;

	/* check the validity of directory name in the given pathname */
	ret = resolve_path(inode, path, &dir, &uni_name);
	if (ret)
		return ret;

	fs_set_vol_flags(sb, VOL_DIRTY);

	/* create a new file */
	ret = create_file(inode, &dir, &uni_name, mode, fid);

	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);

	return ret;
}

/* read data from a opened file */
s32 exfat_fscore_read_link(struct inode *inode, FILE_ID_T *fid, void *buffer, u64 count, u64 *rcount)
{
	s32 ret = 0;
	s32 offset, sec_offset;
	u32 clu_offset;
	u32 clu;
	u64 logsector, oneblkread, read_bytes;
	struct buffer_head *tmp_bh = NULL;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	/* check if the given file ID is opened */
	if (fid->type != TYPE_FILE)
		return -EPERM;

	if (fid->rwoffset > fid->size)
		fid->rwoffset = fid->size;

	if (count > (fid->size - fid->rwoffset))
		count = fid->size - fid->rwoffset;

	if (count == 0) {
		if (rcount)
			*rcount = 0;
		return 0;
	}

	read_bytes = 0;

	while (count > 0) {
		clu_offset = fid->rwoffset >> fsi->cluster_size_bits;
		clu = fid->start_clu;

		if (fid->flags == 0x03) {
			clu += clu_offset;
		} else {
			/* hint information */
			if ((clu_offset > 0) &&
				((fid->hint_bmap.off != CLUS_EOF) && (fid->hint_bmap.off > 0)) &&
				(clu_offset >= fid->hint_bmap.off)) {
				clu_offset -= fid->hint_bmap.off;
				clu = fid->hint_bmap.clu;
			}

			while (clu_offset > 0) {
				ret = get_next_clus_safe(sb, &clu);
				if (ret)
					goto err_out;

				clu_offset--;
			}
		}

		/* hint information */
		fid->hint_bmap.off = fid->rwoffset >> fsi->cluster_size_bits;
		fid->hint_bmap.clu = clu;

		offset = (s32)(fid->rwoffset & (fsi->cluster_size - 1)); /* byte offset in cluster   */
		sec_offset = offset >> sb->s_blocksize_bits;            /* sector offset in cluster */
		offset &= (sb->s_blocksize - 1);                         /* byte offset in sector    */

		logsector = CLUS_TO_SECT(fsi, clu) + sec_offset;

		oneblkread = (u64)(sb->s_blocksize - offset);
		if (oneblkread > count)
			oneblkread = count;

		if ((offset == 0) && (oneblkread == sb->s_blocksize)) {
			ret = exfat_read_sect(sb, logsector, &tmp_bh, 1);
			if (ret)
				goto err_out;
			memcpy(((s8 *) buffer)+read_bytes, ((s8 *) tmp_bh->b_data), (s32) oneblkread);
		} else {
			ret = exfat_read_sect(sb, logsector, &tmp_bh, 1);
			if (ret)
				goto err_out;
			memcpy(((s8 *) buffer)+read_bytes, ((s8 *) tmp_bh->b_data)+offset, (s32) oneblkread);
		}
		count -= oneblkread;
		read_bytes += oneblkread;
		fid->rwoffset += oneblkread;
	}

err_out:
	brelse(tmp_bh);

	/* set the size of read bytes */
	if (rcount != NULL)
		*rcount = read_bytes;

	return ret;
}

/* write data into a opened file */
s32 exfat_fscore_write_link(struct inode *inode, FILE_ID_T *fid, void *buffer, u64 count, u64 *wcount)
{
	s32 ret = 0;
	s32 modified = false, offset, sec_offset;
	u32 clu_offset, num_clusters, num_alloc;
	u32 clu, last_clu;
	u64 logsector, oneblkwrite, write_bytes;
	CHAIN_T new_clu;
	TIMESTAMP_T tm;
	DENTRY_T *ep, *ep2;
	ENTRY_SET_CACHE_T *es = NULL;
	struct buffer_head *tmp_bh = NULL;
	struct super_block *sb = inode->i_sb;
	u32 blksize = (u32)sb->s_blocksize;
	u32 blksize_mask = (u32)(sb->s_blocksize-1);
	u8 blksize_bits = sb->s_blocksize_bits;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	/* check if the given file ID is opened */
	if (fid->type != TYPE_FILE)
		return -EPERM;

	if (fid->rwoffset > fid->size)
		fid->rwoffset = fid->size;

	if (count == 0) {
		if (wcount)
			*wcount = 0;
		return 0;
	}

	fs_set_vol_flags(sb, VOL_DIRTY);

	if (fid->size == 0)
		num_clusters = 0;
	else
		num_clusters = ((fid->size-1) >> fsi->cluster_size_bits) + 1;

	write_bytes = 0;

	while (count > 0) {
		clu_offset = (fid->rwoffset >> fsi->cluster_size_bits);
		clu = last_clu = fid->start_clu;

		if (fid->flags == 0x03) {
			if ((clu_offset > 0) && (!IS_CLUS_EOF(clu))) {
				last_clu += clu_offset - 1;

				if (clu_offset == num_clusters)
					clu = CLUS_EOF;
				else
					clu += clu_offset;
			}
		} else {
			/* hint information */
			if ((clu_offset > 0) &&
				((fid->hint_bmap.off != CLUS_EOF) && (fid->hint_bmap.off > 0)) &&
				(clu_offset >= fid->hint_bmap.off)) {
				clu_offset -= fid->hint_bmap.off;
				clu = fid->hint_bmap.clu;
			}

			while ((clu_offset > 0) && (!IS_CLUS_EOF(clu))) {
				last_clu = clu;
				ret = get_next_clus_safe(sb, &clu);
				if (ret)
					goto err_out;

				clu_offset--;
			}
		}

		if (IS_CLUS_EOF(clu)) {
			num_alloc = ((count-1) >> fsi->cluster_size_bits) + 1;
			new_clu.dir = IS_CLUS_EOF(last_clu) ? CLUS_EOF : last_clu+1;
			new_clu.size = 0;
			new_clu.flags = fid->flags;

			/* (1) allocate a chain of clusters */
			ret = fsi->fs_func->alloc_cluster(sb, num_alloc, &new_clu, ALLOC_COLD);
			if (ret)
				goto err_out;

			/* (2) append to the FAT chain */
			if (IS_CLUS_EOF(last_clu)) {
				if (new_clu.flags == 0x01)
					fid->flags = 0x01;
				fid->start_clu = new_clu.dir;
				modified = true;
			} else {
				if (new_clu.flags != fid->flags) {
					/* no-fat-chain bit is disabled,
					 * so fat-chain should be synced with
					 * alloc-bmp
					 */
					exfat_chain_cont_cluster(sb, fid->start_clu, num_clusters);
					fid->flags = 0x01;
					modified = true;
				}
				if (new_clu.flags == 0x01) {
					ret = exfat_ent_set(sb, last_clu, new_clu.dir);
					if (ret)
						goto err_out;
				}
			}

			num_clusters += num_alloc;
			clu = new_clu.dir;
		}

		/* hint information */
		fid->hint_bmap.off = fid->rwoffset >> fsi->cluster_size_bits;
		fid->hint_bmap.clu = clu;

		/* byte offset in cluster   */
		offset = (s32)(fid->rwoffset & (fsi->cluster_size-1));
		/* sector offset in cluster */
		sec_offset = offset >> blksize_bits;
		/* byte offset in sector    */
		offset &= blksize_mask;
		logsector = CLUS_TO_SECT(fsi, clu) + sec_offset;

		oneblkwrite = (u64)(blksize - offset);
		if (oneblkwrite > count)
			oneblkwrite = count;

		if ((offset == 0) && (oneblkwrite == blksize)) {
			ret = exfat_read_sect(sb, logsector, &tmp_bh, 0);
			if (ret)
				goto err_out;

			memcpy(((s8 *)tmp_bh->b_data),
				((s8 *)buffer)+write_bytes,
				(s32)oneblkwrite);

			ret = exfat_write_sect(sb, logsector, tmp_bh, 0);
			if (ret) {
				brelse(tmp_bh);
				goto err_out;
			}
		} else {
			if ((offset > 0) || ((fid->rwoffset+oneblkwrite) < fid->size)) {
				ret = exfat_read_sect(sb, logsector, &tmp_bh, 1);
				if (ret)
					goto err_out;
			} else {
				ret = exfat_read_sect(sb, logsector, &tmp_bh, 0);
				if (ret)
					goto err_out;
			}

			memcpy(((s8 *) tmp_bh->b_data)+offset, ((s8 *) buffer)+write_bytes, (s32) oneblkwrite);
			ret = exfat_write_sect(sb, logsector, tmp_bh, 0);
			if (ret) {
				brelse(tmp_bh);
				goto err_out;
			}
		}

		count -= oneblkwrite;
		write_bytes += oneblkwrite;
		fid->rwoffset += oneblkwrite;

		fid->attr |= ATTR_ARCHIVE;

		if (fid->size < fid->rwoffset) {
			fid->size = fid->rwoffset;
			modified = true;
		}
	}

	brelse(tmp_bh);

	/* (3) update the direcoty entry */
	/* get_entry_(set_)in_dir shoulb be check DIR_DELETED flag. */
	es = exfat_get_dentry_set_in_dir(sb, &(fid->dir), fid->entry, ES_ALL_ENTRIES, &ep);
	if (!es) {
		ret = -EIO;
		goto err_out;
	}
	ep2 = ep+1;

	fsi->fs_func->set_entry_time(ep, exfat_tm_now(EXFAT_SB(sb), &tm), TM_MODIFY);
	fsi->fs_func->set_entry_attr(ep, fid->attr);

	if (modified) {
		if (fsi->fs_func->get_entry_flag(ep2) != fid->flags)
			fsi->fs_func->set_entry_flag(ep2, fid->flags);

		if (fsi->fs_func->get_entry_size(ep2) != fid->size)
			fsi->fs_func->set_entry_size(ep2, fid->size);

		if (fsi->fs_func->get_entry_clu0(ep2) != fid->start_clu)
			fsi->fs_func->set_entry_clu0(ep2, fid->start_clu);
	}

	if (exfat_update_dir_chksum_with_entry_set(sb, es)) {
		ret = -EIO;
		goto err_out;
	}
	exfat_release_dentry_set(es);

	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);

err_out:
	/* set the size of written bytes */
	if (wcount)
		*wcount = write_bytes;

	return ret;
}

/* resize the file length */
s32 exfat_fscore_truncate(struct inode *inode, u64 old_size, u64 new_size)
{
	u32 num_clusters_new, num_clusters_da, num_clusters_phys;
	u32 last_clu = CLUS_FREE;
	CHAIN_T clu;
	TIMESTAMP_T tm;
	DENTRY_T *ep, *ep2;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	ENTRY_SET_CACHE_T *es = NULL;
	s32 evict = (fid->dir.dir == DIR_DELETED) ? 1 : 0;

	/* check if the given file ID is opened */
	if ((fid->type != TYPE_FILE) && (fid->type != TYPE_DIR))
		return -EPERM;

	/* TO CHECK inode type and size */
	MMSG("%s: inode(%p) type(%s) size:%lld->%lld\n", __func__, inode,
		(fid->type == TYPE_FILE) ? "file" : "dir", old_size, new_size);

	/* XXX : This is for debugging. */

	/* It can be when write failed */
#if 0
	if (fid->size != old_size) {
		DMSG("%s: inode(%p) size-mismatch(old:%lld != fid:%lld)\n",
				__func__, inode, old_size, fid->size);
		WARN_ON(1);
	}
#endif
	/*
	 * There is no lock to protect fid->size.
	 * So, we should get old_size and use it.
	 */
	if (old_size <= new_size)
		return 0;

	fs_set_vol_flags(sb, VOL_DIRTY);

	/* Reserved count update */
	#define num_clusters(v) ((v) ? (u32)(((v) - 1) >> fsi->cluster_size_bits) + 1 : 0)
	num_clusters_da = num_clusters(EXFAT_I(inode)->i_size_aligned);
	num_clusters_new = num_clusters(i_size_read(inode));
	num_clusters_phys = num_clusters(EXFAT_I(inode)->i_size_ondisk);

	/* num_clusters(i_size_old) should be equal to num_clusters_da */
	BUG_ON((num_clusters(old_size)) != (num_clusters(EXFAT_I(inode)->i_size_aligned)));

	/* for debugging (FIXME: is okay on no-da case?) */
	BUG_ON(num_clusters_da < num_clusters_phys);

	if ((num_clusters_da != num_clusters_phys) &&
			(num_clusters_new < num_clusters_da)) {
		/* Decrement reserved clusters
		 * n_reserved = num_clusters_da - max(new,phys)
		 */
		int n_reserved = (num_clusters_new > num_clusters_phys) ?
				(num_clusters_da - num_clusters_new) :
				(num_clusters_da - num_clusters_phys);

		fsi->reserved_clusters -= n_reserved;
		BUG_ON(fsi->reserved_clusters < 0);
	}

	clu.dir = fid->start_clu;
	/* In no-da case, num_clusters_phys is equal to below value
	 * clu.size = (u32)((old_size-1) >> fsi->cluster_size_bits) + 1;
	 */
	clu.size = num_clusters_phys;
	clu.flags = fid->flags;

	if (new_size > 0) {
		/* Truncate FAT chain num_clusters after the first cluster
		 * num_clusters = min(new, phys);
		 */
		u32 num_clusters = (num_clusters_new < num_clusters_phys) ?
					num_clusters_new : num_clusters_phys;

		/* Follow FAT chain
		 * (defensive coding - works fine even with corrupted FAT table
		 */
		if (clu.flags == 0x03) {
			clu.dir += num_clusters;
			clu.size -= num_clusters;
#if 0
		/* exfat_extent_get_clus can`t know last_cluster
		 * when find target cluster in cache.
		 */
		} else if (fid->type == TYPE_FILE) {
			u32 fclus = 0;
			s32 err = exfat_extent_get_clus(inode, num_clusters,
					&fclus, &(clu.dir), &last_clu, 0);
			if (err)
				return -EIO;
			ASSERT(fclus == num_clusters);

			if ((num_clusters > 1) && (last_clu == fid->start_clu)) {
				u32 fclus_tmp = 0;
				u32 temp = 0;

				err = exfat_extent_get_clus(inode, num_clusters - 1,
						&fclus_tmp, &last_clu, &temp, 0);
				if (err)
					return -EIO;
				ASSERT(fclus_tmp == (num_clusters - 1));
			}

			num_clusters -= fclus;
			clu.size -= fclus;
#endif
		} else {
			while (num_clusters > 0) {
				last_clu = clu.dir;
				if (get_next_clus_safe(sb, &(clu.dir)))
					return -EIO;

				num_clusters--;
				clu.size--;
			}
		}

		/* Optimization avialable: */
#if 0
		if (num_clusters_new < num_clusters) {
			< loop >
		} else {
			// num_clusters_new >= num_clusters_phys
			// FAT truncation is not necessary

			clu.dir = CLUS_EOF;
			clu.size = 0;
		}
#endif
	} else if (new_size == 0) {
		fid->flags = 0x03;
		fid->start_clu = CLUS_EOF;
	}
	fid->size = new_size;

	if (fid->type == TYPE_FILE)
		fid->attr |= ATTR_ARCHIVE;

	/*
	 * clu.dir: free from
	 * clu.size: # of clusters to free (exFAT, 0x03 only), no fat_free if 0
	 * clu.flags: fid->flags (exFAT only)
	 */

	/* (1) update the directory entry */
	if (!evict) {
		es = exfat_get_dentry_set_in_dir(sb, &(fid->dir), fid->entry, ES_ALL_ENTRIES, &ep);
		if (!es)
			return -EIO;
		ep2 = ep+1;

		fsi->fs_func->set_entry_time(ep, exfat_tm_now(EXFAT_SB(sb), &tm), TM_MODIFY);
		fsi->fs_func->set_entry_attr(ep, fid->attr);

		/* File size should be zero if there is no cluster allocated */
		if (IS_CLUS_EOF(fid->start_clu))
			fsi->fs_func->set_entry_size(ep2, 0);
		else
			fsi->fs_func->set_entry_size(ep2, new_size);

		if (new_size == 0) {
			/* Any directory can not be truncated to zero */
			BUG_ON(fid->type != TYPE_FILE);

			fsi->fs_func->set_entry_flag(ep2, 0x01);
			fsi->fs_func->set_entry_clu0(ep2, CLUS_FREE);
		}

		if (exfat_update_dir_chksum_with_entry_set(sb, es))
			return -EIO;
		exfat_release_dentry_set(es);
	}

	/* (2) cut off from the FAT chain */
	if ((fid->flags == 0x01) &&
		(!IS_CLUS_FREE(last_clu)) && (!IS_CLUS_EOF(last_clu))) {
		if (exfat_ent_set(sb, last_clu, CLUS_EOF))
			return -EIO;
	}

	/* (3) invalidate cache and free the clusters */
	/* clear extent cache */
	exfat_extent_cache_inval_inode(inode);

	/* hint information */
	fid->hint_bmap.off = CLUS_EOF;
	fid->hint_bmap.clu = CLUS_EOF;
	if (fid->rwoffset > fid->size)
		fid->rwoffset = fid->size;

	/* hint_stat will be used if this is directory. */
	fid->hint_stat.eidx = 0;
	fid->hint_stat.clu = fid->start_clu;
	fid->hint_femp.eidx = -1;

	/* free the clusters */
	if (fsi->fs_func->free_cluster(sb, &clu, evict))
		return -EIO;

	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);

	return 0;
}

static void update_parent_info(FILE_ID_T *fid, struct inode *parent_inode)
{
	FS_INFO_T *fsi = &(EXFAT_SB(parent_inode->i_sb)->fsi);
	FILE_ID_T *parent_fid = &(EXFAT_I(parent_inode)->fid);

	 /*
	  * the problem that FILE_ID_T caches wrong parent info.
	  *
	  * because of flag-mismatch of fid->dir,
	  * there is abnormal traversing cluster chain.
	  */
	if (unlikely((parent_fid->flags != fid->dir.flags)
		|| (parent_fid->size != (fid->dir.size<<fsi->cluster_size_bits))
		|| (parent_fid->start_clu != fid->dir.dir))) {

		fid->dir.dir = parent_fid->start_clu;
		fid->dir.flags = parent_fid->flags;
		fid->dir.size = ((parent_fid->size + (fsi->cluster_size-1))
						>> fsi->cluster_size_bits);
	}
}

/* rename or move a old file into a new file */
s32 exfat_fscore_rename(struct inode *old_parent_inode, FILE_ID_T *fid,
		struct inode *new_parent_inode, struct dentry *new_dentry)
{
	s32 ret;
	s32 dentry;
	CHAIN_T olddir, newdir;
	CHAIN_T *p_dir = NULL;
	UNI_NAME_T uni_name;
	DENTRY_T *ep;
	struct super_block *sb = old_parent_inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	u8 *new_path = (u8 *) new_dentry->d_name.name;
	struct inode *new_inode = new_dentry->d_inode;
	int num_entries;
	FILE_ID_T *new_fid = NULL;
	u32 new_entry_type = TYPE_UNUSED;
	s32 new_entry = 0;

	/* check the validity of pointer parameters */
	if ((new_path == NULL) || (strlen(new_path) == 0))
		return -EINVAL;

	if (fid->dir.dir == DIR_DELETED) {
		EMSG("%s : abnormal access to deleted source dentry\n", __func__);
		return -ENOENT;
	}

	/* patch 1.2.4 : the problem that FILE_ID_T caches wrong parent info. */
	update_parent_info(fid, old_parent_inode);

	olddir.dir = fid->dir.dir;
	olddir.size = fid->dir.size;
	olddir.flags = fid->dir.flags;

	dentry = fid->entry;

	ep = exfat_get_dentry_in_dir(sb, &olddir, dentry, NULL);
	if (!ep)
		return -EIO;

	if (fsi->fs_func->get_entry_attr(ep) & ATTR_READONLY)
		return -EPERM;

	/* check whether new dir is existing directory and empty */
	if (new_inode) {
		ret = -EIO;
		new_fid = &EXFAT_I(new_inode)->fid;

		if (new_fid->dir.dir == DIR_DELETED) {
			EMSG("%s : abnormal access to deleted target dentry\n", __func__);
			goto out;
		}

		/* patch 1.2.4 :
		 * the problem that FILE_ID_T caches wrong parent info.
		 *
		 * FIXME : is needed?
		 */
		update_parent_info(new_fid, new_parent_inode);

		p_dir = &(new_fid->dir);
		new_entry = new_fid->entry;
		ep = exfat_get_dentry_in_dir(sb, p_dir, new_entry, NULL);
		if (!ep)
			goto out;

		new_entry_type = fsi->fs_func->get_entry_type(ep);

		/* if new_inode exists, update fid */
		new_fid->size = i_size_read(new_inode);

		if (new_entry_type == TYPE_DIR) {
			CHAIN_T new_clu;

			new_clu.dir = new_fid->start_clu;
			new_clu.size = ((new_fid->size-1) >> fsi->cluster_size_bits) + 1;
			new_clu.flags = new_fid->flags;

			ret = check_dir_empty(sb, &new_clu);
			if (ret)
				return ret;
		}
	}

	/* check the validity of directory name in the given new pathname */
	ret = resolve_path(new_parent_inode, new_path, &newdir, &uni_name);
	if (ret)
		return ret;

	fs_set_vol_flags(sb, VOL_DIRTY);

	if (olddir.dir == newdir.dir)
		ret = rename_file(new_parent_inode, &olddir, dentry, &uni_name, fid);
	else
		ret = move_file(new_parent_inode, &olddir, dentry, &newdir, &uni_name, fid);

	if ((!ret) && new_inode) {
		/* delete entries of new_dir */
		ep = exfat_get_dentry_in_dir(sb, p_dir, new_entry, NULL);
		if (!ep) {
			ret = -EIO;
			goto del_out;
		}

		num_entries = fsi->fs_func->count_ext_entries(sb, p_dir, new_entry, ep);
		if (num_entries < 0) {
			ret = -EIO;
			goto del_out;
		}


		if (fsi->fs_func->delete_dir_entry(sb, p_dir, new_entry, 0, num_entries+1)) {
			ret = -EIO;
			goto del_out;
		}

		/* Free the clusters if new_inode is a dir(as if exfat_fscore_rmdir) */
		if (new_entry_type == TYPE_DIR) {
			/* new_fid, new_clu_to_free */
			CHAIN_T new_clu_to_free;

			new_clu_to_free.dir = new_fid->start_clu;
			new_clu_to_free.size = ((new_fid->size-1) >> fsi->cluster_size_bits) + 1;
			new_clu_to_free.flags = new_fid->flags;

			if (fsi->fs_func->free_cluster(sb, &new_clu_to_free, 1)) {
				/* just set I/O error only */
				ret = -EIO;
			}

			new_fid->size = 0;
			new_fid->start_clu = CLUS_EOF;
			new_fid->flags = 0x03;
		}
del_out:
		/* Update new_inode fid
		 * Prevent syncing removed new_inode
		 * (new_fid is already initialized above code ("if (new_inode)")
		 */
		new_fid->dir.dir = DIR_DELETED;
	}
out:
	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);

	return ret;
}

/* remove a file */
s32 exfat_fscore_remove(struct inode *inode, FILE_ID_T *fid)
{
	s32 ret;
	s32 dentry;
	CHAIN_T dir, clu_to_free;
	DENTRY_T *ep;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	dir.dir = fid->dir.dir;
	dir.size = fid->dir.size;
	dir.flags = fid->dir.flags;

	dentry = fid->entry;

	if (fid->dir.dir == DIR_DELETED) {
		EMSG("%s : abnormal access to deleted dentry\n", __func__);
		return -ENOENT;
	}

	ep = exfat_get_dentry_in_dir(sb, &dir, dentry, NULL);
	if (!ep)
		return -EIO;

	if (fsi->fs_func->get_entry_attr(ep) & ATTR_READONLY)
		return -EPERM;

	fs_set_vol_flags(sb, VOL_DIRTY);

	/* (1) update the directory entry */
	ret = remove_file(inode, &dir, dentry);
	if (ret)
		goto out;

	clu_to_free.dir = fid->start_clu;
	clu_to_free.size = ((fid->size-1) >> fsi->cluster_size_bits) + 1;
	clu_to_free.flags = fid->flags;

	/* (2) invalidate extent cache and free the clusters
	 */
	/* clear extent cache */
	exfat_extent_cache_inval_inode(inode);
	ret = fsi->fs_func->free_cluster(sb, &clu_to_free, 0);
	/* WARN : DO NOT RETURN ERROR IN HERE */

	/* (3) update FILE_ID_T  */
	fid->size = 0;
	fid->start_clu = CLUS_EOF;
	fid->flags = 0x03;
	fid->dir.dir = DIR_DELETED;

	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);
out:
	return ret;
}


/*
 * Get the information of a given file
 * REMARK : This function does not need any file name on linux
 *
 * info.Size means the value saved on disk.
 * But root directory doesn`t have real dentry,
 * so the size of root directory returns calculated one exceptively.
 */
s32 exfat_fscore_read_inode(struct inode *inode, DIR_ENTRY_T *info)
{
	s32 count;
	CHAIN_T dir;
	TIMESTAMP_T tm;
	DENTRY_T *ep, *ep2;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	ENTRY_SET_CACHE_T *es = NULL;
	u8 is_dir = (fid->type == TYPE_DIR) ? 1 : 0;

	exfat_extent_cache_init_inode(inode);

	/* if root directory */
	if (is_dir && (fid->dir.dir == fsi->root_dir) && (fid->entry == -1)) {
		info->Attr = ATTR_SUBDIR;
		memset((s8 *) &info->CreateTimestamp, 0, sizeof(DATE_TIME_T));
		memset((s8 *) &info->ModifyTimestamp, 0, sizeof(DATE_TIME_T));
		memset((s8 *) &info->AccessTimestamp, 0, sizeof(DATE_TIME_T));
		//strcpy(info->NameBuf.sfn, ".");
		//strcpy(info->NameBuf.lfn, ".");

		dir.dir = fsi->root_dir;
		dir.flags = 0x01;
		dir.size = 0; /* UNUSED */

		/* FAT16 root_dir */
		if (IS_CLUS_FREE(fsi->root_dir)) {
			info->Size = fsi->dentries_in_root << DENTRY_SIZE_BITS;
		} else {
			u32 num_clu;

			if (__count_num_clusters(sb, &dir, &num_clu))
				return -EIO;
			info->Size = (u64)num_clu << fsi->cluster_size_bits;
		}

		count = __count_dos_name_entries(sb, &dir, TYPE_DIR, NULL);
		if (count < 0)
			return -EIO;
		info->NumSubdirs = count;

		return 0;
	}

	/* get the directory entry of given file or directory */
	/* es should be released */
	es = exfat_get_dentry_set_in_dir(sb, &(fid->dir), fid->entry, ES_2_ENTRIES, &ep);
	if (!es)
		return -EIO;
	ep2 = ep+1;

	/* set FILE_INFO structure using the acquired DENTRY_T */
	info->Attr = fsi->fs_func->get_entry_attr(ep);

	fsi->fs_func->get_entry_time(ep, &tm, TM_CREATE);
	info->CreateTimestamp.Year = tm.year;
	info->CreateTimestamp.Month = tm.mon;
	info->CreateTimestamp.Day = tm.day;
	info->CreateTimestamp.Hour = tm.hour;
	info->CreateTimestamp.Minute = tm.min;
	info->CreateTimestamp.Second = tm.sec;
	info->CreateTimestamp.MilliSecond = 0;

	fsi->fs_func->get_entry_time(ep, &tm, TM_MODIFY);
	info->ModifyTimestamp.Year = tm.year;
	info->ModifyTimestamp.Month = tm.mon;
	info->ModifyTimestamp.Day = tm.day;
	info->ModifyTimestamp.Hour = tm.hour;
	info->ModifyTimestamp.Minute = tm.min;
	info->ModifyTimestamp.Second = tm.sec;
	info->ModifyTimestamp.MilliSecond = 0;

	memset((s8 *) &info->AccessTimestamp, 0, sizeof(DATE_TIME_T));

	info->NumSubdirs = 0;
	info->Size = fsi->fs_func->get_entry_size(ep2);

	exfat_release_dentry_set(es);

	if (is_dir) {
		u32 dotcnt = 0;

		dir.dir = fid->start_clu;
		dir.flags = fid->flags;
		dir.size = fid->size >> fsi->cluster_size_bits;
		/*
		 * NOTE :
		 * If "dir.flags" has 0x01, "dir.size" is meaningless.
		 */
#if 0
		if (info->Size == 0) {
			s32 num_clu;

			if (__count_num_clusters(sb, &dir, &num_clu))
				return -EIO;
			info->Size = (u64)num_clu << fsi->cluster_size_bits;
		}
#endif
		count = __count_dos_name_entries(sb, &dir, TYPE_DIR, &dotcnt);
		if (count < 0)
			return -EIO;

		count += EXFAT_MIN_SUBDIR;
		info->NumSubdirs = count;
	}

	return 0;
}

/* set the information of a given file
 * REMARK : This function does not need any file name on linux
 */
s32 exfat_fscore_write_inode(struct inode *inode, DIR_ENTRY_T *info, s32 sync)
{
	s32 ret = -EIO;
	TIMESTAMP_T tm;
	DENTRY_T *ep, *ep2;
	ENTRY_SET_CACHE_T *es = NULL;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	u8 is_dir = (fid->type == TYPE_DIR) ? 1 : 0;
	u64 on_disk_size;

	/* SKIP WRITING INODE :
	 * if the indoe is already unlinked,
	 * there is no need for updating inode
	 */
	if (fid->dir.dir == DIR_DELETED)
		return 0;

	if (is_dir && (fid->dir.dir == fsi->root_dir) && (fid->entry == -1))
		return 0;

	fs_set_vol_flags(sb, VOL_DIRTY);

	/* get the directory entry of given file or directory */
	es = exfat_get_dentry_set_in_dir(sb, &(fid->dir), fid->entry, ES_ALL_ENTRIES, &ep);
	if (!es)
		return -EIO;
	ep2 = ep+1;

	fsi->fs_func->set_entry_attr(ep, info->Attr);

	/* set FILE_INFO structure using the acquired DENTRY_T */
	tm.sec  = info->CreateTimestamp.Second;
	tm.min  = info->CreateTimestamp.Minute;
	tm.hour = info->CreateTimestamp.Hour;
	tm.day  = info->CreateTimestamp.Day;
	tm.mon  = info->CreateTimestamp.Month;
	tm.year = info->CreateTimestamp.Year;
	fsi->fs_func->set_entry_time(ep, &tm, TM_CREATE);

	tm.sec  = info->ModifyTimestamp.Second;
	tm.min  = info->ModifyTimestamp.Minute;
	tm.hour = info->ModifyTimestamp.Hour;
	tm.day  = info->ModifyTimestamp.Day;
	tm.mon  = info->ModifyTimestamp.Month;
	tm.year = info->ModifyTimestamp.Year;
	fsi->fs_func->set_entry_time(ep, &tm, TM_MODIFY);

	/* File size should be zero if there is no cluster allocated */
	on_disk_size = info->Size;

	if (IS_CLUS_EOF(fid->start_clu))
		on_disk_size = 0;

	fsi->fs_func->set_entry_size(ep2, on_disk_size);

	ret = exfat_update_dir_chksum_with_entry_set(sb, es);
	exfat_release_dentry_set(es);

	fs_sync(sb, sync);
	/* Comment below code to prevent super block update frequently */
	//fs_set_vol_flags(sb, VOL_CLEAN);

	return ret;
}


/*
 * Input: inode, (logical) clu_offset, target allocation area
 * Output: errcode, cluster number
 * *clu = (~0), if it's unable to allocate a new cluster
 */
s32 exfat_fscore_map_clus(struct inode *inode, u32 clu_offset, u32 *clu, int dest)
{
	s32 ret, modified = false;
	u32 last_clu;
	CHAIN_T new_clu;
	DENTRY_T *ep;
	ENTRY_SET_CACHE_T *es = NULL;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	u32 local_clu_offset = clu_offset;
	s32 reserved_clusters = fsi->reserved_clusters;
	u32 num_to_be_allocated = 0, num_clusters = 0;

	fid->rwoffset = (s64)(clu_offset) << fsi->cluster_size_bits;

	if (EXFAT_I(inode)->i_size_ondisk > 0)
		num_clusters = (u32)((EXFAT_I(inode)->i_size_ondisk-1) >> fsi->cluster_size_bits) + 1;

	if (clu_offset >= num_clusters)
		num_to_be_allocated = clu_offset - num_clusters + 1;

	if ((dest == ALLOC_NOWHERE) && (num_to_be_allocated > 0)) {
		*clu = CLUS_EOF;
		return 0;
	}

	/* check always request cluster is 1 */
	//ASSERT(num_to_be_allocated == 1);

	*clu = last_clu = fid->start_clu;

	/* XXX: Defensive code needed.
	 * what if i_size_ondisk != # of allocated clusters
	 */
	if (fid->flags == 0x03) {
		if ((clu_offset > 0) && (!IS_CLUS_EOF(*clu))) {
			last_clu += clu_offset - 1;

			if (clu_offset == num_clusters)
				*clu = CLUS_EOF;
			else
				*clu += clu_offset;
		}
	} else if (fid->type == TYPE_FILE) {
		u32 fclus = 0;
		s32 err = exfat_extent_get_clus(inode, clu_offset,
				&fclus, clu, &last_clu, 1);
		if (err)
			return -EIO;

		clu_offset -= fclus;
	} else {
		/* hint information */
		if ((clu_offset > 0) &&
			((fid->hint_bmap.off != CLUS_EOF) && (fid->hint_bmap.off > 0)) &&
			(clu_offset >= fid->hint_bmap.off)) {
			clu_offset -= fid->hint_bmap.off;
			/* hint_bmap.clu should be valid */
			ASSERT(fid->hint_bmap.clu >= 2);
			*clu = fid->hint_bmap.clu;
		}

		while ((clu_offset > 0) && (!IS_CLUS_EOF(*clu))) {
			last_clu = *clu;
			if (get_next_clus_safe(sb, clu))
				return -EIO;
			clu_offset--;
		}
	}

	if (IS_CLUS_EOF(*clu)) {
		fs_set_vol_flags(sb, VOL_DIRTY);

		new_clu.dir = (IS_CLUS_EOF(last_clu)) ? CLUS_EOF : last_clu + 1;
		new_clu.size = 0;
		new_clu.flags = fid->flags;

		/* (1) allocate a cluster */
		if (num_to_be_allocated < 1) {
			/* Broken FAT (i_sze > allocated FAT) */
			EMSG("%s: invalid fat chain : inode(%p) "
				"num_to_be_allocated(%d) "
				"i_size_ondisk(%lld) fid->flags(%02x) "
				"fid->start(%08x) fid->hint_off(%u) "
				"fid->hint_clu(%u) fid->rwoffset(%llu) "
				"modified_clu_off(%d) last_clu(%08x) "
				"new_clu(%08x)", __func__, inode,
				num_to_be_allocated,
				(EXFAT_I(inode)->i_size_ondisk),
				fid->flags, fid->start_clu,
				fid->hint_bmap.off, fid->hint_bmap.clu,
				fid->rwoffset, clu_offset,
				last_clu, new_clu.dir);
			exfat_fs_error(sb, "broken FAT chain.");
			return -EIO;
		}

		ret = fsi->fs_func->alloc_cluster(sb, num_to_be_allocated, &new_clu, ALLOC_COLD);
		if (ret)
			return ret;

		if (IS_CLUS_EOF(new_clu.dir) || IS_CLUS_FREE(new_clu.dir)) {
			exfat_fs_error(sb, "bogus cluster new allocated"
				"(last_clu : %u, new_clu : %u)",
				last_clu, new_clu.dir);
			ASSERT(0);
			return -EIO;
		}

		/* (2) append to the FAT chain */
		if (IS_CLUS_EOF(last_clu)) {
			if (new_clu.flags == 0x01)
				fid->flags = 0x01;
			fid->start_clu = new_clu.dir;
			modified = true;
		} else {
			if (new_clu.flags != fid->flags) {
				/* no-fat-chain bit is disabled,
				 * so fat-chain should be synced with alloc-bmp
				 */
				exfat_chain_cont_cluster(sb, fid->start_clu, num_clusters);
				fid->flags = 0x01;
				modified = true;
			}
			if (new_clu.flags == 0x01)
				if (exfat_ent_set(sb, last_clu, new_clu.dir))
					return -EIO;
		}

		num_clusters += num_to_be_allocated;
		*clu = new_clu.dir;

		if (fid->dir.dir != DIR_DELETED) {
			es = exfat_get_dentry_set_in_dir(sb, &(fid->dir), fid->entry, ES_ALL_ENTRIES, &ep);
			if (!es)
				return -EIO;
			/* get stream entry */
			ep++;

			/* (3) update directory entry */
			if (modified) {
				if (fsi->fs_func->get_entry_flag(ep) != fid->flags)
					fsi->fs_func->set_entry_flag(ep, fid->flags);

				if (fsi->fs_func->get_entry_clu0(ep) != fid->start_clu)
					fsi->fs_func->set_entry_clu0(ep, fid->start_clu);

				fsi->fs_func->set_entry_size(ep, fid->size);
			}

			if (exfat_update_dir_chksum_with_entry_set(sb, es))
				return -EIO;
			exfat_release_dentry_set(es);
		}

		/* add number of new blocks to inode */
		inode->i_blocks += num_to_be_allocated << (fsi->cluster_size_bits - sb->s_blocksize_bits);
#if 0
		fs_sync(sb, 0);
		fs_set_vol_flags(sb, VOL_CLEAN);
#endif
		/* (4) Move *clu pointer along FAT chains (hole care)
		 * because the caller of this function expect *clu to be the last cluster.
		 * This only works when num_to_be_allocated >= 2,
		 * *clu = (the first cluster of the allocated chain) => (the last cluster of ...)
		 */
		if (fid->flags == 0x03) {
			*clu += num_to_be_allocated - 1;
		} else {
			while (num_to_be_allocated > 1) {
				if (get_next_clus_safe(sb, clu))
					return -EIO;
				num_to_be_allocated--;
			}
		}

	}

	/* update reserved_clusters */
	fsi->reserved_clusters = reserved_clusters;

	/* hint information */
	fid->hint_bmap.off = local_clu_offset;
	fid->hint_bmap.clu = *clu;

	return 0;
}

/* remove an entry, BUT don't truncate */
s32 exfat_fscore_unlink(struct inode *inode, FILE_ID_T *fid)
{
	s32 dentry;
	CHAIN_T dir;
	DENTRY_T *ep;
	struct super_block *sb = inode->i_sb;

	dir.dir = fid->dir.dir;
	dir.size = fid->dir.size;
	dir.flags = fid->dir.flags;

	dentry = fid->entry;

	if (fid->dir.dir == DIR_DELETED) {
		EMSG("%s : abnormal access to deleted dentry\n", __func__);
		return -ENOENT;
	}

	ep = exfat_get_dentry_in_dir(sb, &dir, dentry, NULL);
	if (!ep)
		return -EIO;

	if (EXFAT_SB(sb)->fsi.fs_func->get_entry_attr(ep) & ATTR_READONLY)
		return -EPERM;

	fs_set_vol_flags(sb, VOL_DIRTY);

	/* (1) update the directory entry */
	if (remove_file(inode, &dir, dentry))
		return -EIO;

	/* This doesn't modify fid */
	fid->dir.dir = DIR_DELETED;

	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);

	return 0;
}

/* create a directory */
s32 exfat_fscore_mkdir(struct inode *inode, u8 *path, FILE_ID_T *fid)
{
	s32 ret/*, dentry*/;
	CHAIN_T dir;
	UNI_NAME_T uni_name;
	struct super_block *sb = inode->i_sb;

	/* check the validity of directory name in the given old pathname */
	ret = resolve_path(inode, path, &dir, &uni_name);
	if (ret)
		goto out;

	fs_set_vol_flags(sb, VOL_DIRTY);

	ret = create_dir(inode, &dir, &uni_name, fid);

	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);
out:
	return ret;
}

/* read a directory entry from the opened directory */
s32 exfat_fscore_readdir(struct inode *inode, DIR_ENTRY_T *dir_entry)
{
	s32 i;
	s32 dentries_per_clu, dentries_per_clu_bits = 0;
	u32 type, clu_offset;
	u64 sector;
	CHAIN_T dir, clu;
	UNI_NAME_T uni_name;
	TIMESTAMP_T tm;
	DENTRY_T *ep;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	u32 dentry = (u32)(fid->rwoffset & 0xFFFFFFFF); /* u32 is enough for directory */

	/* check if the given file ID is opened */
	if (fid->type != TYPE_DIR)
		return -EPERM;

	if (fid->entry == -1) {
		dir.dir = fsi->root_dir;
		dir.size = 0; /* just initialize, but will not use */
		dir.flags = 0x01;
	} else {
		dir.dir = fid->start_clu;
		dir.size = fid->size >> fsi->cluster_size_bits;
		dir.flags = fid->flags;
		exfat_debug_bug_on(dentry >= (dir.size * fsi->dentries_per_clu));
	}

	if (IS_CLUS_FREE(dir.dir)) { /* FAT16 root_dir */
		dentries_per_clu = fsi->dentries_in_root;

		/* Prevent readdir over directory size */
		if (dentry >= dentries_per_clu) {
			clu.dir = CLUS_EOF;
		} else {
			clu.dir = dir.dir;
			clu.size = dir.size;
			clu.flags = dir.flags;
		}
	} else {
		dentries_per_clu = fsi->dentries_per_clu;
		dentries_per_clu_bits = ilog2(dentries_per_clu);

		clu_offset = dentry >> dentries_per_clu_bits;
		clu.dir = dir.dir;
		clu.size = dir.size;
		clu.flags = dir.flags;

		if (clu.flags == 0x03) {
			clu.dir += clu_offset;
			clu.size -= clu_offset;
		} else {
			/* hint_information */
			if ((clu_offset > 0) &&
				((fid->hint_bmap.off != CLUS_EOF) && (fid->hint_bmap.off > 0)) &&
				(clu_offset >= fid->hint_bmap.off)) {
				clu_offset -= fid->hint_bmap.off;
				clu.dir = fid->hint_bmap.clu;
			}

			while (clu_offset > 0) {
				if (get_next_clus_safe(sb, &(clu.dir)))
					return -EIO;

				clu_offset--;
			}
		}
	}

	while (!IS_CLUS_EOF(clu.dir)) {
		if (IS_CLUS_FREE(dir.dir)) /* FAT16 root_dir */
			i = dentry % dentries_per_clu;
		else
			i = dentry & (dentries_per_clu-1);

		for ( ; i < dentries_per_clu; i++, dentry++) {
			ep = exfat_get_dentry_in_dir(sb, &clu, i, &sector);
			if (!ep)
				return -EIO;

			type = fsi->fs_func->get_entry_type(ep);

			if (type == TYPE_UNUSED)
				break;

			if ((type != TYPE_FILE) && (type != TYPE_DIR))
				continue;

			exfat_dcache_lock(sb, sector);
			dir_entry->Attr = fsi->fs_func->get_entry_attr(ep);

			fsi->fs_func->get_entry_time(ep, &tm, TM_CREATE);
			dir_entry->CreateTimestamp.Year = tm.year;
			dir_entry->CreateTimestamp.Month = tm.mon;
			dir_entry->CreateTimestamp.Day = tm.day;
			dir_entry->CreateTimestamp.Hour = tm.hour;
			dir_entry->CreateTimestamp.Minute = tm.min;
			dir_entry->CreateTimestamp.Second = tm.sec;
			dir_entry->CreateTimestamp.MilliSecond = 0;

			fsi->fs_func->get_entry_time(ep, &tm, TM_MODIFY);
			dir_entry->ModifyTimestamp.Year = tm.year;
			dir_entry->ModifyTimestamp.Month = tm.mon;
			dir_entry->ModifyTimestamp.Day = tm.day;
			dir_entry->ModifyTimestamp.Hour = tm.hour;
			dir_entry->ModifyTimestamp.Minute = tm.min;
			dir_entry->ModifyTimestamp.Second = tm.sec;
			dir_entry->ModifyTimestamp.MilliSecond = 0;

			memset((s8 *) &dir_entry->AccessTimestamp, 0, sizeof(DATE_TIME_T));

			*(uni_name.name) = 0x0;
			fsi->fs_func->get_uniname_from_ext_entry(sb, &dir, dentry, uni_name.name);
			if (*(uni_name.name) == 0x0)
				exfat_get_uniname_from_dos_entry(sb, (DOS_DENTRY_T *) ep, &uni_name, 0x1);
			exfat_nls_uni16s_to_vfsname(sb, &uni_name,
				dir_entry->NameBuf.lfn,
				dir_entry->NameBuf.lfnbuf_len);
			exfat_dcache_unlock(sb, sector);

			ep = exfat_get_dentry_in_dir(sb, &clu, i+1, NULL);
			if (!ep)
				return -EIO;

			dir_entry->Size = fsi->fs_func->get_entry_size(ep);

			/*
			 * Update hint information :
			 * fat16 root directory does not need it.
			 */
			if (!IS_CLUS_FREE(dir.dir)) {
				fid->hint_bmap.off = dentry >> dentries_per_clu_bits;
				fid->hint_bmap.clu = clu.dir;
			}

			fid->rwoffset = (s64) ++dentry;

			return 0;
		}

		/* fat16 root directory */
		if (IS_CLUS_FREE(dir.dir))
			break;

		if (clu.flags == 0x03) {
			if ((--clu.size) > 0)
				clu.dir++;
			else
				clu.dir = CLUS_EOF;
		} else {
			if (get_next_clus_safe(sb, &(clu.dir)))
				return -EIO;
		}
	}

	dir_entry->NameBuf.lfn[0] = '\0';

	fid->rwoffset = (s64)dentry;

	return 0;
}

/* remove a directory */
s32 exfat_fscore_rmdir(struct inode *inode, FILE_ID_T *fid)
{
	s32 ret;
	s32 dentry;
	DENTRY_T *ep;
	CHAIN_T dir, clu_to_free;
	struct super_block *sb = inode->i_sb;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	dir.dir = fid->dir.dir;
	dir.size = fid->dir.size;
	dir.flags = fid->dir.flags;

	dentry = fid->entry;

	if (fid->dir.dir == DIR_DELETED) {
		EMSG("%s : abnormal access to deleted dentry\n", __func__);
		return -ENOENT;
	}

	ep = exfat_get_dentry_in_dir(sb, &dir, dentry, NULL);
	if (!ep)
		return -EIO;

	if (EXFAT_SB(sb)->fsi.fs_func->get_entry_attr(ep) & ATTR_READONLY)
		return -EPERM;

	clu_to_free.dir = fid->start_clu;
	clu_to_free.size = ((fid->size-1) >> fsi->cluster_size_bits) + 1;
	clu_to_free.flags = fid->flags;

	ret = check_dir_empty(sb, &clu_to_free);
	if (ret) {
		if (ret == -EIO)
			EMSG("%s : failed to check_dir_empty : err(%d)\n",
				__func__, ret);
		return ret;
	}

	fs_set_vol_flags(sb, VOL_DIRTY);

	/* (1) update the directory entry */
	ret = remove_file(inode, &dir, dentry);
	if (ret) {
		EMSG("%s : failed to remove_file : err(%d)\n", __func__, ret);
		return ret;
	}

	fid->dir.dir = DIR_DELETED;

	fs_sync(sb, 0);
	fs_set_vol_flags(sb, VOL_CLEAN);

	return ret;
}
