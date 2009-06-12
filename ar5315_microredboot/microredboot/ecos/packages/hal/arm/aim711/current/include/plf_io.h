#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      Platform specific registers
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-03-16
// Purpose:     ARM/KS32C platform specific registers
// Description: 
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Platform doesn't need address munging even if configures as big-endian

#define HAL_IO_MACROS_NO_ADDRESS_MUNGING

// non-caching by accessing addr|0x04000000

#define KS32C_REG_BASE              0x07ff0000

// -----------------------------------------------------------------------------
// System config (register bases and caching)
#define KS32C_SYSCFG                (KS32C_REG_BASE + 0x0000)

#define KS32C_SYSCFG_SDM            0x80000000
#define KS32C_SYSCFG_PD_ID_MASK     0x3c000000
#define KS32C_SYSCFG_SRBBP_MASK     0x03ff0000 // address/64k
#define KS32C_SYSCFG_ISBBP_MASK     0x0000ffc0 // a25-a16
#define KS32C_SYSCFG_CM_MASK        0x00000030
#define KS32C_SYSCFG_CM_4R_4C       0x00000000
#define KS32C_SYSCFG_CM_0R_8C       0x00000010
#define KS32C_SYSCFG_CM_8R_0C       0x00000020
#define KS32C_SYSCFG_WE             0x00000004 // only KS32C50100?
#define KS32C_SYSCFG_CE             0x00000002
#define KS32C_SYSCFG_SE             0x00000001

#define KS32C_CLKCON                (KS32C_REG_BASE + 0x3000)

#define KS32C_EXTACON0              (KS32C_REG_BASE + 0x3008)
#define KS32C_EXTACON1              (KS32C_REG_BASE + 0x300c)

#define KS32C_EXTACON0_EXT0_shift   0
#define KS32C_EXTACON0_EXT1_shift   16
#define KS32C_EXTACON1_EXT2_shift   0
#define KS32C_EXTACON1_EXT3_shift   16

#define KS32C_EXTACON_TCOS_shift    0
#define KS32C_EXTACON_TACS_shift    3
#define KS32C_EXTACON_TCOH_shift    6
#define KS32C_EXTACON_TACC_shift    9

#define KS32C_EXTACON_INIT(_tacs_,_tcos_,_tacc_,_tcoh_) \
    ( ((_tacs_)<<KS32C_EXTACON_TACS_shift) \
    | ((_tcos_)<<KS32C_EXTACON_TCOS_shift) \
    | ((_tacc_)<<KS32C_EXTACON_TACC_shift) \
    | ((_tcoh_)<<KS32C_EXTACON_TCOH_shift) )

// Memory banks data width
#define KS32C_EXTDBWTH              (KS32C_REG_BASE + 0x3010)

#define KS32C_EXTDBWTH_MASK         3
#define KS32C_EXTDBWTH_8BIT         1
#define KS32C_EXTDBWTH_16BIT        2
#define KS32C_EXTDBWTH_32BIT        3

#define KS32C_EXTDBWTH_DSR0_shift   0
#define KS32C_EXTDBWTH_DSR1_shift   2
#define KS32C_EXTDBWTH_DSR2_shift   4
#define KS32C_EXTDBWTH_DSR3_shift   6
#define KS32C_EXTDBWTH_DSR4_shift   8
#define KS32C_EXTDBWTH_DSR5_shift   10
#define KS32C_EXTDBWTH_DSD0_shift   12
#define KS32C_EXTDBWTH_DSD1_shift   14
#define KS32C_EXTDBWTH_DSD2_shift   16
#define KS32C_EXTDBWTH_DSD3_shift   18
#define KS32C_EXTDBWTH_DSX0_shift   20
#define KS32C_EXTDBWTH_DSX1_shift   22
#define KS32C_EXTDBWTH_DSX2_shift   24
#define KS32C_EXTDBWTH_DSX3_shift   26

