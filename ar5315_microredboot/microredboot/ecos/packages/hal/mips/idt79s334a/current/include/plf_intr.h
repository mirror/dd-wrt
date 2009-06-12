#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      RefIDT 79S344A Interrupt and clock support
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
// Author(s):    tmichals
// Contributors: 
//              
// Date:         2002-09-20
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the IDT79S334A board.
//              
// Usage:
//              #include <cyg/hal/plf_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/plf_io.h>

#define HAL_PLATFORM_RESET()            CYG_EMPTY_STATEMENT

#define HAL_PLATFORM_RESET_ENTRY        0xbfc00000


//--------------------------------------------------------------------------
// Interrupt vectors.

#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED
// the default for all MIPS variants is to use the 6 bits
// in the cause register.

// These are only HW interrupts
#define CYGNUM_HAL_INTERRUPT_0                0
#define CYGNUM_HAL_INTERRUPT_1                1
#define CYGNUM_HAL_INTERRUPT_2                2
#define CYGNUM_HAL_INTERRUPT_3                3
#define CYGNUM_HAL_INTERRUPT_4                4
#define CYGNUM_HAL_INTERRUPT_5                5

// Min/Max ISR numbers and how many there are
#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     30
#define CYGNUM_HAL_ISR_COUNT                   31

// The vector used by the Real time clock. The default here is to use
// interrupt 5, which is connected to the counter/comparator registers
// in many MIPS variants.

#define CYGNUM_HAL_PCI_A      CYGNUM_HAL_INTERRUPT_1
#define CYGNUM_HAL_PCI_B      CYGNUM_HAL_INTERRUPT_2
#define CYGNUM_HAL_PCI_C      CYGNUM_HAL_INTERRUPT_4
#define CYGNUM_EXPANSION	  CYGNUM_HAL_INTERRUPT_3


#ifndef CYGNUM_HAL_INTERRUPT_RTC
#define CYGNUM_HAL_INTERRUPT_RTC            CYGNUM_HAL_INTERRUPT_5
#endif

/* please NOTE that slot D is taken for RTC */
#define CYGNUM_LAST_IDT_INTERRUPT	20

#define CYGNUM_HAL_INTERRUPT_SIO_0 25
#define CYGNUM_HAL_INTERRUPT_SIO_1 26

#define CYGNUM_HAL_PCI_A      CYGNUM_HAL_INTERRUPT_1
#define CYGNUM_HAL_PCI_B      CYGNUM_HAL_INTERRUPT_2
#define CYGNUM_HAL_PCI_C      CYGNUM_HAL_INTERRUPT_4
#define CYGNUM_EXPANSION	  CYGNUM_HAL_INTERRUPT_3


#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#endif



//--------------------------------------------------------------------------
// Interrupt controller access
// The default code here simply uses the fields present in the CP0 status
// and cause registers to implement this functionality.
// Beware of nops in this code. They fill delay slots and avoid CP0 hazards
// that might otherwise cause following code to run in the wrong state or
// cause a resource conflict.

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#define HAL_INTERRUPT_MASK_CPU( _vector_ )          \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "nor    $2,$2,$0\n"                     \
        "and    $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK_CPU( _vector_ )        \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "or     $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE_CPU( _vector_ )   \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$13\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "nor    $2,$2,$0\n"                     \
        "and    $3,$3,$2\n"                     \
        "mtc0   $3,$13\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )


externC void cyg_hal_interrupt_mask(cyg_uint32 vector);
externC void cyg_hal_interrupt_unmask(cyg_uint32 vector);
externC void cyg_hal_interrupt_acknowledge(cyg_uint32 vector);

#define HAL_INTERRUPT_MASK( _vector_ )          \
CYG_MACRO_START                                 \
	cyg_hal_interrupt_mask ( (_vector_) );		\
CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )                  \
    CYG_MACRO_START                                       \
        cyg_hal_interrupt_unmask ( (_vector_) );          \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )             \
    CYG_MACRO_START                                       \
        cyg_hal_interrupt_acknowledge ( (_vector_) );     \
    CYG_MACRO_END

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#endif


#endif /* ifndef CYGONCE_HAL_PLF_INTR_H */
//--------------------------------------------------------------------------
// End of plf_intr.h
