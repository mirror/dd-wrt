#ifndef CYGONCE_ISO_DIRENT_H
#define CYGONCE_ISO_DIRENT_H
/*========================================================================
//
//      dirent.h
//
//      POSIX file control functions
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
// Author(s):     nickg
// Contributors:  
// Date:          2000-06-26
// Purpose:       This file provides the macros, types and functions
//                for directory operations required by POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <dirent.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* INCLUDES */

#include <pkgconf/isoinfra.h>

#include <sys/types.h>
#include <limits.h>    
    
#ifdef __cplusplus
extern "C" {
#endif

#if CYGINT_ISO_DIRENT    
#ifdef CYGBLD_ISO_DIRENT_HEADER
# include CYGBLD_ISO_DIRENT_HEADER
#endif

/* PROTOTYPES */
    
extern DIR *opendir( const char *dirname );

extern struct dirent *readdir( DIR *dirp );
    
extern int readdir_r( DIR *dirp, struct dirent *entry, struct dirent **result );

extern void rewinddir( DIR *dirp );

extern int closedir( DIR *dirp );

#endif
    
#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* CYGONCE_ISO_DIRENT_H multiple inclusion protection */

/* EOF dirent.h */
