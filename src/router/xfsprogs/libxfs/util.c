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

#include <xfs.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

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
	if (flags & XFS_ICHGTIME_MOD) {
		ip->i_d.di_mtime.t_sec = (__int32_t)tv.tv_sec;
		ip->i_d.di_mtime.t_nsec = (__int32_t)tv.tv_nsec;
	}
	if (flags & XFS_ICHGTIME_CHG) {
		ip->i_d.di_ctime.t_sec = (__int32_t)tv.tv_sec;
		ip->i_d.di_ctime.t_nsec = (__int32_t)tv.tv_nsec;
	}
}

/*
 * Given a mount structure and an inode number, return a pointer
 * to a newly allocated in-core inode coresponding to the given
 * inode number.
 *
 * Initialize the inode's attributes and extent pointers if it
 * already has them (it will not if the inode has no links).
 *
 * NOTE: this has slightly different behaviour to the kernel in
 * that this version requires the already allocated *ip being 
 * passed in while the kernel version does the allocation and 
 * returns it in **ip.
 */
int
libxfs_iread(
	xfs_mount_t     *mp,
	xfs_trans_t	*tp,
	xfs_ino_t	ino,
	xfs_inode_t	*ip,
	xfs_daddr_t	bno)
{
	xfs_buf_t	*bp;
	xfs_dinode_t	*dip;
	int		error;

	ip->i_ino = ino;
	ip->i_mount = mp;

        /*
         * Fill in the location information in the in-core inode.
         */
        error = xfs_imap(mp, tp, ip->i_ino, &ip->i_imap, 0);
        if (error)
                return error;

        /*
         * Get pointers to the on-disk inode and the buffer containing it.
         */
        error = xfs_imap_to_bp(mp, tp, &ip->i_imap, &bp, XBF_LOCK, 0);
        if (error)
                return error;
        dip = (xfs_dinode_t *)xfs_buf_offset(bp, ip->i_imap.im_boffset);

	/*
	 * If we got something that isn't an inode it means someone
	 * (nfs or dmi) has a stale handle.
	 */
	if (be16_to_cpu(dip->di_magic) != XFS_DINODE_MAGIC) {
		xfs_trans_brelse(tp, bp);
		return EINVAL;
	}

	/*
	 * If the on-disk inode is already linked to a directory
	 * entry, copy all of the inode into the in-core inode.
	 * xfs_iformat() handles copying in the inode format
	 * specific information.
	 * Otherwise, just get the truly permanent information.
	 */
	if (dip->di_mode) {
		xfs_dinode_from_disk(&ip->i_d, dip);
		error = xfs_iformat(ip, dip);
		if (error)  {
			xfs_trans_brelse(tp, bp);
			return error;
		}
	} else {
		ip->i_d.di_magic = be16_to_cpu(dip->di_magic);
		ip->i_d.di_version = dip->di_version;
		ip->i_d.di_gen = be32_to_cpu(dip->di_gen);
		ip->i_d.di_flushiter = be16_to_cpu(dip->di_flushiter);
		/*
		 * Make sure to pull in the mode here as well in
		 * case the inode is released without being used.
		 * This ensures that xfs_inactive() will see that
		 * the inode is already free and not try to mess
		 * with the uninitialized part of it.
		 */
		ip->i_d.di_mode = 0;
		/*
		 * Initialize the per-fork minima and maxima for a new
		 * inode here.  xfs_iformat will do it for old inodes.
		 */
		ip->i_df.if_ext_max =
			XFS_IFORK_DSIZE(ip) / (uint)sizeof(xfs_bmbt_rec_t);
	}

	/*
	 * The inode format changed when we moved the link count and
	 * made it 32 bits long.  If this is an old format inode,
	 * convert it in memory to look like a new one.  If it gets
	 * flushed to disk we will convert back before flushing or
	 * logging it.  We zero out the new projid_lo/hi field and the old link
	 * count field.  We'll handle clearing the pad field (the remains
	 * of the old uuid field) when we actually convert the inode to
	 * the new format. We don't change the version number so that we
	 * can distinguish this from a real new format inode.
	 */
	if (ip->i_d.di_version == 1) {
		ip->i_d.di_nlink = ip->i_d.di_onlink;
		ip->i_d.di_onlink = 0;
		xfs_set_projid(&ip->i_d, 0);
	}

	ip->i_delayed_blks = 0;
	ip->i_size = ip->i_d.di_size;

	/*
	 * Use xfs_trans_brelse() to release the buffer containing the
	 * on-disk inode, because it was acquired with xfs_trans_read_buf()
	 * in xfs_itobp() above.  If tp is NULL, this is just a normal
	 * brelse().  If we're within a transaction, then xfs_trans_brelse()
	 * will only release the buffer if it is not dirty within the
	 * transaction.  It will be OK to release the buffer in this case,
	 * because inodes on disk are never destroyed and we will be
	 * locking the new in-core inode before putting it in the hash
	 * table where other processes can find it.  Thus we don't have
	 * to worry about the inode being changed just because we released
	 * the buffer.
	 */
	xfs_trans_brelse(tp, bp);
	return 0;
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
	boolean_t	*call_again,
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
			    ialloc_context, call_again, &ino);
	if (error != 0)
		return error;
	if (*call_again || ino == NULLFSINO) {
		*ipp = NULL;
		return 0;
	}
	ASSERT(*ialloc_context == NULL);

	error = xfs_trans_iget(tp->t_mountp, tp, ino, 0, 0, &ip);
	if (error != 0)
		return error;
	ASSERT(ip != NULL);

	ip->i_d.di_mode = (__uint16_t)mode;
	ip->i_d.di_onlink = 0;
	ip->i_d.di_nlink = nlink;
	ASSERT(ip->i_d.di_nlink == nlink);
	ip->i_d.di_uid = cr->cr_uid;
	ip->i_d.di_gid = cr->cr_gid;
	xfs_set_projid(&ip->i_d, pip ? 0 : fsx->fsx_projid);
	memset(&(ip->i_d.di_pad[0]), 0, sizeof(ip->i_d.di_pad));

	/*
	 * If the superblock version is up to where we support new format
	 * inodes and this is currently an old format inode, then change
	 * the inode version number now.  This way we only do the conversion
	 * here rather than here and in the flush/logging code.
	 */
	if (xfs_sb_version_hasnlink(&tp->t_mountp->m_sb) &&
	    ip->i_d.di_version == 1) {
		ip->i_d.di_version = 2;
		/*
		 * old link count, projid_lo/hi field, pad field
		 * already zeroed
		 */
	}

	if (pip && (pip->i_d.di_mode & S_ISGID)) {
		ip->i_d.di_gid = pip->i_d.di_gid;
		if ((pip->i_d.di_mode & S_ISGID) && (mode & S_IFMT) == S_IFDIR)
			ip->i_d.di_mode |= S_ISGID;
	}

	ip->i_d.di_size = 0;
	ip->i_d.di_nextents = 0;
	ASSERT(ip->i_d.di_nblocks == 0);
	xfs_trans_ichgtime(tp, ip, XFS_ICHGTIME_CHG|XFS_ICHGTIME_MOD);
	/*
	 * di_gen will have been taken care of in xfs_iread.
	 */
	ip->i_d.di_extsize = pip ? 0 : fsx->fsx_extsize;
	ip->i_d.di_dmevmask = 0;
	ip->i_d.di_dmstate = 0;
	ip->i_d.di_flags = pip ? 0 : fsx->fsx_xflags;
	flags = XFS_ILOG_CORE;
	switch (mode & S_IFMT) {
	case S_IFIFO:
	case S_IFSOCK:
		/* doesn't make sense to set an rdev for these */
		rdev = 0;
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
	xfs_icdinode_t		*dip;
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
	printf("    di_magic %x\n", dip->di_magic);
	printf("    di_mode %o\n", dip->di_mode);
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
	printf("   di_nlink %x\n", dip->di_nlink);
	printf("   di_uid %d\n", dip->di_uid);
	printf("   di_gid %d\n", dip->di_gid);
	printf("   di_nextents %d\n", dip->di_nextents);
	printf("   di_size %llu\n", (unsigned long long)dip->di_size);
	printf("   di_gen %x\n", dip->di_gen);
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

	iip = ip->i_itemp;
	mp = ip->i_mount;

	/* set *dip = inode's place in the buffer */
	dip = (xfs_dinode_t *)xfs_buf_offset(bp, ip->i_imap.im_boffset);

	ASSERT(ip->i_d.di_magic == XFS_DINODE_MAGIC);
	if ((ip->i_d.di_mode & S_IFMT) == S_IFREG) {
		ASSERT( (ip->i_d.di_format == XFS_DINODE_FMT_EXTENTS) ||
			(ip->i_d.di_format == XFS_DINODE_FMT_BTREE) );
	}
	else if ((ip->i_d.di_mode & S_IFMT) == S_IFDIR) {
		ASSERT( (ip->i_d.di_format == XFS_DINODE_FMT_EXTENTS) ||
			(ip->i_d.di_format == XFS_DINODE_FMT_BTREE)   ||
			(ip->i_d.di_format == XFS_DINODE_FMT_LOCAL) );
	}
	ASSERT(ip->i_d.di_nextents+ip->i_d.di_anextents <= ip->i_d.di_nblocks);
	ASSERT(ip->i_d.di_forkoff <= mp->m_sb.sb_inodesize);

	/*
	 * Copy the dirty parts of the inode into the on-disk
	 * inode.  We always copy out the core of the inode,
	 * because if the inode is dirty at all the core must
	 * be.
	 */
	xfs_dinode_to_disk(dip, &ip->i_d);

	/*
	 * If this is really an old format inode and the superblock version
	 * has not been updated to support only new format inodes, then
	 * convert back to the old inode format.  If the superblock version
	 * has been updated, then make the conversion permanent.
	 */
	ASSERT(ip->i_d.di_version == 1 ||
		xfs_sb_version_hasnlink(&mp->m_sb));
	if (ip->i_d.di_version == 1) {
		if (!xfs_sb_version_hasnlink(&mp->m_sb)) {
			/*
			 * Convert it back.
			 */
			ASSERT(ip->i_d.di_nlink <= XFS_MAXLINK_1);
			dip->di_onlink = cpu_to_be16(ip->i_d.di_nlink);
		} else {
			/*
			 * The superblock version has already been bumped,
			 * so just make the conversion to the new inode
			 * format permanent.
			 */
			ip->i_d.di_version = 2;
			dip->di_version =  2;
			ip->i_d.di_onlink = 0;
			dip->di_onlink = 0;
			memset(&(ip->i_d.di_pad[0]), 0, sizeof(ip->i_d.di_pad));
			memset(&(dip->di_pad[0]), 0, sizeof(dip->di_pad));
			ASSERT(xfs_get_projid(ip->i_d) == 0);
		}
	}

	xfs_iflush_fork(ip, dip, iip, XFS_DATA_FORK, bp);
	if (XFS_IFORK_Q(ip)) 
		xfs_iflush_fork(ip, dip, iip, XFS_ATTR_FORK, bp);

	return 0;
}

