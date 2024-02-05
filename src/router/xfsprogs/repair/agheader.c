// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "globals.h"
#include "agheader.h"
#include "protos.h"
#include "err_protos.h"

/*
 * XXX (dgc): What is the point of all the check and repair here when phase 5
 * recreates the AGF/AGI/AGFL completely from scratch?
 */

static int
verify_set_agf(xfs_mount_t *mp, xfs_agf_t *agf, xfs_agnumber_t i)
{
	xfs_rfsblock_t agblocks;
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
				(xfs_rfsblock_t) mp->m_sb.sb_agblocks * i;

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
	if (be32_to_cpu(agf->agf_flfirst) >= libxfs_agfl_size(mp)) {
		do_warn(_("flfirst %d in agf %d too large (max = %u)\n"),
			be32_to_cpu(agf->agf_flfirst),
			i, libxfs_agfl_size(mp) - 1);
		if (!no_modify)
			agf->agf_flfirst = cpu_to_be32(0);
	}

	if (be32_to_cpu(agf->agf_fllast) >= libxfs_agfl_size(mp)) {
		do_warn(_("fllast %d in agf %d too large (max = %u)\n"),
			be32_to_cpu(agf->agf_fllast),
			i, libxfs_agfl_size(mp) - 1);
		if (!no_modify)
			agf->agf_fllast = cpu_to_be32(0);
	}

	/* don't check freespace btrees -- will be checked by caller */

	if (!xfs_has_crc(mp))
		return retval;

	if (platform_uuid_compare(&agf->agf_uuid, &mp->m_sb.sb_meta_uuid)) {
		char uu[64];

		retval = XR_AG_AGF;
		platform_uuid_unparse(&agf->agf_uuid, uu);
		do_warn(_("bad uuid %s for agf %d\n"), uu, i);

		if (!no_modify)
			platform_uuid_copy(&agf->agf_uuid,
					   &mp->m_sb.sb_meta_uuid);
	}
	return retval;
}

static int
verify_set_agi(xfs_mount_t *mp, xfs_agi_t *agi, xfs_agnumber_t agno)
{
	xfs_rfsblock_t agblocks;
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
				(xfs_rfsblock_t) mp->m_sb.sb_agblocks * agno;

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

	if (!xfs_has_crc(mp))
		return retval;

	if (platform_uuid_compare(&agi->agi_uuid, &mp->m_sb.sb_meta_uuid)) {
		char uu[64];

		retval = XR_AG_AGI;
		platform_uuid_unparse(&agi->agi_uuid, uu);
		do_warn(_("bad uuid %s for agi %d\n"), uu, agno);

		if (!no_modify)
			platform_uuid_copy(&agi->agi_uuid,
					   &mp->m_sb.sb_meta_uuid);
	}

	return retval;
}

/*
 * superblock comparison - compare arbitrary superblock with
 *			filesystem mount-point superblock
 *
 * the verified fields include id and geometry.
 *
 * the inprogress fields, version numbers, and counters
 * are allowed to differ as well as all fields after the
 * counters to cope with the pre-6.5 mkfs non-zeroed
 * secondary superblock sectors.
 */
static int
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
 * If the fs feature bits on a secondary superblock don't match the
 * primary, we need to update them.
 */
