#ifndef CYGONCE_ISO_LIMITS_H
#define CYGONCE_ISO_LIMITS_H
/*========================================================================
//
//      limits.h
//
//      ISO standard limits
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
// Purpose:       This file provides the limits properties
//                required by ISO C and POSIX 1003.1
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation), as well as
//                being partially provided by the compiler.
//
// Usage:         #include <limits.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* DEFINES */

/*-----------------------------------------------------------------------------
 * Minimum values from POSIX.1 tables 2-3, 2-7 and 2-7a.

 * These are the standard-mandated minimum values.
 * These values do not vary with the implementation - they may
 * simply be defined
 */

/* Minimum number of operations in one list I/O call.  */
#define _POSIX_AIO_LISTIO_MAX	2

/* Minimal number of outstanding asynchronous I/O operations.  */
#define _POSIX_AIO_MAX		1

/* Maximum length of arguments to `execve', including environment.  */
#define	_POSIX_ARG_MAX		4096

/* Maximum simultaneous processes per real user ID.  */
#define	_POSIX_CHILD_MAX	6

/* Minimal number of timer expiration overruns.  */
#define _POSIX_DELAYTIMER_MAX	32

/* Maximum link count of a file.  */
#define	_POSIX_LINK_MAX		8

/* Size of storage required for a login name */
#define _POSIX_LOGIN_NAME_MAX   9

/* Number of bytes in a terminal canonical input queue.  */
#define	_POSIX_MAX_CANON	255

/* Number of bytes for which space will be
   available in a terminal input queue.  */
#define	_POSIX_MAX_INPUT	255

/* Maximum number of message queues open for a process.  */
#define _POSIX_MQ_OPEN_MAX	8

/* Maximum number of supported message priorities.  */
#define _POSIX_MQ_PRIO_MAX	32

/* Number of bytes in a filename.  */
#define	_POSIX_NAME_MAX		14

/* Number of simultaneous supplementary group IDs per process.  */
#define	_POSIX_NGROUPS_MAX	0

/* Number of files one process can have open at once.  */
#define	_POSIX_OPEN_MAX		16

/* Number of bytes in a pathname.  */
#define	_POSIX_PATH_MAX		255

/* Number of bytes than can be written atomically to a pipe.  */
#define	_POSIX_PIPE_BUF		512

/* Minimal number of realtime signals reserved for the application.  */
#define _POSIX_RTSIG_MAX	8

/* Number of semaphores a process can have.  */
#define _POSIX_SEM_NSEMS_MAX	256

/* Maximal value of a semaphore.  */
#define _POSIX_SEM_VALUE_MAX	32767

/* Number of pending realtime signals.  */
#define _POSIX_SIGQUEUE_MAX	32

/* Largest value of a `ssize_t'.  */
#define	_POSIX_SSIZE_MAX	32767

/* Number of streams a process can have open at once.  */
#define	_POSIX_STREAM_MAX	8

/* Controlling the iterations of destructors for thread-specific data.  */
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS	4

/* The number of data keys per process.  */
#define _POSIX_THREAD_KEYS_MAX	128

/* The number of threads per process.  */
#define _POSIX_THREAD_THREADS_MAX	64

/* Maximum number of characters in a tty name.  */
#define	_POSIX_TTY_NAME_MAX	9

/* Number of timer for a process.  */
#define _POSIX_TIMER_MAX	32

/* Maximum length of a timezone name (element of `tzname').  */
#define	_POSIX_TZNAME_MAX	3

/* Maximum clock resolution in nanoseconds.  */
#define _POSIX_CLOCKRES_MIN	20000000


#ifdef CYGBLD_ISO_SSIZET_HEADER
# include CYGBLD_ISO_SSIZET_HEADER
#else
# ifndef __STRICT_ANSI__
#  define SSIZE_MAX LONG_MAX
# endif
#endif

/* INCLUDES */

#ifdef CYGBLD_ISO_OPEN_MAX_HEADER
# include CYGBLD_ISO_OPEN_MAX_HEADER
#else
# ifndef __STRICT_ANSI__
#  define OPEN_MAX _POSIX_OPEN_MAX
# endif
#endif

#ifdef CYGBLD_ISO_LINK_MAX_HEADER
# include CYGBLD_ISO_LINK_MAX_HEADER
#else
# ifndef __STRICT_ANSI__
#  define LINK_MAX _POSIX_LINK_MAX
# endif
#endif

#ifdef CYGBLD_ISO_NAME_MAX_HEADER
# include CYGBLD_ISO_NAME_MAX_HEADER
#else
# ifndef __STRICT_ANSI__
#  define NAME_MAX _POSIX_NAME_MAX
# endif
#endif

#ifdef CYGBLD_ISO_PATH_MAX_HEADER
# include CYGBLD_ISO_PATH_MAX_HEADER
#else
# ifndef __STRICT_ANSI__
#  define PATH_MAX _POSIX_PATH_MAX
# endif
#endif

#if CYGINT_ISO_POSIX_LIMITS
# ifdef CYGBLD_ISO_POSIX_LIMITS_HEADER
#  include CYGBLD_ISO_POSIX_LIMITS_HEADER
# endif
#endif

#endif /* CYGONCE_ISO_LIMITS_H multiple inclusion protection */
       /* Yes it must be ended here! */

/* When using a crosscompiler targeting linux, the next limits.h file
   in the include sequence may be the glibc header - which breaks our
   world. So skip it by defining _LIBC_LIMITS_H_ */
#define _LIBC_LIMITS_H_

/* Secondly only include if we haven't already been included by it. */
#ifndef _GCC_LIMITS_H_
# include_next <limits.h>
#endif

/* EOF limits.h */
