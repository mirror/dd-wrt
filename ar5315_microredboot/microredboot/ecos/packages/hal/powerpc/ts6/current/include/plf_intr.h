#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      MPC8260 platform specific interrupt definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    pfine
// Contributors: jskov
// Date:         2002-02-27
// Purpose:      Define platform specific interrupt support
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


//----------------------------------------------------------------------------
// Reset.
/* This reset sequence will generate a checkstop reset
 * It should probably live in mpc8260 variant directory, or be copied
 * to the VADS directory.
 */
#define HAL_PLATFORM_RESET()       \
    CYG_MACRO_START                         \
        cyg_uint32 tmp0, tmp1; \
        cyg_uint32 *tmp2 = (cyg_uint32 *) 0x04710c94; \
        *tmp2 = 0x1;                   /* set RMR[CSRE] bit */ \
        asm volatile("lis %0, 0xB001;" /* tmp0 = 0xB0010000 */\
                     "mtspr %2, %0;"   /* HID0 = 0xB0010000, set HID0[EMCP] */\
                     "isync;"          /* paranoia */\
                     "li %1, 0x0;"     /* tmp0 = 0x0        */\
                     "mfmsr %0;"       /* tmp0 = MSR */\
                     "rlwimi %0,%1,0,19,19;"   /* */\
                     "rlwimi %0,%1,0,26,27;"   /* */\
                     "mtmsr %0;"       /* clear MSR[EE][IR][DR] */\
                     "isync;"          /* probably required here */\
                     "lis %1, 0xF000;" /* tmp1 = 0xF0000000 */\
                     "eieio;"          /* paranoia */\
                     "lwz %0,0(%1);"   /* Attempt to access illegal memory. */\
                     : "=r" (tmp0), "=r" (tmp1)\
                     : "I" (CYGARC_REG_HID0));\
    CYG_MACRO_END

// FIXME - What about the LOWROM configuarion ?
#ifdef DCSPRI_HAL_TS6_ROM_MLT_RAM
#define HAL_PLATFORM_RESET_ENTRY 0x00000100
#else
#define HAL_PLATFORM_RESET_ENTRY 0xFFF00100
#endif

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h


