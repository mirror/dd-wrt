#ifndef CYGONCE_HAL_VAR_INTR_H
#define CYGONCE_HAL_VAR_INTR_H
//==========================================================================
//
//      var_intr.h
//
//      VR4300 Interrupt and clock support
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
// Author(s):    hmt, nickg
// Contributors: nickg, jskov,
//               gthomas, jlarmour
// Date:         2001-05-24
// Purpose:      uPD985xx Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for variants of the NEC uPD985xx
//               architecture.
//              
// Usage:
//              #include <cyg/hal/var_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/var_arch.h>
#include <cyg/hal/plf_intr.h>

//--------------------------------------------------------------------------
// Interrupt controller stuff.

#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED
// Interrupts dealt with via the status and cause registers
// must be numbered in bit order:
#define CYGNUM_HAL_INTERRUPT_STATUS_CAUSE_LOW  (0)
// The first two are the "software interrupts" - you just set a bit.
#define CYGNUM_HAL_INTERRUPT_SOFT_ZERO         (0)
#define CYGNUM_HAL_INTERRUPT_SOFT_ONE          (1)
#define CYGNUM_HAL_INTERRUPT_FREE_TWO          (2)
#define CYGNUM_HAL_INTERRUPT_USB               (3)
#define CYGNUM_HAL_INTERRUPT_ETHER             (4)
#define CYGNUM_HAL_INTERRUPT_FREE_FIVE         (5)
#define CYGNUM_HAL_INTERRUPT_SYSCTL            (6)
#define CYGNUM_HAL_INTERRUPT_COMPARE           (7)

// Number 6 "SYSCTL" is all external sources in the system controller and
// will normally be decoded into one of 8-12 instead.  If you use number 6
// directly, then this will disable *all* system controller sources.
// Startup code will ensure number 6 is unmasked by default, and it will
// have an arbitration routine installed to call all of the subsequent
// interrupts from the S_ISR register.  This has to be an external routine
// because the S_ISR register is read-clear, and the interrupt sources are
// edge-triggered so they do not re-assert themselves - so we must address
// multiple sources per actual interrupt, in a loop.

#define CYGNUM_HAL_INTERRUPT_SYSCTL_LOW        (8)
#define CYGNUM_HAL_INTERRUPT_SYSCTL_HI        (12)

#define CYGNUM_HAL_INTERRUPT_TM0  	       (8) // TIMER CH0 interrupt.
#define CYGNUM_HAL_INTERRUPT_TM1  	       (9) // TIMER CH1 interrupt.
#define CYGNUM_HAL_INTERRUPT_UART 	      (10) // UART interrupt.
#define CYGNUM_HAL_INTERRUPT_EXT  	      (11) // External Interrupt.
#define CYGNUM_HAL_INTERRUPT_WU   	      (12) // Wakeup Interrupt.

#define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_UART

// Min/Max ISR numbers and how many there are
#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     12
#define CYGNUM_HAL_ISR_COUNT                   13

// The vector used by the Real time clock. The default here is to use
// interrupt 5, which is connected to the counter/comparator registers
// in many MIPS variants.

#ifndef CYGNUM_HAL_INTERRUPT_RTC
#define CYGNUM_HAL_INTERRUPT_RTC            CYGNUM_HAL_INTERRUPT_COMPARE
#endif

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#endif // CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#ifndef __ASSEMBLER__

// ------------------------------------------------------------------------

// This is placed in memory at a fixed location because we must share it
// with RedBoot, along with the VSR table and Virtual Vector table.
// It has to be an array to get the correct code generation to access it
// over all that distance.
externC volatile cyg_uint32 hal_interrupt_sr_mask_shadow_base[];
#define hal_interrupt_sr_mask_shadow (hal_interrupt_sr_mask_shadow_base[0])

// We have to have local versions of these to preserve the mask bits in the
// SR correctly when an interrupt occurs within one of these code sequences
// which are doing a read-modify-write to the main interrupt bit of the SR.

// Disable, it doesn't matter what the SR IM bits are - but it is possible
// for control to return with interrupts enabled if a context switch occurs
// away from the thread that disabled interrupts.  Therefore we also make
// sure the contents of the SR match the shadow variable at the end.

#define HAL_DISABLE_INTERRUPTS(_old_)                                   \
CYG_MACRO_START                                                         \
    register int _tmp;                                                  \
    asm volatile (                                                      \
        "mfc0   $8,$12; nop;"                                           \
        "move   %0,$8;"                                                 \
        "and    $8,$8,0xfffffffe;"                                      \
        "mtc0   $8,$12;"                                                \
        "nop; nop; nop;"                                                \
        : "=r"(_tmp)                                                    \
        :                                                               \
        : "$8"                                                          \
        );                                                              \
    /* interrupts disabled so can now inject the correct IM bits */     \
    (_old_) = _tmp & 1;                                                 \
    _tmp &= 0xffff00fe;                                                 \
    _tmp |= (hal_interrupt_sr_mask_shadow & 0xff00);                    \
    asm volatile (                                                      \
        "mtc0   %0,$12;"                                                \
        "nop; nop; nop;"                                                \
        :                                                               \
        : "r"(_tmp)                                                     \
        );                                                              \
