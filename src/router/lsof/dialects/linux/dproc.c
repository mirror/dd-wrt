/*
 * dproc.c - Linux process access functions for /proc-based lsof
 */


/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright 1997 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dproc.c,v 1.29 2015/07/07 19:46:33 abe Exp $";
#endif

#include "lsof.h"


/*
 * Local definitions
 */

#define	FDINFO_FLAGS		1	/* fdinfo flags available */
#define	FDINFO_POS		2	/* fdinfo position available */
#define FDINFO_ALL		(FDINFO_FLAGS | FDINFO_POS)
#define	LSTAT_TEST_FILE		"/"
#define LSTAT_TEST_SEEK		1

#if	!defined(ULLONG_MAX)
#define	ULLONG_MAX		18446744073709551615ULL
#endif	/* !defined(ULLONG_MAX) */


/*
 * Local structures
 */

struct l_fdinfo {
	int flags;			/* flags: line value */
	off_t pos;			/* pos: line value */
};


/*
 * Local variables
 */

static short Cckreg;			/* conditional status of regular file
					 * checking:
					 *     0 = unconditionally check
					 *     1 = conditionally check */
static short Ckscko;			/* socket file only checking status:
					 *     0 = none
					 *     1 = check only socket files */


/*
 * Local function prototypes
 */

_PROTOTYPE(static MALLOC_S alloc_cbf,(MALLOC_S len, char **cbf, MALLOC_S cbfa));
_PROTOTYPE(static int get_fdinfo,(char *p, struct l_fdinfo *fi));
_PROTOTYPE(static int getlinksrc,(char *ln, char *src, int srcl, char **rest));
_PROTOTYPE(static int isefsys,(char *path, char *type, int l,
			       efsys_list_t **rep, struct lfile **lfr));
_PROTOTYPE(static int nm2id,(char *nm, int *id, int *idl));
_PROTOTYPE(static int read_id_stat,(char *p, int id, char **cmd, int *ppid,
				    int *pgid));
_PROTOTYPE(static void process_proc_map,(char *p, struct stat *s, int ss));
_PROTOTYPE(static int process_id,(char *idp, int idpl, char *cmd, UID_ARG uid,
				  int pid, int ppid, int pgid, int tid));
_PROTOTYPE(static int statEx,(char *p, struct stat *s, int *ss));
 

#if	defined(HASSELINUX)
_PROTOTYPE(static int cmp_cntx_eq,(char *pcntx, char *ucntx));


#include <fnmatch.h>


/*
 * cmp_cntx_eq -- compare program and user security contexts
 */

static int
cmp_cntx_eq(pcntx, ucntx)
	char *pcntx;			       /* program context */
	char *ucntx;			       /* user supplied context */
{
	return !fnmatch(ucntx, pcntx, 0);
}


/*
 * enter_cntx_arg() - enter name ecurity context argument
 */

int
enter_cntx_arg(cntx)
	char *cntx;			       /* context */
{
	cntxlist_t *cntxp;
/*
 * Search the argument list for a duplicate.
 */
	for (cntxp = CntxArg; cntxp; cntxp = cntxp->next) {
	    if (!strcmp(cntxp->cntx, cntx)) {
		if (!Fwarn) {
		    (void) fprintf(stderr, "%s: duplicate context: %s\n",
			Pn, cntx);
		}
		return(1);
	    }
	}
/*
 * Create and link a new context argument list entry.
 */
	if (!(cntxp = (cntxlist_t *)malloc((MALLOC_S)sizeof(cntxlist_t)))) {
	    (void) fprintf(stderr, "%s: no space for context: %s\n", Pn, cntx);
	    Exit(1);
	}
	cntxp->f = 0;
	cntxp->cntx = cntx;
	cntxp->next = CntxArg;
	CntxArg = cntxp;
	return(0);
}
#endif	/* defined(HASSELINUX) */


/*
 * alloc_cbf() -- allocate a command buffer
 */

static MALLOC_S
alloc_cbf(len, cbf, cbfa)
	MALLOC_S len;				/* required length */
	char **cbf;				/* current buffer */
	MALLOC_S cbfa;				/* current buffer allocation */
{
	if (*cbf)
	    *cbf = (char *)realloc((MALLOC_P *)*cbf, len);
	else
	    *cbf = (char *)malloc(len);
	if (!*cbf) {
	    (void) fprintf(stderr,
		"%s: can't allocate command %d bytes\n", Pn, (int)len);
	     Exit(1);
	}
	return(len);
}


/*
 * gather_proc_info() -- gather process information
 */

void
gather_proc_info()
{
	char *cmd, *tcmd;
	struct dirent *dp;
	unsigned char ht, pidts;
	int n, nl, pgid, pid, ppid, rv, tid, tpgid, tppid, tx;
	static char *path = (char *)NULL;
	static int pathl = 0;
	static char *pidpath = (char *)NULL;
	static MALLOC_S pidpathl = 0;
	static MALLOC_S pidx = 0;
	static DIR *ps = (DIR *)NULL;
	struct stat sb;
	static char *taskpath = (char *)NULL;
	static int taskpathl = 0;
	static char *tidpath = (char *)NULL;
	static int tidpathl = 0;
	DIR *ts;
	UID_ARG uid;

/*
 * Do one-time setup.
 */
	if (!pidpath) {
	    pidx = strlen(PROCFS) + 1;
	    pidpathl = pidx + 64 + 1;	/* 64 is growth room */
	    if (!(pidpath = (char *)malloc(pidpathl))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for \"%s/\"<pid>\n",
		    Pn, (int)pidpathl, PROCFS);
		Exit(1);
	    }
	    (void) snpf(pidpath, pidpathl, "%s/", PROCFS);
	}
/*
 * Get lock and net information.
 */
	(void) make_proc_path(pidpath, pidx, &path, &pathl, "locks");
	(void) get_locks(path);
	(void) make_proc_path(pidpath, pidx, &path, &pathl, "net/");
	(void) set_net_paths(path, strlen(path));
