#ifndef CYGONCE_HAL_VAR_REGS_H
#define CYGONCE_HAL_VAR_REGS_H

//==========================================================================
//
//      var_regs.h
//
//      PowerPC 5xx variant CPU definitions
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
// Author(s):    Bob Koninckx
// Contributors: Bob Koninckx
// Date:         2001-09-12
// Purpose:      Provide MPC5xx register definitions
// Description:  Provide MPC5xx register definitions
//               The short difinitions (sans CYGARC_REG_) are exported only
//               if CYGARC_HAL_COMMON_EXPORT_CPU_MACROS is defined.
// Usage:        Included via the acrhitecture register header:
//               #include <cyg/hal/ppc_regs.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

//--------------------------------------------------------------------------
// Special purpose registers
// 
#define CYGARC_REG_XER          1
#define CYGARC_REG_LR           8
#define CYGARC_REG_CTR          9
#define CYGARC_REG_EIE          80
#define CYGARC_REG_EID          81
#define CYGARC_REG_NRI          82
#define CYGARC_REG_CMPA         144
#define CYGARC_REG_CMPB         145
#define CYGARC_REG_CMPC         146
#define CYGARC_REG_CMPD         147
#define CYGARC_REG_ECR          148
#define CYGARC_REG_DER          149
#define CYGARC_REG_COUNTA       150
#define CYGARC_REG_COUNTB       151
#define CYGARC_REG_CMPE         152
#define CYGARC_REG_CMPF         153
#define CYGARC_REG_CMPG         154
#define CYGARC_REG_CMPH         155
#define CYGARC_REG_LCTRL1       156
#define CYGARC_REG_LCTRL2       157
#define CYGARC_REG_BAR          159
#define CYGARC_REG_MI_GRA       528
#define CYGARC_REG_L2U_GRA      536
#define CYGARC_REG_BBCMCR       560
#define CYGARC_REG_L2U_MCR      568
#define CYGARC_REG_DPDR         630
#define CYGARC_REG_IMMR         638
#define CYGARC_REG_MI_RBA0      784
#define CYGARC_REG_MI_RBA1      785
#define CYGARC_REG_MI_RBA2      786
#define CYGARC_REG_MI_RBA3      787
#define CYGARC_REG_L2U_RBA0     792
#define CYGARC_REG_L2U_RBA1     793
#define CYGARC_REG_L2U_RBA2     794
#define CYGARC_REG_L2U_RBA3     795
#define CYGARC_REG_MI_RA0       816
#define CYGARC_REG_MI_RA1       817
#define CYGARC_REG_MI_RA2       818
#define CYGARC_REG_MI_RA3       819
#define CYGARC_REG_L2U_RA0      824
#define CYGARC_REG_L2U_RA1      825
#define CYGARC_REG_L2U_RA2      826
#define CYGARC_REG_L2U_RA3      827
#define CYGARC_REG_FPECR        1022

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
//# define XER                CYGARC_REG_XER // Leave these out, they conflict with GDB stubs
//# define LR                 CYGARC_REG_LR  // are not used anyway
# define CTR                CYGARC_REG_CTR
# define EIE                CYGARC_REG_EIE
# define EID                CYGARC_REG_EID  
# define NRI                CYGARC_REG_NRI
# define CMPA               CYGARC_REG_CMPA
# define CMPB               CYGARC_REG_CMPB
# define CMPC               CYGARC_REG_CMPC
# define CMPD               CYGARC_REG_CMPD
# define ECR                CYGARC_REG_ECR
# define DER                CYGARC_REG_DER
# define COUNTA             CYGARC_REG_COUNTA
# define COUNTB             CYGARC_REG_COUNTB
# define CMPE               CYGARC_REG_CMPE
# define CMPF               CYGARC_REG_CMPF
# define CMPG               CYGARC_REG_CMPG
# define CMPH               CYGARC_REG_CMPH
# define LCTRL1             CYGARC_REG_LCTRL1
# define LCTRL2             CYGARC_REG_LCTRL2
# define BAR                CYGARC_REG_BAR
# define MI_GRA             CYGARC_REG_MI_GRA
# define L2U_GRA            CYGARC_REG_L2U_GRA
# define BBCMCR             CYGARC_REG_BBCMCR
# define L2U_MCR            CYGARC_REG_L2U_MCR
# define DPDR               CYGARC_REG_DPDR
# define IMMR               CYGARC_REG_IMMR
# define MI_RBA0            CYGARC_REG_MI_RBA0
# define MI_RBA1            CYGARC_REG_MI_RBA1
# define MI_RBA2            CYGARC_REG_MI_RBA2
# define MI_RBA3            CYGARC_REG_MI_RBA3
# define L2U_RBA0           CYGARC_REG_L2U_RBA0
# define L2U_RBA1           CYGARC_REG_L2U_RBA1
# define L2U_RBA2           CYGARC_REG_L2U_RBA2
# define L2U_RBA3           CYGARC_REG_L2U_RBA3
# define MI_RA0             CYGARC_REG_MI_RA0
# define MI_RA1             CYGARC_REG_MI_RA1
# define MI_RA2             CYGARC_REG_MI_RA2
# define MI_RA3             CYGARC_REG_MI_RA3
# define L2U_RA0            CYGARC_REG_L2U_RA0
# define L2U_RA1            CYGARC_REG_L2U_RA1
# define L2U_RA2            CYGARC_REG_L2U_RA2
# define L2U_RA3            CYGARC_REG_L2U_RA3
# define FPECR              CYGARC_REG_FPECR
#endif //#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//-----------------------------------------------------------------------------
// Development Support.
#define CYGARC_REG_DER             149

#define CYGARC_REG_ICTRL           158  // instruction support control reg
#define CYGARC_REG_ICTRL_SERSHOW   0x00000000 // serialized, show cycles
#define CYGARC_REG_ICTRL_NOSERSHOW 0x00000007 //non-serialized&no show cycles

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define DER             CYGARC_REG_DER

#define ICTRL           CYGARC_REG_ICTRL
#define ICTRL_SERSHOW   CYGARC_REG_ICTRL_SERSHOW
#define ICTRL_NOSERSHOW CYGARC_REG_ICTRL_NOSERSHOW
#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//-----------------------------------------------------------------------------
// Bit definitions
//-----------------------------------------------------------------------------
// The internal memory map register (IMMR)
#define CYGARC_REG_IMM_IMMR_PARTNUM    0xff000000  // part number mask (ro)
#define CYGARC_REG_IMM_IMMR_MASKNUM    0x00ff0000  // mask number mask (ro)
#define CYGARC_REG_IMM_IMMR_ISB        0x0000000e  // internal space base
#define CYGARC_REG_IMM_IMMR_CLES       0x00000100  // core little endian swap
#define CYGARC_REG_IMM_IMMR_FLEN       0x00000800  // flash enable

//-----------------------------------------------------------------------------
// System protection control (SYPCR)
#define CYGARC_REG_IMM_SYPR_SWTC_MASK  0xffff0000
#define CYGARC_REG_IMM_SYPR_BMT_MASK   0x0000ff00
#define CYGARC_REG_IMM_SYPR_BME        0x00000080
#define CYGARC_REG_IMM_SYPR_SWF        0x00000008
#define CYGARC_REG_IMM_SYPR_SWE        0x00000004
#define CYGARC_REG_IMM_SYPR_SWRI       0x00000002 
#define CYGARC_REG_IMM_SYPR_SWP        0x00000001

//-----------------------------------------------------------------------------
// Interrupt pend register (SIPEND)
#define CYGARC_REG_IMM_SIPEND_IRQ0     0x80000000  // External interrupt priority 0
#define CYGARC_REG_IMM_SIPEND_LVL0     0x40000000  // Internal interrupt level 0
#define CYGARC_REG_IMM_SIPEND_IRQ1     0x20000000  // External interrupt priority 1
#define CYGARC_REG_IMM_SIPEND_LVL1     0x10000000  // Internal interrupt level 1
#define CYGARC_REG_IMM_SIPEND_IRQ2     0x08000000  // External interrupt priority 2
#define CYGARC_REG_IMM_SIPEND_LVL2     0x04000000  // Internal interrupt level 2
#define CYGARC_REG_IMM_SIPEND_IRQ3     0x02000000  // External interrupt priority 3
#define CYGARC_REG_IMM_SIPEND_LVL3     0x01000000  // Internal interrupt level 3
#define CYGARC_REG_IMM_SIPEND_IRQ4     0x00800000  // External interrupt prioeity 4
#define CYGARC_REG_IMM_SIPEND_LVL4     0x00400000  // Internal interrupt level 4
#define CYGARC_REG_IMM_SIPEND_IRQ5     0x00200000  // External interrupt priority 5
#define CYGARC_REG_IMM_SIPEND_LVL5     0x00100000  // Internal interrupt level 5
#define CYGARC_REG_IMM_SIPEND_IRQ6     0x00080000  // External interrupt priority 6
#define CYGARC_REG_IMM_SIPEND_LVL6     0x00040000  // Internal interrupt level 6
#define CYGARC_REG_IMM_SIPEND_IRQ7     0x00020000  // External interrupt priority 7
#define CYGARC_REG_IMM_SIPEND_LVL7     0x00010000  // Internal interrupt level 7

