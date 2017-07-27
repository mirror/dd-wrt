/*
 * dstore.c - pstat-based HP-UX global storage for lsof
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
 * Global storage definitions
 */

_T_LONG_T CloneMaj;			/* clone major device number */
int HasNFS = -1;			/* NFS-mounted file system status:
					 *    -1: not yet tested;
					 *     0: tested and none mounted;
					 *     1: tested and some mounted */
int HaveCloneMaj = 0;			/* CloneMaj status */

#if	defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {
	{ (long)PS_FRDONLY,	FF_READ		},
	{ (long)PS_FWRONLY,	FF_WRITE	},
	{ (long)PS_FAPPEND,	FF_APPEND	},
	{ (long)PS_FNODELY,	FF_NDELAY	},
	{ (long)PS_FNBLOCK,	FF_NBLOCK	},
	{ (long)PS_FSYNC,	FF_SYNC		},
	{ (long)PS_FDSYNC,	FF_DSYNC	},
	{ (long)PS_FRSYNC,	FF_RSYNC	},
	{ (long)PS_FLGFILE,	FF_LARGEFILE	},
	{ (long)0,		NULL		}
};


/*
 * Pof_tab[] - table for print process open file flags
 */

struct pff_tab Pof_tab[] = {
	{ (long)PS_FEXCLOS,	POF_CLOEXEC	},
	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */
