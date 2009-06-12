#ifndef CYGONCE_MOD_REGS_SYS_H
#define CYGONCE_MOD_REGS_SYS_H

//==========================================================================
//
//      mod_regs_sys.h
//
//      System Controler Register
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

#define CYGARC_MDCR    0xFFFF3E
#define CYGARC_SYSCR   0xFFFF3D
#define CYGARC_MSTPCRH 0xFFFF40
#define CYGARC_MSTPCRL 0xFFFF41
#define CYGARC_FLMCR1  0xFFFFC8
#define CYGARC_FLMCR2  0xFFFFC9
#define CYGARC_EBR1    0xFFFFCA
#define CYGARC_EBR2    0xFFFFCB
#define CTGARC_RAMCR   0xFFFECE
#define CYGARC_SBYCR   0xFFFF3A
#define CYGARC_SCKCR   0xFFFF3B
#define CYGARC_PLLCR   0xFFFF45

#endif
