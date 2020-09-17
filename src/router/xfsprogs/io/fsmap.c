// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "libfrog/paths.h"
#include "io.h"
#include "input.h"
#include "libfrog/fsgeom.h"

static cmdinfo_t	fsmap_cmd;
static dev_t		xfs_data_dev;

static void
fsmap_help(void)
{
	printf(_(
"\n"
" Prints the block mapping for the filesystem hosting the current file"
"\n"
" fsmap prints the map of disk blocks used by the whole filesystem.\n"
" When possible, owner and offset information will be included in the\n"
" space report.\n"
"\n"
" By default, each line of the listing takes the following form:\n"
"     extent: major:minor [startblock..endblock]: owner startoffset..endoffset length\n"
" The owner field is either an inode number or a special value.\n"
" All the file offsets and disk blocks are in units of 512-byte blocks.\n"
" -d -- query only the data device (default).\n"
" -l -- query only the log device.\n"
" -r -- query only the realtime device.\n"
" -n -- query n extents at a time.\n"
" -m -- output machine-readable format.\n"
" -v -- Verbose information, show AG and offsets.  Show flags legend on 2nd -v\n"
"\n"
"The optional start and end arguments require one of -d, -l, or -r to be set.\n"
"\n"));
}

#define OWNER_BUF_SZ	32
static const char *
special_owner(
	int64_t		owner,
	char		*buf)
{
	switch (owner) {
	case XFS_FMR_OWN_FREE:
		return _("free space");
	case XFS_FMR_OWN_UNKNOWN:
		return _("unknown");
	case XFS_FMR_OWN_FS:
		return _("static fs metadata");
	case XFS_FMR_OWN_LOG:
		return _("journalling log");
	case XFS_FMR_OWN_AG:
		return _("per-AG metadata");
	case XFS_FMR_OWN_INOBT:
		return _("inode btree");
	case XFS_FMR_OWN_INODES:
		return _("inodes");
	case XFS_FMR_OWN_REFC:
		return _("refcount btree");
	case XFS_FMR_OWN_COW:
		return _("cow reservation");
	case XFS_FMR_OWN_DEFECTIVE:
		return _("defective");
	default:
		snprintf(buf, OWNER_BUF_SZ, _("special %u:%u"),
				FMR_OWNER_TYPE(owner), FMR_OWNER_CODE(owner));
		return buf;
	}
}

static void
dump_map(
	unsigned long long	*nr,
	struct fsmap_head	*head)
{
	unsigned long long	i;
	struct fsmap		*p;
	char			owner[OWNER_BUF_SZ];
	char			*fork;

	for (i = 0, p = head->fmh_recs; i < head->fmh_entries; i++, p++) {
		printf("\t%llu: %u:%u [%lld..%lld]: ", i + (*nr),
			major(p->fmr_device), minor(p->fmr_device),
			(long long)BTOBBT(p->fmr_physical),
			(long long)BTOBBT(p->fmr_physical + p->fmr_length - 1));
		fork = (p->fmr_flags & FMR_OF_ATTR_FORK) ? _("attr") : _("data");
		if (p->fmr_flags & FMR_OF_SPECIAL_OWNER)
			printf("%s", special_owner(p->fmr_owner, owner));
		else if (p->fmr_flags & FMR_OF_EXTENT_MAP)
			printf(_("inode %lld %s extent map"),
				(long long) p->fmr_owner, fork);
		else
			printf(_("inode %lld %s %lld..%lld"),
				(long long)p->fmr_owner, fork,
				(long long)BTOBBT(p->fmr_offset),
				(long long)BTOBBT(p->fmr_offset + p->fmr_length - 1));
		printf(_(" %lld\n"),
			(long long)BTOBBT(p->fmr_length));
	}

	(*nr) += head->fmh_entries;
}

static void
dump_map_machine(
	unsigned long long	*nr,
	struct fsmap_head	*head)
{
	unsigned long long	i;
	struct fsmap		*p;
	char			*fork;

