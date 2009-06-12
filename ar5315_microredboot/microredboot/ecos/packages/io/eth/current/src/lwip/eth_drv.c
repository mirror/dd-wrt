//==========================================================================
//
//      src/lwip/eth_drv.c
//
//      Hardware independent ethernet driver for lwIP
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    Jani Monoses <jani@iv.ro>
// Contributors: 
// Date:         2002-04-05
// Purpose:      Hardware independent ethernet driver
// Description:  Based on the standalone driver for RedBoot.
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

#include <cyg/hal/hal_tables.h>
#include <cyg/kernel/kapi.h>

#include "lwip/opt.h"
#include "lwip/ip.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "netif/etharp.h"


// Interfaces exported to drivers

static void eth_drv_init(struct eth_drv_sc *sc, unsigned char *enaddr);
static void eth_drv_recv(struct eth_drv_sc *sc, int total_len);
static void eth_drv_tx_done(struct eth_drv_sc *sc, CYG_ADDRWORD key, int status);

struct eth_drv_funs eth_drv_funs = { eth_drv_init, eth_drv_recv, eth_drv_tx_done };

#ifdef CYGDBG_IO_ETH_DRIVERS_DEBUG 
int cyg_io_eth_net_debug = CYGDBG_IO_ETH_DRIVERS_DEBUG_VERBOSITY;
#endif

extern void lwip_dsr_stuff(void);
extern void lwip_set_addr(struct netif *);

//DSR called from the low level driver.Signals the input_thread
void
eth_drv_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
  struct eth_drv_sc *sc = (struct eth_drv_sc *) data;
  sc->state |= ETH_DRV_NEEDS_DELIVERY;
  lwip_dsr_stuff();	
}

err_t ecosif_init(struct netif *netif);

// This function is called during system initialization to register a
// network interface with the system.
static void
eth_drv_init(struct eth_drv_sc *sc, unsigned char *enaddr)
{
  struct netif *netif = &sc->sc_arpcom.ac_if;
  
  netif->state = sc;
  ecosif_init(netif);
  
  // enaddr == 0 -> hardware init was incomplete (no ESA)
  if (enaddr != 0) {
    // Set up hardware address
    memcpy(netif->hwaddr, enaddr, ETHER_ADDR_LEN);
    // Perform any hardware initialization
    (sc->funs->start) (sc, (unsigned char *) &netif->hwaddr, 0);
  }
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

#endif

//
// Send a packet of data to the hardware
//

static void
eth_drv_send(struct netif *netif, struct pbuf *p)
{
  struct eth_drv_sg sg_list[MAX_ETH_DRV_SG];
  struct eth_drv_sc *sc = netif->state;
  int sg_len = 0;
  struct pbuf *q;

#ifdef _LOCK_WITH_ROM_MONITOR
    bool need_lock = false;
    int debug_chan;
#endif

  while (!(sc->funs->can_send) (sc)); 

  for (q = p; q != NULL; q = q->next) {
    sg_list[sg_len].buf = (CYG_ADDRESS) q->payload;
    sg_list[sg_len++].len = q->len;
  }
#ifdef _LOCK_WITH_ROM_MONITOR
  debug_chan = CYGACC_CALL_IF_SET_DEBUG_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
  if (debug_chan == RedBoot_TCP_CHANNEL) {
    need_lock = true;
    cyg_drv_dsr_lock();
  }
#endif // _LOCK_WITH_ROM_MONITOR

  (sc->funs->send) (sc, sg_list, sg_len, p->tot_len,
		    (CYG_ADDRWORD) p);

#ifdef _LOCK_WITH_ROM_MONITOR
  // Unlock the driver & hardware.  It can once again be safely shared.
  if (need_lock) {
    cyg_drv_dsr_unlock();
  }
#endif // _LOCK_WITH_ROM_MONITOR

}

//
// This function is called from the hardware driver when an output operation
// has completed - i.e. the packet has been sent.
//
static void
eth_drv_tx_done(struct eth_drv_sc *sc, CYG_ADDRWORD key, int status)
{
#if 0	
  struct pbuf *p = (struct pbuf *)key;
  struct netif *netif = &sc->sc_arpcom.ac_if;

  CYGARC_HAL_SAVE_GP();
  CYGARC_HAL_RESTORE_GP();
#endif  
}

static void ecosif_input(struct netif *netif, struct pbuf* pbuf);

#define MAX_ETH_MSG 1540
//
// This function is called from a hardware driver to indicate that an input
// packet has arrived.  The routine will set up appropriate network resources
// to hold the data and call back into the driver to retrieve the data.
//
static void
eth_drv_recv(struct eth_drv_sc *sc, int total_len)
{
  struct eth_drv_sg sg_list[MAX_ETH_DRV_SG];
  struct netif *netif = &sc->sc_arpcom.ac_if;

  struct pbuf *p, *q;

  int sg_len = 0;
  CYGARC_HAL_SAVE_GP();

  if ((total_len > MAX_ETH_MSG) || (total_len < 0)) {
    total_len = MAX_ETH_MSG;
  }

  p = pbuf_alloc(PBUF_RAW, total_len, PBUF_POOL);

  if (p == NULL) {
    LWIP_DEBUGF(0, ("ecosif_input: low_level_input returned NULL\n"));
    return;
  }

  for (q = p; q != NULL; q = q->next) {
    sg_list[sg_len].buf = (CYG_ADDRESS) q->payload;
    sg_list[sg_len++].len = q->len;
  }
  (sc->funs->recv) (sc, sg_list, sg_len);
  ecosif_input(netif, p);
  CYGARC_HAL_RESTORE_GP();
}


#define IFNAME0 'e'
#define IFNAME1 't'



//
// low_level_output():
//
// Should do the actual transmission of the packet. The packet is
// contained in the pbuf that is passed to the function. This pbuf
// might be chained.We pass the data down to the eCos hw independent 
// ethernet driver
//

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  eth_drv_send(netif, p);
  return ERR_OK;
}

//
// ecosif_output():
//
// This function is called by the TCP/IP stack when an IP packet
// should be sent. It calls the function called low_level_output() to
// do the actual transmission of the packet.
//
//
static err_t
ecosif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{

  p = etharp_output(netif, ipaddr, p);
  if (p) {
    low_level_output(netif, p);
    p = NULL;
  }
  return ERR_OK;
}



//
// ecosif_input():
// This function is called when the eCos hw independent driver
// has some data to pass up to lwIP.It does it through ecosif_input.
//
static void
ecosif_input(struct netif *netif, struct pbuf *p)
{
  struct eth_hdr *ethhdr;
  
  ethhdr = p->payload;

  switch (htons(ethhdr->type)) {
  case ETHTYPE_IP:
    LWIP_DEBUGF(0, ("ecosif_input: IP packet\n"));
    etharp_ip_input(netif, p);
    pbuf_header(p, -14);
    netif->input(p, netif);
    break;
  case ETHTYPE_ARP:
    LWIP_DEBUGF(0, ("ecosif_input: ARP packet\n"));
    etharp_arp_input(netif, (struct eth_addr *) &netif->hwaddr, p);
    break;
  default:
    pbuf_free(p);
    break;
  }

}

err_t
ecosif_init(struct netif *netif)
{
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->hwaddr_len = 6;
  netif->output = ecosif_output;
  netif->linkoutput = low_level_output;
  netif->mtu = 1500;
  lwip_set_addr(netif);
  return ERR_OK;
}
