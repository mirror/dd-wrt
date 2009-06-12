//==========================================================================
//
//      if_i21143.c
//
//	Intel 21143 ethernet driver
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
// Portions of this software may have been derived from OpenBSD or
// other sources, and are covered by the appropriate copyright
// disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: 
// Date:         2000-09-17
// Purpose:      
// Description:  hardware driver for 21143 Intel PRO/100+ ethernet
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_intel_i21143.h>

// Config for the instantiating package:
#include CYGDAT_DEVS_ETH_INTEL_I21143_CFG

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>          // HAL_DCACHE_STORE
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <net/if.h>  /* Needed for struct ifnet */
#endif

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci.h>
#include CYGHWR_MEMORY_LAYOUT_H
#else
#error "Need PCI package here"
#endif

// ------------------------------------------------------------------------
// Exported statistics and the like
// defined in <cyg/io/eth/eth_drv_stats.h> - already included.

#define KEEP_STATISTICS

#ifdef KEEP_STATISTICS
# define INCR_STAT( _x_ ) (p_i21143->stats. _x_ ++)
#else
# define INCR_STAT( _x_ ) CYG_EMPTY_STATEMENT
#endif

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------

#define RX_DESCRIPTORS (4) // 4 packets.
#define RX_BUFFERS     RX_DESCRIPTORS
#define TX_DESCRIPTORS MAX_ETH_DRV_SG // Enough for one tx, even if fragmented

#define MAX_RX_PACKET_SIZE  1536        // maximum Rx packet size
#define MAX_TX_PACKET_SIZE  1536        // maximum Tx packet size

// ------------------------------------------------------------------------

typedef struct buffer_descriptor {
    volatile cyg_uint32 des0;
    volatile cyg_uint32 des1;
    volatile cyg_uint32 buf1;
    volatile cyg_uint32 buf2;
} BUFDES;


typedef struct i21143 {
    cyg_uint8                           // (split up for atomic byte access)
        found:1,                        // was hardware discovered?
        mac_addr_ok:1,                  // can we bring up?
        active:1,                       // has this if been brung up?
        hardwired_esa:1,                // set if ESA is hardwired via CDL
        spare1:4; 
    cyg_uint8 tx_endbuf;                // transmit in progress flag too
#define NO_TX_IN_PROGRESS (255)
    cyg_uint8  index;                   // 0 or 1 or whatever
    cyg_uint8  line_status;             // line status when last inspected
    cyg_uint32 devid;                   // PCI device id
    cyg_uint32 io_address;              // memory mapped I/O address
    cyg_uint8  mac_address[6];          // mac (hardware) address
    cyg_uint16 phy_autoneg_remote;      // remote link status cache
    void *ndp;                          // Network Device Pointer
    
    int next_rx_descriptor;             // descriptor index for buffers
    volatile BUFDES *rx_ring;           // location of Rx descriptors

    volatile BUFDES *tx_ring;           // location of Tx descriptors
    unsigned long tx_keys[1];           // keys for tx q management

    // Interrupt handling stuff
    cyg_vector_t    vector;             // interrupt vector
    cyg_handle_t    interrupt_handle;   // handle for int.handler
    cyg_interrupt   interrupt_object;

    // Refer to following items only though uncached addresses from the
    // tx_ring and rxring pointers:
    volatile BUFDES _tx[TX_DESCRIPTORS];
    volatile BUFDES _rx[RX_DESCRIPTORS];

    cyg_uint8 _rxbufs[ RX_BUFFERS ] [ MAX_RX_PACKET_SIZE ] ;

#ifdef KEEP_STATISTICS
    struct stats {
        unsigned int tx_good;
        unsigned int tx_max_collisions;
        unsigned int tx_late_collisions;
        unsigned int tx_underrun;
        unsigned int tx_carrier_loss;
        unsigned int tx_deferred;
        unsigned int tx_sqetesterrors;
        unsigned int tx_single_collisions;
        unsigned int tx_mult_collisions;
        unsigned int tx_total_collisions;

        unsigned int rx_good;
        unsigned int rx_crc_errors;
        unsigned int rx_align_errors;
        unsigned int rx_resource_errors;
//        unsigned int rx_overrun_errors;
        unsigned int rx_collisions;
        unsigned int rx_short_frames;
        unsigned int rx_too_long_frames;
        unsigned int rx_symbol_errors;

        unsigned int interrupts;
        unsigned int rx_count;
        unsigned int rx_deliver;
        unsigned int rx_resource;
        unsigned int rx_restart;

        unsigned int tx_count;
        unsigned int tx_complete;
        unsigned int tx_dropped;
    } stats;
#endif // KEEP_STATISTICS
} I21143;

// ------------------------------------------------------------------------
// After defining that, now we can instantiate the device(s):


#include CYGDAT_DEVS_ETH_INTEL_I21143_INL

// ------------------------------------------------------------------------
// Access to cached/uncached/physical memory, to map from CPU-view
// addresses to PCI-bus master's view - however that is:
// 
// We'll use these macros, so that different platforms can define them
// externally:
//  o CYGHWR_PCI_VIRT_TO_BUS( x )
//    - address for the PCI device to use
//  o CYGHWR_CACHED_TO_UNCACHED( x )
//    - access to memory that the PCI device will look at
//  o CYGHWR_BUS_TO_UNCACHED( x )
//    - back from an address in the PCI structures to one for us to look at
//      to see what it said.

#ifndef CYGHWR_PCI_VIRT_TO_BUS
# ifdef CYGARC_PHYSICAL_ADDRESS
#  define CYGHWR_PCI_VIRT_TO_BUS( _x_ ) CYGARC_PHYSICAL_ADDRESS( (_x_) )
# else
#  error No CYGHWR_PCI_VIRT_TO_BUS() defined!
//#define CYGHWR_PCI_VIRT_TO_BUS( _x_ )  (_x_)
# endif
#endif

#ifndef CYGHWR_CACHED_TO_UNCACHED
# ifdef CYGARC_UNCACHED_ADDRESS
#  define CYGHWR_CACHED_TO_UNCACHED( _x_ ) CYGARC_UNCACHED_ADDRESS( (_x_) )
# else   
#  define CYGHWR_CACHED_TO_UNCACHED( _x_ )  (_x_)
# endif
#endif

#ifndef CYGHWR_BUS_TO_UNCACHED
# ifdef CYGARC_UNCACHED_ADDRESS
#  define CYGHWR_BUS_TO_UNCACHED( _x_ ) CYGARC_UNCACHED_ADDRESS( (_x_) )
# else   
#  define CYGHWR_BUS_TO_UNCACHED( _x_ )  (_x_)
# endif
#endif

// ------------------------------------------------------------------------
// Prototypes and declarations

static int pci_init_find_21143s( void );
static int i21143_configure(struct i21143* p_i21143, int promisc);
static void i21143_reset(struct i21143* p_i21143);
static void InitTxRing(struct i21143* p_i21143);
static void InitRxRing(struct i21143* p_i21143);
static void eth_set_mac_address( struct i21143* p_i21143, cyg_uint8 *addr );

static void mii_write_register( int ioaddr, int regnum, int value );
static int mii_read_register( int ioaddr, int regnum );

static int get_eeprom_size( int ioaddr );
static int read_eeprom_word( int ioaddr, int addrbits, int address );

// ------------------------------------------------------------------------
//
//                   21143 GENERAL STATUS REGISTER
//
// ------------------------------------------------------------------------
#define GEN_STATUS_FDX          0x04    // 1 = full duplex, 0 = half
#define GEN_STATUS_100MBPS      0x02    // 1 = 100 Mbps, 0 = 10 Mbps
#define GEN_STATUS_LINK         0x01    // 1 = link up, 0 = link down

// returns status in terms of the above:
static int i21143_status( struct i21143* p_i21143 );

// A cheap check for changes: true if changed, is all:
static int i21143_status_changed( struct i21143* p_i21143 );

// ------------------------------------------------------------------------

#define CYGDAT_DEVS_ETH_DESCRIPTION "Intel i21143 10/100 Ethernet [DEC]"

#define ETH_DEV_DOT3STATSETHERCHIPSET 1,3,6,1,2,1,10,7,8,2,5

// ------------------------------------------------------------------------

#ifdef CYGDBG_DEVS_ETH_INTEL_I21143_CHATTER
#define nDEBUG_TRAFFIC  // This one prints stuff as packets come and go
#define DEBUG          // Startup printing mainly
#define nDEBUG_EE       // Some EEPROM specific retries &c
#define nDEBUG_TRAFFIC_TXDETAILS // tx bufs layout
#define nDEBUG_DUMP_REGS
#define nDEBUG_MAC
#define nDEBUG_STARTSTOPRESET
#endif

// ------------------------------------------------------------------------
// I/O access macros as inlines for type safety

#if (CYG_BYTEORDER == CYG_MSBFIRST)

#define HAL_CTOLE32(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))
#define HAL_LE32TOC(x)  ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24) & 0xff))

#define HAL_CTOLE16(x)  ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define HAL_LE16TOC(x)  ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))

//static inline void OUTB(cyg_uint8 value, cyg_uint32 io_address)
//{
//    HAL_WRITE_UINT8( io_address, value);
//}
//
//static inline void OUTW(cyg_uint16 value, cyg_uint32 io_address)
//{
//    HAL_WRITE_UINT16( io_address, (((value & 0xff) << 8) | ((value & 0xff00) >> 8)) );
//}

static inline void OUTL(cyg_uint32 value, cyg_uint32 io_address)
{
    HAL_WRITE_UINT32( io_address,
                      ((((value) & 0xff) << 24) | (((value) & 0xff00) << 8) | (((value) & 0xff0000) >> 8) | (((value) >> 24) & 0xff)) );
}

//static inline cyg_uint8 INB(cyg_uint32 io_address)
//{   
//    cyg_uint8 d;
//    HAL_READ_UINT8( io_address, d );
//    return d;
//}
//
//static inline cyg_uint16 INW(cyg_uint32 io_address)
//{
//    cyg_uint16 d;
//    HAL_READ_UINT16( io_address, d );
//    return (((d & 0xff) << 8) | ((d & 0xff00) >> 8));
//}

static inline cyg_uint32 INL(cyg_uint32 io_address)
{
    cyg_uint32 d;
    HAL_READ_UINT32( io_address, d );
    return ((((d) & 0xff) << 24) | (((d) & 0xff00) << 8) | (((d) & 0xff0000) >> 8) | (((d) >> 24) & 0xff));
}
#else

// Maintaining the same styleee as above...
#define HAL_CTOLE32(x)  ((((x))))
#define HAL_LE32TOC(x)  ((((x))))

#define HAL_CTOLE16(x)  ((((x))))
#define HAL_LE16TOC(x)  ((((x))))

//static inline void OUTB(cyg_uint8  value, cyg_uint32 io_address)
//{   HAL_WRITE_UINT8( io_address, value );   }
//
//static inline void OUTW(cyg_uint16 value, cyg_uint32 io_address)
//{   HAL_WRITE_UINT16( io_address, value );   }

static inline void OUTL(cyg_uint32 value, cyg_uint32 io_address)
{   HAL_WRITE_UINT32( io_address, value );   }

//static inline cyg_uint8  INB(cyg_uint32 io_address)
// {   cyg_uint8  _t_; HAL_READ_UINT8(  io_address, _t_ ); return _t_;   }
//
//static inline cyg_uint16 INW(cyg_uint32 io_address)
// {   cyg_uint16 _t_; HAL_READ_UINT16( io_address, _t_ ); return _t_;   }

static inline cyg_uint32 INL(cyg_uint32 io_address)
 {   cyg_uint32 _t_; HAL_READ_UINT32( io_address, _t_ ); return _t_;   }

#endif // byteorder

// ------------------------------------------------------------------------

static inline void udelay(int delay)
{
    CYGACC_CALL_IF_DELAY_US(delay);
}

// ------------------------------------------------------------------------
#ifdef DEBUG_DUMP_REGS
#define MII 1
#define ETH 2
#define BOTH 3
void debug_dump_regs( cyg_uint32 ioaddr, int which )
{
    int i;

    if ( MII & which )
        for ( i = 0; i <= 0x19 ; i++ ) {
            int v;
            if ( 0x08 == i ) i = 0x10;
            v = mii_read_register( ioaddr, i );
            diag_printf( "MII/PHY register %2x%c = %04x\n", i, 'h', v );
        }
    if ( ETH & which )
        for ( i = 0; i < 16; i++ ) {
            cyg_uint32 v;
            v = INL( (ioaddr + (i << 3)) );
            diag_printf( "21143 register CSR%2d (at %2x%c) = %08x\n", i, (i << 3), 'h', v );
        }
}
#endif
// ------------------------------------------------------------------------
//
//               REGISTERS (CSRs) and THEIR BITS
//
// ------------------------------------------------------------------------

// All registers are 32bit entities on 64bit boundaries.

