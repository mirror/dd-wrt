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

static const struct {
	short offset;
	short type;	/* 0 = integer
			 * 1 = binary / string (no translation)
			 */
} xfs_sb_info[] = {
    { offsetof(xfs_sb_t, sb_magicnum),   0 },
    { offsetof(xfs_sb_t, sb_blocksize),  0 },
    { offsetof(xfs_sb_t, sb_dblocks),    0 },
    { offsetof(xfs_sb_t, sb_rblocks),    0 },
    { offsetof(xfs_sb_t, sb_rextents),   0 },
    { offsetof(xfs_sb_t, sb_uuid),       1 },
    { offsetof(xfs_sb_t, sb_logstart),   0 },
    { offsetof(xfs_sb_t, sb_rootino),    0 },
    { offsetof(xfs_sb_t, sb_rbmino),     0 },
    { offsetof(xfs_sb_t, sb_rsumino),    0 },
    { offsetof(xfs_sb_t, sb_rextsize),   0 },
    { offsetof(xfs_sb_t, sb_agblocks),   0 },
    { offsetof(xfs_sb_t, sb_agcount),    0 },
    { offsetof(xfs_sb_t, sb_rbmblocks),  0 },
    { offsetof(xfs_sb_t, sb_logblocks),  0 },
    { offsetof(xfs_sb_t, sb_versionnum), 0 },
    { offsetof(xfs_sb_t, sb_sectsize),   0 },
    { offsetof(xfs_sb_t, sb_inodesize),  0 },
    { offsetof(xfs_sb_t, sb_inopblock),  0 },
    { offsetof(xfs_sb_t, sb_fname[0]),   1 },
    { offsetof(xfs_sb_t, sb_blocklog),   0 },
    { offsetof(xfs_sb_t, sb_sectlog),    0 },
    { offsetof(xfs_sb_t, sb_inodelog),   0 },
    { offsetof(xfs_sb_t, sb_inopblog),   0 },
    { offsetof(xfs_sb_t, sb_agblklog),   0 },
    { offsetof(xfs_sb_t, sb_rextslog),   0 },
    { offsetof(xfs_sb_t, sb_inprogress), 0 },
    { offsetof(xfs_sb_t, sb_imax_pct),   0 },
    { offsetof(xfs_sb_t, sb_icount),     0 },
    { offsetof(xfs_sb_t, sb_ifree),      0 },
    { offsetof(xfs_sb_t, sb_fdblocks),   0 },
    { offsetof(xfs_sb_t, sb_frextents),  0 },
    { offsetof(xfs_sb_t, sb_uquotino),   0 },
    { offsetof(xfs_sb_t, sb_gquotino),   0 },
    { offsetof(xfs_sb_t, sb_qflags),     0 },
    { offsetof(xfs_sb_t, sb_flags),      0 },
    { offsetof(xfs_sb_t, sb_shared_vn),  0 },
    { offsetof(xfs_sb_t, sb_inoalignmt), 0 },
    { offsetof(xfs_sb_t, sb_unit),	 0 },
    { offsetof(xfs_sb_t, sb_width),	 0 },
    { offsetof(xfs_sb_t, sb_dirblklog),	 0 },
    { offsetof(xfs_sb_t, sb_logsectlog), 0 },
    { offsetof(xfs_sb_t, sb_logsectsize),0 },
    { offsetof(xfs_sb_t, sb_logsunit),	 0 },
    { offsetof(xfs_sb_t, sb_features2),	 0 },
    { offsetof(xfs_sb_t, sb_bad_features2), 0 },
    { sizeof(xfs_sb_t),			 0 }
};

/*
 * Reference counting access wrappers to the perag structures.
 * Because we never free per-ag structures, the only thing we
 * have to protect against changes is the tree structure itself.
 */
