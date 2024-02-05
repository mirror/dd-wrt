// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <pthread.h>
#include <stdlib.h>
#include <paths.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/statvfs.h>
#include "platform_defs.h"
#include "input.h"
#include "libfrog/paths.h"
#include "xfs_scrub.h"
#include "common.h"
#include "descr.h"
#include "unicrash.h"
#include "progress.h"

/*
 * XFS Online Metadata Scrub (and Repair)
 *
 * The XFS scrubber uses custom XFS ioctls to probe more deeply into the
 * internals of the filesystem.  It takes advantage of scrubbing ioctls
 * to check all the records stored in a metadata object and to
 * cross-reference those records against the other filesystem metadata.
 *
 * After the program gathers command line arguments to figure out
 * exactly what the program is going to do, scrub execution is split up
 * into several separate phases:
 *
 * The "find geometry" phase queries XFS for the filesystem geometry.
 * The block devices for the data, realtime, and log devices are opened.
 * Kernel ioctls are test-queried to see if they actually work (the scrub
 * ioctl in particular), and any other filesystem-specific information
 * is gathered.
 *
 * In the "check internal metadata" phase, we call the metadata scrub
 * ioctl to check the filesystem's internal per-AG btrees.  This
 * includes the AG superblock, AGF, AGFL, and AGI headers, freespace
 * btrees, the regular and free inode btrees, the reverse mapping
 * btrees, and the reference counting btrees.  If the realtime device is
 * enabled, the realtime bitmap and reverse mapping btrees are checked.
 * Quotas, if enabled, are also checked in this phase.
 *
 * Each AG (and the realtime device) has its metadata checked in a
 * separate thread for better performance.  Errors in the internal
 * metadata can be fixed here prior to the inode scan; refer to the
 * section about the "repair filesystem" phase for more information.
 *
 * The "scan all inodes" phase uses BULKSTAT to scan all the inodes in
 * an AG in disk order.  The BULKSTAT information provides enough
 * information to construct a file handle that is used to check the
 * following parts of every file:
 *
 *  - The inode record
 *  - All three block forks (data, attr, CoW)
 *  - If it's a symlink, the symlink target.
 *  - If it's a directory, the directory entries.
 *  - All extended attributes
 *  - The parent pointer
 *
 * Multiple threads are started to check each the inodes of each AG in
 * parallel.  Errors in file metadata can be fixed here; see the section
 * about the "repair filesystem" phase for more information.
 *
 * Next comes the (configurable) "repair filesystem" phase.  The user
 * can instruct this program to fix all problems encountered; to fix
 * only optimality problems and leave the corruptions; or not to touch
 * the filesystem at all.  Any metadata repairs that did not succeed in
 * the previous two phases are retried here; if there are uncorrectable
 * errors, xfs_scrub stops here.
 *
 * To perform the actual repairs (or optimizations), we iterate all the
 * items on the per-AG action item list and ask the kernel to repair
 * them.  Items which are successfully repaired are removed from the
 * list.  If an item is not acted upon successfully (or the kernel asks us
 * to try again), we retry the actions until there is nothing left to
 * fix or we fail to make forward progress.  In that event, the
 * unfinished items are recorded as errors.  If there are no errors at
 * this point, we call FSTRIM on the filesystem.
 *
 * The next phase is the "check directory tree" phase.  In this phase,
 * every directory is opened (via file handle) to confirm that each
 * directory is connected to the root.  Directory entries are checked
 * for ambiguous Unicode normalization mappings, which is to say that we
 * look for pairs of entries whose utf-8 strings normalize to the same
 * code point sequence and map to different inodes, because that could
 * be used to trick a user into opening the wrong file.  The names of
 * extended attributes are checked for Unicode normalization collisions.
 *
 * In the "verify data file integrity" phase, we employ GETFSMAP to read
 * the reverse-mappings of all AGs and issue direct-reads of the
 * underlying disk blocks.  We rely on the underlying storage to have
 * checksummed the data blocks appropriately.  Multiple threads are
 * started to check each AG in parallel; a separate thread pool is used
 * to handle the direct reads.
 *
 * In the "check summary counters" phase, use GETFSMAP to tally up the
 * blocks and BULKSTAT to tally up the inodes we saw and compare that to
 * the statfs output.  This gives the user a rough estimate of how
 * thorough the scrub was.
 */