// -----------------------------------------------------------------------------
// Bank locations and timing
#define KS32C_ROMCON0               (KS32C_REG_BASE + 0x3014)
#define KS32C_ROMCON1               (KS32C_REG_BASE + 0x3018)
#define KS32C_ROMCON2               (KS32C_REG_BASE + 0x301c)
#define KS32C_ROMCON3               (KS32C_REG_BASE + 0x3020)
#define KS32C_ROMCON4               (KS32C_REG_BASE + 0x3024)
#define KS32C_ROMCON5               (KS32C_REG_BASE + 0x3028)

#define KS32C_ROMCON_PMC_MASK       0x00000003
#define KS32C_ROMCON_PMC_ROM        0x00000000
#define KS32C_ROMCON_PMC_4W_PAGE    0x00000001
#define KS32C_ROMCON_PMC_8W_PAGE    0x00000002
#define KS32C_ROMCON_PMC_16W_PAGE   0x00000003

#define KS32C_ROMCON_TPA_MASK       0x0000000c
#define KS32C_ROMCON_TPA_5C         0x00000000
#define KS32C_ROMCON_TPA_2C         0x00000004
#define KS32C_ROMCON_TPA_3C         0x00000008
#define KS32C_ROMCON_TPA_4C         0x0000000c

#define KS32C_ROMCON_TACC_MASK      0x00000070
#define KS32C_ROMCON_TACC_DISABLE   0x00000000
#define KS32C_ROMCON_TACC_2C        0x00000010
#define KS32C_ROMCON_TACC_3C        0x00000020
#define KS32C_ROMCON_TACC_4C        0x00000030
#define KS32C_ROMCON_TACC_5C        0x00000040
#define KS32C_ROMCON_TACC_6C        0x00000050
#define KS32C_ROMCON_TACC_7C        0x00000060

#define KS32C_ROMCON_BASE_MASK      0x000ffc00
#define KS32C_ROMCON_BASE_shift     10

#define KS32C_ROMCON_NEXT_MASK      0x3ff00000
#define KS32C_ROMCON_NEXT_shift     20



#define KS32C_DRAMCON0              (KS32C_REG_BASE + 0x302c)
#define KS32C_DRAMCON1              (KS32C_REG_BASE + 0x3030)
#define KS32C_DRAMCON2              (KS32C_REG_BASE + 0x3034)
#define KS32C_DRAMCON3              (KS32C_REG_BASE + 0x3038)

#define KS32C_DRAMCON_CAN_8         0x00000000
#define KS32C_DRAMCON_CAN_9         0x40000000
#define KS32C_DRAMCON_CAN_10        0x80000000
#define KS32C_DRAMCON_CAN_11        0xc0000000
#define KS32C_DRAMCON_TRP_1C        0x00000000
#define KS32C_DRAMCON_TRP_2C        0x00000100
#define KS32C_DRAMCON_TRP_3C        0x00000200
#define KS32C_DRAMCON_TRP_4C        0x00000300
#define KS32C_DRAMCON_TRC_1C        0x00000000
#define KS32C_DRAMCON_TRC_2C        0x00000080
#define KS32C_DRAMCON_RESERVED      0x00000010
#define KS32C_DRAMCON_TCP_1C        0x00000000
#define KS32C_DRAMCON_TCP_2C        0x00000008
#define KS32C_DRAMCON_TCS_1C        0x00000000
#define KS32C_DRAMCON_TCS_2C        0x00000002
#define KS32C_DRAMCON_TCS_3C        0x00000004
#define KS32C_DRAMCON_TCS_4C        0x00000006
#define KS32C_DRAMCON_EDO           0x00000001

#define KS32C_DRAMCON_BASE_MASK      0x000ffc00
#define KS32C_DRAMCON_BASE_shift     10

#define KS32C_DRAMCON_NEXT_MASK      0x3ff00000
#define KS32C_DRAMCON_NEXT_shift     20


#define KS32C_REFEXTCON             (KS32C_REG_BASE + 0x303c)

