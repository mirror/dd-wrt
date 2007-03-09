//==========================================================================
//
//      src/net/eth_drv.c
//
//      Hardware independent ethernet driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      Hardware independent ethernet driver
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// High-level ethernet driver

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/netisr.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#ifndef NBPFILTER
#define NBPFILTER 0
#endif

#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/drv_api.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_if.h>
#include <pkgconf/io_eth_drivers.h> // module configury; SIMULATED_FAILURES
#include <pkgconf/net.h>            // CYGPKG_NET_FAST_THREAD_TICKLE_DEVS?

#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>

#ifndef min
#define min( _x_, _y_ ) ((_x_) < (_y_) ? (_x_) : (_y_))
#endif

// ------------------------------------------------------------------------
#ifdef CYGPKG_IO_ETH_DRIVERS_SIMULATED_FAILURES

#define noLOG_RANDOM 32 // so you can tell this is really being random
#ifdef LOG_RANDOM
static struct {
    unsigned int *which;
    unsigned int random;
    unsigned int r100;
} random_log[LOG_RANDOM];

static int random_index = 0;
#endif

static unsigned int
randomize( unsigned int *p )
{
    unsigned int r100;
    HAL_CLOCK_READ( &r100 );
    r100 ^= *p;
    *p = (r100 * 1103515245) + 12345;
    r100 &= 127;
    if ( r100 >= 100 ) // spread the overflow around evenly
        r100 = 4 * (r100 - 100);
    if ( r100 >= 100 ) // and again - (125,126,127=>100,104,108)
        r100 = 12 * (r100 - 100); // =>(0,48,96)
#ifdef LOG_RANDOM
    random_log[random_index].which  = p;
    random_log[random_index].random = *p;
    random_log[random_index].r100   = r100;
    random_index++;
    random_index &= (LOG_RANDOM-1);
#endif
    return r100;
}

#define SIMULATE_FAIL_SEND     1
#define SIMULATE_FAIL_RECV     2
#define SIMULATE_FAIL_CORRUPT  3

static struct simulated_failure_state {
    struct eth_drv_sc *sc;
    unsigned int r_tx_fail;
    unsigned int r_rx_fail;
    unsigned int r_rx_corrupt;
    cyg_tick_count_t droptime;
    cyg_tick_count_t passtime;
} simulated_failure_states[2] = {{0},{0}};

static int
simulate_fail( struct eth_drv_sc *sc, int which )
{
    struct simulated_failure_state *s;  
    
    for ( s = &simulated_failure_states[0]; s < &simulated_failure_states[2];
          s++ ) {
        if ( 0 == s->sc ) {
            s->sc = sc;
            s->r_tx_fail    = (unsigned int)sc;
            s->r_rx_fail    = (unsigned int)sc ^ 0x01234567;
            s->r_rx_corrupt = (unsigned int)sc ^ 0xdeadbeef;
            s->droptime = 0;
            s->passtime = 0;
        }
        if ( sc == s->sc )
            break;
    }
    if ( &simulated_failure_states[2] == s ) {
        CYG_FAIL( "No free slot in simulated_failure_states[]" );
        return 1; // always fail
    }

#ifdef CYGPKG_IO_ETH_DRIVERS_SIMULATE_LINE_CUT
    // Regardless of the question, we say "yes" during the period of
    // unpluggedness...
    {
        cyg_tick_count_t now = cyg_current_time();
        if ( now > s->droptime && 0 == s->passtime ) { // [initial condition]
            s->droptime = 0; // go into a passing phase
            (void)randomize( &s->r_tx_fail );
            (void)randomize( &s->r_rx_fail );
            (void)randomize( &s->r_rx_corrupt );
            s->passtime = s->r_tx_fail + s->r_rx_fail + s->r_rx_corrupt;
            s->passtime &= 0x3fff; // 16k cS is up to 160S, about 2.5 minutes
            s->passtime += now;
        }
        else if ( now > s->passtime && 0 == s->droptime ) {
            s->passtime = 0; // go into a dropping phase
            (void)randomize( &s->r_tx_fail );
            (void)randomize( &s->r_rx_fail );
            (void)randomize( &s->r_rx_corrupt );
            s->droptime = s->r_tx_fail + s->r_rx_fail + s->r_rx_corrupt;
            s->droptime &= 0x0fff; // 4k cS is up to 40S, about 1/2 a minute
            s->droptime += now;
        }

        if ( now < s->droptime )
            return 1; // Say "no"
    }
#endif

    switch ( which ) {
#ifdef CYGPKG_IO_ETH_DRIVERS_SIMULATE_DROP_TX
    case SIMULATE_FAIL_SEND: {
        unsigned int z = randomize( &s->r_tx_fail );
        return z < CYGPKG_IO_ETH_DRIVERS_SIMULATE_DROP_TX;
    }
#endif
#ifdef CYGPKG_IO_ETH_DRIVERS_SIMULATE_DROP_RX
    case SIMULATE_FAIL_RECV: {
        unsigned int z = randomize( &s->r_rx_fail );
        return z < CYGPKG_IO_ETH_DRIVERS_SIMULATE_DROP_RX;
    }
#endif
#ifdef CYGPKG_IO_ETH_DRIVERS_SIMULATE_CORRUPT_RX
    case SIMULATE_FAIL_CORRUPT: {
        unsigned int z = randomize( &s->r_rx_corrupt );
        return z < CYGPKG_IO_ETH_DRIVERS_SIMULATE_CORRUPT_RX;
    }
#endif
    default:
        // do nothing - for when options above are not enabled.
    }
    return 0;
}