/*
 * If only socket files have been selected, or socket files have been selected
 * ANDed with other selection options, enable the skipping of regular files.
 *
 * If socket files and some process options have been selected, enable
 * conditional skipping of regular file; i.e., regular files will be skipped
 * unless they belong to a process selected by one of the specified options.
 */
	if (Selflags & SELNW) {

	/*
	 * Some network files selection options have been specified.
	 */
	    if (Fand || !(Selflags & ~SELNW)) {

	    /*
	     * Selection ANDing or only network file options have been
	     * specified, so set unconditional skipping of regular files
	     * and socket file only checking.
	     */
		Cckreg = 0;
		Ckscko = 1;
	    } else {

	    /*
	     * If ORed file selection options have been specified, or no ORed
	     * process selection options have been specified, enable
	     * unconditional file checking and clear socket file only checking.
	     *
	     * If only ORed process selection options have been specified,
	     * enable conditional file skipping and socket file only checking.
	     */
		if ((Selflags & SELFILE) || !(Selflags & SELPROC))
		    Cckreg = Ckscko = 0;
		else
		    Cckreg = Ckscko = 1;
	    }
	} else {

	/*
	 * No network file selection options were specified.  Enable
	 * unconditional file checking and clear socket file only checking.
	 */
	    Cckreg = Ckscko = 0;
	}
/*
 * Read /proc, looking for PID directories.  Open each one and
 * gather its process and file information.
 */
	if (!ps) {
	    if (!(ps = opendir(PROCFS))) {
		(void) fprintf(stderr, "%s: can't open %s\n", Pn, PROCFS);
		Exit(1);
	    }
	} else
	    (void) rewinddir(ps);
	while ((dp = readdir(ps))) {
	    if (nm2id(dp->d_name, &pid, &n))
		continue;
	/*
	 * Build path to PID's directory.
	 */
	    if ((pidx + n + 1 + 1) > pidpathl) {
		pidpathl = pidx + n + 1 + 1 + 64;
		if (!(pidpath = (char *)realloc((MALLOC_P *)pidpath, pidpathl)))
		{
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for \"%s/%s/\"\n",
			Pn, (int)pidpathl, PROCFS, dp->d_name);
		    Exit(1);
		}
	    }
	    (void) snpf(pidpath + pidx, pidpathl - pidx, "%s/", dp->d_name);
	    n += (pidx + 1);
	/*
	 * Process the PID's stat info.
	 */
	    if (stat(pidpath, &sb))
		continue;
	    uid = (UID_ARG)sb.st_uid;
	    ht = pidts = 0;

#if	defined(HASTASKS)
	/*
	 * If task reporting is selected, check the tasks of the process first,
	 * so that the "-p<PID> -aK" options work properly.
	 */
	    if ((Selflags & SELTASK)) {
		(void) make_proc_path(pidpath, n, &taskpath, &taskpathl,
				      "task");
		tx = n + 4;
		if ((ts = opendir(taskpath))) {

		/*
		 * Process the PID's tasks.  Record the open files of those
		 * whose TIDs do not match the PID and which are themselves
		 * not zombies.
		 */
		    while ((dp = readdir(ts))) {

		    /*
		     * Get the task ID.  Skip the task if its ID matches the
		     * process PID.
		     */
			if (nm2id(dp->d_name, &tid, &nl))
			    continue;
			if  (tid == pid) {
			    pidts = 1;
			    continue;
			}
		    /*
		     * Form the path for the TID.
		     */
			if ((tx + 1 + nl + 1 + 4) > tidpathl) {
			    tidpathl = tx + 1 + n + 1 + 4 + 64;
			    if (tidpath)
				tidpath = (char *)realloc((MALLOC_P *)tidpath,
							  tidpathl);
			    else
				tidpath = (char *)malloc((MALLOC_S)tidpathl);
			    if (!tidpath) {
				(void) fprintf(stderr,
				    "%s: can't allocate %d task bytes", Pn,
				    tidpathl);
				(void) fprintf(stderr, " for \"%s/%s/stat\"\n",
				    taskpath, dp->d_name);
				Exit(1);
			    }
			}
			(void) snpf(tidpath, tidpathl, "%s/%s/stat", taskpath,
			    dp->d_name);
		    /*
		     * Check the task state.
		     */
			rv = read_id_stat(tidpath, tid, &tcmd, &tppid,
					  &tpgid);
			if ((rv < 0) || (rv == 1))
			    continue;
		    /*
		     * Attempt to record the task.
		     */
			if (!process_id(tidpath, (tx + 1 + nl+ 1), tcmd, uid,
					pid, tppid, tpgid, tid))
			{
			    ht = 1;
			}
		    }
		    (void) closedir(ts);
		}
	    }
#endif	/* defined(HASTASKS) */

	/*
	 * If the main process is a task and task selection has been specified
	 * along with option ANDing, enter the main process temporarily as a
	 * task,  so that the "-aK" option set lists the main process along
	 * with its tasks.
	 */
	    (void) make_proc_path(pidpath, n, &path, &pathl, "stat");
	    if (((rv = read_id_stat(path, pid, &cmd, &ppid, &pgid)) >= 0) 
	    &&   (rv != 1))
	    {
		tid = (Fand && ht && pidts && (Selflags & SELTASK)) ? pid : 0;
		if ((!process_id(pidpath, n, cmd, uid, pid, ppid, pgid, tid))
		&&  tid)
		{
		    Lp->tid = 0;
		}
	    }
	}
}


/*
 * get_fdinfo() - get values from /proc/<PID>fdinfo/FD
 */

static int
get_fdinfo(p, fi)
	char *p;			/* path to fdinfo file */
	struct l_fdinfo *fi;		/* pointer to local fdinfo values
					 * return structure */
{
	char buf[MAXPATHLEN + 1], *ep, **fp;
	FILE *fs;
	int rv = 0;
	unsigned long ul;
	unsigned long long ull;
/*
 * Signal no values returned (0) if no fdinfo pointer was provided or if the
 * fdinfo path can't be opened.
 */
	if (!fi)
	    return(0);
	if (!p || !*p || !(fs = fopen(p, "r")))
	    return(0);
/*
 * Read the fdinfo file.
 */
	while (fgets(buf, sizeof(buf), fs)) {
	    if (get_fields(buf, (char *)NULL, &fp, (int *)NULL, 0) < 2)
		continue;
	    if (!fp[0] || !*fp[0] || !fp[1] || !*fp[1])
		continue;
	    if (!strcmp(fp[0], "flags:")) {

	    /*
	     * Process a "flags:" line.
	     */
		ep = (char *)NULL;
		if ((ul = strtoul(fp[1], &ep, 0)) == ULONG_MAX
		||  !ep || *ep)
		    continue;
		fi->flags = (unsigned int)ul;
		if ((rv |= FDINFO_FLAGS) == FDINFO_ALL)
		    break;
	    } else if (!strcmp(fp[0], "pos:")) {

	    /*
	     * Process a "pos:" line.
	     */
		ep = (char *)NULL;
		if ((ull = strtoull(fp[1], &ep, 0)) == ULLONG_MAX
		||  !ep || *ep)
		    continue;
		fi->pos = (off_t)ull;
		if ((rv |= FDINFO_POS) == FDINFO_ALL)
		    break;
	    }
	}
	fclose(fs);
/*
 * Signal via the return value what information was obtained. (0 == none)
 */
	return(rv);
}


