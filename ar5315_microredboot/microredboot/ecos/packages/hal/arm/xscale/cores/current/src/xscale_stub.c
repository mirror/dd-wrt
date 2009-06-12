//==========================================================================
//
//      xscale_stub.c
//
//      HAL stub support code for Intel XScale cores.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-10-18
// Purpose:      XScale core stub support
// Description:  Implementations of HW debugging support.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_stub.h>           // Stub macros

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
/*------------------------------------------------------------------------*/
//  HW Debug support

// Define this to support two watchpoints. If not defined, one watchpoint with
// a power of two range is supported.
#define USE_TWO_WATCHPOINTS 1

static inline void set_ibcr0(unsigned x)
{
    asm volatile ("mcr p15,0,%0,c14,c8,0" : : "r"(x) );
}

static inline unsigned get_ibcr0(void)
{
    unsigned x;
    asm volatile ("mrc p15,0,%0,c14,c8,0" : "=r"(x) : );
    return x;
}

static inline void set_ibcr1(unsigned x)
{
    asm volatile ("mcr p15,0,%0,c14,c9,0" : : "r"(x) );
}

static inline unsigned get_ibcr1(void)
{
    unsigned x;
    asm volatile ("mrc p15,0,%0,c14,c9,0" : "=r"(x) : );
    return x;
}

static inline void set_dbr0(unsigned x)
{
    asm volatile ("mcr p15,0,%0,c14,c0,0" : : "r"(x) );
}

static inline unsigned get_dbr0(void)
{
    unsigned x;
    asm volatile ("mrc p15,0,%0,c14,c0,0" : "=r"(x) : );
    return x;
}

static inline void set_dbr1(unsigned x)
{
    asm volatile ("mcr p15,0,%0,c14,c3,0" : : "r"(x) );
}

static inline unsigned get_dbr1(void)
{
    unsigned x;
    asm volatile ("mrc p15,0,%0,c14,c3,0" : "=r"(x) : );
    return x;
}

static inline void set_dbcon(unsigned x)
{
    asm volatile ("mcr p15,0,%0,c14,c4,0" : : "r"(x) );
}

static inline unsigned get_dbcon(void)
{
    unsigned x;
    asm volatile ("mrc p15,0,%0,c14,c4,0" : "=r"(x) : );
    return x;
}

static inline void set_dcsr(unsigned x)
{
    asm volatile ("mcr p14,0,%0,c10,c0,0" : : "r"(x) );
}

static inline unsigned get_dcsr(void)
{
    unsigned x;
    asm volatile ("mrc p14,0,%0,c10,c0,0" : "=r"(x) : );
    return x;
}

int cyg_hal_plf_hw_breakpoint(int setflag, void *vaddr, int len)
{
    unsigned int addr = (unsigned)vaddr;

    if (setflag) {
	if (!(get_ibcr0() & 1))
	    set_ibcr0(addr | 1);
	else if (!(get_ibcr1() & 1))
	    set_ibcr1(addr | 1);
	else
	    return -1;
    } else {
	unsigned x = (addr | 1);
	if (get_ibcr0() == x)
	    set_ibcr0(0);
	else if (get_ibcr1() == x)
	    set_ibcr1(0);
	else
	    return -1;
    }

    return 0;
}

#define WATCH_MODE_NONE   0
#define WATCH_MODE_WRITE  1
#define WATCH_MODE_ACCESS 2
#define WATCH_MODE_READ   3

#ifndef HAL_STUB_HW_WATCHPOINT_LIST_SIZE
#error
#endif