/*
 * Known debug tweaks (pass -d and set the environment variable):
 * XFS_SCRUB_FORCE_ERROR	-- pretend all metadata is corrupt
 * XFS_SCRUB_FORCE_REPAIR	-- repair all metadata even if it's ok
 * XFS_SCRUB_NO_KERNEL		-- pretend there is no kernel ioctl
 * XFS_SCRUB_NO_SCSI_VERIFY	-- disable SCSI VERIFY (if present)
 * XFS_SCRUB_PHASE		-- run only this scrub phase
 * XFS_SCRUB_THREADS		-- start exactly this number of threads
 * XFS_SCRUB_DISK_ERROR_INTERVAL-- simulate a disk error every this many bytes
 * XFS_SCRUB_DISK_VERIFY_SKIP	-- pretend disk verify read calls succeeded
 *
 * Available even in non-debug mode:
 * SERVICE_MODE			-- compress all error codes to 1 for LSB
 *				   service action compliance
 */

/* Program name; needed for libfrog error reports. */
char				*progname = "xfs_scrub";

/* Debug level; higher values mean more verbosity. */
unsigned int			debug;

/* Display resource usage at the end of each phase? */
static bool			display_rusage;

/* Background mode; higher values insert more pauses between scrub calls. */
unsigned int			bg_mode;

/* Number of threads we're allowed to use. */
unsigned int			force_nr_threads;

/* Verbosity; higher values print more information. */
bool				verbose;

/* Should we scrub the data blocks? */
static bool			scrub_data;

/* Size of a memory page. */
long				page_size;

/* Should we FSTRIM after a successful run? */
bool				want_fstrim = true;

/* If stdout/stderr are ttys, we can use richer terminal control. */
bool				stderr_isatty;
bool				stdout_isatty;

/*
 * If we are running as a service, we need to be careful about what
 * error codes we return to the calling process.
 */
bool				is_service;

#define SCRUB_RET_SUCCESS	(0)	/* no problems left behind */
#define SCRUB_RET_CORRUPT	(1)	/* corruption remains on fs */
#define SCRUB_RET_UNOPTIMIZED	(2)	/* fs could be optimized */
#define SCRUB_RET_OPERROR	(4)	/* operational problems */
#define SCRUB_RET_SYNTAX	(8)	/* cmdline args rejected */

static void __attribute__((noreturn))
usage(void)
{
	fprintf(stderr, _("Usage: %s [OPTIONS] mountpoint\n"), progname);
	fprintf(stderr, "\n");
	fprintf(stderr, _("Options:\n"));
	fprintf(stderr, _("  -a count     Stop after this many errors are found.\n"));
	fprintf(stderr, _("  -b           Background mode.\n"));
	fprintf(stderr, _("  -C fd        Print progress information to this fd.\n"));
	fprintf(stderr, _("  -e behavior  What to do if errors are found.\n"));
	fprintf(stderr, _("  -k           Do not FITRIM the free space.\n"));
	fprintf(stderr, _("  -m path      Path to /etc/mtab.\n"));
	fprintf(stderr, _("  -n           Dry run.  Do not modify anything.\n"));
	fprintf(stderr, _("  -T           Display timing/usage information.\n"));
	fprintf(stderr, _("  -v           Verbose output.\n"));
	fprintf(stderr, _("  -V           Print version.\n"));
	fprintf(stderr, _("  -x           Scrub file data too.\n"));

	exit(SCRUB_RET_SYNTAX);
}

#ifndef RUSAGE_BOTH
# define RUSAGE_BOTH		(-2)
#endif

/* Get resource usage for ourselves and all children. */
static int
scrub_getrusage(
	struct rusage		*usage)
{
	struct rusage		cusage;
	int			err;

