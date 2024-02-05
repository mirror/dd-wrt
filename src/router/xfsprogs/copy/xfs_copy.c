// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include "xfs_copy.h"
#include "libxlog.h"
#include "libfrog/platform.h"

#define	rounddown(x, y)	(((x)/(y))*(y))
#define uuid_equal(s,d) (platform_uuid_compare((s),(d)) == 0)

extern int	platform_check_ismounted(char *, char *, struct stat *, int);

static char 		*logfile_name;
static FILE		*logerr;
static char		LOGFILE_NAME[] = "/var/tmp/xfs_copy.log.XXXXXX";

static char		*source_name;
static int		source_fd;

static unsigned int	source_blocksize;	/* source filesystem blocksize */
static unsigned int	source_sectorsize;	/* source disk sectorsize */

static xfs_agblock_t	first_agbno;

static uint64_t	barcount[11];

static unsigned int	num_targets;
static target_control	*target;

static wbuf		w_buf;
static wbuf		btree_buf;

static unsigned int	kids;

static thread_control	glob_masks;
static thread_args	*targ;

static pthread_mutex_t	mainwait;

#define ACTIVE		1
#define INACTIVE	2

xfs_off_t	write_log_trailer(int fd, wbuf *w, xfs_mount_t *mp);
xfs_off_t	write_log_header(int fd, wbuf *w, xfs_mount_t *mp);
static int	format_logs(struct xfs_mount *);

/* general purpose message reporting routine */

#define OUT	0x01		/* use stdout stream */
#define ERR	0x02		/* use stderr stream */
#define LOG	0x04		/* use logerr stream */
#define PRE	0x08		/* append strerror string */
#define LAST	0x10		/* final message we print */

static void
signal_maskfunc(int addset, int newset)
{
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, addset);
	sigprocmask(newset, &set, NULL);
}

static void
do_message(int flags, int code, const char *fmt, ...)
{
	va_list	ap;
	int	eek = 0;

	if (flags & LOG) {
		va_start(ap, fmt);
		if (vfprintf(logerr, fmt, ap) <= 0)
			eek = 1;
		va_end(ap);
	}
	if (eek)
		flags |= ERR;	/* failed, force stderr */
	if (flags & ERR) {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	} else if (flags & OUT) {
		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		va_end(ap);
	}

	if (flags & PRE) {
		do_message(flags & ~PRE, 0, ":  %s\n", strerror(code));
		if (flags & LAST)
			fprintf(stderr,
				_("Check logfile \"%s\" for more details\n"),
				logfile_name);
	}

	/* logfile is broken, force a write to stderr */
	if (eek) {
		fprintf(stderr, _("%s:  could not write to logfile \"%s\".\n"),
			progname, logfile_name);
		fprintf(stderr,
			_("Aborting XFS copy -- logfile error -- reason: %s\n"),
			strerror(errno));
		rcu_unregister_thread();
		pthread_exit(NULL);
	}
}

#define do_out(args...)		do_message(OUT|LOG, 0, ## args)
#define do_log(args...)		do_message(ERR|LOG, 0, ## args)
#define do_warn(args...)	do_message(LOG, 0, ## args)
#define do_error(e,s)		do_message(ERR|LOG|PRE, e, s)
#define do_fatal(e,s)		do_message(ERR|LOG|PRE|LAST, e, s)
#define do_vfatal(e,s,args...)	do_message(ERR|LOG|PRE|LAST, e, s, ## args)
#define die_perror() \
		do { \
			do_message(ERR|LOG|PRE|LAST, errno, \
				_("Aborting XFS copy - reason")); \
			exit(1); \
		} while (0)

/* workaround craziness in the xlog routines */
int xlog_recover_do_trans(struct xlog *log, struct xlog_recover *t, int p)
{
	return 0;
}

static void
check_errors(void)
{
	int	i, first_error = 0;

	for (i = 0; i < num_targets; i++)  {
		if (target[i].state != INACTIVE) {
			if (platform_flush_device(target[i].fd, 0)) {
				target[i].error = errno;
				target[i].state = INACTIVE;
				target[i].err_type = 2;
			}
		}

		if (target[i].state == INACTIVE)  {
			if (first_error == 0)  {
				first_error++;
				do_log(
				_("THE FOLLOWING COPIES FAILED TO COMPLETE\n"));
			}
			do_log("    %s -- ", target[i].name);
			switch (target[i].err_type) {
			case 0:
				do_log(_("write error"));
				break;
			case 1:
				do_log(_("lseek error"));
				break;
			case 2:
				do_log(_("flush error"));
				break;
			default:
				do_log(_("unknown error type %d"),
						target[i].err_type);
				break;
			}
			do_log(_(" at offset %lld\n"), target[i].position);
		}
	}
	if (first_error == 0)  {
		fprintf(stdout, _("All copies completed.\n"));
		fflush(NULL);
	} else  {
		fprintf(stderr, _("See \"%s\" for more details.\n"),
			logfile_name);
		exit(1);
	}
}

/*
 * don't have to worry about alignment and mins because those
 * are taken care of when the buffer's read in
 */
