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
// Contributors: gthomas, hmt
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
#include <sys/kernel.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/sockio.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/if.h>
#include <net/route.h>
#include <net/netisr.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <arpa/inet.h>

#include <machine/cpu.h>

#include <pkgconf/net.h>

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>

#include <cyg/infra/cyg_ass.h>

#if !CYGPKG_NET_DRIVER_FRAMEWORK   // Interface
#error At least one network driver framework must be defined!
#else
#include <cyg/io/eth/netdev.h>

// Define table boundaries
CYG_HAL_TABLE_BEGIN( __NETDEVTAB__, netdev );
CYG_HAL_TABLE_END( __NETDEVTAB_END__, netdev );

// Used for system-wide "ticks per second"
int hz = 100;
int tick = 10000;  // usec per "tick"

volatile struct timeval mono_time;
volatile struct timeval ktime;

// Low-level network debugging
int net_debug = 0;

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
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


// Round a number 'n' up to a multiple of 'm'
#define round(n,m) ((((n)+((m)-1))/(m))*(m))

#define NET_MEMPOOL_SIZE  round(CYGPKG_NET_MEM_USAGE/4,MSIZE)
#define NET_MBUFS_SIZE    round(CYGPKG_NET_MEM_USAGE/4,MSIZE)
#define NET_CLUSTERS_SIZE round(CYGPKG_NET_MEM_USAGE/2,MCLBYTES)

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

#ifdef CYGDBG_NET_TIMING_STATS
static struct net_stats  stats_malloc, stats_free, 
    stats_memcpy, stats_memset,
    stats_mbuf_alloc, stats_mbuf_free, stats_cluster_alloc;
extern struct net_stats stats_in_cksum;

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
    show_net_stats(&stats_in_cksum,      "Checksum");
    show_net_stats(&stats_memcpy,        "Net memcpy");
    show_net_stats(&stats_memset,        "Net memset");
}
#endif /* CYGDBG_NET_TIMING_STATS */ 

void *
cyg_net_malloc(u_long size, int type, int flags)
{
    void *res;
    START_STATS();
    if (flags & M_NOWAIT) {
        res = cyg_mempool_var_try_alloc(net_mem, size);
    } else {
        res = cyg_mempool_var_alloc(net_mem, size);
    }
    FINISH_STATS(stats_malloc);
    return (res);
}

void 
cyg_net_free(caddr_t addr, int type)
{
    START_STATS();
    cyg_mempool_var_free(net_mem, addr);
    FINISH_STATS(stats_free);
}

void *
cyg_net_mbuf_alloc(int type, int flags)
{
    void *res;    
    START_STATS();
    mbstat.m_mbufs++;
    if (flags & M_NOWAIT) {
        res = cyg_mempool_fix_try_alloc(net_mbufs);
    } else {
        res = cyg_mempool_fix_alloc(net_mbufs);
    }
    FINISH_STATS(stats_mbuf_alloc);
    // Check that this nastiness works OK
    CYG_ASSERT( dtom(res) == res, "dtom failed, base of mbuf" );
    CYG_ASSERT( dtom((char *)res + MSIZE/2) == res, "dtom failed, mid mbuf" );
    return (res);
}

void 
cyg_net_mbuf_free(caddr_t addr, int type)
{
    START_STATS();
    mbstat.m_mbufs--;
    cyg_mempool_fix_free(net_mbufs, addr);
    FINISH_STATS(stats_mbuf_free);
}

