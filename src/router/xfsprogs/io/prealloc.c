// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#if defined(HAVE_FALLOCATE)
#include <linux/falloc.h>
#endif
#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

#ifndef FALLOC_FL_PUNCH_HOLE
#define FALLOC_FL_PUNCH_HOLE	0x02
#endif

#ifndef FALLOC_FL_COLLAPSE_RANGE
#define FALLOC_FL_COLLAPSE_RANGE 0x08
#endif

#ifndef FALLOC_FL_ZERO_RANGE
#define FALLOC_FL_ZERO_RANGE 0x10
#endif

#ifndef FALLOC_FL_INSERT_RANGE
#define FALLOC_FL_INSERT_RANGE 0x20
#endif

#ifndef FALLOC_FL_UNSHARE_RANGE
#define FALLOC_FL_UNSHARE_RANGE 0x40
#endif

static cmdinfo_t allocsp_cmd;
static cmdinfo_t freesp_cmd;
static cmdinfo_t resvsp_cmd;
static cmdinfo_t unresvsp_cmd;
static cmdinfo_t zero_cmd;
#if defined(HAVE_FALLOCATE)
static cmdinfo_t falloc_cmd;
static cmdinfo_t fpunch_cmd;
static cmdinfo_t fcollapse_cmd;
static cmdinfo_t finsert_cmd;
static cmdinfo_t fzero_cmd;
#endif

static int
offset_length(
	char		*offset,
	char		*length,
	xfs_flock64_t	*segment)
{
	size_t		blocksize, sectsize;

	init_cvtnum(&blocksize, &sectsize);
	memset(segment, 0, sizeof(*segment));
	segment->l_whence = SEEK_SET;
	segment->l_start = cvtnum(blocksize, sectsize, offset);
	if (segment->l_start < 0) {
		printf(_("non-numeric offset argument -- %s\n"), offset);
		return 0;
	}
	segment->l_len = cvtnum(blocksize, sectsize, length);
	if (segment->l_len < 0) {
		printf(_("non-numeric length argument -- %s\n"), length);
		return 0;
	}
	return 1;
}