/*
 * getlinksrc() - get the source path name for the /proc/<PID>/fd/<FD> link
 */


static int
getlinksrc(ln, src, srcl, rest)
	char *ln;			/* link path */
	char *src;			/* link source path return address */
	int srcl;			/* length of src[] */
	char **rest;			/* pointer to what follows the ':' in
					 * the link source path (NULL if no
					 * return requested) */
{
	char *cp;
	int ll;

	if (rest)
	    *rest = (char *)NULL;
	if ((ll = readlink(ln, src, srcl - 1)) < 1
	||  ll >= srcl)
	    return(-1);
	src[ll] = '\0';
	if (*src == '/')
	    return(ll);
	if ((cp = strchr(src, ':'))) {
	    *cp = '\0';
	    ll = strlen(src);
	    if (rest)
		*rest = cp + 1;
	}
	return(ll);
}


/*
 * initialize() - perform all initialization
 */

void
initialize()
{
	int fd;
	struct l_fdinfo fi;
	char path[MAXPATHLEN];
	struct stat sb;
/*
 * Test for -i and -X option conflict.
 */
	if (Fxopt && (Fnet || Nwad)) {
	    (void) fprintf(stderr, "%s: -i is useless when -X is specified.\n",
		Pn);
	    usage(1, 0, 0);
	}
/*
 * Open LSTAT_TEST_FILE and seek to byte LSTAT_TEST_SEEK, then lstat the
 * /proc/<PID>/fd/<FD> for LSTAT_TEST_FILE to see what position is reported.
 * If the result is LSTAT_TEST_SEEK, enable offset reporting.
 *
 * If the result isn't LSTAT_TEST_SEEK, next check the fdinfo file for the
 * open LSTAT_TEST_FILE file descriptor.  If it exists and contains a "pos:"
 * value, and if the value is LSTAT_TEST_SEEK, enable offset reporting.
 */
	if ((fd = open(LSTAT_TEST_FILE, O_RDONLY)) >= 0) {
	    if (lseek(fd, (off_t)LSTAT_TEST_SEEK, SEEK_SET)
	    == (off_t)LSTAT_TEST_SEEK) {
		(void) snpf(path, sizeof(path), "%s/%d/fd/%d", PROCFS, Mypid,
			    fd);
		if (!lstat(path, &sb)) {
		    if (sb.st_size == (off_t)LSTAT_TEST_SEEK)
			OffType = 1;
		}
	    }
	    if (!OffType) {
		(void) snpf(path, sizeof(path), "%s/%d/fdinfo/%d", PROCFS,
			    Mypid, fd);
		if (get_fdinfo(path, &fi) & FDINFO_POS) {
		    if (fi.pos == (off_t)LSTAT_TEST_SEEK)
			OffType = 2;
		}
	    }
	    (void) close(fd);
	}
	if (!OffType) {
	    if (Foffset && !Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: can't report offset; disregarding -o.\n",
		    Pn);
	    Foffset = 0;
	    Fsize = 1;
	}
	if (Fsv && (OffType != 2)) {
	    if (!Fwarn && FsvByf)
		(void) fprintf(stderr,
		    "%s: WARNING: can't report file flags; disregarding +f.\n",
		    Pn);
	    Fsv = 0;
	}
/*
 * Make sure the local mount info table is loaded if doing anything other
 * than just Internet lookups.  (HasNFS is defined during the loading of the
 * local mount table.)
 */
	if (Selinet == 0)
	    (void) readmnt();
}


/*
 * make_proc_path() - make a path in a /proc directory
 *
 * entry:
 *	pp = pointer to /proc prefix
 *	lp = length of prefix
 *	np = pointer to malloc'd buffer to receive new file's path
 *	nl = length of new file path buffer
 *	sf = new path's suffix
 *
 * return: length of new path
 *	np = updated with new path
 *	nl = updated with new path length
 */

int
make_proc_path(pp, pl, np, nl, sf)
	char *pp;			/* path prefix -- e.g., /proc/<pid>/ */
	int pl;				/* strlen(pp) */
	char **np;			/* malloc'd receiving buffer */
	int *nl;			/* strlen(*np) */
	char *sf;			/* suffix of new path */
{
	char *cp;
	MALLOC_S rl, sl;

	sl = strlen(sf);
	if ((rl = pl + sl + 1) > *nl) {
	    if ((cp = *np))
		cp = (char *)realloc((MALLOC_P *)cp, rl);
	    else
		cp = (char *)malloc(rl);
	    if (!cp) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for %s%s\n",
		    Pn, (int)rl, pp, sf);
		Exit(1);
	    }
	    *nl = rl;
	    *np = cp;
	}
	(void) snpf(*np, *nl, "%s", pp);
	(void) snpf(*np + pl, *nl - pl, "%s", sf);
	return(rl - 1);
}


/*
 * isefsys() -- is path on a file system exempted with -e
 *
 * Note: alloc_lfile() must have been called in advance.
 */

static int
isefsys(path, type, l, rep, lfr)
	char *path;			/* path to file */
	char *type;			/* unknown file type */
	int l;				/* link request: 0 = report
					 *               1 = link */
	efsys_list_t **rep;		/* returned Efsysl pointer, if not
					 * NULL */
	struct lfile **lfr;		/* allocated struct lfile pointer */
{
	efsys_list_t *ep;
	int ds, len;
	struct mounts *mp;
	char nmabuf[MAXPATHLEN + 1];

