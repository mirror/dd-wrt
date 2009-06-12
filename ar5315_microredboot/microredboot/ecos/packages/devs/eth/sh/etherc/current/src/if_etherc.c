//==========================================================================
//
//      dev/if_etherc.c
//
//      Ethernet device driver for SH EtherC CPU module controller
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2002-01-30
// Purpose:      
// Description:  Hardware driver for SH EtherC CPU module controller
//
// Notes:        The KEEP_STATISTICS code is not implemented yet. Look
//               for FIXME macro.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_sh_etherc.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // delays
#include <string.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#include <net/if.h>  /* Needed for struct ifnet */
#include <pkgconf/io_eth_drivers.h>
#endif
#include CYGHWR_MEMORY_LAYOUT_H

#define FIXME 0

// Set size so it is 16-byte aligned
#define _BUF_SIZE (1544+8)

#define IEEE_8023_MIN_FRAME           64    // Smallest possible ethernet frame

#ifdef CYGPKG_INFRA_DEBUG
// Then we log, OOI, the number of times we get a bad packet number
// from the tx done fifo.
int etherc_txfifo_good = 0;
int etherc_txfifo_bad = 0;
#endif

#include "sh_etherc.h"
#define __WANT_DEVS
#include CYGDAT_DEVS_ETH_SH_ETHERC_INL
#undef  __WANT_DEVS

static void       etherc_poll(struct eth_drv_sc *sc);

static cyg_uint16 etherc_read_MII(struct etherc_priv_data *cpd, int id, int reg);
static void       etherc_write_MII(struct etherc_priv_data *cpd, int id, int reg, cyg_uint16 value);
#define _MII_READ(_priv_, _id_, _reg_) etherc_read_MII(_priv_, _id_, _reg_)
#define _MII_WRITE(_priv_, _id_, _reg_, _val_) etherc_write_MII(_priv_, _id_, _reg_, _val_)
#define _MII_HAS_EXTENDED
#include "phyter.inl"

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
// This ISR is called when the ethernet interrupt occurs
static cyg_uint32
etherc_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    struct etherc_priv_data *cpd = (struct etherc_priv_data *)data;

    DEBUG_FUNCTION();

    INCR_STAT( interrupts );

    cyg_drv_interrupt_mask(cpd->interrupt);
    cyg_drv_interrupt_acknowledge(cpd->interrupt);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

static void
etherc_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // This conditioning out is necessary because of explicit calls to this
    // DSR - which would not ever be called in the case of a polled mode
    // usage ie. in RedBoot.
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    struct etherc_priv_data* cpd = (struct etherc_priv_data *)data;
    struct cyg_netdevtab_entry *ndp = (struct cyg_netdevtab_entry *)(cpd->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

    // but here, it must be a *sc:
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );
#else
# ifndef CYGPKG_REDBOOT
#  error Empty Etherc ethernet DSR is compiled.  Is this what you want?
# endif
#endif
}
#endif // CYGPKG_IO_ETH_DRIVERS_STAND_ALONE


// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void
etherc_deliver(struct eth_drv_sc *sc)
{
    struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    // Service the interrupt:
    etherc_poll(sc);
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(cpd->interrupt);
}

static int
etherc_int_vector(struct eth_drv_sc *sc)
{
    struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;

    return (cpd->interrupt);
}


static bool 
sh_etherc_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;
    cyg_uint8 *p, *d;
    cyg_uint32 reg;
    int i;
    bool esa_configured = true;

    DEBUG_FUNCTION();

    cpd->txbusy = 0;

    // Ensure that addresses of ring descriptors and buffers are properly aligned
    // and in uncached memory.
    cpd->rx_buffers = (cyg_uint8*)CYGARC_UNCACHED_ADDRESS(((cyg_uint32)cpd->rx_buffers+16-1) & ~(16-1));
    cpd->rx_ring = (cyg_uint8*)CYGARC_UNCACHED_ADDRESS(((cyg_uint32)cpd->rx_ring+16-1) & ~(16-1));
    cpd->tx_buffers = (cyg_uint8*)CYGARC_UNCACHED_ADDRESS(((cyg_uint32)cpd->tx_buffers+16-1) & ~(16-1));
    cpd->tx_ring = (cyg_uint8*)CYGARC_UNCACHED_ADDRESS(((cyg_uint32)cpd->tx_ring+16-1) & ~(16-1));

#if DEBUG & 8
    db_printf("Etherc at base 0x%08x\n", cpd->base);
