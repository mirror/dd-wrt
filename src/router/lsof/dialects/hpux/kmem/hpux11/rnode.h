/*
 * rnode.h for HP-UX 10.30 and above
 *
 * This header file defines the rnode structure for lsof.  Lsof uses it to get
 * infomation about remote (NFS) nodes -- e.g., node number and size.
 *
 * V. Abell <abe@purdue.edu>
 * February, 1998
 */

#if	!defined(LSOF_RNODE_H)
#define	LSOF_RNODE_H

#include "kernbits.h"

#define	_KERNEL
#include <sys/spinlock.h>
#undef	_KERNEL

#include "vnode.h"

typedef struct krwlock {
	lock_t *interlock;
	u_int delay;
	int read_count;
	char want_write;
	char want_upgrade;
	char waiting;
	char no_swap;
} krwlock_t;

typedef struct kmutex {
	lock_t *spin_lockp;
	int lockp_type;
} kmutex_t;

typedef struct nfs_fhandle {
	int fh_len;
	char fh_buf[64];
} nfs_fhandle_t;

typedef struct rnode {
	KA_T r_freef;
	KA_T r_freeb;
	KA_T r_hash;
	vnode_t r_vnode;	/* the vnode that contains this rnode */
	krwlock_t r_rwlock;
	kmutex_t r_statelock;
	nfs_fhandle_t r_fh;
	uint16_t r_flags;
	int16_t r_error;
	KA_T r_rcred;
	KA_T r_wcred;
	KA_T r_unlcred;
	KA_T r_unlname;
	KA_T r_unldvp;
	int64_t r_size;		/* This should be an off_t, but there's an
				 * unresolvable conflict between the kernel
				 * and application off_t sizes. */
	struct vattr r_attr;	/* the vnode attributes -- e.g., node number,
				 * size, etc.  (See ./vnode.h.) */

/*
 * These q4 elements are ignored.

	time_t r_attrtime;
	time_t r_mtime;
	int32_t r_mapcnt;
	uint32_t r_count;
	int32_t r_seq;
	int *r_acc;
	int *r_dir;
	int *r_direof;
	symlink_cache r_symlink;
	u_char r_verf;
	commit_t r_commit;
	recover_t r_recover;
	uint32_t r_truncaddr;
	uint32_t r_iocnt;
	kcondvar_t r_trunccv;
	kmutex_t r_serialize;
	u_char r_cookieverf;
	int *r_lmpl;
	daddr_t r_lastr;
	kcondvar_t r_cv;
	int *r_owner;
	short r_ownercount;

 * Those q4 elements were ignored.
 */

} rnode_t;

#endif	/* !defined(LSOF_RNODE_H) */