static int
allocsp_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (xfsctl(file->name, file->fd, XFS_IOC_ALLOCSP64, &segment) < 0) {
		perror("XFS_IOC_ALLOCSP64");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
freesp_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (xfsctl(file->name, file->fd, XFS_IOC_FREESP64, &segment) < 0) {
		perror("XFS_IOC_FREESP64");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
resvsp_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (xfsctl(file->name, file->fd, XFS_IOC_RESVSP64, &segment) < 0) {
		perror("XFS_IOC_RESVSP64");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
unresvsp_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (xfsctl(file->name, file->fd, XFS_IOC_UNRESVSP64, &segment) < 0) {
		perror("XFS_IOC_UNRESVSP64");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
zero_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (xfsctl(file->name, file->fd, XFS_IOC_ZERO_RANGE, &segment) < 0) {
		perror("XFS_IOC_ZERO_RANGE");
		exitcode = 1;
		return 0;
	}
	return 0;
}


#if defined (HAVE_FALLOCATE)
static void
falloc_help(void)
{
	printf(_(
"\n"
" modifies space associated with part of a file via fallocate"
"\n"
" Example:\n"
" 'falloc 0 1m' - fills all holes within the first megabyte\n"
"\n"
" falloc uses the fallocate system call to alter space allocations in the\n"
" open file.  The following operations are supported:\n"
" All the file offsets are in units of bytes.\n"
" -c -- collapses the given range.\n"
" -i -- inserts a hole into the given range of the file.\n"
" -k -- do not change file size.\n"
" -p -- unmap the given range from the file.\n"
" -u -- unshare shared extents in the given range.\n"
"\n"));
}

static int
fallocate_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;
	int		mode = 0;
	int		c;

	while ((c = getopt(argc, argv, "cikpu")) != EOF) {
		switch (c) {
		case 'c':
			mode = FALLOC_FL_COLLAPSE_RANGE;
			break;
		case 'i':
			mode = FALLOC_FL_INSERT_RANGE;
			break;
		case 'k':
			mode = FALLOC_FL_KEEP_SIZE;
			break;
		case 'p':
			mode = FALLOC_FL_PUNCH_HOLE|FALLOC_FL_KEEP_SIZE;
			break;
		case 'u':
			mode = FALLOC_FL_UNSHARE_RANGE;
			break;
		default:
			exitcode = 1;
			command_usage(&falloc_cmd);
		}
	}
        if (optind != argc - 2) {
		exitcode = 1;
                return command_usage(&falloc_cmd);
	}

	if (!offset_length(argv[optind], argv[optind+1], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (fallocate(file->fd, mode,
			segment.l_start, segment.l_len)) {
		perror("fallocate");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
fpunch_f(
	 int		argc,
	 char		**argv)
{
	xfs_flock64_t	segment;
	int		mode = FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (fallocate(file->fd, mode,
			segment.l_start, segment.l_len)) {
		perror("fallocate");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
fcollapse_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;
	int		mode = FALLOC_FL_COLLAPSE_RANGE;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (fallocate(file->fd, mode,
			segment.l_start, segment.l_len)) {
		perror("fallocate");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
finsert_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;
	int		mode = FALLOC_FL_INSERT_RANGE;

	if (!offset_length(argv[1], argv[2], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (fallocate(file->fd, mode,
			segment.l_start, segment.l_len)) {
		perror("fallocate");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
fzero_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;
	int		mode = FALLOC_FL_ZERO_RANGE;
	int		c;

	while ((c = getopt(argc, argv, "k")) != EOF) {
		switch (c) {
		case 'k':
			mode |= FALLOC_FL_KEEP_SIZE;
			break;
		default:
			command_usage(&fzero_cmd);
		}
	}
        if (optind != argc - 2)
                return command_usage(&fzero_cmd);

	if (!offset_length(argv[optind], argv[optind + 1], &segment))
		return 0;

	if (fallocate(file->fd, mode, segment.l_start, segment.l_len)) {
		perror("fallocate");
		return 0;
	}
	return 0;
}

static int
funshare_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;
	int		mode = FALLOC_FL_UNSHARE_RANGE;
	int		index = 1;

	if (!offset_length(argv[index], argv[index + 1], &segment)) {
		exitcode = 1;
		return 0;
	}

	if (fallocate(file->fd, mode,
			segment.l_start, segment.l_len)) {
		perror("fallocate");
		exitcode = 1;
		return 0;
	}
	return 0;
}
#endif	/* HAVE_FALLOCATE */

void
prealloc_init(void)
{
	allocsp_cmd.name = "allocsp";
	allocsp_cmd.cfunc = allocsp_f;
	allocsp_cmd.argmin = 2;
	allocsp_cmd.argmax = 2;
	allocsp_cmd.flags = CMD_NOMAP_OK;
	allocsp_cmd.args = _("off len");
	allocsp_cmd.oneline = _("allocates zeroed space for part of a file");

	freesp_cmd.name = "freesp";
	freesp_cmd.cfunc = freesp_f;
	freesp_cmd.argmin = 2;
	freesp_cmd.argmax = 2;
	freesp_cmd.flags = CMD_NOMAP_OK;
	freesp_cmd.args = _("off len");
	freesp_cmd.oneline = _("frees space associated with part of a file");

	resvsp_cmd.name = "resvsp";
	resvsp_cmd.cfunc = resvsp_f;
	resvsp_cmd.argmin = 2;
	resvsp_cmd.argmax = 2;
	resvsp_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	resvsp_cmd.args = _("off len");
	resvsp_cmd.oneline =
		_("reserves space associated with part of a file");

	unresvsp_cmd.name = "unresvsp";
	unresvsp_cmd.cfunc = unresvsp_f;
	unresvsp_cmd.argmin = 2;
	unresvsp_cmd.argmax = 2;
	unresvsp_cmd.args = _("off len");
	unresvsp_cmd.flags = CMD_NOMAP_OK;
	unresvsp_cmd.oneline =
		_("frees reserved space associated with part of a file");

	zero_cmd.name = "zero";
	zero_cmd.cfunc = zero_f;
	zero_cmd.argmin = 2;
	zero_cmd.argmax = 2;
	zero_cmd.flags = CMD_NOMAP_OK;
	zero_cmd.args = _("off len");
	zero_cmd.oneline =
		_("Converts the given range of a file to allocated zeros");

	add_command(&allocsp_cmd);
	add_command(&freesp_cmd);
	add_command(&resvsp_cmd);
	add_command(&unresvsp_cmd);
	add_command(&zero_cmd);

#if defined (HAVE_FALLOCATE)
	falloc_cmd.name = "falloc";
	falloc_cmd.cfunc = fallocate_f;
	falloc_cmd.argmin = 2;
	falloc_cmd.argmax = -1;
	falloc_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	falloc_cmd.args = _("[-c] [-k] [-p] [-u] off len");
	falloc_cmd.oneline =
	_("allocates space associated with part of a file via fallocate");
	falloc_cmd.help = falloc_help;
	add_command(&falloc_cmd);

	fpunch_cmd.name = "fpunch";
	fpunch_cmd.cfunc = fpunch_f;
	fpunch_cmd.argmin = 2;
	fpunch_cmd.argmax = 2;
	fpunch_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fpunch_cmd.args = _("off len");
	fpunch_cmd.oneline =
	_("de-allocates space associated with part of a file via fallocate");
	add_command(&fpunch_cmd);

	fcollapse_cmd.name = "fcollapse";
	fcollapse_cmd.cfunc = fcollapse_f;
	fcollapse_cmd.argmin = 2;
	fcollapse_cmd.argmax = 2;
	fcollapse_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fcollapse_cmd.args = _("off len");
	fcollapse_cmd.oneline =
	_("de-allocates space and eliminates the hole by shifting extents");
	add_command(&fcollapse_cmd);

	finsert_cmd.name = "finsert";
	finsert_cmd.cfunc = finsert_f;
	finsert_cmd.argmin = 2;
	finsert_cmd.argmax = 2;
	finsert_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	finsert_cmd.args = _("off len");
	finsert_cmd.oneline =
	_("creates new space for writing within file by shifting extents");
	add_command(&finsert_cmd);

	fzero_cmd.name = "fzero";
	fzero_cmd.cfunc = fzero_f;
	fzero_cmd.argmin = 2;
	fzero_cmd.argmax = 3;
	fzero_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fzero_cmd.args = _("[-k] off len");
	fzero_cmd.oneline =
	_("zeroes space and eliminates holes by preallocating");
	add_command(&fzero_cmd);

	fzero_cmd.name = "funshare";
	fzero_cmd.cfunc = funshare_f;
	fzero_cmd.argmin = 2;
	fzero_cmd.argmax = 2;
	fzero_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fzero_cmd.args = _("off len");
	fzero_cmd.oneline =
	_("unshares shared blocks within the range");
	add_command(&fzero_cmd);
#endif	/* HAVE_FALLOCATE */
}
