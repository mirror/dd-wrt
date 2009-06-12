//=============================================================================
//
//      mod_regs_wdt.h
//
//      WDT (Watch Dog Timer) Module register definitions
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
// Date:        2002-01-17
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
#define CYGARC_REG_WTCSR_W              0xfffffe80 // 16 bit
#define CYGARC_REG_WTCSR_R              0xfffffe80 // 8 bit
#define CYGARC_REG_WTCNT_W              0xfffffe80 // 16 bit
#define CYGARC_REG_WTCNT_R              0xfffffe81 // 8 bit
#define CYGARC_REG_RSTCSR_W             0xfffffe82 // 16 bit
#define CYGARC_REG_RSTCSR_R             0xfffffe83 // 8 bit

#define CYGARC_REG_WTCSR_W_MAGIC        0xa518
#define CYGARC_REG_WTCNT_W_MAGIC        0x5a00
#define CYGARC_REG_RSTCSR_W_WOVF_MAGIC  0xa51f
#define CYGARC_REG_RSTCSR_W_RSTx_MAGIC  0x5a1f

#define CYGARC_REG_WTCSR_INIT   (CYGARC_REG_WTCSR_W_MAGIC|0x00)

#define CYGARC_REG_WTCSR_OVF             0x0080
#define CYGARC_REG_WTCSR_WTIT            0x0040
#define CYGARC_REG_WTCSR_TME             0x0020


#define CYGARC_REG_RSTCSR_RSTE           0x0040
#define CYGARC_REG_RSTCSR_RSTS           0x0020