static int
do_write(
	thread_args	*args,
	wbuf		*buf)
{
	int		res;
	int		error = 0;

	if (!buf)
		buf = &w_buf;

	if (target[args->id].position != buf->position)  {
		if (lseek(args->fd, buf->position, SEEK_SET) < 0)  {
			error = target[args->id].err_type = 1;
		} else  {
			target[args->id].position = buf->position;
		}
	}

	if ((res = write(target[args->id].fd, buf->data,
				buf->length)) == buf->length)  {
		target[args->id].position += res;
	} else  {
		error = 2;
	}

	if (error) {
		target[args->id].error = errno;
		target[args->id].position = buf->position;
	}
	return error;
}

static void *
begin_reader(void *arg)
{
	thread_args	*args = arg;

	rcu_register_thread();
	for (;;) {
		pthread_mutex_lock(&args->wait);
		if (do_write(args, NULL))
			goto handle_error;
	        pthread_mutex_lock(&glob_masks.mutex);
		if (--glob_masks.num_working == 0)
			pthread_mutex_unlock(&mainwait);
		pthread_mutex_unlock(&glob_masks.mutex);
	}
	/* NOTREACHED */

handle_error:
	/* error will be logged by primary thread */

	pthread_mutex_lock(&glob_masks.mutex);
	target[args->id].state = INACTIVE;
	if (--glob_masks.num_working == 0)
		pthread_mutex_unlock(&mainwait);
	pthread_mutex_unlock(&glob_masks.mutex);
	rcu_unregister_thread();
	pthread_exit(NULL);
	return NULL;
}

static void
handler(int sig)
{
	pid_t	pid;
	int	status, i;

	pid = wait(&status);

	kids--;

	for (i = 0; i < num_targets; i++)  {
		if (target[i].pid == pid)  {
			if (target[i].state == INACTIVE)  {
				/* thread got an I/O error */

				if (target[i].err_type == 0)  {
					do_warn(
		_("%s:  write error on target %d \"%s\" at offset %lld\n"),
						progname, i, target[i].name,
						target[i].position);
				} else  {
					do_warn(
		_("%s:  lseek error on target %d \"%s\" at offset %lld\n"),
						progname, i, target[i].name,
						target[i].position);
				}

				do_vfatal(target[i].error,
					_("Aborting target %d - reason"), i);

				if (kids == 0)  {
					do_log(
				_("Aborting XFS copy - no more targets.\n"));
					check_errors();
					pthread_exit(NULL);
				}

				signal(SIGCHLD, handler);
				return;
			} else  {
				/* it just croaked it bigtime, log it */

				do_warn(
	_("%s:  thread %d died unexpectedly, target \"%s\" incomplete\n"),
					progname, i, target[i].name);
				do_warn(_("%s:  offset was probably %lld\n"),
					progname, target[i].position);
				do_fatal(target[i].error,
					_("Aborting XFS copy - reason"));
				pthread_exit(NULL);
			}
		}
	}

	/* unknown child -- something very wrong */

	do_warn(_("%s: Unknown child died (should never happen!)\n"), progname);
	die_perror();
	pthread_exit(NULL);
	signal(SIGCHLD, handler);
}

static void
usage(void)
{
	fprintf(stderr,
		_("Usage: %s [-bdV] [-L logfile] source target [target ...]\n"),
		progname);
	exit(1);
}

static void
init_bar(uint64_t source_blocks)
{
	int	i;

	for (i = 0; i < 11; i++)
		barcount[i] = (source_blocks/10)*i;
}

static int
bump_bar(int tenths, uint64_t numblocks)
{
	static char *bar[11] = {
		" 0% ",
		" ... 10% ",
		" ... 20% ",
		" ... 30% ",
		" ... 40% ",
		" ... 50% ",
		" ... 60% ",
		" ... 70% ",
		" ... 80% ",
		" ... 90% ",
		" ... 100%\n\n",
	};

	if (tenths > 10)  {
		printf("%s", bar[10]);
		fflush(stdout);
	} else  {
		while (tenths < 10 && numblocks > barcount[tenths])  {
			printf("%s", bar[tenths]);
			fflush(stdout);
			tenths++;
		}
	}
	return tenths;
}

static xfs_off_t source_position = -1;

static wbuf *
wbuf_init(wbuf *buf, int data_size, int data_align, int min_io_size, int id)
{
	ASSERT(data_size % BBSIZE == 0);
	while ((buf->data = memalign(data_align, data_size)) == NULL) {
		data_size >>= 1;
		if (data_size < min_io_size)
			return NULL;
	}
	ASSERT(min_io_size % BBSIZE == 0);
	buf->data_align = data_align;
	buf->min_io_size = min_io_size;
	buf->size = data_size;
	buf->id = id;
	return buf;
}

static void
read_wbuf(int fd, wbuf *buf, xfs_mount_t *mp)
{
	int		res = 0;
	xfs_off_t	lres = 0;
	xfs_off_t	newpos;
	size_t		diff;

	newpos = rounddown(buf->position, (xfs_off_t) buf->min_io_size);

	if (newpos != buf->position)  {
		diff = buf->position - newpos;
		buf->position = newpos;

		buf->length += diff;
	}

	if (source_position != buf->position)  {
		lres = lseek(fd, buf->position, SEEK_SET);
		if (lres < 0LL)  {
			do_warn(_("%s:  lseek failure at offset %lld\n"),
				progname, source_position);
			die_perror();
		}
		source_position = buf->position;
	}

	ASSERT(source_position % source_sectorsize == 0);

	/* round up length for direct I/O if necessary */

	if (buf->length % buf->min_io_size != 0)
		buf->length = roundup(buf->length, buf->min_io_size);

	if (buf->length > buf->size)  {
		do_warn(_("assert error:  buf->length = %d, buf->size = %d\n"),
			buf->length, buf->size);
		exit(1);
	}

	if ((res = read(fd, buf->data, buf->length)) < 0)  {
		do_warn(_("%s:  read failure at offset %lld\n"),
				progname, source_position);
		die_perror();
	}

	if (res < buf->length &&
	    source_position + res == mp->m_sb.sb_dblocks * source_blocksize)
		res = buf->length;
	else
		ASSERT(res == buf->length);
	source_position += res;
	buf->length = res;
}

