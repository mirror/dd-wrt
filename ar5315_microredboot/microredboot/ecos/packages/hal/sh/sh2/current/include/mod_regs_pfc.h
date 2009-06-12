//=============================================================================
//
//      mod_regs_pfc.h
//
//      PFC (pin function controller) Module register definitions
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
// Date:        2002-01-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#define CYGARC_REG_PACR                 0xfffffc80 // port A control
#define CYGARC_REG_PAIOR                0xfffffc82 // port A IO
#define CYGARC_REG_PBCR                 0xfffffc88 // port A control
#define CYGARC_REG_PBIOR                0xfffffc8a // port A IO
#define CYGARC_REG_PBCR2                0xfffffc8e // port A control 2

#define CYGARC_REG_PADR                 0xfffffc84 // port A data
#define CYGARC_REG_PBDR                 0xfffffc8c // port B data


// Port A
#define CYGARC_REG_PACR_PA13MD          0x2000
#define CYGARC_REG_PACR_PA12MD          0x1000
#define CYGARC_REG_PACR_PA11MD          0x0800
#define CYGARC_REG_PACR_PA10MD          0x0400
#define CYGARC_REG_PACR_PA9MD           0x0200
#define CYGARC_REG_PACR_PA8MD           0x0100
#define CYGARC_REG_PACR_PA7MD           0x0080
#define CYGARC_REG_PACR_PA6MD           0x0040
#define CYGARC_REG_PACR_PA5MD           0x0020
#define CYGARC_REG_PACR_PA4MD           0x0010
#define CYGARC_REG_PACR_PA3MD           0x0008
#define CYGARC_REG_PACR_PA2MD           0x0004
#define CYGARC_REG_PACR_PA1MD           0x0002
#define CYGARC_REG_PACR_PA0MD           0x0001

#define CYGARC_REG_PAIOR_PA13IOR        0x2000
#define CYGARC_REG_PAIOR_PA12IOR        0x1000
#define CYGARC_REG_PAIOR_PA11IOR        0x0800
#define CYGARC_REG_PAIOR_PA10IOR        0x0400
#define CYGARC_REG_PAIOR_PA9IOR         0x0200
#define CYGARC_REG_PAIOR_PA8IOR         0x0100
#define CYGARC_REG_PAIOR_PA7IOR         0x0080
#define CYGARC_REG_PAIOR_PA6IOR         0x0040
#define CYGARC_REG_PAIOR_PA5IOR         0x0020
#define CYGARC_REG_PAIOR_PA4IOR         0x0010
//#define CYGARC_REG_PAIOR_PA3IOR       0x0008
#define CYGARC_REG_PAIOR_PA2IOR         0x0004
#define CYGARC_REG_PAIOR_PA1IOR         0x0002
#define CYGARC_REG_PAIOR_PA0IOR         0x0001

#define CYGARC_REG_PBCR_PB15MD_mask     0xc000
#define CYGARC_REG_PBCR_PB15MD_shift    14
#define CYGARC_REG_PBCR_PB14MD_mask     0x3000
#define CYGARC_REG_PBCR_PB14MD_shift    12
#define CYGARC_REG_PBCR_PB13MD_mask     0x0c00
#define CYGARC_REG_PBCR_PB13MD_shift    10
#define CYGARC_REG_PBCR_PB12MD_mask     0x0300
#define CYGARC_REG_PBCR_PB12MD_shift    8
#define CYGARC_REG_PBCR_PB11MD_mask     0x00c0
#define CYGARC_REG_PBCR_PB11MD_shift    6
#define CYGARC_REG_PBCR_PB10MD_mask     0x0030
#define CYGARC_REG_PBCR_PB10MD_shift    4
#define CYGARC_REG_PBCR_PB9MD_mask      0x000c
#define CYGARC_REG_PBCR_PB9MD_shift     2
#define CYGARC_REG_PBCR_PB8MD_mask      0x0003
#define CYGARC_REG_PBCR_PB8MD_shift     0

#define CYGARC_REG_PBCR2_PB7MD_mask     0xc000
#define CYGARC_REG_PBCR2_PB7MD_shift    14
#define CYGARC_REG_PBCR2_PB6MD_mask     0x3000
#define CYGARC_REG_PBCR2_PB6MD_shift    12
#define CYGARC_REG_PBCR2_PB5MD_mask     0x0c00
#define CYGARC_REG_PBCR2_PB5MD_shift    10
#define CYGARC_REG_PBCR2_PB4MD_mask     0x0300
#define CYGARC_REG_PBCR2_PB4MD_shift    8
#define CYGARC_REG_PBCR2_PB3MD_mask     0x00c0
#define CYGARC_REG_PBCR2_PB3MD_shift    6
#define CYGARC_REG_PBCR2_PB2MD_mask     0x0030
#define CYGARC_REG_PBCR2_PB2MD_shift    4
#define CYGARC_REG_PBCR2_PB1MD_mask     0x000c
#define CYGARC_REG_PBCR2_PB1MD_shift    2
#define CYGARC_REG_PBCR2_PB0MD_mask     0x0003
#define CYGARC_REG_PBCR2_PB0MD_shift    0

