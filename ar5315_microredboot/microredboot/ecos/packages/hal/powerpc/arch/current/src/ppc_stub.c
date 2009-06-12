//========================================================================
//
//      ppc_stub.c
//
//      Helper functions for stub, generic to all PowerPC processors
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
// Author(s):     Red Hat, jskov
// Contributors:  Red Hat, jskov, gthomas
// Date:          1998-08-20
// Purpose:       
// Description:   Helper functions for stub, generic to all PowerPC processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#ifdef CYGNUM_HAL_NO_VECTOR_TRACE
#define USE_BREAKPOINTS_FOR_SINGLE_STEP
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

#ifndef OFFSETOF
#define OFFSETOF(_struct_, _member_) (int)((char *)(&(((_struct_*)0)->_member_))-(char *)((_struct_*)0))
#endif

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    switch (trap_number)
    {
    case CYGNUM_HAL_VECTOR_MACHINE_CHECK:
        /* Machine check */
    case CYGNUM_HAL_VECTOR_DSI:
        /* Data access */
        return SIGSEGV;
      
    case CYGNUM_HAL_VECTOR_ISI:
        /* Instruction access (Ifetch) */
    case CYGNUM_HAL_VECTOR_ALIGNMENT:
        /* Data access */
        return SIGBUS;
                    
    case CYGNUM_HAL_VECTOR_INTERRUPT:
        /* External interrupt */
      return SIGINT;

    case CYGNUM_HAL_VECTOR_TRACE:
        /* Instruction trace */
        return SIGTRAP;
      
    case CYGNUM_HAL_VECTOR_PROGRAM:
#ifdef CYGPKG_HAL_POWERPC_PPC40x
        // The 40x is b0rken, returning 0 for these bits. Translate to
        // SIGTRAP to allow thread debugging.
        return SIGTRAP;
#else
        // The register PS contains the value of SRR1 at the time of
        // exception entry. Bits 11-15 contain information about the
        // cause of the exception. Bits 16-31 the PS (MSR) state.
#ifdef USE_BREAKPOINTS_FOR_SINGLE_STEP
        if (__is_single_step(get_register(PC))) {
            return SIGTRAP;
        }
#endif
        switch ((get_register (PS) >> 17) & 0xf){
        case 1:                         /* trap */
            return SIGTRAP;
        case 2:                         /* privileged instruction */
        case 4:                         /* illegal instruction */
            return SIGILL;
        case 8:                         /* floating point */
            return SIGFPE;
        default:                        /* should never happen! */
            return SIGILL;
        }            
#endif

    case CYGNUM_HAL_VECTOR_RESERVED_A:
    case CYGNUM_HAL_VECTOR_RESERVED_B:
        return SIGILL;

    case CYGNUM_HAL_VECTOR_FP_UNAVAILABLE:
        /* FPU disabled */
    case CYGNUM_HAL_VECTOR_FP_ASSIST:
        /* FPU assist */
        return SIGFPE;

    case CYGNUM_HAL_VECTOR_DECREMENTER:
        /* Decrementer alarm */
        return SIGALRM;

    case CYGNUM_HAL_VECTOR_SYSTEM_CALL:
        /* System call */
        return SIGSYS;

#if defined(CYGPKG_HAL_POWERPC_MPC8xx) || defined(CYGPKG_HAL_POWERPC_MPC5xx)
    case CYGNUM_HAL_VECTOR_SW_EMUL:
        /* A SW_EMUL is generated instead of PROGRAM for illegal
           instructions. */
        return SIGILL;

    case CYGNUM_HAL_VECTOR_DATA_BP:
    case CYGNUM_HAL_VECTOR_INSTRUCTION_BP:
    case CYGNUM_HAL_VECTOR_PERIPHERAL_BP:
    case CYGNUM_HAL_VECTOR_NMI:
        /* Developer port debugging exceptions. */
        return SIGTRAP;

#if defined(CYGNUM_HAL_VECTOR_ITLB_MISS)	
    case CYGNUM_HAL_VECTOR_ITLB_MISS:
        /* Software reload of TLB required. */
        return SIGTRAP;
#endif
#if defined(CYGNUM_HAL_VECTOR_DTLB_MISS)	
    case CYGNUM_HAL_VECTOR_DTLB_MISS:
        /* Software reload of TLB required. */
        return SIGTRAP;
#endif
    case CYGNUM_HAL_VECTOR_ITLB_ERROR:
        /* Invalid instruction access. */
        return SIGBUS;

    case CYGNUM_HAL_VECTOR_DTLB_ERROR:
        /* Invalid data access. */
        return SIGSEGV;
#endif // defined(CYGPKG_HAL_POWERPC_MPC8xx)
        
    default:
        return SIGTERM;
    }
}


