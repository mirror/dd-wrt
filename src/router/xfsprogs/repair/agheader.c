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

#include <libxfs.h>
#include "globals.h"
#include "agheader.h"
#include "protos.h"
#include "err_protos.h"

int
verify_set_agf(xfs_mount_t *mp, xfs_agf_t *agf, xfs_agnumber_t i)
{
	xfs_drfsbno_t agblocks;
	int retval = 0;

	/* check common fields */

	if (be32_to_cpu(agf->agf_magicnum) != XFS_AGF_MAGIC)  {
		retval = XR_AG_AGF;
		do_warn(_("bad magic # 0x%x for agf %d\n"),
			be32_to_cpu(agf->agf_magicnum), i);

		if (!no_modify)
			agf->agf_magicnum = cpu_to_be32(XFS_AGF_MAGIC);
	}

	if (!XFS_AGF_GOOD_VERSION(be32_to_cpu(agf->agf_versionnum)))  {
		retval = XR_AG_AGF;
		do_warn(_("bad version # %d for agf %d\n"),
			be32_to_cpu(agf->agf_versionnum), i);

		if (!no_modify)
			agf->agf_versionnum = cpu_to_be32(XFS_AGF_VERSION);
	}

	if (be32_to_cpu(agf->agf_seqno) != i)  {
		retval = XR_AG_AGF;
		do_warn(_("bad sequence # %d for agf %d\n"),
			be32_to_cpu(agf->agf_seqno), i);

		if (!no_modify)
			agf->agf_seqno = cpu_to_be32(i);
	}

	if (be32_to_cpu(agf->agf_length) != mp->m_sb.sb_agblocks)  {
		if (i != mp->m_sb.sb_agcount - 1)  {
			retval = XR_AG_AGF;
			do_warn(_("bad length %d for agf %d, should be %d\n"),
				be32_to_cpu(agf->agf_length), i,
				mp->m_sb.sb_agblocks);
			if (!no_modify)
				agf->agf_length = 
					cpu_to_be32(mp->m_sb.sb_agblocks);
		} else  {
			agblocks = mp->m_sb.sb_dblocks -
				(xfs_drfsbno_t) mp->m_sb.sb_agblocks * i;

			if (be32_to_cpu(agf->agf_length) != agblocks)  {
				retval = XR_AG_AGF;
				do_warn(
	_("bad length %d for agf %d, should be %" PRIu64 "\n"),
					be32_to_cpu(agf->agf_length),
						i, agblocks);
				if (!no_modify)
					agf->agf_length = cpu_to_be32(agblocks);
			}
		}
	}

	/*
	 * check first/last AGF fields.  if need be, lose the free
	 * space in the AGFL, we'll reclaim it later.
	 */
	if (be32_to_cpu(agf->agf_flfirst) >= XFS_AGFL_SIZE(mp))  {
		do_warn(_("flfirst %d in agf %d too large (max = %zu)\n"),
			be32_to_cpu(agf->agf_flfirst),
			i, XFS_AGFL_SIZE(mp));
		if (!no_modify)
			agf->agf_flfirst = cpu_to_be32(0);
	}

	if (be32_to_cpu(agf->agf_fllast) >= XFS_AGFL_SIZE(mp))  {
		do_warn(_("fllast %d in agf %d too large (max = %zu)\n"),
			be32_to_cpu(agf->agf_fllast),
			i, XFS_AGFL_SIZE(mp));
		if (!no_modify)
			agf->agf_fllast = cpu_to_be32(0);
	}

	/* don't check freespace btrees -- will be checked by caller */

	return(retval);
}

int
verify_set_agi(xfs_mount_t *mp, xfs_agi_t *agi, xfs_agnumber_t agno)
{
	xfs_drfsbno_t agblocks;
	int retval = 0;

	/* check common fields */

	if (be32_to_cpu(agi->agi_magicnum) != XFS_AGI_MAGIC)  {
		retval = XR_AG_AGI;
		do_warn(_("bad magic # 0x%x for agi %d\n"),
			be32_to_cpu(agi->agi_magicnum), agno);

		if (!no_modify)
			agi->agi_magicnum = cpu_to_be32(XFS_AGI_MAGIC);
	}

	if (!XFS_AGI_GOOD_VERSION(be32_to_cpu(agi->agi_versionnum)))  {
		retval = XR_AG_AGI;
		do_warn(_("bad version # %d for agi %d\n"),
			be32_to_cpu(agi->agi_versionnum), agno);

		if (!no_modify)
			agi->agi_versionnum = cpu_to_be32(XFS_AGI_VERSION);
	}

	if (be32_to_cpu(agi->agi_seqno) != agno)  {
		retval = XR_AG_AGI;
		do_warn(_("bad sequence # %d for agi %d\n"),
			be32_to_cpu(agi->agi_seqno), agno);

		if (!no_modify)
			agi->agi_seqno = cpu_to_be32(agno);
	}

	if (be32_to_cpu(agi->agi_length) != mp->m_sb.sb_agblocks)  {
		if (agno != mp->m_sb.sb_agcount - 1)  {
			retval = XR_AG_AGI;
			do_warn(_("bad length # %d for agi %d, should be %d\n"),
				be32_to_cpu(agi->agi_length), agno,
					mp->m_sb.sb_agblocks);
			if (!no_modify)
				agi->agi_length = 
					cpu_to_be32(mp->m_sb.sb_agblocks);
		} else  {
			agblocks = mp->m_sb.sb_dblocks -
				(xfs_drfsbno_t) mp->m_sb.sb_agblocks * agno;

			if (be32_to_cpu(agi->agi_length) != agblocks)  {
				retval = XR_AG_AGI;
				do_warn(
	_("bad length # %d for agi %d, should be %" PRIu64 "\n"),
					be32_to_cpu(agi->agi_length),
						agno, agblocks);
				if (!no_modify)
					agi->agi_length = cpu_to_be32(agblocks);
			}
		}
	}

	/* don't check inode btree -- will be checked by caller */

	return(retval);
}

