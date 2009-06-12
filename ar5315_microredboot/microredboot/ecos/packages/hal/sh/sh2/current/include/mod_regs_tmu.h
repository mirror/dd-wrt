//=============================================================================
//
//      mod_regs_tmu.h
//
//      TMU (timer unit) Module register definitions
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2000-10-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// TMU registers
#define CYGARC_REG_TOCR                 0xfffffe90 //  8 bit
#define CYGARC_REG_TSTR                 0xfffffe92 //  8 bit
#define CYGARC_REG_TCOR0                0xfffffe94 // 32 bit
#define CYGARC_REG_TCNT0                0xfffffe98 // 32 bit
#define CYGARC_REG_TCR0                 0xfffffe9c // 16 bit
#define CYGARC_REG_TCOR1                0xfffffea0 // 32 bit
#define CYGARC_REG_TCNT1                0xfffffea4 // 32 bit
#define CYGARC_REG_TCR1                 0xfffffea8 // 16 bit
#define CYGARC_REG_TCOR2                0xfffffeac // 32 bit
#define CYGARC_REG_TCNT2                0xfffffeb0 // 32 bit
#define CYGARC_REG_TCR2                 0xfffffeb4 // 16 bit
#define CYGARC_REG_TCPR2                0xfffffeb8 // 32 bit

// TSTR
#define CYGARC_REG_TSTR_STR0            0x0001
#define CYGARC_REG_TSTR_STR1            0x0002
#define CYGARC_REG_TSTR_STR2            0x0004

// TCR0/1/2
#define CYGARC_REG_TCR_TPSC0            0x0001
#define CYGARC_REG_TCR_TPSC1            0x0002
#define CYGARC_REG_TCR_TPSC2            0x0004
#define CYGARC_REG_TCR_CKEG0            0x0008
#define CYGARC_REG_TCR_CKEG1            0x0010
#define CYGARC_REG_TCR_UNIE             0x0020
#define CYGARC_REG_TCR_UNF              0x0100

#define CYGARC_REG_TCR_TPSC_4           (0)
#define CYGARC_REG_TCR_TPSC_16          (CYGARC_REG_TCR_TPSC0)
#define CYGARC_REG_TCR_TPSC_64          (CYGARC_REG_TCR_TPSC1)
#define CYGARC_REG_TCR_TPSC_256         (CYGARC_REG_TCR_TPSC0|CYGARC_REG_TCR_TPSC1)

// TCR2 additional bits
#define CYGARC_REG_TCR_ICPE0            0x0040
#define CYGARC_REG_TCR_ICPE1            0x0080
#define CYGARC_REG_TCR_ICPF             0x0200
