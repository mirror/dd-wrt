// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 *
 *  super.c: exFAT glue layer for supporting VFS
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/pagemap.h>
#include <linux/mpage.h>
#include <linux/buffer_head.h>
#include <linux/exportfs.h>
#include <linux/mount.h>
#include <linux/vfs.h>
#include <linux/parser.h>
#include <linux/uio.h>
#include <linux/writeback.h>
#include <linux/log2.h>
#include <linux/hash.h>
#include <linux/backing-dev.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/namei.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/swap.h> /* for mark_page_accessed() */
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <asm/current.h>
#include <asm/unaligned.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#include <linux/aio.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)
#error EXFAT only supports linux kernel version 3.0 or higher
#endif

#include "version.h"
#include "config.h"

#include "exfat.h"
#include "core.h"

/* skip iterating emit_dots when dir is empty */
#define ITER_POS_FILLED_DOTS	(2)

static struct kset *exfat_kset;
static struct kmem_cache *exfat_inode_cachep;

static DEFINE_MUTEX(_lock_core);

static int exfat_default_codepage = CONFIG_EXFAT_DEFAULT_CODEPAGE;
static char exfat_default_iocharset[] = CONFIG_EXFAT_DEFAULT_IOCHARSET;
static const char exfat_iocharset_with_utf8[] = "iso8859-1";

static void exfat_truncate(struct inode *inode, loff_t old_size);
static int exfat_get_block(struct inode *inode, sector_t iblock,
				   struct buffer_head *bh_result, int create);

static struct inode *exfat_iget(struct super_block *sb, loff_t i_pos);
static struct inode *exfat_build_inode(struct super_block *sb, const FILE_ID_T *fid, loff_t i_pos);
static void exfat_detach(struct inode *inode);
static void exfat_attach(struct inode *inode, loff_t i_pos);
static inline unsigned long exfat_hash(loff_t i_pos);
static s32 __exfat_sync_fs(struct super_block *sb, s32 do_sync);
static int __exfat_write_inode(struct inode *inode, int sync);
static int exfat_sync_inode(struct inode *inode);
static int exfat_write_inode(struct inode *inode, struct writeback_control *wbc);
static void exfat_write_super(struct super_block *sb);
static void exfat_write_failed(struct address_space *mapping, loff_t to);

static void exfat_init_namebuf(DENTRY_NAMEBUF_T *nb);
static int exfat_alloc_namebuf(DENTRY_NAMEBUF_T *nb);
static void exfat_free_namebuf(DENTRY_NAMEBUF_T *nb);

static int __exfat_getattr(struct inode *inode, struct kstat *stat);
static void __exfat_writepage_end_io(struct bio *bio, int err);
static inline void lock_super(struct super_block *sb);
static inline void unlock_super(struct super_block *sb);
static int exfat_create_compat(struct inode *dir, struct dentry *dentry);
static int __exfat_revalidate(struct dentry *dentry);
static int __exfat_revalidate_ci(struct dentry *dentry, unsigned int flags);
static int __exfat_file_fsync(struct file *filp, loff_t start, loff_t end, int datasync);
static struct dentry *__exfat_lookup(struct inode *dir, struct dentry *dentry);
static int __exfat_mkdir(struct inode *dir, struct dentry *dentry);
static int __exfat_rename(struct inode *old_dir, struct dentry *old_dentry,
		struct inode *new_dir, struct dentry *new_dentry);
static int __exfat_show_options(struct seq_file *m, struct super_block *sb);
static inline ssize_t __exfat_blkdev_direct_IO(int rw, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t offset,
		unsigned long nr_segs);
static inline ssize_t __exfat_direct_IO(int rw, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t offset,
		loff_t count, unsigned long nr_segs);
static int __exfat_d_hash(const struct dentry *dentry, struct qstr *qstr);
static int __exfat_d_hashi(const struct dentry *dentry, struct qstr *qstr);
static int __exfat_cmp(const struct dentry *dentry, unsigned int len,
		const char *str, const struct qstr *name);
static int __exfat_cmpi(const struct dentry *dentry, unsigned int len,
		const char *str, const struct qstr *name);

/* mount the file system volume */
static s32 exfat_mount(struct super_block *sb)
{
	s32 err;

	/* acquire the core lock for file system ccritical section */
	mutex_lock(&_lock_core);

	err = exfat_meta_cache_init(sb);
	if (err)
		goto out;

	err = exfat_fscore_mount(sb);
out:
	if (err)
		exfat_meta_cache_shutdown(sb);

	/* release the core lock for file system critical section */
	mutex_unlock(&_lock_core);

	return err;
}

/* unmount the file system volume */
static s32 exfat_umount(struct super_block *sb)
{
	s32 err;

	/* acquire the core lock for file system ccritical section */
	mutex_lock(&_lock_core);

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_umount(sb);
	exfat_meta_cache_shutdown(sb);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));

	/* release the core lock for file system critical section */
	mutex_unlock(&_lock_core);

	return err;
}

static s32 exfat_set_vol_flags(struct super_block *sb, u16 new_flag, s32 always_sync)
{
	s32 err;

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_set_vol_flags(sb, new_flag, always_sync);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

/* read the target string of symlink */
static s32 exfat_read_link(struct inode *inode, FILE_ID_T *fid, void *buffer, u64 count, u64 *rcount)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	/* check the validity of pointer parameters */
	ASSERT(fid && buffer);

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_read_link(inode, fid, buffer, count, rcount);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

/* write the target string of symlink */
static s32 exfat_write_link(struct inode *inode, FILE_ID_T *fid, void *buffer, u64 count, u64 *wcount)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	/* check the validity of pointer parameters */
	ASSERT(fid && buffer);

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_write_link(inode, fid, buffer, count, wcount);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

/* remove a file */
static s32 exfat_remove(struct inode *inode, FILE_ID_T *fid)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	/* check the validity of pointer parameters */
	ASSERT(fid);

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_remove(inode, fid);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

/* get the information of a given file */
static s32 exfat_read_inode(struct inode *inode, DIR_ENTRY_T *info)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_read_inode(inode, info);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

/* return the cluster number in the given cluster offset */
static s32 exfat_map_clus(struct inode *inode, u32 clu_offset, u32 *clu, int dest)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	/* check the validity of pointer parameters */
	ASSERT(clu);

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_map_clus(inode, clu_offset, clu, dest);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

/* read a directory entry from the opened directory */
static s32 __exfat_readdir(struct inode *inode, DIR_ENTRY_T *dir_entry)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	/* check the validity of pointer parameters */
	ASSERT(dir_entry);

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_readdir(inode, dir_entry);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

/* reflect the internal dirty flags to VFS bh dirty flags */
static s32 exfat_cache_flush(struct super_block *sb, int do_sync)
{
	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	exfat_fcache_flush(sb, do_sync);
	exfat_dcache_flush(sb, do_sync);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
       /* EMPTY */
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0) */
static inline void bio_set_dev(struct bio *bio, struct block_device *bdev)
{
	bio->bi_bdev = bdev;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)
#define current_time(x)	CURRENT_TIME_SEC
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
static int exfat_getattr(const struct path *path, struct kstat *stat,
			u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_backing_inode(path->dentry);

	return __exfat_getattr(inode, stat);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0) */
static int exfat_getattr(struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat)
{
	struct inode *inode = dentry->d_inode;

	return __exfat_getattr(inode, stat);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
static inline void __exfat_clean_bdev_aliases(struct block_device *bdev, sector_t block)
{
	clean_bdev_aliases(bdev, block, 1);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0) */
static inline void __exfat_clean_bdev_aliases(struct block_device *bdev, sector_t block)
{
	unmap_underlying_metadata(bdev, block);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
static int exfat_rename(struct inode *old_dir, struct dentry *old_dentry,
		struct inode *new_dir, struct dentry *new_dentry,
		unsigned int flags)
{
	/*
	 * The VFS already checks for existence, so for local filesystems
	 * the RENAME_NOREPLACE implementation is equivalent to plain rename.
	 * Don't support any other flags
	 */
	if (flags & ~RENAME_NOREPLACE)
		return -EINVAL;
	return __exfat_rename(old_dir, old_dentry, new_dir, new_dentry);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0) */
static int exfat_rename(struct inode *old_dir, struct dentry *old_dentry,
		struct inode *new_dir, struct dentry *new_dentry)
{
	return __exfat_rename(old_dir, old_dentry, new_dir, new_dentry);
}

// setattr_prepare() was backported to several LTS kernels
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 39) && \
    LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
#define SETATTR_PREPARE_AVAILABLE
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 37) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
#define SETATTR_PREPARE_AVAILABLE
#endif

#ifndef SETATTR_PREPARE_AVAILABLE
static int setattr_prepare(struct dentry *dentry, struct iattr *attr)
{
	struct inode *inode = dentry->d_inode;

	return inode_change_ok(inode, attr);
}
#endif
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
static inline void __exfat_submit_bio_write(struct bio *bio)
{
	bio_set_op_attrs(bio, REQ_OP_WRITE, 0);
	submit_bio(bio);
}

static inline unsigned int __exfat_full_name_hash(const struct dentry *dentry, const char *name, unsigned int len)
{
	return full_name_hash(dentry, name, len);
}

static inline unsigned long __exfat_init_name_hash(const struct dentry *dentry)
{
	return init_name_hash(dentry);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0) */
static inline void __exfat_submit_bio_write(struct bio *bio)
{
	submit_bio(WRITE, bio);
}

static inline unsigned int __exfat_full_name_hash(const struct dentry *unused, const char *name, unsigned int len)
{
	return full_name_hash(name, len);
}

static inline unsigned long __exfat_init_name_hash(const struct dentry *unused)
{
	return init_name_hash();
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 21)
       /* EMPTY */
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 21) */
static inline void inode_lock(struct inode *inode)
{
	       mutex_lock(&inode->i_mutex);
}

