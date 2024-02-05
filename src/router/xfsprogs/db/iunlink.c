// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <djwong@kernel.org>
 */
#include "libxfs.h"
#include "command.h"
#include "output.h"
#include "init.h"

static xfs_filblks_t
count_rtblocks(
	struct xfs_inode	*ip)
{
	struct xfs_iext_cursor	icur;
	struct xfs_bmbt_irec	got;
	xfs_filblks_t		count = 0;
	struct xfs_ifork	*ifp = xfs_ifork_ptr(ip, XFS_DATA_FORK);
	int			error;

	error = -libxfs_iread_extents(NULL, ip, XFS_DATA_FORK);
	if (error) {
		dbprintf(
_("could not read AG %u agino %u extents, err=%d\n"),
				XFS_INO_TO_AGNO(ip->i_mount, ip->i_ino),
				XFS_INO_TO_AGINO(ip->i_mount, ip->i_ino),
				error);
		return 0;
	}

	for_each_xfs_iext(ifp, &icur, &got)
		if (!isnullstartblock(got.br_startblock))
			count += got.br_blockcount;
	return count;
}

static xfs_agino_t
get_next_unlinked(
	xfs_agnumber_t		agno,
	xfs_agino_t		agino,
	bool			verbose)
{
	struct xfs_buf		*ino_bp;
	struct xfs_dinode	*dip;
	struct xfs_inode	*ip;
	xfs_ino_t		ino;
	xfs_agino_t		ret;
	int			error;

	ino = XFS_AGINO_TO_INO(mp, agno, agino);
	error = -libxfs_iget(mp, NULL, ino, 0, &ip);
	if (error)
		goto bad;

	if (verbose) {
		xfs_filblks_t	blocks, rtblks = 0;

		if (XFS_IS_REALTIME_INODE(ip))
			rtblks = count_rtblocks(ip);
		blocks = ip->i_nblocks - rtblks;

		dbprintf(_(" blocks %llu rtblocks %llu\n"),
				blocks, rtblks);
	} else {
		dbprintf("\n");
	}

	error = -libxfs_imap_to_bp(mp, NULL, &ip->i_imap, &ino_bp);
	if (error)
		goto bad;

	dip = xfs_buf_offset(ino_bp, ip->i_imap.im_boffset);
	ret = be32_to_cpu(dip->di_next_unlinked);
	libxfs_buf_relse(ino_bp);

	return ret;
bad:
	dbprintf(_("AG %u agino %u: %s\n"), agno, agino, strerror(error));
	return NULLAGINO;
}

static void
dump_unlinked_bucket(
	xfs_agnumber_t	agno,
	struct xfs_buf	*agi_bp,
	unsigned int	bucket,
	bool		quiet,
	bool		verbose)
{
	struct xfs_agi	*agi = agi_bp->b_addr;
	xfs_agino_t	agino;
	unsigned int	i = 0;

	agino = be32_to_cpu(agi->agi_unlinked[bucket]);
	if (agino != NULLAGINO)
		dbprintf(_("AG %u bucket %u agino %u"), agno, bucket, agino);
	else if (!quiet && agino == NULLAGINO)
		dbprintf(_("AG %u bucket %u agino NULL\n"), agno, bucket);

	while (agino != NULLAGINO) {
		agino = get_next_unlinked(agno, agino, verbose);
		if (agino != NULLAGINO)
			dbprintf(_("    [%u] agino %u"), i++, agino);
		else if (!quiet && agino == NULLAGINO)
			dbprintf(_("    [%u] agino NULL\n"), i++);
	}
}

static void
dump_unlinked(
	struct xfs_perag	*pag,
	unsigned int		bucket,
	bool			quiet,
	bool			verbose)
{
	struct xfs_buf		*agi_bp;
	xfs_agnumber_t		agno = pag->pag_agno;
	int			error;

	error = -libxfs_ialloc_read_agi(pag, NULL, &agi_bp);
	if (error) {
		dbprintf(_("AGI %u: %s\n"), agno, strerror(errno));
		return;
	}

	if (bucket != -1U) {
		dump_unlinked_bucket(agno, agi_bp, bucket, quiet, verbose);
		goto relse;
	}

	for (bucket = 0; bucket < XFS_AGI_UNLINKED_BUCKETS; bucket++) {
		dump_unlinked_bucket(agno, agi_bp, bucket, quiet, verbose);
	}

relse:
	libxfs_buf_relse(agi_bp);
}

