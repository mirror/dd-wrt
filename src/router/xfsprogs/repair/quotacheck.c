// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include <libxfs.h>
#include "globals.h"
#include "versions.h"
#include "err_protos.h"
#include "libfrog/avl64.h"
#include "quotacheck.h"

/* Allow the xfs_repair caller to skip quotacheck entirely. */
static bool noquota;

void quotacheck_skip(void)
{
	noquota = true;
}

/*
 * XFS_*QUOTA_CHKD flags for all the quota types that we verified.  This field
 * will be cleared if we encounter any problems (runtime errors, mismatches).
 */
static uint16_t chkd_flags;

/*
 * Return CHKD flags for the quota types that we checked.  If we encountered
 * any errors at all, return zero.
 */
uint16_t
quotacheck_results(void)
{
	return chkd_flags;
}

/* Global incore dquot tree */
struct qc_dquots {
	pthread_mutex_t		lock;
	struct avl64tree_desc	tree;

	/* One of XFS_DQTYPE_USER/PROJ/GROUP */
	xfs_dqtype_t		type;
};

#define qc_dquots_foreach(dquots, pos, n) \
	for (pos = (dquots)->tree.avl_firstino, n = pos ? pos->avl_nextino : NULL; \
			pos != NULL; \
			pos = n, n = pos ? pos->avl_nextino : NULL)

static struct qc_dquots *user_dquots;
static struct qc_dquots *group_dquots;
static struct qc_dquots *proj_dquots;

/* This record was found in the on-disk dquot information. */
#define QC_REC_ONDISK		(1U << 31)

struct qc_rec {
	struct avl64node	node;
	pthread_mutex_t		lock;

	xfs_dqid_t		id;
	uint32_t		flags;
	uint64_t		bcount;
	uint64_t		rtbcount;
	uint64_t		icount;
};

static const char *
qflags_typestr(
	xfs_dqtype_t		type)
{
	if (type & XFS_DQTYPE_USER)
		return _("user quota");
	else if (type & XFS_DQTYPE_GROUP)
		return _("group quota");
	else if (type & XFS_DQTYPE_PROJ)
		return _("project quota");
	return NULL;
}

/* Operations for the avl64 tree. */

static uint64_t
qc_avl_start(
	struct avl64node	*node)
{
	struct qc_rec		*qrec;

	qrec = container_of(node, struct qc_rec, node);
	return qrec->id;
}

static uint64_t
qc_avl_end(
	struct avl64node	*node)
{
	return qc_avl_start(node) + 1;
}

static struct avl64ops qc_cache_ops = {
	.avl_start		= qc_avl_start,
	.avl_end		= qc_avl_end,
};

/* Find a qc_rec in the incore cache, or allocate one if need be. */
static struct qc_rec *
qc_rec_get(
	struct qc_dquots	*dquots,
	xfs_dqid_t		id,
	bool			can_alloc)
{
	struct qc_rec		*qrec;
	struct avl64node	*node;

	pthread_mutex_lock(&dquots->lock);
	node = avl64_find(&dquots->tree, id);
	if (!node && can_alloc) {
		qrec = calloc(sizeof(struct qc_rec), 1);
		if (qrec) {
			qrec->id = id;
			node = avl64_insert(&dquots->tree, &qrec->node);
			if (!node)
				free(qrec);
			else
				pthread_mutex_init(&qrec->lock, NULL);
		}
	}
	pthread_mutex_unlock(&dquots->lock);

	return node ? container_of(node, struct qc_rec, node) : NULL;
}

/* Bump up an incore dquot's counters. */
static void
qc_adjust(
	struct qc_dquots	*dquots,
	xfs_dqid_t		id,
	uint64_t		bcount,
	uint64_t		rtbcount)
{
	struct qc_rec		*qrec = qc_rec_get(dquots, id, true);

	if (!qrec) {
		do_warn(_("Ran out of memory while running quotacheck!\n"));
		chkd_flags = 0;
		return;
	}

	pthread_mutex_lock(&qrec->lock);
	qrec->bcount += bcount;
	qrec->rtbcount += rtbcount;
	qrec->icount++;
	pthread_mutex_unlock(&qrec->lock);
}

