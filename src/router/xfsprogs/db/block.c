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
#include "block.h"
#include "bmap.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "inode.h"
#include "io.h"
#include "output.h"
#include "init.h"

static int	ablock_f(int argc, char **argv);
static void     ablock_help(void);
static int	daddr_f(int argc, char **argv);
static void     daddr_help(void);
static int	dblock_f(int argc, char **argv);
static void     dblock_help(void);
static int	fsblock_f(int argc, char **argv);
static void     fsblock_help(void);
static void	print_rawdata(void *data, int len);

static const cmdinfo_t	ablock_cmd =
	{ "ablock", NULL, ablock_f, 1, 1, 1, N_("filoff"),
	  N_("set address to file offset (attr fork)"), ablock_help };
static const cmdinfo_t	daddr_cmd =
	{ "daddr", NULL, daddr_f, 0, 1, 1, N_("[d]"),
	  N_("set address to daddr value"), daddr_help };
static const cmdinfo_t	dblock_cmd =
	{ "dblock", NULL, dblock_f, 1, 1, 1, N_("filoff"),
	  N_("set address to file offset (data fork)"), dblock_help };
static const cmdinfo_t	fsblock_cmd =
	{ "fsblock", "fsb", fsblock_f, 0, 1, 1, N_("[fsb]"),
	  N_("set address to fsblock value"), fsblock_help };

static void
ablock_help(void)
{
	dbprintf(_(
"\n Example:\n"
"\n"
" 'ablock 23' - sets the file position to the 23rd filesystem block in\n"
" the inode's attribute fork.  The filesystem block size is specified in\n"
" the superblock.\n\n"
));
}

/*ARGSUSED*/
static int
ablock_f(
	int		argc,
	char		**argv)
{
	bmap_ext_t	bm;
	xfs_dfiloff_t	bno;
	xfs_dfsbno_t	dfsbno;
	int		haveattr;
	int		nex;
	char		*p;

	bno = (xfs_dfiloff_t)strtoull(argv[1], &p, 0);
	if (*p != '\0') {
		dbprintf(_("bad block number %s\n"), argv[1]);
		return 0;
	}
	push_cur();
	set_cur_inode(iocur_top->ino);
	haveattr = XFS_DFORK_Q((xfs_dinode_t *)iocur_top->data);
	pop_cur();
	if (!haveattr) {
		dbprintf(_("no attribute data for file\n"));
		return 0;
	}
	nex = 1;
	bmap(bno, 1, XFS_ATTR_FORK, &nex, &bm);
	if (nex == 0) {
		dbprintf(_("file attr block is unmapped\n"));
		return 0;
	}
	dfsbno = bm.startblock + (bno - bm.startoff);
	ASSERT(typtab[TYP_ATTR].typnm == TYP_ATTR);
	set_cur(&typtab[TYP_ATTR], (__int64_t)XFS_FSB_TO_DADDR(mp, dfsbno),
		blkbb, DB_RING_ADD, NULL);
	return 0;
}

void
block_init(void)
{
	add_command(&ablock_cmd);
	add_command(&daddr_cmd);
	add_command(&dblock_cmd);
	add_command(&fsblock_cmd);
}

static void
daddr_help(void)
{
	dbprintf(_(
"\n Example:\n"
"\n"
" 'daddr 102' - sets position to the 102nd absolute disk block\n"
" (512 byte block).\n"
));
}

static int
daddr_f(
	int		argc,
	char		**argv)
{
	__int64_t	d;
	char		*p;

	if (argc == 1) {
		dbprintf(_("current daddr is %lld\n"), iocur_top->off >> BBSHIFT);
		return 0;
	}
	d = (__int64_t)strtoull(argv[1], &p, 0);
	if (*p != '\0' ||
	    d >= mp->m_sb.sb_dblocks << (mp->m_sb.sb_blocklog - BBSHIFT)) {
		dbprintf(_("bad daddr %s\n"), argv[1]);
		return 0;
	}
	ASSERT(typtab[TYP_DATA].typnm == TYP_DATA);
	set_cur(&typtab[TYP_DATA], d, 1, DB_RING_ADD, NULL);
	return 0;
}

