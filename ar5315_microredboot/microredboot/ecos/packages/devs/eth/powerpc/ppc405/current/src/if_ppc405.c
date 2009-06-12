//==========================================================================
//
//      dev/if_ppc405.c
//
//      Ethernet device driver for PowerPC PPC405 boards
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
// Date:         2003-08-15
// Purpose:      
// Description:  hardware driver for PPC405
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet device driver for PPC405

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_powerpc_ppc405.h>
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
#include <cyg/hal/hal_if.h>
#include <cyg/hal/ppc_regs.h>

#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth_phy.h>

//
// PHY access functions
//
static void ppc405_eth_phy_init(void);
static void ppc405_eth_phy_put_reg(int reg, int phy, unsigned short data);
static bool ppc405_eth_phy_get_reg(int reg, int phy, unsigned short *val);

#include "ppc405_enet.h"
#include CYGDAT_DEVS_PPC405_ETH_INL

#define os_printf diag_printf

// For fetching the ESA from RedBoot
#include <cyg/hal/hal_if.h>
#ifndef CONFIG_ESA
#define CONFIG_ESA 6
#endif

static void          ppc405_eth_int(struct eth_drv_sc *data);

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED

static cyg_interrupt ppc405_emac_interrupt;
static cyg_handle_t  ppc405_emac_interrupt_handle;
static cyg_interrupt ppc405_mal_txeob_interrupt;
static cyg_handle_t  ppc405_mal_txeob_interrupt_handle;
static cyg_interrupt ppc405_mal_rxeob_interrupt;
static cyg_handle_t  ppc405_mal_rxeob_interrupt_handle;
static cyg_interrupt ppc405_mal_txde_interrupt;
static cyg_handle_t  ppc405_mal_txde_interrupt_handle;
static cyg_interrupt ppc405_mal_rxde_interrupt;
static cyg_handle_t  ppc405_mal_rxde_interrupt_handle;
static cyg_interrupt ppc405_mal_serr_interrupt;
static cyg_handle_t  ppc405_mal_serr_interrupt_handle;

#define EMAC_INTERRUPT_HANDLER(_int_,_hdlr_)                                                    \
    cyg_drv_interrupt_create(_int_,                                                             \
                             0,                                                                 \
                             (cyg_addrword_t)sc, /*  Data item passed to interrupt handler */   \
                             (cyg_ISR_t *)ppc405_eth_isr,                                       \
                             (cyg_DSR_t *)eth_drv_dsr,                                          \
                             &ppc405_##_hdlr_##_interrupt_handle,                               \
                             &ppc405_##_hdlr_##_interrupt);                                     \
    cyg_drv_interrupt_attach(ppc405_##_hdlr_##_interrupt_handle);                               \
    cyg_drv_interrupt_acknowledge(_int_);                                                       \
    cyg_drv_interrupt_unmask(_int_);

// This ISR is called when the ethernet interrupt occurs
static int
ppc405_eth_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;

    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_MAL_SERR);
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_MAL_TX_EOB);
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_MAL_RX_EOB);
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_MAL_TX_DE);
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_MAL_RX_DE);
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_EMAC0);
    qi->ints = vector;
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}
#endif

// Deliver function (ex-DSR) handles the ethernet [logical] processing
static void
ppc405_eth_deliver(struct eth_drv_sc *sc)
{
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    cyg_uint32 old_ints;
#endif
    ppc405_eth_int(sc);
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Allow interrupts to happen again
    HAL_DISABLE_INTERRUPTS(old_ints);
    cyg_drv_interrupt_acknowledge(qi->ints);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_MAL_SERR);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_MAL_TX_EOB);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_MAL_RX_EOB);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_MAL_TX_DE);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_MAL_RX_DE);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EMAC0);
    HAL_RESTORE_INTERRUPTS(old_ints);
#endif
}

//
// PHY unit access
//
static void 
ppc405_eth_phy_init(void)
{
    // Set up MII hardware - nothing to do on this platform
}

static void 
ppc405_eth_phy_put_reg(int reg, int phy, unsigned short data)
{
    unsigned long reg_val;

    reg_val = EMAC0_STACR_STAC_WRITE | EMAC0_STACR_OPBC_66;
    reg_val |= (phy << EMAC0_STACR_PCDA_SHIFT) | reg;
    reg_val |= (data << EMAC0_STACR_PHYD_SHIFT);
#ifdef PHY_DEBUG
    os_printf("PHY PUT - reg: %d, phy: %d, val: %04x [%08x]\n", reg, phy, data, reg_val);
#endif
    while ((EMAC0_STACR & EMAC0_STACR_OC) == 0) ;  // Wait for MII free
    EMAC0_STACR = reg_val;
    while ((EMAC0_STACR & EMAC0_STACR_OC) == 0) ;  // Wait for MII complete
}

