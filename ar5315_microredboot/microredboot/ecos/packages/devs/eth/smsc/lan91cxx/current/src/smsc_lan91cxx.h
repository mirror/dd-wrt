#ifndef CYGONCE_DEVS_ETH_SMSC_LAN91CXX_LAN91CXX_H
#define CYGONCE_DEVS_ETH_SMSC_LAN91CXX_LAN91CXX_H
//==========================================================================
//
//      lan91cxx.h
//
//      SMCS LAN91C110 (LAN91CXX compatible) Ethernet chip
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
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
// Author(s):    jskov
// Contributors: jskov, hmt, jco, nickg
// Date:         2001-01-22
// Purpose:      Hardware description of LAN9000 series, LAN91C96/110.
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_endian.h>

#define LAN91CXX_TCR         0x00
#define LAN91CXX_EPH_STATUS  0x01
#define LAN91CXX_RCR         0x02
#define LAN91CXX_COUNTER     0x03
#define LAN91CXX_MIR         0x04
#define LAN91CXX_MCR         0x05 // Other than 91C111
#define LAN91CXX_RPCR        0x05 // 91C111 only
#define LAN91CXX_RESERVED_0  0x06
#define LAN91CXX_BS          0x07
#define LAN91CXX_CONFIG      0x08
#define LAN91CXX_BASE_REG    0x09
#define LAN91CXX_IA01        0x0a
#define LAN91CXX_IA23        0x0b
#define LAN91CXX_IA45        0x0c
#define LAN91CXX_GENERAL     0x0d // 91C96 - was "RESERVED_1" for others
#define LAN91CXX_CONTROL     0x0e
#define LAN91CXX_BS2         0x0f
#define LAN91CXX_MMU_COMMAND 0x10
#define LAN91CXX_PNR         0x11
#define LAN91CXX_FIFO_PORTS  0x12
#define LAN91CXX_POINTER     0x13
#define LAN91CXX_DATA_HIGH   0x14
#define LAN91CXX_DATA        0x15
#define LAN91CXX_INTERRUPT   0x16
#define LAN91CXX_BS3         0x17
#define LAN91CXX_MT01        0x18
#define LAN91CXX_MT23        0x19
#define LAN91CXX_MT45        0x1a
#define LAN91CXX_MT67        0x1b
#define LAN91CXX_MGMT        0x1c
#define LAN91CXX_REVISION    0x1d
#define LAN91CXX_ERCV        0x1e
#define LAN91CXX_BS4         0x1f

#define LAN91CXX_RCR_SOFT_RST   0x8000    // soft reset
#define LAN91CXX_RCR_FILT_CAR   0x4000    // filter carrier
#define LAN91CXX_RCR_ABORT_ENB  0x2000    // abort on collision
#define LAN91CXX_RCR_STRIP_CRC  0x0200    // strip CRC
#define LAN91CXX_RCR_RXEN       0x0100    // enable RX
#define LAN91CXX_RCR_ALMUL      0x0004    // receive all muticasts
#define LAN91CXX_RCR_PRMS       0x0002    // promiscuous
#define LAN91CXX_RCR_RX_ABORT   0x0001    // set when abort due to long frame

#define LAN91CXX_TCR_SWFDUP     0x8000    // Switched Full Duplex mode
#define LAN91CXX_TCR_ETEN_TYPE  0x4000    // ETEN type (91C96) 0 <=> like a 91C94
#define LAN91CXX_TCR_EPH_LOOP   0x2000    // loopback mode
#define LAN91CXX_TCR_STP_SQET   0x1000    // Stop transmission on SQET error
#define LAN91CXX_TCR_FDUPLX     0x0800    // full duplex
#define LAN91CXX_TCR_MON_CSN    0x0400    // monitor carrier during tx (91C96)
#define LAN91CXX_TCR_NOCRC      0x0100    // does not append CRC to frames
#define LAN91CXX_TCR_PAD_EN     0x0080    // pads frames with 00 to min length
#define LAN91CXX_TCR_FORCOL     0x0004    // force collision
#define LAN91CXX_TCR_LLOOP      0x0002    // local loopback (91C96)
#define LAN91CXX_TCR_TXENA      0x0001    // enable

