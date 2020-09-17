// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include "handle.h"
#include "libfrog/paths.h"
#include "libfrog/workqueue.h"
#include "xfs_scrub.h"
#include "common.h"
#include "libfrog/bitmap.h"
#include "disk.h"
#include "filemap.h"
#include "fscounters.h"
#include "inodes.h"
#include "read_verify.h"
#include "spacemap.h"
#include "vfs.h"

/*
 * Phase 6: Verify data file integrity.
 *
 * Identify potential data block extents with GETFSMAP, then feed those
 * extents to the read-verify pool to get the verify commands batched,
 * issued, and (if there are problems) reported back to us.  If there
 * are errors, we'll record the bad regions and (if available) use rmap
 * to tell us if metadata are now corrupt.  Otherwise, we'll scan the
 * whole directory tree looking for files that overlap the bad regions
 * and report the paths of the now corrupt files.
 */

/* Verify disk blocks with GETFSMAP */

struct media_verify_state {
	struct read_verify_pool	*rvp_data;
	struct read_verify_pool	*rvp_log;
	struct read_verify_pool	*rvp_realtime;
	struct bitmap		*d_bad;		/* bytes */
	struct bitmap		*r_bad;		/* bytes */
};

/* Find the fd for a given device identifier. */
static struct read_verify_pool *
dev_to_pool(
	struct scrub_ctx		*ctx,
	struct media_verify_state	*vs,
	dev_t				dev)
{
	if (dev == ctx->fsinfo.fs_datadev)
		return vs->rvp_data;
	else if (dev == ctx->fsinfo.fs_logdev)
		return vs->rvp_log;
	else if (dev == ctx->fsinfo.fs_rtdev)
		return vs->rvp_realtime;
	abort();
}

/* Find the device major/minor for a given file descriptor. */
static dev_t
disk_to_dev(
	struct scrub_ctx	*ctx,
	struct disk		*disk)
{
	if (disk == ctx->datadev)
		return ctx->fsinfo.fs_datadev;
	else if (disk == ctx->logdev)
		return ctx->fsinfo.fs_logdev;
	else if (disk == ctx->rtdev)
		return ctx->fsinfo.fs_rtdev;
	abort();
}

/* Find the incore bad blocks bitmap for a given disk. */
static struct bitmap *
bitmap_for_disk(
	struct scrub_ctx		*ctx,
	struct disk			*disk,
	struct media_verify_state	*vs)
{
	dev_t				dev = disk_to_dev(ctx, disk);

	if (dev == ctx->fsinfo.fs_datadev)
		return vs->d_bad;
	else if (dev == ctx->fsinfo.fs_rtdev)
		return vs->r_bad;
	return NULL;
}

struct disk_ioerr_report {
	struct scrub_ctx	*ctx;
	struct disk		*disk;
};

struct owner_decode {
	uint64_t		owner;
	const char		*descr;
};

static const struct owner_decode special_owners[] = {
	{XFS_FMR_OWN_FREE,	"free space"},
	{XFS_FMR_OWN_UNKNOWN,	"unknown owner"},
	{XFS_FMR_OWN_FS,	"static FS metadata"},
	{XFS_FMR_OWN_LOG,	"journalling log"},
	{XFS_FMR_OWN_AG,	"per-AG metadata"},
	{XFS_FMR_OWN_INOBT,	"inode btree blocks"},
	{XFS_FMR_OWN_INODES,	"inodes"},
	{XFS_FMR_OWN_REFC,	"refcount btree"},
	{XFS_FMR_OWN_COW,	"CoW staging"},
	{XFS_FMR_OWN_DEFECTIVE,	"bad blocks"},
	{0, NULL},
};

/* Decode a special owner. */
static const char *
decode_special_owner(
	uint64_t			owner)
{
	const struct owner_decode	*od = special_owners;

	while (od->descr) {
		if (od->owner == owner)
			return od->descr;
		od++;
	}

	return NULL;
}

/* Routines to translate bad physical extents into file paths and offsets. */

struct badfile_report {
	struct scrub_ctx		*ctx;
	const char			*descr;
	struct media_verify_state	*vs;
	struct file_bmap		*bmap;
};

