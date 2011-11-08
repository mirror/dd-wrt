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

#include <libxfs.h>
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dir.h"
#include "dir2.h"
#include "dinode.h"
#include "scan.h"
#include "versions.h"
#include "attr_repair.h"
#include "bmap.h"
#include "threads.h"

/*
 * inode clearing routines
 */

/*
 * return the offset into the inode where the attribute fork starts
 */
/* ARGSUSED */
int
calc_attr_offset(xfs_mount_t *mp, xfs_dinode_t *dino)
{
	int	offset = (__psint_t)XFS_DFORK_DPTR(dino) - (__psint_t)dino;
	xfs_bmdr_block_t        *dfp;

	/*
	 * don't worry about alignment when calculating offset
	 * because the data fork is already 8-byte aligned
	 */
	switch (dino->di_format)  {
	case XFS_DINODE_FMT_DEV:
		offset += sizeof(xfs_dev_t);
		break;
	case XFS_DINODE_FMT_LOCAL:
		offset += be64_to_cpu(dino->di_size);
		break;
	case XFS_DINODE_FMT_EXTENTS:
		offset += be32_to_cpu(dino->di_nextents) *
						sizeof(xfs_bmbt_rec_t);
		break;
	case XFS_DINODE_FMT_BTREE:
		dfp = (xfs_bmdr_block_t *)XFS_DFORK_DPTR(dino);
		offset += be16_to_cpu(dfp->bb_numrecs) *
						sizeof(xfs_bmbt_rec_t);
		break;
	default:
		do_error(_("Unknown inode format.\n"));
		abort();
		break;
	}

	return(offset);
}

/* ARGSUSED */
int
clear_dinode_attr(xfs_mount_t *mp, xfs_dinode_t *dino, xfs_ino_t ino_num)
{
	ASSERT(dino->di_forkoff != 0);

	if (!no_modify)
		fprintf(stderr,
_("clearing inode %" PRIu64 " attributes\n"), ino_num);
	else
		fprintf(stderr,
_("would have cleared inode %" PRIu64 " attributes\n"), ino_num);

	if (be16_to_cpu(dino->di_anextents) != 0)  {
		if (no_modify)
			return(1);
		dino->di_anextents = cpu_to_be16(0);
	}

	if (dino->di_aformat != XFS_DINODE_FMT_EXTENTS)  {
		if (no_modify)
			return(1);
		dino->di_aformat = XFS_DINODE_FMT_EXTENTS;
	}

	/* get rid of the fork by clearing forkoff */

	/* Originally, when the attr repair code was added, the fork was cleared
	 * by turning it into shortform status.  This meant clearing the
	 * hdr.totsize/count fields and also changing aformat to LOCAL
	 * (vs EXTENTS).  Over various fixes, the aformat and forkoff have
	 * been updated to not show an attribute fork at all, however.
	 * It could be possible that resetting totsize/count are not needed,
	 * but just to be safe, leave it in for now.
	 */

	if (!no_modify) {
		xfs_attr_shortform_t *asf = (xfs_attr_shortform_t *)
				XFS_DFORK_APTR(dino);
		asf->hdr.totsize = cpu_to_be16(sizeof(xfs_attr_sf_hdr_t));
		asf->hdr.count = 0;
		dino->di_forkoff = 0;  /* got to do this after asf is set */
	}

	/*
	 * always returns 1 since the fork gets zapped
	 */
	return(1);
}

/* ARGSUSED */
int
clear_dinode_core(xfs_dinode_t *dinoc, xfs_ino_t ino_num)
{
	int dirty = 0;

	if (be16_to_cpu(dinoc->di_magic) != XFS_DINODE_MAGIC)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_magic = cpu_to_be16(XFS_DINODE_MAGIC);
	}

	if (!XFS_DINODE_GOOD_VERSION(dinoc->di_version) ||
	    (!fs_inode_nlink && dinoc->di_version > 1))  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_version = (fs_inode_nlink) ? 2 : 1;
	}

	if (be16_to_cpu(dinoc->di_mode) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_mode = 0;
	}

	if (be16_to_cpu(dinoc->di_flags) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_flags = 0;
	}

	if (be32_to_cpu(dinoc->di_dmevmask) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_dmevmask = 0;
	}

	if (dinoc->di_forkoff != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_forkoff = 0;
	}

	if (dinoc->di_format != XFS_DINODE_FMT_EXTENTS)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_format = XFS_DINODE_FMT_EXTENTS;
	}

	if (dinoc->di_aformat != XFS_DINODE_FMT_EXTENTS)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_aformat = XFS_DINODE_FMT_EXTENTS;
	}

	if (be64_to_cpu(dinoc->di_size) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_size = 0;
	}

	if (be64_to_cpu(dinoc->di_nblocks) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_nblocks = 0;
	}

	if (be16_to_cpu(dinoc->di_onlink) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_onlink = 0;
	}

	if (be32_to_cpu(dinoc->di_nextents) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_nextents = 0;
	}

	if (be16_to_cpu(dinoc->di_anextents) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_anextents = 0;
	}

	if (dinoc->di_version > 1 &&
			be32_to_cpu(dinoc->di_nlink) != 0)  {
		dirty = 1;

		if (no_modify)
			return(1);

		dinoc->di_nlink = 0;
	}

	return(dirty);
}

/* ARGSUSED */
int
clear_dinode_unlinked(xfs_mount_t *mp, xfs_dinode_t *dino)
{

	if (be32_to_cpu(dino->di_next_unlinked) != NULLAGINO)  {
		if (!no_modify)
			dino->di_next_unlinked = cpu_to_be32(NULLAGINO);
		return(1);
	}

	return(0);
}

/*
 * this clears the unlinked list too so it should not be called
 * until after the agi unlinked lists are walked in phase 3.
 * returns > zero if the inode has been altered while being cleared
 */
int
clear_dinode(xfs_mount_t *mp, xfs_dinode_t *dino, xfs_ino_t ino_num)
{
	int dirty;

	dirty = clear_dinode_core(dino, ino_num);
	dirty += clear_dinode_unlinked(mp, dino);

	/* and clear the forks */

	if (dirty && !no_modify)
		memset(XFS_DFORK_DPTR(dino), 0, XFS_LITINO(mp));

	return(dirty);
}


/*
 * misc. inode-related utility routines
 */

/*
 * verify_ag_bno is heavily used. In the common case, it
 * performs just two number of compares
 * Returns 1 for bad ag/bno pair or 0 if it's valid.
 */
static __inline int
verify_ag_bno(xfs_sb_t *sbp,
		xfs_agnumber_t agno,
		xfs_agblock_t agbno)
{
	if (agno < (sbp->sb_agcount - 1)) 
		return (agbno >= sbp->sb_agblocks);
	if (agno == (sbp->sb_agcount - 1)) 
		return (agbno >= (sbp->sb_dblocks -
				((xfs_drfsbno_t)(sbp->sb_agcount - 1) *
				 sbp->sb_agblocks)));
	return 1;
}

/*
 * returns 0 if inode number is valid, 1 if bogus
 */
int
verify_inum(xfs_mount_t		*mp,
		xfs_ino_t	ino)
{
	xfs_agnumber_t	agno;
	xfs_agino_t	agino;
	xfs_agblock_t	agbno;
	xfs_sb_t	*sbp = &mp->m_sb;;

	/* range check ag #, ag block.  range-checking offset is pointless */

	agno = XFS_INO_TO_AGNO(mp, ino);
	agino = XFS_INO_TO_AGINO(mp, ino);
	agbno = XFS_AGINO_TO_AGBNO(mp, agino);
	if (agbno == 0)
		return 1;

	if (ino == 0 || ino == NULLFSINO)
		return(1);

	if (ino != XFS_AGINO_TO_INO(mp, agno, agino))
		return(1);

	return verify_ag_bno(sbp, agno, agbno);
}

/*
 * have a separate routine to ensure that we don't accidentally
 * lose illegally set bits in the agino by turning it into an FSINO
 * to feed to the above routine
 */
int
verify_aginum(xfs_mount_t	*mp,
		xfs_agnumber_t	agno,
		xfs_agino_t	agino)
{
	xfs_agblock_t	agbno;
	xfs_sb_t	*sbp = &mp->m_sb;;

	/* range check ag #, ag block.  range-checking offset is pointless */

	if (agino == 0 || agino == NULLAGINO)
		return(1);

	/*
	 * agino's can't be too close to NULLAGINO because the min blocksize
	 * is 9 bits and at most 1 bit of that gets used for the inode offset
	 * so if the agino gets shifted by the # of offset bits and compared
	 * to the legal agbno values, a bogus agino will be too large.  there
	 * will be extra bits set at the top that shouldn't be set.
	 */
	agbno = XFS_AGINO_TO_AGBNO(mp, agino);
	if (agbno == 0)
		return 1;

	return verify_ag_bno(sbp, agno, agbno);
}

/*
 * return 1 if block number is good, 0 if out of range
 */
int
verify_dfsbno(xfs_mount_t	*mp,
		xfs_dfsbno_t	fsbno)
{
	xfs_agnumber_t	agno;
	xfs_agblock_t	agbno;
	xfs_sb_t	*sbp = &mp->m_sb;;

	/* range check ag #, ag block.  range-checking offset is pointless */

	agno = XFS_FSB_TO_AGNO(mp, fsbno);
	agbno = XFS_FSB_TO_AGBNO(mp, fsbno);

	return verify_ag_bno(sbp, agno, agbno) == 0;
}

#define XR_DFSBNORANGE_VALID	0
#define XR_DFSBNORANGE_BADSTART	1
#define XR_DFSBNORANGE_BADEND	2
#define XR_DFSBNORANGE_OVERFLOW	3

static __inline int
verify_dfsbno_range(xfs_mount_t	*mp,
		xfs_dfsbno_t	fsbno,
		xfs_dfilblks_t	count)
{
	xfs_agnumber_t	agno;
	xfs_agblock_t	agbno;
	xfs_sb_t	*sbp = &mp->m_sb;;

	/* the start and end blocks better be in the same allocation group */
	agno = XFS_FSB_TO_AGNO(mp, fsbno);
	if (agno != XFS_FSB_TO_AGNO(mp, fsbno + count - 1)) {
		return XR_DFSBNORANGE_OVERFLOW;
	}

	agbno = XFS_FSB_TO_AGBNO(mp, fsbno);
	if (verify_ag_bno(sbp, agno, agbno)) {
		return XR_DFSBNORANGE_BADSTART;
	}

	agbno = XFS_FSB_TO_AGBNO(mp, fsbno + count - 1);
	if (verify_ag_bno(sbp, agno, agbno)) {
		return XR_DFSBNORANGE_BADEND;
	}

	return (XR_DFSBNORANGE_VALID);
}

int
verify_agbno(xfs_mount_t	*mp,
		xfs_agnumber_t	agno,
		xfs_agblock_t	agbno)
{
	xfs_sb_t	*sbp = &mp->m_sb;;

	/* range check ag #, ag block.  range-checking offset is pointless */
	return verify_ag_bno(sbp, agno, agbno) == 0;
}

/*
 * return address of block fblock if it's within the range described
 * by the extent list.  Otherwise, returns a null address.
 */
/* ARGSUSED */
xfs_dfsbno_t
get_bmbt_reclist(
	xfs_mount_t		*mp,
	xfs_bmbt_rec_t		*rp,
	int			numrecs,
	xfs_dfiloff_t		fblock)
{
	int			i;
	xfs_bmbt_irec_t 	irec;

	for (i = 0; i < numrecs; i++) {
		libxfs_bmbt_disk_get_all(rp + i, &irec);
		if (irec.br_startoff >= fblock &&
				irec.br_startoff + irec.br_blockcount < fblock)
			return (irec.br_startblock + fblock - irec.br_startoff);
	}
	return(NULLDFSBNO);
}


static int
process_rt_rec(
	xfs_mount_t		*mp,
	xfs_bmbt_irec_t 	*irec,
	xfs_ino_t		ino,
	xfs_drfsbno_t		*tot,
	int			check_dups)
{
	xfs_dfsbno_t		b;
	xfs_drtbno_t		ext;
	int			state;
	int			pwe;		/* partially-written extent */

	/*
	 * check numeric validity of the extent
	 */
	if (irec->br_startblock >= mp->m_sb.sb_rblocks) {
		do_warn(
_("inode %" PRIu64 " - bad rt extent start block number %" PRIu64 ", offset %" PRIu64 "\n"),
			ino,
			irec->br_startblock,
			irec->br_startoff);
		return 1;
	}
	if (irec->br_startblock + irec->br_blockcount - 1 >= mp->m_sb.sb_rblocks) {
		do_warn(
_("inode %" PRIu64 " - bad rt extent last block number %" PRIu64 ", offset %" PRIu64 "\n"),
			ino,
			irec->br_startblock + irec->br_blockcount - 1,
			irec->br_startoff);
		return 1;
	}
	if (irec->br_startblock + irec->br_blockcount - 1 < irec->br_startblock) {
		do_warn(
_("inode %" PRIu64 " - bad rt extent overflows - start %" PRIu64 ", "
  "end %" PRIu64 ", offset %" PRIu64 "\n"),
			ino,
			irec->br_startblock,
			irec->br_startblock + irec->br_blockcount - 1,
			irec->br_startoff);
		return 1;
	}

	/*
	 * verify that the blocks listed in the record
	 * are multiples of an extent
	 */
	if (xfs_sb_version_hasextflgbit(&mp->m_sb) == 0 &&
			(irec->br_startblock % mp->m_sb.sb_rextsize != 0 ||
			 irec->br_blockcount % mp->m_sb.sb_rextsize != 0)) {
		do_warn(
_("malformed rt inode extent [%" PRIu64 " %" PRIu64 "] (fs rtext size = %u)\n"),
			irec->br_startblock,
			irec->br_blockcount,
			mp->m_sb.sb_rextsize);
		return 1;
	}

	/*
	 * set the appropriate number of extents
	 * this iterates block by block, this can be optimised using extents
	 */
	for (b = irec->br_startblock; b < irec->br_startblock +
			irec->br_blockcount; b += mp->m_sb.sb_rextsize)  {
		ext = (xfs_drtbno_t) b / mp->m_sb.sb_rextsize;
		pwe = xfs_sb_version_hasextflgbit(&mp->m_sb) &&
				irec->br_state == XFS_EXT_UNWRITTEN &&
				(b % mp->m_sb.sb_rextsize != 0);

		if (check_dups == 1)  {
			if (search_rt_dup_extent(mp, ext) && !pwe)  {
				do_warn(
_("data fork in rt ino %" PRIu64 " claims dup rt extent,"
  "off - %" PRIu64 ", start - %" PRIu64 ", count %" PRIu64 "\n"),
					ino,
					irec->br_startoff,
					irec->br_startblock,
					irec->br_blockcount);
				return 1;
			}
			continue;
		}

		state = get_rtbmap(ext);
		switch (state)  {
		case XR_E_FREE:
		case XR_E_UNKNOWN:
			set_rtbmap(ext, XR_E_INUSE);
			break;
		case XR_E_BAD_STATE:
			do_error(
_("bad state in rt block map %" PRIu64 "\n"),
				ext);
		case XR_E_FS_MAP:
		case XR_E_INO:
		case XR_E_INUSE_FS:
			do_error(
_("data fork in rt inode %" PRIu64 " found metadata block %" PRIu64 " in rt bmap\n"),
				ino, ext);
		case XR_E_INUSE:
			if (pwe)
				break;
		case XR_E_MULT:
			set_rtbmap(ext, XR_E_MULT);
			do_warn(
_("data fork in rt inode %" PRIu64 " claims used rt block %" PRIu64 "\n"),
				ino, ext);
			return 1;
		case XR_E_FREE1:
		default:
			do_error(
_("illegal state %d in rt block map %" PRIu64 "\n"),
				state, b);
		}
	}

	/*
	 * bump up the block counter
	 */
	*tot += irec->br_blockcount;

	return 0;
}

/*
 * return 1 if inode should be cleared, 0 otherwise
 * if check_dups should be set to 1, that implies that
 * the primary purpose of this call is to see if the
 * file overlaps with any duplicate extents (in the
 * duplicate extent list).
 */
/* ARGSUSED */
int
process_bmbt_reclist_int(
	xfs_mount_t		*mp,
	xfs_bmbt_rec_t		*rp,
	int			numrecs,
	int			type,
	xfs_ino_t		ino,
	xfs_drfsbno_t		*tot,
	blkmap_t		**blkmapp,
	xfs_dfiloff_t		*first_key,
	xfs_dfiloff_t		*last_key,
	int			check_dups,
	int			whichfork)
{
	xfs_bmbt_irec_t		irec;
	xfs_dfilblks_t		cp = 0;		/* prev count */
	xfs_dfsbno_t		sp = 0;		/* prev start */
	xfs_dfiloff_t		op = 0;		/* prev offset */
	xfs_dfsbno_t		b;
	char			*ftype;
	char			*forkname;
	int			i;
	int			state;
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;
	xfs_agblock_t		ebno;
	xfs_extlen_t		blen;
	xfs_agnumber_t		locked_agno = -1;
	int			error = 1;

	if (whichfork == XFS_DATA_FORK)
		forkname = _("data");
	else
		forkname = _("attr");

	if (type == XR_INO_RTDATA)
		ftype = _("real-time");
	else
		ftype = _("regular");

	for (i = 0; i < numrecs; i++) {
		libxfs_bmbt_disk_get_all(rp + i, &irec);
		if (i == 0)
			*last_key = *first_key = irec.br_startoff;
		else
			*last_key = irec.br_startoff;
		if (i > 0 && op + cp > irec.br_startoff)  {
			do_warn(
_("bmap rec out of order, inode %" PRIu64" entry %d "
  "[o s c] [%" PRIu64 " %" PRIu64 " %" PRIu64 "], "
  "%d [%" PRIu64 " %" PRIu64 " %" PRIu64 "]\n"),
				ino, i, irec.br_startoff, irec.br_startblock,
				irec.br_blockcount, i - 1, op, sp, cp);
			goto done;
		}
		op = irec.br_startoff;
		cp = irec.br_blockcount;
		sp = irec.br_startblock;

		/*
		 * check numeric validity of the extent
		 */
		if (irec.br_blockcount == 0)  {
			do_warn(
_("zero length extent (off = %" PRIu64 ", fsbno = %" PRIu64 ") in ino %" PRIu64 "\n"),
				irec.br_startoff,
				irec.br_startblock,
				ino);
			goto done;
		}

		if (type == XR_INO_RTDATA && whichfork == XFS_DATA_FORK) {
			/*
			 * realtime bitmaps don't use AG locks, so returning
			 * immediately is fine for this code path.
			 */
			if (process_rt_rec(mp, &irec, ino, tot, check_dups))
				return 1;
			/*
			 * skip rest of loop processing since that'irec.br_startblock
			 * all for regular file forks and attr forks
			 */
			continue;
		}

		/*
		 * regular file data fork or attribute fork
		 */
		switch (verify_dfsbno_range(mp, irec.br_startblock,
						irec.br_blockcount)) {
			case XR_DFSBNORANGE_VALID:
				break;

			case XR_DFSBNORANGE_BADSTART:
				do_warn(
_("inode %" PRIu64 " - bad extent starting block number %" PRIu64 ", offset %" PRIu64 "\n"),
					ino,
					irec.br_startblock,
					irec.br_startoff);
				goto done;

			case XR_DFSBNORANGE_BADEND:
				do_warn(
_("inode %" PRIu64 " - bad extent last block number %" PRIu64 ", offset %" PRIu64 "\n"),
					ino,
					irec.br_startblock + irec.br_blockcount - 1,
					irec.br_startoff);
				goto done;

			case XR_DFSBNORANGE_OVERFLOW:
				do_warn(
_("inode %" PRIu64 " - bad extent overflows - start %" PRIu64 ", "
  "end %" PRIu64 ", offset %" PRIu64 "\n"),
					ino,
					irec.br_startblock,
					irec.br_startblock + irec.br_blockcount - 1,
					irec.br_startoff);
				goto done;
		}
		if (irec.br_startoff >= fs_max_file_offset)  {
			do_warn(
_("inode %" PRIu64 " - extent offset too large - start %" PRIu64 ", "
  "count %" PRIu64 ", offset %" PRIu64 "\n"),
				ino, irec.br_startblock, irec.br_blockcount,
				irec.br_startoff);
			goto done;
		}

		if (blkmapp && *blkmapp) {
			error = blkmap_set_ext(blkmapp, irec.br_startoff,
					irec.br_startblock, irec.br_blockcount);
			if (error) {
				/*
				 * we don't want to clear the inode due to an
				 * internal bmap tracking error, but if we've
				 * run out of memory then we simply can't
				 * validate that the filesystem is consistent.
				 * Hence just abort at this point with an ENOMEM
				 * error.
				 */
				do_abort(
_("Fatal error: inode %" PRIu64 " - blkmap_set_ext(): %s\n"
  "\t%s fork, off - %" PRIu64 ", start - %" PRIu64 ", cnt %" PRIu64 "\n"),
					ino, strerror(error), forkname,
					irec.br_startoff, irec.br_startblock,
					irec.br_blockcount);
			}
		}

		/*
		 * Profiling shows that the following loop takes the
		 * most time in all of xfs_repair.
		 */
		agno = XFS_FSB_TO_AGNO(mp, irec.br_startblock);
		agbno = XFS_FSB_TO_AGBNO(mp, irec.br_startblock);
		ebno = agbno + irec.br_blockcount;
		if (agno != locked_agno) {
			if (locked_agno != -1)
				pthread_mutex_unlock(&ag_locks[locked_agno]);
			pthread_mutex_lock(&ag_locks[agno]);
			locked_agno = agno;
		}

		if (check_dups) {
			/*
			 * if we're just checking the bmap for dups,
			 * return if we find one, otherwise, continue
			 * checking each entry without setting the
			 * block bitmap
			 */
			if (search_dup_extent(agno, agbno, ebno)) {
				do_warn(
_("%s fork in ino %" PRIu64 " claims dup extent, "
  "off - %" PRIu64 ", start - %" PRIu64 ", cnt %" PRIu64 "\n"),
					forkname, ino, irec.br_startoff,
					irec.br_startblock,
					irec.br_blockcount);
				goto done;
			}
			*tot += irec.br_blockcount;
			continue;
		}

		for (b = irec.br_startblock;
		     agbno < ebno;
		     b += blen, agbno += blen) {
			state = get_bmap_ext(agno, agbno, ebno, &blen);
			switch (state)  {
			case XR_E_FREE:
			case XR_E_FREE1:
				do_warn(
_("%s fork in ino %" PRIu64 " claims free block %" PRIu64 "\n"),
					forkname, ino, (__uint64_t) b);
				/* fall through ... */
			case XR_E_UNKNOWN:
				set_bmap_ext(agno, agbno, blen, XR_E_INUSE);
				break;

			case XR_E_BAD_STATE:
				do_error(_("bad state in block map %" PRIu64 "\n"), b);

			case XR_E_FS_MAP:
			case XR_E_INO:
			case XR_E_INUSE_FS:
				do_warn(
_("%s fork in inode %" PRIu64 " claims metadata block %" PRIu64 "\n"),
					forkname, ino, b);
				goto done;

			case XR_E_INUSE:
			case XR_E_MULT:
				set_bmap_ext(agno, agbno, blen, XR_E_MULT);
				do_warn(
_("%s fork in %s inode %" PRIu64 " claims used block %" PRIu64 "\n"),
					forkname, ftype, ino, b);
				goto done;

			default:
				do_error(
_("illegal state %d in block map %" PRIu64 "\n"),
					state, b);
			}
		}
		*tot += irec.br_blockcount;
	}
	error = 0;
done:
	if (locked_agno != -1)
		pthread_mutex_unlock(&ag_locks[locked_agno]);
	return error;
}

