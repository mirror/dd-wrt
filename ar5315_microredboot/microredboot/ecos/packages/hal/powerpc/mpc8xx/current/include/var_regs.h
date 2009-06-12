#ifndef CYGONCE_HAL_VAR_REGS_H
#define CYGONCE_HAL_VAR_REGS_H

//==========================================================================
//
//      var_regs.h
//
//      PowerPC 8xx variant CPU definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Contributors: jskov, gthomas
// Date:         2000-02-04
// Purpose:      Provide MPC8xx register definitions
// Description:  Provide MPC8xx register definitions
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

#include <cyg/hal/plf_regs.h>

//--------------------------------------------------------------------------
// Instruction cache control.
#define CYGARC_REG_IC_CST          560
#define CYGARC_REG_IC_ADR          561
#define CYGARC_REG_IC_DAT          562

#define CYGARC_REG_IC_CMD_CE       0x02000000      // cache enable
#define CYGARC_REG_IC_CMD_CD       0x04000000      // cache disable
#define CYGARC_REG_IC_CMD_LL       0x06000000      // load & lock
#define CYGARC_REG_IC_CMD_UL       0x08000000      // unlock line
#define CYGARC_REG_IC_CMD_UA       0x0a000000      // unlock all
#define CYGARC_REG_IC_CMD_IA       0x0c000000      // invalidate all

#define CYGARC_REG_IC_ADR_SETID_SHIFT 4            // set id is bits 21-27
#define CYGARC_REG_IC_ADR_WAY0     0x00000000      // select way0
#define CYGARC_REG_IC_ADR_WAY1     0x00001000      // select way1

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define IC_CST          CYGARC_REG_IC_CST
#define IC_ADR          CYGARC_REG_IC_ADR
#define IC_DAT          CYGARC_REG_IC_DAT

#define IC_CMD_CE       CYGARC_REG_IC_CMD_CE
#define IC_CMD_CD       CYGARC_REG_IC_CMD_CD
#define IC_CMD_LL       CYGARC_REG_IC_CMD_LL
#define IC_CMD_UL       CYGARC_REG_IC_CMD_UL
#define IC_CMD_UA       CYGARC_REG_IC_CMD_UA
#define IC_CMD_IA       CYGARC_REG_IC_CMD_IA

#define IC_ADR_SETID_SHIFT CYGARC_REG_IC_ADR_SETID_SHIFT
#define IC_ADR_WAY0        CYGARC_REG_IC_ADR_WAY0
#define IC_ADR_WAY1        CYGARC_REG_IC_ADR_WAY1
#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
// Data cache control.
#define CYGARC_REG_DC_CST          568
#define CYGARC_REG_DC_ADR          569
#define CYGARC_REG_DC_DAT          570

#define CYGARC_REG_DC_CMD_CE       0x02000000      // cache enable
#define CYGARC_REG_DC_CMD_CD       0x04000000      // cache disable
#define CYGARC_REG_DC_CMD_LL       0x06000000      // lock line
#define CYGARC_REG_DC_CMD_UL       0x08000000      // unlock line
#define CYGARC_REG_DC_CMD_UA       0x0a000000      // unlock all
#define CYGARC_REG_DC_CMD_IA       0x0c000000      // invalidate all
#define CYGARC_REG_DC_CMD_FL       0x0e000000      // flush line
#define CYGARC_REG_DC_CMD_SW       0x01000000      // set writethrough
#define CYGARC_REG_DC_CMD_CW       0x03000000      // clear writethrough
#define CYGARC_REG_DC_CMD_SS       0x05000000      // set little endian swap
#define CYGARC_REG_DC_CMD_CS       0x07000000      // clear little endian swap

#define CYGARC_REG_DC_ADR_SETID_SHIFT 4            // set id is bits 21-27
#define CYGARC_REG_DC_ADR_WAY0     0x00000000      // select way0
#define CYGARC_REG_DC_ADR_WAY1     0x00001000      // select way1

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define DC_CST             CYGARC_REG_DC_CST
#define DC_ADR             CYGARC_REG_DC_ADR
#define DC_DAT             CYGARC_REG_DC_DAT

#define DC_CMD_CE          CYGARC_REG_DC_CMD_CE
#define DC_CMD_CD          CYGARC_REG_DC_CMD_CD
#define DC_CMD_LL          CYGARC_REG_DC_CMD_LL
#define DC_CMD_UL          CYGARC_REG_DC_CMD_UL
#define DC_CMD_UA          CYGARC_REG_DC_CMD_UA
#define DC_CMD_IA          CYGARC_REG_DC_CMD_IA
#define DC_CMD_FL          CYGARC_REG_DC_CMD_FL
#define DC_CMD_SW          CYGARC_REG_DC_CMD_SW
#define DC_CMD_CW          CYGARC_REG_DC_CMD_CW
#define DC_CMD_SS          CYGARC_REG_DC_CMD_SS
#define DC_CMD_CS          CYGARC_REG_DC_CMD_CS

#define DC_ADR_SETID_SHIFT CYGARC_REG_DC_ADR_SETID_SHIFT
#define DC_ADR_WAY0        CYGARC_REG_DC_ADR_WAY0
#define DC_ADR_WAY1        CYGARC_REG_DC_ADR_WAY1
#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
// MMU control.
#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define M_CASID         793             // current address space id register

#define MI_CTR          784             // instruction MMU control
#define MI_EPN          787             // instruction MMU effective page num
#define MI_TWC          789             // instruction MMU tablewalk count
#define MI_RPN          790             // instruction MMU real page num
#define MI_DCAM         816             // instruction MMU CAM read
#define MI_DRAM0        817             // instruction MMU RAM read 0
#define MI_DRAM1        818             // instruction MMU RAM read 1

#define MI_EPN_EPNMASK  0xfffff000      // effective page no mask
#define MI_EPN_EV       0x00000200      // entry valid

#define MI_RPN_RPNMASK  0xfffff000      // real page no mask
#define MI_RPN_PPRWRW   0x000008f0      // page protection (rw/rw, page valid)
#define MI_RPN_LPS      0x0000000C      // large page size
#define MI_RPN_SH       0x00000004      // shared page (1 = no ASID cmp)
#define MI_RPN_CI       0x00000002      // cache inhibited
#define MI_RPN_V        0x00000001      // entry valid

#define MI_TWC_PS8MB    0x0000000c      // page size = 8MB
#define MI_TWC_G        0x00000010      // guarded
#define MI_TWC_WT       0x00000002      // writethrough
#define MI_TWC_V        0x00000001      // entry valid

#define MI_CTR_INDX_SHIFT 8             // the ITLB_INDX starts at bit 23

#define MD_CTR          792             // data MMU control
#define MD_EPN          795             // data MMU effective page num
#define MD_TWC          797             // data MMU tablewalk count
#define MD_RPN          798             // data MMU real page num
#define MD_DCAM         824             // data MMU CAM read
#define MD_DRAM0        825             // data MMU RAM read 0
#define MD_DRAM1        826             // data MMU RAM read 1

#define MD_RPN_CHANGED  0x00000100      // page changed

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#define CYGARC_REG_MI_CTR         784
#define CYGARC_REG_MI_CTR_CIDEF   0x20000000

#define CYGARC_REG_MD_CTR         792
#define CYGARC_REG_MD_CTR_CIDEF   0x20000000
#define CYGARC_REG_MD_CTR_WTDEF   0x10000000

//--------------------------------------------------------------------------
// Internal Memory Map.
#define CYGARC_REG_IMMR            638  // internal memory map base register
#define CYGARC_REG_IMMR_BASEMASK   0xffff0000 // imm base location mask (rw)
#define CYGARC_REG_IMMR_PARTNUM    0x0000ff00 // part number mask (ro)
#define CYGARC_REG_IMMR_MASKNUM    0x000000ff // mask number mask (ro)

