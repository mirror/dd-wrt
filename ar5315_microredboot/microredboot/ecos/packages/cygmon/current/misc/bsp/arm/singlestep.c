//==========================================================================
//
//      singlestep.c
//
//      ARM(R) specific single-step support.
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
// Purpose:      ARM(R) specific single-step support. 
// Description:  ARM is a Registered Trademark of Advanced RISC Machines Limited.
//               Other Brands and Trademarks are the property of their
//               respective owners.
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <stdlib.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "insn.h"

#define DEBUG_SINGLESTEP         0
#define DEBUG_SINGLESTEP_VERBOSE 0

/*
 * Structure to hold opcodes hoisted when breakpoints are
 * set for single-stepping or async interruption.
 */
struct _bp_save {
    unsigned long *addr;
    unsigned long opcode;
};

#define NUM_BREAKS_SAVED 2
static struct _bp_save _breaks[NUM_BREAKS_SAVED];

/*
 *  Insert a breakpoint at 'pc' using first available
 *  _bp_save struct.
 */
static void
insert_ss_break(unsigned long *pc)
{
    struct _bp_save *p = _breaks;
    union arm_insn inst;

    if (p->addr && (++p)->addr)
	return;

    /*
     * We can't set a breakpoint at 0
     */
    if (pc == 0)
    {
#if DEBUG_SINGLESTEP
        bsp_printf("Setting BP at <0x%08lx>: Error\n", pc);
#endif /* DEBUG_SINGLESTEP */
        return;
    }

    /*
     * Make sure we are on a long word boundary.
     */
    if (((unsigned long)pc & 0x3) != 0)
    {
        /*
         * All ARM(R) instructions are on a word boundary.
         * This would be invalid.  Don't set a bkpt here.
         */
#if DEBUG_SINGLESTEP
        bsp_printf("Setting BP at <0x%08lx>: Error\n", pc);
#endif /* DEBUG_SINGLESTEP */
        return;
    }


    /*
     * What is the current instruction
     */
    if (bsp_memory_read(pc, 0, ARM_INST_SIZE * 8, 1, &(inst.word)) == 0)
    {
        /*
         * Unable to read this address, probably an invalid address.
         * Don't set a breakpoint here, as it will likely cause a bus error
         */
#if DEBUG_SINGLESTEP
        bsp_printf("Setting BP at <0x%08lx>: Error\n", pc);
#endif /* DEBUG_SINGLESTEP */
        return;
    }

    if (inst.word != BREAKPOINT_INSN)
    {
        /*
         * Only insert a breakpoint if we haven't done so already
         *
         * We may try to insert 2 breakpoints if we to a branch to
         * the immediately following instruction.
         */
#if DEBUG_SINGLESTEP
        bsp_printf("Setting BP at <0x%08lx>: inst <0x%08lx>\n", pc, inst.word);
#endif /* DEBUG_SINGLESTEP */

        p->addr = pc;
        p->opcode = inst.word;
        inst.word = BREAKPOINT_INSN;
        if (bsp_memory_write(pc, 0, ARM_INST_SIZE * 8, 1, &(inst.word)) == 0)
        {
            /*
             * Unable to write this address, probably an invalid address.
             * Don't set a breakpoint here, as it will likely cause a bus error
             */
#if DEBUG_SINGLESTEP
            bsp_printf("Setting BP at <0x%08lx>: Error\n", pc);
#endif /* DEBUG_SINGLESTEP */
            return;
        }

        /* flush icache and dcache, now */
        bsp_flush_dcache((void *)pc, ARM_INST_SIZE);
        bsp_flush_icache((void *)pc, ARM_INST_SIZE);

#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Done setting BP at <0x%08lx>: inst <0x%08lx>\n", pc, *pc);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
    }
}

/*
 *  Cleanup after a singlestep.
 */
void
bsp_singlestep_cleanup(void *registers)
{
    struct _bp_save *p = _breaks;
    int i;
    
    for (i = 0; i < NUM_BREAKS_SAVED; i++, p++)
    {
        if (p->addr)
        {
            unsigned long *old_addr = p->addr;

#if DEBUG_SINGLESTEP_VERBOSE
            bsp_printf("Remove BP at <0x%08lx>: inst <0x%08lx>\n", old_addr, *old_addr);
#endif /* DEBUG_SINGLESTEP */
            *(p->addr) = p->opcode;
            p->addr = NULL;

            /* flush icache and dcache, now */
            bsp_flush_dcache((void *)old_addr, ARM_INST_SIZE);
            bsp_flush_icache((void *)old_addr, ARM_INST_SIZE);

#if DEBUG_SINGLESTEP_VERBOSE
            bsp_printf("Done removing BP at <0x%08lx>: inst <0x%08lx>\n", old_addr, *old_addr);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        }
    }
}

/*
 * Rotate right a value by count
 */
static unsigned long ror(unsigned long value, unsigned count)
{
    while (count-- > 0)
    {
        if (value & 0x1)
            value = (value >> 1) | 0x80000000;
        else
            value = (value >> 1);
    }

    return(value);
}

/*
 * Rotate right a value by 1 with extend
 */
static unsigned long rrx(union arm_psr sr, unsigned long value)
{
    if (sr.psr.c_bit)
        value = (value >> 1) | 0x80000000;
    else
        value = (value >> 1);

    return(value);
}

/*
 * Logical shift left by count
 */
static unsigned long lsl(unsigned long value, unsigned count)
{
    value <<= count;

    return(value);
}

/*
 * Logical shift right by count
 */
static unsigned long lsr(unsigned long value, unsigned count)
{
    value >>= count;

    return(value); 
}

