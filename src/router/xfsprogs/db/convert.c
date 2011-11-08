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
#include "command.h"
#include "convert.h"
#include "output.h"
#include "init.h"

#define	M(A)	(1 << CT_ ## A)
#define	agblock_to_bytes(x)	\
	((__uint64_t)(x) << mp->m_sb.sb_blocklog)
#define	agino_to_bytes(x)	\
	((__uint64_t)(x) << mp->m_sb.sb_inodelog)
#define	agnumber_to_bytes(x)	\
	agblock_to_bytes((__uint64_t)(x) * mp->m_sb.sb_agblocks)
#define	daddr_to_bytes(x)	\
	((__uint64_t)(x) << BBSHIFT)
#define	fsblock_to_bytes(x)	\
	(agnumber_to_bytes(XFS_FSB_TO_AGNO(mp, (x))) + \
	 agblock_to_bytes(XFS_FSB_TO_AGBNO(mp, (x))))
#define	ino_to_bytes(x)		\
	(agnumber_to_bytes(XFS_INO_TO_AGNO(mp, (x))) + \
	 agino_to_bytes(XFS_INO_TO_AGINO(mp, (x))))
#define	inoidx_to_bytes(x)	\
	((__uint64_t)(x) << mp->m_sb.sb_inodelog)

typedef enum {
	CT_NONE = -1,
	CT_AGBLOCK,		/* xfs_agblock_t */
	CT_AGINO,		/* xfs_agino_t */
	CT_AGNUMBER,		/* xfs_agno_t */
	CT_BBOFF,		/* byte offset in daddr */
	CT_BLKOFF,		/* byte offset in fsb/agb */
	CT_BYTE,		/* byte in filesystem */
	CT_DADDR,		/* daddr_t */
	CT_FSBLOCK,		/* xfs_fsblock_t */
	CT_INO,			/* xfs_ino_t */
	CT_INOIDX,		/* index of inode in fsblock */
	CT_INOOFF,		/* byte offset in inode */
	NCTS
} ctype_t;

typedef struct ctydesc {
	ctype_t		ctype;
	int		allowed;
	const char	**names;
} ctydesc_t;

typedef union {
	xfs_agblock_t	agblock;
	xfs_agino_t	agino;
	xfs_agnumber_t	agnumber;
	int		bboff;
	int		blkoff;
	__uint64_t	byte;
	xfs_daddr_t	daddr;
	xfs_fsblock_t	fsblock;
	xfs_ino_t	ino;
	int		inoidx;
	int		inooff;
} cval_t;

static __uint64_t		bytevalue(ctype_t ctype, cval_t *val);
static int		convert_f(int argc, char **argv);
static int		getvalue(char *s, ctype_t ctype, cval_t *val);
static ctype_t		lookupcty(char *ctyname);

static const char	*agblock_names[] = { "agblock", "agbno", NULL };
static const char	*agino_names[] = { "agino", "aginode", NULL };
static const char	*agnumber_names[] = { "agnumber", "agno", NULL };
static const char	*bboff_names[] = { "bboff", "daddroff", NULL };
static const char	*blkoff_names[] = { "blkoff", "fsboff", "agboff",
					    NULL };
static const char	*byte_names[] = { "byte", "fsbyte", NULL };
static const char	*daddr_names[] = { "daddr", "bb", NULL };
static const char	*fsblock_names[] = { "fsblock", "fsb", "fsbno", NULL };
static const char	*ino_names[] = { "ino", "inode", NULL };
static const char	*inoidx_names[] = { "inoidx", "offset", NULL };
static const char	*inooff_names[] = { "inooff", "inodeoff", NULL };

static const ctydesc_t	ctydescs[NCTS] = {
	{ CT_AGBLOCK, M(AGNUMBER)|M(BBOFF)|M(BLKOFF)|M(INOIDX)|M(INOOFF),
	  agblock_names },
	{ CT_AGINO, M(AGNUMBER)|M(INOOFF), agino_names },
	{ CT_AGNUMBER,
	  M(AGBLOCK)|M(AGINO)|M(BBOFF)|M(BLKOFF)|M(INOIDX)|M(INOOFF),
	  agnumber_names },
	{ CT_BBOFF, M(AGBLOCK)|M(AGNUMBER)|M(DADDR)|M(FSBLOCK), bboff_names },
	{ CT_BLKOFF, M(AGBLOCK)|M(AGNUMBER)|M(FSBLOCK), blkoff_names },
	{ CT_BYTE, 0, byte_names },
	{ CT_DADDR, M(BBOFF), daddr_names },
	{ CT_FSBLOCK, M(BBOFF)|M(BLKOFF)|M(INOIDX), fsblock_names },
	{ CT_INO, M(INOOFF), ino_names },
	{ CT_INOIDX, M(AGBLOCK)|M(AGNUMBER)|M(FSBLOCK)|M(INOOFF),
	  inoidx_names },
	{ CT_INOOFF,
	  M(AGBLOCK)|M(AGINO)|M(AGNUMBER)|M(FSBLOCK)|M(INO)|M(INOIDX),
	  inooff_names },
};

static const cmdinfo_t	convert_cmd =
	{ "convert", NULL, convert_f, 3, 9, 0, "type num [type num]... type",
	  "convert from one address form to another", NULL };

