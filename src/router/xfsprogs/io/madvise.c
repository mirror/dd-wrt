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
#include <sys/mman.h>
#include "init.h"
#include "io.h"

static cmdinfo_t madvise_cmd;

static void
madvise_help(void)
{
	printf(_(
"\n"
" advise the page cache about access patterns expected for a mapping\n"
"\n"
" Modifies page cache behavior when operating on the current mapping.\n"
" The range arguments are required by some advise commands ([*] below).\n"
" With no arguments, the POSIX_MADV_NORMAL advice is implied.\n"
" -d -- don't need these pages (POSIX_MADV_DONTNEED) [*]\n"
" -r -- expect random page references (POSIX_MADV_RANDOM)\n"
" -s -- expect sequential page references (POSIX_MADV_SEQUENTIAL)\n"
" -w -- will need these pages (POSIX_MADV_WILLNEED) [*]\n"
" Notes:\n"
"   NORMAL sets the default readahead setting on the file.\n"
"   RANDOM sets the readahead setting on the file to zero.\n"
"   SEQUENTIAL sets double the default readahead setting on the file.\n"
"   WILLNEED forces the maximum readahead.\n"
"\n"));
}

int
madvise_f(
	int		argc,
	char		**argv)
{
	off64_t		offset, llength;
	size_t		length;
	void		*start;
	int		advise = MADV_NORMAL, c;
	size_t		blocksize, sectsize;

	while ((c = getopt(argc, argv, "drsw")) != EOF) {
		switch (c) {
		case 'd':	/* Don't need these pages */
			advise = MADV_DONTNEED;
			break;
		case 'r':	/* Expect random page references */
			advise = MADV_RANDOM;
			break;
		case 's':	/* Expect sequential page references */
			advise = MADV_SEQUENTIAL;
			break;
		case 'w':	/* Will need these pages */
			advise = MADV_WILLNEED;
			break;
		default:
			return command_usage(&madvise_cmd);
		}
	}

	if (optind == argc) {
		offset = mapping->offset;
		length = mapping->length;
	} else if (optind == argc - 2) {
		init_cvtnum(&blocksize, &sectsize);
		offset = cvtnum(blocksize, sectsize, argv[optind]);
		if (offset < 0) {
			printf(_("non-numeric offset argument -- %s\n"),
				argv[optind]);
			return 0;
		}
		optind++;
		llength = cvtnum(blocksize, sectsize, argv[optind]);
		if (llength < 0) {
			printf(_("non-numeric length argument -- %s\n"),
				argv[optind]);
			return 0;
		} else if (llength > (size_t)llength) {
			printf(_("length argument too large -- %lld\n"),
				(long long)llength);
			return 0;
		} else
			length = (size_t)llength;
	} else {
		return command_usage(&madvise_cmd);
	}

	start = check_mapping_range(mapping, offset, length, 1);
	if (!start)
		return 0;

	if (madvise(start, length, advise) < 0) {
		perror("madvise");
		return 0;
	}
	return 0;
}

void
madvise_init(void)
{
	madvise_cmd.name = "madvise";
	madvise_cmd.altname = "ma";
	madvise_cmd.cfunc = madvise_f;
	madvise_cmd.argmin = 0;
	madvise_cmd.argmax = -1;
	madvise_cmd.flags = CMD_NOFILE_OK | CMD_FOREIGN_OK;
	madvise_cmd.args = _("[-drsw] [off len]");
	madvise_cmd.oneline = _("give advice about use of memory");
	madvise_cmd.help = madvise_help;

	add_command(&madvise_cmd);
}
