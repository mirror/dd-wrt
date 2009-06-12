//==========================================================================
//
//      src/ecos/support.c
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// Copyright (C) 2002, 2003 Gary Thomas
// Copyright (C) 2003 Andrew Lunn
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

//==========================================================================
//
//      ecos/support.c
//
//      eCos wrapper and support functions
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
// Author(s):    gthomas, hmt
// Contributors: gthomas, hmt, andrew.lunn@ascom.ch
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


// Support routines, etc., used by network code

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/sockio.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/if.h>
#include <net/netisr.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_if.h>
#include <cyg/infra/cyg_ass.h>

#ifdef CYGPKG_NET_INET6
#include <netinet/in.h>
#include <netinet6/in6_var.h>
#endif

#if !CYGPKG_NET_DRIVER_FRAMEWORK   // Interface
#error At least one network driver framework must be defined!
#else
#include <cyg/io/eth/netdev.h>

// Define table boundaries
CYG_HAL_TABLE_BEGIN( __NETDEVTAB__, netdev );
CYG_HAL_TABLE_END( __NETDEVTAB_END__, netdev );
CYG_HAL_TABLE_BEGIN( __NET_INIT_TAB__, _Net_inits );
CYG_HAL_TABLE_END( __NET_INIT_TAB_END__, _Net_inits );
extern struct init_tab_entry __NET_INIT_TAB__[], __NET_INIT_TAB_END__;

// Used for system-wide "ticks per second"
#undef ticks
int hz = 100;
int tick = 10000;  // usec per "tick"
volatile struct timeval ktime;
int proc = 0;  // unused
int proc0 = 0;  // unused

volatile struct timeval mono_time;

// Low-level network debugging & logging
#ifdef CYGPKG_NET_FREEBSD_LOGGING
int cyg_net_log_mask = CYGPKG_NET_FREEBSD_LOGGING;
#endif

#ifdef CYGPKG_NET_INET6
#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL+2048)
#else
#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
#endif
static char netint_stack[STACK_SIZE];
static cyg_thread netint_thread_data;
static cyg_handle_t netint_thread_handle;

cyg_flag_t netint_flags;  
#define NETISR_ANY 0xFFFFFFFF  // Any possible bit...

extern void cyg_test_exit(void);  // TEMP
void
cyg_panic(const char *msg, ...)
{
    cyg_uint32 old_ints;
    CYG_FAIL( msg );
    HAL_DISABLE_INTERRUPTS(old_ints);
    diag_printf("PANIC: %s\n", msg);
    cyg_test_exit();  // FIXME
}

#define NET_MEMPOOL_SIZE  roundup(CYGPKG_NET_MEMPOOL_SIZE,MSIZE)
#define NET_MBUFS_SIZE    roundup(CYGPKG_NET_MBUFS_SIZE,MSIZE)
#define NET_CLUSTERS_SIZE roundup(CYGPKG_NET_CLUSTERS_SIZE,MCLBYTES)

static unsigned char net_mempool_area[NET_MEMPOOL_SIZE];
static cyg_mempool_var net_mem_pool;
static cyg_handle_t    net_mem;
static unsigned char net_mbufs_area[NET_MBUFS_SIZE];
static cyg_mempool_fix net_mbufs_pool;
static cyg_handle_t    net_mbufs;
static unsigned char net_clusters_area[NET_CLUSTERS_SIZE];
static cyg_mempool_fix net_clusters_pool;
static cyg_handle_t    net_clusters;
static char            net_clusters_refcnt[(NET_CLUSTERS_SIZE/MCLBYTES)+1];
int nmbclusters = (NET_CLUSTERS_SIZE/MCLBYTES);

#ifdef CYGDBG_NET_TIMING_STATS
static struct net_stats  stats_malloc, stats_free, 
    stats_memcpy, stats_memset,
    stats_mbuf_alloc, stats_mbuf_free, stats_cluster_alloc;

// Display a number of ticks as microseconds
// Note: for improved calculation significance, values are kept in ticks*1000
static long rtc_resolution[] = CYGNUM_KERNEL_COUNTERS_RTC_RESOLUTION;
static long ns_per_system_clock;

static void
show_ticks_in_us(cyg_uint32 ticks)
{
    long long ns;
    ns_per_system_clock = 1000000/rtc_resolution[1];
    ns = (ns_per_system_clock * ((long long)ticks * 1000)) / 
        CYGNUM_KERNEL_COUNTERS_RTC_PERIOD;
    ns += 5;  // for rounding to .01us
    diag_printf("%7d.%02d", (int)(ns/1000), (int)((ns%1000)/10));
}

