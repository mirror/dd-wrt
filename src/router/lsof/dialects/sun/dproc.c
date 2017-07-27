/*
 * dproc.c - Solaris lsof functions for accessing process information
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
static char *rcsid = "$Id: dproc.c,v 1.36 2010/01/18 19:03:54 abe Exp $";
#endif

#include "lsof.h"

#if	solaris<20500
#include "kernelbase.h"
#endif	/* solaris<20500 */

#if	defined(HAS_CRED_IMPL_H)
# if	solaris>=110000
#define	_KERNEL
# endif	/* solaris>=110000 */

#include <sys/cred_impl.h>

# if	solaris>=110000
#undef	_KERNEL
# endif	/* solaris>=110000 */
#endif	/* defined(HAS_CRED_IMPL_H) */


/*
 * Local definitions
 */

#if	defined(__sparc) || defined(__sparcv9)
#define	ARCH64B	"sparcv9"
#else	/* !defined(__sparc) && !defined(__sparcv9) */
# if	defined(__i386) || defined(__amd64)
#define	ARCH64B	"amd64"
# endif	/* defined(__i386) || defined(__amd64) */
#endif	/* defined(__sparc) || defined(__sparcv9) */

#if	solaris>=20501
#define	KVMHASHBN	8192		/* KVM hash bucket count -- MUST BE
					 * A POWER OF 2!!! */
#define	HASHKVM(va)	((int)((va * 31415) >> 3) & (KVMHASHBN-1))
					/* virtual address hash function */

# if	solaris<70000
#define	KAERR	(u_longlong_t)-1	/* kvm_physaddr() error return */
#define	KBUFT	char			/* kernel read buffer type */
#define	KPHYS	u_longlong_t		/* kernel physical address type */
#define	KVIRT	u_int			/* kernel virtual address type */
# else	/* solaris>=70000 */
#define	KAERR	(uint64_t)-1		/* kvm_physaddr() error return */
#define	KBUFT	void			/* kernel read buffer type */
#define	KPHYS	uint64_t		/* kernel physical address type */
#define	KVIRT	uintptr_t		/* kernel virtual address type */
# endif	/* solaris<70000 */
#endif	/* solaris>=20501 */


/*
 * Local structures
 */

#if	solaris>=20501
typedef struct kvmhash {
	KVIRT vpa;			/* virtual page address */
	KPHYS pa;			/* physical address */
	struct kvmhash *nxt;		/* next virtual address */
} kvmhash_t;
#endif	/* solaris>=20501 */


/*
 * Local variables
 */

#if	solaris>=20501
static struct as *Kas = (struct as *)NULL;
					/* pointer to kernel's address space
					 * map in kernel virtual memory */
static kvmhash_t **KVMhb = (kvmhash_t **)NULL;
					/* KVM hash buckets */
static int PageSz = 0;			/* page size */
static int PSMask = 0;			/* page size mask */
static int PSShft = 0;			/* page size shift */

# if	solaris<70000
static struct as Kam;			/* kernel's address space map */
static int Kmd = -1;			/* memory device file descriptor */
# endif	/* solaris<70000 */
#endif	/* solaris>=20501 */

#if	solaris>=20500
static KA_T Kb = (KA_T)NULL;		/* KERNELBASE for Solaris 2.5 */
#endif	/* solaris>=20500 */

static int Np;				/* number of P[], Pgid[] and Pid[]
					 * entries  */
static int Npa = 0;			/* number of P[], Pgid[] and Pid[]
					 * entries for which space has been
					 * allocated */
static struct proc *P = NULL;		/* local proc structure table */
static int *Pgid = NULL;		/* process group IDs for P[] entries */
static int *Pid = NULL;			/* PIDs for P[] entries */
static KA_T PrAct = (KA_T)NULL;		/* kernel's *practive address */
static gid_t Savedgid;			/* saved (effective) GID */
static KA_T Sgvops;			/* [_]segvn_ops address */
static int Switchgid = 0;		/* must switch GIDs for kvm_open() */

#if	defined(HASZONES)
static znhash_t **ZoneNm = (znhash_t **)NULL;
					/* zone names hash buckets */
#endif	/* defined(HASZONES) */


/*
 * Local function prototypes
 */

_PROTOTYPE(static void get_kernel_access,(void));
_PROTOTYPE(static void process_text,(KA_T pa));
_PROTOTYPE(static void read_proc,(void));
_PROTOTYPE(static void readfsinfo,(void));

#if	solaris>=20501
_PROTOTYPE(static void readkam,(KA_T addr));
#endif	/* solaris>=20501 */

#if	solaris>=20501 && solaris<70000
_PROTOTYPE(extern u_longlong_t kvm_physaddr,(kvm_t *, struct as *, u_int));
#endif	/* solaris>=20501 && solaris<70000 */

#if	defined(HASZONES)
_PROTOTYPE(static int hash_zn,(char *zn));
#endif	/* defined(HASZONES) */



/*
 * close_kvm() - close kernel virtual memory access
 */

