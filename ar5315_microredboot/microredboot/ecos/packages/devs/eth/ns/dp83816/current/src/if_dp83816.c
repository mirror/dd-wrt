//==========================================================================
//
//      dev/if_dp83816.c
//
//      Ethernet device driver for NS DP83816 ethernet controller
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003, 2004 Gary Thomas
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
// Contributors: 
// Date:         2003-09-29
// Purpose:      
// Description:
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

#include "dp83816.h"
#include CYGDAT_DEVS_ETH_NS_DP83816_INL

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
// This ISR is called when the ethernet interrupt occurs
static cyg_uint32
dp83816_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
    dp83816_priv_data_t *dp = (dp83816_priv_data_t *)sc->driver_private;

    DEBUG_FUNCTION();

    cyg_drv_interrupt_mask(dp->interrupt);
    cyg_drv_interrupt_acknowledge(dp->interrupt);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}
#endif // CYGINT_IO_ETH_INT_SUPPORT_REQUIRED

// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void
dp83816_deliver(struct eth_drv_sc *sc)
{
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    dp83816_priv_data_t *dp = (dp83816_priv_data_t *)sc->driver_private;
#endif

    DEBUG_FUNCTION();

    // Service the interrupt:
    dp83816_poll(sc);
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(dp->interrupt);
#endif
}

static bool
dp83816_reset(dp83816_priv_data_t *dp)
{
    unsigned char *bp;
    dp83816_bd_t *bdp;
    cyg_uint32 stat;
    int i, timeout;

    DP_OUT(dp->base, DP_CR, _CR_RST);  // Reset device
    timeout = 10000;
    do {
        DP_IN(dp->base, DP_CR, stat);
    } while (((stat & _CR_RST) != 0) && (--timeout > 0));
    if (timeout == 0) {
        diag_printf("DP83816 - reset timed out! - stat: %x\n", stat);
        return false;
    }
    // Rx ring
    bdp = dp->rxnext = CYGARC_UNCACHED_ADDRESS(dp->rxd);
    bp = dp->rxbuf;
    for (i = 0; i < dp->rxnum; i++, bdp++) {
        bdp->next = (dp83816_bd_t *)_h2le(CYGARC_PHYSICAL_ADDRESS(bdp+1));
        bdp->stat = _h2le(BD_INTR | _DP83816_BUFSIZE);  // Max buffer
        bdp->buf = (unsigned char *)_h2le(CYGARC_PHYSICAL_ADDRESS(bp));
        bp += _DP83816_BUFSIZE;
    }
    bdp--;  bdp->next = (dp83816_bd_t *)_h2le(CYGARC_PHYSICAL_ADDRESS(dp->rxd));
    DP_OUT(dp->base, DP_RXCFG, _RXCFG_MXDMA_128 | ((64/32)<<_RXCFG_DRTH_SHIFT));
    DP_OUT(dp->base, DP_RXDP, CYGARC_PHYSICAL_ADDRESS(dp->rxd));
    // Tx ring
    bdp = dp->txfill = dp->txint = CYGARC_UNCACHED_ADDRESS(dp->txd);
    bp = dp->txbuf;
    for (i = 0; i < dp->txnum; i++, bdp++) {
        bdp->next = (dp83816_bd_t *)_h2le(CYGARC_PHYSICAL_ADDRESS(bdp+1));
        bdp->stat = 0;  // Driver owns buffer for now
        bdp->buf = (unsigned char *)_h2le(CYGARC_PHYSICAL_ADDRESS(bp));
        bp += _DP83816_BUFSIZE;
    }
    bdp--;  bdp->next = (dp83816_bd_t *)_h2le(CYGARC_PHYSICAL_ADDRESS(dp->txd));
    DP_OUT(dp->base, DP_TXCFG, _TXCFG_ATP |
                               _TXCFG_MXDMA_128 |
                               ((256/32)<<_TXCFG_FLTH_SHIFT) |
                               ((512/32)<<_TXCFG_DRTH_SHIFT));
    DP_OUT(dp->base, DP_TXDP, CYGARC_PHYSICAL_ADDRESS(dp->txd));
    dp->txbusy = 0;
    // Fill in ESA
    for (i = 0;  i < 6;  i+=2) {
        DP_OUT(dp->base, DP_RFCR, i);
        DP_OUT(dp->base, DP_RFDR, dp->enaddr[i] | (dp->enaddr[i+1]<<8));
    }
    // Setup up acceptance criteria
    DP_OUT(dp->base, DP_RFCR, _RFCR_RFEN | _RFCR_AAB | _RFCR_APM);
    // Set up interrupts
    DP_IN(dp->base, DP_ISR, stat);  // Clear any current interrupts
    DP_OUT(dp->base, DP_IMR, 0x00000000);  // Disable them all!
    DP_OUT(dp->base, DP_IER, 0);
    return true;
}

