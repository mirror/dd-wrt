//==========================================================================
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
// Author(s):    nickg
// Contributors: nickg, jlarmour
// Date:         1999-01-21
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling

#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>             // hal_ctrlc_isr()
#include <cyg/hal/mips-regs.h>          // FPU cause register definitions

#include CYGHWR_MEMORY_LAYOUT_H

/*------------------------------------------------------------------------*/
/* If required, define a variable to store the clock period.              */

#ifdef CYGHWR_HAL_CLOCK_PERIOD_DEFINED

CYG_WORD32 cyg_hal_clock_period;

#endif

/*------------------------------------------------------------------------*/
/* First level C exception handler.                                       */

externC void __handle_exception (void);

externC HAL_SavedRegisters *_hal_registers;

externC void* volatile __mem_fault_handler;

externC cyg_uint8 cyg_hal_mips_process_fpe( HAL_SavedRegisters *regs );

externC cyg_uint32 cyg_hal_exception_handler(HAL_SavedRegisters *regs)
{
#if defined(CYGHWR_HAL_MIPS_FPU) || \
    (defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && defined(CYGPKG_HAL_EXCEPTIONS))
    int vec = regs->vector>>2;
#endif

#if defined(CYGHWR_HAL_MIPS_FPU)
    // Special handling of FPU exceptions
    if (CYGNUM_HAL_VECTOR_FPE == vec) {

#if defined(CYGSEM_HAL_MIPS_EMULATE_UNIMPLEMENTED_FPU_OPS)
        // We may be required to emulate certain unimplemented Floating Point
        // operations

        // cyg_hal_mips_process_fpe() returns non-zero if it could handle
        // the exception successfully. If so, we just return

        if ( cyg_hal_mips_process_fpe(regs) )
            return 0;
#endif

        // Find the cause of the FPU exception, clear the flag and use
        // the decoded vector number.
        {
            cyg_uint32 cause = regs->fcr31;

            if (cause & FCR31_CAUSE_I) {
                vec = CYGNUM_HAL_EXCEPTION_FPU_INEXACT;
                cause &= ~(FCR31_FLAGS_I | FCR31_CAUSE_I);
            } else if (cause & FCR31_CAUSE_U) {
                vec = CYGNUM_HAL_EXCEPTION_FPU_UNDERFLOW;
                cause &= ~(FCR31_FLAGS_U | FCR31_CAUSE_U);
            } else if (cause & FCR31_CAUSE_O) {
                vec = CYGNUM_HAL_EXCEPTION_FPU_OVERFLOW;
                cause &= ~(FCR31_FLAGS_O | FCR31_CAUSE_O);
            } else if (cause & FCR31_CAUSE_Z) {
                vec = CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO;
                cause &= ~(FCR31_FLAGS_Z | FCR31_CAUSE_Z);
            } else if (cause & FCR31_CAUSE_V) {
                vec = CYGNUM_HAL_EXCEPTION_FPU_INVALID;
                cause &= ~(FCR31_FLAGS_V | FCR31_CAUSE_V);
            }
            regs->fcr31 = cause;

#if 1
            // Update the FPU status register with the new
            // setting. This is a workaround for the signal2 test
            // which does a longjump in the exception handler and thus
            // never gets around to executing the register restore
            // code.
            asm ("ctc1	%0,$31" : : "r" (cause));
#endif
        }
    }
#endif // CYGHWR_HAL_MIPS_FPU


#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

    // If we caught an exception inside the stubs, see if we were expecting it
    // and if so jump to the saved address
    if (__mem_fault_handler) {
        regs->pc = (CYG_HAL_MIPS_REG)(signed long)__mem_fault_handler;
        return 0; // Caught an exception inside stubs        
    }

    // Set the pointer to the registers of the current exception
    // context. At entry the GDB stub will expand the
    // HAL_SavedRegisters structure into a (bigger) register array.
    _hal_registers = regs;
    __handle_exception();

#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && defined(CYGPKG_HAL_EXCEPTIONS)

    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.
    
    cyg_hal_deliver_exception( vec, (CYG_ADDRWORD)regs );

#else
    
    CYG_FAIL("Exception!!!");
    
#endif    
    return 0;
}

