/*
 * dproc.c - /dev/kmem-based HP-UX process access functions for lsof
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
static char *rcsid = "$Id: dproc.c,v 1.18 2008/10/08 13:24:36 abe Exp $";
#endif

#if	defined(HPUXKERNBITS)
# if	HPUXKERNBITS>=64
#define _INO_T
typedef int ino_t;
#define _TIME_T
typedef int time_t;
# else	/* HPUXKERNBITS<64 */
#define	_RLIM_T
#  if !defined(__STDC_32_MODE__)
typedef unsigned long long rlim_t;
#  else	/* defined(__STDC_32_MODE__) */
typedef	unsigned long rlim_t;
#  endif	/* !defined(__STDC_32_MODE__) */
# endif	/* HPUXKERNBITS>=64 */
#endif	/* defined(HPUXKERNBITS) */

#include "lsof.h"

#if     defined(HASNCACHE)
#include <sys/dnlc.h>
#endif  /* defined(HASNCACHE) */


#if	HPUXV>=1010
/*
 * HP doesn't include a definition for the proc structure in HP-UX 10.10
 * or above in an attempt to force use of pstat(2).  Unfortunately, pstat(2)
 * doesn't return the information lsof needs.  Hence, this private proc
 * structure definition.
 */

#include <sys/vas.h>

#define	SZOMB	3

# if	HPUXV<1020
struct proc {
	caddr_t d1[2];			/* dummy to occupy space */
	caddr_t p_firstthreadp;		/* thread pointer */
	caddr_t d2[4];			/* dummy to occupy space */
	int p_stat;			/* process status */
	caddr_t d3[9];			/* dummy to occupy space */
	uid_t p_uid;			/* UID */
	caddr_t d4[2];			/* dummy to occupy space */
	gid_t p_pgid;			/* process group ID */
	pid_t p_pid;			/* PID */
	pid_t p_ppid;			/* parent PID */
	caddr_t d5[9];			/* dummy to occupy space */
	vas_t *p_vas;			/* virtual address space */
	caddr_t d6[16];			/* dummy to occupy space */
	int p_maxof;			/* max open files allowed */
	struct vnode *p_cdir;		/* current directory */
	struct vnode *p_rdir;		/* root directory */
	struct ofile_t **p_ofilep;	/* file descriptor chunks */
	caddr_t d7[43];			/* dummy to occupy space */
};
# endif	/* HPUXV<1020 */

# if	HPUXV>=1020 && HPUXV<1030
struct proc {
	caddr_t d1[2];			/* dummy to occupy space */
	caddr_t p_firstthreadp;		/* thread pointer */
	caddr_t d2[6];			/* dummy to occupy space */
	int p_stat;			/* process status */
	caddr_t d3[14];			/* dummy to occupy space */
	uid_t p_uid;			/* real UID */
	uid_t p_suid;			/* effective UID */
	caddr_t d4;			/* dummy to occupy space */
	gid_t p_pgid;			/* process group ID */
	pid_t p_pid;			/* PID */
	pid_t p_ppid;			/* parent PID */
	caddr_t d5[9];			/* dummy to occupy space */
	vas_t *p_vas;			/* virtual address space */
	caddr_t d6[16];			/* dummy to occupy space */
	int p_maxof;			/* max open files allowed */
	struct vnode *p_cdir;		/* current directory */
	struct vnode *p_rdir;		/* root directory */
	struct ofile_t **p_ofilep;	/* file descriptor chunks */
	caddr_t d7[84];			/* dummy to occupy space */
};
# endif	/* HPUXV>=1020 && HPUXV<1030 */
#endif	/* HPUXV<1010 */


/*
 * Local static values
 */

static KA_T Kp;				/* kernel's process table address */
static int Np;				/* number of kernel processes */

#if	HPUXV>=800
static MALLOC_S Nva = 0;		/* number of entries allocated to
					 * vnode address cache */
static KA_T *Vp = (KA_T *)NULL;		/* vnode address cache */
#endif	/* HPUXV>=800 */


_PROTOTYPE(static void get_kernel_access,(void));

