#ifndef CYGONCE_POSIX_EXPORT_H
#define CYGONCE_POSIX_EXPORT_H
//=============================================================================
//
//      export.h
//
//      POSIX export header
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-09-18
// Purpose:       POSIX export header
// Description:   This header contains definitions that the POSIX package exports
//                to other packages. These are generally interfaces that are not
//                provided by the public API.
//                
//              
// Usage:
//              #ifdef CYGPKG_POSIX
//              #include <export.h>
//              #endif
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

#include <stddef.h>             // NULL, size_t

#include <limits.h>
#include <signal.h>

#include <sys/types.h>

#include <sched.h>              // SCHED_*

//=============================================================================
// POSIX API function management.
// These macros should be inserted near the start and all returns of
// any function that is part of the POSIX API.

__externC void cyg_posix_function_start();
__externC void cyg_posix_function_finish();

#define CYG_POSIX_FUNCTION_START() cyg_posix_function_start()

#define CYG_POSIX_FUNCTION_FINISH() cyg_posix_function_finish()

//-----------------------------------------------------------------------------
// Signal mask management
//
// These are exported to allow functions in other packages to
// manipulate the current threads signal mask. they are currently only
// used in the implementation of cyg_pselect() in the FILEIO package.

#ifdef CYGPKG_POSIX_SIGNALS
__externC void cyg_pthread_sigmask_set (const sigset_t *set, sigset_t *oset);
__externC cyg_bool cyg_posix_sigpending(void);
__externC void cyg_posix_deliver_signals(const sigset_t *mask);

#define CYG_PTHREAD_SIGMASK_SET cyg_pthread_sigmask_set

#define CYG_POSIX_SIGPENDING() cyg_posix_sigpending()

#define CYG_POSIX_DELIVER_SIGNALS cyg_posix_deliver_signals
#endif

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_POSIX_EXPORT_H
// End of export.h
