/*
 * dproc.c -- Darwin process access functions for libproc-based lsof
 */


/*
 * Portions Copyright 2005-2007 Apple Inc.  All rights reserved.
 *
 * Copyright 2005 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Allan Nathanson, Apple Inc., and Victor A. Abell, Purdue
 * University.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors, nor Apple Inc. nor Purdue University are
 *    responsible for any consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either
 *    by explicit claim or by omission.  Credit to the authors, Apple
 *    Inc. and Purdue University must appear in documentation and sources.
 *    and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */


#ifndef lint
static char copyright[] =
"@(#) Copyright 2005-2007 Apple Inc. and Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dproc.c,v 1.9 2013/01/02 17:03:05 abe Exp $";
#endif

#include "lsof.h"


/*
 * Local definitions
 */

#define	PIDS_INCR	(sizeof(int) * 32)	/* PID space increment */
#define	VIPS_INCR	16			/* Vips space increment */

#if	DARWINV>=900
#define	THREADS_INCR	(sizeof(uint64_t) * 32)	/* Threads space increment */
#endif	/* DARWINV>=900 */

#ifdef	PROC_PIDLISTFILEPORTS
#define	FILEPORTS_INCR	(sizeof(struct proc_fileportinfo) * 32)	/* Fileports space increment */
#endif	/* PROC_PIDLISTFILEPORTS */

/*
 * Local static variables
 */

static struct proc_fdinfo *Fds = (struct proc_fdinfo *)NULL;
						/* FD buffer */
static int NbPids = 0;				/* bytes allocated to Pids */
static int NbFds = 0;				/* bytes allocated to FDs */
static int *Pids = (int *)NULL;			/* PID buffer */

#if	DARWINV>=900
static int NbThreads = 0;			/* Threads bytes allocated */
static uint64_t *Threads = (uint64_t *)NULL;	/* Thread buffer */
#endif	/* DARWINV>=900 */

#ifdef	PROC_PIDLISTFILEPORTS
static struct proc_fileportinfo *Fps = (struct proc_fileportinfo *)NULL;
						/* fileport buffer */
static int NbFps = 0;				/* bytes allocated to fileports */
#endif	/* PROC_PIDLISTFILEPORTS */

/*
 * Local structure definitions
 */

static struct vips_info {
	dev_t	dev;
	ino_t	ino;
} *Vips	= (struct vips_info *)NULL;		/* recorded vnodes */
static int NbVips = 0;				/* bytes allocated to Vips */
static int NVips = 0;				/* entries allocated to Vips */


/*
 * Local function prototypes
 */
_PROTOTYPE(static void enter_vn_text,(struct vnode_info_path *vip, int *n));
_PROTOTYPE(static void process_fds,(int pid, uint32_t n, int ckscko));
_PROTOTYPE(static void process_text,(int pid));

#if	DARWINV>=900
_PROTOTYPE(static void process_threads,(int pid, uint32_t n));
#endif	/* DARWINV>=900 */

#ifdef	PROC_PIDLISTFILEPORTS
_PROTOTYPE(static void process_fileports,(int pid, int ckscko));
#endif	/* PROC_PIDLISTFILEPORTS */

/*
 * enter_vn_text() -- enter vnode information text reference
 */

static void
enter_vn_text(vip, n)
	struct vnode_info_path *vip;	/* vnode info */
	int *n;				/* number of vips[] entries in use */
{
	int i;
/*
 * Ignore the request if the vnode information has already been entered.
 */
	for (i = 0; i < *n; i++) {
	    if ((vip->vip_vi.vi_stat.vst_dev == Vips[i].dev)
	    &&  (vip->vip_vi.vi_stat.vst_ino == Vips[i].ino))
	    {
		return;
	    }
	}
/*
 * Save the text file information.
 */
	alloc_lfile(" txt", -1);
	Cfp = (struct file *)NULL;
	(void) enter_vnode_info(vip);
	if (Lf->sf)
	    link_lfile();
/*
 * Record the entry of the vnode information.
 */
	if (i >= NVips) {

	/*
	 * Allocate space for recording the vnode information.
	 */
	    NVips += VIPS_INCR;
	    NbVips += (int)(VIPS_INCR * sizeof(struct vips_info));
	    if (!Vips)
		Vips = (struct vips_info *)malloc((MALLOC_S)NbVips);
	    else
		Vips = (struct vips_info *)realloc((MALLOC_P *)Vips,
						   (MALLOC_S)NbVips);
	    if (!Vips) {
		(void) fprintf(stderr, "%s: PID %d: no text recording space\n",
		    Pn, Lp->pid);
		Exit(1);
	    }
	}
/*
 * Record the vnode information.
 */
	Vips[*n].dev = vip->vip_vi.vi_stat.vst_dev;
	Vips[*n].ino = vip->vip_vi.vi_stat.vst_ino;
	(*n)++;
}


