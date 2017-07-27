/*
 * dproc.c -- pstat-based HP-UX process access functions for lsof
 */


/*
 * Copyright 1999 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1999 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#define	FDS_ALLOC_INCR	256		/* fds[] allocation increment */
#define	FDS_ALLOC_INIT	64		/* initial fds[] allocation */
#define	FINFOINCR	128		/* pst_fileinfo2 table allocation
					 * increment */
#define INCLMEM(s, m)	((size_t)(offsetof(struct s, m) \
			+	  sizeof(((struct s *)0)->m)))
					/* size of struct s, including
					 * member m */
#define	PSTATINCR	512		/* pst_status table allocation
					 * increment */
#define	TXTVMINCR	64		/* text and vm info table table
					 * allocation increment */
#define	VMREGINCR	64		/* VM region table table allocation
					 * increment */


/*
 * Local structures
 */

struct pstatck {
	size_t moff;			/* offset of size member in pst_static
					 * -- from offsetof(...member) */
	size_t msz;			/* structure's pst_static member
					 * inclusion size -- from INCLMEM(s, m)
					 * macro */
	size_t ssz;			/* structure size -- from
					 * sizeof(struct) */
	char *sn;			/* structure name */
} PstatCk[] = {
	{ (size_t)offsetof(struct pst_static, pst_status_size),
	  (size_t)INCLMEM(pst_static, pst_status_size),
	  sizeof(struct pst_status),
	  "pst_status" },
	{ (size_t)offsetof(struct pst_static, pst_vminfo_size),
	  (size_t)INCLMEM(pst_static, pst_vminfo_size),
	  sizeof(struct pst_vminfo),
	  "pst_vminfo" },
	{ (size_t)offsetof(struct pst_static, pst_filedetails_size),
	  (size_t)INCLMEM(pst_static, pst_filedetails_size),
	  sizeof(struct pst_filedetails),
	  "pst_filedetails" },
	{ (size_t)offsetof(struct pst_static, pst_socket_size),
	  (size_t)INCLMEM(pst_static, pst_socket_size),
	  sizeof(struct pst_socket),
	  "pst_socket" },
	{ (size_t)offsetof(struct pst_static, pst_stream_size),
	  (size_t)INCLMEM(pst_static, pst_stream_size),
	  sizeof(struct pst_stream),
	  "pst_stream" },
	{ (size_t)offsetof(struct pst_static, pst_mpathnode_size),
	  (size_t)INCLMEM(pst_static, pst_mpathnode_size),
	  sizeof(struct pst_mpathnode),
	  "pst_mpathnode" },
	{ (size_t)offsetof(struct pst_static, pst_fileinfo2_size),
	  (size_t)INCLMEM(pst_static, pst_fileinfo2_size),
	  sizeof(struct pst_fileinfo2),
	  "pst_fileinfo2" },
};
#define	NPSTATCK	(sizeof(PstatCk) /sizeof(struct pstatck))


/*
 * Local static variables
 */

static int HvRtPsfid = -1;		/* "/" psfileid status:
					 *     -1: not yet tested;
					 *	0: tested and unknown;
					 *	1: tested and known */
static struct psfileid RtPsfid;		/* "/" psfileid */


/*
 * Local function prototypes
 */

_PROTOTYPE(static void get_kernel_access,(void));
_PROTOTYPE(static void process_text,(struct pst_status *p));
_PROTOTYPE(static struct pst_fileinfo2 *read_files,(struct pst_status *p,
						    int *n));
_PROTOTYPE(static struct pst_status *read_proc,(int *n));
_PROTOTYPE(static struct pst_vm_status *read_vmreg,(struct pst_status *p,
						    int *n));


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
	int cwds, fd, *fds, fdsa, i, j, l, nf, np, rtds;
	struct pst_fileinfo2 *f;
	long flag;
	KA_T ka, na;
	MALLOC_S nb;
	struct pst_status *p;
	struct pst_filedetails pd;
	struct pst_socket *s;
	short pss, sf;