static bool 
dp83816_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    dp83816_priv_data_t *dp = (dp83816_priv_data_t *)sc->driver_private;
    cyg_uint8 *base;
    bool esa_ok;
    unsigned char enaddr[6];

    // Get physical device address
#ifdef CYGPKG_REDBOOT
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    esa_ok = flash_get_config(dp->esa_key, enaddr, CONFIG_ESA);
#else
    esa_ok = false;
#endif
#else
    esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
                                         dp->esa_key, enaddr, CONFIG_ESA);
#endif
    if (esa_ok) {
        memcpy(dp->enaddr, enaddr, sizeof(enaddr));
    } else {
        // Can't figure out ESA
        diag_printf("DP83816 - Warning! ESA unknown\n");
    }

    DEBUG_FUNCTION();

    CYGHWR_NS_DP83816_PLF_INIT(dp);
    base = dp->base;
    if (!base) return false;  // No device found

    if (!dp83816_reset(dp)) return false;

    diag_printf("DP83816 - ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                dp->enaddr[0], dp->enaddr[1], dp->enaddr[2],
                dp->enaddr[3], dp->enaddr[4], dp->enaddr[5] );

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    cyg_drv_interrupt_create(
        dp->interrupt,
        0,                       // Priority - unused
        (cyg_addrword_t)sc,      // Data item passed to ISR & DSR
        dp83816_isr,             // ISR
        eth_drv_dsr,             // DSR
        &dp->interrupt_handle,   // handle to intr obj
        &dp->interrupt_object ); // space for int obj

    cyg_drv_interrupt_attach(dp->interrupt_handle);
    cyg_drv_interrupt_unmask(dp->interrupt);
#endif

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, dp->enaddr);

    return true;
}

static void
dp83816_stop(struct eth_drv_sc *sc)
{
    dp83816_priv_data_t *dp = (dp83816_priv_data_t *)sc->driver_private;

    DP_OUT(dp->base, DP_IMR, 0x00000000);  // Disable interrupts
    DP_OUT(dp->base, DP_IER, 0);
    DP_OUT(dp->base, DP_CR, _CR_RXD | _CR_TXD);
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
dp83816_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    dp83816_priv_data_t *dp = (dp83816_priv_data_t *)sc->driver_private;

    DP_OUT(dp->base, DP_IMR, 0xFFFFFFFF);  // Enable interrupts
    DP_OUT(dp->base, DP_IER, 1);
    DP_OUT(dp->base, DP_CR, _CR_RXE | _CR_TXE);
}

//
// This routine is called to perform special "control" opertions
//
static int
dp83816_control(struct eth_drv_sc *sc, unsigned long key,
               void *data, int data_len)
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
// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
//
static int
dp83816_can_send(struct eth_drv_sc *sc)
{
    dp83816_priv_data_t *dp = (dp83816_priv_data_t *)sc->driver_private;

    DEBUG_FUNCTION();
    return (dp->txnum - dp->txbusy);
}

//
// This routine is called to send data to the hardware.  It is known a-priori
// that there is free buffer space (dp->tx_next).
//
static void 
dp83816_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
            int total_len, unsigned long key)
{
    struct dp83816_priv_data *dp = (struct dp83816_priv_data *)sc->driver_private;
    int i, len;
    unsigned char *data;
    dp83816_bd_t *bdp = dp->txfill;

    DEBUG_FUNCTION();

    len = total_len;
    if (len < IEEE_8023_MIN_FRAME) len = IEEE_8023_MIN_FRAME;
    data = (unsigned char *)CYGARC_VIRTUAL_ADDRESS(_le2h((unsigned long)bdp->buf));
    for (i = 0;  i < sg_len;  i++) {
        memcpy(data, (unsigned char *)sg_list[i].buf, sg_list[i].len);
        data += sg_list[i].len;
    }
    bdp->key = key;
    bdp->stat = _h2le(len | BD_OWN | BD_INTR);
    dp->txbusy++;
    bdp = (dp83816_bd_t *)CYGARC_UNCACHED_ADDRESS(CYGARC_VIRTUAL_ADDRESS(_le2h((unsigned long)bdp->next)));
    dp->txfill = bdp;
    // Kick the device, in case it went idle
    DP_OUT(dp->base, DP_CR, _CR_TXE);
}

static void
dp83816_TxEvent(struct eth_drv_sc *sc)
{
    struct dp83816_priv_data *dp = (struct dp83816_priv_data *)sc->driver_private;
    dp83816_bd_t *bdp = dp->txint;

    DEBUG_FUNCTION();
    while ((_le2h(bdp->stat) & (BD_OWN|BD_INTR)) == BD_INTR) {
        // Tell higher level we sent this packet
        (sc->funs->eth_drv->tx_done)(sc, bdp->key, 0);
        bdp->stat = 0;  // retake buffer
        bdp->key = 0;
        dp->txbusy--;
        bdp = (dp83816_bd_t *)CYGARC_UNCACHED_ADDRESS(CYGARC_VIRTUAL_ADDRESS(_le2h((unsigned long)bdp->next)));
    }
    dp->txint = bdp;
}

