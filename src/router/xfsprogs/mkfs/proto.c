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

#include <xfs/libxfs.h>
#include <sys/stat.h>
#include "xfs_mkfs.h"

/*
 * Prototypes for internal functions.
 */
static long getnum(char **pp);
static char *getstr(char **pp);
static void fail(char *msg, int i);
static void getres(xfs_trans_t *tp, uint blocks);
static void rsvfile(xfs_mount_t *mp, xfs_inode_t *ip, long long len);
static int newfile(xfs_trans_t *tp, xfs_inode_t *ip, xfs_bmap_free_t *flist,
	xfs_fsblock_t *first, int dolocal, int logit, char *buf, int len);
static char *newregfile(char **pp, int *len);
static void rtinit(xfs_mount_t *mp);
static long filesize(int fd);

/*
 * Use this for block reservations needed for mkfs's conditions
 * (basically no fragmentation).
 */
#define	MKFS_BLOCKRES_INODE	\
	((uint)(XFS_IALLOC_BLOCKS(mp) + ((mp)->m_in_maxlevels - 1)))
#define	MKFS_BLOCKRES(rb)	\
	((uint)(MKFS_BLOCKRES_INODE + XFS_DA_NODE_MAXDEPTH + \
	(XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK) - 1) + (rb)))


char *
setup_proto(
	char	*fname)
{
	char		*buf;
	static char	dflt[] = "d--755 0 0 $";
	int		fd;
	long		size;

	if (!fname)
		return dflt;
	if ((fd = open(fname, O_RDONLY)) < 0 || (size = filesize(fd)) < 0) {
		fprintf(stderr, _("%s: failed to open %s: %s\n"),
			progname, fname, strerror(errno));
		exit(1);
	}
	buf = malloc(size + 1);
	if (read(fd, buf, size) < size) {
		fprintf(stderr, _("%s: read failed on %s: %s\n"),
			progname, fname, strerror(errno));
		exit(1);
	}
	if (buf[size - 1] != '\n') {
		fprintf(stderr, _("%s: proto file %s premature EOF\n"),
			progname, fname);
		exit(1);
	}
	buf[size] = '\0';
	/*
	 * Skip past the stuff there for compatibility, a string and 2 numbers.
	 */
	(void)getstr(&buf);	/* boot image name */
	(void)getnum(&buf);	/* block count */
	(void)getnum(&buf);	/* inode count */
	return buf;
}

static long
getnum(
	char	**pp)
{
	char	*s;

	s = getstr(pp);
	return atol(s);
}

static void
fail(
	char	*msg,
	int	i)
{
	fprintf(stderr, "%s: %s [%d - %s]\n", progname, msg, i, strerror(i));
	exit(1);
}

void
res_failed(
	int	i)
{
	fail(_("cannot reserve space"), i);
}

static void
getres(
	xfs_trans_t	*tp,
	uint		blocks)
{
	int		i;
	xfs_mount_t	*mp;
	uint		r;

	mp = tp->t_mountp;
	for (i = 0, r = MKFS_BLOCKRES(blocks); r >= blocks; r--) {
		i = libxfs_trans_reserve(tp, r, 0, 0, 0, 0);
		if (i == 0)
			return;
	}
	res_failed(i);
	/* NOTREACHED */
}

static char *
getstr(
	char	**pp)
{
	char	c;
	char	*p;
	char	*rval;

	p = *pp;
	while ((c = *p)) {
		switch (c) {
		case ' ':
		case '\t':
		case '\n':
			p++;
			continue;
		case ':':
			p++;
			while (*p++ != '\n')
				;
			continue;
		default:
			rval = p;
			while (c != ' ' && c != '\t' && c != '\n' && c != '\0')
				c = *++p;
			*p++ = '\0';
			*pp = p;
			return rval;
		}
	}
	if (c != '\0') {
		fprintf(stderr, _("%s: premature EOF in prototype file\n"),
			progname);
		exit(1);
	}
	return NULL;
}