struct xfs_perag *
xfs_perag_get(struct xfs_mount *mp, xfs_agnumber_t agno)
{
	struct xfs_perag	*pag;
	int			ref = 0;

	rcu_read_lock();
	pag = radix_tree_lookup(&mp->m_perag_tree, agno);
	if (pag) {
		ASSERT(atomic_read(&pag->pag_ref) >= 0);
		ref = atomic_inc_return(&pag->pag_ref);
	}
	trace_xfs_perag_get(mp, agno, ref, _RET_IP_);
	rcu_read_unlock();
	return pag;
}

void
xfs_perag_put(struct xfs_perag *pag)
{
	int	ref;

	ASSERT(atomic_read(&pag->pag_ref) > 0);
	ref = atomic_dec_return(&pag->pag_ref);
	trace_xfs_perag_put(pag->pag_mount, pag->pag_agno, ref, _RET_IP_);
}

void
xfs_sb_from_disk(
	xfs_sb_t	*to,
	xfs_dsb_t	*from)
{
	to->sb_magicnum = be32_to_cpu(from->sb_magicnum);
	to->sb_blocksize = be32_to_cpu(from->sb_blocksize);
	to->sb_dblocks = be64_to_cpu(from->sb_dblocks);
	to->sb_rblocks = be64_to_cpu(from->sb_rblocks);
	to->sb_rextents = be64_to_cpu(from->sb_rextents);
	memcpy(&to->sb_uuid, &from->sb_uuid, sizeof(to->sb_uuid));
	to->sb_logstart = be64_to_cpu(from->sb_logstart);
	to->sb_rootino = be64_to_cpu(from->sb_rootino);
	to->sb_rbmino = be64_to_cpu(from->sb_rbmino);
	to->sb_rsumino = be64_to_cpu(from->sb_rsumino);
	to->sb_rextsize = be32_to_cpu(from->sb_rextsize);
	to->sb_agblocks = be32_to_cpu(from->sb_agblocks);
	to->sb_agcount = be32_to_cpu(from->sb_agcount);
	to->sb_rbmblocks = be32_to_cpu(from->sb_rbmblocks);
	to->sb_logblocks = be32_to_cpu(from->sb_logblocks);
	to->sb_versionnum = be16_to_cpu(from->sb_versionnum);
	to->sb_sectsize = be16_to_cpu(from->sb_sectsize);
	to->sb_inodesize = be16_to_cpu(from->sb_inodesize);
	to->sb_inopblock = be16_to_cpu(from->sb_inopblock);
	memcpy(&to->sb_fname, &from->sb_fname, sizeof(to->sb_fname));
	to->sb_blocklog = from->sb_blocklog;
	to->sb_sectlog = from->sb_sectlog;
	to->sb_inodelog = from->sb_inodelog;
	to->sb_inopblog = from->sb_inopblog;
	to->sb_agblklog = from->sb_agblklog;
	to->sb_rextslog = from->sb_rextslog;
	to->sb_inprogress = from->sb_inprogress;
	to->sb_imax_pct = from->sb_imax_pct;
	to->sb_icount = be64_to_cpu(from->sb_icount);
	to->sb_ifree = be64_to_cpu(from->sb_ifree);
	to->sb_fdblocks = be64_to_cpu(from->sb_fdblocks);
	to->sb_frextents = be64_to_cpu(from->sb_frextents);
	to->sb_uquotino = be64_to_cpu(from->sb_uquotino);
	to->sb_gquotino = be64_to_cpu(from->sb_gquotino);
	to->sb_qflags = be16_to_cpu(from->sb_qflags);
	to->sb_flags = from->sb_flags;
	to->sb_shared_vn = from->sb_shared_vn;
	to->sb_inoalignmt = be32_to_cpu(from->sb_inoalignmt);
	to->sb_unit = be32_to_cpu(from->sb_unit);
	to->sb_width = be32_to_cpu(from->sb_width);
	to->sb_dirblklog = from->sb_dirblklog;
	to->sb_logsectlog = from->sb_logsectlog;
	to->sb_logsectsize = be16_to_cpu(from->sb_logsectsize);
	to->sb_logsunit = be32_to_cpu(from->sb_logsunit);
	to->sb_features2 = be32_to_cpu(from->sb_features2);
	to->sb_bad_features2 = be32_to_cpu(from->sb_bad_features2);
}