//-----------------------------------------------------------------------------
// Interrupt mask register (SIMASK)
#define CYGARC_REG_IMM_SIMASK_IRM0     0x80000000  // External interrupt priority 0
#define CYGARC_REG_IMM_SIMASK_LVM0     0x40000000  // Internal interrupt level 0
#define CYGARC_REG_IMM_SIMASK_IRM1     0x20000000  // External interrupt priority 1
#define CYGARC_REG_IMM_SIMASK_LVM1     0x10000000  // Internal interrupt level 1
#define CYGARC_REG_IMM_SIMASK_IRM2     0x08000000  // External interrupt priority 2
#define CYGARC_REG_IMM_SIMASK_LVM2     0x04000000  // Internal interrupt level 2
#define CYGARC_REG_IMM_SIMASK_IRM3     0x02000000  // External interrupt priority 3
#define CYGARC_REG_IMM_SIMASK_LVM3     0x01000000  // Internal interrupt level 3
#define CYGARC_REG_IMM_SIMASK_IRM4     0x00800000  // External interrupt priority 4
#define CYGARC_REG_IMM_SIMASK_LVM4     0x00400000  // Internal interrupt level 4
#define CYGARC_REG_IMM_SIMASK_IRM5     0x00200000  // External interrupt priority 5
#define CYGARC_REG_IMM_SIMASK_LVM5     0x00100000  // Internal interrupt level 5
#define CYGARC_REG_IMM_SIMASK_IRM6     0x00080000  // External interrupt priority 6
#define CYGARC_REG_IMM_SIMASK_LVM6     0x00040000  // Internal interrupt level 6
#define CYGARC_REG_IMM_SIMASK_IRM7     0x00020000  // External interrupt priority 7
#define CYGARC_REG_IMM_SIMASK_LVM7     0x00010000  // Internal interrupt level 7

//-----------------------------------------------------------------------------
// Interrupt edge level register (CIEL)
#define CYGARC_REG_IMM_SIEL_ED0        0x80000000  // Falling edge external interrupt priority 0
#define CYGARC_REG_IMM_SIEL_WM0        0x40000000  // Wake-up mask external interrupt priority 0
#define CYGARC_REG_IMM_SIEL_ED1        0x20000000  // Falling edge external interrupt priority 1
#define CYGARC_REG_IMM_SIEL_WM1        0x10000000  // Wake-up mask external interrupt priority 1
#define CYGARC_REG_IMM_SIEL_ED2        0x08000000  // Falling edge external interrupt priority 2
#define CYGARC_REG_IMM_SIEL_WM2        0x04000000  // Wake-up mask external interrupt priority 2
#define CYGARC_REG_IMM_SIEL_ED3        0x02000000  // Falling edge external interrupt priority 3
#define CYGARC_REG_IMM_SIEL_WM3        0x01000000  // Wake-up mask external interrupt priority 3
#define CYGARC_REG_IMM_SIEL_ED4        0x00800000  // Falling edge external interrupt prioriry 4
#define CYGARC_REG_IMM_SIEL_WM4        0x00400000  // Wake-up mask external interrupt priority 4
#define CYGARC_REG_IMM_SIEL_ED5        0x00200000  // Falling edge external interrupt priority 5
#define CYGARC_REG_IMM_SIEL_WM5        0x00100000  // Wake-up mask external interrupt priority 5
#define CYGARC_REG_IMM_SIEL_ED6        0x00080000  // Falling edge external interrupt priority 6
#define CYGARC_REG_IMM_SIEL_WM6        0x00040000  // Wake-up mask external interrupt priority 6
#define CYGARC_REG_IMM_SIEL_ED7        0x00020000  // Falling edge external interrupt priority 7
#define CYGARC_REG_IMM_SIEL_WM7        0x00010000  // Wake-up mask external interrupt priority 7

//-----------------------------------------------------------------------------
// Memory controller
#define CYGARC_REG_IMM_BR_BA_MASK      0xffff8000       // base address
#define CYGARC_REG_IMM_BR_AT_MASK      0x00007000       // address type
#define CYGARC_REG_IMM_BR_PS_8         0x00000400       // port size 8 bits
#define CYGARC_REG_IMM_BR_PS_16        0x00000800       // port size 16 bits
#define CYGARC_REG_IMM_BR_PS_32        0x00000000       // port size 32 bits
#define CYGARC_REG_IMM_BR_WP           0x00000100       // write protect
#define CYGARC_REG_IMM_BR_WEBS         0x00000020       // write enable byte select
#define CYGARC_REG_IMM_BR_TBDIP        0x00000010       // toggle burst data in progress
#define CYGARC_REG_IMM_BR_LBDIP        0x00000008       // late burst data in progress
#define CYGARC_REG_IMM_BR_SETA         0x00000004       // externam transfer acknowledge
#define CYGARC_REG_IMM_BR_BI           0x00000002       // burst inhibit
#define CYGARC_REG_IMM_BR_V            0x00000001       // valid bit

#define CYGARC_REG_IMM_OR_AM           0xffff8000       // address mask   
#define CYGARC_REG_IMM_OR_ATM          0x00007000       // address type mask
#define CYGARC_REG_IMM_OR_CSNT         0x00000800       // GPCM : chip select negatoion time
#define CYGARC_REG_IMM_OR_ACS_0        0x00000000       // GPCM : CS output immediately
#define CYGARC_REG_IMM_OR_ACS_4        0x00000400       // GPCM : CS output 1/4 clock cycle later
#define CYGARC_REG_IMM_OR_ACS_2        0x00000600       // GPCM : CS output 1/2 clock cycle later
#define CYGARC_REG_IMM_OR_SCY_MASK     0x000000f0       // cycle length in clocks
#define CYGARC_REG_IMM_OR_BSCY         0x0000000E       // burst cycle length in clocks
#define CYGARC_REG_IMM_OR_TRLX         0x00000001       // timing relaxed
#define CYGARC_REG_IMM_OR_EHTR         0x00000100       // extended hold time on read
#define CYGARC_REG_IMM_OR_SCY_SHIFT    4            

//-----------------------------------------------------------------------------
// Time base status and control (TBSCR)
#define CYGARC_REG_IMM_TBSCR_REFA      0x0080           // reference interrupt status A
#define CYGARC_REG_IMM_TBSCR_REFB      0x0040           // reference interrupt status B
#define CYGARC_REG_IMM_TBSCR_REFAE     0x0008           // reference interrupt enable A
#define CYGARC_REG_IMM_TBSCR_REFBE     0x0004           // reference interrupt enable B
#define CYGARC_REG_IMM_TBSCR_TBF       0x0002           // timebase freeze
#define CYGARC_REG_IMM_TBSCR_TBE       0x0001           // timebase enable
#define CYGARC_REG_IMM_TBSCR_IRQ0      0x8000           // highest interrupt level
#define CYGARC_REG_IMM_TBSCR_IRQMASK   0xff00           // irq priority mask

//-----------------------------------------------------------------------------
// Real time clock
#define CYGARC_REG_IMM_RTCSC_SEC       0x0080           // once per second interrupt
#define CYGARC_REG_IMM_RTCSC_ALR       0x0040           // alarm interrupt
#define CYGARC_REG_IMM_RTCSC_4M        0x0010           // source select
#define CYGARC_REG_IMM_RTCSC_SIE       0x0008           // second interrupt enable
#define CYGARC_REG_IMM_RTCSC_ALE       0x0004           // alarm interrupt enable
#define CYGARC_REG_IMM_RTCSC_RTF       0x0002           // real time clock freeze
#define CYGARC_REG_IMM_RTCSC_RTE       0x0001           // real time clock enable
#define CYGARC_REG_IMM_RTCSC_IRQ0      0x8000           // highest interrupt priority
#define CYGARC_REG_IMM_RTCSC_IRQMASK   0xff00           // irq priority mask

//-----------------------------------------------------------------------------
// Periodic interrupt status an control
#define CYGARC_REG_IMM_PISCR_PS        0x0080           // periodic interrupt status
#define CYGARC_REG_IMM_PISCR_PIE       0x0004           // periodic interrupt enable
#define CYGARC_REG_IMM_PISCR_PITF      0x0002           // periodic interrupt timer freeze
#define CYGARC_REG_IMM_PISCR_PTE       0x0001           // periodic timer enable
#define CYGARC_REG_IMM_PISCR_IRQ0      0x8000           // highest intetrupt priority
#define CYGARC_REG_IMM_PISCR_IRQMASK   0xff00           // irq priority mask