#ifndef CYGARC_REG_IMM_BASE        // Can be defined by platform
#define CYGARC_REG_IMM_BASE        0xfa200000 // the internal memory map base
#endif

// CP Microcode Revision Number
#define CYGARC_REG_REV_NUM         ((CYGARC_REG_IMM_BASE)+0x3cb0)

// system protection control
#define CYGARC_REG_IMM_SYPCR       ((CYGARC_REG_IMM_BASE)+0x004)
#define CYGARC_REG_IMM_SYPCR_SWTC_MASK 0xffff0000
#define CYGARC_REG_IMM_SYPCR_BMT_MASK  0x0000ff00
#define CYGARC_REG_IMM_SYPCR_BME       0x00000080
#define CYGARC_REG_IMM_SYPCR_SWF       0x00000008
#define CYGARC_REG_IMM_SYPCR_SWE       0x00000004
#define CYGARC_REG_IMM_SYPCR_SWRI      0x00000002
#define CYGARC_REG_IMM_SYPCR_SWP       0x00000001

// interrupt pend register
#define CYGARC_REG_IMM_SIPEND      ((CYGARC_REG_IMM_BASE)+0x010)
#define CYGARC_REG_IMM_SIPEND_IRQ0 0x80000000 // irq0 is bit 0...

// interrupt mask
#define CYGARC_REG_IMM_SIMASK      ((CYGARC_REG_IMM_BASE)+0x014)
#define CYGARC_REG_IMM_SIMASK_IRQ0 0x80000000 // ... irq n is bit n*2

// interrupt edge level mask
#define CYGARC_REG_IMM_SIEL        ((CYGARC_REG_IMM_BASE)+0x018)
#define CYGARC_REG_IMM_SIEL_IRQ0   0x80000000

// interrupt vector
#define CYGARC_REG_IMM_SIVEC       ((CYGARC_REG_IMM_BASE)+0x01c)

// memory controller
#define CYGARC_REG_IMM_BR0         ((CYGARC_REG_IMM_BASE)+0x100)
#define CYGARC_REG_IMM_OR0         ((CYGARC_REG_IMM_BASE)+0x104)
#define CYGARC_REG_IMM_BR1         ((CYGARC_REG_IMM_BASE)+0x108)
#define CYGARC_REG_IMM_OR1         ((CYGARC_REG_IMM_BASE)+0x10c)
#define CYGARC_REG_IMM_BR2         ((CYGARC_REG_IMM_BASE)+0x110)
#define CYGARC_REG_IMM_OR2         ((CYGARC_REG_IMM_BASE)+0x114)
#define CYGARC_REG_IMM_BR3         ((CYGARC_REG_IMM_BASE)+0x118)
#define CYGARC_REG_IMM_OR3         ((CYGARC_REG_IMM_BASE)+0x11c)
#define CYGARC_REG_IMM_BR4         ((CYGARC_REG_IMM_BASE)+0x120)
#define CYGARC_REG_IMM_OR4         ((CYGARC_REG_IMM_BASE)+0x124)
#define CYGARC_REG_IMM_BR5         ((CYGARC_REG_IMM_BASE)+0x128)
#define CYGARC_REG_IMM_OR5         ((CYGARC_REG_IMM_BASE)+0x12c)
#define CYGARC_REG_IMM_BR6         ((CYGARC_REG_IMM_BASE)+0x130)
#define CYGARC_REG_IMM_OR6         ((CYGARC_REG_IMM_BASE)+0x134)
#define CYGARC_REG_IMM_BR7         ((CYGARC_REG_IMM_BASE)+0x138)
#define CYGARC_REG_IMM_OR7         ((CYGARC_REG_IMM_BASE)+0x13c)