	printf(_("EXT,MAJOR,MINOR,PSTART,PEND,OWNER,OSTART,OEND,LENGTH\n"));
	for (i = 0, p = head->fmh_recs; i < head->fmh_entries; i++, p++) {
		printf("%llu,%u,%u,%lld,%lld,", i + (*nr),
			major(p->fmr_device), minor(p->fmr_device),
			(long long)BTOBBT(p->fmr_physical),
			(long long)BTOBBT(p->fmr_physical + p->fmr_length - 1));
		fork = (p->fmr_flags & FMR_OF_ATTR_FORK) ? "attr" : "data";
		if (p->fmr_flags & FMR_OF_SPECIAL_OWNER)
			printf("special_%u:%u,,,", FMR_OWNER_TYPE(p->fmr_owner),
				FMR_OWNER_CODE(p->fmr_owner));
		else if (p->fmr_flags & FMR_OF_EXTENT_MAP)
			printf(_("inode_%lld_%s_bmbt,,,"),
				(long long) p->fmr_owner, fork);
		else
			printf(_("inode_%lld_%s,%lld,%lld,"),
				(long long)p->fmr_owner, fork,
				(long long)BTOBBT(p->fmr_offset),
				(long long)BTOBBT(p->fmr_offset + p->fmr_length - 1));
		printf("%lld\n",
			(long long)BTOBBT(p->fmr_length));
	}

	(*nr) += head->fmh_entries;
}

/*
 * Verbose mode displays:
 *   extent: major:minor [startblock..endblock]: startoffset..endoffset \
 *	ag# (agoffset..agendoffset) totalbbs flags
 */
#define MINRANGE_WIDTH	16
#define MINAG_WIDTH	2
#define MINTOT_WIDTH	5
#define NFLG		7		/* count of flags */
#define	FLG_NULL	00000000	/* Null flag */
#define	FLG_ATTR_FORK	01000000	/* attribute fork */
#define	FLG_SHARED	00100000	/* shared extent */
#define	FLG_PRE		00010000	/* Unwritten extent */
#define	FLG_BSU		00001000	/* Not on begin of stripe unit  */
#define	FLG_ESU		00000100	/* Not on end   of stripe unit  */
#define	FLG_BSW		00000010	/* Not on begin of stripe width */
#define	FLG_ESW		00000001	/* Not on end   of stripe width */
static void
dump_map_verbose(
	unsigned long long	*nr,
	struct fsmap_head	*head,
	bool			*dumped_flags,
	struct xfs_fsop_geom	*fsgeo)
{
	unsigned long long	i;
	struct fsmap		*p;
	int			agno;
	off64_t			agoff, bperag;
	int			foff_w, boff_w, aoff_w, tot_w, agno_w, own_w;
	int			nr_w, dev_w;
	char			rbuf[40], bbuf[40], abuf[40], obuf[40];
	char			nbuf[40], dbuf[40], gbuf[40];
	char			owner[OWNER_BUF_SZ];
	int			sunit, swidth;
	int			flg = 0;

	foff_w = boff_w = aoff_w = own_w = MINRANGE_WIDTH;
	dev_w = 3;
	nr_w = 4;
	tot_w = MINTOT_WIDTH;
	bperag = (off64_t)fsgeo->agblocks *
		  (off64_t)fsgeo->blocksize;
	sunit = (fsgeo->sunit * fsgeo->blocksize);
	swidth = (fsgeo->swidth * fsgeo->blocksize);

