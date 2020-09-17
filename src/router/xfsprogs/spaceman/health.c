// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Oracle.
 * All Rights Reserved.
 */
#include "platform_defs.h"
#include "libxfs.h"
#include "command.h"
#include "init.h"
#include "input.h"
#include "libfrog/logging.h"
#include "libfrog/paths.h"
#include "libfrog/fsgeom.h"
#include "libfrog/bulkstat.h"
#include "space.h"

static cmdinfo_t health_cmd;
static unsigned long long reported;
static bool comprehensive;
static bool quiet;

static bool has_realtime(const struct xfs_fsop_geom *g)
{
	return g->rtblocks > 0;
}

static bool has_finobt(const struct xfs_fsop_geom *g)
{
	return g->flags & XFS_FSOP_GEOM_FLAGS_FINOBT;
}

static bool has_rmapbt(const struct xfs_fsop_geom *g)
{
	return g->flags & XFS_FSOP_GEOM_FLAGS_RMAPBT;
}

static bool has_reflink(const struct xfs_fsop_geom *g)
{
	return g->flags & XFS_FSOP_GEOM_FLAGS_REFLINK;
}

struct flag_map {
	unsigned int		mask;
	bool			(*has_fn)(const struct xfs_fsop_geom *g);
	const char		*descr;
};

static const struct flag_map fs_flags[] = {
	{
		.mask = XFS_FSOP_GEOM_SICK_COUNTERS,
		.descr = "summary counters",
	},
	{
		.mask = XFS_FSOP_GEOM_SICK_UQUOTA,
		.descr = "user quota",
	},
	{
		.mask = XFS_FSOP_GEOM_SICK_GQUOTA,
		.descr = "group quota",
	},
	{
		.mask = XFS_FSOP_GEOM_SICK_PQUOTA,
		.descr = "project quota",
	},
	{
		.mask = XFS_FSOP_GEOM_SICK_RT_BITMAP,
		.descr = "realtime bitmap",
		.has_fn = has_realtime,
	},
	{
		.mask = XFS_FSOP_GEOM_SICK_RT_SUMMARY,
		.descr = "realtime summary",
		.has_fn = has_realtime,
	},
	{0},
};

static const struct flag_map ag_flags[] = {
	{
		.mask = XFS_AG_GEOM_SICK_SB,
		.descr = "superblock",
	},
	{
		.mask = XFS_AG_GEOM_SICK_AGF,
		.descr = "AGF header",
	},
	{
		.mask = XFS_AG_GEOM_SICK_AGFL,
		.descr = "AGFL header",
	},
	{
		.mask = XFS_AG_GEOM_SICK_AGI,
		.descr = "AGI header",
	},
	{
		.mask = XFS_AG_GEOM_SICK_BNOBT,
		.descr = "free space by block btree",
	},
	{
		.mask = XFS_AG_GEOM_SICK_CNTBT,
		.descr = "free space by length btree",
	},
	{
		.mask = XFS_AG_GEOM_SICK_INOBT,
		.descr = "inode btree",
	},
	{
		.mask = XFS_AG_GEOM_SICK_FINOBT,
		.descr = "free inode btree",
		.has_fn = has_finobt,
	},
	{
		.mask = XFS_AG_GEOM_SICK_RMAPBT,
		.descr = "reverse mappings btree",
		.has_fn = has_rmapbt,
	},
	{
		.mask = XFS_AG_GEOM_SICK_REFCNTBT,
		.descr = "reference count btree",
		.has_fn = has_reflink,
	},
	{0},
};

static const struct flag_map inode_flags[] = {
	{
		.mask = XFS_BS_SICK_INODE,
		.descr = "inode core",
	},
	{
		.mask = XFS_BS_SICK_BMBTD,
		.descr = "data fork",
	},
	{
		.mask = XFS_BS_SICK_BMBTA,
		.descr = "extended attribute fork",
	},
	{
		.mask = XFS_BS_SICK_BMBTC,
		.descr = "copy on write fork",
	},
	{
		.mask = XFS_BS_SICK_DIR,
		.descr = "directory",
	},
	{
		.mask = XFS_BS_SICK_XATTR,
		.descr = "extended attributes",
	},
	{
		.mask = XFS_BS_SICK_SYMLINK,
		.descr = "symbolic link target",
	},
	{
		.mask = XFS_BS_SICK_PARENT,
		.descr = "parent pointers",
	},
	{0},
};

