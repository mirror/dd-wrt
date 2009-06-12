//=============================================================================
//
//      mod_regs_frt.h
//
//      FRT (Free-Running Timer) Module register definitions
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
// Date:        2002-01-16
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// Free Running Timer

#define CYGARC_REG_TIER              0xfffffe10
#define CYGARC_REG_FTCSR             0xfffffe11
#define CYGARC_REG_FRC               0xfffffe12
#define CYGARC_REG_OCR               0xfffffe14
#define CYGARC_REG_TCR               0xfffffe16
#define CYGARC_REG_TOCR              0xfffffe17
#define CYGARC_REG_FICR              0xfffffe18


#define CYGARC_REG_TIER_ICIE         0x80 // input capture irq enable
#define CYGARC_REG_TIER_OCIAE        0x08 // output compare A irq enable
#define CYGARC_REG_TIER_OCIBE        0x04 // output compare B irq enable
#define CYGARC_REG_TIER_OVIE         0x02 // overflow irq enable

#define CYGARC_REG_FTCSR_ICF         0x80 // input capture flag
#define CYGARC_REG_FTCSR_OCFA        0x08 // output compare flag A
#define CYGARC_REG_FTCSR_OCFB        0x04 // output compare flag B
#define CYGARC_REG_FTCSR_OVF         0x02 // timer overflow flag
#define CYGARC_REG_FTCSR_CCLRA       0x01 // counter clear A

#define CYGARC_REG_TCR_IEDG          0x80 // capture on falling (0) or rising (1)
#define CYGARC_REG_TCR_CLK_8         0x00 // clk/8
#define CYGARC_REG_TCR_CLK_32        0x01 // clk/32
#define CYGARC_REG_TCR_CLK_128       0x02 // clk/128
#define CYGARC_REG_TCR_CLK_EXT       0x03 // external clock

#define CYGARC_REG_TOCR_OCRS         0x08 // Select OCRA (0) or OCRB (1) on _OCR
#define CYGARC_REG_TOCR_OLVLA        0x02 // enable output on match A
#define CYGARC_REG_TOCR_OLVLB        0x01 // enable output on match B
