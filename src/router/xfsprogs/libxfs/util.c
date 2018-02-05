/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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
#include "libxfs_io.h"
#include "init.h"
#include "xfs_fs.h"
#include "xfs_shared.h"
#include "xfs_format.h"
#include "xfs_log_format.h"
#include "xfs_trans_resv.h"
#include "xfs_mount.h"
#include "xfs_defer.h"
#include "xfs_inode_buf.h"
#include "xfs_inode_fork.h"
#include "xfs_inode.h"
#include "xfs_trans.h"
#include "xfs_bmap.h"
#include "xfs_bmap_btree.h"
#include "xfs_trans_space.h"
#include "xfs_ialloc.h"
#include "xfs_alloc.h"
#include "xfs_bit.h"
#include "xfs_da_format.h"
#include "xfs_da_btree.h"
#include "xfs_dir2_priv.h"

/*
 * Calculate the worst case log unit reservation for a given superblock
 * configuration. Copied and munged from the kernel code, and assumes a
 * worse case header usage (maximum log buffer sizes)
 */
int
xfs_log_calc_unit_res(
	struct xfs_mount	*mp,
	int			unit_bytes)
{
	int			iclog_space;
	int			iclog_header_size;
	int			iclog_size;
	uint			num_headers;

	if (xfs_sb_version_haslogv2(&mp->m_sb)) {
		iclog_size = XLOG_MAX_RECORD_BSIZE;
		iclog_header_size = BBTOB(iclog_size / XLOG_HEADER_CYCLE_SIZE);
	} else {
		iclog_size = XLOG_BIG_RECORD_BSIZE;
		iclog_header_size = BBSIZE;
	}

	/*
	 * Permanent reservations have up to 'cnt'-1 active log operations
	 * in the log.  A unit in this case is the amount of space for one
	 * of these log operations.  Normal reservations have a cnt of 1
	 * and their unit amount is the total amount of space required.
	 *
	 * The following lines of code account for non-transaction data
	 * which occupy space in the on-disk log.
	 *
	 * Normal form of a transaction is:
	 * <oph><trans-hdr><start-oph><reg1-oph><reg1><reg2-oph>...<commit-oph>
	 * and then there are LR hdrs, split-recs and roundoff at end of syncs.
	 *
	 * We need to account for all the leadup data and trailer data
	 * around the transaction data.
	 * And then we need to account for the worst case in terms of using
	 * more space.
	 * The worst case will happen if:
	 * - the placement of the transaction happens to be such that the
	 *   roundoff is at its maximum
	 * - the transaction data is synced before the commit record is synced
	 *   i.e. <transaction-data><roundoff> | <commit-rec><roundoff>
	 *   Therefore the commit record is in its own Log Record.
	 *   This can happen as the commit record is called with its
	 *   own region to xlog_write().
	 *   This then means that in the worst case, roundoff can happen for
	 *   the commit-rec as well.
	 *   The commit-rec is smaller than padding in this scenario and so it is
	 *   not added separately.
	 */

	/* for trans header */
	unit_bytes += sizeof(xlog_op_header_t);
	unit_bytes += sizeof(xfs_trans_header_t);

	/* for start-rec */
	unit_bytes += sizeof(xlog_op_header_t);

	/*
	 * for LR headers - the space for data in an iclog is the size minus
	 * the space used for the headers. If we use the iclog size, then we
	 * undercalculate the number of headers required.
	 *
	 * Furthermore - the addition of op headers for split-recs might
	 * increase the space required enough to require more log and op
	 * headers, so take that into account too.
	 *
	 * IMPORTANT: This reservation makes the assumption that if this
	 * transaction is the first in an iclog and hence has the LR headers
	 * accounted to it, then the remaining space in the iclog is
	 * exclusively for this transaction.  i.e. if the transaction is larger
	 * than the iclog, it will be the only thing in that iclog.
	 * Fundamentally, this means we must pass the entire log vector to
	 * xlog_write to guarantee this.
	 */
	iclog_space = iclog_size - iclog_header_size;
	num_headers = howmany(unit_bytes, iclog_space);

	/* for split-recs - ophdrs added when data split over LRs */
	unit_bytes += sizeof(xlog_op_header_t) * num_headers;

	/* add extra header reservations if we overrun */
	while (!num_headers ||
	       howmany(unit_bytes, iclog_space) > num_headers) {
		unit_bytes += sizeof(xlog_op_header_t);
		num_headers++;
	}
	unit_bytes += iclog_header_size * num_headers;

	/* for commit-rec LR header - note: padding will subsume the ophdr */
	unit_bytes += iclog_header_size;

	/* for roundoff padding for transaction data and one for commit record */
	if (xfs_sb_version_haslogv2(&mp->m_sb) && mp->m_sb.sb_logsunit > 1) {
		/* log su roundoff */
		unit_bytes += 2 * mp->m_sb.sb_logsunit;
	} else {
		/* BB roundoff */
		unit_bytes += 2 * BBSIZE;
        }

	return unit_bytes;
}

