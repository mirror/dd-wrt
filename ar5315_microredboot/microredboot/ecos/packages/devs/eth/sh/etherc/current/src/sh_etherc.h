#ifndef CYGONCE_DEVS_ETH_SH_ETHERC_H
#define CYGONCE_DEVS_ETH_SH_ETHERC_H
//==========================================================================
//
//      sh_etherc.h
//
//      SH EtherC Ethernet CPU module controller
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
// Purpose:      Hardware description of SH EtherC controller.
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_io.h>

//------------------------------------------------------------------------
// Get macros from platform header
#define __WANT_CONFIG
#include CYGDAT_DEVS_ETH_SH_ETHERC_INL
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
#define DEBUG_FUNCTION() do { db_printf("%s\n", __FUNCTION__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#endif

// ------------------------------------------------------------------------
// Debug print function
#if defined(CYGPKG_REDBOOT) && DEBUG

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
// Controller registers in offset form
#define _REG_EDMR                 0x00
#define _REG_EDTRR                0x04
#define _REG_EDRRR                0x08
#define _REG_TDLAR                0x0c
#define _REG_RDLAR                0x10
#define _REG_EESR                 0x14
#define _REG_EESIPR               0x18
#define _REG_TRSCER               0x1c
#define _REG_RMFCR                0x20
#define _REG_TFTR                 0x24
#define _REG_FDR                  0x28
#define _REG_RCR                  0x2c
#define _REG_EDOCR                0x30
#define _REG_RBWAR                0x40
#define _REG_RDFAR                0x44
#define _REG_TBRAR                0x4c
#define _REG_TDFAR                0x50

#define _REG_ECMR                 0x60
#define _REG_ECSR                 0x64
#define _REG_ECSIPR               0x68
#define _REG_PIR                  0x6c
#define _REG_MAHR                 0x70
#define _REG_MALR                 0x74
#define _REG_RFLR                 0x78
#define _REG_PSR                  0x7c
#define _REG_TROCR                0x80
#define _REG_CDCR                 0x84
#define _REG_LCCR                 0x88
#define _REG_CNDCR                0x8c
#define _REG_IFLCR                0x90
#define _REG_CECFR                0x94
#define _REG_FRECR                0x98
#define _REG_TSFRCR               0x9c
#define _REG_TLFRCR               0xa0
#define _REG_RFCR                 0xa4
#define _REG_MAFCR                0xa8

//----------------------------------------------------------------------------
// Receive buffer Descriptor
#define ETHERC_RD_STAT      0x00        // 32 bit
#define ETHERC_RD_RBL       0x04        // 16 bit - receive buffer length
#define ETHERC_RD_RDL       0x06        // 16 bit - receive data length (-CRC)
#define ETHERC_RD_RBA       0x08        // 32 bit - receive buffer address
#define ETHERC_RD_PAD       0x0c        // 32 bit
#define ETHERC_RD_SIZE      0x10

#define ETHERC_RD_STAT_RACT        0x80000000
#define ETHERC_RD_STAT_RDLE        0x40000000
#define ETHERC_RD_STAT_RFP_OTO     0x30000000 // one buffer to one frame
#define ETHERC_RD_STAT_RFE         0x08000000
#define ETHERC_RD_STAT_RFOF        0x00000200
#define ETHERC_RD_STAT_RMAF        0x00000080
#define ETHERC_RD_STAT_RRF         0x00000010
#define ETHERC_RD_STAT_RTLF        0x00000008
#define ETHERC_RD_STAT_RTSF        0x00000004
#define ETHERC_RD_STAT_PRE         0x00000002
#define ETHERC_RD_STAT_CERF        0x00000001

#define ETHERC_RD_STAT_CLEAR       0x70000000

// Transmit buffer Descriptor
#define ETHERC_TD_STAT      0x00        // 32 bit
#define ETHERC_TD_TDL       0x04        // 16 bit - transmit data length
#define ETHERC_TD_PAD0      0x06        // 16 bit
#define ETHERC_TD_TBA       0x08        // 32 bit - transmit buffer address
#define ETHERC_TD_PAD1      0x0c        // 32 bit
#define ETHERC_TD_SIZE      0x10

