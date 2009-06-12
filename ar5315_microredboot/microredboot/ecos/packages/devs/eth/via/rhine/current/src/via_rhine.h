#ifndef CYGONCE_DEVS_ETH_VIA_RHINE_H
#define CYGONCE_DEVS_ETH_VIA_RHINE_H
//==========================================================================
//
//      via_rhine.h
//
//      VIA Rhine Ethernet chip
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
// Date:         2001-05-30
// Purpose:      Hardware description of VIA Rhine series.
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_io.h>
#include <cyg/io/pci_hw.h>             // HAL_PCI_ macros

//------------------------------------------------------------------------
// Get macros from platform header
#define __WANT_CONFIG
#include CYGDAT_DEVS_ETH_VIA_RHINE_INL
#undef  __WANT_CONFIG

//------------------------------------------------------------------------
// Set to perms of:
// 0 disables all debug output
// 1 for process debug output
// 2 for added data IO output: get_reg, put_reg
// 4 for packet allocation/free output
// 8 for only startup status, so we can tell we're installed OK
#define DEBUG 0x00

#if DEBUG & 1
# define DEBUG_FUNCTION() do { diag_printf("%s\n", __FUNCTION__); } while (0)
#else
# define DEBUG_FUNCTION() do {} while(0)
#endif

// ------------------------------------------------------------------------
// Macros for keeping track of statistics
#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
#define KEEP_STATISTICS
#endif

#ifdef KEEP_STATISTICS
#define INCR_STAT( _x_ )        (cpd->stats. _x_ ++)
#else
#define INCR_STAT( _x_ )        CYG_EMPTY_STATEMENT
#endif

//------------------------------------------------------------------------
// Cache translation
#ifndef CYGARC_UNCACHED_ADDRESS
# define CYGARC_UNCACHED_ADDRESS(x) (x)
#endif

//------------------------------------------------------------------------
// Address translation
#ifndef HAL_PCI_CPU_TO_BUS
# error "HAL PCI support must define translation macros"
#endif

//------------------------------------------------------------------------
// Macros for accessing structure elements