#endif

    // Find ESA - check possible sources in sequence and stop when
    // one provides the ESA:
    //   RedBoot option (via provide_esa)
    //   Compile-time configuration
    //   EEPROM
    //   <fail configuration of device>
    if (NULL != cpd->provide_esa) {
        esa_configured = cpd->provide_esa(cpd);
# if DEBUG & 8
        if (esa_configured)
            diag_printf("Got ESA from RedBoot option\n");
# endif
    }
    if (!esa_configured && cpd->hardwired_esa) {
        // ESA is already set in cpd->esa[]
        esa_configured = true;
    }
    if (!esa_configured) {
# if DEBUG & 8
        diag_printf("EtherC - no EEPROM, static ESA or RedBoot config option.\n");
# endif
        return false;
    }

#if DEBUG & 9
    db_printf("ETHERC ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                cpd->esa[0], cpd->esa[1], cpd->esa[2],
                cpd->esa[3], cpd->esa[4], cpd->esa[5] );
#endif

    // Prepare RX and TX rings
    p = cpd->rx_ring;
    d = cpd->rx_buffers;
    for (i = 0; i < cpd->rx_ring_cnt; i++) {
        _SU32(p, ETHERC_RD_STAT) = ETHERC_RD_STAT_RACT | ETHERC_RD_STAT_RFP_OTO;
        _SU16(p, ETHERC_RD_RBL) = _BUF_SIZE;
        _SU16(p, ETHERC_RD_RDL) = 0;
        _SU32(p, ETHERC_RD_RBA) = (cyg_uint32)d;
        _SU32(p, ETHERC_RD_PAD) = 0;
        p += ETHERC_RD_SIZE;
        d += _BUF_SIZE;
    }
    // Set ring-end marker
    p -= ETHERC_RD_SIZE;
    _SU32(p, ETHERC_RD_STAT) |= ETHERC_RD_STAT_RDLE;
    cpd->rx_ring_next = 0;

    p = cpd->tx_ring;
    d = cpd->tx_buffers;
    for (i = 0; i < cpd->tx_ring_cnt; i++) {
        _SU32(p, ETHERC_TD_STAT) = ETHERC_TD_STAT_TFP_OTO;
        _SU16(p, ETHERC_TD_TDL) = 0;
        _SU16(p, ETHERC_TD_PAD0) = 0;
        _SU32(p, ETHERC_TD_TBA) = (cyg_uint32)d;
        _SU32(p, ETHERC_TD_PAD1) = 0;
        p += ETHERC_TD_SIZE;
        d += _BUF_SIZE;
    }
    // Set ring-end marker
    p -= ETHERC_RD_SIZE;
    _SU32(p, ETHERC_TD_STAT) |= ETHERC_TD_STAT_TDLE;
    cpd->tx_ring_free = cpd->tx_ring_alloc = cpd->tx_ring_owned = 0;

    // Reset ethernet module, then wait for (at least) 16 clocks.
    put_reg(cpd, _REG_EDMR, CYGARC_REG_EDMR_SWR | CYGARC_REG_EDMR_DL16);
    for (i = 0; i < 16; i++);
    put_reg(cpd, _REG_EDMR, CYGARC_REG_EDMR_DL16);

    // Program ring data into controller
    put_reg(cpd, _REG_RDLAR, (cyg_uint32)cpd->rx_ring);
    put_reg(cpd, _REG_TDLAR, (cyg_uint32)cpd->tx_ring);

    // Set ESA
    put_reg(cpd, _REG_MAHR, (cpd->esa[0] << 24) | (cpd->esa[1] << 16) | (cpd->esa[2] << 8) | cpd->esa[3]);
    put_reg(cpd, _REG_MALR, (cpd->esa[4] << 8) | cpd->esa[5]);

    // Set receive mode: receive continuously
    put_reg(cpd, _REG_RCR, CYGARC_REG_RCR_RNC);

    // Stop controller, set duplex mode
    put_reg(cpd, _REG_ECMR, CYGARC_REG_ECMR_DM);
    cpd->active = 0;

    // Clear pending interrupt flags
    reg = get_reg(cpd, _REG_EESR);
    put_reg(cpd, _REG_EESR, reg);

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    // Attach ISR/DSRs.
    cpd->interrupt = CYGNUM_HAL_INTERRUPT_EDMAC_EINT;
    cyg_drv_interrupt_create(cpd->interrupt,
                             1,                   // Priority
                             (cyg_addrword_t)cpd, // Data item passed to ISR & DSR
                             etherc_isr,          // ISR
                             etherc_dsr,          // DSR
                             &cpd->interrupt_handle, // handle to intr obj
                             &cpd->interrupt_object ); // space for int obj

    cyg_drv_interrupt_attach(cpd->interrupt_handle);
    cyg_drv_interrupt_unmask(cpd->interrupt);
    put_reg(cpd, _REG_EESIPR, CYGARC_REG_EESIPR_TCIP|CYGARC_REG_EESIPR_FRIP);
#if DEBUG & 8
    db_printf("Attached interrupt on vector %d\n", cpd->interrupt);
#endif
#endif

    // Record the net dev pointer
    cpd->ndp = (void *)tab;


    // Initialize the PHY
