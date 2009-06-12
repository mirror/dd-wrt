#ifndef CYGONCE_ISO_CTYPE_H
#define CYGONCE_ISO_CTYPE_H
/*========================================================================
//
//      ctype.h
//
//      ISO ctype functions
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
// Purpose:       This file provides the ctype functions required by 
//                ISO C 9899:1990 and POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <ctype.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

#if CYGINT_ISO_CTYPE
# ifdef CYGBLD_ISO_CTYPE_HEADER
#  include CYGBLD_ISO_CTYPE_HEADER
# else

#ifdef __cplusplus
extern "C" {
#endif

/* FUNCTION PROTOTYPES */

//=========================================================================*/

/* 7.3.1 Character testing functions */

extern int
isalnum( int );

extern int
isalpha( int );

extern int
iscntrl( int );

extern int
isdigit( int );

extern int
isgraph( int );

extern int
islower( int );

extern int
isprint( int );

extern int
ispunct( int );

extern int
isspace( int );

extern int
isupper( int );

extern int
isxdigit( int );

/*=========================================================================*/

/* 7.3.2 Character case mapping functions */

extern int
tolower( int );

extern int
toupper( int );

#ifdef __cplusplus
}   /* extern "C" */
#endif

# endif
#endif

#endif /* CYGONCE_ISO_CTYPE_H multiple inclusion protection */

/* EOF ctype.h */
