//==========================================================================
//
//      if_upd985xx.c
//
//	Ethernet drivers
//	NEC UPD985XX device ethernet specific support
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
// Author(s):    hmt, gthomas
// Contributors:
// Date:         2001-06-28
// Purpose:      
// Description:  hardware driver for uPD985xx ethernet devices
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_mips_upd985xx.h>
#include <pkgconf/io_eth_drivers.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <net/if.h>  /* Needed for struct ifnet */
#else
#include <cyg/hal/hal_if.h>
#endif

#include <cyg/devs/eth/upd985xx_eth.h>

#include <cyg/io/eth/eth_drv_stats.h>

// ------------------------------------------------------------------------

#ifdef CYGDBG_DEVS_ETH_MIPS_UPD985XX_CHATTER
#define nDEBUG_TRAFFIC	// This one prints stuff as packets come and go
#define nDEBUG_IOCTL	// ioctl() call printing
#define DEBUG      	// Startup printing mainly
#endif

#define os_printf diag_printf
#define db_printf diag_printf

#define STATIC static

// ------------------------------------------------------------------------
// I/O access macros as inlines for later changes to >1 device?
//
// (If we need to do this, then these macros would *assume* the
//  presence of a valid p_eth_upd985xx just like we always have)

static inline void OUTL( volatile cyg_uint32 *io_address, cyg_uint32 value )
{   *io_address = value;   }

static inline cyg_uint32 INL( volatile cyg_uint32 *io_address )
{   return *io_address;    }

// These map cachable addresses to uncachable ones and vice versa.
// This is all fixed on MIPS.  8-9xxxxxxx uncachable, A-Bxxxxxxx cachable.
#define VIRT_TO_BUS( _x_ ) virt_to_bus((cyg_uint32)(_x_))
static inline cyg_uint8 *virt_to_bus(cyg_uint32 p_memory)
{
    return (cyg_uint8 *)(0xa0000000u + (p_memory  & ~0xe0000000u));
}
#define BUS_TO_VIRT( _x_ ) bus_to_virt((cyg_uint32)(_x_))
static inline cyg_uint8 *bus_to_virt(cyg_uint32 p_memory)
{
    return (cyg_uint8 *)(0x80000000u + (p_memory  & ~0xe0000000u));
}


// ------------------------------------------------------------------------
#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_S1
#define FLUSH_WRITES() CYG_MACRO_START          \
    (void) INL( ETH_RXSR );                     \
CYG_MACRO_END
#else
#define FLUSH_WRITES() CYG_EMPTY_STATEMENT
#endif

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------

// 128 bytes extra for VLAN packets should be enough; AFAICT usually the
// encapsulation is only 4 or 10 bytes extra.
#define MAX_ETHERNET_PACKET_SIZE  1536  // Ethernet Rx packet size
#define MAX_OVERSIZE_PACKET_SIZE  1664  // VLAN Rx packet size
#define MAX_RX_PACKET_SIZE        MAX_OVERSIZE_PACKET_SIZE

#define NUM_RXBUFS (8)

// This one is the hardware definition.
struct bufdesc {
    volatile cyg_uint32 attr;
    cyg_uint8 *ptr;
};

// Rx databuffer.
STATIC cyg_uint8 rx_databuf[ NUM_RXBUFS ] [ MAX_RX_PACKET_SIZE ];

#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E3
// Tx databuffer
STATIC cyg_uint8 tx_databuf[ MAX_RX_PACKET_SIZE ];
#endif

struct eth_upd985xx {
    cyg_uint8 active, index, tx_busy, mac_addr_ok;
    cyg_uint8 vector; // interrupt numbers are small
    cyg_uint8 phy_status; // from PHY_STATUS_ flags below
    cyg_uint8 hardwired_esa;
#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2
    cyg_uint8 promisc;
#endif
    cyg_uint8 mac_address[6];

    cyg_handle_t   interrupt_handle;
    cyg_interrupt  interrupt_object;

    struct cyg_netdevtab_entry *ndp;

    // these shall hold uncached addresses of the structures following...
    volatile struct bufdesc *txring;
    volatile struct bufdesc *rxring_active;
    volatile struct bufdesc *rxring_next;
    int rxring_active_index;
    int rxring_next_index;
    cyg_uint32 intrs;
    cyg_uint32 tx_keys[1];

    // -----------------------------------------------------------------
    // Statistics counters
    cyg_uint32 count_rx_resource;
    cyg_uint32 count_rx_restart;
    cyg_uint32 count_interrupts;
    cyg_uint32 count_bad_isr_restarts;
    cyg_uint32 count_bad_tx_completion;

    // -----------------------------------------------------------------
    // DO NOT ACCESS THESE DIRECTLY - THE DEVICE HAS TO SEE THEM UNCACHED

    // Initially, enough for one whole transmission to be described in one go,
    // plus a null link on the end.
    struct bufdesc tx_bufdesc[ MAX_ETH_DRV_SG + 2 ];

    // Pending rx buffers, of full size.
    struct bufdesc rx_bufdesc[ NUM_RXBUFS+1 ];

    // -----------------------------------------------------------------
};

struct eth_upd985xx eth_upd985xx[CYGNUM_DEVS_ETH_MIPS_UPD985XX_DEV_COUNT] = {
    {
        index: 0,
        vector: CYGNUM_HAL_INTERRUPT_ETHER, 
#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2
        promisc: 0,
#endif
#ifdef CYGSEM_DEVS_ETH_UPD985XX_ETH0_SET_ESA
        hardwired_esa: 1,
        mac_address: CYGDAT_DEVS_ETH_UPD985XX_ETH0_ESA,
        mac_addr_ok: 1,
#else
        hardwired_esa: 0,
        mac_addr_ok: 0,
#endif
    }
};

// eth0

ETH_DRV_SC(upd985xx_sc0,
           &eth_upd985xx[0],            // Driver specific data
           CYGDAT_DEVS_ETH_UPD985XX_ETH0_NAME, // name for this interface
           eth_upd985xx_start,
           eth_upd985xx_stop,
           eth_upd985xx_ioctl,
           eth_upd985xx_can_send,
           eth_upd985xx_send,
           eth_upd985xx_recv,
           eth_upd985xx_deliver,
           eth_upd985xx_poll,
           eth_upd985xx_int_vector
    );

NETDEVTAB_ENTRY(upd985xx_netdev0, 
                "upd985xx-" CYGDAT_DEVS_ETH_UPD985XX_ETH0_NAME,
                upd985xx_eth_upd985xx_init, 
                &upd985xx_sc0);


// This is in a macro so that if more devices arrive it can easily be changed
#define CHECK_NDP_SC_LINK() CYG_MACRO_START                              \
    CYG_ASSERT( ((void *)ndp == (void *)&upd985xx_netdev0), "Bad ndp" ); \
    CYG_ASSERT( ((void *)sc == (void *)&upd985xx_sc0), "Bad sc" );       \
    CYG_ASSERT( (void *)p_eth_upd985xx == sc->driver_private,            \
                "sc pointer bad" );                                      \
    CYG_ASSERT( (void *)p_eth_upd985xx == (void *)&eth_upd985xx[0],      \
                "bad p_eth_upd985x" );                                   \
CYG_MACRO_END

#define NUM_ELEMENTS( _x_ ) (sizeof( (_x_) ) / sizeof( (_x_[0]) ) )

// ------------------------------------------------------------------------
//
//                       FUNCTION PROTOTYPES
//
// ------------------------------------------------------------------------
STATIC void InitRxRing(struct eth_upd985xx* p_eth_upd985xx);
STATIC void NextRxRing(struct eth_upd985xx* p_eth_upd985xx);
STATIC void InitTxRing(struct eth_upd985xx* p_eth_upd985xx);
STATIC void ResetTxRing(struct eth_upd985xx* p_eth_upd985xx);

