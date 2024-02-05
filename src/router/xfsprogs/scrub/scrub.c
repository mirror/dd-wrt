// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "list.h"
#include "libfrog/paths.h"
#include "libfrog/fsgeom.h"
#include "libfrog/scrub.h"
#include "xfs_scrub.h"
#include "common.h"
#include "progress.h"
#include "scrub.h"
#include "xfs_errortag.h"
#include "repair.h"
#include "descr.h"

/* Online scrub and repair wrappers. */

/* Format a scrub description. */
static int
format_scrub_descr(
	struct scrub_ctx		*ctx,
	char				*buf,
	size_t				buflen,
	void				*where)
{
	struct xfs_scrub_metadata	*meta = where;
	const struct xfrog_scrub_descr	*sc = &xfrog_scrubbers[meta->sm_type];

	switch (sc->type) {
	case XFROG_SCRUB_TYPE_AGHEADER:
	case XFROG_SCRUB_TYPE_PERAG:
		return snprintf(buf, buflen, _("AG %u %s"), meta->sm_agno,
				_(sc->descr));
		break;
	case XFROG_SCRUB_TYPE_INODE:
		return scrub_render_ino_descr(ctx, buf, buflen,
				meta->sm_ino, meta->sm_gen, "%s",
				_(sc->descr));
		break;
	case XFROG_SCRUB_TYPE_FS:
		return snprintf(buf, buflen, _("%s"), _(sc->descr));
		break;
	case XFROG_SCRUB_TYPE_NONE:
		assert(0);
		break;
	}
	return -1;
}

/* Predicates for scrub flag state. */

static inline bool is_corrupt(struct xfs_scrub_metadata *sm)
{
	return sm->sm_flags & XFS_SCRUB_OFLAG_CORRUPT;
}

static inline bool is_unoptimized(struct xfs_scrub_metadata *sm)
{
	return sm->sm_flags & XFS_SCRUB_OFLAG_PREEN;
}

static inline bool xref_failed(struct xfs_scrub_metadata *sm)
{
	return sm->sm_flags & XFS_SCRUB_OFLAG_XFAIL;
}

static inline bool xref_disagrees(struct xfs_scrub_metadata *sm)
{
	return sm->sm_flags & XFS_SCRUB_OFLAG_XCORRUPT;
}

static inline bool is_incomplete(struct xfs_scrub_metadata *sm)
{
	return sm->sm_flags & XFS_SCRUB_OFLAG_INCOMPLETE;
}

static inline bool is_suspicious(struct xfs_scrub_metadata *sm)
{
	return sm->sm_flags & XFS_SCRUB_OFLAG_WARNING;
}

/* Should we fix it? */
static inline bool needs_repair(struct xfs_scrub_metadata *sm)
{
	return is_corrupt(sm) || xref_disagrees(sm);
}

/* Warn about strange circumstances after scrub. */
static inline void
scrub_warn_incomplete_scrub(
	struct scrub_ctx		*ctx,
	struct descr			*dsc,
	struct xfs_scrub_metadata	*meta)
{
	if (is_incomplete(meta))
		str_info(ctx, descr_render(dsc), _("Check incomplete."));

	if (is_suspicious(meta)) {
		if (debug)
			str_info(ctx, descr_render(dsc),
					_("Possibly suspect metadata."));
		else
			str_warn(ctx, descr_render(dsc),
					_("Possibly suspect metadata."));
	}

	if (xref_failed(meta))
		str_info(ctx, descr_render(dsc),
				_("Cross-referencing failed."));
}

