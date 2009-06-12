//==========================================================================
//
//      dev/if_fec.c
//
//      Fast ethernet device driver for PowerPC MPC8260 boards
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    mtek
// Contributors: pfine
// Date:         2002-02-20
// Purpose:      
// Description:  hardware driver for MPC8260 FEC
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_eth_powerpc_quicc2.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/var_intr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/mpc8260.h>

#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#endif

#include "fec.h"
#include "EnetPHY.h"

#define ALIGN_TO_CACHE_LINES(x)  ( (long)((x) + 31) & 0xffffffe0 )

static unsigned char fec_eth_rxbufs[CYGNUM_DEVS_ETH_POWERPC_QUICC2_RxNUM *
                                    (CYGNUM_DEVS_ETH_POWERPC_QUICC2_BUFSIZE + 32)];
static unsigned char fec_eth_txbufs[CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM *
                                    (CYGNUM_DEVS_ETH_POWERPC_QUICC2_BUFSIZE + 32)];

// Buffer descriptors are in dual ported RAM, which is marked non-cached
#define FEC_BDs_NONCACHED
static struct fec_bd *const fec_eth_rxring = (struct fec_bd *) 
  (QUICC2_VADS_IMM_BASE + FEC_PRAM_RxBD_Base);
static struct fec_bd *const fec_eth_txring = (struct fec_bd *) 
  (QUICC2_VADS_IMM_BASE + FEC_PRAM_TxBD_Base);

static struct fec_eth_info fec_eth0_info;

static unsigned short _default_enaddr[] = {0x1234, 0x5678, 0x90a1};
static unsigned char enaddr[6];

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
RedBoot_config_option("Network hardware address [MAC]",
                      fec_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0
    );
RedBoot_config_option("Attempt to find 100 Mbps Ethernet",
                      fec_100,
                      ALWAYS_ENABLED, true,
                      CONFIG_BOOL, 0
    );
#endif
#endif

#define os_printf diag_printf

// CONFIG_ESA and CONFIG_BOOL are defined in redboot/include/flash_config.h
#ifndef CONFIG_ESA
#define CONFIG_ESA 6      // ethernet address length ...
#endif

#ifndef CONFIG_BOOL
#define CONFIG_BOOL 1
#endif

ETH_DRV_SC(fec_eth0_sc,
           &fec_eth0_info,     // Driver specific data
           "eth0",             // Name for this interface
           fec_eth_start,
           fec_eth_stop,
           fec_eth_control,
           fec_eth_can_send,
           fec_eth_send,
           fec_eth_recv,
           fec_eth_deliver,
           fec_eth_int,
           fec_eth_int_vector);

NETDEVTAB_ENTRY(fec_netdev, 
                "fec_eth", 
                fec_eth_init, 
                &fec_eth0_sc);

#ifdef CYGPKG_NET
static cyg_interrupt fec_eth_interrupt;
static cyg_handle_t  fec_eth_interrupt_handle;
#endif
static void          fec_eth_int(struct eth_drv_sc *data);

#define FEC_ETH_INT CYGNUM_HAL_INTERRUPT_FCC2

// This ISR is called when the ethernet interrupt occurs
#ifdef CYGPKG_NET
static int
fec_eth_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
  cyg_drv_interrupt_mask(FEC_ETH_INT);
  return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}
#endif

// Deliver function (ex-DSR) handles the ethernet [logical] processing
static void
fec_eth_deliver(struct eth_drv_sc * sc)
{
  fec_eth_int(sc);
#ifdef CYGPKG_NET
  //  Clearing the event register acknowledges FCC2 interrupt ...
  //  cyg_drv_interrupt_acknowledge(FEC_ETH_INT);
  cyg_drv_interrupt_unmask(FEC_ETH_INT);
#endif

}


// Initialize the interface - performed at system startup
// This function must set up the interface, including arranging to
// handle interrupts, etc, so that it may be "started" cheaply later.
static bool 
fec_eth_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;

    volatile t_PQ2IMM    *IMM = (volatile t_PQ2IMM *) QUICC2_VADS_IMM_BASE;
    volatile t_Fcc_Pram  *fcc =  (volatile t_Fcc_Pram *) (QUICC2_VADS_IMM_BASE + FEC_PRAM_OFFSET);
    volatile t_EnetFcc_Pram *E_fcc = &(fcc->SpecificProtocol.e);