static void
read_ag_header(int fd, xfs_agnumber_t agno, wbuf *buf, ag_header_t *ag,
		xfs_mount_t *mp, int blocksize, int sectorsize)
{
	xfs_daddr_t	off;
	int		length;
	xfs_off_t	newpos;
	size_t		diff;

	/* initial settings */

	diff = 0;
	off = XFS_AG_DADDR(mp, agno, XFS_SB_DADDR);
	buf->position = (xfs_off_t) off * (xfs_off_t) BBSIZE;
	length = buf->length = first_agbno * blocksize;
	if (length == 0) {
		do_log(_("ag header buffer invalid!\n"));
		exit(1);
	}

	/* handle alignment stuff */

	newpos = rounddown(buf->position, (xfs_off_t) buf->min_io_size);
	if (newpos != buf->position)  {
		diff = buf->position - newpos;
		buf->position = newpos;
		buf->length += diff;
	}

	/* round up length for direct I/O if necessary */

	if (buf->length % buf->min_io_size != 0)
		buf->length = roundup(buf->length, buf->min_io_size);

	read_wbuf(fd, buf, mp);
	ASSERT(buf->length >= length);

	ag->xfs_sb = (struct xfs_dsb *) (buf->data + diff);
	ASSERT(be32_to_cpu(ag->xfs_sb->sb_magicnum) == XFS_SB_MAGIC);
	ag->xfs_agf = (xfs_agf_t *) (buf->data + diff + sectorsize);
	ASSERT(be32_to_cpu(ag->xfs_agf->agf_magicnum) == XFS_AGF_MAGIC);
	ag->xfs_agi = (xfs_agi_t *) (buf->data + diff + 2 * sectorsize);
	ASSERT(be32_to_cpu(ag->xfs_agi->agi_magicnum) == XFS_AGI_MAGIC);
	ag->xfs_agfl = (struct xfs_agfl *) (buf->data + diff + 3 * sectorsize);
}


static void
write_wbuf(void)
{
	int		i;
	int		badness = 0;

	/* verify target threads */
	for (i = 0; i < num_targets; i++)
		if (target[i].state != INACTIVE)
			glob_masks.num_working++;

	/* release target threads */
	for (i = 0; i < num_targets; i++)
		if (target[i].state != INACTIVE)
			pthread_mutex_unlock(&targ[i].wait);	/* wake up */
		else
			badness++;

	/*
	 * If all the targets are inactive then there won't be any io
	 * threads left to release mainwait.  We're screwed, so bail out.
	 */
	if (badness == num_targets) {
		check_errors();
		exit(1);
	}

	signal_maskfunc(SIGCHLD, SIG_UNBLOCK);
	pthread_mutex_lock(&mainwait);
	signal_maskfunc(SIGCHLD, SIG_BLOCK);
}

static void
sb_update_uuid(
	struct xfs_mount	*mp,
	ag_header_t		*ag_hdr, /* AG hdr to update for this copy */
	thread_args		*tcarg)	 /* Args for this thread, with UUID */
{
	/*
	 * If this filesystem has CRCs, the original UUID is stamped into
	 * all metadata.  If we don't have an existing meta_uuid field in the
	 * the original filesystem and we are changing the UUID in this copy,
	 * we must copy the original sb_uuid to the sb_meta_uuid slot and set
	 * the incompat flag for the feature on this copy.
	 */
	if (xfs_has_crc(mp) && !xfs_has_metauuid(mp) &&
	    !uuid_equal(&tcarg->uuid, &mp->m_sb.sb_uuid)) {
		uint32_t feat;

		feat = be32_to_cpu(ag_hdr->xfs_sb->sb_features_incompat);
		feat |= XFS_SB_FEAT_INCOMPAT_META_UUID;
		ag_hdr->xfs_sb->sb_features_incompat = cpu_to_be32(feat);
		platform_uuid_copy(&ag_hdr->xfs_sb->sb_meta_uuid,
				   &mp->m_sb.sb_uuid);
	}

	/* Copy the (possibly new) fs-identifier UUID into sb_uuid */
	platform_uuid_copy(&ag_hdr->xfs_sb->sb_uuid, &tcarg->uuid);

	/* We may have changed the UUID, so update the superblock CRC */
	if (xfs_has_crc(mp))
		xfs_update_cksum((char *)ag_hdr->xfs_sb, mp->m_sb.sb_sectsize,
				XFS_SB_CRC_OFF);
}

