/*
 * dproc.c - SCO UnixWare process access functions for lsof
 */


/*
 * Copyright 1996 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1996 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dproc.c,v 1.14 2002/10/08 20:18:34 abe Exp $";
#endif

#include "lsof.h"


/*
 * Local static values
 */

static int Np;				/* occupied P[] count */
static int Npa = 0;			/* allocated P[] count */
static struct proc *P = (struct proc *)NULL;
					/* proc table */
static KA_T Pract;			/* kernel's practive address */
static KA_T Sgdnops;			/* kernel's segdev_ops address */
static KA_T Sgvnops;			/* kernel's segvn_ops address */
static struct var Var;			/* kernel variables */


/*
 * Local definitions
 */

#define	PROCINCR	32		/* increment for increasing P[] */


/*
 * Local function prototypes.
 */

_PROTOTYPE(static int get_clonemaj,(void));
_PROTOTYPE(static void read_proc,(void));
_PROTOTYPE(static void get_kernel_access,(void));
_PROTOTYPE(static void readfsinfo,(void));
_PROTOTYPE(static void process_text,(KA_T pa));


/*
 * gather_proc_info() -- gather process information
 */

void
gather_proc_info()
{
	struct cred cr;
	struct execinfo ex;
	static struct fd_entry *fe;
	struct fd_entry *f;
	KA_T fa;
	int i, nf;
	MALLOC_S len;
	static int nfea = 0;
	struct proc *p;
	int pgid, pid, px;
	struct pid pids;
	short pss, sf;
	uid_t uid;

#if	UNIXWAREV>=70103
	struct pollx plx;
#endif	/* UNIXWAREV>=70103 */

/*
 * Examine proc structures and their associated information.
 */
	(void) read_proc();
	for (p = P, px = 0; px < Np; p++, px++) {
	    if ((p->p_flag & P_DESTROY) || (p->p_flag & P_GONE)
	    ||  !p->p_pidp

#if	!defined(HAS_P_PGID)
	    || !p->p_pgidp
#endif	/* !defined(HAS_P_PGID) */

	    || !p->p_cred || !p->p_execinfo)
		continue;
	/*
	 * Get Process ID, Process group ID, and User ID.
	 */
	    if (!p->p_pidp
	    ||  kread((KA_T)p->p_pidp, (char *)&pids, sizeof(pids)))
		continue;
	    pid = (int)pids.pid_id;

#if	defined(HAS_P_PGID)
	    pgid = (int)p->p_pgid;
#else	/* !defined(HAS_P_PGID) */
	    if (!p->p_pgidp
	    ||  kread((KA_T)p->p_pgidp, (char *)&pids, sizeof(pids)))
		continue;
	    pgid = (int)pids.pid_id;
#endif	/* defined(HAS_P_PGID) */

	    if (!p->p_cred
	    ||  kread((KA_T)p->p_cred, (char *)&cr, sizeof(cr)))
		continue;
	    uid = cr.cr_uid;
	    if (is_proc_excl(pid, pgid, (UID_ARG)uid, &pss, &sf))
		continue;
	/*
	 * Get the execution information -- for the command name.
	 */
	    if (!p->p_execinfo
	    ||  kread((KA_T)p->p_execinfo, (char *)&ex, sizeof(ex)))
		continue;
	/*
	 * Allocate a local process structure.
	 */
	    if (is_cmd_excl(ex.ei_comm, &pss, &sf))
		continue;
	    alloc_lproc(pid, pgid, (int)p->p_ppid, (UID_ARG)uid, ex.ei_comm,
		(int)pss, (int)sf);
	    Plf = NULL;
	/*
	 * Save current working directory information.
	 */
	    if (p->p_cdir) {
		alloc_lfile(CWD, -1);
		process_node((KA_T)p->p_cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (p->p_rdir) {
		alloc_lfile(RTD, -1);
		process_node((KA_T)p->p_rdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Print information on the text file.
	 */
	    if (Sgvnops && p->p_as)
		process_text((KA_T)p->p_as);
	/*
	 * Save information on file descriptors.
	 */
	    if (!p->p_fdtab.fdt_entrytab || (nf = p->p_fdtab.fdt_sizeused) < 1)
		continue;
	    len = (MALLOC_S)(nf * sizeof(struct fd_entry));
	    if (nf > nfea) {
		if (fe)
		    fe = (struct fd_entry *)realloc((MALLOC_P *)fe, len);
		else
		    fe = (struct fd_entry *)malloc(len);
		if (!fe) {
		    (void) fprintf(stderr,
			"%s: PID %d; no space for %d file descriptors\n",
			Pn, pid, nf);
		    Exit(1);
		}
		nfea = nf;
	    }
	    if (kread((KA_T)p->p_fdtab.fdt_entrytab, (char *)fe, len))
		continue;
	    for (f = fe, i = 0; i < nf; f++, i++) {
		if ((fa = (KA_T)f->fd_file) && (f->fd_status & FD_INUSE)) {

#if	UNIXWAREV>=70103
		    if (f->fd_flag & FPOLLED) {
			if (kread(fa, (char *)&plx, sizeof(plx))
			||  !(fa = (KA_T)plx.px_fp))
			    continue;
		    }
#endif	/* UNIXWAREV>=70103 */

		    alloc_lfile(NULL, i);
		    process_file(fa);
		    if (Lf->sf) {

#if	defined(HASFSTRUCT)
			if (Fsv & FSV_FG)
			    Lf->pof = (long)f->fd_flag;
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
 * get_clonemaj() - get clone major device number
 */

static int
get_clonemaj()
{
	KA_T v;

#if	UNIXWAREV<70000
	char buf[32];
	struct cdevsw *c, *cd;
	int i, sz;
	MALLOC_S len;
	int rv = 0;
/*
 * Read the cdevsw[] size and allocate temporary space for it.
 */
	if (get_Nl_value("ncdev", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&sz, sizeof(sz)) || !sz)
	    return(rv);
	len = (MALLOC_S)(sz * sizeof(struct cdevsw));
	if (!(cd = (struct cdevsw *)malloc(len))) {
	    (void) fprintf(stderr, "%s: can't allocate %d bytes for cdevsw\n",
		Pn);
	    Exit(1);
	}
/*
 * Read the cdevsw[] from kernel memory.
 */
	if (get_Nl_value("cdev", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)cd, (int)len)) {
	    (void) free((MALLOC_P *)cd);
	    return(rv);
	}
/*
 * Scan the cdevsw[], reading it's names, looking for "clone".
 * Record its cdevsw[] index (i.e., major device number).
 */
	len = sizeof(buf) - 1;
	buf[len] = '\0';
	for (c = cd, i = 0; i < sz; c++, i++) {
	    if (!c->d_name
	    ||  kread((KA_T)c->d_name, buf, len)
	    ||  strcmp(buf, "clone") != 0)
		continue;
	    CloneMaj = i;
	    HaveCloneMaj = rv = 1;
	    break;
	}
	(void) free((MALLOC_P *)cd);
	return(rv);
#else	/* UNIXWAREV>=70000 */
/*
 * At UnixWare 7 the clone major device is found in the kernel's
 * clonemajor variable.
 */
	if (get_Nl_value("cmaj", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&CloneMaj, sizeof(CloneMaj)))
	    return(0);
	return((HaveCloneMaj = 1));
#endif	/* UNIXWAREV<70000 */

}


/*
 * get_kernel_access() - get access to kernel memory
 */

static void
get_kernel_access()
{
	KA_T v;
/*
 * Check kernel version.
 */
	(void) ckkv("UW", (char *)NULL, LSOF_VSTR, (char *)NULL);

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
 * Open kernel memory access.
 */
	if ((Kd = open(Memory ? Memory : KMEM, O_RDONLY, 0)) < 0) {
	    (void) fprintf(stderr, "%s: can't open %s: %s\n", Pn,
		Memory ? Memory : KMEM, strerror(errno));
	    Exit(1);
	}

#if	defined(WILLDROPGID)
/*
 * Drop setgid permission, if necessary.
 */
	if (!Memory)
	    (void) dropgid();
#else	/* !defined(WILLDROPGID) */
/*
 * See if the name list file is readable.
 */
	if (Nmlst && !is_readable(Nmlst, 1))
	    Exit(1);
#endif	/* defined(WILLDROPGID) */

/*
 * Access kernel symbols and values.
 */
	(void) build_Nl(Drive_Nl);
        if (nlist(Nmlst ? Nmlst : N_UNIX, Nl) < 0) {
	    (void) fprintf(stderr, "%s: can't read kernel name list from %s\n",
		Pn, Nmlst ? Nmlst : N_UNIX);
	    Exit(1);
	}
	if (get_Nl_value("var", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&Var, sizeof(Var))) {
	    (void) fprintf(stderr,
		"%s: can't read system configuration info\n", Pn);
	    Exit(1);
	}
	if (get_Nl_value("proc", Drive_Nl, &Pract) < 0 || !Pract) {
	    (void) fprintf(stderr,
		"%s: can't find active process chain pointer\n", Pn);
	    Exit(1);
	}
	if (get_Nl_value("sgdnops", Drive_Nl, &Sgdnops) < 0 || !Sgdnops)
	    Sgdnops = (unsigned long)0;
	if (get_Nl_value("sgvnops", Drive_Nl, &Sgvnops) < 0 || !Sgvnops)
	    Sgvnops = (unsigned long)0;
/*
 * Identify the clone major device number.
 */
	if (!get_clonemaj()) {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING; can't identify major clone device number\n",
		    Pn);
	}
}


/*
 * initialize() - perform all initialization
 */

void
initialize()
{
	get_kernel_access();
	readfsinfo();
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
	READLEN_T br;

#if	UNIXWAREV<7000
	if (lseek(Kd, (long)addr, L_SET) == (long)-1L)
		return(-1);
	br = (READLEN_T) read(Kd, buf, len);
#else	/* UNIXWAREV>=7000 */
	br = (READLEN_T) pread(Kd, buf, len, (off_t)addr);
#endif	/* UNIXWAREV<7000 */

	return((br == len) ? 0 : 1);
}


/*
 * process_text() - process text access information
 */

static void
process_text(pa)
	KA_T pa;			/* kernel address space description
					 * pointer */
{
	struct as as;
	struct segdev_data dv;
	char *fd;
	int i, j, k, l;
	struct seg s;
	KA_T v[MAXSEGS];
	struct segvn_data vn;
	KA_T vp;
/*
 * Get address space description.
 */
	if (kread(pa, (char *)&as, sizeof(as)))
	    return;
/*
 * Loop through the segments.  The loop should stop when the segment
 * pointer returns to its starting point, but just in case, it's stopped
 * when MAXSEGS have been recorded or 2*MAXSEGS have been examined.
 */
	s.s_next = as.a_segs;
	for (i = j = k = 0; i < MAXSEGS && j < 2*MAXSEGS; j++) {
	    if (!s.s_next || kread((KA_T)s.s_next, (char *)&s, sizeof(s)))
		break;
	    fd = (char *)NULL;
	    vp = (KA_T)NULL;
	    if (Sgvnops == (KA_T)s.s_ops && s.s_data) {

	    /*
	     * Process a virtual node segment.
	     */
		if (kread((KA_T)s.s_data, (char *)&vn, sizeof(vn)))
		    break;
		if ((vp = (KA_T)vn.svd_vp)) {
		    if ((vn.svd_flags & SEGVN_PGPROT)
		    ||  (vn.svd_prot & PROT_EXEC))
			fd = " txt";
		    else
			fd = " mem";
		}
	    } else if (Sgdnops == (KA_T)s.s_ops && s.s_data) {

	    /*
	     * Process a special device segment.
	     */
		if (kread((KA_T)s.s_data, (char *)&dv, sizeof(dv)))
		    break;
		if ((vp = (KA_T)dv.vp))
		    fd = "mmap";
	    }
	    if (fd && vp) {

	    /*
	     * Process the vnode pointer.  First make sure it's unique.
	     */
		for (l = 0; l < k; l++) {
		    if (v[l] == vp)
			break;
		}
		if (l >= k) {
		    alloc_lfile(fd, -1);
		    process_node(vp);
		    if (Lf->sf) {
			link_lfile();
			i++;
		    }
		}
		v[k++] = vp;
	    }
	/*
	 * Follow the segment link to the starting point in the address
	 * space description.  (The i and j counters place an absolute
	 * limit on the loop.)
	 */
	    if (s.s_next == as.a_segs)
		break;
	}
}


/*
 * readfsinfo() - read file system information
 */

static void
readfsinfo()
{
	char buf[FSTYPSZ+1];
	int i, len;

	if ((Fsinfomax = sysfs(GETNFSTYP)) == -1) {
	    (void) fprintf(stderr, "%s: sysfs(GETNFSTYP) error: %s\n",
		Pn, strerror(errno));
	    Exit(1);
	} 
	if (Fsinfomax == 0)
	    return;
	if (!(Fsinfo = (char **)malloc((MALLOC_S)(Fsinfomax * sizeof(char *)))))
	{
	    (void) fprintf(stderr, "%s: no space for sysfs info\n", Pn);
	    Exit(1);
	}
	for (i = 1; i <= Fsinfomax; i++) {
	    if (sysfs(GETFSTYP, i, buf) == -1) {
		(void) fprintf(stderr, "%s: sysfs(GETFSTYP) error: %s\n",
		    Pn, strerror(errno));
		Exit(1);
	    }
	    buf[FSTYPSZ] = '\0';
	    len = strlen(buf) + 1;
	    if (!(Fsinfo[i-1] = (char *)malloc((MALLOC_S)len))) {
		(void) fprintf(stderr,
		    "%s: no space for file system entry %s\n", Pn, buf);
		Exit(1);
	    }
	    (void) snpf(Fsinfo[i-1], len, "%s", buf);
	}
}


/*
 * read_proc() - read the process table
 */

static void
read_proc()
{
	MALLOC_S len;
	struct proc *p;
	KA_T pa;
	char tbuf[32];
	int try;

	if (!P) {

	/*
	 * Allocate initial space for local proc table.
	 */
	    if ((Npa = Var.v_proc) < 1) {
		(void) fprintf(stderr, "%s: bad proc table size: %d\n",
		    Pn, Var.v_proc);
		Exit(1);
	    }
	    Npa += PROCINCR;
	    len = (MALLOC_S)(Npa * sizeof(struct proc));
	    if (!(P = (struct proc *)malloc(len))) {
		(void) fprintf(stderr, "%s: no space for %d proc structures\n",
		    Pn, Npa);
		Exit(1);
	    }
	}
/*
 * Scan the active process chain.
 */
	for (try = 0; try < PROCTRYLM; try++) {

	/*
	 * Read the active process chain head.
	 */
	    pa = (KA_T)NULL;
	    if (!Pract || kread((KA_T)Pract, (char *)&pa, sizeof(pa)) || !pa) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: active proc chain ptr err; addr=%s, val=%s\n",
			Pn, print_kptr(Pract, tbuf, sizeof(tbuf)),
			print_kptr(pa, (char *)NULL, 0));
		continue;
	    }
	/*
	 * Follow the active process chain, accumulating proc structures.
	 */
	    for (Np = 0, p = P; pa;) {
		if (Np >= Npa) {

		/*
		 * Allocate more proc table space.
		 */
		    Npa += PROCINCR;
		    len = (MALLOC_S)(Npa * sizeof(struct proc));
		    if (!(P = (struct proc *)realloc((MALLOC_P *)P, len))) {
			(void) fprintf(stderr,
			    "%s: can't realloc %d proc table entries (%d)\n",
			    Pn, Npa, len);
			Exit(1);
		    }
		    p = &P[Np];
		}
		if (kread(pa, (char *)p, sizeof(struct proc)))
		    break;
		pa = (KA_T)p->p_next;
		if ((p->p_flag & P_DESTROY) || (p->p_flag & P_GONE)
		||  !p->p_pidp

#if	!defined(HAS_P_PGID)
		|| !p->p_pgidp
#endif	/* !defined(HAS_P_PGID) */

		|| !p->p_cred || !p->p_execinfo)
		    continue;
		Np++;
		p++;
	    }
	/*
	 * See if enough processes were accumulated.
	 */
	    if (Np >= PROCMIN)
		break;
	}
/*
 * Quit if not enough proc structures could be collected.
 */
	if (try >= PROCTRYLM) {
	    (void) fprintf(stderr, "%s: can't read proc table\n", Pn);
	    Exit(1);
	}
	if (Np < Npa && !RptTm) {

	/*
	 * If not repeating, reduce the local proc table size to a minimum.
	 */
	    len = (MALLOC_S)(Np * sizeof(struct proc));
	    if (!(P = (struct proc *)realloc((MALLOC_P *)P, len))) {
		(void) fprintf(stderr,
		    "%s: can't reduce proc table to %d entries\n", Pn, Np);
		Exit(1);
	    }
	    Npa = Np;
	}
}