/* Count the realtime blocks allocated to a file. */
static xfs_filblks_t
qc_count_rtblocks(
	struct xfs_inode	*ip)
{
	struct xfs_iext_cursor	icur;
	struct xfs_bmbt_irec	got;
	xfs_filblks_t		count = 0;
	struct xfs_ifork	*ifp = xfs_ifork_ptr(ip, XFS_DATA_FORK);
	int			error;

	error = -libxfs_iread_extents(NULL, ip, XFS_DATA_FORK);
	if (error) {
		do_warn(
_("could not read ino %"PRIu64" extents, err=%d\n"),
			ip->i_ino, error);
		chkd_flags = 0;
		return 0;
	}

	for_each_xfs_iext(ifp, &icur, &got)
		if (!isnullstartblock(got.br_startblock))
			count += got.br_blockcount;
	return count;
}

/* Add this inode's information to the quota counts. */
void
quotacheck_adjust(
	struct xfs_mount	*mp,
	xfs_ino_t		ino)
{
	struct xfs_inode	*ip;
	uint64_t		blocks;
	uint64_t		rtblks = 0;
	int			error;

	/*
	 * If the fs doesn't have any quota files to check against, skip this
	 * step.
	 */
	if (!user_dquots && !group_dquots && !proj_dquots)
		return;

	/* Skip if a previous quotacheck adjustment failed. */
	if (chkd_flags == 0)
		return;

	/* Quota files are not included in quota counts. */
	if (ino == mp->m_sb.sb_uquotino ||
	    ino == mp->m_sb.sb_gquotino ||
	    ino == mp->m_sb.sb_pquotino)
		return;

	error = -libxfs_iget(mp, NULL, ino, 0, &ip);
	if (error) {
		do_warn(
	_("could not open file %"PRIu64" for quotacheck, err=%d\n"),
				ino, error);
		chkd_flags = 0;
		return;
	}

	/* Count the file's blocks. */
	if (XFS_IS_REALTIME_INODE(ip))
		rtblks = qc_count_rtblocks(ip);
	blocks = ip->i_nblocks - rtblks;

	if (user_dquots)
		qc_adjust(user_dquots, i_uid_read(VFS_I(ip)), blocks, rtblks);
	if (group_dquots)
		qc_adjust(group_dquots, i_gid_read(VFS_I(ip)), blocks, rtblks);
	if (proj_dquots)
		qc_adjust(proj_dquots, ip->i_projid, blocks, rtblks);

	libxfs_irele(ip);
}

/* Check the ondisk dquot's id and type match what the incore dquot expects. */
static bool
qc_dquot_check_type(
	struct xfs_mount	*mp,
	xfs_dqtype_t		type,
	xfs_dqid_t		id,
	struct xfs_disk_dquot	*ddq)
{
	uint8_t			ddq_type;

	ddq_type = ddq->d_type & XFS_DQTYPE_REC_MASK;

	if (be32_to_cpu(ddq->d_id) != id)
		return false;

	/*
	 * V5 filesystems always expect an exact type match.  V4 filesystems
	 * expect an exact match for user dquots and for non-root group and
	 * project dquots.
	 */
	if (xfs_has_crc(mp) || type == XFS_DQTYPE_USER || id)
		return ddq_type == type;

	/*
	 * V4 filesystems support either group or project quotas, but not both
	 * at the same time.  The non-user quota file can be switched between
	 * group and project quota uses depending on the mount options, which
	 * means that we can encounter the other type when we try to load quota
	 * defaults.  Quotacheck will soon reset the the entire quota file
	 * (including the root dquot) anyway, but don't log scary corruption
	 * reports to dmesg.
	 */
	return ddq_type == XFS_DQTYPE_GROUP || ddq_type == XFS_DQTYPE_PROJ;
}

