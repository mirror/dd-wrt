/*
 * dstore.c -- Darwin global storage for libproc-based lsof
 */


/*
 * Portions Copyright 2005 Apple Computer, Inc.  All rights reserved.
 *
 * Copyright 2005 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Allan Nathanson, Apple Computer, Inc., and Victor A.
 * Abell, Purdue University.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors, nor Apple Computer, Inc. nor Purdue University
 *    are responsible for any consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either
 *    by explicit claim or by omission.  Credit to the authors, Apple
 *    Computer, Inc. and Purdue University must appear in documentation
 *    and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */


#ifndef lint
static char copyright[] =
"@(#) Copyright 2005 Apple Computer, Inc. and Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dstore.c,v 1.4 2008/10/21 16:15:16 abe Exp abe $";
#endif


#include "lsof.h"


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

# if	defined(FHASLOCK)
	{ (long)FHASLOCK,	FF_HASLOCK	},
# endif	/* defined(FHASLOCK) */

	{ (long)O_NOCTTY,	FF_NOCTTY	},
	{ (long)O_EVTONLY,	FF_EVTONLY	},
	{ (long)0,		NULL 		}
};


/*
 * Pof_tab[] - table for print process open file flags
 */

struct pff_tab Pof_tab[] = {

# if	defined(PROC_FP_SHARED)
	{ (long)PROC_FP_SHARED,	"SH"		},
# endif	/* defined(PROC_FP_SHARED) */

# if	defined(PROC_FP_CLEXEC)
	{ (long)PROC_FP_CLEXEC,	POF_CLOEXEC	},
# endif	/* defined(PROC_FP_CLEXEC) */

# if	defined(PROC_FP_GUARDED)
	{ (long)PROC_FP_GUARDED,"GRD"		},
# endif	/* defined(PROC_FP_GUARDED) */

# if	defined(UF_CLOSING)
	{ (long)UF_CLOSING,	POF_CLOSING	},
# endif	/* defined(UF_CLOSING) */

# if	defined(UF_EXCLOSE)
	{ (long)UF_EXCLOSE,	POF_CLOEXEC	},
# endif	/* defined(UF_EXCLOSE) */

# if	defined(UF_RESERVED)
	{ (long)UF_RESERVED,	POF_RESERVED	},
# endif	/* defined(UF_RESERVED) */

	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */


#if	defined(PROC_FP_GUARDED)
/*
 * Pgf_tab[] - table for print process open file guard flags
 */

struct pff_tab Pgf_tab[] = {
	{ (long)PROC_FI_GUARD_CLOSE,		"CLOSE"		},
	{ (long)PROC_FI_GUARD_DUP,		"DUP"		},
	{ (long)PROC_FI_GUARD_SOCKET_IPC,	"SOCKET"	},
	{ (long)PROC_FI_GUARD_FILEPORT,		"FILEPORT"	},

	{ (long)0,				NULL		}
};
#endif	/* defined(PROC_FP_GUARDED) */
