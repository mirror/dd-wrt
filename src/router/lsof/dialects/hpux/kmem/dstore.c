/*
 * dstore.c - /dev/kmem-based HP-UX global storage for lsof
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
static char *rcsid = "$Id: dstore.c,v 1.12 2007/04/24 16:25:30 abe Exp $";
#endif


#include "lsof.h"


/*
 * Global storage definitions
 */

#if	defined(HAS_AFS)

# if    defined(HASAOPT)
char *AFSApath = (char *)NULL;		/* alternate AFS name list path
					 * (from -A) */
# endif /* defined(HASAOPT) */

struct vfs *AFSVfsp = (struct vfs *)NULL;
					/* AFS vfs struct kernel address */
#endif	/* defined(HAS_AFS) */

int CloneMaj;				/* clone major device number */


/*
 * Drive_Nl -- table to drive the building of Nl[] via build_Nl()
 *             (See lsof.h and misc.c.)
 */

struct drive_Nl Drive_Nl[] = {

# if	defined(hp9000s300) || defined(__hp9000s300)
	{ "arFid",	"_afs_rootFid"		},
	{ "avops",	"_afs_vnodeops"		},
	{ "avol",	"_afs_volumes"		},
	{ X_NCACHE,	"_ncache"		},
	{ X_NCSIZE,	"_ncsize"		},
	{ "proc",	"_proc"			},
	{ "nvops",	"_nfs_vnodeops"		},
	{ "nvops3",	"_nfs_vnodeops3"	},
	{ "nv3ops",	"_nfs3_vnodeops"	},
	{ "nproc",	"_nproc"		},
	{ "uvops",	"_ufs_vnodeops"		},
	{ "vfops",	"_vnodefops"		},

#  if	HPUXV<800
	{ "upmap",	"_Usrptmap"		},
	{ "upt",	"_usrpt"		},
#  endif	/* HPUXV<800 */
# endif	/* defined(hp9000s300) || defined(__hp9000s300) */

# if	defined(hp9000s800) || defined(__hp9000s800)
	{ "arFid",	"afs_rootFid"		},
	{ "avops",	"afs_vnodeops"		},
	{ "avol",	"afs_volumes"		},
	{ X_NCACHE,	"ncache"		},
	{ X_NCSIZE,	"ncsize"		},
	{ "proc",	"proc"			},
	{ "nvops",	"nfs_vnodeops"		},
	{ "nvops3",	"nfs_vnodeops3"		},
	{ "nv3ops",	"nfs3_vnodeops"		},
	{ "nproc",	"nproc"			},
	{ "uvops",	"ufs_vnodeops"		},
	{ "vfops",	"vnodefops"		},

#  if	HPUXV<800
	{ "ubase",	"ubase"			},
	{ "npids",	"npids"			},
#  else	/* HPUXV>=800 */
#   if	HPUXV>=1000
#    if	HPUXV>=1030
	{ "clmaj",	"clonemajor"		},
#    endif	/* HPUXV>=1030 */
	{ "cvops",	"cdfs_vnodeops"		},
	{ "fvops",	"fifo_vnodeops"		},
	{ "pvops",	"pipe_vnodeops"		},
	{ "svops",	"spec_vnodeops"		},
	{ "vvops",	"vx_vnodeops"		},
#   endif	/* HPUXV>=1000 */
#  endif	/* HPUXV<800 */
# endif	/* defined(hp9000s800) || defined(__hp9000s800) */

	{ "mvops",	"mvfs_vnodeops"		},

# if	HPUXV>=1100
	{ "chunksz",	"sizeof_fd_chunk_t"	},
# endif	/* HPUXV>=1100 */

	{ "",		""			},
	{ NULL,		NULL			}
};


int HaveCloneMaj = 0;			/* CloneMaj status */
int Kd = -1;				/* /dev/kmem file descriptor */
KA_T Kpa;				/* kernel proc structure address */

