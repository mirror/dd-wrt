/*
 * dstore.c - Solaris global storage for lsof
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
static char *rcsid = "$Id: dstore.c,v 1.23 2010/01/18 19:03:54 abe Exp $";
#endif


#include "lsof.h"


/*
 * Global storage definitions
 */

#if	defined(HAS_AFS)

# if	defined(HASAOPT)
char *AFSApath = (char *)NULL;		/* alternate AFS name list path
					 * (from -a) */
# endif	/* defined(HASAOPT) */

dev_t AFSdev;				/* AFS file system device number */
int AFSdevStat = 0;			/* AFSdev status: 0 = unknown;
					 *		  1 = known */
int AFSfstype = -1;			/* AFS file system type index */
KA_T AFSVfsp = (KA_T)NULL;		/* AFS vfs struct kernel address */
#endif	/* defined(HAS_AFS) */

struct clone *Clone = NULL;		/* clone list */
major_t CloneMaj;			/* clone major device number */


/*
 * Drive_Nl -- table to drive the building of Nl[] via build_Nl()
 *	       (See lsof.h and misc.c.)
 */

struct drive_Nl Drive_Nl[] = {
	{ "afsops",	"afs_ops"		 },
	{ "arFid",	"afs_rootFid"		 },
	{ "avops",	"afs_vnodeops"		 },
	{ "Avops",	"Afs_vnodeops"		 },
	{ "avol",	"afs_volumes"		 },
	{ "auvops",	"auto_vnodeops"		 },
	{ "ctfsadir",	"ctfs_ops_adir"		 },
	{ "ctfsbund",	"ctfs_ops_bundle"	 },
	{ "ctfscdir",	"ctfs_ops_cdir"		 },
	{ "ctfsctl",	"ctfs_ops_ctl",		 },
	{ "ctfsevt",	"ctfs_ops_event",	 },
	{ "ctfslate",	"ctfs_ops_latest",	 },
	{ "ctfsroot",	"ctfs_ops_root",	 },
	{ "ctfsstat",	"ctfs_ops_stat",	 },
	{ "ctfssym",	"ctfs_ops_sym",		 },
	{ "ctfstdir",	"ctfs_ops_tdir",	 },
	{ "ctfstmpl",	"ctfs_ops_tmpl",	 },
	{ "cvops",	"cachefs_vnodeops"	 },
	{ "clmaj",	"clonemaj"		 },
	{ "clmaj_alt",	"clone_major"		 },
	{ "fdops",	"fdvnodeops"		 },
	{ "fd_ops",	"fd_vnodeops"		 },
	{ "fvops",	"fifo_vnodeops"		 },
	{ "hvops",	"hsfs_vnodeops"		 },
	{ "lvops",	"lo_vnodeops"		 },
	{ "mntops",	"mntvnodeops"		 },
	{ "mvops",	"mvfs_vnodeops"		 },

#if	solaris<90000
	{ X_NCACHE,	"ncache"		 },
	{ X_NCSIZE,	"ncsize"		 },
#else	/* solaris>=90000 */
	{ X_NCACHE,	"nc_hash"		 },
	{ X_NCSIZE,	"nc_hashsz"		 },
	{ "hshav",	"nc_hashavelen"		 },
#endif	/* solaris<90000 */

#if	defined(NCACHE_NEGVN)
	{ NCACHE_NEGVN,	NCACHE_NEGVN		 },
#endif	/* defined(NCACHE_NEGVN) */