#ifdef CYGPKG_NET
STATIC int eth_upd985xx_configure(struct eth_upd985xx* p_eth_upd985xx,
                                  int promisc, int oversized);
#endif

STATIC void PacketRxReady(struct eth_upd985xx* p_eth_upd985xx);
STATIC void TxDone(struct eth_upd985xx* p_eth_upd985xx);

#define PHY_STATUS_LINK    (1)
#define PHY_STATUS_FDX     (2)
#define PHY_STATUS_100MBPS (4)
STATIC int eth_upd985xx_status( struct eth_upd985xx *p_eth_upd985xx );
STATIC int eth_set_mac_address( struct eth_upd985xx *p_eth_upd985xx, void *data );

// ------------------------------------------------------------------------
//
//                       MII ACCESS TO PHY DEVICE
//
// ------------------------------------------------------------------------

STATIC cyg_bool mii_read( cyg_uint32 reg, cyg_uint32 *pvalue,
                          struct eth_upd985xx *p_eth_upd985xx )
{
    int i = 1000;
    // wait a bit for it to be idle
    while ( 0 != ((ETH_MIND_NVALID | ETH_MIND_SCANA | ETH_MIND_BUSY)
                  & INL(ETH_MIND)) )
        if ( --i < 0 )
            return false;
    // Tell it the register address and PHY address
    OUTL( ETH_MADR, ETH_MADR_PHY_DEVICE_PHYS_ADDRESS | reg );
    OUTL( ETH_MCMD, ETH_MCMD_RSTAT ); // "do a read"
    // wait for the read to complete
    while ( 0 != ((ETH_MIND_NVALID | ETH_MIND_SCANA | ETH_MIND_BUSY)
                  & INL(ETH_MIND)) )
        if ( --i < 0 )
            return false;
    // so get the data
    *pvalue = INL( ETH_MRDD );
    return true;
}

#if 0
STATIC cyg_bool mii_write( cyg_uint32 reg, cyg_uint32 value,
                          struct eth_upd985xx *p_eth_upd985xx )
{
    int i = 1000;
    // wait a bit for it to be idle
    while ( 0 != ((ETH_MIND_NVALID | ETH_MIND_SCANA | ETH_MIND_BUSY)
                  & INL(ETH_MIND)) )
        if ( --i < 0 )
            return false;
    // Tell it the register address and PHY address
    OUTL( ETH_MADR, ETH_MADR_PHY_DEVICE_PHYS_ADDRESS | reg );
    // And write the data:
    OUTL( ETH_MWTD, value );
    // wait a bit for it to be idle
    while ( 0 != ((ETH_MIND_NVALID | ETH_MIND_SCANA | ETH_MIND_BUSY)
                  & INL(ETH_MIND)) )
        if ( --i < 0 )
            return false;
    return true;
}
#endif

// ------------------------------------------------------------------------
//
//                       INTERRUPT HANDLERS
//
// ------------------------------------------------------------------------

STATIC cyg_uint32 eth_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask( vector );

    return CYG_ISR_CALL_DSR;        // schedule DSR
}

// ------------------------------------------------------------------------
// This is a callback from the higher level thread in consequence of the DSR
STATIC void
eth_upd985xx_deliver(struct eth_drv_sc *sc)
{
    register int intrs;
    struct eth_upd985xx *p_eth_upd985xx;
    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;

    p_eth_upd985xx->count_interrupts++;

    intrs = INL( ETH_ISR ); // Read-clear
    // Acknowledge once at the start anyway to prevent an interrupt loop in
    // case of a transient - interrupts latch in the interrupt controller
    // as well as in the ethernet device.
    cyg_drv_interrupt_acknowledge(p_eth_upd985xx->vector);

#ifdef DEBUG_TRAFFIC
    os_printf("\n[[[[[[[ Deliver intrs = %x\n", intrs );
#endif

    // Guard possible external entry points
    if ( ! p_eth_upd985xx->active )
        return; // without unmasking the interrupt

    while ( intrs ) {
        if ( 0xffff0000 & intrs ) {
            // Then something very bad has happened
            p_eth_upd985xx->count_bad_isr_restarts++;
            CYG_ASSERT ( p_eth_upd985xx->active, "Device not active!" );
            eth_upd985xx_stop( sc );
            eth_upd985xx_start( sc, NULL, 0 );
            intrs = INL( ETH_ISR ); // Read-clear
        }
        p_eth_upd985xx->intrs = intrs;
        if ( ( ETH_ISR_XMTDN | ETH_ISR_TABR ) & intrs ) {
            // Scan for completed Txen and inform the stack
            TxDone(p_eth_upd985xx);
        }
        if ( ( ETH_ISR_RCVDN | ETH_ISR_RBDRS | ETH_ISR_RBDRU ) & intrs ) {
            // Pass any rx data up the stack
            PacketRxReady(p_eth_upd985xx);
        }
        // Now we have made the interrupt causes go away, acknowledge and
        // *then* read the ISR again.  That way the race can result in a
        // spurious interrupt rather than a lost interrupt.
        cyg_drv_interrupt_acknowledge(p_eth_upd985xx->vector);
        intrs = INL( ETH_ISR ); // Read-clear
#ifdef DEBUG_TRAFFIC
        if ( intrs )
            os_printf("------- Again intrs = %x\n", intrs );
#endif
    }
#ifdef DEBUG_TRAFFIC
    os_printf("]]]]]]]] Done intrs = %x\n\n", intrs );
#endif

    cyg_drv_interrupt_unmask(p_eth_upd985xx->vector);
}

// ------------------------------------------------------------------------
// Device table entry to operate the chip in a polled mode.
// Only diddle the interface we were asked to!

STATIC void
eth_upd985xx_poll(struct eth_drv_sc *sc)
{
    struct eth_upd985xx *p_eth_upd985xx;

    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;

    // As it happens, this driver always requests the DSR to be called:
    (void)eth_isr( p_eth_upd985xx->vector, (cyg_addrword_t)sc );
    eth_upd985xx_deliver( sc );
}

// ------------------------------------------------------------------------
// Determine interrupt vector used by a device - for attaching GDB stubs
// packet handler.
STATIC int
eth_upd985xx_int_vector(struct eth_drv_sc *sc)
{
    struct eth_upd985xx *p_eth_upd985xx;
    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;
    return (p_eth_upd985xx->vector);
}

// ------------------------------------------------------------------------

STATIC int
eth_set_mac_address( struct eth_upd985xx *p_eth_upd985xx, void *data )
{
    cyg_uint8 *p = (cyg_uint8 *)data;
    cyg_uint8 *mac_address;

    mac_address = &p_eth_upd985xx->mac_address[0];

    mac_address[5] = p[5];
    mac_address[4] = p[4];
    mac_address[3] = p[3];
    mac_address[2] = p[2];
    mac_address[1] = p[1];
    mac_address[0] = p[0];

    p_eth_upd985xx->mac_addr_ok = 1;

    // Set the ESA in the device regs
    OUTL( ETH_LSA2,
          (p_eth_upd985xx->mac_address[1]) |
          (p_eth_upd985xx->mac_address[0] << 8 ) );
    OUTL( ETH_LSA1,
          (p_eth_upd985xx->mac_address[5]) |
          (p_eth_upd985xx->mac_address[4] << 8 ) |
          (p_eth_upd985xx->mac_address[3] << 16 ) |
          (p_eth_upd985xx->mac_address[2] << 24) );

    return 0; // OK
}

// ------------------------------------------------------------------------

STATIC void
eth_upd985xx_reset( struct eth_upd985xx *p_eth_upd985xx )
{
    int i;

    // Reset whole device: Software Reset (clears automatically)
    OUTL( ETH_CCR, ETH_CCR_SRT );
    for ( i = 0; i < 10000; i++ ) /* nothing */;
    // Reset internal units
    OUTL( ETH_MACC2, ETH_MACC2_MCRST | ETH_MACC2_RFRST | ETH_MACC2_TFRST );
    for ( i = 0; i < 10000; i++ ) /* nothing */;
    FLUSH_WRITES();
    OUTL( ETH_MACC2, 0 ); // (and release reset)
    // Enable CRC adding, padding
    FLUSH_WRITES();
    OUTL( ETH_MACC1,
          ETH_MACC1_CRCEN | ETH_MACC1_PADEN |
          ETH_MACC1_TXFC | ETH_MACC1_RXFC | ETH_MACC1_PARF );
    FLUSH_WRITES();
    OUTL( ETH_MACC2, ETH_MACC2_APD ); // Auto VLAN pad
    FLUSH_WRITES();
    OUTL( ETH_HT1, 0 );
    FLUSH_WRITES();
    OUTL( ETH_HT2, 0 );

    // Enable rx of broadcasts, multicasts, but not promiscuous...
    FLUSH_WRITES();
#if defined( CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2 ) && \
    !defined( CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2_E2ONLY )
    // Unless we are faking it.
    OUTL( ETH_AFR, ETH_AFR_ABC | ETH_AFR_PRM | ETH_AFR_PRO );
#else
    OUTL( ETH_AFR, ETH_AFR_ABC | ETH_AFR_PRM );
#endif

    FLUSH_WRITES();
    OUTL( ETH_IPGT, 0x00000013 );
    FLUSH_WRITES();
    OUTL( ETH_IPGR, 0x00000e13 );
    FLUSH_WRITES();
    OUTL( ETH_CLRT, 0x0000380f );
    FLUSH_WRITES();
    OUTL( ETH_LMAX, MAX_ETHERNET_PACKET_SIZE );

    // Select a clock for the MII
    FLUSH_WRITES();
    OUTL( ETH_MIIC, ETH_MIIC_66 ); // Example code sets to 66.
    // Set VLAN type reg
    FLUSH_WRITES();
    OUTL( ETH_VLTP, ETH_VLTP_VLTP );

    // Set the ESA in the device regs
    if ( p_eth_upd985xx->mac_addr_ok ) {
        FLUSH_WRITES();
        OUTL( ETH_LSA2,
              (p_eth_upd985xx->mac_address[1]) |
              (p_eth_upd985xx->mac_address[0] << 8 ) );
        FLUSH_WRITES();
        OUTL( ETH_LSA1,
              (p_eth_upd985xx->mac_address[5]) |
              (p_eth_upd985xx->mac_address[4] << 8 ) |
              (p_eth_upd985xx->mac_address[3] << 16 ) |
              (p_eth_upd985xx->mac_address[2] << 24) );
    }

    FLUSH_WRITES();
    OUTL( ETH_RXFCR, ETH_RXFCR_UWM_DEFAULT |
                     ETH_RXFCR_LWM_DEFAULT | ETH_RXFCR_DRTH16W ); 

    FLUSH_WRITES();
    // Fault E4 - use only 32 for FLTH, not the previously recommended 48 (words)
    // Tag: CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E4
    // but no config opt is provided.
    OUTL( ETH_TXFCR, ETH_TXFCR_TPTV_DEFAULT |
                     ETH_TXFCR_TX_DRTH_DEFAULT | (32 << ETH_TXFCR_TX_FLTH_SHIFT) );

    // Transmit and receive config regs we hit when enabling those
    // functions separately, and the wet string end of the receiver
    // which is controlled by   MACC1 |= ETH_MACC1_SRXEN;

    // Tx and Rx interrupts enabled internally; 
    // Tx done/aborted, and rx OK.
    FLUSH_WRITES();
    OUTL( ETH_MSR, ETH_ISR_XMTDN | ETH_ISR_TABR | 
                   ETH_ISR_RCVDN | ETH_ISR_RBDRS | ETH_ISR_RBDRU );
    FLUSH_WRITES();
}

// ------------------------------------------------------------------------
//
//                NETWORK INTERFACE INITIALIZATION
//
// ------------------------------------------------------------------------
STATIC bool
upd985xx_eth_upd985xx_init(struct cyg_netdevtab_entry * ndp)
{
    struct eth_drv_sc *sc;
    cyg_uint8 *mac_address;
    struct eth_upd985xx *p_eth_upd985xx;

#ifdef DEBUG
    db_printf("upd985xx_eth_upd985xx_init\n");
#endif

    sc = (struct eth_drv_sc *)(ndp->device_instance);
    p_eth_upd985xx = (struct eth_upd985xx *)(sc->driver_private);

    CHECK_NDP_SC_LINK();

    p_eth_upd985xx->tx_busy = 0;

    // record the net dev pointer
    p_eth_upd985xx->ndp = (void *)ndp;

    mac_address = &p_eth_upd985xx->mac_address[0];

#ifdef CYGSEM_DEVS_ETH_UPD985XX_ETH0_GET_EEPROM_ESA
    if ( ! p_eth_upd985xx->hardwired_esa ) {
        cyg_uint8 *p;
        union macar {
            struct {
                cyg_uint32 macar1, macar2, macar3;
            } integers;
            cyg_uint8 bytes[12];
        } eeprom;

        eeprom.integers.macar1 = INL( MACAR1 );  // MAC Address Register 1
        eeprom.integers.macar2 = INL( MACAR2 );  // MAC Address Register 2
        eeprom.integers.macar3 = INL( MACAR3 );  // MAC Address Register 3

        if ( (0 != eeprom.integers.macar1 ||
              0 != eeprom.integers.macar2 ||
              0 != eeprom.integers.macar3 )
             && 
             (0xffffffff != eeprom.integers.macar1 ||
              0xffffffff != eeprom.integers.macar2 ||
              0xffffffff != eeprom.integers.macar3 ) ) {
            // Then we have good data in the EEPROM
#ifdef DEBUG
            os_printf( "EEPROM data %08x %08x %08x\n", 
                       eeprom.integers.macar1,
                       eeprom.integers.macar2,
                       eeprom.integers.macar3 );
#endif
            p = &eeprom.bytes[0]; // pick up either set of ESA info
            if ( 1 == p_eth_upd985xx->index )
                p += 6;

            mac_address[5] = p[5];
            mac_address[4] = p[4];
            mac_address[3] = p[3];
            mac_address[2] = p[2];
            mac_address[1] = p[1];
            mac_address[0] = p[0];
            p_eth_upd985xx->mac_addr_ok = 1;
        }
        else {
            // Fake it so we can get RedBoot going on a board with no EEPROM
            mac_address[0] = 0;
            mac_address[1] = 0xBA;
            mac_address[2] = 0xD0;
            mac_address[3] = 0xEE;
            mac_address[4] = 0x00;
            mac_address[5] = p_eth_upd985xx->index;
            p_eth_upd985xx->mac_addr_ok = 1;
        }
    }
#endif // CYGSEM_DEVS_ETH_UPD985XX_ETH0_GET_EEPROM_ESA
    
    // Init the underlying hardware and insert the ESA:
    eth_upd985xx_reset(p_eth_upd985xx);

#ifdef DEBUG
    os_printf("MAC Address %s, ESA = %02X %02X %02X %02X %02X %02X\n",
              p_eth_upd985xx->mac_addr_ok ? "OK" : "**BAD**", 
              mac_address[0], mac_address[1], mac_address[2], mac_address[3],
              mac_address[4], mac_address[5]);
#endif

    // Set up the pointers to data structures
    InitTxRing(p_eth_upd985xx);

    // Construct the interrupt handler
    p_eth_upd985xx->active = 0;
    cyg_drv_interrupt_acknowledge(p_eth_upd985xx->vector);
    cyg_drv_interrupt_mask(p_eth_upd985xx->vector);
    cyg_drv_interrupt_create(
        p_eth_upd985xx->vector,
        0,                              // Priority - unused
        (CYG_ADDRWORD)sc,               // Data item passed to ISR & DSR
        eth_isr,                        // ISR
        eth_drv_dsr,                    // DSR (generic)
        &p_eth_upd985xx->interrupt_handle, // handle to intr obj
        &p_eth_upd985xx->interrupt_object ); // space for int obj

    cyg_drv_interrupt_attach(p_eth_upd985xx->interrupt_handle);

    // Initialize upper level driver
    if ( p_eth_upd985xx->mac_addr_ok )
        (sc->funs->eth_drv->init)(sc, &(p_eth_upd985xx->mac_address[0]) );
    else
        (sc->funs->eth_drv->init)(sc, 0 );
    
    return (1);
}

