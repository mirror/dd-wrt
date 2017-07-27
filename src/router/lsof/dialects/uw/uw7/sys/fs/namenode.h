/*
 * Copyright (c) 1998 The Santa Cruz Operation, Inc.. All Rights Reserved. 
 *                                                                         
 *        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF THE               
 *                   SANTA CRUZ OPERATION INC.                             
 *                                                                         
 *   The copyright notice above does not evidence any actual or intended   
 *   publication of such source code.                                      
 */

#ifndef _FS_NAMEFS_NAMENODE_H	/* wrapper symbol for kernel use */
#define _FS_NAMEFS_NAMENODE_H	/* subject to change without notice */

#ident	"@(#)kern:fs/namefs/namenode.h	1.13.2.1"
#ident	"$Header: $"

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef _KERNEL_HEADERS

#include <util/types.h>	/* REQUIRED */
#include <util/ksynch.h>	/* REQUIRED */
#include <fs/vnode.h>	/* REQUIRED */
#include <acc/dac/acl.h>	/* REQUIRED */

#elif defined(_KERNEL) || defined(_KMEMUSER)

#include <sys/types.h>	/* REQUIRED */
#include <sys/ksynch.h>	/* REQUIRED */
#include <sys/vnode.h>	/* REQUIRED */
#include <sys/acl.h>	/* REQUIRED */

#endif /* _KERNEL_HEADERS */

/*
 * This structure is used to pass a file descriptor from user
 * level to the kernel. It is first used by fattach() and then
 * be NAMEFS.
 */
struct namefd {
	int fd;
};

#if defined(_KERNEL) || defined(_KMEMUSER)

/*
 * Each NAMEFS object is identified by a struct namenode/vnode pair.
 */
struct namenode {
	struct vnode    nm_vnode;	/* represents mounted file desc.*/
	ushort		nm_flag;	/* flags defined below */
	struct vattr    nm_vattr;	/* attributes of mounted file desc.*/
	struct vnode	*nm_filevp;	/* file desc. prior to mounting */
	struct file	*nm_filep;	/* file pointer of nm_filevp */
	struct vnode	*nm_mountpt;	/* mount point prior to mounting */
	struct namenode *nm_nextp;	/* next link in the linked list */
	struct namenode *nm_backp;	/* back link in linked list */
	struct acl	*nm_aclp;	/* ACL entries */
	rwsleep_t	nm_lock;	/* protects namenode */
};

#endif /* _KERNEL || _KMEMUSER */

#ifdef _KERNEL

/*
 * Valid flags for namenodes.
 */
#define NMUNMOUNT	 01	/* the namenode is unmounted */
#define NMREMOVED	 02	/* the namenode is removed from namenode list */
#define NMREAD     	 04	/* the mount file has FREAD set */
#define NMWRITE     	010	/* the mount file has FWRITE set */

/*
 * Constants.
 */
#define	NMBSIZE		1024	/* NAMEFS block size */
#define	NMFSIZE		1024	/* NAMEFS fundamental block size */

/*
 * Macros to convert a vnode to a namenode, and vice versa.
 */
#define VTONM(vp) ((struct namenode *)((vp)->v_data))
#define NMTOV(nm) (&(nm)->nm_vnode)

#define	STREAM_LOCK(stp) LOCK((stp)->sd_mutex, PLSTR);
#define	STREAM_UNLOCK(stp, pl) UNLOCK((stp)->sd_mutex, pl);

extern int nm_tflush;		/* the frequency of flush namefs */
extern int namefs_fstype;	/* index into vfssw returned by vfs_attach */

#endif	/* _KERNEL */

#if defined(__cplusplus)
	}
#endif

#endif	/* _FS_NAMEFS_NAMENODE_H */
