/*
 * Copyright (c) 2000-2001,2005-2006 Silicon Graphics, Inc.
 * Copyright (C) 2010 Red Hat, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

static void xfs_trans_free_items(struct xfs_trans *tp);

/*
 * Simple transaction interface
 */

kmem_zone_t	*xfs_log_item_desc_zone;

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
 *
 * The log item will now point to its new descriptor with its li_desc field.
 */
void
libxfs_trans_add_item(
	struct xfs_trans	*tp,
	struct xfs_log_item	*lip)
{
	struct xfs_log_item_desc *lidp;

	ASSERT(lip->li_mountp == tp->t_mountp);
	ASSERT(lip->li_ailp == tp->t_mountp->m_ail);

	lidp = calloc(sizeof(struct xfs_log_item_desc), 1);
	if (!lidp) {
		fprintf(stderr, _("%s: lidp calloc failed (%d bytes): %s\n"),
			progname, (int)sizeof(struct xfs_log_item_desc),
			strerror(errno));
		exit(1);
	}

	lidp->lid_item = lip;
	lidp->lid_flags = 0;
	list_add_tail(&lidp->lid_trans, &tp->t_items);

	lip->li_desc = lidp;
}

/*
 * Unlink and free the given descriptor.
 */
void
libxfs_trans_del_item(
	struct xfs_log_item	*lip)
{
	list_del_init(&lip->li_desc->lid_trans);
	free(lip->li_desc);
	lip->li_desc = NULL;
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
	struct xfs_mount	*mp;
	struct xfs_trans	*trans = *tpp;
	struct xfs_trans_res	tres;
	unsigned int		old_blk_res;
	int			error;

	/*
	 * Copy the critical parameters from one trans to the next.
	 */
	mp = trans->t_mountp;
	tres.tr_logres = trans->t_log_res;
	tres.tr_logcount = trans->t_log_count;
	old_blk_res = trans->t_blk_res;

	/*
	 * Commit the current transaction.
	 * If this commit failed, then it'd just unlock those items that
	 * are marked to be released. That also means that a filesystem shutdown
	 * is in progress. The caller takes the responsibility to cancel
	 * the duplicate transaction that gets returned.
	 */
	error = xfs_trans_commit(trans);
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
	error = libxfs_trans_alloc(mp, &tres, 0, 0, 0, tpp);
	trans = *tpp;
	trans->t_blk_res = old_blk_res;

	return 0;
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
	struct xfs_sb	*sb = &mp->m_sb;
	struct xfs_trans *ptr;

	/*
	 * Attempt to reserve the needed disk blocks by decrementing
	 * the number needed from the number available.	 This will
	 * fail if the count would go below zero.
	 */
	if (blocks > 0) {
		if (sb->sb_fdblocks < blocks)
			return -ENOSPC;
	}