/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return _hal_registers->vector >> 8;
}

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}

#ifdef CYGHWR_HAL_POWERPC_FPU
static int
reg_offset(regnames_t reg)
{
  // We let the compiler determine the offsets in order to avoid all
  // possible alignment problems
  int base_offset;
  // 32 general purpose registers
  if(reg < F0)   return reg * 4;

  // first sixteen floating point regs
  base_offset = OFFSETOF(GDB_Registers, f0);
  if(reg < F16)  return base_offset + ((reg - F0) * 8);

  // last sixteen floating point regs
  base_offset = OFFSETOF(GDB_Registers, f16);
  if(reg < PC) 	 return base_offset + ((reg - F16) * 8);

  // Other 32 bit regs
  if(reg < PS)   return(OFFSETOF(GDB_Registers, pc));
  if(reg < CND)  return(OFFSETOF(GDB_Registers, msr));
  if(reg < LR)   return(OFFSETOF(GDB_Registers, cr));
  if(reg < CNT)  return(OFFSETOF(GDB_Registers, lr));
  if(reg < XER)  return(OFFSETOF(GDB_Registers, ctr));
  if(reg < MQ)   return(OFFSETOF(GDB_Registers, xer));
  
  return OFFSETOF(GDB_Registers, mq);
}

// Return the currently-saved value corresponding to register REG of
// the exception context.
target_register_t
get_register (regnames_t reg)
{
   target_register_t val;
   int offset = reg_offset(reg);

   if (REGSIZE(reg) > sizeof(target_register_t))
   return -1;

   val = _registers[offset/sizeof(target_register_t)];

   return val;
}

// Store VALUE in the register corresponding to WHICH in the exception
// context.
void
put_register (regnames_t which, target_register_t value)
{
   int offset = reg_offset(which);

   if (REGSIZE(which) > sizeof(target_register_t))
   return;

   _registers[offset/sizeof(target_register_t)] = value;
}

// Write the contents of register WHICH into VALUE as raw bytes. This
// is only used for registers larger than sizeof(target_register_t).
// Return non-zero if it is a valid register.
int
get_register_as_bytes (regnames_t which, char *value)
{
  int offset = reg_offset(which);

  memcpy (value, (char *)_registers + offset, REGSIZE(which));
  return 1;
}

// Alter the contents of saved register WHICH to contain VALUE. This
// is only used for registers larger than sizeof(target_register_t).
// Return non-zero if it is a valid register.
int
put_register_as_bytes (regnames_t which, char *value)
{
  int offset = reg_offset(which);

  memcpy ((char *)_registers + offset, value, REGSIZE(which));
  return 1;
}
#endif

/*----------------------------------------------------------------------
 * Single-step support
 */

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

#ifdef USE_BREAKPOINTS_FOR_SINGLE_STEP

#if (HAL_BREAKINST_SIZE == 1)
typedef cyg_uint8 t_inst;
#elif (HAL_BREAKINST_SIZE == 2)
typedef cyg_uint16 t_inst;
#elif (HAL_BREAKINST_SIZE == 4)
typedef cyg_uint32 t_inst;
#else
#error "Don't know how to handle that size"
#endif

typedef struct
{
  t_inst *targetAddr;
  t_inst savedInstr;
} instrBuffer;

static instrBuffer sstep_instr[2];
static target_register_t irq_state = 0;

static void 
__insert_break(int indx, target_register_t pc)
{
    sstep_instr[indx].targetAddr = (t_inst *)pc;
    sstep_instr[indx].savedInstr = *(t_inst *)pc;
    *(t_inst*)pc = (t_inst)HAL_BREAKINST;
    __data_cache(CACHE_FLUSH);
    __instruction_cache(CACHE_FLUSH);
}

static void 
__remove_break(int indx)
{
    if (sstep_instr[indx].targetAddr != 0) {
        *(sstep_instr[indx].targetAddr) = sstep_instr[indx].savedInstr;
        sstep_instr[indx].targetAddr = 0;
        __data_cache(CACHE_FLUSH);
        __instruction_cache(CACHE_FLUSH);
    }
}

int
__is_single_step(target_register_t pc)
{
    return (sstep_instr[0].targetAddr == pc) ||
        (sstep_instr[1].targetAddr == pc);
}