void
show_net_stats(struct net_stats *stats, const char *title)
{
    int ave;
    ave = stats->total_time / stats->count;
    diag_printf("%s:\n", title);
    diag_printf("  count: %6d", stats->count);
    diag_printf(", min: ");
    show_ticks_in_us(stats->min_time);
    diag_printf(", max: ");
    show_ticks_in_us(stats->max_time);
    diag_printf(", total: ");
    show_ticks_in_us(stats->total_time);
    diag_printf(", ave: ");
    show_ticks_in_us(ave);
    diag_printf("\n");
    // Reset stats
    memset(stats, 0, sizeof(*stats));
}

void
show_net_times(void)
{
    show_net_stats(&stats_malloc,        "Net malloc");
    show_net_stats(&stats_free,          "Net free");
    show_net_stats(&stats_mbuf_alloc,    "Mbuf alloc");
    show_net_stats(&stats_mbuf_free,     "Mbuf free");
    show_net_stats(&stats_cluster_alloc, "Cluster alloc");
    show_net_stats(&stats_memcpy,        "Net memcpy");
    show_net_stats(&stats_memset,        "Net memset");
}
#endif /* CYGDBG_NET_TIMING_STATS */ 

void *
cyg_net_malloc(u_long size, int type, int flags)
{
    void *res;

    START_STATS();
    log(LOG_MDEBUG, "Net malloc[%d] = ", size);
    if (flags & M_NOWAIT) {
        res = cyg_mempool_var_try_alloc(net_mem, size);
    } else {
        res = cyg_mempool_var_alloc(net_mem, size);
    }
    if ((flags & M_ZERO) && res) {
      memset(res,0,size);
    }
    FINISH_STATS(stats_malloc);
    log(LOG_MDEBUG, "%p\n", res);
    return (res);
}

void 
cyg_net_free(caddr_t addr, int type)
{
    START_STATS();
    cyg_mempool_var_free(net_mem, addr);
    FINISH_STATS(stats_free);
}

#ifdef CYGDBG_NET_SHOW_MBUFS

struct mbuf *mbinfo[300];

void cyg_net_show_mbufs(void)
{
    int i;
    diag_printf(" MBUF   : TYPE FLGS     DATA[LEN]   NEXT    NEXTPKT\n");
    for( i = 0; i < 300; i++ )
    {
        struct mbuf *m = mbinfo[i];
        char *type;
        if( m == 0 ) continue;

        switch( m->m_hdr.mh_type )
        {
        case MT_FREE: type="FREE"; break;
        case MT_DATA: type="DATA"; break;
        case MT_HEADER: type="HEADER"; break;
        case MT_SONAME: type="SONAME"; break;
        case MT_FTABLE: type="FTABLE"; break;
        case MT_CONTROL: type="CONTROL"; break;
        case MT_OOBDATA: type="OOBDATA"; break;
        default: type="UNKNOWN"; break;
        }

        diag_printf("%08x: %s %04x %08x[%03d] %08x %08x\n",
                    m, type,
                    m->m_hdr.mh_flags,
                    m->m_hdr.mh_data,
                    m->m_hdr.mh_len,
                    m->m_hdr.mh_next,
                    m->m_hdr.mh_nextpkt);
    }
    diag_printf(" MBUF   : TYPE FLGS     DATA[LEN]   NEXT    NEXTPKT\n");    
}

#endif

void *
cyg_net_mbuf_alloc(void)
{
    void *res;    

    START_STATS();
    log(LOG_MDEBUG, "Alloc mbuf = ");
    res = cyg_mempool_fix_try_alloc(net_mbufs);
FINISH_STATS(stats_mbuf_alloc);
#ifdef CYGDBG_NET_SHOW_MBUFS    
    {
        int i;
        for( i = 0; i < (sizeof(mbinfo)/sizeof(mbinfo[0])); i++ )
            if( mbinfo[i] == 0 )
            {
                mbinfo[i] = (struct mbuf *)res;
                break;
            }
    }
#endif
    // Check that this nastiness works OK
    CYG_ASSERT( dtom(res) == res, "dtom failed, base of mbuf" );
    CYG_ASSERT( dtom((char *)res + MSIZE/2) == res, "dtom failed, mid mbuf" );
    log(LOG_MDEBUG, "%p\n", res);
    return (res);
}

