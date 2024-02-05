// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005-2006 Silicon Graphics, Inc.
 * Copyright (C) 2010 Red Hat, Inc.
 * All Rights Reserved.
 */

#include "libxfs_priv.h"
#include "xfs_fs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_mount.h"
#include "xfs_inode_buf.h"
#include "xfs_inode_fork.h"
#include "xfs_inode.h"
#include "xfs_trans.h"
#include "xfs_sb.h"
#include "xfs_defer.h"
#include "xfs_trace.h"

static void xfs_trans_free_items(struct xfs_trans *tp);
STATIC struct xfs_trans *xfs_trans_dup(struct xfs_trans *tp);
static int xfs_trans_reserve(struct xfs_trans *tp, struct xfs_trans_res *resp,
		uint blocks, uint rtextents);
static int __xfs_trans_commit(struct xfs_trans *tp, bool regrant);

/*
 * Simple transaction interface
 */

struct kmem_cache	*xfs_trans_cache;

/*
 * Initialize the precomputed transaction reservation values
 * in the mount structure.
 */
void
libxfs_trans_init(
	struct xfs_mount	*mp)
{
	xfs_trans_resv_calc(mp, &mp->m_resv);
}

/*
 * Add the given log item to the transaction's list of log items.
 */
void
libxfs_trans_add_item(
	struct xfs_trans	*tp,
	struct xfs_log_item	*lip)
{
	ASSERT(lip->li_mountp == tp->t_mountp);
	ASSERT(lip->li_ailp == tp->t_mountp->m_ail);
	ASSERT(list_empty(&lip->li_trans));
	ASSERT(!test_bit(XFS_LI_DIRTY, &lip->li_flags));

	list_add_tail(&lip->li_trans, &tp->t_items);
}

/*
 * Unlink and free the given descriptor.
 */
void
libxfs_trans_del_item(
	struct xfs_log_item	*lip)
{
	clear_bit(XFS_LI_DIRTY, &lip->li_flags);
	list_del_init(&lip->li_trans);
}

/*
 * Roll from one trans in the sequence of PERMANENT transactions to
 * the next: permanent transactions are only flushed out when
 * committed with XFS_TRANS_RELEASE_LOG_RES, but we still want as soon
 * as possible to let chunks of it go to the log. So we commit the
 * chunk we've been working on and get a new transaction to continue.
 */
int
libxfs_trans_roll(
	struct xfs_trans	**tpp)
{
	struct xfs_trans	*trans = *tpp;
	struct xfs_trans_res	tres;
	int			error;

	/*
	 * Copy the critical parameters from one trans to the next.
	 */
	tres.tr_logres = trans->t_log_res;
	tres.tr_logcount = trans->t_log_count;

	*tpp = xfs_trans_dup(trans);

	/*
	 * Commit the current transaction.
	 * If this commit failed, then it'd just unlock those items that
	 * are marked to be released. That also means that a filesystem shutdown
	 * is in progress. The caller takes the responsibility to cancel
	 * the duplicate transaction that gets returned.
	 */
	error = __xfs_trans_commit(trans, true);
	if (error)
		return error;

	/*
	 * Reserve space in the log for the next transaction.
	 * This also pushes items in the "AIL", the list of logged items,
	 * out to disk if they are taking up space at the tail of the log
	 * that we want to use.  This requires that either nothing be locked
	 * across this call, or that anything that is locked be logged in
	 * the prior and the next transactions.
	 */
	tres.tr_logflags = XFS_TRANS_PERM_LOG_RES;
	return xfs_trans_reserve(*tpp, &tres, 0, 0);
}

/*
 * Free the transaction structure.  If there is more clean up
 * to do when the structure is freed, add it here.
 */
static void
xfs_trans_free(
	struct xfs_trans	*tp)
{
	kmem_cache_free(xfs_trans_cache, tp);
}

/*
 * This is called to create a new transaction which will share the
 * permanent log reservation of the given transaction.  The remaining
 * unused block and rt extent reservations are also inherited.  This
 * implies that the original transaction is no longer allowed to allocate
 * blocks.  Locks and log items, however, are no inherited.  They must
 * be added to the new transaction explicitly.
 */
STATIC struct xfs_trans *
xfs_trans_dup(
	struct xfs_trans	*tp)
{
	struct xfs_trans	*ntp;

	ntp = kmem_cache_zalloc(xfs_trans_cache, 0);