/* Do a read-only check of some metadata. */
static enum check_outcome
xfs_check_metadata(
	struct scrub_ctx		*ctx,
	struct xfs_fd			*xfdp,
	struct xfs_scrub_metadata	*meta,
	bool				is_inode)
{
	DEFINE_DESCR(dsc, ctx, format_scrub_descr);
	unsigned int			tries = 0;
	int				error;

	assert(!debug_tweak_on("XFS_SCRUB_NO_KERNEL"));
	assert(meta->sm_type < XFS_SCRUB_TYPE_NR);
	descr_set(&dsc, meta);

	dbg_printf("check %s flags %xh\n", descr_render(&dsc), meta->sm_flags);
retry:
	error = -xfrog_scrub_metadata(xfdp, meta);
	if (debug_tweak_on("XFS_SCRUB_FORCE_REPAIR") && !error)
		meta->sm_flags |= XFS_SCRUB_OFLAG_CORRUPT;
	switch (error) {
	case 0:
		/* No operational errors encountered. */
		break;
	case ENOENT:
		/* Metadata not present, just skip it. */
		return CHECK_DONE;
	case ESHUTDOWN:
		/* FS already crashed, give up. */
		str_error(ctx, descr_render(&dsc),
_("Filesystem is shut down, aborting."));
		return CHECK_ABORT;
	case EIO:
	case ENOMEM:
		/* Abort on I/O errors or insufficient memory. */
		str_liberror(ctx, error, descr_render(&dsc));
		return CHECK_ABORT;
	case EDEADLOCK:
	case EBUSY:
	case EFSBADCRC:
	case EFSCORRUPTED:
		/*
		 * The first two should never escape the kernel,
		 * and the other two should be reported via sm_flags.
		 */
		str_liberror(ctx, error, _("Kernel bug"));
		return CHECK_DONE;
	default:
		/* Operational error. */
		str_liberror(ctx, error, descr_render(&dsc));
		return CHECK_DONE;
	}

	/*
	 * If the kernel says the test was incomplete or that there was
	 * a cross-referencing discrepancy but no obvious corruption,
	 * we'll try the scan again, just in case the fs was busy.
	 * Only retry so many times.
	 */
	if (tries < 10 && (is_incomplete(meta) ||
			   (xref_disagrees(meta) && !is_corrupt(meta)))) {
		tries++;
		goto retry;
	}

	/* Complain about incomplete or suspicious metadata. */
	scrub_warn_incomplete_scrub(ctx, &dsc, meta);

	/*
	 * If we need repairs or there were discrepancies, schedule a
	 * repair if desired, otherwise complain.
	 */
	if (is_corrupt(meta) || xref_disagrees(meta)) {
		if (ctx->mode < SCRUB_MODE_REPAIR) {
			str_corrupt(ctx, descr_render(&dsc),
_("Repairs are required."));
			return CHECK_DONE;
		}

		return CHECK_REPAIR;
	}

	/*
	 * If we could optimize, schedule a repair if desired,
	 * otherwise complain.
	 */
	if (is_unoptimized(meta)) {
		if (ctx->mode != SCRUB_MODE_REPAIR) {
			if (!is_inode) {
				/* AG or FS metadata, always warn. */
				str_info(ctx, descr_render(&dsc),
_("Optimization is possible."));
			} else if (!ctx->preen_triggers[meta->sm_type]) {
				/* File metadata, only warn once per type. */
				pthread_mutex_lock(&ctx->lock);
				if (!ctx->preen_triggers[meta->sm_type])
					ctx->preen_triggers[meta->sm_type] = true;
				pthread_mutex_unlock(&ctx->lock);
			}
			return CHECK_DONE;
		}

		return CHECK_REPAIR;
	}

	/*
	 * This metadata object itself looks ok, but we noticed inconsistencies
	 * when comparing it with the other filesystem metadata.  If we're in
	 * repair mode we need to queue it for a "repair" so that phase 4 will
	 * re-examine the object as repairs progress to see if the kernel will
	 * deem it completely consistent at some point.
	 */
	if (xref_failed(meta) && ctx->mode == SCRUB_MODE_REPAIR)
		return CHECK_REPAIR;

	/* Everything is ok. */
	return CHECK_DONE;
}

/* Bulk-notify user about things that could be optimized. */
void
scrub_report_preen_triggers(
	struct scrub_ctx		*ctx)
{
	int				i;

