/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include "init.h"
#include "io.h"

static cmdinfo_t fadvise_cmd;

static void
fadvise_help(void)
{
	printf(_(
"\n"
" advise the page cache about expected I/O patterns on the current file\n"
"\n"
" Modifies kernel page cache behaviour when operating on the current file.\n"
" The range arguments are required by some advise commands ([*] below).\n"
" With no arguments, the POSIX_FADV_NORMAL advice is implied.\n"
" -d -- don't need these pages (POSIX_FADV_DONTNEED) [*]\n"
" -n -- data will be accessed once (POSIX_FADV_NOREUSE) [*]\n"
" -r -- expect random page references (POSIX_FADV_RANDOM)\n"
" -s -- expect sequential page references (POSIX_FADV_SEQUENTIAL)\n"
" -w -- will need these pages (POSIX_FADV_WILLNEED) [*]\n"
" Notes: these interfaces are not supported in Linux kernels before 2.6.\n"
"   NORMAL sets the default readahead setting on the file.\n"
"   RANDOM sets the readahead setting on the file to zero.\n"
"   SEQUENTIAL sets double the default readahead setting on the file.\n"
"   WILLNEED and NOREUSE are equivalent, and force the maximum readahead.\n"
"\n"));
}

static int
fadvise_f(
	int		argc,
	char		**argv)
{
	off64_t		offset = 0, length = 0;
	int		c, range = 0, advise = POSIX_FADV_NORMAL;

	while ((c = getopt(argc, argv, "dnrsw")) != EOF) {
		switch (c) {
		case 'd':	/* Don't need these pages */
			advise = POSIX_FADV_DONTNEED;
			range = 1;
			break;
		case 'n':	/* Data will be accessed once */
			advise = POSIX_FADV_NOREUSE;
			range = 1;
			break;
		case 'r':	/* Expect random page references */
			advise = POSIX_FADV_RANDOM;
			range = 0;
			break;
		case 's':	/* Expect sequential page references */
			advise = POSIX_FADV_SEQUENTIAL;
			range = 0;
			break;
		case 'w':	/* Will need these pages */
			advise = POSIX_FADV_WILLNEED;
			range = 1;
			break;
		default:
			return command_usage(&fadvise_cmd);
		}
	}
	if (range) {
		size_t	blocksize, sectsize;

		if (optind != argc - 2)
			return command_usage(&fadvise_cmd);
		init_cvtnum(&blocksize, &sectsize);
		offset = cvtnum(blocksize, sectsize, argv[optind]);
		if (offset < 0) {
			printf(_("non-numeric offset argument -- %s\n"),
				argv[optind]);
			return 0;
		}
		optind++;
		length = cvtnum(blocksize, sectsize, argv[optind]);
		if (length < 0) {
			printf(_("non-numeric length argument -- %s\n"),
				argv[optind]);
			return 0;
		}
	} else if (optind != argc) {
		return command_usage(&fadvise_cmd);
	}

	if (posix_fadvise64(file->fd, offset, length, advise) < 0) {
		perror("fadvise");
		return 0;
	}
	return 0;
}

void
fadvise_init(void)
{
	fadvise_cmd.name = "fadvise";
	fadvise_cmd.cfunc = fadvise_f;
	fadvise_cmd.argmin = 0;
	fadvise_cmd.argmax = -1;
	fadvise_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fadvise_cmd.args = _("[-dnrsw] [off len]");
	fadvise_cmd.oneline = _("advisory commands for sections of a file");
	fadvise_cmd.help = fadvise_help;

	add_command(&fadvise_cmd);
}
