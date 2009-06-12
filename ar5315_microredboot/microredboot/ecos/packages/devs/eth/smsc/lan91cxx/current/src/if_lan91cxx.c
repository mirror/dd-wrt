//==========================================================================
//
//      dev/if_lan91cxx.c
//
//      Ethernet device driver for SMSC LAN91CXX compatible controllers
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
// Copyright (C) 2004 Andrew Lunn
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
// Author(s):    hmt, based on lan900 (for LAN91C110) driver by jskov
//               jskov, based on CS8900 driver by Gary Thomas
// Contributors: gthomas, jskov, hmt, jco@ict.es, nickg
// Date:         2001-01-22
// Purpose:      
// Description:  hardware driver for LAN91CXX "LAN9000" ethernet
// Notes:        Pointer register is not saved/restored on receive interrupts.
//               The pointer is shared by both receive/transmit code.
//               But the net stack manages atomicity for you here.
//
//               The controller has an autorelease mode that allows TX packets
//               to be freed automatically on successful transmission - but
//               that is not used since we're only sending one packet at a
//               time anyway.
//               We may want to pingpong in future for throughput reasons.
//
//               <jco@ict.es> Added support for PCMCIA mode and shifted
//               address buses.
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Based on LAN91C110 and LAN91C96

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_smsc_lan91cxx.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_INFRA_DEBUG
// Then we log, OOI, the number of times we get a bad packet number
// from the tx done fifo.
int lan91cxx_txfifo_good = 0;
int lan91cxx_txfifo_bad = 0;
#endif

// Set to perms of:
// 0 disables all debug output
// 1 for process debug output
// 2 for added data IO output: get_reg, put_reg
// 4 for packet allocation/free output
// 8 for only startup status, so we can tell we're installed OK
#define DEBUG 0


#if DEBUG
#if defined(CYGPKG_REDBOOT)
static void db_printf( char *fmt, ... )
{
    extern int start_console(void);
    extern void end_console(int);
    va_list a;
    int old_console;
    va_start( a, fmt );
    old_console = start_console();  
    diag_vprintf( fmt, a );
    end_console(old_console);
    va_end( a );
}
#else
#define db_printf diag_printf
#endif
#else
#define db_printf( fmt, ... )
#endif


#if DEBUG & 1
#define DEBUG_FUNCTION() do { db_printf("%s\n", __FUNCTION__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#endif

#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
#define KEEP_STATISTICS
#endif

#ifdef KEEP_STATISTICS
#define INCR_STAT( _x_ )        (cpd->stats. _x_ ++)
#else
#define INCR_STAT( _x_ )        CYG_EMPTY_STATEMENT
#endif

#include "smsc_lan91cxx.h"

#ifdef LAN91CXX_IS_LAN91C111
static void lan91cxx_write_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr,
			       cyg_uint8 phyreg, cyg_uint16 value);
static cyg_uint16 lan91cxx_read_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr,
				    cyg_uint8 phyreg);
#endif

static void lan91cxx_poll(struct eth_drv_sc *sc);


#ifdef LAN91CXX_IS_LAN91C111
// Revision A of the LAN91C111 has a bug in which it does not set the
// ODD bit in the status word and control byte of received packets. We
// work around this by assuming the bit is always set and tacking the
// extra odd byte onto the packet anyway. Higher protocol levels never
// believe the packet size reported by the driver and always use the
// values in the protocol headers. So this workaround is quite safe.
// In theory nobody should be using the RevA part now, but it appears
// that some people still have some in their parts bins.
#define LAN91CXX_RX_STATUS_IS_ODD(__cpd,__stat) \
        (((__cpd)->c111_reva)?1:((__stat) & LAN91CXX_RX_STATUS_ODDFRM))
#define LAN91CXX_CONTROLBYTE_IS_ODD(__cpd,__val) \
        (((__cpd)->c111_reva)?1:((__val) & LAN91CXX_CONTROLBYTE_ODD))
#else
#define LAN91CXX_RX_STATUS_IS_ODD(__cpd,__stat) ((__stat) & LAN91CXX_RX_STATUS_ODDFRM)
#define LAN91CXX_CONTROLBYTE_IS_ODD(__cpd,__val) ((__val) & LAN91CXX_CONTROLBYTE_ODD)
#endif

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
static cyg_interrupt lan91cxx_interrupt;
static cyg_handle_t  lan91cxx_interrupt_handle;

// This ISR is called when the ethernet interrupt occurs
static int lan91cxx_isr(cyg_vector_t vector, cyg_addrword_t data)
             /* , HAL_SavedRegisters *regs */
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    INCR_STAT( interrupts );

    cyg_drv_interrupt_mask(cpd->interrupt);
    cyg_drv_interrupt_acknowledge(cpd->interrupt);

    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}
#endif

// The deliver function (ex-DSR)  handles the ethernet [logical] processing
static void
lan91cxx_deliver(struct eth_drv_sc *sc)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    // Service the interrupt:
    lan91cxx_poll(sc);
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(cpd->interrupt);
}

static int
lan91cxx_int_vector(struct eth_drv_sc *sc)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;

    return (cpd->interrupt);
}

static bool 
smsc_lan91cxx_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    unsigned short val;
    int i;
#if CYGINT_DEVS_ETH_SMSC_LAN91CXX_PCMCIA_MODE
    unsigned char ecor, ecsr;
#endif
    cyg_bool esa_configured = false;

    DEBUG_FUNCTION();

    cpd->txbusy = cpd->within_send = 0;
        
#ifdef CYGNUM_DEVS_ETH_SMSC_LAN91CXX_SHIFT_ADDR
    cpd->addrsh = CYGNUM_DEVS_ETH_SMSC_LAN91CXX_SHIFT_ADDR;
#else
    cpd->addrsh = 0;
#endif