/* Compare this on-disk dquot against whatever we observed. */
static void
qc_check_dquot(
	struct xfs_mount	*mp,
	struct xfs_disk_dquot	*ddq,
	struct qc_dquots	*dquots,
	xfs_dqid_t		dqid)
{
	struct qc_rec		*qrec;
	struct qc_rec		empty = {
		.bcount		= 0,
		.rtbcount	= 0,
		.icount		= 0,
	};
	xfs_dqid_t		id = be32_to_cpu(ddq->d_id);

	qrec = qc_rec_get(dquots, id, false);
	if (!qrec)
		qrec = &empty;

	if (!qc_dquot_check_type(mp, dquots->type, dqid, ddq)) {
		const char	*dqtypestr;

		dqtypestr = qflags_typestr(ddq->d_type & XFS_DQTYPE_REC_MASK);
		if (dqtypestr)
			do_warn(_("%s id %u saw type %s id %u\n"),
					qflags_typestr(dquots->type), dqid,
					dqtypestr, be32_to_cpu(ddq->d_id));
		else
			do_warn(_("%s id %u saw type %x id %u\n"),
					qflags_typestr(dquots->type), dqid,
					ddq->d_type & XFS_DQTYPE_REC_MASK,
					be32_to_cpu(ddq->d_id));
		chkd_flags = 0;
	}

	if (be64_to_cpu(ddq->d_bcount) != qrec->bcount) {
		do_warn(_("%s id %u has bcount %llu, expected %"PRIu64"\n"),
				qflags_typestr(dquots->type), id,
				(unsigned long long)be64_to_cpu(ddq->d_bcount),
				qrec->bcount);
		chkd_flags = 0;
	}

	if (be64_to_cpu(ddq->d_rtbcount) != qrec->rtbcount) {
		do_warn(_("%s id %u has rtbcount %llu, expected %"PRIu64"\n"),
				qflags_typestr(dquots->type), id,
				(unsigned long long)be64_to_cpu(ddq->d_rtbcount),
				qrec->rtbcount);
		chkd_flags = 0;
	}

	if (be64_to_cpu(ddq->d_icount) != qrec->icount) {
		do_warn(_("%s id %u has icount %llu, expected %"PRIu64"\n"),
				qflags_typestr(dquots->type), id,
				(unsigned long long)be64_to_cpu(ddq->d_icount),
				qrec->icount);
		chkd_flags = 0;
	}

	if ((ddq->d_type & XFS_DQTYPE_BIGTIME) &&
	    !xfs_has_bigtime(mp)) {
		do_warn(
	_("%s id %u is marked bigtime but file system does not support large timestamps\n"),
				qflags_typestr(dquots->type), id);
		chkd_flags = 0;
	}

	/*
	 * Mark that we found the record on disk.  Skip locking here because
	 * we're checking the dquots serially.
	 */
	qrec->flags |= QC_REC_ONDISK;
}

/* Walk every dquot in every block in this quota inode extent and compare. */
static int
qc_walk_dquot_extent(
	struct xfs_inode	*ip,
	struct xfs_bmbt_irec	*map,
	struct qc_dquots	*dquots)
{
	struct xfs_mount	*mp = ip->i_mount;
	struct xfs_buf		*bp;
	struct xfs_dqblk	*dqb;
	xfs_filblks_t		dqchunklen;
	xfs_filblks_t		bno;
	unsigned int		dqperchunk;
	int			error = 0;

	dqchunklen = XFS_FSB_TO_BB(mp, XFS_DQUOT_CLUSTER_SIZE_FSB);
	dqperchunk = libxfs_calc_dquots_per_chunk(dqchunklen);

	for (bno = 0;
	     bno < map->br_blockcount;
	     bno += XFS_DQUOT_CLUSTER_SIZE_FSB) {
		unsigned int	dqnr;
		uint64_t	dqid;

		error = -libxfs_buf_read(mp->m_dev,
				XFS_FSB_TO_DADDR(mp, map->br_startblock + bno),
				dqchunklen, 0, &bp, &xfs_dquot_buf_ops);
		if (error) {
			do_warn(
_("cannot read %s inode %"PRIu64", block %"PRIu64", disk block %"PRIu64", err=%d\n"),
				qflags_typestr(dquots->type), ip->i_ino,
				map->br_startoff + bno,
				map->br_startblock + bno, error);
			chkd_flags = 0;
			return error;
		}

		dqb = bp->b_addr;
		dqid = (map->br_startoff + bno) * dqperchunk;
		for (dqnr = 0;
		     dqnr < dqperchunk && dqid <= UINT_MAX;
		     dqnr++, dqb++, dqid++)
			qc_check_dquot(mp, &dqb->dd_diskdq, dquots, dqid);
		libxfs_buf_relse(bp);
	}

	return error;
}

/* Check the incore quota counts with what's on disk. */
void
quotacheck_verify(
	struct xfs_mount	*mp,
	xfs_dqtype_t		type)
{
	struct xfs_bmbt_irec	map;
	struct xfs_iext_cursor	icur;
	struct xfs_inode	*ip;
	struct xfs_ifork	*ifp;
	struct qc_dquots	*dquots = NULL;
	struct avl64node	*node, *n;
	xfs_ino_t		ino = NULLFSINO;
	int			error;

	switch (type) {
	case XFS_DQTYPE_USER:
		ino = mp->m_sb.sb_uquotino;
		dquots = user_dquots;
		break;
	case XFS_DQTYPE_GROUP:
		ino = mp->m_sb.sb_gquotino;
		dquots = group_dquots;
		break;
	case XFS_DQTYPE_PROJ:
		ino = mp->m_sb.sb_pquotino;
		dquots = proj_dquots;
		break;
	}

	/*
	 * If we decided not to build incore records or there were errors in
	 * collecting them that caused us to clear chkd_flags, bail out early.
	 * No sense in complaining more about garbage.
	 */
	if (!dquots || !chkd_flags)
		return;

	error = -libxfs_iget(mp, NULL, ino, 0, &ip);
	if (error) {
		do_warn(
	_("could not open %s inode %"PRIu64" for quotacheck, err=%d\n"),
			qflags_typestr(type), ino, error);
		chkd_flags = 0;
		return;
	}

	ifp = xfs_ifork_ptr(ip, XFS_DATA_FORK);
	error = -libxfs_iread_extents(NULL, ip, XFS_DATA_FORK);
	if (error) {
		do_warn(
	_("could not read %s inode %"PRIu64" extents, err=%d\n"),
			qflags_typestr(type), ip->i_ino, error);
		chkd_flags = 0;
		goto err;
	}

	/* Walk each extent of the quota inode and compare counters. */
	for_each_xfs_iext(ifp, &icur, &map) {
		if (map.br_startblock != HOLESTARTBLOCK) {
			error = qc_walk_dquot_extent(ip, &map, dquots);
			if (error)
				goto err;
		}
	}

	/*
	 * We constructed incore dquots to account for every file we saw on
	 * disk, and then walked all on-disk dquots to compare.  Complain about
	 * incore dquots that weren't touched during the comparison, because
	 * that means something is missing from the dquot file.
	 */
	qc_dquots_foreach(dquots, node, n) {
		struct qc_rec	*qrec;

		qrec = container_of(node, struct qc_rec, node);
		if (!(qrec->flags & QC_REC_ONDISK)) {
			do_warn(
_("%s record for id %u not found on disk (bcount %"PRIu64" rtbcount %"PRIu64" icount %"PRIu64")\n"),
				qflags_typestr(type), qrec->id,
				qrec->bcount, qrec->rtbcount, qrec->icount);
			chkd_flags = 0;
		}
	}
err:
	libxfs_irele(ip);
}

