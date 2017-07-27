/*
 * dstore.c - NEXTSTEP and OPENSTEP global storage for lsof
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
static char *rcsid = "$Id: dstore.c,v 1.10 2001/08/09 11:44:07 abe Exp $";
#endif


#include "lsof.h"


/*
 * Global storage definitions
 */

#if	defined(HAS_AFS)

# if    defined(HASAOPT)
char *AFSApath = (char *)NULL;		/* alternate AFS name list path
					 * (from -a) */
# endif /* defined(HASAOPT) */

struct vfs *AFSVfsp = (struct vfs *)NULL;
					/* AFS vfs struct kernel address */
#endif	/* defined(HAS_AFS) */

/*
 * Drive_Nl -- table to drive the building of Nl[] via build_Nl()
 *             (See lsof.h and misc.c.)
 */

struct drive_Nl Drive_Nl[] = {
        { "arFid",	 "_afs_rootFid"		},
        { "avops",	 "_afs_vnodeops"	},
        { "avol",	 "_afs_volumes"		},
        { "aproc",	 "_allproc"		},
        { "fvops",	"_fifo_vnodeops"	},
        { "lfsvh",	"_lf_svnode_hash"	},
        { "mxproc",	 "_max_proc"		},

#if	defined(X_NCACHE)
        { X_NCACHE,	 "_ncache"		},
#endif	/* defined(X_NCACHE) */

#if	defined(X_NCSIZE)
        { X_NCSIZE,	 "_ncsize"		},
#endif	/* defined(X_NCSIZE) */

        { "nvops",	 "_nfs_vnodeops"	},
        { "svops",	 "_spec_vnodeops"	},
        { "uvops",	 "_ufs_vnodeops"	},
	{ "",		"",			},
        { NULL,		NULL			},
};

struct file *Fileptr;		/* for process_file() in lib/prfp.c */
int Kd = -1;			/* /dev/kmem file descriptor */

#if	defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {
	{ (long)FREAD,		FF_READ		},
	{ (long)FWRITE,		FF_WRITE	},
	{ (long)FNDELAY,	FF_NDELAY	},
	{ (long)FAPPEND,	FF_APPEND	},
	{ (long)FASYNC,		FF_ASYNC	},
	{ (long)FMARK,		FF_MARK		},
	{ (long)FDEFER,		FF_DEFER	},
	{ (long)FSHLOCK,	FF_SHLOCK	},
	{ (long)FEXLOCK,	FF_EXLOCK	},

#if	defined(POSIX_KERN)
	{ (long)FPOSIX_PIPE,	FF_POSIX_PIPE	},
#endif	/* defined(POSIX_KERN) */

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

	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */
