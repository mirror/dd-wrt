/*
 * dproc.c - Darwin process access functions for /dev/kmem-based lsof
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
static char *rcsid = "$Id: dproc.c,v 1.8 2005/11/01 20:24:51 abe Exp $";
#endif

#include "lsof.h"

#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/message.h>
#include <mach/vm_map.h>


/*
 * Local definitions
 */

#define	NPHASH	1024				/* Phash bucket count --
						 * MUST BE A POWER OF 2!!! */
#define PHASH(a)	(((int)((a * 31415) >> 3)) & (NPHASH - 1))
#define PINCRSZ		256			/* Proc[] size inrement */


/*
 * Local structures
 */

struct phash {
    KA_T ka;					/* kernel proc struct address */
    struct proc *la;				/* local proc struct address */
    struct phash *next;				/* next phash entry */
};


/*
 * Local function prototypes
 */

_PROTOTYPE(static pid_t get_parent_pid,(KA_T kpa));
_PROTOTYPE(static int read_procs,());
_PROTOTYPE(static void process_map,(pid_t pid));
_PROTOTYPE(static void enter_vn_text,(KA_T va, int *n));

#if	DARWINV>=700
_PROTOTYPE(static char *getcmdnm,(pid_t pid));
#endif	/* DARWINV>=700 */

_PROTOTYPE(static void get_kernel_access,(void));


/*
 * Local static values
 */

static KA_T Akp = (KA_T)NULL;		/* kernel allproc chain address */
static int Np = 0;			/* PA[] and Proc[] entry count */
static int Npa = 0;			/* Proc[] structure allocation count */
static MALLOC_S Nv = 0;			/* allocated Vp[] entries */
static KA_T *Pa = (KA_T *)NULL;		/* Proc[] addresses */
struct phash **Phash = (struct phash **)NULL;
					/* kernel proc address hash pointers */
static struct proc *Proc = (struct proc *)NULL;
					/* local copy of prc struct chain */
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
	char *cmd;
	struct filedesc fd;
	int i, nf;
	MALLOC_S nb;
	static struct file **ofb = NULL;
	static int ofbb = 0;
	struct proc *p;
	int pgid;
	int ppid = 0;
	static char *pof = (char *)NULL;
	static int pofb = 0;
	short pss, sf;
	int px;
	uid_t uid;

#if	DARWINV<800
	struct pcred pc;
#else	/* DARWINV>=800 */
	struct ucred uc;
#endif	/* DARWINV<800 */

/*
 * Read the process table.
 */
	if (read_procs()) {
	    (void) fprintf(stderr, "%s: can't read process table\n", Pn);
	    Exit(1);
	}