/*
 * Decide if we want to run quotacheck on a particular quota type.  Returns
 * true only if the inode isn't lost, the fs says quotacheck ran, and the
 * ondisk pointer to the quota inode isn't "unset" (e.g. NULL or zero).
 */
static inline bool
qc_has_quotafile(
	struct xfs_mount	*mp,
	xfs_dqtype_t		type)
{
	bool			lost;
	xfs_ino_t		ino;
	unsigned int		qflag;

	switch (type) {
	case XFS_DQTYPE_USER:
		lost = lost_uquotino;
		ino = mp->m_sb.sb_uquotino;
		qflag = XFS_UQUOTA_CHKD;
		break;
	case XFS_DQTYPE_GROUP:
		lost = lost_gquotino;
		ino = mp->m_sb.sb_gquotino;
		qflag = XFS_GQUOTA_CHKD;
		break;
	case XFS_DQTYPE_PROJ:
		lost = lost_pquotino;
		ino = mp->m_sb.sb_pquotino;
		qflag = XFS_PQUOTA_CHKD;
		break;
	default:
		return false;
	}

	if (lost)
		return false;
	if (!(mp->m_sb.sb_qflags & qflag))
		return false;
	if (ino == NULLFSINO || ino == 0)
		return false;
	return true;
}

/* Initialize an incore dquot tree. */
static struct qc_dquots *
qc_dquots_init(
	xfs_dqtype_t		type)
{
	struct qc_dquots	*dquots;

	dquots = calloc(1, sizeof(struct qc_dquots));
	if (!dquots)
		return NULL;

	dquots->type = type;
	pthread_mutex_init(&dquots->lock, NULL);
	avl64_init_tree(&dquots->tree, &qc_cache_ops);
	return dquots;
}

/* Set up incore context for quota checks. */
int
quotacheck_setup(
	struct xfs_mount	*mp)
{
	chkd_flags = 0;

	/*
	 * If the superblock said quotas are disabled or was missing pointers
	 * to any quota inodes, don't bother checking.
	 */
	if (!fs_quotas || lost_quotas || noquota)
		return 0;

	if (qc_has_quotafile(mp, XFS_DQTYPE_USER)) {
		user_dquots = qc_dquots_init(XFS_DQTYPE_USER);
		if (!user_dquots)
			goto err;
		chkd_flags |= XFS_UQUOTA_CHKD;
	}

	if (qc_has_quotafile(mp, XFS_DQTYPE_GROUP)) {
		group_dquots = qc_dquots_init(XFS_DQTYPE_GROUP);
		if (!group_dquots)
			goto err;
		chkd_flags |= XFS_GQUOTA_CHKD;
	}

	if (qc_has_quotafile(mp, XFS_DQTYPE_PROJ)) {
		proj_dquots = qc_dquots_init(XFS_DQTYPE_PROJ);
		if (!proj_dquots)
			goto err;
		chkd_flags |= XFS_PQUOTA_CHKD;
	}

	return 0;
err:
	chkd_flags = 0;
	quotacheck_teardown();
	return ENOMEM;
}

/* Purge all quotacheck records in a given cache. */
static void
qc_purge(
	struct qc_dquots	**dquotsp)
{
	struct qc_dquots	*dquots = *dquotsp;
	struct qc_rec		*qrec;
	struct avl64node	*node;
	struct avl64node	*n;

	if (!dquots)
		return;

	qc_dquots_foreach(dquots, node, n) {
		qrec = container_of(node, struct qc_rec, node);
		free(qrec);
	}
	free(dquots);
	*dquotsp = NULL;
}

/* Tear down all the incore context from quotacheck. */
void
quotacheck_teardown(void)
{
	qc_purge(&user_dquots);
	qc_purge(&group_dquots);
	qc_purge(&proj_dquots);
}