/* Report on bad extents found during a media scan. */
static int
report_badfile(
	uint64_t		start,
	uint64_t		length,
	void			*arg)
{
	struct badfile_report	*br = arg;
	unsigned long long	bad_offset;
	unsigned long long	bad_length;

	/* Clamp the bad region to the file mapping. */
	if (start < br->bmap->bm_physical) {
		length -= br->bmap->bm_physical - start;
		start = br->bmap->bm_physical;
	}
	length = min(length, br->bmap->bm_length);

	/* Figure out how far into the bmap is the bad mapping and report it. */
	bad_offset = start - br->bmap->bm_physical;
	bad_length = min(start + length,
			 br->bmap->bm_physical + br->bmap->bm_length) - start;

	str_unfixable_error(br->ctx, br->descr,
_("media error at data offset %llu length %llu."),
			br->bmap->bm_offset + bad_offset, bad_length);
	return 0;
}

/* Report if this extent overlaps a bad region. */
static int
report_data_loss(
	struct scrub_ctx		*ctx,
	int				fd,
	int				whichfork,
	struct fsxattr			*fsx,
	struct file_bmap		*bmap,
	void				*arg)
{
	struct badfile_report		*br = arg;
	struct media_verify_state	*vs = br->vs;
	struct bitmap			*bmp;

	br->bmap = bmap;

	/* Only report errors for real extents. */
	if (bmap->bm_flags & (BMV_OF_PREALLOC | BMV_OF_DELALLOC))
		return 0;

	if (fsx->fsx_xflags & FS_XFLAG_REALTIME)
		bmp = vs->r_bad;
	else
		bmp = vs->d_bad;

	return -bitmap_iterate_range(bmp, bmap->bm_physical, bmap->bm_length,
			report_badfile, br);
}

/* Report if the extended attribute data overlaps a bad region. */
static int
report_attr_loss(
	struct scrub_ctx		*ctx,
	int				fd,
	int				whichfork,
	struct fsxattr			*fsx,
	struct file_bmap		*bmap,
	void				*arg)
{
	struct badfile_report		*br = arg;
	struct media_verify_state	*vs = br->vs;
	struct bitmap			*bmp = vs->d_bad;

	/* Complain about attr fork extents that don't look right. */
	if (bmap->bm_flags & (BMV_OF_PREALLOC | BMV_OF_DELALLOC)) {
		str_info(ctx, br->descr,
_("found unexpected unwritten/delalloc attr fork extent."));
		return 0;
	}

	if (fsx->fsx_xflags & FS_XFLAG_REALTIME) {
		str_info(ctx, br->descr,
_("found unexpected realtime attr fork extent."));
		return 0;
	}

	if (bitmap_test(bmp, bmap->bm_physical, bmap->bm_length))
		str_corrupt(ctx, br->descr,
_("media error in extended attribute data."));

	return 0;
}

/* Iterate the extent mappings of a file to report errors. */
static int
report_fd_loss(
	struct scrub_ctx		*ctx,
	const char			*descr,
	int				fd,
	void				*arg)
{
	struct badfile_report		br = {
		.ctx			= ctx,
		.vs			= arg,
		.descr			= descr,
	};
	struct file_bmap		key = {0};
	int				ret;

	/* data fork */
	ret = scrub_iterate_filemaps(ctx, fd, XFS_DATA_FORK, &key,
			report_data_loss, &br);
	if (ret) {
		str_liberror(ctx, ret, descr);
		return ret;
	}

	/* attr fork */
	ret = scrub_iterate_filemaps(ctx, fd, XFS_ATTR_FORK, &key,
			report_attr_loss, &br);
	if (ret) {
		str_liberror(ctx, ret, descr);
		return ret;
	}

	return 0;
}

/* Report read verify errors in unlinked (but still open) files. */
static int
report_inode_loss(
	struct scrub_ctx		*ctx,
	struct xfs_handle		*handle,
	struct xfs_bulkstat		*bstat,
	void				*arg)
{
	char				descr[DESCR_BUFSZ];
	int				fd;
	int				error, err2;

	/* Ignore linked files and things we can't open. */
	if (bstat->bs_nlink != 0)
		return 0;
	if (!S_ISREG(bstat->bs_mode) && !S_ISDIR(bstat->bs_mode))
		return 0;

	scrub_render_ino_descr(ctx, descr, DESCR_BUFSZ,
			bstat->bs_ino, bstat->bs_gen, _("(unlinked)"));

	/* Try to open the inode. */
	fd = scrub_open_handle(handle);
	if (fd < 0) {
		error = errno;
		if (error == ESTALE)
			return error;

		str_info(ctx, descr,
_("Disappeared during read error reporting."));
		return error;
	}

	/* Go find the badness. */
	error = report_fd_loss(ctx, descr, fd, arg);

	err2 = close(fd);
	if (err2)
		str_errno(ctx, descr);

	return error;
}

