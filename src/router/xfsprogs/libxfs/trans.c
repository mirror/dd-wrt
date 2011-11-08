/*
 * Copyright (c) 2000-2001,2005-2006 Silicon Graphics, Inc.
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

#include <xfs.h>

/*
 * Simple transaction interface
 */

xfs_trans_t *
libxfs_trans_alloc(
	xfs_mount_t	*mp,
	int		type)
{
	xfs_trans_t	*ptr;

	if ((ptr = calloc(sizeof(xfs_trans_t), 1)) == NULL) {
		fprintf(stderr, _("%s: xact calloc failed (%d bytes): %s\n"),
			progname, (int)sizeof(xfs_trans_t), strerror(errno));
		exit(1);
	}
	ptr->t_mountp = mp;
	ptr->t_type = type;
	INIT_LIST_HEAD(&ptr->t_items);
#ifdef XACT_DEBUG
	fprintf(stderr, "allocated new transaction %p\n", ptr);
#endif
	return ptr;
}

xfs_trans_t *
libxfs_trans_dup(
	xfs_trans_t	*tp)
{
	xfs_trans_t	*ptr;

	ptr = libxfs_trans_alloc(tp->t_mountp, tp->t_type);
#ifdef XACT_DEBUG
	fprintf(stderr, "duplicated transaction %p (new=%p)\n", tp, ptr);
#endif
	return ptr;
}

int
libxfs_trans_reserve(
	xfs_trans_t	*tp,
	uint		blocks,
	uint		logspace,
	uint		rtextents,
	uint		flags,
	uint		logcount)
{
	xfs_sb_t	*mpsb = &tp->t_mountp->m_sb;

	/*
	 * Attempt to reserve the needed disk blocks by decrementing
	 * the number needed from the number available.	 This will
	 * fail if the count would go below zero.
	 */
	if (blocks > 0) {
		if (mpsb->sb_fdblocks < blocks)
			return ENOSPC;
	}
	/* user space, don't need log/RT stuff (preserve the API though) */
	return 0;
}

