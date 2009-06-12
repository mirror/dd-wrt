//==========================================================================
//
//      dev/if_fec.c
//
//      Fast ethernet device driver for PowerPC MPC8xxT boards
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
// Date:         2001-01-21
// Purpose:      
// Description:  hardware driver for MPC8xxT FEC
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet device driver for MPC8xx FEC

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_powerpc_fec.h>
#include <pkgconf/io_eth_drivers.h>
#include CYGDAT_DEVS_FEC_ETH_INL

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

#include "fec.h"

// Align buffers on a cache boundary
#define RxBUFSIZE CYGNUM_DEVS_ETH_POWERPC_FEC_RxNUM*CYGNUM_DEVS_ETH_POWERPC_FEC_BUFSIZE
#define TxBUFSIZE CYGNUM_DEVS_ETH_POWERPC_FEC_TxNUM*CYGNUM_DEVS_ETH_POWERPC_FEC_BUFSIZE
static unsigned char fec_eth_rxbufs[RxBUFSIZE] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));
static unsigned char fec_eth_txbufs[TxBUFSIZE] __attribute__((aligned(HAL_DCACHE_LINE_SIZE)));

static struct fec_eth_info fec_eth0_info;
static unsigned char _default_enaddr[] = { 0x08, 0x00, 0x3E, 0x28, 0x7A, 0xBA};
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
#endif
#endif

#define os_printf diag_printf

// For fetching the ESA from RedBoot
#include <cyg/hal/hal_if.h>
#ifndef CONFIG_ESA
#define CONFIG_ESA 6
#endif

ETH_DRV_SC(fec_eth0_sc,
           &fec_eth0_info,   // Driver specific data
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

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
#define _FEC_USE_INTS
#ifdef _FEC_USE_INTS
static cyg_interrupt fec_eth_interrupt;
static cyg_handle_t  fec_eth_interrupt_handle;
#else
#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_MINIMUM
static char fec_fake_int_stack[STACK_SIZE];
static cyg_thread fec_fake_int_thread_data;
static cyg_handle_t fec_fake_int_thread_handle;
static void fec_fake_int(cyg_addrword_t);
#endif // _FEC_USE_INTS
#endif // CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
static void          fec_eth_int(struct eth_drv_sc *data);

#ifndef FEC_ETH_INT
#error  FEC_ETH_INT must be defined
#endif

#ifndef FEC_ETH_PHY
#error  FEC_ETH_PHY must be defined
#endif

#ifndef FEC_ETH_RESET_PHY
#define FEC_ETH_RESET_PHY()
#endif

#ifndef FEC_EPPC_BD_OFFSET
#define FEC_EPPC_BD_OFFSET CYGNUM_DEVS_ETH_POWERPC_FEC_BD_OFFSET
#endif

#ifdef CYGSEM_DEVS_ETH_POWERPC_FEC_STATUS_LEDS
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
#else
#define set_led(b)
#define clear_led(b)
#endif

#ifdef _FEC_USE_INTS
// This ISR is called when the ethernet interrupt occurs
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
#ifdef _FEC_USE_INTS
    // Allow interrupts to happen again
    cyg_drv_interrupt_acknowledge(FEC_ETH_INT);
    cyg_drv_interrupt_unmask(FEC_ETH_INT);
#endif
}

#ifdef CYGSEM_DEVS_ETH_POWERPC_FEC_RESET_PHY
//
// PHY unit access (via MII channel)
//
static void
phy_write(int reg, int addr, unsigned short data)
{
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    volatile struct fec *fec = (volatile struct fec *)((unsigned char *)eppc + FEC_OFFSET);
    int timeout = 0x100000;

    fec->iEvent = iEvent_MII;    
    fec->MiiData = MII_Start | MII_Write | MII_Phy(addr) | MII_Reg(reg) | MII_TA | data;
    while (!(fec->iEvent & iEvent_MII) && (--timeout > 0)) ;
}

static bool
phy_read(int reg, int addr, unsigned short *val)
{
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    volatile struct fec *fec = (volatile struct fec *)((unsigned char *)eppc + FEC_OFFSET);
    int timeout = 0x100000;

    fec->iEvent = iEvent_MII;    
    fec->MiiData = MII_Start | MII_Read | MII_Phy(addr) | MII_Reg(reg) | MII_TA;
    while (!(fec->iEvent & iEvent_MII)) {
        if (--timeout <= 0) {
            return false;
        }
    }
    *val = fec->MiiData & 0x0000FFFF;
    return true;
}
#endif // CYGSEM_DEVS_ETH_POWERPC_FEC_RESET_PHY

