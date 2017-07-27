/*
 * dstore.c - AIX global storage for lsof
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
static char *rcsid = "$Id: dstore.c,v 1.12 2004/12/30 18:40:59 abe Exp $";
#endif


#include "lsof.h"


/*
 * Global storage definitions
 */

#if	defined(HAS_AFS)
struct nlist AFSnl[] = {
	{ "afs_rootFid",	0, 0, 0, 0, 0 },
	{ "afs_volumes",	0, 0, 0, 0, 0 },
};

# if    defined(HASAOPT)
char *AFSApath = (char *)NULL;		/* alternate AFS name list path
					 * (from -a) */
# endif /* defined(HASAOPT) */

KA_T AFSVfsp = (KA_T)NULL;		/* AFS vfs struct kernel address */
#endif	/* defined(HAS_AFS) */

# if	AIXV>=4140
struct clone *Clone = (struct clone *)NULL;
					/* local clone information */
int CloneMaj = -1;			/* clone major device number */
int ClonePtc = -1;			/* /dev/ptc minor device number */
# endif	/* AIXV>=4140 */

int Kd = -1;				/* /dev/kmem file descriptor */
struct l_vfs *Lvfs = NULL;		/* local vfs structure table */
int Km = -1;				/* /dev/mem file descriptor */

struct nlist Nl[] = {

#if	AIXV<4100
	{ "u",			 0, 0, 0, 0, 0 },
#else	/* AIXV>=4100 */
	{ "__ublock",		 0, 0, 0, 0, 0 },
#endif	/* AIXV<4100 */

};

#if	defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {

# if	defined(FREAD)
	{ (long)FREAD,		FF_READ		},
# else	/* !defined(FREAD) */
#  if	defined(_FREAD)
	{ (long)_FREAD,		FF_READ		},
#  endif	/* defined(_FREAD) */
# endif	/* defined(FREAD) */

# if	defined(FWRITE)
	{ (long)FWRITE,		FF_WRITE	},
# else	/* !defined(FWRITE) */
#  if	defined(_FWRITE)
	{ (long)_FWRITE,	FF_WRITE	},
#  endif	/* defined(_FWRITE) */
# endif	/* defined(FWRITE) */

# if	defined(FNONBLOCK)
	{ (long)FNONBLOCK,	FF_NBLOCK	},
# else	/* !defined(FNONBLOCK) */
#  if	defined(_FNONBLOCK)
	{ (long)_FNONBLOCK,	FF_NBLOCK	},
#  endif	/* defined(_FNONBLOCK) */
# endif	/* defined(FNONBLOCK) */

# if	defined(FAPPEND)
	{ (long)FAPPEND,	FF_APPEND	},
# else	/* !defined(FAPPEND) */
#  if	defined(_FAPPEND)
	{ (long)_FAPPEND,	FF_APPEND	},
#  endif	/* defined(_FAPPEND) */
# endif	/* defined(FAPPEND) */

# if	defined(FSYNC)
	{ (long)FSYNC,		FF_SYNC		},
# else	/* !defined(FSYNC) */
#  if	defined(_FSYNC)
	{ (long)_FSYNC,		FF_SYNC		},
#  endif	/* defined(_FSYNC) */
# endif	/* defined(FSYNC) */

# if	defined(FEXEC)
	{ (long)FEXEC,		FF_EXEC		},
# else	/* !defined(FEXEC) */
#  if	defined(_FEXEC)
	{ (long)_FEXEC,		FF_EXEC		},
#  endif	/* defined(_FEXEC) */
# endif	/* defined(FEXEC) */

# if	defined(FCREAT)
	{ (long)FCREAT,		FF_CREAT	},
# else	/* !defined(FCREAT) */
#  if	defined(_FCREAT)
	{ (long)_FCREAT,	FF_CREAT	},
#  endif	/* defined(_FCREAT) */
# endif	/* defined(FCREAT) */

# if	defined(FTRUNC)
	{ (long)FTRUNC,		FF_TRUNC	},
# else	/* !defined(FTRUNC) */
#  if	defined(_FTRUNC)
	{ (long)_FTRUNC,	FF_TRUNC	},
#  endif	/* defined(_FTRUNC) */
# endif	/* defined(FTRUNC) */

# if	defined(FEXCL)
	{ (long)FEXCL,		FF_EXCL		},
# else	/* !defined(FEXCL) */
#  if	defined(_FEXCL)
	{ (long)_EXCL,		FF_EXCL		},
#  endif	/* defined(_FEXCL) */
# endif	/* defined(FEXCL) */

# if	defined(FNOCTTY)
	{ (long)FNOCTTY,	FF_NOCTTY	},
# else	/* !defined(FNOCTTY) */
#  if	defined(_FNOCTTY)
	{ (long)_FNOCTTY,	FF_NOCTTY	},
#  endif	/* defined(_FNOCTTY) */
# endif	/* defined(FNOCTTY) */

