/*
 * vnode.h for HP-UX 10.30 and above
 *
 * This header file defines the locklist, vnode and vattr structures for lsof
 * in a manner that can be compiled at the application level.
 *
 * V. Abell <abe@purdue.edu>
 * February, 1998
 */

#if	!defined(LSOF_VNODE_H)
#define	LSOF_VNODE_H
#define	_SYS_VNODE_INCLUDED	/* prevent inclusion of <sys/vnode.h> */

#include "kernbits.h"
#include <sys/types.h>
#include <sys/sem_beta.h>
#include <sys/time.h>

#define	VROOT		0x01

typedef struct locklist {		/* lock list */
	KA_T ll_link;
	short ll_count;
	short ll_flags;			/* flags */
	KA_T ll_proc;			/* proc structure address (unused) */
	KA_T ll_kthreadp;		/* thread structure address */

	/* ll_start and ll_end should be typed off_t, but there's an
	 * unresolvable conflict between the size of the kernel's off_t
	 * and the 32 and 64 bit application off_t sizes.
	 */

	int64_t ll_start;		/* lock start */
	int64_t ll_end;			/* lock end */
	short ll_type;			/* lock type -- e.g., F_RDLCK or
					 * F_WRLCK */
	KA_T ll_vp;
	KA_T ll_waitq;
	KA_T ll_fwd;			/* forward link */
	KA_T ll_rev;
	KA_T ll_sib_fwd;
	KA_T ll_sib_rev;
} locklist_t;

enum vtype {
	VNON = 0,
	VREG = 0x1,
	VDIR = 0x2,
	VBLK = 0x3,
	VCHR = 0x4,
	VLNK = 0x5,
	VSOCK = 0x6,
	VBAD = 0x7,
	VFIFO = 0x8,
	VFNWK = 0x9,
	VEMPTYDIR = 0xa
};

enum vfstype {
	VDUMMY = 0,
	VNFS = 0x1,
	VUFS = 0x2,
	VDEV_VN = 0x3,
	VNFS_SPEC = 0x4,
	VNFS_BDEV = 0x5,
	VNFS_FIFO = 0x6,
	VCDFS = 0x7,
	VVXFS = 0x8,
	VDFS = 0x9,
	VEFS = 0xa,
	VLOFS = 0xb
};

typedef struct vnode {
	u_short v_flag;			/* flags -- e.g., VROOT */
	u_short v_shlockc;		/* shared lock count */
	u_short v_exlockc;		/* exclusive lock count */
	u_short v_tcount;
	int v_count;
	KA_T v_vfsmountedhere;
	KA_T v_op;			/* operations switch */
	KA_T v_socket;
	KA_T v_stream;			/* associated stream */
	KA_T v_vfsp;			/* pointer to virtual file system
					 * structure */
	enum vtype v_type;		/* vnode type */
	dev_t v_rdev;			/* device -- for VCHR and VBLK
					 * vnodes */
	caddr_t v_data;			/* private data -- i.e., pointer to
					 * successor node structure */
	enum vfstype v_fstype;
	KA_T v_vas;
	vm_sema_t v_lock;
	KA_T v_cleanblkhd;
	KA_T v_dirtyblkhd;
	int v_writecount;
	KA_T v_locklist;		/* locklist structure chain pointer */
	int v_scount;
	int32_t v_nodeid;
	KA_T v_ncachedhd;
	KA_T v_ncachevhd;
	KA_T v_pfdathd;
	u_int v_last_fsync;
} vnode_t;

typedef struct vattr {
	enum vtype va_type;
	u_short va_mode;
	short va_nlink;
	uid_t va_uid;
	gid_t va_gid;
	int32_t va_fsid;
	int32_t va_nodeid;		/* node ID number (e.g., inode
					 * number) */
	off64_t va_size;		/* file size */
	int32_t va_blocksize;
	struct timeval va_atime;
	struct timeval va_mtime;
	struct timeval va_ctime;
	dev_t va_rdev;
	blkcnt64_t va_blocks;
	dev_t va_realdev;
	u_short va_basemode;
	u_short va_acl;
	u_short va_sysVacl;
	u_short va_dummy;
	short va_fstype;
} vattr_t;

#endif	/* !defined(LSOF_VNODE_H) */
