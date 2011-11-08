/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

kmem_zone_t	*xfs_buf_item_zone;
kmem_zone_t	*xfs_ili_zone;		/* inode log item zone */

/*
 * Following functions from fs/xfs/xfs_trans_buf.c
 */

/*
 * Check to see if a buffer matching the given parameters is already
 * a part of the given transaction.
 */
xfs_buf_t *
xfs_trans_buf_item_match(
	xfs_trans_t		*tp,
	xfs_buftarg_t		*target,
	xfs_daddr_t		blkno,
	int			len)
{
        struct xfs_log_item_desc *lidp;
        struct xfs_buf_log_item *blip;

        len = BBTOB(len);
        list_for_each_entry(lidp, &tp->t_items, lid_trans) {
                blip = (struct xfs_buf_log_item *)lidp->lid_item;
                if (blip->bli_item.li_type == XFS_LI_BUF &&
                    XFS_BUF_TARGET(blip->bli_buf) == target->dev &&
                    XFS_BUF_ADDR(blip->bli_buf) == blkno &&
                    XFS_BUF_COUNT(blip->bli_buf) == len)
                        return blip->bli_buf;
        }

        return NULL;
}
/*
 * The following are from fs/xfs/xfs_buf_item.c
 */

/*
 * Allocate a new buf log item to go with the given buffer.
 * Set the buffer's b_fsprivate field to point to the new
 * buf log item.  If there are other item's attached to the
 * buffer (see xfs_buf_attach_iodone() below), then put the
 * buf log item at the front.
 */
void
xfs_buf_item_init(
	xfs_buf_t		*bp,
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
	if (XFS_BUF_FSPRIVATE3(bp, xfs_mount_t *) != mp)
		XFS_BUF_SET_FSPRIVATE3(bp, mp);
	XFS_BUF_SET_BDSTRAT_FUNC(bp, xfs_bdstrat_cb);
	if (XFS_BUF_FSPRIVATE(bp, void *) != NULL) {
		lip = XFS_BUF_FSPRIVATE(bp, xfs_log_item_t *);
		if (lip->li_type == XFS_LI_BUF) {
#ifdef LI_DEBUG
			fprintf(stderr,
				"reused buf item %p for pre-logged buffer %p\n",
				lip, bp);
#endif
			return;
		}
	}

	bip = (xfs_buf_log_item_t *)kmem_zone_zalloc(xfs_buf_item_zone,
						    KM_SLEEP);
#ifdef LI_DEBUG
	fprintf(stderr, "adding buf item %p for not-logged buffer %p\n",
		bip, bp);
#endif
	bip->bli_item.li_type = XFS_LI_BUF;
	bip->bli_item.li_mountp = mp;
	bip->bli_buf = bp;
	bip->bli_format.blf_type = XFS_LI_BUF;
	bip->bli_format.blf_blkno = (__int64_t)XFS_BUF_ADDR(bp);
	bip->bli_format.blf_len = (ushort)BTOBB(XFS_BUF_COUNT(bp));
	XFS_BUF_SET_FSPRIVATE(bp, bip);
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

/*
 * Initialize the inode log item for a newly allocated (in-core) inode.
 */
void
xfs_inode_item_init(
	xfs_inode_t		*ip,
	xfs_mount_t		*mp)
{
	xfs_inode_log_item_t	*iip;

	ASSERT(ip->i_itemp == NULL);
	iip = ip->i_itemp = (xfs_inode_log_item_t *)
			kmem_zone_zalloc(xfs_ili_zone, KM_SLEEP);
#ifdef LI_DEBUG
	fprintf(stderr, "inode_item_init for inode %llu, iip=%p\n",
		ip->i_ino, iip);
#endif

	iip->ili_item.li_type = XFS_LI_INODE;
	iip->ili_item.li_mountp = mp;
	iip->ili_inode = ip;
	iip->ili_format.ilf_type = XFS_LI_INODE;
	iip->ili_format.ilf_ino = ip->i_ino;
	iip->ili_format.ilf_blkno = ip->i_imap.im_blkno;
	iip->ili_format.ilf_len = ip->i_imap.im_len;
	iip->ili_format.ilf_boffset = ip->i_imap.im_boffset;
}