// ------------------------------------------------------------------------
//
//  Function : eth_upd985xx_start
//
// ------------------------------------------------------------------------
STATIC void
eth_upd985xx_start( struct eth_drv_sc *sc,
                    unsigned char *enaddr, int flags )
{
    struct eth_upd985xx *p_eth_upd985xx;
    cyg_uint32 ss; 
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif

    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;

#ifdef DEBUG
    os_printf("eth_upd985xx_start %d flg %x\n", p_eth_upd985xx->index, *(int *)p_eth_upd985xx );
#endif

    if ( p_eth_upd985xx->active )
        eth_upd985xx_stop( sc );

    p_eth_upd985xx->active = 1;
    
#ifdef CYGPKG_NET
    /* Enable promiscuous mode if requested, reception of oversized frames always.
     * The latter is needed for VLAN support and shouldn't hurt even if we're not
     * using VLANs.
     */
    eth_upd985xx_configure(p_eth_upd985xx, !!(ifp->if_flags & IFF_PROMISC), 1);
#endif

    // renegotiate link status
    p_eth_upd985xx->phy_status = eth_upd985xx_status( p_eth_upd985xx );

    if ( p_eth_upd985xx->phy_status & PHY_STATUS_FDX ) {
        cyg_uint32 ss;
        // then enable full duplex in the MAC
        ss = INL( ETH_MACC1 );
        ss |= ETH_MACC1_FDX;
        OUTL( ETH_MACC1, ss );
    }

#ifdef DEBUG
    {
        int status = p_eth_upd985xx->phy_status;
        os_printf("eth_upd985xx_start %d Link = %s, %s Mbps, %s Duplex\n",
                  p_eth_upd985xx->index,
                  status & PHY_STATUS_LINK ? "Up" : "Down",
                  status & PHY_STATUS_100MBPS ?  "100" : "10",
                  status & PHY_STATUS_FDX ? "Full" : "Half"
            );
    }
#endif


    // Start the receive engine
    p_eth_upd985xx->count_rx_restart++;
    // Initialize all but one buffer: [B0,B1,B2,...Bx,NULL,LINK]
    InitRxRing( p_eth_upd985xx );
    // Point the hardware at the list of buffers
    OUTL( ETH_RXDPR, (cyg_uint32)p_eth_upd985xx->rxring_active );
    // Tell it about the buffers via the rx descriptor count
    OUTL( ETH_RXPDR, ETH_RXPDR_AL | (NUM_RXBUFS-1) );
    // Ack any pending interrupts from the system
    p_eth_upd985xx->intrs = INL( ETH_ISR ); // Read-clear
    // Start the rx.
    OUTL( ETH_RXCR, ETH_RXCR_RXE | ETH_RXCR_DRBS_16 );

    // Enable the wet string end of the receiver
    ss = INL( ETH_MACC1 );
    ss |= ETH_MACC1_SRXEN;
    OUTL( ETH_MACC1, ss );

    // And unmask the interrupt
    cyg_drv_interrupt_acknowledge(p_eth_upd985xx->vector);
    cyg_drv_interrupt_unmask(p_eth_upd985xx->vector);
}

// ------------------------------------------------------------------------
//
//  Function : eth_upd985xx_status; 10/100 and Full/Half Duplex (FDX/HDX)
//
// ------------------------------------------------------------------------
STATIC int eth_upd985xx_status( struct eth_upd985xx *p_eth_upd985xx )
{
    int status;
    int i, j;
    // Some of these bits latch and only reflect "the truth" on a 2nd reading.
    // So read and discard.
    mii_read( PHY_CONTROL_REG, &i, p_eth_upd985xx );
    mii_read( PHY_STATUS_REG, &i, p_eth_upd985xx );
    // Use the "and" of the local and remote capabilities words to infer
    // what is selected:
    status = 0;
    if ( mii_read( PHY_STATUS_REG, &i, p_eth_upd985xx ) ) {
        if ( PHY_STATUS_LINK_OK & i )
            status |= PHY_STATUS_LINK;
    }
    if ( mii_read( PHY_AUTONEG_ADVERT, &j, p_eth_upd985xx ) && 
         mii_read( PHY_AUTONEG_REMOTE, &i, p_eth_upd985xx ) ) {
#if defined( DEBUG_TRAFFIC ) || defined( DEBUG_IOCTL )
        os_printf( "MII: capabilities are %04x, %04x; common %04x\n",
                   i, j, i & j );
#endif
        j &= i; // select only common capabilities

        if ( (PHY_AUTONEG_100BASET4 |
              PHY_AUTONEG_100BASETX_FDX |
              PHY_AUTONEG_100BASETX_HDX)  & j )
            status |= PHY_STATUS_100MBPS;
        if ( (PHY_AUTONEG_100BASETX_FDX | PHY_AUTONEG_10BASET_FDX) & j )
            status |= PHY_STATUS_FDX;
    }
    return status;
}

// ------------------------------------------------------------------------
//
//  Function : eth_upd985xx_stop
//
// ------------------------------------------------------------------------

STATIC void eth_upd985xx_stop( struct eth_drv_sc *sc )
{
    struct eth_upd985xx *p_eth_upd985xx;

    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;

    // No more interrupts
    cyg_drv_interrupt_acknowledge(p_eth_upd985xx->vector);
    cyg_drv_interrupt_mask(p_eth_upd985xx->vector);

#ifdef DEBUG
    os_printf("eth_upd985xx_stop %d flg %x\n", p_eth_upd985xx->index, *(int *)p_eth_upd985xx );
#endif

    p_eth_upd985xx->active = 0;         // stop people tormenting it

    if ( p_eth_upd985xx->tx_busy ) {
        // Then it is finshed now, by force:
        cyg_uint32 key = p_eth_upd985xx->tx_keys[ 0 ];
        // Turn off the transmitter (before the callback to the stack).
        OUTL( ETH_TXCR, 0 );
#ifdef DEBUG_TRAFFIC
        os_printf("Stop: tidying up TX, KEY %x\n", key );
#endif
        // Leave tx_busy true so no recursion can occur here.
        // Then tell the stack we are done:
        if ( key ) {
            (sc->funs->eth_drv->tx_done)( sc, key, 0 );
        }
    }
    p_eth_upd985xx->tx_keys[ 0 ] = 0;
    p_eth_upd985xx->tx_busy = p_eth_upd985xx->active = 0;

    eth_upd985xx_reset(p_eth_upd985xx);

    ResetTxRing( p_eth_upd985xx );
}