/*
 * Arithmetic shift right by count
 */
static unsigned long asr(unsigned long value, unsigned count)
{
    unsigned long sign_ext_mask = 0;

    if (value & 0x80000000)
    {
        if (count >= sizeof(value)*8)
            sign_ext_mask = ~0;
        else
            sign_ext_mask = (~0 << (sizeof(value)*8 - count));
    }
    value = (value >> count) | sign_ext_mask;

    return(value); 
}

/*
 * Calculate an immediate shift operand based on input shift operand, 
 * shift value and register address.
 */
static unsigned long immediate_shift_operand(ex_regs_t *regs, unsigned shift_immediate, 
                                             unsigned shift, unsigned Rm)
{
    unsigned char *regs_array = (unsigned char *)regs;
    unsigned char *reg_ptr = &regs_array[bsp_regbyte(Rm)];
    unsigned long reg_value = *((unsigned long *)(reg_ptr));
    unsigned long rc = 0;

    BSP_ASSERT((shift_immediate >= 0) && (shift_immediate <= 0x1f));
    BSP_ASSERT((shift >= 0) && (shift <= 0x3));
    BSP_ASSERT((Rm >= 0) && (Rm <= 0xf));
    BSP_ASSERT(bsp_regsize(Rm) == sizeof(unsigned long));

    /*
     * According to the ARM(R) Manual, if Rm is PC then,
     * the value used is the address of the current instruction
     * plus 8
     */
    if (Rm == REG_PC)
        reg_value += 8;

    switch (shift)
    {
    case SHIFT_LSL:
        rc = lsl(reg_value, shift_immediate);
        break;

    case SHIFT_LSR:
        if (shift_immediate == 0)
        {
            /*
             * Special Case: LSR IMM(0) == 0
             */
            rc = 0;
        } else {
            rc = lsr(reg_value, shift_immediate);
        }
        break;

    case SHIFT_ASR:
        if (shift_immediate == 0)
        {
            /*
             * Special Case: ASR IMM(0)
             */
            if (reg_value & 0x80000000)
            {
                rc = 0xFFFFFFFF;
            } else {
                rc = 0;
            }
        } else {
            rc = asr(reg_value, shift_immediate);
        }
        break;

    case SHIFT_ROR:
        if (shift_immediate == 0)
        {
            /*
             * SHIFT_RRX
             * Special case: ROR(0) implies RRX
             */
            rc = rrx((union arm_psr)(unsigned long)regs->_cpsr, reg_value);
        } else {
            rc = ror(reg_value, shift_immediate);
        }
        break;

    default:
        BSP_ASSERT(0);
        break;
    }

    return (rc);
}

/*
 * Calculate a register shift operand based on input shift operand, 
 * and target registers.
 */
static unsigned long register_shift_operand(ex_regs_t *regs, unsigned Rs,
                                            unsigned shift, unsigned Rm)
{
    unsigned char *regs_array = (unsigned char *)regs;
    unsigned char *Rs_ptr = &regs_array[bsp_regbyte(Rs)];
    unsigned char *Rm_ptr = &regs_array[bsp_regbyte(Rm)];
    unsigned long Rs_val = *((unsigned long *)(Rs_ptr));
    unsigned long Rm_val = *((unsigned long *)(Rm_ptr));
    unsigned long rc = 0;

    /*
     * Use only the least significant byte of Rs
     */
    Rs_val &= 0xFF;

    BSP_ASSERT((Rs >= 0) && (Rs <= 0xf));
    BSP_ASSERT((shift >= 0) && (shift <= 0x3));
    BSP_ASSERT((Rm >= 0) && (Rm <= 0xf));
    BSP_ASSERT(bsp_regsize(Rs) == sizeof(unsigned long));
    BSP_ASSERT(bsp_regsize(Rm) == sizeof(unsigned long));
    BSP_ASSERT((Rs_val >=0) && (Rs_val <= 0xff));

    /*
     * According to the ARM(R) Manual, if Rm is PC then,
     * the value used is the address of the current instruction
     * plus 8
     */
    if (Rm == REG_PC)
        Rm_val += 8;

    switch (shift)
    {
    case SHIFT_LSL: rc = lsl(Rm_val, Rs_val); break;
    case SHIFT_LSR: rc = lsr(Rm_val, Rs_val); break;
    case SHIFT_ASR: rc = asr(Rm_val, Rs_val); break;
    case SHIFT_ROR: rc = ror(Rm_val, Rs_val); break;
    default:        BSP_ASSERT(0);            break;
    }

    return (rc);
}

/*
 * Calculate a branch exchange operand based on input destination register
 */
static unsigned long branch_exchange_operand(ex_regs_t *regs, unsigned Rm)
{
    unsigned char *regs_array = (unsigned char *)regs;
    unsigned char *reg_ptr = &regs_array[bsp_regbyte(Rm)];
    unsigned long reg_value = *((unsigned long *)(reg_ptr));

    BSP_ASSERT((Rm >= 0) && (Rm <= 0xf));
    BSP_ASSERT(bsp_regsize(Rm) == sizeof(unsigned long));

    /*
     * Clear the low-order bit
     */
    return (reg_value & ~0x1);
}

/*
 * Handle a load to the PC
 */
