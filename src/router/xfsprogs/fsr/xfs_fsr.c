/*
 * Copyright (c) 2000-2002 Silicon Graphics, Inc.
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

#include <xfs/xfs.h>
#include <libxfs.h>
#include <xfs/xfs_types.h>
#include <xfs/jdm.h>
#include <xfs/xfs_dfrag.h>
#include <xfs/xfs_bmap_btree.h>
#include <xfs/xfs_dinode.h>
#include <xfs/xfs_attr_sf.h>

#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <mntent.h>
#include <syslog.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>
#ifndef __UCLIBC__
#include <sys/xattr.h>
#endif


#ifndef XFS_XFLAG_NODEFRAG
#define XFS_XFLAG_NODEFRAG 0x00002000 /* src dependancy, remove later */
#endif

#define _PATH_FSRLAST		"/var/tmp/.fsrlast_xfs"
#define _PATH_PROC_MOUNTS	"/proc/mounts"


char *progname;

int vflag;
int gflag;
static int Mflag;
/* static int nflag; */
int dflag = 0;
/* static int sflag; */
int argv_blksz_dio;
extern int max_ext_size;
static int npasses = 10;
static int startpass = 0;

struct getbmap  *outmap = NULL;
int             outmap_size = 0;
int		RealUid;
int		tmp_agi;
static __int64_t	minimumfree = 2048;

#define MNTTYPE_XFS             "xfs"

#define SMBUFSZ		1024
#define ROOT		0
#define NULLFD		-1
#define GRABSZ		64
#define TARGETRANGE	10
#define	V_NONE		0
#define	V_OVERVIEW	1
#define	V_ALL		2
#define BUFFER_SIZE	(1<<16)
#define BUFFER_MAX	(1<<24)
#define min(x, y) ((x) < (y) ? (x) : (y))

static time_t howlong = 7200;		/* default seconds of reorganizing */
static char *leftofffile = _PATH_FSRLAST; /* where we left off last */
static time_t endtime;
static time_t starttime;
static xfs_ino_t	leftoffino = 0;
static int	pagesize;

void usage(int ret);
static int  fsrfile(char *fname, xfs_ino_t ino);
static int  fsrfile_common( char *fname, char *tname, char *mnt,
                            int fd, xfs_bstat_t *statp);
static int  packfile(char *fname, char *tname, int fd,
                     xfs_bstat_t *statp, struct fsxattr *fsxp);
static void fsrdir(char *dirname);
static int  fsrfs(char *mntdir, xfs_ino_t ino, int targetrange);
static void initallfs(char *mtab);
static void fsrallfs(char *mtab, int howlong, char *leftofffile);
static void fsrall_cleanup(int timeout);
static int  getnextents(int);
int xfsrtextsize(int fd);
int xfs_getrt(int fd, struct statvfs64 *sfbp);
char * gettmpname(char *fname);
char * getparent(char *fname);
int fsrprintf(const char *fmt, ...);
int read_fd_bmap(int, xfs_bstat_t *, int *);
int cmp(const void *, const void *);
static void tmp_init(char *mnt);
static char * tmp_next(char *mnt);
static void tmp_close(char *mnt);
int xfs_getgeom(int , xfs_fsop_geom_v1_t * );
static int getmntany(FILE *, struct mntent *, struct mntent *, struct stat64 *);

xfs_fsop_geom_v1_t fsgeom;	/* geometry of active mounted system */

#define NMOUNT 64
static int numfs;

typedef struct fsdesc {
	char *dev;
	char *mnt;
	int  npass;
} fsdesc_t;

fsdesc_t	*fs, *fsbase, *fsend;
int		fsbufsize = 10;	/* A starting value */
int		nfrags = 0;	/* Debug option: Coerse into specific number
				 * of extents */
int		openopts = O_CREAT|O_EXCL|O_RDWR|O_DIRECT;

int
xfs_fsgeometry(int fd, xfs_fsop_geom_v1_t *geom)
{
    return ioctl(fd, XFS_IOC_FSGEOMETRY_V1, geom);
}

int
xfs_bulkstat_single(int fd, xfs_ino_t *lastip, xfs_bstat_t *ubuffer)
{
    xfs_fsop_bulkreq_t  bulkreq;

    bulkreq.lastip = (__u64 *)lastip;
    bulkreq.icount = 1;
    bulkreq.ubuffer = ubuffer;
    bulkreq.ocount = NULL;
    return ioctl(fd, XFS_IOC_FSBULKSTAT_SINGLE, &bulkreq);
}

int
xfs_bulkstat(int fd, xfs_ino_t *lastip, int icount,
                    xfs_bstat_t *ubuffer, __s32 *ocount)
{
    xfs_fsop_bulkreq_t  bulkreq;

    bulkreq.lastip = (__u64 *)lastip;
    bulkreq.icount = icount;
    bulkreq.ubuffer = ubuffer;
    bulkreq.ocount = ocount;
    return ioctl(fd, XFS_IOC_FSBULKSTAT, &bulkreq);
}

int
xfs_swapext(int fd, xfs_swapext_t *sx)
{
    return ioctl(fd, XFS_IOC_SWAPEXT, sx);
}

int
xfs_fscounts(int fd, xfs_fsop_counts_t *counts)
{
    return ioctl(fd, XFS_IOC_FSCOUNTS, counts);
}

void
aborter(int unused)
{
	fsrall_cleanup(1);
	exit(1);
}