/*
 * Change the requested timestamp in the given inode.
 *
 * This was once shared with the kernel, but has diverged to the point
 * where it's no longer worth the hassle of maintaining common code.
 */
void
libxfs_trans_ichgtime(
	struct xfs_trans	*tp,
	struct xfs_inode	*ip,
	int			flags)
{
	struct timespec tv;
	struct timeval	stv;

	gettimeofday(&stv, (struct timezone *)0);
	tv.tv_sec = stv.tv_sec;
	tv.tv_nsec = stv.tv_usec * 1000;
	if (flags & XFS_ICHGTIME_MOD)
		VFS_I(ip)->i_mtime = tv;
	if (flags & XFS_ICHGTIME_CHG)
		VFS_I(ip)->i_ctime = tv;
	if (flags & XFS_ICHGTIME_CREATE) {
		ip->i_d.di_crtime.t_sec = (int32_t)tv.tv_sec;
		ip->i_d.di_crtime.t_nsec = (int32_t)tv.tv_nsec;
	}
}

STATIC uint16_t
xfs_flags2diflags(
	struct xfs_inode	*ip,
	unsigned int		xflags)
{
	/* can't set PREALLOC this way, just preserve it */
	uint16_t		di_flags =
		(ip->i_d.di_flags & XFS_DIFLAG_PREALLOC);

	if (xflags & FS_XFLAG_IMMUTABLE)
		di_flags |= XFS_DIFLAG_IMMUTABLE;
	if (xflags & FS_XFLAG_APPEND)
		di_flags |= XFS_DIFLAG_APPEND;
	if (xflags & FS_XFLAG_SYNC)
		di_flags |= XFS_DIFLAG_SYNC;
	if (xflags & FS_XFLAG_NOATIME)
		di_flags |= XFS_DIFLAG_NOATIME;
	if (xflags & FS_XFLAG_NODUMP)
		di_flags |= XFS_DIFLAG_NODUMP;
	if (xflags & FS_XFLAG_NODEFRAG)
		di_flags |= XFS_DIFLAG_NODEFRAG;
	if (xflags & FS_XFLAG_FILESTREAM)
		di_flags |= XFS_DIFLAG_FILESTREAM;
	if (S_ISDIR(VFS_I(ip)->i_mode)) {
		if (xflags & FS_XFLAG_RTINHERIT)
			di_flags |= XFS_DIFLAG_RTINHERIT;
		if (xflags & FS_XFLAG_NOSYMLINKS)
			di_flags |= XFS_DIFLAG_NOSYMLINKS;
		if (xflags & FS_XFLAG_EXTSZINHERIT)
			di_flags |= XFS_DIFLAG_EXTSZINHERIT;
		if (xflags & FS_XFLAG_PROJINHERIT)
			di_flags |= XFS_DIFLAG_PROJINHERIT;
	} else if (S_ISREG(VFS_I(ip)->i_mode)) {
		if (xflags & FS_XFLAG_REALTIME)
			di_flags |= XFS_DIFLAG_REALTIME;
		if (xflags & FS_XFLAG_EXTSIZE)
			di_flags |= XFS_DIFLAG_EXTSIZE;
	}

	return di_flags;
}

STATIC uint64_t
xfs_flags2diflags2(
	struct xfs_inode	*ip,
	unsigned int		xflags)
{
	uint64_t		di_flags2 =
		(ip->i_d.di_flags2 & XFS_DIFLAG2_REFLINK);

	if (xflags & FS_XFLAG_DAX)
		di_flags2 |= XFS_DIFLAG2_DAX;
	if (xflags & FS_XFLAG_COWEXTSIZE)
		di_flags2 |= XFS_DIFLAG2_COWEXTSIZE;

	return di_flags2;
}

