/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
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

#include "libxfs.h"
#include "libxcmd.h"
#include "libxlog.h"
#include "agheader.h"
#include "globals.h"
#include "protos.h"
#include "err_protos.h"
#include "xfs_multidisk.h"

#define BSIZE	(1024 * 1024)

/*
 * copy the fields of a superblock that are present in primary and
 * secondaries -- preserve fields that are different in the primary.
 */
static void
copy_sb(xfs_sb_t *source, xfs_sb_t *dest)
{
	xfs_ino_t	rootino;
	xfs_ino_t	rbmino;
	xfs_ino_t	rsumino;
	xfs_ino_t	uquotino;
	xfs_ino_t	gquotino;
	xfs_ino_t	pquotino;
	uint16_t	versionnum;

	rootino = dest->sb_rootino;
	rbmino = dest->sb_rbmino;
	rsumino = dest->sb_rsumino;
	uquotino = dest->sb_uquotino;
	gquotino = dest->sb_gquotino;
	pquotino = dest->sb_pquotino;

	versionnum = dest->sb_versionnum;

	*dest = *source;

	dest->sb_rootino = rootino;
	dest->sb_rbmino = rbmino;
	dest->sb_rsumino = rsumino;
	dest->sb_uquotino = uquotino;
	dest->sb_gquotino = gquotino;
	dest->sb_pquotino = pquotino;

	dest->sb_versionnum = versionnum;

	/*
	 * copy over version bits that are stamped into all
	 * secondaries and cannot be changed at run time in
	 * the primary superblock
	 */
	if (xfs_sb_version_hasdalign(source))
		dest->sb_versionnum |= XFS_SB_VERSION_DALIGNBIT;
	if (xfs_sb_version_hasextflgbit(source))
		dest->sb_versionnum |= XFS_SB_VERSION_EXTFLGBIT;

	/*
	 * these are all supposed to be zero or will get reset anyway
	 */
	dest->sb_icount = 0;
	dest->sb_ifree = 0;
	dest->sb_fdblocks = 0;
	dest->sb_frextents = 0;

	memset(source->sb_fname, 0, 12);
}

int
verify_sb_blocksize(xfs_sb_t *sb)
{
	/* check to make sure blocksize is legal 2^N, 9 <= N <= 16 */
	if (sb->sb_blocksize == 0)
		return XR_BAD_BLOCKSIZE;
	if (sb->sb_blocksize != (1 << sb->sb_blocklog))
		return XR_BAD_BLOCKLOG;
	if (sb->sb_blocklog < XFS_MIN_BLOCKSIZE_LOG ||
	    sb->sb_blocklog > XFS_MAX_BLOCKSIZE_LOG)
		return XR_BAD_BLOCKLOG;

	return 0;
}

/*
 * find a secondary superblock, copy it into the sb buffer.
 * start is the point to begin reading BSIZE bytes.
 * skip contains a byte-count of how far to advance for next read.
 */
static int
__find_secondary_sb(
	xfs_sb_t	*rsb,
	uint64_t	start,
	uint64_t	skip)
{
	xfs_off_t	off;
	xfs_sb_t	*sb;
	xfs_sb_t	bufsb;
	char		*c_bufsb;
	int		done;
	int		i;
	int		dirty;
	int		retval;
	int		bsize;

	sb = (xfs_sb_t *)memalign(libxfs_device_alignment(), BSIZE);
	if (!sb) {
		do_error(
	_("error finding secondary superblock -- failed to memalign buffer\n"));
		exit(1);
	}

	memset(&bufsb, 0, sizeof(xfs_sb_t));
	retval = 0;
	dirty = 0;
	bsize = 0;

	/*
	 * skip first sector since we know that's bad
	 */
	for (done = 0, off = start; !done ; off += skip)  {
		/*
		 * read disk 1 MByte at a time.
		 */
		if (lseek(x.dfd, off, SEEK_SET) != off)  {
			done = 1;
		}

		if (!done && (bsize = read(x.dfd, sb, BSIZE)) <= 0)  {
			done = 1;
		}

		do_warn(".");

		/*
		 * check the buffer 512 bytes at a time since
		 * we don't know how big the sectors really are.
		 */
		for (i = 0; !done && i < bsize; i += BBSIZE)  {
			c_bufsb = (char *)sb + i;
			libxfs_sb_from_disk(&bufsb, (xfs_dsb_t *)c_bufsb);

			if (verify_sb(c_bufsb, &bufsb, 0) != XR_OK)
				continue;

			do_warn(_("found candidate secondary superblock...\n"));

			/*
			 * found one.  now verify it by looking
			 * for other secondaries.
			 */
			memmove(rsb, &bufsb, sizeof(xfs_sb_t));
			rsb->sb_inprogress = 0;
			copied_sunit = 1;

			if (verify_set_primary_sb(rsb, 0, &dirty) == XR_OK)  {
				do_warn(
			_("verified secondary superblock...\n"));
				done = 1;
				retval = 1;
			} else  {
				do_warn(
			_("unable to verify superblock, continuing...\n"));
			}
		}
	}

	free(sb);
	return retval;
}

