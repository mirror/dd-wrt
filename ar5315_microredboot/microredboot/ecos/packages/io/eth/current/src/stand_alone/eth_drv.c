//==========================================================================
//
//      src/stand_alone/eth_drv.c
//
//      Stand-alone hardware independent networking support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>
#include <string.h>

// High-level ethernet driver


// Interfaces exported to drivers

static void eth_drv_init(struct eth_drv_sc *sc, unsigned char *enaddr);
static void eth_drv_recv(struct eth_drv_sc *sc, int total_len);
static void eth_drv_tx_done(struct eth_drv_sc *sc, CYG_ADDRWORD key, int status);

struct eth_drv_funs eth_drv_funs = {eth_drv_init, eth_drv_recv, eth_drv_tx_done};

#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
int cyg_io_eth_net_debug = CYGDBG_IO_ETH_DRIVERS_DEBUG_VERBOSITY;
// Usually just the header is enough, the body slows things too much.
#define DIAG_DUMP_BUF_HDR( a, b ) if (0 < cyg_io_eth_net_debug) diag_dump_buf( (a), (b) )
#define DIAG_DUMP_BUF_BDY( a, b ) if (1 < cyg_io_eth_net_debug) diag_dump_buf( (a), (b) )
#else
#define DIAG_DUMP_BUF_HDR( a, b )
#define DIAG_DUMP_BUF_BDY( a, b )
#endif

struct eth_drv_sc *__local_enet_sc = NULL;

#ifdef CYGSEM_IO_ETH_DRIVERS_PASS_PACKETS
//
// Horrible hack: In order to allow the stand-alone networking code to work
// alongside eCos (or any other stack), separate IP addresses must be used.
// When a packet arrives at the interface, we check to see which IP address
// it corresponds to and only pass it "up" if it's not for the stand-alone
// layer.
//
// tres degolas :-(
//
extern char __local_ip_addr[4]; 
#endif // PASS_PACKETS

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
//
// Another horrible hack: In order to do the above passing on of
// application packets safely - and passing on completion events for
// pending transmissions (which is not conditional) - we must lock the
// application scheduler before calling into it.  There are several reasons
// for this: a) We are likely running on a RedBoot special debug stack and
// so the application's stack checking fires; b) we could even get
// descheduled if the arrival of a packet causes a higher priority thread
// to awaken!

#include <cyg/hal/dbg-threads-api.h>

// Use with care!  Local variable defined!
# define    LOCK_APPLICATION_SCHEDULER()                                \
{   /* NEW BLOCK */                                                     \
    threadref currthread;                                               \
    int threadok;                                                       \
    threadok = dbg_currthread( &currthread );                           \
    if ( threadok ) {                                                   \
        threadok = dbg_scheduler( &currthread, 1, 1 ); /* lock */       \
    }

# define  UNLOCK_APPLICATION_SCHEDULER()                        	\
    if ( threadok ) {                                           	\
        dbg_scheduler( &currthread, 0, 1 ); /* unlock */        	\
    }                                                           	\
}   /* END BLOCK */

#else
# define    LOCK_APPLICATION_SCHEDULER() CYG_EMPTY_STATEMENT
# define  UNLOCK_APPLICATION_SCHEDULER() CYG_EMPTY_STATEMENT
#endif // GDB_THREAD_SUPPORT

//
// Buffer 'get' support.  The problem is that this function only gets
// called when some data is required, but packets may arrive on the device
// at any time.  More particularly, on some devices when data arrive, all
// of that data needs to be consumed immediately or be lost.  This process
// is driven by interrupts, which in the stand-along case are simulated by
// calling the "poll" interface.
//
// Thus there will be a pool of buffers, some free and some full, to try
// and manage this.
//

#define MAX_ETH_MSG 1540
#define NUM_ETH_MSG CYGNUM_IO_ETH_DRIVERS_NUM_PKT

struct eth_msg {
    struct eth_msg *next, *prev;
    int len;   // Actual number of bytes in message
    unsigned char data[MAX_ETH_MSG];
};

struct eth_msg_hdr {
    struct eth_msg *first, *last;
};

static struct eth_msg_hdr eth_msg_free, eth_msg_full;
static struct eth_msg eth_msgs[NUM_ETH_MSG];

// Prototypes for functions used in this module
static void eth_drv_start(struct eth_drv_sc *sc);

// These functions are defined in RedBoot and control access to
// the "default" console.
extern int  start_console(void);
extern void end_console(int);