static void
dblock_help(void)
{
	dbprintf(_(
"\n Example:\n"
"\n"
" 'dblock 23' - sets the file position to the 23rd filesystem block in\n"
" the inode's data fork.  The filesystem block size is specified in the\n"
" superblock.\n\n"
));
}

static int
dblock_f(
	int		argc,
	char		**argv)
{
	bbmap_t		bbmap;
	bmap_ext_t	*bmp;
	xfs_dfiloff_t	bno;
	xfs_dfsbno_t	dfsbno;
	int		nb;
	int		nex;
	char		*p;
	typnm_t		type;

	bno = (xfs_dfiloff_t)strtoull(argv[1], &p, 0);
	if (*p != '\0') {
		dbprintf(_("bad block number %s\n"), argv[1]);
		return 0;
	}
	push_cur();
	set_cur_inode(iocur_top->ino);
	type = inode_next_type();
	pop_cur();
	if (type == TYP_NONE) {
		dbprintf(_("no type for file data\n"));
		return 0;
	}
	nex = nb = type == TYP_DIR2 ? mp->m_dirblkfsbs : 1;
	bmp = malloc(nb * sizeof(*bmp));
	bmap(bno, nb, XFS_DATA_FORK, &nex, bmp);
	if (nex == 0) {
		dbprintf(_("file data block is unmapped\n"));
		free(bmp);
		return 0;
	}
	dfsbno = bmp->startblock + (bno - bmp->startoff);
	ASSERT(typtab[type].typnm == type);
	if (nex > 1)
		make_bbmap(&bbmap, nex, bmp);
	set_cur(&typtab[type], (__int64_t)XFS_FSB_TO_DADDR(mp, dfsbno),
		nb * blkbb, DB_RING_ADD, nex > 1 ? &bbmap : NULL);
	free(bmp);
	return 0;
}

static void
fsblock_help(void)
{
	dbprintf(_(
"\n Example:\n"
"\n"
" 'fsblock 1023' - sets the file position to the 1023rd filesystem block.\n"
" The filesystem block size is specified in the superblock and set during\n"
" mkfs time.  Offset is absolute (not AG relative).\n\n"
));
}

static int
fsblock_f(
	int		argc,
	char		**argv)
{
	xfs_agblock_t	agbno;
	xfs_agnumber_t	agno;
	xfs_dfsbno_t	d;
	char		*p;

	if (argc == 1) {
		dbprintf(_("current fsblock is %lld\n"),
			XFS_DADDR_TO_FSB(mp, iocur_top->off >> BBSHIFT));
		return 0;
	}
	d = strtoull(argv[1], &p, 0);
	if (*p != '\0') {
		dbprintf(_("bad fsblock %s\n"), argv[1]);
		return 0;
	}
	agno = XFS_FSB_TO_AGNO(mp, d);
	agbno = XFS_FSB_TO_AGBNO(mp, d);
	if (agno >= mp->m_sb.sb_agcount || agbno >= mp->m_sb.sb_agblocks) {
		dbprintf(_("bad fsblock %s\n"), argv[1]);
		return 0;
	}
	ASSERT(typtab[TYP_DATA].typnm == TYP_DATA);
	set_cur(&typtab[TYP_DATA], XFS_AGB_TO_DADDR(mp, agno, agbno),
		blkbb, DB_RING_ADD, NULL);
	return 0;
}

void
print_block(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	print_rawdata(iocur_top->data, iocur_top->len);
}

static void
print_rawdata(
	void	*data,
	int	len)
{
	int	i;
	int	j;
	int	lastaddr;
	int	offchars;
	unsigned char	*p;

	lastaddr = (len - 1) & ~(32 - 1);
	if (lastaddr < 0x10)
		offchars = 1;
	else if (lastaddr < 0x100)
		offchars = 2;
	else if (lastaddr < 0x1000)
		offchars = 3;
	else
		offchars = 4;
	for (i = 0, p = data; i < len; i += 32) {
		dbprintf("%-0*.*x:", offchars, offchars, i);
		for (j = 0; j < 32 && i + j < len; j++, p++) {
			if ((j & 3) == 0)
				dbprintf(" ");
			dbprintf("%02x", *p);
		}
		dbprintf("\n");
	}
}