#if CYGINT_DEVS_ETH_SMSC_LAN91CXX_PCMCIA_MODE

    // If the chip is configured in PCMCIA mode, the internal
    // registers mapped in the attribute memory should be
    // initialized (i.e. to enable the I/O map)
    
    ecor = get_att(sc, LAN91CXX_ECOR);
    
    // pulse SRESET on ECOR
    ecor |= LAN91CXX_ECOR_RESET;
    put_att(sc, LAN91CXX_ECOR, ecor);
    
    HAL_DELAY_US(1);
    
    ecor &= ~LAN91CXX_ECOR_RESET;
    put_att(sc, LAN91CXX_ECOR, ecor);

    // then, enable I/O map
    ecor |= LAN91CXX_ECOR_ENABLE;    
    put_att(sc, LAN91CXX_ECOR, ecor);

    // verify the register contents
    if (ecor != get_att(sc, LAN91CXX_ECOR))
        db_printf("LAN91CXX - Cannot access PCMCIA attribute registers\n");
	
    ecsr = get_att(sc, LAN91CXX_ECSR);
#ifdef CYGSEM_DEVS_ETH_SMSC_LAN91CXX_8_BIT
#error "91CXX 8-bit mode not yet supported."
    ecsr |= LAN91CXX_ECSR_IOIS8;
#else
    ecsr &= ~LAN91CXX_ECSR_IOIS8;
#endif
    put_att(sc, LAN91CXX_ECSR, ecsr);
        
#endif

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    // Initialize environment, setup interrupt handler
    cyg_drv_interrupt_create(cpd->interrupt,
                             CYGNUM_DEVS_ETH_SMSC_LAN91CXX_INT_PRIO,
                             (cyg_addrword_t)sc, //  Data item passed to interrupt handler
                             (cyg_ISR_t *)lan91cxx_isr,
                             (cyg_DSR_t *)eth_drv_dsr, // The logical driver DSR
                             &lan91cxx_interrupt_handle,
                             &lan91cxx_interrupt);
    cyg_drv_interrupt_attach(lan91cxx_interrupt_handle);
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    cyg_drv_interrupt_acknowledge(cpd->interrupt);
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    cyg_drv_interrupt_unmask(cpd->interrupt);
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
    
    // probe chip by reading the signature in BS register
    val = get_banksel(sc);
#if DEBUG & 9
    db_printf("LAN91CXX - supposed BankReg @ %x = %04x\n",
                cpd->base+LAN91CXX_BS, val );
#endif

    if ((0xff00 & val) !=  0x3300) {
        CYG_FAIL("No 91Cxx signature" );
        diag_printf("smsc_lan91cxx_init: No 91Cxx signature found\n");
        return false;
    }

    val = get_reg(sc, LAN91CXX_REVISION);

#if DEBUG & 9
    db_printf("LAN91CXX - type: %01x, rev: %01x\n",
                (val>>4)&0xf, val & 0xf);
#endif

#ifdef LAN91CXX_IS_LAN91C111
    // Set RevA flag for LAN91C111 so we can cope with the odd-bit bug.
    cpd->c111_reva = (val == 0x3390);
#endif
    
    // The controller may provide a function used to set up the ESA
    if (cpd->config_enaddr)
        (*cpd->config_enaddr)(cpd);

    // Reset chip
    put_reg(sc, LAN91CXX_RCR, LAN91CXX_RCR_SOFT_RST);
    put_reg(sc, LAN91CXX_RCR, 0);

    val = get_reg(sc, LAN91CXX_EPH_STATUS);
#ifndef LAN91CXX_IS_LAN91C111
    // LINK_OK on 91C111 is just a general purpose input and may not
    // have anything to do with the link.
    if (!(val & LAN91CXX_STATUS_LINK_OK)) {
 db_printf("no link\n");
        return false;  // Link not connected
    }
#endif


#if DEBUG & 9
    db_printf("LAN91CXX - status: %04x\n", val);
#endif

#if 0 < CYGINT_DEVS_ETH_SMSC_LAN91CXX_STATIC_ESA
    // Use statically configured ESA from the private data
#if DEBUG & 9
    db_printf("LAN91CXX - static ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                cpd->enaddr[0],
                cpd->enaddr[1],
                cpd->enaddr[2],
                cpd->enaddr[3],
                cpd->enaddr[4],
                cpd->enaddr[5] );
#endif // DEBUG
    // Set up hardware address
    for (i = 0;  i < sizeof(cpd->enaddr);  i += 2)
        put_reg(sc, LAN91CXX_IA01+i/2,
                cpd->enaddr[i] | (cpd->enaddr[i+1] << 8));
#else // not CYGINT_DEVS_ETH_SMSC_LAN91CXX_STATIC_ESA
    // Find ESA - check possible sources in sequence and stop when
    // one provides the ESA:
    //   RedBoot option (via provide_esa)
    //   Compile-time configuration
    //   EEPROM

    if (NULL != cpd->provide_esa) {
        esa_configured = cpd->provide_esa(cpd);
# if DEBUG & 8
        if (esa_configured)
            db_printf("Got ESA from RedBoot option\n");
# endif
    }
    if (!esa_configured && cpd->hardwired_esa) {
        // ESA is already set in cpd->esa[]
        esa_configured = true;
# if DEBUG & 8
        db_printf("Got ESA from cpd\n");
# endif
    }
    if (esa_configured) {
        // Set up hardware address
        for (i = 0;  i < sizeof(cpd->enaddr);  i += 2)
            put_reg(sc, LAN91CXX_IA01+i/2,
                    cpd->enaddr[i] | (cpd->enaddr[i+1] << 8));
    } else {
        // Use the address from the serial EEPROM
        // Read out hardware address
        for (i = 0;  i < sizeof(cpd->enaddr);  i += 2) {
            unsigned short z = get_reg(sc, LAN91CXX_IA01+i/2 );
            cpd->enaddr[i] =   (unsigned char)(0xff & z);
            cpd->enaddr[i+1] = (unsigned char)(0xff & (z >> 8));
        }
        esa_configured = true;
# if DEBUG & 8
        db_printf("Got ESA from eeprom\n");
# endif
    }
