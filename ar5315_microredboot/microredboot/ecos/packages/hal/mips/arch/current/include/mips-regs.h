#ifndef CYGONCE_HAL_MIPS_REGS_H
#define CYGONCE_HAL_MIPS_REGS_H
//========================================================================
//
//      mips-regs.h
//
//      Register defines for MIPS processors
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, nickg
// Contributors:  Red Hat, nickg, dmoseley
// Date:          1998-06-08
// Purpose:       
// Description:   Register defines for MIPS processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/hal.h>

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

/* This value must agree with NUMREGS in mips-stub.h. */

#if defined(CYGPKG_HAL_MIPS_GDB_REPORT_CP0)
#define NUM_REGS   107
#else
#define NUM_REGS    90
#endif

#ifdef __mips64
  #define REG_SIZE 8
#else
  #define REG_SIZE 4
#endif

/* General register names for assembly code. */

#define zero            $0
#define at              $1              /* assembler temporary */
#define atmp            $1              /* assembler temporary */
#define v0              $2              /* value holders */
#define v1              $3
#define a0              $4              /* arguments */
#define a1              $5
#define a2              $6
#define a3              $7
#define t0              $8              /* temporaries */
#define t1              $9
#define t2              $10
#define t3              $11
#define t4              $12
#define t5              $13
#define t6              $14
#define t7              $15
#define s0              $16             /* saved registers */
#define s1              $17
#define s2              $18
#define s3              $19
#define s4              $20
#define s5              $21
#define s6              $22
#define s7              $23
#define t8              $24             /* temporaries */
#define t9              $25
#define k0              $26             /* kernel registers */
#define k1              $27
#define gp              $28             /* global pointer */
#define sp              $29             /* stack pointer */
#define s8              $30             /* saved register */
#define fp              $30             /* frame pointer (obsolete usage) */
#define ra              $31             /* return address */

/* MIPS registers, numbered in the order in which gdb expects to see them. */
#define ZERO            0
#define AT              1
#define ATMP            1
#define V0              2
#define V1              3
#define A0              4
#define A1              5
#define A2              6
#define A3              7

#define T0              8
#define T1              9
#define T2              10
#define T3              11
#define T4              12
#define T5              13
#define T6              14
#define T7              15

#define S0              16
#define S1              17
#define S2              18
#define S3              19
#define S4              20
#define S5              21
#define S6              22
#define S7              23

#define T8              24
#define T9              25
#define K0              26
#define K1              27
#define GP              28
#define SP              29
#define S8              30
#define RA              31

#define SR              32
#define LO              33
#define HI              34
#define BAD_VA          35
#define CAUSE           36
#define PC              37

#define F0              38
#define F1              39
#define F2              40
#define F3              41
#define F4              42
#define F5              43
#define F6              44
#define F7              45
#define F8              46
#define F9              47
#define F10             48
#define F11             49
#define F12             50
#define F13             51
#define F14             52
#define F15             53
#define F16             54
#define F17             55
#define F18             56
#define F19             57
#define F20             58
#define F21             59
#define F22             60
#define F23             61
#define F24             62
#define F25             63
#define F26             64
#define F27             65
#define F28             66
#define F29             67
#define F30             68
#define F31             69

#define FCR31           70

/* System Control Coprocessor (CP0) exception processing registers */
#define C0_CONTEXT      $4              /* Context */
#define C0_BADVADDR     $8              /* Bad Virtual Address */
#define C0_COUNT        $9              /* Count */
#define C0_COMPARE      $11             /* Compare */
#define C0_STATUS       $12             /* Processor Status */
#define C0_CAUSE        $13             /* Exception Cause */
#define C0_EPC          $14             /* Exception PC */
#define C0_WATCHLO      $18             /* Watchpoint LO */
#define C0_WATCHHI      $19             /* Watchpoint HI */
#define C0_XCONTEXT     $20             /* XContext */
#define C0_ECC          $26             /* ECC */
#define C0_CACHEERR     $27             /* CacheErr */
#define C0_ERROREPC     $30             /* ErrorEPC */

/* Status register fields */
#define SR_CUMASK       0xf0000000      /* Coprocessor usable bits */
#define SR_CU3          0x80000000      /* Coprocessor 3 usable */
#define SR_CU2          0x40000000      /* coprocessor 2 usable */
#define SR_CU1          0x20000000      /* Coprocessor 1 usable */
#define SR_CU0          0x10000000      /* Coprocessor 0 usable */

#define SR_FR           0x04000000      /* Enable 32 floating-point registers */
#define SR_RE           0x02000000      /* Reverse Endian in user mode */

#define SR_BEV          0x00400000      /* Bootstrap Exception Vector */
#define SR_TS           0x00200000      /* TLB shutdown (reserved on R4600) */
#define SR_SR           0x00100000      /* Soft Reset */

#define SR_CH           0x00040000      /* Cache Hit */
#define SR_CE           0x00020000      /* ECC register modifies check bits */
#define SR_DE           0x00010000      /* Disable cache errors */

#define SR_IMASK        0x0000ff00      /* Interrupt Mask */
#define SR_IMASK8       0x00000000      /* Interrupt Mask level=8 */
#define SR_IMASK7       0x00008000      /* Interrupt Mask level=7 */
#define SR_IMASK6       0x0000c000      /* Interrupt Mask level=6 */
#define SR_IMASK5       0x0000e000      /* Interrupt Mask level=5 */
#define SR_IMASK4       0x0000f000      /* Interrupt Mask level=4 */
#define SR_IMASK3       0x0000f800      /* Interrupt Mask level=3 */
#define SR_IMASK2       0x0000fc00      /* Interrupt Mask level=2 */
#define SR_IMASK1       0x0000fe00      /* Interrupt Mask level=1 */
#define SR_IMASK0       0x0000ff00      /* Interrupt Mask level=0 */

