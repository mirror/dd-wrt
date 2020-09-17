// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include <sys/sendfile.h>
#include "init.h"
#include "io.h"

static cmdinfo_t sendfile_cmd;

static void
sendfile_help(void)
{
	printf(_(
"\n"
" transfer a range of bytes from the given offset between files\n"
"\n"
" Example:\n"
" 'send -f 2 512 20' - writes 20 bytes at 512 bytes into the open file\n"
"\n"
" Copies data between one file descriptor and another.  Because this copying\n"
" is done within the kernel, sendfile does not need to transfer data to and\n"
" from user space.\n"
" -q -- quiet mode, do not write anything to standard output.\n"
" -f -- specifies an input file from which to source data to write\n"
" -i -- specifies an input file name from which to source data to write.\n"
" An offset and length in the source file can be optionally specified.\n"
"\n"));
}

static int
send_buffer(
	off64_t		offset,
	size_t		count,
	int		fd,
	long long	*total)
{
	off64_t		off = offset;
	ssize_t		bytes, bytes_remaining = count;
	int		ops = 0;

	*total = 0;
	while (count > 0) {
		bytes = sendfile(file->fd, fd, &off, bytes_remaining);
		if (bytes == 0)
			break;
		if (bytes < 0) {
			perror("sendfile");
			return -1;
		}
		ops++;
		*total += bytes;
		if (bytes >= bytes_remaining)
			break;
		bytes_remaining -= bytes;
	}
	return ops;
}

static int
sendfile_f(
	int		argc,
	char		**argv)
{
	off64_t		offset = 0;
	long long	count, total;
	size_t		blocksize, sectsize;
	struct timeval	t1, t2;
	char		*infile = NULL;
	int		Cflag, qflag;
	int		c, fd = -1;

	Cflag = qflag = 0;
	init_cvtnum(&blocksize, &sectsize);
	while ((c = getopt(argc, argv, "Cf:i:q")) != EOF) {
		switch (c) {
		case 'C':
			Cflag = 1;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'f':
			fd = atoi(argv[1]);
			if (fd < 0 || fd >= filecount) {
				printf(_("value %d is out of range (0-%d)\n"),
					fd, filecount-1);
				exitcode = 1;
				return 0;
			}
			break;
		case 'i':
			infile = optarg;
			break;
		default:
			exitcode = 1;
			return command_usage(&sendfile_cmd);
		}
	}
	if (infile && fd != -1) {
		exitcode = 1;
		return command_usage(&sendfile_cmd);
	}

	if (!infile)
		fd = filetable[fd].fd;
	else if ((fd = openfile(infile, NULL, IO_READONLY, 0, NULL)) < 0) {
		exitcode = 1;
		return 0;
	}

	if (optind == argc - 2) {
		offset = cvtnum(blocksize, sectsize, argv[optind]);
		if (offset < 0) {
			printf(_("non-numeric offset argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			goto done;
		}
		optind++;
		count = cvtnum(blocksize, sectsize, argv[optind]);
		if (count < 0) {
			printf(_("non-numeric length argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			goto done;
		}
	} else {
		struct stat	stat;

		if (fstat(fd, &stat) < 0) {
			perror("fstat");
			exitcode = 1;
			goto done;
		}
		count = stat.st_size;
	}

	gettimeofday(&t1, NULL);
	c = send_buffer(offset, count, fd, &total);
	if (c < 0) {
		exitcode = 1;
		goto done;
	}

	if (qflag)
		goto done;
	gettimeofday(&t2, NULL);
	t2 = tsub(t2, t1);

	report_io_times("sent", &t2, (long long)offset, count, total, c, Cflag);
done:
	if (infile)
		close(fd);
	return 0;
}

void
sendfile_init(void)
{
	sendfile_cmd.name = "sendfile";
	sendfile_cmd.altname = "send";
	sendfile_cmd.cfunc = sendfile_f;
	sendfile_cmd.argmin = 2;
	sendfile_cmd.argmax = -1;
	sendfile_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	sendfile_cmd.args =
		_("[-q] -i infile | -f N [off len]");
	sendfile_cmd.oneline =
		_("Transfer data directly between file descriptors");
	sendfile_cmd.help = sendfile_help;

	add_command(&sendfile_cmd);
}