#if DEBUG & 9
    db_printf("LAN91CXX - ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                cpd->enaddr[0],
                cpd->enaddr[1],
                cpd->enaddr[2],
                cpd->enaddr[3],
                cpd->enaddr[4],
                cpd->enaddr[5] );
#endif // DEBUG
#endif // !CYGINT_DEVS_ETH_SMSC_LAN91CXX_STATIC_ESA

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, cpd->enaddr);
    return true;
}

static void
lan91cxx_stop(struct eth_drv_sc *sc)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    DEBUG_FUNCTION();

    CYG_ASSERT( cpd->within_send < 10, "stop: Excess send recursions" );
    cpd->within_send++;
    // Complete any outstanding activity:
    if ( cpd->txbusy ) {
        cpd->txbusy = 0;
#if DEBUG & 9
        db_printf("LAN91CXX - Stopping, cleaning up pending TX\n" );
#endif
        (sc->funs->eth_drv->tx_done)(sc, cpd->txkey, 0);
    }
    // Reset chip
    put_reg(sc, LAN91CXX_RCR, LAN91CXX_RCR_SOFT_RST);
    put_reg(sc, LAN91CXX_RCR, 0);
    cpd->txbusy = cpd->within_send = 0;
}

//
// This function is called to "start up" the interface.  It may be called
// multiple times, even when the hardware is already running.  It will be
// called whenever something "hardware oriented" changes and should leave
// the hardware ready to send/receive packets.
//
static void
lan91cxx_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    cyg_uint16 intr;
#ifdef LAN91CXX_IS_LAN91C111
    cyg_uint16 phy_ctl;
    int delay;
#endif
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif
    DEBUG_FUNCTION();

#ifdef LAN91CXX_IS_LAN91C111
    HAL_DELAY_US(100000);

    // 91C111 Errata. Internal PHY comes up disabled. Must enable here.
    phy_ctl = lan91cxx_read_phy(sc, 0, LAN91CXX_PHY_CTRL);
    phy_ctl &= ~LAN91CXX_PHY_CTRL_MII_DIS;
    lan91cxx_write_phy(sc, 0, LAN91CXX_PHY_CTRL, phy_ctl);

    // Start auto-negotiation
    put_reg(sc, LAN91CXX_RPCR,
	    LAN91CXX_RPCR_LEDA_RX | LAN91CXX_RPCR_LEDB_LINK | LAN91CXX_RPCR_ANEG);

    // wait for auto-negotiation to finish.
    // give it ~5 seconds before giving up (no cable?)
    delay = 50;
    while (!(lan91cxx_read_phy(sc, 0, LAN91CXX_PHY_STAT) & 0x20)) {
	if (--delay <= 0)
	    break;
	HAL_DELAY_US(100000);
    }
#endif

    put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_reset_mmu);

    put_reg(sc, LAN91CXX_INTERRUPT, 0);   // disable interrupts
    intr = get_reg(sc, LAN91CXX_INTERRUPT);
    put_reg(sc, LAN91CXX_INTERRUPT, intr &      // ack old interrupts
            (LAN91CXX_INTERRUPT_TX_INT | LAN91CXX_INTERRUPT_TX_EMPTY_INT | 
            LAN91CXX_INTERRUPT_RX_OVRN_INT | LAN91CXX_INTERRUPT_ERCV_INT));
    put_reg(sc, LAN91CXX_RCR, 
#ifdef RCR_HAS_ABORT_ENB // 91C96 does not - page 46.
            LAN91CXX_RCR_ABORT_ENB |
#endif
            LAN91CXX_RCR_STRIP_CRC |
            LAN91CXX_RCR_RXEN | LAN91CXX_RCR_ALMUL);
    put_reg(sc, LAN91CXX_TCR, LAN91CXX_TCR_TXENA | LAN91CXX_TCR_PAD_EN);
    put_reg(sc, LAN91CXX_CONTROL, 0);
    put_reg(sc, LAN91CXX_INTERRUPT,       // enable interrupts
            LAN91CXX_INTERRUPT_RCV_INT_M);

#ifdef CYGPKG_NET
    if (( 0
#ifdef ETH_DRV_FLAGS_PROMISC_MODE
         != (flags & ETH_DRV_FLAGS_PROMISC_MODE)
#endif
        ) || (ifp->if_flags & IFF_PROMISC)
        ) {
        // Then we select promiscuous mode.
        unsigned short rcr;
        rcr = get_reg(sc, LAN91CXX_RCR );
        rcr |= LAN91CXX_RCR_PRMS;
        put_reg(sc, LAN91CXX_RCR, rcr );
    }
#endif
}