/* Scan a directory for matches in the read verify error list. */
static int
report_dir_loss(
	struct scrub_ctx	*ctx,
	const char		*path,
	int			dir_fd,
	void			*arg)
{
	return report_fd_loss(ctx, path, dir_fd, arg);
}

/*
 * Scan the inode associated with a directory entry for matches with
 * the read verify error list.
 */
static int
report_dirent_loss(
	struct scrub_ctx	*ctx,
	const char		*path,
	int			dir_fd,
	struct dirent		*dirent,
	struct stat		*sb,
	void			*arg)
{
	int			fd;
	int			error, err2;

	/* Ignore things we can't open. */
	if (!S_ISREG(sb->st_mode) && !S_ISDIR(sb->st_mode))
		return 0;

	/* Ignore . and .. */
	if (!strcmp(".", dirent->d_name) || !strcmp("..", dirent->d_name))
		return 0;

	/*
	 * If we were given a dirent, open the associated file under
	 * dir_fd for badblocks scanning.  If dirent is NULL, then it's
	 * the directory itself we want to scan.
	 */
	fd = openat(dir_fd, dirent->d_name,
			O_RDONLY | O_NOATIME | O_NOFOLLOW | O_NOCTTY);
	if (fd < 0) {
		if (errno == ENOENT)
			return 0;
		str_errno(ctx, path);
		return errno;
	}

	/* Go find the badness. */
	error = report_fd_loss(ctx, path, fd, arg);

	err2 = close(fd);
	if (err2)
		str_errno(ctx, path);
	if (!error && err2)
		error = err2;

	return error;
}

/* Use a fsmap to report metadata lost to a media error. */
static int
report_ioerr_fsmap(
	struct scrub_ctx	*ctx,
	struct fsmap		*map,
	void			*arg)
{
	const char		*type;
	char			buf[DESCR_BUFSZ];
	uint64_t		err_physical = *(uint64_t *)arg;
	uint64_t		err_off;

	/* Don't care about unwritten extents. */
	if (map->fmr_flags & FMR_OF_PREALLOC)
		return 0;

	if (err_physical > map->fmr_physical)
		err_off = err_physical - map->fmr_physical;
	else
		err_off = 0;

	/* Report special owners */
	if (map->fmr_flags & FMR_OF_SPECIAL_OWNER) {
		snprintf(buf, DESCR_BUFSZ, _("disk offset %"PRIu64),
				(uint64_t)map->fmr_physical + err_off);
		type = decode_special_owner(map->fmr_owner);
		str_corrupt(ctx, buf, _("media error in %s."), type);
	}

	/* Report extent maps */
	if (map->fmr_flags & FMR_OF_EXTENT_MAP) {
		bool		attr = (map->fmr_flags & FMR_OF_ATTR_FORK);

		scrub_render_ino_descr(ctx, buf, DESCR_BUFSZ,
				map->fmr_owner, 0, " %s",
				attr ? _("extended attribute") :
				       _("file data"));
		str_corrupt(ctx, buf, _("media error in extent map"));
	}

	/*
	 * XXX: If we had a getparent() call we could report IO errors
	 * efficiently.  Until then, we'll have to scan the dir tree
	 * to find the bad file's pathname.
	 */

	return 0;
}

/*
 * For a range of bad blocks, visit each space mapping that overlaps the bad
 * range so that we can report lost metadata.
 */
static int
report_ioerr(
	uint64_t			start,
	uint64_t			length,
	void				*arg)
{
	struct fsmap			keys[2];
	struct disk_ioerr_report	*dioerr = arg;
	dev_t				dev;

	dev = disk_to_dev(dioerr->ctx, dioerr->disk);

	/* Go figure out which blocks are bad from the fsmap. */
	memset(keys, 0, sizeof(struct fsmap) * 2);
	keys->fmr_device = dev;
	keys->fmr_physical = start;
	(keys + 1)->fmr_device = dev;
	(keys + 1)->fmr_physical = start + length - 1;
	(keys + 1)->fmr_owner = ULLONG_MAX;
	(keys + 1)->fmr_offset = ULLONG_MAX;
	(keys + 1)->fmr_flags = UINT_MAX;
	return -scrub_iterate_fsmap(dioerr->ctx, keys, report_ioerr_fsmap,
			&start);
}

/* Report all the media errors found on a disk. */
static int
report_disk_ioerrs(
	struct scrub_ctx		*ctx,
	struct disk			*disk,
	struct media_verify_state	*vs)
{
	struct disk_ioerr_report	dioerr = {
		.ctx			= ctx,
		.disk			= disk,
	};
	struct bitmap			*tree;

