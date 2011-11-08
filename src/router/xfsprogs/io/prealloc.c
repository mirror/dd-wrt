/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
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

#if defined(HAVE_FALLOCATE)
#include <linux/falloc.h>
#endif
#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include "init.h"
#include "io.h"

#ifndef FALLOC_FL_PUNCH_HOLE
#define FALLOC_FL_PUNCH_HOLE	0x02
#endif

static cmdinfo_t allocsp_cmd;
static cmdinfo_t freesp_cmd;
static cmdinfo_t resvsp_cmd;
static cmdinfo_t unresvsp_cmd;
static cmdinfo_t zero_cmd;
#if defined(HAVE_FALLOCATE)
static cmdinfo_t falloc_cmd;
static cmdinfo_t fpunch_cmd;
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

	if (!offset_length(argv[1], argv[2], &segment))
		return 0;

	if (xfsctl(file->name, file->fd, XFS_IOC_ALLOCSP64, &segment) < 0) {
		perror("XFS_IOC_ALLOCSP64");
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

	if (!offset_length(argv[1], argv[2], &segment))
		return 0;

	if (xfsctl(file->name, file->fd, XFS_IOC_FREESP64, &segment) < 0) {
		perror("XFS_IOC_FREESP64");
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

	if (!offset_length(argv[1], argv[2], &segment))
		return 0;

	if (xfsctl(file->name, file->fd, XFS_IOC_RESVSP64, &segment) < 0) {
		perror("XFS_IOC_RESVSP64");
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

	if (!offset_length(argv[1], argv[2], &segment))
		return 0;

	if (xfsctl(file->name, file->fd, XFS_IOC_UNRESVSP64, &segment) < 0) {
		perror("XFS_IOC_UNRESVSP64");
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

	if (!offset_length(argv[1], argv[2], &segment))
		return 0;

	if (xfsctl(file->name, file->fd, XFS_IOC_ZERO_RANGE, &segment) < 0) {
		perror("XFS_IOC_ZERO_RANGE");
		return 0;
	}
	return 0;
}


#if defined (HAVE_FALLOCATE)
static int
fallocate_f(
	int		argc,
	char		**argv)
{
	xfs_flock64_t	segment;
	int		mode = 0;
	int		c;

	while ((c = getopt(argc, argv, "kp")) != EOF) {
		switch (c) {
		case 'k':
			mode = FALLOC_FL_KEEP_SIZE;
			break;
		case 'p':
			mode = FALLOC_FL_PUNCH_HOLE;
			break;
		default:
			command_usage(&falloc_cmd);
		}
	}
        if (optind != argc - 2)
                return command_usage(&falloc_cmd);

	if (!offset_length(argv[optind], argv[optind+1], &segment))
		return 0;

	if (fallocate(file->fd, mode,
			segment.l_start, segment.l_len)) {
		perror("fallocate");
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

	if (!offset_length(argv[1], argv[2], &segment))
		return 0;

	if (fallocate(file->fd, mode,
			segment.l_start, segment.l_len)) {
		perror("fallocate");
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
	falloc_cmd.args = _("[-k] [-p] off len");
	falloc_cmd.oneline =
		_("allocates space associated with part of a file via fallocate");
	add_command(&falloc_cmd);

	fpunch_cmd.name = "fpunch";
	fpunch_cmd.cfunc = fpunch_f;
	fpunch_cmd.argmin = 2;
	fpunch_cmd.argmax = 2;
	fpunch_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fpunch_cmd.args = _("off len");
	fpunch_cmd.oneline =
		_("de-allocates space assocated with part of a file via fallocate");
	add_command(&fpunch_cmd);
#endif	/* HAVE_FALLOCATE */
}
