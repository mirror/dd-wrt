/*=============================================================================
//
//      hal_verde.h
//
//      Verde I/O Coprocessor support (register layout, etc)
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-12-03
// Purpose:      
// Description:  Verde I/O Processor support.
// Usage:        #include <cyg/hal/hal_verde.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_VERDE_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_VERDE_H

#include <pkgconf/system.h>
#include <cyg/hal/hal_xscale.h>


// --------------------------------------------------------------------------
// Address Translation Unit  (Chapter 3)
#define ATU_ATUVID	REG16(0,0xffffe100)
#define ATU_ATUDID	REG16(0,0xffffe102)
#define ATU_ATUCMD	REG16(0,0xffffe104)
#define ATU_ATUSR	REG16(0,0xffffe106)
#define ATU_ATURID	REG8(0,0xffffe108)
#define ATU_ATUCCR	REG8(0,0xffffe109)
#define ATU_ATUCLSR	REG8(0,0xffffe10c)
#define ATU_ATULT	REG8(0,0xffffe10d)
#define ATU_ATUHTR	REG8(0,0xffffe10e)
#define ATU_ATUBIST	REG8(0,0xffffe10f)
#define ATU_IABAR0	REG32(0,0xffffe110)
#define ATU_IAUBAR0	REG32(0,0xffffe114)
#define ATU_IABAR1	REG32(0,0xffffe118)
#define ATU_IAUBAR1	REG32(0,0xffffe11c)
#define ATU_IABAR2	REG32(0,0xffffe120)
#define ATU_IAUBAR2	REG32(0,0xffffe124)
#define ATU_ASVIR	REG16(0,0xffffe12c)
#define ATU_ASIR	REG16(0,0xffffe12e)
#define ATU_ERBAR	REG32(0,0xffffe130)
#define ATU_ATUILR	REG8(0,0xffffe13c)
#define ATU_ATUIPR	REG8(0,0xffffe13d)
#define ATU_ATUMGNT	REG8(0,0xffffe13e)
#define ATU_ATUMLAT	REG8(0,0xffffe13f)
#define ATU_IALR0	REG32(0,0xffffe140)
#define ATU_IATVR0	REG32(0,0xffffe144)
#define ATU_ERLR	REG32(0,0xffffe148)
#define ATU_ERTVR	REG32(0,0xffffe14c)
#define ATU_IALR1	REG32(0,0xffffe150)
#define ATU_IALR2	REG32(0,0xffffe154)
#define ATU_IATVR2	REG32(0,0xffffe158)
#define ATU_OIOWTVR	REG32(0,0xffffe15c)
#define ATU_OMWTVR0	REG32(0,0xffffe160)
#define ATU_OUMWTVR0	REG32(0,0xffffe164)
#define ATU_OMWTVR1	REG32(0,0xffffe168)
#define ATU_OUMWTVR1	REG32(0,0xffffe16c)
#define ATU_OUDWTVR	REG32(0,0xffffe178)
#define ATU_ATUCR	REG32(0,0xffffe180)
#define ATU_PCSR	REG32(0,0xffffe184)
#define ATU_ATUISR	REG32(0,0xffffe188)
#define ATU_ATUIMR	REG32(0,0xffffe18c)
#define ATU_IABAR3	REG32(0,0xffffe190)
#define ATU_IAUBAR3	REG32(0,0xffffe194)
#define ATU_IALR3	REG32(0,0xffffe198)
#define ATU_IATVR3	REG32(0,0xffffe19c)
#define ATU_OCCAR	REG32(0,0xffffe1a4)
#define ATU_OCCDR	REG32(0,0xffffe1ac)
#define ATU_PDSCR	REG32(0,0xffffe1bc)
#define ATU_PMCAPID	REG8(0,0xffffe1c0)
#define ATU_PMNEXT	REG8(0,0xffffe1c1)
#define ATU_APMCR	REG16(0,0xffffe1c2)
#define ATU_APMCSR	REG16(0,0xffffe1c4)
#define ATU_PCIXCAPID	REG8(0,0xffffe1e0)
#define ATU_PCIXNEXT	REG8(0,0xffffe1e1)
#define ATU_PCIXCMD	REG16(0,0xffffe1e2)
#define ATU_PCIXSR	REG32(0,0xffffe1e4)

#define PCSR_RESET_I_BUS 0x20
#define PCSR_RESET_P_BUS 0x10
#define PCSR_CFG_RETRY   0x04


// --------------------------------------------------------------------------
// Application Accelerator Unit  (Chapter 6)
#define AAU_ACR     REG32(0,0xffffe800)
#define AAU_ASR     REG32(0,0xffffe804)
#define AAU_ADAR    REG32(0,0xffffe808)
#define AAU_ANDAR   REG32(0,0xffffe80c)
#define AAU_SAR1    REG32(0,0xffffe810)
#define AAU_SAR2    REG32(0,0xffffe814)
#define AAU_SAR3    REG32(0,0xffffe818)
#define AAU_SAR4    REG32(0,0xffffe81c)
#define AAU_DAR     REG32(0,0xffffe820)
#define AAU_ABCR    REG32(0,0xffffe824)
#define AAU_ADCR    REG32(0,0xffffe828)
#define AAU_SAR5    REG32(0,0xffffe82c)
#define AAU_SAR6    REG32(0,0xffffe830)
#define AAU_SAR7    REG32(0,0xffffe834)
#define AAU_SAR8    REG32(0,0xffffe838)
#define AAU_EDCR0   REG32(0,0xffffe83c)
#define AAU_SAR9    REG32(0,0xffffe840)
#define AAU_SAR10   REG32(0,0xffffe844)
#define AAU_SAR11   REG32(0,0xffffe848)
#define AAU_SAR12   REG32(0,0xffffe84c)
#define AAU_SAR13   REG32(0,0xffffe850)
#define AAU_SAR14   REG32(0,0xffffe854)
#define AAU_SAR15   REG32(0,0xffffe858)
#define AAU_SAR16   REG32(0,0xffffe85c)
#define AAU_EDCR1   REG32(0,0xffffe860)
#define AAU_SAR17   REG32(0,0xffffe864)
#define AAU_SAR18   REG32(0,0xffffe868)
#define AAU_SAR19   REG32(0,0xffffe86c)
#define AAU_SAR20   REG32(0,0xffffe870)
#define AAU_SAR21   REG32(0,0xffffe874)
#define AAU_SAR22   REG32(0,0xffffe878)
#define AAU_SAR23   REG32(0,0xffffe87c)
#define AAU_SAR24   REG32(0,0xffffe880)
#define AAU_EDCR2   REG32(0,0xffffe884)
#define AAU_SAR25   REG32(0,0xffffe888)
#define AAU_SAR26   REG32(0,0xffffe88c)
#define AAU_SAR27   REG32(0,0xffffe890)
#define AAU_SAR28   REG32(0,0xffffe894)
#define AAU_SAR29   REG32(0,0xffffe898)
#define AAU_SAR30   REG32(0,0xffffe89c)
#define AAU_SAR31   REG32(0,0xffffe8a0)
#define AAU_SAR32   REG32(0,0xffffe8a4)
#define AAU_RES0    REG32(0,0xffffe8a8)
#define AAU_RES1    REG32(0,0xffffe900)
#define AAU_RES2    REG32(0,0xfffff000)

#define ACR_ENABLE   1
#define ACR_RESUME   2

#define ASR_ACTIVE   0x400


// --------------------------------------------------------------------------
// Memory Controller  (Chapter 7)
#define MCU_SDIR	REG32(0,0xffffe500)
#define MCU_SDCR	REG32(0,0xffffe504)
#define MCU_SDBR	REG32(0,0xffffe508)
#define MCU_SBR0	REG32(0,0xffffe50c)
#define MCU_SBR1	REG32(0,0xffffe510)
#define MCU_ECCR	REG32(0,0xffffe534)
#define MCU_ELOG0	REG32(0,0xffffe538)
#define MCU_ELOG1	REG32(0,0xffffe53c)
#define MCU_ECAR0	REG32(0,0xffffe540)
#define MCU_ECAR1	REG32(0,0xffffe544)
#define MCU_ECTST	REG32(0,0xffffe548)
#define MCU_MCISR	REG32(0,0xffffe54c)
#define MCU_RFR         REG32(0,0xffffe550)
#define MCU_DBUDSR      REG32(0,0xffffe554)
#define MCU_DBDDSR      REG32(0,0xffffe558)
#define MCU_CUDSR       REG32(0,0xffffe55c)
#define MCU_CDDSR       REG32(0,0xffffe560)
#define MCU_CEUDSR      REG32(0,0xffffe564)
#define MCU_CEDDSR      REG32(0,0xffffe568)
#define MCU_CSUDSR      REG32(0,0xffffe56c)
#define MCU_CSDDSR      REG32(0,0xffffe570)
#define MCU_REUDSR      REG32(0,0xffffe574)
#define MCU_REDDSR      REG32(0,0xffffe578)
#define MCU_ABUDSR      REG32(0,0xffffe57c)
#define MCU_ABDDSR      REG32(0,0xffffe580)
#define MCU_DSDR        REG32(0,0xffffe584)
#define MCU_REDR        REG32(0,0xffffe588)
#define MCU_RES10       REG32(0,0xffffe58c)

// Banksize specific component of SBRx register bits
#define		SBR_32MEG	1
#define		SBR_64MEG	2
#define		SBR_128MEG	4
#define		SBR_256MEG	8
#define		SBR_512MEG     16

// Refresh rates for 200MHz
#define         RFR_3_9us      0x300
#define         RFR_7_8us      0x600
#define         RFR_15_6us     0xC00

#define DSDR_REC_VAL    0x00000231
#define REDR_REC_VAL    0x00000000
#define SDCR_INIT_VAL   0x00000018  // 64-bit - unbuffered DIMM & turn off compensations (SECRET BITS!!!)

// SDRAM MODE COMMANDS
#define SDIR_CMD_NOP            0x00000005
#define SDIR_CMD_PRECHARGE_ALL  0x00000004
#define SDIR_CMD_ENABLE_DLL     0x00000006
#define SDIR_CMD_CAS_LAT_2_A    0x00000002
#define SDIR_CMD_CAS_LAT_2_B    0x00000000
#define SDIR_CMD_AUTO_REFRESH   0x00000007


// --------------------------------------------------------------------------
// Peripheral Bus Interface Unit  (Chapter 8)
#define PBIU_PBCR	REG32(0,0xffffe680)
#define PBIU_PBSR	REG32(0,0xffffe684)
#define PBIU_PBAR0	REG32(0,0xffffe688)
#define PBIU_PBLR0	REG32(0,0xffffe68c)
#define PBIU_PBAR1	REG32(0,0xffffe690)
#define PBIU_PBLR1	REG32(0,0xffffe694)
#define PBIU_PBAR2	REG32(0,0xffffe698)
#define PBIU_PBLR2	REG32(0,0xffffe69c)
#define PBIU_PBAR3	REG32(0,0xffffe6a0)
#define PBIU_PBLR3	REG32(0,0xffffe6a4)
#define PBIU_PBAR4	REG32(0,0xffffe6a8)
#define PBIU_PBLR4	REG32(0,0xffffe6ac)
#define PBIU_PBAR5	REG32(0,0xffffe6b0)
#define PBIU_PBLR5	REG32(0,0xffffe6b4)
#define PBIU_PBVR0	REG32(0,0xffffe6c0)
#define PBIU_PBVR1	REG32(0,0xffffe6c4)
#define PBIU_PBVR2	REG32(0,0xffffe6c8)
#define PBIU_PBVR3	REG32(0,0xffffe6cc)
#define PBIU_PBVR4	REG32(0,0xffffe6d0)
#define PBIU_PBVR5	REG32(0,0xffffe6d8)
#define PBIU_PBVR6	REG32(0,0xffffe6dc)

#define PBCR_ENABLE     1

#define PBCR_ERR_VALID    0x01
#define PBCR_ERR_WRITE    0x02
#define PBCR_ERR_TYPEMASK 0x0C

#define PBAR_FLASH      0x200
#define PBAR_RCWAIT_1   0x000
#define PBAR_RCWAIT_4   0x040
#define PBAR_RCWAIT_8   0x080
#define PBAR_RCWAIT_12  0x0C0
#define PBAR_RCWAIT_16  0x100
#define PBAR_RCWAIT_20  0x1C0
#define PBAR_ADWAIT_4   0x000
#define PBAR_ADWAIT_8   0x004
#define PBAR_ADWAIT_12  0x008
#define PBAR_ADWAIT_16  0x00C
#define PBAR_ADWAIT_20  0x01C
#define PBAR_BUS_8      0x000
#define PBAR_BUS_16     0x001
#define PBAR_BUS_32     0x002

#define PBLR_SZ_4K      0xfffff000
#define PBLR_SZ_8K      0xffffe000
#define PBLR_SZ_16K     0xffffc000
#define PBLR_SZ_32K     0xffff8000
#define PBLR_SZ_64K     0xffff0000
#define PBLR_SZ_128K    0xfffe0000
#define PBLR_SZ_256K    0xfffc0000
#define PBLR_SZ_512K    0xfff80000
#define PBLR_SZ_1M      0xfff00000
#define PBLR_SZ_2M      0xffe00000
#define PBLR_SZ_4M      0xffc00000
#define PBLR_SZ_8M      0xff800000
#define PBLR_SZ_16M     0xff000000
#define PBLR_SZ_32M     0xfe000000
#define PBLR_SZ_64M     0xfc000000
#define PBLR_SZ_128M    0xf8000000
#define PBLR_SZ_256M    0xf0000000
#define PBLR_SZ_512M    0xe0000000
#define PBLR_SZ_1G      0xc0000000
#define PBLR_SZ_2G      0x80000000
#define PBLR_SZ_DISABLE 0x00000000

// --------------------------------------------------------------------------
// I2C (Chapter 9)
#define I2C_BASE0       0xfffff680
#define I2C_BASE1       0xfffff6A0

#define I2C_ICR	        0x00
#define I2C_ISR	        0x04
#define I2C_ISAR	0x08
#define I2C_IDBR	0x0c
#define I2C_IBMR	0x14

#define I2C_ICR0	REG32(I2C_BASE0,I2C_ICR)
#define I2C_ICR1	REG32(I2C_BASE1,I2C_ICR)
#define I2C_ISR0	REG32(I2C_BASE0,I2C_ISR)
#define I2C_ISR1	REG32(I2C_BASE1,I2C_ISR)
#define I2C_ISAR0	REG32(I2C_BASE0,I2C_ISAR)
#define I2C_ISAR1	REG32(I2C_BASE1,I2C_ISAR)
#define I2C_IDBR0	REG32(I2C_BASE0,I2C_IDBR)
#define I2C_IDBR1	REG32(I2C_BASE1,I2C_IDBR)
#define I2C_IBMR0	REG32(I2C_BASE0,I2C_IBMR)
#define I2C_IBMR1	REG32(I2C_BASE1,I2C_IBMR)

// Control Register bits
#define	ICR_START	0x0001  /* 1:send a Start condition to the I2C when in master mode */
#define	ICR_STOP	0x0002  /* 1:send a Stop condition after next byte transferred in master mode */
#define	ICR_ACK		0x0004  /* Ack/Nack control: 1:Nack, 0:Ack (negative or positive pulse) */
#define	ICR_TRANSFER	0x0008  /* 1:send/receive byte, 0:cleared by I2C unit when done */
#define	ICR_ABORT	0x0010  /* 1:I2C sends STOP w/out data permission, 0:ICR bit used only */
#define	ICR_SCLENB	0x0020  /* I2C clock output: 1:Enabled, 0:Disabled. ICCR configured before ! */
#define	ICR_ENB		0x0040  /* I2C unit: 1:Enabled, 0:Disabled */
#define	ICR_GCALL	0x0080  /* General Call: 1:Disabled, 0:Enabled */
#define	ICR_IEMPTY	0x0100  /* 1: IDBR Transmit Empty Interrupt Enable */
#define	ICR_IFULL	0x0200  /* 1: IDBR Receive Full Interrupt Enable */
#define	ICR_IERR	0x0400  /* 1: Bus Error Interrupt Enable */
#define	ICR_ISTOP	0x0800  /* 1: Slave Stop Detected Interrupt Enable */
#define	ICR_IARB	0x1000  /* 1: Arbitration Loss Detected Interrupt Enable */
#define	ICR_ISADDR	0x2000  /* 1: Slave Address Detected Interrupt Enable */
#define	ICR_RESET	0x4000  /* 1: I2C unit reset */