// Simple queue management functions

static void
eth_drv_msg_put(struct eth_msg_hdr *hdr, struct eth_msg *msg)
{
    if (hdr->first != (struct eth_msg *)hdr) {
        // Something already in queue
        hdr->last->next = msg;
        msg->prev = hdr->last;
        msg->next = (struct eth_msg *)hdr;
        hdr->last = msg;
    } else {
        hdr->first = hdr->last = msg;
        msg->next = msg->prev = (struct eth_msg *)hdr;
    }
}

static struct eth_msg *
eth_drv_msg_get(struct eth_msg_hdr *hdr)
{
    struct eth_msg *msg;
    if (hdr->first != (struct eth_msg *)hdr) {
        msg = hdr->first;
        hdr->first = msg->next;
        msg->next->prev = (struct eth_msg *)hdr;
    } else {
        msg = (struct eth_msg *)NULL;
    }
    return msg;
}

void
eth_drv_buffers_init(void)
{
    int i;
    struct eth_msg *msg = eth_msgs;

    eth_msg_full.first = eth_msg_full.last = (struct eth_msg *)&eth_msg_full;
    eth_msg_free.first = eth_msg_free.last = (struct eth_msg *)&eth_msg_free;
    for (i = 0;  i < NUM_ETH_MSG;  i++, msg++) {
        eth_drv_msg_put(&eth_msg_free, msg);
    }
}

//
// This function is called during system initialization to register a
// network interface with the system.
//
static void
eth_drv_init(struct eth_drv_sc *sc, unsigned char *enaddr)
{
    // enaddr == 0 -> hardware init was incomplete (no ESA)
    if (enaddr != 0) {
        // Set up hardware address
        memcpy(&sc->sc_arpcom.esa, enaddr, ETHER_ADDR_LEN);
        __local_enet_sc = sc;
        eth_drv_start(sc);
    }
}

//
// This [internal] function will be called to stop activity on an interface.
//
void
eth_drv_stop(void)
{
    struct eth_drv_sc *sc = __local_enet_sc;

    if (sc != NULL)
        (sc->funs->stop)(sc);
}

//
// This [internal] function will be called to start activity on an interface.
//
static void
eth_drv_start(struct eth_drv_sc *sc)
{
    // Perform any hardware initialization
    (sc->funs->start)(sc, (unsigned char *)&sc->sc_arpcom.esa, 0);
}

//
// Send a packet of data to the hardware
//
static int packet_sent;

void
eth_drv_write(char *eth_hdr, char *buf, int len)
{
    struct eth_drv_sg sg_list[MAX_ETH_DRV_SG];
    struct eth_drv_sc *sc = __local_enet_sc;
    int sg_len = 2;
    void *dbg = CYGACC_CALL_IF_DBG_DATA();
    int old_state;
    int wait_time = 5;  // Timeout before giving up
    void *eth_drv_old = 0;

    if (dbg) {
        sc = (struct eth_drv_sc *)dbg;  // Use control from installed driver
        eth_drv_old = sc->funs->eth_drv_old;
        if (eth_drv_old == 0) {
            sc->funs->eth_drv_old = sc->funs->eth_drv;        
            sc->funs->eth_drv = &eth_drv_funs;    // Substitute stand-alone driver
            old_state = sc->state;
            if (!(old_state & ETH_DRV_STATE_ACTIVE)) {
                // This interface not fully initialized, do it now
                (sc->funs->start)(sc, (unsigned char *)sc->sc_arpcom.esa, 0);
                sc->state |= ETH_DRV_STATE_ACTIVE;
            }
        }
    }

    while (!(sc->funs->can_send)(sc)) {
        // Give driver a chance to service hardware
        (sc->funs->poll)(sc);
        CYGACC_CALL_IF_DELAY_US(2*100000);
        if (--wait_time <= 0)
            goto reset_and_out;  // Give up on sending packet
    }

    sg_list[0].buf = (CYG_ADDRESS)eth_hdr;
    sg_list[0].len = (6+6+2);  // FIXME
    sg_list[1].buf = (CYG_ADDRESS)buf;
    sg_list[1].len = len;
    packet_sent = 0;
#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
    if (cyg_io_eth_net_debug) {
        int old_console;
        old_console = start_console();
        diag_printf("Ethernet send:\n");
        DIAG_DUMP_BUF_HDR(eth_hdr, 14);
        DIAG_DUMP_BUF_BDY(buf, len);
        end_console(old_console);
    }
#endif

    (sc->funs->send)(sc, sg_list, sg_len, len+14, (CYG_ADDRWORD)&packet_sent);

    wait_time = 50000;
    while (1) {
        (sc->funs->poll)(sc);

	if(packet_sent)
	    break;
	
        CYGACC_CALL_IF_DELAY_US(2*10);
        if (--wait_time <= 0)
            goto reset_and_out;  // Give up on sending packet
    }
 reset_and_out:   
    if (dbg) {
//        if (!(old_state & ETH_DRV_STATE_ACTIVE)) {
//            // This interface was not fully initialized, shut it back down
//            (sc->funs->stop)(sc);
//        }
        if (eth_drv_old == 0) {
            sc->funs->eth_drv = sc->funs->eth_drv_old;
            sc->funs->eth_drv_old = (struct eth_drv_funs *)0;
        }
    }
}