// ------------------------------------------------------------------------
//
//  Function : InitRxRing
//
// ------------------------------------------------------------------------
STATIC void InitRxRing(struct eth_upd985xx* p_eth_upd985xx)
{
    int i;
    struct bufdesc *bp;

    // first just blat the various flags and addresses: the first N
    // bufdescs point to data buffers, the last one is NULL.
    bp = (struct bufdesc *)VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[0] );
    // Record the initial active buffer:
    p_eth_upd985xx->rxring_active = bp;
    p_eth_upd985xx->rxring_active_index = 0;
    for ( i = 0; i < NUM_RXBUFS - 1; i++, bp++ ) {
        bp->ptr = VIRT_TO_BUS( &rx_databuf[i][0] );
        bp->attr = ( ETH_BUF_D_L_DATA | ETH_BUF_OWN_CPU
                     | (ETH_BUF_SIZE & sizeof( rx_databuf[0] )) );
    }
    CYG_ASSERT( i == NUM_RXBUFS-1, "Penultimate rx buffer index mismatch" );
    CYG_ASSERT( (cyg_uint8 *)bp == 
                VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[NUM_RXBUFS-1] ),
                "Penultimate rx buffer address mismatch" );

    // NULL out the penultimate one
    bp->ptr = NULL;
    bp->attr = 0;
    // And record it as next one to use
    p_eth_upd985xx->rxring_next = bp;
    p_eth_upd985xx->rxring_next_index = NUM_RXBUFS-1;

    // Step on to the extra entry at the end which makes a ring:
    bp++;
    CYG_ASSERT( (cyg_uint8 *)bp == 
                VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[NUM_RXBUFS] ),
                "Ultimate rx buffer address mismatch" );

    // Link the Ultimate back to the start
    bp->ptr = (cyg_uint8 *)p_eth_upd985xx->rxring_active; // Zeroth entry
    bp->attr = ETH_BUF_D_L_LINK;

    // All done.
}


// ------------------------------------------------------------------------
//
//  Function : NextRxRing
//
// ------------------------------------------------------------------------

STATIC void NextRxRing(struct eth_upd985xx* p_eth_upd985xx )
{
    volatile struct bufdesc *next, *dead;
    int iactive;
    int inext;

    iactive = p_eth_upd985xx->rxring_active_index;
    inext = p_eth_upd985xx->rxring_next_index;

    // Preconditions:
    CYG_ASSERT( 0 <=   inext && inext   < NUM_RXBUFS, "Bad inext" );
    CYG_ASSERT( 0 <= iactive && iactive < NUM_RXBUFS, "Bad iactive" );
    CYG_ASSERT( VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[ inext ] )
                == (cyg_uint8 *)p_eth_upd985xx->rxring_next, "Next rx_bufdesc bad" );
    CYG_ASSERT( VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[ iactive ] )
                == (cyg_uint8 *)p_eth_upd985xx->rxring_active, "Active rx_bufdesc bad" );
    CYG_ASSERT( ETH_BUF_D_L_LINK == p_eth_upd985xx->rxring_next->attr, "Next not a link" );
    CYG_ASSERT( ETH_BUF_D_L_DATA & p_eth_upd985xx->rxring_active->attr, "Active not data" );
    CYG_ASSERT( NULL == p_eth_upd985xx->rxring_next->ptr, "Next not NULL" );
    CYG_ASSERT( VIRT_TO_BUS( &rx_databuf[iactive][0] ) ==
                p_eth_upd985xx->rxring_active->ptr, "Active bad data pointer" );
    CYG_ASSERT( (iactive - 1 == inext) || (0 == iactive && NUM_RXBUFS - 1 == inext),
                "Chasing pointers mismatch" );

    // Select the new bufdesc to be active - ie. next to scan for reception:
    if ( ++iactive >= NUM_RXBUFS )
        iactive = 0;
    dead = p_eth_upd985xx->rxring_active; // the one that just died
    // Step ahead the new active buffer:
    p_eth_upd985xx->rxring_active = (volatile struct bufdesc *)
        VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[ iactive ] );
    p_eth_upd985xx->rxring_active_index = iactive;

    // Blow away the currently active entry; we have dealt with it already
    // and it is needed for an end stop to the ring:
    dead->ptr = NULL;
    dead->attr = 0;
   
    // Select the next bufdesc to enliven
    next = p_eth_upd985xx->rxring_next;
    next->ptr = VIRT_TO_BUS( &rx_databuf[inext][0] );
    next->attr = ( ETH_BUF_D_L_DATA | ETH_BUF_OWN_CPU
                   | (ETH_BUF_SIZE & sizeof( rx_databuf[0] )) );

    // And update the external info to reflect this:
    if ( ++inext >= NUM_RXBUFS )
        inext = 0;
    next = (volatile struct bufdesc *)
        VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[ inext ] );

    p_eth_upd985xx->rxring_next = next;
    p_eth_upd985xx->rxring_next_index = inext;

    // Postconditions:
    CYG_ASSERT( 0 <=   inext && inext   < NUM_RXBUFS, "Bad inext" );
    CYG_ASSERT( 0 <= iactive && iactive < NUM_RXBUFS, "Bad iactive" );
    CYG_ASSERT( VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[ inext ] )
                == (cyg_uint8 *)p_eth_upd985xx->rxring_next, "Next rx_bufdesc bad" );
    CYG_ASSERT( VIRT_TO_BUS( &p_eth_upd985xx->rx_bufdesc[ iactive ] )
                == (cyg_uint8 *)p_eth_upd985xx->rxring_active, "Active rx_bufdesc bad" );
    CYG_ASSERT( ETH_BUF_D_L_LINK == p_eth_upd985xx->rxring_next->attr, "Next not a link" );
    CYG_ASSERT( ETH_BUF_D_L_DATA & p_eth_upd985xx->rxring_active->attr, "Active not data" );
    CYG_ASSERT( NULL == p_eth_upd985xx->rxring_next->ptr, "Next not NULL" );
    CYG_ASSERT( VIRT_TO_BUS( &rx_databuf[iactive][0] ) ==
                p_eth_upd985xx->rxring_active->ptr, "Active bad data pointer" );
    CYG_ASSERT( (iactive - 1 == inext) || (0 == iactive && NUM_RXBUFS - 1 == inext),
                "Chasing pointers mismatch" );
}

