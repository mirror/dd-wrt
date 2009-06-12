//==========================================================================
//
//      syntheth.c
//
//      Network device driver for the synthetic target
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002, 2003 Bart Veer
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
// Alternative licenses for eCos may be arranged by contacting the
// copyright holder(s).
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    bartv
// Contributors: bartv
// Date:         2002-08-07
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/devs_eth_ecosynth.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <errno.h>
#include <string.h>

#define __ECOS 1
#include <sys/types.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/eth_drv_stats.h>

#ifdef CYGPKG_NET
# include <net/if.h>
#else
# define IFF_PROMISC 0
#endif

#include <cyg/hal/hal_io.h>
#include "protocol.h"

// ----------------------------------------------------------------------------
// Device instances. The synthetic target ethernet package can support
// up to four ethernet devices, eth0 to eth3. A synth_eth structure
// holds the data that is specific to a given device. Each device
// needs an instance of this structure, followed by ETH_DRV_SC and
// NETDEVTAB_ENTRY macros. Another macro SYNTH_ETH_INSTANCE takes
// care of all that, to avoid unnecessary duplication of code here.
//
// NOTE: unfortunately this involves duplicating the eth_hwr_funs
// structure. This could be eliminated but it would require bypassing
// the ETH_DRV_SC macro.

#define ETHERNET_MINTU 14
#define ETHERNET_MAXTU 1514


typedef struct synth_eth {
    int             synth_id;           // Device id within the auxiliary
    int             up;                 // Has there been a call to start()?
    int             in_send;            // Detect recursive calls
    int             tx_done;
    unsigned long   tx_key;             // Allow mbuf's to be freed
    volatile int    rx_pending;         // There is pending data.
    int             rx_len;             // Length of buffered data.
    unsigned char   MAC[6];             // Obtained from the underlying ethernet device
    cyg_vector_t    interrupt;          // Interrupt number allocated by the auxiliary
    int             multi_supported;    // Does the driver support multicasting?
    cyg_handle_t    interrupt_handle;   // Allow the ISR and DSR to be installed.
    cyg_interrupt   interrupt_data;
    unsigned char   tx_data[ETHERNET_MAXTU];
    unsigned char   rx_data[ETHERNET_MAXTU];
} synth_eth;

#define SYNTH_ETH_INSTANCE( _number_)           \
static synth_eth synth_eth##_number_ = {        \
    synth_id:   -1,                             \
    up:          1,                             \
    in_send:     0,                             \
    tx_done:     0,                             \
    tx_key:      0L,                            \
    rx_pending:  0,                             \
    rx_len:      0,                             \
    MAC:         { 0, 0, 0, 0, 0, 0 },          \
    interrupt:   0,                             \
    interrupt_handle: 0                         \
};                                              \
ETH_DRV_SC(synth_eth_sc##_number_,              \
           (void*) &synth_eth##_number_,        \
           "eth" #_number_,                     \
           synth_eth_start,                     \
           synth_eth_stop,                      \
           synth_eth_ioctl,                     \
           synth_eth_can_send,                  \
           synth_eth_send,                      \
           synth_eth_recv,                      \
           synth_eth_deliver,                   \
           synth_eth_poll,                      \
           synth_eth_intvector);                \
NETDEVTAB_ENTRY(synth_eth_netdev##_number_,     \
                "synth_eth" #_number_,          \
                synth_eth_init,                 \
                &synth_eth_sc##_number_);

#ifdef CYGVAR_DEVS_ETH_ECOSYNTH_ETH0
SYNTH_ETH_INSTANCE(0);
#endif
#ifdef CYGVAR_DEVS_ETH_ECOSYNTH_ETH1
SYNTH_ETH_INSTANCE(1);
#endif
#ifdef CYGVAR_DEVS_ETH_ECOSYNTH_ETH2
SYNTH_ETH_INSTANCE(2);
#endif
#ifdef CYGVAR_DEVS_ETH_ECOSYNTH_ETH3
SYNTH_ETH_INSTANCE(3);
#endif

// ----------------------------------------------------------------------------
// Data transmits.
//
// The eCos application will just send the data to the auxiliary,
// which will in turn pass it on to the rawether utility. There is no
// need for any response. Flow control is implicit: if the eCos
// application tries to send ethernet packets too quickly then those
// get passed on to the auxiliary, which in turn will pass them on to
// the rawether process. If rawether is still busy with the previous
// packet then the auxiliary will block on a pipe write, and in turn
// the eCos application will block. As long as rawether manages to
// complete its operations reasonably quickly these blocks should not
// be noticeable to the user.
//
// So can_send() should always return true for an interface that is up
// and running. The send operation needs to take the sg list, turn it
// into a single buffer, and transmit it to the auxiliary. At that
// point the transmission is already complete so eth_drv_dsr() should
// be called to call deliver() and release the buffer.
//
// However there are some complications. The first is polled operation,
// where eth_drv_dsr() is a no-op and should not really be called at
// all because there are no interrupts going off. The second is that
// calling eth_drv_dsr() directly will cause recursive operation:
// send() -> dsr -> can_send()/send() -> ...
// This is a bad idea, so can_send() has to check that we are not
// already inside a send(). Data transmission will proceed merrily
// once the send has returned.

static int
synth_eth_can_send(struct eth_drv_sc* sc)
{
    synth_eth*  eth = (synth_eth*)(sc->driver_private);
    return synth_auxiliary_running && eth->up && !eth->in_send && !eth->tx_done;
}

static void
synth_eth_send(struct eth_drv_sc* sc,
               struct eth_drv_sg* sg_list, int sg_len, int total_len,
               unsigned long key)
{
    synth_eth*  eth = (synth_eth*)(sc->driver_private);

    CYG_PRECONDITION((total_len >= ETHERNET_MINTU) && (total_len <= ETHERNET_MAXTU), "Only normal-sized ethernet packets are supported");
    CYG_PRECONDITION(!eth->in_send && !eth->tx_done, "Ethernet device must not still be in use for transmits");

    eth->in_send = 1;
    eth->tx_key  = key;
    if (synth_auxiliary_running && eth->up) {
        int i;
        unsigned char* buf = eth->tx_data;
        for (i = 0; i < sg_len; i++) {
            memcpy(buf, (void*) sg_list[i].buf, sg_list[i].len);
            buf += sg_list[i].len;
            CYG_LOOP_INVARIANT(buf <= &(eth->tx_data[ETHERNET_MAXTU]), "sg list must not exceed ethernet MTU");
        }
        CYG_POSTCONDITION(buf == &(eth->tx_data[total_len]), "sg list lengths should match total_len");

        synth_auxiliary_xchgmsg(eth->synth_id, SYNTH_ETH_TX, 0, 0, eth->tx_data, total_len,
                                (void*) 0, (unsigned char*)0, (int*)0, 0);
    }

    // The transfer has now completed, one way or another, so inform
    // the higher-level code immediately.
    eth->tx_done = 1;
    eth_drv_dsr(eth->interrupt, 0, (cyg_addrword_t) sc);
    eth->in_send = 0;
}

// ----------------------------------------------------------------------------
// Receives.
//
// These are rather more complicated because there are real interrupts
// involved, and polling needs to be supported as well. The actual
// transfer of data from auxiliary to eCos happens inside deliver(),
// and the data is buffered up in the synth_eth structure. All that
// needs to be done here is scatter the existing data into the
// sg_list. If higher-level code has run out of space then the
// sg_list may contain null pointers.

static void
synth_eth_recv(struct eth_drv_sc* sc, struct eth_drv_sg* sg_list, int sg_len)
{
    synth_eth*      eth     = (synth_eth*)(sc->driver_private);
    unsigned char*  buf     = eth->rx_data;
    int             len     = eth->rx_len;
    int             i;

    for (i = 0; i < sg_len; i++) {
        if (0 == sg_list[i].buf) {
            break;
        } else if (len <= sg_list[i].len) {
            memcpy((void*)sg_list[i].buf, buf, len);
            break;
        } else {
            memcpy((void*)sg_list[i].buf, buf, sg_list[i].len);
            buf += sg_list[i].len;
            len -= sg_list[i].len;
        }
    }
}

// The ISR does not have to do anything, the DSR does the real work.
static cyg_uint32
synth_eth_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_acknowledge(vector);
    return CYG_ISR_CALL_DSR;
}

// The DSR also does not have to do very much. The data argument is
// actually the eth_drv_sc structure, which must match the vector.
// Interrupts only go off when there are pending receives, so set the
// rx_pending flag and call the generic DSR to do the real work.
static void
synth_eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    struct eth_drv_sc* sc = (struct eth_drv_sc*) data;
    synth_eth* eth        = (synth_eth*)(sc->driver_private);
    CYG_ASSERT(eth->interrupt == vector, "Interrupt vectors cannot change during a run");

    eth->rx_pending = 1;
    eth_drv_dsr(vector, count, data);
}

// ----------------------------------------------------------------------------
// Delivery. This is invoked by a thread inside the TCP/IP stack, or by
// the poll function.
static void
synth_eth_deliver(struct eth_drv_sc* sc)
{
    synth_eth* eth = (synth_eth*)(sc->driver_private);

    if (eth->tx_done) {
        eth->tx_done = 0;
        (*sc->funs->eth_drv->tx_done)(sc, eth->tx_key, 1);
    }
    while (eth->rx_pending) {
        int more = 1;
        eth->rx_pending = 0;

        while (more && eth->up && synth_auxiliary_running) {
            synth_auxiliary_xchgmsg(eth->synth_id, SYNTH_ETH_RX, 0, 0, (void*) 0, 0,
                                    &more, eth->rx_data, &(eth->rx_len), ETHERNET_MAXTU);
            CYG_LOOP_INVARIANT(!more || (0 != eth->rx_len), "Auxiliary must send at least one packet if several are available");
            if (eth->rx_len > 0) {
                CYG_ASSERT((eth->rx_len >= ETHERNET_MINTU) && (eth->rx_len <= ETHERNET_MAXTU), "Only normal-sized ethernet packets are supported");
                // Inform higher-level code that data is available.
                // This should result in a call to recv() with a
                // suitable sg_list. If out of memory, recv()
                // will see a null pointer.
                (*sc->funs->eth_drv->recv)(sc, eth->rx_len);
            }
        };
    }
}


// ----------------------------------------------------------------------------
// Polling support. Transmits are relatively straightforward because
// all the hard work is handled by send(). Receives are rather more
// complicated because interrupts are disabled so we never know when
// there is really pending data. However deliver() will do the right
// thing even if there is no data, so simply faking up an interrupt
// is enough. This does mean extra traffic between application and
// auxiliary, but polling does rather imply that.
static void
synth_eth_poll(struct eth_drv_sc* sc)
{
    synth_eth* eth = (synth_eth*)(sc->driver_private);
    if (synth_auxiliary_running && eth->up) {
        eth->rx_pending = 1;
    }
    synth_eth_deliver(sc);
}

static int
synth_eth_intvector(struct eth_drv_sc* sc)
{
    synth_eth* eth = (synth_eth*)(sc->driver_private);
    return eth->interrupt;
}

// ----------------------------------------------------------------------------
// ioctl()'s.
//
// SET_MAC_ADDRESS is not currently implemented, and probably should
// not be implemented because the underlying ethernet device may not
// support it.
//
// SET_MC_ALL is supported if the underlying hardware does. This is
// needed for IPV6 support. More selective multicasting via
// SET_MC_LIST is not supported, because it imposes too heavy a
// requirement on the underlying Linux device. SET_MC_LIST can be
// used to disable multicast support.
//
// GET_IF_STATS_UD and GET_IF_STATS are not currently implemented
static int
synth_eth_ioctl(struct eth_drv_sc* sc, unsigned long key, void* data, int data_length)
{
    synth_eth* eth = (synth_eth*)(sc->driver_private);
    int result = EINVAL;
    
    switch(key) {
      case ETH_DRV_SET_MC_ALL:
        {
            if (eth->multi_supported) {
                synth_auxiliary_xchgmsg(eth->synth_id, SYNTH_ETH_MULTIALL, 1, 0, (unsigned char*) 0, 0,
                                       (void*)0, (unsigned char*)0, (int*)0, 0);
                result = 0;
            }
            break;
        }
      case ETH_DRV_SET_MC_LIST:
        {
            struct eth_drv_mc_list* mcl = (struct eth_drv_mc_list*) data;
            if (eth->multi_supported && (0 == mcl->len)) {
                synth_auxiliary_xchgmsg(eth->synth_id, SYNTH_ETH_MULTIALL, 0, 0, (unsigned char*) 0, 0,
                                       (void*)0, (unsigned char*)0, (int*)0, 0);
                result = 0;
            }
            break;
        }
      default:
        break;
    }
    
    return result;
}

// ----------------------------------------------------------------------------
// Starting and stopping an interface. This includes restarting in
// promiscuous mode.
static void
synth_eth_start(struct eth_drv_sc* sc, unsigned char* enaddr, int flags)
{
    synth_eth* eth = (synth_eth*)(sc->driver_private);

    eth->up         = 0;
    eth->rx_pending = 0;
    
    if ((-1 != eth->synth_id) && synth_auxiliary_running) {
        synth_auxiliary_xchgmsg(eth->synth_id, SYNTH_ETH_START, flags & IFF_PROMISC, 0, (void*) 0, 0,
                                (int*)0, (void*) 0, (int*) 0, 0);
    }
    eth->up = 1;
    if (enaddr != (unsigned char*)0) {
        memcpy(enaddr, eth->MAC, 6);
    }
}

static void
synth_eth_stop(struct eth_drv_sc* sc)
{
    synth_eth* eth = (synth_eth*)(sc->driver_private);

    eth->up         = 0;
    if ((-1 != eth->synth_id) && synth_auxiliary_running) {
        synth_auxiliary_xchgmsg(eth->synth_id, SYNTH_ETH_STOP, 0, 0, (void*) 0, 0,
                                (int*) 0, (void*) 0, (int*) 0, 0);
    }
    eth->rx_pending = 0;
}


// ----------------------------------------------------------------------------
// Initialization.
//
// This requires instantiating a device of class ethernet with a specific
// device name held in the eth_drv_sc structure. No additional device data
// is needed. If a device can be instantiated then the interrupt vector
// and the MAC address can be obtained, and an interrupt handler can be
// installed.
static bool
synth_eth_init(struct cyg_netdevtab_entry* ndp)
{
    bool result = false;
    struct eth_drv_sc* sc   = (struct eth_drv_sc*)(ndp->device_instance);
    struct synth_eth*  eth  = (struct synth_eth*)(sc->driver_private);

    if (synth_auxiliary_running) {
        eth->synth_id = synth_auxiliary_instantiate("devs/eth/synth/ecosynth", SYNTH_MAKESTRING(CYGPKG_DEVS_ETH_ECOSYNTH), "ethernet",
                                                    sc->dev_name, (const char*) 0);
        
        if (-1 != eth->synth_id) {
            unsigned char data[7];
            result = true;
            synth_auxiliary_xchgmsg(eth->synth_id, SYNTH_ETH_GETPARAMS, 0, 0, (const unsigned char*) 0, 0,
                                    &(eth->interrupt), data, 0, 7);
            memcpy(eth->MAC, data, 6);
            eth->multi_supported = data[6];
            cyg_drv_interrupt_create(eth->interrupt,
                                     0,
                                     (CYG_ADDRWORD) sc,
                                     &synth_eth_isr,
                                     &synth_eth_dsr,
                                     &(eth->interrupt_handle),
                                     &(eth->interrupt_data));
            cyg_drv_interrupt_attach(eth->interrupt_handle);
            cyg_drv_interrupt_unmask(eth->interrupt);
        }
    }
    (*sc->funs->eth_drv->init)(sc, eth->MAC);

#ifdef CYGPKG_NET
    if (eth->multi_supported) {
      sc->sc_arpcom.ac_if.if_flags |= IFF_ALLMULTI;
    }
#endif    
    return result;
}