static int
guess_default_geometry(
	uint64_t		*agsize,
	uint64_t		*agcount,
	libxfs_init_t		*x)
{
	struct fs_topology	ft;
	int			blocklog;
	uint64_t		dblocks;
	int			multidisk;

	memset(&ft, 0, sizeof(ft));
	get_topology(x, &ft, 1);

	/*
	 * get geometry from get_topology result.
	 * Use default block size (2^12)
	 */
	blocklog = 12;
	multidisk = ft.dswidth | ft.dsunit;
	dblocks = x->dsize >> (blocklog - BBSHIFT);
	calc_default_ag_geometry(blocklog, dblocks, multidisk,
				 agsize, agcount);

	return blocklog;
}

int
find_secondary_sb(xfs_sb_t *rsb)
{
	int		retval = 0;
	uint64_t	agcount;
	uint64_t	agsize;
	uint64_t	skip;
	int		blocklog;

	/*
	 * Attempt to find secondary sb with a coarse approach,
	 * first trying agblocks and blocksize read from sb, providing
	 * they're sane.
	 */
	do_warn(_("\nattempting to find secondary superblock...\n"));

	if (verify_sb_blocksize(rsb) == 0) {
		skip = (uint64_t)rsb->sb_agblocks * rsb->sb_blocksize;
		if (skip >= XFS_AG_MIN_BYTES && skip <= XFS_AG_MAX_BYTES)
			retval = __find_secondary_sb(rsb, skip, skip);
	}

        /* If that failed, retry coarse approach, using default geometry */
        if (!retval) {
                blocklog = guess_default_geometry(&agsize, &agcount, &x);
                skip = agsize << blocklog;
                retval = __find_secondary_sb(rsb, skip, skip);
        }

        /* If that failed, fall back to the brute force method */
        if (!retval)
                retval = __find_secondary_sb(rsb, XFS_AG_MIN_BYTES, BSIZE);

	return retval;
}

/*
 * Calculate what the inode alignment field ought to be based on internal
 * superblock info and determine if it is valid.
 *
 * For standard v5 superblocks, the inode alignment must either match
 * XFS_INODE_BIG_CLUSTER_SIZE or a multiple based on the inode size. For v5
 * superblocks with sparse inode chunks enabled, inode alignment must match the
 * inode chunk size.
 *
 * Return true if the alignment is valid, false otherwise.
 */
static bool
sb_validate_ino_align(struct xfs_sb *sb)
{
	xfs_extlen_t	align;

	if (!xfs_sb_version_hasalign(sb))
		return true;

	/* standard cluster size alignment is always valid */
	align = XFS_INODE_BIG_CLUSTER_SIZE >> sb->sb_blocklog;
	if (align == sb->sb_inoalignmt)
		return true;

	/* alignment scaled by inode size is v5 only for now */
	if (!xfs_sb_version_hascrc(sb))
		return false;

	align = (XFS_INODE_BIG_CLUSTER_SIZE *
		 sb->sb_inodesize / XFS_DINODE_MIN_SIZE) >> sb->sb_blocklog;
	if (align == sb->sb_inoalignmt)
		return true;

	/*
	 * Sparse inodes requires inoalignmt to match full inode chunk size and
	 * spino_align to match the scaled alignment (as calculated above).
	 */
	if (xfs_sb_version_hassparseinodes(sb)) {
		if (align != sb->sb_spino_align)
			return false;

		align = (sb->sb_inodesize * XFS_INODES_PER_CHUNK)
			>> sb->sb_blocklog;
		if (align == sb->sb_inoalignmt)
			return true;
	}

	return false;
}

