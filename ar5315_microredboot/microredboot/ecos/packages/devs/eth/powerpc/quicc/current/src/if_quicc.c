//==========================================================================
//
//      dev/if_quicc.c
//
//      Ethernet device driver for PowerPC QUICC (MPC8xx) boards
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Contributors: gthomas, nickg
// Date:         2000-01-10
// Purpose:      
// Description:  hardware driver for MPC8xx QUICC
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet device driver for MPC8xx QUICC

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_powerpc_quicc.h>
#include <pkgconf/io_eth_drivers.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#endif

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>

#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include "quicc_eth.h"

static unsigned char quicc_eth_rxbufs[CYGNUM_DEVS_ETH_POWERPC_QUICC_RxNUM]
                                     [CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static unsigned char quicc_eth_txbufs[CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM]
                                     [CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE]  __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));

static struct quicc_eth_info quicc_eth0_info;
static unsigned char _default_enaddr[] = { 0x08, 0x00, 0x3E, 0x28, 0x79, 0xB8};
static unsigned char enaddr[6];
#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("Network hardware address [MAC]",
                      quicc_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
#endif
#endif

// For fetching the ESA from RedBoot
#include <cyg/hal/hal_if.h>
#ifndef CONFIG_ESA
#define CONFIG_ESA 6
#endif

ETH_DRV_SC(quicc_eth0_sc,
           &quicc_eth0_info,   // Driver specific data
           "eth0",             // Name for this interface
           quicc_eth_start,
           quicc_eth_stop,
           quicc_eth_control,
           quicc_eth_can_send,
           quicc_eth_send,
           quicc_eth_recv,
           quicc_eth_deliver,
           quicc_eth_int,
           quicc_eth_int_vector);

NETDEVTAB_ENTRY(quicc_netdev, 
                "quicc_eth", 
                quicc_eth_init, 
                &quicc_eth0_sc);

// LED activity [exclusive of hardware bits]
#ifndef _get_led
#define _get_led()  
#define _set_led(v) 
#endif
#ifndef LED_TxACTIVE
#define LED_TxACTIVE  7
#define LED_RxACTIVE  6
#define LED_IntACTIVE 5
#endif

static void
set_led(int bit)
{
  _set_led(_get_led() | (1<<bit));
}

static void
clear_led(int bit)
{
  _set_led(_get_led() & ~(1<<bit));
}

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
static cyg_interrupt quicc_eth_interrupt;
static cyg_handle_t  quicc_eth_interrupt_handle;
#endif
static void          quicc_eth_int(struct eth_drv_sc *data);
static void          quicc_eth_command(struct eth_drv_sc *sc, unsigned long cmd);

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
// This ISR is called when the ethernet interrupt occurs
static int
quicc_eth_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_mask(QUICC_ETH_INT);
    cyg_drv_interrupt_acknowledge(QUICC_ETH_INT);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}
#endif

// Deliver function (ex-DSR) handles the ethernet [logical] processing
static void
quicc_eth_deliver(struct eth_drv_sc * sc)
{
    quicc_eth_int(sc);
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(QUICC_ETH_INT);
#endif
}

//
// Initialize the interface - performed at system startup
// This function must set up the interface, including arranging to
// handle interrupts, etc, so that it may be "started" cheaply later.
//
static bool 
quicc_eth_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    struct cp_bufdesc *rxbd, *txbd;
    unsigned char *RxBUF, *TxBUF, *ep, *ap; 
    volatile struct ethernet_pram *enet_pram;
    volatile struct scc_regs *scc;
    int TxBD, RxBD;
    int cache_state;
    int i;
    bool esa_ok = false;

#ifdef QUICC_ETH_FETCH_ESA
    QUICC_ETH_FETCH_ESA(esa_ok);
#endif

    if (!esa_ok) {
#if defined(CYGPKG_REDBOOT) && \
    defined(CYGSEM_REDBOOT_FLASH_CONFIG)
        esa_ok = flash_get_config("quicc_esa", enaddr, CONFIG_ESA);
#else
        esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
                                             "quicc_esa", enaddr, CONFIG_ESA);
