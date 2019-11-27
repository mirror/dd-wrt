// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  blkdev.c: exFAT Block Device Driver Glue Layer
 */

#include <linux/blkdev.h>
#include <linux/log2.h>
#include <linux/backing-dev.h>
#include "exfat.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
	/* EMPTY */
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0) */
static struct backing_dev_info *inode_to_bdi(struct inode *bd_inode)
{
	return bd_inode->i_mapping->backing_dev_info;
}
#endif

s32 exfat_bdev_open_dev(struct super_block *sb)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (fsi->bd_opened)
		return 0;

	fsi->bd_opened = true;
	return 0;
}

s32 exfat_bdev_close_dev(struct super_block *sb)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	fsi->bd_opened = false;
	return 0;
}

static inline s32 block_device_ejected(struct super_block *sb)
{
	struct inode *bd_inode = sb->s_bdev->bd_inode;
	struct backing_dev_info *bdi = inode_to_bdi(bd_inode);

	return (bdi->dev == NULL);
}

s32 exfat_bdev_check_bdi_valid(struct super_block *sb)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (block_device_ejected(sb)) {
		if (!(fsi->prev_eio & EXFAT_EIO_BDI)) {
			fsi->prev_eio |= EXFAT_EIO_BDI;
			exfat_log_msg(sb, KERN_ERR, "%s: block device is "
				"eliminated.(bdi:%p)", __func__, sb->s_bdi);
			exfat_debug_warn_on(1);
		}
		return -ENXIO;
	}

	return 0;
}


/* Make a readahead request */
s32 exfat_bdev_readahead(struct super_block *sb, u64 secno, u64 num_secs)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	u32 sects_per_page = (PAGE_SIZE >> sb->s_blocksize_bits);
	struct blk_plug plug;
	u64 i;

	if (!fsi->bd_opened)
		return -EIO;

	blk_start_plug(&plug);
	for (i = 0; i < num_secs; i++) {
		if (i && !(i & (sects_per_page - 1))) {
#ifdef MODULE
			/* TODO: fix this by using proper APIs */
			blk_finish_plug(&plug);
			blk_start_plug(&plug);
#else
			blk_flush_plug(current);
#endif
		}
		sb_breadahead(sb, (sector_t)(secno + i));
	}
	blk_finish_plug(&plug);

	return 0;
}

s32 exfat_bdev_mread(struct super_block *sb, u64 secno, struct buffer_head **bh, u64 num_secs, s32 read)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	u8 blksize_bits = sb->s_blocksize_bits;

	if (!fsi->bd_opened)
		return -EIO;

	brelse(*bh);

	if (read)
		*bh = __bread(sb->s_bdev, (sector_t)secno, num_secs << blksize_bits);
	else
		*bh = __getblk(sb->s_bdev, (sector_t)secno, num_secs << blksize_bits);

	/* read successfully */
	if (*bh)
		return 0;

	/*
	 * patch 1.2.4 : reset ONCE warning message per volume.
	 */
	if (!(fsi->prev_eio & EXFAT_EIO_READ)) {
		fsi->prev_eio |= EXFAT_EIO_READ;
		exfat_log_msg(sb, KERN_ERR, "%s: No bh. I/O error.", __func__);
		exfat_debug_warn_on(1);
	}

	return -EIO;
}

s32 exfat_bdev_mwrite(struct super_block *sb, u64 secno, struct buffer_head *bh, u64 num_secs, s32 sync)
{
	u64 count;
	struct buffer_head *bh2;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (!fsi->bd_opened)
		return -EIO;

	if (secno == bh->b_blocknr) {
		set_buffer_uptodate(bh);
		mark_buffer_dirty(bh);
		if (sync && (sync_dirty_buffer(bh) != 0))
			return -EIO;
	} else {
		count = num_secs << sb->s_blocksize_bits;

		bh2 = __getblk(sb->s_bdev, (sector_t)secno, count);

		if (!bh2)
			goto no_bh;

		lock_buffer(bh2);
		memcpy(bh2->b_data, bh->b_data, count);
		set_buffer_uptodate(bh2);
		mark_buffer_dirty(bh2);
		unlock_buffer(bh2);
		if (sync && (sync_dirty_buffer(bh2) != 0)) {
			__brelse(bh2);
			goto no_bh;
		}
		__brelse(bh2);
	}
	return 0;
no_bh:
	/*
	 * patch 1.2.4 : reset ONCE warning message per volume.
	 */
	if (!(fsi->prev_eio & EXFAT_EIO_WRITE)) {
		fsi->prev_eio |= EXFAT_EIO_WRITE;
		exfat_log_msg(sb, KERN_ERR, "%s: No bh. I/O error.", __func__);
		exfat_debug_warn_on(1);
	}

	return -EIO;
}