	len = (int) strlen(path);
	for (ep = Efsysl; ep; ep = ep->next) {

	/*
	 * Look for a matching exempt file system path at the beginning of
	 * the file path.
	 */
	    if (ep->pathl > len)
		continue;
	    if (strncmp(ep->path, path, ep->pathl))
		continue;
	/*
	 * If only reporting, return information as requested.
	 */
	    if (!l) {
		if (rep)
		    *rep = ep;
		return(0);
	    }
	/*
	 * Process an exempt file.
	 */
	    ds = 0;
	    if ((mp = ep->mp)) {
		if (mp->ds & SB_DEV) {
		    Lf->dev = mp->dev;
		    ds = Lf->dev_def = 1;
		}
		if (mp->ds & SB_RDEV) {
		    Lf->rdev = mp->rdev;
		    ds = Lf->rdev_def = 1;
		}
	    }
	    if (!ds)
		(void) enter_dev_ch("UNKNOWN");
	    Lf->ntype = N_UNKN;
	    (void) snpf(Lf->type, sizeof(Lf->type), "%s",
			(type ? type : "UNKN"));
	    (void) enter_nm(path);
	    (void) snpf(nmabuf, sizeof(nmabuf), "(%ce %s)",
		ep->rdlnk ? '+' : '-', ep->path);
	    nmabuf[sizeof(nmabuf) - 1] = '\0';
	    (void) add_nma(nmabuf, strlen(nmabuf));
	    if (Lf->sf) {
		if (lfr)
		    *lfr = Lf;
		link_lfile();
	    } else if (lfr)
		*lfr = (struct lfile *)NULL;
	    return(0);
	}
	return(1);
}


/*
 * nm2id() - convert a name to an integer ID
 */

static int
nm2id(nm, id, idl)
	char *nm;			/* pointer to name */
	int *id;			/* pointer to ID receiver */
	int *idl;			/* pointer to ID length receiver */
{
	register int tid, tidl;

	for (*id = *idl = tid = tidl = 0; *nm; nm++) {

#if	defined(__STDC__)	/* { */
	    if (!isdigit((unsigned char)*nm))
#else	/* !defined(__STDC__)	   } { */
	    if (!isascii(*nm) || !isdigit((unsigned char)*cp))
#endif	/* defined(__STDC__)	   } */

		{
		    return(1);
		}
		tid = tid * 10 + (int)(*nm - '0');
		tidl++;
	}
	*id = tid;
	*idl = tidl;
	return(0);
}


/*
 * open_proc_stream() -- open a /proc stream
 */

FILE *
open_proc_stream(p, m, buf, sz, act)
	char *p;			/* pointer to path to open */
	char *m;			/* pointer to mode -- e.g., "r" */
	char **buf;			/* pointer tp setvbuf() address
					 * (NULL if none) */
	size_t *sz;			/* setvbuf() size (0 if none or if
					 * getpagesize() desired */
	int act;			/* fopen() failure action:
					 *     0 : return (FILE *)NULL
					 *   <>0 : fprintf() an error message
					 *         and Exit(1)
					 */
{
	FILE *fs;			/* opened stream */
	static size_t psz = (size_t)0;	/* page size */
	size_t tsz;			/* temporary size */
/*
 * Open the stream.
 */
	if (!(fs = fopen(p, m))) {
	    if (!act)
		return((FILE *)NULL);
	    (void) fprintf(stderr, "%s: can't fopen(%s, \"%s\"): %s\n",
		Pn, p, m, strerror(errno));
	    Exit(1);
	}
/*
 * Return the stream if no buffer change is required.
 */
	if (!buf)
	    return(fs);
/*
 * Determine the buffer size required.
 */
	if (!(tsz = *sz)) {
	    if (!psz)
		psz = getpagesize();
	    tsz = psz;
	}
/*
 * Allocate a buffer for the stream, as required.
 */
	if (!*buf) {
	    if (!(*buf = (char *)malloc((MALLOC_S)tsz))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for %s stream buffer\n",
		    Pn, (int)tsz, p);
		Exit(1);
	    }
	    *sz = tsz;
	}
/*
 * Assign the buffer to the stream.
 */
	if (setvbuf(fs, *buf, _IOFBF, tsz)) {
	    (void) fprintf(stderr, "%s: setvbuf(%s)=%d failure: %s\n",
		Pn, p, (int)tsz, strerror(errno));
	    Exit(1);
	}
	return(fs);
}


/*
 * process_id - process ID: PID or LWP
 *
 * return:  0 == ID processed
 *          1 == ID not processed
 */

static int
process_id(idp, idpl, cmd, uid, pid, ppid, pgid, tid)
	char *idp;			/* pointer to ID's path */
	int idpl;			/* pointer to ID's path length */
	char *cmd;			/* pointer to ID's command */
	UID_ARG uid;			/* ID's UID */
	int pid;			/* ID's PID */
	int ppid;			/* parent PID */
	int pgid;			/* parent GID */
	int tid;			/* task ID, if non-zero */
{
	int av;
	static char *dpath = (char *)NULL;
	static int dpathl = 0;
	short efs, enls, enss, lnk, oty, pn, pss, sf;
	int fd, i, ls, n, ss, sv;
	struct l_fdinfo fi;
	DIR *fdp;
	struct dirent *fp;
	static char *ipath = (char *)NULL;
	static int ipathl = 0;
	int j = 0;
	struct lfile *lfr;
	struct stat lsb, sb;
	char nmabuf[MAXPATHLEN + 1], pbuf[MAXPATHLEN + 1];
	static char *path = (char *)NULL;
	static int pathl = 0;
	static char *pathi = (char *)NULL;
	static int pathil = 0;
	char *rest;
	int txts = 0;

#if	defined(HASSELINUX)
	cntxlist_t *cntxp;
#endif	/* defined(HASSELINUX) */

/*
 * See if process is excluded.
 */
	if (is_proc_excl(pid, pgid, uid, &pss, &sf, tid)
	||  is_cmd_excl(cmd, &pss, &sf))
	{

#if	defined(HASEPTOPTS)
	    if (!FeptE)
		return(1);
#else	/* !defined(HASEPTOPTS) */
	    return(1);
#endif	/* defined(HASEPTOPTS) */

	}
	if (Cckreg && !FeptE) {

	/*
	 * If conditional checking of regular files is enabled, enable
	 * socket file only checking, based on the process' selection
	 * status.
	 */
	    Ckscko = (sf & SELPROC) ? 0 : 1;
	}
	alloc_lproc(pid, pgid, ppid, uid, cmd, (int)pss, (int)sf);
	Lp->tid = tid;
	Plf = (struct lfile *)NULL;
/*
 * Process the ID's current working directory info.
 */
	efs = 0;
	if (!Ckscko) {
	    (void) make_proc_path(idp, idpl, &path, &pathl, "cwd");
	    alloc_lfile(CWD, -1);
	    if (getlinksrc(path, pbuf, sizeof(pbuf), (char **)NULL) < 1) {
		if (!Fwarn) {
		    zeromem((char *)&sb, sizeof(sb));
		    lnk = ss = 0;
		    (void) snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
			strerror(errno));
		    nmabuf[sizeof(nmabuf) - 1] = '\0';
		    (void) add_nma(nmabuf, strlen(nmabuf));
		    pn = 1;
		} else
		    pn = 0;
	    } else {
		lnk = pn = 1;
		if (Efsysl && !isefsys(pbuf, "UNKNcwd", 1, NULL, &lfr)) {
		    efs = 1;
		    pn = 0;
		} else {
		    ss = SB_ALL;
		    if (HasNFS) {
			if ((sv = statsafely(path, &sb)))
			sv = statEx(pbuf, &sb, &ss);
		    } else
			sv = stat(path, &sb);
		    if (sv) {
			ss = 0;
			if (!Fwarn) {
			    (void) snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
				strerror(errno));
			    nmabuf[sizeof(nmabuf) - 1] = '\0';
			    (void) add_nma(nmabuf, strlen(nmabuf));
			}
		    }
		}
	    }
	    if (pn) {
		(void) process_proc_node(lnk ? pbuf : path,
					 path, &sb, ss,
					 (struct stat *)NULL, 0);
		if (Lf->sf)
		    link_lfile();
	    }
	}