#define noLOG_CORRUPTION 32 // so you can tell this is really being random
#ifdef LOG_CORRUPTION
static struct {
    int len;
    int thislen;
    int off;
    unsigned char xor;
    unsigned char idx;
} corruption_log[LOG_CORRUPTION];

static int corruption_index = 0;
#endif

static void
simulate_fail_corrupt_sglist( struct eth_drv_sg *sg_list, int sg_len )
{
    unsigned int z, len, i, off;
    HAL_CLOCK_READ( &z );
    z += simulated_failure_states[0].r_rx_corrupt;
    z += simulated_failure_states[1].r_rx_corrupt;

    CYG_ASSERT( MAX_ETH_DRV_SG >= sg_len, "sg_len overflow in corrupt" );

    for ( i = 0, len = 0; i < sg_len && sg_list[i].buf && sg_list[i].len; i++ )
        len =+ sg_list[i].len;

    CYG_ASSERT( 1500 >= len, "sg...len > ether MTU" );
    if ( 14 >= len ) // normal ether header
        return;

    off = z & 2047; // next (2^N-1) > MTU
    while ( off > len )
        off -= len;

    for ( i = 0; i < sg_len && sg_list[i].buf && sg_list[i].len; i++ ) {
        if ( off < sg_list[i].len ) { // corrupt this one
            unsigned char *p = (unsigned char *)sg_list[i].buf;
            p[off] ^= (0xff & (z >> 11));
#ifdef LOG_CORRUPTION
            corruption_log[corruption_index].len = len;
            corruption_log[corruption_index].thislen = sg_list[i].len;
            corruption_log[corruption_index].off = off;
            corruption_log[corruption_index].xor = (0xff & (z >> 11));
            corruption_log[corruption_index].idx = i;
            corruption_index++;
            corruption_index &= (LOG_CORRUPTION-1);
#endif
            return;
        }
        off -= sg_list[i].len;
    }    
    CYG_FAIL( "Didn't corrupt anything" );
}

#endif // CYGPKG_IO_ETH_DRIVERS_SIMULATED_FAILURES
// ------------------------------------------------------------------------

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

#include <cyg/hal/hal_if.h>

// Use with care!  Local variable defined!
#define START_CONSOLE()                                                                 \
{   /* NEW BLOCK */                                                                     \
    int _cur_console =                                                                  \
        CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);      \
    {                                                                                   \
        int i;                                                                          \
        if ( CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,                 \
                                          "info_console_force", &i,                     \
                                          CYGNUM_FLASH_CFG_TYPE_CONFIG_BOOL ) ) {       \
            if ( i ) {                                                                  \
                if ( CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,         \
                                                  "info_console_number", &i,            \
                                                  CYGNUM_FLASH_CFG_TYPE_CONFIG_INT ) ){ \
                    /* Then i is the console to force it to: */                         \
                    CYGACC_CALL_IF_SET_CONSOLE_COMM( i );                               \
                }                                                                       \
            }                                                                           \
        }                                                                               \
    }

