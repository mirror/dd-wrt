#ifndef CYGONCE_ISO_SYS_TYPES_H
#define CYGONCE_ISO_SYS_TYPES_H
/*========================================================================
//
//      sys/types.h
//
//      POSIX types
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Nick Garnett
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
// Purpose:       This file provides various types required by POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <sys/types.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

/* This is the "standard" way to get size_t from stddef.h,
 * which is the canonical location of the definition.
 */
#define __need_size_t
#include <stddef.h>

#ifdef CYGBLD_ISO_SSIZET_HEADER
# include CYGBLD_ISO_SSIZET_HEADER
#else
typedef long ssize_t;
#endif

#ifdef CYGBLD_ISO_FSTYPES_HEADER
# include CYGBLD_ISO_FSTYPES_HEADER
#else
typedef short dev_t;
typedef unsigned int ino_t;
typedef unsigned int mode_t;
typedef unsigned short nlink_t;
typedef long off_t;
#endif

#ifdef CYGBLD_ISO_SCHEDTYPES_HEADER
# include CYGBLD_ISO_SCHEDTYPES_HEADER
#else
typedef unsigned short gid_t;
typedef unsigned short uid_t;
typedef int pid_t;
#endif

#if CYGINT_ISO_PMUTEXTYPES
# include CYGBLD_ISO_PMUTEXTYPES_HEADER
#endif

#if CYGINT_ISO_PTHREADTYPES
# include CYGBLD_ISO_PTHREADTYPES_HEADER
#endif

/* Include <sys/select.h> for backward compatibility for now */
#define __NEED_FD_SETS_ONLY
#include <sys/select.h>
#undef __NEED_FD_SETS_ONLY

#if !defined(_POSIX_SOURCE)
# if CYGINT_ISO_BSDTYPES
#  ifdef CYGBLD_ISO_BSDTYPES_HEADER
#   include CYGBLD_ISO_BSDTYPES_HEADER
#  endif
# endif
#endif // !defined(_POSIX_SOURCE)

#endif /* CYGONCE_ISO_SYS_TYPES_H multiple inclusion protection */

/* EOF sys/types.h */
