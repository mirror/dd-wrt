// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */

#include <sys/uio.h>
#include <xfs/xfs.h>
#include "command.h"
#include "input.h"
#include "init.h"
#include "libfrog/paths.h"
#include "libfrog/fsgeom.h"
#include "libfrog/scrub.h"
#include "io.h"

static struct cmdinfo scrub_cmd;
static struct cmdinfo repair_cmd;

static void
scrub_help(void)
{
	const struct xfrog_scrub_descr	*d;
	int				i;

	printf(_(
"\n"
" Scrubs a piece of XFS filesystem metadata.  The first argument is the type\n"
" of metadata to examine.  Allocation group metadata types take one AG number\n"
" as the second parameter.  Inode metadata types act on the currently open file\n"
" or (optionally) take an inode number and generation number to act upon as\n"
" the second and third parameters.\n"
"\n"
" Example:\n"
" 'scrub inobt 3' - scrub the inode btree in AG 3.\n"
" 'scrub bmapbtd 128 13525' - scrubs the extent map of inode 128 gen 13525.\n"
"\n"
" Known metadata scrub types are:"));
	for (i = 0, d = xfrog_scrubbers; i < XFS_SCRUB_TYPE_NR; i++, d++)
		printf(" %s", d->name);
	printf("\n");
}