	if ((ptr = calloc(sizeof(xfs_trans_t), 1)) == NULL) {
		fprintf(stderr, _("%s: xact calloc failed (%d bytes): %s\n"),
			progname, (int)sizeof(xfs_trans_t), strerror(errno));
		exit(1);
	}
	ptr->t_mountp = mp;
	ptr->t_blk_res = blocks;
	INIT_LIST_HEAD(&ptr->t_items);
#ifdef XACT_DEBUG
	fprintf(stderr, "allocated new transaction %p\n", ptr);
#endif
	*tpp = ptr;
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

void
libxfs_trans_cancel(
	xfs_trans_t	*tp)
{
#ifdef XACT_DEBUG
	xfs_trans_t	*otp = tp;
#endif
	if (tp != NULL) {
		xfs_trans_free_items(tp);
		free(tp);
		tp = NULL;
	}
#ifdef XACT_DEBUG
	fprintf(stderr, "## cancelled transaction %p\n", otp);
#endif
}

int
libxfs_trans_iget(
	xfs_mount_t		*mp,
	xfs_trans_t		*tp,
	xfs_ino_t		ino,
	uint			flags,
	uint			lock_flags,
	xfs_inode_t		**ipp)
{
	int			error;
	xfs_inode_t		*ip;
	xfs_inode_log_item_t	*iip;

	if (tp == NULL)
		return libxfs_iget(mp, tp, ino, lock_flags, ipp);

	error = libxfs_iget(mp, tp, ino, lock_flags, &ip);
	if (error)
		return error;
	ASSERT(ip != NULL);

	if (ip->i_itemp == NULL)
		xfs_inode_item_init(ip, mp);
	iip = ip->i_itemp;
	xfs_trans_add_item(tp, (xfs_log_item_t *)(iip));

	/* initialize i_transp so we can find it incore */
	ip->i_transp = tp;

	*ipp = ip;
	return 0;
}

void
libxfs_trans_ijoin(
	xfs_trans_t		*tp,
	xfs_inode_t		*ip,
	uint			lock_flags)
{
	xfs_inode_log_item_t	*iip;

	ASSERT(ip->i_transp == NULL);
	if (ip->i_itemp == NULL)
		xfs_inode_item_init(ip, ip->i_mount);
	iip = ip->i_itemp;
	ASSERT(iip->ili_flags == 0);
	ASSERT(iip->ili_inode != NULL);

	xfs_trans_add_item(tp, (xfs_log_item_t *)(iip));

	ip->i_transp = tp;
#ifdef XACT_DEBUG
	fprintf(stderr, "ijoin'd inode %llu, transaction %p\n", ip->i_ino, tp);
#endif
}

void
libxfs_trans_ijoin_ref(
	xfs_trans_t		*tp,
	xfs_inode_t		*ip,
	int			lock_flags)
{
	ASSERT(ip->i_transp == tp);
	ASSERT(ip->i_itemp != NULL);

	xfs_trans_ijoin(tp, ip, lock_flags);

#ifdef XACT_DEBUG
	fprintf(stderr, "ijoin_ref'd inode %llu, transaction %p\n", ip->i_ino, tp);
#endif
}

void
libxfs_trans_inode_alloc_buf(
	xfs_trans_t		*tp,
	xfs_buf_t		*bp)
{
	xfs_buf_log_item_t	*bip;

	ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
	ASSERT(XFS_BUF_FSPRIVATE(bp, void *) != NULL);
	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
	bip->bli_flags |= XFS_BLI_INODE_ALLOC_BUF;
	xfs_trans_buf_set_type(tp, bp, XFS_BLFT_DINO_BUF);
}

/*
 * This is called to mark the fields indicated in fieldmask as needing
 * to be logged when the transaction is committed.  The inode must
 * already be associated with the given transaction.
 *
 * The values for fieldmask are defined in xfs_log_format.h.  We always
 * log all of the core inode if any of it has changed, and we always log
 * all of the inline data/extents/b-tree root if any of them has changed.
 */
void
xfs_trans_log_inode(
	xfs_trans_t		*tp,
	xfs_inode_t		*ip,
	uint			flags)
{
	ASSERT(ip->i_transp == tp);
	ASSERT(ip->i_itemp != NULL);
#ifdef XACT_DEBUG
	fprintf(stderr, "dirtied inode %llu, transaction %p\n", ip->i_ino, tp);
#endif

	tp->t_flags |= XFS_TRANS_DIRTY;
	ip->i_itemp->ili_item.li_desc->lid_flags |= XFS_LID_DIRTY;

	/*
	 * Always OR in the bits from the ili_last_fields field.
	 * This is to coordinate with the xfs_iflush() and xfs_iflush_done()
	 * routines in the eventual clearing of the ilf_fields bits.
	 * See the big comment in xfs_iflush() for an explanation of
	 * this coordination mechanism.
	 */
	flags |= ip->i_itemp->ili_last_fields;
	ip->i_itemp->ili_fields |= flags;
}

int
libxfs_trans_roll_inode(
	struct xfs_trans	**tpp,
	struct xfs_inode	*ip)
{
	int			error;

	xfs_trans_log_inode(*tpp, ip, XFS_ILOG_CORE);
	error = xfs_trans_roll(tpp);
	if (!error)
		xfs_trans_ijoin(*tpp, ip, 0);
	return error;
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
	xfs_trans_t		*tp,
	xfs_buf_t		*bp,
	uint			first,
	uint			last)
{
	xfs_buf_log_item_t	*bip;

	ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
	ASSERT(XFS_BUF_FSPRIVATE(bp, void *) != NULL);
	ASSERT((first <= last) && (last < XFS_BUF_COUNT(bp)));
#ifdef XACT_DEBUG
	fprintf(stderr, "dirtied buffer %p, transaction %p\n", bp, tp);
#endif

	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);

	tp->t_flags |= XFS_TRANS_DIRTY;
	bip->bli_item.li_desc->lid_flags |= XFS_LID_DIRTY;
	xfs_buf_item_log(bip, first, last);
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
	struct xfs_buf_log_item	*bip = bp->b_fspriv;
	bool			ret;

	ret = (bip->bli_item.li_desc->lid_flags & XFS_LID_DIRTY);
	libxfs_trans_log_buf(tp, bp, 0, bp->b_bcount);
	return ret;
}