#define CSR0         (ioaddr + ( 0 << 3))
#define CSR1         (ioaddr + ( 1 << 3))
#define CSR2         (ioaddr + ( 2 << 3))
#define CSR3         (ioaddr + ( 3 << 3))
#define CSR4         (ioaddr + ( 4 << 3))
#define CSR5         (ioaddr + ( 5 << 3))
#define CSR6         (ioaddr + ( 6 << 3))
#define CSR7         (ioaddr + ( 7 << 3))
#define CSR8         (ioaddr + ( 8 << 3))
#define CSR9         (ioaddr + ( 9 << 3))
#define CSR10        (ioaddr + (10 << 3))
#define CSR11        (ioaddr + (11 << 3))
#define CSR12        (ioaddr + (12 << 3))
#define CSR13        (ioaddr + (13 << 3))
#define CSR14        (ioaddr + (14 << 3))
#define CSR15        (ioaddr + (15 << 3))

#define CSR1_PM CSR1
#define CSR2_PM CSR2

#define CSR_BUSMODE          CSR0 
#define CSR_TXPOLL           CSR1 
#define CSR_RXPOLL           CSR2 
#define CSR_RXBASE           CSR3 
#define CSR_TXBASE           CSR4 
#define CSR_STATUS           CSR5 
#define CSR_OPMODE           CSR6 
#define CSR_INTR_ENABLE      CSR7 
#define CSR_MISS_OFL_COUNT   CSR8 
#define CSR_ROM_MII_MGT      CSR9 
#define CSR_ROM_PGM_ADDR     CSR10
#define CSR_INTR_MITIGATION  CSR11
#define CSR_SIA_STATUS       CSR12
#define CSR_SIA_CONN         CSR13
#define CSR_SIA_TXRX         CSR14
#define CSR_SIA_GPPORT       CSR15

#define CSR_WUFILTER         CSR1_PM
#define CSR_WUEVENTS         CSR2_PM

// -------- BUSMODE
#define CSR_BUSMODE_PM            (1<<26)
#define CSR_BUSMODE_WIE           (1<<24)
#define CSR_BUSMODE_RLE           (1<<23)
#define CSR_BUSMODE_RME           (1<<21)
#define CSR_BUSMODE_DBO_BE        (1<<20)
#define CSR_BUSMODE_DBO_LE        (  0  )
#define CSR_BUSMODE_TAP_SHIFT (17)
#define CSR_BUSMODE_CAL_SHIFT (14)
#define CSR_BUSMODE_PBL_SHIFT ( 8)
#define CSR_BUSMODE_ENDIAN_BE     (1<< 7)
#define CSR_BUSMODE_ENDIAN_LE     (  0  )
#define CSR_BUSMODE_DSL_SHIFT ( 2)
#define CSR_BUSMODE_BAR           (1<< 1)
#define CSR_BUSMODE_RESET         (1<< 0)

// -------- STATUS
#define CSR_STATUS_LC             (1<<27)
#define CSR_STATUS_GPI            (1<<26)
#define CSR_STATUS_EB             (7 << 23)

#define CSR_STATUS_TXSTATUS       (7 << 20)
#define CSR_STATUS_TXSTATUS_STOPPED       (0 << 20)
#define CSR_STATUS_TXSTATUS_SUSPENDED     (6 << 20)

#define CSR_STATUS_RXSTATUS       (7 << 17)
#define CSR_STATUS_RXSTATUS_STOPPED       (0 << 17)
#define CSR_STATUS_RXSTATUS_SUSPENDED     (4 << 17)

#define CSR_STATUS_NIS            (1<<16)
#define CSR_STATUS_AIS            (1<<15)
#define CSR_STATUS_ERI            (1<<14)
#define CSR_STATUS_FBE            (1<<13)
#define CSR_STATUS_LNF            (1<<12)
#define CSR_STATUS_GTE            (1<<11)
#define CSR_STATUS_ETI            (1<<10)
#define CSR_STATUS_RWT            (1<< 9)
#define CSR_STATUS_RX_STOPPED     (1<< 8)
#define CSR_STATUS_RBU            (1<< 7)
#define CSR_STATUS_RX_INTR        (1<< 6)
#define CSR_STATUS_UNF            (1<< 5)
#define CSR_STATUS_LNP_ANC        (1<< 4)
#define CSR_STATUS_TJT            (1<< 3)
#define CSR_STATUS_TBU            (1<< 2)
#define CSR_STATUS_TX_STOPPED     (1<< 1)
#define CSR_STATUS_TX_INTR        (1<< 0)

// -------- OPMODE

#define CSR_OPMODE_SC          (1<<31)
#define CSR_OPMODE_RA          (1<<30)
#define CSR_OPMODE_IGNOREMSB   (1<<26)
#define CSR_OPMODE_MUST_BE_ONE (1<<25)
#define CSR_OPMODE_SCR         (1<<24)
#define CSR_OPMODE_PCS         (1<<23)
#define CSR_OPMODE_TTM         (1<<22)
#define CSR_OPMODE_SF          (1<<21)
#define CSR_OPMODE_HBD         (1<<19)
#define CSR_OPMODE_PS          (1<<18)
#define CSR_OPMODE_PS_MIISYM        (1<<18)
#define CSR_OPMODE_CA          (1<<17)
#define CSR_OPMODE_TX_THRES_SHIFT (14)
#define CSR_OPMODE_TX_START    (1<<13)
#define CSR_OPMODE_FC          (1<<12)
#define CSR_OPMODE_LOOPBACK_SHIFT (10)
#define CSR_OPMODE_FD          (1<< 9)
#define CSR_OPMODE_MULTICAST   (1<< 7)
#define CSR_OPMODE_PROMISC     (1<< 6)
#define CSR_OPMODE_SB          (1<< 5)
#define CSR_OPMODE_IF          (1<< 4)
#define CSR_OPMODE_PB          (1<< 3)
#define CSR_OPMODE_HO          (1<< 2)
#define CSR_OPMODE_RX_START    (1<< 1)
#define CSR_OPMODE_HP          (1<< 0)

// -------- INTR_ENABLE
// uses the same bits as STATUS, hurrah!

// -------- MISS_OFL_COUNT
#define CSR_MISS_OFL_COUNT_NO_RXBUFS_MASK  (0x0001ffff)
#define CSR_MISS_OFL_COUNT_NO_RXBUFS_SHIFT ( 0)
#define CSR_MISS_OFL_COUNT_NO_RXFIFO_MASK  (0x1ffe0000)
#define CSR_MISS_OFL_COUNT_NO_RXFIFO_SHIFT (17)

// -------- ROM_MII_MGT
#define CSR_ROM_MII_MGT_MDI   (1<<19)
#define CSR_ROM_MII_MGT_MOM   (1<<18)
#define CSR_ROM_MII_MGT_MOM_READ   (1<<18)
#define CSR_ROM_MII_MGT_MDO   (1<<17)
#define CSR_ROM_MII_MGT_MDC   (1<<16)
#define CSR_ROM_MII_MGT_RD    (1<<14)
#define CSR_ROM_MII_MGT_WR    (1<<13)
#define CSR_ROM_MII_MGT_BR    (1<<12)
#define CSR_ROM_MII_MGT_SR    (1<<11)
#define CSR_ROM_MII_MGT_REG   (1<<10)
#define CSR_ROM_MII_MGT_SR_DO (1<< 3)
#define CSR_ROM_MII_MGT_SR_DI (1<< 2)
#define CSR_ROM_MII_MGT_SR_CK (1<< 1)
#define CSR_ROM_MII_MGT_SR_CS (1<< 0)

#define CSR_ROM_MII_MGT_DATA  (0xff)

// -------- SIA defaults
#define CSR_SIA_CONN_DEFAULT   (0)
#define CSR_SIA_TXRX_DEFAULT   (0)
#define CSR_SIA_GPPORT_DEFAULT (8)

// ------------------------------------------------------------------------
//
//               RECEIVE/TRANSMIT FRAME DESCRIPTORS
//
// ------------------------------------------------------------------------


//#if (CYG_BYTEORDER == CYG_MSBFIRST)

// Common to both Rx and Tx DES0
#define DES0_STATUS_OWN          (1<<31)
#define DES0_STATUS_OWN_DONE     (0<<31)
#define DES0_STATUS_OWN_OPEN     (1<<31)

#define DES0_STATUS_ERROR        (1<<15)

// Rx specific results:
#define RDES0_STATUS_FF          (1<<30)
#define RDES0_STATUS_DE          (1<<14)
#define RDES0_STATUS_RF          (1<<11)
#define RDES0_STATUS_MF          (1<<10)
#define RDES0_STATUS_FIRST       (1<< 9)
#define RDES0_STATUS_LAST        (1<< 8)
#define RDES0_STATUS_TOOLONG     (1<< 7)
#define RDES0_STATUS_LATECOLL    (1<< 6)
#define RDES0_STATUS_FT          (1<< 5)
#define RDES0_STATUS_RW          (1<< 4)
#define RDES0_STATUS_RE          (1<< 3)
#define RDES0_STATUS_DB          (1<< 2)
#define RDES0_STATUS_CRC         (1<< 1)
#define RDES0_STATUS_ZERO        (1<< 0)

#define RDES0_COUNT_MASK     (0x3fff0000)      
#define RDES0_COUNT_SHIFT    ( 16 )      

// Tx specific results:
#define TDES0_STATUS_TJTO        (1<<14)
#define TDES0_STATUS_LO          (1<<11)
#define TDES0_STATUS_NC          (1<<10)
#define TDES0_STATUS_LC          (1<< 9)
#define TDES0_STATUS_EC          (1<< 8)
#define TDES0_STATUS_HF          (1<< 7)
#define TDES0_STATUS_LF          (1<< 2)
#define TDES0_STATUS_UFL         (1<< 1)
#define TDES0_STATUS_DE          (1<< 0)
#define TDES0_STATUS_CC_MASK     (0x78) 
#define TDES0_STATUS_CC_SHIFT    ( 3 )

// Common to both Rx and Tx DES1
#define DES1_ENDRING             (1<<25)
#define DES1_2ACHAIN             (1<<24)
#define DES1_B2SIZE_MASK         (0x7ff << 11)
#define DES1_B2SIZE_SHIFT        (11)
#define DES1_B1SIZE_MASK         (0x7ff)
#define DES1_B1SIZE_SHIFT        (0)

// Rx DES1 has no special bits - Rx descriptors have no "controls"

// Tx specific controls in DES1:
#define TDES1_CONTROL_INTERRUPT  (1<<31)
#define TDES1_CONTROL_LAST       (1<<30)
#define TDES1_CONTROL_FIRST      (1<<29)
#define TDES1_CONTROL_SETUP_FT1  (1<<28)
#define TDES1_CONTROL_SETUP      (1<<27)
#define TDES1_CONTROL_NO_CRC     (1<<26)
#define TDES1_CONTROL_NO_PAD     (1<<23)
#define TDES1_CONTROL_SETUP_FT0  (1<<22)

// ------------------------------------------------------------------------
//
// PHY common constants - registers are read over MII.
// 
// I don't know how much they have in common, but I think MII is pretty
// standard, and the "mandated" registers ought to be common.

#define PHY_CONTROL_REG     (0)
#define PHY_STATUS_REG      (1)
#define PHY_ID_ONE          (2)
#define PHY_ID_TWO          (3)
#define PHY_AUTONEG_ADVERT  (4)
#define PHY_AUTONEG_REMOTE  (5)

#define PHY_CONTROL_RESET           (1<<15)
#define PHY_CONTROL_LOOPBACK        (1<<14)
#define PHY_CONTROL_SPEED100        (1<<13)
#define PHY_CONTROL_AUTONEG_EN      (1<<12)
#define PHY_CONTROL_POWERDOWN       (1<<11)
#define PHY_CONTROL_MII_DIS         (1<<10)
#define PHY_CONTROL_AUTONEG_RST     (1<< 9)
#define PHY_CONTROL_DPLX_FULL       (1<< 8)
#define PHY_CONTROL_COLLTEST        (1<< 7)
  
#define PHY_STATUS_CAP_T4           (1<<15)
#define PHY_STATUS_CAP_100TXF       (1<<14)
#define PHY_STATUS_CAP_100TXH       (1<<13)
#define PHY_STATUS_CAP_10TF         (1<<12)
#define PHY_STATUS_CAP_10TH         (1<<11)
#define PHY_STATUS_CAP_SUPR         (1<< 6)
#define PHY_STATUS_AUTONEG_ACK      (1<< 5)
#define PHY_STATUS_REMOTEFAULT      (1<< 4)
#define PHY_STATUS_CAP_AUTONEG      (1<< 3)
#define PHY_STATUS_LINK_OK          (1<< 2)
#define PHY_STATUS_JABBER           (1<< 1)
#define PHY_STATUS_EXTREGS          (1<< 0)

