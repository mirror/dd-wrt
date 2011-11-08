/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
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

#include <xfs.h>

kmem_zone_t	*xfs_trans_zone;
kmem_zone_t	*xfs_log_item_desc_zone;

/*
 * Various log reservation values.
 *
 * These are based on the size of the file system block because that is what
 * most transactions manipulate.  Each adds in an additional 128 bytes per
 * item logged to try to account for the overhead of the transaction mechanism.
 *
 * Note:  Most of the reservations underestimate the number of allocation
 * groups into which they could free extents in the xfs_bmap_finish() call.
 * This is because the number in the worst case is quite high and quite
 * unusual.  In order to fix this we need to change xfs_bmap_finish() to free
 * extents in only a single AG at a time.  This will require changes to the
 * EFI code as well, however, so that the EFI for the extents not freed is
 * logged again in each transaction.  See SGI PV #261917.
 *
 * Reservation functions here avoid a huge stack in xfs_trans_init due to
 * register overflow from temporaries in the calculations.
 */


/*
 * In a write transaction we can allocate a maximum of 2
 * extents.  This gives:
 *    the inode getting the new extents: inode size
 *    the inode's bmap btree: max depth * block size
 *    the agfs of the ags from which the extents are allocated: 2 * sector
 *    the superblock free block counter: sector size
 *    the allocation btrees: 2 exts * 2 trees * (2 * max depth - 1) * block size
 * And the bmap_finish transaction can free bmap blocks in a join:
 *    the agfs of the ags containing the blocks: 2 * sector size
 *    the agfls of the ags containing the blocks: 2 * sector size
 *    the super block free block counter: sector size
 *    the allocation btrees: 2 exts * 2 trees * (2 * max depth - 1) * block size
 */
STATIC uint
xfs_calc_write_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((mp->m_sb.sb_inodesize +
		     XFS_FSB_TO_B(mp, XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK)) +
		     2 * mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     XFS_ALLOCFREE_LOG_RES(mp, 2) +
		     128 * (4 + XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK) +
			    XFS_ALLOCFREE_LOG_COUNT(mp, 2))),
		    (2 * mp->m_sb.sb_sectsize +
		     2 * mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     XFS_ALLOCFREE_LOG_RES(mp, 2) +
		     128 * (5 + XFS_ALLOCFREE_LOG_COUNT(mp, 2))));
}

/*
 * In truncating a file we free up to two extents at once.  We can modify:
 *    the inode being truncated: inode size
 *    the inode's bmap btree: (max depth + 1) * block size
 * And the bmap_finish transaction can free the blocks and bmap blocks:
 *    the agf for each of the ags: 4 * sector size
 *    the agfl for each of the ags: 4 * sector size
 *    the super block to reflect the freed blocks: sector size
 *    worst case split in allocation btrees per extent assuming 4 extents:
 *		4 exts * 2 trees * (2 * max depth - 1) * block size
 *    the inode btree: max depth * blocksize
 *    the allocation btrees: 2 trees * (max depth - 1) * block size
 */
STATIC uint
xfs_calc_itruncate_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((mp->m_sb.sb_inodesize +
		     XFS_FSB_TO_B(mp, XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK) + 1) +
		     128 * (2 + XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK))),
		    (4 * mp->m_sb.sb_sectsize +
		     4 * mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     XFS_ALLOCFREE_LOG_RES(mp, 4) +
		     128 * (9 + XFS_ALLOCFREE_LOG_COUNT(mp, 4)) +
		     128 * 5 +
		     XFS_ALLOCFREE_LOG_RES(mp, 1) +
		     128 * (2 + XFS_IALLOC_BLOCKS(mp) + mp->m_in_maxlevels +
			    XFS_ALLOCFREE_LOG_COUNT(mp, 1))));
}

