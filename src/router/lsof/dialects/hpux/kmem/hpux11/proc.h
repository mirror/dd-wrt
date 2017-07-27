/*
 * proc.h for Hp-UX 10.30 and above
 *
 * This header file defines the proc structure for lsof.  Lsof uses it to
 * get process information, including PGID, PID, PPID, UID, CWD, and open
 * file pointers.
 *
 * V. Abell <abe@purdue.edu>
 * February, 1998
 */

#if	!defined(LSOF_PROC_H)
#define	LSOF_PROC_H

#include "kernbits.h"
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/user.h>

struct pprof {
	KA_T pr_base;
	u_long pr_size;
	u_long pr_off;
	u_int pr_scale;
};

typedef enum proc_flag {
	SLOAD = 0x1,
	SSYS = 0x2,
	SDONTTRC = 0x4,
	STRC = 0x8,
	SWTED_PARENT = 0x10,
	SDEACTSELF = 0x20,
	SPVFORK = 0x40,
	SWEXIT = 0x80,
	SPGID_EXIT_ADJUSTED = 0x100,
	SVFORK = 0x200,
	SWANTS_ALLCPU = 0x400,
	SSERIAL = 0x800,
	SDEACT = 0x1000,
	SWAITIO = 0x2000,
	SWTED_DEBUGGER = 0x4000,
	SWCONT = 0x8000,
	SDBG_CREATING = 0x10000,
	SDBG_WAITING = 0x20000,
	SDBG_ACTIVE = 0x40000,
	SDBG_LIMBO = 0x80000,
	SDBG_ATTACHING = 0x100000,
	SDBG_EXITING = 0x200000,
	SDBG_KILLED = 0x400000,
	SDBG_INEXEC = 0x800000,
	SDBG_TRACESELF = 0x1000000,
	SDBG_STOPPED = 0x2000000,
	SDBG_EXITREQ = 0x4000000,
	SREAPING = 0x10000000
} proc_flag_t;

typedef enum proc_flag2 {
	S2CLDSTOP = 0x1,
	S2EXEC = 0x2,
	SGRAPHICS = 0x4,
	SADOPTIVE = 0x8,
	SADOPTIVE_WAIT = 0x10,
	SPMT = 0x40,
	S2SENDDILSIG = 0x100,
	SLKDONE = 0x200,
	SISNFSLM = 0x400,
	S2POSIX_NO_TRUNC = 0x800,
	S2SYSCALL_BYPID = 0x1000,
	S2ADOPTEE = 0x2000,
	SCRITICAL = 0x4000,
	SMULTITHREADED = 0x8000,
	S2NOCLDWAIT = 0x10000,
	S_USE_THRD_CACHE = 0x20000,
	S2PASS_VIOREF = 0x40000,
	S2VIOREF_NPROC = 0x80000,
	SUSRMULTITHREADED = 0x100000
} proc_flag2_t;

typedef enum proc_state {
	SUNUSED = 0,
	SWAIT = 0x1,
	SIDL = 0x2,
	SZOMB = 0x3,
	SSTOP = 0x4,
	SINUSE = 0x5
} proc_state_t;

typedef enum proc_sync_flag {
	P_OP_PENDING_READER = 0x1,
	P_OP_PENDING_WRITER = 0x2
} proc_sync_flag_t;

typedef enum proc_sync_reason {
	P_OP_NONE = 0,
	P_OP_THREAD_MGMT = 0x1,
	P_OP_EXIT = 0x2,
	P_OP_EXEC = 0x3,
	P_OP_SUSPEND = 0x4,
	P_OP_CONTINUE = 0x5,
	P_OP_SIGTRAP = 0x6,
	P_OP_FORK = 0x7,
	P_OP_VFORK = 0x8,
	P_OP_CORE = 0x9,
	KT_OP_SUSPEND = 0xa,
	KT_OP_RESUME = 0xb,
	KT_OP_CREATE = 0xc,
	KT_OP_TERMINATE = 0xd,
	KT_OP_LWPEXIT = 0xe,
	KT_OP_ABORT_SYSCALL = 0xf
} proc_sync_reason_t;