/*
 * Allocate an inode on disk and return a copy of its in-core version.
 * Set mode, nlink, and rdev appropriately within the inode.
 * The uid and gid for the inode are set according to the contents of
 * the given cred structure.
 *
 * This was once shared with the kernel, but has diverged to the point
 * where it's no longer worth the hassle of maintaining common code.
 */
int
libxfs_ialloc(
	xfs_trans_t	*tp,
	xfs_inode_t	*pip,
	mode_t		mode,
	nlink_t		nlink,
	xfs_dev_t	rdev,
	struct cred	*cr,
	struct fsxattr	*fsx,
	int		okalloc,
	xfs_buf_t	**ialloc_context,
	xfs_inode_t	**ipp)
{
	xfs_ino_t	ino;
	xfs_inode_t	*ip;
	uint		flags;
	int		error;

	/*
	 * Call the space management code to pick
	 * the on-disk inode to be allocated.
	 */
	error = xfs_dialloc(tp, pip ? pip->i_ino : 0, mode, okalloc,
			    ialloc_context, &ino);
	if (error != 0)
		return error;
	if (*ialloc_context || ino == NULLFSINO) {
		*ipp = NULL;
		return 0;
	}
	ASSERT(*ialloc_context == NULL);

	error = xfs_trans_iget(tp->t_mountp, tp, ino, 0, 0, &ip);
	if (error != 0)
		return error;
	ASSERT(ip != NULL);

	VFS_I(ip)->i_mode = mode;
	set_nlink(VFS_I(ip), nlink);
	ip->i_d.di_uid = cr->cr_uid;
	ip->i_d.di_gid = cr->cr_gid;
	xfs_set_projid(&ip->i_d, pip ? 0 : fsx->fsx_projid);
	xfs_trans_ichgtime(tp, ip, XFS_ICHGTIME_CHG | XFS_ICHGTIME_MOD);

	/*
	 * We only support filesystems that understand v2 format inodes. So if
	 * this is currently an old format inode, then change the inode version
	 * number now.  This way we only do the conversion here rather than here
	 * and in the flush/logging code.
	 */
	if (ip->i_d.di_version == 1) {
		ip->i_d.di_version = 2;
		/*
		 * old link count, projid_lo/hi field, pad field
		 * already zeroed
		 */
	}

	if (pip && (VFS_I(pip)->i_mode & S_ISGID)) {
		ip->i_d.di_gid = pip->i_d.di_gid;
		if ((VFS_I(pip)->i_mode & S_ISGID) && (mode & S_IFMT) == S_IFDIR)
			VFS_I(ip)->i_mode |= S_ISGID;
	}

	ip->i_d.di_size = 0;
	ip->i_d.di_nextents = 0;
	ASSERT(ip->i_d.di_nblocks == 0);
	ip->i_d.di_extsize = pip ? 0 : fsx->fsx_extsize;
	ip->i_d.di_dmevmask = 0;
	ip->i_d.di_dmstate = 0;
	ip->i_d.di_flags = pip ? 0 : xfs_flags2diflags(ip, fsx->fsx_xflags);

	if (ip->i_d.di_version == 3) {
		ASSERT(ip->i_d.di_ino == ino);
		ASSERT(uuid_equal(&ip->i_d.di_uuid, &mp->m_sb.sb_meta_uuid));
		VFS_I(ip)->i_version = 1;
		ip->i_d.di_flags2 = pip ? 0 : xfs_flags2diflags2(ip,
				fsx->fsx_xflags);
		ip->i_d.di_crtime.t_sec = (int32_t)VFS_I(ip)->i_mtime.tv_sec;
		ip->i_d.di_crtime.t_nsec = (int32_t)VFS_I(ip)->i_mtime.tv_nsec;
		ip->i_d.di_cowextsize = pip ? 0 : fsx->fsx_cowextsize;
	}

	flags = XFS_ILOG_CORE;
	switch (mode & S_IFMT) {
	case S_IFIFO:
	case S_IFSOCK:
		/* doesn't make sense to set an rdev for these */
		rdev = 0;
		/* FALLTHROUGH */
	case S_IFCHR:
	case S_IFBLK:
		ip->i_d.di_format = XFS_DINODE_FMT_DEV;
		ip->i_df.if_u2.if_rdev = rdev;
		flags |= XFS_ILOG_DEV;
		break;
	case S_IFREG:
	case S_IFDIR:
		if (pip && (pip->i_d.di_flags & XFS_DIFLAG_ANY)) {
			uint	di_flags = 0;

			if ((mode & S_IFMT) == S_IFDIR) {
				if (pip->i_d.di_flags & XFS_DIFLAG_RTINHERIT)
					di_flags |= XFS_DIFLAG_RTINHERIT;
				if (pip->i_d.di_flags & XFS_DIFLAG_EXTSZINHERIT) {
					di_flags |= XFS_DIFLAG_EXTSZINHERIT;
					ip->i_d.di_extsize = pip->i_d.di_extsize;
				}
			} else {
				if (pip->i_d.di_flags & XFS_DIFLAG_RTINHERIT) {
					di_flags |= XFS_DIFLAG_REALTIME;
				}
				if (pip->i_d.di_flags & XFS_DIFLAG_EXTSZINHERIT) {
					di_flags |= XFS_DIFLAG_EXTSIZE;
					ip->i_d.di_extsize = pip->i_d.di_extsize;
				}
			}
			if (pip->i_d.di_flags & XFS_DIFLAG_PROJINHERIT)
				di_flags |= XFS_DIFLAG_PROJINHERIT;
			ip->i_d.di_flags |= di_flags;
		}
		/* FALLTHROUGH */
	case S_IFLNK:
		ip->i_d.di_format = XFS_DINODE_FMT_EXTENTS;
		ip->i_df.if_flags = XFS_IFEXTENTS;
		ip->i_df.if_bytes = ip->i_df.if_real_bytes = 0;
		ip->i_df.if_u1.if_extents = NULL;
		break;
	default:
		ASSERT(0);
	}
	/* Attribute fork settings for new inode. */
	ip->i_d.di_aformat = XFS_DINODE_FMT_EXTENTS;
	ip->i_d.di_anextents = 0;

	/*
	 * set up the inode ops structure that the libxfs code relies on
	 */
	if (XFS_ISDIR(ip))
		ip->d_ops = ip->i_mount->m_dir_inode_ops;
	else
		ip->d_ops = ip->i_mount->m_nondir_inode_ops;

	/*
	 * Log the new values stuffed into the inode.
	 */
	xfs_trans_log_inode(tp, ip, flags);
	*ipp = ip;
	return 0;
}