//-----------------------------------------------------------------------------
// Queued analog to digital convertor
#define CYGARC_REG_IMM_QUACR1_CIE1       0x8000
#define CYGARC_REG_IMM_QUACR1_PIE1       0x4000
#define CYGARC_REG_IMM_QUACR1_SSE1       0x2000
#define CYGARC_REG_IMM_QUACR1_MO1        0x1f00
#define CYGARC_REG_IMM_QUACR2_CIE2       0x8000
#define CYGARC_REG_IMM_QUACR2_PIE2       0x4000
#define CYGARC_REG_IMM_QUACR2_SSE2       0x2000
#define CYGARC_REG_IMM_QUACR2_MO2        0x1f00
#define CYGARC_REG_IMM_QUACR2_RESUME     0x0080
#define CYGARC_REG_IMM_QUACR2_BQ2        0x007f
#define CYGARC_REG_IMM_QUASR0_CF1        0x8000
#define CYGARC_REG_IMM_QUASR0_PF1        0x4000
#define CYGARC_REG_IMM_QUASR0_CF2        0x2000
#define CYGARC_REG_IMM_QUASR0_PF2        0x1000
#define CYGARC_REG_IMM_QUASR0_TOR1       0x0800
#define CYGARC_REG_IMM_QUASR0_TOR2       0x0400
#define CYGARC_REG_IMM_QUASR0_QS         0x03c0
#define CYGARC_REG_IMM_QUASR0_CWP        0x003f
#define CYGARC_REG_IMM_QUADC64INT_IRL1   0xf800
#define CYGARC_REG_IMM_QUADC64INT_IRL2   0x07c0
#define CYGARC_REG_IMM_QUADC64INT_IRL1_SHIFT 11
#define CYGARC_REG_IMM_QUADC64INT_IRL2_SHIFT  6

//-----------------------------------------------------------------------------
// PLL Change of lock
#define CYGARC_REG_IMM_COLIR_IRQ0      0x8000           // the highest interrupt priority
#define CYGARC_REG_IMM_COLIR_COLIRQ    0xff00           // interrupt priority mask
#define CYGARC_REG_IMM_COLIR_COLIS     0x0080           // change of lock detected
#define CYGARC_REG_IMM_COLIR_COLIE     0x0040           // change of lock interrupt enable

//-----------------------------------------------------------------------------
// SCI SPI registers
#define CYGARC_REG_IMM_SCCxR1_LOOPS     0x4000
#define CYGARC_REG_IMM_SCCxR1_WOMS      0x2000
#define CYGARC_REG_IMM_SCCxR1_ILT       0x1000
#define CYGARC_REG_IMM_SCCxR1_PT        0x0800
#define CYGARC_REG_IMM_SCCxR1_PE        0x0400
#define CYGARC_REG_IMM_SCCxR1_M         0x0200
#define CYGARC_REG_IMM_SCCxR1_WAKE      0x0100
#define CYGARC_REG_IMM_SCCxR1_TIE       0x0080
#define CYGARC_REG_IMM_SCCxR1_TCIE      0x0040
#define CYGARC_REG_IMM_SCCxR1_RIE       0x0020
#define CYGARC_REG_IMM_SCCxR1_ILIE      0x0010
#define CYGARC_REG_IMM_SCCxR1_TE        0x0008
#define CYGARC_REG_IMM_SCCxR1_RE        0x0004
#define CYGARC_REG_IMM_SCCxR1_RWU       0x0002
#define CYGARC_REG_IMM_SCCxR1_SBK       0x0001
#define CYGARC_REG_IMM_QSCI1CR_QTPNT    0xf000
#define CYGARC_REG_IMM_QSCI1CR_QTHFI    0x0800
#define CYGARC_REG_IMM_QSCI1CR_QBHFI    0x0400
#define CYGARC_REG_IMM_QSCI1CR_QTHEI    0x0200
#define CYGARC_REG_IMM_QSCI1CR_QBHEI    0x0100
#define CYGARC_REG_IMM_QSCI1CR_QTE      0x0040
#define CYGARC_REG_IMM_QSCI1CR_QRE      0x0020
#define CYGARC_REG_IMM_QSCI1CR_QTWE     0x0010
#define CYGARC_REG_IMM_QSCI1CR_QTSZ     0x000f
#define CYGARC_REG_IMM_SPCR1_SPE        0x8000
#define CYGARC_REG_IMM_SPCR1_DSCLK      0x7f00  
#define CYGARC_REG_IMM_SPCR1_DTL        0x00ff
#define CYGARC_REG_IMM_SPCR2_SPIFIE     0x8000
#define CYGARC_REG_IMM_SPCR2_WREN       0x4000
#define CYGARC_REG_IMM_SPCR2_WRT0       0x2000
#define CYGARC_REG_IMM_SPCR2_ENDQP      0x1f00
#define CYGARC_REG_IMM_SPCR2_NEWQP      0x001f
#define CYGARC_REG_IMM_SPCR3_LOOPQ      0x0400
#define CYGARC_REG_IMM_SPCR3_HMIE       0x0200
#define CYGARC_REG_IMM_SPCR3_HALT       0x0100
#define CYGARC_REG_IMM_SPCR3_SPSR       0x00ff
#define CYGARC_REG_IMM_SCxSR_TDRE       0x0100
#define CYGARC_REG_IMM_SCxSR_TC         0x0080
#define CYGARC_REG_IMM_SCxSR_RDRF       0x0040
#define CYGARC_REG_IMM_SCxSR_RAF        0x0020
#define CYGARC_REG_IMM_SCxSR_IDLE       0x0010
#define CYGARC_REG_IMM_SCxSR_OR         0x0008
#define CYGARC_REG_IMM_SCxSR_NF         0x0004
#define CYGARC_REG_IMM_SCxSR_FE         0x0002
#define CYGARC_REG_IMM_SCxSR_PF         0x0001
#define CYGARC_REG_IMM_QSCI1SR_QOR      0x1000
#define CYGARC_REG_IMM_QSCI1SR_QTHF     0x0800
#define CYGARC_REG_IMM_QSCI1SR_QBHF     0x0400
#define CYGARC_REG_IMM_QSCI1SR_QTHE     0x0200
#define CYGARC_REG_IMM_QSCI1SR_QBHE     0x0100
#define CYGARC_REG_IMM_QSCI1SR_QRPNT    0x00f0
#define CYGARC_REG_IMM_QSCI1SR_QRPEND   0x000f
#define CYGARC_REG_IMM_SPSR_SPCR3       0xff00
#define CYGARC_REG_IMM_SPSR_SPIF        0x0080
#define CYGARC_REG_IMM_SPSR_MODF        0x0040
#define CYGARC_REG_IMM_SPSR_HALTA       0x0020
#define CYGARC_REG_IMM_SPSR_CPTQ        0x001f
#define CYGARC_REG_IMM_QDSCI_IL_ILDSCI  0x1f00
#define CYGARC_REG_IMM_QDSCI_IL_ILDSCI_SHIFT 8
#define CYGARC_REG_IMM_QSPI_IL_ILQSPI   0x001f
#define CYGARC_REG_IMM_QSPI_IL_ILQSPI_SHIFT  0

//-----------------------------------------------------------------------------
// TPU register settings
#define CYGARC_REG_IMM_TICR_CIRL       0x0700
#define CYGARC_REG_IMM_TICR_CIRL_SHIFT      8
#define CYGARC_REG_IMM_TICR_ILBS       0x00c0
#define CYGARC_REG_IMM_TICR_ILBS_SHIFT      6

//-----------------------------------------------------------------------------
// TOUCAN registers
#define CYGARC_REG_IMM_CANCTRL0_BOFFMSK  0x8000
#define CYGARC_REG_IMM_CANCTRL0_ERRMSK   0x4000
#define CYGARC_REG_IMM_CANCTRL0_RXMOD    0x0c00
#define CYGARC_REG_IMM_CANCTRL0_TXMOD    0x0300
#define CYGARC_REG_IMM_CANCTRL0_CANCTLL1 0x00ff
#define CYGARC_REG_IMM_TCNMCR_STOP       0x8000
#define CYGARC_REG_IMM_TCNMCR_FRZ        0x4000
#define CYGARC_REG_IMM_TCNMCR_HALT       0x1000
#define CYGARC_REG_IMM_TCNMCR_NOTRDY     0x0800
#define CYGARC_REG_IMM_TCNMCR_WAKEMSK    0x0400
#define CYGARC_REG_IMM_TCNMCR_SOFTRST    0x0200
#define CYGARC_REG_IMM_TCNMCR_FRZACK     0x0100
#define CYGARC_REG_IMM_TCNMCR_SUPV       0x0080
#define CYGARC_REG_IMM_TCNMCR_SELFWAKE   0x0040
#define CYGARC_REG_IMM_TCNMCR_APS        0x0020
#define CYGARC_REG_IMM_TCNMCR_STOPACK    0x0010
#define CYGARC_REG_IMM_ESTAT_BOFFINT     0x0004
#define CYGARC_REG_IMM_ESTAT_ERRINT      0x0002
#define CYGARC_REG_IMM_ESTAT_WAKEINT     0x0001
#define CYGARC_REG_IMM_CANICR_IRL        0x0700
#define CYGARC_REG_IMM_CANICR_IRL_SHIFT       8
#define CYGARC_REG_IMM_CANICR_ILBS       0x00c0
#define CYGARC_REG_IMM_CANICR_ILBS_SHIFT      6