// Status Register bits
#define	ISR_RWMODE	0x0001  /* 1: I2C in master receive = slave transmit mode */
#define	ISR_ACK		0x0002  /* 1: I2C received/sent a Nack, 0: Ack */
#define	ISR_BUSY	0x0004  /* 1: Processor's I2C unit busy */
#define	ISR_BUSBUSY	0x0008  /* 1: I2C bus busy. Processor's I2C unit not involved */
#define	ISR_STOP	0x0010  /* 1: Slave Stop detected (when in slave mode: receive or transmit) */
#define	ISR_ARB		0x0020  /* 1: Arbitration Loss Detected */
#define	ISR_EMPTY	0x0040  /* 1: Transfer finished on I2C bus. If enabled in ICR, interrupt signaled */
#define	ISR_FULL	0x0080  /* 1: IDBR received new byte from I2C bus. If ICR, interrupt signaled */
#define	ISR_GCALL	0x0100  /* 1: I2C unit received a General Call address */
#define	ISR_SADDR	0x0200  /* 1: I2C unit detected a 7-bit address matching the general call or ISAR */
#define	ISR_ERROR	0x0400  /* Bit set by unit when a Bus Error detected */

#define	IDBR_MASK	0x000000ff
#define	IDBR_MODE	0x01

// --------------------------------------------------------------------------
// Arbitration (Chapter 11)
#define ARB_IACR        REG32(0,0xffffe780)
#define ARB_MTTR1       REG32(0,0xffffe784)
#define ARB_MTTR2       REG32(0,0xffffe788)

