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

#define CYGARC_P1DDR 0xFFFE20
#define CYGARC_P2DDR 0xFFFE21
#define CYGARC_P3DDR 0xFFFE22
#define CYGARC_P4DDR 0xFFFE23
#define CYGARC_P5DDR 0xFFFE24
#define CYGARC_P6DDR 0xFFFE25
#define CYGARC_P7DDR 0xFFFE26
#define CYGARC_P8DDR 0xFFFE27
#define CYGARC_P9DDR 0xFFFE28
#define CYGARC_PADDR 0xFFFE29
#define CYGARC_PBDDR 0xFFFE2A
#define CYGARC_PCDDR 0xFFFE2B
#define CYGARC_PDDDR 0xFFFE2C
#define CYGARC_PEDDR 0xFFFE2D
#define CYGARC_PFDDR 0xFFFE2E
#define CYGARC_PGDDR 0xFFFE2F
#define CYGARC_PHDDR 0xFFFF74

#define CYGARC_PFCR0 0xFFFE32
#define CYGARC_PFCR1 0xFFFE33
#define CYGARC_PFCR2 0xFFFE34

#define CYGARC_PAPCR 0xFFFE36
#define CYGARC_PBPCR 0xFFFE37
#define CYGARC_PCPCR 0xFFFE38
#define CYGARC_PDPCR 0xFFFE39
#define CYGARC_PEPCR 0xFFFE3A

#define CYGARC_P3ODR 0xFFFE3C
#define CYGARC_PAODR 0xFFFE3D

#define CYGARC_P1DR  0xFFFF60
#define CYGARC_P2DR  0xFFFF61
#define CYGARC_P3DR  0xFFFF62
#define CYGARC_P4DR  0xFFFF63
#define CYGARC_P5DR  0xFFFF64
#define CYGARC_P6DR  0xFFFF65
#define CYGARC_P7DR  0xFFFF66
#define CYGARC_P8DR  0xFFFF67
#define CYGARC_P9DR  0xFFFF68
#define CYGARC_PADR  0xFFFF69
#define CYGARC_PBDR  0xFFFF6A
#define CYGARC_PCDR  0xFFFF6B
#define CYGARC_PDDR  0xFFFF6C
#define CYGARC_PEDR  0xFFFF6D
#define CYGARC_PFDR  0xFFFF6E
#define CYGARC_PGDR  0xFFFF6F
#define CYGARC_PHDR  0xFFFF72

#define CYGARC_PORT1 0xFFFF50
#define CYGARC_PORT2 0xFFFF51
#define CYGARC_PORT3 0xFFFF52
#define CYGARC_PORT4 0xFFFF53
#define CYGARC_PORT5 0xFFFF54
#define CYGARC_PORT6 0xFFFF55
#define CYGARC_PORT7 0xFFFF56
#define CYGARC_PORT8 0xFFFF57
#define CYGARC_PORT9 0xFFFF58
#define CYGARC_PORTA 0xFFFF59
#define CYGARC_PORTB 0xFFFF5A
#define CYGARC_PORTC 0xFFFF5B
#define CYGARC_PORTD 0xFFFF5C
#define CYGARC_PORTE 0xFFFF5D
#define CYGARC_PORTF 0xFFFF5E
#define CYGARC_PORTG 0xFFFF5F
#define CYGARC_PORTH 0xFFFF70

#endif
