#ifndef CYGONCE_PLF_INTS_H
#define CYGONCE_PLF_INTS_H
//==========================================================================
//
//      plf_ints.h
//
//      HAL extended support for platform specific interrupts
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2001-02-24
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the MB93091 (FRV400) are defined here.
// Usage:
//               #include <cyg/hal/plf_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Define here extended support for this particular platform

// Interrupts are mapped onto "channels" by the interrupt controller
// The channel in turn is presented to the CPU as a "level" which can
// be masked.  The eCos interrupt names below are for the default mapping
// of interrupt sources to channels.

#define CYGNUM_HAL_INTERRUPT_TIMER0 CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_14
#define CYGNUM_HAL_INTERRUPT_TIMER1 CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_13
#define CYGNUM_HAL_INTERRUPT_TIMER2 CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_12
#define CYGNUM_HAL_INTERRUPT_DMA0   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_11
#define CYGNUM_HAL_INTERRUPT_DMA1   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_10
#define CYGNUM_HAL_INTERRUPT_DMA2   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_9
#define CYGNUM_HAL_INTERRUPT_DMA3   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_8
#define CYGNUM_HAL_INTERRUPT_UART0  CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_2
#define CYGNUM_HAL_INTERRUPT_UART1  CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1
#define CYGNUM_HAL_INTERRUPT_EXT0   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_7
#define CYGNUM_HAL_INTERRUPT_EXT1   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_6
#define CYGNUM_HAL_INTERRUPT_EXT2   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_5
#define CYGNUM_HAL_INTERRUPT_EXT3   CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_4

#define CYGNUM_HAL_INTERRUPT_RTC     CYGNUM_HAL_INTERRUPT_TIMER1
#define CYGNUM_HAL_INTERRUPT_SERIALA CYGNUM_HAL_INTERRUPT_UART0
#define CYGNUM_HAL_INTERRUPT_SERIALB CYGNUM_HAL_INTERRUPT_UART1
#define CYGNUM_HAL_INTERRUPT_LAN     CYGNUM_HAL_INTERRUPT_EXT2
#define CYGNUM_HAL_INTERRUPT_PCIINTA CYGNUM_HAL_INTERRUPT_EXT1
#define CYGNUM_HAL_INTERRUPT_PCIINTB CYGNUM_HAL_INTERRUPT_EXT0
#define CYGNUM_HAL_INTERRUPT_PCIINTC CYGNUM_HAL_INTERRUPT_EXT1
#define CYGNUM_HAL_INTERRUPT_PCIINTD CYGNUM_HAL_INTERRUPT_EXT0


//----------------------------------------------------------------------------
// Reset.
#define HAL_PLATFORM_RESET()                                               \
    CYG_MACRO_START                                                        \
    cyg_uint32 ctrl;                                                       \
                                                                           \
    /* By disabling interupts we will just hang in the loop below      */  \
    /* if for some reason the software reset fails.                    */  \
    HAL_DISABLE_INTERRUPTS(ctrl);                                          \
                                                                           \
    /* Software reset. */                                                  \
    HAL_WRITE_UINT32(_FRV400_HW_RESET, _FRV400_HW_RESET_HR);               \
                                                                           \
    for(;;); /* hang here forever if reset fails */                        \
    CYG_MACRO_END

#define HAL_PLATFORM_RESET_ENTRY 0xFF000000

#endif // CYGONCE_PLF_INTS_H
