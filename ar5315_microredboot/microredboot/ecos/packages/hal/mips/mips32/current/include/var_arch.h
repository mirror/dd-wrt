#ifndef CYGONCE_HAL_VAR_ARCH_H
#define CYGONCE_HAL_VAR_ARCH_H

//==========================================================================
//
//      var_arch.h
//
//      Architecture specific abstractions
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
// Author(s):    nickg
// Contributors: nickg, dmoseley
// Date:         1999-02-17
// Purpose:      Define architecture abstractions
// Description:  This file contains any extra or modified definitions for
//               this variant of the architecture.
// Usage:        #include <cyg/hal/var_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef __ASSEMBLER__
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#endif

//--------------------------------------------------------------------------
// Define macros for accessing CP0 registers

#define HAL_GET_CP0_REGISTER_32( _regval_, _cp0_regno_, _cp0_regsel_ )  \
{                                                                       \
    cyg_uint32 tmp;                                                     \
    asm volatile ("mfc0   %0,$%1,%2\nnop\n"                             \
	           : "=r" (tmp)                                         \
	           : "i"  (_cp0_regno_), "i"  (_cp0_regsel_)  );        \
    _regval_ = tmp;                                                     \
}

#define HAL_SET_CP0_REGISTER_32( _regval_, _cp0_regno_, _cp0_regsel_ )          \
{                                                                               \
    cyg_uint32 tmp = _regval_;                                                  \
    asm volatile ("mtc0   %1,$%2,%3\nnop\n"                                     \
	           : "=r" (tmp)                                                 \
	           : "r" (tmp), "i"  (_cp0_regno_), "i" (_cp0_regsel_) );       \
}

#define HAL_GET_CP0_REGISTER_64( _regval_, _cp0_regno_, _cp0_regsel_ ) \
        HAL_GET_CP0_REGISTER_32( _regval_, _cp0_regno_, _cp0_regsel_ )
#define HAL_SET_CP0_REGISTER_64( _regval_, _cp0_regno_, _cp0_regsel_ ) \
        HAL_SET_CP0_REGISTER_32( _regval_, _cp0_regno_, _cp0_regsel_ )

//--------------------------------------------------------------------------

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
/* System Control Coprocessor (CP0) exception processing registers */
/* These supplement the definitions in mips-regs.h */
#define C0_INDEX        $0              /* Index into TLB Array - 4Kc core */
#define C0_RANDOM       $1              /* Randomly generated index into TLB Array - 4Kc core */
#define C0_ENTRYLO0     $2              /* Low-order portion of the TLB entry for even-numbered virtual pages - 4Kc core */
#define C0_ENTRYLO1     $3              /* Low-order portion of the TLB entry for odd-numbered virtual pages - 4Kc core */
#define CO_PAGEMASK     $5              /* Pointer to page table entry in memory - 4Kc core */
#define C0_WIRED        $6              /* Number of fixed TLB entries - 4Kc core */
#define C0_ENTRYHI      $10             /* High-order portion of the TLB entry - 4Kc core */
#define C0_PRId         $15             /* Processor Identification and Revision */
#define C0_CONFIG       $16             /* Configuration Register */
#define C0_LLADDR       $17             /* Load linked address */
#define C0_LLADDR       $17             /* Load linked address */
#define C0_DEBUG        $23             /* Debug control and exception status */
#define C0_DEPC         $24             /* Program counter at last debug exception */
#define C0_TAGLO        $28             /* Low-order portion of cache tag interface */
#define C0_TAGHI        $29             /* High-order portion of cache tag interface (not implemented in 4K cores */
#define C0_DESAVE       $31             /* Debug handler scratch pad register */

/* Coprocessor Register selector field */
#define C0_SELECTOR_0   0x0
#define C0_SELECTOR_1   0x1