#define LAN91CXX_POINTER_RCV        0x8000
#define LAN91CXX_POINTER_AUTO_INCR  0x4000
#define LAN91CXX_POINTER_READ       0x2000
#define LAN91CXX_POINTER_ETEN       0x1000
#define LAN91CXX_POINTER_NOT_EMPTY  0x0800


#define LAN91CXX_INTERRUPT_TX_IDLE_M      0x8000 // (91C96)
#define LAN91CXX_INTERRUPT_ERCV_INT_M     0x4000
#define LAN91CXX_INTERRUPT_EPH_INT_M      0x2000
#define LAN91CXX_INTERRUPT_RX_OVRN_INT_M  0x1000
#define LAN91CXX_INTERRUPT_ALLOC_INT_M    0x0800
#define LAN91CXX_INTERRUPT_TX_EMPTY_INT_M 0x0400
#define LAN91CXX_INTERRUPT_TX_INT_M       0x0200
#define LAN91CXX_INTERRUPT_RCV_INT_M      0x0100
#define LAN91CXX_INTERRUPT_TX_IDLE        0x0080 // (91C96)
#define LAN91CXX_INTERRUPT_ERCV_INT       0x0040 // also ack
#define LAN91CXX_INTERRUPT_EPH_INT        0x0020
#define LAN91CXX_INTERRUPT_RX_OVRN_INT    0x0010 // also ack
#define LAN91CXX_INTERRUPT_ALLOC_INT      0x0008
#define LAN91CXX_INTERRUPT_TX_EMPTY_INT   0x0004 // also ack
#define LAN91CXX_INTERRUPT_TX_INT         0x0002 // also ack
#define LAN91CXX_INTERRUPT_RCV_INT        0x0001

#if 1 // Whichever we choose, the behaviour is the same.
#define LAN91CXX_INTERRUPT_TX_SET         0x0002 // TX
#define LAN91CXX_INTERRUPT_TX_SET_ACK     0x0000 // -none-
#define LAN91CXX_INTERRUPT_TX_FIFO_ACK    0x0002 // TX alone
#define LAN91CXX_INTERRUPT_TX_SET_M       0x0200 // TX alone
#else
#define LAN91CXX_INTERRUPT_TX_SET         0x0006 // TX_EMPTY + TX
#define LAN91CXX_INTERRUPT_TX_SET_ACK     0x0004 // TX_EMPTY and not plain TX
#define LAN91CXX_INTERRUPT_TX_FIFO_ACK    0x0002 // TX alone
#define LAN91CXX_INTERRUPT_TX_SET_M       0x0600 // TX_EMPTY + TX
#endif

#define LAN91CXX_CONTROL_RCV_BAD       0x4000
#define LAN91CXX_CONTROL_AUTO_RELEASE  0x0800
#define LAN91CXX_CONTROL_LE_ENABLE     0x0080
#define LAN91CXX_CONTROL_CR_ENABLE     0x0040
#define LAN91CXX_CONTROL_TE_ENABLE     0x0020

// These are for setting the MAC address in the 91C96 serial EEPROM
#define LAN91CXX_CONTROL_EEPROM_SELECT 0x0004
#define LAN91CXX_CONTROL_RELOAD        0x0002
#define LAN91CXX_CONTROL_STORE         0x0001
#define LAN91CXX_CONTROL_EEPROM_BUSY   0x0003
#define LAN91CXX_ESA_EEPROM_OFFSET     0x0020

