#ifndef CYGONCE_HAL_I386_STUB_H
#define CYGONCE_HAL_I386_STUB_H
//==========================================================================
//
//      i386_stub.h
//
//      i386/PC GDB stub support
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
// Author(s):    pjo
// Contributors: pjo, nickg
// Date:         1999-10-15
// Purpose:      i386/PC GDB stub support
// Description:  Definitions for the GDB stubs. Most of this comes from
//               the original libstub code.
//              
// Usage:
//               #include <cyg/hal/plf_intr.h>
//               ...
//
//####DESCRIPTIONEND####
//
//==========================================================================


#ifndef CYGHWR_HAL_I386_FPU
#define NUMREGS     16
#else
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
#define NUMREGS     41
#else
#define NUMREGS     32
#endif
#endif

// Size of a given register.
#define REGSIZE(X)  (((X) >= REG_FST0  && (X) <= REG_FST7) ? 10 :              \
                    (((X) >= REG_MMX0  && (X) <= REG_MMX7) ? 10 :              \
                    (((X) == REG_GDT || (X) == REG_IDT) ? 6 :                  \
                    (((X) == REG_MSR || (X) == REG_LDT || (X) == REG_TR) ? 8 : \
                    (((X) >= REG_XMM0  && (X) <= REG_XMM7) ? 16 : 4)))))

enum regnames
{
    // General regs (0 - 15)
    EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI,
    PC /* also known as eip */,
    PS /* also known as eflags */,
    CS, SS, DS, ES, FS, GS,

    // FPU regs. (16 - 31)
    REG_FST0, REG_FST1, REG_FST2, REG_FST3,
    REG_FST4, REG_FST5, REG_FST6, REG_FST7,
    REG_FCTRL, REG_FSTAT, REG_FTAG,
    REG_FISEG, REG_FIOFF,
    REG_FOSEG, REG_FOOFF,
    REG_FOP,

    // SSE regs (32 - 40)
    REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3,
    REG_XMM4, REG_XMM5, REG_XMM6, REG_XMM7,
    REG_MXCSR,

    // Registers below this point are _not_ part of the g/G packet
    // so they play no part in defining NUMREGS. GDB accesses these
    // individually with p/P packets.

    // MMX regs (41 - 48)
    // These are here as placeholders for register numbering purposes.
    // GDB never asks for these directly as their values are stored
    // in FSTn registers.
    REG_MMX0, REG_MMX1, REG_MMX2, REG_MMX3,
    REG_MMX4, REG_MMX5, REG_MMX6, REG_MMX7,

    // Misc pentium regs (49 - 56)
    REG_CR0, REG_CR2, REG_CR3, REG_CR4,
    REG_GDT, REG_IDT, REG_LDT, REG_TR,

    // Pseudo reg used by gdb to access other regs (57)
    REG_MSR
};

typedef enum regnames regnames_t ;
typedef unsigned long target_register_t ;

typedef struct
{
    target_register_t eax;
    target_register_t ecx;
    target_register_t edx;
    target_register_t ebx;
    target_register_t esp;
    target_register_t ebp;
    target_register_t esi;
    target_register_t edi;
    target_register_t pc;
    target_register_t ps;
    target_register_t cs;
    target_register_t ss;
    target_register_t ds;
    target_register_t es;
    target_register_t fs;
    target_register_t gs;
#ifdef CYGHWR_HAL_I386_FPU
    target_register_t fcw;
    target_register_t fsw;
    target_register_t ftw;
    target_register_t ipoff;
    target_register_t cssel;
    target_register_t dataoff;
    target_register_t opsel;
    unsigned char     st0[10];
    unsigned char     st1[10];
    unsigned char     st2[10];
    unsigned char     st3[10];
    unsigned char     st4[10];
    unsigned char     st5[10];
    unsigned char     st6[10];
    unsigned char     st7[10];
#endif
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
    unsigned char     xmm0[16];
    unsigned char     xmm1[16];
    unsigned char     xmm2[16];
    unsigned char     xmm3[16];
    unsigned char     xmm4[16];
    unsigned char     xmm5[16];
    unsigned char     xmm6[16];
    unsigned char     xmm7[16];
    target_register_t mxcsr;
#endif
#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
    target_register_t cr0;
    target_register_t cr2;
    target_register_t cr3;
    target_register_t cr4;

    unsigned char     gdtr[6];
    unsigned char     idtr[6];
    target_register_t ldt;
    target_register_t tr;
#endif
} GDB_SavedRegisters;

#define HAL_STUB_REGISTERS_SIZE \
 ((sizeof(GDB_SavedRegisters) + sizeof(target_register_t) - 1) / sizeof(target_register_t))

#define PS_C			0x00000001
#define PS_Z			0x00000040
#define PS_V			0x00000080
#define PS_T			0x00000100

#define SP			(ESP)
#define EIP			(PC)

// We have to rewind the PC only in case of a breakpoint
// set by a user interrupt (ie: a Ctrl-C)

// There should be another way to do it

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#define HAL_STUB_PLATFORM_STUBS_FIXUP()                         \
    CYG_MACRO_START                                             \
    if (break_buffer.targetAddr==get_register(PC)-1)		\
        put_register(PC, get_register(PC) - 1);                 \
    CYG_MACRO_END
#endif //CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT

// I386 stub has special needs for register handling, so special
// put_register and get_register are provided.
#define CYGARC_STUB_REGISTER_ACCESS_DEFINED 1

// The x86 has float (and other) registers that are larger than it can
// hold in a target_register_t.
#define TARGET_HAS_LARGE_REGISTERS

#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
// Handle arch-specific query packets.
extern int cyg_hal_stub_process_query (char *p, char *b, int n);
#define CYG_HAL_STUB_PROCESS_QUERY(__pkt__, __buf__, __bufsz__) \
  cyg_hal_stub_process_query ((__pkt__), (__buf__), (__bufsz__))

// These masks are used to protect the user from writing to reserved bits
// of the control registers.
#define REG_CR4_MASK    0x000007FF
#define REG_CR3_MASK    0xFFFFF018
#define REG_CR2_MASK    0xFFFFFFFF
#define REG_CR0_MASK    0xE005003F
#endif

/* Find out what our last trap was. */
extern int __get_trap_number(void) ;

/* Given a trap number, compute the signal code for it. */
extern int __computeSignal(unsigned int trap_number) ;

/* Enable single stepping after the next user resume instruction. */
extern void __single_step(void) ;
extern void __clear_single_step(void) ;

extern void __install_breakpoints(void) ;
extern void __clear_breakpoints(void) ;


//---------------------------------------------------------------------------
#endif /* CYGONCE_HAL_I386_STUB_H */
// End of i386_stub.h