//-----------------------------------------------------------------------------
// MIOS registers
#define CYGARC_REG_IMM_MIOS1ER0_EN15     0x8000
#define CYGARC_REG_IMM_MIOS1ER0_EN14     0x4000
#define CYGARC_REG_IMM_MIOS1ER0_EN13     0x2000
#define CYGARC_REG_IMM_MIOS1ER0_EN12     0x1000
#define CYGARC_REG_IMM_MIOS1ER0_EN11     0x0800
#define CYGARC_REG_IMM_MIOS1ER0_EN6      0x0040
#define CYGARC_REG_IMM_MIOS1ER0_EN3      0x0008
#define CYGARC_REG_IMM_MIOS1ER0_EN2      0x0004
#define CYGARC_REG_IMM_MIOS1ER0_EN1      0x0002
#define CYGARC_REG_IMM_MIOS1ER0_EN0      0x0001
#define CYGARC_REG_IMM_MIOS1ER1_EN31     0x8000
#define CYGARC_REG_IMM_MIOS1ER1_EN30     0x4000
#define CYGARC_REG_IMM_MIOS1ER1_EN29     0x2000
#define CYGARC_REG_IMM_MIOS1ER1_EN28     0x1000
#define CYGARC_REG_IMM_MIOS1ER1_EN27     0x0800
#define CYGARC_REG_IMM_MIOS1ER1_EN22     0x0040
#define CYGARC_REG_IMM_MIOS1ER1_EN19     0x0008
#define CYGARC_REG_IMM_MIOS1ER1_EN18     0x0004
#define CYGARC_REG_IMM_MIOS1ER1_EN17     0x0002
#define CYGARC_REG_IMM_MIOS1ER1_EN16     0x0001
#define CYGARC_REG_IMM_MIOS1SR0_FL15     0x8000
#define CYGARC_REG_IMM_MIOS1SR0_FL14     0x4000
#define CYGARC_REG_IMM_MIOS1SR0_FL13     0x2000
#define CYGARC_REG_IMM_MIOS1SR0_FL12     0x1000
#define CYGARC_REG_IMM_MIOS1SR0_FL11     0x0800
#define CYGARC_REG_IMM_MIOS1SR0_FL6      0x0040
#define CYGARC_REG_IMM_MIOS1SR0_FL3      0x0008
#define CYGARC_REG_IMM_MIOS1SR0_FL2      0x0004
#define CYGARC_REG_IMM_MIOS1SR0_FL1      0x0002
#define CYGARC_REG_IMM_MIOS1SR0_FL0      0x0001
#define CYGARC_REG_IMM_MIOS1SR1_FL31     0x8000
#define CYGARC_REG_IMM_MIOS1SR1_FL30     0x4000
#define CYGARC_REG_IMM_MIOS1SR1_FL29     0x2000
#define CYGARC_REG_IMM_MIOS1SR1_FL28     0x1000
#define CYGARC_REG_IMM_MIOS1SR1_FL27     0x0800
#define CYGARC_REG_IMM_MIOS1SR1_FL22     0x0040
#define CYGARC_REG_IMM_MIOS1SR1_FL19     0x0008
#define CYGARC_REG_IMM_MIOS1SR1_FL18     0x0004
#define CYGARC_REG_IMM_MIOS1SR1_FL17     0x0002
#define CYGARC_REG_IMM_MIOS1SR1_FL16     0x0001
#define CYGARC_REG_IMM_MIOS1LVL_LVL      0x0700
#define CYGARC_REG_IMM_MIOS1LVL_LVL_SHIFT     8
#define CYGARC_REG_IMM_MIOS1LVL_TM       0x00c0
#define CYGARC_REG_IMM_MIOS1LVL_TM_SHIFT      6

//-----------------------------------------------------------------------------
// Periodic interrupt timer count
#define CYGARC_REG_IMM_PITC_COUNT_SHIFT 16              // count is stored in bits 0-15

//-----------------------------------------------------------------------------
// System clock control   
#define CYGARC_REG_IMM_SCCR_TBS         0x02000000      // Time base source
#define CYGARC_REG_IMM_SCCR_RTDIV       0x01000000      // rtc clock divide
#define CYGARC_REG_IMM_SCCR_RTSEL       0x00100000      // rtc clock select

//-------------------------------------
// TouCAN (CAN 2.0B Controller)
#define CYGARC_REG_IMM_CANICR_IRL                  0x0700
#define CYGARC_REG_IMM_CANICR_IRL_SHIFT            8
#define CYGARC_REG_IMM_CANICR_ILBS                 0x00c0
#define CYGARC_REG_IMM_CANICR_ILBS_SHIFT           6

#define CYGARC_REG_IMM_TCNMCR_STOP                 0x8000
#define CYGARC_REG_IMM_TCNMCR_FRZ                  0x4000
#define CYGARC_REG_IMM_TCNMCR_HALT                 0x1000
#define CYGARC_REG_IMM_TCNMCR_NOTRDY               0x0800
#define CYGARC_REG_IMM_TCNMCR_WAKEMSK              0x0400
#define CYGARC_REG_IMM_TCNMCR_SOFTRST              0x0200
#define CYGARC_REG_IMM_TCNMCR_FRZACK               0x0100
#define CYGARC_REG_IMM_TCNMCR_SUPV                 0x0080
#define CYGRAC_REG_IMM_TCNMCR_SELFWAKE             0x0040
#define CYGARC_REG_IMM_TCNMCR_APS                  0x0020
#define CYGARC_REG_IMM_TCNMCR_STOPACK              0x0010

#define CYGARC_REG_IMM_CANCTRL0_CANCTRL1_BOFFMSK   0x8000
#define CYGARC_REG_IMM_CANCTRL0_CANCTRL1_ERRMSK    0x4000
#define CYGARC_REG_IMM_CANCTRL0_CANCTRL1_RXMOD     0x0c00
#define CYGARC_REG_IMM_CANCTRL0_CANCTRL1_TXMOD     0x0300
#define CYGARC_REG_IMM_CANCTRL0_CANCTRL1_RXMOD_SHIFT   10
#define CYGARC_REG_IMM_CANCTRL0_CANCTRL1_TXMOD_SHIFT    8

#define CYGARC_REG_IMM_ESTAT_BITERR                0xc000
#define CYGARC_REG_IMM_ESTAT_ACKERR                0x2000
#define CYGARC_REG_IMM_ESTAT_CRCERR                0x1000
#define CYGARC_REG_IMM_ESTAT_FORMERR               0x0800
#define CYGARC_REG_IMM_ESTAT_STUFFERR              0x0400
#define CYGARC_REG_IMM_ESTAT_TXWARN                0x0200
#define CYGARC_REG_IMM_ESTAT_RXWARN                0x0100
#define CYGARC_REG_IMM_ESTAT_IDLE                  0x0080
#define CYGARC_REG_IMM_ESTAT_TX_RX                 0x0040
#define CYGARC_REG_IMM_ESTAT_FCS                   0x0030
#define CYGARC_REG_IMM_ESTAT_BOFFINT               0x0004
#define CYGARC_REG_IMM_ESTAT_ERRINT                0x0002
#define CYGARC_REG_IMM_ESTAT_WAKEINT               0x0001
#define CYGARC_REG_IMM_ESTAT_BITERR_SHIFT              14
#define CYGARC_REG_IMM_ESTAT_FCS_SHIFT                  4
                       
//-----------------------------------------------------------------------------
// All registers in the internal memory map
#define CYGARC_REG_IMM_BASE                  (0x002fc000)

			// General register definitions
#define CYGARC_REG_IMM_SIUMCR                (CYGARC_REG_IMM_BASE+0x0000)
#define CYGARC_REG_IMM_SYPCR                 (CYGARC_REG_IMM_BASE+0x0004)
#define CYGARC_REG_IMM_SWSR                  (CYGARC_REG_IMM_BASE+0x000e)
#define CYGARC_REG_IMM_SIPEND                (CYGARC_REG_IMM_BASE+0x0010)
#define CYGARC_REG_IMM_SIMASK                (CYGARC_REG_IMM_BASE+0x0014)
#define CYGARC_REG_IMM_SIEL                  (CYGARC_REG_IMM_BASE+0x0018)
#define CYGARC_REG_IMM_SIVEC                 (CYGARC_REG_IMM_BASE+0x001c)
#define CYGARC_REG_IMM_TESR                  (CYGARC_REG_IMM_BASE+0x0020)
#define CYGARC_REG_IMM_SGPIODT1              (CYGARC_REG_IMM_BASE+0x0024)
#define CYGARC_REG_IMM_SGPIODT2              (CYGARC_REG_IMM_BASE+0x0028)
#define CYGARC_REG_IMM_SGPIOCR               (CYGARC_REG_IMM_BASE+0x002c)
#define CYGARC_REG_IMM_EMCR                  (CYGARC_REG_IMM_BASE+0x0030)
#define CYGARC_REG_IMM_PDMCR                 (CYGARC_REG_IMM_BASE+0x003c)

			// Memory controller registers
