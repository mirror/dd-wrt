/*=============================================================================
//
//      v850flash.S
//
//      CEB v850 flash boot code
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
// Author(s):    jlarmour
// Contributors: 
// Date:         2001-03-21
// Purpose:      Loader image to program into V850 internal flash to allow
//               programming of eCos ROM startup applications into the EPROM.
// Description:  Compile with:
//            v850-elf-gcc -o v850flash.img v850flash.S -Ttext=0 -nostartfiles
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

        .text
        .org 0
        .globl _start // dummy _start to keep linker happy
_start:
        mov 7,r1
        st.b r1,0xFFFFF04C[r0]
        mov 1,r1
        st.b r1,0xFFFFF064[r0]

        jr 0x0FFFF4 // jump to 0x100000
        jr 0x0FFFF2 // jump to 0x100002
        
        .org 0x20
        .rept 0x3E
        jr 0xFFFC0000  // jump to symmetric location in RAM
        nop
        nop
        nop
        nop
        nop
        nop
        .endr

// EOF v850flash.S