#if defined(CYGPKG_HAL_POWERPC_VADS) || defined(CYGPKG_HAL_POWERPC_TS6)
    volatile t_BCSR *CSR   = (t_BCSR *) 0x04500000;
#endif

    int i;
    bool esa_ok;
    bool fec_100;
    unsigned char *c_ptr;
    UINT16 link_speed;

    // Link the memory to the driver control memory
    qi->fcc_reg = & (IMM->fcc_regs[FCC2]);

    // just in case :  disable Transmit and Receive 
    qi->fcc_reg->fcc_gfmr &= ~(FEC_GFMR_EN_Rx | FEC_GFMR_EN_Tx);
    
    // Via BCSR, (re)start LXT970
#if defined(CYGPKG_HAL_POWERPC_VADS) || defined(CYGPKG_HAL_POWERPC_TS6)
    EnableResetPHY(CSR);
#endif

    // Try to read the ethernet address of the transciever ...
#ifdef CYGPKG_REDBOOT
    esa_ok = flash_get_config("fec_100", &fec_100, CONFIG_BOOL);
#else
    esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET, 
                                         "fec_100", &fec_100, CONFIG_BOOL);
#endif
    
    link_speed = NOTLINKED;
    if(esa_ok && fec_100){
        // Via MII Management pins, tell LXT970 to initialize 
        os_printf("Attempting to acquire 100 Mbps half_duplex link ...");
        InitEthernetPHY((VUINT32 *) &(IMM->io_regs[PORT_C].pdir),
                        (VUINT32 *) &(IMM->io_regs[PORT_C].pdat),
                        HUNDRED_HD);

        link_speed = LinkTestPHY();
        os_printf("\n");
        if(link_speed == NOTLINKED){
            os_printf("Failed to get 100 Mbps half_duplex link.\n");
        }
    }
    if(link_speed == NOTLINKED){
        os_printf("Attempting to acquire 10 Mbps half_duplex link ...");
        InitEthernetPHY((VUINT32 *) &(IMM->io_regs[PORT_C].pdir),
                        (VUINT32 *) &(IMM->io_regs[PORT_C].pdat),
                        TEN_HD);
        link_speed = LinkTestPHY();
        os_printf("\n");
        if(link_speed == NOTLINKED){
            link_speed = LinkTestPHY();
            os_printf("Failed to get 10 Mbps half_duplex link.\n");
        }
            
    }
    switch ( link_speed ) {
      
    case HUNDRED_FD: 
      os_printf("100 MB full-duplex ethernet link \n"); 
      break;
    case HUNDRED_HD: 
      os_printf("100 MB half-duplex ethernet link \n"); 
      break;
    case TEN_FD: 
      os_printf("10 MB full-duplex ethernet link \n"); 
      break;
    case TEN_HD: 
      os_printf("10 MB half-duplex ethernet link \n"); 
      break;
    default:     
      os_printf("NO ethernet link \n");
    }

    // Connect PORTC pins: (C19) to clk13, (C18) to clk 14
    IMM->io_regs[PORT_C].ppar |= 0x00003000;
    IMM->io_regs[PORT_C].podr &= ~(0x00003000);
    IMM->io_regs[PORT_C].psor &= ~(0x00003000);
    IMM->io_regs[PORT_C].pdir &= ~(0x00003000);

    // Connect clk13 to RxClk and clk14 to TxClk on FCC2
    IMM->cpm_mux_cmxfcr &= 0x7f007f00; // clear fcc2 clocks
    IMM->cpm_mux_cmxfcr |= 0x00250000; // set fcc2 clocks  (see 15-14)
    IMM->cpm_mux_cmxuar  = 0x0000;     // Utopia address reg, just clear

    // Initialize parallel port registers to connect FCC2 to MII
    IMM->io_regs[PORT_B].podr &= 0xffffc000; // clear bits 18-31 
    IMM->io_regs[PORT_B].psor &= 0xffffc000;
    IMM->io_regs[PORT_B].pdir &= 0xffffc000;

    IMM->io_regs[PORT_B].psor |= 0x00000004;
    IMM->io_regs[PORT_B].pdir |= 0x000003c5;
    IMM->io_regs[PORT_B].ppar |= 0x00003fff; 

    // Initialize Receive Buffer Descriptors
    qi->rbase = fec_eth_rxring;
    qi->rxbd  = fec_eth_rxring;
    qi->rnext = fec_eth_rxring;
    c_ptr = fec_eth_rxbufs;

    for(i=0; i<CYGNUM_DEVS_ETH_POWERPC_QUICC2_RxNUM; i++) {
      
      fec_eth_rxring[i].ctrl   = (FEC_BD_Rx_Empty | FEC_BD_Rx_Int);
      fec_eth_rxring[i].length = 0;                   // reset 
      c_ptr = (unsigned char *) ALIGN_TO_CACHE_LINES(c_ptr);
      fec_eth_rxring[i].buffer = (volatile unsigned char *)c_ptr;
      c_ptr += CYGNUM_DEVS_ETH_POWERPC_QUICC2_BUFSIZE;
    }
    
    fec_eth_rxring[CYGNUM_DEVS_ETH_POWERPC_QUICC2_RxNUM-1].ctrl |= FEC_BD_Rx_Wrap;

    // Initialize Transmit Buffer Descriptors
    qi->tbase = fec_eth_txring;
    qi->txbd  = fec_eth_txring;
    qi->tnext = fec_eth_txring;
    c_ptr = fec_eth_txbufs;

    for(i=0; i<CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM; i++) {
      
      fec_eth_txring[i].ctrl   = (FEC_BD_Tx_Pad | FEC_BD_Tx_Int);  
      fec_eth_txring[i].length = 0;   // reset : Write before send
      c_ptr = (unsigned char *) ALIGN_TO_CACHE_LINES(c_ptr);
      fec_eth_txring[i].buffer = (volatile unsigned char  *)c_ptr;
      c_ptr += CYGNUM_DEVS_ETH_POWERPC_QUICC2_BUFSIZE;
    }

    fec_eth_txring[CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM-1].ctrl |= FEC_BD_Tx_Wrap;
    
    // Common FCC Parameter RAM initialization
    fcc->riptr = FEC_PRAM_RIPTR;   // in dual port RAM (see 28-11)
    fcc->tiptr = FEC_PRAM_TIPTR;   // in dual port RAM (see 28-11)
    fcc->mrblr = FEC_PRAM_MRBLR;   // ?? FROM 8101 code ...
    fcc->rstate &= FEC_FCR_INIT;
    fcc->rstate |= FEC_FCR_MOT_BO;
    fcc->rbase = (long) fec_eth_rxring;
    fcc->tstate &= FEC_FCR_INIT;
    fcc->tstate |= FEC_FCR_MOT_BO;
    fcc->tbase = (long) fec_eth_txring;

    // Ethernet Specific FCC Parameter RAM Initialization     
    E_fcc->c_mask   = FEC_PRAM_C_MASK; // (see 30-9)
    E_fcc->c_pres   = FEC_PRAM_C_PRES;
    E_fcc->crcec    = 0;
    E_fcc->alec     = 0;
    E_fcc->disfc    = 0;
    E_fcc->ret_lim  = FEC_PRAM_RETLIM;
    E_fcc->p_per    = FEC_PRAM_PER_LO;
    E_fcc->gaddr_h  = 0;
    E_fcc->gaddr_l  = 0;
    E_fcc->tfcstat  = 0;
    E_fcc->mflr     = FEC_MAX_FLR;

    // Try to read the ethernet address of the transciever ...