static inline void inode_unlock(struct inode *inode)
{
	       mutex_unlock(&inode->i_mutex);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
static inline int exfat_remount_syncfs(struct super_block *sb)
{
	sync_filesystem(sb);
	return 0;
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0) */
static inline int exfat_remount_syncfs(struct super_block *sb)
{
	/*
	 * We don`t need to call sync_filesystem(sb),
	 * Because VFS calls it.
	 */
	return 0;
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
static inline sector_t __exfat_bio_sector(struct bio *bio)
{
	return bio->bi_iter.bi_sector;
}

static inline void __exfat_set_bio_iterate(struct bio *bio, sector_t sector,
		unsigned int size, unsigned int idx, unsigned int done)
{
	struct bvec_iter *iter = &(bio->bi_iter);

	iter->bi_sector = sector;
	iter->bi_size = size;
	iter->bi_idx = idx;
	iter->bi_bvec_done = done;
}

static void __exfat_truncate_pagecache(struct inode *inode,
					loff_t to, loff_t newsize)
{
	truncate_pagecache(inode, newsize);
}

static int exfat_d_hash(const struct dentry *dentry, struct qstr *qstr)
{
	return __exfat_d_hash(dentry, qstr);
}

static int exfat_d_hashi(const struct dentry *dentry, struct qstr *qstr)
{
	return __exfat_d_hashi(dentry, qstr);
}

//instead of exfat_readdir
static int exfat_iterate(struct file *filp, struct dir_context *ctx)
{
	struct inode *inode = filp->f_path.dentry->d_inode;
	struct super_block *sb = inode->i_sb;
	DIR_ENTRY_T de;
	DENTRY_NAMEBUF_T *nb = &(de.NameBuf);
	unsigned long inum;
	loff_t cpos;
	int err = 0, fake_offset = 0;

	exfat_init_namebuf(nb);
	lock_super(sb);

	cpos = ctx->pos;
	if (!dir_emit_dots(filp, ctx))
		goto out;
	if (ctx->pos == ITER_POS_FILLED_DOTS) {
		cpos = 0;
		fake_offset = 1;
	}
	if (cpos & (DENTRY_SIZE - 1)) {
		err = -ENOENT;
		goto out;
	}

	/* name buffer should be allocated before use */
	err = exfat_alloc_namebuf(nb);
	if (err)
		goto out;
get_new:
	EXFAT_I(inode)->fid.size = i_size_read(inode);
	EXFAT_I(inode)->fid.rwoffset = cpos >> DENTRY_SIZE_BITS;

	if (cpos >= EXFAT_I(inode)->fid.size)
		goto end_of_dir;

	err = __exfat_readdir(inode, &de);
	if (err) {
		// at least we tried to read a sector
		// move cpos to next sector position (should be aligned)
		if (err == -EIO) {
			cpos += 1 << (sb->s_blocksize_bits);
			cpos &= ~((u32)sb->s_blocksize-1);
		}

		err = -EIO;
		goto end_of_dir;
	}

	cpos = EXFAT_I(inode)->fid.rwoffset << DENTRY_SIZE_BITS;

	if (!nb->lfn[0])
		goto end_of_dir;

	if (!memcmp(nb->sfn, DOS_CUR_DIR_NAME, DOS_NAME_LENGTH)) {
		inum = inode->i_ino;
	} else if (!memcmp(nb->sfn, DOS_PAR_DIR_NAME, DOS_NAME_LENGTH)) {
		inum = parent_ino(filp->f_path.dentry);
	} else {
		loff_t i_pos = ((loff_t) EXFAT_I(inode)->fid.start_clu << 32) |
			((EXFAT_I(inode)->fid.rwoffset-1) & 0xffffffff);
		struct inode *tmp = exfat_iget(sb, i_pos);

		if (tmp) {
			inum = tmp->i_ino;
			iput(tmp);
		} else {
			inum = iunique(sb, EXFAT_ROOT_INO);
		}
	}

	/* Before calling dir_emit(), sb_lock should be released.
	 * Because page fault can occur in dir_emit() when the size of buffer given
	 * from user is larger than one page size
	 */
	unlock_super(sb);
	if (!dir_emit(ctx, nb->lfn, strlen(nb->lfn), inum,
			(de.Attr & ATTR_SUBDIR) ? DT_DIR : DT_REG))
		goto out_unlocked;
	lock_super(sb);

	ctx->pos = cpos;
	goto get_new;

end_of_dir:
	if (!cpos && fake_offset)
		cpos = ITER_POS_FILLED_DOTS;
	ctx->pos = cpos;
out:
	unlock_super(sb);
out_unlocked:
	/*
	 * To improve performance, free namebuf after unlock sb_lock.
	 * If namebuf is not allocated, this function do nothing
	 */
	exfat_free_namebuf(nb);
	return err;
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0) */
static inline sector_t __exfat_bio_sector(struct bio *bio)
{
	return bio->bi_sector;
}

static inline void __exfat_set_bio_iterate(struct bio *bio, sector_t sector,
		unsigned int size, unsigned int idx, unsigned int done)
{
	bio->bi_sector = sector;
	bio->bi_idx = idx;
	bio->bi_size = size; //PAGE_SIZE;
}

static void __exfat_truncate_pagecache(struct inode *inode,
					loff_t to, loff_t newsize)
{
	truncate_pagecache(inode, to, newsize);
}

static int exfat_d_hash(const struct dentry *dentry,
		const struct inode *inode, struct qstr *qstr)
{
	return __exfat_d_hash(dentry, qstr);
}

static int exfat_d_hashi(const struct dentry *dentry,
		const struct inode *inode, struct qstr *qstr)
{
	return __exfat_d_hashi(dentry, qstr);
}

static int exfat_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
	struct inode *inode = filp->f_path.dentry->d_inode;
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	FS_INFO_T *fsi = &(sbi->fsi);
	DIR_ENTRY_T de;
	DENTRY_NAMEBUF_T *nb = &(de.NameBuf);
	unsigned long inum;
	loff_t cpos;
	int err = 0, fake_offset = 0;

	exfat_init_namebuf(nb);
	lock_super(sb);

	cpos = filp->f_pos;
	/* Fake . and .. for the root directory. */
	while (cpos < ITER_POS_FILLED_DOTS) {
		if (inode->i_ino == EXFAT_ROOT_INO)
			inum = EXFAT_ROOT_INO;
		else if (cpos == 0)
			inum = inode->i_ino;
		else /* (cpos == 1) */
			inum = parent_ino(filp->f_path.dentry);

		if (filldir(dirent, "..", cpos+1, cpos, inum, DT_DIR) < 0)
			goto out;
		cpos++;
		filp->f_pos++;
	}
	if (cpos == ITER_POS_FILLED_DOTS) {
		cpos = 0;
		fake_offset = 1;
	}
	if (cpos & (DENTRY_SIZE - 1)) {
		err = -ENOENT;
		goto out;
	}

	/* name buffer should be allocated before use */
	err = exfat_alloc_namebuf(nb);
	if (err)
		goto out;
get_new:
	EXFAT_I(inode)->fid.size = i_size_read(inode);
	EXFAT_I(inode)->fid.rwoffset = cpos >> DENTRY_SIZE_BITS;

	if (cpos >= EXFAT_I(inode)->fid.size)
		goto end_of_dir;

	err = __exfat_readdir(inode, &de);
	if (err) {
		// at least we tried to read a sector
		// move cpos to next sector position (should be aligned)
		if (err == -EIO) {
			cpos += 1 << (sb->s_blocksize_bits);
			cpos &= ~((u32)sb->s_blocksize-1);
		}

		err = -EIO;
		goto end_of_dir;
	}

	cpos = EXFAT_I(inode)->fid.rwoffset << DENTRY_SIZE_BITS;

	if (!nb->lfn[0])
		goto end_of_dir;

	if (!memcmp(nb->sfn, DOS_CUR_DIR_NAME, DOS_NAME_LENGTH)) {
		inum = inode->i_ino;
	} else if (!memcmp(nb->sfn, DOS_PAR_DIR_NAME, DOS_NAME_LENGTH)) {
		inum = parent_ino(filp->f_path.dentry);
	} else {
		loff_t i_pos = ((loff_t) EXFAT_I(inode)->fid.start_clu << 32) |
			   ((EXFAT_I(inode)->fid.rwoffset-1) & 0xffffffff);
		struct inode *tmp = exfat_iget(sb, i_pos);

		if (tmp) {
			inum = tmp->i_ino;
			iput(tmp);
		} else {
			inum = iunique(sb, EXFAT_ROOT_INO);
		}
	}

	/* Before calling dir_emit(), sb_lock should be released.
	 * Because page fault can occur in dir_emit() when the size of buffer given
	 * from user is larger than one page size
	 */
	unlock_super(sb);
	if (filldir(dirent, nb->lfn, strlen(nb->lfn), cpos, inum,
				(de.Attr & ATTR_SUBDIR) ? DT_DIR : DT_REG) < 0)
		goto out_unlocked;
	lock_super(sb);

	filp->f_pos = cpos;
	goto get_new;

end_of_dir:
	if (!cpos && fake_offset)
		cpos = ITER_POS_FILLED_DOTS;
	filp->f_pos = cpos;
out:
	unlock_super(sb);
out_unlocked:
	/*
	 * To improve performance, free namebuf after unlock sb_lock.
	 * If namebuf is not allocated, this function do nothing
	 */
	exfat_free_namebuf(nb);
	return err;
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
	/* EMPTY */
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0) */
static inline struct inode *file_inode(const struct file *f)
{
	return f->f_dentry->d_inode;
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
static inline int __is_sb_dirty(struct super_block *sb)
{
	return EXFAT_SB(sb)->s_dirt;
}

static inline void __set_sb_clean(struct super_block *sb)
{
	EXFAT_SB(sb)->s_dirt = 0;
}

/* Workqueue wrapper for exfat_write_super () */
static void __write_super_delayed(struct work_struct *work)
{
	struct exfat_sb_info *sbi;
	struct super_block *sb;

	sbi = container_of(work, struct exfat_sb_info, write_super_work.work);
	sb = sbi->host_sb;

	/* XXX: Is this needed? */
	if (!sb || !down_read_trylock(&sb->s_umount)) {
		DMSG("%s: skip delayed work(write_super).\n", __func__);
		return;
	}

	DMSG("%s: do delayed_work(write_super).\n", __func__);

	spin_lock(&sbi->work_lock);
	sbi->write_super_queued = 0;
	spin_unlock(&sbi->work_lock);

	exfat_write_super(sb);

	up_read(&sb->s_umount);
}

static void setup_exfat_sync_super_wq(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	mutex_init(&sbi->s_lock);
	spin_lock_init(&sbi->work_lock);
	INIT_DELAYED_WORK(&sbi->write_super_work, __write_super_delayed);
	sbi->host_sb = sb;
}

static inline bool __cancel_delayed_work_sync(struct exfat_sb_info *sbi)
{
	return cancel_delayed_work_sync(&sbi->write_super_work);
}

static inline void lock_super(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	mutex_lock(&sbi->s_lock);
}

static inline void unlock_super(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	mutex_unlock(&sbi->s_lock);
}

static int exfat_revalidate(struct dentry *dentry, unsigned int flags)
{
	if (flags & LOOKUP_RCU)
		return -ECHILD;

	return __exfat_revalidate(dentry);
}

static int exfat_revalidate_ci(struct dentry *dentry, unsigned int flags)
{
	if (flags & LOOKUP_RCU)
		return -ECHILD;

	return __exfat_revalidate_ci(dentry, flags);
}

static struct inode *exfat_iget(struct super_block *sb, loff_t i_pos)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *info;
	struct hlist_head *head = sbi->inode_hashtable + exfat_hash(i_pos);
	struct inode *inode = NULL;

	spin_lock(&sbi->inode_hash_lock);
	hlist_for_each_entry(info, head, i_hash_fat) {
		BUG_ON(info->vfs_inode.i_sb != sb);

		if (i_pos != info->i_pos)
			continue;
		inode = igrab(&info->vfs_inode);
		if (inode)
			break;
	}
	spin_unlock(&sbi->inode_hash_lock);
	return inode;
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0) */
static inline int __is_sb_dirty(struct super_block *sb)
{
	return sb->s_dirt;
}

static inline void __set_sb_clean(struct super_block *sb)
{
	sb->s_dirt = 0;
}

static void setup_exfat_sync_super_wq(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	sbi->host_sb = sb;
}

static inline bool __cancel_delayed_work_sync(struct exfat_sb_info *sbi)
{
	/* DO NOTHING */
	return 0;
}

static inline void clear_inode(struct inode *inode)
{
	end_writeback(inode);
}

static int exfat_revalidate(struct dentry *dentry, struct nameidata *nd)
{
	if (nd && nd->flags & LOOKUP_RCU)
		return -ECHILD;

	return __exfat_revalidate(dentry);
}

static int exfat_revalidate_ci(struct dentry *dentry, struct nameidata *nd)
{
	if (nd && nd->flags & LOOKUP_RCU)
		return -ECHILD;

	return __exfat_revalidate_ci(dentry, nd ? nd->flags : 0);

}

static struct inode *exfat_iget(struct super_block *sb, loff_t i_pos)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *info;
	struct hlist_node *node;
	struct hlist_head *head = sbi->inode_hashtable + exfat_hash(i_pos);
	struct inode *inode = NULL;

	spin_lock(&sbi->inode_hash_lock);
	hlist_for_each_entry(info, node, head, i_hash_fat) {
		BUG_ON(info->vfs_inode.i_sb != sb);

		if (i_pos != info->i_pos)
			continue;
		inode = igrab(&info->vfs_inode);
		if (inode)
			break;
	}
	spin_unlock(&sbi->inode_hash_lock);
	return inode;
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
static struct dentry *exfat_lookup(struct inode *dir, struct dentry *dentry,
						   unsigned int flags)
{
	return __exfat_lookup(dir, dentry);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0) */
static struct dentry *exfat_lookup(struct inode *dir, struct dentry *dentry,
						   struct nameidata *nd)
{
	return __exfat_lookup(dir, dentry);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
	/* NOTHING NOW */
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0) */
#define GLOBAL_ROOT_UID (0)
#define GLOBAL_ROOT_GID (0)

static inline bool uid_eq(uid_t left, uid_t right)
{
	return left == right;
}

static inline bool gid_eq(gid_t left, gid_t right)
{
	return left == right;
}

static inline uid_t from_kuid_munged(struct user_namespace *to, uid_t kuid)
{
	return kuid;
}

static inline gid_t from_kgid_munged(struct user_namespace *to, gid_t kgid)
{
	return kgid;
}

static inline uid_t make_kuid(struct user_namespace *from, uid_t uid)
{
	return uid;
}

static inline gid_t make_kgid(struct user_namespace *from, gid_t gid)
{
	return gid;
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
static struct dentry *__d_make_root(struct inode *root_inode)
{
	return d_make_root(root_inode);
}

static void __exfat_do_truncate(struct inode *inode, loff_t old, loff_t new)
{
	down_write(&EXFAT_I(inode)->truncate_lock);
	truncate_setsize(inode, new);
	exfat_truncate(inode, old);
	up_write(&EXFAT_I(inode)->truncate_lock);
}

static sector_t exfat_aop_bmap(struct address_space *mapping, sector_t block)
{
	sector_t blocknr;

	/* exfat_get_cluster() assumes the requested blocknr isn't truncated. */
	down_read(&EXFAT_I(mapping->host)->truncate_lock);
	blocknr = generic_block_bmap(mapping, block, exfat_get_block);
	up_read(&EXFAT_I(mapping->host)->truncate_lock);
	return blocknr;
}

static int exfat_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	return __exfat_mkdir(dir, dentry);
}

static int exfat_show_options(struct seq_file *m, struct dentry *root)
{
	return __exfat_show_options(m, root->d_sb);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0) */
static inline void set_nlink(struct inode *inode, unsigned int nlink)
{
	inode->i_nlink = nlink;
}

static struct dentry *__d_make_root(struct inode *root_inode)
{
	return d_alloc_root(root_inode);
}

static void __exfat_do_truncate(struct inode *inode, loff_t old, loff_t new)
{
		truncate_setsize(inode, new);
		exfat_truncate(inode, old);
}

static sector_t exfat_aop_bmap(struct address_space *mapping, sector_t block)
{
	sector_t blocknr;

	/* exfat_get_cluster() assumes the requested blocknr isn't truncated. */
	down_read(&mapping->host->i_alloc_sem);
	blocknr = generic_block_bmap(mapping, block, exfat_get_block);
	up_read(&mapping->host->i_alloc_sem);
	return blocknr;
}

static int exfat_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
	return __exfat_mkdir(dir, dentry);
}

static int exfat_show_options(struct seq_file *m, struct vfsmount *mnt)
{
	return __exfat_show_options(m, mnt->mnt_sb);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0)
#define __exfat_generic_file_fsync(filp, start, end, datasync) \
		generic_file_fsync(filp, start, end, datasync)

static int exfat_file_fsync(struct file *filp, loff_t start, loff_t end, int datasync)
{
	return __exfat_file_fsync(filp, start, end, datasync);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0) */
#define __exfat_generic_file_fsync(filp, start, end, datasync) \
		generic_file_fsync(filp, datasync)