// ------------------------------------------------------------------------
//
//  Function : PacketRxReady        (Called from delivery thread)
//
// ------------------------------------------------------------------------
STATIC void PacketRxReady(struct eth_upd985xx* p_eth_upd985xx)
{
    struct cyg_netdevtab_entry *ndp;
    struct eth_drv_sc *sc;
    cyg_uint32 ss, length;
    cyg_bool reset_required = 0;

    ndp = (struct cyg_netdevtab_entry *)(p_eth_upd985xx->ndp);
    sc = (struct eth_drv_sc *)(ndp->device_instance);

    CHECK_NDP_SC_LINK();

#ifdef DEBUG_TRAFFIC
    ss = INL( ETH_RXSR );
    os_printf("PacketRxReady: RXSR %x\n", ss );
#endif

    if ( ETH_ISR_RBDRU & p_eth_upd985xx->intrs )
        reset_required = 1; // Out of buffers
    if ( ! ETH_ISR_RCVDN & p_eth_upd985xx->intrs )
        reset_required = 1; // or if no reception completed, reset anyway

    // For all ready rx blocks...
    do {
        volatile struct bufdesc *bp;

        bp = p_eth_upd985xx->rxring_active; // Current rx candidate

        ss = bp->attr;
#ifdef DEBUG_TRAFFIC
        os_printf("PacketRxReady attr %x at %x\n", ss, bp );
#endif
        if ( ETH_BUF_OWN_CPU == (ETH_BUF_OWN & ss) ) {
            // Then the packet is untouched...
            break;
        }
#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2
        // Perform address recognition by hand, hardware is in promisc mode
        // (we have settable "software" promisc mode too of course)
        if ( ETH_BUF_OK & ss ) {
            cyg_uint8 *esa = (cyg_uint8 *)bp->ptr; // (this is a non-cachable address)
            int ok = 0;
            if ( p_eth_upd985xx->promisc )
                ok = 1; // accept the packet
            else
            if ( p_eth_upd985xx->mac_address[0] == esa[0] &&
                 p_eth_upd985xx->mac_address[1] == esa[1] &&
                 p_eth_upd985xx->mac_address[2] == esa[2] &&
                 p_eth_upd985xx->mac_address[3] == esa[3] &&
                 p_eth_upd985xx->mac_address[4] == esa[4] &&
                 p_eth_upd985xx->mac_address[5] == esa[5] )
                ok = 1; // Then they are equal - accept
            else
            if ( 0xff == esa[0] &&
                 0xff == esa[1] &&
                 0xff == esa[2] &&
                 0xff == esa[3] &&
                 0xff == esa[4] &&
                 0xff == esa[5] )
                ok = 1; // Then they are equal - accept

            if ( !ok )
                ss = 0; // Easiest way...
        }
#endif
        if ( ETH_BUF_OK & ss ) {
            length = ETH_BUF_SIZE & ss;
#ifdef DEBUG_TRAFFIC
            os_printf("PacketRxReady found a packet size %d attr %x\n", length, ss );
#endif
            // Asserts for the length in-range can fire, with good status
            // in the block, so be defensive here instead.  Belt and braces.
            if ( 63 < length && length <= MAX_RX_PACKET_SIZE ) {
                CYG_ASSERT( ETH_BUF_D_L_DATA == (ETH_BUF_D_L & ss), "Not data buffer" );
                CYG_ASSERT( length > 63, "Tiny packet" );
                CYG_ASSERT( length <= MAX_RX_PACKET_SIZE, "Too big packet" );
                CYG_ASSERT( ETH_BUF_LAST & ss, "Not last buffer" );
                (sc->funs->eth_drv->recv)( sc, length );
            } // Else drop it on the floor.
        }

        // Step along to the next buffer descriptor...
        NextRxRing( p_eth_upd985xx );
        if ( ! reset_required ) {
            // And tell the device it can have a biscuit:
            OUTL( ETH_RXPDR, ETH_RXPDR_AL | 1 );

            // Now, before moving on to the next packet, find out if receptions
            // had caught up with us before adding that new buffer:
            ss = INL( ETH_RXPDR );
            ss &= ETH_RXPDR_RNOD;
            ss >>= ETH_RXPDR_RNOD_SHIFT;
            if ( 1 >= ss ) {
                // Then it was zero before.  So the rx engine is stopped.
#ifdef DEBUG_TRAFFIC
                os_printf( "***ZERO rx buffers were left\n" );
#endif
                reset_required = 1;
            }
            // Otherwise we carry on as usual.
        }
    } while ( 1 );

    if ( reset_required ) {
        p_eth_upd985xx->count_rx_resource++;
        // Disable the wet string end of the receiver
        ss = INL( ETH_MACC1 );
        ss &=~ETH_MACC1_SRXEN;
        OUTL( ETH_MACC1, ss );
        // Disable the DMA engine
        OUTL( ETH_RXCR, 0 );
        // Reset the RxRing from scratch
        InitRxRing( p_eth_upd985xx );
        // Point the hardware at the list of buffers
        OUTL( ETH_RXDPR, (cyg_uint32)p_eth_upd985xx->rxring_active );
        // Tell it about the buffers via the rx descriptor count:
        ss = INL( ETH_RXPDR );
        ss &= ETH_RXPDR_RNOD;
        ss >>= ETH_RXPDR_RNOD_SHIFT;
        // This awful register *increments* by what you write, even if the
        // machinery is halted.  Vile filthy evil rubbish.
        OUTL( ETH_RXPDR, ETH_RXPDR_AL | ((NUM_RXBUFS-1) - ss)  );
        ss = INL( ETH_RXPDR );
        CYG_ASSERT( (ETH_RXPDR_AL | (NUM_RXBUFS-1)) == ss, "RXPDR not right" );
        // Start the rx.
        OUTL( ETH_RXCR, ETH_RXCR_RXE | ETH_RXCR_DRBS_16 );
        // Enable the wet string end of the receiver
        ss = INL( ETH_MACC1 );
        ss |= ETH_MACC1_SRXEN;
        OUTL( ETH_MACC1, ss );
        // All done.
#ifdef DEBUG_TRAFFIC
        os_printf( "***Rx Machine restarted\n" );
#endif
    }
}

// and the callback function

STATIC void
eth_upd985xx_recv( struct eth_drv_sc *sc,
                   struct eth_drv_sg *sg_list, int sg_len )
{
    struct eth_upd985xx *p_eth_upd985xx;
    int total_len;
    struct eth_drv_sg *last_sg;
    cyg_uint8 *from_p;
    volatile struct bufdesc *bp;

    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;
    
    // Guard possible external entry points
    if ( ! p_eth_upd985xx->active )
        return;

    bp = p_eth_upd985xx->rxring_active; // Current rx candidate

#ifdef DEBUG_TRAFFIC
    os_printf("Rx status %x\n", bp->attr );
#endif

    if ( 0 == (ETH_BUF_OK & bp->attr) )
        return;
        
    total_len = ETH_BUF_SIZE & bp->attr;
    
#ifdef DEBUG_TRAFFIC
    os_printf("Rx %d %x (status %x): %d sg's, %d bytes\n",
              p_eth_upd985xx->index, (int)p_eth_upd985xx,
              bp->attr,
              sg_len, total_len);
#endif

    // Copy the data to the network stack
    from_p = bp->ptr; // (this is a non-cachable address)

    // check we have memory to copy into; we would be called even if
    // caller was out of memory in order to maintain our state.
    if ( 0 == sg_len || 0 == sg_list )
        return; // caller was out of mbufs

    CYG_ASSERT( 0 < sg_len, "sg_len underflow" );
    CYG_ASSERT( MAX_ETH_DRV_SG >= sg_len, "sg_len overflow" );

    for ( last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++ ) {
        cyg_uint8 *to_p;
        int l;
            
        to_p = (cyg_uint8 *)(sg_list->buf);
        l = sg_list->len;

        CYG_ASSERT( 0 <= l, "sg length -ve" );

        if ( 0 >= l || 0 == to_p )
            return; // caller was out of mbufs

        if ( l > total_len )
            l = total_len;

        memcpy( to_p, from_p, l );
        from_p += l;
        total_len -= l;
    }

    CYG_ASSERT( 0 == total_len, "total_len mismatch in rx" );
    CYG_ASSERT( last_sg == sg_list, "sg count mismatch in rx" );
    CYG_ASSERT( bp->ptr < from_p, "from_p wild in rx" );
    CYG_ASSERT( bp->ptr + MAX_RX_PACKET_SIZE >= from_p,
                "from_p overflow in rx" );
}    


// ------------------------------------------------------------------------
//
//  Function : InitTxRing
//
// ------------------------------------------------------------------------
STATIC void InitTxRing(struct eth_upd985xx* p_eth_upd985xx)
{
    int i;
    volatile struct bufdesc *bp;
    
    p_eth_upd985xx->txring =
        (struct bufdesc *)VIRT_TO_BUS( &p_eth_upd985xx->tx_bufdesc[0] );

    bp = p_eth_upd985xx->txring;

    for ( i = 0; i < NUM_ELEMENTS( p_eth_upd985xx->tx_bufdesc ); i++, bp++ ) {
        bp->ptr = NULL;
        bp->attr = 0;
    }
    // Last one is a NULL link
    bp--;
    bp->ptr = NULL;
    bp->attr = ETH_BUF_D_L_LINK;

    ResetTxRing(p_eth_upd985xx);
}