static void handle_pc_load(unsigned size, unsigned long operand)
{
    unsigned long mem_value = 0;

    if (size == LS_SIZE_WORD)
    {
        if (bsp_memory_read((void*)(operand & ~0x3), 0, 32, 1, &mem_value) == 0)
        {
            /*
             * Unable to read the memory address.
             * Don't try any further.
             */
#if DEBUG_SINGLESTEP
            bsp_printf("Setting BP at *(0x%08lx): Error\n", operand & ~0x3);
#endif /* DEBUG_SINGLESTEP */
            return;
        } else {
#if DEBUG_SINGLESTEP
            bsp_printf("Setting BP at *(0x%08lx): data <0x%08lx>\n", operand & ~0x3, mem_value);
#endif /* DEBUG_SINGLESTEP */
        }

        /*
         * Handle rotations if required
         */
        switch (operand & 0x3)
        {
        case 0x0: break;
        case 0x1: mem_value = ror(mem_value,  8); break;
        case 0x2: mem_value = ror(mem_value, 16); break;
        case 0x3: mem_value = ror(mem_value, 24); break;
        }
    } else {
        /*
         * Byte load of the PC
         */
        if (bsp_memory_read((void*)operand, 0, 8, 1, &mem_value) == 0)
        {
            /*
             * Unable to read the memory address.
             * Don't try any further.
             */
#if DEBUG_SINGLESTEP
            bsp_printf("Setting BP at *(0x%08lx): Error\n", operand & ~0x3);
#endif /* DEBUG_SINGLESTEP */
            return;
        } else {
#if DEBUG_SINGLESTEP
            bsp_printf("Setting BP at *(0x%08lx): data <0x%08lx>\n", operand & ~0x3, mem_value);
#endif /* DEBUG_SINGLESTEP */
        }
    }
    
    insert_ss_break((unsigned long *)mem_value);
}

/*
 * Calculate a load/store w/ Immediate offset operand based on input
 * source register, offset value, and opcode (add/sub)
 */
static unsigned long load_store_immediate_operand(ex_regs_t *regs,
                                                  unsigned p_bit, 
                                                  unsigned u_bit,
                                                  unsigned Rn,
                                                  unsigned offset)
{
    unsigned char *regs_array = (unsigned char *)regs;
    unsigned char *reg_ptr = &regs_array[bsp_regbyte(Rn)];
    unsigned long rc = *((unsigned long *)(reg_ptr));

    BSP_ASSERT((Rn >= 0) && (Rn <= 0xf));
    BSP_ASSERT(bsp_regsize(Rn) == sizeof(unsigned long));
    BSP_ASSERT((offset >= 0) && (offset <= 0xfff));
    BSP_ASSERT((p_bit >= 0) && (p_bit <= 1));
    BSP_ASSERT((u_bit >= 0) && (u_bit <= 1));

    /*
     * According to the ARM(R) Manual, if Rn is PC then,
     * the value used is the address of the current instruction
     * plus 8
     */
    if (Rn == REG_PC)
        rc += 8;

    /*
     * Do the update pre-index update
     */
    if (p_bit == LS_INDEX_PRE)
    {
        if (u_bit == LS_OFFSET_SUB)
            rc -= offset;
        else /* opcode == LS_OFFSET_ADD */
            rc += offset;
    }

   return (rc);
}

/*
 * Calculate a load/store w/ Register offset operand based on input
 * source register, offset value, and opcode (add/sub)
 *
 * This calculates the appropriate pre-indexed operand
 */
static unsigned long load_store_register_operand(ex_regs_t *regs,
                                                 unsigned p_bit, 
                                                 unsigned u_bit,
                                                 unsigned Rn,
                                                 unsigned Rm,
                                                 unsigned shift,
                                                 unsigned shift_immed)
{
    unsigned char *regs_array = (unsigned char *)regs;
    unsigned char *Rn_ptr = &regs_array[bsp_regbyte(Rn)];
    unsigned long Rn_val = *((unsigned long *)(Rn_ptr));
    unsigned long rc, index;

    BSP_ASSERT((Rn >= 0) && (Rn <= 0xf));
    BSP_ASSERT((Rm >= 0) && (Rm <= 0xf));
    BSP_ASSERT(bsp_regsize(Rn) == sizeof(unsigned long));
    BSP_ASSERT(bsp_regsize(Rm) == sizeof(unsigned long));
    BSP_ASSERT((p_bit >= 0) && (p_bit <= 1));
    BSP_ASSERT((u_bit >= 0) && (u_bit <= 1));
    BSP_ASSERT((shift >= 0) && (shift <= 0x3));
    BSP_ASSERT((shift_immed >= 0) && (shift_immed <= 0x1F));

    /*
     * According to the ARM(R) Manual, if Rn is PC then
     * the value used is the address of the current
     * instruction plus 8
     */
    if (Rn == REG_PC)
        Rn_val += 8;

    /*
     * According to the ARM(R) Manual, if Rm is PC then
     * the result is unpredictable.  Don't do anything
     * here.  Just return.
     */
    if (Rm == REG_PC)
        return 0;

    index = immediate_shift_operand(regs, shift_immed, shift, Rm);
    
    rc = Rn_val;

    /*
     * Do the update pre-index update
     */
    if (p_bit == LS_INDEX_PRE)
    {
        if (u_bit == LS_OFFSET_SUB)
            rc = Rn_val - index;
        else /* opcode == LS_OFFSET_ADD */
            rc = Rn_val + index;
    }

   return (rc);
}

/*
 * Decode all data processing immediate instructions
 */