/*
 * Utility routine common used to apply a delta to a field in the
 * in-core superblock.
 * Switch on the field indicated and apply the delta to that field.
 * Fields are not allowed to dip below zero, so if the delta would
 * do this do not apply it and return EINVAL.
 *
 * Originally derived from xfs_mod_incore_sb_unlocked().
 */
int
libxfs_mod_incore_sb(
	xfs_mount_t	*mp,
	xfs_sb_field_t	field,
	int64_t		delta,
	int		rsvd)
{
	long long	lcounter;	/* long counter for 64 bit fields */

	switch (field) {
	case XFS_SBS_FDBLOCKS:
		lcounter = (long long)mp->m_sb.sb_fdblocks;
		lcounter += delta;
		if (lcounter < 0)
			return XFS_ERROR(ENOSPC);
		mp->m_sb.sb_fdblocks = lcounter;
		return 0;
	default:
		ASSERT(0);
		return XFS_ERROR(EINVAL);
	}
}

int
libxfs_bmap_finish(
	xfs_trans_t	**tp,
	xfs_bmap_free_t *flist,
	int		*committed)
{
	xfs_bmap_free_item_t	*free;	/* free extent list item */
	xfs_bmap_free_item_t	*next;	/* next item on free list */
	int			error;

	if (flist->xbf_count == 0) {
		*committed = 0;
		return 0;
	}

	for (free = flist->xbf_first; free != NULL; free = next) {
		next = free->xbfi_next;
		if ((error = xfs_free_extent(*tp, free->xbfi_startblock,
				free->xbfi_blockcount)))
			return error;
		xfs_bmap_del_free(flist, NULL, free);
	}
	*committed = 0;
	return 0;
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
	xfs_bmap_free_t free_list;
	xfs_bmbt_irec_t *imapp;
	xfs_bmbt_irec_t imaps[1];
	int		reccount;
	uint		resblks;
	xfs_fileoff_t	startoffset_fsb;
	xfs_trans_t	*tp;
	int		xfs_bmapi_flags;
	int		committed;
	int		error;

	if (len <= 0)
		return EINVAL;

	count = len;
	error = 0;
	imapp = &imaps[0];
	reccount = 1;
	xfs_bmapi_flags = XFS_BMAPI_WRITE | (alloc_type ? XFS_BMAPI_PREALLOC : 0);
	mp = ip->i_mount;
	startoffset_fsb = XFS_B_TO_FSBT(mp, offset);
	allocatesize_fsb = XFS_B_TO_FSB(mp, count);

	/* allocate file space until done or until there is an error */
	while (allocatesize_fsb && !error) {
		datablocks = allocatesize_fsb;

		tp = xfs_trans_alloc(mp, XFS_TRANS_DIOSTRAT);
		resblks = (uint)XFS_DIOSTRAT_SPACE_RES(mp, datablocks);
		error = xfs_trans_reserve(tp, resblks, 0, 0, 0, 0);
		if (error)
			break;
		xfs_trans_ijoin(tp, ip, 0);
		xfs_trans_ihold(tp, ip);

		xfs_bmap_init(&free_list, &firstfsb);
		error = xfs_bmapi(tp, ip, startoffset_fsb, allocatesize_fsb,
				xfs_bmapi_flags, &firstfsb, 0, imapp,
				&reccount, &free_list);

		if (error)
			break;

		/* complete the transaction */
		error = xfs_bmap_finish(&tp, &free_list, &committed);
		if (error)
			break;

		error = xfs_trans_commit(tp, 0);
		if (error)
			break;

		allocated_fsb = imapp->br_blockcount;
		if (reccount == 0)
			return ENOSPC;

		startoffset_fsb += allocated_fsb;
		allocatesize_fsb -= allocated_fsb;
	}
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
 * Get a buffer for the dir/attr block, fill in the contents.
 * Don't check magic number, the caller will (it's xfs_repair).
 *
 * Originally from xfs_da_btree.c in the kernel, but only used
 * in userspace so it now resides here.
 */
