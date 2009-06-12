//==========================================================================
//
//      if_i82544.c
//
//	Intel 82544 ethernet driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    hmt, gthomas
// Contributors: Ron Spence, Pacific Softworks, jskov
// Date:         2000-02-01
// Purpose:      
// Description:  hardware driver for 82544 Intel PRO/100+ ethernet
// Notes:        CU commands such as dump and config should, according
//               to the docs, set the CU active state while executing.
//               That does not seem to be the case though, and the
//               driver polls the completion bit in the packet status
//               word instead.
//
//               Platform code may provide this vector:
//               CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT if it
//               requires the interrupts to be handled via demuxers
//               attached to a distinct interrupt.
//
//               Platform code may alternatively define:
//               CYGHWR_DEVS_ETH_INTEL_I82544_DEMUX_ALL if it is necessary
//               to demux all interrupt sources - for example if they are
//               wire-or'd together on some hardware but distinct on
//               others.  In this circumstance it is permitted for
//               cyg_pci_translate_interrupt [HAL_PCI_TRANSLATE_INTERRUPT]
//               to return invalid for 2nd and subsequent devices.
//
//               Platform code can also define these three:
//               CYGPRI_DEVS_ETH_INTEL_I82544_MASK_INTERRUPTS(p_i82544,old)
//               CYGPRI_DEVS_ETH_INTEL_I82544_UNMASK_INTERRUPTS(p_i82544,old)
//               CYGPRI_DEVS_ETH_INTEL_I82544_ACK_INTERRUPTS(p_i82544)
//               which are particularly useful when nested interrupt
//               management is needed (which is always IMHO).
//
//               Platform code can define this:
//               CYGHWR_DEVS_ETH_INTEL_I82544_MISSED_INTERRUPT(p_i82544)
//               to detect a dropped interrupt and loop again or
//               direct-call the DSR to reschedule the delivery routine.
//               Only a problem on edge-triggered interrupt systems.
//
//               Platform code can also provide this macro:
//               CYGPRI_DEVS_ETH_INTEL_I82544_INTERRUPT_ACK_LOOP(p_i82544)
//               to handle delaying for acks to register on the interrupt
//               controller as necessary on the EBSA.
//
//               Platform can define CYGHWR_DEVS_ETH_INTEL_I82544_GET_ESA()
//               as an external means to get ESAs, possibly from RedBoot
//               configuration info that's stored in flash memory.
//
//               Platform def CYGHWR_DEVS_ETH_INTEL_I82544_HAS_NO_EEPROM
//               removes all code for dealing with the EEPROM for those
//               targets where there is none fitted.  Either an external
//               means to get ESAs should be used, or we must rely on
//               hard-wiring the ESA's into each executable by means of the
//               usual CDL configuration.
//
//               Platform def CYGHWR_DEVS_ETH_INTEL_I82544_HAS_ONE_EEPROM
//               is for hardware with multiple devices, but only one with a
//               serial EEPROM installed.  The 2nd device would get either
//               the same ESA - because they are certain to be on different
//               segment and internets - or the same ESA incremented by
//               CYGHWR_DEVS_ETH_INTEL_I82544_HAS_ONE_EEPROM_MAC_ADJUST.
//               CYGHWR_DEVS_ETH_INTEL_I82544_HAS_ONE_EEPROM should be the
//               number (0 or 1) of the device that does have the EEPROM.
//
//               CYGHWR_DEVS_ETH_INTEL_I82544_PCIMEM_DISCONTIGUOUS enables
//               checking code for breaks in the physical address of PCI
//               window memory.  This can happen on some boards where a
//               smaller SDRAM is fitted than the hardware allows, so some
//               higher-order address bits are ignored.  We make SDRAM
//               contiguous in mapped memory, but what the i82544 sees
//               might be discontiguous.  The checking code skips any
//               allocated chunk who appears to contain such a break, and
//               tries again.
//
//               CYGHWR_DEVS_ETH_INTEL_I82544_RESET_TIMEOUT( int32 )
//               CYGHWR_DEVS_ETH_INTEL_I82544_TIMEOUT_FIRED( int32 ) if
//               both defined give the driver a means to detect that we
//               have been fixated on the same transmit operation for too
//               long - we missed an interrupt or the device crashed.  The
//               int32 argument is used to hold a eg. the value of a
//               fast-running hardware timer.
//
//               Platform def CYGHWR_DEVS_ETH_INTEL_I82544_USE_ASD is
//               used to select Auto Speed Detection for setting up the
//               link parameters.
//
//        FIXME: replace -1/-2 return values with proper E-defines
//        FIXME: For 82557/8 compatibility i82544_configure() function
//               probably needs some tweaking - config bits differ
//               slightly but crucially.
//        FIXME: EEPROM code not tested on a BE system.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <pkgconf/devs_eth_intel_i82544.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
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
// So we can check the validity of the PCI window against the MLTs opinion,
// and thereby what the malloc heap consumes willy-nilly:
#include CYGHWR_MEMORY_LAYOUT_H
#else
#error "Need PCI package here"
#endif

#include <cyg/devs/eth/i82544_info.h>

#include CYGDAT_DEVS_ETH_INTEL_I82544_INL

// ------------------------------------------------------------------------

#ifdef CYGDBG_DEVS_ETH_INTEL_I82544_CHATTER
#define DEBUG_82544 // This one prints stuff as packets come and go
#define DEBUG          // Startup printing mainly
#define noDEBUG_EE       // Some EEPROM specific retries &c
#endif

#ifdef CYGDBG_USE_ASSERTS
static struct {
    int can_send;
    int deliver;
    int stats;
    int waitcmd_timeouts;
    int waitcmd_timeouts_cu;
    int lockup_timeouts;
    int bad_cu_idles;
} missed_interrupt = { 0,0,0, 0,0, 0, 0 };
#endif

#ifndef CYGPKG_REDBOOT

#define os_printf diag_printf
#define db_printf diag_printf

#else

static void os_printf( char *fmt, ... )
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

#define db_printf os_printf

#endif

// ------------------------------------------------------------------------
//
//                             MEMORY ADDRESSING
// 
// ------------------------------------------------------------------------
// Macros for writing shared memory structures - no need for byte flipping

#define READMEM8(   _reg_, _val_ ) ((CYG_BYTE)(_val_) = *((volatile CYG_BYTE *)(_reg_)))
#define WRITEMEM8(  _reg_, _val_ ) (*((volatile CYG_BYTE *)(_reg_)) = (CYG_BYTE)(_val_))
#define READMEM16(  _reg_, _val_ ) ((CYG_WORD16)(_val_) = *((volatile CYG_WORD16 *)(_reg_)))
#define WRITEMEM16( _reg_, _val_ ) (*((volatile CYG_WORD16 *)(_reg_)) = (CYG_WORD16)(_val_))
#define READMEM32(  _reg_, _val_ ) ((CYG_WORD32)(_val_) = *((volatile CYG_WORD32 *)(_reg_)))
#define WRITEMEM32( _reg_, _val_ ) (*((volatile CYG_WORD32 *)(_reg_)) = (CYG_WORD32)(_val_))
#define READMEM64(  _reg_, _val_ ) ((CYG_WORD64)(_val_) = *((volatile CYG_WORD64 *)(_reg_)))
#define WRITEMEM64( _reg_, _val_ ) (*((volatile CYG_WORD64 *)(_reg_)) = (CYG_WORD64)(_val_))

#define OUTL( _v_, _a_ ) WRITEMEM32( _a_, _v_ )

static inline cyg_uint32 INL(cyg_uint32 io_address)
 {   cyg_uint32 _t_; READMEM32( io_address, _t_ ); return _t_;   }


// ------------------------------------------------------------------------
// Map from CPU-view addresses to PCI-bus master's view - however that is:

#ifdef CYGHWR_INTEL_I82544_PCI_VIRT_TO_BUS

#define VIRT_TO_BUS( _x_ ) CYGHWR_INTEL_I82544_PCI_VIRT_TO_BUS( _x_ )
#define BUS_TO_VIRT( _x_ ) CYGHWR_INTEL_I82544_PCI_BUS_TO_VIRT( _x_ )

#else // use default mappings: get a physical address to give to the device

#define VIRT_TO_BUS( _x_ ) virt_to_bus((cyg_uint32)(_x_))
static inline cyg_uint32 virt_to_bus(cyg_uint32 p_memory)
{ return CYGARC_PHYSICAL_ADDRESS(p_memory);    }

#define BUS_TO_VIRT( _x_ ) bus_to_virt((cyg_uint32)(_x_))
static inline cyg_uint32 bus_to_virt(cyg_uint32 p_bus)
{ return CYGARC_UNCACHED_ADDRESS(p_bus);    }

#endif // not defined CYGHWR_INTEL_I82544_PCI_VIRT_TO_BUS

// ------------------------------------------------------------------------
//                                                                      
//                   82544 REGISTER OFFSETS
//                                                                      
// ------------------------------------------------------------------------

// General registers
#define I82544_CTRL     0x00000
#define I82544_STATUS   0x00008
#define I82544_EECD     0x00010
#define I82544_CTRL_EXT 0x00018
#define I82544_MDIC     0x00020
#define I82544_FCAL     0x00028
#define I82544_FCAH     0x0002c
#define I82544_FCT      0x00030
#define I82544_VET      0x00038
#define I82544_FCTTV    0x00170
#define I82544_TXCW     0x00178
#define I82544_RXCW     0x00180
#define I82544_PBA      0x01000

// Interrupt control registers
#define I82544_ICR      0x000c0
#define I82544_ICS      0x000c8
#define I82544_IMS      0x000d0
#define I82544_IMC      0x000d8

// Receive registers
#define I82544_RCTL     0x00100
#define I82544_FCRTL    0x02160
#define I82544_FCRTH    0x02168
#define I82544_RDBAL    0x02800
#define I82544_RDBAH    0x02804
#define I82544_RDLEN    0x02808
#define I82544_RDH      0x02810
#define I82544_RDT      0x02818
#define I82544_RDTR     0x02820
#define I82544_RXDCTL   0x02828
#define I82544_RXCSUM   0x05000
#define I82544_MTA      0x05200
#define I82544_RAT      0x05400
#define I82544_VFTA     0x05600

#define I82544_RCTL_EN  (1<<1)
#define I82544_RCTL_BAM (1<<15)

// Transmit registers
#define I82544_TCTL     0x00400
#define I82544_TIPG     0x00410
#define I82544_TBT      0x00448
#define I82544_AIT      0x00458
#define I82544_TXDMAC   0x03000
#define I82544_TDBAL    0x03800
#define I82544_TDBAH    0x03804
#define I82544_TDLEN    0x03808
#define I82544_TDH      0x03810
#define I82544_TDT      0x03818
#define I82544_TIDV     0x03820
#define I82544_TXDCTL   0x03828
#define I82544_TSPMT    0x03830


#define I82544_TCTL_EN  (1<<1)
#define I82544_TCTL_PSP (1<<3)


// ------------------------------------------------------------------------
//
//               82544 DEVICE CONTROL WORD DEFNITIONS
//
// ------------------------------------------------------------------------