static void decode_dpi_inst(ex_regs_t *regs, union arm_insn inst)
{
    if (inst.dpi.Rd == REG_PC)
    {
        unsigned long operand = ror(inst.dpi.immediate, (inst.dpi.rotate << 1));
        unsigned long *dest = 0;
        unsigned carry = ((union arm_psr)(unsigned long)(regs->_cpsr)).psr.c_bit;
        unsigned char *regs_array = (unsigned char *)regs;
        unsigned char *Rn_ptr = &regs_array[bsp_regbyte(inst.dpi.Rn)];
        unsigned long Rn_val = *((unsigned long *)(Rn_ptr));

#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Decoded an data processing immediate instruction.\n");
        bsp_printf("inst.dpi.immediate = 0x%x\n", inst.dpi.immediate);
        bsp_printf("inst.dpi.rotate    = 0x%x\n", inst.dpi.rotate);
        bsp_printf("inst.dpi.Rd        = 0x%x\n", inst.dpi.Rd);
        bsp_printf("inst.dpi.Rn        = 0x%x\n", inst.dpi.Rn);
        bsp_printf("inst.dpi.S_bit     = 0x%x\n", inst.dpi.S_bit);
        bsp_printf("inst.dpi.opcode    = 0x%x\n", inst.dpi.opcode);
        bsp_printf("inst.dpi.cond      = 0x%x\n", inst.dpi.cond);
        bsp_printf("operand            = 0x%x\n", operand);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

        /*
         * According to the ARM(R) Manual, if Rn is PC then
         * the value used is the address of the current
         * instruction plus 8
         */
        if (inst.dpi.Rn == REG_PC)
            Rn_val += 8;

        switch (inst.dpi.opcode) {
        case DP_OPCODE_ADC: dest = (unsigned long *)(Rn_val + operand + carry);    break;
        case DP_OPCODE_ADD: dest = (unsigned long *)(Rn_val + operand);            break;
        case DP_OPCODE_AND: dest = (unsigned long *)(Rn_val & operand);            break;
        case DP_OPCODE_BIC: dest = (unsigned long *)(Rn_val & ~operand);           break;
        case DP_OPCODE_EOR: dest = (unsigned long *)(Rn_val ^ operand);            break;
        case DP_OPCODE_MOV: dest = (unsigned long *)operand;                       break;
        case DP_OPCODE_MVN: dest = (unsigned long *)(~operand);                    break;
        case DP_OPCODE_ORR: dest = (unsigned long *)(Rn_val | operand);            break;
        case DP_OPCODE_RSB: dest = (unsigned long *)(operand - Rn_val);            break;
        case DP_OPCODE_RSC: dest = (unsigned long *)(operand - Rn_val - !carry);   break;
        case DP_OPCODE_SBC: dest = (unsigned long *)(Rn_val - operand - !carry);   break;
        case DP_OPCODE_SUB: dest = (unsigned long *)(Rn_val - operand);            break;
        default:            dest = (unsigned long *)0;                             break;
        }
        dest = (unsigned long *)((unsigned long)dest & ~0x3);
        insert_ss_break(dest);
    }
}

/*
 * Decode all data processing immediate w/ shift instructions
 */
static void decode_dpis_inst(ex_regs_t *regs, union arm_insn inst)
{
    if (inst.dpis.Rd == REG_PC)
    {
        unsigned long operand = immediate_shift_operand(regs, inst.dpis.shift_immed, 
                                                        inst.dpis.shift, inst.dpis.Rm);
        unsigned long *dest = 0;
        unsigned carry = ((union arm_psr)(unsigned long)(regs->_cpsr)).psr.c_bit;
        unsigned char *regs_array = (unsigned char *)regs;
        unsigned char *Rn_ptr = &regs_array[bsp_regbyte(inst.dpis.Rn)];
        unsigned long Rn_val = *((unsigned long *)(Rn_ptr));

#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Decoded an data processing immediate shift instruction.\n");
        bsp_printf("inst.dpis.Rm          = 0x%x\n", inst.dpis.Rm);
        bsp_printf("inst.dpis.shift       = 0x%x\n", inst.dpis.shift);
        bsp_printf("inst.dpis.shift_immed = 0x%x\n", inst.dpis.shift_immed);
        bsp_printf("inst.dpis.Rd          = 0x%x\n", inst.dpis.Rd);
        bsp_printf("inst.dpis.Rn          = 0x%x\n", inst.dpis.Rn);
        bsp_printf("inst.dpis.S_bit       = 0x%x\n", inst.dpis.S_bit);
        bsp_printf("inst.dpis.opcode      = 0x%x\n", inst.dpis.opcode);
        bsp_printf("inst.dpis.cond        = 0x%x\n", inst.dpis.cond);
        bsp_printf("operand               = 0x%x\n", operand);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

        /*
         * According to the ARM(R) Manual, if Rn is PC then
         * the value used is the address of the current
         * instruction plus 8
         */
        if (inst.dpis.Rn == REG_PC)
            Rn_val += 8;

        switch (inst.dpis.opcode) {
        case DP_OPCODE_ADC: dest = (unsigned long *)(Rn_val + operand + carry);    break;
        case DP_OPCODE_ADD: dest = (unsigned long *)(Rn_val + operand);            break;
        case DP_OPCODE_AND: dest = (unsigned long *)(Rn_val & operand);            break;
        case DP_OPCODE_BIC: dest = (unsigned long *)(Rn_val & ~operand);           break;
        case DP_OPCODE_EOR: dest = (unsigned long *)(Rn_val ^ operand);            break;
        case DP_OPCODE_MOV: dest = (unsigned long *)operand;                       break;
        case DP_OPCODE_MVN: dest = (unsigned long *)(~operand);                    break;
        case DP_OPCODE_ORR: dest = (unsigned long *)(Rn_val | operand);            break;
        case DP_OPCODE_RSB: dest = (unsigned long *)(operand - Rn_val);            break;
        case DP_OPCODE_RSC: dest = (unsigned long *)(operand - Rn_val - !carry);   break;
        case DP_OPCODE_SBC: dest = (unsigned long *)(Rn_val - operand - !carry);   break;
        case DP_OPCODE_SUB: dest = (unsigned long *)(Rn_val - operand);            break;
        default:            dest = (unsigned long *)0;                             break;
        }
        dest = (unsigned long *)((unsigned long)dest & ~0x3);
        insert_ss_break(dest);
    }
}