/*
 * verify a superblock -- does not verify root inode #
 *	can only check that geometry info is internally
 *	consistent.  because of growfs, that's no guarantee
 *	of correctness (e.g. geometry may have changed)
 *
 * fields verified or consistency checked:
 *
 *			sb_magicnum
 *
 *			sb_versionnum
 *
 *			sb_inprogress
 *
 *			sb_blocksize	(as a group)
 *			sb_blocklog
 *
 * geometry info -	sb_dblocks	(as a group)
 *			sb_agcount
 *			sb_agblocks
 *			sb_agblklog
 *
 * inode info -		sb_inodesize	(x-checked with geo info)
 *			sb_inopblock
 *
 * sector size info -
 *			sb_sectsize
 *			sb_sectlog
 *			sb_logsectsize
 *			sb_logsectlog
 *
 * not checked here -
 *			sb_rootino
 *			sb_fname
 *			sb_fpack
 *			sb_logstart
 *			sb_uuid
 *
 *			ALL real-time fields
 *			final 4 summary counters
 */

int
verify_sb(char *sb_buf, xfs_sb_t *sb, int is_primary_sb)
{
	uint32_t	bsize;
	int		i;
	int		ret;

	/* check magic number and version number */

	if (sb->sb_magicnum != XFS_SB_MAGIC)
		return(XR_BAD_MAGIC);

	if (!xfs_sb_good_version(sb))
		return(XR_BAD_VERSION);

	/* does sb think mkfs really finished ? */

	if (is_primary_sb && sb->sb_inprogress == 1)
		return(XR_BAD_INPROGRESS);

	/*
	 * before going *any further*, validate the sector size and if the
	 * version says we should have CRCs enabled, validate that.
	 */

	/* check to make sure sectorsize is legal 2^N, 9 <= N <= 15 */
	if (sb->sb_sectsize == 0)
		return(XR_BAD_SECT_SIZE_DATA);

	bsize = 1;
	for (i = 0; bsize < sb->sb_sectsize &&
		i < sizeof(sb->sb_sectsize) * NBBY; i++)  {
		bsize <<= 1;
	}

	if (i < XFS_MIN_SECTORSIZE_LOG || i > XFS_MAX_SECTORSIZE_LOG)
		return(XR_BAD_SECT_SIZE_DATA);

	/* check sb sectorsize field against sb sectlog field */
	if (i != sb->sb_sectlog)
		return(XR_BAD_SECT_SIZE_DATA);

	/* sector size in range - CRC check time */
	if (xfs_sb_version_hascrc(sb) &&
	    !libxfs_verify_cksum(sb_buf, sb->sb_sectsize, XFS_SB_CRC_OFF))
		return XR_BAD_CRC;

	/* check to ensure blocksize and blocklog are legal */
	ret = verify_sb_blocksize(sb);
	if (ret != 0)
		return ret;

	/* sanity check ag count, size fields against data size field */

	if (sb->sb_dblocks == 0 ||
		sb->sb_dblocks > XFS_MAX_DBLOCKS(sb) ||
		sb->sb_dblocks < XFS_MIN_DBLOCKS(sb))
		return(XR_BAD_FS_SIZE_DATA);

	if (sb->sb_agblklog != (uint8_t)libxfs_log2_roundup(sb->sb_agblocks))
		return(XR_BAD_FS_SIZE_DATA);

	if (sb->sb_inodesize < XFS_DINODE_MIN_SIZE                     ||
	    sb->sb_inodesize > XFS_DINODE_MAX_SIZE                     ||
	    sb->sb_inodelog < XFS_DINODE_MIN_LOG                       ||
	    sb->sb_inodelog > XFS_DINODE_MAX_LOG                       ||
	    sb->sb_inodesize != (1 << sb->sb_inodelog)                 ||
	    sb->sb_logsunit > XLOG_MAX_RECORD_BSIZE                    ||
	    sb->sb_inopblock != howmany(sb->sb_blocksize, sb->sb_inodesize) ||
	    (sb->sb_blocklog - sb->sb_inodelog != sb->sb_inopblog))
		return XR_BAD_INO_SIZE_DATA;

	if (xfs_sb_version_hassector(sb))  {

		/* check to make sure log sector is legal 2^N, 9 <= N <= 15 */

		if (sb->sb_logsectsize == 0)
			return(XR_BAD_SECT_SIZE_DATA);

		bsize = 1;

		for (i = 0; bsize < sb->sb_logsectsize &&
			i < sizeof(sb->sb_logsectsize) * NBBY; i++)  {
			bsize <<= 1;
		}

		if (i < XFS_MIN_SECTORSIZE_LOG || i > XFS_MAX_SECTORSIZE_LOG)
			return(XR_BAD_SECT_SIZE_DATA);

		/* check sb log sectorsize field against sb log sectlog field */

		if (i != sb->sb_logsectlog)
			return(XR_BAD_SECT_SIZE_DATA);
	}

	/*
	 * real-time extent size is always set
	 */
	if (sb->sb_rextsize * sb->sb_blocksize > XFS_MAX_RTEXTSIZE)
		return(XR_BAD_RT_GEO_DATA);

	if (sb->sb_rextsize * sb->sb_blocksize < XFS_MIN_RTEXTSIZE)
			return(XR_BAD_RT_GEO_DATA);

	if (sb->sb_rblocks == 0)  {
		if (sb->sb_rextents != 0)
			return(XR_BAD_RT_GEO_DATA);

		if (sb->sb_rbmblocks != 0)
			return(XR_BAD_RT_GEO_DATA);

		if (sb->sb_rextslog != 0)
			return(XR_BAD_RT_GEO_DATA);

		if (sb->sb_frextents != 0)
			return(XR_BAD_RT_GEO_DATA);
	} else  {
		/*
		 * if we have a real-time partition, sanity-check geometry
		 */
		if (sb->sb_rblocks / sb->sb_rextsize != sb->sb_rextents)
			return(XR_BAD_RT_GEO_DATA);

		if (sb->sb_rextslog !=
				libxfs_highbit32((unsigned int)sb->sb_rextents))
			return(XR_BAD_RT_GEO_DATA);

		if (sb->sb_rbmblocks != (xfs_extlen_t) howmany(sb->sb_rextents,
						NBBY * sb->sb_blocksize))
			return(XR_BAD_RT_GEO_DATA);
	}

	/*
	 * verify correctness of inode alignment if it's there
	 */
	if (!sb_validate_ino_align(sb))
		return(XR_BAD_INO_ALIGN);

	/*
	 * verify max. % of inodes (sb_imax_pct)
	 */
	if (sb->sb_imax_pct > 100)
		return(XR_BAD_INO_MAX_PCT);

	/*
	 * verify stripe alignment fields if present
	 */
	if (xfs_sb_version_hasdalign(sb)) {
		if ((!sb->sb_unit && sb->sb_width) ||
		    (sb->sb_unit && sb->sb_agblocks % sb->sb_unit))
			return(XR_BAD_SB_UNIT);
		if ((sb->sb_unit && !sb->sb_width) ||
		    (sb->sb_width && sb->sb_unit && sb->sb_width % sb->sb_unit))
			return(XR_BAD_SB_WIDTH);
	} else if (sb->sb_unit || sb->sb_width)
		return XR_BAD_SB_WIDTH;

	/* Directory block log */
	if (sb->sb_blocklog + sb->sb_dirblklog > XFS_MAX_BLOCKSIZE_LOG)
		return XR_BAD_DIR_SIZE_DATA;

	return(XR_OK);
}