/*
 * gather_proc_info() -- gather process information
 */

void
gather_proc_info()
{
	short cckreg;			/* conditional status of regular file
					 * checking:
					 *     0 = unconditionally check
					 *     1 = conditionally check */
	short ckscko;			/* socket file only checking status:
					 *     0 = none
					 *     1 = check only socket files,
					 *	   including TCP and UDP
					 *	   streams with eXPORT data,
					 *	   where supported */
	int cre, cres, ef, i, nb, np, pid;
	short pss, sf;
	struct proc_taskallinfo tai;
	struct proc_vnodepathinfo vpi;
/*
 * Define socket and regular file conditional processing flags.
 *
 * If only socket files have been selected, or socket files have been
 * selected, ANDed with other selection options, enable the skipping of
 * regular files.
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
		cckreg = 0;
		ckscko = 1;
	    } else {

	    /*
	     * If ORed file selection options have been specified, or no
	     * ORed process selection options have been specified, enable
	     * unconditional file checking and clear socket file only
	     * checking.
	     *
	     * If only ORed process selection options have been specified,
	     * enable conditional file skipping and socket file only checking.
	     */
		if ((Selflags & SELFILE) || !(Selflags & SELPROC))
		    cckreg = ckscko = 0;
		else
		    cckreg = ckscko = 1;
	    }
	} else {

	/*
	 * No network file selection options were specified.  Enable
	 * unconditional file checking and clear socket file only checking.
	 */
	    cckreg = ckscko = 0;
	}
/*
 * Determine how many bytes are needed to contain the PIDs on the system;
 * make sure sufficient buffer space is allocated to hold them (and a few
 * extra); then read the list of PIDs.
 */
	if ((nb = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0)) <= 0) {
	    (void) fprintf(stderr, "%s: can't get PID byte count: %s\n",
		Pn, strerror(errno));
	    Exit(1);
	}
	if (nb > NbPids) {
	    while (nb > NbPids) {
		NbPids += PIDS_INCR;
	    }
	    if (!Pids)
		Pids = (int *)malloc((MALLOC_S)NbPids);
	    else
		Pids = (int *)realloc((MALLOC_P *)Pids, (MALLOC_S)NbPids);
	    if (!Pids) {
		(void) fprintf(stderr,
		    "%s: can't allocate space for %d PIDs\n", Pn,
		    (int)(NbPids / sizeof(int *)));
		Exit(1);
	    }
	}
/*
 * Get the list of PIDs.
 */
	for (ef = 0; !ef;) {
	    if ((nb = proc_listpids(PROC_ALL_PIDS, 0, Pids, NbPids)) <= 0) {
		(void) fprintf(stderr, "%s: can't get list of PIDs: %s\n",
		    Pn, strerror(errno));
		Exit(1);
	    }

	    if ((nb + sizeof(int)) < NbPids) {

	    /*
	     * There is room in the buffer for at least one more PID.
	     */
		np = nb / sizeof(int);
		ef = 1;
	    } else {

	    /*
	     * The PID buffer must be enlarged.
	     */
		NbPids += PIDS_INCR;
		Pids = (int *)realloc((MALLOC_P *)Pids, (MALLOC_S)NbPids);
		if (!Pids) {
		    (void) fprintf(stderr,
			"%s: can't allocate space for %d PIDs\n", Pn,
			(int)(NbPids / sizeof(int *)));
		    Exit(1);
		}
	    }
	}
