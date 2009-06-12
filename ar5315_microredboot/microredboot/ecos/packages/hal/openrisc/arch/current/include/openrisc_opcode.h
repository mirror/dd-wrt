//=============================================================================
//
//      openrisc_opcode.h
//
//  Define the instruction formats and opcode values for the OpenRISC
//  instruction set...or at least just enough of them to implement
//  single-stepping.
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
// Author(s):   sfurman
// Contributors:
// Date:        2003-02-28
// Purpose:     Allow dissection of OpenRISC instructions
// Description: The types and macros defined here define the instruction
//              formats of the OpenRISC instruction set...or at least the
//              very limited subset necessary to allow single-stepping.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#ifndef _OPENRISC_OPCODE_H
#define _OPENRISC_OPCODE_H


// Define the instruction formats.
typedef union {
    unsigned word;

    // (Possibly conditional) relative jump w/ immediate displacement
    struct {
        unsigned op:      6;    // OP_J, OP_JAL, OP_BNF, or OP_BF
        signed   target: 26;
    } JType;

    // Absolute jump w/ register contents used as PC target address
    struct {
        unsigned op:      6;	// OP_JR or OP_JALR
        unsigned unused1:10;
        unsigned rB:      5;	// Register containing new PC
        unsigned unused2:11;
    } JRType;

} InstFmt;

/*
 * Values for the 'op' field.
 */
#define OP_J            0x00
#define OP_JAL          0x01
#define OP_BNF          0x03
#define OP_BF           0x04
#define OP_RFE          0x09
#define OP_JR           0x11
#define OP_JALR         0x12

#endif  /* _OPENRISC_OPCODE_H */

// EOF openrisc_opcode.h