int
main(int argc, char **argv)
{
	int		i, j;
	int		logfd;
	int		howfar = 0;
	int		open_flags;
	xfs_off_t	pos;
	size_t		length;
	int		c;
	uint64_t	size, sizeb;
	uint64_t	numblocks = 0;
	int		wblocks = 0;
	int		num_threads = 0;
	struct dioattr	d;
	int		wbuf_size;
	int		wbuf_align;
	int		wbuf_miniosize;
	int		source_is_file = 0;
	int		buffered_output = 0;
	int		duplicate = 0;
	uint		btree_levels, current_level;
	ag_header_t	ag_hdr;
	xfs_mount_t	*mp;
	xfs_mount_t	mbuf;
	struct xlog	xlog;
	struct xfs_buf	*sbp;
	xfs_sb_t	*sb;
	xfs_agnumber_t	num_ags, agno;
	xfs_agblock_t	bno;
	xfs_daddr_t	begin, next_begin, ag_begin, new_begin, ag_end;
	struct xfs_btree_block *block;
	xfs_alloc_ptr_t	*ptr;
	xfs_alloc_rec_t	*rec_ptr;
	extern char	*optarg;
	extern int	optind;
	libxfs_init_t	xargs;
	thread_args	*tcarg;
	struct stat	statbuf;
	int		error;

	progname = basename(argv[0]);

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	while ((c = getopt(argc, argv, "bdL:V")) != EOF)  {
		switch (c) {
		case 'b':
			buffered_output = 1;
			break;
		case 'd':
			duplicate = 1;
			break;
		case 'L':
			logfile_name = optarg;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		default:
			usage();
		}
	}

	if (argc - optind < 2)
		usage();

	if (logfile_name)  {
		logfd = open(logfile_name, O_CREAT|O_WRONLY|O_EXCL, 0600);
	} else  {
		logfile_name = LOGFILE_NAME;
		logfd = mkstemp(logfile_name);
	}

	if (logfd < 0)  {
		fprintf(stderr, _("%s: couldn't open log file \"%s\"\n"),
			progname, logfile_name);
		perror(_("Aborting XFS copy - reason"));
		exit(1);
	}

	if ((logerr = fdopen(logfd, "w")) == NULL)  {
		fprintf(stderr, _("%s: couldn't set up logfile stream\n"),
			progname);
		perror(_("Aborting XFS copy - reason"));
		exit(1);
	}

	source_name = argv[optind];
	source_fd = -1;
	optind++;

	num_targets = argc - optind;
	if ((target = malloc(sizeof(target_control) * num_targets)) == NULL)  {
		do_log(_("Couldn't allocate target array\n"));
		die_perror();
	}
	for (i = 0; optind < argc; i++, optind++)  {
		target[i].name = argv[optind];
		target[i].fd = -1;
		target[i].position = -1;
		target[i].state = INACTIVE;
		target[i].error = 0;
		target[i].err_type = 0;
	}

	/* open up source -- is it a file? */

	open_flags = O_RDONLY;

	if ((source_fd = open(source_name, open_flags)) < 0)  {
		do_log(_("%s:  couldn't open source \"%s\"\n"),
			progname, source_name);
		die_perror();
	}

	if (fstat(source_fd, &statbuf) < 0)  {
		do_log(_("%s:  couldn't stat source \"%s\"\n"),
			progname, source_name);
		die_perror();
	}

	if (S_ISREG(statbuf.st_mode))
		source_is_file = 1;

	if (source_is_file && platform_test_xfs_fd(source_fd))  {
		if (fcntl(source_fd, F_SETFL, open_flags | O_DIRECT) < 0)  {
			do_log(_("%s: Cannot set direct I/O flag on \"%s\".\n"),
				progname, source_name);
			die_perror();
		}
		if (xfsctl(source_name, source_fd, XFS_IOC_DIOINFO, &d) < 0)  {
			do_log(_("%s: xfsctl on file \"%s\" failed.\n"),
				progname, source_name);
			die_perror();
		}

		wbuf_align = d.d_mem;
		wbuf_size = min(d.d_maxiosz, 1 * 1024 * 1024);
		wbuf_miniosize = d.d_miniosz;
	} else  {
		/* set arbitrary I/O params, miniosize at least 1 disk block */

		wbuf_align = getpagesize();
		wbuf_size = 1 * 1024 * 1024;
		wbuf_miniosize = -1;	/* set after mounting source fs */
	}

	if (!source_is_file)  {
		/*
		 * check to make sure a filesystem isn't mounted
		 * on the device
		 */
		if (platform_check_ismounted(source_name, NULL, &statbuf, 0))  {
			do_log(
	_("%s:  Warning -- a filesystem is mounted on the source device.\n"),
				progname);
			do_log(
	_("\t\tGenerated copies may be corrupt unless the source is\n"));
			do_log(
	_("\t\tunmounted or mounted read-only.  Copy proceeding...\n"));
		}
	}

	/* prepare the libxfs_init structure */

	memset(&xargs, 0, sizeof(xargs));
	xargs.isdirect = LIBXFS_DIRECT;
	xargs.isreadonly = LIBXFS_ISREADONLY;

	if (source_is_file)  {
		xargs.dname = source_name;
		xargs.disfile = 1;
	} else
		xargs.volname = source_name;

	if (!libxfs_init(&xargs))  {
		do_log(_("%s: couldn't initialize XFS library\n"
			"%s: Aborting.\n"), progname, progname);
		exit(1);
	}

	memset(&mbuf, 0, sizeof(xfs_mount_t));

	/* We don't yet know the sector size, so read maximal size */
	libxfs_buftarg_init(&mbuf, xargs.ddev, xargs.logdev, xargs.rtdev);
	error = -libxfs_buf_read_uncached(mbuf.m_ddev_targp, XFS_SB_DADDR,
			1 << (XFS_MAX_SECTORSIZE_LOG - BBSHIFT), 0, &sbp, NULL);
	if (error) {
		do_log(_("%s: couldn't read superblock, error=%d\n"),
				progname, error);
		exit(1);
	}

	sb = &mbuf.m_sb;
	libxfs_sb_from_disk(sb, sbp->b_addr);

	/* Do it again, now with proper length and verifier */
	libxfs_buf_relse(sbp);

	error = -libxfs_buf_read_uncached(mbuf.m_ddev_targp, XFS_SB_DADDR,
			1 << (sb->sb_sectlog - BBSHIFT), 0, &sbp,
			&xfs_sb_buf_ops);
	if (error) {
		do_log(_("%s: couldn't read superblock, error=%d\n"),
				progname, error);
		exit(1);
	}
	libxfs_buf_relse(sbp);

	mp = libxfs_mount(&mbuf, sb, xargs.ddev, xargs.logdev, xargs.rtdev, 0);
	if (mp == NULL) {
		do_log(_("%s: %s filesystem failed to initialize\n"
			"%s: Aborting.\n"), progname, source_name, progname);
		exit(1);
	} else if (mp->m_sb.sb_inprogress)  {
		do_log(_("%s %s filesystem failed to initialize\n"
			"%s: Aborting.\n"), progname, source_name, progname);
		exit(1);
	} else if (mp->m_sb.sb_logstart == 0)  {
		do_log(_("%s: %s has an external log.\n%s: Aborting.\n"),
			progname, source_name, progname);
		exit(1);
	} else if (mp->m_sb.sb_rextents != 0)  {
		do_log(_("%s: %s has a real-time section.\n"
			"%s: Aborting.\n"), progname, source_name, progname);
		exit(1);
	}


	/*
	 * Set up the mount pointer to access the log and check whether the log
	 * is clean. Fail on a dirty or corrupt log in non-duplicate mode
	 * because the log is formatted as part of the copy and we don't want to
	 * destroy data. We also need the current log cycle to format v5
	 * superblock logs correctly.
	 */
	memset(&xlog, 0, sizeof(struct xlog));
	mp->m_log = &xlog;
	c = xlog_is_dirty(mp, mp->m_log, &xargs, 0);
	if (!duplicate) {
		if (c == 1) {
			do_log(_(
"Error: source filesystem log is dirty. Mount the filesystem to replay the\n"
"log, unmount and retry xfs_copy.\n"));
			exit(1);
		} else if (c < 0) {
			do_log(_(
"Error: could not determine the log head or tail of the source filesystem.\n"
"Mount the filesystem to replay the log or run xfs_repair.\n"));
			exit(1);
		}
	}

	source_blocksize = mp->m_sb.sb_blocksize;
	source_sectorsize = mp->m_sb.sb_sectsize;

	if (wbuf_miniosize == -1)
		wbuf_miniosize = source_sectorsize;

	ASSERT(source_blocksize % source_sectorsize == 0);
	ASSERT(source_sectorsize % BBSIZE == 0);

	if (source_blocksize < source_sectorsize)  {
		do_log(_("Error:  filesystem block size is smaller than the"
			" disk sectorsize.\nAborting XFS copy now.\n"));
		exit(1);
	}

	first_agbno = XFS_AGFL_BLOCK(mp) + 1;

	/* now open targets */

	open_flags = O_RDWR;

	for (i = 0; i < num_targets; i++)  {
		int	write_last_block = 0;

		if (stat(target[i].name, &statbuf) < 0)  {
			/* ok, assume it's a file and create it */

			do_out(_("Creating file %s\n"), target[i].name);

			open_flags |= O_CREAT;
			if (!buffered_output)
				open_flags |= O_DIRECT;
			write_last_block = 1;
		} else if (S_ISREG(statbuf.st_mode))  {
			open_flags |= O_TRUNC;
			if (!buffered_output)
				open_flags |= O_DIRECT;
			write_last_block = 1;
		} else  {
			/*
			 * check to make sure a filesystem isn't mounted
			 * on the device
			 */
			if (platform_check_ismounted(target[i].name,
							NULL, &statbuf, 0))  {
				do_log(_("%s:  a filesystem is mounted "
					"on target device \"%s\".\n"
					"%s cannot copy to mounted filesystems."
					"  Aborting\n"),
					progname, target[i].name, progname);
				exit(1);
			}
		}

		target[i].fd = open(target[i].name, open_flags, 0644);
		if (target[i].fd < 0)  {
			do_log(_("%s:  couldn't open target \"%s\"\n"),
				progname, target[i].name);
			die_perror();
		}

		if (write_last_block)  {
			/* ensure regular files are correctly sized */

			if (ftruncate(target[i].fd, mp->m_sb.sb_dblocks *
						source_blocksize))  {
				do_log(_("%s:  cannot grow data section.\n"),
					progname);
				die_perror();
			}
			if (platform_test_xfs_fd(target[i].fd))  {
				if (xfsctl(target[i].name, target[i].fd,
						XFS_IOC_DIOINFO, &d) < 0)  {
					do_log(
				_("%s:  xfsctl on \"%s\" failed.\n"),
						progname, target[i].name);
					die_perror();
				} else {
					wbuf_align = max(wbuf_align, d.d_mem);
					wbuf_size = min(d.d_maxiosz, wbuf_size);
					wbuf_miniosize = max(d.d_miniosz,
								wbuf_miniosize);
				}
			}
		} else  {
			char	*lb[XFS_MAX_SECTORSIZE] = { NULL };
			off64_t	off;

			/* ensure device files are sufficiently large */

			off = mp->m_sb.sb_dblocks * source_blocksize;
			off -= sizeof(lb);
			if (pwrite(target[i].fd, lb, sizeof(lb), off) < 0)  {
				do_log(_("%s:  failed to write last block\n"),
					progname);
				do_log(_("\tIs target \"%s\" too small?\n"),
					target[i].name);
				die_perror();
			}
		}
	}

	/* initialize locks and bufs */

	if (pthread_mutex_init(&glob_masks.mutex, NULL) != 0)  {
		do_log(_("Couldn't initialize global thread mask\n"));
		die_perror();
	}
	glob_masks.num_working = 0;

	if (wbuf_init(&w_buf, wbuf_size, wbuf_align,
					wbuf_miniosize, 0) == NULL)  {
		do_log(_("Error initializing wbuf 0\n"));
		die_perror();
	}

	wblocks = wbuf_size / BBSIZE;

	if (wbuf_init(&btree_buf, max(source_blocksize, wbuf_miniosize),
				wbuf_align, wbuf_miniosize, 1) == NULL)  {
		do_log(_("Error initializing btree buf 1\n"));
		die_perror();
	}

	if (pthread_mutex_init(&mainwait,NULL) != 0)  {
		do_log(_("Error creating first semaphore.\n"));
		die_perror();
		exit(1);
	}
	/* need to start out blocking */
	pthread_mutex_lock(&mainwait);

	/* set up sigchild signal handler */

	signal(SIGCHLD, handler);
	signal_maskfunc(SIGCHLD, SIG_BLOCK);

	/* make children */

	if ((targ = malloc(num_targets * sizeof(thread_args))) == NULL)  {
		do_log(_("Couldn't malloc space for thread args\n"));
		die_perror();
		exit(1);
	}

	for (i = 0, tcarg = targ; i < num_targets; i++, tcarg++)  {
		if (!duplicate)
			platform_uuid_generate(&tcarg->uuid);
		else
			platform_uuid_copy(&tcarg->uuid, &mp->m_sb.sb_uuid);

		if (pthread_mutex_init(&tcarg->wait, NULL) != 0)  {
			do_log(_("Error creating thread mutex %d\n"), i);
			die_perror();
			exit(1);
		}
		/* need to start out blocking */
		pthread_mutex_lock(&tcarg->wait);
	}

	for (i = 0, tcarg = targ; i < num_targets; i++, tcarg++)  {
		tcarg->id = i;
		tcarg->fd = target[i].fd;

		target[i].state = ACTIVE;
		num_threads++;

		if (pthread_create(&target[i].pid, NULL,
					begin_reader, (void *)tcarg))  {
			do_log(_("Error creating thread for target %d\n"), i);
			die_perror();
		}
	}

	ASSERT(num_targets == num_threads);

	/* set up statistics */

	num_ags = mp->m_sb.sb_agcount;

	init_bar(mp->m_sb.sb_blocksize / BBSIZE
			* ((uint64_t)mp->m_sb.sb_dblocks
			    - (uint64_t)mp->m_sb.sb_fdblocks + 10 * num_ags));

	kids = num_targets;

	for (agno = 0; agno < num_ags && kids > 0; agno++)  {
		/* read in first blocks of the ag */

		read_ag_header(source_fd, agno, &w_buf, &ag_hdr, mp,
			source_blocksize, source_sectorsize);

		/* set the in_progress bit for the first AG */

		if (agno == 0)
			ag_hdr.xfs_sb->sb_inprogress = 1;

		/* save what we need (agf) in the btree buffer */

		memmove(btree_buf.data, ag_hdr.xfs_agf, source_sectorsize);
		ag_hdr.xfs_agf = (xfs_agf_t *) btree_buf.data;
		btree_buf.length = source_blocksize;

		/* write the ag header out */

		write_wbuf();

		/* traverse btree until we get to the leftmost leaf node */

		bno = be32_to_cpu(ag_hdr.xfs_agf->agf_roots[XFS_BTNUM_BNOi]);
		current_level = 0;
		btree_levels = be32_to_cpu(ag_hdr.xfs_agf->
						agf_levels[XFS_BTNUM_BNOi]);

		ag_end = XFS_AGB_TO_DADDR(mp, agno,
				be32_to_cpu(ag_hdr.xfs_agf->agf_length) - 1)
				+ source_blocksize / BBSIZE;

		for (;;) {
			/* none of this touches the w_buf buffer */

			if (current_level >= btree_levels) {
				do_log(
			_("Error: current level %d >= btree levels %d\n"),
					current_level, btree_levels);
				exit(1);
			}

			current_level++;

			btree_buf.position = pos = (xfs_off_t)
				XFS_AGB_TO_DADDR(mp,agno,bno) << BBSHIFT;
			btree_buf.length = source_blocksize;

			read_wbuf(source_fd, &btree_buf, mp);
			block = (struct xfs_btree_block *)
				 ((char *)btree_buf.data +
				  pos - btree_buf.position);

			if (be32_to_cpu(block->bb_magic) !=
			    (xfs_has_crc(mp) ?
			     XFS_ABTB_CRC_MAGIC : XFS_ABTB_MAGIC)) {
				do_log(_("Bad btree magic 0x%x\n"),
				        be32_to_cpu(block->bb_magic));
				exit(1);
			}

			if (be16_to_cpu(block->bb_level) == 0)
				break;

			ptr = XFS_ALLOC_PTR_ADDR(mp, block, 1,
							mp->m_alloc_mxr[1]);
			bno = be32_to_cpu(ptr[0]);
		}

		/* align first data copy but don't overwrite ag header */

		pos = w_buf.position >> BBSHIFT;
		length = w_buf.length >> BBSHIFT;
		next_begin = pos + length;
		ag_begin = next_begin;

		ASSERT(w_buf.position % source_sectorsize == 0);

		/* handle the rest of the ag */

		for (;;) {
			if (be16_to_cpu(block->bb_level) != 0)  {
				do_log(
			_("WARNING:  source filesystem inconsistent.\n"));
				do_log(
			_("  A leaf btree rec isn't a leaf.  Aborting now.\n"));
				exit(1);
			}

			rec_ptr = XFS_ALLOC_REC_ADDR(mp, block, 1);
			for (i = 0; i < be16_to_cpu(block->bb_numrecs);
							i++, rec_ptr++)  {
				/* calculate in daddr's */

				begin = next_begin;

				/*
				 * protect against pathological case of a
				 * hole right after the ag header in a
				 * mis-aligned case
				 */

				if (begin < ag_begin)
					begin = ag_begin;

				/*
				 * round size up to ensure we copy a
				 * range bigger than required
				 */

				sizeb = XFS_AGB_TO_DADDR(mp, agno,
					be32_to_cpu(rec_ptr->ar_startblock)) -
						begin;
				size = roundup(sizeb <<BBSHIFT, wbuf_miniosize);
				if (size > 0)  {
					/* copy extent */

					w_buf.position = (xfs_off_t)
						begin << BBSHIFT;

					while (size > 0)  {
						/*
						 * let lower layer do alignment
						 */
						if (size > w_buf.size)  {
							w_buf.length = w_buf.size;
							size -= w_buf.size;
							sizeb -= wblocks;
							numblocks += wblocks;
						} else  {
							w_buf.length = size;
							numblocks += sizeb;
							size = 0;
						}

						read_wbuf(source_fd, &w_buf, mp);
						write_wbuf();

						w_buf.position += w_buf.length;

						howfar = bump_bar(
							howfar, numblocks);
					}
				}

				/* round next starting point down */

				new_begin = XFS_AGB_TO_DADDR(mp, agno,
						be32_to_cpu(rec_ptr->ar_startblock) +
					 	be32_to_cpu(rec_ptr->ar_blockcount));
				next_begin = rounddown(new_begin,
						w_buf.min_io_size >> BBSHIFT);
			}

			if (be32_to_cpu(block->bb_u.s.bb_rightsib) == NULLAGBLOCK)
				break;

			/* read in next btree record block */

			btree_buf.position = pos = (xfs_off_t)
				XFS_AGB_TO_DADDR(mp, agno, be32_to_cpu(
						block->bb_u.s.bb_rightsib)) << BBSHIFT;
			btree_buf.length = source_blocksize;

			/* let read_wbuf handle alignment */

			read_wbuf(source_fd, &btree_buf, mp);

			block = (struct xfs_btree_block *)
				 ((char *) btree_buf.data +
				  pos - btree_buf.position);

			ASSERT(be32_to_cpu(block->bb_magic) == XFS_ABTB_MAGIC ||
			       be32_to_cpu(block->bb_magic) == XFS_ABTB_CRC_MAGIC);
		}

		/*
		 * write out range of used blocks after last range
		 * of free blocks in AG
		 */
		if (next_begin < ag_end)  {
			begin = next_begin;

			sizeb = ag_end - begin;
			size = roundup(sizeb << BBSHIFT, wbuf_miniosize);

			if (size > 0)  {
				/* copy extent */

				w_buf.position = (xfs_off_t) begin << BBSHIFT;

				while (size > 0)  {
					/*
					 * let lower layer do alignment
					 */
					if (size > w_buf.size)  {
						w_buf.length = w_buf.size;
						size -= w_buf.size;
						sizeb -= wblocks;
						numblocks += wblocks;
					} else  {
						w_buf.length = size;
						numblocks += sizeb;
						size = 0;
					}

					read_wbuf(source_fd, &w_buf, mp);
					write_wbuf();

					w_buf.position += w_buf.length;

					howfar = bump_bar(howfar, numblocks);
				}
			}
		}
	}

	if (kids > 0)  {
		if (!duplicate)
			/* write a clean log using the specified UUID */
			format_logs(mp);
		else
			num_ags = 1;

		/* reread and rewrite superblocks (UUID and in-progress) */
		/* [backwards, so inprogress bit only updated when done] */

		for (i = num_ags - 1; i >= 0; i--)  {
			read_ag_header(source_fd, i, &w_buf, &ag_hdr, mp,
				source_blocksize, source_sectorsize);
			if (i == 0)
				ag_hdr.xfs_sb->sb_inprogress = 0;

			/* do each thread in turn, each has its own UUID */

			for (j = 0, tcarg = targ; j < num_targets; j++)  {
				sb_update_uuid(mp, &ag_hdr, tcarg);
				do_write(tcarg, NULL);
				tcarg++;
			}
		}

		bump_bar(100, 0);
	}

	check_errors();
	libxfs_umount(mp);
	libxfs_destroy(&xargs);

	return 0;
}