#define I82544_CTRL_FD          (1<<0)
#define I82544_CTRL_BEM         (1<<1)
#define I82544_CTRL_LRST        (1<<3)
#define I82544_CTRL_ASDE        (1<<5)
#define I82544_CTRL_SLU         (1<<6)
#define I82544_CTRL_ILOS        (1<<7)
#define I82544_CTRL_SPEED       (3<<8)
#define I82544_CTRL_FRCSPD      (1<<11)
#define I82544_CTRL_FRCDPLX     (1<<12)
#define I82544_CTRL_SWDPINSLO   (15<<18)
#define I82544_CTRL_SWDPINSIO   (15<<22)
#define I82544_CTRL_RST         (1<<26)
#define I82544_CTRL_RFCE        (1<<27)
#define I82544_CTRL_TFCE        (1<<28)
#define I82544_CTRL_VME         (1<<30)
#define I82544_CTRL_PHY_RST     (1<<31)

#define I82544_CTRL_PHY_RESET           (1<<18)
#define I82544_CTRL_PHY_RESET_DIR       (1<<22)
#define I82544_CTRL_MDIO                (1<<20)
#define I82544_CTRL_MDIO_DIR            (1<<24)
#define I82544_CTRL_MDC                 (1<<21)
#define I82544_CTRL_MDC_DIR             (1<<25)

#define I82544_CTRL_EXT_PHY_RESET4      (1<<4)
#define I82544_CTRL_EXT_PHY_RESET_DIR4  (1<<8)

#define PHY_ADDRESS 1

// ------------------------------------------------------------------------
//
//               82544 DEVICE STATUS WORD DEFNITIONS
//
// ------------------------------------------------------------------------

#define I82544_STATUS_FD        0x0001
#define I82544_STATUS_LU        0x0002
#define I82544_STATUS_TXOFF     0x0010
#define I82544_STATUS_TBIMODE   0x0020
#define I82544_STATUS_SPEED     0x00C0
#define I82544_STATUS_ASDV      0x0300
#define I82544_STATUS_PCI_SPD   0x0800
#define I82544_STATUS_BUS64     0x1000
#define I82544_STATUS_PCIX_MODE 0x2000
#define I82544_STATUS_PCIXSPD   0xC000

// ------------------------------------------------------------------------
//
//                   82544 EEPROM INTERFACE
//
// ------------------------------------------------------------------------

//  EEPROM_Ctrl bits.
#define EE_SHIFT_CLK	0x01            // EEPROM shift clock.
#define EE_CS		0x02            // EEPROM chip select.
#define EE_DATA_WRITE	0x04            // EEPROM chip data in.
#define EE_DATA_READ	0x08            // EEPROM chip data out.
#define EE_REQ          0x40            // EEPROM request (82546 only)
#define EE_GNT          0x80            // EEPROM grant   (82546 only)
#define EE_PRES         0x100           // EEPROM present (82546 only)
#define EE_SIZE         0x200           // EEPROM size    (82546 only)
#define EE_ENB		(0x10|EE_CS)


// ------------------------------------------------------------------------
//
//               RECEIVE DESCRIPTORS
//
// ------------------------------------------------------------------------

#define I82544_RD_BUFFER        0
#define I82544_RD_LENGTH        8
#define I82544_RD_CSUM          10
#define I82544_RD_STATUS        12
#define I82544_RD_ERRORS        13
#define I82544_RD_SPECIAL       14
#define I82544_RD_SIZE          16

#define I82544_RD_STATUS_DD     (1<<0)
#define I82544_RD_STATUS_EOP    (1<<1)
#define I82544_RD_STATUS_IXSM   (1<<2)
#define I82544_RD_STATUS_VP     (1<<3)
#define I82544_RD_STATUS_TCPCS  (1<<5)
#define I82544_RD_STATUS_IPCS   (1<<6)
#define I82544_RD_STATUS_PIF    (1<<7)

// ------------------------------------------------------------------------
//
//               TRANSMIT DESCRIPTORS
//
// ------------------------------------------------------------------------

// Currently we only use the legacy Tx descriptor

#define I82544_TD_BUFFER        0
#define I82544_TD_LENGTH        8
#define I82544_TD_CSO           10
#define I82544_TD_CMD           11
#define I82544_TD_STATUS        12
#define I82544_TD_CSS           13
#define I82544_TD_SPECIAL       14
#define I82544_TD_SIZE          16

#define I82544_TD_CMD_EOP       (1<<0)
#define I82544_TD_CMD_IFCS      (1<<1)
#define I82544_TD_CMD_IC        (1<<2)
#define I82544_TD_CMD_RS        (1<<3)
#define I82544_TD_CMD_RPS       (1<<4)
#define I82544_TD_CMD_DEXT      (1<<5)
#define I82544_TD_CMD_VLE       (1<<6)
#define I82544_TD_CMD_IDE       (1<<7)

#define I82544_TD_STATUS_DD     (1<<0)
#define I82544_TD_STATUS_EC     (1<<1)
#define I82544_TD_STATUS_LC     (1<<2)
#define I82544_TD_STATUS_TU     (1<<3)

// ------------------------------------------------------------------------
//
//                      DEVICES AND PACKET QUEUES
//
// ------------------------------------------------------------------------

#define MAX_RX_PACKET_SIZE  1536        // maximum Rx packet size
#define MAX_TX_PACKET_SIZE  1536        // maximum Tx packet size


// ------------------------------------------------------------------------
// Use arrays provided by platform header to verify pointers.

#ifdef CYGDBG_USE_ASSERTS
#define CHECK_NDP_SC_LINK()                                             \
    CYG_MACRO_START                                                     \
    int i, valid_netdev = 0, valid_sc = 0;                              \
    for(i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82544_DEV_COUNT; i++) {       \
        if (i82544_netdev_array[i] == ndp) valid_netdev = 1;            \
        if (i82544_sc_array[i] == sc) valid_sc = 1;                     \
        if (valid_sc || valid_netdev) break;                            \
    }                                                                   \
    CYG_ASSERT( valid_netdev, "Bad ndp" );                              \
    CYG_ASSERT( valid_sc, "Bad sc" );                                   \
    CYG_ASSERT( (void *)p_i82544 == i82544_sc_array[i]->driver_private, \
                "sc pointer bad" );                                     \
    CYG_MACRO_END
#else
#define CHECK_NDP_SC_LINK()
#endif

#define IF_BAD_82544( _p_ )                                             \
if (({                                                                  \
    int i, valid_p = 0;                                                 \
    for(i = 0; i < CYGNUM_DEVS_ETH_INTEL_I82544_DEV_COUNT; i++) {       \
        if (i82544_priv_array[i] == (_p_)) {                            \
            valid_p = 1;                                                \
            break;                                                      \
        }                                                               \
    }                                                                   \
    CYG_ASSERT(valid_p, "Bad pointer-to-i82544");                       \
    (!valid_p);                                                         \
}))

// ------------------------------------------------------------------------
//
// Managing the memory that is windowed onto the PCI bus
//
// ------------------------------------------------------------------------

static cyg_uint32 i82544_heap_size;
static cyg_uint8 *i82544_heap_base;
static cyg_uint8 *i82544_heap_free;

static void *mem_reserved_ioctl = (void*)0;
// uncacheable memory reserved for ioctl calls

// ------------------------------------------------------------------------
//
//                       FUNCTION PROTOTYPES
//
// ------------------------------------------------------------------------

static int pci_init_find_82544s(void);

static void i82544_reset(struct i82544* p_i82544);
static void i82544_setup(struct i82544* p_i82544);
static int eth_set_mac_address(struct i82544* p_i82544, cyg_uint8 *addr, int eeprom );

static void InitRxRing(struct i82544* p_i82544);
static void InitTxRing(struct i82544* p_i82544);

static cyg_uint32
eth_isr(cyg_vector_t vector, cyg_addrword_t data);

static int i82544_configure(struct i82544* p_i82544, int promisc, int oversized);

// debugging/logging only:
//void dump_txcb(TxCB* p_txcb);
void DisplayStatistics(void);
void update_statistics(struct i82544* p_i82544);
//void dump_rfd(RFD* p_rfd, int anyway );
void dump_all_rfds( int intf );
void dump_packet(cyg_uint8 *p_buffer, int length);

static void i82544_stop( struct eth_drv_sc *sc );

// ------------------------------------------------------------------------

static void
udelay(int delay)
{
    CYGACC_CALL_IF_DELAY_US(delay);
}

// ------------------------------------------------------------------------
// If we are demuxing for all interrupt sources, we must mask and unmask
// *all* interrupt sources together.

static inline int 
Mask82544Interrupt(struct i82544* p_i82544)
{
    cyg_drv_interrupt_mask(p_i82544->vector);

    return 1;
}

static inline void
UnMask82544Interrupt(struct i82544* p_i82544, int old)
{
    if (old & 1)
        cyg_drv_interrupt_unmask(p_i82544->vector);
}


static inline void
Acknowledge82544Interrupt(struct i82544* p_i82544)
{
    cyg_drv_interrupt_acknowledge(p_i82544->vector);
}

// ------------------------------------------------------------------------
// Memory management
//
// Simply carve off from the front of the PCI mapped window into real memory

static CYG_ADDRESS
pciwindow_mem_alloc(int size)
{
    CYG_ADDRESS p_memory;
    int _size = size;

#ifdef DEBUG
//    db_printf("pciwindow_mem_alloc %d\n",size);
#endif
    
    CYG_ASSERT(
        (CYGHWR_INTEL_I82544_PCI_MEM_MAP_BASE <= (int)i82544_heap_free)
        &&
        ((CYGHWR_INTEL_I82544_PCI_MEM_MAP_BASE + 
          CYGHWR_INTEL_I82544_PCI_MEM_MAP_SIZE) > (int)i82544_heap_free)
        &&
        (0 < i82544_heap_size)
        &&
        (CYGHWR_INTEL_I82544_PCI_MEM_MAP_SIZE >= i82544_heap_size)
        &&
        (CYGHWR_INTEL_I82544_PCI_MEM_MAP_BASE == (int)i82544_heap_base),
        "Heap variables corrupted" );

    p_memory = 0;
    size = (size + 3) & ~3;
    if ( (i82544_heap_free+size) < (i82544_heap_base+i82544_heap_size) ) {
        cyg_uint32 *p;
        p_memory = (CYG_ADDRESS)i82544_heap_free;
        i82544_heap_free += size;
        for ( p = (cyg_uint32 *)p_memory; _size > 0; _size -= 4 )
            *p++ = 0;
    }

    CYG_ASSERT(
        NULL == p_memory ||
        VIRT_TO_BUS( p_memory ) + size == VIRT_TO_BUS( i82544_heap_free ),
        "Discontiguous PCI memory in real addresses" );

    return p_memory;
}


// ------------------------------------------------------------------------
//
//                     MDIO
//
// Device-specific bit-twiddling and line driving side-effects

// CYGACC_CALL_IF_DELAY_US() drags in huge amounts of scheduler locking and
// the like 'cos it's a VV call!  We only want a delay of 1uS tops, so:

#define MII_DELAY() do { int z; for ( z = 0; z < 100; z++ ) ; } while (0)

#if 0
# define MII_PRINTF diag_printf
# define MII_STUFF "%4s | %4s | %4s | %4s [%08x]\n",    \
    (*_ctrl & (1<<20)) ? "MDIO" : "---",                     \
    (*_ctrl & (1<<24)) ? "Wr" : "Rd",                      \
    (*_ctrl & (1<<21)) ? "CLK" : "clk",                      \
    *_ctrl
#else
# define MII_PRINTF( foo )
# define MII_STUFF
#endif

