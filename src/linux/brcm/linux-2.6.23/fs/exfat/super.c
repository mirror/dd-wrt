// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 */

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#include <linux/fs_context.h>
#include <linux/fs_parser.h>
#else
#include <linux/cred.h>
#include <linux/parser.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/mount.h>
#include <linux/cred.h>
#include <linux/statfs.h>
#include <linux/seq_file.h>
#include <linux/blkdev.h>
#include <linux/fs_struct.h>
#include <linux/nls.h>
#include <linux/buffer_head.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <linux/aio.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
#include <linux/iversion.h>
#endif
#include <linux/slab.h>

#include "exfat_raw.h"
#include "exfat_fs.h"

#ifndef CONFIG_EXFAT_DEFAULT_IOCHARSET /* if Kconfig lacked iocharset */
#define CONFIG_EXFAT_DEFAULT_IOCHARSET  "utf8"
#endif

static char exfat_default_iocharset[] = CONFIG_EXFAT_DEFAULT_IOCHARSET;
static struct kmem_cache *exfat_inode_cachep;

static void exfat_free_iocharset(struct exfat_sb_info *sbi)
{
	if (sbi->options.iocharset != exfat_default_iocharset)
		kfree(sbi->options.iocharset);
}

static void exfat_delayed_free(struct rcu_head *p)
{
	struct exfat_sb_info *sbi = container_of(p, struct exfat_sb_info, rcu);

	unload_nls(sbi->nls_io);
	exfat_free_iocharset(sbi);
	exfat_free_upcase_table(sbi);
	kfree(sbi);
}

static void exfat_put_super(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	mutex_lock(&sbi->s_lock);
	exfat_free_bitmap(sbi);
	brelse(sbi->boot_bh);
	mutex_unlock(&sbi->s_lock);

	call_rcu(&sbi->rcu, exfat_delayed_free);
}

static int exfat_sync_fs(struct super_block *sb, int wait)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	int err = 0;

	if (!wait)
		return 0;

	/* If there are some dirty buffers in the bdev inode */
	mutex_lock(&sbi->s_lock);
	sync_blockdev(sb->s_bdev);
	if (exfat_set_vol_flags(sb, VOL_CLEAN))
		err = -EIO;
	mutex_unlock(&sbi->s_lock);
	return err;
}

static int exfat_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	struct super_block *sb = dentry->d_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	unsigned long long id = huge_encode_dev(sb->s_bdev->bd_dev);

	if (sbi->used_clusters == EXFAT_CLUSTERS_UNTRACKED) {
		mutex_lock(&sbi->s_lock);
		if (exfat_count_used_clusters(sb, &sbi->used_clusters)) {
			mutex_unlock(&sbi->s_lock);
			return -EIO;
		}
		mutex_unlock(&sbi->s_lock);
	}

	buf->f_type = sb->s_magic;
	buf->f_bsize = sbi->cluster_size;
	buf->f_blocks = sbi->num_clusters - 2; /* clu 0 & 1 */
	buf->f_bfree = buf->f_blocks - sbi->used_clusters;
	buf->f_bavail = buf->f_bfree;
	buf->f_fsid.val[0] = (unsigned int)id;
	buf->f_fsid.val[1] = (unsigned int)(id >> 32);
	/* Unicode utf16 255 characters */
	buf->f_namelen = EXFAT_MAX_FILE_LEN * NLS_MAX_CHARSET_SIZE;
	return 0;
}

int exfat_set_vol_flags(struct super_block *sb, unsigned short new_flag)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct boot_sector *p_boot = (struct boot_sector *)sbi->boot_bh->b_data;
	bool sync;

	/* flags are not changed */
	if (sbi->vol_flag == new_flag)
		return 0;

	sbi->vol_flag = new_flag;

	/* skip updating volume dirty flag,
	 * if this volume has been mounted with read-only
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	if (sb_rdonly(sb))
#else
	if (sb->s_flags & MS_RDONLY)
#endif
		return 0;

	p_boot->vol_flags = cpu_to_le16(new_flag);

	if (new_flag == VOL_DIRTY && !buffer_dirty(sbi->boot_bh))
		sync = true;
	else
		sync = false;

	set_buffer_uptodate(sbi->boot_bh);
	mark_buffer_dirty(sbi->boot_bh);

	if (sync)
		sync_dirty_buffer(sbi->boot_bh);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
static int exfat_show_options(struct seq_file *m, struct dentry *root)
{
	struct exfat_sb_info *sbi = EXFAT_SB(root->d_sb);
#else
static int exfat_show_options(struct seq_file *m, struct vfsmount *mnt)
{
	struct exfat_sb_info *sbi = EXFAT_SB(mnt->mnt_sb);
#endif
	struct exfat_mount_options *opts = &sbi->options;

	/* Show partition info */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	if (!uid_eq(opts->fs_uid, GLOBAL_ROOT_UID))
		seq_printf(m, ",uid=%u", from_kuid_munged(&init_user_ns, opts->fs_uid));
	if (!gid_eq(opts->fs_gid, GLOBAL_ROOT_GID))
		seq_printf(m, ",gid=%u", from_kgid_munged(&init_user_ns, opts->fs_gid));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
    if (__kuid_val(opts->fs_uid))
		seq_printf(m, ",uid=%u", from_kuid_munged(&init_user_ns, opts->fs_uid));
	if (__kgid_val(opts->fs_gid))
		seq_printf(m, ",gid=%u", from_kgid_munged(&init_user_ns, opts->fs_gid));