// These are the same for both AUTONEG registers
#define PHY_AUTONEG_NEXT            (1<<15)
#define PHY_AUTONEG_ACK             (1<<14)
#define PHY_AUTONEG_REMOTEFAULT     (1<<13)
#define PHY_AUTONEG_100BASET4       (1<< 9)
#define PHY_AUTONEG_100BASETX_FDX   (1<< 8)
#define PHY_AUTONEG_100BASETX_HDX   (1<< 7)
#define PHY_AUTONEG_10BASET_FDX     (1<< 6)
#define PHY_AUTONEG_10BASET_HDX     (1<< 5)
#define PHY_AUTONEG_CSMA_802_3      (1<< 0)

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------

// Use arrays provided by platform header to verify pointers.
#ifdef CYGDBG_USE_ASSERTS
#define CHECK_NDP_SC_LINK()                                                     \
    CYG_MACRO_START                                                             \
    int zzz, valid_netdev = 0, valid_sc = 0;                                    \
    for(zzz = 0; zzz < CYGNUM_DEVS_ETH_INTEL_I21143_DEV_COUNT; zzz++) {         \
        if (i21143_netdev_array[zzz] == ndp) valid_netdev = 1;                  \
        if (i21143_sc_array[zzz] == sc) valid_sc = 1;                           \
        if (valid_sc || valid_netdev) break;                                    \
    }                                                                           \
    CYG_ASSERT( valid_netdev, "Bad ndp" );                                      \
    CYG_ASSERT( valid_sc, "Bad sc" );                                           \
    CYG_ASSERT( (void *)p_i21143 == i21143_sc_array[zzz]->driver_private,       \
                "sc pointer bad" );                                             \
    CYG_MACRO_END
#else
#define CHECK_NDP_SC_LINK()
#endif

#define IF_BAD_21143( _p_ )                                             \
if (({                                                                  \
    int zzz, valid_p = 0;                                               \
    for(zzz = 0; zzz < CYGNUM_DEVS_ETH_INTEL_I21143_DEV_COUNT; zzz++) { \
        if (i21143_priv_array[zzz] == (_p_)) {                          \
            valid_p = 1;                                                \
            break;                                                      \
        }                                                               \
    }                                                                   \
    CYG_ASSERT(valid_p, "Bad pointer-to-i21143");                       \
    (!valid_p);                                                         \
}))



#if defined( DEBUG_TRAFFIC_TXDETAILS ) || defined( DEBUG_MAC )
static void dump_tx_details( struct i21143 *p_i21143, char *s )
{
    int i;
    cyg_uint32 prev = 0x0f0f0f0f;
    for ( i = 0; i < TX_DESCRIPTORS ; i++ ) {
        if ( 0 != (p_i21143->tx_ring[i].des1 |
                   p_i21143->tx_ring[i].buf1 |
                   p_i21143->tx_ring[i].buf2)
             || ((p_i21143->tx_ring[i].des0 != prev ) ) ) {
            prev = p_i21143->tx_ring[i].des0;
            diag_printf( "%10s: bd[%2d] = %08x %08x %8x %x",
                         s,
                         i,
                         p_i21143->tx_ring[i].des0,
                         p_i21143->tx_ring[i].des1,
                         p_i21143->tx_ring[i].buf1,
                         p_i21143->tx_ring[i].buf2 );
            diag_printf( " %s, %s%s %s\n",
                         p_i21143->tx_ring[i].des0 & DES0_STATUS_OWN_OPEN ? "Open" : "Done" ,
                         p_i21143->tx_ring[i].des1 & TDES1_CONTROL_FIRST ? "First" : "" ,
                         p_i21143->tx_ring[i].des1 & TDES1_CONTROL_LAST ? "Last" : "" ,
                         p_i21143->tx_ring[i].des1 & TDES1_CONTROL_INTERRUPT ? "Interrupt" : "" );
        }
    }
}
#endif

// ------------------------------------------------------------------------
//
//                NETWORK INTERFACE INITIALIZATION
//
//  Function : i21143_init
//
//  Description :
//       This routine resets, configures, and initializes the chip.
//
// ------------------------------------------------------------------------
static bool
i21143_init(struct cyg_netdevtab_entry * ndp)
{
    static int initialized = 0; // only probe PCI et al *once*

    struct eth_drv_sc *sc;
    struct i21143 *p_i21143;
    cyg_uint8 mac_address[ETHER_ADDR_LEN];

#ifdef DEBUG
    diag_printf("intel_i21143_init\n");
#endif

    sc = (struct eth_drv_sc *)(ndp->device_instance);
    p_i21143 = (struct i21143 *)(sc->driver_private);

    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG
        diag_printf( "Bad device private pointer %x\n", sc->driver_private );
#endif
        return 0;
    }

    CHECK_NDP_SC_LINK();

    p_i21143->ndp = (void *)ndp;

    if ( 0 == initialized++ ) {
        // then this is the first time ever:
        if ( ! pci_init_find_21143s() ) {
#ifdef DEBUG
            diag_printf( "pci_init_find_21143s failed" );
#endif
            return 0;
        }
    }

    // If this device is not present, exit
    if (0 == p_i21143->found)
        return 0;

    p_i21143->mac_addr_ok = 0;

    if ( p_i21143->hardwired_esa )
        eth_set_mac_address( p_i21143, NULL ); // use that already there
                                        // (actually a NOP because !active)
    else {
        int ee_addrbits;
        int val[3];
        int good = 0;

        ee_addrbits = get_eeprom_size( p_i21143->io_address );
#ifdef DEBUG_EE
        diag_printf( "EEPROM size = %d\n", ee_addrbits );
#endif
        if ( 6 == ee_addrbits || 8 == ee_addrbits ) {
            int i;
            for ( i = 0; i < 3; i++ ) {
                val[i] = read_eeprom_word( p_i21143->io_address,
                                           ee_addrbits,
                                           i );
#ifdef DEBUG_EE
                diag_printf( "EEPROM: word at %04x = %04x\n", i, val[i] );
#endif
            }
            if ( 0x0000 != (val[0] | val[1] | val[2] ) && // some nonzero bits
                 0xffff != (val[0] & val[1] & val[2] ) && // not all ones
                 0x0000 == (val[0] & 0x80) ) {            // not multicast
                mac_address[0] = val[0] & 0xff;
                mac_address[1] = val[0] >> 8;
                mac_address[2] = val[1] & 0xff;
                mac_address[3] = val[1] >> 8;
                mac_address[4] = val[2] & 0xff;
                mac_address[5] = val[2] >> 8;
                good = 1;
#ifdef DEBUG_MAC
                diag_printf( "EEPROM ESA OK: %04x %04x %04x\n",
                             val[0], val[1], val[2] );
#endif
            }
        }                
        if ( !good ) {
            // Invent one
            mac_address[0] = 0x01;
            mac_address[1] = 0x23;
            mac_address[2] = 0x45;
            mac_address[3] = 0x67;
            mac_address[4] = 0x98;
            mac_address[5] = 0x76;
        }
        // Just copy into the structure - not into hardware 'cos !active
        eth_set_mac_address( p_i21143, &mac_address[0] );
    }

#if defined( DEBUG ) || defined( DEBUG_MAC )
    diag_printf("MAC Address %s, ESA = %02X %02X %02X %02X %02X %02X\n",
                p_i21143->mac_addr_ok ? "OK" : "**BAD**", 
                p_i21143->mac_address[0],
                p_i21143->mac_address[1],
                p_i21143->mac_address[2],
                p_i21143->mac_address[3],
                p_i21143->mac_address[4],
                p_i21143->mac_address[5]);
#endif

    // Initialize upper level driver
    if ( p_i21143->mac_addr_ok )
        (sc->funs->eth_drv->init)(sc, &(p_i21143->mac_address[0]) );
    else
        (sc->funs->eth_drv->init)(sc, NULL );

    return (1);
}

// ------------------------------------------------------------------------
//
//  Function : i21143_start
//
// ------------------------------------------------------------------------
static void 
i21143_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags )
{
    struct i21143 *p_i21143;
    cyg_uint32 ioaddr;
    cyg_uint32 l;
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif

    p_i21143 = (struct i21143 *)sc->driver_private;
    ioaddr = p_i21143->io_address; // get 21143's I/O address
    
    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG
        diag_printf( "i21143_start: Bad device pointer %x\n", p_i21143 );
#endif
        return;
    }

    if ( ! p_i21143->mac_addr_ok ) {
#ifdef DEBUG
        diag_printf("i21143_start %d: invalid MAC address, "
                  "can't bring up interface\n",
                  p_i21143->index );
#endif
        return;
    }

#ifdef DEBUG
    diag_printf( "i21143_start: enters\n" );
#endif

    if ( p_i21143->active )
        i21143_stop( sc );

    // Update the cached copy of the status reg just in case
    l = i21143_status( p_i21143 );
    if ( l != p_i21143->line_status ) {
        // It has changed!
        i21143_reset(p_i21143);             // sets line_status itself
    }

    // Enable device
    p_i21143->active = 1;

    // Initialize tx status
    p_i21143->tx_endbuf = NO_TX_IN_PROGRESS;

    eth_set_mac_address( p_i21143, NULL ); // to put it in the device

    // After eth_set_mac_address() initialize the data structs for use:
    InitRxRing( p_i21143 );
    InitTxRing( p_i21143 );

    // Enable promiscuous mode if requested.
    i21143_configure(p_i21143, 0
#ifdef CYGPKG_NET
                     || !!(ifp->if_flags & IFF_PROMISC)
#endif
#ifdef ETH_DRV_FLAGS_PROMISC_MODE
                     || !!(flags & ETH_DRV_FLAGS_PROMISC_MODE)
#endif

                     );
#ifdef DEBUG
    {
        int status = p_i21143->line_status;
        diag_printf("i21143_start %d flg %x Link = %s, %s Mbps, %s Duplex\n",
                    p_i21143->index,
                    *(int *)p_i21143,
                    (GEN_STATUS_LINK    & status) ?   "Up" : "Down",
                    (GEN_STATUS_100MBPS & status) ?  "100" : "10",
                    (GEN_STATUS_FDX     & status) ? "Full" : "Half");
    }
#endif

    // Load pointer to Rx Ring and enable receiver
    OUTL( CYGHWR_PCI_VIRT_TO_BUS( (cyg_uint32)p_i21143->rx_ring ), CSR_RXBASE );
    // Enable receive interrupts
    l = INL( CSR_INTR_ENABLE );
    l |= 0
     | CSR_STATUS_NIS      
     | CSR_STATUS_AIS      
     // CSR_STATUS_ERI      
     // CSR_STATUS_FBE      
     // CSR_STATUS_LNF      
     // CSR_STATUS_GTE      
     // CSR_STATUS_ETI      
     // CSR_STATUS_RWT      
     | CSR_STATUS_RX_STOPPED
     | CSR_STATUS_RBU      
     | CSR_STATUS_RX_INTR  
     // CSR_STATUS_UNF      
     // CSR_STATUS_LNP_ANC  
     // CSR_STATUS_TJT      
     | CSR_STATUS_TBU      
     | CSR_STATUS_TX_STOPPED
     | CSR_STATUS_TX_INTR  
        ;
    OUTL( l, CSR_INTR_ENABLE );
    // and start the receiver
    l = INL( CSR_OPMODE );
    l |= (CSR_OPMODE_RX_START);
    OUTL( l, CSR_OPMODE );
    INCR_STAT( rx_restart );
#ifdef DEBUG_DUMP_REGS
    debug_dump_regs( ioaddr, BOTH );
#endif
}

// ------------------------------------------------------------------------
//
//  Function : i21143_status
//
// ------------------------------------------------------------------------
int
i21143_status( struct i21143* p_i21143 )
{
    int status;
    int i, j;
    cyg_uint32 ioaddr;
    
    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG
        diag_printf( "i21143_status: Bad device pointer %x\n", p_i21143 );
#endif
        return 0;
    }

    ioaddr = p_i21143->io_address; // get 21143's I/O address

    // Some of these bits latch and only reflect "the truth" on a 2nd reading.
    // So read and discard.
    status = mii_read_register( ioaddr, PHY_CONTROL_REG );
    status = mii_read_register( ioaddr, PHY_STATUS_REG  );
    // Use the "and" of the local and remote capabilities words to infer
    // what is selected:
    status = 0;
    i = mii_read_register( ioaddr, PHY_STATUS_REG );
    if ( PHY_STATUS_LINK_OK & i )
        status |= GEN_STATUS_LINK;

    i = mii_read_register( ioaddr, PHY_AUTONEG_ADVERT );
    j = mii_read_register( ioaddr, PHY_AUTONEG_REMOTE );
    p_i21143->phy_autoneg_remote = j;
#if defined( DEBUG_TRAFFIC ) || defined( DEBUG_IOCTL )
    diag_printf( "MII: capabilities are %04x, %04x; common %04x\n",
               i, j, i & j );