/*
 * Decode all data processing register w/ shift instructions
 */
static void decode_dprs_inst(ex_regs_t *regs, union arm_insn inst)
{
    if (inst.dprs.Rd == REG_PC)
    {
        unsigned long operand = register_shift_operand(regs, inst.dprs.Rs,
                                                       inst.dprs.shift, inst.dprs.Rm);
        unsigned long *dest = 0;
        unsigned carry = ((union arm_psr)(unsigned long)(regs->_cpsr)).psr.c_bit;
        unsigned char *regs_array = (unsigned char *)regs;
        unsigned char *Rn_ptr = &regs_array[bsp_regbyte(inst.dprs.Rn)];
        unsigned long Rn_val = *((unsigned long *)(Rn_ptr));

#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Decoded an data processing register shift instruction.\n");
        bsp_printf("inst.dprs.Rm          = 0x%x\n", inst.dprs.Rm);
        bsp_printf("inst.dprs.rsv3        = 0x%x\n", inst.dprs.rsv3);
        bsp_printf("inst.dprs.shift       = 0x%x\n", inst.dprs.shift);
        bsp_printf("inst.dprs.rsv2        = 0x%x\n", inst.dprs.rsv2);
        bsp_printf("inst.dprs.Rs          = 0x%x\n", inst.dprs.Rs);
        bsp_printf("inst.dprs.Rd          = 0x%x\n", inst.dprs.Rd);
        bsp_printf("inst.dprs.Rn          = 0x%x\n", inst.dprs.Rn);
        bsp_printf("inst.dprs.S_bit       = 0x%x\n", inst.dprs.S_bit);
        bsp_printf("inst.dprs.opcode      = 0x%x\n", inst.dprs.opcode);
        bsp_printf("inst.dprs.rsv1        = 0x%x\n", inst.dprs.rsv1);
        bsp_printf("inst.dprs.cond        = 0x%x\n", inst.dprs.cond);
        bsp_printf("operand               = 0x%x\n", operand);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

        /*
         * According to the ARM(R) Manual, if Rn is PC then
         * the value used is the address of the current
         * instruction plus 8
         */
        if (inst.dprs.Rn == REG_PC)
            Rn_val += 8;

        switch (inst.dprs.opcode) {
        case DP_OPCODE_ADC: dest = (unsigned long *)(Rn_val + operand + carry);    break;
        case DP_OPCODE_ADD: dest = (unsigned long *)(Rn_val + operand);            break;
        case DP_OPCODE_AND: dest = (unsigned long *)(Rn_val & operand);            break;
        case DP_OPCODE_BIC: dest = (unsigned long *)(Rn_val & ~operand);           break;
        case DP_OPCODE_EOR: dest = (unsigned long *)(Rn_val ^ operand);            break;
        case DP_OPCODE_MOV: dest = (unsigned long *)operand;                       break;
        case DP_OPCODE_MVN: dest = (unsigned long *)(~operand);                    break;
        case DP_OPCODE_ORR: dest = (unsigned long *)(Rn_val | operand);            break;
        case DP_OPCODE_RSB: dest = (unsigned long *)(operand - Rn_val);            break;
        case DP_OPCODE_RSC: dest = (unsigned long *)(operand - Rn_val - !carry);   break;
        case DP_OPCODE_SBC: dest = (unsigned long *)(Rn_val - operand - !carry);   break;
        case DP_OPCODE_SUB: dest = (unsigned long *)(Rn_val - operand);            break;
        default:            dest = (unsigned long *)0;                             break;
        }

        dest = (unsigned long *)((unsigned long)dest & ~0x3);
        insert_ss_break(dest);
    }
}

/*
 * Decode all multiply instructions
 */
static void decode_m_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * According to the ARM(R) Manual, if Rd is PC then
     * the result is unpredictable.  Don't do anything
     * here.  Just return.
     */
}

/*
 * Decode all multiply long instructions
 */
static void decode_ml_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * According to the ARM(R) Manual, if Rd is PC then
     * the result is unpredictable.  Don't do anything
     * here.  Just return.
     */
}


/*
 * Decode all move from status register instructions
 */
static void decode_mrs_inst(ex_regs_t *regs, union arm_insn inst)
{
#if 0
    if (inst.mrs.Rd == REG_PC)
    {
        unsigned long *dest = 0;

#if DEBUG_SINGLESTEP_VERBOSE
    bsp_printf("Decoded an move from status register instruction.\n");
    bsp_printf("inst.mrs.SBZ        = 0x%x\n", inst.mrs.SBZ);
    bsp_printf("inst.mrs.Rd         = 0x%x\n", inst.mrs.Rd);
    bsp_printf("inst.mrs.SBO        = 0x%x\n", inst.mrs.SBO);
    bsp_printf("inst.mrs.rsv2       = 0x%x\n", inst.mrs.rsv2);
    bsp_printf("inst.mrs.R_bit      = 0x%x\n", inst.mrs.R_bit);
    bsp_printf("inst.mrs.rsv1       = 0x%x\n", inst.mrs.rsv1);
    bsp_printf("inst.mrs.cond       = 0x%x\n", inst.mrs.cond);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

        if (inst.mrs.R_bit == 1)
            dest = (unsigned long *)regs->_spsr;
        else
            dest = (unsigned long *)regs->_cpsr;

        dest = (unsigned long *)((unsigned long)dest & ~0x3);
        insert_ss_break(dest);
    }
#endif
}