int cyg_hal_plf_hw_watchpoint(int setflag, void *vaddr, int len, int type)
{
    unsigned int mode, addr = (unsigned)vaddr;
    unsigned dbcon = get_dbcon();
#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 1
    unsigned int mask, bit_nr;

    mask = 0x80000000;
    bit_nr = 31;
    while (bit_nr) {
	if (len & mask)
	    break;
	bit_nr--;
	mask >>= 1;
    }
    mask = ~(0xffffffff << bit_nr);
#endif

    if (setflag) {
	/* set a watchpoint */
	if (type == 2)
	    mode = WATCH_MODE_WRITE;  // break on write
	else if (type == 3)
	    mode = WATCH_MODE_READ;   // break on read
	else if (type == 4)
	    mode = WATCH_MODE_ACCESS; // break on any access
	else
	    return 1;

#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 1
	mode |= 0x100;
#endif

	if (!(dbcon & 3)) {
	    set_dbr0(addr);
#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 1
	    set_dbr1(mask);
#endif
	    set_dbcon(dbcon | mode);
#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 2
	} else if (!(dbcon & (3 << 2))) {
	    set_dbr1(addr);
	    set_dbcon(dbcon | (mode << 2));
#endif
	} else
	    return 1;

    } else {
	/* clear a watchpoint */
	if ((dbcon & 3) && get_dbr0() == addr)
	    set_dbcon(dbcon & ~3);
#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 2
	else if ((dbcon & (3 << 2)) && get_dbr1() == addr)
	    set_dbcon(dbcon & ~(3 << 2));
#endif
	else
	    return 1;
    }
    return 0;
}

#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 2

// The XScale hardware does not provide a way of positively identinfying
// which of the two watchpoints triggered and exception. The following
// code makes a best effort at determining this by decoding the opcode
// of the instruction which caused the watchpoint trigger. It is *not*
// 100% reliable.

// Some bits common to most ld/st instructions.
#define I_bit (1 << 25)
#define P_bit (1 << 24)
#define U_bit (1 << 23)
#define B_bit (1 << 22)
#define W_bit (1 << 21)
#define L_bit (1 << 20)

// Return non-zero if opcode at given PC is a store instruction for
// purposes of triggering watchpoints.
static int
is_store_insn(unsigned pc)
{
    unsigned opcode = *(unsigned *)pc;

    if ((opcode & 0x0fb00ff0) == 0x01000090) {
	// SWP          xxxx 0001 0B00 _Rn_ _Rd_ 0000 1001 _Rm_
	return 1;
    }

    if ((opcode & 0x0c000000) == 0x04000000) {
	// LDR/STR      xxxx 010P UBWL _Rn_ _Rd_ iiii iiii iiii
	// LDR/STR      xxxx 011P UBWL _Rn_ _Rd_ ssss sSh0 _Rm_
	// Addressing mode 2,  Load/Store word or unsigned byte
	return (opcode & L_bit) == 0;
    }

    if ((opcode & 0x0e000090) == 0x00000090 &&
	(opcode & 0x00000060) &&
	((opcode & (1 << 22)) || (opcode & 0x00000f00) == 0) &&
	((opcode & (P_bit | W_bit)) != W_bit)) {
	// LDR/STR    xxxx 000P U1WL _Rn_ _Rd_ iiii 1SH1 iiii
	// LDR/STR    xxxx 000P U0WL _Rn_ _Rd_ 0000 1SH1 _Rm_
	// Addressing Mode 3, Load/Store halfword, load signed byte
	return (opcode & L_bit) == 0;
    }

    if ((opcode & 0x0e000000) == 0x08000000) {
	// LDM/STM      xxxx 100P USWL _Rn_ rrrr rrrr rrrr rrrr
	return (opcode & L_bit) == 0;
    }

    if ((opcode & 0x0e000000) == 0x0c000000) {
	// LDC/STC      xxxx 110P UNWL _Rn_ CRd_ CP#_ iiii iiii
	return (opcode & L_bit) == 0;
    }

    return 0;
}

static int
is_thumb_store_insn(unsigned pc)
{
    unsigned short opcode = *(unsigned short *)pc;

    opcode &= 0xfe00;

    if (opcode == 0xb400)
	return 1;  // PUSH
    if (opcode == 0x5000)
	return 1;  // STR Rd, [Rn, Rm]
    if (opcode == 0x5400)
	return 1;  // STRB Rd, [Rn, Rm]
    if (opcode == 0x5200)
	return 1;  // STRH Rd, [Rn, Rm]

    opcode &= 0xf800;
    if (opcode == 0xc000)
	return 1;  // STM
    if (opcode == 0x6000)
	return 1;  // STR Rd, [Rn, #5bit_offset]
    if (opcode == 0x9000)
	return 1;  // STR Rd, [SP, #8bit_offset]
    if (opcode == 0x7000)
	return 1;  // STRB Rd, [Rn, #5bit_offset]
    if (opcode == 0x8000)
	return 1;  // STRH Rd, [Rn, #5bit_offset]

    return 0;
}