#endif
    j &= i; // select only common capabilities

    if ( (PHY_AUTONEG_100BASET4 |
          PHY_AUTONEG_100BASETX_FDX |
          PHY_AUTONEG_100BASETX_HDX)  & j )
        status |= GEN_STATUS_100MBPS;
    if ( (PHY_AUTONEG_100BASETX_FDX | PHY_AUTONEG_10BASET_FDX) & j )
        status |= GEN_STATUS_FDX;

    return status;
}

int
i21143_status_changed( struct i21143* p_i21143 )
{
    cyg_uint32 ioaddr;
    int j;
    ioaddr = p_i21143->io_address; // get 21143's I/O address

    j = mii_read_register( ioaddr, PHY_AUTONEG_REMOTE );
    return ( j != p_i21143->phy_autoneg_remote );
}

// ------------------------------------------------------------------------
//
//  Function : i21143_stop
//
// ------------------------------------------------------------------------

static void
i21143_stop( struct eth_drv_sc *sc )
{
    struct i21143 *p_i21143;

    p_i21143 = (struct i21143 *)sc->driver_private;

    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG
        diag_printf( "i21143_stop: Bad device pointer %x\n", p_i21143 );
#endif
        return;
    }
   
#ifdef DEBUG_STARTSTOPRESET
    diag_printf("i21143_stop %d flg %x\n", p_i21143->index, *(int *)p_i21143 );
#endif

    p_i21143->active = 0;               // stop people tormenting it

    if ( NO_TX_IN_PROGRESS != p_i21143->tx_endbuf) {
        cyg_uint32 key;
        
        CYG_ASSERT( TX_DESCRIPTORS > p_i21143->tx_endbuf, "tx_endbuf range" );

        key = p_i21143->tx_keys[ 0 ];
        // Common "done" code - key nonzero => report done
        if (key) {
            p_i21143->tx_keys[ 0 ] = 0;
            p_i21143->tx_endbuf = NO_TX_IN_PROGRESS;
            // Then tell the stack we are done:
            INCR_STAT( tx_complete );
            (sc->funs->eth_drv->tx_done)( sc, key, 0 );
        }
    }

    i21143_reset(p_i21143);             // that should stop it
}


// ------------------------------------------------------------------------
//
//  Function : InitRxRing
//
// ------------------------------------------------------------------------
static void
InitRxRing(struct i21143* p_i21143)
{
    int i;
    volatile BUFDES *bp;

    p_i21143->rx_ring =
        (BUFDES *)CYGHWR_CACHED_TO_UNCACHED( &(p_i21143->_rx[0]) );

    bp = p_i21143->rx_ring;
    for ( i = 0 ; i < RX_DESCRIPTORS; i++ ) {
        bp->des0 = DES0_STATUS_OWN_OPEN; // NIC owns it, not the CPU
        bp->des1 = (MAX_RX_PACKET_SIZE << DES1_B1SIZE_SHIFT);
        bp->buf1 = CYGHWR_PCI_VIRT_TO_BUS( &( p_i21143->_rxbufs[i][0] ) );
        bp->buf2 = 0;
        bp++;
    }
    bp--; // last one
    bp->des1 |= DES1_ENDRING;

    p_i21143->next_rx_descriptor = 0;
}

// ------------------------------------------------------------------------
//
//  Function : PacketRxReady     (Called from delivery thread & foreground)
//
// ------------------------------------------------------------------------
static void
PacketRxReady(struct i21143* p_i21143)
{
    volatile BUFDES *bp;
    int index;
    struct eth_drv_sc *sc;
    int count;
    {
        struct cyg_netdevtab_entry *ndp;
        ndp = (struct cyg_netdevtab_entry *)(p_i21143->ndp);
        sc = (struct eth_drv_sc *)(ndp->device_instance);
        CHECK_NDP_SC_LINK();
    }

    CYG_ASSERT( 0 <= p_i21143->next_rx_descriptor, "rx descriptor underflow" );
    CYG_ASSERT( RX_DESCRIPTORS > p_i21143->next_rx_descriptor, "rx descriptor overflow" );

    index = p_i21143->next_rx_descriptor;

    // Scan receive buffers
    for ( count = 0; count < RX_DESCRIPTORS; count++ ) {
        cyg_uint32 ldes0;
        int length;

        bp = &p_i21143->rx_ring[ index ];

        CYG_ASSERT( 0 == bp->buf2, "Corrupt bp->buf2" );
        CYG_ASSERT( 0 != bp->buf1, "Null bp->buf1" );
        CYG_ASSERT( (MAX_RX_PACKET_SIZE << DES1_B1SIZE_SHIFT) == (bp->des1 &~ DES1_ENDRING),
                    "Corrupt bp->des1" );

        ldes0 = bp->des0;

#ifdef DEBUG_TRAFFIC
        diag_printf( "PacketRxReady: index %d des0 = %08x, %s, %s, %d bytes\n",
                     index, ldes0,
                     (ldes0 & DES0_STATUS_OWN) ? "open" : "done",
                     ((RDES0_STATUS_FIRST | RDES0_STATUS_LAST | DES0_STATUS_ERROR) & ldes0)
                     == (RDES0_STATUS_FIRST | RDES0_STATUS_LAST) ? "OK" : "--",
                     (RDES0_COUNT_MASK & ldes0) >> RDES0_COUNT_SHIFT );
#endif        

        if ( DES0_STATUS_OWN_OPEN == (ldes0 & DES0_STATUS_OWN) )
            // This buffer bilong the device
            break;

        INCR_STAT( rx_count );

        // Otherwise, this one is for us
        if ( ((RDES0_STATUS_FIRST | RDES0_STATUS_LAST | DES0_STATUS_ERROR) & ldes0)
             == (RDES0_STATUS_FIRST | RDES0_STATUS_LAST) ) {
            // A complete packet and no error bit
        
            length = (RDES0_COUNT_MASK & ldes0) >> RDES0_COUNT_SHIFT;

            CYG_ASSERT( MAX_RX_PACKET_SIZE >= length, "Oversize Rx" );

            // tell the callback the right packet
            p_i21143->next_rx_descriptor = index;

            INCR_STAT( rx_good );

#ifdef CYGPKG_NET
            if ( length > sizeof( struct ether_header ) )
            // then it is acceptable; offer the data to the network stack
#endif
            (sc->funs->eth_drv->recv)( sc, length );

            // All done!
        }
#ifdef KEEP_STATISTICS
        else {
            if ( RDES0_STATUS_LAST & ldes0 ) {
                // Then we have good error info; analyse and count the errors
                if ( ldes0 & ( RDES0_STATUS_CRC ) )
                    INCR_STAT( rx_crc_errors );
                if ( ldes0 & ( RDES0_STATUS_DB ) )
                    INCR_STAT( rx_align_errors );
                if ( ldes0 & ( RDES0_STATUS_LATECOLL ) )
                    INCR_STAT( rx_collisions );
                if ( ldes0 & ( RDES0_STATUS_TOOLONG ) )
                    INCR_STAT( rx_too_long_frames );
                if ( ldes0 & ( RDES0_STATUS_RF ) )
                    INCR_STAT( rx_short_frames );
                if ( ldes0 & ( RDES0_STATUS_RE ) )
                    INCR_STAT( rx_symbol_errors );
            }
            else
                // Dunno what went wrong really...
                INCR_STAT( rx_resource_errors );
        }
#endif // KEEP_STATISTICS

        // Open the buffer for the device to use
        bp->des0 = DES0_STATUS_OWN_OPEN; // NIC owns it, not the CPU

        // Step i around the ring buffer...
        index++;
        if ( index >= RX_DESCRIPTORS )
            index = 0;
    }

    // record the right packet for next time
    p_i21143->next_rx_descriptor = index;

    CYG_ASSERT( 0 <= p_i21143->next_rx_descriptor, "rx descriptor underflow" );
    CYG_ASSERT( RX_DESCRIPTORS > p_i21143->next_rx_descriptor, "rx descriptor overflow" );

    {   //  We should not have needed ioaddr before this point.
        cyg_uint32 ioaddr;
        cyg_uint32 l;

        // Now examine the state of the receive engine and prompt it accordingly.
        ioaddr = p_i21143->io_address; // get 21143's I/O address
        l = INL( CSR_STATUS );

#ifdef DEBUG_TRAFFIC
        diag_printf( "PacketRxReady: done scan, next %d, status %08x %s\n",
                     p_i21143->next_rx_descriptor, l,
                     (CSR_STATUS_RXSTATUS & l) != CSR_STATUS_RXSTATUS_STOPPED ?
                     "Running" : "Stopped" );
#endif

        if ( (CSR_STATUS_RXSTATUS & l) != CSR_STATUS_RXSTATUS_STOPPED ) {
            // Then ping it into life
            OUTL( 0, CSR_RXPOLL );
        }
        else {
            // Oh dear, it's stopped; we must initialize and start the rx machine
            InitRxRing( p_i21143 );
            OUTL( CYGHWR_PCI_VIRT_TO_BUS( (cyg_uint32)p_i21143->rx_ring ), CSR_RXBASE );
            l = INL( CSR_OPMODE );
            l |= CSR_OPMODE_RX_START;
            OUTL( l, CSR_OPMODE );
            INCR_STAT( rx_resource );
            INCR_STAT( rx_restart );
        }
    }
}


// and the callback function

static void 
i21143_recv( struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len )
{
    volatile BUFDES *bp;
    struct i21143 *p_i21143;
    cyg_uint32 from_p;
    int total_len;
    struct eth_drv_sg *last_sg;

    p_i21143 = (struct i21143 *)(sc->driver_private);

    CYG_ASSERT( 0 <= p_i21143->next_rx_descriptor, "rx descriptor underflow" );
    CYG_ASSERT( RX_DESCRIPTORS > p_i21143->next_rx_descriptor, "rx descriptor overflow" );

    bp = &p_i21143->rx_ring[ p_i21143->next_rx_descriptor ];

    CYG_ASSERT( 0 == bp->buf2, "Corrupt bp->buf2" );
    CYG_ASSERT( 0 != bp->buf1, "Null bp->buf1" );
    CYG_ASSERT( DES0_STATUS_OWN_DONE == (bp->des0 & DES0_STATUS_OWN), "bp not done" );

    // Copy the data to the network stack
    from_p = CYGHWR_BUS_TO_UNCACHED( bp->buf1 );
    total_len = (RDES0_COUNT_MASK & bp->des0) >> RDES0_COUNT_SHIFT;

#ifdef DEBUG_TRAFFIC
    diag_printf("Recv callback: index %d des0 = %08x: %d sg's, %d bytes\n",
                p_i21143->next_rx_descriptor, bp->des0, sg_len, total_len);
#endif

    // check we have memory to copy into; we would be called even if
    // caller was out of memory in order to maintain our state.
    if ( 0 == sg_len || 0 == sg_list )
        return; // caller was out of mbufs

    INCR_STAT( rx_deliver );

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

        memcpy( to_p, (unsigned char *)from_p, l );
        from_p += l;
        total_len -= l;
    }

    CYG_ASSERT( 0 == total_len, "total_len mismatch in rx" );
    CYG_ASSERT( last_sg == sg_list, "sg count mismatch in rx" );
    CYG_ASSERT( CYGHWR_BUS_TO_UNCACHED( bp->buf1 ) < from_p, "from_p wild in rx" );
    CYG_ASSERT( CYGHWR_BUS_TO_UNCACHED( bp->buf1 ) + MAX_RX_PACKET_SIZE >= from_p,
                "from_p overflow in rx" );
}


// ------------------------------------------------------------------------
//
//  Function : InitTxRing
//
// ------------------------------------------------------------------------
static void
InitTxRing(struct i21143* p_i21143)
{
    int i;
    volatile BUFDES *bp;

    p_i21143->tx_ring =
        (BUFDES *)CYGHWR_CACHED_TO_UNCACHED( &(p_i21143->_tx[0]) );

    bp = p_i21143->tx_ring;
    for ( i = 0 ; i < TX_DESCRIPTORS; i++ ) {
        bp->des0 = DES0_STATUS_OWN_DONE; // CPU owns it, not the NIC
        bp->des1 = 0;
        bp->buf1 = bp->buf2 = 0;
        bp++;
    }
    bp--; // last one
    bp->des1 |= DES1_ENDRING;
}

// ------------------------------------------------------------------------
//
//  Function : TxDone          (Called from delivery thread)
//
// This returns Tx's from the Tx Machine to the stack (ie. reports
// completion) - allowing for missed interrupts, and so on.
// ------------------------------------------------------------------------

