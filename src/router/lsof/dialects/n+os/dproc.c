/*
 * dproc.c - NEXTSTEP and OPENSTEP process access functions for lsof
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
static char *rcsid = "$Id: dproc.c,v 1.12 2001/08/09 11:44:07 abe Exp $";
#endif

#include "lsof.h"


/*
 * Local static values
 */

static int Mxp = 0;			/* maximum number of processes */
static int Np;				/* number of entries in P[] */
static int Nv = 0;			/* allocated Vp[] entries */
static struct proc *P = (struct proc *)NULL;
					/* local proc structure table */
static KA_T *Pa = (KA_T *)NULL;		/* kernel address for each P[] entry */
static KA_T Kp;				/* kernel process table pointer */
static KA_T *Vp = (KA_T *)NULL;		/* vnode address cache */


_PROTOTYPE(static void get_kernel_access,(void));
_PROTOTYPE(static void process_map,(caddr_t map));
_PROTOTYPE(static void read_proc,(void));


/*
 * ckkv - check kernel version
 */

void
ckkv(d, er, ev, ea)
	char *d;			/* dialect */
	char *er;			/* expected release */
	char *ev;			/* expected version */
	char *ea;			/* expected architecture */
{

#if	defined(HASKERNIDCK)
	char m[128], *t;
	kernel_version_t kv;
	kern_return_t kr;
	char *vt = (char *)NULL;

	if (Fwarn)
	    return;
/*
 * Read Mach kernel version.
 */
	if ((kr = host_kernel_version(host_self(), kv)) != KERN_SUCCESS) {
	    (void) snpf(m, sizeof(m), "%s: can't get kernel version:", Pn);
	    mach_error(m, kr);
	    Exit(1);
	}
/*
 * Skip blank-separated tokens until reaching "Mach".  The kernel version
 * string follows.  Eliminate anything but decimal digits and periods from
 * the kernel version string.
 */
	if ((t = strtok(kv, " "))) {
	    do {
		if (strcmp(t, "Mach") == 0)
		    break;
	    } while ((t = strtok((char *)NULL, " ")));
	    if (t)
		vt = strtok((char *)NULL, " ");
	}
	if (vt) {
	    for (t = vt; *t; t++) {
		if (*t == '.' || (*t >= '0' && *t <= '9'))
		    continue;
		*t = '\0';
		break;
	    }
	}
/*
 * Warn if the actual and expected versions don't match.
 */
	if (!ev || !vt || strcmp(ev, vt))
	    (void) fprintf(stderr,
		"%s: WARNING: compiled for %s version %s; this is %s\n",
		Pn, d, ev ? ev : "UNKNOWN", vt ? vt : "UNKNOWN");
#endif	/* defined(HASKERNIDCK) */

}


/*
 * gather_proc_info() -- gather process information
 */