/* Status register fields */
#define SR_RP           0x08000000      /* Enter reduced-power mode */
#define SR_NMI          0x00080000      /* Reset vector called through assertion of the NMI signal */

/* Cause register fields */
#define CAUSE_IV        0x00800000      /* Interrupt vector to use -- Bit=0 -> offset=0x180;
                                                                      Bit=1 -> offset=0x200; */
#define CAUSE_WP        0x00400000      /* Watch exception deferred due to either Status[EXL] or Status[ERL] */
#define CAUSE_MIPS32IP7 CAUSE_IP8       /* The MIPS32 architecture refers to these bits using a 0 base, */
#define CAUSE_MIPS32IP6 CAUSE_IP7       /* but the generic mips-regs.h refers to them with a 1 base */
#define CAUSE_MIPS32IP5 CAUSE_IP6
#define CAUSE_MIPS32IP4 CAUSE_IP5
#define CAUSE_MIPS32IP3 CAUSE_IP4
#define CAUSE_MIPS32IP2 CAUSE_IP3
#define CAUSE_MIPS32IP1 CAUSE_IP2
#define CAUSE_MIPS32IP0 CAUSE_IP1

#define CAUSE_MIPS32HW5 CAUSE_MIPS32IP1
#define CAUSE_MIPS32HW4 CAUSE_MIPS32IP1
#define CAUSE_MIPS32HW3 CAUSE_MIPS32IP1
#define CAUSE_MIPS32HW2 CAUSE_MIPS32IP1
#define CAUSE_MIPS32HW1 CAUSE_MIPS32IP1
#define CAUSE_MIPS32HW0 CAUSE_MIPS32IP1
#define CAUSE_MIPS32SW1 CAUSE_MIPS32IP1
#define CAUSE_MIPS32SW0 CAUSE_MIPS32IP0

/* Exception Codes */
#define EXC_WATCH       23              /* Reference to the Watch address */
#define EXC_MCHECK      24              /* Machine Check */

/* Processor Identification fields */
#define PRId_COMPANY_ID_MASK              0x00FF0000  /* Which company manufactured this chip */
#define PRId_COMPANY_MIPS_TECHNOLOGIES    0x00010000
#define PRId_PROCESSOR_ID_MASK            0x0000FF00  /* Which processor is this */
#define PRId_PROCESSOR_4Kc                0x00008000
#define PRId_PROCESSOR_4Kp_4Km            0x00008300
#define PRId_REVISION                     0x000000FF /* Which revision is this */

/* Config register fields */
#define CONFIG_M                          0x80000000 /* Hardwired to '1' to indicate presence of Config1 register */
#define CONFIG_K23                        0x70000000 /* Controls cacheability of kseg2 and kseg3 in BAT */
#define CONFIG_KU                         0x0E000000 /* Controls cacheability of ksegu in BAT */
#define CONFIG_MDU                        0x00100000 /* MDU Type: 0 == Fast Multiplier Array; 1 == Iterative */
#define CONFIG_MM                         0x00060000 /* Merge mode */
#define CONFIG_BM                         0x00010000 /* Burst mode: 0 == Sequential; 1 == SubBlock */
#define CONFIG_BE                         0x00008000 /* Endian mode: 0 == Little Endian; 1 == Big Endian */
#define CONFIG_AT                         0x00006000 /* Architecture Type */
#define CONFIG_AR                         0x00001C00 /* Architecture Revision */
#define CONFIG_MT                         0x00000380 /* MMU Type */
#define CONFIG_K0                         0x00000007 /* kseg0 coherency algorithm */