/*
 * Process the ID's root directory info.
 */
	lnk = ss = 0;
	if (!Ckscko) {
	    (void) make_proc_path(idp, idpl, &path, &pathl, "root");
	    alloc_lfile(RTD, -1);
	    if (getlinksrc(path, pbuf, sizeof(pbuf), (char **)NULL) < 1) {
		if (!Fwarn) {
		    zeromem((char *)&sb, sizeof(sb));
		    (void) snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
			strerror(errno));
		    nmabuf[sizeof(nmabuf) - 1] = '\0';
		    (void) add_nma(nmabuf, strlen(nmabuf));
		    pn = 1;
		} else
		    pn = 0;
	    } else {
		lnk = pn = 1;
		if (Efsysl && !isefsys(pbuf, "UNKNrtd", 1, NULL, NULL))
		    pn = 0;
		else {
		    ss = SB_ALL;
		    if (HasNFS) {
			if ((sv = statsafely(path, &sb)))
			    sv = statEx(pbuf, &sb, &ss);
		    } else
			sv = stat(path, &sb);
		    if (sv) {
			ss = 0;
			if (!Fwarn) {
			    (void) snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
				strerror(errno));
			    nmabuf[sizeof(nmabuf) - 1] = '\0';
			    (void) add_nma(nmabuf, strlen(nmabuf));
			}
		    }
		}
	    }
	    if (pn) {
		(void) process_proc_node(lnk ? pbuf : path,
					 path, &sb, ss,
					 (struct stat *)NULL, 0);
		if (Lf->sf)
		    link_lfile();
	    }
	}
/*
 * Process the ID's execution info.
 */
	lnk = ss = txts = 0;
	if (!Ckscko) {
	    (void) make_proc_path(idp, idpl, &path, &pathl, "exe");
	    alloc_lfile("txt", -1);
	    if (getlinksrc(path, pbuf, sizeof(pbuf), (char **)NULL) < 1) {
		zeromem((void *)&sb, sizeof(sb));
		if (!Fwarn) {
		    if ((errno != ENOENT) || uid) {
			(void) snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
			    strerror(errno));
			nmabuf[sizeof(nmabuf) - 1] = '\0';
			(void) add_nma(nmabuf, strlen(nmabuf));
		    }
		    pn = 1;
		} else
		    pn = 0;
	    } else {
		lnk = pn = 1;
		if (Efsysl && !isefsys(pbuf, "UNKNtxt", 1, NULL, NULL))
		    pn = 0;
		else {
		    ss = SB_ALL;
		    if (HasNFS) {
			if ((sv = statsafely(path, &sb))) {
			    sv = statEx(pbuf, &sb,  &ss);
			    if (!sv && (ss & SB_DEV) && (ss & SB_INO))
				txts = 1;
			}
		    } else
			sv = stat(path, &sb);
		    if (sv) {
			ss = 0;
			if (!Fwarn) {
			    (void) snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
				strerror(errno));
			    nmabuf[sizeof(nmabuf) - 1] = '\0';
			    (void) add_nma(nmabuf, strlen(nmabuf));
			}
		    } else
			txts = 1;
		}
	    }
	    if (pn) {
		(void) process_proc_node(lnk ? pbuf : path,
					 path, &sb, ss,
					 (struct stat *)NULL, 0);
		if (Lf->sf)
		    link_lfile();
	    }
	}
/*
 * Process the ID's memory map info.
 */
	if (!Ckscko) {
	    (void) make_proc_path(idp, idpl, &path, &pathl, "maps");
	    (void) process_proc_map(path, txts ? &sb : (struct stat *)NULL,
				    txts ? ss : 0);
	}

#if	defined(HASSELINUX)
/*
 * Process the PID's SELinux context.
 */
	if (Fcntx) {

	/*
	 * If the -Z (cntx) option was specified, match the valid contexts.
	 */
	    errno = 0;
	    if (getpidcon(pid, &Lp->cntx) == -1) {
		Lp->cntx = (char *)NULL;
		if (!Fwarn) {
		    (void) snpf(nmabuf, sizeof(nmabuf),
			"(getpidcon: %s)", strerror(errno));
		    if (!(Lp->cntx = strdup(nmabuf))) {
			(void) fprintf(stderr,
			    "%s: no context error space: PID %ld",
			    Pn, (long)Lp->pid);
			Exit(1);
		    }
		}
	    } else if (CntxArg) {

	    /*
	     * See if context includes the process.
	     */
		for (cntxp = CntxArg; cntxp; cntxp = cntxp->next) {
		    if (cmp_cntx_eq(Lp->cntx, cntxp->cntx)) {
			cntxp->f = 1;
			Lp->pss |= PS_PRI;
			Lp->sf |= SELCNTX;
			break;
		    }
		}
	    }
	}