	err = getrusage(RUSAGE_BOTH, usage);
	if (!err)
		return err;

	err = getrusage(RUSAGE_SELF, usage);
	if (err)
		return err;

	err = getrusage(RUSAGE_CHILDREN, &cusage);
	if (err)
		return err;

	usage->ru_minflt += cusage.ru_minflt;
	usage->ru_majflt += cusage.ru_majflt;
	usage->ru_nswap += cusage.ru_nswap;
	usage->ru_inblock += cusage.ru_inblock;
	usage->ru_oublock += cusage.ru_oublock;
	usage->ru_msgsnd += cusage.ru_msgsnd;
	usage->ru_msgrcv += cusage.ru_msgrcv;
	usage->ru_nsignals += cusage.ru_nsignals;
	usage->ru_nvcsw += cusage.ru_nvcsw;
	usage->ru_nivcsw += cusage.ru_nivcsw;
	return 0;
}

/*
 * Scrub Phase Dispatch
 *
 * The operations of the scrub program are split up into several
 * different phases.  Each phase builds upon the metadata checked in the
 * previous phase, which is to say that we may skip phase (X + 1) if our
 * scans in phase (X) reveal corruption.  A phase may be skipped
 * entirely.
 */

/* Resource usage for each phase. */
struct phase_rusage {
	struct rusage		ruse;
	struct timeval		time;
	unsigned long long	verified_bytes;
	void			*brk_start;
	const char		*descr;
};

/* Operations for each phase. */
#define DATASCAN_DUMMY_FN	((void *)1)
#define REPAIR_DUMMY_FN		((void *)2)
struct phase_ops {
	char		*descr;
	int		(*fn)(struct scrub_ctx *ctx);
	int		(*estimate_work)(struct scrub_ctx *ctx, uint64_t *items,
					 unsigned int *threads, int *rshift);
	bool		must_run;
};

/* Start tracking resource usage for a phase. */
static int
phase_start(
	struct phase_rusage	*pi,
	unsigned int		phase,
	const char		*descr)
{
	int			error;

	memset(pi, 0, sizeof(*pi));
	error = scrub_getrusage(&pi->ruse);
	if (error) {
		perror(_("getrusage"));
		return error;
	}
	pi->brk_start = sbrk(0);

	error = gettimeofday(&pi->time, NULL);
	if (error) {
		perror(_("gettimeofday"));
		return error;
	}

	pi->descr = descr;
	if ((verbose || display_rusage) && descr) {
		fprintf(stdout, _("Phase %u: %s\n"), phase, descr);
		fflush(stdout);
	}
	return error;
}

static inline unsigned long long
kbytes(unsigned long long x)
{
	return (x + 1023) / 1024;
}

static void
report_mem_usage(
	const char			*phase,
	const struct phase_rusage	*pi)
{
#if defined(HAVE_MALLINFO2) || defined(HAVE_MALLINFO)
# ifdef HAVE_MALLINFO2
	struct mallinfo2		mall_now = mallinfo2();
# else
	struct mallinfo			mall_now = mallinfo();
# endif
	fprintf(stdout, _("%sMemory used: %lluk/%lluk (%lluk/%lluk), "),
		phase,
		kbytes(mall_now.arena), kbytes(mall_now.hblkhd),
		kbytes(mall_now.uordblks), kbytes(mall_now.fordblks));
#else
	fprintf(stdout, _("%sMemory used: %lluk, "),
		phase,
		kbytes(((char *) sbrk(0)) - ((char *) pi->brk_start)));
#endif
}