/*
 * Compute current working and root directory statuses and the statuses of
 * the first FDS_ALLOC_INIT FDs.
 */
	if (Fand && Fdl) {
	    cwds = (ck_fd_status(CWD, -1) != 2) ? 0 : 1;
	    rtds = (ck_fd_status(RTD, -1) != 2) ? 0 : 1;
	    nb = (MALLOC_S)(sizeof(int) * FDS_ALLOC_INIT);
	    if (!(fds = (int *)malloc(nb))) {
		(void) fprintf(stderr,
		    "%s: can't allocate %d FD status entries\n", Pn,
		    FDS_ALLOC_INIT);
		Exit(1);
	    }
	    for (fdsa = 0; fdsa < FDS_ALLOC_INIT; fdsa++) {
		if (Fand && Fdl)
		    fds[fdsa] = (ck_fd_status(NULL, fdsa) == 2) ? 1 : 0;
		else
		    fds[fdsa] = 1;
	    }
	} else {
	    cwds = rtds = 1;
	    fdsa = 0;
	    fds = (int *)NULL;
	}
/*
 * If only socket files have been selected, or socket files have been selected
 * ANDed with other selection options, enable the skipping of regular files.
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
	     * If ORed file selection options have been specified, or no ORed
	     * process selection options have been specified, enable
	     * unconditional file checking and clear socket file only checking.
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
 * Examine proc structures and their associated information.
 */
	for (i = 0, p = read_proc(&np); i < np; i++, p++) {
	    if (!p->pst_stat || p->pst_stat == PS_ZOMBIE)
		continue;
	    if (is_proc_excl((int)p->pst_pid, (int)p->pst_pgrp,
			     (UID_ARG)p->pst_uid, &pss, &sf))
		continue;
	/*
	 * Make sure the command name is NUL-terminated.
	 */
	    p->pst_ucomm[PST_UCOMMLEN - 1] = '\0';
	    if (is_cmd_excl(p->pst_ucomm, &pss, &sf))
		continue;
	    if (cckreg) {

	    /*
	     * If conditional checking of regular files is enabled, enable
	     * socket file only checking, based on the process' selection
	     * status.
	     */
		ckscko = (sf & SELPROC) ? 0 : 1;
	    }
	    alloc_lproc((int)p->pst_pid, (int)p->pst_pgrp, (int)p->pst_ppid,
			(UID_ARG)p->pst_uid, p->pst_ucomm, (int)pss, (int)sf);
	    Plf = (struct lfile *)NULL;
	/*
	 * Save current working directory information.
	 */
	    if (!ckscko && cwds
	    &&  IS_PSFILEID(&p->pst_cdir) && (p->pst_cdir.psf_fileid > 0)
	    ) {
		alloc_lfile(CWD, -1);
		if ((na = read_det(&p->pst_fid_cdir, p->pst_hi_fileid_cdir,
				   p->pst_lo_fileid_cdir,
				   p->pst_hi_nodeid_cdir,
				   p->pst_lo_nodeid_cdir, &pd)))
		    (void) process_finfo(&pd, &p->pst_fid_cdir,
					 &p->pst_cdir, na);
		else {
		    (void) snpf(Namech, Namechl,
			"can't read %s pst_filedetails%s%s", CWD,
			errno ? ": " : "", errno ? strerror(errno) : "");
		    enter_nm(Namech);
		}
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Save root directory information.
	 */
	    if (!ckscko && rtds
	    &&  IS_PSFILEID(&p->pst_rdir) && (p->pst_rdir.psf_fileid > 0)
	    ) {
		if (HvRtPsfid < 0)
		    (void) scanmnttab();
	 	if (!HvRtPsfid
		||  memcmp((void *)&RtPsfid, (void *)&p->pst_rdir,
			       sizeof(RtPsfid)))
		{
		    alloc_lfile(RTD, -1);
		    if ((na = read_det(&p->pst_fid_rdir,
				       p->pst_hi_fileid_rdir,
				       p->pst_lo_fileid_rdir,
				       p->pst_hi_nodeid_rdir,
				       p->pst_lo_nodeid_rdir, &pd)))
			(void) process_finfo(&pd, &p->pst_fid_rdir,
					     &p->pst_rdir, na);
		    else {
			(void) snpf(Namech, Namechl,
			    "can't read %s pst_filedetails%s%s", RTD,
			    errno ? ": " : "",
			    errno ? strerror(errno) : "");
			enter_nm(Namech);
		    }
		    if (Lf->sf)
			link_lfile();
		}
	    }
	/*
	 * Print information on the text files.
	 */
	    if (!ckscko)
		(void) process_text(p);
	/*
	 * Loop through user's files.
	 */
	    for (j = 0, f = read_files(p, &nf); j < nf; j++, f++) {
		fd = (int)f->psf_fd;
	    /*
	     * Check FD status and allocate local file space, as required.
	     */
		if (Fand && Fdl && fds) {

		/*
		 * Check and update the FD status array.
		 */
		    if (fd >= fdsa) {
			for (l = fdsa; l <= fd; l += FDS_ALLOC_INCR)
			    ;
			nb = (MALLOC_S)(l * sizeof(int));
			if (!(fds = (int *)realloc((MALLOC_P *)fds, nb))) {
			    (void) fprintf(stderr,
				"%s: can't reallocate %d FD status entries\n",
				Pn, l);
			    Exit(1);
			}
			while (fdsa < l) {
			    fds[fdsa] = (ck_fd_status(NULL, fdsa) == 2) ? 1 : 0;
			    fdsa++;
			}
		    }
		    if (!fds[fd])
			continue;
		}
		alloc_lfile(NULL, (int)f->psf_fd);
	    /*
	     * Construct access code.
	     */
		if ((flag = (long)(f->psf_flag & ~PS_FEXCLOS))
		== (long)PS_FRDONLY)
		    Lf->access = 'r';
		else if (flag == (long)PS_FWRONLY)
		    Lf->access = 'w';
		else
		    Lf->access = 'u';

#if	defined(HASFSTRUCT)
	    /*
	     * Save file structure values.
	     */
		if (Fsv & FSV_CT) {
		    Lf->fct = (long)f->psf_count;
		    Lf->fsv |= FSV_CT;
		}
		if (Fsv & FSV_FA) {
		    ka = (((KA_T)(f->psf_hi_fileid & 0xffffffff) << 32)
		       |  (KA_T)(f->psf_lo_fileid & 0xffffffff));
		    if ((Lf->fsa = ka))
			Lf->fsv |= FSV_FA;
		}
		if (Fsv & FSV_FG) {
		    Lf->ffg = flag;
		    Lf->fsv |= FSV_FG;
		}
		Lf->pof = (long)(f->psf_flag & PS_FEXCLOS);
#endif	/* defined(HASFSTRUCT) */

	    /*
	     * Save file offset.  _PSTAT64 should alwaus be defined, but just
	     * to be safe, check for it.
	     */

#if	defined(_PSTAT64)
		Lf->off = (SZOFFTYPE)f->_PSF_OFFSET64;
#else	/* !defined(_PSTAT64) */
		Lf->off = (SZOFFTYPE)f->psf_offset;
#endif	/* defined(_PSTAT64) */

	    /*
	     * Process the file by its type.
	     */
		switch (f->psf_ftype) {
		case PS_TYPE_VNODE:
		    if (ckscko || Selinet)
			break;
		    if ((na = read_det(&f->psf_fid, f->psf_hi_fileid,
				       f->psf_lo_fileid, f->psf_hi_nodeid,
				       f->psf_lo_nodeid, &pd)))
			(void) process_finfo(&pd, &f->psf_fid, &f->psf_id, na);
		    else {
			(void) snpf(Namech, Namechl,
			    "can't read pst_filedetails%s%s",
			    errno ? ": " : "",
			    errno ? strerror(errno) : "");
			enter_nm(Namech);
		    }
		    break;
		case PS_TYPE_SOCKET:
		    switch (f->psf_subtype) {
		    case PS_SUBTYPE_SOCK:
			(void) process_socket(f, (struct pst_socket *)NULL);
			break;
		    case PS_SUBTYPE_SOCKSTR:
			if ((s = read_sock(f)))
			    (void) process_socket(f, s);
			else
			    (void) process_stream(f, (int)ckscko);
			break;
		    default:
			(void) snpf(Namech, Namechl,
			    "unknown socket sub-type: %d", (int)f->psf_subtype);
			enter_nm(Namech);
		    }
		    break;
		case PS_TYPE_STREAMS:
		    (void) process_stream(f, (int)ckscko);
		    break;
		case PS_TYPE_UNKNOWN:
		    (void) snpf(Lf->type, sizeof(Lf->type), "UNKN");
		    (void) enter_nm("no more information");
		    break;
		case PS_TYPE_UNSP:
		    (void) snpf(Lf->type, sizeof(Lf->type), "UNSP");
		    (void) enter_nm("no more information");
		    break;
		case PS_TYPE_LLA:
		    (void) snpf(Lf->type, sizeof(Lf->type), "LLA");
		    (void) enter_nm("no more information");
		    break;
		}
		if (Lf->sf)
		    link_lfile();
	    }
	/*
	 * Examine results.
	 */
	    if (examine_lproc())
		return;
	}
}


