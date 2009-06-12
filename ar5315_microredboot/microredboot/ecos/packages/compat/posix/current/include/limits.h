#ifndef CYGONCE_LIMITS_H
#define CYGONCE_LIMITS_H
//=============================================================================
//
//      limits.h
//
//      POSIX limits header
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
// Purpose:       POSIX limits header
// Description:   This file contains the compile time definitions that describe
//                the minima and current values of various values and defined
//                in POSIX.1 section 2.8.
//              
// Usage:
//              #include <limits.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/posix.h>

//-----------------------------------------------------------------------------
// Runtime invariants
// These are all equal to or greater than the minimum values defined above.

// From table 2-4

#define NGROUPS_MAX             _POSIX_NGROUPS_MAX

// From table 2-5

#define AIO_LISTIO_MAX          _POSIX_AIO_LISTIO_MAX

#define AIO_MAX                 _POSIX_AIO_MAX

#define AIO_PRIO_DELTA_MAX      0

#define ARG_MAX                 _POSIX_ARG_MAX

#define CHILD_MAX               _POSIX_CHILD_MAX

#define DELAYTIMER_MAX          _POSIX_DELAYTIMER_MAX

#define LOGIN_NAME_MAX          _POSIX_LOGIN_NAME_MAX

#define PAGESIZE                1

#define PTHREAD_DESTRUCTOR_ITERATIONS CYGNUM_POSIX_PTHREAD_DESTRUCTOR_ITERATIONS

#define PTHREAD_KEYS_MAX        CYGNUM_POSIX_PTHREAD_KEYS_MAX

// Minimum size needed on a pthread stack to contain its per-thread control
// structures and per-thread data. Since we cannot include all the headers
// necessary to calculate this value here, the following is a generous estimate.
#define PTHREAD_STACK_OVERHEAD  (0x180+(PTHREAD_KEYS_MAX*sizeof(void *)))

#define PTHREAD_STACK_MIN       (CYGNUM_HAL_STACK_SIZE_MINIMUM+PTHREAD_STACK_OVERHEAD)

#define PTHREAD_THREADS_MAX     CYGNUM_POSIX_PTHREAD_THREADS_MAX

#define RTSIG_MAX               _POSIX_RTSIG_MAX

#define SEM_NSEMS_MAX           _POSIX_SEM_NSEMS_MAX

#define SEM_VALUE_MAX           _POSIX_SEM_VALUE_MAX

#define SIGQUEUE_MAX            _POSIX_SIGQUEUE_MAX

#define STREAM_MAX              _POSIX_STREAM_MAX

#define TIMER_MAX               _POSIX_TIMER_MAX

#define TTY_NAME_MAX            _POSIX_TTY_NAME_MAX

#define TZNAME_MAX              _POSIX_TZNAME_MAX


// From table 2-6.

#define MAX_CANON               _POSIX_MAX_CANON

#define MAX_INPUT               _POSIX_MAX_INPUT

#define PIPE_BUF                _POSIX_PIPE_BUF


//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_LIMITS_H
// End of limits.h
