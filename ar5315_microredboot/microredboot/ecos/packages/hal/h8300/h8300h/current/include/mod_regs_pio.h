#ifndef CYGONCE_MOD_REGS_PIO_H
#define CYGONCE_MOD_REGS_PIO_H

//==========================================================================
//
//      mod_regs_pio.h
//
//      I/O Port Controler Register
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

#define CYGARC_P1DDR 0xFEE000
#define CYGARC_P2DDR 0xFEE001
#define CYGARC_P3DDR 0xFEE002
#define CYGARC_P4DDR 0xFEE003
#define CYGARC_P5DDR 0xFEE004
#define CYGARC_P6DDR 0xFEE005
/*#define CYGARC_P7DDR 0xFEE006*/
#define CYGARC_P8DDR 0xFEE007
#define CYGARC_P9DDR 0xFEE008
#define CYGARC_PADDR 0xFEE009
#define CYGARC_PBDDR 0xFEE00A

#define CYGARC_P1DR  0xFFFFD0
#define CYGARC_P2DR  0xFFFFD1
#define CYGARC_P3DR  0xFFFFD2
#define CYGARC_P4DR  0xFFFFD3
#define CYGARC_P5DR  0xFFFFD4
#define CYGARC_P6DR  0xFFFFD5
#define CYGARC_P7DR  0xFFFFD6
#define CYGARC_P8DR  0xFFFFD7
#define CYGARC_P9DR  0xFFFFD8
#define CYGARC_PADR  0xFFFFD9
#define CYGARC_PBDR  0xFFFFDA

#define CYGARC_P2CR  0xFEE03C
#define CYGARC_P4CR  0xFEE03E
#define CYGARC_P5CR  0xFEE03F

#endif
