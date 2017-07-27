/*
 * dfile.c - Darwin file processing functions for /dev/kmem-based lsof
 */


/*
 * Copyright 2005 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 2005 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id$";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#if	DARWINV>=800
#define	file		fileglob
#define	f_flag		fg_flag
#define	f_type		fg_type
#define	f_count		fg_count
#define	f_ops		fg_ops
#define	f_offset	fg_offset
#define	f_data		fg_data
#endif	/* DARWINV>=800 */

#if	defined(HASPSXSEM)
#define	PSEMNAMLEN	31		/* from kern/posix_sem.c */
#endif	/* defined(HASPSXSEM) */

#if	defined(HASPSXSHM)
#define	PSHMNAMLEN	31		/* from kern/posix_shm.c */
#endif	/* defined(HASPSXSHM) */


/*
 * Local structure definitions
 */

#if	defined(HASPSXSEM)
struct pseminfo {			/* from kern/posix_sem.c */
    unsigned int psem_flags;
    unsigned int psem_usecount;
    mode_t psem_mode;
    uid_t psem_uid;
    gid_t psem_gid;
    char psem_name[PSEMNAMLEN + 1];
    void *psem_semobject;
    struct proc *sem_proc;
};

struct psemnode {
    struct pseminfo *pinfo;
};
#endif	/* defined(HASPSXSEM) */

#if	defined(HASPSXSHM)		/* from kern/posix_shm.c */
struct pshminfo {
    unsigned int pshm_flags;
    unsigned int pshm_usecount;
    off_t pshm_length;
    mode_t pshm_mode;
    uid_t pshm_uid;
    gid_t pshm_gid;       
    char pshm_name[PSHMNAMLEN + 1];
    void *pshm_memobject;
};

struct pshmnode {
    off_t mapp_addr;

# if	DARWINV<800
    size_t map_size;
# else	/* DARWINV>=800 */
    user_size_t map_size;
# endif	/* DARWINV>=800 */

    struct pshminfo *pinfo;
};
#endif	/* defined(HASPSXSHM) */


#if	DARWINV>=800
/*
 * print_v_path() - print vnode's path
 */

int
print_v_path(lf)
	struct lfile *lf;
{
	if (lf->V_path) {
	    safestrprt(lf->V_path, stdout, 0);
	    return(1);
	}
	return(0);
}
#endif	/* DARWINV>=800 */


#if	defined(HASKQUEUE)
/*
 * process_kqueue() -- process kqueue file
 */

void
process_kqueue(ka)
	KA_T ka;			/* kqueue file structure address */
{
	struct kqueue kq;		/* kqueue structure */

	(void) snpf(Lf->type, sizeof(Lf->type), "KQUEUE");
	enter_dev_ch(print_kptr(ka, (char *)NULL, 0));
	if (!ka || kread(ka, (char *)&kq, sizeof(kq)))
	    return;
	(void) snpf(Namech, Namechl, "count=%d, state=%#x", kq.kq_count,
	    kq.kq_state);
	enter_nm(Namech);
}
#endif	/* defined(HASKQUEUE) */


#if	DARWINV>=800
/*
 * process_pipe() - process a file structure whose type is DTYPE_PIPE
 */

void
process_pipe(pa)
	KA_T pa;			/* pipe structure address */
{
	(void) snpf(Lf->type, sizeof(Lf->type), "PIPE");
	enter_dev_ch(print_kptr(pa, (char *)NULL, 0));
	Namech[0] = '\0';
}
#endif	/* DARWINV>=800 */


#if	defined(HASPSXSEM)
/*
 * process_psxsem() -- process POSIX semaphore file
 */

void
process_psxsem(pa)
	KA_T pa;			/* psxsem file structure address */
{
	struct pseminfo pi;
 	struct psemnode pn;

	(void) snpf(Lf->type, sizeof(Lf->type), "PSXSEM");
	enter_dev_ch(print_kptr(pa, (char *)NULL, 0));
	if (!Fsize)
	    Lf->off_def = 1;
	if (pa && !kread(pa, (char *)&pn, sizeof(pn))) {
	    if (pn.pinfo && !kread((KA_T)pn.pinfo, (char *)&pi, sizeof(pi))) {
		if (pi.psem_name[0]) {
		    pi.psem_name[PSEMNAMLEN] = '\0';
		    (void) snpf(Namech, Namechl, "%s", pi.psem_name);
		    enter_nm(Namech);
		}
	    }
	}
}
#endif	/* defined(HASPSXSEM) */


#if	defined(HASPSXSHM)
/*
 * process_psxshm() -- process POSIX shared memory file
 */