/*
 * In renaming a files we can modify:
 *    the four inodes involved: 4 * inode size
 *    the two directory btrees: 2 * (max depth + v2) * dir block size
 *    the two directory bmap btrees: 2 * max depth * block size
 * And the bmap_finish transaction can free dir and bmap blocks (two sets
 *	of bmap blocks) giving:
 *    the agf for the ags in which the blocks live: 3 * sector size
 *    the agfl for the ags in which the blocks live: 3 * sector size
 *    the superblock for the free block count: sector size
 *    the allocation btrees: 3 exts * 2 trees * (2 * max depth - 1) * block size
 */
STATIC uint
xfs_calc_rename_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((4 * mp->m_sb.sb_inodesize +
		     2 * XFS_DIROP_LOG_RES(mp) +
		     128 * (4 + 2 * XFS_DIROP_LOG_COUNT(mp))),
		    (3 * mp->m_sb.sb_sectsize +
		     3 * mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     XFS_ALLOCFREE_LOG_RES(mp, 3) +
		     128 * (7 + XFS_ALLOCFREE_LOG_COUNT(mp, 3))));
}

/*
 * For creating a link to an inode:
 *    the parent directory inode: inode size
 *    the linked inode: inode size
 *    the directory btree could split: (max depth + v2) * dir block size
 *    the directory bmap btree could join or split: (max depth + v2) * blocksize
 * And the bmap_finish transaction can free some bmap blocks giving:
 *    the agf for the ag in which the blocks live: sector size
 *    the agfl for the ag in which the blocks live: sector size
 *    the superblock for the free block count: sector size
 *    the allocation btrees: 2 trees * (2 * max depth - 1) * block size
 */
STATIC uint
xfs_calc_link_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((mp->m_sb.sb_inodesize +
		     mp->m_sb.sb_inodesize +
		     XFS_DIROP_LOG_RES(mp) +
		     128 * (2 + XFS_DIROP_LOG_COUNT(mp))),
		    (mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     XFS_ALLOCFREE_LOG_RES(mp, 1) +
		     128 * (3 + XFS_ALLOCFREE_LOG_COUNT(mp, 1))));
}

/*
 * For removing a directory entry we can modify:
 *    the parent directory inode: inode size
 *    the removed inode: inode size
 *    the directory btree could join: (max depth + v2) * dir block size
 *    the directory bmap btree could join or split: (max depth + v2) * blocksize
 * And the bmap_finish transaction can free the dir and bmap blocks giving:
 *    the agf for the ag in which the blocks live: 2 * sector size
 *    the agfl for the ag in which the blocks live: 2 * sector size
 *    the superblock for the free block count: sector size
 *    the allocation btrees: 2 exts * 2 trees * (2 * max depth - 1) * block size
 */
STATIC uint
xfs_calc_remove_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((mp->m_sb.sb_inodesize +
		     mp->m_sb.sb_inodesize +
		     XFS_DIROP_LOG_RES(mp) +
		     128 * (2 + XFS_DIROP_LOG_COUNT(mp))),
		    (2 * mp->m_sb.sb_sectsize +
		     2 * mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     XFS_ALLOCFREE_LOG_RES(mp, 2) +
		     128 * (5 + XFS_ALLOCFREE_LOG_COUNT(mp, 2))));
}

/*
 * For symlink we can modify:
 *    the parent directory inode: inode size
 *    the new inode: inode size
 *    the inode btree entry: 1 block
 *    the directory btree: (max depth + v2) * dir block size
 *    the directory inode's bmap btree: (max depth + v2) * block size
 *    the blocks for the symlink: 1 kB
 * Or in the first xact we allocate some inodes giving:
 *    the agi and agf of the ag getting the new inodes: 2 * sectorsize
 *    the inode blocks allocated: XFS_IALLOC_BLOCKS * blocksize
 *    the inode btree: max depth * blocksize
 *    the allocation btrees: 2 trees * (2 * max depth - 1) * block size
 */
