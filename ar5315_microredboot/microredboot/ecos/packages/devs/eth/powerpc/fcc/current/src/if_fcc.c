//==========================================================================
//
//      dev/if_fcc.c
//
//      Fast ethernet device driver for PowerPC MPC8xxx (QUICC-II) boards
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003, 2004 Gary Thomas
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
// Contributors: mtek, pfine
// Date:         2003-08-19
// Purpose:      
// Description:  hardware driver for MPC8xxx FCC
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/devs_eth_powerpc_fcc.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/mpc8xxx.h>

#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#endif

#ifdef CYGPKG_DEVS_ETH_PHY
#include <cyg/io/eth_phy.h>
#endif

#include "fcc.h"

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
#endif
#endif

#ifdef CYGDAT_DEVS_FCC_ETH_INL
#include CYGDAT_DEVS_FCC_ETH_CDL   // platform configury
#include CYGDAT_DEVS_FCC_ETH_INL   // platform details
#else
#error "No board instance defined!"
#endif

#define ALIGN_TO_CACHE_LINES(x)  ( (long)((x) + 31) & 0xffffffe0 )

// Buffer descriptors are in dual ported RAM, which is marked non-cached
#define FCC_BDs_NONCACHED

#define os_printf diag_printf

// CONFIG_ESA and CONFIG_BOOL are defined in redboot/include/flash_config.h
#ifndef CONFIG_ESA
#define CONFIG_ESA 6      // ethernet address length ...
#endif

#ifndef CONFIG_BOOL
#define CONFIG_BOOL 1
#endif

static void          fcc_eth_int(struct eth_drv_sc *data);

// This ISR is called when the ethernet interrupt occurs
#ifdef CYGPKG_NET
static int
fcc_eth_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
    struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;

    cyg_drv_interrupt_mask(qi->int_vector);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}
#endif

// Deliver function (ex-DSR) handles the ethernet [logical] processing
static void
fcc_eth_deliver(struct eth_drv_sc * sc)
{
#ifdef CYGPKG_NET
    struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
#endif

    fcc_eth_int(sc);
#ifdef CYGPKG_NET
    //  Clearing the event register acknowledges FCC interrupt ...
    cyg_drv_interrupt_unmask(qi->int_vector);
#endif

}


// Initialize the interface - performed at system startup
// This function must set up the interface, including arranging to
// handle interrupts, etc, so that it may be "started" cheaply later.
static bool 
fcc_eth_init(struct cyg_netdevtab_entry *dtp)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)dtp->device_instance;
    struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
    volatile t_Fcc_Pram  *fcc =  (volatile t_Fcc_Pram *)0;
    volatile t_EnetFcc_Pram *E_fcc;
    int i, fcc_chan;
    bool esa_ok;
    unsigned char *c_ptr;
    unsigned char _enaddr[6];
    unsigned long rxbase, txbase;
    struct fcc_bd *rxbd, *txbd;
    // The FCC seems rather picky about these...
    static long rxbd_base = 0x3000;
    static long txbd_base = 0xB000;
#ifdef CYGPKG_DEVS_ETH_PHY
    unsigned short phy_state = 0;
#endif

    // Set up pointers to FCC controller
    switch (qi->int_vector) {
    case CYGNUM_HAL_INTERRUPT_FCC1:
        qi->fcc_reg = &(IMM->fcc_regs[FCC1]);
        fcc =  (volatile t_Fcc_Pram *)((unsigned long)IMM + FCC1_PRAM_OFFSET);
        fcc_chan = FCC1_PAGE_SUBBLOCK;
        break;
    case CYGNUM_HAL_INTERRUPT_FCC2:
        qi->fcc_reg = &(IMM->fcc_regs[FCC2]);
        fcc =  (volatile t_Fcc_Pram *)((unsigned long)IMM + FCC2_PRAM_OFFSET);
        fcc_chan = FCC2_PAGE_SUBBLOCK;
        break;
	case CYGNUM_HAL_INTERRUPT_FCC3:
		qi->fcc_reg = &(IMM->fcc_regs[FCC3]);
        fcc =  (volatile t_Fcc_Pram *)((unsigned long)IMM + FCC3_PRAM_OFFSET);
        fcc_chan = FCC3_PAGE_SUBBLOCK;
        break;
    default:
        os_printf("Can't initialize '%s' - unknown FCC!\n", dtp->name);
        return false;
    }

    // just in case :  disable Transmit and Receive 
    qi->fcc_reg->fcc_gfmr &= ~(FCC_GFMR_EN_Rx | FCC_GFMR_EN_Tx);
    
    // Try to read the ethernet address of the transciever ...