/*------------------------------------------------------------------------*/
/* default ISR                                                            */

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT) &&      \
    defined(CYGHWR_HAL_GDB_PORT_VECTOR) &&              \
    defined(HAL_CTRLC_ISR)

#ifndef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN    
    if( vector == CYGHWR_HAL_GDB_PORT_VECTOR )
#endif        
    {
        cyg_uint32 result = HAL_CTRLC_ISR( vector, data );
        if( result != 0 ) return result;
    }
    
#if defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
#if defined(HAL_DIAG_IRQ_CHECK)
    {
        cyg_uint32 ret;
        /* let ROM monitor handle unexpected interrupts */
        HAL_DIAG_IRQ_CHECK(vector, ret);
        if (ret<=0)
            return ret;
    }
#endif // def HAL_DIAG_IRQ_CHECK
#endif // def CYGSEM_HAL_USE_ROM_MONITOR_CygMon
#endif
    
    CYG_TRACE1(true, "Interrupt: %d", vector);
    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}

#else // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

externC cyg_uint32 hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT) &&      \
    defined(CYGHWR_HAL_GDB_PORT_VECTOR) &&              \
    defined(HAL_CTRLC_ISR)

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
#if defined(HAL_DIAG_IRQ_CHECK)
    {
        cyg_uint32 ret;
        /* let ROM monitor handle unexpected interrupts */
        HAL_DIAG_IRQ_CHECK(vector, ret);
        if (ret<=0)
            return ret;
    }
#endif // def HAL_DIAG_IRQ_CHECK
#endif // def CYGSEM_HAL_USE_ROM_MONITOR_CygMon
#endif

#ifdef CYGPKG_REDBOOT
    hal_ctrlc_isr( vector, data );
#endif    
    
    return 0;
}

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

/*------------------------------------------------------------------------*/
/* data copy and bss zero functions                                       */

typedef void (CYG_SYM_ADDRESS)(void);

// All these must use this type of address to stop them being given relocations
// relative to $gp (i.e. assuming they would be in .sdata)
extern CYG_SYM_ADDRESS __ram_data_start;
extern CYG_SYM_ADDRESS __ram_data_end;
extern CYG_SYM_ADDRESS __rom_data_start;    

#ifdef CYG_HAL_STARTUP_ROM      
void hal_copy_data(void)
{
    char *p = (char *)&__ram_data_start;
    char *q = (char *)&__rom_data_start;
    
    while( p != (char *)&__ram_data_end )
        *p++ = *q++;
}
#endif

/*------------------------------------------------------------------------*/

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

void
cyg_hal_invoke_constructors(void)
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
    
} // cyg_hal_invoke_constructors()

/*------------------------------------------------------------------------*/
/* Determine the index of the ls bit of the supplied mask.                */

cyg_uint32 hal_lsbit_index(cyg_uint32 mask)
{
    cyg_uint32 n = mask;

    static const signed char tab[64] =
    { -1, 0, 1, 12, 2, 6, 0, 13, 3, 0, 7, 0, 0, 0, 0, 14, 10,
      4, 0, 0, 8, 0, 0, 25, 0, 0, 0, 0, 0, 21, 27 , 15, 31, 11,
      5, 0, 0, 0, 0, 0, 9, 0, 0, 24, 0, 0 , 20, 26, 30, 0, 0, 0,
      0, 23, 0, 19, 29, 0, 22, 18, 28, 17, 16, 0
    };

    n &= ~(n-1UL);
    n = (n<<16)-n;
    n = (n<<6)+n;
    n = (n<<4)+n;

    return tab[n>>26];
}

/*------------------------------------------------------------------------*/
/* Determine the index of the ms bit of the supplied mask.                */

cyg_uint32 hal_msbit_index(cyg_uint32 mask)
{
    cyg_uint32 x = mask;    
    cyg_uint32 w;

    /* Phase 1: make word with all ones from that one to the right */
    x |= x >> 16;
    x |= x >> 8;
    x |= x >> 4;
    x |= x >> 2;
    x |= x >> 1;

    /* Phase 2: calculate number of "1" bits in the word        */
    w = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
    w = w + (w >> 4);
    w = (w & 0x000F000F) + ((w >> 8) & 0x000F000F);
    return (cyg_uint32)((w + (w >> 16)) & 0xFF) - 1;

}

/*------------------------------------------------------------------------*/
/* Delay for some number of useconds.                                     */
void 
hal_delay_us(int us)
{
    cyg_uint32 val1, val2;
    int diff;
    long usticks;
    long ticks;

    // Calculate the number of counter register ticks per microsecond.
    
    usticks = (CYGNUM_HAL_RTC_PERIOD * CYGNUM_HAL_RTC_DENOMINATOR) / 1000000;

    // Make sure that the value is not zero. This will only happen if the
    // CPU is running at < 2MHz.
    if( usticks == 0 ) usticks = 1;
    
    while( us > 0 )
    {
        int us1 = us;

        // Wait in bursts of less than 10000us to avoid any overflow
        // problems in the multiply.
        if( us1 > 10000 )
            us1 = 10000;

        us -= us1;

        ticks = us1 * usticks;

        HAL_CLOCK_READ(&val1);
        while (ticks > 0) {
            do {
                HAL_CLOCK_READ(&val2);
            } while (val1 == val2);
            diff = val2 - val1;
            if (diff < 0) diff += CYGNUM_HAL_RTC_PERIOD;
            ticks -= diff;
            val1 = val2;
        }
    }
}

/*------------------------------------------------------------------------*/

void hal_arch_program_new_stack(void *_func)
{
    externC void hal_program_new_stack( void *func, CYG_ADDRESS addr);
    hal_program_new_stack( (void *)_func,
                   (CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - sizeof(CYG_ADDRESS)) & ~15 );
}

/*------------------------------------------------------------------------*/
/* Idle thread action                                                     */

#include <cyg/infra/diag.h>