int
main(int argc, char **argv)
{
	struct stat64 sb, sb2;
	char *argname;
	char *cp;
	int c;
	struct mntent mntpref;
	register struct mntent *mntp;
	struct mntent ment;
	char *mtab = NULL;
	register FILE *mtabp;

	setlinebuf(stdout);
	progname = basename(argv[0]);

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	gflag = ! isatty(0);

	while ((c = getopt(argc, argv, "C:p:e:MgsdnvTt:f:m:b:N:FV")) != -1) {
		switch (c) {
		case 'M':
			Mflag = 1;
			break;
		case 'g':
			gflag = 1;
			break;
		case 'n':
			/* nflag = 1; */
			break;
		case 'v':
			++vflag;
			break;
		case 'd':
			dflag = 1;
			break;
		case 's':		/* frag stats only */
			/* sflag = 1; */
			fprintf(stderr,
				_("%s: Stats not yet supported for XFS\n"),
				progname);
			usage(1);
			break;
		case 't':
			howlong = atoi(optarg);
			break;
		case 'f':
			leftofffile = optarg;
			break;
		case 'm':
			mtab = optarg;
			break;
		case 'b':
			argv_blksz_dio = atoi(optarg);
			break;
		case 'p':
			npasses = atoi(optarg);
			break;
		case 'C':
			/* Testing opt: coerses frag count in result */
			if (getenv("FSRXFSTEST") != NULL) {
				nfrags = atoi(optarg);
				openopts |= O_SYNC;
			}
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		default:
			usage(1);
		}
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

	if (vflag)
		setbuf(stdout, NULL);

	starttime = time(0);

	/* Save the caller's real uid */
	RealUid = getuid();

	pagesize = getpagesize();

	if (optind < argc) {
		for (; optind < argc; optind++) {
			argname = argv[optind];
			mntp = NULL;
			if (lstat64(argname, &sb) < 0) {
				fprintf(stderr,
					_("%s: could not stat: %s: %s\n"),
					progname, argname, strerror(errno));
				continue;
			}
			if (S_ISLNK(sb.st_mode) && stat64(argname, &sb2) == 0 &&
			    (S_ISBLK(sb2.st_mode) || S_ISCHR(sb2.st_mode)))
				sb = sb2;
			if (S_ISBLK(sb.st_mode) || (S_ISDIR(sb.st_mode))) {
				if ((mtabp = setmntent(mtab, "r")) == NULL) {
					fprintf(stderr,
						_("%s: cannot read %s\n"),
						progname, mtab);
					exit(1);
				}
				bzero(&mntpref, sizeof(mntpref));
				if (S_ISDIR(sb.st_mode))
					mntpref.mnt_dir = argname;
				else
					mntpref.mnt_fsname = argname;

				if (getmntany(mtabp, &ment, &mntpref, &sb) &&
				    strcmp(ment.mnt_type, MNTTYPE_XFS) == 0) {
					mntp = &ment;
					if (S_ISBLK(sb.st_mode)) {
						cp = mntp->mnt_dir;
						if (cp == NULL ||
						    stat64(cp, &sb2) < 0) {
							fprintf(stderr, _(
						"%s: could not stat: %s: %s\n"),
							progname, argname,
							strerror(errno));
							continue;
						}
						sb = sb2;
						argname = cp;
					}
				}
			}
			if (mntp != NULL) {
				fsrfs(mntp->mnt_dir, 0, 100);
			} else if (S_ISCHR(sb.st_mode)) {
				fprintf(stderr, _(
					"%s: char special not supported: %s\n"),
				        progname, argname);
				exit(1);
			} else if (S_ISDIR(sb.st_mode) || S_ISREG(sb.st_mode)) {
				if (!platform_test_xfs_path(argname)) {
					fprintf(stderr, _(
				        "%s: cannot defragment: %s: Not XFS\n"),
				        progname, argname);
					continue;
				}
				if (S_ISDIR(sb.st_mode))
					fsrdir(argname);
				else
					fsrfile(argname, sb.st_ino);
			} else {
				printf(
			_("%s: not fsys dev, dir, or reg file, ignoring\n"),
					argname);
			}
		}
	} else {
		initallfs(mtab);
		fsrallfs(mtab, howlong, leftofffile);
	}
	return 0;
}

void
usage(int ret)
{
	fprintf(stderr, _(
"Usage: %s [-d] [-v] [-n] [-s] [-g] [-t time] [-p passes] [-f leftf] [-m mtab]\n"
"       %s [-d] [-v] [-n] [-s] [-g] xfsdev | dir | file ...\n\n"
"Options:\n"
"       -n              Do nothing, only interesting with -v. Not\n"
"                       effective with in mtab mode.\n"
"       -s		Print statistics only.\n"
"       -g              Print to syslog (default if stdout not a tty).\n"
"       -t time         How long to run in seconds.\n"
"       -p passes	Number of passes before terminating global re-org.\n"
"       -f leftoff      Use this instead of %s.\n"
"       -m mtab         Use something other than /etc/mtab.\n"
"       -d              Debug, print even more.\n"
"       -v		Verbose, more -v's more verbose.\n"
		), progname, progname, _PATH_FSRLAST);
	exit(ret);
}

/*
 * initallfs -- read the mount table and set up an internal form
 */
static void
initallfs(char *mtab)
{
	FILE *fp;
	struct mntent *mp;
	int mi;
	char *cp;
	struct stat64 sb;

	fp = setmntent(mtab, "r");
	if (fp == NULL) {
		fsrprintf(_("could not open mtab file: %s\n"), mtab);
		exit(1);
	}

	/* malloc a number of descriptors, increased later if needed */
	if (!(fsbase = (fsdesc_t *)malloc(fsbufsize * sizeof(fsdesc_t)))) {
		fsrprintf(_("out of memory: %s\n"), strerror(errno));
		exit(1);
	}
	fsend = (fsbase + fsbufsize - 1);

	/* find all rw xfs file systems */
	mi = 0;
	fs = fsbase;
	while ((mp = getmntent(fp))) {
		int rw = 0;

		if (strcmp(mp->mnt_type, MNTTYPE_XFS ) != 0 ||
		    stat64(mp->mnt_fsname, &sb) == -1 ||
		    !S_ISBLK(sb.st_mode))
			continue;

		cp = strtok(mp->mnt_opts,",");
		do {
			if (strcmp("rw", cp) == 0)
				rw++;
		} while ((cp = strtok(NULL, ",")) != NULL);
		if (rw == 0) {
			if (dflag)
				fsrprintf(_("Skipping %s: not mounted rw\n"),
					mp->mnt_fsname);
			continue;
		}

		if (mi == fsbufsize) {
			fsbufsize += NMOUNT;
			if ((fsbase = (fsdesc_t *)realloc((char *)fsbase,
			              fsbufsize * sizeof(fsdesc_t))) == NULL) {
				fsrprintf(_("out of memory: %s\n"),
					strerror(errno));
				exit(1);
			}
			if (!fsbase) {
				fsrprintf(_("out of memory on realloc: %s\n"),
				          strerror(errno));
				exit(1);
			}
			fs = (fsbase + mi);  /* Needed ? */
		}

		fs->dev = strdup(mp->mnt_fsname);
		fs->mnt = strdup(mp->mnt_dir);

		if (fs->mnt == NULL || fs->mnt == NULL) {
			fsrprintf(_("strdup(%s) failed\n"), mp->mnt_fsname);
			exit(1);
		}
		mi++;
		fs++;
	}
	numfs = mi;
	fsend = (fsbase + numfs);
	endmntent(fp);
	if (numfs == 0) {
		fsrprintf(_("no rw xfs file systems in mtab: %s\n"), mtab);
		exit(0);
	}
	if (vflag || dflag) {
		fsrprintf(_("Found %d mounted, writable, XFS filesystems\n"),
		           numfs);
		if (dflag)
			for (fs = fsbase; fs < fsend; fs++)
			    fsrprintf("\t%-30.30s%-30.30s\n", fs->dev, fs->mnt);
	}
}

static void
fsrallfs(char *mtab, int howlong, char *leftofffile)
{
	int fd;
	int error;
	int found = 0;
	char *fsname;
	char buf[SMBUFSZ];
	int mdonly = Mflag;
	char *ptr;
	xfs_ino_t startino = 0;
	fsdesc_t *fsp;
	struct stat64 sb, sb2;

	fsrprintf("xfs_fsr -m %s -t %d -f %s ...\n", mtab, howlong, leftofffile);

	endtime = starttime + howlong;
	fs = fsbase;

	/* where'd we leave off last time? */
	if (lstat64(leftofffile, &sb) == 0) {
		if ( (fd = open(leftofffile, O_RDONLY)) == -1 ) {
			fsrprintf(_("%s: open failed\n"), leftofffile);
		}
		else if ( fstat64(fd, &sb2) == 0) {
			/*
			 * Verify that lstat & fstat point to the
			 * same regular file (no links/no quick spoofs)
			 */
			if ( (sb.st_dev  != sb2.st_dev) ||
			     (sb.st_ino  != sb2.st_ino) ||
			     ((sb.st_mode & S_IFMT) != S_IFREG) ||
			     ((sb2.st_mode & S_IFMT) != S_IFREG) ||
			     (sb2.st_uid  != ROOT) ||
			     (sb2.st_nlink != 1)
			   )
			{
				fsrprintf(_("Can't use %s: mode=0%o own=%d"
					" nlink=%d\n"),
					leftofffile, sb.st_mode,
					sb.st_uid, sb.st_nlink);
				close(fd);
				fd = NULLFD;
			}
		}
		else {
			close(fd);
			fd = NULLFD;
		}
	}
	else {
		fd = NULLFD;
	}

	if (fd != NULLFD) {
		if (read(fd, buf, SMBUFSZ) == -1) {
			fs = fsbase;
			fsrprintf(_("could not read %s, starting with %s\n"),
				leftofffile, *fs->dev);
		} else {
			for (fs = fsbase; fs < fsend; fs++) {
				fsname = fs->dev;
				if ((strncmp(buf,fsname,strlen(fsname)) == 0)
				    && buf[strlen(fsname)] == ' ') {
					found = 1;
					break;
				}
			}
			if (! found)
				fs = fsbase;

			ptr = strchr(buf, ' ');
			if (ptr) {
				startpass = atoi(++ptr);
				ptr = strchr(ptr, ' ');
				if (ptr) {
					startino = strtoull(++ptr, NULL, 10);
				}
			}
			if (startpass < 0)
				startpass = 0;

			/* Init pass counts */
			for (fsp = fsbase; fsp < fs; fsp++) {
				fsp->npass = startpass + 1;
			}
			for (fsp = fs; fsp <= fsend; fsp++) {
				fsp->npass = startpass;
			}
		}
		close(fd);
	}

	if (vflag) {
		fsrprintf(_("START: pass=%d ino=%llu %s %s\n"),
			  fs->npass, (unsigned long long)startino,
			  fs->dev, fs->mnt);
	}

	signal(SIGABRT, aborter);
	signal(SIGHUP, aborter);
	signal(SIGINT, aborter);
	signal(SIGQUIT, aborter);
	signal(SIGTERM, aborter);

	/* reorg for 'howlong' -- checked in 'fsrfs' */
	while (endtime > time(0)) {
		pid_t pid;
		if (fs == fsend)
			fs = fsbase;
		if (fs->npass == npasses) {
			fsrprintf(_("Completed all %d passes\n"), npasses);
			break;
		}
		if (npasses > 1 && !fs->npass)
			Mflag = 1;
		else
			Mflag = mdonly;
		pid = fork();
		switch(pid) {
		case -1:
			fsrprintf(_("couldn't fork sub process:"));
			exit(1);
			break;
		case 0:
			error = fsrfs(fs->mnt, startino, TARGETRANGE);
			exit (error);
			break;
		default:
			wait(&error);
			close(fd);
			if (WIFEXITED(error) && WEXITSTATUS(error) == 1) {
				/* child timed out & did fsrall_cleanup */
				exit(0);
			}
			break;
		}
		startino = 0;  /* reset after the first time through */
		fs->npass++;
		fs++;
	}
	fsrall_cleanup(endtime <= time(0));
}

/*
 * fsrall_cleanup -- close files, print next starting location, etc.
 */
static void
fsrall_cleanup(int timeout)
{
	int fd;
	int ret;
	char buf[SMBUFSZ];

	/* record where we left off */
	unlink(leftofffile);
	fd = open(leftofffile, O_WRONLY|O_CREAT|O_EXCL, 0644);
	if (fd == -1)
		fsrprintf(_("open(%s) failed: %s\n"),
		          leftofffile, strerror(errno));
	else {
		if (timeout) {
			ret = sprintf(buf, "%s %d %llu\n", fs->dev,
			        fs->npass, (unsigned long long)leftoffino);
			if (write(fd, buf, ret) < strlen(buf))
				fsrprintf(_("write(%s) failed: %s\n"),
					leftofffile, strerror(errno));
			close(fd);
		}
	}

	if (timeout)
		fsrprintf(_("%s startpass %d, endpass %d, time %d seconds\n"),
			progname, startpass, fs->npass,
			time(0) - endtime + howlong);
}

/*
 * fsrfs -- reorganize a file system
 */
static int
fsrfs(char *mntdir, xfs_ino_t startino, int targetrange)
{

	int	fsfd, fd;
	int	count = 0;
	int	ret;
	__s32	buflenout;
	xfs_bstat_t buf[GRABSZ];
	char	fname[64];
	char	*tname;
	jdm_fshandle_t	*fshandlep;
	xfs_ino_t	lastino = startino;

	fsrprintf(_("%s start inode=%llu\n"), mntdir,
		(unsigned long long)startino);

	fshandlep = jdm_getfshandle( mntdir );
	if ( ! fshandlep ) {
		fsrprintf(_("unable to get handle: %s: %s\n"),
		          mntdir, strerror( errno ));
		return -1;
	}

	if ((fsfd = open(mntdir, O_RDONLY)) < 0) {
		fsrprintf(_("unable to open: %s: %s\n"),
		          mntdir, strerror( errno ));
		return -1;
	}

	if (xfs_getgeom(fsfd, &fsgeom) < 0 ) {
		fsrprintf(_("Skipping %s: could not get XFS geometry\n"),
			  mntdir);
		return -1;
	}

	tmp_init(mntdir);

	while ((ret = xfs_bulkstat(fsfd,
				&lastino, GRABSZ, &buf[0], &buflenout) == 0)) {
		xfs_bstat_t *p;
		xfs_bstat_t *endp;

		if (buflenout == 0)
			goto out0;

		/* Each loop through, defrag targetrange percent of the files */
		count = (buflenout * targetrange) / 100;

		qsort((char *)buf, buflenout, sizeof(struct xfs_bstat), cmp);

		for (p = buf, endp = (buf + buflenout); p < endp ; p++) {
			/* Do some obvious checks now */
			if (((p->bs_mode & S_IFMT) != S_IFREG) ||
			     (p->bs_extents < 2))
				continue;

			if ((fd = jdm_open(fshandlep, p, O_RDWR)) < 0) {
				/* This probably means the file was
				 * removed while in progress of handling
				 * it.  Just quietly ignore this file.
				 */
				if (dflag)
					fsrprintf(_("could not open: "
						"inode %llu\n"), p->bs_ino);
				continue;
			}

			/* Don't know the pathname, so make up something */
			sprintf(fname, "ino=%lld", (long long)p->bs_ino);

			/* Get a tmp file name */
			tname = tmp_next(mntdir);

			ret = fsrfile_common(fname, tname, mntdir, fd, p);

			leftoffino = p->bs_ino;

			close(fd);

			if (ret == 0) {
				if (--count <= 0)
					break;
			}
		}
		if (endtime && endtime < time(0)) {
			tmp_close(mntdir);
			close(fsfd);
			fsrall_cleanup(1);
			exit(1);
		}
	}
	if (ret < 0)
		fsrprintf(_("%s: xfs_bulkstat: %s\n"), progname, strerror(errno));
out0:
	tmp_close(mntdir);
	close(fsfd);
	return 0;
}

/*
 * To compare bstat structs for qsort.
 */
int
cmp(const void *s1, const void *s2)
{
	return( ((xfs_bstat_t *)s2)->bs_extents -
	        ((xfs_bstat_t *)s1)->bs_extents);

}

/*
 * reorganize by directory hierarchy.
 * Stay in dev (a restriction based on structure of this program -- either
 * call efs_{n,u}mount() around each file, something smarter or this)
 */
static void
fsrdir(char *dirname)
{
	fsrprintf(_("%s: Directory defragmentation not supported\n"), dirname);
}

/*
 * Sets up the defragmentation of a file based on the
 * filepath.  It collects the bstat information, does
 * an open on the file and passes this all to fsrfile_common.
 */
static int
fsrfile(char *fname, xfs_ino_t ino)
{
	xfs_bstat_t	statbuf;
	jdm_fshandle_t	*fshandlep;
	int	fd, fsfd;
	int	error = 0;
	char	*tname;

	fshandlep = jdm_getfshandle(getparent (fname) );
	if (! fshandlep) {
		fsrprintf(_("unable to construct sys handle for %s: %s\n"),
			fname, strerror(errno));
		return -1;
	}

	/*
	 * Need to open something on the same filesystem as the
	 * file.  Open the parent.
	 */
	fsfd = open(getparent(fname), O_RDONLY);
	if (fsfd < 0) {
		fsrprintf(_("unable to open sys handle for %s: %s\n"),
			fname, strerror(errno));
		return -1;
	}

	if ((xfs_bulkstat_single(fsfd, &ino, &statbuf)) < 0) {
		fsrprintf(_("unable to get bstat on %s: %s\n"),
			fname, strerror(errno));
		close(fsfd);
		return -1;
	}

	fd = jdm_open( fshandlep, &statbuf, O_RDWR);
	if (fd < 0) {
		fsrprintf(_("unable to open handle %s: %s\n"),
			fname, strerror(errno));
		close(fsfd);
		return -1;
	}

	/* Get the fs geometry */
	if (xfs_getgeom(fsfd, &fsgeom) < 0 ) {
		fsrprintf(_("Unable to get geom on fs for: %s\n"), fname);
		close(fsfd);
		return -1;
	}

	close(fsfd);

	tname = gettmpname(fname);

	if (tname)
		error = fsrfile_common(fname, tname, NULL, fd, &statbuf);

	close(fd);

	return error;
}


/*
 * This is the common defrag code for either a full fs
 * defragmentation or a single file.  Check as much as
 * possible with the file, fork a process to setuid to the
 * target file owner's uid and defragment the file.
 * This is done so the new extents created in a tmp file are
 * reflected in the owners' quota without having to do any
 * special code in the kernel.  When the existing extents
 * are removed, the quotas will be correct.  It's ugly but
 * it saves us from doing some quota  re-construction in
 * the extent swap.  The price is that the defragmentation
 * will fail if the owner of the target file is already at
 * their quota limit.
 */
static int
fsrfile_common(
	char		*fname,
	char		*tname,
	char		*fsname,
	int		fd,
	xfs_bstat_t	*statp)
{
	int		error;
	struct statvfs64 vfss;
	struct fsxattr	fsx;
	unsigned long	bsize;

	if (vflag)
		fsrprintf("%s\n", fname);

	if (fsync(fd) < 0) {
		fsrprintf(_("sync failed: %s: %s\n"), fname, strerror(errno));
		return -1;
	}

	if (statp->bs_size == 0) {
		if (vflag)
			fsrprintf(_("%s: zero size, ignoring\n"), fname);
		return(0);
	}

	/* Check if a mandatory lock is set on the file to try and
	 * avoid blocking indefinitely on the reads later. Note that
	 * someone could still set a mandatory lock after this check
	 * but before all reads have completed to block fsr reads.
	 * This change just closes the window a bit.
	 */
	if ( (statp->bs_mode & S_ISGID) && ( ! (statp->bs_mode&S_IXGRP) ) ) {
		struct flock fl;

		fl.l_type = F_RDLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = (off_t)0;
		fl.l_len = 0;
		if ((fcntl(fd, F_GETLK, &fl)) < 0 ) {
			if (vflag)
				fsrprintf(_("locking check failed: %s\n"),
					fname);
			return(-1);
		}
		if (fl.l_type != F_UNLCK) {
			/* Mandatory lock is set */
			if (vflag)
				fsrprintf(_("mandatory lock: %s: ignoring\n"),
					fname);
			return(-1);
		}
	}

	/*
	 * Check if there is room to copy the file.
	 *
	 * Note that xfs_bstat.bs_blksize returns the filesystem blocksize,
	 * not the optimal I/O size as struct stat.
	 */
	if (statvfs64(fsname ? fsname : fname, &vfss) < 0) {
		fsrprintf(_("unable to get fs stat on %s: %s\n"),
			fname, strerror(errno));
		return -1;
	}
	bsize = vfss.f_frsize ? vfss.f_frsize : vfss.f_bsize;
	if (statp->bs_blksize * statp->bs_blocks >
	    vfss.f_bfree * bsize - minimumfree) {
		fsrprintf(_("insufficient freespace for: %s: "
			    "size=%lld: ignoring\n"), fname,
			    statp->bs_blksize * statp->bs_blocks);
		return 1;
	}

	if ((ioctl(fd, XFS_IOC_FSGETXATTR, &fsx)) < 0) {
		fsrprintf(_("failed to get inode attrs: %s\n"), fname);
		return(-1);
	}
	if (fsx.fsx_xflags & (XFS_XFLAG_IMMUTABLE|XFS_XFLAG_APPEND)) {
		if (vflag)
			fsrprintf(_("%s: immutable/append, ignoring\n"), fname);
		return(0);
	}
	if (fsx.fsx_xflags & XFS_XFLAG_NODEFRAG) {
		if (vflag)
			fsrprintf(_("%s: marked as don't defrag, ignoring\n"),
			    fname);
		return(0);
	}
	if (fsx.fsx_xflags & XFS_XFLAG_REALTIME) {
		if (xfs_getrt(fd, &vfss) < 0) {
			fsrprintf(_("cannot get realtime geometry for: %s\n"),
				fname);
			return(-1);
		}
		if (statp->bs_size > ((vfss.f_bfree * bsize) - minimumfree)) {
			fsrprintf(_("low on realtime free space: %s: "
				"ignoring file\n"), fname);
			return(-1);
		}
	}

	if ((RealUid != ROOT) && (RealUid != statp->bs_uid)) {
		fsrprintf(_("cannot open: %s: Permission denied\n"), fname);
		return -1;
	}

	/*
	 * Previously the code forked here, & the child changed it's uid to
	 * that of the file's owner and then called packfile(), to keep
	 * quota counts correct.  (defragged files could use fewer blocks).
	 *
	 * Instead, just fchown() the temp file to the uid,gid of the
	 * file we're defragging, in packfile().
	 */

	if ((error = packfile(fname, tname, fd, statp, &fsx)))
		return error;
	return -1; /* no error */
}

/*
 * Attempt to set the attr fork up correctly. This is simple for attr1
 * filesystems as they have a fixed inode fork offset. In that case
 * just create an attribute and that's all we need to do.
 *
 * For attr2 filesystems, see if we have the actual fork offset in
 * the bstat structure. If so, just create additional attributes on
 * the temporary inode until the offset matches.
 *
 * If it doesn't exist, we can only do best effort. Add an attribute at a time
 * to move the inode fork around, but take into account that the attribute
 * might be too small to move the fork every time we add one.  This should
 * hopefully put the fork offset in the right place. It's not a big deal if we
 * don't get it right - the kernel will reject it when we try to swap extents.
 */
static int
fsr_setup_attr_fork(
	int		fd,
	int		tfd,
	xfs_bstat_t	*bstatp)
{
#ifndef __UCLIBC__
	struct stat64	tstatbuf;
	int		i;
	int		last_forkoff = 0;
	int		no_change_cnt = 0;
	int		ret;

	if (!(bstatp->bs_xflags & XFS_XFLAG_HASATTR))
		return 0;

	/*
	 * use the old method if we have attr1 or the kernel does not yet
	 * support passing the fork offset in the bulkstat data.
	 */
	if (!(fsgeom.flags & XFS_FSOP_GEOM_FLAGS_ATTR2) ||
	    bstatp->bs_forkoff == 0) {
		/* attr1 */
		ret = fsetxattr(tfd, "user.X", "X", 1, XATTR_CREATE);
		if (ret) {
			fsrprintf(_("could not set ATTR\n"));
			return -1;
		}
		goto out;
	}

	/* attr2 w/ fork offsets */

	if (fstat64(tfd, &tstatbuf) < 0) {
		fsrprintf(_("unable to stat temp file: %s\n"),
					strerror(errno));
		return -1;
	}

	i = 0;
	do {
		xfs_bstat_t	tbstat;
		xfs_ino_t	ino;
		char		name[64];
		int		diff;

		/*
		 * bulkstat the temp inode  to see what the forkoff is. Use
		 * this to compare against the target and determine what we
		 * need to do.
		 */
		ino = tstatbuf.st_ino;
		if ((xfs_bulkstat_single(tfd, &ino, &tbstat)) < 0) {
			fsrprintf(_("unable to get bstat on temp file: %s\n"),
						strerror(errno));
			return -1;
		}
		if (dflag)
			fsrprintf(_("orig forkoff %d, temp forkoff %d\n"),
					bstatp->bs_forkoff, tbstat.bs_forkoff);

		snprintf(name, sizeof(name), "user.%d", i);

		/*
		 * If there is no attribute, then we need to create one to get
		 * an attribute fork at the default location.
		 */
		if (!tbstat.bs_forkoff) {
			ret = fsetxattr(tfd, name, "XX", 2, XATTR_CREATE);
			if (ret) {
				fsrprintf(_("could not set ATTR\n"));
				return -1;
			}
			continue;
		}

		/*
		 * make a progress check so we don't get stuck trying to extend
		 * a large btree form attribute fork.
		 */
		if (last_forkoff == tbstat.bs_forkoff) {
			if (no_change_cnt++ > 10)
				break;
		}
		no_change_cnt = 0;
		last_forkoff = tbstat.bs_forkoff;

		/* work out which way to grow the fork */
		diff = tbstat.bs_forkoff - bstatp->bs_forkoff;
		if (abs(diff) > fsgeom.inodesize - sizeof(struct xfs_dinode)) {
			fsrprintf(_("forkoff diff %d too large!\n"), diff);
			return -1;
		}

		/* if they are equal, we are done */
		if (!diff)
			goto out;

		/*
		 * if the temp inode fork offset is smaller then we have to
		 * grow the data fork
		 */
		if (diff < 0) {
			/*
			 * create some temporary extents in the inode to move
			 * the fork in the direction we need. This can be done
			 * by preallocating some single block extents at
			 * non-contiguous offsets.
			 */
			/* XXX: unimplemented! */
			goto out;
		}

		/* we need to grow the attr fork, so create another attr */
		ret = fsetxattr(tfd, name, "XX", 2, XATTR_CREATE);
		if (ret) {
			fsrprintf(_("could not set ATTR\n"));
			return -1;
		}

	} while (++i < 100); /* don't go forever */

out:
	if (dflag)
		fsrprintf(_("set temp attr\n"));
#endif
	return 0;
}

/*
 * Do the defragmentation of a single file.
 * We already are pretty sure we can and want to
 * defragment the file.  Create the tmp file, copy
 * the data (maintaining holes) and call the kernel
 * extent swap routinte.
 */
static int
packfile(char *fname, char *tname, int fd,
	 xfs_bstat_t *statp, struct fsxattr *fsxp)
{
	int 		tfd;
	int		srval;
	int		nextents, extent, cur_nextents, new_nextents;
	unsigned	blksz_dio;
	unsigned	dio_min;
	struct dioattr	dio;
	static xfs_swapext_t   sx;
	struct xfs_flock64  space;
	off64_t 	cnt, pos;
	void 		*fbuf;
	int 		ct, wc, wc_b4;
	char		ffname[SMBUFSZ];
	int		ffd = -1;

	/*
	 * Work out the extent map - nextents will be set to the
	 * minimum number of extents needed for the file (taking
	 * into account holes), cur_nextents is the current number
	 * of extents.
	 */
	nextents = read_fd_bmap(fd, statp, &cur_nextents);

	if (cur_nextents == 1 || cur_nextents <= nextents) {
		if (vflag)
			fsrprintf(_("%s already fully defragmented.\n"), fname);
		return 1; /* indicates no change/no error */
	}

	if (dflag)
		fsrprintf(_("%s extents=%d can_save=%d tmp=%s\n"),
		          fname, cur_nextents, (cur_nextents - nextents),
		          tname);

	if ((tfd = open(tname, openopts, 0666)) < 0) {
		if (vflag)
			fsrprintf(_("could not open tmp file: %s: %s\n"),
				   tname, strerror(errno));
		return -1;
	}
	unlink(tname);

	/* Setup extended attributes */
	if (fsr_setup_attr_fork(fd, tfd, statp) != 0) {
		fsrprintf(_("failed to set ATTR fork on tmp: %s:\n"), tname);
		close(tfd);
		return -1;
	}

	/* Setup extended inode flags, project identifier, etc */
	if (fsxp->fsx_xflags || fsxp->fsx_projid) {
		if (ioctl(tfd, XFS_IOC_FSSETXATTR, fsxp) < 0) {
			fsrprintf(_("could not set inode attrs on tmp: %s\n"),
				tname);
			close(tfd);
			return -1;
		}
	}

	if ((ioctl(tfd, XFS_IOC_DIOINFO, &dio)) < 0 ) {
		fsrprintf(_("could not get DirectIO info on tmp: %s\n"), tname);
		close(tfd);
		return -1;
	}

	dio_min = dio.d_miniosz;
	if (statp->bs_size <= dio_min) {
		blksz_dio = dio_min;
	} else {
		blksz_dio = min(dio.d_maxiosz, BUFFER_MAX - pagesize);
		if (argv_blksz_dio != 0)
			blksz_dio = min(argv_blksz_dio, blksz_dio);
		blksz_dio = (min(statp->bs_size, blksz_dio) / dio_min) * dio_min;
	}

	if (dflag) {
		fsrprintf(_("DEBUG: "
			"fsize=%lld blsz_dio=%d d_min=%d d_max=%d pgsz=%d\n"),
			statp->bs_size, blksz_dio, dio.d_miniosz,
			dio.d_maxiosz, pagesize);
	}

	if (!(fbuf = (char *)memalign(dio.d_mem, blksz_dio))) {
		fsrprintf(_("could not allocate buf: %s\n"), tname);
		close(tfd);
		return -1;
	}

	if (nfrags) {
		/* Create new tmp file in same AG as first */
		sprintf(ffname, "%s.frag", tname);

		/* Open the new file for sync writes */
		if ((ffd = open(ffname, openopts, 0666)) < 0) {
			fsrprintf(_("could not open fragfile: %s : %s\n"),
				   ffname, strerror(errno));
			close(tfd);
			free(fbuf);
			return -1;
		}
		unlink(ffname);
	}

	/* Loop through block map allocating new extents */
	for (extent = 0; extent < nextents; extent++) {
		pos = outmap[extent].bmv_offset;
		if (outmap[extent].bmv_block == -1) {
			space.l_whence = SEEK_SET;
			space.l_start = pos;
			space.l_len = outmap[extent].bmv_length;
			if (ioctl(tfd, XFS_IOC_UNRESVSP64, &space) < 0) {
				fsrprintf(_("could not trunc tmp %s\n"),
					   tname);
			}
			lseek64(tfd, outmap[extent].bmv_length, SEEK_CUR);
			continue;
		} else if (outmap[extent].bmv_length == 0) {
			/* to catch holes at the beginning of the file */
			continue;
		}
		if (! nfrags) {
			space.l_whence = SEEK_CUR;
			space.l_start = 0;
			space.l_len = outmap[extent].bmv_length;

			if (ioctl(tfd, XFS_IOC_RESVSP64, &space) < 0) {
				fsrprintf(_("could not pre-allocate tmp space:"
					" %s\n"), tname);
				close(tfd);
				free(fbuf);
				return -1;
			}
			lseek64(tfd, outmap[extent].bmv_length, SEEK_CUR);
		}
	} /* end of space allocation loop */

	if (lseek64(tfd, 0, SEEK_SET)) {
		fsrprintf(_("Couldn't rewind on temporary file\n"));
		close(tfd);
		free(fbuf);
		return -1;
	}

	/* Check if the temporary file has fewer extents */
	new_nextents = getnextents(tfd);
	if (dflag)
		fsrprintf(_("Temporary file has %d extents (%d in original)\n"), new_nextents, cur_nextents);
	if (cur_nextents <= new_nextents) {
		if (vflag)
			fsrprintf(_("No improvement will be made (skipping): %s\n"), fname);
		free(fbuf);
		close(tfd);
		return 1; /* no change/no error */
	}

	/* Loop through block map copying the file. */
	for (extent = 0; extent < nextents; extent++) {
		pos = outmap[extent].bmv_offset;
		if (outmap[extent].bmv_block == -1) {
			lseek64(tfd, outmap[extent].bmv_length, SEEK_CUR);
			lseek64(fd, outmap[extent].bmv_length, SEEK_CUR);
			continue;
		} else if (outmap[extent].bmv_length == 0) {
			/* to catch holes at the beginning of the file */
			continue;
		}
		for (cnt = outmap[extent].bmv_length; cnt > 0;
		     cnt -= ct, pos += ct) {
			if (nfrags && --nfrags) {
				ct = min(cnt, dio_min);
			} else if (cnt % dio_min == 0) {
				ct = min(cnt, blksz_dio);
			} else {
				ct = min(cnt + dio_min - (cnt % dio_min),
					blksz_dio);
			}
			ct = read(fd, fbuf, ct);
			if (ct == 0) {
				/* EOF, stop trying to read */
				extent = nextents;
				break;
			}
			/* Ensure we do direct I/O to correct block
			 * boundaries.
			 */
			if (ct % dio_min != 0) {
				wc = ct + dio_min - (ct % dio_min);
			} else {
				wc = ct;
			}
			wc_b4 = wc;
			if (ct < 0 || ((wc = write(tfd, fbuf, wc)) != wc_b4)) {
				if (ct < 0)
					fsrprintf(_("bad read of %d bytes "
						"from %s: %s\n"), wc_b4,
						fname, strerror(errno));
				else if (wc < 0)
					fsrprintf(_("bad write of %d bytes "
						"to %s: %s\n"), wc_b4,
						tname, strerror(errno));
				else {
					/*
					 * Might be out of space
					 *
					 * Try to finish write
					 */
					int resid = ct-wc;

					if ((wc = write(tfd, ((char *)fbuf)+wc,
							resid)) == resid) {
						/* worked on second attempt? */
						continue;
					}
					else if (wc < 0) {
						fsrprintf(_("bad write2 of %d "
							"bytes to %s: %s\n"),
							resid, tname,
							strerror(errno));
					} else {
						fsrprintf(_("bad copy to %s\n"),
							tname);
					}
				}
				free(fbuf);
				close(tfd);
				return -1;
			}
			if (nfrags) {
				/* Do a matching write to the tmp file */
				wc_b4 = wc;
				if (((wc = write(ffd, fbuf, wc)) != wc_b4)) {
					fsrprintf(_("bad write of %d bytes "
						"to %s: %s\n"),
						wc_b4, ffname, strerror(errno));
				}
			}
		}
	}
	ftruncate64(tfd, statp->bs_size);
	if (ffd > 0) close(ffd);
	fsync(tfd);

	free(fbuf);

	sx.sx_stat     = *statp; /* struct copy */
	sx.sx_version  = XFS_SX_VERSION;
	sx.sx_fdtarget = fd;
	sx.sx_fdtmp    = tfd;
	sx.sx_offset   = 0;
	sx.sx_length   = statp->bs_size;

	/* switch to the owner's id, to keep quota in line */
        if (fchown(tfd, statp->bs_uid, statp->bs_gid) < 0) {
                if (vflag)
                        fsrprintf(_("failed to fchown tmpfile %s: %s\n"),
                                   tname, strerror(errno));
		close(tfd);
                return -1;
        }

	/* Swap the extents */
	srval = xfs_swapext(fd, &sx);
	if (srval < 0) {
		if (errno == ENOTSUP) {
			if (vflag || dflag)
			   fsrprintf(_("%s: file type not supported\n"), fname);
		} else if (errno == EFAULT) {
			/* The file has changed since we started the copy */
			if (vflag || dflag)
			   fsrprintf(_("%s: file modified defrag aborted\n"),
				     fname);
		} else if (errno == EBUSY) {
			/* Timestamp has changed or mmap'ed file */
			if (vflag || dflag)
			   fsrprintf(_("%s: file busy\n"), fname);
		} else {
			fsrprintf(_("XFS_IOC_SWAPEXT failed: %s: %s\n"),
				  fname, strerror(errno));
		}
		close(tfd);
		return -1;
	}

	/* Report progress */
	if (vflag)
		fsrprintf(_("extents before:%d after:%d %s %s\n"),
			  cur_nextents, new_nextents,
			  (new_nextents <= nextents ? "DONE" : "    " ),
		          fname);
	close(tfd);
	return 0;
}

char *
gettmpname(char *fname)
{
	static char	buf[PATH_MAX+1];
	char		sbuf[SMBUFSZ];
	char		*ptr;

	sprintf(sbuf, "/.fsr%d", getpid());

	strcpy(buf, fname);
	ptr = strrchr(buf, '/');
	if (ptr) {
		*ptr = '\0';
	} else {
		strcpy(buf, ".");
	}

	if ((strlen(buf) + strlen (sbuf)) > PATH_MAX) {
		fsrprintf(_("tmp file name too long: %s\n"), fname);
		return(NULL);
	}

	strcat(buf, sbuf);

	return(buf);
}

char *
getparent(char *fname)
{
	static char	buf[PATH_MAX+1];
	char		*ptr;

	strcpy(buf, fname);
	ptr = strrchr(buf, '/');
	if (ptr) {
		if (ptr == &buf[0])
			++ptr;
		*ptr = '\0';
	} else {
		strcpy(buf, ".");
	}

	return(buf);
}

/*
 * Read in block map of the input file, coalesce contiguous
 * extents into a single range, keep all holes. Convert from 512 byte
 * blocks to bytes.
 *
 * This code was borrowed from mv.c with some minor mods.
 */
#define MAPSIZE	128
#define	OUTMAP_SIZE_INCREMENT	MAPSIZE

int	read_fd_bmap(int fd, xfs_bstat_t *sin, int *cur_nextents)
{
	int		i, cnt;
	struct getbmap	map[MAPSIZE];

#define	BUMP_CNT	\
	if (++cnt >= outmap_size) { \
		outmap_size += OUTMAP_SIZE_INCREMENT; \
		outmap = (struct getbmap *)realloc(outmap, \
		                           outmap_size*sizeof(*outmap)); \
		if (outmap == NULL) { \
			fsrprintf(_("realloc failed: %s\n"), \
				strerror(errno)); \
			exit(1); \
		} \
	}

	/*	Initialize the outmap array.  It always grows - never shrinks.
	 *	Left-over memory allocation is saved for the next files.
	 */
	if (outmap_size == 0) {
		outmap_size = OUTMAP_SIZE_INCREMENT; /* Initial size */
		outmap = (struct getbmap *)malloc(outmap_size*sizeof(*outmap));
		if (!outmap) {
			fsrprintf(_("malloc failed: %s\n"),
				strerror(errno));
			exit(1);
		}
	}

	outmap[0].bmv_block = 0;
	outmap[0].bmv_offset = 0;
	outmap[0].bmv_length = sin->bs_size;

	/*
	 * If a non regular file is involved then forget holes
	 */

	if (!S_ISREG(sin->bs_mode))
		return(1);

	outmap[0].bmv_length = 0;

	map[0].bmv_offset = 0;
	map[0].bmv_block = 0;
	map[0].bmv_entries = 0;
	map[0].bmv_count = MAPSIZE;
	map[0].bmv_length = -1;

	cnt = 0;
	*cur_nextents = 0;

	do {
		if (ioctl(fd, XFS_IOC_GETBMAP, map) < 0) {
			fsrprintf(_("failed reading extents: inode %llu"),
			         (unsigned long long)sin->bs_ino);
			exit(1);
		}

		/* Concatenate extents together and replicate holes into
		 * the output map.
		 */
		*cur_nextents += map[0].bmv_entries;
		for (i = 0; i < map[0].bmv_entries; i++) {
			if (map[i + 1].bmv_block == -1) {
				BUMP_CNT;
				outmap[cnt] = map[i+1];
			} else if (outmap[cnt].bmv_block == -1) {
				BUMP_CNT;
				outmap[cnt] = map[i+1];
			} else {
				outmap[cnt].bmv_length += map[i + 1].bmv_length;
			}
		}
	} while (map[0].bmv_entries == (MAPSIZE-1));
	for (i = 0; i <= cnt; i++) {
		outmap[i].bmv_offset = BBTOB(outmap[i].bmv_offset);
		outmap[i].bmv_length = BBTOB(outmap[i].bmv_length);
	}

	outmap[cnt].bmv_length = sin->bs_size - outmap[cnt].bmv_offset;

	return(cnt+1);
}

/*
 * Read the block map and return the number of extents.
 */
int
getnextents(int fd)
{
	int		nextents;
	struct getbmap	map[MAPSIZE];

	map[0].bmv_offset = 0;
	map[0].bmv_block = 0;
	map[0].bmv_entries = 0;
	map[0].bmv_count = MAPSIZE;
	map[0].bmv_length = -1;

	nextents = 0;

	do {
		if (ioctl(fd,XFS_IOC_GETBMAP, map) < 0) {
			fsrprintf(_("failed reading extents"));
			exit(1);
		}

		nextents += map[0].bmv_entries;
	} while (map[0].bmv_entries == (MAPSIZE-1));

	return(nextents);
}

/*
 * Get the fs geometry
 */
int
xfs_getgeom(int fd, xfs_fsop_geom_v1_t * fsgeom)
{
	if (xfs_fsgeometry(fd, fsgeom) < 0) {
		return -1;
	}
	return 0;
}

/*
 * Get xfs realtime space information
 */
int
xfs_getrt(int fd, struct statvfs64 *sfbp)
{
	unsigned long	bsize;
	unsigned long	factor;
	xfs_fsop_counts_t cnt;

	if (!fsgeom.rtblocks)
		return -1;

	if (xfs_fscounts(fd, &cnt) < 0) {
		close(fd);
		return -1;
	}
	bsize = (sfbp->f_frsize ? sfbp->f_frsize : sfbp->f_bsize);
	factor = fsgeom.blocksize / bsize;         /* currently this is == 1 */
	sfbp->f_bfree = (cnt.freertx * fsgeom.rtextsize) * factor;
	return 0;
}

int
fsrprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (gflag) {
		static int didopenlog;
		if (!didopenlog) {
			openlog("fsr", LOG_PID, LOG_USER);
			didopenlog = 1;
		}
		vsyslog(LOG_INFO, fmt, ap);
	} else
		vprintf(fmt, ap);
	va_end(ap);
	return 0;
}