static char *
next_log_chunk(char *p, int offset, void *private)
{
	wbuf	*buf = (wbuf *)private;

	if (buf->length < (int)(p - buf->data) + offset) {
		/* need to flush this one, then start afresh */

		do_write(buf->owner, NULL);
		memset(buf->data, 0, buf->length);
		return buf->data;
	}
	return p + offset;
}

/*
 * Writes a log header at the start of the log (with the real
 * filesystem UUID embedded into it), and writes to all targets.
 *
 * Returns the next buffer-length-aligned disk address.
 */
xfs_off_t
write_log_header(int fd, wbuf *buf, xfs_mount_t *mp)
{
	char		*p = buf->data;
	xfs_off_t	logstart;
	int		offset;

	logstart = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart) << BBSHIFT;
	buf->position = rounddown(logstart, (xfs_off_t)buf->length);

	memset(p, 0, buf->size);
	if (logstart % buf->length)  {	/* unaligned */
		read_wbuf(fd, buf, mp);
		offset = logstart - buf->position;
		p += offset;
		memset(p, 0, buf->length - offset);
	}

	offset = libxfs_log_header(p, &buf->owner->uuid,
			xfs_has_logv2(mp) ? 2 : 1,
			mp->m_sb.sb_logsunit, XLOG_FMT, NULLCOMMITLSN,
			NULLCOMMITLSN, next_log_chunk, buf);
	do_write(buf->owner, NULL);

	return roundup(logstart + offset, buf->length);
}