/*
 * Decode all move immediate to status register instructions
 */
static void decode_misr_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Can't update the PC w/ this instruction.
     * Don't set any more breakpoints
     */
}


/*
 * Decode all move register to status registers instructions
 */
static void decode_mrsr_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Can't update the PC w/ this instruction.
     * Don't set any more breakpoints
     */
}


/*
 * Decode all branch/exchange instructions
 */
static void decode_bx_inst(ex_regs_t *regs, union arm_insn inst)
{
    unsigned long operand = branch_exchange_operand(regs, inst.bx.Rm);

#if DEBUG_SINGLESTEP_VERBOSE
    bsp_printf("Decoded an branch/exchange shift instruction.\n");
    bsp_printf("inst.bx.Rm          = 0x%x\n", inst.bx.Rm);
    bsp_printf("inst.bx.rsv2        = 0x%x\n", inst.bx.rsv2);
    bsp_printf("inst.bx.SBO3        = 0x%x\n", inst.bx.SBO3);
    bsp_printf("inst.bx.SBO2        = 0x%x\n", inst.bx.SBO2);
    bsp_printf("inst.bx.SBO1        = 0x%x\n", inst.bx.SBO1);
    bsp_printf("inst.bx.rsv1        = 0x%x\n", inst.bx.rsv1);
    bsp_printf("inst.bx.cond        = 0x%x\n", inst.bx.cond);
    bsp_printf("operand             = 0x%x\n", operand);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

    insert_ss_break((unsigned long *)operand);
}


/*
 * Decode all load/store immediate offset instructions
 */
static void decode_lsio_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Only support direct loads of the PC
     *
     * According to the ARM(R) manual, automatic updates of the PC
     * are UNPREDICTABLE (ie implementation defined).
     */
    if ((inst.lsio.Rd == REG_PC) && (inst.lsio.L_bit == LS_LOAD))
    {
        unsigned long operand = load_store_immediate_operand(regs, inst.lsio.P_bit,
                                                             inst.lsio.U_bit, inst.lsio.Rn,
                                                             inst.lsio.immediate);

#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Decoded an load/store w/ immediate offset instruction.\n");
        bsp_printf("inst.lsio.immediate = 0x%x\n", inst.lsio.immediate);
        bsp_printf("inst.lsio.Rd        = 0x%x\n", inst.lsio.Rd);
        bsp_printf("inst.lsio.Rn        = 0x%x\n", inst.lsio.Rn);
        bsp_printf("inst.lsio.L_bit     = 0x%x\n", inst.lsio.L_bit);
        bsp_printf("inst.lsio.W_bit     = 0x%x\n", inst.lsio.W_bit);
        bsp_printf("inst.lsio.B_bit     = 0x%x\n", inst.lsio.B_bit);
        bsp_printf("inst.lsio.U_bit     = 0x%x\n", inst.lsio.U_bit);
        bsp_printf("inst.lsio.P_bit     = 0x%x\n", inst.lsio.P_bit);
        bsp_printf("inst.lsio.rsv1      = 0x%x\n", inst.lsio.rsv1);
        bsp_printf("inst.lsio.cond      = 0x%x\n", inst.lsio.cond);
        bsp_printf("operand             = 0x%x\n", operand);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

        handle_pc_load(inst.lsio.B_bit, operand);
    }
}


/*
 * Decode all load/store register offset instructions
 */
static void decode_lsro_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Only support direct loads of the PC
     *
     * According to the ARM(R) manual, automatic updates of the PC
     * are UNPREDICTABLE (ie implementation defined).
     */
    if ((inst.lsro.Rd == REG_PC) && (inst.lsro.L_bit == LS_LOAD))
    {
        unsigned long operand = load_store_register_operand(regs,
                                                            inst.lsro.P_bit, 
                                                            inst.lsro.U_bit,
                                                            inst.lsro.Rn,
                                                            inst.lsro.Rm,
                                                            inst.lsro.shift,
                                                            inst.lsro.shift_immed);
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Decoded an load/store w/ register offset instruction.\n");
        bsp_printf("inst.lsro.Rm          = 0x%x\n", inst.lsro.Rm);
        bsp_printf("inst.lsro.rsv2        = 0x%x\n", inst.lsro.rsv2);
        bsp_printf("inst.lsro.shift       = 0x%x\n", inst.lsro.shift);
        bsp_printf("inst.lsro.shift_immed = 0x%x\n", inst.lsro.shift_immed);
        bsp_printf("inst.lsro.Rd          = 0x%x\n", inst.lsro.Rd);
        bsp_printf("inst.lsro.Rn          = 0x%x\n", inst.lsro.Rn);
        bsp_printf("inst.lsro.L_bit       = 0x%x\n", inst.lsro.L_bit);
        bsp_printf("inst.lsro.W_bit       = 0x%x\n", inst.lsro.W_bit);
        bsp_printf("inst.lsro.B_bit       = 0x%x\n", inst.lsro.B_bit);
        bsp_printf("inst.lsro.U_bit       = 0x%x\n", inst.lsro.U_bit);
        bsp_printf("inst.lsro.P_bit       = 0x%x\n", inst.lsro.P_bit);
        bsp_printf("inst.lsro.rsv1        = 0x%x\n", inst.lsro.rsv1);
        bsp_printf("inst.lsro.cond        = 0x%x\n", inst.lsro.cond);
        bsp_printf("operand               = 0x%x\n", operand);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

        handle_pc_load(inst.lsro.B_bit, operand);
    }
}


/*
 * Decode all load/store halfword/signed byte immediate offset instructions
 */
