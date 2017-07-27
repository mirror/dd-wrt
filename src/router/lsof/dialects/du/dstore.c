/*
 * dstore.c - DEC OSF/1, Digital UNIX, Tru64 UNIX global storage for lsof
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
static char *rcsid = "$Id: dstore.c,v 1.10 2000/08/09 20:06:50 abe Exp $";
#endif


#include "lsof.h"


int CloneMaj;				/* clone major device number */


/*
 * Drive_Nl -- table to drive the building of Nl[] via build_Nl()
 *             (See lsof.h and misc.c.)
 */

struct drive_Nl Drive_Nl[] = {
        { "cldev",	"clonedev"		},
        { "fids",	"fids"			},
	{ "msfsubc",	"msfs_ubcops"		},

#if	DUV>=50100
	{ "advfsvfs",	"msfs_vfsops"		},
	{ "cdfsvfs",	"cdfs_vfsops"		},
	{ "dvdfsvfs",	"dvdfs_vfsops"		},
	{ "fdfsvfs",	"fdfs_vfsops"		},
	{ "fsfsrvp",	"fdfs_root_directory"	},
	{ "nchsz",	"nchsz"			},
	{ "ncpus",	"ncpus"			},
	{ "nfsvfs",	"nfs_vfsops"		},
	{ "nfs3vfs",	"nfs3_vfsops"		},
	{ "procptr",	"processor_ptr"		},
	{ "ufsvfs",	"ufs_vfsops"		},
#else	/* DUV<50100 */
	{ X_NCACHE,	"namecache"		},
	{ X_NCSIZE,	"nchsize"		},
#endif	/* DUV>=50100 */

        { "vnmaxp",	"vn_maxprivate"		},

#if	DUV<30000
        { "proc",	"proc"			},
        { "nproc",	"nproc"			},
#else	/* DUV>=30000 */
        { "npid",	"npid"			},
        { "pidt",	"pidtab"		},
#endif	/* DUV<30000 */

	{ "",		"",			},
	{ NULL,		NULL,			}
};

struct file *Fileptr;			/* for process_file() in lib/prfp.c */
int HaveCloneMaj = 0;			/* status of CloneMaj */
int Kd = -1;
struct l_vfs *Lvfs = NULL;

# if    DUV>=30000
KA_T *Pa = NULL;			/* kernel proc structure addresses */
# endif /* DUV>=30000 */

#if	defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {
	{ (long)FREAD,		FF_READ		},
	{ (long)FWRITE,		FF_WRITE	},
	{ (long)FNONBLOCK,	FF_NBLOCK	},
	{ (long)FNDELAY,	FF_NDELAY	},
	{ (long)FAPPEND,	FF_APPEND	},
	{ (long)FASYNC,		FF_ASYNC	},
	{ (long)FMARK,		FF_MARK		},
	{ (long)FDEFER,		FF_DEFER	},
	{ (long)FSHLOCK,	FF_SHLOCK	},
	{ (long)FEXLOCK,	FF_EXLOCK	},

# if	defined(FKERNEL)
	{ (long)FKERNEL,	FF_KERNEL	},
# endif	/* defined(FKERNEL) */

# if	defined(FKERNEL)
	{ (long)FVTEXT,		FF_VTEXT	},
# endif	/* defined(FVTEXT) */

# if	defined(FSYNC)
	{ (long)FSYNC,		FF_SYNC		},
# endif	/* defined(FSYNC) */

# if	defined(FDSYNC)
	{ (long)FDSYNC,		FF_DSYNC	},
# endif	/* defined(FDSYNC) */

# if	defined(FRSYNC)
	{ (long)FRSYNC,		FF_RSYNC	},
# endif	/* defined(FRSYNC) */

	{ (long)0,		NULL		}
};


/*
 * Pof_tab[] - table for print process open file flags
 */

struct pff_tab Pof_tab[] = {

# if	defined(UF_EXCLOSE)
	{ (long)UF_EXCLOSE,		POF_CLOEXEC	},
# else	/* !defined(UF_EXCLOSE) */
	{ (long)1,			POF_CLOEXEC	},
# endif	/* defined(UF_EXCLOSE) */

# if	defined(UF_MAPPED)
	{ (long)UF_MAPPED,		POF_MAPPED	},
# endif	/* defined(UF_MAPPED) */

# if	defined(UF_RESERVED_WAIT)
	{ (long)UF_RESERVED_WAIT,	POF_RSVWT	},
# endif	/* defined(UF_RESERVED_WAIT) */

	{ (long)0,			NULL		}
};
#endif	/* defined(HASFSTRUCT) */

struct proc *Ps = NULL;			/* local proc structures */
int Psn = 0;				/* entries in Paddr[] and Ps[] */
int Vnmxp;				/* vnode's max private area length */
