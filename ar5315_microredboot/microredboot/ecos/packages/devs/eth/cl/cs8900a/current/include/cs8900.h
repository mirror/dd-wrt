#ifndef _CYGONCE_ETH_CL_CS8900_H_
#define _CYGONCE_ETH_CL_CS8900_H_
//==========================================================================
//
//      dev/cs8900.h
//
//      Cirrus Logic CS8900 Ethernet chip
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
// Date:         2001-11-07
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_io.h>
#include <pkgconf/devs_eth_cl_cs8900a.h>

#define __WANT_CONFIG
#include CYGDAT_DEVS_ETH_CL_CS8900A_INL
#undef __WANT_CONFIG

// ------------------------------------------------------------------------
// Debugging details

// Set to perms of:
// 0 disables all debug output
// 1 for process debug output
// 2 for added data IO output: get_reg, put_reg
// 4 for packet allocation/free output
// 8 for only startup status, so we can tell we're installed OK
#define DEBUG 0x0

#if DEBUG & 1
#define DEBUG_FUNCTION() do { diag_printf("%s\n", __FUNCTION__); } while (0)
#define DEBUG_LINE() do { diag_printf("%d\n", __LINE__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#define DEBUG_LINE() do {} while(0)
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

// ------------------------------------------------------------------------
// Private driver structure
struct cs8900a_priv_data;
typedef cyg_bool (*provide_esa_t)(struct cs8900a_priv_data* cpd);

typedef struct cs8900a_priv_data {
    bool txbusy, hardwired_esa;
    int rxmode;
    cyg_uint8 esa[6];
    provide_esa_t provide_esa;
    cyg_vector_t interrupt;             // Interrupt vector used by controller
    cyg_handle_t  interrupt_handle;
    cyg_interrupt interrupt_object;
    cyg_addrword_t base;
    cyg_uint32 txkey;   // Used to ack when packet sent
    struct cyg_netdevtab_entry *tab;
#ifdef CYGPKG_KERNEL
    cyg_tick_count_t txstart;
#endif
} cs8900a_priv_data_t;

// ------------------------------------------------------------------------
// Macros for accessing CS registers
// These can be overridden by the platform header

#ifndef CS_IN
# define CS_IN(_b_, _o_, _d_)  HAL_READ_UINT16 ((cyg_addrword_t)(_b_)+(_o_), (_d_))
# define CS_OUT(_b_, _o_, _d_) HAL_WRITE_UINT16((cyg_addrword_t)(_b_)+(_o_), (_d_))
#endif

// ------------------------------------------------------------------------
// Macros allowing platform to customize some of the driver details

#ifndef CYGHWR_CL_CS8900A_PLF_RESET
# define CYGHWR_CL_CS8900A_PLF_RESET(_b_) do { } while (0)
#endif

#ifndef CYGHWR_CL_CS8900A_PLF_POST_RESET
# define CYGHWR_CL_CS8900A_PLF_POST_RESET(_b_) do { } while (0)
#endif

#ifndef CYGHWR_CL_CS8900A_PLF_INT_CLEAR
# define CYGHWR_CL_CS8900A_PLF_INT_CLEAR(_cdp_)
#endif

#ifndef CYGHWR_CL_CS8900A_PLF_INIT
#define CYGHWR_CL_CS8900A_PLF_INIT(_cdp_) do { } while (0)
#endif


// ------------------------------------------------------------------------
// Directly visible registers. 
// Platform can override stepping or layout if necessary.
#ifndef CS8900A_step
# define CS8900A_step 2
#endif
#ifndef CS8900A_RTDATA
# define CS8900A_RTDATA (0*CS8900A_step)
# define CS8900A_TxCMD  (2*CS8900A_step)
# define CS8900A_TxLEN  (3*CS8900A_step)
# define CS8900A_ISQ    (4*CS8900A_step)
# define CS8900A_PPTR   (5*CS8900A_step)
# define CS8900A_PDATA  (6*CS8900A_step)
#endif

#ifndef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED
#define ISQ_RxEvent     0x0004
#define ISQ_TxEvent     0x0008
#define ISQ_BufEvent    0x000C
#define ISQ_RxMissEvent 0x0010
#define ISQ_TxColEvent  0x0012
#define ISQ_EventMask   0x003F
#else
#define ISQ_RxEvent     0x0400
#define ISQ_TxEvent     0x0800
#define ISQ_BufEvent    0x0C00
#define ISQ_RxMissEvent 0x1000
#define ISQ_TxColEvent  0x1200
#define ISQ_EventMask   0x3F00
#endif

// ------------------------------------------------------------------------
// Registers available via "page pointer" (indirect access)
#ifndef CYGIMP_DEVS_ETH_CL_CS8900A_DATABUS_BYTE_SWAPPED

#define PP_ChipID    0x0000  // Chip identifier - must be 0x630e
#define PP_ChipRev   0x0002  // Chip revision, model codes
#define PP_ChipID_CL           0x630e

#define PP_IntReg    0x0022  // Interrupt configuration
#define PP_IntReg_IRQ0         0x0000  // Use INTR0 pin
#define PP_IntReg_IRQ1         0x0001  // Use INTR1 pin
#define PP_IntReg_IRQ2         0x0002  // Use INTR2 pin
#define PP_IntReg_IRQ3         0x0003  // Use INTR3 pin

#define PP_RxCFG     0x0102  // Receiver configuration
#define PP_RxCFG_Skip1         0x0040  // Skip (i.e. discard) current frame
#define PP_RxCFG_Stream        0x0080  // Enable streaming mode
#define PP_RxCFG_RxOK          0x0100  // RxOK interrupt enable
#define PP_RxCFG_RxDMAonly     0x0200  // Use RxDMA for all frames
#define PP_RxCFG_AutoRxDMA     0x0400  // Select RxDMA automatically
#define PP_RxCFG_BufferCRC     0x0800  // Include CRC characters in frame
#define PP_RxCFG_CRC           0x1000  // Enable interrupt on CRC error
#define PP_RxCFG_RUNT          0x2000  // Enable interrupt on RUNT frames
#define PP_RxCFG_EXTRA         0x4000  // Enable interrupt on frames with extra data

#define PP_RxCTL     0x0104  // Receiver control
#define PP_RxCTL_IAHash        0x0040  // Accept frames that match hash
#define PP_RxCTL_Promiscuous   0x0080  // Accept any frame
#define PP_RxCTL_RxOK          0x0100  // Accept well formed frames
#define PP_RxCTL_Multicast     0x0200  // Accept multicast frames
#define PP_RxCTL_IA            0x0400  // Accept frame that matches IA
#define PP_RxCTL_Broadcast     0x0800  // Accept broadcast frames
#define PP_RxCTL_CRC           0x1000  // Accept frames with bad CRC
#define PP_RxCTL_RUNT          0x2000  // Accept runt frames
#define PP_RxCTL_EXTRA         0x4000  // Accept frames that are too long

#define PP_TxCFG     0x0106  // Transmit configuration
#define PP_TxCFG_CRS           0x0040  // Enable interrupt on loss of carrier
#define PP_TxCFG_SQE           0x0080  // Enable interrupt on Signal Quality Error
#define PP_TxCFG_TxOK          0x0100  // Enable interrupt on successful xmits
#define PP_TxCFG_Late          0x0200  // Enable interrupt on "out of window" 
#define PP_TxCFG_Jabber        0x0400  // Enable interrupt on jabber detect
#define PP_TxCFG_Collision     0x0800  // Enable interrupt if collision
#define PP_TxCFG_16Collisions  0x8000  // Enable interrupt if > 16 collisions

#define PP_TxCmd     0x0108  // Transmit command status
#define PP_TxCmd_TxStart_5     0x0000  // Start after 5 bytes in buffer
#define PP_TxCmd_TxStart_381   0x0040  // Start after 381 bytes in buffer
#define PP_TxCmd_TxStart_1021  0x0080  // Start after 1021 bytes in buffer
#define PP_TxCmd_TxStart_Full  0x00C0  // Start after all bytes loaded
#define PP_TxCmd_Force         0x0100  // Discard any pending packets
#define PP_TxCmd_OneCollision  0x0200  // Abort after a single collision
#define PP_TxCmd_NoCRC         0x1000  // Do not add CRC
#define PP_TxCmd_NoPad         0x2000  // Do not pad short packets