/*
 * Loop through the identified processes.
 */
	for (i = 0; i < np; i++) {
	    if (!(pid = Pids[i]))
		continue;
	    nb = proc_pidinfo(pid, PROC_PIDTASKALLINFO, 0, &tai, sizeof(tai));
	    if (nb <= 0) {
		if ((errno == EPERM) || (errno == ESRCH))
		    continue;
		if (!Fwarn) {
		    (void) fprintf(stderr, "%s: PID %d information error: %s\n",
			Pn, pid, strerror(errno));
		}
		continue;
	    } else if (nb < sizeof(tai)) {
		(void) fprintf(stderr,
		    "%s: PID %d: proc_pidinfo(PROC_PIDTASKALLINFO);\n",
		    Pn, pid);
		(void) fprintf(stderr,
		    "      too few bytes; expected %ld, got %d\n",
		    sizeof(tai), nb);
		Exit(1);
	    }
	/*
	 * Check for process or command exclusion.
	 */
	    if (is_proc_excl((int)pid, (int)tai.pbsd.pbi_pgid,
			     (UID_ARG)tai.pbsd.pbi_uid, &pss, &sf))
	    {
		continue;
	    }
	    tai.pbsd.pbi_comm[sizeof(tai.pbsd.pbi_comm) - 1] = '\0';
	    if (is_cmd_excl(tai.pbsd.pbi_comm, &pss, &sf))
		continue;
	    if (tai.pbsd.pbi_name[0]) {
		tai.pbsd.pbi_name[sizeof(tai.pbsd.pbi_name) - 1] = '\0';
		if (is_cmd_excl(tai.pbsd.pbi_name, &pss, &sf))
		    continue;
	    }
	    if (cckreg) {

	    /*
	     * If conditional checking of regular files is enabled, enable
	     * socket file only checking, based on the process' selection
	     * status.
	     */
		ckscko = (sf & SELPROC) ? 0 : 1;
	    }
	/*
	 * Get root and current directory information.
	 */
	    if (!ckscko) {
		nb = proc_pidinfo(pid, PROC_PIDVNODEPATHINFO, 0, &vpi,
		     sizeof(vpi));
		if (nb <= 0) {
		    cre = errno;
		    cres = 1;
		} else if (nb < sizeof(vpi)) {
		    (void) fprintf(stderr,
			"%s: PID %d: proc_pidinfo(PROC_PIDVNODEPATHINFO);\n",
			Pn, pid);
		    (void) fprintf(stderr,
			"      too few bytes; expected %ld, got %d\n",
			sizeof(vpi), nb);
		    Exit(1);
		} else
		    cres = 0;
	    }
	/*
	 * Allocate local process space.
	 */
	    alloc_lproc((int)pid, (int)tai.pbsd.pbi_pgid,
		(int)tai.pbsd.pbi_ppid, (UID_ARG)tai.pbsd.pbi_uid,
		(tai.pbsd.pbi_name[0] != '\0') ? tai.pbsd.pbi_name
					       : tai.pbsd.pbi_comm,
		(int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;
	/*
	 * Save current working directory information.
	 */
	    if (!ckscko) {
		if (cres || vpi.pvi_cdir.vip_path[0]) {
		    alloc_lfile(CWD, -1);
		    Cfp = (struct file *)NULL;
		    if (cres) {

		    /*
		     * If the CWD|RTD information access error is ESRCH,
		     * ignore it; otherwise report the error's message in the
		     * CWD's NAME  column.
		     */
			if (cre != ESRCH) {
			    (void) snpf(Namech, Namechl, "%s|%s info error: %s",
				CWD + 1, RTD + 1, strerror(cre));
			    Namech[Namechl - 1] = '\0';
			    enter_nm(Namech);
			    if (Lf->sf)
				link_lfile();
			}
		    } else {
			(void) enter_vnode_info(&vpi.pvi_cdir);
			if (Lf->sf)
			    link_lfile();
		    }
		}
	    }
	/*
	 * Save root directory information.
	 */
	    if (!ckscko) {
		if (!cres && vpi.pvi_rdir.vip_path[0]) {
		    alloc_lfile(RTD, -1);
		    Cfp = (struct file *)NULL;
		    (void) enter_vnode_info(&vpi.pvi_rdir);
		    if (Lf->sf)
			link_lfile();
		}
	    }

#if	DARWINV>=900
	/*
	 * Check for per-thread current working directories
	 */
	    if (!ckscko) {
		if (tai.pbsd.pbi_flags & PROC_FLAG_THCWD) {
	    	    (void) process_threads(pid, tai.ptinfo.pti_threadnum);
		}
	    }
#endif	/* DARWINV>=900 */

	/*
	 * Print text file information.
	 */
	    if (!ckscko)
		(void) process_text(pid);

#ifdef	PROC_PIDLISTFILEPORTS
	/*
	 * Loop through the fileports
	 */
	    (void) process_fileports(pid, ckscko);
#endif	/* PROC_PIDLISTFILEPORTS */

	/*
	 * Loop through the file descriptors.
	 */
	    (void) process_fds(pid, tai.pbsd.pbi_nfiles, ckscko);
	/*
	 * Examine results.
	 */
	    if (examine_lproc())
		return;
	}
}


/*
 * initialize() -- perform all initialization
 */

void
initialize()
{
}


/*
 * process_fds() -- process file descriptors
 */

static void
process_fds(pid, n, ckscko)
	int pid;			/* PID of interest */
	uint32_t n;			/* max FDs */
	int ckscko;			/* check socket files only */
{
	int i, isock, nb, nf;
	struct proc_fdinfo *fdp;
/*
 * Make sure an FD buffer has been allocated.
 */
	if (!Fds) {
	    NbFds = sizeof(struct proc_fdinfo) * n;
	    Fds = (struct proc_fdinfo *)malloc((MALLOC_S)NbFds);
	} else if (NbFds < sizeof(struct proc_fdinfo) * n) {

	/*
	 * More proc_fdinfo space is required.  Allocate it.
	 */
	    NbFds = sizeof(struct proc_fdinfo) * n;
	    Fds = (struct proc_fdinfo *)realloc((MALLOC_P *)Fds,
						(MALLOC_S)NbFds);
	}
	if (!Fds) {
	    (void) fprintf(stderr,
		"%s: PID %d: can't allocate space for %d FDs\n",
		Pn, pid, (int)(NbFds / sizeof(struct proc_fdinfo)));
	    Exit(1);
	}
/*
 * Get FD information for the process.
 */
	nb = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, Fds, NbFds);
	if (nb <= 0) {
	    if (errno == ESRCH) {

	    /*
	     * Quit if no FD information is available for the process.
	     */
		return;
	    }
	/*
	 * Make a dummy file entry with an error message in its NAME column.
	 */
	    alloc_lfile(" err", -1);
	    (void) snpf(Namech, Namechl, "FD info error: %s", strerror(errno));
	    Namech[Namechl - 1] = '\0';
	    enter_nm(Namech);
	    if (Lf->sf)
		link_lfile();
	    return;
	}
	nf = (int)(nb / sizeof(struct proc_fdinfo));
/*
 * Loop through the file descriptors.
 */
	for (i = 0; i < nf; i++) {
	    fdp = &Fds[i];
	    alloc_lfile(NULL, (int)fdp->proc_fd);
	/*
	 * Process the file by its type.
	 */
	    isock = 0;
	    switch (fdp->proc_fdtype) {
	    case PROX_FDTYPE_ATALK:
		if (!ckscko)
		    (void) process_atalk(pid, fdp->proc_fd);
		break;
	    case PROX_FDTYPE_FSEVENTS:
		if (!ckscko)
		    (void) process_fsevents(pid, fdp->proc_fd);
		break;
	    case PROX_FDTYPE_KQUEUE:
		if (!ckscko)
		    (void) process_kqueue(pid, fdp->proc_fd);
		break;
	    case PROX_FDTYPE_PIPE:
		if (!ckscko)
		    (void) process_pipe(pid, fdp->proc_fd);
		break;
	    case PROX_FDTYPE_PSEM:
		if (!ckscko)
		    (void) process_psem(pid, fdp->proc_fd);
		break;
	    case PROX_FDTYPE_SOCKET:
		(void) process_socket(pid, fdp->proc_fd);
		isock = 1;
		break;
	    case PROX_FDTYPE_PSHM:
		(void) process_pshm(pid, fdp->proc_fd);
		break;
	    case PROX_FDTYPE_VNODE:
		(void) process_vnode(pid, fdp->proc_fd);
		break;
	    default:
		(void) snpf(Namech, Namechl - 1, "unknown file type: %d",
		    fdp->proc_fdtype);
		Namech[Namechl - 1] = '\0';
		(void) enter_nm(Namech);
		break;
	    }
	    if (Lf->sf) {
		if (!ckscko || isock)
		    link_lfile();
	    }
	}
}


