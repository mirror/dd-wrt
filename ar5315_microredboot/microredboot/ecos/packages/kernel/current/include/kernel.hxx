#ifndef CYGONCE_KERNEL_KERNEL_HXX
#define CYGONCE_KERNEL_KERNEL_HXX

//==========================================================================
//
//      kernel.hxx
//
//      Kernel mega-include file
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-09
// Purpose:     Include all kernel files
// Description: This file contains includes for all the kernel
//              headers. This simplifys things in the sources.
// Usage:       #include <cyg/kernel/kernel.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>

#include <cyg/infra/cyg_ass.h>            // assertion macros
#include <cyg/infra/cyg_trac.h>           // tracing macros

#include <cyg/kernel/errors.h>

#include <cyg/kernel/instrmnt.h>

#include <cyg/kernel/diag.h>

#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/intr.hxx>
#include <cyg/kernel/clock.hxx>

#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/mutex.hxx>

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/clock.inl>

// -------------------------------------------------------------------------
#endif  // #ifndef CYGONCE_KERNEL_KERNEL_HXX
// EOF kernel.hxx