static void
rsvfile(
	xfs_mount_t	*mp,
	xfs_inode_t	*ip,
	long long	llen)
{
	int		error;
	xfs_trans_t	*tp;

	error = libxfs_alloc_file_space(ip, 0, llen, 1, 0);

	if (error) {
		fail(_("error reserving space for a file"), error);
		exit(1);
	}

	/*
	 * update the inode timestamp, mode, and prealloc flag bits
	 */
	tp = libxfs_trans_alloc(mp, 0);

	libxfs_trans_ijoin(tp, ip, 0);
	libxfs_trans_ihold(tp, ip);

	ip->i_d.di_mode &= ~S_ISUID;

	/*
	 * Note that we don't have to worry about mandatory
	 * file locking being disabled here because we only
	 * clear the S_ISGID bit if the Group execute bit is
	 * on, but if it was on then mandatory locking wouldn't
	 * have been enabled.
	 */
	if (ip->i_d.di_mode & S_IXGRP)
		ip->i_d.di_mode &= ~S_ISGID;

	libxfs_trans_ichgtime(tp, ip, XFS_ICHGTIME_MOD | XFS_ICHGTIME_CHG);

	ip->i_d.di_flags |= XFS_DIFLAG_PREALLOC;

	libxfs_trans_log_inode(tp, ip, XFS_ILOG_CORE);
	libxfs_trans_commit(tp, 0);
}

static int
newfile(
	xfs_trans_t	*tp,
	xfs_inode_t	*ip,
	xfs_bmap_free_t	*flist,
	xfs_fsblock_t	*first,
	int		dolocal,
	int		logit,
	char		*buf,
	int		len)
{
	xfs_buf_t	*bp;
	xfs_daddr_t	d;
	int		error;
	int		flags;
	xfs_bmbt_irec_t	map;
	xfs_mount_t	*mp;
	xfs_extlen_t	nb;
	int		nmap;

	flags = 0;
	mp = ip->i_mount;
	if (dolocal && len <= XFS_IFORK_DSIZE(ip)) {
		xfs_idata_realloc(ip, len, XFS_DATA_FORK);
		if (buf)
			memmove(ip->i_df.if_u1.if_data, buf, len);
		ip->i_d.di_size = len;
		ip->i_df.if_flags &= ~XFS_IFEXTENTS;
		ip->i_df.if_flags |= XFS_IFINLINE;
		ip->i_d.di_format = XFS_DINODE_FMT_LOCAL;
		flags = XFS_ILOG_DDATA;
	} else if (len > 0) {
		nb = XFS_B_TO_FSB(mp, len);
		nmap = 1;
		error = libxfs_bmapi(tp, ip, 0, nb, XFS_BMAPI_WRITE, first, nb,
				&map, &nmap, flist);
		if (error) {
			fail(_("error allocating space for a file"), error);
		}
		if (nmap != 1) {
			fprintf(stderr,
				_("%s: cannot allocate space for file\n"),
				progname);
			exit(1);
		}
		d = XFS_FSB_TO_DADDR(mp, map.br_startblock);
		bp = libxfs_trans_get_buf(logit ? tp : 0, mp->m_dev, d,
			nb << mp->m_blkbb_log, 0);
		memmove(XFS_BUF_PTR(bp), buf, len);
		if (len < XFS_BUF_COUNT(bp))
			memset(XFS_BUF_PTR(bp) + len, 0, XFS_BUF_COUNT(bp) - len);
		if (logit)
			libxfs_trans_log_buf(tp, bp, 0, XFS_BUF_COUNT(bp) - 1);
		else
			libxfs_writebuf(bp, LIBXFS_EXIT_ON_FAILURE);
	}
	ip->i_d.di_size = len;
	return flags;
}

static char *
newregfile(
	char		**pp,
	int		*len)
{
	char		*buf;
	int		fd;
	char		*fname;
	long		size;

	fname = getstr(pp);
	if ((fd = open(fname, O_RDONLY)) < 0 || (size = filesize(fd)) < 0) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, fname, strerror(errno));
		exit(1);
	}
	if ((*len = (int)size)) {
		buf = malloc(size);
		if (read(fd, buf, size) < size) {
			fprintf(stderr, _("%s: read failed on %s: %s\n"),
				progname, fname, strerror(errno));
			exit(1);
		}
	} else
		buf = 0;
	close(fd);
	return buf;
}