static bool 
ppc405_eth_phy_get_reg(int reg, int phy, unsigned short *val)
{
    unsigned long reg_val;

    reg_val = EMAC0_STACR_STAC_READ | EMAC0_STACR_OPBC_66;
    reg_val |= (phy << EMAC0_STACR_PCDA_SHIFT) | reg;
#ifdef PHY_DEBUG
    os_printf("PHY GET - reg: %d, phy: %d [%08x] = ", reg, phy, reg_val);
#endif
    while ((EMAC0_STACR & EMAC0_STACR_OC) == 0) ;  // Wait for MII free
    EMAC0_STACR = reg_val;
    while ((EMAC0_STACR & EMAC0_STACR_OC) == 0) ;  // Wait for MII complete
    if ((EMAC0_STACR & EMAC0_STACR_PHYE) == 0) {
        // Operation completed with no error
        *val = (EMAC0_STACR & EMAC0_STACR_PHYD) >> EMAC0_STACR_PHYD_SHIFT;
#ifdef PHY_DEBUG
        os_printf("%04x\n", *val);
#endif
        return true;
    } else {
        // No response
#ifdef PHY_DEBUG
        os_printf("***ERROR***\n");
#endif
        return false;
    }
}

//
// [re]Initialize the ethernet controller
//   Done separately since shutting down the device requires a 
//   full reconfiguration when re-enabling.
//   when 
static bool
ppc405_eth_reset(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    volatile mal_bd_t *rxbd, *RxBD, *txbd, *TxBD;
    unsigned char *RxBUF, *TxBUF;
    int i, int_state;
    unsigned long mal_status, mode;
    unsigned short phy_state = 0;

    // Ignore unless device is idle/stopped
    if ((EMAC0_MR0 & (EMAC0_MR0_RXI|EMAC0_MR0_TXI)) != (EMAC0_MR0_RXI|EMAC0_MR0_TXI)) {
        return true;
    }

    // Make sure interrupts are off while we mess with the device
    HAL_DISABLE_INTERRUPTS(int_state);

    // Reset EMAC controller
    EMAC0_MR0 |= EMAC0_MR0_SRST;
    i = 0;
    while ((EMAC0_MR0 & EMAC0_MR0_SRST) != 0) {
        if (++i >= 500000) {
            os_printf("PPC405 Ethernet does not reset\n");
            HAL_RESTORE_INTERRUPTS(int_state);
            return false;
        }
    }

    TxBD = qi->txbd_table;
    txbd = (mal_bd_t *)CYGARC_UNCACHED_ADDRESS(TxBD);
    RxBD = qi->rxbd_table;
    rxbd = (mal_bd_t *)CYGARC_UNCACHED_ADDRESS(RxBD);
    qi->tbase = qi->txbd = qi->tnext = txbd;
    qi->rbase = qi->rxbd = qi->rnext = rxbd;
    qi->txactive = 0;

    RxBUF = qi->rxbuf;
    TxBUF = qi->txbuf;

    // setup buffer descriptors
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_PPC405_RxNUM;  i++) {
        rxbd->length = 0;
        rxbd->buffer = (unsigned long)RxBUF;
        rxbd->status = MAL_BD_R | MAL_BD_I;
        RxBUF += CYGNUM_DEVS_ETH_POWERPC_PPC405_BUFSIZE;
        rxbd++;
    }
    rxbd--;
    rxbd->status |= MAL_BD_W;  // Last buffer
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_PPC405_TxNUM;  i++) {
        txbd->length = 0;
        txbd->buffer = (unsigned long)TxBUF;
        txbd->status = 0;
        TxBUF += CYGNUM_DEVS_ETH_POWERPC_PPC405_BUFSIZE;
        txbd++;
    }
    txbd--;
    txbd->status |= MAL_BD_W;  // Last buffer

    // Tell memory access layer where the buffer descriptors are
    CYGARC_MTDCR(MAL0_TXCARR, MAL_CASR_C0|MAL_CASR_C1);  // Disable/reset channel #0 & #1
    CYGARC_MTDCR(MAL0_RXCARR, MAL_CASR_C0);  // Disable/reset channel #0
    CYGARC_MTDCR(MAL0_CFG, MAL_CFG_SR);
    i = 0;
    CYGARC_MFDCR(MAL0_CFG, mal_status);
    while ((mal_status & MAL_CFG_SR) != 0) {
        if (++i >= 500000) {
            os_printf("PPC405 MAL does not reset\n");
            HAL_RESTORE_INTERRUPTS(int_state);
            return false;
        }
    }
    CYGARC_MTDCR(MAL0_CFG, MAL_CFG_PLBB | MAL_CFG_OPBBL | MAL_CFG_LEA | MAL_CFG_PLBT_DEFAULT);
    CYGARC_MTDCR(MAL0_TXCTP0R, TxBD);
    CYGARC_MTDCR(MAL0_RXCTP0R, RxBD);
    CYGARC_MTDCR(MAL0_RXBS0, (CYGNUM_DEVS_ETH_POWERPC_PPC405_BUFSIZE/16));  // Receive buffer size

    // Set device physical address (ESA)
    EMAC0_IAHR = (enaddr[0]<<8) | (enaddr[1]<<0);
    EMAC0_IALR = (enaddr[2]<<24) | (enaddr[3]<<16) | (enaddr[4]<<8) | (enaddr[5]<<0);

    // Operating mode
    if (!_eth_phy_init(qi->phy)) {
        return false;
    }
    phy_state = _eth_phy_state(qi->phy);
    os_printf("PPC405 ETH: ");
    mode = EMAC0_MR1_RFS_4096 | EMAC0_MR1_TFS_2048 | EMAC0_MR1_TR0_MULTI | EMAC0_MR1_APP;
    if ((phy_state & ETH_PHY_STAT_LINK) != 0) {
        if ((phy_state & ETH_PHY_STAT_100MB) != 0) {
            // Link can handle 100Mb
            mode |= EMAC0_MR1_MF_100MB;
            os_printf("100Mb");
            if ((phy_state & ETH_PHY_STAT_FDX) != 0) {
                mode |= EMAC0_MR1_FDE | EMAC0_MR1_EIFC | EMAC0_MR1_IST;
                os_printf("/Full Duplex");
            } 
        } else {
            // Assume 10Mb, half duplex
            mode |= EMAC0_MR1_MF_10MB;
            os_printf("10Mb");
        }
    } else {
        os_printf("/***NO LINK***");
        return false;
    }
    os_printf("\n");
    EMAC0_MR1 = mode;

    // Configure receiver
    EMAC0_RMR = EMAC0_RMR_IAE | EMAC0_RMR_BAE | EMAC0_RMR_RRP | EMAC0_RMR_RFP;

    // Transmit threshold to 256 bytes
    EMAC0_TRTR = ((256/EMAC0_TRTR_TRT_SCALE)-1) << EMAC0_TRTR_TRT_SHIFT;

    // Receive FIFO watermarks
    EMAC0_RWMR = (0x1F<<EMAC0_RWMR_RLWM_SHIFT) | (0x10<<EMAC0_RWMR_RHWM_SHIFT);

    // Frame gap
    EMAC0_IPGVR = 8;

    // Enable MAL
    CYGARC_MTDCR(MAL0_TXCASR, MAL_CASR_C0);
    CYGARC_MTDCR(MAL0_RXCASR, MAL_CASR_C0);

    // Reset all interrupts
    EMAC0_ISR = 0xFFFFFFFF;
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    qi->ints = 0;
#endif

    // Enable interface
    EMAC0_MR0 |= (EMAC0_MR0_RXE|EMAC0_MR0_TXE);

    // Restore interrupts
    HAL_RESTORE_INTERRUPTS(int_state);
    return true;
}

