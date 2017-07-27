/*
 * dproc.c - SCO OpenServer process access functions for lsof
 */


/*
 * Copyright 1995 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1995 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dproc.c,v 1.18 2007/04/24 16:22:40 abe Exp $";
#endif

#include "lsof.h"


/*
 * Local static values
 */

static KA_T Kp;				/* kernel process table address */
static KA_T *Nc = (KA_T *)NULL;		/* node cache */
static int Nn = 0;			/* number of Nc[] entries allocated */

#if	OSRV<500
static int Npp = 0;			/* number of pregions per process */
static struct pregion *Pr = (struct pregion *)NULL;
					/* pregion buffer */
static int Prsz = 0;			/* size of Pr */
#endif	/* OSRV<500 */


static struct var Var;			/* kernel variables */


_PROTOTYPE(static void get_cdevsw,(void));
_PROTOTYPE(static void get_kernel_access,(void));

#if	!defined(N_UNIX)
_PROTOTYPE(static int is_boot,(char *p));
#endif	/* !defined(N_UNIX) */

_PROTOTYPE(static int open_kmem,(int nx));
_PROTOTYPE(static void process_text,(KA_T prp));
_PROTOTYPE(static void readfsinfo,(void));


/*
 * Ckkv - check kernel version
 */

static void
Ckkv(d, er, ev, ea)
	char *d;			/* dialect */
	char *er;			/* expected release */
	char *ev;			/* expected version */
	char *ea;			/* expected architecture */
{

#if	defined(HASKERNIDCK)
	struct scoutsname s;

	if (Fwarn)
	    return;
/*
 * Read OSR kernel information.
 */
	if (__scoinfo(&s, sizeof(s)) < 0) {
	    (void) fprintf(stderr, "%s: can't get __scoinfo: %s\n",
		Pn, strerror(errno));
	    Exit(1);
	}
/*
 * Warn if the actual and expected releases don't match.
 */
	if (!er || strcmp(er, s.release))
	    (void) fprintf(stderr,
		"%s: WARNING: compiled for %s release %s; this is %s.\n",
		Pn, d, er ? er : "UNKNOWN", s.release);
#endif	/* defined(HASKERNIDCK) */

}


/*
 * gather_proc_info() -- gather process information
 */