#if	HPUXV>=1010
KA_T Ktp;				/* kernel thread pointer from proc
					 * struct */
#endif	/* HPUXV>=1010 */

struct l_vfs *Lvfs = NULL;		/* local vfs structure table */

#if	HPUXV<800
int Mem = -1;			/* /dev/mem file descriptor */
#endif	/* HPUXV<800 */

#if	HPUXV<800 && defined(hp9000s800)
int npids;			/* number of PIDs (for uvadd()) */
struct proc *proc;		/* process table address (for uvadd()) */
#endif	/* HPUXV<800 && defined(hp9000s300) */

#if	defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {
	{ (long)FREAD,		FF_READ		},
	{ (long)FWRITE,		FF_WRITE	},
	{ (long)FNDELAY,	FF_NDELAY	},
	{ (long)FAPPEND,	FF_APPEND	},
	{ (long)FMARK,		FF_MARK		},
	{ (long)FDEFER,		FF_DEFER	},
	{ (long)FNBLOCK,	FF_NBLOCK	},
	{ (long)FNOCTTY,	FF_NOCTTY	},

# if	defined(FSYNC)
	{ (long)FSYNC,		FF_SYNC		},
# else	/* !defined(FSYNC) */
#  if	defined(O_SYNC)
	{ (long)O_SYNC,		FF_SYNC		},
#  endif	/* defined(O_SYNC) */
# endif	/* defined(FSYNC) */

# if	defined(FCOPYAVOID)
	{ (long)FCOPYAVOID,	FF_COPYAVOID	},
# endif	/* defined(FCOPYAVOID) */

# if	defined(FPOSIX_AIO)
	{ (long)FPOSIX_AIO,	FF_POSIX_AIO	},
# endif	/* defined(FPOSIX_AIO) */

# if	defined(FLARGEFILE)
	{ (long)FLARGEFILE,	FF_LARGEFILE	},
# else	/* !defined(FLARGEFILE) */
#  if	HPUXV>=1100
	{ (long)0x800,		FF_LARGEFILE	},
#  endif	/* HPUXV>=1100 */
# endif	/* defined(FLARGEFILE) */

	{ (long)0x100,		FF_KERNEL	},
	{ (long)0,		NULL		}
};


/*
 * Pof_tab[] - table for print process open file flags
 */

# if	HPUXV>=1020
#define	UF_EXCLOSE	0x1
#define	UF_MAPPED	0x2
#define	UF_FDLOCK	0x4
#define	UF_INUSE	0x8
# endif	/* HPUXV>=1020 */

struct pff_tab Pof_tab[] = {

# if	defined(UF_EXCLOSE)
	{ (long)UF_EXCLOSE,		POF_CLOEXEC	},
# endif	/* defined(UF_EXCLOSE) */

# if	defined(UF_MAPPED)
	{ (long)UF_MAPPED,		POF_MAPPED	},
# endif	/* defined(UF_MAPPED) */

# if	defined(UF_FDLOCK)
	{ (long)UF_FDLOCK,		POF_FDLOCK	},
# endif	/* defined(UF_FDLOCK) */

# if	defined(UF_INUSE)
	{ (long)UF_INUSE,		POF_INUSE	},
# endif	/* defined(UF_INUSE) */

	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */

#if	HPUXV<800
int Swap = -1;			/* swap device file descriptor */
#endif	/* HPUXV<800 */

#if	HPUXV<800 && defined(hp9000s800)
struct user *ubase;		/* user area base (for uvadd()) */
#endif	/* HPUXV<800 && defined(hp9000s800) */

#if	HPUXV<800 && defined(hp9000s300)
struct user *ubase;		/* user area base (for uvadd()) */
struct pte *Usrptmap;		/* user page table map pointer */
struct pte *usrpt;		/* user page table pointer
				 * (for bktomx from vmmac.h) */
#endif	/* HPUXV<800 && defined(hp9000s300) */

KA_T Vnfops;			/* vnodefops switch address */