#ifdef CYGPKG_REDBOOT
    esa_ok = flash_get_config(qi->esa_key, _enaddr, CONFIG_ESA);
#else
    esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET, 
                                         qi->esa_key, _enaddr, CONFIG_ESA);
#endif
    if (esa_ok) {
        memcpy(qi->enaddr, _enaddr, sizeof(qi->enaddr));
    } else {
        // No 'flash config' data available - use default
        os_printf("FCC_ETH - Warning! Using default ESA for '%s'\n", dtp->name);
    }

    // Initialize Receive Buffer Descriptors
    rxbase = rxbd_base;
    fcc->riptr = rxbase;           // temp work buffer
    fcc->mrblr = FCC_PRAM_MRBLR;   // Max Rx buffer 
    fcc->rstate &= FCC_FCR_INIT;
    fcc->rstate |= FCC_FCR_MOT_BO;
    rxbase += 64;
    rxbd_base += sizeof(struct fcc_bd)*qi->rxnum + 64;
    rxbd = (struct fcc_bd *)(CYGARC_IMM_BASE + rxbase);
    fcc->rbase = (CYG_WORD)rxbd;
    c_ptr = qi->rxbuf;
    qi->rbase = rxbd;
    qi->rxbd  = rxbd;
    qi->rnext = rxbd;

    for (i = 0; i < qi->rxnum; i++, rxbd++) {
        rxbd->ctrl   = (FCC_BD_Rx_Empty | FCC_BD_Rx_Int);
        rxbd->length = 0;                   // reset 
        c_ptr = (unsigned char *) ALIGN_TO_CACHE_LINES(c_ptr);
        rxbd->buffer = (volatile unsigned char *)c_ptr;
        c_ptr += CYGNUM_DEVS_ETH_POWERPC_FCC_BUFSIZE;
    }
    rxbd--;
    rxbd->ctrl |= FCC_BD_Rx_Wrap;

    // Initialize Transmit Buffer Descriptors
    txbase = txbd_base;
    fcc->tiptr = txbase;   // in dual port RAM (see 28-11)
    fcc->tstate &= FCC_FCR_INIT;
    fcc->tstate |= FCC_FCR_MOT_BO;
    txbase += 64;
    txbd_base += sizeof(struct fcc_bd)*qi->txnum + 64;
    txbd = (struct fcc_bd *)(CYGARC_IMM_BASE + txbase);
    fcc->tbase = (CYG_WORD)txbd;
    c_ptr = qi->txbuf;
    qi->tbase = txbd;
    qi->txbd  = txbd;
    qi->tnext = txbd;

    for (i = 0; i < qi->txnum; i++, txbd++) {
        txbd->ctrl   = (FCC_BD_Tx_Pad | FCC_BD_Tx_Int);  
        txbd->length = 0;   // reset : Write before send
        c_ptr = (unsigned char *) ALIGN_TO_CACHE_LINES(c_ptr);
        txbd->buffer = (volatile unsigned char  *)c_ptr;
        c_ptr += CYGNUM_DEVS_ETH_POWERPC_FCC_BUFSIZE;
    }
    txbd--;
    txbd->ctrl |= FCC_BD_Tx_Wrap;
    
    // Ethernet Specific FCC Parameter RAM Initialization     
    E_fcc = &(fcc->SpecificProtocol.e);
    E_fcc->c_mask   = FCC_PRAM_C_MASK; // (see 30-9)
    E_fcc->c_pres   = FCC_PRAM_C_PRES;
    E_fcc->crcec    = 0;
    E_fcc->alec     = 0;
    E_fcc->disfc    = 0;
    E_fcc->ret_lim  = FCC_PRAM_RETLIM;
    E_fcc->p_per    = FCC_PRAM_PER_LO;
    E_fcc->gaddr_h  = 0;
    E_fcc->gaddr_l  = 0;
    E_fcc->tfcstat  = 0;
    E_fcc->mflr     = FCC_MAX_FLR;

    E_fcc->paddr1_h = ((short)qi->enaddr[5] << 8) | qi->enaddr[4];
    E_fcc->paddr1_m = ((short)qi->enaddr[3] << 8) | qi->enaddr[2];
    E_fcc->paddr1_l = ((short)qi->enaddr[1] << 8) | qi->enaddr[0];

    E_fcc->iaddr_h  = 0;
    E_fcc->iaddr_l  = 0;
    E_fcc->minflr   = FCC_MIN_FLR;
    E_fcc->taddr_h  = 0;
    E_fcc->taddr_m  = 0;
    E_fcc->taddr_l  = 0;
    E_fcc->pad_ptr  = fcc->tiptr; // No special padding char ...
    E_fcc->cf_type  = 0;
    E_fcc->maxd1    = FCC_PRAM_MAXD;
    E_fcc->maxd2    = FCC_PRAM_MAXD;

    // FCC register initialization 
    qi->fcc_reg->fcc_gfmr = FCC_GFMR_INIT; 
    qi->fcc_reg->fcc_psmr = FCC_PSMR_INIT;
    qi->fcc_reg->fcc_dsr  = FCC_DSR_INIT;