/*
 * return 1 if inode should be cleared, 0 otherwise, sets block bitmap
 * as a side-effect
 */
int
process_bmbt_reclist(
	xfs_mount_t		*mp,
	xfs_bmbt_rec_t		*rp,
	int			numrecs,
	int			type,
	xfs_ino_t		ino,
	xfs_drfsbno_t		*tot,
	blkmap_t		**blkmapp,
	xfs_dfiloff_t		*first_key,
	xfs_dfiloff_t		*last_key,
	int			whichfork)
{
	return process_bmbt_reclist_int(mp, rp, numrecs, type, ino, tot,
				blkmapp, first_key, last_key, 0, whichfork);
}

/*
 * return 1 if inode should be cleared, 0 otherwise, does not set
 * block bitmap
 */
int
scan_bmbt_reclist(
	xfs_mount_t		*mp,
	xfs_bmbt_rec_t		*rp,
	int			numrecs,
	int			type,
	xfs_ino_t		ino,
	xfs_drfsbno_t		*tot,
	int			whichfork)
{
	xfs_dfiloff_t		first_key = 0;
	xfs_dfiloff_t		last_key = 0;

	return process_bmbt_reclist_int(mp, rp, numrecs, type, ino, tot,
				NULL, &first_key, &last_key, 1, whichfork);
}

/*
 * these two are meant for routines that read and work with inodes
 * one at a time where the inodes may be in any order (like walking
 * the unlinked lists to look for inodes).  the caller is responsible
 * for writing/releasing the buffer.
 */
xfs_buf_t *
get_agino_buf(xfs_mount_t	 *mp,
		xfs_agnumber_t	agno,
		xfs_agino_t	agino,
		xfs_dinode_t	**dipp)
{
	ino_tree_node_t *irec;
	xfs_buf_t *bp;
	int size;

	if ((irec = find_inode_rec(mp, agno, agino)) == NULL)
		return(NULL);

	size = XFS_FSB_TO_BB(mp, MAX(1, XFS_INODES_PER_CHUNK/inodes_per_block));
	bp = libxfs_readbuf(mp->m_dev, XFS_AGB_TO_DADDR(mp, agno,
		XFS_AGINO_TO_AGBNO(mp, irec->ino_startnum)), size, 0);
	if (!bp) {
		do_warn(_("cannot read inode (%u/%u), disk block %" PRIu64 "\n"),
			agno, irec->ino_startnum,
			XFS_AGB_TO_DADDR(mp, agno,
				XFS_AGINO_TO_AGBNO(mp, irec->ino_startnum)));
		return(NULL);
	}

	*dipp = xfs_make_iptr(mp, bp, agino -
		XFS_OFFBNO_TO_AGINO(mp, XFS_AGINO_TO_AGBNO(mp,
						irec->ino_startnum),
		0));

	return(bp);
}

/*
 * these next routines return the filesystem blockno of the
 * block containing the block "bno" in the file whose bmap
 * tree (or extent list) is rooted by "rootblock".
 *
 * the next routines are utility routines for the third
 * routine, get_bmapi().
 *
 * NOTE: getfunc_extlist only used by dirv1 checking code
 */
xfs_dfsbno_t
getfunc_extlist(xfs_mount_t		*mp,
		xfs_ino_t		ino,
		xfs_dinode_t		*dip,
		xfs_dfiloff_t		bno,
		int			whichfork)
{
	xfs_bmbt_irec_t		irec;
	xfs_dfsbno_t		final_fsbno = NULLDFSBNO;
	xfs_bmbt_rec_t		*rootblock = (xfs_bmbt_rec_t *)
						XFS_DFORK_PTR(dip, whichfork);
	xfs_extnum_t		nextents = XFS_DFORK_NEXTENTS(dip, whichfork);
	int			i;

	for (i = 0; i < nextents; i++)  {
		libxfs_bmbt_disk_get_all(rootblock + i, &irec);
		if (irec.br_startoff <= bno &&
				bno < irec.br_startoff + irec.br_blockcount) {
			final_fsbno = bno - irec.br_startoff + irec.br_startblock;
			break;
		}
	}

	return(final_fsbno);
}

/*
 * NOTE: getfunc_btree only used by dirv1 checking code... 
 */
xfs_dfsbno_t
getfunc_btree(xfs_mount_t		*mp,
		xfs_ino_t		ino,
		xfs_dinode_t		*dip,
		xfs_dfiloff_t		bno,
		int			whichfork)
{
	int			i;
#ifdef DEBUG
	int			prev_level;
#endif
	int			found;
	int			numrecs;
	xfs_bmbt_rec_t		*rec;
	xfs_bmbt_irec_t		irec;
	xfs_bmbt_ptr_t		*pp;
	xfs_bmbt_key_t		*key;
	xfs_bmdr_key_t		*rkey;
	xfs_bmdr_ptr_t		*rp;
	xfs_dfsbno_t		fsbno;
	xfs_buf_t		*bp;
	xfs_dfsbno_t		final_fsbno = NULLDFSBNO;
	struct xfs_btree_block	*block;
	xfs_bmdr_block_t	*rootblock = (xfs_bmdr_block_t *)
						XFS_DFORK_PTR(dip, whichfork);

	ASSERT(rootblock->bb_level != 0);
	/*
	 * deal with root block, it's got a slightly different
	 * header structure than interior nodes.  We know that
	 * a btree should have at least 2 levels otherwise it
	 * would be an extent list.
	 */
	rkey = XFS_BMDR_KEY_ADDR(rootblock, 1);
	rp = XFS_BMDR_PTR_ADDR(rootblock, 1,
		xfs_bmdr_maxrecs(mp, XFS_DFORK_SIZE(dip, mp, whichfork), 1));
	found = -1;
	for (i = 0; i < be16_to_cpu(rootblock->bb_numrecs) - 1; i++) {
		if (be64_to_cpu(rkey[i].br_startoff) <= bno && 
				bno < be64_to_cpu(rkey[i + 1].br_startoff)) {
			found = i;
			break;
		}
	}
	if (i == be16_to_cpu(rootblock->bb_numrecs) - 1 && 
				bno >= be64_to_cpu(rkey[i].br_startoff))
		found = i;

	ASSERT(found != -1);

	fsbno = be64_to_cpu(rp[found]);

	ASSERT(verify_dfsbno(mp, fsbno));

	bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, fsbno),
				XFS_FSB_TO_BB(mp, 1), 0);
	if (!bp) {
		do_error(_("cannot read bmap block %" PRIu64 "\n"), fsbno);
		return(NULLDFSBNO);
	}
	block = XFS_BUF_TO_BLOCK(bp);
	numrecs = be16_to_cpu(block->bb_numrecs);

	/*
	 * ok, now traverse any interior btree nodes
	 */
#ifdef DEBUG
	prev_level = be16_to_cpu(block->bb_level);
#endif

	while (be16_to_cpu(block->bb_level) > 0)  {
#ifdef DEBUG
		ASSERT(be16_to_cpu(block->bb_level) < prev_level);

		prev_level = be16_to_cpu(block->bb_level);
#endif
		if (numrecs > mp->m_bmap_dmxr[1]) {
			do_warn(
_("# of bmap records in inode %" PRIu64 " exceeds max (%u, max - %u)\n"),
				ino, numrecs,
				mp->m_bmap_dmxr[1]);
			libxfs_putbuf(bp);
			return(NULLDFSBNO);
		}
		if (verbose && numrecs < mp->m_bmap_dmnr[1]) {
			do_warn(
_("- # of bmap records in inode %" PRIu64 " less than minimum (%u, min - %u), proceeding ...\n"),
				ino, numrecs, mp->m_bmap_dmnr[1]);
		}
		key = XFS_BMBT_KEY_ADDR(mp, block, 1);
		pp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[1]);
		for (found = -1, i = 0; i < numrecs - 1; i++) {
			if (be64_to_cpu(key[i].br_startoff) <= bno && bno < 
					be64_to_cpu(key[i + 1].br_startoff)) {
				found = i;
				break;
			}
		}
		if (i == numrecs - 1 && bno >= be64_to_cpu(key[i].br_startoff))
			found = i;

		ASSERT(found != -1);
		fsbno = be64_to_cpu(pp[found]);

		ASSERT(verify_dfsbno(mp, fsbno));

		/*
		 * release current btree block and read in the
		 * next btree block to be traversed
		 */
		libxfs_putbuf(bp);
		bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, fsbno),
					XFS_FSB_TO_BB(mp, 1), 0);
		if (!bp) {
			do_error(_("cannot read bmap block %" PRIu64 "\n"),
				fsbno);
			return(NULLDFSBNO);
		}
		block = XFS_BUF_TO_BLOCK(bp);
		numrecs = be16_to_cpu(block->bb_numrecs);
	}

	/*
	 * current block must be a leaf block
	 */
	ASSERT(be16_to_cpu(block->bb_level) == 0);
	if (numrecs > mp->m_bmap_dmxr[0]) {
		do_warn(
_("# of bmap records in inode %" PRIu64 " greater than maximum (%u, max - %u)\n"),
			ino, numrecs, mp->m_bmap_dmxr[0]);
		libxfs_putbuf(bp);
		return(NULLDFSBNO);
	}
	if (verbose && numrecs < mp->m_bmap_dmnr[0])
		do_warn(
_("- # of bmap records in inode %" PRIu64 " less than minimum (%u, min - %u), continuing...\n"),
			ino, numrecs, mp->m_bmap_dmnr[0]);

	rec = XFS_BMBT_REC_ADDR(mp, block, 1);
	for (i = 0; i < numrecs; i++)  {
		libxfs_bmbt_disk_get_all(rec + i, &irec);
		if (irec.br_startoff <= bno &&
				bno < irec.br_startoff + irec.br_blockcount) {
			final_fsbno = bno - irec.br_startoff +
							irec.br_startblock;
			break;
		}
	}
	libxfs_putbuf(bp);

	if (final_fsbno == NULLDFSBNO)
		do_warn(_("could not map block %" PRIu64 "\n"), bno);

	return(final_fsbno);
}

/*
 * this could be smarter.  maybe we should have an open inode
 * routine that would get the inode buffer and return back
 * an inode handle.  I'm betting for the moment that this
 * is used only by the directory and attribute checking code
 * and that the avl tree find and buffer cache search are
 * relatively cheap.  If they're too expensive, we'll just
 * have to fix this and add an inode handle to the da btree
 * cursor.
 *
 * caller is responsible for checking doubly referenced blocks
 * and references to holes
 *
 * NOTE: get_bmapi only used by dirv1 checking code
 */
xfs_dfsbno_t
get_bmapi(xfs_mount_t *mp, xfs_dinode_t *dino_p,
		xfs_ino_t ino_num, xfs_dfiloff_t bno, int whichfork)
{
	xfs_dfsbno_t		fsbno;

	switch (XFS_DFORK_FORMAT(dino_p, whichfork)) {
	case XFS_DINODE_FMT_EXTENTS:
		fsbno = getfunc_extlist(mp, ino_num, dino_p, bno, whichfork);
		break;
	case XFS_DINODE_FMT_BTREE:
		fsbno = getfunc_btree(mp, ino_num, dino_p, bno, whichfork);
		break;
	case XFS_DINODE_FMT_LOCAL:
		do_error(_("get_bmapi() called for local inode %" PRIu64 "\n"),
			ino_num);
		fsbno = NULLDFSBNO;
		break;
	default:
		/*
		 * shouldn't happen
		 */
		do_error(_("bad inode format for inode %" PRIu64 "\n"), ino_num);
		fsbno = NULLDFSBNO;
	}

	return(fsbno);
}

/*
 * higher level inode processing stuff starts here:
 * first, one utility routine for each type of inode
 */

/*
 * return 1 if inode should be cleared, 0 otherwise
 */