static inline cyg_uint32 mii_init( int ioaddr )
{
    cyg_uint32 ctrl;
    cyg_uint32 *_ctrl = &ctrl;
    *_ctrl = INL( ioaddr + I82544_CTRL );    
    *_ctrl &=~ I82544_CTRL_MDC;    
    *_ctrl |= I82544_CTRL_MDC_DIR;
    *_ctrl &= ~I82544_CTRL_MDIO_DIR;
    *_ctrl &= ~I82544_CTRL_MDIO;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );    
    MII_PRINTF( "mii_init  : " MII_STUFF  );
    MII_DELAY();
    return *_ctrl;
}

static inline void mii_clock_up( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl |= I82544_CTRL_MDC;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_clock_up  : " MII_STUFF  );
    MII_DELAY();
}

static inline void mii_clock_down( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl &=~ I82544_CTRL_MDC;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_clock_down: " MII_STUFF  );
    MII_DELAY();
}

static inline void mii_read_mode( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl &= ~I82544_CTRL_MDIO_DIR;
    *_ctrl &= ~I82544_CTRL_MDIO;
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_read_mode : " MII_STUFF  );
    MII_DELAY();
}

static inline int mii_read_data_bit( int ioaddr, cyg_uint32 *_ctrl )
{
    *_ctrl = INL( ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_read_data : " MII_STUFF  );
    return I82544_CTRL_MDIO == (I82544_CTRL_MDIO & *_ctrl);
}

static inline void mii_write_data_bit( int ioaddr, int databit, cyg_uint32 *_ctrl )
{
    if ( databit )
        *_ctrl |= I82544_CTRL_MDIO;
    else
        *_ctrl &= ~I82544_CTRL_MDIO;
    *_ctrl |= I82544_CTRL_MDIO_DIR; // drive the mdio line
    OUTL( *_ctrl, ioaddr + I82544_CTRL );
    MII_PRINTF( "mii_write_data: " MII_STUFF  );
    MII_DELAY();
}

// Pass ioaddr around "invisibly"
#define MII_INIT()                cyg_uint32 _ctrl_val = mii_init(ioaddr); \
                                  cyg_uint32 *_ctrl = &_ctrl_val;

#define MII_CLOCK_UP()            mii_clock_up(ioaddr, _ctrl)
#define MII_CLOCK_DOWN()          mii_clock_down(ioaddr, _ctrl)
#define MII_READ_MODE()           mii_read_mode(ioaddr, _ctrl)
#define MII_READ_DATA_BIT()       mii_read_data_bit(ioaddr, _ctrl)
#define MII_WRITE_DATA_BIT( _d_ ) mii_write_data_bit(ioaddr,(_d_),_ctrl)

// ------------------------------------------------------------------------
//
//                     MDIO
//
// Management data over the MII interface - nasty hand driven serial stuff
//

static void mii_write_bits( int ioaddr, int val, int bitcount, cyg_uint32 *_ctrl )
{
    int i;
    // These are deliberately signed ints so that we can send an overlong
    // preamble if we want by saying "send -1 of width 40 bits" and relying
    // on sign extension.
    for ( i = bitcount - 1; i >= 0; i-- ) {
        MII_WRITE_DATA_BIT( (val >> i) & 1 );
        MII_DELAY();
        MII_CLOCK_UP();
        MII_CLOCK_DOWN();
    }
}

static int mii_read_bits( int ioaddr, int bitcount, cyg_uint32 *_ctrl )
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

#define MII_WRITE_BITS( val, bitcount ) mii_write_bits( ioaddr, val, bitcount, _ctrl )
#define MII_READ_BITS( bitcount )       mii_read_bits( ioaddr, bitcount, _ctrl )

// Now define subsections of the protocol in terms of the above

#define MII_WRITE_PREAMBLE()    MII_WRITE_BITS( -1, 32 )  // >32 x 1s
#define MII_WRITE_START()       MII_WRITE_BITS( 1, 2 )    // 01
#define MII_WRITE_WRITE_CMD()   MII_WRITE_BITS( 1, 2 )    // 01
#define MII_WRITE_READ_CMD()    MII_WRITE_BITS( 2, 2 )    // 10

#define MII_WRITE_PHY_ADDR(_p_) MII_WRITE_BITS( _p_, 5 )
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

static int mii_read_register( struct i82544 *p_i82544, int phy, int regnum )
{
    int value = 0;
    cyg_uint32 ioaddr = p_i82544->io_address;

    if( p_i82544->device == 0x1004 )
    {
        // An 82543, read MII register via software defined pins in
        // CTRL register.

        MII_INIT();
        MII_WRITE_PREAMBLE();
        MII_WRITE_START();
        MII_WRITE_READ_CMD();
        MII_WRITE_PHY_ADDR(phy);
        MII_WRITE_REGNUM( regnum );
        MII_READ_TURNAROUND();
        value = MII_READ_REGVAL();
        MII_IDLE();
    }
    else
    {
        // Others, read MII register via MDIC register.
        
        cyg_uint32 mdic = (2<<26) | (phy<<21) | (regnum<<16);

        OUTL( mdic, ioaddr + I82544_MDIC );

        // Wait for ready
        do
        {
            mdic = INL( ioaddr + I82544_MDIC );
        } while( (mdic & (1<<28)) == 0 );

        value = mdic & 0xFFFF;
    }
    return value;
}

#ifndef CYGHWR_DEVS_ETH_INTEL_I82544_USE_ASD
static void mii_write_register( struct i82544 *p_i82544, int phy, int regnum, int value )
{
    cyg_uint32 ioaddr = p_i82544->io_address;

    if( p_i82544->device == 0x1004 )
    {
        // An 82543, write MII register via software defined pins in
        // CTRL register.
        
        MII_INIT();
        MII_WRITE_PREAMBLE();
        MII_WRITE_START();
        MII_WRITE_WRITE_CMD();
        MII_WRITE_PHY_ADDR(phy);
        MII_WRITE_REGNUM( regnum );
        MII_WRITE_TURNAROUND();
        MII_WRITE_REGVAL( value );
        MII_IDLE();
    }
    else
    {
        // Others, write MII register via MDIC register.

        cyg_uint32 mdic = (1<<26) | (phy<<21) | (regnum<<16) | (value&0xFFFF);

        OUTL( mdic, ioaddr + I82544_MDIC );

        // Wait for ready
        do
        {
            mdic = INL( ioaddr + I82544_MDIC );
        } while( (mdic & (1<<28)) == 0 );
    }
}
#endif

#ifdef DEBUG
// dump out the PHY registers
static void show_phy( struct i82544 *p_i82544, int phy )
{
    int i;

    os_printf("PHY %d regs:",phy);
    for( i = 0; i < 32; i++ )
    {
        cyg_uint32 mdic;
        
        mdic = mii_read_register( p_i82544, phy, i );

        if( (i%8)==0 ) os_printf("\n");

        os_printf("%04x ",mdic);
    }
    os_printf("\n");        
}
#endif

// ------------------------------------------------------------------------
//
//                       GET EEPROM SIZE
//
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
//
// Serial EEPROM access  - much like the other Intel ethernet
//

// CYGACC_CALL_IF_DELAY_US() drags in huge amounts of scheduler locking and
// the like 'cos it's a VV call!  Waste of time, mostly.

#define EE_DELAY() do { int z; for ( z = 0; z < 10000; z++ ) ; } while (0)

#if 0
# define EE_PRINTF diag_printf
# define EE_STUFF "%4s | %4s | %4s | %4s [%08x]\n",     \
    (l & EE_SHIFT_CLK) ? "CLK"  : "clk",                      \
    (l & EE_CS) ? "eeCS" : "--",                       \
    (l & EE_DATA_WRITE) ? "eeDW" : "---",                      \
    (l & EE_DATA_READ) ? "eeDR" : "---",                      \
    l & 0xfffff
#else
# define EE_PRINTF( foo )
# define EE_STUFF
#endif


static inline void ee_select( int ioaddr, struct i82544 *p_i82544 )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    if (p_i82544->device == 0x1010 ||
        p_i82544->device == 0x100e) {
	// i82546 requires REQ/GNT before EEPROM access
	l |= EE_REQ;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();
	while ((l & EE_GNT) == 0)
	    l = INL( ioaddr + I82544_EECD );
    }
    l &= ~0x3f;
    l |= EE_ENB;
    OUTL( l, ioaddr + I82544_EECD );
    EE_DELAY();
    l |= EE_CS;
    OUTL( l, ioaddr + I82544_EECD );
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_select       : " EE_STUFF  );
    EE_DELAY();
}

static inline void ee_deselect( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD ) & ~0x3f;
    l |= EE_ENB;
    OUTL( l, ioaddr + I82544_EECD );
    EE_PRINTF( "ee_deselect 1   : " EE_STUFF  );
    EE_DELAY();
    EE_DELAY();
    EE_DELAY();
    l &= ~EE_CS;
    OUTL( l, ioaddr + I82544_EECD );
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_deselect 2   : " EE_STUFF  );
    EE_DELAY();
    EE_DELAY();
    EE_DELAY();
    if (l & EE_REQ) {
	l &= ~EE_REQ;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();
    }
}

static inline void ee_clock_up( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    l |= EE_SHIFT_CLK;
    OUTL( l, ioaddr + I82544_EECD );
    EE_DELAY();
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_clock_up     : " EE_STUFF  );
    EE_DELAY();
}

static inline void ee_clock_down( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    l &=~ EE_SHIFT_CLK;
    OUTL( l, ioaddr + I82544_EECD );
    EE_DELAY();
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_clock_down   : " EE_STUFF  );
    EE_DELAY();
}

static inline int ee_read_data_bit( int ioaddr )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_read_data    : " EE_STUFF  );
    return EE_DATA_READ == (EE_DATA_READ & l);
}

static inline void ee_write_data_bit( int ioaddr, int databit )
{
    cyg_uint32 l;
    l = INL( ioaddr + I82544_EECD );
    if ( databit )
        l |= EE_DATA_WRITE;
    else
        l &= ~EE_DATA_WRITE;
    OUTL( l, ioaddr + I82544_EECD );
    l = INL( ioaddr + I82544_EECD );
    EE_PRINTF( "ee_write_data   : " EE_STUFF  );
    EE_DELAY();
}

// Pass ioaddr around "invisibly"
#define EE_SELECT()              ee_select(ioaddr, p_i82544)
#define EE_DESELECT()            ee_deselect(ioaddr)
#define EE_CLOCK_UP()            ee_clock_up(ioaddr)
#define EE_CLOCK_DOWN()          ee_clock_down(ioaddr)
#define EE_READ_DATA_BIT()       ee_read_data_bit(ioaddr)
#define EE_WRITE_DATA_BIT( _d_ ) ee_write_data_bit(ioaddr,(_d_))

// ------------------------------------------------------------------------