#define END_CONSOLE()                                   \
    CYGACC_CALL_IF_SET_CONSOLE_COMM(_cur_console);      \
}   /* END BLOCK */

#else
#define START_CONSOLE()
#define END_CONSOLE()
#endif
// ------------------------------------------------------------------------

#ifdef CYGPKG_NET_FREEBSD_STACK
extern char *_ioctl_name(u_long cmd);
typedef void void_fun(void *);
#endif

static int  eth_drv_ioctl(struct ifnet *, u_long, caddr_t);
static void eth_drv_send(struct ifnet *);
static void eth_drv_start(struct eth_drv_sc *sc);

#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG 
int cyg_io_eth_net_debug = CYGDBG_IO_ETH_DRIVERS_DEBUG_VERBOSITY;
#endif

// Interfaces exported to drivers

static void eth_drv_init(struct eth_drv_sc *sc, unsigned char *enaddr);
static void eth_drv_recv(struct eth_drv_sc *sc, int total_len);
static void eth_drv_tx_done(struct eth_drv_sc *sc, CYG_ADDRESS key, int status);

struct eth_drv_funs eth_drv_funs = {eth_drv_init, eth_drv_recv, eth_drv_tx_done};

//
// This function is called during system initialization to register a
// network interface with the system.
//
static void
eth_drv_init(struct eth_drv_sc *sc, unsigned char *enaddr)
{
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#ifdef CYGPKG_NET_FREEBSD_STACK
    int unit;
    char *np, *xp;
#endif

    // Set up hardware address
    if (NULL != enaddr)
        bcopy(enaddr, &sc->sc_arpcom.ac_enaddr, ETHER_ADDR_LEN);

    // Initialize ifnet structure
    ifp->if_softc = sc;
    ifp->if_start = eth_drv_send;
    ifp->if_ioctl = eth_drv_ioctl;
    ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
#ifdef IFF_NOTRAILERS
    ifp->if_flags |= IFF_NOTRAILERS;
#endif
#ifdef CYGPKG_NET_FREEBSD_STACK
    ifp->if_name = xp = ifp->if_xname;
    np = (char *)sc->dev_name;
    unit = 0;
    while (*np && !((*np >= '0') && (*np <= '9'))) *xp++ = *np++;
    if (*np) {
        *xp = '\0';
        while (*np) {
            unit = (unit * 10) + (*np++ - '0');
        }
        ifp->if_unit = unit;
    }
    ifp->if_init = (void_fun *)eth_drv_start;
    ifp->if_output = ether_output;
#else
    bcopy((void *)sc->dev_name, ifp->if_xname, IFNAMSIZ);
#endif
    sc->state = 0;

    // Attach the interface
#ifdef CYGPKG_NET_FREEBSD_STACK
    ether_ifattach(ifp, 0);
#else
    if_attach(ifp);
    ether_ifattach(ifp);
#endif

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG
// Set up interfaces so debug environment can share this device
    {
        void *dbg = CYGACC_CALL_IF_DBG_DATA();
        if (!dbg) {
            CYGACC_CALL_IF_DBG_DATA_SET((void *)sc);
        }
    }
#endif
}

//
// This [internal] function will be called to stop activity on an interface.
//
static void
eth_drv_stop(struct eth_drv_sc *sc)
{
    (sc->funs->stop)(sc);
    sc->state &= ~ETH_DRV_STATE_ACTIVE;
}

//
// This [internal] function will be called to start activity on an interface.
//
static void
eth_drv_start(struct eth_drv_sc *sc)
{
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;

    // Perform any hardware initialization
    (sc->funs->start)(sc, (unsigned char *)&sc->sc_arpcom.ac_enaddr, 0);
#ifdef CYGPKG_NET_FREEBSD_STACK
    // resend multicast addresses if present
    if(ifp->if_multiaddrs.lh_first && ifp->if_ioctl) {
        int s = splimp();
	ifp->if_ioctl(ifp, SIOCADDMULTI, 0);
	splx(s);
    }
#endif
    // Set 'running' flag, and clear output active flag.
    ifp->if_flags |= IFF_RUNNING;
    ifp->if_flags &= ~IFF_OACTIVE;
    sc->state |= ETH_DRV_STATE_ACTIVE;
    eth_drv_send(ifp);  // Try and start up transmit
}