# if	defined(FRSHARE)
	{ (long)FRSHARE,	FF_RSHARE	},
# else	/* !defined(FRSHARE) */
#  if	defined(_FRSHARE)
	{ (long)_FRSHARE,	FF_RSHARE	},
#  endif	/* defined(_FRSHARE) */
# endif	/* defined(FRSHARE) */

# if	defined(FDEFER)
	{ (long)FDEFER,		FF_DEFER	},
# else	/* !defined(FDEFER) */
#  if	defined(_FDEFER)
	{ (long)_FDEFER,	FF_DEFER	},
#  endif	/* defined(_FDEFER) */
# endif	/* defined(FDEFER) */

# if	defined(FDELAY)
	{ (long)FDELAY,		FF_DELAY	},
# else	/* !defined(FDELAY) */
#  if	defined(_FDELAY)
	{ (long)_FDELAY,	FF_DELAY	},
#  endif	/* defined(_FDELAY) */
# endif	/* defined(FDELAY) */

# if	defined(FNDELAY)
	{ (long)FNDELAY,	FF_NDELAY	},
# else	/* !defined(FNDELAY) */
#  if	defined(_FNDELAY)
	{ (long)_FNDELAY,	FF_NDELAY	},
#  endif	/* defined(_FNDELAY) */
# endif	/* defined(FNDELAY) */

# if	defined(FNSHARE)
	{ (long)FNSHARE,	FF_NSHARE	},
# else	/* !defined(FNSHARE) */
#  if	defined(_FNSHARE)
	{ (long)_FNSHARE,	FF_NSHARE	},
#  endif	/* defined(_FNSHARE) */
# endif	/* defined(FNSHARE) */

# if	defined(FASYNC)
	{ (long)FASYNC,		FF_ASYNC	},
# else	/* !defined(FASYNC) */
#  if	defined(_FASYNC)
	{ (long)_FASYNC,	FF_ASYNC	},
#  endif	/* defined(_FASYNC) */
# endif	/* defined(FASYNC) */

# if	defined(FAIO)
	{ (long)FAIO,		FF_AIO		},
# else	/* !defined(FAIO) */
#  if	defined(_FAIO)
	{ (long)_FAIO,		FF_AIO		},
#  endif	/* defined(_FAIO) */
# endif	/* defined(FAIO) */

# if	defined(FCIO)
	{ (long)FCIO,		FF_CIO		},
# else	/* !defined(FCIO) */
#  if	defined(_FCIO)
	{ (long)_FCIO,		FF_CIO		},
#  endif	/* defined(_FCIO) */
# endif	/* defined(FCIO) */

# if	defined(FMOUNT)
	{ (long)FMOUNT,		FF_MOUNT	},
# else	/* !defined(FMOUNT) */
#  if	defined(_FMOUNT)
	{ (long)_FMOUNT,	FF_MOUNT	},
#  endif	/* defined(_FMOUNT) */
# endif	/* defined(FMOUNT) */

# if	defined(FSYNCALL)
	{ (long)FSYNCALL,	FF_SYNC		},
# else	/* !defined(FSYNCALL) */
#  if	defined(_FSYNCALL)
	{ (long)_FSYNCALL,	FF_SYNC		},
#  endif	/* defined(_FSYNCALL) */
# endif	/* defined(FSYNCALL) */

# if	defined(FNOCACHE)
	{ (long)FNOCACHE,	FF_NOCACHE	},
# else	/* defined(FNOCACHE) */
#  if	defined(_FNOCACHE)
	{ (long)_FNOCACHE,	FF_NOCACHE	},
#  endif	/* defined(_FNOCACHE) */
# endif	/* defined(FNOCACHE) */

# if	defined(FREADSYNC)
	{ (long)FREADSYNC,	FF_RSYNC	},
# else	/* !defined(FREADSYNC) */
#  if	defined(_FREADSYNC)
	{ (long)_FREADSYNC,	FF_RSYNC	},
#  endif	/* defined(_FREADSYNC) */
# endif	/* defined(FREADSYNC) */

# if	defined(FDATASYNC)
	{ (long)FDATASYNC,	FF_DSYNC	},
# else	/* !defined(FDATASYNC) */
#  if	defined(_FDATASYNC)
	{ (long)_FDATASYNC,	FF_DSYNC	},
#  endif	/* defined(_FDATASYNC) */
# endif	/* defined(FDATASYNC) */

# if	defined(FDEFERIND)
	{ (long)FDEFERIND,	FF_DEFERIND	},
# else	/* !defined(FDEFERIND) */
#  if	defined(_FDEFERIND)
	{ (long)_FDEFERIND,	FF_DEFERIND	},
#  endif	/* defined(_FDEFERIND) */
# endif	/* defined(FDEFERIND) */

# if	defined(FDATAFLUSH)
	{ (long)FDATAFLUSH,	FF_DATAFLUSH	},