// ------------------------------------------------------------------------
//
//  Function : ResetTxRing
//
// ------------------------------------------------------------------------
STATIC void ResetTxRing(struct eth_upd985xx* p_eth_upd985xx)
{
    int i;
    volatile struct bufdesc *bp;
    bp = p_eth_upd985xx->txring;
    for ( i = 0; i < NUM_ELEMENTS( p_eth_upd985xx->tx_bufdesc ) - 1; i++, bp++ ) {
        bp->attr =
            ETH_BUF_LAST |
            ETH_BUF_D_L_DATA |
            ETH_BUF_OWN_CPU |
            (ETH_BUF_SIZE & 0);
    }
}

// ------------------------------------------------------------------------
//
//  Function : TxDone          (Called from delivery thread)
//
// This returns Tx's from the Tx Machine to the stack (ie. reports
// completion) - allowing for missed interrupts, and so on.
// ------------------------------------------------------------------------

STATIC void TxDone(struct eth_upd985xx* p_eth_upd985xx)
{
    struct cyg_netdevtab_entry *ndp;
    struct eth_drv_sc *sc;

    ndp = (struct cyg_netdevtab_entry *)(p_eth_upd985xx->ndp);
    sc = (struct eth_drv_sc *)(ndp->device_instance);

    CHECK_NDP_SC_LINK();

    if ( p_eth_upd985xx->tx_busy ) {
        cyg_uint32 ss;

        ss = INL( ETH_TXSR ); // Get tx status
        if ( ss & (ETH_TXSR_CSE |
                   ETH_TXSR_TUDR |
                   ETH_TXSR_TGNT |
                   ETH_TXSR_LCOL |
                   ETH_TXSR_ECOL |
                   ETH_TXSR_TEDFR |
                   ETH_TXSR_TDFR |
                   ETH_TXSR_TBRO |
                   ETH_TXSR_TMUL |
                   ETH_TXSR_TDONE ) ) {
            // Then it finished; somehow...
            cyg_uint32 key = p_eth_upd985xx->tx_keys[ 0 ];
            
            // Turn off the transmitter (before the callback to the stack).
            OUTL( ETH_TXCR, 0 );

#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E8
            // Must take action after certain types of tx failure:
            if ( ss & (ETH_TXSR_TUDR |
                       ETH_TXSR_LCOL |
                       ETH_TXSR_ECOL) ) {
                p_eth_upd985xx->count_bad_tx_completion++;
                CYG_ASSERT ( p_eth_upd985xx->active, "Device not active!" );
                eth_upd985xx_stop( sc );
                eth_upd985xx_start( sc, NULL, 0 );
                key = 0; // Important!  Stop above already fed it back.
            }
#endif

#ifdef DEBUG_TRAFFIC
            os_printf("TxDone %d %x: KEY %x\n",
                      p_eth_upd985xx->index, (int)p_eth_upd985xx, key );
#endif
            // Finished, ready for the next one
            p_eth_upd985xx->tx_keys[ 0 ] = 0;
            p_eth_upd985xx->tx_busy = 0;
            // Then tell the stack we are done:
            if (key) {
                (sc->funs->eth_drv->tx_done)( sc, key,
                       0 == (p_eth_upd985xx->intrs & ETH_ISR_TABR) );
            }
        }
    }
}


// ------------------------------------------------------------------------
//
//  Function : eth_upd985xx_can_send
//
// ------------------------------------------------------------------------

STATIC int 
eth_upd985xx_can_send(struct eth_drv_sc *sc)
{
    struct eth_upd985xx *p_eth_upd985xx;

    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;

    // Guard possible external entry points
    if ( ! p_eth_upd985xx->active )
        return 0;

    return ! p_eth_upd985xx->tx_busy;
}

// ------------------------------------------------------------------------
//
//  Function : eth_upd985xx_send
//
// ------------------------------------------------------------------------

STATIC void 
eth_upd985xx_send(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list, int sg_len, int total_len,
            unsigned long key)
{
    struct eth_upd985xx *p_eth_upd985xx;
    struct eth_drv_sg *last_sg;
    volatile struct bufdesc *bp;
#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E3
    struct eth_drv_sg local_sg[2];
#endif

    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;

#ifdef DEBUG_TRAFFIC
    os_printf("Tx %d %x: %d sg's, %d bytes, KEY %x\n",
              p_eth_upd985xx->index, (int)p_eth_upd985xx, sg_len, total_len, key );
#endif

    if ( ! p_eth_upd985xx->active )
        return;                         // device inactive, no return
    
    CYG_ASSERT( ! p_eth_upd985xx->tx_busy, "Can't send when busy!" );

    p_eth_upd985xx->tx_busy++;

    p_eth_upd985xx->tx_keys[0] = key;
    bp = &p_eth_upd985xx->txring[0]; // Current free tx
    CYG_ASSERT( 0 < sg_len, "sg_len underflow" );
    CYG_ASSERT( MAX_ETH_DRV_SG >= sg_len, "sg_len overflow" );

#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E3
    // We must copy any Tx that is more than two SGs into just one buffer.
    if ( sg_len > 2 ) {
        cyg_uint8 *from_p, *to_p;
        to_p = &tx_databuf[0]; // normal cached address
        if ( sizeof( tx_databuf ) < total_len )
            total_len = sizeof( tx_databuf );
        for ( last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++ ) {
            int l;

            from_p = (cyg_uint8 *)(sg_list->buf); // normal cached address
            l = sg_list->len;
           
            if ( l > total_len )
                l = total_len;

            memcpy( to_p, from_p, l ); // All in cached memory
            to_p += l;
            total_len -= l;

            if ( 0 > total_len ) 
                break; // Should exit via sg_last normally
        }

        // Set up SGs describing the single tx buffer
        total_len = to_p - &tx_databuf[0];
        local_sg[0].buf = (CYG_ADDRESS)&tx_databuf[0];
        local_sg[0].len = (CYG_ADDRWORD)total_len;
        local_sg[1].buf = (CYG_ADDRESS)0;
        local_sg[1].len = (CYG_ADDRWORD)0;

        // And make the subsequent code use it.
        sg_len = 1;
        sg_list = &local_sg[0];
    }
#endif

    for ( last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++ ) {
        cyg_uint8 *from_p;
        int l;
            
        from_p = (cyg_uint8 *)(sg_list->buf); // normal cached address
        l = sg_list->len;

        if ( l > total_len )
            l = total_len;

        // Ensure the mbuf contents really is in RAM where DMA can see it.
        // (Must round to cache lines apparantly for 4120)
        HAL_DCACHE_STORE( ((CYG_ADDRESS)from_p) &~(HAL_DCACHE_LINE_SIZE-1),
                          l + HAL_DCACHE_LINE_SIZE );
        
        bp->ptr = VIRT_TO_BUS( from_p ); // uncached real RAM address
        bp->attr &=~(ETH_BUF_LAST | ETH_BUF_SIZE);
        bp->attr |= ETH_BUF_SIZE & l;
        bp->attr |= ETH_BUF_D_L_DATA;

        total_len -= l;
        bp++;

        if ( 0 > total_len ) 
            break; // Should exit via sg_last normally
    }

    CYG_ASSERT( bp > &p_eth_upd985xx->txring[0], "bp underflow" );
    CYG_ASSERT( bp < &p_eth_upd985xx->txring[
        NUM_ELEMENTS(p_eth_upd985xx->tx_bufdesc)
        ], "bp underflow" );

    bp--;
    bp->attr |= ETH_BUF_LAST;

    // Make the rest be null links
    for ( bp++; bp <
              &p_eth_upd985xx->txring[NUM_ELEMENTS(p_eth_upd985xx->tx_bufdesc)];
          bp++ ) {
        bp->attr = ETH_BUF_D_L_LINK;
        bp->ptr = NULL;
    }

    CYG_ASSERT( 0 == total_len, "length mismatch in tx" );
    CYG_ASSERT( last_sg == sg_list, "sg count mismatch in tx" );

    // And start off the tx system

    // Point the hardware at the list of buffers
    OUTL( ETH_TXDPR, (cyg_uint32)p_eth_upd985xx->txring );
    // and start the tx.

    // Fault E4 - use only 8 for DTBS, not the previously recommended 16.
    // Tag: CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E4
    // but no config opt is provided.

    // Fault E7: ETH_TXCR_AFCE must not be used.
    // Tag: CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E7
    // but no config opt is provided.

    OUTL( ETH_TXCR, ETH_TXCR_TXE | ETH_TXCR_DTBS_8 /* | ETH_TXCR_AFCE */ );
}