/*
 * Copy in core superblock to ondisk one.
 *
 * The fields argument is mask of superblock fields to copy.
 */
void
xfs_sb_to_disk(
	xfs_dsb_t	*to,
	xfs_sb_t	*from,
	__int64_t	fields)
{
	xfs_caddr_t	to_ptr = (xfs_caddr_t)to;
	xfs_caddr_t	from_ptr = (xfs_caddr_t)from;
	xfs_sb_field_t	f;
	int		first;
	int		size;

	ASSERT(fields);
	if (!fields)
		return;

	while (fields) {
		f = (xfs_sb_field_t)xfs_lowbit64((__uint64_t)fields);
		first = xfs_sb_info[f].offset;
		size = xfs_sb_info[f + 1].offset - first;

		ASSERT(xfs_sb_info[f].type == 0 || xfs_sb_info[f].type == 1);

		if (size == 1 || xfs_sb_info[f].type == 1) {
			memcpy(to_ptr + first, from_ptr + first, size);
		} else {
			switch (size) {
			case 2:
				*(__be16 *)(to_ptr + first) =
					cpu_to_be16(*(__u16 *)(from_ptr + first));
				break;
			case 4:
				*(__be32 *)(to_ptr + first) =
					cpu_to_be32(*(__u32 *)(from_ptr + first));
				break;
			case 8:
				*(__be64 *)(to_ptr + first) =
					cpu_to_be64(*(__u64 *)(from_ptr + first));
				break;
			default:
				ASSERT(0);
			}
		}

		fields &= ~(1LL << f);
	}
}

/*
 * xfs_mount_common
 *
 * Mount initialization code establishing various mount
 * fields from the superblock associated with the given
 * mount structure
 *
 * Note: this requires user-space public scope for libxfs_mount
 */
void
xfs_mount_common(xfs_mount_t *mp, xfs_sb_t *sbp)
{
	mp->m_agfrotor = mp->m_agirotor = 0;
	spin_lock_init(&mp->m_agirotor_lock);
	mp->m_maxagi = mp->m_sb.sb_agcount;
	mp->m_blkbit_log = sbp->sb_blocklog + XFS_NBBYLOG;
	mp->m_blkbb_log = sbp->sb_blocklog - BBSHIFT;
	mp->m_sectbb_log = sbp->sb_sectlog - BBSHIFT;
	mp->m_agno_log = xfs_highbit32(sbp->sb_agcount - 1) + 1;
	mp->m_agino_log = sbp->sb_inopblog + sbp->sb_agblklog;
	mp->m_blockmask = sbp->sb_blocksize - 1;
	mp->m_blockwsize = sbp->sb_blocksize >> XFS_WORDLOG;
	mp->m_blockwmask = mp->m_blockwsize - 1;

	mp->m_alloc_mxr[0] = xfs_allocbt_maxrecs(mp, sbp->sb_blocksize, 1);
	mp->m_alloc_mxr[1] = xfs_allocbt_maxrecs(mp, sbp->sb_blocksize, 0);
	mp->m_alloc_mnr[0] = mp->m_alloc_mxr[0] / 2;
	mp->m_alloc_mnr[1] = mp->m_alloc_mxr[1] / 2;

	mp->m_inobt_mxr[0] = xfs_inobt_maxrecs(mp, sbp->sb_blocksize, 1);
	mp->m_inobt_mxr[1] = xfs_inobt_maxrecs(mp, sbp->sb_blocksize, 0);
	mp->m_inobt_mnr[0] = mp->m_inobt_mxr[0] / 2;
	mp->m_inobt_mnr[1] = mp->m_inobt_mxr[1] / 2;

	mp->m_bmap_dmxr[0] = xfs_bmbt_maxrecs(mp, sbp->sb_blocksize, 1);
	mp->m_bmap_dmxr[1] = xfs_bmbt_maxrecs(mp, sbp->sb_blocksize, 0);
	mp->m_bmap_dmnr[0] = mp->m_bmap_dmxr[0] / 2;
	mp->m_bmap_dmnr[1] = mp->m_bmap_dmxr[1] / 2;

	mp->m_bsize = XFS_FSB_TO_BB(mp, 1);
	mp->m_ialloc_inos = (int)MAX((__uint16_t)XFS_INODES_PER_CHUNK,
					sbp->sb_inopblock);
	mp->m_ialloc_blks = mp->m_ialloc_inos >> sbp->sb_inopblog;
}