/*
 * superblock comparison - compare arbitrary superblock with
 *			filesystem mount-point superblock
 *
 * the verified fields include id and geometry.

 * the inprogress fields, version numbers, and counters
 * are allowed to differ as well as all fields after the
 * counters to cope with the pre-6.5 mkfs non-zeroed
 * secondary superblock sectors.
 */

int
compare_sb(xfs_mount_t *mp, xfs_sb_t *sb)
{
	fs_geometry_t fs_geo, sb_geo;

	get_sb_geometry(&fs_geo, &mp->m_sb);
	get_sb_geometry(&sb_geo, sb);

	if (memcmp(&fs_geo, &sb_geo,
		   (char *) &fs_geo.sb_shared_vn - (char *) &fs_geo))
		return(XR_SB_GEO_MISMATCH);

	return(XR_OK);
}

/*
 * Possible fields that may have been set at mkfs time,
 * sb_inoalignmt, sb_unit, sb_width and sb_dirblklog.
 * The quota inode fields in the secondaries should be zero.
 * Likewise, the sb_flags and sb_shared_vn should also be
 * zero and the shared version bit should be cleared for
 * current mkfs's.
 *
 * And everything else in the buffer beyond either sb_width,
 * sb_dirblklog (v2 dirs), or sb_logsectsize can be zeroed.
 *
 * Note: contrary to the name, this routine is called for all
 * superblocks, not just the secondary superblocks.
 */
