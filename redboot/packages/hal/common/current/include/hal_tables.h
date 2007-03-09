#ifndef CYGONCE_HAL_TABLES_H
#define CYGONCE_HAL_TABLES_H

/*==========================================================================
//
//      hal_tables.h
//
//      Data table management
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
// Author(s):   nickg
// Date:        2000-09-04
// Purpose:     Provide HAL tables
// Description: This file defines a mechanism to include "tables" of objects
//              that are always included in the image no matter what, and are
//              constrained between labels.
//              
// Usage:       #include <cyg/hal/hal_tables.h>
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>

/*------------------------------------------------------------------------*/

#define __string(_x) #_x
#define __xstring(_x) __string(_x)

#ifndef CYG_HAL_TABLE_BEGIN
#define CYG_HAL_TABLE_BEGIN( _label, _name )                                 \
__asm__(".section \".ecos.table." __xstring(_name) ".begin\",\"aw\"\n"       \
    ".globl " __xstring(CYG_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(CYG_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(CYGARC_P2ALIGNMENT) "\n"                           \
__xstring(CYG_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

#ifndef CYG_HAL_TABLE_END
#define CYG_HAL_TABLE_END( _label, _name )                                   \
__asm__(".section \".ecos.table." __xstring(_name) ".finish\",\"aw\"\n"      \
    ".globl " __xstring(CYG_LABEL_DEFN(_label)) "\n"                         \
    ".type    " __xstring(CYG_LABEL_DEFN(_label)) ",object\n"                \
    ".p2align " __xstring(CYGARC_P2ALIGNMENT) "\n"                           \
__xstring(CYG_LABEL_DEFN(_label)) ":\n"                                      \
    ".previous\n"                                                            \
       )
#endif

// This macro must be applied to any types whose objects are to be placed in
// tables
#ifndef CYG_HAL_TABLE_TYPE
#define CYG_HAL_TABLE_TYPE CYGBLD_ATTRIB_ALIGN( CYGARC_ALIGNMENT )
#endif

#ifndef CYG_HAL_TABLE_EXTRA
#define CYG_HAL_TABLE_EXTRA( _name ) \
        CYGBLD_ATTRIB_SECTION(".ecos.table." __xstring(_name) ".extra")
#endif

#ifndef CYG_HAL_TABLE_ENTRY
#define CYG_HAL_TABLE_ENTRY( _name ) \
        CYGBLD_ATTRIB_SECTION(".ecos.table." __xstring(_name) ".data")
#endif

#ifndef CYG_HAL_TABLE_QUALIFIED_ENTRY
#define CYG_HAL_TABLE_QUALIFIED_ENTRY( _name, _qual ) \
        CYGBLD_ATTRIB_SECTION(".ecos.table." __xstring(_name) ".data." \
                              __xstring(_qual))
#endif

/*------------------------------------------------------------------------*/
/* EOF hal_tables.h                                                       */
#endif // CYGONCE_HAL_TABLES_H