static int
get_eeprom_size( struct i82544 *p_i82544 )
{
    cyg_uint32 l, ioaddr = p_i82544->io_address;
    int i, tmp, addrbits;

    l = INL( ioaddr + I82544_EECD );

#ifdef DEBUG_EE
    diag_printf( "get_eeprom_size\n" );
#endif

    if (p_i82544->device == 0x1010 ||
        p_i82544->device == 0x100e) {
	
	l |= EE_REQ;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();
	while ((l & EE_GNT) == 0)
	    l = INL( ioaddr + I82544_EECD );
	l &= ~0x3f;
	l |= EE_ENB;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();
	l |= EE_CS;
	OUTL( l, ioaddr + I82544_EECD );
	l = INL( ioaddr + I82544_EECD );
	EE_DELAY();

	for (i = 3; i >= 0; i--) { // Doc says to shift out a zero then:
	    tmp = (6 & (1 << i)) ? 1 : 0; // "6" is the "read" command.
	    EE_WRITE_DATA_BIT(tmp);
	    EE_CLOCK_UP();
	    EE_CLOCK_DOWN();
	}
	// Now clock out address zero, looking for the dummy 0 data bit
	for ( i = 1; i <= 10; i++ ) {
	    EE_WRITE_DATA_BIT(0);
	    EE_CLOCK_UP();
	    EE_CLOCK_DOWN();
	    if (EE_READ_DATA_BIT() == 0)
		break; // The dummy zero est arrive'
	}

	if (6 != i && 8 != i)
	    diag_printf("no EEPROM found\n");

	addrbits = i;
        
	tmp = 0;
	for (i = 15; i >= 0; i--) {
	    EE_CLOCK_UP();
	    if (EE_READ_DATA_BIT())
		tmp |= (1<<i);
	    EE_CLOCK_DOWN();
	}

	l = INL( ioaddr + I82544_EECD ) & ~0x3f;
	l |= EE_ENB;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();
	EE_DELAY();
	EE_DELAY();
	l &= ~EE_CS;
	OUTL( l, ioaddr + I82544_EECD );
	l = INL( ioaddr + I82544_EECD );
	EE_DELAY();
	EE_DELAY();
	EE_DELAY();

	l &= ~EE_REQ;
	OUTL( l, ioaddr + I82544_EECD );
	EE_DELAY();

	return addrbits;
    }

    return 6;
}

static int
read_eeprom_word( struct i82544 *p_i82544, int addrbits, int address )
{
    int i, tmp;
    cyg_uint32 ioaddr = p_i82544->io_address;

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

//        CYG_ASSERT( (0 == tmp) == (0 == i), "Looking for zero handshake bit" );
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
//    diag_printf( "eeprom address %4x: data %4x\n", address, tmp );
#endif

    return tmp;
}

// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
//
//                NETWORK INTERFACE INITIALIZATION
//
//  Function : Init82544
//
//  Description :
//       This routine resets, configures, and initializes the chip.
//       It also clears the ethernet statistics structure, and selects
//       which statistics are supported by this driver.
//
// ------------------------------------------------------------------------

static bool
i82544_init(struct cyg_netdevtab_entry * ndp)
{
    static int initialized = 0; // only probe PCI et al *once*

    struct eth_drv_sc *sc;
    cyg_uint32 ioaddr;
    int count;
    struct i82544 *p_i82544;
    cyg_uint8 mac_address[ETHER_ADDR_LEN];

#ifdef DEBUG
    db_printf("i82544_init\n");
#endif

    sc = (struct eth_drv_sc *)(ndp->device_instance);
    p_i82544 = (struct i82544 *)(sc->driver_private);

    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "Bad device private pointer %x\n", sc->driver_private );
#endif
        return 0;
    }

    CHECK_NDP_SC_LINK();

    if ( 0 == initialized++ ) {
        // then this is the first time ever:
        if ( ! pci_init_find_82544s() ) {
#ifdef DEBUG
            os_printf( "pci_init_find_82544s failed\n" );
#endif
            return 0;
        }
    }

    // If this device is not present, exit
    if (0 == p_i82544->found)
        return 0;

    p_i82544->mac_addr_ok = 0;

    ioaddr = p_i82544->io_address; // get I/O address for 82544

#ifdef DEBUG
    os_printf("Init82544 %d @ %x\n", p_i82544->index, (int)ndp);
#endif

    // Reset device
    i82544_reset(p_i82544);

    i82544_setup(p_i82544);
        
    InitRxRing(p_i82544);
    InitTxRing(p_i82544);

    
    if (p_i82544->hardwired_esa) {
        // Hardwire the address without consulting the EEPROM.
        // When this flag is set, the p_i82544 will already contain
        // the ESA. Copy it to a mac_address for call to set_mac_addr
        mac_address[0] = p_i82544->mac_address[0];
        mac_address[1] = p_i82544->mac_address[1];
        mac_address[2] = p_i82544->mac_address[2];
        mac_address[3] = p_i82544->mac_address[3];
        mac_address[4] = p_i82544->mac_address[4];
        mac_address[5] = p_i82544->mac_address[5];

        eth_set_mac_address(p_i82544, mac_address, 0);

    } else {

        // Acquire the ESA either from extenal means (probably RedBoot
        // variables) or from the attached EEPROM - if there is one.

#ifdef CYGHWR_DEVS_ETH_INTEL_I82544_GET_ESA
        int ok = false;
        CYGHWR_DEVS_ETH_INTEL_I82544_GET_ESA( p_i82544, mac_address, ok );
        if ( ok )
            eth_set_mac_address(p_i82544, mac_address, 0);

#else // ! CYGHWR_DEVS_ETH_INTEL_I82544_GET_ESA

#ifndef CYGHWR_DEVS_ETH_INTEL_I82544_HAS_NO_EEPROM
        int addr_length, i;
        cyg_uint16 checksum;

        // read eeprom and get 82544's mac address
        addr_length = get_eeprom_size(p_i82544);
        // (this is the length of the *EEPROM*s address, not MAC address)

        // If length is 1, it _probably_ means there's no EEPROM
        // present.  Couldn't find an explicit mention of this in the
        // docs, but length=1 appears to be the behaviour in that case.
        if (1 == addr_length) {
#ifdef DEBUG_EE
            os_printf("Error: No EEPROM present for device %d\n", 
                      p_i82544->index);
#endif
        } else {
            for (checksum = 0, i = 0, count = 0; count < 64; count++) {
                cyg_uint16 value;
                // read word from eeprom
                value = read_eeprom_word(p_i82544, addr_length, count);
#ifdef DEBUG_EE
                os_printf( "%02x: %04x\n", count, value );
#endif
                checksum += value;
                if (count < 3) {
                    mac_address[i++] = value & 0xFF;
                    mac_address[i++] = (value >> 8) & 0xFF;
                }
            }

#ifndef CYGHWR_DEVS_ETH_INTEL_I82544_HAS_ONE_EEPROM_WITHOUT_CRC
            // If the EEPROM checksum is wrong, the MAC address read
            // from the EEPROM is probably wrong as well. In that
            // case, we don't set mac_addr_ok, but continue the
            // initialization. If then somebody calls i82544_start
            // without calling eth_set_mac_address() first, we refuse
            // to bring up the interface, because running with an
            // invalid MAC address is not a very brilliant idea.
        
            if ((checksum & 0xFFFF) != 0xBABA)  {
                // selftest verified checksum, verify again
#ifdef DEBUG_EE
                os_printf("Warning: Invalid EEPROM checksum %04X for device %d\n",
                          checksum, p_i82544->index);
#endif
            } else // trailing block
#endif
            {
                p_i82544->mac_addr_ok = 1;
#ifdef DEBUG_EE
                os_printf("Valid EEPROM checksum\n");
#endif
		// Second port of dual-port 82546 uses EEPROM ESA | 1
		if (p_i82544->device == 0x1010 ||
                    p_i82544->device == 0x100e) {
		    cyg_uint8 devfn = CYG_PCI_DEV_GET_DEVFN(p_i82544->devid);
		    if (CYG_PCI_DEV_GET_FN(devfn) == 1)
			mac_address[5] |= 1;
		}
                eth_set_mac_address(p_i82544, mac_address, 0);
            }
        }

        // record the MAC address in the device structure
        p_i82544->mac_address[0] = mac_address[0];
        p_i82544->mac_address[1] = mac_address[1];
        p_i82544->mac_address[2] = mac_address[2];
        p_i82544->mac_address[3] = mac_address[3];
        p_i82544->mac_address[4] = mac_address[4];
        p_i82544->mac_address[5] = mac_address[5];

#endif // ! CYGHWR_DEVS_ETH_INTEL_I82544_HAS_NO_EEPROM
#endif // ! CYGHWR_DEVS_ETH_INTEL_I82544_GET_ESA
    }

#ifdef DEBUG
    os_printf("i82544_init: MAC Address = %02X %02X %02X %02X %02X %02X\n",
              p_i82544->mac_address[0], p_i82544->mac_address[1],
              p_i82544->mac_address[2], p_i82544->mac_address[3],
              p_i82544->mac_address[4], p_i82544->mac_address[5]);
#endif
    
    // and record the net dev pointer
    p_i82544->ndp = (void *)ndp;
    
    p_i82544->within_send = 0; // init recursion level

    // Initialize upper level driver
    if ( p_i82544->mac_addr_ok )
        (sc->funs->eth_drv->init)(sc, &(p_i82544->mac_address[0]) );
    else
    {
        (sc->funs->eth_drv->init)(sc, NULL );
    }


#ifdef DEBUG

    os_printf("CTRL %08x\n",INL( ioaddr + I82544_CTRL ));    
    os_printf("STATUS %08x\n",INL( ioaddr + I82544_STATUS ));    
    
#endif    
    
    return (1);
}

// ------------------------------------------------------------------------
//
//  Function : i82544_setup
//
// ------------------------------------------------------------------------