#ifdef	PROC_PIDLISTFILEPORTS
/*
 * process_fileports() -- process fileports
 */

static void
process_fileports(pid, ckscko)
	int pid;			/* PID of interest */
	int ckscko;			/* check socket files only */
{
	int ef, i, isock, nb = 0, nf;
	struct proc_fileportinfo *fpi;

/*
 * Get fileport information for the process.
 */
	for (ef = 0; !ef;) {
	    nb = proc_pidinfo(pid, PROC_PIDLISTFILEPORTS, 0, Fps, NbFps);
	    if (nb == 0) {

		/*
		 * Quit if no fileport information
		 */
		return;
	    } else if (nb < 0) {
		if (errno == ESRCH) {

		/*
		 * Quit if no fileport information is available for the process.
		 */
		    return;
		}
	    /*
	     * Make a dummy file entry with an error message in its NAME column.
	     */
		alloc_lfile(" err", -1);
		(void) snpf(Namech, Namechl, "FILEPORT info error: %s", strerror(errno));
		Namech[Namechl - 1] = '\0';
		enter_nm(Namech);
		if (Lf->sf)
		    link_lfile();
	    }

	    if ((nb + sizeof(struct proc_fileportinfo)) < NbFps) {

    	    /*
	     * There is room in the buffer for at least one more fileport.
	     */
		ef = 1;
	    } else {
		if (Fps && ((nb = proc_pidinfo(pid, PROC_PIDLISTFILEPORTS, 0, NULL, 0)) <= 0)) {
		    (void) fprintf(stderr, "%s: can't get fileport byte count: %s\n",
					Pn, strerror(errno));
		    Exit(1);
		}

		/*
		 * The fileport buffer must be enlarged.
		 */
		while (nb > NbFps) {
		    NbFps += FILEPORTS_INCR;
		}
		if (!Fps)
		    Fps = (struct proc_fileportinfo *)malloc((MALLOC_S)NbFps);
		else
		    Fps = (struct proc_fileportinfo *)realloc((MALLOC_P *)Fps, (MALLOC_S)NbFps);
	    }
	}

/*
 * Loop through the fileports.
 */
	nf = (int)(nb / sizeof(struct proc_fileportinfo));
	for (i = 0; i < nf; i++) {
	    fpi = &Fps[i];
	/*
	 * fileport reported as "fp." with "(fileport=0xXXXX)" in the Name column
	 */
	    alloc_lfile(" fp.", -1);
	    Lf->fileport = fpi->proc_fileport;
	/*
	 * Process the file by its type.
	 */
	    isock = 0;
	    switch (fpi->proc_fdtype) {
	    case PROX_FDTYPE_PIPE:
		if (!ckscko)
		    (void) process_fileport_pipe(pid, fpi->proc_fileport);
		break;
	    case PROX_FDTYPE_SOCKET:
		(void) process_fileport_socket(pid, fpi->proc_fileport);
		isock = 1;
		break;
	    case PROX_FDTYPE_PSHM:
		(void) process_fileport_pshm(pid, fpi->proc_fileport);
		break;
	    case PROX_FDTYPE_VNODE:
		(void) process_fileport_vnode(pid, fpi->proc_fileport);
		break;
	    default:
		(void) snpf(Namech, Namechl - 1, "unknown file type: %d",
		    fpi->proc_fileport);
		Namech[Namechl - 1] = '\0';
		(void) enter_nm(Namech);
		break;
	    }
	    if (Lf->sf) {
		if (!ckscko || isock)
		    link_lfile();
	    }
	}
}
#endif	/* PROC_PIDLISTFILEPORTS */