/* Report usage stats. */
static int
phase_end(
	struct phase_rusage	*pi,
	unsigned int		phase)
{
	struct rusage		ruse_now;
	struct timeval		time_now;
	char			phasebuf[DESCR_BUFSZ];
	double			dt;
	unsigned long long	in, out;
	unsigned long long	io;
	double			i, o, t;
	double			din, dout, dtot;
	char			*iu, *ou, *tu, *dinu, *doutu, *dtotu;
	int			error;

	if (!display_rusage)
		return 0;

	error = gettimeofday(&time_now, NULL);
	if (error) {
		perror(_("gettimeofday"));
		return error;
	}
	dt = timeval_subtract(&time_now, &pi->time);

	error = scrub_getrusage(&ruse_now);
	if (error) {
		perror(_("getrusage"));
		return error;
	}

	if (phase)
		snprintf(phasebuf, DESCR_BUFSZ, _("Phase %u: "), phase);
	else
		phasebuf[0] = 0;

	report_mem_usage(phasebuf, pi);

	fprintf(stdout, _("time: %5.2f/%5.2f/%5.2fs\n"),
		timeval_subtract(&time_now, &pi->time),
		timeval_subtract(&ruse_now.ru_utime, &pi->ruse.ru_utime),
		timeval_subtract(&ruse_now.ru_stime, &pi->ruse.ru_stime));

	/* I/O usage */
	in =  ((unsigned long long)ruse_now.ru_inblock -
			pi->ruse.ru_inblock) << BBSHIFT;
	out = ((unsigned long long)ruse_now.ru_oublock -
			pi->ruse.ru_oublock) << BBSHIFT;
	io = in + out;
	if (io) {
		i = auto_space_units(in, &iu);
		o = auto_space_units(out, &ou);
		t = auto_space_units(io, &tu);
		din = auto_space_units(in / dt, &dinu);
		dout = auto_space_units(out / dt, &doutu);
		dtot = auto_space_units(io / dt, &dtotu);
		fprintf(stdout,
_("%sI/O: %.1f%s in, %.1f%s out, %.1f%s tot\n"),
			phasebuf, i, iu, o, ou, t, tu);
		fprintf(stdout,
_("%sI/O rate: %.1f%s/s in, %.1f%s/s out, %.1f%s/s tot\n"),
			phasebuf, din, dinu, dout, doutu, dtot, dtotu);
	}
	fflush(stdout);

	return 0;
}

/* Run all the phases of the scrubber. */
static bool
run_scrub_phases(
	struct scrub_ctx	*ctx,
	FILE			*progress_fp)
{
	struct phase_ops phases[] =
	{
		{
			.descr = _("Find filesystem geometry."),
			.fn = phase1_func,
			.must_run = true,
		},
		{
			.descr = _("Check internal metadata."),
			.fn = phase2_func,
			.estimate_work = phase2_estimate,
		},
		{
			.descr = _("Scan all inodes."),
			.fn = phase3_func,
			.estimate_work = phase3_estimate,
		},
		{
			.descr = _("Defer filesystem repairs."),
			.fn = REPAIR_DUMMY_FN,
			.estimate_work = phase4_estimate,
		},
		{
			.descr = _("Check directory tree."),
			.fn = phase5_func,
			.estimate_work = phase5_estimate,
		},
		{
			.descr = _("Verify data file integrity."),
			.fn = DATASCAN_DUMMY_FN,
			.estimate_work = phase6_estimate,
		},
		{
			.descr = _("Check summary counters."),
			.fn = phase7_func,
			.must_run = true,
		},
		{
			NULL
		},
	};
	struct phase_rusage	pi;
	struct phase_ops	*sp;
	uint64_t		max_work;
	unsigned int		debug_phase = 0;
	unsigned int		phase;
	int			rshift;
	int			ret = 0;

	if (debug_tweak_on("XFS_SCRUB_PHASE"))
		debug_phase = atoi(getenv("XFS_SCRUB_PHASE"));

	/* Run all phases of the scrub tool. */
	for (phase = 1, sp = phases; sp->fn; sp++, phase++) {
		/* Turn on certain phases if user said to. */
		if (sp->fn == DATASCAN_DUMMY_FN && scrub_data) {
			sp->fn = phase6_func;
		} else if (sp->fn == REPAIR_DUMMY_FN &&
			   ctx->mode == SCRUB_MODE_REPAIR) {
			sp->descr = _("Repair filesystem.");
			sp->fn = phase4_func;
			sp->must_run = true;
		}

		/* Skip certain phases unless they're turned on. */
		if (sp->fn == REPAIR_DUMMY_FN ||
		    sp->fn == DATASCAN_DUMMY_FN)
			continue;

		/* Allow debug users to force a particular phase. */
		if (debug_phase && phase != debug_phase && !sp->must_run)
			continue;

		/* Run this phase. */
		ret = phase_start(&pi, phase, sp->descr);
		if (ret)
			break;
		if (sp->estimate_work) {
			unsigned int		work_threads;

			ret = sp->estimate_work(ctx, &max_work,
					&work_threads, &rshift);
			if (ret)
				break;

			/*
			 * The thread that starts the worker threads is also
			 * allowed to contribute to the progress counters and
			 * whatever other per-thread data we need to allocate.
			 */
			work_threads++;
			ret = progress_init_phase(ctx, progress_fp, phase,
					max_work, rshift, work_threads);
			if (ret)
				break;
			ret = descr_init_phase(ctx, work_threads);
		} else {
			ret = progress_init_phase(ctx, NULL, phase, 0, 0, 0);
			if (ret)
				break;
			ret = descr_init_phase(ctx, 1);
		}
		if (ret)
			break;
		ret = sp->fn(ctx);
		if (ret) {
			str_info(ctx, ctx->mntpoint,
_("Scrub aborted after phase %d."),
					phase);
			break;
		}
		progress_end_phase();
		descr_end_phase();
		ret = phase_end(&pi, phase);
		if (ret)
			break;

		/* Too many errors? */
		if (scrub_excessive_errors(ctx)) {
			ret = ECANCELED;
			break;
		}
	}

	return ret;
}

