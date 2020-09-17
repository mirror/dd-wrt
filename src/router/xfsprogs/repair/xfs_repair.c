// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "libxlog.h"
#include <sys/resource.h>
#include "xfs_multidisk.h"
#include "avl.h"
#include "libfrog/avl64.h"
#include "globals.h"
#include "versions.h"
#include "agheader.h"
#include "protos.h"
#include "incore.h"
#include "err_protos.h"
#include "prefetch.h"
#include "threads.h"
#include "progress.h"
#include "dinode.h"
#include "slab.h"
#include "rmap.h"
#include "libfrog/fsgeom.h"
#include "libfrog/platform.h"
#include "bulkload.h"
#include "quotacheck.h"

/*
 * option tables for getsubopt calls
 */

/*
 * -o: user-supplied override options
 */
enum o_opt_nums {
	ASSUME_XFS = 0,
	IHASH_SIZE,
	BHASH_SIZE,
	AG_STRIDE,
	FORCE_GEO,
	PHASE2_THREADS,
	BLOAD_LEAF_SLACK,
	BLOAD_NODE_SLACK,
	NOQUOTA,
	O_MAX_OPTS,
};

static char *o_opts[] = {
	[ASSUME_XFS]		= "assume_xfs",
	[IHASH_SIZE]		= "ihash",
	[BHASH_SIZE]		= "bhash",
	[AG_STRIDE]		= "ag_stride",
	[FORCE_GEO]		= "force_geometry",
	[PHASE2_THREADS]	= "phase2_threads",
	[BLOAD_LEAF_SLACK]	= "debug_bload_leaf_slack",
	[BLOAD_NODE_SLACK]	= "debug_bload_node_slack",
	[NOQUOTA]		= "noquota",
	[O_MAX_OPTS]		= NULL,
};

/*
 * -c: conversion options
 */
enum c_opt_nums {
	CONVERT_LAZY_COUNT = 0,
	C_MAX_OPTS,
};

static char *c_opts[] = {
	[CONVERT_LAZY_COUNT]	= "lazycount",
	[C_MAX_OPTS]		= NULL,
};


static int	bhash_option_used;
static long	max_mem_specified;	/* in megabytes */
static int	phase2_threads = 32;
static bool	report_corrected;

static void
usage(void)
{
	do_warn(_(
"Usage: %s [options] device\n"
"\n"
"Options:\n"
"  -f           The device is a file\n"
"  -L           Force log zeroing. Do this as a last resort.\n"
"  -l logdev    Specifies the device where the external log resides.\n"
"  -m maxmem    Maximum amount of memory to be used in megabytes.\n"
"  -n           No modify mode, just checks the filesystem for damage.\n"
"               (Cannot be used together with -e.)\n"
"  -P           Disables prefetching.\n"
"  -r rtdev     Specifies the device where the realtime section resides.\n"
"  -v           Verbose output.\n"
"  -c subopts   Change filesystem parameters - use xfs_admin.\n"
"  -o subopts   Override default behaviour, refer to man page.\n"
"  -t interval  Reporting interval in seconds.\n"
"  -d           Repair dangerously.\n"
"  -e           Exit with a non-zero code if any errors were repaired.\n"
"               (Cannot be used together with -n.)\n"
"  -V           Reports version and exits.\n"), progname);
	exit(1);
}