/* ARGSUSED */
int
process_btinode(
	xfs_mount_t		*mp,
	xfs_agnumber_t		agno,
	xfs_agino_t		ino,
	xfs_dinode_t		*dip,
	int			type,
	int			*dirty,
	xfs_drfsbno_t		*tot,
	__uint64_t		*nex,
	blkmap_t		**blkmapp,
	int			whichfork,
	int			check_dups)
{
	xfs_bmdr_block_t	*dib;
	xfs_dfiloff_t		last_key;
	xfs_dfiloff_t		first_key = 0;
	xfs_ino_t		lino;
	xfs_bmbt_ptr_t		*pp;
	xfs_bmbt_key_t		*pkey;
	char			*forkname;
	int			i;
	int			level;
	int			numrecs;
	bmap_cursor_t		cursor;

	dib = (xfs_bmdr_block_t *)XFS_DFORK_PTR(dip, whichfork);
	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	*tot = 0;
	*nex = 0;

	if (whichfork == XFS_DATA_FORK)
		forkname = _("data");
	else
		forkname = _("attr");

	level = be16_to_cpu(dib->bb_level);
	numrecs = be16_to_cpu(dib->bb_numrecs);

	if ((level == 0) || (level > XFS_BM_MAXLEVELS(mp, whichfork))) {
		/*
		 * XXX - if we were going to fix up the inode,
		 * we'd try to treat the fork as an interior
		 * node and see if we could get an accurate
		 * level value from one of the blocks pointed
		 * to by the pointers in the fork.  For now
		 * though, we just bail (and blow out the inode).
		 */
		do_warn(
_("bad level %d in inode %" PRIu64 " bmap btree root block\n"),
			level, XFS_AGINO_TO_INO(mp, agno, ino));
		return(1);
	}
	if (numrecs == 0) {
		do_warn(
_("bad numrecs 0 in inode %" PRIu64 " bmap btree root block\n"),
			XFS_AGINO_TO_INO(mp, agno, ino));
		return(1);
	}
	/*
	 * use bmdr/dfork_dsize since the root block is in the data fork
	 */
	if (XFS_BMDR_SPACE_CALC(numrecs) > XFS_DFORK_SIZE(dip, mp, whichfork)) {
		do_warn(
	_("indicated size of %s btree root (%d bytes) greater than space in "
	  "inode %" PRIu64 " %s fork\n"),
			forkname, XFS_BMDR_SPACE_CALC(numrecs), lino, forkname);
		return(1);
	}

	init_bm_cursor(&cursor, level + 1);

	pp = XFS_BMDR_PTR_ADDR(dib, 1,
		xfs_bmdr_maxrecs(mp, XFS_DFORK_SIZE(dip, mp, whichfork), 0));
	pkey = XFS_BMDR_KEY_ADDR(dib, 1);
	last_key = NULLDFILOFF;

	for (i = 0; i < numrecs; i++)  {
		/*
		 * XXX - if we were going to do more to fix up the inode
		 * btree, we'd do it right here.  For now, if there's a
		 * problem, we'll bail out and presumably clear the inode.
		 */
		if (!verify_dfsbno(mp, be64_to_cpu(pp[i])))  {
			do_warn(_("bad bmap btree ptr 0x%llx in ino %" PRIu64 "\n"),
			       (unsigned long long) be64_to_cpu(pp[i]), lino);
			return(1);
		}

		if (scan_lbtree(be64_to_cpu(pp[i]), level, scanfunc_bmap, type, 
				whichfork, lino, tot, nex, blkmapp, &cursor,
				1, check_dups))
			return(1);
		/*
		 * fix key (offset) mismatches between the keys in root
		 * block records and the first key of each child block.
		 * fixes cases where entries have been shifted between
		 * blocks but the parent hasn't been updated
		 */
		if (!check_dups && cursor.level[level-1].first_key !=
					be64_to_cpu(pkey[i].br_startoff))  {
			if (!no_modify)  {
				do_warn(
	_("correcting key in bmbt root (was %llu, now %" PRIu64") in inode "
	  "%" PRIu64" %s fork\n"),
				       (unsigned long long)
					       be64_to_cpu(pkey[i].br_startoff),
					cursor.level[level-1].first_key,
					XFS_AGINO_TO_INO(mp, agno, ino),
					forkname);
				*dirty = 1;
				pkey[i].br_startoff = cpu_to_be64(
					cursor.level[level-1].first_key);
			} else  {
				do_warn(
	_("bad key in bmbt root (is %llu, would reset to %" PRIu64 ") in inode "
	  "%" PRIu64 " %s fork\n"),
				       (unsigned long long)
					       be64_to_cpu(pkey[i].br_startoff),
					cursor.level[level-1].first_key,
					XFS_AGINO_TO_INO(mp, agno, ino),
					forkname);
			}
		}
		/*
		 * make sure that keys are in ascending order.  blow out
		 * inode if the ordering doesn't hold
		 */
		if (check_dups == 0)  {
			if (last_key != NULLDFILOFF && last_key >=
			    cursor.level[level-1].first_key)  {
				do_warn(
	_("out of order bmbt root key %" PRIu64 " in inode %" PRIu64 " %s fork\n"),
					first_key,
					XFS_AGINO_TO_INO(mp, agno, ino),
					forkname);
				return(1);
			}
			last_key = cursor.level[level-1].first_key;
		}
	}
	/*
	 * Ideally if all the extents are ok (perhaps after further
	 * checks below?) we'd just move this back into extents format.
	 * But for now clear it, as the kernel will choke on this
	 */
	if (*nex <= XFS_DFORK_SIZE(dip, mp, whichfork) /
			sizeof(xfs_bmbt_rec_t)) {
		do_warn(
	_("extent count for ino %" PRIu64 " %s fork too low (%" PRIu64 ") for file format\n"),
				lino, forkname, *nex);
		return(1);
	}
	/*
	 * Check that the last child block's forward sibling pointer
	 * is NULL.
	 */
	if (check_dups == 0 &&
		cursor.level[0].right_fsbno != NULLDFSBNO)  {
		do_warn(
	_("bad fwd (right) sibling pointer (saw %" PRIu64 " should be NULLDFSBNO)\n"),
			cursor.level[0].right_fsbno);
		do_warn(
	_("\tin inode %" PRIu64 " (%s fork) bmap btree block %" PRIu64 "\n"),
			XFS_AGINO_TO_INO(mp, agno, ino), forkname,
			cursor.level[0].fsbno);
		return(1);
	}

	return(0);
}

/*
 * return 1 if inode should be cleared, 0 otherwise
 */
/* ARGSUSED */
int
process_exinode(
	xfs_mount_t		*mp,
	xfs_agnumber_t		agno,
	xfs_agino_t		ino,
	xfs_dinode_t		*dip,
	int			type,
	int			*dirty,
	xfs_drfsbno_t		*tot,
	__uint64_t		*nex,
	blkmap_t		**blkmapp,
	int			whichfork,
	int			check_dups)
{
	xfs_ino_t		lino;
	xfs_bmbt_rec_t		*rp;
	xfs_dfiloff_t		first_key;
	xfs_dfiloff_t		last_key;

	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	rp = (xfs_bmbt_rec_t *)XFS_DFORK_PTR(dip, whichfork);
	*tot = 0;
	*nex = XFS_DFORK_NEXTENTS(dip, whichfork);
	/*
	 * XXX - if we were going to fix up the btree record,
	 * we'd do it right here.  For now, if there's a problem,
	 * we'll bail out and presumably clear the inode.
	 */
	if (check_dups == 0)
		return(process_bmbt_reclist(mp, rp, *nex, type, lino,
					tot, blkmapp, &first_key, &last_key,
					whichfork));
	else
		return(scan_bmbt_reclist(mp, rp, *nex, type, lino, tot,
					whichfork));
}

/*
 * return 1 if inode should be cleared, 0 otherwise
 */
static int
process_lclinode(
	xfs_mount_t		*mp,
	xfs_agnumber_t		agno,
	xfs_agino_t		ino,
	xfs_dinode_t		*dip,
	int			whichfork)
{
	xfs_attr_shortform_t	*asf;
	xfs_ino_t		lino;

	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	if (whichfork == XFS_DATA_FORK && be64_to_cpu(dip->di_size) >
						XFS_DFORK_DSIZE(dip, mp)) {
		do_warn(
	_("local inode %" PRIu64 " data fork is too large (size = %lld, max = %d)\n"),
		       lino, (unsigned long long) be64_to_cpu(dip->di_size),
			XFS_DFORK_DSIZE(dip, mp));
		return(1);
	} else if (whichfork == XFS_ATTR_FORK) {
		asf = (xfs_attr_shortform_t *)XFS_DFORK_APTR(dip);
		if (be16_to_cpu(asf->hdr.totsize) > XFS_DFORK_ASIZE(dip, mp)) {
			do_warn(
	_("local inode %" PRIu64 " attr fork too large (size %d, max = %d)\n"),
				lino, be16_to_cpu(asf->hdr.totsize),
				XFS_DFORK_ASIZE(dip, mp));
			return(1);
		}
		if (be16_to_cpu(asf->hdr.totsize) < sizeof(xfs_attr_sf_hdr_t)) {
			do_warn(
	_("local inode %" PRIu64 " attr too small (size = %d, min size = %zd)\n"),
				lino, be16_to_cpu(asf->hdr.totsize),
				sizeof(xfs_attr_sf_hdr_t));
			return(1);
		}
	}

	return(0);
}

int
process_symlink_extlist(xfs_mount_t *mp, xfs_ino_t lino, xfs_dinode_t *dino)
{
	xfs_dfiloff_t		expected_offset;
	xfs_bmbt_rec_t		*rp;
	xfs_bmbt_irec_t		irec;
	int			numrecs;
	int			i;
	int			max_blocks;

	if (be64_to_cpu(dino->di_size) <= XFS_DFORK_DSIZE(dino, mp)) {
		if (dino->di_format == XFS_DINODE_FMT_LOCAL)  
			return 0;
		do_warn(
_("mismatch between format (%d) and size (%" PRId64 ") in symlink ino %" PRIu64 "\n"),
			dino->di_format,
			(__int64_t)be64_to_cpu(dino->di_size), lino);
		return 1;
	}
	if (dino->di_format == XFS_DINODE_FMT_LOCAL) {
		do_warn(
_("mismatch between format (%d) and size (%" PRId64 ") in symlink inode %" PRIu64 "n"),
			dino->di_format,
			(__int64_t)be64_to_cpu(dino->di_size), lino);
		return 1;
	}

	rp = (xfs_bmbt_rec_t *)XFS_DFORK_DPTR(dino);
	numrecs = be32_to_cpu(dino->di_nextents);

	/*
	 * the max # of extents in a symlink inode is equal to the
	 * number of max # of blocks required to store the symlink
	 */
	if (numrecs > max_symlink_blocks)  {
		do_warn(
_("bad number of extents (%d) in symlink %" PRIu64 " data fork\n"),
			numrecs, lino);
		return(1);
	}

	max_blocks = max_symlink_blocks;
	expected_offset = 0;

	for (i = 0; i < numrecs; i++)  {
		libxfs_bmbt_disk_get_all(rp + i, &irec);

		if (irec.br_startoff != expected_offset)  {
			do_warn(
_("bad extent #%d offset (%" PRIu64 ") in symlink %" PRIu64 " data fork\n"),
				i, irec.br_startoff, lino);
			return(1);
		}
		if (irec.br_blockcount == 0 || irec.br_blockcount > max_blocks) {
			do_warn(
_("bad extent #%d count (%" PRIu64 ") in symlink %" PRIu64 " data fork\n"),
				i, irec.br_blockcount, lino);
			return(1);
		}

		max_blocks -= irec.br_blockcount;
		expected_offset += irec.br_blockcount;
	}

	return(0);
}

/*
 * takes a name and length and returns 1 if the name contains
 * a \0, returns 0 otherwise
 */