// Return non-zero if given waddr matches an access at addr.
static int
waddr_match(unsigned waddr, unsigned addr, int size)
{
    if (addr <= waddr && waddr < (addr + size))
	return 1;
    return 0;
}

// Return non-zero if given value matches value at watchpoint address.
static int
wval_match(unsigned waddr, unsigned val, int size)
{
    unsigned wval = *(unsigned *)(waddr & ~3);
    int i;

    if (size == 4)
	return (wval == val);
    if (size == 2) {
	val &= 0xffff;
	return ((wval & 0xffff) == val || ((wval >> 16) == val));
    }
    if (size == 1) {
	val &= 0xff;
	for (i = 0; i < 4; i++) {
	    if ((wval & 0xff) == val)
		return 1;
	    wval >>= 8;
	}
    }
    return 0;
}

static char _sztab[8] = { 4, 2, 1, 1, 4, 2, 1, 2 };

// Given the watch addresses and watch modes for each of the enabled
// watchpoints, figure out which one triggered the current exception.
static unsigned
find_thumb_watch_address(unsigned wa0, int mode0, unsigned wa1, int mode1)
{
    unsigned pc = get_register(PC) - 4;
    unsigned short opcode = *(unsigned short *)pc;
    unsigned short opcode_f8, opcode_fe;
    unsigned val, wd0, wd1, addr = 0;
    int  is_store, use_val, i, offset, size, Rn, Rd, Rm;

    opcode_f8 = opcode & 0xf800;
    opcode_fe = opcode & 0xfe00;

    size = 0;
    is_store = 0;
    val = use_val = 0;

    switch (opcode_f8) {
    case 0xc000: // STMIA Rn!, <list>
	is_store = 1;
    case 0xc800: // LDMIA Rn!, <list>
	Rn = (opcode >> 8) & 7;
	is_store = (opcode & 0x800) == 0;
	for (i = 0; i < 8; i++)
	    if (opcode & (1 << i))
		size += 4;
	if (!is_store && (opcode & (1 << Rn))) {
	    // We can't reconstruct address from opcode because base
	    // was destroyed. Best we can do is try to match data at
            // watchpoint addresses with data in one of the registers.
	    wd0 = *(unsigned *)(wa0 & ~3);
	    wd1 = *(unsigned *)(wa1 & ~3);
	    if (wd0 != wd1) {
		for (i = size = 0; i < 8; i++) {
		    if (opcode & (1 << i)) {
			val = get_register(i);
			if (val == wd0)
			    return wa0;
			else if (val == wd1)
			    return wa1;
		    }
		}
	    }
	    return wa0;  // 50% chance of being right
	} else
	    addr = get_register(Rn) - size;
	break;
    case 0x6000: // STR  Rd, [Rn, #5bit]
    case 0x7000: // STRB Rd, [Rn, #5bit]
    case 0x8000: // STRH Rd, [Rn, #5bit]
	is_store = 1;
    case 0x6800: // LDR  Rd, [Rn, #5bit]
    case 0x7800: // LDRB Rd, [Rn, #5bit]
    case 0x8800: // LDRH Rd, [Rn, #5bit]
	Rd = opcode & 7;
	Rn = (opcode >> 3) & 7;
	if ((opcode & 0xf000) == 0x6000)
	    size = 4;
	else if ((opcode & 0xf000) == 0x8000)
	    size = 2;
	else
	    size = 1;
	if (!is_store && Rd == Rn) {
	    // We can't reconstruct address from opcode because base
	    // or offset register was destroyed. Best we can do is try
	    // to match data at watchpoint addresses with data in Rd.
	    use_val = 1;
	    val = get_register(Rd);
	} else {
	    offset = ((opcode >> 6) & 0x1f) * size;
	    addr = get_register(Rn) + offset;
	}
	break;
    case 0x4800: // LDR Rd, [PC, #8bit]
	size = 4;
	addr = pc + 4 + ((opcode & 0xff) * 4);
	break;
    case 0x9000: // STR Rd, [SP, #8bit]
	is_store = 1;
    case 0x9800: // LDR Rd, [SP, #8bit]
	size = 4;
	addr = get_register(SP) + ((opcode & 0xff) * 4);
	break;
    default:
	switch (opcode_fe) {
	case 0x5000:  // STR   Rd, [Rn, Rm]
	case 0x5400:  // STRB  Rd, [Rn, Rm]
	case 0x5200:  // STRH  Rd, [Rn, Rm]
	    is_store = 1;
	case 0x5600:  // LDRSB Rd, [Rn, Rm]
	case 0x5800:  // LDR   Rd, [Rn, Rm]
	case 0x5c00:  // LDRB  Rd, [Rn, Rm]
	case 0x5a00:  // LDRH  Rd, [Rn, Rm]
	case 0x5e00:  // LDRSH Rd, [Rn, Rm]
	    Rd = opcode & 7;
	    Rn = (opcode >> 3) & 7;
	    Rm = (opcode >> 6) & 7;
	    size = _sztab[(opcode >> 9) & 7];
	    if (!is_store && (Rd == Rn || Rd == Rm)) {
		// We can't reconstruct address from opcode because base
		// or offset register was destroyed. Best we can do is try
		// to match data at watchpoint addresses with data in Rd.
		use_val = 1;
		val = get_register(Rd);
	    } else
		addr = Rn + Rm;
	    break;
	case 0xb400:  // PUSH
	    is_store = 1;
	case 0xbc00:  // POP
	    for (i = 0; i < 9; i++)
		if (opcode & (1 << i))
		    size += 4;
	    addr = get_register(SP);
	    if (!is_store)
		addr -= size;
	    break;
	}
	break;
    }
    if (use_val) {
	// We can read from watchpoint addresses and compare against
	// whatever is in the Rd from a load. This is not perfect,
	// but its the best we can do.
	if (wval_match(wa0, val, size))
	    return wa0;
	if (wval_match(wa1, val, size))
	    return wa1;
    } else if (size) {
	if (waddr_match(wa0, addr, size))
	    return wa0;
	if (waddr_match(wa1, addr, size))
	    return wa1;
    }
    return wa0;  // should never happen, but return valid address
}