/*
 * process_text() -- process text information
 */

static void
process_text(pid)
	int pid;			/* PID */
{
	uint64_t a;
	int i, n, nb;
	struct proc_regionwithpathinfo rwpi;

	for (a = (uint64_t)0, i = n = 0; i < 10000; i++) {
	    nb = proc_pidinfo(pid, PROC_PIDREGIONPATHINFO, a, &rwpi,
			      sizeof(rwpi));
	    if (nb <= 0) {
		if ((errno == ESRCH) || (errno == EINVAL)) {

		/*
		 * Quit if no more text information is available for the
		 * process.
		 */
		    return;
		}
	    /*
	     * Warn about all other errors via a NAME column message.
	     */
		alloc_lfile(" txt", -1);
		Cfp = (struct file *)NULL;
		(void) snpf(Namech, Namechl,
		    "region info error: %s", strerror(errno));
		Namech[Namechl - 1] = '\0';
		enter_nm(Namech);
		if (Lf->sf)
		    link_lfile();
		return;
	    } else if (nb < sizeof(rwpi)) {
		(void) fprintf(stderr,
		    "%s: PID %d: proc_pidinfo(PROC_PIDREGIONPATHINFO);\n",
		    Pn, pid);
		(void) fprintf(stderr,
		    "      too few bytes; expected %ld, got %d\n",
		    sizeof(rwpi), nb);
		Exit(1);
	    }
	    if (rwpi.prp_vip.vip_path[0])
		enter_vn_text(&rwpi.prp_vip, &n);
	    a = rwpi.prp_prinfo.pri_address + rwpi.prp_prinfo.pri_size;
	}
}