static void decode_lshwi_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * According to the ARM(R) Manual, if Rd is PC then
     * the result is unpredictable.  Don't do anything
     * here.  Just return.
     */
}


/*
 * Decode all load/store halfword/signed byte register offset instructions
 */
static void decode_lshwr_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * According to the ARM(R) Manual, if Rd is PC then
     * the result is unpredictable.  Don't do anything
     * here.  Just return.
     */
}


/*
 * Decode all swap/swap byte instructions
 */
static void decode_swap_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * According to the ARM(R) Manual, if Rd is PC then
     * the result is unpredictable.  Don't do anything
     * here.  Just return.
     */
}


/*
 * Decode all load/store multiple instructions
 */
static void decode_lsm_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Only support direct load multiples where the PC is in the
     * register list.
     *
     * According to the ARM(R) manual, automatic updates of the PC
     * are UNPREDICTABLE (ie implementation defined).
     */
    if ((inst.lsm.L_bit == LS_LOAD) && (inst.lsm.Reg_List & (0x1 << REG_PC)))
    {
        unsigned char *regs_array = (unsigned char *)regs;
        unsigned char *Rn_ptr = &regs_array[bsp_regbyte(inst.lsm.Rn)];
        unsigned long Rn_val = *((unsigned long *)(Rn_ptr));
        unsigned long offset_to_pc = 0;
        int i;
        unsigned long **dest = 0;

#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Decoded an load multiple instruction.\n");
        bsp_printf("inst.lsm.Reg_List   = 0x%x\n", inst.lsm.Reg_List);

        bsp_printf("inst.lsm.Rn         = 0x%x\n", inst.lsm.Rn);
        bsp_printf("inst.lsm.L_bit      = 0x%x\n", inst.lsm.L_bit);
        bsp_printf("inst.lsm.W_bit      = 0x%x\n", inst.lsm.W_bit);
        bsp_printf("inst.lsm.S_bit      = 0x%x\n", inst.lsm.S_bit);
        bsp_printf("inst.lsm.U_bit      = 0x%x\n", inst.lsm.U_bit);
        bsp_printf("inst.lsm.P_bit      = 0x%x\n", inst.lsm.P_bit);
        bsp_printf("inst.lsm.rsv1       = 0x%x\n", inst.lsm.rsv1);
        bsp_printf("inst.lsm.cond       = 0x%x\n", inst.lsm.cond);
#endif /* DEBUG_SINGLESTEP_VERBOSE */

        if (inst.lsm.U_bit == 0)
        {
            /*
             * We are using a ascending stack.
             * That means the PC is actually the register
             * nearest to Rn currently.
             */
            if (inst.lsm.P_bit == 1)
                /*
                 * Using a pre-decrement.
                 */
                offset_to_pc = -bsp_regsize(REG_PC);
            else
                offset_to_pc = 0;
        } else {
            /*
             * We are using an descending stack.
             * That means the PC is actually the register
             * farthest from Rn currently.
             *
             * Find the number of registers stored before the PC
             */
            for (i = 0; i < REG_PC; i++)
            {
                if ((inst.lsm.Reg_List & (0x1 << i)) != 0)
                {
                    /*
                     * Bit #i is set.  Increment our count.
                     */
                    offset_to_pc += bsp_regsize(i);
                }
            }

            /*
             * Adjust the offset if we do a decrement/increment __BEFORE__
             * the write.
             */
            if (inst.lsm.P_bit == 1)
                offset_to_pc += bsp_regsize(REG_PC);
        }

        /*
         * Now let's calculate the real address of the stored PC
         * making sure to mask out the two LO bits.
         */
        dest = (unsigned long **)((Rn_val + offset_to_pc) & ~0x3);

#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Rn_val = 0x%08lx\n", Rn_val);
        bsp_printf("offset_to_pc = 0x%08lx\n", offset_to_pc);
        bsp_printf("dest = 0x%08lx\n", dest);
        bsp_printf("*dest = 0x%08lx\n", *dest);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        
        insert_ss_break(*dest);
    }
}


/*
 * Decode all coprocessor data processing instructions
 */
static void decode_cpdp_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Can't possibly predict what this instruction will do.
     * Don't do anything here.  Just return.
     */
}


/*
 * Decode all coprocessor register transfer instructions
 */
static void decode_cprt_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Can't possibly predict what this instruction will do.
     * Don't do anything here.  Just return.
     */
}


/*
 * Decode all coprocessor load/store instructions
 */
static void decode_cpls_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Can't possibly predict what this instruction will do.
     * Don't do anything here.  Just return.
     */
}


/*
 * Decode all branch/branch w/ link instructions
 */
static void decode_bbl_inst(ex_regs_t *regs, union arm_insn inst)
{
    unsigned long disp = inst.bbl.offset;

    /*
     * Sign extend the 24 bit value
     */
    if (disp & 0x00800000)
        disp |= 0xff000000;
    
    /*
     * Convert to long words
     */
    disp <<= 2;

    /*
     * Note: when the processor actually executes this instruction, the pc
     *       will point to the address of the current instruction + 8 because
     *       of the fetch/decode/execute cycle
     */
    disp += 8;
    
    insert_ss_break((unsigned long *)(regs->_pc + disp));
}


/*
 * Decode all swi instructions
 */
static void decode_swi_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Can't possibly predict where we should set the breakpoint for this.
     * Don't do anything here.  Just return.
     */
}


/*
 * Decode all undefined instructions
 */
static void decode_undef_inst(ex_regs_t *regs, union arm_insn inst)
{
    /*
     * Can't possibly predict what this instruction will do.
     * Don't do anything here.  Just return.
     */
}