/*
 * get_kernel_access() -- access the required information in the kernel
 */

static void
get_kernel_access()
{
	int err = 0;
	int i;
	struct pst_static pst;
	_T_LONG_T *szp;
/*
 * Check the kernel version.
 */
	(void) ckkv("HP-UX", LSOF_VSTR, (char *)NULL, (char *)NULL);
/*
 * Check PSTAT support.  First make sure we can read pst_static up through
 * its pst_static_size member.  If not, quit.  If we can, read the full
 * pst_static structure.
 */
	if (pstat_getstatic(&pst, (size_t)INCLMEM(pst_static, pst_static_size),
			    1, 0) != 1)
	{
	    (void) fprintf(stderr,
		"%s: FATAL: can't determine PSTAT static size: %s\n",
		Pn, strerror(errno));
	    Exit(1);
	}
	if (pstat_getstatic(&pst, (size_t)pst.pst_static_size, 1, 0) != 1) {
	    (void) fprintf(stderr,
		"%s: FATAL: can't read %ld bytes of pst_static\n",
		Pn, (long)pst.pst_static_size);
	    Exit(1);
	}
/*
 * Check all the pst_static members defined in PstatCk[].
 */
	for (i = 0; i < NPSTATCK; i++) {
	    if (pst.pst_static_size < PstatCk[i].msz) {
		(void) fprintf(stderr,
		    "%s: FATAL: pst_static doesn't contain %s_size\n",
		    Pn, PstatCk[i].sn);
		err = 1;
		continue;
	    }
	    szp = (_T_LONG_T *)(((char *)&pst) + PstatCk[i].moff);
	    if (*szp < PstatCk[i].ssz) {
		(void) fprintf(stderr,
		    "%s: FATAL: %s_size should be: %llu; is %llu\n",
		    Pn, PstatCk[i].sn, (unsigned long long)PstatCk[i].ssz,
		    (unsigned long long)*szp);
		err = 1;
	    }
	}
/*
 * Save the clone major device number, if pst_static is big enough to hold it.
 */
	if (pst.pst_static_size >= (size_t)INCLMEM(pst_static, clonemajor)) {
	    CloneMaj = pst.clonemajor;
	    HaveCloneMaj = 1;
	}
	if (!err)
	    return;
	Exit(1);
}