static int exfat_file_fsync(struct file *filp, int datasync)
{
	return __exfat_file_fsync(filp, 0, 0, datasync);
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
static void exfat_writepage_end_io(struct bio *bio)
{
	__exfat_writepage_end_io(bio, blk_status_to_errno(bio->bi_status));
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0)
static void exfat_writepage_end_io(struct bio *bio)
{
	__exfat_writepage_end_io(bio, bio->bi_error);
}
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0) */
static void exfat_writepage_end_io(struct bio *bio, int err)
{
	if (test_bit(BIO_UPTODATE, &bio->bi_flags))
		err = 0;
	__exfat_writepage_end_io(bio, err);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
static int exfat_cmp(const struct dentry *dentry,
		unsigned int len, const char *str, const struct qstr *name)
{
	return __exfat_cmp(dentry, len, str, name);
}

static int exfat_cmpi(const struct dentry *dentry,
		unsigned int len, const char *str, const struct qstr *name)
{
	return __exfat_cmpi(dentry, len, str, name);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
static int exfat_cmp(const struct dentry *parent, const struct dentry *dentry,
		unsigned int len, const char *str, const struct qstr *name)
{
	return __exfat_cmp(dentry, len, str, name);
}

static int exfat_cmpi(const struct dentry *parent, const struct dentry *dentry,
		unsigned int len, const char *str, const struct qstr *name)
{
	return __exfat_cmpi(dentry, len, str, name);
}
#else
static int exfat_cmp(const struct dentry *parent, const struct inode *pinode,
		const struct dentry *dentry, const struct inode *inode,
		unsigned int len, const char *str, const struct qstr *name)
{
	return __exfat_cmp(dentry, len, str, name);
}

static int exfat_cmpi(const struct dentry *parent, const struct inode *pinode,
		const struct dentry *dentry, const struct inode *inode,
		unsigned int len, const char *str, const struct qstr *name)
{
	return __exfat_cmpi(dentry, len, str, name);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
static ssize_t exfat_direct_IO(struct kiocb *iocb, struct iov_iter *iter)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	size_t count = iov_iter_count(iter);
	int rw = iov_iter_rw(iter);
	loff_t offset = iocb->ki_pos;

	return __exfat_direct_IO(rw, iocb, inode,
			(void *)iter, offset, count, 0 /* UNUSED */);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
static ssize_t exfat_direct_IO(struct kiocb *iocb,
		struct iov_iter *iter,
		loff_t offset)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	size_t count = iov_iter_count(iter);
	int rw = iov_iter_rw(iter);

	return __exfat_direct_IO(rw, iocb, inode,
			(void *)iter, offset, count, 0 /* UNUSED */);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
static ssize_t exfat_direct_IO(int rw, struct kiocb *iocb,
		struct iov_iter *iter,
		loff_t offset)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	size_t count = iov_iter_count(iter);

	return __exfat_direct_IO(rw, iocb, inode,
			(void *)iter, offset, count, 0 /* UNUSED */);
}
#else
static ssize_t exfat_direct_IO(int rw, struct kiocb *iocb,
	   const struct iovec *iov, loff_t offset, unsigned long nr_segs)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	size_t count = iov_length(iov, nr_segs);

	return __exfat_direct_IO(rw, iocb, inode,
			(void *)iov, offset, count, nr_segs);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
static inline ssize_t __exfat_blkdev_direct_IO(int unused, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t unused_1,
		unsigned long nr_segs)
{
	struct iov_iter *iter = (struct iov_iter *)iov_u;

	return blockdev_direct_IO(iocb, inode, iter, exfat_get_block);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
static inline ssize_t __exfat_blkdev_direct_IO(int unused, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t offset,
		unsigned long nr_segs)
{
	struct iov_iter *iter = (struct iov_iter *)iov_u;

	return blockdev_direct_IO(iocb, inode, iter, offset, exfat_get_block);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
static inline ssize_t __exfat_blkdev_direct_IO(int rw, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t offset,
		unsigned long nr_segs)
{
	struct iov_iter *iter = (struct iov_iter *)iov_u;

	return blockdev_direct_IO(rw, iocb, inode, iter,
					offset, exfat_get_block);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
static inline ssize_t __exfat_blkdev_direct_IO(int rw, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t offset,
		unsigned long nr_segs)
{
	const struct iovec *iov = (const struct iovec *)iov_u;

	return blockdev_direct_IO(rw, iocb, inode, iov,
					offset, nr_segs, exfat_get_block);
}
#else
static inline ssize_t __exfat_blkdev_direct_IO(int rw, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t offset,
		unsigned long nr_segs)
{
	const struct iovec *iov = (const struct iovec *)iov_u;

	return blockdev_direct_IO(rw, iocb, inode, inode->i_sb->s_bdev, iov,
					offset, nr_segs, exfat_get_block, NULL);
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
static const char *exfat_follow_link(struct dentry *dentry, struct inode *inode, struct delayed_call *done)
{
	struct exfat_inode_info *ei = EXFAT_I(inode);

	return (char *)(ei->target);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
static const char *exfat_follow_link(struct dentry *dentry, void **cookie)
{
	struct exfat_inode_info *ei = EXFAT_I(dentry->d_inode);

	return *cookie = (char *)(ei->target);
}
#else
static void *exfat_follow_link(struct dentry *dentry, struct nameidata *nd)
{
	struct exfat_inode_info *ei = EXFAT_I(dentry->d_inode);

	nd_set_link(nd, (char *)(ei->target));
	return NULL;
}
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
static int exfat_create(struct inode *dir, struct dentry *dentry, umode_t mode,
			 bool excl)
{
	return exfat_create_compat(dir, dentry);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
static int exfat_create(struct inode *dir, struct dentry *dentry, umode_t mode,
			struct nameidata *nd)
{
	return exfat_create_compat(dir, dentry);
}
#else
static int exfat_create(struct inode *dir, struct dentry *dentry, int mode,
			struct nameidata *nd)
{
	return exfat_create_compat(dir, dentry);
}
#endif

static inline loff_t exfat_make_i_pos(FILE_ID_T *fid)
{
	return ((loff_t) fid->dir.dir << 32) | (fid->entry & 0xffffffff);
}

static void exfat_init_namebuf(DENTRY_NAMEBUF_T *nb)
{
	nb->lfn = NULL;
	nb->sfn = NULL;
	nb->lfnbuf_len = 0;
	nb->sfnbuf_len = 0;
}

static int exfat_alloc_namebuf(DENTRY_NAMEBUF_T *nb)
{
	nb->lfn = __getname();
	if (!nb->lfn)
		return -ENOMEM;
	nb->sfn = nb->lfn + MAX_VFSNAME_BUF_SIZE;
	nb->lfnbuf_len = MAX_VFSNAME_BUF_SIZE;
	nb->sfnbuf_len = MAX_VFSNAME_BUF_SIZE;
	return 0;
}

static void exfat_free_namebuf(DENTRY_NAMEBUF_T *nb)
{
	if (!nb->lfn)
		return;

	__putname(nb->lfn);
	exfat_init_namebuf(nb);
}

#define EXFAT_DSTATE_LOCKED	(void *)(0xCAFE2016)
#define EXFAT_DSTATE_UNLOCKED	(void *)(0x00000000)

static inline void __lock_d_revalidate(struct dentry *dentry)
{
	spin_lock(&dentry->d_lock);
	dentry->d_fsdata = EXFAT_DSTATE_LOCKED;
	spin_unlock(&dentry->d_lock);
}

static inline void __unlock_d_revalidate(struct dentry *dentry)
{
	spin_lock(&dentry->d_lock);
	dentry->d_fsdata = EXFAT_DSTATE_UNLOCKED;
	spin_unlock(&dentry->d_lock);
}

/* __check_dstate_locked requires dentry->d_lock */
static inline int __check_dstate_locked(struct dentry *dentry)
{
	if (dentry->d_fsdata == EXFAT_DSTATE_LOCKED)
		return 1;

	return 0;
}

/*
 * If new entry was created in the parent, it could create the 8.3
 * alias (the shortname of logname).  So, the parent may have the
 * negative-dentry which matches the created 8.3 alias.
 *
 * If it happened, the negative dentry isn't actually negative
 * anymore.  So, drop it.
 */
static int __exfat_revalidate_common(struct dentry *dentry)
{
	int ret = 1;

	spin_lock(&dentry->d_lock);
	if ((!dentry->d_inode) && (!__check_dstate_locked(dentry) &&
		(dentry->d_time != GET_IVERSION(dentry->d_parent->d_inode)))) {
		ret = 0;
	}
	spin_unlock(&dentry->d_lock);
	return ret;
}

static int __exfat_revalidate(struct dentry *dentry)
{
	/* This is not negative dentry. Always valid. */
	if (dentry->d_inode)
		return 1;
	return __exfat_revalidate_common(dentry);
}

static int __exfat_revalidate_ci(struct dentry *dentry, unsigned int flags)
{
	/*
	 * This is not negative dentry. Always valid.
	 *
	 * Note, rename() to existing directory entry will have ->d_inode,
	 * and will use existing name which isn't specified name by user.
	 *
	 * We may be able to drop this positive dentry here. But dropping
	 * positive dentry isn't good idea. So it's unsupported like
	 * rename("filename", "FILENAME") for now.
	 */
	if (dentry->d_inode)
		return 1;
#if 0	/* Blocked below code for lookup_one_len() called by stackable FS */
	/*
	 * This may be nfsd (or something), anyway, we can't see the
	 * intent of this. So, since this can be for creation, drop it.
	 */
	if (!flags)
		return 0;
#endif
	/*
	 * Drop the negative dentry, in order to make sure to use the
	 * case sensitive name which is specified by user if this is
	 * for creation.
	 */
	if (flags & (LOOKUP_CREATE | LOOKUP_RENAME_TARGET))
		return 0;
	return __exfat_revalidate_common(dentry);
}


/* returns the length of a struct qstr, ignoring trailing dots */
static unsigned int __exfat_striptail_len(unsigned int len, const char *name)
{
	while (len && name[len - 1] == '.')
		len--;
	return len;
}

static unsigned int exfat_striptail_len(const struct qstr *qstr)
{
	return __exfat_striptail_len(qstr->len, qstr->name);
}

/*
 * Compute the hash for the exfat name corresponding to the dentry.
 * Note: if the name is invalid, we leave the hash code unchanged so
 * that the existing dentry can be used. The exfat fs routines will
 * return ENOENT or EINVAL as appropriate.
 */
static int __exfat_d_hash(const struct dentry *dentry, struct qstr *qstr)
{
	unsigned int len = exfat_striptail_len(qstr);

	qstr->hash = __exfat_full_name_hash(dentry, qstr->name, len);
	return 0;
}

/*
 * Compute the hash for the exfat name corresponding to the dentry.
 * Note: if the name is invalid, we leave the hash code unchanged so
 * that the existing dentry can be used. The exfat fs routines will
 * return ENOENT or EINVAL as appropriate.
 */
static int __exfat_d_hashi(const struct dentry *dentry, struct qstr *qstr)
{
	struct nls_table *t = EXFAT_SB(dentry->d_sb)->nls_io;
	const unsigned char *name;
	unsigned int len;
	unsigned long hash;

	name = qstr->name;
	len = exfat_striptail_len(qstr);

	hash = __exfat_init_name_hash(dentry);
	while (len--)
		hash = partial_name_hash(nls_tolower(t, *name++), hash);
	qstr->hash = end_name_hash(hash);

	return 0;
}

/*
 * Case sensitive compare of two exfat names.
 */
static int __exfat_cmp(const struct dentry *dentry, unsigned int len,
		const char *str, const struct qstr *name)
{
	unsigned int alen, blen;

	/* A filename cannot end in '.' or we treat it like it has none */
	alen = exfat_striptail_len(name);
	blen = __exfat_striptail_len(len, str);
	if (alen == blen) {
		if (strncmp(name->name, str, alen) == 0)
			return 0;
	}
	return 1;
}

/*
 * Case insensitive compare of two exfat names.
 */
static int __exfat_cmpi(const struct dentry *dentry, unsigned int len,
		const char *str, const struct qstr *name)
{
	struct nls_table *t = EXFAT_SB(dentry->d_sb)->nls_io;
	unsigned int alen, blen;

	/* A filename cannot end in '.' or we treat it like it has none */
	alen = exfat_striptail_len(name);
	blen = __exfat_striptail_len(len, str);
	if (alen == blen) {
		if (nls_strnicmp(t, name->name, str, alen) == 0)
			return 0;
	}
	return 1;
}

static const struct dentry_operations exfat_dentry_ops = {
	.d_revalidate   = exfat_revalidate,
	.d_hash         = exfat_d_hash,
	.d_compare      = exfat_cmp,
};

static const struct dentry_operations exfat_ci_dentry_ops = {
	.d_revalidate   = exfat_revalidate_ci,
	.d_hash         = exfat_d_hashi,
	.d_compare      = exfat_cmpi,
};

static int exfat_file_mmap(struct file *file, struct vm_area_struct *vm_struct)
{
	return generic_file_mmap(file, vm_struct);
}

static int exfat_ioctl_volume_id(struct inode *dir)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);
	FS_INFO_T *fsi = &(sbi->fsi);

	return fsi->vol_id;
}

static long exfat_generic_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode = file_inode(filp);

	if (cmd != EXFAT_IOCTL_GET_VOLUME_ID)
		return -ENOTSUPP;

	return exfat_ioctl_volume_id(inode);
}

static int __exfat_getattr(struct inode *inode, struct kstat *stat)
{
	generic_fillattr(inode, stat);
	stat->blksize = EXFAT_SB(inode->i_sb)->fsi.cluster_size;

	return 0;
}

static void __exfat_writepage_end_io(struct bio *bio, int err)
{
	struct page *page = bio->bi_io_vec->bv_page;
	struct super_block *sb = page->mapping->host->i_sb;

	ASSERT(bio->bi_vcnt == 1); /* Single page endio */
	ASSERT(bio_data_dir(bio)); /* Write */

	if (err) {
		SetPageError(page);
		mapping_set_error(page->mapping, err);
	}

	end_page_writeback(page);
	bio_put(bio);

	// Update trace info.
	atomic_dec(&EXFAT_SB(sb)->stat_n_pages_queued);
}

static int __exfat_file_fsync(struct file *filp, loff_t start, loff_t end, int datasync)
{
	return __exfat_generic_file_fsync(filp, start, end, datasync);
}

static const struct file_operations exfat_dir_operations = {
	.llseek     = generic_file_llseek,
	.read       = generic_read_dir,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	.iterate    = exfat_iterate,
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0) */
	.readdir    = exfat_readdir,
#endif
	.fsync      = exfat_file_fsync,
	.unlocked_ioctl = exfat_generic_ioctl,
};

/* create a file */
static s32 __exfat_create(struct inode *inode, u8 *path, u8 mode, FILE_ID_T *fid)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	/* check the validity of pointer parameters */
	ASSERT(fid && path);

	if (unlikely(!strlen(path)))
		return -EINVAL;

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_create(inode, path, mode, fid);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

static int exfat_create_compat(struct inode *dir, struct dentry *dentry)
{
	struct super_block *sb = dir->i_sb;
	struct inode *inode;
	FILE_ID_T fid;
	loff_t i_pos;
	int err;

	lock_super(sb);

	err = __exfat_create(dir, (u8 *) dentry->d_name.name, FM_REGULAR, &fid);
	if (err)
		goto out;

	__lock_d_revalidate(dentry);

	INC_IVERSION(dir);
	dir->i_ctime = dir->i_mtime = dir->i_atime = current_time(dir);
	if (IS_DIRSYNC(dir))
		(void) exfat_sync_inode(dir);
	else
		mark_inode_dirty(dir);

	i_pos = exfat_make_i_pos(&fid);
	inode = exfat_build_inode(sb, &fid, i_pos);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out;
	}
	INC_IVERSION(inode);
	inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
	/* timestamp is already written, so mark_inode_dirty() is unneeded. */

	d_instantiate(dentry, inode);
out:
	__unlock_d_revalidate(dentry);
	unlock_super(sb);

	return err;
}


static int exfat_find(struct inode *dir, struct qstr *qname, FILE_ID_T *fid)
{
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	u8 *path = (u8 *)qname->name;
	int err;

	if (qname->len == 0)
		return -ENOENT;

	/* check the validity of pointer parameters */
	ASSERT(fid && path);

	if (unlikely(!strlen(path)))
		return -EINVAL;

	mutex_lock(&(sbi->s_vlock));
	err = exfat_fscore_lookup(dir, path, fid);
	mutex_unlock(&(sbi->s_vlock));
	if (err)
		return -ENOENT;

	return 0;
}

static int exfat_d_anon_disconn(struct dentry *dentry)
{
	return IS_ROOT(dentry) && (dentry->d_flags & DCACHE_DISCONNECTED);
}

static struct dentry *__exfat_lookup(struct inode *dir, struct dentry *dentry)
{
	struct super_block *sb = dir->i_sb;
	struct inode *inode;
	struct dentry *alias;
	int err;
	FILE_ID_T fid;
	loff_t i_pos;
	u64 ret;
	mode_t i_mode;

	lock_super(sb);
	err = exfat_find(dir, &dentry->d_name, &fid);
	if (err) {
		if (err == -ENOENT) {
			inode = NULL;
			goto out;
		}
		goto error;
	}

	i_pos = exfat_make_i_pos(&fid);
	inode = exfat_build_inode(sb, &fid, i_pos);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto error;
	}

	i_mode = inode->i_mode;
	if (S_ISLNK(i_mode) && !EXFAT_I(inode)->target) {
		EXFAT_I(inode)->target = kmalloc((i_size_read(inode)+1), GFP_KERNEL);
		if (!EXFAT_I(inode)->target) {
			err = -ENOMEM;
			goto error;
		}
		exfat_read_link(dir, &fid, EXFAT_I(inode)->target, i_size_read(inode), &ret);
		*(EXFAT_I(inode)->target + i_size_read(inode)) = '\0';
	}

	alias = d_find_alias(inode);

	/*
	 * Checking "alias->d_parent == dentry->d_parent" to make sure
	 * FS is not corrupted (especially double linked dir).
	 */
	if (alias && alias->d_parent == dentry->d_parent &&
	    !exfat_d_anon_disconn(alias)) {

		/*
		 * Unhashed alias is able to exist because of revalidate()
		 * called by lookup_fast. You can easily make this status
		 * by calling create and lookup concurrently
		 * In such case, we reuse an alias instead of new dentry
		 */
		if (d_unhashed(alias)) {
			exfat_msg(sb, KERN_INFO, "rehashed a dentry(%p) "
				"in read lookup", alias);
			d_drop(dentry);
			d_rehash(alias);
		} else if (!S_ISDIR(i_mode)) {
			/*
			 * This inode has non anonymous-DCACHE_DISCONNECTED
			 * dentry. This means, the user did ->lookup() by an
			 * another name (longname vs 8.3 alias of it) in past.
			 *
			 * Switch to new one for reason of locality if possible.
			 */
			d_move(alias, dentry);
		}
		iput(inode);
		unlock_super(sb);
		return alias;
	}
	dput(alias);
out:
	/* initialize d_time even though it is positive dentry */
	dentry->d_time = GET_IVERSION(dir);
	unlock_super(sb);

	dentry = d_splice_alias(inode, dentry);

	return dentry;

error:
	unlock_super(sb);
	return ERR_PTR(err);
}


static int exfat_unlink(struct inode *dir, struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	int err;

	lock_super(sb);

	EXFAT_I(inode)->fid.size = i_size_read(inode);

	/* check the validity of pointer parameters */
	ASSERT(fid);

	mutex_lock(&(sbi->s_vlock));
	err = exfat_fscore_unlink(dir, fid);
	mutex_unlock(&(sbi->s_vlock));
	if (err)
		goto out;

	__lock_d_revalidate(dentry);

	INC_IVERSION(dir);
	dir->i_mtime = dir->i_atime = current_time(dir);
	if (IS_DIRSYNC(dir))
		(void) exfat_sync_inode(dir);
	else
		mark_inode_dirty(dir);

	clear_nlink(inode);
	inode->i_mtime = inode->i_atime = current_time(inode);
	exfat_detach(inode);
	dentry->d_time = GET_IVERSION(dir);
out:
	__unlock_d_revalidate(dentry);
	unlock_super(sb);
	return err;
}

static int exfat_symlink(struct inode *dir, struct dentry *dentry, const char *target)
{
	struct super_block *sb = dir->i_sb;
	struct inode *inode;
	FILE_ID_T fid;
	loff_t i_pos;
	int err;
	u64 len = (u64) strlen(target);
	u64 ret;

	/* symlink option check */
	if (!EXFAT_SB(sb)->options.symlink)
		return -ENOTSUPP;

	lock_super(sb);

	err = __exfat_create(dir, (u8 *) dentry->d_name.name, FM_SYMLINK, &fid);
	if (err)
		goto out;

	err = exfat_write_link(dir, &fid, (char *) target, len, &ret);

	if (err) {
		exfat_remove(dir, &fid);
		goto out;
	}

	__lock_d_revalidate(dentry);

	INC_IVERSION(dir);
	dir->i_ctime = dir->i_mtime = dir->i_atime = current_time(dir);
	if (IS_DIRSYNC(dir))
		(void) exfat_sync_inode(dir);
	else
		mark_inode_dirty(dir);

	i_pos = exfat_make_i_pos(&fid);
	inode = exfat_build_inode(sb, &fid, i_pos);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out;
	}
	INC_IVERSION(inode);
	inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
	/* timestamp is already written, so mark_inode_dirty() is unneeded. */

	EXFAT_I(inode)->target = kmemdup(target, len + 1, GFP_KERNEL);
	if (!EXFAT_I(inode)->target) {
		err = -ENOMEM;
		goto out;
	}

	d_instantiate(dentry, inode);
out:
	__unlock_d_revalidate(dentry);
	unlock_super(sb);
	return err;
}

static int __exfat_mkdir(struct inode *dir, struct dentry *dentry)
{
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct inode *inode;
	FILE_ID_T fid;
	loff_t i_pos;
	u8 *path;
	int err;

	lock_super(sb);

	path = (u8 *) dentry->d_name.name;

	/* check the validity of pointer parameters */
	ASSERT(path);

	if (unlikely(!strlen(path))) {
		err = -EINVAL;
		goto out;
	}

	mutex_lock(&(sbi->s_vlock));
	err = exfat_fscore_mkdir(dir, path, &fid);
	mutex_unlock(&(sbi->s_vlock));
	if (err)
		goto out;

	__lock_d_revalidate(dentry);

	INC_IVERSION(dir);
	dir->i_ctime = dir->i_mtime = dir->i_atime = current_time(dir);
	if (IS_DIRSYNC(dir))
		(void) exfat_sync_inode(dir);
	else
		mark_inode_dirty(dir);
	inc_nlink(dir);

	i_pos = exfat_make_i_pos(&fid);
	inode = exfat_build_inode(sb, &fid, i_pos);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out;
	}
	INC_IVERSION(inode);
	inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
	/* timestamp is already written, so mark_inode_dirty() is unneeded. */

	d_instantiate(dentry, inode);

out:
	__unlock_d_revalidate(dentry);
	unlock_super(sb);

	return err;
}