//
// [re]Initialize the ethernet controller
//   Done separately since shutting down the device requires a 
//   full reconfiguration when re-enabling.
//   when 
static bool
fec_eth_reset(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    volatile struct fec *fec = (volatile struct fec *)((unsigned char *)eppc + FEC_OFFSET);
    volatile struct fec_bd *rxbd, *txbd;
    unsigned char *RxBUF, *TxBUF;
    int cache_state, int_state;
    int i;
    int TxBD, RxBD;

    // Ignore unless device is idle/stopped
    if ((qi->fec->eControl & eControl_EN) != 0) {
        return true;
    }

    // Make sure interrupts are off while we mess with the device
    HAL_DISABLE_INTERRUPTS(int_state);

    // Ensure consistent state between cache and what the FEC sees
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
      HAL_DCACHE_SYNC();
      HAL_DCACHE_DISABLE();
    }

    // Shut down ethernet controller, in case it is already running
    fec->eControl = eControl_RESET;
    i = 0;
    while ((fec->eControl & eControl_RESET) != 0) {
      if (++i >= 500000) {
	os_printf("FEC Ethernet does not reset\n");
	if (cache_state)
	  HAL_DCACHE_ENABLE();
        HAL_RESTORE_INTERRUPTS(int_state);
	return false;
      }
    }

    fec->iMask  = 0x0000000;  // Disables all interrupts
    fec->iEvent = 0xFFFFFFFF; // Clear all interrupts
    fec->iVector = (1<<29);   // Caution - must match FEC_ETH_INT above

    TxBD = _mpc8xx_allocBd(CYGNUM_DEVS_ETH_POWERPC_FEC_TxNUM * sizeof(struct cp_bufdesc));
    RxBD = _mpc8xx_allocBd(CYGNUM_DEVS_ETH_POWERPC_FEC_RxNUM * sizeof(struct cp_bufdesc));
    txbd = (struct fec_bd *)(TxBD + (cyg_uint32)eppc);
    rxbd = (struct fec_bd *)(RxBD + (cyg_uint32)eppc);
    qi->tbase = qi->txbd = qi->tnext = txbd;
    qi->rbase = qi->rxbd = qi->rnext = rxbd;
    qi->txactive = 0;

    RxBUF = &fec_eth_rxbufs[0];
    TxBUF = &fec_eth_txbufs[0];

    // setup buffer descriptors
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_FEC_RxNUM;  i++) {
        rxbd->length = 0;
        rxbd->buffer = RxBUF;
        rxbd->ctrl   = FEC_BD_Rx_Empty;
        RxBUF += CYGNUM_DEVS_ETH_POWERPC_FEC_BUFSIZE;
        rxbd++;
    }
    rxbd--;
    rxbd->ctrl |= FEC_BD_Rx_Wrap;  // Last buffer
    for (i = 0;  i < CYGNUM_DEVS_ETH_POWERPC_FEC_TxNUM;  i++) {
        txbd->length = 0;
        txbd->buffer = TxBUF;
        txbd->ctrl   = 0;
        TxBUF += CYGNUM_DEVS_ETH_POWERPC_FEC_BUFSIZE;
        txbd++;
    }
    txbd--;
    txbd->ctrl |= FEC_BD_Tx_Wrap;  // Last buffer

    // Reset interrupts
    fec->iMask  = 0x00000000;  // No interrupts enabled
    fec->iEvent = 0xFFFFFFFF;  // Clear all interrupts

    // Initialize shared PRAM
    fec->RxRing = qi->rbase;
    fec->TxRing = qi->tbase;

    // Size of receive buffers
    fec->RxBufSize = CYGNUM_DEVS_ETH_POWERPC_FEC_BUFSIZE;

    // Receiver control
    fec->RxControl = RxControl_MII | RxControl_DRT;