static void
newdirent(
	xfs_mount_t	*mp,
	xfs_trans_t	*tp,
	xfs_inode_t	*pip,
	struct xfs_name	*name,
	xfs_ino_t	inum,
	xfs_fsblock_t	*first,
	xfs_bmap_free_t	*flist,
	xfs_extlen_t	total)
{
	int	error;

	error = libxfs_dir_createname(tp, pip, name, inum, first, flist, total);
	if (error)
		fail(_("directory createname error"), error);
}

static void
newdirectory(
	xfs_mount_t	*mp,
	xfs_trans_t	*tp,
	xfs_inode_t	*dp,
	xfs_inode_t	*pdp)
{
	int	error;

	error = libxfs_dir_init(tp, dp, pdp);
	if (error)
		fail(_("directory create error"), error);
}

static void
parseproto(
	xfs_mount_t	*mp,
	xfs_inode_t	*pip,
	struct fsxattr	*fsxp,
	char		**pp,
	char		*name)
{
#define	IF_REGULAR	0
#define	IF_RESERVED	1
#define	IF_BLOCK	2
#define	IF_CHAR		3
#define	IF_DIRECTORY	4
#define	IF_SYMLINK	5
#define	IF_FIFO		6

	char		*buf;
	int		committed;
	int		error;
	xfs_fsblock_t	first;
	int		flags;
	xfs_bmap_free_t	flist;
	int		fmt;
	int		i;
	xfs_inode_t	*ip;
	int		len;
	long long	llen;
	int		majdev;
	int		mindev;
	int		mode;
	char		*mstr;
	xfs_trans_t	*tp;
	int		val;
	int		isroot = 0;
	cred_t		creds;
	char		*value;
	struct xfs_name	xname;

	memset(&creds, 0, sizeof(creds));
	mstr = getstr(pp);
	switch (mstr[0]) {
	case '-':
		fmt = IF_REGULAR;
		break;
	case 'r':
		fmt = IF_RESERVED;
		break;
	case 'b':
		fmt = IF_BLOCK;
		break;
	case 'c':
		fmt = IF_CHAR;
		break;
	case 'd':
		fmt = IF_DIRECTORY;
		break;
	case 'l':
		fmt = IF_SYMLINK;
		break;
	case 'p':
		fmt = IF_FIFO;
		break;
	default:
		fprintf(stderr, _("%s: bad format string %s\n"),
			progname, mstr);
		exit(1);
	}
	mode = 0;
	switch (mstr[1]) {
	case '-':
		break;
	case 'u':
		mode |= S_ISUID;
		break;
	default:
		fprintf(stderr, _("%s: bad format string %s\n"),
			progname, mstr);
		exit(1);
	}
	switch (mstr[2]) {
	case '-':
		break;
	case 'g':
		mode |= S_ISGID;
		break;
	default:
		fprintf(stderr, _("%s: bad format string %s\n"),
			progname, mstr);
		exit(1);
	}
	val = 0;
	for (i = 3; i < 6; i++) {
		if (mstr[i] < '0' || mstr[i] > '7') {
			fprintf(stderr, _("%s: bad format string %s\n"),
				progname, mstr);
			exit(1);
		}
		val = val * 8 + mstr[i] - '0';
	}
	mode |= val;
	creds.cr_uid = (int)getnum(pp);
	creds.cr_gid = (int)getnum(pp);
	xname.name = (uchar_t *)name;
	xname.len = name ? strlen(name) : 0;
	tp = libxfs_trans_alloc(mp, 0);
	flags = XFS_ILOG_CORE;
	xfs_bmap_init(&flist, &first);
	switch (fmt) {
	case IF_REGULAR:
		buf = newregfile(pp, &len);
		getres(tp, XFS_B_TO_FSB(mp, len));
		error = libxfs_inode_alloc(&tp, pip, mode|S_IFREG, 1, 0,
					   &creds, fsxp, &ip);
		if (error)
			fail(_("Inode allocation failed"), error);
		flags |= newfile(tp, ip, &flist, &first, 0, 0, buf, len);
		if (buf)
			free(buf);
		libxfs_trans_ijoin(tp, pip, 0);
		newdirent(mp, tp, pip, &xname, ip->i_ino, &first, &flist, 1);
		libxfs_trans_ihold(tp, pip);
		break;

	case IF_RESERVED:			/* pre-allocated space only */
		value = getstr(pp);
		llen = cvtnum(mp->m_sb.sb_blocksize, mp->m_sb.sb_sectsize, value);
		getres(tp, XFS_B_TO_FSB(mp, llen));

		error = libxfs_inode_alloc(&tp, pip, mode|S_IFREG, 1, 0,
					  &creds, fsxp, &ip);
		if (error)
			fail(_("Inode pre-allocation failed"), error);

		libxfs_trans_ijoin(tp, pip, 0);

		newdirent(mp, tp, pip, &xname, ip->i_ino, &first, &flist, 1);
		libxfs_trans_ihold(tp, pip);
		libxfs_trans_log_inode(tp, ip, flags);

		error = libxfs_bmap_finish(&tp, &flist, &committed);
		if (error)
			fail(_("Pre-allocated file creation failed"), error);
		libxfs_trans_commit(tp, 0);
		rsvfile(mp, ip, llen);
		return;

	case IF_BLOCK:
		getres(tp, 0);
		majdev = (int)getnum(pp);
		mindev = (int)getnum(pp);
		error = libxfs_inode_alloc(&tp, pip, mode|S_IFBLK, 1,
				IRIX_MKDEV(majdev, mindev), &creds, fsxp, &ip);
		if (error) {
			fail(_("Inode allocation failed"), error);
		}
		libxfs_trans_ijoin(tp, pip, 0);
		newdirent(mp, tp, pip, &xname, ip->i_ino, &first, &flist, 1);
		libxfs_trans_ihold(tp, pip);
		flags |= XFS_ILOG_DEV;
		break;

	case IF_CHAR:
		getres(tp, 0);
		majdev = (int)getnum(pp);
		mindev = (int)getnum(pp);
		error = libxfs_inode_alloc(&tp, pip, mode|S_IFCHR, 1,
				IRIX_MKDEV(majdev, mindev), &creds, fsxp, &ip);
		if (error)
			fail(_("Inode allocation failed"), error);
		libxfs_trans_ijoin(tp, pip, 0);
		newdirent(mp, tp, pip, &xname, ip->i_ino, &first, &flist, 1);
		libxfs_trans_ihold(tp, pip);
		flags |= XFS_ILOG_DEV;
		break;

	case IF_FIFO:
		getres(tp, 0);
		error = libxfs_inode_alloc(&tp, pip, mode|S_IFIFO, 1, 0,
				&creds, fsxp, &ip);
		if (error)
			fail(_("Inode allocation failed"), error);
		libxfs_trans_ijoin(tp, pip, 0);
		newdirent(mp, tp, pip, &xname, ip->i_ino, &first, &flist, 1);
		libxfs_trans_ihold(tp, pip);
		break;
	case IF_SYMLINK:
		buf = getstr(pp);
		len = (int)strlen(buf);
		getres(tp, XFS_B_TO_FSB(mp, len));
		error = libxfs_inode_alloc(&tp, pip, mode|S_IFLNK, 1, 0,
				&creds, fsxp, &ip);
		if (error)
			fail(_("Inode allocation failed"), error);
		flags |= newfile(tp, ip, &flist, &first, 1, 1, buf, len);
		libxfs_trans_ijoin(tp, pip, 0);
		newdirent(mp, tp, pip, &xname, ip->i_ino, &first, &flist, 1);
		libxfs_trans_ihold(tp, pip);
		break;
	case IF_DIRECTORY:
		getres(tp, 0);
		error = libxfs_inode_alloc(&tp, pip, mode|S_IFDIR, 1, 0,
				&creds, fsxp, &ip);
		if (error)
			fail(_("Inode allocation failed"), error);
		ip->i_d.di_nlink++;		/* account for . */
		if (!pip) {
			pip = ip;
			mp->m_sb.sb_rootino = ip->i_ino;
			libxfs_mod_sb(tp, XFS_SB_ROOTINO);
			mp->m_rootip = ip;
			isroot = 1;
		} else {
			libxfs_trans_ijoin(tp, pip, 0);
			newdirent(mp, tp, pip, &xname, ip->i_ino,
				  &first, &flist, 1);
			pip->i_d.di_nlink++;
			libxfs_trans_ihold(tp, pip);
			libxfs_trans_log_inode(tp, pip, XFS_ILOG_CORE);
		}
		newdirectory(mp, tp, ip, pip);
		libxfs_trans_log_inode(tp, ip, flags);
		error = libxfs_bmap_finish(&tp, &flist, &committed);
		if (error)
			fail(_("Directory creation failed"), error);
		libxfs_trans_ihold(tp, ip);
		libxfs_trans_commit(tp, 0);
		/*
		 * RT initialization.  Do this here to ensure that
		 * the RT inodes get placed after the root inode.
		 */
		if (isroot)
			rtinit(mp);
		tp = NULL;
		for (;;) {
			name = getstr(pp);
			if (!name)
				break;
			if (strcmp(name, "$") == 0)
				break;
			parseproto(mp, ip, fsxp, pp, name);
		}
		libxfs_iput(ip, 0);
		return;
	}
	libxfs_trans_log_inode(tp, ip, flags);
	error = libxfs_bmap_finish(&tp, &flist, &committed);
	if (error) {
		fail(_("Error encountered creating file from prototype file"),
			error);
	}
	libxfs_trans_commit(tp, 0);
}