/*
 * Set breakpoint instructions for single stepping.
 */
void
bsp_singlestep_setup(void *registers)
{
    ex_regs_t *regs = (ex_regs_t *)registers;
    union arm_insn inst;

    if (bsp_memory_read((void*)(regs->_pc), 0, ARM_INST_SIZE * 8, 1, &(inst.word)) == 0)
    {
        /*
         * Unable to read the instruction at the current address.
         * Let's not do anything with this.  We can't set breakpoints
         * so let's get out now.
         */
        return;
    }

    /*
     * Handle the simple case -- linear code
     */
    insert_ss_break((unsigned long *)(regs->_pc + ARM_INST_SIZE));

    /*
     * Now, we need to decode the instructions and figure out what
     * they would do.
     */
    if ((inst.mrs.rsv1 == MRS_RSV1_VALUE) &&
        (inst.mrs.rsv2 == MRS_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("MRS type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_mrs_inst(regs, inst);
    } else if ((inst.misr.rsv1 == MISR_RSV1_VALUE) &&
               (inst.misr.rsv2 == MISR_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("MISR type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_misr_inst(regs, inst);
    } else if ((inst.mrsr.rsv1 == MRSR_RSV1_VALUE) &&
               (inst.mrsr.rsv2 == MRSR_RSV2_VALUE) &&
               (inst.mrsr.rsv3 == MRSR_RSV3_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("MRSR type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_mrsr_inst(regs, inst);
    } else if (inst.dpi.rsv1 == DPI_RSV1_VALUE) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("DPI type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_dpi_inst(regs, inst);
    } else if ((inst.bx.rsv1 == BX_RSV1_VALUE) &&
               (inst.bx.rsv2 == BX_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("BX type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_bx_inst(regs, inst);
    } else if ((inst.dpis.rsv1 == DPIS_RSV1_VALUE) &&
               (inst.dpis.rsv2 == DPIS_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("DPIS type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_dpis_inst(regs, inst);
    } else if ((inst.dprs.rsv1 == DPRS_RSV1_VALUE) &&
               (inst.dprs.rsv2 == DPRS_RSV2_VALUE) &&
               (inst.dprs.rsv3 == DPRS_RSV3_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("DPRS type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_dprs_inst(regs, inst);
    } else if ((inst.m.rsv1 == M_RSV1_VALUE) &&
               (inst.m.rsv2 == M_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("M type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_m_inst(regs, inst);
    } else if ((inst.ml.rsv1 == ML_RSV1_VALUE) &&
               (inst.ml.rsv2 == ML_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("ML type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_ml_inst(regs, inst);
    } else if (inst.lsio.rsv1 == LSIO_RSV1_VALUE) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("LSIO type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_lsio_inst(regs, inst);
    } else if ((inst.lsro.rsv1 == LSRO_RSV1_VALUE) &&
               (inst.lsro.rsv2 == LSRO_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("LSRO type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_lsro_inst(regs, inst);
    } else if ((inst.lshwi.rsv1 == LSHWI_RSV1_VALUE) &&
               (inst.lshwi.rsv2 == LSHWI_RSV2_VALUE) &&
               (inst.lshwi.rsv3 == LSHWI_RSV3_VALUE) &&
               (inst.lshwi.rsv4 == LSHWI_RSV4_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("LSHWI type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_lshwi_inst(regs, inst);
    } else if ((inst.lshwr.rsv1 == LSHWR_RSV1_VALUE) &&
               (inst.lshwr.rsv2 == LSHWR_RSV2_VALUE) &&
               (inst.lshwr.rsv3 == LSHWR_RSV3_VALUE) &&
               (inst.lshwr.rsv4 == LSHWR_RSV4_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("LSHWR type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_lshwr_inst(regs, inst);
    } else if ((inst.swap.rsv1 == SWAP_RSV1_VALUE) &&
               (inst.swap.rsv2 == SWAP_RSV2_VALUE) &&
               (inst.swap.rsv3 == SWAP_RSV3_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("SWAP type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_swap_inst(regs, inst);
    } else if (inst.lsm.rsv1 == LSM_RSV1_VALUE) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("LSM type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_lsm_inst(regs, inst);
    } else if ((inst.cpdp.rsv1 == CPDP_RSV1_VALUE) &&
               (inst.cpdp.rsv2 == CPDP_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("CPDP type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_cpdp_inst(regs, inst);
    } else if ((inst.cprt.rsv1 == CPRT_RSV1_VALUE) &&
               (inst.cprt.rsv2 == CPRT_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("CPRT type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_cprt_inst(regs, inst);
    } else if (inst.cpls.rsv1 == CPLS_RSV1_VALUE) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("CPLS type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_cpls_inst(regs, inst);
    } else if (inst.bbl.rsv1 == BBL_RSV1_VALUE) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("BBL type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_bbl_inst(regs, inst);
    } else if (inst.swi.rsv1 == SWI_RSV1_VALUE) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("SWI type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_swi_inst(regs, inst);
    } else if ((inst.undef.rsv1 == UNDEF_RSV1_VALUE) &&
               (inst.undef.rsv2 == UNDEF_RSV2_VALUE)) {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("UNDEF type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
        decode_undef_inst(regs, inst);
    } else {
#if DEBUG_SINGLESTEP_VERBOSE
        bsp_printf("Unknown instruction type: 0x%08lx\n", inst.word);
#endif /* DEBUG_SINGLESTEP_VERBOSE */
    }
}

void
bsp_skip_instruction(void *registers)
{
    ex_regs_t *regs = (ex_regs_t *)registers;
    regs->_pc += ARM_INST_SIZE;
}