//
// This routine is called to perform special "control" opertions
//
static int
lan91cxx_control(struct eth_drv_sc *sc, unsigned long key,
               void *data, int data_length)
{
    unsigned char *esa = (unsigned char *)data;
    int i;
    unsigned short reg;
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;

    DEBUG_FUNCTION();

    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
#if 9 & DEBUG
        db_printf("LAN91CXX - set ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                esa[0],
                esa[1],
                esa[2],
                esa[3],
                esa[4],
                esa[5] );
#ifndef CYGSEM_DEVS_ETH_SMSC_LAN91CXX_WRITE_EEPROM
        db_printf("*** PERMANENT EEPROM WRITE NOT ENABLED ***\n");
#endif
#endif // DEBUG

#ifdef CYGSEM_DEVS_ETH_SMSC_LAN91CXX_WRITE_EEPROM
        // Only now can we command the chip to perform EEPROM writes:

        // select arbitrary writing to the EEPROM
        reg = get_reg(sc, LAN91CXX_CONTROL);
        reg |= LAN91CXX_CONTROL_EEPROM_SELECT;
        put_reg(sc, LAN91CXX_CONTROL, reg );

        for (i = 0;  i < sizeof(cpd->enaddr);  i += 2) {
            int j;
            // Set the address register
            put_reg(sc, LAN91CXX_POINTER, LAN91CXX_ESA_EEPROM_OFFSET + i/2);
            // Poke the data
            put_reg(sc, LAN91CXX_GENERAL, esa[i] | (esa[i+1] << 8));
            // Command the store
            reg = get_reg(sc, LAN91CXX_CONTROL);
            reg |= LAN91CXX_CONTROL_STORE;
            put_reg(sc, LAN91CXX_CONTROL, reg );
            // and poll for completion
            for ( j = 1024 * 1024; 0 < j ; j-- ) {
                reg = get_reg(sc, LAN91CXX_CONTROL);
                if ( 0 == (reg & LAN91CXX_CONTROL_EEPROM_BUSY) )
                    break;
            }
            CYG_ASSERT( 0 < j, "EEPROM write timout!" );
        }

        reg = get_reg(sc, LAN91CXX_CONTROL);
        CYG_ASSERT( 0 == (reg & LAN91CXX_CONTROL_EEPROM_BUSY),
                    "EEPROM still busy!" );
        // Clear the EEPROM selection bit
        reg &=~LAN91CXX_CONTROL_EEPROM_SELECT;
        put_reg(sc, LAN91CXX_CONTROL, reg );
        // and check it "took"
        reg = get_reg(sc, LAN91CXX_CONTROL);
        CYG_ASSERT( 0 == (reg & LAN91CXX_CONTROL_EEPROM_SELECT),
                    "EEPROM still selected!" );
        // and command a complete reload
        reg |= LAN91CXX_CONTROL_RELOAD;
        put_reg(sc, LAN91CXX_CONTROL, reg );
        for ( i = 1024 * 1024; 0 < i ; i-- ) {
            reg = get_reg(sc, LAN91CXX_CONTROL);
            if ( 0 == (reg & LAN91CXX_CONTROL_EEPROM_BUSY) )
                break;
        }
        CYG_ASSERT( 0 < i, "EEPROM reload timout!" );
        // Now extract the MAC address that is in the chip, and tell the
        // system about it.
        for (i = 0;  i < sizeof(cpd->enaddr);  i += 2) {
            unsigned short z = get_reg(sc, LAN91CXX_IA01+i/2 );
            cpd->enaddr[i] =   (unsigned char)(0xff & z);
            cpd->enaddr[i+1] = (unsigned char)(0xff & (z >> 8));
        }
#if DEBUG & 9
        db_printf("LAN91CXX - eeprom new ESA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    cpd->enaddr[0],
                    cpd->enaddr[1],
                    cpd->enaddr[2],
                    cpd->enaddr[3],
                    cpd->enaddr[4],
                    cpd->enaddr[5] );
#endif // DEBUG
        for (i = 0;  i < sizeof(cpd->enaddr);  i++ ) {
            CYG_ASSERT( esa[i] == cpd->enaddr[i], "ESA not written correctly" );
            if ( esa[i] != cpd->enaddr[i] )
                return 1; // the operation failed.
        }
#else // not CYGSEM_DEVS_ETH_SMSC_LAN91CXX_WRITE_EEPROM
        // Whatever, we can write the MAC address into the interface info,
        // and the chip registers no problem.
        for ( i = 0; i < sizeof(cpd->enaddr);  i++ )
            cpd->enaddr[i] = esa[i];
        for (i = 0;  i < sizeof(cpd->enaddr);  i += 2) {
            reg = cpd->enaddr[i] | (cpd->enaddr[i+1] << 8);
            put_reg(sc, LAN91CXX_IA01+i/2, reg );
        }
#endif // !CYGSEM_DEVS_ETH_SMSC_LAN91CXX_WRITE_EEPROM
        return 0;

#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
        // Extract the MAC address that is in the chip, and tell the
        // system about it.
        for (i = 0;  i < sizeof(cpd->enaddr);  i += 2) {
            unsigned short z = get_reg(sc, LAN91CXX_IA01+i/2 );
            esa[i] =   (unsigned char)(0xff & z);
            esa[i+1] = (unsigned char)(0xff & (z >> 8));
        }
        return 0;
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
        strcpy( p->description, "SMSC LAN91Cxx" );
        // CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

        reg = get_reg(sc, LAN91CXX_EPH_STATUS);
        if ((reg & LAN91CXX_STATUS_LINK_OK) == 0) {
            p->operational = 2;         // LINK DOWN
            p->duplex = 1;              // UNKNOWN
            p->speed = 0;
        }
        else {
            p->operational = 3;         // LINK UP
            p->duplex = 2;              // 2 = SIMPLEX, 3 = DUPLEX
            p->speed = 10 * 1000000;    // it's only a 10Mbit device
        }

#ifdef KEEP_STATISTICS
        {
            struct smsc_lan91cxx_stats *ps = &(cpd->stats);

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

        p->tx_queue_len = 1;

        return 0; // OK
    }
#endif
    default:
        break;
    }
    return 1;
}

//
// This routine is called to see if it is possible to send another packet.
// It will return non-zero if a transmit is possible, zero otherwise.
//
static int
lan91cxx_can_send(struct eth_drv_sc *sc)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    int tcr;

    DEBUG_FUNCTION();

#ifndef LAN91CXX_IS_LAN91C111
    // LINK_OK on 91C111 is just a general purpose input and may not
    // have anything to do with the link.
    if ((get_reg(sc, LAN91CXX_EPH_STATUS) & LAN91CXX_STATUS_LINK_OK) == 0) {
	db_printf("no link\n");
        return false;  // Link not connected
    }
#endif

    CYG_ASSERT( cpd->within_send < 10, "can_send: Excess send recursions" );
    cpd->within_send++;

    tcr = get_reg(sc, LAN91CXX_TCR);
    if ( 0 == (LAN91CXX_TCR_TXENA & tcr) ) {
#if DEBUG & 1
        db_printf("%s: ENGINE RESTART: tcr %x\n", __FUNCTION__, tcr );
#endif
        // Complete any outstanding activity:
        if ( cpd->txbusy ) {
            cpd->txbusy = 0;
#if DEBUG & 9
            db_printf("LAN91CXX - can_send, cleaning up pending TX\n" );
#endif
            (sc->funs->eth_drv->tx_done)(sc, cpd->txkey, 0);
        }
        tcr |= LAN91CXX_TCR_TXENA;
        put_reg(sc, LAN91CXX_TCR, tcr);
    }

    // This helps unstick deadly embraces.
    lan91cxx_poll( sc ); // Deal with any outstanding rx state
    cpd->within_send--;

    return (cpd->txbusy == 0) && (0 == cpd->within_send);
}

