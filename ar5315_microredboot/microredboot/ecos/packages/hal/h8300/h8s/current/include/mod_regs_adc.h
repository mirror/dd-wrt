#ifndef CYGONCE_MOD_REGS_ADC_H
#define CYGONCE_MOD_REGS_ADC_H

//==========================================================================
//
//      mod_regs_adc.h
//
//      A/D D/A Converter Register
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

#define CYGARC_DASTCR 0xFEE01A
#define CYGARC_DADR0  0xFFFFA4
#define CYGARC_DADR1  0xFFFFA5
#define CYGARC_DACR01 0xFFFFA6
#define CYGARC_DADR2  0xFFFFA8
#define CYGARC_DADR3  0xFFFFA9
#define CYGARC_DACR23 0xFFFFAA

#define CYGARC_ADDRA  0xFFFF90
#define CYGARC_ADDRAH 0xFFFF90
#define CYGARC_ADDRAL 0xFFFF91
#define CYGARC_ADDRB  0xFFFF92
#define CYGARC_ADDRBH 0xFFFF92
#define CYGARC_ADDRBL 0xFFFF93
#define CYGARC_ADDRC  0xFFFF94
#define CYGARC_ADDRCH 0xFFFF94
#define CYGARC_ADDRCL 0xFFFF95
#define CYGARC_ADDRD  0xFFFF96
#define CYGARC_ADDRDH 0xFFFF96
#define CYGARC_ADDRDL 0xFFFF97
#define CYGARC_ADDRE  0xFFFF98
#define CYGARC_ADDREH 0xFFFF98
#define CYGARC_ADDREL 0xFFFF99
#define CYGARC_ADDRF  0xFFFF9A
#define CYGARC_ADDRFH 0xFFFF9A
#define CYGARC_ADDRFL 0xFFFF9B
#define CYGARC_ADDRG  0xFFFF9C
#define CYGARC_ADDRGH 0xFFFF9C
#define CYGARC_ADDRGL 0xFFFF9D
#define CYGARC_ADDRH  0xFFFF9E
#define CYGARC_ADDRHH 0xFFFF9E
#define CYGARC_ADDRHL 0xFFFF9F

#define CYGARC_ADCSR  0xFFFFA0
#define CYGARC_ADCR   0xFFFFA1

#endif