s32 exfat_bdev_sync_all(struct super_block *sb)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (!fsi->bd_opened)
		return -EIO;

	return sync_blockdev(sb->s_bdev);
}

/*
 *  Sector Read/Write Functions
 */
s32 exfat_read_sect(struct super_block *sb, u64 sec, struct buffer_head **bh, s32 read)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	BUG_ON(!bh);
	if ((sec >= fsi->num_sectors) && (fsi->num_sectors > 0)) {
		exfat_fs_error_ratelimit(sb,
				"%s: out of range (sect:%llu)", __func__, sec);
		return -EIO;
	}

	if (exfat_bdev_mread(sb, sec, bh, 1, read)) {
		exfat_fs_error_ratelimit(sb,
				"%s: I/O error (sect:%llu)", __func__, sec);
		return -EIO;
	}

	return 0;
}

s32 exfat_write_sect(struct super_block *sb, u64 sec, struct buffer_head *bh, s32 sync)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	BUG_ON(!bh);
	if ((sec >= fsi->num_sectors) && (fsi->num_sectors > 0)) {
		exfat_fs_error_ratelimit(sb,
				"%s: out of range (sect:%llu)", __func__, sec);
		return -EIO;
	}

	if (exfat_bdev_mwrite(sb, sec, bh, 1, sync)) {
		exfat_fs_error_ratelimit(sb, "%s: I/O error (sect:%llu)",
						__func__, sec);
		return -EIO;
	}

	return 0;
}

static inline void __blkdev_write_bhs(struct buffer_head **bhs, s32 nr_bhs)
{
	s32 i;

	for (i = 0; i < nr_bhs; i++)
		write_dirty_buffer(bhs[i], WRITE);
}

static inline s32 __blkdev_sync_bhs(struct buffer_head **bhs, s32 nr_bhs)
{
	s32 i, err = 0;

	for (i = 0; i < nr_bhs; i++) {
		wait_on_buffer(bhs[i]);
		if (!err && !buffer_uptodate(bhs[i]))
			err = -EIO;
	}
	return err;
}

static inline s32 __buffer_zeroed(struct super_block *sb, u64 blknr, u64 num_secs)
{
	struct buffer_head *bhs[MAX_BUF_PER_PAGE];
	s32 nr_bhs = MAX_BUF_PER_PAGE;
	u64 last_blknr = blknr + num_secs;
	s32 err, i, n;
	struct blk_plug plug;

	/* Zeroing the unused blocks on this cluster */
	n = 0;
	blk_start_plug(&plug);
	while (blknr < last_blknr) {
		bhs[n] = sb_getblk(sb, (sector_t)blknr);
		if (!bhs[n]) {
			err = -ENOMEM;
			blk_finish_plug(&plug);
			goto error;
		}
		memset(bhs[n]->b_data, 0, sb->s_blocksize);
		set_buffer_uptodate(bhs[n]);
		mark_buffer_dirty(bhs[n]);

		n++;
		blknr++;

		if (blknr == last_blknr)
			break;

		if (n == nr_bhs) {
			__blkdev_write_bhs(bhs, n);

			for (i = 0; i < n; i++)
				brelse(bhs[i]);
			n = 0;
		}
	}
	__blkdev_write_bhs(bhs, n);
	blk_finish_plug(&plug);

	err = __blkdev_sync_bhs(bhs, n);
	if (err)
		goto error;

	for (i = 0; i < n; i++)
		brelse(bhs[i]);

	return 0;

error:
	EMSG("%s: failed zeroed sect %llu\n", __func__, blknr);
	for (i = 0; i < n; i++)
		bforget(bhs[i]);

	return err;
}

s32 exfat_write_msect_zero(struct super_block *sb, u64 sec, u64 num_secs)
{
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);

	if (((sec+num_secs) > fsi->num_sectors) && (fsi->num_sectors > 0)) {
		exfat_fs_error_ratelimit(sb, "%s: out of range(sect:%llu len:%llu)",
						__func__, sec, num_secs);
		return -EIO;
	}

	/* Just return -EAGAIN if it is failed */
	if (__buffer_zeroed(sb, sec, num_secs))
		return -EAGAIN;

	return 0;
}
