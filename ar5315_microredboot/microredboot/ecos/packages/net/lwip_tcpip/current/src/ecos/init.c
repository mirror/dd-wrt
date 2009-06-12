//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 eCosCentric 
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================

/*
 * init.c - misc lwip ecos glue functions 
 */
#include <pkgconf/system.h>
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#if LWIP_SLIP
#include "netif/slipif.h"
#endif

#if PPP_SUPPORT
#include "netif/ppp/ppp.h"
#endif

#include "netif/loopif.h"
#include <cyg/hal/hal_if.h>
#include <cyg/infra/diag.h>

#ifdef CYGPKG_LWIP_ETH
#include "netif/etharp.h"

#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>


// Define table boundaries
CYG_HAL_TABLE_BEGIN(__NETDEVTAB__, netdev);
CYG_HAL_TABLE_END(__NETDEVTAB_END__, netdev);
static void ecosglue_init(void);
#endif

void inline IP_ADDR(struct ip_addr *ipaddr, char a, char b, char c, char d)
{
	IP4_ADDR(ipaddr,a,b,c,d);
}


void tcpip_init_done(void * arg)
{
	sys_sem_t *sem = arg;
	sys_sem_signal(*sem);
}

struct netif mynetif, loopif;
void lwip_set_addr(struct netif *netif);
#if PPP_SUPPORT
#define PPP_USER "pppuser"
#define PPP_PASS "ppppass"

void 
pppMyCallback(void *a , int e, void * arg)
{
	diag_printf("callback %d \n",e);
}

/* These temporarily here */
unsigned long
sys_jiffies(void)
{
   return cyg_current_time();
}

void 
ppp_trace(int level, const char *format,...)
{
    va_list args;

    (void)level;
    va_start(args, format);
    diag_vprintf(format, args);
    va_end(args);
}	
#endif

#if LWIP_HAVE_LOOPIF
struct netif ecos_loopif;
#endif

/*
 * Called by the eCos application at startup
 * wraps various init calls
 */
int
lwip_init(void)
{
	struct ip_addr ipaddr, netmask, gw;
	static int inited = 0;
	sys_sem_t sem;
	if (inited)
		return 1;
	inited++;
	
	sys_init();	/* eCos specific initialization */
	mem_init();	/* heap based memory allocator */
	memp_init();	/* pool based memory allocator */
	pbuf_init();	/* packet buffer allocator */
	netif_init();	/* netif layer */
	
	/* Start the stack.It will spawn a new dedicated thread */
	sem = sys_sem_new(0);
	tcpip_init(tcpip_init_done,&sem);
	sys_sem_wait(sem);
	sys_sem_free(sem);

#if LWIP_HAVE_LOOPIF
	IP4_ADDR(&gw, 127,0,0,1);
	IP4_ADDR(&ipaddr, 127,0,0,1);
	IP4_ADDR(&netmask, 255,0,0,0);
  
	netif_add(&ecos_loopif, &ipaddr, &netmask, &gw, NULL, loopif_init,
	    tcpip_input);
#endif
	
#if LWIP_SLIP	
	lwip_set_addr(&mynetif);
	slipif_init(&mynetif);
	netif_set_default(&mynetif);
#elif PPP_SUPPORT
	pppInit();
#if PAP_SUPPORT || CHAP_SUPPORT
	pppSetAuth(PPPAUTHTYPE_PAP, PPP_USER, PPP_PASS);
#endif
	pppOpen(sio_open(2), pppMyCallback, NULL);
#else	
	ecosglue_init();		
#endif	
	return 0;
}

void
lwip_set_addr(struct netif *netif)
{
	struct ip_addr ipaddr, netmask, gw;

	IP_ADDR(&gw, CYGPKG_LWIP_SERV_ADDR);
	IP_ADDR(&ipaddr, CYGPKG_LWIP_MY_ADDR);
	IP_ADDR(&netmask, CYGPKG_LWIP_NETMASK);
	netif_set_addr(netif, &ipaddr, &netmask, &gw);
	netif->next = netif_list;
	netif_list = netif;
	
	netif->input = tcpip_input;
	//netif->input = ip_input;
}

#ifdef CYGPKG_LWIP_ETH
//io eth stuff

cyg_sem_t delivery;

void
lwip_dsr_stuff(void)
{
  cyg_semaphore_post(&delivery);
}

//Input thread signalled by DSR calls deliver() on low level drivers
static void
input_thread(void *arg)
{
  cyg_netdevtab_entry_t *t;

  for (;;) {
    cyg_semaphore_wait(&delivery);

    for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
      struct eth_drv_sc *sc = (struct eth_drv_sc *)t->device_instance;
      if (sc->state & ETH_DRV_NEEDS_DELIVERY) {
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
        cyg_bool was_ctrlc_int;
#endif
	sc->state &= ~ETH_DRV_NEEDS_DELIVERY;
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
        was_ctrlc_int = HAL_CTRLC_CHECK((*sc->funs->int_vector)(sc), (int)sc);
          if (!was_ctrlc_int) // Fall through and run normal code
		  
#endif
	(sc->funs->deliver) (sc);
      }
    }
  }

}

// Initialize all network devices
static void
init_hw_drivers(void)
{
  cyg_netdevtab_entry_t *t;

  for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
    if (t->init(t)) {
      t->status = CYG_NETDEVTAB_STATUS_AVAIL;
    } else {
      // What to do if device init fails?
      t->status = 0;		// Device not [currently] available
    }
  }
}

static void
arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
}


static void
ecosglue_init(void)
{
  cyg_semaphore_init(&delivery, 0);
  init_hw_drivers();
  sys_thread_new(input_thread, (void*)0, CYGPKG_LWIP_ETH_THREAD_PRIORITY);
  etharp_init();
  sys_timeout(ARP_TMR_INTERVAL, (sys_timeout_handler) arp_timer, NULL);
}

#endif //CYGPKG_LWIP_ETH