#ifdef CYGSEM_DEVS_ETH_SH_ETHERC_FORCE_10MBPS
#if DEBUG & 8
    db_printf("Forcing 10Mb link\n");
#endif
    _MII_SPEED_FORCE_10MB(cpd, 1);
    _MII_RENEGOTIATE(cpd, 1);
#else
    // Wait for automatic negotiation to complete
    _MII_RENEGOTIATION_WAIT(cpd, 1);
#endif

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, cpd->esa);

#if DEBUG & 9
    db_printf("Done\n");
#endif
    return true;
}

static void
etherc_suspend(struct etherc_priv_data *cpd)
{
    cyg_uint32 reg;
#if 0
    bool still_active;
#endif

    reg = get_reg(cpd, _REG_ECMR);
    reg &= ~(CYGARC_REG_ECMR_TE | CYGARC_REG_ECMR_RE);
    put_reg(cpd, _REG_ECMR, reg);

#if 0 // 
    // Try to find out if controller stopped. Supposedly, it should
    // communicate this by clearing the active signal of the active
    // RX/TX descriptors.

    // Check RX
    do {
        still_active = false;
        reg = get_reg(cpd, _REG_RDFAR);
#if 1
        {
            int i;
            cyg_uint8* p;
            p = cpd->rx_ring;
            for (i = 0; i < cpd->rx_ring_cnt; i++) {
                if ((cyg_uint32)p == reg) {
                    if (_SU32(reg, ETHERC_RD_STAT) & ETHERC_RD_STAT_RACT) {
                        still_active = true;
                        break;
                    }
                }
            }
        }
#else
        if (_SU32(reg, ETHERC_RD_STAT) & ETHERC_RD_STAT_RACT)
            still_active = true;
#endif
    } while (still_active);

    // Check TX
    do {
        still_active = false;
        reg = get_reg(cpd, _REG_TDFAR);
#if 1
        {
            int i;
            cyg_uint8* p;
            p = cpd->tx_ring;
            for (i = 0; i < cpd->tx_ring_cnt; i++) {
                if ((cyg_uint32)p == reg) {
                    if (_SU32(reg, ETHERC_TD_STAT) & ETHERC_TD_STAT_TACT) {
                        still_active = true;
                        break;
                    }
                }
            }
        }
#else
        if (_SU32(reg, ETHERC_TD_STAT) & ETHERC_TD_STAT_TACT)
            still_active = true;
#endif
    } while (still_active);

    db_printf("all done\n");
#endif
    cpd->active = 0;
}

static void
etherc_stop(struct eth_drv_sc *sc)
{
    struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();
    
    etherc_suspend(cpd);
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
etherc_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    cyg_uint32 reg, prm_mode = 0;
    struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif
    DEBUG_FUNCTION();

    // If device is already active, suspend it
    if (cpd->active) {
        etherc_suspend(cpd);
    }

#ifdef CYGPKG_NET
    if (( 0
#ifdef ETH_DRV_FLAGS_PROMISC_MODE
          != (flags & ETH_DRV_FLAGS_PROMISC_MODE)
#endif
            ) || (ifp->if_flags & IFF_PROMISC)
        ) {
        // Then we select promiscuous mode.
        prm_mode = CYGARC_REG_ECMR_PRM;
    }
#endif

    // Unsuspend the device
    reg = get_reg(cpd, _REG_ECMR);
    reg |= CYGARC_REG_ECMR_TE | CYGARC_REG_ECMR_RE;
    reg &= ~CYGARC_REG_ECMR_PRM;
    reg |= prm_mode;
    put_reg(cpd, _REG_ECMR, reg);

    put_reg(cpd, _REG_EDRRR, CYGARC_REG_EDRRR_RR);

    cpd->active = 1;
}