void
close_kvm()
{
	if (!Kd)
	    return;
	if (Kd) {
	    if (kvm_close(Kd) != 0) {
		(void) fprintf(stderr, "%s: kvm_close failed\n", Pn);
		Exit(1);
	    }
	    Kd = (kvm_t *)NULL;
	}

#if	solaris>=20501 && solaris<70000
	if (Kmd >= 0) {
	    (void) close(Kmd);
	    Kmd = -1;
	}
#endif	/* solaris>=20501 && solaris<70000 */

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
	static int ft = 1;
	int i, j;
	struct proc *p;
	int pgid, pid, px;
	long pofv;
	short pss, sf;
	struct user *u;
	uid_t uid;

#if	solaris>=20400
	int k;

# if	!defined(NFPCHUNK)
#define	uf_ofile	uf_file
#define	uf_pofile	uf_flag
#define	u_flist		u_finfo.fi_list
#define	u_nofiles	u_finfo.fi_nfiles
#define	NFPREAD		64
# else	/* defined(NFPCHUNK) */
#define	NFPREAD		NFPCHUNK
# endif	/* !defined(NFPCHUNK) */
	uf_entry_t uf[NFPREAD];
#endif	/* solaris>=20400 */
#if	solaris>=20500
	struct cred pc;
#endif	/* solaris>=20500 */

#if	defined(HASZONES)
	struct zone z;
	int zh;
	char zn[ZONENAME_MAX + 1];
	znhash_t *zp, *zpn;
#endif	/* defined(HASZONES) */

	if (ft) {
/*
 * Do first-time only operations.
 */
	/*
	 * Get the segment vnodeops address.
	 */
	    if (get_Nl_value("sgvops", Drive_Nl, &Sgvops) < 0)
		Sgvops = (KA_T)NULL;
	    ft = 0;
	} else if (!HasALLKMEM) {

	/*
	 * If not the first time and the ALLKMEM device isn't available, it is
	 * necessary to close and reopen the KVM device, so that kvm_open()
	 * will acquire a fresh address for the head of the linked list process
	 * table.
	 */
	    close_kvm();
	    open_kvm();

#if	solaris>=20501
	/*
	 * If not the first time and the ALLKMEM device isn't available,
	 * re-read the kernel's address space map.
	 */
	    readkam((KA_T)NULL);
#endif	/* solaris>=20501 */

	}
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
	read_proc();
/*
 * Loop through processes.
 */
	for (p = P, px = 0; px < Np; p++, px++) {

	/*
	 * Get the process ID.
	 */

	    if (Fpgid)
		pgid = Pgid[px];
	    else
		pgid = 0;
	    pid = Pid[px];

#if solaris<20500
	    uid = p->p_uid;
#else	/* solaris >=20500 */
	/*
	 * Read credentials for Solaris 2.5 and above process.
	 */
	    if (kread((KA_T)p->p_cred, (char *)&pc, sizeof(pc)))
		continue;
	    uid = pc.cr_uid;
#endif	/* solaris<20500 */

	/*
	 * See if the process is excluded.
	 */
	    if  (is_proc_excl(pid, pgid, (UID_ARG)uid, &pss, &sf))
		continue;

#if	defined(HASZONES)
	/*
	 * If the -z (zone) option was specified, get the zone name.
	 */
	    if (Fzone) {
		zn[0] = zn[sizeof(zn) - 1] = '\0';
		if (p->p_zone
		&& !kread((KA_T)p->p_zone, (char *)&z, sizeof(z)))
		{
		    if (!z.zone_name
		    ||  kread((KA_T)z.zone_name, (char *)&zn, sizeof(zn) - 1))
			zn[0] = '\0';
		}
	    }
#endif	/* defined(HASZONES) */

	/*
	 * Get the user area associated with the process.
	 */
	    u = &p->p_user;
	/*
	 * Allocate a local process structure and start filling it.
	 */
	    if (is_cmd_excl(u->u_comm, &pss, &sf))
		continue;
	    if (cckreg) {

	    /*
	     * If conditional checking of regular files is enabled, enable
	     * socket file only checking, based on the process' selection
	     * status.
	     */
		ckscko = (sf & SELPROC) ? 0 : 1;
	    }
	    alloc_lproc(pid, pgid, (int)p->p_ppid, (UID_ARG)uid, u->u_comm,
		(int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;

#if	defined(HASZONES)
	/*
	 * If zone processing is enabled and requested, and if there is a zone
	 * name:
	 *
	 *	o Skip processes excluded by zone name.
	 *	o Save zone name.
	 */
	    if (Fzone && zn[0]) {
		zh = hash_zn(zn);
		if (ZoneArg) {

		/*
		 * See if zone name excludes the process.
		 */
		    for (zp = ZoneArg[zh]; zp; zp = zp->next) {
			if (!strcmp(zn, zp->zn))
			    break;
		    }
		    if (!zp)
			continue;
		    zp->f = 1;
		    Lp->pss |= PS_PRI;
		    Lp->sf |= SELZONE;
		}
	    /*
	     * Make sure the zone name is cached, then save a pointer to it in
	     * the local proc structure.
	     */
		if (!ZoneNm) {
		    if (!(ZoneNm = (znhash_t **)calloc(HASHZONE,
					        sizeof(znhash_t *))))
		    {
			(void) fprintf(stderr,
			    "%s: no space for zone name hash\n", Pn);
			Exit(1);
		    }
		}
		for (zp = ZoneNm[zh]; zp; zp = zp->next) {
		    if (!strcmp(zn, zp->zn))
			break;
		}
		if (!zp) {

		/*
		 * The zone name isn't cached, so cache it.
		 */
		    if (!(zp = (znhash_t *)malloc((MALLOC_S)sizeof(znhash_t))))
		    {
			(void) fprintf(stderr,
			    "%s: no zone name cache space: %s\n", Pn, zn);
			Exit(1);
		    }
		    if (!(zp->zn = mkstrcpy(zn, (MALLOC_S *)NULL))) {
			(void) fprintf(stderr,
			    "%s: no zone name space at PID %d: %s\n",
			    Pn, (int)Lp->pid, zn);
			Exit(1);
		    }
		    zp->next = ZoneNm[zh];
		    ZoneNm[zh] = zp;
		}
		Lp->zn = zp->zn;
	    }
#endif	/* defined(HASZONES) */

	/*
	 * Save file count.
	 */
	    Unof = u->u_nofiles;
	/*
	 * Save current working directory information.
	 */
	    if (!ckscko && u->u_cdir) {
		alloc_lfile(CWD, -1);

#if	defined(FILEPTR)
		FILEPTR = (struct file *)NULL;
#endif	/* defined(FILEPTR) */

		process_node((KA_T)u->u_cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (!ckscko && u->u_rdir) {
		alloc_lfile(RTD, -1);

#if	defined(FILEPTR)
		FILEPTR = (struct file *)NULL;
#endif	/* defined(FILEPTR) */

		process_node((KA_T)u->u_rdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save information on text files.
	 */
	    if (!ckscko && p->p_as && Sgvops) {

#if	defined(FILEPTR)
		FILEPTR = (struct file *)NULL;
#endif	/* defined(FILEPTR) */

		process_text((KA_T)p->p_as);
	    }
	/*
	 * Save information on file descriptors.
	 *
	 * Under Solaris the file pointers are stored in dynamically-linked
	 * ufchunk structures, each containing NFPREAD file pointers.  The
	 * first ufchunk structure is in the user area.
	 *
	 * Under Solaris 2.4 the file pointers are in a dynamically allocated,
	 * contiguous memory block.
	 */

#if	solaris<20400
	    for (i = 0, j = 0; i < u->u_nofiles; i++) {
		if (++j > NFPCHUNK) {
		    if (!u->u_flist.uf_next)
			break;
		    if (kread((KA_T)u->u_flist.uf_next,
			(char *)&u->u_flist, sizeof(struct ufchunk)))
			    break;
		    j = 1;
		}
		if (!u->u_flist.uf_ofile[j-1])
#else	/* solaris>=20400 */
	    for (i = 0, j = NFPREAD; i < u->u_nofiles; i++) {
		if (++j > NFPREAD) {
		    k = u->u_nofiles - i;
		    if (k > NFPREAD)
			k = NFPREAD;
		    if (kread((KA_T)((unsigned long)u->u_flist +
				     i * sizeof(uf_entry_t)),
				     (char*)&uf, k * sizeof(uf_entry_t)))
		    {
			break;
		    }
		    j = 1;
		}
		if (!uf[j-1].uf_ofile)
#endif	/* solaris<20400 */

		    continue;
		alloc_lfile((char *)NULL, i);

#if	solaris<20400
		pofv = (long)u->u_flist.uf_pofile[j-1];
		process_file((KA_T)u->u_flist.uf_ofile[j-1]);
#else	/* solaris>=20400 */
		pofv = uf[j-1].uf_pofile;
		process_file((KA_T)uf[j-1].uf_ofile);
#endif	/* solaris <20400 */

		if (Lf->sf) {

#if	defined(HASFSTRUCT)
		    if (Fsv & FSV_FG)
			Lf->pof = pofv;
#endif	/* defined(HASFSTRUCT) */

		    link_lfile();
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
	int i;
	struct stat sb;
	KA_T v;

#if	defined(HAS_AFS)
	struct nlist *nl = (struct nlist *)NULL;
#endif	/* defined(HAS_AFS) */

/*
 * Check the Solaris or SunOS version number; check the SunOS architecture.
 */
	(void) ckkv("Solaris", LSOF_VSTR, (char *)NULL, (char *)NULL);

#if	solaris>=70000
/*
 * Compare the Solaris 7 and above lsof compilation bit size with the kernel
 * bit size.
 *
 * Quit on a mismatch.
 */
	{
	    char *cp, isa[1024];
	    short kbits = 32;

# if	defined(_LP64)
	    short xkbits = 64;
# else	/* !defined(_LP64) */
	    short xkbits = 32;
# endif	/* defined(_LP64) */

	    if (sysinfo(SI_ISALIST, isa, (long)sizeof(isa)) < 0) {
		(void) fprintf(stderr, "%s: can't get ISA list: %s\n",
		    Pn, strerror(errno));
		Exit(1);
	    }
	    for (cp = isa; *cp;) {
		if (strncmp(cp, ARCH64B, strlen(ARCH64B)) == 0) {
		    kbits = 64;
		    break;
		}
		if (!(cp = strchr(cp, ' ')))
		    break;
		cp++;
	    }
	    if (kbits != xkbits) {
		(void) fprintf(stderr,
		    "%s: FATAL: lsof was compiled for a %d bit kernel,\n",
		    Pn, (int)xkbits);
		(void) fprintf(stderr,
		    "      but this machine has booted a %d bit kernel.\n",
		    (int)kbits);
		Exit(1);
	    }
	}
#endif	/* solaris>=70000 */

/*
 * Get kernel symbols.
 */
	if (Nmlst && !is_readable(Nmlst, 1))
	    Exit(1);
	(void) build_Nl(Drive_Nl);

#if	defined(HAS_AFS)
	if (!Nmlst) {

	/*
	 * If AFS is defined and we're getting kernel symbol values from
	 * from N_UNIX, make a copy of Nl[] for possible use with the AFS
	 * modload file.
	 */
	    if (!(nl = (struct nlist *)malloc(Nll))) {
		(void) fprintf(stderr, "%s: no space (%d) for Nl[] copy\n",
		    Pn, Nll);
		Exit(1);
	    }
	    (void) memcpy((void *)nl, (void *)Nl, (size_t)Nll);
	}
#endif	/* defined(HAS_AFS) */

	if (nlist(Nmlst ? Nmlst : N_UNIX, Nl) < 0) {
	    (void) fprintf(stderr, "%s: can't read namelist from %s\n",
		Pn, Nmlst ? Nmlst : N_UNIX);
	    Exit(1);
	}

#if	defined(HAS_AFS)
	if (nl) {

	/*
	 * If AFS is defined and we're getting kernel symbol values from
	 * N_UNIX, and if any X_AFS_* symbols isn't there, see if it is in the
	 * the AFS modload file.  Make sure that other symbols that appear in
	 * both name list files have the same values.
	 */
	    if ((get_Nl_value("arFID", Drive_Nl, &v) >= 0 && !v)
	    ||  (get_Nl_value("avops", Drive_Nl, &v) >= 0 && !v)
	    ||  (get_Nl_value("avol",  Drive_Nl, &v) >= 0 && !v))
		(void) ckAFSsym(nl);
	    (void) free((MALLOC_P *)nl);
	}
#endif	/* defined(HAS_AFS) */

/*
 * Determine the availability of the ALLKMEM device.  If it is available, the
 * active processes will be gathered directly from the active process chain.
 *
 * If ALLKMEM isn't available, the active processes will be gathered via the
 * kvm_*proc() functions.
 */
	if (statsafely(ALLKMEM, &sb) == 0)
	    HasALLKMEM = 1;

#if	defined(HASVXFSUTIL)
/*
 * If the VXFS utility library is being used, attempt to get the VXFS inode
 * offsets before setgid permission is surrendered.
 */
	if (access_vxfs_ioffsets() && !Fwarn) {

	/*
	 * Warn that the VxFS offsets are unavailable.
	 */
	    (void) fprintf(stderr,
		"%s: WARNING: vxfsu_get_ioffsets() returned an error.\n", Pn);
	    (void) fprintf(stderr,
		"%s: WARNING: Thus, no vx_inode information is available\n",
		Pn);
	    (void) fprintf(stderr,
		"%s: WARNING: for display or selection of VxFS files.\n", Pn);
	}
#endif	/* defined(HASVXFSUTIL) */

#if	defined(WILLDROPGID)
/*
 * If Solaris kernel memory is coming from KMEM, the process is willing to
 * surrender GID permission, and the ALLKMEM device is not available, set up
 * for GID switching after the first call to open_kvm().
 */
	if (!Memory && !HasALLKMEM) {
	    Savedgid = getegid();
	    if (Setgid)
		Switchgid = 1;
	}
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
 * Open access to kernel memory.
 */
	open_kvm();

#if	solaris>=20500
/*
 * Get the kernel's KERNELBASE value for Solaris 2.5 and above.
 */
	v = (KA_T)0;
	if (get_Nl_value("kbase", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&Kb, sizeof(Kb))) {
	    (void) fprintf(stderr,
		"%s: can't read kernel base address from %s\n",
		Pn, print_kptr(v, (char *)NULL, 0));
	    Exit(1);
	}
#endif	/* solaris>=20500 */

/*
 * Get the Solaris clone major device number, if possible.
 */
	v = (KA_T)0;
	if ((get_Nl_value("clmaj", Drive_Nl, &v) < 0) || !v) {
	   if (get_Nl_value("clmaj_alt", Drive_Nl, &v) < 0)
		v = (KA_T)0;
	}
	if (v && kread((KA_T)v, (char *)&CloneMaj, sizeof(CloneMaj)) == 0)
	    HaveCloneMaj = 1;
/*
 * If the ALLKMEM device is available, check for the address of the kernel's
 * active process chain.  If it's not available, clear the ALLKMEM status.
 */
	if (HasALLKMEM) {
	   if ((get_Nl_value("pract", Drive_Nl, &PrAct) < 0) || !PrAct)
		HasALLKMEM = 0;
	}

#if	solaris>=20501
/*
 * If the ALLKMEM device isn't available, get the kernel's virtual to physical
 * map structure for Solaris 2.5.1 and above.
 */
	if (!HasALLKMEM) {
	    if (get_Nl_value("kasp", Drive_Nl, &v) >= 0 && v) {
		PageSz = getpagesize();
		PSMask = PageSz - 1;
		for (i = 1, PSShft = 0; i < PageSz; i <<= 1, PSShft++)
		    ;
		(void) readkam(v);
	    }
	}
#endif	/* solaris>=20501 */

#if	defined(WILLDROPGID)
/*
 * If the ALLKMEM device is available -- i.e., we're not using the kvm_*proc()
 * functions to read proc structures -- and if we're willing to drop setgid
 * permission, do so.
 */
	if (HasALLKMEM)
	    (void) dropgid();
#endif	/* defined(WILLDROPGID) */

}


#if	defined(HASZONES)
/*
 * enter_zone_arg() - enter zone name argument
 */

int
enter_zone_arg(zn)
	char *zn;				/* zone name */
{
	int zh;
	znhash_t *zp, *zpn;
/*
 * Allocate zone argument hash space, as required.
 */
	if (!ZoneArg) {
	    if (!(ZoneArg = (znhash_t **)calloc(HASHZONE, sizeof(znhash_t *))))
	    {
		(void) fprintf(stderr, "%s: no space for zone arg hash\n", Pn);
		Exit(1);
	    }
	}
/*
 * Hash the zone name and search the argument hash.
 */
	zh = hash_zn(zn);
	for (zp = ZoneArg[zh]; zp; zp = zp->next) {
	    if (!strcmp(zp->zn, zn))
		break;
	}
	if (zp)	{

	/*
	 * Process a duplicate.
	 */
	    if (!Fwarn)
		(void) fprintf(stderr, "%s: duplicate zone name: %s\n", Pn, zn);
	    return(1);
	}
/*
 * Create a new hash entry and link it to its bucket.
 */
	if (!(zpn = (znhash_t *)malloc((MALLOC_S)sizeof(znhash_t)))) {
	    (void) fprintf(stderr, "%s no hash space for zone: %s\n", Pn, zn);
	    Exit(1);
	}
	zpn->f = 0;
	zpn->zn = zn;
	zpn->next = ZoneArg[zh];
	ZoneArg[zh] = zpn;
	return(0);
}


/*
 * hash_zn() - hash zone name
 */

static int
hash_zn(zn)
	char *zn;				/* zone name */
{
	register int i, h;
	size_t l;

	if (!(l = strlen(zn)))
	    return(0);
	if (l == 1)
	    return((int)*zn & (HASHZONE - 1));
	for (i = h = 0; i < (int)(l - 1); i++) {
	    h ^= ((int)zn[i] * (int)zn[i+1]) << ((i*3)%13);
	}
	return(h & (HASHZONE - 1));
}
#endif	/* defined(HASZONES) */


/*
 * initialize() - perform all initialization
 */

void
initialize()
{
	get_kernel_access();
/*
 * Read Solaris file system information and construct the clone table.
 *
 * The clone table is needed to identify sockets.
 */
	readfsinfo();

#if	defined(HASDCACHE)
	readdev(0);
#else	/* !defined(HASDCACHE) */
	read_clone();
#endif	/*defined(HASDCACHE) */

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
	register int br;
/*
 * Because lsof reads kernel data and follows pointers found there at a
 * rate considerably slower than the kernel, lsof sometimes acquires
 * invalid pointers.  If the invalid pointers are fed to kvm_[k]read(),
 * a segmentation violation may result, so legal kernel addresses are
 * limited by the value of the KERNELBASE symbol (Kb value from the
 * kernel's _kernelbase variable for Solaris 2.5 and above).
 */

#if	solaris>=20500
#define	KVMREAD	kvm_kread
	if (addr < Kb)
#else	/* solaris<20500 */
#define	KVMREAD kvm_read
	if (addr < (KA_T)KERNELBASE)
#endif	/* solaris>=20500 */

	    return(1);

#if	solaris>=20501

/*
 * Do extra address checking for Solaris above 2.5 when the ALLKMEM device
 * isn't available.
 *
 * Make sure the virtual address represents real physical memory by testing
 * it with kvm_physaddr().
 *
 * For Solaris below 7 read the kernel data with llseek() and read().  For
 * Solaris 7 and above use kvm_pread().
 */
	if (Kas && !HasALLKMEM) {

# if	solaris>20501
	    register int b2r;
	    register char *bp;
# endif	/* solaris>20501 */

	    register int h, ip, tb;
	    register kvmhash_t *kp;
	    KPHYS pa;
	    register KVIRT va, vpa;

# if	solaris<20600
	    for (tb = 0, va = (KVIRT)addr;
		 tb < len;
		 tb += br, va += (KVIRT)br)
# else	/* solaris>=20600 */
	    for (bp = buf, tb = 0, va = (KVIRT)addr;
		 tb < len;
		 bp += br, tb += br, va += (KVIRT)br)
# endif	/* solaris<20600 */

	    {
		vpa = (va & (KVIRT)~PSMask) >> PSShft;
		ip = (int)(va & (KVIRT)PSMask);
		h = HASHKVM(vpa);
		for (kp = KVMhb[h]; kp; kp = kp->nxt) {
		    if (kp->vpa == vpa) {
			pa = kp->pa;
			break;
		    }
		}
		if (!kp) {
		    if ((pa = kvm_physaddr(Kd, Kas, va)) == KAERR)
			return(1);
		    if (!(kp = (kvmhash_t *)malloc(sizeof(kvmhash_t)))) {
			(void) fprintf(stderr, "%s: no kvmhash_t space\n", Pn);
			Exit(1);
		    }
		    kp->nxt = KVMhb[h];
		    pa = kp->pa = (pa & ~(KPHYS)PSMask);
		    kp->vpa = vpa;
		    KVMhb[h] = kp;
		}

# if	solaris<20600
		br = (int)(len - tb);
		if ((ip + br) > PageSz)
		    br = PageSz - ip;
# else	/* solaris>=20600 */
		b2r = (int)(len - tb);
		if ((ip + b2r) > PageSz)
		    b2r = PageSz - ip;
		pa |= (KPHYS)ip;

#  if	solaris<70000
		if (llseek(Kmd, (offset_t)pa, SEEK_SET) == (offset_t)-1)
		    return(1);
		if ((br = (int)read(Kmd, (void *)bp, (size_t)b2r)) <= 0)
		    return(1);
#  else	/* solaris>=70000 */
		if ((br = kvm_pread(Kd, pa, (void *)bp, (size_t)b2r)) <= 0)
		    return(1);
#  endif	/* solaris<70000 */
# endif	/* solaris<20600 */

	    }

# if	solaris>=20600
	    return(0);
# endif	/* solaris>=20600 */

	}
#endif	/* solaris>=20501 */

/*
 * Use kvm_read for Solaris < 2.5; use kvm_kread() Solaris >= 2.5.
 */
	br = KVMREAD(Kd, (u_long)addr, buf, len);
	return(((READLEN_T)br == len) ? 0 : 1);
}


/*
 * open_kvm() - open kernel virtual memory access
 */

void
open_kvm()
{
	if (Kd)
	    return;

#if	defined(WILLDROPGID)
/*
 * If this Solaris process began with setgid permission and its been
 * surrendered, regain it.
 */
	(void) restoregid();
#endif	/* defined(WILLDROPGID) */

	if (!(Kd = kvm_open(Nmlst, Memory, NULL, O_RDONLY, Pn))) {
	    (void) fprintf(stderr,
		"%s: kvm_open(namelist=%s, corefile=%s): %s\n",
		Pn,
		Nmlst ? Nmlst : "default",
		Memory  ? Memory  : "default",
		strerror(errno));
	    Exit(1);
	}

#if	solaris>=20501 && solaris<70000
	if ((Kmd = open((Memory ? Memory : KMEM), O_RDONLY)) < 0) {
	    (void) fprintf(stderr, "%s: open(\"/dev/mem\"): %s\n", Pn, 
		strerror(errno));
	    Exit(1);
	}
#endif	/* solaris>=20501 && solaris<70000 */

#if	defined(WILLDROPGID)
/*
 * If this process has setgid permission, and is willing to surrender it,
 * do so.
 */
	(void) dropgid();
/*
 * If this Solaris process must switch GIDs, enable switching after the
 * first call to this function.
 */
	if (Switchgid == 1)
	    Switchgid = 2;
#endif	/* define(WILLDROPGID) */

}


/*
 * process_text() - process text access information
 */

#if	solaris>=90000
#include <sys/avl.h>

/*
 * Avl trees are implemented as follows: types in AVL trees contain an
 * avl_node_t.  These avl_nodes connect to other avl nodes embedded in
 * objects of the same type.  The avl_tree contains knowledge about the
 * size of the structure and the offset of the AVL node in the object
 * so we can convert between AVL nodes and (in this case) struct seg. 
 *
 * This code was provided by Casper Dik <Casper.Dik@holland.sun.com>.
 */

#define READ_AVL_NODE(n,o,s) \
	if (kread((KA_T)AVL_NODE2DATA(n, o), (char*) s, sizeof(*s))) \
		return -1

static int
get_first_seg(avl_tree_t *av, struct seg *s)
{
	avl_node_t *node = av->avl_root;
	size_t off = av->avl_offset;
	int count = 0;

	while (node != NULL && ++count < MAXSEGS * 2) {
	    READ_AVL_NODE(node, off, s);
	    node = s->s_tree.avl_child[0];
	    if (node == NULL)
		return 0;
	}
	return -1;
}

static int
get_next_seg(avl_tree_t *av, struct seg *s)
{
	avl_node_t *node = &s->s_tree;
	size_t off = av->avl_offset;
	int count = 0;

	if (node->avl_child[1]) {
	    /*
	     * Has right child, go all the way to the leftmost child of
	     * the right child.
	     */
	    READ_AVL_NODE(node->avl_child[1], off, s);
	    while (node->avl_child[0] != NULL && ++count < 2 * MAXSEGS)
		 READ_AVL_NODE(node->avl_child[0],off,s);
	    if (count < 2 * MAXSEGS)
		return 0;
	} else {
	    /*
	     * No right child, go up until we find a node we're not a right
	     * child of.
	     */
	    for (;count < 2 * MAXSEGS; count++) {
		int index = AVL_XCHILD(node);
		avl_node_t *parent = AVL_XPARENT(node);

		if (parent == NULL)
		    return -1;

		READ_AVL_NODE(parent, off, s);

		if (index == 0)
		    return 0;
	    }
	}
	return -1;
}

static void
process_text(pa)
	KA_T pa;			/* address space description pointer */
{
	struct as as;
	int i, j, k;
	struct seg s;
	struct segvn_data vn;
	avl_tree_t *avtp;
	KA_T v[MAXSEGS];
/*
 * Get address space description.
 */
	if (kread((KA_T)pa, (char *)&as, sizeof(as))) {
	    alloc_lfile(" txt", -1);
	    (void) snpf(Namech, Namechl, "can't read text segment list (%s)",
		print_kptr(pa, (char *)NULL, 0));
	    enter_nm(Namech);
	    if (Lf->sf)
		link_lfile();
	    return;
	}
/*
 * Loop through the segments.  The loop should stop when the segment
 * pointer returns to its starting point, but just in case, it's stopped
 * when MAXSEGS unique segments have been recorded or 2*MAXSEGS segments
 * have been examined.
 */
	for (avtp = &as.a_segtree, i = j = 0;
	     (i < MAXSEGS) && (j < 2*MAXSEGS);
	     j++)
	{
	    if (j ? get_next_seg(avtp, &s) : get_first_seg(avtp, &s))
		break;
	    if ((KA_T)s.s_ops == Sgvops && s.s_data) {
		if (kread((KA_T)s.s_data, (char *)&vn, sizeof(vn)))
		    break;
		if (vn.vp) {
			
		/*
		 * This is a virtual node segment.
		 *
		 * If its vnode pointer has not been seen already, record the
		 * vnode pointer and process the vnode.
		 */
		    for (k = 0; k < i; k++) {
			if (v[k] == (KA_T)vn.vp)
			    break;
		    }
		    if (k >= i) {
			v[i++] = (KA_T)vn.vp;
			alloc_lfile(" txt", -1);

# if	defined(FILEPTR)
			FILEPTR = (struct file *)NULL;
# endif	/* defined(FILEPTR) */

			process_node((KA_T)vn.vp);
			if (Lf->sf)
			    link_lfile();
		    }
		}
	    }
	}
}

#else	/* solaris<90000 */

# if	solaris>=20400
#define S_NEXT s_next.list
# else	/* solaris<20400 */
#define S_NEXT s_next
# endif	/* solaris>=20400 */

static void
process_text(pa)
	KA_T pa;			/* address space description pointer */
{
	struct as as;
	int i, j, k;
	struct seg s;
	struct segvn_data vn;
	KA_T v[MAXSEGS];
/*
 * Get address space description.
 */
	if (kread((KA_T)pa, (char *)&as, sizeof(as))) {
	    alloc_lfile(" txt", -1);
	    (void) snpf(Namech, Namechl, "can't read text segment list (%s)",
		print_kptr(pa, (char *)NULL, 0));
	    enter_nm(Namech);
	    if (Lf->sf)
		link_lfile();
	    return;
	}
/*
 * Loop through the segments.  The loop should stop when the segment
 * pointer returns to its starting point, but just in case, it's stopped
 * when MAXSEGS unique segments have been recorded or 2*MAXSEGS segments
 * have been examined.
 */
	for (s.s_next = as.a_segs, i = j = 0;
	     i < MAXSEGS && j < 2*MAXSEGS;
	     j++)
	{
	    if (!s.S_NEXT
	    ||  kread((KA_T)s.S_NEXT, (char *)&s, sizeof(s)))
		break;
	    if ((KA_T)s.s_ops == Sgvops && s.s_data) {
		if (kread((KA_T)s.s_data, (char *)&vn, sizeof(vn)))
		    break;
		if (vn.vp) {
			
		/*
		 * This is a virtual node segment.
		 *
		 * If its vnode pointer has not been seen already, record the
		 * vnode pointer and process the vnode.
		 */
		    for (k = 0; k < i; k++) {
			if (v[k] == (KA_T)vn.vp)
			    break;
		    }
		    if (k >= i) {
			v[i++] = (KA_T)vn.vp;
			alloc_lfile(" txt", -1);

# if	defined(FILEPTR)
			FILEPTR = (struct file *)NULL;
# endif	/* defined(FILEPTR) */

			process_node((KA_T)vn.vp);
			if (Lf->sf)
			    link_lfile();
		    }
		}
	    }
	/*
	 * Follow the segment link to the starting point in the address
	 * space description.  (The i and j counters place an absolute
	 * limit on the loop.)
	 */

# if	solaris<20400
	    if (s.s_next == as.a_segs)
# else	/* solaris>=20400 */
	    if (s.s_next.list == as.a_segs.list)
# endif	/* solaris<20400 */

		break;
	}
}
#endif  /* solaris>=90000 */


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
	    if (buf[0] == '\0') {
		Fsinfo[i-1] = "";
		continue;
	    }
	    buf[FSTYPSZ] = '\0';
	    len = strlen(buf) + 1;
	    if (!(Fsinfo[i-1] = (char *)malloc((MALLOC_S)len))) {
		(void) fprintf(stderr,
		    "%s: no space for file system entry %s\n", Pn, buf);
		Exit(1);
	    }
	    (void) snpf(Fsinfo[i-1], len, "%s", buf);

# if	defined(HAS_AFS)
	    if (strcasecmp(buf, "afs") == 0)
		AFSfstype = i;
# endif	/* defined(HAS_AFS) */

	}
}


#if	solaris>=20501
/*
 * readkam() - read kernel's address map structure
 */

static void
readkam(addr)
	KA_T addr;			/* kernel virtual address */
{
	register int i;
	register kvmhash_t *kp, *kpp;
	static KA_T kas = (KA_T)NULL;

	if (addr)
	    kas = addr;
	Kas = (struct as *)NULL;

#if	solaris<70000
	if (kas && !kread(kas, (char *)&Kam, sizeof(Kam)))
	    Kas = (KA_T)&Kam;
#else	/* solaris>=70000 */
	Kas = (struct as *)kas;
#endif	/* solaris<70000 */

	if (Kas) {
	    if (!KVMhb) {
		if (!(KVMhb = (kvmhash_t **)calloc(KVMHASHBN,
						   sizeof(kvmhash_t *))))
		{
		     (void) fprintf(stderr,
			"%s: no space (%d) for KVM hash buckets\n",
			Pn, (int)(KVMHASHBN * sizeof(kvmhash_t *)));
		    Exit(1);
		}
	    } else if (!addr) {
		for (i = 0; i < KVMHASHBN; i++) {
		    if ((kp = KVMhb[i])) {
			while (kp) {
			    kpp = kp->nxt;
			    (void) free((void *)kp);
			    kp = kpp;
			}
			KVMhb[i] = (kvmhash_t *)NULL;
		    }
		}
	    }
	}
}
#endif	/* solaris>=20501 */


/*
 * read_proc() - read proc structures
 *
 * As a side-effect, Kd is set by a call to kvm_open().
 */

static void
read_proc()
{
	int ct, ctl, knp, n, try;
	MALLOC_S len;
	struct proc *p;
	KA_T pa, paf, pan;
	struct pid pg, pids;
/*
 * Try PROCTRYLM times to read a valid proc table.
 */
	for (try = 0; try < PROCTRYLM; try++) {

	/*
	 * Get a proc structure count estimate.
	 */
	    if (get_Nl_value("nproc", Drive_Nl, &pa) < 0 || !pa
	    ||  kread(pa, (char *)&knp, sizeof(knp))
	    ||  knp < 1)
		knp = PROCDFLT;
	/*
	 * Pre-allocate space, as required.
	 */
	    n = knp + PROCDFLT/4;
	    if (n > Npa) {

	    /*
	     * Allocate proc structure space.
	     */
		len = (n * sizeof(struct proc));
		if (P)
		    P = (struct proc *)realloc((MALLOC_P *)P, len);
		else
		    P = (struct proc *)malloc(len);
		if (!P) {
		    (void) fprintf(stderr, "%s: no proc table space\n", Pn);
		    Exit(1);
		}
	    /*
	     * Pre-allocate PGID and PID number space.
	     */
		len = (MALLOC_S)(n * sizeof(int));
		if (Fpgid) {
		    if (Pgid)
			Pgid = (int *)realloc((MALLOC_P *)Pgid, len);
		    else
			Pgid = (int *)malloc(len);
		    if (!Pgid) {
			(void) fprintf(stderr, "%s: no PGID table space\n", Pn);
			Exit(1);
		    }
		}
		if (Pid)
		    Pid = (int *)realloc((MALLOC_P *)Pid, len);
		else
		    Pid = (int *)malloc(len);
		if (!Pid) {
		    (void) fprintf(stderr, "%s: no PID table space\n", Pn);
		    Exit(1);
		}
		Npa = n;
	    }
	    if (HasALLKMEM) {

	    /*
	     * Prepare for a proc table scan via direct reading of the active
	     * chain.
	     */
		if (!PrAct || kread(PrAct, (char *)&paf, sizeof(pa))) {
		    (void) fprintf(stderr, "%s: can't read practive from %s\n",
			Pn, print_kptr(PrAct, (char *)NULL, 0));
		    Exit(1);
		}
		ct = 1;
		ctl = knp << 3;
		pan = paf;
		pa = (KA_T)NULL;
	    } else {

	    /*
	     * Prepare for a proc table scan via the kvm_*proc() functions.
	     */
		if (kvm_setproc(Kd) != 0) {
		    (void) fprintf(stderr, "%s: kvm_setproc: %s\n", Pn,
			strerror(errno));
		    Exit(1);
		}
	    }
	/*
	 * Accumulate proc structures.
	 */
	    Np = 0;
	    for (;;) {
		if (Np >= Npa) {

		/*
		 * Expand the local proc table.
		 */
		    Npa += PROCDFLT/2;
		    len = (MALLOC_S)(Npa * sizeof(struct proc));
		    if (!(P = (struct proc *)realloc((MALLOC_P *)P, len))) {
			(void) fprintf(stderr,
			    "%s: no more (%d) proc space\n", Pn, Npa);
			Exit(1);
		    }
		/*
		 * Expand the PGID and PID tables.
		 */
		    len = (MALLOC_S)(Npa * sizeof(int));
		    if (Fpgid) {
			if (!(Pgid = (int *)realloc((MALLOC_P *)Pgid, len))) {
			    (void) fprintf(stderr,
				"%s: no more (%d) PGID space\n", Pn, Npa);
			    Exit(1);
			}
		    }
		    if (!(Pid = (int *)realloc((MALLOC_P *)Pid, len))) {
			(void) fprintf(stderr,
			    "%s: no more (%d) PID space\n", Pn, Npa);
			Exit(1);
		    }
		}
	    /*
	     * Read the next proc structure.
	     */
		if (HasALLKMEM) {

		/*
		 * If the ALLKMEM device exists, read proc structures directly
		 * from the active chain.
		 */
		    if (!pa)
			pa = paf;
		    else {
			pa = pan;
			if ((pan == paf) || (++ct > ctl))
			   break;
		    }
		    if (!pa)
			break;
		    p = (struct proc *)&P[Np];
		    if (kread(pa, (char *)p, sizeof(struct proc)))
			break;
		    pan = (KA_T)p->p_next;
		} else {

		/*
		 * If the ALLKMEM device doesn't exist, read proc structures
		 * via kbm_getproc().
		 */
		    if (!(p = kvm_nextproc(Kd)))
			break;
		}
	    /*
	     * Check process status.
	     */
		if (p->p_stat == 0 || p->p_stat == SZOMB)
		    continue;

#if	solaris >=20500
		/*
		 * Check Solaris 2.5 and above p_cred pointer.
		 */
	    	    if (!p->p_cred)
			continue;
#endif	/* solaris >=20500 */

		/*
		 * Read Solaris PGID and PID numbers.
		 */
		if (Fpgid) {
		    if (!p->p_pgidp
		    ||  kread((KA_T)p->p_pgidp, (char *)&pg, sizeof(pg)))
			continue;
		}
		if (!p->p_pidp
		||  kread((KA_T)p->p_pidp, (char *)&pids, sizeof(pids)))
		    continue;
	    /*
	     * Save the PGID and PID numbers in local tables.
	     */
		if (Fpgid)
		    Pgid[Np] = (int)pg.pid_id;
		Pid[Np] = (int)pids.pid_id;
	    /*
	     * If the proc structure came from kvm_getproc(), save it in the
	     * local table.
	     */
		if (!HasALLKMEM)
		    P[Np] = *p;
		Np++;
	    }
	/*
	 * If not enough processes were saved in the local table, try again.
	 *
	 * If the ALLKMEM device isn't available, it is necessary to close and
	 * reopen the KVM device, so that kvm_open() will acquire a fresh
	 * address for the head of the linked list process table.
	 */
	    if (Np >= PROCMIN)
		break;
	    if (!HasALLKMEM) {
		close_kvm();
		open_kvm();
	    }
	}
/*
 * Quit if no proc structures were stored in the local table.
 */
	if (try >= PROCTRYLM) {
	    (void) fprintf(stderr, "%s: can't read proc table\n", Pn);
	    Exit(1);
	}
	if (Np < Npa && !RptTm) {

	/*
	 * Reduce the local proc structure table size to its minimum if
	 * not in repeat mode.
	 */
	    len = (MALLOC_S)(Np * sizeof(struct proc));
	    if (!(P = (struct proc *)realloc((MALLOC_P *)P, len))) {
		(void) fprintf(stderr, "%s: can't reduce proc table to %d\n",
		    Pn, Np);
		Exit(1);
	    }
	/*
	 * Reduce the Solaris PGID and PID tables to their minimum if
	 * not in repeat mode.
	 */
	    len = (MALLOC_S)(Np * sizeof(int));
	    if (Fpgid) {
		if (!(Pgid = (int *)realloc((MALLOC_P *)Pgid, len))) {
		    (void) fprintf(stderr,
			"%s: can't reduce PGID table to %d\n", Pn, Np);
		    Exit(1);
		}
	    }
	    if (!(Pid = (int *)realloc((MALLOC_P *)Pid, len))) {
		(void) fprintf(stderr,
		    "%s: can't reduce PID table to %d\n", Pn, Np);
		Exit(1);
	    }
	    Npa = Np;
	}
}


#if	defined(WILLDROPGID)
/*
 * restoregid() -- restore setgid permission, as required
 */

void
restoregid()
{
	if (Switchgid == 2 && !Setgid) {
	    if (setgid(Savedgid) != 0) {
		(void) fprintf(stderr,
		    "%s: can't set effective GID to %d: %s\n",
		    Pn, (int)Savedgid, strerror(errno));
		Exit(1);
	    }
	    Setgid = 1;
	}
}
#endif	/* defined(WILLDROPGID) */


#if	defined(HASNCACHE) && solaris>=90000


/*
 * Local static values
 */

static int Mhl;				/* local name cache hash mask */
static int Nhl = 0;			/* size of local name cache hash
					 * pointer table */
struct l_nch {
	KA_T vp;			/* vnode address */
	KA_T dp;			/* parent vnode address */
	struct l_nch *pa;		/* parent Ncache address */
	char *nm;			/* name */
	int nl;				/* name length */
};

static struct l_nch *Ncache = (struct l_nch *)NULL;
					/* the local name cache */
static struct l_nch **Nchash = (struct l_nch **)NULL;
					/* Ncache hash pointers */
static int Ncfirst = 1;			/* first-call status */
static KA_T NegVN = (KA_T)NULL;		/* negative vnode address */
static int Nla = 0;			/* entries allocated to Ncache[] */
static int Nlu = 0;			/* entries used in Ncache[] */

_PROTOTYPE(static struct l_nch *ncache_addr,(KA_T v));

#define ncachehash(v)		Nchash+((((int)(v)>>2)*31415)&Mhl)

_PROTOTYPE(static int ncache_isroot,(KA_T va, char *cp));

#define	LNCHINCRSZ	64	/* local size increment */
#define	XNC		15	/* extra name characters to read beyond those
				 * in name[] of the ncache_t structure -- this
				 * is an efficiency hint and MUST BE AT LEAST
				 * ONE. */


/*
 * ncache_addr() - look up a node's local ncache address
 */

static struct l_nch *

ncache_addr(v)
	KA_T v;					/* vnode's address */
{
	struct l_nch **hp;

	for (hp = ncachehash(v); *hp; hp++) {
	    if ((*hp)->vp == v)
		return(*hp);
	}
	return((struct l_nch *)NULL);
}


/*
 * ncache_isroot() - is head of name cache path a file system root?
 */

static int
ncache_isroot(va, cp)
	KA_T va;			/* kernel vnode address */
	char *cp;			/* partial path */
{
	char buf[MAXPATHLEN];
	int i;
	MALLOC_S len;
	struct mounts *mtp;
	struct stat sb;
	struct vnode v;
	static int vca = 0;
	static int vcn = 0;
	static KA_T *vc = (KA_T *)NULL;

	if (!va)
	    return(0);
/*
 * Search the root vnode cache.
 */
	for (i = 0; i < vcn; i++) {
	    if (va == vc[i])
		return(1);
	}
/*
 * Read the vnode and see if it's a VDIR node with the VROOT flag set.  If
 * it is, then the path is complete.
 *
 * If it isn't, and if the file has an inode number, search the mount table
 * and see if the file system's inode number is known.  If it is, form the
 * possible full path, safely stat() it, and see if it's inode number matches
 * the one we have for this file.  If it does, then the path is complete.
 */
	if (kread((KA_T)va, (char *)&v, sizeof(v))
	||  v.v_type != VDIR || !(v.v_flag & VROOT)) {

	/*
	 * The vnode tests failed.  Try the inode tests.
	 */
	    if (Lf->inp_ty != 1 || !Lf->inode
	    ||  !Lf->fsdir || (len = strlen(Lf->fsdir)) < 1)
		return(0);
	    if ((len + 1 + strlen(cp) + 1) > sizeof(buf))
		return(0);
	    for (mtp = readmnt(); mtp; mtp = mtp->next) {
		if (!mtp->dir || !mtp->inode)
		    continue;
		if (strcmp(Lf->fsdir, mtp->dir) == 0)
		    break;
	    }
	    if (!mtp)
		return(0);
	    (void) strcpy(buf, Lf->fsdir);
	    if (buf[len - 1] != '/')
		buf[len++] = '/';
	    (void) strcpy(&buf[len], cp);
	    if (statsafely(buf, &sb) != 0
	    ||  (INODETYPE)sb.st_ino != Lf->inode)
		return(0);
	}
/*
 * Add the vnode address to the root vnode cache.
 */
	if (vcn >= vca) {
	    vca += 10;
	    len = (MALLOC_S)(vca * sizeof(KA_T));
	    if (!vc)
		vc = (KA_T *)malloc(len);
	    else
		vc = (KA_T *)realloc(vc, len);
	    if (!vc) {
		(void) fprintf(stderr, "%s: no space for root vnode table\n",
		    Pn);
		Exit(1);
	    }
	}
	vc[vcn++] = va;
	return(1);
}


/*
 * ncache_load() - load the kernel's name cache
 */

void
ncache_load()
{
	char *cp;
	struct l_nch **hp, *lc;
	int h, i, len, n, xl;
	static int iNch = 0;
	nc_hash_t *kh;
	static KA_T kha = (KA_T)NULL;
	static nc_hash_t *khl = (nc_hash_t *)NULL;
	KA_T kn;
	static ncache_t *nc = (ncache_t *)NULL;
	static int Nch = 0;
	static int nmo = 0;
	KA_T v;
	static int xn = 0;

	if (!Fncache)
	    return;
	if (Ncfirst) {

	/*
	 * Do startup (first-time) functions.
	 */
	    Ncfirst = 0;
	/*
	 * Establish DNLC hash size.
	 */
	    v = (KA_T)0;
	    if (get_Nl_value(X_NCSIZE, (struct drive_Nl *)NULL, &v) < 0
	    ||  !v
	    ||  kread((KA_T)v, (char *)&Nch, sizeof(Nch)))
	    {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: can't read DNLC hash size: %s\n",
			Pn, print_kptr(v, (char *)NULL, 0));
		iNch = Nch = 0;
		return;
	    }
	    if ((iNch = Nch) < 1) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: DNLC hash size: %d\n", Pn, Nch);
		iNch = Nch = 0;
		return;
	    }
	/*
	 * Get negative vnode address.
	 */
	    if (get_Nl_value(NCACHE_NEGVN, (struct drive_Nl *)NULL, &NegVN)
	    < 0)
		NegVN = (KA_T)NULL;
	/*
	 * Establish DNLC hash address.
	 */
	    v = (KA_T)0;
	    if (get_Nl_value(X_NCACHE,(struct drive_Nl *)NULL,(KA_T *)&v) < 0
	    || !v
	    || kread(v, (char *)&kha, sizeof(kha))
	    || !kha
	    ) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: no DNLC hash address\n", Pn);
		iNch = Nch = 0;
		return;
	    }
	/*
	 * Allocate space for a local copy of the kernel's hash table.
	 */
	    len = Nch * sizeof(nc_hash_t);
	    if (!(khl = (nc_hash_t *)malloc((MALLOC_S)len))) {
		(void) fprintf(stderr,
		    "%s: can't allocate DNLC hash space: %d\n", Pn, len);
		Exit(1);
	    }
	/*
	 * Allocate space for a kernel DNLC entry, plus additional name space
	 * for efficiency.
	 */
	    xn = XNC;
	    if (!(nc = (ncache_t *)malloc((MALLOC_S)(sizeof(ncache_t) + XNC))))
	    {
		(void) fprintf(stderr,
		    "%s: can't allocate DNLC ncache_t space\n", Pn);
		Exit(1);
	    }
	    nmo = offsetof(struct ncache, name);
	/*
	 * Allocate estimated space for the local cache, based on the
	 * hash table count and the current average hash length.
	 */
	    v = (KA_T)0;
	    if ((get_Nl_value("hshav", (struct drive_Nl *)NULL, (KA_T *)&v) < 0)
	    || !v
	    || kread(v, (char *)&i, sizeof(i))
	    || (i < 1)
	    ) {
		i = 16;
		if (!Fwarn) {
		    (void) fprintf(stderr,
			"%s: can't read DNLC average hash bucket size,", Pn);
		    (void) fprintf(stderr, " using %d\n", i);
		}
	    }
	    Nla = Nch * i;
	    if (!(Ncache = (struct l_nch *)calloc(Nla, sizeof(struct l_nch)))) {

no_local_space:

		(void) fprintf(stderr,
		    "%s: no space for %d byte local name cache\n", Pn, len);
		Exit(1);
	    }
	} else {

	/*
	 * Do setup for repeat calls.
	 */
	    if (!iNch || !Nla || !Ncache)
		return;
	    if (Nchash) {
		(void) free((FREE_P *)Nchash);
		Nchash = (struct l_nch **)NULL;
	    }
	    if (Ncache && Nlu) {

	    /*
	     * Free space malloc'd to names in local name cache.
	     */
	        for (i = 0, lc = Ncache; i < Nlu; i++, lc++) {
		    if (lc->nm) {
			(void) free((FREE_P *)lc->nm);
			lc->nm = (char *)NULL;
		    }
	        }
	    }
	    Nch = iNch;
	    Nlu = 0;
	}
/*
 * Read the kernel's DNLC hash.
 */
	if (kread(kha, (char *)khl, (Nch * sizeof(nc_hash_t)))) {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: can't read DNLC hash: %s\n",
		    Pn, print_kptr(kha, (char *)NULL, 0));
	    iNch = Nch = 0;
	    return;
	}
/*
 * Build a local copy of the kernel name cache.
 */
	for (i = n = 0, kh = khl, lc = Ncache; i < Nch; i++, kh++) {

	/*
	 * Skip empty hash buckets.
	 */
	    if (!kh->hash_next || ((KA_T)kh->hash_next == kha))
		continue;
	/*
	 * Process a hash bucket.
	 */
	    for (kn = (KA_T)kh->hash_next, h = 0;
		 kn && (h < Nch) && (!h || (h && kn != (KA_T)kh->hash_next));
		 kn = (KA_T)nc->hash_next, h++)
	    {
		if (kread(kn, (char *)nc, sizeof(ncache_t) + XNC))
		    break;
		if (!nc->vp || (len = (int)nc->namlen) < 1)
		    continue;
		if (NegVN && ((KA_T)nc->vp == NegVN))
		    continue;
		if ((len < 3) && (nc->name[0] == '.')) {
		    if ((len < 2) || (nc->name[1] == '.'))
			continue;
		}
	    /*
	     * If not all the name has been read, read the rest of it,
	     * allocating more space at the end of the ncache structure as
	     * required.
	     */
		if (len > (XNC + 1)) {
		    if (len > (xn + 1)) {
			while (len > (xn + 1))
			    xn = xn + xn;
			xn = ((xn + 7) & ~7) - 1;
			if (!(nc = (ncache_t *)realloc((MALLOC_P *)nc,
					(sizeof(ncache_t) + xn)))
			) {
			    (void) fprintf(stderr,
				"%s: can't extend DNLC ncache_t buffer\n", Pn);
			    Exit(1);
			}
		    }
		    cp = &nc->name[XNC + 1];
		    v = (KA_T)((char *)kn + nmo + XNC + 1);
		    xl = len - XNC - 1;
		    if (kread(v, cp, xl))
			continue;
		}
	    /*
	     * Allocate space for the name in the local name cache entry.
	     */
		if (!(cp = (char *)malloc(len + 1))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for name cache name\n",
			Pn, len + 1);
		    Exit(1);
		}
		(void) strncpy(cp, nc->name, len);
		cp[len] = '\0';
	    /*
	     * Make sure there is space for another local name cache entry.
	     * If not, allocate twice as many entries.
	     */
		if (n >= Nla) {
		    Nla = Nla + Nla;
		    if (!(Ncache = (struct l_nch *)realloc(Ncache,
				   (MALLOC_S)(Nla * sizeof(struct l_nch))))
		    ) {
			(void) fprintf(stderr,
			    "%s: can't enlarge local name cache\n", Pn);
			Exit(1);
		    }
		    lc = &Ncache[n];
		}
	    /*
	     * Complete the local cache entry.
	     */
		lc->vp = (KA_T)nc->vp;
		lc->dp = (KA_T)nc->dp;
		lc->pa = (struct l_nch *)NULL;
		lc->nm = cp;
		lc->nl = len;
		lc++;
		n++;
	    }
	}
/*
 * Reduce memory usage, as required.
 */
	if ((Nlu = n) < 1) {

	/*
	 * No DNLC entries were located, an unexpected result.
	 */
	    if (!RptTm && Ncache) {

	    /*
	     * If not in repeat mode, free the space that has been malloc'd
	     * to the local name cache.
	     */
		(void) free((FREE_P *)Ncache);
	 	Ncache = (struct l_nch *)NULL;
		Nla = Nlu = 0;
	    }
	/*
	 * Issue a warning and disable furthe DNLC processing.
	 */
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: unusable local name cache size: %d\n", Pn, n);
	    iNch = Nch = 0;
	    return;
	}
	if ((Nlu < Nla) && !RptTm) {
	    len = Nlu * sizeof(struct l_nch);
	    if (!(Ncache = (struct l_nch *)realloc(Ncache, len)))
		goto no_local_space;
	    Nla = Nlu;
	}
/*
 * Build a hash table to locate Ncache entries.
 */
	for (Nhl = 1; Nhl < Nlu; Nhl <<= 1)
	    ;
	Nhl <<= 1;
	Mhl = Nhl - 1;
	if (!(Nchash = (struct l_nch **)calloc(Nhl + Nlu,
					sizeof(struct l_nch *))))
	{
	    (void) fprintf(stderr,
		"%s: no space for %d name cache hash pointers\n",
		Pn, Nhl + Nlu);
	    Exit(1);
	}
	for (i = 0, lc = Ncache; i < Nlu; i++, lc++) {
	    for (hp = ncachehash(lc->vp), h = 1; *hp; hp++) {
		if ((*hp)->vp == lc->vp && strcmp((*hp)->nm, lc->nm) == 0
		&&  (*hp)->dp == lc->dp
		) {
		    h = 0;
		    break;
		}
	    }
	    if (h)
		*hp = lc;
	}
/*
 * Make a final pass through the local cache and convert parent vnode
 * addresses to local name cache pointers.
 */
	for (i = 0, lc = Ncache; i < Nlu; i++, lc++) {
	    if (!lc->dp)
		continue;
	     if (NegVN && (lc->dp == NegVN)) {
		lc->pa = (struct l_nch *)NULL;
		continue;
	     }
	    lc->pa = ncache_addr(lc->dp);
	}
}


/*
 * ncache_lookup() - look up a node's name in the kernel's name cache
 */

char *
ncache_lookup(buf, blen, fp)
	char *buf;			/* receiving name buffer */
	int blen;			/* receiving buffer length */
	int *fp;			/* full path reply */
{
	char *cp = buf;
	struct l_nch *lc;
	struct mounts *mtp;
	int nl, rlen;

	*cp = '\0';
	*fp = 0;

# if	defined(HASFSINO)
/*
 * If the entry has an inode number that matches the inode number of the
 * file system mount point, return an empty path reply.  That tells the
 * caller to print the file system mount point name only.
 */
	if (Lf->inp_ty == 1 && Lf->fs_ino && Lf->inode == Lf->fs_ino)
	    return(cp);
# endif	/* defined(HASFSINO) */

/*
 * Look up the name cache entry for the node address.
 */
	if (!Nlu || !(lc = ncache_addr(Lf->na))) {

	/*
	 * If the node has no cache entry, see if it's the mount
	 * point of a known file system.
	 */
	    if (!Lf->fsdir || !Lf->dev_def || Lf->inp_ty != 1)
		return((char *)NULL);
	    for (mtp = readmnt(); mtp; mtp = mtp->next) {
		if (!mtp->dir || !mtp->inode)
		    continue;
		if (Lf->dev == mtp->dev
		&&  mtp->inode == Lf->inode
		&&  strcmp(mtp->dir, Lf->fsdir) == 0)
		    return(cp);
	    }
	    return((char *)NULL);
	}
/*
 * Begin the path assembly.
 */
	if ((nl = lc->nl) > (blen - 1))
	    return((char *)NULL);
	cp = buf + blen - nl - 1;
	rlen = blen - nl - 1;
	(void) strcpy(cp, lc->nm);
/*
 * Look up the name cache entries that are parents of the node address.
 * Quit when:
 *
 *	there's no parent;
 *	the name is too large to fit in the receiving buffer.
 */
	for (;;) {
	    if (!lc->pa) {
		if (ncache_isroot(lc->dp, cp))
		    *fp = 1;
		break;
	    }
	    lc = lc->pa;
	    if (((nl = lc->nl) + 1) > rlen)
		break;
	    *(cp - 1) = '/';
	    cp--;
	    rlen--;
	    (void) strncpy((cp - nl), lc->nm, nl);
	    cp -= nl;
	    rlen -= nl;
	}
	return(cp);
}
#endif	/* defined(HASNCACHE) && solaris>=90000 */
