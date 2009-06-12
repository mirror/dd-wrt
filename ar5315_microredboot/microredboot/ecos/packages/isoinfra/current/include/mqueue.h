#ifndef CYGONCE_ISO_MQUEUE_H
#define CYGONCE_ISO_MQUEUE_H
/*========================================================================
//
//      mqueue.h
//
//      POSIX message queue functions
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
// Date:          2000-05-05
// Purpose:       This file provides the macros, types and functions
//                for message queues required by POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <mqueue.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */
#include <pkgconf/kernel.h>            /* CYGFUN_KERNEL_THREADS_TIMER */

/* INCLUDES */

#ifdef _POSIX_MESSAGE_PASSING
# ifdef CYGBLD_ISO_MQUEUE_HEADER
#  include CYGBLD_ISO_MQUEUE_HEADER
# else

#include <sys/types.h>  /* size_t and ssize_t */

/* TYPES */

struct mq_attr {
    long mq_flags;    /* mqueue flags */
    long mq_maxmsg;   /* max number of messages */
    long mq_msgsize;  /* max message size */
    long mq_curmsgs;  /* number of messages currently queued */
};

typedef void *mqd_t;

#ifdef __cplusplus
extern "C" {
#endif

/* FUNCTIONS */

extern mqd_t
mq_open( const char * /* name */, int /* oflag */, ... );

extern int
mq_close( mqd_t /* mqdes */ );

extern int
mq_unlink( const char * /* name */ );

extern int
mq_send( mqd_t /* mqdes */, const char * /* msg_ptr */, size_t /* msg_len */,
         unsigned int /* msg_prio */ );

extern ssize_t
mq_receive( mqd_t /* mqdes */, char * /* msg_ptr */, size_t /* msg_len */,
            unsigned int * /* msg_prio */ );

#ifdef _POSIX_REALTIME_SIGNALS

struct sigevent;

extern int
mq_notify( mqd_t /* mqdes */, const struct sigevent * /* notification */ );
#endif

extern int
mq_setattr( mqd_t /* mqdes */, const struct mq_attr * /* mqstat */,
            struct mq_attr * /* omqstat */ );

extern int
mq_getattr( mqd_t /* mqdes */, struct mq_attr * /* mqstat */ );

# ifdef CYGFUN_KERNEL_THREADS_TIMER
/* POSIX 1003.1d Draft functions - FIXME: should be conditionalized */

struct timespec; /* forward declaration */

extern int 
mq_timedsend( mqd_t /* mqdes */, const char * /* msg_ptr */, 
              size_t /* msg_len */, unsigned int /* msg_prio */,
              const struct timespec * /* abs_timeout */ );

extern ssize_t 
mq_timedreceive( mqd_t /* mqdes */, char * /* msg_ptr */, 
                 size_t /* msg_len */, unsigned int * /* msg_prio */,
                 const struct timespec * /* abs_timeout */ );
# endif

#ifdef __cplusplus
}   /* extern "C" */
#endif


# endif  /* ifndef CYGBLD_ISO_MQUEUE_HEADER */
#endif   /* ifdef _POSIX_MESSAGE_PASSING */


#endif /* CYGONCE_ISO_MQUEUE_H multiple inclusion protection */

/* EOF mqueue.h */
