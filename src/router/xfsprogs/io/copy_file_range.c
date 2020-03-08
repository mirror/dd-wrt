/*
 *  Copyright (c) 2016 Netapp, Inc. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <sys/syscall.h>
#include <sys/uio.h>
#include <xfs/xfs.h>
#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t copy_range_cmd;

static void
copy_range_help(void)
{
	printf(_("\n\
 Copies a range of bytes from a file into the open file, overwriting any data\n\
 already there.\n\
\n\
 Example:\n\
 'copy_range -s 100 -d 200 -l 300 some_file' - copies 300 bytes from some_file\n\
                                               at offset 100 into the open\n\
					       file at offset 200\n\
 'copy_range some_file' - copies all bytes from some_file into the open file\n\
                          at position 0\n\
"));
}

static loff_t
copy_file_range2(int fd, loff_t *src, loff_t *dst, size_t len)
{
	loff_t ret;

	do {
		ret = syscall(__NR_copy_file_range, fd, src, file->fd, dst, len, 0);
		if (ret == -1) {
			perror("copy_range");
			return errno;
		} else if (ret == 0)
			break;
		len -= ret;
	} while (len > 0);

	return 0;
}

static off64_t
copy_src_filesize(int fd)
{
	struct stat st;

	if (fstat(fd, &st) < 0) {
		perror("fstat");
		return -1;
	};
	return st.st_size;
}

static int
copy_dst_truncate(void)
{
	int ret = ftruncate(file->fd, 0);
	if (ret < 0)
		perror("ftruncate");
	return ret;
}

static int
copy_range_f(int argc, char **argv)
{
	loff_t src = 0;
	loff_t dst = 0;
	size_t len = 0;
	char *sp;
	int opt;
	int ret;
	int fd;

	while ((opt = getopt(argc, argv, "s:d:l:")) != -1) {
		switch (opt) {
		case 's':
			src = strtoull(optarg, &sp, 10);
			if (!sp || sp == optarg) {
				printf(_("invalid source offset -- %s\n"), sp);
				return 0;
			}
			break;
		case 'd':
			dst = strtoull(optarg, &sp, 10);
			if (!sp || sp == optarg) {
				printf(_("invalid destination offset -- %s\n"), sp);
				return 0;
			}
			break;
		case 'l':
			len = strtoull(optarg, &sp, 10);
			if (!sp || sp == optarg) {
				printf(_("invalid length -- %s\n"), sp);
				return 0;
			}
			break;
		}
	}

	if (optind != argc - 1)
		return command_usage(&copy_range_cmd);

	fd = openfile(argv[optind], NULL, IO_READONLY, 0, NULL);
	if (fd < 0)
		return 0;

	if (src == 0 && dst == 0 && len == 0) {
		len = copy_src_filesize(fd);
		copy_dst_truncate();
	}

	ret = copy_file_range2(fd, &src, &dst, len);
	close(fd);
	return ret;
}

void
copy_range_init(void)
{
	copy_range_cmd.name = "copy_range";
	copy_range_cmd.cfunc = copy_range_f;
	copy_range_cmd.argmin = 1;
	copy_range_cmd.argmax = 7;
	copy_range_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	copy_range_cmd.args = _("[-s src_off] [-d dst_off] [-l len] src_file");
	copy_range_cmd.oneline = _("Copy a range of data between two files");
	copy_range_cmd.help = copy_range_help;

	add_command(&copy_range_cmd);
}