void
write_primary_sb(xfs_sb_t *sbp, int size)
{
	xfs_dsb_t	*buf;

	if (no_modify)
		return;

	buf = memalign(libxfs_device_alignment(), size);
	if (buf == NULL) {
		do_error(_("failed to memalign superblock buffer\n"));
		return;
	}
	memset(buf, 0, size);

	if (lseek(x.dfd, 0LL, SEEK_SET) != 0LL) {
		free(buf);
		do_error(_("couldn't seek to offset 0 in filesystem\n"));
	}

	libxfs_sb_to_disk(buf, sbp);

	if (xfs_sb_version_hascrc(sbp))
		xfs_update_cksum((char *)buf, size, XFS_SB_CRC_OFF);

	if (write(x.dfd, buf, size) != size) {
		free(buf);
		do_error(_("primary superblock write failed!\n"));
	}

	free(buf);
}

/*
 * get a possible superblock -- checks for internal consistency
 */
int
get_sb(xfs_sb_t *sbp, xfs_off_t off, int size, xfs_agnumber_t agno)
{
	int error, rval;
	xfs_dsb_t *buf;

	buf = memalign(libxfs_device_alignment(), size);
	if (buf == NULL) {
		do_error(
	_("error reading superblock %u -- failed to memalign buffer\n"),
			agno);
		exit(1);
	}
	memset(buf, 0, size);
	memset(sbp, 0, sizeof(*sbp));

	/* try and read it first */

	if (lseek(x.dfd, off, SEEK_SET) != off)  {
		do_warn(
	_("error reading superblock %u -- seek to offset %" PRId64 " failed\n"),
			agno, off);
		free(buf);
		return(XR_EOF);
	}

	if ((rval = read(x.dfd, buf, size)) != size)  {
		error = errno;
		do_warn(
	_("superblock read failed, offset %" PRId64 ", size %d, ag %u, rval %d\n"),
			off, size, agno, rval);
		do_error("%s\n", strerror(error));
	}
	libxfs_sb_from_disk(sbp, buf);

	rval = verify_sb((char *)buf, sbp, agno == 0);
	free(buf);
	return rval;
}