static void
TxDone(struct i21143* p_i21143)
{
    struct eth_drv_sc *sc;
    unsigned long key = 0;
    cyg_uint32 ioaddr;

    {
        struct cyg_netdevtab_entry *ndp;
        ndp = (struct cyg_netdevtab_entry *)(p_i21143->ndp);
        sc = (struct eth_drv_sc *)(ndp->device_instance);
        CHECK_NDP_SC_LINK();
    }

    ioaddr = p_i21143->io_address; // get 21143's I/O address

#ifdef DEBUG_TRAFFIC_TXDETAILS
    dump_tx_details( p_i21143, "TxDone" );
#endif

    if ( NO_TX_IN_PROGRESS != p_i21143->tx_endbuf) {
        cyg_uint32 l;
        volatile BUFDES *bp;
        cyg_uint32 des;

        CYG_ASSERT( TX_DESCRIPTORS > p_i21143->tx_endbuf, "tx_endbuf range" );

        l = INL( CSR_STATUS ); // Important: read this first

        bp = p_i21143->tx_ring;
        bp += p_i21143->tx_endbuf; // The descriptor with the "end" marker

        // It's not clear where error status gets written if the error is
        // *before* the end-marked bufdesc.  We'll see.

        des = bp->des1;
        CYG_ASSERT( TDES1_CONTROL_LAST & des , "Last buf not marked" );
        CYG_ASSERT( TDES1_CONTROL_INTERRUPT & des , "No interrupt req" );
        CYG_ASSERT( 0 == (DES1_2ACHAIN & des), "Buffer chained" );
        CYG_ASSERT( 0 == ((TDES1_CONTROL_SETUP |
                           TDES1_CONTROL_SETUP_FT1 |
                           TDES1_CONTROL_SETUP_FT0 |
                           TDES1_CONTROL_NO_CRC |
                           TDES1_CONTROL_NO_PAD)
                          & des),   "Buffer wierd flags" );
        des = bp->des0;
        if ( DES0_STATUS_OWN_DONE == (des & DES0_STATUS_OWN) ) {
            // Then it finished; somehow...
            key = p_i21143->tx_keys[ 0 ];
#ifdef DEBUG_TRAFFIC
            diag_printf("TxDone: KEY %x, %d descs, status %08x\n",
                        key, p_i21143->tx_endbuf+1, des );
#endif
#ifdef KEEP_STATISTICS
            // Then look for an error code from the tx records...
            if ( ! ( DES0_STATUS_ERROR & des ) )
                INCR_STAT( tx_good );
            // Not all the following are errors, so check all
            if ( des & ( TDES0_STATUS_LO | TDES0_STATUS_NC ) )
                INCR_STAT( tx_carrier_loss );
            if ( des & ( TDES0_STATUS_LC ) )
                INCR_STAT( tx_late_collisions );
            if ( des & ( TDES0_STATUS_EC ) )
                INCR_STAT( tx_max_collisions );
            if ( des & ( TDES0_STATUS_LF ) )
                INCR_STAT( tx_sqetesterrors );
            if ( des & ( TDES0_STATUS_UFL ) )
                INCR_STAT( tx_underrun );
            if ( des & ( TDES0_STATUS_DE ) )
                INCR_STAT( tx_deferred );
            // Now count collisions per se:
            des = ((des & TDES0_STATUS_CC_MASK) >> TDES0_STATUS_CC_SHIFT);
            if ( 0 < des ) {
                INCR_STAT( tx_total_collisions );
                if ( 1 < des )
                    INCR_STAT( tx_mult_collisions );
                else 
                    INCR_STAT( tx_single_collisions );
            }
#endif // KEEP_STATISTICS
        }
        else {
            // a Tx is in progress, but it has not yet completed;
            // probably wise to check on the tx machine status.
            if ( ((CSR_STATUS_TXSTATUS & l) == CSR_STATUS_TXSTATUS_STOPPED) ||
                 ((CSR_STATUS_TXSTATUS & l) == CSR_STATUS_TXSTATUS_SUSPENDED) ) {
                // then it's not active right now!
                key = p_i21143->tx_keys[ 0 ];
#ifdef DEBUG_TRAFFIC
                diag_printf("TxDone STOPPED!  KEY %x, %d descs, status %08x, CSR_STATUS %08x\n",
                            key, p_i21143->tx_endbuf, des, l );
#endif
                INCR_STAT( tx_underrun ); // at a guess...
            }
        }

        // Common "done" code - key nonzero => report done
        if (key) {
            p_i21143->tx_keys[ 0 ] = 0;
            p_i21143->tx_endbuf = NO_TX_IN_PROGRESS;
            // Then tell the stack we are done:
            INCR_STAT( tx_complete );
            (sc->funs->eth_drv->tx_done)( sc, key, 0 );
        }
    }
}


// ------------------------------------------------------------------------
//
//  Function : i21143_can_send
//
// ------------------------------------------------------------------------

static int 
i21143_can_send(struct eth_drv_sc *sc)
{
    struct i21143 *p_i21143;

    p_i21143 = (struct i21143 *)sc->driver_private;

    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG_TRAFFIC
        diag_printf( "i21143_can_send: Bad device pointer %x\n", p_i21143 );
#endif
        return 0;
    }

    if ( p_i21143->active ) {
        if ( NO_TX_IN_PROGRESS != p_i21143->tx_endbuf )
            // Poll for Tx completion
            TxDone( p_i21143 );

        // Poll for receptions
        PacketRxReady( p_i21143 );
    }

#ifdef neverDEBUG_TRAFFIC
    diag_printf( "i21143_can_send: returning %d\n",
                 (NO_TX_IN_PROGRESS == p_i21143->tx_endbuf) && p_i21143->active );
#endif
    
    return (NO_TX_IN_PROGRESS == p_i21143->tx_endbuf) && p_i21143->active;
}

// ------------------------------------------------------------------------
//
//  Function : i21143_send
//
// ------------------------------------------------------------------------

static void 
i21143_send(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list, int sg_len, int total_len,
            unsigned long key)
{
    struct i21143 *p_i21143;
    cyg_uint32 ioaddr;
    struct eth_drv_sg *last_sg;
    volatile BUFDES *bp;
    int bufcount;
    cyg_uint32 x;

    p_i21143 = (struct i21143 *)sc->driver_private;

    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG_TRAFFIC
        diag_printf( "i21143_send: Bad device pointer %x\n", p_i21143 );
#endif
        return;
    }

    if ( NO_TX_IN_PROGRESS != p_i21143->tx_endbuf) {
        // Then we cannot do this
#ifdef DEBUG_TRAFFIC
        diag_printf( "i21143_send: tx busy %x, %d bufs key %x\n",
                     p_i21143, p_i21143->tx_endbuf, p_i21143->tx_keys[0] );
#endif
        CYG_FAIL( "Send call when already busy" );
        INCR_STAT( tx_dropped );
        return;
    }

    ioaddr = p_i21143->io_address;

    bp = p_i21143->tx_ring;
    bufcount = 0;

#ifdef DEBUG_TRAFFIC
    diag_printf( "i21143_send() key %x\n", key );
#endif

    INCR_STAT( tx_count );

    for ( last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++ ) {
        cyg_uint8 *from_p;
        int l;

        from_p = (cyg_uint8 *)(sg_list->buf); // normal cached address
        l = sg_list->len;

        if ( l > total_len )
            l = total_len;

        // Ensure the mbuf contents really is in RAM where DMA can see it.
        // (Must round to cache lines apparantly for some MIPS)
        HAL_DCACHE_STORE( ((CYG_ADDRESS)from_p) &~(HAL_DCACHE_LINE_SIZE-1),
                          l + HAL_DCACHE_LINE_SIZE );
        
        // There are two pointer, length pairs in each descriptor.  We only
        // bother using the first one - the code saved is small but so is
        // the overhead of memory.
        bp->buf1 = CYGHWR_PCI_VIRT_TO_BUS( from_p ); // uncached real RAM address
        bp->buf2 = 0;
        // Record length and add endring flag iff end of ring - never leave
        // it blank in memory!
        x = (l << DES1_B1SIZE_SHIFT) & DES1_B1SIZE_MASK; // plus zero buf2 size.
        if ( (TX_DESCRIPTORS-1) == bufcount )
            x |= DES1_ENDRING;
        bp->des1 = x;

        // Leave writing the zeroth bufdesc DES0 until all are complete:
        // the tx engine will be stopped looking there.
        if ( 0 < bufcount ) // then we can write DES0 also.
            bp->des0 = DES0_STATUS_OWN_OPEN;

        total_len -= l;
        bp++;
        bufcount++;

        if ( 0 > total_len ) 
            break; // Should exit via sg_last normally
    }

    CYG_ASSERT( 0 == total_len, "length mismatch in tx" );
    CYG_ASSERT( last_sg == sg_list, "sg count mismatch in tx" );

    CYG_ASSERT( bp > &p_i21143->tx_ring[0], "bp underflow" );

    bp--; // back to the last-used bp.
    bufcount--;
    p_i21143->tx_endbuf = bufcount;     // record that we are busy
    p_i21143->tx_keys[0] = key;         // and the key to return

    CYG_ASSERT( bp < &p_i21143->tx_ring[ TX_DESCRIPTORS ], "bp underflow" );

    // Mark this the last valid buffer.
    bp->des1 |= TDES1_CONTROL_LAST | TDES1_CONTROL_INTERRUPT;

    CYG_ASSERT( &(p_i21143->tx_ring[bufcount]) == bp, "Bp/bufcount misstep" );

    bp++;
    bufcount++;

    // Make the rest be null links
    for ( /* bp */ ; bp < &p_i21143->tx_ring[ TX_DESCRIPTORS ]; bp++ ) {
        bp->buf1 = 0;
        bp->buf2 = 0;
        x = 0; // zero size for both buffers
        if ( (TX_DESCRIPTORS-1) == bufcount )
            x |= DES1_ENDRING;
        bp->des1 = x;
        bp->des0 = DES0_STATUS_OWN_OPEN;
        bufcount++;
    }

    // Now it's safe to write the zeroth descriptor:
    p_i21143->tx_ring->des1 |= TDES1_CONTROL_FIRST;
    p_i21143->tx_ring->des0 = DES0_STATUS_OWN_OPEN;

#ifdef DEBUG_TRAFFIC_TXDETAILS
    dump_tx_details( p_i21143, "i21143_send" );
#endif

    // And start off the tx system
    x = INL( CSR_STATUS );

#ifdef DEBUG_TRAFFIC
    diag_printf( "i21143_send: ready, status %08x %s\n",
                 x,
                 (CSR_STATUS_TXSTATUS & x) != CSR_STATUS_TXSTATUS_STOPPED ?
                 "Running" : "Stopped" );
#endif

    if ( CSR_STATUS_TXSTATUS_STOPPED == (CSR_STATUS_TXSTATUS & x) ) {
        cyg_uint32 l;
        // Then we must initialize and start the tx machine
        OUTL( CYGHWR_PCI_VIRT_TO_BUS( (cyg_uint32)p_i21143->tx_ring ), CSR_TXBASE );
        l = INL( CSR_OPMODE );
        l |= CSR_OPMODE_TX_START;
        OUTL( l, CSR_OPMODE );
    }
    else if ( CSR_STATUS_TXSTATUS_SUSPENDED == (CSR_STATUS_TXSTATUS & x) ) {
        // Then we just have to ping it
        OUTL( 0, CSR_TXPOLL );
    }
    else {
#ifdef DEBUG_TRAFFIC
        diag_printf( "i21143_send: tx not ready %x CSR_STATUS %08x\n", p_i21143, x );
#endif
        CYG_FAIL( "Tx is not ready to tx" );

        // Try to recover by brutal means...
        i21143_stop( sc );              // will return the key
        i21143_start( sc, NULL, 0 );
    }
}