static void 
i82544_setup( struct i82544 *p_i82544 )
{
    cyg_uint32 ioaddr;    
    cyg_uint32 ctrl;
#ifndef CYGHWR_DEVS_ETH_INTEL_I82544_USE_ASD
    cyg_uint32 ctrl_ext;
#endif

    ioaddr = p_i82544->io_address; // get 82544's I/O address

#ifdef CYGHWR_DEVS_ETH_INTEL_I82544_USE_ASD
    // Use Auto-negotiation

    ctrl = INL( ioaddr + I82544_CTRL );
    
    // Set link up bit
    ctrl |= I82544_CTRL_SLU | I82544_CTRL_ASDE;
    ctrl &= ~(I82544_CTRL_ILOS | I82544_CTRL_FRCSPD | I82544_CTRL_FRCDPLX);
    OUTL( ctrl, ioaddr + I82544_CTRL );
    udelay(20);

    // we can assume link is up with autonegotiation
    p_i82544->link = 1;

    // wait up to 5 seconds for link to come up
    {
	int delay_cnt = 500;
	while ((mii_read_register( p_i82544, PHY_ADDRESS, 1 ) & 0x4) == 0) {
	    udelay(10000);
	    if (--delay_cnt <= 0)
		break;
	}
    }
    
#else
    // The following sequence of resets and bit twiddling seem to be
    // necessary to get the 82543 working. Not sure what is necessary
    // for the 82544.

    ctrl = INL( ioaddr + I82544_CTRL );
    
    // Set link up bit
    ctrl |= I82544_CTRL_SLU;    
    ctrl &= ~I82544_CTRL_ILOS;
    OUTL( ctrl, ioaddr + I82544_CTRL );
    udelay(20);

    // Force PHY physical reset
    // We can only access the PHY after we have done this.

    ctrl_ext = INL( ioaddr + I82544_CTRL_EXT );
    ctrl_ext |= I82544_CTRL_EXT_PHY_RESET_DIR4;
    OUTL( ctrl_ext, ioaddr + I82544_CTRL_EXT );
    udelay( 20000 );

    ctrl_ext = INL( ioaddr + I82544_CTRL_EXT );    
    ctrl_ext &= ~I82544_CTRL_EXT_PHY_RESET4;
    OUTL( ctrl_ext, ioaddr + I82544_CTRL_EXT );
    udelay( 20000 );

    ctrl_ext = INL( ioaddr + I82544_CTRL_EXT );    
    ctrl_ext |= I82544_CTRL_EXT_PHY_RESET4;
    OUTL( ctrl_ext, ioaddr + I82544_CTRL_EXT );
    udelay( 20000 );

#ifdef DEBUG    
    show_phy( p_i82544, PHY_ADDRESS );
#endif
    
#if 0
    // Reset PHY
    // Does not appear to be necessary.
    {
        cyg_uint16 phy_ctrl;        
        phy_ctrl = mii_read_register( p_i82544, PHY_ADDRESS, 0 );
        phy_ctrl = 0x9000;
//        os_printf("PHY ctrl %04x\n",phy_ctrl);
        mii_write_register( p_i82544, PHY_ADDRESS, 0, phy_ctrl );
        do {
            phy_ctrl = mii_read_register( p_i82544, PHY_ADDRESS, 0 );
//            os_printf("PHY ctrl %04x\n",phy_ctrl);
        } while( phy_ctrl & 0x8000 );
    }
    show_phy( p_i82544, PHY_ADDRESS );
#endif

#if 0
    // Tinker with PHY configuration.
    // Only on 82543? May not be necessary at all, since disabling
    // this does not see to make any difference.
    {
        cyg_uint16 data;

        // Set CRS on Tx bit in PHY specific CR
        data = mii_read_register( p_i82544, PHY_ADDRESS, 16 );
        os_printf("PSCR %04x\n",data);
        data |= 0x0800;
        mii_write_register( p_i82544, PHY_ADDRESS, 16, data );

        // Set TX clock to 25MHz in PHY extended CR
        data = mii_read_register( p_i82544, PHY_ADDRESS, 20 );
        os_printf("PSECR %04x\n",data);
        data |= 0x0070;
        mii_write_register( p_i82544, PHY_ADDRESS, 20, data );
    }
    show_phy( p_i82544, PHY_ADDRESS );
#endif
    
#if 1
    // Force speed renegotiation.

    {
        cyg_uint16 phy_ctrl;
        cyg_uint16 phy_stat;
	int delay_cnt = 100 * 5;  // wait five seconds, then give up
        
	p_i82544->link = 0;
        phy_ctrl = mii_read_register( p_i82544, PHY_ADDRESS, 0 );
        phy_ctrl |= 0x1200;
//        os_printf("PHY ctrl %04x\n",phy_ctrl);
        mii_write_register( p_i82544, PHY_ADDRESS, 0, phy_ctrl );
        // Wait for it to complete
        do {
            udelay(10000);
            phy_stat = mii_read_register( p_i82544, PHY_ADDRESS, 1 );
            phy_stat = mii_read_register( p_i82544, PHY_ADDRESS, 1 );
        } while( (phy_stat & 0x0020) == 0 && (delay_cnt-- > 0) );

        if (phy_stat & 0x0020)
            p_i82544->link = 1;
    }

#ifdef DEBUG    
    show_phy( p_i82544, PHY_ADDRESS );
#endif    
#endif
    
#if 0
    // Reset link
    OUTL( ctrl | I82544_CTRL_LRST, ioaddr + I82544_CTRL );
    udelay(20);
    OUTL( ctrl, ioaddr + I82544_CTRL );
    udelay(20);
    show_phy( p_i82544, PHY_ADDRESS );
#endif


    
    // Transfer speed and duplicity settings from PHY to MAC

    // In theory the MAC is supposed to auto-configure from what the
    // PHY has autonegotiated. In practice this does not seem to work
    // (on the 82543 at least, it always thinks it is 1000MHz full
    // duplex) and we have to transfer the settings from the PHY by
    // hand. Additionally, the settings in the PHY ctrl register seem
    // bogus, so we read the values out of the PHY specific status
    // register instead.
    
    if (p_i82544->link) {
        cyg_uint16 phy_pssr;
        
        phy_pssr = mii_read_register( p_i82544, PHY_ADDRESS, 17 );

        ctrl = INL( ioaddr + I82544_CTRL );
//        os_printf("CTRL %08x\n",ctrl);
        ctrl &= ~(I82544_CTRL_SPEED | I82544_CTRL_FD);
        if( phy_pssr & (1<<13) )
            ctrl |= I82544_CTRL_FD;

        // Transfer speed
        ctrl |= ((phy_pssr>>14)&3)<<8;

        ctrl |= I82544_CTRL_FRCDPLX | I82544_CTRL_FRCSPD;

        OUTL( ctrl, ioaddr + I82544_CTRL );
//        os_printf("CTRL %08x\n",ctrl);
    }

    
#if 0
#ifdef DEBUG
    {
        int status = i82544_status( sc );
        static int speed[4] = { 10, 100, 1000, 1000 };
        os_printf("i82544_start %d flg %x Link = %s, %d Mbps, %s Duplex\n",
                  p_i82544->index,
                  *(int *)p_i82544,
                  status & GEN_STATUS_LINK ? "Up" : "Down",
                  speed[(status & GEN_STATUS_BPS)>>GEN_STATUS_BPS_SHIFT],
                  status & GEN_STATUS_FDX ? "Full" : "Half");
    }
#endif
#endif

    // Hang around here for a bit to let the device settle down. We
    // don't seem to get away without this.
    
    udelay( 1000000 );
    
#if 0

    // Having done all that, the interface still does not work
    // properly, UNLESS I now wait >= 45 seconds here. After that it
    // seems happy. I cannot find any difference in the state of the
    // PHY or the 82543 to explain this.
    
    show_phy( p_i82544, PHY_ADDRESS );
    os_printf("CTRL %08x\n",INL( ioaddr + I82544_CTRL ));    
    os_printf("STATUS %08x\n",INL( ioaddr + I82544_STATUS ));    
    os_printf("ICR %08x\n",INL( ioaddr + I82544_ICR ));    
    os_printf("RCTL %08x\n",INL( ioaddr + I82544_RCTL ));    
    os_printf("TCTL %08x\n",INL( ioaddr + I82544_TCTL ));    

    os_printf("Waiting 45 seconds\n");
    {
        int i;
        cyg_uint32 status = INL( ioaddr + I82544_STATUS );
//        for( i = 0; i < 60; i++ )       // works
//        for( i = 0; i < 45; i++ )       // works
//        for( i = 0; i < 40; i++ )       // fails
//        for( i = 0; i < 35; i++ )       // fails
//        for( i = 0; i < 30; i++ )       // fails
        {
            cyg_uint32 s;
            PC_WRITE_SCREEN_32( 60, i );
            udelay(1000000);
            s = INL( ioaddr + I82544_STATUS );
            if( s != status )
            {
                os_printf("%d STATUS change %08x\n",i,s);
                status = s;
            }
        }
    }

    show_phy( p_i82544, PHY_ADDRESS );
    os_printf("CTRL %08x\n",INL( ioaddr + I82544_CTRL ));    
    os_printf("STATUS %08x\n",INL( ioaddr + I82544_STATUS ));    
    os_printf("ICR %08x\n",INL( ioaddr + I82544_ICR ));    
    os_printf("RCTL %08x\n",INL( ioaddr + I82544_RCTL ));    
    os_printf("TCTL %08x\n",INL( ioaddr + I82544_TCTL ));    
    
#endif
#endif // CYGHWR_DEVS_ETH_INTEL_I82544_USE_ASD

    // Set up interrupts

    // Clear any pending interrupts
    ctrl = INL( ioaddr + I82544_ICR );

    // clear all mask bits
    OUTL( 0xFFFFFFFF, ioaddr + I82544_IMC );

    // Set interrupt bits for:
    // 1 = Transmit queue empty
    // 7 = Receiver timeout interrupt
    OUTL( (1<<1)|(1<<7), ioaddr + I82544_IMS );
        
}

// ------------------------------------------------------------------------
//
//  Function : i82544_start
//
// ------------------------------------------------------------------------

static void 
i82544_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags )
{
    struct i82544 *p_i82544;
    cyg_uint32 ioaddr;
    cyg_uint32 txctl, rxctl;
    
#ifdef CYGPKG_NET
    struct ifnet *ifp = &sc->sc_arpcom.ac_if;
#endif
#ifdef DEBUG
    db_printf("i82544_start\n");
#endif

    p_i82544 = (struct i82544 *)sc->driver_private;
    ioaddr = p_i82544->io_address; // get 82544's I/O address
    
    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "i82544_start: Bad device pointer %x\n", p_i82544 );
#endif
        return;
    }

    if ( ! p_i82544->mac_addr_ok ) {
#ifdef DEBUG
        os_printf("i82544_start %d: invalid MAC address, "
                  "can't bring up interface\n",
                  p_i82544->index );
#endif
        return;
    }

    if ( p_i82544->active )
        i82544_stop( sc );

    // Enable device
    p_i82544->active = 1;

    /* Enable promiscuous mode if requested, reception of oversized frames always.
     * The latter is needed for VLAN support and shouldn't hurt even if we're not
     * using VLANs.
     */
    i82544_configure(p_i82544, 0
#ifdef CYGPKG_NET
                     || !!(ifp->if_flags & IFF_PROMISC)
#endif
#ifdef ETH_DRV_FLAGS_PROMISC_MODE
                     || !!(flags & ETH_DRV_FLAGS_PROMISC_MODE)
#endif
                     , 1);

    // enable receiver
    rxctl = INL( ioaddr + I82544_RCTL );
    rxctl |= I82544_RCTL_EN;
    OUTL( rxctl, ioaddr + I82544_RCTL );

    // Enable transmitter
    txctl = INL( ioaddr + I82544_TCTL );
    txctl |= I82544_TCTL_EN;
    OUTL( txctl, ioaddr + I82544_TCTL );
    
}

// ------------------------------------------------------------------------
//
//  Function : i82544_status
//
// ------------------------------------------------------------------------
int
i82544_status( struct eth_drv_sc *sc )
{
    int status;
    struct i82544 *p_i82544;
    cyg_uint32 ioaddr;
#ifdef DEBUG
    db_printf("i82544_status\n");
#endif
    
    p_i82544 = (struct i82544 *)sc->driver_private;
    
    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "i82544_status: Bad device pointer %x\n", p_i82544 );
#endif
        return 0;
    }

    ioaddr = p_i82544->io_address; // get 82544's I/O address

    status = INL(ioaddr + I82544_STATUS);

    return status;
}

// ------------------------------------------------------------------------
//
//  Function : BringDown82544
//
// ------------------------------------------------------------------------

static void
i82544_stop( struct eth_drv_sc *sc )
{
    struct i82544 *p_i82544;
    cyg_uint32 ioaddr;
    cyg_uint32 txctl, rxctl;
    
#ifdef DEBUG
    db_printf("i82544_stop\n");
#endif
    
    p_i82544 = (struct i82544 *)sc->driver_private;

    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "i82544_stop: Bad device pointer %x\n", p_i82544 );
#endif
        return;
    }
   
#ifdef DEBUG
    os_printf("i82544_stop %d flg %x\n", p_i82544->index, *(int *)p_i82544 );
#endif

    p_i82544->active = 0;               // stop people tormenting it

    ioaddr = p_i82544->io_address;
    
    // disable receiver
    rxctl = INL( ioaddr + I82544_RCTL );
    rxctl &= ~I82544_RCTL_EN;
    OUTL( rxctl, ioaddr + I82544_RCTL );

    // Enable transmitter
    txctl = INL( ioaddr + I82544_TCTL );
    txctl &= ~I82544_TCTL_EN;
    OUTL( txctl, ioaddr + I82544_TCTL );
    
}


// ------------------------------------------------------------------------
//
//  Function : InitRxRing
//
// ------------------------------------------------------------------------