	/*
	 * Initialize the new transaction structure.
	 */
	ntp->t_mountp = tp->t_mountp;
	INIT_LIST_HEAD(&ntp->t_items);
	INIT_LIST_HEAD(&ntp->t_dfops);
	ntp->t_highest_agno = NULLAGNUMBER;

	ASSERT(tp->t_flags & XFS_TRANS_PERM_LOG_RES);

	ntp->t_flags = XFS_TRANS_PERM_LOG_RES |
		       (tp->t_flags & XFS_TRANS_RESERVE) |
		       (tp->t_flags & XFS_TRANS_NO_WRITECOUNT);
	/* We gave our writer reference to the new transaction */
	tp->t_flags |= XFS_TRANS_NO_WRITECOUNT;

	ntp->t_blk_res = tp->t_blk_res - tp->t_blk_res_used;
	tp->t_blk_res = tp->t_blk_res_used;

	/* move deferred ops over to the new tp */
	xfs_defer_move(ntp, tp);

	return ntp;
}

/*
 * This is called to reserve free disk blocks and log space for the
 * given transaction.  This must be done before allocating any resources
 * within the transaction.
 *
 * This will return ENOSPC if there are not enough blocks available.
 * It will sleep waiting for available log space.
 * The only valid value for the flags parameter is XFS_RES_LOG_PERM, which
 * is used by long running transactions.  If any one of the reservations
 * fails then they will all be backed out.
 *
 * This does not do quota reservations. That typically is done by the
 * caller afterwards.
 */
static int
xfs_trans_reserve(
	struct xfs_trans	*tp,
	struct xfs_trans_res	*resp,
	uint			blocks,
	uint			rtextents)
{
	int			error = 0;

	/*
	 * Attempt to reserve the needed disk blocks by decrementing
	 * the number needed from the number available.  This will
	 * fail if the count would go below zero.
	 */
	if (blocks > 0) {
		if (tp->t_mountp->m_sb.sb_fdblocks < blocks)
			return -ENOSPC;
		tp->t_blk_res += blocks;
	}

	/*
	 * Reserve the log space needed for this transaction.
	 */
	if (resp->tr_logres > 0) {
		ASSERT(tp->t_log_res == 0 ||
		       tp->t_log_res == resp->tr_logres);
		ASSERT(tp->t_log_count == 0 ||
		       tp->t_log_count == resp->tr_logcount);

		if (resp->tr_logflags & XFS_TRANS_PERM_LOG_RES)
			tp->t_flags |= XFS_TRANS_PERM_LOG_RES;
		else
			ASSERT(!(tp->t_flags & XFS_TRANS_PERM_LOG_RES));

		tp->t_log_res = resp->tr_logres;
		tp->t_log_count = resp->tr_logcount;
	}

	/*
	 * Attempt to reserve the needed realtime extents by decrementing
	 * the number needed from the number available.  This will
	 * fail if the count would go below zero.
	 */
	if (rtextents > 0) {
		if (tp->t_mountp->m_sb.sb_rextents < rtextents) {
			error = -ENOSPC;
			goto undo_blocks;
		}
		tp->t_rtx_res += rtextents;
	}

	return 0;

	/*
	 * Error cases jump to one of these labels to undo any
	 * reservations which have already been performed.
	 */
undo_blocks:
	if (blocks > 0)
		tp->t_blk_res = 0;

	return error;
}

int
libxfs_trans_alloc(
	struct xfs_mount	*mp,
	struct xfs_trans_res	*resp,
	unsigned int		blocks,
	unsigned int		rtextents,
	unsigned int		flags,
	struct xfs_trans	**tpp)

{
	struct xfs_trans	*tp;
	int			error;

	tp = kmem_cache_zalloc(xfs_trans_cache, 0);
	tp->t_mountp = mp;
	INIT_LIST_HEAD(&tp->t_items);
	INIT_LIST_HEAD(&tp->t_dfops);
	tp->t_highest_agno = NULLAGNUMBER;

	error = xfs_trans_reserve(tp, resp, blocks, rtextents);
	if (error) {
		xfs_trans_cancel(tp);
		return error;
	}

	trace_xfs_trans_alloc(tp, _RET_IP_);

	*tpp = tp;
	return 0;
}

/*
 * Create an empty transaction with no reservation.  This is a defensive
 * mechanism for routines that query metadata without actually modifying
 * them -- if the metadata being queried is somehow cross-linked (think a
 * btree block pointer that points higher in the tree), we risk deadlock.
 * However, blocks grabbed as part of a transaction can be re-grabbed.
 * The verifiers will notice the corrupt block and the operation will fail
 * back to userspace without deadlocking.
 *
 * Note the zero-length reservation; this transaction MUST be cancelled
 * without any dirty data.
 */