char *
err_string(int err_code)
{
	static char *err_message[XR_BAD_ERR_CODE];
	static int done;

	if (!done) {
		err_message[XR_OK] = _("no error");
		err_message[XR_BAD_MAGIC] = _("bad magic number");
		err_message[XR_BAD_BLOCKSIZE] = _("bad blocksize field");
		err_message[XR_BAD_BLOCKLOG] = _("bad blocksize log field");
		err_message[XR_BAD_VERSION] = _("bad or unsupported version");
		err_message[XR_BAD_INPROGRESS] =
			_("filesystem mkfs-in-progress bit set");
		err_message[XR_BAD_FS_SIZE_DATA] =
			_("inconsistent filesystem geometry information");
		err_message[XR_BAD_INO_SIZE_DATA] =
	_("bad inode size or inconsistent with number of inodes/block"),
		err_message[XR_BAD_SECT_SIZE_DATA] = _("bad sector size");
		err_message[XR_AGF_GEO_MISMATCH] =
	_("AGF geometry info conflicts with filesystem geometry");
		err_message[XR_AGI_GEO_MISMATCH] =
	_("AGI geometry info conflicts with filesystem geometry");
		err_message[XR_SB_GEO_MISMATCH] =
	_("AG superblock geometry info conflicts with filesystem geometry");
		err_message[XR_EOF] = _("attempted to perform I/O beyond EOF");
		err_message[XR_BAD_RT_GEO_DATA] =
	_("inconsistent filesystem geometry in realtime filesystem component");
		err_message[XR_BAD_INO_MAX_PCT] =
			_("maximum indicated percentage of inodes > 100%");
		err_message[XR_BAD_INO_ALIGN] =
			_("inconsistent inode alignment value");
		err_message[XR_INSUFF_SEC_SB] =
	_("not enough secondary superblocks with matching geometry");
		err_message[XR_BAD_SB_UNIT] =
			_("bad stripe unit in superblock");
		err_message[XR_BAD_SB_WIDTH] =
			_("bad stripe width in superblock");
		err_message[XR_BAD_SVN] =
			_("bad shared version number in superblock");
		err_message[XR_BAD_CRC] =
			_("bad CRC in superblock");
		err_message[XR_BAD_DIR_SIZE_DATA] =
			_("inconsistent directory geometry information");
		err_message[XR_BAD_LOG_GEOMETRY] =
			_("inconsistent log geometry information");
		done = 1;
	}

	if (err_code < XR_OK || err_code >= XR_BAD_ERR_CODE)
		do_abort(_("bad error code - %d\n"), err_code);

	return(err_message[err_code]);
}

static void
noval(char opt, char *tbl[], int idx)
{
	do_warn(_("-%c %s option cannot have a value\n"), opt, tbl[idx]);
	usage();
}

static void
respec(char opt, char *tbl[], int idx)
{
	do_warn("-%c ", opt);
	if (tbl)
		do_warn("%s ", tbl[idx]);
	do_warn(_("option respecified\n"));
	usage();
}

static void
unknown(char opt, char *s)
{
	do_warn(_("unknown option -%c %s\n"), opt, s);
	usage();
}

/*
 * sets only the global argument flags and variables
 */
static void
process_args(int argc, char **argv)
{
	char *p;
	int c;

	log_spec = 0;
	fs_is_dirty = 0;
	verbose = 0;
	no_modify = 0;
	dangerously = 0;
	isa_file = 0;
	zap_log = 0;
	dumpcore = 0;
	full_ino_ex_data = 0;
	force_geo = 0;
	assume_xfs = 0;
	copied_sunit = 0;
	sb_inoalignmt = 0;
	sb_unit = 0;
	sb_width = 0;
	ag_stride = 0;
	thread_count = 1;
	report_interval = PROG_RPT_DEFAULT;
	report_corrected = false;

	/*
	 * XXX have to add suboption processing here
	 * attributes, quotas, nlinks, aligned_inos, sb_fbits
	 */
	while ((c = getopt(argc, argv, "c:o:fl:m:r:LnDvVdPet:")) != EOF)  {
		switch (c) {
		case 'D':
			dumpcore = 1;
			break;
		case 'o':
			p = optarg;
			while (*p != '\0')  {
				char *val;

				switch (getsubopt(&p, o_opts, &val))  {
				case ASSUME_XFS:
					if (val)
						noval('o', o_opts, ASSUME_XFS);
					if (assume_xfs)
						respec('o', o_opts, ASSUME_XFS);
					assume_xfs = 1;
					break;
				case IHASH_SIZE:
					do_warn(
		_("-o ihash option has been removed and will be ignored\n"));
					break;
				case BHASH_SIZE:
					if (max_mem_specified)
						do_abort(
		_("-o bhash option cannot be used with -m option\n"));
					if (!val)
						do_abort(
		_("-o bhash requires a parameter\n"));
					libxfs_bhash_size = (int)strtol(val, NULL, 0);
					bhash_option_used = 1;
					break;
				case AG_STRIDE:
					if (!val)
						do_abort(
		_("-o ag_stride requires a parameter\n"));
					ag_stride = (int)strtol(val, NULL, 0);
					break;
				case FORCE_GEO:
					if (val)
						noval('o', o_opts, FORCE_GEO);
					if (force_geo)
						respec('o', o_opts, FORCE_GEO);
					force_geo = 1;
					break;
				case PHASE2_THREADS:
					if (!val)
						do_abort(
		_("-o phase2_threads requires a parameter\n"));
					phase2_threads = (int)strtol(val, NULL, 0);
					break;
				case BLOAD_LEAF_SLACK:
					if (!val)
						do_abort(
		_("-o debug_bload_leaf_slack requires a parameter\n"));
					bload_leaf_slack = (int)strtol(val, NULL, 0);
					break;
				case BLOAD_NODE_SLACK:
					if (!val)
						do_abort(
		_("-o debug_bload_node_slack requires a parameter\n"));
					bload_node_slack = (int)strtol(val, NULL, 0);
					break;
				case NOQUOTA:
					quotacheck_skip();
					break;
				default:
					unknown('o', val);
					break;
				}
			}
			break;
		case 'c':
			p = optarg;
			while (*p) {
				char *val;

				switch (getsubopt(&p, c_opts, &val)) {
				case CONVERT_LAZY_COUNT:
					if (!val)
						do_abort(
		_("-c lazycount requires a parameter\n"));
					lazy_count = (int)strtol(val, NULL, 0);
					convert_lazy_count = 1;
					break;
				default:
					unknown('c', val);
					break;
				}
			}
			break;
		case 'l':
			log_name = optarg;
			log_spec = 1;
			break;
		case 'r':
			rt_name = optarg;
			rt_spec = 1;
			break;
		case 'f':
			isa_file = 1;
			break;
		case 'm':
			if (bhash_option_used)
				do_abort(_("-m option cannot be used with "
						"-o bhash option\n"));
			max_mem_specified = strtol(optarg, NULL, 0);
			break;
		case 'L':
			zap_log = 1;
			break;
		case 'n':
			no_modify = 1;
			break;
		case 'd':
			dangerously = 1;
			break;
		case 'v':
			verbose++;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		case 'P':
			do_prefetch = 0;
			break;
		case 't':
			report_interval = (int)strtol(optarg, NULL, 0);
			break;
		case 'e':
			report_corrected = true;
			break;
		default:
			usage();
		}
	}

	if (argc - optind != 1)
		usage();

	if ((fs_name = argv[optind]) == NULL)
		usage();

	if (report_corrected && no_modify)
		usage();
}

