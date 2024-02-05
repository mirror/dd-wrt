// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "libxlog.h"
#include <signal.h>
#include "command.h"
#include "init.h"
#include "input.h"
#include "io.h"
#include "init.h"
#include "sig.h"
#include "output.h"
#include "malloc.h"
#include "type.h"

static char		**cmdline;
static int		ncmdline;
char			*fsdevice;
int			blkbb;
int			exitcode;
int			expert_mode;
static int		force;
static struct xfs_mount	xmount;
struct xfs_mount	*mp;
static struct xlog	xlog;
xfs_agnumber_t		cur_agno = NULLAGNUMBER;

static void
usage(void)
{
	fprintf(stderr, _(
		"Usage: %s [-ifFrxV] [-p prog] [-l logdev] [-c cmd]... device\n"
		), progname);
	exit(1);
}

static void
init(
	int		argc,
	char		**argv)
{
	struct xfs_sb	*sbp;
	struct xfs_buf	*bp;
	unsigned int	agcount;
	int		c;
	int		error;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	progname = basename(argv[0]);
	while ((c = getopt(argc, argv, "c:fFip:rxVl:")) != EOF) {
		switch (c) {
		case 'c':
			cmdline = xrealloc(cmdline, (ncmdline+1)*sizeof(char*));
			cmdline[ncmdline++] = optarg;
			break;
		case 'f':
			x.disfile = 1;
			break;
		case 'F':
			force = 1;
			break;
		case 'i':
			x.isreadonly = (LIBXFS_ISREADONLY|LIBXFS_ISINACTIVE);
			break;
		case 'p':
			progname = optarg;
			break;
		case 'r':
			x.isreadonly = LIBXFS_ISREADONLY;
			break;
		case 'l':
			x.logname = optarg;
			break;
		case 'x':
			expert_mode = 1;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		default:
			usage();
		}
	}
	if (optind + 1 != argc)
		usage();

	fsdevice = argv[optind];
	if (!x.disfile)
		x.volname = fsdevice;
	else
		x.dname = fsdevice;
	x.isdirect = LIBXFS_DIRECT;

	x.bcache_flags = CACHE_MISCOMPARE_PURGE;
	if (!libxfs_init(&x)) {
		fputs(_("\nfatal error -- couldn't initialize XFS library\n"),
			stderr);
		exit(1);
	}

	/*
	 * Read the superblock, but don't validate it - we are a diagnostic
	 * tool and so need to be able to mount busted filesystems.
	 */
	memset(&xmount, 0, sizeof(struct xfs_mount));
	libxfs_buftarg_init(&xmount, x.ddev, x.logdev, x.rtdev);
	error = -libxfs_buf_read_uncached(xmount.m_ddev_targp, XFS_SB_DADDR,
			1 << (XFS_MAX_SECTORSIZE_LOG - BBSHIFT), 0, &bp, NULL);
	if (error) {
		fprintf(stderr, _("%s: %s is invalid (cannot read first 512 "
			"bytes)\n"), progname, fsdevice);
		exit(1);
	}

	/* copy SB from buffer to in-core, converting architecture as we go */
	libxfs_sb_from_disk(&xmount.m_sb, bp->b_addr);
	libxfs_buf_relse(bp);

	sbp = &xmount.m_sb;
	if (sbp->sb_magicnum != XFS_SB_MAGIC) {
		fprintf(stderr, _("%s: %s is not a valid XFS filesystem (unexpected SB magic number 0x%08x)\n"),
			progname, fsdevice, sbp->sb_magicnum);
		if (!force) {
			fprintf(stderr, _("Use -F to force a read attempt.\n"));
			exit(EXIT_FAILURE);
		}
	}

	agcount = sbp->sb_agcount;
	mp = libxfs_mount(&xmount, sbp, x.ddev, x.logdev, x.rtdev,
			  LIBXFS_MOUNT_DEBUGGER);
	if (!mp) {
		fprintf(stderr,
			_("%s: device %s unusable (not an XFS filesystem?)\n"),
			progname, fsdevice);
		exit(1);
	}
	mp->m_log = &xlog;
	blkbb = 1 << mp->m_blkbb_log;

	/* Did we limit a broken agcount in libxfs_mount? */
	if (sbp->sb_agcount != agcount)
		exitcode = 1;

	/*
	 * xfs_check needs corrected incore superblock values
	 */
	if (sbp->sb_rootino != NULLFSINO &&
	    xfs_has_lazysbcount(mp)) {
		int error = -libxfs_initialize_perag_data(mp, sbp->sb_agcount);
		if (error) {
			fprintf(stderr,
	_("%s: cannot init perag data (%d). Continuing anyway.\n"),
				progname, error);
		}
	}

	if (xfs_has_sparseinodes(mp))
		type_set_tab_spcrc();
	else if (xfs_has_crc(mp))
		type_set_tab_crc();

	push_cur();
	init_commands();
	init_sig();
}

int
main(
	int	argc,
	char	**argv)
{
	int	c, i, done = 0;
	char	*input;
	char	**v;
	int	start_iocur_sp;

	init(argc, argv);
	start_iocur_sp = iocur_sp;

	for (i = 0; !done && i < ncmdline; i++) {
		v = breakline(cmdline[i], &c);
		if (c)
			done = command(c, v);
		xfree(v);
	}
	if (cmdline) {
		xfree(cmdline);
		goto close_devices;
	}

	pushfile(stdin);
	while (!done) {
		if ((input = fetchline()) == NULL)
			break;
		v = breakline(input, &c);
		if (c)
			done = command(c, v);
		doneline(input, v);
	}

close_devices:
	/*
	 * Make sure that we pop the all the buffer contexts we hold so that
	 * they are released before we purge the caches during unmount.
	 */
	while (iocur_sp > start_iocur_sp)
		pop_cur();
	libxfs_umount(mp);
	libxfs_destroy(&x);

	return exitcode;
}
