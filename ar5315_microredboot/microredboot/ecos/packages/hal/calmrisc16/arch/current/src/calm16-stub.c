//========================================================================
//
//      calm16-stub.h
//
//      Helper functions for stub, generic to all CalmRISC16 processors
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
// Author(s):     Red Hat, msalter
// Contributors:  Red Hat, msalter
// Date:          2001-02-12
// Purpose:       
// Description:   Helper functions for stub, generic to CalmRISC16 processors
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

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

typedef cyg_uint16 t_inst;

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
install_async_breakpoint(void *pc)
{
  asyncBuffer.targetAddr = pc;
  asyncBuffer.savedInstr = *(t_inst *)pc;
  *(t_inst *)pc = *(t_inst *)_breakinst;
  __instruction_cache(CACHE_FLUSH);
  __data_cache(CACHE_FLUSH);
}

/*--------------------------------------------------------------------*/
/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    switch (trap_number) {
      case CYGNUM_HAL_VECTOR_FIQ:
//    case CYGNUM_HAL_VECTOR_IRQ:
	return SIGINT;
    }
    return SIGTRAP;
}

/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return _hal_registers->vector;
}

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
int __is_bsp_syscall(void) 
{
    return __get_trap_number() >= CYGNUM_HAL_VECTOR_SWI;
}
#endif


/* Set the current pc register value based on current vector. */

void set_pc (target_register_t pc)
{
    put_register (REG_PC, pc);
    switch (__get_trap_number()) {
      case CYGNUM_HAL_VECTOR_FIQ:
	put_register (REG_SPC_FIQ, pc);
	break;
      case CYGNUM_HAL_VECTOR_SWI:
	put_register (REG_LR, pc);
	break;
      default:
	put_register (REG_SPC_IRQ, pc);
	break;
    }
}

/* Get the current pc register value based on current vector. */

target_register_t get_pc(void)
{
    switch (__get_trap_number()) {
      case CYGNUM_HAL_VECTOR_SWI:
	return get_register (REG_LR);
      case CYGNUM_HAL_VECTOR_FIQ:
	return get_register (REG_SPC_FIQ);
      default:
	break;
    }
    return get_register (REG_SPC_IRQ);
}


/*----------------------------------------------------------------------
 * Single-step support
 */

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

void __single_step (void)
{
    put_register(REG_SR, get_register(REG_SR) | ((target_register_t)CYGARC_SR_TE << 16));
}


/* Clear the single-step state. */

void __clear_single_step (void)
{
    put_register(REG_SR, get_register(REG_SR) & ~((target_register_t)CYGARC_SR_TE << 16));
}


void __install_breakpoints ()
{
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
    return get_pc() == (target_register_t)(unsigned long)&_breakinst;
}


/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
   we can safely just skip 2. */

void __skipinst (void)
{
    set_pc(get_pc() + 2);
}

unsigned short __read_prog_uint16(void *addr)
{
    unsigned val;
    asm("ldc %0, @%1" : "=r"(val) : "r"(addr) );
    return val;
}

unsigned char __read_prog_uint8(void *addr)
{
    unsigned short s;
    int is_odd = ((unsigned long)addr & 1) == 1;

    s = __read_prog_uint16((void *)((unsigned long)addr & ~1));
    if (is_odd)
	return s & 0xff;
    else
	return (s >> 8) & 0xff;
}

unsigned long __read_prog_uint32(void *addr)
{
    unsigned long u;

    u = (unsigned long)__read_prog_uint16(addr) << 16;
    u |= __read_prog_uint16((void *)((unsigned long)addr + 2));

    return u;
}

void __write_prog_uint16(void *addr, unsigned short val)
{
    hal_plf_write_prog_halfword((unsigned long)addr, val);
}

void __write_prog_uint32(void *addr, unsigned long val)
{
    hal_plf_write_prog_halfword((unsigned long)addr, (val >> 16) & 0xffff);
    hal_plf_write_prog_halfword((unsigned long)addr + 2, val & 0xffff);
}

void __write_prog_uint8(void *addr, unsigned char val)
{
    unsigned short s;
    int is_odd = ((unsigned long)addr & 1) == 1;

    s = __read_prog_uint16((void *)((unsigned long)addr & ~1));

    if (is_odd)
	s = (s & 0xff00) | val;
    else
	s = (s & 0xff) | (val << 8);

    hal_plf_write_prog_halfword((unsigned long)addr & ~1, s);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
