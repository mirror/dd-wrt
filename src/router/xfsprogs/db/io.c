// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "dquot.h"
#include "inode.h"
#include "io.h"
#include "output.h"
#include "init.h"
#include "malloc.h"
#include "crc.h"
#include "bit.h"

static int	pop_f(int argc, char **argv);
static void     pop_help(void);
static int	push_f(int argc, char **argv);
static void     push_help(void);
static int	stack_f(int argc, char **argv);
static void     stack_help(void);
static int      forward_f(int argc, char **argv);
static void     forward_help(void);
static int      back_f(int argc, char **argv);
static void     back_help(void);
static int      ring_f(int argc, char **argv);
static void     ring_help(void);

static const cmdinfo_t	pop_cmd =
	{ "pop", NULL, pop_f, 0, 0, 0, NULL,
	  N_("pop location from the stack"), pop_help };
static const cmdinfo_t	push_cmd =
	{ "push", NULL, push_f, 0, 2, 0, N_("[command]"),
	  N_("push location to the stack"), push_help };
static const cmdinfo_t	stack_cmd =
	{ "stack", NULL, stack_f, 0, 0, 0, NULL,
	  N_("view the location stack"), stack_help };
static const cmdinfo_t  forward_cmd =
	{ "forward", "f", forward_f, 0, 0, 0, NULL,
	  N_("move forward to next entry in the position ring"), forward_help };
static const cmdinfo_t  back_cmd =
	{ "back", "b", back_f, 0, 0, 0, NULL,
	  N_("move to the previous location in the position ring"), back_help };
static const cmdinfo_t  ring_cmd =
	{ "ring", NULL, ring_f, 0, 1, 0, NULL,
	  N_("show position ring or move to a specific entry"), ring_help };

iocur_t	*iocur_base;
iocur_t	*iocur_top;
int	iocur_sp = -1;
int	iocur_len;

#define RING_ENTRIES 20
static iocur_t iocur_ring[RING_ENTRIES];
static int     ring_head = -1;
static int     ring_tail = -1;
static int     ring_current = -1;

void
io_init(void)
{
	add_command(&pop_cmd);
	add_command(&push_cmd);
	add_command(&stack_cmd);
	add_command(&forward_cmd);
	add_command(&back_cmd);
	add_command(&ring_cmd);
}

static inline void set_cur_boff(int off)
{
	iocur_top->boff = off;
	iocur_top->off = ((xfs_off_t)iocur_top->bb << BBSHIFT) + off;
	iocur_top->data = (void *)((char *)iocur_top->buf + off);
}

void
off_cur(
	int	off,
	int	len)
{
	if (iocur_top == NULL || off + len > BBTOB(iocur_top->blen))
		dbprintf(_("can't set block offset to %d\n"), off);
	else {
		set_cur_boff(off);
		iocur_top->len = len;
	}
}

void
pop_cur(void)
{
	if (iocur_sp < 0) {
		dbprintf(_("can't pop anything from I/O stack\n"));
		return;
	}
	if (iocur_top->bp) {
		libxfs_buf_relse(iocur_top->bp);
		iocur_top->bp = NULL;
	}
	if (iocur_top->bbmap) {
		free(iocur_top->bbmap);
		iocur_top->bbmap = NULL;
	}
	if (--iocur_sp >= 0) {
		iocur_top = iocur_base + iocur_sp;
		cur_typ = iocur_top->typ;
	} else {
		iocur_top = iocur_base;
		iocur_sp = 0;
	}
}

/*ARGSUSED*/
static int
pop_f(
	int	argc,
	char	**argv)
{
	pop_cur();
	return 0;
}

static void
pop_help(void)
{
	dbprintf(_(
"\n"
" Changes the address and data type to the first entry on the stack.\n"
"\n"
		));
}

void
print_iocur(
	char	*tag,
	iocur_t	*ioc)
{
	int	i;

	dbprintf("%s\n", tag);
	dbprintf(_("\tbyte offset %lld, length %d\n"), ioc->off, ioc->len);
	dbprintf(_("\tbuffer block %lld (fsbno %lld), %d bb%s\n"), ioc->bb,
		(xfs_fsblock_t)XFS_DADDR_TO_FSB(mp, ioc->bb), ioc->blen,
		ioc->blen == 1 ? "" : "s");
	if (ioc->bbmap) {
		dbprintf(_("\tblock map"));
		for (i = 0; i < ioc->bbmap->nmaps; i++)
			dbprintf(" %lld:%d", ioc->bbmap->b[i].bm_bn,
					     ioc->bbmap->b[i].bm_len);
		dbprintf("\n");
	}
	dbprintf(_("\tinode %lld, dir inode %lld, type %s\n"), ioc->ino,
		ioc->dirino, ioc->typ == NULL ? _("none") : ioc->typ->name);
}