void *
cyg_net_cluster_alloc(void)
{
    void *res;

    START_STATS();
    log(LOG_MDEBUG, "Allocate cluster = ");
    res = cyg_mempool_fix_try_alloc(net_clusters);
    FINISH_STATS(stats_cluster_alloc);
    log(LOG_MDEBUG, "%p\n", res);
    return res;
}

static struct vm_zone *vm_zones = (struct vm_zone *)NULL;

vm_zone_t 
zinit(char *name, int size, int nentries, int flags, int zalloc)
{
    void *res;
    vm_zone_t zone = (vm_zone_t)0;
    elem *p;

    log(LOG_MDEBUG, "zinit '%s', size: %d, num: %d, flags: %d, alloc: %d\n", 
        name, size, nentries, flags, zalloc);
    res = cyg_mempool_var_try_alloc(net_mem, sizeof(struct vm_zone));
    if (res) {
        zone = (vm_zone_t)res;
        res = cyg_mempool_var_try_alloc(net_mem, size*nentries);
    }
    if (!res) {
        log(LOG_MDEBUG, "Can't allocate memory for %s\n", name);
        panic("zinit: Out of memory\n");
    }
    p = (elem *)res;
    zone->pool = (elem *)0;
    zone->elem_size = size;
    zone->name = name;
    zone->free = zone->total = nentries;
    zone->next = vm_zones;
    zone->alloc_tries = zone->alloc_fails = zone->alloc_frees = 0;
    vm_zones = zone;
    while (nentries-- > 0) {
        p->next = zone->pool;
        zone->pool = p;
        p = (elem *)((char *)p + size);
    }
    p = zone->pool;
#if 0
    while (p) {
        log(LOG_MDEBUG, "p: %p, next: %p\n", p, p->next);
        p = p->next;
    }
#endif
    return zone;
}

void *    
zalloci(vm_zone_t zone)
{
    elem *p;

    p = zone->pool;
    zone->alloc_tries++;
    if (p) {
        zone->pool = p->next;
        zone->free--;        
    } else {
        zone->alloc_fails++;
    }
    log(LOG_MDEBUG, "zalloci from %s => %p\n", zone->name, p);
    return (void *)p;
}

void      
zfreei(vm_zone_t zone, void *item)
{
    elem *p = (elem *)item;

    log(LOG_MDEBUG, "zfreei to %s <= %p\n", zone->name, p);
    p->next = zone->pool;
    zone->pool = p;
    zone->free++;
    zone->alloc_frees++;
}

static void
cyg_kmem_init(void)
{
    unsigned char *p;
#ifdef CYGPKG_NET_DEBUG
    diag_printf("Network stack using %d bytes for misc space\n", NET_MEMPOOL_SIZE);
    diag_printf("                    %d bytes for mbufs\n", NET_MBUFS_SIZE);
    diag_printf("                    %d bytes for mbuf clusters\n", NET_CLUSTERS_SIZE);
#endif
    cyg_mempool_var_create(&net_mempool_area,
                           NET_MEMPOOL_SIZE,
                           &net_mem,
                           &net_mem_pool);
    // Align the mbufs on MSIZE boudaries so that dtom() can work.
    p = (unsigned char *)(((long)(&net_mbufs_area) + MSIZE - 1) & ~(MSIZE-1));
    cyg_mempool_fix_create(p,
                           ((&(net_mbufs_area[NET_MBUFS_SIZE])) - p) & ~(MSIZE-1),
                           MSIZE,
                           &net_mbufs,
                           &net_mbufs_pool);
    cyg_mempool_fix_create(&net_clusters_area,
                           NET_CLUSTERS_SIZE,
                           MCLBYTES,
                           &net_clusters,
                           &net_clusters_pool);
    mbutl = (struct mbuf *)&net_clusters_area;
    mclrefcnt = net_clusters_refcnt;
}

