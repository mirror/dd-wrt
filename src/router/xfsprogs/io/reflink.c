// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 Oracle, Inc.
 * All Rights Reserved.
 */

#include <sys/uio.h>
#include <xfs/xfs.h>
#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t dedupe_cmd;
static cmdinfo_t reflink_cmd;

static void
dedupe_help(void)
{
	printf(_("\n\
 Links a range of bytes (in block size increments) from a file into a range\n\
 of bytes in the open file.  The contents of both file ranges must match.\n\
\n\
 Example:\n\
 'dedupe some_file 0 4096 32768' - links 32768 bytes from some_file at\n\
                                    offset 0 to into the open file at\n\
                                    position 4096\n\
\n\
 Reflink a range of blocks from a given input file to the open file.  Both\n\
 files share the same range of physical disk blocks; a write to the shared\n\
 range of either file should result in the write landing in a new block and\n\
 that range of the file being remapped (i.e. copy-on-write).  Both files\n\
 must reside on the same filesystem, and the contents of both ranges must\n\
 match.\n\
"));
}

static uint64_t
dedupe_ioctl(
	int		fd,
	uint64_t	soffset,
	uint64_t	doffset,
	uint64_t	len,
	int		*ops)
{
	struct xfs_extent_data		*args;
	struct xfs_extent_data_info	*info;
	int				error;
	uint64_t			deduped = 0;

	args = calloc(1, sizeof(struct xfs_extent_data) +
			 sizeof(struct xfs_extent_data_info));
	if (!args)
		goto done;
	info = (struct xfs_extent_data_info *)(args + 1);
	args->logical_offset = soffset;
	args->length = len;
	args->dest_count = 1;
	info->fd = file->fd;
	info->logical_offset = doffset;

	while (args->length > 0 || !*ops) {
		error = ioctl(fd, XFS_IOC_FILE_EXTENT_SAME, args);
		if (error) {
			perror("XFS_IOC_FILE_EXTENT_SAME");
			exitcode = 1;
			goto done;
		}
		if (info->status < 0) {
			fprintf(stderr, "XFS_IOC_FILE_EXTENT_SAME: %s\n",
					_(strerror(-info->status)));
			goto done;
		}
		if (info->status == XFS_EXTENT_DATA_DIFFERS) {
			fprintf(stderr, "XFS_IOC_FILE_EXTENT_SAME: %s\n",
					_("Extents did not match."));
			goto done;
		}
		if (args->length != 0 &&
		    (info->bytes_deduped == 0 ||
		     info->bytes_deduped > args->length))
			break;

		(*ops)++;
		args->logical_offset += info->bytes_deduped;
		info->logical_offset += info->bytes_deduped;
		if (args->length >= info->bytes_deduped)
			args->length -= info->bytes_deduped;
		deduped += info->bytes_deduped;
	}
done:
	free(args);
	return deduped;
}