int
null_check(char *name, int length)
{
	int i;

	ASSERT(length < MAXPATHLEN);

	for (i = 0; i < length; i++, name++)  {
		if (*name == '\0')
			return(1);
	}

	return(0);
}

/*
 * like usual, returns 0 if everything's ok and 1 if something's
 * bogus
 */
int
process_symlink(
	xfs_mount_t	*mp,
	xfs_ino_t	lino,
	xfs_dinode_t	*dino,
	blkmap_t 	*blkmap)
{
	xfs_dfsbno_t		fsbno;
	xfs_buf_t		*bp = NULL;
	char			*symlink, *cptr, *buf_data;
	int			i, size, amountdone;
	char			data[MAXPATHLEN];

	/*
	 * check size against kernel symlink limits.  we know
	 * size is consistent with inode storage format -- e.g.
	 * the inode is structurally ok so we don't have to check
	 * for that
	 */
	if (be64_to_cpu(dino->di_size) >= MAXPATHLEN)  {
	       do_warn(_("symlink in inode %" PRIu64 " too long (%llu chars)\n"),
		       lino, (unsigned long long) be64_to_cpu(dino->di_size));
		return(1);
	}

	/*
	 * have to check symlink component by component.
	 * get symlink contents into data area
	 */
	symlink = &data[0];
	if (be64_to_cpu(dino->di_size) <= XFS_DFORK_DSIZE(dino, mp))  {
		/*
		 * local symlink, just copy the symlink out of the
		 * inode into the data area
		 */
		memmove(symlink, XFS_DFORK_DPTR(dino), 
						be64_to_cpu(dino->di_size));
	} else {
		/*
		 * stored in a meta-data file, have to bmap one block
		 * at a time and copy the symlink into the data area
		 */
		i = size = amountdone = 0;
		cptr = symlink;

		while (amountdone < be64_to_cpu(dino->di_size)) {
			fsbno = blkmap_get(blkmap, i);
			if (fsbno != NULLDFSBNO)
				bp = libxfs_readbuf(mp->m_dev,
						XFS_FSB_TO_DADDR(mp, fsbno),
						XFS_FSB_TO_BB(mp, 1), 0);
			if (!bp || fsbno == NULLDFSBNO) {
				do_warn(
_("cannot read inode %" PRIu64 ", file block %d, disk block %" PRIu64 "\n"),
					lino, i, fsbno);
				return(1);
			}

			buf_data = (char *)XFS_BUF_PTR(bp);
			size = MIN(be64_to_cpu(dino->di_size) - amountdone, 
						XFS_FSB_TO_BB(mp, 1) * BBSIZE);
			memmove(cptr, buf_data, size);
			cptr += size;
			amountdone += size;
			i++;
			libxfs_putbuf(bp);
		}
	}
	data[be64_to_cpu(dino->di_size)] = '\0';

	/*
	 * check for nulls
	 */
	if (null_check(symlink, be64_to_cpu(dino->di_size)))  {
		do_warn(
_("found illegal null character in symlink inode %" PRIu64 "\n"),
			lino);
		return(1);
	}

	/*
	 * check for any component being too long
	 */
	if (be64_to_cpu(dino->di_size) >= MAXNAMELEN)  {
		cptr = strchr(symlink, '/');

		while (cptr != NULL)  {
			if (cptr - symlink >= MAXNAMELEN)  {
				do_warn(
_("component of symlink in inode %" PRIu64 " too long\n"),
					lino);
				return(1);
			}
			symlink = cptr + 1;
			cptr = strchr(symlink, '/');
		}

		if (strlen(symlink) >= MAXNAMELEN)  {
			do_warn(
_("component of symlink in inode %" PRIu64 " too long\n"),
				lino);
			return(1);
		}
	}

	return(0);
}

/*
 * called to process the set of misc inode special inode types
 * that have no associated data storage (fifos, pipes, devices, etc.).
 */
static int
process_misc_ino_types(xfs_mount_t	*mp,
			xfs_dinode_t	*dino,
			xfs_ino_t	lino,
			int		type)
{
	/*
	 * disallow mountpoint inodes until such time as the
	 * kernel actually allows them to be created (will
	 * probably require a superblock version rev, sigh).
	 */
	if (type == XR_INO_MOUNTPOINT)  {
		do_warn(
_("inode %" PRIu64 " has bad inode type (IFMNT)\n"), lino);
		return(1);
	}

	/*
	 * must also have a zero size
	 */
	if (be64_to_cpu(dino->di_size) != 0)  {
		switch (type)  {
		case XR_INO_CHRDEV:
			do_warn(
_("size of character device inode %" PRIu64 " != 0 (%" PRId64 " bytes)\n"), lino,
				(__int64_t)be64_to_cpu(dino->di_size));
			break;
		case XR_INO_BLKDEV:
			do_warn(
_("size of block device inode %" PRIu64 " != 0 (%" PRId64 " bytes)\n"), lino,
				(__int64_t)be64_to_cpu(dino->di_size));
			break;
		case XR_INO_SOCK:
			do_warn(
_("size of socket inode %" PRIu64 " != 0 (%" PRId64 " bytes)\n"), lino,
				(__int64_t)be64_to_cpu(dino->di_size));
			break;
		case XR_INO_FIFO:
			do_warn(
_("size of fifo inode %" PRIu64 " != 0 (%" PRId64 " bytes)\n"), lino,
				(__int64_t)be64_to_cpu(dino->di_size));
			break;
		default:
			do_warn(_("Internal error - process_misc_ino_types, "
				  "illegal type %d\n"), type);
			abort();
		}

		return(1);
	}

	return(0);
}

static int
process_misc_ino_types_blocks(xfs_drfsbno_t totblocks, xfs_ino_t lino, int type)
{
	/*
	 * you can not enforce all misc types have zero data fork blocks
	 * by checking dino->di_nblocks because atotblocks (attribute
	 * blocks) are part of nblocks. We must check this later when atotblocks
	 * has been calculated or by doing a simple check that anExtents == 0.
	 * We must also guarantee that totblocks is 0. Thus nblocks checking
	 * will be done later in process_dinode_int for misc types.
	 */

	if (totblocks != 0)  {
		switch (type)  {
		case XR_INO_CHRDEV:
			do_warn(
_("size of character device inode %" PRIu64 " != 0 (%" PRIu64 " blocks)\n"),
				lino, totblocks);
			break;
		case XR_INO_BLKDEV:
			do_warn(
_("size of block device inode %" PRIu64 " != 0 (%" PRIu64 " blocks)\n"),
				lino, totblocks);
			break;
		case XR_INO_SOCK:
			do_warn(
_("size of socket inode %" PRIu64 " != 0 (%" PRIu64 " blocks)\n"),
				lino, totblocks);
			break;
		case XR_INO_FIFO:
			do_warn(
_("size of fifo inode %" PRIu64 " != 0 (%" PRIu64 " blocks)\n"),
				lino, totblocks);
			break;
		default:
			return(0);
		}
		return(1);
	}
	return (0);
}

static inline int
dinode_fmt(
	xfs_dinode_t *dino)
{
	return be16_to_cpu(dino->di_mode) & S_IFMT;
}

static inline void
change_dinode_fmt(
	xfs_dinode_t	*dino,
	int		new_fmt)
{
	int		mode = be16_to_cpu(dino->di_mode);

	ASSERT((new_fmt & ~S_IFMT) == 0);

	mode &= ~S_IFMT;
	mode |= new_fmt;
	dino->di_mode = cpu_to_be16(mode);
}

static int
check_dinode_mode_format(
	xfs_dinode_t *dinoc)
{
	if (dinoc->di_format >= XFS_DINODE_FMT_UUID)
		return -1;	/* FMT_UUID is not used */

	switch (dinode_fmt(dinoc)) {
	case S_IFIFO:
	case S_IFCHR:
	case S_IFBLK:
	case S_IFSOCK:
		return (dinoc->di_format != XFS_DINODE_FMT_DEV) ? -1 : 0;

	case S_IFDIR:
		return (dinoc->di_format < XFS_DINODE_FMT_LOCAL ||
			dinoc->di_format > XFS_DINODE_FMT_BTREE) ? -1 : 0;

	case S_IFREG:
		return (dinoc->di_format < XFS_DINODE_FMT_EXTENTS ||
			dinoc->di_format > XFS_DINODE_FMT_BTREE) ? -1 : 0;

	case S_IFLNK:
		return (dinoc->di_format < XFS_DINODE_FMT_LOCAL ||
			dinoc->di_format > XFS_DINODE_FMT_EXTENTS) ? -1 : 0;

	default: ;
	}
	return 0;	/* invalid modes are checked elsewhere */
}

/*
 * If inode is a superblock inode, does type check to make sure is it valid.
 * Returns 0 if it's valid, non-zero if it needs to be cleared.
 */

static int
process_check_sb_inodes(
	xfs_mount_t	*mp,
	xfs_dinode_t	*dinoc,
	xfs_ino_t	lino,
	int		*type,
	int		*dirty)
{
	if (lino == mp->m_sb.sb_rootino) {
	 	if (*type != XR_INO_DIR)  {
			do_warn(_("root inode %" PRIu64 " has bad type 0x%x\n"),
				lino, dinode_fmt(dinoc));
			*type = XR_INO_DIR;
			if (!no_modify)  {
				do_warn(_("resetting to directory\n"));
				change_dinode_fmt(dinoc, S_IFDIR);
				*dirty = 1;
			} else
				do_warn(_("would reset to directory\n"));
		}
		return 0;
	}
	if (lino == mp->m_sb.sb_uquotino)  {
		if (*type != XR_INO_DATA)  {
			do_warn(_("user quota inode %" PRIu64 " has bad type 0x%x\n"),
				lino, dinode_fmt(dinoc));
			mp->m_sb.sb_uquotino = NULLFSINO;
			return 1;
		}
		return 0;
	}
	if (lino == mp->m_sb.sb_gquotino)  {
		if (*type != XR_INO_DATA)  {
			do_warn(_("group quota inode %" PRIu64 " has bad type 0x%x\n"),
				lino, dinode_fmt(dinoc));
			mp->m_sb.sb_gquotino = NULLFSINO;
			return 1;
		}
		return 0;
	}
	if (lino == mp->m_sb.sb_rsumino) {
		if (*type != XR_INO_RTSUM) {
			do_warn(
_("realtime summary inode %" PRIu64 " has bad type 0x%x, "),
				lino, dinode_fmt(dinoc));
			if (!no_modify)  {
				do_warn(_("resetting to regular file\n"));
				change_dinode_fmt(dinoc, S_IFREG);
				*dirty = 1;
			} else  {
				do_warn(_("would reset to regular file\n"));
			}
		}
		if (mp->m_sb.sb_rblocks == 0 && dinoc->di_nextents != 0)  {
			do_warn(
_("bad # of extents (%u) for realtime summary inode %" PRIu64 "\n"),
				be32_to_cpu(dinoc->di_nextents), lino);
			return 1;
		}
		return 0;
	}
	if (lino == mp->m_sb.sb_rbmino) {
		if (*type != XR_INO_RTBITMAP) {
			do_warn(
_("realtime bitmap inode %" PRIu64 " has bad type 0x%x, "),
				lino, dinode_fmt(dinoc));
			if (!no_modify)  {
				do_warn(_("resetting to regular file\n"));
				change_dinode_fmt(dinoc, S_IFREG);
				*dirty = 1;
			} else  {
				do_warn(_("would reset to regular file\n"));
			}
		}
		if (mp->m_sb.sb_rblocks == 0 && dinoc->di_nextents != 0)  {
			do_warn(
_("bad # of extents (%u) for realtime bitmap inode %" PRIu64 "\n"),
				be32_to_cpu(dinoc->di_nextents), lino);
			return 1;
		}
		return 0;
	}
	return 0;
}

/*
 * general size/consistency checks:
 *
 * if the size <= size of the data fork, directories  must be
 * local inodes unlike regular files which would be extent inodes.
 * all the other mentioned types have to have a zero size value.
 *
 * if the size and format don't match, get out now rather than
 * risk trying to process a non-existent extents or btree
 * type data fork.
 */
