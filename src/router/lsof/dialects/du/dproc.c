/*
 * dproc.c - DEC OSF/1, Digital UNIX, Tru64 UNIX process access functions for
 *	     lsof
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
static char *rcsid = "$Id: dproc.c,v 1.23 2005/08/08 19:56:44 abe Exp $";
#endif

#include "lsof.h"


#if	DUV>=50000
# if	DUV>=50100 && defined(HASNCACHE)
#include <stddef.h>
#include <sys/namei.h>
#define	_KERNEL	1
#include <kern/processor.h>
#include <nfs/nfs_clnt.h>
#undef	_KERNEL
#include <cdfs/cdfsmount.h>
#include <dvdfs/dvdfsmount.h>
#include <ufs/ufsmount.h>
# endif	/* DUV>=50100 && defined(HASNCACHE) */
_PROTOTYPE(static KA_T vpo2vp,(struct vm_ubc_object *vpo));
#endif	/* DUV>=50000 */

_PROTOTYPE(static void enter_vn_text,(KA_T va, int *n));
_PROTOTYPE(static void get_kernel_access,(void));

#if	DUV<30000
_PROTOTYPE(static void process_text,(KA_T tp, KA_T utp));
#else	/* DUV>=30000 */
_PROTOTYPE(static void process_text,(KA_T tp));
#endif	/* DUV<30000 */

_PROTOTYPE(static void read_proc,(void));


/*
 * Local defintions
 */

#define	PAPSINCR	1024		/* Pa and Ps table increment */
#define	PAPSINIT	512		/* Pa and Ps table initial size */


/*
 * Local static values
 */

#if     DUV<30000
static KA_T Kp;				/* kernel proc[] address */
#endif  /* DUV<30000 */

static int Np = 0;			/* number of processes */
static MALLOC_S Nv = 0;			/* allocateed Vp[] entries */

#if	DUV>=30000
static KA_T Pidtab;			/* kernel pidtab[] address */
#endif	/* DUV>=30000 */

static KA_T *Vp = NULL;			/* vnode address cache */


/*
 * enter_vn_text() - enter a vnode text reference
 */

