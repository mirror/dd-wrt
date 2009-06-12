#ifndef __ARM_CPU_H__
#define __ARM_CPU_H__
//==========================================================================
//
//      cpu.h
//
//      ARM specific processor defines
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
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      ARM specific processor defines 
// Description:  ARM is a Registered Trademark of Advanced RISC Machines
//               Limited.
//               Other Brands and Trademarks are the property of their
//               respective owners.               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include <bsp/bsp.h>
#include <bsp/defs.h>
#ifdef __ECOS__
#include <cyg/hal/hal_arch.h>
#endif

/*
 * Only define __NEED_UNDERSCORE__ for arm-coff targets
 */
#if !defined(__ELF__)
#  define __NEED_UNDERSCORE__
#endif

/*
 * Macros to glue together two tokens.
 */
#  ifdef __STDC__
#    define XGLUE(a,b) a##b
#  else
#    define XGLUE(a,b) a/**/b
#  endif

#  define GLUE(a,b) XGLUE(a,b)

/*
 * Symbol Names with leading underscore if necessary
 */
#  ifdef __NEED_UNDERSCORE__
#    define SYM_NAME(name) GLUE(_,name)
#  else
#    define SYM_NAME(name) name
#  endif /* __NEED_UNDERSCORE__ */

/*
 * Various macros to better handle assembler/object format differences
 */
#if defined(__ASSEMBLER__)

/*
 * Assembly function start definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_START name
	.global _\name
	.align  4
    _\name:
.endm
#else
.macro FUNC_START name
	.global \name
	.align  4
    \name:
.endm
#endif

/*
 * Assembly function end definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_END name
.endm
#else
.macro FUNC_END name
.endm
#endif

/*
 * Register Prefix
 */
#  ifndef __REGISTER_PREFIX__
#    define __REGISTER_PREFIX__
#  endif /* __REGISTER_PREFIX__ */

/*
 * Immediate Prefix
 */
#  ifndef __IMM_PREFIX__
#    define __IMM_PREFIX__ #
#  endif /* __IMM_PREFIX__ */

/*
 * use the right prefix for registers.
 */
#  define REG(x) GLUE(__REGISTER_PREFIX__,x)

/*
 * use the right prefix for immediate values.
 */
#  define IMM(x) GLUE(__IMM_PREFIX__,x)

#endif /* defined(__ASSEMBLER__) */


/*
 * Setup register defines and such
 */
#if defined(__ASSEMBLER__)

#  define r0   REG (r0)
#  define r1   REG (r1)
#  define r2   REG (r2)
#  define r3   REG (r3)

#  define r4   REG (r4)
#  define r5   REG (r5)
#  define r6   REG (r6)
#  define r7   REG (r7)
#  define r8   REG (r8)
#  define r9   REG (r9)
#  define r10  REG (r10)
#  define r11  REG (r11)
#  define r12  REG (r12)
#  define r13  REG (r13)
#  define sp   REG (sp)
#  define r14  REG (r14)
#  define lr   REG (lr)
#  define pc   REG (pc)

#  define f0   REG (f0)
#  define f1   REG (f1)
#  define f2   REG (f2)
#  define f3   REG (f3)
#  define f4   REG (f4)
#  define f5   REG (f5)
#  define f6   REG (f6)
#  define f7   REG (f7)
#  define fps  REG (fps)

#  define cpsr REG (cpsr)
#  define spsr REG (spsr)

/*
 * Register offset definitions
 * These numbers are offsets into the ex_regs_t struct.
 */
#  define r0_o   0
#  define r1_o   4
#  define r2_o   8
#  define r3_o   12
#  define r4_o   16
#  define r5_o   20
#  define r6_o   24
#  define r7_o   28
#  define r8_o   32
#  define r9_o   36
#  define r10_o  40
#  define r11_o  44
#  define r12_o  48
#  define r13_o  52
#  define sp_o   r13_o
#  define r14_o  56
#  define lr_o   r14_o
#  define pc_o   60

#  define f0_o   64
#  define f1_o   76
#  define f2_o   88
#  define f3_o   100
#  define f4_o   112
#  define f5_o   124
#  define f6_o   136
#  define f7_o   148
#  define fps_o  160