void cyg_kmem_print_stats( void )
{
    cyg_mempool_info info;
    struct vm_zone *zone;

    diag_printf( "Network stack mbuf stats:\n" );
    diag_printf( "   mbufs %d, clusters %d, free clusters %d\n",
                 mbstat.m_mbufs,	/* mbufs obtained from page pool */
                 mbstat.m_clusters,	/* clusters obtained from page pool */
                 /* mbstat.m_spare, */	/* spare field */
                 mbstat.m_clfree	/* free clusters */
        );
    diag_printf( "   Failed to get %d times\n"
                 "   Waited to get %d times\n"
                 "   Drained queues to get %d times\n",
                 mbstat.m_drops,	/* times failed to find space */
                 mbstat.m_wait, 	/* times waited for space */
                 mbstat.m_drain         /* times drained protocols for space */
                 /* mbstat.m_mtypes[256]; type specific mbuf allocations */
        );

    zone = vm_zones;
    while (zone) {
        diag_printf("VM zone '%s':\n", zone->name);
        diag_printf("  Total: %d, Free: %d, Allocs: %d, Frees: %d, Fails: %d\n",
                    zone->total, zone->free,
                    zone->alloc_tries, zone->alloc_frees, zone->alloc_fails);
        zone = zone->next;
    }

    cyg_mempool_var_get_info( net_mem, &info );
    diag_printf( "Misc mpool: total %7d, free %7d, max free block %d\n",
                 info.totalmem,
                 info.freemem,
                 info.maxfree
        );

    cyg_mempool_fix_get_info( net_mbufs, &info );
    diag_printf( "Mbufs pool: total %7d, free %7d, blocksize %4d\n",
                 info.totalmem,
                 info.freemem,
                 info.blocksize
        );


    cyg_mempool_fix_get_info( net_clusters, &info );
    diag_printf( "Clust pool: total %7d, free %7d, blocksize %4d\n",
                 info.totalmem,
                 info.freemem,
                 info.blocksize
        );
}

// This API is for our own automated network tests.  It's not in any header
// files because it's not at all supported.
int cyg_net_get_mem_stats( int which, cyg_mempool_info *p )
{
    CYG_CHECK_DATA_PTR( p, "Bad pointer to mempool_info" );
    CYG_ASSERT( 0 <= which, "Mempool selector underflow" );
    CYG_ASSERT( 2 >=which, "Mempool selector overflow" );
    
    if ( p )
        switch ( which ) {
        case 0:
            cyg_mempool_var_get_info( net_mem, p );
            break;
        case 1:
            cyg_mempool_fix_get_info( net_mbufs, p );
            break;
        case 2:
            cyg_mempool_fix_get_info( net_clusters, p );
            break;
        default:
            return 0;
        }
    return (int)p;
}

int
cyg_mtocl(u_long x)
{
    int res;
    res = (((u_long)(x) - (u_long)mbutl) >> MCLSHIFT);
    return res;
}

struct mbuf *
cyg_cltom(u_long x)
{
    struct mbuf *res;
    res = (struct mbuf *)((caddr_t)((u_long)mbutl + ((u_long)(x) << MCLSHIFT)));
    return res;
}

externC void 
net_memcpy(void *d, void *s, int n)
{
    START_STATS();
    memcpy(d, s, n);
    FINISH_STATS(stats_memcpy);
}

externC void 
net_memset(void *s, int v, int n)
{
    START_STATS();
    memset(s, v, n);
    FINISH_STATS(stats_memset);
}

// Rather than bring in the whole BSD 'random' code...
int
arc4random(void)
{
    cyg_uint32 res;
    static unsigned long seed = 0xDEADB00B;
    HAL_CLOCK_READ(&res);  // Not so bad... (but often 0..N where N is small)
    seed = ((seed & 0x007F00FF) << 7) ^
        ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
        (res << 13) ^ (res >> 9);    // using the clock too!
    return (int)seed;
}

void 
get_random_bytes(void *buf, size_t len)
{
    unsigned long ranbuf, *lp;
    lp = (unsigned long *)buf;
    while (len > 0) {
        ranbuf = arc4random();
        *lp++ = ranbuf;
        len -= sizeof(ranbuf);
    }
}

int
read_random_unlimited(void *buf, size_t len)
{
    get_random_bytes(buf, len);
    return len;
}

void read_random(void *buf, size_t len) 
{
    CYG_ASSERT(0 == (len & ~3), "Only multiple of words allowed");
  
    get_random_bytes(buf, len);
}

void 
microtime(struct timeval *tp)
{
    *tp = ktime;
    log(LOG_DEBUG, "%s: = %d.%d\n", __FUNCTION__, tp->tv_sec, tp->tv_usec);
    ktime.tv_usec++;  // In case clock isn't running yet
}

