/**
 * sload.c
 *
 * Copyright (C) 2015 Huawei Ltd.
 * Witten by:
 *   Hou Pengyang <houpengyang@huawei.com>
 *   Liu Shuoran <liushuoran@huawei.com>
 *   Jaegeuk Kim <jaegeuk@kernel.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define _GNU_SOURCE
#include "fsck.h"
#include <libgen.h>
#include <dirent.h>
#include <mntent.h>

#ifdef HAVE_LIBSELINUX
#include <selinux/selinux.h>
#include <selinux/label.h>
#endif

#ifdef WITH_ANDROID
#include <selinux/label.h>
#include <private/android_filesystem_config.h>

static void handle_selabel(struct dentry *de, int dir, char *target_out)
{
	uint64_t capabilities;
	unsigned int mode = 0;
	unsigned int uid = 0;
	unsigned int gid = 0;

	fs_config(de->path, dir, target_out, &uid,
			&gid, &mode, &capabilities);
	de->mode = mode;
	de->uid = uid;
	de->gid = gid;
	de->capabilities = capabilities;
}
#else
#define handle_selabel(...)
#endif

static int filter_dot(const struct dirent *d)
{
	return (strcmp(d->d_name, "..") && strcmp(d->d_name, "."));
}

static void f2fs_make_directory(struct f2fs_sb_info *sbi,
				int entries, struct dentry *de)
{
	int i = 0;

	for (i = 0; i < entries; i++) {
		if (de[i].file_type == F2FS_FT_DIR)
			f2fs_mkdir(sbi, de + i);
		else if (de[i].file_type == F2FS_FT_REG_FILE)
			f2fs_create(sbi, de + i);
		else if (de[i].file_type == F2FS_FT_SYMLINK)
			f2fs_symlink(sbi, de + i);
	}
}

static int build_directory(struct f2fs_sb_info *sbi, const char *full_path,
			const char *dir_path, const char *target_out_dir,
			nid_t dir_ino, struct selabel_handle *sehnd)
{
	int entries = 0;
	struct dentry *dentries;
	struct dirent **namelist = NULL;
	struct stat stat;
	int i, ret = 0;

	entries = scandir(full_path, &namelist, filter_dot, (void *)alphasort);
	if (entries < 0) {
		ERR_MSG("No entries in %s\n", full_path);
		return -ENOENT;
	}

	dentries = calloc(entries, sizeof(struct dentry));
	if (dentries == NULL)
		return -ENOMEM;

	for (i = 0; i < entries; i++) {
		dentries[i].name = (unsigned char *)strdup(namelist[i]->d_name);
		if (dentries[i].name == NULL) {
			ERR_MSG("Skip: ENOMEM\n");
			continue;
		}
		dentries[i].len = strlen((char *)dentries[i].name);

		ret = asprintf(&dentries[i].path, "%s/%s",
					dir_path, namelist[i]->d_name);
		ASSERT(ret > 0);
		ret = asprintf(&dentries[i].full_path, "%s/%s",
					full_path, namelist[i]->d_name);
		ASSERT(ret > 0);
		free(namelist[i]);

		ret = lstat(dentries[i].full_path, &stat);
		if (ret < 0) {
			ERR_MSG("Skip: lstat failure\n");
			continue;
		}
		dentries[i].size = stat.st_size;
		dentries[i].mode = stat.st_mode &
			(S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO);
		dentries[i].mtime = stat.st_mtime;

		handle_selabel(dentries + i, S_ISDIR(stat.st_mode),
							target_out_dir);

#ifdef HAVE_LIBSELINUX
		if (sehnd && selabel_lookup(sehnd, &dentries[i].secon,
					dentries[i].path, stat.st_mode) < 0)
			ERR_MSG("Cannot lookup security context for %s\n",
						dentries[i].path);
#endif

		dentries[i].pino = dir_ino;

		if (S_ISREG(stat.st_mode)) {
			dentries[i].file_type = F2FS_FT_REG_FILE;
		} else if (S_ISDIR(stat.st_mode)) {
			dentries[i].file_type = F2FS_FT_DIR;
		} else if (S_ISCHR(stat.st_mode)) {
			dentries[i].file_type = F2FS_FT_CHRDEV;
		} else if (S_ISBLK(stat.st_mode)) {
			dentries[i].file_type = F2FS_FT_BLKDEV;
		} else if (S_ISFIFO(stat.st_mode)) {
			dentries[i].file_type = F2FS_FT_FIFO;
		} else if (S_ISSOCK(stat.st_mode)) {
			dentries[i].file_type = F2FS_FT_SOCK;
		} else if (S_ISLNK(stat.st_mode)) {
			dentries[i].file_type = F2FS_FT_SYMLINK;
			dentries[i].link = calloc(F2FS_BLKSIZE, 1);
			ASSERT(dentries[i].link);
			ret = readlink(dentries[i].full_path,
					dentries[i].link, F2FS_BLKSIZE - 1);
			ASSERT(ret >= 0);
		} else {
			MSG(1, "unknown file type on %s", dentries[i].path);
			i--;
			entries--;
		}
	}

	free(namelist);

	f2fs_make_directory(sbi, entries, dentries);

	for (i = 0; i < entries; i++) {
		if (dentries[i].file_type == F2FS_FT_REG_FILE) {
			f2fs_build_file(sbi, dentries + i);
		} else if (dentries[i].file_type == F2FS_FT_DIR) {
			char *subdir_full_path = NULL;
			char *subdir_dir_path;

			ret = asprintf(&subdir_full_path, "%s/",
							dentries[i].full_path);
			ASSERT(ret > 0);
			ret = asprintf(&subdir_dir_path, "%s/",
							dentries[i].path);
			ASSERT(ret > 0);

			build_directory(sbi, subdir_full_path, subdir_dir_path,
					target_out_dir, dentries[i].ino, sehnd);
			free(subdir_full_path);
			free(subdir_dir_path);
		} else if (dentries[i].file_type == F2FS_FT_SYMLINK) {
			/*
			 * It is already done in f2fs_make_directory
			 * f2fs_make_symlink(sbi, dir_ino, &dentries[i]);
			 */
		} else {
			MSG(1, "Error unknown file type\n");
		}