static void
print_ring(void)
{
	int i;
	iocur_t *ioc;

	if (ring_current == -1) {
		dbprintf(_("no entries in location ring.\n"));
		return;
	}

	dbprintf(_("      type    bblock  bblen    fsbno     inode\n"));

	i = ring_head;
	for (;;) {
		ioc = &iocur_ring[i];
		if (i == ring_current)
			printf("*%2d: ", i);
		else
			printf(" %2d: ", i);

		dbprintf("%-7.7s %8lld %5d %8lld %9lld\n",
			 ioc->typ == NULL ? "none" : ioc->typ->name,
			 ioc->bb,
			 ioc->blen,
			 (xfs_fsblock_t)XFS_DADDR_TO_FSB(mp, ioc->bb),
			 ioc->ino
			);

		if (i == ring_tail)
			break;

		i = (i+(RING_ENTRIES-1))%RING_ENTRIES;
	}
}


void
push_cur(void)
{
	if (iocur_sp + 1 >= iocur_len) {
		iocur_base = xrealloc(iocur_base,
			sizeof(*iocur_base) * (iocur_len + 1));
		iocur_len++;
	}
	iocur_sp++;
	iocur_top = iocur_base + iocur_sp;
	memset(iocur_top, 0, sizeof(*iocur_base));
	iocur_top->ino = iocur_sp > 0 ? iocur_top[-1].ino : NULLFSINO;
	iocur_top->dirino = iocur_sp > 0 ? iocur_top[-1].dirino : NULLFSINO;
	iocur_top->mode = iocur_sp > 0 ? iocur_top[-1].mode : 0;
	cur_typ = NULL;
}

void
push_cur_and_set_type(void)
{
	/* save current state */
	push_cur();
	if (iocur_top[-1].typ && iocur_top[-1].typ->typnm == TYP_INODE)
		set_cur_inode(iocur_top[-1].ino);
	else
		set_cur(iocur_top[-1].typ, iocur_top[-1].bb,
			iocur_top[-1].blen, DB_RING_IGN,
			iocur_top[-1].bbmap);
}

static int
push_f(
	int		argc,
	char		**argv)
{
	const cmdinfo_t	*ct;

	if (argc > 1) {
		/* check we can execute command */
		ct = find_command(argv[1]);
		if (ct == NULL) {
			dbprintf(_("no such command %s\n"), argv[1]);
			return 0;
		}
		if (!ct->canpush) {
			dbprintf(_("no push form allowed for %s\n"), argv[1]);
			return 0;
		}
	}

	push_cur_and_set_type();

	/* run requested command */
	if (argc>1)
		(void)command(argc-1, argv+1);
	return 0;
}

static void
push_help(void)
{
	dbprintf(_(
"\n"
" Allows you to push the current address and data type on the stack for\n"
" later return.  'push' also accepts an additional command to execute after\n"
" storing the current address (ex: 'push a rootino' from the superblock).\n"
"\n"
		));
}

/* move forward through the ring */
/* ARGSUSED */
static int
forward_f(
	int		argc,
	char		**argv)
{
	if (ring_current == -1) {
		dbprintf(_("ring is empty\n"));
		return 0;
	}
	if (ring_current == ring_head) {
		dbprintf(_("no further entries\n"));
		return 0;
	}

	ring_current = (ring_current+1)%RING_ENTRIES;

	set_cur(iocur_ring[ring_current].typ,
		iocur_ring[ring_current].bb,
		iocur_ring[ring_current].blen,
		DB_RING_IGN,
		iocur_ring[ring_current].bbmap);

	return 0;
}

static void
forward_help(void)
{
	dbprintf(_(
"\n"
" The 'forward' ('f') command moves to the next location in the position\n"
" ring, updating the current position and data type.  If the current location\n"
" is the top entry in the ring, then the 'forward' command will have\n"
" no effect.\n"
"\n"
		));
}

/* move backwards through the ring */
/* ARGSUSED */
static int
back_f(
	int		argc,
	char		**argv)
{
	if (ring_current == -1) {
		dbprintf(_("ring is empty\n"));
		return 0;
	}
	if (ring_current == ring_tail) {
		dbprintf(_("no previous entries\n"));
		return 0;
	}

	ring_current = (ring_current+(RING_ENTRIES-1))%RING_ENTRIES;

	set_cur(iocur_ring[ring_current].typ,
		iocur_ring[ring_current].bb,
		iocur_ring[ring_current].blen,
		DB_RING_IGN,
		iocur_ring[ring_current].bbmap);

	return 0;
}