// DRAM
#define KS32C_REFEXTCON_TCSR_1C     0x00000000
#define KS32C_REFEXTCON_TCHR_1C     0x00000000
// SDRAM
#define KS32C_REFEXTCON_TRC_4C      0x00060000
// DRAM+SDRAM
#define KS32C_REFEXTCON_REN         0x00010000
#define KS32C_REFEXTCON_VSF         0x00008000
#define KS32C_REFEXTCON_BASE        0x00000360

#define KS32C_REFEXTCON_RCV_shift   21

//-----------------------------------------------------------------------------
// INTC

#define KS32C_INTMOD                (KS32C_REG_BASE + 0x4000)
#define KS32C_INTPND                (KS32C_REG_BASE + 0x4004)
#define KS32C_INTMSK                (KS32C_REG_BASE + 0x4008)
#define KS32C_INTPRI0               (KS32C_REG_BASE + 0x400c)
#define KS32C_INTPRI1               (KS32C_REG_BASE + 0x4010)
#define KS32C_INTPRI2               (KS32C_REG_BASE + 0x4014)
#define KS32C_INTPRI3               (KS32C_REG_BASE + 0x4018)
#define KS32C_INTPRI4               (KS32C_REG_BASE + 0x401c)
#define KS32C_INTPRI5               (KS32C_REG_BASE + 0x4020)
#define KS32C_INTOFFSET             (KS32C_REG_BASE + 0x4024)
#define KS32C_PNDPRI                (KS32C_REG_BASE + 0x4028)
#define KS32C_PNDTEST               (KS32C_REG_BASE + 0x402c)
#define KS32C_INTOFFSET_FIQ         (KS32C_REG_BASE + 0x4030)
#define KS32C_INTOFFSET_IRQ         (KS32C_REG_BASE + 0x4034)

#define KS32C_INTMSK_GLOBAL         (1<<21)

//-----------------------------------------------------------------------------
// PIO

#define KS32C_IOPMOD                (KS32C_REG_BASE + 0x5000)
#define KS32C_IOPCON                (KS32C_REG_BASE + 0x5004)

#define KS32C_IOPCON_XIRQ_MASK      0x1f
#define KS32C_IOPCON_XIRQ_LEVEL     0x00
#define KS32C_IOPCON_XIRQ_RISING    0x01
#define KS32C_IOPCON_XIRQ_FALLING   0x02
#define KS32C_IOPCON_XIRQ_BOTH_EDGE 0x03
#define KS32C_IOPCON_XIRQ_FILTERING 0x04
#define KS32C_IOPCON_XIRQ_AKTIV_LOW 0x00
#define KS32C_IOPCON_XIRQ_AKTIV_HI  0x08
#define KS32C_IOPCON_XIRQ_ENABLE    0x10

#define KS32C_IOPCON_XIRQ0_shift     0
#define KS32C_IOPCON_XIRQ1_shift     5
#define KS32C_IOPCON_XIRQ2_shift    10
#define KS32C_IOPCON_XIRQ3_shift    15

#define KS32C_IOPCON_DRQ_MASK       0x07
#define KS32C_IOPCON_DRQ_AKTIV_LOW  0x00
#define KS32C_IOPCON_DRQ_AKTIV_HI   0x01
#define KS32C_IOPCON_DRQ_FILTERING  0x02
#define KS32C_IOPCON_DRQ_ENABLE     0x04

#define KS32C_IOPCON_DRQ0_shift     20
#define KS32C_IOPCON_DRQ1_shift     23

#define KS32C_IOPCON_DAK_MASK       0x03
#define KS32C_IOPCON_DAK_AKTIV_LOW  0x00
#define KS32C_IOPCON_DAK_AKTIV_HI   0x01
#define KS32C_IOPCON_DAK_ENABLE     0x02

#define KS32C_IOPCON_DAK0_shift     26
#define KS32C_IOPCON_DAK1_shift     28

#define KS32C_IOPCON_TOEN_ENABLE    0x01

#define KS32C_IOPCON_TO0_shift      30
#define KS32C_IOPCON_TO1_shift      31

