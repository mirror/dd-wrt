// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

struct kmem_cache	*xfs_buf_item_cache;
struct kmem_cache	*xfs_ili_cache;		/* inode log item cache */

/*
 * Following functions from fs/xfs/xfs_trans_buf.c
 */

/*
 * Check to see if a buffer matching the given parameters is already
 * a part of the given transaction.
 */
struct xfs_buf *
xfs_trans_buf_item_match(
	xfs_trans_t		*tp,
	struct xfs_buftarg	*btp,
	struct xfs_buf_map	*map,
	int			nmaps)
{
	struct xfs_log_item	*lip;
	struct xfs_buf_log_item *blip;
	int			len = 0;
	int			i;

	for (i = 0; i < nmaps; i++)
		len += map[i].bm_len;

	list_for_each_entry(lip, &tp->t_items, li_trans) {
		blip = (struct xfs_buf_log_item *)lip;
		if (blip->bli_item.li_type == XFS_LI_BUF &&
		    blip->bli_buf->b_target->bt_bdev == btp->bt_bdev &&
		    xfs_buf_daddr(blip->bli_buf) == map[0].bm_bn &&
		    blip->bli_buf->b_length == len) {
			ASSERT(blip->bli_buf->b_map_count == nmaps);
			return blip->bli_buf;
		}
	}

	return NULL;
}
/*
 * The following are from fs/xfs/xfs_buf_item.c
 */

static const struct xfs_item_ops xfs_buf_item_ops = {
};

/*
 * Allocate a new buf log item to go with the given buffer.
 * Set the buffer's b_log_item field to point to the new
 * buf log item.  If there are other item's attached to the
 * buffer (see xfs_buf_attach_iodone() below), then put the
 * buf log item at the front.
 */
void
xfs_buf_item_init(
	struct xfs_buf		*bp,
	xfs_mount_t		*mp)
{
	xfs_log_item_t		*lip;
	xfs_buf_log_item_t	*bip;

#ifdef LI_DEBUG
	fprintf(stderr, "buf_item_init for buffer %p\n", bp);
#endif

	/*
	 * Check to see if there is already a buf log item for
	 * this buffer.	 If there is, it is guaranteed to be
	 * the first.  If we do already have one, there is
	 * nothing to do here so return.
	 */
	if (bp->b_log_item != NULL) {
		lip = bp->b_log_item;
		if (lip->li_type == XFS_LI_BUF) {
#ifdef LI_DEBUG
			fprintf(stderr,
				"reused buf item %p for pre-logged buffer %p\n",
				lip, bp);
#endif
			return;
		}
	}

	bip = kmem_cache_zalloc(xfs_buf_item_cache, 0);
#ifdef LI_DEBUG
	fprintf(stderr, "adding buf item %p for not-logged buffer %p\n",
		bip, bp);
#endif
	xfs_log_item_init(mp, &bip->bli_item, XFS_LI_BUF, &xfs_buf_item_ops);
	bip->bli_buf = bp;
	bip->__bli_format.blf_type = XFS_LI_BUF;
	bip->__bli_format.blf_blkno = (int64_t)xfs_buf_daddr(bp);
	bip->__bli_format.blf_len = (unsigned short)bp->b_length;
	bp->b_log_item = bip;
}


/*
 * Mark bytes first through last inclusive as dirty in the buf
 * item's bitmap.
 */
void
xfs_buf_item_log(
	xfs_buf_log_item_t	*bip,
	uint			first,
	uint			last)
{
	/*
	 * Mark the item as having some dirty data for
	 * quick reference in xfs_buf_item_dirty.
	 */
	bip->bli_flags |= XFS_BLI_DIRTY;
}

static inline struct xfs_inode_log_item *INODE_ITEM(struct xfs_log_item *lip)
{
	return container_of(lip, struct xfs_inode_log_item, ili_item);
}

static uint64_t
xfs_inode_item_sort(
	struct xfs_log_item	*lip)
{
	return INODE_ITEM(lip)->ili_inode->i_ino;
}

/*
 * Prior to finally logging the inode, we have to ensure that all the
 * per-modification inode state changes are applied. This includes VFS inode
 * state updates, format conversions, verifier state synchronisation and
 * ensuring the inode buffer remains in memory whilst the inode is dirty.
 *
 * We have to be careful when we grab the inode cluster buffer due to lock
 * ordering constraints. The unlinked inode modifications (xfs_iunlink_item)
 * require AGI -> inode cluster buffer lock order. The inode cluster buffer is
 * not locked until ->precommit, so it happens after everything else has been
 * modified.
 *
 * Further, we have AGI -> AGF lock ordering, and with O_TMPFILE handling we
 * have AGI -> AGF -> iunlink item -> inode cluster buffer lock order. Hence we
 * cannot safely lock the inode cluster buffer in xfs_trans_log_inode() because
 * it can be called on a inode (e.g. via bumplink/droplink) before we take the
 * AGF lock modifying directory blocks.
 *
 * Rather than force a complete rework of all the transactions to call
 * xfs_trans_log_inode() once and once only at the end of every transaction, we
 * move the pinning of the inode cluster buffer to a ->precommit operation. This
 * matches how the xfs_iunlink_item locks the inode cluster buffer, and it
 * ensures that the inode cluster buffer locking is always done last in a
 * transaction. i.e. we ensure the lock order is always AGI -> AGF -> inode
 * cluster buffer.
 *
 * If we return the inode number as the precommit sort key then we'll also
 * guarantee that the order all inode cluster buffer locking is the same all the
 * inodes and unlink items in the transaction.
 */