#define PP_BufCFG    0x010A  // Buffer configuration
#define PP_BufCFG_SWI          0x0040  // Force interrupt via software
#define PP_BufCFG_RxDMA        0x0080  // Enable interrupt on Rx DMA
#define PP_BufCFG_TxRDY        0x0100  // Enable interrupt when ready for Tx
#define PP_BufCFG_TxUE         0x0200  // Enable interrupt in Tx underrun
#define PP_BufCFG_RxMiss       0x0400  // Enable interrupt on missed Rx packets
#define PP_BufCFG_Rx128        0x0800  // Enable Rx interrupt after 128 bytes
#define PP_BufCFG_TxCol        0x1000  // Enable int on Tx collision ctr overflow
#define PP_BufCFG_Miss         0x2000  // Enable int on Rx miss ctr overflow
#define PP_BufCFG_RxDest       0x8000  // Enable int on Rx dest addr match

#define PP_LineCTL   0x0112  // Line control
#define PP_LineCTL_Rx          0x0040  // Enable receiver
#define PP_LineCTL_Tx          0x0080  // Enable transmitter

#define PP_RER       0x0124  // Receive event
#define PP_RER_IAHash          0x0040  // Frame hash match
#define PP_RER_Dribble         0x0080  // Frame had 1-7 extra bits after last byte
#define PP_RER_RxOK            0x0100  // Frame received with no errors
#define PP_RER_Hashed          0x0200  // Frame address hashed OK
#define PP_RER_IA              0x0400  // Frame address matched IA
#define PP_RER_Broadcast       0x0800  // Broadcast frame
#define PP_RER_CRC             0x1000  // Frame had CRC error
#define PP_RER_RUNT            0x2000  // Runt frame
#define PP_RER_EXTRA           0x4000  // Frame was too long

#define PP_TER       0x0128 // Transmit event
#define PP_TER_CRS             0x0040  // Carrier lost
#define PP_TER_SQE             0x0080  // Signal Quality Error
#define PP_TER_TxOK            0x0100  // Packet sent without error
#define PP_TER_Late            0x0200  // Out of window
#define PP_TER_Jabber          0x0400  // Stuck transmit?
#define PP_TER_NumCollisions   0x7800  // Number of collisions
#define PP_TER_16Collisions    0x8000  // > 16 collisions

#define PP_SelfCtl   0x0114  // Chip control
#define PP_SelfCtl_Reset       0x0040  // Self-clearing reset

#define PP_BusCtl    0x0116  // Bus control
#define PP_BusCtl_ResetRxDMA   0x0040  // Reset receiver DMA engine
#define PP_BusCtl_DMAextend    0x0100
#define PP_BusCtl_UseSA        0x0200
#define PP_BusCtl_MemoryE      0x0400  // Enable "memory mode"
#define PP_BusCtl_DMAburst     0x0800
#define PP_BusCtl_IOCH_RDYE    0x1000
#define PP_BusCtl_RxDMAsize    0x2000
#define PP_BusCtl_EnableIRQ    0x8000  // Enable interrupts

#define PP_LineStat  0x0134  // Line status
#define PP_LineStat_LinkOK     0x0080  // Line is connected and working
#define PP_LineStat_AUI        0x0100  // Connected via AUI
#define PP_LineStat_10BT       0x0200  // Connected via twisted pair
#define PP_LineStat_Polarity   0x1000  // Line polarity OK (10BT only)
#define PP_LineStat_CRS        0x4000  // Frame being received

#define PP_SelfStat  0x0136  // Chip status
#define PP_SelfStat_InitD      0x0080  // Chip initialization complete
#define PP_SelfStat_SIBSY      0x0100  // EEPROM is busy
#define PP_SelfStat_EEPROM     0x0200  // EEPROM present
#define PP_SelfStat_EEPROM_OK  0x0400  // EEPROM checks out
#define PP_SelfStat_ELPresent  0x0800  // External address latch logic available
#define PP_SelfStat_EEsize     0x1000  // Size of EEPROM

#define PP_BusStat   0x0138  // Bus status
#define PP_BusStat_TxBid       0x0080  // Tx error
#define PP_BusStat_TxRDY       0x0100  // Ready for Tx data

#define PP_LAF       0x0150  // Logical address filter (6 bytes)
#define PP_IA        0x0158  // Individual address (MAC)

#else

#define PP_ChipID    0x0000  // Chip identifier - must be 0x0e63
#define PP_ChipRev   0x0200  // Chip revision, model codes
#define PP_ChipID_CL           0x0e63