void
process_psxshm(pa)
	KA_T pa;			/* psxshm file structure address */
{
	struct pshminfo pi;
	struct pshmnode pn;
	int pns = 0;

	(void) snpf(Lf->type, sizeof(Lf->type), "PSXSHM");
	enter_dev_ch(print_kptr(pa, (char *)NULL, 0));
	if (pa && !kread(pa, (char *)&pn, sizeof(pn))) {
	    pns = 1;
	    if (pn.pinfo && !kread((KA_T)pn.pinfo, (char *)&pi, sizeof(pi))) {
		if (pi.pshm_name[0]) {
		    pi.pshm_name[PSEMNAMLEN] = '\0';
		    (void) snpf(Namech, Namechl, "%s", pi.pshm_name);
		    enter_nm(Namech);
		} else if (pi.pshm_memobject) {
		    (void) snpf(Namech, Namechl, "obj=%s",
			print_kptr((KA_T)pi.pshm_memobject, (char *)NULL, 0));
		    enter_nm(Namech);
		}
	    }
	}
	if (Foffset)
	    Lf->off_def = 1;
	else if (pns) {
	    Lf->sz = (SZOFFTYPE)pn.map_size;
	    Lf->sz_def = 1;
	}
}
#endif	/* defined(HASPSXSHM) */


/*
 * process_file() - process file
 */

/*
 * The caller may define:
 *
 *	FILEPTR	as the name of the location to store a pointer
 *			to the current file struct -- e.g.,
 *
 *			struct file *foobar;
 *			#define FILEPTR	foobar
 */

void
process_file(fp)
	KA_T fp;			/* kernel file structure address */
{

#if	DARWINV<800
	struct file f;
#else	/* DARWINV>=800 */
	struct fileglob f;
	struct fileproc fileproc;
#endif	/* DARWINV>=800 */

	int flag;

#if	defined(FILEPTR)
/*
 * Save file structure address for process_node().
 */
	FILEPTR = &f;
#endif	/* defined(FILEPTR) */

/*
 * Read file structure.
 */

#if	DARWINV<800
	if (kread((KA_T)fp, (char *)&f, sizeof(f))) {
	    (void) snpf(Namech, Namechl, "can't read file struct from %s",
		print_kptr(fp, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
#else	/* DARWINV>=800 */
	if (kread((KA_T)fp, (char *)&fileproc, sizeof(fileproc))) {
	    (void) snpf(Namech, Namechl, "can't read fileproc struct from %s",
		print_kptr(fp, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
	if (kread((KA_T)fileproc.f_fglob, (char *)&f, sizeof(f))) {
	    (void) snpf(Namech, Namechl, "can't read fileglob struct from %s",
		print_kptr((KA_T)fileproc.f_fglob, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
#endif	/* DARWINV>=800 */

	Lf->off = (SZOFFTYPE)f.f_offset;
	if (f.f_count) {

	/*
	 * Construct access code.
	 */
	    if ((flag = (f.f_flag & (FREAD | FWRITE))) == FREAD)
		Lf->access = 'r';
	    else if (flag == FWRITE)
		Lf->access = 'w';
	    else if (flag == (FREAD | FWRITE))
		Lf->access = 'u';

#if	defined(HASFSTRUCT)
	/*
	 * Save file structure values.
	 */
	    if (Fsv & FSV_CT) {
		Lf->fct = (long)f.f_count;
		Lf->fsv |= FSV_CT;
	    }
	    if (Fsv & FSV_FA) {
		Lf->fsa = fp;
		Lf->fsv |= FSV_FA;
	    }
	    if (Fsv & FSV_FG) {
		Lf->ffg = (long)f.f_flag;
		Lf->fsv |= FSV_FG;
	    }
	    if (Fsv & FSV_NI) {
		Lf->fna = (KA_T)f.f_data;
		Lf->fsv |= FSV_NI;
	    }
#endif	/* defined(HASFSTRUCT) */

	/*
	 * Process structure by its type.
	 */
	    switch (f.f_type) {


#if	defined(DTYPE_PIPE)
	    case DTYPE_PIPE:
# if	defined(HASPIPEFN)
		if (!Selinet)
		    HASPIPEFN((KA_T)f.f_data);
# endif	/* defined(HASPIPEFN) */
		return;
#endif	/* defined(DTYPE_PIPE) */

	    case DTYPE_VNODE:
		if (!Selinet)
		    process_node((KA_T)f.f_data);
		return;
	    case DTYPE_SOCKET:
		process_socket((KA_T)f.f_data);
		return;

#if	defined(HASKQUEUE)
	    case DTYPE_KQUEUE:
		process_kqueue((KA_T)f.f_data);
		return;
#endif	/* defined(HASKQUEUE) */

#if	defined(HASPSXSEM)
	    case DTYPE_PSXSEM:
		process_psxsem((KA_T)f.f_data);
		return;
#endif	/* defined(HASPSXSEM) */

#if	defined(HASPSXSHM)
	    case DTYPE_PSXSHM:
		process_psxshm((KA_T)f.f_data);
		return;
#endif	/* defined(HASPSXSHM) */

#if	defined(HASPRIVFILETYPE)
	    case PRIVFILETYPE:
		HASPRIVFILETYPE((KA_T)f.f_data);
		return;
#endif	/* defined(HASPRIVFILETYPE) */

	    default:
		if (f.f_type || f.f_ops) {
		    (void) snpf(Namech, Namechl,
			"%s file struct, ty=%#x, op=%p",
			print_kptr(fp, (char *)NULL, 0), f.f_type, f.f_ops);
		    enter_nm(Namech);
		    return;
		}
	    }
	}
	enter_nm("no more information");
}