void
libxfs_iprint(
	xfs_inode_t		*ip)
{
	struct xfs_icdinode	*dip;
	xfs_bmbt_rec_host_t	*ep;
	xfs_extnum_t		i;
	xfs_extnum_t		nextents;

	printf("Inode %lx\n", (unsigned long)ip);
	printf("    i_ino %llx\n", (unsigned long long)ip->i_ino);

	if (ip->i_df.if_flags & XFS_IFEXTENTS)
		printf("EXTENTS ");
	printf("\n");
	printf("    i_df.if_bytes %d\n", ip->i_df.if_bytes);
	printf("    i_df.if_u1.if_extents/if_data %lx\n",
		(unsigned long)ip->i_df.if_u1.if_extents);
	if (ip->i_df.if_flags & XFS_IFEXTENTS) {
		nextents = ip->i_df.if_bytes / (uint)sizeof(*ep);
		for (ep = ip->i_df.if_u1.if_extents, i = 0; i < nextents;
								i++, ep++) {
			xfs_bmbt_irec_t rec;

			xfs_bmbt_get_all(ep, &rec);
			printf("\t%d: startoff %llu, startblock 0x%llx,"
				" blockcount %llu, state %d\n",
				i, (unsigned long long)rec.br_startoff,
				(unsigned long long)rec.br_startblock,
				(unsigned long long)rec.br_blockcount,
				(int)rec.br_state);
		}
	}
	printf("    i_df.if_broot %lx\n", (unsigned long)ip->i_df.if_broot);
	printf("    i_df.if_broot_bytes %x\n", ip->i_df.if_broot_bytes);

	dip = &ip->i_d;
	printf("\nOn disk portion\n");
	printf("    di_mode %o\n", VFS_I(ip)->i_mode);
	printf("    di_version %x\n", (uint)dip->di_version);
	switch (ip->i_d.di_format) {
	case XFS_DINODE_FMT_LOCAL:
		printf("    Inline inode\n");
		break;
	case XFS_DINODE_FMT_EXTENTS:
		printf("    Extents inode\n");
		break;
	case XFS_DINODE_FMT_BTREE:
		printf("    B-tree inode\n");
		break;
	default:
		printf("    Other inode\n");
		break;
	}
	printf("   di_nlink %x\n", VFS_I(ip)->i_nlink);
	printf("   di_uid %d\n", dip->di_uid);
	printf("   di_gid %d\n", dip->di_gid);
	printf("   di_nextents %d\n", dip->di_nextents);
	printf("   di_size %llu\n", (unsigned long long)dip->di_size);
	printf("   di_gen %x\n", VFS_I(ip)->i_generation);
	printf("   di_extsize %d\n", dip->di_extsize);
	printf("   di_flags %x\n", dip->di_flags);
	printf("   di_nblocks %llu\n", (unsigned long long)dip->di_nblocks);
}

