#ifndef CYGONCE_ISO_SCHED_H
#define CYGONCE_ISO_SCHED_H
/*========================================================================
//
//      sched.h
//
//      POSIX scheduler functions
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
// Date:          2000-04-14
// Purpose:       This file provides the scheduler macros, types and functions
//                required by POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <sched.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

#include <time.h>

#if CYGINT_ISO_SCHED_IMPL
# ifdef CYGBLD_ISO_SCHED_IMPL_HEADER
#  include CYGBLD_ISO_SCHED_IMPL_HEADER
# else

//-----------------------------------------------------------------------------
// Scheduling Policys

#define SCHED_OTHER	        1
#define SCHED_FIFO	        2
#define SCHED_RR	        3

//-----------------------------------------------------------------------------
//Process scheduling functions.

#ifdef __cplusplus
extern "C" {
#endif

// Set scheduling parameters for given process.
extern int sched_setparam (pid_t pid, const struct sched_param *param);

// Get scheduling parameters for given process.
extern int sched_getparam (pid_t pid, struct sched_param *param);

// Set scheduling policy and/or parameters for given process.
extern int sched_setscheduler (pid_t pid,
                               int policy,
                               const struct sched_param *param);

// Get scheduling policy for given process.
extern int sched_getscheduler (pid_t pid);

// Force current thread to relinquish the processor.
extern int sched_yield (void);

//-----------------------------------------------------------------------------
// Scheduler parameter limits.

// Get maximum priority value for a policy.
extern int sched_get_priority_max (int policy);

// Get minimum priority value for a policy.
extern int sched_get_priority_min (int policy);

// Get the SCHED_RR interval for the given process.
extern int sched_rr_get_interval (pid_t pid, struct timespec *t);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif 

#endif /* if CYGINT_ISO_SCHED_IMPL */

#endif /* CYGONCE_ISO_SCHED_H multiple inclusion protection */

/* EOF sched.h */
