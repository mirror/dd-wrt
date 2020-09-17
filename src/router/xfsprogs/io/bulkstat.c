// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "libfrog/logging.h"
#include "libfrog/fsgeom.h"
#include "libfrog/bulkstat.h"
#include "libfrog/paths.h"
#include "io.h"
#include "input.h"

static void
dump_bulkstat_time(
	const char		*tag,
	uint64_t		sec,
	uint32_t		nsec)
{
	printf("\t%s = %"PRIu64".%"PRIu32"\n", tag, sec, nsec);
}

static void
dump_bulkstat(
	struct xfs_bulkstat	*bstat)
{
	printf("bs_ino = %"PRIu64"\n", bstat->bs_ino);
	printf("\tbs_size = %"PRIu64"\n", bstat->bs_size);

	printf("\tbs_blocks = %"PRIu64"\n", bstat->bs_blocks);
	printf("\tbs_xflags = 0x%"PRIx64"\n", bstat->bs_xflags);

	dump_bulkstat_time("bs_atime", bstat->bs_atime, bstat->bs_atime_nsec);
	dump_bulkstat_time("bs_ctime", bstat->bs_ctime, bstat->bs_ctime_nsec);
	dump_bulkstat_time("bs_mtime", bstat->bs_mtime, bstat->bs_mtime_nsec);
	dump_bulkstat_time("bs_btime", bstat->bs_btime, bstat->bs_btime_nsec);

	printf("\tbs_gen = 0x%"PRIx32"\n", bstat->bs_gen);
	printf("\tbs_uid = %"PRIu32"\n", bstat->bs_uid);
	printf("\tbs_gid = %"PRIu32"\n", bstat->bs_gid);
	printf("\tbs_projectid = %"PRIu32"\n", bstat->bs_projectid);

	printf("\tbs_blksize = %"PRIu32"\n", bstat->bs_blksize);
	printf("\tbs_rdev = %"PRIu32"\n", bstat->bs_rdev);
	printf("\tbs_cowextsize_blks = %"PRIu32"\n", bstat->bs_cowextsize_blks);
	printf("\tbs_extsize_blks = %"PRIu32"\n", bstat->bs_extsize_blks);

	printf("\tbs_nlink = %"PRIu32"\n", bstat->bs_nlink);
	printf("\tbs_extents = %"PRIu32"\n", bstat->bs_extents);
	printf("\tbs_aextents = %"PRIu32"\n", bstat->bs_aextents);
	printf("\tbs_version = %"PRIu16"\n", bstat->bs_version);
	printf("\tbs_forkoff = %"PRIu16"\n", bstat->bs_forkoff);

	printf("\tbs_sick = 0x%"PRIx16"\n", bstat->bs_sick);
	printf("\tbs_checked = 0x%"PRIx16"\n", bstat->bs_checked);
	printf("\tbs_mode = 0%"PRIo16"\n", bstat->bs_mode);
};

static void
bulkstat_help(void)
{
	printf(_(
"Bulk-queries the filesystem for inode stat information and prints it.\n"
"\n"
"   -a <agno>  Only iterate this AG.\n"
"   -d         Print debugging output.\n"
"   -e <ino>   Stop after this inode.\n"
"   -n <nr>    Ask for this many results at once.\n"
"   -s <ino>   Inode to start with.\n"
"   -v <ver>   Use this version of the ioctl (1 or 5).\n"));
}

static void
set_xfd_flags(
	struct xfs_fd	*xfd,
	uint32_t	ver)
{
	switch (ver) {
	case 1:
		xfd->flags |= XFROG_FLAG_BULKSTAT_FORCE_V1;
		break;
	case 5:
		xfd->flags |= XFROG_FLAG_BULKSTAT_FORCE_V5;
		break;
	default:
		break;
	}
}

