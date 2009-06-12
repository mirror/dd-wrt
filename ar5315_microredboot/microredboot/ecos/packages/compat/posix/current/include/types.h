#ifndef CYGONCE_POSIX_TYPES_H
#define CYGONCE_POSIX_TYPES_H
//=============================================================================
//
//      types.h
//
//      POSIX types header
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
// Purpose:       POSIX types header
// Description:   This header contains various POSIX type definitions. These types
//                are implementation defined.
//              
// Usage:         #include <sys/types.h>
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <cyg/infra/cyg_type.h>

//-----------------------------------------------------------------------------
// Basic types.

typedef cyg_uint32 pthread_t;
typedef int pthread_key_t;
typedef int pthread_once_t;

//-----------------------------------------------------------------------------
// Scheduling parameters. At present only the priority is defined.
// Strictly this should be in <sched.h>, but the requirement for pthread_attr_t
// to contain a sched_param object means that it must be here.

struct sched_param
{
    int                 sched_priority;
};

//-----------------------------------------------------------------------------
// Thread attribute structure.

typedef struct pthread_attr_t
{
    unsigned int        detachstate:2,
                        scope:2,
                        inheritsched:2,
                        schedpolicy:2,
                        stackaddr_valid:1,
                        stacksize_valid:1;
    struct sched_param  schedparam;
    void                *stackaddr;
    size_t              stacksize;
} pthread_attr_t;

// Values for detachstate
#define PTHREAD_CREATE_JOINABLE	        1
#define PTHREAD_CREATE_DETACHED	        2

// Values for scope
#define PTHREAD_SCOPE_SYSTEM            1
#define PTHREAD_SCOPE_PROCESS           2

// Values for inheritsched
#define PTHREAD_INHERIT_SCHED           1
#define PTHREAD_EXPLICIT_SCHED          2

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_POSIX_TYPES_H
// End of types.h