#define KS32C_IOPDATA               (KS32C_REG_BASE + 0x5008)

#define KS32C_IOPDATA_P0            (1<<0)
#define KS32C_IOPDATA_P1            (1<<1)
#define KS32C_IOPDATA_P2            (1<<2)
#define KS32C_IOPDATA_P3            (1<<3)
#define KS32C_IOPDATA_P4            (1<<4)
#define KS32C_IOPDATA_P5            (1<<5)
#define KS32C_IOPDATA_P6            (1<<6)
#define KS32C_IOPDATA_P7            (1<<7)
#define KS32C_IOPDATA_P8_XIRQ0      (1<<8)
#define KS32C_IOPDATA_P9_XIRQ1      (1<<9)
#define KS32C_IOPDATA_P10_XIRQ2     (1<<10)
#define KS32C_IOPDATA_P11_XIRQ3     (1<<11)
#define KS32C_IOPDATA_P12_DRQ0      (1<<12)
#define KS32C_IOPDATA_P13_DRQ1      (1<<13)
#define KS32C_IOPDATA_P14_DAK0      (1<<14)
#define KS32C_IOPDATA_P15_DAK1      (1<<15)
#define KS32C_IOPDATA_P16_TO0       (1<<16)
#define KS32C_IOPDATA_P17_TO1       (1<<17)

//-----------------------------------------------------------------------------
// Timers

#define KS32C_TMOD                  (KS32C_REG_BASE + 0x6000)
#define KS32C_TDATA0                (KS32C_REG_BASE + 0x6004)
#define KS32C_TDATA1                (KS32C_REG_BASE + 0x6008)
#define KS32C_TCNT0                 (KS32C_REG_BASE + 0x600c)
#define KS32C_TCNT1                 (KS32C_REG_BASE + 0x6010)

#define KS32C_TMOD_TE0              0x00000001
#define KS32C_TMOD_TMD0             0x00000002
#define KS32C_TMOD_TCLR0            0x00000004
#define KS32C_TMOD_TE1              0x00000008
#define KS32C_TMOD_TMD1             0x00000010
#define KS32C_TMOD_TCLR1            0x00000020


//-----------------------------------------------------------------------------
// UART

#define KS32C_UART0_BASE            (KS32C_REG_BASE + 0xd000)
#define KS32C_UART1_BASE            (KS32C_REG_BASE + 0xe000)

#define KS32C_UART_LCON             0x0000
#define KS32C_UART_CON              0x0004
#define KS32C_UART_STAT             0x0008
#define KS32C_UART_TXBUF            0x000c
#define KS32C_UART_RXBUF            0x0010
#define KS32C_UART_BRDIV            0x0014
#define KS32C_UART_BRDCNT           0x0018
#define KS32C_UART_BRDCLK           0x001c

#define KS32C_UART_LCON_5_DBITS     0x00
#define KS32C_UART_LCON_6_DBITS     0x01
#define KS32C_UART_LCON_7_DBITS     0x02
#define KS32C_UART_LCON_8_DBITS     0x03
#define KS32C_UART_LCON_1_SBITS     0x00
#define KS32C_UART_LCON_2_SBITS     0x04
#define KS32C_UART_LCON_NO_PARITY   0x00
#define KS32C_UART_LCON_EVEN_PARITY 0x00
#define KS32C_UART_LCON_ODD_PARITY  0x28
#define KS32C_UART_LCON_1_PARITY    0x30
#define KS32C_UART_LCON_0_PARITY    0x38
#define KS32C_UART_LCON_SCS         0x40
#define KS32C_UART_LCON_IR          0x80

#define KS32C_UART_CON_RXM_MASK     0x03
#define KS32C_UART_CON_RXM_INT      0x01
#define KS32C_UART_CON_TXM_MASK     0x0c
#define KS32C_UART_CON_TXM_INT      0x08
#define KS32C_UART_CON_RX_ERR_INT   0x04