static inline int
check_v5_feature_mismatch(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct xfs_sb		*sb)
{
	bool			dirty = false;

	if (!xfs_has_crc(mp) || agno == 0)
		return 0;

	if (mp->m_sb.sb_features_compat != sb->sb_features_compat) {
		if (no_modify) {
			do_warn(
	_("would fix compat feature mismatch in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_compat,
					sb->sb_features_compat);
		} else {
			do_warn(
	_("will fix compat feature mismatch in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_compat,
					sb->sb_features_compat);
			dirty = true;
		}
	}

	/*
	 * Ignore XFS_SB_FEAT_INCOMPAT_NEEDSREPAIR becauses the repair upgrade
	 * path sets it only on the primary while upgrading.
	 */ 
	if ((mp->m_sb.sb_features_incompat ^ sb->sb_features_incompat) &
			~XFS_SB_FEAT_INCOMPAT_NEEDSREPAIR) {
		if (no_modify) {
			do_warn(
	_("would fix incompat feature mismatch in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_incompat,
					sb->sb_features_incompat);
		} else {
			do_warn(
	_("will fix incompat feature mismatch in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_incompat,
					sb->sb_features_incompat);
			dirty = true;
		}
	}

	if (mp->m_sb.sb_features_ro_compat != sb->sb_features_ro_compat) {
		if (no_modify) {
			do_warn(
	_("would fix ro compat feature mismatch in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_ro_compat,
					sb->sb_features_ro_compat);
		} else {
			do_warn(
	_("will fix ro compat feature mismatch in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_ro_compat,
					sb->sb_features_ro_compat);
			dirty = true;
		}
	}

	/*
	 * Log incompat feature bits are set and cleared from the primary super
	 * as needed to protect against log replay on old kernels finding log
	 * records that they cannot handle.  Secondary sb resyncs performed as
	 * part of a geometry update to the primary sb (e.g. growfs, label/uuid
	 * changes) will copy the log incompat feature bits, but it's not a
	 * corruption for a secondary to have a bit set that is clear in the
	 * primary super.
	 */
	if (mp->m_sb.sb_features_log_incompat != sb->sb_features_log_incompat) {
		if (no_modify) {
			do_log(
	_("would sync log incompat feature in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_log_incompat,
					sb->sb_features_log_incompat);
		} else {
			do_warn(
	_("will sync log incompat feature in AG %u super, 0x%x != 0x%x\n"),
					agno, mp->m_sb.sb_features_log_incompat,
					sb->sb_features_log_incompat);
			dirty = true;
		}
	}

	if (!dirty)
		return 0;

	sb->sb_features_compat = mp->m_sb.sb_features_compat;
	sb->sb_features_ro_compat = mp->m_sb.sb_features_ro_compat;
	sb->sb_features_incompat = mp->m_sb.sb_features_incompat;
	sb->sb_features_log_incompat = mp->m_sb.sb_features_log_incompat;
	return XR_AG_SB_SEC;
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
static int
secondary_sb_whack(
	struct xfs_mount *mp,
	struct xfs_buf	*sbuf,
	struct xfs_sb	*sb,
	xfs_agnumber_t	i)
{
	struct xfs_dsb	*dsb = sbuf->b_addr;
	int		do_bzero = 0;
	int		size;
	char		*ip;
	int		rval = 0;
	uuid_t		tmpuuid;

	rval = do_bzero = 0;

	/*
	 * Check for garbage beyond the last valid field.
	 * Use field addresses instead so this code will still
	 * work against older filesystems when the superblock
	 * gets rev'ed again with new fields appended.
	 *
	 * size is the size of data which is valid for this sb.
	 */
	if (xfs_sb_version_hasmetauuid(sb))
		size = offsetof(xfs_sb_t, sb_meta_uuid)
			+ sizeof(sb->sb_meta_uuid);
	else if (xfs_sb_version_hascrc(sb))
		size = offsetof(xfs_sb_t, sb_lsn)
			+ sizeof(sb->sb_lsn);
	else if (xfs_sb_version_hasmorebits(sb))
		size = offsetof(xfs_sb_t, sb_bad_features2)
			+ sizeof(sb->sb_bad_features2);
	else if (xfs_sb_version_haslogv2(sb))
		size = offsetof(xfs_sb_t, sb_logsunit)
			+ sizeof(sb->sb_logsunit);
	else if (xfs_sb_version_hassector(sb))
		size = offsetof(xfs_sb_t, sb_logsectsize)
			+ sizeof(sb->sb_logsectsize);
	else /* only support dirv2 or more recent */
		size = offsetof(xfs_sb_t, sb_dirblklog)
			+ sizeof(sb->sb_dirblklog);

	/* Check the buffer we read from disk for garbage outside size */
	for (ip = (char *)sbuf->b_addr + size;
	     ip < (char *)sbuf->b_addr + mp->m_sb.sb_sectsize;
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
			/*
			 * zero both the in-memory sb and the disk buffer,
			 * because the former was read from disk and
			 * may contain newer version fields that shouldn't
			 * be set, and the latter is never updated past
			 * the last field - just zap them both.
			 */
			memcpy(&tmpuuid, &sb->sb_meta_uuid, sizeof(uuid_t));
			memset((void *)((intptr_t)sb + size), 0,
				mp->m_sb.sb_sectsize - size);
			memset((char *)sbuf->b_addr + size, 0,
				mp->m_sb.sb_sectsize - size);
			/* Preserve meta_uuid so we don't fail uuid checks */
			memcpy(&sb->sb_meta_uuid, &tmpuuid, sizeof(uuid_t));
		} else
			do_warn(
	_("would zero unused portion of %s superblock (AG #%u)\n"),
				!i ? _("primary") : _("secondary"), i);
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
		if (!do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(_("bad flags field in superblock %d\n"), i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	/*
	 * quota inodes and flags in secondary superblocks are never set by
	 * mkfs.  However, they could be set in a secondary if a fs with quotas
	 * was growfs'ed since growfs copies the new primary into the
	 * secondaries.
	 *
	 * Also, the in-core inode flags now have different meaning to the
	 * on-disk flags, and so libxfs_sb_to_disk cannot directly write the
	 * sb_gquotino/sb_pquotino fields without specific sb_qflags being set.
	 * Hence we need to zero those fields directly in the sb buffer here.
	 */

	if (sb->sb_inprogress == 1 && sb->sb_uquotino != NULLFSINO)  {
		if (!no_modify)
			sb->sb_uquotino = 0;
		if (!do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("non-null user quota inode field in superblock %d\n"),
				i);

		} else
			rval |= XR_AG_SB_SEC;
	}

	if (sb->sb_inprogress == 1 && sb->sb_gquotino != NULLFSINO)  {
		if (!no_modify) {
			sb->sb_gquotino = 0;
			dsb->sb_gquotino = 0;
		}
		if (!do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("non-null group quota inode field in superblock %d\n"),
				i);

		} else
			rval |= XR_AG_SB_SEC;
	}

	/*
	 * Note that sb_pquotino is not considered a valid sb field for pre-v5
	 * superblocks. If it is anything other than 0 it is considered garbage
	 * data beyond the valid sb and explicitly zeroed above.
	 */
	if (xfs_has_pquotino(mp) &&
	    sb->sb_inprogress == 1 && sb->sb_pquotino != NULLFSINO)  {
		if (!no_modify) {
			sb->sb_pquotino = 0;
			dsb->sb_pquotino = 0;
		}
		if (!do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("non-null project quota inode field in superblock %d\n"),
				i);

		} else
			rval |= XR_AG_SB_SEC;
	}

	if (sb->sb_inprogress == 1 && sb->sb_qflags)  {
		if (!no_modify)
			sb->sb_qflags = 0;
		if (!do_bzero)  {
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
	if (!xfs_sb_version_hasalign(sb) && sb->sb_inoalignmt != 0)  {
		if (!no_modify)
			sb->sb_inoalignmt = 0;
		if (!do_bzero)  {
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
		if (!do_bzero)  {
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
		if (!do_bzero)  {
			rval |= XR_AG_SB;
			do_warn(
		_("bad log/data device sector size fields in superblock %d\n"),
				i);
		} else
			rval |= XR_AG_SB_SEC;
	}

	rval |= check_v5_feature_mismatch(mp, i, sb);

	if (xfs_sb_version_needsrepair(sb)) {
		if (i == 0) {
			if (!no_modify)
				do_warn(
	_("clearing needsrepair flag and regenerating metadata\n"));
			else
				do_warn(
	_("would clear needsrepair flag and regenerate metadata\n"));
			/*
			 * If needsrepair is set on the primary super, there's
			 * a possibility that repair crashed during an upgrade.
			 * Set features_changed to ensure that the secondary
			 * supers are rewritten with the new feature bits once
			 * we've finished the upgrade.
			 */
			features_changed = true;
		} else {
			/*
			 * Quietly clear needsrepair on the secondary supers as
			 * part of ensuring them.  If needsrepair is set on the
			 * primary, it will be cleared at the end of repair
			 * once we've flushed all other dirty blocks to disk.
			 */
			sb->sb_features_incompat &=
					~XFS_SB_FEAT_INCOMPAT_NEEDSREPAIR;
			rval |= XR_AG_SB_SEC;
		}
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
verify_set_agheader(xfs_mount_t *mp, struct xfs_buf *sbuf, xfs_sb_t *sb,
	xfs_agf_t *agf, xfs_agi_t *agi, xfs_agnumber_t i)
{
	int rval = 0;
	int status = XR_OK;
	int status_sb = XR_OK;

	status = verify_sb(sbuf->b_addr, sb, (i == 0));

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

	rval |= secondary_sb_whack(mp, sbuf, sb, i);

	rval |= verify_set_agf(mp, agf, i);
	rval |= verify_set_agi(mp, agi, i);

	return(rval);
}