#define LAN91CXX_STATUS_TX_UNRN        0x8000
#define LAN91CXX_STATUS_LINK_OK        0x4000
#define LAN91CXX_STATUS_CTR_ROL        0x1000
#define LAN91CXX_STATUS_EXC_DEF        0x0800
#define LAN91CXX_STATUS_LOST_CARR      0x0400
#define LAN91CXX_STATUS_LATCOL         0x0200
#define LAN91CXX_STATUS_WAKEUP         0x0100
#define LAN91CXX_STATUS_TX_DEFR        0x0080
#define LAN91CXX_STATUS_LTX_BRD        0x0040
#define LAN91CXX_STATUS_SQET           0x0020
#define LAN91CXX_STATUS_16COL          0x0010
#define LAN91CXX_STATUS_LTX_MULT       0x0008
#define LAN91CXX_STATUS_MUL_COL        0x0004
#define LAN91CXX_STATUS_SNGL_COL       0x0002
#define LAN91CXX_STATUS_TX_SUC         0x0001

#define LAN91CXX_MMU_noop              0x0000
#define LAN91CXX_MMU_alloc_for_tx      0x0020
#define LAN91CXX_MMU_reset_mmu         0x0040
#define LAN91CXX_MMU_rem_rx_frame      0x0060
#define LAN91CXX_MMU_rem_tx_frame      0x0070 // (91C96) only when TX stopped
#define LAN91CXX_MMU_remrel_rx_frame   0x0080
#define LAN91CXX_MMU_rel_packet        0x00a0
#define LAN91CXX_MMU_enq_packet        0x00c0
#define LAN91CXX_MMU_reset_tx_fifo     0x00e0

#define LAN91CXX_CONTROLBYTE_CRC       0x1000
#define LAN91CXX_CONTROLBYTE_ODD       0x2000
#define LAN91CXX_CONTROLBYTE_RX        0x4000

#define LAN91CXX_RX_STATUS_ALIGNERR    0x8000
#define LAN91CXX_RX_STATUS_BCAST       0x4000
#define LAN91CXX_RX_STATUS_BADCRC      0x2000
#define LAN91CXX_RX_STATUS_ODDFRM      0x1000
#define LAN91CXX_RX_STATUS_TOOLONG     0x0800
#define LAN91CXX_RX_STATUS_TOOSHORT    0x0400
#define LAN91CXX_RX_STATUS_HASHVALMASK 0x007e // MASK
#define LAN91CXX_RX_STATUS_MCAST       0x0001
#define LAN91CXX_RX_STATUS_BAD     \
    (LAN91CXX_RX_STATUS_ALIGNERR | \
     LAN91CXX_RX_STATUS_BADCRC   | \
     LAN91CXX_RX_STATUS_TOOLONG  | \
     LAN91CXX_RX_STATUS_TOOSHORT)

// Attribute memory registers in PCMCIA mode
#define LAN91CXX_ECOR                  0x8000
#define LAN91CXX_ECOR_RESET            (1<<7)
#define LAN91CXX_ECOR_LEVIRQ           (1<<6)
#define LAN91CXX_ECOR_ATTWR            (1<<2)
#define LAN91CXX_ECOR_ENABLE           (1<<0)

#define LAN91CXX_ECSR                  0x8002
#define LAN91CXX_ECSR_IOIS8            (1<<5)
#define LAN91CXX_ECSR_PWRDWN           (1<<2)
#define LAN91CXX_ECSR_INTR             (1<<1)

// These are for manipulating the MII interface
#define LAN91CXX_MGMT_MDO              0x0001
#define LAN91CXX_MGMT_MDI              0x0002
#define LAN91CXX_MGMT_MCLK             0x0004
#define LAN91CXX_MGMT_MDOE             0x0008

// Internal PHY registers (91c111)
#define LAN91CXX_PHY_CTRL              0
#define LAN91CXX_PHY_STAT              1
#define LAN91CXX_PHY_ID1               2
#define LAN91CXX_PHY_ID2               3
#define LAN91CXX_PHY_AUTO_AD           4
#define LAN91CXX_PHY_AUTO_CAP          5
#define LAN91CXX_PHY_CONFIG1          16
#define LAN91CXX_PHY_CONFIG2          17
#define LAN91CXX_PHY_STATUS_OUT       18
#define LAN91CXX_PHY_MASK             19

