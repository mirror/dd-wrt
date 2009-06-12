#ifndef CYGONCE_SEMAPHORE_H
#define CYGONCE_SEMAPHORE_H
//=============================================================================
//
//      semaphore.h
//
//      POSIX semaphore header
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
// Purpose:       POSIX semaphore header
// Description:   This header contains all the definitions needed to support
//                semaphores under eCos. The reader is referred to the POSIX
//                standard or equivalent documentation for details of the
//                functionality contained herein.
//              
// Usage:
//              #include <semaphore.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <stddef.h>             // NULL, size_t

#include <limits.h>

#include <sys/types.h>

//-----------------------------------------------------------------------------
// Semaphore object definition

// This structure must exactly correspond in size and layout to the underlying
// eCos C++ class that implements this object.

typedef struct
{
  CYG_WORD32    sem_value;
  CYG_ADDRESS   sem_queue;
} sem_t;

//-----------------------------------------------------------------------------
// Semaphore functions

// Initialize semaphore to value.
// pshared is not supported under eCos.
externC int sem_init  (sem_t *sem, int pshared, unsigned int value);

// Destroy the semaphore.
externC int sem_destroy  (sem_t *sem);

// Decrement value if >0 or wait for a post.
externC int sem_wait  (sem_t *sem);

// Decrement value if >0, return -1 if not.
externC int sem_trywait  (sem_t *sem);

// Increment value and wake a waiter if one is present.
externC int sem_post  (sem_t *sem);

// Get current value
externC int sem_getvalue  (sem_t *sem, int *sval);

//-----------------------------------------------------------------------------
// Named semaphore functions
// These are an optional feature under eCos

// Open an existing named semaphore, or create it.
externC sem_t *sem_open  (const char *name, int oflag, ...);

// Close descriptor for semaphore.
externC int sem_close  (sem_t *sem);

// Remove named semaphore
externC int sem_unlink  (const char *name);

//-----------------------------------------------------------------------------
// Special return value for sem_open()

#define SEM_FAILED      ((sem_t *)NULL)

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_SEMAPHORE_H
// End of semaphore.h
