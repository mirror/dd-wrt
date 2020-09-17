// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012 Red Hat, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "libfrog/fsgeom.h"
#include "command.h"
#include "init.h"
#include "libfrog/paths.h"
#include "space.h"
#include "input.h"

static cmdinfo_t trim_cmd;

/*
 * Trim unused space in xfs filesystem.
 */
static int
trim_f(
	int			argc,
	char			**argv)
{
	struct fstrim_range	trim = {0};
	struct xfs_fd		*xfd = &file->xfd;
	struct xfs_fsop_geom	*fsgeom = &xfd->fsgeom;
	xfs_agnumber_t		agno = 0;
	off64_t			offset = 0;
	ssize_t			length = 0;
	ssize_t			minlen = 0;
	int			aflag = 0;
	int			fflag = 0;
	int			ret;
	int			c;

	while ((c = getopt(argc, argv, "a:fm:")) != EOF) {
		switch (c) {
		case 'a':
			aflag = 1;
			agno = cvt_u32(optarg, 10);
			if (errno) {
				printf(_("bad agno value %s\n"), optarg);
				return command_usage(&trim_cmd);
			}
			break;
		case 'f':
			fflag = 1;
			break;
		case 'm':
			minlen = cvtnum(fsgeom->blocksize, fsgeom->sectsize,
					optarg);
			break;
		default:
			return command_usage(&trim_cmd);
		}
	}

	if (aflag && fflag)
		return command_usage(&trim_cmd);

	if (optind != argc - 2 && !(aflag || fflag))
		return command_usage(&trim_cmd);
	if (optind != argc) {
		offset = cvtnum(fsgeom->blocksize, fsgeom->sectsize,
				argv[optind]);
		length = cvtnum(fsgeom->blocksize, fsgeom->sectsize,
				argv[optind + 1]);
	} else if (agno) {
		offset = cvt_agbno_to_b(xfd, agno, 0);
		length = cvt_off_fsb_to_b(xfd, fsgeom->agblocks);
	} else {
		offset = 0;
		length = cvt_off_fsb_to_b(xfd, fsgeom->datablocks);
	}

	trim.start = offset;
	trim.len = length;
	trim.minlen = minlen;

	ret = ioctl(file->xfd.fd, FITRIM, (unsigned long)&trim);
	if (ret < 0) {
		fprintf(stderr, "%s: ioctl(FITRIM) [\"%s\"]: %s\n",
			progname, file->name, strerror(errno));
		exitcode = 1;
	}
	return 0;
}

static void
trim_help(void)
{
	printf(_(
"\n"
"Discard filesystem free space\n"
"\n"
" -a agno       -- trim all the freespace in the given AG agno\n"
" -f            -- trim all the freespace in the entire filesystem\n"
" offset length -- trim the freespace in the range {offset, length}\n"
" -m minlen     -- skip freespace extents smaller than minlen\n"
"\n"
"One of -a, -f, or the offset/length pair are required.\n"
"\n"));

}

void
trim_init(void)
{
	trim_cmd.name = "trim";
	trim_cmd.altname = "tr";
	trim_cmd.cfunc = trim_f;
	trim_cmd.argmin = 1;
	trim_cmd.argmax = 4;
	trim_cmd.args = "[-m minlen] ( -a agno | -f | offset length )";
	trim_cmd.flags = CMD_FLAG_ONESHOT;
	trim_cmd.oneline = _("Discard filesystem free space");
	trim_cmd.help = trim_help;

	add_command(&trim_cmd);
}

