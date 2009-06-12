//==========================================================================
//
//      dev/if_sc_lpe.c
//
//      Ethernet device driver for Socket Communications Compact Flash card
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
// Contributors: gthomas, jskov
// Date:         2000-07-07
// Purpose:      
// Description:  hardware driver for LPCF+ ethernet
//              
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
#include <cyg/io/pcmcia.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/netdev.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#else
#include <cyg/hal/hal_if.h>
#endif

#include <cyg/io/dp83902a.h>

#define DP_DATA         0x10
#define DP_CARD_RESET   0x1f

#define SC_LPE_MANUF 0x0104

#ifdef CYGPKG_KERNEL
#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
static char sc_lpe_card_handler_stack[STACK_SIZE];
static cyg_thread sc_lpe_card_handler_thread_data;
static cyg_handle_t sc_lpe_card_handler_thread_handle;
#endif  // CYGPKG_KERNEL

__inline__ static void
do_delay(int _ticks)
{
#ifdef CYGPKG_KERNEL
    cyg_thread_delay(_ticks);
#else
    CYGACC_CALL_IF_DELAY_US(10000*_ticks);
#endif
}

//
// This runs as a separate thread to handle the card.  In particular, insertions
// and deletions need to be handled and they take time/coordination, thus the
// separate thread.
//
#ifdef CYGPKG_KERNEL
static void
#else
static int
#endif
sc_lpe_card_handler(cyg_addrword_t param)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)param;
    dp83902a_priv_data_t *dp = (dp83902a_priv_data_t*)sc->driver_private;
    struct cf_slot *slot;
    struct cf_cftable cftable;
    struct cf_config config;
    int i, len, ptr, cor = 0;
    unsigned char buf[256], *cp;
    cyg_uint8* base;
    unsigned char *vers_product, *vers_manuf, *vers_revision, *vers_date;
#ifndef CYGPKG_KERNEL
    int tries = 0;
