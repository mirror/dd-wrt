//========================================================================
//
//      h8300h_stub.c
//
//      Helper functions for H8/300H stub
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
// Author(s):     Yoshinori Sato
// Contributors:  Yoshinori Sato
// Date:          2002-05-03
// Purpose:       
// Description:   Helper functions for H8/300H stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

/*--------------------------------------------------------------------*/
/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    switch (trap_number) {
    case CYGNUM_HAL_VECTOR_TRAP3:
        return SIGTRAP;
    default:
        return SIGINT;
    }
}

/*--------------------------------------------------------------------*/
/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
    extern int CYG_LABEL_NAME(_intvector);
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return CYG_LABEL_NAME(_intvector);
}

/*--------------------------------------------------------------------*/
/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}


/*----------------------------------------------------------------------
 * Single-step support. 
 */

typedef struct {
    unsigned short *addr;
    unsigned short inst;
} breakinfo;

static breakinfo InstBuffer = {(unsigned short *)-1,0};

/* Clear any single-step breakpoint(s) that may have been set.  */

void __clear_single_step (void)
{
    if ((long)InstBuffer.addr != -1L) {
	*InstBuffer.addr = InstBuffer.inst;
	InstBuffer.addr = (unsigned short *)-1;
    }
}

/* calculate next pc */

enum jump_type{none,aabs,aind,ret,reg,relb,relw};

/* opcode decode table define
   ptn: opcode pattern
   msk: opcode bitmask
   len: instruction length (<0 next table index)
   jmp: jump operation mode */

struct optable {
    unsigned char pattern;
    unsigned char mask;
    signed   char length;
    char          type;
} __attribute__((aligned(1),packed));

#define OPTABLE(ptn,msk,len,jmp) {ptn,msk,len,jmp}

const static struct optable optable_0[] = {
    OPTABLE(0x00,0xff, 1,none), /* 0x00 */
    OPTABLE(0x01,0xff,-1,none), /* 0x01 */
    OPTABLE(0x02,0xfe, 1,none), /* 0x02-0x03 */
    OPTABLE(0x04,0xee, 1,none), /* 0x04-0x05/0x14-0x15 */
    OPTABLE(0x06,0xfe, 1,none), /* 0x06-0x07 */
    OPTABLE(0x08,0xea, 1,none), /* 0x08-0x09/0x0c-0x0d/0x18-0x19/0x1c-0x1d */
    OPTABLE(0x0a,0xee, 1,none), /* 0x0a-0x0b/0x1a-0x1b */
    OPTABLE(0x0e,0xee, 1,none), /* 0x0e-0x0f/0x1e-0x1f */
    OPTABLE(0x10,0xfc, 1,none), /* 0x10-0x13 */
    OPTABLE(0x16,0xfe, 1,none), /* 0x16-0x17 */
    OPTABLE(0x20,0xe0, 1,none), /* 0x20-0x3f */
    OPTABLE(0x40,0xf0, 1,relb), /* 0x40-0x4f */
    OPTABLE(0x50,0xfc, 1,none), /* 0x50-0x53 */
    OPTABLE(0x54,0xfd, 1,ret ), /* 0x54/0x56 */
    OPTABLE(0x55,0xff, 1,relb), /* 0x55 */
    OPTABLE(0x57,0xff, 1,none), /* 0x57 */
    OPTABLE(0x58,0xfb, 2,relw), /* 0x58/0x5c */
    OPTABLE(0x59,0xfb, 1,reg ), /* 0x59/0x5b */
    OPTABLE(0x5a,0xfb, 2,aabs), /* 0x5a/0x5e */
    OPTABLE(0x5b,0xfb, 2,aind), /* 0x5b/0x5f */
    OPTABLE(0x60,0xe8, 1,none), /* 0x60-0x67/0x70-0x77 */
    OPTABLE(0x68,0xfa, 1,none), /* 0x68-0x69/0x6c-0x6d */
    OPTABLE(0x6a,0xfe,-2,none), /* 0x6a-0x6b */
    OPTABLE(0x6e,0xfe, 2,none), /* 0x6e-0x6f */
    OPTABLE(0x78,0xff, 4,none), /* 0x78 */
    OPTABLE(0x79,0xff, 2,none), /* 0x79 */
    OPTABLE(0x7a,0xff, 3,none), /* 0x7a */
    OPTABLE(0x7b,0xff, 2,none), /* 0x7b */
    OPTABLE(0x7c,0xfc, 2,none), /* 0x7c-0x7f */
    OPTABLE(0x80,0x80, 1,none), /* 0x80-0xff */
};

const static struct optable optable_1[] = {
    OPTABLE(0x00,0xff,-3,none), /* 0x0100 */
    OPTABLE(0x40,0xf0,-3,none), /* 0x0140-0x14f */
    OPTABLE(0x80,0xf0, 1,none), /* 0x0180-0x018f */
    OPTABLE(0xc0,0xc0, 2,none), /* 0x01c0-0x01ff */
};