#ifdef CYGPKG_REDBOOT
    esa_ok = flash_get_config("fec_esa", enaddr, CONFIG_ESA);
#else
    esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET, 
                                         "fec_esa", enaddr, CONFIG_ESA);
#endif
    if (!esa_ok) {
      // If can't use the default ...
      os_printf("FEC_ETH - Warning! ESA unknown\n");
      memcpy(enaddr, _default_enaddr, sizeof(enaddr));
    }

    E_fcc->paddr1_h = ((short)enaddr[5] << 8) | enaddr[4]; // enaddr[2]; 
    E_fcc->paddr1_m = ((short)enaddr[3] << 8) | enaddr[2]; // enaddr[1];
    E_fcc->paddr1_l = ((short)enaddr[1] << 8) | enaddr[0]; // enaddr[0];

    E_fcc->iaddr_h  = 0;
    E_fcc->iaddr_l  = 0;
    E_fcc->minflr   = FEC_MIN_FLR;
    E_fcc->taddr_h  = 0;
    E_fcc->taddr_m  = 0;
    E_fcc->taddr_l  = 0;
    E_fcc->pad_ptr  = FEC_PRAM_TIPTR; // No special padding char ...
    E_fcc->cf_type  = 0;
    E_fcc->maxd1    = FEC_PRAM_MAXD;
    E_fcc->maxd2    = FEC_PRAM_MAXD;

    // FCC register initialization 
    IMM->fcc_regs[FCC2].fcc_gfmr = FEC_GFMR_INIT; 
    IMM->fcc_regs[FCC2].fcc_psmr = FEC_PSMR_INIT;
    IMM->fcc_regs[FCC2].fcc_dsr  = FEC_DSR_INIT;