#define KS32C_UART_STAT_DTR         0x10
#define KS32C_UART_STAT_RDR         0x20
#define KS32C_UART_STAT_TXE         0x40  // tx empty
#define KS32C_UART_STAT_TC          0x80  // tx complete

//-----------------------------------------------------------------------------
// Cache
#define KS32C_CACHE_SET0_ADDR       0x10000000
#define KS32C_CACHE_SET1_ADDR       0x10800000
#define KS32C_CACHE_TAG_ADDR        0x11000000

//-----------------------------------------------------------------------------
// GDMA
#define KS32C_GDMA_CON0             (KS32C_REG_BASE + 0xb000)
#define KS32C_GDMA_SRC0             (KS32C_REG_BASE + 0xb004)
#define KS32C_GDMA_DST0             (KS32C_REG_BASE + 0xb008)
#define KS32C_GDMA_CNT0             (KS32C_REG_BASE + 0xb00c)
#define KS32C_GDMA_CON1             (KS32C_REG_BASE + 0xc000)
#define KS32C_GDMA_SRC1             (KS32C_REG_BASE + 0xc004)
#define KS32C_GDMA_DST1             (KS32C_REG_BASE + 0xc008)
#define KS32C_GDMA_CNT1             (KS32C_REG_BASE + 0xc00c)

//-----------------------------------------------------------------------------
// I2C
#define KS32C_I2CCON                (KS32C_REG_BASE + 0xf000)
#define KS32C_I2C_CON_BF            (1<<0)
#define KS32C_I2C_CON_IEN           (1<<1)
#define KS32C_I2C_CON_LRB           (1<<2)
#define KS32C_I2C_CON_ACK           (1<<3)
#define KS32C_I2C_CON_START         (1<<4)
#define KS32C_I2C_CON_STOP          (2<<4)
#define KS32C_I2C_CON_RESTART       (3<<4)
#define KS32C_I2C_CON_BUSY          (1<<6)
#define KS32C_I2C_CON_RESET         (1<<7)
#define KS32C_I2CBUF                (KS32C_REG_BASE +0xf004)
#define KS32C_I2CPS                 (KS32C_REG_BASE +0xf008)

#define KS32C_I2C_FREQ(freq)        ((CYGNUM_HAL_CPUCLOCK/(freq) - 3)/16)
#define KS32C_I2C_RD                (0x01)
#define KS32C_I2C_WR                (0x00)

#ifndef __ASSEMBLER__
typedef struct hal_ks32c_i2c_msg_s
{
    cyg_uint8   devaddr;
    cyg_int8    status;
    cyg_uint8*  pbuf;
    cyg_uint32  bufsize;
} hal_ks32c_i2c_msg_t;

//  Transfer the I2C messages.
externC int
hal_ks32c_i2c_transfer(cyg_uint32 nmsg, hal_ks32c_i2c_msg_t* pmsgs);
#endif

//-----------------------------------------------------------------------------
// Memory map is 1-1

#define CYGARC_PHYSICAL_ADDRESS(_x_) (_x_)

//-----------------------------------------------------------------------------
// AIM 711 specific

// Mamory maping
#define AIM711_ROM0_LA_START    0x02000000
#define AIM711_ROM0_LA_END      0x02200000
#define AIM711_DRAM_LA_START    0x00000000
#define AIM711_DRAM_LA_END      0x00800000
#define AIM711_EXT0_LA_START    0x03fd0000
#define AIM711_EXT0_LA_END      0x03fd4000
#define AIM711_EXT1_LA_START    0x03fd4000
#define AIM711_EXT1_LA_END      0x03fd8000
#define AIM711_EXT2_LA_START    0x03fd8000
#define AIM711_EXT2_LA_END      0x03fdc000
#define AIM711_EXT3_LA_START    0x03fdc000
#define AIM711_EXT3_LA_END      0x03fc0000

#define AIM711_COM0_DEBUG_BASE  KS32C_UART0_BASE
#define AIM711_COM1_BASE        (AIM711_EXT0_LA_START|0x04000000 + 8)
#define AIM711_COM2_BASE        KS32C_UART1_BASE
#define AIM711_EXTBUS_BASE      (AIM711_EXT2_LA_START|0x04000000)

