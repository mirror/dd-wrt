#ifndef CYGONCE_MOD_REGS_DMAC_H
#define CYGONCE_MOD_REGS_DMAC_H

//==========================================================================
//
//      mod_regs_dmac.h
//
//      DMA Controler Register
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

#define CYGARC_MAR0AR  0xFFFF20
#define CYGARC_MAR0AE  0xFFFF21
#define CYGARC_MAR0AH  0xFFFF22
#define CYGARC_MAR0AL  0xFFFF23
#define CYGARC_ETCR0AL 0xFFFF24
#define CYGARC_ETCR0AH 0xFFFF25
#define CYGARC_IOAR0A  0xFFFF26
#define CYGARC_DTCR0A  0xFFFF27
#define CYGARC_MAR0BR  0xFFFF28
#define CYGARC_MAR0BE  0xFFFF29
#define CYGARC_MAR0BH  0xFFFF2A
#define CYGARC_MAR0BL  0xFFFF2B
#define CYGARC_ETCR0BL 0xFFFF2C
#define CYGARC_ETCR0BH 0xFFFF2D
#define CYGARC_IOAR0B  0xFFFF2E
#define CYGARC_DTCR0B  0xFFFF2F
#define CYGARC_MAR1AR  0xFFFF30
#define CYGARC_MAR1AE  0xFFFF31
#define CYGARC_MAR1AH  0xFFFF32
#define CYGARC_MAR1AL  0xFFFF33
#define CYGARC_ETCR1AL 0xFFFF34
#define CYGARC_ETCR1AH 0xFFFF35
#define CYGARC_IOAR1A  0xFFFF36
#define CYGARC_DTCR1A  0xFFFF37
#define CYGARC_MAR1BR  0xFFFF38
#define CYGARC_MAR1BE  0xFFFF39
#define CYGARC_MAR1BH  0xFFFF3A
#define CYGARC_MAR1BL  0xFFFF3B
#define CYGARC_ETCR1BL 0xFFFF3C
#define CYGARC_ETCR1BH 0xFFFF3D
#define CYGARC_IOAR1B  0xFFFF3E
#define CYGARC_DTCR1B  0xFFFF3F

#endif