#  define cpsr_o 164
#  define spsvc_o 168
#  define ARM_EX_REGS_T_SIZE 172

#else /* !defined(__ASSEMBLER__) */

  /*
   * Register name that is used in help strings and such
   */
# define REGNAME_EXAMPLE "r0"

  /*
   *  Register numbers. These are assumed to match the
   *  register numbers used by GDB.
   */
  enum __regnames {
      REG_R0,
      REG_R1,
      REG_R2,
      REG_R3,
      REG_R4,
      REG_R5,
      REG_R6,
      REG_R7,
      REG_R8,
      REG_R9,
      REG_R10,
      REG_R11,
      REG_R12,
      REG_R13,
      REG_SP=REG_R13,
      REG_R14,
      REG_LR=REG_R14,
      REG_PC,

      REG_F0,
      REG_F1,
      REG_F2,
      REG_F3,
      REG_F4,
      REG_F5,
      REG_F6,
      REG_F7,
      REG_FPS,

      REG_CPSR,
      REG_SPSVC,
      REG_MAX=REG_SPSVC
  };

  /*
   * 12-byte struct for storing Floating point registers
   */
  typedef struct
  {
      unsigned long high;
      unsigned long middle;
      unsigned long low;
  } fp_reg;

  /*
   *  How registers are stored for exceptions.
   */
#ifdef __ECOS__
#define ex_regs_t HAL_SavedRegisters
#define _r0       d[0]
#define _r1       d[1]
#define _r2       d[2]
#define _r3       d[3]
#define _r4       d[4]
#define _r5       d[5]
#define _r6       d[6]
#define _r7       d[7]
#define _r8       d[8]
#define _r9       d[9]
#define _r10      d[10]
#define _r11      fp
#define _r12      ip
#define _r13      sp
#define _r14      lr
#define _pc       pc
#define _cpsr     cpsr
#define _spsvc    msr
#else
  typedef struct
  {
    unsigned long _r0;
    unsigned long _r1;
    unsigned long _r2;
    unsigned long _r3;
    unsigned long _r4;
    unsigned long _r5;
    unsigned long _r6;
    unsigned long _r7;
    unsigned long _r8;
    unsigned long _r9;
    unsigned long _r10;
    unsigned long _r11;
    unsigned long _r12;
    unsigned long _r13;
    unsigned long _r14;
    unsigned long _pc;

    fp_reg        _f0;
    fp_reg        _f1;
    fp_reg        _f2;
    fp_reg        _f3;
    fp_reg        _f4;
    fp_reg        _f5;
    fp_reg        _f6;
    fp_reg        _f7;
    unsigned long _fps;
    unsigned long _cpsr;

    unsigned long _spsvc;  /* saved svc mode sp */

  } ex_regs_t;
#endif
#   define   _sp  _r13
#   define   _lr  _r14

extern void __icache_flush(void *addr, int nbytes);
extern void __dcache_flush(void *addr, int nbytes);

#endif /* __ASSEMBLER__ */


/*
 * Program Status Register Definitions
 */
#if defined(__ASSEMBLER__)
#  define ARM_PSR_NEGATIVE       0x80000000  /* Negative Bit                           */
#  define ARM_PSR_ZERO           0x40000000  /* Zero Bit                               */
#  define ARM_PSR_CARRY          0x20000000  /* Carry Bit                              */
#  define ARM_PSR_OVERFLOW       0x10000000  /* Overflow Bit                           */
#  define ARM_PSR_IRQ            0x00000080  /* IRQ Bit                                */
#  define ARM_PSR_FIQ            0x00000040  /* FIQ Bit                                */
#  define ARM_PSR_THUMB_STATE    0x00000020  /* Thumb/ARM(R) Execution                 */
#  define ARM_PSR_MODE_MASK      0x0000001F  /* ARM(R) Processor Mode Mask             */
#else /* ! defined(__ASSEMBLER__) */
  struct psr_struct {
      unsigned mode      : 5;
      unsigned t_bit     : 1;
      unsigned f_bit     : 1;
      unsigned i_bit     : 1;
      unsigned rsv1      : 20;  /* == 0x00000 */
      unsigned v_bit     : 1;
      unsigned c_bit     : 1;
      unsigned z_bit     : 1;
      unsigned n_bit     : 1;
  };

  union arm_psr {
      unsigned long word;
      struct psr_struct psr;
  };
