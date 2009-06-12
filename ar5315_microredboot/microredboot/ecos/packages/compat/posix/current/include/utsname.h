#ifndef CYGONCE_UTSNAME_H
#define CYGONCE_UTSNAME_H
//=============================================================================
//
//      utsname.h
//
//      POSIX utsname header
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
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-03-17
// Purpose:       POSIX utsname header
// Description:   This header contains all the definitions needed to support
//                utsnames under eCos. The reader is referred to the POSIX
//                standard or equivalent documentation for details of the
//                functionality contained herein.
//              
// Usage:
//              #include <utsname.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>   // CYGNUM_HAL_STACK_SIZE_MINIMUM

#include <stddef.h>             // NULL, size_t

#include <limits.h>

#include <sys/types.h>

#include <sched.h>              // SCHED_*

//=============================================================================
// UTSName structure

struct utsname
{
    char        sysname[CYG_POSIX_UTSNAME_LENGTH];
    char        nodename[CYG_POSIX_UTSNAME_NODENAME_LENGTH];
    char        release[CYG_POSIX_UTSNAME_LENGTH];
    char        version[CYG_POSIX_UTSNAME_LENGTH];
    char        machine[CYG_POSIX_UTSNAME_LENGTH];
};

//=============================================================================
// uname() function

__externC int uname( struct utsname *name );

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_UTSNAME_H
// End of utsname.h