	{ "nvops",	"nfs_vnodeops"		 },
	{ "n3vops",	"nfs3_vnodeops"		 },
	{ "n4vops",	"nfs4_vnodeops"		 },
	{ "nmvops",	"nm_vnodeops"		 },
        { "nproc",	"nproc"			 },
	{ "pdvops",	"pcfs_dvnodeops"	 },
	{ "pfvops",	"pcfs_fvnodeops"	 },
	{ "portvops",	"port_vnodeops"		 },
	{ "pract",	"practive"		 },
	{ "prvops",	"prvnodeops"		 },
	{ "sam1vops",	"samfs_vnodeops"	 },
	{ "sam2vops",	"samfs_client_vnodeops"	 },
	{ "sam3vops",	"samfs_vnodeopsp"	 },
	{ "sam4vops",	"samfs_client_vnodeopsp" },
	{ "sdevops",	"sdev_vnodeops"		 },
	{ "sgvops",	"segvn_ops"		 },
	{ "shvops",	"sharefs_ops_data"	 },
	{ "sckvops",	"sock_vnodeops"		 },
	{ "socketvops",	"socket_vnodeops"	 },
	{ "spvops",	"spec_vnodeops"		 },
	{ "sncavops",	"socknca_vnodeops"	 },
	{ "stpivops",	"socktpi_vnodeops"	 },
	{ "tvops",	"tmp_vnodeops"		 },
	{ "uvops",	"ufs_vnodeops"		 },
	{ "vvfops",	"fdd_vnops"		 },
	{ "vvfcops",	"fdd_chain_vnops"	 },
	{ "vvfclops",	"vx_fcl_vnodeops_p"	 },
	{ "vvops",	"vx_vnodeops"		 },
	{ "vvops_p",	"vx_vnodeops_p"		 },

#if	solaris>=20500
	{ "devops",	"dv_vnodeops"		 },
	{ "doorops",	"door_vnodeops"		 },
	{ "kbase",	"_kernelbase"		 },
#endif	/* solaris>=20500 */

#if	solaris>=20501
	{ "kasp",	"kas"			 },
#endif	/* solaris>=20501 */

#if	solaris>=110000
	{ "devipnetops","devipnet_vnodeops"	 },
	{ "devnetops",	"devnet_vnodeops"	 },
	{ "devptsops",	"devpts_vnodeops"	 },
	{ "devvtops",	"devvt_vnodeops"	 },
#endif	/* solaris>=110000 */

	{ "zfsdops",	"zfs_dvnodeops"		 },
	{ "zfseops",	"zfs_evnodeops"		 },
	{ "zfsfops",	"zfs_fvnodeops"		 },
	{ "zfsshops",	"zfs_sharevnodeops"	 },
	{ "zfssymops",	"zfs_symvnodeops"	 },
	{ "zfsxdops",	"zfs_xdvnodeops"	 },
	{ "",		""			 },
	{ NULL,		NULL			 }
};

char **Fsinfo = NULL;			/* file system information */
int Fsinfomax = 0;			/* maximum file system type */
int HasALLKMEM = 0;			/* has ALLKMEM device */
int HaveCloneMaj = 0;			/* clone major device number has
					 * been identified and is in
					 * CloneMaj */
kvm_t *Kd = NULL;			/* kvm descriptor */
struct l_vfs *Lvfs = NULL;		/* local vfs structure table */
struct netclone *Netclone = NULL;	/* net clone devices from
					 * /devices/pseudo */

#if	defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {
	{ (long)FREAD,		FF_READ		},
	{ (long)FWRITE,		FF_WRITE	},
	{ (long)FNDELAY,	FF_NDELAY	},
	{ (long)FAPPEND,	FF_APPEND	},
	{ (long)FSYNC,		FF_SYNC		},

# if	defined(FREVOKED)
	{ (long)FREVOKED,	FF_REVOKED	},
# endif	/* defined(FREVOKED) */

	{ (long)FDSYNC,		FF_DSYNC	},
	{ (long)FRSYNC,		FF_RSYNC	},

# if	defined(FOFFMAX)
	{ (long)FOFFMAX,	FF_LARGEFILE	},
# endif	/* defined(FFOFFMAX) */

	{ (long)FNONBLOCK,	FF_NBLOCK	},
	{ (long)FNOCTTY,	FF_NOCTTY	},
	{ (long)FASYNC,		FF_ASYNC	},
	{ (long)FNODSYNC,	FF_NODSYNC	},
	{ (long)0,		NULL		}
};


/*
 * Pof_tab[] - table for print process open file flags
 */

struct pff_tab Pof_tab[] = {

# if	defined(UF_EXCLOSE)
	{ (long)UF_EXCLOSE,	POF_CLOEXEC	},
# endif	/* defined(UF_EXCLOSE) */

# if	defined(FD_CLOEXEC)
	{ (long)FD_CLOEXEC,	POF_CLOEXEC	},
# endif	/* defined(FD_CLOEXEC) */

# if	defined(UF_FDLOCK)
	{ (long)UF_FDLOCK,	POF_FDLOCK	},
# endif	/* defined(UF_FDLOCK) */

	{ (long)0,		NULL		}
};
#endif	/* defined(HASFSTRUCT) */

struct pseudo *Pseudo = NULL;		/* non-clone devices from
					 * /devices/pseudo */
int Unof;				/* u_nofiles value */