#ifdef CYGPKG_NET
    // clear the events of FCC2
    IMM->fcc_regs[FCC2].fcc_fcce = 0xFFFF0000;   
    IMM->fcc_regs[FCC2].fcc_fccm = FEC_EV_TXE | FEC_EV_TXB | FEC_EV_RXF;

    // Set up to handle interrupts
    cyg_drv_interrupt_create(FEC_ETH_INT,
                             0,  // Highest //CYGARC_SIU_PRIORITY_HIGH,
                             (cyg_addrword_t)sc, //  Data passed to ISR
                             (cyg_ISR_t *)fec_eth_isr,
                             (cyg_DSR_t *)eth_drv_dsr,
                             &fec_eth_interrupt_handle,
                             &fec_eth_interrupt);
    cyg_drv_interrupt_attach(fec_eth_interrupt_handle);
    cyg_drv_interrupt_acknowledge(FEC_ETH_INT);
    cyg_drv_interrupt_unmask(FEC_ETH_INT);
#else

    // Mask the interrupts 
    IMM->fcc_regs[FCC2].fcc_fccm = 0;
#endif

    // Issue Init RX & TX Parameters Command for FCC2
    while ((IMM->cpm_cpcr & CPCR_FLG) != CPCR_READY_TO_RX_CMD); 
    
    IMM->cpm_cpcr = CPCR_INIT_TX_RX_PARAMS |
      CPCR_FCC2_CH |
      CPCR_MCN_FEC | 
      CPCR_FLG;              /* ISSUE COMMAND */
    
    while ((IMM->cpm_cpcr & CPCR_FLG) != CPCR_READY_TO_RX_CMD); 

    // Initialize upper level driver for ecos
    (sc->funs->eth_drv->init)(sc, (unsigned char *)&enaddr);

    return true;
}
 
//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
fec_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    
  // Enable the device : 
  // Set the ENT/ENR bits in the GFMR -- Enable Transmit/Receive
  qi->fcc_reg->fcc_gfmr |= (FEC_GFMR_EN_Rx | FEC_GFMR_EN_Tx);
  
}

//
// This function is called to shut down the interface.
//
static void
fec_eth_stop(struct eth_drv_sc *sc)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
  
  // Disable the device : 
  // Clear the ENT/ENR bits in the GFMR -- Disable Transmit/Receive
  qi->fcc_reg->fcc_gfmr &= ~(FEC_GFMR_EN_Rx | FEC_GFMR_EN_Tx);
}


//
// This function is called for low level "control" operations
//
static int
fec_eth_control(struct eth_drv_sc *sc, unsigned long key,
                void *data, int length)
{
  switch (key) {
  case ETH_DRV_SET_MAC_ADDRESS:
    return 0;
    break;
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
fec_eth_can_send(struct eth_drv_sc *sc)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
  volatile struct fec_bd *txbd = qi->txbd;
  int cache_state;

  HAL_DCACHE_IS_ENABLED(cache_state);
#ifndef FEC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fec_eth_txring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM);
  }
#endif

  return ((txbd->ctrl & (FCC_BD_Tx_TC | FCC_BD_Tx_Ready)) == 0);
}

//
// This routine is called to send data to the hardware.
static void 
fec_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
             int total_len, unsigned long key)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
  struct fec_bd *txbd, *txfirst;
  volatile char *bp;
  int i, txindex, cache_state;

  HAL_DCACHE_IS_ENABLED(cache_state);
#ifndef FEC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fec_eth_txring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM);
  }
#endif
 
  // Find a free buffer
  txbd = txfirst = qi->txbd;
  while (txbd->ctrl & FEC_BD_Tx_Ready) {
    // This buffer is busy, move to next one
    if (txbd->ctrl & FEC_BD_Tx_Wrap) {
      txbd = qi->tbase;
    } else {
      txbd++;
    }
    if (txbd == txfirst) {
#ifdef CYGPKG_NET
      panic ("No free xmit buffers");
#else
      os_printf("FEC Ethernet: No free xmit buffers\n");
#endif
    }
  }

  // Remember the next buffer to try
  if (txbd->ctrl & FEC_BD_Tx_Wrap) {
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

  // Make sure no stale data buffer ...
  if (cache_state) {
    HAL_DCACHE_FLUSH(txbd->buffer, txbd->length);
  }
  // Send it on it's way
  txbd->ctrl |= FEC_BD_Tx_Ready | FEC_BD_Tx_Last | FEC_BD_Tx_TC;
#ifndef FEC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_FLUSH(fec_eth_txring, 
                     8*CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM);  
  }