static int
bulkstat_f(
	int			argc,
	char			**argv)
{
	struct xfs_fd		xfd = XFS_FD_INIT(file->fd);
	struct xfs_bulkstat_req	*breq;
	uint64_t		startino = 0;
	uint64_t		endino = -1ULL;
	uint32_t		batch_size = 4096;
	uint32_t		agno = 0;
	uint32_t		ver = 0;
	bool			has_agno = false;
	bool			debug = false;
	unsigned int		i;
	int			c;
	int			ret;

	while ((c = getopt(argc, argv, "a:de:n:s:v:")) != -1) {
		switch (c) {
		case 'a':
			agno = cvt_u32(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			has_agno = true;
			break;
		case 'd':
			debug = true;
			break;
		case 'e':
			endino = cvt_u64(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			break;
		case 'n':
			batch_size = cvt_u32(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			break;
		case 's':
			startino = cvt_u64(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			break;
		case 'v':
			ver = cvt_u32(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			if (ver != 1 && ver != 5) {
				fprintf(stderr, "version must be 1 or 5.\n");
				return 1;
			}
			break;
		default:
			bulkstat_help();
			return 0;
		}
	}
	if (optind != argc) {
		bulkstat_help();
		return 0;
	}

	ret = -xfd_prepare_geometry(&xfd);
	if (ret) {
		xfrog_perror(ret, "xfd_prepare_geometry");
		exitcode = 1;
		return 0;
	}

	ret = -xfrog_bulkstat_alloc_req(batch_size, startino, &breq);
	if (ret) {
		xfrog_perror(ret, "alloc bulkreq");
		exitcode = 1;
		return 0;
	}

	if (has_agno)
		xfrog_bulkstat_set_ag(breq, agno);

	set_xfd_flags(&xfd, ver);

	while ((ret = -xfrog_bulkstat(&xfd, breq)) == 0) {
		if (debug)
			printf(
_("bulkstat: startino=%lld flags=0x%x agno=%u ret=%d icount=%u ocount=%u\n"),
				(long long)breq->hdr.ino,
				(unsigned int)breq->hdr.flags,
				(unsigned int)breq->hdr.agno,
				ret,
				(unsigned int)breq->hdr.icount,
				(unsigned int)breq->hdr.ocount);
		if (breq->hdr.ocount == 0)
			break;

		for (i = 0; i < breq->hdr.ocount; i++) {
			if (breq->bulkstat[i].bs_ino > endino)
				break;
			dump_bulkstat(&breq->bulkstat[i]);
		}
	}
	if (ret) {
		xfrog_perror(ret, "xfrog_bulkstat");
		exitcode = 1;
	}

	free(breq);
	return 0;
}

static void
bulkstat_single_help(void)
{
	printf(_(
"Queries the filesystem for a single inode's stat information and prints it.\n"
"If a given inode is not allocated, information about the next allocated \n"
"inode will be printed instead.\n"
"\n"
"   -v (ver)  Use this version of the ioctl (1 or 5).\n"
"   -d        Print debugging information.\n"
"\n"
"Pass in inode numbers or a special inode name:\n"
"    root     Root directory.\n"));
}

struct single_map {
	const char		*tag;
	uint64_t		code;
};

struct single_map tags[] = {
	{"root", XFS_BULK_IREQ_SPECIAL_ROOT},
	{NULL, 0},
};

static int
bulkstat_single_f(
	int			argc,
	char			**argv)
{
	struct xfs_fd		xfd = XFS_FD_INIT(file->fd);
	struct xfs_bulkstat	bulkstat;
	unsigned long		ver = 0;
	unsigned int		i;
	bool			debug = false;
	int			c;
	int			ret;

	while ((c = getopt(argc, argv, "dv:")) != -1) {
		switch (c) {
		case 'd':
			debug = true;
			break;
		case 'v':
			errno = 0;
			ver = strtoull(optarg, NULL, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			if (ver != 1 && ver != 5) {
				fprintf(stderr, "version must be 1 or 5.\n");
				return 1;
			}
			break;
		default:
			bulkstat_single_help();
			return 0;
		}
	}

	ret = -xfd_prepare_geometry(&xfd);
	if (ret) {
		xfrog_perror(ret, "xfd_prepare_geometry");
		exitcode = 1;
		return 0;
	}

	set_xfd_flags(&xfd, ver);

	for (i = optind; i < argc; i++) {
		struct single_map	*sm = tags;
		uint64_t		ino;
		unsigned int		flags = 0;

		/* Try to look up our tag... */
		for (sm = tags; sm->tag; sm++) {
			if (!strcmp(argv[i], sm->tag)) {
				ino = sm->code;
				flags |= XFS_BULK_IREQ_SPECIAL;
				break;
			}
		}

		/* ...or else it's an inode number. */
		if (sm->tag == NULL) {
			errno = 0;
			ino = strtoull(argv[i], NULL, 10);
			if (errno) {
				perror(argv[i]);
				exitcode = 1;
				return 0;
			}
		}

		ret = -xfrog_bulkstat_single(&xfd, ino, flags, &bulkstat);
		if (ret) {
			xfrog_perror(ret, "xfrog_bulkstat_single");
			continue;
		}

		if (debug)
			printf(
_("bulkstat_single: startino=%"PRIu64" flags=0x%"PRIx32" ret=%d\n"),
				ino, flags, ret);

		dump_bulkstat(&bulkstat);
	}

	return 0;
}

static void
dump_inumbers(
	struct xfs_inumbers	*inumbers)
{
	printf("xi_startino = %"PRIu64"\n", inumbers->xi_startino);
	printf("\txi_allocmask = 0x%"PRIx64"\n", inumbers->xi_allocmask);
	printf("\txi_alloccount = %"PRIu8"\n", inumbers->xi_alloccount);
	printf("\txi_version = %"PRIu8"\n", inumbers->xi_version);
}

static void
inumbers_help(void)
{
	printf(_(
"Queries the filesystem for inode group information and prints it.\n"
"\n"
"   -a <agno>  Only iterate this AG.\n"
"   -d         Print debugging output.\n"
"   -e <ino>   Stop after this inode.\n"
"   -n <nr>    Ask for this many results at once.\n"
"   -s <ino>   Inode to start with.\n"
"   -v <ver>   Use this version of the ioctl (1 or 5).\n"));
}

static int
inumbers_f(
	int			argc,
	char			**argv)
{
	struct xfs_fd		xfd = XFS_FD_INIT(file->fd);
	struct xfs_inumbers_req	*ireq;
	uint64_t		startino = 0;
	uint64_t		endino = -1ULL;
	uint32_t		batch_size = 4096;
	uint32_t		agno = 0;
	uint32_t		ver = 0;
	bool			has_agno = false;
	bool			debug = false;
	unsigned int		i;
	int			c;
	int			ret;

	while ((c = getopt(argc, argv, "a:de:n:s:v:")) != -1) {
		switch (c) {
		case 'a':
			agno = cvt_u32(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			has_agno = true;
			break;
		case 'd':
			debug = true;
			break;
		case 'e':
			endino = cvt_u64(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			break;
		case 'n':
			batch_size = cvt_u32(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			break;
		case 's':
			startino = cvt_u64(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			break;
		case 'v':
			ver = cvt_u32(optarg, 10);
			if (errno) {
				perror(optarg);
				return 1;
			}
			if (ver != 1 && ver != 5) {
				fprintf(stderr, "version must be 1 or 5.\n");
				return 1;
			}
			break;
		default:
			bulkstat_help();
			return 0;
		}
	}
	if (optind != argc) {
		bulkstat_help();
		return 0;
	}

	ret = -xfd_prepare_geometry(&xfd);
	if (ret) {
		xfrog_perror(ret, "xfd_prepare_geometry");
		exitcode = 1;
		return 0;
	}

	ret = -xfrog_inumbers_alloc_req(batch_size, startino, &ireq);
	if (ret) {
		xfrog_perror(ret, "alloc inumbersreq");
		exitcode = 1;
		return 0;
	}

	if (has_agno)
		xfrog_inumbers_set_ag(ireq, agno);

	set_xfd_flags(&xfd, ver);

	while ((ret = -xfrog_inumbers(&xfd, ireq)) == 0) {
		if (debug)
			printf(
_("bulkstat: startino=%"PRIu64" flags=0x%"PRIx32" agno=%"PRIu32" ret=%d icount=%"PRIu32" ocount=%"PRIu32"\n"),
				ireq->hdr.ino,
				ireq->hdr.flags,
				ireq->hdr.agno,
				ret,
				ireq->hdr.icount,
				ireq->hdr.ocount);
		if (ireq->hdr.ocount == 0)
			break;

		for (i = 0; i < ireq->hdr.ocount; i++) {
			if (ireq->inumbers[i].xi_startino > endino)
				break;
			dump_inumbers(&ireq->inumbers[i]);
		}
	}
	if (ret) {
		xfrog_perror(ret, "xfrog_inumbers");
		exitcode = 1;
	}

	free(ireq);
	return 0;
}

static cmdinfo_t	bulkstat_cmd = {
	.name = "bulkstat",
	.cfunc = bulkstat_f,
	.argmin = 0,
	.argmax = -1,
	.flags = CMD_NOMAP_OK | CMD_FLAG_ONESHOT,
	.help = bulkstat_help,
};

static cmdinfo_t	bulkstat_single_cmd = {
	.name = "bulkstat_single",
	.cfunc = bulkstat_single_f,
	.argmin = 1,
	.argmax = -1,
	.flags = CMD_NOMAP_OK | CMD_FLAG_ONESHOT,
	.help = bulkstat_single_help,
};

static cmdinfo_t	inumbers_cmd = {
	.name = "inumbers",
	.cfunc = inumbers_f,
	.argmin = 0,
	.argmax = -1,
	.flags = CMD_NOMAP_OK | CMD_FLAG_ONESHOT,
	.help = inumbers_help,
};

void
bulkstat_init(void)
{
	bulkstat_cmd.args =
_("[-a agno] [-d] [-e endino] [-n batchsize] [-s startino] [-v version]");
	bulkstat_cmd.oneline = _("Bulk stat of inodes in a filesystem");

	bulkstat_single_cmd.args = _("[-d] [-v version] inum...");
	bulkstat_single_cmd.oneline = _("Stat one inode in a filesystem");

	inumbers_cmd.args =
_("[-a agno] [-d] [-e endino] [-n batchsize] [-s startino] [-v version]");
	inumbers_cmd.oneline = _("Query inode groups in a filesystem");

	add_command(&bulkstat_cmd);
	add_command(&bulkstat_single_cmd);
	add_command(&inumbers_cmd);
}