	for (i = 0; i < XFS_SCRUB_TYPE_NR; i++) {
		pthread_mutex_lock(&ctx->lock);
		if (ctx->preen_triggers[i]) {
			ctx->preen_triggers[i] = false;
			pthread_mutex_unlock(&ctx->lock);
			str_info(ctx, ctx->mntpoint,
_("Optimizations of %s are possible."), _(xfrog_scrubbers[i].descr));
		} else {
			pthread_mutex_unlock(&ctx->lock);
		}
	}
}

/* Save a scrub context for later repairs. */
static int
scrub_save_repair(
	struct scrub_ctx		*ctx,
	struct action_list		*alist,
	struct xfs_scrub_metadata	*meta)
{
	struct action_item		*aitem;

	/* Schedule this item for later repairs. */
	aitem = malloc(sizeof(struct action_item));
	if (!aitem) {
		str_errno(ctx, _("adding item to repair list"));
		return errno;
	}

	memset(aitem, 0, sizeof(*aitem));
	aitem->type = meta->sm_type;
	aitem->flags = meta->sm_flags;
	switch (xfrog_scrubbers[meta->sm_type].type) {
	case XFROG_SCRUB_TYPE_AGHEADER:
	case XFROG_SCRUB_TYPE_PERAG:
		aitem->agno = meta->sm_agno;
		break;
	case XFROG_SCRUB_TYPE_INODE:
		aitem->ino = meta->sm_ino;
		aitem->gen = meta->sm_gen;
		break;
	default:
		break;
	}

	action_list_add(alist, aitem);
	return 0;
}

/*
 * Scrub a single XFS_SCRUB_TYPE_*, saving corruption reports for later.
 *
 * Returns 0 for success.  If errors occur, this function will log them and
 * return a positive error code.
 */
static int
scrub_meta_type(
	struct scrub_ctx		*ctx,
	unsigned int			type,
	xfs_agnumber_t			agno,
	struct action_list		*alist)
{
	struct xfs_scrub_metadata	meta = {
		.sm_type		= type,
		.sm_agno		= agno,
	};
	enum check_outcome		fix;
	int				ret;

	background_sleep();

	/* Check the item. */
	fix = xfs_check_metadata(ctx, &ctx->mnt, &meta, false);
	progress_add(1);

	switch (fix) {
	case CHECK_ABORT:
		return ECANCELED;
	case CHECK_REPAIR:
		ret = scrub_save_repair(ctx, alist, &meta);
		if (ret)
			return ret;
		fallthrough;
	case CHECK_DONE:
		return 0;
	default:
		/* CHECK_RETRY should never happen. */
		abort();
	}
}

/*
 * Scrub all metadata types that are assigned to the given XFROG_SCRUB_TYPE_*,
 * saving corruption reports for later.  This should not be used for
 * XFROG_SCRUB_TYPE_INODE or for checking summary metadata.
 */
static bool
scrub_all_types(
	struct scrub_ctx		*ctx,
	enum xfrog_scrub_type		scrub_type,
	xfs_agnumber_t			agno,
	struct action_list		*alist)
{
	const struct xfrog_scrub_descr	*sc;
	unsigned int			type;