void
parse_proto(
	xfs_mount_t	*mp,
	struct fsxattr	*fsx,
	char		**pp)
{
	parseproto(mp, NULL, fsx, pp, NULL);
}

/*
 * Allocate the realtime bitmap and summary inodes, and fill in data if any.
 */
static void
rtinit(
	xfs_mount_t	*mp)
{
	xfs_dfiloff_t	bno;
	int		committed;
	xfs_dfiloff_t	ebno;
	xfs_bmbt_irec_t	*ep;
	int		error;
	xfs_fsblock_t	first;
	xfs_bmap_free_t	flist;
	int		i;
	xfs_bmbt_irec_t	map[XFS_BMAP_MAX_NMAP];
	xfs_extlen_t	nsumblocks;
	int		nmap;
	xfs_inode_t	*rbmip;
	xfs_inode_t	*rsumip;
	xfs_trans_t	*tp;
	struct cred	creds;
	struct fsxattr	fsxattrs;

	/*
	 * First, allocate the inodes.
	 */
	tp = libxfs_trans_alloc(mp, 0);
	if ((i = libxfs_trans_reserve(tp, MKFS_BLOCKRES_INODE, 0, 0, 0, 0)))
		res_failed(i);
	memset(&creds, 0, sizeof(creds));
	memset(&fsxattrs, 0, sizeof(fsxattrs));
	error = libxfs_inode_alloc(&tp, NULL, S_IFREG, 1, 0,
					&creds, &fsxattrs, &rbmip);
	if (error) {
		fail(_("Realtime bitmap inode allocation failed"), error);
	}
	/*
	 * Do our thing with rbmip before allocating rsumip,
	 * because the next call to ialloc() may
	 * commit the transaction in which rbmip was allocated.
	 */
	mp->m_sb.sb_rbmino = rbmip->i_ino;
	rbmip->i_d.di_size = mp->m_sb.sb_rbmblocks * mp->m_sb.sb_blocksize;
	rbmip->i_d.di_flags = XFS_DIFLAG_NEWRTBM;
	*(__uint64_t *)&rbmip->i_d.di_atime = 0;
	libxfs_trans_log_inode(tp, rbmip, XFS_ILOG_CORE);
	libxfs_mod_sb(tp, XFS_SB_RBMINO);
	libxfs_trans_ihold(tp, rbmip);
	mp->m_rbmip = rbmip;
	error = libxfs_inode_alloc(&tp, NULL, S_IFREG, 1, 0,
					&creds, &fsxattrs, &rsumip);
	if (error) {
		fail(_("Realtime summary inode allocation failed"), error);
	}
	mp->m_sb.sb_rsumino = rsumip->i_ino;
	rsumip->i_d.di_size = mp->m_rsumsize;
	libxfs_trans_log_inode(tp, rsumip, XFS_ILOG_CORE);
	libxfs_mod_sb(tp, XFS_SB_RSUMINO);
	libxfs_trans_ihold(tp, rsumip);
	libxfs_trans_commit(tp, 0);
	mp->m_rsumip = rsumip;
	/*
	 * Next, give the bitmap file some zero-filled blocks.
	 */
	tp = libxfs_trans_alloc(mp, 0);
	if ((i = libxfs_trans_reserve(tp, mp->m_sb.sb_rbmblocks +
			(XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK) - 1), 0, 0, 0, 0)))
		res_failed(i);
	libxfs_trans_ijoin(tp, rbmip, 0);
	libxfs_trans_ihold(tp, rbmip);
	bno = 0;
	xfs_bmap_init(&flist, &first);
	while (bno < mp->m_sb.sb_rbmblocks) {
		nmap = XFS_BMAP_MAX_NMAP;
		error = libxfs_bmapi(tp, rbmip, bno,
				(xfs_extlen_t)(mp->m_sb.sb_rbmblocks - bno),
				XFS_BMAPI_WRITE, &first, mp->m_sb.sb_rbmblocks,
				map, &nmap, &flist);
		if (error) {
			fail(_("Allocation of the realtime bitmap failed"),
				error);
		}
		for (i = 0, ep = map; i < nmap; i++, ep++) {
			libxfs_device_zero(mp->m_dev,
				XFS_FSB_TO_DADDR(mp, ep->br_startblock),
				XFS_FSB_TO_BB(mp, ep->br_blockcount));
			bno += ep->br_blockcount;
		}
	}

	error = libxfs_bmap_finish(&tp, &flist, &committed);
	if (error) {
		fail(_("Completion of the realtime bitmap failed"), error);
	}
	libxfs_trans_commit(tp, 0);

	/*
	 * Give the summary file some zero-filled blocks.
	 */
	tp = libxfs_trans_alloc(mp, 0);
	nsumblocks = mp->m_rsumsize >> mp->m_sb.sb_blocklog;
	if ((i = libxfs_trans_reserve(tp,
			nsumblocks + (XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK) - 1),
			0, 0, 0, 0)))
		res_failed(i);
	libxfs_trans_ijoin(tp, rsumip, 0);
	libxfs_trans_ihold(tp, rsumip);
	bno = 0;
	xfs_bmap_init(&flist, &first);
	while (bno < nsumblocks) {
		nmap = XFS_BMAP_MAX_NMAP;
		error = libxfs_bmapi(tp, rsumip, bno,
				(xfs_extlen_t)(nsumblocks - bno),
				XFS_BMAPI_WRITE, &first, nsumblocks,
				map, &nmap, &flist);
		if (error) {
			fail(_("Allocation of the realtime summary failed"),
				error);
		}
		for (i = 0, ep = map; i < nmap; i++, ep++) {
			libxfs_device_zero(mp->m_dev,
				XFS_FSB_TO_DADDR(mp, ep->br_startblock),
				XFS_FSB_TO_BB(mp, ep->br_blockcount));
			bno += ep->br_blockcount;
		}
	}
	error = libxfs_bmap_finish(&tp, &flist, &committed);
	if (error) {
		fail(_("Completion of the realtime summary failed"), error);
	}
	libxfs_trans_commit(tp, 0);

	/*
	 * Free the whole area using transactions.
	 * Do one transaction per bitmap block.
	 */
	for (bno = 0; bno < mp->m_sb.sb_rextents; bno = ebno) {
		tp = libxfs_trans_alloc(mp, 0);
		if ((i = libxfs_trans_reserve(tp, 0, 0, 0, 0, 0)))
			res_failed(i);
		xfs_bmap_init(&flist, &first);
		ebno = XFS_RTMIN(mp->m_sb.sb_rextents,
			bno + NBBY * mp->m_sb.sb_blocksize);
		error = libxfs_rtfree_extent(tp, bno, (xfs_extlen_t)(ebno-bno));
		if (error) {
			fail(_("Error initializing the realtime space"),
				error);
		}
		error = libxfs_bmap_finish(&tp, &flist, &committed);
		if (error) {
			fail(_("Error completing the realtime space"), error);
		}
		libxfs_trans_commit(tp, 0);
	}
}

static long
filesize(
	int		fd)
{
	struct stat64	stb;

	if (fstat64(fd, &stb) < 0)
		return -1;
	return (long)stb.st_size;
}