#define CYGARC_REG_IMM_BR_BA_MASK  0xffff8000 // base address
#define CYGARC_REG_IMM_BR_AT_MASK  0x00007000 // address type
#define CYGARC_REG_IMM_BR_PS_8     0x00000400 // port size 8 bits
#define CYGARC_REG_IMM_BR_PS_16    0x00000800 // port size 16 bits
#define CYGARC_REG_IMM_BR_PS_32    0x00000000 // port size 32 bits
#define CYGARC_REG_IMM_BR_PARE     0x00000200 // parity enable 
#define CYGARC_REG_IMM_BR_WP       0x00000100 // write protect  
#define CYGARC_REG_IMM_BR_MS_GPCM  0x00000000 // machine select G.P.C.M
#define CYGARC_REG_IMM_BR_MS_UPMA  0x00000080 // machine select U.P.M.A
#define CYGARC_REG_IMM_BR_MS_UPMB  0x000000c0 // machine select U.P.M.B
#define CYGARC_REG_IMM_BR_V        0x00000001 // valid bit

#define CYGARC_REG_IMM_OR_AM     0xffff8000 // address mask
#define CYGARC_REG_IMM_OR_ATM    0x00007000 // address type mask
#define CYGARC_REG_IMM_OR_CSNT   0x00000800 // GPCM:chip select negation time
#define CYGARC_REG_IMM_OR_SAM    0x00000800 // UPMx:start address multiplex
#define CYGARC_REG_IMM_OR_ACS_0  0x00000000 // GPCM:CS output immediately
#define CYGARC_REG_IMM_OR_ACS_4  0x00000400 // GPCM:CS output 1/4 clock later
#define CYGARC_REG_IMM_OR_ACS_2  0x00000600 // GPCM:CS output 1/2 clock later
#define CYGARC_REG_IMM_OR_G5LA   0x00000400 // UPMx:general-purpose line 5 A
#define CYGARC_REG_IMM_OR_G5LS   0x00000200 // UPMx:general-purpose line 5 S
#define CYGARC_REG_IMM_OR_BI     0x00000100 // burst inhibit
#define CYGARC_REG_IMM_OR_SCY_MASK 0x000000f0 // cycle length in clocks
#define CYGARC_REG_IMM_OR_SCY_SHIFT 4
#define CYGARC_REG_IMM_OR_SETA     0x00000008 // external transfer ack
#define CYGARC_REG_IMM_OR_TRLX     0x00000004 // timing relaxed
#define CYGARC_REG_IMM_OR_EHTR     0x00000002 // extended hold time on read

// timebase status and control
#define CYGARC_REG_IMM_TBSCR       ((CYGARC_REG_IMM_BASE)+0x200) 
#define CYGARC_REG_IMM_TBSCR_REFA  0x0080 // reference interrupt status A
#define CYGARC_REG_IMM_TBSCR_REFB  0x0040 // reference interrupt status B
#define CYGARC_REG_IMM_TBSCR_REFAE 0x0008 // reference interrupt enable A
#define CYGARC_REG_IMM_TBSCR_REFBE 0x0004 // reference interrupt enable B
#define CYGARC_REG_IMM_TBSCR_TBF   0x0002 // timebase freeze
#define CYGARC_REG_IMM_TBSCR_TBE   0x0001 // timebase enable
#define CYGARC_REG_IMM_TBSCR_IRQ0  0x8000 // highest interrupt level
#define CYGARC_REG_IMM_TBSCR_IRQMASK 0xff00 // irq priority mask

// timebase reference register 0
#define CYGARC_REG_IMM_TBREF0      ((CYGARC_REG_IMM_BASE)+0x204)
// timebase reference register 1
#define CYGARC_REG_IMM_TBREF1      ((CYGARC_REG_IMM_BASE)+0x208)

