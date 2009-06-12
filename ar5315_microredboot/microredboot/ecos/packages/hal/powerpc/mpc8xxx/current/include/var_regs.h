#ifndef CYGONCE_HAL_VAR_REGS_H
#define CYGONCE_HAL_VAR_REGS_H

//==========================================================================
//
//      var_regs.h
//
//      PowerPC MPC8xxx CPU definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    pfine
// Contributors: jskov
// Date:         2001-12-12
// Purpose:      Provide MPC8260 register definitions
// Description:  Provide MPC8260 register definitions
//               The short definitions (sans CYGARC_REG_) are exported only
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
#define CYGARC_REG_LR   8              // Link Register
#define CYGARC_REG_CTR   9              // Counter Register

#define CYGARC_REG_DSISR  18
#define CYGARC_REG_DAR    19
#define CYGARC_REG_DEC    22
#define CYGARC_REG_SDR1   25

#define CYGARC_REG_TBL  268
#define CYGARC_REG_TBU  269

#define CYGARC_REG_SPRG0  272
#define CYGARC_REG_SPRG1  273
#define CYGARC_REG_SPRG2  274
#define CYGARC_REG_SPRG3  275
#define CYGARC_REG_EAR    282
#define CYGARC_REG_PVR    287

#define CYGARC_REG_IBAT0U          528
#define CYGARC_REG_IBAT0L          529
#define CYGARC_REG_IBAT1U          530
#define CYGARC_REG_IBAT1L          531
#define CYGARC_REG_IBAT2U          532
#define CYGARC_REG_IBAT2L          533
#define CYGARC_REG_IBAT3U          534
#define CYGARC_REG_IBAT3L          535

#define CYGARC_REG_DBAT0U          536
#define CYGARC_REG_DBAT0L          537
#define CYGARC_REG_DBAT1U          538
#define CYGARC_REG_DBAT1L          539
#define CYGARC_REG_DBAT2U          540
#define CYGARC_REG_DBAT2L          541
#define CYGARC_REG_DBAT3U          542
#define CYGARC_REG_DBAT3L          543

#define CYGARC_REG_DMISS   976
#define CYGARC_REG_DCMP    977
#define CYGARC_REG_HASH1   978
#define CYGARC_REG_HASH2   979
#define CYGARC_REG_IMISS   980
#define CYGARC_REG_ICMP    981
#define CYGARC_REG_RPA     982


// Hardware Implementation Defined Special Purpose Registers
#define CYGARC_REG_HID0   1008
#define CYGARC_REG_HID1   1009
#define CYGARC_REG_IABR   1010
#define CYGARC_REG_HID2   1011
#define CYGARC_REG_DABR   1013

// MPC8260 Internal Memory Mapped Registers
// These values are the offsets from the base memory address, which
// is stored in the IMMR register (0x101A8).
#define CYGARC_REG_IMM_SIUMCR    0x0000   // SIU Module Configuration Register
#define CYGARC_REG_IMM_SYPCR     0x0004   // System Protection Control Register
#define CYGARC_REG_IMM_SWSR      0x000E   // Software Service Register
#define CYGARC_REG_IMM_BCR       0x0024   // Bus Configuration Register
#define CYGARC_REG_IMM_PPC_ACR   0x0028   // .60x Bus Arbiter Config Register
#define CYGARC_REG_IMM_PPC_ALRH  0x002C   // .60x Bus Arb-Level[High] Register
#define CYGARC_REG_IMM_PPC_ALRL  0x0030   // .60x Bus Arb-Level[Low] Register
#define CYGARC_REG_IMM_LCL_ACR   0x0034   // Local Arbiter Config Register
#define CYGARC_REG_IMM_LCL_ALRH  0x0038   // Local Arb-Level[High] Register
#define CYGARC_REG_IMM_LCL_ALRL  0x003C   // Local Arb-Level[Low] Register
#define CYGARC_REG_IMM_TESCR1    0x0040   // .60x Bus Transfer Error Status and
                                          //    Control Register 1
#define CYGARC_REG_IMM_TESCR2    0x0044   // .60x Bus Transfer Error Status and
                                          //    Control Register 2

#define CYGARC_REG_IMM_LTESCR1   0x0048   // Local Bus Transfer Error Status
                                          //    and Control Register 1
#define CYGARC_REG_IMM_LTESCR2   0x004C   // Local Bus Transfer Error Status and
                                          // //Control Register 2