#endif
        if (!esa_ok) {
            // Can't figure out ESA
            diag_printf("QUICC_ETH - Warning! ESA unknown\n");
            memcpy(&enaddr, &_default_enaddr, sizeof(enaddr));
        }
    }

    // Ensure consistent state between cache and what the QUICC sees
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
    }

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Set up to handle interrupts
    cyg_drv_interrupt_create(QUICC_ETH_INT,
                             CYGARC_SIU_PRIORITY_HIGH,
                             (cyg_addrword_t)sc, //  Data item passed to interrupt handler
                             (cyg_ISR_t *)quicc_eth_isr,
                             (cyg_DSR_t *)eth_drv_dsr,
                             &quicc_eth_interrupt_handle,
                             &quicc_eth_interrupt);
    cyg_drv_interrupt_attach(quicc_eth_interrupt_handle);
    cyg_drv_interrupt_acknowledge(QUICC_ETH_INT);
    cyg_drv_interrupt_unmask(QUICC_ETH_INT);
#endif

    qi->pram = enet_pram = &eppc->pram[QUICC_ETH_SCC].enet_scc;
    qi->ctl = scc = &eppc->scc_regs[QUICC_ETH_SCC];  // Use SCCx

    // Shut down ethernet, in case it is already running
    scc->scc_gsmr_l &= ~(QUICC_SCC_GSML_ENR | QUICC_SCC_GSML_ENT);

    memset((void *)enet_pram, 0, sizeof(*enet_pram));

    TxBD = _mpc8xx_allocBd(CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM * sizeof(struct cp_bufdesc));
    RxBD = _mpc8xx_allocBd(CYGNUM_DEVS_ETH_POWERPC_QUICC_RxNUM * sizeof(struct cp_bufdesc));

    txbd = (struct cp_bufdesc *)((char *)eppc + TxBD);
    rxbd = (struct cp_bufdesc *)((char *)eppc + RxBD);
    qi->tbase = txbd;
    qi->txbd = txbd;    
    qi->tnext = txbd;
    qi->rbase = rxbd;
    qi->rxbd = rxbd;
    qi->rnext = rxbd;
    qi->txactive = 0;

    RxBUF = &quicc_eth_rxbufs[0][0];
    TxBUF = &quicc_eth_txbufs[0][0];

    // setup buffer descriptors
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_QUICC_RxNUM;  i++) {
        rxbd->length = 0;
        rxbd->buffer = RxBUF;
        rxbd->ctrl   = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int;
        RxBUF += CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;
        rxbd++;
    }
    rxbd--;
    rxbd->ctrl |= QUICC_BD_CTL_Wrap;  // Last buffer
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM;  i++) {
        txbd->length = 0;
        txbd->buffer = TxBUF;
        txbd->ctrl   = 0;
        TxBUF += CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;
        txbd++;
    }
    txbd--;
    txbd->ctrl |= QUICC_BD_CTL_Wrap;  // Last buffer

    // Set up parallel ports for connection to ethernet tranceiver
    eppc->pio_papar |= (QUICC_ETH_PA_RXD | QUICC_ETH_PA_TXD);
    eppc->pio_padir &= ~(QUICC_ETH_PA_RXD | QUICC_ETH_PA_TXD);
    eppc->pio_paodr &= ~QUICC_ETH_PA_TXD;

    eppc->pio_pcpar &= ~(QUICC_ETH_PC_COLLISION | QUICC_ETH_PC_Rx_ENABLE);
    eppc->pio_pcdir &= ~(QUICC_ETH_PC_COLLISION | QUICC_ETH_PC_Rx_ENABLE);
    eppc->pio_pcso  |= (QUICC_ETH_PC_COLLISION | QUICC_ETH_PC_Rx_ENABLE);

    eppc->pio_papar |= (QUICC_ETH_PA_Tx_CLOCK | QUICC_ETH_PA_Rx_CLOCK);
    eppc->pio_padir &= ~(QUICC_ETH_PA_Tx_CLOCK | QUICC_ETH_PA_Rx_CLOCK);

    // Set up clock routing
    eppc->si_sicr &= ~QUICC_ETH_SICR_MASK;
    eppc->si_sicr |= QUICC_ETH_SICR_ENET;
    eppc->si_sicr &= ~QUICC_ETH_SICR_ENABLE;

    // Set up DMA mode
    eppc->dma_sdcr = 0x0001;

    // Initialize shared PRAM
    enet_pram->rbase = RxBD;
    enet_pram->tbase = TxBD;

    // Set Big Endian mode
    enet_pram->rfcr = QUICC_SCC_FCR_BE;
    enet_pram->tfcr = QUICC_SCC_FCR_BE;

    // Size of receive buffers
    enet_pram->mrblr = CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;

    // Initialize CRC calculations
    enet_pram->c_pres = 0xFFFFFFFF;
    enet_pram->c_mask = 0xDEBB20E3;  // Actual CRC formula
    enet_pram->crcec = 0;
    enet_pram->alec = 0;
    enet_pram->disfc = 0;

    // Frame padding
    enet_pram->pads = 0x8888;
    enet_pram->pads = 0x0000;

    // Retries
    enet_pram->ret_lim = 15;
    enet_pram->ret_cnt = 0;

    // Frame sizes
    enet_pram->mflr = IEEE_8023_MAX_FRAME;
    enet_pram->minflr = IEEE_8023_MIN_FRAME;
    enet_pram->maxd1 = CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;
    enet_pram->maxd2 = CYGNUM_DEVS_ETH_POWERPC_QUICC_BUFSIZE;

    // Group address hash
    enet_pram->gaddr1 = 0;
    enet_pram->gaddr2 = 0;
    enet_pram->gaddr3 = 0;
    enet_pram->gaddr4 = 0;

    // Device physical address
    ep = &enaddr[sizeof(enaddr)];
    ap = (unsigned char *)&enet_pram->paddr_h;
    for (i = 0;  i < sizeof(enaddr);  i++) {
        *ap++ = *--ep;
    }

    // Persistence counter
    enet_pram->p_per = 0; 

    // Individual address filter
    enet_pram->iaddr1 = 0;
    enet_pram->iaddr2 = 0;
    enet_pram->iaddr3 = 0;
    enet_pram->iaddr4 = 0;

    // Temp address
    enet_pram->taddr_h = 0;
    enet_pram->taddr_m = 0;
    enet_pram->taddr_l = 0;

    // Initialize the CPM (set up buffer pointers, etc).
    quicc_eth_command(sc, QUICC_CPM_CR_INIT_TXRX);

    // Clear any pending interrupt/exceptions
    scc->scc_scce = 0xFFFF;

    // Enable interrupts
    scc->scc_sccm = QUICC_SCCE_INTS | QUICC_SCCE_GRC | QUICC_SCCE_BSY;

    // Set up SCCx to run in ethernet mode
    scc->scc_gsmr_h = 0;
    scc->scc_gsmr_l = QUICC_SCC_GSML_TCI | QUICC_SCC_GSML_TPL_48 |
        QUICC_SCC_GSML_TPP_01 | QUICC_SCC_GSML_MODE_ENET;

    // Sync delimiters
    scc->scc_dsr = 0xD555;

    // Protocol specifics (as if GSML wasn't enough)
    scc->scc_psmr = QUICC_PMSR_ENET_CRC | QUICC_PMSR_SEARCH_AFTER_22 |
        QUICC_PMSR_RCV_SHORT_FRAMES;

