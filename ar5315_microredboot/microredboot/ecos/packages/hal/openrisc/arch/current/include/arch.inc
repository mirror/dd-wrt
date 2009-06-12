##=============================================================================
##
##      arch.inc
##
##      OpenRISC assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   sfurman
## Contributors:
## Date:        2003-01-15
## Purpose:     Architecture definitions.
## Description: This file contains various definitions and macros that are
##              useful for writing assembly code for the OpenRISC CPU family.
## Usage:
##              #include <cyg/hal/arch.inc>
##              ...
##              
##
######DESCRIPTIONEND####
##
##=============================================================================

#ifndef CYGONCE_HAL_ARCH_INC
#define CYGONCE_HAL_ARCH_INC

# Declare given label name as the start of a function accessible from C code
#define FUNC_START(name)        \
        .type _##name,@function;   \
        .globl _##name;            \
_##name:   

#define FUNC_END(name)

# Make aliases for ABI distinguished registers
#define sp r1
#define fp r2
#define lr r9

# Size of GPR regs - 4 bytes for or32
#define OR1K_GPRSIZE    4

# Size of all other registers
#define OR1K_REGSIZE    4

# Utility macro: Load a 32-bit constant into a register
        .macro  load32i reg const
        l.movhi \reg,hi(\const)
        l.ori   \reg,\reg,lo(\const)
        .endm

##-----------------------------------------------------------------------------

## OpenRISC thread and interrupt saved state structure. These offsets
## must match the layout of the HAL_SavedRegisters struct in
## hal_arch.h. Do not change this without changing the layout there,
## or viceversa.

#define OR1KREGS_GPRS   0
#define OR1KREG_MACHI   (OR1KREGS_GPRS + OR1K_GPRSIZE * 32)
#define OR1KREG_MACLO   (OR1KREG_MACHI + OR1K_REGSIZE)
#define OR1KREG_VECTOR  (OR1KREG_MACLO + OR1K_REGSIZE)
#define OR1KREG_SR      (OR1KREG_VECTOR+ OR1K_REGSIZE)
#define OR1KREG_PC      (OR1KREG_SR    + OR1K_REGSIZE)
#define OR1KREG_EEAR    (OR1KREG_PC    + OR1K_REGSIZE)

#define SIZEOF_OR1KREGS (OR1KREG_EEAR  + OR1K_REGSIZE)

#include <cyg/hal/openrisc.inc>
#include <cyg/hal/platform.inc>

#endif // #ifndef CYGONCE_HAL_ARCH_INC

// EOF arch.inc
