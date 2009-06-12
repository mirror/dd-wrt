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
// Date:         2003-01-01
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGARC_MAR0AH  0xFFFEE0
#define CYGARC_MAR0AL  0xFFFEE2
#define CYGARC_IOAR0A  0xFFFEE4
#define CYGARC_ETCR0A  0xFFFEE6
#define CYGARC_MAR0BH  0xFFFEE8
#define CYGARC_MAR0BL  0xFFFEEA
#define CYGARC_IOAR0B  0xFFFEEC
#define CYGARC_ETCR0B  0xFFFEEE
#define CYGARC_MAR1AH  0xFFFEF0
#define CYGARC_MAR1AL  0xFFFEF2
#define CYGARC_IOAR1A  0xFFFEF4
#define CYGARC_ETCR1A  0xFFFEF6
#define CYGARC_MAR1BH  0xFFFEF8
#define CYGARC_MAR1BL  0xFFFEFA
#define CYGARC_IOAR1B  0xFFFEFC
#define CYGARC_ETCR1B  0xFFFEFE
#define CYGARC_DMAWER  0xFFFF20
#define CYGARC_DMATCR  0xFFFF21
#define CYGARC_DMACR0A 0xFFFF22
#define CYGARC_DMACR0B 0xFFFF23
#define CYGARC_DMACR1A 0xFFFF24
#define CYGARC_DMACR1B 0xFFFF25
#define CYGARC_DMABCRH 0xFFFF26
#define CYGARC_DMABCRL 0xFFFF27

#define CYGARC_EDSAR0  0xFFFDC0
#define CYGARC_EDDAR0  0xFFFDC4
#define CYGARC_EDTCR0  0xFFFDC8
#define CYGARC_EDMDR0  0xFFFDCC
#define CYGARC_EDMDR0H 0xFFFDCC
#define CYGARC_EDMDR0L 0xFFFDCD
#define CYGARC_EDACR0  0xFFFDCE
#define CYGARC_EDSAR1  0xFFFDD0
#define CYGARC_EDDAR1  0xFFFDD4
#define CYGARC_EDTCR1  0xFFFDD8
#define CYGARC_EDMDR1  0xFFFDDC
#define CYGARC_EDMDR1H 0xFFFDDC
#define CYGARC_EDMDR1L 0xFFFDDD
#define CYGARC_EDACR1  0xFFFDDE
#define CYGARC_EDSAR2  0xFFFDE0
#define CYGARC_EDDAR2  0xFFFDE4
#define CYGARC_EDTCR2  0xFFFDE8
#define CYGARC_EDMDR2  0xFFFDEC
#define CYGARC_EDMDR2H 0xFFFDEC
#define CYGARC_EDMDR2L 0xFFFDED
#define CYGARC_EDACR2  0xFFFDEE
#define CYGARC_EDSAR3  0xFFFDF0
#define CYGARC_EDDAR3  0xFFFDF4
#define CYGARC_EDTCR3  0xFFFDF8
#define CYGARC_EDMDR3  0xFFFDFC
#define CYGARC_EDMDR3H 0xFFFDFC
#define CYGARC_EDMDR3L 0xFFFDFD
#define CYGARC_EDACR3  0xFFFDFE

#endif