	sc = xfrog_scrubbers;
	for (type = 0; type < XFS_SCRUB_TYPE_NR; type++, sc++) {
		int			ret;

		if (sc->type != scrub_type)
			continue;
		if (sc->flags & XFROG_SCRUB_DESCR_SUMMARY)
			continue;

		ret = scrub_meta_type(ctx, type, agno, alist);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Scrub primary superblock.  This will be useful if we ever need to hook
 * a filesystem-wide pre-scrub activity off of the sb 0 scrubber (which
 * currently does nothing).  If errors occur, this function will log them and
 * return nonzero.
 */
int
scrub_primary_super(
	struct scrub_ctx		*ctx,
	struct action_list		*alist)
{
	return scrub_meta_type(ctx, XFS_SCRUB_TYPE_SB, 0, alist);
}

/* Scrub each AG's header blocks. */
int
scrub_ag_headers(
	struct scrub_ctx		*ctx,
	xfs_agnumber_t			agno,
	struct action_list		*alist)
{
	return scrub_all_types(ctx, XFROG_SCRUB_TYPE_AGHEADER, agno, alist);
}

/* Scrub each AG's metadata btrees. */
int
scrub_ag_metadata(
	struct scrub_ctx		*ctx,
	xfs_agnumber_t			agno,
	struct action_list		*alist)
{
	return scrub_all_types(ctx, XFROG_SCRUB_TYPE_PERAG, agno, alist);
}

/* Scrub whole-FS metadata btrees. */
int
scrub_fs_metadata(
	struct scrub_ctx		*ctx,
	struct action_list		*alist)
{
	return scrub_all_types(ctx, XFROG_SCRUB_TYPE_FS, 0, alist);
}

/* Scrub FS summary metadata. */
int
scrub_fs_summary(
	struct scrub_ctx		*ctx,
	struct action_list		*alist)
{
	return scrub_meta_type(ctx, XFS_SCRUB_TYPE_FSCOUNTERS, 0, alist);
}

/* How many items do we have to check? */
unsigned int
scrub_estimate_ag_work(
	struct scrub_ctx		*ctx)
{
	const struct xfrog_scrub_descr	*sc;
	int				type;
	unsigned int			estimate = 0;

	sc = xfrog_scrubbers;
	for (type = 0; type < XFS_SCRUB_TYPE_NR; type++, sc++) {
		switch (sc->type) {
		case XFROG_SCRUB_TYPE_AGHEADER:
		case XFROG_SCRUB_TYPE_PERAG:
			estimate += ctx->mnt.fsgeom.agcount;
			break;
		case XFROG_SCRUB_TYPE_FS:
			estimate++;
			break;
		default:
			break;
		}
	}
	return estimate;
}

/*
 * Scrub file metadata of some sort.  If errors occur, this function will log
 * them and return nonzero.
 */
int
scrub_file(
	struct scrub_ctx		*ctx,
	int				fd,
	const struct xfs_bulkstat	*bstat,
	unsigned int			type,
	struct action_list		*alist)
{
	struct xfs_scrub_metadata	meta = {0};
	struct xfs_fd			xfd;
	struct xfs_fd			*xfdp = &ctx->mnt;
	enum check_outcome		fix;

	assert(type < XFS_SCRUB_TYPE_NR);
	assert(xfrog_scrubbers[type].type == XFROG_SCRUB_TYPE_INODE);

	meta.sm_type = type;
	meta.sm_ino = bstat->bs_ino;
	meta.sm_gen = bstat->bs_gen;

	/*
	 * If the caller passed us a file descriptor for a scrub, use it
	 * instead of scrub-by-handle because this enables the kernel to skip
	 * costly inode btree lookups.
	 */
	if (fd >= 0) {
		memcpy(&xfd, xfdp, sizeof(xfd));
		xfd.fd = fd;
		xfdp = &xfd;
	}

	/* Scrub the piece of metadata. */
	fix = xfs_check_metadata(ctx, xfdp, &meta, true);
	if (fix == CHECK_ABORT)
		return ECANCELED;
	if (fix == CHECK_DONE)
		return 0;

	return scrub_save_repair(ctx, alist, &meta);
}

/*
 * Test the availability of a kernel scrub command.  If errors occur (or the
 * scrub ioctl is rejected) the errors will be logged and this function will
 * return false.
 */
static bool
__scrub_test(
	struct scrub_ctx		*ctx,
	unsigned int			type,
	bool				repair)
{
	struct xfs_scrub_metadata	meta = {0};
	struct xfs_error_injection	inject;
	static bool			injected;
	int				error;

	if (debug_tweak_on("XFS_SCRUB_NO_KERNEL"))
		return false;
	if (debug_tweak_on("XFS_SCRUB_FORCE_REPAIR") && !injected) {
		inject.fd = ctx->mnt.fd;
		inject.errtag = XFS_ERRTAG_FORCE_SCRUB_REPAIR;
		error = ioctl(ctx->mnt.fd, XFS_IOC_ERROR_INJECTION, &inject);
		if (error == 0)
			injected = true;
	}

	meta.sm_type = type;
	if (repair)
		meta.sm_flags |= XFS_SCRUB_IFLAG_REPAIR;
	error = -xfrog_scrub_metadata(&ctx->mnt, &meta);
	switch (error) {
	case 0:
		return true;
	case EROFS:
		str_info(ctx, ctx->mntpoint,
_("Filesystem is mounted read-only; cannot proceed."));
		return false;
	case ENOTRECOVERABLE:
		str_info(ctx, ctx->mntpoint,
_("Filesystem is mounted norecovery; cannot proceed."));
		return false;
	case EOPNOTSUPP:
	case ENOTTY:
		if (debug || verbose)
			str_info(ctx, ctx->mntpoint,
_("Kernel %s %s facility not detected."),
					_(xfrog_scrubbers[type].descr),
					repair ? _("repair") : _("scrub"));
		return false;
	case ENOENT:
		/* Scrubber says not present on this fs; that's fine. */
		return true;
	default:
		str_info(ctx, ctx->mntpoint, "%s", strerror(errno));
		return true;
	}
}

bool
can_scrub_fs_metadata(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_PROBE, false);
}

bool
can_scrub_inode(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_INODE, false);
}