/*
 * May do an aligned read of the last buffer in the log (& zero
 * the start of that buffer).  Returns the disk address at the
 * end of last aligned buffer in the log.
 */
xfs_off_t
write_log_trailer(int fd, wbuf *buf, xfs_mount_t *mp)
{
	xfs_off_t	logend;
	int		offset;

	logend = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart) << BBSHIFT;
	logend += XFS_FSB_TO_B(mp, mp->m_sb.sb_logblocks);

	buf->position = rounddown(logend, (xfs_off_t)buf->length);

	if (logend % buf->length)  {	/* unaligned */
		read_wbuf(fd, buf, mp);
		offset = (int)(logend - buf->position);
		memset(buf->data, 0, offset);
		do_write(buf->owner, NULL);
	}

	return buf->position;
}

/*
 * Clear a log by writing a record at the head, the tail and zeroing everything
 * in between.
 */
static void
clear_log(
	struct xfs_mount	*mp,
	thread_args		*tcarg)
{
	xfs_off_t		pos;
	xfs_off_t		end_pos;

	w_buf.owner = tcarg;
	w_buf.length = rounddown(w_buf.size, w_buf.min_io_size);
	pos = write_log_header(source_fd, &w_buf, mp);
	end_pos = write_log_trailer(source_fd, &w_buf, mp);
	w_buf.position = pos;
	memset(w_buf.data, 0, w_buf.length);

	while (w_buf.position < end_pos)  {
		do_write(tcarg, NULL);
		w_buf.position += w_buf.length;
	}
}