#else
    if (opts->fs_uid != 0)
		seq_printf(m, ",uid=%u", opts->fs_uid);
	if (opts->fs_gid != 0)
		seq_printf(m, ",gid=%u", opts->fs_gid);
#endif
    
	seq_printf(m, ",fmask=%04o,dmask=%04o", opts->fs_fmask, opts->fs_dmask);
	if (opts->allow_utime)
		seq_printf(m, ",allow_utime=%04o", opts->allow_utime);
	if (opts->utf8)
		seq_puts(m, ",iocharset=utf8");
	else if (sbi->nls_io)
		seq_printf(m, ",iocharset=%s", sbi->nls_io->charset);
	if (opts->errors == EXFAT_ERRORS_CONT)
		seq_puts(m, ",errors=continue");
	else if (opts->errors == EXFAT_ERRORS_PANIC)
		seq_puts(m, ",errors=panic");
	else
		seq_puts(m, ",errors=remount-ro");
	if (opts->discard)
		seq_puts(m, ",discard");
	if (opts->time_offset)
		seq_printf(m, ",time_offset=%d", opts->time_offset);
	return 0;
}

static struct inode *exfat_alloc_inode(struct super_block *sb)
{
	struct exfat_inode_info *ei;

	ei = kmem_cache_alloc(exfat_inode_cachep, GFP_NOFS);
	if (!ei)
		return NULL;