#define CYGARC_REG_IMM_BR0                   (CYGARC_REG_IMM_BASE+0x0100)
#define CYGARC_REG_IMM_OR0                   (CYGARC_REG_IMM_BASE+0x0104)
#define CYGARC_REG_IMM_BR1                   (CYGARC_REG_IMM_BASE+0x0108)
#define CYGARC_REG_IMM_OR1                   (CYGARC_REG_IMM_BASE+0x010c)
#define CYGARC_REG_IMM_BR2                   (CYGARC_REG_IMM_BASE+0x0110)
#define CYGARC_REG_IMM_OR2                   (CYGARC_REG_IMM_BASE+0x0114)
#define CYGARC_REG_IMM_BR3                   (CYGARC_REG_IMM_BASE+0x0118)
#define CYGARC_REG_IMM_OR3                   (CYGARC_REG_IMM_BASE+0x011c)
#define CYGARC_REG_IMM_DMBR                  (CYGARC_REG_IMM_BASE+0x0140)
#define CYGARC_REG_IMM_DMOR                  (CYGARC_REG_IMM_BASE+0x0144)
#define CYGARC_REG_IMM_MSTAT                 (CYGARC_REG_IMM_BASE+0x0178)

			// System integration timers
#define CYGARC_REG_IMM_TBSCR                 (CYGARC_REG_IMM_BASE+0x0200)
#define CYGARC_REG_IMM_TBREF0                (CYGARC_REG_IMM_BASE+0x0204)
#define CYGARC_REG_IMM_TBREF1                (CYGARC_REG_IMM_BASE+0x0208)
#define CYGARC_REG_IMM_RTCSC                 (CYGARC_REG_IMM_BASE+0x0220)
#define CYGARC_REG_IMM_RTC                   (CYGARC_REG_IMM_BASE+0x0224)
#define CYGARC_REG_IMM_RTSEC                 (CYGARC_REG_IMM_BASE+0x0228)
#define CYGARC_REG_IMM_RTCAL                 (CYGARC_REG_IMM_BASE+0x022c)
#define CYGARC_REG_IMM_PISCR                 (CYGARC_REG_IMM_BASE+0x0240)
#define CYGARC_REG_IMM_PITC                  (CYGARC_REG_IMM_BASE+0x0244)
#define CYGARC_REG_IMM_PITR                  (CYGARC_REG_IMM_BASE+0x0248)

			// Clocks and resets
#define CYGARC_REG_IMM_SCCR                  (CYGARC_REG_IMM_BASE+0x0280)
#define CYGARC_REG_IMM_PLPRCR                (CYGARC_REG_IMM_BASE+0x0284)
#define CYGARC_REG_IMM_RSR                   (CYGARC_REG_IMM_BASE+0x0288)
#define CYGARC_REG_IMM_COLIR                 (CYGARC_REG_IMM_BASE+0x028c)
#define CYGARC_REG_IMM_VSRMCR                (CYGARC_REG_IMM_BASE+0x0290)

			// System IntegrationTimer Keys
#define CYGARC_REG_IMM_TBSCRK                (CYGARC_REG_IMM_BASE+0x0300)
#define CYGARC_REG_IMM_TBREF0K               (CYGARC_REG_IMM_BASE+0x0304)
#define CYGARC_REG_IMM_TBREF1K               (CYGARC_REG_IMM_BASE+0x0308)
#define CYGARC_REG_IMM_TBK                   (CYGARC_REG_IMM_BASE+0x030c)
#define CYGARC_REG_IMM_RTCSCK                (CYGARC_REG_IMM_BASE+0x0320)
#define CYGARC_REG_IMM_RTCK                  (CYGARC_REG_IMM_BASE+0x0324)
#define CYGARC_REG_IMM_RTSECK                (CYGARC_REG_IMM_BASE+0x0328)
#define CYGARC_REG_IMM_RTCALK                (CYGARC_REG_IMM_BASE+0x032c)
#define CYGARC_REG_IMM_PISCRK                (CYGARC_REG_IMM_BASE+0x0340)
#define CYGARC_REG_IMM_PITCK                 (CYGARC_REG_IMM_BASE+0x0344)

			// Clocks and reset keys
#define CYGARC_REG_IMM_SCCRK                 (CYGARC_REG_IMM_BASE+0x0380)
#define CYGARC_REG_IMM_PLPRCRK               (CYGARC_REG_IMM_BASE+0x0384)
#define CYGARC_REG_IMM_RSRK                  (CYGARC_REG_IMM_BASE+0x0388)

		//-------------------------------------
		// CMF (CDR Mone T FLASH EEPROM)
		//-------------------------------------
			// CMF_A
#define CYGARC_REG_IMM_CMFMCR_A              (CYGARC_REG_IMM_BASE+0x0800)
#define CYGARC_REG_IMM_CMFTST_A              (CYGARC_REG_IMM_BASE+0x0804)
#define CYGARC_REG_IMM_CMFCTL_A              (CYGARC_REG_IMM_BASE+0x0808)
	
			// CMF_B
#define CYGARC_REG_IMM_CMFMCR_B              (CYGARC_REG_IMM_BASE+0x0840)
#define CYGARC_REG_IMM_CMFTST_B              (CYGARC_REG_IMM_BASE+0x0844)
#define CYGARC_REG_IMM_CMFCTL_B              (CYGARC_REG_IMM_BASE+0x0848)

		//-------------------------------------
		// DPTRAM (Dual-Port TPU RAM)
		//-------------------------------------
#define CYGARC_REG_IMM_DPTMCR                (CYGARC_REG_IMM_BASE+0x4000)
#define CYGARC_REG_IMM_RAMTST                (CYGARC_REG_IMM_BASE+0x4002)
#define CYGARC_REG_IMM_RAMBAR                (CYGARC_REG_IMM_BASE+0x4004)
#define CYGARC_REG_IMM_MISRH                 (CYGARC_REG_IMM_BASE+0x4006)
#define CYGARC_REG_IMM_MISRL                 (CYGARC_REG_IMM_BASE+0x4008)
#define CYGARC_REG_IMM_MISCNT                (CYGARC_REG_IMM_BASE+0x400a)

		//-------------------------------------
		// TPU3 (Time processing unit)
		//-------------------------------------
			// TPU-A
#define CYGARC_REG_IMM_TPUMCR_A              (CYGARC_REG_IMM_BASE+0x8000)
#define CYGARC_REG_IMM_TCR_A                 (CYGARC_REG_IMM_BASE+0x8002)
#define CYGARC_REG_IMM_DSCR_A                (CYGARC_REG_IMM_BASE+0x8004)
#define CYGARC_REG_IMM_DSSR_A                (CYGARC_REG_IMM_BASE+0x8006)
#define CYGARC_REG_IMM_TICR_A                (CYGARC_REG_IMM_BASE+0x8008)
#define CYGARC_REG_IMM_CIER_A                (CYGARC_REG_IMM_BASE+0x800a)
#define CYGARC_REG_IMM_CFSR0_A               (CYGARC_REG_IMM_BASE+0x800c)
#define CYGARC_REG_IMM_CFSR1_A               (CYGARC_REG_IMM_BASE+0x800e)
#define CYGARC_REG_IMM_CFSR2_A               (CYGARC_REG_IMM_BASE+0x8010)
#define CYGARC_REG_IMM_CFSR3_A               (CYGARC_REG_IMM_BASE+0x8012)
#define CYGARC_REG_IMM_HSQR0_A               (CYGARC_REG_IMM_BASE+0x8014)
#define CYGARC_REG_IMM_HSQR1_A               (CYGARC_REG_IMM_BASE+0x8016)
#define CYGARC_REG_IMM_HSRR0_A               (CYGARC_REG_IMM_BASE+0x8018)
#define CYGARC_REG_IMM_HSRR1_A               (CYGARC_REG_IMM_BASE+0x801a)
#define CYGARC_REG_IMM_CPR0_A                (CYGARC_REG_IMM_BASE+0x801c)
#define CYGARC_REG_IMM_CPR1_A                (CYGARC_REG_IMM_BASE+0x801e)
#define CYGARC_REG_IMM_CISR_A                (CYGARC_REG_IMM_BASE+0x8020)
#define CYGARC_REG_IMM_LR_A                  (CYGARC_REG_IMM_BASE+0x8022)
#define CYGARC_REG_IMM_SGLR_A                (CYGARC_REG_IMM_BASE+0x8024)
#define CYGARC_REG_IMM_DCNR_A                (CYGARC_REG_IMM_BASE+0x8026)
#define CYGARC_REG_IMM_TPUMCR2_A             (CYGARC_REG_IMM_BASE+0x8028)
#define CYGARC_REG_IMM_TPUMCR3_A             (CYGARC_REG_IMM_BASE+0x802a)
#define CYGARC_REG_IMM_ISDR_A                (CYGARC_REG_IMM_BASE+0x802c)
#define CYGARC_REG_IMM_ISCR_A                (CYGARC_REG_IMM_BASE+0x802e)

              		// TPU-B
