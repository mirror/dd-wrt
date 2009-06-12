#ifndef CYGONCE_DEVS_ETH_AMD_LANCE_H
#define CYGONCE_DEVS_ETH_AMD_LANCE_H
//==========================================================================
//
//      amd_lance.h
//
//      AMD Lance Ethernet chip
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
// Author(s):    jskov, iz
// Contributors: jskov, hmt, iz
// Date:         2002-07-17
// Purpose:      Hardware description of AMD Lance series.
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_io.h>

//------------------------------------------------------------------------
// Get macros from platform header
#define __WANT_CONFIG
#include CYGDAT_DEVS_ETH_AMD_LANCEPCI_INL
#undef  __WANT_CONFIG

//------------------------------------------------------------------------
// Set to perms of:
// 0 disables all debug output
// 1 for process debug output
// 2 for added data IO output: get_reg, put_reg
// 4 for packet allocation/free output
// 8 for only startup status, so we can tell we're installed OK
#define DEBUG 0x0

#if DEBUG & 1
#define DEBUG_FUNCTION() do { os_printf("%s\n", __FUNCTION__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#endif

// ------------------------------------------------------------------------
// Macros for keeping track of statistics
#if defined(ETH_DRV_GET_IF_STATS) || defined (ETH_DRV_GET_IF_STATS_UD)
# define KEEP_STATISTICS
#endif

#ifdef KEEP_STATISTICS
# define INCR_STAT( _x_ )        (cpd->stats. _x_ ++)
#else
# define INCR_STAT( _x_ )        CYG_EMPTY_STATEMENT
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

// ------------------------------------------------------------------------
// Macros for accessing structure elements

