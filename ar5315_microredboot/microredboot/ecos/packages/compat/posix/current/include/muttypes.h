#ifndef CYGONCE_POSIX_MUTTYPES_H
#define CYGONCE_POSIX_MUTTYPES_H
//=============================================================================
//
//      muttypes.h
//
//      POSIX mutex types header
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
// Contributors:  nickg,jlarmour
// Date:          2000-03-17
// Purpose:       POSIX types header
// Description:   This header contains POSIX type definitions for mutexes
//                and cond vars. These types are implementation defined.
//              
// Usage:         #include <sys/types.h>
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/posix.h>
#include <pkgconf/kernel.h>

#include <cyg/infra/cyg_type.h>

//-----------------------------------------------------------------------------
// Mutex object
// This structure must exactly correspond in size and layout to the underlying
// eCos C++ class that implements this object. Because we have to support
// PTHREAD_MUTEX_INITIALIZER we cannot abstract this object very easily.

typedef struct
{
    CYG_WORD32  locked;
    CYG_ADDRESS owner;
    CYG_ADDRESS queue;

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
    CYG_WORD32  protocol;       // this mutex's protocol
#endif    
    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING
    CYG_WORD32  ceiling;        // mutex priority ceiling
#endif
    
} pthread_mutex_t;

#if defined(CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC) &&\
    defined(CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING)
#define PTHREAD_MUTEX_INITIALIZER { 0, 0, 0, 0, 0 }
#elif defined(CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC) ||\
    defined(CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING)
#define PTHREAD_MUTEX_INITIALIZER { 0, 0, 0, 0 }
#else
#define PTHREAD_MUTEX_INITIALIZER { 0, 0, 0 }
#endif

//-----------------------------------------------------------------------------
// Mutex attributes structure

typedef struct
{
    int         protocol;
#ifdef _POSIX_THREAD_PRIO_PROTECT    
    int         prioceiling;
#endif    
} pthread_mutexattr_t;

// Values for protocol
#define PTHREAD_PRIO_NONE       1
#if defined(_POSIX_THREAD_PRIO_INHERIT)
#define PTHREAD_PRIO_INHERIT    2
#endif
#if defined(_POSIX_THREAD_PRIO_PROTECT)
#define PTHREAD_PRIO_PROTECT    3
#endif

//-----------------------------------------------------------------------------
// Condition Variable structure.
// Like mutexes, this must match the underlying eCos implementation class.

typedef struct
{
    CYG_ADDRESS         mutex;
    CYG_ADDRESS         queue;    
} pthread_cond_t;

#define PTHREAD_COND_INITIALIZER { 0, 0 }

//-----------------------------------------------------------------------------
// Condition variable attributes structure

typedef struct
{
    int         dummy;
} pthread_condattr_t;

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_POSIX_MUTTYPES_H
// End of muttypes.h
