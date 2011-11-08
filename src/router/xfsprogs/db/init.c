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
#include <signal.h>
#include "command.h"
#include "init.h"
#include "input.h"
#include "io.h"
#include "init.h"
#include "sig.h"
#include "output.h"
#include "malloc.h"

static char	**cmdline;
static int	ncmdline;
char		*fsdevice;
int		blkbb;
int		exitcode;
int		expert_mode;
int		force;
xfs_mount_t	xmount;
xfs_mount_t	*mp;
libxfs_init_t	x;
xfs_agnumber_t	cur_agno = NULLAGNUMBER;

static void
usage(void)
{
	fprintf(stderr, _(
		"Usage: %s [-fFrxV] [-p prog] [-l logdev] [-c cmd]... device\n"
		), progname);
	exit(1);
}

void
init(
	int		argc,
	char		**argv)
{
	xfs_sb_t	*sbp;
	void		*bufp = NULL;
	int		c;

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
		case '?':
			usage();
			/*NOTREACHED*/
		}
	}
	if (optind + 1 != argc) {
		usage();
		/*NOTREACHED*/
	}

	fsdevice = argv[optind];
	if (!x.disfile)
		x.volname = fsdevice;
	else
		x.dname = fsdevice;

	if (!libxfs_init(&x)) {
		fputs(_("\nfatal error -- couldn't initialize XFS library\n"),
			stderr);
		exit(1);
	}

	if (read_bbs(XFS_SB_DADDR, 1, &bufp, NULL)) {
		fprintf(stderr, _("%s: %s is invalid (cannot read first 512 "
			"bytes)\n"), progname, fsdevice);
		exit(1);
	}

	/* copy SB from buffer to in-core, converting architecture as we go */
	libxfs_sb_from_disk(&xmount.m_sb, bufp);
	xfree(bufp);

	sbp = &xmount.m_sb;
	if (sbp->sb_magicnum != XFS_SB_MAGIC) {
		fprintf(stderr, _("%s: %s is not a valid XFS filesystem (unexpected SB magic number 0x%08x)\n"),
			progname, fsdevice, sbp->sb_magicnum);
		if (!force)
			exit(EXIT_FAILURE);
	}

	mp = libxfs_mount(&xmount, sbp, x.ddev, x.logdev, x.rtdev,
				LIBXFS_MOUNT_ROOTINOS | LIBXFS_MOUNT_DEBUGGER);
	if (!mp) {
		mp = libxfs_mount(&xmount, sbp, x.ddev, x.logdev, x.rtdev,
				LIBXFS_MOUNT_DEBUGGER);
		if (!mp) {
			fprintf(stderr, _("%s: device %s unusable (not an XFS "
				"filesystem?)\n"), progname, fsdevice);
			exit(1);
		}
	}
	blkbb = 1 << mp->m_blkbb_log;

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

	pushfile(stdin);
	init(argc, argv);

	for (i = 0; !done && i < ncmdline; i++) {
		v = breakline(cmdline[i], &c);
		if (c)
			done = command(c, v);
		xfree(v);
	}
	if (cmdline) {
		xfree(cmdline);
		return exitcode;
	}

	while (!done) {
		if ((input = fetchline()) == NULL)
			break;
		v = breakline(input, &c);
		if (c)
			done = command(c, v);
		doneline(input, v);
	}
	return exitcode;
}
