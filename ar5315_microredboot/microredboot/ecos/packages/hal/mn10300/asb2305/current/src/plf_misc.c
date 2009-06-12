//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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
// Author(s):    dmoseley (based on the original by nickg)
// Contributors: nickg, jlarmour, dmoseley
// Date:         2000-08-11
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

#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling

#include <cyg/hal/hal_cache.h>          // Cache handling

#include <cyg/hal/hal_if.h>

#include <cyg/hal/plf_io.h>

const cyg_uint8 hal_diag_digits[] = {
        0x81, // 0
        0xf3, // 1
        0x49, // 2
        0x61, // 3
        0x33, // 4
        0x25, // 5
        0x05, // 6
        0xf1, // 7
        0x01, // 8
        0x21, // 9
        0x11, // A
        0x07, // B
        0x8d, // C
        0x43, // D
        0x0d, // E
        0x1d  // F
};

const char hal_diag_hex_digits[] = "0123456789ABCDEF";

cyg_uint32 hal_led_old_display = 0x5f17ffff; /* "rh  " */

/*------------------------------------------------------------------------*/
/* LED support                                                            */
cyg_uint8 cyg_hal_plf_led_val(CYG_WORD hexdig)
{
    return hal_diag_digits[(hexdig & 0xF)];
}

/*------------------------------------------------------------------------*/

#include CYGHWR_MEMORY_LAYOUT_H
#if defined(CYGPKG_CYGMON)
extern unsigned long cygmon_memsize;
#endif

void hal_platform_init(void)
{
    *(cyg_uint8*)(&hal_led_old_display) = cyg_hal_plf_led_val(8);
    HAL_WRITE_UINT32(HAL_LED_ADDRESS,hal_led_old_display);

#if defined(CYG_HAL_STARTUP_ROM)
    // Note that the hardware seems to come up with the
    // caches containing random data. Hence they must be
    // invalidated before being enabled.
    // However, we only do this if we are in ROM. If we are
    // in RAM, then we leave the caches in the state chosen
    // by the ROM monitor. If we enable them when the monitor
    // is not expecting it, we can end up breaking things if the
    // monitor is not doing cache flushes.

    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
#endif

#if defined(CYGPKG_CYGMON)
    cygmon_memsize = 16 * 1024 * 1024 - 0x200;  // 16 MB - 0x200 (for _hal_vsr_table and _hal_virtual_vector_table)
#endif

    // Set up eCos/ROM interfaces
    hal_if_init();
    
#if defined(CYGPKG_KERNEL)                      && \
    defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT)   && \
    defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)
    {
        extern CYG_ADDRESS hal_virtual_vector_table[32];
        extern void patch_dbg_syscalls(void * vector);
        patch_dbg_syscalls( (void *)(&hal_virtual_vector_table[0]) );
    }
#endif    
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
    {
        static void hal_ctrlc_isr_init(void);
        hal_ctrlc_isr_init();
    }
#endif    

    // Make sure the TBR points at the base of ROM
#if 0
    {
        #define TBR 0xC0000024
        cyg_uint32 TBR_val;
        HAL_READ_UINT32(TBR, TBR_val);
        TBR_val = (TBR_val & 0x00FFFFFF) | 0x90000000; //(CYGMEM_REGION_rom & 0xFF000000);
        HAL_WRITE_UINT32(TBR, TBR_val);
    }
#endif

    // Make sure the MTBR points at the base of ROM
#if 0
    {
        #define mTBR 0xC0000028
        cyg_uint32 mTBR_val;
        HAL_READ_UINT32(mTBR, mTBR_val);
        mTBR_val = (mTBR_val & 0x00FFFFFF) | (CYGMEM_REGION_rom & 0xFF000000);
        HAL_WRITE_UINT32(mTBR, mTBR_val);
    }
#endif
}

/*------------------------------------------------------------------------*/
/* Functions to support the detection and execution of a user provoked    */
/* program break. These are usually called from interrupt routines.       */

/*------------------------------------------------------------------------*/
/* Control C ISR support                                                  */
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)

#if CYGHWR_HAL_MN10300_AM33_STB_GDB_PORT == 0

// We use serial0 on AM33
#define SERIAL_CR       ((volatile cyg_uint16 *)0xd4002000)
#define SERIAL_ICR      ((volatile cyg_uint8 *) 0xd4002004)
#define SERIAL_TXR      ((volatile cyg_uint8 *) 0xd4002008)
#define SERIAL_RXR      ((volatile cyg_uint8 *) 0xd4002009)
#define SERIAL_SR       ((volatile cyg_uint16 *)0xd400200c)

// Timer 1 provided baud rate divisor
#define TIMER_MD       ((volatile cyg_uint8 *)0xd4003000)
#define TIMER_BR       ((volatile cyg_uint8 *)0xd4003010)
#define TIMER_CR       ((volatile cyg_uint8 *)0xd4003020)

#define SIO_LSTAT_TRDY  0x20
#define SIO_LSTAT_RRDY  0x10

#else

#error Unsupported GDB port

#endif

struct Hal_SavedRegisters *hal_saved_interrupt_state;

static void hal_ctrlc_isr_init(void)
{
//    cyg_uint16 cr;

//    HAL_READ_UINT16( SERIAL_CR, cr );
//    cr |= LCR_RXE;
//    HAL_WRITE_UINT16( SERIAL_CR, cr );
    HAL_INTERRUPT_SET_LEVEL( CYGHWR_HAL_GDB_PORT_VECTOR, 4 );
    HAL_INTERRUPT_UNMASK( CYGHWR_HAL_GDB_PORT_VECTOR ); 
}

cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    char c;
    cyg_uint16 sr;
    
    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR ); 

    HAL_READ_UINT16( SERIAL_SR, sr );

    if( sr & SIO_LSTAT_RRDY )
    {
        HAL_READ_UINT8( SERIAL_RXR, c);

        if( cyg_hal_is_break( &c , 1 ) )
            cyg_hal_user_break( (CYG_ADDRWORD *)hal_saved_interrupt_state );

        
    }
    return 1;
}

#endif

void hal_arch_funcall_new_stack(void (*func)(void), void* stack_base, cyg_uint32 stack_size)
{
    register cyg_uint32 stack_top = (cyg_uint32)stack_base + stack_size;
    register cyg_uint32 old_stack;
    asm volatile (" mov sp, %0" : "=r" (old_stack) : );
    asm volatile (" mov %0, sp" : : "r" (stack_top) );
    func();
    asm volatile (" mov %0, sp" : : "r" (old_stack) );
}

/*------------------------------------------------------------------------*/
/* Syscall support                                                        */
#ifdef CYGPKG_CYGMON
// Cygmon provides syscall handling for this board
#include <cyg/hal/hal_stub.h>
int __get_syscall_num (void)
{
    return SIGSYS;
}
#endif

/*------------------------------------------------------------------------*/
/* flash write-protect support                                            */
int plf_flash_query_soft_wp(void *addr, int len)
{
    if (((unsigned long)addr & 0xFC000000UL) == 0x84000000UL)
	return !(*(cyg_uint8*)0xA6FA0000 & 0x02); // system flash
    else
	return 0; // boot prom
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