#ifdef QUICC_ETH_ENABLE
    QUICC_ETH_ENABLE();
#endif

#ifdef QUICC_ETH_RESET_PHY
    QUICC_ETH_RESET_PHY();
#endif

    // Enable ethernet interface
#ifdef QUICC_ETH_PC_Tx_ENABLE
    eppc->pio_pcpar |= QUICC_ETH_PC_Tx_ENABLE;
    eppc->pio_pcdir &= ~QUICC_ETH_PC_Tx_ENABLE;
#else
    eppc->pip_pbpar |= QUICC_ETH_PB_Tx_ENABLE;
    eppc->pip_pbdir |= QUICC_ETH_PB_Tx_ENABLE;
#endif

    if (cache_state)
        HAL_DCACHE_ENABLE();

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, (unsigned char *)&enaddr);

    // Set LED state
    clear_led(LED_TxACTIVE);
    clear_led(LED_RxACTIVE);

    return true;
}

//
// This function is called to shut down the interface.
//
static void
quicc_eth_stop(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct scc_regs *scc = qi->ctl;

    // Disable the device!
    scc->scc_gsmr_l &= ~(QUICC_SCC_GSML_ENR | QUICC_SCC_GSML_ENT);
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
quicc_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct scc_regs *scc = qi->ctl;

    // Enable the device!
    scc->scc_gsmr_l |= QUICC_SCC_GSML_ENR | QUICC_SCC_GSML_ENT;
}