#define CYGARC_REG_PBIOR_PB15IOR_OUT    0x8000
#define CYGARC_REG_PBIOR_PB14IOR_OUT    0x4000
#define CYGARC_REG_PBIOR_PB13IOR_OUT    0x2000
#define CYGARC_REG_PBIOR_PB12IOR_OUT    0x1000
#define CYGARC_REG_PBIOR_PB11IOR_OUT    0x0800
#define CYGARC_REG_PBIOR_PB10IOR_OUT    0x0400
#define CYGARC_REG_PBIOR_PB9IOR_OUT     0x0200
#define CYGARC_REG_PBIOR_PB8IOR_OUT     0x0100
#define CYGARC_REG_PBIOR_PB7IOR_OUT     0x0080
#define CYGARC_REG_PBIOR_PB6IOR_OUT     0x0040
#define CYGARC_REG_PBIOR_PB5IOR_OUT     0x0020
#define CYGARC_REG_PBIOR_PB4IOR_OUT     0x0010
#define CYGARC_REG_PBIOR_PB3IOR_OUT     0x0008
#define CYGARC_REG_PBIOR_PB2IOR_OUT     0x0004
#define CYGARC_REG_PBIOR_PB1IOR_OUT     0x0002
#define CYGARC_REG_PBIOR_PB0IOR_OUT     0x0001

#define CYGARC_REG_PBIOR_PB15IOR_IN     0x0000
#define CYGARC_REG_PBIOR_PB14IOR_IN     0x0000
#define CYGARC_REG_PBIOR_PB13IOR_IN     0x0000
#define CYGARC_REG_PBIOR_PB12IOR_IN     0x0000
#define CYGARC_REG_PBIOR_PB11IOR_IN     0x0000
#define CYGARC_REG_PBIOR_PB10IOR_IN     0x0000
#define CYGARC_REG_PBIOR_PB9IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB8IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB7IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB6IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB5IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB4IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB3IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB2IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB1IOR_IN      0x0000
#define CYGARC_REG_PBIOR_PB0IOR_IN      0x0000

//#define CYGARC_REG_PADR_PA15DR        0x8000
//#define CYGARC_REG_PADR_PA14DR        0x4000
#define CYGARC_REG_PADR_PA13DR          0x2000
#define CYGARC_REG_PADR_PA12DR          0x1000
#define CYGARC_REG_PADR_PA11DR          0x0800
#define CYGARC_REG_PADR_PA10DR          0x0400
#define CYGARC_REG_PADR_PA9DR           0x0200
#define CYGARC_REG_PADR_PA8DR           0x0100
#define CYGARC_REG_PADR_PA7DR           0x0080
#define CYGARC_REG_PADR_PA6DR           0x0040
#define CYGARC_REG_PADR_PA5DR           0x0020
#define CYGARC_REG_PADR_PA4DR           0x0010
//#define CYGARC_REG_PADR_PA3DR         0x0008
#define CYGARC_REG_PADR_PA2DR           0x0004
#define CYGARC_REG_PADR_PA1DR           0x0002
#define CYGARC_REG_PADR_PA0DR           0x0001

#define CYGARC_REG_PBDR_PB15DR          0x8000
#define CYGARC_REG_PBDR_PB14DR          0x4000
#define CYGARC_REG_PBDR_PB13DR          0x2000
#define CYGARC_REG_PBDR_PB12DR          0x1000
#define CYGARC_REG_PBDR_PB11DR          0x0800
#define CYGARC_REG_PBDR_PB10DR          0x0400
#define CYGARC_REG_PBDR_PB9DR           0x0200
#define CYGARC_REG_PBDR_PB8DR           0x0100
#define CYGARC_REG_PBDR_PB7DR           0x0080
#define CYGARC_REG_PBDR_PB6DR           0x0040
#define CYGARC_REG_PBDR_PB5DR           0x0020
#define CYGARC_REG_PBDR_PB4DR           0x0010
#define CYGARC_REG_PBDR_PB3DR           0x0008
#define CYGARC_REG_PBDR_PB2DR           0x0004
#define CYGARC_REG_PBDR_PB1DR           0x0002
#define CYGARC_REG_PBDR_PB0DR           0x0001