void __attribute__((noreturn))
do_error(char const *msg, ...)
{
	va_list args;

	fprintf(stderr, _("\nfatal error -- "));

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	if (dumpcore)
		abort();
	exit(1);
}

/*
 * like do_error, only the error is internal, no system
 * error so no oserror processing
 */
void __attribute__((noreturn))
do_abort(char const *msg, ...)
{
	va_list args;

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	if (dumpcore)
		abort();
	exit(1);
}

void
do_warn(char const *msg, ...)
{
	va_list args;

	fs_is_dirty = 1;

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
}

/* no formatting */

void
do_log(char const *msg, ...)
{
	va_list args;

	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
}

/* Make sure a fixed-location inode is where it should be. */
static void
validate_sb_ino(
	xfs_ino_t	*ino,
	xfs_ino_t	expected_ino,
	const char	*tag)
{
	if (*ino == expected_ino)
		return;

	do_warn(
_("sb %s inode value %" PRIu64 " %sinconsistent with calculated value %"PRIu64"\n"),
		tag, *ino, *ino == NULLFSINO ? "(NULLFSINO) " : "",
		expected_ino);

	if (!no_modify)
		do_warn(
_("resetting superblock %s inode pointer to %"PRIu64"\n"),
			tag, expected_ino);
	else
		do_warn(
_("would reset superblock %s inode pointer to %"PRIu64"\n"),
			tag, expected_ino);

	/*
	 * Just set the value -- safe since the superblock doesn't get flushed
	 * out if no_modify is set.
	 */
	*ino = expected_ino;
}

/* Does the root directory inode look like a plausible root directory? */
static bool
has_plausible_rootdir(
	struct xfs_mount	*mp)
{
	struct xfs_inode	*ip;
	xfs_ino_t		ino;
	int			error;
	bool			ret = false;

	error = -libxfs_iget(mp, NULL, mp->m_sb.sb_rootino, 0, &ip);
	if (error)
		goto out;
	if (!S_ISDIR(VFS_I(ip)->i_mode))
		goto out_rele;

	error = -libxfs_dir_lookup(NULL, ip, &xfs_name_dotdot, &ino, NULL);
	if (error)
		goto out_rele;

	/* The root directory '..' entry points to the directory. */
	if (ino == mp->m_sb.sb_rootino)
		ret = true;

out_rele:
	libxfs_irele(ip);
out:
	return ret;
}

/*
 * If any of the secondary SBs contain a *correct* value for sunit, write that
 * back to the primary superblock.
 */
static void
guess_correct_sunit(
	struct xfs_mount	*mp)
{
	struct xfs_sb		sb;
	struct xfs_buf		*bp;
	xfs_ino_t		calc_rootino = NULLFSINO;
	xfs_agnumber_t		agno;
	unsigned int		new_sunit;
	unsigned int		sunit_guess;
	int			error;