//
// This function supports "I/O control" operations on an interface.
//
static int  
eth_drv_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
    struct eth_drv_sc *sc = ifp->if_softc;
#ifndef CYGPKG_NET_FREEBSD_STACK
    struct ifaddr *ifa = (struct ifaddr *) data;
#endif
    struct ifreq *ifr = (struct ifreq *)data;
    int     s, error = 0;

// DEBUG
#ifdef CYGPKG_NET_FREEBSD_STACK
    log(LOG_IOCTL, "%s: cmd: %s, data:\n", __FUNCTION__, _ioctl_name(cmd));
    log_dump(LOG_IOCTL, data, 32);
#endif
// DEBUG

    s = splnet();

#ifdef CYGPKG_NET_FREEBSD_STACK
    if ((error = ether_ioctl(ifp, cmd, data)) > 0) {
#else
    if ((error = ether_ioctl(ifp, &sc->sc_arpcom, cmd, data)) > 0) {
#endif
        splx(s);
        return error;
    }

    switch (cmd) {

    case SIOCSIFADDR:
#ifndef CYGPKG_NET_FREEBSD_STACK // Now in if_ethersubr.c
        ifp->if_flags |= IFF_UP;

        switch (ifa->ifa_addr->sa_family) {
#ifdef INET
        case AF_INET:
            eth_drv_start(sc);
            arp_ifinit(&sc->sc_arpcom, ifa);
            break;
#endif
        default:
            eth_drv_start(sc);
            break;
        }
#endif // CYGPKG_NET_FREEBSD_STACK
        break;

    case SIOCGIFHWADDR:
        // Get hardware (MAC) address
        ifr->ifr_hwaddr.sa_family = AF_INET;
        bcopy(&sc->sc_arpcom.ac_enaddr, &ifr->ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
        break;

    case SIOCSIFHWADDR:
        // Set hardware (MAC) address
        bcopy(&ifr->ifr_hwaddr.sa_data, &sc->sc_arpcom.ac_enaddr, ETHER_ADDR_LEN);
        if ((sc->funs->control)(sc, ETH_DRV_SET_MAC_ADDRESS,
                                &sc->sc_arpcom.ac_enaddr, ETHER_ADDR_LEN)) {
            error = EINVAL;
        }
        break;

#ifdef SIOCGIFSTATS
    case SIOCGIFSTATS:
#ifdef SIOCGIFSTATSUD
    case SIOCGIFSTATSUD:
#endif
        // Get interface statistics:
        if ((sc->funs->control)(sc, (cmd == SIOCGIFSTATS)
                                ? ETH_DRV_GET_IF_STATS
                                : ETH_DRV_GET_IF_STATS_UD,
                                data, 0 ) ) {
            error = EINVAL;
        }
        break;
#endif // SIOCGIFSTATS

    case SIOCSIFFLAGS:
        if ((ifp->if_flags & IFF_UP) == 0 &&
            (ifp->if_flags & IFF_RUNNING) != 0) {
            /*
             * If interface is marked down and it is running, then
             * stop it.
             */
            eth_drv_stop(sc);
            ifp->if_flags &= ~IFF_RUNNING;
        } else
            if ((ifp->if_flags & IFF_UP) != 0 &&
                (ifp->if_flags & IFF_RUNNING) == 0) {
                /*
                 * If interface is marked up and it is stopped, then
                 * start it.
                 */
                eth_drv_start(sc);
            } else {
                /*
                 * Reset the interface to pick up changes in any other
                 * flags that affect hardware registers.
                 */
                eth_drv_stop(sc);
                eth_drv_start(sc);
            }
        break;

#ifdef CYGPKG_NET_FREEBSD_STACK
    case SIOCADDMULTI:
    case SIOCDELMULTI:
    {
	struct ifmultiaddr *ifma;
        struct eth_drv_mc_list mc_list;
        int mode = (ifp->if_flags & IFF_ALLMULTI) ? ETH_DRV_SET_MC_ALL :
                                                    ETH_DRV_SET_MC_LIST;

#ifdef DEBUG
        log(LOG_ADDR, "%s Multi\n",(cmd == SIOCADDMULTI) ? "Add" : "Del");
#endif
        mc_list.len = 0;
        LIST_FOREACH(ifma, &((ifp)->if_multiaddrs), ifma_link) {
  	    if (ifma->ifma_addr->sa_family != AF_LINK) {
	      continue;
	    }
#ifdef DEBUG
            log_dump(LOG_ADDR, LLADDR((struct sockaddr_dl *)ifma->ifma_addr), 6);
#endif
            if ((LLADDR((struct sockaddr_dl *)ifma->ifma_addr)[0] & 0x01) == 0) {
#ifdef DEBUG
                log(LOG_ADDR, "** Not a multicast address - ignored\n");
#endif
                continue;
            }
            if (mc_list.len < ETH_DRV_MAX_MC) {
                bcopy(LLADDR((struct sockaddr_dl *)ifma->ifma_addr),
                      mc_list.addrs[mc_list.len], ETHER_ADDR_LEN);
                mc_list.len++;
            } else {
                mode = ETH_DRV_SET_MC_ALL;
            }
        }
        // Note: drivers may behave like IFF_ALLMULTI if the list is 
        // more than their hardware can handle, e.g. some can only handle 1.
        if ((sc->funs->control)(sc, mode, &mc_list, sizeof(mc_list))) {
            diag_printf( "[%s] Warning: Driver can't set multi-cast mode\n",
                         __FUNCTION__ );
            error = EINVAL;
        }
        break;
    }
#endif

    default:
        error = EINVAL;
        break;
    }

    splx(s);
    return (error);
}

//
// Control whether any special locking needs to take place if we intend to
// cooperate with a ROM monitor (e.g. RedBoot) using this hardware.  
//
#if defined(CYGSEM_HAL_USE_ROM_MONITOR) && \
    defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG) && \
   !defined(CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_COMMS)