#define ETHERC_TD_STAT_TACT        0x80000000
#define ETHERC_TD_STAT_TDLE        0x40000000
#define ETHERC_TD_STAT_TFP_OTO     0x30000000 // one buffer to one frame
#define ETHERC_TD_STAT_TDFE        0x08000000
#define ETHERC_TD_STAT_ITF         0x00000010
#define ETHERC_TD_STAT_CND         0x00000008
#define ETHERC_TD_STAT_DLC         0x00000004
#define ETHERC_TD_STAT_CD          0x00000002
#define ETHERC_TD_STAT_TRO         0x00000001


// Initialization Buffer
#define ETHERC_IB_MODE            0
#define ETHERC_IB_PADR0           2
#define ETHERC_IB_PADR1           4
#define ETHERC_IB_PADR2           6
#define ETHERC_IB_LADRF0          8
#define ETHERC_IB_LADRF1          10
#define ETHERC_IB_LADRF2          12
#define ETHERC_IB_LADRF3          14
#define ETHERC_IB_RDRA            16
#define ETHERC_IB_TDRA            20
#define ETHERC_IB_SIZE            24

#define ETHERC_IB_TDRA_CNT_shift  29
#define ETHERC_IB_TDRA_PTR_mask   0x00ffffff
#define ETHERC_IB_RDRA_CNT_shift  29
#define ETHERC_IB_RDRA_PTR_mask   0x00ffffff

// ------------------------------------------------------------------------

#ifdef KEEP_STATISTICS
struct sh_etherc_stats {
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

struct etherc_priv_data;
typedef cyg_bool (*provide_esa_t)(struct etherc_priv_data* cpd);

typedef struct etherc_priv_data {
    int index;
    cyg_uint8                           // (split up for atomic byte access)
        mac_addr_ok:1,                  // can we bring up?
        active:1,                       // has this if been brung up?
        hardwired_esa:1,                // set if ESA is hardwired via CDL
        txbusy:1,                       // A packet has been sent
        spare1:3; 

    unsigned long txkey;                // Used to ack when packet sent
    unsigned char* base;                // Base address of controller EPROM region
    int interrupt;                      // Interrupt vector used by controller
    unsigned char esa[6];            // Controller ESA
    // Function to configure the ESA - may fetch ESA from EPROM or 
    // RedBoot config option.
    void (*config_esa)(struct etherc_priv_data* cpd);
    void *ndp;                          // Network Device Pointer
    provide_esa_t provide_esa;

    cyg_handle_t  interrupt_handle;
    cyg_interrupt interrupt_object;
    int devid;

    cyg_uint8* rx_buffers;              // ptr to base of buffer mem
    cyg_uint8* rx_ring;                 // ptr to base of rx ring memory
    int rx_ring_cnt;                    // number of entries in ring
    int rx_ring_next;                   // index of next full ring entry

    cyg_uint8* tx_buffers;
    cyg_uint8* tx_ring;
    int tx_ring_cnt;
    int tx_ring_free;                   // index of next free ring entry
    int tx_ring_alloc;                  // index of first controller owned ring
    int tx_ring_owned;                  // number of controller owned ring entries

    int rxpacket;
#ifdef KEEP_STATISTICS
    struct sh_etherc_stats stats;
#endif
#if DEBUG & 1
    cyg_uint32 txd;
#endif
} etherc_priv_data;

// ------------------------------------------------------------------------

static __inline__ cyg_uint32
get_reg(struct etherc_priv_data *cpd, int regno)
{
    cyg_int32 val;

    HAL_READ_UINT32(cpd->base+regno, val);

#if DEBUG & 2
    db_printf("read reg %d val 0x%08x\n", regno, val);
#endif
    return val;
}

static __inline__ void
put_reg(struct etherc_priv_data *cpd, int regno, cyg_uint32 val)
{
    HAL_WRITE_UINT32(cpd->base+regno, val);

#if DEBUG & 2
    db_printf("write reg %d val 0x%08x\n", regno, val);
#endif
}

// ------------------------------------------------------------------------
#endif // CYGONCE_DEVS_ETH_SH_ETHERC_H
// EOF sh_etherc.h