#define IACR_PRI_HIGH    0
#define IACR_PRI_MED     1
#define IACR_PRI_LOW     2
#define IACR_PRI_OFF     3

// macros to set priority for various units
#define IACR_ATU(x)      ((x) << 0)
#define IACR_DMA0(x)     ((x) << 4)
#define IACR_DMA1(x)     ((x) << 6)
#define IACR_CORE(x)     ((x) << 10)
#define IACR_AAU(x)      ((x) << 12)
#define IACR_PBI(x)      ((x) << 14)


// --------------------------------------------------------------------------
// Timers (Chapter 14)
#define TU_TMR0	    REG32(0,0xffffe7e0)
#define TU_TMR1	    REG32(0,0xffffe7e4)
#define TU_TCR0	    REG32(0,0xffffe7e8)
#define TU_TCR1	    REG32(0,0xffffe7ec)
#define TU_TRR0	    REG32(0,0xffffe7f0)
#define TU_TRR1	    REG32(0,0xffffe7f4)
#define TU_TISR	    REG32(0,0xffffe7f8)
#define TU_WDTCR    REG32(0,0xffffe7fc)

#define TMR_TC      0x01  // terminal count
#define TMR_ENABLE  0x02  // timer enable
#define TMR_RELOAD  0x04  // auto reload enable
#define TMR_CLK_1   0x00  // CCLK (core clock)
#define TMR_CLK_4   0x10  // CCLK/4
#define TMR_CLK_8   0x20  // CCLK/8
#define TMR_CLK_16  0x30  // CCLK/16