/*
 * emulate getmntany
 */
static int
getmntany(FILE *fp, struct mntent *mp, struct mntent *mpref, struct stat64 *s)
{
	struct mntent *t;
	struct stat64 ms;

	while ((t = getmntent(fp))) {
		if (mpref->mnt_fsname) {	/* device */
			if (stat64(t->mnt_fsname, &ms) < 0)
				continue;
			if (s->st_rdev != ms.st_rdev)
				continue;
		}
		if (mpref->mnt_dir) {		/* mount point */
			if (stat64(t->mnt_dir, &ms) < 0)
				continue;
			if (s->st_ino != ms.st_ino || s->st_dev != ms.st_dev)
				continue;
		}
		*mp = *t;
		break;
	}
	return (t != NULL);
}


/*
 * Initialize a directory for tmp file use.  This is used
 * by the full filesystem defragmentation when we're walking
 * the inodes and do not know the path for the individual
 * files.  Multiple directories are used to spread out the
 * tmp data around to different ag's (since file data is
 * usually allocated to the same ag as the directory and
 * directories allocated round robin from the same
 * parent directory).
 */
static void
tmp_init(char *mnt)
{
	int 	i;
	static char	buf[SMBUFSZ];
	mode_t	mask;

	tmp_agi = 0;
	sprintf(buf, "%s/.fsr", mnt);

	mask = umask(0);
	if (mkdir(buf, 0700) < 0) {
		if (errno == EEXIST) {
			if (dflag)
				fsrprintf(_("tmpdir already exists: %s\n"),
						buf);
		} else {
			fsrprintf(_("could not create tmpdir: %s: %s\n"),
					buf, strerror(errno));
			exit(-1);
		}
	}
	for (i=0; i < fsgeom.agcount; i++) {
		sprintf(buf, "%s/.fsr/ag%d", mnt, i);
		if (mkdir(buf, 0700) < 0) {
			if (errno == EEXIST) {
				if (dflag)
					fsrprintf(
					_("tmpdir already exists: %s\n"), buf);
			} else {
				fsrprintf(_("cannot create tmpdir: %s: %s\n"),
				       buf, strerror(errno));
				exit(-1);
			}
		}
	}
	(void)umask(mask);
	return;
}

static char *
tmp_next(char *mnt)
{
	static char	buf[SMBUFSZ];

	sprintf(buf, "%s/.fsr/ag%d/tmp%d",
	        ( (strcmp(mnt, "/") == 0) ? "" : mnt),
	        tmp_agi,
	        getpid());

	if (++tmp_agi == fsgeom.agcount)
		tmp_agi = 0;

	return(buf);
}

static void
tmp_close(char *mnt)
{
	static char	buf[SMBUFSZ];
	int i;

	/* No data is ever actually written so we can just do rmdir's */
	for (i=0; i < fsgeom.agcount; i++) {
		sprintf(buf, "%s/.fsr/ag%d", mnt, i);
		if (rmdir(buf) < 0) {
			if (errno != ENOENT) {
				fsrprintf(
					_("could not remove tmpdir: %s: %s\n"),
			 		buf, strerror(errno));
			}
		}
	}
	sprintf(buf, "%s/.fsr", mnt);
	if (rmdir(buf) < 0) {
		if (errno != ENOENT) {
			fsrprintf(_("could not remove tmpdir: %s: %s\n"),
			          buf, strerror(errno));
		}
	}
}