/*
 * Format the log to a particular cycle number. This is required for version 5
 * superblock filesystems to provide metadata LSN validity guarantees.
 */
static void
format_log(
	struct xfs_mount	*mp,
	thread_args		*tcarg,
	wbuf			*buf)
{
	int			logstart;
	int			length;
	int			cycle = XLOG_INIT_CYCLE;

	buf->owner = tcarg;
	buf->length = buf->size;
	buf->position = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart) << BBSHIFT;

	logstart = XFS_FSB_TO_BB(mp, mp->m_sb.sb_logstart);
	length = XFS_FSB_TO_BB(mp, mp->m_sb.sb_logblocks);

	/*
	 * Bump the cycle number on v5 superblock filesystems to guarantee that
	 * all existing metadata LSNs are valid (behind the current LSN) on the
	 * target fs.
	 */
	if (xfs_has_crc(mp))
		cycle = mp->m_log->l_curr_cycle + 1;

	/*
	 * Format the entire log into the memory buffer and write it out. If the
	 * write fails, mark the target inactive so the failure is reported.
	 */
	libxfs_log_clear(NULL, buf->data, logstart, length, &buf->owner->uuid,
			 xfs_has_logv2(mp) ? 2 : 1,
			 mp->m_sb.sb_logsunit, XLOG_FMT, cycle, true);
	if (do_write(buf->owner, buf))
		target[tcarg->id].state = INACTIVE;
}

static int
format_logs(
	struct xfs_mount	*mp)
{
	thread_args		*tcarg;
	int			i;
	wbuf			logbuf;
	int			logsize;

	if (xfs_has_crc(mp)) {
		logsize = XFS_FSB_TO_B(mp, mp->m_sb.sb_logblocks);
		if (!wbuf_init(&logbuf, logsize, w_buf.data_align,
			       w_buf.min_io_size, w_buf.id))
			return -ENOMEM;
	}

	for (i = 0, tcarg = targ; i < num_targets; i++)  {
		if (xfs_has_crc(mp))
			format_log(mp, tcarg, &logbuf);
		else
			clear_log(mp, tcarg);
		tcarg++;
	}

	if (xfs_has_crc(mp))
		free(logbuf.data);

	return 0;
}
