#ifndef CYGONCE_MOD_REGS_TMR_H
#define CYGONCE_MOD_REGS_TMR_H

//==========================================================================
//
//      mod_regs_tmr.h
//
//       TPU/TMR Register
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

#define CYGARC_TSTR   0xFFFFC0
#define CYGARC_TSNC   0XFFFFC1

#define CYGARC_TCR0   0xFFFFD0
#define CYGARC_TMDR0  0xFFFFD1
#define CYGARC_TIORH0 0xFFFFD2
#define CYGARC_TIORL0 0xFFFFD3
#define CYGARC_TIER0  0xFFFFD4
#define CYGARC_TSR0   0xFFFFD5
#define CYGARC_TCNT0  0xFFFFD6
#define CYGARC_GRA0   0xFFFFD8
#define CYGARC_GRB0   0xFFFFDA
#define CYGARC_GRC0   0xFFFFDC
#define CYGARC_GRD0   0xFFFFDE
#define CYGARC_TCR1   0xFFFFE0
#define CYGARC_TMDR1  0xFFFFE1
#define CYGARC_TIORH1 0xFFFFE2
#define CYGARC_TIORL1 0xFFFFE3
#define CYGARC_TIER1  0xFFFFE4
#define CYGARC_TSR1   0xFFFFE5
#define CYGARC_TCNT1  0xFFFFE6
#define CYGARC_GRA1   0xFFFFE8
#define CYGARC_GRB1   0xFFFFEA
#define CYGARC_TCR2   0xFFFFF0
#define CYGARC_TMDR2  0xFFFFF1
#define CYGARC_TIORH2 0xFFFFF2
#define CYGARC_TIORL2 0xFFFFF3
#define CYGARC_TIER2  0xFFFFF4
#define CYGARC_TSR2   0xFFFFF5
#define CYGARC_TCNT2  0xFFFFF6
#define CYGARC_GRA2   0xFFFFF8
#define CYGARC_GRB2   0xFFFFFA
#define CYGARC_TCR3   0xFFFE80
#define CYGARC_TMDR3  0xFFFE81
#define CYGARC_TIORH3 0xFFFE82
#define CYGARC_TIORL3 0xFFFE83
#define CYGARC_TIER3  0xFFFE84
#define CYGARC_TSR3   0xFFFE85
#define CYGARC_TCNT3  0xFFFE86
#define CYGARC_GRA3   0xFFFE88
#define CYGARC_GRB3   0xFFFE8A
#define CYGARC_GRC3   0xFFFE8C
#define CYGARC_GRD3   0xFFFE8E
#define CYGARC_TCR4   0xFFFE90
#define CYGARC_TMDR4  0xFFFE91
#define CYGARC_TIORH4 0xFFFE92
#define CYGARC_TIORL4 0xFFFE93
#define CYGARC_TIER4  0xFFFE94
#define CYGARC_TSR4   0xFFFE95
#define CYGARC_TCNT4  0xFFFE96
#define CYGARC_GRA4   0xFFFE98
#define CYGARC_GRB4   0xFFFE9A
#define CYGARC_TCR5   0xFFFEA0
#define CYGARC_TMDR5  0xFFFEA1
#define CYGARC_TIORH5 0xFFFEA2
#define CYGARC_TIORL5 0xFFFEA3
#define CYGARC_TIER5  0xFFFEA4
#define CYGARC_TSR5   0xFFFEA5
#define CYGARC_TCNT5  0xFFFEA6
#define CYGARC_GRA5   0xFFFEA8
#define CYGARC_GRB5   0xFFFEAA

#define CYGARC_8TCR0   0xFFFFB0
#define CYGARC_8TCR1   0xFFFFB1
#define CYGARC_8TCSR0  0xFFFFB2
#define CYGARC_8TCSR1  0xFFFFB3
#define CYGARC_8TCORA0 0xFFFFB4
#define CYGARC_8TCORA1 0xFFFFB5
#define CYGARC_8TCORB0 0xFFFFB6
#define CYGARC_8TCORB1 0xFFFFB7
#define CYGARC_8TCNT0  0xFFFFB8
#define CYGARC_8TCNT1  0xFFFFB9

#endif
