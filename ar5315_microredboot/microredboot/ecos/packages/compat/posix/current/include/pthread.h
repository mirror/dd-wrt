#ifndef CYGONCE_PTHREAD_H
#define CYGONCE_PTHREAD_H
//=============================================================================
//
//      pthread.h
//
//      POSIX pthread header
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Purpose:       POSIX pthread header
// Description:   This header contains all the definitions needed to support
//                pthreads under eCos. The reader is referred to the POSIX
//                standard or equivalent documentation for details of the
//                functionality contained herein.
//              
// Usage:
//              #include <pthread.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>   // CYGNUM_HAL_STACK_SIZE_MINIMUM

#include <stddef.h>             // NULL, size_t

#include <limits.h>

#include <sys/types.h>

#include <sched.h>              // SCHED_*

//=============================================================================
// General thread operations

//-----------------------------------------------------------------------------
// Thread creation and management.

// Create a thread.
__externC int pthread_create (pthread_t *__pthread,
                              const pthread_attr_t *__attr,
                              void *(*__start_routine) (void *),
                              void *__arg);

// Get current thread id.
__externC pthread_t pthread_self (void);

// Compare two thread identifiers.
__externC int pthread_equal (pthread_t __thread1, pthread_t __thread2);

// Terminate current thread.
__externC void pthread_exit (void *__retval) CYGBLD_ATTRIB_NORET;

// Wait for the thread to terminate. If thread_return is not NULL then
// the retval from the thread's call to pthread_exit() is stored at
// *thread_return.
__externC int pthread_join (pthread_t __pthread, void **__thread_return);

// Set the detachstate of the thread to "detached". The thread then does not
// need to be joined and its resources will be freed when it exits.
__externC int pthread_detach (pthread_t __pthread);

//-----------------------------------------------------------------------------
// Thread attribute handling.

// Initialize attributes object with default attributes:
// detachstate          == PTHREAD_JOINABLE
// scope                == PTHREAD_SCOPE_SYSTEM
// inheritsched         == PTHREAD_EXPLICIT_SCHED
// schedpolicy          == SCHED_OTHER
// schedparam           == unset
// stackaddr            == unset
// stacksize            == 0
// 
__externC int pthread_attr_init (pthread_attr_t *__attr);

// Destroy thread attributes object
__externC int pthread_attr_destroy (pthread_attr_t *__attr);


// Set the detachstate attribute
__externC int pthread_attr_setdetachstate (pthread_attr_t *__attr,
                                           int __detachstate);

// Get the detachstate attribute
__externC int pthread_attr_getdetachstate (const pthread_attr_t *__attr,
                                           int *__detachstate);


// Set scheduling contention scope
__externC int pthread_attr_setscope (pthread_attr_t *__attr, int __scope);

// Get scheduling contention scope
__externC int pthread_attr_getscope (const pthread_attr_t *__attr, int *__scope);


// Set scheduling inheritance attribute
__externC int pthread_attr_setinheritsched (pthread_attr_t *__attr, int __inherit);

// Get scheduling inheritance attribute
__externC int pthread_attr_getinheritsched (const pthread_attr_t *__attr,
                                            int *__inherit);


// Set scheduling policy
__externC int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy);

// Get scheduling policy
__externC int pthread_attr_getschedpolicy (const pthread_attr_t *__attr,
                                           int *__policy);


// Set scheduling parameters
__externC int pthread_attr_setschedparam (pthread_attr_t *__attr,
                                          const struct sched_param *__param);

// Get scheduling parameters
__externC int pthread_attr_getschedparam (const pthread_attr_t *__attr,
                                          struct sched_param *__param);


// Set starting address of stack. Whether this is at the start or end of
// the memory block allocated for the stack depends on whether the stack
// grows up or down.
__externC int pthread_attr_setstackaddr (pthread_attr_t *__attr, void *__stackaddr);

// Get any previously set stack address.
__externC int pthread_attr_getstackaddr (const pthread_attr_t *__attr,
                                         void **__stackaddr);


// Set minimum creation stack size.
__externC int pthread_attr_setstacksize (pthread_attr_t *__attr,
                                         size_t __stacksize);

// Get current minimal stack size.
__externC int pthread_attr_getstacksize (const pthread_attr_t *__attr,
                                         size_t *__stacksize);