#define _SU8( _base_, _offset_) \
        *((volatile cyg_uint8 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SU16( _base_, _offset_) \
        *((volatile cyg_uint16 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SU32( _base_, _offset_) \
        *((volatile cyg_uint32 *)((CYG_ADDRWORD)_base_+(_offset_)))

#define _SI8( _base_, _offset_) \
        *((volatile cyg_int8 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SI16( _base_, _offset_) \
        *((volatile cyg_int16 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SI32( _base_, _offset_) \
        *((volatile cyg_int32 *)((CYG_ADDRWORD)_base_+(_offset_)))

// ------------------------------------------------------------------------
// Macros for accessing controller registers
#ifndef HAL_PCI_IO_READ_UINT8
# define HAL_PCI_IO_READ_UINT8(addr, datum)   HAL_READ_UINT8(addr, datum)
# define HAL_PCI_IO_WRITE_UINT8(addr, datum)  HAL_WRITE_UINT8(addr, datum)
# define HAL_PCI_IO_READ_UINT16(addr, datum)  HAL_READ_UINT16(addr, datum)
# define HAL_PCI_IO_WRITE_UINT16(addr, datum) HAL_WRITE_UINT16(addr, datum)
# define HAL_PCI_IO_READ_UINT32(addr, datum)  HAL_READ_UINT32(addr, datum)
# define HAL_PCI_IO_WRITE_UINT32(addr, datum) HAL_WRITE_UINT32(addr, datum)
#endif

// ------------------------------------------------------------------------
// Control registers
#define RHINE_PAR0         0x00
#define RHINE_PAR1         0x01
#define RHINE_PAR2         0x02
#define RHINE_PAR3         0x03
#define RHINE_PAR4         0x04
#define RHINE_PAR5         0x05

#define RHINE_RCR          0x06
#define RHINE_TCR          0x07
#define RHINE_CR0          0x08
#define RHINE_CR1          0x09

#define RHINE_ISR          0x0c         // 16 bit
#define RHINE_IMR          0x0e         // 16 bit

#define RHINE_CUR_RX       0x18
#define RHINE_CUR_TX       0x1c


#define RHINE_PHYADR       0x6c
#define RHINE_MIISR        0x6d
#define RHINE_BCR0         0x6e
#define RHINE_BCR1         0x6f
#define RHINE_MIICR        0x70
#define RHINE_MIIAD        0x71
#define RHINE_MIIDATA      0x72         // 16 bit


#define RHINE_EECSR        0x74

#define RHINE_CFGA         0x78
#define RHINE_CFGB         0x79
#define RHINE_CFGC         0x7a
#define RHINE_CFGD         0x7b

#define RHINE_STICKYHW     0x83
#define RHINE_WOL_CR_CLR   0xa4
#define RHINE_WOL_CG_CLR   0xa7
#define RHINE_PWR_CSR_CLR  0xac

#define RHINE_RCR_RRSF     0x80
#define RHINE_RCR_RFT_64   0x00
#define RHINE_RCR_RFT_SF   0x60
#define RHINE_RCR_PRO      0x10
#define RHINE_RCR_AB       0x08
#define RHINE_RCR_AM       0x04
#define RHINE_RCR_AR       0x02
#define RHINE_RCR_SEP      0x01

#define RHINE_TCR_RTSF     0x80
#define RHINE_TCR_TFT_64   0x00
#define RHINE_TCR_TFT_SF   0x60
#define RHINE_TCR_OFFSET   0x08
#define RHINE_TCR_LB1      0x04
#define RHINE_TCR_LB0      0x02

#define RHINE_CR0_RDMD       0x40
#define RHINE_CR0_TDMD       0x20
#define RHINE_CR0_TXON       0x10
#define RHINE_CR0_RXON       0x08
#define RHINE_CR0_STOP       0x04
#define RHINE_CR0_STRT       0x02
#define RHINE_CR0_INIT       0x01

#define RHINE_CR1_SRST       0x80
#define RHINE_CR1_DPOLL      0x08
#define RHINE_CR1_FDX        0x04
#define RHINE_CR1_ETEN       0x02
#define RHINE_CR1_EREN       0x01

#define RHINE_ISR_KEYI       0x8000
#define RHINE_ISR_SRCI       0x4000
#define RHINE_ISR_ABTI       0x2000
#define RHINE_ISR_NORBF      0x1000
#define RHINE_ISR_PKRACE     0x0800
#define RHINE_ISR_OVFI       0x0400
#define RHINE_ISR_ETI        0x0200
#define RHINE_ISR_ERI        0x0100
#define RHINE_ISR_CNT        0x0080
#define RHINE_ISR_BE         0x0040
#define RHINE_ISR_RU         0x0020
#define RHINE_ISR_TU         0x0010
#define RHINE_ISR_TXE        0x0008
#define RHINE_ISR_RXE        0x0004
#define RHINE_ISR_PTX        0x0002
#define RHINE_ISR_PRX        0x0001

#define RHINE_IMR_KEYI       0x8000
#define RHINE_IMR_SRCI       0x4000
#define RHINE_IMR_ABTI       0x2000
#define RHINE_IMR_NORBF      0x1000
#define RHINE_IMR_PKRACE     0x0800
#define RHINE_IMR_OVFI       0x0400
#define RHINE_IMR_ETI        0x0200
#define RHINE_IMR_ERI        0x0100
#define RHINE_IMR_CNT        0x0080
#define RHINE_IMR_BE         0x0040
#define RHINE_IMR_RU         0x0020
#define RHINE_IMR_TU         0x0010
#define RHINE_IMR_TXE        0x0008
#define RHINE_IMR_RXE        0x0004
#define RHINE_IMR_PTX        0x0002
#define RHINE_IMR_PRX        0x0001

#define RHINE_IMR_INIT (RHINE_IMR_PTX | RHINE_IMR_PRX | RHINE_IMR_RU)

#define RHINE_BCR0_MAGIC_INIT 0x00

#define RHINE_BCR1_POT2      0x04
#define RHINE_BCR1_POT1      0x02
#define RHINE_BCR1_MAGIC_INIT (RHINE_BCR1_POT1|RHINE_BCR1_POT2)


#define RHINE_MIICR_MAUTO    0x80
#define RHINE_MIICR_RCMD     0x40
#define RHINE_MIICR_WCMD     0x20
#define RHINE_MIICR_MDPM     0x10
#define RHINE_MIICR_MOUT     0x08
#define RHINE_MIICR_MDO      0x04
#define RHINE_MIICR_MDI      0x02
#define RHINE_MIICR_MDC      0x01

#define RHINE_MIISR_GPIO1POL 0x80
#define RHINE_MIISR_MFDC     0x20
#define RHINE_MIISR_PHYOPT   0x10
#define RHINE_MIISR_MIIERR   0x08
#define RHINE_MIISR_MRERR    0x04
#define RHINE_MIISR_LNKFL    0x02
#define RHINE_MIISR_SPEED    0x01

#define RHINE_EECSR_EEPR     0x80
#define RHINE_EECSR_EMBP     0x40
#define RHINE_EECSR_LOAD     0x20
#define RHINE_EECSR_DPM      0x10
#define RHINE_EECSR_ECS      0x08
#define RHINE_EECSR_ECK      0x04
#define RHINE_EECSR_EDI      0x02
#define RHINE_EECSR_EDO      0x01



#define RHINE_CFGA_EELOAD    0x80
#define RHINE_CFGA_JUMPER    0x40
#define RHINE_CFGA_MMIEN     0x20
#define RHINE_CFGA_MIIOPT    0x10
#define RHINE_CFGA_AUTOOPT   0x08
#define RHINE_CFGA_GPIO2I    0x04
#define RHINE_CFGA_GPIO2O    0x02
#define RHINE_CFGA_GPIO2OE   0x01

#define RHINE_CFGB_QPKTDIS   0x80
#define RHINE_CFGB_TRACEN    0x40
#define RHINE_CFGB_MRDM      0x20
#define RHINE_CFGB_TXARBIT   0x10
#define RHINE_CFGB_RXARBIT   0x08
#define RHINE_CFGB_MWWAIT    0x04
#define RHINE_CFGB_MRWAIT    0x02
#define RHINE_CFGB_LATMEN    0x01

#define RHINE_CFGC_BROPT     0x40
#define RHINE_CFGC_DLYEN     0x20
#define RHINE_CFGC_BTSEL     0x08
#define RHINE_CFGC_BPS2      0x04
#define RHINE_CFGC_BPS1      0x02
#define RHINE_CFGC_BPS0      0x01

#define RHINE_CFGD_GPIOEN    0x80
#define RHINE_CFGD_DIAG      0x40
#define RHINE_CFGD_MRDLEN    0x20
#define RHINE_CFGD_MAGIC     0x10
#define RHINE_CFGD_CRANDOM   0x08
#define RHINE_CFGD_CAP       0x04
#define RHINE_CFGD_MBA       0x02
#define RHINE_CFGD_BAKOPT    0x01




//----------------------------------------------------------------------------
// Receive buffer Descriptor
#define RHINE_RDES0              0x00   // frame length, status registers
#define RHINE_RDES1              0x04   // receive length
#define RHINE_RDES2              0x08   // rx data buffer
#define RHINE_RDES3              0x0c   // next
#define RHINE_RD_SIZE            0x10

#define RHINE_RDES0_OWN          0x80000000
#define RHINE_RDES0_FLNG_mask    0x07ff0000
#define RHINE_RDES0_FLNG_shift   16
#define RHINE_RDES0_RXOK         0x00008000
#define RHINE_RDES0_RES1         0x00004000
#define RHINE_RDES0_MAR          0x00002000
#define RHINE_RDES0_BAR          0x00001000
#define RHINE_RDES0_PHY          0x00000800
#define RHINE_RDES0_CHN          0x00000400
#define RHINE_RDES0_STP          0x00000200
#define RHINE_RDES0_EDP          0x00000100
#define RHINE_RDES0_BUFF         0x00000080
#define RHINE_RDES0_SERR         0x00000040
#define RHINE_RDES0_RUNT         0x00000020
#define RHINE_RDES0_LONG         0x00000010
#define RHINE_RDES0_FOV          0x00000008
#define RHINE_RDES0_FAE          0x00000004
#define RHINE_RDES0_CRC          0x00000002
#define RHINE_RDES0_RERR         0x00000001

#define RHINE_RD_RLEN_IC         0x00800000
#define RHINE_RD_RLEN_C          0x00008000
#define RHINE_RD_RLEN_RLEN_mask  0x000007ff


//----------------------------------------------------------------------------
// Transmit buffer Descriptor
#define RHINE_TDES0        0x00        // status & own
#define RHINE_TDES1        0x04        // tx config & length
#define RHINE_TDES2        0x08        // tx data buffer
#define RHINE_TDES3        0x0c        // next
#define RHINE_TD_SIZE      0x10

#define RHINE_TDES0_OWN        0x80000000
#define RHINE_TDES0_TXOK       0x00008000
#define RHINE_TDES0_JAB        0x00004000
#define RHINE_TDES0_SERR       0x00002000
#define RHINE_TDES0_RES1       0x00001000
#define RHINE_TDES0_RES2       0x00000800
#define RHINE_TDES0_CRS        0x00000400
#define RHINE_TDES0_OWC        0x00000200
#define RHINE_TDES0_ABT        0x00000100
#define RHINE_TDES0_CDH        0x00000080
#define RHINE_TDES0_NCR_mask   0x00000038
#define RHINE_TDES0_NCR_shift  3
#define RHINE_TDES0_RES3       0x00000004
#define RHINE_TDES0_UDF        0x00000002
#define RHINE_TDES0_DFR        0x00000001

#define RHINE_TDES1_TCR_mask   0x00ff0000
#define RHINE_TDES1_TCR_shift  16
#define RHINE_TDES1_IC         0x00800000
#define RHINE_TDES1_EDP        0x00400000
#define RHINE_TDES1_STP        0x00200000
#define RHINE_TDES1_CRC        0x00010000
#define RHINE_TDES1_C          0x00008000
#define RHINE_TDES1_TLNG_mask  0x000007ff

// ------------------------------------------------------------------------

#define MII_BMCR               0
#define MII_BMSR               1

#define MII_BMCR_RENEGOTIATE   0x3300

#define MII_BMSR_AN_COMPLETE   0x0020
#define MII_BMSR_LINK          0x0004

// ------------------------------------------------------------------------

#ifdef KEEP_STATISTICS
struct via_rhine_stats {
    unsigned int tx_good             ;
    unsigned int tx_max_collisions   ;
    unsigned int tx_late_collisions  ;
    unsigned int tx_underrun         ;
    unsigned int tx_carrier_loss     ;
    unsigned int tx_deferred         ;
    unsigned int tx_sqetesterrors    ;
    unsigned int tx_single_collisions;
    unsigned int tx_mult_collisions  ;
    unsigned int tx_total_collisions ;
    unsigned int rx_good             ;
    unsigned int rx_crc_errors       ;
    unsigned int rx_align_errors     ;
    unsigned int rx_resource_errors  ;
    unsigned int rx_overrun_errors   ;
    unsigned int rx_collisions       ;
    unsigned int rx_short_frames     ;
    unsigned int rx_too_long_frames  ;
    unsigned int rx_symbol_errors    ;
    unsigned int interrupts          ;
    unsigned int rx_count            ;
    unsigned int rx_deliver          ;
    unsigned int rx_resource         ;
    unsigned int rx_restart          ;
    unsigned int tx_count            ;
    unsigned int tx_complete         ;
    unsigned int tx_dropped          ;
};
#endif

typedef struct rhine_priv_data {
    int index;
    cyg_uint8                           // (split up for atomic byte access)
        found:1,                        // was hardware discovered?
        mac_addr_ok:1,                  // can we bring up?
        active:1,                       // has this if been brung up?
        hardwired_esa:1,                // set if ESA is hardwired via CDL
        spare1:4; 

    int txbusy;                         // A packet has been sent
    unsigned long txkey;                // Used to ack when packet sent
    unsigned char* base;                // Base address of controller IO region
    cyg_vector_t interrupt;             // Interrupt vector used by controller
    unsigned char esa[6];               // Controller ESA
    // Function to configure the ESA - may fetch ESA from EPROM or 
    // RedBoot config option.
    void (*config_esa)(struct rhine_priv_data* cpd);
    void *ndp;                          // Network Device Pointer

    cyg_handle_t  interrupt_handle;
    cyg_interrupt interrupt_object;

    int devid;

    int phys_id;                        // ID of physical MII controller

    cyg_uint8* rx_buffers;              // ptr to base of buffer mem
    cyg_uint8* rx_ring;                 // ptr to base of rx ring memory
    int rx_ring_cnt;                    // number of entries in ring
    int rx_ring_log_cnt;                // log of above
    int rx_ring_next;                   // index of next full ring entry

    cyg_uint8* tx_buffers;
    cyg_uint8* tx_ring;
    int tx_ring_cnt;
    int tx_ring_log_cnt;
    int tx_ring_free;                   // index of next free ring entry
    int tx_ring_alloc;                  // index of first controller owned ring
    int tx_ring_owned;                  // number of controller owned ring entries

    int rxpacket;
#ifdef KEEP_STATISTICS
    struct via_rhine_stats stats;
#endif
} rhine_priv_data;

// ------------------------------------------------------------------------
#endif CYGONCE_DEVS_ETH_VIA_RHINE_H
// EOF via_rhine.h
