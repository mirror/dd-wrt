/*
 * dstore.c - Darwin global storage for /dev/kmem-based lsof
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
static char *rcsid = "$Id: dstore.c,v 1.5 2005/11/01 20:24:51 abe Exp $";
#endif


#include "lsof.h"

struct file *Cfp;			/* curent file's file struct pointer */


/*
 * Drive_Nl -- table to drive the building of Nl[] via build_Nl()
 *             (See lsof.h and misc.c.)
 */

struct drive_Nl Drive_Nl[] = {

	{ "aproc",	"_allproc"	},
	{ "nproc",	"_nprocs"	},
	{ X_NCACHE,	"_nchashtbl"	},
	{ X_NCSIZE,	"_nchash"	},
	{ "",		""		},
	{ NULL,		NULL		}
};

int Kd = -1;				/* KMEM descriptor */
KA_T Kpa;				/* kernel proc struct address */
struct l_vfs *Lvfs = NULL;		/* local vfs structure table */

int Np = 0;				/* number of kernel processes */

struct kinfo_proc *P = NULL;		/* local process table copy */

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
	{ (long)FFSYNC,		FF_FSYNC	},
	{ (long)FMARK,		FF_MARK		},
	{ (long)FDEFER,		FF_DEFER	},
	{ (long)FHASLOCK,	FF_HASLOCK	},
	{ (long)O_NOCTTY,	FF_NOCTTY	},
	{ (long)O_EVTONLY,	FF_EVTONLY	},
	{ (long)0,		NULL 		}
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

	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */
