/*==========================================================================
//
//      hal_misc.c
//
//      HAL miscellaneous functions
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
// Author(s):    nickg, gthomas
// Contributors: nickg, gthomas
// Date:         1999-02-20
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/hal_arm.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif
#ifdef CYGPKG_CYGMON
#include <pkgconf/cygmon.h>
#endif

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header

externC void diag_printf(const char *fmt, ...);

/*------------------------------------------------------------------------*/
/* First level C exception handler.                                       */

externC void __handle_exception (void);

externC HAL_SavedRegisters *_hal_registers;
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
externC void* volatile __mem_fault_handler;
#endif


#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
/* Force exception handling into the GDB stubs.  This is done by taking over
   the exception vectors while executing in the stubs.  This allows for the
   debugged program to handle exceptions itself, except while the GDB
   processing is underway.  The only vector that can't be handled this way
   is the illegal instruction vector which is used for breakpoint/single-step
   and must be maintained by the stubs at all times.
   Note: the interrupt vectors are _not_ preempted as the stubs probably can't
   handle them properly.
*/

#define ARM_VECTORS 8
extern unsigned long vectors[];  // exception vectors as defined by the stubs

#if !defined(CYGPKG_CYGMON)
static unsigned long *hardware_vectors = (unsigned long *)0x20;
static unsigned long hold_vectors[ARM_VECTORS];
static int exception_level;

static void
__take_over_debug_traps(void)
{
    hold_vectors[CYGNUM_HAL_VECTOR_ABORT_PREFETCH] = hardware_vectors[CYGNUM_HAL_VECTOR_ABORT_PREFETCH];
    hardware_vectors[CYGNUM_HAL_VECTOR_ABORT_PREFETCH] = vectors[CYGNUM_HAL_VECTOR_ABORT_PREFETCH];
    hold_vectors[CYGNUM_HAL_VECTOR_ABORT_DATA] = hardware_vectors[CYGNUM_HAL_VECTOR_ABORT_DATA];
    hardware_vectors[CYGNUM_HAL_VECTOR_ABORT_DATA] = vectors[CYGNUM_HAL_VECTOR_ABORT_DATA];
}

static void
__restore_debug_traps(void)
{
    hardware_vectors[CYGNUM_HAL_VECTOR_ABORT_PREFETCH] = hold_vectors[CYGNUM_HAL_VECTOR_ABORT_PREFETCH];
    hardware_vectors[CYGNUM_HAL_VECTOR_ABORT_DATA] = hold_vectors[CYGNUM_HAL_VECTOR_ABORT_DATA];
}
#endif // !CYGPKG_CYGMON
#endif

void
exception_handler(HAL_SavedRegisters *regs)
{
    // Special case handler for code which has chosen to take care
    // of data exceptions (i.e. code which expects them to happen)
    // This is common in discovery code, e.g. checking for a particular
    // device which may generate an exception when probing if the
    // device is not present
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
    if (__mem_fault_handler && 
        regs->vector == CYGNUM_HAL_EXCEPTION_DATA_ACCESS) {
        regs->pc = (unsigned long)__mem_fault_handler;
        return; // Caught an exception inside stubs        
    }
#endif

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS) && !defined(CYGPKG_CYGMON)
    if (++exception_level == 1) __take_over_debug_traps();

    _hal_registers = regs;
    __handle_exception();

    if (--exception_level == 0) __restore_debug_traps();

#elif defined(CYGPKG_KERNEL_EXCEPTIONS)

    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.

    cyg_hal_deliver_exception( regs->vector, (CYG_ADDRWORD)regs );

#else

    CYG_FAIL("Exception!!!");
    
#endif    
    
    return;
}

void hal_spurious_IRQ(HAL_SavedRegisters *regs) CYGBLD_ATTRIB_WEAK;
void
hal_spurious_IRQ(HAL_SavedRegisters *regs)
{
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
    exception_handler(regs);
#else
    CYG_FAIL("Spurious interrupt!!");
#endif    
}

/*------------------------------------------------------------------------*/
/* C++ support - run initial constructors                                 */

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

void
cyg_hal_invoke_constructors (void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    static pfunc *p = &__CTOR_END__[-1];
    
    cyg_hal_stop_constructors = 0;
    for (; p >= __CTOR_LIST__; p--) {
        (*p) ();
        if (cyg_hal_stop_constructors) {
            p--;
            break;
        }
    }
#else
    pfunc *p;

    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
        (*p) ();
#endif
}

/*------------------------------------------------------------------------*/
/* Architecture default ISR                                               */

externC cyg_uint32
hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    CYG_TRACE1(true, "Interrupt: %d", vector);

    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}

/*-------------------------------------------------------------------------*/
/* Misc functions                                                          */

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
/* This function will generate a breakpoint exception.  It is used at the
   beginning of a program to sync up with a debugger and can be used
   otherwise as a quick means to stop program execution and "break" into
   the debugger. */

void
breakpoint(void)
{
    HAL_BREAKPOINT(_breakinst);
}


/* This function returns the opcode for a 'trap' instruction.  */

unsigned long
__break_opcode (void)
{
    return HAL_BREAKINST;
}
#endif

int
hal_lsbindex(int mask)
{
    int i;
    for (i = 0;  i < 32;  i++) {
      if (mask & (1<<i)) return (i);
    }
    return (-1);
}

int
hal_msbindex(int mask)
{
    int i;
    for (i = 31;  i >= 0;  i--) {
      if (mask & (1<<i)) return (i);
    }
    return (-1);
}

#ifdef  CYGHWR_HAL_ARM_DUMP_EXCEPTIONS
void
dump_frame(unsigned char *frame)
{
    HAL_SavedRegisters *rp = (HAL_SavedRegisters *)frame;
    int i;
    diag_dump_buf(frame, 128);
    diag_printf("Registers:\n");
    for (i = 0;  i <= 10;  i++) {
        if ((i == 0) || (i == 6)) diag_printf("R%d: ", i);
        diag_printf("%08X ", rp->d[i]);
        if ((i == 5) || (i == 10)) diag_printf("\n");
    }
    diag_printf("FP: %08X, SP: %08X, LR: %08X, PC: %08X, PSR: %08X\n",
                rp->fp, rp->sp, rp->lr, rp->pc, rp->cpsr);
}
#endif

#if 0
void
show_frame_in(HAL_SavedRegisters *frame)
{
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    diag_printf("[IN] IRQ Frame:\n");
    dump_frame((unsigned char *)frame);
    HAL_RESTORE_INTERRUPTS(old);
}

void
show_frame_out(HAL_SavedRegisters *frame)
{
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    diag_printf("[OUT] IRQ Frame:\n");
    dump_frame((unsigned char *)frame);
    HAL_RESTORE_INTERRUPTS(old);
}
#endif

#ifdef  CYGHWR_HAL_ARM_DUMP_EXCEPTIONS
// Debug routines
void cyg_hal_report_undefined_instruction(HAL_SavedRegisters *frame)
{
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    diag_printf("[UNDEFINED INSTRUCTION] Frame:\n");
    dump_frame((unsigned char *)frame);
    HAL_RESTORE_INTERRUPTS(old);
}

void cyg_hal_report_software_interrupt(HAL_SavedRegisters *frame)
{
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    diag_printf("[SOFTWARE INTERRUPT] Frame:\n");
    dump_frame((unsigned char *)frame);
    HAL_RESTORE_INTERRUPTS(old);
}

void cyg_hal_report_abort_prefetch(HAL_SavedRegisters *frame)
{
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    diag_printf("[ABORT PREFETCH] Frame:\n");
    dump_frame((unsigned char *)frame);    
    HAL_RESTORE_INTERRUPTS(old);
}

void cyg_hal_report_abort_data(HAL_SavedRegisters *frame)
{
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    diag_printf("[ABORT DATA] Frame:\n");
    dump_frame((unsigned char *)frame);
    HAL_RESTORE_INTERRUPTS(old);
}

void cyg_hal_report_exception_handler_returned(HAL_SavedRegisters *frame)
{    
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    diag_printf("Exception handler returned!\n");
    dump_frame((unsigned char *)frame);
    HAL_RESTORE_INTERRUPTS(old);
}
#endif

/*------------------------------------------------------------------------*/
// EOF hal_misc.c