#ifndef __ASSEMBLER__
// For full read/write access, you have to use coprocessor insns.
#define TMR0_READ(val)    asm volatile ("mrc p6, 0, %0, c0, c1, 0" : "=r" (val))
#define _TMR0_WRITE(val)  asm volatile ("mcr p6, 0, %0, c0, c1, 0" : : "r" (val))
#define TMR1_READ(val)    asm volatile ("mrc p6, 0, %0, c1, c1, 0" : "=r" (val))
#define _TMR1_WRITE(val)  asm volatile ("mcr p6, 0, %0, c1, c1, 0" : : "r" (val))
#define TCR0_READ(val)    asm volatile ("mrc p6, 0, %0, c2, c1, 0" : "=r" (val))
#define _TCR0_WRITE(val)  asm volatile ("mcr p6, 0, %0, c2, c1, 0" : : "r" (val))
#define TCR1_READ(val)    asm volatile ("mrc p6, 0, %0, c3, c1, 0" : "=r" (val))
#define _TCR1_WRITE(val)  asm volatile ("mcr p6, 0, %0, c3, c1, 0" : : "r" (val))
#define TRR0_READ(val)    asm volatile ("mrc p6, 0, %0, c4, c1, 0" : "=r" (val))
#define _TRR0_WRITE(val)  asm volatile ("mcr p6, 0, %0, c4, c1, 0" : : "r" (val))
#define TRR1_READ(val)    asm volatile ("mrc p6, 0, %0, c5, c1, 0" : "=r" (val))
#define _TRR1_WRITE(val)  asm volatile ("mcr p6, 0, %0, c5, c1, 0" : : "r" (val))
#define TISR_READ(val)    asm volatile ("mrc p6, 0, %0, c6, c1, 0" : "=r" (val))
#define _TISR_WRITE(val)  asm volatile ("mcr p6, 0, %0, c6, c1, 0" : : "r" (val))
#define _WDTCR_READ(val)  asm volatile ("mrc p6, 0, %0, c7, c1, 0" : "=r" (val))
#define _WDTCR_WRITE(val) asm volatile ("mcr p6, 0, %0, c7, c1, 0" : : "r" (val))