/*
 * initialize() -- perform all initialization
 */

void
initialize()
{
	get_kernel_access();
}


/*
 * process_text() -- process text access information
 */

static void
process_text(p)
	struct pst_status *p;		/* pst_status for process */
{
	int i, j, nr, ntvu;
	int meme = 0;
	static int mems = -1;
	KA_T na;
	MALLOC_S nb;
	static int ntva;
	struct pst_vm_status *rp;
	static int txts = -1;
	struct txtvm {
	    char *fd;
	    struct pst_fid opfid;
	    struct psfileid psfid;
	    KA_T na;
	    struct pst_filedetails pd;
	};
	static struct txtvm *tv = (struct txtvm *)NULL;
/*
 * Get and remember "mem" and "txt" FD statuses.
 */
	if (mems < 0) {
	    if (Fand && Fdl)
		mems = (ck_fd_status("mem", -1) == 2) ? 1 : 0;
	    else
		mems = 1;
	}
	if (txts < 0) {
	    if (Fand && Fdl)
		txts = (ck_fd_status("txt", -1) == 2) ? 1 : 0;
	    else
		txts = 1;
	}
	if (!mems && !txts)
	    return;
/*
 * Pre-allocate sufficient tv[] space for text file.
 */
	if (!tv) {
	    ntva = TXTVMINCR;
	    nb = (MALLOC_S)(ntva * sizeof(struct txtvm));
	    if (!(tv = (struct txtvm *)malloc(nb))) {

no_txtvm_space:

		(void) fprintf(stderr,
		    "%s: no memory for text and VM info array; PID: %d\n",
		    Pn, (int)p->pst_pid);
		Exit(1);
	    }
	}
/*
 * Enter text file in tv[], if possible.
 */
	if (txts && IS_PSFILEID(&p->pst_text) && (p->pst_text.psf_fileid > 0))
	{
	    if ((na = read_det(&p->pst_fid_text, p->pst_hi_fileid_text,
			       p->pst_lo_fileid_text, p->pst_hi_nodeid_text,
			       p->pst_lo_nodeid_text, &tv[0].pd)))
	    {
		tv[0].fd = "txt";
		tv[0].na = na;
		tv[0].opfid = p->pst_fid_text;
		tv[0].psfid = p->pst_text;
		ntvu = 1;
	    } else {
		alloc_lfile("txt", -1);
		(void) snpf(Namech, Namechl,
		    "can't read txt pst_filedetails%s%s",
		    errno ? ": " : "", errno ? strerror(errno) : "");
		enter_nm(Namech);
		if (Lf->sf)
		    link_lfile();
		ntvu = 0;
	    }
	} else
	    ntvu = 0;
/*
 * Get unique VM regions.
 */
	if (mems) {
	    for (i = 0, rp = read_vmreg(p, &nr); (i < nr); i++, rp++) {

	    /*
	     * Skip duplicate regions.
	     */
		for (j = 0; j < ntvu; j++) {
		    if (memcmp((void *)&rp->pst_id, (void *)&tv[j].psfid,
			       sizeof(struct psfileid))
		    == 0)
			break;
		}
		if (j < ntvu)
		    continue;
	    /*
	     * Make sure there's tv[] space for this region.
	     */
		if (ntvu >= ntva) {
		    ntva += TXTVMINCR;
		    nb = (MALLOC_S)(ntva * sizeof(struct txtvm));
		    if (!(tv = (struct txtvm *)realloc((MALLOC_P *)tv, nb)))
			goto no_txtvm_space;
		}
	    /*
	     * See if we can read the file details for this region.
	     */
		if ((na = read_det(&rp->pst_fid, rp->pst_hi_fileid,
				   rp->pst_lo_fileid, rp->pst_hi_nodeid,
				   rp->pst_lo_nodeid, &tv[ntvu].pd)))
		{
		    tv[ntvu].fd = "mem";
		    tv[ntvu].na = na;
		    tv[ntvu].opfid = rp->pst_fid;
		    tv[ntvu].psfid = rp->pst_id;
		    ntvu++;
		} else if (!meme) {
		    alloc_lfile("mem", -1);
		    (void) snpf(Namech, Namechl,
			"can't read mem pst_filedetails%s%s",
			errno ? ": " : "", errno ? strerror(errno) : "");
		    enter_nm(Namech);
		    if (Lf->sf)
			link_lfile();
		    meme = 1;
		}
	    }
	}
/*
 * Process information for unique regions.
 */
	for (i = 0; i < ntvu; i++) {
	    alloc_lfile(tv[i].fd, -1);
	    (void) process_finfo(&tv[i].pd, &tv[i].opfid, &tv[i].psfid,
				 tv[i].na);
	    if (Lf->sf)
		link_lfile();
	}
}
 