//
// This routine is called to send data to the hardware.
static void 
lan91cxx_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
            int total_len, unsigned long key)
{
    struct lan91cxx_priv_data *cpd = 
        (struct lan91cxx_priv_data *)sc->driver_private;
    int i, len, plen, tcr;

    unsigned short *sdata = NULL;
    unsigned short ints, control;
    cyg_uint16 packet, status;

    DEBUG_FUNCTION();

    INCR_STAT( tx_count );

    // Worry about the TX engine stopping.
    tcr = get_reg(sc, LAN91CXX_TCR);
    if ( 0 == (LAN91CXX_TCR_TXENA & tcr) ) {
#if DEBUG & 1
        db_printf("%s: ENGINE RESTART: tcr %x\n", __FUNCTION__, tcr );
#endif
        tcr |= LAN91CXX_TCR_TXENA;
        put_reg(sc, LAN91CXX_TCR, tcr);
    }

    // This helps unstick deadly embraces.
    CYG_ASSERT( cpd->within_send < 10, "send: Excess send recursions" );
    cpd->within_send++;
    lan91cxx_poll( sc ); // Deal with any outstanding rx state
    cpd->within_send--;

    cpd->txbusy = 1;
    cpd->txkey = key;

    // Find packet length
    plen = 0;
    for (i = 0;  i < sg_len;  i++)
        plen += sg_list[i].len;

    CYG_ASSERT( plen == total_len, "sg data length mismatch" );

    // Alloc new TX packet
    do {
        put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_alloc_for_tx
#ifndef LAN91CXX_IS_LAN91C111
		| ((plen >> 8) & 0x07)
#endif
	    );

        i = 1024 * 1024;
        do {
            status = get_reg(sc, LAN91CXX_INTERRUPT);
        } while (0 == (status & LAN91CXX_INTERRUPT_ALLOC_INT) && (--i > 0) );
        if ( i )
            packet = get_reg(sc, LAN91CXX_PNR);
        else
            packet = 0xffff;
#if DEBUG & 1
        db_printf("%s: allocated packet %04x\n", __FUNCTION__, packet);
#endif
        packet = packet >> 8;
        if (packet & 0x80) {
            // Hm.. Isn't this a dead end?
#if DEBUG & 1
            db_printf("%s: Allocation failed! Retrying...\n", __FUNCTION__ );
#endif
            // Not if we can make progress with what's filling memory.
            lan91cxx_poll( sc ); // Deal with any outstanding state
            continue;
        }
    } while (0);

#if DEBUG & 4
    db_printf("#####Tx packet allocated %x (previous %x)\n",
                packet, cpd->txpacket);
#endif
    cpd->txpacket = packet;

    put_reg(sc, LAN91CXX_PNR, packet);
    // Note: Check FIFO state here before continuing?
    put_reg(sc, LAN91CXX_POINTER, LAN91CXX_POINTER_AUTO_INCR | 0x0000);
    // Pointer is now set, and the proper bank is selected for
    // data writes.

    // Prepare header:
    put_data(sc, CYG_CPU_TO_LE16(0));        // reserve space for status word
    // packet length (includes status, byte-count and control shorts)
    put_data(sc, CYG_CPU_TO_LE16(0x7FE & (plen + 6)) ); // Always even, always < 15xx(dec)

    // Put data into buffer
    for (i = 0;  i < sg_len;  i++) {
        sdata = (unsigned short *)sg_list[i].buf;
        len = sg_list[i].len;

        CYG_ASSERT(0 == (len & 1) || (i == (sg_len-1)), "odd length");
        CYG_ASSERT( sdata, "No sg data pointer here" );
        while(len >= sizeof(*sdata)) {
            put_data(sc, *sdata++);
            len -= sizeof(*sdata);
        }
    }
    CYG_ASSERT( sdata, "No sg data pointer outside" );

    // Lay down the control short unconditionally at the end.
    // (or it might use random memory contents)
    control = 0;
    if ( 1 & plen ) {
        // Need to set ODD flag and insert the data
        unsigned char onebyte = *(unsigned char*)sdata;
        control = onebyte;
        control |= LAN91CXX_CONTROLBYTE_ODD;
    }
    control |= LAN91CXX_CONTROLBYTE_CRC; // Just in case...
    put_data(sc, CYG_CPU_TO_LE16(control));

    // Enqueue the packet
    put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_enq_packet);

    // Ack TX empty int and unmask it.
    ints = get_reg(sc, LAN91CXX_INTERRUPT) & 0xff00;
    put_reg(sc, LAN91CXX_INTERRUPT, ints | LAN91CXX_INTERRUPT_TX_SET_ACK);
    put_reg(sc, LAN91CXX_INTERRUPT, ints | LAN91CXX_INTERRUPT_TX_SET_M);

#if DEBUG & 1
    ints = get_reg(sc, LAN91CXX_INTERRUPT);
    db_printf("%s:END: ints at TX: %04x\n", __FUNCTION__, ints);
#endif
}