	/* Try reading secondary supers to see if we find a good sb_unit. */
	for (agno = 1; agno < mp->m_sb.sb_agcount; agno++) {
		error = -libxfs_sb_read_secondary(mp, NULL, agno, &bp);
		if (error)
			continue;
		libxfs_sb_from_disk(&sb, bp->b_addr);
		libxfs_buf_relse(bp);

		calc_rootino = libxfs_ialloc_calc_rootino(mp, sb.sb_unit);
		if (calc_rootino == mp->m_sb.sb_rootino)
			break;
	}

	/* If we found a reasonable value, log where we found it. */
	if (calc_rootino == mp->m_sb.sb_rootino) {
		do_warn(_("AG %u superblock contains plausible sb_unit value\n"),
				agno);
		new_sunit = sb.sb_unit;
		goto fix;
	}

	/* Try successive powers of two. */
	for (sunit_guess = 1;
	     sunit_guess <= XFS_AG_MAX_BLOCKS(mp->m_sb.sb_blocklog);
	     sunit_guess *= 2) {
		calc_rootino = libxfs_ialloc_calc_rootino(mp, sunit_guess);
		if (calc_rootino == mp->m_sb.sb_rootino)
			break;
	}

	/* If we found a reasonable value, log where we found it. */
	if (calc_rootino == mp->m_sb.sb_rootino) {
		do_warn(_("Found an sb_unit value that looks plausible\n"));
		new_sunit = sunit_guess;
		goto fix;
	}

	do_warn(_("Could not estimate a plausible sb_unit value\n"));
	return;

fix:
	if (!no_modify)
		do_warn(_("Resetting sb_unit to %u\n"), new_sunit);
	else
		do_warn(_("Would reset sb_unit to %u\n"), new_sunit);

	/*
	 * Just set the value -- safe since the superblock doesn't get flushed
	 * out if no_modify is set.
	 */
	mp->m_sb.sb_unit = new_sunit;

	/* Make sure that swidth is still a multiple of sunit. */
	if (mp->m_sb.sb_width % mp->m_sb.sb_unit == 0)
		return;

	if (!no_modify)
		do_warn(_("Resetting sb_width to %u\n"), new_sunit);
	else
		do_warn(_("Would reset sb_width to %u\n"), new_sunit);
}

/*
 * Make sure that the first 3 inodes in the filesystem are the root directory,
 * the realtime bitmap, and the realtime summary, in that order.
 */
static void
calc_mkfs(
	struct xfs_mount	*mp)
{
	xfs_ino_t		rootino;

	rootino = libxfs_ialloc_calc_rootino(mp, mp->m_sb.sb_unit);

	/*
	 * If the root inode isn't where we think it is, check its plausibility
	 * as a root directory.  It's possible that somebody changed sunit
	 * since the filesystem was created, which can change the value of the
	 * above computation.  Don't blow up the root directory if this is the
	 * case.
	 */
	if (mp->m_sb.sb_rootino != rootino && has_plausible_rootdir(mp)) {
		do_warn(
_("sb root inode value %" PRIu64 " valid but in unaligned location (expected %"PRIu64") possibly due to sunit change\n"),
			mp->m_sb.sb_rootino, rootino);
		guess_correct_sunit(mp);
		rootino = mp->m_sb.sb_rootino;
	}

	validate_sb_ino(&mp->m_sb.sb_rootino, rootino,
			_("root"));
	validate_sb_ino(&mp->m_sb.sb_rbmino, rootino + 1,
			_("realtime bitmap"));
	validate_sb_ino(&mp->m_sb.sb_rsumino, rootino + 2,
			_("realtime summary"));
}

/*
 * v5 superblock metadata track the LSN of last modification and thus require
 * that the current LSN is always moving forward. The current LSN is reset if
 * the log has been cleared, which puts the log behind parts of the filesystem
 * on-disk and can disrupt log recovery.
 *
 * We have tracked the maximum LSN of every piece of metadata that has been read
 * in via the read verifiers. Compare the max LSN with the log and if the log is
 * behind, bump the cycle number and reformat the log.
 */
static void
format_log_max_lsn(
	struct xfs_mount	*mp)
{
	struct xlog		*log = mp->m_log;
	int			max_cycle;
	int			max_block;
	int			new_cycle;
	xfs_daddr_t		logstart;
	xfs_daddr_t		logblocks;
	int			logversion;

	if (!xfs_sb_version_hascrc(&mp->m_sb))
		return;