void
libxfs_trans_brelse(
	xfs_trans_t		*tp,
	xfs_buf_t		*bp)
{
	xfs_buf_log_item_t	*bip;
#ifdef XACT_DEBUG
	fprintf(stderr, "released buffer %p, transaction %p\n", bp, tp);
#endif

	if (tp == NULL) {
		ASSERT(XFS_BUF_FSPRIVATE2(bp, void *) == NULL);
		libxfs_putbuf(bp);
		return;
	}
	ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
	ASSERT(bip->bli_item.li_type == XFS_LI_BUF);
	if (bip->bli_recur > 0) {
		bip->bli_recur--;
		return;
	}
	/* If dirty/stale, can't release till transaction committed */
	if (bip->bli_flags & XFS_BLI_STALE)
		return;
	if (bip->bli_item.li_desc->lid_flags & XFS_LID_DIRTY)
		return;
	xfs_trans_del_item(&bip->bli_item);
	if (bip->bli_flags & XFS_BLI_HOLD)
		bip->bli_flags &= ~XFS_BLI_HOLD;
	XFS_BUF_SET_FSPRIVATE2(bp, NULL);
	libxfs_putbuf(bp);
}

void
libxfs_trans_binval(
	xfs_trans_t		*tp,
	xfs_buf_t		*bp)
{
	xfs_buf_log_item_t	*bip;
#ifdef XACT_DEBUG
	fprintf(stderr, "binval'd buffer %p, transaction %p\n", bp, tp);
#endif

	ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
	ASSERT(XFS_BUF_FSPRIVATE(bp, void *) != NULL);

	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
	if (bip->bli_flags & XFS_BLI_STALE)
		return;
	XFS_BUF_UNDELAYWRITE(bp);
	xfs_buf_stale(bp);
	bip->bli_flags |= XFS_BLI_STALE;
	bip->bli_flags &= ~XFS_BLI_DIRTY;
	bip->bli_format.blf_flags &= ~XFS_BLF_INODE_BUF;
	bip->bli_format.blf_flags |= XFS_BLF_CANCEL;
	bip->bli_item.li_desc->lid_flags |= XFS_LID_DIRTY;
	tp->t_flags |= XFS_TRANS_DIRTY;
}

void
libxfs_trans_bjoin(
	xfs_trans_t		*tp,
	xfs_buf_t		*bp)
{
	xfs_buf_log_item_t	*bip;

	ASSERT(XFS_BUF_FSPRIVATE2(bp, void *) == NULL);
#ifdef XACT_DEBUG
	fprintf(stderr, "bjoin'd buffer %p, transaction %p\n", bp, tp);
#endif

	xfs_buf_item_init(bp, tp->t_mountp);
	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
	xfs_trans_add_item(tp, (xfs_log_item_t *)bip);
	XFS_BUF_SET_FSPRIVATE2(bp, tp);
}

void
libxfs_trans_bhold(
	xfs_trans_t		*tp,
	xfs_buf_t		*bp)
{
	xfs_buf_log_item_t	*bip;

	ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
	ASSERT(XFS_BUF_FSPRIVATE(bp, void *) != NULL);
#ifdef XACT_DEBUG
	fprintf(stderr, "bhold'd buffer %p, transaction %p\n", bp, tp);
#endif

	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
	bip->bli_flags |= XFS_BLI_HOLD;
}

xfs_buf_t *
libxfs_trans_get_buf_map(
	xfs_trans_t		*tp,
	struct xfs_buftarg	*btp,
	struct xfs_buf_map	*map,
	int			nmaps,
	uint			f)
{
	xfs_buf_t		*bp;
	xfs_buf_log_item_t	*bip;

	if (tp == NULL)
		return libxfs_getbuf_map(btp, map, nmaps, 0);

	bp = xfs_trans_buf_item_match(tp, btp, map, nmaps);
	if (bp != NULL) {
		ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
		bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
		ASSERT(bip != NULL);
		bip->bli_recur++;
		return bp;
	}

	bp = libxfs_getbuf_map(btp, map, nmaps, 0);
	if (bp == NULL)
		return NULL;
#ifdef XACT_DEBUG
	fprintf(stderr, "trans_get_buf buffer %p, transaction %p\n", bp, tp);
#endif

	xfs_buf_item_init(bp, tp->t_mountp);
	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t*);
	bip->bli_recur = 0;
	xfs_trans_add_item(tp, (xfs_log_item_t *)bip);

	/* initialize b_fsprivate2 so we can find it incore */
	XFS_BUF_SET_FSPRIVATE2(bp, tp);
	return bp;
}