static void
lan91cxx_TxEvent(struct eth_drv_sc *sc, int stat)
{
    unsigned short packet, ints, tcr;
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    int success = 1;

    DEBUG_FUNCTION();

    INCR_STAT( tx_complete );

    // Ack and mask TX interrupt set
    ints = get_reg(sc, LAN91CXX_INTERRUPT) & 0xff00;
    ints |= LAN91CXX_INTERRUPT_TX_SET_ACK;
    ints &= ~LAN91CXX_INTERRUPT_TX_SET_M;
    put_reg(sc, LAN91CXX_INTERRUPT, ints);

    // Get number of completed packet and read the status word
    packet = get_reg(sc, LAN91CXX_FIFO_PORTS);
#if DEBUG & 1
    db_printf("%s:START: fifo %04x ints %04x\n", __FUNCTION__, packet, ints);
#endif

#ifdef KEEP_STATISTICS
    {
        unsigned short reg;

        reg = get_reg( sc, LAN91CXX_EPH_STATUS );
        
        // Covering each bit in turn...
        if ( reg & LAN91CXX_STATUS_TX_UNRN   ) INCR_STAT( tx_underrun );
        //if ( reg & LAN91CXX_STATUS_LINK_OK ) INCR_STAT(  );
        //if ( reg & LAN91CXX_STATUS_CTR_ROL ) INCR_STAT(  );
        //if ( reg & LAN91CXX_STATUS_EXC_DEF ) INCR_STAT(  );
        if ( reg & LAN91CXX_STATUS_LOST_CARR ) INCR_STAT( tx_carrier_loss );
        if ( reg & LAN91CXX_STATUS_LATCOL    ) INCR_STAT( tx_late_collisions );
        //if ( reg & LAN91CXX_STATUS_WAKEUP  ) INCR_STAT(  );
        if ( reg & LAN91CXX_STATUS_TX_DEFR   ) INCR_STAT( tx_deferred );
        //if ( reg & LAN91CXX_STATUS_LTX_BRD ) INCR_STAT(  );
        if ( reg & LAN91CXX_STATUS_SQET      ) INCR_STAT( tx_sqetesterrors );
        if ( reg & LAN91CXX_STATUS_16COL     ) INCR_STAT( tx_max_collisions );
        //if ( reg & LAN91CXX_STATUS_LTX_MULT) INCR_STAT(  );
        if ( reg & LAN91CXX_STATUS_MUL_COL   ) INCR_STAT( tx_mult_collisions );
        if ( reg & LAN91CXX_STATUS_SNGL_COL  ) INCR_STAT( tx_single_collisions );
        if ( reg & LAN91CXX_STATUS_TX_SUC    ) INCR_STAT( tx_good );

        cpd->stats.tx_total_collisions = 
            cpd->stats.tx_late_collisions + 
            cpd->stats.tx_max_collisions + 
            cpd->stats.tx_mult_collisions + 
            cpd->stats.tx_single_collisions;

        // We do not need to look in the Counter Register (LAN91CXX_COUNTER)
        // because it just mimics the info we already have above.
    }
#endif // KEEP_STATISTICS
    // We do not really care about Tx failure.  Ethernet is not a reliable
    // medium.  But we do care about the TX engine stopping.
    tcr = get_reg(sc, LAN91CXX_TCR);
    if ( 0 == (LAN91CXX_TCR_TXENA & tcr) ) {
#if DEBUG & 1
        db_printf("%s: ENGINE RESTART: tcr %x ints %04x\n", __FUNCTION__, tcr, ints);
#endif
        tcr |= LAN91CXX_TCR_TXENA;
        put_reg(sc, LAN91CXX_TCR, tcr);
        success = 0; // And treat this as an error...
    }

    packet &= 0xff;

    // It certainly appears that occasionally the tx fifo tells lies; we
    // get the wrong packet number.  Freeing the one we allocated seems to
    // give correct operation.
#ifdef CYGPKG_INFRA_DEBUG
    // Then we log, OOI, the number of times we get a bad packet number
    // from the tx done fifo.
    if (cpd->txpacket != packet )
        lan91cxx_txfifo_bad++;
    else
        lan91cxx_txfifo_good++;
#endif
#if DEBUG & 4
    db_printf("#####Tx packet freed %x (expected %x)\n", packet, cpd->txpacket );
#endif
    // and then free the packet
    put_reg(sc, LAN91CXX_PNR, cpd->txpacket);
    put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_rel_packet);

    // Ack the TX int which is supposed to clear the packet from the TX
    // completion queue.
    ints = get_reg(sc, LAN91CXX_INTERRUPT) & 0xff00;
    ints |= LAN91CXX_INTERRUPT_TX_FIFO_ACK;
    put_reg(sc, LAN91CXX_INTERRUPT, ints);