/*
 * read_det() -- read the pst_filedetails structure
 */

KA_T
read_det(ki, hf, lf, hn, ln, pd)
	struct pst_fid *ki;		/* kernel file ID */
	uint32_t hf;			/* high file ID bits */
	uint32_t lf;			/* low file ID bits */
	uint32_t hn;			/* high node ID bits */
	uint32_t ln;			/* low node ID bits */
	struct pst_filedetails *pd;	/* details receiver */
{
	KA_T na;

	errno = 0;
	na = (KA_T)(((KA_T)(hn & 0xffffffff) << 32) | (KA_T)(ln & 0xffffffff));
	if (pstat_getfiledetails(pd, sizeof(struct pst_filedetails), ki) <= 0
	||  hf != pd->psfd_hi_fileid || lf != pd->psfd_lo_fileid
	||  hn != pd->psfd_hi_nodeid || ln != pd->psfd_lo_nodeid)
	    return((KA_T)0);
	return(na);
}


/*
 * read_files() -- read the file descriptor information for a process
 */

static struct pst_fileinfo2 *
read_files(p, n)
	struct pst_status *p;		/* pst_status for the process */
	int *n;				/* returned fi[] entry count */
{
	size_t ec;
	static struct pst_fileinfo2 *fi = (struct pst_fileinfo2 *)NULL;
	MALLOC_S nb;
	int nf = 0;
	static int nfa = 0;
	int rc;
	static size_t sz = sizeof(struct pst_fileinfo2);
/*
 * Read the pst_fileinfo2 information for all files of the process
 * into fi[].
 */
	do {
	    if (nf >= nfa) {

	    /*
	     * Increase the size of fi[].
	     */
		nfa += FINFOINCR;
		nb = (MALLOC_S)(nfa * sizeof(struct pst_fileinfo2));
		if (!fi)
		    fi = (struct pst_fileinfo2 *)malloc(nb);
		else
		    fi = (struct pst_fileinfo2 *)realloc((MALLOC_P *)fi, nb);
		if (!fi) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for pst_filinfo\n",
			Pn, nb);
		    Exit(1);
		}
	    }
	/*
	 * Read the next block of pst_fileinfo2 structures.
	 */
	    ec = (size_t)(nfa - nf);
	    if ((rc = pstat_getfile2(fi + nf, sz, ec, nf, p->pst_pid)) > 0) {
		nf += rc;
		if (rc < (int)ec)
		    rc = 0;
	    }
	} while (rc > 0);
	*n = nf;
	return(fi);
}