void 
getmicrotime(struct timeval *tp)
{
    *tp = ktime;
    log(LOG_DEBUG, "%s: = %d.%d\n", __FUNCTION__, tp->tv_sec, tp->tv_usec);
    ktime.tv_usec++;  // In case clock isn't running yet
}

void 
getmicrouptime(struct timeval *tp)
{
    *tp = ktime;
    log(LOG_DEBUG, "%s: = %d.%d\n", __FUNCTION__, tp->tv_sec, tp->tv_usec);
    ktime.tv_usec++;  // In case clock isn't running yet
}

// Taken from kern/kern_clock.c
/*
 * Compute number of ticks in the specified amount of time.
 */
#ifndef LONG_MAX
#define LONG_MAX 0x7FFFFFFF
#endif
int
tvtohz(struct timeval *tv)
{
        register unsigned long ticks;
        register long sec, usec;

        /*
         * If the number of usecs in the whole seconds part of the time
         * difference fits in a long, then the total number of usecs will
         * fit in an unsigned long.  Compute the total and convert it to
         * ticks, rounding up and adding 1 to allow for the current tick
         * to expire.  Rounding also depends on unsigned long arithmetic
         * to avoid overflow.
         *
         * Otherwise, if the number of ticks in the whole seconds part of
         * the time difference fits in a long, then convert the parts to
         * ticks separately and add, using similar rounding methods and
         * overflow avoidance.  This method would work in the previous
         * case but it is slightly slower and assumes that hz is integral.
         *
         * Otherwise, round the time difference down to the maximum
         * representable value.
         *
         * If ints have 32 bits, then the maximum value for any timeout in
         * 10ms ticks is 248 days.
         */
        sec = tv->tv_sec;
        usec = tv->tv_usec;
        if (usec < 0) {
                sec--;
                usec += 1000000;
        }
        if (sec < 0) {
#ifdef DIAGNOSTIC
                if (usec > 0) {
                        sec++;
                        usec -= 1000000;
                }
                printf("tvotohz: negative time difference %ld sec %ld usec\n",
                       sec, usec);
#endif
                ticks = 1;
        } else if (sec <= LONG_MAX / 1000000)
                ticks = (sec * 1000000 + (unsigned long)usec + (tick - 1))
                        / tick + 1;
        else if (sec <= LONG_MAX / hz)
                ticks = sec * hz
                        + ((unsigned long)usec + (tick - 1)) / tick + 1;
        else
                ticks = LONG_MAX;
        if (ticks > INT_MAX)
                ticks = INT_MAX;
        return ((int)ticks);
}

void
get_mono_time(void)
{
    panic("get_mono_time");
}

void 
csignal(pid_t pgid, int signum, uid_t uid, uid_t euid)
{
    panic("csignal");
}

int
bcmp(const void *_p1, const void *_p2, size_t len)
{
    int res = 0;
    unsigned char *p1 = (unsigned char *)_p1;
    unsigned char *p2 = (unsigned char *)_p2;
    while (len-- > 0) {
        res = *p1++ - *p2++;
        if (res) break;
    }
    return res;
}

int
copyout(const void *s, void *d, size_t len)
{
    memcpy(d, s, len);
    return 0;
}

int
copyin(const void *s, void *d, size_t len)
{
    memcpy(d, s, len);
    return 0;
}

void
ovbcopy(const void *s, void *d, size_t len)
{
    memmove(d, s, len);
}


// ------------------------------------------------------------------------
// THE NETWORK THREAD ITSELF
//
// Network software interrupt handler
//   This function is run as a separate thread to allow
// processing of network events (mostly incoming packets)
// at "user level" instead of at interrupt time.
//
// The actual handlers are 'registered' at system startup
//

// The set of handlers
static netisr_t *_netisr_handlers[NETISR_MAX+1];
struct ifqueue  ipintrq;
#ifdef INET6
struct ifqueue  ip6intrq;
#endif
char *hostname = "eCos_node";

// Register a 'netisr' handler for a given level
int 
register_netisr(int level, netisr_t *fun)
{
    CYG_ASSERT(level <= NETISR_MAX, "invalid netisr level");
    CYG_ASSERT(_netisr_handlers[level] == 0, "re-registered netisr");
    _netisr_handlers[level] = fun;
    return 0;  // ignored
}

//int unregister_netisr __P((int));

