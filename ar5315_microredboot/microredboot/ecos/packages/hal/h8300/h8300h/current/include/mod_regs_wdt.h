#ifndef CYGONCE_MOD_REGS_WDT_H
#define CYGONCE_MOD_REGS_WDT_H

//==========================================================================
//
//      mod_regs_wdt.h
//
//      Watchdog Timer Register
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

#define CYGARC_TCSR    0xFFFF8C
#define CYGARC_TCNT    0xFFFF8D
#define CYGARC_RSTCSRW 0xFFFF8E
#define CYGARC_RSTCSRR 0xFFFF8F

#define CYGARC_TCNT_MAGIC 0x5A00
#define CYGARC_TCSR_MAGIC 0xA500

#define CYGARC_WDT_OVF  0x80
#define CYGARC_WDT_WT   0x40
#define CYGARC_WDT_TME  0x20
#define CYGARC_WDT_CKS2 0x04
#define CYGARC_WDT_CKS1 0x02
#define CYGARC_WDT_CKS0 0x01

#define CYGARC_WDT_CKS  0x07  //Max Delay

#define CYGARC_WDT_PERIOD ((1000000000/(CYGHWR_HAL_H8300_PROCESSOR_SPEED/4096))*256)

#endif

// EOF mod_regs_wdt.h
