//========================================================================
//
//      mips-stub.h
//
//      Helper functions for stub, generic to all MIPS processors
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
// Contributors:  Red Hat, nickg
// Date:          1998-06-08
// Purpose:       
// Description:   Helper functions for stub, generic to all MIPS processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/mips-regs.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/mips_opcode.h>

typedef unsigned long t_inst;

/*----------------------------------------------------------------------
 * Asynchronous interrupt support
 */

static struct
{
  t_inst *targetAddr;
  t_inst savedInstr;
} asyncBuffer;

/* Called to asynchronously interrupt a running program.
   Must be passed address of instruction interrupted.
   This is typically called in response to a debug port
   receive interrupt.
*/

void
install_async_breakpoint(void *epc)
{
  long gp_save;

  /* This may be called from a separately linked program,
     so we need to save and restore the incoming gp register
     and setup our stub local gp register. 
     Alternatively, we could skip this foolishness if we
     compiled libstub with "-G 0". */

  __asm__ volatile ( "move   %0,$28\n"
		     ".extern _gp\n"
		     "la     $28,_gp\n"
		     : "=r" (gp_save) );

  asyncBuffer.targetAddr = epc;
  asyncBuffer.savedInstr = *(t_inst *)epc;
  *(t_inst *)epc = *(t_inst *)_breakinst;
  __instruction_cache(CACHE_FLUSH);
  __data_cache(CACHE_FLUSH);

  __asm__  volatile ( "move   $28,%0\n" :: "r"(gp_save) );
}

/*--------------------------------------------------------------------*/
/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
  switch (trap_number)
    {
    case EXC_INT:
      /* External interrupt */
      return SIGINT;

    case EXC_RI:
      /* Reserved instruction */
    case EXC_CPU:
      /* Coprocessor unusable */
      return SIGILL;

    case EXC_BP:
      /* Break point */
      if (asyncBuffer.targetAddr != NULL)
	{
	  /* BP installed by serial driver to stop running program */
	  *asyncBuffer.targetAddr = asyncBuffer.savedInstr;
          __instruction_cache(CACHE_FLUSH);
          __data_cache(CACHE_FLUSH);
	  asyncBuffer.targetAddr = NULL;
	  return SIGINT;
	}
      return SIGTRAP;

    case EXC_OVF:
      /* Arithmetic overflow */
    case EXC_TRAP:
      /* Trap exception */
    case EXC_FPE:
      /* Floating Point Exception */
      return SIGFPE;

    case EXC_IBE:
      /* Bus error (Ifetch) */
    case EXC_DBE:
      /* Bus error (data load or store) */
      return SIGBUS;

    case EXC_MOD:
      /* TLB modification exception */
    case EXC_TLBL:
      /* TLB miss (Load or Ifetch) */
    case EXC_TLBS:
      /* TLB miss (Store) */
    case EXC_ADEL:
      /* Address error (Load or Ifetch) */
    case EXC_ADES:
      /* Address error (Store) */
      return SIGSEGV;

    case EXC_SYS:
      /* System call */
      return SIGSYS;

    default:
      return SIGTERM;
    }
}

/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
  return (get_register (CAUSE) & CAUSE_EXCMASK) >> CAUSE_EXCSHIFT;
}

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
int __is_bsp_syscall(void) 
{
    return __get_trap_number() == EXC_SYS;
}
#endif

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void set_pc (target_register_t pc)
{
  put_register (PC, pc);
}


/*----------------------------------------------------------------------
 * Single-step support
 */

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
  InstFmt inst;
  t_inst *pc = (t_inst *) get_register (PC);

  instrBuffer.targetAddr = pc + 1;              /* set default */

  inst.word = *pc;                              /* read the next instruction  */