static int exfat_rmdir(struct inode *dir, struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	int err;

	lock_super(sb);

	EXFAT_I(inode)->fid.size = i_size_read(inode);

	/* check the validity of pointer parameters */
	ASSERT(fid);

	mutex_lock(&(sbi->s_vlock));
	err = exfat_fscore_rmdir(inode, fid);
	mutex_unlock(&(sbi->s_vlock));
	if (err)
		goto out;

	__lock_d_revalidate(dentry);

	INC_IVERSION(dir);
	dir->i_mtime = dir->i_atime = current_time(dir);
	if (IS_DIRSYNC(dir))
		(void) exfat_sync_inode(dir);
	else
		mark_inode_dirty(dir);
	drop_nlink(dir);

	clear_nlink(inode);
	inode->i_mtime = inode->i_atime = current_time(inode);
	exfat_detach(inode);
	dentry->d_time = GET_IVERSION(dir);
out:
	__unlock_d_revalidate(dentry);
	unlock_super(sb);
	return err;
}

static int __exfat_rename(struct inode *old_dir, struct dentry *old_dentry,
		struct inode *new_dir, struct dentry *new_dentry)
{
	struct inode *old_inode, *new_inode;
	struct super_block *sb = old_dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	loff_t i_pos;
	int err;

	lock_super(sb);

	old_inode = old_dentry->d_inode;
	new_inode = new_dentry->d_inode;

	EXFAT_I(old_inode)->fid.size = i_size_read(old_inode);

	/* check the validity of pointer parameters */
	ASSERT(&(EXFAT_I(old_inode)->fid));

	mutex_lock(&(sbi->s_vlock));
	err = exfat_fscore_rename(old_dir, &(EXFAT_I(old_inode)->fid),
				new_dir, new_dentry);
	mutex_unlock(&(sbi->s_vlock));
	if (err)
		goto out;

	__lock_d_revalidate(old_dentry);
	__lock_d_revalidate(new_dentry);

	INC_IVERSION(new_dir);
	new_dir->i_ctime = new_dir->i_mtime = new_dir->i_atime = current_time(new_dir);
	if (IS_DIRSYNC(new_dir))
		(void) exfat_sync_inode(new_dir);
	else
		mark_inode_dirty(new_dir);

	i_pos = exfat_make_i_pos(&(EXFAT_I(old_inode)->fid));
	exfat_detach(old_inode);
	exfat_attach(old_inode, i_pos);
	if (IS_DIRSYNC(new_dir))
		(void) exfat_sync_inode(old_inode);
	else
		mark_inode_dirty(old_inode);

	if ((S_ISDIR(old_inode->i_mode)) && (old_dir != new_dir)) {
		drop_nlink(old_dir);
		if (!new_inode)
			inc_nlink(new_dir);
	}

	INC_IVERSION(old_dir);
	old_dir->i_ctime = old_dir->i_mtime = current_time(old_dir);
	if (IS_DIRSYNC(old_dir))
		(void) exfat_sync_inode(old_dir);
	else
		mark_inode_dirty(old_dir);

	if (new_inode) {
		exfat_detach(new_inode);

		/* skip drop_nlink if new_inode already has been dropped */
		if (new_inode->i_nlink) {
			drop_nlink(new_inode);
			if (S_ISDIR(new_inode->i_mode))
				drop_nlink(new_inode);
		} else {
			EMSG("%s : abnormal access to an inode dropped\n",
				__func__);
			WARN_ON(new_inode->i_nlink == 0);
		}
		new_inode->i_ctime = current_time(new_inode);
#if 0
		(void) exfat_sync_inode(new_inode);
#endif
	}

out:
	__unlock_d_revalidate(old_dentry);
	__unlock_d_revalidate(new_dentry);
	unlock_super(sb);
	return err;
}

static int exfat_cont_expand(struct inode *inode, loff_t size)
{
	struct address_space *mapping = inode->i_mapping;
	loff_t start = i_size_read(inode), count = size - i_size_read(inode);
	int err, err2;

	err = generic_cont_expand_simple(inode, size);
	if (err)
		return err;

	inode->i_ctime = inode->i_mtime = current_time(inode);
	mark_inode_dirty(inode);

	if (!IS_SYNC(inode))
		return 0;

	err = filemap_fdatawrite_range(mapping, start, start + count - 1);
	err2 = sync_mapping_buffers(mapping);
	err = (err)?(err):(err2);
	err2 = write_inode_now(inode, 1);
	err = (err)?(err):(err2);
	if (err)
		return err;

	return filemap_fdatawait_range(mapping, start, start + count - 1);
}

static int exfat_allow_set_time(struct exfat_sb_info *sbi, struct inode *inode)
{
	mode_t allow_utime = sbi->options.allow_utime;

	if (!uid_eq(current_fsuid(), inode->i_uid)) {
		if (in_group_p(inode->i_gid))
			allow_utime >>= 3;
		if (allow_utime & MAY_WRITE)
			return 1;
	}

	/* use a default check */
	return 0;
}

static int exfat_sanitize_mode(const struct exfat_sb_info *sbi,
				   struct inode *inode, umode_t *mode_ptr)
{
	mode_t i_mode, mask, perm;

	i_mode = inode->i_mode;

	if (S_ISREG(i_mode) || S_ISLNK(i_mode))
		mask = sbi->options.fs_fmask;
	else
		mask = sbi->options.fs_dmask;

	perm = *mode_ptr & ~(S_IFMT | mask);

	/* Of the r and x bits, all (subject to umask) must be present.*/
	if ((perm & (S_IRUGO | S_IXUGO)) != (i_mode & (S_IRUGO | S_IXUGO)))
		return -EPERM;

	if (exfat_mode_can_hold_ro(inode)) {
		/* Of the w bits, either all (subject to umask) or none must be present. */
		if ((perm & S_IWUGO) && ((perm & S_IWUGO) != (S_IWUGO & ~mask)))
			return -EPERM;
	} else {
		/* If exfat_mode_can_hold_ro(inode) is false, can't change w bits. */
		if ((perm & S_IWUGO) != (S_IWUGO & ~mask))
			return -EPERM;
	}

	*mode_ptr &= S_IFMT | perm;

	return 0;
}

static int exfat_setattr(struct dentry *dentry, struct iattr *attr)
{

	struct exfat_sb_info *sbi = EXFAT_SB(dentry->d_sb);
	struct inode *inode = dentry->d_inode;
	unsigned int ia_valid;
	int error;
	loff_t old_size;

	if ((attr->ia_valid & ATTR_SIZE)
		&& (attr->ia_size > i_size_read(inode))) {
		error = exfat_cont_expand(inode, attr->ia_size);
		if (error || attr->ia_valid == ATTR_SIZE)
			goto out;
		attr->ia_valid &= ~ATTR_SIZE;
	}

	/* Check for setting the inode time. */
	ia_valid = attr->ia_valid;
	if ((ia_valid & (ATTR_MTIME_SET | ATTR_ATIME_SET | ATTR_TIMES_SET))
		&& exfat_allow_set_time(sbi, inode)) {
		attr->ia_valid &= ~(ATTR_MTIME_SET | ATTR_ATIME_SET | ATTR_TIMES_SET);
	}

	error = setattr_prepare(dentry, attr);
	attr->ia_valid = ia_valid;
	if (error) {
		if (sbi->options.quiet)
			error = 0;
		goto out;
	}

	if (((attr->ia_valid & ATTR_UID) &&
		 (!uid_eq(attr->ia_uid, sbi->options.fs_uid))) ||
		((attr->ia_valid & ATTR_GID) &&
		 (!gid_eq(attr->ia_gid, sbi->options.fs_gid))) ||
		((attr->ia_valid & ATTR_MODE) &&
		 (attr->ia_mode & ~(S_IFREG | S_IFLNK | S_IFDIR | S_IRWXUGO)))) {
		error = -EPERM;
	}

	if (error) {
		if (sbi->options.quiet)
			error = 0;
		goto out;
	}

	/*
	 * We don't return -EPERM here. Yes, strange, but this is too
	 * old behavior.
	 */
	if (attr->ia_valid & ATTR_MODE) {
		if (exfat_sanitize_mode(sbi, inode, &attr->ia_mode) < 0)
			attr->ia_valid &= ~ATTR_MODE;
	}

	EXFAT_I(inode)->fid.size = i_size_read(inode);

	/* patch 1.2.0 : fixed the problem of size mismatch. */
	if (attr->ia_valid & ATTR_SIZE) {
		old_size = i_size_read(inode);

		/* TO CHECK evicting directory works correctly */
		MMSG("%s: inode(%p) truncate size (%llu->%llu)\n", __func__,
			inode, (u64)old_size, (u64)attr->ia_size);
		__exfat_do_truncate(inode, old_size, attr->ia_size);
	}
	setattr_copy(inode, attr);
	mark_inode_dirty(inode);

out:
	return error;
}