static void
InitRxRing(struct i82544* p_i82544)
{
    int i;
    CYG_ADDRESS rxring;
    cyg_uint32 ioaddr = p_i82544->io_address;    
    cyg_uint32 rxctl;
    
#ifdef DEBUG_82544
    os_printf("InitRxRing %d\n", p_i82544->index);
#endif

    // Allocate array of Rx desriptors
    rxring = pciwindow_mem_alloc(
        MAX_RX_DESCRIPTORS * I82544_RD_SIZE + 32 );

    // assign ring structure, aligning it on a 16 byte boudary.
    p_i82544->rx_ring = (rxring + 15) & ~15;
    
    // Allocate and fill in buffer pointers
    for ( i = 0; i < MAX_RX_DESCRIPTORS; i++) {
        CYG_ADDRESS rd = p_i82544->rx_ring + (i*I82544_RD_SIZE);
        CYG_ADDRESS buf = pciwindow_mem_alloc(MAX_RX_PACKET_SIZE);
        WRITEMEM64( rd + I82544_RD_BUFFER, VIRT_TO_BUS(buf) );
    }

    // Set the receiver queue registers

    OUTL( VIRT_TO_BUS(p_i82544->rx_ring), ioaddr + I82544_RDBAL );
    OUTL( 0, ioaddr + I82544_RDBAH );
    OUTL( MAX_RX_DESCRIPTORS * I82544_RD_SIZE, ioaddr + I82544_RDLEN );
    OUTL( 0, ioaddr + I82544_RDH );
    OUTL( MAX_RX_DESCRIPTORS - 5, ioaddr + I82544_RDT );

    // zero out RAT
    
    for( i = 0; i < 32; i++ )
        OUTL( 0, ioaddr + I82544_RAT +(i*4) );

    // Zero out MTA
    for( i = 0; i < 128; i++ )
        OUTL( 0, ioaddr + I82544_MTA +(i*4) );
    
    // Set up receiver to accept broadcasts
    rxctl = INL( ioaddr + I82544_RCTL );
    rxctl |= I82544_RCTL_BAM;
    OUTL( rxctl, ioaddr + I82544_RCTL );

#ifdef DEBUG_82544
    os_printf("RCTL %08x\n",rxctl);
#endif
    
    p_i82544->next_rx_descriptor = 0;
    p_i82544->rx_pointer = 0;
}

// ------------------------------------------------------------------------
//
//  Function : PacketRxReady     (Called from delivery thread & foreground)
//
// ------------------------------------------------------------------------

static void
PacketRxReady(struct i82544* p_i82544)
{
    struct cyg_netdevtab_entry *ndp;
    struct eth_drv_sc *sc;
    cyg_int32 rxp;
    cyg_uint32 ioaddr;
    
#ifdef DEBUG_82544
//    db_printf("PacketRxReady\n");
#endif
    
    ndp = (struct cyg_netdevtab_entry *)(p_i82544->ndp);
    sc = (struct eth_drv_sc *)(ndp->device_instance);

    CHECK_NDP_SC_LINK();

    ioaddr = p_i82544->io_address;


    rxp = p_i82544->rx_pointer;
    
    for(;;)
    {
        cyg_int32 rxh, rxt;
        CYG_ADDRESS dp;
        cyg_uint8 status;

        rxh = INL( ioaddr + I82544_RDH );

#if 0 //def DEBUG_82544
        os_printf("rxp %04d rxh %04x\n",rxp,rxh);
#endif
        
        // If the head pointer has not advanced, there have been no
        // packets received.
        if( rxh == rxp )
            break;

        // Form packet address
        dp = p_i82544->rx_ring + (rxp * I82544_RD_SIZE);

        // Get status
        READMEM8( dp + I82544_RD_STATUS, status );

#if 0 //def DEBUG_82544
        {
            cyg_uint16 length;
            READMEM16( dp + I82544_RD_LENGTH, length );
            os_printf("rxp %04d status %02x length %d\n",rxp,status,length);
        }
#endif
        
        if( status & I82544_RD_STATUS_EOP )
        {
            cyg_uint16 length;

            READMEM16( dp + I82544_RD_LENGTH, length );

#ifdef DEBUG_82544
        os_printf("rxp %04d length %d\n",rxp,length);
#endif
            CYG_ASSERT( MAX_RX_PACKET_SIZE >= length, "Oversize Rx" );

            // tell the callback the right packet
            p_i82544->next_rx_descriptor = rxp;

#ifdef CYGPKG_NET
            if ( length > sizeof( struct ether_header ) )
            // then it is acceptable; offer the data to the network stack
#endif
            (sc->funs->eth_drv->recv)( sc, length );

            // All done!
        }

        // Advance rxp pointer
        rxp = ( rxp + 1 ) % MAX_RX_DESCRIPTORS;

        // We can now also advance the tail pointer by 1
        rxt = INL( ioaddr + I82544_RDT );
        dp = p_i82544->rx_ring + (rxt * I82544_RD_SIZE);
        WRITEMEM8( dp + I82544_RD_STATUS, status );
        rxt = ( rxt + 1 ) % MAX_RX_DESCRIPTORS;
        OUTL( rxt, ioaddr + I82544_RDT );
        
#ifdef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
	// Can't deliver more than one packet in polled standalone mode
	break;
#endif
    }

    // Save next rx pointer for next time.
    p_i82544->rx_pointer = rxp;
    
}

// and the callback function

static void 
i82544_recv( struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len )
{
    struct i82544 *p_i82544;
    cyg_int32 rxp;
    CYG_ADDRESS dp;
    CYG_ADDRESS from_p;
    cyg_uint16 total_len;
    struct eth_drv_sg *last_sg;
    
#ifdef DEBUG_82544
    db_printf("i82544_recv\n");
#endif
    
    p_i82544 = (struct i82544 *)sc->driver_private;
    
    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG_82544
        os_printf( "i82544_recv: Bad device pointer %x\n", p_i82544 );
#endif
        return;
    }

    rxp = p_i82544->next_rx_descriptor;
    // Form packet address
    dp = p_i82544->rx_ring + (rxp * I82544_RD_SIZE);

#if 0 //def DEBUG_82544
        {
            int i;
            os_printf("RxD %08x",dp);
            for( i = 0; i < 16; i++ )
            {
                cyg_uint8 b;
                if( (i%8) == 0 ) os_printf("\n");
                READMEM8( dp + i, b );
                os_printf("%02x ",b);
            }
            os_printf("\n");
        }
#endif        
    // Copy the data to the network stack
    READMEM64( dp + I82544_RD_BUFFER, from_p );
    from_p = BUS_TO_VIRT(from_p);
    READMEM16( dp + I82544_RD_LENGTH, total_len );

#ifdef DEBUG_82544
    db_printf("RXP: %04x len %d\n",rxp,total_len);
#endif
    
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

#if 0 //def DEBUG_82544
    {
        int i,ll = l;
        os_printf("Pkt len %d",l);
        if( ll > 32 ) ll = 32;
        for( i = 0; i < ll; i++ )
        {
            cyg_uint8 b;
            if( (i%8) == 0 ) os_printf("\n %04x: ",i);
            b = ((cyg_uint8 *)from_p)[i];
            os_printf("%02x ",b);
        }
        os_printf("\n");
        
    }
#endif
        memcpy( to_p, (unsigned char *)from_p, l );
        from_p += l;
        total_len -= l;
    }

    CYG_ASSERT( 0 == total_len, "total_len mismatch in rx" );
    CYG_ASSERT( last_sg == sg_list, "sg count mismatch in rx" );

}


// ------------------------------------------------------------------------
//
//  Function : InitTxRing
//
// ------------------------------------------------------------------------

static void
InitTxRing(struct i82544* p_i82544)
{
    int i;
    cyg_uint32 ioaddr = p_i82544->io_address;
    CYG_ADDRESS txring;
    cyg_uint32 txctl;
    
#ifdef DEBUG_82544
    os_printf("InitTxRing %d\n", p_i82544->index);
#endif

    // Allocate array of Tx desriptors
    txring = pciwindow_mem_alloc(
        MAX_TX_DESCRIPTORS * I82544_TD_SIZE + 32 );

    // assign ring structure, aligning it on a 16 byte boudary.
    p_i82544->tx_ring = (txring + 15) & ~15;
    
    // Allocate and fill in buffer pointers
    for ( i = 0; i < MAX_TX_DESCRIPTORS; i++) {
        CYG_ADDRESS td = p_i82544->tx_ring + (i*I82544_TD_SIZE);
        WRITEMEM64( td + I82544_TD_BUFFER, 0 );
    }

    // Set the transmitter queue registers

    OUTL( VIRT_TO_BUS(p_i82544->tx_ring), ioaddr + I82544_TDBAL );
    OUTL( 0, ioaddr + I82544_TDBAH );
    OUTL( MAX_TX_DESCRIPTORS * I82544_TD_SIZE, ioaddr + I82544_TDLEN );
    OUTL( 0, ioaddr + I82544_TDH );
    OUTL( 0, ioaddr + I82544_TDT );

    // Set IPG values
    OUTL( 8 | (8<<10) | (6<<20), ioaddr + I82544_TIPG );
    
    // Program tx ctrl register
    txctl = INL( ioaddr + I82544_TCTL );
    txctl |= (15<<4);   // collision threshold
    txctl |= (64<<12);  // collision distance
    txctl |= I82544_TCTL_PSP;
    OUTL( txctl, ioaddr + I82544_TCTL );

    p_i82544->tx_in_progress = 0;
    p_i82544->tx_pointer = 0;
        
}

// ------------------------------------------------------------------------
//
//  Function : TxDone          (Called from delivery thread)
//
// This returns Tx's from the Tx Machine to the stack (ie. reports
// completion) - allowing for missed interrupts, and so on.
// ------------------------------------------------------------------------

static void
TxDone(struct i82544* p_i82544)
{
    struct cyg_netdevtab_entry *ndp;
    struct eth_drv_sc *sc;
    cyg_uint32 ioaddr;
    cyg_int32 txp = p_i82544->tx_pointer;
    
#ifdef DEBUG_82544
//    db_printf("TxDone\n");
#endif
    
    ndp = (struct cyg_netdevtab_entry *)(p_i82544->ndp);
    sc = (struct eth_drv_sc *)(ndp->device_instance);

    CHECK_NDP_SC_LINK();

    ioaddr = p_i82544->io_address;      // get device I/O address

    if( !p_i82544->active )
        return;

    for(;;)
    {
        cyg_uint8 status;
        cyg_uint8 cmd;
        CYG_ADDRESS dp;
        cyg_int32 txh;
        
        txh = INL( ioaddr + I82544_TDH );

        // If there has been no advance on the transmit header,
        // nothing to do.
        if( txh == txp )
            break;

#ifdef DEBUG_82544
        os_printf("TxDone: TxH %04d TxP %04d\n",txh,txp);
#endif        
        
        // Get descriptor address
        dp = p_i82544->tx_ring + (txp * I82544_TD_SIZE);

        READMEM8( dp + I82544_TD_CMD, cmd );
        READMEM8( dp + I82544_TD_STATUS, status );
#ifdef DEBUG_82544
        os_printf("cmd %02x status %02x\n",cmd,status);
#endif        
        
        // Zero out buffer and command
        WRITEMEM64( dp + I82544_TD_BUFFER, 0 );
        WRITEMEM8( dp + I82544_TD_CMD, 0 );

        if( cmd & I82544_TD_CMD_EOP )
        {
            // A done end of packet descrptor

            if( p_i82544->tx_keys[txp] != 0 )
            {
                // Call network stack with correct value of txp in structure.
                // There may be recursive calls in here, so we need to make sure
                // that txp is updated correctly.
                p_i82544->tx_pointer = ( txp + 1 ) % MAX_TX_DESCRIPTORS;
                (sc->funs->eth_drv->tx_done)( sc, p_i82544->tx_keys[txp], 0 );
                txp = p_i82544->tx_pointer;
                continue;
            }
        }

        // Advance tx pointer
        txp = ( txp + 1 ) % MAX_TX_DESCRIPTORS;

    }

    // restore txp to data structure.
    p_i82544->tx_pointer = txp;

}