#define PP_IntReg    0x2200  // Interrupt configuration
#define PP_IntReg_IRQ0         0x0000  // Use INTR0 pin
#define PP_IntReg_IRQ1         0x0100  // Use INTR1 pin
#define PP_IntReg_IRQ2         0x0200  // Use INTR2 pin
#define PP_IntReg_IRQ3         0x0300  // Use INTR3 pin

#define PP_RxCFG     0x0201  // Receiver configuration
#define PP_RxCFG_Skip1         0x4000  // Skip (i.e. discard) current frame
#define PP_RxCFG_Stream        0x8000  // Enable streaming mode
#define PP_RxCFG_RxOK          0x0001  // RxOK interrupt enable
#define PP_RxCFG_RxDMAonly     0x0002  // Use RxDMA for all frames
#define PP_RxCFG_AutoRxDMA     0x0004  // Select RxDMA automatically
#define PP_RxCFG_BufferCRC     0x0008  // Include CRC characters in frame
#define PP_RxCFG_CRC           0x0010  // Enable interrupt on CRC error
#define PP_RxCFG_RUNT          0x0020  // Enable interrupt on RUNT frames
#define PP_RxCFG_EXTRA         0x0040  // Enable interrupt on frames with extra data

#define PP_RxCTL     0x0401  // Receiver control
#define PP_RxCTL_IAHash        0x4000  // Accept frames that match hash
#define PP_RxCTL_Promiscuous   0x8000  // Accept any frame
#define PP_RxCTL_RxOK          0x0001  // Accept well formed frames
#define PP_RxCTL_Multicast     0x0002  // Accept multicast frames
#define PP_RxCTL_IA            0x0004  // Accept frame that matches IA
#define PP_RxCTL_Broadcast     0x0008  // Accept broadcast frames
#define PP_RxCTL_CRC           0x0010  // Accept frames with bad CRC
#define PP_RxCTL_RUNT          0x0020  // Accept runt frames
#define PP_RxCTL_EXTRA         0x0040  // Accept frames that are too long

#define PP_TxCFG     0x0601  // Transmit configuration
#define PP_TxCFG_CRS           0x4000  // Enable interrupt on loss of carrier
#define PP_TxCFG_SQE           0x8000  // Enable interrupt on Signal Quality Error
#define PP_TxCFG_TxOK          0x0001  // Enable interrupt on successful xmits
#define PP_TxCFG_Late          0x0002  // Enable interrupt on "out of window" 
#define PP_TxCFG_Jabber        0x0004  // Enable interrupt on jabber detect
#define PP_TxCFG_Collision     0x0008  // Enable interrupt if collision
#define PP_TxCFG_16Collisions  0x0080  // Enable interrupt if > 16 collisions

#define PP_TxCmd     0x0801  // Transmit command status
#define PP_TxCmd_TxStart_5     0x0000  // Start after 5 bytes in buffer
#define PP_TxCmd_TxStart_381   0x4000  // Start after 381 bytes in buffer
#define PP_TxCmd_TxStart_1021  0x8000  // Start after 1021 bytes in buffer
#define PP_TxCmd_TxStart_Full  0xC000  // Start after all bytes loaded
#define PP_TxCmd_Force         0x0001  // Discard any pending packets
#define PP_TxCmd_OneCollision  0x0002  // Abort after a single collision
#define PP_TxCmd_NoCRC         0x0010  // Do not add CRC
#define PP_TxCmd_NoPad         0x0020  // Do not pad short packets

#define PP_BufCFG    0x0A01  // Buffer configuration
#define PP_BufCFG_SWI          0x4000  // Force interrupt via software
#define PP_BufCFG_RxDMA        0x8000  // Enable interrupt on Rx DMA
#define PP_BufCFG_TxRDY        0x0001  // Enable interrupt when ready for Tx
#define PP_BufCFG_TxUE         0x0002  // Enable interrupt in Tx underrun
#define PP_BufCFG_RxMiss       0x0004  // Enable interrupt on missed Rx packets
#define PP_BufCFG_Rx128        0x0008  // Enable Rx interrupt after 128 bytes
#define PP_BufCFG_TxCol        0x0010  // Enable int on Tx collision ctr overflow
#define PP_BufCFG_Miss         0x0020  // Enable int on Rx miss ctr overflow
#define PP_BufCFG_RxDest       0x0080  // Enable int on Rx dest addr match