//
// This routine is called to perform special "control" opertions
//
static int
etherc_control(struct eth_drv_sc *sc, unsigned long key,
               void *data, int data_length)
{
    cyg_uint8 *esa = (cyg_uint8 *)data;
    int i, res;
    cyg_uint16 reg;
    struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;
    cyg_bool was_active = cpd->active;

    DEBUG_FUNCTION();

    // If device is already active, suspend it
    if (cpd->active) {
        etherc_suspend(cpd);
    }

    res = 0;                            // expect success
    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
#if 9 & DEBUG
        db_printf("ETHERC - set ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                esa[0], esa[1], esa[2], esa[3], esa[4], esa[5] );
#endif // DEBUG

        for ( i = 0; i < sizeof(cpd->esa);  i++ )
            cpd->esa[i] = esa[i];
        put_reg(cpd, _REG_MAHR,(cpd->esa[0] << 24) | (cpd->esa[1] << 16) | (cpd->esa[2] << 8) | cpd->esa[3]);
        put_reg(cpd, _REG_MALR, (cpd->esa[4] << 8) | cpd->esa[5]);
        break;

#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
        // Extract the MAC address that is in the chip, and tell the
        // system about it.
        cyg_uint32 reg;
        reg = get_reg(cpd, _REG_MAHR);
        cpd->esa[0] = (reg >> 24) & 0xff;
        cpd->esa[1] = (reg >> 16) & 0xff;
        cpd->esa[2] = (reg >> 08) & 0xff;
        cpd->esa[3] = (reg >> 00) & 0xff;
        reg = get_reg(cpd, _REG_MALR);
        cpd->esa[4] = (reg >> 08) & 0xff;
        cpd->esa[5] = (reg >> 00) & 0xff;
        break;
#endif

#ifdef ETH_DRV_GET_IF_STATS_UD
    case ETH_DRV_GET_IF_STATS_UD: // UD == UPDATE
#endif
        // drop through
#ifdef ETH_DRV_GET_IF_STATS
    case ETH_DRV_GET_IF_STATS:
#endif

#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
    {
        struct ether_drv_stats *p = (struct ether_drv_stats *)data;
        // Chipset entry is no longer supported; RFC1573.
        for ( i = 0; i < SNMP_CHIPSET_LEN; i++ )
            p->snmp_chipset[i] = 0;

        // This perhaps should be a config opt, so you can make up your own
        // description, or supply it from the instantiation.
        strcpy( p->description, "SH Etherc" );
        // CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

        if (_MII_LINK_STATE(cpd, 1)) {
            // Link is up
            p->operational = 3;         // LINK UP
            if (_MII_DUPLEX_STATE(cpd, 1))
                p->duplex = 3;              // 3 = DUPLEX
            else
                p->duplex = 2;              // 2 = SIMPLEX
            if (_MII_SPEED_STATE(cpd, 1))
                p->speed = 100 * 1000000;
            else
                p->speed = 10 * 1000000;
        } else {
            // Link is down
            p->operational = 2;         // LINK DOWN
            p->duplex = 1;              // UNKNOWN
            p->speed = 0;
        }
#if FIXME
#ifdef KEEP_STATISTICS
        {
            struct sh_etherc_stats *ps = &(cpd->stats);

            // Admit to it...
            p->supports_dot3        = true;

            p->tx_good              = ps->tx_good             ;
            p->tx_max_collisions    = ps->tx_max_collisions   ;
            p->tx_late_collisions   = ps->tx_late_collisions  ;
            p->tx_underrun          = ps->tx_underrun         ;
            p->tx_carrier_loss      = ps->tx_carrier_loss     ;
            p->tx_deferred          = ps->tx_deferred         ;
            p->tx_sqetesterrors     = ps->tx_sqetesterrors    ;
            p->tx_single_collisions = ps->tx_single_collisions;
            p->tx_mult_collisions   = ps->tx_mult_collisions  ;
            p->tx_total_collisions  = ps->tx_total_collisions ;
            p->rx_good              = ps->rx_good             ;
            p->rx_crc_errors        = ps->rx_crc_errors       ;
            p->rx_align_errors      = ps->rx_align_errors     ;
            p->rx_resource_errors   = ps->rx_resource_errors  ;
            p->rx_overrun_errors    = ps->rx_overrun_errors   ;
            p->rx_collisions        = ps->rx_collisions       ;
            p->rx_short_frames      = ps->rx_short_frames     ;
            p->rx_too_long_frames   = ps->rx_too_long_frames  ;
            p->rx_symbol_errors     = ps->rx_symbol_errors    ;
        
            p->interrupts           = ps->interrupts          ;
            p->rx_count             = ps->rx_count            ;
            p->rx_deliver           = ps->rx_deliver          ;
            p->rx_resource          = ps->rx_resource         ;
            p->rx_restart           = ps->rx_restart          ;
            p->tx_count             = ps->tx_count            ;
            p->tx_complete          = ps->tx_complete         ;
            p->tx_dropped           = ps->tx_dropped          ;
        }
#endif // KEEP_STATISTICS
#endif // FIXME

        p->tx_queue_len = 1;
        break;
    }
#endif
    default:
        res = 1;
        break;
    }

    // Restore controller state
    if (was_active) {
        // Unsuspend the device
        reg = get_reg(cpd, _REG_ECMR);
        reg |= CYGARC_REG_ECMR_RE | CYGARC_REG_ECMR_TE;
        put_reg(cpd, _REG_ECMR, reg);
    }

    return res;
}

//
// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
//
static int
etherc_can_send(struct eth_drv_sc *sc)
{
    struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    if (!_MII_LINK_STATE(cpd, 1))
        return 0;                       // Link not connected

    return (0 == cpd->txbusy);
}