int
libxfs_trans_alloc_empty(
	struct xfs_mount		*mp,
	struct xfs_trans		**tpp)
{
	struct xfs_trans_res		resv = {0};

	return xfs_trans_alloc(mp, &resv, 0, 0, XFS_TRANS_NO_WRITECOUNT, tpp);
}

/*
 * Allocate a transaction that can be rolled.  Since userspace doesn't have
 * a need for log reservations, we really only tr_itruncate to get the
 * permanent log reservation flag to avoid blowing asserts.
 */
int
libxfs_trans_alloc_rollable(
	struct xfs_mount	*mp,
	unsigned int		blocks,
	struct xfs_trans	**tpp)
{
	return libxfs_trans_alloc(mp, &M_RES(mp)->tr_itruncate, blocks,
			0, 0, tpp);
}

void
libxfs_trans_cancel(
	struct xfs_trans	*tp)
{
	bool			dirty;

	trace_xfs_trans_cancel(tp, _RET_IP_);

	if (tp == NULL)
		return;
	dirty = (tp->t_flags & XFS_TRANS_DIRTY);

	/*
	 * It's never valid to cancel a transaction with deferred ops attached,
	 * because the transaction is effectively dirty.  Complain about this
	 * loudly before freeing the in-memory defer items.
	 */
	if (!list_empty(&tp->t_dfops)) {
		ASSERT(list_empty(&tp->t_dfops));
		ASSERT(tp->t_flags & XFS_TRANS_PERM_LOG_RES);
		dirty = true;
		xfs_defer_cancel(tp);
	}

	if (dirty) {
		fprintf(stderr, _("Cancelling dirty transaction!\n"));
		abort();
	}

	xfs_trans_free_items(tp);
	xfs_trans_free(tp);
}

static void
xfs_buf_item_put(
	struct xfs_buf_log_item	*bip)
{
	struct xfs_buf		*bp = bip->bli_buf;

	bp->b_log_item = NULL;
	kmem_cache_free(xfs_buf_item_cache, bip);
}

/* from xfs_trans_buf.c */

/*
 * Add the locked buffer to the transaction.
 *
 * The buffer must be locked, and it cannot be associated with any
 * transaction.
 *
 * If the buffer does not yet have a buf log item associated with it,
 * then allocate one for it.  Then add the buf item to the transaction.
 */
STATIC void
_libxfs_trans_bjoin(
	struct xfs_trans	*tp,
	struct xfs_buf		*bp,
	int			reset_recur)
{
	struct xfs_buf_log_item	*bip;

	ASSERT(bp->b_transp == NULL);

	/*
	 * The xfs_buf_log_item pointer is stored in b_log_item.  If
	 * it doesn't have one yet, then allocate one and initialize it.
	 * The checks to see if one is there are in xfs_buf_item_init().
	 */
	xfs_buf_item_init(bp, tp->t_mountp);
	bip = bp->b_log_item;
	if (reset_recur)
		bip->bli_recur = 0;

	/*
	 * Attach the item to the transaction so we can find it in
	 * xfs_trans_get_buf() and friends.
	 */
	xfs_trans_add_item(tp, &bip->bli_item);
	bp->b_transp = tp;

}

void
libxfs_trans_bjoin(
	struct xfs_trans	*tp,
	struct xfs_buf		*bp)
{
	_libxfs_trans_bjoin(tp, bp, 0);
	trace_xfs_trans_bjoin(bp->b_log_item);
}

/*
 * Cancel the previous buffer hold request made on this buffer
 * for this transaction.
 */
void
libxfs_trans_bhold_release(
	xfs_trans_t		*tp,
	struct xfs_buf		*bp)
{
	struct xfs_buf_log_item *bip = bp->b_log_item;

	ASSERT(bp->b_transp == tp);
	ASSERT(bip != NULL);

	bip->bli_flags &= ~XFS_BLI_HOLD;
	trace_xfs_trans_bhold_release(bip);
}

/*
 * Get and lock the buffer for the caller if it is not already
 * locked within the given transaction.  If it is already locked
 * within the transaction, just increment its lock recursion count
 * and return a pointer to it.
 *
 * If the transaction pointer is NULL, make this just a normal
 * get_buf() call.
 */