#define _SU8( _base_, _offset_) \
        *((cyg_uint8 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SU16( _base_, _offset_) \
        *((cyg_uint16 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SU32( _base_, _offset_) \
        *((cyg_uint32 *)((CYG_ADDRWORD)_base_+(_offset_)))

#define _SI8( _base_, _offset_) \
        *((cyg_int8 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SI16( _base_, _offset_) \
        *((cyg_int16 *)((CYG_ADDRWORD)_base_+(_offset_)))
#define _SI32( _base_, _offset_) \
        *((cyg_int32 *)((CYG_ADDRWORD)_base_+(_offset_)))

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
// IO map registers
#define LANCE_IO_EEPROM   0x00
#define LANCE_IO_ID       0x0e
#define LANCE_IO_RDP      0x10
#define LANCE_IO_RAP      0x12
#define LANCE_IO_RESET    0x14
#define LANCE_IO_BDP      0x16

// The ID of the 79C790 is 0x5757 - that may be different in other
// (older) cards.
#define LANCE_IO_ID_KEY   0x5757

// ------------------------------------------------------------------------
// Controller registers come in three sets: CSR, BCR and ANR. Use
// high-bits do differentiate, make the put/get functions do the right
// thing depending the state of these bits.
#define LANCE_RAP_MASK    0x007f
//#define LANCE_CSR_FLAG  0x0000        // implied
#define LANCE_BCR_FLAG    0x0080
#define LANCE_ANR_FLAG    0x0100


// CSR registers
#define LANCE_CSR_CSCR    0
#define LANCE_CSR_IBA0    1
#define LANCE_CSR_IBA1    2
#define LANCE_CSR_IM      3
#define LANCE_CSR_TFC     4
#define LANCE_CSR_ECI     5
#define LANCE_CSR_LAR0    8
#define LANCE_CSR_LAR1    9
#define LANCE_CSR_LAR2    10
#define LANCE_CSR_LAR3    11
#define LANCE_CSR_PAR0    12
#define LANCE_CSR_PAR1    13
#define LANCE_CSR_PAR2    14
#define LANCE_CSR_MODE    15
#define LANCE_CSR_BARRL   24
#define LANCE_CSR_BARRU   25
#define LANCE_CSR_BATRL   30
#define LANCE_CSR_BATRU   31
#define LANCE_CSR_RRC     72
#define LANCE_CSR_TRC     74
#define LANCE_CSR_RRLEN   76
#define LANCE_CSR_TRLEN   78
#define LANCE_CSR_ID_LO   88
#define LANCE_CSR_ID_HI   89


#define LANCE_CSR_CSCR_ERR       0x8000
#define LANCE_CSR_CSCR_RES       0x4000
#define LANCE_CSR_CSCR_CERR      0x2000
#define LANCE_CSR_CSCR_MISS      0x1000
#define LANCE_CSR_CSCR_MERR      0x0800
#define LANCE_CSR_CSCR_RINT      0x0400
#define LANCE_CSR_CSCR_TINT      0x0200
#define LANCE_CSR_CSCR_IDON      0x0100
#define LANCE_CSR_CSCR_INTR      0x0080
#define LANCE_CSR_CSCR_IENA      0x0040
#define LANCE_CSR_CSCR_RXON      0x0020
#define LANCE_CSR_CSCR_TXON      0x0010
#define LANCE_CSR_CSCR_TDMD      0x0008
#define LANCE_CSR_CSCR_STOP      0x0004
#define LANCE_CSR_CSCR_STRT      0x0002
#define LANCE_CSR_CSCR_INIT      0x0001

#define LANCE_CSR_CSCR_EV_MASK   0x007f

#define LANCE_CSR_IM_MISSM       0x1000
#define LANCE_CSR_IM_MERRM       0x0800
#define LANCE_CSR_IM_RINTM       0x0400
#define LANCE_CSR_IM_TINTM       0x0200
#define LANCE_CSR_IM_IDONM       0x0100
#define LANCE_CSR_IM_DXSUFLO     0x0040
#define LANCE_CSR_IM_LAPPEN      0x0020
#define LANCE_CSR_IM_DXMT2PD     0x0010
#define LANCE_CSR_IM_EMBA        0x0008
#define LANCE_CSR_IM_BSWP        0x0004

#define LANCE_CSR_TFC_TXDPOLL    0x1000
#define LANCE_CSR_TFC_APAD_XMT   0x0800
#define LANCE_CSR_TFC_ASTRP_RCV  0x0400
#define LANCE_CSR_TFC_MFCO       0x0200
#define LANCE_CSR_TFC_MFCOM      0x0100
#define LANCE_CSR_TFC_UINTCMD    0x0080
#define LANCE_CSR_TFC_UINT       0x0040
#define LANCE_CSR_TFC_RCVCCO     0x0020
#define LANCE_CSR_TFC_RCVCCOM    0x0010
#define LANCE_CSR_TFC_TXSTRT     0x0008
#define LANCE_CSR_TFC_TXSTRTM    0x0004

#define LANCE_CSR_ECI_TOKINTD      0x8000
#define LANCE_CSR_ECI_LTINTEN      0x4000
#define LANCE_CSR_ECI_SINT         0x0800
#define LANCE_CSR_ECI_SINTE        0x0400
#define LANCE_CSR_ECI_EXDINT       0x0080
#define LANCE_CSR_ECI_EXDINTE      0x0040
#define LANCE_CSR_ECI_MPPLBA       0x0020
#define LANCE_CSR_ECI_MPINT        0x0010
#define LANCE_CSR_ECI_MPINTE       0x0008
#define LANCE_CSR_ECI_MPEN         0x0004
#define LANCE_CSR_ECI_MPMODE       0x0002
#define LANCE_CSR_ECI_SPND         0x0001

#define LANCE_CSR_MODE_PROM        0x8000
#define LANCE_CSR_MODE_DRCVBC      0x4000
#define LANCE_CSR_MODE_DRCVPA      0x2000
#define LANCE_CSR_MODE_PORTSEL     0x0180
#define LANCE_CSR_MODE_INTL        0x0040
#define LANCE_CSR_MODE_DRTY        0x0020
#define LANCE_CSR_MODE_FCOLL       0x0010
#define LANCE_CSR_MODE_DXMTFCS     0x0008
#define LANCE_CSR_MODE_LOOP        0x0004
#define LANCE_CSR_MODE_DTX         0x0002
#define LANCE_CSR_MODE_DRX         0x0001

// BCR registers
#define LANCE_BCR_SWSTYLE (20 |LANCE_BCR_FLAG)
#define LANCE_BCR_MIIADDR (33 |LANCE_BCR_FLAG)
#define LANCE_BCR_MIIDATA (34 |LANCE_BCR_FLAG)

#define LANCE_BCR_MIIADDR_PHYAD    0x03e0


//----------------------------------------------------------------------------
// Receive buffer Descriptor
#if 1
#define LANCE_RD_PTR       0x00        // 32 bit
#define LANCE_RD_BLEN      0x04        // 16 bit (2's complement, negative)
#define LANCE_RD_MLEN      0x06        // 16 bit
#define LANCE_RD_SIZE      0x08

#define LANCE_RD_PTR_OWN       0x80000000
#define LANCE_RD_PTR_ERR       0x40000000
#define LANCE_RD_PTR_FRAM      0x20000000
#define LANCE_RD_PTR_OFLO      0x10000000
#define LANCE_RD_PTR_CRC       0x08000000
#define LANCE_RD_PTR_BUFF      0x04000000
#define LANCE_RD_PTR_STP       0x02000000
#define LANCE_RD_PTR_ENP       0x01000000
#define LANCE_RD_PTR_MASK      0x00ffffff
#else

#define LANCE_RD_PTR       0x00
#define LANCE_RD_BLEN      0x04
#define LANCE_RD_MLEN      0x08
#define LANCE_RD_USER      0x0c
#define LANCE_RD_SIZE      0x10

#define LANCE_RD_BLEN_OWN       0x80000000
#define LANCE_RD_BLEN_ERR       0x40000000
#define LANCE_RD_BLEN_FRAM      0x20000000
#define LANCE_RD_BLEN_OFLO      0x10000000
#define LANCE_RD_BLEN_CRC       0x08000000
#define LANCE_RD_BLEN_BUFF      0x04000000
#define LANCE_RD_BLEN_STP       0x02000000
#define LANCE_RD_BLEN_ENP       0x01000000
#define LANCE_RD_BLEN_BPE       0x00800000
#define LANCE_RD_BLEN_PAM       0x00400000
#define LANCE_RD_BLEN_LAFM      0x00200000
#define LANCE_RD_BLEN_BAM       0x00100000
#define LANCE_RD_BLEN_MASK      0x0000ffff
#endif

// Transmit buffer Descriptor
#if 1
#define LANCE_TD_PTR       0x00        // 32 bit
#define LANCE_TD_LEN       0x04        // 16 bit (2's complement, negative)
#define LANCE_TD_MISC      0x06        // 16 bit
#define LANCE_TD_SIZE      0x08

#define LANCE_TD_PTR_OWN       0x80000000
#define LANCE_TD_PTR_ERR       0x40000000
#define LANCE_TD_PTR_ADD_FCS   0x20000000
#define LANCE_TD_PTR_MORE      0x10000000
#define LANCE_TD_PTR_ONE       0x08000000
#define LANCE_TD_PTR_DEF       0x04000000
#define LANCE_TD_PTR_STP       0x02000000
#define LANCE_TD_PTR_ENP       0x01000000
#define LANCE_TD_PTR_MASK      0x00ffffff
#else
#define LANCE_TD_PTR       0x00
#define LANCE_TD_LEN       0x04
#define LANCE_TD_MISC      0x08
#define LANCE_TD_USER      0x0c
#define LANCE_TD_SIZE      0x10

#define LANCE_TD_LEN_OWN       0x80000000
#define LANCE_TD_LEN_ERR       0x40000000
#define LANCE_TD_LEN_ADD_FCS   0x20000000
#define LANCE_TD_LEN_MORE      0x10000000
#define LANCE_TD_LEN_ONE       0x08000000
#define LANCE_TD_LEN_DEF       0x04000000
#define LANCE_TD_LEN_STP       0x02000000
#define LANCE_TD_LEN_ENP       0x01000000
#define LANCE_TD_LEN_BPE       0x00800000
#define LANCE_TD_LEN_MASK      0x0000ffff

#define LANCE_TD_FLAGS_BUFF     0x80000000
#define LANCE_TD_FLAGS_UFLO     0x40000000
#define LANCE_TD_FLAGS_EX_DEF   0x20000000
#define LANCE_TD_FLAGS_LCOL     0x10000000
#define LANCE_TD_FLAGS_LCAR     0x08000000
#define LANCE_TD_FLAGS_RTRY     0x04000000
#define LANCE_TD_FLAGS_TRC_MASK 0x0000000f
#endif


#define LANCE_TD_MISC_BUFF     0x8000
#define LANCE_TD_MISC_UFLO     0x4000
#define LANCE_TD_MISC_EXDEF    0x2000
#define LANCE_TD_MISC_LCOL     0x1000
#define LANCE_TD_MISC_LCAR     0x0800
#define LANCE_TD_MISC_RTRY     0x0400
#define LANCE_TD_MISC_TDR      0x03ff

// Initialization Buffer
#define LANCE_IB_MODE            0
#define LANCE_IB_PADR0           2
#define LANCE_IB_PADR1           4
#define LANCE_IB_PADR2           6
#define LANCE_IB_LADRF0          8
#define LANCE_IB_LADRF1          10
#define LANCE_IB_LADRF2          12
#define LANCE_IB_LADRF3          14
#define LANCE_IB_RDRA            16
#define LANCE_IB_TDRA            20
#define LANCE_IB_SIZE            24

#define LANCE_IB_TDRA_CNT_shift  29
#define LANCE_IB_TDRA_PTR_mask   0x00ffffff
#define LANCE_IB_RDRA_CNT_shift  29
#define LANCE_IB_RDRA_PTR_mask   0x00ffffff

// ------------------------------------------------------------------------

#ifdef KEEP_STATISTICS
struct amd_lancepci_stats {
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

typedef struct lancepci_priv_data {
    int index;
    cyg_uint8                           // (split up for atomic byte access)
        found:1,                        // was hardware discovered?
        mac_addr_ok:1,                  // can we bring up?
        active:1,                       // has this if been brung up?
        hardwired_esa:1,                // set if ESA is hardwired via CDL
        txbusy:1,                       // A packet has been sent
        txbusyh:1,                      // A packet has been sent for HW
        spare1:2; 

    cyg_uint16 event;
    
    unsigned long txkey;                // Used to ack when packet sent
    unsigned char* base;                // Base address of controller EPROM region
    int interrupt;                      // Interrupt vector used by controller
    unsigned char esa[6];            // Controller ESA
    // Function to configure the ESA - may fetch ESA from EPROM or 
    // RedBoot config option.
    void (*config_esa)(struct lancepci_priv_data* cpd);
    void *ndp;                          // Network Device Pointer

    cyg_handle_t  interrupt_handle;
    cyg_interrupt interrupt_object;
    int devid;

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
    struct amd_lancepci_stats stats;
#endif
#if DEBUG & 1
    cyg_uint32 txd;
#endif
    cyg_uint8* init_table;				// lance init table pointer

} lancepci_priv_data;

// ------------------------------------------------------------------------

static __inline__ cyg_uint16
get_reg(struct eth_drv_sc *sc, int regno)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
    cyg_uint16 val, addr;

    if (regno & LANCE_ANR_FLAG) {
        // We could do this with recursive calls to get/put reg
        // functions, but might as well just do it directly.
        // First set ANR address
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_RAP, LANCE_BCR_MIIADDR & LANCE_RAP_MASK);
        HAL_PCI_IO_READ_UINT16(cpd->base+LANCE_IO_BDP, addr);
        addr &= LANCE_BCR_MIIADDR_PHYAD;
        addr |= (regno & LANCE_RAP_MASK);
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_BDP, addr);
        // Then read ANR register data
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_RAP, LANCE_BCR_MIIDATA & LANCE_RAP_MASK);
        HAL_PCI_IO_READ_UINT16(cpd->base+LANCE_IO_BDP, val);
    } else {
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_RAP, regno & LANCE_RAP_MASK);
        if (regno & LANCE_BCR_FLAG)
            HAL_PCI_IO_READ_UINT16(cpd->base+LANCE_IO_BDP, val);
        else
            HAL_PCI_IO_READ_UINT16(cpd->base+LANCE_IO_RDP, val);
    }
#if DEBUG & 2
    os_printf("read %s reg %d val 0x%04x\n", 
                (regno & LANCE_ANR_FLAG) ? "anr" : (regno & LANCE_BCR_FLAG) ? "bcr" : "csr", 
                regno & LANCE_RAP_MASK, val);
#endif
    return val;
}

static __inline__ void
put_reg(struct eth_drv_sc *sc, int regno, cyg_uint16 val)
{
    struct lancepci_priv_data *cpd =
        (struct lancepci_priv_data *)sc->driver_private;
    cyg_uint16 addr;

    if (regno & LANCE_ANR_FLAG) {
        // We could do this with recursive calls to get/put reg
        // functions, but might as well just do it directly.
        // First set ANR address
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_RAP, LANCE_BCR_MIIADDR & LANCE_RAP_MASK);
        HAL_PCI_IO_READ_UINT16(cpd->base+LANCE_IO_BDP, addr);
        addr &= LANCE_BCR_MIIADDR_PHYAD;
        addr |= (regno & LANCE_RAP_MASK);
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_BDP, addr);
        // Then write ANR register data
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_RAP, LANCE_BCR_MIIDATA & LANCE_RAP_MASK);
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_BDP, val);
    } else {
        HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_RAP, regno & LANCE_RAP_MASK);
        if (regno & LANCE_BCR_FLAG)
            HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_BDP, val);
        else
            HAL_PCI_IO_WRITE_UINT16(cpd->base+LANCE_IO_RDP, val);
    }

#if DEBUG & 2
    os_printf("write %s reg %d val 0x%04x\n", 
                (regno & LANCE_ANR_FLAG) ? "anr" : (regno & LANCE_BCR_FLAG) ? "bcr" : "csr", 
                regno & LANCE_RAP_MASK, val);
#endif
}

// ------------------------------------------------------------------------
#endif // CYGONCE_DEVS_ETH_AMD_LANCE_H
// EOF amd_lance.h