//diag_printf("pc %08x %08x\n",pc,inst.word);
 
  switch (inst.RType.op) {                    /* override default if branch */
    case OP_SPECIAL:
      switch (inst.RType.func) {
        case OP_JR:
        case OP_JALR:
          instrBuffer.targetAddr = (t_inst *) get_register (inst.RType.rs);
          break;
      };
      break;

    case OP_REGIMM:
      switch (inst.IType.rt) {
        case OP_BLTZ:
        case OP_BLTZL:
        case OP_BLTZAL:
        case OP_BLTZALL:
            if ((int)get_register (inst.IType.rs) < 0 )
            instrBuffer.targetAddr =
              (t_inst *)(((signed short)inst.IType.imm<<2)
                                        + (get_register (PC) + 4));
          else
            instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
          break;
        case OP_BGEZ:
        case OP_BGEZL:
        case OP_BGEZAL:
        case OP_BGEZALL:
            if ((int)get_register (inst.IType.rs) >= 0 )
            instrBuffer.targetAddr =
              (t_inst *)(((signed short)inst.IType.imm<<2)
                                        + (get_register (PC) + 4));
          else
            instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
          break;
      };
      break;

    case OP_J:
    case OP_JAL:
      instrBuffer.targetAddr =
        (t_inst *)((inst.JType.target<<2)
                   + ((get_register (PC) + 4)&0xf0000000));
      break;

    case OP_BEQ:
    case OP_BEQL:
      if (get_register (inst.IType.rs) == get_register (inst.IType.rt))
        instrBuffer.targetAddr =
          (t_inst *)(((signed short)inst.IType.imm<<2)
                     + (get_register (PC) + 4));
      else
        instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
      break;
    case OP_BNE:
    case OP_BNEL:
      if (get_register (inst.IType.rs) != get_register (inst.IType.rt))
        instrBuffer.targetAddr =
          (t_inst *)(((signed short)inst.IType.imm<<2)
                     + (get_register (PC) + 4));
      else
        instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
      break;
    case OP_BLEZ:
    case OP_BLEZL:
        if ((int)get_register (inst.IType.rs) <= 0)
        instrBuffer.targetAddr =
          (t_inst *)(((signed short)inst.IType.imm<<2) + (get_register (PC) + 4));
      else
        instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
      break;
    case OP_BGTZ:
    case OP_BGTZL:
        if ((int)get_register (inst.IType.rs) > 0)
        instrBuffer.targetAddr =
          (t_inst *)(((signed short)inst.IType.imm<<2) + (get_register (PC) + 4));
      else
        instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
      break;

#ifdef CYGHWR_HAL_MIPS_FPU

    case OP_COP1:
      if (inst.RType.rs == OP_BC)
          switch (inst.RType.rt) {
            case COPz_BCF:
            case COPz_BCFL:
                if (get_register (FCR31) & FCR31_C)
                instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
              else
                instrBuffer.targetAddr =
                  (t_inst *)(((signed short)inst.IType.imm<<2)
                                            + (get_register (PC) + 4));
              break;
            case COPz_BCT:
            case COPz_BCTL:
                if (get_register (FCR31) & FCR31_C)
                instrBuffer.targetAddr =
                  (t_inst *)(((signed short)inst.IType.imm<<2)
                                            + (get_register (PC) + 4));
              else
                instrBuffer.targetAddr = (t_inst *)(get_register (PC) + 8);
              break;
          };
      break;
#endif

  }
}


/* Clear the single-step state. */

void __clear_single_step (void)
{
//diag_printf("clear_ss ta %08x\n",instrBuffer.targetAddr);
  if (instrBuffer.targetAddr != NULL)
    {
      *instrBuffer.targetAddr = instrBuffer.savedInstr;
      instrBuffer.targetAddr = NULL;
    }
  instrBuffer.savedInstr = NOP_INSTR;
}


void __install_breakpoints ()
{
//diag_printf("install_bpt ta %08x\n",instrBuffer.targetAddr);
  if (instrBuffer.targetAddr != NULL)
    {
      instrBuffer.savedInstr = *instrBuffer.targetAddr;
      *instrBuffer.targetAddr = __break_opcode ();
//diag_printf("ta %08x si %08x *ta %08x\n",
//            instrBuffer.targetAddr,instrBuffer.savedInstr,*instrBuffer.targetAddr);

      /* Ensure that the planted breakpoint makes it out to memory and
         is subsequently loaded into the instruction cache.
      */
      __data_cache (CACHE_FLUSH) ;
      __instruction_cache (CACHE_FLUSH) ;
    }

  /* Install the breakpoints in the breakpoint list */
  __install_breakpoint_list();
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
    return get_register (PC) == (target_register_t)(unsigned long)&_breakinst;
}


/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
   we can safely just skip 4. */

void __skipinst (void)
{
    put_register (PC, get_register (PC) + 4);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