STATIC uint
xfs_calc_symlink_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((mp->m_sb.sb_inodesize +
		     mp->m_sb.sb_inodesize +
		     XFS_FSB_TO_B(mp, 1) +
		     XFS_DIROP_LOG_RES(mp) +
		     1024 +
		     128 * (4 + XFS_DIROP_LOG_COUNT(mp))),
		    (2 * mp->m_sb.sb_sectsize +
		     XFS_FSB_TO_B(mp, XFS_IALLOC_BLOCKS(mp)) +
		     XFS_FSB_TO_B(mp, mp->m_in_maxlevels) +
		     XFS_ALLOCFREE_LOG_RES(mp, 1) +
		     128 * (2 + XFS_IALLOC_BLOCKS(mp) + mp->m_in_maxlevels +
			    XFS_ALLOCFREE_LOG_COUNT(mp, 1))));
}

/*
 * For create we can modify:
 *    the parent directory inode: inode size
 *    the new inode: inode size
 *    the inode btree entry: block size
 *    the superblock for the nlink flag: sector size
 *    the directory btree: (max depth + v2) * dir block size
 *    the directory inode's bmap btree: (max depth + v2) * block size
 * Or in the first xact we allocate some inodes giving:
 *    the agi and agf of the ag getting the new inodes: 2 * sectorsize
 *    the superblock for the nlink flag: sector size
 *    the inode blocks allocated: XFS_IALLOC_BLOCKS * blocksize
 *    the inode btree: max depth * blocksize
 *    the allocation btrees: 2 trees * (max depth - 1) * block size
 */
STATIC uint
xfs_calc_create_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((mp->m_sb.sb_inodesize +
		     mp->m_sb.sb_inodesize +
		     mp->m_sb.sb_sectsize +
		     XFS_FSB_TO_B(mp, 1) +
		     XFS_DIROP_LOG_RES(mp) +
		     128 * (3 + XFS_DIROP_LOG_COUNT(mp))),
		    (3 * mp->m_sb.sb_sectsize +
		     XFS_FSB_TO_B(mp, XFS_IALLOC_BLOCKS(mp)) +
		     XFS_FSB_TO_B(mp, mp->m_in_maxlevels) +
		     XFS_ALLOCFREE_LOG_RES(mp, 1) +
		     128 * (2 + XFS_IALLOC_BLOCKS(mp) + mp->m_in_maxlevels +
			    XFS_ALLOCFREE_LOG_COUNT(mp, 1))));
}

/*
 * Making a new directory is the same as creating a new file.
 */
STATIC uint
xfs_calc_mkdir_reservation(
	struct xfs_mount	*mp)
{
	return xfs_calc_create_reservation(mp);
}

/*
 * In freeing an inode we can modify:
 *    the inode being freed: inode size
 *    the super block free inode counter: sector size
 *    the agi hash list and counters: sector size
 *    the inode btree entry: block size
 *    the on disk inode before ours in the agi hash list: inode cluster size
 *    the inode btree: max depth * blocksize
 *    the allocation btrees: 2 trees * (max depth - 1) * block size
 */
STATIC uint
xfs_calc_ifree_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		mp->m_sb.sb_inodesize +
		mp->m_sb.sb_sectsize +
		mp->m_sb.sb_sectsize +
		XFS_FSB_TO_B(mp, 1) +
		MAX((__uint16_t)XFS_FSB_TO_B(mp, 1),
		    XFS_INODE_CLUSTER_SIZE(mp)) +
		128 * 5 +
		XFS_ALLOCFREE_LOG_RES(mp, 1) +
		128 * (2 + XFS_IALLOC_BLOCKS(mp) + mp->m_in_maxlevels +
		       XFS_ALLOCFREE_LOG_COUNT(mp, 1));
}

/*
 * When only changing the inode we log the inode and possibly the superblock
 * We also add a bit of slop for the transaction stuff.
 */
STATIC uint
xfs_calc_ichange_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		mp->m_sb.sb_inodesize +
		mp->m_sb.sb_sectsize +
		512;

}

/*
 * Growing the data section of the filesystem.
 *	superblock
 *	agi and agf
 *	allocation btrees
 */