static void
report_modifications(
	struct scrub_ctx	*ctx)
{
	if (ctx->repairs == 0 && ctx->preens == 0)
		return;

	if (ctx->repairs && ctx->preens)
		fprintf(stdout,
_("%s: repairs made: %llu; optimizations made: %llu.\n"),
				ctx->mntpoint, ctx->repairs, ctx->preens);
	else if (ctx->preens == 0)
		fprintf(stdout,
_("%s: repairs made: %llu.\n"),
				ctx->mntpoint, ctx->repairs);
	else if (ctx->repairs == 0)
		fprintf(stdout,
_("%s: optimizations made: %llu.\n"),
				ctx->mntpoint, ctx->preens);
}

static void
report_outcome(
	struct scrub_ctx	*ctx)
{
	unsigned long long	actionable_errors;

	actionable_errors = ctx->corruptions_found + ctx->runtime_errors;

	if (actionable_errors == 0 &&
	    ctx->unfixable_errors == 0 &&
	    ctx->warnings_found == 0) {
		log_info(ctx, _("No problems found."));
		return;
	}

	if (ctx->unfixable_errors) {
		fprintf(stderr, _("%s: unfixable errors found: %llu\n"),
				ctx->mntpoint, ctx->unfixable_errors);
		log_err(ctx, _("unfixable errors found: %llu"),
				ctx->unfixable_errors);
	}

	if (ctx->corruptions_found > 0) {
		fprintf(stderr, _("%s: corruptions found: %llu\n"),
				ctx->mntpoint, ctx->corruptions_found);
		log_err(ctx, _("corruptions found: %llu"),
				ctx->corruptions_found);
	}

	if (ctx->runtime_errors > 0) {
		fprintf(stderr, _("%s: operational errors found: %llu\n"),
				ctx->mntpoint, ctx->runtime_errors);
		log_err(ctx, _("operational errors found: %llu"),
				ctx->runtime_errors);
	}

	if (ctx->warnings_found > 0) {
		fprintf(stderr, _("%s: warnings found: %llu\n"), ctx->mntpoint,
				ctx->warnings_found);
		log_warn(ctx, _("warnings found: %llu"), ctx->warnings_found);
	}

	/*
	 * Don't advise the user to run repair unless we were successful in
	 * setting up the scrub and we actually saw corruptions.  Warnings
	 * are not corruptions.
	 */
	if (ctx->scrub_setup_succeeded && actionable_errors > 0) {
		char		*msg;

		if (ctx->mode == SCRUB_MODE_DRY_RUN)
			msg = _("%s: Re-run xfs_scrub without -n.\n");
		else
			msg = _("%s: Unmount and run xfs_repair.\n");

		fprintf(stderr, msg, ctx->mntpoint);
	}
}