/*
 * Examine proc structures and their associated information.
 */
	for (p = Proc, px = 0; px < Np; p++, px++)
	{

#if	DARWINV<800
	    if (!p->p_cred || kread((KA_T)p->p_cred, (char *)&pc, sizeof(pc)))
		continue;
	    pgid = pc.p_rgid;
	    uid = pc.p_ruid;
#else	/* DARWINV>=800 */
	    if (!p->p_ucred || kread((KA_T)p->p_ucred, (char *)&uc, sizeof(uc)))
		continue;
	    pgid = uc.cr_rgid;
	    uid = uc.cr_uid;
#endif	/* DARWINV<800 */

#if	defined(HASPPID)
	    ppid = get_parent_pid((KA_T)p->p_pptr);
#endif	/* defined(HASPPID) */

	/*
	 * Get the command name.
	 */

#if	DARWINV<700
	    cmd = p->P_COMM;
#else	/* DARWINV>=700 */
	   if (!strcmp(p->p_comm, "LaunchCFMApp")) {
		if (!(cmd = getcmdnm(p->p_pid)))
		    cmd = p->p_comm;
	   } else
		cmd = p->p_comm;
#endif	/* DARWINV<700 */

	/*
	 * See if process is excluded.
	 *
	 * Read file structure pointers.
	 */
	    if (is_proc_excl(p->p_pid, pgid, (UID_ARG)uid, &pss, &sf))
		continue;
	    if (!p->p_fd ||  kread((KA_T)p->p_fd, (char *)&fd, sizeof(fd)))
		continue;
	    if (!fd.fd_refcnt || fd.fd_lastfile > fd.fd_nfiles)
		continue;
	/*
	 * Allocate a local process structure.
	 *
	 * Set kernel's proc structure address.
	 */
	    if (is_cmd_excl(cmd, &pss, &sf))
		continue;
	    alloc_lproc(p->p_pid, pgid, ppid, (UID_ARG)uid, cmd, (int)pss,
			(int)sf);
	    Plf = (struct lfile *)NULL;
	    Kpa = Pa[px];
	/*
	 * Save current working directory information.
	 */
	    if (fd.fd_cdir) {
		alloc_lfile(CWD, -1);
		Cfp = (struct file *)NULL;
		process_node((KA_T)fd.fd_cdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (fd.fd_rdir) {
		alloc_lfile(RTD, -1);
		Cfp = (struct file *)NULL;
		process_node((KA_T)fd.fd_rdir);
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Process the VM map.
	 */
	    process_map(p->p_pid);
	/*
	 * Read open file structure pointers.
	 */
	    if (!fd.fd_ofiles || (nf = fd.fd_nfiles) <= 0)
		continue;
	    nb = (MALLOC_S)(sizeof(struct file *) * nf);
	    if (nb > ofbb) {
		if (!ofb)
		    ofb = (struct file **)malloc(nb);
		else
		    ofb = (struct file **)realloc((MALLOC_P *)ofb, nb);
		if (!ofb) {
		    (void) fprintf(stderr, "%s: PID %d, no file * space\n",
			Pn, p->p_pid);
		    Exit(1);
		}
		ofbb = nb;
	    }
	    if (kread((KA_T)fd.fd_ofiles, (char *)ofb, nb))
		continue;

	    nb = (MALLOC_S)(sizeof(char) * nf);
	    if (nb > pofb) {
		if (!pof)
		    pof = (char *)malloc(nb);
		else
		    pof = (char *)realloc((MALLOC_P *)pof, nb);
		if (!pof) {
		    (void) fprintf(stderr, "%s: PID %d, no file flag space\n",
			Pn, p->p_pid);
		    Exit(1);
		}
		pofb = nb;
	    }
	    if (!fd.fd_ofileflags || kread((KA_T)fd.fd_ofileflags, pof, nb))
		zeromem(pof, nb);

	/*
	 * Save information on file descriptors.
	 */
	    for (i = 0; i < nf; i++) {
		if (ofb[i] && !(pof[i] & UF_RESERVED)) {
		    alloc_lfile(NULL, i);
		    process_file((KA_T)(Cfp = ofb[i]));
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


#if	DARWINV>=700
static char *
getcmdnm(pid)
	pid_t pid;			/* process ID */
{
	static int am;
	static char *ap = (char *)NULL;
	char *cp, *ep, *sp;
	int mib[3];
	size_t sz;

	if (!ap) {

	/*
	 * Allocate space for the maximum argument size.
	 */
	    mib[0] = CTL_KERN;
	    mib[1] = KERN_ARGMAX;
	    sz = sizeof(am);
	    if (sysctl(mib, 2, &am, &sz, NULL, 0) == -1) {
		(void) fprintf(stderr, "%s: can't get arg max, PID %d\n",
		    Pn, pid);
		Exit(1);
	    }
	    if (!(ap = (char *)malloc((MALLOC_S)am))) {
		(void) fprintf(stderr, "%s: no arg ptr (%d) space, PID %d\n",
		    Pn, am, pid);
		Exit(1);
	    }
	}
/*
 * Get the arguments for the process.
 */
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROCARGS;
	mib[2] = pid;
	sz = (size_t)am;
	if (sysctl(mib, 3, ap, &sz, NULL, 0) == -1)
	    return((char *)NULL);
/*
 * Skip to the first NUL character, which should end the saved exec path.
 */
	for (cp = ap; *cp && (cp < (ap + sz)); cp++) {
	    ;
	}
	if (cp >= (ap + sz))
	    return((char *)NULL);
/*
 * Skip trailing NULs, which should find the beginning of the command.
 */
	while (!*cp && (cp < (ap + sz))) {
	    cp++;
	}
	if (cp >= (ap + sz))
	    return((char *)NULL);
/*
 * Make sure that the command is NUL-terminated.
 */
	for (sp = cp; *cp && (cp < (ap + sz)); cp++) {
	    ;
	}
	if (cp >= (ap + sz))
	    return((char *)NULL);
	ep = cp;
/*
 * Locate the start of the command's base name and return it.
 */
	for (ep = cp, cp--; cp >= sp; cp--) {
	    if (*cp == '/') {
		return(cp + 1);
	    }
	}
	return(sp);
}
#endif	/* DARWINV>=700 */


/*
 * get_kernel_access() - get access to kernel memory
 */

static void
get_kernel_access()
{

/*
 * Check kernel version.
 */
	(void) ckkv("Darwin", LSOF_VSTR, (char *)NULL, (char *)NULL);
/*
 * Set name list file path.
 */
	if (!Nmlst)
	    Nmlst = N_UNIX;

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
	if ((Kd = open(Memory ? Memory : KMEM, O_RDONLY, 0)) < 0)
	{
	    (void) fprintf(stderr, "%s: open(%s): %s\n", Pn,
	        Memory ? Memory : KMEM,
		strerror(errno));
	    Exit(1);
	}
	(void) build_Nl(Drive_Nl);
	if (nlist(Nmlst, Nl) < 0) {
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


/*
 * get_parent_pid() - get parent process PID
 */

static pid_t
get_parent_pid(kpa)
	KA_T kpa;			/* kernel parent process address */
{
	struct phash *ph;

	if (kpa) {
	    for (ph = Phash[PHASH(kpa)]; ph; ph = ph->next) {
		if (ph->ka == kpa)
		    return((pid_t)ph->la->p_pid);
	    }
	}
	return((pid_t)0);
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

	if ((off_t)addr & (off_t)0x3) {

	/*
	 * No read is possible if the address is not aligned on a word
	 * boundary.
	 */
	    return(1);
	}
	if (lseek(Kd, (off_t)addr, SEEK_SET) == (off_t)-1)
	    return(1);
	br = read(Kd, buf, len);
	return((br == len) ? 0 : 1);
}


/*
 * prcess_map() - process VM map
 */

static void
process_map(pid)
	pid_t pid;			/* process id */
{
	vm_address_t address = 0;
	mach_msg_type_number_t count;
	vm_region_extended_info_data_t e_info;
	int n = 0;
	mach_port_t object_name;
	vm_size_t size = 0;
	vm_map_t task;
	vm_region_top_info_data_t t_info;

	struct vm_object {		/* should come from <vm/vm_object.h> */

#if	DARWINV<800
	    KA_T		Dummy1[15];
#else	/* DARWINV>=800 */
	    KA_T		Dummy1[14];
#endif	/* DARWINV>=800 */

	    memory_object_t	pager;
	} vmo;

	struct vnode_pager {		/* from <osfmk/vm/bsd_vm.c> */
	    KA_T		Dummy1[4];
	    struct vnode	*vnode;
	} vp;

/*
 * Get the task port associated with the process
 */
	if (task_for_pid((mach_port_name_t)mach_task_self(), pid,
			 (mach_port_name_t *)&task)
	!= KERN_SUCCESS) {
	    return;
	}
/*
 * Go through the task's address space, looking for blocks of memory
 * backed by an external pager (i.e, a "vnode")
 */
	for (address = 0;; address += size) {
	    count = VM_REGION_EXTENDED_INFO_COUNT;
	    if (vm_region(task, &address, &size, VM_REGION_EXTENDED_INFO,
			  (vm_region_info_t)&e_info, &count, &object_name)
	    != KERN_SUCCESS) {
		break;
	    }
	    if (!e_info.external_pager)
		continue;
	    count = VM_REGION_TOP_INFO_COUNT;
	    if (vm_region(task, &address, &size, VM_REGION_TOP_INFO,
			  (vm_region_info_t)&t_info, &count, &object_name)
	    != KERN_SUCCESS) {
		break;
	    }
	/*
	 * The returned "obj_id" is the "vm_object_t" address.
	 */
	    if (!t_info.obj_id)
		continue;
	    if (kread(t_info.obj_id, (char *)&vmo, sizeof(vmo)))
		break;
	/*
	 * If the "pager" is backed by a vnode then the "vm_object_t"
	 * "memory_object_t" address is actually a "struct vnode_pager *".
	 */
	    if (!vmo.pager)
		continue;
	    if (kread((KA_T)vmo.pager, (char *)&vp, sizeof(vp)))
		break;
	    (void) enter_vn_text((KA_T)vp.vnode, &n);
	}
	return;
}


/*
 * read_procs() - read proc structures
 */

static int
read_procs()
{
	int h, i, np, pe;
	KA_T kp, kpn;
	MALLOC_S msz;
	struct proc *p;
	struct phash *ph, *phn;
 
	if (!Akp) {

	/*
	 * Get kernel allproc structure pointer once.
	 */
	    if (get_Nl_value("aproc", Drive_Nl, &Akp) < 0 || !Akp) {
		(void) fprintf(stderr, "%s: can't get proc table address\n",
		    Pn);
		Exit(1);
	    }
	}
/*
 * Get the current number of processes and calculate PA and Proc[] allocation
 * sizes large enough to handle it.
 */
	if (get_Nl_value("nproc", Drive_Nl, &kp) < 0 || !kp) {
	    (void) fprintf(stderr, "%s: can't get nproc address\n", Pn);
	    Exit(1);
	}
	if (kread(kp, (char *)&np, sizeof(np))) {
	    (void) fprintf(stderr, "%s: can't read process count from %s\n",
		Pn, print_kptr(kp, (char *)NULL, 0));
	    Exit(1);
	}
	for (np += np, pe = PINCRSZ; pe < np; pe += PINCRSZ)
	    ;
/*
 * Allocate or reallocate the Pa[] and Proc[] tables.
 */
	msz = (MALLOC_S)(pe * sizeof(struct proc));
	if (!Proc)
	    Proc = (struct proc *)malloc(msz);
	else if (pe > Npa)
	    Proc = (struct proc *)realloc((MALLOC_P *)Proc, msz);
	if (!Proc) {
	    (void) fprintf(stderr, "%s: no space for proc table\n", Pn);
	    Exit(1);
	}
	msz = (MALLOC_S)(pe * sizeof(KA_T));
	if (!Pa)
	    Pa = (KA_T *)malloc(msz);
	else if (pe > Npa)
	    Pa = (KA_T *)realloc((MALLOC_P *)Pa, msz);
	if (!Pa) {
	    (void) fprintf(stderr, "%s: no space for proc addr table\n", Pn);
	    Exit(1);
	}
	Npa = pe;
/*
 * Allocate or reset the Phash[] table.
 */
	if (!Phash) {
	    Phash = (struct phash **)calloc(NPHASH, sizeof(struct phash *));
	} else {
	    for (h = 0; h < NPHASH; h++) {
		for (ph = Phash[h]; ph; ph = phn) {
		    phn = ph->next;
		    (void) free((MALLOC_P *)ph);
		}
		Phash[h] = (struct phash *)NULL;
	    }
	}
	if (!Phash) {
	    (void) fprintf(stderr, "%s: no space for proc address hash\n", Pn);
	    Exit(1);
	}
/*
 * Read the proc structures on the kernel's chain.
 */
	for (i = Np = 0, kp = Akp, p = Proc, pe += pe;
	     kp && i < pe;
	     i++, kp = kpn)
	{
	    if (kread(kp, (char *)p, sizeof(struct proc)))
		break;
	    kpn = (KA_T)(((KA_T)p->p_list.le_next == Akp) ? NULL
							  : p->p_list.le_next);
	    if (p->p_stat == 0 || p->p_stat == SZOMB)
		continue;
	/*
	 * Cache the proc structure's addresses.
	 */
	    h = PHASH(kp);
	    if (!(ph = (struct phash *)malloc((MALLOC_S)sizeof(struct phash))))
	    {
		(void) fprintf(stderr, "%s: no space for phash struct\n", Pn);
		Exit(1);
	    }
	    ph->ka = kp;
	    ph->la = p;
	    ph->next = Phash[h];
	    Phash[h] = ph;
	    p++;
	    Pa[Np++] = kp;
	    if (Np >= Npa) {

	    /*
	     * Enlarge Pa[] and Proc[].
	     */
		msz = (int)((Npa + PINCRSZ) * sizeof(struct proc));
		if (!(Proc = (struct proc *)realloc((MALLOC_P *)Proc, msz))) {
		    (void) fprintf(stderr, "%s: no additional proc space\n",
			Pn);
		    Exit(1);
		}
		msz = (int)((Npa + PINCRSZ) * sizeof(KA_T));
		if (!(Pa = (KA_T *)realloc((MALLOC_P *)Pa, msz))) {
		    (void) fprintf(stderr,
			"%s: no additional proc addr space\n", Pn);
		    Exit(1);
		}
		Npa += PINCRSZ;
	    }
	}
/*
 * If too many processes were read, the chain following probably failed;
 * report that and exit.
 */
	if (i >= pe) {
	    (void) fprintf(stderr, "%s: can't follow kernel proc chain\n", Pn);
	    Exit(1);
	}
/*
 * If not in repeat mode, reduce Pa[] and Proc[] to their minimums.
 */
	if (Np < Npa && !RptTm) {
	    msz = (MALLOC_S)(Np * sizeof(struct proc));
	    if (!(Proc = (struct proc *)realloc((MALLOC_P *)Proc, msz))) {
		(void) fprintf(stderr, "%s: can't reduce proc table\n", Pn);
		Exit(1);
	    }
	    msz = (MALLOC_S)(Np * sizeof(KA_T));
	    if (!(Pa = (KA_T *)realloc((MALLOC_P *)Pa, msz))) {
		(void) fprintf(stderr, "%s: can't reduce proc addr table\n",
		    Pn);
		Exit(1);
	    }
	    Npa = Np;
	}
/*
 * Return 0 if any processes were loaded; 1 if none were.
 */
	return((Np > 0) ? 0 : 1);
}