/* Convert a flag mask to a report. */
static void
report_sick(
	const char			*descr,
	const struct flag_map		*maps,
	unsigned int			sick,
	unsigned int			checked)
{
	const struct flag_map		*f;
	bool				bad;

	for (f = maps; f->mask != 0; f++) {
		if (f->has_fn && !f->has_fn(&file->xfd.fsgeom))
			continue;
		bad = sick & f->mask;
		if (!bad && !(checked & f->mask))
			continue;
		reported++;
		if (!bad && quiet)
			continue;
		printf("%s %s: %s\n", descr, _(f->descr),
				bad ? _("unhealthy") : _("ok"));
	}
}

/* Report on an AG's health. */
static int
report_ag_sick(
	xfs_agnumber_t		agno)
{
	struct xfs_ag_geometry	ageo = { 0 };
	char			descr[256];
	int			ret;

	ret = -xfrog_ag_geometry(file->xfd.fd, agno, &ageo);
	if (ret) {
		xfrog_perror(ret, "ag_geometry");
		return 1;
	}
	snprintf(descr, sizeof(descr) - 1, _("AG %u"), agno);
	report_sick(descr, ag_flags, ageo.ag_sick, ageo.ag_checked);
	return 0;
}

/* Report on an inode's health. */
static int
report_inode_health(
	unsigned long long	ino,
	const char		*descr)
{
	struct xfs_bulkstat	bs;
	char			d[256];
	int			ret;

	if (!descr) {
		snprintf(d, sizeof(d) - 1, _("inode %llu"), ino);
		descr = d;
	}

	ret = -xfrog_bulkstat_single(&file->xfd, ino, 0, &bs);
	if (ret) {
		xfrog_perror(ret, descr);
		return 1;
	}

	report_sick(descr, inode_flags, bs.bs_sick, bs.bs_checked);
	return 0;
}

/* Report on a file's health. */
static int
report_file_health(
	const char	*path)
{
	struct stat	stata, statb;
	int		ret;

	ret = lstat(path, &statb);
	if (ret) {
		perror(path);
		return 1;
	}

	ret = fstat(file->xfd.fd, &stata);
	if (ret) {
		perror(file->name);
		return 1;
	}

	if (stata.st_dev != statb.st_dev) {
		fprintf(stderr, _("%s: not on the open filesystem"), path);
		return 1;
	}

	return report_inode_health(statb.st_ino, path);
}

#define BULKSTAT_NR		(128)

/*
 * Report on all files' health for a given @agno.  If @agno is NULLAGNUMBER,
 * report on all files in the filesystem.
 */
static int
report_bulkstat_health(
	xfs_agnumber_t		agno)
{
	struct xfs_bulkstat_req	*breq;
	char			descr[256];
	uint32_t		i;
	int			error;

	error = -xfrog_bulkstat_alloc_req(BULKSTAT_NR, 0, &breq);
	if (error) {
		xfrog_perror(error, "bulk alloc req");
		exitcode = 1;
		return 1;
	}

	if (agno != NULLAGNUMBER)
		xfrog_bulkstat_set_ag(breq, agno);

	do {
		error = -xfrog_bulkstat(&file->xfd, breq);
		if (error)
			break;
		for (i = 0; i < breq->hdr.ocount; i++) {
			snprintf(descr, sizeof(descr) - 1, _("inode %"PRIu64),
					breq->bulkstat[i].bs_ino);
			report_sick(descr, inode_flags,
					breq->bulkstat[i].bs_sick,
					breq->bulkstat[i].bs_checked);
		}
	} while (breq->hdr.ocount > 0);

	if (error)
		xfrog_perror(error, "bulkstat");

	free(breq);
	return error;
}