/*
 * Writes a modified inode's changes out to the inode's on disk home.
 * Originally based on xfs_iflush_int() from xfs_inode.c in the kernel.
 */
int
libxfs_iflush_int(xfs_inode_t *ip, xfs_buf_t *bp)
{
	xfs_inode_log_item_t	*iip;
	xfs_dinode_t		*dip;
	xfs_mount_t		*mp;

	ASSERT(XFS_BUF_FSPRIVATE(bp, void *) != NULL);
	ASSERT(ip->i_d.di_format != XFS_DINODE_FMT_BTREE ||
		ip->i_d.di_nextents > ip->i_df.if_ext_max);
	ASSERT(ip->i_d.di_version > 1);

	iip = ip->i_itemp;
	mp = ip->i_mount;

	/* set *dip = inode's place in the buffer */
	dip = xfs_buf_offset(bp, ip->i_imap.im_boffset);

	ASSERT(ip->i_d.di_magic == XFS_DINODE_MAGIC);
	if (XFS_ISREG(ip)) {
		ASSERT( (ip->i_d.di_format == XFS_DINODE_FMT_EXTENTS) ||
			(ip->i_d.di_format == XFS_DINODE_FMT_BTREE) );
	} else if (XFS_ISDIR(ip)) {
		ASSERT( (ip->i_d.di_format == XFS_DINODE_FMT_EXTENTS) ||
			(ip->i_d.di_format == XFS_DINODE_FMT_BTREE)   ||
			(ip->i_d.di_format == XFS_DINODE_FMT_LOCAL) );
	}
	ASSERT(ip->i_d.di_nextents+ip->i_d.di_anextents <= ip->i_d.di_nblocks);
	ASSERT(ip->i_d.di_forkoff <= mp->m_sb.sb_inodesize);

	/* bump the change count on v3 inodes */
	if (ip->i_d.di_version == 3)
		VFS_I(ip)->i_version++;

	/* Check the inline directory data. */
	if (S_ISDIR(VFS_I(ip)->i_mode) &&
	    ip->i_d.di_format == XFS_DINODE_FMT_LOCAL &&
	    xfs_dir2_sf_verify(ip))
		return -EFSCORRUPTED;

	/*
	 * Copy the dirty parts of the inode into the on-disk
	 * inode.  We always copy out the core of the inode,
	 * because if the inode is dirty at all the core must
	 * be.
	 */
	xfs_inode_to_disk(ip, dip, iip->ili_item.li_lsn);

	xfs_iflush_fork(ip, dip, iip, XFS_DATA_FORK);
	if (XFS_IFORK_Q(ip))
		xfs_iflush_fork(ip, dip, iip, XFS_ATTR_FORK);

	/* generate the checksum. */
	xfs_dinode_calc_crc(mp, dip);

	return 0;
}

int
libxfs_mod_incore_sb(
	struct xfs_mount *mp,
	int		field,
	int64_t		delta,
	int		rsvd)
{
	long long	lcounter;	/* long counter for 64 bit fields */

	switch (field) {
	case XFS_TRANS_SB_FDBLOCKS:
		lcounter = (long long)mp->m_sb.sb_fdblocks;
		lcounter += delta;
		if (lcounter < 0)
			return -ENOSPC;
		mp->m_sb.sb_fdblocks = lcounter;
		return 0;
	default:
		ASSERT(0);
		return -EINVAL;
	}
}