CYG_MACRO_END

// Enable and restore, we must pick up hal_interrupt_sr_mask_shadow because
// it contains the truth.  This is also for the convenience of the
// mask/unmask macros below.
#define HAL_ENABLE_INTERRUPTS() HAL_RESTORE_INTERRUPTS(1)

#define HAL_RESTORE_INTERRUPTS(_old_)                                   \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mfc0   $8,$12; nop;"                                           \
        "or     $8,$8,%0;"         /* inject IE bit  */                 \
        "and    $8,$8,0xffff00ff;" /* clear IM bits  */                 \
        "or     $8,$8,%1;"         /* insert true IM */                 \
        "mtc0   $8,$12;"                                                \
        "nop; nop; nop;"                                                \
        :                                                               \
        : "r"((_old_) & 1),"r"(hal_interrupt_sr_mask_shadow & 0xff00)   \
        : "$8"                                                          \
        );                                                              \
CYG_MACRO_END

#define HAL_QUERY_INTERRUPTS( _state_ )         \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   %0,$12; nop;"                   \
        "and    %0,%0,0x1;"                     \
        : "=r"(_state_)                         \
        );                                      \
CYG_MACRO_END

#define CYGHWR_HAL_INTERRUPT_ENABLE_DISABLE_RESTORE_DEFINED

// ------------------------------------------------------------------------

// For the bits which are in the SR, we only need to diddle the shadow
// variable; restore interrupts will pick that up at the end of the macro.
// Neat, huh.

#ifndef CYGOPT_HAL_MIPS_UPD985XX_HARDWARE_BUGS_S2
// Vanilla versions here: trick versions with the workaround follow:

#define HAL_INTERRUPT_MASK( _vector_ )                                  \
CYG_MACRO_START                                                         \
register int _intstate;                                                 \
register int _shift;                                                    \
HAL_DISABLE_INTERRUPTS( _intstate );                                    \
if ( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW > (_vector_) ) {                   \
    /* mask starts at bit 8 */                                          \
    _shift = 8 + (_vector_) - CYGNUM_HAL_INTERRUPT_STATUS_CAUSE_LOW;    \
    hal_interrupt_sr_mask_shadow &=~(1 << _shift);                      \
}                                                                       \
else {                                                                  \
    _shift = (_vector_) - CYGNUM_HAL_INTERRUPT_SYSCTL_LOW;              \
    *S_IMR &=~(1 << _shift);                                            \
}                                                                       \
HAL_RESTORE_INTERRUPTS( _intstate );                                    \
CYG_MACRO_END


#define HAL_INTERRUPT_UNMASK( _vector_ )                                \
CYG_MACRO_START                                                         \
register int _intstate;                                                 \
register int _shift;                                                    \
HAL_DISABLE_INTERRUPTS( _intstate );                                    \
if ( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW > (_vector_) ) {                   \
    /* mask starts at bit 8 */                                          \
    _shift = 8 + (_vector_) - CYGNUM_HAL_INTERRUPT_STATUS_CAUSE_LOW;    \
    hal_interrupt_sr_mask_shadow |= (1 << _shift);                      \
}                                                                       \
else {                                                                  \
    _shift = (_vector_) - CYGNUM_HAL_INTERRUPT_SYSCTL_LOW;              \
    *S_IMR |= (1 << _shift);                                            \
}                                                                       \
HAL_RESTORE_INTERRUPTS( _intstate );                                    \
CYG_MACRO_END


#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                           \
CYG_MACRO_START                                                         \
register int _intstate;                                                          \
HAL_DISABLE_INTERRUPTS( _intstate );                                    \
/* Default clears the bit in the cause register.  But VR4120 doc   */   \
/* says this is a NOP so we ignore low numbered sources except the */   \
/* software interrupt bits.                                        */   \
if ( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW <= (_vector_) ||                   \
     CYGNUM_HAL_INTERRUPT_SYSCTL     == (_vector_) ) {                  \
    register int i;                                                     \
    i = *S_ISR; /* This is read-clear! */                               \
}                                                                       \
else if ( CYGNUM_HAL_INTERRUPT_SOFT_ZERO ==  (_vector_) ||              \
          CYGNUM_HAL_INTERRUPT_SOFT_ONE  ==  (_vector_) ) {             \
    /* These two are acknowledged by writing the bit to zero in */      \
    /* the cause register.  NB not the status register!         */      \
    asm volatile (                                                      \
        "mfc0   $3,$13\n"                                               \
        "la     $2,0x00000100\n"                                        \
        "sllv   $2,$2,%0\n"                                             \
        "andi   $2,$2,0x0300\n"                                         \
        "nor    $2,$2,$0\n"                                             \
        "and    $3,$3,$2\n"                                             \
        "mtc0   $3,$13\n"                                               \
        "nop; nop; nop\n"                                               \
        :                                                               \
        : "r"((_vector_)-CYGNUM_HAL_INTERRUPT_STATUS_CAUSE_LOW)         \
        : "$2", "$3"                                                    \
        );                                                              \
}                                                                       \
HAL_RESTORE_INTERRUPTS( _intstate );                                    \
CYG_MACRO_END

