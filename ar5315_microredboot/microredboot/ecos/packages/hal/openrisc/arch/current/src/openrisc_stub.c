//========================================================================
//
//      openrisc_stub.c
//
//      OpenRISC-specific code for remote debugging via gdb
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
// Author(s):     sfurman
// Contributors:  Red Hat, jskov, gthomas
// Date:          2003-02-07
// Purpose:       
// Description:   Helper functions for gdb stub for OpenRISC processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/openrisc_opcode.h>
#include <cyg/infra/cyg_ass.h>          // assertion macros

#ifdef CYGNUM_HAL_NO_VECTOR_TRACE
#define USE_BREAKPOINTS_FOR_SINGLE_STEP
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    switch (trap_number)
    {
        // Either no matching page-table entry or protection fault
        // while executing load/store operation
    case CYGNUM_HAL_VECTOR_DATA_PAGE_FAULT:
        // Either no matching page-table entry or protection fault
        // while executing load/store operation
    case CYGNUM_HAL_VECTOR_INSTR_PAGE_FAULT:
        return SIGSEGV;
      
        // No matching entry in D-TLB
    case CYGNUM_HAL_VECTOR_DTLB_MISS:
        // No matching entry in I-TLB
    case CYGNUM_HAL_VECTOR_ITLB_MISS:
        // Unaligned load/store memory access
    case CYGNUM_HAL_VECTOR_UNALIGNED_ACCESS:
        // Access to non-existent physical memory/device
    case CYGNUM_HAL_VECTOR_BUS_ERROR:
        return SIGBUS;

        // TRAP instruction executed
    case CYGNUM_HAL_VECTOR_TRAP:
        return SIGTRAP;

        /* System call instruction executed */
    case CYGNUM_HAL_VECTOR_SYSTEM_CALL:
        return SIGSYS;

        /* Tick timer interrupt fired */
    case CYGNUM_HAL_VECTOR_TICK_TIMER:
        return SIGALRM;

        /* External interrupt */
    case CYGNUM_HAL_VECTOR_INTERRUPT:
      return SIGINT;

      // Illegal or reserved instruction
    case CYGNUM_HAL_VECTOR_RESERVED_INSTRUCTION:
        return SIGILL;

        // Numeric overflow, etc.
    case CYGNUM_HAL_VECTOR_RANGE:
        return SIGFPE;

    default:
        return SIGTERM;
    }
}


/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return _hal_registers->vector;
}

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}



/*----------------------------------------------------------------------
 * Single-step support
 */

// Type of a single OpenRISC instruction
typedef cyg_uint32 t_inst;

/* Saved instruction data for single step support.  */
static struct
{
  t_inst *targetAddr;
  t_inst savedInstr;
} instrBuffer;


/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

void __single_step (void)
{
  t_inst *pc = (t_inst *) get_register (PC);
  t_inst *targetAddr;
  InstFmt insn;
  int flag;

  targetAddr = pc + 1;              /* set default */

  insn.word = *pc;
  switch (insn.JType.op) {
  case OP_J:
  case OP_JAL:
    targetAddr = pc + insn.JType.target;
    break;

  case OP_BNF:
    flag = get_register(SR) & SPR_SR_F;
    if (!flag)
      targetAddr = pc + insn.JType.target;
    break;

  case OP_BF:
    flag = get_register(SR) & SPR_SR_F;
    if (flag)
      targetAddr = pc + insn.JType.target;
    break;

  case OP_JR:
  case OP_JALR:
    targetAddr = (t_inst*)get_register(insn.JRType.rB);
    break;

    /* We don't step into interrupts, syscalls or traps */
  default:
    break;
  }

  instrBuffer.targetAddr = targetAddr;
  instrBuffer.savedInstr = *targetAddr;
  *targetAddr = __break_opcode ();

  // No need to flush caches; Generic stub code will handle this.
}
  
/* Clear the single-step state. */
void __clear_single_step (void)
{
  if (instrBuffer.targetAddr != NULL)
    {
      *instrBuffer.targetAddr = instrBuffer.savedInstr;
      instrBuffer.targetAddr = NULL;
      instrBuffer.savedInstr = 0;
    }
}


void __install_breakpoints ()
{
  /*  if (instrBuffer.targetAddr != NULL)
    {
      instrBuffer.savedInstr = *instrBuffer.targetAddr;
      *instrBuffer.targetAddr = __break_opcode ();
      } */

  /* Install the breakpoints in the breakpoint list */
  __install_breakpoint_list();

  // No need to flush caches here; Generic stub code will handle this.
}

void __clear_breakpoints (void)
{
  __clear_breakpoint_list();
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

target_register_t 
get_register (regnames_t reg)
{
    GDB_Registers* gdb_regs;

    gdb_regs = (GDB_Registers*)_registers;

    if (reg >= R0 && reg <= R31)
        return gdb_regs->r[reg];
    if (reg == PC)
        return gdb_regs->pc;
    if (reg == SR)
        return gdb_regs->sr;
    return 0xdeadbeef;
}

void 
put_register (regnames_t reg, target_register_t value)
{
    GDB_Registers* gdb_regs;

    gdb_regs = (GDB_Registers*)_registers;

    if (reg >= R0 && reg <= R31) {
        gdb_regs->r[reg] = value;
    } else if (reg == PC) {
        gdb_regs->pc = value;
    } else if (reg == SR) {
        gdb_regs->sr = value;
    } else {
        CYG_FAIL("Attempt to write to non-existent register ");
    }
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

// EOF openrisc_stub.c