int
libxfs_trans_get_buf_map(
	struct xfs_trans	*tp,
	struct xfs_buftarg	*target,
	struct xfs_buf_map	*map,
	int			nmaps,
	xfs_buf_flags_t		flags,
	struct xfs_buf		**bpp)
{
	struct xfs_buf		*bp;
	struct xfs_buf_log_item	*bip;
	int			error;

	*bpp = NULL;
	if (!tp)
		return libxfs_buf_get_map(target, map, nmaps, 0, bpp);

	/*
	 * If we find the buffer in the cache with this transaction
	 * pointer in its b_fsprivate2 field, then we know we already
	 * have it locked.  In this case we just increment the lock
	 * recursion count and return the buffer to the caller.
	 */
	bp = xfs_trans_buf_item_match(tp, target, map, nmaps);
	if (bp != NULL) {
		ASSERT(bp->b_transp == tp);
		bip = bp->b_log_item;
		ASSERT(bip != NULL);
		bip->bli_recur++;
		trace_xfs_trans_get_buf_recur(bip);
		*bpp = bp;
		return 0;
	}

	error = libxfs_buf_get_map(target, map, nmaps, 0, &bp);
	if (error)
		return error;

	ASSERT(!bp->b_error);

	_libxfs_trans_bjoin(tp, bp, 1);
	trace_xfs_trans_get_buf(bp->b_log_item);
	*bpp = bp;
	return 0;
}

struct xfs_buf *
libxfs_trans_getsb(
	struct xfs_trans	*tp)
{
	struct xfs_mount	*mp = tp->t_mountp;
	struct xfs_buf		*bp;
	struct xfs_buf_log_item	*bip;
	int			len = XFS_FSS_TO_BB(mp, 1);
	DEFINE_SINGLE_BUF_MAP(map, XFS_SB_DADDR, len);

	if (tp == NULL)
		return libxfs_getsb(mp);

	bp = xfs_trans_buf_item_match(tp, mp->m_dev, &map, 1);
	if (bp != NULL) {
		ASSERT(bp->b_transp == tp);
		bip = bp->b_log_item;
		ASSERT(bip != NULL);
		bip->bli_recur++;
		trace_xfs_trans_getsb_recur(bip);
		return bp;
	}

	bp = libxfs_getsb(mp);
	if (bp == NULL)
		return NULL;

	_libxfs_trans_bjoin(tp, bp, 1);
	trace_xfs_trans_getsb(bp->b_log_item);
	return bp;
}

int
libxfs_trans_read_buf_map(
	struct xfs_mount	*mp,
	struct xfs_trans	*tp,
	struct xfs_buftarg	*target,
	struct xfs_buf_map	*map,
	int			nmaps,
	xfs_buf_flags_t		flags,
	struct xfs_buf		**bpp,
	const struct xfs_buf_ops *ops)
{
	struct xfs_buf		*bp;
	struct xfs_buf_log_item	*bip;
	int			error;

	*bpp = NULL;

	if (tp == NULL)
		return libxfs_buf_read_map(target, map, nmaps, flags, bpp, ops);

	bp = xfs_trans_buf_item_match(tp, target, map, nmaps);
	if (bp) {
		ASSERT(bp->b_transp == tp);
		ASSERT(bp->b_log_item != NULL);
		bip = bp->b_log_item;
		bip->bli_recur++;
		trace_xfs_trans_read_buf_recur(bip);
		goto done;
	}

	error = libxfs_buf_read_map(target, map, nmaps, flags, &bp, ops);
	if (error)
		return error;

	_libxfs_trans_bjoin(tp, bp, 1);
done:
	trace_xfs_trans_read_buf(bp->b_log_item);
	*bpp = bp;
	return 0;
}

/*
 * Release a buffer previously joined to the transaction. If the buffer is
 * modified within this transaction, decrement the recursion count but do not
 * release the buffer even if the count goes to 0. If the buffer is not modified
 * within the transaction, decrement the recursion count and release the buffer
 * if the recursion count goes to 0.
 *
 * If the buffer is to be released and it was not already dirty before this
 * transaction began, then also free the buf_log_item associated with it.
 *
 * If the transaction pointer is NULL, this is a normal xfs_buf_relse() call.
 */
void
libxfs_trans_brelse(
	struct xfs_trans	*tp,
	struct xfs_buf		*bp)
{
	struct xfs_buf_log_item	*bip = bp->b_log_item;

	ASSERT(bp->b_transp == tp);