#define CYGARC_REG_IMM_PDTEA     0x0050   // .60x Bus DMA Transfer
                                          //    Error Address
#define CYGARC_REG_IMM_PDTEM     0x0054   // .60x Bus DMA Transfer Error MSNUM
#define CYGARC_REG_IMM_LDTEA     0x0058   // Local Bus DMA Xfer Error Address
#define CYGARC_REG_IMM_LDTEM     0x005C   // Local Bus DMA Transfer Error MSNUM
 
#define CYGARC_REG_IMM_SCCR      0x0C80   // System Clock Control Register
#define CYGARC_REG_IMM_SCMR      0x0C88   // System Clock Mode Register
#define CYGARC_REG_IMM_BR0       0x0100   // Base Register Bank 0
#define CYGARC_REG_IMM_OR0       0x0104   // Option Register Bank 0
#define CYGARC_REG_IMM_BR1       0x0108   // Base Register Bank 1
#define CYGARC_REG_IMM_OR1       0x010C   // Option Register Bank 1
#define CYGARC_REG_IMM_BR2       0x0110   // Base Register Bank 2
#define CYGARC_REG_IMM_OR2       0x0114   // Option Regiser Bank 2
#define CYGARC_REG_IMM_BR3       0x0118   // Base Register Bank 3
#define CYGARC_REG_IMM_OR3       0x011C   // Option Register Bank 3
#define CYGARC_REG_IMM_BR4       0x0120   // Base Register Bank 4
#define CYGARC_REG_IMM_OR4       0x0124   // Option Register Bank 4
#define CYGARC_REG_IMM_BR5       0x0128   // Base Register Bank 5
#define CYGARC_REG_IMM_OR5       0x012C   // Option Register Bank 5
#define CYGARC_REG_IMM_BR6       0x0130   // Base Register Bank 6
#define CYGARC_REG_IMM_OR6       0x0134   // Option Register Bank 6
#define CYGARC_REG_IMM_BR7       0x0138   // Base Register Bank 7
#define CYGARC_REG_IMM_OR7       0x013C   // Option Register Bank 7
#define CYGARC_REG_IMM_BR8       0x0140   // Base Register Bank 8
#define CYGARC_REG_IMM_OR8       0x0144   // Option Regiser Bank 8
#define CYGARC_REG_IMM_BR9       0x0148   // Base Register Bank 9
#define CYGARC_REG_IMM_OR9       0x014C   // Option Register Bank 9
#define CYGARC_REG_IMM_BR10      0x0150   // Base Register Bank 10
#define CYGARC_REG_IMM_OR10      0x0154   // Option Register Bank 10
#define CYGARC_REG_IMM_BR11      0x0158   // Base Register Bank 11
#define CYGARC_REG_IMM_OR11      0x015C   // Option Register Bank 11

#define CYGARC_REG_IMM_MAR       0x0168   // Memory Address Register
#define CYGARC_REG_IMM_MAMR      0x0170   // Machine A mode Register
#define CYGARC_REG_IMM_MBMR      0x0174   // Machine B mode Register
#define CYGARC_REG_IMM_MCMR      0x0178   // Machine C mode Register

#define CYGARC_REG_IMM_MPTPR     0x0184   // Memory Periodic Timer
                                          //    Prescaler Register
#define CYGARC_REG_IMM_MDR       0x0188   // Memory Data Register
#define CYGARC_REG_IMM_PSDMR     0x0190   // PowerPC Bus SDRAM Machine
                                          //    Mode Register
#define CYGARC_REG_IMM_LSDMR     0x0194   // Local Bus SDRAM Machine
                                          //    Mode Register
#define CYGARC_REG_IMM_PURT      0x0198   // .60x Bus-assigned UPM Refresh timer
#define CYGARC_REG_IMM_PSRT      0x019C   // .60x Bus Assigned SDRAM
                                          //    Refresh Timer
#define CYGARC_REG_IMM_LURT      0x01A0   // Local Bus-assigned UPM
                                          //    Refresh timer
#define CYGARC_REG_IMM_LSRT      0x01A4   // Local Bus Assigned SDRAM
                                          //    Refresh Timer
#define CYGARC_REG_IMM_IMMR      0x01A8   // Internal I/O base register offset

