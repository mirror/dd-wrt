// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (c) 2016 Netapp, Inc. All rights reserved.
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
 'copy_range -f 2' - copies all bytes from open file 2 into the current open file\n\
                          at position 0\n\
"));
}

/*
 * Issue a raw copy_file_range syscall; for our test program we don't want the
 * glibc buffered copy fallback.
 */
static loff_t
copy_file_range_cmd(int fd, long long *src_off, long long *dst_off, size_t len)
{
	loff_t ret;

	do {
		ret = syscall(__NR_copy_file_range, fd, src_off,
				file->fd, dst_off, len, 0);
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
copy_range_f(int argc, char **argv)
{
	long long src_off = 0;
	long long dst_off = 0;
	long long llen;
	size_t len = 0;
	bool len_specified = false;
	int opt;
	int ret;
	int fd;
	int src_path_arg = 1;
	int src_file_nr = 0;
	size_t fsblocksize, fssectsize;

	init_cvtnum(&fsblocksize, &fssectsize);

	while ((opt = getopt(argc, argv, "s:d:l:f:")) != -1) {
		switch (opt) {
		case 's':
			src_off = cvtnum(fsblocksize, fssectsize, optarg);
			if (src_off < 0) {
				printf(_("invalid source offset -- %s\n"), optarg);
				exitcode = 1;
				return 0;
			}
			break;
		case 'd':
			dst_off = cvtnum(fsblocksize, fssectsize, optarg);
			if (dst_off < 0) {
				printf(_("invalid destination offset -- %s\n"), optarg);
				exitcode = 1;
				return 0;
			}
			break;
		case 'l':
			llen = cvtnum(fsblocksize, fssectsize, optarg);
			if (llen == -1LL) {
				printf(_("invalid length -- %s\n"), optarg);
				exitcode = 1;
				return 0;
			}
			/*
			 * If size_t can't hold what's in llen, report a
			 * length overflow.
			 */
			if ((size_t)llen != llen) {
				errno = EOVERFLOW;
				perror("copy_range");
				exitcode = 1;
				return 0;
			}
			len = llen;
			len_specified = true;
			break;
		case 'f':
			src_file_nr = atoi(argv[1]);
			if (src_file_nr < 0 || src_file_nr >= filecount) {
				printf(_("file value %d is out of range (0-%d)\n"),
					src_file_nr, filecount - 1);
				exitcode = 1;
				return 0;
			}
			/* Expect no src_path arg */
			src_path_arg = 0;
			break;
		default:
			exitcode = 1;
			return command_usage(&copy_range_cmd);
		}
	}

	if (optind != argc - src_path_arg) {
		exitcode = 1;
		return command_usage(&copy_range_cmd);
	}

	if (src_path_arg) {
		fd = openfile(argv[optind], NULL, IO_READONLY, 0, NULL);
		if (fd < 0) {
			exitcode = 1;
			return 0;
		}
	} else {
		fd = filetable[src_file_nr].fd;
	}

	if (!len_specified) {
		off64_t	sz;

		sz = copy_src_filesize(fd);
		if (sz < 0 || (unsigned long long)sz > SIZE_MAX) {
			ret = 1;
			goto out;
		}
		if (sz > src_off)
			len = sz - src_off;
	}

	ret = copy_file_range_cmd(fd, &src_off, &dst_off, len);
out:
	close(fd);
	if (ret < 0)
		exitcode = 1;
	return ret;
}

void
copy_range_init(void)
{
	copy_range_cmd.name = "copy_range";
	copy_range_cmd.cfunc = copy_range_f;
	copy_range_cmd.argmin = 1;
	copy_range_cmd.argmax = 8;
	copy_range_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	copy_range_cmd.args = _("[-s src_off] [-d dst_off] [-l len] src_file | -f N");
	copy_range_cmd.oneline = _("Copy a range of data between two files");
	copy_range_cmd.help = copy_range_help;

	add_command(&copy_range_cmd);
}