static const struct inode_operations exfat_dir_inode_operations = {
	.create        = exfat_create,
	.lookup        = exfat_lookup,
	.unlink        = exfat_unlink,
	.symlink       = exfat_symlink,
	.mkdir         = exfat_mkdir,
	.rmdir         = exfat_rmdir,
	.rename        = exfat_rename,
	.setattr       = exfat_setattr,
	.getattr       = exfat_getattr,
#ifdef CONFIG_EXFAT_VIRTUAL_XATTR
	.listxattr      = exfat_listxattr,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0)
	.setxattr       = exfat_setxattr,
	.getxattr       = exfat_getxattr,
	.removexattr    = exfat_removexattr,
#endif
#endif
};

static const struct inode_operations exfat_symlink_inode_operations = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
	.readlink    = generic_readlink,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)
	.get_link = exfat_follow_link,
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0) */
	.follow_link = exfat_follow_link,
#endif
#ifdef CONFIG_EXFAT_VIRTUAL_XATTR
	.listxattr      = exfat_listxattr,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0)
	.setxattr       = exfat_setxattr,
	.getxattr       = exfat_getxattr,
	.removexattr    = exfat_removexattr,
#endif
#endif
};

static int exfat_file_release(struct inode *inode, struct file *filp)
{
	struct super_block *sb = inode->i_sb;

	/* Moved below code from exfat_write_inode
	 * TO FIX size-mismatch problem.
	 */
	/* FIXME : Added bug_on to confirm that there is no size mismatch */
	exfat_debug_bug_on(EXFAT_I(inode)->fid.size != i_size_read(inode));
	EXFAT_I(inode)->fid.size = i_size_read(inode);
	__exfat_sync_fs(sb, 0);
	return 0;
}

static const struct file_operations exfat_file_operations = {
	.llseek      = generic_file_llseek,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
	.read_iter   = generic_file_read_iter,
	.write_iter  = generic_file_write_iter,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	.read        = new_sync_read,
	.write       = new_sync_write,
	.read_iter   = generic_file_read_iter,
	.write_iter  = generic_file_write_iter,
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0) */
	.read        = do_sync_read,
	.write       = do_sync_write,
	.aio_read    = generic_file_aio_read,
	.aio_write   = generic_file_aio_write,
#endif
	.mmap        = exfat_file_mmap,
	.release     = exfat_file_release,
	.unlocked_ioctl  = exfat_generic_ioctl,
	.fsync       = exfat_file_fsync,
	.splice_read = generic_file_splice_read,
};

static const struct address_space_operations exfat_aops;

/* resize the file length */
static s32 __exfat_truncate(struct inode *inode, u64 old_size, u64 new_size)
{
	s32 err;
	struct super_block *sb = inode->i_sb;

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_truncate(inode, old_size, new_size);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

static void exfat_truncate(struct inode *inode, loff_t old_size)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	FS_INFO_T *fsi = &(sbi->fsi);
	unsigned int blocksize = 1 << inode->i_blkbits;
	loff_t aligned_size;
	int err;

	lock_super(sb);

	if (EXFAT_I(inode)->fid.start_clu == 0) {
		/* Stange statement:
		 * Empty start_clu != ~0 (not allocated)
		 */
		exfat_fs_error(sb, "tried to truncate zeroed cluster.");
		goto out;
	}

	err = __exfat_truncate(inode, old_size, i_size_read(inode));
	if (err)
		goto out;

	inode->i_ctime = inode->i_mtime = current_time(inode);
	if (IS_DIRSYNC(inode))
		(void) exfat_sync_inode(inode);
	else
		mark_inode_dirty(inode);

	// FIXME: Please check
	// inode->i_blocks = ((EXFAT_I(inode)->i_size_ondisk + (fsi->cluster_size - 1))
	inode->i_blocks = ((i_size_read(inode) + (fsi->cluster_size - 1)) &
			~((loff_t)fsi->cluster_size - 1)) >> inode->i_blkbits;
out:
	/*
	 * This protects against truncating a file bigger than it was then
	 * trying to write into the hole.
	 *
	 * comment by sh.hong:
	 * This seems to mean 'intra page/block' truncate and writing.
	 * I couldn't find a reason to change the values prior to __exfat_truncate
	 * Therefore, I switched the order of operations
	 * so that it's possible to utilize i_size_ondisk in __exfat_truncate
	 */

	aligned_size = i_size_read(inode);
	if (aligned_size & (blocksize - 1)) {
		aligned_size |= (blocksize - 1);
		aligned_size++;
	}

	if (EXFAT_I(inode)->i_size_ondisk > i_size_read(inode))
		EXFAT_I(inode)->i_size_ondisk = aligned_size;

	if (EXFAT_I(inode)->i_size_aligned > i_size_read(inode))
		EXFAT_I(inode)->i_size_aligned = aligned_size;

	/* After truncation :
	 * 1) Delayed allocation is OFF
	 *    i_size = i_size_ondisk <= i_size_aligned
	 *    (useless size var.)
	 *    (block-aligned)
	 * 2) Delayed allocation is ON
	 *    i_size = i_size_ondisk = i_size_aligned
	 *    (will be block-aligned after write)
	 *    or
	 *    i_size_ondisk < i_size <= i_size_aligned (block_aligned)
	 *    (will be block-aligned after write)
	 */

	unlock_super(sb);
}

static const struct inode_operations exfat_file_inode_operations = {
	.setattr     = exfat_setattr,
	.getattr     = exfat_getattr,
#ifdef CONFIG_EXFAT_VIRTUAL_XATTR
	.listxattr      = exfat_listxattr,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0)
	.setxattr       = exfat_setxattr,
	.getxattr       = exfat_getxattr,
	.removexattr    = exfat_removexattr,
#endif
#endif
};

/* 2-level option flag */
#define BMAP_NOT_CREATE				0
#define BMAP_ADD_BLOCK				1
#define BMAP_ADD_CLUSTER			2
#define BLOCK_ADDED(bmap_ops)	(bmap_ops)
static int exfat_bmap(struct inode *inode, sector_t sector, sector_t *phys,
				  unsigned long *mapped_blocks, int *create)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	FS_INFO_T *fsi = &(sbi->fsi);
	const unsigned long blocksize = sb->s_blocksize;
	const unsigned char blocksize_bits = sb->s_blocksize_bits;
	sector_t last_block;
	unsigned int cluster, clu_offset, sec_offset;
	int err = 0;

	*phys = 0;
	*mapped_blocks = 0;

	/* core code should handle EIO */
#if 0
	if (fsi->prev_eio && BLOCK_ADDED(*create))
		return -EIO;
#endif

	last_block = (i_size_read(inode) + (blocksize - 1)) >> blocksize_bits;
	if ((sector >= last_block) && (*create == BMAP_NOT_CREATE))
		return 0;

	/* Is this block already allocated? */
	clu_offset = sector >> fsi->sect_per_clus_bits;  /* cluster offset */

	EXFAT_I(inode)->fid.size = i_size_read(inode);

	if (*create & BMAP_ADD_CLUSTER)
		err = exfat_map_clus(inode, clu_offset, &cluster, 1);
	else
		err = exfat_map_clus(inode, clu_offset, &cluster, ALLOC_NOWHERE);

	if (err) {
		if (err != -ENOSPC)
			return -EIO;
		return err;
	}

	if (!IS_CLUS_EOF(cluster)) {
		/* sector offset in cluster */
		sec_offset = sector & (fsi->sect_per_clus - 1);

		*phys = CLUS_TO_SECT(fsi, cluster) + sec_offset;
		*mapped_blocks = fsi->sect_per_clus - sec_offset;
	}
#if 0
	else {
		/* Debug purpose (new clu needed) */
		ASSERT((*create & BMAP_ADD_CLUSTER) == 0);
		ASSERT(sector >= last_block);
	}
#endif

	if (sector < last_block)
		*create = BMAP_NOT_CREATE;
#if 0
	else if (sector >= last_block)
		*create = non-zero;

	if (iblock <= last mapped-block)
		*phys != 0
		*create = BMAP_NOT_CREATE
	else if (iblock <= last cluster)
		*phys != 0
		*create = non-zero
#endif
	return 0;
}

static int exfat_get_block(struct inode *inode, sector_t iblock,
				   struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	unsigned long max_blocks = bh_result->b_size >> inode->i_blkbits;
	int err = 0;
	unsigned long mapped_blocks;
	sector_t phys;
	loff_t pos;
	int bmap_create = create ? BMAP_ADD_CLUSTER : BMAP_NOT_CREATE;

	lock_super(sb);
	err = exfat_bmap(inode, iblock, &phys, &mapped_blocks, &bmap_create);
	if (err) {
		if (err != -ENOSPC)
			exfat_fs_error_ratelimit(sb, "%s: failed to bmap "
					"(inode:%p iblock:%u, err:%d)",
					__func__, inode, (u32)iblock, err);
		goto unlock_ret;
	}

	if (phys) {
		max_blocks = min(mapped_blocks, max_blocks);

		/* Treat newly added block / cluster */
		if (BLOCK_ADDED(bmap_create) || buffer_delay(bh_result)) {

			/* Update i_size_ondisk */
			pos = (iblock + 1) << sb->s_blocksize_bits;
			if (EXFAT_I(inode)->i_size_ondisk < pos) {
				/* Debug purpose */
				if ((pos - EXFAT_I(inode)->i_size_ondisk) > bh_result->b_size) {
					/* This never happens without DA */
					MMSG("Jumping get_block\n");
				}

				EXFAT_I(inode)->i_size_ondisk = pos;
			}

			if (BLOCK_ADDED(bmap_create)) {
				/* Old way (w/o DA)
				 * create == 1 only if iblock > i_size
				 * (in block unit)
				 */

				/* 20130723 CHECK
				 * If happened concurrently with truncation,
				 * buffer_delay() can be left on whilst
				 * i_size < (position of i_block).
				 *
				 * It's not a big deal as we're re-writing
				 * previously allocated blocks, but in such cases,
				 * it has to be an area with i_size_aligned expanded
				 * beforehand.
				 */

				/* FOR GRACEFUL ERROR HANDLING */
				if (buffer_delay(bh_result) &&
					(pos > EXFAT_I(inode)->i_size_aligned)) {
					exfat_fs_error(sb, "requested for bmap "
						"out of range(pos:(%llu)>i_size_aligned(%llu)\n",
						pos, EXFAT_I(inode)->i_size_aligned);
					exfat_debug_bug_on(1);
					err = -EIO;
					goto unlock_ret;
				}
				set_buffer_new(bh_result);

				/*
				 * adjust i_size_aligned if i_size_ondisk is
				 * bigger than it. (i.e. non-DA)
				 */
				if (EXFAT_I(inode)->i_size_ondisk >
					EXFAT_I(inode)->i_size_aligned) {
					EXFAT_I(inode)->i_size_aligned =
						EXFAT_I(inode)->i_size_ondisk;
				}
			}

			if (buffer_delay(bh_result))
				clear_buffer_delay(bh_result);

#if 0
			/* Debug purpose */
			if (EXFAT_I(inode)->i_size_ondisk >
					EXFAT_I(inode)->i_size_aligned) {
				/* Only after truncate
				 * and the two size variables should indicate
				 * same i_block
				 */
				unsigned int blocksize = 1 << inode->i_blkbits;
				BUG_ON(EXFAT_I(inode)->i_size_ondisk -
					EXFAT_I(inode)->i_size_aligned >= blocksize);
			}
#endif
		}
		map_bh(bh_result, sb, phys);
	}

	bh_result->b_size = max_blocks << sb->s_blocksize_bits;
unlock_ret:
	unlock_super(sb);
	return err;
}

static int exfat_readpage(struct file *file, struct page *page)
{
	int ret;

	ret =  mpage_readpage(page, exfat_get_block);
	return ret;
}

static int exfat_readpages(struct file *file, struct address_space *mapping,
			   struct list_head *pages, unsigned int nr_pages)
{
	int ret;

	ret =  mpage_readpages(mapping, pages, nr_pages, exfat_get_block);
	return ret;
}

static inline void exfat_submit_fullpage_bio(struct block_device *bdev,
			sector_t sector, unsigned int length, struct page *page)
{
	/* Single page bio submit */
	struct bio *bio;

	BUG_ON((length > PAGE_SIZE) || (length == 0));

	/*
	 * If __GFP_WAIT is set, then bio_alloc will always be able to allocate
	 * a bio. This is due to the mempool guarantees. To make this work, callers
	 * must never allocate more than 1 bio at a time from this pool.
	 *
	 * #define GFP_NOIO	(__GFP_WAIT)
	 */
	bio = bio_alloc(GFP_NOIO, 1);

	bio_set_dev(bio, bdev);
	bio->bi_vcnt = 1;
	bio->bi_io_vec[0].bv_page = page;	/* Inline vec */
	bio->bi_io_vec[0].bv_len = length;	/* PAGE_SIZE */
	bio->bi_io_vec[0].bv_offset = 0;
	__exfat_set_bio_iterate(bio, sector, length, 0, 0);

	bio->bi_end_io = exfat_writepage_end_io;
	__exfat_submit_bio_write(bio);
}

