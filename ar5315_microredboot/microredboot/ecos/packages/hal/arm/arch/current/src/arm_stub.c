//========================================================================
//
//      arm_stub.c
//
//      Helper functions for stub, generic to all ARM processors
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Contributors:  Red Hat, gthomas, jskov
// Date:          1998-11-26
// Purpose:       
// Description:   Helper functions for stub, generic to all ARM processors
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

#ifdef CYGPKG_HAL_ARM_SIM
#error "GDB Stub support not implemented for ARM SIM"
#endif

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

// Use bit 0 as a thumb-mode flag for next address to be executed.
// Alternative would be to keep track of it using a C variable, but
// since bit 0 is used by the BX instruction, we might as well do the
// same thing and thus avoid checking two different flags.
#define IS_THUMB_ADDR(addr)     ((addr) & 1)
#define MAKE_THUMB_ADDR(addr) ((addr) | 1)
#define UNMAKE_THUMB_ADDR(addr) ((addr) & ~1)

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

#if defined(CYGNUM_HAL_BREAKPOINT_LIST_SIZE) && (CYGNUM_HAL_BREAKPOINT_LIST_SIZE > 0)
cyg_uint32 __arm_breakinst = HAL_BREAKINST_ARM;
cyg_uint16 __thumb_breakinst = HAL_BREAKINST_THUMB;
#endif

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    // Check to see if we stopped because of a hw watchpoint/breakpoint.
#ifdef HAL_STUB_IS_STOPPED_BY_HARDWARE
    {
	void *daddr;
	if (HAL_STUB_IS_STOPPED_BY_HARDWARE(daddr))
	    return SIGTRAP;
    }
