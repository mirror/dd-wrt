//========================================================================
//
//      v850_stub.c
//
//      Helper functions for stub, generic to all NEC processors
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
// Author(s):     Red Hat, gthomas, jlarmour
// Contributors:  Red Hat, gthomas, jskov
// Date:          1998-11-26
// Purpose:       
// Description:   Helper functions for stub, generic to all NEC processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>
#ifdef CYGPKG_CYGMON
#include <pkgconf/cygmon.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    unsigned short curins, *pc;
    switch (trap_number) {
    case CYGNUM_HAL_VECTOR_INTWDT: // watchdog timer NMI
        pc = (unsigned short *)_hal_registers->pc;
        curins = *pc;
        if (curins == 0x0585) {
            // "br *" - used for breakpoint
            return SIGTRAP;
        } else {
            // Anything else - just ignore it happened
            return 0;
        }
    case CYGNUM_HAL_VECTOR_NMI:
        return SIGINT;
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

static unsigned short *ss_saved_pc = 0;
static unsigned short  ss_saved_instr[2];
static int             ss_saved_instr_size;

#define FIXME() {diag_printf("FIXME - %s\n", __FUNCTION__); }

static unsigned short *
next_pc(unsigned short *pc)
{
    unsigned short curins = *pc;
    unsigned short *newpc = pc;

    switch ((curins & 0x0780) >> 7) {
    case 0x0:
        if ((curins & 0x60) == 0x60) {
            int Rn = curins & 0x1F;
            newpc = (unsigned short *)get_register(Rn);
        } else {
            newpc = pc+1;
        }
        break;
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
        // Arithmetic - no branch opcodes
        newpc = pc+1;
        break;
    case 0x6:
    case 0x7:
    case 0x8:
    case 0x9:
    case 0xA:
        // Load and store - no branch opcodes
        newpc = pc+2;
        break;
    case 0xB:
        // Conditional branch
        if (1) {
            unsigned long psw = get_register(PSW);
#define PSW_SAT 0x10
#define PSW_CY  0x08
#define PSW_OV  0x04
#define PSW_S   0x02
#define PSW_Z   0x01
            long disp = ((curins & 0xF800) >> 8) | ((curins & 0x70) >> 4);    
            int cc = curins & 0x0F;
            int S = (psw & PSW_S) != 0;
            int Z = (psw & PSW_Z) != 0;
            int OV = (psw & PSW_OV) != 0;
            int CY = (psw & PSW_CY) != 0;
            int do_branch = 0;
            if (curins & 0x8000) disp |= 0xFFFFFF00;
            switch (cc) {
            case 0x0: // BV
                do_branch = (OV == 1);
                break;
            case 0x1: // BL
                do_branch = (CY == 1);
                break;
            case 0x2: // BE
                do_branch = (Z == 1);
                break;
            case 0x3: // BNH
                do_branch = ((CY | Z) == 1);
                break;
            case 0x4: // BN
                do_branch = (S == 1);
                break;
            case 0x5: // - always
                do_branch = 1;
                break;
            case 0x6: // BLT
                do_branch = ((S ^ OV) == 1);
                break;
            case 0x7: // BLE
                do_branch = (((S ^ OV) | Z) == 1);
                break;
            case 0x8: // BNV
                do_branch = (OV == 0);
                break;
            case 0x9: // BNL
                do_branch = (CY == 0);
                break;
            case 0xA: // BNE
                do_branch = (Z == 0);
                break;
            case 0xB: // BH
                do_branch = ((CY | Z) == 0);
                break;
            case 0xC: // BP
                do_branch = (S == 0);
                break;
            case 0xD: // BSA
                do_branch = ((psw & PSW_SAT) != 0);
                break;
            case 0xE: // BGE
                do_branch = ((S ^ OV) == 0);
                break;
            case 0xF: // BGT
                do_branch = (((S ^ OV) | Z) == 0);
                break;
            }
            if (do_branch) {
                newpc = pc + disp;
            } else {
                newpc = pc + 1;
            }
        }
        break;
    case 0xC:
    case 0xD:
    case 0xE:
        // Arithmetic & load/store - no branch opcodes
        newpc = pc+2;
        break;
    case 0xF:
        if ((curins & 0x60) >= 0x40) {
            // Bitfield and extended instructions - no branch opcodes
            newpc = pc+2;
        } else {
            // JR/JARL
            long disp = ((curins & 0x3F) << 16) | *(pc+1);
            if (curins & 0x20) disp |= 0xFFC00000;
            newpc = pc + (disp>>1);
        }
    }
    return newpc;
}

void __single_step (void)
{
    unsigned short *pc = (unsigned short *)get_register(PC);
    unsigned short *break_pc;
    unsigned short  _breakpoint[] = {0x07E0, 0x0780};
    unsigned short *breakpoint = _breakpoint;
    // If the current instruction is a branch, decide if the branch will
    // be taken to determine where to set the breakpoint.
    break_pc = next_pc(pc);
    // Now see what kind of breakpoint can be used.
    // Note: since this is a single step, always use the 32 bit version.
    ss_saved_pc = break_pc;
    ss_saved_instr_size = 2;
    ss_saved_instr[0] = *break_pc;
    *break_pc++ = *breakpoint++;
    ss_saved_instr[1] = *break_pc;
    *break_pc++ = *breakpoint++;
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
    unsigned short *pc, *val;
    int i;
    if (ss_saved_instr_size != 0) {
        pc = ss_saved_pc;
        val = ss_saved_instr;
        for (i = 0;  i < ss_saved_instr_size;  i++) {
            *pc++ = *val++;
        }
        ss_saved_instr_size = 0;
    }
}

#if !defined(CYGPKG_CYGMON)
void __install_breakpoints (void)
{
//    FIXME();
}

void __clear_breakpoints (void)
{
//    FIXME();
}
#endif // !CYGPKG_CYGMON

/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */

int
__is_breakpoint_function ()
{
    return get_register (PC) == (target_register_t)&_breakinst;
}


/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
*/

void __skipinst (void)
{
    unsigned short *pc = (unsigned short *)get_register(PC);
    pc = next_pc(pc);
    put_register(PC, (unsigned long)pc);
}


#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

// EOF v850_stub.c