//
// This function is called for low level "control" operations
//
static int
quicc_eth_control(struct eth_drv_sc *sc, unsigned long key,
                  void *data, int length)
{
    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
        return 0;
        break;

#ifdef ETH_DRV_GET_IF_STATS
    case ETH_DRV_GET_IF_STATS:
    {
        struct ether_drv_stats *p = (struct ether_drv_stats *)data;
        struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;

        strcpy( p->description, "QUICC (MPC8xx) SCC Ethernet" );
        CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

        // Really need to determine the following values properly, for
        // now just assume the link is up, full duplex, unknown speed.
        
        p->operational = 3;            // LINK UP
        p->duplex = 1;
        p->speed = 0;

        {
            p->supports_dot3        = false;

            // Those commented out are not available on this chip.

            p->tx_good              = qi->tx_good             ;
            //p->tx_max_collisions    = qi->tx_max_collisions ;
            p->tx_late_collisions   = qi->tx_late_collisions  ;
            p->tx_underrun          = qi->tx_underrun         ;
            p->tx_carrier_loss      = qi->tx_carrier_loss     ;
            p->tx_deferred          = qi->tx_deferred         ;
            //p->tx_sqetesterrors   = qi->tx_sqetesterrors    ;
            //p->tx_single_collisions = qi->tx_single_collisions;
            //p->tx_mult_collisions   = qi->tx_mult_collisions  ;
            //p->tx_total_collisions  = qi->tx_total_collisions ;
            p->rx_good              = qi->rx_good             ;
            p->rx_crc_errors        = qi->rx_crc_errors       ;
            p->rx_align_errors      = qi->rx_align_errors     ;
            p->rx_resource_errors   = qi->rx_resource_errors  ;
            p->rx_overrun_errors    = qi->rx_overrun_errors   ;
            p->rx_collisions        = qi->rx_collisions       ;
            p->rx_short_frames      = qi->rx_short_frames     ;
            p->rx_too_long_frames   = qi->rx_long_frames      ;
            //p->rx_symbol_errors   = qi->rx_symbol_errors    ;
        
            p->interrupts           = qi->interrupts          ;
            p->rx_count             = qi->rx_count            ;
            p->rx_deliver           = qi->rx_deliver          ;
            p->rx_resource          = qi->rx_resource         ;
            p->rx_restart           = qi->rx_restart          ;
            p->tx_count             = qi->tx_count            ;
            p->tx_complete          = qi->tx_complete         ;
            p->tx_dropped           = qi->tx_dropped          ;
        }

        p->tx_queue_len = CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM;

        return 0; // OK
    }
#endif

        
    default:
        return 1;
        break;
    }
}

//
// This function is called to see if another packet can be sent.
// It should return the number of packets which can be handled.
// Zero should be returned if the interface is busy and can not send any more.
//
static int
quicc_eth_can_send(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;

    return (qi->txactive < CYGNUM_DEVS_ETH_POWERPC_QUICC_TxNUM);
}

//
// This routine is called to send data to the hardware.
static void 
quicc_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
               int total_len, unsigned long key)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct cp_bufdesc *txbd, *txfirst;
    volatile char *bp;
    int i, txindex, cache_state;
    unsigned int ctrl;

    qi->tx_count++;
    
    // Find a free buffer
    txbd = txfirst = qi->txbd;
    if ((txbd->ctrl & (QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int )))
