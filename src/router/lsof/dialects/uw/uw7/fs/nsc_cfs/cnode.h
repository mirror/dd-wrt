/*
 * cnode.h -- Vic Abell's definition of an nsc_cfs node - with help from
 *	      the kind folks at SCO
 */


/*
 * Copyright 2001 Purdue Research Foundation, West Lafayette, Indiana
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

#if	!defined(FS_NSC_CFS_CNODE_H)
#define	FS_NSC_CFS_CNODE_H	1

#include <sys/nsc_synch.h>


/*
 * chandle_t definition from the kernel's <fs/nsc_cfs/cfs.h>
 */

#define CFS_FHSIZE 44

typedef struct cfhandle {
	char fh_data[CFS_FHSIZE];
} cfhandle_t;


/*
 * CFS node structure
 */

typedef struct cnode {
	struct cnode	*c_freef;	/* free list forward pointer */
	struct cnode	*c_freeb;	/* free list back pointer */
	struct cnode	*c_hash;	/* cnode hash chain */
	struct vnode	c_vnode;	/* vnode for remote file */
	cfhandle_t	c_fh;		/* file handle */
	void *c_hp;			/* Hash pointer */
	u_long		c_flags;	/* flags, see below */
	union {
		daddr_t C_nextr;	/* next byte read offset (read-ahead) */
		int	C_lastcookie;	/* last readdir cookie */
	} c_c;
	long		c_owner;	/* proc index for locker of cnode */
	long		c_count;	/* number of cnode locks for c_owner */
	long		c_lwpid;	/* lwp id of locker of cnode */
	CONDITION_T	c_cxlock;	/* Synch structure */
	int		c_rw_excl;	/* RW locked exclusively */
	/* Credentials here are only for use by cfs-as-nfs-client. */
	struct cred	*c_cred;	/* current credentials */
	struct vattr	c_attr;		/* cached vnode attributes */
/*
 * Lsof needs nothing below c_attr.
 */
} cnode_t;
#endif	/* !defined(FS_NSC_CFS_CNODE_H) */