#ifdef CYGPKG_NET
    // clear the events of FCCX
    qi->fcc_reg->fcc_fcce = 0xFFFF;   
    qi->fcc_reg->fcc_fccm = FCC_EV_TXE | FCC_EV_TXB | FCC_EV_RXF;

    // Set up to handle interrupts
    cyg_drv_interrupt_create(qi->int_vector,
                             0,  // Highest //CYGARC_SIU_PRIORITY_HIGH,
                             (cyg_addrword_t)sc, //  Data passed to ISR
                             (cyg_ISR_t *)fcc_eth_isr,
                             (cyg_DSR_t *)eth_drv_dsr,
                             &qi->fcc_eth_interrupt_handle,
                             &qi->fcc_eth_interrupt);
    cyg_drv_interrupt_attach(qi->fcc_eth_interrupt_handle);
    cyg_drv_interrupt_acknowledge(qi->int_vector);
    cyg_drv_interrupt_unmask(qi->int_vector);
#else

    // Mask the interrupts 
    qi->fcc_reg->fcc_fccm = 0;
#endif

    // Issue Init RX & TX Parameters Command for FCCx
    while ((IMM->cpm_cpcr & CPCR_FLG) != CPCR_READY_TO_RX_CMD); 
    IMM->cpm_cpcr = CPCR_INIT_TX_RX_PARAMS |
        fcc_chan |
        CPCR_MCN_FCC | 
        CPCR_FLG;              /* ISSUE COMMAND */
    while ((IMM->cpm_cpcr & CPCR_FLG) != CPCR_READY_TO_RX_CMD); 

    // Operating mode
    if (!_eth_phy_init(qi->phy)) {
        return false;
    }
#ifdef CYGSEM_DEVS_ETH_POWERPC_FCC_RESET_PHY
    _eth_phy_reset(qi->phy);
#endif
    phy_state = _eth_phy_state(qi->phy);
    os_printf("FCC %s: ", sc->dev_name);
    if ((phy_state & ETH_PHY_STAT_LINK) != 0) {
        if ((phy_state & ETH_PHY_STAT_100MB) != 0) {
            // Link can handle 100Mb
            os_printf("100Mb");
            if ((phy_state & ETH_PHY_STAT_FDX) != 0) {
                os_printf("/Full Duplex");
            } 
        } else {
            // Assume 10Mb, half duplex
            os_printf("10Mb");
        }
    } else {
        os_printf("/***NO LINK***\n");
#ifdef CYGPKG_REDBOOT
        return false;
#endif
    }
    os_printf("\n");


    // Initialize upper level driver for ecos
    (sc->funs->eth_drv->init)(sc, (unsigned char *)&qi->enaddr);

    return true;
}
 
