#ifndef CYGONCE_LIBC_I18N_MB_H
#define CYGONCE_LIBC_I18N_MB_H
/*===========================================================================
//
//      mb.h
//
//      Definition of MB_CUR_MAX when using multiple locales
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
// Date:         2001-06-08
// Purpose:     
// Description: 
// Usage:        Do not include this file directly - use #include <ctype.h>
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_i18n.h>   /* Configuration header */

#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
extern int __mb_cur_max;
# define MB_CUR_MAX (__mb_cur_max)
#endif

#endif /* CYGONCE_LIBC_I18N_MB_H multiple inclusion protection */

/* EOF mb.h */