//-----------------------------------------------------------------------------
// Thread scheduling controls

// Set scheduling policy and parameters for the thread
__externC int pthread_setschedparam (pthread_t __pthread,
                                     int __policy,
                                     const struct sched_param *__param);

// Get scheduling policy and parameters for the thread
__externC int pthread_getschedparam (pthread_t __pthread,
                                     int *__policy,
                                     struct sched_param *__param);



//=============================================================================
// Dynamic package initialization

// Initializer for pthread_once_t instances
#define PTHREAD_ONCE_INIT       0

// Call init_routine just the once per control variable.
__externC int pthread_once (pthread_once_t *__once_control,
                            void (*__init_routine) (void));



//=============================================================================
//Thread specific data

// Create a key to identify a location in the thread specific data area.
// Each thread has its own distinct thread-specific data area but all are
// addressed by the same keys. The destructor function is called whenever a
// thread exits and the value associated with the key is non-NULL.
__externC int pthread_key_create (pthread_key_t *__key,
                                  void (*__destructor) (void *));

// Delete key.
__externC int pthread_key_delete (pthread_key_t __key);

// Store the pointer value in the thread-specific data slot addressed
// by the key.
__externC int pthread_setspecific (pthread_key_t __key, const void *__pointer);

// Retrieve the pointer value in the thread-specific data slot addressed
// by the key.
__externC void *pthread_getspecific (pthread_key_t __key);



//=============================================================================
// Thread Cancellation

//-----------------------------------------------------------------------------
// Data structure used to manage cleanup functions

struct pthread_cleanup_buffer
{
    struct pthread_cleanup_buffer *prev;        // Chain cleanup buffers
    void (*routine) (void *);     	        // Function to call
    void *arg;				        // Arg to pass
};

//-----------------------------------------------------------------------------
// Thread cancelled return value.
// This is a value returned as the retval in pthread_join() of a
// thread that has been cancelled. By making it the address of a
// location we define we can ensure that it differs from NULL and any
// other valid pointer (as required by the standard).

__externC int pthread_canceled_dummy_var;

#define PTHREAD_CANCELED                ((void *)(&pthread_canceled_dummy_var))

//-----------------------------------------------------------------------------
// Cancelability enable and type

#define PTHREAD_CANCEL_ENABLE           1
#define PTHREAD_CANCEL_DISABLE          2

#define PTHREAD_CANCEL_ASYNCHRONOUS     1
#define PTHREAD_CANCEL_DEFERRED         2

//-----------------------------------------------------------------------------
// Functions

// Set cancel state of current thread to ENABLE or DISABLE.
// Returns old state in *oldstate.
__externC int pthread_setcancelstate (int __state, int *__oldstate);

// Set cancel type of current thread to ASYNCHRONOUS or DEFERRED.
// Returns old type in *oldtype.
__externC int pthread_setcanceltype (int __type, int *__oldtype);

// Cancel the thread.
__externC int pthread_cancel (pthread_t __pthread);

// Test for a pending cancellation for the current thread and terminate
// the thread if there is one.
__externC void pthread_testcancel (void);

// Install a cleanup routine.
// Note that pthread_cleanup_push() and pthread_cleanup_pop() are macros that
// must be used in matching pairs and at the same brace nesting level.
#define pthread_cleanup_push(__routine, __arg)                          \
    {                                                                   \
        struct pthread_cleanup_buffer _buffer_;                         \
        pthread_cleanup_push_inner (&_buffer_, (__routine), (__arg));

// Remove a cleanup handler installed by the matching pthread_cleanup_push().
// If execute is non-zero, the handler function is called.
#define pthread_cleanup_pop(__execute)                          \
        pthread_cleanup_pop_inner (&_buffer_, (__execute));     \
    }


// These two functions actually implement the cleanup push and pop functionality.
__externC void pthread_cleanup_push_inner (struct pthread_cleanup_buffer *__buffer,
                                           void (*__routine) (void *),
                                           void *__arg);

__externC void pthread_cleanup_pop_inner (struct pthread_cleanup_buffer *__buffer,
                                          int __execute);


// -------------------------------------------------------------------------
// eCos-specific function to measure stack usage of the supplied thread

#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
__externC size_t pthread_measure_stack_usage (pthread_t __pthread);
#endif

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_PTHREAD_H
// End of pthread.h