// ------------------------------------------------------------------------
//
//  Function : i21143_reset
//
// ------------------------------------------------------------------------
static void
i21143_reset(struct i21143* p_i21143)
{
    cyg_uint32 ioaddr = p_i21143->io_address;
    cyg_uint32 l;
    int i, status;
    // First stop the tx and rx engines - doc suggests that's necessary
    // before writing the reset reg, but it seems a little paranoid.

    l = INL( CSR_OPMODE );
    l &=~ (CSR_OPMODE_RX_START | CSR_OPMODE_TX_START);
    OUTL( l, CSR_OPMODE );

    for ( i = 0; i < 10000; i++) {
        l = INL( CSR_STATUS );
        if ( ((CSR_STATUS_TXSTATUS & l) == CSR_STATUS_TXSTATUS_STOPPED) &&
             ((CSR_STATUS_RXSTATUS & l) == CSR_STATUS_RXSTATUS_STOPPED) )
            break;
    }
#ifdef DEBUG_STARTSTOPRESET
    diag_printf( "i21143_reset: rx and tx idle after %d iters, status %x\n",
                 i, l );
#endif

    // Acknowledge all the interrupts from these activities
    // within the device:
    OUTL( 0xffffffff, CSR_STATUS ); // clear all bits

    // Now reset the device.  We are going to re-init all params anyway, so
    // ignore other settings:
    OUTL( CSR_BUSMODE_RESET, CSR_BUSMODE );

    udelay( 100 ); // let reset take effect.  "50 PCI clock cycles"

    for ( i = 0; i < 10000; i++) {
        l = INL( CSR_BUSMODE );
        if ( (CSR_BUSMODE_RESET & l) == 0 )
            break;
    }
#ifdef DEBUG_STARTSTOPRESET
    diag_printf( "i21143_reset: reset zero after %d iters, busmode %x\n",
                 i, l );
#endif

    // Gross initialization
    OUTL( 0 // No other options
          // CSR_BUSMODE_PM        
          // CSR_BUSMODE_WIE       
          // CSR_BUSMODE_RLE       
          // CSR_BUSMODE_RME       
          // CSR_BUSMODE_DBO_BE    
          // CSR_BUSMODE_DBO_LE    
          // CSR_BUSMODE_TAP_SHIFT 
          | (1 << CSR_BUSMODE_CAL_SHIFT)
          | (4 << CSR_BUSMODE_PBL_SHIFT)
          // CSR_BUSMODE_ENDIAN_BE 
          // CSR_BUSMODE_ENDIAN_LE 
          // CSR_BUSMODE_DSL_SHIFT 
          // CSR_BUSMODE_BAR       
          // CSR_BUSMODE_RESET     

          , CSR_BUSMODE );

    // No interrupt sources enabled yet
    OUTL( 0, CSR_INTR_ENABLE );

    OUTL( 0
          // CSR_ROM_MII_MGT_MDI
          | CSR_ROM_MII_MGT_MOM_READ
          // CSR_ROM_MII_MGT_MDO  
          // CSR_ROM_MII_MGT_MDC  
          // CSR_ROM_MII_MGT_RD   
          // CSR_ROM_MII_MGT_WR   
          // CSR_ROM_MII_MGT_BR   
          // CSR_ROM_MII_MGT_SR   
          // CSR_ROM_MII_MGT_REG  
          // CSR_ROM_MII_MGT_SR_DO
          // CSR_ROM_MII_MGT_SR_DI
          // CSR_ROM_MII_MGT_SR_CK
          // CSR_ROM_MII_MGT_SR_CS
          | 0xff,

          CSR_ROM_MII_MGT );

    // -----------------------------------
    // Now find out about the status of the PHY via MII
#if 0
    // Try resetting the PHY and power-cycling it:
    mii_write_register( ioaddr, PHY_CONTROL_REG, PHY_CONTROL_RESET | PHY_CONTROL_AUTONEG_EN);
    while (1) {
        int v;
        v = mii_read_register( ioaddr, PHY_CONTROL_REG );
        if ( 0 == (v & PHY_CONTROL_RESET) )
            break;
    }

    mii_write_register( ioaddr, PHY_CONTROL_REG,
                        PHY_CONTROL_POWERDOWN | PHY_CONTROL_MII_DIS | PHY_CONTROL_AUTONEG_EN );
    udelay( 1000 );
    mii_write_register( ioaddr, PHY_CONTROL_REG, PHY_CONTROL_AUTONEG_EN | PHY_CONTROL_AUTONEG_RST );

    udelay( 1000 );

    while (1) {
        int v;
        v = mii_read_register( ioaddr, PHY_STATUS_REG );
        if ( PHY_STATUS_AUTONEG_ACK & v )
            break;
    }    
#endif

#ifdef DEBUG_DUMP_REGS
    debug_dump_regs( ioaddr, MII );
#endif

    status = i21143_status( p_i21143 );
    p_i21143->line_status = status;     // record it for SNMP info
#ifdef DEBUG_STARTSTOPRESET
    diag_printf("i21143_reset %d flg %x Link = %s, %s Mbps, %s Duplex\n",
                p_i21143->index,
                *(int *)p_i21143,
                (GEN_STATUS_LINK    & status) ?   "Up" : "Down",
                (GEN_STATUS_100MBPS & status) ?  "100" : "10",
                (GEN_STATUS_FDX     & status) ? "Full" : "Half");
#endif

    OUTL( -1, CSR_ROM_PGM_ADDR );
    OUTL(  0, CSR_INTR_MITIGATION );

    // CSR13, 14 = 0; select SIA mode temporarily.
//    OUTL( CSR_OPMODE_MUST_BE_ONE, CSR_OPMODE);
    OUTL( CSR_SIA_CONN_DEFAULT, CSR_SIA_CONN );
    OUTL( CSR_SIA_TXRX_DEFAULT, CSR_SIA_TXRX );
    OUTL( CSR_SIA_GPPORT_DEFAULT, CSR_SIA_GPPORT );

    // Last one, CSR6
    l = 0
        | CSR_OPMODE_SC          
        // CSR_OPMODE_RA
        // CSR_OPMODE_IGNOREMSB   
        | CSR_OPMODE_MUST_BE_ONE 
        // CSR_OPMODE_SCR      do not set for MII mode 
        // CSR_OPMODE_PCS      do not set for MII mode   
        // CSR_OPMODE_TTM      set for 10Mb, not set for 100Mb
        | CSR_OPMODE_SF          // Set the store&forward bit so we can keep-
        | CSR_OPMODE_HBD         //  -up at 100Mbit
        | CSR_OPMODE_PS_MIISYM 
        | CSR_OPMODE_CA          
        // CSR_OPMODE_TX_THRES_SHIFT
        // CSR_OPMODE_TX_START    
        // CSR_OPMODE_FC          
        // (0 << CSR_OPMODE_LOOPBACK_SHIFT)
        // CSR_OPMODE_FD
        // CSR_OPMODE_MULTICAST         // Pass all multicast
        // CSR_OPMODE_PROMISC           // Promisc mode
        // CSR_OPMODE_SB          
        // CSR_OPMODE_IF                // Inverse filtering (!)         
        // CSR_OPMODE_PB          
        // CSR_OPMODE_HO                // 0: Perfect filter
        // CSR_OPMODE_RX_START          //    1: hashing for /all/ addresses
        // CSR_OPMODE_HP                // 0: Perfect filter of N address
        ;                               //    1: imperfect hash filter + 1 fixed addr 
    
    if ( GEN_STATUS_FDX & status )
        l |= CSR_OPMODE_FD;
    if ( ! (GEN_STATUS_100MBPS & status) )
        l |= CSR_OPMODE_TTM;

    OUTL( l, CSR_OPMODE );

}

// ------------------------------------------------------------------------
//
//                       INTERRUPT HANDLERS
//
// ------------------------------------------------------------------------

static cyg_uint32
eth_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask( vector );
    
#ifdef DEBUG_TRAFFIC
    diag_printf( "i21143_isr, vector %d\n", vector );
#endif

    return CYG_ISR_CALL_DSR;        // schedule DSR
}


// An indirection is used because (if we have multiple devices) we don't
// know all the "sc" values at the time the interrupts are created.
static 
void eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    struct i21143* p_i21143 = (struct i21143 *)data;
    struct cyg_netdevtab_entry *ndp =
        (struct cyg_netdevtab_entry *)(p_i21143->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

    INCR_STAT( interrupts );

    // but here, it must be a *sc:
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );
}

// ------------------------------------------------------------------------
// Deliver routine:

void
i21143_deliver(struct eth_drv_sc *sc)
{
    struct i21143* p_i21143 = (struct i21143 *)(sc->driver_private);
    cyg_uint32 status;
    cyg_uint32 ioaddr;

    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG_TRAFFIC
        diag_printf( "i21143_deliver: Bad device pointer %x\n", p_i21143 );
#endif
        return;
    }

    ioaddr = p_i21143->io_address;
    status = INL( CSR_STATUS );

#ifdef DEBUG_TRAFFIC
    diag_printf( "i21143_deliver: status %08x\n", status );
#endif
    
    // Acknowledge all INT sources that were active
    OUTL( status, CSR_STATUS );

    // Search for link status changes
    if ( i21143_status_changed( p_i21143 ) ) {
#ifdef DEBUG_TRAFFIC
        diag_printf( "i21143_can_send: status changed\n" );
#endif
        i21143_stop( sc );
        i21143_start( sc, NULL, 0 );
    }

    if ( (CSR_STATUS_TBU | CSR_STATUS_TX_STOPPED | CSR_STATUS_TX_INTR)
         & status ) {
        TxDone( p_i21143 );
    }

    if ( (CSR_STATUS_RX_STOPPED | CSR_STATUS_RBU | CSR_STATUS_RX_INTR)
         & status ) {
        PacketRxReady( p_i21143 );
    }

    cyg_drv_interrupt_acknowledge(p_i21143->vector);
    cyg_drv_interrupt_unmask( p_i21143->vector );
}

// ------------------------------------------------------------------------
// Device table entry to operate the chip in a polled mode.
// Only diddle the interface we were asked to!

void
i21143_poll(struct eth_drv_sc *sc)
{
    struct i21143 *p_i21143;
    p_i21143 = (struct i21143 *)sc->driver_private;
    
    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG_TRAFFIC
        diag_printf( "i21143_poll: Bad device pointer %x\n", p_i21143 );
#endif
        return;
    }

    // As it happens, this driver always requests the DSR to be called:
    (void)eth_isr( p_i21143->vector, (cyg_addrword_t)p_i21143 );

    // (no harm in calling this ints-off also, when polled)
    i21143_deliver( sc );
}

// ------------------------------------------------------------------------
// Determine interrupt vector used by a device - for attaching GDB stubs
// packet handler.
int
i21143_int_vector(struct eth_drv_sc *sc)
{
    struct i21143 *p_i21143;
    p_i21143 = (struct i21143 *)sc->driver_private;
    return (p_i21143->vector);
}

// ------------------------------------------------------------------------
//
//  Function : pci_init_find_21143s
//
// This is called exactly once at the start of time to:
//  o scan the PCI bus for objects
//  o record them in the device table
//  o acquire all the info needed for the driver to access them
//  o instantiate interrupts for them
//  o attach those interrupts appropriately
// ------------------------------------------------------------------------
static cyg_pci_match_func find_21143s_match_func;

static cyg_bool
find_21143s_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
    return
        (/* (0x8086 == v) || */ (0x1011 == v)) && // (Intel or) DEC
        (0x0019 == d); //  [DC21142/3][PCI/CardBus 10/100 Mbit Ethernet Ctlr] 
}

static int
pci_init_find_21143s( void )
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;
    cyg_uint32 l;
    int device_index;
    int found_devices = 0;

#ifdef DEBUG
    diag_printf("pci_init_find_21143s()\n");
#endif

    cyg_pci_init();
#ifdef DEBUG
    diag_printf("Finished cyg_pci_init();\n");