static int
dump_iunlinked_f(
	int			argc,
	char			**argv)
{
	struct xfs_perag	*pag;
	xfs_agnumber_t		agno = NULLAGNUMBER;
	unsigned int		bucket = -1U;
	bool			quiet = false;
	bool			verbose = false;
	int			c;

	while ((c = getopt(argc, argv, "a:b:qv")) != EOF) {
		switch (c) {
		case 'a':
			agno = atoi(optarg);
			if (agno >= mp->m_sb.sb_agcount) {
				dbprintf(_("Unknown AG %u, agcount is %u.\n"),
						agno, mp->m_sb.sb_agcount);
				return 0;
			}
			break;
		case 'b':
			bucket = atoi(optarg);
			if (bucket >= XFS_AGI_UNLINKED_BUCKETS) {
				dbprintf(_("Unknown bucket %u, max is 63.\n"),
						bucket);
				return 0;
			}
			break;
		case 'q':
			quiet = true;
			break;
		case 'v':
			verbose = true;
			break;
		default:
			dbprintf(_("Bad option for dump_iunlinked command.\n"));
			return 0;
		}
	}

	if (agno != NULLAGNUMBER) {
		struct xfs_perag	*pag = libxfs_perag_get(mp, agno);

		dump_unlinked(pag, bucket, quiet, verbose);
		libxfs_perag_put(pag);
		return 0;
	}

	for_each_perag(mp, agno, pag)
		dump_unlinked(pag, bucket, quiet, verbose);

	return 0;
}

static const cmdinfo_t	dump_iunlinked_cmd =
	{ "dump_iunlinked", NULL, dump_iunlinked_f, 0, -1, 0,
	  N_("[-a agno] [-b bucket] [-q] [-v]"),
	  N_("dump chain of unlinked inode buckets"), NULL };

/*
 * Look up the inode cluster buffer and log the on-disk unlinked inode change
 * we need to make.
 */
static int
iunlink_log_dinode(
	struct xfs_trans	*tp,
	struct xfs_inode	*ip,
	struct xfs_perag	*pag,
	xfs_agino_t		next_agino)
{
	struct xfs_mount	*mp = tp->t_mountp;
	struct xfs_dinode	*dip;
	struct xfs_buf		*ibp;
	int			offset;
	int			error;

	error = -libxfs_imap_to_bp(mp, tp, &ip->i_imap, &ibp);
	if (error)
		return error;

	dip = xfs_buf_offset(ibp, ip->i_imap.im_boffset);

	dip->di_next_unlinked = cpu_to_be32(next_agino);
	offset = ip->i_imap.im_boffset +
			offsetof(struct xfs_dinode, di_next_unlinked);

	libxfs_dinode_calc_crc(mp, dip);
	libxfs_trans_log_buf(tp, ibp, offset, offset + sizeof(xfs_agino_t) - 1);
	return 0;
}

static int
iunlink_insert_inode(
	struct xfs_trans	*tp,
	struct xfs_perag	*pag,
	struct xfs_buf		*agibp,
	struct xfs_inode	*ip)
{
	struct xfs_mount	*mp = tp->t_mountp;
	struct xfs_agi		*agi = agibp->b_addr;
	xfs_agino_t		next_agino;
	xfs_agino_t		agino = XFS_INO_TO_AGINO(mp, ip->i_ino);
	short			bucket_index = agino % XFS_AGI_UNLINKED_BUCKETS;
	int			offset;
	int			error;

	/*
	 * Get the index into the agi hash table for the list this inode will
	 * go on.  Make sure the pointer isn't garbage and that this inode
	 * isn't already on the list.
	 */
	next_agino = be32_to_cpu(agi->agi_unlinked[bucket_index]);
	if (next_agino == agino || !xfs_verify_agino_or_null(pag, next_agino))
		return EFSCORRUPTED;