#ifdef CYGPKG_NET
// ------------------------------------------------------------------------
//
//  Function : eth_upd985xx_configure
//
//  Return : 0 = It worked.
//           non0 = It failed.
// ------------------------------------------------------------------------

STATIC int
eth_upd985xx_configure(struct eth_upd985xx* p_eth_upd985xx, int promisc, int oversized)
{
    int ss;

    // We implement permission of oversize packets by changing LMAX (rather
    // than enabling HUGEN in ETH_MACC1) because we rely on only one
    // reception per rx descriptor.  General oversize packets could eat
    // many rx descriptors and we would become ...confused.

    // Sanity check the numbers we're about to use.
    CYG_ASSERT( sizeof( rx_databuf[0] ) >= MAX_OVERSIZE_PACKET_SIZE,
                "Oversize packet would overflow rx buffer" );
    CYG_ASSERT( sizeof( rx_databuf[0] ) >= MAX_ETHERNET_PACKET_SIZE,
                "Ethernet packet would overflow rx buffer" );
    if ( oversized )
        OUTL( ETH_LMAX, MAX_OVERSIZE_PACKET_SIZE );
    else
        OUTL( ETH_LMAX, MAX_ETHERNET_PACKET_SIZE );

#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2
    ss = promisc ? 1 : 0; // avoid unused var warning
    p_eth_upd985xx->promisc = ss;
#ifdef CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2_E2ONLY
    // Then we must also set the mode in the chip
    ss = INL( ETH_AFR );
    if ( promisc )
        ss |= ETH_AFR_PRO;
    else
        ss &=~ETH_AFR_PRO;
    OUTL( ETH_AFR, ss );
#endif // CYGOPT_DEVS_ETH_MIPS_UPD985XX_HARDWARE_BUGS_E1E2_E2ONLY
#else
    ss = INL( ETH_AFR );
    if ( promisc )
        ss |= ETH_AFR_PRO;
    else
        ss &=~ETH_AFR_PRO;
    OUTL( ETH_AFR, ss );
#endif
    return 0; // OK
}
#endif

// ------------------------------------------------------------------------
//
//  Function : eth_upd985xx_ioctl
//
// ------------------------------------------------------------------------
STATIC int 
eth_upd985xx_ioctl(struct eth_drv_sc *sc, unsigned long key,
                   void *data, int data_length)
{
    struct eth_upd985xx *p_eth_upd985xx;

    p_eth_upd985xx = (struct eth_upd985xx *)sc->driver_private;

#ifdef DEBUG_IOCTL
    db_printf( "eth_upd985xx_ioctl: device eth%d at %x; key is 0x%x, data at %x[%d]\n",
               p_eth_upd985xx->index, p_eth_upd985xx, key, data, data_length );
#endif

    // DO NOT guard possible external entry points - want to be able eg. to
    // set a mac address of a down interface before bringing it up!

    switch ( key ) {

#ifdef ETH_DRV_SET_MAC_ADDRESS
    case ETH_DRV_SET_MAC_ADDRESS:
        if ( 6 != data_length )
            return -2;
        return eth_set_mac_address( p_eth_upd985xx, data );
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
        int i;

        // Chipset entry is no longer supported; RFC1573.
        for ( i = 0; i < SNMP_CHIPSET_LEN; i++ )
            p->snmp_chipset[i] = 0;

        // This perhaps should be a config opt, so you can make up your own
        // description, or supply it from the instantiation.
        strcpy( p->description, "NEC uPD985xx on-chip ethernet (CANDY)" );
        // CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

        i = eth_upd985xx_status( p_eth_upd985xx );

        if ( !( i & PHY_STATUS_LINK) ) {
            p->operational = 2;         // LINK DOWN
            p->duplex = 1;              // UNKNOWN
            p->speed = 0;
        }
        else {
            p->operational = 3;            // LINK UP
            p->duplex = (i & PHY_STATUS_FDX) ? 3 : 2; // 2 = SIMPLEX, 3 = DUPLEX
            p->speed = ((i & PHY_STATUS_100MBPS) ? 100 : 10) * 1000000;
        }

        // Admit to it...
        p->supports_dot3        = true;

        // Those commented out are not available on this chip.
        p->tx_good              = INL( ETH_TPCT   )   ;
        p->tx_max_collisions    = INL( ETH_TXCL   )   ;
        p->tx_late_collisions   = INL( ETH_TLCL   )   ;
        //p->tx_underrun          = INL(    )   ;
        p->tx_carrier_loss      = INL( ETH_TCSE   )   ;
        p->tx_deferred          = INL( ETH_TDFR   ) +
                                  INL( ETH_TXDF   )    ;
        //p->tx_sqetesterrors     = INL(    )   ;
        p->tx_single_collisions = INL( ETH_TSCL   )   ;
        p->tx_mult_collisions   = INL( ETH_TMCL   )   ;
        p->tx_total_collisions  = INL( ETH_TSCL   ) +
                                  INL( ETH_TMCL   ) +
                                  INL( ETH_TLCL   ) +
                                  INL( ETH_TXCL   )   ;
        p->rx_good              = INL( ETH_RPKT   )   ;
        p->rx_crc_errors        = INL( ETH_RFCS   )   ;
        p->rx_align_errors      = INL( ETH_RALN   )   ;
        p->rx_resource_errors   = p_eth_upd985xx->count_rx_resource;
        //p->rx_overrun_errors    = INL(    )   ;
        //p->rx_collisions        = INL(    )   ;
        p->rx_short_frames      = INL( ETH_RUND   )   ;
        p->rx_too_long_frames   = INL( ETH_ROVR   )   ;
        p->rx_symbol_errors     = INL( ETH_RXUO   )   ;

        p->interrupts           = p_eth_upd985xx->count_interrupts;
        p->rx_count             = INL( ETH_RBYT   )   ;
        p->rx_deliver           = INL( ETH_RPKT   )   ;
        p->rx_resource          = p_eth_upd985xx->count_rx_resource;
        p->rx_restart           = p_eth_upd985xx->count_rx_resource +
                                  p_eth_upd985xx->count_rx_restart;
        p->tx_count             = INL( ETH_TBYT   )   ;
        p->tx_complete          = INL( ETH_TPCT   )   ;
        p->tx_dropped           = INL( ETH_TNCL   )   ;
        
        p->tx_queue_len = 1;
        
        return 0; // OK
    }
#endif

    default:
        break;
    }
    return -1;
}

// ------------------------------------------------------------------------

// EOF if_upd985xx.c