#if DEBUG & 1
    // Hm... The free doesn't seem to have the desired effect?!?
    ints = get_reg(sc, LAN91CXX_INTERRUPT);
    packet = get_reg(sc, LAN91CXX_FIFO_PORTS);
    db_printf("%s:END: fifo %04x ints %04x\n", __FUNCTION__, packet, ints);
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
// 'lan91cxx_recv' will be called to actually fetch it from the hardware.
//
static void
lan91cxx_RxEvent(struct eth_drv_sc *sc)
{
    struct lan91cxx_priv_data *cpd = 
        (struct lan91cxx_priv_data *)sc->driver_private;
    unsigned short stat, len;
#ifdef LAN91CXX_32BIT_RX
    cyg_uint32 val;
#endif

    DEBUG_FUNCTION();

    stat = get_reg(sc, LAN91CXX_FIFO_PORTS);
#if DEBUG & 1
    db_printf("RxEvent - FIFOs: 0x%04x\n", stat);
#endif
    if ( 0x8000 & stat ) {
        // Then the Rx FIFO is empty
#if DEBUG & 4
        db_printf("#####RxEvent with empty fifo\n");
#endif
        return;
    }

    INCR_STAT( rx_count );

#if DEBUG & 4
    db_printf("#####Rx packet allocated %x (previous %x)\n",
                0xff & (stat >> 8), cpd->rxpacket );
#endif
    // There is an Rx Packet ready
    cpd->rxpacket = 0xff & (stat >> 8);

    // Read status and (word) length
    put_reg(sc, LAN91CXX_POINTER, (LAN91CXX_POINTER_RCV | LAN91CXX_POINTER_READ |
                                 LAN91CXX_POINTER_AUTO_INCR | 0x0000));
#ifdef LAN91CXX_32BIT_RX
    val = get_data(sc);
    val = CYG_LE32_TO_CPU(val);
    stat = val & 0xffff;
    len = ((val >> 16) & 0xffff) - 6;   // minus header/footer words
#else
    stat = get_data(sc);
    stat = CYG_LE16_TO_CPU(stat);
    len = get_data(sc);
    len = CYG_LE16_TO_CPU(len) - 6;     // minus header/footer words
#endif

#ifdef KEEP_STATISTICS
    if ( stat & LAN91CXX_RX_STATUS_ALIGNERR ) INCR_STAT( rx_align_errors );
    //if ( stat & LAN91CXX_RX_STATUS_BCAST    ) INCR_STAT(  );
    if ( stat & LAN91CXX_RX_STATUS_BADCRC   ) INCR_STAT( rx_crc_errors );
    if ( stat & LAN91CXX_RX_STATUS_TOOLONG  ) INCR_STAT( rx_too_long_frames );
    if ( stat & LAN91CXX_RX_STATUS_TOOSHORT ) INCR_STAT( rx_short_frames );
    //if ( stat & LAN91CXX_RX_STATUS_MCAST    ) INCR_STAT(  );
#endif // KEEP_STATISTICS

    if ((stat & LAN91CXX_RX_STATUS_BAD) == 0) {
        INCR_STAT( rx_good );
        // Then it's OK

        if( LAN91CXX_RX_STATUS_IS_ODD(cpd,stat) )
            len++;

#if DEBUG & 1
        db_printf("RxEvent good rx - stat: 0x%04x, len: 0x%04x\n", stat, len);
#endif
        // Check for bogusly short packets; can happen in promisc mode:
        // Asserted against and checked by upper layer driver.
#ifdef CYGPKG_NET
        if ( len > sizeof( struct ether_header ) )
            // then it is acceptable; offer the data to the network stack
#endif
        (sc->funs->eth_drv->recv)(sc, len);

        return;
    }

    // Not OK for one reason or another...
#if DEBUG & 1
    db_printf("RxEvent - bad rx: stat: 0x%04x, len: 0x%04x\n", stat, len);
#endif

    // Free packet
    put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_remrel_rx_frame);
}

//
// This function is called as a result of the "eth_drv_recv()" call above.
// Its job is to actually fetch data for a packet from the hardware once
// memory buffers have been allocated for the packet.  Note that the buffers
// may come in pieces, using a scatter-gather list.  This allows for more
// efficient processing in the upper layers of the stack.
//
static void
lan91cxx_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
#if (4 & DEBUG) || defined(CYGPKG_INFRA_DEBUG) || \
    defined(KEEP_STATISTICS) || defined(LAN91CXX_IS_LAN91C111)
    struct lan91cxx_priv_data *cpd = 
        (struct lan91cxx_priv_data *)sc->driver_private;
#endif
    int i;
    short mlen=0, plen;
    rxd_t *data=NULL, val;
    unsigned char *cp, cval;

    DEBUG_FUNCTION();

    INCR_STAT( rx_deliver );

    put_reg(sc, LAN91CXX_POINTER, (LAN91CXX_POINTER_RCV | LAN91CXX_POINTER_READ |
                                 LAN91CXX_POINTER_AUTO_INCR));
    val = get_data(sc);

    // packet length (minus header/footer)
#ifdef LAN91CXX_32BIT_RX
    val = CYG_LE32_TO_CPU(val);
    plen = (val >> 16) - 6;
#else
    val = CYG_LE16_TO_CPU(val);
    plen = get_data(sc);
    plen = CYG_LE16_TO_CPU(plen) - 6;
#endif

    if( LAN91CXX_RX_STATUS_IS_ODD(cpd,val) )
	plen++;

    for (i = 0;  i < sg_len;  i++) {
        data = (rxd_t *)sg_list[i].buf;
        mlen = sg_list[i].len;

        CYG_ASSERT(0 == (mlen & (sizeof(*data) - 1)) || (i == (sg_len-1)), "odd length");

#if DEBUG & 1
        db_printf("%s : mlen %x, plen %x\n", __FUNCTION__, mlen, plen);
#endif
        if (data) {
            while (mlen >= sizeof(*data)) {
                *data++ = get_data(sc);
                mlen -= sizeof(*data);
                plen -= sizeof(*data);
            }
        }
        else { // must actively discard ie. read it from the chip anyway.
            while (mlen >= sizeof(*data)) {
                (void)get_data(sc);
                mlen -= sizeof(*data);
                plen -= sizeof(*data);
            }
        }
    }
    val = get_data(sc); // Read control word (and potential data) unconditionally
#ifdef LAN91CXX_32BIT_RX
    val = CYG_LE32_TO_CPU(val);
    if (plen & 2) {
	if (data)
	    *(cyg_uint16 *)data = val & 0xffff;
	cp = (unsigned char *)data + 2;
	val >>= 16;
	mlen -= 2;
    } else
#else
    val = CYG_LE16_TO_CPU(val);
#endif
	cp = (unsigned char *)data;

    CYG_ASSERT(val & LAN91CXX_CONTROLBYTE_RX, 
               "Controlbyte is not for Rx");
    CYG_ASSERT( (1 == mlen) == (0 != LAN91CXX_CONTROLBYTE_IS_ODD(cpd,val)), 
                "Controlbyte does not match");
    if (data && (1 == mlen) && LAN91CXX_CONTROLBYTE_IS_ODD(cpd,val) ) {
        cval = val & 0x00ff;    // last byte contains data
        *cp = cval;
    }

    val = get_reg(sc, LAN91CXX_FIFO_PORTS);
#if DEBUG & 4
    if ( 0x8000 & val ) // Then the Rx FIFO is empty
        db_printf("#####Rx packet NOT freed, stat is %x (expected %x)\n",
                    val, cpd->rxpacket);
    else
        db_printf("#####Rx packet freed %x (expected %x)\n",
                    0xff & (val >> 8), cpd->rxpacket );
#endif
    CYG_ASSERT( (0xff & (val >> 8)) == cpd->rxpacket, "Unexpected rx packet" );

    // Free packet
    put_reg(sc, LAN91CXX_MMU_COMMAND, LAN91CXX_MMU_remrel_rx_frame);
}

