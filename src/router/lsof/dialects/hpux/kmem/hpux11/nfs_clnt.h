/*
 * nfs_clnt.h for HP-UX 10.30 and above
 *
 * This header file defines the mntinfo structure for lsof.  Lsof uses it to
 * obtain the device number of an NFS mount point.
 *
 * V. Abell
 * February, 1998
 */

#if	!defined(LSOF_NFS_CLNT_H)
#define	LSOF_NFS_CLNT_H

#include "kernbits.h"
#include "rnode.h"
#include <rpc/types.h>
#undef	TCP_NODELAY
#undef	TCP_MAXSEG
#include <rpc/rpc.h>
#include <rpc/clnt.h>
#include <sys/xti.h>
#undef	TCP_NODELAY
#undef	TCP_MAXSEG

typedef struct kcondvar {     
	uint32_t _dummy1[6];  
} kcondvar_t; 

typedef struct mntinfo {
	kmutex_t mi_lock;
	KA_T mi_knetconfig;
	struct netbuf mi_addr;
	struct netbuf mi_syncaddr;
	KA_T mi_rootvp;
	uint32_t mi_flags;
	int32_t mi_tsize;
	int32_t mi_stsize;
	int32_t mi_timeo;
	int32_t mi_retrans;
	char mi_hostname[32];
	KA_T mi_netname;
	int mi_netnamelen;
	int mi_authflavor;
	int32_t mi_acregmin;
	int32_t mi_acregmax;
	int32_t mi_acdirmin;
	int32_t mi_acdirmax;
	struct rpc_timers mi_timers[4];
	int32_t mi_curread;
	int32_t mi_curwrite;
	KA_T mi_async_reqs;
	KA_T mi_async_tail;
	kcondvar_t mi_async_reqs_cv;
	uint16_t mi_threads;
	uint16_t mi_max_threads;
	kcondvar_t mi_async_cv;
	uint32_t mi_async_count;
	kmutex_t mi_async_lock;
	KA_T mi_pathconf;
	u_long mi_prog;
	u_long mi_vers;
	KA_T mi_rfsnames;
	KA_T mi_reqs;
	KA_T mi_call_type;
	KA_T mi_timer_type;
	clock_t mi_printftime;
	KA_T mi_aclnames;
	KA_T mi_aclreqs;
	KA_T mi_acl_call_type;
	KA_T mi_acl_timer_type;
	char mi_fsmnt[512];
	uint64_t mi_maxfilesize;
	dev_t mi_mntno;			/* mounted device number */
} mntinfo_t;

#endif	/* !defined(LSOF_NFS_CLNT_H) */
