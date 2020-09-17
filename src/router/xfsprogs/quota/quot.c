// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include <stdbool.h>
#include "command.h"
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include "init.h"
#include "quota.h"
#include "libfrog/logging.h"
#include "libfrog/fsgeom.h"
#include "libfrog/bulkstat.h"

typedef struct du {
	struct du	*next;
	uint64_t	blocks;
	uint64_t	blocks30;
	uint64_t	blocks60;
	uint64_t	blocks90;
	uint64_t	nfiles;
	uint32_t	id;
} du_t;

#define	TSIZE		500
static uint64_t	sizes[TSIZE];
static uint64_t	overflow;

#define	NDU		60000
#define	DUHASH		8209
static du_t		du[3][NDU];
static du_t		*duhash[3][DUHASH];
static int		ndu[3];	/* #usr/grp/prj */

#define NBSTAT 		4069

static time_t now;
static cmdinfo_t quot_cmd;

static void
quot_help(void)
{
	printf(_(
"\n"
" display a summary of filesystem ownership\n"
"\n"
" -a -- summarise for all local XFS filesystem mount points\n"
" -c -- display three columns giving file size in kilobytes, number of files\n"
"       of that size, and cumulative total of kilobytes in that size or\n"
"       smaller file.  The last row is used as an overflow bucket and is the\n"
"       total of all files greater than 500 kilobytes.\n"
" -v -- display three columns containing the number of kilobytes not\n"
"       accessed in the last 30, 60, and 90 days.\n"
" -g -- display group summary\n"
" -p -- display project summary\n"
" -u -- display user summary\n"
" -b -- display number of blocks used\n"
" -i -- display number of inodes used\n"
" -r -- display number of realtime blocks used\n"
" -n -- skip identifier-to-name translations, just report IDs\n"
" -N -- suppress the initial header\n"
" -f -- send output to a file\n"
" The (optional) user/group/project can be specified either by name or by\n"
" number (i.e. uid/gid/projid).\n"
"\n"));
}

static void
quot_bulkstat_add(
	struct xfs_bulkstat	*p,
	uint		flags)
{
	du_t		*dp;
	du_t		**hp;
	uint64_t	size;
	uint32_t	i, id;

	if ((p->bs_mode & S_IFMT) == 0)
		return;
	size = howmany((p->bs_blocks * p->bs_blksize), 0x400ULL);

	if (flags & HISTOGRAM_FLAG) {
		if (!(S_ISDIR(p->bs_mode) || S_ISREG(p->bs_mode)))
			return;
		if (size >= TSIZE) {
			overflow += size;
			size = TSIZE - 1;
		}
		sizes[(int)size]++;
		return;
	}
	for (i = 0; i < 3; i++) {
		id = (i == 0) ? p->bs_uid : ((i == 1) ?
			p->bs_gid : p->bs_projectid);
		hp = &duhash[i][id % DUHASH];
		for (dp = *hp; dp; dp = dp->next)
			if (dp->id == id)
				break;
		if (dp == NULL) {
			if (ndu[i] >= NDU)
				return;
			dp = &du[i][(ndu[i]++)];
			dp->next = *hp;
			*hp = dp;
			dp->id = id;
			dp->nfiles = 0;
			dp->blocks = 0;
			dp->blocks30 = 0;
			dp->blocks60 = 0;
			dp->blocks90 = 0;
		}
		dp->blocks += size;

		if (now - p->bs_atime > 30 * (60*60*24))
			dp->blocks30 += size;
		if (now - p->bs_atime > 60 * (60*60*24))
			dp->blocks60 += size;
		if (now - p->bs_atime > 90 * (60*60*24))
			dp->blocks90 += size;
		dp->nfiles++;
	}
}