// Indicate that special locking precautions are warranted.
#define _LOCK_WITH_ROM_MONITOR

// This defines the [well known] channel that RedBoot will use when it is
// using the network hardware for the debug channel.
#define RedBoot_TCP_CHANNEL CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS

// Define this if you ever need to call 'diag_printf()' from interrupt level
// code (ISR) and the debug channel might be using the network hardware. If
// this is not the case, then disabling interrupts here is over-kill.
//#define _LOCK_USING_INTERRUPTS
#endif

//
// This routine is called to start transmitting if there is data
// available.
//
static void 
eth_drv_send(struct ifnet *ifp)
{
    struct eth_drv_sc *sc = ifp->if_softc;
#if MAX_ETH_DRV_SG > 64
    static  // Avoid large stack requirements
#endif
    struct eth_drv_sg sg_list[MAX_ETH_DRV_SG];
    int sg_len;
    struct mbuf *m0, *m;
    int len, total_len;
    unsigned char *data;
#ifdef _LOCK_WITH_ROM_MONITOR
#ifdef _LOCK_USING_INTERRUPTS
    cyg_uint32 ints;
#endif
    bool need_lock = false;
    int debug_chan;
#endif // _LOCK_WITH_ROM_MONITOR

    // This is now only called from network threads, so no guarding is
    // required; locking is in place via the splfoo() mechanism already.

    if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING) {
         return;
    }

    // If nothing on the queue, no need to bother hardware
    if (IF_IS_EMPTY(&ifp->if_snd)) {
        return;
    }

    while ((sc->funs->can_send)(sc) > 0) {
        IF_DEQUEUE(&ifp->if_snd, m0);
        if (m0 == 0) {
            break;
        }

#ifdef CYGPKG_IO_ETH_DRIVERS_SIMULATED_FAILURES
        if ( simulate_fail( sc, SIMULATE_FAIL_SEND ) ) {
            // must free the mbufs
            m_freem(m0);
            continue; // next packet to send
        }
#endif

#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
        if (cyg_io_eth_net_debug) {
            START_CONSOLE();
            diag_printf("Sending %d bytes\n", m0->m_pkthdr.len);
            END_CONSOLE();
        }
#endif

        /* We need to use m->m_pkthdr.len, so require the header */
        if ((m0->m_flags & M_PKTHDR) == 0)
            panic("eth_drv_send: no header mbuf");

#if NBPFILTER > 0
        /* Tap off here if there is a BPF listener. */
        if (ifp->if_bpf)
            bpf_mtap(ifp->if_bpf, m0);
#endif

        // Extract data pointers (don't actually move data here)
        sg_len = 0;  total_len = 0;
        for (m = m0; m ; m = m->m_next) {
            data = mtod(m, u_char *);
            len = m->m_len;
            total_len += len;
            sg_list[sg_len].buf = (CYG_ADDRESS)data;
            sg_list[sg_len].len = len;
            if ( len )
                sg_len++;
#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
            if (cyg_io_eth_net_debug) {
                START_CONSOLE();
                diag_printf("xmit %d bytes at %x sg[%d]\n", len, data, sg_len);
                if ( cyg_io_eth_net_debug > 1)
                    diag_dump_buf(data, len);
                END_CONSOLE();
            }
#endif
            if ( MAX_ETH_DRV_SG < sg_len ) {
#ifdef CYGPKG_IO_ETH_DRIVERS_WARN_NO_MBUFS
                int needed = 0;
                struct mbuf *m1;
                for (m1 = m0; m1 ; m1 = m1->m_next) needed++;
                START_CONSOLE();
                diag_printf("too many mbufs to tx, %d > %d, need %d\n", 
                            sg_len, MAX_ETH_DRV_SG, needed );
                END_CONSOLE();
#endif
                sg_len = 0;
                break; // drop it on the floor
            }
        }

#ifdef _LOCK_WITH_ROM_MONITOR
        // Firm lock on this portion of the driver.  Since we are about to
        // start messing with the actual hardware, it is imperative that the
        // current thread not loose control of the CPU at this time.  Otherwise,
        // the hardware could be left in an unusable state.  This caution is
        // only warranted if there is a possibility of some other thread trying
        // to use the hardware simultaneously.  The network stack would prevent
        // this implicitly since all accesses are controlled by the "splX()"
        // locks, but if there is a ROM monitor, such as RedBoot, also using
        // the hardware, all bets are off.

        // Note: these operations can be avoided if it were well known that
        // RedBoot was not using the network hardware for diagnostic I/O.  This
        // can be inferred by checking which I/O channel RedBoot is currently
        // hooked to.
        debug_chan = CYGACC_CALL_IF_SET_DEBUG_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
        if (debug_chan == RedBoot_TCP_CHANNEL) {
            need_lock = true;
#ifdef _LOCK_USING_INTERRUPTS
            HAL_DISABLE_INTERRUPTS(ints);
#endif
            cyg_drv_dsr_lock();
        }
#endif // _LOCK_WITH_ROM_MONITOR

        // Tell hardware to send this packet
        if ( sg_len )
            (sc->funs->send)(sc, sg_list, sg_len, total_len, (unsigned long)m0);

#ifdef _LOCK_WITH_ROM_MONITOR
        // Unlock the driver & hardware.  It can once again be safely shared.
        if (need_lock) {
            cyg_drv_dsr_unlock();
#ifdef _LOCK_USING_INTERRUPTS
            HAL_RESTORE_INTERRUPTS(ints);
#endif
        }
#endif // _LOCK_WITH_ROM_MONITOR
#undef _LOCK_WITH_ROM_MONITOR
    }
}

