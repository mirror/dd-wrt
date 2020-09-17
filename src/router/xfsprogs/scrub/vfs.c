// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "handle.h"
#include "libfrog/paths.h"
#include "libfrog/workqueue.h"
#include "xfs_scrub.h"
#include "common.h"
#include "vfs.h"

#ifndef AT_NO_AUTOMOUNT
# define AT_NO_AUTOMOUNT	0x800
#endif

/*
 * Helper functions to assist in traversing a directory tree using regular
 * VFS calls.
 */

/* Scan a filesystem tree. */
struct scan_fs_tree {
	unsigned int		nr_dirs;
	pthread_mutex_t		lock;
	pthread_cond_t		wakeup;
	struct stat		root_sb;
	bool			aborted;
	scan_fs_tree_dir_fn	dir_fn;
	scan_fs_tree_dirent_fn	dirent_fn;
	void			*arg;
};

/* Per-work-item scan context. */
struct scan_fs_tree_dir {
	char			*path;
	struct scan_fs_tree	*sft;
	bool			rootdir;
};

static void scan_fs_dir(struct workqueue *wq, xfs_agnumber_t agno, void *arg);

/* Increment the number of directories that are queued for processing. */
static void
inc_nr_dirs(
	struct scan_fs_tree	*sft)
{
	pthread_mutex_lock(&sft->lock);
	sft->nr_dirs++;
	pthread_mutex_unlock(&sft->lock);
}

/*
 * Decrement the number of directories that are queued for processing and if
 * we ran out of dirs to process, wake up anyone who was waiting for processing
 * to finish.
 */
static void
dec_nr_dirs(
	struct scan_fs_tree	*sft)
{
	pthread_mutex_lock(&sft->lock);
	sft->nr_dirs--;
	if (sft->nr_dirs == 0)
		pthread_cond_signal(&sft->wakeup);
	pthread_mutex_unlock(&sft->lock);
}

/* Queue a directory for scanning. */
static int
queue_subdir(
	struct scrub_ctx	*ctx,
	struct scan_fs_tree	*sft,
	struct workqueue	*wq,
	const char		*path,
	bool			is_rootdir)
{
	struct scan_fs_tree_dir	*new_sftd;
	int			error;

	new_sftd = malloc(sizeof(struct scan_fs_tree_dir));
	if (!new_sftd)
		return errno;

	new_sftd->path = strdup(path);
	if (!new_sftd->path) {
		error = errno;
		goto out_sftd;
	}

	new_sftd->sft = sft;
	new_sftd->rootdir = is_rootdir;

	inc_nr_dirs(sft);
	error = -workqueue_add(wq, scan_fs_dir, 0, new_sftd);
	if (error) {
		dec_nr_dirs(sft);
		str_liberror(ctx, error, _("queueing directory scan work"));
		goto out_path;
	}

	return 0;
out_path:
	free(new_sftd->path);
out_sftd:
	free(new_sftd);
	return error;
}

/* Scan a directory sub tree. */
static void
scan_fs_dir(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;
	struct scan_fs_tree_dir	*sftd = arg;
	struct scan_fs_tree	*sft = sftd->sft;
	DIR			*dir;
	struct dirent		*dirent;
	char			newpath[PATH_MAX];
	struct stat		sb;
	int			dir_fd;
	int			error;

	/* Open the directory. */
	dir_fd = open(sftd->path, O_RDONLY | O_NOATIME | O_NOFOLLOW | O_NOCTTY);
	if (dir_fd < 0) {
		if (errno != ENOENT)
			str_errno(ctx, sftd->path);
		goto out;
	}

	/* Caller-specific directory checks. */
	error = sft->dir_fn(ctx, sftd->path, dir_fd, sft->arg);
	if (error) {
		sft->aborted = true;
		error = close(dir_fd);
		if (error)
			str_errno(ctx, sftd->path);
		goto out;
	}

	/* Iterate the directory entries. */
	dir = fdopendir(dir_fd);
	if (!dir) {
		str_errno(ctx, sftd->path);
		sft->aborted = true;
		close(dir_fd);
		goto out;
	}
	rewinddir(dir);
	for (dirent = readdir(dir);
	     !sft->aborted && dirent != NULL;
	     dirent = readdir(dir)) {
		snprintf(newpath, PATH_MAX, "%s/%s", sftd->path,
				dirent->d_name);

		/* Get the stat info for this directory entry. */
		error = fstatat(dir_fd, dirent->d_name, &sb,
				AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW);
		if (error) {
			str_errno(ctx, newpath);
			continue;
		}

		/* Ignore files on other filesystems. */
		if (sb.st_dev != sft->root_sb.st_dev)
			continue;

		/* Caller-specific directory entry function. */
		error = sft->dirent_fn(ctx, newpath, dir_fd, dirent, &sb,
				sft->arg);
		if (error) {
			sft->aborted = true;
			break;
		}

		if (scrub_excessive_errors(ctx)) {
			sft->aborted = true;
			break;
		}

		/* If directory, call ourselves recursively. */
		if (S_ISDIR(sb.st_mode) && strcmp(".", dirent->d_name) &&
		    strcmp("..", dirent->d_name)) {
			error = queue_subdir(ctx, sft, wq, newpath, false);
			if (error) {
				str_liberror(ctx, error,
_("queueing subdirectory scan"));
				sft->aborted = true;
				break;
			}
		}
	}

	/* Close dir, go away. */
	error = closedir(dir);
	if (error)
		str_errno(ctx, sftd->path);

out:
	dec_nr_dirs(sft);
	free(sftd->path);
	free(sftd);
}

