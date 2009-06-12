#ifndef CYGONCE_MOD_REGS_TMR_H
#define CYGONCE_MOD_REGS_TMR_H

//==========================================================================
//
//      mod_regs_tmr.h
//
//      16bit/8bit Timer Register
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

#define CYGARC_TSTR   0xFFFF60
#define CYGARC_TSNC   0XFFFF61
#define CYGARC_TMDR   0xFFFF62
#define CYGARC_TOLR   0xFFFF63
#define CYGARC_TISRA  0xFFFF64
#define CYGARC_TISRB  0xFFFF65
#define CYGARC_TISRC  0xFFFF66
#define CYGARC_TCR0   0xFFFF68
#define CYGARC_TIOR0  0xFFFF69
#define CYGARC_TCNT0H 0xFFFF6A
#define CYGARC_TCNT0L 0xFFFF6B
#define CYGARC_GRA0H  0xFFFF6C
#define CYGARC_GRA0L  0xFFFF6D
#define CYGARC_GRB0H  0xFFFF6E
#define CYGARC_GRB0L  0xFFFF6F
#define CYGARC_TCR1   0xFFFF70
#define CYGARC_TIOR1  0xFFFF71
#define CYGARC_TCNT1H 0xFFFF72
#define CYGARC_TCNT1L 0xFFFF73
#define CYGARC_GRA1H  0xFFFF74
#define CYGARC_GRA1L  0xFFFF75
#define CYGARC_GRB1H  0xFFFF76
#define CYGARC_GRB1L  0xFFFF77
#define CYGARC_TCR3   0xFFFF78
#define CYGARC_TIOR3  0xFFFF79
#define CYGARC_TCNT3H 0xFFFF7A
#define CYGARC_TCNT3L 0xFFFF7B
#define CYGARC_GRA3H  0xFFFF7C
#define CYGARC_GRA3L  0xFFFF7D
#define CYGARC_GRB3H  0xFFFF7E
#define CYGARC_GRB3L  0xFFFF7F

#define CYGARC_8TCR0  0xFFFF80
#define CYGARC_8TCR1  0xFFFF81
#define CYGARC_8TCSR0 0xFFFF82
#define CYGARC_8TCSR1 0xFFFF83
#define CYGARC_TCORA0 0xFFFF84
#define CYGARC_TCORA1 0xFFFF85
#define CYGARC_TCORB0 0xFFFF86
#define CYGARC_TCORB1 0xFFFF87
#define CYGARC_8TCNT0 0xFFFF88
#define CYGARC_8TCNT1 0xFFFF89

#define CYGARC_8TCR2  0xFFFF90
#define CYGARC_8TCR3  0xFFFF91
#define CYGARC_8TCSR2 0xFFFF92
#define CYGARC_8TCSR3 0xFFFF93
#define CYGARC_TCORA2 0xFFFF94
#define CYGARC_TCORA3 0xFFFF95
#define CYGARC_TCORB2 0xFFFF96
#define CYGARC_TCORB3 0xFFFF97
#define CYGARC_8TCNT2 0xFFFF98
#define CYGARC_8TCNT3 0xFFFF99

#endif
