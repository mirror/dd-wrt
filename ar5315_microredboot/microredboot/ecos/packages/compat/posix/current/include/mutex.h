#ifndef CYGONCE_POSIX_MUTEX_H
#define CYGONCE_POSIX_MUTEX_H
//=============================================================================
//
//      mutex.h
//
//      POSIX mutex and condition variable function definitions
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
// Author(s):     nickg,jlarmour
// Contributors:  
// Date:          2001-09-10
// Purpose:       POSIX mutex and condition variable function definitions
// Description:   This header contains POSIX API definitions for mutexes
//                and cond vars.
//              
// Usage:         #include <pthread.h>
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/posix.h>

#include <sys/types.h>   // pthread_* types

//=============================================================================
// Mutexes

//-----------------------------------------------------------------------------
// Mutex attributes manipulation functions

// Initialize attribute object
externC int pthread_mutexattr_init ( pthread_mutexattr_t *attr);

// Destroy attribute object
externC int pthread_mutexattr_destroy ( pthread_mutexattr_t *attr);

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT)

// Set priority inversion protection protocol
externC int pthread_mutexattr_setprotocol ( pthread_mutexattr_t *attr,
                                            int protocol);

// Get priority inversion protection protocol
externC int pthread_mutexattr_getprotocol ( pthread_mutexattr_t *attr,
                                            int *protocol);

#if defined(_POSIX_THREAD_PRIO_PROTECT)

// Set priority for priority ceiling protocol
externC int pthread_mutexattr_setprioceiling ( pthread_mutexattr_t *attr,
                                               int prioceiling);

// Get priority for priority ceiling protocol
externC int pthread_mutexattr_getprioceiling ( pthread_mutexattr_t *attr,
                                               int *prioceiling);

// Set priority ceiling of given thread, returning old ceiling.
externC int pthread_mutex_setprioceiling( pthread_mutex_t *mutex,
                                          int prioceiling,
                                          int *old_ceiling);

// Get priority ceiling of given thread
externC int pthread_mutex_getprioceiling( pthread_mutex_t *mutex,
                                          int *prioceiling);
#endif

#endif

//-----------------------------------------------------------------------------
// Mutex functions

// Initialize mutex. If mutex_attr is NULL, use default attributes.
externC int pthread_mutex_init (pthread_mutex_t *mutex,
                                const pthread_mutexattr_t *mutex_attr);

// Destroy mutex.
externC int pthread_mutex_destroy (pthread_mutex_t *mutex);

// Lock mutex, waiting for it if necessary.
externC int pthread_mutex_lock (pthread_mutex_t *mutex);

// Try to lock mutex.
externC int pthread_mutex_trylock (pthread_mutex_t *mutex);


// Unlock mutex.
externC int pthread_mutex_unlock (pthread_mutex_t *mutex);



//=============================================================================
// Condition Variables

//-----------------------------------------------------------------------------
// Attribute manipulation functions
// We do not actually support any attributes at present, so these do nothing.

// Initialize condition variable attributes
externC int pthread_condattr_init (pthread_condattr_t *attr);

// Destroy condition variable attributes
externC int pthread_condattr_destroy (pthread_condattr_t *attr);

//-----------------------------------------------------------------------------
// Condition variable functions

// Initialize condition variable.
externC int pthread_cond_init (pthread_cond_t *cond,
                               const pthread_condattr_t *attr);

// Destroy condition variable.
externC int pthread_cond_destroy (pthread_cond_t *cond);

// Wake up one thread waiting for condition variable
externC int pthread_cond_signal (pthread_cond_t *cond);

// Wake up all threads waiting for condition variable
externC int pthread_cond_broadcast (pthread_cond_t *cond);

// Block on condition variable until signalled. The mutex is
// assumed to be locked before this call, will be unlocked
// during the wait, and will be re-locked on wakeup.
externC int pthread_cond_wait (pthread_cond_t *cond,
                               pthread_mutex_t *mutex);

// Block on condition variable until signalled, or the timeout expires.
externC int pthread_cond_timedwait (pthread_cond_t *cond,
                                    pthread_mutex_t *mutex,
                                    const struct timespec *abstime);


//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_POSIX_MUTEX_H
// End of mutex.h