static void
enter_vn_text(va, n)
	KA_T va;			/* vnode address */
	int *n;				/* number of vnodes in vp[] */
{
	int i;

/*
 * Ignore the request if the vnode has already been printed.
 */
	for (i = 0; i < *n; i++) {
	    if (va == Vp[i])
		return;
	}
/*
 * Print the vnode.
 */
	alloc_lfile(" txt", -1);
	FILEPTR = (struct file *)NULL;
	process_node(va);
	if (Lf->sf)
	    link_lfile();
	if (i >= Nv) {

	/*
	 * Allocate space for remembering the vnode.
	 */
	    Nv += 10;
	    if (!Vp)
		Vp=(KA_T *)malloc((MALLOC_S)(sizeof(KA_T) * 10));
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
	MALLOC_S b;
	struct file *fp;
	int i, j;
	struct pgrp pg;
	int pgid, px;
	struct proc *p;
	short pss, sf;
	struct ucred pcred;
	uid_t uid;
	struct utask ut, *utp;

#if	DUV>=30000
	struct pid_entry pe;
#endif	/* DUV>=30000 */

#if	DUV<50000
	static int nufb = 0;
	static struct file **ufb = (struct file **)NULL;
#else	/* DUV>=50000 */
	int k, l;
	KA_T ka;
	struct ufile_entry *ofb[U_FE_OF_ALLOC_SIZE];
	struct ufile_entry ufe[U_FE_ALLOC_SIZE];
	struct ufile_entry ufeo[U_FE_OF_ALLOC_SIZE];
#endif	/* DUV<50000 */

#if	defined(HASFSTRUCT) && DUV>=40000
	static char *pof = (char *)NULL;
	static int pofb = 0;
	long pv;
#endif	/* defined(HASFSTRUCT) && DUV>=40000 */

/*
 * Clear file lock information.
 */
	(void) clr_flinfo();
/*
 * Read process table entries.
 */
	read_proc();
/*
 * Examine proc structures and their associated information.
 */
	for (p = Ps, px = 0, utp = &ut; px < Psn; p++, px++) {
	    if (p->p_stat == 0 || p->p_stat == SZOMB)
		continue;
	    if (Fpgid) {
		if (!p->p_pgrp
		||  kread((KA_T)p->p_pgrp, (char *)&pg, sizeof(pg)))
		    continue;
		pgid = pg.pg_id;
	    } else
		pgid = 0;
	    if (p->p_rcred == NULL
	    ||  kread((KA_T)p->p_rcred, (char *)&pcred, sizeof(pcred)))
		continue;
	    uid = (uid_t)pcred.cr_uid;
	    if (is_proc_excl(p->p_pid, pgid, (UID_ARG)uid, &pss, &sf))
		continue;

#if	DUV<30000
	    if (!p->utask
	    ||  kread((KA_T)p->utask, (char *)&ut, sizeof(ut)))
#else	/* DUV>=30000 */
	    if (kread((KA_T)((char *)Pa[px] + sizeof(struct proc)),
		(char *)&ut, sizeof(ut)))
#endif	/* DUV<30000 */

		continue;
	/*
	 * Allocate a local process structure.
	 */
	    if (is_cmd_excl(utp->u_comm, &pss, &sf))
		continue;
	    alloc_lproc((int)p->p_pid, pgid, (int)p->p_ppid, (UID_ARG)uid,
		utp->u_comm, (int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;
	/*
	 * Save current working directory information.
	 */
	    if (utp->uu_utnd.utnd_cdir) {
		alloc_lfile(CWD, -1);
		FILEPTR = (struct file *)NULL;
		process_node((KA_T)utp->uu_utnd.utnd_cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (utp->uu_utnd.utnd_rdir) {
		alloc_lfile(RTD, -1);
		FILEPTR = (struct file *)NULL;
		process_node((KA_T)utp->uu_utnd.utnd_rdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Print information on the text file.
	 */

#if	DUV<30000
	    if (p->task)
		process_text((KA_T)p->task, (KA_T)p->utask);
#else	/* DUV>=30000 */
		process_text((KA_T)((char *)Pa[px] - sizeof(struct task)));
#endif	/* DUV<30000 */

	/*
	 * Save information on file descriptors.
	 */

#if	DUV<50000
	    for (i = j = 0; i <= utp->uu_file_state.uf_lastfile; i++) {
		if (i < NOFILE_IN_U) {
		    fp = utp->uu_file_state.uf_ofile[i];

# if	defined(HASFSTRUCT) && DUV>=40000
		    if (Fsv & FSV_FG)
			pv = (long)utp->uu_file_state.uf_pofile[i];
# endif	/* defined(HASFSTRUCT) && DUV>=40000 */

		} else {
		    if (!j) {
			b = (MALLOC_S)(utp->uu_file_state.uf_of_count
			  *	       sizeof(struct file *));
			if (b > nufb) {
			    if (!ufb)
				ufb = (struct file **)malloc(b);
			    else
				ufb = (struct file **)realloc((MALLOC_P *)ufb,
							      b);
			    if (!ufb) {
				(void) fprintf(stderr,
				    "%s: PID %d, no file * space\n",
				    Pn, Lp->pid);
				Exit(1);
			    }
			    nufb = b;
			}
			if (kread((KA_T)utp->uu_file_state.uf_ofile_of,
			    (char *)ufb, b))
				break;

# if	defined(HASFSTRUCT) && DUV>=40000
			if (Fsv & FSV_FG) {
			    b = (MALLOC_S)(utp->uu_file_state.uf_of_count
			      *		   sizeof(char));
			    if (b > pofb) {
				if (!pof)
				    pof = (char *)malloc(b);
				else
				    pof = (char *)realloc((MALLOC_P *)pof, b);
				if (!pof) {
				    (void) fprintf(stderr,
					"%s: PID %d: no file flags space\n",
					Pn, Lp->pid);
				    Exit(1);
				}
				pofb = b;
			    }
			    if (kread((KA_T)utp->uu_file_state.uf_pofile_of,
				      pof, b))
				zeromem(pof, b);
			}
# endif	/* defined(HASFSTRUCT) && DUV>=40000 */

		    }
		    fp = ufb[j];

# if	defined(HASFSTRUCT) && DUV>=40000
		    if (Fsv & FSV_FG)
			pv = pof[j];
# endif	/* defined(HASFSTRUCT) && DUV>=40000 */

		    j++;
		}
		if (fp && (ulong)fp != 0xffffffffffffffff) {
		    alloc_lfile(NULL, i);
		    process_file((KA_T)fp);
		    if (Lf->sf) {

# if	defined(HASFSTRUCT) && DUV>=40000
			if (Fsv & FSV_FG) {
			    if ((Lf->pof = pv))
				Lf->fsv |= FSV_FG;
			}
# endif	/* defined(HASFSTRUCT) && DUV>=40000 */

			link_lfile();
		    }
		}
	    }
#else	/* DUV>=50000 */
	    for (i = j = k = 0; i <= utp->uu_file_state.uf_lastfile; i++) {
		if (i < NOFILE_IN_U) {
		    if (!k) {
			l = i/U_FE_ALLOC_SIZE;
		        if (!(ka = (KA_T)utp->uu_file_state.uf_entry[l])) {
			    i += U_FE_ALLOC_SIZE - 1;
			    continue;
		        }
		    }
		} else {
		    if (!j) {
			ka = (KA_T)utp->uu_file_state.uf_of_entry;
			if (!ka || kread(ka, (char *)&ofb, sizeof(ofb)))
			    break;
			k = 0;
		    }
		    if (!k) {
			l = j/U_FE_OF_ALLOC_SIZE;
			if (!(ka = (KA_T)ofb[l])) {
			    j += U_FE_OF_ALLOC_SIZE;
			    i += U_FE_OF_ALLOC_SIZE - 1;
			    continue;
			}
			if (kread(ka, (char *)&ufeo, sizeof(ufeo)))
			    break;
		    }
		    fp = ufeo[k].ufe_ofile;

# if	defined(HASFSTRUCT) && DUV>=40000
		    if (Fsv & FSV_FG)
			pv = ufeo[k].ufe_oflags;
# endif	/* defined(HASFSTRUCT) && DUV>=40000 */

		    if (++k >= U_FE_OF_ALLOC_SIZE)
			k = 0;
		    j++;
		}
		if (!j) {
		    if (!k) {
			if (kread(ka, (char *)&ufe, sizeof(ufe)))
			    break;
		    }
		    fp = ufe[k].ufe_ofile;

# if	defined(HASFSTRUCT) && DUV>=40000
		    if (Fsv & FSV_FG)
			pv = ufe[k].ufe_oflags;
# endif	/* defined(HASFSTRUCT) && DUV>=40000 */

		    if (++k >= U_FE_ALLOC_SIZE)
			k = 0;
		}
		if (fp && (ulong)fp != 0xffffffffffffffff) {
		    alloc_lfile(NULL, i);
		    process_file((KA_T)fp);
		    if (Lf->sf) {

# if	defined(HASFSTRUCT) && DUV>=40000
			if (Fsv & FSV_FG) {
			    if ((Lf->pof = pv))
				Lf->fsv |= FSV_FG;
			}
# endif	/* defined(HASFSTRUCT) && DUV>=40000 */

			link_lfile();
		    }
		}
	    }
#endif	/* DUV>=50000 */

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
	dev_t dev;
	int rv;
	KA_T v;
/*
 * Check kernel version.
 */
	(void) ckkv("DU", LSOF_VSTR, (char *)NULL, (char *)NULL);
/*
 * Set name list file path.
 */

#if	DUV<40000
	if (!Nmlst) {
	    if (!(Nmlst = get_nlist_path(1))) {
		(void) fprintf(stderr, "%s: can't get kernel name list path\n",
		    Pn);
		Exit(1);
	    }
	}
#endif	/* DUV<40000 */

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
		Memory ? Memory : KMEM, sys_errlist[errno]);
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
 * Access kernel symbols.
 */
	(void) build_Nl(Drive_Nl);

#if	DUV>=40000
	if (!Nmlst)
	    rv = knlist(Nl);
	else
#endif	/* DUV>=40000 */
	
            rv = nlist(Nmlst, Nl);
	if (rv == -1) {
	    (void) fprintf(stderr,
		"%s: can't read kernel name list from %s: %s\n",
		Pn, Nmlst ? Nmlst : "knlist(3)", strerror(errno));
	    Exit(1);
	}

#if	DUV<30000
	if (get_Nl_value("proc", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Kp, sizeof(Kp))
	||  get_Nl_value("nproc", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Np, sizeof(Np)))
#else	/* DUV>=30000 */
	if (get_Nl_value("npid", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Np, sizeof(Np))
	||  get_Nl_value("pidt", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Pidtab, sizeof(Pidtab)))
#endif	/* DUV<30000 */

	{
	    (void) fprintf(stderr, "%s: can't read proc table info\n", Pn);
	    Exit(1);
	}
	if (get_Nl_value("vnmaxp", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&Vnmxp, sizeof(Vnmxp))) {
	    (void) fprintf(stderr, "%s: can't determine vnode length\n", Pn);
	    Exit(1);
	}
	if (get_Nl_value("cldev", Drive_Nl, &v) < 0 || !v
	||  kread(v, (char *)&dev, sizeof(dev))) {
	    if (!Fwarn)
		(void) fprintf(stderr, "%s: can't read clone device number\n",
		    Pn);
	    HaveCloneMaj = 0;
	} else {
	    CloneMaj = GET_MAJ_DEV(dev);
	    HaveCloneMaj = 1;
	}
}


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
	char *ba, buf[MAXPATHLEN+2], *ps;
	int len, rv;
/*
 * Get bootfile name.
 */
	len = 0;
	if ((rv = getsysinfo(GSI_BOOTEDFILE, &buf[1], sizeof(buf) - 1, &len,
			     (char *)NULL))
	!= 1)
	{
	    if (rv < 0) {
		(void) fprintf(stderr, "%s: can't get booted file name: %s\n",
		    Pn, strerror(errno));
		Exit(1);
	    }
	    return((char *)NULL);
	}
/*
 * Check for a non-NULL path.
 */
	buf[sizeof(buf) - 2] = '\0';
	len = strlen(&buf[1]);
	if (len < 1)
	    return((char *)NULL);
/*
 * If no path return is requested by the value of ap, return a NULL string
 * pointer.
 */
	if (!ap)
	    return("");
/*
 * Make sure the path has a leading '/'.
 */
	if (buf[1] != '/') {
	    buf[0] = '/';
	    ba = buf;
	    len++;
	} else
	   ba = &buf[1];
/*
 * Allocate permanent space for the path, copy it to the space, and return
 * a pointer to the space.
 */
	len++;
	if (!(ps = (char *)malloc(len))) {
	    (void) fprintf(stderr,
		"%s: can't allocate %d bytes for boot file path: %s\n",
		Pn, len, ba);
	    Exit(1);
	}
	(void) snpf(ps, len, "%s", ba);
	return(ps);
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

	if (lseek(Kd, addr, L_SET) == (off_t)-1L)
	    return(-1);
	br = read(Kd, buf, len);
	return((br == len) ? 0 : 1);
}


/*
 * process_text() - print text information
 */
static void

#if	DUV<30000
process_text(tp, utp)
#else	/* DUV>=30000 */
process_text(tp)
#endif	/* DUV<30000 */

	KA_T tp;			/* kernel task structure */

#if	DUV<30000
	KA_T utp;			/* user task structure address for
					 * the task */
#endif	/* DUV<30000 */

{
	int i;
	KA_T ka, kb;
	int n = 0;
	struct task t;
	struct vm_anon_object vmao;
	struct vm_map_entry vmme;
	struct vm_map vmm;
	struct vm_object vmo;
	struct vm_seg vms;

#if	DUV<40000
	struct vm_vp_object vpo;
#else	/* DUV>=40000 */
	struct vm_ubc_object vpo;
#endif	/* DUV<40000 */

/*
 * Read task structure from kernel.
 */
	if (kread(tp, (char *)&t, sizeof(t))

#if	DUV<30000
	||  (KA_T)t.u_address != utp
#endif	/* DUV<30000 */

	)
	    return;
/*
 * Print information about the text vnode referenced in the procfs
 * structure inside the task structure.
 */
	if (t.procfs.pr_exvp)
	    enter_vn_text((KA_T)t.procfs.pr_exvp, &n);
/*
 * Read the vm_map structure.  Search its vm_map_entry structure list.
 */
	if (!t.map
	||  kread((KA_T)t.map, (char *)&vmm, sizeof(vmm)))
	    return;
	if (!vmm.vm_is_mainmap)
	    return;

#if	defined(VM_SKIPLIST)
	for (i = 0, ka = (KA_T)vmm.vm_links.vml_sl_next[0];
	     i < vmm.vm_nentries && ka != (KA_T)t.map;
	     i++, ka = (KA_T)vmme.vme_links.vml_sl_next[0])
#else	/* !defined(VM_SKIPLIST) */
	for (i = 0, ka = (KA_T)vmm.vm_links.next;
	     i < vmm.vm_nentries && ka != (KA_T)t.map;
	     i++, ka = (KA_T)vmme.vme_links.next)
#endif	/* defined(VM_SKIPLIST) */

	{

	/*
	 * Read the next vm_map_entry structure and its object.
	 */
	    if (kread(ka, (char *)&vmme, sizeof(vmme)))
		return;
	    if (!(kb = (KA_T)vmme.vme_uobject.vm_object)
	    ||  kread(kb, (char *)&vmo, sizeof(vmo)))
		continue;
	/*
	 * Process by object type.
	 */
	    switch (vmo.ob_type) {
	    case OT_ANON:

	    /*
	     * If an anonymous object is backed by an OT_UBC or OT_VP object,
	     * read its vm_ubc_object or vm_vp_object to find a vnode pointer.
	     */
		if (kread(kb, (char *)&vmao, sizeof(vmao)))
		    break;
		if (!vmao.ao_bobject
		||  kread((KA_T)vmao.ao_bobject, (char *)&vmo, sizeof(vmo)))
		    break;

#if	DUV<40000
		if (vmo.ob_type != OT_VP
		||  kread((KA_T)vmao.ao_bobject, (char *)&vpo, sizeof(vpo)))
		    break;
		enter_vn_text((KA_T)vpo.vo_vp, &n);
#else	/* DUV>=40000 */
		if (vmo.ob_type != OT_UBC
		||  kread((KA_T)vmao.ao_bobject, (char *)&vpo, sizeof(vpo)))
		    break;
# if	DUV>=50000
		enter_vn_text(vpo2vp(&vpo), &n);
# else	/* DUV<50000 */
		enter_vn_text((KA_T)vpo.vu_vfp.vp, &n);
#endif	/* DUV>=50000 */
#endif	/* DUV<40000 */
		break;
	    /*
	     * If this is a segment object, read the segment map, and search
	     * for backing objects whose object type is OT_UBC or OT_VP.
	     */

	    case OT_SEG:
		for (kb=(KA_T)vmme.vme_seg; kb; kb=(KA_T)vms.seg_vnext) {
		    if (kread(kb, (char *)&vms, sizeof(vms)))
			break;
		    if (!vms.seg_vop
		    ||  kread((KA_T)vms.seg_vop, (char *)&vmo, sizeof(vmo)))
			continue;

#if	DUV<40000
		    if (vmo.ob_type != OT_VP)
#else	/* DUV>=40000 */
		    if (vmo.ob_type != OT_UBC)
#endif	/* DUV<40000 */

			continue;
		    if (kread((KA_T)vms.seg_vop, (char *)&vpo, sizeof(vpo)))
			break;

#if	DUV<40000
		    enter_vn_text((KA_T)vpo.vo_vp, &n);
#else	/* DUV>=40000 */
# if	DUV>=50000
		    enter_vn_text(vpo2vp(&vpo), &n);
# else	/* DUV<50000 */
		    enter_vn_text((KA_T)vpo.vu_vfp.vp, &n);
#endif	/* DUV<40000 */
#endif	/* DUV>=50000 */

		}
	    }
	}
}


/*
 * read_proc() - read process table entries
 */

static void
read_proc()
{
	static int ap = 0;
	MALLOC_S len;
	struct proc *p;
	KA_T pa;
	int px, try;

#if     DUV>=30000
	struct pid_entry pe;
#endif  /* DUV>=30000 */

	if (!Ps) {

	/*
	 * Allocate local proc table space.
	 */
	    if (Np < 1) {
		(void) fprintf(stderr, "%s: proc table has no entries\n", Pn);
		Exit(1);
	    }
	    len = (MALLOC_S)(PAPSINIT * sizeof(struct proc));
	    if (!(Ps = (struct proc *)malloc(len))) {
		(void) fprintf(stderr, "%s: no proc table space (%d bytes)\n",
		    Pn, len);
		Exit(1);
	    }

#if	DUV>=30000
	/*
	 * Allocate kernel proc address table space.
	 */
	    len = (MALLOC_S)(PAPSINIT * sizeof(KA_T));
	    if (!(Pa = (KA_T *)malloc(len))) {
		(void) fprintf(stderr,
		    "%s: no proc address table space (%d bytes)\n", Pn, len);
		Exit(1);
	    }
#endif	/* DUV>=30000 */

	    ap = PAPSINIT;
	}
/*
 * Try to read the proc structure table PROCTRYLM times.
 * The operation must yield PROCMIN structures.
 */
	for (try = 0; try < PROCTRYLM; try++) {
	    for (p = Ps, Psn = px = 0; px < Np; px++) {

	    /*
	     * Insure Ps and Psa space.
	     */
		if (Psn >= ap) {
		    ap += PAPSINCR;
		    len = (MALLOC_S)(ap * sizeof(struct proc));
		    if (!(Ps = (struct proc *)realloc((MALLOC_P *)Ps, len))) {
			(void) fprintf(stderr,
			    "%s: no more proc table space (%d bytes)\n",
			    Pn, len);
			Exit(1);
		    }
		    p = &Ps[Psn];

#if	DUV>=30000
		    len = (MALLOC_S)(ap * sizeof(KA_T));
		    if (!(Pa = (KA_T *)realloc((MALLOC_P *)Pa, len))) {
			(void) fprintf(stderr,
			    "%s: no more proc address table space (%d bytes)\n",
			    Pn, len);
			Exit(1);
		    }
#endif	/* DUV>=30000 */

		}

#if     DUV<30000
		pa = Kp + (KA_T)(px * sizeof(struct proc));
		if (kread(pa, (char *)p, sizeof(struct proc)))
		    continue;
#else   /* DUV>=30000 */
		pa = Pidtab + (KA_T)(px * sizeof(struct pid_entry));
		if (kread(pa, (char *)&pe, sizeof(struct pid_entry)))
		    continue;
		if ((pa = (KA_T)pe.pe_proc) == NULL
		||  kread(pa, (char *)p, sizeof(struct proc)))
		    continue;
		if (pe.pe_pid != p->p_pid)
		    continue;
		Pa[Psn] = pa;
#endif  /* DUV<30000 */

		Psn++;
		p++;
	    }
	/*
	 * Check the results of the scan.
	 */
	    if (Psn >= PROCMIN)
		break;
	}
/*
 * Quit if not enough proc structures could be accumulated.
 */
	if (try >= PROCTRYLM) {
	    (void) fprintf(stderr, "%s: can't read proc table\n", Pn);
	    Exit(1);
	}
	if (Psn < Np && !RptTm) {

	/*
	 * Reduce the local proc structure tables to their minimum if
	 * not in repeat mode.
	 */
	    ap = Psn;
	    len = (MALLOC_S)(Psn * sizeof(struct proc));
	    if (!(Ps = (struct proc *)realloc((MALLOC_P *)Ps, len))) {
		(void) fprintf(stderr,
		    "%s: can't reduce proc table to %d bytes\n",
		    Pn, len);
		Exit(1);
	    }

#if	DUV>=30000
	    len = (MALLOC_S)(Psn * sizeof(KA_T));
	    if (!(Pa = (KA_T *)realloc((MALLOC_P *)Pa, len))) {
		(void) fprintf(stderr,
		    "%s: can't reduce proc address table to %d bytes\n",
		    Pn, len);
		Exit(1);
	    }
#endif	/* DUV>=30000 */

	}
}


#if	DUV>=50000
/*
 * vfp2vp() -- convert VM object's vu_vfp to a vnode pointer
 */

static KA_T
vpo2vp(vpo)
	struct vm_ubc_object *vpo;	/* pointer to local vm_ubc_object */
{
	struct advfsbfs {		/* This structure is referenced in
					 * vm_ubc.h (as msfsbfs), but never
					 * defined in a distributed header
					 * file, so we make a hack definition
					 * here. */
	    unsigned long d1[18];	/* dummies */
	    struct vnode *vp;		/* vnode */
	} bfa;
	static int ft = 1;
	KA_T ka;
	static KA_T ops = (KA_T)0;
/*
 * If this is the first time, get the msfs (AdvFS) UBC operation switch
 * address.
 */
	if (ft) {
	    ft = 0;

#if	defined(ADVFSV)
	    if (get_Nl_value("msfsubc", Drive_Nl, &ops) < 0)
#endif	/* defined(ADVFSV) */

		ops = (KA_T)0;

	}
	ka = (KA_T)vpo->vu_vfp.vp;
	if (!ops || ((KA_T)vpo->vu_ops != ops))
	    return(ka);
	if (!ka || kread(ka, (char *)&bfa, sizeof(bfa)))
	    return(ka);
	return((KA_T)bfa.vp);
} 
#endif	/* DUV>=50000 */


#if	DUV>=50100 && defined(HASNCACHE)
/*
 * Kernel name cache functions and associate definiitions for Tru64 UNIX
 * 5.1 and above.
 */


/*
 * Definitions
 */


/*
 * Structures
 */

struct l_nch {
	struct namecache *nc;		/* namecache entry */
	struct l_nch *nxt;		/* next hashed entry */
};


/*
 * Static variables
 */

static int Hmsk = 0;			/* Nchash[] mask -- (size - 1), where
					 * size is a power of two */
static int Nc;				/* number of cached namecache structs */
static struct l_nch **Nchash = (struct l_nch **)NULL;
					/* hash pointers buckets */
static int Ncfirst = 1;			/* first-call status */

/*
 * Definitions
 */

#define ncachehash(i)		(((int)(i*31415)>>3)&Hmsk)


/*
 * Prototypes
 */

_PROTOTYPE(static struct l_nch *ncache_addr,(unsigned long id));
_PROTOTYPE(static int ncache_ckrootid,(KA_T na, unsigned long id));
_PROTOTYPE(static int ncache_isroot,(KA_T na, char *cp));


/*
 * ncache_addr() -- look up a node's local ncache address
 */

static struct l_nch *
ncache_addr(id)
	unsigned long id;		/* node's capability ID */
{
	register struct l_nch *hp;

	for (hp = Nchash[ncachehash(id)]; hp; hp = hp->nxt) {
	    if ((hp->nc)->nc_vpid == id)
		return(hp);
	}
	return((struct l_nch *)NULL);
}


/*
 * ncache_ckrootid() - check for a root node ID
 */

static int
ncache_ckrootid(na, id)
	KA_T na;			/* vnode address */
	unsigned long id;		/* root ID to check */
{

#if	defined(ADVFSV)
	struct advfsmount {		/* This structure should be defined in
					 * a distributed header file, but it
					 * isn't, so we make a hack definition
					 * here. */
	    u_long d1[10];		/* dummies */
	    struct vnode *am_rootvp;	/* root vnode pointer */
	} am;
	static KA_T aops = (KA_T)0;
#endif	/* defined(ADVFSV) */

	struct cdfsmount cm;
	static KA_T cops = (KA_T)0;
	struct dvdfsmount dm;
	static KA_T dops = (KA_T)0;
	static KA_T fops = (KA_T)0;
	static KA_T frvp = (KA_T)0;
	static int ft = 1;
	register int i;
	static unsigned long *ic = (unsigned long *)NULL;
	MALLOC_S len;
	struct mount m;
	static int nia = 0;
	static int niu = 0;
	struct mntinfo nm;
	static KA_T nops = (KA_T)0;
	static KA_T n3ops = (KA_T)0;
	KA_T rv;
	struct ufsmount um;
	static KA_T uops = (KA_T)0;
	struct vnode v;
/*
 * Check the cache.
 */
	if (ic && niu) {
	    for (i = 0; i < niu; i++) {
		if (id == ic[i])
		    return(1);
	    }
	}
/*
 * Read the vnode and the associated mount structure.
 */
	if (!na || kread(na, (char *)&v, sizeof(v)))
	    return(0);
	if (!v.v_mount || kread((KA_T)v.v_mount, (char *)&m, sizeof(m)))
	    return(0);
/*
 * If this is the first time this function has been used, get the necessary
 * kernel addresses.
 */
	if (ft) {
	    ft = 0;

#if	defined(ADVFSV)
	    if (get_Nl_value("advfsvfs", (struct drive_Nl *)NULL, &aops) < 0)
		aops = (KA_T)0;
#endif	/* defined(ADVFSV) */

	    if (get_Nl_value("cdfsvfs", (struct drive_Nl *)NULL, &cops) < 0)
		cops = (KA_T)0;
	    if (get_Nl_value("dvdfsvfs", (struct drive_Nl *)NULL, &dops) < 0)
		dops = (KA_T)0;
	    if (get_Nl_value("fdfsvfs", (struct drive_Nl *)NULL, &fops) < 0)
		fops = (KA_T)0;
	    if (get_Nl_value("fsfsrvp", (struct drive_Nl *)NULL, &frvp) < 0)
		frvp = (KA_T)0;
	    if (get_Nl_value("nfsvfs", (struct drive_Nl *)NULL, &nops) < 0)
		nops = (KA_T)0;
	    if (get_Nl_value("nfs3vfs", (struct drive_Nl *)NULL, &n3ops) < 0)
		n3ops = (KA_T)0;
	    if (get_Nl_value("ufsvfs", (struct drive_Nl *)NULL, &uops) < 0)
		uops = (KA_T)0;
	}
/*
 * See if we know how to find the root vnode pointer for this file system
 * type.
 */

#if	defined(ADVFSV)
	if (aops && (aops == (KA_T)m.m_op)) {

	/*
	 * Set AdvFS (MSFS) root vnode address.
	 */
	    if (!m.m_info || kread((KA_T)m.m_info, (char *)&am, sizeof(am)))
		return(0);
	    rv = (KA_T)am.am_rootvp;
	} else
#endif	/* defined(ADVFSV) */

	if (cops && (cops == (KA_T)m.m_op)) {

	/*
	 * Set CDFS root vnode address.
	 */
	    if (!m.m_info || kread((KA_T)m.m_info, (char *)&cm, sizeof(cm)))
		return(0);
	    rv = (KA_T)cm.um_rootvp;
	} else if (dops && (dops == (KA_T)m.m_op)) {

	/*
	 * Set DVDFS root vnode address.
	 */
	    if (!m.m_info || kread((KA_T)m.m_info, (char *)&dm, sizeof(dm)))
		return(0);
	    rv = (KA_T)dm.dm_rootvp;
	} else if (fops && (fops == (KA_T)m.m_op)) {

	/*
	 * Set FDFS root vnode address.
	 */
	    rv = frvp;
	} else if ((nops && (nops == (KA_T)m.m_op))
	       ||  (n3ops && (n3ops == (KA_T)m.m_op)))
	{

	/*
	 * Set NFS[3] root vnode address.
	 */
	    if (!m.m_info || kread((KA_T)m.m_info, (char *)&nm, sizeof(nm)))
		return(0);
	    rv = (KA_T)nm.mi_rootvp;
	} else if (uops && (uops == (KA_T)m.m_op)) {

	/*
	 * Set UFS root vnode address.
	 */
	    if (!m.m_info || kread((KA_T)m.m_info, (char *)&um, sizeof(um)))
		return(0);
	    rv = (KA_T)um.um_rootvp;
	} else
	    return(0);
/*
 * Read the root vnode.
 */
	if (!rv || kread(rv, (char *)&v, sizeof(v)))
	    return(0);
	if (id != v.v_id)
	    return(0);
/*
 * A new root vnode has been located.  Cache it.
 */
	if (niu >= nia) {
	    if (!nia) {
		len = (MALLOC_S)(10 * sizeof(unsigned long));
		ic = (unsigned long *)malloc(len);
	    } else {
		len = (MALLOC_S)((nia + 10) * sizeof(unsigned long));
		ic = (unsigned long *)realloc((MALLOC_P *)ic, len);
	    }
	    if (!ic) {
		(void) fprintf(stderr,
		    "%s: no space for root node VPID table\n", Pn);
		Exit(1);
	    }
	    nia += 10;
	}
	ic[niu++] = id;
	return(1);
}


/*
 * ncache_isroot() - is head of name cache path a file system root?
 */

static int
ncache_isroot(na, cp)
	KA_T na;				/* vnode address */
	char *cp;				/* partial path */
{
	char buf[MAXPATHLEN];
	int i;
	MALLOC_S len;
	struct mounts *mtp;
	static KA_T *nc = (KA_T *)NULL;
	static int nca = 0;
	static int ncn = 0;
	struct stat sb;
	struct vnode v;

	if (!na)
	    return(0);
/*
 * Search the root capability node address cache.
 */
	for (i = 0; i < ncn; i++) {
	    if (na == nc[i])
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
	if (kread((KA_T)na, (char *)&v, sizeof(v))
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
 * Add the capability ID to the root capability ID cache.
 */
	if (ncn >= nca) {
	    if (!nca) {
		len = (MALLOC_S)(10 * sizeof(KA_T));
		nc = (KA_T *)malloc(len);
	    } else {
		len = (MALLOC_S)((nca + 10) * sizeof(KA_T));
		nc = (KA_T *)realloc((MALLOC_P *)nc, len);
	    }
	    if (!nc) {
		(void) fprintf(stderr,
		    "%s: no space for root node address table\n", Pn);
		Exit(1);
	    }
	    nca += 10;
	}
	nc[ncn++] = na;
	return(1);
}


/*
 * ncache_load() - load the kernel's name cache
 */

void
ncache_load()
{
	register int h, i, n;
	KA_T ka, ncp;
	int len;
	register struct l_nch *lp;
	struct l_nch *lpnxt;
	static struct namecache *nc = (struct namecache *)NULL;
	static int ncl = 0;
	static int nchsz = 0;
	static int ncpc = 0;
	static int ncpus = 0;
	register struct namecache *np;
	static KA_T *pp = (KA_T *)NULL;

	if (!Fncache)
	    return;
	if (Ncfirst) {

	/*
	 * Do startup (first-time) functions.
	 */
	    Ncfirst = 0;
	/*
	 * Get CPU count.
	 */
	    ka = (KA_T)0;
	    if (get_Nl_value("ncpus", (struct drive_Nl *)NULL, &ka) < 0
	    ||  !ka
	    ||  kread(ka, (char *)&ncpus, sizeof(ncpus)))
	    {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: can't read processor count: %s\n",
			Pn, print_kptr(ka, (char *)NULL, 0));
		ncl = nchsz = ncpc = ncpus = 0;
		return;
	    }
	    if (ncpus < 1) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: processor count: %d\n", Pn, ncpus);
		ncl = nchsz = ncpc = ncpus = 0;
		return;
	    }
	/*
	 * Get the per-processor table address.
	 */
	    ka = (KA_T)0;
	    if (get_Nl_value("procptr", (struct drive_Nl *)NULL, &ka) < 0
	    ||  !ka
	    ||  kread(ka, (char *)&ka, sizeof(ka))
	    ||  !ka)
	    {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: per processor table address: %s\n",
			Pn, print_kptr(ka, (char *)NULL, 0));
		ncl = nchsz = ncpc = ncpus = 0;
		return;
	    }
	/*
	 * Allocate space for the processor structure addresses and read them.
	 */
	    len = (int)(ncpus * sizeof(KA_T));
	    if (!(pp = (KA_T *)malloc((MALLOC_S)len))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d bytes for processor addresses\n",
			Pn, len);
		Exit(1);
	    }
	    if (kread(ka, (char *)pp, len)) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: can't read processor addresses: %s\n",
			Pn, print_kptr(ka, (char *)NULL, 0));
		ncl = nchsz = ncpc = ncpus = 0;
		return;
	    }
	    for (i = 0; i < ncpus; i++) {
		if (pp[i])
		    pp[i] = (KA_T)((char *)pp[i]
			  +	   offsetof(struct processor, namecache));
	    }
	/*
	 * Get the per-processor nchash size.
	 */
	    ka = (KA_T)0;
	    if (get_Nl_value("nchsz", (struct drive_Nl *)NULL, &ka) < 0
	    ||  !ka
	    ||  kread((KA_T)ka, (char *)&nchsz, sizeof(nchsz)))
	    {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: processor nchash count address: %s\n",
			Pn, print_kptr(ka, (char *)NULL, 0));
		ncl = nchsz = ncpc = ncpus = 0;
		return;
	    }
	    if (nchsz < 1) {
		if (!Fwarn)
		    (void) fprintf(stderr,
			"%s: WARNING: bad per processor nchash count: %d\n",
			Pn, nchsz);
		nchsz = ncpus = 1;
		return;
	    }
	/*
	 * Allocate space for nchsz * NCHSIZE * ncpus namecache structures.
	 */
	    ncpc = (int)(nchsz * NCHSIZE);
	    ncl = (int)(ncpc * sizeof(struct namecache));
	    len = (int)(ncl * ncpus);
	    if (!(nc = (struct namecache *)malloc((MALLOC_S)len))) {
		(void) fprintf(stderr,
		    "%s: no space for %d namecache entries (%d bytes)\n",
		    Pn, ncpc * ncpus, len);
		Exit(1);
	    }
	} else {

	/*
	 * Do setup for repeat calls.
	 */
	    if (Nchash) {
		for (i = 0; i <= Hmsk; i++) {
		    for (lp = Nchash[i]; lp; lp = lpnxt) {
			lpnxt = lp->nxt;
			(void) free((MALLOC_P *)lp);
		    }
		}
		(void) free((MALLOC_P *)Nchash);
		Nchash = (struct l_nch **)NULL;
		Nc = 0;
	    }
	}
/*
 * Loop through the processors, reading the processor structure pointer
 * for the processor, then its name cache.  Build a local name cache
 * table of struct namecache entries for all processors.
 */
	for (i = n = 0; i < ncpus; i++) {
	    if (!pp[i])
		continue;
	    if (kread(pp[i], (char *)&ncp, sizeof(ncp)) || !ncp)
		continue;
	    if (kread(ncp, (char *)&nc[n], ncl))
		continue;
	    n += ncpc;
	}
/*
 * Compute a hash table size and allocate it.
 */
	if (!n)
	    return;
	for (i = 1; i < n; i <<= 1)
	    ;
	i += i;
	Hmsk = i - 1;
	if (!(Nchash = (struct l_nch **)calloc(i, sizeof(struct l_nch *)))) {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: no space for %d byte name cache hash buckets\n",
		    Pn, (int)(i * sizeof(struct l_nch *)));
	    Exit(1);
	}
/*
 * Assign hash pointers to the accumulated namecache entries.
 */
	for (i = Nc = 0; i < n; i++) {
	    if (!nc[i].nc_vpid)
		continue;
	    if (((len = nc[i].nc_nlen) < 1) || (len > NCHNAMLEN))
		continue;
	    if (len < 3 && nc[i].nc_name[0] == '.') {
		if ((len == 1) || ((len == 2) && (nc[i].nc_name[1] == '.')))
		    continue;
	    }
	    h = ncachehash(nc[i].nc_vpid);
	/*
	 * Look for an existing hash entry.  Choose among duplicates the one
	 * with the largest nc_dvpid.
	 */
	    for (lp = Nchash[h]; lp; lp = lp->nxt) {
		if ((np = lp->nc) && (np->nc_vpid == nc[i].nc_vpid)) {
		    if (nc[i].nc_dvpid > np->nc_dvpid)
			lp->nc = &nc[i];
		    break;
		}
	    }
	    if (lp)
		continue;
	/*
	 * Allocate and fill a new local name cache entry.
	 */
	    if (!(lp = (struct l_nch *)malloc(sizeof(struct l_nch)))) {
		(void) fprintf(stderr, "%s: can't allocate l_nch entry\n", Pn);
		Exit(1);
	    }
	    lp->nc = &nc[i];
	    lp->nxt = Nchash[h];
	    Nchash[h] = lp;
	    Nc++;
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
	struct namecache *nc;
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
	if (Nc == 0 || !(lc = ncache_addr(Lf->id)) || !(nc = lc->nc)) {

	/*
	 * If the node has no cache entry, see if it's the mount
	 * point of a known file system.
	 */
	    if (!Lf->fsdir || !Lf->dev_def || Lf->inp_ty != 1)
		return((char *)NULL);
	    for (mtp = readmnt(); mtp; mtp = mtp->next) {
		if (!mtp->dir || !mtp->inode)
		    continue;
		if ((Lf->dev == mtp->dev)
		&&  (mtp->inode == Lf->inode)
		&&  (strcmp(mtp->dir, Lf->fsdir) == 0))
		    return(cp);
	    }
	    return((char *)NULL);
	}
/*
 * Start the path assembly.
 */
	if ((nl = nc->nc_nlen) > (blen - 1))
	    return((char *)NULL);
	cp = buf + blen - nl - 1;
	rlen = blen - nl - 1;
	(void) strncpy(cp, nc->nc_name, nl);
	cp[nl] = '\0';
/*
 * Look up the name cache entries that are parents of the node address.
 * Quit when:
 *
 *	there's no parent;
 *	the name length is too large to fit in the receiving buffer.
 */
	for (;;) {
	    if (!nc->nc_dvpid) {
		if (ncache_isroot((KA_T)nc->nc_vp, cp))
		    *fp = 1;
		break;
	    }
	    if (!(lc = ncache_addr(nc->nc_dvpid))) {
		if (ncache_ckrootid((KA_T)nc->nc_vp, nc->nc_dvpid))
		    *fp = 1;
		break;
	    }
	    if (!(nc = lc->nc))
		break;
	    if (((nl = nc->nc_nlen) + 1) > rlen)
		break;
	    *(cp - 1) = '/';
	    cp--;
	    rlen--;
	    (void) strncpy(cp - nl, nc->nc_name, nl);
	    cp -= nl;
	    rlen -= nl;
	}
	return(cp);
}
#endif	/* DUV>=50100 && defined(HASNCACHE) */
