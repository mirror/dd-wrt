/* Do not define CYGONCE_ISO_STDDEF_H. It is not appropriate for this file */
/*========================================================================
//
//      stddef.h
//
//      ISO standard defines
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-14
// Purpose:       This file provides the standard defines required by ISO C
// Description:   This file is really provided by the compiler, although in
//                due course we may override things here.
//
//                The main purpose of this file is as a comment to the FAQ
//                about where stddef.h lives: it is provided by the compiler,
//                which for current gcc can be found by looking in the
//                include directory in the same place as the specs file
//                which you can determine from TARGET-gcc -v,
//                e.g. arm-elf-gcc -v
// Usage:         #include <stddef.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

#include_next <stddef.h>

/* EOF stddef.h */