// Compute the target address for this instruction, if the instruction
// is some sort of branch/flow change.

struct xl_form {
    unsigned int op : 6;
    unsigned int bo : 5;
    unsigned int bi : 5;
    unsigned int reserved : 5;
    unsigned int xo : 10;
    unsigned int lk : 1;
};

struct i_form {
    unsigned int op : 6;
    signed   int li : 24;
    unsigned int aa : 1;
    unsigned int lk : 1;
};

struct b_form {
    unsigned int op : 6;
    unsigned int bo : 5;
    unsigned int bi : 5;
    signed   int bd : 14;
    unsigned int aa : 1;
    unsigned int lk : 1;
};

union ppc_insn {
    unsigned int   word;
    struct i_form  i;
    struct b_form  b;
    struct xl_form xl;
};

static target_register_t
__branch_pc(target_register_t pc)
{
    union ppc_insn insn;

    insn.word = *(t_inst *)pc;

    // Decode the instruction to determine the instruction which will follow
    // Note: there are holes in this process, but the important ones work
    switch (insn.i.op) {
    case 16:
	/* bcx */
	if (insn.b.aa) {
	    return (target_register_t)(insn.b.bd << 2);
        } else {
	    return (target_register_t)((insn.b.bd << 2) + (long)pc);
        }
    case 18:
	/* bx */
	if (insn.i.aa) {
	    return (target_register_t)(insn.i.li << 2);
        } else {
	    return (target_register_t)((insn.i.li << 2) + (long)pc);
        }
    case 19:
	if (insn.xl.reserved == 0) {
	    if (insn.xl.xo == 528) {
		/* bcctrx */
                return (target_register_t)(get_register(CNT) & ~3);
	    } else if (insn.xl.xo == 16) {
		/* bclrx */
                return (target_register_t)(get_register(LR) & ~3);
	    }
	}
	break;
    default:
	break;
    }
    return (pc+4);
}

void __single_step(void)
{
    target_register_t msr = get_register(PS);
    target_register_t pc = get_register(PC);
    target_register_t next_pc = __branch_pc(pc);

    // Disable interrupts.
    irq_state = msr & MSR_EE;
    msr &= ~MSR_EE;
    put_register (PS, msr);

    // Set a breakpoint at the next instruction
    __insert_break(0, pc+4);
    if (next_pc != (pc+4)) {
        __insert_break(1, next_pc);
    }
}

/* Clear the single-step state. */

void __clear_single_step(void)
{
    target_register_t msr = get_register (PS);

    // Restore interrupt state.
    // FIXME: Should check whether the executed instruction changed the
    // interrupt state - or single-stepping a MSR changing instruction
    // may result in a wrong EE. Not a very likely scenario though.
    msr |= irq_state;

    // This function is called much more than its counterpart
    // __single_step.  Only re-enable interrupts if they where
    // disabled during the previous cal to __single_step. Otherwise,
    // this function only makes "extra sure" that no trace or branch
    // exception will happen.
    irq_state = 0;

    put_register (PS, msr);

    // Remove breakpoints
    __remove_break(0);
    __remove_break(1);
}

#else

static target_register_t irq_state = 0;

void __single_step (void)
{
    target_register_t msr = get_register (PS);

    // Set single-step flag in the exception context.
    msr |= (MSR_SE | MSR_BE);
    // Disable interrupts.
    irq_state = msr & MSR_EE;
    msr &= ~MSR_EE;

    put_register (PS, msr);
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
    target_register_t msr = get_register (PS);

    // Clear single-step flag in the exception context.
    msr &= ~(MSR_SE | MSR_BE);
    // Restore interrupt state.
    // FIXME: Should check whether the executed instruction changed the
    // interrupt state - or single-stepping a MSR changing instruction
    // may result in a wrong EE. Not a very likely scenario though.
    msr |= irq_state;

    // This function is called much more than its counterpart
    // __single_step.  Only re-enable interrupts if they where
    // disabled during the previous cal to __single_step. Otherwise,
    // this function only makes "extra sure" that no trace or branch
    // exception will happen.
    irq_state = 0;

    put_register (PS, msr);
}
#endif

void __install_breakpoints (void)
{
    /* NOP since single-step HW exceptions are used instead of
       breakpoints. */
}

void __clear_breakpoints (void)
{
}


/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */

int
__is_breakpoint_function ()
{
    return get_register (PC) == (target_register_t)&_breakinst;
}


/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
   we can safely just skip 4. */

void __skipinst (void)
{
    put_register (PC, get_register (PC) + 4);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