// real time clock
#define CYGARC_REG_IMM_RTCSC       ((CYGARC_REG_IMM_BASE)+0x220)
#define CYGARC_REG_IMM_RTCSC_SEC   0x0080 // once per second interrupt
#define CYGARC_REG_IMM_RTCSC_ALR   0x0040 // alarm interrupt
#define CYGARC_REG_IMM_RTCSC_38K   0x0010 // source select
#define CYGARC_REG_IMM_RTCSC_SIE   0x0008 // second interrupt enable
#define CYGARC_REG_IMM_RTCSC_ALE   0x0004 // alarm interrupt enable
#define CYGARC_REG_IMM_RTCSC_RTF   0x0002 // real time clock freeze
#define CYGARC_REG_IMM_RTCSC_RTE   0x0001 // real time clock enable
#define CYGARC_REG_IMM_RTCSC_IRQ0  0x8000 // highest interrupt level
#define CYGARC_REG_IMM_RTCSC_IRQMASK 0xff00 // irq priority mask

// periodic interrupt status & ctrl
#define CYGARC_REG_IMM_PISCR       ((CYGARC_REG_IMM_BASE)+0x240)
#define CYGARC_REG_IMM_PISCR_PS    0x0080 // periodic interrupt status
#define CYGARC_REG_IMM_PISCR_PIE   0x0004 // periodic interrupt enable
#define CYGARC_REG_IMM_PISCR_PITF  0x0002 // periodic interrupt timer freeze
#define CYGARC_REG_IMM_PISCR_PTE   0x0001 // periodic timer enable
#define CYGARC_REG_IMM_PISCR_IRQ0  0x8000 // highest interrupt level
#define CYGARC_REG_IMM_PISCR_IRQMASK 0xff00 // irq priority mask

// periodic interrupt timer count
#define CYGARC_REG_IMM_PITC        ((CYGARC_REG_IMM_BASE)+0x244)
#define CYGARC_REG_IMM_PITC_COUNT_SHIFT 16 // count is stored in bits 0-15

// system clock control
#define CYGARC_REG_IMM_SCCR        ((CYGARC_REG_IMM_BASE)+0x280)
#define CYGARC_REG_IMM_SCCR_TBS    0x02000000 // timebase source
#define CYGARC_REG_IMM_SCCR_RTDIV  0x01000000 // rtc clock divide
#define CYGARC_REG_IMM_SCCR_RTSEL  0x00800000 // rtc clock select

// CPM interrupt vector register
#define CYGARC_REG_IMM_CIVR        ((CYGARC_REG_IMM_BASE)+0x930)
#define CYGARC_REG_IMM_CIVR_IACK   0x0001 // set this to update register
#define CYGARC_REG_IMM_CIVR_VECTOR_SHIFT 11 // vector is at bits 0-4

// CPM interrupt configuration reg
#define CYGARC_REG_IMM_CICR        ((CYGARC_REG_IMM_BASE)+0x940)
#define CYGARC_REG_IMM_CICR_IEN    0x00000080      // interrupt enable
#define CYGARC_REG_IMM_CICR_IRQMASK 0x0000e000     // irq priority mask
#define CYGARC_REG_IMM_CICR_IRQ_SHIFT 13

// CPM interrupt in-pending register
#define CYGARC_REG_IMM_CIPR        ((CYGARC_REG_IMM_BASE)+0x944)
// CPM interrupt mask register
#define CYGARC_REG_IMM_CIMR        ((CYGARC_REG_IMM_BASE)+0x948)
// CPM interrupt in-service register
#define CYGARC_REG_IMM_CISR        ((CYGARC_REG_IMM_BASE)+0x94C)


#define CYGARC_SIU_PRIORITY_LOW    7 // the lowest irq priority
#define CYGARC_SIU_PRIORITY_HIGH   0 // the highest irq priority

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

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
#endif // ifdef CYGONCE_HAL_VAR_REGS_H
// End of var_regs.h