void *
cyg_net_cluster_alloc(void)
{
    void *res;
    START_STATS();
    res = cyg_mempool_fix_try_alloc(net_clusters);
    FINISH_STATS(stats_cluster_alloc);
    return res;
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

void 
microtime(struct timeval *tp)
{
    panic("microtime");
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
    memcpy(d, s, len);
}

// ------------------------------------------------------------------------
// THE NETWORK THREAD ITSELF
//
// Network software interrupt handler
//   This function is run as a separate thread to allow
// processing of network events (mostly incoming packets)
// at "user level" instead of at interrupt time.
//
static void
cyg_netint(cyg_addrword_t param)
{
    cyg_flag_value_t curisr;
    int spl;
    while (true) {
        curisr = cyg_flag_wait(&netint_flags, NETISR_ANY, 
                               CYG_FLAG_WAITMODE_OR|CYG_FLAG_WAITMODE_CLR);
        spl = splsoftnet(); // Prevent any overlapping "stack" processing
#ifdef INET
        if (curisr & (1 << NETISR_ARP)) {
            // Pending ARP requests
            arpintr();
        }
        if (curisr & (1 << NETISR_IP)) {
            // Pending IPv4 input
            ipintr();
        }
#endif
#ifdef INET6
        if (curisr & (1 << NETISR_IPV6)) {
            // Pending IPv6 input
            ip6intr();
        }
#endif
#if NBRIDGE > 0
        if (curisr & (1 << NETISR_BRIDGE)) {
            // Pending bridge input
            bridgeintr();
        }
#endif
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

void
cyg_net_init(void)
{
    static int _init = false;
    cyg_netdevtab_entry_t *t;

    if (_init) return;

    cyg_do_net_init();  // Just forces linking in the initializer/constructor
    // Initialize interrupt "flags"
    cyg_flag_init(&netint_flags);
    // Initialize timeouts and net service thread (pseudo-DSRs)
    cyg_alarm_timeout_init();
    // Initialize tsleep/wakeup support
    cyg_tsleep_init();
    // Initialize network memory system
    cyg_kmem_init();
    mbinit();
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

    // Initialize all network devices
    for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
//        diag_printf("Init device '%s'\n", t->name);
        if (t->init(t)) {
            t->status = CYG_NETDEVTAB_STATUS_AVAIL;
        } else {
            // What to do if device init fails?
            t->status = 0;  // Device not [currently] available
        }
    }
    // And attach the loopback interface
#ifdef CYGPKG_NET_NLOOP
#if 0 < CYGPKG_NET_NLOOP
    loopattach(0);
#endif
#endif
#if NBRIDGE > 0
    bridgeattach(0);
#endif
    // Start up the network processing
    ifinit();
    domaininit();

    // Done
    _init = true;
}

// Copyright (C) 2002 Gary Thomas

#include <net/if.h>
#include <net/route.h>
#include <net/netdb.h>
externC void if_indextoname(int indx, char *buf, int len);

typedef void pr_fun(char *fmt, ...);

static void
_mask(struct sockaddr *sa, char *buf, int _len)
{
    char *cp = ((char *)sa) + 4;
    int len = sa->sa_len - 4;
    int tot = 0;

    while (len-- > 0) {
        if (tot) *buf++ = '.';
        buf += diag_sprintf(buf, "%d", *cp++);
        tot++;
    }

    while (tot < 4) {
        if (tot) *buf++ = '.';
        buf += diag_sprintf(buf, "%d", 0);
        tot++;
    }
}

static void
_show_ifp(struct ifnet *ifp, pr_fun *pr)
{
    struct ifaddr *ifa;
    char addr[64], netmask[64], broadcast[64];

    (*pr)("%-8s", ifp->if_xname);
    TAILQ_FOREACH(ifa, &ifp->if_addrlist, ifa_list) {
        if (ifa->ifa_addr->sa_family != AF_LINK) {
            getnameinfo (ifa->ifa_addr, ifa->ifa_addr->sa_len, addr, sizeof(addr), 0, 0, 0);
            getnameinfo (ifa->ifa_dstaddr, ifa->ifa_dstaddr->sa_len, broadcast, sizeof(broadcast), 0, 0, 0);
            _mask(ifa->ifa_netmask, netmask, 64);
            (*pr)("IP: %s, Broadcast: %s, Netmask: %s\n", addr, broadcast, netmask);
            (*pr)("        ");
            if ((ifp->if_flags & IFF_UP)) (*pr)("UP ");
            if ((ifp->if_flags & IFF_BROADCAST)) (*pr)("BROADCAST ");
            if ((ifp->if_flags & IFF_LOOPBACK)) (*pr)("LOOPBACK ");
            if ((ifp->if_flags & IFF_RUNNING)) (*pr)("RUNNING ");
            if ((ifp->if_flags & IFF_PROMISC)) (*pr)("PROMISC ");
            if ((ifp->if_flags & IFF_MULTICAST)) (*pr)("MULTICAST ");
            if ((ifp->if_flags & IFF_ALLMULTI)) (*pr)("ALLMULTI ");
            (*pr)("MTU: %d, Metric: %d\n", ifp->if_mtu, ifp->if_metric);
            (*pr)("        Rx - Packets: %d, Bytes: %d", ifp->if_data.ifi_ipackets, ifp->if_data.ifi_ibytes);
            (*pr)(", Tx - Packets: %d, Bytes: %d\n", ifp->if_data.ifi_opackets, ifp->if_data.ifi_obytes);
        }
    }
}

static int
_dumpentry(struct radix_node *rn, void *vw)
{
    struct rtentry *rt = (struct rtentry *)rn;
    struct sockaddr *dst, *gate, *netmask, *genmask;
    char addr[32], *cp;
    pr_fun *pr = (pr_fun *)vw;

    dst = rt_key(rt);
    gate = rt->rt_gateway;
    netmask = rt_mask(rt);
    genmask = rt->rt_genmask;
    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == RTF_UP) {
        if (netmask == NULL) {
            return 0;
        }
        _inet_ntop(dst, addr, sizeof(addr));
        (*pr)("%-15s ", addr);
        if (gate != NULL) {
            _inet_ntop(gate, addr, sizeof(addr));
            (*pr)("%-15s ", addr);
        } else {
            (*pr)("%-15s ", " ");
        }
        if (netmask != NULL) {
            _mask(netmask, addr, sizeof(addr));
            (*pr)("%-15s ", addr);
        } else {
            (*pr)("%-15s ", " ");
        }
        cp = addr;
        if ((rt->rt_flags & RTF_UP)) *cp++ = 'U';
        if ((rt->rt_flags & RTF_GATEWAY)) *cp++ = 'G';
        if ((rt->rt_flags & RTF_STATIC)) *cp++ = 'S';
        if ((rt->rt_flags & RTF_DYNAMIC)) *cp++ = 'D';
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
    for (ifp = ifnet.tqh_first; ifp != 0; ifp = ifp->if_list.tqe_next) {
        _show_ifp(ifp, pr);
    }
    cyg_scheduler_unlock();
}

#endif // CYGPKG_NET_DRIVER_FRAMEWORK

// EOF support.c