#else // DEFINED:  CYGOPT_HAL_MIPS_UPD985XX_HARDWARE_BUGS_S2

#ifdef __cplusplus
extern "C" {
#endif
extern void cyg_hal_interrupt_unmask( int vec );
extern void cyg_hal_interrupt_mask( int vec );
extern void cyg_hal_interrupt_acknowledge( int vec );
#ifdef __cplusplus
}      /* extern "C" */
#endif

#define HAL_INTERRUPT_MASK( _vector_ )                                  \
CYG_MACRO_START                                                         \
register int _intstate;                                                 \
register int _shift;                                                    \
HAL_DISABLE_INTERRUPTS( _intstate );                                    \
if ( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW > (_vector_) ) {                   \
    /* mask starts at bit 8 */                                          \
    _shift = 8 + (_vector_) - CYGNUM_HAL_INTERRUPT_STATUS_CAUSE_LOW;    \
    hal_interrupt_sr_mask_shadow &=~(1 << _shift);                      \
}                                                                       \
else {                                                                  \
    cyg_hal_interrupt_mask( (_vector_) );                               \
}                                                                       \
HAL_RESTORE_INTERRUPTS( _intstate );                                    \
CYG_MACRO_END


#define HAL_INTERRUPT_UNMASK( _vector_ )                                \
CYG_MACRO_START                                                         \
register int _intstate;                                                 \
register int _shift;                                                    \
HAL_DISABLE_INTERRUPTS( _intstate );                                    \
if ( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW > (_vector_) ) {                   \
    /* mask starts at bit 8 */                                          \
    _shift = 8 + (_vector_) - CYGNUM_HAL_INTERRUPT_STATUS_CAUSE_LOW;    \
    hal_interrupt_sr_mask_shadow |= (1 << _shift);                      \
}                                                                       \
else {                                                                  \
    cyg_hal_interrupt_unmask( (_vector_) );                             \
}                                                                       \
HAL_RESTORE_INTERRUPTS( _intstate );                                    \
CYG_MACRO_END


#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                           \
CYG_MACRO_START                                                         \
register int _intstate;                                                          \
HAL_DISABLE_INTERRUPTS( _intstate );                                    \
/* Default clears the bit in the cause register.  But VR4120 doc   */   \
/* says this is a NOP so we ignore low numbered sources except the */   \
/* software interrupt bits.                                        */   \
if ( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW <= (_vector_) ||                   \
     CYGNUM_HAL_INTERRUPT_SYSCTL     == (_vector_) ) {                  \
    cyg_hal_interrupt_acknowledge( (_vector_) );                        \
}                                                                       \
else if ( CYGNUM_HAL_INTERRUPT_SOFT_ZERO ==  (_vector_) ||              \
          CYGNUM_HAL_INTERRUPT_SOFT_ONE  ==  (_vector_) ) {             \
    /* These two are acknowledged by writing the bit to zero in */      \
    /* the cause register.  NB not the status register!         */      \
    asm volatile (                                                      \
        "mfc0   $3,$13\n"                                               \
        "la     $2,0x00000100\n"                                        \
        "sllv   $2,$2,%0\n"                                             \
        "andi   $2,$2,0x0300\n"                                         \
        "nor    $2,$2,$0\n"                                             \
        "and    $3,$3,$2\n"                                             \
        "mtc0   $3,$13\n"                                               \
        "nop; nop; nop\n"                                               \
        :                                                               \
        : "r"((_vector_)-CYGNUM_HAL_INTERRUPT_STATUS_CAUSE_LOW)         \
        : "$2", "$3"                                                    \
        );                                                              \
}                                                                       \
HAL_RESTORE_INTERRUPTS( _intstate );                                    \
CYG_MACRO_END

#endif // CYGOPT_HAL_MIPS_UPD985XX_HARDWARE_BUGS_S2

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

//--------------------------------------------------------------------------
// Useful for debugging...

#define HAL_READ_INTR_REGS( _status, _cause )   \
{                                               \
    asm volatile (                              \
        "mfc0   %0,$12; nop;"                   \
        : "=r"(_status)                         \
        );                                      \
    asm volatile (                              \
        "mfc0   %0,$13; nop;"                   \
        : "=r"(_cause)                          \
        );                                      \
}    

//--------------------------------------------------------------------------
#endif // ! __ASSEMBLER__

#endif // ifndef CYGONCE_HAL_VAR_INTR_H
// End of var_intr.h