/*
 * This routine allocates disk space for the given file.
 * Originally derived from xfs_alloc_file_space().
 */
int
libxfs_alloc_file_space(
	xfs_inode_t	*ip,
	xfs_off_t	offset,
	xfs_off_t	len,
	int		alloc_type,
	int		attr_flags)
{
	xfs_mount_t	*mp;
	xfs_off_t	count;
	xfs_filblks_t	datablocks;
	xfs_filblks_t	allocated_fsb;
	xfs_filblks_t	allocatesize_fsb;
	xfs_fsblock_t	firstfsb;
	struct xfs_defer_ops free_list;
	xfs_bmbt_irec_t *imapp;
	xfs_bmbt_irec_t imaps[1];
	int		reccount;
	uint		resblks;
	xfs_fileoff_t	startoffset_fsb;
	xfs_trans_t	*tp;
	int		xfs_bmapi_flags;
	int		error;

	if (len <= 0)
		return -EINVAL;

	count = len;
	error = 0;
	imapp = &imaps[0];
	reccount = 1;
	xfs_bmapi_flags = alloc_type ? XFS_BMAPI_PREALLOC : 0;
	mp = ip->i_mount;
	startoffset_fsb = XFS_B_TO_FSBT(mp, offset);
	allocatesize_fsb = XFS_B_TO_FSB(mp, count);

	/* allocate file space until done or until there is an error */
	while (allocatesize_fsb && !error) {
		datablocks = allocatesize_fsb;

		resblks = (uint)XFS_DIOSTRAT_SPACE_RES(mp, datablocks);
		error = xfs_trans_alloc(mp, &M_RES(mp)->tr_write, resblks,
					0, 0, &tp);
		/*
		 * Check for running out of space
		 */
		if (error) {
			ASSERT(error == -ENOSPC);
			break;
		}
		xfs_trans_ijoin(tp, ip, 0);

		xfs_defer_init(&free_list, &firstfsb);
		error = xfs_bmapi_write(tp, ip, startoffset_fsb, allocatesize_fsb,
				xfs_bmapi_flags, &firstfsb, 0, imapp,
				&reccount, &free_list);

		if (error)
			goto error0;

		/*
		 * Complete the transaction
		 */
		error = xfs_defer_finish(&tp, &free_list);
		if (error)
			goto error0;

		error = xfs_trans_commit(tp);
		if (error)
			break;

		allocated_fsb = imapp->br_blockcount;
		if (reccount == 0)
			return -ENOSPC;

		startoffset_fsb += allocated_fsb;
		allocatesize_fsb -= allocated_fsb;
	}
	return error;

error0:	/* Cancel bmap, cancel trans */
	xfs_defer_cancel(&free_list);
	xfs_trans_cancel(tp);
	return error;
}

unsigned int
libxfs_log2_roundup(unsigned int i)
{
	unsigned int	rval;

	for (rval = 0; rval < NBBY * sizeof(i); rval++) {
		if ((1 << rval) >= i)
			break;
	}
	return rval;
}

/*
 * Wrapper around call to libxfs_ialloc. Takes care of committing and
 * allocating a new transaction as needed.
 *
 * Originally there were two copies of this code - one in mkfs, the
 * other in repair - now there is just the one.
 */
int
libxfs_inode_alloc(
	xfs_trans_t	**tp,
	xfs_inode_t	*pip,
	mode_t		mode,
	nlink_t		nlink,
	xfs_dev_t	rdev,
	struct cred	*cr,
	struct fsxattr	*fsx,
	xfs_inode_t	**ipp)
{
	xfs_buf_t	*ialloc_context;
	xfs_inode_t	*ip;
	int		error;

	ialloc_context = (xfs_buf_t *)0;
	error = libxfs_ialloc(*tp, pip, mode, nlink, rdev, cr, fsx,
			   1, &ialloc_context, &ip);
	if (error) {
		*ipp = NULL;
		return error;
	}
	if (!ialloc_context && !ip) {
		*ipp = NULL;
		return -ENOSPC;
	}

	if (ialloc_context) {

		xfs_trans_bhold(*tp, ialloc_context);

		error = xfs_trans_roll(tp);
		if (error) {
			fprintf(stderr, _("%s: cannot duplicate transaction: %s\n"),
				progname, strerror(error));
			exit(1);
		}
		xfs_trans_bjoin(*tp, ialloc_context);
		error = libxfs_ialloc(*tp, pip, mode, nlink, rdev, cr,
				   fsx, 1, &ialloc_context, &ip);
		if (!ip)
			error = -ENOSPC;
		if (error)
			return error;
	}

	*ipp = ip;
	return error;
}