/*
 * read_proc() -- read process table status information
 */

static struct pst_status *
read_proc(n)
	int *n;				/* returned ps[] entry count */
{
	size_t el;
	int i = 0;
	MALLOC_S nb;
	int np = 0;
	static int npa = 0;
	static struct pst_status *ps = (struct pst_status *)NULL;
	int rc;
	size_t sz = sizeof(struct pst_status);
/*
 * Read the pst_status information for all processes into ps[].
 */
	do {
	    if (np >= npa) {

	    /*
	     * Increase the size of ps[].
	     */
		npa += PSTATINCR;
		nb = (MALLOC_S)(npa * sizeof(struct pst_status));
		if (!ps)
		    ps = (struct pst_status *)malloc(nb);
		else
		    ps = (struct pst_status *)realloc((MALLOC_P *)ps, nb);
		if (!ps) {

ps_alloc_error:
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for pst_status table\n",
			Pn, nb);
		    Exit(1);
		}
	    }
	/*
	 * Read the next block of pst_status structures.
	 */
	    el = (size_t)(npa - np);
	    if ((rc = pstat_getproc(ps + np, sz, el, i)) > 0) {
		np += rc;
		i = (ps + np - 1)->pst_idx + 1;
		if (rc < el)
		    rc = 0;
	    }
	} while (rc > 0);