//    fec->RxControl = RxControl_MII | RxControl_LOOP | RxControl_PROM;
    fec->RxHash = IEEE_8023_MAX_FRAME; // Largest possible ethernet frame

    // Transmit control
    fec->TxControl = 4+0;

    // Use largest possible Tx FIFO
    fec->TxWater = 3;

    // DMA control
    fec->FunCode = ((2<<29) | (2<<27) | (0<<24));

    // MII speed control (50MHz)
    fec->MiiSpeed = 0x14;

    // Group address hash
    fec->hash[0] = 0;
    fec->hash[1] = 0;

    // Device physical address
    fec->addr[0] = *(unsigned long *)&enaddr[0];
    fec->addr[1] = *(unsigned long *)&enaddr[4];
    // os_printf("FEC ESA = %08x/%08x\n", fec->addr[0], fec->addr[1]);

    // Enable device
    fec->eControl = eControl_EN | eControl_MUX;
    fec->RxUpdate = 0x0F0F0F0F;  // Any write tells machine to look for work

#ifdef _FEC_USE_INTS
    // Set up for interrupts
    fec->iMask = iEvent_TFINT | iEvent_TXB |
                 iEvent_RFINT | iEvent_RXB;
    fec->iEvent = 0xFFFFFFFF;  // Clear all interrupts
#endif

    if (cache_state)
        HAL_DCACHE_ENABLE();

    // Set LED state
    clear_led(LED_TxACTIVE);
    clear_led(LED_RxACTIVE);

    HAL_RESTORE_INTERRUPTS(int_state);
    return true;
}

//
// Initialize the interface - performed at system startup
// This function must set up the interface, including arranging to
// handle interrupts, etc, so that it may be "started" cheaply later.
//
static bool 
fec_eth_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    volatile struct fec *fec = (volatile struct fec *)((unsigned char *)eppc + FEC_OFFSET);
    int cache_state;
    unsigned long proc_rev;
    bool esa_ok;
#ifdef CYGSEM_DEVS_ETH_POWERPC_FEC_RESET_PHY
    int phy_timeout = 5*1000;  // Wait 5 seconds max for link to clear
    bool phy_ok;
    unsigned short phy_state = 0;
#endif

    // Ensure consistent state between cache and what the FEC sees
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
      HAL_DCACHE_SYNC();
      HAL_DCACHE_DISABLE();
    }

    qi->fec = fec;
    fec_eth_stop(sc);  // Make sure it's not running yet

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
#ifdef _FEC_USE_INTS
    // Set up to handle interrupts
    cyg_drv_interrupt_create(FEC_ETH_INT,
                             CYGARC_SIU_PRIORITY_HIGH,
                             (cyg_addrword_t)sc, //  Data item passed to interrupt handler
                             (cyg_ISR_t *)fec_eth_isr,
                             (cyg_DSR_t *)eth_drv_dsr,
                             &fec_eth_interrupt_handle,
                             &fec_eth_interrupt);
    cyg_drv_interrupt_attach(fec_eth_interrupt_handle);
    cyg_drv_interrupt_acknowledge(FEC_ETH_INT);
    cyg_drv_interrupt_unmask(FEC_ETH_INT);
#else // _FEC_USE_INTS
    // Hack - use a thread to simulate interrupts
    cyg_thread_create(1,                 // Priority
                      fec_fake_int,   // entry
                      (cyg_addrword_t)sc, // entry parameter
                      "CS8900 int",      // Name
                      &fec_fake_int_stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &fec_fake_int_thread_handle,    // Handle
                      &fec_fake_int_thread_data       // Thread data structure
            );
    cyg_thread_resume(fec_fake_int_thread_handle);  // Start it
#endif
#endif

    // Set up parallel port for connection to ethernet tranceiver
    eppc->pio_pdpar = 0x1FFF;
    CYGARC_MFSPR( CYGARC_REG_PVR, proc_rev );
#define PROC_REVB 0x0020
    if ((proc_rev & 0x0000FFFF) == PROC_REVB) {
        eppc->pio_pddir = 0x1C58;
    } else {
        eppc->pio_pddir = 0x1FFF;
    }

    // Get physical device address
#ifdef CYGPKG_REDBOOT
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    esa_ok = flash_get_config("fec_esa", enaddr, CONFIG_ESA);
#else
    esa_ok = false;
#endif
#else
    esa_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET,         
                                         "fec_esa", enaddr, CONFIG_ESA);