static void
cyg_netint(cyg_addrword_t param)
{
    cyg_flag_value_t curisr;
    int lvl, spl;

    while (true) {
        curisr = cyg_flag_wait(&netint_flags, NETISR_ANY, 
                               CYG_FLAG_WAITMODE_OR|CYG_FLAG_WAITMODE_CLR);
        spl = splsoftnet(); // Prevent any overlapping "stack" processing
        for (lvl = NETISR_MIN;  lvl <= NETISR_MAX;  lvl++) {
            if (curisr & (1<<lvl)) {
                CYG_ASSERT(_netisr_handlers[lvl] != 0, "unregistered netisr handler");
                (*_netisr_handlers[lvl])();
            }
        }
        splx(spl);
    }
}


// This just sets one of the pseudo-ISR bits used above.
void
setsoftnet(void)
{
    // This is called if we are out of MBUFs - it doesn't do anything, and
    // that situation is handled OK, so don't bother with the diagnostic:

    // diag_printf("setsoftnet\n");

    // No need to do this because it is ignored anyway:
    // schednetisr(NETISR_SOFTNET);
}


/* Update the kernel globel ktime. */
static void 
cyg_ktime_func(cyg_handle_t alarm,cyg_addrword_t data)
{
    cyg_tick_count_t now = cyg_current_time();

    ktime.tv_usec = (now % hz) * tick;
    ktime.tv_sec = 1 + now / hz;
}

static void
cyg_ktime_init(void)
{
    cyg_handle_t ktime_alarm_handle;
    static cyg_alarm ktime_alarm;
    cyg_handle_t counter;

    // Do not start at 0 - net stack thinks 0 an invalid time;
    // Have a valid time available from right now:
    ktime.tv_usec = 0;
    ktime.tv_sec = 1;

    cyg_clock_to_counter(cyg_real_time_clock(),&counter);
    cyg_alarm_create(counter,
                     cyg_ktime_func,
                     0,
                     &ktime_alarm_handle,
                     &ktime_alarm);

    /* We want one alarm every 10ms. */
    cyg_alarm_initialize(ktime_alarm_handle,cyg_current_time()+1,1);
    cyg_alarm_enable(ktime_alarm_handle);
}

int
cyg_ticks(void)
{
    cyg_tick_count_t now = cyg_current_time();
    return (int)now;
}

//
// Network initialization
//   This function is called during system initialization to setup the whole
// networking environment.

// Linker magic to execute this function as 'init'
extern void cyg_do_net_init(void);

extern void ifinit(void);
extern void loopattach(int);
extern void bridgeattach(int);

// Internal init functions:
extern void cyg_alarm_timeout_init(void);
extern void cyg_tsleep_init(void);

static void
cyg_net_init_devs(void *ignored)
{
    cyg_netdevtab_entry_t *t;
    // Initialize all network devices
    for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
        log(LOG_INIT, "Init device '%s'\n", t->name);
        if (t->init(t)) {
            t->status = CYG_NETDEVTAB_STATUS_AVAIL;
        } else {
            // What to do if device init fails?
            t->status = 0;  // Device not [currently] available
        }
    }
#if 0  // Bridge code not available yet
#if NBRIDGE > 0
    bridgeattach(0);
#endif
#endif
}
SYSINIT(devs, SI_SUB_DEVICES, SI_ORDER_FIRST, cyg_net_init_devs, NULL)

void
cyg_net_init(void)
{
    static int _init = false;
    struct init_tab_entry *init_entry;
#ifdef CYGPKG_NET_FORCE_SERIAL_CONSOLE
    int orig_console =
        CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
#endif

    if (_init) return;

#ifdef CYGPKG_NET_FORCE_SERIAL_CONSOLE
    // Force default serial console during system initialization
    // This avoids problems with debug messages occurring while the
    // networking subsystem is being setup.
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
#endif

    cyg_do_net_init();  // Just forces linking in the initializer/constructor
    // Initialize interrupt "flags"
    cyg_flag_init(&netint_flags);
    // Initialize timeouts and net service thread (pseudo-DSRs)
    cyg_alarm_timeout_init();
    // Initialize tsleep/wakeup support
    cyg_tsleep_init();
    // Initialize network memory system
    cyg_kmem_init();
    // Initialize network time
    cyg_ktime_init();
    // Create network background thread
    cyg_thread_create(CYGPKG_NET_THREAD_PRIORITY, // Priority
                      cyg_netint,               // entry
                      0,                        // entry parameter
                      "Network support",        // Name
                      &netint_stack[0],         // Stack
                      STACK_SIZE,               // Size
                      &netint_thread_handle,    // Handle
                      &netint_thread_data       // Thread data structure
        );
    cyg_thread_resume(netint_thread_handle);    // Start it

    // Run through dynamic initializers
    for (init_entry = __NET_INIT_TAB__; init_entry != &__NET_INIT_TAB_END__;  init_entry++) {
        log(LOG_INIT, "[%s] Init: %s(%p)\n", __FUNCTION__, init_entry->name, init_entry->data);
        (*init_entry->fun)(init_entry->data);
    }
    log(LOG_INIT, "[%s] Done\n", __FUNCTION__);

    // Done
    _init = true;

#ifdef CYGPKG_NET_FORCE_SERIAL_CONSOLE
    // Revert to the original console, which might be a network connection
    CYGACC_CALL_IF_SET_CONSOLE_COMM(orig_console);
#endif
}