#endif /* __ASSEMBLER__ */

/*
 * PSR Mode values
 */
#define ARM_PSR_MODE_USER      0x00000010  /* User mode                              */
#define ARM_PSR_MODE_FIQ       0x00000011  /* FIQ mode                               */
#define ARM_PSR_MODE_IRQ       0x00000012  /* IRQ mode                               */
#define ARM_PSR_MODE_SVC       0x00000013  /* SVC mode                               */
#define ARM_PSR_MODE_ABORT     0x00000017  /* ABORT mode                             */
#define ARM_PSR_MODE_UNDEF     0x0000001B  /* UNDEF mode                             */
#define ARM_PSR_MODE_SYSTEM    0x0000001F  /* System Mode                            */
#define ARM_PSR_NUM_MODES      7

/*
 * Core Exception vectors.
 */
#define BSP_CORE_EXC_RESET                     0
#define BSP_CORE_EXC_UNDEFINED_INSTRUCTION     1
#define BSP_CORE_EXC_SOFTWARE_INTERRUPT        2
#define BSP_CORE_EXC_PREFETCH_ABORT            3
#define BSP_CORE_EXC_DATA_ABORT                4
#define BSP_CORE_EXC_ADDRESS_ERROR_26_BIT      5
#define BSP_CORE_EXC_IRQ                       6
#define BSP_CORE_EXC_FIQ                       7
#define BSP_MAX_EXCEPTIONS                     8
#define BSP_CORE_EXC(vec_num)                  (unsigned long*)(vec_num << 2)

#define BREAKPOINT_INSN                        0xE7FFDEFE   /* Illegal inst opcode */
#define SYSCALL_SWI                            0x00180001

#if defined(__ASSEMBLER__)
  .macro BREAKPOINT
         .word BREAKPOINT_INSN
  .endm
  .macro SYSCALL
         swi  IMM(SYSCALL_SWI)
  .endm
  .macro __CLI
         stmfd  sp!, {r0}
         mrs    r0, cpsr
         bic    r0, r0, IMM(ARM_PSR_IRQ | ARM_PSR_FIQ)
         msr    cpsr, r0
         ldmfd  sp!, {r0}
  .endm
  .macro __STI
         stmfd  sp!, {r0}
         mrs    r0, cpsr
         orr    r0, r0, IMM(ARM_PSR_IRQ | ARM_PSR_FIQ)
         msr    cpsr, r0
         ldmfd  sp!, {r0}
  .endm

#  if 0
  /*
   * Use this code to verify a particular processing mode
   */
	mrs	r0, cpsr
        and     r0, r0, IMM(ARM_PSR_MODE_MASK)
        ldr     r1, =ARM_PSR_MODE_IRQ
        cmps    r0, r1
0:      bne     0b
        PORT_TOGGLE_DEBUG
#  endif /* 0 */

#else /* !defined(__ASSEMBLER__) */

#  define BREAKPOINT() asm volatile(" .word 0xE7FFDEFE")
#  define SYSCALL()    asm volatile(" swi   %0" : /* No outputs */ : "i" (SYSCALL_SWI))
#  define __cli()      asm volatile("
         stmfd  sp!, {r0}
         mrs    r0, cpsr
         bic    r0, r0, #0x000000C0
         msr    cpsr, r0
         ldmfd  sp!, {r0}")
#  define __sti()      asm volatile("
         stmfd  sp!, {r0}
         mrs    r0, cpsr
         orr    r0, r0, #0x000000C0
         msr    cpsr, r0
         ldmfd  sp!, {r0}")
#  define __mcr(cp_num, opcode1, Rd, CRn, CRm, opcode2) \
     asm volatile (" mcr " cp_num  ", " \
                           opcode1 ", " \
                           "%0"    ", " \
                           CRn     ", " \
                           CRm     ", " \
                           opcode2 : /* no outputs */ : "r" (Rd))
