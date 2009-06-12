#ifndef CYGONCE_INFRA_CYG_TYPE_INC
#define CYGONCE_INFRA_CYG_TYPE_INC

//==========================================================================
//
//      cyg_type.inc
//
//      Standard types, and some useful coding macros.
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
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-07-25
// Purpose:      Define symbols for use in linker scripts and "generic" asm
// Description:  This file should only use #defines - it should be safe
//               to include from both linker scripts and assembler files
// Usage:        #include <cyg/infra/cyg_type.inc>
//
//              
//####DESCRIPTIONEND####

#include <cyg/hal/basetype.h>

// -------------------------------------------------------------------------
// Label name macros. Some toolsets generate labels with initial
// underscores and others don't. CYG_LABEL_NAME should be used on
// labels in C/C++ code that are defined in assembly code or linker
// scripts. CYG_LABEL_DEFN is for use in assembly code and linker
// scripts where we need to manufacture labels that can be used from
// C/C++.
// These are default implementations that should work for most targets.
// They may be overridden in basetype.h if necessary.

#ifndef CYG_LABEL_DEFN
# define CYG_LABEL_DEFN(_label) _label
#endif

// -------------------------------------------------------------------------
// Provide a default architecture alignment.
// This may be overridden in basetype.h if necessary.

#ifndef CYGARC_ALIGNMENT
# define CYGARC_ALIGNMENT 8
#endif
// And corresponding power of two alignment
#ifndef CYGARC_P2ALIGNMENT
# define CYGARC_P2ALIGNMENT 3
#endif

#endif /* CYGONCE_INFRA_CYG_TYPE_INC */

// EOF cyg_type.inc
