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
#include "command.h"
#include "input.h"
#include "init.h"
#include "path.h"
#include "space.h"

static cmdinfo_t prealloc_cmd;

/*
 * Control preallocation amounts.
 */
static int
prealloc_f(
	int	argc,
	char	**argv)
{
	struct xfs_fs_eofblocks eofb = {0};
	int	c;

	eofb.eof_version = XFS_EOFBLOCKS_VERSION;

	while ((c = getopt(argc, argv, "g:m:p:su:")) != EOF) {
		switch (c) {
		case 'g':
			eofb.eof_flags |= XFS_EOF_FLAGS_GID;
			eofb.eof_gid = cvt_u32(optarg, 10);
			if (errno)
				return command_usage(&prealloc_cmd);
			break;
		case 'u':
			eofb.eof_flags |= XFS_EOF_FLAGS_UID;
			eofb.eof_uid = cvt_u32(optarg, 10);
			if (errno)
				return command_usage(&prealloc_cmd);
			break;
		case 'p':
			eofb.eof_flags |= XFS_EOF_FLAGS_PRID;
			eofb.eof_prid = cvt_u32(optarg, 10);
			if (errno)
				return command_usage(&prealloc_cmd);
			break;
		case 's':
			eofb.eof_flags |= XFS_EOF_FLAGS_SYNC;
			break;
		case 'm':
			eofb.eof_flags |= XFS_EOF_FLAGS_MINFILESIZE;
			eofb.eof_min_file_size = cvtnum(file->geom.blocksize,
							file->geom.sectsize,
							optarg);
			break;
		case '?':
		default:
			return command_usage(&prealloc_cmd);
		}
	}
	if (optind != argc)
		return command_usage(&prealloc_cmd);

	if (ioctl(file->fd, XFS_IOC_FREE_EOFBLOCKS, &eofb) < 0) {
		fprintf(stderr, _("%s: XFS_IOC_FREE_EOFBLOCKS on %s: %s\n"),
			progname, file->name, strerror(errno));
	}
	return 0;
}

static void
prealloc_help(void)
{
	printf(_(
"\n"
"Remove speculative preallocation\n"
"\n"
" -g gid    -- remove prealloc on files matching group <gid>\n"
" -m minlen -- only consider files larger than <minlen>\n"
" -p prid   -- remove prealloc on files matching project <prid>\n"
" -s        -- wait for removal to complete\n"
" -u uid    -- remove prealloc on files matching user <uid>\n"
"\n"
"If none of -u, -g, or -p are specified, this command acts on all files.\n"
"minlen can take units.\n"
"\n"));

}

void
prealloc_init(void)
{
	prealloc_cmd.name = "prealloc";
	prealloc_cmd.altname = "prealloc";
	prealloc_cmd.cfunc = prealloc_f;
	prealloc_cmd.argmin = 1;
	prealloc_cmd.argmax = -1;
	prealloc_cmd.args = "[-s] [-u id] [-g id] [-p id] [-m minlen]";
	prealloc_cmd.flags = CMD_FLAG_ONESHOT;
	prealloc_cmd.oneline = _("Remove speculative preallocation");
	prealloc_cmd.help = prealloc_help;

	add_command(&prealloc_cmd);
}