xfs_buf_t *
libxfs_trans_getsb(
	xfs_trans_t		*tp,
	xfs_mount_t		*mp,
	int			flags)
{
	xfs_buf_t		*bp;
	xfs_buf_log_item_t	*bip;
	int			len = XFS_FSS_TO_BB(mp, 1);
	DEFINE_SINGLE_BUF_MAP(map, XFS_SB_DADDR, len);

	if (tp == NULL)
		return libxfs_getsb(mp, flags);

	bp = xfs_trans_buf_item_match(tp, mp->m_dev, &map, 1);
	if (bp != NULL) {
		ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
		bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
		ASSERT(bip != NULL);
		bip->bli_recur++;
		return bp;
	}

	bp = libxfs_getsb(mp, flags);
#ifdef XACT_DEBUG
	fprintf(stderr, "trans_get_sb buffer %p, transaction %p\n", bp, tp);
#endif

	xfs_buf_item_init(bp, mp);
	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t*);
	bip->bli_recur = 0;
	xfs_trans_add_item(tp, (xfs_log_item_t *)bip);

	/* initialize b_fsprivate2 so we can find it incore */
	XFS_BUF_SET_FSPRIVATE2(bp, tp);
	return bp;
}

int
libxfs_trans_read_buf_map(
	xfs_mount_t		*mp,
	xfs_trans_t		*tp,
	struct xfs_buftarg	*btp,
	struct xfs_buf_map	*map,
	int			nmaps,
	uint			flags,
	xfs_buf_t		**bpp,
	const struct xfs_buf_ops *ops)
{
	xfs_buf_t		*bp;
	xfs_buf_log_item_t	*bip;
	int			error;

	*bpp = NULL;

	if (tp == NULL) {
		bp = libxfs_readbuf_map(btp, map, nmaps, flags, ops);
		if (!bp) {
			return (flags & XBF_TRYLOCK) ?  -EAGAIN : -ENOMEM;
		}
		if (bp->b_error)
			goto out_relse;
		goto done;
	}

	bp = xfs_trans_buf_item_match(tp, btp, map, nmaps);
	if (bp != NULL) {
		ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
		ASSERT(XFS_BUF_FSPRIVATE(bp, void *) != NULL);
		bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t*);
		bip->bli_recur++;
		goto done;
	}

	bp = libxfs_readbuf_map(btp, map, nmaps, flags, ops);
	if (!bp) {
		return (flags & XBF_TRYLOCK) ?  -EAGAIN : -ENOMEM;
	}
	if (bp->b_error)
		goto out_relse;

#ifdef XACT_DEBUG
	fprintf(stderr, "trans_read_buf buffer %p, transaction %p\n", bp, tp);
#endif

	xfs_buf_item_init(bp, tp->t_mountp);
	bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
	bip->bli_recur = 0;
	xfs_trans_add_item(tp, (xfs_log_item_t *)bip);

	/* initialise b_fsprivate2 so we can find it incore */
	XFS_BUF_SET_FSPRIVATE2(bp, tp);
done:
	*bpp = bp;
	return 0;
out_relse:
	error = bp->b_error;
	xfs_buf_relse(bp);
	return error;
}

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
		tp->t_frextents_delta += delta;
		break;
	default:
		ASSERT(0);
		return;
	}
	tp->t_flags |= (XFS_TRANS_SB_DIRTY | XFS_TRANS_DIRTY);
}


/*
 * Transaction commital code follows (i.e. write to disk in libxfs)
 */

static void
inode_item_done(
	xfs_inode_log_item_t	*iip)
{
	xfs_dinode_t		*dip;
	xfs_inode_t		*ip;
	xfs_mount_t		*mp;
	xfs_buf_t		*bp;
	int			error;

	ip = iip->ili_inode;
	mp = iip->ili_item.li_mountp;
	ASSERT(ip != NULL);

	if (!(iip->ili_fields & XFS_ILOG_ALL)) {
		ip->i_transp = NULL;	/* disassociate from transaction */
		iip->ili_flags = 0;	/* reset all flags */
		return;
	}

	/*
	 * Get the buffer containing the on-disk inode.
	 */
	error = xfs_imap_to_bp(mp, NULL, &ip->i_imap, &dip, &bp, 0, 0);
	if (error) {
		fprintf(stderr, _("%s: warning - imap_to_bp failed (%d)\n"),
			progname, error);
		return;
	}

	XFS_BUF_SET_FSPRIVATE(bp, iip);
	error = libxfs_iflush_int(ip, bp);
	if (error) {
		fprintf(stderr, _("%s: warning - iflush_int failed (%d)\n"),
			progname, error);
		return;
	}

	ip->i_transp = NULL;	/* disassociate from transaction */
	XFS_BUF_SET_FSPRIVATE(bp, NULL);	/* remove log item */
	XFS_BUF_SET_FSPRIVATE2(bp, NULL);	/* remove xact ptr */
	libxfs_writebuf(bp, 0);
#ifdef XACT_DEBUG
	fprintf(stderr, "flushing dirty inode %llu, buffer %p\n",
			ip->i_ino, bp);
#endif
}