static int exfat_writepage(struct page *page, struct writeback_control *wbc)
{
	struct inode * const inode = page->mapping->host;
	struct super_block *sb = inode->i_sb;
	loff_t i_size = i_size_read(inode);
	const pgoff_t end_index = i_size >> PAGE_SHIFT;
	const unsigned int blocks_per_page = PAGE_SIZE >> inode->i_blkbits;
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	struct buffer_head *bh, *head;
	sector_t block, block_0, last_phys;
	int ret;
	unsigned int nr_blocks_towrite = blocks_per_page;

	/* Don't distinguish 0-filled/clean block.
	 * Just write back the whole page
	 */
	if (fsi->cluster_size < PAGE_SIZE)
		goto confused;

	if (!PageUptodate(page)) {
		MMSG("%s: Not up-to-date page -> block_write_full_page\n",
				__func__);
		goto confused;
	}

	if (page->index >= end_index) {
		/* last page or outside i_size */
		unsigned int offset = i_size & (PAGE_SIZE-1);

		/* If a truncation is in progress */
		if (page->index > end_index || !offset)
			goto confused;

		/* 0-fill after i_size */
		zero_user_segment(page, offset, PAGE_SIZE);
	}

	if (!page_has_buffers(page)) {
		MMSG("WP: No buffers -> block_write_full_page\n");
		goto confused;
	}

	block = (sector_t)page->index << (PAGE_SHIFT - inode->i_blkbits);
	block_0 = block; /* first block */
	head = page_buffers(page);
	bh = head;

	last_phys = 0;
	do {
		BUG_ON(buffer_locked(bh));

		if (!buffer_dirty(bh) || !buffer_uptodate(bh)) {
			if (nr_blocks_towrite == blocks_per_page)
				nr_blocks_towrite = (unsigned int) (block - block_0);

			BUG_ON(nr_blocks_towrite >= blocks_per_page);

			// !uptodate but dirty??
			if (buffer_dirty(bh))
				goto confused;

			// Nothing to writeback in this block
			bh = bh->b_this_page;
			block++;
			continue;
		}

		if (nr_blocks_towrite != blocks_per_page)
			// Dirty -> Non-dirty -> Dirty again case
			goto confused;

		/* Map if needed */
		if (!buffer_mapped(bh) || buffer_delay(bh)) {
			BUG_ON(bh->b_size != (1 << (inode->i_blkbits)));
			ret = exfat_get_block(inode, block, bh, 1);
			if (ret)
				goto confused;

			if (buffer_new(bh)) {
				clear_buffer_new(bh);
				__exfat_clean_bdev_aliases(bh->b_bdev, bh->b_blocknr);
			}
		}

		/* continuity check */
		if (((last_phys + 1) != bh->b_blocknr) && (last_phys != 0)) {
			DMSG("Non-contiguous block mapping in single page");
			goto confused;
		}

		last_phys = bh->b_blocknr;
		bh = bh->b_this_page;
		block++;
	} while (bh != head);

	if (nr_blocks_towrite == 0) {
		DMSG("Page dirty but no dirty bh? alloc_208\n");
		goto confused;
	}


	/* Write-back */
	do {
		clear_buffer_dirty(bh);
		bh = bh->b_this_page;
	} while (bh != head);

	BUG_ON(PageWriteback(page));
	set_page_writeback(page);

	// Trace # of pages queued (Approx.)
	atomic_inc(&EXFAT_SB(sb)->stat_n_pages_queued);

	exfat_submit_fullpage_bio(head->b_bdev,
		head->b_blocknr << (sb->s_blocksize_bits - SECTOR_SIZE_BITS),
		nr_blocks_towrite << inode->i_blkbits,
		page);

	unlock_page(page);

	return 0;

confused:
	ret = block_write_full_page(page, exfat_get_block, wbc);
	return ret;
}

static int exfat_writepages(struct address_space *mapping,
				struct writeback_control *wbc)
{
	MMSG("%s(inode:%p) with nr_to_write = 0x%08lx "
		"(ku %d, bg %d, tag %d, rc %d )\n",
		__func__, mapping->host, wbc->nr_to_write,
		wbc->for_kupdate, wbc->for_background, wbc->tagged_writepages,
		wbc->for_reclaim);

	ASSERT(mapping->a_ops == &exfat_aops);

	return mpage_writepages(mapping, wbc, exfat_get_block);
}

static void exfat_write_failed(struct address_space *mapping, loff_t to)
{
	struct inode *inode = mapping->host;

	if (to > i_size_read(inode)) {
		__exfat_truncate_pagecache(inode, to, i_size_read(inode));
		exfat_truncate(inode, EXFAT_I(inode)->i_size_aligned);
	}
}

static int exfat_check_writable(struct super_block *sb)
{
	if (exfat_fscore_check_bdi_valid(sb))
		return -EIO;

	if (EXFAT_IS_SB_RDONLY(sb))
		return -EROFS;

	return 0;
}

static int __exfat_write_begin(struct file *file, struct address_space *mapping,
				 loff_t pos, unsigned int len,
				 unsigned int flags, struct page **pagep,
				 void **fsdata, get_block_t *get_block,
				 loff_t *bytes, const char *fname)
{
	struct super_block *sb = mapping->host->i_sb;
	int ret;

	ret = exfat_check_writable(sb);
	if (unlikely(ret < 0))
		return ret;

	*pagep = NULL;
	ret = cont_write_begin(file, mapping, pos, len, flags, pagep, fsdata,
					get_block, bytes);

	if (ret < 0)
		exfat_write_failed(mapping, pos+len);

	return ret;
}

static int exfat_write_begin(struct file *file, struct address_space *mapping,
				 loff_t pos, unsigned int len, unsigned int flags,
				 struct page **pagep, void **fsdata)
{
	return __exfat_write_begin(file, mapping, pos, len, flags,
				pagep, fsdata, exfat_get_block,
				&EXFAT_I(mapping->host)->i_size_ondisk,
				__func__);
}

static int exfat_write_end(struct file *file, struct address_space *mapping,
				   loff_t pos, unsigned int len, unsigned int copied,
				   struct page *pagep, void *fsdata)
{
	struct inode *inode = mapping->host;
	FILE_ID_T *fid = &(EXFAT_I(inode)->fid);
	int err;

	err = generic_write_end(file, mapping, pos, len, copied, pagep, fsdata);

	/* FOR GRACEFUL ERROR HANDLING */
	if (EXFAT_I(inode)->i_size_aligned < i_size_read(inode)) {
		exfat_fs_error(inode->i_sb, "invalid size(size(%llu) "
			"> aligned(%llu)\n", i_size_read(inode),
			EXFAT_I(inode)->i_size_aligned);
		exfat_debug_bug_on(1);
	}

	if (err < len)
		exfat_write_failed(mapping, pos+len);

	if (!(err < 0) && !(fid->attr & ATTR_ARCHIVE)) {
		inode->i_mtime = inode->i_ctime = current_time(inode);
		fid->attr |= ATTR_ARCHIVE;
		mark_inode_dirty(inode);
	}

	return err;
}

static inline ssize_t __exfat_direct_IO(int rw, struct kiocb *iocb,
		struct inode *inode, void *iov_u, loff_t offset,
		loff_t count, unsigned long nr_segs)
{
	struct address_space *mapping = inode->i_mapping;
	loff_t size = offset + count;
	ssize_t ret;

	if (rw == WRITE) {
		/*
		 * FIXME: blockdev_direct_IO() doesn't use ->write_begin(),
		 * so we need to update the ->i_size_aligned to block boundary.
		 *
		 * But we must fill the remaining area or hole by nul for
		 * updating ->i_size_aligned
		 *
		 * Return 0, and fallback to normal buffered write.
		 */
		if (EXFAT_I(inode)->i_size_aligned < size)
			return 0;
	}

	/*
	 * exFAT need to use the DIO_LOCKING for avoiding the race
	 * condition of exfat_get_block() and ->truncate().
	 */
	ret = __exfat_blkdev_direct_IO(rw, iocb, inode, iov_u, offset, nr_segs);
	if (ret < 0 && (rw & WRITE))
		exfat_write_failed(mapping, size);

	return ret;
}

static const struct address_space_operations exfat_aops = {
	.readpage    = exfat_readpage,
	.readpages   = exfat_readpages,
	.writepage   = exfat_writepage,
	.writepages  = exfat_writepages,
	.write_begin = exfat_write_begin,
	.write_end   = exfat_write_end,
	.direct_IO   = exfat_direct_IO,
	.bmap        = exfat_aop_bmap
};

static inline unsigned long exfat_hash(loff_t i_pos)
{
	return hash_32(i_pos, EXFAT_HASH_BITS);
}

static void exfat_attach(struct inode *inode, loff_t i_pos)
{
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);
	struct hlist_head *head = sbi->inode_hashtable + exfat_hash(i_pos);

	spin_lock(&sbi->inode_hash_lock);
	EXFAT_I(inode)->i_pos = i_pos;
	hlist_add_head(&EXFAT_I(inode)->i_hash_fat, head);
	spin_unlock(&sbi->inode_hash_lock);
}

static void exfat_detach(struct inode *inode)
{
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);

	spin_lock(&sbi->inode_hash_lock);
	hlist_del_init(&EXFAT_I(inode)->i_hash_fat);
	EXFAT_I(inode)->i_pos = 0;
	spin_unlock(&sbi->inode_hash_lock);
}


/* doesn't deal with root inode */
static int exfat_fill_inode(struct inode *inode, const FILE_ID_T *fid)
{
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);
	FS_INFO_T *fsi = &(sbi->fsi);
	DIR_ENTRY_T info;
	u64 size = fid->size;

	memcpy(&(EXFAT_I(inode)->fid), fid, sizeof(FILE_ID_T));

	EXFAT_I(inode)->i_pos = 0;
	EXFAT_I(inode)->target = NULL;
	inode->i_uid = sbi->options.fs_uid;
	inode->i_gid = sbi->options.fs_gid;
	INC_IVERSION(inode);
	inode->i_generation = get_seconds();

	if (exfat_read_inode(inode, &info) < 0) {
		MMSG("%s: failed to read stat!\n", __func__);
		return -EIO;
	}

	if (info.Attr & ATTR_SUBDIR) { /* directory */
		inode->i_generation &= ~1;
		inode->i_mode = exfat_make_mode(sbi, info.Attr, S_IRWXUGO);
		inode->i_op = &exfat_dir_inode_operations;
		inode->i_fop = &exfat_dir_operations;

		set_nlink(inode, info.NumSubdirs);
	} else if (info.Attr & ATTR_SYMLINK) { /* symbolic link */
		inode->i_op = &exfat_symlink_inode_operations;
		inode->i_generation |= 1;
		inode->i_mode = exfat_make_mode(sbi, info.Attr, S_IRWXUGO);
	} else { /* regular file */
		inode->i_generation |= 1;
		inode->i_mode = exfat_make_mode(sbi, info.Attr, S_IRWXUGO);
		inode->i_op = &exfat_file_inode_operations;
		inode->i_fop = &exfat_file_operations;
		inode->i_mapping->a_ops = &exfat_aops;

		inode->i_mapping->nrpages = 0;

	}

	/*
	 * Use fid->size instead of info.Size
	 * because info.Size means the value saved on disk
	 */
	i_size_write(inode, size);

	/* ondisk and aligned size should be aligned with block size */
	if (size & (inode->i_sb->s_blocksize - 1)) {
		size |= (inode->i_sb->s_blocksize - 1);
		size++;
	}

	EXFAT_I(inode)->i_size_aligned = size;
	EXFAT_I(inode)->i_size_ondisk = size;

	exfat_save_attr(inode, info.Attr);

	inode->i_blocks = ((i_size_read(inode) + (fsi->cluster_size - 1))
		   & ~((loff_t)fsi->cluster_size - 1)) >> inode->i_blkbits;

	exfat_time_fat2unix(sbi, &inode->i_mtime, &info.ModifyTimestamp);
	exfat_time_fat2unix(sbi, &inode->i_ctime, &info.CreateTimestamp);
	exfat_time_fat2unix(sbi, &inode->i_atime, &info.AccessTimestamp);

	return 0;
}

static struct inode *exfat_build_inode(struct super_block *sb,
				   const FILE_ID_T *fid, loff_t i_pos)
{
	struct inode *inode;
	int err;

	inode = exfat_iget(sb, i_pos);
	if (inode)
		goto out;
	inode = new_inode(sb);
	if (!inode) {
		inode = ERR_PTR(-ENOMEM);
		goto out;
	}
	inode->i_ino = iunique(sb, EXFAT_ROOT_INO);
	SET_IVERSION(inode, 1);
	err = exfat_fill_inode(inode, fid);
	if (err) {
		iput(inode);
		inode = ERR_PTR(err);
		goto out;
	}
	exfat_attach(inode, i_pos);
	insert_inode_hash(inode);
out:
	return inode;
}

static struct inode *exfat_alloc_inode(struct super_block *sb)
{
	struct exfat_inode_info *ei;

	ei = kmem_cache_alloc(exfat_inode_cachep, GFP_NOFS);
	if (!ei)
		return NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
	init_rwsem(&ei->truncate_lock);
#endif
	return &ei->vfs_inode;
}

static void exfat_destroy_inode(struct inode *inode)
{
	kfree(EXFAT_I(inode)->target);
	EXFAT_I(inode)->target = NULL;

	kmem_cache_free(exfat_inode_cachep, EXFAT_I(inode));
}

static int __exfat_write_inode(struct inode *inode, int sync)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	DIR_ENTRY_T info;
	s32 err;

	if (inode->i_ino == EXFAT_ROOT_INO)
		return 0;

	info.Attr = exfat_make_attr(inode);
	info.Size = i_size_read(inode);

	exfat_time_unix2fat(sbi, &inode->i_mtime, &info.ModifyTimestamp);
	exfat_time_unix2fat(sbi, &inode->i_ctime, &info.CreateTimestamp);
	exfat_time_unix2fat(sbi, &inode->i_atime, &info.AccessTimestamp);

	/* FIXME : Do we need handling error? */
	mutex_lock(&(sbi->s_vlock));
	err = exfat_fscore_write_inode(inode, &info, sync);
	mutex_unlock(&(sbi->s_vlock));

	return err;
}

static int exfat_sync_inode(struct inode *inode)
{
	return __exfat_write_inode(inode, 1);
}

static int exfat_write_inode(struct inode *inode, struct writeback_control *wbc)
{
	return __exfat_write_inode(inode, wbc->sync_mode == WB_SYNC_ALL);
}

static void exfat_evict_inode(struct inode *inode)
{
	truncate_inode_pages(&inode->i_data, 0);

	if (!inode->i_nlink) {
		loff_t old_size = i_size_read(inode);

		i_size_write(inode, 0);

		EXFAT_I(inode)->fid.size = old_size;

		/* TO CHECK evicting directory works correctly */
		MMSG("%s: inode(%p) evict %s (size(%llu) to zero)\n",
				__func__, inode,
				S_ISDIR(inode->i_mode) ? "directory" : "file",
				(u64)old_size);
		__exfat_truncate(inode, old_size, 0);
	}

	invalidate_inode_buffers(inode);
	clear_inode(inode);
	exfat_extent_cache_inval_inode(inode);
	exfat_detach(inode);

	/* after end of this function, caller will remove inode hash */
	/* remove_inode_hash(inode); */
}