static void
quot_bulkstat_mount(
	char			*fsdir,
	unsigned int		flags)
{
	struct xfs_fd		fsxfd = XFS_FD_INIT_EMPTY;
	struct xfs_bulkstat_req	*breq;
	int			i, sts, ret;
	du_t			**dp;

	/*
	 * Initialize tables between checks; because of the qsort
	 * in report() the hash tables must be rebuilt each time.
	 */
	for (sts = 0; sts < TSIZE; sts++)
		sizes[sts] = 0;
	overflow = 0;
	for (i = 0; i < 3; i++)
		for (dp = duhash[i]; dp < &duhash[i][DUHASH]; dp++)
			*dp = NULL;
	ndu[0] = ndu[1] = ndu[2] = 0;

	ret = -xfd_open(&fsxfd, fsdir, O_RDONLY);
	if (ret) {
		xfrog_perror(ret, fsdir);
		return;
	}

	ret = -xfrog_bulkstat_alloc_req(NBSTAT, 0, &breq);
	if (ret) {
		xfrog_perror(ret, "calloc");
		xfd_close(&fsxfd);
		return;
	}

	while ((sts = -xfrog_bulkstat(&fsxfd, breq)) == 0) {
		if (breq->hdr.ocount == 0)
			break;
		for (i = 0; i < breq->hdr.ocount; i++)
			quot_bulkstat_add(&breq->bulkstat[i], flags);
	}
	if (sts < 0)
		xfrog_perror(sts, "XFS_IOC_FSBULKSTAT");
	free(breq);
	xfd_close(&fsxfd);
}

static int
qcompare(
	du_t		*p1,
	du_t		*p2)
{
	if (p1->blocks > p2->blocks)
		return -1;
	if (p1->blocks < p2->blocks)
		return 1;
	if (p1->id > p2->id)
		return 1;
	else if (p1->id < p2->id)
		return -1;
	return 0;
}

typedef char *(*idtoname_t)(uint32_t);

static void
quot_report_mount_any_type(
	FILE		*fp,
	du_t		*dp,
	int		count,
	idtoname_t	names,
	uint		form,
	uint		type,
	fs_path_t	*mount,
	uint		flags)
{
	char		*cp;

	fprintf(fp, _("%s (%s) %s:\n"),
		mount->fs_name, mount->fs_dir, type_to_string(type));
	qsort(dp, count, sizeof(dp[0]),
		(int (*)(const void *, const void *))qcompare);
	for (; dp < &dp[count]; dp++) {
		if (dp->blocks == 0)
			return;
		fprintf(fp, "%8llu    ", (unsigned long long) dp->blocks);
		if (form & XFS_INODE_QUOTA)
			fprintf(fp, "%8llu    ",
				(unsigned long long) dp->nfiles);
		if (!(flags & NO_LOOKUP_FLAG) &&
		    ((cp = (names)(dp->id)) != NULL))
			fprintf(fp, "%-8.8s", cp);
		else
			fprintf(fp, "#%-7d", dp->id);
		if (flags & VERBOSE_FLAG)
			fprintf(fp, "    %8llu    %8llu    %8llu",
			       (unsigned long long) dp->blocks30,
			       (unsigned long long) dp->blocks60,
			       (unsigned long long) dp->blocks90);
		fputc('\n', fp);
	}
}

static void
quot_report_mount(
	FILE		*fp,
	uint		form,
	uint		type,
	fs_path_t	*mount,
	uint		flags)
{
	switch (type) {
	case XFS_GROUP_QUOTA:
		quot_report_mount_any_type(fp, du[1], ndu[1], gid_to_name,
						form, type, mount, flags);
		break;
	case XFS_PROJ_QUOTA:
		quot_report_mount_any_type(fp, du[2], ndu[2], prid_to_name,
						form, type, mount, flags);
		break;
	case XFS_USER_QUOTA:
		quot_report_mount_any_type(fp, du[0], ndu[0], uid_to_name,
						form, type, mount, flags);
	}
}

static void
quot_report(
	FILE		*fp,
	uint		form,
	uint		type,
	char		*dir,
	uint		flags)
{
	fs_cursor_t	cursor;
	fs_path_t	*mount;

	now = time(NULL);
	fs_cursor_initialise(dir, FS_MOUNT_POINT, &cursor);
	while ((mount = fs_cursor_next_entry(&cursor))) {
		quot_bulkstat_mount(mount->fs_dir, flags);
		quot_report_mount(fp, form, type, mount, flags);
	}
}