# else	/* !defined(FDATAFLUSH) */
#  if	defined(_FDATAFLUSH)
	{ (long)_FDATAFLUSH,	FF_DATAFLUSH	},
#  endif	/* defined(_FDATAFLUSH) */
# endif	/* defined(FDATAFLUSH) */

# if	defined(FCLREAD)
	{ (long)FCLREAD,	FF_CLREAD	},
# else	/* !defined(FCLREAD) */
#  if	defined(_FCLREAD)
	{ (long)_FCLREAD,	FF_CLREAD	},
#  endif	/* defined(_FCLREAD) */
# endif	/* defined(FCLREAD) */

# if	defined(FLARGEFILE)
	{ (long)FLARGEFILE,	FF_LARGEFILE	},
# else	/* !defined(FLARGEFILE) */
#  if	defined(_FLARGEFILE)
	{ (long)_FLARGEFILE,	FF_LARGEFILE	},
#  endif	/* defined(_FLARGEFILE) */
# endif	/* defined(FLARGEFILE) */

# if	defined(FDIRECT)
	{ (long)FDIRECT,	FF_DIRECT	},
# else	/* !defined(FDIRECT) */
#  if	defined(_FDIRECT)
	{ (long)_FDIRECT,	FF_DIRECT	},
#  endif	/* defined(_FDIRECT) */
# endif	/* defined(FDIRECT) */

# if	defined(FSNAPSHOT)
	{ (long)FSNAPSHOT,	FF_SNAP		},
# else	/* !defined(FSNAPSHOT) */
#  if	defined(_FSNAPSHOT)
	{ (long)_FSNAPSHOT,	FF_SNAP		},
#  endif	/* defined(_FSNAPSHOT) */
# endif	/* defined(FAIO) */

# if	defined(FDOCLONE)
	{ (long)FDOCLONE,	FF_DOCLONE	},
# else	/* !defined(FDOCLONE) */
#  if	defined(_FDOCLONE)
	{ (long)_FDOCLONE,	FF_DOCLONE	},
#  endif	/* defined(_FDOCLONE) */
# endif	/* defined(FDOCLONE) */

# if	defined(FKERNEL)
	{ (long)FKERNEL,	FF_KERNEL	},
# else	/* !defined(FKERNEL) */
#  if	defined(_FKERNEL)
	{ (long)_FKERNEL,	FF_KERNEL	},
#  endif	/* defined(_FKERNEL) */
# endif	/* defined(FKERNEL) */

# if	defined(FMSYNC)
	{ (long)FMSYNC,		FF_MSYNC	},
# else	/* !defined(FMSYNC) */
#  if	defined(_FMSYNC)
	{ (long)_FMSYNC,	FF_MSYNC	},
#  endif	/* defined(_FMSYNC) */
# endif	/* defined(FMSYNC) */

# if	defined(GCFDEFER)
	{ (long)GCFDEFER,	FF_GCFDEFER	},
# endif	/* defined(GCFDEFER) */

# if	defined(GCFMARK)
	{ (long)GCFMARK,	FF_GCFMARK	},
# endif	/* defined(GCFMARK) */

	{ (long)0,		NULL		}
};


/*
 * Pof_tab[] - table for print process open file flags
 */

struct pff_tab Pof_tab[] = {

# if	defined(UF_EXCLOSE)
	{ (long)UF_EXCLOSE,	POF_CLOEXEC	},
# endif	/* defined(UF_EXCLOSE) */

# if	defined(UF_MAPPED)
	{ (long)UF_MAPPED,	POF_MAPPED	},
# endif	/* defined(UF_MAPPED) */

# if	defined(UF_FDLOCK)
	{ (long)UF_FDLOCK,	POF_FDLOCK	},
# endif	/* defined(UF_FDLOCK) */

# if	defined(UF_AUD_READ)
	{ (long)UF_AUD_READ,	POF_BNRD	},
# endif	/* defined(UF_AUD_READ) */

# if	defined(UF_AUD_WRITE)
	{ (long)UF_AUD_WRITE,	POF_BNWR	},
# endif	/* defined(UF_AUD_WRITE) */

# if	defined(UF_FSHMAT)
	{ (long)UF_FSHMAT,	POF_FSHMAT	},
# endif	/* defined(UF_FSHMAT) */

# if	defined(UF_CLOSING)
	{ (long)UF_CLOSING,	POF_CLOSING	},
# endif	/* defined(UF_CLOSING) */

# if	defined(UF_ALLOCATED)
	{ (long)UF_ALLOCATED,	POF_ALLOCATED	},
# endif	/* defined(UF_ALLOCATED) */

	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */

#if	AIXV>=4110
struct ublock __ublock;			/* dummy so we can define _KERNEL
					 * for <sys/user.h> */

# if	AIXA>2
void aix_dstore_dummy_function() {}		/* for ia64 idebug */
# endif	/* AIXA>2 */
#endif	/* AIXV>=4110 */
