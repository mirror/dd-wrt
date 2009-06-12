#ifndef CYGONCE_PPC_FADS_H
#define CYGONCE_PPC_FADS_H

//=============================================================================
//####UNSUPPORTEDBEGIN####
//
// -------------------------------------------
// This source file has been contributed to eCos/Red Hat. It may have been
// changed slightly to provide an interface consistent with those of other 
// files.
//
// The functionality and contents of this file is supplied "AS IS"
// without any form of support and will not necessarily be kept up
// to date by Red Hat.
//
// The style of programming used in this file may not comply with the
// eCos programming guidelines. Please do not use as a base for other
// files.
//
// All inquiries about this file, or the functionality provided by it,
// should be directed to the 'ecos-discuss' mailing list (see
// http://sourceware.cygnus.com/ecos/intouch.html for details).
//
// Contributed by: Kevin Hester <khester@opticworks.com>
// Maintained by:  <Unmaintained>
// -------------------------------------------
//
//####UNSUPPORTEDEND####
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

// Board specific structures for the motorola FADS 860 evaluation board
// kevinh 



/*-----------------------------------*/
/* Board Control and Status Register */
/*-----------------------------------*/

typedef struct bcsr 

{
        UWORD  bcsr0; 
        UWORD  bcsr1;
        UWORD  bcsr2;
        UWORD  bcsr3;

} BCSR;

#endif
