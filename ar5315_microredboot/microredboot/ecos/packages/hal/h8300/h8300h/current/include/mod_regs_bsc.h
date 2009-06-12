#ifndef CYGONCE_MOD_REGS_BSC_H
#define CYGONCE_MOD_REGS_BSC_H

//==========================================================================
//
//      mod_regs_bsc.h
//
//      Bus Controler Register
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
// Author(s):    yoshinori sato
// Contributors: yoshinori sato
// Date:         2002-02-19
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGARC_BRCR   0xFEE013
#define CYGARC_ADRCR  0xFEE01E
#define CYGARC_CSCR   0xFEE01F
#define CYGARC_ABWCR  0xFEE020
#define CYGARC_ASTCR  0xFEE021
#define CYGARC_WCRH   0xFEE022
#define CYGARC_WCRL   0xFEE023
#define CYGARC_BCR    0xFEE024
#define CYGARC_DRCRA  0xFEE026
#define CYGARC_DRCRB  0xFEE027
#define CYGARC_RTMCSR 0xFEE028
#define CYGARC_RTCNT  0xFEE029
#define CYGARC_RTCOR  0xFEE02A

#endif