static void
quot_histogram_mount(
	FILE		*fp,
	fs_path_t	*mount,
	uint		flags)
{
	uint64_t	t = 0;
	int		i;

	fprintf(fp, _("%s (%s):\n"), mount->fs_name, mount->fs_dir);

	for (i = 0; i < TSIZE - 1; i++)
		if (sizes[i] > 0) {
			t += sizes[i] * i;
			fprintf(fp, _("%d\t%llu\t%llu\n"), i,
			       (unsigned long long) sizes[i],
			       (unsigned long long) t);
		}
	fprintf(fp, _("%d\t%llu\t%llu\n"), TSIZE - 1,
		(unsigned long long) sizes[TSIZE - 1],
		(unsigned long long) (overflow + t));
}

static void
quot_histogram(
	FILE		*fp,
	char		*dir,
	uint		flags)
{
	fs_cursor_t	cursor;
	fs_path_t	*mount;

	fs_cursor_initialise(dir, FS_MOUNT_POINT, &cursor);
	while ((mount = fs_cursor_next_entry(&cursor))) {
		quot_bulkstat_mount(mount->fs_dir, flags);
		quot_histogram_mount(fp, mount, flags);
	}
}

static void
quot_any_type(
	FILE		*fp,
	uint		form,
	uint		type,
	char		*dir,
	uint		flags)
{
	if (flags & HISTOGRAM_FLAG)
		quot_histogram(fp, dir, flags);
	else
		quot_report(fp, form, type, dir, flags);
}

static int
quot_f(
	int		argc,
	char		**argv)
{
	FILE		*fp = NULL;
	char		*fname = NULL;
	int		c, flags = 0, type = 0, form = 0;

	while ((c = getopt(argc, argv, "abcf:ghinpruv")) != EOF) {
		switch (c) {
		case 'f':
			fname = optarg;
			break;
		case 'b':
			form |= XFS_BLOCK_QUOTA;
			break;
		case 'i':
			form |= XFS_INODE_QUOTA;
			break;
		case 'r':
			form |= XFS_RTBLOCK_QUOTA;
			break;
		case 'g':
			type |= XFS_GROUP_QUOTA;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			break;
		case 'a':
			flags |= ALL_MOUNTS_FLAG;
			break;
		case 'c':
			flags |= HISTOGRAM_FLAG;
			break;
		case 'n':
			flags |= NO_LOOKUP_FLAG;
			break;
		case 'v':
			flags |= VERBOSE_FLAG;
			break;
		default:
			return command_usage(&quot_cmd);
		}
	}

	if (!form)
		form = XFS_BLOCK_QUOTA;

	if (!type) {
		type = XFS_USER_QUOTA;
	} else if (type != XFS_GROUP_QUOTA &&
	           type != XFS_PROJ_QUOTA &&
	           type != XFS_USER_QUOTA) {
		return command_usage(&quot_cmd);
	}

	if ((fp = fopen_write_secure(fname)) == NULL)
		return 0;

	if (argc == optind) {
		if (flags & ALL_MOUNTS_FLAG)
			quot_any_type(fp, form, type, NULL, flags);
		else if (fs_path->fs_flags & FS_MOUNT_POINT)
			quot_any_type(fp, form, type, fs_path->fs_dir, flags);
	} else while (argc > optind) {
		quot_any_type(fp, form, type, argv[optind++], flags);
	}

	if (fname)
		fclose(fp);
	return 0;
}

void
quot_init(void)
{
	quot_cmd.name = "quot";
	quot_cmd.cfunc = quot_f;
	quot_cmd.argmin = 0;
	quot_cmd.argmax = -1;
	quot_cmd.args = _("[-bir] [-g|-p|-u] [-acv] [-f file]");
	quot_cmd.oneline = _("summarize filesystem ownership");
	quot_cmd.help = quot_help;

	if (expert)
		add_command(&quot_cmd);
}