/* returns element on list with highest reference count */
static fs_geo_list_t *
get_best_geo(fs_geo_list_t *list)
{
	int cnt = 0;
	fs_geo_list_t *current, *rval = NULL;

	current = list;

	while (current != NULL)  {
		if (current->refs > cnt)  {
			rval = current;
			cnt = current->refs;
		}
		current = current->next;
	}

	return(rval);
}

/* adds geometry info to linked list.  returns (sometimes new) head of list */
static fs_geo_list_t *
add_geo(fs_geo_list_t *list, fs_geometry_t *geo_p, int index)
{
	fs_geo_list_t	*current = list;

	while (current != NULL)  {
		if (memcmp(geo_p, &current->geo, sizeof(fs_geometry_t)) == 0)  {
			current->refs++;
			return(list);
		}

		current = current->next;
	}

	if ((current = malloc(sizeof(fs_geo_list_t))) == NULL) {
		do_error(_("couldn't malloc geometry structure\n"));
		exit(1);
	}

	current->geo = *geo_p;
	current->refs = 1;
	current->next = list;
	current->index = index;

	return(current);
}

static void
free_geo(fs_geo_list_t *list)
{
	fs_geo_list_t	*next;
	fs_geo_list_t	*current;

	for (current = list; current != NULL; current = next)  {
		next = current->next;
		free(current);
	}
}

void
get_sb_geometry(fs_geometry_t *geo, xfs_sb_t *sbp)
{
	memset(geo, 0, sizeof(fs_geometry_t));

	/*
	 * blindly set fields that we know are always good
	 */
	geo->sb_blocksize = sbp->sb_blocksize;
	geo->sb_dblocks = sbp->sb_dblocks;
	geo->sb_rblocks = sbp->sb_rblocks;
	geo->sb_rextents = sbp->sb_rextents;
	geo->sb_logstart = sbp->sb_logstart;
	geo->sb_rextsize = sbp->sb_rextsize;
	geo->sb_agblocks = sbp->sb_agblocks;
	geo->sb_agcount = sbp->sb_agcount;
	geo->sb_rbmblocks = sbp->sb_rbmblocks;
	geo->sb_logblocks = sbp->sb_logblocks;
	geo->sb_sectsize = sbp->sb_sectsize;
	geo->sb_inodesize = sbp->sb_inodesize;

	if (xfs_sb_version_hasalign(sbp))
		geo->sb_ialignbit = 1;

	if (xfs_sb_version_hasdalign(sbp))
		geo->sb_salignbit = 1;

	if (xfs_sb_version_hasextflgbit(sbp))
		geo->sb_extflgbit = 1;

	geo->sb_fully_zeroed = 1;
}

/*
 * the way to verify that a primary sb is consistent with the
 * filesystem is find the secondaries given the info in the
 * primary and compare the geometries in the secondaries against
 * the geometry indicated by the primary.
 *
 * returns 0 if ok, else error code (XR_EOF, XR_INSUFF_SEC_SB, etc).
 */
