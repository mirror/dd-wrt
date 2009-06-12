#ifndef CYGONCE_MOD_REGS_INTC_H
#define CYGONCE_MOD_REGS_INTC_H

//==========================================================================
//
//      mod_regs_intc.h
//
//      Interrupt Controler Register
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

#define CYGARC_IPRA  0xFFFE00
#define CYGARC_IPRB  0xFFFE02
#define CYGARC_IPRC  0xFFFE04
#define CYGARC_IPRD  0xFFFE06
#define CYGARC_IPRE  0xFFFE08
#define CYGARC_IPRF  0xFFFE0A
#define CYGARC_IPRG  0xFFFE0C
#define CYGARC_IPRH  0xFFFE0E
#define CYGARC_IPRI  0xFFFE10
#define CYGARC_IPRJ  0xFFFE12
#define CYGARC_IPRK  0xFFFE14
#define CYGARC_ITSR  0xFFFE16
#define CYGARC_SSIER 0xFFFE18
#define CYGARC_ISCRH 0xFFFE1A
#define CYGARC_ISCRL 0xFFFE1C

#define CYGARC_INTCR 0xFFFF31
#define CYGARC_IER   0xFFFF32
#define CYGARC_IERH  0xFFFF32
#define CYGARC_IERL  0xFFFF33
#define CYGARC_ISR   0xFFFF34
#define CYGARC_ISRH  0xFFFF34
#define CYGARC_ISRL  0xFFFF35

#endif