//
// This routine is called to send data to the hardware.
static void 
etherc_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
            int total_len, unsigned long key)
{
    struct etherc_priv_data *cpd = 
        (struct etherc_priv_data *)sc->driver_private;
    int i, len, plen, ring_entry;

    cyg_uint8* sdata = NULL;
    cyg_uint8 *d, *buf, *txd;

    DEBUG_FUNCTION();

    INCR_STAT( tx_count );

    cpd->txbusy = 1;
    cpd->txkey = key;

    // Find packet length
    plen = 0;
    for (i = 0;  i < sg_len;  i++)
        plen += sg_list[i].len;

    CYG_ASSERT( plen == total_len, "sg data length mismatch" );

    // Get next TX descriptor
    ring_entry = cpd->tx_ring_free;
    do {
        if (cpd->tx_ring_owned == cpd->tx_ring_cnt) {
            // Is this a dead end? Probably is.
#if DEBUG & 1
            db_printf("%s: Allocation failed! Retrying...\n", __FUNCTION__ );
#endif
            continue;
        }

        cpd->tx_ring_free++;
        cpd->tx_ring_owned++;
        if (cpd->tx_ring_free == cpd->tx_ring_cnt)
            cpd->tx_ring_free = 0;
    } while (0);

    txd = cpd->tx_ring + ring_entry*ETHERC_TD_SIZE;
    buf = cpd->tx_buffers + ring_entry*_BUF_SIZE;
    CYG_ASSERT(0 == (_SU32(txd, ETHERC_TD_STAT) & ETHERC_TD_STAT_TACT),
               "TX descriptor not free");

#if DEBUG & 4
    db_printf("#####Tx descriptor 0x%08x buffer 0x%08x\n",
                txd, buf);
#endif

    // Put data into buffer
    d = buf;
    for (i = 0;  i < sg_len;  i++) {
        sdata = (cyg_uint8 *)sg_list[i].buf;
        len = sg_list[i].len;

        CYG_ASSERT( sdata, "No sg data pointer here" );
        while(len--)
            *d++ = *sdata++;
    }
    CYG_ASSERT( sdata, "No sg data pointer outside" );

    if (plen < IEEE_8023_MIN_FRAME) {
#if DEBUG & 1
        db_printf("Packet too short (%d) - padding to %d bytes.\n", plen, IEEE_8023_MIN_FRAME);
#endif
        for (i = plen; i < IEEE_8023_MIN_FRAME; i++)
            *d++ = 0;
        plen = IEEE_8023_MIN_FRAME;
    }

    _SU16(txd, ETHERC_TD_TDL) = plen;
    _SU32(txd, ETHERC_TD_STAT) |= ETHERC_TD_STAT_TACT;

#if DEBUG & 1
    db_printf("Last TX: LEN %04x STAT %08x PTR %08x\n", 
              _SU16(txd, ETHERC_TD_TDL),
              _SU32(txd, ETHERC_TD_STAT),
              _SU32(txd, ETHERC_TD_TBA));
#endif

    // Set transmit demand
    put_reg(cpd, _REG_EDTRR, CYGARC_REG_EDTRR_TR);

#if DEBUG & 1
    {
        cyg_uint32 reg;
        reg = get_reg(cpd, _REG_EESR);
        db_printf("%s:END: EESR at TX: 0x%08x\n", __FUNCTION__, reg);
    }
#endif
}