//
// Initialize the interface - performed at system startup
// This function must set up the interface, including arranging to
// handle interrupts, etc, so that it may be "started" cheaply later.
//

static bool 
ppc405_eth_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    bool esa_ok;
    unsigned char enaddr[6];

    ppc405_eth_stop(sc);  // Make sure it's not running yet

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Set up to handle interrupts
    EMAC_INTERRUPT_HANDLER(CYGNUM_HAL_INTERRUPT_MAL_SERR, mal_serr);
    EMAC_INTERRUPT_HANDLER(CYGNUM_HAL_INTERRUPT_MAL_TX_EOB, mal_txeob);
    EMAC_INTERRUPT_HANDLER(CYGNUM_HAL_INTERRUPT_MAL_RX_EOB, mal_rxeob);
    EMAC_INTERRUPT_HANDLER(CYGNUM_HAL_INTERRUPT_MAL_TX_DE, mal_txde);
    EMAC_INTERRUPT_HANDLER(CYGNUM_HAL_INTERRUPT_MAL_RX_DE, mal_rxde);
    EMAC_INTERRUPT_HANDLER(CYGNUM_HAL_INTERRUPT_EMAC0, emac);
#endif

    // Get physical device address
#ifdef CYGPKG_REDBOOT
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    esa_ok = flash_get_config(qi->esa_key, enaddr, CONFIG_ESA);
#else
    esa_ok = false;
