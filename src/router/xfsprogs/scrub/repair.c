// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "list.h"
#include "libfrog/paths.h"
#include "xfs_scrub.h"
#include "common.h"
#include "scrub.h"
#include "progress.h"
#include "repair.h"

/*
 * Prioritize action items in order of how long we can wait.
 * 0 = do it now, 10000 = do it later.
 *
 * To minimize the amount of repair work, we want to prioritize metadata
 * objects by perceived corruptness.  If CORRUPT is set, the fields are
 * just plain bad; try fixing that first.  Otherwise if XCORRUPT is set,
 * the fields could be bad, but the xref data could also be bad; we'll
 * try fixing that next.  Finally, if XFAIL is set, some other metadata
 * structure failed validation during xref, so we'll recheck this
 * metadata last since it was probably fine.
 *
 * For metadata that lie in the critical path of checking other metadata
 * (superblock, AG{F,I,FL}, inobt) we scrub and fix those things before
 * we even get to handling their dependencies, so things should progress
 * in order.
 */

/* Sort action items in severity order. */
static int
PRIO(
	const struct action_item *aitem,
	int			order)
{
	if (aitem->flags & XFS_SCRUB_OFLAG_CORRUPT)
		return order;
	else if (aitem->flags & XFS_SCRUB_OFLAG_XCORRUPT)
		return 100 + order;
	else if (aitem->flags & XFS_SCRUB_OFLAG_XFAIL)
		return 200 + order;
	else if (aitem->flags & XFS_SCRUB_OFLAG_PREEN)
		return 300 + order;
	abort();
}

/* Sort the repair items in dependency order. */
static int
xfs_action_item_priority(
	const struct action_item	*aitem)
{
	switch (aitem->type) {
	case XFS_SCRUB_TYPE_SB:
	case XFS_SCRUB_TYPE_AGF:
	case XFS_SCRUB_TYPE_AGFL:
	case XFS_SCRUB_TYPE_AGI:
	case XFS_SCRUB_TYPE_BNOBT:
	case XFS_SCRUB_TYPE_CNTBT:
	case XFS_SCRUB_TYPE_INOBT:
	case XFS_SCRUB_TYPE_FINOBT:
	case XFS_SCRUB_TYPE_REFCNTBT:
	case XFS_SCRUB_TYPE_RMAPBT:
	case XFS_SCRUB_TYPE_INODE:
	case XFS_SCRUB_TYPE_BMBTD:
	case XFS_SCRUB_TYPE_BMBTA:
	case XFS_SCRUB_TYPE_BMBTC:
		return PRIO(aitem, aitem->type - 1);
	case XFS_SCRUB_TYPE_DIR:
	case XFS_SCRUB_TYPE_XATTR:
	case XFS_SCRUB_TYPE_SYMLINK:
	case XFS_SCRUB_TYPE_PARENT:
		return PRIO(aitem, XFS_SCRUB_TYPE_DIR);
	case XFS_SCRUB_TYPE_RTBITMAP:
	case XFS_SCRUB_TYPE_RTSUM:
		return PRIO(aitem, XFS_SCRUB_TYPE_RTBITMAP);
	case XFS_SCRUB_TYPE_UQUOTA:
	case XFS_SCRUB_TYPE_GQUOTA:
	case XFS_SCRUB_TYPE_PQUOTA:
		return PRIO(aitem, XFS_SCRUB_TYPE_UQUOTA);
	case XFS_SCRUB_TYPE_FSCOUNTERS:
		/* This should always go after AG headers no matter what. */
		return PRIO(aitem, INT_MAX);
	}
	abort();
}

/* Make sure that btrees get repaired before headers. */
static int
xfs_action_item_compare(
	void				*priv,
	const struct list_head		*a,
	const struct list_head		*b)
{
	const struct action_item	*ra;
	const struct action_item	*rb;

	ra = container_of(a, struct action_item, list);
	rb = container_of(b, struct action_item, list);

	return xfs_action_item_priority(ra) - xfs_action_item_priority(rb);
}

/*
 * Figure out which AG metadata must be fixed before we can move on
 * to the inode scan.
 */
