//=============================================================================
//
//      mod_regs_cmt.h
//
//      CMT (Compare Match Timer) Module register definitions
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
// Date:        2002-04-17
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// Compare Match Timer

#define CYGARC_REG_CMSTR              0xffff83d0
#define CYGARC_REG_CMCSR0             0xffff83d2
#define CYGARC_REG_CMCNT0             0xffff83d4
#define CYGARC_REG_CMCOR0             0xffff83d6
#define CYGARC_REG_CMCSR1             0xffff83d8
#define CYGARC_REG_CMCNT1             0xffff83da
#define CYGARC_REG_CMCOR1             0xffff83dc


#define CYGARC_REG_CMSTR_STR1         0x0002 // start compare time 1
#define CYGARC_REG_CMSTR_STR0         0x0001 // start compare time 0

#define CYGARC_REG_CMCSR_CMF          0x0080 // compare match flag
#define CYGARC_REG_CMCSR_CMIE         0x0040 // compare match interrupt enable
#define CYGARC_REG_CMCSR_CLK_8        0x0000 // system clock / 8
#define CYGARC_REG_CMCSR_CLK_32       0x0001 // system clock / 32
#define CYGARC_REG_CMCSR_CLK_128      0x0002 // system clock / 128
#define CYGARC_REG_CMCSR_CLK_512      0x0003 // system clock / 512
