/*
 * dstore.c - SCO OpenServer global storage for lsof
 */


/*
 * Copyright 1995 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 1995 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dstore.c,v 1.9 2002/12/03 18:23:08 abe Exp $";
#endif


#include "lsof.h"


char **Cdevsw = NULL;		/* names from kernel's cdevsw[].d_name */
int Cdevcnt = 0;		/* Cdevsw[] count */
int CloneMajor;			/* clone major device */


/*
 * Drive_Nl -- table to drive the building of Nl[] via build_Nl()
 *             (See lsof.h and misc.c.)
 */

struct drive_Nl Drive_Nl[] = {
	{ "ncdev",	"cdevcnt"		},
	{ "cdev",	"cdevsw"		},
	{ "dnlc",	"dnlc__cache"		},  /* OSRV>=504 */
	{ "ndnlc",	"dnlc__cacheents"	},  /* OSRV>=504 */
	{ "pdnlc",	"dnlc__cache_is_ptr"	},  /* OSRV>=507 */
	{ "dtnc",	"dtcache"		},  /* 500<=OSRV<504 */
	{ "htnc",	"htcache"		},  /* 500<=OSRV<504 */
	{ "hz",		"Hz"			},
	{ "lbolt",	"lbolt"			},
	{ "nfnc",	"ncache"		},  /* HAS_NFS */
	{ "nnfnc",	"nc_size"		},  /* HAS_NFS */
	{ "nxdm",	"nxdevmaps"		},  /* OSRV>=40 */
	{ "pregpp",	"pregpp"		},  /* OSRV<500 */
	{ "proc",	"proc"			},
	{ "scouts",	"scoutsname"		},  /* OSRV>=500 */
	{ "sockd",	"sockdev"		},
	{ "sockt",	"socktab"		},
	{ "s5nc",	"s5cache"		},  /* OSRV<504 */
	{ "var",	"v"			},
	{ "ndtnc",	"v_dtcacheents"		},  /* 500<=OSRV<504 */
	{ "nhtnc",	"v_htcacheents"		},  /* 500<=OSRV<504 */
	{ "ns5nc",	"v_s5cacheents"		},  /* 500<=OSRV<504 */
	{ "xdm",	"xdevmap"		},  /* OSRV>=40 */
	{ NULL,		NULL			}
};

int EventMajor;			/* event major device number */
char **Fsinfo = NULL;		/* file system information */
int Fsinfomax = 0;		/* maximum file system type */
int HaveCloneMajor = 0;		/* have clone major device number = 1 */
int HaveEventMajor = 0;		/* have event major device number */
int HaveSockdev = 0;		/* socket device number status: 1 = available */
int Hz = -1;			/* system clock frequency */
int Kd = -1;			/* /dev/kmem file descriptor */
KA_T Lbolt = (KA_T)0;		/* kernel's lbolt variable address */
int Sockdev;			/* socket device number */
KA_T Socktab = (KA_T)0;		/* address of socket pointer table */

#if	defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {
	{ (long)FREAD,		FF_READ		},
	{ (long)FWRITE,		FF_WRITE	},
	{ (long)FNDELAY,	FF_NDELAY	},
	{ (long)FAPPEND,	FF_APPEND	},
	{ (long)FNONBLOCK,	FF_NBLOCK	},
	{ (long)FRCACH,		FF_RCACH	},
	{ (long)FSTOPIO,	FF_STOPIO	},

# if	defined(FASYNC)
	{ (long)FASYNC,		FF_ASYNC	},
# endif	/* defined(FASYNC`) */

# if	defined(FNET)
	{ (long)FNET,		FF_NET		},
# endif	/* defined(FNET) */

# if	defined(FSYNC)
	{ (long)FSYNC,		FF_SYNC		},
# endif	/* defined(FSYNC) */

	{ (long)0,		NULL 		}
};


/*
 * Pof_tab[] - table for print process open file flags
 */

struct pff_tab Pof_tab[] = {

	{ (long)EXCLOSE,	POF_CLOEXEC	},

# if	defined(AUD_READ)
	{ (long)AUD_READ,	POF_BNRD	},
# endif	/* defined(AUD_READ) */

# if	defined(AUD_WRITE)
	{ (long)AUD_WRITE,	POF_BNWR	},
# endif	/* defined(AUDWRITE) */

# if	defined(SEC_SIGHUPPED)
	{ (long)SEC_SIGHUPPED,	POF_BNHUP	},
# endif	/* defined(SEC_SIGHUPPED) */

	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */


#if	OSRV>=40
/*
 * The following items are needed by the internal kernel major()
 * and minor() macros for mapping extended minor numbers.
 */

int nxdevmaps = -1;		/* maximum kernel xdevmap[] index */
struct XDEVMAP *Xdevmap;	/* dynamically allocated xdevmap[] */
#endif	/* OSRV>=40 */