#ifdef CYGPKG_NET
            panic ("No free xmit buffers");
#else
            diag_printf("QUICC Ethernet: No free xmit buffers\n");
#endif

    // Remember the next buffer to try
    if (txbd->ctrl & QUICC_BD_CTL_Wrap) {
        qi->txbd = qi->tbase;
    } else {
        qi->txbd = txbd+1;
    }
    txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
    qi->txkey[txindex] = key;
    // Set up buffer
    txbd->length = total_len;
    bp = txbd->buffer;
    for (i = 0;  i < sg_len;  i++) {
        memcpy((void *)bp, (void *)sg_list[i].buf, sg_list[i].len);
        bp += sg_list[i].len;
    }
    // Note: the MPC8xx does not seem to snoop/invalidate the data cache properly!
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_FLUSH(txbd->buffer, txbd->length);  // Make sure no stale data
    }
    // Send it on it's way
    ctrl = txbd->ctrl & ~QUICC_BD_TX_PAD;
    if (txbd->length < IEEE_8023_MIN_FRAME) {
        ctrl |= QUICC_BD_TX_PAD;
    }
    txbd->ctrl = ctrl | QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int | 
        QUICC_BD_TX_LAST | QUICC_BD_TX_TC;
    qi->txactive++;
    set_led(LED_TxACTIVE);
}

//
// This function is called when a packet has been received.  It's job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'quicc_eth_recv' will be called to actually fetch it from the hardware.
//
static void
quicc_eth_RxEvent(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct cp_bufdesc *rxbd;
	

    rxbd = qi->rnext;
    while ((rxbd->ctrl & (QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int)) == QUICC_BD_CTL_Int) {

	qi->rx_count++;
        
        if( rxbd->ctrl & QUICC_BD_RX_MISS )
        {
            qi->rx_miss++;
        }
        if( rxbd->ctrl & QUICC_BD_RX_LG )
        {
            qi->rx_long_frames++;
        }
        if( rxbd->ctrl & QUICC_BD_RX_NO )
        {
            qi->rx_align_errors++;
        }
        if( rxbd->ctrl & QUICC_BD_RX_SH )
        {
            qi->rx_short_frames++;
        }
        if( rxbd->ctrl & QUICC_BD_RX_CR )
        {
            qi->rx_crc_errors++;
        }
        if( rxbd->ctrl & QUICC_BD_RX_OV )
        {
            qi->rx_overrun_errors++;
        }

        if( rxbd->ctrl & QUICC_BD_RX_CL )
        {
            qi->rx_collisions++;
        }

        if( (rxbd->ctrl & QUICC_BD_RX_ERRORS) == 0 )
        {
            qi->rx_good++;
			
            // OK frame - Prepare for callback
            qi->rxbd = rxbd;  // Save for callback
            set_led(LED_RxACTIVE);
            // Remove the CRC from the end
            (sc->funs->eth_drv->recv)(sc, rxbd->length - 4);
			
            clear_led(LED_RxACTIVE);
        }
        
      
        // Clear flags and wrap if needed else step up BD pointer 
        if (rxbd->ctrl & QUICC_BD_CTL_Wrap) 
        {
            rxbd->ctrl = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int | QUICC_BD_CTL_Wrap;
            rxbd = qi->rbase;
        } 
        else 
        {
            rxbd->ctrl = QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int;
            rxbd++;
        }
        
    }
    // Remember where we left off
    qi->rnext = (struct cp_bufdesc *)rxbd;
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// It's job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
quicc_eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    unsigned char *bp;
    int i, cache_state;
    int sg_list_null_buffer = 0;
    
    bp = (unsigned char *)qi->rxbd->buffer;
    // Note: the MPC8xx does not seem to snoop/invalidate the data cache properly!
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_INVALIDATE(qi->rxbd->buffer, qi->rxbd->length);  // Make sure no stale data
    }
    for (i = 0;  i < sg_len;  i++) {
        if (sg_list[i].buf != 0) {
            memcpy((void *)sg_list[i].buf, bp, sg_list[i].len);
            bp += sg_list[i].len;
        }
        else
            sg_list_null_buffer = 1;
    }

    // A NULL sg_list buffer usually means no mbufs, so we don't count
    // it as a delivery, instead we count it as a resource error.
    
    if (!sg_list_null_buffer)
        qi->rx_deliver++;
    else
        qi->rx_resource++;

}