	if (!tp) {
		libxfs_buf_relse(bp);
		return;
	}

	trace_xfs_trans_brelse(bip);
	ASSERT(bip->bli_item.li_type == XFS_LI_BUF);

	/*
	 * If the release is for a recursive lookup, then decrement the count
	 * and return.
	 */
	if (bip->bli_recur > 0) {
		bip->bli_recur--;
		return;
	}

	/*
	 * If the buffer is invalidated or dirty in this transaction, we can't
	 * release it until we commit.
	 */
	if (test_bit(XFS_LI_DIRTY, &bip->bli_item.li_flags))
		return;
	if (bip->bli_flags & XFS_BLI_STALE)
		return;

	/*
	 * Unlink the log item from the transaction and clear the hold flag, if
	 * set. We wouldn't want the next user of the buffer to get confused.
	 */
	xfs_trans_del_item(&bip->bli_item);
	bip->bli_flags &= ~XFS_BLI_HOLD;

	/* drop the reference to the bli */
	xfs_buf_item_put(bip);

	bp->b_transp = NULL;
	libxfs_buf_relse(bp);
}

/*
 * Mark the buffer as not needing to be unlocked when the buf item's
 * iop_unlock() routine is called.  The buffer must already be locked
 * and associated with the given transaction.
 */
/* ARGSUSED */
void
libxfs_trans_bhold(
	xfs_trans_t		*tp,
	struct xfs_buf		*bp)
{
	struct xfs_buf_log_item	*bip = bp->b_log_item;

	ASSERT(bp->b_transp == tp);
	ASSERT(bip != NULL);

	bip->bli_flags |= XFS_BLI_HOLD;
	trace_xfs_trans_bhold(bip);
}

/*
 * Mark a buffer dirty in the transaction.
 */
void
libxfs_trans_dirty_buf(
	struct xfs_trans	*tp,
	struct xfs_buf		*bp)
{
	struct xfs_buf_log_item	*bip = bp->b_log_item;

	ASSERT(bp->b_transp == tp);
	ASSERT(bip != NULL);

	tp->t_flags |= XFS_TRANS_DIRTY;
	set_bit(XFS_LI_DIRTY, &bip->bli_item.li_flags);
}

/*
 * This is called to mark bytes first through last inclusive of the given
 * buffer as needing to be logged when the transaction is committed.
 * The buffer must already be associated with the given transaction.
 *
 * First and last are numbers relative to the beginning of this buffer,
 * so the first byte in the buffer is numbered 0 regardless of the
 * value of b_blkno.
 */
void
libxfs_trans_log_buf(
	struct xfs_trans	*tp,
	struct xfs_buf		*bp,
	uint			first,
	uint			last)
{
	struct xfs_buf_log_item	*bip = bp->b_log_item;

	ASSERT(first <= last && last < BBTOB(bp->b_length));

	xfs_trans_dirty_buf(tp, bp);

	trace_xfs_trans_log_buf(bip);
	xfs_buf_item_log(bip, first, last);
}

void
libxfs_trans_binval(
	xfs_trans_t		*tp,
	struct xfs_buf		*bp)
{
	struct xfs_buf_log_item	*bip = bp->b_log_item;

	ASSERT(bp->b_transp == tp);
	ASSERT(bip != NULL);

	trace_xfs_trans_binval(bip);

	if (bip->bli_flags & XFS_BLI_STALE)
		return;
	XFS_BUF_UNDELAYWRITE(bp);
	xfs_buf_stale(bp);

	bip->bli_flags |= XFS_BLI_STALE;
	bip->bli_flags &= ~XFS_BLI_DIRTY;
	bip->__bli_format.blf_flags &= ~XFS_BLF_INODE_BUF;
	bip->__bli_format.blf_flags |= XFS_BLF_CANCEL;
	set_bit(XFS_LI_DIRTY, &bip->bli_item.li_flags);
	tp->t_flags |= XFS_TRANS_DIRTY;
}

/*
 * Mark the buffer as being one which contains newly allocated
 * inodes.  We need to make sure that even if this buffer is
 * relogged as an 'inode buf' we still recover all of the inode
 * images in the face of a crash.  This works in coordination with
 * xfs_buf_item_committed() to ensure that the buffer remains in the
 * AIL at its original location even after it has been relogged.
 */
/* ARGSUSED */
void
libxfs_trans_inode_alloc_buf(
	xfs_trans_t		*tp,
	struct xfs_buf		*bp)
{
	struct xfs_buf_log_item	*bip = bp->b_log_item;

