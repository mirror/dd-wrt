#ifndef NDPI_MAIN_COMMON_H
#define NDPI_MAIN_COMMON_H
#include <linux/atomic.h>
#include <asm/atomic.h>

#ifndef  !defined(arch_atomic_fetch_add) && !defined(atomic_fetch_add)
#define atomic_fetch_add __sync_fetch_and_add
#endif

struct ndpi_net;

NDPI_STATIC int inet_ntop_port(int family,void *ip, u_int16_t port, char *lbuf, size_t bufsize);
NDPI_STATIC int ndpi_delete_acct(struct ndpi_net *n,int all);
NDPI_STATIC ssize_t nflow_read(struct ndpi_net *n, char __user *buf,
	            size_t count, loff_t *ppos);
NDPI_STATIC int dbg_ipt_opt(char *lbuf,size_t count);
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
NDPI_STATIC uint32_t dbg_ipt_opt_get(const char *lbuf);
#endif

enum dbg_trace {
	DBG_TRACE_CT,
	DBG_TRACE_PKT,
	DBG_TRACE_NAT,
	DBG_TRACE_CNT,
	DBG_TRACE_FLOW_MEM,
	DBG_TRACE_HOSTNM,
	DBG_TRACE_EXCLUDE,
	DBG_TRACE_TLS,
	DBG_TRACE_JA3,
	DBG_TRACE_JA3MATCH,
	DBG_TRACE_CACHE,
	DBG_TRACE_MATCH,
	DBG_TRACE_MATCH2,
	DBG_TRACE_GUESSED,
	DBG_TRACE_DDONE,
	DBG_TRACE_DPI,
	DBG_TRACE_RE,
	DBG_TRACE_TG1,
	DBG_TRACE_TG2,
	DBG_TRACE_TG3,
	DBG_TRACE_SPROC,
	DBG_TRACE_SPROC_H,
	DBG_TRACE_SPROC_I,
	DBG_TRACE_GPROC,
	DBG_TRACE_GPROC_H,
	DBG_TRACE_GPROC_H2,
	DBG_TRACE_GPROC_I,
	DBG_TRACE_MATCH_CMD,
	DBG_TRACE_NETNS
};

#define _DBG_TRACE_CT (ndpi_log_debug & (1 << DBG_TRACE_CT))
#define _DBG_TRACE_PKT (ndpi_log_debug & (1 << DBG_TRACE_PKT))
#define _DBG_TRACE_NAT (ndpi_log_debug & (1 << DBG_TRACE_NAT))
#define _DBG_TRACE_CNT (ndpi_log_debug & (1 << DBG_TRACE_CNT))
#define _DBG_TRACE_FLOW_MEM (ndpi_log_debug & (1 << DBG_TRACE_FLOW_MEM))
#define _DBG_TRACE_HOSTNM (ndpi_log_debug & (1 << DBG_TRACE_HOSTNM))
#define _DBG_TRACE_EXCLUDE (ndpi_log_debug & (1 << DBG_TRACE_EXCLUDE))
#define _DBG_TRACE_TLS (ndpi_log_debug & (1 << DBG_TRACE_TLS))
#define _DBG_TRACE_JA3 (ndpi_log_debug & (1 << DBG_TRACE_JA3))
#define _DBG_TRACE_JA3MATCH (ndpi_log_debug & (1 << DBG_TRACE_JA3MATCH))
#define _DBG_TRACE_CACHE (ndpi_log_debug & (1 << DBG_TRACE_CACHE))
#define _DBG_TRACE_MATCH (ndpi_log_debug & (1 << DBG_TRACE_MATCH))
#define _DBG_TRACE_MATCH2 (ndpi_log_debug & (1 << DBG_TRACE_MATCH2))
#define _DBG_TRACE_GUESSED (ndpi_log_debug & (1 << DBG_TRACE_GUESSED))
#define _DBG_TRACE_DDONE (ndpi_log_debug & (1 << DBG_TRACE_DDONE))
#define _DBG_TRACE_DPI (ndpi_log_debug & (1 << DBG_TRACE_DPI))
#define _DBG_TRACE_RE (ndpi_log_debug & (1 << DBG_TRACE_RE))
#define _DBG_TRACE_TG1 (ndpi_log_debug & (1 << DBG_TRACE_TG1))
#define _DBG_TRACE_TG2 (ndpi_log_debug & (1 << DBG_TRACE_TG2))
#define _DBG_TRACE_TG3 (ndpi_log_debug & (1 << DBG_TRACE_TG3))
#define _DBG_TRACE_SPROC (ndpi_log_debug & (1 << DBG_TRACE_SPROC))
#define _DBG_TRACE_SPROC_H (ndpi_log_debug & (1 << DBG_TRACE_SPROC_H))
#define _DBG_TRACE_SPROC_I (ndpi_log_debug & (1 << DBG_TRACE_SPROC_I))
#define _DBG_TRACE_GPROC (ndpi_log_debug & (1 << DBG_TRACE_GPROC))
#define _DBG_TRACE_GPROC_H (ndpi_log_debug & (1 << DBG_TRACE_GPROC_H))
#define _DBG_TRACE_GPROC_H2 (ndpi_log_debug & (1 << DBG_TRACE_GPROC_H2))
#define _DBG_TRACE_GPROC_I (ndpi_log_debug & (1 << DBG_TRACE_GPROC_I))
#define _DBG_TRACE_MATCH_CMD (ndpi_log_debug & (1 << DBG_TRACE_MATCH_CMD))
#define _DBG_TRACE_NETNS (ndpi_log_debug & (1 << DBG_TRACE_NETNS))

#include "../lib/third_party/include/ahocorasick.h"
NDPI_STATIC AC_ERROR_t      ac_automata_add_exact(AC_AUTOMATA_t *, AC_PATTERN_t *);

/*extern unsigned long  ndpi_flow_limit;
extern unsigned long  bt_hash_size;
extern unsigned long  bt6_hash_size;
extern unsigned long  bt_hash_tmo;
extern unsigned long  ndpi_enable_flow;
extern unsigned long  flow_read_debug;
extern unsigned long  ndpi_log_debug;
*/
#define NDPI_FLOW_OPT_MAX 7
//extern char  ndpi_flow_opt[NDPI_FLOW_OPT_MAX+1];
//extern struct kmem_cache *ct_info_cache;

#define XCHGP(a,b) { void *__c = a; a = b; b = __c; }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
#define ACCESS_OK(a,b,c) access_ok(b,c)
#else
#define ACCESS_OK(a,b,c) access_ok(a,b,c)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
static inline void getnstimeofday64(struct timespec64 *ts) {
	ktime_get_real_ts64(ts);
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
static inline time64_t ktime_get_seconds(void)
{
	struct timespec t;

	ktime_get_ts(&t);

	return t.tv_sec;
}

static inline time64_t ktime_get_real_seconds(void)
{
	struct timeval tv;

	do_gettimeofday(&tv);

	return tv.tv_sec;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
#define pde_data(inode) PDE_DATA(inode)
#endif
#endif