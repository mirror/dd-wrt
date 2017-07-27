/*
 * dproc.c - AIX process access functions for lsof
 */


/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dproc.c,v 1.26 2008/10/21 16:14:18 abe Exp $";
#endif


#include "lsof.h"

_PROTOTYPE(static void get_kernel_access,(void));

#if	AIXA<2
_PROTOTYPE(static struct le *getle,(KA_T a, KA_T sid, char **err));
#endif	/* AIXA<2 */

#if	AIXV>=4110
_PROTOTYPE(static void getlenm,(struct le *le, KA_T sid));
#endif	/* AIXV>=4110 */

_PROTOTYPE(static int kreadx,(KA_T addr, char *buf, int len, KA_T sid));

#if	AIXA<2
_PROTOTYPE(static void process_text,(KA_T sid));
#else	/* AIXA>=2 */
_PROTOTYPE(static void getsoinfo,(void));
_PROTOTYPE(static void process_text,(pid_t pid));
#endif	/* AIXA<2 */

#if	defined(SIGDANGER)
# if	defined(HASINTSIGNAL)
_PROTOTYPE(static int lowpgsp,(int sig));
# else	/* !defined(HASINTSIGNAL) */
_PROTOTYPE(static void lowpgsp,(int sig));
# endif	/* defined(HASINTSIGNAL) */
#endif	/* defined(SIGDANGER) */


/*
 * Local definitions
 */

#if	AIXV<4300
#define	PROCINFO	procinfo
#else	/* AIXV>=4300 */
#define	PROCINFO_INCR	256
# if	AIXA<1
#define	FDSINFO		fdsinfo
#define	GETPROCS	getprocs
#define	PROCINFO	procsinfo
# else	/* AIXA>=1 */
#define	FDSINFO		fdsinfo64
#define	GETPROCS	getprocs64
#define	PROCINFO	procentry64

#  if	AIXA>1
/*
 * AIX 5 and greater ia64 loader definitions
 */

#include <sys/ldr.h>

#define	SOHASHBUCKS	128		/* SoHash[] bucket count
					 * MUST BE A POWER OF 2!!! */
#define	SOHASH(d, n)	((((int)(((GET_MIN_DEV(d) & 0x7fffffff) * SOHASHBUCKS) \
			         + n) * 31415) >> 7) & (SOHASHBUCKS - 1))

typedef struct so_hash {
	dev_t dev;			/* device (st_dev) */
	int nlink;			/* link count (st_nlink) */
	char *nm;			/* name (mi_name) */
	INODETYPE node;			/* node number (st_ino) */
	struct so_hash *next;		/* next entry in hash bucket */
	SZOFFTYPE sz;			/* size (st_size) */
} so_hash_t;

so_hash_t **SoHash = (so_hash_t **)NULL;
#  endif	/* AIXA>1 */
# endif	/* AIXA<1 */
#endif	/* AIXV<4300 */

#define	PROCSIZE	sizeof(struct PROCINFO)

/*
 * Create the FDSINFOSIZE definition for allocating FDSINFO space.  (This
 * isn't as straightforward as it might seem, because someone made a bad
 * decision to change the struct fdsinfo* family at AIX 5.2.)
 */

#define	FDSINFOSIZE	sizeof(struct FDSINFO)	/* (If we're lucky.) */

#if	defined(OPEN_SHRT_MAX)
# if	OPEN_SHRT_MAX<OPEN_MAX
#undef	FDSINFOSIZE				/* (We weren't lucky.) */
#define	FDSELEMSIZE	(sizeof(struct FDSINFO)/OPEN_SHRT_MAX)
#define	FDSINFOSIZE	(OPEN_MAX * FDSELEMSIZE)
# endif	/* OPEN_SHRT_MAX<OPEN_MAX */
#endif	/* defined(OPEN_SHRT_MAX) */


#if	AIXV>=4110
/*
 * Loader access definitions for AIX 4.1.1 and above.
 */

#define	LIBNMLN		40			/* maximum library table name
						 * length */

#define	LIBMASK		0xf0000000		/* library table mask */
#define	LIBNMCOMP	0xd0000000		/* library table name has
						 * multiple components */
# if	AIXA<1
#define	RDXMASK		0x0fffffff		/* kreadx() address mask */
# else	/* AIXA>=1 */
#define	RDXMASK		0x0fffffffffffffff	/* kreadx() address mask */
#define	URDXMASK	0x0fffffff00000000	/* upper part of RDXMASK */
# endif	/* AIXA<1 */
#endif	/* AIXV>=4110 */


/*
 * Loader structure definitions.  (AIX doesn't supply ld_data.h.)
 */

struct le {				/* loader entry */

	struct le *next;		/* next entry pointer */

#if	AIXV<4300
	ushort dummy1;
	ushort dummy2;
	uint dummy3;
	struct file *fp;		/* file table entry pointer */

# if	AIXV>=4110
	int ft;				/* file type indicator */
	unsigned dummy4;
	char *dummy5;
	unsigned dummy6;
	char *dummy7[3];
	char *nm;			/* name */
# endif	/* AIXV>=4110 */
#else	/* AIXV>=4300 */
# if	AIXA<2
	uint flags;
	struct file *fp;		/* file table entry pointer */
	char *nm;			/* name */
# else	/* AIXA>=2 */
	KA_T d1[2];
	KA_T nm;			/* name */
	KA_T d2[10];
	struct file *fp;		/* file table entry pointer */
# endif	/* AIXA<2 */
#endif	/* AIXV<4300 */

};


