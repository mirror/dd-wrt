#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-02-12
// Purpose:      CalmRISC16 platform IO support
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_misc.h>

// coprocessor regs
#define HAL_COP_SR_RBR 0x1c
#define HAL_COP_SR_TBR 0x1d
#define HAL_COP_TBR    0x1e
#define HAL_COP_RBR    0x1f

#ifndef __ASSEMBLER__

static inline void cyg_hal_plf_write_sr_rbr(int val)
{
    asm volatile ("cld %0,%1\n" : : "i"(HAL_COP_SR_RBR), "r"(val));
}

static inline void cyg_hal_plf_write_sr_tbr(int val)
{
    asm volatile ("cld %0,%1\n" : : "i"(HAL_COP_SR_TBR), "r"(val));
}

static inline void cyg_hal_plf_write_rbr(int val)
{
    asm volatile ("cld %0,%1\n" : : "i"(HAL_COP_RBR), "r"(val));
}

static inline void cyg_hal_plf_write_tbr(int val)
{
    asm volatile ("cld %0,%1\n" : : "i"(HAL_COP_TBR), "r"(val));
}

static inline int cyg_hal_plf_read_sr_rbr(void)
{
    int val;
    asm volatile ("cld %0,%1\n" : "=r"(val) : "i"(HAL_COP_SR_RBR));
    return val;
}

static inline int cyg_hal_plf_read_sr_tbr(void)
{
    int val;
    asm volatile ("cld %0,%1\n" : "=r"(val) : "i"(HAL_COP_SR_TBR));
    return val;
}

static inline int cyg_hal_plf_read_rbr(void)
{
    int val;
    asm volatile ("cld %0,%1\n" : "=r"(val) : "i"(HAL_COP_RBR));
    return val;
}

static inline int cyg_hal_plf_read_tbr(void)
{
    int val;
    asm volatile ("cld %0,%1\n" : "=r"(val) : "i"(HAL_COP_TBR));
    return val;
}
#endif

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