static void
quicc_eth_command( struct eth_drv_sc *sc, unsigned long cmd)
{
   volatile EPPC *eppc = (volatile EPPC *)eppc_base();
   
   eppc->cp_cr = QUICC_CPM_SCCx | cmd | QUICC_CPM_CR_BUSY;
   while (eppc->cp_cr & QUICC_CPM_CR_BUSY )
       continue;
}

static void
quicc_eth_TxEvent(struct eth_drv_sc *sc, int stat)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct cp_bufdesc *txbd;
    int txindex;
    bool restart = false;

    txbd = qi->tnext;
    
    while ((txbd->ctrl & (QUICC_BD_CTL_Ready | QUICC_BD_CTL_Int)) == QUICC_BD_CTL_Int) {

        txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);

        qi->tx_complete++;
        
        (sc->funs->eth_drv->tx_done)(sc, qi->txkey[txindex], 0);
        txbd->ctrl &= ~QUICC_BD_CTL_Int;  // Reset int pending bit

        if (txbd->ctrl & QUICC_BD_TX_LC )
            qi->tx_late_collisions++, restart = true;
        if (txbd->ctrl & QUICC_BD_TX_RL )
            qi->tx_retransmit_error++, restart = true;
        if (txbd->ctrl & QUICC_BD_TX_UN )
            qi->tx_underrun++, restart = true;
        if (txbd->ctrl & QUICC_BD_TX_CSL )
            qi->tx_carrier_loss++;
        if (txbd->ctrl & QUICC_BD_TX_HB )
            qi->tx_heartbeat_loss++;        
        if (txbd->ctrl & QUICC_BD_TX_DEF )
            qi->tx_deferred++;        

        if( (txbd->ctrl & QUICC_BD_TX_ERRORS) == 0 )
            qi->tx_good++;

        
        if (txbd->ctrl & QUICC_BD_CTL_Wrap) {
            txbd->ctrl = QUICC_BD_CTL_Wrap;
            txbd = qi->tbase;
        } else {
            txbd->ctrl = 0;            
            txbd++;
        }
	qi->txactive--;
    }

    if (qi->txactive == 0) {
        clear_led(LED_TxACTIVE);
    }
    
    // Remember where we left off
    qi->tnext = (struct cp_bufdesc *)txbd;

    if (restart)
    {
        quicc_eth_command(sc,QUICC_CPM_CR_RESTART_TX);
        qi->tx_restart++;        
    }
    
}

//
// Interrupt processing
//
static void          
quicc_eth_int(struct eth_drv_sc *sc)
{
    struct quicc_eth_info *qi = (struct quicc_eth_info *)sc->driver_private;
    volatile struct scc_regs *scc = qi->ctl;
    unsigned short scce;

    qi->interrupts++;

    while ( (scce = scc->scc_scce) != 0 )
    {
        scc->scc_scce = scce;
        
        if ( (scce & (QUICC_SCCE_TXE | QUICC_SCCE_TX)) != 0)
        {
            quicc_eth_TxEvent(sc, scce);
        }
        if ( (scce & ( QUICC_SCCE_RXF | QUICC_SCCE_RX )) != 0)
        {
            quicc_eth_RxEvent(sc);
        }
        if ( (scce & QUICC_SCCE_BSY) != 0)
        {
            qi->rx_resource_errors++;
        }
        if ( (scce & QUICC_SCCE_GRC) != 0 )
        {
            quicc_eth_command(sc, QUICC_CPM_CR_RESTART_TX);
            qi->tx_restart++;
            quicc_eth_command(sc, QUICC_CPM_CR_HUNT_MODE);
            qi->rx_restart++;
        }
    }
}

//
// Interrupt vector
//
static int          
quicc_eth_int_vector(struct eth_drv_sc *sc)
{
    return (QUICC_ETH_INT);
}