#if	AIXV>=4300
/*
 * The elements of interest from the AIX >= 4.3 loader anchor structure.
 */
struct la {			/* loader anchor */

# if	AIXA<2
    struct le *list;
    struct le *exec;
# else	/* AIXA>=2 */
    KA_T exec;
    KA_T list;
# endif	/* AIXA<2 */
};
#endif	/* AIXV>=4300 */


/*
 * Local static values
 */

static int Np = 0;			/* number of processes */
static struct PROCINFO *P = (struct PROCINFO *)NULL;
					/* the process table */
static struct user *Up;			/* user structure */

#if	AIXV>=4110
# if	AIXA<2
static KA_T Soff;			/* shared library VM offset */
int Soff_stat = 0;			/* Soff-available status */
# endif	/* AIXA<2 */
static KA_T Uo;				/* user area VM offset */
#endif	/* AIXV>=4110 */


/*
 * ckkv() - check kernel version
 */

void
ckkv(d, er, ev, ea)
	char *d;			/* dialect */
	char *er;			/* expected release */
	char *ev;			/* expected version */
	char *ea;			/* expected architecture */
{

#if	defined(HASKERNIDCK)
# if	AIXV<5000

/*
 * Use oslevel below AIX 5.
 */
	int br, p[2], pid;
	char buf[128], *cp;
	struct stat sb;

	if (Fwarn)
	    return;
/*
 * Make sure we can execute OSLEVEL.  If OSLEVEL doesn't exist and the AIX
 * version is below 4.1, return quietly.
 */

#define	OSLEVEL		"oslevel"
#define	OSLEVELPATH	"/usr/bin/oslevel"
	
	if (stat(OSLEVELPATH, &sb)) {

#  if	AIXV<4100
	    if (errno == ENOENT)
		return;
#  endif	/* AIXV<4100 */

	    (void) fprintf(stderr, "%s: can't execute %s: %s\n",
		Pn, OSLEVELPATH, strerror(errno));
	    Exit(1);
	}
	if ((sb.st_mode & (S_IROTH | S_IXOTH)) != (S_IROTH | S_IXOTH)) {
	    (void) fprintf(stderr, "%s: can't execute %s, modes: %o\n",
		Pn, OSLEVELPATH, sb.st_mode);
	    Exit(1);
	}
/*
 * Open a pipe for receiving the version number from OSLEVEL.  Fork a
 * child to run OSLEVEL.  Retrieve the OSLEVEL output.
 */
	if (pipe(p)) {
	    (void) fprintf(stderr, "%s: can't create pipe to: %s\n",
		Pn, OSLEVELPATH);
	    Exit(1);
	}
	if ((pid = fork()) == 0) {
	    (void) close(1);
	    (void) close(2);
	    (void) close(p[0]);
	    dup2(p[1], 1);
	    dup2(p[1], 2);
	    (void) close(p[1]);
	    execl(OSLEVELPATH, OSLEVEL, NULL);
	    _exit(0);
	}
	if (pid < 0) {
	    (void) fprintf(stderr, "%s: can't fork a child for %s: %s\n",
		Pn, OSLEVELPATH, strerror(errno));
	    Exit(1);
	}
	(void) close(p[1]);
	br = read(p[0], buf, sizeof(buf) - 1);
	(void) close(p[0]);
	(void) wait(NULL);
/*
 * Warn if the actual and expected versions don't match.
 */
	if (br > 0) {
	    buf[br] = '\0';
	    if ((cp = strrchr(buf, '\n')))
		*cp = '\0';
	} else
	    (void) snpf(buf, sizeof(buf), "UNKNOWN");
# else	/* AIXV>=5000 */

/*
 * Use uname() for AIX 5 and above.
 */
	char buf[64];
	struct utsname u;

	(void) memset((void *)&u, 0, sizeof(u));
	(void) uname(&u);
	(void) snpf(buf, sizeof(buf) - 1, "%s.%s.0.0", u.version, u.release);
	buf[sizeof(buf) - 1] = '\0';
# endif	/* AIXV<5000 */
	if (!ev || strcmp(buf, ev))
	    (void) fprintf(stderr,
		"%s: WARNING: compiled for %s version %s; this is %s.\n",
		Pn, d, ev ? ev : "UNKNOWN", buf);
#endif	/* defined(HASKERNIDCK) */

}


/*
 * gather_proc_info() - gather process information
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
	KA_T cdir, fp, pdir, rdir;
	char *cmd;
	int hl, i, nf, np;
	struct PROCINFO *p;
	short pss, sf;
	struct user us;

#if	AIXV>=4300
	static struct FDSINFO *fds = (struct FDSINFO *)NULL;
	MALLOC_S msz;
# if	AIXA==1
	pid32_t pid;		/* Since we're operating with types defined
				 * under _KERNEL (see machine.), but
				 * getprocs64() expects application types
				 * (where pid_t is 32 bits), the pid variable
				 * must be cast in an application-compatible
				 * manner.
				 */
# else	/* AIXA!=1 */
	pid_t pid;
# endif	/* AIXA==1 */
# if	AIXV==4330
	static int trx = 0;
	unsigned int mxof;
	static int uo = 0;