#endif
#else
    esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
                                         qi->esa_key, enaddr, CONFIG_ESA);
#endif
    if (!esa_ok) {
        // Can't figure out ESA
        os_printf("PPC405_ETH - Warning! ESA unknown\n");
        memcpy(&enaddr, qi->enaddr, sizeof(enaddr));
    }

    // Configure the device
    if (!ppc405_eth_reset(sc, enaddr, 0)) {
        return false;
    }

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, (unsigned char *)&enaddr);
    
    return true;
}
 
//
// This function is called to shut down the interface.
//
static void
ppc405_eth_stop(struct eth_drv_sc *sc)
{
    EMAC0_MR0 &= ~(EMAC0_MR0_RXE|EMAC0_MR0_TXE);
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
ppc405_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    EMAC0_MR0 |= (EMAC0_MR0_RXE|EMAC0_MR0_TXE);
}

//
// This function is called for low level "control" operations
//
static int
ppc405_eth_control(struct eth_drv_sc *sc, unsigned long key,
                  void *data, int length)
{
    os_printf("%s.%d\n", __FUNCTION__, __LINE__);
    return 1;
#if 0
#ifdef ETH_DRV_SET_MC_ALL
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    volatile struct ppc405 *ppc405 = qi->ppc405;
#endif

    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
        return 0;
        break;
#ifdef ETH_DRV_SET_MC_ALL
    case ETH_DRV_SET_MC_ALL:
    case ETH_DRV_SET_MC_LIST:
        ppc405->RxControl &= ~RxControl_PROM;
        ppc405->hash[0] = 0xFFFFFFFF;
        ppc405->hash[1] = 0xFFFFFFFF;
        return 0;
        break;
#endif
    default:
        return 1;
        break;
    }
#endif
}

//
// This function is called to see if another packet can be sent.
// It should return the number of packets which can be handled.
// Zero should be returned if the interface is busy and can not send any more.
//
static int
ppc405_eth_can_send(struct eth_drv_sc *sc)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;

    return (qi->txactive < CYGNUM_DEVS_ETH_POWERPC_PPC405_TxNUM);
}

//
// This routine is called to send data to the hardware.

static void 
ppc405_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
               int total_len, unsigned long key)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    volatile mal_bd_t *txbd;
    volatile char *bp;
    int i, txindex;

    // Find a free buffer
    txbd = qi->txbd;
    // Set up buffer
    bp = (volatile char *)CYGARC_UNCACHED_ADDRESS(txbd->buffer);
    for (i = 0;  i < sg_len;  i++) {
        memcpy((void *)bp, (void *)sg_list[i].buf, sg_list[i].len);
        bp += sg_list[i].len;
    } 
    txbd->length = total_len;
    txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
    qi->txkey[txindex] = key;
    // Send it on it's way
    txbd->status = (txbd->status & MAL_BD_W) | MAL_BD_R | MAL_BD_L | MAL_BD_I | 
        MAL_BD_TX_GFCS | MAL_BD_TX_GPAD;
    qi->txactive++;
    EMAC0_TMR0 = EMAC0_TMR0_GNP0;  // Start channel 0
    // Remember the next buffer to try
    if (txbd->status & MAL_BD_W) {
        qi->txbd = qi->tbase;
    } else {
        qi->txbd = txbd+1;
    }
}

//
// This function is called when a packet has been received.  It's job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'ppc405_eth_recv' will be called to actually fetch it from the hardware.
//
static void
ppc405_eth_RxEvent(struct eth_drv_sc *sc)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    volatile mal_bd_t *rxbd, *rxfirst;

    rxbd = rxfirst = qi->rnext;
    while ((rxbd->status & MAL_BD_R) == 0) {
        qi->rxbd = rxbd;  // Save for callback
        (sc->funs->eth_drv->recv)(sc, rxbd->length-4);  // Adjust for FCS
        if (rxbd->status & MAL_BD_W) {
            rxbd = qi->rbase;
        } else {
            rxbd++;
        }
    }
    // Remember where we left off
    qi->rnext = (mal_bd_t *)rxbd;
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// It's job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
ppc405_eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    unsigned char *bp;
    int i;

    bp = (unsigned char *)CYGARC_UNCACHED_ADDRESS(qi->rxbd->buffer);
    for (i = 0;  i < sg_len;  i++) {
        if (sg_list[i].buf != 0) {
            memcpy((void *)sg_list[i].buf, bp, sg_list[i].len);
            bp += sg_list[i].len;
        }
    }
    qi->rxbd->status = (qi->rxbd->status & (MAL_BD_W|MAL_BD_I)) | MAL_BD_R;
}