//
// This function is called from the hardware driver when an output operation
// has completed - i.e. the packet has been sent.
//
static void
eth_drv_tx_done(struct eth_drv_sc *sc, CYG_ADDRWORD key, int status)
{
    CYGARC_HAL_SAVE_GP();
    if ((int *)key == &packet_sent) {
        *(int *)key = 1;
    } else {
        // It's possible that this acknowledgement is for a different
        // [logical] driver.  Try and pass it on.
#if defined(CYGDBG_IO_ETH_DRIVERS_DEBUG) && \
           (CYGDBG_IO_ETH_DRIVERS_DEBUG_VERBOSITY >=2 )
        // Note: not normally enabled - too verbose
        if (cyg_io_eth_net_debug > 1) {
            int old_console;
            old_console = start_console();
            diag_printf("tx_done for other key: %x\n", key);
            end_console(old_console);
        }
#endif
        LOCK_APPLICATION_SCHEDULER();
        if (sc->funs->eth_drv_old) {
            (sc->funs->eth_drv_old->tx_done)(sc, key, status);
        } else {
            (sc->funs->eth_drv->tx_done)(sc, key, status);
        }
        UNLOCK_APPLICATION_SCHEDULER();
    }
    CYGARC_HAL_RESTORE_GP();
}

//
// Receive one packet of data from the hardware, if available
//
int
eth_drv_read(char *eth_hdr, char *buf, int len)
{
    struct eth_drv_sc *sc = __local_enet_sc;
    struct eth_msg *msg;
    int res;
    void *dbg = CYGACC_CALL_IF_DBG_DATA();
    int old_state;
    void *eth_drv_old = 0;

    if (dbg) {
        sc = (struct eth_drv_sc *)dbg;  // Use control from installed driver
        eth_drv_old = sc->funs->eth_drv_old;
        if (eth_drv_old == 0) {
            sc->funs->eth_drv_old = sc->funs->eth_drv;
            sc->funs->eth_drv = &eth_drv_funs;    // Substitute stand-alone driver
            old_state = sc->state;
            if (!(old_state & ETH_DRV_STATE_ACTIVE)) {
                // This interface not fully initialized, do it now
                (sc->funs->start)(sc, (unsigned char *)sc->sc_arpcom.esa, 0);
                sc->state |= ETH_DRV_STATE_ACTIVE;
            }
        }
    }
    (sc->funs->poll)(sc);  // Give the driver a chance to fetch packets
    msg = eth_drv_msg_get(&eth_msg_full);
    if (msg && len >= msg->len - 14) {
        memcpy(eth_hdr, msg->data, 14);
        memcpy(buf, &msg->data[14], msg->len-14);
        res = msg->len;
    } else {
        res = 0;
    }
    if (msg) {
        eth_drv_msg_put(&eth_msg_free, msg);
    }
   
    if (dbg) {
        if (eth_drv_old == 0) {
            sc->funs->eth_drv = sc->funs->eth_drv_old;
            sc->funs->eth_drv_old = (struct eth_drv_funs *)0;
        }
//        if (!old_state & ETH_DRV_STATE_ACTIVE) {
//            // This interface was not fully initialized, shut it back down
//            (sc->funs->stop)(sc);
//        }
    }
    return res;
}