#define PP_LineCTL   0x1201  // Line control
#define PP_LineCTL_Rx          0x4000  // Enable receiver
#define PP_LineCTL_Tx          0x8000  // Enable transmitter

#define PP_RER       0x2401  // Receive event
#define PP_RER_IAHash          0x4000  // Frame hash match
#define PP_RER_Dribble         0x8000  // Frame had 1-7 extra bits after last byte
#define PP_RER_RxOK            0x0001  // Frame received with no errors
#define PP_RER_Hashed          0x0002  // Frame address hashed OK
#define PP_RER_IA              0x0004  // Frame address matched IA
#define PP_RER_Broadcast       0x0008  // Broadcast frame
#define PP_RER_CRC             0x0010  // Frame had CRC error
#define PP_RER_RUNT            0x0020  // Runt frame
#define PP_RER_EXTRA           0x0040  // Frame was too long

#define PP_TER       0x2801 // Transmit event
#define PP_TER_CRS             0x4000  // Carrier lost
#define PP_TER_SQE             0x8000  // Signal Quality Error
#define PP_TER_TxOK            0x0001  // Packet sent without error
#define PP_TER_Late            0x0002  // Out of window
#define PP_TER_Jabber          0x0004  // Stuck transmit?
#define PP_TER_NumCollisions   0x0078  // Number of collisions
#define PP_TER_16Collisions    0x0080  // > 16 collisions

#define PP_SelfCtl   0x1401  // Chip control
#define PP_SelfCtl_Reset       0x4000  // Self-clearing reset

#define PP_BusCtl    0x1601  // Bus control
#define PP_BusCtl_ResetRxDMA   0x4000  // Reset receiver DMA engine
#define PP_BusCtl_DMAextend    0x0001
#define PP_BusCtl_UseSA        0x0002
#define PP_BusCtl_MemoryE      0x0004  // Enable "memory mode"
#define PP_BusCtl_DMAburst     0x0008
#define PP_BusCtl_IOCH_RDYE    0x0010
#define PP_BusCtl_RxDMAsize    0x0020
#define PP_BusCtl_EnableIRQ    0x0080  // Enable interrupts

#define PP_LineStat  0x3401  // Line status
#define PP_LineStat_LinkOK     0x8000  // Line is connected and working
#define PP_LineStat_AUI        0x0001  // Connected via AUI
#define PP_LineStat_10BT       0x0002  // Connected via twisted pair
#define PP_LineStat_Polarity   0x0010  // Line polarity OK (10BT only)
#define PP_LineStat_CRS        0x0040  // Frame being received

#define PP_SelfStat  0x3601  // Chip status
#define PP_SelfStat_InitD      0x8000  // Chip initialization complete
#define PP_SelfStat_SIBSY      0x0001  // EEPROM is busy
#define PP_SelfStat_EEPROM     0x0002  // EEPROM present
#define PP_SelfStat_EEPROM_OK  0x0004  // EEPROM checks out
#define PP_SelfStat_ELPresent  0x0008  // External address latch logic available
#define PP_SelfStat_EEsize     0x0010  // Size of EEPROM

#define PP_BusStat   0x3801  // Bus status
#define PP_BusStat_TxBid       0x8000  // Tx error
#define PP_BusStat_TxRDY       0x0001  // Ready for Tx data

#define PP_LAF       0x5001  // Logical address filter (6 bytes)
#define PP_IA        0x5801  // Individual address (MAC)

#endif

// ------------------------------------------------------------------------
// "page pointer" access functions
static __inline__ cyg_uint16
get_reg(cyg_addrword_t base, int regno)
{
    cyg_uint16 val;
    HAL_WRITE_UINT16(base+CS8900A_PPTR, regno);
    HAL_READ_UINT16(base+CS8900A_PDATA, val);
#if DEBUG & 2
    diag_printf("get_reg(%p, %d) => 0x%04x\n", base, regno, val);
#endif
    return val;
}

static __inline__ void
put_reg(cyg_addrword_t base, int regno, cyg_uint16 val)
{
#if DEBUG & 2
    diag_printf("put_reg(%p, %d, 0x%04x)\n", base, regno, val);
#endif
    HAL_WRITE_UINT16(base+CS8900A_PPTR, regno);
    HAL_WRITE_UINT16(base+CS8900A_PDATA, val);
}

#endif // _CYGONCE_ETH_CL_CS8900_H_
