//==========================================================================
//
//      include/machine/param.h
//
//      Architecture/platform specific parameters
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


#ifndef _MACHINE_PARAM_H_
#define _MACHINE_PARAM_H_

#include <pkgconf/net.h>

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than CLBYTES (the software page size), and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define MSIZE           128             /* size of an mbuf */
#define MCLSHIFT        11              /* convert bytes to m_buf clusters */
#define MCLBYTES        (1 << MCLSHIFT) /* size of a m_buf cluster */
#define MCLOFSET        (MCLBYTES - 1)  /* offset within a m_buf cluster */
#define CLBYTES         4096            /* size of actual cluster */

/*
 * Round p (pointer or byte index) up to a correctly-aligned value
 * for all data types (int, long, ...).   The result is u_int and
 * must be cast to any desired pointer type.
 */
#define ALIGNBYTES      (sizeof(int) - 1)
#define ALIGN(p)        (((u_int)(p) + ALIGNBYTES) &~ ALIGNBYTES)

// These symbols are used in the IPV6 stuff
// (be more defensive about external setup)
#ifdef __linux__
#undef __linux__
#endif
#ifdef __bsdi__
#undef __bsdi__
#endif
#ifdef __FreeBSD__
#undef __FreeBSD__
#endif
#ifdef __OpenBSD__
#undef __OpenBSD__
#endif
#ifdef __NetBSD__
#undef __NetBSD__
#endif


#define __linux__   0
#define __bsdi__    0
#define __FreeBSD__ 0
#define __OpenBSD__ 1
#define __NetBSD__  0

// These definitions here to avoid needing <sys/systm.h>
// This probably doesn't belong here, but we need these definitions
#include <lib/libkern/libkern.h>
#define SCARG(p,k)      ((p)->k.datum)  /* get arg from args pointer */
#include <stdarg.h>

// TEMP

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include <cyg/io/file.h>

struct net_stats {
    int              count;
    cyg_uint32       min_time, max_time, total_time;
};

#ifdef CYGDBG_NET_TIMING_STATS
#define START_STATS()                                   \
    cyg_uint32 start_time, end_time, elapsed_time;      \
    HAL_CLOCK_READ(&start_time);
#define FINISH_STATS(stats)                                                             \
    HAL_CLOCK_READ(&end_time);                                                          \
    if (end_time < start_time) {                                                        \
        elapsed_time = (end_time+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD) - start_time;       \
    } else {                                                                            \
        elapsed_time = end_time - start_time;                                           \
    }                                                                                   \
    if (stats.min_time == 0) {                                                          \
        stats.min_time = 0x7FFFFFFF;                                                    \
    }                                                                                   \
    if (elapsed_time < stats.min_time)                                                  \
        stats.min_time = elapsed_time;                                                  \
    if (elapsed_time > stats.max_time)                                                  \
        stats.max_time = elapsed_time;                                                  \
    stats.total_time += elapsed_time;                                                   \
    stats.count++;
#else
#define START_STATS() 
#define FINISH_STATS(X)
#endif

// timeout support
typedef void (timeout_fun)(void *);
extern cyg_uint32 timeout(timeout_fun *fun, void *arg, cyg_int32 delta);
extern void untimeout(timeout_fun *fun, void *arg);
extern int uiomove(caddr_t cp, int n, struct uio *uio);
extern int copyout(const void *s, void *d, size_t len);
extern int copyin(const void *s, void *d, size_t len);
extern void ovbcopy(const void *s, void *d, size_t len);
extern void get_mono_time(void);
extern int arc4random(void);
extern void get_random_bytes(void *buf, size_t len);

extern void *hashinit(int elements, int type, int flags, u_long *hashmask);

#endif // _MACHINE_PARAM_H_