# endif	/* AIXV==4330 */
#endif	/* AIXV>=4300 */

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
 * Read the process table.
 */

#if	AIXV<4300
	if (!P) {
	    if (!(P = (struct PROCINFO *)malloc((MALLOC_S)PROCSIZE))) {
		(void) fprintf(stderr,
		    "%s: can't allocate space for 1 proc\n", Pn);
		Exit(1);
	    }
	    Np = 1;
	}
	while (((np = getproc(P, Np, PROCSIZE)) == -1) && errno == ENOSPC) {
	    Np = P->p_pid + 10;
	    if (!(P = (struct PROCINFO *)realloc((MALLOC_P *)P,
					 (size_t)(Np * PROCSIZE))))
	    {
		(void) fprintf(stderr, "%s: no space for %d procinfo's\n",
		    Pn, Np);
		Exit(1);
	    }
	}
#else	/* AIXV>=4300 */
	if (!P) {
	    msz = (MALLOC_S)(PROCSIZE * PROCINFO_INCR);
	    if (!(P = (struct PROCINFO *)malloc(msz))) {
		(void) fprintf(stderr,
		    "%s: can't allocate space for %d procs\n",
		    Pn, PROCINFO_INCR);
		Exit(1);
	    }
	    Np = PROCINFO_INCR;
	}
	np = pid = 0;
	p = P;
	while ((i = GETPROCS(p, PROCSIZE, (struct FDSINFO *)NULL, 0, &pid,
			     PROCINFO_INCR))
	== PROCINFO_INCR) {
	    np += PROCINFO_INCR;
	    if (np >= Np) {
	        msz = (MALLOC_S)(PROCSIZE * (Np + PROCINFO_INCR));
		if (!(P = (struct PROCINFO *)realloc((MALLOC_P *)P, msz))) {
		    (void) fprintf(stderr,
			"%s: no more space for proc storage\n", Pn);
		    Exit(1);
		}
		Np += PROCINFO_INCR;
	    }
	    p = (struct PROCINFO *)((char *)P + (np * PROCSIZE));
	}
	if (i > 0)
	    np += i;
#endif	/* AIXV<4300 */