static void
etherc_TxEvent(struct eth_drv_sc *sc, int stat)
{
     struct etherc_priv_data *cpd =
        (struct etherc_priv_data *)sc->driver_private;
    int success = 1;
    cyg_uint8 *txd;
    cyg_uint32 pkt_stat;

    DEBUG_FUNCTION();

    if (0 == cpd->tx_ring_owned) {
#if DEBUG & 1
        db_printf("%s: got TX completion when no outstanding packets\n", __FUNCTION__);
#endif
        // Ack the TX int
        put_reg(cpd, _REG_EESR, CYGARC_REG_EESR_TC);
        return;
    }
        

    INCR_STAT( tx_complete );

    txd = cpd->tx_ring + cpd->tx_ring_alloc*ETHERC_TD_SIZE;
    pkt_stat = _SU32(txd, ETHERC_TD_STAT);
    if (pkt_stat & ETHERC_TD_STAT_TACT) {
#if DEBUG & 1
        db_printf("%s: got TX completion when buffer is still owned\n", __FUNCTION__);
#endif
        // first dirty ring entry not freed - wtf?
    }

    if (pkt_stat & ETHERC_TD_STAT_TDFE) {
        // We had an error. Tell the stack.
        success = 0;
#if DEBUG & 1
        db_printf("%s: TX failure, retrying...\n", __FUNCTION__);
#endif
    }

    cpd->tx_ring_alloc++;
    if (cpd->tx_ring_alloc == cpd->tx_ring_cnt)
        cpd->tx_ring_alloc = 0;
    cpd->tx_ring_owned--;

#if FIXME
#ifdef KEEP_STATISTICS
    {
        cyg_uint16 reg;

        reg = get_reg( sc, ETHERC_CSR_CSCR );
        
        // Covering each bit in turn...
        if ( reg & ETHERC_STATUS_TX_UNRN   ) INCR_STAT( tx_underrun );
        //if ( reg & ETHERC_STATUS_LINK_OK ) INCR_STAT(  );
        //if ( reg & ETHERC_STATUS_CTR_ROL ) INCR_STAT(  );
        //if ( reg & ETHERC_STATUS_EXC_DEF ) INCR_STAT(  );
        if ( reg & ETHERC_STATUS_LOST_CARR ) INCR_STAT( tx_carrier_loss );
        if ( reg & ETHERC_STATUS_LATCOL    ) INCR_STAT( tx_late_collisions );
        //if ( reg & ETHERC_STATUS_WAKEUP  ) INCR_STAT(  );
        if ( reg & ETHERC_STATUS_TX_DEFR   ) INCR_STAT( tx_deferred );
        //if ( reg & ETHERC_STATUS_LTX_BRD ) INCR_STAT(  );
        if ( reg & ETHERC_STATUS_SQET      ) INCR_STAT( tx_sqetesterrors );
        if ( reg & ETHERC_STATUS_16COL     ) INCR_STAT( tx_max_collisions );
        //if ( reg & ETHERC_STATUS_LTX_MULT) INCR_STAT(  );
        if ( reg & ETHERC_STATUS_MUL_COL   ) INCR_STAT( tx_mult_collisions );
        if ( reg & ETHERC_STATUS_SNGL_COL  ) INCR_STAT( tx_single_collisions );
        if ( reg & ETHERC_STATUS_TX_SUC    ) INCR_STAT( tx_good );

        cpd->stats.tx_total_collisions = 
            cpd->stats.tx_late_collisions + 
            cpd->stats.tx_max_collisions + 
            cpd->stats.tx_mult_collisions + 
            cpd->stats.tx_single_collisions;

        // We do not need to look in the Counter Register (ETHERC_COUNTER)
        // because it just mimics the info we already have above.
    }
#endif // KEEP_STATISTICS
#endif // FIXME

    // Ack the TX int
    put_reg(cpd, _REG_EESR, CYGARC_REG_EESR_TC);

#if DEBUG & 4
    db_printf("#####Tx packet freed 0x%08x\n", txd );
#endif

    if ( cpd->txbusy ) {
        cpd->txbusy = 0;
        (sc->funs->eth_drv->tx_done)(sc, cpd->txkey, success);
    }
}