int
libxfs_da_read_bufr(
	xfs_trans_t	*trans,
	xfs_inode_t	*dp,
	xfs_dablk_t	bno,
	xfs_daddr_t	mappedbno,
	xfs_dabuf_t	**bpp,
	int		whichfork)
{
	return xfs_da_do_buf(trans, dp, bno, &mappedbno, bpp, whichfork, 2,
		(inst_t *)__return_address);
}

/*
 * Hold dabuf at transaction commit.
 *
 * Originally from xfs_da_btree.c in the kernel, but only used
 * in userspace so it now resides here.
 */
void
libxfs_da_bhold(xfs_trans_t *tp, xfs_dabuf_t *dabuf)
{
	int	i;

	for (i = 0; i < dabuf->nbuf; i++)
		xfs_trans_bhold(tp, dabuf->bps[i]);
}

/*
 * Join dabuf to transaction.
 *
 * Originally from xfs_da_btree.c in the kernel, but only used
 * in userspace so it now resides here.
 */
void
libxfs_da_bjoin(xfs_trans_t *tp, xfs_dabuf_t *dabuf)
{
	int	i;

	for (i = 0; i < dabuf->nbuf; i++)
		xfs_trans_bjoin(tp, dabuf->bps[i]);
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
	boolean_t	call_again;
	int		i;
	xfs_buf_t	*ialloc_context;
	xfs_inode_t	*ip;
	xfs_trans_t	*ntp;
	int		error;

	call_again = B_FALSE;
	ialloc_context = (xfs_buf_t *)0;
	error = libxfs_ialloc(*tp, pip, mode, nlink, rdev, cr, fsx,
			   1, &ialloc_context, &call_again, &ip);
	if (error)
		return error;

	if (call_again) {
		xfs_trans_bhold(*tp, ialloc_context);
		ntp = xfs_trans_dup(*tp);
		xfs_trans_commit(*tp, 0);
		*tp = ntp;
		if ((i = xfs_trans_reserve(*tp, 0, 0, 0, 0, 0))) {
			fprintf(stderr, _("%s: cannot reserve space: %s\n"),
				progname, strerror(i));
			exit(1);
		}
		xfs_trans_bjoin(*tp, ialloc_context);
		error = libxfs_ialloc(*tp, pip, mode, nlink, rdev, cr,
				   fsx, 1, &ialloc_context,
				   &call_again, &ip);
		if (!ip)
			error = ENOSPC;
		if (error)
			return error;
	}
	if (!ip)
		error = ENOSPC;

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
	fprintf(stderr, "Please capture the filesystem metadata with "
			"xfs_metadump and\nreport it to xfs@oss.sgi.com.\n");
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