static void
scrub_ioctl(
	int				fd,
	int				type,
	uint64_t			control,
	uint32_t			control2)
{
	struct xfs_scrub_metadata	meta;
	const struct xfrog_scrub_descr	*sc;
	int				error;

	sc = &xfrog_scrubbers[type];
	memset(&meta, 0, sizeof(meta));
	meta.sm_type = type;
	switch (sc->type) {
	case XFROG_SCRUB_TYPE_AGHEADER:
	case XFROG_SCRUB_TYPE_PERAG:
		meta.sm_agno = control;
		break;
	case XFROG_SCRUB_TYPE_INODE:
		meta.sm_ino = control;
		meta.sm_gen = control2;
		break;
	case XFROG_SCRUB_TYPE_NONE:
	case XFROG_SCRUB_TYPE_FS:
		/* no control parameters */
		break;
	}
	meta.sm_flags = 0;

	error = ioctl(fd, XFS_IOC_SCRUB_METADATA, &meta);
	if (error)
		perror("scrub");
	if (meta.sm_flags & XFS_SCRUB_OFLAG_CORRUPT)
		printf(_("Corruption detected.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_PREEN)
		printf(_("Optimization possible.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_XFAIL)
		printf(_("Cross-referencing failed.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_XCORRUPT)
		printf(_("Corruption detected during cross-referencing.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_INCOMPLETE)
		printf(_("Scan was not complete.\n"));
}

static int
parse_args(
	int				argc,
	char				**argv,
	struct cmdinfo			*cmdinfo,
	void				(*fn)(int, int, uint64_t, uint32_t))
{
	char				*p;
	int				type = -1;
	int				i, c;
	uint64_t			control = 0;
	uint32_t			control2 = 0;
	const struct xfrog_scrub_descr	*d = NULL;

	while ((c = getopt(argc, argv, "")) != EOF) {
		switch (c) {
		default:
			return command_usage(cmdinfo);
		}
	}
	if (optind > argc - 1)
		return command_usage(cmdinfo);

	for (i = 0, d = xfrog_scrubbers; i < XFS_SCRUB_TYPE_NR; i++, d++) {
		if (strcmp(d->name, argv[optind]) == 0) {
			type = i;
			break;
		}
	}
	if (type < 0) {
		printf(_("Unknown type '%s'.\n"), argv[optind]);
		return command_usage(cmdinfo);
	}
	optind++;

	switch (d->type) {
	case XFROG_SCRUB_TYPE_INODE:
		if (optind == argc) {
			control = 0;
			control2 = 0;
		} else if (optind == argc - 2) {
			control = strtoull(argv[optind], &p, 0);
			if (*p != '\0') {
				fprintf(stderr,
					_("Bad inode number '%s'.\n"),
					argv[optind]);
				return 0;
			}
			control2 = strtoul(argv[optind + 1], &p, 0);
			if (*p != '\0') {
				fprintf(stderr,
					_("Bad generation number '%s'.\n"),
					argv[optind + 1]);
				return 0;
			}
		} else {
			fprintf(stderr,
				_("Must specify inode number and generation.\n"));
			return 0;
		}
		break;
	case XFROG_SCRUB_TYPE_AGHEADER:
	case XFROG_SCRUB_TYPE_PERAG:
		if (optind != argc - 1) {
			fprintf(stderr,
				_("Must specify one AG number.\n"));
			return 0;
		}
		control = strtoul(argv[optind], &p, 0);
		if (*p != '\0') {
			fprintf(stderr,
				_("Bad AG number '%s'.\n"), argv[optind]);
			return 0;
		}
		break;
	case XFROG_SCRUB_TYPE_FS:
	case XFROG_SCRUB_TYPE_NONE:
		if (optind != argc) {
			fprintf(stderr,
				_("No parameters allowed.\n"));
			return 0;
		}
		break;
	default:
		ASSERT(0);
		break;
	}
	fn(file->fd, type, control, control2);

	return 0;
}

static int
scrub_f(
	int				argc,
	char				**argv)
{
	return parse_args(argc, argv, &scrub_cmd, scrub_ioctl);
}

void
scrub_init(void)
{
	scrub_cmd.name = "scrub";
	scrub_cmd.altname = "sc";
	scrub_cmd.cfunc = scrub_f;
	scrub_cmd.argmin = 1;
	scrub_cmd.argmax = -1;
	scrub_cmd.flags = CMD_NOMAP_OK;
	scrub_cmd.args = _("type [agno|ino gen]");
	scrub_cmd.oneline = _("scrubs filesystem metadata");
	scrub_cmd.help = scrub_help;

	add_command(&scrub_cmd);
}

static void
repair_help(void)
{
	const struct xfrog_scrub_descr	*d;
	int				i;

	printf(_(
"\n"
" Repairs a piece of XFS filesystem metadata.  The first argument is the type\n"
" of metadata to examine.  Allocation group metadata types take one AG number\n"
" as the second parameter.  Inode metadata types act on the currently open file\n"
" or (optionally) take an inode number and generation number to act upon as\n"
" the second and third parameters.\n"
"\n"
" Example:\n"
" 'repair inobt 3' - repairs the inode btree in AG 3.\n"
" 'repair bmapbtd 128 13525' - repairs the extent map of inode 128 gen 13525.\n"
"\n"
" Known metadata repairs types are:"));
	for (i = 0, d = xfrog_scrubbers; i < XFS_SCRUB_TYPE_NR; i++, d++)
		printf(" %s", d->name);
	printf("\n");
}

static void
repair_ioctl(
	int				fd,
	int				type,
	uint64_t			control,
	uint32_t			control2)
{
	struct xfs_scrub_metadata	meta;
	const struct xfrog_scrub_descr	*sc;
	int				error;

	sc = &xfrog_scrubbers[type];
	memset(&meta, 0, sizeof(meta));
	meta.sm_type = type;
	switch (sc->type) {
	case XFROG_SCRUB_TYPE_AGHEADER:
	case XFROG_SCRUB_TYPE_PERAG:
		meta.sm_agno = control;
		break;
	case XFROG_SCRUB_TYPE_INODE:
		meta.sm_ino = control;
		meta.sm_gen = control2;
		break;
	case XFROG_SCRUB_TYPE_NONE:
	case XFROG_SCRUB_TYPE_FS:
		/* no control parameters */
		break;
	}
	meta.sm_flags = XFS_SCRUB_IFLAG_REPAIR;

	error = ioctl(fd, XFS_IOC_SCRUB_METADATA, &meta);
	if (error)
		perror("repair");
	if (meta.sm_flags & XFS_SCRUB_OFLAG_CORRUPT)
		printf(_("Corruption remains.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_PREEN)
		printf(_("Optimization possible.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_XFAIL)
		printf(_("Cross-referencing failed.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_XCORRUPT)
		printf(_("Corruption still detected during cross-referencing.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_INCOMPLETE)
		printf(_("Repair was not complete.\n"));
	if (meta.sm_flags & XFS_SCRUB_OFLAG_NO_REPAIR_NEEDED)
		printf(_("Metadata did not need repair or optimization.\n"));
}

static int
repair_f(
	int				argc,
	char				**argv)
{
	return parse_args(argc, argv, &repair_cmd, repair_ioctl);
}

void
repair_init(void)
{
	if (!expert)
		return;
	repair_cmd.name = "repair";
	repair_cmd.altname = "fix";
	repair_cmd.cfunc = repair_f;
	repair_cmd.argmin = 1;
	repair_cmd.argmax = -1;
	repair_cmd.flags = CMD_NOMAP_OK;
	repair_cmd.args = _("type [agno|ino gen]");
	repair_cmd.oneline = _("repairs filesystem metadata");
	repair_cmd.help = repair_help;

	add_command(&repair_cmd);
}