#endif	/* defined(HASSELINUX) */

/*
 * Process the ID's file descriptor directory.
 */
	if ((i = make_proc_path(idp, idpl, &dpath, &dpathl, "fd/")) < 3)
	    return(0);
	dpath[i - 1] = '\0';
	if ((OffType == 2)
	&&  ((j = make_proc_path(idp, idpl, &ipath, &ipathl, "fdinfo/")) >= 7))
	    oty = 1;
	else
	    oty = 0;
	if (!(fdp = opendir(dpath))) {
	    if (!Fwarn) {
		(void) snpf(nmabuf, sizeof(nmabuf), "%s (opendir: %s)",
		    dpath, strerror(errno));
		alloc_lfile("NOFD", -1);
		nmabuf[sizeof(nmabuf) - 1] = '\0';
		(void) add_nma(nmabuf, strlen(nmabuf));
		link_lfile();
	    }
	    return(0);
	}
	dpath[i - 1] = '/';
	while ((fp = readdir(fdp))) {
	    if (nm2id(fp->d_name, &fd, &n))
		continue;
	    (void) make_proc_path(dpath, i, &path, &pathl, fp->d_name);
	    (void) alloc_lfile((char *)NULL, fd);
	    if (getlinksrc(path, pbuf, sizeof(pbuf), &rest) < 1) {
		zeromem((char *)&sb, sizeof(sb));
		lnk = ss = 0;
		if (!Fwarn) {
		    (void) snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
			strerror(errno));
		    nmabuf[sizeof(nmabuf) - 1] = '\0';
		    (void) add_nma(nmabuf, strlen(nmabuf));
		    pn = 1;
		} else
		    pn = 0;
	    } else {
		lnk = 1;
		if (Efsysl && !isefsys(pbuf, "UNKNfd", 1, NULL, &lfr)) {
		    efs = 1;
		    pn = 0;
		} else {
		    if (HasNFS) {
			if (lstatsafely(path, &lsb)) {
			    (void) statEx(pbuf, &lsb, &ls);
			    enls = errno;
			} else {
			    enls = 0;
			    ls = SB_ALL;
			}
			if (statsafely(path, &sb)) {
			    (void) statEx(pbuf, &sb, &ss);
			    enss = errno;
			} else {
			    enss = 0;
			    ss = SB_ALL;
			}
		    } else {
			ls = lstat(path, &lsb) ? 0 : SB_ALL;
			enls = errno;
			ss = stat(path, &sb) ? 0 : SB_ALL;
			enss = errno;
		    }
		    if (!ls && !Fwarn) {
			(void) snpf(nmabuf, sizeof(nmabuf), "lstat: %s)",
			    strerror(enls));
			nmabuf[sizeof(nmabuf) - 1] = '\0';
			(void) add_nma(nmabuf, strlen(nmabuf));
		    }
		    if (!ss && !Fwarn) {
			(void) snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
			    strerror(enss));
			nmabuf[sizeof(nmabuf) - 1] = '\0';
			(void) add_nma(nmabuf, strlen(nmabuf));
		    }
		    if (Ckscko) {
			if ((ss & SB_MODE)
			&&  ((sb.st_mode & S_IFMT) == S_IFSOCK))
			{
			    pn = 1;
			} else
			    pn = 0;
		    } else
			pn = 1;
		}
	    }
	    if (pn || (efs && lfr && oty)) {
		if (oty) {
		    (void) make_proc_path(ipath, j, &pathi, &pathil,
					  fp->d_name);
		    if ((av = get_fdinfo(pathi, &fi)) & FDINFO_POS) {
			if (efs) {
			    if (Foffset) {
				lfr->off = (SZOFFTYPE)fi.pos;
				lfr->off_def = 1;
			    }
			} else {
			    ls |= SB_SIZE;
			    lsb.st_size = fi.pos;
			}
		    } else
			ls &= ~SB_SIZE;

#if	!defined(HASNOFSFLAGS)
		    if ((av & FDINFO_FLAGS) && (Fsv & FSV_FG)) {
			if (efs) {
			    lfr->ffg = (long)fi.flags;
			    lfr->fsv |= FSV_FG;
			} else {
			    Lf->ffg = (long)fi.flags;
			    Lf->fsv |= FSV_FG;
			}
		     }
# endif	/* !defined(HASNOFSFLAGS) */

		}
		if (pn) {
		    process_proc_node(lnk ? pbuf : path, path, &sb, ss, &lsb,
				      ls);
		    if ((Lf->ntype == N_ANON_INODE) && rest && *rest)
			enter_nm(rest);
		    if (Lf->sf)
			link_lfile();
		}
	    }
	}
	(void) closedir(fdp);
	return(0);
}


/*
 * process_proc_map() - process the memory map of a process
 */