#endif
    if (!esa_ok) {
        // Can't figure out ESA
        os_printf("FEC_ETH - Warning! ESA unknown\n");
        memcpy(&enaddr, &_default_enaddr, sizeof(enaddr));
    }

    // Configure the device
    if (!fec_eth_reset(sc, enaddr, 0)) {
        return false;
    }

    fec->iEvent = 0xFFFFFFFF;  // Clear all interrupts

#ifdef CYGSEM_DEVS_ETH_POWERPC_FEC_RESET_PHY
    // Reset PHY (transceiver)
    FEC_ETH_RESET_PHY();

    phy_ok = 0;
    if (phy_read(PHY_BMSR, FEC_ETH_PHY, &phy_state)) {
        if ((phy_state & PHY_BMSR_LINK) !=  PHY_BMSR_LINK) {
            unsigned short reset_mode;
            int i;
            phy_write(PHY_BMCR, FEC_ETH_PHY, PHY_BMCR_RESET);
            for (i = 0;  i < 10;  i++) {
                phy_ok = phy_read(PHY_BMCR, FEC_ETH_PHY, &phy_state);
                if (!phy_ok) break;
                if (!(phy_state & PHY_BMCR_RESET)) break;
            }
            if (!phy_ok || (phy_state & PHY_BMCR_RESET)) {
                os_printf("FEC: Can't get PHY unit to soft reset: %x\n", phy_state);
                return false;
            }

            fec->iEvent = 0xFFFFFFFF;  // Clear all interrupts
            reset_mode = PHY_BMCR_RESTART;
#ifdef CYGNUM_DEVS_ETH_POWERPC_FEC_LINK_MODE_Auto
            reset_mode |= PHY_BMCR_AUTO_NEG;
#endif
#ifdef CYGNUM_DEVS_ETH_POWERPC_FEC_LINK_MODE_100Mb
            reset_mode |= PHY_BMCR_100MB;
#endif
            phy_write(PHY_BMCR, FEC_ETH_PHY, reset_mode);
            while (phy_timeout-- >= 0) {
                int ev = fec->iEvent;
                unsigned short state;
                fec->iEvent = ev;
                if (ev & iEvent_MII) {
                    phy_ok = phy_read(PHY_BMSR, FEC_ETH_PHY, &state);
                    if (phy_ok && (state & PHY_BMSR_LINK)) {
                        break;
                    } else {
                        CYGACC_CALL_IF_DELAY_US(10000);   // 10ms
                    }
                }
            }
            if (phy_timeout <= 0) {
                os_printf("** FEC Warning: PHY LINK UP failed\n");
            }
        }
        else {
            os_printf("** FEC Info: PHY LINK already UP \n");
        }
    }
#endif // CYGSEM_DEVS_ETH_POWERPC_FEC_RESET_PHY

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, (unsigned char *)&enaddr);
    
    if (cache_state)
      HAL_DCACHE_ENABLE();

    return true;
}
 
//
// This function is called to shut down the interface.
//
static void
fec_eth_stop(struct eth_drv_sc *sc)
{
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;

    // Disable the device!
    qi->fec->eControl &= ~eControl_EN;
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
    // Enable the device!
    fec_eth_reset(sc, enaddr, flags);
}

