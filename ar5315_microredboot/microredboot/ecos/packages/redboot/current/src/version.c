//==========================================================================
//
//      version.c
//
//      RedBoot version "string"
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-12-13
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/redboot.h>
#include <cyg/hal/hal_arch.h>

//
// These strings are used for informational purposes only.  If the end
// user wishes to replace them, simply redefine them in a platform specific
// file, e.g. in a local file used to provide additional local RedBoot
// customization.  Examples of such are platforms which define a special
// 'exec' command used to start a Linux kernel.
//
// CAUTION! Since this file is compiled using very special rules, it is not
// possible to replace it with a local version contained in the "build" tree.
// Replacing the information exported herein is accomplished via the techniques
// outlined above.
//

// Do not change the following two lines at all. They are fiddled by release
// scripts.
#define _CERTIFICATE Non-certified
//#define CYGDAT_REDBOOT_CUSTOM_VERSION current

#if defined(CYGDAT_REDBOOT_CUSTOM_VERSION)
#define _REDBOOT_VERSION CYGDAT_REDBOOT_CUSTOM_VERSION
#elif defined(CYGPKG_REDBOOT_current)
#define _REDBOOT_VERSION UNKNOWN
#else
#define _REDBOOT_VERSION CYGPKG_REDBOOT
#endif

#define __s(x) #x
#define _s(x) __s(x)

char RedBoot_version[] CYGBLD_ATTRIB_WEAK =
   "\nRedBoot(tm) bootstrap and debug environment [" _s(CYG_HAL_STARTUP) "]"
   "\n" _s(_CERTIFICATE) " release, version " _s(_REDBOOT_VERSION) 
   " - built " __TIME__ ", " __DATE__ "\n\n";

// Override default GDB stubs 'info'
// Note: this can still be a "weak" symbol since it will occur in the .o
// file explicitly mentioned in the link command.  User programs will 
// still be allowed to override it.
char GDB_stubs_version[] CYGBLD_ATTRIB_WEAK = 
    "eCos GDB stubs [via RedBoot] - built " __DATE__ " / " __TIME__;