static int
process_check_inode_sizes(
	xfs_mount_t	*mp,
	xfs_dinode_t	*dino,
	xfs_ino_t	lino,
	int		type)
{
	xfs_fsize_t	size = be64_to_cpu(dino->di_size);

	switch (type)  {

	case XR_INO_DIR:
		if (size <= XFS_DFORK_DSIZE(dino, mp) &&
				dino->di_format != XFS_DINODE_FMT_LOCAL) {
			do_warn(
_("mismatch between format (%d) and size (%" PRId64 ") in directory ino %" PRIu64 "\n"),
				dino->di_format, size, lino);
			return 1;
		}
		if (size > XFS_DIR2_LEAF_OFFSET) {
			do_warn(
_("directory inode %" PRIu64 " has bad size %" PRId64 "\n"),
				lino, size);
			return 1;
		}
		break;

	case XR_INO_SYMLINK:
		if (process_symlink_extlist(mp, lino, dino))  {
			do_warn(_("bad data fork in symlink %" PRIu64 "\n"), lino);
			return 1;
		}
		break;

	case XR_INO_CHRDEV:	/* fall through to FIFO case ... */
	case XR_INO_BLKDEV:	/* fall through to FIFO case ... */
	case XR_INO_SOCK:	/* fall through to FIFO case ... */
	case XR_INO_MOUNTPOINT:	/* fall through to FIFO case ... */
	case XR_INO_FIFO:
		if (process_misc_ino_types(mp, dino, lino, type))
			return 1;
		break;

	case XR_INO_RTDATA:
		/*
		 * if we have no realtime blocks, any inode claiming
		 * to be a real-time file is bogus
		 */
		if (mp->m_sb.sb_rblocks == 0)  {
			do_warn(
_("found inode %" PRIu64 " claiming to be a real-time file\n"), lino);
			return 1;
		}
		break;

	case XR_INO_RTBITMAP:
		if (size != (__int64_t)mp->m_sb.sb_rbmblocks *
					mp->m_sb.sb_blocksize) {
			do_warn(
_("realtime bitmap inode %" PRIu64 " has bad size %" PRId64 " (should be %" PRIu64 ")\n"),
				lino, size,
				(__int64_t) mp->m_sb.sb_rbmblocks *
					mp->m_sb.sb_blocksize);
			return 1;
		}
		break;

	case XR_INO_RTSUM:
		if (size != mp->m_rsumsize)  {
			do_warn(
_("realtime summary inode %" PRIu64 " has bad size %" PRId64 " (should be %d)\n"),
				lino, size, mp->m_rsumsize);
			return 1;
		}
		break;

	default:
		break;
	}
	return 0;
}

/*
 * check for illegal values of forkoff
 */
static int
process_check_inode_forkoff(
	xfs_mount_t	*mp,
	xfs_dinode_t	*dino,
	xfs_ino_t	lino)
{
	if (dino->di_forkoff == 0)
		return 0;

	switch (dino->di_format)  {
	case XFS_DINODE_FMT_DEV:
		if (dino->di_forkoff != (roundup(sizeof(xfs_dev_t), 8) >> 3)) {
			do_warn(
_("bad attr fork offset %d in dev inode %" PRIu64 ", should be %d\n"),
				dino->di_forkoff, lino,
				(int)(roundup(sizeof(xfs_dev_t), 8) >> 3));
			return 1;
		}
		break;
	case XFS_DINODE_FMT_LOCAL:	/* fall through ... */
	case XFS_DINODE_FMT_EXTENTS:	/* fall through ... */
	case XFS_DINODE_FMT_BTREE:
		if (dino->di_forkoff >= (XFS_LITINO(mp) >> 3)) {
			do_warn(
_("bad attr fork offset %d in inode %" PRIu64 ", max=%d\n"),
				dino->di_forkoff, lino,
				XFS_LITINO(mp) >> 3);
			return 1;
		}
		break;
	default:
		do_error(_("unexpected inode format %d\n"), dino->di_format);
		break;
	}
	return 0;
}

/*
 * Updates the inodes block and extent counts if they are wrong
 */
static int
process_inode_blocks_and_extents(
	xfs_dinode_t	*dino,
	xfs_drfsbno_t	nblocks,
	__uint64_t	nextents,
	__uint64_t	anextents,
	xfs_ino_t	lino,
	int		*dirty)
{
	if (nblocks != be64_to_cpu(dino->di_nblocks))  {
		if (!no_modify)  {
			do_warn(
_("correcting nblocks for inode %" PRIu64 ", was %llu - counted %" PRIu64 "\n"), lino,
			       (unsigned long long) be64_to_cpu(dino->di_nblocks),
			       nblocks);
			dino->di_nblocks = cpu_to_be64(nblocks);
			*dirty = 1;
		} else  {
			do_warn(
_("bad nblocks %llu for inode %" PRIu64 ", would reset to %" PRIu64 "\n"),
			       (unsigned long long) be64_to_cpu(dino->di_nblocks),
			       lino, nblocks);
		}
	}

	if (nextents > MAXEXTNUM)  {
		do_warn(
_("too many data fork extents (%" PRIu64 ") in inode %" PRIu64 "\n"),
			nextents, lino);
		return 1;
	}
	if (nextents != be32_to_cpu(dino->di_nextents))  {
		if (!no_modify)  {
			do_warn(
_("correcting nextents for inode %" PRIu64 ", was %d - counted %" PRIu64 "\n"),
				lino,
				be32_to_cpu(dino->di_nextents),
				nextents);
			dino->di_nextents = cpu_to_be32(nextents);
			*dirty = 1;
		} else  {
			do_warn(
_("bad nextents %d for inode %" PRIu64 ", would reset to %" PRIu64 "\n"),
				be32_to_cpu(dino->di_nextents),
				lino, nextents);
		}
	}

	if (anextents > MAXAEXTNUM)  {
		do_warn(
_("too many attr fork extents (%" PRIu64 ") in inode %" PRIu64 "\n"),
			anextents, lino);
		return 1;
	}
	if (anextents != be16_to_cpu(dino->di_anextents))  {
		if (!no_modify)  {
			do_warn(
_("correcting anextents for inode %" PRIu64 ", was %d - counted %" PRIu64 "\n"),
				lino,
				be16_to_cpu(dino->di_anextents), anextents);
			dino->di_anextents = cpu_to_be16(anextents);
			*dirty = 1;
		} else  {
			do_warn(
_("bad anextents %d for inode %" PRIu64 ", would reset to %" PRIu64 "\n"),
				be16_to_cpu(dino->di_anextents),
				lino, anextents);
		}
	}
	return 0;
}

/*
 * check data fork -- if it's bad, clear the inode
 */
static int
process_inode_data_fork(
	xfs_mount_t	*mp,
	xfs_agnumber_t	agno,
	xfs_agino_t	ino,
	xfs_dinode_t	*dino,
	int		type,
	int		*dirty,
	xfs_drfsbno_t	*totblocks,
	__uint64_t	*nextents,
	blkmap_t	**dblkmap,
	int		check_dups)
{
	xfs_ino_t	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	int		err = 0;

	*nextents = be32_to_cpu(dino->di_nextents);
	if (*nextents > be64_to_cpu(dino->di_nblocks))
		*nextents = 1;

	if (dino->di_format != XFS_DINODE_FMT_LOCAL && type != XR_INO_RTDATA)
		*dblkmap = blkmap_alloc(*nextents, XFS_DATA_FORK);
	*nextents = 0;

	switch (dino->di_format) {
	case XFS_DINODE_FMT_LOCAL:
		err = process_lclinode(mp, agno, ino, dino, XFS_DATA_FORK);
		*totblocks = 0;
		break;
	case XFS_DINODE_FMT_EXTENTS:
		err = process_exinode(mp, agno, ino, dino, type, dirty,
			totblocks, nextents, dblkmap, XFS_DATA_FORK,
			check_dups);
		break;
	case XFS_DINODE_FMT_BTREE:
		err = process_btinode(mp, agno, ino, dino, type, dirty,
			totblocks, nextents, dblkmap, XFS_DATA_FORK,
			check_dups);
		break;
	case XFS_DINODE_FMT_DEV:	/* fall through */
		err = 0;
		break;
	default:
		do_error(_("unknown format %d, ino %" PRIu64 " (mode = %d)\n"),
			dino->di_format, lino, be16_to_cpu(dino->di_mode));
	}

	if (err)  {
		do_warn(_("bad data fork in inode %" PRIu64 "\n"), lino);
		if (!no_modify)  {
			*dirty += clear_dinode(mp, dino, lino);
			ASSERT(*dirty > 0);
		}
		return 1;
	}

	if (check_dups)  {
		/*
		 * if check_dups was non-zero, we have to
		 * re-process data fork to set bitmap since the
		 * bitmap wasn't set the first time through
		 */
		switch (dino->di_format) {
		case XFS_DINODE_FMT_LOCAL:
			err = process_lclinode(mp, agno, ino, dino, 
						XFS_DATA_FORK);
			break;
		case XFS_DINODE_FMT_EXTENTS:
			err = process_exinode(mp, agno, ino, dino, type,
				dirty, totblocks, nextents, dblkmap,
				XFS_DATA_FORK, 0);
			break;
		case XFS_DINODE_FMT_BTREE:
			err = process_btinode(mp, agno, ino, dino, type,
				dirty, totblocks, nextents, dblkmap,
				XFS_DATA_FORK, 0);
			break;
		case XFS_DINODE_FMT_DEV:	/* fall through */
			err = 0;
			break;
		default:
			do_error(_("unknown format %d, ino %" PRIu64 " (mode = %d)\n"),
				dino->di_format, lino,
				be16_to_cpu(dino->di_mode));
		}

		if (no_modify && err != 0)
			return 1;

		ASSERT(err == 0);
	}
	return 0;
}

/*
 * Process extended attribute fork in inode
 */