static cyg_bool
check_link(struct i82544 *p_i82544)
{
    if ( p_i82544->link == 0 ) 
    {
        cyg_uint16 phy_pssr;
        cyg_uint16 phy_stat;

	phy_stat = mii_read_register( p_i82544, PHY_ADDRESS, 1 );
        if (phy_stat & 0x20)
	{
            cyg_uint32 ioaddr;
	    cyg_uint32 ctrl;

	    p_i82544->link = 1;

	    ioaddr = p_i82544->io_address;      // get device I/O address

	    phy_pssr = mii_read_register( p_i82544, PHY_ADDRESS, 17 );

	    ctrl = INL( ioaddr + I82544_CTRL );
	    ctrl &= ~(I82544_CTRL_SPEED | I82544_CTRL_FD);
	    if( phy_pssr & (1<<13) )
		ctrl |= I82544_CTRL_FD;

	    // Transfer speed
	    ctrl |= ((phy_pssr>>14)&3)<<8;
	    ctrl |= I82544_CTRL_FRCDPLX | I82544_CTRL_FRCSPD;

	    OUTL( ctrl, ioaddr + I82544_CTRL );
	}
    }

    return p_i82544->link == 1;
}


// ------------------------------------------------------------------------
//
//  Function : i82544_can_send
//
// ------------------------------------------------------------------------

static int 
i82544_can_send(struct eth_drv_sc *sc)
{
    struct i82544 *p_i82544;
    cyg_uint32 ioaddr;
    
#ifdef DEBUG_82544
//    db_printf("i82544_can_send\n");
#endif

    p_i82544 = (struct i82544 *)sc->driver_private;

    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG_82544
        os_printf( "i82544_send: Bad device pointer %x\n", p_i82544 );
#endif
        return 0;
    }

    ioaddr = p_i82544->io_address;      // get device I/O address

    if ( p_i82544->active )
    {
        cyg_int32 txh, txt, diff;
        

	if (!check_link(p_i82544))
	    return 0;

        // Poll for Tx completion
        TxDone( p_i82544 );

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
	// We are not prepared to receive a packet now if we are in a polled
	// standalone configuration.

        // Poll for receptions
        PacketRxReady( p_i82544 );
#endif

        // Now see whether the Tx queue has space for another
        // transmit.  We look at the difference between the head and
        // tail pointer, and if there is space for at least 5 more
        // descriptors, we allow a new transmission to go ahead.
        txh = INL( ioaddr + I82544_TDH );
        txt = INL( ioaddr + I82544_TDT );

        diff = (txh-1) - txt;
        if( diff < 0 ) diff += MAX_TX_DESCRIPTORS;
#ifdef DEBUG_82544
//        os_printf("TxH %04d TxT %04d diff %04d\n",txh,txt,diff);
#endif
        return diff > 5;
    }
    
    return false;

}

// ------------------------------------------------------------------------
//
//  Function : i82544_send
//
// ------------------------------------------------------------------------

static void 
i82544_send(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list, int sg_len, int total_len,
            unsigned long key)
{
    struct i82544 *p_i82544;
    cyg_uint32 ioaddr;
    struct eth_drv_sg *last_sg;
    cyg_int32 txt;

    
#ifdef DEBUG_82544
    db_printf("i82544_send\n");
#endif

    p_i82544 = (struct i82544 *)sc->driver_private;

    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG_82544
        os_printf( "i82544_send: Bad device pointer %x\n", p_i82544 );
#endif
        return;
    }

#ifdef DEBUG_82544
    os_printf("Tx %d %x: %d sg's, %d bytes, KEY %x\n",
              p_i82544->index, (int)p_i82544, sg_len, total_len, key );
#endif

    if ( ! p_i82544->active )
        return;                         // device inactive, no return
    ioaddr = p_i82544->io_address;      // get device I/O address

    // Get the tx tail pointer
    txt = INL( ioaddr + I82544_TDT );
    
    for ( last_sg = &sg_list[sg_len]; sg_list < last_sg; sg_list++ )
    {
        cyg_uint8 *from_p;
        cyg_uint8 cmd = 0;
        CYG_ADDRESS dp;
        int l;

        from_p = (cyg_uint8 *)(sg_list->buf); // normal cached address
        l = sg_list->len;

        if ( l > total_len )
            l = total_len;

        // Get the descriptor address from the tail pointer.
        dp = p_i82544->tx_ring + (txt * I82544_TD_SIZE);

        total_len -= l;
        
#ifdef HAL_DCACHE_STORE
        HAL_DCACHE_STORE( ((CYG_ADDRESS)from_p) &~(HAL_DCACHE_LINE_SIZE-1),
                          l + (HAL_DCACHE_LINE_SIZE-1) );
#endif

        WRITEMEM64( dp + I82544_TD_BUFFER, VIRT_TO_BUS(from_p) );
        WRITEMEM16( dp + I82544_TD_LENGTH, l );

        // Set EOP bit on last packet
        if( total_len <= 0 )
            cmd |= I82544_TD_CMD_EOP;

        // Get status back
//        cmd |= I82544_TD_CMD_RPS | I82544_TD_CMD_RS;

        cmd |= I82544_TD_CMD_IFCS;
        
        // Write command byte
        WRITEMEM8( dp + I82544_TD_CMD, cmd );

        // Zero out rest of fields
        WRITEMEM8( dp + I82544_TD_STATUS, 0 );
        WRITEMEM8( dp + I82544_TD_CSO, 0 );
        WRITEMEM8( dp + I82544_TD_CSS, 0 );
        WRITEMEM16( dp + I82544_TD_SPECIAL, 0 );

        // Store the key for this transmission in the matching slot
        // for the descriptor.
        p_i82544->tx_keys[txt] = key;

        // Increment tx tail pointer
        txt = (txt + 1) % MAX_TX_DESCRIPTORS;
        
    }

    // And finally, cause the transmission to happen by setting the
    // tail pointer in the hardware.
    OUTL( txt, ioaddr + I82544_TDT );

}

// ------------------------------------------------------------------------
//
//  Function : i82544_reset
//
// ------------------------------------------------------------------------
static void
i82544_reset(struct i82544* p_i82544)
{
    cyg_uint32 ioaddr = p_i82544->io_address;
    cyg_uint32 ctrl;

#ifdef DEBUG
    db_printf("i82544_reset\n");
#endif

    ctrl = INL( ioaddr + I82544_CTRL );

    // reset controller
    OUTL( ctrl | I82544_CTRL_RST, ioaddr + I82544_CTRL );
    udelay(20);
    ctrl = INL( ioaddr + I82544_CTRL );

}

// ------------------------------------------------------------------------
//
//                       INTERRUPT HANDLERS
//
// ------------------------------------------------------------------------

static cyg_uint32
eth_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    struct i82544* p_i82544 = (struct i82544 *)data;
    cyg_uint16 status;
    cyg_uint32 ioaddr;

#ifdef DEBUG_82544
//    db_printf("eth_isr\n");
#endif
    
    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG_82544
        os_printf( "i82544_isr: Bad device pointer %x\n", p_i82544 );
#endif
        return 0;
    }

    ioaddr = p_i82544->io_address;

    status = INL( ioaddr + I82544_ICR );

#ifdef DEBUG_82544    
    db_printf("eth_isr %04x\n",status);
#endif    

    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);        // schedule DSR
}


// ------------------------------------------------------------------------

#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
static void
eth_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    struct i82544* p_i82544 = (struct i82544 *)data;
    struct cyg_netdevtab_entry *ndp =
        (struct cyg_netdevtab_entry *)(p_i82544->ndp);
    struct eth_drv_sc *sc = (struct eth_drv_sc *)(ndp->device_instance);

#ifdef DEBUG_82544
    db_printf("eth_dsr\n");
#endif
    
    eth_drv_dsr( vector, count, (cyg_addrword_t)sc );

    cyg_drv_interrupt_acknowledge(p_i82544->vector);
}
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE

// ------------------------------------------------------------------------
// Deliver routine:

void
i82544_deliver(struct eth_drv_sc *sc)
{
    struct i82544* p_i82544 = (struct i82544 *)(sc->driver_private);

#ifdef DEBUG
    db_printf("i82544_deliver\n");
#endif
    
    // First pass any rx data up the stack
    PacketRxReady(p_i82544);

    // Then scan for completed Tx and inform the stack
    TxDone(p_i82544);
}

// ------------------------------------------------------------------------
// Device table entry to operate the chip in a polled mode.
// Only diddle the interface we were asked to!

void
i82544_poll(struct eth_drv_sc *sc)
{
    struct i82544 *p_i82544;
    p_i82544 = (struct i82544 *)sc->driver_private;
    
#ifdef DEBUG
    db_printf("i82544_poll\n");
#endif
    
    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "i82544_poll: Bad device pointer %x\n", p_i82544 );
#endif
        return;
    }

    if (!check_link(p_i82544))
        return;

    // As it happens, this driver always requests the DSR to be called:
    (void)eth_isr( p_i82544->vector, (cyg_addrword_t)p_i82544 );

    // (no harm in calling this ints-off also, when polled)
    i82544_deliver( sc );
}

// ------------------------------------------------------------------------
// Determine interrupt vector used by a device - for attaching GDB stubs
// packet handler.

int
i82544_int_vector(struct eth_drv_sc *sc)
{
    struct i82544 *p_i82544;
    p_i82544 = (struct i82544 *)sc->driver_private;
    return (p_i82544->vector);
}


// ------------------------------------------------------------------------
//
//  Function : pci_init_find_82544s
//
// This is called exactly once at the start of time to:
//  o scan the PCI bus for objects
//  o record them in the device table
//  o acquire all the info needed for the driver to access them
//  o instantiate interrupts for them
//  o attach those interrupts appropriately
// ------------------------------------------------------------------------

static cyg_pci_match_func find_82544s_match_func;

// Intel 82543 and 82544 are virtually identical, with different
// dev codes
static cyg_bool
find_82544s_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
    return ((0x8086 == v) &&
            ((0x1004 == d) ||   // 82543
             (0x100d == d) ||   // 82543
             (0x1008 == d) ||   // 82544
             (0x1010 == d) ||   // 82546
             (0x100e == d)      // 82540EM
            )
           );
}