// Interrupt Controller
#define CYGARC_REG_IMM_SICR      0x0C00   // SIU Interrupt Config Register
#define CYGARC_REG_IMM_SIVEC     0x0C04   // SIU Interrupt Vector Register
#define CYGARC_REG_IMM_SIPNR_H   0x0C08   // SIU Interrupt Pending Reg. High
#define CYGARC_REG_IMM_SIPNR_L   0x0C0C   // SIU Interrupt Pending Reg. Low
#define CYGARC_REG_IMM_SIPRR     0x0C10   // SIU Interrupt Priority Register
#define CYGARC_REG_IMM_SCPRR_H   0x0C14   // CPM Interrupt Priority Reg. High
#define CYGARC_REG_IMM_SCPRR_L   0x0C18   // CPM Interrupt Priority Reg. Low
#define CYGARC_REG_IMM_SIMR_H    0x0C1C   // SIU Interrupt Mask Register High
#define CYGARC_REG_IMM_SIMR_L    0x0C20   // SIU Interrupt Mask Register High
#define CYGARC_REG_IMM_SIEXR     0x0C24   // SIU External Interrupt Ctrl Reg.

// Parallel I/O (GPIO) 
#define CYGARC_REG_IMM_PDIRA     0x0D00   // Port A data direction
#define CYGARC_REG_IMM_PPARA     0x0D04   // Port A pin assignment
#define CYGARC_REG_IMM_PSORA     0x0D08   // Port A special options
#define CYGARC_REG_IMM_PODRA     0x0D0C   // Port A open drain
#define CYGARC_REG_IMM_PDATA     0x0D10   // Port A data
#define CYGARC_REG_IMM_PDIRB     0x0D20   // Port B data direction
#define CYGARC_REG_IMM_PPARB     0x0D24   // Port B pin assignment
#define CYGARC_REG_IMM_PSORB     0x0D28   // Port B special options
#define CYGARC_REG_IMM_PODRB     0x0D2C   // Port A open drain
#define CYGARC_REG_IMM_PDATB     0x0D30   // Port A data
#define CYGARC_REG_IMM_PDIRC     0x0D40   // Port C data direction
#define CYGARC_REG_IMM_PPARC     0x0D44   // Port C pin assignment
#define CYGARC_REG_IMM_PSORC     0x0D48   // Port C special options
#define CYGARC_REG_IMM_PODRC     0x0D4C   // Port A open drain
#define CYGARC_REG_IMM_PDATC     0x0D50   // Port A data
#define CYGARC_REG_IMM_PDIRD     0x0D60   // Port D data direction
#define CYGARC_REG_IMM_PPARD     0x0D64   // Port D pin assignment
#define CYGARC_REG_IMM_PSORD     0x0D68   // Port D special options
#define CYGARC_REG_IMM_PODRD     0x0D6C   // Port A open drain
#define CYGARC_REG_IMM_PDATD     0x0D70   // Port A data

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define HID0       CYGARC_REG_HID0
#define HID1       CYGARC_REG_HID1
#define HID2       CYGARC_REG_HID2
#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

// BATs
#define IBAT0U          528
#define IBAT0L          529
#define IBAT1U          530
#define IBAT1L          531
#define IBAT2U          532
#define IBAT2L          533
#define IBAT3U          534
#define IBAT3L          535

#define DBAT0U          536
#define DBAT0L          537
#define DBAT1U          538
#define DBAT1L          539
#define DBAT2U          540
#define DBAT2L          541
#define DBAT3U          542
#define DBAT3L          543

#define UBAT_BEPIMASK   0xfffe0000      // effective address mask
#define UBAT_BLMASK     0x00001ffc      // block length mask
#define UBAT_VS         0x00000002      // supervisor mode valid bit
#define UBAT_VP         0x00000001      // problem mode valid bit

#define LBAT_BRPNMASK   0xfffe0000      // real address mask
#define LBAT_W          0x00000040      // write-through
#define LBAT_I          0x00000020      // caching-inhibited
#define LBAT_M          0x00000010      // memory coherence
#define LBAT_G          0x00000008      // guarded

#define LBAT_PP_NA      0x00000000      // no access
#define LBAT_PP_RO      0x00000001      // read-only
#define LBAT_PP_RW      0x00000002      // read/write


#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//-----------------------------------------------------------------------------
#endif // ifdef CYGONCE_HAL_VAR_REGS_H
// End of var_regs.h