#endif  

}

//
// This function is called when a packet has been received.  It's job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'fec_eth_recv' will be called to actually fetch it from the hardware.
//
static void
fec_eth_RxEvent(struct eth_drv_sc *sc)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
  struct fec_bd *rxbd;
  int cache_state;

  HAL_DCACHE_IS_ENABLED(cache_state);
#ifndef FEC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fec_eth_rxring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_QUICC2_RxNUM);
  }
#endif

  rxbd = qi->rnext;
  while ((rxbd->ctrl & FEC_BD_Rx_Empty) == 0) {
    qi->rxbd = rxbd;  // Save for callback

    // This is the right way of doing it, but dcbi has a bug ...
    //    if (cache_state) {
    //      HAL_DCACHE_INVALIDATE(rxbd->buffer, rxbd->length); 
    //    }
    (sc->funs->eth_drv->recv)(sc, rxbd->length);
    if (cache_state) {
      HAL_DCACHE_FLUSH(rxbd->buffer, rxbd->length); 
    }

    rxbd->ctrl |= FEC_BD_Rx_Empty;
    if (rxbd->ctrl & FEC_BD_Rx_Wrap) {
      rxbd = qi->rbase;
    } else {
      rxbd++;
    }
  }
  // Remember where we left off
  qi->rnext = (struct fec_bd *)rxbd;

  // Make sure no stale data
#ifndef FEC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_FLUSH(fec_eth_rxring, 
                     8*CYGNUM_DEVS_ETH_POWERPC_QUICC2_RxNUM);
  }
#endif

}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// It's job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
fec_eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
  unsigned char *bp;
  int i;
  
  bp = (unsigned char *)qi->rxbd->buffer;

  for (i = 0;  i < sg_len;  i++) {
    if (sg_list[i].buf != 0) {
      memcpy((void *)sg_list[i].buf, bp, sg_list[i].len);
      bp += sg_list[i].len;
    }
  }

}

static void
fec_eth_TxEvent(struct eth_drv_sc *sc, int stat)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
  struct fec_bd *txbd;
  int txindex, cache_state;

  // Make sure no stale data
  HAL_DCACHE_IS_ENABLED(cache_state);
#ifndef FEC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fec_eth_txring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM);
  }
#endif

  txbd = qi->tnext;
  // Note: TC field is used to indicate the buffer has/had data in it
  while ( (txbd->ctrl & (FEC_BD_Tx_TC | FEC_BD_Tx_Ready)) == FEC_BD_Tx_TC ) {
    txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
    (sc->funs->eth_drv->tx_done)(sc, qi->txkey[txindex], 0);
    txbd->ctrl &= ~FEC_BD_Tx_TC;
    if (txbd->ctrl & FEC_BD_Tx_Wrap) {
      txbd = qi->tbase;
    } else {
      txbd++;
    }
  }
  // Remember where we left off
  qi->tnext = (struct fec_bd *)txbd;

  // Make sure no stale data  
#ifndef FEC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_FLUSH(fec_eth_txring, 
                     8*CYGNUM_DEVS_ETH_POWERPC_QUICC2_TxNUM);
  }
#endif

}

//
// Interrupt processing
//
static void          
fec_eth_int(struct eth_drv_sc *sc)
{
  struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
  unsigned long iEvent;

  while ((iEvent = qi->fcc_reg->fcc_fcce) != 0){

    // Writing 1's clear fcce, Writing 0's have no effect
    qi->fcc_reg->fcc_fcce = iEvent; 

    // Tx Done or Tx Error
    if ( iEvent & (FEC_EV_TXB | FEC_EV_TXE) ) {     
      fec_eth_TxEvent(sc, iEvent);
    }
  
    // Complete or non-complete frame receive
    if (iEvent & (FEC_EV_RXF | FEC_EV_RXB) ) {    
      fec_eth_RxEvent(sc);
    }

  }
    

}

//
// Interrupt vector
//
static int          
fec_eth_int_vector(struct eth_drv_sc *sc)
{
    return (FEC_ETH_INT);
}