static __uint64_t
bytevalue(ctype_t ctype, cval_t *val)
{
	switch (ctype) {
	case CT_AGBLOCK:
		return agblock_to_bytes(val->agblock);
	case CT_AGINO:
		return agino_to_bytes(val->agino);
	case CT_AGNUMBER:
		return agnumber_to_bytes(val->agnumber);
	case CT_BBOFF:
		return (__uint64_t)val->bboff;
	case CT_BLKOFF:
		return (__uint64_t)val->blkoff;
	case CT_BYTE:
		return val->byte;
	case CT_DADDR:
		return daddr_to_bytes(val->daddr);
	case CT_FSBLOCK:
		return fsblock_to_bytes(val->fsblock);
	case CT_INO:
		return ino_to_bytes(val->ino);
	case CT_INOIDX:
		return inoidx_to_bytes(val->inoidx);
	case CT_INOOFF:
		return (__uint64_t)val->inooff;
	case CT_NONE:
	case NCTS:
		break;
	}
	/* NOTREACHED */
	return 0;
}

static int
convert_f(int argc, char **argv)
{
	ctype_t		c;
	int		conmask;
	cval_t		cvals[NCTS];
	int		i;
	int		mask;
	__uint64_t	v;
	ctype_t		wtype;

	/* move past the "convert" command */
	argc--;
	argv++;

	if ((argc % 2) != 1) {
		dbprintf(_("bad argument count %d to convert, expected 3,5,7,9 "
			 "arguments\n"), argc);
		return 0;
	}
	if ((wtype = lookupcty(argv[argc - 1])) == CT_NONE) {
		dbprintf(_("unknown conversion type %s\n"), argv[argc - 1]);
		return 0;
	}

	for (i = mask = conmask = 0; i < (argc - 1) / 2; i++) {
		c = lookupcty(argv[i * 2]);
		if (c == CT_NONE) {
			dbprintf(_("unknown conversion type %s\n"), argv[i * 2]);
			return 0;
		}
		if (c == wtype) {
			dbprintf(_("result type same as argument\n"));
			return 0;
		}
		if (conmask & (1 << c)) {
			dbprintf(_("conflicting conversion type %s\n"),
				argv[i * 2]);
			return 0;
		}
		if (!getvalue(argv[i * 2 + 1], c, &cvals[c]))
			return 0;
		mask |= 1 << c;
		conmask |= ~ctydescs[c].allowed;
	}
	if (cur_agno != NULLAGNUMBER && (conmask & M(AGNUMBER)) == 0) {
		cvals[CT_AGNUMBER].agnumber = cur_agno;
		mask |= M(AGNUMBER);
		conmask |= ~ctydescs[CT_AGNUMBER].allowed;
	}
	v = 0;
	for (c = (ctype_t)0; c < NCTS; c++) {
		if (!(mask & (1 << c)))
			continue;
		v += bytevalue(c, &cvals[c]);
	}
	switch (wtype) {
	case CT_AGBLOCK:
		v = xfs_daddr_to_agbno(mp, v >> BBSHIFT);
		break;
	case CT_AGINO:
		v = (v >> mp->m_sb.sb_inodelog) %
		    (mp->m_sb.sb_agblocks << mp->m_sb.sb_inopblog);
		break;
	case CT_AGNUMBER:
		v = xfs_daddr_to_agno(mp, v >> BBSHIFT);
		break;
	case CT_BBOFF:
		v &= BBMASK;
		break;
	case CT_BLKOFF:
		v &= mp->m_blockmask;
		break;
	case CT_BYTE:
		break;
	case CT_DADDR:
		v >>= BBSHIFT;
		break;
	case CT_FSBLOCK:
		v = XFS_DADDR_TO_FSB(mp, v >> BBSHIFT);
		break;
	case CT_INO:
		v = XFS_AGINO_TO_INO(mp, xfs_daddr_to_agno(mp, v >> BBSHIFT),
			(v >> mp->m_sb.sb_inodelog) %
			(mp->m_sb.sb_agblocks << mp->m_sb.sb_inopblog));
		break;
	case CT_INOIDX:
		v = (v >> mp->m_sb.sb_inodelog) & (mp->m_sb.sb_inopblock - 1);
		break;
	case CT_INOOFF:
		v &= mp->m_sb.sb_inodesize - 1;
		break;
	case CT_NONE:
	case NCTS:
		/* NOTREACHED */
		break;
	}
	dbprintf("0x%llx (%llu)\n", v, v);
	return 0;
}

void
convert_init(void)
{
	add_command(&convert_cmd);
}

static int
getvalue(char *s, ctype_t ctype, cval_t *val)
{
	char		*p;
	__uint64_t	v;

	v = strtoull(s, &p, 0);
	if (*p != '\0') {
		dbprintf(_("%s is not a number\n"), s);
		return 0;
	}
	switch (ctype) {
	case CT_AGBLOCK:
		val->agblock = (xfs_agblock_t)v;
		break;
	case CT_AGINO:
		val->agino = (xfs_agino_t)v;
		break;
	case CT_AGNUMBER:
		val->agnumber = (xfs_agnumber_t)v;
		break;
	case CT_BBOFF:
		val->bboff = (int)v;
		break;
	case CT_BLKOFF:
		val->blkoff = (int)v;
		break;
	case CT_BYTE:
		val->byte = (__uint64_t)v;
		break;
	case CT_DADDR:
		val->daddr = (xfs_daddr_t)v;
		break;
	case CT_FSBLOCK:
		val->fsblock = (xfs_fsblock_t)v;
		break;
	case CT_INO:
		val->ino = (xfs_ino_t)v;
		break;
	case CT_INOIDX:
		val->inoidx = (int)v;
		break;
	case CT_INOOFF:
		val->inooff = (int)v;
		break;
	case CT_NONE:
	case NCTS:
		/* NOTREACHED */
		break;
	}
	return 1;
}

static ctype_t
lookupcty(char *ctyname)
{
	ctype_t		cty;
	const char	**name;

	for (cty = (ctype_t)0; cty < NCTS; cty++) {
		for (name = ctydescs[cty].names; *name; name++) {
			if (strcmp(ctyname, *name) == 0)
				return cty;
		}
	}
	return CT_NONE;
}