#if	HPUXV>=800
_PROTOTYPE(static void process_text,(KA_T vasp));
#endif	/* HPUXV>=800 */


/*
 * gather_proc_info() -- gather process information
 */

void
gather_proc_info()
{
	KA_T fp;
	int err, i, j;

#if	HPUXV>=1020 && HPUXV<1100
	struct ofile_t {
	    struct ofa {
		KA_T ofile;
		int d1;
		int pofile;
	    } ofa[SFDCHUNK];
	};
	struct ofa *ofap;
	int ofasz = (int)sizeof(struct ofa);
	struct ofile_t oft;
	char *oftp = (char *)&oft;
	int oftsz = (int)sizeof(struct ofile_t);
#else	/* HPUXV<1020 || HPUXV>=1100 */
# if	HPUXV>=1100
	struct ofa {
	    KA_T ofile;
	    int d1;
	    short d2;
	    char d3;
	    char pofile;
	};
	struct ofa *ofap;
	int ofasz = (int)sizeof(struct ofa);
	char *oftp = (char *)NULL;
	int oftsz = (int)(sizeof(struct ofa) * SFDCHUNK);
	KA_T v;
# endif	/* HPUXV>=1100 */
#endif	/* HPUXV>=1020 && HPUXV<1100 */

#if	HPUXV>=800
	char *c, *s;
	KA_T pfp, ofp;

#if	HPUXV<1020
	struct ofile_t oft;
	char *oftp = (char *)&oft;
	int oftsz = (int)sizeof(struct ofile_t);
#endif	/* HPUXV<1020 */

	struct pst_status ps;

# if	HPUXV<1010
	struct user us;
# else	/* HPUXV>=1010 */
	struct user {
		char u_comm[PST_CLEN];
	} us;
# endif	/* HPUXV<1010 */
#else	/* HPUXV<800 */
	int k;
	long sw;
	char us[U_SIZE];	/* must read HP-UX SWAP in DEV_BSIZE chunks */

# if	defined(hp9000s300)
	struct pte pte1, pte2;
	KA_T pte_off, pte_addr;
# endif	/* defined(hp9000s300) */
#endif	/* HPUXV>=800 */

	struct proc *p;
	struct proc pbuf;
	short pss, sf;
	int px;
	struct user *u;

#if	defined(HASFSTRUCT)
# if	HPUXV>=1020 || (HPUXV>=900 && HPUXV<1000)
#define	USESPOFILE	1
	long pof;
# endif	/* HPUXV>=1020 || (HPUXV>=900 && HPUXV<1000) */
#endif	/* defined(HASFSTRUCT) */

#if	HPUXV>=1100
/*
 * Define FD chunk size and pointer for HP-UX >= 11.
 */
	if (!oftp) {
	    if ((get_Nl_value("chunksz", Drive_Nl, &v) >= 0) && v) {
		if (kread(v, (char *)&oftsz, sizeof(oftsz))) {
		    (void) fprintf(stderr, "%s: can't get FD chunk size\n",
			Pn);
		    Exit(1);
		}
		if (!oftsz) {
		    (void) fprintf(stderr, "%s: bad FD chunk size: %d\n",
			Pn, oftsz);
		    Exit(1);
		}
	    }
	    ofasz = (int)(oftsz / SFDCHUNK);
	    if (oftsz != (ofasz * SFDCHUNK)) {
		(void) fprintf(stderr,
		    "%s: FD chunk size (%d) not exact multiple of %d\n",
		    Pn, oftsz, SFDCHUNK);
		Exit(1);
	    }
	    if (!(oftp = (char *)malloc((MALLOC_S)oftsz))) {
		(void) fprintf(stderr, "%s: no space for %d FD bytes\n",
		    Pn, oftsz);
		Exit(1);
	    }
	}
#endif	/* HPUXV>=1100 */

/*
 * Examine proc structures and their associated information.
 */

#if	HPUXV>=800
	u = &us;
	(void) zeromem((char *)u, U_SIZE);
	for (p = &pbuf, px = 0; px < Np; px++)
#else	/* HPUXV<800 */
	for (p = &pbuf, px = 0, u = (struct user *)us; px < Np; px++)
#endif	/* HPUXV>=800 */

	{
	    Kpa = Kp + (KA_T)(px * sizeof(struct proc));
	    if (kread(Kpa, (char *)&pbuf, sizeof(pbuf)))
		continue;
	    if (p->p_stat == 0 || p->p_stat == SZOMB)
		continue;
	/*
	 * See if process is excluded.
	 */
	    if (is_proc_excl(p->p_pid, (int)p->p_pgid, (UID_ARG)p->p_uid,
			     &pss, &sf))
		continue;

#if	HPUXV>=1010
	/*
	 * Save the kernel thread pointer.
	 */
	    Ktp = (KA_T)p->p_firstthreadp;
#endif	/* HPUXV>=1010 */

	/*
	 * Read the user area.
	 */

#if	HPUXV>=800
	/*
	 * Use the pstat() syscall to read process status.
	 */

	    if (pstat(PSTAT_PROC, &ps, sizeof(ps), 0, p->p_pid) != 1) {
	 	if (!Fwarn)
		    (void) fprintf(stderr, "%s: can't pstat process %d: %s\n",
			Pn, p->p_pid, strerror(errno));
		continue;
	    }
	/*
	 * Use the pst_cmd command buffer.
	 */
	    c = ps.pst_cmd;
	    ps.pst_cmd[PST_CLEN - 1] = '\0';	/* paranoia */
	/*
	 * Skip to the last component of the first path name.  Also skip any
	 * leading `-', signifying a login shell.  Copy the result to u_comm[].
	 */
	    if (*c == '-')
		c++;
	    for (s = c; *c && (*c != ' '); c++) {
		if (*c == '/')
		    s = c + 1;
	    }
	    for (i = 0; i < MAXCOMLEN; i++) {
		if (*s == '\0' || *s == ' ' || *s == '/')
		    break;
		u->u_comm[i] = *s++;
	    }
	    u->u_comm[i] = '\0';
#else	/* HPUXV<800 */
	/*
	 * Read the user area from the swap file or memory.
	 */
	    if ((p->p_flag & SLOAD) == 0) {

	    /*
	     * If the process is not loaded, read the user area from the swap
	     * file.
	     */
		if (Swap < 0)
		    continue;
		sw = (long)p->p_swaddr;

# if	defined(hp9000s800)
	    sw += (long)ctod(btoc(STACKSIZE * NBPG));
# endif	/* defined(hp9000s800) */

		if (lseek(Swap, (off_t)dtob(sw), L_SET) == (off_t)-1
		||  read(Swap, u, U_SIZE) != U_SIZE)
		    continue;
	    } else {

	    /*
	     * Read the user area via the page table.
	     */

# if	defined(hp9000s300)
		pte_off = (KA_T) &Usrptmap[btokmx(p->p_p0br) + p->p_szpt - 1];
		if (kread(pte_off, (char *)&pte1, sizeof(pte1)))
		    continue;
		pte_addr = (KA_T)(ctob(pte1.pg_pfnum + 1)
			 - ((UPAGES + FLOAT) * sizeof(pte2)));
		if (mread(pte_addr, (char *)&pte2, sizeof(pte2)))
		    continue;
		if (mread((KA_T)ctob(pte2.pg_pfnum), (char *)u,
			  sizeof(struct user)))
		    continue;
# endif	/* defined(hp9000s300) */

# if	defined(hp9000s800)
	        if (kread((KA_T)uvadd((struct proc *)Kpa), (char *)u,
			  sizeof(struct user)))
		    continue;
	    }
# endif	/* defined(hp9000s800) */
#endif	/* HPUXV>=800 */

	/*
	 * Allocate a local process structure.
	 */
	    if (is_cmd_excl(u->u_comm, &pss, &sf))
		continue;
	    alloc_lproc(p->p_pid, (int)p->p_pgid, (int)p->p_ppid,
			(UID_ARG)p->p_uid, u->u_comm, (int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;
	/*
	 * Save current working directory information.
	 */
	    if (CURDIR) {
		alloc_lfile(CWD, -1);
		process_node((KA_T)CURDIR);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (ROOTDIR) {
		alloc_lfile(RTD, -1);
		process_node((KA_T)ROOTDIR);
		if (Lf->sf)
		    link_lfile();
	    }

#if	HPUXV>=800
	/*
	 * Print information on the text file.
	 */
	    if (p->p_vas)
		process_text((KA_T)p->p_vas);
#endif	/* HPUXV>=800 */

	/*
	 * Loop through user's files.
	 */

#if	HPUXV>=800
	    for (i = 0, j = SFDCHUNK, pfp = (KA_T)p->p_ofilep;
		 i < p->p_maxof;
		 i++)
#else	/* HPUXV<800 */
	    for (i = j = k = 0;; i++)
#endif	/* HPUXV>=800 */

	    {

#if	HPUXV>=800
		if (j >= SFDCHUNK) {
		    if (!pfp || kread((KA_T)pfp, (char *)&ofp, sizeof(ofp))
		    ||  !ofp || kread((KA_T)ofp, oftp, oftsz))
			break;
		    j = 0;
		    pfp += sizeof(KA_T);

# if	HPUXV>=1020
		    ofap = (struct ofa *)oftp;
# endif	/* HPUXV>=1020 */

		}
		j++;

# if	HPUXV>=1020
#  if	defined(USESPOFILE)
		pof = (long)ofap->pofile;
#  endif	/* defined(USESPOFILE) */

		fp = (KA_T)ofap->ofile;
		ofap = (struct ofa *)((char *)ofap + ofasz);
		if (fp)
# else	/* HPUXV<1020 */
#  if	defined(USESPOFILE)
		pof = (long)oft.pofile[j - 1];
#  endif	/* defined(USESPOFILE) */

		if ((fp = (KA_T)oft.ofile[j - 1]))
# endif	/* HPUXV>=1020 */
#else	/* HPUXV<800 */
		if (j >= SFDCHUNK) {

		/*
		 * Get next file pointer "chunk".
		 */
		    while (++k < NFDCHUNKS && !u->u_ofilep[k])
			;
		    if (k >= NFDCHUNKS)
			break;
		    if (kread((KA_T)u->u_ofilep[k], (char *)&u->u_ofile,
			      sizeof(struct ofile_t)))
		    {
			break;
		    }
		    j = 0;
		}
		j++;
		if ((fp = (KA_T)u->u_ofile.ofile[j - 1]))
#endif	/* HPUXV>=800 */

		/*
		 * Process the file pointer.
		 */

		{
		    alloc_lfile(NULL, i);
		    process_file(fp);
		    if (Lf->sf) {

#if	defined(USESPOFILE)
			if (Fsv & FSV_FG)
			    Lf->pof = pof;
#endif	/* defined(USESPOFILE) */

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
 * get_kernel_access() - access the required information in the kernel
 */

static void
get_kernel_access()
{
	KA_T v;
/*
 * Check the kernel version.
 */
	(void) ckkv("HP-UX", LSOF_VSTR, (char *)NULL, (char *)NULL);

#if	HPUXV>=1030
/*
 * See if build and run bit sizes match.  Exit if they don't.
 */
	{
	    long rv;

	    if ((rv = sysconf(_SC_KERNEL_BITS)) < 0) {
		(void) fprintf(stderr,
		    "%s: sysconf(_SC_KERNEL_BITS) returns: %s\n",
		    Pn, strerror(errno));
		Exit(1);
	    }
	    if (rv != (long)HPUXKERNBITS) {
		(void) fprintf(stderr,
		    "%s: FATAL: %s was built for a %d bit kernel, but this\n",
		    Pn, Pn, HPUXKERNBITS);
		(void) fprintf(stderr, "      is a %ld bit kernel.\n", rv);
		Exit(1);
	    }
	}
#endif	/* HPUXV>=1030 */

#if	defined(HAS_AFS)
	struct NLIST_TYPE *nl = (struct NLIST_TYPE *)NULL;
#endif	/* defined(HAS_AFS) */

#if	HPUXV<800
/*
 * Open access to /dev/mem and SWAP.
 */
	if ((Mem = open("/dev/mem", O_RDONLY, 0)) < 0) {
	    (void) fprintf(stderr, "%s: can't open /dev/mem: %s\n",
		Pn, strerror(errno));
	    err = 1;
	}
	if (!Memory || strcmp(Memory, KMEM) == 0) {
	    if ((Swap = open(SWAP, O_RDONLY, 0)) < 0) {
		(void) fprintf(stderr, "%s: %s: %s\n",
		    Pn, SWAP, strerror(errno));
		err = 1;
	    }
	}
#endif	/* HPUXV<800 */

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
	    int errno_save = errno;

	    (void) fprintf(stderr, "%s: can't open ", Pn);
	    safestrprt(Memory ? Memory : KMEM, stderr, 0);
	    (void) fprintf(stderr, ": %s\n", strerror(errno_save));
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

	(void) build_Nl(Drive_Nl);

#if	defined(HAS_AFS)
	if (!Nmlst) {

	/*
	 * If AFS is defined and we're getting kernel symbol values from
	 * from N_UNIX, make a copy of Nl[] for possible use with the AFS
	 * module name list file.
	 */
	    if (!(nl = (struct NLIST_TYPE *)malloc(Nll))) {
		(void) fprintf(stderr,
		    "%s: no space (%d) for Nl[] copy\n", Pn, Nll);
		Exit(1);
	    }
	    (void) memcpy((void *)nl, (void *)Nl, (size_t)Nll);
	}
#endif	/* defined(HAS_AFS) */

/*
 * Access kernel symbols.
 */
	if (NLIST_TYPE(Nmlst ? Nmlst : N_UNIX, Nl) < 0) {
	    (void) fprintf(stderr, "%s: can't read namelist from: ", Pn);
	    safestrprt(Nmlst ? Nmlst : N_UNIX, stderr, 1);
            Exit(1);
	}
	if (get_Nl_value("proc", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&Kp, sizeof(Kp))
	||  get_Nl_value("nproc", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&Np, sizeof(Np))
	||  !Kp || Np < 1) {
	    (void) fprintf(stderr, "%s: can't read proc table info\n", Pn);
	    Exit(1);
	}
	if (get_Nl_value("vfops", Drive_Nl, (KA_T *)&Vnfops) < 0)
	    Vnfops = (KA_T)NULL;

#if	HPUXV<800 && defined(hp9000s300)
	if (get_Nl_value("upmap", Drive_Nl, (unsigned long *)&Usrptmap) < 0) {
	    (void) fprintf(stderr, "%s: can't get kernel's Usrptmap\n", Pn);
	    Exit(1);
	}
	if (get_Nl_value("upt", Drive_Nl, (unsigned long *)&usrpt) < 0) {
	    (void) fprintf(stderr, "%s: can't get kernel's usrpt\n", Pn);
	    Exit(1);
	}
#endif	/* HPUXV<800 && defined(hp9000s300) */

#if	HPUXV<800 && defined(hp9000s800)
	proc = (struct proc *)Kp;
	if (get_Nl_value("ubase", Drive_Nl, (unsigned long *)&ubase) < 0) {
	    (void) fprintf(stderr, "%s: can't get kernel's ubase\n", Pn);
	    Exit(1);
	}
	if (get_Nl_value("npids", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&npids, sizeof(npids))) {
	    (void) fprintf(stderr, "%s: can't get kernel's npids\n", Pn);
	    Exit(1);
	}
#endif	/* HPUXV<800 && defined(hp9000s800) */

#if	HPUXV>=1030
	if (get_Nl_value("clmaj", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&CloneMaj, sizeof(CloneMaj)))
	    HaveCloneMaj = 0;
	else
	    HaveCloneMaj = 1;
#endif	/* HPUXV>=1030 */

#if	defined(HAS_AFS)
	if (nl) {

	/*
	 * If AFS is defined and we're getting kernel symbol values from
	 * N_UNIX, and if any X_AFS_* symbols isn't there, see if it is in the
	 * the AFS module name list file.  Make sure that other symbols that
	 * appear in both name list files have the same values.
	 */
	    if ((get_Nl_value("arFid", Drive_Nl, &v) >= 0 && !v)
	    ||  (get_Nl_value("avops", Drive_Nl, &v) >= 0 && !v)
	    ||  (get_Nl_value("avol", Drive_Nl, &v) >= 0 && !v))
		(void) ckAFSsym(nl);
	    (void) free((FREE_P *)nl);
	}
#endif	/* defined(HAS_AFS) */

}


/*
 * initialize() - perform all initialization
 */

void
initialize()
{
	get_kernel_access();
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

	if (lseek(Kd, (off_t)addr, L_SET) == (off_t)-1L)
	    return(-1);
	br = read(Kd, buf, len);
	return((br == len) ? 0 : 1);
}


#if	HPUXV<800
/*
 * mread() -- read from /dev/mem
 */

static int
mread(addr, buf, len)
	KA_T addr;			/* /dev/mem address */
	char *buf;			/* buffer to receive data */
	READLEN_T len;			/* length to read */
{
	int br;

	if (lseek(Mem, addr, L_SET) == (off_t)-1L)
	    return(1);
	br = read(Mem, buf, len);
	return((br == len) ? 0 : 1);
}
#endif	/* HPUXV<800 */


#if	HPUXV>=800
/*
 * process_text() - process text access information
 */

static void
process_text(vasp)
	KA_T vasp;			/* kernel's virtual address space
					 * pointer */
{
	char fd[FDLEN];
	int i, j, lm;
	MALLOC_S len;
	struct pregion p;
	KA_T prp;
	struct region r;
	struct vas v;
	KA_T va;
/*
 * Read virtual address space pointer.
 */
	if (kread(vasp, (char *)&v, sizeof(v)))
	    return;
/*
 * Follow the virtual address space pregion structure chain.
 */
	for (i = lm = 0, prp = (KA_T)v.va_next;
	     prp != vasp;
	     prp = (KA_T)p.p_next, lm++)
	{

	/*
	 * Avoid infinite loop.
	 */
	    if (lm > 1000) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: too many virtual address regions for PID %d\n",
			Pn, Lp->pid);
		return;
	    }
	/*
	 * Read the pregion and region.
	 */
	    if (kread(prp, (char *)&p, sizeof(p)))
		return;
	    if (kread((KA_T)p.p_reg, (char *)&r, sizeof(r)))
		return;
	/*
	 * Skip file entries with no file pointers.
	 */
	    if (!(va = (KA_T)r.r_fstore))
		continue;
	/*
	 * Skip entries whose vnodes have already been displayed.
	 *
	 *  Record new, unique vnode pointers.
	 */
	    for (j = 0; j < i; j++) {
		if (Vp[j] == va)
		    break;
	    }
	    if (j < i)
		continue;
	    if (i >= Nva) {
		Nva += 10;
		len = (MALLOC_S)(Nva * sizeof(KA_T));
		if (!Vp)
		    Vp = (KA_T *)malloc(len);
		else
		    Vp = (KA_T *)realloc((MALLOC_P *)Vp, len);
		if (!Vp) {
		    (void) fprintf(stderr,
			"%s: no more space for text vnode pointers\n", Pn);
		    Exit(1);
		}
	    }
	    Vp[i++] = va;
	/*
	 * Allocate local file structure.
	 */
	    switch (p.p_type) {
	    case PT_DATA:
	    case PT_TEXT:
		alloc_lfile(" txt", -1);
		break;
	    case PT_MMAP:
		alloc_lfile(" mem", -1);
		break;
	    default:
		(void) snpf(fd, sizeof(fd), "R%02d", p.p_type);
		alloc_lfile(fd, -1);
	    }
	/*
	 * Save vnode information.
	 */
	    process_node(va);
	    if (Lf->sf)
		link_lfile();
	}
}
#endif	/* HPUXV>=800 */