#include <net/if.h>
#include <net/netdb.h>
#include <net/route.h>
externC void if_indextoname(int indx, char *buf, int len);

typedef void pr_fun(char *fmt, ...);

static void
_mask(struct sockaddr *sa, char *buf, int _len)
{
    unsigned char *cp = ((char *)sa) + 4;
    int len = sa->sa_len - 4;
    int tot = 0;

    while (len-- > 0) {
        if (tot) *buf++ = '.';
        buf += diag_sprintf(buf, "%u", *cp++);
        tot++;
    }

    while (tot < 4) {
        if (tot) *buf++ = '.';
        buf += diag_sprintf(buf, "%d", 0);
        tot++;
    }
}

#ifdef CYGPKG_NET_INET6
static void
_mask6(struct sockaddr *sa, char *buf, int _len)
{
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
  int addrword = 0;
  int bits = 0;
  int index;

  while (addrword < 4) {
    if (sin6->sin6_addr.s6_addr32[addrword] == 0) {
      break;
    }
    HAL_LSBIT_INDEX(index, sin6->sin6_addr.s6_addr32[addrword++]);
    bits += (32-index);
    if (index != 0) {
      break;
    }
  }
  diag_sprintf(buf, "%d", bits);
}
#endif
static void
_show_ifp(struct ifnet *ifp, pr_fun *pr)
{
    struct ifaddr *ifa;
    char name[64], addr[64], netmask[64], broadcast[64];

    if_indextoname(ifp->if_index, name, 64);
    TAILQ_FOREACH(ifa, &ifp->if_addrlist, ifa_list) {
        if (ifa->ifa_addr->sa_family != AF_LINK) {
	    (*pr)("%-8s", name);
            getnameinfo (ifa->ifa_addr, ifa->ifa_addr->sa_len, addr, 
                         sizeof(addr), 0, 0, NI_NUMERICHOST);
	    if (ifa->ifa_addr->sa_family == AF_INET) {
	      getnameinfo (ifa->ifa_dstaddr, ifa->ifa_dstaddr->sa_len, 
                           broadcast, sizeof(broadcast), 
                           0, 0, NI_NUMERICHOST);
	      _mask(ifa->ifa_netmask, netmask, 64);
	      (*pr)("IP: %s, Broadcast: %s, Netmask: %s\n", 
                    addr, broadcast, netmask);
	    }
#ifdef CYGPKG_NET_INET6 
            if (ifa->ifa_addr->sa_family == AF_INET6) {
              struct in6_ifaddr * ifa6 = (struct in6_ifaddr *) ifa;
              _mask6(ifa->ifa_netmask, netmask, 64);
              (*pr)("IP: %s/%s   ", addr,netmask);
              if (ifa6->ia6_flags & IN6_IFF_ANYCAST) (*pr) ("Anycast ");
              if (ifa6->ia6_flags & IN6_IFF_TENTATIVE) (*pr) ("Tentative ");
              if (ifa6->ia6_flags & IN6_IFF_DUPLICATED) (*pr) ("Duplicate ");
              if (ifa6->ia6_flags & IN6_IFF_DETACHED) (*pr) ("Detached ");
              if (ifa6->ia6_flags & IN6_IFF_DEPRECATED) (*pr) ("Deprecated ");
              if (ifa6->ia6_flags & IN6_IFF_NODAD) (*pr) ("NoDAD ");
              if (ifa6->ia6_flags & IN6_IFF_AUTOCONF) (*pr) ("AutoConf ");
              if (ifa6->ia6_flags & IN6_IFF_TEMPORARY) (*pr) ("Tempory ");
              if (ifa6->ia6_flags & IN6_IFF_HOME) (*pr) ("Home ");
              (*pr) ("\n");
            }
#endif
            (*pr)("        ");
            if ((ifp->if_flags & IFF_UP)) (*pr)("UP ");
            if ((ifp->if_flags & IFF_BROADCAST)) (*pr)("BROADCAST ");
            if ((ifp->if_flags & IFF_LOOPBACK)) (*pr)("LOOPBACK ");
            if ((ifp->if_flags & IFF_RUNNING)) (*pr)("RUNNING ");
            if ((ifp->if_flags & IFF_PROMISC)) (*pr)("PROMISC ");
            if ((ifp->if_flags & IFF_MULTICAST)) (*pr)("MULTICAST ");
            if ((ifp->if_flags & IFF_ALLMULTI)) (*pr)("ALLMULTI ");
            (*pr)("MTU: %d, Metric: %d\n", ifp->if_mtu, ifp->if_metric);
            (*pr)("        Rx - Packets: %d, Bytes: %d", 
                  ifa->if_data.ifi_ipackets, ifa->if_data.ifi_ibytes);
            (*pr)(", Tx - Packets: %d, Bytes: %d\n", 
                  ifa->if_data.ifi_opackets, ifa->if_data.ifi_obytes);
        }
    }
}