#endif

    devid = CYG_PCI_NULL_DEVID;

    for (device_index = 0; 
         device_index < CYGNUM_DEVS_ETH_INTEL_I21143_DEV_COUNT;
         device_index++) {
        struct i21143 *p_i21143 = i21143_priv_array[device_index];

        p_i21143->index = device_index;

        // See above for find_21143s_match_func - it selects any of several
        // variants.  This is necessary in case we have multiple mixed-type
        // devices on one board in arbitrary orders.
        if (cyg_pci_find_matching( &find_21143s_match_func, NULL, &devid )) {
#ifdef DEBUG
            diag_printf("eth%d = 21143\n", device_index);
#endif
            cyg_pci_get_device_info(devid, &dev_info);

            p_i21143->interrupt_handle = 0; // Flag not attached.
            if (cyg_pci_translate_interrupt(&dev_info, &p_i21143->vector)) {
#ifdef DEBUG
                diag_printf(" Wired to HAL vector %d\n", p_i21143->vector);
#endif
                cyg_drv_interrupt_create(
                    p_i21143->vector,
                    0,                  // Priority - unused
                    (CYG_ADDRWORD)p_i21143, // Data item passed to ISR & DSR
                    eth_isr,            // ISR
                    eth_dsr,            // DSR
                    &p_i21143->interrupt_handle, // handle to intr obj
                    &p_i21143->interrupt_object ); // space for int obj

                cyg_drv_interrupt_attach(p_i21143->interrupt_handle);

                cyg_drv_interrupt_configure( p_i21143->vector, 1, 0 );

                // Don't unmask the interrupt yet, that could get us into a
                // race.
            }
            else {
                p_i21143->vector=0;
#ifdef DEBUG
                diag_printf(" Does not generate interrupts.\n");
#endif
            }

            if (cyg_pci_configure_device(&dev_info)) {
#ifdef DEBUG
                int i;
                diag_printf("Found device on bus %d, devfn 0x%02x:\n",
                          CYG_PCI_DEV_GET_BUS(devid),
                          CYG_PCI_DEV_GET_DEVFN(devid));

                if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
                    diag_printf(" Note that board is active. Probed"
                              " sizes and CPU addresses invalid!\n");
                }
                diag_printf(" Vendor    0x%04x", dev_info.vendor);
                diag_printf("\n Device    0x%04x", dev_info.device);
                diag_printf("\n Command   0x%04x, Status 0x%04x\n",
                          dev_info.command, dev_info.status);
                
                diag_printf(" Class/Rev 0x%08x", dev_info.class_rev);
                diag_printf("\n Header 0x%02x\n", dev_info.header_type);

                diag_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                          dev_info.header.normal.sub_vendor, 
                          dev_info.header.normal.sub_id);

                for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
                    diag_printf(" BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                    diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                              dev_info.base_size[i], dev_info.base_map[i]);
                }
                diag_printf(" eth%d configured\n", device_index);
#endif
                found_devices++;
                p_i21143->found = 1;
                p_i21143->active = 0;
                p_i21143->devid = devid;
                p_i21143->io_address = dev_info.base_map[0];
#ifdef DEBUG
                diag_printf(" I/O address = 0x%08x\n", dev_info.base_map[0]);
#endif
                
#ifdef pciDEBUG
                diag_printf( "Before wakeup\n" );
                for ( i = 0; i <= 0xe0; i += 4 ) {
                    if ( i == 0x18 ) i = 0x28;
                    if ( i == 0x58 ) i = 0xdc;
                    cyg_pci_read_config_uint32( devid, i, &l );
                    diag_printf( "PCI Config reg %02x = 0x%08x\n", i, l );
                }
#endif
                // Awaken the device through the CFDD device-specific
                // register - which boots up with it asleep - great.
                l = 0;
                cyg_pci_write_config_uint32( devid, 0x40, l );
#ifdef pciDEBUG
                diag_printf( "After wakeup\n" );
                for ( i = 0; i <= 0xe0; i += 4 ) {
                    if ( i == 0x18 ) i = 0x28;
                    if ( i == 0x58 ) i = 0xdc;
                    cyg_pci_read_config_uint32( devid, i, &l );
                    diag_printf( "PCI Config reg %02x = 0x%08x\n", i, l );
                }
#endif
                // Now the PCI part of the device is configured, reset
                // it. This should make it safe to enable the
                // interrupt
                i21143_reset(p_i21143);

                // This is the indicator for "uses an interrupt"
                if (p_i21143->interrupt_handle != 0) {
                    cyg_drv_interrupt_acknowledge(p_i21143->vector);
                    cyg_drv_interrupt_unmask(p_i21143->vector);
                }
#ifdef DEBUG
                diag_printf(" **** Device enabled for I/O only and Bus Master\n");
#endif
            }
            else {
                p_i21143->found = 0;
                p_i21143->active = 0;
#ifdef DEBUG
                diag_printf("Failed to configure device %d\n",device_index);
#endif
            }
        }
        else {
            p_i21143->found = 0;
            p_i21143->active = 0;
#ifdef DEBUG
            diag_printf("eth%d not found\n", device_index);
#endif
        }
    }

    if (0 == found_devices)
        return 0;

    // Now a delay to ensure the hardware has "come up" before you try to
    // use it.
    udelay( 1000 );
    return 1;
}

// ------------------------------------------------------------------------
//
//  Function : i21143_configure
//
//  Return : 0 = It worked.
//           non0 = It failed.
// ------------------------------------------------------------------------

static int i21143_configure(struct i21143* p_i21143, int promisc )
{
    cyg_uint32 ioaddr;
    cyg_uint32 l;

    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG
        diag_printf( "configure: Bad device pointer %x\n",
                   p_i21143 );
#endif
        return -1;
    }

    ioaddr = p_i21143->io_address;  

    l = INL( CSR_OPMODE );
    if ( promisc )
        l |= CSR_OPMODE_PROMISC;
    else
        l &=~ CSR_OPMODE_PROMISC;
    OUTL( l, CSR_OPMODE );

    return 0;
}

// ------------------------------------------------------------------------
//
//  Function : i21143_ioctl
//
// ------------------------------------------------------------------------
static int
i21143_ioctl(struct eth_drv_sc *sc, unsigned long key,
             void *data, int data_length)
{
    struct i21143 *p_i21143;

    p_i21143 = (struct i21143 *)sc->driver_private;

    IF_BAD_21143( p_i21143 ) {
#ifdef DEBUG
        diag_printf( "i21143_ioctl/control: Bad device pointer %x\n", p_i21143 );
#endif
        return -1;
    }

#ifdef DEBUG_TRAFFIC
    diag_printf( "i21143_ioctl: device eth%d at %x; key is 0x%x, data at %x[%d]\n",
               p_i21143->index, p_i21143, key, data, data_length );
#endif

    switch ( key ) {

#ifdef ETH_DRV_SET_MAC_ADDRESS
    case ETH_DRV_SET_MAC_ADDRESS: {
        int act;
        if ( 6 != data_length )
            return -2;
        act = p_i21143->active;
        i21143_stop( sc );
        eth_set_mac_address( p_i21143, data );
        if ( act )
            i21143_start( sc, NULL, 0 );
        return 0;
    }
#endif

#ifdef ETH_DRV_GET_IF_STATS_UD
    case ETH_DRV_GET_IF_STATS_UD: // UD == UPDATE
//        ETH_STATS_INIT( sc );    // so UPDATE the statistics structure
#endif
        // drop through
#ifdef ETH_DRV_GET_IF_STATS
    case ETH_DRV_GET_IF_STATS:
#endif
#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
    {
        struct ether_drv_stats *p = (struct ether_drv_stats *)data;
        int i;
        static unsigned char my_chipset[]
            = { ETH_DEV_DOT3STATSETHERCHIPSET };

        strcpy( p->description, CYGDAT_DEVS_ETH_DESCRIPTION );
        CYG_ASSERT( 48 > strlen(p->description), "Description too long" );

        for ( i = 0; i < SNMP_CHIPSET_LEN; i++ )
            if ( 0 == (p->snmp_chipset[i] = my_chipset[i]) )
                break;

        i = p_i21143->line_status;      // use cached copy - else sloooow

        if ( !( i & GEN_STATUS_LINK) ) {
            p->operational = 2;         // LINK DOWN
            p->duplex = 1;              // UNKNOWN
            p->speed = 0;
        }
        else {
            p->operational = 3;            // LINK UP
            p->duplex = (i & GEN_STATUS_FDX) ? 3 : 2; // 2 = SIMPLEX, 3 = DUPLEX
            p->speed = ((i & GEN_STATUS_100MBPS) ? 100 : 10) * 1000000;
        }

#ifdef KEEP_STATISTICS
        {
            struct stats *ps = &p_i21143->stats;

            // Admit to it...
            p->supports_dot3        = true;

            // Those commented out are not available on this chip.

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
//            p->rx_overrun_errors    = ps->rx_overrun_errors   ;
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
    return -1;
}

// ------------------------------------------------------------------------

static void
eth_set_mac_address( struct i21143* p_i21143, cyg_uint8 *addr )
{
    cyg_uint32 ioaddr = p_i21143->io_address;
    cyg_uint32 l;
    int i;
    volatile BUFDES *bp;
    cyg_uint8 *datap;

#define SETUP_SIZE (192)

    cyg_uint32 buf[ SETUP_SIZE/4 ]; // for word alignment

    // We need to send a pseudo-transmit which contains setup data.

    if ( NULL == addr )
        addr = &p_i21143->mac_address[0];
    else {
        p_i21143->mac_address[0] = addr[0]; 
        p_i21143->mac_address[1] = addr[1]; 
        p_i21143->mac_address[2] = addr[2]; 
        p_i21143->mac_address[3] = addr[3]; 
        p_i21143->mac_address[4] = addr[4]; 
        p_i21143->mac_address[5] = addr[5]; 
    }

    if ( ! p_i21143->active ) {
        // Then do not torment the hardware - starting
        // the interface will do that.
        p_i21143->mac_addr_ok = 1;
        return;
    }

    InitTxRing( p_i21143 ); // Since we are about to use this struct

    bp = p_i21143->tx_ring;

    bp->des0 = DES0_STATUS_OWN_DONE; // Safety: not for the NIC yet

    bp->buf1 = CYGHWR_PCI_VIRT_TO_BUS( ((cyg_uint32)&buf[0]) );
    bp->buf2 = 0;
    
    datap = (cyg_uint8 *)CYGHWR_BUS_TO_UNCACHED( bp->buf1 );

    bp->des1 =
        DES1_ENDRING |
        TDES1_CONTROL_SETUP |
        //TDES1_CONTROL_SETUP_FT0 |  // 0,0 <=> perfect filtering
        //TDES1_CONTROL_SETUP_FT1 | 
        (SETUP_SIZE << DES1_B1SIZE_SHIFT);
    // The filtering type is 00 = perfect filtering of 16 addresses
    // (including the broadcast address, I think)

    for ( i = 0; i < SETUP_SIZE ; i++ )
        datap[i] = 0xff; // fill the space with the broadcast address

    // Insert the address: do it in an endian agnostic fashion.
    // [If we were doing perfect plus hash table it would be slot
    //  13 at offset 156.  Lower halfwords only.]
    // So we'll use slot 13 anyway; the other slots are all FFs.

    datap[156] = addr[0];
    datap[157] = addr[1];
    datap[158] = addr[1];
    datap[159] = addr[0];
    datap[160] = addr[2];
    datap[161] = addr[3];
    datap[162] = addr[3];
    datap[163] = addr[2];
    datap[164] = addr[4];
    datap[165] = addr[5];
    datap[166] = addr[5];
    datap[167] = addr[4];

    // Make the rest be null links
    for ( i = 1; i < TX_DESCRIPTORS; i++ ) {
        int x;
        p_i21143->tx_ring[ i ].buf1 = 0;
        p_i21143->tx_ring[ i ].buf2 = 0;
        x = 0; // zero size for both buffers
        if ( (TX_DESCRIPTORS-1) == i )
            x |= DES1_ENDRING;
         p_i21143->tx_ring[ i ].des1 = x;
         p_i21143->tx_ring[ i ].des0 = DES0_STATUS_OWN_OPEN;
    }

    // Tell the device the address
    OUTL( CYGHWR_PCI_VIRT_TO_BUS( (cyg_uint32)bp ), CSR_TXBASE );
    // Open the descriptor to the device
    bp->des0 = DES0_STATUS_OWN_OPEN;

#ifdef DEBUG_MAC
    dump_tx_details( p_i21143, "set_mac starting:" );
#endif
    // And set it going
    l = INL( CSR_OPMODE );
    l |= CSR_OPMODE_TX_START;
    OUTL( l, CSR_OPMODE );

    // Wait for the buffer to be closed
    for ( i = 0; i < 10000; i++ )
        if ( DES0_STATUS_OWN_DONE == (DES0_STATUS_OWN & bp->des0) )
            break;
#ifdef DEBUG_MAC
    diag_printf( "eth_set_mac_address: %d loops: bufdesc status tdes0 %08x, tdes1 %08x\n",
                 i, bp->des0, bp->des1 );
#endif

    // Wait for it to be done
    for ( i = 0; i < 10000; i++ ) {
        l = INL( CSR_STATUS );
        if ( ((CSR_STATUS_TXSTATUS & l) == CSR_STATUS_TXSTATUS_STOPPED) ||
             ((CSR_STATUS_TXSTATUS & l) == CSR_STATUS_TXSTATUS_SUSPENDED) )
            break;
    }

#ifdef DEBUG_MAC
    diag_printf( "eth_set_mac_address: tx done after %d iters; status %x\n",
                 i, l );
#endif

#ifdef DEBUG_MAC
    dump_tx_details( p_i21143, "set_mac all done:" );
#endif

    // Kill the tx engine
    l = INL( CSR_OPMODE );
    l &=~ CSR_OPMODE_TX_START;
    OUTL( l, CSR_OPMODE );

    for ( i = 0; i < 10000; i++ ) {
        l = INL( CSR_STATUS );
        if ( ((CSR_STATUS_TXSTATUS & l) == CSR_STATUS_TXSTATUS_STOPPED) )
            break;
    }
#ifdef DEBUG_MAC
    diag_printf( "eth_set_mac_address: tx stopped after %d iters; status %x\n",
                 i, l );
#endif

    // Acknowledge all the interrupts from these activities
    // within the device:
    OUTL( 0xffffffff, CSR_STATUS ); // clear all bits

    p_i21143->mac_addr_ok = 1;
    
}

// ------------------------------------------------------------------------
//
//                     MDIO
//
// Device-specific bit-twiddling and line driving side-effects

// CYGACC_CALL_IF_DELAY_US() drags in huge amounts of scheduler locking and
// the like 'cos it's a VV call!  We only want a delay of 1uS tops, so:

#define MII_DELAY() do { int z; for ( z = 0; z < 20; z++ ) ; } while (0)

#if 0
# define MII_PRINTF diag_printf
# define MII_STUFF "%4s | %4s | %4s | %4s [%08x]\n",    \
    (l & (1<<19)) ? "MDI" : "---",                      \
    (l & (1<<18)) ? "Read" : "Wr",                      \
    (l & (1<<17)) ? "MDO" : "---",                      \
    (l & (1<<16)) ? "CLK" : "clk",                      \
    l
#else
# define MII_PRINTF( foo )
# define MII_STUFF
#endif

static inline void mii_clock_up( int ioaddr )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    l |= CSR_ROM_MII_MGT_MDC;
    OUTL( l, CSR_ROM_MII_MGT );
    MII_PRINTF( "mii_clock_up  : " MII_STUFF  );
    MII_DELAY();
}

static inline void mii_clock_down( int ioaddr )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    l &=~ CSR_ROM_MII_MGT_MDC;
    OUTL( l, CSR_ROM_MII_MGT );
    MII_PRINTF( "mii_clock_down: " MII_STUFF  );
    MII_DELAY();
}

static inline void mii_read_mode( int ioaddr )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    l |= CSR_ROM_MII_MGT_MOM_READ;
    OUTL( l, CSR_ROM_MII_MGT );
    MII_PRINTF( "mii_read_mode : " MII_STUFF  );
    MII_DELAY();
}