#define CYGARC_REG_IMM_TPUMCR_B              (CYGARC_REG_IMM_BASE+0x8400)
#define CYGARC_REG_IMM_TCR_B                 (CYGARC_REG_IMM_BASE+0x8402)
#define CYGARC_REG_IMM_DSCR_B                (CYGARC_REG_IMM_BASE+0x8404)
#define CYGARC_REG_IMM_DSSR_B                (CYGARC_REG_IMM_BASE+0x8406)
#define CYGARC_REG_IMM_TICR_B                (CYGARC_REG_IMM_BASE+0x8408)
#define CYGARC_REG_IMM_CIER_B                (CYGARC_REG_IMM_BASE+0x840a)
#define CYGARC_REG_IMM_CFSR0_B               (CYGARC_REG_IMM_BASE+0x840c)
#define CYGARC_REG_IMM_CFSR1_B               (CYGARC_REG_IMM_BASE+0x840e)
#define CYGARC_REG_IMM_CFSR2_B               (CYGARC_REG_IMM_BASE+0x8410)
#define CYGARC_REG_IMM_CFSR3_B               (CYGARC_REG_IMM_BASE+0x8412)
#define CYGARC_REG_IMM_HSQR0_B               (CYGARC_REG_IMM_BASE+0x8414)
#define CYGARC_REG_IMM_HSQR1_B               (CYGARC_REG_IMM_BASE+0x8416)
#define CYGARC_REG_IMM_HSRR0_B               (CYGARC_REG_IMM_BASE+0x8418)
#define CYGARC_REG_IMM_HSRR1_B               (CYGARC_REG_IMM_BASE+0x841a)
#define CYGARC_REG_IMM_CPR0_B                (CYGARC_REG_IMM_BASE+0x841c)
#define CYGARC_REG_IMM_CPR1_B                (CYGARC_REG_IMM_BASE+0x841e)
#define CYGARC_REG_IMM_CISR_B                (CYGARC_REG_IMM_BASE+0x8420)
#define CYGARC_REG_IMM_LR_B                  (CYGARC_REG_IMM_BASE+0x8422)
#define CYGARC_REG_IMM_SGLR_B                (CYGARC_REG_IMM_BASE+0x8424)
#define CYGARC_REG_IMM_DCNR_B                (CYGARC_REG_IMM_BASE+0x8426)
#define CYGARC_REG_IMM_TPUMCR2_B             (CYGARC_REG_IMM_BASE+0x8428)
#define CYGARC_REG_IMM_TPUMCR3_B             (CYGARC_REG_IMM_BASE+0x842a)
#define CYGARC_REG_IMM_ISDR_B                (CYGARC_REG_IMM_BASE+0x842c)
#define CYGARC_REG_IMM_ISCR_B                (CYGARC_REG_IMM_BASE+0x842e)


		//-------------------------------------
		// QADC64 (Queued Analog-to-digital Converter)
		//-------------------------------------
			// QUADC-A
#define CYGARC_REG_IMM_QUADC64MCR_A          (CYGARC_REG_IMM_BASE+0x8800)
#define CYGARC_REG_IMM_QUADC64TEST_A         (CYGARC_REG_IMM_BASE+0x8802)
#define CYGARC_REG_IMM_QUADC64INT_A          (CYGARC_REG_IMM_BASE+0x8804)
#define CYGARC_REG_IMM_PORTQA_A_PORTQB_A     (CYGARC_REG_IMM_BASE+0x8806)
#define CYGARC_REG_IMM_DDRQA_A_DDRQB_A       (CYGARC_REG_IMM_BASE+0x8808)
#define CYGARC_REG_IMM_QUACR0_A              (CYGARC_REG_IMM_BASE+0x880a)
#define CYGARC_REG_IMM_QUACR1_A              (CYGARC_REG_IMM_BASE+0x880c)
#define CYGARC_REG_IMM_QUACR2_A              (CYGARC_REG_IMM_BASE+0x880e)
#define CYGARC_REG_IMM_QUASR0_A              (CYGARC_REG_IMM_BASE+0x8810)
#define CYGARC_REG_IMM_QUASR1_A              (CYGARC_REG_IMM_BASE+0x8812)
#define CYGARC_REG_IMM_CCW_A                 (CYGARC_REG_IMM_BASE+0x8a00)
#define CYGARC_REG_IMM_RJURR_A               (CYGARC_REG_IMM_BASE+0x8a80)
#define CYGARC_REG_IMM_LJSRR_A               (CYGARC_REG_IMM_BASE+0x8b00)
#define CYGARC_REG_IMM_LJURR_A               (CYGARC_REG_IMM_BASE+0x8b80)

			// QUADC-B
#define CYGARC_REG_IMM_QUADC64MCR_B          (CYGARC_REG_IMM_BASE+0x8c00)
#define CYGARC_REG_IMM_QUADC64TEST_B         (CYGARC_REG_IMM_BASE+0x8c02)
#define CYGARC_REG_IMM_QUADC64INT_B          (CYGARC_REG_IMM_BASE+0x8c04)
#define CYGARC_REG_IMM_PORTQA_B_PORTQB_B     (CYGARC_REG_IMM_BASE+0x8c06)
#define CYGARC_REG_IMM_DDRQA_B_DDRQB_B       (CYGARC_REG_IMM_BASE+0x8c08)
#define CYGARC_REG_IMM_QUACR0_B              (CYGARC_REG_IMM_BASE+0x8c0a)
#define CYGARC_REG_IMM_QUACR1_B              (CYGARC_REG_IMM_BASE+0x8c0c)
#define CYGARC_REG_IMM_QUACR2_B              (CYGARC_REG_IMM_BASE+0x8c0e)
#define CYGARC_REG_IMM_QUASR0_B              (CYGARC_REG_IMM_BASE+0x8c10)
#define CYGARC_REG_IMM_QUASR1_B              (CYGARC_REG_IMM_BASE+0x8c12)
#define CYGARC_REG_IMM_CCW_B                 (CYGARC_REG_IMM_BASE+0x8e00)
#define CYGARC_REG_IMM_RJURR_B               (CYGARC_REG_IMM_BASE+0x8e80)
#define CYGARC_REG_IMM_LJSRR_B               (CYGARC_REG_IMM_BASE+0x8f00)
#define CYGARC_REG_IMM_LJURR_B               (CYGARC_REG_IMM_BASE+0x8f80)

		//-------------------------------------
		// QSMCM (Queued Serial Multi-Channel Module)
		//-------------------------------------
#define CYGARC_REG_IMM_QSMCMMCR              (CYGARC_REG_IMM_BASE+0x9000)
#define CYGARC_REG_IMM_QTEST                 (CYGARC_REG_IMM_BASE+0x9002)
#define CYGARC_REG_IMM_QDSCI_IL              (CYGARC_REG_IMM_BASE+0x9004)
#define CYGARC_REG_IMM_QSPI_IL               (CYGARC_REG_IMM_BASE+0x9006)
#define CYGARC_REG_IMM_SCC1R0                (CYGARC_REG_IMM_BASE+0x9008)
#define CYGARC_REG_IMM_SCC1R1                (CYGARC_REG_IMM_BASE+0x900a)
#define CYGARC_REG_IMM_SC1SR                 (CYGARC_REG_IMM_BASE+0x900c)
#define CYGARC_REG_IMM_SC1DR                 (CYGARC_REG_IMM_BASE+0x900e)
#define CYGARC_REG_IMM_PORTQS                (CYGARC_REG_IMM_BASE+0x9014)
#define CYGARC_REG_IMM_PQSPAR_DDRQST         (CYGARC_REG_IMM_BASE+0x9016)
#define CYGARC_REG_IMM_SPCR0                 (CYGARC_REG_IMM_BASE+0x9018)
#define CYGARC_REG_IMM_SPCR1                 (CYGARC_REG_IMM_BASE+0x901a)
#define CYGARC_REG_IMM_SPCR2                 (CYGARC_REG_IMM_BASE+0x901c)
#define CYGARC_REG_IMM_SPCR3                 (CYGARC_REG_IMM_BASE+0x901e)
#define CYGARC_REG_IMM_SPSR                  (CYGARC_REG_IMM_BASE+0x901f)
#define CYGARC_REG_IMM_SCC2R0                (CYGARC_REG_IMM_BASE+0x9020)
#define CYGARC_REG_IMM_SCC2R1                (CYGARC_REG_IMM_BASE+0x9022)
#define CYGARC_REG_IMM_SC2SR                 (CYGARC_REG_IMM_BASE+0x9024)
#define CYGARC_REG_IMM_SC2DR                 (CYGARC_REG_IMM_BASE+0x9026)
#define CYGARC_REG_IMM_QSCI1CR               (CYGARC_REG_IMM_BASE+0x9028)
#define CYGARC_REG_IMM_QSCI1SR               (CYGARC_REG_IMM_BASE+0x902a)
#define CYGARC_REG_IMM_SCTQ                  (CYGARC_REG_IMM_BASE+0x902c)
#define CYGARC_REG_IMM_SCRQ                  (CYGARC_REG_IMM_BASE+0x904c)
#define CYGARC_REG_IMM_RECRAM                (CYGARC_REG_IMM_BASE+0x9140)
#define CYGARC_REG_IMM_TRAN_RAM              (CYGARC_REG_IMM_BASE+0x9180)
#define CYGARC_REG_IMM_COMD_RAM              (CYGARC_REG_IMM_BASE+0x91c0)

		//-------------------------------------
		// MIOS1 (Modular Input/Output Subsystem)
		//-------------------------------------
			// MPIOS Pulse width modulation submodule 0