//
// This function is called when a packet has been received.  Its job is
// to prepare to unload the packet from the hardware.  Once the length of
// the packet is known, the upper layer of the driver can be told.  When
// the upper layer is ready to unload the packet, the internal function
// 'etherc_recv' will be called to actually fetch it from the hardware.
//
static void
etherc_RxEvent(struct eth_drv_sc *sc)
{
    struct etherc_priv_data *cpd = 
        (struct etherc_priv_data *)sc->driver_private;
    cyg_uint8 *rxd;
    cyg_uint32 rstat;
    cyg_uint32 ints;
    cyg_uint16 len;

    DEBUG_FUNCTION();

    ints = get_reg(cpd, _REG_EESR);
#if DEBUG & 1
    db_printf("RxEvent - ESSR: 0x%08x\n", ints);
#endif

    while (1) {
        // Get state of next (supposedly) full ring entry
        cpd->rxpacket = cpd->rx_ring_next;
        rxd = cpd->rx_ring + cpd->rxpacket*ETHERC_RD_SIZE;
        rstat = _SU32(rxd, ETHERC_RD_STAT);

        // Keep going until we hit an entry that is owned by the
        // controller.
        if (rstat & ETHERC_RD_STAT_RACT) {
#if DEBUG & 1
            int i;
            for (i = 0; i < cpd->rx_ring_cnt; i++) {
                rxd = cpd->rx_ring + i*ETHERC_RD_SIZE;
                rstat = _SU32(rxd, ETHERC_RD_STAT);

                if (!(rstat & ETHERC_RD_STAT_RACT)) {
                    int i;
                    cyg_uint32 rstat;
                    cyg_uint16 mlen, blen;
                    cyg_uint8* rxd;

                    db_printf("%s: Inconsistent RX state for %d\n", __FUNCTION__, cpd->rxpacket);
                    for (i = 0; i < cpd->rx_ring_cnt; i++) {
                        rxd = cpd->rx_ring + i*ETHERC_RD_SIZE;
                
                        rstat = _SU32(rxd, ETHERC_RD_STAT);
                        blen = _SU16(rxd, ETHERC_RD_RBL);
                        mlen = _SU16(rxd, ETHERC_RD_RDL);
                        db_printf(" %02d: 0x%08x:0x%04x:0x%04x\n", i, rstat, blen, mlen);
                    }
                }
            }
#endif
            break;
        }

#if DEBUG & 4
        db_printf("#####Rx packet at index %d\n", cpd->rxpacket);
#endif

        // Increment counts
        INCR_STAT( rx_count );
        cpd->rx_ring_next++;
        if (cpd->rx_ring_next == cpd->rx_ring_cnt) cpd->rx_ring_next = 0;

        len = _SU16(rxd, ETHERC_RD_RDL);

#ifdef KEEP_STATISTICS
        //if ( rstat & ETHERC_RD_PTR_FRAM ) INCR_STAT( rx_frame_errors );
        //if ( rstat & ETHERC_RD_PTR_OFLO ) INCR_STAT(  );
        if ( rstat & ETHERC_RD_STAT_RFOF ) INCR_STAT( rx_crc_errors );
        //if ( rstat & ETHERC_RD_PTR_BUFF ) INCR_STAT(  );
#endif // KEEP_STATISTICS

        if (0 == (rstat & ETHERC_RD_STAT_RFE)) {
            // It's OK
            INCR_STAT( rx_good );

#if DEBUG & 1
            db_printf("RxEvent good rx - stat: 0x%08x, len: 0x%04x\n", rstat, len);
#endif
            // Check for bogusly short packets; can happen in promisc
            // mode: Asserted against and checked by upper layer
            // driver.
#ifdef CYGPKG_NET
            if ( len > sizeof( struct ether_header ) )
                // then it is acceptable; offer the data to the network stack
#endif
                (sc->funs->eth_drv->recv)(sc, len);
        } else {
            // Not OK for one reason or another...
#if DEBUG & 1
            db_printf("RxEvent - No RX bit: stat: 0x%08x, len: 0x%04x\n",
                        rstat, len);
#endif
        }

        // Free packet (clear all status flags, and set RACT)
        _SU32(rxd, ETHERC_RD_STAT) &= ETHERC_RD_STAT_CLEAR;
        _SU32(rxd, ETHERC_RD_STAT) |= ETHERC_RD_STAT_RACT;
    }

    // Ack RX interrupt set
    put_reg(cpd, _REG_EESR, CYGARC_REG_EESR_FR);
    // ensure Receive Request is enabled
    put_reg(cpd, _REG_EDRRR, CYGARC_REG_EDRRR_RR);
#if DEBUG & 1
    ints = get_reg(cpd, _REG_EESR);
    db_printf("RxEvent exit - ESSR: 0x%08x\n", ints);
#endif
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// Its job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
etherc_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    struct etherc_priv_data *cpd = 
        (struct etherc_priv_data *)sc->driver_private;
    int i, mlen=0, plen;
    cyg_uint8 *data, *rxd, *buf;

    DEBUG_FUNCTION();

    rxd = cpd->rx_ring + cpd->rxpacket*ETHERC_TD_SIZE;
    buf = cpd->rx_buffers + cpd->rxpacket*_BUF_SIZE;

    INCR_STAT( rx_deliver );

    plen = _SU16(rxd, ETHERC_RD_RDL);

    for (i = 0;  i < sg_len;  i++) {
        data = (cyg_uint8*)sg_list[i].buf;
        mlen = sg_list[i].len;

#if DEBUG & 1
        db_printf("%s : mlen %04x, plen %04x\n", __FUNCTION__, mlen, plen);
#endif
        if (data) {
            while (mlen > 0) {
                *data++ = *buf++;
                mlen--;
                plen--;
            }
        }
    }
}

static void
etherc_poll(struct eth_drv_sc *sc)
{
    cyg_uint32 event;
    struct etherc_priv_data *cpd = 
        (struct etherc_priv_data *)sc->driver_private;

//    DEBUG_FUNCTION();

    while (1) {
        // Get the (unmasked) requests
        event = get_reg(cpd, _REG_EESR);
        if (0 == event)
            break;

        if (event & CYGARC_REG_EESR_FR) {
            etherc_RxEvent(sc);
        }
        else if (event & CYGARC_REG_EESR_TC) {
            etherc_TxEvent(sc, event);
        } 
        else if (event & CYGARC_REG_EESR_RDE) {
#if DEBUG & 1
            int i;
            cyg_uint32 rstat;
            cyg_uint16 mlen, blen;
            cyg_uint8* rxd;
            struct etherc_priv_data *cpd = 
                (struct etherc_priv_data *)sc->driver_private;

            db_printf("%s: Ran out of RX buffers (%04x)\n", __FUNCTION__, event);
            for (i = 0; i < cpd->rx_ring_cnt; i++) {
                rxd = cpd->rx_ring + i*ETHERC_RD_SIZE;
                
                rstat = _SU32(rxd, ETHERC_RD_STAT);
                blen = _SU16(rxd, ETHERC_RD_RBL);
                mlen = _SU16(rxd, ETHERC_RD_RDL);
                db_printf(" %02d: 0x%08x:0x%04x:0x%04x\n", i, rstat, blen, mlen);
            }
#endif
            // Just clear the flag - RF is handled earlier in the loop
            // so a new RX block should be ready when this code
            // executes.
            put_reg(cpd, _REG_EESR, CYGARC_REG_EESR_RDE);
        }
        else {
#if DEBUG & 1
            db_printf("%s: Unknown interrupt: 0x%04x\n", __FUNCTION__, event);
#endif
            put_reg(cpd, _REG_EESR, event);
        }
    }

    // Make sure RX is enabled
    if (cpd->active) {
        cyg_uint32 reg;
        reg = get_reg(cpd, _REG_ECMR);
        reg |= CYGARC_REG_ECMR_RE;
        put_reg(cpd, _REG_ECMR, reg);
    }
}