	ASSERT(bp->b_transp == tp);
	ASSERT(bip != NULL);
	bip->bli_flags |= XFS_BLI_INODE_ALLOC_BUF;
	xfs_trans_buf_set_type(tp, bp, XFS_BLFT_DINO_BUF);
}

/*
 * For userspace, ordered buffers just need to be marked dirty so
 * the transaction commit will write them and mark them up-to-date.
 * In essence, they are just like any other logged buffer in userspace.
 *
 * If the buffer is already dirty, trigger the "already logged" return condition.
 */
bool
libxfs_trans_ordered_buf(
	struct xfs_trans	*tp,
	struct xfs_buf		*bp)
{
	struct xfs_buf_log_item	*bip = bp->b_log_item;
	bool			ret;

	ret = test_bit(XFS_LI_DIRTY, &bip->bli_item.li_flags);
	libxfs_trans_log_buf(tp, bp, 0, BBTOB(bp->b_length));
	return ret;
}

/* end of xfs_trans_buf.c */

/*
 * Record the indicated change to the given field for application
 * to the file system's superblock when the transaction commits.
 * For now, just store the change in the transaction structure.
 * Mark the transaction structure to indicate that the superblock
 * needs to be updated before committing.
 *
 * Originally derived from xfs_trans_mod_sb().
 */
void
libxfs_trans_mod_sb(
	xfs_trans_t		*tp,
	uint			field,
	long			delta)
{
	switch (field) {
	case XFS_TRANS_SB_RES_FDBLOCKS:
		return;
	case XFS_TRANS_SB_FDBLOCKS:
		if (delta < 0) {
			tp->t_blk_res_used += (uint)-delta;
			if (tp->t_blk_res_used > tp->t_blk_res) {
				fprintf(stderr,
_("Transaction block reservation exceeded! %u > %u\n"),
					tp->t_blk_res_used, tp->t_blk_res);
				ASSERT(0);
			}
		}
		tp->t_fdblocks_delta += delta;
		break;
	case XFS_TRANS_SB_ICOUNT:
		ASSERT(delta > 0);
		tp->t_icount_delta += delta;
		break;
	case XFS_TRANS_SB_IFREE:
		tp->t_ifree_delta += delta;
		break;
	case XFS_TRANS_SB_FREXTENTS:
		/*
		 * Track the number of rt extents allocated in the transaction.
		 * Make sure it does not exceed the number reserved.
		 */
		if (delta < 0) {
			tp->t_rtx_res_used += (uint)-delta;
			if (tp->t_rtx_res_used > tp->t_rtx_res) {
				fprintf(stderr,
_("Transaction rt block reservation exceeded! %u > %u\n"),
					tp->t_rtx_res_used, tp->t_rtx_res);
				ASSERT(0);
			}
		}
		tp->t_frextents_delta += delta;
		break;
	default:
		ASSERT(0);
		return;
	}
	tp->t_flags |= (XFS_TRANS_SB_DIRTY | XFS_TRANS_DIRTY);
}

static void
xfs_inode_item_put(
	struct xfs_inode_log_item	*iip)
{
	struct xfs_inode		*ip = iip->ili_inode;

	ASSERT(iip->ili_item.li_buf == NULL);

	ip->i_itemp = NULL;

	list_del_init(&iip->ili_item.li_bio_list);
	kmem_cache_free(xfs_ili_cache, iip);
}


/*
 * Transaction commital code follows (i.e. write to disk in libxfs)
 *
 * XXX (dgc): should failure to flush the inode (e.g. due to uncorrected
 * corruption) result in transaction commit failure w/ EFSCORRUPTED?
 */
static void
inode_item_done(
	struct xfs_inode_log_item	*iip)
{
	struct xfs_buf			*bp;
	int				error;

	ASSERT(iip->ili_inode != NULL);

	if (!(iip->ili_fields & XFS_ILOG_ALL))
		goto free_item;

	bp = iip->ili_item.li_buf;
	iip->ili_item.li_buf = NULL;

	/*
	 * Flush the inode and disassociate it from the transaction regardless
	 * of whether the flush succeed or not. If we fail the flush, make sure
	 * we still release the buffer reference we currently hold.
	 */
	error = libxfs_iflush_int(iip->ili_inode, bp);
	bp->b_transp = NULL;	/* remove xact ptr */