static int
process_inode_attr_fork(
	xfs_mount_t	*mp,
	xfs_agnumber_t	agno,
	xfs_agino_t	ino,
	xfs_dinode_t	*dino,
	int		type,
	int		*dirty,
	xfs_drfsbno_t	*atotblocks,
	__uint64_t	*anextents,
	int		check_dups,
	int		extra_attr_check,
	int		*retval)
{
	xfs_ino_t	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	blkmap_t	*ablkmap = NULL;
	int		repair = 0;
	int		err;

	if (!XFS_DFORK_Q(dino)) {
		*anextents = 0;
		if (dino->di_aformat != XFS_DINODE_FMT_EXTENTS) {
			do_warn(_("bad attribute format %d in inode %" PRIu64 ", "),
				dino->di_aformat, lino);
			if (!no_modify) {
				do_warn(_("resetting value\n"));
				dino->di_aformat = XFS_DINODE_FMT_EXTENTS;
				*dirty = 1;
			} else
				do_warn(_("would reset value\n"));
		}
		return 0;
	}

	*anextents = be16_to_cpu(dino->di_anextents);
	if (*anextents > be64_to_cpu(dino->di_nblocks))
		*anextents = 1;

	switch (dino->di_aformat) {
	case XFS_DINODE_FMT_LOCAL:
		*anextents = 0;
		*atotblocks = 0;
		err = process_lclinode(mp, agno, ino, dino, XFS_ATTR_FORK);
		break;
	case XFS_DINODE_FMT_EXTENTS:
		ablkmap = blkmap_alloc(*anextents, XFS_ATTR_FORK);
		*anextents = 0;
		err = process_exinode(mp, agno, ino, dino, type, dirty,
				atotblocks, anextents, &ablkmap,
				XFS_ATTR_FORK, check_dups);
		break;
	case XFS_DINODE_FMT_BTREE:
		ablkmap = blkmap_alloc(*anextents, XFS_ATTR_FORK);
		*anextents = 0;
		err = process_btinode(mp, agno, ino, dino, type, dirty,
				atotblocks, anextents, &ablkmap,
				XFS_ATTR_FORK, check_dups);
		break;
	default:
		do_warn(_("illegal attribute format %d, ino %" PRIu64 "\n"),
				dino->di_aformat, lino);
		err = 1;
		break;
	}

	if (err) {
		/*
		 * clear the attribute fork if necessary.  we can't
		 * clear the inode because we've already put the
		 * inode space info into the blockmap.
		 *
		 * XXX - put the inode onto the "move it" list and
		 *	log the the attribute scrubbing
		 */
		do_warn(_("bad attribute fork in inode %" PRIu64), lino);

		if (!no_modify)  {
			if (delete_attr_ok)  {
				do_warn(_(", clearing attr fork\n"));
				*dirty += clear_dinode_attr(mp, dino, lino);
				dino->di_aformat = XFS_DINODE_FMT_LOCAL;
			} else  {
				do_warn("\n");
				*dirty += clear_dinode(mp, dino, lino);
			}
			ASSERT(*dirty > 0);
		} else  {
			do_warn(_(", would clear attr fork\n"));
		}

		*atotblocks = 0;
		*anextents = 0;
		blkmap_free(ablkmap);
		*retval = 1;

		return delete_attr_ok ? 0 : 1;
	}

	if (check_dups)  {
		switch (dino->di_aformat) {
		case XFS_DINODE_FMT_LOCAL:
			err = process_lclinode(mp, agno, ino, dino, 
						XFS_ATTR_FORK);
			break;
		case XFS_DINODE_FMT_EXTENTS:
			err = process_exinode(mp, agno, ino, dino,
				type, dirty, atotblocks, anextents,
				&ablkmap, XFS_ATTR_FORK, 0);
			break;
		case XFS_DINODE_FMT_BTREE:
			err = process_btinode(mp, agno, ino, dino,
				type, dirty, atotblocks, anextents,
				&ablkmap, XFS_ATTR_FORK, 0);
			break;
		default:
			do_error(_("illegal attribute fmt %d, ino %" PRIu64 "\n"),
				dino->di_aformat, lino);
		}

		if (no_modify && err != 0) {
			blkmap_free(ablkmap);
			return 1;
		}

		ASSERT(err == 0);
	}

	/*
	 * do attribute semantic-based consistency checks now
	 */

	/* get this only in phase 3, not in both phase 3 and 4 */
	if (extra_attr_check &&
			process_attributes(mp, lino, dino, ablkmap, &repair)) {
		do_warn(
	_("problem with attribute contents in inode %" PRIu64 "\n"),
			lino);
		if (!repair) {
			/* clear attributes if not done already */
			if (!no_modify)  {
				*dirty += clear_dinode_attr(mp, dino, lino);
				dino->di_aformat = XFS_DINODE_FMT_LOCAL;
			} else  {
				do_warn(_("would clear attr fork\n"));
			}
			*atotblocks = 0;
			*anextents = 0;
		}
		else {
			*dirty = 1; /* it's been repaired */
		}
	}
	blkmap_free(ablkmap);
	return 0;
}

/*
 * check nlinks feature, if it's a version 1 inode,
 * just leave nlinks alone.  even if it's set wrong,
 * it'll be reset when read in.
 */

static int
process_check_inode_nlink_version(
	xfs_dinode_t	*dino,
	xfs_ino_t	lino)
{
	int		dirty = 0;

	if (dino->di_version > 1 && !fs_inode_nlink)  {
		/*
		 * do we have a fs/inode version mismatch with a valid
		 * version 2 inode here that has to stay version 2 or
		 * lose links?
		 */
		if (be32_to_cpu(dino->di_nlink) > XFS_MAXLINK_1)  {
			/*
			 * yes.  are nlink inodes allowed?
			 */
			if (fs_inode_nlink_allowed)  {
				/*
				 * yes, update status variable which will
				 * cause sb to be updated later.
				 */
				fs_inode_nlink = 1;
				do_warn
	(_("version 2 inode %" PRIu64 " claims > %u links, "),
					lino, XFS_MAXLINK_1);
				if (!no_modify)  {
					do_warn(
	_("updating superblock version number\n"));
				} else  {
					do_warn(
	_("would update superblock version number\n"));
				}
			} else  {
				/*
				 * no, have to convert back to onlinks
				 * even if we lose some links
				 */
				do_warn(
	_("WARNING:  version 2 inode %" PRIu64 " claims > %u links, "),
					lino, XFS_MAXLINK_1);
				if (!no_modify)  {
					do_warn(_("converting back to version 1,\n"
						"this may destroy %d links\n"),
						be32_to_cpu(dino->di_nlink) -
							XFS_MAXLINK_1);

					dino->di_version = 1;
					dino->di_nlink = cpu_to_be32(XFS_MAXLINK_1);
					dino->di_onlink = cpu_to_be16(XFS_MAXLINK_1);
					dirty = 1;
				} else  {
					do_warn(_("would convert back to version 1,\n"
						"\tthis might destroy %d links\n"),
						be32_to_cpu(dino->di_nlink) -
							XFS_MAXLINK_1);
				}
			}
		} else  {
			/*
			 * do we have a v2 inode that we could convert back
			 * to v1 without losing any links?  if we do and
			 * we have a mismatch between superblock bits and the
			 * version bit, alter the version bit in this case.
			 *
			 * the case where we lost links was handled above.
			 */
			do_warn(_("found version 2 inode %" PRIu64 ", "), lino);
			if (!no_modify)  {
				do_warn(_("converting back to version 1\n"));
				dino->di_version = 1;
				dino->di_onlink = cpu_to_be16(
					be32_to_cpu(dino->di_nlink));
				dirty = 1;
			} else  {
				do_warn(_("would convert back to version 1\n"));
			}
		}
	}

	/*
	 * ok, if it's still a version 2 inode, it's going
	 * to stay a version 2 inode.  it should have a zero
	 * onlink field, so clear it.
	 */
	if (dino->di_version > 1 &&
			dino->di_onlink != 0 && fs_inode_nlink > 0) {
		if (!no_modify) {
			do_warn(
_("clearing obsolete nlink field in version 2 inode %" PRIu64 ", was %d, now 0\n"),
				lino, be16_to_cpu(dino->di_onlink));
			dino->di_onlink = 0;
			dirty = 1;
		} else  {
			do_warn(
_("would clear obsolete nlink field in version 2 inode %" PRIu64 ", currently %d\n"),
				lino, be16_to_cpu(dino->di_onlink));
		}
	}
	return dirty;
}

/*
 * returns 0 if the inode is ok, 1 if the inode is corrupt
 * check_dups can be set to 1 *only* when called by the
 * first pass of the duplicate block checking of phase 4.
 * *dirty is set > 0 if the dinode has been altered and
 * needs to be written out.
 *
 * for detailed, info, look at process_dinode() comments.
 */