const static struct optable optable_2[] = {
    OPTABLE(0x00,0x20, 2,none), /* 0x6a0?/0x6a8?/0x6b0?/0x6b8? */
    OPTABLE(0x20,0x20, 3,none), /* 0x6a2?/0x6aa?/0x6b2?/0x6ba? */
};

const static struct optable optable_3[] = {
    OPTABLE(0x69,0xfb, 2,none), /* 0x010069/0x01006d/014069/0x01406d */
    OPTABLE(0x6b,0xff,-4,none), /* 0x01006b/0x01406b */
    OPTABLE(0x6f,0xff, 3,none), /* 0x01006f/0x01406f */
    OPTABLE(0x78,0xff, 5,none), /* 0x010078/0x014078 */
};

const static struct optable optable_4[] = {
    OPTABLE(0x00,0x78, 3,none), /* 0x0100690?/0x01006d0?/0140690/0x01406d0?/0x0100698?/0x01006d8?/0140698?/0x01406d8? */
    OPTABLE(0x20,0x78, 4,none), /* 0x0100692?/0x01006d2?/0140692/0x01406d2?/0x010069a?/0x01006da?/014069a?/0x01406da? */
};

const static struct {
    const struct optable *op;
    int length;
} optables[] = {
    {optable_0,sizeof(optable_0)/sizeof(struct optable)},
    {optable_1,sizeof(optable_1)/sizeof(struct optable)},
    {optable_2,sizeof(optable_2)/sizeof(struct optable)},
    {optable_3,sizeof(optable_3)/sizeof(struct optable)},
    {optable_4,sizeof(optable_4)/sizeof(struct optable)},
};

const static unsigned char condmask[] = {
    0x00,0x40,0x01,0x04,0x02,0x08,0x10,0x20
};

static int isbranch(int reson)
{
    unsigned char cond = get_register(CCR);
    /* encode complex conditions */
    /* B4: N^V
       B5: Z|(N^V)
       B6: C|Z */
    __asm__("bld #3,%w0\n\t"
	    "bxor #1,%w0\n\t"
	    "bst #4,%w0\n\t"
	    "bor #2,%w0\n\t"
	    "bst #5,%w0\n\t"
	    "bld #2,%w0\n\t"
	    "bor #0,%w0\n\t"
	    "bst #6,%w0\n\t"
	    :"=&r"(cond):"g"(cond):"cc");
    cond &= condmask[reson >> 1];
    if (!(reson & 1))
	return cond == 0;
    else
	return cond != 0;
}

static unsigned short *getnextpc(unsigned short *pc)
{
    const struct optable *op;
    unsigned char *fetch_p;
    unsigned char inst;
    unsigned long addr;
    unsigned long *sp;
    int op_len;
    op = optables[0].op;
    op_len = optables[0].length;
    fetch_p = (unsigned char *)pc;
    inst = *fetch_p++;
    do {
	if ((inst & op->mask) == op->pattern) {
	    if (op->length < 0) {
		op = optables[-op->length].op;
		op_len = optables[-op->length].length + 1;
		inst = *fetch_p++;
	    } else {
		switch (op->type) {
		case none:
		    return pc + op->length;
		case aabs:
		    addr = *(unsigned long *)pc;
		    return (unsigned short *)(addr & 0x00ffffff);
		case aind:
		    addr = *pc & 0xff;
		    return (unsigned short *)(*(unsigned long *)addr);
		case ret:
		    sp = (unsigned long *)get_register(SP);
		    return (unsigned short *)(*(sp+3) & 0x00ffffff);
		case reg:
		    addr = get_register((*pc >> 4) & 0x07);
		    return (unsigned short *)addr;
		case relb:
		    if ((inst = 0x55) || isbranch(inst & 0x0f))
			(unsigned char *)pc += (signed char)(*fetch_p);
		    return pc+1; /* skip myself */
		case relw:
		    if ((inst = 0x5c) || isbranch((*fetch_p & 0xf0) >> 4))
			(unsigned char *)pc += (signed short)(*(pc+1));
		    return pc+2; /* skip myself */
		}
	    }
	} else
	    op++;
    } while(--op_len > 0);
    return NULL;
}

/* Set breakpoint(s) to simulate a single step from the current PC.  */

void __single_step (void)
{
    unsigned short *nextpc;
    nextpc = getnextpc((unsigned short *)get_register(PC));
    InstBuffer.addr = nextpc;
    InstBuffer.inst = *nextpc;
    *nextpc = HAL_BREAKINST;
}

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

externC void CYG_LABEL_NAME(breakinst)(void);
int
__is_breakpoint_function ()
{
    return get_register (PC) == (target_register_t)&CYG_LABEL_NAME(breakinst);
}


/* Skip the current instruction. */
/* only TRAPA instruction */

void __skipinst (void)
{
    put_register (PC, get_register(PC) + 2);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