//
// This function is called from the hardware driver when an output operation
// has completed - i.e. the packet has been sent.
//
static struct mbuf *mbuf_key;

static void
eth_drv_tx_done(struct eth_drv_sc *sc, CYG_ADDRESS key, int status)
{
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
    struct mbuf *m0 = (struct mbuf *)key;
    CYGARC_HAL_SAVE_GP();

    // Check for errors here (via 'status')
    ifp->if_opackets++;
    // Done with packet

    // Guard against a NULL return - can be caused by race conditions in
    // the driver, this is the neatest fixup:
    if (m0) { 
        mbuf_key = m0;
        m_freem(m0);
    }
    // Start another if possible
    eth_drv_send(ifp);
    CYGARC_HAL_RESTORE_GP();
}

//
// This function is called from a hardware driver to indicate that an input
// packet has arrived.  The routine will set up appropriate network resources
// (mbuf's) to hold the data and call back into the driver to retrieve the data.
//
static void
eth_drv_recv(struct eth_drv_sc *sc, int total_len)
{
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
    struct ether_header _eh, *eh=&_eh;
    struct mbuf *top, **mp, *m;
    int mlen;
    unsigned char *data;
#if MAX_ETH_DRV_SG > 64
    static  // Avoid large stack requirements
#endif
    struct eth_drv_sg sg_list[MAX_ETH_DRV_SG];
    int sg_len;

    if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING) {
        return;  // Interface not up, ignore this request
    }

    CYG_ASSERT( 0 != total_len, "total_len is zero!" );
    CYG_ASSERT( 0 <= total_len, "total_len is negative!" );
    CYG_ASSERT( sizeof( struct ether_header ) <= total_len,
                "No ether header here!" );

    if ( total_len < sizeof( struct ether_header ) )
        // Our arithmetic below would go wrong
        return;

    CYGARC_HAL_SAVE_GP();  // This is down here to make matching restore neat

    /* Pull packet off interface. */
    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == 0) {
#ifdef CYGPKG_IO_ETH_DRIVERS_WARN_NO_MBUFS
        START_CONSOLE();
        diag_printf("warning: eth_recv out of MBUFs\n");
#ifdef CYGDBG_NET_SHOW_MBUFS        
        cyg_net_show_mbufs();
#endif
        END_CONSOLE();
#endif
    }

    // Set up buffers
    // Unload ethernet header separately so IP/UDP/TCP headers are aligned
    sg_list[0].buf = (CYG_ADDRESS)eh;
    sg_list[0].len = sizeof(*eh);
    sg_len = 1;

    // Compute total length (minus ethernet header)
    total_len -= sizeof(*eh);

    top = 0;
    mlen = MHLEN;
    mp = &top;

    if (m) {
        m->m_pkthdr.rcvif = ifp;
        m->m_pkthdr.len = total_len;
    } else {
        sg_list[sg_len].buf = (CYG_ADDRESS)0;
        sg_list[sg_len].len = total_len;
        sg_len++;
        total_len = 0;
    }

    while (total_len > 0) {
        if (top) {
            MGET(m, M_DONTWAIT, MT_DATA);
            if (m == 0) {
                m_freem(top);
#ifdef CYGPKG_IO_ETH_DRIVERS_WARN_NO_MBUFS
                START_CONSOLE();
                diag_printf("out of MBUFs [2]");
#ifdef CYGDBG_NET_SHOW_MBUFS                
                cyg_net_show_mbufs();
#endif
                END_CONSOLE();
#endif
                sg_list[sg_len].buf = (CYG_ADDRESS)0;
                sg_list[sg_len].len = total_len;
                sg_len++;
                top = 0;
                break;
            }
            mlen = MLEN;
        }
        if (total_len >= MINCLSIZE) {
            MCLGET(m, M_DONTWAIT);
            if ((m->m_flags & M_EXT) == 0) {
                m_freem(top);
                m_free(m);
#ifdef CYGPKG_IO_ETH_DRIVERS_WARN_NO_MBUFS
                START_CONSOLE();
                diag_printf("warning: eth_recv out of MBUFs\n");
#ifdef CYGDBG_NET_SHOW_MBUFS                
                cyg_net_show_mbufs();
#endif
                END_CONSOLE();
#endif
                sg_list[sg_len].buf = (CYG_ADDRESS)0;
                sg_list[sg_len].len = total_len;
                sg_len++;
                top = 0;
                break;
            }
            mlen = MCLBYTES;
        }
        m->m_len = mlen = min(total_len, mlen);
        total_len -= mlen;
        data = mtod(m, caddr_t);
        sg_list[sg_len].buf = (CYG_ADDRESS)data;
        sg_list[sg_len].len = mlen;
        sg_len++;
        *mp = m;
        mp = &m->m_next;
    } // endwhile

    // Ask hardware to unload buffers
    (sc->funs->recv)(sc, sg_list, sg_len);