/* ARGSUSED */
int
process_dinode_int(xfs_mount_t *mp,
		xfs_dinode_t *dino,
		xfs_agnumber_t agno,
		xfs_agino_t ino,
		int was_free,		/* 1 if inode is currently free */
		int *dirty,		/* out == > 0 if inode is now dirty */
		int *used,		/* out == 1 if inode is in use */
		int verify_mode,	/* 1 == verify but don't modify inode */
		int uncertain,		/* 1 == inode is uncertain */
		int ino_discovery,	/* 1 == check dirs for unknown inodes */
		int check_dups,		/* 1 == check if inode claims
					 * duplicate blocks		*/
		int extra_attr_check, /* 1 == do attribute format and value checks */
		int *isa_dir,		/* out == 1 if inode is a directory */
		xfs_ino_t *parent)	/* out -- parent if ino is a dir */
{
	xfs_drfsbno_t		totblocks = 0;
	xfs_drfsbno_t		atotblocks = 0;
	int			di_mode;
	int			type;
	int			retval = 0;
	__uint64_t		nextents;
	__uint64_t		anextents;
	xfs_ino_t		lino;
	const int		is_free = 0;
	const int		is_used = 1;
	blkmap_t		*dblkmap = NULL;

	*dirty = *isa_dir = 0;
	*used = is_used;
	type = XR_INO_UNKNOWN;

	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	di_mode = be16_to_cpu(dino->di_mode);

	/*
	 * if in verify mode, don't modify the inode.
	 *
	 * if correcting, reset stuff that has known values
	 *
	 * if in uncertain mode, be silent on errors since we're
	 * trying to find out if these are inodes as opposed
	 * to assuming that they are.  Just return the appropriate
	 * return code in that case.
	 *
	 * If uncertain is set, verify_mode MUST be set.
	 */
	ASSERT(uncertain == 0 || verify_mode != 0);

	if (be16_to_cpu(dino->di_magic) != XFS_DINODE_MAGIC)  {
		retval = 1;
		if (!uncertain)
			do_warn(_("bad magic number 0x%x on inode %" PRIu64 "%c"),
				be16_to_cpu(dino->di_magic), lino,
				verify_mode ? '\n' : ',');
		if (!verify_mode) {
			if (!no_modify)  {
				do_warn(_(" resetting magic number\n"));
				dino->di_magic = cpu_to_be16(XFS_DINODE_MAGIC);
				*dirty = 1;
			} else
				do_warn(_(" would reset magic number\n"));
		}
	}

	if (!XFS_DINODE_GOOD_VERSION(dino->di_version) ||
	    (!fs_inode_nlink && dino->di_version > 1))  {
		retval = 1;
		if (!uncertain)
			do_warn(_("bad version number 0x%x on inode %" PRIu64 "%c"),
				(__s8)dino->di_version, lino,
				verify_mode ? '\n' : ',');
		if (!verify_mode) {
			if (!no_modify) {
				do_warn(_(" resetting version number\n"));
				dino->di_version = (fs_inode_nlink) ?  2 : 1;
				*dirty = 1;
			} else
				do_warn(_(" would reset version number\n"));
		}
	}

	/*
	 * blow out of here if the inode size is < 0
	 */
	if ((xfs_fsize_t)be64_to_cpu(dino->di_size) < 0)  {
		if (!uncertain)
			do_warn(
_("bad (negative) size %" PRId64 " on inode %" PRIu64 "\n"),
				(__int64_t)be64_to_cpu(dino->di_size),
				lino);
		if (verify_mode)
			return 1;
		goto clear_bad_out;
	}

	/*
	 * if not in verify mode, check to sii if the inode and imap
	 * agree that the inode is free
	 */
	if (!verify_mode && di_mode == 0) {
		/*
		 * was_free value is not meaningful if we're in verify mode
		 */
		if (was_free) {
			/*
			 * easy case, inode free -- inode and map agree, clear
			 * it just in case to ensure that format, etc. are
			 * set correctly
			 */
			if (!no_modify)
				*dirty += clear_dinode(mp, dino, lino);
			*used = is_free;
			return 0;
		}
		/*
		 * the inode looks free but the map says it's in use.
		 * clear the inode just to be safe and mark the inode
		 * free.
		 */
		do_warn(
	_("imap claims a free inode %" PRIu64 " is in use, "), lino);
		if (!no_modify)  {
			do_warn(_("correcting imap and clearing inode\n"));
			*dirty += clear_dinode(mp, dino, lino);
			retval = 1;
		} else
			do_warn(_("would correct imap and clear inode\n"));
		*used = is_free;
		return retval;
	}

	/*
	 * because of the lack of any write ordering guarantee, it's
	 * possible that the core got updated but the forks didn't.
	 * so rather than be ambitious (and probably incorrect),
	 * if there's an inconsistency, we get conservative and
	 * just pitch the file.  blow off checking formats of
	 * free inodes since technically any format is legal
	 * as we reset the inode when we re-use it.
	 */
	if (di_mode != 0 && check_dinode_mode_format(dino) != 0) {
		if (!uncertain)
			do_warn(
	_("bad inode format in inode %" PRIu64 "\n"), lino);
		if (verify_mode)
			return 1;
		goto clear_bad_out;
	}

	/*
	 * check that we only have valid flags set, and those that are set make
	 * sense.
	 */
	if (dino->di_flags) {
		uint16_t flags = be16_to_cpu(dino->di_flags);

		if (flags & ~XFS_DIFLAG_ANY) {
			do_warn(_("Bad flags set in inode %" PRIu64), lino);
			flags &= ~XFS_DIFLAG_ANY;
		}

		if (flags & (XFS_DIFLAG_REALTIME | XFS_DIFLAG_RTINHERIT)) {
			/* need an rt-dev! */
			if (!rt_name) {
				do_warn(
	_("inode %" PRIu64 " has RT flag set but there is no RT device"),
					lino);
				flags &= ~(XFS_DIFLAG_REALTIME |
						XFS_DIFLAG_RTINHERIT);
			}
		}
		if (flags & XFS_DIFLAG_NEWRTBM) {
			/* must be a rt bitmap inode */
			if (lino != mp->m_sb.sb_rbmino) {
				do_warn(_("inode %" PRIu64 " not rt bitmap"),
					lino);
				flags &= ~XFS_DIFLAG_NEWRTBM;
			}
		}
		if (flags & (XFS_DIFLAG_RTINHERIT |
			     XFS_DIFLAG_EXTSZINHERIT |
			     XFS_DIFLAG_PROJINHERIT |
			     XFS_DIFLAG_NOSYMLINKS)) {
			/* must be a directory */
			if (di_mode && !S_ISDIR(di_mode)) {
				do_warn(
	_("directory flags set on non-directory inode %" PRIu64 ),
					lino);
				flags &= ~(XFS_DIFLAG_RTINHERIT |
						XFS_DIFLAG_EXTSZINHERIT |
						XFS_DIFLAG_PROJINHERIT |
						XFS_DIFLAG_NOSYMLINKS);
			}
		}
		if (flags & (XFS_DIFLAG_REALTIME | XFS_XFLAG_EXTSIZE)) {
			/* must be a file */
			if (di_mode && !S_ISREG(di_mode)) {
				do_warn(
	_("file flags set on non-file inode %" PRIu64),
					lino);
				flags &= ~(XFS_DIFLAG_REALTIME |
						XFS_XFLAG_EXTSIZE);
			}
		}
		if (!verify_mode && flags != be16_to_cpu(dino->di_flags)) {
			if (!no_modify) {
				do_warn(_(", fixing bad flags.\n"));
				dino->di_flags = cpu_to_be16(flags);
				*dirty = 1;
			} else
				do_warn(_(", would fix bad flags.\n"));
		}
	}

	if (verify_mode)
		return retval;

	/*
	 * clear the next unlinked field if necessary on a good
	 * inode only during phase 4 -- when checking for inodes
	 * referencing duplicate blocks.  then it's safe because
	 * we've done the inode discovery and have found all the inodes
	 * we're going to find.  check_dups is set to 1 only during
	 * phase 4.  Ugly.
	 */
	if (check_dups && !no_modify)
		*dirty += clear_dinode_unlinked(mp, dino);

	/* set type and map type info */

	switch (di_mode & S_IFMT) {
	case S_IFDIR:
		type = XR_INO_DIR;
		*isa_dir = 1;
		break;
	case S_IFREG:
		if (be16_to_cpu(dino->di_flags) & XFS_DIFLAG_REALTIME)
			type = XR_INO_RTDATA;
		else if (lino == mp->m_sb.sb_rbmino)
			type = XR_INO_RTBITMAP;
		else if (lino == mp->m_sb.sb_rsumino)
			type = XR_INO_RTSUM;
		else
			type = XR_INO_DATA;
		break;
	case S_IFLNK:
		type = XR_INO_SYMLINK;
		break;
	case S_IFCHR:
		type = XR_INO_CHRDEV;
		break;
	case S_IFBLK:
		type = XR_INO_BLKDEV;
		break;
	case S_IFSOCK:
		type = XR_INO_SOCK;
		break;
	case S_IFIFO:
		type = XR_INO_FIFO;
		break;
	default:
		do_warn(_("bad inode type %#o inode %" PRIu64 "\n"),
				di_mode & S_IFMT, lino);
		goto clear_bad_out;
	}

	/*
	 * type checks for superblock inodes
	 */
	if (process_check_sb_inodes(mp, dino, lino, &type, dirty) != 0)
		goto clear_bad_out;

	/*
	 * only regular files with REALTIME or EXTSIZE flags set can have
	 * extsize set, or directories with EXTSZINHERIT.
	 */
	if (be32_to_cpu(dino->di_extsize) != 0) {
		if ((type == XR_INO_RTDATA) ||
		    (type == XR_INO_DIR && (be16_to_cpu(dino->di_flags) &
					XFS_DIFLAG_EXTSZINHERIT)) ||
		    (type == XR_INO_DATA && (be16_to_cpu(dino->di_flags) &
				 XFS_DIFLAG_EXTSIZE)))  {
			/* s'okay */ ;
		} else {
			do_warn(
_("bad non-zero extent size %u for non-realtime/extsize inode %" PRIu64 ", "),
					be32_to_cpu(dino->di_extsize), lino);
			if (!no_modify)  {
				do_warn(_("resetting to zero\n"));
				dino->di_extsize = 0;
				*dirty = 1;
			} else
				do_warn(_("would reset to zero\n"));
		}
	}

	/*
	 * general size/consistency checks:
	 */
	if (process_check_inode_sizes(mp, dino, lino, type) != 0)
		goto clear_bad_out;

	/*
	 * check for illegal values of forkoff
	 */
	if (process_check_inode_forkoff(mp, dino, lino) != 0)
		goto clear_bad_out;

	/*
	 * check data fork -- if it's bad, clear the inode
	 */
	if (process_inode_data_fork(mp, agno, ino, dino, type, dirty,
			&totblocks, &nextents, &dblkmap, check_dups) != 0)
		goto bad_out;

	/*
	 * check attribute fork if necessary.  attributes are
	 * always stored in the regular filesystem.
	 */
	if (process_inode_attr_fork(mp, agno, ino, dino, type, dirty,
			&atotblocks, &anextents, check_dups, extra_attr_check,
			&retval))
		goto bad_out;

	/*
	 * enforce totblocks is 0 for misc types
	 */
	if (process_misc_ino_types_blocks(totblocks, lino, type))
		goto clear_bad_out;

	/*
	 * correct space counters if required
	 */
	if (process_inode_blocks_and_extents(dino, totblocks + atotblocks,
			nextents, anextents, lino, dirty) != 0)
		goto clear_bad_out;

	/*
	 * do any semantic type-based checking here
	 */
	switch (type)  {
	case XR_INO_DIR:
		if (xfs_sb_version_hasdirv2(&mp->m_sb) ?
				process_dir2(mp, lino, dino, ino_discovery,
						dirty, "", parent, dblkmap) :
				process_dir(mp, lino, dino, ino_discovery,
						dirty, "", parent, dblkmap)) {
			do_warn(
	_("problem with directory contents in inode %" PRIu64 "\n"),
				lino);
			goto clear_bad_out;
		}
		break;
	case XR_INO_SYMLINK:
		if (process_symlink(mp, lino, dino, dblkmap) != 0) {
			do_warn(
	_("problem with symbolic link in inode %" PRIu64 "\n"),
				lino);
			goto clear_bad_out;
		}
		break;
	default:
		break;
	}

	blkmap_free(dblkmap);

	/*
	 * check nlinks feature, if it's a version 1 inode,
	 * just leave nlinks alone.  even if it's set wrong,
	 * it'll be reset when read in.
	 */
	*dirty += process_check_inode_nlink_version(dino, lino);

	return retval;

clear_bad_out:
	if (!no_modify)  {
		*dirty += clear_dinode(mp, dino, lino);
		ASSERT(*dirty > 0);
	}
bad_out:
	*used = is_free;
	*isa_dir = 0;
	blkmap_free(dblkmap);
	return 1;
}

/*
 * returns 1 if inode is used, 0 if free.
 * performs any necessary salvaging actions.
 * note that we leave the generation count alone
 * because nothing we could set it to would be
 * guaranteed to be correct so the best guess for
 * the correct value is just to leave it alone.
 *
 * The trick is detecting empty files.  For those,
 * the core and the forks should all be in the "empty"
 * or zero-length state -- a zero or possibly minimum length
 * (in the case of dirs) extent list -- although inline directories
 * and symlinks might be handled differently.  So it should be
 * possible to sanity check them against each other.
 *
 * If the forks are an empty extent list though, then forget it.
 * The file is toast anyway since we can't recover its storage.
 *
 * Parameters:
 *	Ins:
 *		mp -- mount structure
 *		dino -- pointer to on-disk inode structure
 *		agno/ino -- inode numbers
 *		free -- whether the map thinks the inode is free (1 == free)
 *		ino_discovery -- whether we should examine directory
 *				contents to discover new inodes
 *		check_dups -- whether we should check to see if the
 *				inode references duplicate blocks
 *				if so, we compare the inode's claimed
 *				blocks against the contents of the
 *				duplicate extent list but we don't
 *				set the bitmap.  If not, we set the
 *				bitmap and try and detect multiply
 *				claimed blocks using the bitmap.
 *	Outs:
 *		dirty -- whether we changed the inode (1 == yes)
 *		used -- 1 if the inode is used, 0 if free.  In no modify
 *			mode, whether the inode should be used or free
 *		isa_dir -- 1 if the inode is a directory, 0 if not.  In
 *			no modify mode, if the inode would be a dir or not.
 *
 *	Return value -- 0 if the inode is good, 1 if it is/was corrupt
 */

int
process_dinode(
	xfs_mount_t	*mp,
	xfs_dinode_t	*dino,
	xfs_agnumber_t	agno,
	xfs_agino_t	ino,
	int		was_free,
	int		*dirty,
	int		*used,
	int		ino_discovery,
	int		check_dups,
	int		extra_attr_check,
	int		*isa_dir,
	xfs_ino_t	*parent)
{
	const int	verify_mode = 0;
	const int	uncertain = 0;

#ifdef XR_INODE_TRACE
	fprintf(stderr, _("processing inode %d/%d\n"), agno, ino);
#endif
	return process_dinode_int(mp, dino, agno, ino, was_free, dirty, used,
				verify_mode, uncertain, ino_discovery,
				check_dups, extra_attr_check, isa_dir, parent);
}

/*
 * a more cursory check, check inode core, *DON'T* check forks
 * this basically just verifies whether the inode is an inode
 * and whether or not it has been totally trashed.  returns 0
 * if the inode passes the cursory sanity check, 1 otherwise.
 */
int
verify_dinode(
	xfs_mount_t	*mp,
	xfs_dinode_t	*dino,
	xfs_agnumber_t	agno,
	xfs_agino_t	ino)
{
	xfs_ino_t	parent;
	int		used = 0;
	int		dirty = 0;
	int		isa_dir = 0;
	const int	verify_mode = 1;
	const int	check_dups = 0;
	const int	ino_discovery = 0;
	const int	uncertain = 0;

	return process_dinode_int(mp, dino, agno, ino, 0, &dirty, &used,
				verify_mode, uncertain, ino_discovery,
				check_dups, 0, &isa_dir, &parent);
}

/*
 * like above only for inode on the uncertain list.  it sets
 * the uncertain flag which makes process_dinode_int quieter.
 * returns 0 if the inode passes the cursory sanity check, 1 otherwise.
 */
int
verify_uncertain_dinode(
	xfs_mount_t	*mp,
	xfs_dinode_t	*dino,
	xfs_agnumber_t	agno,
	xfs_agino_t	ino)
{
	xfs_ino_t	parent;
	int		used = 0;
	int		dirty = 0;
	int		isa_dir = 0;
	const int	verify_mode = 1;
	const int	check_dups = 0;
	const int	ino_discovery = 0;
	const int	uncertain = 1;

	return process_dinode_int(mp, dino, agno, ino, 0, &dirty, &used,
				verify_mode, uncertain, ino_discovery,
				check_dups, 0, &isa_dir, &parent);
}