int
secondary_sb_wack(xfs_mount_t *mp, xfs_buf_t *sbuf, xfs_sb_t *sb,
	xfs_agnumber_t i)
{
	int do_bzero;
	int size;
	char *ip;
	int rval;

	rval = do_bzero = 0;

	/*
	 * mkfs's that stamped a feature bit besides the ones in the mask
	 * (e.g. were pre-6.5 beta) could leave garbage in the secondary
	 * superblock sectors.  Anything stamping the shared fs bit or better
	 * into the secondaries is ok and should generate clean secondary
	 * superblock sectors.  so only run the zero check on the
	 * potentially garbaged secondaries.
	 */
	if (pre_65_beta ||
	    (sb->sb_versionnum & XR_GOOD_SECSB_VNMASK) == 0 ||
	    sb->sb_versionnum < XFS_SB_VERSION_4)  {
		/*
		 * Check for garbage beyond the last field.
		 * Use field addresses instead so this code will still
		 * work against older filesystems when the superblock
		 * gets rev'ed again with new fields appended.
		 */
		if (xfs_sb_version_hasmorebits(sb))
			size = (__psint_t)&sb->sb_features2
				+ sizeof(sb->sb_features2) - (__psint_t)sb;
		else if (xfs_sb_version_haslogv2(sb))
			size = (__psint_t)&sb->sb_logsunit
				+ sizeof(sb->sb_logsunit) - (__psint_t)sb;
		else if (xfs_sb_version_hassector(sb))
			size = (__psint_t)&sb->sb_logsectsize
				+ sizeof(sb->sb_logsectsize) - (__psint_t)sb;
		else if (xfs_sb_version_hasdirv2(sb))
			size = (__psint_t)&sb->sb_dirblklog
				+ sizeof(sb->sb_dirblklog) - (__psint_t)sb;
		else
			size = (__psint_t)&sb->sb_width
				+ sizeof(sb->sb_width) - (__psint_t)sb;
		for (ip = (char *)((__psint_t)sb + size);
		     ip < (char *)((__psint_t)sb + mp->m_sb.sb_sectsize);
		     ip++)  {
			if (*ip)  {
				do_bzero = 1;
				break;
			}
		}

		if (do_bzero)  {
			rval |= XR_AG_SB_SEC;
			if (!no_modify)  {
				do_warn(
		_("zeroing unused portion of %s superblock (AG #%u)\n"),
					!i ? _("primary") : _("secondary"), i);
				memset((void *)((__psint_t)sb + size), 0,
					mp->m_sb.sb_sectsize - size);
			} else
				do_warn(
		_("would zero unused portion of %s superblock (AG #%u)\n"),
					!i ? _("primary") : _("secondary"), i);
		}
	}

	/*
	 * now look for the fields we can manipulate directly.
	 * if we did a zero and that zero could have included
	 * the field in question, just silently reset it.  otherwise,
	 * complain.
	 *
	 * for now, just zero the flags field since only
	 * the readonly flag is used
	 */
	if (sb->sb_flags)  {
		if (!no_modify)
			sb->sb_flags = 0;
		if (sb->sb_versionnum & XR_PART_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(_("bad flags field in superblock %d\n"), i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	/*
	 * quota inodes and flags in secondary superblocks
	 * are never set by mkfs.  However, they could be set
	 * in a secondary if a fs with quotas was growfs'ed since
	 * growfs copies the new primary into the secondaries.
	 */
	if (sb->sb_inprogress == 1 && sb->sb_uquotino)  {
		if (!no_modify)
			sb->sb_uquotino = 0;
		if (sb->sb_versionnum & XR_PART_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("non-null user quota inode field in superblock %d\n"),
				i);

		} else
			rval |= XR_AG_SB_SEC;
	}

	if (sb->sb_inprogress == 1 && sb->sb_gquotino)  {
		if (!no_modify)
			sb->sb_gquotino = 0;
		if (sb->sb_versionnum & XR_PART_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("non-null group quota inode field in superblock %d\n"),
				i);

		} else
			rval |= XR_AG_SB_SEC;
	}

	if (sb->sb_inprogress == 1 && sb->sb_qflags)  {
		if (!no_modify)
			sb->sb_qflags = 0;
		if (sb->sb_versionnum & XR_PART_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(_("non-null quota flags in superblock %d\n"),
				i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	/*
	 * if the secondaries agree on a stripe unit/width or inode
	 * alignment, those fields ought to be valid since they are
	 * written at mkfs time (and the corresponding sb version bits
	 * are set).
	 */
	if (!xfs_sb_version_hasshared(sb) && sb->sb_shared_vn != 0)  {
		if (!no_modify)
			sb->sb_shared_vn = 0;
		if (sb->sb_versionnum & XR_PART_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("bad shared version number in superblock %d\n"),
				i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	if (!xfs_sb_version_hasalign(sb) && sb->sb_inoalignmt != 0)  {
		if (!no_modify)
			sb->sb_inoalignmt = 0;
		if (sb->sb_versionnum & XR_PART_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("bad inode alignment field in superblock %d\n"),
				i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	if (!xfs_sb_version_hasdalign(sb) &&
	    (sb->sb_unit != 0 || sb->sb_width != 0))  {
		if (!no_modify)
			sb->sb_unit = sb->sb_width = 0;
		if (sb->sb_versionnum & XR_GOOD_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("bad stripe unit/width fields in superblock %d\n"),
				i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	if (!xfs_sb_version_hassector(sb) &&
	    (sb->sb_sectsize != BBSIZE || sb->sb_sectlog != BBSHIFT ||
	     sb->sb_logsectsize != 0 || sb->sb_logsectlog != 0))  {
		if (!no_modify)  {
			sb->sb_sectsize = BBSIZE;
			sb->sb_sectlog = BBSHIFT;
			sb->sb_logsectsize = 0;
			sb->sb_logsectlog = 0;
		}
		if (sb->sb_versionnum & XR_GOOD_SECSB_VNMASK || !do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("bad log/data device sector size fields in superblock %d\n"),
				i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	return(rval);
}

/*
 * verify and reset the ag header if required.
 *
 * lower 4 bits of rval are set depending on what got modified.
 * (see agheader.h for more details)
 *
 * NOTE -- this routine does not tell the user that it has
 * altered things.  Rather, it is up to the caller to do so
 * using the bits encoded into the return value.
 */

int
verify_set_agheader(xfs_mount_t *mp, xfs_buf_t *sbuf, xfs_sb_t *sb,
	xfs_agf_t *agf, xfs_agi_t *agi, xfs_agnumber_t i)
{
	int rval = 0;
	int status = XR_OK;
	int status_sb = XR_OK;

	status = verify_sb(sb, (i == 0));

	if (status != XR_OK)  {
		do_warn(_("bad on-disk superblock %d - %s\n"),
			i, err_string(status));
	}

	status_sb = compare_sb(mp, sb);

	if (status_sb != XR_OK)  {
		do_warn(_("primary/secondary superblock %d conflict - %s\n"),
			i, err_string(status_sb));
	}

	if (status != XR_OK || status_sb != XR_OK)  {
		if (!no_modify)  {
			*sb = mp->m_sb;

			/*
			 * clear the more transient fields
			 */
			sb->sb_inprogress = 1;

			sb->sb_icount = 0;
			sb->sb_ifree = 0;
			sb->sb_fdblocks = 0;
			sb->sb_frextents = 0;

			sb->sb_qflags = 0;
		}

		rval |= XR_AG_SB;
	}

	rval |= secondary_sb_wack(mp, sbuf, sb, i);

	rval |= verify_set_agf(mp, agf, i);
	rval |= verify_set_agi(mp, agi, i);

	return(rval);
}