/*
 * Userspace versions of common diagnostic routines (varargs fun).
 */
void
libxfs_fs_repair_cmn_err(int level, xfs_mount_t *mp, char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "  This is a bug.\n");
	fprintf(stderr, "%s version %s\n", progname, VERSION);
	fprintf(stderr,
		"Please capture the filesystem metadata with xfs_metadump and\n"
		"report it to linux-xfs@vger.kernel.org\n");
	va_end(ap);
}

void
libxfs_fs_cmn_err(int level, xfs_mount_t *mp, char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	va_end(ap);
}

void
cmn_err(int level, char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	va_end(ap);
}

/*
 * Warnings specifically for verifier errors.  Differentiate CRC vs. invalid
 * values, and omit the stack trace unless the error level is tuned high.
 */
void
xfs_verifier_error(
	struct xfs_buf		*bp)
{
	xfs_alert(NULL, "Metadata %s detected at %s block 0x%llx/0x%x",
		  bp->b_error == -EFSBADCRC ? "CRC error" : "corruption",
		  bp->b_ops->name, bp->b_bn, BBTOB(bp->b_length));
}

/*
 * This is called from I/O verifiers on v5 superblock filesystems. In the
 * kernel, it validates the metadata LSN parameter against the current LSN of
 * the active log. We don't have an active log in userspace so this kind of
 * validation is not required. Therefore, this function always returns true in
 * userspace.
 *
 * xfs_repair piggybacks off this mechanism to help track the largest metadata
 * LSN in use on a filesystem. Keep a record of the largest LSN seen such that
 * repair can validate it against the state of the log.
 */
xfs_lsn_t	libxfs_max_lsn = 0;
pthread_mutex_t	libxfs_max_lsn_lock = PTHREAD_MUTEX_INITIALIZER;

bool
xfs_log_check_lsn(
	struct xfs_mount	*mp,
	xfs_lsn_t		lsn)
{
	int			cycle = CYCLE_LSN(lsn);
	int			block = BLOCK_LSN(lsn);
	int			max_cycle;
	int			max_block;

	if (lsn == NULLCOMMITLSN)
		return true;

	pthread_mutex_lock(&libxfs_max_lsn_lock);

	max_cycle = CYCLE_LSN(libxfs_max_lsn);
	max_block = BLOCK_LSN(libxfs_max_lsn);

	if ((cycle > max_cycle) ||
	    (cycle == max_cycle && block > max_block))
		libxfs_max_lsn = lsn;

	pthread_mutex_unlock(&libxfs_max_lsn_lock);

	return true;
}

static struct xfs_buftarg *
xfs_find_bdev_for_inode(
	struct xfs_inode	*ip)
{
	struct xfs_mount	*mp = ip->i_mount;

	if (XFS_IS_REALTIME_INODE(ip))
		return mp->m_rtdev_targp;
	return mp->m_ddev_targp;
}

static xfs_daddr_t
xfs_fsb_to_db(struct xfs_inode *ip, xfs_fsblock_t fsb)
{
	if (XFS_IS_REALTIME_INODE(ip))
		 return XFS_FSB_TO_BB(ip->i_mount, fsb);
	return XFS_FSB_TO_DADDR(ip->i_mount, (fsb));
}

int
libxfs_zero_extent(
	struct xfs_inode *ip,
	xfs_fsblock_t	start_fsb,
	xfs_off_t	count_fsb)
{
	xfs_daddr_t	sector = xfs_fsb_to_db(ip, start_fsb);
	ssize_t		size = XFS_FSB_TO_BB(ip->i_mount, count_fsb);

	return libxfs_device_zero(xfs_find_bdev_for_inode(ip), sector, size);
}