	if (error) {
		fprintf(stderr, _("%s: warning - iflush_int failed (%d)\n"),
			progname, error);
		goto free;
	}

	libxfs_buf_mark_dirty(bp);
free:
	libxfs_buf_relse(bp);
free_item:
	xfs_inode_item_put(iip);
}

static void
buf_item_done(
	xfs_buf_log_item_t	*bip)
{
	struct xfs_buf		*bp;
	int			hold;
	extern struct kmem_cache	*xfs_buf_item_cache;

	bp = bip->bli_buf;
	ASSERT(bp != NULL);
	bp->b_transp = NULL;			/* remove xact ptr */

	hold = (bip->bli_flags & XFS_BLI_HOLD);
	if (bip->bli_flags & XFS_BLI_DIRTY)
		libxfs_buf_mark_dirty(bp);

	bip->bli_flags &= ~XFS_BLI_HOLD;
	xfs_buf_item_put(bip);
	if (hold)
		return;
	libxfs_buf_relse(bp);
}

static void
trans_committed(
	xfs_trans_t		*tp)
{
	struct xfs_log_item	*lip, *next;

	list_for_each_entry_safe(lip, next, &tp->t_items, li_trans) {
		xfs_trans_del_item(lip);

		if (lip->li_type == XFS_LI_BUF)
			buf_item_done((xfs_buf_log_item_t *)lip);
		else if (lip->li_type == XFS_LI_INODE)
			inode_item_done((struct xfs_inode_log_item *)lip);
		else {
			fprintf(stderr, _("%s: unrecognised log item type\n"),
				progname);
			ASSERT(0);
		}
	}
}

static void
buf_item_unlock(
	xfs_buf_log_item_t	*bip)
{
	struct xfs_buf		*bp = bip->bli_buf;
	uint			hold;

	/* Clear the buffer's association with this transaction. */
	bip->bli_buf->b_transp = NULL;

	hold = bip->bli_flags & XFS_BLI_HOLD;
	bip->bli_flags &= ~XFS_BLI_HOLD;
	xfs_buf_item_put(bip);
	if (!hold)
		libxfs_buf_relse(bp);
}

static void
inode_item_unlock(
	struct xfs_inode_log_item	*iip)
{
	xfs_inode_item_put(iip);
}

/* Detach and unlock all of the items in a transaction */
static void
xfs_trans_free_items(
	struct xfs_trans	*tp)
{
	struct xfs_log_item	*lip, *next;

	list_for_each_entry_safe(lip, next, &tp->t_items, li_trans) {
		xfs_trans_del_item(lip);
		if (lip->li_type == XFS_LI_BUF)
			buf_item_unlock((xfs_buf_log_item_t *)lip);
		else if (lip->li_type == XFS_LI_INODE)
			inode_item_unlock((struct xfs_inode_log_item *)lip);
		else {
			fprintf(stderr, _("%s: unrecognised log item type\n"),
				progname);
			ASSERT(0);
		}
	}
}

/*
 * Sort transaction items prior to running precommit operations. This will
 * attempt to order the items such that they will always be locked in the same
 * order. Items that have no sort function are moved to the end of the list
 * and so are locked last.
 *
 * This may need refinement as different types of objects add sort functions.
 *
 * Function is more complex than it needs to be because we are comparing 64 bit
 * values and the function only returns 32 bit values.
 */
static int
xfs_trans_precommit_sort(
	void			*unused_arg,
	const struct list_head	*a,
	const struct list_head	*b)
{
	struct xfs_log_item	*lia = container_of(a,
					struct xfs_log_item, li_trans);
	struct xfs_log_item	*lib = container_of(b,
					struct xfs_log_item, li_trans);
	int64_t			diff;

	/*
	 * If both items are non-sortable, leave them alone. If only one is
	 * sortable, move the non-sortable item towards the end of the list.
	 */
	if (!lia->li_ops->iop_sort && !lib->li_ops->iop_sort)
		return 0;
	if (!lia->li_ops->iop_sort)
		return 1;
	if (!lib->li_ops->iop_sort)
		return -1;

	diff = lia->li_ops->iop_sort(lia) - lib->li_ops->iop_sort(lib);
	if (diff < 0)
		return -1;
	if (diff > 0)
		return 1;
	return 0;
}

/*
 * Run transaction precommit functions.
 *
 * If there is an error in any of the callouts, then stop immediately and
 * trigger a shutdown to abort the transaction. There is no recovery possible
 * from errors at this point as the transaction is dirty....
 */