static void
process_proc_map(p, s, ss)
	char *p;			/* path to process maps file */
	struct stat *s;			/* executing text file state buffer */
	int ss;				/* *s status -- i.e., SB_* values */
{
	char buf[MAXPATHLEN + 1], *ep, fmtbuf[32], **fp, nmabuf[MAXPATHLEN + 1];
	dev_t dev;
	int ds, efs, en, i, mss, nf, sv;
	int eb = 6;
	INODETYPE inode;
	MALLOC_S len;
	long maj, min;
	FILE *ms;
	int ns = 0;
	struct stat sb;
	struct saved_map {
	    dev_t dev;
	    INODETYPE inode;
	};
	static struct saved_map *sm = (struct saved_map *)NULL;
	efsys_list_t *rep;
	static int sma = 0;
	static char *vbuf = (char *)NULL;
	static size_t vsz = (size_t)0;
/*
 * Open the /proc/<pid>/maps file, assign a page size buffer to its stream,
 * and read it/
 */
	if (!(ms = open_proc_stream(p, "r", &vbuf, &vsz, 0)))
	    return;
	while (fgets(buf, sizeof(buf), ms)) {
	    if ((nf = get_fields(buf, ":", &fp, &eb, 1)) < 7)
		continue;			/* not enough fields */
	    if (!fp[6] || !*fp[6])
		continue;			/* no path name */
	/*
	 * See if the path ends in " (deleted)".  If it does, strip the
	 * " (deleted)" characters and remember that they were there.
	 */
	    if (((ds = (int)strlen(fp[6])) > 10)
	    &&  !strcmp(fp[6] + ds - 10, " (deleted)"))
	    {
		*(fp[6] + ds - 10) = '\0';
	    } else
		ds = 0;
	/*
	 * Assemble the major and minor device numbers.
	 */
	    ep = (char *)NULL;
	    if (!fp[3] || !*fp[3]
	    ||  (maj = strtol(fp[3], &ep, 16)) == LONG_MIN || maj == LONG_MAX
	    ||  !ep || *ep)
		continue;
	    ep = (char *)NULL;
	    if (!fp[4] || !*fp[4]
	    ||  (min = strtol(fp[4], &ep, 16)) == LONG_MIN || min == LONG_MAX
	    ||  !ep || *ep)
		continue;
	/*
	 * Assemble the device and inode numbers.  If they are both zero, skip
	 * the entry.
	 */
	    dev = (dev_t)makedev((int)maj, (int)min);
	    if (!fp[5] || !*fp[5])
		continue;
	    ep = (char *)NULL;
	    if ((inode = strtoull(fp[5], &ep, 0)) == ULLONG_MAX
	    ||  !ep || *ep)
		continue;
	    if (!dev && !inode)
		continue;
	/*
	 * See if the device + inode pair match that of the executable.
	 * If they do, skip this map entry.
	 */
	    if (s && (ss & SB_DEV) && (ss & SB_INO)
	    &&  (dev == s->st_dev) && (inode == (INODETYPE)s->st_ino))
		continue;
	/*
	 * See if this device + inode pair has already been processed as
	 * a map entry.
	 */
	    for (i = 0; i < ns; i++) {
		if (dev == sm[i].dev && inode == sm[i].inode)
		    break;
	    }
	    if (i < ns)
		continue;
	/*
	 * Record the processing of this map entry's device and inode pair.
	 */
	    if (ns >= sma) {
		sma += 10;
		len = (MALLOC_S)(sma * sizeof(struct saved_map));
		if (sm)
		    sm = (struct saved_map *)realloc(sm, len);
		else
		    sm = (struct saved_map *)malloc(len);
		if (!sm) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for saved maps, PID %d\n",
			Pn, (int)len, Lp->pid);
		    Exit(1);
		}
	    }
	    sm[ns].dev = dev;
	    sm[ns++].inode = inode;
	/*
	 * Allocate space for the mapped file, then get stat(2) information
	 * for it.  Skip the stat(2) operation if this is on an exempt file
	 * system.
	 */
	    alloc_lfile("mem", -1);
	    if (Efsysl && !isefsys(fp[6], (char *)NULL, 0, &rep, NULL))
		efs = sv = 1;
	    else
		efs = 0;
	    if (!efs) {
		if (HasNFS)
		    sv = statsafely(fp[6], &sb);
		else
		    sv = stat(fp[6], &sb);
	    }
	    if (sv || efs) {
		en = errno;
	    /*
	     * Applying stat(2) to the file was not possible (file is on an
	     * exempt file system) or stat(2) failed, so manufacture a partial
	     * stat(2) reply from the process' maps file entry.
	     *
	     * If the file has been deleted, reset its type to "DEL";
	     * otherwise generate a stat() error name addition.
	     */
		zeromem((char *)&sb, sizeof(sb));
		sb.st_dev = dev;
		sb.st_ino = (ino_t)inode;
		sb.st_mode = S_IFREG;
		mss = SB_DEV | SB_INO | SB_MODE;
		if (ds)
		    alloc_lfile("DEL", -1);
		else if (!efs && !Fwarn) {
		    (void) snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
			strerror(en));
		    nmabuf[sizeof(nmabuf) - 1] = '\0';
		    (void) add_nma(nmabuf, strlen(nmabuf));
		}
	    } else if ((sb.st_dev != dev) || ((INODETYPE)sb.st_ino != inode)) {

	    /*
	     * The stat(2) device and inode numbers don't match those obtained
	     * from the process' maps file.
	     *
	     * If the file has been deleted, reset its type to "DEL"; otherwise
	     * generate inconsistency name additions.
	     *
	     * Manufacture a partial stat(2) reply from the maps file
	     * information.
	     */
		if (ds)
		    alloc_lfile("DEL", -1);
		else if (!Fwarn) {
		    char *sep;

		    if (sb.st_dev != dev) {
			(void) snpf(nmabuf, sizeof(nmabuf),
			    "(path dev=%d,%d%s",
			    GET_MAJ_DEV(sb.st_dev), GET_MIN_DEV(sb.st_dev),
			    ((INODETYPE)sb.st_ino == inode) ? ")" : ",");
			nmabuf[sizeof(nmabuf) - 1] = '\0';
			(void) add_nma(nmabuf, strlen(nmabuf));
			sep = "";
		    } else
			sep = "(path ";
		    if ((INODETYPE)sb.st_ino != inode) {
			(void) snpf(fmtbuf, sizeof(fmtbuf), "%%sinode=%s)",
			    InodeFmt_d);
			(void) snpf(nmabuf, sizeof(nmabuf), fmtbuf,
			    sep, (INODETYPE)sb.st_ino);
			nmabuf[sizeof(nmabuf) - 1] = '\0';
			(void) add_nma(nmabuf, strlen(nmabuf));
		    }
		}
		zeromem((char *)&sb, sizeof(sb));
		sb.st_dev = dev;
		sb.st_ino = (ino_t)inode;
		sb.st_mode = S_IFREG;
		mss = SB_DEV | SB_INO | SB_MODE;
	    } else
		mss = SB_ALL;
	/*
	 * Record the file's information.
	 */
	    if (!efs)
		process_proc_node(fp[6], fp[6], &sb, mss, (struct stat *)NULL,
				  0);
	    else {

	    /*
	     * If this file is on an exempt file system, complete the lfile
	     * structure, but change its type and add the exemption note to
	     * the NAME column.
	     */
		Lf->dev = sb.st_dev;
		Lf->inode = (ino_t)sb.st_ino;
		Lf->dev_def = Lf->inp_ty = 1;
		(void) enter_nm(fp[6]);
		(void) snpf(Lf->type, sizeof(Lf->type), "%s",
			    (ds ? "UNKNdel" : "UNKNmem"));
		(void) snpf(nmabuf, sizeof(nmabuf), "(%ce %s)",
		    rep->rdlnk ? '+' : '-', rep->path);
		nmabuf[sizeof(nmabuf) - 1] = '\0';
		(void) add_nma(nmabuf, strlen(nmabuf));
	    }
	    if (Lf->sf)
		link_lfile();
	}
	(void) fclose(ms);
}


