//========================================================================
//
//      raise.cxx
//
//      ISO C and POSIX 1003.1 signals implementation for raise()
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
// Date:          2000-04-18
// Purpose:       Provide implementation of ISO C and POSIX 1003.1 raise()
// Description:   This file provides the linkable symbol version of raise()
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_signals.h>  // libc signals configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support

// The following lines allow __raise() to be included non-inline which
// actually defines it as a function here
#define CYGPRI_LIBC_SIGNALS_RAISE_INLINE
#ifndef CYGIMP_LIBC_SIGNALS_RAISE_INLINE
# define CYGIMP_LIBC_SIGNALS_RAISE_INLINE
#endif

#include <signal.h>                // Main signals definitions

// GLOBALS

#ifdef CYGDBG_USE_TRACING
cyg_uint8
cyg_libc_signals_raise_trace_level = CYGNUM_LIBC_SIGNALS_RAISE_TRACE_LEVEL;
#endif

// EOF raise.cxx
