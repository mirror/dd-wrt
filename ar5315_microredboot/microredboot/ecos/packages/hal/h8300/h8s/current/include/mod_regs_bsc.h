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
// Date:         2003-01-01
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGARC_ABWCR  0xFFFEC0
#define CYGARC_ASTCR  0xFFFEC1
#define CYGARC_WTCRAH 0xFFFEC2
#define CYGARC_WTCRAL 0xFFFEC3
#define CYGARC_WTCRBH 0xFFFEC4
#define CYGARC_WTCRBL 0xFFFEC5
#define CYGARC_RDNCR  0xFFFEC6
#define CYGARC_CSACRH 0xFFFEC8
#define CYGARC_CSACRL 0xFFFEC9
#define CYGARC_BROMCRH 0xFFFECA
#define CYGARC_BROMCRL 0xFFFECB
#define CYGARC_BCR    0xFFFECC
#define CYGARC_DRAMCR 0xFFFED0
#define CYGARC_DRACCR 0xFFFED2
#define CYGARC_REFCR  0xFFFED4
#define CYGARC_RTCNT  0xFFFED6
#define CYGARC_RTCOR  0xFFFED7

#endif
