//========================================================================
//
//      frv_stub.c
//
//      Helper functions for stub, generic to all FUJITSU processors
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
// Author(s):     Red Hat, gthomas
// Contributors:  Red Hat, gthomas, jskov, msalter
// Date:          2001-09-16
// Purpose:       
// Description:   Helper functions for stub, generic to all FUJITSU processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

#if defined(CYGNUM_HAL_BREAKPOINT_LIST_SIZE) && (CYGNUM_HAL_BREAKPOINT_LIST_SIZE > 0)
cyg_uint32 __frv_breakinst = HAL_BREAKINST;
#endif

// Sadly, this doesn't seem to work on the FRV400 either
//#define USE_HW_STEP  

#ifdef CYGSEM_HAL_FRV_HW_DEBUG
static inline unsigned __get_dcr(void)
{
    unsigned retval;

    asm volatile (
        "movsg   dcr,%0\n"
        : "=r" (retval)
        : /* no inputs */  );

    return retval;
}

static inline void __set_dcr(unsigned val)
{
    asm volatile (
        "movgs   %0,dcr\n"
        : /* no outputs */
        : "r" (val) );
}

#endif

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    // should also catch CYGNUM_HAL_VECTOR_UNDEF_INSTRUCTION here but we
    // can't tell the different between a real one and a breakpoint :-(
    switch (trap_number) {
      // Interrupts
    case CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1 ... CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_15:
        return SIGINT;
    case CYGNUM_HAL_VECTOR_INSTR_ACCESS_MMU_MISS:
    case CYGNUM_HAL_VECTOR_INSTR_ACCESS_ERROR:
    case CYGNUM_HAL_VECTOR_INSTR_ACCESS_EXCEPTION:
    case CYGNUM_HAL_VECTOR_MEMORY_ADDRESS_NOT_ALIGNED:
    case CYGNUM_HAL_VECTOR_DATA_ACCESS_ERROR:
    case CYGNUM_HAL_VECTOR_DATA_ACCESS_MMU_MISS:
    case CYGNUM_HAL_VECTOR_DATA_ACCESS_EXCEPTION:
    case CYGNUM_HAL_VECTOR_DATA_STORE_ERROR:
        return SIGBUS;
    case CYGNUM_HAL_VECTOR_PRIVELEDGED_INSTRUCTION:
    case CYGNUM_HAL_VECTOR_ILLEGAL_INSTRUCTION:
    case CYGNUM_HAL_VECTOR_REGISTER_EXCEPTION:
    case CYGNUM_HAL_VECTOR_FP_DISABLED:
    case CYGNUM_HAL_VECTOR_MP_DISABLED:
    case CYGNUM_HAL_VECTOR_FP_EXCEPTION:
    case CYGNUM_HAL_VECTOR_MP_EXCEPTION:
    case CYGNUM_HAL_VECTOR_DIVISION_EXCEPTION:
    case CYGNUM_HAL_VECTOR_COMMIT_EXCEPTION:
    case CYGNUM_HAL_VECTOR_COMPOUND_EXCEPTION:
      return SIGILL;
    default:
        return SIGTRAP;
    }
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
  // Might want to be more specific here
  return (_hal_registers->vector == CYGNUM_HAL_VECTOR_SYSCALL);
}
#endif // defined(CYGSEM_REDBOOT_BSP_SYSCALLS)

/* Set the currently-saved pc register value to PC. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}


/*----------------------------------------------------------------------
 * Single-step support
 */

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

#ifndef CYGSEM_HAL_FRV_HW_DEBUG
#if CYGINT_HAL_FRV_ARCH_FR400 == 1
#define VLIW_DEPTH 2
#endif
#if CYGINT_HAL_FRV_ARCH_FR500 == 1
#define VLIW_DEPTH 4
#endif
/*
 * Structure to hold opcodes hoisted when breakpoints are
 * set for single-stepping or async interruption.
 */
struct _bp_save {
    unsigned long  *addr;
    unsigned long   opcode;
};

/*
 * We single-step by setting breakpoints.
 *
 * This is where we save the original instructions.
 */
static struct _bp_save step_bp[VLIW_DEPTH+1];