#  define __mrc(cp_num, opcode1, Rd, CRn, CRm, opcode2) \
     asm volatile (" mrc " cp_num  ", " \
                           opcode1 ", " \
                           "%0"    ", " \
                           CRn     ", " \
                           CRm     ", " \
                           opcode2 : "=r" (Rd) : /* no inputs */)

  static inline unsigned __get_cpsr(void)
  {
      unsigned long retval;
      asm volatile (" mrs  %0, cpsr" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_cpsr(unsigned val)
  {
      asm volatile (" msr  cpsr, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_spsr(void)
  {
      unsigned long retval;
      asm volatile (" mrs  %0, spsr" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_spsr(unsigned val)
  {
      asm volatile (" msr  spsr, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_sp(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, sp" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_sp(unsigned val)
  {
      asm volatile (" mov  sp, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_fp(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, fp" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_fp(unsigned val)
  {
      asm volatile (" mov  fp, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_pc(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, pc" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_pc(unsigned val)
  {
      asm volatile (" mov  pc, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_lr(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, lr" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_lr(unsigned val)
  {
      asm volatile (" mov  lr, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_r8(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, r8" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_r8(unsigned val)
  {
      asm volatile (" mov  r8, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_r9(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, r9" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_r9(unsigned val)
  {
      asm volatile (" mov  r9, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_r10(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, r10" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_r10(unsigned val)
  {
      asm volatile (" mov  r10, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_r11(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, r11" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_r11(unsigned val)
  {
      asm volatile (" mov  r11, %0" : /* no outputs */ : "r" (val)  );
  }

  static inline unsigned __get_r12(void)
  {
      unsigned long retval;
      asm volatile (" mov  %0, r12" : "=r" (retval) : /* no inputs */  );
      return retval;
  }

  static inline void __set_r12(unsigned val)
  {
      asm volatile (" mov  r12, %0" : /* no outputs */ : "r" (val)  );
  }

#endif /* defined(__ASSEMBLER__) */

#define GDB_BREAKPOINT_VECTOR BSP_CORE_EXC_UNDEFINED_INSTRUCTION
#define GDB_SYSCALL_VECTOR    BSP_CORE_EXC_SOFTWARE_INTERRUPT

#define ARM_INST_SIZE            sizeof(unsigned long)
#define GDB_BREAKPOINT_INST_SIZE ARM_INST_SIZE

#ifdef __CPU_LH77790A__
#  include <bsp/lh77790a.h>
#endif /* __CPU_LH77790A__ */

#if !defined(__ASSEMBLER__)
/*
 * Define the CPU specific data
 */
#ifdef __CPU_LH77790A__
  typedef struct {
      unsigned char lh77790a_port_control_shadow;
  } arm_cpu_data;
#endif /* __CPU_LH77790A__ */
#endif /* !defined(__ASSEMBLER__) */

#ifdef __CPU_SA110__
#include <bsp/sa-110.h>
#endif /* __CPU_SA110__ */

#ifdef __CPU_SA1100__
#include <bsp/sa-1100.h>
#endif /* __CPU_SA110__ */

#ifdef __CPU_710T__
#include <bsp/arm710t.h>
#endif /* __CPU_710T__ */

#ifdef MMU
/*
 * ARM(R) MMU Definitions
 */

#ifndef __ASSEMBLER__
extern void *page1;
#endif /* __ASSEMBLER__ */

/*
 * ARM(R) Cache and MMU Control Registers
 *
 * Accessed through coprocessor instructions.
 */
#ifdef __ASSEMBLER__
#  define ARM_CACHE_COPROCESSOR_NUM             p15
#  define ARM_COPROCESSOR_OPCODE_DONT_CARE      0x0
#  define ARM_COPROCESSOR_RM_DONT_CARE          c0
#else /* __ASSEMBLER__ */
#  define ARM_CACHE_COPROCESSOR_NUM             "p15"
#  define ARM_COPROCESSOR_OPCODE_DONT_CARE      "0x0"
#  define ARM_COPROCESSOR_RM_DONT_CARE          "c0"
#endif /* __ASSEMBLER__ */

#ifdef __ASSEMBLER__
#  define ARM_ID_REGISTER                        c0
#  define ARM_CONTROL_REGISTER                   c1
#  define ARM_TRANSLATION_TABLE_BASE_REGISTER    c2
#  define ARM_DOMAIN_ACCESS_CONTROL_REGISTER     c3
#  define ARM_FAULT_STATUS_REGISTER              c5
#  define ARM_FAULT_ADDRESS_REGISTER             c6
#  define ARM_CACHE_OPERATIONS_REGISTER          c7
#  define ARM_TLB_OPERATIONS_REGISTER            c8
#  define ARM_READ_BUFFER_OPERATIONS_REGISTER    c9
#else /* __ASSEMBLER__ */
#  define ARM_ID_REGISTER                        "c0"
#  define ARM_CONTROL_REGISTER                   "c1"
#  define ARM_TRANSLATION_TABLE_BASE_REGISTER    "c2"
#  define ARM_DOMAIN_ACCESS_CONTROL_REGISTER     "c3"
#  define ARM_FAULT_STATUS_REGISTER              "c5"
#  define ARM_FAULT_ADDRESS_REGISTER             "c6"
#  define ARM_CACHE_OPERATIONS_REGISTER          "c7"
#  define ARM_TLB_OPERATIONS_REGISTER            "c8"
#  define ARM_READ_BUFFER_OPERATIONS_REGISTER    "c9"
#endif /* __ASSEMBLER__ */

/*
 * SA-1100 Cache and MMU ID Register value
 */
#define ARM_ID_MASK                              0xFFFFFFF0
#define ARM_ID_VALUE                             0x4401a110

/*
 * SA-1100 Cache Control Register Bit Fields and Masks
 */
#define ARM_MMU_DISABLED                         0x00000000
#define ARM_MMU_ENABLED                          0x00000001
#define ARM_MMU_MASK                             0x00000001
#define ARM_ADDRESS_FAULT_DISABLED               0x00000000
#define ARM_ADDRESS_FAULT_ENABLED                0x00000002
#define ARM_ADDRESS_FAULT_MASK                   0x00000002
#define ARM_DATA_CACHE_DISABLED                  0x00000000
#define ARM_DATA_CACHE_ENABLED                   0x00000004
#define ARM_DATA_CACHE_MASK                      0x00000004
#define ARM_WRITE_BUFFER_DISABLED                0x00000000
#define ARM_WRITE_BUFFER_ENABLED                 0x00000008
#define ARM_WRITE_BUFFER_MASK                    0x00000008
#define ARM_LITTLE_ENDIAN                        0x00000000
#define ARM_BIG_ENDIAN                           0x00000080
#define ARM_ACCESS_CHECKS_NONE                   0x00000000
#define ARM_ACCESS_CHECKS_SYSTEM                 0x00000100
#define ARM_ACCESS_CHECKS_ROM                    0x00000200
#define ARM_INSTRUCTION_CACHE_DISABLED           0x00000000
#define ARM_INSTRUCTION_CACHE_ENABLED            0x00001000
#define ARM_INSTRUCTION_CACHE_MASK               0x00001000
#define ARM_VIRTUAL_IVR_BASE_00000000            0x00000000
#define ARM_VIRTUAL_IVR_BASE_FFFF0000            0x00002000
#define ARM_CONTROL_SBZ_MASK                     0x00001FFF

/*
 * SA-1100 Translation Table Base Bit Masks
 */
#define ARM_TRANSLATION_TABLE_MASK               0xFFFFC000

/*
 * SA-1100 Domain Access Control Bit Masks
 */
#define ARM_DOMAIN_0_MASK                        0x00000003
#define ARM_DOMAIN_1_MASK                        0x0000000C
#define ARM_DOMAIN_2_MASK                        0x00000030
#define ARM_DOMAIN_3_MASK                        0x000000C0
#define ARM_DOMAIN_4_MASK                        0x00000300
#define ARM_DOMAIN_5_MASK                        0x00000C00
#define ARM_DOMAIN_6_MASK                        0x00003000
#define ARM_DOMAIN_7_MASK                        0x0000C000
#define ARM_DOMAIN_8_MASK                        0x00030000
#define ARM_DOMAIN_9_MASK                        0x000C0000
#define ARM_DOMAIN_10_MASK                       0x00300000
#define ARM_DOMAIN_11_MASK                       0x00C00000
#define ARM_DOMAIN_12_MASK                       0x03000000
#define ARM_DOMAIN_13_MASK                       0x0C000000
#define ARM_DOMAIN_14_MASK                       0x30000000
#define ARM_DOMAIN_15_MASK                       0xC0000000

#define ARM_ACCESS_TYPE_NO_ACCESS(domain_num)    (0x0 << (domain_num))
#define ARM_ACCESS_TYPE_CLIENT(domain_num)       (0x1 << (domain_num))
#define ARM_ACCESS_TYPE_MANAGER(domain_num)      (0x3 << (domain_num))

/*
 * SA-1100 Fault Status Bit Masks
 */
#define ARM_FAULT_STATUS_MASK                    0x0000000F
#define ARM_DOMAIN_MASK                          0x000000F0
#define ARM_DATA_BREAKPOINT_MASK                 0x00000200

/*
 * SA-1100 Cache Control Operations Definitions
 */
#ifdef __ASSEMBLER__
#  define ARM_FLUSH_CACHE_INST_DATA_OPCODE       0x0
#  define ARM_FLUSH_CACHE_INST_DATA_RM           c7
#  define ARM_FLUSH_CACHE_INST_OPCODE            0x0
#  define ARM_FLUSH_CACHE_INST_RM                c5
#  define ARM_FLUSH_CACHE_DATA_OPCODE            0x0
#  define ARM_FLUSH_CACHE_DATA_RM                c6
#  define ARM_FLUSH_CACHE_DATA_SINGLE_OPCODE     0x1
#  define ARM_FLUSH_CACHE_DATA_SINGLE_RM         c6
#  define ARM_CLEAN_CACHE_DATA_ENTRY_OPCODE      0x1
#  define ARM_CLEAN_CACHE_DATA_ENTRY_RM          c10
#  define ARM_DRAIN_CACHE_WRITE_BUFFER_OPCODE    0x4
#  define ARM_DRAIN_CACHE_WRITE_BUFFER_RM        c10
#else /* __ASSEMBLER__ */                        
#  define ARM_FLUSH_CACHE_INST_DATA_OPCODE       "0x0"
#  define ARM_FLUSH_CACHE_INST_DATA_RM           "c7"
#  define ARM_FLUSH_CACHE_INST_OPCODE            "0x0"
#  define ARM_FLUSH_CACHE_INST_RM                "c5"
#  define ARM_FLUSH_CACHE_DATA_OPCODE            "0x0"
#  define ARM_FLUSH_CACHE_DATA_RM                "c6"
#  define ARM_FLUSH_CACHE_DATA_SINGLE_OPCODE     "0x1"
#  define ARM_FLUSH_CACHE_DATA_SINGLE_RM         "c6"
#  define ARM_CLEAN_CACHE_DATA_ENTRY_OPCODE      "0x1"
#  define ARM_CLEAN_CACHE_DATA_ENTRY_RM          "c10"
#  define ARM_DRAIN_CACHE_WRITE_BUFFER_OPCODE    "0x4"
#  define ARM_DRAIN_CACHE_WRITE_BUFFER_RM        "c10"
#endif /* __ASSEMBLER__ */

/*                                               
 * SA-1100 TLB Operations Definitions             
 */                                              
#ifdef __ASSEMBLER__
#  define ARM_FLUSH_INST_DATA_TLB_OPCODE         0x0
#  define ARM_FLUSH_INST_DATA_TLB_RM             c7
#  define ARM_FLUSH_INST_TLB_OPCODE              0x0
#  define ARM_FLUSH_INST_TLB_RM                  c5
#  define ARM_FLUSH_DATA_TLB_OPCODE              0x0
#  define ARM_FLUSH_DATA_TLB_RM                  c6
#  define ARM_FLUSH_DATA_ENTRY_TLB_OPCODE        0x1
#  define ARM_FLUSH_DATA_ENTRY_TLB_RM            c6
#else /* __ASSEMBLER__ */
#  define ARM_FLUSH_INST_DATA_TLB_OPCODE         "0x0"
#  define ARM_FLUSH_INST_DATA_TLB_RM             "c7"
#  define ARM_FLUSH_INST_TLB_OPCODE              "0x0"
#  define ARM_FLUSH_INST_TLB_RM                  "c5"
#  define ARM_FLUSH_DATA_TLB_OPCODE              "0x0"
#  define ARM_FLUSH_DATA_TLB_RM                  "c6"
#  define ARM_FLUSH_DATA_ENTRY_TLB_OPCODE        "0x1"
#  define ARM_FLUSH_DATA_ENTRY_TLB_RM            "c6"
#endif /* __ASSEMBLER__ */

/*
 * SA-1100 Read-Buffer Operations Definitions
 */
#ifdef __ASSEMBLER__
#  define ARM_FLUSH_ALL_BUFFERS_OPCODE           0x0
#  define ARM_FLUSH_ALL_BUFFERS_RM               c0
#  define ARM_FLUSH_BUFFER_0_OPCODE              0x1
#  define ARM_FLUSH_BUFFER_0_RM                  c0
#  define ARM_FLUSH_BUFFER_1_OPCODE              0x1
#  define ARM_FLUSH_BUFFER_1_RM                  c1
#  define ARM_FLUSH_BUFFER_2_OPCODE              0x1
#  define ARM_FLUSH_BUFFER_2_RM                  c2
#  define ARM_FLUSH_BUFFER_3_OPCODE              0x1
#  define ARM_FLUSH_BUFFER_3_RM                  c3
#  define ARM_LOAD_BUFFER_0_1_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_0_1_WORD_RM            c0
#  define ARM_LOAD_BUFFER_0_4_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_0_4_WORD_RM            c4
#  define ARM_LOAD_BUFFER_0_8_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_0_8_WORD_RM            c8
#  define ARM_LOAD_BUFFER_1_1_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_1_1_WORD_RM            c1
#  define ARM_LOAD_BUFFER_1_4_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_1_4_WORD_RM            c5
#  define ARM_LOAD_BUFFER_1_8_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_1_8_WORD_RM            c9
#  define ARM_LOAD_BUFFER_2_1_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_2_1_WORD_RM            c2
#  define ARM_LOAD_BUFFER_2_4_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_2_4_WORD_RM            c6
#  define ARM_LOAD_BUFFER_2_8_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_2_8_WORD_RM            cA
#  define ARM_LOAD_BUFFER_3_1_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_3_1_WORD_RM            c3
#  define ARM_LOAD_BUFFER_3_4_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_3_4_WORD_RM            c7
#  define ARM_LOAD_BUFFER_3_8_WORD_OPCODE        0x2
#  define ARM_LOAD_BUFFER_3_8_WORD_RM            cB
#  define ARM_DISABLE_USER_MCR_ACCESS_OPCODE     0x4
#  define ARM_DISABLE_USER_MCR_ACCESS_RM         c0
#  define ARM_ENABLE_USER_MCR_ACCESS_OPCODE      0x5
#  define ARM_ENABLE_USER_MCR_ACCESS_RM          c0
#else /* __ASSEMBLER__ */
#  define ARM_FLUSH_ALL_BUFFERS_OPCODE           "0x0"
#  define ARM_FLUSH_ALL_BUFFERS_RM               "c0"
#  define ARM_FLUSH_BUFFER_0_OPCODE              "0x1"
#  define ARM_FLUSH_BUFFER_0_RM                  "c0"
#  define ARM_FLUSH_BUFFER_1_OPCODE              "0x1"
#  define ARM_FLUSH_BUFFER_1_RM                  "c1"
#  define ARM_FLUSH_BUFFER_2_OPCODE              "0x1"
#  define ARM_FLUSH_BUFFER_2_RM                  "c2"
#  define ARM_FLUSH_BUFFER_3_OPCODE              "0x1"
#  define ARM_FLUSH_BUFFER_3_RM                  "c3"
#  define ARM_LOAD_BUFFER_0_1_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_0_1_WORD_RM            "c0"
#  define ARM_LOAD_BUFFER_0_4_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_0_4_WORD_RM            "c4"
#  define ARM_LOAD_BUFFER_0_8_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_0_8_WORD_RM            "c8"
#  define ARM_LOAD_BUFFER_1_1_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_1_1_WORD_RM            "c1"
#  define ARM_LOAD_BUFFER_1_4_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_1_4_WORD_RM            "c5"
#  define ARM_LOAD_BUFFER_1_8_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_1_8_WORD_RM            "c9"
#  define ARM_LOAD_BUFFER_2_1_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_2_1_WORD_RM            "c2"
#  define ARM_LOAD_BUFFER_2_4_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_2_4_WORD_RM            "c6"
#  define ARM_LOAD_BUFFER_2_8_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_2_8_WORD_RM            "cA"
#  define ARM_LOAD_BUFFER_3_1_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_3_1_WORD_RM            "c3"
#  define ARM_LOAD_BUFFER_3_4_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_3_4_WORD_RM            "c7"
#  define ARM_LOAD_BUFFER_3_8_WORD_OPCODE        "0x2"
#  define ARM_LOAD_BUFFER_3_8_WORD_RM            "cB"
#  define ARM_DISABLE_USER_MCR_ACCESS_OPCODE     "0x4"
#  define ARM_DISABLE_USER_MCR_ACCESS_RM         "c0"
#  define ARM_ENABLE_USER_MCR_ACCESS_OPCODE      "0x5"
#  define ARM_ENABLE_USER_MCR_ACCESS_RM          "c0"
#endif /* __ASSEMBLER__ */

/*
 * ARM(R) First Level Descriptor Format Definitions
 */
#ifndef __ASSEMBLER__
struct ARM_MMU_FIRST_LEVEL_FAULT {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_FAULT_ID 0x0

struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE {
    int id : 2;
    int imp : 2;
    int domain : 4;
    int sbz : 1;
    int base_address : 23;
};
#define ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID 0x1

struct ARM_MMU_FIRST_LEVEL_SECTION {
    int id : 2;
    int b : 1;
    int c : 1;
    int imp : 1;
    int domain : 4;
    int sbz0 : 1;
    int ap : 2;
    int sbz1 : 8;
    int base_address : 12;
};
#define ARM_MMU_FIRST_LEVEL_SECTION_ID 0x2

struct ARM_MMU_FIRST_LEVEL_RESERVED {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_RESERVED_ID 0x3

#define ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, table_index) \
            (unsigned long *)((unsigned long)(ttb_base) + ((table_index) << 2))
#define ARM_MMU_SECTION(ttb_base, actual_base, virtual_base, cacheable, bufferable, perm) \
    {                                                                                     \
        register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;                               \
                                                                                          \
        desc.word = 0;                                                                    \
        desc.section.id = ARM_MMU_FIRST_LEVEL_SECTION_ID;                                 \
        desc.section.domain = 0;                                                          \
        desc.section.c = (cacheable);                                                     \
        desc.section.b = (bufferable);                                                    \
        desc.section.ap = (perm);                                                         \
        desc.section.base_address = (actual_base);                                        \
        *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, (virtual_base)) = desc.word;    \
    }

union ARM_MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct ARM_MMU_FIRST_LEVEL_FAULT fault;
    struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct ARM_MMU_FIRST_LEVEL_SECTION section;
    struct ARM_MMU_FIRST_LEVEL_RESERVED reserved;
};

#endif /* __ASSEMBLER__ */

#define ARM_UNCACHEABLE                         0
#define ARM_CACHEABLE                           1
#define ARM_UNBUFFERABLE                        0
#define ARM_BUFFERABLE                          1

#define ARM_ACCESS_PERM_NONE_NONE               0
#define ARM_ACCESS_PERM_RO_NONE                 0
#define ARM_ACCESS_PERM_RO_RO                   0
#define ARM_ACCESS_PERM_RW_NONE                 1
#define ARM_ACCESS_PERM_RW_RO                   2
#define ARM_ACCESS_PERM_RW_RW                   3

#define ARM_SECTION_SIZE                        SZ_1M
#define ARM_SMALL_PAGE_SIZE                     SZ_4K
#define ARM_LARGE_PAGE_SIZE                     SZ_64K

#define ARM_FIRST_LEVEL_PAGE_TABLE_SIZE         SZ_16K
#define ARM_SECOND_LEVEL_PAGE_TABLE_SIZE        SZ_1K

#endif /* MMU */

#endif // __ARM_CPU_H__