/* Compile-time features discoverable via version strings */
#ifdef HAVE_LIBICU
# define XFS_SCRUB_HAVE_UNICODE	"+"
#else
# define XFS_SCRUB_HAVE_UNICODE	"-"
#endif

int
main(
	int			argc,
	char			**argv)
{
	struct scrub_ctx	ctx = {0};
	struct phase_rusage	all_pi;
	char			*mtab = NULL;
	FILE			*progress_fp = NULL;
	struct fs_path		*fsp;
	int			vflag = 0;
	int			c;
	int			fd;
	int			ret = SCRUB_RET_SUCCESS;
	int			error;

	fprintf(stdout, "EXPERIMENTAL xfs_scrub program in use! Use at your own risk!\n");

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	if (unicrash_load()) {
		fprintf(stderr,
	_("%s: couldn't initialize Unicode library.\n"),
				progname);
		goto out;
	}

	pthread_mutex_init(&ctx.lock, NULL);
	ctx.mode = SCRUB_MODE_REPAIR;
	ctx.error_action = ERRORS_CONTINUE;
	while ((c = getopt(argc, argv, "a:bC:de:km:nTvxV")) != EOF) {
		switch (c) {
		case 'a':
			ctx.max_errors = cvt_u64(optarg, 10);
			if (errno) {
				perror(optarg);
				usage();
			}
			break;
		case 'b':
			force_nr_threads = 1;
			bg_mode++;
			break;
		case 'C':
			errno = 0;
			fd = cvt_u32(optarg, 10);
			if (errno) {
				perror(optarg);
				usage();
			}
			progress_fp = fdopen(fd, "w");
			if (!progress_fp) {
				perror(optarg);
				usage();
			}
			break;
		case 'd':
			debug++;
			break;
		case 'e':
			if (!strcmp("continue", optarg))
				ctx.error_action = ERRORS_CONTINUE;
			else if (!strcmp("shutdown", optarg))
				ctx.error_action = ERRORS_SHUTDOWN;
			else {
				fprintf(stderr,
	_("Unknown error behavior \"%s\".\n"),
						optarg);
				usage();
			}
			break;
		case 'k':
			want_fstrim = false;
			break;
		case 'm':
			mtab = optarg;
			break;
		case 'n':
			ctx.mode = SCRUB_MODE_DRY_RUN;
			break;
		case 'T':
			display_rusage = true;
			break;
		case 'v':
			verbose = true;
			break;
		case 'V':
			vflag++;
			break;
		case 'x':
			scrub_data = true;
			break;
		default:
			usage();
		}
	}

	if (vflag) {
		if (vflag == 1)
			fprintf(stdout, _("%s version %s\n"),
					progname, VERSION);
		else
			fprintf(stdout, _("%s version %s %sUnicode\n"),
					progname, VERSION,
					XFS_SCRUB_HAVE_UNICODE);
		fflush(stdout);
		return SCRUB_RET_SUCCESS;
	}

	/* Override thread count if debugger */
	if (debug_tweak_on("XFS_SCRUB_THREADS")) {
		unsigned int	x;

		x = cvt_u32(getenv("XFS_SCRUB_THREADS"), 10);
		if (errno) {
			perror("nr_threads");
			usage();
		}
		force_nr_threads = x;
	}

	if (optind != argc - 1)
		usage();

	ctx.mntpoint = argv[optind];

	stdout_isatty = isatty(STDOUT_FILENO);
	stderr_isatty = isatty(STDERR_FILENO);

	/* If interactive, start the progress bar. */
	if (stdout_isatty && !progress_fp)
		progress_fp = fdopen(1, "w+");

	if (getenv("SERVICE_MODE"))
		is_service = true;

	/* Initialize overall phase stats. */
	error = phase_start(&all_pi, 0, NULL);
	if (error)
		return SCRUB_RET_OPERROR;

	/* Find the mount record for the passed-in argument. */
	if (stat(argv[optind], &ctx.mnt_sb) < 0) {
		fprintf(stderr,
			_("%s: could not stat: %s: %s\n"),
			progname, argv[optind], strerror(errno));
		ctx.runtime_errors++;
		goto out;
	}

	/*
	 * If the user did not specify an explicit mount table, try to use
	 * /proc/mounts if it is available, else /etc/mtab.  We prefer
	 * /proc/mounts because it is kernel controlled, while /etc/mtab
	 * may contain garbage that userspace tools like pam_mounts wrote
	 * into it.
	 */
	if (!mtab) {
		if (access(_PATH_PROC_MOUNTS, R_OK) == 0)
			mtab = _PATH_PROC_MOUNTS;
		else
			mtab = _PATH_MOUNTED;
	}

	fs_table_initialise(0, NULL, 0, NULL);
	fsp = fs_table_lookup_mount(ctx.mntpoint);
	if (!fsp) {
		fprintf(stderr, _("%s: Not a XFS mount point.\n"),
				ctx.mntpoint);
		ret |= SCRUB_RET_SYNTAX;
		goto out;
	}
	memcpy(&ctx.fsinfo, fsp, sizeof(struct fs_path));

	/* Set up a page-aligned buffer for read verification. */
	page_size = sysconf(_SC_PAGESIZE);
	if (page_size < 0) {
		str_errno(&ctx, ctx.mntpoint);
		goto out;
	}

	if (debug_tweak_on("XFS_SCRUB_FORCE_REPAIR"))
		ctx.mode = SCRUB_MODE_REPAIR;

	/* Scrub a filesystem. */
	error = run_scrub_phases(&ctx, progress_fp);
	if (error && ctx.runtime_errors == 0)
		ctx.runtime_errors++;

	/*
	 * Excessive errors will cause the scrub phases to bail out early.
	 * We don't want every thread yelling that into the output, so check
	 * if we hit the threshold and tell the user *once*.
	 */
	if (scrub_excessive_errors(&ctx))
		str_info(&ctx, ctx.mntpoint, _("Too many errors; aborting."));

	if (debug_tweak_on("XFS_SCRUB_FORCE_ERROR"))
		str_info(&ctx, ctx.mntpoint, _("Injecting error."));

	/* Clean up scan data. */
	error = scrub_cleanup(&ctx);
	if (error && ctx.runtime_errors == 0)
		ctx.runtime_errors++;

out:
	report_modifications(&ctx);
	report_outcome(&ctx);

	if (ctx.corruptions_found) {
		if (ctx.error_action == ERRORS_SHUTDOWN)
			xfs_shutdown_fs(&ctx);
		ret |= SCRUB_RET_CORRUPT;
	}
	if (ctx.warnings_found)
		ret |= SCRUB_RET_UNOPTIMIZED;
	if (ctx.runtime_errors)
		ret |= SCRUB_RET_OPERROR;
	phase_end(&all_pi, 0);
	if (progress_fp)
		fclose(progress_fp);
	unicrash_unload();

	/*
	 * If we're being run as a service, the return code must fit the LSB
	 * init script action error guidelines, which is to say that we
	 * compress all errors to 1 ("generic or unspecified error", LSB 5.0
	 * section 22.2) and hope the admin will scan the log for what
	 * actually happened.
	 *
	 * We have to sleep 2 seconds here because journald uses the pid to
	 * connect our log messages to the systemd service.  This is critical
	 * for capturing all the log messages if the scrub fails, because the
	 * fail service uses the service name to gather log messages for the
	 * error report.
	 *
	 * Note: We don't count a lack of kernel support as a service failure
	 * because we haven't determined that there's anything wrong with the
	 * filesystem.
	 */
	if (is_service) {
		sleep(2);
		if (!ctx.scrub_setup_succeeded)
			return 0;
		if (ret != SCRUB_RET_SUCCESS)
			return 1;
	}

	return ret;
}