//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
fcc_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
    
  // Enable the device : 
  // Set the ENT/ENR bits in the GFMR -- Enable Transmit/Receive
  qi->fcc_reg->fcc_gfmr |= (FCC_GFMR_EN_Rx | FCC_GFMR_EN_Tx);
  
}

//
// This function is called to shut down the interface.
//
static void
fcc_eth_stop(struct eth_drv_sc *sc)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
  
  // Disable the device : 
  // Clear the ENT/ENR bits in the GFMR -- Disable Transmit/Receive
  qi->fcc_reg->fcc_gfmr &= ~(FCC_GFMR_EN_Rx | FCC_GFMR_EN_Tx);
}


//
// This function is called for low level "control" operations
//
static int
fcc_eth_control(struct eth_drv_sc *sc, unsigned long key,
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
fcc_eth_can_send(struct eth_drv_sc *sc)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
  volatile struct fcc_bd *txbd = qi->txbd;
#ifndef FCC_BDs_NONCACHED
  int cache_state;
#endif

#ifndef FCC_BDs_NONCACHED
  HAL_DCACHE_IS_ENABLED(cache_state);
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fcc_eth_txring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM);
  }
#endif

  return ((txbd->ctrl & (FCC_BD_Tx_TC | FCC_BD_Tx_Ready)) == 0);
}

//
// This routine is called to send data to the hardware.
static void 
fcc_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
             int total_len, unsigned long key)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
  struct fcc_bd *txbd, *txfirst;
  volatile char *bp;
  int i, txindex;
  int cache_state;    

  HAL_DCACHE_IS_ENABLED(cache_state);
#ifndef FCC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fcc_eth_txring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM);
  }
#endif
 
  // Find a free buffer
  txbd = txfirst = qi->txbd;
  while (txbd->ctrl & FCC_BD_Tx_Ready) {
    // This buffer is busy, move to next one
    if (txbd->ctrl & FCC_BD_Tx_Wrap) {
      txbd = qi->tbase;
    } else {
      txbd++;
    }
    if (txbd == txfirst) {
#ifdef CYGPKG_NET
      panic ("No free xmit buffers");
#else
      os_printf("FCC Ethernet: No free xmit buffers\n");
#endif
    }
  }

  // Remember the next buffer to try
  if (txbd->ctrl & FCC_BD_Tx_Wrap) {
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
  txbd->ctrl |= FCC_BD_Tx_Ready | FCC_BD_Tx_Last | FCC_BD_Tx_TC;

#ifndef FCC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_FLUSH(fcc_eth_txring, 
                     8*CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM);  
  }
#endif  

}

//
// This function is called when a packet has been received.  It's job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'fcc_eth_recv' will be called to actually fetch it from the hardware.
//
static void
fcc_eth_RxEvent(struct eth_drv_sc *sc)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
  struct fcc_bd *rxbd;
  int cache_state;

  HAL_DCACHE_IS_ENABLED(cache_state);
#ifndef FCC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fcc_eth_rxring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_FCC_RxNUM);
  }
#endif

  rxbd = qi->rnext;
  while ((rxbd->ctrl & FCC_BD_Rx_Empty) == 0) {
    qi->rxbd = rxbd;  // Save for callback

    // This is the right way of doing it, but dcbi has a bug ...
    //    if (cache_state) {
    //      HAL_DCACHE_INVALIDATE(rxbd->buffer, rxbd->length); 
    //    }
    if ((rxbd->ctrl & FCC_BD_Rx_ERRORS) == 0) {
        (sc->funs->eth_drv->recv)(sc, rxbd->length);
#if 1 // Coherent caches?
        if (cache_state) {
            HAL_DCACHE_FLUSH(rxbd->buffer, rxbd->length); 
        }
#endif
    }
    // Reset control flags to known [empty] state, clearing error bits
    if (rxbd->ctrl & FCC_BD_Rx_Wrap) {
      rxbd->ctrl = FCC_BD_Rx_Empty | FCC_BD_Rx_Int | FCC_BD_Rx_Wrap;
      rxbd = qi->rbase;
    } else {
      rxbd->ctrl = FCC_BD_Rx_Empty | FCC_BD_Rx_Int;
      rxbd++;
    }
  }
  // Remember where we left off
  qi->rnext = (struct fcc_bd *)rxbd;

  // Make sure no stale data