/*
 * Scan the entire filesystem.  This function returns 0 on success; if there
 * are errors, this function will log them and returns nonzero.
 */
int
scan_fs_tree(
	struct scrub_ctx	*ctx,
	scan_fs_tree_dir_fn	dir_fn,
	scan_fs_tree_dirent_fn	dirent_fn,
	void			*arg)
{
	struct workqueue	wq;
	struct scan_fs_tree	sft = {
		.root_sb	= ctx->mnt_sb,
		.dir_fn		= dir_fn,
		.dirent_fn	= dirent_fn,
		.arg		= arg,
	};
	int			ret;

	ret = pthread_mutex_init(&sft.lock, NULL);
	if (ret) {
		str_liberror(ctx, ret, _("creating directory scan lock"));
		return ret;
	}
	ret = pthread_cond_init(&sft.wakeup, NULL);
	if (ret) {
		str_liberror(ctx, ret, _("creating directory scan signal"));
		goto out_mutex;
	}

	ret = -workqueue_create(&wq, (struct xfs_mount *)ctx,
			scrub_nproc_workqueue(ctx));
	if (ret) {
		str_liberror(ctx, ret, _("creating directory scan workqueue"));
		goto out_cond;
	}

	ret = queue_subdir(ctx, &sft, &wq, ctx->mntpoint, true);
	if (ret) {
		str_liberror(ctx, ret, _("queueing directory scan"));
		goto out_wq;
	}

	/*
	 * Wait for the wakeup to trigger, which should only happen when the
	 * last worker thread decrements nr_dirs to zero.  Once the worker
	 * triggers the wakeup and unlocks the sft lock, it's no longer safe
	 * for any worker thread to access sft, as we now own the lock and are
	 * about to tear everything down.
	 */
	pthread_mutex_lock(&sft.lock);
	if (sft.nr_dirs)
		pthread_cond_wait(&sft.wakeup, &sft.lock);
	assert(sft.nr_dirs == 0);
	pthread_mutex_unlock(&sft.lock);

	ret = -workqueue_terminate(&wq);
	if (ret) {
		str_liberror(ctx, ret, _("finishing directory scan work"));
		goto out_wq;
	}

	if (!ret && sft.aborted)
		ret = -1;

out_wq:
	workqueue_destroy(&wq);
out_cond:
	pthread_cond_destroy(&sft.wakeup);
out_mutex:
	pthread_mutex_destroy(&sft.lock);
	return ret;
}

#ifndef FITRIM
struct fstrim_range {
	__u64 start;
	__u64 len;
	__u64 minlen;
};
#define FITRIM		_IOWR('X', 121, struct fstrim_range)	/* Trim */
#endif

/* Call FITRIM to trim all the unused space in a filesystem. */
void
fstrim(
	struct scrub_ctx	*ctx)
{
	struct fstrim_range	range = {0};
	int			error;

	range.len = ULLONG_MAX;
	error = ioctl(ctx->mnt.fd, FITRIM, &range);
	if (error && errno != EOPNOTSUPP && errno != ENOTTY)
		perror(_("fstrim"));
}