STATIC uint
xfs_calc_growdata_reservation(
	struct xfs_mount	*mp)
{
	return mp->m_sb.sb_sectsize * 3 +
		XFS_ALLOCFREE_LOG_RES(mp, 1) +
		128 * (3 + XFS_ALLOCFREE_LOG_COUNT(mp, 1));
}

/*
 * Growing the rt section of the filesystem.
 * In the first set of transactions (ALLOC) we allocate space to the
 * bitmap or summary files.
 *	superblock: sector size
 *	agf of the ag from which the extent is allocated: sector size
 *	bmap btree for bitmap/summary inode: max depth * blocksize
 *	bitmap/summary inode: inode size
 *	allocation btrees for 1 block alloc: 2 * (2 * maxdepth - 1) * blocksize
 */
STATIC uint
xfs_calc_growrtalloc_reservation(
	struct xfs_mount	*mp)
{
	return 2 * mp->m_sb.sb_sectsize +
		XFS_FSB_TO_B(mp, XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK)) +
		mp->m_sb.sb_inodesize +
		XFS_ALLOCFREE_LOG_RES(mp, 1) +
		128 * (3 + XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK) +
		       XFS_ALLOCFREE_LOG_COUNT(mp, 1));
}

/*
 * Growing the rt section of the filesystem.
 * In the second set of transactions (ZERO) we zero the new metadata blocks.
 *	one bitmap/summary block: blocksize
 */
STATIC uint
xfs_calc_growrtzero_reservation(
	struct xfs_mount	*mp)
{
	return mp->m_sb.sb_blocksize + 128;
}

/*
 * Growing the rt section of the filesystem.
 * In the third set of transactions (FREE) we update metadata without
 * allocating any new blocks.
 *	superblock: sector size
 *	bitmap inode: inode size
 *	summary inode: inode size
 *	one bitmap block: blocksize
 *	summary blocks: new summary size
 */
STATIC uint
xfs_calc_growrtfree_reservation(
	struct xfs_mount	*mp)
{
	return mp->m_sb.sb_sectsize +
		2 * mp->m_sb.sb_inodesize +
		mp->m_sb.sb_blocksize +
		mp->m_rsumsize +
		128 * 5;
}

/*
 * Logging the inode modification timestamp on a synchronous write.
 *	inode
 */
STATIC uint
xfs_calc_swrite_reservation(
	struct xfs_mount	*mp)
{
	return mp->m_sb.sb_inodesize + 128;
}

/*
 * Logging the inode mode bits when writing a setuid/setgid file
 *	inode
 */
STATIC uint
xfs_calc_writeid_reservation(xfs_mount_t *mp)
{
	return mp->m_sb.sb_inodesize + 128;
}

/*
 * Converting the inode from non-attributed to attributed.
 *	the inode being converted: inode size
 *	agf block and superblock (for block allocation)
 *	the new block (directory sized)
 *	bmap blocks for the new directory block
 *	allocation btrees
 */
STATIC uint
xfs_calc_addafork_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		mp->m_sb.sb_inodesize +
		mp->m_sb.sb_sectsize * 2 +
		mp->m_dirblksize +
		XFS_FSB_TO_B(mp, XFS_DAENTER_BMAP1B(mp, XFS_DATA_FORK) + 1) +
		XFS_ALLOCFREE_LOG_RES(mp, 1) +
		128 * (4 + XFS_DAENTER_BMAP1B(mp, XFS_DATA_FORK) + 1 +
		       XFS_ALLOCFREE_LOG_COUNT(mp, 1));
}

/*
 * Removing the attribute fork of a file
 *    the inode being truncated: inode size
 *    the inode's bmap btree: max depth * block size
 * And the bmap_finish transaction can free the blocks and bmap blocks:
 *    the agf for each of the ags: 4 * sector size
 *    the agfl for each of the ags: 4 * sector size
 *    the super block to reflect the freed blocks: sector size
 *    worst case split in allocation btrees per extent assuming 4 extents:
 *		4 exts * 2 trees * (2 * max depth - 1) * block size
 */