static int
xfs_inode_item_precommit(
	struct xfs_trans	*tp,
	struct xfs_log_item	*lip)
{
	struct xfs_inode_log_item *iip = INODE_ITEM(lip);
	struct xfs_inode	*ip = iip->ili_inode;
	struct inode		*inode = VFS_I(ip);
	unsigned int		flags = iip->ili_dirty_flags;

	/*
	 * Don't bother with i_lock for the I_DIRTY_TIME check here, as races
	 * don't matter - we either will need an extra transaction in 24 hours
	 * to log the timestamps, or will clear already cleared fields in the
	 * worst case.
	 */
	if (inode->i_state & I_DIRTY_TIME) {
		spin_lock(&inode->i_lock);
		inode->i_state &= ~I_DIRTY_TIME;
		spin_unlock(&inode->i_lock);
	}

	/*
	 * If we're updating the inode core or the timestamps and it's possible
	 * to upgrade this inode to bigtime format, do so now.
	 */
	if ((flags & (XFS_ILOG_CORE | XFS_ILOG_TIMESTAMP)) &&
	    xfs_has_bigtime(ip->i_mount) &&
	    !xfs_inode_has_bigtime(ip)) {
		ip->i_diflags2 |= XFS_DIFLAG2_BIGTIME;
		flags |= XFS_ILOG_CORE;
	}

	/*
	 * Inode verifiers do not check that the extent size hint is an integer
	 * multiple of the rt extent size on a directory with both rtinherit
	 * and extszinherit flags set.  If we're logging a directory that is
	 * misconfigured in this way, clear the hint.
	 */
	if ((ip->i_diflags & XFS_DIFLAG_RTINHERIT) &&
	    (ip->i_diflags & XFS_DIFLAG_EXTSZINHERIT) &&
	    (ip->i_extsize % ip->i_mount->m_sb.sb_rextsize) > 0) {
		ip->i_diflags &= ~(XFS_DIFLAG_EXTSIZE |
				   XFS_DIFLAG_EXTSZINHERIT);
		ip->i_extsize = 0;
		flags |= XFS_ILOG_CORE;
	}

	/*
	 * Record the specific change for fdatasync optimisation. This allows
	 * fdatasync to skip log forces for inodes that are only timestamp
	 * dirty. Once we've processed the XFS_ILOG_IVERSION flag, convert it
	 * to XFS_ILOG_CORE so that the actual on-disk dirty tracking
	 * (ili_fields) correctly tracks that the version has changed.
	 */
	spin_lock(&iip->ili_lock);
	iip->ili_fsync_fields |= (flags & ~XFS_ILOG_IVERSION);
	if (flags & XFS_ILOG_IVERSION)
		flags = ((flags & ~XFS_ILOG_IVERSION) | XFS_ILOG_CORE);

	if (!iip->ili_item.li_buf) {
		struct xfs_buf	*bp;
		int		error;

		/*
		 * We hold the ILOCK here, so this inode is not going to be
		 * flushed while we are here. Further, because there is no
		 * buffer attached to the item, we know that there is no IO in
		 * progress, so nothing will clear the ili_fields while we read
		 * in the buffer. Hence we can safely drop the spin lock and
		 * read the buffer knowing that the state will not change from
		 * here.
		 */
		spin_unlock(&iip->ili_lock);
		error = xfs_imap_to_bp(ip->i_mount, tp, &ip->i_imap, &bp);
		if (error)
			return error;

		/*
		 * We need an explicit buffer reference for the log item but
		 * don't want the buffer to remain attached to the transaction.
		 * Hold the buffer but release the transaction reference once
		 * we've attached the inode log item to the buffer log item
		 * list.
		 */
		xfs_buf_hold(bp);
		spin_lock(&iip->ili_lock);
		iip->ili_item.li_buf = bp;
		bp->b_flags |= _XBF_INODES;
		list_add_tail(&iip->ili_item.li_bio_list, &bp->b_li_list);
		xfs_trans_brelse(tp, bp);
	}

	/*
	 * Always OR in the bits from the ili_last_fields field.  This is to
	 * coordinate with the xfs_iflush() and xfs_buf_inode_iodone() routines
	 * in the eventual clearing of the ili_fields bits.  See the big comment
	 * in xfs_iflush() for an explanation of this coordination mechanism.
	 */
	iip->ili_fields |= (flags | iip->ili_last_fields);
	spin_unlock(&iip->ili_lock);

	/*
	 * We are done with the log item transaction dirty state, so clear it so
	 * that it doesn't pollute future transactions.
	 */
	iip->ili_dirty_flags = 0;
	return 0;
}

static const struct xfs_item_ops xfs_inode_item_ops = {
	.iop_sort	= xfs_inode_item_sort,
	.iop_precommit	= xfs_inode_item_precommit,
};

/*
 * Initialize the inode log item for a newly allocated (in-core) inode.
 */
void
xfs_inode_item_init(
	xfs_inode_t			*ip,
	xfs_mount_t			*mp)
{
	struct xfs_inode_log_item	*iip;

	ASSERT(ip->i_itemp == NULL);
	iip = ip->i_itemp = kmem_cache_zalloc(xfs_ili_cache, 0);
#ifdef LI_DEBUG
	fprintf(stderr, "inode_item_init for inode %llu, iip=%p\n",
		ip->i_ino, iip);
#endif

	spin_lock_init(&iip->ili_lock);

        xfs_log_item_init(mp, &iip->ili_item, XFS_LI_INODE,
						&xfs_inode_item_ops);
	iip->ili_inode = ip;
}