#ifndef FCC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_FLUSH(fcc_eth_rxring, 
                     8*CYGNUM_DEVS_ETH_POWERPC_FCC_RxNUM);
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
fcc_eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
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
fcc_eth_TxEvent(struct eth_drv_sc *sc, int stat)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
  struct fcc_bd *txbd;
  int txindex;
#ifndef FCC_BDs_NONCACHED
  int cache_state;
#endif

#ifndef FCC_BDs_NONCACHED
  // Make sure no stale data
  HAL_DCACHE_IS_ENABLED(cache_state);
  if (cache_state) {
    HAL_DCACHE_INVALIDATE(fcc_eth_txring, 
                          8*CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM);
  }
#endif

  txbd = qi->tnext;
  // Note: TC field is used to indicate the buffer has/had data in it
  while ( (txbd->ctrl & (FCC_BD_Tx_TC | FCC_BD_Tx_Ready)) == FCC_BD_Tx_TC ) {
      if ((txbd->ctrl & FCC_BD_Tx_ERRORS) != 0) {
#if 0
          diag_printf("FCC Tx error BD: %x/%x- ", txbd, txbd->ctrl);
          if ((txbd->ctrl & FCC_BD_Tx_LC) != 0) diag_printf("Late Collision/");
          if ((txbd->ctrl & FCC_BD_Tx_RL) != 0) diag_printf("Retry limit/");
//          if ((txbd->ctrl & FCC_BD_Tx_RC) != 0) diag_printf("Late Collision/");
          if ((txbd->ctrl & FCC_BD_Tx_UN) != 0) diag_printf("Underrun/");
          if ((txbd->ctrl & FCC_BD_Tx_CSL) != 0) diag_printf("Carrier Lost/");
          diag_printf("\n");
#endif
      }

    txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
    (sc->funs->eth_drv->tx_done)(sc, qi->txkey[txindex], 0);
    txbd->ctrl &= ~FCC_BD_Tx_TC;
    if (txbd->ctrl & FCC_BD_Tx_Wrap) {
      txbd = qi->tbase;
    } else {
      txbd++;
    }
  }
  // Remember where we left off
  qi->tnext = (struct fcc_bd *)txbd;

  // Make sure no stale data  
#ifndef FCC_BDs_NONCACHED
  if (cache_state) {
    HAL_DCACHE_FLUSH(fcc_eth_txring, 
                     8*CYGNUM_DEVS_ETH_POWERPC_FCC_TxNUM);
  }
#endif

}

//
// Interrupt processing
//
static void          
fcc_eth_int(struct eth_drv_sc *sc)
{
  struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
  unsigned short iEvent;

  while ((iEvent = qi->fcc_reg->fcc_fcce) != 0){
    // Clear pending interrupts (writing 1's to this register)
    qi->fcc_reg->fcc_fcce = iEvent; 
    // Tx Done or Tx Error
    if ( iEvent & (FCC_EV_TXB | FCC_EV_TXE) ) {     
      fcc_eth_TxEvent(sc, iEvent);
    }
    // Complete or non-complete frame receive
    if (iEvent & (FCC_EV_RXF | FCC_EV_RXB) ) {    
      fcc_eth_RxEvent(sc);
    }
  }
}

//
// Interrupt vector
//
static int          
fcc_eth_int_vector(struct eth_drv_sc *sc)
{
    struct fcc_eth_info *qi = (struct fcc_eth_info *)sc->driver_private;
    return (qi->int_vector);
}