// Given the watch addresses and watch modes for each of the enabled
// watchpoints, figure out which one triggered the current exception.
static unsigned
find_watch_address(unsigned wa0, int mode0, unsigned wa1, int mode1)
{
    unsigned pc = get_register(PC) - 4;
    unsigned cpsr = get_register(PS);
    unsigned opcode, Rn, Rd, Rm, base, addr, val = 0, wd0, wd1;
    int  is_store, use_val, i, offset, shift, size;

    if (cpsr & CPSR_THUMB_ENABLE)
	is_store = is_thumb_store_insn(pc);
    else
	is_store = is_store_insn(pc);

    // First try the easy cases where all we need to know is whether or
    // not the instruction is a load or store.
    if ((mode0 == WATCH_MODE_READ && mode1 == WATCH_MODE_WRITE) ||
	(mode1 == WATCH_MODE_READ && mode0 == WATCH_MODE_WRITE)) {
	if (is_store)
	    return (mode0 == WATCH_MODE_WRITE) ? wa0 : wa1;
	else
	    return (mode0 == WATCH_MODE_READ) ? wa0 : wa1;
    }
    if ((mode0 == WATCH_MODE_READ && is_store) ||
	(mode0 == WATCH_MODE_WRITE && !is_store))
	return wa1;
    if ((mode1 == WATCH_MODE_READ && is_store) ||
	(mode1 == WATCH_MODE_WRITE && !is_store))
	return wa0;

    // Okay. Now try to figure out address by decoding the opcode.

    if (cpsr & CPSR_THUMB_ENABLE)
	return find_thumb_watch_address(wa0, mode0, wa1, mode1);

    opcode = *(unsigned *)pc;
    Rn = (opcode >> 16) & 15;
    Rd = (opcode >> 12) & 15;
    Rm = opcode & 15;

    size = use_val = 0;
    addr = 0;

    if ((opcode & 0x0fb00ff0) == 0x01000090) {
	// SWP          xxxx 0001 0B00 _Rn_ _Rd_ 0000 1001 _Rm_
	addr = get_register(Rn);
	size = (opcode & B_bit) ? 1 : 4;
    } else if ((opcode & 0x0c000000) == 0x04000000) {
	// LDR/STR      xxxx 010P UBWL _Rn_ _Rd_ iiii iiii iiii
	// LDR/STR      xxxx 011P UBWL _Rn_ _Rd_ ssss sSh0 _Rm_
	// Addressing mode 2,  Load/Store word or unsigned byte

	size = (opcode & B_bit) ? 1 : 4;

	if ((opcode & (P_bit | W_bit)) == (P_bit | W_bit)) {
	    // This is easy because address is written back to Rn
	    addr = get_register(Rn);
	} else if (!is_store &&
		   (Rd == Rn || ((opcode & I_bit) && Rd == Rm))) {
	    // We can't reconstruct address from opcode because base
	    // or offset register was destroyed. Best we can do is try
	    // to match data at watchpoint addresses with data in Rd.
	    use_val = 1;
	    val = get_register(Rd);
	} else {
	    if (opcode & I_bit) {
		shift = (opcode >> 7) & 0x1f;
		offset = get_register(Rm);
		switch ((opcode >> 5) & 3) {
		case 0:
		    offset <<= shift;
		    break;
		case 1:
		    offset >>= shift;
		    offset &= (0xffffffffU >> shift);
		    break;
		case 2:
		    offset >>= shift;
		    break;
		case 3:
		    if (shift) {
			for (i = 0; i < shift; i++)
			    offset = ((offset >> 1) & 0x7fffffff) | ((offset & 1) << 31);
		    } else {
			offset >>= 1;
			offset &= 0x80000000;
			offset |= ((cpsr & 0x20000000) << 2);
		    }
		    break;
		}
	    } else
		offset = opcode & 0xfff;

	    if ((opcode & U_bit) == 0)
		offset = -offset;

	    if (Rn == 15)
		base = pc + 8;
	    else
		base = get_register(Rn);

	    if (opcode & P_bit)
		addr = base + offset; // pre-index
	    else
		addr = base - offset; // post-index writeback

	    size = (opcode & B_bit) ? 1 : 4;
	}
    } else if ((opcode & 0x0e000090) == 0x00000090 &&
	       (opcode & 0x00000060) &&
	       ((opcode & (1 << 22)) || (opcode & 0x00000f00) == 0) &&
	       ((opcode & (P_bit | W_bit)) != W_bit)) {
	// LDR/STR    xxxx 000P U1WL _Rn_ _Rd_ iiii 1SH1 iiii
	// LDR/STR    xxxx 000P U0WL _Rn_ _Rd_ 0000 1SH1 _Rm_
	// Addressing Mode 3, Load/Store halfword, load signed byte

	size = (opcode & (1 << 5)) ? 2 : 1;

	if ((opcode & (P_bit | W_bit)) == (P_bit | W_bit)) {
	    // This is easy because address is written back to Rn
	    addr = get_register(Rn);
	} if (!is_store &&
	      (Rd == Rn || ((opcode & (1 << 22)) && Rd == Rm))) {
	    // We can't reconstruct address from opcode because base
	    // or offset register was destroyed. Best we can do is try
	    // to match data at watchpoint addresses with data in Rd.
	    use_val = 1;
	    val = get_register(Rd);
	} else {
	    if (opcode & (1 << 22))
		offset = ((opcode >> 4) & 0xf0) | (opcode & 0x0f);
	    else
		offset = get_register(opcode & 15);

	    if ((opcode & U_bit) == 0)
		offset = -offset;

	    if (Rn == 15)
		base = pc + 8;
	    else
		base = get_register(Rn);

	    if (opcode & P_bit)
		addr = base + offset; // pre-index
	    else
	    addr = base - offset; // post-index writeback
	}
    } else if ((opcode & 0x0e000000) == 0x08000000) {
	// LDM/STM      xxxx 100P USWL _Rn_ rrrr rrrr rrrr rrrr
	for (i = size = 0; i < 16; i++)
	    if (opcode & (1 << i))
		size += 4;

	base = get_register(Rn);
	if (!is_store && (opcode & (1 << Rn))) {
	    // We can't reconstruct address from opcode because base
	    // was destroyed. Best we can do is try to match data at
            // watchpoint addresses with data in one of the registers.
	    wd0 = *(unsigned *)(wa0 & ~3);
	    wd1 = *(unsigned *)(wa1 & ~3);
	    if (wd0 != wd1) {
		for (i = size = 0; i < 16; i++) {
		    if (opcode & (1 << i)) {
			val = get_register(i);
			if (val == wd0)
			    return wa0;
			else if (val == wd1)
			    return wa1;
		    }
		}
	    }
	    return wa0;  // 50% chance of being right
	} else {
	    if (opcode & U_bit){
		if (opcode & W_bit)
		    addr = base - size;
		else
		    addr = base;
		if (opcode & P_bit)
		    addr += 4;
	    } else {
		if (opcode & W_bit)
		    addr = base;
		else
		    addr = base - size;
		if ((opcode & P_bit) == 0)
		    addr += 4;
	    }
	}
    } else if ((opcode & 0x0e000000) == 0x0c000000) {
	// LDC/STC      xxxx 110P UNWL _Rn_ CRd_ CP#_ iiii iiii
	size = 4;
	offset = (opcode & 0xff) * 4;
	if ((opcode & U_bit) == 0)
	    offset = -offset;
	if ((opcode & P_bit) && (opcode & W_bit))
	    addr = get_register(Rn);
	else
	    addr = get_register(Rn) + offset;
    }

    if (use_val) {
	// We can read from watchpoint addresses and compare against
	// whatever is in the Rd from a load. This is not perfect,
	// but its the best we can do.
	if (wval_match(wa0, val, size))
	    return wa0;
	if (wval_match(wa1, val, size))
	    return wa1;
    } else {
	if (waddr_match(wa0, addr, size))
	    return wa0;
	if (waddr_match(wa1, addr, size))
	    return wa1;
    }
    return wa0;  // should never happen, but return valid address
}
#endif

