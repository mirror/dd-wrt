/*
 * Copyright (c) 2012 Red Hat, Inc.
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

#include "libxfs.h"
#include <linux/fs.h>
#include "command.h"
#include "init.h"
#include "path.h"
#include "space.h"
#include "input.h"

static cmdinfo_t trim_cmd;

/*
 * Trim unused space in xfs filesystem.
 */
static int
trim_f(
	int		argc,
	char		**argv)
{
	struct fstrim_range trim = {0};
	xfs_agnumber_t	agno = 0;
	off64_t		offset = 0;
	ssize_t		length = 0;
	ssize_t		minlen = 0;
	int		aflag = 0;
	int		fflag = 0;
	int		ret;
	int		c;

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
			minlen = cvtnum(file->geom.blocksize,
					file->geom.sectsize, optarg);
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
		offset = cvtnum(file->geom.blocksize, file->geom.sectsize,
				argv[optind]);
		length = cvtnum(file->geom.blocksize, file->geom.sectsize,
				argv[optind + 1]);
	} else if (agno) {
		offset = (off64_t)agno * file->geom.agblocks * file->geom.blocksize;
		length = file->geom.agblocks * file->geom.blocksize;
	} else {
		offset = 0;
		length = file->geom.datablocks * file->geom.blocksize;
	}

	trim.start = offset;
	trim.len = length;
	trim.minlen = minlen;

	ret = ioctl(file->fd, FITRIM, (unsigned long)&trim);
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