	/*
	 * If the log is ahead of the highest metadata LSN we've seen, we're
	 * safe and there's nothing to do.
	 */
	max_cycle = CYCLE_LSN(libxfs_max_lsn);
	max_block = BLOCK_LSN(libxfs_max_lsn);
	if (max_cycle < log->l_curr_cycle ||
	    (max_cycle == log->l_curr_cycle && max_block < log->l_curr_block))
		return;

	/*
	 * Going to the next cycle should be sufficient but we bump by a few
	 * counts to help cover any metadata LSNs we could have missed.
	 */
	new_cycle = max_cycle + 3;
	logstart = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart);
	logblocks = XFS_FSB_TO_BB(mp, mp->m_sb.sb_logblocks);
	logversion = xfs_sb_version_haslogv2(&mp->m_sb) ? 2 : 1;

	do_warn(_("Maximum metadata LSN (%d:%d) is ahead of log (%d:%d).\n"),
		max_cycle, max_block, log->l_curr_cycle, log->l_curr_block);

	if (no_modify) {
		do_warn(_("Would format log to cycle %d.\n"), new_cycle);
		return;
	}

	do_warn(_("Format log to cycle %d.\n"), new_cycle);
	libxfs_log_clear(log->l_dev, NULL, logstart, logblocks,
			 &mp->m_sb.sb_uuid, logversion, mp->m_sb.sb_logsunit,
			 XLOG_FMT, new_cycle, true);
}

/*
 * mkfs increases the AG count for "multidisk" configurations, we want
 * to target these for an increase in thread count. Hence check the superlock
 * geometry information to determine if mkfs considered this a multidisk
 * configuration.
 */
static bool
is_multidisk_filesystem(
	struct xfs_mount	*mp)
{
	struct xfs_sb		*sbp = &mp->m_sb;

	/* High agcount filesystems are always considered "multidisk" */
	if (sbp->sb_agcount >= XFS_MULTIDISK_AGCOUNT)
		return true;

	/*
	 * If it doesn't have a sunit/swidth, mkfs didn't consider it a
	 * multi-disk array, so we don't either.
	 */
	if (!sbp->sb_unit)
		return false;

	ASSERT(sbp->sb_width);
	return true;
}

/*
 * if the sector size of the filesystem we are trying to repair is
 * smaller than that of the underlying filesystem (i.e. we are repairing
 * an image), the we have to turn off direct IO because we cannot do IO
 * smaller than the host filesystem's sector size.
 */
static void
check_fs_vs_host_sectsize(
	struct xfs_sb	*sb)
{
	int	fd, ret;
	long	old_flags;
	struct xfs_fsop_geom	geom = { 0 };

	fd = libxfs_device_to_fd(x.ddev);

	ret = -xfrog_geometry(fd, &geom);
	if (ret) {
		do_log(_("Cannot get host filesystem geometry.\n"
	"Repair may fail if there is a sector size mismatch between\n"
	"the image and the host filesystem.\n"));
		geom.sectsize = BBSIZE;
	}

	if (sb->sb_sectsize < geom.sectsize) {
		old_flags = fcntl(fd, F_GETFL, 0);
		if (fcntl(fd, F_SETFL, old_flags & ~O_DIRECT) < 0) {
			do_warn(_(
	"Sector size on host filesystem larger than image sector size.\n"
	"Cannot turn off direct IO, so exiting.\n"));
			exit(1);
		}
	}
}