//**************************************************************
//************ CAUTION!! ***************************************
//**************************************************************
//
// Attempt to analyze the current instruction.  This code is not
// perfect in the case of VLIW sequences, although it's close.
// Consider these sequences:
//
//      ldi.p   @(gr5,0),gr4
//      jmpl    @(gr4,gr0)
// and 
//
//      ldi.p   @(gr5,0),gr4
//      add.p   gr6,gr7,gr8
//      jmpl    @(gr4,gr0)
//
// In these cases, the only way to effectively calculate the 
// target address (of the jump) would be to simulate the actions
// of the pipelined instructions which come beforehand.
//
// Of course, this only affects single stepping through a VLIW
// sequence which contains such pipelined effects and a branch.
// Hopefully this is rare.
//
// Note: testing of the above sequence on the FR400 yielded an
// illegal instruction (invalid VLIW sequence), so this may not
// turn out to be a problem in practice, just theory.
//
//**************************************************************
//**************************************************************

static int
_analyze_instr(unsigned long pc, unsigned long *targ, 
               unsigned long *next, int *is_vliw)
{
    unsigned long opcode;
    int n, is_branch = 0;

    opcode = *(unsigned long *)pc;
    switch ((opcode >> 18) & 0x7f) {
    case 6:
    case 7:
        /* bcc, fbcc */
        is_branch = 1;
        n = (int)(opcode << 16);
        n >>= 16;
        *targ = pc + n*4;
        pc += 4;
        break;
    case 12:
        /* jmpl */
        n = (int)(get_register((opcode>>12)&63));
        n += (int)(get_register(opcode&63));
        pc = n;
        break;
    case 13:
        /* jmpil */
        n = (int)(get_register((opcode>>12)&63));
        n += (((int)(opcode << 20)) >> 20);
        pc = n;
        break;
    case 15:
        /* call */
        n = (opcode >> 25) << 18;
        n |= (opcode & 0x3ffff);
        n <<= 8;
        n >>= 8;
        pc += n*4;
        break;
    case 14:
        /* ret */
        is_branch = 1;
        *targ = get_register(LR);
        pc += 4;
        break;
    default:
        pc += 4;
        break;
    }
    *next = pc;
    *is_vliw = (opcode & 0x80000000) == 0;
    return is_branch;
}
#endif

void __single_step (void)
{
#ifdef CYGSEM_HAL_FRV_HW_DEBUG
    __set_dcr(__get_dcr() | _DCR_SE);
    diag_printf("Setting single step - DCR: %x\n", __get_dcr());
#else
    unsigned long pc, targ, next_pc;
    int i, is_branch = 0;
    int is_vliw;

    for (i = 0;  i < VLIW_DEPTH+1;  i++) {
        step_bp[i].addr = NULL;
    }

    pc = get_register(PC);
    i = 1;
    while (i < (VLIW_DEPTH+1)) {
        is_branch = _analyze_instr(pc, &targ, &next_pc, &is_vliw);
        if (is_branch && next_pc != targ) {
            step_bp[i].addr = (unsigned long *)targ;
            step_bp[i].opcode = *(unsigned long *)targ;
            *(unsigned long *)targ = HAL_BREAKINST;
            HAL_DCACHE_STORE(targ, 4);
            HAL_ICACHE_INVALIDATE(targ, 4);
        }
        if (is_vliw) {
            pc += 4;
            i++;
        } else {
            break;
        }
    }
    step_bp[0].addr = (unsigned long *)next_pc;
    step_bp[0].opcode = *(unsigned long *)next_pc;
    *(unsigned long *)next_pc = HAL_BREAKINST;
    HAL_DCACHE_STORE(next_pc, 4);
    HAL_ICACHE_INVALIDATE(next_pc, 4);
#endif
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
#ifdef CYGSEM_HAL_FRV_HW_DEBUG
    __set_dcr(__get_dcr() & ~_DCR_SE);
#else
    struct _bp_save *p;
    int i;

    for (i = 0;  i < VLIW_DEPTH+1;  i++) {
        p = &step_bp[i];
        if (p->addr) {
            *(p->addr) = p->opcode;
            HAL_DCACHE_STORE((cyg_uint32)p->addr, 4);
            HAL_ICACHE_INVALIDATE((cyg_uint32)p->addr, 4);
            p->addr = NULL;
        }
    }
#endif
}

void __install_breakpoints (void)
{
#if defined(CYGNUM_HAL_BREAKPOINT_LIST_SIZE) && (CYGNUM_HAL_BREAKPOINT_LIST_SIZE > 0)
    /* Install the breakpoints in the breakpoint list */
    __install_breakpoint_list();
#endif
}

void __clear_breakpoints (void)
{
#if defined(CYGNUM_HAL_BREAKPOINT_LIST_SIZE) && (CYGNUM_HAL_BREAKPOINT_LIST_SIZE > 0)
    __clear_breakpoint_list();
#endif
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
    unsigned long pc = get_register(PC);

    pc += 4;
    put_register(PC, pc);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