static void
back_help(void)
{
	dbprintf(_(
"\n"
" The 'back' ('b') command moves to the previous location in the position\n"
" ring, updating the current position and data type.  If the current location\n"
" is the last entry in the ring, then the 'back' command will have no effect.\n"
"\n"
		));
}

/* show or go to specific point in ring */
static int
ring_f(
	int		argc,
	char		**argv)
{
	int index;

	if (argc == 1) {
		print_ring();
		return 0;
	}

	index = (int)strtoul(argv[1], NULL, 0);
	if (index < 0 || index >= RING_ENTRIES) {
		dbprintf(_("invalid entry: %d\n"), index);
		return 0;
	}

	ring_current = index;

	set_cur(iocur_ring[index].typ,
		iocur_ring[index].bb,
		iocur_ring[index].blen,
		DB_RING_IGN,
		iocur_ring[index].bbmap);

	return 0;
}

static void
ring_help(void)
{
	dbprintf(_(
"\n"
" The position ring automatically keeps track of each disk location and\n"
" structure type for each change of position you make during your xfs_db\n"
" session.  The last %d most recent entries are kept in the ring.\n"
"\n"
" To display the current list of ring entries type 'ring' by itself on\n"
" the command line.  The entry highlighted by an asterisk ('*') is the\n"
" current entry.\n"
"\n"
" To move to another entry in the ring type 'ring <num>' where <num> is\n"
" your desired entry from the ring position list.\n"
"\n"
" You may also use the 'forward' ('f') or 'back' ('b') commands to move\n"
" to the previous or next entry in the ring, respectively.\n"
"\n"
" Note: Unlike the 'stack', 'push' and 'pop' commands, the ring tracks your\n"
" location implicitly.  Use the 'push' and 'pop' commands if you wish to\n"
" store a specific location explicitly for later return.\n"
"\n"),
		RING_ENTRIES);
}


void
ring_add(void)
{
	if (ring_head == -1) {
		/* only get here right after startup */
		ring_head = 0;
		ring_tail = 0;
		ring_current = 0;
		iocur_ring[0] = *iocur_top;
	} else {
		if (ring_current == ring_head) {
			ring_head = (ring_head+1)%RING_ENTRIES;
			iocur_ring[ring_head] = *iocur_top;
			if (ring_head == ring_tail)
				ring_tail = (ring_tail+1)%RING_ENTRIES;
			ring_current = ring_head;
		} else {
			ring_current = (ring_current+1)%RING_ENTRIES;
			iocur_ring[ring_current] = *iocur_top;
		}
	}
}

static void
write_cur_buf(void)
{
	int ret;

	ret = -libxfs_bwrite(iocur_top->bp);
	if (ret != 0)
		dbprintf(_("write error: %s\n"), strerror(ret));

	/* re-read buffer from disk */
	ret = -libxfs_readbufr(mp->m_ddev_targp, iocur_top->bb, iocur_top->bp,
			      iocur_top->blen, 0);
	if (ret != 0)
		dbprintf(_("read error: %s\n"), strerror(ret));
}

static void
write_cur_bbs(void)
{
	int ret;

	ret = -libxfs_bwrite(iocur_top->bp);
	if (ret != 0)
		dbprintf(_("write error: %s\n"), strerror(ret));


	/* re-read buffer from disk */
	ret = -libxfs_readbufr_map(mp->m_ddev_targp, iocur_top->bp, 0);
	if (ret != 0)
		dbprintf(_("read error: %s\n"), strerror(ret));
}

void
xfs_dummy_verify(
	struct xfs_buf *bp)
{
	return;
}

void
xfs_verify_recalc_crc(
	struct xfs_buf *bp)
{
	xfs_buf_update_cksum(bp, iocur_top->typ->crc_off);
}

void
write_cur(void)
{
	bool skip_crc = false;

	if (iocur_sp < 0) {
		dbprintf(_("nothing to write\n"));
		return;
	}

	if (!xfs_has_crc(mp) ||
	    !iocur_top->bp->b_ops ||
	    iocur_top->bp->b_ops->verify_write == xfs_dummy_verify)
		skip_crc = true;

	if (!skip_crc) {
		if (iocur_top->ino_buf)
			xfs_inode_set_crc(iocur_top->bp);
		else if (iocur_top->dquot_buf)
			xfs_dquot_set_crc(iocur_top->bp);
	}
	if (iocur_top->bbmap)
		write_cur_bbs();
	else
		write_cur_buf();

	/* If we didn't write the crc automatically, re-check inode validity */
	if (xfs_has_crc(mp) &&
	    skip_crc && iocur_top->ino_buf) {
		iocur_top->ino_crc_ok = libxfs_verify_cksum(iocur_top->data,
						mp->m_sb.sb_inodesize,
						XFS_DINODE_CRC_OFF);
	}

}