	/*
	 * Go through the extents and figure out the width
	 * needed for all columns.
	 */
	for (i = 0, p = head->fmh_recs; i < head->fmh_entries; i++, p++) {
		if (p->fmr_flags & FMR_OF_PREALLOC ||
		    p->fmr_flags & FMR_OF_ATTR_FORK ||
		    p->fmr_flags & FMR_OF_SHARED)
			flg = 1;
		if (sunit &&
		    (p->fmr_physical  % sunit != 0 ||
		     ((p->fmr_physical + p->fmr_length) % sunit) != 0 ||
		     p->fmr_physical % swidth != 0 ||
		     ((p->fmr_physical + p->fmr_length) % swidth) != 0))
			flg = 1;
		if (flg)
			*dumped_flags = true;
		snprintf(nbuf, sizeof(nbuf), "%llu", (*nr) + i);
		nr_w = max(nr_w, strlen(nbuf));
		if (head->fmh_oflags & FMH_OF_DEV_T)
			snprintf(dbuf, sizeof(dbuf), "%u:%u",
				major(p->fmr_device),
				minor(p->fmr_device));
		else
			snprintf(dbuf, sizeof(dbuf), "0x%x", p->fmr_device);
		dev_w = max(dev_w, strlen(dbuf));
		snprintf(bbuf, sizeof(bbuf), "[%lld..%lld]:",
			(long long)BTOBBT(p->fmr_physical),
			(long long)BTOBBT(p->fmr_physical + p->fmr_length - 1));
		boff_w = max(boff_w, strlen(bbuf));
		if (p->fmr_flags & FMR_OF_SPECIAL_OWNER)
			own_w = max(own_w, strlen(
					special_owner(p->fmr_owner, owner)));
		else {
			snprintf(obuf, sizeof(obuf), "%lld",
				(long long)p->fmr_owner);
			own_w = max(own_w, strlen(obuf));
		}
		if (p->fmr_flags & FMR_OF_EXTENT_MAP)
			foff_w = max(foff_w, strlen(_("extent_map")));
		else if (p->fmr_flags & FMR_OF_SPECIAL_OWNER)
			;
		else {
			snprintf(rbuf, sizeof(rbuf), "%lld..%lld",
				(long long)BTOBBT(p->fmr_offset),
				(long long)BTOBBT(p->fmr_offset + p->fmr_length - 1));
			foff_w = max(foff_w, strlen(rbuf));
		}
		if (p->fmr_device == xfs_data_dev) {
			agno = p->fmr_physical / bperag;
			agoff = p->fmr_physical - (agno * bperag);
			snprintf(abuf, sizeof(abuf),
				"(%lld..%lld)",
				(long long)BTOBBT(agoff),
				(long long)BTOBBT(agoff + p->fmr_length - 1));
		} else
			abuf[0] = 0;
		aoff_w = max(aoff_w, strlen(abuf));
		tot_w = max(tot_w,
			numlen(BTOBBT(p->fmr_length), 10));
	}
	agno_w = max(MINAG_WIDTH, numlen(fsgeo->agcount, 10));
	if (*nr == 0)
		printf("%*s: %-*s %-*s %-*s %-*s %*s %-*s %*s%s\n",
			nr_w, _("EXT"),
			dev_w, _("DEV"),
			boff_w, _("BLOCK-RANGE"),
			own_w, _("OWNER"),
			foff_w, _("FILE-OFFSET"),
			agno_w, _("AG"),
			aoff_w, _("AG-OFFSET"),
			tot_w, _("TOTAL"),
			flg ? _(" FLAGS") : "");
	for (i = 0, p = head->fmh_recs; i < head->fmh_entries; i++, p++) {
		flg = FLG_NULL;
		if (p->fmr_flags & FMR_OF_PREALLOC)
			flg |= FLG_PRE;
		if (p->fmr_flags & FMR_OF_ATTR_FORK)
			flg |= FLG_ATTR_FORK;
		if (p->fmr_flags & FMR_OF_SHARED)
			flg |= FLG_SHARED;
		/*
		 * If striping enabled, determine if extent starts/ends
		 * on a stripe unit boundary.
		 */
		if (sunit) {
			if (p->fmr_physical  % sunit != 0)
				flg |= FLG_BSU;
			if (((p->fmr_physical +
			      p->fmr_length ) % sunit ) != 0)
				flg |= FLG_ESU;
			if (p->fmr_physical % swidth != 0)
				flg |= FLG_BSW;
			if (((p->fmr_physical +
			      p->fmr_length ) % swidth ) != 0)
				flg |= FLG_ESW;
		}
		if (head->fmh_oflags & FMH_OF_DEV_T)
			snprintf(dbuf, sizeof(dbuf), "%u:%u",
				major(p->fmr_device),
				minor(p->fmr_device));
		else
			snprintf(dbuf, sizeof(dbuf), "0x%x", p->fmr_device);
		snprintf(bbuf, sizeof(bbuf), "[%lld..%lld]:",
			(long long)BTOBBT(p->fmr_physical),
			(long long)BTOBBT(p->fmr_physical + p->fmr_length - 1));
		if (p->fmr_flags & FMR_OF_SPECIAL_OWNER) {
			snprintf(obuf, sizeof(obuf), "%s",
				special_owner(p->fmr_owner, owner));
			snprintf(rbuf, sizeof(rbuf), " ");
		} else {
			snprintf(obuf, sizeof(obuf), "%lld",
				(long long)p->fmr_owner);
			snprintf(rbuf, sizeof(rbuf), "%lld..%lld",
				(long long)BTOBBT(p->fmr_offset),
				(long long)BTOBBT(p->fmr_offset + p->fmr_length - 1));
		}
		if (p->fmr_device == xfs_data_dev) {
			agno = p->fmr_physical / bperag;
			agoff = p->fmr_physical - (agno * bperag);
			snprintf(abuf, sizeof(abuf),
				"(%lld..%lld)",
				(long long)BTOBBT(agoff),
				(long long)BTOBBT(agoff + p->fmr_length - 1));
			snprintf(gbuf, sizeof(gbuf),
				"%lld",
				(long long)agno);
		} else {
			abuf[0] = 0;
			gbuf[0] = 0;
		}
		if (p->fmr_flags & FMR_OF_EXTENT_MAP)
			printf("%*llu: %-*s %-*s %-*s %-*s %-*s %-*s %*lld\n",
				nr_w, (*nr) + i,
				dev_w, dbuf,
				boff_w, bbuf,
				own_w, obuf,
				foff_w, _("extent map"),
				agno_w, gbuf,
				aoff_w, abuf,
				tot_w, (long long)BTOBBT(p->fmr_length));
		else {
			printf("%*llu: %-*s %-*s %-*s %-*s", nr_w, (*nr) + i,
				dev_w, dbuf, boff_w, bbuf, own_w, obuf,
				foff_w, rbuf);
			printf(" %-*s %-*s", agno_w, gbuf,
				aoff_w, abuf);
			printf(" %*lld", tot_w,
				(long long)BTOBBT(p->fmr_length));
			if (flg == FLG_NULL)
				printf("\n");
			else
				printf(" %-*.*o\n", NFLG, NFLG, flg);
		}
	}