static int
dedupe_f(
	int		argc,
	char		**argv)
{
	off64_t		soffset, doffset;
	long long	count, total;
	char		*infile;
	int		condensed, quiet_flag;
	size_t		fsblocksize, fssectsize;
	struct timeval	t1, t2;
	int		c, ops = 0, fd = -1;

	condensed = quiet_flag = 0;
	init_cvtnum(&fsblocksize, &fssectsize);

	while ((c = getopt(argc, argv, "Cq")) != EOF) {
		switch (c) {
		case 'C':
			condensed = 1;
			break;
		case 'q':
			quiet_flag = 1;
			break;
		default:
			exitcode = 1;
			return command_usage(&dedupe_cmd);
		}
	}
	if (optind != argc - 4) {
		exitcode = 1;
		return command_usage(&dedupe_cmd);
	}
	infile = argv[optind];
	optind++;
	soffset = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (soffset < 0) {
		printf(_("non-numeric src offset argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}
	optind++;
	doffset = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (doffset < 0) {
		printf(_("non-numeric dest offset argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}
	optind++;
	count = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (count < 0) {
		printf(_("non-positive length argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}

	fd = openfile(infile, NULL, IO_READONLY, 0, NULL);
	if (fd < 0) {
		exitcode = 1;
		return 0;
	}

	gettimeofday(&t1, NULL);
	total = dedupe_ioctl(fd, soffset, doffset, count, &ops);
	if (ops == 0 || quiet_flag)
		goto done;
	gettimeofday(&t2, NULL);
	t2 = tsub(t2, t1);

	report_io_times("deduped", &t2, (long long)doffset, count, total, ops,
			condensed);
done:
	close(fd);
	return 0;
}

static void
reflink_help(void)
{
	printf(_("\n\
 Links a range of bytes (in block size increments) from a file into a range\n\
 of bytes in the open file.  The two extent ranges need not contain identical\n\
 data.\n\
\n\
 Example:\n\
 'reflink some_file 0 4096 32768' - links 32768 bytes from some_file at\n\
                                    offset 0 to into the open file at\n\
                                    position 4096\n\
 'reflink some_file' - links all bytes from some_file into the open file\n\
                       at position 0\n\
\n\
 Reflink a range of blocks from a given input file to the open file.  Both\n\
 files share the same range of physical disk blocks; a write to the shared\n\
 range of either file should result in the write landing in a new block and\n\
 that range of the file being remapped (i.e. copy-on-write).  Both files\n\
 must reside on the same filesystem.\n\
"));
}

static uint64_t
reflink_ioctl(
	int			fd,
	uint64_t		soffset,
	uint64_t		doffset,
	uint64_t		len,
	int			*ops)
{
	struct xfs_clone_args	args;
	int			error;

	if (soffset == 0 && doffset == 0 && len == 0) {
		error = ioctl(file->fd, XFS_IOC_CLONE, fd);
		if (error)
			perror("XFS_IOC_CLONE");
	} else {
		args.src_fd = fd;
		args.src_offset = soffset;
		args.src_length = len;
		args.dest_offset = doffset;
		error = ioctl(file->fd, XFS_IOC_CLONE_RANGE, &args);
		if (error)
			perror("XFS_IOC_CLONE_RANGE");
	}
	if (!error)
		(*ops)++;
	return error ? 0 : len;
}

static int
reflink_f(
	int		argc,
	char		**argv)
{
	off64_t		soffset, doffset;
	long long	count = 0, total;
	char		*infile = NULL;
	int		condensed, quiet_flag;
	size_t		fsblocksize, fssectsize;
	struct timeval	t1, t2;
	int		c, ops = 0, fd = -1;

	condensed = quiet_flag = 0;
	doffset = soffset = 0;
	init_cvtnum(&fsblocksize, &fssectsize);

	while ((c = getopt(argc, argv, "Cq")) != EOF) {
		switch (c) {
		case 'C':
			condensed = 1;
			break;
		case 'q':
			quiet_flag = 1;
			break;
		default:
			exitcode = 1;
			return command_usage(&reflink_cmd);
		}
	}
	if (optind != argc - 4 && optind != argc - 1) {
		exitcode = 1;
		return command_usage(&reflink_cmd);
	}
	infile = argv[optind];
	optind++;
	if (optind == argc)
		goto clone_all;
	soffset = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (soffset < 0) {
		printf(_("non-numeric src offset argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}
	optind++;
	doffset = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (doffset < 0) {
		printf(_("non-numeric dest offset argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}
	optind++;
	count = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (count < 0) {
		printf(_("non-positive length argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}

clone_all:
	fd = openfile(infile, NULL, IO_READONLY, 0, NULL);
	if (fd < 0) {
		exitcode = 1;
		return 0;
	}

	gettimeofday(&t1, NULL);
	total = reflink_ioctl(fd, soffset, doffset, count, &ops);
	if (ops == 0)
		goto done;

	if (quiet_flag)
		goto done;
	gettimeofday(&t2, NULL);
	t2 = tsub(t2, t1);

	report_io_times("linked", &t2, (long long)doffset, count, total, ops,
			condensed);
done:
	close(fd);
	return 0;
}

void
reflink_init(void)
{
	reflink_cmd.name = "reflink";
	reflink_cmd.altname = "rl";
	reflink_cmd.cfunc = reflink_f;
	reflink_cmd.argmin = 1;
	reflink_cmd.argmax = -1;
	reflink_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK | CMD_FLAG_ONESHOT;
	reflink_cmd.args =
_("infile [src_off dst_off len]");
	reflink_cmd.oneline =
		_("reflinks an entire file, or a number of bytes at a specified offset");
	reflink_cmd.help = reflink_help;

	add_command(&reflink_cmd);

	dedupe_cmd.name = "dedupe";
	dedupe_cmd.altname = "dd";
	dedupe_cmd.cfunc = dedupe_f;
	dedupe_cmd.argmin = 4;
	dedupe_cmd.argmax = -1;
	dedupe_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK | CMD_FLAG_ONESHOT;
	dedupe_cmd.args =
_("infile src_off dst_off len");
	dedupe_cmd.oneline =
		_("dedupes a number of bytes at a specified offset");
	dedupe_cmd.help = dedupe_help;

	add_command(&dedupe_cmd);
}