void
gather_proc_info()
{
	int i, nf, px;
	MALLOC_S nb;
	short pss, sf;
	struct task {			/* (Should come from <kern/task.h>.) */
		caddr_t d1[SIMPLE_LOCK_SIZE + 2];
		caddr_t map;
		caddr_t d2[SIMPLE_LOCK_SIZE + 9];
		struct utask *u_address;
		struct proc *proc;
	} t;
	struct utask *u;
	static struct file **uf = (struct file **)NULL;
	static MALLOC_S ufb = 0;
	struct utask ut;

#if	defined(HASFSTRUCT)
	static char *pof = (char *)NULL;
	static MALLOC_S pofb = 0;
#endif	/* defined(HASFSTRUCT) */


/*
 * Clear previously loaded tables and read the process table.
 */

#if	STEPV>=31
	(void) clr_svnc();
#endif	/* STEPV>=31 */

	(void) read_proc();
/*
 * Process proc structures pre-loaded in initialize().
 */
	for (px = 0, u = &ut; px < Np; px++) {
	    if (is_proc_excl(P[px].p_pid, (int)P[px].p_pgrp,
			     (UID_ARG)P[px].p_uid, &pss, &sf))
		continue;
	/*
	 * Read the task associated with the process, and the user
	 * area assocated with the task.
	 */
	    if (kread((KA_T)P[px].task, (char *)&t, sizeof(t)))
		continue;
	    if ((KA_T)t.proc != Pa[px])
		continue;
	    if (kread((KA_T)t.u_address, (char *)&ut, sizeof(ut)))
		continue;
	    if ((KA_T)ut.uu_procp != Pa[px])
		continue;
	/*
	 * Allocate a local process structure and start filling it.
	 */
	    if (is_cmd_excl(u->u_comm, &pss, &sf))
		continue;
	    alloc_lproc(P[px].p_pid, (int)P[px].p_pgrp, (int)P[px].p_ppid,
			(UID_ARG)P[px].p_uid, u->u_comm, (int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;
	/*
	 * Save current working directory information.
	 */
	    if (u->u_cdir) {
		alloc_lfile(CWD, -1);
		FILEPTR = (struct file *)NULL;
		process_node((KA_T)u->u_cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (u->u_rdir) {
		alloc_lfile(RTD, -1);
		FILEPTR = (struct file *)NULL;
		process_node((KA_T)u->u_rdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Print information on the text files of the virtual memory
	 * address map.
	 */
	    if (t.map)
		process_map(t.map);
	/*
	 * Save information on file descriptors.
	 *
	 * NEXTSTEP file pointers come from a structure whose pointer is
	 * in the user task area.
	 */
	    nf = ut.uu_ofile_cnt;
	    nb = (MALLOC_S)(sizeof(struct file *) * nf);
	    if (nb > ufb) {
		if (!uf)
		    uf = (struct file **)malloc(nb);
		else
		    uf = (struct file **)realloc((MALLOC_P *)uf, nb);
		if (!uf) {
		    (void) fprintf(stderr, "%s: no uu_ofile space\n", Pn);
		    Exit(1);
		}
		ufb = nb;
	    }
	    if (kread((KA_T)ut.uu_ofile, (char *)uf, nb))
		continue;

#if	defined(HASFSTRUCT)
	    if (Fsv & FSV_FG) {
		nb = (MALLOC_S)(sizeof(char) * nf);
		if (nb > pofb) {
		    if (!pof)
			pof = (char *)malloc(nb);
		    else
			pof = (char *)realloc((MALLOC_P *)pof, nb);
		    if (!pof) {
			(void) fprintf(stderr, "%s: no uu_pofile space\n", Pn);
			Exit(1);
		    }
		    pofb = nb;
		}
		if (kread((KA_T)ut.uu_pofile, (char *)pof, nb))
		    zeromem(pof, nb);
	    }
#endif	/* defined(HASFSTRUCT) */

	    for (i = 0; i < nf; i++) {
		if (uf[i]) {
		    alloc_lfile((char *)NULL, i);
		    process_file((KA_T)uf[i]);
		    if (Lf->sf) {

#if	defined(HASFSTRUCT)
			if (Fsv & FSV_FG)
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
 * get_kernel_access() - access the required information in the kernel
 */

static void
get_kernel_access()
{
	int i;
	KA_T lv;

#if	defined(HAS_AFS)
	struct nlist *nl = (struct nlist *)NULL;
	unsigned long v[3];
#endif	/* defined(HAS_AFS) */

/*
 * Check kernel version against compiled version.
 */
	ckkv("NEXTSTEP", (char *)NULL, LSOF_VSTR, (char *)NULL);

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
 * Access the kernel memory file.
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
 * See if the name list file is readable.  Build Nl.
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
		if (!(nl = (struct nlist *)malloc(Nll))) {
			(void) fprintf(stderr,
				"%s: no space (%d) for Nl[] copy\n", Pn, Nll);
			Exit(1);
		}
		(void) bcopy((char *)Nl, (char *)nl, Nll);
	}
#endif	/* defined(HAS_AFS) */

/*
 * Access the name list file.
 */
	if (nlist(Nmlst ? Nmlst : VMUNIX, Nl) < 0) {
		(void) fprintf(stderr, "%s: can't read namelist from %s\n",
			Pn, Nmlst ? Nmlst : VMUNIX);
                Exit(1);
	}
	if (get_Nl_value("aproc", Drive_Nl, &lv) < 0 || !lv) {
		(void) fprintf(stderr, "%s: can't get proc table address\n",
			Pn);
		Exit(1);
	}

#if	defined(HAS_AFS)
	if (nl) {

	/*
	 * If AFS is defined and we're getting kernel symbol values from
	 * N_UNIX, and if any X_AFS_* symbols isn't there, see if it is in the
	 * the AFS module name list file.  Make sure that other symbols that
	 * appear in both name list files have the same values.
	 */
		if (get_Nl_value("arFID", Drive_Nl, &v[0]) >= 0 
		&&  get_Nl_value("avol",  Drive_Nl, &v[1]) >= 0
		&&  get_Nl_value("avol",  Drive_Nl, &v[2]) >= 0
		&&  (!vo[0] || !v[1] || !v[2]))
			(void) ckAFSsym(nl);
		(void) free((MALLOC_P *)nl);
	}
#endif	/* defined(HAS_AFS) */

/*
 * Establish a maximum process count estimate.
 */
	if (get_Nl_value("mxproc", Drive_Nl, &lv) < 0
	||  kread((KA_T)lv, (char *)&Mxp, sizeof(Mxp))
	||  Mxp < 1)
		Mxp = PROCDFLT;
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


/*
 * process_map() - process vm map for vnode references
 */

static void
process_map(map)
	caddr_t map;
{
	int i, j, n, ne;

#if	STEPV<40
/*
 * Structures for NeXTSTEP and OPENSTEP < 4.0.
 */
	struct vm_map_entry {	/* (Should come from <vm/vm_map.h>). */
		struct vm_map_entry *prev;
		struct vm_map_entry *next;
		unsigned int start;
		unsigned int end;
		caddr_t object;
		unsigned int offset;
		unsigned int
			is_a_map:1,
			is_sub_map:1,
			copy_on_write:1,
			needs_copy:1;
		int protection;
		int max_protection;
		int inheritance;
		int wired_count;
	} vme, *vmep;

#define VME_NEXT(entry) entry.next

	struct vm_map {		/* (Should come from <vm/vm_map.h>.) */
		caddr_t d1[SIMPLE_LOCK_SIZE + 2];
		struct vm_map_entry header;
		int nentries;
		caddr_t pmap;
		unsigned int size;
		boolean_t is_main_map;
	} vmm;
	struct vm_object {	/* (Should come from <vm/vm_object.h>.) */
		caddr_t d1[SIMPLE_LOCK_SIZE + 4];
		unsigned int size;
		short ref_count, resident_page_count;
		caddr_t copy;
		caddr_t pager;
		int pager_request, pager_name;
		unsigned int paging_offset;
		caddr_t shadow;
	} vmo, vmso;
#else	/* STEPV>=40 */
/*
 * Structures for OPENSTEP >= 4.0.
 */
	struct vm_map_links {   /* (Should come from <vm/vm_map.h>). */
		struct vm_map_entry *prev;
		struct vm_map_entry *next;
		unsigned int start;
		unsigned int end;
	};
	struct vm_map_entry {	/* (Should come from <vm/vm_map.h>). */
		struct vm_map_links links;
		caddr_t object;
		unsigned int offset;
		unsigned int
			is_shared:1,
			is_sub_map:1,
			in_transition:1,
			needs_wakeup:1,
			behavior:2,
			needs_copy:1,
			protection:3,
			max_protection:3,
			inheritance:2,
			pad1:1,
			alias:8;
		unsigned short wired_count;
		unsigned short user_wired_count;
	} vme, *vmep;

#define VME_NEXT(entry) entry.links.next

	struct vm_map_header {   /* (Should come from <vm/vm_map.h>.) */
		struct vm_map_links links;
		int nentries;
		int entries_pageable;
	};
	struct vm_map {		     /* (Should come from <vm/vm_map.h>.) */
		caddr_t d1[SIMPLE_LOCK_SIZE + 2];
		struct vm_map_header hdr;
		caddr_t pmap;
		unsigned int size;
		boolean_t is_main_map; /* Darwin header has this as ref_count,
					* but we'll take some liberties ... */
	} vmm;
	struct vm_object {	     /* (Should come from <vm/vm_object.h>.) */
		caddr_t d1[SIMPLE_LOCK_SIZE + 4];
		unsigned int size;
		short ref_count, resident_page_count;
		caddr_t copy;
		caddr_t shadow;
		unsigned int shadow_offset;
		caddr_t pager;
		unsigned int paging_offset;
		int pager_request;
	} vmo, vmso;
#endif	/* STEPV<40 */

	struct vstruct {	/* (Should come from <vm/vnode_pager.h>.) */
		boolean_t is_device;
		caddr_t vs_pf;
		caddr_t pfMapEntry;
		unsigned int vs_swapfile:1;
		short vs_count;
		int vs_size;
		caddr_t vs_vp;
	} vmp;
/*
 * Read the vm map.
 */
	if (!map
	||  kread((KA_T)map, (char *)&vmm, sizeof(vmm)))
		return;
	if (!vmm.is_main_map)
		return;
/*
 * Look for non-map and non-sub-map vm map entries that have an object
 * with a shadow whose pager pointer addresses a non-swap-file istruct
 * that has a vnode pointer.  Process the unique vnodes found.
 */ 
#if	STEPV<40
	vme = vmm.header;
	ne = vmm.nentries;
#else	/* STEPV>=40 */
	if (!vmm.hdr.links.next
	||  kread((KA_T)vmm.hdr.links.next, (char *)&vme, sizeof(vme)))
	    return;
	ne = vmm.hdr.nentries;
#endif	/* STEPV<40 */

	if (ne > 1000)
	    ne = 1000;
	for (i = n = 0; i < ne; i++) {
	    if (i) {
		if (!VME_NEXT(vme)
		||  kread((KA_T)VME_NEXT(vme), (char *)&vme, sizeof(vme)))
		    continue;
	    }

#if	STEPV<40
	    if (vme.is_a_map || vme.is_sub_map)
#else	/* STEPV>=40 */
	    if (vme.is_sub_map)
#endif	/* STEPV<40 */

		continue;
	    if (!vme.object
	    ||  kread((KA_T)vme.object, (char *)&vmo, sizeof(vmo)))
		continue;
	    if (!vmo.shadow
	    ||  kread((KA_T)vmo.shadow, (char *)&vmso, sizeof(vmso)))
		continue;
	    if (!vmso.pager
	    ||  kread((KA_T)vmso.pager, (char *)&vmp, sizeof(vmp)))
		continue;
	    if (vmp.is_device || vmp.vs_swapfile || !vmp.vs_vp)
		continue;
	/*
	 * See if the vnode has been processed before.
	 */
	    for (j = 0; j < n; j++) {
		if ((KA_T)vmp.vs_vp == Vp[j])
		    break;
	    }
	    if (j < n)
		continue;
	/*
	 * Process a new vnode.
	 */
	    alloc_lfile("txt", -1);
	    FILEPTR = (struct file *)NULL;
	    process_node((KA_T)vmp.vs_vp);
	    if (Lf->sf)
		link_lfile();
	/*
	 * Allocate space for remembering the vnode.
	 */
	    if (!Vp) {
		if (!(Vp = (KA_T *)malloc((MALLOC_S)
					  (sizeof(struct vnode *) * 10))))
		{
		    (void) fprintf(stderr, "%s: no txt ptr space, PID %d\n",
			Pn, Lp->pid);
		    Exit(1);
		}
		Nv = 10;
	    } else if (n >= Nv) {
		Nv += 10;
		if (!(Vp = (KA_T *)realloc((MALLOC_P *)Vp,
			       (MALLOC_S)(Nv * sizeof(struct vnode *)))))
		{
		    (void) fprintf(stderr,
			"%s: no more txt ptr space, PID %d\n", Pn, Lp->pid);
		    Exit(1);
		}
	    }
	    Vp[n++] = (KA_T)vmp.vs_vp;
	}
}


/*
 * read_proc() - read proc structures
 */

static void
read_proc()
{
	static KA_T apav = (KA_T)0;
	static int apax = -1;
	int i, try;
	static int sz = 0;
	KA_T kp;
	struct proc *p;
/*
 * Try PROCTRYLM times to read a valid proc table.
 */
	for (try = 0; try < PROCTRYLM; try++) {

	/*
	 * Read kernel's process list pointer.  This needs to be done each
	 * time lsof rereads the process list.
	 */
	    if (apax < 0) {
		if ((apax = get_Nl_value("aproc", Drive_Nl, &apav)) < 0) {
		    (void) fprintf(stderr,
			"%s: can't get process table address pointer\n", Pn);
		    Exit(1);
		}
	    }
	    if (kread((KA_T)apav, (char *)&Kp, sizeof(Kp))) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: can't read %s from %#x\n",
			Pn, Nl[apax].n_un.n_name, apav);
		continue;
	    }

	/*
	 * Pre-allocate proc structure space.
	 */
		if (sz == 0) {
		    sz = Mxp;
		    if (!(P = (struct proc *)malloc((MALLOC_S)
				(sz * sizeof(struct proc)))))
		    {
			(void) fprintf(stderr, "%s: no proc table space\n",
			    Pn);
			Exit(1);
		    }
		    if (!(Pa = (KA_T *)malloc((MALLOC_S)(sz * sizeof(KA_T)))))
		    {
			(void) fprintf(stderr, "%s: no proc pointer space\n",
			    Pn);
			Exit(1);
		    }
		}
	/*
	 * Accumulate proc structures.
	 */
		for (kp = Kp, Np = 0; kp; ) {
			if (kread(kp, (char *)&P[Np], sizeof(struct proc))) {
				Np = 0;
				break;
			}
			Pa[Np] = kp;
			kp = (KA_T)P[Np].p_nxt;
			if (P[Np].p_stat == 0 || P[Np].p_stat == SZOMB)
				continue;
			Np++;
			if (Np >= sz) {

			/*
			 * Expand the local proc table.
			 */
				sz += PROCDFLT/2;
				if (!(P = (struct proc *)realloc((MALLOC_P *)P,
					(MALLOC_S)(sizeof(struct proc) * sz))))
				{
					(void) fprintf(stderr,
						"%s: no more (%d) proc space\n",
						Pn, sz);
					Exit(1);
				}
				if (!(Pa = (KA_T *)realloc((MALLOC_P *)Pa,
					(MALLOC_S)(sizeof(KA_T) * sz))))
				{
					(void) fprintf(stderr,
					    "%s: no more (%d) proc ptr space\n",
					    Pn, sz);
					Exit(1);
				}
			}
		}
	/*
	 * If not enough processes were saved in the local table, try again.
	 */
		if (Np >= PROCMIN)
			break;
	}
/*
 * Quit if no proc structures were stored in the local table.
 */
	if (try >= PROCTRYLM) {
		(void) fprintf(stderr, "%s: can't read proc table\n", Pn);
		Exit(1);
	}
	if (Np < sz && !RptTm) {

	/*
	 * Reduce the local proc structure table size to a minimum if
	 * not in repeat mode.
	 */
		if (!(P = (struct proc *)realloc((MALLOC_P *)P,
			 (MALLOC_S)(sizeof(struct proc) * Np))))
		{
			(void) fprintf(stderr,
				"%s: can't reduce proc table to %d\n",
				Pn, Np);
			Exit(1);
		}
		if (!(Pa = (KA_T *)realloc((MALLOC_P *)Pa,
			  (MALLOC_S)(sizeof(KA_T) * Np))))
		{
			(void) fprintf(stderr,
				"%s: can't reduce proc ptrs to %d\n",
				Pn, Np);
			Exit(1);
		}
	}
}