#ifdef CYGPKG_IO_ETH_DRIVERS_SIMULATED_FAILURES
    if ( simulate_fail( sc, SIMULATE_FAIL_RECV ) ) {
        // toss the packet - note that some hardware gets
        // fussy if the packet isn't "unloaded", thus we
        // have to wait until now to throw it away
        if (top) {
	    m_free(top);
        }
        ifp->if_ierrors++;
        return;
    }

    if ( simulate_fail( sc, SIMULATE_FAIL_CORRUPT ) ) {
        // Corrupt the data
        simulate_fail_corrupt_sglist( sg_list, sg_len );
    }
#endif

#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
    if (cyg_io_eth_net_debug) {
        int i;
        START_CONSOLE();
        for (i = 0;  i < sg_len;  i++) {
            if (sg_list[i].buf) {
                diag_printf("rx %d bytes at %x sg[%d]\n", sg_list[i].len, sg_list[i].buf, i);
                if ( cyg_io_eth_net_debug > 1 )
                    diag_dump_buf((void *)sg_list[i].buf, sg_list[i].len);
            }
        }
        END_CONSOLE();
    }
#endif
    m = top;
    if (m == 0) {
        ifp->if_ierrors++;
    }
    else {
        ifp->if_ipackets++;

#if NBPFILTER > 0
#error FIXME - Need mbuf with ethernet header attached
        /*
         * Check if there's a BPF listener on this interface.
         * If so, hand off the raw packet to bpf.
         */
        if (ifp->if_bpf)
            bpf_mtap(ifp->if_bpf, m);
#endif

        // Push data into protocol stacks
        ether_input(ifp, eh, m);
    }
    CYGARC_HAL_RESTORE_GP();
}