#endif
    bool first = true;

    slot = (struct cf_slot*)dp->plf_priv;
    cyg_drv_dsr_lock();
    while (true) {
        cyg_drv_dsr_unlock();   // Give DSRs a chance to run (card insertion)
        cyg_drv_dsr_lock();
        if ((slot->state == CF_SLOT_STATE_Inserted) ||
            ((slot->state == CF_SLOT_STATE_Ready) && first)) {
            first = false;
            if (slot->state != CF_SLOT_STATE_Ready) {
                cf_change_state(slot, CF_SLOT_STATE_Ready);
            }
            if (slot->state != CF_SLOT_STATE_Ready) {
                diag_printf("CF card won't go ready!\n");
#ifndef CYGPKG_KERNEL
                return false;
#else
                continue;
#endif
            }
            len = sizeof(buf);
            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_MANFID, buf, &len, &ptr)) {
                if (*(short *)&buf[2] != SC_LPE_MANUF) {
                    diag_printf("Not a SC LPE, sorry\n");
                    continue;
                }
            } 
            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_VERS_1, buf, &len, &ptr)) {
                // Find individual strings
                cp = &buf[4];
                vers_product = cp;
                while (*cp++) ;  // Skip to nul
                vers_manuf = cp;
                while (*cp++) ;  // Skip to nul
                vers_revision = cp;
                while (*cp++) ;  // Skip to nul
                vers_date = cp;
#ifndef CYGPKG_KERNEL
                if (tries != 0) diag_printf("\n");
                diag_printf("%s: %s %s %s\n", vers_manuf, vers_product, vers_revision, vers_date);
#endif
            }
            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_CONFIG, buf, &len, &ptr)) {
                if (cf_parse_config(buf, len, &config)) {
                    cor = config.base;
                }
            }
            if (!cor) {
//                diag_printf("Couldn't find COR pointer!\n");
                continue;
            }

            ptr = 0;
            if (cf_get_CIS(slot, CF_CISTPL_CFTABLE_ENTRY, buf, &len, &ptr)) {
                if (cf_parse_cftable(buf, len, &cftable)) {
                    cyg_uint8 tmp;
                    // Initialize dp83902a IO details
                    dp->base = base = (cyg_uint8*)&slot->io[cftable.io_space.base[0]];
                    dp->data = base + DP_DATA;
                    dp->interrupt = slot->int_num;
                    cf_set_COR(slot, cor, cftable.cor);
                    // Reset card  (read issues RESET, write clears it)
                    HAL_READ_UINT8(base+DP_CARD_RESET, tmp);
                    HAL_WRITE_UINT8(base+DP_CARD_RESET, tmp);
                    // Wait for card
                    do {
                        DP_IN(base, DP_ISR, tmp);
                    } while (0 == (tmp & DP_ISR_RESET));

                    // Fetch hardware address from card - terrible, but not well defined
                    // Patterned after what Linux drivers do
                    if (!dp->hardwired_esa) {
                        static unsigned char sc_lpe_addr[] = { 0x00, 0xC0, 0x1B, 0x00, 0x99, 0x9E};
                        if ((slot->attr[0x1C0] == sc_lpe_addr[0]) &&
                            (slot->attr[0x1C2] == sc_lpe_addr[1]) &&
                            (slot->attr[0x1C4] == sc_lpe_addr[2])) {
                            sc_lpe_addr[3] = slot->attr[0x1C6];
                            sc_lpe_addr[4] = slot->attr[0x1C8];
                            sc_lpe_addr[5] = slot->attr[0x1CA];
                        } else {
                            // Coudn't find it in the CIS (attribute) data
                            unsigned char prom[32];

                            // Tell device to give up ESA
                            DP_OUT(base, DP_DCR, 0x48);  // Bytewide access
                            DP_OUT(base, DP_RBCH, 0);    // Remote byte count
                            DP_OUT(base, DP_RBCL, 0);
                            DP_OUT(base, DP_ISR, 0xFF);  // Clear any pending interrupts
                            DP_OUT(base, DP_IMR, 0x00);  // Mask all interrupts 
                            DP_OUT(base, DP_RCR, 0x20);  // Monitor
                            DP_OUT(base, DP_TCR, 0x02);  // loopback
                            DP_OUT(base, DP_RBCH, 32);   // Remote byte count
                            DP_OUT(base, DP_RBCL, 0);
                            DP_OUT(base, DP_RSAL, 0);    // Remote address
                            DP_OUT(base, DP_RSAH, 0);
                            DP_OUT(base, DP_CR, DP_CR_START|DP_CR_RDMA);  // Read data
                            for (i = 0;  i < 32;  i++) {
                                HAL_READ_UINT8(base+DP_DATAPORT, prom[i]);
                            }
                            if ((prom[0] == sc_lpe_addr[0]) &&
                                (prom[2] == sc_lpe_addr[1]) &&
                                (prom[4] == sc_lpe_addr[2])) {
                                diag_printf("Getting address from port\n");
                                sc_lpe_addr[3] = prom[6];
                                sc_lpe_addr[4] = prom[8];
                                sc_lpe_addr[5] = prom[10];
                            } else {
                                diag_printf("No valid ESA found in CIS! Hardwiring to 00:C0:1B:00:99:9E\n");
                            }
                        }
                        for (i = 0;  i < 6;  i++) {
                            dp->esa[i] = sc_lpe_addr[i];
                        }
                    }

                    // Initialize upper level driver
                    (sc->funs->eth_drv->init)(sc, dp->esa);
                    // Tell system card is ready to talk
                    dp->tab->status = CYG_NETDEVTAB_STATUS_AVAIL;
#ifndef CYGPKG_KERNEL
                    cyg_drv_dsr_unlock();
                    return true;
#endif
                } else {
                    diag_printf("Can't parse CIS\n");
                    continue;
                }
            } else {
                diag_printf("Can't fetch config info\n");
                continue;
            }
        } else if (slot->state == CF_SLOT_STATE_Removed) {
            diag_printf("Compact Flash card removed!\n");
        } else {
            cyg_drv_dsr_unlock();
            do_delay(50);  // FIXME!
#ifndef CYGPKG_KERNEL
            if (tries == 0) diag_printf("... Waiting for network card: ");
            diag_printf(".");
            if (++tries == 10) {
                // 5 seconds have elapsed - give up
                return false;
            }
            cf_hwr_poll(slot);  // Check to see if card has been inserted
#endif
            cyg_drv_dsr_lock();
        }
    }
}

bool 
cyg_sc_lpe_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    dp83902a_priv_data_t *dp = (dp83902a_priv_data_t *)sc->driver_private;
    struct cf_slot* slot;

    cf_init();  // Make sure Compact Flash subsystem is initialized
    slot = dp->plf_priv = (void*)cf_get_slot(0);
    dp->tab = tab;

#ifdef CYGPKG_KERNEL
    // Create card handling [background] thread
    cyg_thread_create(CYGPKG_NET_THREAD_PRIORITY-1,          // Priority
                      sc_lpe_card_handler,                   // entry
                      (cyg_addrword_t)sc,                    // entry parameter
                      "SC LP-E card support",                // Name
                      &sc_lpe_card_handler_stack[0],         // Stack
                      STACK_SIZE,                            // Size
                      &sc_lpe_card_handler_thread_handle,    // Handle
                      &sc_lpe_card_handler_thread_data       // Thread data structure
            );
    cyg_thread_resume(sc_lpe_card_handler_thread_handle);    // Start it

    // Initialize environment, setup interrupt handler
    // eth_drv_dsr is used to tell the fast net thread to run the deliver funcion.
    cf_register_handler(slot, eth_drv_dsr, sc);

    return false;  // Device is not ready until inserted, powered up, etc.
#else
    // Initialize card
    return sc_lpe_card_handler((cyg_addrword_t)sc);
#endif
}

int
cyg_sc_lpe_int_vector(struct eth_drv_sc *sc)
{
    dp83902a_priv_data_t *dp = (dp83902a_priv_data_t *)sc->driver_private;
    struct cf_slot* slot = (struct cf_slot*)dp->plf_priv;

    return slot->int_num;
}