//
// This function is called when a packet has been received.  It's job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'dp83816_recv' will be called to actually fetch it from the hardware.
//
static void
dp83816_RxEvent(struct eth_drv_sc *sc)
{
    struct dp83816_priv_data *dp = (struct dp83816_priv_data *)sc->driver_private;
    dp83816_bd_t *bdp = CYGARC_UNCACHED_ADDRESS(dp->rxd);
    dp83816_bd_t *bdfirst = CYGARC_UNCACHED_ADDRESS(dp->rxd);
    int len, err;

    DEBUG_FUNCTION();

    while (true) {
        if ((_le2h(bdp->stat) & BD_OWN) != 0) {
            err = _le2h(bdp->stat) & (BD_RXA|BD_RXO|BD_LONG|BD_RUNT|BD_ISE|BD_CRCE|BD_FAE|BD_COL);
            if (err != 0) {
                diag_printf("RxError: %x\n", err);
            }
            len = _le2h(bdp->stat) & BD_LENGTH_MASK;
            dp->rxnext = bdp;
            (sc->funs->eth_drv->recv)(sc, len);
            bdp->stat = _h2le(BD_INTR | _DP83816_BUFSIZE);  // Give back buffer
        }
        bdp = (dp83816_bd_t *)CYGARC_UNCACHED_ADDRESS(CYGARC_VIRTUAL_ADDRESS(_le2h((unsigned long)bdp->next)));
        if (bdp == bdfirst) {
            break;
        }
    }
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// It's job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
dp83816_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct dp83816_priv_data *dp = (struct dp83816_priv_data *)sc->driver_private;
    dp83816_bd_t *bdp = dp->rxnext;
    unsigned char *data;
    int i;

    data = (unsigned char *)CYGARC_VIRTUAL_ADDRESS(_le2h((unsigned long)bdp->buf));
    for (i = 0;  i < sg_len;  i++) {
        memcpy((void *)sg_list[i].buf, data, sg_list[i].len);
        data += sg_list[i].len;
    }
}

static void
dp83816_warm_reset(struct eth_drv_sc *sc)
{
    struct dp83816_priv_data *dp = (struct dp83816_priv_data *)sc->driver_private;
    dp83816_bd_t *bdp;
    int i;

    // Free up any active Tx buffers
    bdp = CYGARC_UNCACHED_ADDRESS(dp->txd);
    for (i = 0; i < dp->txnum; i++, bdp++) {
        if (bdp->key) {
            (sc->funs->eth_drv->tx_done)(sc, bdp->key, 0);
        }
    }
    // Reset the device
    dp83816_reset(dp);
    DP_OUT(dp->base, DP_CR, _CR_RXE | _CR_TXE);
}

static void
dp83816_poll(struct eth_drv_sc *sc)
{
    struct dp83816_priv_data *dp = (struct dp83816_priv_data *)sc->driver_private;
    unsigned long stat, cr_stat;

    DP_IN(dp->base, DP_ISR, stat);
    do {
        if ((stat & (_ISR_TXDESC|_ISR_TXOK)) != 0) {
            dp83816_TxEvent(sc);
        }
        if ((stat & (_ISR_RXDESC|_ISR_RXOK|_ISR_RXERR)) != 0) {
            dp83816_RxEvent(sc);
        }
        DP_IN(dp->base, DP_CR, cr_stat);
        if ((stat & (_ISR_HIBERR|_ISR_TXURN|_ISR_RXORN)) != 0) {            
#if 0
            diag_printf("DP83816 - major error: %x, cmd_stat: %x\n", stat, cr_stat);
#endif
            // Try to reset the device
            dp83816_warm_reset(sc);
        }
        if (((cr_stat & _CR_RXE) == 0) ||
            ((dp->txbusy > 1) && ((cr_stat & _CR_TXE) == 0))) {
#if 0
            // What happened?
            diag_printf("DP83816 went to lunch? - stat: %x/%x, txbusy: %x\n", cr_stat, stat, dp->txbusy);
#endif
            // Try to reset the device
            dp83816_warm_reset(sc);
        }
        DP_IN(dp->base, DP_ISR, stat);
    } while (stat != 0);
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    CYGHWR_NS_DP83816_PLF_INT_CLEAR(dp);
#endif
}

static int
dp83816_int_vector(struct eth_drv_sc *sc)
{
    struct dp83816_priv_data *dp = (struct dp83816_priv_data *)sc->driver_private;
    return dp->interrupt;
}