static void exfat_put_super(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	int err;

	exfat_log_msg(sb, KERN_INFO, "trying to unmount...");

	__cancel_delayed_work_sync(sbi);

	if (__is_sb_dirty(sb))
		exfat_write_super(sb);

	err = exfat_umount(sb);

	if (sbi->nls_disk) {
		unload_nls(sbi->nls_disk);
		sbi->nls_disk = NULL;
		sbi->options.codepage = exfat_default_codepage;
	}
	if (sbi->nls_io) {
		unload_nls(sbi->nls_io);
		sbi->nls_io = NULL;
	}
	if (sbi->options.iocharset != exfat_default_iocharset) {
		kfree(sbi->options.iocharset);
		sbi->options.iocharset = exfat_default_iocharset;
	}

	sb->s_fs_info = NULL;

	kobject_del(&sbi->sb_kobj);
	kobject_put(&sbi->sb_kobj);
	if (!sbi->use_vmalloc)
		kfree(sbi);
	else
		vfree(sbi);

	exfat_log_msg(sb, KERN_INFO, "unmounted successfully! %s",
				err ? "(with previous I/O errors)" : "");
}

static inline void __flush_delayed_meta(struct super_block *sb, s32 sync)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	if (sbi->options.delayed_meta)
		exfat_cache_flush(sb, sync);
}

static void exfat_write_super(struct super_block *sb)
{
	int time = 0;

	lock_super(sb);

	__set_sb_clean(sb);

	/* flush delayed FAT/DIR dirty */
	__flush_delayed_meta(sb, 0);

	if (!EXFAT_IS_SB_RDONLY(sb))
		__exfat_sync_fs(sb, 0);

	unlock_super(sb);

	time = jiffies;

	/* Issuing bdev requests is needed
	 * to guarantee DIR updates in time
	 * whether w/ or w/o delayed DIR dirty feature.
	 * (otherwise DIR updates could be delayed for 5 + 5 secs at max.)
	 */
	sync_blockdev(sb->s_bdev);

	MMSG("BD: exfat_write_super (bdev_sync for %ld ms)\n",
			(jiffies - time) * 1000 / HZ);
}

/* synchronize a file system volume */
static s32 __exfat_sync_fs(struct super_block *sb, s32 do_sync)
{
	s32 err;

	mutex_lock(&(EXFAT_SB(sb)->s_vlock));
	err = exfat_fscore_sync_fs(sb, do_sync);
	mutex_unlock(&(EXFAT_SB(sb)->s_vlock));
	return err;
}

static int exfat_sync_fs(struct super_block *sb, int wait)
{
	int err = 0;

	/* If there are some dirty buffers in the bdev inode */
	if (__is_sb_dirty(sb)) {
		lock_super(sb);
		__set_sb_clean(sb);

		err = __exfat_sync_fs(sb, 1);

		unlock_super(sb);
	}

	return err;
}

static int exfat_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	/*
	 * patch 1.2.2 :
	 * fixed the slow-call problem because of volume-lock contention.
	 */
	struct super_block *sb = dentry->d_sb;
	u64 id = huge_encode_dev(sb->s_bdev->bd_dev);
	FS_INFO_T *fsi = &(EXFAT_SB(sb)->fsi);
	VOL_INFO_T info;

	/* exfat_statfs will try to get a volume lock if needed */
	if (fsi->used_clusters == UINT_MAX) {
		s32 err;

		mutex_lock(&(EXFAT_SB(sb)->s_vlock));
		err = exfat_fscore_statfs(sb, &info);
		mutex_unlock(&(EXFAT_SB(sb)->s_vlock));

		return -EIO;
	}

	info.ClusterSize = fsi->cluster_size;
	info.NumClusters = fsi->num_clusters - 2; /* clu 0 & 1 */
	info.UsedClusters = fsi->used_clusters + fsi->reserved_clusters;
	info.FreeClusters = info.NumClusters - info.UsedClusters;

	if (fsi->prev_eio)
		exfat_msg(sb, KERN_INFO, "called statfs with previous"
				" I/O error(0x%02X).", fsi->prev_eio);

	buf->f_type = sb->s_magic;
	buf->f_bsize = info.ClusterSize;
	buf->f_blocks = info.NumClusters;
	buf->f_bfree = info.FreeClusters;
	buf->f_bavail = info.FreeClusters;
	buf->f_fsid.val[0] = (u32)id;
	buf->f_fsid.val[1] = (u32)(id >> 32);
	buf->f_namelen = 260;

	return 0;
}

static int exfat_remount(struct super_block *sb, int *flags, char *data)
{
	unsigned long prev_sb_flags;
	char *orig_data = kstrdup(data, GFP_KERNEL);
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	FS_INFO_T *fsi = &(sbi->fsi);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
	*flags |= MS_NODIRATIME;
#else
	*flags |= SB_NODIRATIME;
#endif

	prev_sb_flags = sb->s_flags;

	exfat_remount_syncfs(sb);

	exfat_set_vol_flags(sb, VOL_CLEAN, 1);

	exfat_log_msg(sb, KERN_INFO, "re-mounted(%s->%s), eio=0x%x, Opts: %s",
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
		(prev_sb_flags & MS_RDONLY) ? "ro" : "rw",
		(*flags & MS_RDONLY) ? "ro" : "rw",
#else
		(prev_sb_flags & SB_RDONLY) ? "ro" : "rw",
		(*flags & SB_RDONLY) ? "ro" : "rw",
#endif
		fsi->prev_eio, orig_data);
	kfree(orig_data);
	return 0;
}

static int __exfat_show_options(struct seq_file *m, struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_mount_options *opts = &sbi->options;
	FS_INFO_T *fsi = &(sbi->fsi);

	/* Show partition info */
	if (fsi->prev_eio)
		seq_printf(m, ",eio=0x%x", fsi->prev_eio);
	if (!uid_eq(opts->fs_uid, GLOBAL_ROOT_UID))
		seq_printf(m, ",uid=%u",
				from_kuid_munged(&init_user_ns, opts->fs_uid));
	if (!gid_eq(opts->fs_gid, GLOBAL_ROOT_GID))
		seq_printf(m, ",gid=%u",
				from_kgid_munged(&init_user_ns, opts->fs_gid));
	seq_printf(m, ",fmask=%04o", opts->fs_fmask);
	seq_printf(m, ",dmask=%04o", opts->fs_dmask);
	if (opts->allow_utime)
		seq_printf(m, ",allow_utime=%04o", opts->allow_utime);
	if (sbi->nls_disk)
		seq_printf(m, ",codepage=%s", sbi->nls_disk->charset);
	if (sbi->nls_io)
		seq_printf(m, ",iocharset=%s", sbi->nls_io->charset);
	if (opts->quiet)
		seq_puts(m, ",quiet");
	if (opts->utf8)
		seq_puts(m, ",utf8");
	seq_printf(m, ",namecase=%u", opts->casesensitive);
	if (opts->tz_utc)
		seq_puts(m, ",tz=UTC");
	seq_printf(m, ",symlink=%u", opts->symlink);
	seq_printf(m, ",bps=%ld", sb->s_blocksize);
	if (opts->errors == EXFAT_ERRORS_CONT)
		seq_puts(m, ",errors=continue");
	else if (opts->errors == EXFAT_ERRORS_PANIC)
		seq_puts(m, ",errors=panic");
	else
		seq_puts(m, ",errors=remount-ro");
	if (opts->discard)
		seq_puts(m, ",discard");
	if (opts->delayed_meta)
		seq_puts(m, ",delayed_meta");

	return 0;
}

static const struct super_operations exfat_sops = {
	.alloc_inode   = exfat_alloc_inode,
	.destroy_inode = exfat_destroy_inode,
	.write_inode   = exfat_write_inode,
	.evict_inode  = exfat_evict_inode,
	.put_super     = exfat_put_super,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
	.write_super   = exfat_write_super,
#endif
	.sync_fs       = exfat_sync_fs,
	.statfs        = exfat_statfs,
	.remount_fs    = exfat_remount,
	.show_options  = exfat_show_options,
};

#define EXFAT_ATTR(name, mode, show, store) \
static struct exfat_attr exfat_attr_##name = __ATTR(name, mode, show, store)

struct exfat_attr {
	struct attribute attr;
	ssize_t (*show)(struct exfat_sb_info *, char *);
	ssize_t (*store)(struct exfat_sb_info *, const char *, size_t);
};

static ssize_t exfat_attr_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
	struct exfat_sb_info *sbi = container_of(kobj, struct exfat_sb_info, sb_kobj);
	struct exfat_attr *a = container_of(attr, struct exfat_attr, attr);

	return a->show ? a->show(sbi, buf) : 0;
}

static ssize_t exfat_attr_store(struct kobject *kobj, struct attribute *attr,
				const char *buf, size_t len)
{
	struct exfat_sb_info *sbi = container_of(kobj, struct exfat_sb_info, sb_kobj);
	struct exfat_attr *a = container_of(attr, struct exfat_attr, attr);

	return a->store ? a->store(sbi, buf, len) : len;
}

static const struct sysfs_ops exfat_attr_ops = {
	.show  = exfat_attr_show,
	.store = exfat_attr_store,
};

static ssize_t eio_show(struct exfat_sb_info *sbi, char *buf)
{
	FS_INFO_T *fsi = &(sbi->fsi);

	return snprintf(buf, PAGE_SIZE, "0x%x\n", fsi->prev_eio);
}
EXFAT_ATTR(eio, 0444, eio_show, NULL);

static ssize_t fratio_show(struct exfat_sb_info *sbi, char *buf)
{
	unsigned int n_total_au = 0;
	unsigned int n_clean_au = 0;
	unsigned int n_full_au = 0;
	unsigned int n_dirty_au = 0;
	unsigned int fr = 0;

	n_total_au = exfat_fscore_get_au_stat(sbi->host_sb, VOL_AU_STAT_TOTAL);
	n_clean_au = exfat_fscore_get_au_stat(sbi->host_sb, VOL_AU_STAT_CLEAN);
	n_full_au = exfat_fscore_get_au_stat(sbi->host_sb, VOL_AU_STAT_FULL);
	n_dirty_au = n_total_au - (n_full_au + n_clean_au);

	if (!n_dirty_au)
		fr = 0;
	else if (!n_clean_au)
		fr = 100;
	else
		fr = (n_dirty_au * 100) / (n_clean_au + n_dirty_au);

	return snprintf(buf, PAGE_SIZE, "%u\n", fr);
}
EXFAT_ATTR(fratio, 0444, fratio_show, NULL);

static ssize_t totalau_show(struct exfat_sb_info *sbi, char *buf)
{
	unsigned int n_au = 0;

	n_au = exfat_fscore_get_au_stat(sbi->host_sb, VOL_AU_STAT_TOTAL);
	return snprintf(buf, PAGE_SIZE, "%u\n", n_au);
}
EXFAT_ATTR(totalau, 0444, totalau_show, NULL);

static ssize_t cleanau_show(struct exfat_sb_info *sbi, char *buf)
{
	unsigned int n_clean_au = 0;

	n_clean_au = exfat_fscore_get_au_stat(sbi->host_sb, VOL_AU_STAT_CLEAN);
	return snprintf(buf, PAGE_SIZE, "%u\n", n_clean_au);
}
EXFAT_ATTR(cleanau, 0444, cleanau_show, NULL);

static ssize_t fullau_show(struct exfat_sb_info *sbi, char *buf)
{
	unsigned int n_full_au = 0;

	n_full_au = exfat_fscore_get_au_stat(sbi->host_sb, VOL_AU_STAT_FULL);
	return snprintf(buf, PAGE_SIZE, "%u\n", n_full_au);
}
EXFAT_ATTR(fullau, 0444, fullau_show, NULL);

static struct attribute *exfat_attrs[] = {
	&exfat_attr_eio.attr,
	&exfat_attr_fratio.attr,
	&exfat_attr_totalau.attr,
	&exfat_attr_cleanau.attr,
	&exfat_attr_fullau.attr,
	NULL,
};

static struct kobj_type exfat_ktype = {
	.default_attrs = exfat_attrs,
	.sysfs_ops     = &exfat_attr_ops,
};

static ssize_t version_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buff)
{
	return snprintf(buff, PAGE_SIZE, "FS Version %s\n", EXFAT_VERSION);
}

static struct kobj_attribute version_attr = __ATTR_RO(version);

static struct attribute *attributes[] = {
	&version_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attributes,
};

enum {
	Opt_uid,
	Opt_gid,
	Opt_umask,
	Opt_dmask,
	Opt_fmask,
	Opt_allow_utime,
	Opt_codepage,
	Opt_charset,
	Opt_quiet,
	Opt_utf8,
	Opt_namecase,
	Opt_tz_utc,
	Opt_symlink,
	Opt_err_cont,
	Opt_err_panic,
	Opt_err_ro,
	Opt_err,
	Opt_discard,
	Opt_delayed_meta,
	Opt_nodelayed_meta,
};

static const match_table_t exfat_tokens = {
	{Opt_uid, "uid=%u"},
	{Opt_gid, "gid=%u"},
	{Opt_umask, "umask=%o"},
	{Opt_dmask, "dmask=%o"},
	{Opt_fmask, "fmask=%o"},
	{Opt_allow_utime, "allow_utime=%o"},
	{Opt_codepage, "codepage=%u"},
	{Opt_charset, "iocharset=%s"},
	{Opt_quiet, "quiet"},
	{Opt_utf8, "utf8"},
	{Opt_namecase, "namecase=%u"},
	{Opt_tz_utc, "tz=UTC"},
	{Opt_symlink, "symlink=%u"},
	{Opt_err_cont, "errors=continue"},
	{Opt_err_panic, "errors=panic"},
	{Opt_err_ro, "errors=remount-ro"},
	{Opt_discard, "discard"},
	{Opt_delayed_meta, "delayed_meta"},
	{Opt_nodelayed_meta, "nodelayed_meta"},
	{Opt_err, NULL}
};

static int parse_options(struct super_block *sb, char *options, int silent,
			 struct exfat_mount_options *opts)
{
	char *p;
	substring_t args[MAX_OPT_ARGS];
	int option;
	char *tmpstr;