/*
 * read_id_stat() - read ID (PID or LWP ID) status
 *
 * return: -1 == ID is unavailable
 *          0 == ID OK
 *          1 == ID is a zombie
 *	    2 == ID is a thread
 */

static int
read_id_stat(p, id, cmd, ppid, pgid)
	char *p;			/* path to status file */
	int id;				/* ID: PID or LWP */
	char **cmd;			/* malloc'd command name */
	int *ppid;			/* returned parent PID for PID type */
	int *pgid;			/* returned process group ID for PID
					 * type */
{
	char buf[MAXPATHLEN], *cp, *cp1, **fp;
	int ch, cx, es, nf;
	static char *cbf = (char *)NULL;
	static MALLOC_S cbfa = 0;
	FILE *fs;
	static char *vbuf = (char *)NULL;
	static size_t vsz = (size_t)0;
/*
 * Open the stat file path, assign a page size buffer to its stream,
 * and read the file's first line.
 */
	if (!(fs = open_proc_stream(p, "r", &vbuf, &vsz, 0)))
	    return(-1);
	if (!(cp = fgets(buf, sizeof(buf), fs))) {

read_id_stat_exit:

	    (void) fclose(fs);
	    return(-1);
	}
/*
 * Skip to the first field, and make sure it is a matching ID.
 */
	cp1 = cp;
	while (*cp && (*cp != ' ') && (*cp != '\t'))
	    cp++;
	if (*cp)
	    *cp = '\0';
	if (atoi(cp1) != id)
	    goto read_id_stat_exit;
/*
 * The second field should contain the command, enclosed in parentheses.
 * If it also has embedded '\n' characters, replace them with '?' characters,
 * accumulating command characters until a closing parentheses appears.
 *
 */
	for (++cp; *cp && (*cp == ' '); cp++)
		;
	if (!cp || (*cp != '('))
	    goto read_id_stat_exit;
	cp++;
/*
 * Enter the command characters safely.  Supply them from the initial read
 * of the stat file line, a '\n' if the initial read didn't yield a ')'
 * command closure, or by reading the rest of the command a character at
 * a time from the stat file.
 */
	for (cx = es = 0;;) {
	    if (!es)
		ch = *cp++;
	    else {
		if ((ch = fgetc(fs)) == EOF)
		    goto read_id_stat_exit;
	    }
	    if (ch == ')')		/* ')' closes the command */
		break;
	    if ((cx + 2) > cbfa)
		cbfa = alloc_cbf((cx + 2), &cbf, cbfa);
	    cbf[cx] = ch;
	    cx++;
	    cbf[cx] = '\0';
	    if (!es && !*cp)
		es = 1;		/* Switch to fgetc() when a '\0' appears. */
	}
	*cmd = cbf;
/*
 * Read the remainder of the stat line if it was necessary to read command
 * characters individually from the stat file.
 *
 * Separate the reminder into fields.
 */
	if (es)
	    cp = fgets(buf, sizeof(buf), fs);
	(void) fclose(fs);
	if (!cp || !*cp)
	    return(-1);
	if ((nf = get_fields(cp, (char *)NULL, &fp, (int *)NULL, 0)) < 3)
	    return(-1);
/*
 * Convert and return parent process (fourth field) and process group (fifth
 * field) IDs.
 */
	if (fp[1] && *fp[1])
	    *ppid = atoi(fp[1]);
	else
	    return(-1);
	if (fp[2] && *fp[2])
	    *pgid = atoi(fp[2]);
	else
	    return(-1);
/*
 * Check the state in the third field.  If it is 'Z', return that indication.
 */
	if (fp[0] && !strcmp(fp[0], "Z"))
	    return(1);
	else if (fp[0] && !strcmp(fp[0], "T"))
	    return(2);
	return(0);
}


/*
 * statEx() - extended stat() to get device numbers when a "safe" stat has
 *	      failed and the system has an NFS mount
 *
 * Note: this function was suggested by Paul Szabo as a way to get device
 *       numbers for NFS files when an NFS mount point has the root_squash
 *       option set.  In that case, even if lsof is setuid(root), the identity
 *	 of its requests to stat() NFS files lose root permission and may fail.
 *
 *	 This function should be used only when links have been successfully
 *	 resolved in the /proc path by getlinksrc().
 */

static int
statEx(p, s, ss)
	char *p;			/* file path */
	struct stat *s;			/* stat() result -- NULL if none
					 * wanted */
	int *ss;			/* stat() status --  SB_* values */
{
	static size_t ca = 0;
	static char *cb = NULL;
	char *cp;
	int ensv = ENOENT;
	struct stat sb;
	int st = 0;
	size_t sz;
/*
 * Make a copy of the path.
 */
	sz = strlen(p);
	if ((sz + 1) > ca) {
	    if (cb)
		cb = (char *)realloc((MALLOC_P *)cb, sz + 1);
	    else
		cb = (char *)malloc(sz + 1);
	    if (!cb) {
		(void) fprintf(stderr,
		    "%s: PID %ld: no statEx path space: %s\n",
		    Pn, (long)Lp->pid, p);
		Exit(1);
	    }
	    ca = sz + 1;
	}
	(void) strcpy(cb, p);
/*
 * Trim trailing leaves from the end of the path one at a time and do a safe
 * stat() on each trimmed result.  Stop when a safe stat() succeeds or doesn't
 * fail because of EACCES or EPERM.
 */
	for (cp = strrchr(cb, '/'); cp && (cp != cb);) {
	    *cp = '\0';
	    if (!statsafely(cb, &sb)) {
		st = 1;
		break;
	    }
	    ensv = errno;
	    if ((ensv != EACCES) && (ensv != EPERM))
		break;
	    cp = strrchr(cb, '/');
	}
/*
 * If a stat() on a trimmed result succeeded, form partial results containing
 * only the device and raw device numbers.
 */
	zeromem((char *)s, sizeof(struct stat));
	if (st) {
	    errno = 0;
	    s->st_dev = sb.st_dev;
	    s->st_rdev = sb.st_rdev;
	    *ss = SB_DEV | SB_RDEV;
	    return(0);
	}
	errno = ensv;
	*ss = 0;
	return(1);
}