	(*nr) += head->fmh_entries;
}

static void
dump_verbose_key(void)
{
	printf(_(" FLAG Values:\n"));
	printf(_("    %*.*o Attribute fork\n"),
		NFLG+1, NFLG+1, FLG_ATTR_FORK);
	printf(_("    %*.*o Shared extent\n"),
		NFLG+1, NFLG+1, FLG_SHARED);
	printf(_("    %*.*o Unwritten preallocated extent\n"),
		NFLG+1, NFLG+1, FLG_PRE);
	printf(_("    %*.*o Doesn't begin on stripe unit\n"),
		NFLG+1, NFLG+1, FLG_BSU);
	printf(_("    %*.*o Doesn't end   on stripe unit\n"),
		NFLG+1, NFLG+1, FLG_ESU);
	printf(_("    %*.*o Doesn't begin on stripe width\n"),
		NFLG+1, NFLG+1, FLG_BSW);
	printf(_("    %*.*o Doesn't end   on stripe width\n"),
		NFLG+1, NFLG+1, FLG_ESW);
}

static int
fsmap_f(
	int			argc,
	char			**argv)
{
	struct fsmap		*p;
	struct fsmap_head	*nhead;
	struct fsmap_head	*head;
	struct fsmap		*l, *h;
	struct xfs_fsop_geom	fsgeo;
	long long		start = 0;
	long long		end = -1;
	int			nmap_size;
	int			map_size;
	int			nflag = 0;
	int			vflag = 0;
	int			mflag = 0;
	int			i = 0;
	int			c;
	unsigned long long	nr = 0;
	size_t			fsblocksize, fssectsize;
	struct fs_path		*fs;
	static bool		tab_init;
	bool			dumped_flags = false;
	int			dflag, lflag, rflag;

	init_cvtnum(&fsblocksize, &fssectsize);

	dflag = lflag = rflag = 0;
	while ((c = getopt(argc, argv, "dlmn:rv")) != EOF) {
		switch (c) {
		case 'd':	/* data device */
			dflag = 1;
			break;
		case 'l':	/* log device */
			lflag = 1;
			break;
		case 'm':	/* machine readable format */
			mflag++;
			break;
		case 'n':	/* number of extents specified */
			nflag = cvt_u32(optarg, 10);
			if (errno)
				return command_usage(&fsmap_cmd);
			break;
		case 'r':	/* rt device */
			rflag = 1;
			break;
		case 'v':	/* Verbose output */
			vflag++;
			break;
		default:
			exitcode = 1;
			return command_usage(&fsmap_cmd);
		}
	}

	if ((dflag + lflag + rflag > 1) || (mflag > 0 && vflag > 0) ||
	    (argc > optind && dflag + lflag + rflag == 0)) {
		exitcode = 1;
		return command_usage(&fsmap_cmd);
	}

	if (argc > optind) {
		start = cvtnum(fsblocksize, fssectsize, argv[optind]);
		if (start < 0) {
			fprintf(stderr,
				_("Bad rmap start_bblock %s.\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
		start <<= BBSHIFT;
	}

	if (argc > optind + 1) {
		end = cvtnum(fsblocksize, fssectsize, argv[optind + 1]);
		if (end < 0) {
			fprintf(stderr,
				_("Bad rmap end_bblock %s.\n"),
				argv[optind + 1]);
			exitcode = 1;
			return 0;
		}
		end <<= BBSHIFT;
	}

	if (vflag) {
		c = -xfrog_geometry(file->fd, &fsgeo);
		if (c) {
			fprintf(stderr,
				_("%s: can't get geometry [\"%s\"]: %s\n"),
				progname, file->name, strerror(c));
			exitcode = 1;
			return 0;
		}
	}

	map_size = nflag ? nflag : 131072 / sizeof(struct fsmap);
	head = malloc(fsmap_sizeof(map_size));
	if (head == NULL) {
		fprintf(stderr, _("%s: malloc of %zu bytes failed.\n"),
			progname, fsmap_sizeof(map_size));
		exitcode = 1;
		return 0;
	}

	memset(head, 0, sizeof(*head));
	l = head->fmh_keys;
	h = head->fmh_keys + 1;
	if (dflag) {
		l->fmr_device = h->fmr_device = file->fs_path.fs_datadev;
	} else if (lflag) {
		l->fmr_device = h->fmr_device = file->fs_path.fs_logdev;
	} else if (rflag) {
		l->fmr_device = h->fmr_device = file->fs_path.fs_rtdev;
	} else {
		l->fmr_device = 0;
		h->fmr_device = UINT_MAX;
	}
	l->fmr_physical = start;
	h->fmr_physical = end;
	h->fmr_owner = ULLONG_MAX;
	h->fmr_flags = UINT_MAX;
	h->fmr_offset = ULLONG_MAX;

	/* Count mappings */
	if (!nflag) {
		head->fmh_count = 0;
		i = ioctl(file->fd, FS_IOC_GETFSMAP, head);
		if (i < 0) {
			fprintf(stderr, _("%s: xfsctl(XFS_IOC_GETFSMAP)"
				" iflags=0x%x [\"%s\"]: %s\n"),
				progname, head->fmh_iflags, file->name,
				strerror(errno));
			exitcode = 1;
			free(head);
			return 0;
		}
		if (head->fmh_entries > map_size + 2) {
			map_size = 11ULL * head->fmh_entries / 10;
			nmap_size = map_size > (1 << 24) ? (1 << 24) : map_size;
			nhead = realloc(head, fsmap_sizeof(nmap_size));
			if (nhead == NULL) {
				fprintf(stderr,
					_("%s: cannot realloc %zu bytes\n"),
					progname, fsmap_sizeof(nmap_size));
			} else {
				head = nhead;
				map_size = nmap_size;
			}
		}
	}

	/*
	 * If this is an XFS filesystem, remember the data device.
	 * (We report AG number/block for data device extents on XFS).
	 */
	if (!tab_init) {
		fs_table_initialise(0, NULL, 0, NULL);
		tab_init = true;
	}
	fs = fs_table_lookup(file->name, FS_MOUNT_POINT);
	xfs_data_dev = fs ? fs->fs_datadev : 0;

	head->fmh_count = map_size;
	do {
		/* Get some extents */
		i = ioctl(file->fd, FS_IOC_GETFSMAP, head);
		if (i < 0) {
			fprintf(stderr, _("%s: xfsctl(XFS_IOC_GETFSMAP)"
				" iflags=0x%x [\"%s\"]: %s\n"),
				progname, head->fmh_iflags, file->name,
				strerror(errno));
			free(head);
			exitcode = 1;
			return 0;
		}

		if (head->fmh_entries == 0)
			break;

		if (vflag)
			dump_map_verbose(&nr, head, &dumped_flags, &fsgeo);
		else if (mflag)
			dump_map_machine(&nr, head);
		else
			dump_map(&nr, head);

		p = &head->fmh_recs[head->fmh_entries - 1];
		if (p->fmr_flags & FMR_OF_LAST)
			break;
		fsmap_advance(head);
	} while (true);

	if (dumped_flags)
		dump_verbose_key();

	free(head);
	return 0;
}

void
fsmap_init(void)
{
	fsmap_cmd.name = "fsmap";
	fsmap_cmd.cfunc = fsmap_f;
	fsmap_cmd.argmin = 0;
	fsmap_cmd.argmax = -1;
	fsmap_cmd.flags = CMD_NOMAP_OK | CMD_FLAG_FOREIGN_OK;
	fsmap_cmd.args = _("[-d|-l|-r] [-m|-v] [-n nx] [start] [end]");
	fsmap_cmd.oneline = _("print filesystem mapping for a range of blocks");
	fsmap_cmd.help = fsmap_help;

	add_command(&fsmap_cmd);
}