#if	DARWINV>=900
/*
 * process_threads() -- process thread information
 */

#define TWD		" twd"          /* per-thread current working directory
					 * fd name */
	
static void
process_threads(pid, n)
	int pid;			/* PID */
	uint32_t n;			/* number of threads */
{
	int i, nb, nt;
/*
 * Make sure a thread buffer has been allocated.
 */
	n += 10;
	if (n > NbThreads) {
	    while (n > NbThreads) {
		NbThreads += THREADS_INCR;
	    }
	    if (!Threads)
		Threads = (uint64_t *)malloc((MALLOC_S)NbThreads);
	    else
		Threads = (uint64_t *)realloc((MALLOC_P *)Threads,
					      (MALLOC_S)NbThreads);
	    if (!Threads) {
		(void) fprintf(stderr,
		    "%s: can't allocate space for %d Threads\n", Pn,
		    (int)(NbThreads / sizeof(int *)));
		Exit(1);
	    }
	}
/*
 * Get thread information for the process.
 */
	nb = proc_pidinfo(pid, PROC_PIDLISTTHREADS, 0, Threads, NbThreads);
	if (nb <= 0) {
	    if (errno == ESRCH) {

	    /*
	     * Quit if no thread information is available for the
	     * process.
	     */
		return;
	    }
	}
	nt = (int)(nb / sizeof(uint64_t));
/*
 * Loop through the threads.
 */
	for (i = 0; i < nt; i++) {
	    uint64_t t;
	    struct proc_threadwithpathinfo tpi;

	    t = Threads[i];
	    nb = proc_pidinfo(pid, PROC_PIDTHREADPATHINFO, t, &tpi,
			      sizeof(tpi));
	    if (nb <= 0) {
		if ((errno == ESRCH) || (errno == EINVAL)) {

		/*
		 * Quit if no more thread information is available for the
		 * process.
		 */
		    return;
		}
	    /*
	     * Warn about all other errors via a NAME column message.
	     */
		alloc_lfile(TWD, -1);
		Cfp = (struct file *)NULL;
		(void) snpf(Namech, Namechl,
		    "thread info error: %s", strerror(errno));
		Namech[Namechl - 1] = '\0';
		enter_nm(Namech);
		if (Lf->sf)
		    link_lfile();
		return;
	    } else if (nb < sizeof(tpi)) {
		(void) fprintf(stderr,
		    "%s: PID %d: proc_pidinfo(PROC_PIDTHREADPATHINFO);\n",
		    Pn, pid);
		(void) fprintf(stderr,
		    "      too few bytes; expected %ld, got %d\n",
		    sizeof(tpi), nb);
		Exit(1);
	    }
	    if (tpi.pvip.vip_path[0]) {
		alloc_lfile(TWD, -1);
		Cfp = (struct file *)NULL;
		(void) enter_vnode_info(&tpi.pvip);
		if (Lf->sf)
		    link_lfile();
	    }
	}
}
#endif	/* DARWINV>=900 */