/*
 * Reduce ps[] to a minimum, unless repeat mode is in effect.
 */
	if (!RptTm && ps && np && (np < npa)) {
	    nb = (MALLOC_S)(np * sizeof(struct pst_status));
	    if (!(ps = (struct pst_status *)realloc((MALLOC_P *)ps, nb)))
		goto ps_alloc_error;
	}
	*n = np;
	return(ps);
}


/*
 * read_vmreg() -- read info about the VM regions of a process
 */

static struct pst_vm_status *
read_vmreg(p, n)
	struct pst_status *p;		/* pst_status for process */
	int *n;				/* returned region count */
{
	size_t ec = (size_t)p->pst_pid;
	MALLOC_S nb;
	int nr, rx;
	static int nra = 0;
	struct pst_vm_status *rp;
	static struct pst_vm_status *reg = (struct pst_vm_status *)NULL;
	size_t sz = sizeof(struct pst_vm_status);
/*
 * Read all VM region information for the process.
 */
	for (nr = rx = 0;; rx++) {
	    if (nr >= nra) {

	    /*
	     * Increase the region table size.
	     */
		nra += VMREGINCR;
		nb = (MALLOC_S)(nra * sizeof(struct pst_vm_status));
		if (!reg)
		    reg = (struct pst_vm_status *)malloc(nb);
		else
		    reg = (struct pst_vm_status *)realloc((MALLOC_P *)reg, nb);
		if (!reg) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for pst_vm_status\n",
			Pn, nb);
		    Exit(1);
		}
	    }
	/*
	 * Read the pst_vm_status structure for the next region.
	 */
	    rp = reg + nr;
	    if (pstat_getprocvm(rp, sz, ec, rx) != 1)
		break;
	    if (IS_PSFILEID(&rp->pst_id) && (rp->pst_id.psf_fileid > 0))
		nr++;
	}
	*n = nr;
	return(reg);
}


/*
 * scanmnttab() -- scan mount table
 */

extern void
scanmnttab()
{
	struct mounts *mp;
/*
 * Scan the mount table to identify NFS file systems and form the psfileid
 * for "/".
 *
 * This function allows the mount table scan to be deferred until its
 * information is needed.
 */
	if ((HvRtPsfid >= 0) && (HasNFS >= 0))
	    return;
	(void) memset((void *)&RtPsfid, 0, sizeof(RtPsfid));
	for (HasNFS = HvRtPsfid = 0, mp = readmnt(); mp; mp = mp->next) {
	    if (mp->MOUNTS_FSTYPE
	    &&  (strcmp(mp->MOUNTS_FSTYPE, MNTTYPE_NFS) == 0
	    ||   strcmp(mp->MOUNTS_FSTYPE, MNTTYPE_NFS3) == 0)) {
		HasNFS = 1;
		mp->is_nfs = 1;
	    } else
		mp->is_nfs = 0;
	    if (!HvRtPsfid && !strcmp(mp->dir, "/")) {
		HvRtPsfid = 1;
		RtPsfid.psf_fsid.psfs_id = mp->dev;
		RtPsfid.psf_fsid.psfs_type = mp->MOUNTS_STAT_FSTYPE;
		RtPsfid.psf_fileid = mp->inode;
	    }
	}
}