	if (next_agino != NULLAGINO) {
		/*
		 * There is already another inode in the bucket, so point this
		 * inode to the current head of the list.
		 */
		error = iunlink_log_dinode(tp, ip, pag, next_agino);
		if (error)
			return error;
	}

	/* Update the bucket. */
	agi->agi_unlinked[bucket_index] = cpu_to_be32(agino);
	offset = offsetof(struct xfs_agi, agi_unlinked) +
			(sizeof(xfs_agino_t) * bucket_index);
	libxfs_trans_log_buf(tp, agibp, offset,
			offset + sizeof(xfs_agino_t) - 1);
	return 0;
}

/*
 * This is called when the inode's link count has gone to 0 or we are creating
 * a tmpfile via O_TMPFILE.  The inode @ip must have nlink == 0.
 *
 * We place the on-disk inode on a list in the AGI.  It will be pulled from this
 * list when the inode is freed.
 */
static int
iunlink(
	struct xfs_trans	*tp,
	struct xfs_inode	*ip)
{
	struct xfs_mount	*mp = tp->t_mountp;
	struct xfs_perag	*pag;
	struct xfs_buf		*agibp;
	int			error;

	ASSERT(VFS_I(ip)->i_nlink == 0);
	ASSERT(VFS_I(ip)->i_mode != 0);

	pag = libxfs_perag_get(mp, XFS_INO_TO_AGNO(mp, ip->i_ino));

	/* Get the agi buffer first.  It ensures lock ordering on the list. */
	error = -libxfs_read_agi(pag, tp, &agibp);
	if (error)
		goto out;

	error = iunlink_insert_inode(tp, pag, agibp, ip);
out:
	libxfs_perag_put(pag);
	return error;
}

static int
create_unlinked(
	struct xfs_mount	*mp)
{
	struct cred		cr = { };
	struct fsxattr		fsx = { };
	struct xfs_inode	*ip;
	struct xfs_trans	*tp;
	unsigned int		resblks;
	int			error;

	resblks = XFS_IALLOC_SPACE_RES(mp);
	error = -libxfs_trans_alloc(mp, &M_RES(mp)->tr_create_tmpfile, resblks,
			0, 0, &tp);
	if (error) {
		dbprintf(_("alloc trans: %s\n"), strerror(error));
		return error;
	}

	error = -libxfs_dir_ialloc(&tp, NULL, S_IFREG | 0600, 0, 0, &cr, &fsx,
			&ip);
	if (error) {
		dbprintf(_("create inode: %s\n"), strerror(error));
		goto out_cancel;
	}

	error = iunlink(tp, ip);
	if (error) {
		dbprintf(_("unlink inode: %s\n"), strerror(error));
		goto out_rele;
	}

	error = -libxfs_trans_commit(tp);
	if (error)
		dbprintf(_("commit inode: %s\n"), strerror(error));

	dbprintf(_("Created unlinked inode %llu in agno %u\n"),
			(unsigned long long)ip->i_ino,
			XFS_INO_TO_AGNO(mp, ip->i_ino));
	libxfs_irele(ip);
	return error;
out_rele:
	libxfs_irele(ip);
out_cancel:
	libxfs_trans_cancel(tp);
	return error;
}

static int
iunlink_f(
	int		argc,
	char		**argv)
{
	int		nr = 1;
	int		c;
	int		error;

	while ((c = getopt(argc, argv, "n:")) != EOF) {
		switch (c) {
		case 'n':
			nr = atoi(optarg);
			if (nr <= 0) {
				dbprintf(_("%s: need positive number\n"));
				return 0;
			}
			break;
		default:
			dbprintf(_("Bad option for iunlink command.\n"));
			return 0;
		}
	}

	for (c = 0; c < nr; c++) {
		error = create_unlinked(mp);
		if (error)
			return 1;
	}

	return 0;
}

static const cmdinfo_t	iunlink_cmd =
	{ "iunlink", NULL, iunlink_f, 0, -1, 0,
	  N_("[-n nr]"),
	  N_("allocate inodes and put them on the unlinked list"), NULL };

void
iunlink_init(void)
{
	add_command(&dump_iunlinked_cmd);
	if (expert_mode)
		add_command(&iunlink_cmd);
}
