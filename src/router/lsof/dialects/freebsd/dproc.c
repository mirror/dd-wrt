/*
 * dproc.c - FreeBSD process access functions for lsof
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
static char *rcsid = "$Id: dproc.c,v 1.19 2015/07/07 20:23:43 abe Exp $";
#endif

#include "lsof.h"


_PROTOTYPE(static void enter_vn_text,(KA_T va, int *n));
_PROTOTYPE(static void get_kernel_access,(void));
_PROTOTYPE(static void process_text,(KA_T vm));


/*
 * Local static values
 */

static MALLOC_S Nv = 0;			/* allocated Vp[] entries */
static KA_T *Vp = NULL;			/* vnode address cache */


/*
 * enter_vn_text() - enter a vnode text reference
 */

static void
enter_vn_text(va, n)
	KA_T va;			/* vnode address */
	int *n;				/* Vp[] entries in use */
{
	int i;
/*
 * Ignore the request if the vnode has already been entered.
 */
	for (i = 0; i < *n; i++) {
	    if (va == Vp[i])
		return;
	}
/*
 * Save the text file information.
 */
	alloc_lfile(" txt", -1);
	Cfp = (struct file *)NULL;
	process_node(va);
	if (Lf->sf)
	    link_lfile();
	if (i >= Nv) {

	/*
	 * Allocate space for remembering the vnode.
	 */
	    Nv += 10;
	    if (!Vp)
		Vp=(KA_T *)malloc((MALLOC_S)(sizeof(struct vnode *)*10));
	    else
		Vp=(KA_T *)realloc((MALLOC_P *)Vp,(MALLOC_S)(Nv*sizeof(KA_T)));
	    if (!Vp) {
		(void) fprintf(stderr, "%s: no txt ptr space, PID %d\n",
		    Pn, Lp->pid);
		Exit(1);
	    }
	}
/*
 * Remember the vnode.
 */
	Vp[*n] = va;
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
	struct filedesc fd;
	int i, nf;
	MALLOC_S nb;

#if	defined(HAS_FILEDESCENT)
	typedef struct filedescent ofb_t;
#else	/* !defined(HAS_FILEDESCENT) */
	typedef struct file* ofb_t;
#endif	/* defined(HAS_FILEDESCENT) */

#if	defined(HAS_FDESCENTTBL)
	struct fdescenttbl fdt;
	KA_T fa;
#endif	/* defined(HAS_FDESCENTTBL) */

	static ofb_t *ofb = NULL;
	static int ofbb = 0;
	int pgid, pid;
	int ppid = 0;
	short pss, sf;
	int px;
	int tid;			/* thread (task) ID */
	uid_t uid;

#if	FREEBSDV<2000
	struct proc *p;
	struct pcred pc;
	struct pgrp pg;
#else	/* FREEBSDV>=2000 */
	struct kinfo_proc *p;
#endif	/* FREEBSDV<2000 */

#if	defined(HASFSTRUCT) && !defined(HAS_FILEDESCENT)
	static char *pof = (char *)NULL;
	static int pofb = 0;
#endif	/* defined(HASFSTRUCT) && !defiled(HAS_FILEDESCENT) */

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

#if	FREEBSDV<2000
	if ((Np = kvm_getprocs(KINFO_PROC_ALL, 0)) < 0)
#else	/* FREEBSDV>=2000 */

# if	!defined(KERN_PROC_PROC)
#define	KERN_PROC_PROC  KERN_PROC_ALL
# endif	/* !defined(KERN_PROC_PROC) */

	if ((P = kvm_getprocs(Kd, Ftask ? KERN_PROC_ALL : KERN_PROC_PROC,
			      0, &Np))
	== NULL)
#endif	/* FREEBSDV<2000 */

	{
	    (void) fprintf(stderr, "%s: can't read process table: %s\n",
		Pn,

#if	FREEBSDV<2000
		kvm_geterr()
#else	/* FREEBSDV>=2000 */
		kvm_geterr(Kd)
#endif	/* FREEBSDV<2000 */

	    );
	    Exit(1);
	}
/*
 * Examine proc structures and their associated information.
 */

#if	FREEBSDV<2000
	for (px = 0; px < Np; px++)
#else	/* FREEBSDV>=2000 */
	for (p = P, px = 0; px < Np; p++, px++)
#endif	/* FREEBSDV<2000 */

	{

#if	FREEBSDV<2000
	/*
	 * Read process information, process group structure (if
	 * necessary), and User ID (if necessary).
	 */
	    if (!(p = kvm_nextproc()))
		continue;
	    if (p->P_STAT == 0 || p->P_STAT == SZOMB)
		continue;
	    pg.pg_id = 0;
	    if (Fpgid && p->P_PGID) {
		if (kread((KA_T)p->P_PGID, (char *)&pg, sizeof(pg)))
		    continue;
	    }
	    pgid = pg.pg_id;
	    if (!p->p_cred
	    ||  kread((KA_T)p->p_cred, (char *)&pc, sizeof(pc)))
		continue;
	    uid = pc.p_ruid;
#else	/* FREEBSDV>=2000 */
	    if (p->P_STAT == 0 || p->P_STAT == SZOMB)
		continue;
	    pgid = p->P_PGID;
# if	FREEBSDV<5000
	    uid = p->kp_eproc.e_ucred.cr_uid;
# else	/* FREEBSDV>=5000 */
	    uid = p->ki_uid;
# endif	/* FREEBSDV<5000 */
#endif	/* FREEBSDV<2000 */

#if	defined(HASPPID)
	    ppid = p->P_PPID;
#endif	/* defined(HASPPID) */

#if	defined(HASTASKS)
	/*
	 * See if process,including its tasks, is excluded.
	 */
	    tid = Ftask ? (int)p->ki_tid : 0;
	    if (is_proc_excl(p->P_PID, pgid, (UID_ARG)uid, &pss, &sf, tid))
		continue;
#else	/* !defined(HASTASKS) */
	/*
	 * See if process is excluded.
	 */
	    if (is_proc_excl(p->P_PID, pgid, (UID_ARG)uid, &pss, &sf))
		continue;
#endif	/* defined(HASTASKS) */

	/*
	 * Read file structure pointers.
	 */
	    if (!p->P_FD
	    ||  kread((KA_T)p->P_FD, (char *)&fd, sizeof(fd)))
		continue;

#if	defined(HAS_FDESCENTTBL)
	    if (!fd.fd_files
	    ||  kread((KA_T)fd.fd_files, (char *)&fdt, sizeof(fdt)))
		continue;
	    if (!fd.fd_refcnt || fd.fd_lastfile > fdt.fdt_nfiles)
		continue;
#else	/* !defined(HAS_FDESCENTTBL) */
	    if (!fd.fd_refcnt || fd.fd_lastfile > fd.fd_nfiles)
		continue;
#endif	/* defined(HAS_FDESCENTTBL) */

	/*
	 * Allocate a local process structure.
	 */
	    if (is_cmd_excl(p->P_COMM, &pss, &sf))
		continue;
	    if (cckreg) {

	    /*
	     * If conditional checking of regular files is enabled, enable
	     * socket file only checking, based on the process' selection
	     * status.
	     */
		ckscko = (sf & SELPROC) ? 0 : 1;
	    }
	    alloc_lproc(p->P_PID, pgid, ppid, (UID_ARG)uid, p->P_COMM,
		(int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;

#if	defined(HASTASKS)
	/*
	 * Save the task (thread) ID.
	 */
	    Lp->tid = tid;
#endif	/* defined(HASTASKS) */

#if	defined(P_ADDR)
	/*
	 * Save the kernel proc struct address, if P_ADDR is defined.
	 */
	    Kpa = (KA_T)p->P_ADDR;
#endif	/* defined(P_ADDR) */

	/*
	 * Save current working directory information.
	 */
	    if (!ckscko && fd.fd_cdir) {
		alloc_lfile(CWD, -1);
		Cfp = (struct file *)NULL;
		process_node((KA_T)fd.fd_cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (!ckscko && fd.fd_rdir) {
		alloc_lfile(RTD, -1);
		Cfp = (struct file *)NULL;
		process_node((KA_T)fd.fd_rdir);
		if (Lf->sf)
		    link_lfile();
	    }

#if	FREEBSDV>=5000
	/*
	 * Save jail directory information.
	 */
	    if (!ckscko && fd.fd_jdir) {
		alloc_lfile("jld", -1);
		Cfp = (struct file *)NULL;
		process_node((KA_T)fd.fd_jdir);
		if (Lf->sf)
		    link_lfile();
	    }
#endif	/* FREEBSDV>=5000 */

	/*
	 * Save information on the text file.
	 */
	    if (!ckscko && p->P_VMSPACE)
		process_text((KA_T)p->P_VMSPACE);
	/*
	 * Read open file structure pointers.
	 */

#if	defined(HAS_FDESCENTTBL)
	    if ((nf = fdt.fdt_nfiles) <= 0)
		continue;
#else	/* !defined(HAS_FDESCENTTBL) */
	    if (!fd.fd_ofiles || (nf = fd.fd_nfiles) <= 0)
		continue;
#endif	/* defined(HAS_FDESCENTTBL) */

	    nb = (MALLOC_S)(sizeof(ofb_t) * nf);
	    if (nb > ofbb) {
		if (!ofb)
		    ofb = (ofb_t *)malloc(nb);
		else
		    ofb = (ofb_t *)realloc((MALLOC_P *)ofb, nb);
		if (!ofb) {
		    (void) fprintf(stderr, "%s: PID %d, no file * space\n",
			Pn, p->P_PID);
		    Exit(1);
		}
		ofbb = nb;
	    }

#if	defined(HAS_FDESCENTTBL)
	    fa = (KA_T)fd.fd_files
	       + (KA_T)offsetof(struct fdescenttbl, fdt_ofiles);
	    if (kread(fa, (char *)ofb, nb))
		continue;
#else	/* !defined(HAS_FDESCENTTBL) */
	    if (kread((KA_T)fd.fd_ofiles, (char *)ofb, nb))
		continue;
#endif	/* defined(HAS_FDESCENTTBL) */


#if	defined(HASFSTRUCT) && !defined(HAS_FILEDESCENT)
	    if (Fsv & FSV_FG) {
		nb = (MALLOC_S)(sizeof(char) * nf);
		if (nb > pofb) {
		    if (!pof)
			pof = (char *)malloc(nb);
		    else
			pof = (char *)realloc((MALLOC_P *)pof, nb);
		    if (!pof) {
			(void) fprintf(stderr,
			    "%s: PID %d, no file flag space\n", Pn, p->P_PID);
			Exit(1);
		    }
		    pofb = nb;
		}
		if (!fd.fd_ofileflags || kread((KA_T)fd.fd_ofileflags, pof, nb))
		    zeromem(pof, nb);
	    }
#endif	/* defined(HASFSTRUCT) && !defined(HAS_FILEDESCENT) */

	/*
	 * Save information on file descriptors.
	 */
	    for (i = 0; i < nf; i++) {

#if	defined(HAS_FILEDESCENT)
		if ((Cfp = ofb[i].fde_file))
#else	/* !defined(HAS_FILEDESCENT) */
		if ((Cfp = ofb[i]))
#endif	/* defined(HAS_FILEDESCENT) */

		{
		    alloc_lfile(NULL, i);
		    process_file((KA_T)Cfp);
		    if (Lf->sf) {

#if	defined(HASFSTRUCT)
			if (Fsv & FSV_FG)
# if	defined(HAS_FILEDESCENT)
			    Lf->pof = (long)ofb[i].fde_flags;
# else	/* !defined(HAS_FILEDESCENT) */
			    Lf->pof = (long)pof[i];
# endif	/* defined(HAS_FILEDESCENT) */
#endif	/* defined(HASFSTRUCT) */

			link_lfile();
		    }
		}
	    }
	/*
	 * Unless threads (tasks) are being processed, examine results.
	 */
	    if (!Ftask) {
		if (examine_lproc())
		    return;
	    }
	}
}


/*
 * get_kernel_access() - get access to kernel memory
 */

static void
get_kernel_access()
{

/*
 * Check kernel version.
 */
	(void) ckkv("FreeBSD", LSOF_VSTR, (char *)NULL, (char *)NULL);
/*
 * Set name list file path.
 */
	if (!Nmlst)

#if	defined(N_UNIX)
	    Nmlst = N_UNIX;
#else	/* !defined(N_UNIX) */
	{
	    if (!(Nmlst = get_nlist_path(1))) {
		(void) fprintf(stderr,
		    "%s: can't get kernel name list path\n", Pn);
		Exit(1);
	    }
	}
#endif	/* defined(N_UNIX) */

#if	defined(WILLDROPGID)
/*
 * If kernel memory isn't coming from KMEM, drop setgid permission
 * before attempting to open the (Memory) file.
 */
	if (Memory)
	    (void) dropgid();
#else	/* !defined(WILLDROPGID) */
/*
 * See if the non-KMEM memory and the name list files are readable.
 */
	if ((Memory && !is_readable(Memory, 1))
	||  (Nmlst && !is_readable(Nmlst, 1)))
	    Exit(1);
#endif	/* defined(WILLDROPGID) */

/*
 * Open kernel memory access.
 */

#if	FREEBSDV<2000
	if (kvm_openfiles(Nmlst, Memory, NULL) == -1)
#else	/* FREEBSDV>=2000 */
	if ((Kd = kvm_open(Nmlst, Memory, NULL, O_RDONLY, NULL)) == NULL)
#endif	/* FREEBSDV<2000 */

	{
	    (void) fprintf(stderr,
		"%s: kvm_open%s(execfile=%s, corefile=%s): %s\n",
		Pn,

#if	FREEBSDV<2000
		"files",
#else	/* FREEBSDV>=2000 */
		"",
#endif	/* FREEBSDV<2000 */

		Nmlst ? Nmlst : "default",
		Memory ? Memory :

#if	defined(_PATH_MEM)
				  _PATH_MEM,
#else	/* !defined(_PATH_MEM) */
				  "default",
#endif	/* defined(_PATH_MEM) */

		strerror(errno));
	    Exit(1);
	}
	(void) build_Nl(Drive_Nl);
	if (kvm_nlist(Kd, Nl) < 0) {
	    (void) fprintf(stderr, "%s: can't read namelist from %s\n",
		Pn, Nmlst);
	    Exit(1);
	}

#if	defined(WILLDROPGID)
/*
 * Drop setgid permission, if necessary.
 */
	if (!Memory)
	    (void) dropgid();
#endif	/* defined(WILLDROPGID) */

}


#if	!defined(N_UNIX)
/*
 * get_nlist_path() - get kernel name list path
 */

char *
get_nlist_path(ap)
	int ap;				/* on success, return an allocated path
					 * string pointer if 1; return a
					 * constant character pointer if 0;
					 * return NULL if failure */
{
	const char *bf;
	static char *bfc;
	MALLOC_S bfl;
/*
 * Get bootfile name.
 */
	if ((bf = getbootfile())) {
	    if (!ap)
		return("");
	    bfl = (MALLOC_S)(strlen(bf) + 1);
	    if (!(bfc = (char *)malloc(bfl))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for boot file path: %s\n",
		    Pn, (int)bfl, bf);
		Exit(1);
	    }
	    (void) snpf(bfc, bfl, "%s", bf);
	    return(bfc);
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

#if	FREEBSDV<2000
	br = kvm_read((void *)addr, (void *)buf, len);
#else	/* FREEBSDV>=2000 */
	br = kvm_read(Kd, (u_long)addr, buf, len);
#endif	/* FREEBSDV<2000 */

	return((br == len) ? 0 : 1);
}


/*
 * process_text() - process text information
 */
void
process_text(vm)
	KA_T vm;				/* vm space pointer */
{
	int i, j;
	KA_T ka;
	int n = 0;
	struct vm_map_entry vmme, *e;
	struct vm_object vmo;
	struct vmspace vmsp;

#if	FREEBSDV<2020
	struct pager_struct pg;
#endif	/* FREEBSDV<2020 */

/*
 * Read the vmspace structure for the process.
 */
	if (kread(vm, (char *)&vmsp, sizeof(vmsp)))
	    return;
/*
 * Read the vm_map structure.  Search its vm_map_entry structure list.
 */
	for (i = 0; i < vmsp.vm_map.nentries; i++) {

	/*
	 * Read the next vm_map_entry.
	 */
	    if (i == 0)
		e = &vmsp.vm_map.header;
	    else {
		if (!(ka = (KA_T)e->next))
		    return;
		e = &vmme;
		if (kread(ka, (char *)e, sizeof(vmme)))
		    return;
	    }

#if	defined(MAP_ENTRY_IS_A_MAP)
	    if (e->eflags & (MAP_ENTRY_IS_A_MAP|MAP_ENTRY_IS_SUB_MAP))
#else	/* !defined(MAP_ENTRY_IS_A_MAP) */
	    if (e->is_a_map || e->is_sub_map)
#endif	/* defined(MAP_ENTRY_IS_A_MAP) */

		continue;
	/*
	 * Read the map entry's object and the object's shadow.
	 * Look for: a PG_VNODE pager handle (FreeBSD < 2.2);
	 * an OBJT_VNODE object type (FreeBSD >= 2.2).
	 */
	    for (j = 0, ka = (KA_T)e->object.vm_object;
		 j < 2 && ka;
		 j++,

#if	FREEBSDV<2020
		 ka = (KA_T)vmo.shadow
#else	/* FREEBSDV>=2020 */
		 ka = (KA_T)vmo.backing_object
#endif	/* FREEBSDV<2020 */
		 )
	    {
		if (kread(ka, (char *)&vmo, sizeof(vmo)))
		    break;

#if	FREEBSDV<2020
		if ((ka = (KA_T)vmo.pager) == NULL
		||  kread(ka, (char *)&pg, sizeof(pg)))
		    continue;
		if (pg.pg_handle == NULL || pg.pg_type != PG_VNODE)
		    continue;
		(void) (enter_vn_text((KA_T)pg.pg_handle, &n));
#else	/* FREEBSDV>=2020 */
		if (vmo.type != OBJT_VNODE
		||  vmo.handle == (void *)NULL)
		    continue;
		(void) (enter_vn_text((KA_T)vmo.handle, &n));
#endif	/* FREEBSDV<2020 */

	    }
	}
}