#ifdef HAVE_LIBSELINUX
		if (dentries[i].secon) {
			inode_set_selinux(sbi, dentries[i].ino, dentries[i].secon);
			MSG(1, "File = %s \n----->SELinux context = %s\n",
					dentries[i].path, dentries[i].secon);
			MSG(1, "----->mode = 0x%x, uid = 0x%x, gid = 0x%x, "
					"capabilities = 0x%lx \n",
					dentries[i].mode, dentries[i].uid,
					dentries[i].gid, dentries[i].capabilities);
		}

		free(dentries[i].secon);
#endif

		free(dentries[i].path);
		free(dentries[i].full_path);
		free((void *)dentries[i].name);
	}

	free(dentries);
	return 0;
}

int f2fs_sload(struct f2fs_sb_info *sbi, const char *from_dir,
				const char *mount_point,
				const char *target_out_dir,
				struct selabel_handle *sehnd)
{
	int ret = 0;
	nid_t mnt_ino = F2FS_ROOT_INO(sbi);

	/* flush NAT/SIT journal entries */
	flush_journal_entries(sbi);

	ret = f2fs_find_path(sbi, (char *)mount_point, &mnt_ino);
	if (ret) {
		ERR_MSG("Failed to get mount point %s\n", mount_point);
		return ret;
	}

	ret = build_directory(sbi, from_dir, mount_point, target_out_dir,
						mnt_ino, sehnd);
	if (ret) {
		ERR_MSG("Failed to build due to %d\n", ret);
		return ret;
	}

#ifdef HAVE_LIBSELINUX
	if (sehnd) {
		char *secontext = NULL;

		/* set root inode selinux context */
		if (selabel_lookup(sehnd, &secontext, mount_point, S_IFDIR) < 0)
			ERR_MSG("cannot lookup security context for %s\n",
								mount_point);
		if (secontext) {
			MSG(1, "Labeling %s as %s, root_ino = %d\n",
					mount_point, secontext, F2FS_ROOT_INO(sbi));
			/* xattr_add for root inode */
			inode_set_selinux(sbi, F2FS_ROOT_INO(sbi), secontext);
		}
		free(secontext);
	}
#endif

	/* update curseg info; can update sit->types */
	move_curseg_info(sbi, SM_I(sbi)->main_blkaddr);
	zero_journal_entries(sbi);
	write_curseg_info(sbi);

	/* flush dirty sit entries */
	flush_sit_entries(sbi);

	write_checkpoint(sbi);
	return 0;
}
