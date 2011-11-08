/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
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
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "inode.h"
#include "io.h"
#include "output.h"
#include "init.h"
#include "malloc.h"

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

void
off_cur(
	int	off,
	int	len)
{
	if (iocur_top == NULL || off + len > BBTOB(iocur_top->blen))
		dbprintf(_("can't set block offset to %d\n"), off);
	else {
		iocur_top->boff = off;
		iocur_top->off = ((xfs_off_t)iocur_top->bb << BBSHIFT) + off;
		iocur_top->len = len;
		iocur_top->data = (void *)((char *)iocur_top->buf + off);
	}
}

void
pop_cur(void)
{
	if (iocur_sp < 0) {
		dbprintf(_("can't pop anything from I/O stack\n"));
		return;
	}
	if (iocur_top->buf)
		xfree(iocur_top->buf);
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
		(xfs_dfsbno_t)XFS_DADDR_TO_FSB(mp, ioc->bb), ioc->blen,
		ioc->blen == 1 ? "" : "s");
	if (ioc->use_bbmap) {
		dbprintf(_("\tblock map"));
		for (i = 0; i < ioc->blen; i++)
			dbprintf(" %d:%lld", i, ioc->bbmap.b[i]);
		dbprintf("\n");
	}
	dbprintf(_("\tinode %lld, dir inode %lld, type %s\n"), ioc->ino,
		ioc->dirino, ioc->typ == NULL ? _("none") : ioc->typ->name);
}

void
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
			 (xfs_dfsbno_t)XFS_DADDR_TO_FSB(mp, ioc->bb),
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

	/* save current state */
	push_cur();
	if (iocur_top[-1].typ && iocur_top[-1].typ->typnm == TYP_INODE)
		set_cur_inode(iocur_top[-1].ino);
	else
		set_cur(iocur_top[-1].typ, iocur_top[-1].bb,
			iocur_top[-1].blen, DB_RING_IGN,
			iocur_top[-1].use_bbmap ? &iocur_top[-1].bbmap : NULL);

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
		iocur_ring[ring_current].use_bbmap ?
			&iocur_ring[ring_current].bbmap : NULL);

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
		iocur_ring[ring_current].use_bbmap ?
			&iocur_ring[ring_current].bbmap : NULL);

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
	if (index < 0 || index >= RING_ENTRIES)
		dbprintf(_("invalid entry: %d\n"), index);

	ring_current = index;

	set_cur(iocur_ring[index].typ,
		iocur_ring[index].bb,
		iocur_ring[index].blen,
		DB_RING_IGN,
		iocur_ring[index].use_bbmap ? &iocur_ring[index].bbmap : NULL);

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


int
write_bbs(
	__int64_t       bbno,
	int             count,
	void            *bufp,
	bbmap_t		*bbmap)
{
	int		c;
	int		i;
	int		j;
	int		rval = EINVAL;	/* initialize for zero `count' case */

	for (j = 0; j < count; j += bbmap ? 1 : count) {
		if (bbmap)
			bbno = bbmap->b[j];
		if (lseek64(x.dfd, bbno << BBSHIFT, SEEK_SET) < 0) {
			rval = errno;
			dbprintf(_("can't seek in filesystem at bb %lld\n"), bbno);
			return rval;
		}
		c = BBTOB(bbmap ? 1 : count);
		i = (int)write(x.dfd, (char *)bufp + BBTOB(j), c);
		if (i < 0) {
			rval = errno;
		} else if (i < c) {
			rval = -1;
		} else
			rval = 0;
		if (rval)
			break;
	}
	return rval;
}

int
read_bbs(
	__int64_t	bbno,
	int		count,
	void		**bufp,
	bbmap_t		*bbmap)
{
	void		*buf;
	int		c;
	int		i;
	int		j;
	int		rval = EINVAL;

	if (count <= 0)
		count = 1;

	c = BBTOB(count);
	if (*bufp == NULL)
		buf = xmalloc(c);
	else
		buf = *bufp;
	for (j = 0; j < count; j += bbmap ? 1 : count) {
		if (bbmap)
			bbno = bbmap->b[j];
		if (lseek64(x.dfd, bbno << BBSHIFT, SEEK_SET) < 0) {
			rval = errno;
			dbprintf(_("can't seek in filesystem at bb %lld\n"), bbno);
			if (*bufp == NULL)
				xfree(buf);
			buf = NULL;
		} else {
			c = BBTOB(bbmap ? 1 : count);
			i = (int)read(x.dfd, (char *)buf + BBTOB(j), c);
			if (i < 0) {
				rval = errno;
				if (*bufp == NULL)
					xfree(buf);
				buf = NULL;
			} else if (i < c) {
				rval = -1;
				if (*bufp == NULL)
					xfree(buf);
				buf = NULL;
			} else
				rval = 0;
		}
		if (buf == NULL)
			break;
	}
	if (*bufp == NULL)
		*bufp = buf;
	return rval;
}

void
write_cur(void)
{
	int ret;

	if (iocur_sp < 0) {
		dbprintf(_("nothing to write\n"));
		return;
	}
	ret = write_bbs(iocur_top->bb, iocur_top->blen, iocur_top->buf,
		iocur_top->use_bbmap ? &iocur_top->bbmap : NULL);
	if (ret == -1)
		dbprintf(_("incomplete write, block: %lld\n"),
			 (iocur_base + iocur_sp)->bb);
	else if (ret != 0)
		dbprintf(_("write error: %s\n"), strerror(ret));
	/* re-read buffer from disk */
	ret = read_bbs(iocur_top->bb, iocur_top->blen, &iocur_top->buf,
		iocur_top->use_bbmap ? &iocur_top->bbmap : NULL);
	if (ret == -1)
		dbprintf(_("incomplete read, block: %lld\n"),
			 (iocur_base + iocur_sp)->bb);
	else if (ret != 0)
		dbprintf(_("read error: %s\n"), strerror(ret));
}

void
set_cur(
	const typ_t	*t,
	__int64_t	d,
	int		c,
	int             ring_flag,
	bbmap_t		*bbmap)
{
	xfs_ino_t	dirino;
	xfs_ino_t	ino;
	__uint16_t	mode;

	if (iocur_sp < 0) {
		dbprintf(_("set_cur no stack element to set\n"));
		return;
	}

#ifdef DEBUG
	if (bbmap)
		printf(_("xfs_db got a bbmap for %lld\n"), (long long)d);
#endif
	ino = iocur_top->ino;
	dirino = iocur_top->dirino;
	mode = iocur_top->mode;
	pop_cur();
	push_cur();
	if (read_bbs(d, c, &iocur_top->buf, bbmap))
		return;
	iocur_top->bb = d;
	iocur_top->blen = c;
	iocur_top->boff = 0;
	iocur_top->data = iocur_top->buf;
	iocur_top->len = BBTOB(c);
	iocur_top->off = d << BBSHIFT;
	iocur_top->typ = cur_typ = t;
	iocur_top->ino = ino;
	iocur_top->dirino = dirino;
	iocur_top->mode = mode;
	if ((iocur_top->use_bbmap = (bbmap != NULL)))
		iocur_top->bbmap = *bbmap;

	/* store location in ring */
	if (ring_flag)
		ring_add();
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
	char	tagbuf[8];

	for (i = iocur_sp; i > 0; i--) {
		snprintf(tagbuf, sizeof(tagbuf), "%d: ", i);
		print_iocur(tagbuf, &iocur_base[i]);
	}
	return 0;
}