	opts->fs_uid = current_uid();
	opts->fs_gid = current_gid();
	opts->fs_fmask = opts->fs_dmask = current->fs->umask;
	opts->allow_utime = U16_MAX;
	opts->codepage = exfat_default_codepage;
	opts->iocharset = exfat_default_iocharset;
	opts->quiet = 0;
	opts->casesensitive = 0;
	opts->utf8 = 0;
	opts->tz_utc = 0;
	opts->symlink = 0;
	opts->errors = EXFAT_ERRORS_RO;
	opts->discard = 0;
	opts->delayed_meta = 1;

	if (!options)
		goto out;

	while ((p = strsep(&options, ",")) != NULL) {
		int token;

		if (!*p)
			continue;
		token = match_token(p, exfat_tokens, args);
		switch (token) {
		case Opt_uid:
			if (match_int(&args[0], &option))
				return 0;
			opts->fs_uid = make_kuid(current_user_ns(), option);
			break;
		case Opt_gid:
			if (match_int(&args[0], &option))
				return 0;
			opts->fs_gid = make_kgid(current_user_ns(), option);
			break;
		case Opt_umask:
		case Opt_dmask:
		case Opt_fmask:
			if (match_octal(&args[0], &option))
				return 0;
			if (token != Opt_dmask)
				opts->fs_fmask = option;
			if (token != Opt_fmask)
				opts->fs_dmask = option;
			break;
		case Opt_allow_utime:
			if (match_octal(&args[0], &option))
				return 0;
			opts->allow_utime = option & (S_IWGRP | S_IWOTH);
			break;
		case Opt_codepage:
			if (match_int(&args[0], &option))
				return 0;
			opts->codepage = option;
			break;
		case Opt_charset:
			if (opts->iocharset != exfat_default_iocharset)
				kfree(opts->iocharset);
			tmpstr = match_strdup(&args[0]);
			if (!tmpstr)
				return -ENOMEM;
			opts->iocharset = tmpstr;
			break;
		case Opt_quiet:
			opts->quiet = 1;
			break;
		case Opt_namecase:
			if (match_int(&args[0], &option))
				return 0;
			opts->casesensitive = (option > 0) ? 1:0;
			break;
		case Opt_utf8:
			opts->utf8 = 1;
			break;
		case Opt_tz_utc:
			opts->tz_utc = 1;
			break;
		case Opt_symlink:
			if (match_int(&args[0], &option))
				return 0;
			opts->symlink = option > 0 ? 1 : 0;
			break;
		case Opt_err_cont:
			opts->errors = EXFAT_ERRORS_CONT;
			break;
		case Opt_err_panic:
			opts->errors = EXFAT_ERRORS_PANIC;
			break;
		case Opt_err_ro:
			opts->errors = EXFAT_ERRORS_RO;
			break;
		case Opt_discard:
			opts->discard = 1;
			break;
		case Opt_nodelayed_meta:
			opts->delayed_meta = 0;
			break;
		default:
			if (!silent) {
				exfat_msg(sb, KERN_ERR,
					"unrecognized mount option \"%s\" "
					"or missing value", p);
			}
			return -EINVAL;
		}
	}

out:
	if (opts->allow_utime == U16_MAX)
		opts->allow_utime = ~opts->fs_dmask & (S_IWGRP | S_IWOTH);

	if (opts->utf8 && strcmp(opts->iocharset,  exfat_iocharset_with_utf8)) {
		exfat_msg(sb, KERN_WARNING,
			"utf8 enabled, \"iocharset=%s\" is recommended",
			exfat_iocharset_with_utf8);
	}

	if (opts->discard) {
		struct request_queue *q = bdev_get_queue(sb->s_bdev);

		if (!blk_queue_discard(q))
			exfat_msg(sb, KERN_WARNING,
				"mounting with \"discard\" option, but "
				"the device does not support discard");
		opts->discard = 0;
	}

	return 0;
}

static void exfat_hash_init(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	int i;

	spin_lock_init(&sbi->inode_hash_lock);
	for (i = 0; i < EXFAT_HASH_SIZE; i++)
		INIT_HLIST_HEAD(&sbi->inode_hashtable[i]);
}

static int exfat_read_root(struct inode *inode)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	FS_INFO_T *fsi = &(sbi->fsi);
	DIR_ENTRY_T info;

	EXFAT_I(inode)->fid.dir.dir = fsi->root_dir;
	EXFAT_I(inode)->fid.dir.flags = 0x01;
	EXFAT_I(inode)->fid.entry = -1;
	EXFAT_I(inode)->fid.start_clu = fsi->root_dir;
	EXFAT_I(inode)->fid.flags = 0x01;
	EXFAT_I(inode)->fid.type = TYPE_DIR;
	EXFAT_I(inode)->fid.version = 0;
	EXFAT_I(inode)->fid.rwoffset = 0;
	EXFAT_I(inode)->fid.hint_bmap.off = CLUS_EOF;
	EXFAT_I(inode)->fid.hint_stat.eidx = 0;
	EXFAT_I(inode)->fid.hint_stat.clu = fsi->root_dir;
	EXFAT_I(inode)->fid.hint_femp.eidx = -1;

	EXFAT_I(inode)->target = NULL;

	if (exfat_read_inode(inode, &info) < 0)
		return -EIO;

	inode->i_uid = sbi->options.fs_uid;
	inode->i_gid = sbi->options.fs_gid;
	INC_IVERSION(inode);
	inode->i_generation = 0;
	inode->i_mode = exfat_make_mode(sbi, ATTR_SUBDIR, S_IRWXUGO);
	inode->i_op = &exfat_dir_inode_operations;
	inode->i_fop = &exfat_dir_operations;

	i_size_write(inode, info.Size);
	EXFAT_I(inode)->fid.size = info.Size;
	inode->i_blocks = ((i_size_read(inode) + (fsi->cluster_size - 1))
		& ~((loff_t)fsi->cluster_size - 1)) >> inode->i_blkbits;
	EXFAT_I(inode)->i_pos = ((loff_t) fsi->root_dir << 32) | 0xffffffff;
	EXFAT_I(inode)->i_size_aligned = i_size_read(inode);
	EXFAT_I(inode)->i_size_ondisk = i_size_read(inode);

	exfat_save_attr(inode, ATTR_SUBDIR);
	inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
	set_nlink(inode, info.NumSubdirs + 2);
	return 0;
}

static void setup_dops(struct super_block *sb)
{
	if (EXFAT_SB(sb)->options.casesensitive == 0)
		sb->s_d_op = &exfat_ci_dentry_ops;
	else
		sb->s_d_op = &exfat_dentry_ops;
}

static int exfat_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root_inode = NULL;
	struct exfat_sb_info *sbi;
	int err;
	char buf[50];
	struct block_device *bdev = sb->s_bdev;
	dev_t bd_dev = bdev ? bdev->bd_dev : 0;

	exfat_log_msg(sb, KERN_INFO, "trying to mount...");

	/*
	 * GFP_KERNEL is ok here, because while we do hold the
	 * supeblock lock, memory pressure can't call back into
	 * the filesystem, since we're only just about to mount
	 * it and have no inodes etc active!
	 */
	sbi = kzalloc(sizeof(struct exfat_sb_info), GFP_KERNEL);
	if (!sbi) {
		exfat_log_msg(sb, KERN_INFO,
			"trying to alloc sbi with vzalloc()");
		sbi = vzalloc(sizeof(struct exfat_sb_info));
		if (!sbi) {
			exfat_log_msg(sb, KERN_ERR, "failed to mount! (ENOMEM)");
			return -ENOMEM;
		}
		sbi->use_vmalloc = 1;
	}

	mutex_init(&sbi->s_vlock);
	sb->s_fs_info = sbi;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
	sb->s_flags |= MS_NODIRATIME;
#else
	sb->s_flags |= SB_NODIRATIME;
#endif
	sb->s_magic = EXFAT_SUPER_MAGIC;
	sb->s_op = &exfat_sops;
	ratelimit_state_init(&sbi->ratelimit, DEFAULT_RATELIMIT_INTERVAL,
						DEFAULT_RATELIMIT_BURST);
	err = parse_options(sb, data, silent, &sbi->options);
	if (err) {
		exfat_log_msg(sb, KERN_ERR, "failed to parse options");
		goto failed_mount;
	}

	setup_exfat_xattr_handler(sb);
	setup_exfat_sync_super_wq(sb);
	setup_dops(sb);

	err = exfat_mount(sb);
	if (err) {
		exfat_log_msg(sb, KERN_ERR, "failed to recognize fat type");
		goto failed_mount;
	}

	/* set up enough so that it can read an inode */
	exfat_hash_init(sb);

	/*
	 * The low byte of FAT's first entry must have same value with
	 * media-field.  But in real world, too many devices is
	 * writing wrong value.  So, removed that validity check.
	 *
	 * if (FAT_FIRST_ENT(sb, media) != first)
	 */

	err = -EINVAL;
	sprintf(buf, "cp%d", sbi->options.codepage);
	sbi->nls_disk = load_nls(buf);
	if (!sbi->nls_disk) {
		exfat_log_msg(sb, KERN_ERR, "codepage %s not found", buf);
		goto failed_mount2;
	}

	sbi->nls_io = load_nls(sbi->options.iocharset);
	if (!sbi->nls_io) {
		exfat_log_msg(sb, KERN_ERR, "IO charset %s not found",
				sbi->options.iocharset);
		goto failed_mount2;
	}

	err = -ENOMEM;
	root_inode = new_inode(sb);
	if (!root_inode) {
		exfat_log_msg(sb, KERN_ERR, "failed to allocate root inode.");
		goto failed_mount2;
	}

	root_inode->i_ino = EXFAT_ROOT_INO;
	SET_IVERSION(root_inode, 1);

	err = exfat_read_root(root_inode);
	if (err) {
		exfat_log_msg(sb, KERN_ERR, "failed to initialize root inode.");
		goto failed_mount2;
	}

	exfat_attach(root_inode, EXFAT_I(root_inode)->i_pos);
	insert_inode_hash(root_inode);

	err = -ENOMEM;
	sb->s_root = __d_make_root(root_inode);
	if (!sb->s_root) {
		exfat_msg(sb, KERN_ERR, "failed to get the root dentry");
		goto failed_mount2;
	}

	/*
	 * Initialize filesystem attributes (for sysfs)
	 * ex: /sys/fs/exfat/mmcblk1[179:17]
	 */
	sbi->sb_kobj.kset = exfat_kset;
	err = kobject_init_and_add(&sbi->sb_kobj, &exfat_ktype, NULL,
			"%s[%d:%d]", sb->s_id, MAJOR(bd_dev), MINOR(bd_dev));
	if (err) {
		exfat_msg(sb, KERN_ERR, "Unable to create exfat attributes for"
					" %s[%d:%d](%d)", sb->s_id,
					MAJOR(bd_dev), MINOR(bd_dev), err);
		goto failed_mount2;
	}

	exfat_log_msg(sb, KERN_INFO, "mounted successfully!");

	return 0;

failed_mount2:
	exfat_umount(sb);
failed_mount:
	exfat_log_msg(sb, KERN_INFO, "failed to mount! (%d)", err);

	if (root_inode)
		iput(root_inode);
	sb->s_root = NULL;

	if (sbi->nls_io)
		unload_nls(sbi->nls_io);
	if (sbi->nls_disk)
		unload_nls(sbi->nls_disk);
	if (sbi->options.iocharset != exfat_default_iocharset)
		kfree(sbi->options.iocharset);
	sb->s_fs_info = NULL;
	if (!sbi->use_vmalloc)
		kfree(sbi);
	else
		vfree(sbi);
	return err;
}

static struct dentry *exfat_fs_mount(struct file_system_type *fs_type,
				 int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, exfat_fill_super);
}

static void init_once(void *foo)
{
	struct exfat_inode_info *ei = (struct exfat_inode_info *)foo;

	INIT_HLIST_NODE(&ei->i_hash_fat);
	inode_init_once(&ei->vfs_inode);
}

static int __init exfat_init_inodecache(void)
{
	exfat_inode_cachep = kmem_cache_create("exfat_inode_cache",
				sizeof(struct exfat_inode_info),
				0, (SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD),
				init_once);
	if (!exfat_inode_cachep)
		return -ENOMEM;
	return 0;
}

static void exfat_destroy_inodecache(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(exfat_inode_cachep);
}

static struct file_system_type exfat_fs_type = {
	.owner       = THIS_MODULE,
	.name        = "exfat",
	.mount       = exfat_fs_mount,
	.kill_sb    = kill_block_super,
	.fs_flags    = FS_REQUIRES_DEV,
};

static int __init init_exfat_fs(void)
{
	int err;

	exfat_log_version();
	err = exfat_fscore_init();
	if (err)
		goto error;

	exfat_kset = kset_create_and_add("exfat", NULL, fs_kobj);
	if (!exfat_kset) {
		pr_err("exFAT: failed to create fs_kobj\n");
		err = -ENOMEM;
		goto error;
	}

	err = sysfs_create_group(&exfat_kset->kobj, &attr_group);
	if (err) {
		pr_err("exFAT: failed to create exfat version attributes\n");
		goto error;
	}

	err = exfat_uevent_init(exfat_kset);
	if (err)
		goto error;

	err = exfat_init_inodecache();
	if (err) {
		pr_err("exFAT: failed to initialize inode cache\n");
		goto error;
	}

	err = register_filesystem(&exfat_fs_type);
	if (err) {
		pr_err("exFAT: failed to register filesystem\n");
		goto error;
	}

	return 0;
error:
	exfat_uevent_uninit();

	if (exfat_kset) {
		sysfs_remove_group(&exfat_kset->kobj, &attr_group);
		kset_unregister(exfat_kset);
		exfat_kset = NULL;
	}

	exfat_destroy_inodecache();
	exfat_fscore_shutdown();

	pr_err("exFAT: failed to initialize FS driver(err:%d)\n", err);
	return err;
}

static void __exit exit_exfat_fs(void)
{
	exfat_uevent_uninit();

	if (exfat_kset) {
		sysfs_remove_group(&exfat_kset->kobj, &attr_group);
		kset_unregister(exfat_kset);
		exfat_kset = NULL;
	}

	exfat_destroy_inodecache();
	unregister_filesystem(&exfat_fs_type);

	exfat_fscore_shutdown();
}

module_init(init_exfat_fs);
module_exit(exit_exfat_fs);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
MODULE_ALIAS_FS("exfat");
#endif
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("FAT/exFAT filesystem support");
MODULE_AUTHOR("Samsung Electronics Co., Ltd.");