STATIC uint
xfs_calc_attrinval_reservation(
	struct xfs_mount	*mp)
{
	return MAX((mp->m_sb.sb_inodesize +
		    XFS_FSB_TO_B(mp, XFS_BM_MAXLEVELS(mp, XFS_ATTR_FORK)) +
		    128 * (1 + XFS_BM_MAXLEVELS(mp, XFS_ATTR_FORK))),
		   (4 * mp->m_sb.sb_sectsize +
		    4 * mp->m_sb.sb_sectsize +
		    mp->m_sb.sb_sectsize +
		    XFS_ALLOCFREE_LOG_RES(mp, 4) +
		    128 * (9 + XFS_ALLOCFREE_LOG_COUNT(mp, 4))));
}

/*
 * Setting an attribute.
 *	the inode getting the attribute
 *	the superblock for allocations
 *	the agfs extents are allocated from
 *	the attribute btree * max depth
 *	the inode allocation btree
 * Since attribute transaction space is dependent on the size of the attribute,
 * the calculation is done partially at mount time and partially at runtime.
 */
STATIC uint
xfs_calc_attrset_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		mp->m_sb.sb_inodesize +
		mp->m_sb.sb_sectsize +
		XFS_FSB_TO_B(mp, XFS_DA_NODE_MAXDEPTH) +
		128 * (2 + XFS_DA_NODE_MAXDEPTH);
}

/*
 * Removing an attribute.
 *    the inode: inode size
 *    the attribute btree could join: max depth * block size
 *    the inode bmap btree could join or split: max depth * block size
 * And the bmap_finish transaction can free the attr blocks freed giving:
 *    the agf for the ag in which the blocks live: 2 * sector size
 *    the agfl for the ag in which the blocks live: 2 * sector size
 *    the superblock for the free block count: sector size
 *    the allocation btrees: 2 exts * 2 trees * (2 * max depth - 1) * block size
 */
STATIC uint
xfs_calc_attrrm_reservation(
	struct xfs_mount	*mp)
{
	return XFS_DQUOT_LOGRES(mp) +
		MAX((mp->m_sb.sb_inodesize +
		     XFS_FSB_TO_B(mp, XFS_DA_NODE_MAXDEPTH) +
		     XFS_FSB_TO_B(mp, XFS_BM_MAXLEVELS(mp, XFS_ATTR_FORK)) +
		     128 * (1 + XFS_DA_NODE_MAXDEPTH +
			    XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK))),
		    (2 * mp->m_sb.sb_sectsize +
		     2 * mp->m_sb.sb_sectsize +
		     mp->m_sb.sb_sectsize +
		     XFS_ALLOCFREE_LOG_RES(mp, 2) +
		     128 * (5 + XFS_ALLOCFREE_LOG_COUNT(mp, 2))));
}

/*
 * Clearing a bad agino number in an agi hash bucket.
 */
STATIC uint
xfs_calc_clear_agi_bucket_reservation(
	struct xfs_mount	*mp)
{
	return mp->m_sb.sb_sectsize + 128;
}

/*
 * Initialize the precomputed transaction reservation values
 * in the mount structure.
 */
void
xfs_trans_init(
	struct xfs_mount	*mp)
{
	struct xfs_trans_reservations *resp = &mp->m_reservations;