/* KSEG cache control codes */
#define CONFIG_KSEG2_3_CACHEABLE          0x30000000 /* KSeg2 and KSeg3 are cacheable/noncoherent/write-through/no write-allocate */
#define CONFIG_KSEG2_3_UNCACHEABLE        0x20000000 /* KSeg2 and KSeg3 are cacheable/noncoherent/write-through/no write-allocate */
#define CONFIG_KSEGU_CACHEABLE            0x06000000 /* KSegu is cacheable/noncoherent/write-through/no write-allocate */
#define CONFIG_KSEGU_UNCACHEABLE          0x04000000 /* KSegu is cacheable/noncoherent/write-through/no write-allocate */
#define CONFIG_KSEG0_CACHEABLE            0x00000003 /* KSeg0 is cacheable/noncoherent/write-through/no write-allocate */
#define CONFIG_KSEG0_UNCACHEABLE          0x00000002 /* KSeg0 is cacheable/noncoherent/write-through/no write-allocate */

/* Merge mode control codes */
#define CONFIG_NO_MERGING                 0x00000000
#define CONFIG_SYSAD_VALID_MERGING        0x00200000
#define CONFIG_FULL_MERGING               0x00400000

/* Architecture Type codes */
#define CONFIG_AT_MIPS32                  0x00000000

/* Architecture Revision codes */
#define CONFIG_AR_REVISION_1              0x00000000

/* MMU Type codes */
#define CONFIG_MMU_TYPE_STANDARD_TLB      0x00000080
#define CONFIG_MMU_TYPE_FIXED             0x00000180

/* Config1 register fields */
#define CONFIG1_MMU_SIZE_MASK             0x7E000000 /* Number of entries in the TLB minus 1 */
#define CONFIG1_IS                        0x01C00000 /* Number of instruction cache sets per way */
#define CONFIG1_IL                        0x00380000 /* Instruction cache line size */
#define CONFIG1_IA                        0x00030000 /* Level of Instruction cache associativity */
#define CONFIG1_DS                        0x0000E000 /* Number of data cache sets per way */
#define CONFIG1_DL                        0x00001C00 /* Data cache line size */
#define CONFIG1_DA                        0x00000380 /* Level of Data cache associativity */
#define CONFIG1_PC                        0x00000010 /* Performance Counter registers implemented */
#define CONFIG1_WR                        0x00000008 /* Watch registers implemented */
#define CONFIG1_CA                        0x00000004 /* Code compression implemented */
#define CONFIG1_EP                        0x00000002 /* EJTAG implemented */
#define CONFIG1_FP                        0x00000001 /* FPU implemented */

/* Instruction cache sets-per-way codes */
#define CONFIG1_ICACHE_64_SETS_PER_WAY    0x00000000
#define CONFIG1_ICACHE_128_SETS_PER_WAY   0x00400000
#define CONFIG1_ICACHE_256_SETS_PER_WAY   0x00800000

/* Instruction cache line size codes */
#define CONFIG1_ICACHE_NOT_PRESET         0x00000000
#define CONFIG1_ICACHE_LINE_SIZE_16_BYTES 0x00180000

/* Instruction cache associativity codes */
#define CONFIG1_ICACHE_DIRECT_MAPPED      0x00000000
#define CONFIG1_ICACHE_2_WAY              0x00010000
#define CONFIG1_ICACHE_3_WAY              0x00020000
#define CONFIG1_ICACHE_4_WAY              0x00030000

/* Data cache sets-per-way codes */
#define CONFIG1_DCACHE_64_SETS_PER_WAY    0x00000000
#define CONFIG1_DCACHE_128_SETS_PER_WAY   0x00002000
#define CONFIG1_DCACHE_256_SETS_PER_WAY   0x00004000

/* Data cache line size codes */
#define CONFIG1_DCACHE_NOT_PRESET         0x00000000
#define CONFIG1_DCACHE_LINE_SIZE_16_BYTES 0x00000C00

/* Data cache associativity codes */
#define CONFIG1_DCACHE_DIRECT_MAPPED      0x00000000
#define CONFIG1_DCACHE_2_WAY              0x00000080
#define CONFIG1_DCACHE_3_WAY              0x00000100
#define CONFIG1_DCACHE_4_WAY              0x00000180

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