	if (!disk)
		return 0;
	tree = bitmap_for_disk(ctx, disk, vs);
	if (!tree)
		return 0;
	return -bitmap_iterate(tree, report_ioerr, &dioerr);
}

/* Given bad extent lists for the data & rtdev, find bad files. */
static int
report_all_media_errors(
	struct scrub_ctx		*ctx,
	struct media_verify_state	*vs)
{
	int				ret;

	ret = report_disk_ioerrs(ctx, ctx->datadev, vs);
	if (ret) {
		str_liberror(ctx, ret, _("walking datadev io errors"));
		return ret;
	}

	ret = report_disk_ioerrs(ctx, ctx->rtdev, vs);
	if (ret) {
		str_liberror(ctx, ret, _("walking rtdev io errors"));
		return ret;
	}

	/* Scan the directory tree to get file paths. */
	ret = scan_fs_tree(ctx, report_dir_loss, report_dirent_loss, vs);
	if (ret)
		return ret;

	/* Scan for unlinked files. */
	return scrub_scan_all_inodes(ctx, report_inode_loss, vs);
}

/* Schedule a read-verify of a (data block) extent. */
static int
check_rmap(
	struct scrub_ctx		*ctx,
	struct fsmap			*map,
	void				*arg)
{
	struct media_verify_state	*vs = arg;
	struct read_verify_pool		*rvp;
	int				ret;

	rvp = dev_to_pool(ctx, vs, map->fmr_device);

	dbg_printf("rmap dev %d:%d phys %"PRIu64" owner %"PRId64
			" offset %"PRIu64" len %"PRIu64" flags 0x%x\n",
			major(map->fmr_device), minor(map->fmr_device),
			(uint64_t)map->fmr_physical, (int64_t)map->fmr_owner,
			(uint64_t)map->fmr_offset, (uint64_t)map->fmr_length,
			map->fmr_flags);

	/* "Unknown" extents should be verified; they could be data. */
	if ((map->fmr_flags & FMR_OF_SPECIAL_OWNER) &&
			map->fmr_owner == XFS_FMR_OWN_UNKNOWN)
		map->fmr_flags &= ~FMR_OF_SPECIAL_OWNER;

	/*
	 * We only care about read-verifying data extents that have been
	 * written to disk.  This means we can skip "special" owners
	 * (metadata), xattr blocks, unwritten extents, and extent maps.
	 * These should all get checked elsewhere in the scrubber.
	 */
	if (map->fmr_flags & (FMR_OF_PREALLOC | FMR_OF_ATTR_FORK |
			      FMR_OF_EXTENT_MAP | FMR_OF_SPECIAL_OWNER))
		return 0;

	/* XXX: Filter out directory data blocks. */

	/* Schedule the read verify command for (eventual) running. */
	ret = read_verify_schedule_io(rvp, map->fmr_physical, map->fmr_length,
			vs);
	if (ret) {
		str_liberror(ctx, ret, _("scheduling media verify command"));
		return ret;
	}

	return 0;
}

/* Wait for read/verify actions to finish, then return # bytes checked. */
static int
clean_pool(
	struct read_verify_pool	*rvp,
	unsigned long long	*bytes_checked)
{
	uint64_t		pool_checked;
	int			ret;

	if (!rvp)
		return 0;

	ret = read_verify_force_io(rvp);
	if (ret)
		return ret;

	ret = read_verify_pool_flush(rvp);
	if (ret)
		goto out_destroy;

	ret = read_verify_bytes(rvp, &pool_checked);
	if (ret)
		goto out_destroy;

	*bytes_checked += pool_checked;
out_destroy:
	read_verify_pool_destroy(rvp);
	return ret;
}

/* Remember a media error for later. */
static void
remember_ioerr(
	struct scrub_ctx		*ctx,
	struct disk			*disk,
	uint64_t			start,
	uint64_t			length,
	int				error,
	void				*arg)
{
	struct media_verify_state	*vs = arg;
	struct bitmap			*tree;
	int				ret;

	tree = bitmap_for_disk(ctx, disk, vs);
	if (!tree) {
		str_liberror(ctx, ENOENT, _("finding bad block bitmap"));
		return;
	}

	ret = -bitmap_set(tree, start, length);
	if (ret)
		str_liberror(ctx, ret, _("setting bad block bitmap"));
}

/*
 * Read verify all the file data blocks in a filesystem.  Since XFS doesn't
 * do data checksums, we trust that the underlying storage will pass back
 * an IO error if it can't retrieve whatever we previously stored there.
 * If we hit an IO error, we'll record the bad blocks in a bitmap and then
 * scan the extent maps of the entire fs tree to figure (and the unlinked
 * inodes) out which files are now broken.
 */