#define CYGARC_REG_IMM_MPWMSMPERR_0          (CYGARC_REG_IMM_BASE+0xa000)
#define CYGARC_REG_IMM_MPWMSMPULR_0          (CYGARC_REG_IMM_BASE+0xa002)
#define CYGARC_REG_IMM_MPWMSMCNTR_0          (CYGARC_REG_IMM_BASE+0xa004)
#define CYGARC_REG_IMM_MPWMSMSCR_0           (CYGARC_REG_IMM_BASE+0xa006)
		
			// MPIOS Pulse width modulation submodule 1
#define CYGARC_REG_IMM_MPWMSMPERR_1          (CYGARC_REG_IMM_BASE+0xa008)
#define CYGARC_REG_IMM_MPWMSMPULR_1          (CYGARC_REG_IMM_BASE+0xa00a)
#define CYGARC_REG_IMM_MPWMSMCNTR_1          (CYGARC_REG_IMM_BASE+0xa00c)
#define CYGARC_REG_IMM_MPWMSMSCR_1           (CYGARC_REG_IMM_BASE+0xa00e)

			// MPIOS Pulse width modulation submodule 2
#define CYGARC_REG_IMM_MPWMSMPERR_2          (CYGARC_REG_IMM_BASE+0xa010)
#define CYGARC_REG_IMM_MPWMSMPULR_2          (CYGARC_REG_IMM_BASE+0xa012)
#define CYGARC_REG_IMM_MPWMSMCNTR_2          (CYGARC_REG_IMM_BASE+0xa014)
#define CYGARC_REG_IMM_MPWMSMSCR_2           (CYGARC_REG_IMM_BASE+0xa016)

			// MPIOS Pulse width modulation submodule 3
#define CYGARC_REG_IMM_MPWMSMPERR_3          (CYGARC_REG_IMM_BASE+0xa018)
#define CYGARC_REG_IMM_MPWMSMPULR_3          (CYGARC_REG_IMM_BASE+0xa01a)
#define CYGARC_REG_IMM_MPWMSMCNTR_3          (CYGARC_REG_IMM_BASE+0xa01c)
#define CYGARC_REG_IMM_MPWMSMSCR_3           (CYGARC_REG_IMM_BASE+0xa01e)

			// MIOS Modulus counter submodule 6
#define CYGARC_REG_IMM_MMCSMCNT_6            (CYGARC_REG_IMM_BASE+0xa030)
#define CYGARC_REG_IMM_MMCSMML_6             (CYGARC_REG_IMM_BASE+0xa032)
#define CYGARC_REG_IMM_MMCSMSCRD_6           (CYGARC_REG_IMM_BASE+0xa034)
#define CYGARC_REG_IMM_MMCSMSCR_6            (CYGARC_REG_IMM_BASE+0xa036)

			// MIOS Double Action submodule 11
#define CYGARC_REG_IMM_MDASMAR_11            (CYGARC_REG_IMM_BASE+0xa058)
#define CYGARC_REG_IMM_MDASMBR_11            (CYGARC_REG_IMM_BASE+0xa05a)
#define CYGARC_REG_IMM_MDASMSCRD_11          (CYGARC_REG_IMM_BASE+0xa05c)
#define CYGARC_REG_IMM_MDASMSCR_11           (CYGARC_REG_IMM_BASE+0xa05e)

			// MIOS Double Action submodule 12
#define CYGARC_REG_IMM_MDASMAR_12            (CYGARC_REG_IMM_BASE+0xa060)
#define CYGARC_REG_IMM_MDASMBR_12            (CYGARC_REG_IMM_BASE+0xa062)
#define CYGARC_REG_IMM_MDASMSCRD_12          (CYGARC_REG_IMM_BASE+0xa064)
#define CYGARC_REG_IMM_MDASMSCR_12           (CYGARC_REG_IMM_BASE+0xa066)

			// MIOS Double Action submodule 13
#define CYGARC_REG_IMM_MDASMAR_13            (CYGARC_REG_IMM_BASE+0xa068)
#define CYGARC_REG_IMM_MDASMBR_13            (CYGARC_REG_IMM_BASE+0xa06a)
#define CYGARC_REG_IMM_MDASMSCRD_13          (CYGARC_REG_IMM_BASE+0xa06c)
#define CYGARC_REG_IMM_MDASMSCR_13           (CYGARC_REG_IMM_BASE+0xa06e)

			// MIOS Double Action submodule 14
#define CYGARC_REG_IMM_MDASMAR_14            (CYGARC_REG_IMM_BASE+0xa070)
#define CYGARC_REG_IMM_MDASMBR_14            (CYGARC_REG_IMM_BASE+0xa072)
#define CYGARC_REG_IMM_MDASMSCRD_14          (CYGARC_REG_IMM_BASE+0xa074)
#define CYGARC_REG_IMM_MDASMSCR_14           (CYGARC_REG_IMM_BASE+0xa076)

			// MIOS Double Action submodule 15
#define CYGARC_REG_IMM_MDASMAR_15            (CYGARC_REG_IMM_BASE+0xa078)
#define CYGARC_REG_IMM_MDASMBR_15            (CYGARC_REG_IMM_BASE+0xa07a)
#define CYGARC_REG_IMM_MDASMSCRD_15          (CYGARC_REG_IMM_BASE+0xa07c)
#define CYGARC_REG_IMM_MDASMSCR_15           (CYGARC_REG_IMM_BASE+0xa07e)

			// MPIOS Pulse width modulation submodule 16
#define CYGARC_REG_IMM_MPWMSMPERR_16         (CYGARC_REG_IMM_BASE+0xa080)
#define CYGARC_REG_IMM_MPWMSMPULR_16         (CYGARC_REG_IMM_BASE+0xa082)
#define CYGARC_REG_IMM_MPWMSMCNTR_16         (CYGARC_REG_IMM_BASE+0xa084)
#define CYGARC_REG_IMM_MPWMSMSCR_16          (CYGARC_REG_IMM_BASE+0xa086)

			// MPIOS Pulse width modulation submodule 17
#define CYGARC_REG_IMM_MPWMSMPERR_17         (CYGARC_REG_IMM_BASE+0xa088)
#define CYGARC_REG_IMM_MPWMSMPULR_17         (CYGARC_REG_IMM_BASE+0xa08a)
#define CYGARC_REG_IMM_MPWMSMCNTR_17         (CYGARC_REG_IMM_BASE+0xa08c)
#define CYGARC_REG_IMM_MPWMSMSCR_17          (CYGARC_REG_IMM_BASE+0xa08e)

			// MPIOS Pulse width modulation submodule 18
#define CYGARC_REG_IMM_MPWMSMPERR_18         (CYGARC_REG_IMM_BASE+0xa090)
#define CYGARC_REG_IMM_MPWMSMPULR_18         (CYGARC_REG_IMM_BASE+0xa092)
#define CYGARC_REG_IMM_MPWMSMCNTR_18         (CYGARC_REG_IMM_BASE+0xa094)
#define CYGARC_REG_IMM_MPWMSMSCR_18          (CYGARC_REG_IMM_BASE+0xa096)

			// MPIOS Pulse width modulation submodule 19
#define CYGARC_REG_IMM_MPWMSMPERR_19         (CYGARC_REG_IMM_BASE+0xa098)
#define CYGARC_REG_IMM_MPWMSMPULR_19         (CYGARC_REG_IMM_BASE+0xa09a)
#define CYGARC_REG_IMM_MPWMSMCNTR_19         (CYGARC_REG_IMM_BASE+0xa09c)
#define CYGARC_REG_IMM_MPWMSMSCR_19          (CYGARC_REG_IMM_BASE+0xa09e)

			// MIOS Modulus counter submodule 22
#define CYGARC_REG_IMM_MMCSMCNT_22           (CYGARC_REG_IMM_BASE+0xa0b0)
#define CYGARC_REG_IMM_MMCSMML_22            (CYGARC_REG_IMM_BASE+0xa0b2)
#define CYGARC_REG_IMM_MMCSMSCRD_22          (CYGARC_REG_IMM_BASE+0xa0b4)
#define CYGARC_REG_IMM_MMCSMSCR_22           (CYGARC_REG_IMM_BASE+0xa0b6)

			// MIOS Double action submodule 27
#define CYGARC_REG_IMM_MDASMAR_27            (CYGARC_REG_IMM_BASE+0xa0d8)
#define CYGARC_REG_IMM_MDASMBR_27            (CYGARC_REG_IMM_BASE+0xa0da)
#define CYGARC_REG_IMM_MDASMSCRD_27          (CYGARC_REG_IMM_BASE+0xa0dc)
#define CYGARC_REG_IMM_MDASMSCR_27           (CYGARC_REG_IMM_BASE+0xa0de)

			// MIOS Double action submodule 28
#define CYGARC_REG_IMM_MDASMAR_28            (CYGARC_REG_IMM_BASE+0xa0e0)
#define CYGARC_REG_IMM_MDASMBR_28            (CYGARC_REG_IMM_BASE+0xa0e2)
#define CYGARC_REG_IMM_MDASMSCRD_28          (CYGARC_REG_IMM_BASE+0xa0e4)
#define CYGARC_REG_IMM_MDASMSCR_28           (CYGARC_REG_IMM_BASE+0xa0e6)

			// MIOS Double action submodule 29