	resp->tr_write = xfs_calc_write_reservation(mp);
	resp->tr_itruncate = xfs_calc_itruncate_reservation(mp);
	resp->tr_rename = xfs_calc_rename_reservation(mp);
	resp->tr_link = xfs_calc_link_reservation(mp);
	resp->tr_remove = xfs_calc_remove_reservation(mp);
	resp->tr_symlink = xfs_calc_symlink_reservation(mp);
	resp->tr_create = xfs_calc_create_reservation(mp);
	resp->tr_mkdir = xfs_calc_mkdir_reservation(mp);
	resp->tr_ifree = xfs_calc_ifree_reservation(mp);
	resp->tr_ichange = xfs_calc_ichange_reservation(mp);
	resp->tr_growdata = xfs_calc_growdata_reservation(mp);
	resp->tr_swrite = xfs_calc_swrite_reservation(mp);
	resp->tr_writeid = xfs_calc_writeid_reservation(mp);
	resp->tr_addafork = xfs_calc_addafork_reservation(mp);
	resp->tr_attrinval = xfs_calc_attrinval_reservation(mp);
	resp->tr_attrset = xfs_calc_attrset_reservation(mp);
	resp->tr_attrrm = xfs_calc_attrrm_reservation(mp);
	resp->tr_clearagi = xfs_calc_clear_agi_bucket_reservation(mp);
	resp->tr_growrtalloc = xfs_calc_growrtalloc_reservation(mp);
	resp->tr_growrtzero = xfs_calc_growrtzero_reservation(mp);
	resp->tr_growrtfree = xfs_calc_growrtfree_reservation(mp);
}

/*
 * Add the given log item to the transaction's list of log items.
 *
 * The log item will now point to its new descriptor with its li_desc field.
 */
void
xfs_trans_add_item(
	struct xfs_trans	*tp,
	struct xfs_log_item	*lip)
{
	struct xfs_log_item_desc *lidp;

	ASSERT(lip->li_mountp = tp->t_mountp);
	ASSERT(lip->li_ailp = tp->t_mountp->m_ail);

	lidp = kmem_zone_zalloc(xfs_log_item_desc_zone, KM_SLEEP | KM_NOFS);

	lidp->lid_item = lip;
	lidp->lid_flags = 0;
	lidp->lid_size = 0;
	list_add_tail(&lidp->lid_trans, &tp->t_items);

	lip->li_desc = lidp;
}

STATIC void
xfs_trans_free_item_desc(
	struct xfs_log_item_desc *lidp)
{
	list_del_init(&lidp->lid_trans);
	kmem_zone_free(xfs_log_item_desc_zone, lidp);
}

/*
 * Unlink and free the given descriptor.
 */
void
xfs_trans_del_item(
	struct xfs_log_item	*lip)
{
	xfs_trans_free_item_desc(lip->li_desc);
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
xfs_trans_roll(
	struct xfs_trans	**tpp,
	struct xfs_inode	*dp)
{
	struct xfs_trans	*trans;
	unsigned int		logres, count;
	int			error;

	/*
	 * Ensure that the inode is always logged.
	 */
	trans = *tpp;
	xfs_trans_log_inode(trans, dp, XFS_ILOG_CORE);

	/*
	 * Copy the critical parameters from one trans to the next.
	 */
	logres = trans->t_log_res;
	count = trans->t_log_count;
	*tpp = xfs_trans_dup(trans);

	/*
	 * Commit the current transaction.
	 * If this commit failed, then it'd just unlock those items that
	 * are not marked ihold. That also means that a filesystem shutdown
	 * is in progress. The caller takes the responsibility to cancel
	 * the duplicate transaction that gets returned.
	 */
	error = xfs_trans_commit(trans, 0);
	if (error)
		return (error);

	trans = *tpp;

	/*
	 * Reserve space in the log for th next transaction.
	 * This also pushes items in the "AIL", the list of logged items,
	 * out to disk if they are taking up space at the tail of the log
	 * that we want to use.  This requires that either nothing be locked
	 * across this call, or that anything that is locked be logged in
	 * the prior and the next transactions.
	 */
	error = xfs_trans_reserve(trans, 0, logres, 0,
				  XFS_TRANS_PERM_LOG_RES, count);
	/*
	 *  Ensure that the inode is in the new transaction and locked.
	 */
	if (error)
		return error;

	xfs_trans_ijoin(trans, dp, XFS_ILOCK_EXCL);
	xfs_trans_ihold(trans, dp);
	return 0;
}