void
gather_proc_info()
{
	int i, j, nf, pbc, px;
	struct proc *p;
	static char *pb = (char *)NULL;
	int pid, pgrp;
	short pss, sf;
	static struct user *u;
	static char *ua = (char *)NULL;
	static MALLOC_S ual = 0;
	unsigned int uid;

#if	defined(HASFSTRUCT)
	static MALLOC_S npofb = 0; 
	char *pof;
	static char *pofb = (char *)NULL;
#endif	/* defined(HASFSTRUCT) */

/*
 * Allocate user structure buffer.
 */
	if (!ua) {
	    ual = (MALLOC_S)(MAXUSIZE * NBPC);
	    if (!(ua = (char *)malloc(ual))) {
		(void) fprintf(stderr,
		    "%s: no space for %d byte user structure buffer\n",
		    Pn, ual);
		Exit(1);
	    }
	    u = (struct user *)ua;
	}
/*
 * Allocate proc structure buffer.
 */
	if (!pb) {
	    if (!(pb = (char *)malloc(sizeof(struct proc) * PROCBFRD))) {
		(void) fprintf(stderr, "%s: no space for %d proc structures\n",
		    Pn, PROCBFRD);
		Exit(1);
	    }
	}
/*
 * Examine proc structures and their associated information.
 */
	for (pbc = px = 0; px < Var.v_proc; px++) {
	    if (px >= pbc) {

	    /*
	     * Refill proc buffer.
	     */
		i = Var.v_proc - px;
		if (i > PROCBFRD)
		    i = PROCBFRD;
		j = kread((KA_T)(Kp + (px * sizeof(struct proc))), pb,
			sizeof(struct proc) * i);
		pbc = px + i;
		p = (struct proc *)pb;
		if (j) {
		    px += i;
		    continue;
		}
	   } else
		p++;
	    if (p->p_stat == 0 || p->p_stat == SZOMB)
		continue;
	/*
	 * Get Process ID, Process group ID, and User ID.
	 */
	    pid = (int)p->p_pid;
	    pgrp = (int)p->p_pgrp;
	    uid = (unsigned int)p->p_uid;
	    if (is_proc_excl(pid, pgrp, (UID_ARG)uid, &pss, &sf))
		continue;
	/*
	 * Get the user area associated with the process.
	 */
	    if (sysi86(RDUBLK, pid, ua, MAXUSIZE * NBPC) == -1)
		continue;
	/*
	 * Allocate a local process structure.
	 */
	    if (is_cmd_excl(u->u_comm, &pss, &sf))
		continue;
	    alloc_lproc(pid, pgrp, (int)p->p_ppid, (UID_ARG)uid, u->u_comm,
		(int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;
	/*
	 * Save current working directory information.
	 */
	    if (u->u_cdir) {
		alloc_lfile(CWD, -1);
		process_node((KA_T)u->u_cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (u->u_rdir) {
		alloc_lfile(RTD, -1);
		process_node((KA_T)u->u_rdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Print information on the text file.
	 */
	    if (p->p_region)
		process_text((KA_T)p->p_region);
	/*
	 * Save information on file descriptors.
	 */

#if	OSRV<42
	    nf = Var.v_nofiles;
#else	/* OSRV>=42 */
	    nf = u->u_nofiles ? u->u_nofiles : Var.v_nofiles;
#endif	/* OSRV<42 */

#if	defined(HASFSTRUCT)
	    if (Fsv & FSV_FG) {

	    /*
	     * If u_pofile is in the u block, set its address.
	     */
		if (nf && u->u_pofile
		&&  ((unsigned)u->u_pofile >= UVUBLK)
		&&  ((MALLOC_S)((unsigned)u->u_pofile - UVUBLK + nf) <= ual))
		{
		     pof = ua + (unsigned)u->u_pofile - UVUBLK;
		} else if (nf && u->u_pofile) {

		/*
		 * Allocate space for u_pofile and read it from kernel memory.
		 */
		    if (nf > npofb) {
			if (!pofb)
			    pofb = (char *)malloc((MALLOC_S)nf);
			else
			    pofb = (char *)realloc((MALLOC_P *)pofb,
						   (MALLOC_S)nf);
			if (!pofb) {
			    (void) fprintf(stderr, "%s: no pofile space\n", Pn);
			    Exit(1);
			}
			npofb = nf;
		    }
		    if (kread((KA_T)u->u_pofile, pofb, nf))
			pof = (char *)NULL;
		    else
			pof = pofb;
		} else
		    pof = (char *)NULL;
	    }
#endif	/* defined(HASFSTRUCT) */

	    for (i = 0; i < nf; i++) {
		if (u->u_ofile[i]) {
		    alloc_lfile((char *)NULL, i);
		    process_file((KA_T)u->u_ofile[i]);
		    if (Lf->sf) {

#if	defined(HASFSTRUCT)
			if (Fsv & FSV_FG && pof)
			    Lf->pof = (long)pof[i];
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
 * get_cdevsw() - get cdevsw[] names and record clone major device number
 */

void
get_cdevsw()
{
	char buf[16];
	struct cdevsw *c, *tmp;
	int i, j, len;
	struct stat sb;
	KA_T v[2];
/*
 * Check cdevsw[] kernel addresses.
 * Read cdevsw[] count from kernel's cdevcnt.
 */
	if (get_Nl_value("cdev",  Drive_Nl, &v[0]) < 0
	||  get_Nl_value("ncdev", Drive_Nl, &v[1]) < 0
	||  !v[0] || !v[1]
	||  kread(v[1], (char *)&Cdevcnt, sizeof(Cdevcnt))
	||  Cdevcnt < 1)
	    return;
/*
 * Allocate cache space.
 */
	if (!(Cdevsw = (char **)malloc(Cdevcnt * sizeof(char *)))) {
	    (void) fprintf(stderr, "%s: no space for %d cdevsw[] names\n",
		Pn, Cdevcnt);
	    Exit(1);
	}
/*
 * Allocate temporary space for a copy of cdevsw[] and read it.
 */
	i = Cdevcnt * sizeof(struct cdevsw);
	if (!(tmp = (struct cdevsw *)malloc(i))) {
	    (void) fprintf(stderr, "%s: no space for %d cdevsw[] entries\n",
		Pn, Cdevcnt);
	    Exit(1);
	}
	if (kread((KA_T)v[0], (char *)tmp, i)) {
	    (void) free((FREE_P *)Cdevsw);
	    Cdevsw = (char **)NULL;
	    Cdevcnt = 0;
	    (void) free((FREE_P *)tmp);
	    return;
	}
/*
 * Cache the names from cdevsw[].d_name.
 * Record the number of the "clone" device.
 */
	j = sizeof(buf) - 1;
	buf[j] = '\0';
	for (c = tmp, i = 0; i < Cdevcnt; c++, i++) {
	    Cdevsw[i] = (char *)NULL;
	    if (!c->d_name)
		continue;
	    if (kread((KA_T)c->d_name, buf, j)) {
		(void) fprintf(stderr,
		    "%s: WARNING: can't read name for cdevsw[%d]: %#x\n",
		    Pn, i, c->d_name);
		continue;
	    }
	    if (!buf[0])
		continue;
	    len = strlen(buf) + 1;
	    if (!(Cdevsw[i] = (char *)malloc(len))) {
		(void) fprintf(stderr, "%s: no space for cdevsw[%d] name: %s\n",
		   Pn, i, buf);
		Exit(1);
	    }
	    (void) snpf(Cdevsw[i], len, "%s", buf);
	    if (!HaveCloneMajor && strcmp(buf, "clone") == 0) {
		CloneMajor = i;
		HaveCloneMajor = 1;
		continue;
	    }
	    if (!HaveEventMajor && strcmp(buf, "ev") == 0) {
		if (stat("/dev/event", &sb) == 0
		&&  GET_MAJ_DEV(sb.st_rdev) == i) {
		    EventMajor = i;
		    HaveEventMajor = 1;
		}
	    }
	}
	(void) free((FREE_P *)tmp);
}


/*
 * get_kernel_access() - get access to kernel memory
 */

static void
get_kernel_access()
{
	time_t lbolt;
	MALLOC_S len;
	KA_T v;
/*
 * Check kernel version.
 */
	(void) Ckkv("OSR", LSOF_VSTR, (char *)NULL, (char *)NULL);
/*
 * See if the name list file is readable.
 */
	if (Nmlst && !is_readable(Nmlst, 1))
	    Exit(1);
/*
 * Access kernel symbols.
 */

#if	defined(N_UNIX)
	(void) build_Nl(Drive_Nl);
	if (nlist(Nmlst ? Nmlst : N_UNIX, Nl) < 0)
#else	/* !defined(N_UNIX) */
	if (!get_nlist_path(0))
#endif	/* defined(N_UNIX) */

	{
	    (void) fprintf(stderr, "%s: can't read kernel name list.\n", Pn);
	    Exit(1);
	}
/*
 * Open access to kernel memory.
 */
	(void) open_kmem(0);

#if     defined(WILLDROPGID)
/*
 * Drop setgid permission.
 */
	(void) dropgid();
#endif  /* defined(WILLDROPGID) */

/*
 * Check proc table pointer.
 */
	if (get_Nl_value("proc", Drive_Nl, &Kp) < 0 || !Kp) {
	    (void) fprintf(stderr, "%s: no proc table pointer\n", Pn);
	    Exit(1);
	}

#if	OSRV<500
/*
 * Read pregion information.
 */
	v = (KA_T)0;
	if (get_Nl_value("pregpp", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Npp, sizeof(Npp))
	||  Npp < 1) {
	    (void) fprintf(stderr,
		"%s: can't read pregion count (%d) from %s\n", Pn, Npp,
		    print_kptr(v, (char *)NULL, 0));
	    Exit(1);
	}
	Prsz = (MALLOC_S)(Npp * sizeof(struct pregion));
	if (!(Pr = (struct pregion *)malloc(Prsz))) {
	    (void) fprintf(stderr,
		"%s: can't allocate space for %d pregions\n",
		Pn, Npp);
	    Exit(1);
	}
#endif	/* OSRV< 500 */

/*
 * Read system configuration information.
 */
	if (get_Nl_value("var", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&Var, sizeof(Var)))
	{
	    (void) fprintf(stderr,
		"%s: can't read system configuration info\n", Pn);
	    Exit(1);
	}
/*
 * Read system clock values -- Hz and lightning bolt timer.
 */
	v = (KA_T)0;
	if (get_Nl_value("hz", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Hz, sizeof(Hz)))
	{
	    if (!Fwarn)
		(void) fprintf(stderr, "%s: WARNING: can't read Hz from %s\n",
		    Pn, print_kptr(v, (char *)NULL, 0));
	    Hz = -1;
	}
	if (get_Nl_value("lbolt", Drive_Nl, &Lbolt) < 0 || !v
	||  kread((KA_T)v, (char *)&lbolt, sizeof(lbolt)))
	{
	    if (!Fwarn)
		(void) fprintf(stderr,
		     "%s: WARNING: can't read lightning bolt timer from %s\n",
		     Pn, print_kptr(v, (char *)NULL, 0));
	    Lbolt = (KA_T)0;
	}
/*
 * Get socket device number and socket table address.
 */
	if (get_Nl_value("sockd", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Sockdev, sizeof(Sockdev)))
	{
	    (void) fprintf(stderr,
		"%s: WARNING: can't identify socket device.\n", Pn);
	    (void) fprintf(stderr,
		"      Socket output may be incomplete.\n");
	    return;
	}
	if (get_Nl_value("sockt", Drive_Nl, &Socktab) < 0 || !Socktab) {
	    (void) fprintf(stderr,
		"%s: WARNING: socket table address is NULL.\n", Pn);
	    (void) fprintf(stderr,
		"      Socket output may be incomplete.\n");
	    return;
	}

#if	OSRV>=40
/*
 * Get extended device table parameters.  These are needed by the kernel
 * versions of the major() and minor() device number macros; they also
 * identify socket devices and assist in the conversion of socket device
 * numbers to socket table addresses.
 */
	v = (KA_T)0;
	if (get_Nl_value("nxdm", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&nxdevmaps, sizeof(nxdevmaps))
	||  nxdevmaps < 0)
	{
	    (void) fprintf(stderr,
		"%s: bad extended device table size (%d) at %s.\n",
		Pn, nxdevmaps, print_kptr(v, (char *)NULL, 0));
	    Exit(1);
	}
	len = (MALLOC_S)((nxdevmaps + 1) * sizeof(struct XDEVMAP));
	if (!(Xdevmap = (struct XDEVMAP *)malloc(len))) {
	    (void) fprintf(stderr, "%s: no space for %d byte xdevmap table\n",
		Pn, len);
	    Exit(1);
	}
	v = (KA_T)0;
	if (get_Nl_value("xdm", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)Xdevmap, len))
	{
	    (void) fprintf(stderr,
		"%s: can't read %d byte xdevmap table at #x\n", Pn, len, v);
	    Exit(1);
	}
#endif	/* OSRV>=40 */

	HaveSockdev = 1;
}


#if	!defined(N_UNIX)
/*
 * get_nlist_path() - get kernel nlist() path
 *
 * As a side effect on a successful return (non-NULL character pointer), the
 * boot path's name list will have been loaded into Nl[].
 */

char *
get_nlist_path(ap)
	int ap;				/* on success, return an allocated path
					 * string pointer if 1; return a
					 * constant character pointer if 0;
					 * return NULL if failure */
{
	FILE *bf;
	char *bfp, b1[MAXPATHLEN+1], b2[MAXPATHLEN+1], *pp, *tp;
	struct dirent *de;
	char *dir[] = { "/", "/stand/", NULL };
	DIR *dp;
	int i;
	MALLOC_S len;
/*
 * If a kernel name list file was specified, use it.
 */
	if (Nmlst) {
	    if (is_boot(Nmlst))
		return(Nmlst);
	    return((char *)NULL);
	}
/*
 * If it's possible to open /etc/ps/booted system, search it for a preferred
 * boot path, defined by the value of a line that begins with "KERNEL=".
 */
	bfp = pp = (char *)NULL;
	if ((bf = fopen("/etc/ps/booted.system", "r"))) {
	    len = strlen("KERNEL=");
	    while (fgets(b1, sizeof(b1), bf)) {
		if (strncmp(b1, "KERNEL=", len) != 0)
		    continue;
		if ((tp = strrchr(&b1[len], '\n'))) {
		    *tp = '\0';
		    if (b1[len]) {
			bfp = &b1[len];
			if (is_boot(bfp)) {
			    pp = bfp;
			    (void) fclose(bf);
			    goto get_nlist_return_path;
			}
			break;
		    }
		}
	    }
	    (void) fclose(bf);
	}
/*
 * Look for possible unix* boot paths.
 */
	for (i = 0; dir[i]; i++) {
	    if (!(dp = opendir(dir[i])))
		continue;
	    while ((de = readdir(dp))) {

	    /*
	     * Use the next entry that begins with "unix".
	     */
		if (strncmp("unix", de->d_name, 4) != 0)
		    continue;
	    /*
	     * Construct a temporary copy of the path name,
	     * If it matches the preferred boot name, skip it.
	     */
		len = strlen(dir[i]) + strlen(de->d_name) + 1;
		if (len >= sizeof(b2))
		    continue;
		(void) snpf(b2, sizeof(b2), "%s%s", dir[i], de->d_name);
		if (bfp && strcmp(b2, bfp) == 0)
		    continue;
	    /*
	     * See if it's the booted kernel.
	     */
		if (is_boot(b2)) {
		    (void) closedir(dp);
		    pp = b2;

get_nlist_return_path:

		/*
		 * A boot path has been located.  As requested return a
		 * malloc'd pointer to it.
		 */
		    if (!ap)
			return("");
		    len = (MALLOC_S)(strlen(pp) + 1);
		    if (!(tp = (char *)malloc(len))) {
			(void) fprintf(stderr,
			    "%s: can't allocate %d bytes for: %s\n",
			    Pn, len , pp);
			Exit(1);
		    }
		    (void) snpf(tp, len, "%s", pp);
		    return(tp);
		}
	    }
	    if (dp)
		(void) closedir(dp);
	}
	return((char *)NULL);
}
#endif	/* !defined(N_UNIX) */


/*
 * initialize() - perform all initialization
 */

void
initialize()
{
	get_kernel_access();
	get_cdevsw();
	readfsinfo();
	if (Fsv & FSV_NI)
	    NiTtl = "INODE-ADDR";
}


#if	!defined(N_UNIX)
/*
 * is_boot() - does the specified path lead to a booted kernel
 */

is_boot(p)
	char *p;			/* specified path */
{
	int i;
	KA_T ka;
	union {
	    struct scoutsname s;
	    unsigned char sc[sizeof(struct scoutsname)];
	} s1, s2;
/*
 * Get the scoutsname structure via __scoinfo() to use as a reference against
 * the one obtained via kread()'ing from the nlist(<possible_kernel>) address.
 *  If __scoinfo() fails, return the default boot path.
 */
	if (__scoinfo(&s1.s, sizeof(s1.s)) < 0)
	    return 0;
/*
 * Get the name list for this boot path.  Using the scoutsname address, read
 * the scoutsname structure and compare it to the _s_scoinfo() one.  If the
 * two match, this is the boot path.
 */
	if (Nl) {
	    (void) free((FREE_P *)Nl);
	    Nl = (struct NLIST_TYPE *)NULL;
	}
	(void) build_Nl(Drive_Nl);
	if (nlist(p, Nl) < 0)
	    return(0);
	if (get_Nl_value("scouts", Drive_Nl, &ka) < 0 || !ka)
	    return(0);
	if (Kd < 0) {
	    if (open_kmem(1))
		return(0);
	}
	if (kread(ka, (char *)&s2.s, sizeof(s2.s)))
	    return(0);
	for (i = 0; i < sizeof(struct scoutsname); i++) {
		if (s1.sc[i] != s2.sc[i])
		    return(0);
	}
	return(1);
}
#endif	/* !defined(N_UNIX) */


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

	if (lseek(Kd, (off_t)addr, SEEK_SET) == (off_t)-1L)
		return(1);
	if ((br = read(Kd, buf, len)) < 0)
		return(1);
	return(((READLEN_T)br == len) ? 0 : 1);
}


/*
 * open_kmem() - open kernel memory access
 */

static int
open_kmem(nx)
	int nx;				/* no Exit(1) if 1 */
{
	if (Kd >= 0)
	    return(0);
/*
 * See if the non-KMEM memory file is readable.
 */
	if (Memory && !is_readable(Memory, 1)) {
	    if (nx)
		return(1);
	    Exit(1);
	}
/*
 * Open kernel memory access.
 */
	if ((Kd = open(Memory ? Memory : KMEM, O_RDONLY, 0)) < 0) {
	    if (nx)
		return(1);
	    (void) fprintf(stderr, "%s: can't open %s: %s\n", Pn,
		Memory ? Memory : KMEM, strerror(errno));
	    Exit(1);
	}
	return(0);
}


/*
 * process_text() - process text access information
 */

static void
process_text(prp)
	KA_T prp;			/* process region pointer */
{
	int i, j, k;
	struct pregion *p;
	struct region r;
	KA_T na;
	char *ty, tyb[8];

#if	OSRV>=500
	KA_T pc;
	struct pregion ps;
#endif	/* OSRV>=500 */

/*
 * Read and process the pregions.
 */

#if	OSRV<500
	if (kread(prp, (char *)Pr, Prsz))
	    return;
	for (i = j = 0, p = Pr; i < Npp; i++, p++)
#else	/* OSRV>=500 */
	for (i = j = 0, p = &ps, pc = prp; pc; pc = (KA_T)p->p_next, i++)
#endif	/* OSRV<500 */

	{

#if	OSRV>=500
	/*
	 * Avoid infinite loop.
	 */
	    if (i > 1000) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: too many virtual address regions for PID %d\n",
			Pn, Lp->pid);
		return;
	    }
	    if ((i && pc == prp)
	    ||  kread((KA_T)pc, (char *)p, sizeof(ps)))
		return;
#endif	/* OSRV>=500 */

	    if (!p->p_reg)
		continue;
	/*
	 * Read the region.
	 * Skip entries with no node pointers and duplicate node addresses.
	 */
	    if (kread((KA_T)p->p_reg, (char *)&r, sizeof(r)))
		continue;
	    if (!(na = (KA_T)r.r_iptr))
		continue;
	    for (k = 0; k < j; k++) {
		if (Nc[k] == na)
		    break;
	    }
	    if (k < j)
		continue;
	/*
	 * Cache the node address for duplicate checking.
	 */
	    if (!Nc) {
		if (!(Nc = (KA_T *)malloc((MALLOC_S)(sizeof(KA_T) * 10)))) {
		    (void) fprintf(stderr, "%s: no txt ptr space, PID %d\n",
			Pn, Lp->pid);
		    Exit(1);
		}
		Nn = 10;
	    } else if (j >= Nn) {
		Nn += 10;
		if (!(Nc = (KA_T *)realloc((MALLOC_P *)Nc,
				   (MALLOC_S)(Nn * sizeof(KA_T)))))
		{
		    (void) fprintf(stderr,
			"%s: no more txt ptr space, PID %d\n", Pn, Lp->pid);
		    Exit(1);
		}
	    }
	    Nc[j++] = na;
	/*
	 * Save text node and mapped region information.
	 */
	    switch (p->p_type) {
 	    case PT_DATA:		/* data and text of */
 	    case PT_TEXT:		/* executing binaries */
 		ty = " txt";
 		break;
  	    case PT_LIBDAT:		/* shared library data and */
  	    case PT_LIBTXT:		/* COFF format text */
  		ty = " ltx";
  		break;
 	    case PT_SHFIL:		/* memory mapped file */
 		ty = " mem";
 		break;
 	    case PT_V86:		/* virtual 8086 mode */
 		ty = " v86";
		break;
 	    case PT_VM86:		/* MERGE386 vm86 region */
 		ty = " m86";
 		break;
  	    default:		/* all others as a hex number */
		(void) snpf(tyb, sizeof(tyb), " M%02x", p->p_type & 0xff);
 		ty = tyb;
	    }
	    if (ty) {
		alloc_lfile(ty, -1);
		process_node(na);
		if (Lf->sf)
		    link_lfile();
	    }
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


#if	defined(HASNCACHE)

/*
 * Prepare for and #include the appropriate header files.
 */

# if	OSRV>=500
#undef	IFIR
#undef	IFIW
#undef	IRCOLL
#undef	IWCOLL
# endif	/* OSRV>=500 */

#include <sys/fs/s5inode.h>

# if	defined(HAS_NFS)
#include <sys/fs/nfs/dnlc.h>
# endif	/* defined(HAS_NFS) */

# if	OSRV>=500
#  if	OSRV<504
#include <sys/fs/dtdnlc.h>
#undef	IFIR
#undef	IFIW
#undef	IRCOLL
#undef	IWCOLL
#define	_INKERNEL
#include <sys/fs/htinode.h>
#undef	_INKERNEL
#  else	/* OSRV>=504 */
#include <sys/fs/dnlc.h>
#  endif	/* OSRV<504 */
# endif	/* OSRV>=500 */


/*
 * Determine the maximum size of the cache name character array.
 */

# if	OSRV<504
#define	MAXNSZ	DIRSIZ
#  if	OSRV>=500 && DTNCMAX>MAXNSZ
#undef	MAXNSZ
#define	MAXNSZ	DTNCMAX
#  endif	/* OSRV>=500 && DTNCMAX>MAXNSZ */
# else	/* OSRV>=504 */
#define	MAXNSZ	DNLC_NAMELEN
# endif	/* OSRV<504 */

# if	defined(HAS_NFS) && NC_NAMLEN>MAXNSZ
#undef	MAXNSZ
#define	MAXNSZ	NC_NAMLEN
# endif	/* defined(HAS_NFS) && NC_NAMLEN>MAXNSZ */


/*
 * Define the local name cache structures.
 */

struct lnch {				/* local name cache structure */
	union {
	    struct ldev {		/* device-inode info */
		dev_t dev;		/* device */
		unsigned long inum;	/* inode number */
		unsigned long pa_inum;	/* parent inode number */
	    } ld;
	    struct lnfs {		/* NFS info */
		KA_T rp;		/* rnode address */
		KA_T dp;		/* parent rnode address */
	    } ln;
	} u;
	char nm[MAXNSZ+1];		/* name */
	unsigned char nl;		/* name length */
	unsigned char dup;		/* duplicate if 1 */
	unsigned char type;		/* type: 0 = device-inode; 1 = NFS */
	struct lnch *pa;		/* parent address */
	struct lnch *next;		/* link to next same-type structure */
};

struct lnch_hh {			/* device-inode and NFS hash head */
	struct lnch *hp[2];		/* [0] = device-inode; [1] = NFS*/
};


/*
 * Local name cache (LNC) definitions, macros, and static values
 */

#define	LCHUNKSZ	256		/* local "chunk" size for reading the
					 * kernel DNLC -- used for OSRV>=504 */
static int LNC_asz = 0;			/* LNC cache allocated size */
static int LNC_csz = 0;			/* LNC cache current size */
#define	LNCHHLEN	64		/* hash head length (must be a
					 * power of 2) */
#define	LNCINCR		256		/* LNC size increment */
#define	LNCINIT		1024		/* LNC initial size */

#define DIN_hash(d, i)	&LNC_hh[((((int)(d + i)>>2)*31415)&(LNCHHLEN-1))]

# if	defined(HAS_NFS)
#define NFS_hash(r)	&LNC_hh[((((int)(r)>>2)*31415)&(LNCHHLEN-1))]
# endif	/* defined(HAS_NFS) */

static struct lnch_hh *LNC_hh = (struct lnch_hh *)NULL;
					/* LNC hash head pointers */
static struct lnch *LNC_nc = (struct lnch *)NULL;
					/* the linear LNC */


/*
 * Local function prototypes
 */

_PROTOTYPE(static struct lnch *DIN_addr,(dev_t *d, unsigned long i));

# if	OSRV>=500
#  if	OSRV>=504
_PROTOTYPE(static void DNLC_load,());
#  else	/* OSRV<504 */
_PROTOTYPE(static void DTFS_load,());
_PROTOTYPE(static void HTFS_load,());
#  endif	/* OSRV>=504 */
# endif /* OSRV>=500 */

_PROTOTYPE(static int LNC_enter,(struct lnch *le, char *nm, int nl, char *fs));
_PROTOTYPE(static void LNC_nosp,(int len));

# if	defined(HAS_NFS)
_PROTOTYPE(static struct lnch *NFS_addr,(KA_T r));
_PROTOTYPE(static void NFS_load,(void));
_PROTOTYPE(static int NFS_root,(KA_T r));
# endif /* defined(HAS_NFS) */

# if	OSRV<504
_PROTOTYPE(static void SYSV_load,());
# endif	/* OSRV<504 */


/*
 * DIN_addr() - look up a node's local device-inode address
 */

static struct lnch *
DIN_addr(d, i)
	dev_t *d;			/* device number */
	unsigned long i;		/* inode number */
{
	struct lnch_hh *hh = DIN_hash(*d, i);
	struct lnch *lc = hh->hp[0];

	while (lc) {
	    if (lc->u.ld.dev == *d && lc->u.ld.inum == i)
		return(lc);
	    lc = lc->next;
	}
	return((struct lnch *)NULL);
}


# if	OSRV>=504
/*
 * DNLC_load() - load DNLC cache
 */

static void
DNLC_load()
{
	int ccl;				/* current "chunk" length */
	int ccs;				/* current "chunk" size */
	int ccx;				/* current "chunk" index */
	static int cha = 0;			/* "chunk" allocation size */
	static int chl = 0;			/* "chunk" allocation length */
	struct dnlc__cachent *cp;
	static struct dnlc__cachent *dnlc = (struct dnlc__cachent *)NULL;
	static int dnlce = 0;
	int i, len;
	static KA_T ka = (KA_T)0;
	struct lnch lc;
	char nm[DNLC_NAMELEN+1];
	KA_T v;
	char *wa;				/* "working" kernel DNLC
						 * address */
/*
 * Do first-time setup, as required.
 */
	if (dnlce == 0) {

	/*
	 * Quit if the DNLC name cache address is unknown.
	 */
	    if (get_Nl_value("dnlc", Drive_Nl, &ka) < 0 || !ka)
		return;
	/*
	 * Determine of the DNLC name cache address is that of an array or a
	 * pointer to the array.
	 */
	    v = (KA_T)NULL;
	    if (get_Nl_value("pdnlc", Drive_Nl, &v) >= 0 && v) {

	    /*
	     * If the DNLC name cache address is that of a pointer to an array,
	     * get the array's address.  If that fails, return without comment
	     * and without further action.
	     */
		if (kread(ka, (char *)&ka, sizeof(ka)))
		    return;
	    }
	/*
	 * Get the kernel's DNLC name cache size.
	 */
	    if (get_Nl_value("ndnlc", Drive_Nl, &v) < 0 || !v
	    ||  kread(v, (char *)&dnlce, sizeof(dnlce))
	    ||  dnlce < 1)
		return;
	/*
	 * Allocate space for a copy of a portion ("chunk") of the kernel's
	 * DNLC name cache.
	 */
	    cha = (dnlce <= LCHUNKSZ) ? dnlce : LCHUNKSZ;
	    chl = sizeof(struct dnlc__cachent) * cha;
	    if (!(dnlc = (struct dnlc__cachent *)malloc(chl))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for DNLC chunk\n",
		    Pn, chl);
		cha = 0;
		Exit(1);
	    }
	}
/*
 * Prepare to read the kernel's DNLC name cache.
 */
	if (!cha || !chl || !dnlc || !ka)
	    return;
/*
 * Build a local copy of the kernel's DNLC name cache, reading the kernel data
 * a "chunk" at a time.
 */
	nm[DNLC_NAMELEN] = '\0';
	lc.type = 0;
	for (ccl = ccs = i = 0, wa = (char *)ka; i < dnlce; i += ccs, wa += ccl)
	{

	/*
	 * Read the next "chunk".
	 */
	    ccs = ((dnlce - i) < cha) ? (dnlce - i) : cha;
	    ccl = sizeof(struct dnlc__cachent) * ccs;
	    if (kread((KA_T)wa, (char *)dnlc, ccl))
		break;
	/*
	 * Process the "chunk".
	 */
	    for (ccx = 0, cp = dnlc; ccx < ccs; cp++, ccx++) {
		if (!cp->dev && !cp->newinum)
		    continue;
		(void) strncpy(nm, cp->name, DNLC_NAMELEN);
		if ((len = strlen(nm)) < 1)
		    continue;
		if (len < 3 && nm[0] == '.') {
		    if (len == 1 || (len == 2 && nm[1] == '.'))
			continue;
		}
		lc.u.ld.dev = cp->dev;
		lc.u.ld.inum = (unsigned long)cp->newinum;
		lc.u.ld.pa_inum = (unsigned long)cp->inum;
		if (LNC_enter(&lc, nm, len, "DNLC"))
		    break;
	    }
	}
/*
 * If not repeating, free kernel DNLC name cache buffer space.
 */
	if (dnlc && !RptTm) {
	    (void) free((MALLOC_P *)dnlc);
	    dnlc = (struct dnlc__cachent *)NULL;
	    dnlce = cha = chl = 0;
	}
}
# endif	/* OSRV>=504 */


# if	OSRV>=500 && OSRV<504
/*
 * DTFS_load() - load DTFS cache
 */

static void
DTFS_load()
{
	struct dtcachent *cp;
	static struct dtcachent *dtnc = (struct dtcachent *)NULL;
	static int dtnce = 0;
	int i, len;
	static KA_T ka = (KA_T)NULL;
	static int kcl = 0;
	struct lnch lc;
	char nm[DTNCMAX+1];
	KA_T v;
/*
 * Do first-time setup, as required.
 */
	if (dtnce == 0) {

	/*
	 * Quit if the DTFS name cache address is unknown.
	 */
	    if (get_Nl_value("dtnc", Drive_Nl, &ka) < 0 || !ka)
		return;
	/*
	 * Get the kernel's DTFS name cache size.
	 */
	    if (get_Nl_value("ndtnc", Drive_Nl, &v) < 0 || !v
	    ||  kread(v, (char *)&dtnce, sizeof(dtnce))
	    ||  dtnce < 1)
		return;
	/*
	 * Allocate space for a copy of the kernel's DTFS name cache.
	 */
	    kcl = sizeof(struct dtcachent) * dtnce;
	    if (!(dtnc = (struct dtcachent *)malloc(kcl))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for DTFS name cache\n",
		    Pn, kcl);
		Exit(1);
	    }
	}
/*
 * Read the kernel's DTFS name cache.
 */
	if (!dtnc || !kcl || !ka || kread(ka, (char *)dtnc, kcl))
	    return;
/*
 * Build a local copy of the kernel's DTFS name cache.
 */
	lc.type = 0;
	nm[DTNCMAX] = '\0';
	for (cp = dtnc, i = 0; i < dtnce; cp++, i++) {
	    if (!cp->dn_dev && !cp->dn_newinum)
		continue;
	    (void) strncpy(nm, cp->dn_name, DTNCMAX);
	    if ((len = strlen(nm)) < 1)
		continue;
	    if (len < 3 && cp->dn_name[0] == '.') {
		if (len == 1 || (len == 2 && cp->dn_name[1] == '.'))
		    continue;
	    }
	    lc.u.ld.dev = cp->dn_dev;
	    lc.u.ld.inum = (unsigned long)cp->dn_newinum;
	    lc.u.ld.pa_inum = (unsigned long)cp->dn_inum;
	    if (LNC_enter(&lc, nm, len, "DTFS"))
		break;
	}
/*
 * If not repeating, free kernel DTFS name cache buffer space.
 */
	if (dtnc && !RptTm) {
	    (void) free((MALLOC_P *)dtnc);
	    dtnc = (struct dtcachent *)NULL;
	    dtnce = kcl = 0;
	}
}


/*
 * HTFS_load() - load HTFS cache
 */

static void
HTFS_load()
{
	struct htcachent *cp;
	static struct htcachent *htnc = (struct htcachent *)NULL;
	static int htnce = 0;
	int i, len;
	static KA_T ka = (KA_T)NULL;
	static int kcl = 0;
	struct lnch lc;
	char nm[DIRSIZ+1];
	KA_T v;
/*
 * Do first-time setup, as required.
 */
	if (htnce == 0) {

	/*
	 * Quit if the HTFS name cache address is unknown.
	 */
	    if (get_Nl_value("htnc", Drive_Nl, &ka) < 0 || !ka)
		return;
	/*
	 * Get the kernel's HTFS name cache size.
	 */
	    if (get_Nl_value("nhtnc", Drive_Nl, &v) < 0 || !v
	    ||  kread(v, (char *)&htnce, sizeof(htnce))
	    ||  htnce < 1)
		return;
	/*
	 * Allocate space for a copy of the kernel's HTFS name cache.
	 */
	    kcl = sizeof(struct htcachent) * htnce;
	    if (!(htnc = (struct htcachent *)malloc(kcl))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for HTFS name cache\n",
		    Pn, kcl);
		Exit(1);
	    }
	}
/*
 * Read the kernel's HTFS name cache.
 */
	if (!htnc || !kcl || !ka || kread(ka, (char *)htnc, kcl))
	    return;
/*
 * Build a local copy of the kernel's HTFS name cache.
 */
	lc.type = 0;
	nm[DIRSIZ] = '\0';
	for (cp = htnc, i = 0; i < htnce; cp++, i++) {
	    if (!cp->dev && !cp->newinum)
		continue;
	    (void) strncpy(nm, cp->name, DIRSIZ);
	    if ((len = strlen(nm)) < 1)
		continue;
	    if (len < 3 && cp->name[0] == '.') {
		if (len == 1 || (len == 2 && cp->name[1] == '.'))
		    continue;
	    }
	    lc.u.ld.dev = (dev_t)cp->dev;
	    lc.u.ld.inum = (unsigned long)cp->newinum;
	    lc.u.ld.pa_inum = (unsigned long)cp->inum;
	    if (LNC_enter(&lc, nm, len, "HTFS"))
		break;
	}
/*
 * If not repeating, free kernel HTFS name cache buffer space.
 */
	if (htnc && !RptTm) {
	    (void) free((MALLOC_P *)htnc);
	    htnc = (struct htcachent *)NULL;
	    htnce = kcl = 0;
	}
}
# endif	/* OSRV>=500 && OSRV<504 */


/*
 * LNC_enter() - make a local name cache entry
 */

static int
LNC_enter(le, nm, nl, fs)
	struct lnch *le;		/* skeleton local entry */
	char *nm;			/* name */
	int nl;				/* name length */
	char *fs;			/* file system name */
{
	struct lnch *lc;
	MALLOC_S len;

	if (LNC_csz >= LNC_asz) {
	    LNC_asz += LNCINCR;
	    len = (MALLOC_S)(LNC_asz * sizeof(struct lnch));
	    if (!(LNC_nc = (struct lnch *)realloc(LNC_nc, len))) {
		(void) fprintf(stderr,
		    "%s: no more space for %d byte local name cache: %s\n",
		    Pn, len, fs);
		Exit(1);
	    }
	}
	lc = &LNC_nc[LNC_csz];
	if ((lc->type = le->type) == 1) {

	/*
	 * Make an NFS entry.
	 */
	    lc->u.ln.rp = le->u.ln.rp;
	    lc->u.ln.dp = le->u.ln.dp;
	} else {
	
	/*
	 * Make a device-inode entry.
	 */
	    lc->u.ld.dev = le->u.ld.dev;
	    lc->u.ld.inum = le->u.ld.inum;
	    lc->u.ld.pa_inum = le->u.ld.pa_inum;
	}
/*
 * Enter the name and its size, clear the duplicate flag,
 * and advance the linear cache entry count.
 */
	if (nl > MAXNSZ) {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: length for \"%s\" too large: %d\n",
		    Pn, nm, nl);
	    nl = MAXNSZ;
	}
	(void) strncpy(lc->nm, nm, nl);
	lc->nm[nl] = '\0';
	lc->nl = (unsigned char)nl;
	lc->dup = 0;
	lc->next = lc->pa = (struct lnch *)NULL;
	LNC_csz++;
	return(0);
}


/*
 * LNC_nosp() - notify that we're out of space for the local name cache
 */

static void
LNC_nosp(len)
	int len;			/* attempted length */
{
	if (!Fwarn)
	    (void) fprintf(stderr,
		"%s: no space for %d byte local name cache\n",
		Pn, len);
	Exit(1);
}


/*
 * ncache_load() - load the kernel's NFS and DEV name caches
 */

void
ncache_load()
{
	struct lnch_hh *hh;
	struct lnch *hl, *hlp, *lc;
	int f, i, len;

	if (!Fncache)
	    return;
/*
 * Initialize local name cache, as required.
 */
	if (LNC_asz == 0) {
	    LNC_asz = LNCINIT;
	    len = LNCINIT * sizeof(struct lnch);
	    if (!(LNC_nc = (struct lnch *)malloc((MALLOC_S)len)))
		(void) LNC_nosp(len);
	}
	LNC_csz = 0;

# if	defined(HAS_NFS)
/*
 * Load NFS cache.
 */
	(void) NFS_load();
# endif	/* defined(HAS_NFS) */

# if	OSRV<504
/*
 * Load the device-inode SYSV file system cache.
 */
	(void) SYSV_load();

#  if	OSRV>=500
/*
 * Load the device-inode DT and HT file system caches.
 */
	(void) DTFS_load();
	(void) HTFS_load();
#  endif	/* OSRV>=500 */
# else	/* OSRV>=504 */
/*
 *  Load the device-inode combined file system cache.
 */
	 (void) DNLC_load();
# endif	/* OSRV<504 */

/*
 * Reduce local name cache memory usage, as required.
 */
	if (LNC_csz < 1) {
	    LNC_csz = 0;
	    if (!RptTm) {
		(void) free((FREE_P *)LNC_nc);
		LNC_nc = (struct lnch *)NULL;
	    }
	    return;
	}
	if (LNC_csz < LNC_asz && !RptTm) {
	    len = LNC_csz * sizeof(struct lnch);
	    if (!(LNC_nc = (struct lnch *)realloc(LNC_nc, len)))
		(void)LNC_nosp(len);
	    LNC_asz = LNC_csz;
	}
/*
 * Initialize hash head pointers.
 */
	if (!LNC_hh) {
	    LNC_hh = (struct lnch_hh *)calloc(LNCHHLEN, sizeof(struct lnch_hh));
	    if (!LNC_hh) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for name cache hash table\n",
		    Pn, LNCHHLEN * sizeof(struct lnch_hh));
		Exit(1);
	    }
	} else
	    (void) zeromem((void *)LNC_hh, (LNCHHLEN * sizeof(struct lnch_hh)));
/*
 * Enter local name cache pointers in the hash table.  Look for entries with
 * the same identifications that have different names.
 */
	for (i = 0, lc = LNC_nc; i < LNC_csz; i++, lc++) {

# if	defined(HAS_NFS)
	    if (lc->type)
		hh = NFS_hash(lc->u.ln.rp);
	    else
# endif	/* defined(HAS_NFS) */

		hh = DIN_hash(lc->u.ld.dev, lc->u.ld.inum);
	    if (!(hl = hh->hp[lc->type])) {
		hh->hp[lc->type] = lc;
		continue;
	    }
	    for (f = 0, hlp = hl; hl; hlp = hl, hl = hl->next) {

# if	defined(HAS_NFS)
		if (lc->type == 1) {
		    if (lc->u.ln.rp != hl->u.ln.rp)
			continue;
		} else
# endif	/* defined(HAS_NFS) */

		{
		    if (lc->u.ld.dev != hl->u.ld.dev
		    ||  lc->u.ld.inum != hl->u.ld.inum)
			continue;
		}
		if (strcmp(lc->nm, hl->nm) == 0)
		    f = 1;
		else {
		    f = 2;	/* same identifiers, different names */
		    break;
		}
	    }
	    if (!f)
		hlp->next = lc;
	    else if (f == 2) {

	    /*
	     * Since entries with the same identification but different names
	     * were located, mark entries with the same identification as
	     * duplicates.
	     */
		for (hl = hh->hp[lc->type]; hl; hl = hl->next) {

# if	defined(HAS_NFS)
		    if (lc->type == 1) {
			if (lc->u.ln.rp == hl->u.ln.rp)
			    hl->dup = 1;
			continue;
		    }
# endif	/* defined(HAS_NFS) */

		    if (hl->u.ld.dev == lc->u.ld.dev
		    &&  hl->u.ld.inum == lc->u.ld.inum)
			hl->dup = 1;
		}
	    }
	}
/*
 * Make a final pass through the local name cache and convert parent
 * identifications to local name cache pointers. Ignore duplicates.
 */
	for (i = 0, lc = LNC_nc; i < LNC_csz; i++, lc++) {
	    if (lc->dup)
		continue;

# if	defined(HAS_NFS)
	    if (lc->type == 1) {
		if (lc->u.ln.dp)
		    lc->pa = NFS_addr(lc->u.ln.dp);
		continue;
	    }
# endif	/* defined(HAS_NFS) */

	    if (lc->u.ld.dev && lc->u.ld.pa_inum)
		lc->pa = DIN_addr(&lc->u.ld.dev, lc->u.ld.pa_inum);
	}
}


/*
 * ncache_lookup() - look up a node's name in the kernel's name caches
 */

char *
ncache_lookup(buf, blen, fp)
	char *buf;			/* receiving name buffer */
	int blen;			/* receiving buffer length */
	int *fp;			/* full path reply */
{
	char *cp = buf;
	int nl, rlen;
	struct lnch *lc;

	*cp = '\0';
	*fp = 0;
/*
 * If the entry has an inode number that matches the inode number of the
 * file system mount point, return an empty path reply.  That tells the
 * caller to print the file system mount point name only.
 */
	if (Lf->inp_ty == 1 && Lf->fs_ino && Lf->inode == Lf->fs_ino)
	    return(cp);
	if (!LNC_nc)
	    return((char *)NULL);

#if	defined(HAS_NFS)
/*
 * Look up the NFS name cache entry.
 */
	if (Lf->is_nfs) {
	    if ((lc = NFS_addr(Lf->na)) && !lc->dup) {
		if ((nl = (int)lc->nl) > (blen - 1))
		    return(*cp ? cp : (char *)NULL);
		cp = buf + blen - nl - 1;
		rlen = blen - nl - 1;
		(void) snpf(cp, nl + 1, "%s", lc->nm);
	    /*
	     * Look up the NFS name cache entries that are parents of the
	     * rnode address.  Quit when:
	     *
	     *	there's no parent;
	     *  the parent is a duplicate;
	     *	the name is too large to fit in the receiving buffer.
	     */
		for (;;) {
		    if (!lc->pa) {
			if (NFS_root(lc->u.ln.dp))
			    *fp = 1;
			break;
		    }
		    lc = lc->pa;
		    if (lc->dup)
			break;
		    if (((nl = (int)lc->nl) + 1) > rlen)
			break;
		    *(cp - 1) = '/';
		    cp--;
		    rlen--;
		    (void) strncpy((cp - nl), lc->nm, nl);
		    cp -= nl;
		    rlen -= nl;
	    	}
		return(*cp ? cp : (char *)NULL);
	    }
	    return((char *)NULL);
	}
# endif	/* defined(HAS_NFS) */

/*
 * Look up the device-inode name cache entry.
 */
	if (Lf->dev_def && Lf->inp_ty == 1
	&&  (lc = DIN_addr(&Lf->dev, Lf->inode)) && !lc->dup) {
	    if ((nl = (int)lc->nl) > (blen - 1))
		return(*cp ? cp : (char *)NULL);
	    cp = buf + blen - nl - 1;
	    rlen = blen - nl - 1;
	    (void) snpf(cp, nl + 1, "%s", lc->nm);
	/*
	 * Look up the LNC name cache entries that are parents of the
	 * device and inode number.  Quit when:
	 *
	 *	there's no parent;
	 *	the parent is a duplicate cache entry;
	 *	the name is too large to fit in the receiving buffer.
	 */
	    for (;;) {
		if (!lc->pa) {
		    if (lc->u.ld.pa_inum && Lf->fs_ino
		    &&  lc->u.ld.pa_inum == Lf->fs_ino)
			*fp = 1;
		    break;
		}
		lc = lc->pa;
		if (lc->dup)
		    break;
		if (lc->u.ld.inum && Lf->fs_ino
		&&  lc->u.ld.inum == Lf->fs_ino) {
		    *fp = 1;
		    break;
		}
		if (((nl = (int)lc->nl) + 1) > rlen)
		    break;
		*(cp - 1) = '/';
		cp--;
		rlen--;
		(void) strncpy((cp - nl), lc->nm, nl);
		cp -= nl;
		rlen -= nl;
	    }
	    return(*cp ? cp : (char *)NULL);
	}
	return((char *)NULL);
}


# if	defined(HAS_NFS)
/*
 * NFS_addr() - look up a node's local NFS_nc address
 */

static struct lnch *
NFS_addr(r)
	KA_T r;				/* rnode's address */
{
	struct lnch_hh *hh = NFS_hash(r);
	struct lnch *lc = hh->hp[1];

	while (lc) {
	    if (lc->u.ln.rp == r)
		return(lc);
	    lc = lc->next;
	}
	return((struct lnch *)NULL);
}


/*
 * NFS_load() -- load kernel's NFS name cache
 */

static void
NFS_load()
{
	struct ncache *cp;
	int i, len;
	struct lnch lc;
	static KA_T ka = (KA_T)NULL;
	static int kcl = 0;
	static struct ncache *nfnc = (struct ncache *)NULL;
	static int nfnce = 0;
	char nm[NC_NAMLEN+1];
	KA_T v;
/*
 * Do first-time setup, as required.
 */
	if (nfnce == 0) {

	/*
	 * Quit if the NFS name cache address is unknown.
	 */
	    if (get_Nl_value("nfnc", Drive_Nl, &ka) < 0 || !ka)
		return;
	/*
	 * Get the kernel's NFS name cache size.
	 */
	    if (get_Nl_value("nnfnc", Drive_Nl, &v) < 0 || !v
	    ||  kread(v, (char *)&nfnce, sizeof(nfnce))
	    ||  nfnce < 1)
		return;
	/*
	 * Allocate space for a copy of the kernel's NFS name cache.
	 */
	    kcl = nfnce * sizeof(struct ncache);
	    if (!(nfnc = (struct ncache *)malloc(kcl))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for NFS name cache\n",
		    Pn, kcl);
		Exit(1);
	    }
	}
/*
 * Read the kernel's NFS name cache.
 */
	if (!nfnc || !kcl || !ka || kread(ka, (char *)nfnc, kcl))
	    return;
/*
 * Build a local copy of the kernel's NFS name cache.
 */
	lc.type = 1;
	for (cp = nfnc, i = 0; i < nfnce; cp++, i++) {
	    if (!cp->rp)
		continue;
	    if ((len = cp->namlen) < 0 || len > NC_NAMLEN)
		continue;
	    (void) strncpy(nm, cp->name, len);
	    nm[len] = '\0';
	    if ((len = strlen(nm)) < 1)
		continue;
	    if (len < 3 && nm[0] == '.') {
		if (len == 1 || (len == 2 && nm[1] == '.'))
		    continue;
	    }
	    lc.u.ln.rp = (KA_T)cp->rp;
	    lc.u.ln.dp = (KA_T)cp->dp;
	    if (LNC_enter(&lc, nm, len, "NFS"))
		break;
	}
/*
 * If not repeating, free kernel NFS name cache buffer space.
 */
	if (nfnc && !RptTm) {
	    (void) free((MALLOC_P *)nfnc);
	    nfnc = (struct ncache *)NULL;
	    kcl = nfnce = 0;
	}
}


static int
NFS_root(r)
	KA_T r;			/* node's rnode address */
{
	int i;
	MALLOC_S len;
	static int rnc = 0;
	static int rnca = 0;
	static KA_T *rc = (KA_T *)NULL;
	struct rnode rn;
	unsigned short *n;
	unsigned long nnum;

# if	OSRV>=500
	unsigned short *n1;
# endif	/* OSRV>=500 */

	if (!Lf->fs_ino || !r)
	    return(0);
/*
 * Search NFS root rnode cache.
 */
	for (i = 0; i < rnc; i++) {
	    if (rc[i] == r)
		return(1);
	}
/*
 * Read rnode and get the node number.
 */
	if (kread(r, (char *)&rn, sizeof(rn)))
	    return(0);

# if	OSRV<500
	n = (unsigned short *)&rn.r_fh.fh_pad[14];
	if (!(nnum = (unsigned long)ntohs(*n)))
	    nnum = (unsigned long)rn.r_fh.fh_u.fh_fgen_u;
# else	/* OSRV>=500 */
	n = (unsigned short *)&rn.r_fh.fh_u.fh_fid_u[4];
	n1 = (unsigned short *)&rn.r_fh.fh_u.fh_fid_u[2];
	if (!(nnum = (unsigned long)*n))
	    nnum = (unsigned long)*n1;
# endif	/* OSRV<500 */

	if (!nnum || nnum != Lf->fs_ino)
	    return(0);
/*
 * Add the rnode address to the NFS root rnode cache.
 */
	if (rnc >= rnca) {
	    if (rnca == 0) {
		len = (MALLOC_S)(10 * sizeof(KA_T));
		if ((rc = (KA_T *)malloc(len)))
		    rnca = 10;
	    } else {
		len = (MALLOC_S)((rnca + 10) * sizeof(KA_T));
		if ((rc = (KA_T *)realloc(rc, len)))
		    rnca += 10;
	    }
	    if (!rc) {
		(void) fprintf(stderr, "%s: no space for root rnode table\n",
		    Pn);
		Exit(1);
	    }
	}
	rc[rnc++] = r;
	return(1);
}
# endif	/* defined(HAS_NFS) */


# if	OSRV<504
/*
 * SYSV_load() - load SYSV cache
 */

static void
SYSV_load()
{
	struct s5cachent *cp;
	int i, len;
	static KA_T ka = (KA_T)NULL;
	static int kcl = 0;
	struct lnch lc;
	char nm[DIRSIZ+1];
	static struct s5cachent *s5nc = (struct s5cachent *)NULL;
	static int s5nce = 0;
	KA_T v;
/*
 * Do first-time setup, as required.
 */
	if (s5nce == 0) {
	
	/*
	 * Quit if the SYSV name cache address is unknown.
	 */
	    if (get_Nl_value("s5nc", Drive_Nl, &ka) < 0 || !ka)
		return;
	/*
	 * Get the kernel's SYSV name cache size.
	 */

#  if	OSRV<500
	    s5nce = Var.v_s5cacheents;	
#  else	/* OSRV>=500 */
	    if (get_Nl_value("nsfnc", Drive_Nl, &v) < 0 || !v
	    ||  kread(v, (char *)&s5nce, sizeof(s5nce)))
		return;
#  endif	/* OSRV<500 */

	    if (s5nce < 1)
		return;
	/*
	 * Allocate space for a copy of the kernel's SYSV name cache.
	 */
	    kcl = sizeof(struct s5cachent) * s5nce;
	    if (!(s5nc = (struct s5cachent *)malloc(kcl))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for SYSV name cache\n",
		    Pn, kcl);
		Exit(1);
	    }
	}
/*
 * Read the kernel's SYSV name cache.
 */
	if (!s5nc || !kcl || !ka || kread(ka, (char *)s5nc, kcl))
	    return;
/*
 * Build a local copy of the kernel's SYSV name cache.
 */
	nm[DIRSIZ] = '\0';
	lc.type = 0;
	for (cp = s5nc, i = 0; i < s5nce; cp++, i++) {
	    if (!cp->dev && !cp->newinum)
		continue;
	    (void) strncpy(nm, cp->name, DIRSIZ);
	    if ((len = strlen(nm)) < 1)
		continue;
	    if (len < 3 && cp->name[0] == '.') {
		if (len == 1 || (len == 2 && cp->name[1] == '.'))
		    continue;
	    }
	    lc.u.ld.dev = (dev_t)cp->dev;
	    lc.u.ld.inum = (unsigned long)cp->newinum;
	    lc.u.ld.pa_inum = (unsigned long)cp->inum;
	    if (LNC_enter(&lc, nm, len, "SYSV"))
		break;
	}
/*
 * If not repeating, free kernel SYSV name cache buffer space.
 */
	if (s5nc && !RptTm) {
	    (void) free((MALLOC_P *)s5nc);
	    s5nc = (struct s5cachent *)NULL;
	    kcl = s5nce = 0;
	}
}
# endif	/* OSRV<504 */
#endif	/* defined(HASNCACHE) */
