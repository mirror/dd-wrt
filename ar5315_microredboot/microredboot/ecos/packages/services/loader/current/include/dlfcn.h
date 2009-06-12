#ifndef CYGONCE_LOADER_DLFCN_H
#define CYGONCE_LOADER_DLFCN_H

//==========================================================================
//
//      dlfcn.h
//
//      ELF dynamic loader API definitions
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-11-15
// Purpose:             Define ELF dynamic loader API
// Description:         The functions defined here collectively implement the
//                      external API of the ELF dynamic loader.
// Usage:               #include <cyg/loader/dlfcn.h>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>

#include <cyg/infra/cyg_type.h>         /* types etc.           */

// =========================================================================
// Mode values

#define RTLD_NOW        0x00    /* Relocate now (default)               */
#define RTLD_LAZY       0x01    /* Relocate opportunistically           */
#define RTLD_GLOBAL     0x00    /* make symbols available globally (default) */
#define RTLD_LOCAL      0x10    /* keep symbols secret                  */

// =========================================================================
// API calls

__externC void *dlopen (const char *file, int mode);

__externC void *dlopenmem(const void *addr, size_t size, int mode);

__externC int dlclose (void *handle);

__externC void *dlsym (void *handle, const char *name);

__externC const char *dlerror (void);

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_LOADER_DLFCN_H
// EOF dlfcn.h