static void
lan91cxx_poll(struct eth_drv_sc *sc)
{
    unsigned short event;
    struct lan91cxx_priv_data *cpd = 
        (struct lan91cxx_priv_data *)sc->driver_private;

//    DEBUG_FUNCTION();
    while (1) {
        cyg_drv_interrupt_acknowledge(cpd->interrupt);
        // Get the (unmasked) requests
        event = get_reg(sc, LAN91CXX_INTERRUPT);
        event = event & (event >> 8) & 0xff;
        if (0 == event)
            break;
#if 0
        if (event & LAN91CXX_INTERRUPT_ERCV_INT) {
            // Early receive interrupt
        }
        else if (event & LAN91CXX_INTERRUPT_EPH_INT) {
            // ethernet protocol handler failures
        }
        else if (event & LAN91CXX_INTERRUPT_RX_OVRN_INT) {
            // receive overrun
        }
        else if (event & LAN91CXX_INTERRUPT_ALLOC_INT) {
            // allocation interrupt
        }
        else
#endif
        if (event & LAN91CXX_INTERRUPT_TX_SET) {
            lan91cxx_TxEvent(sc, event);
        }
        if (event & LAN91CXX_INTERRUPT_RCV_INT) {
            lan91cxx_RxEvent(sc);
        }
        if (event & ~(LAN91CXX_INTERRUPT_TX_SET | LAN91CXX_INTERRUPT_RCV_INT))
            db_printf("%s: Unknown interrupt: 0x%04x\n",
			__FUNCTION__, event);
    }
}

#ifdef LAN91CXX_IS_LAN91C111

static cyg_uint16
lan91cxx_read_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr, cyg_uint8 phyreg)
{
    int i, mask, input_idx, clk_idx = 0;
    cyg_uint16 mii_reg, value;
    cyg_uint8 bits[64];

    // 32 consecutive ones on MDO to establish sync
    for (i = 0; i < 32; ++i)
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

    // Start code <01>
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

    // Read command <10>
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;

    // Output the PHY address, msb first
    for (mask = 0x10; mask; mask >>= 1) {
	if (phyaddr & mask)
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
	else
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    }

    // Output the phy register number, msb first
    for (mask = 0x10; mask; mask >>= 1) {
	if (phyreg & mask)
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
	else
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    }

    // Tristate and turnaround (1 bit times)
    bits[clk_idx++] = 0;

    // Input starts at this bit time
    input_idx = clk_idx;

    // Will input 16 bits
    for (i = 0; i < 16; ++i)
	bits[clk_idx++] = 0;

    // Final clock bit
    bits[clk_idx++] = 0;

    // Get the current MII register value
    mii_reg = get_reg(sc, LAN91CXX_MGMT);

    // Turn off all MII Interface bits
    mii_reg &= ~(LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MCLK | 
		 LAN91CXX_MGMT_MDI | LAN91CXX_MGMT_MDO);

    // Clock all 64 cycles
    for (i = 0; i < sizeof(bits); ++i) {
	// Clock Low - output data
	put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i]);
	HAL_DELAY_US(50);

	// Clock Hi - input data
	put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i] | LAN91CXX_MGMT_MCLK);
	HAL_DELAY_US(50);

	bits[i] |= get_reg(sc, LAN91CXX_MGMT) & LAN91CXX_MGMT_MDI;
    }

    // Return to idle state
    put_reg(sc, LAN91CXX_MGMT, mii_reg);
    HAL_DELAY_US(50);

    // Recover input data
    for (value = 0, i = 0; i < 16; ++i) {
	value <<= 1;
	if (bits[input_idx++] & LAN91CXX_MGMT_MDI)
	    value |= 1;
    }
    return value;
}

static void
lan91cxx_write_phy(struct eth_drv_sc *sc, cyg_uint8 phyaddr,
		   cyg_uint8 phyreg, cyg_uint16 value)
{
    int i, mask, clk_idx = 0;
    cyg_uint16 mii_reg;
    cyg_uint8 bits[65];

    // 32 consecutive ones on MDO to establish sync
    for (i = 0; i < 32; ++i)
	bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

    // Start code <01>
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

    // Write command <01>
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;

    // Output the PHY address, msb first
    for (mask = 0x10; mask; mask >>= 1) {
	if (phyaddr & mask)
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
	else
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    }

    // Output the phy register number, msb first
    for (mask = 0x10; mask; mask >>= 1) {
	if (phyreg & mask)
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
	else
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    }

    // Tristate and turnaround (2 bit times)
    bits[clk_idx++] = 0;
    bits[clk_idx++] = 0;

    // Write out 16 bits of data, msb first
    for (mask = 0x8000; mask; mask >>= 1) {
	if (value & mask)
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MDO;
	else
	    bits[clk_idx++] = LAN91CXX_MGMT_MDOE;
    }

    // Final clock bit (tristate)
    bits[clk_idx++] = 0;

    // Get the current MII register value
    mii_reg = get_reg(sc, LAN91CXX_MGMT);

    // Turn off all MII Interface bits
    mii_reg &= ~(LAN91CXX_MGMT_MDOE | LAN91CXX_MGMT_MCLK | 
		 LAN91CXX_MGMT_MDI | LAN91CXX_MGMT_MDO);

    // Clock all cycles
    for (i = 0; i < sizeof(bits); ++i) {
	// Clock Low - output data
	put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i]);
	HAL_DELAY_US(50);

	// Clock Hi - input data
	put_reg(sc, LAN91CXX_MGMT, mii_reg | bits[i] | LAN91CXX_MGMT_MCLK);
	HAL_DELAY_US(50);

//	bits[i] |= get_reg(sc, LAN91CXX_MGMT) & LAN91CXX_MGMT_MDI;
    }

    // Return to idle state
    put_reg(sc, LAN91CXX_MGMT, mii_reg);
    HAL_DELAY_US(50);
}
#endif // LAN91CXX_IS_LAN91C111

// EOF if_lan91cxx.c