/*
 * xfs_initialize_perag_data
 *
 * Read in each per-ag structure so we can count up the number of
 * allocated inodes, free inodes and used filesystem blocks as this
 * information is no longer persistent in the superblock. Once we have
 * this information, write it into the in-core superblock structure.
 *
 * Note: this requires user-space public scope for libxfs_mount
 */
int
xfs_initialize_perag_data(xfs_mount_t *mp, xfs_agnumber_t agcount)
{
	xfs_agnumber_t	index;
	xfs_perag_t	*pag;
	xfs_sb_t	*sbp = &mp->m_sb;
	uint64_t	ifree = 0;
	uint64_t	ialloc = 0;
	uint64_t	bfree = 0;
	uint64_t	bfreelst = 0;
	uint64_t	btree = 0;
	int		error;

	for (index = 0; index < agcount; index++) {
		/*
		 * read the agf, then the agi. This gets us
		 * all the information we need and populates the
		 * per-ag structures for us.
		 */
		error = xfs_alloc_pagf_init(mp, NULL, index, 0);
		if (error)
			return error;

		error = xfs_ialloc_pagi_init(mp, NULL, index);
		if (error)
			return error;
		pag = xfs_perag_get(mp, index);
		ifree += pag->pagi_freecount;
		ialloc += pag->pagi_count;
		bfree += pag->pagf_freeblks;
		bfreelst += pag->pagf_flcount;
		btree += pag->pagf_btreeblks;
		xfs_perag_put(pag);
	}
	/*
	 * Overwrite incore superblock counters with just-read data
	 */
	spin_lock(&mp->m_sb_lock);
	sbp->sb_ifree = ifree;
	sbp->sb_icount = ialloc;
	sbp->sb_fdblocks = bfree + bfreelst + btree;
	spin_unlock(&mp->m_sb_lock);

	/* Fixup the per-cpu counters as well. */
	xfs_icsb_reinit_counters(mp);

	return 0;
}

/*
 * xfs_mod_sb() can be used to copy arbitrary changes to the
 * in-core superblock into the superblock buffer to be logged.
 * It does not provide the higher level of locking that is
 * needed to protect the in-core superblock from concurrent
 * access.
 */
void
xfs_mod_sb(xfs_trans_t *tp, __int64_t fields)
{
	xfs_buf_t	*bp;
	int		first;
	int		last;
	xfs_mount_t	*mp;
	xfs_sb_field_t	f;

	ASSERT(fields);
	if (!fields)
		return;
	mp = tp->t_mountp;
	bp = xfs_trans_getsb(tp, mp, 0);
	first = sizeof(xfs_sb_t);
	last = 0;

	/* translate/copy */
	xfs_sb_to_disk(XFS_BUF_TO_SBP(bp), &mp->m_sb, fields);

	/* find modified range */
	f = (xfs_sb_field_t)xfs_highbit64((__uint64_t)fields);
	ASSERT((1LL << f) & XFS_SB_MOD_BITS);
	last = xfs_sb_info[f + 1].offset - 1;

	f = (xfs_sb_field_t)xfs_lowbit64((__uint64_t)fields);
	ASSERT((1LL << f) & XFS_SB_MOD_BITS);
	first = xfs_sb_info[f].offset;

	xfs_trans_log_buf(tp, bp, first, last);
}
