#ifndef CYGONCE_LIBC_SETJMP_H
#define CYGONCE_LIBC_SETJMP_H
/*===========================================================================
//
//      setjmp.h
//
//      ISO C standard non-local jumps
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-30
// Purpose:     
// Description:  Header file to implement ISO C standard non-local jumps as
//               per ISO C para 7.6
// Usage:        #include <setjmp.h>
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_setjmp.h> /* Configuration header */

/* INCLUDES */

#include <cyg/hal/hal_arch.h>    /* HAL architecture specific implementation */

/* TYPE DEFINITIONS */

/* jmp_buf as per ANSI 7.6. This is simply the underlying HAL buffer */

typedef hal_jmp_buf jmp_buf;

/* MACROS */

/* setjmp() function, as described in ANSI para 7.6.1.1 */
#define setjmp( __env__ )  hal_setjmp( __env__ )

/* FUNCTION PROTOTYPES */

#ifdef __cplusplus
extern "C" {
#endif

/* longjmp() function, as described in ANSI para 7.6.2.1 */
extern void
longjmp( jmp_buf, int ) CYGBLD_ATTRIB_NORET;

#ifdef __cplusplus
} /* extern "C" */
#endif 

#endif /* CYGONCE_LIBC_SETJMP_H multiple inclusion protection */

/* EOF setjmp.h */
