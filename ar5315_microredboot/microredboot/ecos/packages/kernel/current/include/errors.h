#ifndef CYGONCE_KERNEL_ERRORS_H
#define CYGONCE_KERNEL_ERRORS_H

//==========================================================================
//
//      errors.h
//
//      Error values from kernel
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
// Date:        1997-10-07
// Purpose:     Define error codes
// Description: Error codes returned by various bits of the kernel.
// Usage:       #include <cyg/kernel/errors.h>
//
//####DESCRIPTIONEND####
//
//==========================================================================

// General successful result:

#define CYGERR_OK                               0

// -------------------------------------------------------------------------
// Define base of codes:

#define CYGERR_KERNEL_BASE                      0xEE000000

// -------------------------------------------------------------------------
// Thread related errors

#define CYGERR_KERNEL_THREAD_BASE               (CYGERR_KERNEL_BASE+0x00010000)

#define CYGERR_KERNEL_THREAD_PRIORITY_INVALID   (CYGERR_KERNEL_THREAD_BASE+1)
#define CYGERR_KERNEL_THREAD_PRIORITY_DUPLICATE (CYGERR_KERNEL_THREAD_BASE+2)

// -------------------------------------------------------------------------
// Interrupt related errors

#define CYGERR_KERNEL_INTR_BASE                 (CYGERR_KERNEL_BASE+0x00020000)

#define CYGERR_KERNEL_INTR_VECTOR_INVALID       (CYGERR_KERNEL_INTR_BASE+1)

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_ERRORS_H
// EOF errors.h