typedef struct proc {
	short p_fandx;
	short p_pandx;
	int p_created_threads;
	KA_T p_firstthreadp;		/* thread pointer (for locks) */
	KA_T p_lastthreadp;
	proc_flag_t p_flag;
	KA_T thread_lock;
	KA_T p_lock;
	KA_T p_detached_zombie;
	KA_T p_fss;
	proc_state_t p_stat;		/* process state */
	char p_nice;
	u_short p_pri;
	int p_livethreads;
	int p_cached_threads_count;
	int p_cached_threads_max;
	KA_T p_cached_threads;
	KA_T p_cache_next;
	KA_T p_cache_prev;
	ksigset_t p_sig;
	ksigset_t p_ksi_avail;
	ksigset_t p_ksifl_alloced;
	KA_T p_ksiactive;
	KA_T p_ksifree;
	KA_T p_sigcountp;
	KA_T p_sigwaiters;
	int p_cursig;
	proc_flag2_t p_flag2;
	int p_coreflags;
	uid_t p_uid;			/* user ID (UID) of process owner */
	uid_t p_suid;
	KA_T p_pgid_p;
	gid_t p_pgid;
	pid_t p_pid;			/* process ID (PID) */
	pid_t p_ppid;			/* parent process ID (PPID) */
	size_t p_maxrss;
	short p_idhash;
	short p_ridhash;
	short p_pgidhx;
	short p_rpgidhx;
	short p_uidhx;
	short p_ruidhx;
	KA_T p_pptr;
	KA_T p_cptr;
	KA_T p_osptr;
	KA_T p_ysptr;
	KA_T p_dptr;
	KA_T p_vas;			/* pointer to VM for process */
	short p_memresv;
	short p_swpresv;
	short p_sysmemresv;
	short p_sysswpresv;
	u_short p_xstat;
	time_t p_deactime;
	short p_ndx;
	sid_t p_sid;
	short p_sidhx;
	short p_rsidhx;
	short p_idwrite;
	KA_T p_semundo;
	KA_T p_dbipcp;
	u_char p_cookie;
	u_char p_reglocks;
	int p_no_swap_count;
	dev_t p_ttyd;
	KA_T p_ttyp;
	KA_T p_nextdeact;
	time_t p_start;
	KA_T p_shadproc;
	KA_T p_bor_lock;
	int p_maxof;			/* maximum open files */
	KA_T p_cdir;			/* pointer to CWD vnode */
	KA_T p_rdir;			/* pointer to root directory vnode */
	KA_T p_ofilep;			/* pointer to ofile_t chain */
	KA_T p_vforkbuf;
	u_int p_schedpolicy;
	u_short p_pindx;
	KA_T p_krusagep;
	KA_T p_timers;
	KA_T p_clic;
	proc_sync_reason_t p_sync_reason;
	void (*p_wide_action_hdlr)();
	proc_sync_flag_t p_sync_flag;
	ushort p_sync_readers;
	ushort p_sync_writers;
	u_int p_sync_thread_cnt;
	int p_suspended_threads;
	int p_captr;
	union {
	    struct {
		u_int zombies_exist:1,
		      recalc_privgrps:1,
		      unused:30;
	    } bits;
		u_int all;
	} p_pl_flags;
	u_int p_seqnum;
	spu_t p_spu_group;
	u_char p_spu_mandatory;
	KA_T p_cred;
	caddr_t p_ki_bitmap;
	KA_T p_aioqp;
	KA_T p_shared;
	KA_T p_nseminfop;
	KA_T p_mqpinfop;
	KA_T p_dbgctltp;
	KA_T p_dbgp;
	KA_T p_trcp;
	KA_T p_p2p;
	KA_T p_gang;
	u_int p_pmon_timer_mask;
	u_int p_pmon_inherit;
	u_long p_pmon_state_flag;
	u_long p_pmon_state_value;
	KA_T p_cnx_features;
	char p_comm[15];
	aid_t p_aid;
	short p_audproc;
	short p_audsusp;
	gid_t p_sgid;
	u_int p_priv[2];
	int p_highestfd;
	short p_cmask;
	time_t p_ticks;
	short p_acflag;
	struct rlimit p_rlimit[11];
	KA_T p_auditperproc;
	struct pprof p_prof;
	char p_spare[48];
} proc_t;

#endif	/* !defined(LSOF_PROC_H) */