static int
_dumpentry(struct radix_node *rn, void *vw)
{
    struct rtentry *rt = (struct rtentry *)rn;
    struct sockaddr *dst, *gate, *netmask, *genmask;
    char addr[64], *cp;
    pr_fun *pr = (pr_fun *)vw;

    dst = rt_key(rt);
    gate = rt->rt_gateway;
    netmask = rt_mask(rt);
    genmask = rt->rt_genmask;
    if ((rt->rt_flags & (RTF_UP | RTF_WASCLONED)) == RTF_UP) {
        _inet_ntop(dst, addr, sizeof(addr));
        (*pr)("%-15s ", addr);
        if (gate != NULL) {
            _inet_ntop(gate, addr, sizeof(addr));
            (*pr)("%-15s ", addr);
        } else {
            (*pr)("%-15s ", " ");
        }
        if (netmask != NULL) {
	    if (dst->sa_family == AF_INET) {
                _mask(netmask, addr, sizeof(addr));
                (*pr)("%-15s ", addr);
	    } 
#ifdef CYGPKG_NET_INET6
	    if (dst->sa_family == AF_INET6) {
	      _mask6(netmask, addr, sizeof(addr));
	      (*pr)("/%-14s ", addr);
	    }
#endif
        } else {
            (*pr)("%-15s ", " ");
        }
        cp = addr;
        if ((rt->rt_flags & RTF_UP)) *cp++ = 'U';
        if ((rt->rt_flags & RTF_GATEWAY)) *cp++ = 'G';
	if ((rt->rt_flags & RTF_HOST)) *cp++ = 'H';
	if ((rt->rt_flags & RTF_REJECT)) *cp++ = '!';
        if ((rt->rt_flags & RTF_STATIC)) *cp++ = 'S';
        if ((rt->rt_flags & RTF_DYNAMIC)) *cp++ = 'D';
	if ((rt->rt_flags & RTF_MODIFIED)) *cp++ = 'M';
        *cp = '\0';
        (*pr)("%-8s ", addr);  // Flags
        if_indextoname(rt->rt_ifp->if_index, addr, 64);
        (*pr)("%-8s ", addr);
        (*pr)("\n");
    }
    return 0;
}

void
show_network_tables(pr_fun *pr)
{
    int i, error;
    struct radix_node_head *rnh;
    struct ifnet *ifp;

    cyg_scheduler_lock();
    (*pr)("Routing tables\n");
    (*pr)("Destination     Gateway         Mask            Flags    Interface\n");
    for (i = 1; i <= AF_MAX; i++) {
        if ((rnh = rt_tables[i]) != NULL) {
            error = rnh->rnh_walktree(rnh, _dumpentry, pr);
        }
    }

    (*pr)("Interface statistics\n");
    for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
        _show_ifp(ifp, pr);
    }
    cyg_scheduler_unlock();
}

#endif // CYGPKG_NET_DRIVER_FRAMEWORK

// EOF support.c