// PHY control bits
#define LAN91CXX_PHY_CTRL_COLTST      (1 << 7)
#define LAN91CXX_PHY_CTRL_DPLX        (1 << 8)
#define LAN91CXX_PHY_CTRL_ANEG_RST    (1 << 9)
#define LAN91CXX_PHY_CTRL_MII_DIS     (1 << 10)
#define LAN91CXX_PHY_CTRL_PDN         (1 << 11)
#define LAN91CXX_PHY_CTRL_ANEG_EN     (1 << 12)
#define LAN91CXX_PHY_CTRL_SPEED       (1 << 13)
#define LAN91CXX_PHY_CTRL_LPBK        (1 << 14)
#define LAN91CXX_PHY_CTRL_RST         (1 << 15)

#define LAN91CXX_RPCR_LEDA_LINK       (0 << 2)
#define LAN91CXX_RPCR_LEDA_TXRX       (4 << 2)
#define LAN91CXX_RPCR_LEDA_RX         (6 << 2)
#define LAN91CXX_RPCR_LEDA_TX         (7 << 2)
#define LAN91CXX_RPCR_LEDB_LINK       (0 << 5)
#define LAN91CXX_RPCR_LEDB_TXRX       (4 << 5)
#define LAN91CXX_RPCR_LEDB_RX         (6 << 5)
#define LAN91CXX_RPCR_LEDB_TX         (7 << 5)
#define LAN91CXX_RPCR_ANEG            (1 << 11)
#define LAN91CXX_RPCR_DPLX            (1 << 12)
#define LAN91CXX_RPCR_SPEED           (1 << 13)


// ------------------------------------------------------------------------

#ifdef KEEP_STATISTICS
struct smsc_lan91cxx_stats {
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

struct lan91cxx_priv_data;
typedef cyg_bool (*provide_esa_t)(struct lan91cxx_priv_data* cpd);

typedef struct lan91cxx_priv_data {
    int txbusy;                         // A packet has been sent
    unsigned long txkey;                // Used to ack when packet sent
    unsigned short* base;               // Base I/O address of controller
                                        // (as it comes out of reset)
#if CYGINT_DEVS_ETH_SMSC_LAN91CXX_PCMCIA_MODE					
    unsigned char* attbase;             // Base attribute address of controller
                                        // only used in PCMCIA mode
#endif				
    int interrupt;                      // Interrupt vector used by controller
    unsigned char enaddr[6];            // Controller ESA
    // Function to configure the ESA - may fetch ESA from EPROM or 
    // RedBoot config option.  Use of the 'config_enaddr()' function
    // is depreciated in favor of the 'provide_esa()' function and
    // 'hardwired_esa' boolean
    void (*config_enaddr)(struct lan91cxx_priv_data* cpd);
    // New function to fetch the ESA from flash via RedBoot
    // (see devs_eth_innovator.inl)
    provide_esa_t provide_esa;
    bool hardwired_esa;
    int txpacket;
    int rxpacket;
    int within_send;
    int addrsh;                         // Address bits to shift
    int c111_reva;                      // true if this is a revA LAN91C111
#ifdef KEEP_STATISTICS
    struct smsc_lan91cxx_stats stats;
#endif
} lan91cxx_priv_data;

// ------------------------------------------------------------------------

#include CYGDAT_DEVS_ETH_SMSC_LAN91CXX_INL

#ifdef LAN91CXX_32BIT_RX
typedef cyg_uint32 rxd_t;
#else
typedef cyg_uint16 rxd_t;
#endif

#ifndef SMSC_PLATFORM_DEFINED_GET_REG
static __inline__ unsigned short
get_reg(struct eth_drv_sc *sc, int regno)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    unsigned short val;
    
    HAL_WRITE_UINT16(cpd->base+(LAN91CXX_BS << cpd->addrsh), CYG_CPU_TO_LE16(regno>>3));
    HAL_READ_UINT16(cpd->base+((regno&0x7) << cpd->addrsh), val);
    val = CYG_LE16_TO_CPU(val);

#if DEBUG & 2
    diag_printf("read reg %d val 0x%04x\n", regno, val);
#endif
    return val;
}
#endif // SMSC_PLATFORM_DEFINED_GET_REG