// Return indication of whether or not we stopped because of a
// watchpoint or hardware breakpoint. If stopped by a watchpoint,
// also set '*data_addr_p' to the data address which triggered the
// watchpoint.
int cyg_hal_plf_is_stopped_by_hardware(void **data_addr_p)
{
    unsigned fsr, dcsr, dbcon, kind = 0;

    // Check for debug event
    asm volatile ("mrc p15,0,%0,c5,c0,0" : "=r"(fsr) : );
    if ((fsr & 0x200) == 0)
	return HAL_STUB_HW_STOP_NONE;

    // There was a debug event. Check the MOE for details
    dcsr = get_dcsr();
    switch ((dcsr >> 2) & 7) {
      case 1:  // HW breakpoint
      case 3:  // BKPT breakpoint
	return HAL_STUB_HW_STOP_BREAK;
      case 2:  // Watchpoint
	dbcon = get_dbcon();
#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 2
	if (dbcon & 0x100) {
#endif
	    if ((kind = (dbcon & 3)) != WATCH_MODE_NONE)
		*data_addr_p = (void *)get_dbr0();
#if HAL_STUB_HW_WATCHPOINT_LIST_SIZE == 2
	} else {
	    // This can get tricky because the debug unit offers no way to
	    // tell which watchpoint triggered.
	    if ((kind = (dbcon & 3)) == WATCH_MODE_NONE) {
		if ((kind = ((dbcon >> 2) & 3)) != WATCH_MODE_NONE)
		    *data_addr_p = (void *)get_dbr1();
	    } else if ((kind = ((dbcon >> 2) & 3)) == WATCH_MODE_NONE) {
		if ((kind = (dbcon & 3)) != WATCH_MODE_NONE)
		    *data_addr_p = (void *)get_dbr0();
	    } else {
		// This is the tricky part. We have to look at the trapping
		// opcode (which has already issued) to try to see if we can
		// tell which watchpoint triggered. Turn off watchpoints while
		// we figure this out.
		set_dcsr(dcsr & ~0x80000000);
		*data_addr_p = (void *)find_watch_address(get_dbr0(), dbcon & 3,
							  get_dbr1(), (dbcon >> 2) & 3);
		set_dcsr(dcsr);

		if (*data_addr_p == (void *)get_dbr0())
		    kind = dbcon & 3;
		else if (*data_addr_p == (void *)get_dbr1())
		    kind = (dbcon >> 2) & 3;
		else
		    kind = WATCH_MODE_NONE;
	    }
	}
#endif
	if (kind == WATCH_MODE_WRITE)
	    return HAL_STUB_HW_STOP_WATCH;
	if (kind == WATCH_MODE_ACCESS)
	    return HAL_STUB_HW_STOP_AWATCH;
	if (kind == WATCH_MODE_READ)
	    return HAL_STUB_HW_STOP_RWATCH;
	// should never get here
	break;
    }
    return HAL_STUB_HW_STOP_NONE;
}
#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

/*------------------------------------------------------------------------*/
// EOF xscale_stub.c

