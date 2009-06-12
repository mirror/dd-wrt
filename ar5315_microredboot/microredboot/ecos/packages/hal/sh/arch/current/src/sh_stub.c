//==========================================================================
//
//      sh_stub.c
//
//      GDB Stub code for SH
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
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
// Contributors: jskov, Ben Lee, Steve Chamberlain, nickg
// Date:         1999-05-18
// Description:  GDB Stub support for sh CPU.
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/sh_regs.h>

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

/* Given a trap value TRAP, return the corresponding signal. */

int 
__computeSignal(unsigned int trap_number)
{
    switch( trap_number ) {
    case CYGNUM_HAL_EXCEPTION_TRAP:
#ifdef CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP
    case CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP:
#endif
        return SIGTRAP;

    case CYGNUM_HAL_EXCEPTION_POWERON:
    case CYGNUM_HAL_EXCEPTION_RESET:
        // Reset - given that the CPU resets if it gets confused we
        // want to treat it as an interupt so the developer has a
        // chance to find out what happened (as opposed to SIGTERM).
        return SIGINT;

#ifdef CYGNUM_HAL_EXCEPTION_TLBMISS_ACCESS
    case CYGNUM_HAL_EXCEPTION_TLBMISS_ACCESS:
    case CYGNUM_HAL_EXCEPTION_TLBMISS_WRITE:
    case CYGNUM_HAL_EXCEPTION_INITIAL_WRITE:
    case CYGNUM_HAL_EXCEPTION_TLBERROR_ACCESS:
    case CYGNUM_HAL_EXCEPTION_TLBERROR_WRITE:
        return SIGSEGV;

    case CYGNUM_HAL_EXCEPTION_DATA_ACCESS:
    case CYGNUM_HAL_EXCEPTION_DATA_WRITE:
           return SIGBUS;
#else
    case CYGNUM_HAL_EXCEPTION_DATA_ACCESS:
    case CYGNUM_HAL_EXCEPTION_DMA_DATA_ACCESS:
        return SIGSEGV;
#endif

    case CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION:
    case CYGNUM_HAL_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION:
        return SIGILL;

    default:
        return SIGTERM;
    }
}

/* Return the trap number corresponding to the last-taken trap. */

int 
__get_trap_number(void)
{
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return _hal_registers->event;
}

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
int __is_bsp_syscall(void) 
{
#if defined(CYGARC_REG_TRA)    
    if (_hal_registers->event == 11 &&
        (*(CYG_WORD *)CYGARC_REG_TRA) == (34<<2))
#else
    if (_hal_registers->event == 34 )
#endif
        return 1;
    else
        return 0;
}
#endif

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void 
set_pc(target_register_t pc)
{
    put_register(PC, pc);
}

#ifdef CYGARC_SH_MOD_UBC

// This implementation of the single-stepper relies on the User Break
// Controller which may not be available on all cores.

// Note: This should be enhanced to coorperate with either two regular
// breakpoints or watchpoints. Requires GDB to be aware of the stub's
// ability though, so for now just use channel A without further
// considerations.

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

void __single_step (void)
{
    // The address of the instruction to execute.
    HAL_WRITE_UINT32(CYGARC_REG_BARA, get_register(PC));
    // Match entire address.
#if (CYGARC_SH_MOD_UBC == 1)
    HAL_WRITE_UINT8(CYGARC_REG_BAMRA, CYGARC_REG_BAMRA_BARA_UNMASKED);
#elif (CYGARC_SH_MOD_UBC == 2)
    // For some reason, matching all bits causes the system to hang
    // (not just run amok - it appears to stop execution).
    //HAL_WRITE_UINT32(CYGARC_REG_BAMRA, 0xffffffff);
    HAL_WRITE_UINT32(CYGARC_REG_BAMRA, 0);
#endif

    // Stop after instruction at matching address has executed.
#if (CYGARC_SH_MOD_UBC == 1)
    HAL_WRITE_UINT16(CYGARC_REG_BRCR, CYGARC_REG_BRCR_ONE_STEP);
#else
    HAL_WRITE_UINT32(CYGARC_REG_BRCR, CYGARC_REG_BRCR_ONE_STEP);
#endif

    // Stop on IFETCH/READ
#if (CYGARC_SH_MOD_UBC == 1)
    HAL_WRITE_UINT16(CYGARC_REG_BBRA, 
                     CYGARC_REG_BBRA_IFETCH|CYGARC_REG_BBRA_READ);
#else
    HAL_WRITE_UINT16(CYGARC_REG_BBRA, 
                     CYGARC_REG_BBRA_CPU|CYGARC_REG_BBRA_IFETCH|CYGARC_REG_BBRA_READ);
#endif

#ifdef CYGPKG_HAL_SH_SH4
    // Must execute at least 11 instructions before reaching
    // any address that may be affected by the UBC settings.
    asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
#endif
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
    // Don't stop on any condition
    HAL_WRITE_UINT16(CYGARC_REG_BBRA, 0);
    // Clear status flags
    HAL_WRITE_UINT16(CYGARC_REG_BRCR, 0);

#ifdef CYGPKG_HAL_SH_SH4
    // Must execute at least 11 instructions before reaching
    // any address that may be affected by the UBC settings.
    asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
#endif
}