static int
xfs_trans_run_precommits(
	struct xfs_trans	*tp)
{
	//struct xfs_mount	*mp = tp->t_mountp;
	struct xfs_log_item	*lip, *n;
	int			error = 0;

	/*
	 * Sort the item list to avoid ABBA deadlocks with other transactions
	 * running precommit operations that lock multiple shared items such as
	 * inode cluster buffers.
	 */
	list_sort(NULL, &tp->t_items, xfs_trans_precommit_sort);

	/*
	 * Precommit operations can remove the log item from the transaction
	 * if the log item exists purely to delay modifications until they
	 * can be ordered against other operations. Hence we have to use
	 * list_for_each_entry_safe() here.
	 */
	list_for_each_entry_safe(lip, n, &tp->t_items, li_trans) {
		if (!test_bit(XFS_LI_DIRTY, &lip->li_flags))
			continue;
		if (lip->li_ops->iop_precommit) {
			error = lip->li_ops->iop_precommit(tp, lip);
			if (error)
				break;
		}
	}
	if (error)
		xfs_force_shutdown(mp, SHUTDOWN_CORRUPT_INCORE);
	return error;
}

/*
 * Commit the changes represented by this transaction
 */
static int
__xfs_trans_commit(
	struct xfs_trans	*tp,
	bool			regrant)
{
	struct xfs_sb		*sbp;
	int			error = 0;

	trace_xfs_trans_commit(tp, _RET_IP_);

	if (tp == NULL)
		return 0;

	error = xfs_trans_run_precommits(tp);
	if (error) {
		if (tp->t_flags & XFS_TRANS_PERM_LOG_RES)
			xfs_defer_cancel(tp);
		goto out_unreserve;
	}

	/*
	 * Finish deferred items on final commit. Only permanent transactions
	 * should ever have deferred ops.
	 */
	WARN_ON_ONCE(!list_empty(&tp->t_dfops) &&
		     !(tp->t_flags & XFS_TRANS_PERM_LOG_RES));
	if (!regrant && (tp->t_flags & XFS_TRANS_PERM_LOG_RES)) {
		error = xfs_defer_finish_noroll(&tp);
		if (error)
			goto out_unreserve;

		/* Run precommits from final tx in defer chain. */
		error = xfs_trans_run_precommits(tp);
		if (error)
			goto out_unreserve;
	}

	if (!(tp->t_flags & XFS_TRANS_DIRTY))
		goto out_unreserve;

	if (tp->t_flags & XFS_TRANS_SB_DIRTY) {
		sbp = &(tp->t_mountp->m_sb);
		if (tp->t_icount_delta)
			sbp->sb_icount += tp->t_icount_delta;
		if (tp->t_ifree_delta)
			sbp->sb_ifree += tp->t_ifree_delta;
		if (tp->t_fdblocks_delta)
			sbp->sb_fdblocks += tp->t_fdblocks_delta;
		if (tp->t_frextents_delta)
			sbp->sb_frextents += tp->t_frextents_delta;
		xfs_log_sb(tp);
	}

	trans_committed(tp);

	/* That's it for the transaction structure.  Free it. */
	xfs_trans_free(tp);
	return 0;

out_unreserve:
	xfs_trans_free_items(tp);
	xfs_trans_free(tp);
	return error;
}

int
libxfs_trans_commit(
	struct xfs_trans	*tp)
{
	return __xfs_trans_commit(tp, false);
}

/*
 * Allocate an transaction, lock and join the inode to it, and reserve quota.
 *
 * The caller must ensure that the on-disk dquots attached to this inode have
 * already been allocated and initialized.  The caller is responsible for
 * releasing ILOCK_EXCL if a new transaction is returned.
 */
int
libxfs_trans_alloc_inode(
	struct xfs_inode	*ip,
	struct xfs_trans_res	*resv,
	unsigned int		dblocks,
	unsigned int		rblocks,
	bool			force,
	struct xfs_trans	**tpp)
{
	struct xfs_trans	*tp;
	struct xfs_mount	*mp = ip->i_mount;
	int			error;

	error = libxfs_trans_alloc(mp, resv, dblocks,
			rblocks / mp->m_sb.sb_rextsize,
			force ? XFS_TRANS_RESERVE : 0, &tp);
	if (error)
		return error;

	xfs_ilock(ip, XFS_ILOCK_EXCL);
	xfs_trans_ijoin(tp, ip, 0);

	*tpp = tp;
	return 0;
}