#define SR_IBIT8        0x00008000      /*  (Intr5) */
#define SR_IBIT7        0x00004000      /*  (Intr4) */
#define SR_IBIT6        0x00002000      /*  (Intr3) */
#define SR_IBIT5        0x00001000      /*  (Intr2) */
#define SR_IBIT4        0x00000800      /*  (Intr1) */
#define SR_IBIT3        0x00000400      /*  (Intr0) */
#define SR_IBIT2        0x00000200      /*  (Software Interrupt 1) */
#define SR_IBIT1        0x00000100      /*  (Software Interrupt 0) */

#define SR_KX           0x00000080      /* xtlb in kernel mode */
#define SR_SX           0x00000040      /* mips3 & xtlb in supervisor mode */
#define SR_UX           0x00000020      /* mips3 & xtlb in user mode */

#define SR_KSU_MASK     0x00000018      /* ksu mode mask */
#define SR_KSU_USER     0x00000010      /* user mode */
#define SR_KSU_SUPV     0x00000008      /* supervisor mode */
#define SR_KSU_KERN     0x00000000      /* kernel mode */

#define SR_ERL          0x00000004      /* error level */
#define SR_EXL          0x00000002      /* exception level */
#define SR_IE           0x00000001      /* interrupt enable */

/* Floating-point unit control/status register (FCR31) */
#define FCR31_FS        0x01000000      /* Flush denormalized to zero */
#define FCR31_C         0x00800000      /* FP compare result */

#define FCR31_CAUSE_E   0x00020000      /* Cause - unimplemented operation */
#define FCR31_CAUSE_V   0x00010000      /* Cause - invalid operation */
#define FCR31_CAUSE_Z   0x00008000      /* Cause - division by zero */
#define FCR31_CAUSE_O   0x00004000      /* Cause - overflow */
#define FCR31_CAUSE_U   0x00002000      /* Cause - underflow */
#define FCR31_CAUSE_I   0x00001000      /* Cause - inexact operation */

#define FCR31_ENABLES_V 0x00000800      /* Enables - invalid operation */
#define FCR31_ENABLES_Z 0x00000400      /* Enables - division by zero */
#define FCR31_ENABLES_O 0x00000200      /* Enables - overflow */
#define FCR31_ENABLES_U 0x00000100      /* Enables - underflow */
#define FCR31_ENABLES_I 0x00000080      /* Enables - inexact operation */

#define FCR31_FLAGS_V   0x00000040      /* Flags - invalid operation */
#define FCR31_FLAGS_Z   0x00000020      /* Flags - division by zero */
#define FCR31_FLAGS_O   0x00000010      /* Flags - overflow */
#define FCR31_FLAGS_U   0x00000008      /* Flags - underflow */
#define FCR31_FLAGS_I   0x00000004      /* Flags - inexact operation */

#define FCR31_RMMASK    0x00000002      /* Rounding mode mask */
#define FCR31_RM_RN     0               /* Round to nearest */
#define FCR31_RM_RZ     1               /* Round to zero */
#define FCR31_RM_RP     2               /* Round to +infinity */
#define FCR31_RM_RM     3               /* Round to -infinity */


/* Cause register fields */
#define CAUSE_BD        0x80000000      /* Branch Delay */
#define CAUSE_CEMASK    0x30000000      /* Coprocessor Error */
#define CAUSE_CESHIFT   28              /* Right justify CE  */
#define CAUSE_IPMASK    0x0000ff00      /* Interrupt Pending */
#define CAUSE_IPSHIFT   8               /* Right justify IP  */
#define CAUSE_IP8       0x00008000      /*  (Intr5) */
#define CAUSE_IP7       0x00004000      /*  (Intr4) */
#define CAUSE_IP6       0x00002000      /*  (Intr3) */
#define CAUSE_IP5       0x00001000      /*  (Intr2) */
#define CAUSE_IP4       0x00000800      /*  (Intr1) */
#define CAUSE_IP3       0x00000400      /*  (Intr0) */
#define CAUSE_SW2       0x00000200      /*  (Software Interrupt 1) */
#define CAUSE_SW1       0x00000100      /*  (Software Interrupt 0) */
#define CAUSE_EXCMASK   0x0000007c      /* Exception Code */
#define CAUSE_EXCSHIFT  2               /* Right justify EXC */

/* Exception Codes */
#define EXC_INT         0               /* External interrupt */
#define EXC_MOD         1               /* TLB modification exception */
#define EXC_TLBL        2               /* TLB miss (Load or Ifetch) */
#define EXC_TLBS        3               /* TLB miss (Store) */
#define EXC_ADEL        4               /* Address error (Load or Ifetch) */
#define EXC_ADES        5               /* Address error (Store) */
#define EXC_IBE         6               /* Bus error (Ifetch) */
#define EXC_DBE         7               /* Bus error (data load or store) */
#define EXC_SYS         8               /* System call */
#define EXC_BP          9               /* Break point */
#define EXC_RI          10              /* Reserved instruction */
#define EXC_CPU         11              /* Coprocessor unusable */
#define EXC_OVF         12              /* Arithmetic overflow */
#define EXC_TRAP        13              /* Trap exception */
#define EXC_FPE         15              /* Floating Point Exception */

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#endif // ifndef CYGONCE_HAL_MIPS_REGS_H