void
action_list_find_mustfix(
	struct action_list		*alist,
	struct action_list		*immediate_alist,
	unsigned long long		*broken_primaries,
	unsigned long long		*broken_secondaries)
{
	struct action_item		*n;
	struct action_item		*aitem;

	list_for_each_entry_safe(aitem, n, &alist->list, list) {
		if (!(aitem->flags & XFS_SCRUB_OFLAG_CORRUPT))
			continue;
		switch (aitem->type) {
		case XFS_SCRUB_TYPE_RMAPBT:
			(*broken_secondaries)++;
			break;
		case XFS_SCRUB_TYPE_FINOBT:
		case XFS_SCRUB_TYPE_INOBT:
			alist->nr--;
			list_move_tail(&aitem->list, &immediate_alist->list);
			immediate_alist->nr++;
			fallthrough;
		case XFS_SCRUB_TYPE_BNOBT:
		case XFS_SCRUB_TYPE_CNTBT:
		case XFS_SCRUB_TYPE_REFCNTBT:
			(*broken_primaries)++;
			break;
		default:
			abort();
			break;
		}
	}
}

/*
 * Allocate a certain number of repair lists for the scrub context.  Returns
 * zero or a positive error number.
 */
int
action_lists_alloc(
	size_t				nr,
	struct action_list		**listsp)
{
	struct action_list		*lists;
	xfs_agnumber_t			agno;

	lists = calloc(nr, sizeof(struct action_list));
	if (!lists)
		return errno;

	for (agno = 0; agno < nr; agno++)
		action_list_init(&lists[agno]);
	*listsp = lists;

	return 0;
}

/* Free the repair lists. */
void
action_lists_free(
	struct action_list		**listsp)
{
	free(*listsp);
	*listsp = NULL;
}

/* Initialize repair list */
void
action_list_init(
	struct action_list		*alist)
{
	INIT_LIST_HEAD(&alist->list);
	alist->nr = 0;
	alist->sorted = false;
}

/* Number of repairs in this list. */
unsigned long long
action_list_length(
	struct action_list		*alist)
{
	return alist->nr;
};

/* Add to the list of repairs. */
void
action_list_add(
	struct action_list		*alist,
	struct action_item		*aitem)
{
	list_add_tail(&aitem->list, &alist->list);
	alist->nr++;
	alist->sorted = false;
}

/* Splice two repair lists. */
void
action_list_splice(
	struct action_list		*dest,
	struct action_list		*src)
{
	if (src->nr == 0)
		return;

	list_splice_tail_init(&src->list, &dest->list);
	dest->nr += src->nr;
	src->nr = 0;
	dest->sorted = false;
}

/* Repair everything on this list. */
int
action_list_process(
	struct scrub_ctx		*ctx,
	int				fd,
	struct action_list		*alist,
	unsigned int			repair_flags)
{
	struct xfs_fd			xfd;
	struct xfs_fd			*xfdp = &ctx->mnt;
	struct action_item		*aitem;
	struct action_item		*n;
	enum check_outcome		fix;

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

	if (!alist->sorted) {
		list_sort(NULL, &alist->list, xfs_action_item_compare);
		alist->sorted = true;
	}

	list_for_each_entry_safe(aitem, n, &alist->list, list) {
		fix = xfs_repair_metadata(ctx, xfdp, aitem, repair_flags);
		switch (fix) {
		case CHECK_DONE:
			if (!(repair_flags & ALP_NOPROGRESS))
				progress_add(1);
			alist->nr--;
			list_del(&aitem->list);
			free(aitem);
			continue;
		case CHECK_ABORT:
			return ECANCELED;
		case CHECK_RETRY:
			continue;
		case CHECK_REPAIR:
			abort();
		}
	}

	if (scrub_excessive_errors(ctx))
		return ECANCELED;
	return 0;
}

/* Defer all the repairs until phase 4. */
void
action_list_defer(
	struct scrub_ctx		*ctx,
	xfs_agnumber_t			agno,
	struct action_list		*alist)
{
	ASSERT(agno < ctx->mnt.fsgeom.agcount);

	action_list_splice(&ctx->action_lists[agno], alist);
}

/* Run actions now and defer unfinished items for later. */
int
action_list_process_or_defer(
	struct scrub_ctx		*ctx,
	xfs_agnumber_t			agno,
	struct action_list		*alist)
{
	int				ret;

	ret = action_list_process(ctx, -1, alist,
			ALP_REPAIR_ONLY | ALP_NOPROGRESS);
	if (ret)
		return ret;

	action_list_defer(ctx, agno, alist);
	return 0;
}