//
// This function is called for low level "control" operations
//
static int
fec_eth_control(struct eth_drv_sc *sc, unsigned long key,
                  void *data, int length)
{
#ifdef ETH_DRV_SET_MC_ALL
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    volatile struct fec *fec = qi->fec;
#endif

    switch (key) {
    case ETH_DRV_SET_MAC_ADDRESS:
        return 0;
        break;
#ifdef ETH_DRV_SET_MC_ALL
    case ETH_DRV_SET_MC_ALL:
    case ETH_DRV_SET_MC_LIST:
        fec->RxControl &= ~RxControl_PROM;
        fec->hash[0] = 0xFFFFFFFF;
        fec->hash[1] = 0xFFFFFFFF;
        return 0;
        break;
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
fec_eth_can_send(struct eth_drv_sc *sc)
{
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;

    return (qi->txactive < CYGNUM_DEVS_ETH_POWERPC_FEC_TxNUM);
}

//
// This routine is called to send data to the hardware.

static void 
fec_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, 
               int total_len, unsigned long key)
{
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    volatile struct fec_bd *txbd, *txfirst;
    volatile char *bp;
    int i, txindex, cache_state;

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
    // Set up buffer
    bp = txbd->buffer;
    for (i = 0;  i < sg_len;  i++) {
        memcpy((void *)bp, (void *)sg_list[i].buf, sg_list[i].len);
        bp += sg_list[i].len;
    } 
    txbd->length = total_len;
    txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
    qi->txkey[txindex] = key;
    // Note: the MPC8xx does not seem to snoop/invalidate the data cache properly!
    HAL_DCACHE_IS_ENABLED(cache_state);
    if (cache_state) {
        HAL_DCACHE_FLUSH(txbd->buffer, txbd->length);  // Make sure no stale data
    }
    // Send it on it's way
    txbd->ctrl |= FEC_BD_Tx_Ready | FEC_BD_Tx_Last | FEC_BD_Tx_TC;
    qi->txactive++;
    qi->fec->TxUpdate = 0x01000000;  // Any write tells machine to look for work
    set_led(LED_TxACTIVE);
    // Remember the next buffer to try
    if (txbd->ctrl & FEC_BD_Tx_Wrap) {
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
// 'fec_eth_recv' will be called to actually fetch it from the hardware.
//
static void
fec_eth_RxEvent(struct eth_drv_sc *sc)
{
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    volatile struct fec_bd *rxbd, *rxfirst;

    rxbd = rxfirst = qi->rnext;
    while (true) {
        if ((rxbd->ctrl & FEC_BD_Rx_Empty) == 0) {
            qi->rxbd = rxbd;  // Save for callback
            set_led(LED_RxACTIVE);     // Remove the CRC
            (sc->funs->eth_drv->recv)(sc, rxbd->length - 4);
        }
        if (rxbd->ctrl & FEC_BD_Rx_Wrap) {
            rxbd = qi->rbase;
        } else {
            rxbd++;
        }
        if (rxbd == rxfirst) {
            break;
        }
    }
    // Remember where we left off
    qi->rnext = (struct fec_bd *)rxbd;
    qi->fec->RxUpdate = 0x0F0F0F0F;  // Any write tells machine to look for work
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
    int i, cache_state;

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
    }
    qi->rxbd->ctrl |= FEC_BD_Rx_Empty;
    clear_led(LED_RxACTIVE);
}

static void
fec_eth_TxEvent(struct eth_drv_sc *sc)
{
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    volatile struct fec_bd *txbd;
    int key, txindex;

    txbd = qi->tnext;
    // Note: TC field is used to indicate the buffer has/had data in it
    while ((txbd->ctrl & (FEC_BD_Tx_Ready|FEC_BD_Tx_TC)) == FEC_BD_Tx_TC) {
        txindex = ((unsigned long)txbd - (unsigned long)qi->tbase) / sizeof(*txbd);
        if ((key = qi->txkey[txindex]) != 0) {
            qi->txkey[txindex] = 0;
            (sc->funs->eth_drv->tx_done)(sc, key, 0);
        }
	if (--qi->txactive == 0) {
	  clear_led(LED_TxACTIVE);
	}
        txbd->ctrl &= ~FEC_BD_Tx_TC;
        if (txbd->ctrl & FEC_BD_Tx_Wrap) {
            txbd = qi->tbase;
        } else {
            txbd++;
        }
    }
    // Remember where we left off
    qi->tnext = (struct fec_bd *)txbd;
}

//
// Interrupt processing
//
static void          
fec_eth_int(struct eth_drv_sc *sc)
{
    struct fec_eth_info *qi = (struct fec_eth_info *)sc->driver_private;
    unsigned long event;

    while ((event = qi->fec->iEvent) != 0) {
        if ((event & iEvent_TFINT) != 0) {
            fec_eth_TxEvent(sc);
        }
        if ((event & iEvent_RFINT) != 0) {
            fec_eth_RxEvent(sc);
        }
        qi->fec->iEvent = event;  // Reset the bits we handled
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

#if defined(CYGINT_IO_ETH_INT_SUPPORT_REQUIRED) && ~defined(_FEC_USE_INTS)
void
fec_fake_int(cyg_addrword_t param)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *) param;
    int int_state;

    while (true) {
      cyg_thread_delay(1);  // 10ms
      HAL_DISABLE_INTERRUPTS(int_state);
      fec_eth_int(sc);
      HAL_RESTORE_INTERRUPTS(int_state);
    }
}
#endif