#ifndef SMSC_PLATFORM_DEFINED_PUT_REG
static __inline__ void
put_reg(struct eth_drv_sc *sc, int regno, unsigned short val)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
	
    HAL_WRITE_UINT16(cpd->base+(LAN91CXX_BS << cpd->addrsh), CYG_CPU_TO_LE16(regno>>3));
    HAL_WRITE_UINT16(cpd->base+((regno&0x7) << cpd->addrsh), CYG_CPU_TO_LE16(val));

#if DEBUG & 2
    diag_printf("write reg %d val 0x%04x\n", regno, val);
#endif
}
#endif // SMSC_PLATFORM_DEFINED_PUT_REG

#ifndef SMSC_PLATFORM_DEFINED_PUT_DATA
// ------------------------------------------------------------------------
// Assumes bank2 has been selected
static __inline__ void
put_data(struct eth_drv_sc *sc, unsigned short val)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
	
    HAL_WRITE_UINT16(cpd->base+((LAN91CXX_DATA & 0x7) << cpd->addrsh), val);

#if DEBUG & 2
    diag_printf("write data 0x%04x\n", val);
#endif
}
#endif // SMSC_PLATFORM_DEFINED_PUT_DATA

#ifndef SMSC_PLATFORM_DEFINED_GET_DATA
// Assumes bank2 has been selected
static __inline__ rxd_t
get_data(struct eth_drv_sc *sc)
{
    rxd_t val;
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
	
#ifdef LAN91CXX_32BIT_RX
    HAL_READ_UINT32(cpd->base+((LAN91CXX_DATA_HIGH & 0x7) << cpd->addrsh), val);
#else
    HAL_READ_UINT16(cpd->base+((LAN91CXX_DATA & 0x7) << cpd->addrsh), val);
#endif

#if DEBUG & 2
    diag_printf("read data 0x%x\n", val);
#endif
    return val;
}
#endif // SMSC_PLATFORM_DEFINED_GET_DATA

// ------------------------------------------------------------------------
// Read the bank register (this one is bank-independent)
#ifndef SMSC_PLATFORM_DEFINED_GET_BANKSEL
static __inline__ unsigned short
get_banksel(struct eth_drv_sc *sc)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    unsigned short val;
    
    HAL_READ_UINT16(cpd->base+(LAN91CXX_BS << cpd->addrsh), val);
    val = CYG_LE16_TO_CPU(val);
#if DEBUG & 2
    diag_printf("read bank val 0x%04x\n", val);
#endif
    return val;
}
#endif


// ------------------------------------------------------------------------
// Write on PCMCIA attribute memory
#if CYGINT_DEVS_ETH_SMSC_LAN91CXX_PCMCIA_MODE					
static __inline__ void
put_att(struct eth_drv_sc *sc, int offs, unsigned char val)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
	
    HAL_WRITE_UINT8(cpd->attbase + (offs << cpd->addrsh), CYG_CPU_TO_LE16(val));

#if DEBUG & 2
    diag_printf("write attr %d val 0x%02x\n", offs, val);
#endif
}

// Read from PCMCIA attribute memory
static __inline__ unsigned char
get_att(struct eth_drv_sc *sc, int offs)
{
    struct lan91cxx_priv_data *cpd =
        (struct lan91cxx_priv_data *)sc->driver_private;
    unsigned char val;
    
    HAL_READ_UINT8(cpd->attbase + (offs << cpd->addrsh), val);
    val = CYG_LE16_TO_CPU(val);
#if DEBUG & 2
    diag_printf("read attr %d val 0x%02x\n", offs, val);
#endif
    return val;
}
#endif // #if CYGINT_DEVS_ETH_SMSC_LAN91CXX_PCMCIA_MODE					

// ------------------------------------------------------------------------
#endif // CYGONCE_DEVS_ETH_SMSC_LAN91CXX_LAN91CXX_H
// EOF smsc_lan91cxx.h