int
verify_set_primary_sb(xfs_sb_t		*rsb,
			int		sb_index,
			int		*sb_modified)
{
	xfs_off_t	off;
	fs_geometry_t	geo;
	xfs_sb_t	*sb;
	fs_geo_list_t	*list;
	fs_geo_list_t	*current;
	xfs_agnumber_t	agno;
	int		num_sbs;
	int		size;
	int		num_ok;
	int		retval;

	/*
	 * We haven't been able to validate the sector size yet properly
	 * (e.g. in the case of repairing an image in a file), so we need to
	 * take into account sector mismatches and so use the maximum possible
	 * sector size rather than the sector size in @rsb.
	 */
	size = NUM_AGH_SECTS * (1 << (XFS_MAX_SECTORSIZE_LOG));
	list = NULL;
	num_ok = 0;
	*sb_modified = 0;
	num_sbs = rsb->sb_agcount;

	sb = (xfs_sb_t *) alloc_ag_buf(size);

	/*
	 * put the primary sb geometry info onto the geometry list
	 */
	get_sb_geometry(&geo, rsb);
	list = add_geo(list, &geo, sb_index);

	/*
	 * scan the secondaries and check them off as we get them so we only
	 * process each one once
	 */
	for (agno = 1; agno < rsb->sb_agcount; agno++) {
		off = (xfs_off_t)agno * rsb->sb_agblocks << rsb->sb_blocklog;

		retval = get_sb(sb, off, size, agno);
		if (retval == XR_EOF)
			goto out_free_list;

		if (retval == XR_OK) {
			/*
			 * save away geometry info. don't bother checking the
			 * sb against the agi/agf as the odds of the sb being
			 * corrupted in a way that it is internally consistent
			 * but not consistent with the rest of the filesystem is
			 * really really low.
			 */
			get_sb_geometry(&geo, sb);
			list = add_geo(list, &geo, agno);
			num_ok++;
		}
	}

	/*
	 * see if we have enough superblocks to bother with
	 */
	retval = 0;
	if (num_ok < num_sbs / 2) {
		retval = XR_INSUFF_SEC_SB;
		goto out_free_list;
	}

	current = get_best_geo(list);

	/*
	 * check that enough sbs agree that we're willing to
	 * go with this geometry.  if not, print out the
	 * geometry and a message about the force option.
	 */
	switch (num_sbs)  {
	case 2:
		/*
		 * If we only have two allocation groups, and the superblock
		 * in the second allocation group differs from the primary
		 * superblock we can't verify the geometry information.
		 * Warn the user about this situation and get out unless
		 * explicitly overridden.
		 */
		if (current->refs != 2)  {
			if (!force_geo)  {
				do_warn(
	_("Only two AGs detected and they do not match - "
	  "cannot validate filesystem geometry.\n"
	  "Use the -o force_geometry option to proceed.\n"));
				exit(1);
			}
		}
		goto out_free_list;
	case 1:
		/*
		 * If we only have a single allocation group there is no
		 * secondary superblock that we can use to verify the geometry
		 * information.  Warn the user about this situation and get
		 * out unless explicitly overridden.
		 */
		if (!force_geo)  {
			do_warn(
	_("Only one AG detected - "
	  "cannot validate filesystem geometry.\n"
	  "Use the -o force_geometry option to proceed.\n"));
			exit(1);
		}
		goto out_free_list;
	default:
		/*
		 * at least half of the probed superblocks have
		 * to agree.  if they don't, this fs is probably
		 * too far gone anyway considering the fact that
		 * XFS normally doesn't alter the secondary superblocks.
		 */
		if (current->refs < num_sbs / 2)  {
			do_warn(
		_("Not enough matching superblocks - cannot proceed.\n"));
			exit(1);
		}
	}

	/*
	 * set the geometry into primary superblock if necessary.
	 */

	if (current->index != sb_index)  {
		*sb_modified = 1;
		off = (xfs_off_t)current->index * current->geo.sb_agblocks
			* current->geo.sb_blocksize;
		if (get_sb(sb, off, current->geo.sb_sectsize,
				current->index) != XR_OK)
			do_error(_("could not read superblock\n"));

		copy_sb(sb, rsb);

		/*
		 * turn off inprogress bit since this is the primary.
		 * also save away values that we need to ensure are
		 * consistent in the other secondaries.
		 */
		rsb->sb_inprogress = 0;
		sb_inoalignmt = sb->sb_inoalignmt;
		sb_unit = sb->sb_unit;
		sb_width = sb->sb_width;
	}

out_free_list:
	free_geo(list);
	free(sb);
	return retval;
}