void hal_idle_thread_action( cyg_uint32 count )
{
#if 0 //def CYGPKG_HAL_MIPS_SIM
    if( (count % 1000) == 0 )
    {
        // This code causes a fake interrupt.
        asm volatile (
            "xor    $24,$24,$24;"
            "mtc0   $24,$13;"
            "lui    $25,%%hi(1f);"
            "ori    $25,$25,%%lo(1f);"
            "j      other_vector;"
            "nop;"
            "1:"
            :
            :
            : "t8", "t9"
            );
    }
#endif
#if 0 //def CYGPKG_HAL_MIPS_TX39_JMR3904

    if( (count % 100000 ) == 0 )
    {
//        cyg_uint32 tval, isr, imr, ilr;
          cyg_uint32 sr = 0, cr = 0, ctr = 0, cpr = 0;
//        HAL_CLOCK_READ( &tval );
//        HAL_READ_UINT32( 0xFFFFC000, isr );
//        HAL_READ_UINT32( 0xFFFFC004, imr );
//        HAL_READ_UINT32( 0xFFFFC01C, ilr );
//        CYG_TRACE2(1, "Timer value, ISR ",tval, isr);
//        CYG_TRACE2(1, "IMR ILR0 ", imr, ilr);

//        asm volatile (
//            "mfc0  %0,$12;"
//            "nop; nop; nop;"
//            "mfc0  %1,$13;"
//            "nop; nop; nop;"
//            "mfc0  %2,$9;"
//            "nop; nop; nop;"
//            "mfc0  %3,$11;"
//            "nop; nop; nop;"
//            : "=r"(sr), "=r"(cr), "=r"(ctr), "=r"(cpr)
//            );

        
//        diag_printf("Status %08x ", sr );
//       diag_printf("Cause %08x ", cr );
//        diag_printf("Counter %08x ", ctr );
//        diag_printf("Compare %08x\n", cpr);

#if 0
        asm volatile (
            "mfc0  %0,$12;"
            "nop; nop; nop;"
            : "=r"(sr)
            );
        diag_write_string("Status "); diag_write_hex( sr );

        asm volatile (
            "mfc0  %0,$13;"
            "nop; nop; nop;"
            : "=r"(cr)
            );
        diag_write_string(" Cause "); diag_write_hex( cr );

        asm volatile (
            "mfc0  %0,$9;"
            "nop; nop; nop;"
            : "=r"(ctr)
            );
        diag_write_string(" Counter "); diag_write_hex( ctr );

        asm volatile (
            "mfc0  %0,$11;"
            "nop; nop; nop;"
            : "=r"(cpr)
            );
        diag_write_string(" Compare "); diag_write_hex( cpr );
        diag_write_string( "\n" );
        
#endif
#if 1         
        asm volatile (
            "mfc0  %0,$12;"
            "nop; nop; nop;"
            : "=r"(sr)
            );

        asm volatile (
            "mfc0  %0,$13;"
            "nop; nop; nop;"
            : "=r"(cr)
            );

        CYG_INSTRUMENT_USER( 1, sr, cr );

        asm volatile (
            "mfc0  %0,$9;"
            "nop; nop; nop;"
            : "=r"(ctr)
            );

        asm volatile (
            "mfc0  %0,$11;"
            "nop; nop; nop;"
            : "=r"(cpr)
            );
        
        CYG_INSTRUMENT_USER( 2, ctr, cpr );
#endif
        
//        if( count == 4 )
//        {
//            HAL_ENABLE_INTERRUPTS();
//        }
        
//        if( count >= 10 )
//            for(;;);
    }
#endif
#if 0
    {
        static CYG_WORD32 istat[3] = { 0xffffffff,0xffffffff,0xffffffff };
        int i;
        for( i = 0; i < 3; i++ )
        {
            CYG_WORD32 reg, sr;
            HAL_READ_UINT32( CYGHWR_HAL_MIPS_VRC4373_INTC_STAT0 + i * CYGHWR_HAL_MIPS_VRC4373_INTC_MASK_OFF, reg );
            if( reg != istat[i] )
            {
                hal_diag_ai_write_char('~');
                hal_diag_ai_write_char('0'+i);
                hal_diag_ai_write_hex8( reg );
                istat[i] = reg;
                HAL_READ_UINT32( CYGHWR_HAL_MIPS_VRC4373_INTC_MASK0 + i * CYGHWR_HAL_MIPS_VRC4373_INTC_MASK_OFF, reg );
                hal_diag_ai_write_char('.');                
                hal_diag_ai_write_hex8( reg );
#if 0                
                asm volatile (
                    "mfc0  %0,$12;"
                    "nop; nop; nop;"
                    : "=r"(sr)
                    );
                hal_diag_ai_write_char('.');                
                hal_diag_ai_write_hex8( sr );
#endif
            }
        }
    }
    {
        static CYG_WORD32 old_pins = 0;
        CYG_WORD32 reg;        
        HAL_READ_UINT32( CYGHWR_HAL_MIPS_VRC4373_INTC_PINS, reg );
        if( reg != old_pins )
        {
            hal_diag_ai_write_char('%');                
            hal_diag_ai_write_hex8( reg );
            old_pins = reg;
        }
    }
#endif
#if 0 //def CYGPKG_HAL_MIPS_VR4300_VRC4373

    // Wiggle one of the leds to show we are running
    
    if( (count % 50000 ) == 0 )
    {
        cyg_uint8 lr;
        { int i; for( i = 0; i < 200; i++ ); }
        HAL_READ_UINT8( 0xc2000008, lr );
        lr ^= 2;
        { int i; for( i = 0; i < 200; i++ ); }
        HAL_WRITE_UINT8( 0xc2000008, lr );

    }
#endif    
}

/*------------------------------------------------------------------------*/
/* End of hal_misc.c                                                      */