#endif
    // should also catch CYGNUM_HAL_VECTOR_UNDEF_INSTRUCTION here but we
    // can't tell the different between a real one and a breakpoint :-(
    switch (trap_number) {
    case CYGNUM_HAL_VECTOR_ABORT_PREFETCH:      // Fall through
    case CYGNUM_HAL_VECTOR_ABORT_DATA:          // Fall through
    case CYGNUM_HAL_VECTOR_reserved:
        return SIGBUS;
    case CYGNUM_HAL_VECTOR_IRQ:
    case CYGNUM_HAL_VECTOR_FIQ:
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


#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
int __is_bsp_syscall(void) 
{
    unsigned long pc = get_register(PC);
    unsigned long cpsr = get_register(PS);  // condition codes

    if (_hal_registers->vector == CYGNUM_HAL_EXCEPTION_INTERRUPT) {
	if (cpsr & CPSR_THUMB_ENABLE)
	    return *(unsigned short *)pc == 0xdf18;
	else
	    return *(unsigned *)pc == 0xef180001;
    }
    return 0;
}
#endif

/* Set the currently-saved pc register value to PC. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}

// Calculate byte offset a given register from start of register save area.
static int
reg_offset(regnames_t reg)
{
    int base_offset;

    if (reg < F0)
	return reg * 4;

    base_offset = 16 * 4;

    if (reg < FPS)
	return base_offset + ((reg - F0) * 12);

    base_offset += (8 * 12);

    if (reg <= PS)
	return base_offset + ((reg - FPS) * 4);

    return -1;  // Should never happen!
}


// Return the currently-saved value corresponding to register REG of
// the exception context.
target_register_t 
get_register (regnames_t reg)
{
    target_register_t val;
    int offset = reg_offset(reg);

    if (REGSIZE(reg) > sizeof(target_register_t) || offset == -1)
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

    if (REGSIZE(which) > sizeof(target_register_t) || offset == -1)
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

    if (offset != -1) {
	memcpy (value, (char *)_registers + offset, REGSIZE(which));
	return 1;
    }
    return 0;
}

// Alter the contents of saved register WHICH to contain VALUE. This
// is only used for registers larger than sizeof(target_register_t).
// Return non-zero if it is a valid register.
int
put_register_as_bytes (regnames_t which, char *value)
{
    int offset = reg_offset(which);

    if (offset != -1) {
	memcpy ((char *)_registers + offset, value, REGSIZE(which));
	return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 * Single-step support
 */

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

static unsigned long  ss_saved_pc = 0;
static unsigned long  ss_saved_instr;
static unsigned short ss_saved_thumb_instr;

#define FIXME() {diag_printf("FIXME - %s\n", __FUNCTION__); }

// return non-zero for v5 and later
static int
v5T_semantics(void)
{
    unsigned id;

    asm volatile ("mrc  p15,0,%0,c0,c0,0\n"
		  : "=r" (id) : /* no inputs */);

    return ((id >> 16) & 0xff) >= 5;
}

static int
ins_will_execute(unsigned long ins)
{
    unsigned long psr = get_register(PS);  // condition codes
    int res = 0;
    switch ((ins & 0xF0000000) >> 28) {
    case 0x0: // EQ
        res = (psr & PS_Z) != 0;
        break;
    case 0x1: // NE
        res = (psr & PS_Z) == 0;
        break;
    case 0x2: // CS
        res = (psr & PS_C) != 0;
        break;
    case 0x3: // CC
        res = (psr & PS_C) == 0;
        break;
    case 0x4: // MI
        res = (psr & PS_N) != 0;
        break;
    case 0x5: // PL
        res = (psr & PS_N) == 0;
        break;
    case 0x6: // VS
        res = (psr & PS_V) != 0;
        break;
    case 0x7: // VC
        res = (psr & PS_V) == 0;
        break;
    case 0x8: // HI
        res = ((psr & PS_C) != 0) && ((psr & PS_Z) == 0);
        break;
    case 0x9: // LS
        res = ((psr & PS_C) == 0) || ((psr & PS_Z) != 0);
        break;
    case 0xA: // GE
        res = ((psr & (PS_N|PS_V)) == (PS_N|PS_V)) ||
            ((psr & (PS_N|PS_V)) == 0);
        break;
    case 0xB: // LT
        res = ((psr & (PS_N|PS_V)) == PS_N) ||
            ((psr & (PS_N|PS_V)) == PS_V);
        break;
    case 0xC: // GT
        res = ((psr & (PS_N|PS_V)) == (PS_N|PS_V)) ||
            ((psr & (PS_N|PS_V)) == 0);
        res = ((psr & PS_Z) == 0) && res;
        break;
    case 0xD: // LE
        res = ((psr & (PS_N|PS_V)) == PS_N) ||
            ((psr & (PS_N|PS_V)) == PS_V);
        res = ((psr & PS_Z) == PS_Z) || res;
        break;
    case 0xE: // AL
        res = TRUE;
        break;
    case 0xF: // NV
        if (((ins & 0x0E000000) >> 24) == 0xA)
	    res = TRUE;
	else
	    res = FALSE;
        break;
    }
    return res;
}

static unsigned long
RmShifted(int shift)
{
    unsigned long Rm = get_register(shift & 0x00F);
    int shift_count;
    if ((shift & 0x010) == 0) {
        shift_count = (shift & 0xF80) >> 7;
    } else {
        shift_count = get_register((shift & 0xF00) >> 8);
    }
    switch ((shift & 0x060) >> 5) {
    case 0x0: // Logical left
        Rm <<= shift_count;
        break;
    case 0x1: // Logical right
        Rm >>= shift_count;
        break;
    case 0x2: // Arithmetic right
        Rm = (unsigned long)((long)Rm >> shift_count);
        break;
    case 0x3: // Rotate right
        if (shift_count == 0) {
            // Special case, RORx
            Rm >>= 1;
            if (get_register(PS) & PS_C) Rm |= 0x80000000;
        } else {
            Rm = (Rm >> shift_count) | (Rm << (32-shift_count));
        }
        break;
    }
    return Rm;
}

// Decide the next instruction to be executed for a given instruction
static unsigned long *
target_ins(unsigned long *pc, unsigned long ins)
{
    unsigned long new_pc, offset, op2;
    unsigned long Rn;
    int i, reg_count, c;

    switch ((ins & 0x0C000000) >> 26) {
    case 0x0:
        // BX or BLX
        if ((ins & 0x0FFFFFD0) == 0x012FFF10) {
            new_pc = (unsigned long)get_register(ins & 0x0000000F);
            return ((unsigned long *)new_pc);
        }
        // Data processing
        new_pc = (unsigned long)(pc+1);
        if ((ins & 0x0000F000) == 0x0000F000) {
            // Destination register is PC
            if ((ins & 0x0FBF0000) != 0x010F0000) {
                Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                if ((ins & 0x02000000) == 0) {
                    op2 = RmShifted(ins & 0x00000FFF);
                } else {
                    op2 = ins & 0x000000FF;
                    i = (ins & 0x00000F00) >> 8;  // Rotate count                
                    op2 = (op2 >> (i*2)) | (op2 << (32-(i*2)));
                }
                switch ((ins & 0x01E00000) >> 21) {
                case 0x0: // AND
                    new_pc = Rn & op2;
                    break;
                case 0x1: // EOR
                    new_pc = Rn ^ op2;
                    break;
                case 0x2: // SUB
                    new_pc = Rn - op2;
                    break;
                case 0x3: // RSB
                    new_pc = op2 - Rn;
                    break;
                case 0x4: // ADD
                    new_pc = Rn + op2;
                    break;
                case 0x5: // ADC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = Rn + op2 + c;
                    break;
                case 0x6: // SBC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = Rn - op2 + c - 1;
                    break;
                case 0x7: // RSC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = op2 - Rn +c - 1;
                    break;
                case 0x8: // TST
                case 0x9: // TEQ
                case 0xA: // CMP
                case 0xB: // CMN
                    break; // PC doesn't change
                case 0xC: // ORR
                    new_pc = Rn | op2;
                    break;
                case 0xD: // MOV
                    new_pc = op2;
                    break;
                case 0xE: // BIC
                    new_pc = Rn & ~op2;
                    break;
                case 0xF: // MVN
                    new_pc = ~op2;
                    break;
                }
            }
        }
        return ((unsigned long *)new_pc);
    case 0x1:
        if ((ins & 0x02000010) == 0x02000010) {
            // Undefined!
            return (pc+1);
        } else {
            if ((ins & 0x00100000) == 0) {
                // STR
                return (pc+1);
            } else {
                // LDR
                if ((ins & 0x0000F000) != 0x0000F000) {
                    // Rd not PC
                    return (pc+1);
                } else {
                    Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                    if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                    if (ins & 0x01000000) {
                        // Add/subtract offset before
                        if ((ins & 0x02000000) == 0) {
                            // Immediate offset
                            if (ins & 0x00800000) {
                                // Add offset
                                Rn += (ins & 0x00000FFF);
                            } else {
                                // Subtract offset
                                Rn -= (ins & 0x00000FFF);
                            }
                        } else {
                            // Offset is in a register
                            if (ins & 0x00800000) {
                                // Add offset
                                Rn += RmShifted(ins & 0x00000FFF);
                            } else {
                                // Subtract offset
                                Rn -= RmShifted(ins & 0x00000FFF);
                            }
                        }
                    }
                    return ((unsigned long *)*(unsigned long *)Rn);
                }
            }
        }
        return (pc+1);
    case 0x2:  // Branch, LDM/STM
        if ((ins & 0x02000000) == 0) {
            // LDM/STM
            if ((ins & 0x00100000) == 0) {
                // STM
                return (pc+1);
            } else {
                // LDM
                if ((ins & 0x00008000) == 0) {
                    // PC not in list
                    return (pc+1);
                } else {
                    Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                    if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                    offset = ins & 0x0000FFFF;
                    reg_count = 0;
                    for (i = 0;  i < 15;  i++) {
                        if (offset & (1<<i)) reg_count++;
                    }                    
                    if (ins & 0x00800000) {
                        // Add offset
                        Rn += reg_count*4;
                    } else {
                        // Subtract offset
                        Rn -= 4;
                    }
                    return ((unsigned long *)*(unsigned long *)Rn);
                }
            }
        } else {
            // Branch
            if (ins_will_execute(ins)) {
                offset = (ins & 0x00FFFFFF) << 2;
                if (ins & 0x00800000) offset |= 0xFC000000;  // sign extend
                new_pc = (unsigned long)(pc+2) + offset;
		// If its BLX, make new_pc a thumb address.
		if ((ins & 0xFE000000) == 0xFA000000) {
		    if ((ins & 0x01000000) == 0x01000000)
			new_pc |= 2;
		    new_pc = MAKE_THUMB_ADDR(new_pc);
		}
                return ((unsigned long *)new_pc);
            } else {
                // Falls through
                return (pc+1);
            }
        }
    case 0x3:  // Coprocessor & SWI
        if (((ins & 0x03000000) == 0x03000000) && ins_will_execute(ins)) {
           // SWI
           return (unsigned long *)(CYGNUM_HAL_VECTOR_SOFTWARE_INTERRUPT * 4);
        } else {
           return (pc+1);
        }
    default:
        // Never reached - but fixes compiler warning.
        return 0;
    }
}

// FIXME: target_ins also needs to check for CPSR/THUMB being set and
//        set the thumb bit accordingly.

static unsigned long
target_thumb_ins(unsigned long pc, unsigned short ins)
{
    unsigned long new_pc = MAKE_THUMB_ADDR(pc+2); // default is fall-through 
                                        // to next thumb instruction
    unsigned long offset, arm_ins, sp;
    int i;

    switch ((ins & 0xf000) >> 12) {
    case 0x4:
        // Check for BX or BLX
        if ((ins & 0xff07) == 0x4700)
            new_pc = (unsigned long)get_register((ins & 0x00078) >> 3);
        break;
    case 0xb:
        // push/pop
        // Look for "pop {...,pc}"
        if ((ins & 0xf00) == 0xd00) {
            // find PC
            sp = (unsigned long)get_register(SP);

            for (offset = i = 0; i < 8; i++)
              if (ins & (1 << i))
                  offset += 4;

            new_pc = *(cyg_uint32 *)(sp + offset);

            if (!v5T_semantics())
                new_pc = MAKE_THUMB_ADDR(new_pc);
        }
        break;
    case 0xd:
        // Bcc | SWI
        // Use ARM function to check condition
        arm_ins = ((unsigned long)(ins & 0x0f00)) << 20;
        if ((arm_ins & 0xF0000000) == 0xF0000000) {
            // SWI
            new_pc = CYGNUM_HAL_VECTOR_SOFTWARE_INTERRUPT * 4;
        } else if (ins_will_execute(arm_ins)) {
            offset = (ins & 0x00FF) << 1;
            if (ins & 0x0080) offset |= 0xFFFFFE00;  // sign extend
            new_pc = MAKE_THUMB_ADDR((unsigned long)(pc+4) + offset);
        }
        break;
    case 0xe:
        // check for B
        if ((ins & 0x0800) == 0) {
            offset = (ins & 0x07FF) << 1;
            if (ins & 0x0400) offset |= 0xFFFFF800;  // sign extend
            new_pc = MAKE_THUMB_ADDR((unsigned long)(pc+4) + offset);
        }
        break;
    case 0xf:
        // BL/BLX (4byte instruction!)
        // First instruction (bit 11 == 0) holds top-part of offset
        if ((ins & 0x0800) == 0) {
	    offset = (ins & 0x07FF) << 12;
	    if (ins & 0x0400) offset |= 0xFF800000;  // sign extend
	    // Get second instruction
	    // Second instruction (bit 11 == 1) holds bottom-part of offset
	    ins = *(unsigned short*)(pc+2);
	    // Check for BL/BLX
	    if ((ins & 0xE800) == 0xE800) {
		offset |= (ins & 0x07ff) << 1;
		new_pc = (unsigned long)(pc+4) + offset;
		// If its BLX, force a full word alignment
		// Otherwise, its a thumb address.
		if (!(ins & 0x1000))
		    new_pc &= ~3;
		else
		    new_pc = MAKE_THUMB_ADDR(new_pc);
	    }
	}
        break;
    }

    return new_pc;
}

void __single_step (void)
{
    unsigned long pc = get_register(PC);
    unsigned long cpsr = get_register(PS);

    // Calculate address of next instruction to be executed
    if (cpsr & CPSR_THUMB_ENABLE) {
        // thumb
        ss_saved_pc = target_thumb_ins(pc, *(unsigned short*)pc);
    } else {
        // ARM
        unsigned long curins = *(unsigned long*)pc;
        if (ins_will_execute(curins)) {
            // Decode instruction to decide what the next PC will be
            ss_saved_pc = (unsigned long) target_ins((unsigned long*)pc, 
                                                     curins);
        } else {
            // The current instruction will not execute (the conditions 
            // don't hold)
            ss_saved_pc = pc+4;
        }
    }

    // Set breakpoint according to type
    if (IS_THUMB_ADDR(ss_saved_pc)) {
        // Thumb instruction
        unsigned long t_pc = UNMAKE_THUMB_ADDR(ss_saved_pc);
        ss_saved_thumb_instr = *(unsigned short*)t_pc;
        *(unsigned short*)t_pc = HAL_BREAKINST_THUMB;
    } else {
        // ARM instruction
        ss_saved_instr = *(unsigned long*)ss_saved_pc;
        *(unsigned long*)ss_saved_pc = HAL_BREAKINST_ARM;
    }
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
    if (ss_saved_pc != 0) {
        // Restore instruction according to type
        if (IS_THUMB_ADDR(ss_saved_pc)) {
            // Thumb instruction
            unsigned long t_pc = UNMAKE_THUMB_ADDR(ss_saved_pc);
            *(unsigned short*)t_pc = ss_saved_thumb_instr;
        } else {
            // ARM instruction
            *(unsigned long*)ss_saved_pc = ss_saved_instr;
        }
        ss_saved_pc = 0;
    }
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
    unsigned long cpsr = get_register(PS);

    if (cpsr & CPSR_THUMB_ENABLE)
        pc += 2;
    else
        pc += 4;

    put_register(PC, pc);
}

//-----------------------------------------------------------------------
// Thumb-aware GDB interrupt handler.
// This is a brute-force replacement of the ones in hal_stub.c. Need to
// find a better way of handling it... Maybe... Probably only ARM/thumb
// that is this weird.

typedef struct
{
    cyg_uint32 targetAddr;
    union {
        cyg_uint32 arm_instr;
        cyg_uint16 thumb_instr;
    } savedInstr;
} instrBuffer;

static instrBuffer break_buffer;

volatile int cyg_hal_gdb_running_step = 0;

// This function is passed thumb/arm information about the PC address
// in bit 0. This information is passed on to the break_buffer.
void 
cyg_hal_gdb_place_break (target_register_t pc)
{
    // Clear flag that we Continued instead of Stepping
    cyg_hal_gdb_running_step = 0;

    if (0 == break_buffer.targetAddr) {
        // Setting a breakpoint in Thumb or ARM code?
       if (IS_THUMB_ADDR(pc)) {
            break_buffer.targetAddr = (cyg_uint32)pc;
            pc = UNMAKE_THUMB_ADDR(pc);
            break_buffer.savedInstr.thumb_instr = *(cyg_uint16*)pc;
            *(cyg_uint16*)pc = HAL_BREAKINST_THUMB;
        } else {
            break_buffer.targetAddr = (cyg_uint32)pc;
            break_buffer.savedInstr.arm_instr = *(cyg_uint32*)pc;
            *(cyg_uint32*)pc = HAL_BREAKINST_ARM;
        }
        
        __data_cache(CACHE_FLUSH);
        __instruction_cache(CACHE_FLUSH);
    }
}

int 
cyg_hal_gdb_remove_break (target_register_t pc)
{
    if ( cyg_hal_gdb_running_step )
        return 0; // Do not remove the break: we must hit it!

    if (pc == UNMAKE_THUMB_ADDR(break_buffer.targetAddr)) {
        if (IS_THUMB_ADDR(break_buffer.targetAddr)) {
            *(cyg_uint16*)pc = break_buffer.savedInstr.thumb_instr;
        } else {
            *(cyg_uint32*)pc = break_buffer.savedInstr.arm_instr;
        }
        break_buffer.targetAddr = 0;

        __data_cache(CACHE_FLUSH);
        __instruction_cache(CACHE_FLUSH);
        return 1;
    }
    return 0;
}

void 
cyg_hal_gdb_interrupt (target_register_t pc)
{
    // Clear flag that we Continued instead of Stepping
    cyg_hal_gdb_running_step = 0;
    // and override existing break? So that a ^C takes effect...
    if (0 != break_buffer.targetAddr)
        cyg_hal_gdb_remove_break( break_buffer.targetAddr );

    if (0 == break_buffer.targetAddr) {
        cyg_uint32 cpsr = get_register(PS);

        if (cpsr & CPSR_THUMB_ENABLE) {
            break_buffer.targetAddr = MAKE_THUMB_ADDR((cyg_uint32)pc);
            break_buffer.savedInstr.thumb_instr = *(cyg_uint16*)pc;
            *(cyg_uint16*)pc = HAL_BREAKINST_THUMB;
        } else {
            break_buffer.targetAddr = (cyg_uint32)pc;
            break_buffer.savedInstr.arm_instr = *(cyg_uint32*)pc;
            *(cyg_uint32*)pc = HAL_BREAKINST_ARM;
        }

        __data_cache(CACHE_FLUSH);
        __instruction_cache(CACHE_FLUSH);
    }
}

int
cyg_hal_gdb_break_is_set (void)
{
    if (0 != break_buffer.targetAddr) {
        return 1;
    }
    return 0;
}

#ifdef CYGHWR_HAL_ARM_ICE_THREAD_SUPPORT
#define ICE_THREAD_KEY0 0xDEAD0001
#define ICE_THREAD_KEY1 0xDEAD0002

#define ICE_THREAD_INBUFSIZ  2048
#define ICE_THREAD_OUTBUFSIZ 2048
#define ICE_THREAD_STACKSIZE 4096

static cyg_uint8 ice_thread_inbuf[ICE_THREAD_INBUFSIZ];
static cyg_uint8 ice_thread_outbuf[ICE_THREAD_OUTBUFSIZ];
static cyg_uint8 ice_thread_stack[ICE_THREAD_STACKSIZE];

static void ice_thread_proc(void);

struct {
    cyg_uint32 _key0;  // Must be ICE_KEY0
    cyg_uint8  *in_buffer;
    cyg_int32  in_buffer_size;
    cyg_uint8  *out_buffer;
    cyg_int32  out_buffer_size;
    cyg_uint8  *stack;
    cyg_int32  stack_size;
    void       (*fun)(void);
    cyg_uint32 _key1;  // Must be ICE_KEY1
} hal_arm_ice_thread_handler = {
    ICE_THREAD_KEY0,
    ice_thread_inbuf,
    ICE_THREAD_INBUFSIZ,
    ice_thread_outbuf,
    ICE_THREAD_OUTBUFSIZ,
    ice_thread_stack,
    ICE_THREAD_STACKSIZE,
    ice_thread_proc,
    ICE_THREAD_KEY1,
};

static int
ice_thread_query(void)
{
    switch (ice_thread_inbuf[1]) {
    case 'L': // get thread list
        stub_pkt_getthreadlist(&ice_thread_inbuf[2], ice_thread_outbuf, sizeof(ice_thread_outbuf));
        break;
    case 'P': // thread or process information
        stub_pkt_getthreadinfo(&ice_thread_inbuf[2], ice_thread_outbuf, sizeof(ice_thread_outbuf));
        break;
    case 'C': // current thread
        stub_pkt_currthread(&ice_thread_inbuf[2], ice_thread_outbuf, sizeof(ice_thread_outbuf));
        break;
    default:
        return 0;
    }
    return 1;
}

static int
ice_thread_set(void)
{
    return 0;
}

static void
ice_thread_proc(void)
{
    switch (ice_thread_inbuf[0]) {
    case 'g': // Fetch thread registers
        stub_format_registers(ice_thread_outbuf);
        return;
    case 'P': // Update a single register
    case 'G': // Update all registers
        stub_update_registers(ice_thread_inbuf, ice_thread_outbuf);
        return;
    case 'H': // Thread set/query
        stub_pkt_changethread(&ice_thread_inbuf[1], ice_thread_outbuf, sizeof(ice_thread_outbuf));
        return;
    case 'q': // Thread queries
        if (ice_thread_query()) return;
        break;
    case 'Q': // Thread set operations
        if (ice_thread_set()) return;
        break;
    case 'T': // Thread alive?
        stub_pkt_thread_alive(&ice_thread_inbuf[1], ice_thread_outbuf, sizeof(ice_thread_outbuf));
        return;
    default:
    }
    strcpy(ice_thread_outbuf, "ENN");  // Dunno
}

#endif // CYGHWR_HAL_ARM_ICE_THREAD_SUPPORT

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
