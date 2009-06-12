//=============================================================================
//
//      mod_regs_adc.h
//
//      ADC (A/D Converter) Module register definitions
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
// Date:        2002-04-23
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// Register definitions
#define CYGARC_REG_ADDRA0               0xffff8400
#define CYGARC_REG_ADDRB0               0xffff8402
#define CYGARC_REG_ADDRC0               0xffff8404
#define CYGARC_REG_ADDRD0               0xffff8406

#define CYGARC_REG_ADDRA1               0xffff8408
#define CYGARC_REG_ADDRB1               0xffff840a
#define CYGARC_REG_ADDRC1               0xffff840c
#define CYGARC_REG_ADDRD1               0xffff840e

#define CYGARC_REG_ADCSR0               0xffff8410
#define CYGARC_REG_ADCR0                0xffff8412
#define CYGARC_REG_ADCSR1               0xffff8411
#define CYGARC_REG_ADCR1                0xffff8413

#define CYGARC_REG_ADCSR_ADF            0x80
#define CYGARC_REG_ADCSR_ADIE           0x40
#define CYGARC_REG_ADCSR_ADST           0x20
#define CYGARC_REG_ADCSR_SCAN           0x10
#define CYGARC_REG_ADCSR_CKS            0x08
#define CYGARC_REG_ADCSR_CH1            0x02
#define CYGARC_REG_ADCSR_CH0            0x01

#define CYGARC_REG_ADCR_TRGE            0x80