// ------------------------------------------------------------------------
// DSR to schedule network delivery thread

extern void ecos_synch_eth_drv_dsr(void); // from ecos/timeout.c in net stack

void
eth_drv_dsr(cyg_vector_t vector,
            cyg_ucount32 count,
            cyg_addrword_t data)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)data;

#ifdef CYGDBG_USE_ASSERTS
    // then check that this really is a "sc"
    {
        cyg_netdevtab_entry_t *t;
        for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++)
            if ( ((struct eth_drv_sc *)t->device_instance) == sc )
                break; // found it
        CYG_ASSERT( t != &__NETDEVTAB_END__, "eth_drv_dsr: Failed to find sc in NETDEVTAB" );
    }
#endif // Checking code

    sc->state |= ETH_DRV_NEEDS_DELIVERY;

    ecos_synch_eth_drv_dsr(); // [request] run delivery function for this dev
}

// This is called from the delivery thread, to do just that:
void eth_drv_run_deliveries( void )
{
    cyg_netdevtab_entry_t *t;
    for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
        struct eth_drv_sc *sc = (struct eth_drv_sc *)t->device_instance;
        if ( ETH_DRV_NEEDS_DELIVERY & sc->state ) {
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
            cyg_bool was_ctrlc_int;
#endif
            sc->state &=~ETH_DRV_NEEDS_DELIVERY;
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
            was_ctrlc_int = HAL_CTRLC_CHECK((*sc->funs->int_vector)(sc), (int)sc);
            if (!was_ctrlc_int) // Fall through and run normal code
#endif
            (*sc->funs->deliver)(sc);
        }
    }
}

// This is called from the delivery thread, to unstick devices if there is
// no network activity.
#ifdef CYGPKG_NET_FAST_THREAD_TICKLE_DEVS
void eth_drv_tickle_devices( void )
{
    cyg_netdevtab_entry_t *t;
    for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
        struct eth_drv_sc *sc = (struct eth_drv_sc *)t->device_instance;
        if ( ETH_DRV_STATE_ACTIVE & sc->state ) {
            struct ifnet *ifp = &sc->sc_arpcom.ac_if;
            // Try to dequeue a packet for this interface, if we can.  This
            // will call can_send() for active interfaces.  It is calls to
            // this function from tx_done() which normally provide
            // continuous transmissions; otherwise we do not get control.
            // This call fixes that.
            if (!IF_IS_EMPTY(&ifp->if_snd)) {
                eth_drv_send(ifp);
            }
        }
    }
}
#endif // CYGPKG_NET_FAST_THREAD_TICKLE_DEVS

// ------------------------------------------------------------------------

#ifdef CYGPKG_IO_PCMCIA
// Lookup a 'netdev' entry, assuming that it is an ethernet device.
cyg_netdevtab_entry_t * 
eth_drv_netdev(char *name)
{
    cyg_netdevtab_entry_t *t;
    struct eth_drv_sc *sc;
    for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
        sc = (struct eth_drv_sc *)t->device_instance;
        if (strcmp(sc->dev_name, name) == 0) {
            return t;
        }
    }
    return (cyg_netdevtab_entry_t *)NULL;
}
#endif // CYGPKG_IO_PCMCIA

// EOF src/net/eth_drv.c
