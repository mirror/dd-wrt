#ifndef CYGONCE_HAL_INTR_VECS_H
#define CYGONCE_HAL_INTR_VECS_H

//==========================================================================
//
//      hal_intr_vecs.h
//
//      HAL Interrupt support for variants with vectored style
//      exception/interrupt mapping (SH1/SH2)
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2002-01-15
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts.
//              
// Usage:        Is included from <cyg/hal/hal_intr.h>
//
//####DESCRIPTIONEND####
//
//==========================================================================

//--------------------------------------------------------------------------
// SH exception vectors. These correspond to VSRs and are the values
// to use for HAL_VSR_GET/SET
//
// Note that exceptions are decoded - there is a VSR slot for each exception
// source, while interrupts are handled via the same VSR.
//
// The SH2 vectors are set up to provide exception/interrupt behavior
// much like that found in the SH3/SH4 CPUs. See hal_var_sp.inc for
// implementation details.
//
// While there are 256 vectors, we only reserve space for 64 VSRs.
// With TRAPs and Interrupts wired to VSRs 32+33, there's still plenty
// of free VSRs for use by the application. 

#define CYGNUM_HAL_VECTOR_POWERON                0 // power-on
// RESERVED 1
#define CYGNUM_HAL_VECTOR_RESET                  2 // reset
// RESERVED 3
#define CYGNUM_HAL_VECTOR_ILLEGAL_INSTRUCTION    4 // reserved instruction
// RESERVED 5
#define CYGNUM_HAL_VECTOR_ILLEGAL_SLOT_INSTRUCTION  6
                                           // illegal instruction in delay slot
// RESERVED 7-8
#define CYGNUM_HAL_VECTOR_CPU_ADDRESS_ERROR      9 // CPU address error
#define CYGNUM_HAL_VECTOR_DMA_ADDRESS_ERROR     10 // DMA address error
//#define CYGNUM_HAL_VECTOR_NMI                 11 // This gets mapped as irq 0
#define CYGNUM_HAL_VECTOR_USER_BREAK            12 // user breakpoint
#define CYGNUM_HAL_VECTOR_H_UDI                 13 // H-UDI
// RESERVED 14-31
#define CYGNUM_HAL_VECTOR_TRAP                  32 // user breakpoint
#define CYGNUM_HAL_VECTOR_INTERRUPT             33 // all interrupts

#ifndef CYG_VECTOR_IS_INTERRUPT
# define CYG_VECTOR_IS_INTERRUPT(v) (CYGNUM_HAL_VECTOR_INTERRUPT == (v))
#endif

#define CYGNUM_HAL_VSR_MIN                   CYGNUM_HAL_VECTOR_POWERON
#ifndef CYGNUM_HAL_VSR_MAX
# define CYGNUM_HAL_VSR_MAX                  63
#endif
#define CYGNUM_HAL_VSR_COUNT                 ( CYGNUM_HAL_VSR_MAX + 1 )

#ifndef CYGNUM_HAL_VSR_EXCEPTION_COUNT
# define CYGNUM_HAL_VSR_EXCEPTION_COUNT       (CYGNUM_HAL_VECTOR_TRAP-CYGNUM_HAL_VECTOR_POWERON+1)
#endif

// The decoded interrupts.
#define CYGNUM_HAL_INTERRUPT_NMI             0
// These are equivalent to HW_EXC vectors 64 and up
#define CYGNUM_HAL_INTERRUPT_HW_EXC_BASE     64
#define CYGNUM_HAL_INTERRUPT_LVL0            1 // note that LVLx and IRQx share vectors!
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ0        1 
#define CYGNUM_HAL_INTERRUPT_LVL1            2 
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ1        2 
#define CYGNUM_HAL_INTERRUPT_LVL2            3 
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ2        3
#define CYGNUM_HAL_INTERRUPT_LVL3            4 
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ3        4
#define CYGNUM_HAL_INTERRUPT_LVL4            5 
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ4        5
#define CYGNUM_HAL_INTERRUPT_LVL5            6 
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ5        6
#define CYGNUM_HAL_INTERRUPT_LVL6            7 
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ6        7
#define CYGNUM_HAL_INTERRUPT_LVL7            8 
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ7        8

#ifndef CYGNUM_HAL_INTERRUPT_LVL_MAX
# define CYGNUM_HAL_INTERRUPT_LVL_MAX         CYGNUM_HAL_INTERRUPT_LVL7
#endif

#ifndef CYGNUM_HAL_ISR_MAX
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_INTERRUPT_IRQ_IRQ7
#endif

#define CYGNUM_HAL_ISR_MIN                   CYGNUM_HAL_INTERRUPT_NMI
#define CYGNUM_HAL_ISR_COUNT                 ( CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1 )

//--------------------------------------------------------------------------
// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

// The exception indexes are given by the HW_EXC vectors. Variants may define additional
// exception vectors.

#define CYGNUM_HAL_EXCEPTION_POWERON                0 // power-on
#define CYGNUM_HAL_EXCEPTION_RESET                  2 // reset
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION    4 // illegal instruction
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION 6 // illegal instruction in the slot
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS            9 // CPU address error
#define CYGNUM_HAL_EXCEPTION_DMA_DATA_ACCESS       10 // DMA address error
#define CYGNUM_HAL_EXCEPTION_USER_BREAK            12 // user break
#define CYGNUM_HAL_EXCEPTION_H_UDI                 13 // H-UDI
#define CYGNUM_HAL_EXCEPTION_TRAP                  32 // unconditional trap

#define CYGNUM_HAL_EXCEPTION_MIN          CYGNUM_HAL_EXCEPTION_POWERON

#ifndef CYGNUM_HAL_EXCEPTION_MAX
# define CYGNUM_HAL_EXCEPTION_MAX         CYGNUM_HAL_EXCEPTION_TRAP
#endif

#define CYGNUM_HAL_EXCEPTION_COUNT           \
                 ( CYGNUM_HAL_EXCEPTION_MAX - CYGNUM_HAL_EXCEPTION_MIN + 1 )

// For the stub exception handling
#define _CYGNUM_HAL_VECTOR_FIRST_MEM_FAULT       CYGNUM_HAL_EXCEPTION_DATA_ACCESS
#define _CYGNUM_HAL_VECTOR_LAST_MEM_FAULT        CYGNUM_HAL_EXCEPTION_DMA_DATA_ACCESS

#ifndef __ASSEMBLER__

#include <cyg/infra/cyg_type.h>

externC void hal_delay_us(int);
#define HAL_DELAY_US(n) hal_delay_us(n)

#endif // __ASSEMBLER__

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_INTR_VECS_H
// End of hal_intr_vecs.h