#define OPT_STRING ("a:cfi:q")

/* Report on health problems in XFS filesystem. */
static int
health_f(
	int			argc,
	char			**argv)
{
	unsigned long long	x;
	xfs_agnumber_t		agno;
	bool			default_report = true;
	int			c;
	int			ret;

	reported = 0;

	if (file->xfd.fsgeom.version != XFS_FSOP_GEOM_VERSION_V5) {
		perror("health");
		return 1;
	}

	/* Set our reporting options appropriately in the first pass. */
	while ((c = getopt(argc, argv, OPT_STRING)) != EOF) {
		switch (c) {
		case 'a':
			default_report = false;
			errno = 0;
			x = strtoll(optarg, NULL, 10);
			if (!errno && x >= NULLAGNUMBER)
				errno = ERANGE;
			if (errno) {
				perror("ag health");
				return 1;
			}
			break;
		case 'c':
			comprehensive = true;
			break;
		case 'f':
			default_report = false;
			break;
		case 'i':
			default_report = false;
			errno = 0;
			x = strtoll(optarg, NULL, 10);
			if (errno) {
				perror("inode health");
				return 1;
			}
			break;
		case 'q':
			quiet = true;
			break;
		default:
			return command_usage(&health_cmd);
		}
	}
	if (optind < argc)
		default_report = false;

	/* Reparse arguments, this time for reporting actions. */
	optind = 1;
	while ((c = getopt(argc, argv, OPT_STRING)) != EOF) {
		switch (c) {
		case 'a':
			agno = strtoll(optarg, NULL, 10);
			ret = report_ag_sick(agno);
			if (!ret && comprehensive)
				ret = report_bulkstat_health(agno);
			if (ret)
				return 1;
			break;
		case 'f':
			report_sick(_("filesystem"), fs_flags,
					file->xfd.fsgeom.sick,
					file->xfd.fsgeom.checked);
			if (comprehensive) {
				ret = report_bulkstat_health(NULLAGNUMBER);
				if (ret)
					return 1;
			}
			break;
		case 'i':
			x = strtoll(optarg, NULL, 10);
			ret = report_inode_health(x, NULL);
			if (ret)
				return 1;
			break;
		default:
			break;
		}
	}

	for (c = optind; c < argc; c++) {
		ret = report_file_health(argv[c]);
		if (ret)
			return 1;
	}

	/* No arguments gets us a summary of fs state. */
	if (default_report) {
		report_sick(_("filesystem"), fs_flags, file->xfd.fsgeom.sick,
				file->xfd.fsgeom.checked);

		for (agno = 0; agno < file->xfd.fsgeom.agcount; agno++) {
			ret = report_ag_sick(agno);
			if (ret)
				return 1;
		}
		if (comprehensive) {
			ret = report_bulkstat_health(NULLAGNUMBER);
			if (ret)
				return 1;
		}
	}

	if (!reported) {
		fprintf(stderr,
_("Health status has not been collected for this filesystem.\n"));
		fprintf(stderr,
_("Please run xfs_scrub(8) to remedy this situation.\n"));
	}

	return 0;
}

static void
health_help(void)
{
	printf(_(
"\n"
"Report all observed filesystem health problems.\n"
"\n"
" -a agno  -- Report health of the given allocation group.\n"
" -c       -- Report on the health of all inodes.\n"
" -f       -- Report health of the overall filesystem.\n"
" -i inum  -- Report health of a given inode number.\n"
" -q       -- Only report unhealthy metadata.\n"
" paths    -- Report health of the given file path.\n"
"\n"));

}

static cmdinfo_t health_cmd = {
	.name = "health",
	.cfunc = health_f,
	.argmin = 0,
	.argmax = -1,
	.args = "[-a agno] [-c] [-f] [-i inum] [-q] [paths]",
	.flags = CMD_FLAG_ONESHOT,
	.help = health_help,
};

void
health_init(void)
{
	health_cmd.oneline = _("Report observed XFS health problems."),
	add_command(&health_cmd);
}