void
libxfs_trans_cancel(
	xfs_trans_t	*tp,
	int		flags)
{
#ifdef XACT_DEBUG
	xfs_trans_t	*otp = tp;
#endif
	if (tp != NULL) {
		xfs_trans_free_items(tp, flags);
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
		return libxfs_iget(mp, tp, ino, lock_flags, ipp, 0);

	error = libxfs_iget(mp, tp, ino, lock_flags, &ip, 0);
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
libxfs_trans_iput(
	xfs_trans_t		*tp,
	xfs_inode_t		*ip,
	uint			lock_flags)
{
	xfs_inode_log_item_t	*iip;

	if (tp == NULL) {
		libxfs_iput(ip, lock_flags);
		return;
	}

	ASSERT(ip->i_transp == tp);
	iip = ip->i_itemp;
	ASSERT(iip != NULL);
	xfs_trans_del_item(&iip->ili_item);

	libxfs_iput(ip, lock_flags);
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
	ip->i_itemp->ili_lock_flags = lock_flags;

#ifdef XACT_DEBUG
	fprintf(stderr, "ijoin_ref'd inode %llu, transaction %p\n", ip->i_ino, tp);
#endif
}

void
libxfs_trans_ihold(
	xfs_trans_t		*tp,
	xfs_inode_t		*ip)
{
	ASSERT(ip->i_transp == tp);
	ASSERT(ip->i_itemp != NULL);

	ip->i_itemp->ili_lock_flags = 1;

#ifdef XACT_DEBUG
	fprintf(stderr, "ihold'd inode %llu, transaction %p\n", ip->i_ino, tp);
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
}

/*
 * This is called to mark the fields indicated in fieldmask as needing
 * to be logged when the transaction is committed.  The inode must
 * already be associated with the given transaction.
 *
 * The values for fieldmask are defined in xfs_inode_item.h.  We always
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
	ip->i_itemp->ili_format.ilf_fields |= flags;
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
	XFS_BUF_STALE(bp);
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
libxfs_trans_get_buf(
	xfs_trans_t		*tp,
	dev_t			dev,
	xfs_daddr_t		d,
	int			len,
	uint			f)
{
	xfs_buf_t		*bp;
	xfs_buf_log_item_t	*bip;
	xfs_buftarg_t		bdev;

	if (tp == NULL)
		return libxfs_getbuf(dev, d, len);

	bdev.dev = dev;
	bp = xfs_trans_buf_item_match(tp, &bdev, d, len);
	if (bp != NULL) {
		ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
		bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t *);
		ASSERT(bip != NULL);
		bip->bli_recur++;
		return bp;
	}

	bp = libxfs_getbuf(dev, d, len);
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
	xfs_buftarg_t		bdev;
	int			len;

	if (tp == NULL)
		return libxfs_getsb(mp, flags);

	bdev.dev = mp->m_dev;
	len = XFS_FSS_TO_BB(mp, 1);
	bp = xfs_trans_buf_item_match(tp, &bdev, XFS_SB_DADDR, len);
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
libxfs_trans_read_buf(
	xfs_mount_t		*mp,
	xfs_trans_t		*tp,
	dev_t			dev,
	xfs_daddr_t		blkno,
	int			len,
	uint			flags,
	xfs_buf_t		**bpp)
{
	xfs_buf_t		*bp;
	xfs_buf_log_item_t	*bip;
	xfs_buftarg_t		bdev;
	int			error;

	*bpp = NULL;

	if (tp == NULL) {
		bp = libxfs_readbuf(dev, blkno, len, flags);
		if (!bp) {
			return (flags & XBF_TRYLOCK) ?
				EAGAIN : XFS_ERROR(ENOMEM);
		}
		if (bp->b_error)
			goto out_relse;
		goto done;
	}

	bdev.dev = dev;
	bp = xfs_trans_buf_item_match(tp, &bdev, blkno, len);
	if (bp != NULL) {
		ASSERT(XFS_BUF_FSPRIVATE2(bp, xfs_trans_t *) == tp);
		ASSERT(XFS_BUF_FSPRIVATE(bp, void *) != NULL);
		bip = XFS_BUF_FSPRIVATE(bp, xfs_buf_log_item_t*);
		bip->bli_recur++;
		goto done;
	}

	bp = libxfs_readbuf(dev, blkno, len, flags);
	if (!bp) {
		return (flags & XBF_TRYLOCK) ?
			EAGAIN : XFS_ERROR(ENOMEM);
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
	extern kmem_zone_t	*xfs_ili_zone;

	ip = iip->ili_inode;
	mp = iip->ili_item.li_mountp;
	ASSERT(ip != NULL);

	if (!(iip->ili_format.ilf_fields & XFS_ILOG_ALL)) {
		ip->i_transp = NULL;	/* disassociate from transaction */
		iip->ili_flags = 0;	/* reset all flags */
		goto ili_done;
	}

	/*
	 * Get the buffer containing the on-disk inode.
	 */
	error = xfs_itobp(mp, NULL, ip, &dip, &bp, 0);
	if (error) {
		fprintf(stderr, _("%s: warning - itobp failed (%d)\n"),
			progname, error);
		goto ili_done;
	}

	XFS_BUF_SET_FSPRIVATE(bp, iip);
	error = libxfs_iflush_int(ip, bp);
	if (error) {
		fprintf(stderr, _("%s: warning - iflush_int failed (%d)\n"),
			progname, error);
		goto ili_done;
	}

	ip->i_transp = NULL;	/* disassociate from transaction */
	XFS_BUF_SET_FSPRIVATE(bp, NULL);	/* remove log item */
	XFS_BUF_SET_FSPRIVATE2(bp, NULL);	/* remove xact ptr */
	libxfs_writebuf(bp, 0);
#ifdef XACT_DEBUG
	fprintf(stderr, "flushing dirty inode %llu, buffer %p (hold=%u)\n",
			ip->i_ino, bp, iip->ili_lock_flags);
#endif
ili_done:
	if (iip->ili_lock_flags) {
		iip->ili_lock_flags = 0;
		return;
	} else {
		libxfs_iput(ip, 0);
	}

	if (ip->i_itemp)
		kmem_zone_free(xfs_ili_zone, ip->i_itemp);
	else
		ASSERT(0);
	ip->i_itemp = NULL;
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
	if (!iip->ili_lock_flags)
		libxfs_iput(ip, 0);
	else
		iip->ili_lock_flags = 0;
}

/*
 * Unlock all of the items of a transaction and free all the descriptors
 * of that transaction.
 */
void
xfs_trans_free_items(
	struct xfs_trans	*tp,
	int			flags)
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
	xfs_trans_t	*tp,
	uint		flags)
{
	xfs_sb_t	*sbp;

	if (tp == NULL)
		return 0;

	if (!(tp->t_flags & XFS_TRANS_DIRTY)) {
#ifdef XACT_DEBUG
		fprintf(stderr, "committed clean transaction %p\n", tp);
#endif
		xfs_trans_free_items(tp, flags);
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
		xfs_mod_sb(tp, XFS_SB_ALL_BITS);
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