static inline int mii_read_data_bit( int ioaddr )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    MII_PRINTF( "mii_read_data : " MII_STUFF  );
    return CSR_ROM_MII_MGT_MDI == (CSR_ROM_MII_MGT_MDI & l);
}

static inline void mii_write_data_bit( int ioaddr, int databit )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    if ( databit )
        l |= CSR_ROM_MII_MGT_MDO;
    else
        l &=~ CSR_ROM_MII_MGT_MDO;
    l &=~ CSR_ROM_MII_MGT_MOM; // drive the mdio line
    OUTL( l, CSR_ROM_MII_MGT );
    MII_PRINTF( "mii_write_data: " MII_STUFF  );
    MII_DELAY();
}

// Pass ioaddr around "invisibly"
#define MII_CLOCK_UP()            mii_clock_up(ioaddr)
#define MII_CLOCK_DOWN()          mii_clock_down(ioaddr)
#define MII_READ_MODE()           mii_read_mode(ioaddr)
#define MII_READ_DATA_BIT()       mii_read_data_bit(ioaddr)
#define MII_WRITE_DATA_BIT( _d_ ) mii_write_data_bit(ioaddr,(_d_))

// ------------------------------------------------------------------------
//
//                     MDIO
//
// Management data over the MII interface - nasty hand driven serial stuff
//

static void mii_write_bits( int ioaddr, int val, int bitcount )
{
    int i;
    // These are deliberately signed ints so that we can send an overlong
    // preamble if we want by saying "send -1 of width 40 bits and relying
    // on sign extension.
    for ( i = bitcount - 1; i >= 0; i-- ) {
        MII_CLOCK_DOWN();
        MII_WRITE_DATA_BIT( (val >> i) & 1 );
        MII_CLOCK_UP();
    }
}

static int mii_read_bits( int ioaddr, int bitcount )
{
    int i;
    int val = 0;
    for ( i = bitcount - 1; i >= 0; i-- ) {
        MII_CLOCK_DOWN();
        val <<= 1;
        val |= MII_READ_DATA_BIT();
        MII_CLOCK_UP();
    }
    return val;
}

#define MII_WRITE_BITS( val, bitcount ) mii_write_bits( ioaddr, val, bitcount )
#define MII_READ_BITS( bitcount )       mii_read_bits( ioaddr, bitcount )

// Now define subsections of the protocol in terms of the above

#define MII_WRITE_PREAMBLE()    MII_WRITE_BITS( -1, 40 )  // >32 x 1s
#define MII_WRITE_START()       MII_WRITE_BITS( 1, 2 )    // 01
#define MII_WRITE_WRITE_CMD()   MII_WRITE_BITS( 1, 2 )    // 01
#define MII_WRITE_READ_CMD()    MII_WRITE_BITS( 2, 2 )    // 10

#define PHY_ADDRESS (1)

#define MII_WRITE_PHY_ADDR()    MII_WRITE_BITS( PHY_ADDRESS, 5 )
#define MII_WRITE_REGNUM( _r_ ) MII_WRITE_BITS( (_r_), 5 )

#define MII_WRITE_TURNAROUND()  MII_WRITE_BITS( 2, 2 )

#define MII_READ_TURNAROUND()   CYG_MACRO_START         \
  MII_READ_MODE(); /* to turn off driving the line */   \
  (void)(MII_READ_BITS( 2 )); /* discard TA "bits" */   \
CYG_MACRO_END

#define MII_IDLE() CYG_MACRO_START                              \
  MII_READ_MODE(); /* to turn off driving the line */           \
  ((void)MII_READ_BITS( 5 )); /* extra clocks in Hi-Z mode */   \
  MII_CLOCK_DOWN();                                             \
CYG_MACRO_END

#define MII_READ_REGVAL()       MII_READ_BITS( 16 )
#define MII_WRITE_REGVAL( _v_ ) MII_WRITE_BITS( (_v_), 16 )

static int mii_read_register( int ioaddr, int regnum )
{
    int value;
    MII_WRITE_PREAMBLE();
    MII_WRITE_START();
    MII_WRITE_READ_CMD();
    MII_WRITE_PHY_ADDR();
    MII_WRITE_REGNUM( regnum );
    MII_READ_TURNAROUND();
    value = MII_READ_REGVAL();
    MII_IDLE();
    return value;
}

static void mii_write_register( int ioaddr, int regnum, int value )
{
    MII_WRITE_PREAMBLE();
    MII_WRITE_START();
    MII_WRITE_WRITE_CMD();
    MII_WRITE_PHY_ADDR();
    MII_WRITE_REGNUM( regnum );
    MII_WRITE_TURNAROUND();
    MII_WRITE_REGVAL( value );
    MII_IDLE();
}

// ------------------------------------------------------------------------
//
// Serial EEPROM access  - much like the other Intel ethernet
//

// CYGACC_CALL_IF_DELAY_US() drags in huge amounts of scheduler locking and
// the like 'cos it's a VV call!  Waste of time, mostly.

#define EE_DELAY() do { int z; for ( z = 0; z < 0x1000; z++ ) ; } while (0)

#if 0
# define EE_PRINTF diag_printf
# define EE_STUFF "%4s | %4s | %4s | %4s [%08x]\n",     \
    (l & (1<<2)) ? "eeDI" : "---",                      \
    (l & (1<<0)) ? "eeCS" : "--",                       \
    (l & (1<<3)) ? "eeDO" : "---",                      \
    (l & (1<<1)) ? "CLK"  : "clk",                      \
    l & 0xfffff
#else
# define EE_PRINTF( foo )
# define EE_STUFF
#endif


static inline void ee_select( int ioaddr )
{
    cyg_uint32 l;
    l = CSR_ROM_MII_MGT_SR | CSR_ROM_MII_MGT_RD
        | CSR_ROM_MII_MGT_MOM_READ; // Keep MII in read mode
    OUTL( l, CSR_ROM_MII_MGT );
    EE_DELAY();
    l |= CSR_ROM_MII_MGT_SR_CS;
    OUTL( l, CSR_ROM_MII_MGT );
    EE_PRINTF( "ee_select    : " EE_STUFF  );
    EE_DELAY();
}

static inline void ee_deselect( int ioaddr )
{
    cyg_uint32 l;
    l = CSR_ROM_MII_MGT_SR | CSR_ROM_MII_MGT_RD
        | CSR_ROM_MII_MGT_MOM_READ; // Keep MII in read mode
    OUTL( l, CSR_ROM_MII_MGT );
    EE_PRINTF( "ee_deselect 1: " EE_STUFF  );
    EE_DELAY();
    EE_DELAY();
    EE_DELAY();
    l = 0
        | CSR_ROM_MII_MGT_MOM_READ; // Keep MII in read mode
    OUTL( l, CSR_ROM_MII_MGT );
    EE_PRINTF( "ee_deselect 2: " EE_STUFF  );
    EE_DELAY();
    EE_DELAY();
    EE_DELAY();
}

static inline void ee_clock_up( int ioaddr )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    l |= CSR_ROM_MII_MGT_SR_CK;
    OUTL( l, CSR_ROM_MII_MGT );
    EE_PRINTF( "ee_clock_up  : " EE_STUFF  );
    EE_DELAY();
}

static inline void ee_clock_down( int ioaddr )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    l &=~ CSR_ROM_MII_MGT_SR_CK;
    OUTL( l, CSR_ROM_MII_MGT );
    EE_PRINTF( "ee_clock_down: " EE_STUFF  );
    EE_DELAY();
}

static inline int ee_read_data_bit( int ioaddr )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    EE_PRINTF( "ee_read_data : " EE_STUFF  );
    return CSR_ROM_MII_MGT_SR_DO == (CSR_ROM_MII_MGT_SR_DO & l);
}

static inline void ee_write_data_bit( int ioaddr, int databit )
{
    cyg_uint32 l;
    l = INL( CSR_ROM_MII_MGT );
    if ( databit )
        l |= CSR_ROM_MII_MGT_SR_DI;
    else
        l &=~ CSR_ROM_MII_MGT_SR_DI;
    OUTL( l, CSR_ROM_MII_MGT );
    EE_PRINTF( "ee_write_data: " EE_STUFF  );
    EE_DELAY();
}

// Pass ioaddr around "invisibly"
#define EE_SELECT()              ee_select(ioaddr)
#define EE_DESELECT()            ee_deselect(ioaddr)
#define EE_CLOCK_UP()            ee_clock_up(ioaddr)
#define EE_CLOCK_DOWN()          ee_clock_down(ioaddr)
#define EE_READ_DATA_BIT()       ee_read_data_bit(ioaddr)
#define EE_WRITE_DATA_BIT( _d_ ) ee_write_data_bit(ioaddr,(_d_))

// ------------------------------------------------------------------------

static int
get_eeprom_size( int ioaddr )
{
    int i, addrbits, tmp;

    // Should already be not-selected, but anyway:
    EE_SELECT();

    // Shift the read command bits out.
    for (i = 3; i >= 0; i--) { // Doc says to shift out a zero then:
        tmp = (6 & (1 << i)) ? 1 : 0; // "6" is the "read" command.
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        EE_CLOCK_DOWN();
    }
    // Now clock out address zero, looking for the dummy 0 data bit
    for ( i = 1; i <= 12; i++ ) {
        EE_WRITE_DATA_BIT(0);
        EE_CLOCK_UP();
        tmp = EE_READ_DATA_BIT();
        EE_CLOCK_DOWN();
        if ( !tmp )
            break;
    }

#ifdef DEBUG_EE
    diag_printf( "eeprom data bits %d\n", i );
#endif
    
    if ( 6 != i && 8 != i && 1 != i) {
#ifdef DEBUG_EE
        diag_printf( "*****EEPROM data bits not 6, 8 or 1*****\n" );
#endif
        addrbits = 1; // Flag no eeprom here.
    }
    else
        addrbits = i;

    // read in the data regardless
    tmp = 0;
    for (i = 15; i >= 0; i--) {
        EE_CLOCK_UP();
        if ( EE_READ_DATA_BIT() )
            tmp |= (1<<i);
        EE_CLOCK_DOWN();
    }

#ifdef DEBUG_EE
    diag_printf( "eeprom first data word %x\n", tmp );
#endif
   
    // Terminate the EEPROM access.
    EE_DESELECT();
    
    return addrbits;
}

static int
read_eeprom_word( int ioaddr, int addrbits, int address )
{
    int i, tmp;

    // Should already be not-selected, but anyway:
    EE_SELECT();

    // Shift the read command bits out.
    for (i = 3; i >= 0; i--) { // Doc says to shift out a zero then:
        tmp = (6 & (1 << i)) ? 1 : 0; // "6" is the "read" command.
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        EE_CLOCK_DOWN();
    }
    // Now clock out address
    for ( i = addrbits - 1; i >= 0 ; i-- ) {
        tmp = (address & (1<<i)) ? 1 : 0;
        EE_WRITE_DATA_BIT(tmp);
        EE_CLOCK_UP();
        tmp = EE_READ_DATA_BIT();
        EE_CLOCK_DOWN();

        CYG_ASSERT( (0 == tmp) == (0 == i), "Looking for zero handshake bit" );
    }

    // read in the data
    tmp = 0;
    for (i = 15; i >= 0; i--) {
        EE_CLOCK_UP();
        if ( EE_READ_DATA_BIT() )
            tmp |= (1<<i);
        EE_CLOCK_DOWN();
    }

    // Terminate the EEPROM access.
    EE_DESELECT();
 
#ifdef DEBUG_EE
    diag_printf( "eeprom address %4x: data %4x\n", address, tmp );
#endif

    return tmp;
}

// ------------------------------------------------------------------------

// EOF if_i21143.c