	init_rwsem(&ei->truncate_lock);
	return &ei->vfs_inode;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static void exfat_free_inode(struct inode *inode)
{
	kmem_cache_free(exfat_inode_cachep, EXFAT_I(inode));
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
static void exfat_i_callback(struct rcu_head *head)
{
	struct inode *inode = container_of(head, struct inode, i_rcu);
	kmem_cache_free(exfat_inode_cachep, EXFAT_I(inode));
}

static void exfat_destroy_inode(struct inode *inode)
{
	call_rcu(&inode->i_rcu, exfat_i_callback);
}
#else
static void exfat_destroy_inode(struct inode *inode)
{
	kmem_cache_free(exfat_inode_cachep, EXFAT_I(inode));
}
#endif

static int exfat_remount(struct super_block *sb, int *flags, char *data)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	*flags |= SB_NODIRATIME;
#else
	*flags |= MS_NODIRATIME;
#endif

	sync_filesystem(sb);
	return 0;
}

static const struct super_operations exfat_sops = {
	.alloc_inode	= exfat_alloc_inode,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
	.free_inode	= exfat_free_inode,
#else
	.destroy_inode	= exfat_destroy_inode,
	.remount_fs	= exfat_remount,
#endif
	.write_inode	= exfat_write_inode,
	.evict_inode	= exfat_evict_inode,
	.put_super	= exfat_put_super,
	.sync_fs	= exfat_sync_fs,
	.statfs		= exfat_statfs,
	.show_options	= exfat_show_options,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
enum {
	Opt_uid,
	Opt_gid,
	Opt_umask,
	Opt_dmask,
	Opt_fmask,
	Opt_allow_utime,
	Opt_charset,
	Opt_errors,
	Opt_discard,
	Opt_time_offset,

	/* Deprecated options */
	Opt_utf8,
	Opt_debug,
	Opt_namecase,
	Opt_codepage,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct constant_table exfat_param_enums[] = {
	{ "continue",		EXFAT_ERRORS_CONT },
	{ "panic",		EXFAT_ERRORS_PANIC },
	{ "remount-ro",		EXFAT_ERRORS_RO },
	{}
};
#else
static const struct fs_parameter_enum exfat_param_enums[] = {
	{ Opt_errors,	"continue",		EXFAT_ERRORS_CONT },
	{ Opt_errors,	"panic",		EXFAT_ERRORS_PANIC },
	{ Opt_errors,	"remount-ro",		EXFAT_ERRORS_RO },
	{}
};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct fs_parameter_spec exfat_parameters[] = {
#else
static const struct fs_parameter_spec exfat_param_specs[] = {
#endif
	fsparam_u32("uid",			Opt_uid),
	fsparam_u32("gid",			Opt_gid),
	fsparam_u32oct("umask",			Opt_umask),
	fsparam_u32oct("dmask",			Opt_dmask),
	fsparam_u32oct("fmask",			Opt_fmask),
	fsparam_u32oct("allow_utime",		Opt_allow_utime),
	fsparam_string("iocharset",		Opt_charset),
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	fsparam_enum("errors",			Opt_errors, exfat_param_enums),
#else
	fsparam_enum("errors",			Opt_errors),
#endif
	fsparam_flag("discard",			Opt_discard),
	fsparam_s32("time_offset",		Opt_time_offset),
	__fsparam(NULL, "utf8",			Opt_utf8, fs_param_deprecated,
		  NULL),
	__fsparam(NULL, "debug",		Opt_debug, fs_param_deprecated,
		  NULL),
	__fsparam(fs_param_is_u32, "namecase",	Opt_namecase,
		  fs_param_deprecated, NULL),
	__fsparam(fs_param_is_u32, "codepage",	Opt_codepage,
		  fs_param_deprecated, NULL),
	{}
};

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 5, 0)
static const struct fs_parameter_description exfat_parameters = {
	.name		= "exfat",
	.specs		= exfat_param_specs,
	.enums		= exfat_param_enums,
};
#endif

static int exfat_parse_param(struct fs_context *fc, struct fs_parameter *param)
{
	struct exfat_sb_info *sbi = fc->s_fs_info;
	struct exfat_mount_options *opts = &sbi->options;
	struct fs_parse_result result;
	int opt;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	opt = fs_parse(fc, exfat_parameters, param, &result);
#else
	opt = fs_parse(fc, &exfat_parameters, param, &result);
#endif
	if (opt < 0)
		return opt;

	switch (opt) {
	case Opt_uid:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		opts->fs_uid = make_kuid(current_user_ns(), result.uint_32);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
        opts->fs_uid = KUIDT_INIT(result.uint_32);
#else
        opts->fs_uid = result.uint_32;
#endif
		break;
	case Opt_gid:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		opts->fs_gid = make_kgid(current_user_ns(), result.uint_32);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
        opts->fs_gid = KUIDT_INIT(result.uint_32);
#else
        opts->fs_gid = result.uint_32;
#endif
		break;
	case Opt_umask:
		opts->fs_fmask = result.uint_32;
		opts->fs_dmask = result.uint_32;
		break;
	case Opt_dmask:
		opts->fs_dmask = result.uint_32;
		break;
	case Opt_fmask:
		opts->fs_fmask = result.uint_32;
		break;
	case Opt_allow_utime:
		opts->allow_utime = result.uint_32 & 0022;
		break;
	case Opt_charset:
		exfat_free_iocharset(sbi);
		opts->iocharset = param->string;
		param->string = NULL;
		break;
	case Opt_errors:
		opts->errors = result.uint_32;
		break;
	case Opt_discard:
		opts->discard = 1;
		break;
	case Opt_time_offset:
		/*
		 * Make the limit 24 just in case someone invents something
		 * unusual.
		 */
		if (result.int_32 < -24 * 60 || result.int_32 > 24 * 60)
			return -EINVAL;
		opts->time_offset = result.int_32;
		break;
	case Opt_utf8:
	case Opt_debug:
	case Opt_namecase:
	case Opt_codepage:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
#else
enum {
	Opt_uid,
	Opt_gid,
	Opt_umask,
	Opt_dmask,
	Opt_fmask,
	Opt_allow_utime,
	Opt_charset,
	Opt_err_cont,
	Opt_err_panic,
	Opt_err_ro,
	Opt_err,
	Opt_discard,
	Opt_time_offset,

	/* Deprecated options */
	Opt_utf8,
	Opt_debug,
	Opt_namecase,
	Opt_codepage,
	Opt_fs,
};

static const match_table_t exfat_tokens = {
	{Opt_uid, "uid=%u"},
	{Opt_gid, "gid=%u"},
	{Opt_umask, "umask=%o"},
	{Opt_dmask, "dmask=%o"},
	{Opt_fmask, "fmask=%o"},
	{Opt_allow_utime, "allow_utime=%o"},
	{Opt_charset, "iocharset=%s"},
	{Opt_err_cont, "errors=continue"},
	{Opt_err_panic, "errors=panic"},
	{Opt_err_ro, "errors=remount-ro"},
	{Opt_discard, "discard"},
	{Opt_codepage, "codepage=%u"},
	{Opt_namecase, "namecase=%u"},
	{Opt_debug, "debug"},
	{Opt_utf8, "utf8"},
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
	opts->allow_utime = -1;
	opts->iocharset = exfat_default_iocharset;
	opts->utf8 = 0;
	opts->errors = EXFAT_ERRORS_RO;
	opts->discard = 0;

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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		opts->fs_uid = make_kuid(current_user_ns(), option);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
        opts->fs_uid = KUIDT_INIT(option);
#else
        opts->fs_uid = option;
#endif
				break;
			case Opt_gid:
				if (match_int(&args[0], &option))
					return 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		opts->fs_gid = make_kgid(current_user_ns(), option);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
        opts->fs_gid = KUIDT_INIT(option);
#else
        opts->fs_gid = option;
#endif
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
				opts->allow_utime = option & (0022);
				break;
			case Opt_charset:
				if (opts->iocharset != exfat_default_iocharset)
					kfree(opts->iocharset);
				tmpstr = match_strdup(&args[0]);
				if (!tmpstr)
					return -ENOMEM;
				opts->iocharset = tmpstr;
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
			case Opt_time_offset:
				if (match_int(&args[0], &option))
					return -EINVAL;
				/*
				 * Make the limit 24 just in case someone
				 * invents something unusual.
				 */
				if (option < -24 * 60 || option > 24 * 60)
					return -EINVAL;
				opts->time_offset = option;
				break;
			case Opt_utf8:
			case Opt_debug:
			case Opt_namecase:
			case Opt_codepage:
				break;
			default:
				if (!silent) {
					exfat_msg(sb, KERN_ERR,
							"unrecognized mount option \"%s\" or missing value",
							p);
				}
				return -EINVAL;
		}
	}

out:
	if (opts->allow_utime == -1)
		opts->allow_utime = ~opts->fs_dmask & (0022);

	if (opts->discard) {
		struct request_queue *q = bdev_get_queue(sb->s_bdev);

		if (!blk_queue_discard(q))
			exfat_msg(sb, KERN_WARNING,
					"mounting with \"discard\" option, but the device does not support discard");
		opts->discard = 0;
	}

	return 0;
}
#endif


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
	struct exfat_inode_info *ei = EXFAT_I(inode);
	struct exfat_chain cdir;
	int num_subdirs, num_clu = 0;

	exfat_chain_set(&ei->dir, sbi->root_dir, 0, ALLOC_FAT_CHAIN);
	ei->entry = -1;
	ei->start_clu = sbi->root_dir;
	ei->flags = ALLOC_FAT_CHAIN;
	ei->type = TYPE_DIR;
	ei->version = 0;
	ei->rwoffset = 0;
	ei->hint_bmap.off = EXFAT_EOF_CLUSTER;
	ei->hint_stat.eidx = 0;
	ei->hint_stat.clu = sbi->root_dir;
	ei->hint_femp.eidx = EXFAT_HINT_NONE;

	exfat_chain_set(&cdir, sbi->root_dir, 0, ALLOC_FAT_CHAIN);
	if (exfat_count_num_clusters(sb, &cdir, &num_clu))
		return -EIO;
	i_size_write(inode, num_clu << sbi->cluster_size_bits);

	num_subdirs = exfat_count_dir_entries(sb, &cdir);
	if (num_subdirs < 0)
		return -EIO;
    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,00)
	set_nlink(inode, num_subdirs + EXFAT_MIN_SUBDIR);
#else
	inode->i_nlink = num_subdirs + EXFAT_MIN_SUBDIR;
#endif

	inode->i_uid = sbi->options.fs_uid;
	inode->i_gid = sbi->options.fs_gid;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
	inode_inc_iversion(inode);
#else
	inode->i_version++;
#endif
	inode->i_generation = 0;
	inode->i_mode = exfat_make_mode(sbi, ATTR_SUBDIR, 0777);
	inode->i_op = &exfat_dir_inode_operations;
	inode->i_fop = &exfat_dir_operations;

	inode->i_blocks = ((i_size_read(inode) + (sbi->cluster_size - 1))
			& ~(sbi->cluster_size - 1)) >> inode->i_blkbits;
	EXFAT_I(inode)->i_pos = ((loff_t)sbi->root_dir << 32) | 0xffffffff;
	EXFAT_I(inode)->i_size_aligned = i_size_read(inode);
	EXFAT_I(inode)->i_size_ondisk = i_size_read(inode);

	exfat_save_attr(inode, ATTR_SUBDIR);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
	inode->i_mtime = inode->i_atime = inode->i_ctime = ei->i_crtime =
		current_time(inode);
#else
	inode->i_mtime = inode->i_atime = inode->i_ctime = ei->i_crtime =
		CURRENT_TIME_SEC;
#endif
	exfat_truncate_atime(&inode->i_atime);
	exfat_cache_init_inode(inode);
	return 0;
}

static int exfat_calibrate_blocksize(struct super_block *sb, int logical_sect)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	if (!is_power_of_2(logical_sect) ||
	    logical_sect < 512 || logical_sect > 4096) {
		exfat_err(sb, "bogus logical sector size %u", logical_sect);
		return -EIO;
	}