static inline void TMR0_WRITE(cyg_uint32 val) { _TMR0_WRITE(val); }
static inline void TMR1_WRITE(cyg_uint32 val) { _TMR1_WRITE(val); }
static inline void TCR0_WRITE(cyg_uint32 val) { _TCR0_WRITE(val); }
static inline void TCR1_WRITE(cyg_uint32 val) { _TCR1_WRITE(val); }
static inline void TRR0_WRITE(cyg_uint32 val) { _TRR0_WRITE(val); }
static inline void TRR1_WRITE(cyg_uint32 val) { _TRR1_WRITE(val); }
static inline void TISR_WRITE(cyg_uint32 val) { _TISR_WRITE(val); }
#endif

// --------------------------------------------------------------------------
// Interrupts (Chapter 15)
#define INTCTL	REG32(0,0xffffe7d0)
#define INTSTR	REG32(0,0xffffe7d4)
#define IINTSRC	REG32(0,0xffffe7d8)
#define FINTSRC	REG32(0,0xffffe7dc)
#define PIRSR	REG32(0,0xffffe1ec)

#ifndef __ASSEMBLER__
#define INTCTL_READ(val)   asm volatile ("mrc p6, 0, %0, c0, c0, 0" : "=r" (val))
#define _INTCTL_WRITE(val) asm volatile ("mcr p6, 0, %0, c0, c0, 0" : : "r" (val))
#define INTSTR_READ(val)   asm volatile ("mrc p6, 0, %0, c4, c0, 0" : "=r" (val))
#define _INTSTR_WRITE(val) asm volatile ("mcr p6, 0, %0, c4, c0, 0" : : "r" (val))
#define IINTSRC_READ(val)  asm volatile ("mrc p6, 0, %0, c8, c0, 0" : "=r" (val))
#define FINTSRC_READ(val)  asm volatile ("mrc p6, 0, %0, c9, c0, 0" : "=r" (val))

static inline void INTCTL_WRITE(cyg_uint32 val) { _INTCTL_WRITE(val); }
static inline void INTSTR_WRITE(cyg_uint32 val) { _INTSTR_WRITE(val); }
#endif

// --------------------------------------------------------------------------
// GPIO (Chapter 15)
#define GPIO_GPOE	REG8(0,0xffffe7c4)
#define GPIO_GPID	REG8(0,0xffffe7c8)
#define GPIO_GPOD	REG8(0,0xffffe7cc)

#endif // CYGONCE_HAL_ARM_XSCALE_HAL_VERDE_H
// EOF hal_verde.h