static int
pci_init_find_82544s( void )
{
    cyg_pci_device_id devid;
    cyg_pci_device dev_info;
    cyg_uint16 cmd;
    int device_index;
    int found_devices = 0;

#ifdef DEBUG
    db_printf("pci_init_find_82544s()\n");
#endif

    // allocate memory to be used in ioctls later
    if (mem_reserved_ioctl != (void*)0) {
#ifdef DEBUG
        db_printf("pci_init_find_82544s() called > once\n");
#endif
        return 0;
    }

    // First initialize the heap in PCI window'd memory
    i82544_heap_size = CYGHWR_INTEL_I82544_PCI_MEM_MAP_SIZE;
    i82544_heap_base = (cyg_uint8 *)CYGHWR_INTEL_I82544_PCI_MEM_MAP_BASE;
    i82544_heap_free = i82544_heap_base;

//    mem_reserved_ioctl = pciwindow_mem_alloc(MAX_MEM_RESERVED_IOCTL);     

    cyg_pci_init();
#ifdef DEBUG
    db_printf("Finished cyg_pci_init();\n");
#endif

    devid = CYG_PCI_NULL_DEVID;

    for (device_index = 0; 
         device_index < CYGNUM_DEVS_ETH_INTEL_I82544_DEV_COUNT;
         device_index++) {
        struct i82544 *p_i82544 = i82544_priv_array[device_index];

        p_i82544->index = device_index;

        // See above for find_82544s_match_func
        if (cyg_pci_find_matching( &find_82544s_match_func, NULL, &devid )) {
#ifdef DEBUG
            db_printf("eth%d = 8254x\n", device_index);
#endif
            cyg_pci_get_device_info(devid, &dev_info);

            p_i82544->interrupt_handle = 0; // Flag not attached.
            if (cyg_pci_translate_interrupt(&dev_info, &p_i82544->vector)) {
#ifdef DEBUG
                db_printf(" Wired to HAL vector %d\n", p_i82544->vector);
#endif
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
                cyg_drv_interrupt_create(
                    p_i82544->vector,
                    0,                  // Priority - unused
                    (CYG_ADDRWORD)p_i82544, // Data item passed to ISR & DSR
#ifdef CYGHWR_DEVS_ETH_INTEL_I82544_DEMUX_ALL
                    eth_mux_isr,        // ISR
#else
                    eth_isr,            // ISR
#endif
                    eth_dsr,            // DSR
                    &p_i82544->interrupt_handle, // handle to intr obj
                    &p_i82544->interrupt_object ); // space for int obj

                cyg_drv_interrupt_attach(p_i82544->interrupt_handle);

                // Don't unmask the interrupt yet, that could get us into a
                // race.
#ifdef CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT
                // ALSO attach it to MUX interrupt for multiplexed
                // interrupts.  This is for certain boards where the
                // PCI backplane is wired "straight through" instead of
                // with a rotation of interrupt lines in the different
                // slots.
                {
                    static cyg_handle_t mux_interrupt_handle = 0;
                    static cyg_interrupt mux_interrupt_object;

                    if ( ! mux_interrupt_handle ) {
#ifdef DEBUG
                        db_printf(" Also attaching to HAL vector %d\n", 
                                  CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT);
#endif
                        cyg_drv_interrupt_create(
                            CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT,
                            0,              // Priority - unused
                            (CYG_ADDRWORD)p_i82544,// Data item passed to ISR and DSR
                            eth_mux_isr,    // ISR
                            eth_dsr,        // DSR
                            &mux_interrupt_handle,
                            &mux_interrupt_object );
                        
                        cyg_drv_interrupt_attach(mux_interrupt_handle);
                    }
                }
#endif // CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
            }
            else {
                p_i82544->vector=0;
#ifdef DEBUG
                db_printf(" Does not generate interrupts.\n");
#endif
            }

            if (cyg_pci_configure_device(&dev_info)) {
#ifdef DEBUG
                int i;
                db_printf("Found device on bus %d, devfn 0x%02x:\n",
                          CYG_PCI_DEV_GET_BUS(devid),
                          CYG_PCI_DEV_GET_DEVFN(devid));

                if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
                    db_printf(" Note that board is active. Probed"
                              " sizes and CPU addresses invalid!\n");
                }
                db_printf(" Vendor    0x%04x", dev_info.vendor);
                db_printf("\n Device    0x%04x", dev_info.device);
                db_printf("\n Command   0x%04x, Status 0x%04x\n",
                          dev_info.command, dev_info.status);
                
                db_printf(" Class/Rev 0x%08x", dev_info.class_rev);
                db_printf("\n Header 0x%02x\n", dev_info.header_type);

                db_printf(" SubVendor 0x%04x, Sub ID 0x%04x\n",
                          dev_info.header.normal.sub_vendor, 
                          dev_info.header.normal.sub_id);

                for(i = 0; i < CYG_PCI_MAX_BAR; i++) {
                    db_printf(" BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                    db_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                              dev_info.base_size[i], dev_info.base_map[i]);
                }
                db_printf(" eth%d configured\n", device_index);
#endif
                found_devices++;
                p_i82544->found = 1;
                p_i82544->active = 0;
                p_i82544->devid = devid;
                p_i82544->device = dev_info.device;
                p_i82544->io_address = dev_info.base_map[0];
#ifdef DEBUG
                db_printf(" I/O address = 0x%08x device = %04x\n", dev_info.base_map[0],
                                                                   dev_info.device);
#endif

                // Don't use cyg_pci_set_device_info since it clears
                // some of the fields we want to print out below.
                cyg_pci_read_config_uint16(dev_info.devid,
                                           CYG_PCI_CFG_COMMAND, &cmd);
                cmd |= (CYG_PCI_CFG_COMMAND_IO         // enable I/O space
                        | CYG_PCI_CFG_COMMAND_MEMORY   // enable memory space
                        | CYG_PCI_CFG_COMMAND_MASTER); // enable bus master
                cyg_pci_write_config_uint16(dev_info.devid,
                                            CYG_PCI_CFG_COMMAND, cmd);

                // Now the PCI part of the device is configured, reset
                // it. This should make it safe to enable the
                // interrupt
                i82544_reset(p_i82544);

                // This is the indicator for "uses an interrupt"
                if (p_i82544->interrupt_handle != 0) {
                    cyg_drv_interrupt_acknowledge(p_i82544->vector);
#ifndef CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
                    cyg_drv_interrupt_unmask(p_i82544->vector);
#endif // !CYGPKG_IO_ETH_DRIVERS_STAND_ALONE
                }
#ifdef DEBUG
                db_printf(" **** Device enabled for I/O and Memory "
                          "and Bus Master\n");
#endif
            }
            else {
                p_i82544->found = 0;
                p_i82544->active = 0;
#ifdef DEBUG
                db_printf("Failed to configure device %d\n",device_index);
#endif
            }
        }
        else {
            p_i82544->found = 0;
            p_i82544->active = 0;
#ifdef DEBUG
            db_printf("eth%d not found\n", device_index);
#endif
        }
    }

    if (0 == found_devices)
        return 0;

#ifdef CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT
    // Now enable the mux shared interrupt if it is in use
    if (mux_interrupt_handle) {
        cyg_drv_interrupt_acknowledge(CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT);
        cyg_drv_interrupt_unmask(CYGNUM_DEVS_ETH_INTEL_I82544_SEPARATE_MUX_INTERRUPT);
    }
#endif

    // Now a delay to ensure the hardware has "come up" before you try to
    // use it.  Yes, really, the full 2 seconds.  It's only really
    // necessary if DEBUG is off - otherwise all that printout wastes
    // enough time.  No kidding.
    udelay( 2000000 );
    return 1;
}

// ------------------------------------------------------------------------
//
//  Function : i82544_configure
//
//  Return : 0 = It worked.
//           non0 = It failed.
// ------------------------------------------------------------------------

static int i82544_configure(struct i82544* p_i82544, int promisc, int oversized)
{
    cyg_uint32  ioaddr;
//    volatile CFG *ccs;
//    volatile cyg_uint8* config_bytes;
//    cyg_uint16 status;
//    int count;

    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "eth_set_promiscuous_mode: Bad device pointer %x\n",
                   p_i82544 );
#endif
        return -1;
    }

    ioaddr = p_i82544->io_address;  

    // Not currently supported

    return 0;
}

// ------------------------------------------------------------------------
//
//  Function : eth_set_mac_address
//
//  Return : 0 = It worked.
//           non0 = It failed.
// ------------------------------------------------------------------------

static int
eth_set_mac_address(struct i82544* p_i82544, cyg_uint8 *addr, int eeprom)
{
    cyg_uint32  ioaddr;
    cyg_uint32 maclo, machi;
    
#ifdef DEBUG
    db_printf("eth_set_mac_address\n");
#endif
    
    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "eth_set_mac_address : Bad device pointer %x\n",
                   p_i82544 );
#endif
        return -1;
    }

    ioaddr = p_i82544->io_address;      
    
    // Write MAC address to RAT[0]

    maclo = addr[0] | (addr[1]<<8) | (addr[2]<<16) | (addr[3]<<24);
    machi = (1<<31) | (0<<16) | addr[4] | (addr[5]<<8);

#ifdef DEBUG
    os_printf("setting: lo %08x hi %08x\n",maclo,machi);
#endif
    
    OUTL( maclo , ioaddr + I82544_RAT );
    OUTL( machi, ioaddr + I82544_RAT + 4 );

    
    // record the MAC address in the device structure
    p_i82544->mac_address[0] = addr[0];
    p_i82544->mac_address[1] = addr[1];
    p_i82544->mac_address[2] = addr[2];
    p_i82544->mac_address[3] = addr[3];
    p_i82544->mac_address[4] = addr[4];
    p_i82544->mac_address[5] = addr[5];
    p_i82544->mac_addr_ok = 1;

#ifdef DEBUG
    os_printf( "Set MAC Address = %02X %02X %02X %02X %02X %02X (ok %d)\n",
               p_i82544->mac_address[0],
               p_i82544->mac_address[1],
               p_i82544->mac_address[2],
               p_i82544->mac_address[3],
               p_i82544->mac_address[4],
               p_i82544->mac_address[5],
               p_i82544->mac_addr_ok       );
#endif

    return p_i82544->mac_addr_ok ? 0 : 1;
}

// ------------------------------------------------------------------------
//
//  Function : eth_get_mac_address
//
// ------------------------------------------------------------------------
#ifdef ETH_DRV_GET_MAC_ADDRESS
static int
eth_get_mac_address(struct i82544* p_i82544, char *addr)
{
#ifdef DEBUG
    db_printf("eth_get_mac_address\n");
#endif
    
    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "eth_get_mac_address : Bad device pointer %x\n",
                   p_i82544 );
#endif
        return -1;
    }

    memcpy( addr, (char *)(&p_i82544->mac_address[0]), 6 );
    return 0;
}
#endif
// ------------------------------------------------------------------------
//
//  Function : i82544_ioctl
//
// ------------------------------------------------------------------------
static int
i82544_ioctl(struct eth_drv_sc *sc, unsigned long key,
             void *data, int data_length)
{
    struct i82544 *p_i82544;

#ifdef DEBUG
    db_printf("i82544_ioctl\n");
#endif
    
    p_i82544 = (struct i82544 *)sc->driver_private;

    IF_BAD_82544( p_i82544 ) {
#ifdef DEBUG
        os_printf( "i82544_ioctl/control: Bad device pointer %x\n", p_i82544 );
#endif
        return -1;
    }

#ifdef ioctlDEBUG
    db_printf( "i82544_ioctl: device eth%d at %x; key is 0x%x, data at %x[%d]\n",
               p_i82544->index, p_i82544, key, data, data_length );
#endif

    switch ( key ) {

#ifdef ETH_DRV_SET_MAC_ADDRESS
    case ETH_DRV_SET_MAC_ADDRESS:
        if ( 6 != data_length )
            return -2;
        return eth_set_mac_address( p_i82544, data, 1 /* do write eeprom */ );
#endif

#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
        return eth_get_mac_address( p_i82544, data );
#endif

    default:
        break;
    }
    return -1;
}

// ------------------------------------------------------------------------

// EOF if_i82544.c