int
main(int argc, char **argv)
{
	xfs_mount_t	*temp_mp;
	xfs_mount_t	*mp;
	xfs_dsb_t	*dsb;
	xfs_buf_t	*sbp;
	xfs_mount_t	xfs_m;
	struct xlog	log = {0};
	char		*msgbuf;
	struct xfs_sb	psb;
	int		rval;
	struct xfs_ino_geometry	*igeo;
	int		error;

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	dinode_bmbt_translation_init();

	temp_mp = &xfs_m;
	setbuf(stdout, NULL);

	process_args(argc, argv);
	xfs_init(&x);

	msgbuf = malloc(DURATION_BUF_SIZE);

	timestamp(PHASE_START, 0, NULL);
	timestamp(PHASE_END, 0, NULL);

	/* -f forces this, but let's be nice and autodetect it, as well. */
	if (!isa_file) {
		int		fd = libxfs_device_to_fd(x.ddev);
		struct stat	statbuf;

		if (fstat(fd, &statbuf) < 0)
			do_warn(_("%s: couldn't stat \"%s\"\n"),
				progname, fs_name);
		else if (S_ISREG(statbuf.st_mode))
			isa_file = 1;
	}

	if (isa_file) {
		/* Best effort attempt to validate fs vs host sector size */
		rval = get_sb(&psb, 0, XFS_MAX_SECTORSIZE, 0);
		if (rval == XR_OK)
			check_fs_vs_host_sectsize(&psb);
	}

	/* do phase1 to make sure we have a superblock */
	phase1(temp_mp);
	timestamp(PHASE_END, 1, NULL);

	if (no_modify && primary_sb_modified)  {
		do_warn(_("Primary superblock would have been modified.\n"
			  "Cannot proceed further in no_modify mode.\n"
			  "Exiting now.\n"));
		exit(1);
	}

	rval = get_sb(&psb, 0, XFS_MAX_SECTORSIZE, 0);
	if (rval != XR_OK) {
		do_warn(_("Primary superblock bad after phase 1!\n"
			  "Exiting now.\n"));
		exit(1);
	}

	/*
	 * Now that we have completely validated the superblock, geometry may
	 * have changed; re-check geometry vs the host filesystem geometry
	 */
	if (isa_file)
		check_fs_vs_host_sectsize(&psb);

	/*
	 * Prepare the mount structure. Point the log reference to our local
	 * copy so it's available to the various phases. The log bits are
	 * initialized in phase 2.
	 */
	memset(&xfs_m, 0, sizeof(xfs_mount_t));
	mp = libxfs_mount(&xfs_m, &psb, x.ddev, x.logdev, x.rtdev, 0);

	if (!mp)  {
		fprintf(stderr,
			_("%s: cannot repair this filesystem.  Sorry.\n"),
			progname);
		exit(1);
	}
	mp->m_log = &log;
	igeo = M_IGEO(mp);

	/* Spit out function & line on these corruption macros */
	if (verbose > 2)
		mp->m_flags |= LIBXFS_MOUNT_WANT_CORRUPTED;

	/*
	 * set XFS-independent status vars from the mount/sb structure
	 */
	glob_agcount = mp->m_sb.sb_agcount;

	chunks_pblock = mp->m_sb.sb_inopblock / XFS_INODES_PER_CHUNK;
	max_symlink_blocks = libxfs_symlink_blocks(mp, XFS_SYMLINK_MAXLEN);

	/*
	 * Automatic striding for high agcount filesystems.
	 *
	 * More AGs indicates that the filesystem is either large or can handle
	 * more IO parallelism. Either way, we should try to process multiple
	 * AGs at a time in such a configuration to try to saturate the
	 * underlying storage and speed the repair process. Only do this if
	 * prefetching is enabled.
	 *
	 * Given mkfs defaults for 16AGs for "multidisk" configurations, we want
	 * to target these for an increase in thread count. Hence a stride value
	 * of 15 is chosen to ensure we get at least 2 AGs being scanned at once
	 * on such filesystems.
	 *
	 * Limit the maximum thread count based on the available CPU power that
	 * is available. If we use too many threads, we might run out of memory
	 * and CPU power before we run out of IO concurrency. We limit to 8
	 * threads/CPU as this is enough threads to saturate a CPU on fast
	 * devices, yet few enough that it will saturate but won't overload slow
	 * devices.
	 *
	 * Multidisk filesystems can handle more IO parallelism so we should try
	 * to process multiple AGs at a time in such a configuration to try to
	 * saturate the underlying storage and speed the repair process. Only do
	 * this if prefetching is enabled.
	 */
	if (!ag_stride && do_prefetch && is_multidisk_filesystem(mp)) {
		/*
		 * For small agcount multidisk systems, just double the
		 * parallelism. For larger AG count filesystems (32 and above)
		 * use more parallelism, and linearly increase the parallelism
		 * with the number of AGs.
		 */
		ag_stride = min(glob_agcount, XFS_MULTIDISK_AGCOUNT / 2) - 1;
	}

	if (ag_stride) {
		int max_threads = platform_nproc() * 8;

		thread_count = (glob_agcount + ag_stride - 1) / ag_stride;
		while (thread_count > max_threads) {
			ag_stride *= 2;
			thread_count = (glob_agcount + ag_stride - 1) /
								ag_stride;
		}
		if (thread_count > 0)
			thread_init();
		else {
			thread_count = 1;
			ag_stride = 0;
		}
	}

	if (ag_stride && report_interval) {
		init_progress_rpt();
		if (msgbuf) {
			do_log(_("        - reporting progress in intervals of %s\n"),
			duration(report_interval, msgbuf));
		}
	}

	/*
	 * Adjust libxfs cache sizes based on system memory,
	 * filesystem size and inode count.
	 *
	 * We'll set the cache size based on 3/4s the memory minus
	 * space used by the inode AVL tree and block usage map.
	 *
	 * Inode AVL tree space is approximately 4 bytes per inode,
	 * block usage map is currently 1 byte for 2 blocks.
	 *
	 * We assume most blocks will be inode clusters.
	 *
	 * Calculations are done in kilobyte units.
	 */

	if (!bhash_option_used || max_mem_specified) {
		unsigned long 	mem_used;
		unsigned long	max_mem;
		struct rlimit	rlim;

		libxfs_bcache_purge();
		cache_destroy(libxfs_bcache);

		mem_used = (mp->m_sb.sb_icount >> (10 - 2)) +
					(mp->m_sb.sb_dblocks >> (10 + 1)) +
					50000;	/* rough estimate of 50MB overhead */
		max_mem = max_mem_specified ? max_mem_specified * 1024 :
					      platform_physmem() * 3 / 4;

		if (getrlimit(RLIMIT_AS, &rlim) != -1 &&
					rlim.rlim_cur != RLIM_INFINITY) {
			rlim.rlim_cur = rlim.rlim_max;
			setrlimit(RLIMIT_AS, &rlim);
			/* use approximately 80% of rlimit to avoid overrun */
			max_mem = min(max_mem, rlim.rlim_cur / 1280);
		} else
			max_mem = min(max_mem, (LONG_MAX >> 10) + 1);

		if (verbose > 1)
			do_log(
	_("        - max_mem = %lu, icount = %" PRIu64 ", imem = %" PRIu64 ", dblock = %" PRIu64 ", dmem = %" PRIu64 "\n"),
				max_mem, mp->m_sb.sb_icount,
				mp->m_sb.sb_icount >> (10 - 2),
				mp->m_sb.sb_dblocks,
				mp->m_sb.sb_dblocks >> (10 + 1));

		if (max_mem <= mem_used) {
			if (max_mem_specified) {
				do_abort(
	_("Required memory for repair is greater that the maximum specified\n"
	  "with the -m option. Please increase it to at least %lu.\n"),
					mem_used / 1024);
			}
			do_log(
	_("Memory available for repair (%luMB) may not be sufficient.\n"
	  "At least %luMB is needed to repair this filesystem efficiently\n"
	  "If repair fails due to lack of memory, please\n"),
				max_mem / 1024, mem_used / 1024);
			if (do_prefetch)
				do_log(
	_("turn prefetching off (-P) to reduce the memory footprint.\n"));
			else
				do_log(
	_("increase system RAM and/or swap space to at least %luMB.\n"),
			mem_used * 2 / 1024);

			max_mem = mem_used;
		}

		max_mem -= mem_used;
		if (max_mem >= (1 << 30))
			max_mem = 1 << 30;
		libxfs_bhash_size = max_mem / (HASH_CACHE_RATIO *
				(igeo->inode_cluster_size >> 10));
		if (libxfs_bhash_size < 512)
			libxfs_bhash_size = 512;

		if (verbose)
			do_log(_("        - block cache size set to %d entries\n"),
				libxfs_bhash_size * HASH_CACHE_RATIO);

		libxfs_bcache = cache_init(0, libxfs_bhash_size,
						&libxfs_bcache_operations);
	}

	/*
	 * calculate what mkfs would do to this filesystem
	 */
	calc_mkfs(mp);

	/*
	 * initialize block alloc map
	 */
	init_bmaps(mp);
	incore_ino_init(mp);
	incore_ext_init(mp);
	rmaps_init(mp);

	/* initialize random globals now that we know the fs geometry */
	inodes_per_block = mp->m_sb.sb_inopblock;

	if (parse_sb_version(&mp->m_sb))  {
		do_warn(
	_("Found unsupported filesystem features.  Exiting now.\n"));
		return(1);
	}

	/* make sure the per-ag freespace maps are ok so we can mount the fs */
	phase2(mp, phase2_threads);
	timestamp(PHASE_END, 2, NULL);

	if (do_prefetch)
		init_prefetch(mp);

	phase3(mp, phase2_threads);
	timestamp(PHASE_END, 3, NULL);

	phase4(mp);
	timestamp(PHASE_END, 4, NULL);

	if (no_modify)
		printf(_("No modify flag set, skipping phase 5\n"));
	else {
		phase5(mp);
	}
	timestamp(PHASE_END, 5, NULL);

	/*
	 * Done with the block usage maps, toss them...
	 */
	rmaps_free(mp);
	free_bmaps(mp);

	if (!bad_ino_btree)  {
		phase6(mp);
		timestamp(PHASE_END, 6, NULL);

		phase7(mp, phase2_threads);
		timestamp(PHASE_END, 7, NULL);
	} else  {
		do_warn(
_("Inode allocation btrees are too corrupted, skipping phases 6 and 7\n"));
	}

	if (lost_quotas && !have_uquotino && !have_gquotino && !have_pquotino) {
		if (!no_modify)  {
			do_warn(
_("Warning:  no quota inodes were found.  Quotas disabled.\n"));
		} else  {
			do_warn(
_("Warning:  no quota inodes were found.  Quotas would be disabled.\n"));
		}
	} else if (lost_quotas)  {
		if (!no_modify)  {
			do_warn(
_("Warning:  quota inodes were cleared.  Quotas disabled.\n"));
		} else  {
			do_warn(
_("Warning:  quota inodes would be cleared.  Quotas would be disabled.\n"));
		}
	} else  {
		if (lost_uquotino)  {
			if (!no_modify)  {
				do_warn(
_("Warning:  user quota information was cleared.\n"
  "User quotas can not be enforced until limit information is recreated.\n"));
			} else  {
				do_warn(
_("Warning:  user quota information would be cleared.\n"
  "User quotas could not be enforced until limit information was recreated.\n"));
			}
		}

		if (lost_gquotino)  {
			if (!no_modify)  {
				do_warn(
_("Warning:  group quota information was cleared.\n"
  "Group quotas can not be enforced until limit information is recreated.\n"));
			} else  {
				do_warn(
_("Warning:  group quota information would be cleared.\n"
  "Group quotas could not be enforced until limit information was recreated.\n"));
			}
		}

		if (lost_pquotino)  {
			if (!no_modify)  {
				do_warn(
_("Warning:  project quota information was cleared.\n"
  "Project quotas can not be enforced until limit information is recreated.\n"));
			} else  {
				do_warn(
_("Warning:  project quota information would be cleared.\n"
  "Project quotas could not be enforced until limit information was recreated.\n"));
			}
		}
	}

	if (ag_stride && report_interval)
		stop_progress_rpt();

	if (no_modify)  {
		/*
		 * Warn if the current LSN is problematic and the log requires a
		 * reformat.
		 */
		format_log_max_lsn(mp);

		do_log(
	_("No modify flag set, skipping filesystem flush and exiting.\n"));
		if (verbose)
			summary_report();
		if (fs_is_dirty)
			return(1);

		return(0);
	}

	/*
	 * Clear the quota flags if they're on.
	 */
	sbp = libxfs_getsb(mp);
	if (!sbp)
		do_error(_("couldn't get superblock\n"));

	dsb = sbp->b_addr;

	if ((mp->m_sb.sb_qflags & XFS_ALL_QUOTA_CHKD) != quotacheck_results()) {
		do_warn(_("Note - quota info will be regenerated on next "
			"quota mount.\n"));
		dsb->sb_qflags &= cpu_to_be16(~(XFS_UQUOTA_CHKD |
						XFS_GQUOTA_CHKD |
						XFS_PQUOTA_CHKD |
						XFS_OQUOTA_CHKD));
	}

	if (copied_sunit) {
		do_warn(
_("Note - stripe unit (%d) and width (%d) were copied from a backup superblock.\n"
  "Please reset with mount -o sunit=<value>,swidth=<value> if necessary\n"),
			be32_to_cpu(dsb->sb_unit), be32_to_cpu(dsb->sb_width));
	}

	libxfs_buf_mark_dirty(sbp);
	libxfs_buf_relse(sbp);

	/*
	 * Done. Flush all cached buffers and inodes first to ensure all
	 * verifiers are run (where we discover the max metadata LSN), reformat
	 * the log if necessary and unmount.
	 */
	libxfs_bcache_flush();
	format_log_max_lsn(mp);

	/* Report failure if anything failed to get written to our fs. */
	error = -libxfs_umount(mp);
	if (error)
		do_error(
	_("File system metadata writeout failed, err=%d.  Re-run xfs_repair."),
				error);

	libxfs_destroy(&x);

	if (verbose)
		summary_report();
	do_log(_("done\n"));

	if (dangerously && !no_modify)
		do_warn(
_("Repair of readonly mount complete.  Immediate reboot encouraged.\n"));

	pftrace_done();

	free(msgbuf);

	if (fs_is_dirty && report_corrected)
		return (4);
	return (0);
}