/*
 * Loop through processes.
 */
	for (p = P, Up = &us; np > 0; np--, p++) {
	    if (p->p_stat == 0 || p->p_stat == SZOMB)
		continue;
	    if (is_proc_excl(p->p_pid, (int)p->p_pgid, (UID_ARG)p->p_uid,
			     &pss, &sf))
		continue;

#if	AIXV<4300
	/*
	 * Get user structure for AIX < 4.3.
	 *
	 * If AIX version is below 4.1.1, use getuser().
	 *
	 * If AIX version is 4.1.1 or above: if readx() is disabled (no -X
	 * option, use  getuser(); if readx() is enabled (-X), use readx().
	 */

# if	AIXV>=4110
	    if (Fxopt
	    &&  kreadx(Uo, (char *)Up, U_SIZE, (KA_T)p->pi_adspace) == 0)
		i = 1;
	    else
		i = 0;
	    if (i == 0) {
		if (getuser(p, PROCSIZE, Up, U_SIZE) != 0)
		    continue;
	    }
	    hl = i;
# else	/* AIXV<4110 */
	    if (getuser(p, PROCSIZE, Up, U_SIZE) != 0)
		continue;
	    hl = 1;
# endif	/* AIXV>=4110 */
	/*
	 * Save directory vnode addresses, command name address, and open file
	 * count from user structure.
	 *
	 * Skip processes excluded by the user structure command name.
	 */
	    cdir = (KA_T)Up->u_cdir;

# if	AIXV<4100
	    pdir = (KA_T)Up->u_pdir;
# endif	/* AIXV<4100 */

	    rdir = (KA_T)Up->u_rdir;
	    cmd = Up->u_comm;
	    nf = Up->u_maxofile;
	    if (is_cmd_excl(cmd, &pss, &sf))
		continue;
	    if (cckreg) {

	    /*
	     * If conditional checking of regular files is enabled, enable
	     * socket file only checking, based on the process' selection
	     * status.
	     */
		ckscko = (sf & SELPROC) ? 0 : 1;
	    }
	    
#else	/* AIXV>=4300 */
	/*
	 * For AIX 4.3 and above, skip processes excluded by the procsinfo
	 * command name.  Use getprocs() to get the file descriptors for
	 * included processes.
	 *
	 * If readx is enabled (-X), use it to get the loader_anchor structure.
	 */
	    if (is_cmd_excl(p->pi_comm, &pss, &sf))
		continue;
	    if (cckreg) {

	    /*
	     * If conditional checking of regular files is enabled, enable
	     * socket file only checking, based on the process' selection
	     * status.
	     */
		ckscko = (sf & SELPROC) ? 0 : 1;
	    }
	    if (!fds) {
		if (!(fds = (struct FDSINFO *)malloc((MALLOC_S)FDSINFOSIZE)))
		{
		    (void) fprintf(stderr,
			"%s: can't allocate fdsinfo struct for PID %d\n",
			Pn, pid);
		    Exit(1);
		}
	    }
	    pid = p->p_pid;
	    if (GETPROCS((struct PROCINFO *)NULL, PROCSIZE, fds, FDSINFOSIZE,
			  &pid, 1)
	    != 1)
		continue;
	    hl = 0;

# if	AIXV==4330
	/*
	 * Handle readx() for AIX 4.3.3 specially, because 4.3.3 was released
	 * with two different user struct definitions in <sys/user.h> and
	 * their form affects using readx() to get the loader table pointers
	 * from U_loader of the user structure (when -X is specified).
	 */
	    if (Fxopt) {
		for (;;) {

		/*
		 * Read the AIX 4.3.3 U_loader pointers.
		 */
		    if (kreadx((KA_T)((char *)Uo
				      + offsetof(struct user, U_loader) + uo),
			       (char *)&Up->U_loader, sizeof(struct la),
			       (KA_T)p->pi_adspace))
			break;
		    if (trx) {
			hl = 1;
			break;
		    }
		/*
		 * Until the correct size of the U_loader offset in lo has been
		 * established, read U_maxofile and match it to pi_maxofile
		 * from the PROCINFO structure.  Try the offsets 0, 48, and
		 * -48.  Note: these offsets are heuristic attempts to adjust
		 * to differences in the user struct as observed on two systems
		 * whose <sys/user.h> header files differed.  U_maxofile 
		 * follows U_loader by the same number of elements in both
		 * user structs, so the U_loader offset should be the same as
		 * the U_maxofile offset.
		 */
		    if (!kreadx((KA_T)((char *)Uo
				      + offsetof(struct user,U_maxofile) + uo),
			        (char *)&mxof, sizeof(mxof),
				(KA_T)p->pi_adspace)
		    && (mxof == p->pi_maxofile))
		    {
			hl = trx = 1;
			break;
		    }
		    if (uo == 0)
			uo = 48;
		    else if (uo == 48)
			uo = -48;
		    else {
			Fxopt = hl = 0;
			trx = 1;
			if (!Fwarn) {
			    (void) fprintf(stderr,
				"%s: WARNING: user struct mismatch;", Pn);
			    (void) fprintf(stderr, " -X option disabled.\n");
			}
			break;
		    }
		}
	    }
# else	/* AIXV!=4330 */
	    if (Fxopt
	    &&  kreadx((KA_T)((char *)Uo + offsetof(struct user, U_loader)),
		       (char *)&Up->U_loader, sizeof(struct la),
		       (KA_T)p->pi_adspace)
	    == 0)
		hl = 1;
# endif	/* AIXV==4330 */

	/*
	 * Save directory vnode addresses, command name, and open file count
	 * from procinfo structure.
	 */
	    cdir = (KA_T)p->pi_cdir;
	    pdir = (KA_T)NULL;
	    rdir = (KA_T)p->pi_rdir;
	    cmd = p->pi_comm;
	    nf = p->pi_maxofile;
#endif	/* AIXV<4300 */

	/*
	 * Allocate a local process structure and start filling it.
	 */
	    alloc_lproc(p->p_pid, (int)p->p_pgid, (int)p->p_ppid,
		(UID_ARG)p->p_uid, cmd, (int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;
	/*
	 * Save current working directory information.
	 */
	    if (!ckscko && cdir) {
		alloc_lfile(CWD, -1);
		process_node(cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (!ckscko && rdir) {
		alloc_lfile(RTD, -1);
		process_node(rdir);
		if (Lf->sf)
		    link_lfile();
	    }

#if	AIXV<4100
	/*
	 * Save parent directory information.
	 */
	    if (!ckscko && pdir) {
		alloc_lfile("  pd", -1);
		process_node(pdir);
		if (Lf->sf)
		    link_lfile();
	    }
#endif	/* AIXV<4100 */

	/*
	 * Save information on text files.
	 */
	    if (!ckscko && hl) {

#if	AIXA<2
# if	AIXA<1
		process_text((KA_T)p->pi_adspace);
# else	/* AIXA==1 */
		{
		    int ck = 1;
		    KA_T sid = (KA_T)p->pi_adspace;

		    if ((Up->U_loader[0] & URDXMASK)
		    ||  (Up->U_loader[1] & URDXMASK))
		    {

		    /*
		     * If the upper part of either loader map address is
		     * non-zero and this is not the lsof process, skip the
		     * processing of text files.  If this is the lsof process,
		     * set the segment address to zero, forcing text file
		     * information to come from kmem rather than mem.
		     */
			if (Mypid == p->p_pid)
			    sid = (KA_T)0;
			else
			    ck = 0;
		    }
		    if (ck)
			process_text(sid);
		}
# endif	/* AIXA<1 */
#else	/* AIXA>=2 */
		process_text(p->p_pid);
#endif	/* AIXA<2 */

	    }
	/*
	 * Save information on file descriptors.
	 */
	    for (i = 0; i < nf; i++) {

#if	AIXV<4300
		fp = (KA_T)Up->u_ufd[i].fp;
#else	/* AIXV>=4300 */
		fp = (KA_T)fds->pi_ufd[i].fp;
#endif	/* AIXV<4300 */

		if (fp) {
		    alloc_lfile((char *)NULL, i);
		    process_file(fp);
		    if (Lf->sf) {

#if	defined(HASFSTRUCT)
			if (Fsv & FSV_FG)

# if	AIXV<4300
			    Lf->pof = (long)(Up->u_ufd[i].flags & 0x7f);
#else	/* AIXV>=4300 */
			    Lf->pof = (long)(fds->pi_ufd[i].flags & 0x7f);
#endif	/* AIXV<4300 */
#endif	/* defined(HASFSTRUCT) */

			link_lfile();
		    }
		}
	    }
	/*
	 * Examine results.
	 */
	    if (examine_lproc())
		return;
	}
}


/*
 * get_kernel_access() - get access to kernel memory
 */

static void
get_kernel_access()
{
	int oe = 0;

#if	defined(AIX_KERNBITS)
	int kb;
	char *kbb, *kbr;
/*
 * Check the kernel bit size against the size for which this executable was
 * configured.
 */
	if (__KERNEL_32()) {
	    kb = 32;
	    kbr = "32";
	} else if (__KERNEL_64()) {
	    kb = 64;
	    kbr = "64";
	} else {
	    kb = 0;
	    kbr = "unknown";
	}
	if ((AIX_KERNBITS == 0) || !kb || (kb != AIX_KERNBITS)) {
	    if (AIX_KERNBITS == 32)
		kbb = "32";
	    else if (AIX_KERNBITS == 64)
		kbb = "64";
	    else
		kbb = "unknown";
	    (void) fprintf(stderr,
		"%s: FATAL: compiled for a kernel of %s bit size.\n", Pn, kbb);
	    (void) fprintf(stderr,
		"      The bit size of this kernel is %s.\n", kbr);
	    Exit(1);
	}
#endif	/* defined(AIX_KERNBITS) */

/*
 * Access /dev/mem.
 */
	if ((Km = open("/dev/mem", O_RDONLY, 0)) < 0) {
	    (void) fprintf(stderr, "%s: can't open /dev/mem: %s\n",
		Pn, strerror(errno));
	    oe++;
	}

#if	defined(WILLDROPGID)
/*
 * If kernel memory isn't coming from KMEM, drop setgid permission
 * before attempting to open the (Memory) file.
 */
	if (Memory)
	    (void) dropgid();
#else	/* !defined(WILLDROPGID) */
/*
 * See if the non-KMEM memory file is readable.
 */
	if (Memory && !is_readable(Memory, 1))
	    Exit(1);
#endif	/* defined(WILLDROPGID) */

/*
 * Access kernel memory file.
 */
	if ((Kd = open(Memory ? Memory : KMEM, O_RDONLY, 0)) < 0) {
	    (void) fprintf(stderr, "%s: can't open %s: %s\n", Pn,
		Memory ? Memory : KMEM, strerror(errno));
	    oe++;
	}
	if (oe)
	    Exit(1);

#if	defined(WILLDROPGID)
/*
 * Drop setgid permission, if necessary.
 */
	if (!Memory)
	    (void) dropgid();
#endif	/* defined(WILLDROPGID) */

/*
 * Get kernel symbols.
 */
	if (knlist(Nl, X_NL_NUM, sizeof(struct nlist)) || !Nl[X_UADDR].n_value)
	{
	    (void) fprintf(stderr, "%s: can't get kernel's %s address\n",
		Pn, Nl[X_UADDR].n_name);
	    Exit(1);
	}

#if	defined(HAS_AFS)
	(void) knlist(AFSnl, X_AFSNL_NUM, sizeof(struct nlist));
#endif	/* defined(HAS_AFS) */

#if	AIXV>=4110
/*
 * Get user area and shared library VM offsets for AIX 4.1.1 and above.
 */
	if (Fxopt) {
	    struct ublock *ub;

# if	AIXA<2
	    struct nlist ll[] = {
		{ "library_anchor"	},

#  if	AIXV>=4330
		{ "library_le_handle"	},
#  else	/* AIXV<4330 */
		{ "library_data_handle"	},
#  endif	/* AIXV>=4330 */

		{ (char *)NULL		}
	    };

	    if (nlist(N_UNIX, ll) == 0
	    &&  ll[0].n_value != (long)0 && ll[1].n_value != (long)0
	    &&  kreadx((KA_T)(ll[1].n_value & RDXMASK), (char *)&Soff,
			sizeof(Soff), (KA_T)0)
	    == 0)
		Soff_stat++;
# endif	/* AIXA<2 */

	    ub = (struct ublock *)Nl[X_UADDR].n_value;
	    Uo = (KA_T)((KA_T)&ub->ub_user & RDXMASK);
	}
#endif	/* AIXV>=4110 */

/*
 * Check the kernel version number.
 */
	(void) ckkv("AIX", (char *)NULL, LSOF_VSTR, (char *)NULL);

#if	defined(SIGDANGER)
/*
 * If SIGDANGER is defined, enable its handler.
 */
	(void) signal(SIGDANGER, lowpgsp);
#endif	/* defined(SIGDANGER) */

}


#if	AIXA<2
/*
 * getle() - get loader entry structure
 */

static struct le *
getle(a, sid, err)
	KA_T a;				/* loader entry kernel address */
	KA_T sid;			/* user structure segment ID */
	char **err;			/* error message (if return is NULL) */
{
	static struct le le;

#if	AIXV<4110
	if (a < Nl[X_UADDR].n_value) {
	    *err = "address too small";
	    return((struct le *)NULL);
	}
	if (((char *)a + sizeof(le)) <= ((char *)Nl[X_UADDR].n_value + U_SIZE))
	    return((struct le *)((char *)Up + a - Nl[X_UADDR].n_value));
#endif	/* AIXV<4110 */

	if (!Fxopt) {
	    *err = "readx() disabled for Stale Segment ID bug (see -X)";
	    return((struct le *)NULL);
	}

#if	AIXV>=4110
	if (!sid) {
	    if (!kread(a, (char *)&le, sizeof(le)))
		return(&le);
	} else {
	    if (!kreadx((KA_T)(a & RDXMASK),(char *)&le,sizeof(le),(KA_T)sid))
		return(&le);
	}
#else	/* AIXV<4110 */
	if (!kreadx((KA_T)a, (char *)&le, sizeof(le), (KA_T)sid))
	    return(&le);
#endif	/* AIXV>=4110 */

getle_err:

	*err = "can't readx()";
	return((struct le *)NULL);
}
#endif	/* AIXA<2 */


#if	AIXV>=4110
/*
 * getlenm() - get loader entry file name for AIX >= 4.1.1
 */

static void
getlenm(le, sid)
	struct le *le;			/* loader entry structure */
	KA_T sid;			/* segment ID */
{
	char buf[LIBNMLN];
	int i;

# if	AIXV<4300
	if ((le->ft & LIBMASK) != LIBNMCOMP)
	    return;
#else	/* AIXV>=4300 */
# if	AIXA<2
	if (!sid) {
	     if (kread((KA_T)le->nm, buf, LIBNMLN))
		return;
	} else {
	    if (!Soff_stat || !le->nm
	    ||  kreadx((KA_T)le->nm & (KA_T)RDXMASK, buf, LIBNMLN, (KA_T)Soff))
		return;
	}
	buf[LIBNMLN - 1] = '\0';
	i = strlen(buf);
	if (i < (LIBNMLN - 3) && buf[i+1])
	    enter_nm(&buf[i+1]);
	else if (buf[0])
	    enter_nm(buf);
# else	/* AIXA>=2 */
	if (!le->nm || kread(le->nm, buf, sizeof(buf)))
	    return;
	buf[LIBNMLN - 1] = '\0';
	if (!strlen(buf))
	    return;
	enter_nm(buf);
# endif	/* AIXA<2 */
#endif	/* AIXV<4300 */

}
#endif	/* AIXV>=4110 */


#if	AIXA>1
/*
 * getsoinfo() - get *.so information for ia64 AIX >= 5
 */

static void
getsoinfo()
{
	char buf[65536];
	uint bufsz = (uint) sizeof(buf);
	int ct, h;
	char *ln = (char *)NULL;
	char *rn = (char *)NULL;
	LDR_Mod_info_t *lp;
	struct stat sb;
	so_hash_t *sp;
/*
 * See if loader information is needed.  Warn if this process has insufficient
 * permission to acquire it from all processes.
 */
	if (!Fxopt)
	    return;
	if ((Myuid != 0) && !Setuidroot && !Fwarn) {
	    (void) fprintf(stderr,
		"%s: WARNING: insufficient permission to access all", Pn);
	    (void) fprintf(stderr, " /%s/object sub-\n", HASPROCFS);
	    (void) fprintf(stderr,
		    "      directories; some loader information may", Pn);
	    (void) fprintf(stderr, " be unavailable.\n");
	}
/*
 * Get the loader module table.  Allocate hash space for it.
 */
	if ((ct = ldr_get_modules(SOL_GLOBAL, (void *)buf, &bufsz)) < 1)
	    return;
	if (!(SoHash = (so_hash_t **)calloc((MALLOC_S)SOHASHBUCKS,
					    sizeof(so_hash_t *))))
	{
	    (void) fprintf(stderr, "%s: no space for *.so hash buckets\n", Pn);
	    Exit(1);
	}
/*
 * Cache the loader module information, complete with stat(2) results.
 */
	for (lp = (LDR_Mod_info_t *)buf; ct; ct--, lp++) {

	/*
	 * Release previous name space allocations.
	 */
	    if (ln) {
		(void) free((MALLOC_P *)ln);
		ln = (char *)NULL;
	    }
	    if (rn) {
		(void) free((MALLOC_P *)rn);
		rn = (char *)NULL;
	    }
	/*
	 * Make a copy of the loader module name.
	 */
	    if (!(rn = mkstrcpy(lp->mi_name, (MALLOC_S *)NULL))) {
		(void) fprintf(stderr, "%s: no space for name: %s\n", Pn,
		    lp->mi_name);
		Exit(1);
	    }
	/*
	 * Resolve symbolic links.
	 */
	    ln = Readlink(rn);
	    if (ln == rn)
		rn = (char *)NULL;
	/*
	 * Get stat(2) information.
	 */
	    if (statsafely(ln, &sb)) {
		if (!Fwarn)
		    (void) fprintf(stderr, "%s: WARNING: can't stat: %s\n",
			Pn, ln);
		    continue;
	    }
	/*
	 * Allocate and fill a loader information hash structure.
	 */
	    if (!(sp = (so_hash_t *)malloc((MALLOC_S)sizeof(so_hash_t)))) {
		(void) fprintf(stderr, "%s: no space for *.so hash entry: %s\n",
		    Pn, ln);
		Exit(1);
	    }
	    sp->dev = sb.st_dev;
	    sp->nlink = (int)sb.st_nlink;
	    sp->nm = ln;
	    ln = (char *)NULL;
	    sp->node = (INODETYPE)sb.st_ino;
	    sp->sz = (SZOFFTYPE)sb.st_size;
	/*
	 * Link the structure to the appropriate hash bucket.
	 */
	    h = SOHASH(sb.st_dev, (INODETYPE)sb.st_ino);
	    if (SoHash[h])
		sp->next = SoHash[h];
	    else
		sp->next = (so_hash_t *)NULL;
	    SoHash[h] = sp;
	}
/*
 * Free any unused name space that was allocated.
 */
	if (ln)
	    (void) free((MALLOC_P *)ln);
	if (rn)
	    (void) free((MALLOC_P *)rn);
}
#endif	/* AIXA>1 */


/*
 * initialize() - perform all initialization
 */

void
initialize()
{
	get_kernel_access();

#if	AIXA>1
	(void) getsoinfo();
#endif	/* AIXA>1 */

}


/*
 * kread() - read from kernel memory
 */

int
kread(addr, buf, len)
	KA_T addr;			/* kernel memory address */
	char *buf;			/* buffer to receive data */
	READLEN_T len;			/* length to read */
{
	int br;

#if	AIXV<4200
	if (lseek(Kd, (off_t)addr, L_SET) == (off_t)-1)
#else	/* AIXV>=4200 */
	if (lseek64(Kd, (off64_t)addr, L_SET) == (off64_t)-1)
#endif	/* AIXV<4200 */

	    return(1);
	br = read(Kd, buf, len);
	return((br == len) ? 0 : 1);
}


/*
 * kreadx() - read kernel segmented memory
 */

int
kreadx(addr, buf, len, sid)
	KA_T addr;			/* kernel address */
	char *buf;			/* destination buffer */
	int len;			/* length */
	KA_T sid;			/* segment ID */
{
	int br;

#if	AIXV<4200
	if (lseek(Km, addr, L_SET) == (off_t)-1)
#else	/* AIXV>=4200 */
	if (lseek64(Km, (off64_t)addr, L_SET) == (off64_t)-1)
#endif	/* AIXV<4200 */

	    return(1);
	br = readx(Km, buf, len, sid);
	return (br == len ? 0 : 1);
}


#if	defined(SIGDANGER)
/*
 * lowpgsp() - hangle a SIGDANGER signal about low paging space
*/

# if	defined(HASINTSIGNAL)
static int
# else	/* !defined(HASINTSIGNAL) */
static void
# endif	/* defined(HASINTSIGNAL) */

lowpgsp(sig)
	int sig;
{
	(void) fprintf(stderr, "%s: FATAL: system paging space is low.\n", Pn);
	Exit(1);
}
#endif	/* defined(SIGDANGER) */


#if	AIXA<2
/*
 * process_text() - process text file information for non-ia64 AIX
 */

static void
process_text(sid)
	KA_T sid;			/* user area segment ID */
{
	char *err, fd[8];
	static struct file **f = (struct file **)NULL;
	int i, j, n;
	struct le *le;
	KA_T ll;
	MALLOC_S msz;
	static MALLOC_S nf = 0;
	struct file *xf = (struct file *)NULL;

#if	AIXV>=4300
	struct la *la = (struct la *)&Up->U_loader;
#endif	/* AIXV>=4300 */

/*
 * Display information on the exec'd entry.
 */

#if	AIXV<4300
	if ((ll = (KA_T)Up->u_loader[1]))
#else	/* AIXV>=4300 */
	if ((ll = (KA_T)la->exec))
#endif	/* AIXV<4300 */

	{
	    alloc_lfile(" txt", -1);
	    if ((le = getle(ll, sid, &err))) {
		if ((xf = le->fp)) {
		    process_file((KA_T)xf);
		    if (Lf->sf) {

#if	AIXV>=4110 && AIXV<4300
			if (!Lf->nm || !Lf->nm[0])
			    getlenm(le, sid);
#endif	/* AIXV>=4110 && AIXV<4300 */

			link_lfile();
		    }
		}
	    } else {
		(void) snpf(Namech, Namechl, "text entry at %s: %s",
		    print_kptr((KA_T)ll, (char *)NULL, 0), err);
		enter_nm(Namech);
		if (Lf->sf)
		    link_lfile();
	    }
	}
/*
 * Display the loader list.
 */
	for (i = n = 0,

#if	AIXV<4300
	     ll = (KA_T)Up->u_loader[0];
#else	/* AIXV>=4300 */
	     ll = (KA_T)la->list;
#endif	/* AIXV<4300 */

	     ll;
	     ll = (KA_T)le->next)
	{
	    (void) snpf(fd, sizeof(fd), " L%02d", i);
	    alloc_lfile(fd, -1);
	    if (!(le = getle(ll, sid, &err))) {
		(void) snpf(Namech, Namechl, "loader entry at %s: %s",
		    print_kptr((KA_T)ll, (char *)NULL, 0), err);
		enter_nm(Namech);
		if (Lf->sf)
		    link_lfile();
		return;
	    }
	/*
	 * Skip entries with no file pointers, the exec'd file, and entries
	 * that have already been processed.
	 */
	    if (!le->fp || (le->fp == xf))
		continue;
	    for (j = 0; j < n; j++) {
		if (f[j] == le->fp)
		    break;
	    }
	    if (j < n)
		continue;
	    if (n >= nf) {

	    /*
	     * Allocate file structure address cache space.
	     */
		nf += 10;
		msz = (MALLOC_S)(nf * sizeof(struct file *));
		if (f)
		    f = (struct file **)realloc((MALLOC_P *)f, msz);
		else
		    f = (struct file **)malloc(msz);
		if (!f) {
		    (void) fprintf(stderr,
			"%s: no space for text file pointers\n", Pn);
		    Exit(1);
		}
	    }
	    f[n++] = le->fp;
	/*
	 * Save the loader entry.
	 */
	    process_file((KA_T)le->fp);
	    if (Lf->sf) {

#if	AIXV>=4110
		if (!Lf->nm || !Lf->nm[0])
		    getlenm(le, sid);
#endif	/* AIXV>=4110 */

		link_lfile();
		i++;
	    }
	}
}
#else	/* AIXA>=2 */
/*
 * process_text() - process text file information for ia64 AIX >= 5
 */

static void
process_text(pid)
	pid_t pid;			/* process PID */
{
	char buf[MAXPATHLEN+1], fd[8], *nm, *pp;
	size_t bufl = sizeof(buf);
	DIR *dfp;
	struct dirent *dp;
	int i;
	struct la *la = (struct la *)&Up->U_loader;
	struct le le;
	struct lfile *lf;
	struct stat sb;
	so_hash_t *sp;
	size_t sz;
	dev_t xdev;
	INODETYPE xnode;
	int xs = 0;
/*
 * Display information on the exec'd entry.
 */
	if (la->exec && !kread((KA_T)la->exec, (char *)&le, sizeof(le))
	&&  le.fp) {
	    alloc_lfile(" txt", -1);
	    process_file((KA_T)le.fp);
	    if (Lf->dev_def && (Lf->inp_ty == 1)) {
		xdev = Lf->dev;
		xnode = Lf->inode;
		xs = 1;
	    }
	    if (Lf->sf) {
		if (!Lf->nm || !Lf->nm[0])
		    getlenm(&le, (KA_T)0);
		link_lfile();
	    }
	}
/*
 * Collect devices and names for the entries in /HASPROCFS/PID/object -- the
 * AIX 5 loader list equivalent.  When things fail in this processing -- most
 * likely for insufficient permissions -- be silent; a warning was issued by
 * getsoinfo().
 */
	(void) snpf(buf, bufl, "/%s/%ld/object", HASPROCFS, (long)pid);
	if (!(dfp = opendir(buf)))
	    return;
	if ((sz = strlen(buf)) >= bufl)
	   return;
	buf[sz++] = '/';
	pp = &buf[sz];
	sz = bufl - sz;
/*
 * Read the entries in the /HASPROCFS/PID/object subdirectory.
 */
	for (dp = readdir(dfp), i = 0; dp; dp = readdir(dfp)) {

	/*
	 * Skip '.', "..", entries with no node number, and entries whose
	 * names are too long.
	 */
	    if (!dp->d_ino || (dp->d_name[0] == '.'))
		continue;
	    if ((dp->d_namlen + 1) >= sz)
		continue;
	    (void) strncpy(pp, dp->d_name, dp->d_namlen);
	    pp[dp->d_namlen] = '\0';
	/*
	 * Get stat(2) information.
	 */
	    if (statsafely(buf, &sb))
		continue;
	/*
	 * Ignore the exec'd and non-regular files.
	 */
	    if (xs && (xdev == sb.st_dev) && (xnode == (INODETYPE)sb.st_ino))
		continue;
	    if (!S_ISREG(sb.st_mode))
		continue;
	/*
	 * Allocate space for a file entry.  Set its basic characteristics.
	 */
	    (void) snpf(fd, sizeof(fd), "L%02d", i++);
	    alloc_lfile(fd, -1);
	    Lf->dev_def = Lf->inp_ty = Lf->nlink_def = Lf->sz_def = 1;
	    Lf->dev = sb.st_dev;
	    Lf->inode = (INODETYPE)sb.st_ino;
	    (void) snpf(Lf->type, sizeof(Lf->type), "VREG");
	/*
	 * Look for a match on device and node numbers in the *.so cache.
	 */
	    for (sp = SoHash[SOHASH(sb.st_dev, (INODETYPE)sb.st_ino)];
		 sp;
		 sp = sp->next)
	    {
		if ((sp->dev == sb.st_dev)
		&&  (sp->node == (INODETYPE)sb.st_ino))
		{

		/*
		 * A match was found; use its name, link count, and size.
		 */
		    nm = sp->nm;
		    Lf->nlink = sp->nlink;
		    Lf->sz = sp->sz;
		    break;
		}
	    }
	    if (!sp) {

	    /*
	     * No match was found; use the /HASPROCFS/object name, its link
	     * count, and its size.
	     */
		nm = pp;
		Lf->nlink_def = sb.st_nlink;
		Lf->sz = sb.st_size;
	    }
	/*
	 * Do selection tests: NFS; link count; file name; and file system.
	 */

# if	defined(HAS_NFS)
	    if (Fnfs && (GET_MIN_DEV(Lf->dev_def) & SDEV_REMOTE))
		Lf->sf |= SELNFS;
# endif	/* defined(HAS_NFS) */

	    if (Nlink && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	    if (Sfile && is_file_named(NULL, VREG, 0, 0))
		Lf->sf |= SELNM;
	    if (Lf->sf) {

	    /*
	     * If the file was selected, enter its name and link it to the
	     * other files of the process.
	     */
		enter_nm(nm);
		link_lfile();
	    }
	}
	(void) closedir(dfp);
}
#endif	/* AIXA<2 */