void
set_cur(
	const typ_t	*type,
	xfs_daddr_t	blknum,
	int		len,
	int		ring_flag,
	bbmap_t		*bbmap)
{
	struct xfs_buf	*bp;
	xfs_ino_t	dirino;
	xfs_ino_t	ino;
	uint16_t	mode;
	const struct xfs_buf_ops *ops = type ? type->bops : NULL;
	int		error;

	if (iocur_sp < 0) {
		dbprintf(_("set_cur no stack element to set\n"));
		return;
	}

	ino = iocur_top->ino;
	dirino = iocur_top->dirino;
	mode = iocur_top->mode;
	pop_cur();
	push_cur();

	if (bbmap) {
#ifdef DEBUG_BBMAP
		int i;
		printf(_("xfs_db got a bbmap for %lld\n"), (long long)blknum);
		printf(_("\tblock map"));
		for (i = 0; i < bbmap->nmaps; i++)
			printf(" %lld:%d", (long long)bbmap->b[i].bm_bn,
					   bbmap->b[i].bm_len);
		printf("\n");
#endif
		iocur_top->bbmap = malloc(sizeof(struct bbmap));
		if (!iocur_top->bbmap)
			return;
		memcpy(iocur_top->bbmap, bbmap, sizeof(struct bbmap));
		error = -libxfs_buf_read_map(mp->m_ddev_targp, bbmap->b,
				bbmap->nmaps, LIBXFS_READBUF_SALVAGE, &bp,
				ops);
	} else {
		error = -libxfs_buf_read(mp->m_ddev_targp, blknum, len,
				LIBXFS_READBUF_SALVAGE, &bp, ops);
		iocur_top->bbmap = NULL;
	}

	/*
	 * Salvage mode means that we still get a buffer even if the verifier
	 * says the metadata is corrupt.  Therefore, the only errors we should
	 * get are for IO errors or runtime errors.
	 */
	if (error)
		return;
	iocur_top->buf = bp->b_addr;
	iocur_top->bp = bp;
	if (!ops) {
		bp->b_ops = NULL;
		bp->b_flags |= LIBXFS_B_UNCHECKED;
	}

	iocur_top->bb = blknum;
	iocur_top->blen = len;
	iocur_top->boff = 0;
	iocur_top->data = iocur_top->buf;
	iocur_top->len = BBTOB(len);
	iocur_top->off = blknum << BBSHIFT;
	iocur_top->typ = cur_typ = type;
	iocur_top->ino = ino;
	iocur_top->dirino = dirino;
	iocur_top->mode = mode;
	iocur_top->ino_buf = 0;
	iocur_top->dquot_buf = 0;

	/* store location in ring */
	if (ring_flag)
		ring_add();
}

void
set_iocur_type(
	const typ_t	*type)
{
	int		bb_count = 1;	/* type's size in basic blocks */
	int		boff = iocur_top->boff;

	/*
	 * Inodes are special; verifier checks all inodes in the chunk, the
	 * set_cur_inode() will help that
	 */
	if (type->typnm == TYP_INODE) {
		xfs_daddr_t	b = iocur_top->bb;
		xfs_agblock_t	agbno;
		xfs_agino_t	agino;
		xfs_ino_t	ino;

		agbno = xfs_daddr_to_agbno(mp, b);
		agino = XFS_OFFBNO_TO_AGINO(mp, agbno,
				iocur_top->boff / mp->m_sb.sb_inodesize);
		ino = XFS_AGINO_TO_INO(mp, xfs_daddr_to_agno(mp, b), agino);
		set_cur_inode(ino);
		return;
	}

	/* adjust buffer size for types with fields & hence fsize() */
	if (type->fields)
		bb_count = BTOBB(byteize(fsize(type->fields,
				       iocur_top->data, 0, 0)));
	set_cur(type, iocur_top->bb, bb_count, DB_RING_IGN, NULL);
	set_cur_boff(boff);
}

static void
stack_help(void)
{
	dbprintf(_(
"\n"
" The stack is used to explicitly store your location and data type\n"
" for later return.  The 'push' operation stores the current address\n"
" and type on the stack, the 'pop' operation returns you to the\n"
" position and datatype of the top entry on the stack.\n"
"\n"
" The 'stack' allows explicit location saves, see 'ring' for implicit\n"
" position tracking.\n"
"\n"
		));
}

/*ARGSUSED*/
static int
stack_f(
	int	argc,
	char	**argv)
{
	int	i;
	char	tagbuf[14];

	for (i = iocur_sp; i > 0; i--) {
		snprintf(tagbuf, sizeof(tagbuf), "%d: ", i);
		print_iocur(tagbuf, &iocur_base[i]);
	}
	return 0;
}