#ifdef CYGSEM_IO_ETH_DRIVERS_PASS_PACKETS
//
// This function is called to copy a message up to the next level.
// It is only used when this driver has usurped the processing of
// network functions.
//
static unsigned char *eth_drv_copy_recv_buf;
static void 
eth_drv_copy_recv(struct eth_drv_sc *sc,
                  struct eth_drv_sg *sg_list,
                  int sg_len)
{
    int i;
    unsigned char *ppp;
    CYGARC_HAL_SAVE_GP();
    ppp = eth_drv_copy_recv_buf;        // Be safe against being called again by accident
    for (i = 0;  i < sg_len;  i++) {
        if ( sg_list[i].buf )           // Be safe against discarding calls
            memcpy((unsigned char *)sg_list[i].buf, 
                   ppp, sg_list[i].len);
        ppp += sg_list[i].len;
    }
    CYGARC_HAL_RESTORE_GP();
}
#endif

//
// This function is called from a hardware driver to indicate that an input
// packet has arrived.  The routine will set up appropriate network resources
// to hold the data and call back into the driver to retrieve the data.
//
static void
eth_drv_recv(struct eth_drv_sc *sc, int total_len)
{
    struct eth_drv_sg sg_list[MAX_ETH_DRV_SG];
    int               sg_len = 0;
    struct eth_msg *msg;
    unsigned char *buf;
    CYGARC_HAL_SAVE_GP();

    if ((total_len > MAX_ETH_MSG) || (total_len < 0)) {        
#ifdef CYGSEM_IO_ETH_DRIVERS_WARN
        int old_console;
        old_console = start_console();
        diag_printf("%s: packet of %d bytes truncated\n", __FUNCTION__, total_len);
        end_console(old_console);
#endif
        total_len = MAX_ETH_MSG;
    }
    msg = eth_drv_msg_get(&eth_msg_free);
    if (msg) {
        buf = msg->data;
    } else {
#ifdef CYGSEM_IO_ETH_DRIVERS_WARN
        int old_console;
        old_console = start_console();
        diag_printf("%s: packet of %d bytes dropped\n", __FUNCTION__, total_len);
        end_console(old_console);
#endif
        buf = (unsigned char *)0;  // Drivers know this means "the bit bucket"
    }
    sg_list[0].buf = (CYG_ADDRESS)buf;
    sg_list[0].len = total_len;
    sg_len = 1;

    (sc->funs->recv)(sc, sg_list, sg_len);
#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG
    if (cyg_io_eth_net_debug) {
        int old_console;
        old_console = start_console();
        diag_printf("Ethernet recv:\n");
        if ( buf ) {
            DIAG_DUMP_BUF_HDR(buf, 14);
            DIAG_DUMP_BUF_BDY(buf+14, total_len-14);
        }
        else
            diag_printf("  ...NULL buffer.\n");
        end_console(old_console);
    }
#endif
#ifdef CYGSEM_IO_ETH_DRIVERS_PASS_PACKETS
    if ((unsigned char *)0 != buf &&    // Only pass on a packet we actually got!
        sc->funs->eth_drv_old != (struct eth_drv_funs *)0) {
        void (*hold_recv)(struct eth_drv_sc *sc,
                          struct eth_drv_sg *sg_list,
                          int sg_len);
        // See if this packet was for us.  If not, pass it upwards
        // This is a major layering violation!!
        if (memcmp(&__local_ip_addr, &buf[14+16], 4)) {
            hold_recv = sc->funs->recv;
            sc->funs->recv = eth_drv_copy_recv;
            eth_drv_copy_recv_buf = buf;
            // This calls into the 'other' driver, giving it a chance to
            // do something with this data (since it wasn't for us)
            LOCK_APPLICATION_SCHEDULER();
            (sc->funs->eth_drv_old->recv)(sc, total_len);
            UNLOCK_APPLICATION_SCHEDULER();
            sc->funs->recv = hold_recv;
        }
    }
#endif
    if (msg) {
        msg->len = total_len;
        eth_drv_msg_put(&eth_msg_full, msg);
#ifdef CYGSEM_IO_ETH_DRIVERS_WARN
    // there was an else with a dump_buf() here but it's
    // meaningless; sg_list[0].buf is NULL!
#endif
    }
    CYGARC_HAL_RESTORE_GP();
}

//
// Determine the interrupt vector used by an interface
//
int
eth_drv_int_vector(void)
{
    struct eth_drv_sc *sc = __local_enet_sc;
    return sc->funs->int_vector(sc);
}


void eth_drv_dsr(cyg_vector_t vector,
                 cyg_ucount32 count,
                 cyg_addrword_t data)
{
    diag_printf("eth_drv_dsr should not be called: vector %d, data %x\n",
                vector, data );
}


// EOF src/stand_alone/eth_drv.c