int
phase6_func(
	struct scrub_ctx		*ctx)
{
	struct media_verify_state	vs = { NULL };
	int				ret, ret2, ret3;

	ret = -bitmap_alloc(&vs.d_bad);
	if (ret) {
		str_liberror(ctx, ret, _("creating datadev badblock bitmap"));
		return ret;
	}

	ret = -bitmap_alloc(&vs.r_bad);
	if (ret) {
		str_liberror(ctx, ret, _("creating realtime badblock bitmap"));
		goto out_dbad;
	}

	ret = read_verify_pool_alloc(ctx, ctx->datadev,
			ctx->mnt.fsgeom.blocksize, remember_ioerr,
			scrub_nproc(ctx), &vs.rvp_data);
	if (ret) {
		str_liberror(ctx, ret, _("creating datadev media verifier"));
		goto out_rbad;
	}
	if (ctx->logdev) {
		ret = read_verify_pool_alloc(ctx, ctx->logdev,
				ctx->mnt.fsgeom.blocksize, remember_ioerr,
				scrub_nproc(ctx), &vs.rvp_log);
		if (ret) {
			str_liberror(ctx, ret,
					_("creating logdev media verifier"));
			goto out_datapool;
		}
	}
	if (ctx->rtdev) {
		ret = read_verify_pool_alloc(ctx, ctx->rtdev,
				ctx->mnt.fsgeom.blocksize, remember_ioerr,
				scrub_nproc(ctx), &vs.rvp_realtime);
		if (ret) {
			str_liberror(ctx, ret,
					_("creating rtdev media verifier"));
			goto out_logpool;
		}
	}
	ret = scrub_scan_all_spacemaps(ctx, check_rmap, &vs);
	if (ret)
		goto out_rtpool;

	ret = clean_pool(vs.rvp_data, &ctx->bytes_checked);
	if (ret)
		str_liberror(ctx, ret, _("flushing datadev verify pool"));

	ret2 = clean_pool(vs.rvp_log, &ctx->bytes_checked);
	if (ret2)
		str_liberror(ctx, ret2, _("flushing logdev verify pool"));

	ret3 = clean_pool(vs.rvp_realtime, &ctx->bytes_checked);
	if (ret3)
		str_liberror(ctx, ret3, _("flushing rtdev verify pool"));

	/*
	 * If the verify flush didn't work or we found no bad blocks, we're
	 * done!  No errors detected.
	 */
	if (ret || ret2 || ret3)
		goto out_rbad;
	if (bitmap_empty(vs.d_bad) && bitmap_empty(vs.r_bad))
		goto out_rbad;

	/* Scan the whole dir tree to see what matches the bad extents. */
	ret = report_all_media_errors(ctx, &vs);

	bitmap_free(&vs.r_bad);
	bitmap_free(&vs.d_bad);
	return ret;

out_rtpool:
	if (vs.rvp_realtime) {
		read_verify_pool_abort(vs.rvp_realtime);
		read_verify_pool_destroy(vs.rvp_realtime);
	}
out_logpool:
	if (vs.rvp_log) {
		read_verify_pool_abort(vs.rvp_log);
		read_verify_pool_destroy(vs.rvp_log);
	}
out_datapool:
	read_verify_pool_abort(vs.rvp_data);
	read_verify_pool_destroy(vs.rvp_data);
out_rbad:
	bitmap_free(&vs.r_bad);
out_dbad:
	bitmap_free(&vs.d_bad);
	return ret;
}

/* Estimate how much work we're going to do. */
int
phase6_estimate(
	struct scrub_ctx	*ctx,
	uint64_t		*items,
	unsigned int		*nr_threads,
	int			*rshift)
{
	unsigned long long	d_blocks;
	unsigned long long	d_bfree;
	unsigned long long	r_blocks;
	unsigned long long	r_bfree;
	unsigned long long	f_files;
	unsigned long long	f_free;
	int			ret;

	ret = scrub_scan_estimate_blocks(ctx, &d_blocks, &d_bfree,
				&r_blocks, &r_bfree, &f_files, &f_free);
	if (ret) {
		str_liberror(ctx, ret, _("estimating verify work"));
		return ret;
	}

	*items = cvt_off_fsb_to_b(&ctx->mnt,
			(d_blocks - d_bfree) + (r_blocks - r_bfree));
	*nr_threads = disk_heads(ctx->datadev);
	*rshift = 20;
	return 0;
}