static void
buf_item_done(
	xfs_buf_log_item_t	*bip)
{
	xfs_buf_t		*bp;
	int			hold;
	extern kmem_zone_t	*xfs_buf_item_zone;

	bp = bip->bli_buf;
	ASSERT(bp != NULL);
	XFS_BUF_SET_FSPRIVATE(bp, NULL);	/* remove log item */
	XFS_BUF_SET_FSPRIVATE2(bp, NULL);	/* remove xact ptr */

	hold = (bip->bli_flags & XFS_BLI_HOLD);
	if (bip->bli_flags & XFS_BLI_DIRTY) {
#ifdef XACT_DEBUG
		fprintf(stderr, "flushing/staling buffer %p (hold=%d)\n",
			bp, hold);
#endif
		libxfs_writebuf_int(bp, 0);
	}
	if (hold)
		bip->bli_flags &= ~XFS_BLI_HOLD;
	else
		libxfs_putbuf(bp);
	/* release the buf item */
	kmem_zone_free(xfs_buf_item_zone, bip);
}

static void
trans_committed(
	xfs_trans_t		*tp)
{
        struct xfs_log_item_desc *lidp, *next;

        list_for_each_entry_safe(lidp, next, &tp->t_items, lid_trans) {
		struct xfs_log_item *lip = lidp->lid_item;

		xfs_trans_del_item(lip);

		if (lip->li_type == XFS_LI_BUF)
			buf_item_done((xfs_buf_log_item_t *)lip);
		else if (lip->li_type == XFS_LI_INODE)
			inode_item_done((xfs_inode_log_item_t *)lip);
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
	xfs_buf_t		*bp = bip->bli_buf;
	uint			hold;

	/* Clear the buffer's association with this transaction. */
	XFS_BUF_SET_FSPRIVATE2(bip->bli_buf, NULL);

	hold = bip->bli_flags & XFS_BLI_HOLD;
	bip->bli_flags &= ~XFS_BLI_HOLD;
	if (!hold)
		libxfs_putbuf(bp);
}

static void
inode_item_unlock(
	xfs_inode_log_item_t	*iip)
{
	xfs_inode_t		*ip = iip->ili_inode;

	/* Clear the transaction pointer in the inode. */
	ip->i_transp = NULL;

	iip->ili_flags = 0;
}

/*
 * Unlock all of the items of a transaction and free all the descriptors
 * of that transaction.
 */
static void
xfs_trans_free_items(
	struct xfs_trans	*tp)
{
	struct xfs_log_item_desc *lidp, *next;

	list_for_each_entry_safe(lidp, next, &tp->t_items, lid_trans) {
		struct xfs_log_item	*lip = lidp->lid_item;

                xfs_trans_del_item(lip);
		if (lip->li_type == XFS_LI_BUF)
			buf_item_unlock((xfs_buf_log_item_t *)lip);
		else if (lip->li_type == XFS_LI_INODE)
			inode_item_unlock((xfs_inode_log_item_t *)lip);
		else {
			fprintf(stderr, _("%s: unrecognised log item type\n"),
				progname);
			ASSERT(0);
		}
	}
}

/*
 * Commit the changes represented by this transaction
 */
int
libxfs_trans_commit(
	xfs_trans_t	*tp)
{
	xfs_sb_t	*sbp;

	if (tp == NULL)
		return 0;

	if (!(tp->t_flags & XFS_TRANS_DIRTY)) {
#ifdef XACT_DEBUG
		fprintf(stderr, "committed clean transaction %p\n", tp);
#endif
		xfs_trans_free_items(tp);
		free(tp);
		tp = NULL;
		return 0;
	}

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

#ifdef XACT_DEBUG
	fprintf(stderr, "committing dirty transaction %p\n", tp);
#endif
	trans_committed(tp);

	/* That's it for the transaction structure.  Free it. */
	free(tp);
	tp = NULL;
	return 0;
}