bool
can_scrub_bmap(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_BMBTD, false);
}

bool
can_scrub_dir(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_DIR, false);
}

bool
can_scrub_attr(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_XATTR, false);
}

bool
can_scrub_symlink(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_SYMLINK, false);
}

bool
can_scrub_parent(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_PARENT, false);
}

bool
xfs_can_repair(
	struct scrub_ctx	*ctx)
{
	return __scrub_test(ctx, XFS_SCRUB_TYPE_PROBE, true);
}

/* General repair routines. */

/* Repair some metadata. */
enum check_outcome
xfs_repair_metadata(
	struct scrub_ctx		*ctx,
	struct xfs_fd			*xfdp,
	struct action_item		*aitem,
	unsigned int			repair_flags)
{
	struct xfs_scrub_metadata	meta = { 0 };
	struct xfs_scrub_metadata	oldm;
	DEFINE_DESCR(dsc, ctx, format_scrub_descr);
	int				error;

	assert(aitem->type < XFS_SCRUB_TYPE_NR);
	assert(!debug_tweak_on("XFS_SCRUB_NO_KERNEL"));
	meta.sm_type = aitem->type;
	meta.sm_flags = aitem->flags | XFS_SCRUB_IFLAG_REPAIR;
	switch (xfrog_scrubbers[aitem->type].type) {
	case XFROG_SCRUB_TYPE_AGHEADER:
	case XFROG_SCRUB_TYPE_PERAG:
		meta.sm_agno = aitem->agno;
		break;
	case XFROG_SCRUB_TYPE_INODE:
		meta.sm_ino = aitem->ino;
		meta.sm_gen = aitem->gen;
		break;
	default:
		break;
	}

	if (!is_corrupt(&meta) && (repair_flags & XRM_REPAIR_ONLY))
		return CHECK_RETRY;

	memcpy(&oldm, &meta, sizeof(oldm));
	descr_set(&dsc, &oldm);

	if (needs_repair(&meta))
		str_info(ctx, descr_render(&dsc), _("Attempting repair."));
	else if (debug || verbose)
		str_info(ctx, descr_render(&dsc),
				_("Attempting optimization."));

	error = -xfrog_scrub_metadata(xfdp, &meta);
	switch (error) {
	case 0:
		/* No operational errors encountered. */
		break;
	case EDEADLOCK:
	case EBUSY:
		/* Filesystem is busy, try again later. */
		if (debug || verbose)
			str_info(ctx, descr_render(&dsc),
_("Filesystem is busy, deferring repair."));
		return CHECK_RETRY;
	case ESHUTDOWN:
		/* Filesystem is already shut down, abort. */
		str_error(ctx, descr_render(&dsc),
_("Filesystem is shut down, aborting."));
		return CHECK_ABORT;
	case ENOTTY:
	case EOPNOTSUPP:
		/*
		 * If we're in no-complain mode, requeue the check for
		 * later.  It's possible that an error in another
		 * component caused us to flag an error in this
		 * component.  Even if the kernel didn't think it
		 * could fix this, it's at least worth trying the scan
		 * again to see if another repair fixed it.
		 */
		if (!(repair_flags & XRM_COMPLAIN_IF_UNFIXED))
			return CHECK_RETRY;
		/*
		 * If we forced repairs or this is a preen, don't
		 * error out if the kernel doesn't know how to fix.
		 */
		if (is_unoptimized(&oldm) ||
		    debug_tweak_on("XFS_SCRUB_FORCE_REPAIR"))
			return CHECK_DONE;
		fallthrough;
	case EINVAL:
		/* Kernel doesn't know how to repair this? */
		str_corrupt(ctx, descr_render(&dsc),
_("Don't know how to fix; offline repair required."));
		return CHECK_DONE;
	case EROFS:
		/* Read-only filesystem, can't fix. */
		if (verbose || debug || needs_repair(&oldm))
			str_error(ctx, descr_render(&dsc),
_("Read-only filesystem; cannot make changes."));
		return CHECK_ABORT;
	case ENOENT:
		/* Metadata not present, just skip it. */
		return CHECK_DONE;
	case ENOMEM:
	case ENOSPC:
		/* Don't care if preen fails due to low resources. */
		if (is_unoptimized(&oldm) && !needs_repair(&oldm))
			return CHECK_DONE;
		fallthrough;
	default:
		/*
		 * Operational error.  If the caller doesn't want us
		 * to complain about repair failures, tell the caller
		 * to requeue the repair for later and don't say a
		 * thing.  Otherwise, print error and bail out.
		 */
		if (!(repair_flags & XRM_COMPLAIN_IF_UNFIXED))
			return CHECK_RETRY;
		str_liberror(ctx, error, descr_render(&dsc));
		return CHECK_DONE;
	}

	if (repair_flags & XRM_COMPLAIN_IF_UNFIXED)
		scrub_warn_incomplete_scrub(ctx, &dsc, &meta);
	if (needs_repair(&meta)) {
		/*
		 * Still broken; if we've been told not to complain then we
		 * just requeue this and try again later.  Otherwise we
		 * log the error loudly and don't try again.
		 */
		if (!(repair_flags & XRM_COMPLAIN_IF_UNFIXED))
			return CHECK_RETRY;
		str_corrupt(ctx, descr_render(&dsc),
_("Repair unsuccessful; offline repair required."));
	} else if (xref_failed(&meta)) {
		/*
		 * This metadata object itself looks ok, but we still noticed
		 * inconsistencies when comparing it with the other filesystem
		 * metadata.  If we're in "final warning" mode, advise the
		 * caller to run xfs_repair; otherwise, we'll keep trying to
		 * reverify the cross-referencing as repairs progress.
		 */
		if (repair_flags & XRM_COMPLAIN_IF_UNFIXED) {
			str_info(ctx, descr_render(&dsc),
 _("Seems correct but cross-referencing failed; offline repair recommended."));
		} else {
			if (verbose)
				str_info(ctx, descr_render(&dsc),
 _("Seems correct but cross-referencing failed; will keep checking."));
			return CHECK_RETRY;
		}
	} else {
		/* Clean operation, no corruption detected. */
		if (needs_repair(&oldm))
			record_repair(ctx, descr_render(&dsc),
					_("Repairs successful."));
		else
			record_preen(ctx, descr_render(&dsc),
					_("Optimization successful."));
	}
	return CHECK_DONE;
}