	if (logical_sect < sb->s_blocksize) {
		exfat_err(sb, "logical sector size too small for device (logical sector size = %u)",
			  logical_sect);
		return -EIO;
	}

	if (logical_sect > sb->s_blocksize) {
		brelse(sbi->boot_bh);
		sbi->boot_bh = NULL;

		if (!sb_set_blocksize(sb, logical_sect)) {
			exfat_err(sb, "unable to set blocksize %u",
				  logical_sect);
			return -EIO;
		}
		sbi->boot_bh = sb_bread(sb, 0);
		if (!sbi->boot_bh) {
			exfat_err(sb, "unable to read boot sector (logical sector size = %lu)",
				  sb->s_blocksize);
			return -EIO;
		}
	}
	return 0;
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
static inline void *check_bytes8(const u8 *start, u8 value, unsigned int bytes)
{
	while (bytes) {
		if (*start != value)
			return (void *)start;
		start++;
		bytes--;
	}
	return NULL;
}

static inline void *memchr_inv(const void *start, int c, size_t bytes)
{
	u8 value = c;
	u64 value64;
	unsigned int words, prefix;

	if (bytes <= 16)
		return (void *)check_bytes8(start, value, bytes);

	value64 = value;
#if defined(CONFIG_ARCH_HAS_FAST_MULTIPLIER) && BITS_PER_LONG == 64
	value64 *= 0x0101010101010101ULL;
#elif defined(CONFIG_ARCH_HAS_FAST_MULTIPLIER)
	value64 *= 0x01010101;
	value64 |= value64 << 32;
#else
	value64 |= value64 << 8;
	value64 |= value64 << 16;
	value64 |= value64 << 32;
#endif

	prefix = (unsigned long)start % 8;
	if (prefix) {
		u8 *r;

		prefix = 8 - prefix;
		r = check_bytes8(start, value, prefix);
		if (r)
			return (void *)r;
		start += prefix;
		bytes -= prefix;
	}

	words = bytes / 8;

	while (words) {
		if (*(u64 *)start != value64)
			return (void *) check_bytes8(start, value, 8);
		start += 8;
		words--;
	}

	return (void *) check_bytes8(start, value, bytes % 8);
}
#endif

static int exfat_read_boot_sector(struct super_block *sb)
{
	struct boot_sector *p_boot;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	/* set block size to read super block */
	sb_min_blocksize(sb, 512);

	/* read boot sector */
	sbi->boot_bh = sb_bread(sb, 0);
	if (!sbi->boot_bh) {
		exfat_err(sb, "unable to read boot sector");
		return -EIO;
	}
	p_boot = (struct boot_sector *)sbi->boot_bh->b_data;

	/* check the validity of BOOT */
	if (le16_to_cpu((p_boot->signature)) != BOOT_SIGNATURE) {
		exfat_err(sb, "invalid boot record signature");
		return -EINVAL;
	}

	if (memcmp(p_boot->fs_name, STR_EXFAT, BOOTSEC_FS_NAME_LEN)) {
		exfat_err(sb, "invalid fs_name"); /* fs_name may unprintable */
		return -EINVAL;
	}

	/*
	 * must_be_zero field must be filled with zero to prevent mounting
	 * from FAT volume.
	 */
	if (memchr_inv(p_boot->must_be_zero, 0, sizeof(p_boot->must_be_zero)))
		return -EINVAL;

	if (p_boot->num_fats != 1 && p_boot->num_fats != 2) {
		exfat_err(sb, "bogus number of FAT structure");
		return -EINVAL;
	}

	sbi->sect_per_clus = 1 << p_boot->sect_per_clus_bits;
	sbi->sect_per_clus_bits = p_boot->sect_per_clus_bits;
	sbi->cluster_size_bits = p_boot->sect_per_clus_bits +
		p_boot->sect_size_bits;
	sbi->cluster_size = 1 << sbi->cluster_size_bits;
	sbi->num_FAT_sectors = le32_to_cpu(p_boot->fat_length);
	sbi->FAT1_start_sector = le32_to_cpu(p_boot->fat_offset);
	sbi->FAT2_start_sector = le32_to_cpu(p_boot->fat_offset);
	if (p_boot->num_fats == 2)
		sbi->FAT2_start_sector += sbi->num_FAT_sectors;
	sbi->data_start_sector = le32_to_cpu(p_boot->clu_offset);
	sbi->num_sectors = le64_to_cpu(p_boot->vol_length);
	/* because the cluster index starts with 2 */
	sbi->num_clusters = le32_to_cpu(p_boot->clu_count) +
		EXFAT_RESERVED_CLUSTERS;

	sbi->root_dir = le32_to_cpu(p_boot->root_cluster);
	sbi->dentries_per_clu = 1 <<
		(sbi->cluster_size_bits - DENTRY_SIZE_BITS);

	sbi->vol_flag = le16_to_cpu(p_boot->vol_flags);
	sbi->clu_srch_ptr = EXFAT_FIRST_CLUSTER;
	sbi->used_clusters = EXFAT_CLUSTERS_UNTRACKED;

	/* check consistencies */
	if (sbi->num_FAT_sectors << p_boot->sect_size_bits <
	    sbi->num_clusters * 4) {
		exfat_err(sb, "bogus fat length");
		return -EINVAL;
	}
	if (sbi->data_start_sector <
	    sbi->FAT1_start_sector + sbi->num_FAT_sectors * p_boot->num_fats) {
		exfat_err(sb, "bogus data start sector");
		return -EINVAL;
	}
	if (sbi->vol_flag & VOL_DIRTY)
		exfat_warn(sb, "Volume was not properly unmounted. Some data may be corrupt. Please run fsck.");
	if (sbi->vol_flag & ERR_MEDIUM)
		exfat_warn(sb, "Medium has reported failures. Some data may be lost.");

	/* exFAT file size is limited by a disk volume size */
	sb->s_maxbytes = (u64)(sbi->num_clusters - EXFAT_RESERVED_CLUSTERS) <<
		sbi->cluster_size_bits;

	/* check logical sector size */
	if (exfat_calibrate_blocksize(sb, 1 << p_boot->sect_size_bits))
		return -EIO;

	return 0;
}

static int exfat_verify_boot_region(struct super_block *sb)
{
	struct buffer_head *bh = NULL;
	u32 chksum = 0;
	__le32 *p_sig, *p_chksum;
	int sn, i;

	/* read boot sector sub-regions */
	for (sn = 0; sn < 11; sn++) {
		bh = sb_bread(sb, sn);
		if (!bh)
			return -EIO;

		if (sn != 0 && sn <= 8) {
			/* extended boot sector sub-regions */
			p_sig = (__le32 *)&bh->b_data[sb->s_blocksize - 4];
			if (le32_to_cpu(*p_sig) != EXBOOT_SIGNATURE)
				exfat_warn(sb, "Invalid exboot-signature(sector = %d): 0x%08x",
					   sn, le32_to_cpu(*p_sig));
		}

		chksum = exfat_calc_chksum32(bh->b_data, sb->s_blocksize,
			chksum, sn ? CS_DEFAULT : CS_BOOT_SECTOR);
		brelse(bh);
	}

	/* boot checksum sub-regions */
	bh = sb_bread(sb, sn);
	if (!bh)
		return -EIO;

	for (i = 0; i < sb->s_blocksize; i += sizeof(u32)) {
		p_chksum = (__le32 *)&bh->b_data[i];
		if (le32_to_cpu(*p_chksum) != chksum) {
			exfat_err(sb, "Invalid boot checksum (boot checksum : 0x%08x, checksum : 0x%08x)",
				  le32_to_cpu(*p_chksum), chksum);
			brelse(bh);
			return -EINVAL;
		}
	}
	brelse(bh);
	return 0;
}

/* mount the file system volume */
static int __exfat_fill_super(struct super_block *sb)
{
	int ret;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	ret = exfat_read_boot_sector(sb);
	if (ret) {
		exfat_err(sb, "failed to read boot sector");
		goto free_bh;
	}

	ret = exfat_verify_boot_region(sb);
	if (ret) {
		exfat_err(sb, "invalid boot region");
		goto free_bh;
	}

	ret = exfat_create_upcase_table(sb);
	if (ret) {
		exfat_err(sb, "failed to load upcase table");
		goto free_bh;
	}

	ret = exfat_load_bitmap(sb);
	if (ret) {
		exfat_err(sb, "failed to load alloc-bitmap");
		goto free_upcase_table;
	}

	ret = exfat_count_used_clusters(sb, &sbi->used_clusters);
	if (ret) {
		exfat_err(sb, "failed to scan clusters");
		goto free_alloc_bitmap;
	}

	return 0;

free_alloc_bitmap:
	exfat_free_bitmap(sbi);
free_upcase_table:
	exfat_free_upcase_table(sbi);
free_bh:
	brelse(sbi->boot_bh);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static int exfat_fill_super(struct super_block *sb, struct fs_context *fc)
#else
static int exfat_fill_super(struct super_block *sb, void *data, int silent)
#endif
{
	struct inode *root_inode;
	int err;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
	struct exfat_sb_info *sbi = sb->s_fs_info;
	struct exfat_mount_options *opts = &sbi->options;

	if (opts->allow_utime == (unsigned short)-1)
		opts->allow_utime = ~opts->fs_dmask & 0022;

	if (opts->discard) {
		struct request_queue *q = bdev_get_queue(sb->s_bdev);

		if (!blk_queue_discard(q)) {
			exfat_warn(sb, "mounting with \"discard\" option, but the device does not support discard");
			opts->discard = 0;
		}
	}
#else
	struct exfat_sb_info *sbi;

	/*
	 * GFP_KERNEL is ok here, because while we do hold the
	 * supeblock lock, memory pressure can't call back into
	 * the filesystem, since we're only just about to mount
	 * it and have no inodes etc active!
	 */
	sbi = kzalloc(sizeof(struct exfat_sb_info), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;

	mutex_init(&sbi->s_lock);
	sb->s_fs_info = sbi;
	ratelimit_state_init(&sbi->ratelimit, DEFAULT_RATELIMIT_INTERVAL,
			DEFAULT_RATELIMIT_BURST);
	err = parse_options(sb, data, silent, &sbi->options);
	if (err) {
		exfat_msg(sb, KERN_ERR, "failed to parse options");
		goto check_nls_io;
	}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	sb->s_flags |= SB_NODIRATIME;
#else
	sb->s_flags |= MS_NODIRATIME;
#endif
	sb->s_magic = EXFAT_SUPER_MAGIC;
	sb->s_op = &exfat_sops;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
	sb->s_time_gran = 10 * NSEC_PER_MSEC;
	sb->s_time_min = EXFAT_MIN_TIMESTAMP_SECS;
	sb->s_time_max = EXFAT_MAX_TIMESTAMP_SECS;
#endif

	err = __exfat_fill_super(sb);
	if (err) {
		exfat_err(sb, "failed to recognize exfat type");
		goto check_nls_io;
	}

	/* set up enough so that it can read an inode */
	exfat_hash_init(sb);

	if (!strcmp(sbi->options.iocharset, "utf8"))
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
		opts->utf8 = 1;
#else
		sbi->options.utf8 = 1;
#endif
	else {
		sbi->nls_io = load_nls(sbi->options.iocharset);
		if (!sbi->nls_io) {
			exfat_err(sb, "IO charset %s not found",
				  sbi->options.iocharset);
			err = -EINVAL;
			goto free_table;
		}
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
	if (sbi->options.utf8)
		sb->s_d_op = &exfat_utf8_dentry_ops;
	else
		sb->s_d_op = &exfat_dentry_ops;
#else
    if (sbi->options.utf8)
		sb->s_root->d_op = &exfat_utf8_dentry_ops;
	else
		sb->s_root->d_op = &exfat_dentry_ops;
#endif

	root_inode = new_inode(sb);
	if (!root_inode) {
		exfat_err(sb, "failed to allocate root inode");
		err = -ENOMEM;
		goto free_table;
	}

	root_inode->i_ino = EXFAT_ROOT_INO;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
	inode_set_iversion(root_inode, 1);
#else
	root_inode->i_version = 1;
#endif
	err = exfat_read_root(root_inode);
	if (err) {
		exfat_err(sb, "failed to initialize root inode");
		goto put_inode;
	}

	exfat_hash_inode(root_inode, EXFAT_I(root_inode)->i_pos);
	insert_inode_hash(root_inode);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,00)
	sb->s_root = d_make_root(root_inode);
#else
	sb->s_root = d_alloc_root(root_inode);
#endif
	if (!sb->s_root) {
		exfat_err(sb, "failed to get the root dentry");
		err = -ENOMEM;
		goto put_inode;
	}

	return 0;

put_inode:
	iput(root_inode);
	sb->s_root = NULL;

free_table:
	exfat_free_upcase_table(sbi);
	exfat_free_bitmap(sbi);
	brelse(sbi->boot_bh);

check_nls_io:
	unload_nls(sbi->nls_io);
	exfat_free_iocharset(sbi);
	sb->s_fs_info = NULL;
	kfree(sbi);
	return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static int exfat_get_tree(struct fs_context *fc)
{
	return get_tree_bdev(fc, exfat_fill_super);
}

static void exfat_free(struct fs_context *fc)
{
	struct exfat_sb_info *sbi = fc->s_fs_info;

	if (sbi) {
		exfat_free_iocharset(sbi);
		kfree(sbi);
	}
}

static int exfat_reconfigure(struct fs_context *fc)
{
	fc->sb_flags |= SB_NODIRATIME;

	/* volume flag will be updated in exfat_sync_fs */
	sync_filesystem(fc->root->d_sb);
	return 0;
}

static const struct fs_context_operations exfat_context_ops = {
	.parse_param	= exfat_parse_param,
	.get_tree	= exfat_get_tree,
	.free		= exfat_free,
	.reconfigure	= exfat_reconfigure,
};

static int exfat_init_fs_context(struct fs_context *fc)
{
	struct exfat_sb_info *sbi;

	sbi = kzalloc(sizeof(struct exfat_sb_info), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;

	mutex_init(&sbi->s_lock);
	ratelimit_state_init(&sbi->ratelimit, DEFAULT_RATELIMIT_INTERVAL,
			DEFAULT_RATELIMIT_BURST);

	sbi->options.fs_uid = current_uid();
	sbi->options.fs_gid = current_gid();
	sbi->options.fs_fmask = current->fs->umask;
	sbi->options.fs_dmask = current->fs->umask;
	sbi->options.allow_utime = -1;
	sbi->options.iocharset = exfat_default_iocharset;
	sbi->options.errors = EXFAT_ERRORS_RO;

	fc->s_fs_info = sbi;
	fc->ops = &exfat_context_ops;
	return 0;
}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
static int exfat_get_sb(struct file_system_type *fs_type,
						int flags, const char *dev_name,
						void *data, struct vfsmount *mnt)
{
	return get_sb_bdev(fs_type, flags, dev_name, data, exfat_fill_super, mnt);
}
#else
static struct dentry *exfat_fs_mount(struct file_system_type *fs_type,
									 int flags, const char *dev_name,
									 void *data) {
	return mount_bdev(fs_type, flags, dev_name, data, exfat_fill_super);
}
#endif

static struct file_system_type exfat_fs_type = {
	.owner			= THIS_MODULE,
	.name			= "exfat",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
	.init_fs_context	= exfat_init_fs_context,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	.parameters		= exfat_parameters,
#else
	.parameters		= &exfat_parameters,
#endif
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
	.get_sb      = exfat_get_sb,
#else
	.mount       = exfat_fs_mount,
#endif
	.kill_sb		= kill_block_super,
	.fs_flags		= FS_REQUIRES_DEV,
};

static void exfat_inode_init_once(void *foo)
{
	struct exfat_inode_info *ei = (struct exfat_inode_info *)foo;

	INIT_HLIST_NODE(&ei->i_hash_fat);
	inode_init_once(&ei->vfs_inode);
}

static int __init init_exfat_fs(void)
{
	int err;

	err = exfat_cache_init();
	if (err)
		return err;

	exfat_inode_cachep = kmem_cache_create("exfat_inode_cache",
			sizeof(struct exfat_inode_info),
			0, SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD,
			exfat_inode_init_once);
	if (!exfat_inode_cachep) {
		err = -ENOMEM;
		goto shutdown_cache;
	}

	err = register_filesystem(&exfat_fs_type);
	if (err)
		goto destroy_cache;

	return 0;

destroy_cache:
	kmem_cache_destroy(exfat_inode_cachep);
shutdown_cache:
	exfat_cache_shutdown();
	return err;
}

static void __exit exit_exfat_fs(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(exfat_inode_cachep);
	unregister_filesystem(&exfat_fs_type);
	exfat_cache_shutdown();
}

module_init(init_exfat_fs);
module_exit(exit_exfat_fs);

#ifdef MODULE_ALIAS_FS
MODULE_ALIAS_FS("exfat");
#endif
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("exFAT filesystem support");
MODULE_AUTHOR("Samsung Electronics Co., Ltd.");
MODULE_VERSION(EXFAT_VERSION);