static void
ppc405_eth_TxEvent(struct eth_drv_sc *sc)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    volatile mal_bd_t *txbd;
    int key, txindex;

    txbd = qi->tnext;
    while ((txbd->status & (MAL_BD_R|MAL_BD_I)) == MAL_BD_I) {
        txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
        if ((key = qi->txkey[txindex]) != 0) {
            qi->txkey[txindex] = 0;
            (sc->funs->eth_drv->tx_done)(sc, key, 0);
        }
        qi->txactive -= 1;
        txbd->status &= MAL_BD_W;  //  Only preserve wrap bit
        if ((txbd->status & MAL_BD_W) != 0) {
            txbd = qi->tbase;
        } else {
            txbd++;
        }
        if (txbd == qi->tnext) {
            break;  // Went through whole list
        }
    }
    // Remember where we left off
    qi->tnext = (mal_bd_t *)txbd;
}

//
// Interrupt processing
//
int dump_mal0_esr = 0;

static void          
ppc405_eth_int(struct eth_drv_sc *sc)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    unsigned long event, tx_event, rx_event, tx_deir, rx_deir;

    CYGARC_MFDCR(MAL0_TXEOBISR, tx_event);
    if (tx_event != 0) {
        ppc405_eth_TxEvent(sc);
        CYGARC_MTDCR(MAL0_TXEOBISR, tx_event);
    }
    CYGARC_MFDCR(MAL0_RXEOBISR, rx_event);
    if (rx_event != 0) {
        ppc405_eth_RxEvent(sc);
        CYGARC_MTDCR(MAL0_RXEOBISR, rx_event);
    }
    if ((event = EMAC0_ISR) != 0) {
        if ((event & ~(EMAC0_ISR_SE0|EMAC0_ISR_SE1)) != 0) {
            os_printf("EMAC0_ISR: %x\n", event);
        }
        if ((event & (EMAC0_ISR_TE0|EMAC0_ISR_TE1)) != 0) {
            // Some problem with transmit
            CYGARC_MTDCR(MAL0_TXCASR, MAL_CASR_C0);
            qi->tnext = qi->tbase;
        }
        EMAC0_ISR = event;  // Reset the bits we handled
    }
    CYGARC_MFDCR(MAL0_ESR, event);
    if ((event & MAL_ESR_INT_MASK) != 0) {
        CYGARC_MFDCR(MAL0_TXDEIR, tx_deir);
        CYGARC_MFDCR(MAL0_RXDEIR, rx_deir);
        if (dump_mal0_esr) {
        os_printf("MAL0_ESR: %x, Tx: %x, Rx: %x\n", event, tx_deir, rx_deir);
        os_printf("Tx buffer headers\n");
        diag_dump_buf((void *)qi->tbase, qi->txnum*sizeof(mal_bd_t));
        os_printf("Rx buffer headers\n");
        diag_dump_buf((void *)qi->rbase, qi->rxnum*sizeof(mal_bd_t));
        }
        if (tx_deir != 0) {
            // Fix Tx descriptor problems
            CYGARC_MTDCR(MAL0_TXDEIR, tx_deir);  // Clear interrupt indicator
            CYGARC_MTDCR(MAL0_TXCASR, MAL_CASR_C0);
            qi->tnext = qi->tbase;
        }
        if (rx_deir != 0) {
            // Fix Rx descriptor problems
            CYGARC_MTDCR(MAL0_RXDEIR, rx_deir);  // Clear interrupt indicator
            CYGARC_MTDCR(MAL0_RXCASR, MAL_CASR_C0);
            qi->rnext = qi->rbase;
        }
        CYGARC_MTDCR(MAL0_ESR, event);  // Clear events just handled
    }
}

//
// Interrupt vector
//
static int          
ppc405_eth_int_vector(struct eth_drv_sc *sc)
{
    struct ppc405_eth_info *qi = (struct ppc405_eth_info *)sc->driver_private;
    return qi->int_vector;
}