// I2C address of RTC (wallclock)
#define AIM711_RTC_ADDR        0xd0

// I2C address and size of EEPROM
#define AIM711_EEPROM_ADDR     0xa0
#define AIM711_EEPROM_SIZE     256
#define AIM711_EEPROM_PAGESIZE 8

// Interrupt vectors with AIM 711 naming
#define AIM711_INTERRUPT_COM1    CYGNUM_HAL_INTERRUPT_EXT0
#define AIM711_INTERRUPT_RTC     CYGNUM_HAL_INTERRUPT_EXT1
#define AIM711_INTERRUPT_IRQ0    CYGNUM_HAL_INTERRUPT_EXT2
#define AIM711_INTERRUPT_IRQ1    CYGNUM_HAL_INTERRUPT_EXT3

// GPIO bits with AIM 711 naming
#define AIM711_GPIO_LED0         KS32C_IOPDATA_P0
#define AIM711_GPIO_LED1         KS32C_IOPDATA_P1
#define AIM711_GPIO_LED2         KS32C_IOPDATA_P2
#define AIM711_GPIO_RESET        KS32C_IOPDATA_P3
#define AIM711_GPIO_POWERLED     KS32C_IOPDATA_P4
#define AIM711_GPIO_UARTIRQ      KS32C_IOPDATA_P8_XIRQ0
#define AIM711_GPIO_RTCIRQ       KS32C_IOPDATA_P9_XIRQ1
#define AIM711_GPIO_DIN0_DRQ0    KS32C_IOPDATA_P12_DRQ0
#define AIM711_GPIO_DIN1_DRQ1    KS32C_IOPDATA_P13_DRQ1
#define AIM711_GPIO_DIN2_IRQ0    KS32C_IOPDATA_P10_XIRQ2
#define AIM711_GPIO_DIN3_IRQ1    KS32C_IOPDATA_P11_XIRQ3
#define AIM711_GPIO_DOUT0_DAK0   KS32C_IOPDATA_P14_DAK0
#define AIM711_GPIO_DOUT1_DAK1   KS32C_IOPDATA_P15_DAK1
#define AIM711_GPIO_DOUT2_TO0    KS32C_IOPDATA_P16_TO0
#define AIM711_GPIO_DOUT3_TO1    KS32C_IOPDATA_P17_TO1

// Macros for usage of GPIO
#define AIM711_GPIO(_which_,_value_) \
do { \
    cyg_uint32 val; \
    HAL_READ_UINT32(KS32C_IOPDATA, val); \
    val &= ~(_which_); \
    val |= (_which_)&(_value_); \
    HAL_WRITE_UINT32(KS32C_IOPDATA, val); \
} while (0)

#define AIM711_GPIO_SET(_x_) \
do { \
    cyg_uint32 val; \
    HAL_READ_UINT32(KS32C_IOPDATA, val); \
    val |= (_x_); \
    HAL_WRITE_UINT32(KS32C_IOPDATA, val); \
} while (0)

#define AIM711_GPIO_CLR(_x_) \
do { \
    cyg_uint32 val; \
    HAL_READ_UINT32(KS32C_IOPDATA, val); \
    val &= ~(_x_); \
    HAL_WRITE_UINT32(KS32C_IOPDATA, val); \
} while (0)

#define AIM711_GPIO_GET(_x_) \
do { \
    cyg_uint32 _val; \
    HAL_READ_UINT32(KS32C_IOPDATA, _val); \
    (_x_) = _val; \
} while (0)

//-----------------------------------------------------------------------------
// AIM 711 specific EEPROM support

#ifndef __ASSEMBLER__
externC int
hal_aim711_eeprom_read(cyg_uint8 *buf, int offset, int len);

externC int
hal_aim711_eeprom_write(cyg_uint8 *buf, int offset, int len);
#endif

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