//----------------------------------------------------------------------------
// MII accessors

#define _MII_WRITE_BIT(_cpd_, _b_)                                                              \
  CYG_MACRO_START                                                                               \
  cyg_uint32 _d_ = (_b_) ? CYGARC_REG_PIR_MDO : 0;                                              \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_WRITE | _d_);                       \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_WRITE | _d_ | CYGARC_REG_PIR_MDC);  \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_WRITE | _d_);                       \
  CYG_MACRO_END

#define _MII_READ_BIT(_cpd_, _b_)                                                       \
  CYG_MACRO_START                                                                       \
  cyg_uint32 _d_;                                                                       \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_READ | CYGARC_REG_PIR_MDC); \
  HAL_READ_UINT32(_cpd_->base+_REG_PIR, _d_);                                           \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_READ);                      \
  (_b_) = (_d_ & CYGARC_REG_PIR_MDI) ? 1 : 0;                                           \
  CYG_MACRO_END

#define _MII_RELEASE(_cpd_)                                                             \
  CYG_MACRO_START                                                                       \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_READ);                      \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_READ | CYGARC_REG_PIR_MDC); \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_READ);                      \
  CYG_MACRO_END

#define _MII_RELEASE_INDEP(_cpd_)                                       \
  CYG_MACRO_START                                                       \
  HAL_WRITE_UINT32(_cpd_->base+_REG_PIR, CYGARC_REG_PIR_MMD_READ);      \
  CYG_MACRO_END


static void
etherc_write_MII(struct etherc_priv_data *cpd, int id, int reg, cyg_uint16 value)
{
    int i;

    // Pre
    for(i = 0; i < 32; i++)
        _MII_WRITE_BIT(cpd, 1);
    // Start of frame
    _MII_WRITE_BIT(cpd, 0);
    _MII_WRITE_BIT(cpd, 1);
    // Operation (write)
    _MII_WRITE_BIT(cpd, 0);
    _MII_WRITE_BIT(cpd, 1);
    // Phy address
    for (i = 4; i >= 0; i--)
        _MII_WRITE_BIT(cpd, (id & (1<<i)) ? 1: 0);
    // Register address
    for (i = 4; i >= 0; i--)
        _MII_WRITE_BIT(cpd, (reg & (1<<i)) ? 1: 0);
    // TA
    _MII_WRITE_BIT(cpd, 1);
    _MII_WRITE_BIT(cpd, 0);
    // Data
    for (i = 15; i >= 0; i--)
        _MII_WRITE_BIT(cpd, (value & (1<<i)) ? 1: 0);
    // Release bus
    _MII_RELEASE_INDEP(cpd);
}

static cyg_uint16
etherc_read_MII(struct etherc_priv_data *cpd, int id, int reg)
{
    bool b;
    int i;
    cyg_uint16 val = 0;

    // Pre
    for(i = 0; i < 32; i++)
        _MII_WRITE_BIT(cpd, 1);
    // Start of frame
    _MII_WRITE_BIT(cpd, 0);
    _MII_WRITE_BIT(cpd, 1);
    // Operation (read)
    _MII_WRITE_BIT(cpd, 1);
    _MII_WRITE_BIT(cpd, 0);
    // Phy address
    for (i = 4; i >= 0; i--)
        _MII_WRITE_BIT(cpd, (id & (1<<i)) ? 1: 0);
    // Register address
    for (i = 4; i >= 0; i--)
        _MII_WRITE_BIT(cpd, (reg & (1<<i)) ? 1: 0);
    // TA
    _MII_RELEASE(cpd);
    // Data
    for (i = 15; i >= 0; i--) {
        _MII_READ_BIT(cpd, b);
        val = val << 1;
        if (b) val |= 1;
    }

    return val;
}

// EOF if_etherc.c