#else // CYGARC_SH_MOD_UBC

/*----------------------------------------------------------------------
 * Single-step support, copied from gdb/sh-stub.c, written by Ben Lee
 * and Steve Chamberlain. Extended to handle braf, bsrf, bt/s and bf/s.
 */

#define COND_BRx_MASK           0xfb00
#define UCOND_DBR_MASK          0xe000
#define UCOND_RBR_MASK          0xf0df
#define TRAPA_MASK              0xff00

#define COND_DISP               0x00ff
#define UCOND_DISP              0x0fff
#define UCOND_REG               0x0f00

#define BFx_INSTR               0x8b00
#define BTx_INSTR               0x8900
#define BRA_INSTR               0xa000
#define BSR_INSTR               0xb000
#define JMP_INSTR               0x402b
#define JSR_INSTR               0x400b
#define RTS_INSTR               0x000b
#define RTE_INSTR               0x002b
#define TRAPA_INSTR             0xc300
#define BxxF_INSTR              0x0003

#define SSTEP_INSTR             0xc320

#define T_BIT_MASK              0x0001

typedef struct {
    short *memAddr;
    short oldInstr;
} stepData;

static stepData instrBuffer;
static char stepped;

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

void __single_step (void)
{
    short *instrMem;
    int displacement;
    int reg;
    unsigned short opcode;

    cyg_uint32 pc = get_register(PC);
    cyg_uint32 sr = get_register(SR);

    instrMem = (short *) pc;

    opcode = *instrMem;
    stepped = 1;

    if ((opcode & UCOND_RBR_MASK) == BxxF_INSTR) { 
        reg = (char) ((opcode & UCOND_REG) >> 8);
        displacement = get_register(reg);
        instrMem = (short *) (pc + displacement + 4);
            /*
             * Remember PC points to second instr.
             * after PC of branch ... so add 4
             */
    } else if ((opcode & COND_BRx_MASK) == BTx_INSTR) {
        if (sr & T_BIT_MASK) {
            displacement = (opcode & COND_DISP) << 1;
            if (displacement & 0x80) {
                displacement |= 0xffffff00;
            }
            /*
             * Remember PC points to second instr.
             * after PC of branch ... so add 4
             */
            instrMem = (short *) (pc + displacement + 4);
        } else {
            instrMem += 1;
        }
    } else if ((opcode & COND_BRx_MASK) == BFx_INSTR) {
        if (sr & T_BIT_MASK) {
            instrMem += 1;
        } else {
            displacement = (opcode & COND_DISP) << 1;
            if (displacement & 0x80)
                displacement |= 0xffffff00;
            /*
             * Remember PC points to second instr.
             * after PC of branch ... so add 4
             */
            instrMem = (short *) (pc + displacement + 4);
        }
    } else if ((opcode & UCOND_DBR_MASK) == BRA_INSTR) {
        displacement = (opcode & UCOND_DISP) << 1;
        if (displacement & 0x0800) {
            displacement |= 0xfffff000;
        }
        
        /*
         * Remember PC points to second instr.
         * after PC of branch ... so add 4
         */
        instrMem = (short *) (pc + displacement + 4);
    } else if ((opcode & UCOND_RBR_MASK) == JSR_INSTR) {
        reg = (char) ((opcode & UCOND_REG) >> 8);
        instrMem = (short *) get_register(reg);
    } else if (opcode == RTS_INSTR) {
        instrMem = (short *) get_register(PR);
    } else if (opcode == RTE_INSTR) {
        instrMem = (short *) get_register(SP);
    } else if ((opcode & TRAPA_MASK) == TRAPA_INSTR) {
        instrMem += 1;                  // skip traps
    } else {
        instrMem += 1;
    }
    
    instrBuffer.memAddr = instrMem;
    instrBuffer.oldInstr = *instrMem;
    *instrMem = SSTEP_INSTR;
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
    /* Undo the effect of a previous doSStep.  If we single stepped,
       restore the old instruction. */

    if (stepped) {
        short *instrMem;
        instrMem = instrBuffer.memAddr;
        *instrMem = instrBuffer.oldInstr;
    }
    stepped = 0;
}

#endif // CYGARC_SH_MOD_UBC


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

#ifdef linux
externC void _breakinst(void);
int
__is_breakpoint_function ()
{
    return get_register(PC) == (target_register_t)&_breakinst;
}
#else
externC void breakinst(void);
int
__is_breakpoint_function ()
{
    return get_register(PC) == (target_register_t)&breakinst;
}
#endif

/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
   we can safely just skip 2. */
void __skipinst (void)
{
    put_register(PC, get_register (PC) + 2);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