#define CYGARC_REG_IMM_MDASMAR_29            (CYGARC_REG_IMM_BASE+0xa0e8)
#define CYGARC_REG_IMM_MDASMBR_29            (CYGARC_REG_IMM_BASE+0xa0ea)
#define CYGARC_REG_IMM_MDASMSCRD_29          (CYGARC_REG_IMM_BASE+0xa0ec)
#define CYGARC_REG_IMM_MDASMSCR_29           (CYGARC_REG_IMM_BASE+0xa0ee)

			// MIOS Double action submodule 30
#define CYGARC_REG_IMM_MDASMAR_30            (CYGARC_REG_IMM_BASE+0xa0f0)
#define CYGARC_REG_IMM_MDASMBR_30            (CYGARC_REG_IMM_BASE+0xa0f2)
#define CYGARC_REG_IMM_MDASMSCRD_30          (CYGARC_REG_IMM_BASE+0xa0f4)
#define CYGARC_REG_IMM_MDASMSCR_30           (CYGARC_REG_IMM_BASE+0xa0f6)

			// MIOS Double action submodule 31
#define CYGARC_REG_IMM_MDASMAR_31            (CYGARC_REG_IMM_BASE+0x0af8)
#define CYGARC_REG_IMM_MDASMBR_31            (CYGARC_REG_IMM_BASE+0xa0fa)
#define CYGARC_REG_IMM_MDASMSCRD_31          (CYGARC_REG_IMM_BASE+0xa0fc)
#define CYGARC_REG_IMM_MDASMSCR_31           (CYGARC_REG_IMM_BASE+0xa0fe)

			// MIOS Paralell port I/O submodule
#define CYGARC_REG_IMM_MPIOSMDR              (CYGARC_REG_IMM_BASE+0xa100)
#define CYGARC_REG_IMM_MPIOSMDDR             (CYGARC_REG_IMM_BASE+0xa102)

			// MIOS Bus interface Submodule
#define CYGARC_REG_IMM_MIOS1TPCR             (CYGARC_REG_IMM_BASE+0xa800)
#define CYGARC_REG_IMM_MIOS1VNR              (CYGARC_REG_IMM_BASE+0xa802)
#define CYGARC_REG_IMM_MIOS1MCR              (CYGARC_REG_IMM_BASE+0xa806)

			// MIOS Counter / Prescaler submodule
#define CYGARC_REG_IMM_MCPSMSCR              (CYGARC_REG_IMM_BASE+0xa816)

			// MIOS Interrupt request submodule 0
#define CYGARC_REG_IMM_MIOS1SR0              (CYGARC_REG_IMM_BASE+0xac00)
#define CYGARC_REG_IMM_MIOS1ER0              (CYGARC_REG_IMM_BASE+0xac04)
#define CYGARC_REG_IMM_MIOS1RPR0             (CYGARC_REG_IMM_BASE+0xac06)
#define CYGARC_REG_IMM_MIOS1LVL0             (CYGARC_REG_IMM_BASE+0xac30)

			// Mios Interrupt request submodule 1
#define CYGARC_REG_IMM_MIOS1SR1              (CYGARC_REG_IMM_BASE+0xac40)
#define CYGARC_REG_IMM_MIOS1ER1              (CYGARC_REG_IMM_BASE+0xac44)
#define CYGARC_REG_IMM_MIOS1RPR1             (CYGARC_REG_IMM_BASE+0xac46)
#define CYGARC_REG_IMM_MIOS1LVL1             (CYGARC_REG_IMM_BASE+0xac70)

		//-------------------------------------
		// TouCAN (CAN 2.0B Controller)
		//-------------------------------------
			// TouCAN_A
#define CYGARC_REG_IMM_TCNMCR_A              (CYGARC_REG_IMM_BASE+0xb080)
#define CYGARC_REG_IMM_TTR_A                 (CYGARC_REG_IMM_BASE+0xb082)
#define CYGARC_REG_IMM_CANICR_A              (CYGARC_REG_IMM_BASE+0xb084)
#define CYGARC_REG_IMM_CANCTRL0_A_CANCTRL1_A (CYGARC_REG_IMM_BASE+0xb086)
#define CYGARC_REG_IMM_PRESDIV_A_CTRL2_A     (CYGARC_REG_IMM_BASE+0xb088)
#define CYGARC_REG_IMM_TIMER_A               (CYGARC_REG_IMM_BASE+0xb08a)
#define CYGARC_REG_IMM_RXGMASKHI_A           (CYGARC_REG_IMM_BASE+0xb090)
#define CYGARC_REG_IMM_RXGMASKLO_A           (CYGARC_REG_IMM_BASE+0xb092)
#define CYGARC_REG_IMM_RX14MASKHI_A          (CYGARC_REG_IMM_BASE+0xb094)
#define CYGARC_REG_IMM_RX14MASKLO_A          (CYGARC_REG_IMM_BASE+0xb096)
#define CYGARC_REG_IMM_RX15MASKHI_A          (CYGARC_REG_IMM_BASE+0xb098)
#define CYGARC_REG_IMM_RX15MASKLO_A          (CYGARC_REG_IMM_BASE+0xb09a)
#define CYGARC_REG_IMM_ESTAT_A               (CYGARC_REG_IMM_BASE+0xb0a0)
#define CYGARC_REG_IMM_IMASK_A               (CYGARC_REG_IMM_BASE+0xb0a2)
#define CYGARC_REG_IMM_IFLAG_A               (CYGARC_REG_IMM_BASE+0xb0a4)
#define CYGARC_REG_IMM_RXECTR_A_TXECTR_A     (CYGARC_REG_IMM_BASE+0xb0a6)

			// TouCAN_B
#define CYGARC_REG_IMM_TCNMCR_B              (CYGARC_REG_IMM_BASE+0xb480)
#define CYGARC_REG_IMM_TTR_B                 (CYGARC_REG_IMM_BASE+0xb482)
#define CYGARC_REG_IMM_CANICR_B              (CYGARC_REG_IMM_BASE+0xb484)
#define CYGARC_REG_IMM_CANCTRL0_B_CANCTRL1_B (CYGARC_REG_IMM_BASE+0xb486)
#define CYGARC_REG_IMM_PRESDIV_B_CTRL2_B     (CYGARC_REG_IMM_BASE+0xb488)
#define CYGARC_REG_IMM_TIMER_B               (CYGARC_REG_IMM_BASE+0xb48a)
#define CYGARC_REG_IMM_RXGMASKHI_B           (CYGARC_REG_IMM_BASE+0xb490)
#define CYGARC_REG_IMM_RXGMASKLO_B           (CYGARC_REG_IMM_BASE+0xb492)
#define CYGARC_REG_IMM_RX14MASKHI_B          (CYGARC_REG_IMM_BASE+0xb494)
#define CYGARC_REG_IMM_RX14MASKLO_B          (CYGARC_REG_IMM_BASE+0xb496)
#define CYGARC_REG_IMM_RX15MASKHI_B          (CYGARC_REG_IMM_BASE+0xb498)
#define CYGARC_REG_IMM_RX15MASKLO_B          (CYGARC_REG_IMM_BASE+0xb49a)
#define CYGARC_REG_IMM_ESTAT_B               (CYGARC_REG_IMM_BASE+0xb4a0)
#define CYGARC_REG_IMM_IMASK_B               (CYGARC_REG_IMM_BASE+0xb4a2)
#define CYGARC_REG_IMM_IFLAG_B               (CYGARC_REG_IMM_BASE+0xb4a4)
#define CYGARC_REG_IMM_RXECTR_A_TXECTR_B     (CYGARC_REG_IMM_BASE+0xb4a6)

		//-------------------------------------
		// UIMB (U-Bus to IMB3 Bus Interface)
		//-------------------------------------
#define CYGARC_REG_IMM_UMCR                  (CYGARC_REG_IMM_BASE+0xbf80)
#define CYGARC_REG_IMM_UTSTCREG              (CYGARC_REG_IMM_BASE+0xbf90)
#define CYGARC_REG_IMM_UIPEND                (CYGARC_REG_IMM_BASE+0xbfa0)

		//-------------------------------------
		// SRAM (Static RAM Access memory)
		//-------------------------------------
#define CYGARC_REG_IMM_SRAMMCR_A             (CYGARC_REG_IMM_BASE+0x84000)
#define CYGARC_REG_IMM_SRAMTST_A             (CYGARC_REG_IMM_BASE+0x84004)
#define CYGARC_REG_IMM_SRAMMCR_B             (CYGARC_REG_IMM_BASE+0x84008)
#define CYGARC_REG_IMM_SRAMTST_B             (CYGARC_REG_IMM_BASE+0x8400c)

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
// definitions without CYGARC_REG_ can come here. If there is a need for it ....
#endif

//-----------------------------------------------------------------------------
#endif // ifdef CYGONCE_HAL_VAR_REGS_H
// End of var_regs.h
